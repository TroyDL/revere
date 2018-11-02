
#include "framework.h"

class test_r_http_r_web_server : public test_fixture
{
public:

    RTF_FIXTURE(test_r_http_r_web_server);
      TEST(test_r_http_r_web_server::test_basic);
    RTF_FIXTURE_END();

    virtual ~test_r_http_r_web_server() throw() {}

    virtual void setup();
    virtual void teardown();

    void test_basic();
};
