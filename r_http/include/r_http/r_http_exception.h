
#ifndef r_http_r_http_exception_h
#define r_http_r_http_exception_h

#include "r_utils/r_exception.h"
#include "r_utils/r_string.h"

namespace r_http
{

/// An internal exception type used to enter io error state.
class r_http_io_exception : public r_utils::r_exception
{
public:
    r_http_io_exception();

    virtual ~r_http_io_exception() noexcept {}

    r_http_io_exception(const char* msg, ...);

    r_http_io_exception(const std::string& msg);
};

/// General exception type for Webby.
class r_http_exception_generic : public r_utils::r_exception
{
public:
    r_http_exception_generic();

    virtual ~r_http_exception_generic() noexcept {}

    r_http_exception_generic(const char* msg, ...);

    r_http_exception_generic(const std::string& msg);
};

class r_http_exception : public r_http_exception_generic
{
public:
    r_http_exception( int statusCode );

    virtual ~r_http_exception() noexcept {}

    r_http_exception( int statusCode, const char* msg, ... );

    r_http_exception( int statusCode, const std::string& msg );

    int get_status() const { return _statusCode; }

private:
    int _statusCode;
};

class r_http_400_exception : public r_http_exception
{
public:
    r_http_400_exception();

    virtual ~r_http_400_exception() noexcept {}

    r_http_400_exception( const char* msg, ... );

    r_http_400_exception( const std::string& msg );
};

class r_http_401_exception : public r_http_exception
{
public:
    r_http_401_exception();

    virtual ~r_http_401_exception() noexcept {}

    r_http_401_exception( const char* msg, ... );

    r_http_401_exception( const std::string& msg );
};

class r_http_403_exception : public r_http_exception
{
public:
    r_http_403_exception();

    virtual ~r_http_403_exception() noexcept {}

    r_http_403_exception( const char* msg, ... );

    r_http_403_exception( const std::string& msg );
};

class r_http_404_exception : public r_http_exception
{
public:
    r_http_404_exception();

    virtual ~r_http_404_exception() noexcept {}

    r_http_404_exception( const char* msg, ... );

    r_http_404_exception( const std::string& msg );
};

class r_http_415_exception : public r_http_exception
{
public:
    r_http_415_exception();

    virtual ~r_http_415_exception() noexcept {}

    r_http_415_exception( const char* msg, ... );

    r_http_415_exception( const std::string& msg );
};

class r_http_453_exception : public r_http_exception
{
public:
    r_http_453_exception();

    virtual ~r_http_453_exception() noexcept{}

    r_http_453_exception( const char* msg, ... );

    r_http_453_exception( const std::string& msg );
};

class r_http_500_exception : public r_http_exception
{
public:
    r_http_500_exception();

    virtual ~r_http_500_exception() noexcept {}

    r_http_500_exception( const char* msg, ... );

    r_http_500_exception( const std::string& msg );
};

class r_http_501_exception : public r_http_exception
{
public:
    r_http_501_exception();

    virtual ~r_http_501_exception() noexcept {}

    r_http_501_exception( const char* msg, ... );

    r_http_501_exception( const std::string& msg );
};

/// \brief Throws an exception corresponding to the given status code
///        or a plain r_http_exception if there isn't one.
void throw_r_http_exception( int statusCode, const char* msg, ... );
void throw_r_http_exception( int statusCode, const std::string& msg );

#define CATCH_TRANSLATE_HTTP_EXCEPTIONS(a) \
    catch( r_http_400_exception& ex ) \
    { \
        CK_LOG_ERROR( "%s", ex.what() ); \
        a.set_status_code( response_bad_request ); \
    } \
    catch( r_http_401_exception& ex ) \
    { \
        CK_LOG_ERROR( "%s", ex.what() ); \
        response.set_status_code( response_unauthorized ); \
    } \
    catch( r_http_403_exception& ex ) \
    { \
        CK_LOG_ERROR( "%s", ex.what() ); \
        response.set_status_code( response_forbidden ); \
    } \
    catch( r_http_404_exception& ex ) \
    { \
        CK_LOG_ERROR( "%s", ex.what() ); \
        response.set_status_code( response_not_found ); \
    } \
    catch( r_http_500_exception& ex ) \
    { \
        CK_LOG_ERROR( "%s", ex.what() ); \
        response.set_status_code( response_internal_server_error ); \
    } \
    catch( r_http_501_exception& ex ) \
    { \
        CK_LOG_ERROR( "%s", ex.what() ); \
        response.set_status_code( response_not_implemented ); \
    }

}

#endif
