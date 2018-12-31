
#include "video.h"
#include "r_vss_client/r_query_generator.h"
#include "r_vss_client/r_media_parser.h"
#include "r_vss_client/r_query.h"
#include "r_vss_client/r_sdp.h"
#include "r_av/r_muxer.h"
#include "r_av/r_options.h"
#include "r_av/r_packet.h"
#include "r_av/r_utils.h"
#include "r_av/r_video_decoder.h"
#include "r_rtsp/r_sdp.h"
#include "r_utils/r_file.h"
#include <string>
#include <chrono>

using namespace r_utils;
using namespace r_http;
using namespace r_av;
using namespace r_rtsp;
using namespace r_vss_client;
using namespace std;
using namespace std::chrono;

r_server_response make_stream_segment(const r_web_server<r_socket>& ws,
                                      r_buffered_socket<r_socket>& conn,
                                      const r_server_request& request)
{
    auto args = request.get_uri().get_get_args();

    r_muxer::OUTPUT_LOCATION location = r_muxer::OUTPUT_LOCATION_BUFFER;

    string fileName = "a.mp4";
    auto startTime = r_time_utils::iso_8601_to_tp(args["start_time"]);
    auto endTime = startTime + seconds(10);

    auto now = std::chrono::system_clock::now();

    if(endTime > (now - seconds(10)))
    {
        auto secs = duration_cast<seconds>(endTime - (now - seconds(10))).count();
        printf("sleeping %lu seconds\n",secs);
        sleep(secs);
    }

    auto sdp = r_vss_client::fetch_sdp_before(args["data_source_id"],
                                              "video",
                                              startTime);

    auto sdesc = r_rtsp::parse_sdp(sdp);

    int sdpVideoIndex = -1;
    for(int i = 0; i < (int)sdesc.media_descriptions.size(); ++i)
        if(sdesc.media_descriptions[i].type == "video")
            sdpVideoIndex = i;
    if(sdpVideoIndex == -1)
        R_THROW(("Unable to find video in SDP."));

    auto ed = r_rtsp::create_annexb_extradata(sdesc.media_descriptions[sdpVideoIndex].encoded_sps,
                                              sdesc.media_descriptions[sdpVideoIndex].encoded_pps);

    r_muxer muxer(location, fileName);

    bool streamCreated = false;

    auto q = r_query_generator::gen(args["data_source_id"],
                                    "video",
                                    startTime,
                                    endTime,
                                    true);
    int64_t ts = 0;
    int64_t increment = 0;
    int videoStreamIndex = -1;

    r_vss_client::r_media_stats stats;

    //printf("q=%s\n",q.value().c_str());
    auto fb = r_vss_client::query(q);

    r_vss_client::r_media_parser parser(fb);

    stats = parser.get_stats();

    int indexOfLastKey = -1;
    int numFrames = 0;

    parser.begin();
    for(int i = 0; parser.valid(); ++i, parser.next())
    {
        system_clock::time_point tp;
        size_t frameSize;
        const uint8_t* p = NULL;
        uint32_t flags;
        parser.current_data(tp, frameSize, p, flags);
        if(flags > 0)
            indexOfLastKey = i;
        ++numFrames;
    }

    if(indexOfLastKey == -1)
        R_THROW(("No Key Frame found!"));

    system_clock::time_point tp;
    size_t frameSize;
    const uint8_t* p = NULL;
    uint32_t flags;

    parser.begin();
    for(int i = 0; i < indexOfLastKey; ++i, parser.next())
    {
        parser.current_data(tp, frameSize, p, flags);

        r_packet pkt(const_cast<uint8_t*>(p), frameSize, false);

        pkt.set_time_base(make_pair(1, sdesc.media_descriptions[sdpVideoIndex].time_base));

        if(!streamCreated)
        {
            streamCreated = true;

            auto codecID = sdp_codec_to_r_av_codec_id(sdesc.media_descriptions[sdpVideoIndex].codec);

            // decode one frame... this gets us resolution (we could also parse sps/pps).
            r_video_decoder dec(codecID, r_av_pix_fmt_yuv420p, get_decoder_options());
            dec.decode(pkt);
            dec.decode(r_packet(0));

            // XXX NOTE: this all needs to be programatically determined.
            r_stream_options soptions;
            soptions.type = "video";
            soptions.codec_id = codecID;
            soptions.format = r_av_pix_fmt_yuv420p;
            soptions.width = dec.get_input_width();
            soptions.height = dec.get_input_height();

            soptions.time_base_num = 1;
            // I can probably just use 90000 here instead of the value from the sdp... But this works too...
            soptions.time_base_den = sdesc.media_descriptions[sdpVideoIndex].time_base;
            increment = (int64_t)((double)sdesc.media_descriptions[sdpVideoIndex].time_base / ((double)stats.frame_rate_num / (double)stats.frame_rate_den));
            videoStreamIndex = muxer.add_stream(soptions);

            muxer.set_extradata(ed, videoStreamIndex);
        }

        pkt.set_pts(ts);
        pkt.set_dts(ts);
        pkt.set_duration(increment);
        ts += increment;
        // XXX (flags>0) isn't good. r_storage defines FLAG_KEY, but r_vss_client should
        // should provide some translational facility.
        muxer.write_packet(pkt, videoStreamIndex, (flags>0)?true:false);
    }

    r_server_response response;

    auto buffer = muxer.finalize_buffer();
    response.set_body(buffer.size(), &buffer[0]);
    response.set_content_type(r_av::filename_to_content_type(fileName));
    
    parser.current_data(tp, frameSize, p, flags);

    response.add_additional_header("next_start_time", r_time_utils::tp_to_iso_8601(tp, false));

    return response;
}
