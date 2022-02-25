
#include "ws.h"
#include "utils.h"
#include "r_utils/3rdparty/json/json.h"
#include "r_utils/r_time_utils.h"
#include "r_utils/r_file.h"
#include "r_utils/r_blob_tree.h"
#include "r_utils/3rdparty/json/json.h"
#include "r_disco/r_camera.h"
#include "r_storage/r_storage_file.h"
#include "r_pipeline/r_stream_info.h"
#include "r_mux/r_muxer.h"
#include "r_codec/r_video_decoder.h"
#include "r_codec/r_video_encoder.h"
#include <functional>

using namespace r_utils;
using namespace r_http;
using namespace r_disco;
using namespace r_storage;
using namespace r_mux;
using namespace r_codec;
using namespace std;
using namespace std::chrono;
using namespace std::placeholders;
using json = nlohmann::json;

const int WEB_SERVER_PORT = 10080;

ws::ws(const string& top_dir, r_devices& devices) :
    _top_dir(top_dir),
    _devices(devices),
    _server(WEB_SERVER_PORT)
{
    _server.add_route(METHOD_GET, "/jpg", std::bind(&ws::_get_jpg, this, _1, _2, _3));
    _server.add_route(METHOD_GET, "/contents", std::bind(&ws::_get_contents, this, _1, _2, _3));
    _server.add_route(METHOD_GET, "/cameras", std::bind(&ws::_get_cameras, this, _1, _2, _3));
    _server.add_route(METHOD_GET, "/export", std::bind(&ws::_get_export, this, _1, _2, _3));

    _server.start();
}

ws::~ws()
{
    _server.stop();
}

r_http::r_server_response ws::_get_jpg(const r_http::r_web_server<r_utils::r_socket>& ws,
                                       r_utils::r_buffered_socket<r_utils::r_socket>& conn,
                                       const r_http::r_server_request& request)
{
    try
    {        
        auto args = request.get_uri().get_get_args();

        if(args.find("camera_id") == args.end())
            R_THROW(("Missing camera_id."));

        auto maybe_camera = _devices.get_camera_by_id(args["camera_id"]);

        if(maybe_camera.is_null())
            R_THROW(("Unknown camera id."));

        if(maybe_camera.value().record_file_path.is_null())
            R_THROW(("Camera has no recording file!"));

        r_storage_file sf(_top_dir + r_fs::PATH_SLASH + "video" + r_fs::PATH_SLASH + maybe_camera.value().record_file_path.value());

        if(args.find("start_time") == args.end())
            R_THROW(("Missing start_time."));

        auto key_bt = sf.query_key(R_STORAGE_MEDIA_TYPE_VIDEO, duration_cast<std::chrono::milliseconds>(r_time_utils::iso_8601_to_tp(args["start_time"]).time_since_epoch()).count());

        uint32_t version = 0;
        auto bt = r_blob_tree::deserialize(&key_bt[0], key_bt.size(), version);

        auto video_codec_name = bt["video_codec_name"].get_string();
        auto video_codec_parameters = bt["video_codec_parameters"].get_string();

        if(bt["frames"].size() != 1)
            R_THROW(("Expected exactly one frame in blob tree."));

        if(!bt["frames"][0].has_key("key"))
            R_THROW(("Expected frame to have key."));

        auto key = (bt["frames"][0]["key"].get_string() == "true");

        if(!bt["frames"][0].has_key("data"))
            R_THROW(("Expected frame to have data."));

        auto frame = bt["frames"][0]["data"].get();

        r_video_decoder decoder(r_mux::encoding_to_av_codec_id(video_codec_name));
        decoder.set_extradata(r_pipeline::get_video_codec_extradata(video_codec_name, video_codec_parameters));
        decoder.attach_buffer(frame.data(), frame.size());
        auto ds = decoder.decode();
        if(ds == R_CODEC_STATE_HAS_OUTPUT)
        {
            auto decoded = decoder.get(AV_PIX_FMT_YUV420P, 640, 480);

            r_video_encoder encoder(AV_CODEC_ID_MJPEG, 100000, 640, 480, {1,1}, AV_PIX_FMT_YUV420P, 0, 1, 0, 0);
            encoder.attach_buffer(decoded.data(), decoded.size());
            auto es = encoder.encode();
            if(es == R_CODEC_STATE_HAS_OUTPUT)
            {
                auto pi = encoder.get();

                r_server_response response;
                response.set_content_type("image/jpeg");
                response.set_body(pi.size, pi.data);
                return response;
            }
        }
    }
    catch(const std::exception& ex)
    {
        R_LOG_ERROR("%s", ex.what());
    }

    R_STHROW(r_http_500_exception, ("Failed to create jpg."));
}

r_http::r_server_response ws::_get_contents(const r_http::r_web_server<r_utils::r_socket>& ws,
                                            r_utils::r_buffered_socket<r_utils::r_socket>& conn,
                                            const r_http::r_server_request& request)
{
    try
    {
        auto args = request.get_uri().get_get_args();

        auto maybe_camera = _devices.get_camera_by_id(args["camera_id"]);

        if(maybe_camera.is_null())
            R_THROW(("Unknown camera id."));

        if(maybe_camera.value().record_file_path.is_null())
            R_THROW(("Camera has no recording file!"));

        r_storage_file sf(_top_dir + r_fs::PATH_SLASH + "video" + r_fs::PATH_SLASH + maybe_camera.value().record_file_path.value());

        if(args.find("media_type") == args.end())
            R_THROW(("Missing media_type."));

        r_storage_media_type mt = R_STORAGE_MEDIA_TYPE_ALL;
        if(args["media_type"] == "audio")
            mt = R_STORAGE_MEDIA_TYPE_AUDIO;
        else if(args["media_type"] == "video")
            mt = R_STORAGE_MEDIA_TYPE_VIDEO;

        if(args.find("start_time") == args.end())
            R_THROW(("Missing start_time."));

        auto start_time_s = args["start_time"];

        bool input_z_time = start_time_s.find("Z") != std::string::npos;

        if(args.find("end_time") == args.end())
            R_THROW(("Missing end_time."));
        
        auto end_time_s = args["end_time"];

        auto key_starts = sf.key_frame_start_times(
            mt,
            duration_cast<std::chrono::milliseconds>(r_time_utils::iso_8601_to_tp(start_time_s).time_since_epoch()).count(),
            duration_cast<std::chrono::milliseconds>(r_time_utils::iso_8601_to_tp(end_time_s).time_since_epoch()).count()
        );

        auto segments = find_contiguous_segments(key_starts);

        json j;
        j["segments"] = json::array();

        for(auto& s : segments)
        {
            // note: instead of push_back here its also possible to use += operator to append json object to array
            j["segments"].push_back({{"start_time", r_time_utils::tp_to_iso_8601(r_time_utils::epoch_millis_to_tp(s.first), input_z_time)},
                                    {"end_time", r_time_utils::tp_to_iso_8601(r_time_utils::epoch_millis_to_tp(s.second), input_z_time)}});
        }

        r_server_response response;
        response.set_content_type("text/json");
        response.set_body(j.dump());
        return response;
    }
    catch(const std::exception& ex)
    {
        R_LOG_ERROR("%s", ex.what());
    }

    R_STHROW(r_http_500_exception, ("Failed to get contents."));
}

r_http::r_server_response ws::_get_cameras(const r_http::r_web_server<r_utils::r_socket>& ws,
                                           r_utils::r_buffered_socket<r_utils::r_socket>& conn,
                                           const r_http::r_server_request& request)
{
    try
    {    
        auto cameras = _devices.get_all_cameras();

        json j;
        j["cameras"] = json::array();

        for(auto c : cameras)
        {
            j["cameras"].push_back(
                {
                    {"id", c.id},
                    {"camera_name", (c.camera_name.is_null())?"":c.camera_name.value()},
                    {"friendly_name", (c.friendly_name.is_null())?"":c.friendly_name.value()},
                    {"ipv4", (c.ipv4.is_null())?"":c.ipv4.value()},
                    {"rtsp_url", (c.rtsp_url.is_null())?"":c.rtsp_url.value()},
                    {"video_codec", (c.video_codec.is_null())?"":c.video_codec.value()},
                    {"audio_codec", (c.audio_codec.is_null())?"":c.audio_codec.value()},
                    {"state", c.state}
                }
            );
        }

        r_server_response response;
        response.set_content_type("text/json");
        response.set_body(j.dump());
        return response;
    }
    catch(const std::exception& ex)
    {
        R_LOG_ERROR("%s", ex.what());
    }

    R_STHROW(r_http_500_exception, ("Failed to get cameras."));
}

static float _compute_framerate(const r_blob_tree& bt)
{
    if(!bt.has_key("frames"))
        R_THROW(("Blob tree missing frames array."));

    auto frame_count = bt.at("frames").size();

    int64_t last_video_ts = 0;
    bool has_last_video_ts = false;;

    vector<int64_t> deltas;

    for(size_t fi = 0; fi < frame_count; ++fi)
    {
        if(!bt.at("frames").has_index(fi))
            R_THROW(("Blob tree missing frame."));

        if(!bt.at("frames").at(fi).has_key("stream_id"))
            R_THROW(("Blob tree missing stream_id."));

        auto stream_id = bt.at("frames").at(fi).at("stream_id").get_value<int>();
        if(stream_id == r_storage::R_STORAGE_MEDIA_TYPE_VIDEO)
        {
            if(!bt.at("frames").at(fi).has_key("ts"))
                R_THROW(("Blob tree missing ts."));

            auto ts = bt.at("frames").at(fi).at("ts").get_value<int64_t>();

            if(has_last_video_ts && (ts > last_video_ts))
                deltas.push_back(ts - last_video_ts);

            last_video_ts = ts;
            has_last_video_ts = true;
        }
    }

    int64_t avg_delta = (std::accumulate(begin(deltas), end(deltas), 0, [](int64_t a, int64_t b) { return a + b; }) / deltas.size());

    return (float)1000 / (float)avg_delta;
}

static void _check_timestamps(const r_blob_tree& bt)
{
    if(!bt.has_key("frames"))
        R_THROW(("Blob tree missing frames array."));

    auto frame_count = bt.at("frames").size();

    int64_t last_video_ts = 0;
    bool has_last_video_ts = false;;

    for(size_t fi = 0; fi < frame_count; ++fi)
    {
        if(!bt.at("frames").has_index(fi))
            R_THROW(("Blob tree missing frame."));

        if(!bt.at("frames").at(fi).has_key("stream_id"))
            R_THROW(("Blob tree missing stream_id."));

        auto stream_id = bt.at("frames").at(fi).at("stream_id").get_value<int>();
        if(stream_id == r_storage::R_STORAGE_MEDIA_TYPE_VIDEO)
        {
            if(!bt.at("frames").at(fi).has_key("ts"))
                R_THROW(("Blob tree missing ts."));

            auto ts = bt.at("frames").at(fi).at("ts").get_value<int64_t>();

            if(has_last_video_ts)
            {
                if(ts < last_video_ts)
                    R_THROW(("Timestamp is not monotonically increasing."));
            }

            last_video_ts = ts;
            has_last_video_ts = true;
        }
    }
}

r_http::r_server_response ws::_get_export(const r_http::r_web_server<r_utils::r_socket>& ws,
                                          r_utils::r_buffered_socket<r_utils::r_socket>& conn,
                                          const r_http::r_server_request& request)
{
    try
    {    
        auto exports_path = _top_dir + r_fs::PATH_SLASH + "exports";

        if(!r_fs::file_exists(exports_path))
            r_fs::mkdir(exports_path);

        auto args = request.get_uri().get_get_args();

        if(args.find("camera_id") == args.end())
            R_THROW(("Missing camera_id."));

        auto maybe_camera = _devices.get_camera_by_id(args["camera_id"]);

        if(maybe_camera.is_null())
            R_THROW(("Unknown camera id."));

        if(maybe_camera.value().record_file_path.is_null())
            R_THROW(("Camera has no recording file!"));

        r_storage_file sf(_top_dir + r_fs::PATH_SLASH + "video" + r_fs::PATH_SLASH + maybe_camera.value().record_file_path.value());

        if(args.find("start_time") == args.end())
            R_THROW(("Missing start_time."));

        auto start_time_s = args["start_time"];

        if(args.find("end_time") == args.end())
            R_THROW(("Missing end_time."));
        
        auto end_time_s = args["end_time"];

        if(args.find("file_name") == args.end())
            R_THROW(("Missing file name."));

        r_muxer muxer(exports_path + r_fs::PATH_SLASH + args["file_name"]);

        auto qs = r_time_utils::iso_8601_to_tp(start_time_s);

        auto qe = r_time_utils::iso_8601_to_tp(end_time_s);

        bool muxer_opened = false;

        int64_t ts_first_frame = 0;

        bool done = false;

        while(!done)
        {
            auto rs = qs;
            auto re = rs;
            if(rs +  std::chrono::minutes(5) > qe)
            {
                re = qe;
                done = true;
            }
            else re = rs + std::chrono::minutes(5);

            auto qr_buffer = sf.query(
                R_STORAGE_MEDIA_TYPE_ALL,
                duration_cast<std::chrono::milliseconds>(rs.time_since_epoch()).count(),
                duration_cast<std::chrono::milliseconds>(re.time_since_epoch()).count()
            );

            qs = re;

            uint32_t version = 0;
            auto bt = r_blob_tree::deserialize(qr_buffer.data(), qr_buffer.size(), version);

            _check_timestamps(bt);

            if(!muxer_opened)
            {
                muxer_opened = true;

                // first extract metadata from the blob tree

                if(!bt.has_key("has_audio"))
                    R_THROW(("Blob tree missing audio indicator."));

                bool has_audio = (bt["has_audio"].get_string() == "true")?true:false;

                if(!bt.has_key("video_codec_name"))
                    R_THROW(("Blob tree missing video codec name."));

                auto video_codec_name = bt["video_codec_name"].get_string();

                if(!bt.has_key("video_codec_parameters"))
                    R_THROW(("Blob tree missing video codec parameters."));

                auto video_codec_parameters = bt["video_codec_parameters"].get_string();

                string audio_codec_name, audio_codec_parameters;
                if(has_audio)
                {
                    if(!bt.has_key("audio_codec_name"))
                        R_THROW(("Blob tree missing audio codec name but has audio!"));
                    audio_codec_name = bt["audio_codec_name"].get_string();

                    if(!bt.has_key("audio_codec_parameters"))
                        R_THROW(("Blob tree missing audio codec parameters but has audio!"));
                    audio_codec_parameters = bt["audio_codec_parameters"].get_string();
                }

                // now, look for the sc_framerate or estimate it...

                r_nullable<float> fr;

                auto parts = r_string_utils::split(video_codec_parameters, ",");
                for(auto part : parts)
                {
                    auto inner_parts = r_string_utils::split(part, "=");
                    if(inner_parts.size() == 2)
                    {
                        if(r_string_utils::strip(inner_parts[0]) == "sc_framerate")
                            fr.set_value(r_string_utils::s_to_float(inner_parts[1]));
                    }
                }

                if(fr.is_null())
                    fr.set_value(_compute_framerate(bt));

                // get the video codec information and add the right kinds of video stream...

                auto video_codec_id = r_mux::encoding_to_av_codec_id(video_codec_name);

                if(video_codec_id == AV_CODEC_ID_H264)
                {
                    auto maybe_sps = r_pipeline::get_h264_sps(video_codec_parameters);
                    if(!maybe_sps.is_null())
                    {
                        auto sps_info = r_pipeline::parse_h264_sps(maybe_sps.value());

                        muxer.add_video_stream(
                            av_d2q(fr, 10000),
                            video_codec_id,
                            sps_info.width,
                            sps_info.height,
                            sps_info.profile_idc,
                            sps_info.level_idc
                        );
                    }
                    auto maybe_pps = r_pipeline::get_h264_pps(video_codec_parameters);
                    muxer.set_video_extradata(r_pipeline::make_h264_extradata(maybe_sps, maybe_pps));
                }
                else if(video_codec_id == AV_CODEC_ID_HEVC)
                {
                    auto maybe_vps = r_pipeline::get_h265_vps(video_codec_parameters);
                    auto maybe_sps = r_pipeline::get_h265_sps(video_codec_parameters);
                    if(!maybe_sps.is_null())
                    {
                        auto sps_info = r_pipeline::parse_h265_sps(maybe_sps.value());

                        muxer.add_video_stream(
                            av_d2q(fr, 10000),
                            video_codec_id,
                            sps_info.width,
                            sps_info.height,
                            sps_info.profile_idc,
                            sps_info.level_idc
                        );

                        //muxer.set_video_bitstream_filter("hevc_mp4toannexb");
                    }
                    auto maybe_pps = r_pipeline::get_h265_pps(video_codec_parameters);
                    muxer.set_video_extradata(r_pipeline::make_h265_extradata(maybe_vps, maybe_sps, maybe_pps));
                }

                // If there is audio, add the audio stream...

                if(has_audio)
                {
                    printf("audio_codec_parameters=%s\n", audio_codec_parameters.c_str());

                    r_nullable<int> audio_rate, audio_channels;
                    auto parts = r_string_utils::split(audio_codec_parameters, ",");
                    for(auto part : parts)
                    {
                        auto inner_parts = r_string_utils::split(part, "=");
                        if(inner_parts.size() == 2)
                        {
                            if(r_string_utils::strip(inner_parts[0]) == "sc_audio_rate")
                                audio_rate.set_value(r_string_utils::s_to_int(inner_parts[1]));
                            if(r_string_utils::strip(inner_parts[0]) == "sc_audio_channels")
                                audio_channels.set_value(r_string_utils::s_to_int(inner_parts[1]));
                        }
                    }

                    auto audio_codec_id = r_mux::encoding_to_av_codec_id(audio_codec_name);

                    if(audio_channels.is_null())
                        audio_channels.set_value(1);

                    if(audio_rate.is_null())
                    {
                        if(audio_codec_id == AV_CODEC_ID_PCM_MULAW)
                            audio_rate.set_value(8000);
                        else if(audio_codec_id == AV_CODEC_ID_PCM_ALAW)
                            audio_rate.set_value(8000);
                    }

                    if(audio_rate.is_null())
                        R_THROW(("Missing audio rate."));

                    muxer.add_audio_stream(
                        audio_codec_id,
                        audio_channels.value(),
                        audio_rate.value()
                    );
                }

                muxer.open();
            }

            if(!bt.has_key("frames"))
                R_THROW(("Blob tree missing frames."));

            auto n_frames = bt["frames"].size();

            printf("N_FRAMES=%ld\n", n_frames);

            for(size_t fi = 0; fi < n_frames; ++fi)
            {
                if(!bt["frames"].has_index(fi))
                    R_THROW(("Blob tree missing frame."));

                if(!bt["frames"][fi].has_key("stream_id"))
                    R_THROW(("Blob tree missing stream id."));

                auto sid = bt["frames"][fi]["stream_id"].get_value<int>();

                if(!bt["frames"][fi].has_key("key"))
                    R_THROW(("Blob tree missing key."));

                auto key = (bt["frames"][fi]["key"].get_string() == "true");

                if(!bt["frames"][fi].has_key("data"))
                    R_THROW(("Blob tree missing data."));

                auto frame = bt["frames"][fi]["data"].get();

                if(!bt["frames"][fi].has_key("ts"))
                    R_THROW(("Blob tree missing ts."));

                auto ts = bt["frames"][fi]["ts"].get_value<int64_t>();

                if(ts_first_frame == 0)
                    ts_first_frame = ts;

                if(sid == R_STORAGE_MEDIA_TYPE_VIDEO)
                    muxer.write_video_frame(frame.data(), frame.size(), ts-ts_first_frame, ts-ts_first_frame, {1, 1000}, key);
                else if(sid == R_STORAGE_MEDIA_TYPE_AUDIO)
                    muxer.write_audio_frame(frame.data(), frame.size(), ts-ts_first_frame, {1, 1000});
            }
        }

        muxer.finalize();

        r_server_response response;
        return response;
    }
    catch(const std::exception& ex)
    {
        R_LOG_ERROR("%s", ex.what());
    }

    R_STHROW(r_http_500_exception, ("Failed to export."));
}
