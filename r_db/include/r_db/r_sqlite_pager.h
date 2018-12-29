
#ifndef _r_db_r_sqlite_pager_h
#define _r_db_r_sqlite_pager_h

#include "r_db/r_sqlite_conn.h"
#include <string>
#include <map>
#include <vector>

namespace r_db
{

class r_sqlite_pager final
{
public:
    r_sqlite_pager(const std::string& columnsToInclude, const std::string& tableName, const std::string& indexColumn, uint32_t requestedPageSize);
    r_sqlite_pager(const r_sqlite_pager& obj) = default;
    r_sqlite_pager(r_sqlite_pager&& obj) noexcept = default;

    ~r_sqlite_pager() noexcept = default;

    r_sqlite_pager& operator=(const r_sqlite_pager& obj) = default;
    r_sqlite_pager& operator=(r_sqlite_pager&& obj) noexcept = default;

    void find(const r_sqlite_conn& conn, const std::string& val = std::string());
    std::map<std::string, std::string> current() const;
    void next(const r_sqlite_conn& conn);
    void prev(const r_sqlite_conn& conn);
    bool valid() const;
    void end(const r_sqlite_conn& conn);

private:
    std::string _columnsToInclude;
    std::string _tableName;
    std::string _indexColumn;
    uint32_t _requestedPageSize;
    uint32_t _index;
    std::vector<std::map<std::string, std::string>> _results;
};

}

#endif
