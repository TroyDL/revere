
#ifndef _r_rtsp_r_session_base_h
#define _r_rtsp_r_session_base_h

#include <ctime>
#include <memory>
#include "r_utils/r_string.h"
#include "r_rtsp/r_rtsp_server.h"
#include "r_rtsp/r_server_request.h"
#include "r_rtsp/r_server_response.h"

namespace r_rtsp
{

class r_rtsp_server;

class r_session_base
{
public:
    r_session_base( r_rtsp_server& server ) :
        _server( server )
    {
    }

    r_session_base( r_rtsp_server& server, uint64_t sessionTimeOutSeconds ) :
        _server( server ),
        _sessionTimeOutSeconds( sessionTimeOutSeconds )
    {
    }

    virtual ~r_session_base() noexcept {}

    virtual std::shared_ptr<r_session_base> clone() const = 0;

    virtual bool handles_this_presentation( const std::string& presentation ) = 0;

    virtual std::shared_ptr<r_server_response> handle_request( std::shared_ptr<r_server_request> request ) = 0;

    std::string get_session_id() const { return _sessionId; }
    void set_session_id( std::string id ) { _sessionId = id; }

    std::chrono::steady_clock::time_point get_last_keep_alive_time() const { return _lastKeepAliveTime; }
    virtual void update_keep_alive_time() { _lastKeepAliveTime = std::chrono::steady_clock::now(); }

    uint64_t get_timeout_interval_seconds() const { return _sessionTimeOutSeconds; }

    virtual std::string get_supported_options() { return "OPTIONS, DESCRIBE, SETUP, PLAY, TEARDOWN, GET_PARAMETER"; }

protected:
    r_rtsp_server& _server;
    std::string _sessionId;
    std::chrono::steady_clock::time_point _lastKeepAliveTime;
    uint64_t _sessionTimeOutSeconds;

private:
    r_session_base( const r_session_base& obj ) = delete;
    r_session_base& operator = ( const r_session_base& obj ) = delete;
};

}

#endif
