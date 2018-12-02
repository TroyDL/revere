#ifndef __r_av_r_demuxer_h
#define __r_av_r_demuxer_h

#include "r_av/r_packet.h"
#include "r_av/r_packet_factory.h"
#include "r_av/r_options.h"
#include "r_utils/r_nullable.h"
#include <vector>
#include <map>
#include <utility>

extern "C"
{
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
}

namespace r_av
{

struct r_stream_statistics
{
    r_utils::r_nullable<uint32_t> averageBitRate;
    r_utils::r_nullable<std::pair<int, int>> frameRate;
    r_utils::r_nullable<std::pair<int, int>> timeBase;
    r_utils::r_nullable<uint16_t> gopSize;
    r_utils::r_nullable<uint32_t> numFrames;
};

enum r_stream_type
{
    STREAM_TYPE_UNKNOWN = -1,
    STREAM_TYPE_VIDEO,
    STREAM_TYPE_AUDIO,
    STREAM_TYPE_DATA,
    STREAM_TYPE_SUBTITLE,
    STREAM_TYPE_ATTACHMENT,
    STREAM_TYPE_NUM
};

struct r_stream_info
{
    r_stream_type type;
    int index;
};

class r_h264mp4_to_annexb;
class r_h264_decoder;

class r_demuxer final
{
    friend class r_h264mp4_to_annexb;
    friend class r_h264_decoder;

public:
    r_demuxer(const std::string& fileName, bool annexBFilter = true);
    r_demuxer(const uint8_t* buffer,
               size_t bufferSize,
               bool annexBFilter = true);
    r_demuxer(const r_demuxer&) = delete;
    r_demuxer(r_demuxer&& obj) noexcept;

    ~r_demuxer() noexcept;

    r_demuxer& operator=(const r_demuxer&) = delete;
    r_demuxer& operator=(r_demuxer&& obj) noexcept;

    void set_packet_factory(std::shared_ptr<r_packet_factory> pf) { _pf = pf; }

    std::string get_sdp();

    std::string get_file_name() const;

    std::vector<r_stream_info> get_stream_types() const;

    int get_video_stream_index() const;
    int get_primary_audio_stream_index() const;

    std::pair<int,int> get_time_base(int streamIndex) const;  // how many ticks in 1 second?
    std::pair<int,int> get_frame_rate(int streamIndex) const;
    void set_output_time_base(int streamIndex, const std::pair<int, int>& tb);
    r_av_codec_id get_stream_codec_id(int streamIndex) const;
    r_pix_fmt get_pix_format(int streamIndex) const;
    std::pair<int,int> get_resolution(int streamIndex) const;
    std::vector<uint8_t> get_extradata(int streamIndex) const;
    AVCodecParameters* get_codec_parameters(int streamIndex) const;

    bool read_frame(int& streamIndex);
    bool end_of_file() const;
    bool is_key() const;

    r_packet get() const;

    static struct r_stream_statistics get_video_stream_statistics(const std::string& fileName);

private:
    void _open_streams();
    void _open_custom_io_context(const uint8_t* buffer, size_t bufferSize);

    static int _read(void* opaque, uint8_t* dest, int size);
    static int64_t _seek(void* opaque, int64_t offset, int whence);

    void _free_packet();
    void _free_filter_packet();

    void _init_annexb_filter();
    void _optional_annexb_filter();

    void _clear() noexcept;

    std::string _fileName;
    AVIOContext* _memoryIOContext;
    std::vector<uint8_t> _storage;
    int64_t _pos;
    AVFormatContext* _context;
    bool _eof;
    AVPacket _deMuxPkt;
    AVPacket _filterPkt;
    std::vector<r_stream_info> _streamTypes;
    int _videoStreamIndex;
    int _audioPrimaryStreamIndex;
    int _currentStreamIndex;
    AVBitStreamFilterContext* _bsfc;
    AVBSFContext *_bsf;
    std::shared_ptr<r_packet_factory> _pf;
    std::map<int, std::pair<int, int>> _outputTimeBases;
};

}

#endif