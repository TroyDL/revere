
#ifndef r_rtsp_r_client_request_h
#define r_rtsp_r_client_request_h

#include "r_utils/r_string_utils.h"
#include "r_utils/interfaces/r_stream_io.h"
#include "r_rtsp/r_methods.h"
#include <memory>
#include <map>

namespace r_rtsp
{

class r_client_request 
{
public:
    r_client_request ( method m = M_OPTIONS );
    r_client_request ( const r_client_request & rhs );
    r_client_request ( r_client_request && rhs ) throw();

    virtual ~r_client_request () throw();

    r_client_request & operator = ( const r_client_request & rhs );
    r_client_request & operator = ( r_client_request && rhs ) throw();

    void set_method( method m );
    method get_method() const;

    void set_user_agent( const std::string& userAgent );
    std::string get_user_agent() const;

    void set_header( const std::string& name,
                            const std::string& value );
    bool get_header( const std::string& name,
                            std::string& value ) const;

    void set_server_ip( const std::string& serverIP );
    std::string get_server_ip() const;

    void set_server_port( int port );
    int get_server_port() const;

    void set_uri( const std::string& uri );
    std::string get_uri() const;

    void write_request( r_utils::r_stream_io& sok );

private:
    method _method;
    std::string _userAgent;
    std::map<std::string, std::string> _additionalHeaders;
    std::string _serverIP;
    int _serverPort;
    std::string _uri;
};

}

#endif
