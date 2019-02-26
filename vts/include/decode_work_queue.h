
#ifndef __decode_work_queue_h
#define __decode_work_queue_h

#include "r_utils/r_actor.h"

enum video_chunk_type
{
    video_chunk_type_key,
    video_chunk_type_vss_result
};

struct video_chunk
{
    video_chunk_type type;
    std::vector<uint8_t> buffer;
};

#endif
