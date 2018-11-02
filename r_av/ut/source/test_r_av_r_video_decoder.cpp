
#include "test_r_av_r_video_decoder.h"
#include "r_av/r_options.h"
#include "r_av/r_muxer.h"
#include "r_av/r_demuxer.h"
#include "r_av/r_locky.h"
#include "r_av/r_video_decoder.h"

#include "gop.cpp"

using namespace std;
using namespace r_utils;
using namespace r_av;

REGISTER_TEST_FIXTURE(test_r_av_r_video_decoder);

void test_r_av_r_video_decoder::setup()
{
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
    r_packet pkt( gop[0].frame, gop[0].frameSize, false );
    decoder.decode( pkt );
    decoder.decode( r_packet(0) );

    RTF_ASSERT( decoder.get_input_width() == 1280 );
    RTF_ASSERT( decoder.get_input_height() == 720 );
}

void test_r_av_r_video_decoder::test_output_dimensions()
{
    r_video_decoder decoder(r_av_codec_id_h264, r_av_pix_fmt_yuv420p, get_decoder_options(2));
    r_packet pkt( gop[0].frame, gop[0].frameSize, false );
    decoder.decode( pkt );
    auto ds = decoder.decode( r_packet(0) ); // null flush packet
    RTF_ASSERT(ds == r_video_decoder::r_decoder_state_accepted);

    decoder.set_output_width( 640 );
    decoder.set_output_height( 360 );

    r_packet pic;
    RTF_ASSERT_NO_THROW( pic = decoder.get() );
    RTF_ASSERT( pic.get_data_size() == 345600 );
}

void test_r_av_r_video_decoder::test_decode_gop()
{
    r_video_decoder decoder(r_av_codec_id_h264, r_av_pix_fmt_yuv420p, get_decoder_options(2));

    r_video_decoder::r_decoder_state ds = r_video_decoder::r_decoder_state_hungry;

    int i = 0;
    while(ds != r_video_decoder::r_decoder_state_eof)
    {
        if(i < NUM_FRAMES_IN_GOP)
        {
            r_packet pkt( gop[i].frame, gop[i].frameSize, false );
            RTF_ASSERT_NO_THROW( ds = decoder.decode( pkt ) );
        }
        else RTF_ASSERT_NO_THROW( ds = decoder.decode(r_packet(0)) );

        if(ds == r_video_decoder::r_decoder_state_eof)
            continue;

        if(ds == r_video_decoder::r_decoder_state_accepted)
        {
            r_packet pic;
            RTF_ASSERT_NO_THROW( pic = decoder.get() );
            RTF_ASSERT( pic.get_data_size() == 1382400 );
        }

        ++i;
    }
}
