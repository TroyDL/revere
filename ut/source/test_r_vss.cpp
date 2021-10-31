
#include "test_r_vss.h"
#include "utils.h"
#include "r_vss/r_stream_keeper.h"
#include "r_storage/r_storage_file.h"
#include "r_disco/r_stream_config.h"
#include "r_utils/r_string_utils.h"
#include "r_utils/r_blob_tree.h"
#include "r_pipeline/r_stream_info.h"
#include "r_mux/r_muxer.h"
#include <gst/gst.h>

using namespace std;
using namespace r_storage;
using namespace r_disco;
using namespace r_utils;
using namespace r_vss;
using namespace r_pipeline;
using namespace r_mux;

REGISTER_TEST_FIXTURE(test_r_vss);

static void _whack_files()
{
    if(r_fs::file_exists("ten_mb_file"))
        r_fs::remove_file("ten_mb_file");
    if(r_fs::file_exists("top_dir/db/cameras.db"))
        r_fs::remove_file("top_dir/db/cameras.db");
    if(r_fs::file_exists("top_dir/db"))
        r_fs::rmdir("top_dir/db");
    if(r_fs::file_exists("top_dir/config"))
        r_fs::rmdir("top_dir/config");
    if(r_fs::file_exists("top_dir"))
        r_fs::rmdir("top_dir");
}

void test_r_vss::setup()
{
    gst_init(NULL, NULL);
    _whack_files();

    r_fs::mkdir("top_dir");
    r_fs::mkdir("top_dir" + r_fs::PATH_SLASH + "config");
    r_fs::mkdir("top_dir" + r_fs::PATH_SLASH + "db");
}

void test_r_vss::teardown()
{
    _whack_files();
}

void test_r_vss::test_r_stream_keeper_basic_recording()
{
    string storage_file_path = "ten_mb_file";

    r_storage_file::allocate(storage_file_path, 65536, 160);

    int port = RTF_NEXT_PORT();
    auto fc = _create_fc(port);

    auto fct = thread([&](){
        fc->start();
    });
    fct.detach();

    r_devices devices("top_dir");
    devices.start();

    r_stream_keeper sk(devices);
    sk.start();

    vector<pair<r_stream_config, string>> configs;

    r_stream_config cfg;
    cfg.id = "9d807570-3d0e-4f87-9773-ae8d6471eab6";
    cfg.ipv4 = "127.0.0.1";
    cfg.rtsp_url = r_string_utils::format("rtsp://127.0.0.1:%d/true_north_h264_aac.mp4", port);
    cfg.video_codec = "h264";
    cfg.video_parameters = "control=stream=0, framerate=23.976023976023978, mediaclk=sender, packetization-mode=1, profile-level-id=64000a, sprop-parameter-sets=Z2QACqzZRifmwFqAgICgAAB9IAAXcAHiRLLA,aOvjyyLA, ts-refclk=local";
    cfg.video_timebase = 90000;
    cfg.audio_codec = "mp4a-latm";
    cfg.audio_timebase = 48000;
    cfg.record_file_path = storage_file_path;
    cfg.record_file_block_size = 65536;
    cfg.n_record_file_blocks = 160;

    configs.push_back(make_pair(cfg, hash_stream_config(cfg)));

    devices.insert_or_update_devices(configs);

    auto c = devices.get_camera_by_id("9d807570-3d0e-4f87-9773-ae8d6471eab6").value();
    c.state = "assigned";
    devices.save_camera(c);

    std::this_thread::sleep_for(std::chrono::seconds(10));
//    sk.stop();

    r_storage_file sf(cfg.record_file_path.value());

    auto kfst = sf.key_frame_start_times(R_STORAGE_MEDIA_TYPE_VIDEO);
    auto result = sf.query(R_STORAGE_MEDIA_TYPE_VIDEO, kfst.front(), kfst.back());

    uint32_t version = 0;
    auto bt = r_blob_tree::deserialize(&result[0], result.size(), version);

    auto video_codec_name = bt["video_codec_name"].get_string();
    auto video_codec_parameters = bt["video_codec_parameters"].get_string();

    printf("video_codec_name=%s\n",video_codec_name.c_str());
    printf("video_codec_parameters=%s\n",video_codec_parameters.c_str());

    auto sps = get_h264_sps(bt["video_codec_parameters"].get_string());
    auto pps = get_h264_pps(bt["video_codec_parameters"].get_string());

    auto sps_info = parse_h264_sps(sps.value());

    r_muxer muxer("output_true_north.mp4");

    muxer.add_video_stream(
        av_d2q(23.96, 10000),
        r_mux::encoding_to_av_codec_id(video_codec_name),
        sps_info.width,
        sps_info.height,
        sps_info.profile_idc,
        sps_info.level_idc
    );

    //muxer.add_audio_stream(r_mux::encoding_to_av_codec_id(audio_codec_name), 

    muxer.set_video_extradata(make_h264_extradata(sps, pps));

    muxer.open();
    
    auto n_frames = bt["frames"].size();

    int64_t dts = 0;
    for(size_t fi = 0; fi < n_frames; ++fi)
    {
        auto key = (bt["frames"][fi]["key"].get_string() == "true");
        auto frame = bt["frames"][fi]["data"].get();
        auto pts = r_string_utils::s_to_int64(bt["frames"][fi]["pts"].get_string());
        
        muxer.write_video_frame(frame.data(), frame.size(), pts, dts, {1, 1000}, key);
        ++dts;
    }

    muxer.finalize();

}
