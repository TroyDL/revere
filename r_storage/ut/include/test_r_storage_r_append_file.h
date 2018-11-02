
#include "framework.h"

class test_r_storage_r_append_file : public test_fixture
{
public:
    RTF_FIXTURE(test_r_storage_r_append_file);
      TEST(test_r_storage_r_append_file::test_allocate);
      TEST(test_r_storage_r_append_file::test_append_side_effects);
      TEST(test_r_storage_r_append_file::test_boundry_times);
      TEST(test_r_storage_r_append_file::test_iteration);
      TEST(test_r_storage_r_append_file::test_payload);
      TEST(test_r_storage_r_append_file::test_find);
      TEST(test_r_storage_r_append_file::test_file_full);
    RTF_FIXTURE_END();

    virtual ~test_r_storage_r_append_file() throw() {}

    virtual void setup();
    virtual void teardown();

    void test_allocate();
    void test_append_side_effects();
    void test_boundry_times();
    void test_iteration();
    void test_payload();
    void test_find();
    void test_file_full();
};
