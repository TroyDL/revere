
#include "framework.h"

class test_r_storage_r_file_index : public test_fixture
{
public:
    RTF_FIXTURE(test_r_storage_r_file_index);
      TEST(test_r_storage_r_file_index::test_allocate);
      TEST(test_r_storage_r_file_index::test_create_invalid);
      TEST(test_r_storage_r_file_index::test_recycle_append);
      TEST(test_r_storage_r_file_index::test_update_end_time);
      TEST(test_r_storage_r_file_index::test_free);
      TEST(test_r_storage_r_file_index::test_basic_iteration);
    RTF_FIXTURE_END();

    virtual ~test_r_storage_r_file_index() throw() {}

    virtual void setup();
    virtual void teardown();

    void test_allocate();
    void test_create_invalid();
    void test_recycle_append();
    void test_update_end_time();
    void test_free();
    void test_basic_iteration();
};
