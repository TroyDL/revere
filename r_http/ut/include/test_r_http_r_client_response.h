
#include "framework.h"

class test_r_http_r_client_response : public test_fixture
{
public:
    RTF_FIXTURE(test_r_http_r_client_response);
      TEST(test_r_http_r_client_response::test_default_constructor);
      TEST(test_r_http_r_client_response::test_copy_constructor);
      TEST(test_r_http_r_client_response::test_assignment_operator);
      TEST(test_r_http_r_client_response::test_receive_response);
      TEST(test_r_http_r_client_response::test_streaming);
      TEST(test_r_http_r_client_response::test_100_continue);
      TEST(test_r_http_r_client_response::test_interrupted_streaming);
      TEST(test_r_http_r_client_response::test_multi_part);
      TEST(test_r_http_r_client_response::TestColonsInHeaders);
      TEST(test_r_http_r_client_response::TestMultipleHeadersWithSameKey);
      TEST(test_r_http_r_client_response::TestSpaceAtHeaderStart);
      TEST(test_r_http_r_client_response::TestTabAtHeaderStart);
      TEST(test_r_http_r_client_response::TestSpaceAtHeaderLine);
      TEST(test_r_http_r_client_response::TestTabAtHeaderLine);
      TEST(test_r_http_r_client_response::TestMultipleKeys);

    RTF_FIXTURE_END();

    virtual ~test_r_http_r_client_response() throw() {}

    virtual void setup();
    virtual void teardown();

    void test_default_constructor();
    void test_copy_constructor();
    void test_assignment_operator();
    void test_receive_response();
    void test_streaming();
    void test_100_continue();
    void test_interrupted_streaming();
    void test_multi_part();
    void TestColonsInHeaders();
    void TestMultipleHeadersWithSameKey();
    void TestSpaceAtHeaderStart();
    void TestTabAtHeaderStart();
    void TestSpaceAtHeaderLine();
    void TestTabAtHeaderLine();
    void TestMultipleKeys();
};
