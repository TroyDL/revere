
#include "framework.h"
#include <stdio.h>

class test_r_utils_r_file_lock : public test_fixture
{
public:

    RTF_FIXTURE(test_r_utils_r_file_lock);
      TEST(test_r_utils_r_file_lock::test_constructor);
      TEST(test_r_utils_r_file_lock::test_exclusive);
      TEST(test_r_utils_r_file_lock::test_shared);
      TEST(test_r_utils_r_file_lock::test_combo);
    RTF_FIXTURE_END();

    virtual ~test_r_utils_r_file_lock() throw() {}

    virtual void setup();
    virtual void teardown();

    void test_constructor();
    void test_exclusive();
    void test_shared();
    void test_combo();

    FILE* lockFile;
};
