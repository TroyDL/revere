
#include "framework.h"

class test_r_rtsp_r_rtsp_server : public test_fixture
{
public:

    RTF_FIXTURE(test_r_rtsp_r_rtsp_server);
      TEST(test_r_rtsp_r_rtsp_server::test_constructor);
      TEST(test_r_rtsp_r_rtsp_server::test_start_stop);
      TEST(test_r_rtsp_r_rtsp_server::test_options);
      TEST(test_r_rtsp_r_rtsp_server::test_describe);
    RTF_FIXTURE_END();

    virtual ~test_r_rtsp_r_rtsp_server() throw() {}

    virtual void setup();
    virtual void teardown();

    void test_constructor();
    void test_start_stop();
    void test_options();
    void test_describe();
};
