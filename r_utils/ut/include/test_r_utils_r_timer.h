
#include "framework.h"

class test_r_utils_r_timer : public test_fixture
{
public:

    RTF_FIXTURE(test_r_utils_r_timer);
      TEST(test_r_utils_r_timer::test_repeating);
    RTF_FIXTURE_END();

    virtual ~test_r_utils_r_timer() throw() {}

    virtual void setup();
    virtual void teardown();

    void test_repeating();
};
