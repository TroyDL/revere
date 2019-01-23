
#ifndef r_motion_context_h
#define r_motion_context_h

#include "r_av/r_video_decoder.h"

#include <string>
#include <chrono>

class motion_context final
{
public:
    motion_context(const std::string& dataSourceID) :
        _dataSourceID(dataSourceID),
        _lastValid(false),
        _lastFrameTS()
    {
    }

    motion_context(const motion_context&) = default;
    motion_context(motion_context&&) = default;

    ~motion_context() noexcept {}

    motion_context& operator=(const motion_context&) = default;
    motion_context& operator=(motion_context&&) = default;

    std::string data_source_id() const {return _dataSourceID;}

    void process(r_av::r_video_decoder& dec);

private:
    std::string _dataSourceID;
    bool _lastValid;
    std::chrono::system_clock::time_point _lastFrameTS;
};

#endif