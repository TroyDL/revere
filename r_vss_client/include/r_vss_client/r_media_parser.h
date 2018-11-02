
#ifndef _r_vss_client_r_media_parser_h
#define _r_vss_client_r_media_parser_h

#include "r_utils/r_blob_tree.h"
#include "r_utils/r_byte_ptr.h"
#include <vector>
#include <string>
#include <chrono>
#include <map>

namespace r_vss_client
{

struct r_media_stats
{
    uint32_t frame_rate_num;
    uint32_t frame_rate_den;
};

class r_media_parser final
{
public:
    r_media_parser(const std::vector<uint8_t>& buffer);
    r_media_parser(const r_media_parser&) = delete;
    r_media_parser(r_media_parser&&) = delete;
    ~r_media_parser() noexcept;

    r_media_parser& operator=(const r_media_parser&) = delete;
    r_media_parser& operator=(r_media_parser&&) = delete;

    void begin();
    bool valid() const;
    void next();
    void current_data(std::chrono::system_clock::time_point& time, size_t& frameSize, const uint8_t*& frame, uint32_t& flags);

    r_media_stats get_stats();

private:
    const std::vector<uint8_t>& _buffer;
    uint32_t _version;
    r_utils::r_blob_tree _bt;
    size_t _frameIndex;
    bool _frameIndexValid;
    r_utils::r_byte_ptr_ro _headerBox;
    r_utils::r_byte_ptr_ro _indexBox;
    r_utils::r_byte_ptr_ro _frameBox;
    uint32_t _numFrames;
    std::string _sdp;
};

class r_media_parser_mux final
{
public:
    r_media_parser_mux();
    r_media_parser_mux(const r_media_parser_mux&) = default;
    r_media_parser_mux(r_media_parser_mux&&) = default;
    ~r_media_parser_mux() noexcept;

    r_media_parser_mux& operator=(const r_media_parser_mux&) = default;
    r_media_parser_mux& operator=(r_media_parser_mux&&) = default;

    void add_stream(const std::string& name, r_media_parser* p);

    void begin();
    bool valid() const;
    void next();
    void current_data(std::string& name, std::chrono::system_clock::time_point& time, size_t& frameSize, const uint8_t*& frame, uint32_t& flags);

private:
    std::pair<std::string, r_media_parser*> _oldest();

    std::map<std::string, r_media_parser*> _streams;
};

}

#endif