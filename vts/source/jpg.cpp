
#include "jpg.h"
#include "r_utils/r_time_utils.h"
#include "r_vss_client/r_frame.h"
#include "r_av/r_video_decoder.h"
#include "r_av/r_video_encoder.h"

using namespace r_utils;
using namespace r_http;
using namespace r_av;
using namespace std;
using namespace std::chrono;

r_server_response make_jpg(const r_web_server<r_socket>& ws,
                           r_buffered_socket<r_socket>& conn,
                           const r_server_request& request)
{
    auto args = request.get_uri().get_get_args();
    if(args.find("time") == args.end())
	    args["time"] = r_time_utils::tp_to_iso_8601(system_clock::now(), false);
    auto iframe = r_vss_client::fetch_key_before(args["data_source_id"], r_time_utils::iso_8601_to_tp(args["time"]));
    R_LOG_NOTICE("make_jpg() - time=%s, iframe=%lu, data_source_id=%s", args["time"].c_str(), iframe.size(), args["data_source_id"].c_str());
    // XXX NOTE: we shouldn't assume h.264 here.
    r_video_decoder decoder(r_av_codec_id_h264, r_av_pix_fmt_yuvj420p, get_decoder_options(2));
    r_packet pkt(&iframe[0], iframe.size(), false);
    decoder.decode(pkt);
    decoder.decode(r_packet(0)); // null flush packet        
    if(args.find("width") == args.end())
        args["width"] = r_string_utils::uint16_to_s(decoder.get_input_width());
    if(args.find("height") == args.end())
        args["height"] = r_string_utils::uint16_to_s(decoder.get_input_height());
    decoder.set_output_width(r_string_utils::s_to_uint16(args["width"]));
    decoder.set_output_height(r_string_utils::s_to_uint16(args["height"]));
    auto pic = decoder.get();
    r_video_encoder e(r_av_codec_id_mjpeg, r_av_pix_fmt_yuvj420p, r_string_utils::s_to_uint16(args["width"]), r_string_utils::s_to_uint16(args["height"]), 15, 500000, 1, 15, get_encoder_options("baseline", "fast", "", 2));
    e.encode_image(pic);
    e.encode_image(r_packet(0)); // null flush packet
    auto epkt = e.get();
    r_server_response response;
    response.set_body(epkt.get_data_size(), epkt.map());
    response.set_content_type("image/jpeg");
    return response;
}
