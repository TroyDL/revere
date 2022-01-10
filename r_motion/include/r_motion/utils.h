
#ifndef __r_motion_utils_h
#define __r_motion_utils_h

#include <vector>
#include <deque>
#include <string>
#include <cstdint>

namespace r_motion
{

enum r_motion_image_type
{
    R_MOTION_IMAGE_TYPE_ARGB,
    R_MOTION_IMAGE_TYPE_GRAY8,
    R_MOTION_IMAGE_TYPE_GRAY16
};

struct r_image
{
    r_motion_image_type type;
    uint16_t width;
    uint16_t height;
    std::vector<uint8_t> data;
};

r_image argb_to_gray8(const r_image& argb);
r_image argb_to_gray16(const r_image& argb);
r_image gray8_to_argb(const r_image& gray);
r_image gray16_to_argb(const r_image& gray);

r_image gray8_subtract(const r_image& a, const r_image& b);
r_image gray8_remove(const r_image& a, const r_image& b);

r_image average_images(const std::deque<r_image>& images);

uint64_t gray8_compute_motion(const r_image& a);

void ppm_write_argb(const std::string& filename, const r_image& image);

}

#endif
