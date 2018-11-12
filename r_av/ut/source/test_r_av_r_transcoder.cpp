
#include "test_r_av_r_transcoder.h"
#include "r_av/r_locky.h"
#include "r_av/r_options.h"
#include "r_av/r_video_encoder.h"
#include "r_av/r_video_decoder.h"
#include "r_av/r_transcoder.h"

#include "gop.cpp"
#include "pic.cpp"

using namespace std;
using namespace r_utils;
using namespace r_av;

REGISTER_TEST_FIXTURE(test_r_av_r_transcoder);

void test_r_av_r_transcoder::setup()
{
    r_locky::register_ffmpeg();
}

void test_r_av_r_transcoder::teardown()
{
    r_locky::unregister_ffmpeg();
}

void test_r_av_r_transcoder::test_constructor()
{
}

void test_r_av_r_transcoder::test_transcode_basic()
{
}
