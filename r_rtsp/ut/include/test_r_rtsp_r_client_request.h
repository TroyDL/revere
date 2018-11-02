
#include "framework.h"

class test_r_rtsp_r_client_request : public test_fixture
{
public:

    RTF_FIXTURE(test_r_rtsp_r_client_request);
      TEST(test_r_rtsp_r_client_request::test_constructor);
      TEST(test_r_rtsp_r_client_request::test_assignment_op);
      TEST(test_r_rtsp_r_client_request::test_set_method);
      TEST(test_r_rtsp_r_client_request::test_set_header);
      TEST(test_r_rtsp_r_client_request::test_set_user_agent);
      TEST(test_r_rtsp_r_client_request::test_set_server_port);
      TEST(test_r_rtsp_r_client_request::test_write_request);
    RTF_FIXTURE_END();

    virtual ~test_r_rtsp_r_client_request() throw() {}

    virtual void setup();
    virtual void teardown();

    void test_constructor();
    void test_assignment_op();
    void test_set_method();
    void test_set_header();
    void test_set_user_agent();
    void test_set_server_port();
    void test_write_request();
};
