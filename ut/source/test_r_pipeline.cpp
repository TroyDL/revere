
#include "test_r_pipeline.h"
#include "bad_guy.h"
#include "r_pipeline/r_gst_source.h"
#include "r_pipeline/r_arg.h"
#include "r_pipeline/r_stream_info.h"
#include "r_fakey/r_fake_camera.h"
#include "r_utils/r_string_utils.h"
#include "r_utils/r_work_q.h"
#include "r_utils/r_std_utils.h"

#include <sys/types.h>
#include <thread>
#include <vector>
#include <functional>
#include <algorithm>
#include <iterator>
#include <chrono>
#ifdef IS_WINDOWS
#include <Windows.h>
#include <tchar.h> 
#include <stdio.h>
#include <strsafe.h>
#pragma warning( push )
#pragma warning( disable : 4244 )
#endif
#include <gst/gst.h>
#ifdef IS_WINDOWS
#pragma warning( pop )
#endif
#ifdef IS_LINUX
#include <dirent.h>
#endif

using namespace std;
using namespace std::chrono;
using namespace r_pipeline;
using namespace r_fakey;
using namespace r_utils;

REGISTER_TEST_FIXTURE(test_r_pipeline);

void test_r_pipeline::setup()
{
    gst_init(NULL, NULL);
}

void test_r_pipeline::teardown()
{
}

static bool _ends_with(const string& a, const string& b)
{
    if (b.size() > a.size()) return false;
    return std::equal(a.begin() + a.size() - b.size(), a.end(), b.begin());
}

static vector<string> _regular_files_in_dir(const string& dir)
{
    vector<string> names;

#ifdef IS_LINUX
    DIR* d = opendir(dir.c_str());
    if(!d)
        throw std::runtime_error("Unable to open directory");
    
    struct dirent* e = readdir(d);

    if(e)
    {
        do
        {
            string name(e->d_name);
            if(e->d_type == DT_REG && name != "." && name != "..")
                names.push_back(name);
            e = readdir(d);
        } while(e);
    }

    closedir(d);
#endif

#ifdef IS_WINDOWS
   WIN32_FIND_DATA ffd;
   TCHAR szDir[1024];
   HANDLE hFind;
   
   StringCchCopyA(szDir, 1024, dir.c_str());
   StringCchCatA(szDir, 1024, "\\*");

   // Find the first file in the directory.

   hFind = FindFirstFileA(szDir, &ffd);

   if (INVALID_HANDLE_VALUE == hFind) 
       throw std::runtime_error("Unable to open directory");

   do
   {
      if(!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
         names.push_back(string(ffd.cFileName)); 
   }
   while (FindNextFileA(hFind, &ffd) != 0);

   FindClose(hFind);
#endif

    return names;
}

static shared_ptr<r_fake_camera> _create_fc(int port)
{
    auto fr = r_utils::r_std_utils::get_env("FAKEY_ROOT");

    auto server_root = (fr.empty())?string("."):fr;

    auto file_names = _regular_files_in_dir(server_root);

    vector<string> media_file_names;
    copy_if(begin(file_names), end(file_names), back_inserter(media_file_names),
        [](const string& file_name){return _ends_with(file_name, ".mp4") || _ends_with(file_name, ".mkv");});

    return make_shared<r_fake_camera>(server_root, media_file_names, port);
}

void test_r_pipeline::test_gst_source_h264_aac()
{
    int port = RTF_NEXT_PORT();

    auto fc = _create_fc(port);

    auto fct = thread([&](){
        fc->start();
    });
    fct.detach();

    vector<r_arg> arguments;
    add_argument(arguments, "url", r_string_utils::format("rtsp://127.0.0.1:%d/true_north_h264_aac.mp4", port));

    bool got_audio = false, got_video = false;

    r_gst_source src;
    src.set_args(arguments);
    src.set_video_sample_cb(
        [&](const sample_context& ctx, const uint8_t* p, size_t sz, bool key, int64_t pts){
            got_video = true;
        }
    );

    uint8_t audio_channels;
    uint32_t audio_sample_rate;
    src.set_audio_sample_cb(
        [&](const sample_context& ctx, const uint8_t* p, size_t sz, bool key,int64_t pts){
            got_audio = true;
            audio_channels = ctx.audio_channels().value();
            audio_sample_rate = ctx.audio_sample_rate().value();
        }
    );

    bool got_ready = false;
    src.set_ready_cb([&](){
        got_ready = true;
    });

    src.play();

    std::this_thread::sleep_for(std::chrono::seconds(5));

    src.stop();

    fc->quit();

    RTF_ASSERT(got_ready && got_audio && got_video);
}

void test_r_pipeline::test_gst_source_h265_aac()
{
    int port = RTF_NEXT_PORT();

    auto fc = _create_fc(port);

    auto fct = thread([&](){
        fc->start();
    });
    fct.detach();

    vector<r_arg> arguments;
    add_argument(arguments, "url", r_string_utils::format("rtsp://127.0.0.1:%d/true_north_h265_aac.mp4", port));

    bool got_audio = false, got_video = false;

    r_gst_source src;
    src.set_args(arguments);
    src.set_video_sample_cb(
        [&](const sample_context& ctx, const uint8_t* p, size_t sz, bool key, int64_t pts){
            got_video = true;
        }
    );
    src.set_audio_sample_cb(
        [&](const sample_context& ctx, const uint8_t* p, size_t sz, bool key, int64_t pts){
            got_audio = true;
        }
    );

    bool got_ready = false;
    src.set_ready_cb([&](){
        got_ready = true;
    });

    src.play();

    std::this_thread::sleep_for(std::chrono::seconds(5));

    src.stop();

    fc->quit();

    RTF_ASSERT(got_ready && got_audio && got_video);
}

void test_r_pipeline::test_gst_source_h264_mulaw()
{
    int port = RTF_NEXT_PORT();

    auto fc = _create_fc(port);

    auto fct = thread([&](){
        fc->start();
    });
    fct.detach();

    vector<r_arg> arguments;
    add_argument(arguments, "url", r_string_utils::format("rtsp://127.0.0.1:%d/true_north_h264_mulaw.mkv", port));

    bool got_audio = false, got_video = false;

    r_gst_source src;
    src.set_args(arguments);
    src.set_video_sample_cb(
        [&](const sample_context& ctx, const uint8_t* p, size_t sz, bool key, int64_t pts){
            got_video = true;
        }
    );
    src.set_audio_sample_cb(
        [&](const sample_context& ctx, const uint8_t* p, size_t sz, bool key, int64_t pts){
            got_audio = true;
        }
    );
    bool got_ready = false;
    src.set_ready_cb([&](){
        got_ready = true;
    });

    src.play();

    std::this_thread::sleep_for(std::chrono::seconds(5));

    src.stop();

    fc->quit();

    RTF_ASSERT(got_ready && got_audio && got_video);
}

void test_r_pipeline::test_gst_source_h265_mulaw()
{
    int port = RTF_NEXT_PORT();

    auto fc = _create_fc(port);

    auto fct = thread([&](){
        fc->start();
    });
    fct.detach();

    vector<r_arg> arguments;
    add_argument(arguments, "url", r_string_utils::format("rtsp://127.0.0.1:%d/true_north_h265_mulaw.mkv", port));

    bool got_audio = false, got_video = false;

    r_gst_source src;
    src.set_args(arguments);
    src.set_video_sample_cb(
        [&](const sample_context& ctx, const uint8_t* p, size_t sz, bool key, int64_t pts){
            got_video = true;
        }
    );
    src.set_audio_sample_cb(
        [&](const sample_context& ctx, const uint8_t* p, size_t sz, bool key, int64_t pts){
            got_audio = true;
        }
    );
    bool got_ready = false;
    src.set_ready_cb([&](){
        got_ready = true;
    });

    src.play();

    std::this_thread::sleep_for(std::chrono::seconds(5));

    src.stop();

    fc->quit();

    RTF_ASSERT(got_ready && got_audio && got_video);
}

void test_r_pipeline::test_gst_source_fetch_sdp()
{
    int port = RTF_NEXT_PORT();

    auto fc = _create_fc(port);

    auto fct = thread([&](){
        fc->start();
    });

    {
        auto medias = fetch_sdp_media(r_string_utils::format("rtsp://127.0.0.1:%d/true_north_h264_aac.mkv", port));

        RTF_ASSERT(medias["video"].rtpmaps[96].encoding == H264_ENCODING);
        RTF_ASSERT(medias["video"].rtpmaps[96].time_base == 90000);
        RTF_ASSERT(medias["audio"].rtpmaps[97].encoding == AAC_LATM_ENCODING);
    }

    {
        auto medias = fetch_sdp_media(r_string_utils::format("rtsp://127.0.0.1:%d/true_north_h265_aac.mkv", port));

        RTF_ASSERT(medias["video"].rtpmaps[96].encoding == H265_ENCODING);
        RTF_ASSERT(medias["video"].rtpmaps[96].time_base == 90000);
        RTF_ASSERT(medias["audio"].rtpmaps[97].encoding == AAC_LATM_ENCODING);
    }

    fc->quit();
    fct.join();

}

void test_r_pipeline::test_gst_source_pull_real()
{
#if 0
    vector<r_arg> arguments;
    add_argument(arguments, "url", "rtsp://192.168.1.20/h265Preview_01_main");
    //add_argument(arguments, "url", "rtsp://192.168.1.20/h264Preview_01_sub");

    add_argument(arguments, "username", "admin");
    add_argument(arguments, "password", "emperor1");

    r_gst_source src;
    src.set_args(arguments);

    map<string, r_sdp_media> medias;
    src.set_sdp_media_cb([&](const map<string, r_sdp_media>& sdp_medias){
        medias = sdp_medias;
        for(auto ap : medias["video"].rtpmaps)
            printf("%d: %d\n", ap.first, ap.second.time_base);
    });

    src.set_video_sample_cb(
        [&](const sample_context& ctx, const uint8_t* p, size_t sz, bool key, int64_t pts){
            printf("bframes ts = %ld, pts = %ld, key = %s, size = %lu\n", ts, pts, (key)?"true":"false",sz);
        }
    );

    src.play();

    std::this_thread::sleep_for(std::chrono::seconds(10));

    src.stop();
#endif
}

void test_r_pipeline::test_gst_source_bframes()
{
    int port = RTF_NEXT_PORT();

    auto fc = _create_fc(port);

    auto fct = thread([&](){
        fc->start();
    });
    fct.detach();

    vector<r_arg> arguments;
    add_argument(arguments, "url", r_string_utils::format("rtsp://127.0.0.1:%d/true_north_h264_aac.mkv", port));

    r_gst_source src;
    src.set_args(arguments);
    src.set_video_sample_cb(
        [&](const sample_context& ctx, const uint8_t* p, size_t sz, bool key, int64_t pts){
            //printf("bframes ts = %s, pts = %s, key = %s, size = %s\n", r_string_utils::int64_to_s(ts).c_str(), r_string_utils::int64_to_s(pts).c_str(), (key)?"true":"false", r_string_utils::size_t_to_s(sz).c_str());
        }
    );

    src.play();

    std::this_thread::sleep_for(std::chrono::seconds(10));

    src.stop();

    fc->quit();
}

void test_r_pipeline::test_stream_info_get_info_frames()
{
    int port = RTF_NEXT_PORT();

    auto fc = _create_fc(port);

    auto fct = thread([&](){
        fc->start();
    });
    fct.detach();

    auto medias = fetch_sdp_media(r_string_utils::format("rtsp://127.0.0.1:%d/true_north_h264_aac.mkv", port));

    auto h264_codec_parameters = sdp_media_to_s(VIDEO_MEDIA, medias).second;

    auto h264_sps_b = get_h264_sps(h264_codec_parameters);
    RTF_ASSERT(!h264_sps_b.value().empty());
    auto h264_pps_b = get_h264_sps(h264_codec_parameters);
    RTF_ASSERT(!h264_pps_b.value().empty());

    medias = fetch_sdp_media(r_string_utils::format("rtsp://127.0.0.1:%d/true_north_h265_aac.mkv", port));

    auto h265_codec_parameters = sdp_media_to_s(VIDEO_MEDIA, medias).second;
    auto h265_vps_b = get_h265_vps(h265_codec_parameters);
    RTF_ASSERT(!h265_vps_b.value().empty());
    auto h265_sps_b = get_h265_sps(h265_codec_parameters);
    RTF_ASSERT(!h265_sps_b.value().empty());
    auto h265_pps_b = get_h265_pps(h265_codec_parameters);
    RTF_ASSERT(!h265_pps_b.value().empty());

    fc->quit();
}

void test_r_pipeline::test_stream_info_make_extradata()
{
    int port = RTF_NEXT_PORT();

    auto fc = _create_fc(port);

    auto fct = thread([&](){
        fc->start();
    });
    fct.detach();

    auto medias = fetch_sdp_media(r_string_utils::format("rtsp://127.0.0.1:%d/true_north_h264_aac.mkv", port));

    auto h264_codec_parameters = sdp_media_to_s(VIDEO_MEDIA, medias).second;

    auto h264_sps_b = get_h264_sps(h264_codec_parameters);
    auto h264_pps_b = get_h264_pps(h264_codec_parameters);

    auto h264_ed = make_h264_extradata(h264_sps_b, h264_pps_b);
    RTF_ASSERT(!h264_ed.empty());

    medias = fetch_sdp_media(r_string_utils::format("rtsp://127.0.0.1:%d/true_north_h265_aac.mkv", port));

    auto h265_codec_parameters = sdp_media_to_s(VIDEO_MEDIA, medias).second;
    auto h265_vps_b = get_h265_vps(h265_codec_parameters);
    auto h265_sps_b = get_h265_sps(h265_codec_parameters);
    auto h265_pps_b = get_h265_pps(h265_codec_parameters);

    auto h265_ed = make_h265_extradata(h265_vps_b, h265_sps_b, h265_pps_b);
    RTF_ASSERT(!h265_ed.empty());

    fc->quit();
}

void test_r_pipeline::test_stream_info_parse_h264_sps()
{
    int port = RTF_NEXT_PORT();

    auto fc = _create_fc(port);

    auto fct = thread([&](){
        fc->start();
    });
    fct.detach();

    auto medias = fetch_sdp_media(r_string_utils::format("rtsp://127.0.0.1:%d/true_north_h264_aac.mkv", port));

    auto h264_codec_parameters = sdp_media_to_s(VIDEO_MEDIA, medias).second;

    auto h264_sps_b = get_h264_sps(h264_codec_parameters);

    auto sps_info = parse_h264_sps(h264_sps_b.value());

    RTF_ASSERT(sps_info.width == 96);
    RTF_ASSERT(sps_info.height = 64);
    RTF_ASSERT(sps_info.profile_idc == 100);
    RTF_ASSERT(sps_info.level_idc == 10);

    fc->quit();
}

void test_r_pipeline::test_stream_info_parse_h265_sps()
{
    int port = RTF_NEXT_PORT();

    auto fc = _create_fc(port);

    auto fct = thread([&](){
        fc->start();
    });
    fct.detach();

    auto medias = fetch_sdp_media(r_string_utils::format("rtsp://127.0.0.1:%d/true_north_h265_aac.mkv", port));

    auto h265_codec_parameters = sdp_media_to_s(VIDEO_MEDIA, medias).second;

    auto h265_sps_b = get_h265_sps(h265_codec_parameters);

    auto sps_info = parse_h265_sps(h265_sps_b.value());

    RTF_ASSERT(sps_info.width == 96);
    RTF_ASSERT(sps_info.height = 56);
    RTF_ASSERT(sps_info.profile_idc == 1);
    RTF_ASSERT(sps_info.level_idc == 30);

    fc->quit();
}
