
#ifndef r_http_r_client_response_h
#define r_http_r_client_response_h

#include <list>
#include <vector>
#include <map>
#include <functional>
#include "r_utils/interfaces/r_stream_io.h"
#include "r_utils/r_string_utils.h"

namespace r_http
{

class r_client_response;

typedef std::function<void(const std::vector<uint8_t>&, const r_client_response&)> chunk_callback;
typedef std::function<void(const std::vector<uint8_t>&, const std::map<std::string, std::string>&, const r_client_response&)> part_callback;

class r_client_response
{
public:
    r_client_response();
    r_client_response(const r_client_response& obj);
    virtual ~r_client_response() noexcept;

    r_client_response& operator = (const r_client_response& obj);

    void read_response(r_utils::r_stream_io& socket);

    std::string get_message();

    void debug_print_request();

    std::vector<uint8_t> release_body();

    const void* get_body() const;

    size_t get_body_size() const;

    std::string get_body_as_string() const;

    std::string get_header(const std::string& header) const;

    std::vector<std::string> get_all_matching_headers(const std::string& header) const;

    bool is_success() const;

    bool is_failure() const;

    int get_status() const { return _statusCode; }

    void register_chunk_callback( chunk_callback cb, bool streaming = false );
    void register_part_callback( part_callback pb );

private:
    void _read_chunked_body(r_utils::r_stream_io& socket);
    void _read_multi_part(r_utils::r_stream_io& socket);
    std::map<std::string, std::string> _read_multi_header_lines(r_utils::r_stream_io& socket, char* lineBuf);
    void _read_end_of_line(r_utils::r_stream_io& socket);

    bool _is_legal_chunk_size_char(char ch) { return isxdigit(ch) ? true : false; } // VS warning: forcing int to bool.
    bool _embed_null(char* lineBuf);
    void _consume_footer(r_utils::r_stream_io& socket);

    void _add_header(const std::string& name, const std::string& value);

    void _clean_socket(r_utils::r_stream_io& socket, char** writer);
    void _read_header_line(r_utils::r_stream_io& socket, char* writer, bool firstLine);
    bool _add_line(std::list<std::string>& lines, const std::string& line);
    void _process_request_lines(const std::list<std::string>& requestLines);
    void _process_body(r_utils::r_stream_io& socket);

    bool _is_end_of_line(char* buffer)
    {
        return (buffer[0] == '\r' && buffer[1] == '\n') || buffer[0] == '\n';
    }

    std::string _initialLine;
    std::map<std::string, std::list<std::string> > _headerParts;
    std::vector<uint8_t> _bodyContents;
    bool _success;
    int  _statusCode;
    chunk_callback _chunkCallback;
    part_callback _partCallback;
    std::vector<uint8_t> _chunk;
    bool _streaming;
};

}

#endif
