
#include "test_r_av_r_transcoder.h"
#include "r_av/r_locky.h"
#include "r_av/r_options.h"
#include "r_av/r_video_encoder.h"
#include "r_av/r_video_decoder.h"
#include "r_av/r_transcoder.h"
#include "r_utils/r_file.h"
#include "r_utils/r_string_utils.h"
#include "gop.cpp"
#include "pic.cpp"
#include "this_world_is_a_lie.cpp"

using namespace std;
using namespace r_utils;
using namespace r_av;

REGISTER_TEST_FIXTURE(test_r_av_r_transcoder);

void test_r_av_r_transcoder::setup()
{
    // 1) demuxer
    // 2) av_packet_rescale_ts                              AVPacket
    // 3) decoder
    // 4) frame->pts = frame->best_effort_pts               AVFrame
    // 5) encoder
    // 6) av_packet_rescale_ts                              AVPacket
    // 7) muxer
    //
    // Step 2 converts from input format contexts time_base into the decoder_ctx->time_base
    // Step 6 converts from encoder contexts time_base into output format contests time_base

    r_locky::register_ffmpeg();

    r_utils::r_fs::write_file(this_world_is_a_lie_mkv, this_world_is_a_lie_mkv_len, "this_world_is_a_lie.mkv");

    r_demuxer dm("this_world_is_a_lie.mkv", true); // 23.976216

    int videoStreamIndex = dm.get_video_stream_index();

    r_video_decoder decoder(dm.get_codec_parameters(dm.get_video_stream_index()), r_av_codec_id_h264, r_av_pix_fmt_yuv420p, get_decoder_options(1));
    decoder.set_output_width(1280);
    decoder.set_output_height(720);

    dm.set_output_time_base(videoStreamIndex, std::make_pair(1, 90000));;

    r_video_encoder encoder(r_av_codec_id_h264, r_av_pix_fmt_yuv420p, 1280, 720, 10, 1000000, 1, 90000, get_encoder_options("main", "medium", "", 2));

    r_muxer m(r_muxer::OUTPUT_LOCATION_FILE, "big.mp4");

    r_stream_options soptions;
    soptions.type = "video";
    soptions.codec_id = r_av_codec_id_h264;
    soptions.format = r_av_pix_fmt_yuv420p;
    soptions.width = 1280;
    soptions.height = 720;
    soptions.time_base_num = 1;
    soptions.time_base_den = 90000;

    auto streamIndex = m.add_stream(soptions);

    int64_t ts = 0;

    int si = 0;
    while(dm.read_frame(si))
    {
        if(si == videoStreamIndex)
        {
            auto dmpkt = dm.get();

            auto ds = decoder.decode(dmpkt);

            if(ds == r_video_decoder::r_decoder_state_accepted)
            {
                auto pic = decoder.get();

                auto es = encoder.encode_image(pic);

                if(es == r_video_encoder::r_encoder_state_accepted)
                {
                    auto enc = encoder.get();
                    m.write_packet(enc, streamIndex, enc.is_key());
                }
            }

        }
    }

    m.finalize_file();
}

void test_r_av_r_transcoder::teardown()
{
    r_locky::unregister_ffmpeg();

    remove("big.mp4");
    remove("this_world_is_a_lie.mkv");
}

void test_r_av_r_transcoder::test_constructor()
{
    r_transcoder transcoder(1, 30, 1, 15, 1.0);
}

void test_r_av_r_transcoder::test_transcode_basic()
{
}
