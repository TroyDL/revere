
#include "framework.h"

class test_r_http_r_client_request : public test_fixture
{
public:

    RTF_FIXTURE(test_r_http_r_client_request);
      TEST(test_r_http_r_client_request::test_default_ctor);
      TEST(test_r_http_r_client_request::test_copy_ctor);
      TEST(test_r_http_r_client_request::test_assignment_op);
      TEST(test_r_http_r_client_request::test_write_request);
    RTF_FIXTURE_END();

    virtual ~test_r_http_r_client_request() throw() {}

    virtual void setup();
    virtual void teardown();

    void test_default_ctor();
    void test_copy_ctor();
    void test_assignment_op();
    void test_write_request();
};
