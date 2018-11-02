
#ifndef r_utils_r_timer_h
#define r_utils_r_timer_h

#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>

namespace r_utils
{

typedef std::function<void()> r_timer_cb;

class r_timer final
{
public:
    r_timer( size_t intervalMillis, r_timer_cb cb );
    r_timer(const r_timer&) = delete;
    r_timer(r_timer&&) = delete;
    ~r_timer() noexcept;

    r_timer& operator=(const r_timer&) = delete;
    r_timer& operator=(r_timer&&) = delete;

    void start();
    void stop();

private:
    void _timer_loop();

    std::thread _thread;
    size_t _intervalMillis;
    r_timer_cb _cb;
    std::recursive_mutex _lok;
    std::condition_variable_any _cond;
    bool _started;
};

}

#endif
