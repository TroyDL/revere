
#ifndef __r_av_r_muxer_h
#define __r_av_r_muxer_h

#include "r_av/r_packet.h"
#include "r_av/r_options.h"
#include <memory>

extern "C"
{
#include "libavformat/avformat.h"
}

namespace r_av
{

struct r_muxer_stream
{
    r_stream_options options;
    AVStream* st;
};

class r_muxer final
{
public:
    enum OUTPUT_LOCATION
    {
        OUTPUT_LOCATION_FILE,
        OUTPUT_LOCATION_BUFFER,
        OUTPUT_LOCATION_RTP
    };

    r_muxer(OUTPUT_LOCATION location,
            const std::string& fileName);

    r_muxer(const r_muxer&) = delete;
    r_muxer(r_muxer&& obj) noexcept;
    ~r_muxer() noexcept;

    r_muxer& operator=(const r_muxer&) = delete;
    r_muxer& operator=(r_muxer&& obj) noexcept;

    AVCodecParameters* get_codec_parameters(int streamIndex) const;

    int add_stream(const r_stream_options& soptions);

    void set_extradata(const std::vector<uint8_t>& ed, int streamIndex);

    void write_packet(const r_packet& input, int streamIndex, bool keyFrame);

    void finalize_file();
    std::vector<uint8_t> finalize_buffer();

private:
    void _write_header();
    void _write_trailer();
    void _open_io();
    void _clear() noexcept;

    OUTPUT_LOCATION _location;
    std::string _fileName;
    std::vector<r_muxer_stream> _streams;
    AVOutputFormat* _format;
    AVFormatContext* _fc;
    int64_t _ts;
    bool _isTS;
    bool _isMP4;
    bool _oweTrailer;
    int64_t _numVideoFramesWritten;
    int64_t _fileNum;
};

}

#endif
