#ifndef r_disco_r_camera_h
#define r_disco_r_camera_h

#include <r_utils/r_nullable.h>
#include <string>

namespace r_disco
{

struct r_camera
{
    std::string id;
    r_utils::r_nullable<std::string> camera_name;
    r_utils::r_nullable<std::string> ipv4;
    r_utils::r_nullable<std::string> rtsp_url;
    r_utils::r_nullable<std::string> rtsp_username;
    r_utils::r_nullable<std::string> rtsp_password;
    r_utils::r_nullable<std::string> video_codec;
    r_utils::r_nullable<std::string> video_codec_parameters;
    r_utils::r_nullable<int> video_timebase;
    r_utils::r_nullable<std::string> audio_codec;
    r_utils::r_nullable<std::string> audio_codec_parameters;
    r_utils::r_nullable<int> audio_timebase;
    std::string state;
    r_utils::r_nullable<std::string> record_file_path;
    r_utils::r_nullable<int> n_record_file_blocks;
    r_utils::r_nullable<int> record_file_block_size;
    std::string stream_config_hash;
};

}

#endif
