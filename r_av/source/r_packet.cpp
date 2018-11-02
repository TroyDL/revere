
#include "r_av/r_packet.h"
#include "r_utils/r_exception.h"

extern "C"
{
#include "libavutil/avutil.h"
}

using namespace r_av;
using namespace r_utils;

static const size_t PADDING = 16;

r_packet::r_packet(size_t sz) :
    _bufferSize(sz + PADDING),
    _requestedSize(sz),
    _owning(true),
    _buffer(nullptr),
    _dataSize(0),
    _pts(0),
    _dts(0),
    _ticksInSecond(90000),
    _key(false),
    _width(0),
    _height(0),
    _duration(0),
    _filterState(r_filter_state_default),
    _sdp()
{
    _buffer = (uint8_t*)av_malloc(_bufferSize);
    if(!_buffer)
        R_STHROW(r_internal_exception, ("Unable to allocate packet buffer."));
}

r_packet::r_packet(r_filter_state fs) :
    _bufferSize(0),
    _requestedSize(0),
    _owning(false),
    _buffer(nullptr),
    _dataSize(0),
    _pts(0),
    _dts(0),
    _ticksInSecond(90000),
    _key(false),
    _width(0),
    _height(0),
    _duration(0),
    _filterState(fs),
    _sdp()
{
}

r_packet::r_packet(uint8_t* src, size_t sz, bool owning) :
    _bufferSize(sz + PADDING),
    _requestedSize(sz),
    _owning(owning),
    _buffer(src),
    _dataSize(sz),
    _pts(0),
    _dts(0),
    _ticksInSecond(90000),
    _key(false),
    _width(0),
    _height(0),
    _duration(0),
    _filterState(r_filter_state_default),
    _sdp()
{
    if(_owning)
    {
        _buffer = (uint8_t*)av_malloc(_bufferSize);
        if(!_buffer)
            R_STHROW(r_internal_exception, ("Unable to allocate packet buffer."));

        memcpy(_buffer, src, sz);
    }
    else _bufferSize = sz;
}

r_packet::r_packet(const r_packet& obj) :
    _bufferSize(0),
    _requestedSize(0),
    _owning(false),
    _buffer(nullptr),
    _dataSize(0),
    _pts(0),
    _dts(0),
    _ticksInSecond(90000),
    _key(false),
    _width(0),
    _height(0),
    _duration(0),
    _filterState(r_filter_state_default),
    _sdp()
{
    _clear();

    _bufferSize = obj._bufferSize;
    _requestedSize = obj._requestedSize;
    _dataSize = obj._dataSize;
    _owning = obj._owning;

    if(obj._owning)
    {
        _buffer = (uint8_t*)av_malloc(_bufferSize);
        if(!_buffer)
            R_STHROW(r_internal_exception, ("Unable to allocate packet buffer."));

        memcpy(_buffer, obj._buffer, _bufferSize);
    }
    else _buffer = obj._buffer;

    migrate_md_from(obj);
}

r_packet::r_packet(r_packet&& obj) noexcept
{
    _bufferSize = std::move(obj._bufferSize);
    _requestedSize = std::move(obj._requestedSize);
    _owning = std::move(obj._owning);
    _buffer = std::move(obj._buffer);
    _dataSize = std::move(obj._dataSize);
    _pts = std::move(obj._pts);
    _dts = std::move(obj._dts);
    _ticksInSecond = std::move(obj._ticksInSecond);
    _key = std::move(obj._key);
    _width = std::move(obj._width);
    _height = std::move(obj._height);
    _duration = std::move(obj._duration);
    _filterState = std::move(obj._filterState);
    _sdp = std::move(obj._sdp);

    obj._buffer = nullptr;
    obj._bufferSize = 0;
    obj._dataSize = 0;
}

r_packet::~r_packet() noexcept
{
    _clear();
}

r_packet& r_packet::operator = (const r_packet& obj)
{
    _clear();

    _bufferSize = obj._bufferSize;
    _requestedSize = obj._requestedSize;
    _dataSize = obj._dataSize;
    _owning = obj._owning;

    if(obj._owning)
    {
        _buffer = (uint8_t*)av_malloc(_bufferSize);
        if(!_buffer)
            R_STHROW(r_internal_exception, ("Unable to allocate packet buffer."));

        memcpy(_buffer, obj._buffer, _bufferSize);
    }
    else _buffer = obj._buffer;

    migrate_md_from(obj);

    return *this;
}

r_packet& r_packet::operator = (r_packet&& obj) throw()
{
    _clear();

    _bufferSize = std::move(obj._bufferSize);
    _requestedSize = std::move(obj._requestedSize);
    _owning = std::move(obj._owning);
    _buffer = std::move(obj._buffer);
    _dataSize = std::move(obj._dataSize);
    _pts = std::move(obj._pts);
    _dts = std::move(obj._dts);
    _ticksInSecond = std::move(obj._ticksInSecond);
    _key = std::move(obj._key);
    _width = std::move(obj._width);
    _height = std::move(obj._height);
    _duration = std::move(obj._duration);
    _filterState = std::move(obj._filterState);
    _sdp = std::move(obj._sdp);

    obj._buffer = nullptr;
    obj._bufferSize = 0;
    obj._dataSize = 0;

    return *this;
}

uint8_t* r_packet::map() const
{
    return _buffer;
}

size_t r_packet::get_buffer_size() const
{
    return _requestedSize;
}

void r_packet::set_data_size(size_t sz)
{
    if(sz > _requestedSize)
        R_STHROW(r_internal_exception, ("Unable to set data size to amount greater than buffer size."));
    _dataSize = sz;
}

size_t r_packet::get_data_size() const
{
    return _dataSize;
}

void r_packet::migrate_md_from(const r_packet& obj)
{
    _pts = obj._pts;
    _dts = obj._dts;
    _ticksInSecond = obj._ticksInSecond;
    _key = obj._key;
    _width = obj._width;
    _height = obj._height;
    _duration = obj._duration;
    _filterState = obj._filterState;
    _sdp = obj._sdp;
}

void r_packet::set_pts(int64_t pts)
{
    _pts = pts;
}

int64_t r_packet::get_pts() const
{
    return _pts;
}

void r_packet::set_dts(int64_t dts)
{
    _dts = dts;
}

int64_t r_packet::get_dts() const
{
    return _dts;
}

void r_packet::set_ts_freq(uint32_t freq)
{
    _ticksInSecond = freq;
}

uint32_t r_packet::get_ts_freq() const
{
    return _ticksInSecond;
}

void r_packet::set_duration(uint32_t duration)
{
    _duration = duration;
}

uint32_t r_packet::get_duration() const
{
    return _duration;
}

void r_packet::set_key(bool key)
{
    _key = key;
}

bool r_packet::is_key() const
{
    return _key;
}

void r_packet::set_width(uint16_t width)
{
    _width = width;
}

uint16_t r_packet::get_width() const
{
    return _width;
}

void r_packet::set_height(uint16_t height)
{
    _height = height;
}

uint16_t r_packet::get_height() const
{
    return _height;
}

void r_packet::_clear() throw()
{
    if(_owning && _buffer)
        av_free(_buffer);

    _buffer = nullptr;
    _bufferSize = 0;
    _requestedSize = 0;
    _dataSize = 0;
    _pts = 0;
    _dts = 0;
    _ticksInSecond = 90000;
    _width = 0;
    _height = 0;
    _filterState = r_filter_state_default;
    _sdp.clear();
}
