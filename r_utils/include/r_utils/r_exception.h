
#ifndef r_utils_r_exception_h
#define r_utils_r_exception_h

#include "r_utils/r_logger.h"
#include "r_utils/r_macro.h"
#include <string>
#include <exception>
#include <assert.h>

namespace r_utils
{

class r_exception : public std::exception
{
public:
    r_exception();
    r_exception(const std::string& msg);
    r_exception(const char* msg, ...);
    virtual ~r_exception() noexcept;

    void set_msg(const std::string& msg) { _msg = msg; }

    virtual const char* what() const noexcept;

protected:
    mutable std::string _msg;
    std::string _stack;
};

class r_not_found_exception : public r_exception
{
public:
    r_not_found_exception();
    virtual ~r_not_found_exception() noexcept {}
    r_not_found_exception(const char* msg, ...);
};

class r_invalid_argument_exception : public r_exception
{
public:
    r_invalid_argument_exception();
    virtual ~r_invalid_argument_exception() noexcept {}
    r_invalid_argument_exception(const char* msg, ...);
};

class r_unauthorized_exception : public r_exception
{
public:
    r_unauthorized_exception();
    virtual ~r_unauthorized_exception() noexcept {}
    r_unauthorized_exception(const char* msg, ...);
};

class r_not_implemented_exception : public r_exception
{
public:
    r_not_implemented_exception();
    virtual ~r_not_implemented_exception() noexcept {}
    r_not_implemented_exception(const char* msg, ...);
};

class r_timeout_exception : public r_exception
{
public:
    r_timeout_exception();
    virtual ~r_timeout_exception() noexcept {}
    r_timeout_exception(const char* msg, ...);
};

class r_io_exception : public r_exception
{
public:
    r_io_exception();
    virtual ~r_io_exception() noexcept {}
    r_io_exception(const char* msg, ...);
};

class r_internal_exception : public r_exception
{
public:
    r_internal_exception();
    virtual ~r_internal_exception() noexcept {}
    r_internal_exception(const char* msg, ...);
};

}

#define R_THROW(ARGS) throw r_utils::r_exception ARGS ;

#define R_STHROW(EXTYPE, ARGS) throw EXTYPE ARGS ;

#define R_LOG_EXCEPTION(E) \
R_MACRO_BEGIN \
    auto parts = r_utils::r_string::split(std::string(E.what()), '\n'); \
    for( auto l : parts ) \
        R_LOG_ERROR("%s",l.c_str()); \
R_MACRO_END

#endif
