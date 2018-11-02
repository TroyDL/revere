
#include "r_pipe/plugins/rtsp_source/r_rtsp_source.h"
#include "r_utils/r_socket.h"
#include "r_rtsp/r_client_request.h"
#include "r_rtsp/r_client_response.h"
#include "r_rtsp/r_rtp.h"
#include "r_utils/r_byte_ptr.h"
#include <math.h>
#include <limits.h>
#include <algorithm>
#include <memory>

using namespace r_pipe;
using namespace std;
using namespace std::chrono;
using namespace r_utils;
using namespace r_rtsp;
using namespace r_av;

static const int MAX_NUM_REDIRECTS = 5;
static const size_t PACKET_BUFFER_SIZE = 1536;
static const size_t NUM_PACKETS = 2048;

const int NT_UNSPECIFIED = 0;
const int NT_CODED_SLICE_NONIDR_PICTURE = 1;
const int nt_coded_slice_partition_A = 2;
const int nt_coded_slice_partition_B = 3;
const int nt_coded_slice_partition_C = 4;
const int nt_coded_slice_IDR_picture = 5;
const int nt_supp_enhance_info = 6;
const int nt_sequence_param_set = 7;
const int nt_picture_param_set = 8;
const int nt_access_unit_delim = 9;
const int nt_end_of_sequence = 10;
const int nt_end_of_stream = 11;
const int nt_filler_data = 12;
const int nt_sequence_param_set_ext = 13;
const int nt_reserved_14 = 14;
const int nt_reserved_15 = 15;
const int nt_reserved_16 = 16;
const int nt_reserved_17 = 17;
const int nt_reserved_18 = 18;
const int nt_coded_slice_aux_pic = 19;
const int nt_reserved_20 = 20;
const int nt_reserved_21 = 21;
const int nt_reserved_22 = 22;
const int nt_reserved_23 = 23;
const int nt_STAP_A = 24;
const int nt_STAP_B = 25;
const int nt_MTAP_16 = 26;
const int nt_MTAP_24 = 27;
const int nt_FU_A = 28;
const int nt_FU_B = 29;

std::mutex r_rtsp_source::_udpPortLock;

r_rtsp_source::r_rtsp_source( bool preferTCP ) :
    _workerMethod( WM_UNKNOWN ),
    _workerRunning( false ),
    _workerThread(),
    _rtspURL(),
    _riverClient(),
    _requestedStreamType(),
    _sessionInfo(),
    _selectedMD(0),
    _sdp(),
    _rtspControl(),
    _discardFirstFrame( false ),
    _seenFirstMarkerBit( false ),
    _udpRTPReceiver(),
    _preferTCP( preferTCP ),
    _packetPool(),
    _packetCache(),
    _lastSeq(0),
    _needSort(false),
    _packetCacheLock(),
    _markersCached( 0 ),
    _markerlessPushThreshold( NUM_PACKETS / 2 ),
    _packetsCachedSinceLastMarker( 0 ),
    _keepAliveInterval( 30000 ), // milliseconds
    _lastKeepAliveTime( steady_clock::now() ),
    _consumerCond(),
    _consumerCondLock(),
    _params(),
    _lastSequence( 0 ),
    _lastSequenceValid( false ),
    _controlSocketInterleavedKeepAlives( false ),
    _healthy(true)
{
    _packetPool = make_shared<r_pool<vector<uint8_t>>>( NUM_PACKETS, PACKET_BUFFER_SIZE );
}

r_rtsp_source::~r_rtsp_source() throw()
{
    try
    {
        if( _workerRunning )
        {
            _workerRunning = false;
            if( _udpRTPReceiver )
                _udpRTPReceiver->close();
            _workerThread.join();
        }
    }
    catch(exception& ex)
    {
        R_LOG_EXCEPTION(ex);
    }
}

void r_rtsp_source::run()
{
    shared_ptr<r_client_request> playRequest = make_shared<r_client_request>( M_PLAY );
    playRequest->set_uri( _parse_resource_path( _rtspURL ) );
    playRequest->set_header( "Range", "npt=0.000-" );
    playRequest->set_header( "Speed", "1.0" );

    _riverClient->write_request( playRequest );

    shared_ptr<r_client_response> playResponse = _riverClient->read_response();

    if( (playResponse->get_status() < STATUS_OK) || (playResponse->get_status() > 300) )
        R_THROW(("Play failed."));

    _workerRunning = true;
    _workerThread = std::thread( &r_rtsp_source::_entry_point, this );
}

void r_rtsp_source::stop()
{
    try
    {
        if( _workerRunning )
        {
            _workerRunning = false;

            if( _udpRTPReceiver )
                _udpRTPReceiver->close();

            _workerThread.join();

            {
                // wake any thread waiting on a get()...
                unique_lock<mutex> g(_consumerCondLock);
                _consumerCond.notify_one();
            }

            shared_ptr<r_client_request> teardownRequest = make_shared<r_client_request>( M_TEARDOWN );
            teardownRequest->set_uri( _parse_resource_path( _rtspURL ) );

            _riverClient->write_request( teardownRequest );

            if( _workerMethod != WM_RTSP_INTERLEAVED )
            {
                shared_ptr<r_client_response> teardownResponse = _riverClient->read_response();

                if( (teardownResponse->get_status() < STATUS_OK) || (teardownResponse->get_status() > 300) )
                    R_THROW(("TEARDOWN failed."));
            }

            _lastSequenceValid = false;
        }
    }
    catch(exception& ex)
    {
        R_LOG_EXCEPTION(ex);
    }
    catch(...)
    {
        R_LOG_NOTICE("Exception thrown in stop().");
    }
}

bool r_rtsp_source::get( r_packet& pkt )
{
    {
        unique_lock<mutex> g(_consumerCondLock);

        bool doneWaiting = false;
        while( !doneWaiting && _workerRunning )
        {
            {
                unique_lock<mutex> cg( _packetCacheLock );
                if( _markersCached > 0 )
                {
                    doneWaiting = true;
                    continue;
                }
            }

            _consumerCond.wait( g );

            if(!_healthy)
                R_THROW(("Unable to read network packet."));
        }
    }

    if( !_workerRunning )
        return false;

    if( _time_for_rtsp_keep_alive() )
        _send_rtsp_keep_alive( _workerMethod );

    // NOTE: This code is fine for unicast, and rtsp interleaved mode... but for multicast we need to
    // keep waiting and consuming frames until we get our first key frame.

    {
        unique_lock<mutex> cg(_packetCacheLock);
        pkt = _dequeue();
    }

    return true;
}

void r_rtsp_source::set_param( const string& name, const string& val )
{
    if( _params.find( name ) == _params.end() )
        _params.insert( make_pair( name, val ) );
    else _params[name] = val;
}

void r_rtsp_source::commit_params()
{
    if( _params.find( "prefer_tcp" ) != _params.end() )
    {
        auto pref = r_string::to_lower(_params["prefer_tcp"]);
        if( pref == "1" || pref == "true" || pref == "yes" )
            _preferTCP = true;
        else _preferTCP = false;
    }

    _requestedStreamType = _params["type"];

    _rtspURL = _params["rtsp_url"];

    _seenFirstMarkerBit = false;
    _discardFirstFrame = false;

    shared_ptr<r_client_response> optionsResponse;
    string rtspServerIP;
    int rtspServerPort = -1;
    int redirectCount = 0;

    while (redirectCount < MAX_NUM_REDIRECTS)
    {
        _parse_rtsp_server_ip_and_port( _rtspURL, rtspServerIP, rtspServerPort );

        _riverClient = make_shared<r_client_connection>( rtspServerIP, rtspServerPort );

        _riverClient->connect();

        shared_ptr<r_client_request> optionsRequest = make_shared<r_client_request>( M_OPTIONS );
        optionsRequest->set_uri( _parse_resource_path( _rtspURL ) );

        _riverClient->write_request( optionsRequest );

        shared_ptr<r_client_response> response = _riverClient->read_response();

        if( response->get_status() == STATUS_MOVED_TEMPORARILY )
        {
            string redirectURL;
            if(!response->get_header("Location", redirectURL))
                R_THROW(( "Redirect response missing Location header." ));

            _rtspURL = redirectURL;
            redirectCount++;
            R_LOG_INFO("RTSP OPTIONS request redirected to %s", _rtspURL.c_str());
        }
        else
        {
            optionsResponse = response;
            break;
        }
    }

    if (redirectCount > 5)
        R_THROW(( "RTSP server has redirected the OPTIONS request more than the max times allowed." ));

    string publicHeaderValue;
    if( !optionsResponse->get_header( "Public", publicHeaderValue ) )
        R_THROW(( "Unable to retrieve server Options." ));

    if( !r_string::contains(publicHeaderValue, "DESCRIBE" ) ||
        !r_string::contains(publicHeaderValue, "SETUP" ) ||
        !r_string::contains(publicHeaderValue, "TEARDOWN" ) ||
        !r_string::contains(publicHeaderValue, "PLAY" ) )
    {
        R_THROW(( "RTSP server does not support minimum required methods(DESCRIBE, SETUP, TEARDOWN, PLAY)." ));
    }

    shared_ptr<r_client_request> describeRequest = make_shared<r_client_request>( M_DESCRIBE );
    describeRequest->set_uri( _parse_resource_path( _rtspURL ) );

    _riverClient->write_request( describeRequest );

    shared_ptr<r_client_response> describeResponse = _riverClient->read_response();

    string describeBody = describeResponse->get_body_as_string();

    _sessionInfo = r_rtsp::parse_sdp(describeBody);

    for(size_t i = 0; i < _sessionInfo.media_descriptions.size(); ++i)
    {
        if(_requestedStreamType == _sessionInfo.media_descriptions[i].type)
            _selectedMD = i;
    }

    _sdp = describeBody;

    _rtspControl = _parse_resource_path( _rtspURL ) + "/" + _sessionInfo.media_descriptions[_selectedMD].control;

    if(_sessionInfo.media_descriptions[_selectedMD].codec != "H264" &&
       _sessionInfo.media_descriptions[_selectedMD].codec != "MP4V" &&
       _sessionInfo.media_descriptions[_selectedMD].codec != "MP4V-ES" &&
       _sessionInfo.media_descriptions[_selectedMD].codec != "PCMU")
        R_THROW(("Unknown codec."));

    if( r_socket_address::is_multicast( _sessionInfo.media_descriptions[_selectedMD].conn.connection_address ) )
    {
        _discardFirstFrame = true;

        shared_ptr<r_client_request> setupRequest = make_shared<r_client_request>( M_SETUP );

        setupRequest->set_uri( _rtspControl );

        int rtcpPort = _sessionInfo.media_descriptions[_selectedMD].port + 1;

        setupRequest->set_header( "Transport",
                                  r_string::format( "RTP/AVP;multicast;client_port=%d-%d",
                                                    _sessionInfo.media_descriptions[_selectedMD].port,
                                                    rtcpPort ) );

        _riverClient->write_request( setupRequest );

        shared_ptr<r_client_response> setupResponse = _riverClient->read_response();

        auto status = setupResponse->get_status();

        if( (status < STATUS_OK) || (status > 300) )
            R_THROW(("RTSP setup failed, returned %d status.", status));

        R_LOG_INFO( "r_rtsp_source - binding to address(%s), port(%d)",
                     _sessionInfo.media_descriptions[_selectedMD].conn.connection_address.c_str(),
                     _sessionInfo.media_descriptions[_selectedMD].port);

        _udpRTPReceiver = make_shared<r_udp_receiver>( _sessionInfo.media_descriptions[_selectedMD].port, (1024*1024), _sessionInfo.media_descriptions[_selectedMD].conn.connection_address );

        _workerMethod = WM_UDP;
    }
    else
    {
        if( _preferTCP )
        {
            if( !_tcp_setup() )
            {
                if( !_udp_setup() )
                {
                    R_THROW(( "Unable to find a transport that worked." ));
                }
                else _workerMethod = WM_UDP;
            }
            else _workerMethod = WM_RTSP_INTERLEAVED;
        }
        else
        {
            if( !_udp_setup() )
            {
                if( !_tcp_setup() )
                {
                    R_THROW(( "Unable to find a transport that worked." ));
                }
                else _workerMethod = WM_RTSP_INTERLEAVED;
            }
            else _workerMethod = WM_UDP;
        }
    }
}

bool r_rtsp_source::_tcp_setup()
{
    shared_ptr<r_client_request> interleavedSetupRequest = make_shared<r_client_request>( M_SETUP );

    interleavedSetupRequest->set_uri( _rtspControl );

    // For now, we are only doing interleaved mode.
    interleavedSetupRequest->set_header( "Transport",
                                         "RTP/AVP/TCP;unicast;interleaved=0-1" );

    _riverClient->write_request( interleavedSetupRequest );

    shared_ptr<r_client_response> interleavedSetupResponse = _riverClient->read_response();

    // Unsupported transport
    if( interleavedSetupResponse->get_status() == 461 )
        return false;

    if( interleavedSetupResponse->get_status() < STATUS_OK || interleavedSetupResponse->get_status() > 300 )
        R_THROW(( "SETUP failed." ));

    return true;
}

bool r_rtsp_source::_udp_setup()
{
    // We need to be able to bind a UDP receiver ephemerally.

    string rtspServerIP;
    int rtspServerPort = -1;
    _parse_rtsp_server_ip_and_port( _rtspURL, rtspServerIP, rtspServerPort );

    int boundPort = 0;
    bool foundFreePort = false;
    int maxPortProbeAttempts = 100;

    shared_ptr<r_udp_receiver> udpRTCPReceiver;

    {
        //@Note this guard and scope are used to prevent two of these sources from
        //grabbing the same ports simultaneously, this is a rare problem but causes annoying
        //hard to trace issues.
        unique_lock<mutex> g(_udpPortLock);
        while( !foundFreePort )
        {
            try
            {
                maxPortProbeAttempts--;

                _udpRTPReceiver = make_shared<r_udp_receiver>();

                boundPort = _udpRTPReceiver->get_bound_port();

                if( (boundPort % 2) != 0 )
                    ++boundPort;

                _udpRTPReceiver = make_shared<r_udp_receiver>( boundPort, (1024*1024));

                udpRTCPReceiver = make_shared<r_udp_receiver>( boundPort + 1, (1024*1024));

                foundFreePort = true;
            }
            catch( r_exception& )
            {
                if( maxPortProbeAttempts <= 0 )
                    R_THROW(( "Unable to discover free consecutive ports for RTP." ));
            }
        }
    }

    R_LOG_INFO( "r_rtsp_source::_udp_setup() (A) Binding to address(%s), rtp port(%d), rtcp port(%d) this(%p)",
                 _sessionInfo.media_descriptions[_selectedMD].conn.connection_address.c_str(),
                 _udpRTPReceiver->get_bound_port(),
                 udpRTCPReceiver->get_bound_port(),
                 this );

    rtspServerIP = _reconcile_rtsp_server_ip(_sessionInfo.media_descriptions[_selectedMD].conn.connection_address,
                                             _sessionInfo.origin.unicast_address,
                                             rtspServerIP);
    _udpRTPReceiver->connect( rtspServerIP, 0 );

    shared_ptr<r_client_request> unicastSetupRequest = make_shared<r_client_request>( M_SETUP );

    unicastSetupRequest->set_uri( _rtspControl );

    string transport = r_string::format( "RTP/AVP;unicast;client_port=%d-%d",
                                         _udpRTPReceiver->get_bound_port(),
                                         udpRTCPReceiver->get_bound_port() );

    unicastSetupRequest->set_header( "Transport",
                                     transport );

    _riverClient->write_request( unicastSetupRequest );

    shared_ptr<r_client_response> unicastSetupResponse = _riverClient->read_response();

    // Unsupported transport
    if( unicastSetupResponse->get_status() == 461 )
        return false;

    if( unicastSetupResponse->get_status() < STATUS_OK || unicastSetupResponse->get_status() > 300 )
        R_THROW(( "SETUP failed." ));

    return true;
}

string r_rtsp_source::get_sdp() const
{
    return _sdp;
}

string r_rtsp_source::get_encoded_sps() const
{
    return _sessionInfo.media_descriptions[_selectedMD].encoded_sps;
}

string r_rtsp_source::get_encoded_pps() const
{
    return _sessionInfo.media_descriptions[_selectedMD].encoded_pps;
}

vector<uint8_t> r_rtsp_source::get_sps() const
{
    if( _sessionInfo.media_descriptions[_selectedMD].encoded_sps.empty() )
        return vector<uint8_t>();

    return r_string::from_base64(_sessionInfo.media_descriptions[_selectedMD].encoded_sps);
}

vector<uint8_t> r_rtsp_source::get_pps() const
{
    if( _sessionInfo.media_descriptions[_selectedMD].encoded_pps.empty() )
        return vector<uint8_t>();

    return r_string::from_base64(_sessionInfo.media_descriptions[_selectedMD].encoded_pps);
}

vector<uint8_t> r_rtsp_source::get_extra_data() const
{
    uint8_t code[] = { 0, 0, 0, 1 };

    auto sps = get_sps();
    auto spsSize = sps.size();
    auto pps = get_pps();
    auto ppsSize = pps.size();

    auto totalSize = 4 + spsSize + ((ppsSize > 0) ? 4 + ppsSize : 0);

    auto extra = vector<uint8_t>(totalSize);
    
    uint8_t* writer = &extra[0];

    memcpy(writer, code, 4);
    writer += 4;
    memcpy(writer, &sps[0], spsSize);
    writer += spsSize;

    if(ppsSize > 0)
    {
        memcpy(writer, code, 4);
        writer += 4;
        memcpy(writer, &pps[0], ppsSize);
    }

    return extra;
}

void r_rtsp_source::_parse_rtsp_server_ip_and_port( const string& rtspURL,
                                                  string& ip,
                                                  int& port ) const
{
   size_t protocolDelim = rtspURL.find( "://" );
   if( protocolDelim == std::string::npos )
      R_THROW(( "RTSP URL(%s) doesn't start with protocol delimeter", rtspURL.c_str() ));

   size_t length = rtspURL.find( "/", protocolDelim + 3 );
   string temp;
   if( length != std::string::npos )
      temp = rtspURL.substr( protocolDelim + 3, length - (protocolDelim + 3) );
   else//If length is std::string::npos it will go to end of string.
      temp = rtspURL.substr( protocolDelim + 3, length );

   //Reverse find ':' this is where our port is.
   size_t bracket =  temp.rfind( "]", length );
   size_t colon = temp.rfind( ":" );
   if( bracket != std::string::npos && colon < bracket )//ipv6
   {
      port = 554;
      ip = temp;
   }
   else
   {
      if( colon != std::string::npos )
         port = r_string::s_to_int(temp.substr( colon+1, length ));
      else port = 554;
      ip = temp.substr( 0, colon );
   }
   r_socket_address test( port, ip );
}

string r_rtsp_source::_parse_resource_path( const string& rtspURL ) const
{
    vector<string> p1 = r_string::split(rtspURL, "://" );

    if(p1.size() == 2)
        return "/" + p1[1].substr( (p1[1].find("/")+1) ); // wont this fail for paths with multiple slashes /foo/bar/baz?

    R_THROW(( "Detected malformed RTSP URL(%s).", rtspURL.c_str() ));
}

string r_rtsp_source::_parse_resource( const string& rtspURL ) const
{
    vector<string> p1 = r_string::split(rtspURL, "://" );

    if(p1.size() == 2)
        return p1[1].substr( (p1[1].rfind("/")+1) );

    R_THROW(( "Detected malformed RTSP URL(%s).", rtspURL.c_str() ));
}

string r_rtsp_source::_reconcile_rtsp_server_ip( const string& sdpCAddr,
                                                 const string& sdpOAddr,
                                                 const string& rtspAddr ) const
{
    if(sdpCAddr != "0.0.0.0")
        return sdpCAddr;

    if(sdpOAddr != "0.0.0.0")
    {
        if(sdpOAddr != rtspAddr)
            return sdpOAddr;
    }

    return rtspAddr;
}

void r_rtsp_source::_entry_point()
{
    while( _workerRunning )
    {
        try
        {
            if( _workerMethod == WM_RTSP_INTERLEAVED )
                _wm_rtsp_interleaved();
            else if( _workerMethod == WM_UDP )
                _wm_udp();
        }
        catch( std::exception& ex )
        {
            _healthy = false;

            R_LOG_NOTICE("Exception caught while attempting to read network packet...");

            {
                // wake any thread waiting on a get()...
                unique_lock<mutex> g(_consumerCondLock);
                _consumerCond.notify_one();
            }
        }
    }
}

void r_rtsp_source::_wm_rtsp_interleaved()
{
    auto buffer = _packetPool->get();

    uint8_t channel = 255;
    bool gotPacket = false;
    bool hasMarker = false;

    gotPacket = _riverClient->receive_interleaved_packet( channel, *buffer );

//        if( channel != 0 || !gotPacket )
//            return;

//        do
//        {
//            gotPacket = _riverClient->receive_interleaved_packet( channel, buffer );
//        }
//        while( (channel != 0) && gotPacket );

    if( !_workerRunning )
        return;

    if( !gotPacket )
        R_THROW(( "Failed to read packet." ));

    if( gotPacket && (channel == 0) )
        hasMarker = r_rtsp::rtp_parse_marker( &(*buffer)[0], buffer->size() );

// Below is a standalone check for packet sequence retrieval... 
#if 0
    static vector<uint16_t> sequenceBuffer(1024);
    static int seqCheckCountdown = 1000;
    uint16_t seq = river::rtp_parse_sequence( buffer->map_data() );
    size_t size = sequenceBuffer.size();
    if( size == 1024 )
        sequenceBuffer.erase(sequenceBuffer.begin());
    sequenceBuffer.push_back(seq);

    --seqCheckCountdown;
    if( seqCheckCountdown == 0 )
    {
        R_LOG_NOTICE("==> SEQUENCE CHECK size = %lu",size);
        seqCheckCountdown = 1000;

        sort( sequenceBuffer.begin(), sequenceBuffer.end(), []( uint16_t a, uint16_t b )->bool{ return a < b; } );
        sequenceBuffer.erase( unique( sequenceBuffer.begin(), sequenceBuffer.end() ), sequenceBuffer.end() );

        uint16_t lastSeq = 0;
        bool lastSeqValid = false;

        for( auto s : sequenceBuffer )
        {
            if( lastSeqValid )
            {
                if( s != (uint16_t)(lastSeq + 1))
                    R_LOG_NOTICE("SEQUENCE ERROR! Expected %d, Got %d\n",(uint16_t)(lastSeq+1),s);
            }

            lastSeq = s;
            lastSeqValid = true;
        }

        sequenceBuffer.clear();
        sequenceBuffer.reserve(1024);

        R_LOG_NOTICE("<== SEQUENCE CHECK");
    }
#endif

    {
        unique_lock<mutex> g(_packetCacheLock);
        _needSort = false;
        _packetCache.push_back( buffer );
        if( hasMarker )
            _markersCached++;
    }

    ++_packetsCachedSinceLastMarker;

    if( hasMarker || (_packetsCachedSinceLastMarker > _markerlessPushThreshold) )
    {
        {
            unique_lock<mutex> cg(_consumerCondLock);
            _consumerCond.notify_one();
        }

        _packetsCachedSinceLastMarker = 0;
    }
}

void r_rtsp_source::_wm_udp()
{
    auto buffer = _packetPool->get();

    bool gotPacket = false;
    bool hasMarker = false;

    int port = 0;
    gotPacket = _udpRTPReceiver->receive( port, *buffer, 5000 );

    if( !gotPacket )
        R_THROW(("Unable to read packet."));

    // Note: we write the buffer even if !gotPacket... this is on purpose because the buffer
    // will never get back into the pool.

    hasMarker = r_rtsp::rtp_parse_marker( &(*buffer)[0], buffer->size() );

    {
        unique_lock<mutex> g(_packetCacheLock);
        uint16_t seq = rtp_parse_sequence(&(*buffer)[0], buffer->size());
        if(seq < _lastSeq)
            _needSort = true;
        _packetCache.push_back( buffer );
        if( hasMarker )
            _markersCached++;
    }

    _packetsCachedSinceLastMarker++;

    if( hasMarker || (_packetsCachedSinceLastMarker > _markerlessPushThreshold) )
    {
        {
            unique_lock<mutex> cg(_consumerCondLock);
            _consumerCond.notify_one();
        }

        _packetsCachedSinceLastMarker = 0;
    }
}

bool r_rtsp_source::_time_for_rtsp_keep_alive() const
{
    return (duration_cast<milliseconds>(steady_clock::now()-_lastKeepAliveTime)>_keepAliveInterval);
}

void r_rtsp_source::_send_rtsp_keep_alive( WORKER_METHOD workerMethod )
{
    auto now = steady_clock::now();

    if( workerMethod == WM_RTSP_INTERLEAVED )
    {
        if( _controlSocketInterleavedKeepAlives == false )
        {
            // In interleaved mode we do keep alives on a NEW RTSP connection... This is to avoid corrupting
            // the video stream with an RTSP success message.
            string rtspServerIP;
            int rtspServerPort = -1;
            _parse_rtsp_server_ip_and_port( _rtspURL, rtspServerIP, rtspServerPort );

            shared_ptr<r_client_connection> keepAliveConnection =
                make_shared<r_client_connection>( rtspServerIP, rtspServerPort );

            keepAliveConnection->set_session_id( _riverClient->get_session_id() );
            keepAliveConnection->connect();

            shared_ptr<r_client_request> keepAliveRequest = make_shared<r_client_request>( M_GET_PARAM );

            keepAliveRequest->set_uri( _parse_resource_path( _rtspURL ) );
            keepAliveConnection->write_request( keepAliveRequest );

            shared_ptr<r_client_response> keepAliveResponse = keepAliveConnection->read_response();

            if( keepAliveResponse->get_status() != STATUS_OK )
            {
                _controlSocketInterleavedKeepAlives = true;
                R_LOG_WARNING( "RTSP keep alive returned unsuccessful status. this(%p)", this );
            }
            else
                _lastKeepAliveTime = now;

            R_LOG_NOTICE("RTSP_INTERLEAVED KEEP ALIVE!");
        }

        if( _controlSocketInterleavedKeepAlives == true )
        {
            shared_ptr<r_client_request> keepAliveRequest = make_shared<r_client_request>( M_GET_PARAM );
            keepAliveRequest->set_uri( _parse_resource_path( _rtspURL ) );
            _riverClient->write_request( keepAliveRequest );

            _lastKeepAliveTime = now;

            R_LOG_NOTICE("RTSP_INTERLEAVED KEEP ALIVE!");
        }
    }
    else
    {
        shared_ptr<r_client_request> keepAliveRequest = make_shared<r_client_request>( M_GET_PARAM );

        keepAliveRequest->set_uri( _parse_resource_path( _rtspURL ) );
        _riverClient->write_request( keepAliveRequest );

        shared_ptr<r_client_response> keepAliveResponse = _riverClient->read_response();

        if( keepAliveResponse->get_status() != STATUS_OK )
            R_LOG_WARNING( "RTSP keep alive returned unsuccessful status. this(%p)", this );
        else
            _lastKeepAliveTime = now;
    }
}

static bool pcmp( const shared_ptr<vector<uint8_t>>& a, const shared_ptr<vector<uint8_t>>& b )
{
    uint16_t seq_a = r_rtsp::rtp_parse_sequence( &(*a)[0], a->size() );
    uint16_t seq_b = r_rtsp::rtp_parse_sequence( &(*b)[0], b->size() );
    // Higher numbers come after lower numbers, except for the uint16_t rollover case.
    return ( (uint16_t)(seq_a - seq_b) > 32767 );
}

r_packet r_rtsp_source::_dequeue()
{
    if(_needSort)
    {
        sort( _packetCache.begin(), _packetCache.end(), pcmp );
        _needSort = false;
    }
    _packetCache.erase( unique( _packetCache.begin(), _packetCache.end() ), _packetCache.end() );

    uint8_t h264_sig[] = { 0, 0, 0, 1 };

    auto marker = _packetCache.begin(), end = _packetCache.end();
    while( !r_rtsp::rtp_parse_marker( &(*(*marker))[0], (*marker)->size() ) && (marker != end) )
        ++marker;

    if( marker == end )
        R_THROW(("No marker in cache!"));

    auto frameEnd = (marker + 1);

    size_t sizeAccum = 0;
    for( auto i = _packetCache.begin(); i != frameEnd; ++i )
        sizeAccum += (*i)->size() + 4; // we allocate space for every packet to have a start code

    auto pkt = r_packet( sizeAccum );

    size_t dataLength = 0;
    uint8_t* output = pkt.map();
    uint8_t* writer = output;

    bool foundIDR = false;

    for( auto i = _packetCache.begin(); i != frameEnd; ++i )
    {
        pkt.set_pts( r_rtsp::rtp_parse_timestamp( &(*(*i))[0], (*i)->size() ) );

        uint16_t seq = r_rtsp::rtp_parse_sequence( &(*(*i))[0], (*i)->size() );

        if( _lastSequenceValid )
        {
            if( seq != (uint16_t)(_lastSequence + 1) )
                R_LOG_NOTICE("r_rtsp_source SEQ ERROR: expected( %u ), actual( %u )    ", (uint16_t)(_lastSequence + 1), seq );
        }

        _lastSequence = seq;
        _lastSequenceValid = true;

        // the below code should be converted over to using payload directly as a r_byte_ptr_rw.
        auto payload = r_rtsp::rtp_parse_payload( &(*(*i))[0], (*i)->size() );
        uint8_t* packetPayload = payload.first;
        size_t packetPayloadSize = payload.second;

        if( _sessionInfo.media_descriptions[_selectedMD].codec != "H264" ) // if it's not H264, just copy the payload into the output
        {
            memcpy(writer, packetPayload, packetPayloadSize);
            writer += packetPayloadSize;
            dataLength += packetPayloadSize;
        }
        else
        {
            // H264 requires depackitization and start code injection...
            uint8_t nalHeader = packetPayload[0];
            int headerLength = 1;
            uint8_t nalUnitType = nalHeader & 0x1F;

            switch( nalUnitType )
            {
            case nt_FU_A:
            case nt_FU_B:
            {
                uint8_t fuHeader = packetPayload[1];
                bool startBit = (fuHeader & 0x80) == 0x80;

                if(startBit)
                {
                    uint8_t actualNalHeader = (nalHeader & 0xE0) | (fuHeader & 0x1F);

                    if( (actualNalHeader & 0x1f) == nt_coded_slice_IDR_picture )
                        foundIDR = true;

                    memcpy(writer, h264_sig, sizeof(h264_sig));
                    writer += sizeof(h264_sig);
                    dataLength += sizeof(h264_sig);

                    memcpy(writer, &actualNalHeader, sizeof(actualNalHeader));
                    writer += sizeof(actualNalHeader);
                    dataLength += sizeof(actualNalHeader);

                    memcpy(writer, packetPayload + 2, packetPayloadSize - 2);
                    writer += packetPayloadSize - 2;
                    dataLength += packetPayloadSize - 2;
                }
                else
                {
                    // We skip the nal header and FU header bytes
                    memcpy(writer, packetPayload + 2, packetPayloadSize - 2);
                    writer += packetPayloadSize - 2;
                    dataLength += packetPayloadSize - 2;
                }

                break;
            }

            case nt_STAP_B:
                headerLength += 2;

            case nt_STAP_A:
            {
                // skip headers
                packetPayload += headerLength;
                packetPayloadSize -= headerLength;

                while(packetPayloadSize > 2)
                {
                    //NAL Unit size
                    size_t nalUnitSize = (packetPayload[0] << 8) | packetPayload[1];
                    packetPayload += 2;
                    packetPayloadSize -= 2;

                    if(nalUnitSize > packetPayloadSize)
                        nalUnitSize = packetPayloadSize;

                    memcpy(writer, h264_sig, sizeof(h264_sig));
                    writer += sizeof(h264_sig);
                    dataLength += sizeof(h264_sig);

                    memcpy(writer, packetPayload, nalUnitSize);
                    writer += nalUnitSize;
                    dataLength += nalUnitSize;

                    // update payload pointer and packetPayloadSize
                    packetPayload += nalUnitSize;
                    packetPayloadSize -= nalUnitSize;
                }
            }
            break;

            default:
                if(nalUnitType >= NT_CODED_SLICE_NONIDR_PICTURE &&
                   nalUnitType <= nt_reserved_23)
                {
                    if( nalUnitType == nt_coded_slice_IDR_picture )
                        foundIDR = true;

                    memcpy(writer, h264_sig, sizeof(h264_sig));
                    writer += sizeof(h264_sig);
                    dataLength += sizeof(h264_sig);

                    memcpy(writer, packetPayload, packetPayloadSize);
                    writer += packetPayloadSize;
                    dataLength += packetPayloadSize;
                }
                else
                {
                    // Copy the data into the frame buffer
                    memcpy(writer, packetPayload, packetPayloadSize);
                    writer += packetPayloadSize;
                    dataLength += packetPayloadSize;
                }
                break;
            };
        }
    }

    pkt.set_data_size( dataLength );

    bool keyFrame = false;

    if( _sessionInfo.media_descriptions[_selectedMD].type == "video" )
    {
        if( _sessionInfo.media_descriptions[_selectedMD].codec == "H264" )
        {
            if( foundIDR )
                keyFrame = true;
        }
        else if( _sessionInfo.media_descriptions[_selectedMD].codec == "MP4V" )
        {
            keyFrame = _contains_mpeg4_key_frame(output, dataLength );
        }
        else R_THROW(("Unsupported codec id."));
    }
    else if( _sessionInfo.media_descriptions[_selectedMD].type == "audio" )
    {
        keyFrame = true;
    }
    else if( _sessionInfo.media_descriptions[_selectedMD].type == "metadata" )
    {
        keyFrame = true;
    }

    pkt.set_key( keyFrame );
    pkt.set_sdp(_sdp);

    _packetCache.erase( _packetCache.begin(), frameEnd );

    _markersCached--;

    return pkt;
}

static const unsigned char* boyer_moore_search( const unsigned char* haystack, size_t hlen,
                                                const unsigned char* needle, size_t nlen )
{
    size_t scan = 0;
    size_t bad_char_skip[UCHAR_MAX + 1]; /* Officially called:
                                          * bad character shift */

    /* Sanity checks on the parameters */
    if (nlen <= 0 || !haystack || !needle)
        return nullptr;

    /* ---- Preprocess ---- */
    /* Initialize the table to default value */
    /* When a character is encountered that does not occur
     * in the needle, we can safely skip ahead for the whole
     * length of the needle.
     */
    for (scan = 0; scan <= UCHAR_MAX; scan = scan + 1)
        bad_char_skip[scan] = nlen;

    /* C arrays have the first byte at [0], therefore:
     * [nlen - 1] is the last byte of the array. */
    size_t last = nlen - 1;

    /* Then populate it with the analysis of the needle */
    for (scan = 0; scan < last; scan = scan + 1)
        bad_char_skip[needle[scan]] = last - scan;

    /* ---- Do the matching ---- */

    /* Search the haystack, while the needle can still be within it. */
    while (hlen >= nlen)
    {
        /* scan from the end of the needle */
        for (scan = last; haystack[scan] == needle[scan]; scan = scan - 1)
            if (scan == 0) /* If the first byte matches, we've found it. */
                return haystack;

        /* otherwise, we need to skip some bytes and start again.
           Note that here we are getting the skip value based on the last byte
           of needle, no matter where we didn't match. So if needle is: "abcd"
           then we are skipping based on 'd' and that value will be 4, and
           for "abcdd" we again skip on 'd' but the value will be only 1.
           The alternative of pretending that the mismatched character was
           the last character is slower in the normal case (E.g. finding
           "abcd" in "...azcd..." gives 4 by using 'd' but only
           4-2==2 using 'z'. */
        hlen     -= bad_char_skip[haystack[last]];
        haystack += bad_char_skip[haystack[last]];
    }

    return nullptr;
}

bool r_rtsp_source::_contains_mpeg4_key_frame( uint8_t* buf, size_t size ) const
{
    uint8_t mpeg4_sig[] = { 0, 0, 1 };

    const uint8_t* pos = (uint8_t*)buf;
    const uint8_t* end = pos + size;

    while( (pos = boyer_moore_search( pos, (end-pos), mpeg4_sig, sizeof(mpeg4_sig) ) ) )
    {
        pos += 3;
        if( *pos == 0x20 )
            return true;
    }

    return false;
}
