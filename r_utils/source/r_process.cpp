
#include "r_utils/r_process.h"
#include "r_utils/r_file.h"
#include <string.h>
#include <vector>

using namespace r_utils;
using namespace std;
using namespace std::chrono;

r_process::r_process(const string& cmd) :
    _pid(),
    _cmd(cmd),
    _iopipe(),
    _stdout(NULL)
{
}

r_process::~r_process() noexcept
{
    if(_pid.valid())
    {
        int status;
        waitpid(_pid.pid, &status, 0);

		if (_stdout)
			fclose(_stdout);
    }
}

void r_process::start()
{
    if(pipe(_iopipe) < 0)
        R_THROW(("Unable to open pipe."));

    _pid.pid = fork();
    if(_pid.pid < 0)
        R_THROW(("Unable to fork()."));

    if(_pid.pid == 0) // 0 is returned in child...
    {
        setpgid(0, 0); // 0 here is special case that means set pgid to pid of caller.

        dup2(_iopipe[1], STDOUT_FILENO);
        close(_iopipe[0]);
        close(_iopipe[1]);

        vector<string> parts;

        // work left to right
        // if you see whitespace, make command part
        // if you see quote, find terminating quote and make command part

        size_t ps=0, pe=0, cmdLen = _cmd.length();
        while(ps < cmdLen)
        {
            bool inQuote = false;

            while(_cmd[ps] == ' ' && ps < cmdLen)
            {
                ++ps;
                pe=ps;
            }

            if(_cmd[ps] == '"')
                inQuote = true;

            while((inQuote || _cmd[pe] != ' ') && pe < cmdLen)
            {
                ++pe;
                if(_cmd[pe] == '"')
                {
                    if(inQuote)
                    {
                        // push command ps+1 -> pe
                        parts.push_back(string(&_cmd[ps+1],pe-(ps+1)));
                        inQuote = false;
                        ps = pe+1;
                        break;
                    }
                    else
                    {
                        inQuote = true;
                    }
                }
                else if((!inQuote && _cmd[pe] == ' ') || pe == cmdLen)
                {
                    // push command ps -> pe
                    parts.push_back(string(&_cmd[ps],pe-ps));
                    ps = pe;
                    break;
                }
            }
        }

        vector<const char*> partPtrs;
        for( auto& p : parts )
            partPtrs.push_back(p.c_str());
        partPtrs.push_back(NULL);
        execve( parts[0].c_str(), (char* const*)&partPtrs[0], NULL );
        R_THROW(("Failed to execve()."));
    }
    else
    {
        close(_iopipe[1]);
        _stdout = fdopen(_iopipe[0], "rb");
        if(!_stdout)
            R_THROW(("Unable to open process stdout."));
    }
}

r_wait_status r_process::wait_for(int& code, milliseconds timeout)
{
    auto remaining = timeout;
    while(duration_cast<milliseconds>(remaining).count() > 0)
    {
        auto before = steady_clock::now();

        int status;
        int res = waitpid(_pid.pid, &status, WNOHANG);
        if( res > 0 )
        {
            code = WEXITSTATUS(status);
            _pid.clear();
            return R_PROCESS_EXITED;
        }

        if( res < 0 )
            R_THROW(("Unable to waitpid()."));

        usleep(250000);

        auto after = steady_clock::now();

        auto delta = duration_cast<milliseconds>(after - before);

        if( remaining > delta )
            remaining -= delta;
        else remaining = milliseconds::zero();
    }

    return R_PROCESS_WAIT_TIMEDOUT;
}

void r_process::kill()
{
    ::kill(-_pid.pid, SIGKILL); //negated pid means kill entire process group id
}

size_t r_process::stdout_read(void* ptr, size_t size, size_t nmemb)
{
    if(!_stdout)
        R_THROW(("Cannot read from null stream."));
    return fread(ptr, size, nmemb, _stdout);
}

bool r_process::stdout_eof()
{
    return (feof(_stdout) > 0) ? true : false;
}

r_pid r_process::get_current_pid()
{
    r_pid p;
    p.pid = getpid();
    return p;
}

list<r_pid> r_process::get_child_processes(const r_pid& parentPID)
{
    list<r_pid> output;

    r_path path("/proc");

    string fileName;
    while(path.read_dir(fileName))
    {
        if(r_string_utils::is_integer(fileName) && r_utils::r_fs::is_dir("/proc/" + fileName))
        {
            string statusPath = "/proc/" + fileName + "/status";
            if(r_utils::r_fs::file_exists(statusPath))
            {
                FILE* inFile = fopen(statusPath.c_str(), "rb");
		        if(!inFile)
		            R_THROW(("Unable to open status file."));

        		char* lineBuffer = NULL;

        		try
        		{
  		            ssize_t bytesRead = 0;
		            do
		            {
		                lineBuffer = NULL;
			            size_t lineSize = 0;
			            bytesRead = getline(&lineBuffer, &lineSize, inFile);

			            if(bytesRead < 0)
			                continue;

			            string line = lineBuffer;

			            vector<string> parts = r_utils::r_string_utils::split(line, ':');

			            if(parts.size() == 2)
			            {
			                if(parts[0] == "PPid")
			                {
			                    string foundPPID = r_utils::r_string_utils::strip(parts[1]);

				                if(r_utils::r_string_utils::s_to_int(foundPPID) == parentPID.pid)
				                {
				                    r_pid pid;
				                    pid.pid = r_utils::r_string_utils::s_to_int(fileName);
				                    output.push_back(pid);
				                }
			                }
			            }

			            if(lineBuffer)
			            {
			                free(lineBuffer);
			                lineBuffer = NULL;
			            }
                    }
		            while(bytesRead >= 0);

		            fclose(inFile);
		        }
		        catch(...)
		        {
  		            if(inFile)
  		                fclose(inFile);
		            if(lineBuffer)
  		                free(lineBuffer);
		            throw;
		        }
            }
        }
    }

    return output;
}

list<r_pid> r_process::get_processes_for_module(const string& moduleName)
{
    list<r_pid> output;

    r_path path("/proc");

    string procName;
    while(path.read_dir(procName))
    {
        if(r_utils::r_string_utils::is_integer(procName) && r_utils::r_fs::is_dir("/proc/" + procName))
        {
            char exe[1024];
            memset(exe,0,1024);
            string path = "/proc/" + procName + "/exe";
            auto ret = readlink( path.c_str(), exe, sizeof(exe)-1 );
            if(ret < 0)
                R_THROW(("Unable to find processes for module(%s).",moduleName.c_str()));
            string module = exe;
            if(r_utils::r_string_utils::contains(module, moduleName))
            {
                r_pid pid;
                pid.pid = r_utils::r_string_utils::s_to_int(procName);
                output.push_back( pid );
            }
        }
    }

    return output;
}
