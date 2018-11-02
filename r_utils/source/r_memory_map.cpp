
#include "r_utils/r_memory_map.h"
#include "r_utils/r_exception.h"
#include <sys/mman.h>

using namespace r_utils;

r_memory_map::r_memory_map(r_memory_map&& obj) noexcept :
    _mem(std::move(obj._mem)),
    _length(std::move(obj._length))
{
    obj._mem = nullptr;
    obj._length = 0;
}

r_memory_map::r_memory_map(int fd, uint64_t offset, uint64_t len, uint32_t prot, uint32_t flags) :
    _mem(nullptr),
    _length(len)
{
    if(fd <= 0)
        R_STHROW(r_invalid_argument_exception, ("Attempting to memory map a bad file descriptor."));
    if(len == 0)
        R_STHROW(r_invalid_argument_exception, ("Unable to memory map 0 length."));
    if(!(flags & mm_type_file) && !(flags & mm_type_anon))
        R_STHROW(r_invalid_argument_exception, ("A mapping must be either a file mapping, or an anonymous mapping (neither was specified)."));
    if(flags & mm_fixed)
        R_STHROW(r_invalid_argument_exception, ("r_memory_map does not support fixed mappings."));

    _mem = mmap64(nullptr, _length, _get_posix_prot_flags(prot), _get_posix_access_flags(flags), fd, offset);
}

r_memory_map::~r_memory_map() noexcept
{
    _close();
}

r_memory_map& r_memory_map::operator=(r_memory_map&& obj) noexcept
{
    _close();
    _mem = std::move(obj._mem);
    obj._mem = nullptr;
    _length = std::move(obj._length);
    obj._length = 0;
    return *this;
}

void r_memory_map::advise(void* addr, size_t length, int advice) const
{
    if(madvise(addr, length, _get_posix_advice(advice)) != 0)
        R_STHROW(r_internal_exception, ("Unable to apply memory mapping advice."));
}

void r_memory_map::sync(const r_byte_ptr_rw& p, int flags) const
{
    if(msync(p.get_ptr(), p.length(), _get_posix_sync_flags(flags)) < 0)
        R_STHROW(r_internal_exception, ("Unable to msync()!"));
}

void r_memory_map::_close() noexcept
{
    if(_mem)
        munmap(_mem, _length);
}

int r_memory_map::_get_posix_prot_flags(int prot) const
{
    int osProtFlags = 0;

    if(prot & mm_prot_read)
        osProtFlags |= PROT_READ;
    if(prot & mm_prot_write)
        osProtFlags |= PROT_WRITE;
    if(prot & mm_prot_exec)
        osProtFlags |= PROT_EXEC;

    return osProtFlags;
}

int r_memory_map::_get_posix_access_flags(int flags) const
{
    int osFlags = 0;

    if(flags & mm_type_file)
        osFlags |= MAP_FILE;
    if(flags & mm_type_anon)
        osFlags |= MAP_ANONYMOUS;
    if(flags & mm_shared)
        osFlags |= MAP_SHARED;
    if(flags & mm_private)
        osFlags |= MAP_PRIVATE;
    if(flags & mm_fixed)
        osFlags |= MAP_FIXED;

    return osFlags;
}

int r_memory_map::_get_posix_advice(int advice) const
{
    int posixAdvice = 0;

    if(advice & mm_advice_random)
        posixAdvice |= MADV_RANDOM;
    if(advice & mm_advice_sequential)
        posixAdvice |= MADV_SEQUENTIAL;
    if(advice & mm_advice_willneed)
        posixAdvice |= MADV_WILLNEED;
    if(advice & mm_advice_dontneed)
        posixAdvice |= MADV_DONTNEED;

    return posixAdvice;
}

int r_memory_map::_get_posix_sync_flags(int flags) const
{
    int posixSync = 0;

    if(flags & mm_sync_async)
        posixSync |= MS_ASYNC;
    if(flags & mm_sync_sync)
        posixSync |= MS_SYNC;
    if(flags & mm_sync_invalidate)
        posixSync |= MS_INVALIDATE;

    return posixSync;
}