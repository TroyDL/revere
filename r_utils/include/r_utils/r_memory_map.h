
#ifndef __r_utils_r_memory_map_h
#define __r_utils_r_memory_map_h

#include "r_utils/r_string_utils.h"
#include "r_utils/r_byte_ptr.h"
#include "r_utils/r_macro.h"

namespace r_utils
{

class r_memory_map final
{
public:
    enum flags
    {
        mm_type_file = 0x01,
        mm_type_anon = 0x02,
        mm_shared = 0x04,
        mm_private = 0x08,
        mm_fixed = 0x10
    };

    enum protection
    {
        mm_prot_name = 0x00,
        mm_prot_read = 0x01,
        mm_prot_write = 0x02,
        mm_prot_exec = 0x04
    };

    enum advice
    {
        mm_advice_normal = 0x00,
        mm_advice_random = 0x01,
        mm_advice_sequential = 0x02,
        mm_advice_willneed = 0x04,
        mm_advice_dontneed = 0x08
    };

    enum synchronous
    {
        mm_sync_async = 0x00,
        mm_sync_sync = 0x01,
        mm_sync_invalidate = 0x02
    };

    r_memory_map() : _mem(nullptr), _length(0) {}
    r_memory_map(const r_memory_map&) = delete;
    r_memory_map(r_memory_map&& obj) noexcept;
    r_memory_map(int fd, uint64_t offset, uint64_t len, uint32_t prot, uint32_t flags);

    ~r_memory_map() noexcept;

    r_memory_map& operator=(const r_memory_map&) = delete;
    r_memory_map& operator=(r_memory_map&&) noexcept;

    r_byte_ptr_rw map() const { return r_byte_ptr_rw((uint8_t*)_mem, _length); }
    uint64_t size() const { return _length; }

    void advise(void* addr, size_t length, int advice) const;

    void sync(const r_byte_ptr_rw& p, int flags) const;

private:
    void _close() noexcept;

    int _get_posix_prot_flags(int prot) const;
    int _get_posix_access_flags(int flags) const;
    int _get_posix_advice(int advice) const;
    int _get_posix_sync_flags(int flags) const;

    void* _mem;
    uint64_t _length;
};

}

#endif
