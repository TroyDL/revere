
#ifndef r_utils_r_work_q_h
#define r_utils_r_work_q_h

#include "r_utils/r_nullable.h"

#include <mutex>
#include <condition_variable>
#include <future>
#include <list>
#include <map>

namespace r_utils
{

template<typename CMD, typename RESULT>
class r_work_q final
{
public:
    std::future<RESULT> post(const CMD& cmd)
    {
        std::unique_lock<std::mutex> g(_lock);

        std::promise<RESULT> p;
        auto waiter = p.get_future();

        _queue.push_front(make_pair(cmd, std::move(p)));

        _cond.notify_one();

        return std::move(waiter);
    }

    r_utils::r_nullable<std::pair<CMD,std::promise<RESULT>>> poll()
    {
        std::unique_lock<std::mutex> g(_lock);

        if(_queue.empty())
        {
            _asleep = true;
            _cond.wait(g, [this](){return !this->_queue.empty() || !this->_asleep;});
        }

        r_utils::r_nullable<std::pair<CMD,std::promise<RESULT>>> result;

        if(!_queue.empty())
        {
            result.assign(std::move(_queue.back()));
            _queue.pop_back();
        }

        return result;
    }

    void wake()
    {
        std::unique_lock<std::mutex> g(_lock);
        _asleep = false;
        _cond.notify_one();
    }

private:
    std::mutex _lock;
    std::condition_variable _cond;
    std::list<std::pair<CMD,std::promise<RESULT>>> _queue;
    bool _asleep {false};
};

}

#endif
