
#include "r_db/r_sqlite_conn.h"
#include "r_utils/r_exception.h"
#include "r_utils/r_string_utils.h"
#include <unistd.h>

using namespace r_db;
using namespace r_utils;
using namespace std;

static const int DEFAULT_NUM_RETRIES = 60;
static const int BASE_SLEEP_MICROS = 4000;

r_sqlite_conn::r_sqlite_conn(const string& fileName) :
    _db(nullptr)
{
    int numRetries = DEFAULT_NUM_RETRIES;

    while(numRetries > 0)
    {
        int flags = SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE | SQLITE_OPEN_NOMUTEX;

        if(sqlite3_open_v2(fileName.c_str(), &_db, flags, nullptr ) == SQLITE_OK)
        {
            sqlite3_busy_timeout(_db, BASE_SLEEP_MICROS / 1000);
            return;
        }

        if(_db != nullptr)
            _clear();

        usleep(((DEFAULT_NUM_RETRIES-numRetries)+1) * BASE_SLEEP_MICROS);

        --numRetries;
    }

    R_STHROW(r_not_found_exception, ( "Unable to open database connection." ));
}

r_sqlite_conn::r_sqlite_conn(r_sqlite_conn&& obj) noexcept :
    _db(std::move(obj._db))
{
    obj._db = nullptr;
}

r_sqlite_conn::~r_sqlite_conn() noexcept
{
    _clear();
}

r_sqlite_conn& r_sqlite_conn::operator=(r_sqlite_conn&& obj) noexcept
{
    _clear();

    _db = std::move(obj._db);
    obj._db = nullptr;

    return *this;
}

vector<map<string, string>> r_sqlite_conn::exec(const string& query) const
{
    if(!_db)
        R_STHROW(r_internal_exception, ("Cannot exec() on moved out instance of r_sqlite_conn."));

    vector<map<string, string>> results;

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(_db, query.c_str(), query.length(), &stmt, nullptr);
    if(rc != SQLITE_OK)
        R_STHROW(r_internal_exception, ("sqlite3_prepare_v2(%s) failed with: %s", query.c_str(), sqlite3_errmsg(_db)));

    try
    {
        bool done = false;
        while(!done)
        {
            rc = sqlite3_step(stmt);

            if(rc == SQLITE_DONE)
                done = true;
            else if(rc == SQLITE_ROW)
            {
                int columnCount = sqlite3_column_count(stmt);

                map<string, string> row;

                for(int i = 0; i < columnCount; ++i)
                {
                    string val;

                    switch(sqlite3_column_type(stmt, i))
                    {
                        case SQLITE_INTEGER:
                            val = r_string_utils::int64_to_s(sqlite3_column_int64(stmt, i));
                        break;
                        case SQLITE_FLOAT:
                            val = r_string_utils::double_to_s(sqlite3_column_double(stmt, i));
                        break;
                        case SQLITE_TEXT:
                        default:
                        {
                            const char* tp = (const char*)sqlite3_column_text(stmt, i);
                            val = (tp)?string(tp):string();
                        }
                    }
    
                    row[sqlite3_column_name(stmt, i)] = val;
                }

                results.push_back(row);
            }
            else R_STHROW(r_internal_exception, ("Query (%s) to db failed. Cause:", query.c_str(), sqlite3_errmsg(_db)));
        }

        if( stmt )
        {
            sqlite3_clear_bindings(stmt);
            sqlite3_reset(stmt);
            sqlite3_finalize(stmt);
            stmt = nullptr;
        }
    }
    catch(...)
    {
        if(stmt)
        {
            sqlite3_clear_bindings(stmt);
            sqlite3_reset(stmt);
            sqlite3_finalize(stmt);
            stmt = nullptr;
        }

        throw;
    }

    return results;
}

string r_sqlite_conn::last_insert_id() const
{
    if(!_db)
        R_STHROW(r_internal_exception, ("Cannot last_insert_id() on moved out instance of r_sqlite_conn."));

    return r_string_utils::int64_to_s(sqlite3_last_insert_rowid(_db));
}

void r_sqlite_conn::_clear() noexcept
{
    if(_db)
    {
        if(sqlite3_close(_db) != SQLITE_OK)
            R_LOG_ERROR("Unable to close database. Leaking db handle. Cause: %s", sqlite3_errmsg(_db));

        _db = nullptr;
    }
}

int r_sqlite_conn::_sqlite3_busy_handlerS(void* obj, int callCount)
{
    return ((r_sqlite_conn*)obj)->_sqlite3_busy_handler(callCount);
}

int r_sqlite_conn::_sqlite3_busy_handler(int callCount)
{
    if(callCount >= DEFAULT_NUM_RETRIES)
        return 0;

    usleep(callCount * BASE_SLEEP_MICROS);

    return 1;
}
