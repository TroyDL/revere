
#include "r_storage/r_file_index.h"
#include "r_storage/r_append_file.h"
#include "r_db/r_sqlite_pager.h"
#include "r_utils/r_string_utils.h"
#include <chrono>

using namespace r_storage;
using namespace r_utils;
using namespace r_db;
using namespace std;
using namespace std::chrono;

r_file_index::r_file_index(const string& indexPath) :
    _indexPath(indexPath)
{
    _upgrade_db(_indexPath);

    _fix_end_times();
}

r_file_index::~r_file_index() noexcept
{
}

void r_file_index::allocate(const string& indexPath)
{
    _upgrade_db(indexPath);
}

void r_file_index::create_invalid_segment_file(r_sqlite_conn& conn, const std::string& path, const std::string& dataSourceID)
{
    // Note: this method doesnt need a transaction because it is called from r_storage_engine which takes care of transactions.
    uint64_t oldestStart = _oldest_start_time(conn);
    conn.exec(r_string_utils::format("INSERT INTO segment_files(valid, "
                                                        "path, "
                                                        "start_time, "
                                                        "end_time, "
                                                        "data_source_id, "
                                                        "type, "
                                                        "sdp) "
                                                        "VALUES(0, '%s', %s, %s, '%s', 'none', '');",
                                                        path.c_str(),
                                                        r_string_utils::uint64_to_s(oldestStart - 2000).c_str(),
                                                        r_string_utils::uint64_to_s(oldestStart - 1000).c_str(),
                                                        dataSourceID.c_str()));
}

void r_file_index::update_end_time(segment_file& sf, uint64_t endTime)
{
    r_sqlite_conn conn(_indexPath, true);

    conn.exec(r_string_utils::format("UPDATE segment_files SET "
                               "end_time=%s "
                               "WHERE id='%s';",
                               r_string_utils::uint64_to_s(endTime).c_str(),
                               sf.id.c_str()));

    sf.end_time = endTime;
}

void r_file_index::free(uint64_t keyA, uint64_t keyB, const std::string& type)
{
    r_sqlite_conn conn(_indexPath, true);

    r_sqlite_transaction(conn, [&](const r_sqlite_conn& conn){
        r_sqlite_pager pager("*", "segment_files", "start_time", 5);
        pager.find(conn, r_string_utils::uint64_to_s(keyA));
        if(pager.valid())
        {
            auto row = pager.current();

            auto oldestStart = _oldest_start_time(conn);

            do
            {
                if(row["type"] == type)
                {
                    oldestStart -= 2000;
                    conn.exec(r_string_utils::format("UPDATE segment_files SET "
                                            "valid=0, "
                                            "start_time=%s, "
                                            "end_time=%s, "
                                            "type='none' "
                                            "WHERE id='%s';",
                                            r_string_utils::uint64_to_s(oldestStart).c_str(),
                                            r_string_utils::uint64_to_s(oldestStart+1000).c_str(),
                                            row["id"].c_str()));
                }

                pager.next(conn);
                row = pager.current();
            }
            while(pager.valid() && (r_string_utils::s_to_uint64(row["start_time"]) < keyB));
        }
    });
}

uint64_t r_file_index::_oldest_start_time(const r_sqlite_conn& conn) const
{
    auto result = conn.exec("SELECT * FROM segment_files ORDER BY start_time ASC LIMIT 1;");

    if(result.empty())
        return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    else return r_string_utils::s_to_uint64(result.front()["start_time"]);
}

void r_file_index::_fix_end_times() const
{
    r_sqlite_conn conn(_indexPath, true);

    r_sqlite_transaction(conn, [&](const r_sqlite_conn& conn){

        auto results = conn.exec("SELECT * FROM segment_files WHERE end_time=0;");

        for(auto row : results)
        {
            r_append_file f(row["path"]);

            conn.exec(r_string_utils::format("UPDATE segment_files SET end_time=%s WHERE id=%s;",
                                    r_string_utils::uint64_to_s(f.last_key()).c_str(),
                                    row["id"].c_str()));
        }
    });
}

void r_file_index::_upgrade_db(const std::string& indexPath)
{
    r_sqlite_conn conn(indexPath, true);

    auto results = conn.exec("PRAGMA user_version;");

    uint16_t version = 0;

    if(!results.empty())
        version = r_string_utils::s_to_uint16(results.front()["user_version"]);

    switch( version )
    {

    case 0:
    {
        R_LOG_NOTICE("upgrade index to version: 1");
        conn.exec("CREATE TABLE IF NOT EXISTS segment_files(id INTEGER PRIMARY KEY AUTOINCREMENT, valid INTEGER, path TEXT, start_time INTEGER, end_time INTEGER, data_source_id TEXT, type TEXT, sdp TEXT);");
        conn.exec("CREATE INDEX IF NOT EXISTS segment_files_start_time_idx ON segment_files(start_time);");
        conn.exec("CREATE INDEX IF NOT EXISTS segment_files_end_time_idx ON segment_files(end_time);");
        conn.exec("PRAGMA user_version=1;");
    }

    default:
        break;
    }
}
