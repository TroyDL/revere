
#ifndef r_utils_r_file_lock_h
#define r_utils_r_file_lock_h

class test_r_utils_r_file_lock;

namespace r_utils
{

class r_file_lock final
{
public:
    r_file_lock(int fd=-1);
    r_file_lock(const r_file_lock&) = delete;
    r_file_lock( r_file_lock&& obj ) noexcept;
    ~r_file_lock() noexcept;

    r_file_lock& operator=(const r_file_lock&) = delete;
    r_file_lock& operator=(r_file_lock&& obj) noexcept;

    void lock(bool exclusive = true);
    void unlock();

private:
    int _fd;
};

class r_file_lock_guard final
{
public:
    r_file_lock_guard(r_file_lock& lok, bool exclusive = true);
    r_file_lock_guard(const r_file_lock_guard&) = delete;
    r_file_lock_guard(r_file_lock&&) = delete;
    ~r_file_lock_guard() noexcept;

    r_file_lock_guard& operator=(const r_file_lock_guard&) = delete;
    r_file_lock_guard& operator=(r_file_lock_guard&&) = delete;

private:
    r_file_lock& _lok;
};

}

#endif
