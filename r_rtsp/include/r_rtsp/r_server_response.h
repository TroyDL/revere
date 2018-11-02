
#ifndef _r_rtsp_r_server_response_h
#define _r_rtsp_r_server_response_h

#include "r_utils/r_socket.h"
#include "r_utils/interfaces/r_stream_io.h"
#include "r_rtsp/r_status.h"
#include "r_rtsp/r_methods.h"

#include <map>
#include <memory>

namespace r_rtsp
{

class r_server_response
{
public:
    r_server_response();
    r_server_response( const r_server_response& obj );
    r_server_response( r_server_response&& obj ) throw();

    virtual ~r_server_response() throw();

    r_server_response& operator = ( const r_server_response& rhs );
    r_server_response& operator = ( r_server_response&& rhs ) throw();

    void set_status( status s );
    status get_status() const;

    void set_header( const std::string& key, const std::string& value );
    bool get_header( const std::string& key, std::string& value );

    void set_body( const std::vector<uint8_t>& body );
    std::vector<uint8_t> get_body() const;

    void set_body( const std::string& body );
    std::string get_body_as_string() const;

    void write_response( r_utils::r_stream_io& sok );

    // Factory Methods...
    //

    static r_server_response create_response_to( method m );

private:
    status _status;
    std::map<std::string, std::string> _headers;
    std::vector<uint8_t> _body;
};

}

#endif
