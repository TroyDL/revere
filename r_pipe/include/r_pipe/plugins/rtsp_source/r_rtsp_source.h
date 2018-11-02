
#ifndef __r_rtsp_r_rtsp_source_h
#define __r_rtsp_r_rtsp_source_h

#include "r_pipe/r_source.h"
#include "r_utils/r_string.h"
#include "r_utils/r_udp_receiver.h"
#include "r_utils/r_pool.h"
#include "r_rtsp/r_client_connection.h"
#include "r_rtsp/r_sdp.h"
#include "r_av/r_packet.h"
#include <thread>
#include <mutex>
#include <deque>
#include <condition_variable>
#include <list>
#include <memory>
#include <vector>

class test_r_rtsp_r_rtsp_source;

class r_rtsp_source : public r_pipe::r_source
{
    friend class ::test_r_rtsp_r_rtsp_source;

public:
    enum WORKER_METHOD
    {
        WM_UNKNOWN = 1,
        WM_RTSP_INTERLEAVED = 2,
        WM_UDP = 3
    };

    enum STREAM_TYPE
    {
        ST_UNKNOWN,
        ST_VIDEO,
        ST_AUDIO,
        ST_METADATA
    };

    enum CODEC_TYPE
    {
        CT_UNKNOWN,
        CT_H264,
        CT_MPEG4,
        CT_PCMU,
        CT_METADATA
    };

    r_rtsp_source( bool preferTCP = true );
    virtual ~r_rtsp_source() throw();

    virtual void run();
    virtual void stop();
    virtual bool get( r_av::r_packet& pkt );
    virtual void set_param( const std::string& name, const std::string& val );
    virtual void commit_params();

    std::string get_sdp() const;

    std::string get_sprop() const;

    std::string get_encoded_sps() const;
    std::string get_encoded_pps() const;

    std::vector<uint8_t> get_sps() const;
    std::vector<uint8_t> get_pps() const;

    std::vector<uint8_t> get_extra_data() const;

private:
    r_rtsp_source( const r_rtsp_source& );
    r_rtsp_source& operator = ( const r_rtsp_source& );

    void _parse_rtsp_server_ip_and_port( const std::string& rtspURL,
                                         std::string& ip,
                                         int& port ) const;

    std::string _parse_resource_path( const std::string& rtspURL ) const;

    std::string _parse_resource( const std::string& rtspURL ) const;

    bool _udp_setup();
    bool _tcp_setup();

    std::string _reconcile_rtsp_server_ip( const std::string& sdpCAddr,
                                           const std::string& sdpOAddr,
                                           const std::string& rtspAddr ) const;

    void _entry_point();

    void _wm_rtsp_interleaved();
    void _wm_udp();

    bool _time_for_rtsp_keep_alive() const;
    void _send_rtsp_keep_alive( WORKER_METHOD workerMethod );

    bool _contains_mpeg4_key_frame( uint8_t* buf, size_t size ) const;

    r_av::r_packet _dequeue();

    WORKER_METHOD _workerMethod;
    bool _workerRunning;
    std::thread _workerThread;
    std::string _rtspURL;
    std::shared_ptr<r_rtsp::r_client_connection> _riverClient;
    std::string _requestedStreamType;
    r_rtsp::r_session_description _sessionInfo;
    size_t _selectedMD;
    std::string _sdp;
    std::string _rtspControl;
    bool _discardFirstFrame;
    bool _seenFirstMarkerBit;
    std::shared_ptr<r_utils::r_udp_receiver> _udpRTPReceiver;
    bool _preferTCP;
    static std::mutex _udpPortLock;
    std::shared_ptr<r_utils::r_pool<std::vector<uint8_t>> > _packetPool;
    std::deque<std::shared_ptr<std::vector<uint8_t>>> _packetCache;
    uint16_t _lastSeq;
    bool _needSort;
    std::mutex _packetCacheLock;
    size_t _markersCached;
    size_t _markerlessPushThreshold;
    size_t _packetsCachedSinceLastMarker;
    std::chrono::milliseconds _keepAliveInterval;
    std::chrono::steady_clock::time_point _lastKeepAliveTime;
    std::condition_variable _consumerCond;
    std::mutex _consumerCondLock;
    std::map<std::string, std::string> _params;
    uint16_t _lastSequence;
    bool _lastSequenceValid;
    bool _controlSocketInterleavedKeepAlives;
    bool _healthy;
};

#endif