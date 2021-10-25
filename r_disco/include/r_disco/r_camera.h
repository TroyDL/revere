#ifndef r_disco_r_camera_h
#define r_disco_r_camera_h

#include <r_utils/r_nullable.h>
#include <string>

namespace r_disco
{

struct r_camera
{
    std::string id;
    std::string ipv4;
    std::string rtsp_url;
    r_utils::r_nullable<std::string> rtsp_username;
    r_utils::r_nullable<std::string> rtsp_password;
    std::string video_codec;
    r_utils::r_nullable<std::string> video_parameters;
    int video_timebase;
    std::string audio_codec;
    r_utils::r_nullable<std::string> audio_parameters;
    int audio_timebase;
    std::string state;
    std::string stream_config_hash;
};

}

#endif
