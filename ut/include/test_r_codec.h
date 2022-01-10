
#include "framework.h"

class test_r_codec : public test_fixture
{
public:
    RTF_FIXTURE(test_r_codec);
      TEST(test_r_codec::test_basic_decode);
    RTF_FIXTURE_END();

    virtual ~test_r_codec() throw() {}

    virtual void setup();
    virtual void teardown();

    void test_basic_decode();
};
