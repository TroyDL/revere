
#include "framework.h"

class test_r_db_r_sqlite_conn : public test_fixture
{
public:

    RTF_FIXTURE(test_r_db_r_sqlite_conn);
      TEST(test_r_db_r_sqlite_conn::test_ctor);
      TEST(test_r_db_r_sqlite_conn::test_insert);
      TEST(test_r_db_r_sqlite_conn::test_paging);
    RTF_FIXTURE_END();

    virtual ~test_r_db_r_sqlite_conn() throw() {}

    virtual void setup();
    virtual void teardown();

    void test_ctor();
    void test_insert();
    void test_paging();
};
