
#ifndef __r_vss_r_recording_context_h
#define __r_vss_r_recording_context_h

#include "r_disco/r_camera.h"
#include "r_pipeline/r_gst_source.h"
#include "r_storage/r_storage_file.h"
#include <mutex>
#include <chrono>

namespace r_vss
{

class r_recording_context
{
public:
    r_recording_context(const r_disco::r_camera& camera);
    r_recording_context(const r_recording_context&) = delete;
    r_recording_context(r_recording_context&&) noexcept;
    ~r_recording_context() noexcept;

    r_recording_context& operator=(const r_recording_context&) = delete;
    r_recording_context& operator=(r_recording_context&&) noexcept;

    bool dead() const;

    r_disco::r_camera camera() const;

    int32_t bytes_per_second() const;

private:
    r_disco::r_camera _camera;
    r_pipeline::r_gst_source _source;
    r_storage::r_storage_file _storage_file;
    r_storage::r_storage_write_context _storage_write_context;
    std::mutex _sample_write_lock;
    std::chrono::system_clock::time_point _last_v_time;
    std::chrono::system_clock::time_point _last_a_time;
    bool _has_audio;
    std::chrono::system_clock::time_point _stream_start_ts;
    uint64_t _v_bytes_received;
    uint64_t _a_bytes_received;
};

}

#endif
