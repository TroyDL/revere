
#include "test_r_av_r_demuxer.h"
#include "r_av/r_demuxer.h"
#include "r_av/r_muxer.h"
#include "r_av/r_locky.h"
#include "r_av/r_options.h"
#include "r_utils/r_file.h"

#include "this_world_is_a_lie.cpp"
#include "got_frames.cpp"

using namespace std;
using namespace r_utils;
using namespace r_av;

REGISTER_TEST_FIXTURE(test_r_av_r_demuxer);

// Write an MP4 to a header file.
void write_header(const string& inputFileName, const string& outputFileName, const string& prefix)
{
    r_demuxer deMuxer(inputFileName);

    auto outFile = r_file::open(outputFileName, "w");

    auto ed = deMuxer.get_extradata(deMuxer.get_video_stream_index());

    fprintf(outFile, "size_t %s_ed_size = %lu;\n", prefix.c_str(), ed.size());
    fprintf(outFile, "uint8_t %s_ed[] = {", prefix.c_str());
    for(size_t i = 0; i < ed.size()-1; ++i)
        fprintf(outFile, "0x%x,", ed[i]);
    fprintf(outFile, "0x%x};\n", ed[ed.size()-1]);

    vector<bool> frameKeyness;

    int frameIndex = 0;
    int streamIndex = 0;
    while( deMuxer.read_frame( streamIndex ) )
    {
        auto frame = deMuxer.get();

        fprintf(outFile, "size_t %s_frame_%d_size = %ld;\n", prefix.c_str(), frameIndex, frame.get_data_size());

        const uint8_t* reader = frame.map();

        fprintf(outFile, "uint8_t %s_frame_%d[] = {", prefix.c_str(), frameIndex);
        for(size_t i = 0; i < frame.get_data_size() - 1; ++i)
        {
            fprintf(outFile, "0x%x,", *reader);
            ++reader;
        }
        fprintf(outFile, "0x%x};\n", *reader);

        if(frame.is_key())
            frameKeyness.push_back(true);
        else frameKeyness.push_back(false);

        ++frameIndex;
    }

    fprintf(outFile, "const size_t %s_num_frames = %d;\n", prefix.c_str(), frameIndex);

    fprintf(outFile, "uint8_t* %s_frames[] = {", prefix.c_str());
    for(int i = 0; i < frameIndex-1; ++i)
    {
        fprintf(outFile, "&%s_frame_%d[0],", prefix.c_str(), i);
    }
    fprintf(outFile, "&%s_frame_%d[0]};\n", prefix.c_str(), frameIndex-1);

    fprintf(outFile, "size_t %s_frames_sizes[] = {", prefix.c_str());
    for(int i = 0; i < frameIndex-1; ++i)
    {
        fprintf(outFile, "%s_frame_%d_size,", prefix.c_str(), i);
    }
    fprintf(outFile, "%s_frame_%d_size};\n", prefix.c_str(), frameIndex-1);

    fprintf(outFile, "bool %s_frames_keyness[] = {", prefix.c_str());
    for(int i = 0; i < frameIndex-1; ++i)
    {
        fprintf(outFile, "%s,", (frameKeyness[i])?"true":"false");
    }
    fprintf(outFile, "%s};", (frameKeyness[frameIndex-1])?"true":"false");
}

void test_r_av_r_demuxer::setup()
{
    r_locky::register_ffmpeg();

    got_ed[0] = got_ed[0];
    got_ed_size = got_ed_size;

    //write_header("got.mp4", "got_frames.cpp", "got");

    {
        r_muxer m(r_muxer::OUTPUT_LOCATION_FILE, "got.mp4" );

        r_stream_options soptions;
        soptions.type = "video";
        soptions.codec_id = r_av_codec_id_h264;
        soptions.format = r_av_pix_fmt_yuv420p;
        soptions.width = 1280;
        soptions.height = 720;
        soptions.time_base_num = 1;
        soptions.time_base_den = 23980;

        auto streamIndex = m.add_stream(soptions);

        int64_t ts = 0;
        for(size_t i = 0; i < got_num_frames; ++i)
        {
            r_packet pkt(got_frames[i], got_frames_sizes[i], false);
            pkt.set_time_base({1, 90000});
            pkt.set_pts(ts);
            pkt.set_dts(ts);
            pkt.set_duration(3753); // 90000 / 23.98 fps
            ts+=3753;
            m.write_packet(pkt, streamIndex, got_frames_keyness[i]);
        }

        m.finalize_file();
    }

    r_utils::r_fs::write_file(this_world_is_a_lie_mkv, this_world_is_a_lie_mkv_len, "this_world_is_a_lie.mkv");
}

void test_r_av_r_demuxer::teardown()
{
    r_locky::unregister_ffmpeg();

    remove("got.mp4");
    remove("this_world_is_a_lie.mkv");
}

void test_r_av_r_demuxer::test_constructor()
{
    r_demuxer dm("got.mp4");
}

void test_r_av_r_demuxer::test_examine_file()
{
    r_demuxer deMuxer( "got.mp4" );

    int videoStreamIndex = deMuxer.get_video_stream_index();

    auto ed = deMuxer.get_extradata(videoStreamIndex);

    auto frameRate = deMuxer.get_frame_rate(videoStreamIndex);
    RTF_ASSERT(frameRate.first == 24000);
    RTF_ASSERT(frameRate.second == 1001);
    RTF_ASSERT(deMuxer.get_stream_codec_id(videoStreamIndex) == r_av_codec_id_h264);
    RTF_ASSERT(deMuxer.get_pix_format(videoStreamIndex) == r_av_pix_fmt_yuv420p);

    auto res = deMuxer.get_resolution(videoStreamIndex);
    RTF_ASSERT(res.first == 1280);
    RTF_ASSERT(res.second == 720);
    //auto ed = deMuxer.get_extradata(videoStreamIndex);
    RTF_ASSERT(ed.size() > 0);

    int streamIndex = 0;

    for(size_t i = 0; i < got_num_frames; ++i)
    {
        deMuxer.read_frame(streamIndex);

        auto pkt = deMuxer.get();

        RTF_ASSERT(deMuxer.is_key() == got_frames_keyness[i]);
        RTF_ASSERT(pkt.get_data_size() == got_frames_sizes[i]);
    }

    deMuxer.read_frame(streamIndex);

    RTF_ASSERT( deMuxer.end_of_file() );
}

void test_r_av_r_demuxer::test_file_from_memory()
{
    auto buffer = r_fs::read_file("got.mp4");
    r_demuxer deMuxer( &buffer[0], buffer.size() );

    int videoStreamIndex = deMuxer.get_video_stream_index();

    auto frameRate = deMuxer.get_frame_rate(videoStreamIndex);
    RTF_ASSERT(frameRate.first == 24000);
    RTF_ASSERT(frameRate.second == 1001);
    RTF_ASSERT(deMuxer.get_stream_codec_id(videoStreamIndex) == r_av_codec_id_h264);
    RTF_ASSERT(deMuxer.get_pix_format(videoStreamIndex) == r_av_pix_fmt_yuv420p);

    auto res = deMuxer.get_resolution(videoStreamIndex);
    RTF_ASSERT(res.first == 1280);
    RTF_ASSERT(res.second == 720);
    auto ed = deMuxer.get_extradata(videoStreamIndex);
    RTF_ASSERT(ed.size() > 0);

    int streamIndex = 0;

    for(size_t i = 0; i < got_num_frames; ++i)
    {
        deMuxer.read_frame(streamIndex);

        auto pkt = deMuxer.get();

        RTF_ASSERT(deMuxer.is_key() == got_frames_keyness[i]);
        RTF_ASSERT(pkt.get_data_size() == got_frames_sizes[i]);
    }

    deMuxer.read_frame(streamIndex);

    RTF_ASSERT( deMuxer.end_of_file() );
}

void test_r_av_r_demuxer::test_get_container_statistics()
{
    auto stats = r_demuxer::get_video_stream_statistics( "got.mp4" );

    RTF_ASSERT( !stats.averageBitRate.is_null() );
    // Is the averageBitRate in the right ballpark?
    RTF_ASSERT( (stats.averageBitRate.value() > 700000) && (stats.averageBitRate.value() < 800000) );
    RTF_ASSERT( !stats.frameRate.is_null() );
    RTF_ASSERT( !stats.gopSize.is_null() );
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
        r_demuxer dmA("got.mp4");

        {
            r_demuxer dmB("this_world_is_a_lie.mkv");
            dmA = std::move(dmB);
        }
    }

    // test move ctor
    {
        r_demuxer dmA("got.mp4");
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