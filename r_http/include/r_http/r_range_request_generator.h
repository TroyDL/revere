
#ifndef r_http_r_range_request_generator_h
#define r_http_r_range_request_generator_h

#include "r_http/r_client_request.h"
#include "r_utils/r_string_utils.h"

class test_r_http_r_range_request_generator;

namespace r_http
{

class r_range_request_generator final
{
    friend class ::test_r_http_r_range_request_generator;
public:
    r_range_request_generator(const std::string& host, int port, const std::string& uri, size_t totalSize, size_t requestSize, size_t pos);
    r_range_request_generator(const r_range_request_generator& obj) = default;
    r_range_request_generator(r_range_request_generator&&) noexcept = default;
    ~r_range_request_generator() noexcept;

    r_range_request_generator& operator=(const r_range_request_generator& obj) = default;
    r_range_request_generator& operator=(r_range_request_generator&&) noexcept = default;

    bool valid() const;
    r_client_request get() const;
    void next();

private:
    std::string _host;
    int _port;
    std::string _uri;
    size_t _totalSize;
    size_t _requestSize;
    size_t _pos;
};

}

#endif
