
#ifndef _r_storage_r_stream_time_h
#define _r_storage_r_stream_time_h

#include <chrono>
#include <cstdint>
#include <vector>

namespace r_storage
{

class r_stream_time final
{
public:
    r_stream_time(uint32_t sourceClockRate);
    r_stream_time(const r_stream_time&) = default;
    r_stream_time(r_stream_time&&) noexcept = default;

    ~r_stream_time() noexcept;

    r_stream_time& operator=(const r_stream_time&) = default;
    r_stream_time& operator=(r_stream_time&&) noexcept = default;

    uint64_t current(uint32_t rtpTime);

private:
    uint64_t _sourceClockRate;
    std::chrono::system_clock::time_point _currentStreamBase;
    std::chrono::system_clock::time_point _targetStreamBase;
    uint64_t _lastTS;
    int _streamBaseUpdateCountdown;
    bool _validLastRTPTime;
    uint32_t _lastRTPTime;
    uint64_t _rtpDelta;
    std::vector<uint32_t> _deltas;
    uint32_t _numDeltas;
};

}

#endif
