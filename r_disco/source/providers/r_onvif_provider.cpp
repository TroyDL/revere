
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
    _agent(agent)
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

vector<r_stream_config> r_onvif_provider::_fetch_configs(const string& top_dir)
{
    // run _session.discover()
    // for each discovered
    //     gen guid
    //     lookup credentials w/ guid
    //     attempt _session.get_rtsp_url()
    //     connect to its rtsp url and pull stream info
    //     build r_stream_config, append to result
    // return result

    std::vector<r_stream_config> configs;

    auto discovered = _session.discover();

    for(auto& d : discovered)
    {
        r_stream_config config;

        try
        {
            r_md5 hash;
            hash.update((uint8_t*)d.address.c_str(), d.address.size());
            hash.finalize();

            config.id = hash.get_as_uuid();

            if(!_agent->is_recording(config.id))
            {
                config.ipv4 = d.ipv4;

                auto credentials = _agent->get_credentials(config.id);

                config.rtsp_username = credentials.first;
                config.rtsp_password = credentials.second;

                auto di = _session.get_rtsp_url(d, config.rtsp_username, config.rtsp_password);

                if(!di.is_null())
                {
                    config.rtsp_url = di.value().rtsp_url;

                    auto sdp_media = fetch_sdp_media(di.value().rtsp_url, config.rtsp_username, config.rtsp_password);

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

                    //stream_config.record_file_path = record_file_path;
                    //stream_config.n_record_file_blocks = n_record_file_blocks;
                    //stream_config.record_file_block_size = record_file_block_size;
                
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

                    configs.push_back(config);
                }
            }
        }
        catch(exception& ex)
        {
            R_LOG_ERROR("%s", ex.what());
        }
    }

    return configs;
}
