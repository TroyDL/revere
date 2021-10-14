
#include "test_r_disco.h"
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

static string _create_mp_config(int port, const vector<tuple<string,string,string,string,string,string,string>>& file_names)
{
    string devices;
    for(auto i = begin(file_names), e = end(file_names); i != e; ++i)
    {
        string id, filename, username, password, record_file_path, n_record_file_blocks, record_file_block_size;
        std::tie(id,filename,username,password,record_file_path,n_record_file_blocks,record_file_block_size) = *i;
        devices += r_string_utils::format(
            "{\"id\": \"%s\", "
            "\"ipv4_address\": \"127.0.0.1\", "
            "\"rtsp_url\": \"rtsp://127.0.0.1:%d/%s\", "
            "\"record_file_path\": \"%s\", "
            "\"n_record_file_blocks\": %s, "
            "\"record_file_block_size\": %s, "
            "\"username\": \"%s\", "
            "\"password\": \"%s\"}%s",
            id.c_str(),
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

static shared_ptr<r_fake_camera> _create_fc(int port, const string& username, const string& password)
{
    auto fr = r_utils::r_std_utils::get_env("FAKEY_ROOT");

    auto server_root = (fr.empty())?string("."):fr;

    auto file_names = _regular_files_in_dir(server_root);

    vector<string> media_file_names;
    copy_if(begin(file_names), end(file_names), back_inserter(media_file_names),
        [](const string& file_name){return _ends_with(file_name, ".mp4") || _ends_with(file_name, ".mkv");});

    return make_shared<r_fake_camera>(server_root, media_file_names, port, username, password);
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
        make_tuple("93950da6-fc12-493c-a051-c22a9fec3440","true_north_h264_aac.mkv","root","1234","/recording/93950da6-fc12-493c-a051-c22a9fec3440","128","65536"),
        make_tuple("93f03645-0cc3-4c7a-a9e5-f3456a986440","true_north_h264_aac.mkv","root","1234","/recording/93f03645-0cc3-4c7a-a9e5-f3456a986440","128","65536"),
        make_tuple("27d0f031-da8d-41a0-9687-5fd689a78bec","true_north_h265_mulaw.mkv","root","1234","/recording/27d0f031-da8d-41a0-9687-5fd689a78bec","128","65536"),
        make_tuple("13be8a39-7e92-4aa1-abbd-7b441856afde","true_north_h265_aac.mkv","root","1234","/recording/13be8a39-7e92-4aa1-abbd-7b441856afde","128","65536")
    });

    r_fs::write_file((uint8_t*)cfg.c_str(), cfg.size(), "top_dir" + r_fs::PATH_SLASH + "config" + r_fs::PATH_SLASH + "manual_config.json");

    r_manual_provider mp("top_dir");

    auto configs = mp.poll();

    RTF_ASSERT(configs[0].video_codec == "h264");
    RTF_ASSERT(configs[0].video_timebase == 90000);
    RTF_ASSERT(configs[0].audio_codec == "mp4a-latm");
    RTF_ASSERT(configs[0].audio_timebase == 48000);
    RTF_ASSERT(configs[0].record_file_path.value() == "/recording/93950da6-fc12-493c-a051-c22a9fec3440");
    RTF_ASSERT(configs[0].n_record_file_blocks.value() == 128);
    RTF_ASSERT(configs[0].record_file_block_size.value() == 65536);

    RTF_ASSERT(configs[1].video_codec == "h264");
    RTF_ASSERT(configs[1].video_timebase == 90000);
    RTF_ASSERT(configs[1].audio_codec == "mp4a-latm");
    RTF_ASSERT(configs[1].audio_timebase == 48000);
    RTF_ASSERT(configs[0].record_file_path.value() == "/recording/93f03645-0cc3-4c7a-a9e5-f3456a986440");
    RTF_ASSERT(configs[0].n_record_file_blocks.value() == 128);
    RTF_ASSERT(configs[0].record_file_block_size.value() == 65536);

    RTF_ASSERT(configs[2].video_codec == "h265");
    RTF_ASSERT(configs[2].video_timebase == 90000);
    RTF_ASSERT(configs[2].audio_codec == "pcmu");
    RTF_ASSERT(configs[2].audio_timebase == 8000);
    RTF_ASSERT(configs[0].record_file_path.value() == "/recording/27d0f031-da8d-41a0-9687-5fd689a78bec");
    RTF_ASSERT(configs[0].n_record_file_blocks.value() == 128);
    RTF_ASSERT(configs[0].record_file_block_size.value() == 65536);

    RTF_ASSERT(configs[3].video_codec == "h265");
    RTF_ASSERT(configs[3].video_timebase == 90000);
    RTF_ASSERT(configs[3].audio_codec == "mp4a-latm");
    RTF_ASSERT(configs[3].audio_timebase == 48000);
    RTF_ASSERT(configs[0].record_file_path.value() == "/recording/13be8a39-7e92-4aa1-abbd-7b441856afde");
    RTF_ASSERT(configs[0].n_record_file_blocks.value() == 128);
    RTF_ASSERT(configs[0].record_file_block_size.value() == 65536);

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
        make_tuple("93950da6-fc12-493c-a051-c22a9fec3440","true_north_h264_aac.mkv","root","1234","/recording/93950da6-fc12-493c-a051-c22a9fec3440","128","65536"),
        make_tuple("93f03645-0cc3-4c7a-a9e5-f3456a986440","true_north_h264_aac.mkv","root","1234","/recording/93f03645-0cc3-4c7a-a9e5-f3456a986440","128","65536"),
        make_tuple("27d0f031-da8d-41a0-9687-5fd689a78bec","true_north_h265_mulaw.mkv","root","1234","/recording/27d0f031-da8d-41a0-9687-5fd689a78bec","128","65536"),
        make_tuple("13be8a39-7e92-4aa1-abbd-7b441856afde","true_north_h265_aac.mkv","root","1234","/recording/13be8a39-7e92-4aa1-abbd-7b441856afde","128","65536")
    });

    r_fs::write_file((uint8_t*)cfg.c_str(), cfg.size(), "top_dir" + r_fs::PATH_SLASH + "config" + r_fs::PATH_SLASH + "manual_config.json");

    r_agent agent("top_dir");

    int num_found_configs = 0;
    agent.set_stream_change_cb([&](const vector<r_stream_config>& configs){
        num_found_configs += (int)configs.size();
    });
    agent.start();

    this_thread::sleep_for(chrono::milliseconds(1000));

    RTF_ASSERT(num_found_configs > 0);

    agent.stop();
}

vector<r_stream_config> _fake_stream_configs()
{
    vector<r_stream_config> configs;

    r_stream_config cfg;
    cfg.id = "93950da6-fc12-493c-a051-c22a9fec3440";
    cfg.ipv4 = "127.0.0.1";
    cfg.rtsp_url = "rtsp://url";
    cfg.video_codec = "h264";
    cfg.video_timebase = 90000;
    cfg.audio_codec = "mp4a-latm";
    cfg.audio_timebase = 48000;
    configs.push_back(cfg);

    cfg.id = "27d0f031-da8d-41a0-9687-5fd689a78bec";
    cfg.ipv4 = "127.0.0.1";
    cfg.rtsp_url = "rtsp://url";
    cfg.video_codec = "h265";
    cfg.video_timebase = 90000;
    cfg.audio_codec = "pcmu";
    cfg.audio_timebase = 8000;
    configs.push_back(cfg);

    return configs;
}

void test_r_disco::test_r_disco_r_devices_basic()
{
    r_devices devices("top_dir");
    devices.start();

    devices.insert_or_update_devices(_fake_stream_configs());

    auto all_cameras = devices.get_all_cameras().value();

    RTF_ASSERT(all_cameras.size() == 2);
    RTF_ASSERT(all_cameras[0].id == "93950da6-fc12-493c-a051-c22a9fec3440");
    RTF_ASSERT(all_cameras[0].state == "discovered");
    RTF_ASSERT(all_cameras[1].id == "27d0f031-da8d-41a0-9687-5fd689a78bec");
    RTF_ASSERT(all_cameras[1].state == "discovered");

    all_cameras[1].rtsp_username = "root";
    all_cameras[1].rtsp_password = "1234";
    all_cameras[1].state = "assigned";
    devices.save_camera(all_cameras[1]);

    auto all_assigned_cameras = devices.get_assigned_cameras().value();

    RTF_ASSERT(all_assigned_cameras.size() == 1);

    RTF_ASSERT(all_assigned_cameras[0].id == "27d0f031-da8d-41a0-9687-5fd689a78bec");
    RTF_ASSERT(all_assigned_cameras[0].state == "assigned");
    RTF_ASSERT(all_assigned_cameras[0].rtsp_username == "root");
    RTF_ASSERT(all_assigned_cameras[0].rtsp_password == "1234");

    devices.stop();
}
