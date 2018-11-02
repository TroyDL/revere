#include "r_av/r_options.h"
#include "r_utils/r_string.h"

using namespace r_av;
using namespace r_utils;
using namespace std;

struct r_codec_options r_av::get_decoder_options(int thread_count)
{
    struct r_codec_options options;
    options.thread_count = thread_count;
    return options;
}

struct r_codec_options r_av::get_encoder_options(const std::string& profile, const std::string& preset, const std::string& tune, int thread_count)
{
    struct r_codec_options options;
    options.profile = profile;
    options.preset = preset;
    options.tune = tune;
    options.thread_count = thread_count;
    return options;
}
