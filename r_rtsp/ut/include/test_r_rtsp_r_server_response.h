
#include "framework.h"

class test_r_rtsp_r_server_response : public test_fixture
{
public:

    RTF_FIXTURE(test_r_rtsp_r_server_response);
      TEST(test_r_rtsp_r_server_response::test_constructor);
      TEST(test_r_rtsp_r_server_response::test_assignment_op);
      TEST(test_r_rtsp_r_server_response::test_set_status);
      TEST(test_r_rtsp_r_server_response::test_set_header);
      TEST(test_r_rtsp_r_server_response::test_set_body);
      TEST(test_r_rtsp_r_server_response::test_write_response);
    RTF_FIXTURE_END();

    virtual ~test_r_rtsp_r_server_response() throw() {}

    virtual void setup();
    virtual void teardown();

    void test_constructor();
    void test_assignment_op();
    void test_set_status();
    void test_set_header();
    void test_set_body();
    void test_write_response();
};
