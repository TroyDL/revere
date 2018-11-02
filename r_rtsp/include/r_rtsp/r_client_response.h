
#ifndef r_rtsp_r_client_response_h
#define r_rtsp_r_client_response_h

#include "r_rtsp/r_status.h"
#include "r_utils/r_socket.h"
#include <map>
#include <memory>
#include <vector>

namespace r_rtsp
{

class r_client_response
{
public:
    r_client_response();
    r_client_response( const r_client_response& rhs );
    r_client_response( r_client_response&& rhs ) throw();
    virtual ~r_client_response() throw();

    r_client_response& operator = ( const r_client_response& obj );
    r_client_response& operator = ( r_client_response&& obj ) throw();

    bool get_header( const std::string& key, std::string& value ) const;

    std::vector<uint8_t> get_body() const;
    std::string get_body_as_string() const;

    void read_response( r_utils::r_stream_io& sok );

    status get_status() const;

    std::string get_session() const;

    std::vector<std::string> get_methods() const;

private:
    void _receive_body( r_utils::r_stream_io& sok, size_t bodySize );
    size_t _parse_lines( std::vector<std::string>& lines );

    std::map<std::string, std::string> _headerPieces;
    status _statusCode;
    std::vector<uint8_t> _body;
};

}

#endif
