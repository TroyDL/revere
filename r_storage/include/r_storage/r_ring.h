
#ifndef __r_storage_r_ring_h
#define __r_storage_r_ring_h

#include "r_utils/r_file.h"
#include "r_utils/r_file_lock.h"
#include "r_utils/r_memory_map.h"
#include "r_utils/r_exception.h"
#include <memory>
#include <string>
#include <chrono>

namespace r_storage
{

class r_ring final
{
public:
    r_ring(const std::string& path, size_t element_size);
    r_ring(const r_ring&) = delete;
    r_ring(r_ring&& other) noexcept = delete;
    ~r_ring() noexcept;
    
    r_ring& operator=(const r_ring&) = delete;
    r_ring& operator=(r_ring&& other) noexcept = delete;

    void write(const uint8_t* p);

    template<typename CB>
    void query(const std::chrono::system_clock::time_point& qs, const std::chrono::system_clock::time_point& qe, CB cb)
    {
        r_utils::r_file_lock_guard g(_lock, false);
        auto n_elements = _n_elements();

        auto now = std::chrono::system_clock::now();
        if(qe <= qs)
            R_THROW(("invalid query"));

        auto start_idx = _idx(qs);
        auto elements_to_query = std::chrono::duration_cast<std::chrono::seconds>(qe-qs).count();
        for(auto i = 0; i < elements_to_query; i++)
        {
            uint8_t* element = _ring_start() + (((start_idx + i) % n_elements) * _element_size);
            cb(element);
        }
    }

    static void allocate(const std::string& path, size_t element_size, size_t n_elements);

private:
    uint8_t* _ring_start() const;
    size_t _n_elements() const;
    std::chrono::system_clock::time_point _created_at() const;
    size_t _idx(const std::chrono::system_clock::time_point& tp) const;
    size_t _unwrapped_idx(const std::chrono::system_clock::time_point& tp) const;

    r_utils::r_file _file;
    r_utils::r_file_lock _lock;
    size_t _element_size;
    size_t _file_size;
    r_utils::r_memory_map _map;
    int64_t _last_write_idx;
};

}

#endif
