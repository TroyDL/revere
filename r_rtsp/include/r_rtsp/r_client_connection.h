
#ifndef r_rtsp_r_client_connection_h
#define r_rtsp_r_client_connection_h

#include "r_utils/r_string_utils.h"
#include "r_utils/r_socket.h"
#include "r_rtsp/r_client_response.h"
#include "r_rtsp/r_client_request.h"

namespace r_rtsp
{

class r_client_connection
{
public:
    r_client_connection( const std::string& serverIP, int port );
    virtual ~r_client_connection() noexcept;

    void connect();
    void disconnect();

    void set_recv_timeout( uint64_t timeoutMillis ) { _ioTimeOut = timeoutMillis; }

    void write_request( std::shared_ptr<r_client_request > request );

    std::shared_ptr<r_client_response> read_response();

    std::string get_session_id() const;

    void set_session_id( const std::string& sessionID );

    bool receive_interleaved_packet( uint8_t& channel, std::vector<uint8_t>& buffer );

private:
    r_client_connection( const r_client_connection& rhs ) = delete;
    r_client_connection& operator = ( const r_client_connection& rhs ) = delete;

    std::string _serverIP;
    int _serverPort;
    r_utils::r_socket _sok;
    int _sequence;
    std::string _sessionID;
    uint64_t _ioTimeOut;
};

}

#endif
