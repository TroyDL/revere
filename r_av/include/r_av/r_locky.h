
#ifndef __r_av_r_locky_h
#define __r_av_r_locky_h

#include <mutex>
#include <list>
#include <memory>

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavdevice/avdevice.h"
#include "libavfilter/avfilter.h"
#include "libavformat/avformat.h"
};

namespace r_av
{

class r_locky
{
public:
    static void register_ffmpeg();
    static void unregister_ffmpeg();
    static bool is_registered();

private:
    r_locky();
    r_locky( const r_locky& obj );
    r_locky& operator = ( const r_locky& obj );

    static std::recursive_mutex* _create_lock();
    static void _destroy_lock( std::recursive_mutex* lock );

    static int _r_locky_cb( void** mutex, enum AVLockOp op );

    static std::list<std::shared_ptr<std::recursive_mutex> > _locks;
    static bool _registered;
};

}

#endif
