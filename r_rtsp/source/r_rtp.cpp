
#include "r_rtsp/r_rtp.h"
#include "r_utils/r_socket.h"

using namespace r_utils;
using namespace r_rtsp;
using namespace std;

uint16_t r_rtsp::rtp_parse_sequence( uint8_t* p, size_t size )
{
    p += 2;

    uint16_t sequenceNumber = r_networking::r_ntohs( *(uint16_t*)p );

    return sequenceNumber;
}

bool r_rtsp::rtp_parse_marker( uint8_t* p, size_t size )
{
    p += 1;
    const uint8_t MARKER_MASK = 0x80;
    uint8_t byte = *p;
    return (byte & MARKER_MASK) > 0;
}

uint32_t r_rtsp::rtp_parse_timestamp( uint8_t* p, size_t size )
{
    p += 4;

    uint32_t ts = r_networking::r_ntohl( *(uint32_t*)p );

    return ts;
}

pair<uint8_t*, size_t> r_rtsp::rtp_parse_payload( uint8_t* p, size_t size )
{
    auto originalP = p;

    const uint8_t EXT_MASK = 0x10;
    const uint8_t CSRC_COUNT_MASK = 0xF;

    uint8_t firstByte = *p;

    uint32_t extension = firstByte & EXT_MASK;
    uint32_t csrcCount = firstByte & CSRC_COUNT_MASK;

    p += 12 + (csrcCount * sizeof(uint32_t));

    if( extension )
    {
        uint16_t shortVal = *(uint16_t*)p;
        uint16_t numExtWords = r_networking::r_ntohs(shortVal);
        p += 2 + (numExtWords * sizeof(uint32_t));
    }

    return make_pair((uint8_t*)p, (size_t)(size - (p-originalP)));
}
