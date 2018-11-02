
#ifndef __r_rtsp_r_server_connection_h
#define __r_rtsp_r_server_connection_h

#include "r_utils/r_socket.h"
#include <memory>
#include <thread>

namespace r_rtsp
{

class r_rtsp_server;

class r_server_connection
{
public:
    r_server_connection( r_rtsp_server* server,
                         r_utils::r_socket clientSocket );

    virtual ~r_server_connection() throw();

    void start();
    void stop();

    bool running() const;

    void write_interleaved_packet( uint8_t channel, const std::vector<uint8_t>& buffer );

    std::string get_session_id() const { return _sessionID; }

private:
    r_server_connection( const r_server_connection& obj ) = delete;
    r_server_connection& operator = ( const r_server_connection& obj ) = delete;

    void _entry_point();

    std::thread _thread;
    r_utils::r_socket _clientSocket;
    r_rtsp_server* _server;
    std::string _sessionID;
    //Called "myrunning" even though it sounds douche, because it indicates the base class has a "running" member
    bool _myrunning;
    bool _isStarted;
};

}

#endif
