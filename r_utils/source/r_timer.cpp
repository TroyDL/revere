
#include "r_utils/r_timer.h"
#include <ctime>

using namespace r_utils;
using namespace std;
using namespace std::chrono;

r_timer::r_timer( size_t intervalMillis, r_timer_cb cb ) :
    _thread(),
    _intervalMillis( intervalMillis ),
    _cb( cb ),
    _lok(),
    _cond(),
    _started( false )
{
}

r_timer::~r_timer() throw()
{
    if( _started )
        stop();
}

void r_timer::start()
{
    if( !_started )
    {
        _started = true;
        _thread = std::thread( &r_timer::_timer_loop, this );
    }
}

void r_timer::stop()
{
    if( _started )
    {
        {
            unique_lock<recursive_mutex> g( _lok );
            _started = false;
            _cond.notify_one();
        }

        _thread.join();
    }
}

void r_timer::_timer_loop()
{
    size_t millisToWait = _intervalMillis;

    while( _started )
    {
        unique_lock<recursive_mutex> g( _lok );

        steady_clock::time_point before = steady_clock::now();

        auto result = (millisToWait>0)?_cond.wait_for( g, chrono::milliseconds( millisToWait ) ) : cv_status::timeout;

        steady_clock::time_point after = steady_clock::now();

        if( !_started )
            continue;

        size_t waitedMillis = chrono::duration_cast<chrono::milliseconds>(after - before).count();

        if( result == cv_status::timeout )
        {
            millisToWait = _intervalMillis;
            _cb();
        }
        else
        {
            if( millisToWait > waitedMillis )
                millisToWait -= waitedMillis;
            else millisToWait = 0;
        }
    }
}
