
#ifndef __r_pipe_r_control_h
#define __r_pipe_r_control_h

#include "r_pipe/r_stateful.h"
#include "r_pipe/r_media_object.h"
#include "r_pipe/r_filter.h"

#include "r_utils/r_string.h"
#include "r_utils/r_nullable.h"

#include <memory>
#include <list>
#include <map>

namespace r_pipe
{

class r_control : public r_stateful
{
public:
    r_control();
    virtual ~r_control() noexcept;

    virtual void run();
    virtual void stop();

    void set_param( const std::string& key, const std::string& name, const std::string& val );
    void commit_params( const std::string& key );

    void add_filter( const std::string& key, std::shared_ptr<r_filter> filter );

    void add_source( const std::string& key, std::shared_ptr<r_source> source );

    void connect( const std::string& keyA, const std::string& keyB );

    void add_media_object( const std::string& key, std::shared_ptr<r_media_object> mo );

    std::shared_ptr<r_media_object> get_media_object( const std::string& key );

    bool healthy() const;

    void set_kv( const std::string& key, const std::string& val );
    r_utils::r_nullable<std::string> get_kv( const std::string& key ) const;

private:
    std::map<std::string,std::shared_ptr<r_media_object> > _elementIndex;
    std::list<std::shared_ptr<r_media_object> > _elements;
    std::map<std::string, std::string> _kvs;
};

}

#endif