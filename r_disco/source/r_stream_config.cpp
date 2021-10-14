
#include "r_disco/r_stream_config.h"
#include "r_utils/r_md5.h"

using namespace r_utils;
using namespace r_disco;
using namespace std;

string r_disco::hash_stream_config(const r_stream_config& sc)
{
    r_md5 h;

    h.update((uint8_t*)sc.id.c_str(), sc.id.length());
    h.update((uint8_t*)sc.ipv4.c_str(), sc.ipv4.length());
    h.update((uint8_t*)sc.rtsp_url.c_str(), sc.rtsp_url.length());

    h.update((uint8_t*)sc.video_codec.c_str(), sc.video_codec.length());
    if(!sc.video_parameters.is_null())
        h.update((uint8_t*)sc.video_parameters.value().c_str(), sc.video_parameters.value().length());
    h.update((uint8_t*)&sc.video_timebase, sizeof(sc.video_timebase));

    h.update((uint8_t*)sc.audio_codec.c_str(), sc.audio_codec.length());
    if(!sc.audio_parameters.is_null())
        h.update((uint8_t*)sc.audio_parameters.value().c_str(), sc.audio_parameters.value().length());
    h.update((uint8_t*)&sc.audio_timebase, sizeof(sc.audio_timebase));

    if(!sc.record_file_path.is_null())
        h.update((uint8_t*)sc.record_file_path.value().c_str(), sc.record_file_path.value().length());
    if(!sc.n_record_file_blocks.is_null())
        h.update((uint8_t*)&sc.n_record_file_blocks.value(), sizeof(sc.n_record_file_blocks.value()));
    if(!sc.record_file_block_size.is_null())
        h.update((uint8_t*)&sc.record_file_block_size.value(), sizeof(sc.record_file_block_size.value()));
    
    h.finalize();

    return h.get_as_string();
}
