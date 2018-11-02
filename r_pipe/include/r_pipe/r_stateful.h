#ifndef __r_pipe_r_stateful_h
#define __r_pipe_r_stateful_h

namespace r_pipe
{

class r_stateful
{
public:
    r_stateful() = default;
    virtual ~r_stateful() noexcept {}

    virtual void run() = 0;
    // stop() should never throw, but is not marked nothrow
    // because our media objects actually handle this condition (sort of).
    virtual void stop() = 0;
};

}

#endif
