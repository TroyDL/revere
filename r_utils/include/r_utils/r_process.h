#ifndef _r_utils_r_process_h
#define _r_utils_r_process_h

#include "r_utils/r_string.h"
#include "r_utils/r_file.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <list>
#include <chrono>

namespace r_utils
{

struct r_pid
{
    pid_t pid;

    r_pid() { pid = -1; }
    r_pid(const r_pid& obj) = default;
    r_pid(r_pid&&) = default;
    r_pid& operator=(const r_pid&) = default;
    r_pid& operator=(r_pid&&) = default;
    bool operator==(const r_pid& obj) { return (pid == obj.pid)?true:false; }

    bool valid() const { return (pid >= 0)?true:false; }
    void clear() { pid = -1; }
};

enum r_wait_status
{
    R_PROCESS_EXITED,
    R_PROCESS_WAIT_TIMEDOUT
};

class r_process
{
public:
    r_process( const std::string& cmd );
    virtual ~r_process() noexcept;
    void start();
    r_pid get_pid() const { return _pid; }
    r_wait_status wait_for(int& code, std::chrono::milliseconds timeout);
    void kill();
    size_t stdout_read(void* ptr, size_t size, size_t nmemb);
    bool stdout_eof();

    static r_pid get_current_pid();
    static std::list<r_pid> get_child_processes(const r_pid& parentPID);
    static std::list<r_pid> get_processes_for_module(const std::string& moduleName);

private:
    r_pid _pid;
    std::string _cmd;
    int _iopipe[2];
    FILE* _stdout;
};

}

#endif
