
#include "framework.h"

class test_r_utils_r_server_threaded : public test_fixture
{
public:
    RTF_FIXTURE(test_r_utils_r_server_threaded);
      TEST(test_r_utils_r_server_threaded::test_basic);
    RTF_FIXTURE_END();

    virtual ~test_r_utils_r_server_threaded() throw() {}

    virtual void setup();
    virtual void teardown();

    void test_basic();
};
