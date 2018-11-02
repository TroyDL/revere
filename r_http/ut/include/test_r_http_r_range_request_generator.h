
#include "framework.h"

class test_r_http_r_range_request_generator : public test_fixture
{
public:
    RTF_FIXTURE(test_r_http_r_range_request_generator);
      TEST(test_r_http_r_range_request_generator::test_basic);
      TEST(test_r_http_r_range_request_generator::test_even);
    RTF_FIXTURE_END();

    virtual ~test_r_http_r_range_request_generator() throw() {}

    virtual void setup();
    virtual void teardown();

    void test_basic();
    void test_even();
};
