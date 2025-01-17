
#ifndef r_http_r_server_request_h
#define r_http_r_server_request_h

#include <list>
#include <map>
#include <memory>
#include <vector>
#include "r_utils/interfaces/r_stream_io.h"
#include "r_utils/r_string_utils.h"
#include "r_utils/r_nullable.h"
#include "r_http/r_uri.h"
#include "r_http/r_methods.h"

namespace r_http
{

class r_server_request
{
public:
    r_server_request();

    r_server_request(const r_server_request& rhs);

    virtual ~r_server_request() noexcept;

    r_server_request& operator = (const r_server_request& rhs);

    void read_request(r_utils::r_stream_io& socket);

    int get_method() const;

    r_uri get_uri() const;

    std::string get_content_type() const;

    r_utils::r_nullable<std::string> get_header( const std::string& key ) const;
    std::map<std::string,std::string> get_headers() const;

    const uint8_t* get_body() const;
    size_t get_body_size() const;
    std::string get_body_as_string() const;

    std::map<std::string,std::string> get_post_vars() const;

    bool is_patch_request() const;
    bool is_put_request() const;
    bool is_post_request() const;
    bool is_get_request() const;
    bool is_delete_request() const;

private:
    void _set_header(const std::string& name, const std::string& value);
    void _clean_socket(r_utils::r_stream_io& socket, char** writer);
    void _read_header_line(r_utils::r_stream_io& socket, char* writer, bool firstLine);
    bool _add_line(std::list<std::string>& lines, const std::string& line);
    void _process_request_lines(const std::list<std::string>& requestLines);
    void _process_body(r_utils::r_stream_io& socket);

    std::string _initialLine;
    std::map<std::string,std::string> _headerParts;
    std::map<std::string,std::string> _postVars;
    std::vector<uint8_t> _body;
    std::string _contentType;
};

}

#endif
