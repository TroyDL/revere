
#ifndef r_utils_r_timer_h
#define r_utils_r_timer_h

#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <chrono>
#include <vector>

namespace r_utils
{

typedef std::function<void()> r_timer_cb;

class r_timer final
{
public:
    void add_timed_event(
        const std::chrono::steady_clock::time_point& now,
        const std::chrono::milliseconds& interval,
        r_timer_cb cb,
        bool initial_fire,
        bool one_shot = false
    );

    // takes the current time and returns the number of milliseconds until the next event.
    std::chrono::milliseconds update(const std::chrono::milliseconds& max_sleep, const std::chrono::steady_clock::time_point& now);

private:
    struct r_timed_event
    {
        std::chrono::steady_clock::time_point next_fire_time;
        bool one_shot;
        std::chrono::milliseconds interval;
        r_timer_cb cb;

        bool update(const std::chrono::steady_clock::time_point& now);
    };

    std::vector<r_timed_event> _timed_events {};
};

}

#endif
