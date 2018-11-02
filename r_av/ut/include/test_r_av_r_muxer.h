
#include "framework.h"

class test_r_av_r_muxer : public test_fixture
{
public:

    RTF_FIXTURE(test_r_av_r_muxer);
      TEST(test_r_av_r_muxer::test_recontainerize);
      TEST(test_r_av_r_muxer::test_move);
    RTF_FIXTURE_END();

    virtual ~test_r_av_r_muxer() throw() {}

    virtual void setup();
    virtual void teardown();

    void test_recontainerize();
    void test_move();
};
