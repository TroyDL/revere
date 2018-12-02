
#include "test_r_av_r_video_encoder.h"
#include "r_av/r_locky.h"
#include "r_av/r_options.h"
#include "r_av/r_video_encoder.h"
#include "r_av/r_video_decoder.h"
#include "r_av/r_muxer.h"
#include "r_utils/r_file.h"

#include "this_world_is_a_lie.cpp"
#include "pic.cpp"

using namespace std;
using namespace r_utils;
using namespace r_av;

REGISTER_TEST_FIXTURE(test_r_av_r_video_encoder);

void test_r_av_r_video_encoder::setup()
{
    r_locky::register_ffmpeg();

    _pic = r_packet(pic_0, pic_0_len);

    r_utils::r_fs::write_file(this_world_is_a_lie_mkv, this_world_is_a_lie_mkv_len, "this_world_is_a_lie.mkv");
}

void test_r_av_r_video_encoder::teardown()
{
    remove("this_world_is_a_lie.mkv");

    //remove("trans.mp4");

    r_locky::unregister_ffmpeg();
}

void test_r_av_r_video_encoder::test_constructor()
{
    RTF_ASSERT_NO_THROW( r_video_encoder e(r_av_codec_id_h264, r_av_pix_fmt_yuv420p, 1280, 720, 15, 300000, 1, 90000, get_encoder_options("baseline", "ultrafast", "", 2)));
}

void test_r_av_r_video_encoder::test_encode_key()
{
    r_video_encoder e(r_av_codec_id_h264, r_av_pix_fmt_yuv420p, 1280, 720, 15, 500000, 1, 15, get_encoder_options("baseline", "ultrafast", "", 2));

    auto es = e.encode_image(_pic, r_video_encoder::r_encoder_packet_type_i);
    RTF_ASSERT(es == r_video_encoder::r_encoder_state_hungry);
    es = e.encode_image(r_packet(0));
    RTF_ASSERT(es == r_video_encoder::r_encoder_state_accepted);
    auto pkt = e.get();
    RTF_ASSERT(pkt.get_data_size() > 5000);
}

void test_r_av_r_video_encoder::test_encode_gop()
{
    {
        for(int i = 0; i < 5; ++i)
        {
            r_demuxer deMuxer("this_world_is_a_lie.mkv");
            int videoStreamIndex = deMuxer.get_video_stream_index();

            r_video_decoder decoder(deMuxer.get_codec_parameters(videoStreamIndex), r_av_codec_id_h264, r_av_pix_fmt_yuv420p, get_decoder_options(1));

            decoder.set_output_width( 640 );
            decoder.set_output_height( 360 );

            deMuxer.set_output_time_base(videoStreamIndex, std::make_pair(1, 90000));

            r_video_encoder e(r_av_codec_id_h264, r_av_pix_fmt_yuv420p, 640, 360, 15, 1000000, 1, 90000, get_encoder_options("main", "medium", "", 2));

            r_muxer m(r_muxer::OUTPUT_LOCATION_FILE, r_string_utils::format("%d.mp4",i) );

            r_stream_options soptions;
            soptions.type = "video";
            soptions.codec_id = r_av_codec_id_h264;
            soptions.format = r_av_pix_fmt_yuv420p;
            soptions.width = 640;
            soptions.height = 360;
            soptions.time_base_num = 1;
            soptions.time_base_den = 90000;

            auto outputStreamIndex = m.add_stream(soptions);

            m.set_extradata(e.get_extradata(), outputStreamIndex);

            printf("%d\n",i);
            int streamIndex = 0;
            while(deMuxer.read_frame(streamIndex))
            {
                if(streamIndex == videoStreamIndex)
                {
                    auto pkt = deMuxer.get();

                    auto ds = decoder.decode(pkt);

                    if(ds == r_video_decoder::r_decoder_state_accepted)
                    {
                        r_packet pic;
                        RTF_ASSERT_NO_THROW( pic = decoder.get() );

                        auto es = e.encode_image(pic);
                        if(es == r_video_encoder::r_encoder_state_accepted)
                        {
                            auto outputPkt = e.get();

                            m.write_packet(outputPkt, outputStreamIndex, outputPkt.is_key());
                        }
                    }
                }
            }
            printf("\n");

            //auto buf = m.finalize_buffer();
            m.finalize_file();
            //r_fs::write_file(&buf[0], buf.size(), r_string_utils::format("%d.mp4",i));
        }
    }

//    {
//        r_demuxer deMuxer("trans.mp4");
//        auto ed = deMuxer.get_extradata(deMuxer.get_video_stream_index());
//        r_fs::write_file(&ed[0],ed.size(),"ed");
//    }

}
