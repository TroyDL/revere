
#include "r_utils/r_file_lock.h"
#include "r_utils/r_exception.h"
#include <sys/file.h>
#include <utility>

using namespace r_utils;
using namespace std;

r_file_lock::r_file_lock(int fd) :
    _fd(fd)
{
}

r_file_lock::r_file_lock(r_file_lock&& obj) noexcept :
    _fd(std::move(obj._fd))
{
}

r_file_lock& r_file_lock::operator=(r_file_lock&& obj) noexcept
{
    _fd = std::move(obj._fd);
    obj._fd = -1;

    return *this;
}

void r_file_lock::lock(bool exclusive)
{
    if(flock(_fd, (exclusive)?LOCK_EX:LOCK_SH) < 0)
        R_STHROW(r_internal_exception, ("Unable to flock() file."));
}

void r_file_lock::unlock()
{
    if(flock(_fd, LOCK_UN) < 0)
        R_STHROW(r_internal_exception, ("Unable to un-flock()!"));
}

r_file_lock_guard::r_file_lock_guard(r_file_lock& lok, bool exclusive) :
    _lok(lok)
{
    _lok.lock(exclusive);
}

r_file_lock_guard::~r_file_lock_guard() noexcept
{
    _lok.unlock();
}
