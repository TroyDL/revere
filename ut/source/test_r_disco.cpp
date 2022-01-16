
#include "test_r_disco.h"
#include "utils.h"
#include "r_pipeline/r_gst_source.h"
#include "r_pipeline/r_arg.h"
#include "r_pipeline/r_stream_info.h"
#include "r_fakey/r_fake_camera.h"
#include "r_utils/r_string_utils.h"
#include "r_utils/r_work_q.h"
#include "r_utils/r_std_utils.h"
#include "r_utils/r_file.h"
#include "r_disco/providers/r_manual_provider.h"
#include "r_disco/r_agent.h"
#include "r_disco/r_devices.h"

#include <sys/types.h>
#include <thread>
#include <vector>
#include <functional>
#include <algorithm>
#include <iterator>
#include <utility>
#include <tuple>

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
using namespace r_pipeline;
using namespace r_fakey;
using namespace r_utils;

REGISTER_TEST_FIXTURE(test_r_disco);

using namespace r_disco;

static string _create_mp_config(int port, const vector<tuple<string,string,string,string,string,string,string,string>>& file_names)
{
    string devices;
    for(auto i = begin(file_names), e = end(file_names); i != e; ++i)
    {
        string id, camera_name, filename, username, password, record_file_path, n_record_file_blocks, record_file_block_size;
        std::tie(id,camera_name, filename,username,password,record_file_path,n_record_file_blocks,record_file_block_size) = *i;
        devices += r_string_utils::format(
            "{\"id\": \"%s\", "
            "\"camera_name\": \"%s\", "
            "\"ipv4_address\": \"127.0.0.1\", "
            "\"rtsp_url\": \"rtsp://127.0.0.1:%d/%s\", "
            "\"record_file_path\": \"%s\", "
            "\"n_record_file_blocks\": %s, "
            "\"record_file_block_size\": %s, "
            "\"username\": \"%s\", "
            "\"password\": \"%s\"}%s",
            id.c_str(),
            camera_name.c_str(),
            port,
            filename.c_str(),
            record_file_path.c_str(),
            n_record_file_blocks.c_str(),
            record_file_block_size.c_str(),
            username.c_str(),
            password.c_str(),
            (next(i) != e)?",":""
        );
    }

    return r_string_utils::format("{\"manual_provider_config\":{\"devices\": [%s]}}",
        devices.c_str()
    );
}

void test_r_disco::setup()
{
    gst_init(NULL, NULL);

    teardown();

    r_fs::mkdir("top_dir");
    r_fs::mkdir("top_dir" + r_fs::PATH_SLASH + "config");
    r_fs::mkdir("top_dir" + r_fs::PATH_SLASH + "db");
}

void test_r_disco::teardown()
{
    auto cfg_json_path = "top_dir" + r_fs::PATH_SLASH + "config" + r_fs::PATH_SLASH + "manual_config.json";
    if(r_fs::file_exists(cfg_json_path))
        r_fs::remove_file(cfg_json_path);
    auto cfg_path = "top_dir" + r_fs::PATH_SLASH + "config";
    if(r_fs::file_exists(cfg_path))
        r_fs::rmdir(cfg_path);
    if(r_fs::file_exists("top_dir" + r_fs::PATH_SLASH + "db" + r_fs::PATH_SLASH + "cameras.db"))
        r_fs::remove_file("top_dir" + r_fs::PATH_SLASH + "db" + r_fs::PATH_SLASH + "cameras.db");
    if(r_fs::file_exists("top_dir" + r_fs::PATH_SLASH + "db"))
        r_fs::rmdir("top_dir" + r_fs::PATH_SLASH + "db");
    if(r_fs::file_exists("top_dir"))
        r_fs::rmdir("top_dir");
}

void test_r_disco::test_r_disco_r_manual_provider()
{
    int port = RTF_NEXT_PORT();

    auto fc = _create_fc(port, "root", "1234");

    auto fct = thread([&](){
        fc->start();
    });
    fct.detach();

    auto cfg = _create_mp_config(port,{
        make_tuple("93950da6-fc12-493c-a051-c22a9fec3440","fake_cam1","true_north_h264_aac.mkv","root","1234","/recording/93950da6-fc12-493c-a051-c22a9fec3440","128","65536"),
        make_tuple("93f03645-0cc3-4c7a-a9e5-f3456a986440","fake_cam2","true_north_h264_aac.mkv","root","1234","/recording/93f03645-0cc3-4c7a-a9e5-f3456a986440","128","65536"),
        make_tuple("27d0f031-da8d-41a0-9687-5fd689a78bec","fake_cam3","true_north_h265_mulaw.mkv","root","1234","/recording/27d0f031-da8d-41a0-9687-5fd689a78bec","128","65536"),
        make_tuple("13be8a39-7e92-4aa1-abbd-7b441856afde","fake_cam4","true_north_h265_aac.mkv","root","1234","/recording/13be8a39-7e92-4aa1-abbd-7b441856afde","128","65536")
    });

    r_fs::write_file((uint8_t*)cfg.c_str(), cfg.size(), "top_dir" + r_fs::PATH_SLASH + "config" + r_fs::PATH_SLASH + "manual_config.json");

    r_manual_provider mp("top_dir", nullptr);

    auto configs = mp.poll();

    RTF_ASSERT(configs[0].camera_name == "fake_cam1");
    RTF_ASSERT(configs[0].video_codec == "h264");
    RTF_ASSERT(configs[0].video_timebase == 90000);
    RTF_ASSERT(configs[0].audio_codec == "mp4a-latm");
    RTF_ASSERT(configs[0].audio_timebase == 48000);
    RTF_ASSERT(configs[0].record_file_path.value() == "/recording/93950da6-fc12-493c-a051-c22a9fec3440");
    RTF_ASSERT(configs[0].n_record_file_blocks.value() == 128);
    RTF_ASSERT(configs[0].record_file_block_size.value() == 65536);

    RTF_ASSERT(configs[1].camera_name == "fake_cam2");
    RTF_ASSERT(configs[1].video_codec == "h264");
    RTF_ASSERT(configs[1].video_timebase == 90000);
    RTF_ASSERT(configs[1].audio_codec == "mp4a-latm");
    RTF_ASSERT(configs[1].audio_timebase == 48000);
    RTF_ASSERT(configs[1].record_file_path.value() == "/recording/93f03645-0cc3-4c7a-a9e5-f3456a986440");
    RTF_ASSERT(configs[1].n_record_file_blocks.value() == 128);
    RTF_ASSERT(configs[1].record_file_block_size.value() == 65536);

    RTF_ASSERT(configs[2].camera_name == "fake_cam3");
    RTF_ASSERT(configs[2].video_codec == "h265");
    RTF_ASSERT(configs[2].video_timebase == 90000);
    RTF_ASSERT(configs[2].audio_codec == "pcmu");
    RTF_ASSERT(configs[2].audio_timebase == 8000);
    RTF_ASSERT(configs[2].record_file_path.value() == "/recording/27d0f031-da8d-41a0-9687-5fd689a78bec");
    RTF_ASSERT(configs[2].n_record_file_blocks.value() == 128);
    RTF_ASSERT(configs[2].record_file_block_size.value() == 65536);

    RTF_ASSERT(configs[3].camera_name == "fake_cam4");
    RTF_ASSERT(configs[3].video_codec == "h265");
    RTF_ASSERT(configs[3].video_timebase == 90000);
    RTF_ASSERT(configs[3].audio_codec == "mp4a-latm");
    RTF_ASSERT(configs[3].audio_timebase == 48000);
    RTF_ASSERT(configs[3].record_file_path.value() == "/recording/13be8a39-7e92-4aa1-abbd-7b441856afde");
    RTF_ASSERT(configs[3].n_record_file_blocks.value() == 128);
    RTF_ASSERT(configs[3].record_file_block_size.value() == 65536);

    fc->quit();
}

void test_r_disco::test_r_disco_r_agent_start_stop()
{
    int port = RTF_NEXT_PORT();

    auto fc = _create_fc(port, "root", "1234");

    auto fct = thread([&](){
        fc->start();
    });
    fct.detach();

    auto cfg = _create_mp_config(port,{
        make_tuple("93950da6-fc12-493c-a051-c22a9fec3440","fake_cam1","true_north_h264_aac.mkv","root","1234","/recording/93950da6-fc12-493c-a051-c22a9fec3440","128","65536"),
        make_tuple("93f03645-0cc3-4c7a-a9e5-f3456a986440","fake_cam2","true_north_h264_aac.mkv","root","1234","/recording/93f03645-0cc3-4c7a-a9e5-f3456a986440","128","65536"),
        make_tuple("27d0f031-da8d-41a0-9687-5fd689a78bec","fake_cam3","true_north_h265_mulaw.mkv","root","1234","/recording/27d0f031-da8d-41a0-9687-5fd689a78bec","128","65536"),
        make_tuple("13be8a39-7e92-4aa1-abbd-7b441856afde","fake_cam4","true_north_h265_aac.mkv","root","1234","/recording/13be8a39-7e92-4aa1-abbd-7b441856afde","128","65536")
    });

    r_fs::write_file((uint8_t*)cfg.c_str(), cfg.size(), "top_dir" + r_fs::PATH_SLASH + "config" + r_fs::PATH_SLASH + "manual_config.json");

    r_agent agent("top_dir");
    agent.set_is_recording_cb([](const std::string&){return false;});

    int num_found_configs = 0;
    agent.set_stream_change_cb([&](const vector<pair<r_stream_config, string>>& configs){
        num_found_configs += (int)configs.size();
    });
    agent.start();

    this_thread::sleep_for(chrono::milliseconds(5000));

    RTF_ASSERT(num_found_configs > 0);

    agent.stop();
}

vector<pair<r_stream_config, string>> _fake_stream_configs()
{
    vector<pair<r_stream_config, string>> configs;

    r_stream_config cfg;
    cfg.id = "93950da6-fc12-493c-a051-c22a9fec3440";
    cfg.camera_name = "fake_cam1";
    cfg.ipv4 = "127.0.0.1";
    cfg.rtsp_url = "rtsp://url";
    cfg.video_codec = "h264";
    cfg.video_timebase = 90000;
    cfg.audio_codec = "mp4a-latm";
    cfg.audio_timebase = 48000;
    cfg.record_file_path = "/recording/93950da6-fc12-493c-a051-c22a9fec3440";
    cfg.n_record_file_blocks = 10;
    cfg.record_file_block_size = 2000;
    configs.push_back(make_pair(cfg, hash_stream_config(cfg)));

    cfg.id = "27d0f031-da8d-41a0-9687-5fd689a78bec";
    cfg.camera_name = "fake_cam2";
    cfg.ipv4 = "127.0.0.1";
    cfg.rtsp_url = "rtsp://url";
    cfg.video_codec = "h265";
    cfg.video_timebase = 90000;
    cfg.audio_codec = "pcmu";
    cfg.audio_timebase = 8000;
    cfg.record_file_path = "/recording/27d0f031-da8d-41a0-9687-5fd689a78bec";
    cfg.n_record_file_blocks = 20;
    cfg.record_file_block_size = 4000;
    configs.push_back(make_pair(cfg, hash_stream_config(cfg)));

    cfg.id = "fae9db8f-08a5-48b7-a22d-1c19a0c05c4f";
    cfg.camera_name = "fake_cam3";
    cfg.ipv4 = "127.0.0.1";
    cfg.rtsp_url = "rtsp://url";
    cfg.video_codec = "h265";
    cfg.video_codec_parameters = "foo=bar";
    cfg.video_timebase = 90000;
    cfg.audio_codec = "mp4a-latm";
    cfg.audio_codec_parameters = "foo=bar";
    cfg.audio_timebase = 8000;
    cfg.record_file_path = "/recording/fae9db8f-08a5-48b7-a22d-1c19a0c05c4f";
    cfg.n_record_file_blocks = 30;
    cfg.record_file_block_size = 6000;
    configs.push_back(make_pair(cfg, hash_stream_config(cfg)));

    return configs;
}

void test_r_disco::test_r_disco_r_devices_basic()
{
    r_devices devices("top_dir");
    devices.start();

    devices.insert_or_update_devices(_fake_stream_configs());

    auto all_cameras = devices.get_all_cameras();

    RTF_ASSERT(all_cameras.size() == 3);
    RTF_ASSERT(all_cameras[0].id == "93950da6-fc12-493c-a051-c22a9fec3440");
    RTF_ASSERT(all_cameras[0].camera_name == "fake_cam1");
    RTF_ASSERT(all_cameras[0].state == "discovered");
    RTF_ASSERT(all_cameras[0].record_file_path == "/recording/93950da6-fc12-493c-a051-c22a9fec3440");
    RTF_ASSERT(all_cameras[0].n_record_file_blocks == 10);
    RTF_ASSERT(all_cameras[0].record_file_block_size == 2000);
    RTF_ASSERT(all_cameras[1].id == "27d0f031-da8d-41a0-9687-5fd689a78bec");
    RTF_ASSERT(all_cameras[1].camera_name == "fake_cam2");
    RTF_ASSERT(all_cameras[1].state == "discovered");
    RTF_ASSERT(all_cameras[1].record_file_path == "/recording/27d0f031-da8d-41a0-9687-5fd689a78bec");
    RTF_ASSERT(all_cameras[1].n_record_file_blocks == 20);
    RTF_ASSERT(all_cameras[1].record_file_block_size == 4000);

    all_cameras[1].rtsp_username = "root";
    all_cameras[1].rtsp_password = "1234";
    all_cameras[1].state = "assigned";
    devices.save_camera(all_cameras[1]);

    auto all_assigned_cameras = devices.get_assigned_cameras();

    RTF_ASSERT(all_assigned_cameras.size() == 1);

    RTF_ASSERT(all_assigned_cameras[0].id == "27d0f031-da8d-41a0-9687-5fd689a78bec");
    RTF_ASSERT(all_assigned_cameras[0].camera_name == "fake_cam2");
    RTF_ASSERT(all_assigned_cameras[0].state == "assigned");
    RTF_ASSERT(all_assigned_cameras[0].record_file_path == "/recording/27d0f031-da8d-41a0-9687-5fd689a78bec");
    RTF_ASSERT(all_assigned_cameras[0].n_record_file_blocks == 20);
    RTF_ASSERT(all_assigned_cameras[0].record_file_block_size == 4000);
    RTF_ASSERT(all_assigned_cameras[0].rtsp_username == "root");
    RTF_ASSERT(all_assigned_cameras[0].rtsp_password == "1234");

    devices.stop();
}

void test_r_disco::test_r_disco_r_devices_modified()
{
#if 0
    r_devices devices("top_dir");
    devices.start();

    devices.insert_or_update_devices(_fake_stream_configs());

    auto all_before = devices.get_all_cameras();

    all_before[1].rtsp_url = "rtsp://new_url";

    devices.save_camera(all_before[1]);

    auto modified = devices.get_modified_cameras(all_before);

    RTF_ASSERT(modified.size() == 1);
    RTF_ASSERT(modified[0].id == "27d0f031-da8d-41a0-9687-5fd689a78bec");

    all_before = devices.get_all_cameras();

    // now, save without modifying anything...
    devices.save_camera(all_before[1]);

    modified = devices.get_modified_cameras(all_before);

    RTF_ASSERT(modified.size() == 0);
#endif
}

void test_r_disco::test_r_disco_r_devices_assigned_cameras_added()
{
#if 0
    r_devices devices("top_dir");
    devices.start();

    devices.insert_or_update_devices(_fake_stream_configs());

    auto all_before = devices.get_all_cameras();

    vector<pair<r_stream_config, string>> configs;

    r_stream_config cfg;
    cfg.id = "9d807570-3d0e-4f87-9773-ae8d6471eab6";
    cfg.ipv4 = "127.0.0.1";
    cfg.rtsp_url = "rtsp://url";
    cfg.video_codec = "h265";
    cfg.video_timebase = 90000;
    cfg.audio_codec = "aac";
    cfg.audio_timebase = 48000;
    configs.push_back(make_pair(cfg, hash_stream_config(cfg)));

    devices.insert_or_update_devices(configs);

    auto c = devices.get_camera_by_id("9d807570-3d0e-4f87-9773-ae8d6471eab6").value();

    c.state = "assigned";

    devices.save_camera(c);

    auto assigned_added = devices.get_assigned_cameras_added(all_before);

    RTF_ASSERT(assigned_added.size() == 1);

    RTF_ASSERT(assigned_added.front().id == "9d807570-3d0e-4f87-9773-ae8d6471eab6");

    auto all_after = devices.get_all_cameras();

    assigned_added = devices.get_assigned_cameras_added(all_after);

    RTF_ASSERT(assigned_added.size() == 0);
#endif
}

void test_r_disco::test_r_disco_r_devices_assigned_cameras_removed()
{
#if 0
    r_devices devices("top_dir");
    devices.start();

    devices.insert_or_update_devices(_fake_stream_configs());

    auto all_before = devices.get_all_cameras();

    vector<pair<r_stream_config, string>> configs;

    r_stream_config cfg;
    cfg.id = "9d807570-3d0e-4f87-9773-ae8d6471eab6";
    cfg.ipv4 = "127.0.0.1";
    cfg.rtsp_url = "rtsp://url";
    cfg.video_codec = "h265";
    cfg.video_timebase = 90000;
    cfg.audio_codec = "aac";
    cfg.audio_timebase = 48000;
    configs.push_back(make_pair(cfg, hash_stream_config(cfg)));

    devices.insert_or_update_devices(configs);

    auto c = devices.get_camera_by_id("9d807570-3d0e-4f87-9773-ae8d6471eab6").value();

    c.state = "assigned";

    devices.save_camera(c);

    auto assigned_before = devices.get_assigned_cameras();

    RTF_ASSERT(assigned_before.size() == 1);

    devices.remove_camera(c);

    auto removed = devices.get_assigned_cameras_removed(assigned_before);

    RTF_ASSERT(removed.size() == 1);
    RTF_ASSERT(removed.front().id == "9d807570-3d0e-4f87-9773-ae8d6471eab6");

    auto assigned_after = devices.get_assigned_cameras();

    RTF_ASSERT(assigned_after.size() == 0);
#endif
}
