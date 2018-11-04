
#ifndef __r_storage_r_append_file_h
#define __r_storage_r_append_file_h

#include "r_utils/r_string_utils.h"
#include "r_utils/r_memory_map.h"
#include "r_utils/r_socket.h"
#include "r_utils/r_file.h"
#include <memory>
#include <stdlib.h>
#include <algorithm>

class test_r_storage_r_append_file;

namespace r_storage
{

extern const uint32_t FLAG_KEY;

struct index
{
    uint64_t key;
    uint64_t offset;
};

bool operator < (struct index const& lhs, struct index const& rhs);
bool operator < (struct index const& entry, uint64_t const key);
bool operator < (uint64_t const key, struct index const& entry);

class r_append_file final
{
    friend class ::test_r_storage_r_append_file;

public:
    class iterator final
    {
    public:
        iterator(const r_append_file* db) :
            _idx(0),
            _idxSet(false),
            _db(db)
        {
        }

        iterator(const iterator& obj) :
            _idx(obj._idx),
            _idxSet(obj._idxSet),
            _db(obj._db)
        {
        }

        iterator(iterator&& obj) noexcept :
            _idx(std::move(obj._idx)),
            _idxSet(std::move(obj._idxSet)),
            _db(std::move(obj._db))
        {
            obj._idx = 0;
            obj._idxSet = false;
        }

        ~iterator() noexcept = default;

        iterator& operator=(const iterator& obj)
        {
            _idx = obj._idx;
            _idxSet = obj._idxSet;
            _db = obj._db;
            return *this;
        }

        iterator& operator=(iterator&& obj) noexcept
        {
            _idx = std::move(obj._idx);
            obj._idx = 0;
            _idxSet = std::move(obj._idxSet);
            obj._idxSet = false;
            _db = std::move(obj._db);
            return *this;
        }

        void find(uint64_t key)
        {
            uint16_t version = 0;
            uint64_t maxEntries = 0;
            uint64_t timeOfFirst = 0;
            _db->_read_header(version, maxEntries, timeOfFirst);

            r_utils::r_byte_ptr_rw p = _db->_map.map();

            p += 128;

            uint64_t doubleWord = p.consume<uint64_t>();
            uint64_t numIndexes = r_utils::r_networking::r_ntohll(doubleWord);

            struct index* start = (struct index*)p.get_ptr();
            struct index* end = (struct index*)(p.get_ptr() + (numIndexes * 16));

            struct index* found = std::lower_bound(start, end, (key - timeOfFirst));

            _idxSet = (found != end);
            _idx = found - start;

        }

        void begin() { _idx = 0; _idxSet = true; }

        void end()
        {
            uint64_t numIndexes = 0;
            _db->_read_num_indexes(numIndexes);
            _idx = numIndexes;
            _idxSet = true;
        }

        bool next()
        {
            if(!_idxSet)
                R_STHROW(r_utils::r_not_found_exception, ("Please initialize iterator to valid location."));

            uint64_t numIndexes = 0;
            _db->_read_num_indexes(numIndexes);

            if(_idx >= numIndexes)
                return false;

            _idx++;

            return true;
        }

        bool prev()
        {
            if(!_idxSet)
                R_STHROW(r_utils::r_not_found_exception, ("Please initialize iterator to valid location."));

            if(_idx <= 0)
                return false;

            _idx--;

            return true;
        }

        bool valid()
        {
            if(!_idxSet)
                R_STHROW(r_utils::r_not_found_exception, ("Please initialize iterator to valid location."));

            uint64_t numIndexes = 0;
            _db->_read_num_indexes(numIndexes);

            if(numIndexes > 0)
            {
                if((_idx >= 0) && (_idx < numIndexes))
                    return true;
            }

            return false;
        }

        void current_data(uint64_t& key, uint32_t& flags, uint32_t& payloadLen, const uint8_t*& payload)
        {
            if(!_idxSet)
                R_STHROW(r_utils::r_not_found_exception, ("Please initialize iterator to valid location."));

            uint64_t timeOfs = 0;
            uint64_t frameOfs = 0;
            _db->_read_index(_idx, timeOfs, frameOfs);

            _db->_read_frame_md(frameOfs, timeOfs, flags, payloadLen);

            uint16_t version = 0;
            uint64_t maxEntries = 0;
            uint64_t timeOfFirst = 0;
            _db->_read_header(version, maxEntries, timeOfFirst);

            key = timeOfFirst + timeOfs;

            r_utils::r_byte_ptr_rw p = _db->_map.map();

            p += (frameOfs + 16);

            payload = p.get_ptr();
        }

    private:
        uint64_t _idx;
        bool _idxSet;
        const r_append_file* _db;
    };

    r_append_file(const std::string& filePath);
    r_append_file(const r_append_file&) = delete;
    r_append_file(r_append_file&& obj) noexcept;

    ~r_append_file() noexcept;

    r_append_file& operator=(const r_append_file&) = delete;
    r_append_file& operator=(r_append_file&& obj) noexcept;

    static void allocate(const std::string& filePath,
                         size_t fileAllocationSize,
                         uint64_t numIndexes,
                         const std::string& dsid);
                                 
    static void reset(const std::string& filePath, const std::string& dsid);

    void append(uint64_t key, uint32_t flags, const uint8_t* pkt, uint32_t size);

    bool fits(uint32_t size, uint32_t numFrames = 1);

    uint64_t first_key() const;
    uint64_t last_key() const;

    iterator get_iterator() const { return iterator(this); }

private:
    void _read_header(uint16_t& version, uint64_t& maxEntries, uint64_t& timeOfFirst) const;
    void _write_first_time(uint64_t timeOfFirst);
    void _read_num_indexes(uint64_t& numIndexes) const;
    void _write_num_indexes(uint64_t numIndexes);
    void _read_index(uint64_t idx, uint64_t& timeOfs, uint64_t& frameOfs) const;
    void _write_index(uint64_t idx, uint64_t timeOfs, uint64_t frameOfs);
    void _read_frame_md(uint64_t ofs, uint64_t& timeOfs, uint32_t& flags, uint32_t& payloadLen) const;

    r_utils::r_file _file;
    r_utils::r_memory_map _map;
    int _currentIndex;
};

}

#endif
