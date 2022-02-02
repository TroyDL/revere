
#include "r_disco/providers/r_onvif_provider.h"
#include "r_disco/r_agent.h"
#include "r_pipeline/r_gst_source.h"
#include "r_utils/r_exception.h"
#include "r_utils/r_string_utils.h"
#include "r_utils/r_md5.h"
#include <string>

using namespace r_disco;
using namespace r_utils;
using namespace r_utils::r_string_utils;
using namespace r_pipeline;
using namespace std;
using namespace std::chrono;

r_onvif_provider::r_onvif_provider(const string& top_dir, r_agent* agent) :
    _top_dir(top_dir),
    _session(),
    _agent(agent),
    _cache()
{
}

r_onvif_provider::~r_onvif_provider()
{
}

vector<r_stream_config> r_onvif_provider::poll()
{
    vector<r_stream_config> configs;

    try
    {
        configs = _fetch_configs(_top_dir);
    }
    catch(exception& ex)
    {
        printf("r_manual_provider::poll() failed: %s\n", ex.what());
        fflush(stdout);
        R_LOG_NOTICE("r_manual_provider::poll() failed: %s", ex.what());
    }

    return configs;
}

r_utils::r_nullable<r_stream_config> r_onvif_provider::interrogate_camera(
    const std::string& id,
    const std::string& camera_name,
    const std::string& ipv4,
    const std::string& xaddrs,
    const std::string& address,
    r_utils::r_nullable<std::string> username,
    r_utils::r_nullable<std::string> password
)
{
    r_nullable<r_stream_config> config_nullable;
    r_stream_config config;

    try
    {
        config.id = id;
        config.camera_name.set_value(camera_name);
        config.ipv4 = ipv4;
        config.xaddrs = xaddrs;
        config.address = address;

        _cache_check_expiration(id);

        auto it = _cache.find(id);

        if(it == _cache.end())
        {
            if(_agent && _agent->_is_recording(config.id))
                return r_nullable<r_stream_config>();

            auto di = _session.get_rtsp_url(
                camera_name,
                ipv4,
                xaddrs,
                address,
                username,
                password
            );

            if(!di.is_null())
            {
                config.rtsp_url = di.value().rtsp_url;

                auto sdp_media = fetch_sdp_media(di.value().rtsp_url, username, password);

                if(sdp_media.find("video") == sdp_media.end())
                    R_THROW(("Unable to fetch video stream information for r_onvif_provider."));

                auto video_media = sdp_media["video"];
                if(video_media.type != VIDEO_MEDIA)
                    R_THROW(("Unknown media type."));
                
                if(video_media.formats.size() == 0)
                    R_THROW(("video media format not found."));

                auto fmt = video_media.formats.front();

                auto rtp_map = video_media.rtpmaps.find(fmt);
                if(rtp_map == video_media.rtpmaps.end())
                    R_THROW(("Unable to find rtp map."));
            
                config.video_codec = encoding_to_str(rtp_map->second.encoding);
                config.video_timebase = rtp_map->second.time_base;

                string video_attributes;
                for(auto b = begin(video_media.attributes), e = end(video_media.attributes); b != e; ++b)
                    video_attributes += b->first + "=" + join(split(b->second, " "), ";") + string((next(b) != e)?";":"");
                config.video_codec_parameters.set_value(video_attributes);

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

                    config.audio_codec = encoding_to_str(rtp_map->second.encoding);
                    config.audio_timebase = rtp_map->second.time_base;

                    string audio_attributes;
                    for(auto b = begin(audio_media.attributes), e = end(audio_media.attributes); b != e; ++b)
                        audio_attributes += b->first + "=" + join(split(b->second, " "), ";") + string((next(b) != e)?";":"");
                    config.audio_codec_parameters.set_value(audio_attributes);
                }

                _r_onvif_provider_cache_entry cache_entry;
                cache_entry.created = steady_clock::now();
                cache_entry.config = config;
                _cache[id] = cache_entry;
            }

        }
        else
        {
            config = it->second.config;
        }
    }
    catch(exception& ex)
    {
        R_LOG_ERROR("%s", ex.what());
    }

    config_nullable.set_value(config);

    return config_nullable;
}

vector<r_stream_config> r_onvif_provider::_fetch_configs(const string& top_dir)
{
    std::vector<r_stream_config> configs;

    auto discovered = _session.discover();

    for(auto& d : discovered)
    {
        r_stream_config config;

        try
        {
            // Onvif device id's are created by hashing the devices address.
            r_md5 hash;
            hash.update((uint8_t*)d.address.c_str(), d.address.size());
            hash.finalize();
            auto id = hash.get_as_uuid();
            auto credentials = _agent->_get_credentials(id);

            auto sc = interrogate_camera(
                id,
                d.camera_name,
                d.ipv4,
                d.xaddrs,
                d.address,
                credentials.first,
                credentials.second
            );

            if(!sc.is_null())
                configs.push_back(sc.value());
        }
        catch(exception& ex)
        {
            R_LOG_ERROR("%s", ex.what());
        }
    }

    return configs;
}

void r_onvif_provider::_cache_check_expiration(const string& id)
{
    auto it = _cache.find(id);
    if(it != _cache.end())
    {
        if(duration_cast<minutes>(steady_clock::now() - it->second.created).count() > 60 + (rand() % 10))
            _cache.erase(it);
    }
}
