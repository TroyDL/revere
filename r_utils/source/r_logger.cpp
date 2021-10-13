
#include "r_utils/r_logger.h"
#include "r_utils/r_string_utils.h"
#include "r_utils/r_exception.h"
#include <exception>

using namespace r_utils;
using namespace std;

static FILE* _log_file = nullptr;

void r_utils::r_logger::write(LOG_LEVEL level,
                              int line,
                              const char* file,
                              const char* format,
                              ...)
{
    va_list args;
    va_start(args, format);
    r_logger::write(level, line, nullptr, format, args);
    va_end(args);
}

void r_utils::r_logger::write(LOG_LEVEL level,
                              int line,
                              const char* file,
                              const char* format,
                              va_list& args)
{
#ifdef IS_WINDOWS
    if(_log_file)
    {
        auto msg = r_string_utils::format(format, args);
        fprintf(_log_file, "%s\n", msg.c_str());
        fflush(_log_file);
    }
#else
    int priority = LOG_USER;

    switch(level)
    {
        case LOG_LEVEL_CRITICAL: { priority |= LOG_CRIT; break; }
        case LOG_LEVEL_ERROR: { priority |= LOG_ERR; break; }
        case LOG_LEVEL_WARNING: { priority |= LOG_WARNING; break; }
        case LOG_LEVEL_NOTICE: { priority |= LOG_NOTICE; break; }
        case LOG_LEVEL_INFO: { priority |= LOG_INFO; break; }
        case LOG_LEVEL_TRACE: { priority |= LOG_INFO; break; }
        case LOG_LEVEL_DEBUG: { priority |= LOG_INFO; break; }
        default: break;
    };

    vsyslog(priority, format, args);
#endif
}

void r_utils_terminate()
{
    R_LOG_CRITICAL("r_utils terminate handler called!");
    printf("r_utils terminate handler called!\n");

    std::exception_ptr p = std::current_exception();

    if(p)
    {
        try
        {
            std::rethrow_exception(p);
        }
        catch(std::exception& ex)
        {
            R_LOG_CRITICAL("%s",ex.what());
            printf("%s\n",ex.what());
        }
        catch(...)
        {
            R_LOG_CRITICAL("unknown exception in r_utils_terminate().");
        }
    }

    fflush(stdout);

    std::abort();
}

void r_utils::r_logger::install_logger()
{
#ifdef IS_WINDOWS
    if(fopen_s(&_log_file, "log.txt", "a+") != 0)
        R_THROW(("Unable to open logger."));
#endif
}

void r_utils::r_logger::uninstall_logger()
{
#ifdef IS_WINDOWS
    if(_log_file)
    {
        fclose(_log_file);
        _log_file = nullptr;
    }
#endif
}

void r_utils::r_logger::install_terminate()
{
    set_terminate(r_utils_terminate);
}
