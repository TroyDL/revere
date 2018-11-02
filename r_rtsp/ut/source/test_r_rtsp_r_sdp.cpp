
#include "test_r_rtsp_r_sdp.h"
#include "r_rtsp/r_sdp.h"

using namespace std;
using namespace r_utils;
using namespace r_rtsp;

REGISTER_TEST_FIXTURE(test_r_rtsp_r_sdp);

void test_r_rtsp_r_sdp::setup()
{
}

void test_r_rtsp_r_sdp::teardown()
{
}

void test_r_rtsp_r_sdp::test_parse_generic()
{
    auto desc = parse_sdp("o=- 0 0 IN IP4 127.0.0.1\n"
                            "s=No Name\n"
                            "t=0 0\n"
                            "a=tool:libavformat 57.83.100\n"
                            "m=video 0 RTP/AVP 96\n"
                            "b=AS:1766\n"
                            "a=rtpmap:96 H264/90000\n"
                            "a=fmtp:96 packetization-mode=1; sprop-parameter-sets=Z2QAH6yyAKALdCAAAAMAIAAAAwPB4wZJ,aOvDyyLA; profile-level-id=64001F\n"
                            "a=control:streamid=0\n"
                            "m=audio 49230 RTP/AVP 96 97 98\n"
                            "b=AS:1766\n"
                            "a=rtpmap:96 L8/8000\n"
                            "a=rtpmap:97 L16/8000\n"
                            "a=rtpmap:98 L16/11025/2\n");

    assert(desc.media_descriptions.size() == 2);

    assert(desc.media_descriptions[0].type == "video");
    assert(desc.media_descriptions[0].codec == "H264");
    assert(desc.media_descriptions[0].time_base == 90000);
    assert(desc.media_descriptions[0].encoded_sps == "Z2QAH6yyAKALdCAAAAMAIAAAAwPB4wZJ");
    assert(desc.media_descriptions[0].encoded_pps == "aOvDyyLA");
    assert(desc.media_descriptions[0].control == "streamid=0");

    assert(desc.media_descriptions[1].type == "audio");
    assert(desc.media_descriptions[1].codec == "L8");
    assert(desc.media_descriptions[1].time_base == 8000);
}

void test_r_rtsp_r_sdp::test_parse_ffmpeg()
{
    auto desc = parse_sdp("o=- 0 0 IN IP4 127.0.0.1\n"
                            "s=No Name\n"
                            "t=0 0\n"
                            "a=tool:libavformat 57.83.100\n"
                            "m=video 0 RTP/AVP 96\n"
                            "b=AS:1766\n"
                            "a=rtpmap:96 H264/90000\n"
                            "a=fmtp:96 packetization-mode=1; sprop-parameter-sets=Z2QAH6yyAKALdCAAAAMAIAAAAwPB4wZJ,aOvDyyLA; profile-level-id=64001F\n"
                            "a=control:streamid=42\n");

    assert(desc.media_descriptions.size() == 1);
    assert(desc.media_descriptions[0].type == "video");
    assert(desc.media_descriptions[0].codec == "H264");
    assert(desc.media_descriptions[0].time_base == 90000);
    assert(desc.media_descriptions[0].encoded_sps == "Z2QAH6yyAKALdCAAAAMAIAAAAwPB4wZJ");
    assert(desc.media_descriptions[0].encoded_pps == "aOvDyyLA");
    assert(desc.media_descriptions[0].control == "streamid=42");
}

void test_r_rtsp_r_sdp::test_parse_hikvision_mp4v()
{
    auto desc = parse_sdp("v=0\n"
                            "o=- 1109162014219182 1109162014219192 IN IP4 x.y.z.w\n"
                            "s=Media Presentation\n"
                            "e=NONE\n"
                            "c=IN IP4 0.0.0.0\n"
                            "t=0 0\n"
                            "m=video 0 RTP/AVP 96\n"
                            "a=rtpmap:96 MP4V-ES/90000\n"
                            "a=fmtp:96 profile-level-id=8;config=000001B0F5000001B50900000100000001200886C400670CA0116851;\n"
                            "a=range:ntp=now\n"
                            "a=control:trackID=1\n"
                            "m=audio 0 RTP/AVP 0\n"
                            "a=rtpmap:0 PCMU/8000\n"
                            "a=control:trackID=2\n"
                            "a=Media_header:MEDIAINFO=494D4B48010100000400010010710110401F000000FA000000000000000000000000000000000000;\n"
                            "a=appversion:1.0\n");

    assert(desc.media_descriptions.size() == 2);
    assert(desc.media_descriptions[0].type == "video");
    assert(desc.media_descriptions[0].codec == "MP4V-ES");
    assert(desc.media_descriptions[0].time_base == 90000);
    assert(desc.media_descriptions[0].encoded_sps == "");
    assert(desc.media_descriptions[0].encoded_pps == "");
    assert(desc.media_descriptions[0].control == "trackID=1");
}

void test_r_rtsp_r_sdp::test_parse_hikvision_h264()
{
    auto desc = parse_sdp("v=0\n"
                            "o=- 1109162014219182 1109162014219192 IN IP4 x.y.z.w\n"
                            "s=Media Presentation\n"
                            "e=NONE\n"
                            "c=IN IP4 0.0.0.0\n"
                            "t=0 0\n"
                            "m=video 0 RTP/AVP 96\n"
                            "a=rtpmap:96 H264/90000\n"
                            "a=control:trackID=1\n"
                            "a=fmtp:96;profile-level-id=4D0014;packetization-mode=0;sprop-parameter-sets=Z0LAH4iLUCgC3QgAADhAAAr8gBA=,aM44gA==\n"
                            "m=audio 0 RTP/AVP 0\n"
                            "a=rtpmap:0 PCMU/8000\n"
                            "a=control:trackID=2\n"
                            "a=Media_header:MEDIAINFO=494D4B48010100000400010010710110401F000000FA000000000000000000000000000000000000;\n"
                            "a=appversion:1.0\n");

    assert(desc.media_descriptions.size() == 2);
    assert(desc.media_descriptions[0].type == "video");
    assert(desc.media_descriptions[0].codec == "H264");
    assert(desc.media_descriptions[0].time_base == 90000);
    assert(desc.media_descriptions[0].encoded_sps == "Z0LAH4iLUCgC3QgAADhAAAr8gBA=");
    assert(desc.media_descriptions[0].encoded_pps == "aM44gA==");
    assert(desc.media_descriptions[0].control == "trackID=1");

    assert(desc.media_descriptions[1].type == "audio");
    assert(desc.media_descriptions[1].codec == "PCMU");
    assert(desc.media_descriptions[1].time_base == 8000);
    assert(desc.media_descriptions[1].control == "trackID=2");
}

void test_r_rtsp_r_sdp::test_parse_axis_h264()
{
    auto desc = parse_sdp("v=0\n"
                            "o=- 1216212527554872 1216212527554872 IN IP4 myserver\n"
                            "s=Media Presentation\n"
                            "e=NONE\n"
                            "c=IN IP4 0.0.0.0\n"
                            "b=AS:50064\n"
                            "t=0 0\n"
                            "a=control:rtsp://myserver/axis-media/media.amp?videocodec=h264&resolution=640x480\n"
                            "a=range:npt=0.000000-\n"
                            "m=video 0 RTP/AVP 96\n"
                            "b=AS:50000\n"
                            "a=framerate:30.0\n"
                            "a=transform:1,0,0;0,1,0;0,0,1\n"
                            "a=control:rtsp://myserver/axis-media/media.amp/trackID=1?videocodec=h264&resolution=640x480\n"
                            "a=rtpmap:96 H264/90000\n"
                            "a=fmtp:96 packetization-mode=1; profile-level-id=420029; sprop-parameter-sets=Z0IAKeKQFAe2AtwEBAaQeJEV,aM48gA==\n"
                            "m=audio 0 RTP/AVP 97\n"
                            "b=AS:64\n"
                            "a=control:rtsp://myserver/axis-media/media.amp/trackID=2?videocodec=h264&resolution=640x480\n"
                            "a=rtpmap:97 mpeg4-generic/16000/1\n"
                            "a=fmtp:97 profile-level-id=15; mode=AAC-hbr;config=1408; SizeLength=13; IndexLength=3;IndexDeltaLength=3; Profile=1; bitrate=64000;\n");

    assert(desc.media_descriptions.size() == 2);
    assert(desc.media_descriptions[0].type == "video");
    assert(desc.media_descriptions[0].codec == "H264");
    assert(desc.media_descriptions[0].time_base == 90000);
    assert(desc.media_descriptions[0].encoded_sps == "Z0IAKeKQFAe2AtwEBAaQeJEV");
    assert(desc.media_descriptions[0].encoded_pps == "aM48gA==");
    assert(desc.media_descriptions[0].control == "rtsp://myserver/axis-media/media.amp/trackID=1?videocodec=h264&resolution=640x480");

    assert(desc.media_descriptions[1].type == "audio");
    assert(desc.media_descriptions[1].codec == "mpeg4-generic");
    assert(desc.media_descriptions[1].time_base == 16000);
    assert(desc.media_descriptions[1].control == "rtsp://myserver/axis-media/media.amp/trackID=2?videocodec=h264&resolution=640x480");
}
