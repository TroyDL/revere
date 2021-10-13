
#include "framework.h"

class test_r_disco : public test_fixture
{
public:
    RTF_FIXTURE(test_r_disco);
      TEST(test_r_disco::test_r_disco_r_manual_provider);
      TEST(test_r_disco::test_r_disco_r_agent_start_stop);
      TEST(test_r_disco::test_r_disco_r_devices_basic);
    RTF_FIXTURE_END();

    virtual ~test_r_disco() throw() {}

    virtual void setup();
    virtual void teardown();

    void test_r_disco_r_manual_provider();
    void test_r_disco_r_agent_start_stop();
    void test_r_disco_r_devices_basic();
};
