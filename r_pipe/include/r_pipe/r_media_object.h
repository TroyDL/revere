#ifndef __r_pipe_r_media_object_h
#define __r_pipe_r_media_object_h

#include "r_pipe/r_source.h"
#include "r_pipe/r_filter.h"
#include "r_av/r_packet.h"

#include <mutex>
#include <thread>
#include <condition_variable>
#include <list>
#include <memory>
#include <stdexcept>
#include <iostream>

namespace r_pipe
{

class r_media_object : public r_stateful
{
public:
    r_media_object();
    virtual ~r_media_object() noexcept;

    virtual void run();
    virtual void stop();

    void set_param( const std::string& name, const std::string& val );
    void commit_params();

    bool healthy() const { return !_errorState; }

    void write( r_av::r_packet& pkt );

    void set_source( std::shared_ptr<r_source> source ) { _source = source; }

    void disconnect_all() { _next.clear(); }

    void connect( std::shared_ptr<r_media_object> next ) { _next.push_back( next ); }

    void attach_filter( std::shared_ptr<r_filter> filter ) { _filter = filter; }
    std::shared_ptr<r_filter> get_filter() const { return _filter; }

    virtual void* entry_point();

private:
    inline bool _queue_full() const
    {
        if( _inputQueue.size() >= _queueLimit )
            return true;
        return false;
    }

    void _enqueue( r_av::r_packet& pkt );
    void* _default_entry_point();
    void* _source_entry_point();

    std::list<std::shared_ptr<r_media_object> > _next;
    std::list<r_av::r_packet> _inputQueue;
    size_t _queueLimit;
    std::recursive_mutex _queueLock;
    std::condition_variable_any _queueCond;
    std::recursive_mutex _producerLock;
    std::condition_variable_any  _producerCond;
    bool _running;
    std::shared_ptr<r_filter> _filter;
    std::shared_ptr<r_source> _source;
    bool _errorState;
    std::thread _thread;
};

}

#endif
