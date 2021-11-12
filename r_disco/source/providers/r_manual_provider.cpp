
#include "r_disco/providers/r_manual_provider.h"
#include "r_pipeline/r_gst_source.h"
#include "r_utils/3rdparty/json/json.h"
#include "r_utils/r_exception.h"
#include "r_utils/r_string_utils.h"
#include "r_utils/r_file.h"
#include <string>

using namespace r_disco;
using namespace r_utils;
using namespace r_utils::r_string_utils;
using namespace r_pipeline;
using namespace std;
using namespace std::chrono;
using json = nlohmann::json;

r_manual_provider::r_manual_provider(const string& top_dir) :
    _top_dir(top_dir)
{
}

r_manual_provider::~r_manual_provider()
{
}

vector<r_stream_config> r_manual_provider::poll()
{
    return _fetch_configs(_top_dir);
}

vector<r_stream_config> r_manual_provider::_fetch_configs(const string& top_dir)
{
    std::vector<r_stream_config> configs;

    auto config_path = top_dir + r_fs::PATH_SLASH + "config";
    if(!r_fs::file_exists(config_path))
        r_fs::mkdir(config_path);

    auto manual_config_path = config_path + r_fs::PATH_SLASH + "manual_config.json";

    if(r_fs::file_exists(manual_config_path))
    {
        auto config_buffer = r_fs::read_file(manual_config_path);

        auto config = string((char*)&config_buffer[0], config_buffer.size());

        auto doc = json::parse(config);

        for(auto device : doc["manual_provider_config"]["devices"])
        {
            auto id = device["id"];
            auto ipv4_address = device["ipv4_address"];
            auto rtsp_url = device["rtsp_url"];
            auto record_file_path = device["record_file_path"];
            int n_record_file_blocks = device["n_record_file_blocks"];
            int record_file_block_size = device["record_file_block_size"];

            r_nullable<string> username, password;
            if(device.contains("username"))
            {
                username.set_value(device["username"]);
                password.set_value(device["password"]);
            }

            auto sdp_media = fetch_sdp_media(rtsp_url, username, password);

            if(sdp_media.find("video") == sdp_media.end())
                R_THROW(("Unable to fetch video stream information for r_manual_provider."));

            auto video_media = sdp_media["video"];
            if(video_media.type != VIDEO_MEDIA)
                R_THROW(("Unknown media type."));
            
            if(video_media.formats.size() == 0)
                R_THROW(("video media format not found."));

            auto fmt = video_media.formats.front();

            auto rtp_map = video_media.rtpmaps.find(fmt);
            if(rtp_map == video_media.rtpmaps.end())
                R_THROW(("Unable to find rtp map."));
            
            r_stream_config stream_config;

            stream_config.id = id;
            stream_config.ipv4 = ipv4_address;
            stream_config.rtsp_url = rtsp_url;
            stream_config.record_file_path = record_file_path;
            stream_config.n_record_file_blocks = n_record_file_blocks;
            stream_config.record_file_block_size = record_file_block_size;
            
            stream_config.video_codec = encoding_to_str(rtp_map->second.encoding);
            stream_config.video_timebase = rtp_map->second.time_base;

            string video_attributes;
            for(auto b = begin(video_media.attributes), e = end(video_media.attributes); b != e; ++b)
                video_attributes += b->first + "=" + join(split(b->second, " "), ";") + string((next(b) != e)?";":"");
            stream_config.video_codec_parameters.set_value(video_attributes);

            if(sdp_media.find("audio") != sdp_media.end())
            {
                auto audio_media = sdp_media["audio"];
                if(audio_media.type != AUDIO_MEDIA)
                    R_THROW(("Unknown media type."));
                
                if(audio_media.formats.size() == 0)
                    R_THROW(("audio media format not found."));

                auto fmt = audio_media.formats.front();

                auto rtp_map = audio_media.rtpmaps.find(fmt);
                if(rtp_map == audio_media.rtpmaps.end())
                    R_THROW(("Unable to find audio rtp map."));

                stream_config.audio_codec = encoding_to_str(rtp_map->second.encoding);
                stream_config.audio_timebase = rtp_map->second.time_base;

                string audio_attributes;
                for(auto b = begin(audio_media.attributes), e = end(audio_media.attributes); b != e; ++b)
                    audio_attributes += b->first + "=" + join(split(b->second, " "), ";") + string((next(b) != e)?";":"");
                stream_config.audio_codec_parameters.set_value(audio_attributes);
            }

            configs.push_back(stream_config);
        }
    }

    return configs;
}
