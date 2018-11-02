
#include "framework.h"

class test_r_http_r_utils : public test_fixture
{
public:

    RTF_FIXTURE(test_r_http_r_utils);
      TEST(test_r_http_r_utils::test_basic);
    RTF_FIXTURE_END();

    virtual ~test_r_http_r_utils() throw() {}

    virtual void setup();
    virtual void teardown();

    void test_basic();
};
