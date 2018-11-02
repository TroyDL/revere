
#include "framework.h"

class test_r_utils_r_time_utils : public test_fixture
{
public:

    RTF_FIXTURE(test_r_utils_r_time_utils);
      TEST(test_r_utils_r_time_utils::test_8601_to_tp);
      TEST(test_r_utils_r_time_utils::test_tp_to_8601);
    RTF_FIXTURE_END();

    virtual ~test_r_utils_r_time_utils() throw() {}

    virtual void setup();
    virtual void teardown();

    void test_8601_to_tp();
    void test_tp_to_8601();
};
