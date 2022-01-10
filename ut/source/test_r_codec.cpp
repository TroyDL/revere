
#include "test_r_codec.h"
#include "r_codec/r_video_decoder.h"
#include "r_mux/r_demuxer.h"
#include "r_utils/r_file.h"

// Added to the global namespace by test_r_mux.cpp, so extern'd here:
extern unsigned char true_north_mp4[];
extern unsigned int true_north_mp4_len;
extern unsigned char bad_guy_mp4[];
extern unsigned int bad_guy_mp4_len;

using namespace std;
using namespace std::chrono;
using namespace r_utils;
using namespace r_mux;
using namespace r_codec;

REGISTER_TEST_FIXTURE(test_r_codec);

void test_r_codec::setup()
{
    r_fs::write_file(true_north_mp4, true_north_mp4_len, "true_north.mp4");
    r_fs::write_file(bad_guy_mp4, bad_guy_mp4_len, "bad_guy.mp4");
}

void test_r_codec::teardown()
{
    r_fs::remove_file("true_north.mp4");
    r_fs::remove_file("bad_guy.mp4");
}

void test_r_codec::test_basic_decode()
{
    r_demuxer demuxer("true_north.mp4", true);
    auto video_stream_index = demuxer.get_video_stream_index();
    auto vsi = demuxer.get_stream_info(video_stream_index);

    r_video_decoder decoder(vsi.codec_id);

    while(demuxer.read_frame())
    {
        auto fi = demuxer.get_frame_info();

        if(fi.index == video_stream_index)
        {
            decoder.attach_buffer(fi.data, fi.size);

            r_video_decoder_state decode_state = R_VIDEO_DECODER_STATE_INITIALIZED;

            while(decode_state != R_VIDEO_DECODER_STATE_HUNGRY)
            {
                decode_state = decoder.decode();

                if(decode_state == R_VIDEO_DECODER_STATE_HAS_OUTPUT)
                {
                    auto frame = decoder.get(AV_PIX_FMT_ARGB, 640, 480);
                    printf("frame.size()=%lu\n",frame.size());
                }
            }
        }
    }
}
