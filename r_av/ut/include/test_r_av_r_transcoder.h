
#include "framework.h"

class test_r_av_r_transcoder : public test_fixture
{
public:

    RTF_FIXTURE(test_r_av_r_transcoder);
      TEST(test_r_av_r_transcoder::test_constructor);
      TEST(test_r_av_r_transcoder::test_transcode_basic);
    RTF_FIXTURE_END();

    virtual ~test_r_av_r_transcoder() throw() {}

    virtual void setup();
    virtual void teardown();

    void test_constructor();
    void test_transcode_basic();

private:
};
