
#include "r_av/r_locky.h"
#include "r_utils/r_logger.h"
#include "r_utils/r_macro.h"

using namespace r_av;
using namespace std;
using namespace r_utils;

list<shared_ptr<recursive_mutex> > r_locky::_locks;
bool r_locky::_registered = false;

void r_locky::register_ffmpeg()
{
    //avcodec_register_all();
    avdevice_register_all();
    //avfilter_register_all();

    //av_register_all();

    avformat_network_init();

    //av_lockmgr_register( r_locky::_r_locky_cb );

    _registered = true;

    FULL_MEM_BARRIER();
}

void r_locky::unregister_ffmpeg()
{
    avformat_network_deinit();

    //av_lockmgr_register( nullptr );

    r_locky::_locks.clear();

    _registered = false;

    FULL_MEM_BARRIER();
}

bool r_locky::is_registered()
{
    return r_locky::_registered;
}

recursive_mutex* r_locky::_create_lock()
{
    shared_ptr<recursive_mutex> newLock = make_shared<recursive_mutex>();
    recursive_mutex* newLockPtr = newLock.get();

    r_locky::_locks.push_back( newLock );

    return newLockPtr;
}

void r_locky::_destroy_lock( recursive_mutex* lock )
{
    for( auto i = _locks.begin(), end = _locks.end(); i != end; i++ )
    {
        auto lockRef = *i;
        if( lockRef.get() == lock )
        {
            _locks.erase( i );
            return;
        }
    }

    R_LOG_NOTICE( "Locky asked to clean up a lock that was not managed!" );
}

int r_locky::_r_locky_cb( void** mutex, enum AVLockOp op )
{
    switch( op )
    {
    case AV_LOCK_CREATE:
        *mutex = r_locky::_create_lock();
        break;
    case AV_LOCK_DESTROY:
        r_locky::_destroy_lock( (recursive_mutex*)*mutex );
        break;
    case AV_LOCK_OBTAIN:
        ((recursive_mutex*)(*mutex))->lock();
        break;
    case AV_LOCK_RELEASE:
        ((recursive_mutex*)(*mutex))->unlock();
        break;
    default:
        break;
    };

    return 0;
}
