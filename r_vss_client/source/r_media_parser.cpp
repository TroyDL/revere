
#include "r_vss_client/r_media_parser.h"
#include "r_utils/r_socket.h"
#include "r_utils/r_time_utils.h"
#include <cmath>

using namespace r_vss_client;
using namespace r_utils;
using namespace std;
using namespace std::chrono;

r_media_parser::r_media_parser(const std::vector<uint8_t>& buffer) :
    _buffer(buffer),
    _version(0),
    _bt(r_blob_tree::deserialize(&_buffer[0], _buffer.size(), _version)),
    _frameIndex(0),
    _frameIndexValid(false),
    _headerBox(r_byte_ptr_ro(_bt["header_box"].get())),
    _indexBox(r_byte_ptr_ro(_bt["index_box"].get())),
    _frameBox(r_byte_ptr_ro(_bt["frame_box"].get())),
    _numFrames(0),
    _sdp()
{
    uint32_t word = _headerBox.consume<uint32_t>();
    _numFrames = r_networking::r_ntohl(word);
    uint16_t shortVal = _headerBox.consume<uint16_t>();
    uint16_t sdpLen = r_networking::r_ntohs(shortVal);
    _sdp = string((const char*)_headerBox.get_ptr(), sdpLen);
}

r_media_parser::~r_media_parser() noexcept
{
}

void r_media_parser::begin()
{
    if(_numFrames == 0)
        R_STHROW(r_internal_exception, ("Invalid media blob."));

    _frameIndex = 0;
    _frameIndexValid = true;
}

bool r_media_parser::valid() const
{
    return _frameIndexValid;
}

void r_media_parser::next()
{
    ++_frameIndex;
    if(_frameIndex >= _numFrames)
        _frameIndexValid = false;
}

void r_media_parser::current_data(std::chrono::system_clock::time_point& time, size_t& frameSize, const uint8_t*& frame, uint32_t& flags)
{
    if(!_frameIndexValid)
        R_STHROW(r_internal_exception, ("Cannot fetch data from invalid iterator position."));

    auto idx = _indexBox;

    idx += (_frameIndex * 20);

    uint64_t doubleWord = idx.consume<uint64_t>();
    uint64_t frameEpoch = r_networking::r_ntohll(doubleWord);
    time = r_time_utils::epoch_millis_to_tp(frameEpoch);

    uint32_t word = idx.consume<uint32_t>();
    uint32_t frameOfs = r_networking::r_ntohl(word);

    auto fb = _frameBox;
    fb += frameOfs;
    frame = fb.get_ptr();

    word = idx.consume<uint32_t>();
    uint32_t fs = r_networking::r_ntohl(word);
    frameSize = fs;

    word = idx.consume<uint32_t>();
    flags = r_networking::r_ntohl(word);
}

static double q2d(uint32_t num, uint32_t den) { return num / (double)den; }

static uint64_t gcd(uint64_t a, uint64_t b)
{
    if(a == 0)
        return b;
    else if(b == 0)
        return a;

    if(a < b)
        return gcd(a, b % a);
    else return gcd(b, a % b);
}

r_media_stats r_media_parser::get_stats()
{
    bool lastValid = false;
    system_clock::time_point lastFrameTime;
    uint64_t accumDuration = 0;
    uint64_t numFrames = 0;

    begin();
    while(valid())
    {
        system_clock::time_point ft;
        size_t frameSize;
        const uint8_t* frame;
        uint32_t flags;

        current_data(ft, frameSize, frame, flags);
        ++numFrames;

        if(lastValid)
        {
            auto delta = (int64_t)duration_cast<microseconds>(ft - lastFrameTime).count();
            accumDuration += delta;
        }

        lastValid = true;
        lastFrameTime = ft;

        next();
    }

    r_media_stats stats;

    auto f = (1.0 / q2d(accumDuration / (numFrames-1), 1000000));

    double integral = std::floor(f);
    double frac = f - integral;

    uint64_t precision = 10000;
    uint64_t gcd_ = gcd(round(frac * precision), precision);

    stats.frame_rate_den = (uint32_t)(precision / gcd_);
    stats.frame_rate_num = (uint32_t)(round(frac * precision) / gcd_) + (integral * stats.frame_rate_den);

    return stats;
}

r_media_parser_mux::r_media_parser_mux() :
    _streams()
{
}

r_media_parser_mux::~r_media_parser_mux() noexcept
{
}

void r_media_parser_mux::add_stream(const string& name, r_media_parser* p)
{
    _streams[name] = p;
}

void r_media_parser_mux::begin()
{
    for(auto mp : _streams)
        mp.second->begin();
}

bool r_media_parser_mux::valid() const
{
    for(auto mp : _streams)
    {
        if(mp.second->valid())
            return true;
    }
    return false;
}

void r_media_parser_mux::next()
{
    auto mp = _oldest();
    mp.second->next();
}

void r_media_parser_mux::current_data(string& name, system_clock::time_point& time, size_t& frameSize, const uint8_t*& frame, uint32_t& flags)
{
    auto mp = _oldest();
    name = mp.first;
    mp.second->current_data(time, frameSize, frame, flags);
}

pair<string, r_media_parser*> r_media_parser_mux::_oldest()
{
    if(_streams.empty())
        R_THROW(("Add a stream before iterating."));

    auto oldest = _streams.begin()->second;

    string oldestName = _streams.begin()->first;

    system_clock::time_point oldestTime;
    size_t oldestFrameSize;
    const uint8_t* oldestFrame;
    uint32_t oldestFlags;
    oldest->current_data(oldestTime, oldestFrameSize, oldestFrame, oldestFlags);

    for(auto i = _streams.begin(), e = _streams.end(); i != e; ++i)
    {
        system_clock::time_point thisTime;
        size_t thisFrameSize;
        const uint8_t* thisFrame;
        uint32_t thisFlags;
        i->second->current_data(thisTime, thisFrameSize, thisFrame, thisFlags);

        if(thisTime < oldestTime)
        {
            oldest = i->second;
            oldestName = i->first;
        }
    }

    return make_pair(oldestName, oldest);
}
