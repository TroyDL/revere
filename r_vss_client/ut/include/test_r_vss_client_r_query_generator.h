
#include "framework.h"

class _test_r_vss_client_r_query_generator : public test_fixture
{
public:

    RTF_FIXTURE(_test_r_vss_client_r_query_generator);
      TEST(_test_r_vss_client_r_query_generator::test_basic);
    RTF_FIXTURE_END();

    virtual ~_test_r_vss_client_r_query_generator() throw() {}

    virtual void setup();
    virtual void teardown();

    void test_basic();
};
