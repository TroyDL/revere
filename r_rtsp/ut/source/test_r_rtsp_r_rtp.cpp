
#include "test_r_rtsp_r_rtp.h"
#include "r_rtsp/r_rtp.h"
#include "r_utils/r_byte_ptr.h"
#include "r_utils/r_socket.h"
#include <thread>

#include "rtp_stream.cpp"

using namespace std;
using namespace r_utils;
using namespace r_rtsp;

REGISTER_TEST_FIXTURE(test_r_rtsp_r_rtp);

void test_r_rtsp_r_rtp::setup()
{
}

void test_r_rtsp_r_rtp::teardown()
{
}

void test_r_rtsp_r_rtp::test_parse_sequence()
{
    uint32_t rtpPort = 0;
    uint8_t* reader = RTP_STREAM;
    uint32_t bytesToRead = RTP_STREAM_SIZE;

    uint16_t cmpSeq = 36397;

    while( bytesToRead > 0 )
    {
        uint32_t packetSize = r_networking::r_ntohl( *(uint32_t*)reader );
        reader+=4;
        bytesToRead-=4;

        uint32_t port = r_networking::r_ntohl( *(uint32_t*)reader );
        reader+=4;
        bytesToRead-=4;

        if( rtpPort == 0 )
            rtpPort = port;

        if( port == rtpPort )
        {
            uint16_t seq = r_rtsp::rtp_parse_sequence( reader, packetSize );
            RTF_ASSERT( seq == cmpSeq );
            cmpSeq++;
        }

        reader+=packetSize;
        bytesToRead-=packetSize;
    }
}

void test_r_rtsp_r_rtp::test_parse_marker()
{
    uint32_t rtpPort = 0;
    uint8_t* reader = RTP_STREAM;
    uint32_t bytesToRead = RTP_STREAM_SIZE;
    size_t numMarkers = 0;
    size_t numPackets = 0;
    while( bytesToRead > 0 )
    {
        uint32_t packetSize = r_networking::r_ntohl( *(uint32_t*)reader );
        reader+=4;
        bytesToRead-=4;

        uint32_t port = r_networking::r_ntohl( *(uint32_t*)reader );
        reader+=4;
        bytesToRead-=4;

        if( rtpPort == 0 )
            rtpPort = port;

        if( port == rtpPort )
        {
            if( r_rtsp::rtp_parse_marker( reader, packetSize ) )
                numMarkers++;
            else numPackets++;
        }

        reader+=packetSize;
        bytesToRead-=packetSize;
    }

    RTF_ASSERT( numMarkers == 50 );
    RTF_ASSERT( numPackets == 949 );
}

void test_r_rtsp_r_rtp::test_parse_timestamp()
{
    uint32_t rtpPort = 0;
    uint8_t* reader = RTP_STREAM;
    uint32_t bytesToRead = RTP_STREAM_SIZE;
    uint32_t lastTS = 0;
    bool lastTSValid = false;
    while( bytesToRead > 0 )
    {
        uint32_t packetSize = r_networking::r_ntohl( *(uint32_t*)reader );
        reader+=4;
        bytesToRead-=4;

        uint32_t port = r_networking::r_ntohl( *(uint32_t*)reader );
        reader+=4;
        bytesToRead-=4;

        if( rtpPort == 0 )
            rtpPort = port;

        if( port == rtpPort )
        {
            uint32_t ts = r_rtsp::rtp_parse_timestamp( reader, packetSize );

            if( lastTSValid && lastTS != ts )
            {
                RTF_ASSERT( ts > lastTS );
                RTF_ASSERT( (ts - lastTS) >= 5995 && (ts - lastTS) <= 6010 );
            }

            lastTSValid = true;
            lastTS = ts;
        }

        reader+=packetSize;
        bytesToRead-=packetSize;
    }
}

void test_r_rtsp_r_rtp::test_parse_payload()
{
    uint32_t rtpPort = 0;
    uint8_t* reader = RTP_STREAM;
    uint32_t bytesToRead = RTP_STREAM_SIZE;
    while( bytesToRead > 0 )
    {
        uint32_t packetSize = r_networking::r_ntohl( *(uint32_t*)reader );
        reader+=4;
        bytesToRead-=4;

        uint32_t port = r_networking::r_ntohl( *(uint32_t*)reader );
        reader+=4;
        bytesToRead-=4;

        if( rtpPort == 0 )
            rtpPort = port;

        if( port == rtpPort )
        {
            auto payload = r_rtsp::rtp_parse_payload( reader, packetSize );

            RTF_ASSERT( payload.second < 1500 ); // packets must be smaller than the MTU
        }

        reader+=packetSize;
        bytesToRead-=packetSize;
    }
}
