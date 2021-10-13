
#ifndef _r_db_r_sqlite_conn_h
#define _r_db_r_sqlite_conn_h

#include "sqlite3/sqlite3.h"
#include "r_utils/r_exception.h"
#include "r_utils/r_nullable.h"
#include "r_utils/r_logger.h"
#include <string>
#include <vector>
#include <map>

namespace r_db
{

r_utils::r_nullable<std::string> to_scalar(const std::vector<std::map<std::string, r_utils::r_nullable<std::string>>>& row);

class r_sqlite_conn final
{
public:
    r_sqlite_conn(const std::string& fileName, bool rw = true);
    r_sqlite_conn(const r_sqlite_conn&) = delete;
    r_sqlite_conn(r_sqlite_conn&& obj) noexcept;

    ~r_sqlite_conn() noexcept;

    r_sqlite_conn& operator=(const r_sqlite_conn&) = delete;
    r_sqlite_conn& operator=(r_sqlite_conn&&) noexcept;

    std::vector<std::map<std::string, r_utils::r_nullable<std::string>>> exec(const std::string& query) const;

    std::string last_insert_id() const;

private:
    void _clear() noexcept;

    sqlite3* _db;
    bool _rw;
};

template<typename T>
void r_sqlite_transaction(const r_sqlite_conn& db, T t)
{
    db.exec("BEGIN");
    try
    {
        t(db);
        db.exec("COMMIT");
    }
    catch(const r_utils::r_exception& ex)
    {
        printf("TRANS ROLLBACK! exception: %s\n", ex.what());
        fflush(stdout);
        R_LOG_ERROR("TRANS ROLLBACK! exception: %s\n", ex.what());
        db.exec("ROLLBACK");
    }
    catch(...)
    {
        printf("TRANS ROLLBACK!\n");
        fflush(stdout);
        R_LOG_ERROR("TRANS ROLLBACK!\n");
        db.exec("ROLLBACK");
        throw;
    }
}

}

#endif
