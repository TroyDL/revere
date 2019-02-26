
#include "r_storage/r_append_file.h"
#include "r_utils/r_file.h"
#include "r_utils/r_socket.h"
#include "r_utils/r_uuid.h"

using namespace r_storage;
using namespace r_utils;
using namespace std;

// file format {
//     header {
//         uint8_t[4]                 magic number {'B', 'L', 'O', 'B'}
//         uint16_t                   version
//         uint64_t                   max_entries
//         uint64_t                   time_of_first_blob
//         uint8_t[16]                ds_id
//         uint8_t[90]                reserved
//     }
//     uint64_t                       num_valid
//     index[max_entries] {
//         uint64_t time_ofs          time offset in millis from header.time_of_first_blob
//         uint64_t data_ofs          offset in data section of blob
//     }
//     data[] {
//         uint64_t                   time_ofs
//         uint32_t                   flags
//         uint32_t                   payload_len
//         uint8_t[payload_len]       payload
//     }
// }
//

const uint32_t r_storage::FLAG_KEY = 0x80000000;

bool r_storage::operator < (struct index const& lhs, struct index const& rhs)
{
    uint64_t lhsEntryRecordTime = r_networking::r_ntohll(lhs.key);
    uint64_t rhsEntryRecordTime = r_networking::r_ntohll(rhs.key);
    return lhsEntryRecordTime < rhsEntryRecordTime;
}

bool r_storage::operator < (struct index const& entry, uint64_t const key)
{
    uint64_t entryRecordTime = r_networking::r_ntohll(entry.key);
    return entryRecordTime < key;
}

bool r_storage::operator < (uint64_t const key, struct index const& entry)
{
    uint64_t entryRecordTime = r_networking::r_ntohll(entry.key);
    return key < entryRecordTime;
}

r_append_file::r_append_file(const string& filePath) :
    _file(),
    _map()
{
    if(!r_fs::file_exists(filePath))
        R_THROW(("Asked to read non existant file."));

    if(!r_fs::is_reg(filePath))
        R_THROW(("Asked to read irregular file."));

    r_fs::r_file_info fileInfo;
    if(r_fs::stat(filePath, &fileInfo) < 0)
        R_THROW(("Unable to stat: %s", filePath.c_str()));

    _file = r_file::open(filePath, "r+b");

    _map = r_memory_map(fileno(_file),
                        0,
                        fileInfo.file_size,
                        r_memory_map::mm_prot_read | r_memory_map::mm_prot_write,
                        r_memory_map::mm_type_file | r_memory_map::mm_shared);
}

r_append_file::r_append_file(r_append_file&& obj) noexcept :
    _file(std::move(obj._file)),
    _map(std::move(obj._map)),
    _currentIndex(std::move(obj._currentIndex))
{
}

r_append_file::~r_append_file() throw()
{
}

r_append_file& r_append_file::operator=(r_append_file&& obj) noexcept
{
    _file = std::move(obj._file);
    _map = std::move(obj._map);
    _currentIndex = std::move(obj._currentIndex);

    return *this;
}

void r_append_file::allocate(const string& filePath,
                             size_t fileAllocationSize,
                             uint64_t numIndexes,
                             const string& dsid)
{
    if(r_fs::file_exists(filePath))
        R_THROW(("Cannot allocate, file file_exists: %s", filePath.c_str()));

    r_file outFile = r_file::open(filePath, "w+b");

    if(r_fs::fallocate(outFile, fileAllocationSize) < 0)
    {
        outFile.close();
        R_THROW(("Unable to pre allocate file."));
    }

    {
        auto map = r_memory_map(fileno(outFile),
                                0,
                                fileAllocationSize,
                                r_memory_map::mm_prot_read | r_memory_map::mm_prot_write,
                                r_memory_map::mm_type_file | r_memory_map::mm_shared);

        auto p = map.map();

        // write our header

        // magic bits
        uint8_t magic[] = { 'B', 'L', 'O', 'B' };
        p.bulk_write(magic, 4);

        // version
        uint16_t version = 1;
        uint16_t shortv = r_networking::r_htons(version);
        p.write<uint16_t>(shortv);

        // max entries
        uint64_t doubleWord = r_networking::r_htonll(numIndexes);
        p.write<uint64_t>(doubleWord);

        // time of first blob (0 since nothing inserted yet)
        doubleWord = 0;
        p.write<uint64_t>(doubleWord);

        // uuid of device
        r_uuid::s_to_uuid(dsid, p.get_ptr());
        p += 16;

        // with new fields, should be 90 padding bytes
        p += 90; // skip the padding at the end of our header...

        // num valid
        doubleWord = 0;
        p.write<uint64_t>(doubleWord); // start with an empty file...
    }
}

void r_append_file::reset(const string& filePath, const string& dsid)
{
    if(!r_fs::file_exists(filePath))
        R_THROW(("Cannot reset non existant file: %s", filePath.c_str()));

    r_fs::r_file_info fileInfo;
    if(r_fs::stat(filePath, &fileInfo) < 0)
        R_THROW(("Unable to stat: %s", filePath.c_str()));

    auto outFile = r_file::open(filePath, "r+b");

    auto map = r_memory_map(fileno(outFile),
                            0,
                            fileInfo.file_size,
                            r_memory_map::mm_prot_read | r_memory_map::mm_prot_write,
                            r_memory_map::mm_type_file | r_memory_map::mm_shared);

    auto p = map.map();

    p += 14;

    // new time of first...
    uint64_t doubleWord = 0;
    p.write<uint64_t>(doubleWord);

    // uuid of device
    r_uuid::s_to_uuid(dsid, p.get_ptr());
    p += 16;

    p += 90; // skip the padding at the end of our header...

    doubleWord = 0;
    p.write<uint64_t>(doubleWord);
}

void r_append_file::append(uint64_t key, uint32_t flags, const uint8_t* pkt, uint32_t size)
{
    uint16_t version = 0;
    uint64_t maxEntries = 0;
    uint64_t timeOfFirst = 0;
    _read_header(version, maxEntries, timeOfFirst);

    uint64_t numIndexes = 0;
    _read_num_indexes(numIndexes);
    if(numIndexes >= maxEntries)
        R_THROW(("Out of indexes."));

    // initially, newFramePos is set to the position of the first frame in the file...
    uint64_t newFramePos = 128 + 8 + (maxEntries * 16);

    // If we have inserted something in this file, we need place the new frame after the last one...
    if(numIndexes > 0)
    {
        uint64_t prevTimeOfs = 0;
        uint64_t prevFrameOfs = 0;
        _read_index((numIndexes - 1), prevTimeOfs, prevFrameOfs);

        uint32_t flags = 0;
        uint32_t payloadLen = 0;
        _read_frame_md(prevFrameOfs, prevTimeOfs, flags, payloadLen);

        newFramePos = prevFrameOfs + 16 + payloadLen;
    }

    auto p = _map.map();

    p += newFramePos;

    if(p.remaining() < (16 + size))
        R_THROW(("Insufficient data space in file."));

    // If this is the first frame in the file, we need to update the first time field in the header...
    if(timeOfFirst == 0)
    {
        _write_first_time(key);
        timeOfFirst = key;        // this should cause the delta of the first frame to be 0
    }

    // write our new frame md...

    uint64_t timeOfs = key - timeOfFirst;
    uint64_t doubleWord = r_networking::r_htonll(timeOfs);
    p.write<uint64_t>(doubleWord);

    // flags
    uint32_t word = r_networking::r_htonl(flags);
    p.write<uint32_t>(word);

    // payload len
    word = r_networking::r_htonl(size);
    p.write<uint32_t>(word);

    // write our new frame data..

    p.bulk_write(pkt, size);

    // update our index...

    _write_index(numIndexes, timeOfs, newFramePos);

    FULL_MEM_BARRIER();

    // publish our frame...
    _write_num_indexes(numIndexes + 1);
}

bool r_append_file::fits(uint32_t size, uint32_t numFrames)
{
    uint16_t version = 0;
    uint64_t maxEntries = 0;
    uint64_t timeOfFirst = 0;
    _read_header(version, maxEntries, timeOfFirst);

    uint64_t numIndexes = 0;
    _read_num_indexes(numIndexes);
    if(numIndexes >= maxEntries || ((numIndexes + numFrames) > maxEntries))
        return false;

    // initially, newFramePos is set to the position of the first frame in the file...
    uint64_t newFramePos = 128 + 8 + (maxEntries * 16);

    // If we have inserted something in this file, we need place the new frame after the last one...
    if(numIndexes > 0)
    {
        uint64_t prevTimeOfs = 0;
        uint64_t prevFrameOfs = 0;
        _read_index((numIndexes - 1), prevTimeOfs, prevFrameOfs);

        uint32_t flags;
        uint32_t payloadLen = 0;
        _read_frame_md(prevFrameOfs, prevTimeOfs, flags, payloadLen);

        newFramePos = prevFrameOfs + 16 + payloadLen;
    }

    auto p = _map.map();

    p += newFramePos;

    if(p.remaining() < ((16 * numFrames) + size))
        return false;

    return true;
}

uint64_t r_append_file::first_key() const
{
    uint64_t numIndexes = 0;
    _read_num_indexes(numIndexes);

    if(numIndexes == 0)
        R_THROW(("Empty file!"));

    uint16_t version = 0;
    uint64_t maxEntries = 0;
    uint64_t timeOfFirst = 0;
    _read_header(version, maxEntries, timeOfFirst);

    return timeOfFirst;
}

uint64_t r_append_file::last_key() const
{
    uint16_t version = 0;
    uint64_t maxEntries = 0;
    uint64_t timeOfFirst = 0;
    _read_header(version, maxEntries, timeOfFirst);

    uint64_t numIndexes = 0;
    _read_num_indexes(numIndexes);

    uint64_t timeOfs = 0;
    uint64_t frameOfs = 0;
    _read_index(numIndexes - 1, timeOfs, frameOfs);

    return timeOfFirst + timeOfs;
}

void r_append_file::_read_header(uint16_t& version, uint64_t& maxEntries, uint64_t& timeOfFirst) const
{
    auto p = _map.map();

    uint8_t magic[4];
    memcpy(magic, p.get_ptr(), 4);

    if(magic[0] != 'B' || magic[1] != 'L' || magic[2] != 'O' || magic[3] != 'B')
        R_STHROW(r_internal_exception, ("Corrupt file!"));

    p += 4;

    uint16_t sh = p.consume<uint16_t>();
    version = r_networking::r_ntohs(sh);

    uint64_t doubleWord = p.consume<uint64_t>();
    maxEntries = r_networking::r_ntohll(doubleWord);

    doubleWord = p.consume<uint64_t>();
    timeOfFirst = r_networking::r_ntohll(doubleWord);
}

void r_append_file::_write_first_time(uint64_t timeOfFirst)
{
    auto p = _map.map();

    p += 14;

    uint64_t doubleWord = r_networking::r_htonll(timeOfFirst);

    p.write<uint64_t>(doubleWord);
}

void r_append_file::_read_num_indexes(uint64_t& numIndexes) const
{
    auto p = _map.map();

    p += 128;

    uint64_t doubleWord = p.consume<uint64_t>();
    numIndexes = r_networking::r_ntohll(doubleWord);
}

void r_append_file::_write_num_indexes(uint64_t numIndexes)
{
    auto p = _map.map();

    p += 128;

    uint64_t doubleWord = r_networking::r_htonll(numIndexes);
    p.write<uint64_t>(doubleWord);
}

void r_append_file::_read_index(uint64_t idx, uint64_t& timeOfs, uint64_t& frameOfs) const
{
    auto p = _map.map();

    p += 128;

    uint64_t doubleWord = p.consume<uint64_t>();
    uint64_t numIndexes = r_networking::r_ntohll(doubleWord);

    if(idx >= numIndexes)
        R_STHROW(r_internal_exception, ("Invalid index."));

    p += (idx * 16);

    doubleWord = p.consume<uint64_t>();
    timeOfs = r_networking::r_ntohll(doubleWord);

    doubleWord = p.consume<uint64_t>();
    frameOfs = r_networking::r_ntohll(doubleWord);
}

void r_append_file::_write_index(uint64_t idx, uint64_t timeOfs, uint64_t frameOfs)
{
    auto p = _map.map();

    // skip header, index count and part of index...
    p += 136 + (idx * 16);

    uint64_t doubleWord = r_networking::r_htonll(timeOfs);
    p.write<uint64_t>(doubleWord);

    doubleWord = r_networking::r_htonll(frameOfs);
    p.write<uint64_t>(doubleWord);
}

void r_append_file::_read_frame_md(uint64_t ofs, uint64_t& timeOfs, uint32_t& flags, uint32_t& payloadLen) const
{
    auto p = _map.map();

    p += ofs;

    uint64_t doubleWord = p.consume<uint64_t>();
    timeOfs = r_networking::r_ntohll(doubleWord);

    uint32_t word = p.consume<uint32_t>();
    flags = r_networking::r_ntohl(word);

    word = p.consume<uint32_t>();
    payloadLen = r_networking::r_ntohl(word);
}
