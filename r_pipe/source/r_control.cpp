
#include "r_pipe/r_control.h"

using namespace r_pipe;
using namespace r_utils;
using namespace std;

r_control::r_control() :
    r_stateful(),
    _elementIndex(),
    _elements(),
    _kvs()
{
}

r_control::~r_control() noexcept
{
    stop();
}

void r_control::run()
{
    list<shared_ptr<r_media_object> >::reverse_iterator i, esentry;
    for( i = _elements.rbegin(), esentry = _elements.rend(); i != esentry; i++ )
        (*i)->run();
}

void r_control::stop()
{
    list<shared_ptr<r_media_object> >::reverse_iterator i, esentry;
    for( i = _elements.rbegin(), esentry = _elements.rend(); i != esentry; i++ )
        (*i)->stop();
}

void r_control::set_param( const string& key, const string& name, const string& val )
{
    _elementIndex[key]->set_param( name, val );
}

void r_control::commit_params( const string& key )
{
    _elementIndex[key]->commit_params();
}

void r_control::add_filter( const string& key, shared_ptr<r_filter> filter )
{
    shared_ptr<r_media_object> p = make_shared<r_media_object>();
    p->attach_filter( filter );

    _elementIndex.insert( make_pair(key, p) );
    _elements.push_back( p );
}

void r_control::add_source( const string& key, shared_ptr<r_source> source )
{
    shared_ptr<r_media_object> p = make_shared<r_media_object>();
    p->set_source( source );

    _elementIndex.insert( make_pair(key, p) );
    _elements.push_back( p );
}

void r_control::connect( const string& keyA, const string& keyB )
{
    _elementIndex[keyA]->connect( _elementIndex[keyB] );
}

void r_control::add_media_object( const string& key, shared_ptr<r_media_object> mo )
{
    _elementIndex.insert( make_pair(key, mo) );
    _elements.push_back( mo );
}

std::shared_ptr<r_media_object> r_control::get_media_object( const string& key )
{
    auto found = _elementIndex.find( key );
    if( found == _elementIndex.end() )
        R_THROW(("Unable to file media object with name: %s",key.c_str()));
    return found->second;
}

bool r_control::healthy() const
{
    bool healthy = true;
    for( auto& mo : _elements )
    {
        if( !mo->healthy() )
            healthy = false;
    }
    return healthy;
}

void r_control::set_kv( const string& key, const string& val )
{
    _kvs[key] = val;
}

r_nullable<string> r_control::get_kv( const string& key ) const
{
    r_nullable<string> result;

    auto f = _kvs.find(key);
    if(f != _kvs.end())
        result.set_value(f->second);

    return result;
}
