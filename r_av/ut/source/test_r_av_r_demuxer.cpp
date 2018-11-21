
#include "test_r_av_r_demuxer.h"
#include "r_av/r_demuxer.h"
#include "r_av/r_muxer.h"
#include "r_av/r_locky.h"
#include "r_av/r_options.h"
#include "r_utils/r_file.h"

#include "gop.cpp"
#include "this_world_is_a_lie.cpp"

using namespace std;
using namespace r_utils;
using namespace r_av;

REGISTER_TEST_FIXTURE(test_r_av_r_demuxer);

void test_r_av_r_demuxer::setup()
{
    r_locky::register_ffmpeg();

    {
        r_muxer m(r_muxer::OUTPUT_LOCATION_FILE, "AVDeMuxerTest.mp4" );

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
            pkt.set_time_base({1, 90000});
            pkt.set_pts(ts);
            pkt.set_dts(ts);
            pkt.set_duration(3000);
            ts += 3000; // 90000 / 30 fps
            m.write_packet( pkt, streamIndex, ((i % 15) == 0) ? true : false );
        }

        m.finalize_file();
    }

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
        for( int i = 0; i < (NUM_FRAMES_IN_GOP * 10); i++ )
        {
            int index = i % NUM_FRAMES_IN_GOP;
            r_packet pkt( gop[index].frame, gop[index].frameSize, false );
            pkt.set_time_base({1, 90000});
            pkt.set_pts(ts);
            pkt.set_dts(ts);
            pkt.set_duration(3000);
            ts += 3000; // 90000 / 30 fps
            m.write_packet( pkt, streamIndex, ((i % 15) == 0) ? true : false );
        }

        m.finalize_file();
    }

    r_utils::r_fs::write_file(this_world_is_a_lie_mkv, this_world_is_a_lie_mkv_len, "this_world_is_a_lie.mkv");
}

void test_r_av_r_demuxer::teardown()
{
    r_locky::unregister_ffmpeg();

    remove("big.mp4");
    remove("AVDeMuxerTest.mp4");
    remove("this_world_is_a_lie.mkv");
}

void test_r_av_r_demuxer::test_constructor()
{
    r_demuxer dm("AVDeMuxerTest.mp4");
}

void test_r_av_r_demuxer::test_examine_file()
{
    r_demuxer deMuxer( "AVDeMuxerTest.mp4" );

    int videoStreamIndex = deMuxer.get_video_stream_index();

    auto frameRate = deMuxer.get_frame_rate(videoStreamIndex);
    RTF_ASSERT(frameRate.first == 30 && frameRate.second == 1);
    RTF_ASSERT(deMuxer.get_stream_codec_id(videoStreamIndex) == r_av_codec_id_h264);
    RTF_ASSERT(deMuxer.get_pix_format(videoStreamIndex) == r_av_pix_fmt_yuv420p);
    auto res = deMuxer.get_resolution(videoStreamIndex);
    RTF_ASSERT(res.first == 1280);
    RTF_ASSERT(res.second == 720);
    auto ed = deMuxer.get_extradata(videoStreamIndex);
    RTF_ASSERT(ed.size() > 0);

    int i = 0;
    int streamIndex = 0;
    while( deMuxer.read_frame( streamIndex ) )
    {
        RTF_ASSERT( streamIndex == videoStreamIndex );

        int index = i % NUM_FRAMES_IN_GOP;

        bool keyness = ((i % NUM_FRAMES_IN_GOP) == 0) ? true : false;

        RTF_ASSERT( deMuxer.is_key() == keyness );

        auto frame = deMuxer.get();

        RTF_ASSERT( frame.get_data_size() == gop[index].frameSize );
        RTF_ASSERT( memcmp(frame.map(), gop[index].frame, frame.get_data_size()) == 0 );

        i++;
    }

    RTF_ASSERT( deMuxer.end_of_file() );
}

void test_r_av_r_demuxer::test_file_from_memory()
{
    auto buffer = r_fs::read_file("AVDeMuxerTest.mp4");
    r_demuxer deMuxer( &buffer[0], buffer.size() );

    int videoStreamIndex = deMuxer.get_video_stream_index();

    int i = 0;
    int streamIndex = 0;
    while( deMuxer.read_frame( streamIndex ) )
    {
        RTF_ASSERT( streamIndex == videoStreamIndex );

        int index = i % NUM_FRAMES_IN_GOP;

        bool keyness = ((i % NUM_FRAMES_IN_GOP) == 0) ? true : false;

        RTF_ASSERT( deMuxer.is_key() == keyness );

        auto frame = deMuxer.get();

        RTF_ASSERT( frame.get_data_size() == gop[index].frameSize );
        RTF_ASSERT( memcmp(frame.map(), gop[index].frame, frame.get_data_size()) == 0 );

        i++;
    }

    RTF_ASSERT( deMuxer.end_of_file() );
}

void test_r_av_r_demuxer::test_get_container_statistics()
{
    auto stats = r_demuxer::get_video_stream_statistics( "big.mp4" );

    RTF_ASSERT( !stats.averageBitRate.is_null() );
    // Is the averageBitRate in the right ballpark?
    RTF_ASSERT( (stats.averageBitRate.value() > 2500000) && (stats.averageBitRate.value() < 3000000) );

    RTF_ASSERT( !stats.frameRate.is_null() );
    RTF_ASSERT( stats.frameRate.value() == 30 );

    RTF_ASSERT( !stats.gopSize.is_null() );
    RTF_ASSERT( stats.gopSize.value() == 15 );
}

void test_r_av_r_demuxer::test_multi_stream_mp4()
{
    r_demuxer deMuxer("this_world_is_a_lie.mkv");

    int videoStreamIndex = deMuxer.get_video_stream_index();

    RTF_ASSERT(deMuxer.get_stream_codec_id(videoStreamIndex) == r_av_codec_id_h264);
    RTF_ASSERT(deMuxer.get_pix_format(videoStreamIndex) == r_av_pix_fmt_yuv420p);
    auto res = deMuxer.get_resolution(videoStreamIndex);
    RTF_ASSERT(res.first == 640);
    RTF_ASSERT(res.second == 360);
    auto ed = deMuxer.get_extradata(videoStreamIndex);
    RTF_ASSERT(ed.size() > 0);
    auto frameRate = deMuxer.get_frame_rate(videoStreamIndex);
    RTF_ASSERT(frameRate.first == 24000 && frameRate.second == 1001);
    auto timeBase = deMuxer.get_time_base(videoStreamIndex);

    int audioStreamIndex = deMuxer.get_primary_audio_stream_index();
    RTF_ASSERT(audioStreamIndex >= 0 && audioStreamIndex != videoStreamIndex);

    int streamIndex = 0;
    while(deMuxer.read_frame(streamIndex))
    {
        if(streamIndex == videoStreamIndex)
        {
            auto pkt = deMuxer.get();
            RTF_ASSERT(fabs((timeBase.second / (double)pkt.get_duration()) - 23.976) < 0.5f);
        }
    }

    RTF_ASSERT( deMuxer.end_of_file() );
}

void test_r_av_r_demuxer::test_move()
{
    // test move assignment
    {
        r_demuxer dmA("big.mp4");

        {
            r_demuxer dmB("this_world_is_a_lie.mkv");
            dmA = std::move(dmB);
        }
    }

    // test move ctor
    {
        r_demuxer dmA("big.mp4");
        r_demuxer dmB = std::move(dmA);
    }
}

void test_r_av_r_demuxer::test_pts()
{
    r_demuxer dm("this_world_is_a_lie.mkv");
    int videoStreamIndex = dm.get_video_stream_index();
    auto tb = dm.get_time_base(videoStreamIndex);
    RTF_ASSERT(tb.first == 1 && tb.second == 1000);
    dm.set_output_time_base(videoStreamIndex, {1, 90000});
    int si = 0;
    do { dm.read_frame(si); } while(si != videoStreamIndex);
    auto pkt_1 = dm.get();
    do { dm.read_frame(si); } while(si != videoStreamIndex);
    auto pkt_2 = dm.get();
    int64_t delta = pkt_2.get_pts() - pkt_1.get_pts();
    int64_t fps = 90000 / delta;
    RTF_ASSERT(fps == 23 || fps == 24);
}