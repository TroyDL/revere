
#ifndef r_storage_r_file_index_h
#define r_storage_r_file_index_h

#include "r_storage/r_segment_file.h"
#include "r_utils/r_exception.h"
#include "r_utils/r_file_lock.h"
#include "r_utils/r_file.h"
#include "r_utils/r_nullable.h"
#include "r_utils/r_string.h"
#include "r_db/r_sqlite_conn.h"
#include "r_db/r_sqlite_pager.h"
#include <functional>
#include <vector>

class test_r_storage_r_file_index;

namespace r_storage
{

class r_file_index final
{
    friend class ::test_r_storage_r_file_index;

public:

    class iterator final
    {
    public:
        iterator(r_file_index* db, const std::string& indexColumn, const std::string& dataSourceID, const std::string& type) :
            _db(db),
            _indexColumn(indexColumn),
            _dataSourceID(dataSourceID),
            _type(type),
            _conn(_db->get_index_path()),
            _pager("*", "segment_files", _indexColumn, 10)
        {
        }

        iterator(const iterator&) = delete;

        iterator(iterator&&) noexcept = default;

        ~iterator() noexcept = default;

        iterator& operator=(const iterator&) = delete;
        iterator& operator=(iterator&&) noexcept = default;

        void find(uint64_t key)
        {
            _pager.find(_conn, r_utils::r_string::uint64_to_s(key));
            _move_to_ds_id(true);
        }

        void begin()
        {
            _pager.find(_conn);
            _move_to_ds_id(true);
        }

        void end()
        {
            R_STHROW(r_utils::r_not_implemented_exception, ("r_file_index::end() not yet implemented."));
        }

        void next()
        {
            _pager.next(_conn);
            _move_to_ds_id(true);
        }

        void prev()
        {
            _pager.prev(_conn);
            _move_to_ds_id(false);
        }
        
        bool valid() const
        {
            return _pager.valid();
        }

        segment_file current_data() const
        {
            auto row = _pager.current();

            segment_file sf;
            sf.id = row["id"];
            sf.valid = (row["valid"]=="1")?true:false;
            sf.path = row["path"];
            sf.start_time = r_utils::r_string::s_to_uint64(row["start_time"]);
            sf.end_time = r_utils::r_string::s_to_uint64(row["end_time"]);
            sf.data_source_id = row["data_source_id"];
            sf.sdp = row["sdp"];

            return sf;
        }

    private:
        void _move_to_ds_id(bool forward)
        {
            bool done = false;
            while(!done && _pager.valid())
            {
                auto row = _pager.current();

                if(row["data_source_id"] == _dataSourceID && row["type"] == _type)
                {
                    done = true;
                    continue;
                }

                if(forward)
                    _pager.next(_conn);
                else _pager.prev(_conn);
            }
        }

        r_file_index* _db;
        std::string _indexColumn;
        std::string _dataSourceID;
        std::string _type;
        r_db::r_sqlite_conn _conn;
        r_db::r_sqlite_pager _pager;
    };

    r_file_index(const std::string& indexPath);
    r_file_index(const r_file_index&) = delete;
    r_file_index(r_file_index&&) = delete;

    ~r_file_index() noexcept;

    r_file_index& operator=(const r_file_index&) = delete;
    r_file_index& operator=(r_file_index&&) = delete;

    static void allocate(const std::string& indexPath);

    void create_invalid_segment_file(r_db::r_sqlite_conn& conn, const std::string& path, const std::string& dataSourceID);

    segment_file recycle_append(uint64_t startTime, const std::string& dataSourceID, const std::string& type, const std::string& sdp);

    void update_end_time(segment_file& sf, uint64_t endTime);

    void free(uint64_t keyA, uint64_t keyB, const std::string& type);

    std::string get_index_path() const { return _indexPath; }

    inline iterator get_iterator(const std::string& indexColumn, const std::string& dsID, const std::string& type) { return iterator(this, indexColumn, dsID, type); }

private:
    uint64_t _oldest_start_time(const r_db::r_sqlite_conn& conn) const;
    void _fix_end_times() const;
    static void _upgrade_db(const std::string& indexPath);

    std::string _indexPath;
};

}

#endif
