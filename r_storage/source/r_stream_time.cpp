
#include "r_storage/r_stream_time.h"
#include "r_utils/r_time_utils.h"
#include <algorithm>
#include <numeric>

using namespace r_storage;
using namespace r_utils;
using namespace std;
using namespace std::chrono;

static const int STREAM_BASE_UPDATE_FREQ = 1000;
static const int STREAM_BASE_CORRECTION_FREQ = 10;
static const uint64_t MILLIS_IN_SECOND = 1000;

r_stream_time::r_stream_time(uint32_t sourceClockRate) :
    _sourceClockRate(sourceClockRate),
    _currentStreamBase(),
    _targetStreamBase(),
    _lastTS(0),
    _streamBaseUpdateCountdown(STREAM_BASE_UPDATE_FREQ),
    _validLastRTPTime(false),
    _lastRTPTime(0),
    _rtpDelta(0),
    _deltas(1000, 0),
    _numDeltas(0)
{
}

r_stream_time::~r_stream_time() noexcept
{
}

uint64_t r_stream_time::current(uint32_t rtpTime)
{
    if( (rtpTime - _lastRTPTime) > _sourceClockRate )
    {
        _validLastRTPTime = false;
        _streamBaseUpdateCountdown = STREAM_BASE_UPDATE_FREQ;
    }

retry:
    if( _validLastRTPTime == false )
    {
        _currentStreamBase = system_clock::now();
        _rtpDelta = 0;
    }

    if( _validLastRTPTime )
    {
        rotate(begin(_deltas), begin(_deltas) + (_deltas.size()-1), end(_deltas));
        _deltas[0] = ::abs(rtpTime - _lastRTPTime);
        if(_numDeltas<1000)
            ++_numDeltas;
    }

    uint32_t currentAvgDelta = 0;
    currentAvgDelta = (_numDeltas>0)?(accumulate(begin(_deltas), begin(_deltas)+_numDeltas, currentAvgDelta) / _numDeltas):0;
    
    uint64_t outputTS = r_time_utils::tp_to_epoch_millis(_currentStreamBase);

    _streamBaseUpdateCountdown--;
    if( _streamBaseUpdateCountdown <= 0 )
    {
        _streamBaseUpdateCountdown = STREAM_BASE_UPDATE_FREQ;

        uint64_t currentDelta = _rtpDelta + currentAvgDelta;

        uint64_t currentStreamTime = r_time_utils::tp_to_epoch_millis(_currentStreamBase) + r_time_utils::convert_clock_freq(currentDelta, _sourceClockRate, MILLIS_IN_SECOND);

        uint64_t currentTime = r_time_utils::tp_to_epoch_millis(system_clock::now());

        if( currentStreamTime > currentTime )
        {
            _streamBaseUpdateCountdown = STREAM_BASE_CORRECTION_FREQ;

            _rtpDelta += currentAvgDelta / 2;
        }
        else
        {
            if( llabs( currentStreamTime - currentTime ) > 15 )
            {
                _validLastRTPTime = false;
                _streamBaseUpdateCountdown = STREAM_BASE_CORRECTION_FREQ;
                goto retry;
            }
            else _rtpDelta += currentAvgDelta;
        }
    }
    else
    {
        if( _validLastRTPTime )
        {
            _rtpDelta += currentAvgDelta;
        }
    }

    outputTS += r_time_utils::convert_clock_freq(_rtpDelta, _sourceClockRate, MILLIS_IN_SECOND);

    if( outputTS <= _lastTS )
        outputTS = _lastTS+1;

    _lastTS = outputTS;

    _lastRTPTime = rtpTime;

    _validLastRTPTime = true;

    return outputTS;
}
