
#include "framework.h"

class test_r_rtsp_r_client_connection : public test_fixture
{
public:

    RTF_FIXTURE(test_r_rtsp_r_client_connection);
      TEST(test_r_rtsp_r_client_connection::test_connect);
    RTF_FIXTURE_END();

    virtual ~test_r_rtsp_r_client_connection() throw() {}

    virtual void setup();
    virtual void teardown();

    void test_connect();
};
