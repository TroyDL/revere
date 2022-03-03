
#include "r_storage/r_storage_file_reader.h"
#include "r_storage/r_rel_block.h"
#include "r_utils/r_file.h"
#include "r_utils/r_exception.h"
#include "r_utils/r_blob_tree.h"
#include <stdexcept>
#include <cmath>
#include <memory>
#include <algorithm>

using namespace r_utils;
using namespace r_storage;
using namespace std;

r_storage_file_reader::r_storage_file_reader(const string& file_name) :
    _file(r_file::open(file_name, "r")),
    _file_lock(fileno(_file)),
    _h(_read_header(file_name)),
    _dumbdex_map(_map_block(0)),
    _block_index(file_name, (uint8_t*)_dumbdex_map->map() + r_storage_file::R_STORAGE_FILE_HEADER_SIZE, _h.num_blocks)
{
}

r_storage_file_reader::~r_storage_file_reader() noexcept
{
}

vector<uint8_t> r_storage_file_reader::query(r_storage_media_type media_type, int64_t start_ts, int64_t end_ts)
{
    r_file_lock_guard g(_file_lock, false);

    r_blob_tree bt;

    bool has_audio = false;

    size_t fi = 0;

    r_nullable<string> video_codec_name;
    r_nullable<string> video_codec_parameters;
    r_nullable<string> audio_codec_name;
    r_nullable<string> audio_codec_parameters;

    _visit_ind_blocks(
        media_type,
        start_ts,
        end_ts,
        [&](r_ind_block_info& ibii) {

            if(ibii.stream_id == R_STORAGE_MEDIA_TYPE_AUDIO)
            {
                has_audio = true;
                if(audio_codec_name.is_null())
                    audio_codec_name.set_value(ibii.audio_codec_name);
                if(audio_codec_parameters.is_null())
                    audio_codec_parameters.set_value(ibii.audio_codec_parameters);
            }
            else if(ibii.stream_id == R_STORAGE_MEDIA_TYPE_VIDEO)
            {
                if(video_codec_name.is_null())
                    video_codec_name.set_value(ibii.video_codec_name);
                if(video_codec_parameters.is_null())
                    video_codec_parameters.set_value(ibii.video_codec_parameters);
            }

            r_rel_block rel_block(ibii.block, ibii.block_size);

            auto rbi = rel_block.begin();

            while(rbi.valid())
            {
                auto frame_info = *rbi;

                vector<uint8_t> frame_buffer(frame_info.size);
                ::memcpy(frame_buffer.data(), frame_info.data, frame_info.size);

                bt["frames"][fi]["ind_block_ts"] = r_string_utils::int64_to_s(ibii.ts);
                bt["frames"][fi]["data"] = frame_buffer;
                bt["frames"][fi]["ts"] = r_string_utils::int64_to_s(frame_info.ts);
                bt["frames"][fi]["key"] = (frame_info.flags>0)?string("true"):string("false");
                bt["frames"][fi]["stream_id"] = r_string_utils::uint8_to_s(ibii.stream_id);

                rbi.next();
                ++fi;
            }
        }
    );

    bt["video_codec_name"] = video_codec_name.value();
    bt["video_codec_parameters"] = video_codec_parameters.value();
    bt["audio_codec_name"] = (has_audio)?audio_codec_name.value():string("");
    bt["audio_codec_parameters"] = (has_audio)?audio_codec_parameters.value():string("");

    bt["has_audio"] = (has_audio)?string("true"):string("false");

    return r_blob_tree::serialize(bt, 1);
}

vector<uint8_t> r_storage_file_reader::query_key(r_storage_media_type media_type, int64_t ts)
{
    r_file_lock_guard g(_file_lock, false);

    r_blob_tree bt;

    r_nullable<string> video_codec_name;
    r_nullable<string> video_codec_parameters;
    r_nullable<string> audio_codec_name;
    r_nullable<string> audio_codec_parameters;

    _visit_ind_block(
        media_type,
        ts,
        [&](r_ind_block_info& ibii) {

            if(ibii.stream_id == R_STORAGE_MEDIA_TYPE_AUDIO)
            {
                if(audio_codec_name.is_null())
                    audio_codec_name.set_value(ibii.audio_codec_name);
                if(audio_codec_parameters.is_null())
                    audio_codec_parameters.set_value(ibii.audio_codec_parameters);
            }
            else if(ibii.stream_id == R_STORAGE_MEDIA_TYPE_VIDEO)
            {
                if(video_codec_name.is_null())
                    video_codec_name.set_value(ibii.video_codec_name);
                if(video_codec_parameters.is_null())
                    video_codec_parameters.set_value(ibii.video_codec_parameters);
            }

            r_rel_block rel_block(ibii.block, ibii.block_size);

            auto rbi = rel_block.begin();

            if(rbi.valid())
            {
                auto frame_info = *rbi;

                vector<uint8_t> frame_buffer(frame_info.size);
                ::memcpy(frame_buffer.data(), frame_info.data, frame_info.size);

                bt["frames"][0]["ind_block_ts"] = r_string_utils::int64_to_s(ibii.ts);
                bt["frames"][0]["data"] = frame_buffer;
                bt["frames"][0]["ts"] = r_string_utils::int64_to_s(frame_info.ts);
                bt["frames"][0]["key"] = (frame_info.flags>0)?string("true"):string("false");
                bt["frames"][0]["stream_id"] = r_string_utils::uint8_to_s(ibii.stream_id);
            }
        }
    );

    bt["video_codec_name"] = video_codec_name.value();
    bt["video_codec_parameters"] = video_codec_parameters.value();
    bt["audio_codec_name"] = (!audio_codec_name.is_null())?audio_codec_name.value():string("");
    bt["audio_codec_parameters"] = (!audio_codec_parameters.is_null())?audio_codec_parameters.value():string("");

    return r_blob_tree::serialize(bt, 1);
}

vector<int64_t> r_storage_file_reader::key_frame_start_times(r_storage_media_type media_type, int64_t start_ts, int64_t end_ts)
{
    r_file_lock_guard g(_file_lock, false);

    vector<int64_t> gop_start_times;

    _visit_ind_blocks(
        media_type,
        start_ts,
        end_ts,
        [&](r_ind_block_info& ibii) {
            gop_start_times.push_back(ibii.ts);
        }
    );

    return gop_start_times;
}

r_storage_file::_storage_file_header r_storage_file_reader::_read_header(const std::string& file_name)
{
    auto tmp_f = r_file::open(file_name, "r+");
    r_storage_file::_storage_file_header h;
    h.num_blocks = 0;
    fread(&h.num_blocks, 1, sizeof(uint32_t), tmp_f);
    h.block_size = 0;
    fread(&h.block_size, 1, sizeof(uint32_t), tmp_f);

    return h;
}

template<typename CB>
void r_storage_file_reader::_visit_ind_blocks(r_storage_media_type media_type, int64_t start_ts, int64_t end_ts, CB cb)
{
    if(media_type >= R_STORAGE_MEDIA_TYPE_MAX)
        R_THROW(("Invalid storage media type."));

    int64_t current_ts = start_ts;

    // 1) By the nature of lower_bound() unless we have an exact match we're going to need to move back one
    // to find the actual query start (general case) (because the blocks are sorted by the ts of the first
    // frame in them)
    // 2) If there is only 1 block, most likely find will return end() (unless there is an exact match).
    // 3) If we move back a block because find returned end, then we don't want to also move back a block for
    // the general case.

    auto fi = _block_index.find_lower_bound(current_ts);

    bool moved = false;
    if(fi == _block_index.end())
        moved = fi.prev();
    
    if(!fi.valid())
        R_THROW(("Unable to locate query start."));

    auto dumbdex_index_entry = *fi;

    if(!moved)
    {
        if(dumbdex_index_entry.first != current_ts)
        {
            if(fi.prev())
                dumbdex_index_entry = *fi;
        }
    }

    auto ind_block_map = _map_block(dumbdex_index_entry.second);
    auto ind_block = make_shared<r_ind_block>((uint8_t*)ind_block_map->map(), _h.block_size);

    auto ibi = ind_block->find_lower_bound(current_ts);

    bool done = false;
    while(!done && current_ts < end_ts && fi.valid())
    {
        if(ibi.valid())
        {
            auto ibii = *ibi;

            current_ts = ibii.ts;

            if(current_ts < end_ts)
            {
                if(ibii.stream_id == media_type || media_type == R_STORAGE_MEDIA_TYPE_ALL)
                    cb(ibii);
            }

            ibi.next();
        }
        else
        {
            fi.next();
            if(!fi.valid())
                done = true;
            else
            {
                dumbdex_index_entry = *fi;
                ind_block_map = _map_block(dumbdex_index_entry.second);
                ind_block = make_shared<r_ind_block>((uint8_t*)ind_block_map->map(), _h.block_size);
                ibi = ind_block->begin();
            }
        }
    }
}

template<typename CB>
void r_storage_file_reader::_visit_ind_block(r_storage_media_type media_type, int64_t ts, CB cb)
{
    if(media_type >= R_STORAGE_MEDIA_TYPE_MAX)
        R_THROW(("Invalid storage media type."));

    int64_t current_ts = ts;

    auto fi = _block_index.find_lower_bound(current_ts);

    bool moved = false;
    if(fi == _block_index.end())
        moved = fi.prev();
    
    if(!fi.valid())
        R_THROW(("Unable to locate query start."));

    auto dumbdex_index_entry = *fi;

    if(!moved)
    {
        if(dumbdex_index_entry.first != current_ts)
        {
            if(fi.prev())
                dumbdex_index_entry = *fi;
        }
    }

    auto ind_block_map = _map_block(dumbdex_index_entry.second);
    auto ind_block = make_shared<r_ind_block>((uint8_t*)ind_block_map->map(), _h.block_size);

    auto ibi = ind_block->find_lower_bound(current_ts);

    if(ibi.valid())
    {
        auto ibii = *ibi;
        cb(ibii);
    }
}

shared_ptr<r_memory_map> r_storage_file_reader::_map_block(uint16_t block)
{
    if(block > _h.num_blocks)
        R_THROW(("Invalid block index."));

    return make_shared<r_memory_map>(
#ifdef IS_WINDOWS
        _fileno(_file),
#endif
#ifdef IS_LINUX
        fileno(_file),
#endif
        ((int64_t)block) * ((int64_t)_h.block_size),
        _h.block_size,
        r_memory_map::RMM_PROT_READ,
        r_memory_map::RMM_TYPE_FILE | r_memory_map::RMM_SHARED
    );
}
