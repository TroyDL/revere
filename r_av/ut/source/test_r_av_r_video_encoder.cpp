
#include "test_r_av_r_video_encoder.h"
#include "r_av/r_locky.h"
#include "r_av/r_options.h"
#include "r_av/r_video_encoder.h"
#include "r_av/r_video_decoder.h"

#include "gop.cpp"
#include "pic.cpp"

using namespace std;
using namespace r_utils;
using namespace r_av;

REGISTER_TEST_FIXTURE(test_r_av_r_video_encoder);

void test_r_av_r_video_encoder::setup()
{
    r_locky::register_ffmpeg();

    _pic = r_packet(pic_0, pic_0_len);
}

void test_r_av_r_video_encoder::teardown()
{
    r_locky::unregister_ffmpeg();
}

void test_r_av_r_video_encoder::test_constructor()
{
    RTF_ASSERT_NO_THROW( r_video_encoder e(r_av_codec_id_h264, r_av_pix_fmt_yuv420p, 1280, 720, 15, 300000, 1, 90000, get_encoder_options("baseline", "ultrafast", "zerolatency", 2)));
}

void test_r_av_r_video_encoder::test_encode_key()
{
    r_video_encoder e(r_av_codec_id_h264, r_av_pix_fmt_yuv420p, 1280, 720, 15, 500000, 1, 15, get_encoder_options("baseline", "ultrafast", "zerolatency", 2));

    auto es = e.encode_image(_pic, r_video_encoder::r_encoder_packet_type_i);
    RTF_ASSERT(es == r_video_encoder::r_encoder_state_hungry);
    es = e.encode_image(r_packet(0));
    RTF_ASSERT(es == r_video_encoder::r_encoder_state_accepted);
    auto pkt = e.get();
    RTF_ASSERT(pkt.get_data_size() > 5000);
}

void test_r_av_r_video_encoder::test_encode_gop()
{
    r_video_decoder decoder(r_av_codec_id_h264, r_av_pix_fmt_yuv420p, get_decoder_options(2));
    decoder.set_output_width( 640 );
    decoder.set_output_height( 360 );

    r_video_encoder e(r_av_codec_id_h264, r_av_pix_fmt_yuv420p, 640, 360, 15, 500000, 1, 15, get_encoder_options("baseline", "ultrafast", "zerolatency", 2));

    list<r_packet> outputFrames;

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
            RTF_ASSERT( pic.get_data_size() == 345600 );
            auto es = e.encode_image(pic);
            if(es == r_video_encoder::r_encoder_state_accepted)
                outputFrames.push_back(e.get());
        }

        ++i;
    }

    r_video_decoder decoder2(r_av_codec_id_h264, r_av_pix_fmt_yuv420p, get_decoder_options(2));
    decoder2.set_extradata(e.get_extradata());
    auto iter = outputFrames.begin();
    ds = r_video_decoder::r_decoder_state_accepted;
    while(ds != r_video_decoder::r_decoder_state_eof)
    {
        if(iter != outputFrames.end())
        {
            ds = decoder2.decode(*iter);
            ++iter;
        }
        else ds = decoder2.decode(r_packet(0));

        if(ds == r_video_decoder::r_decoder_state_accepted)
        {
            auto out = decoder2.get();
            RTF_ASSERT(out.get_data_size() == (640 * 360 * 1.5));
        }
    }
}
