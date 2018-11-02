
#include "test_r_av_r_muxer.h"
#include "r_av/r_options.h"
#include "r_av/r_muxer.h"
#include "r_av/r_demuxer.h"
#include "r_av/r_locky.h"

#include "gop.cpp"

using namespace std;
using namespace r_utils;
using namespace r_av;

REGISTER_TEST_FIXTURE(test_r_av_r_muxer);

void test_r_av_r_muxer::setup()
{
    r_locky::register_ffmpeg();

    {
        r_muxer m(r_muxer::OUTPUT_LOCATION_FILE, "a.mp4");

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
        for( int i = 0; i < NUM_FRAMES_IN_GOP; i++ )
        {
            int index = i % NUM_FRAMES_IN_GOP;
            r_packet pkt(gop[index].frame, gop[index].frameSize, false);
            pkt.set_pts(ts);
            pkt.set_dts(ts);
            pkt.set_duration(3000);
            ts += 3000; // 90000 / 30 fps
            m.write_packet( pkt, streamIndex, ((i % 15) == 0) ? true : false );
        }

        m.finalize_file();
    }

    {
        r_muxer m(r_muxer::OUTPUT_LOCATION_FILE, "b.mp4");

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
        for( int i = 0; i < (NUM_FRAMES_IN_GOP * 10); i++ )
        {
            int index = i % NUM_FRAMES_IN_GOP;
            r_packet pkt( gop[index].frame, gop[index].frameSize, false );
            pkt.set_pts(ts);
            pkt.set_dts(ts);
            pkt.set_duration(3000);
            ts += 3000; // 90000 / 30 fps
            m.write_packet( pkt, streamIndex, ((i % 15) == 0) ? true : false );
        }

        m.finalize_file();
    }
}

void test_r_av_r_muxer::teardown()
{
    remove("a.mp4");
    remove("b.mp4");
    r_locky::unregister_ffmpeg();
}

void test_r_av_r_muxer::test_recontainerize()
{
    r_muxer c(r_muxer::OUTPUT_LOCATION_FILE, "bar.mp4");

    r_stream_options soptions;
    soptions.type = "video";
    soptions.codec_id = r_av_codec_id_h264;
    soptions.format = r_av_pix_fmt_yuv420p;
    soptions.width = 1280;
    soptions.height = 720;
    soptions.time_base_num = 1;
    soptions.time_base_den = 90000;
    soptions.frame_rate_num = 30;
    soptions.frame_rate_den = 1;

    auto streamIndex = c.add_stream(soptions);

    int64_t ts = 0;
    for( int i = 0; i < NUM_FRAMES_IN_GOP; i++ )
    {
        int index = i % NUM_FRAMES_IN_GOP;
        r_packet pkt(gop[index].frame, gop[index].frameSize, false);
        pkt.set_pts(ts);
        pkt.set_dts(ts);
        pkt.set_duration(3000);
        ts += 3000;
        c.write_packet( pkt, streamIndex, ((i % 15) == 0) ? true : false );
    }

    c.finalize_file();

    r_demuxer deMuxer( "bar.mp4" );

    int videoStreamIndex = deMuxer.get_video_stream_index();

    int i = 0;
    streamIndex = 0;
    while( deMuxer.read_frame( streamIndex ) )
    {
        RTF_ASSERT( streamIndex == videoStreamIndex );

        if((i % 15) == 0)
            RTF_ASSERT(deMuxer.is_key() == true);
        else RTF_ASSERT(deMuxer.is_key() == false);

        ++i;
    }

    remove("bar.mp4");
}

void test_r_av_r_muxer::test_move()
{
    // test move assignment
    {
        r_muxer dmA(r_muxer::OUTPUT_LOCATION_FILE, "a.mp4");

        {
            r_muxer dmB(r_muxer::OUTPUT_LOCATION_FILE, "b.mp4");
            dmA = std::move(dmB);
        }
    }

    // test move ctor
    {
        r_muxer dmA(r_muxer::OUTPUT_LOCATION_FILE, "a.mp4");
        r_muxer dmB = std::move(dmA);
    }
}
