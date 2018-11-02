
#ifndef _r_rtsp_rtp_h
#define _r_rtsp_rtp_h

#include <memory>

namespace r_rtsp
{

uint16_t rtp_parse_sequence( uint8_t* p, size_t size );

bool rtp_parse_marker( uint8_t* p, size_t size );

uint32_t rtp_parse_timestamp( uint8_t* p, size_t size );

std::pair<uint8_t*, size_t> rtp_parse_payload( uint8_t* p, size_t size );

}

#endif
