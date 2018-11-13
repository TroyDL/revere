
#include "test_r_av_r_transcoder.h"
#include "r_av/r_locky.h"
#include "r_av/r_options.h"
#include "r_av/r_video_encoder.h"
#include "r_av/r_video_decoder.h"
#include "r_av/r_transcoder.h"

#include "gop.cpp"
#include "pic.cpp"

using namespace std;
using namespace r_utils;
using namespace r_av;

REGISTER_TEST_FIXTURE(test_r_av_r_transcoder);

void test_r_av_r_transcoder::setup()
{
    r_locky::register_ffmpeg();

    {
        r_muxer m(r_muxer::OUTPUT_LOCATION_FILE, "big.mp4");

        r_stream_options soptions;
        soptions.type = "video";
        soptions.codec_id = r_av_codec_id_h264;
        soptions.format = r_av_pix_fmt_yuv420p;
        soptions.width = 1280;
        soptions.height = 720;
        soptions.time_base_num = 1;
        soptions.time_base_den = 90000;
        soptions.frame_rate_num = 15;
        soptions.frame_rate_den = 1;

        auto streamIndex = m.add_stream(soptions);

        int64_t ts = 0;
        for( int i = 0; i < NUM_FRAMES_IN_GOP * 10; i++ )
        {
            int index = i % NUM_FRAMES_IN_GOP;
            r_packet pkt(gop[index].frame, gop[index].frameSize, false);
            pkt.set_pts(ts);
            pkt.set_dts(ts);
            pkt.set_duration(3000);
            ts += 3000; // 90000 / 30 fps
            m.write_packet(pkt, streamIndex, ((i % 15) == 0)?true:false);
        }

        m.finalize_file();
    }
}

void test_r_av_r_transcoder::teardown()
{
    r_locky::unregister_ffmpeg();

    //remove("big.mp4");
}

void test_r_av_r_transcoder::test_constructor()
{
    r_transcoder transcoder(1, 30, 1, 15, 1.0);
}

void test_r_av_r_transcoder::test_transcode_basic()
{
    r_demuxer dm("big.mp4");
    r_video_decoder decoder(r_av_codec_id_h264, r_av_pix_fmt_yuv420p, get_decoder_options(1));
    r_video_encoder encoder(r_av_codec_id_h264, r_av_pix_fmt_yuv420p, 640, 360, 15, 1000000, 1, 15, get_encoder_options("baseline", "ultrafast", "zerolatency", 2));
    r_muxer m(r_muxer::OUTPUT_LOCATION_FILE, "small.mp4");

    r_stream_options soptions;
    soptions.type = "video";
    soptions.codec_id = r_av_codec_id_h264;
    soptions.format = r_av_pix_fmt_yuv420p;
    soptions.width = 640;
    soptions.height = 360;
    soptions.time_base_num = 1;
    soptions.time_base_den = 90000;
    soptions.frame_rate_num = 15;
    soptions.frame_rate_den = 1;

    auto streamIndex = m.add_stream(soptions);

    m.set_extradata(encoder.get_extradata(), streamIndex);

    //auto stats = r_demuxer::get_video_stream_statistics("big.mp4");

    //r_transcoder transcoder(stats.timeBaseNum.value(), stats.timeBaseDen.value(), 1, 15, 1.0);
    r_transcoder transcoder(1, 15, 1, 15, 1.0);

    decoder.set_output_width(640);
    decoder.set_output_height(360);

    int dmIndex = dm.get_video_stream_index();

    while(!dm.end_of_file())
    {
        if(transcoder.decode(dm, dmIndex, decoder))
        {
            transcoder.encode_yuv420p_and_mux(encoder, m, streamIndex, decoder.get());
        }
    }

    m.finalize_file();
}
