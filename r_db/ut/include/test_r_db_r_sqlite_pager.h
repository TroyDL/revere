
#include "framework.h"

class test_r_db_r_sqlite_pager : public test_fixture
{
public:

    RTF_FIXTURE(test_r_db_r_sqlite_pager);
      TEST(test_r_db_r_sqlite_pager::test_next);
      TEST(test_r_db_r_sqlite_pager::test_find);
      TEST(test_r_db_r_sqlite_pager::test_prev);
    RTF_FIXTURE_END();

    virtual ~test_r_db_r_sqlite_pager() throw() {}

    virtual void setup();
    virtual void teardown();

    void test_next();
    void test_find();
    void test_prev();
};
