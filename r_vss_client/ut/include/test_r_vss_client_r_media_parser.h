
#include "framework.h"

class test_r_vss_client_r_media_parser : public test_fixture
{
public:

    RTF_FIXTURE(test_r_vss_client_r_media_parser);
      TEST(test_r_vss_client_r_media_parser::test_basic);
    RTF_FIXTURE_END();

    virtual ~test_r_vss_client_r_media_parser() throw() {}

    virtual void setup();
    virtual void teardown();

    void test_basic();
};
