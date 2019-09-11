
#include "framework.h"
#include "r_av/r_packet.h"

class test_r_av_r_video_encoder : public test_fixture
{
public:

    RTF_FIXTURE(test_r_av_r_video_encoder);
      TEST(test_r_av_r_video_encoder::test_constructor);
      TEST(test_r_av_r_video_encoder::test_encode_key);
      TEST(test_r_av_r_video_encoder::test_encode_gop);
      TEST(test_r_av_r_video_encoder::test_encode_jpeg);
    RTF_FIXTURE_END();

    virtual ~test_r_av_r_video_encoder() throw() {}

    virtual void setup();
    virtual void teardown();

    void test_constructor();
    void test_encode_key();
    void test_encode_gop();
    void test_encode_jpeg();
    //void test_encode_many_jpeg();

private:
    r_av::r_packet _pic;
};
