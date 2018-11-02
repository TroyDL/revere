
#include "framework.h"

class test_r_rtsp_r_sdp : public test_fixture
{
public:

    RTF_FIXTURE(test_r_rtsp_r_sdp);
      TEST(test_r_rtsp_r_sdp::test_parse_generic);
      TEST(test_r_rtsp_r_sdp::test_parse_ffmpeg);
      TEST(test_r_rtsp_r_sdp::test_parse_hikvision_mp4v);
      TEST(test_r_rtsp_r_sdp::test_parse_hikvision_h264);
      TEST(test_r_rtsp_r_sdp::test_parse_axis_h264);
    RTF_FIXTURE_END();

    virtual ~test_r_rtsp_r_sdp() throw() {}

    virtual void setup();
    virtual void teardown();

    void test_parse_generic();
    void test_parse_ffmpeg();
    void test_parse_hikvision_mp4v();
    void test_parse_hikvision_h264();
    void test_parse_axis_h264();
};
