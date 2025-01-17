
#ifndef _r_http_r_client_request_h
#define _r_http_r_client_request_h

#include "r_utils/interfaces/r_stream_io.h"
#include "r_utils/r_string_utils.h"

#include <unordered_map>
#include <memory>
#include <vector>

#include "r_http/r_uri.h"
#include "r_http/r_methods.h"

class test_r_http;

namespace r_http
{

const std::string get_request = "GET";
const std::string post_request = "POST";
const std::string put_request = "PUT";
const std::string delete_request = "DELETE";
const std::string patch_request = "PATCH";

class r_client_request
{
    friend class ::test_r_http;

public:
    r_client_request( const std::string& host, int hostPort );

    r_client_request( const r_client_request& rhs );

    virtual ~r_client_request() noexcept;

    r_client_request& operator = ( const r_client_request& rhs );

    void set_method( int method );

    void write_request( r_utils::r_stream_io& socket ) const;

    void set_accept_type( const std::string& acceptType );

    void set_content_type( const std::string& contentType );

    void set_uri( const r_uri& res );

    void set_basic_authentication( const std::string& user, const std::string& pass );

    void add_post_var( const std::string& name, const std::string& value );

    void add_header( const std::string& name, const std::string& value );

    void set_body( const uint8_t* src, size_t size );
    void set_body( const std::string& body );

private:
    std::string _get_headers_as_string( r_utils::r_stream_io& socket ) const;

    r_uri _uri;
    std::string _acceptType;
    std::string _contentType;
    int _method;
    std::string _authData;
    std::unordered_map<std::string, std::string> _postVars;
    std::unordered_map<std::string, std::string> _headerParts;

    mutable std::vector<uint8_t> _body;
    std::string _host;
    int _hostPort;
};

}

#endif
