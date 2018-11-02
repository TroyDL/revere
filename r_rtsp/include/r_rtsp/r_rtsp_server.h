
#ifndef _r_rtsp_r_rtsp_server_h
#define _r_rtsp_r_rtsp_server_h

#include <map>
#include <memory>
#include <mutex>

#include "r_utils/r_socket.h"
#include "r_utils/r_timer.h"
#include "r_rtsp/r_session_base.h"
#include "r_rtsp/r_server_request.h"
#include "r_rtsp/r_server_response.h"
#include "r_rtsp/r_server_connection.h"

#include <list>

namespace r_rtsp
{

class r_rtsp_server
{
public:
    r_rtsp_server( const std::string& serverIP, int port = 554 );
    virtual ~r_rtsp_server() throw();

    void attach_session_prototype( std::shared_ptr<r_session_base> session );

    void start();
    void stop();

    std::shared_ptr<r_server_response> route_request( std::shared_ptr<r_server_request> request );

    static std::string get_next_session_id();

    void stop_session( std::string id );

    void check_inactive_sessions();

private:
    r_rtsp_server( const r_rtsp_server& obj ) = delete;
    r_rtsp_server& operator = ( const r_rtsp_server& obj ) = delete;

    void* _entry_point();

    std::shared_ptr<r_server_response> _handle_options( std::shared_ptr<r_server_request> request );
    std::shared_ptr<r_server_response> _handle_describe( std::shared_ptr<r_server_request> request );
    void _handle_setup( std::shared_ptr<r_server_request> request );

    std::shared_ptr<r_session_base> _locate_session_prototype( const std::string& resourcePath );

    std::thread _thread;
    int _port;
    bool _running;
    std::list<std::shared_ptr<r_session_base> > _sessionPrototypes;
    std::map<std::string, std::shared_ptr<r_session_base> > _sessions;
    std::list<std::shared_ptr<r_server_connection> > _connections;
    std::shared_ptr<r_utils::r_socket> _serverSocket;
    std::string _serverIP;
    std::recursive_mutex _sessionsLock;
    std::recursive_mutex _connectionsLock;
    std::recursive_mutex _prototypesLock;
    r_utils::r_timer _aliveCheck;

    static uint32_t _next_session_id;
    static std::recursive_mutex _session_id_lock;
};

}

#endif
