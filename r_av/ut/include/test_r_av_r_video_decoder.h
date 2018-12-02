
#include "framework.h"

class test_r_av_r_video_decoder : public test_fixture
{
public:

    RTF_FIXTURE(test_r_av_r_video_decoder);
      TEST(test_r_av_r_video_decoder::test_constructor);
      TEST(test_r_av_r_video_decoder::test_input_dimensions);
      TEST(test_r_av_r_video_decoder::test_output_dimensions);
    RTF_FIXTURE_END();

    virtual ~test_r_av_r_video_decoder() throw() {}

    virtual void setup();
    virtual void teardown();

    void test_constructor();
    void test_input_dimensions();
    void test_output_dimensions();
    void test_decode_gop();
};
