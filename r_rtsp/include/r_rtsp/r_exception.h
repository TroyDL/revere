
#ifndef r_rtsp_r_rtsp_exception_h
#define r_rtsp_r_rtsp_exception_h

#include "r_utils/r_exception.h"
#include "r_utils/r_string.h"

namespace r_rtsp
{

class r_rtsp_exception : public r_utils::r_exception
{
public:
    r_rtsp_exception();
    r_rtsp_exception( const char* msg, ... );
    r_rtsp_exception( const std::string& msg );
    virtual ~r_rtsp_exception() throw() {}
};

class rtsp_exception : public r_rtsp_exception
{
public:
    rtsp_exception( int statusCode );
    rtsp_exception( int statusCode, const char* msg, ... );
    rtsp_exception( int statusCode, const std::string& msg );
    virtual ~rtsp_exception() throw() {}
    int get_status() const { return _statusCode; }
private:
    int _statusCode;
};

#define rtsp_exception_class(code) \
class rtsp_##code##_exception : public rtsp_exception \
{ \
public: \
    rtsp_##code##_exception() : \
        rtsp_exception( code ) \
    { \
    } \
    rtsp_##code##_exception( const char* msg, ... ) : \
        rtsp_exception( code ) \
    { \
        va_list args; \
        va_start( args, msg ); \
        set_msg( r_utils::r_string::format( msg, args ) );      \
        va_end( args ); \
    } \
    rtsp_##code##_exception( const std::string& msg ) : \
        rtsp_exception( code, msg ) \
    { \
    } \
    virtual ~rtsp_##code##_exception() throw() {} \
    int get_status() const { return _statusCode; } \
private: \
    int _statusCode; \
}

rtsp_exception_class(400);
rtsp_exception_class(401);
rtsp_exception_class(403);
rtsp_exception_class(404);
rtsp_exception_class(415);
rtsp_exception_class(453);
rtsp_exception_class(500);

/// \brief Throws an exception corresponding to the given status code
///        or a plain rtsp_exception if there isn't one.
void throw_rtsp_exception( int statusCode, const char* msg, ... );
void throw_rtsp_exception( int statusCode, const std::string& msg );

#if 0
#define RTSP_THROW(PARAMS) \
CK_MACRO_BEGIN \
    try \
    { \
        throw_rtsp_exception PARAMS ; \
    } \
    catch(rtsp_exception& e) \
    { \
        R_LOG_WARNING("Exception thrown. Msg: \"%s\", At: %s(%d)\n", \
                       e.what(),                            \
                       __FILE__,                               \
                       __LINE__);                              \
        throw; \
    } \
CK_MACRO_END
#endif

}

#endif
