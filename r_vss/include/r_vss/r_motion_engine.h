
#ifndef __r_vss_r_motion_engine_h
#define __r_vss_r_motion_engine_h

#include "r_motion/r_motion_state.h"
#include "r_utils/r_blocking_q.h"
#include "r_codec/r_video_decoder.h"
#include "r_pipeline/r_gst_buffer.h"
#include "r_disco/r_devices.h"
#include <vector>
#include <map>
#include <memory>
#include <thread>

namespace r_vss
{

struct r_work_item
{
    r_pipeline::r_gst_buffer frame;
    std::string video_codec_name;
    std::string video_codec_parameters;
    std::string id;
    int64_t ts;
};

struct r_work_context
{
public:
    r_work_context(AVCodecID codec_id) :
        motion_state(),
        video_decoder(codec_id)
    {
    }
    ~r_work_context() noexcept
    {
    }
    r_motion::r_motion_state motion_state;
    r_codec::r_video_decoder video_decoder;
    r_disco::r_camera camera;
};

class r_motion_engine final
{
public:
    r_motion_engine() = delete;
    r_motion_engine(r_disco::r_devices& devices, const std::string& top_dir);
    r_motion_engine(const r_motion_engine&) = delete;
    r_motion_engine(r_motion_engine&&) = delete;
    ~r_motion_engine() noexcept;

    r_motion_engine& operator=(const r_motion_engine&) = delete;
    r_motion_engine& operator=(r_motion_engine&&) = delete;

    void start();
    void stop() noexcept;

    void post_frame(r_pipeline::r_gst_buffer buffer, int64_t ts, const std::string& video_codec_name, const std::string& video_codec_parameters, const std::string& id);

private:
    void _entry_point();
    std::map<std::string, std::shared_ptr<r_work_context>>::iterator _create_work_context(const r_work_item& item);
    r_disco::r_devices& _devices;
    std::string _top_dir;
    r_utils::r_blocking_q<r_work_item> _work;
    std::map<std::string, std::shared_ptr<r_work_context>> _work_contexts;
    bool _running;
    std::thread _thread;
};

}

#endif
