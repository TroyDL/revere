
#include "test_r_av_r_video_decoder.h"
#include "r_av/r_options.h"
#include "r_av/r_muxer.h"
#include "r_av/r_demuxer.h"
#include "r_av/r_locky.h"
#include "r_av/r_video_decoder.h"

#include "got_frames.cpp"

using namespace std;
using namespace r_utils;
using namespace r_av;

REGISTER_TEST_FIXTURE(test_r_av_r_video_decoder);

void test_r_av_r_video_decoder::setup()
{
    // kill some unused variable warnings
    got_frames_keyness[0] = got_frames_keyness[0];
    got_ed[0] = got_ed[0];
    got_ed_size = got_ed_size;

    r_locky::register_ffmpeg();
}

void test_r_av_r_video_decoder::teardown()
{
    r_locky::unregister_ffmpeg();
}

void test_r_av_r_video_decoder::test_constructor()
{
    RTF_ASSERT_NO_THROW( r_video_decoder d(r_av_codec_id_h264, r_av_pix_fmt_yuv420p, get_decoder_options(2)));
}

void test_r_av_r_video_decoder::test_input_dimensions()
{
    r_video_decoder decoder(r_av_codec_id_h264, r_av_pix_fmt_yuv420p, get_decoder_options(2));
    r_packet pkt(got_frames[0], got_frames_sizes[0], false );
    decoder.decode( pkt );
    decoder.decode( r_packet(0) );

    RTF_ASSERT( decoder.get_input_width() == 1280 );
    RTF_ASSERT( decoder.get_input_height() == 720 );
}

void test_r_av_r_video_decoder::test_output_dimensions()
{
    r_video_decoder decoder(r_av_codec_id_h264, r_av_pix_fmt_yuv420p, get_decoder_options(2));
    r_packet pkt(got_frames[0], got_frames_sizes[0], false );
    decoder.decode( pkt );
    decoder.decode( r_packet(0) );

    decoder.set_output_width( 640 );
    decoder.set_output_height( 360 );

    r_packet pic;
    RTF_ASSERT_NO_THROW( pic = decoder.get() );
    RTF_ASSERT( pic.get_data_size() == 345600 );
}
