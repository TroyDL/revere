
#ifndef r_rtsp_r_server_request_h
#define r_rtsp_r_server_request_h

#include "r_rtsp/r_methods.h"

#include "r_utils/r_socket.h"
#include "r_utils/interfaces/r_stream_io.h"
#include <map>
#include <list>

namespace r_rtsp
{

static const unsigned int MAX_HEADER_LINE = 2048;

class r_server_request
{
public:
    r_server_request();
    r_server_request( const r_server_request& rhs );
    r_server_request( r_server_request&& rhs ) throw();

    virtual ~r_server_request() throw();

    r_server_request& operator = ( const r_server_request& rhs );
    r_server_request& operator = ( r_server_request&& rhs ) throw();

    method get_method() const;

    std::string get_url() const;
    std::string get_uri() const;

    void set_peer_ip( const std::string& peerIP );
    std::string get_peer_ip() const;

    void set_local_ip( const std::string& localIP );
    std::string get_local_ip() const;

    bool get_header( const std::string& key, std::string& value ) const;
    void set_header( const std::string& key, const std::string& value );

    void read_request( r_utils::r_stream_io& sok );

    void dump()
    {
        for(auto& s : _requestLines)
            printf("%s\n",s.c_str());
        fflush(stdout);
    }

private:
    void _parse_initial_line( std::string& str );
    void _parse_additional_lines( const std::string& str );

    std::string _initialLine;
    std::list<std::string> _requestLines;
    std::map<std::string, std::string> _headerParts;
    std::string _peerIP;
    std::string _localIP;
};

}

#endif
