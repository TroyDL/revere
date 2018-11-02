
#ifndef __r_av_r_packet_h
#define __r_av_r_packet_h

#include <cstdint>
#include <cstdlib>
#include <string>
#include "r_av/r_options.h"

class test_r_av_r_packet;

namespace r_av
{

class r_packet
{
    friend class ::test_r_av_r_packet;

public:
    r_packet(size_t sz=0);
    r_packet(r_filter_state fs);
    r_packet(uint8_t* src, size_t sz, bool owning = true);
    r_packet(const r_packet& obj);
    r_packet(r_packet&& obj) noexcept;
    virtual ~r_packet() throw();

    r_packet& operator = (const r_packet& obj);
    r_packet& operator = (r_packet&& obj) noexcept;

    bool empty() const { return _requestedSize == 0; }

    uint8_t* map() const;

    size_t get_buffer_size() const;

    void set_data_size(size_t sz);
    size_t get_data_size() const;

    void migrate_md_from(const r_packet& obj);

    void set_pts(int64_t pts);
    int64_t get_pts() const;

    void set_dts(int64_t dts);
    int64_t get_dts() const;

    void set_ts_freq(uint32_t freq);
    uint32_t get_ts_freq() const;

    void set_duration(uint32_t duration);
    uint32_t get_duration() const;

    void set_key(bool key);
    bool is_key() const;

    void set_width(uint16_t width);
    uint16_t get_width() const;

    void set_height(uint16_t height);
    uint16_t get_height() const;

    void set_sdp(const std::string& sdp) { _sdp = sdp; }
    std::string get_sdp() const { return _sdp; }

private:
    void _clear() noexcept;

    size_t _bufferSize;
    size_t _requestedSize;
    bool _owning;
    uint8_t* _buffer;
    size_t _dataSize;
    int64_t _pts;
    int64_t _dts;
    uint32_t _ticksInSecond;
    bool _key;
    uint16_t _width;
    uint16_t _height;
    uint32_t _duration;
    r_filter_state _filterState;
    std::string _sdp;
};

}

#endif
