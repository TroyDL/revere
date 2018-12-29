
#ifndef r_storage_r_file_index_h
#define r_storage_r_file_index_h

#include "r_storage/r_segment_file.h"
#include "r_utils/r_exception.h"
#include "r_utils/r_file_lock.h"
#include "r_utils/r_file.h"
#include "r_utils/r_nullable.h"
#include "r_utils/r_string_utils.h"
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
            _pager.find(_conn, r_utils::r_string_utils::uint64_to_s(key));
            _move_to_ds_id(true);
        }

        void begin()
        {
            _pager.find(_conn);
            _move_to_ds_id(true);
        }

        void end()
        {
            _pager.end(_conn);
            _move_to_ds_id(false);
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
            sf.start_time = r_utils::r_string_utils::s_to_uint64(row["start_time"]);
            sf.end_time = r_utils::r_string_utils::s_to_uint64(row["end_time"]);
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

    template<typename CB>
    segment_file recycle_append(uint64_t startTime, const std::string& dataSourceID, const std::string& type, const std::string& sdp, CB cb)
    {
        r_db::r_sqlite_conn conn(_indexPath, true);

        segment_file sf;

        r_db::r_sqlite_transaction(conn, [&](const r_db::r_sqlite_conn& conn){
            auto results = conn.exec("SELECT * FROM segment_files ORDER BY start_time ASC LIMIT 1;");
            if(results.empty())
                R_STHROW(r_utils::r_not_found_exception, ("Empty segment_files?"));

            conn.exec(r_utils::r_string_utils::format("UPDATE segment_files SET "
                                    "valid=1, "
                                    "start_time=%s, "
                                    "end_time=0, "
                                    "data_source_id='%s', "
                                    "type='%s', "
                                    "sdp='%s' "
                                    "WHERE id='%s';",
                                    r_utils::r_string_utils::uint64_to_s(startTime).c_str(),
                                    dataSourceID.c_str(),
                                    type.c_str(),
                                    sdp.c_str(),
                                    results.front()["id"].c_str()));

            sf.id = results.front()["id"];
            sf.valid = true;
            sf.path = results.front()["path"];
            sf.start_time = startTime;
            sf.end_time = 0;
            sf.data_source_id = dataSourceID;
            sf.type = type;

            cb(sf);
        });

        return sf;
    }

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
