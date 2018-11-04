
#include "r_storage/r_queries.h"
#include "r_storage/r_append_file.h"
#include "r_storage/r_file_index.h"
#include "r_db/r_sqlite_conn.h"
#include "r_utils/r_time_utils.h"
#include "r_utils/r_string.h"
#include "r_utils/r_byte_ptr.h"
#include "r_utils/r_blob_tree.h"
#include "r_utils/3rdparty/json/json.h"

using namespace r_storage;
using namespace r_db;
using namespace r_utils;
using json = nlohmann::json;
using namespace std;
using namespace std::chrono;

string r_storage::contents(const string& indexPath,
                           const string& dataSourceID,
                           const system_clock::time_point& start,
                           const system_clock::time_point& end,
                           bool utc)
{
    auto startTime = r_time::tp_to_epoch_millis(start);
    auto endTime = r_time::tp_to_epoch_millis(end);

    r_sqlite_conn dbconn(indexPath);

    auto results = dbconn.exec(r_string_utils::format("SELECT start_time, CASE WHEN end_time=0 THEN 32896105872547 ELSE end_time END as end_time FROM segment_files WHERE start_time >= %s AND end_time < %s AND data_source_id='%s' AND type='video' ORDER BY start_time;",
                                                r_string_utils::uint64_to_s(startTime).c_str(),
                                                r_string_utils::uint64_to_s(endTime).c_str(),
                                                dataSourceID.c_str()));

    json j;

    j["data"]["segments"] = json::array();

    bool inSeg = false;
    uint64_t segStart = 0;
    uint64_t segEnd = 0;

    for(auto r : results)
    {
        if(inSeg)
        {
            auto fileStart = r_string_utils::s_to_uint64(r["start_time"]);

            if((fileStart-segEnd) < 3000)
                segEnd = r_string_utils::s_to_uint64(r["end_time"]);
            else
            {
                // write segments json
                j["data"]["segments"] += {{"end_time", (segEnd==32896105872547)?"0":r_time::tp_to_iso_8601(r_time::epoch_millis_to_tp(segEnd), utc)},
                                          {"start_time", r_time::tp_to_iso_8601(r_time::epoch_millis_to_tp(segStart), utc)}};
                inSeg = false;
            }
        }

        if(!inSeg)
        {
            inSeg = true;
            segStart = r_string_utils::s_to_uint64(r["start_time"]);
            segEnd = r_string_utils::s_to_uint64(r["end_time"]);
        }
    }

    if(inSeg)
        j["data"]["segments"] += {{"end_time", (segEnd==32896105872547)?"0":r_time::tp_to_iso_8601(r_time::epoch_millis_to_tp(segEnd), utc)},
                                  {"start_time", r_time::tp_to_iso_8601(r_time::epoch_millis_to_tp(segStart), utc)}};

    return j.dump();
}

vector<uint8_t> r_storage::key_before(const string& indexPath,
                                      const string& dataSourceID,
                                      const system_clock::time_point& time)
{
    r_sqlite_conn dbconn(indexPath);

    auto epochTime = r_time::tp_to_epoch_millis(time);

    auto results = dbconn.exec(r_string_utils::format("SELECT * FROM segment_files "
                                                "WHERE start_time <= %s AND data_source_id='%s' AND type='video' ORDER BY start_time DESC LIMIT 1;",
                                                r_string_utils::uint64_to_s(epochTime).c_str(),
                                                dataSourceID.c_str()));
    
    if(results.empty())
        R_STHROW(r_not_found_exception, ("No files found near %s for dataSourceID %s", r_time::tp_to_iso_8601(time, false).c_str(), dataSourceID.c_str()));

    r_append_file file(results.front()["path"]);

    auto iter = file.get_iterator();

    iter.find(epochTime);

    uint64_t key;
    uint32_t flags, payloadLen;
    const uint8_t* payload;

    bool foundKey = false;
    while(!foundKey && iter.valid())
    {
        iter.current_data(key, flags, payloadLen, payload);
        if(flags & FLAG_KEY)
        {
            foundKey = true;
            continue;
        }
        else iter.prev();
    }

    vector<uint8_t> result(payloadLen);
    memcpy(&result[0], payload, payloadLen);
    return result;
}

vector<uint8_t> r_storage::query(const string& indexPath,
                                 const string& dataSourceID,
                                 const string& type,
                                 bool previousPlayable,
                                 const system_clock::time_point& start,
                                 const system_clock::time_point& end)
{
    vector<uint8_t> frameBox;
    uint32_t frameBoxPos = 0;
    vector<uint8_t> indexBox;
    uint32_t numFramesBoxed = 0;
    vector<uint8_t> headerBox;

    r_file_index index(indexPath);

    auto fileIter = index.get_iterator("start_time", dataSourceID, type);

    auto epochStart = r_time::tp_to_epoch_millis(start);
    auto epochEnd = r_time::tp_to_epoch_millis(end);

    fileIter.find(epochStart);

    auto sf = fileIter.current_data();

    // find uses lower_bound() which will find the first file start_time greater
    // than our start time... so we need to back up one.

    if(sf.start_time > epochStart)
    {
        fileIter.prev();

        if(!fileIter.valid())
            R_STHROW(r_not_found_exception, ("Unable to find query start_time."));

        sf = fileIter.current_data();
    }

    string sdp = sf.sdp;

    bool fileNeedsFind = true;

    do
    {
        r_append_file af(sf.path);

        auto frameIter = af.get_iterator();

        if(fileNeedsFind)
        {
            frameIter.find(epochStart);
            fileNeedsFind = false;
        }
        else frameIter.begin();

        if(!frameIter.valid())
            R_STHROW(r_not_found_exception, ("Unable to find valid position in append file."));    

        uint64_t frameKey;
        uint32_t flags, payloadLen;
        const uint8_t* payload;
        frameIter.current_data(frameKey, flags, payloadLen, payload);

        if(previousPlayable)
        {
            while(!(flags & r_storage::FLAG_KEY))
            {
                frameIter.prev();
                frameIter.current_data(frameKey, flags, payloadLen, payload);
            }
            previousPlayable = false;
        }

        do
        {
            auto fbp = r_byte_ptr_rw::append(frameBox, payloadLen);
            fbp.bulk_write(payload, payloadLen);

            // write index box entry
            auto ibp = r_byte_ptr_rw::append(indexBox, 20);
            // frame time
            uint64_t doubleWord = r_networking::r_htonll(frameKey);
            ibp.write<uint64_t>(doubleWord);
            // offset of frame in framebox
            uint32_t word = r_networking::r_htonl(frameBoxPos);
            ibp.write<uint32_t>(word);
            // frame size
            word = r_networking::r_htonl(payloadLen);
            ibp.write<uint32_t>(word);
            // flags (keyness, etc...)
            word = r_networking::r_htonl(flags);
            ibp.write<uint32_t>(word);

            frameBoxPos += payloadLen;
            ++numFramesBoxed;

            frameIter.next();
            if(frameIter.valid())
                frameIter.current_data(frameKey, flags, payloadLen, payload);
        }
        while(frameIter.valid() && frameKey <= epochEnd);
        
        fileIter.next();
        if(fileIter.valid())
            sf = fileIter.current_data();
    }
    while(fileIter.valid() && sf.start_time <= epochEnd);

    auto hbp = r_byte_ptr_rw::append(headerBox, sizeof(uint32_t) + sizeof(uint16_t) + sdp.length());
    uint32_t word = r_networking::r_htonl(numFramesBoxed);
    hbp.write<uint32_t>(word);
    uint16_t sdpLen = sdp.length();
    uint16_t shortVal = r_networking::r_htons(sdpLen);
    hbp.write<uint16_t>(shortVal);
    hbp.bulk_write((uint8_t*)sdp.c_str(), sdp.length());

    r_blob_tree rbt;
    rbt["header_box"] = make_pair(headerBox.size(), &headerBox[0]);
    rbt["index_box"] = make_pair(indexBox.size(), &indexBox[0]);
    rbt["frame_box"] = make_pair(frameBox.size(), &frameBox[0]);

    return rbt.serialize(rbt, 1);
}

string r_storage::sdp_before(const string& indexPath,
                             const string& dataSourceID,
                             const string& type,
                             const system_clock::time_point& time)
{
    r_sqlite_conn dbconn(indexPath);

    auto epochTime = r_time::tp_to_epoch_millis(time);

    auto results = dbconn.exec(r_string_utils::format("SELECT sdp FROM segment_files "
                                                "WHERE start_time <= %s AND data_source_id='%s' AND type='%s' ORDER BY start_time DESC LIMIT 1;",
                                                r_string_utils::uint64_to_s(epochTime).c_str(),
                                                dataSourceID.c_str(),
                                                type.c_str()));
    
    if(results.empty())
        R_STHROW(r_not_found_exception, ("No files found near %s for dataSourceID %s", r_time::tp_to_iso_8601(time, false).c_str(), dataSourceID.c_str()));

    return results.front()["sdp"];
}
