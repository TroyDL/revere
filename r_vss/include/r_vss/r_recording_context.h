
#ifndef __r_vss_r_recording_context_h
#define __r_vss_r_recording_context_h

#include "r_disco/r_camera.h"
#include "r_pipeline/r_gst_source.h"

namespace r_vss
{

class r_recording_context
{
public:
    r_recording_context(const r_disco::r_camera& camera);
    ~r_recording_context() noexcept;

    r_disco::r_camera camera() const;

private:
    r_disco::r_camera _camera;
    r_pipeline::r_gst_source _source;
};

}

#endif
