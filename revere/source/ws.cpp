
#include "ws.h"
#include "r_utils/3rdparty/json/json.h"
#include "r_utils/r_time_utils.h"
#include "r_utils/r_file.h"
#include "r_utils/r_blob_tree.h"
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

const int WEB_SERVER_PORT = 10080;

ws::ws(const string& top_dir, r_devices& devices) :
    _top_dir(top_dir),
    _devices(devices),
    _server(WEB_SERVER_PORT)
{
    _server.add_route(METHOD_GET, "/jpg", std::bind(&ws::_get_jpg, this, _1, _2, _3));

    _server.start();

// ws.add_route(METHOD_GET, "/data_sources", std::bind(&data_sources::handle_get, &ds, _1, _2, _3));
//
// then handle like this:
//
// #include "r_utils/3rdparty/json/json.h"
// using json = nlohmann::json;
//
// r_server_response data_sources::handle_get(const r_web_server<r_socket>& ws,
//                                            r_buffered_socket<r_socket>& conn,
//                                            const r_server_request& request)
// {
//     r_sqlite_conn dbconn(_dataSourcesPath);
//
//     json j;
//
//     j["data"]["data_sources"] = json::array();
//
//     auto results = dbconn.exec("SELECT * FROM data_sources;");
//
//     for(auto r : results)
//     {
//         j["data"]["data_sources"] += {{"id", r["id"]},
//                                       {"type", r["type"]},
//                                       {"rtsp_url", r["rtsp_url"]},
//                                       {"recording", r["recording"]},
//                                       {"transport_pref", r["transport_pref"]},
//                                       {"auth_username", r["auth_username"]},
//                                       {"auth_password", r["auth_password"]}};
//     }
//
//     r_server_response response;
//
//     response.set_body(j.dump());
//
//     return response;
// }

}

ws::~ws()
{
    _server.stop();
}

r_http::r_server_response ws::_get_jpg(const r_http::r_web_server<r_utils::r_socket>& ws,
                                       r_utils::r_buffered_socket<r_utils::r_socket>& conn,
                                       const r_http::r_server_request& request)
{
    auto args = request.get_uri().get_get_args();

    auto maybe_camera = _devices.get_camera_by_id(args["camera_id"]);

    if(maybe_camera.is_null())
        R_STHROW(r_http_404_exception, ("Unknown camera id."));

    if(maybe_camera.value().record_file_path.is_null())
        R_STHROW(r_http_404_exception, ("Camera has no recording file!"));

    r_storage_file sf(_top_dir + r_fs::PATH_SLASH + "video" + r_fs::PATH_SLASH + maybe_camera.value().record_file_path.value());

    auto key_bt = sf.query_key(R_STORAGE_MEDIA_TYPE_VIDEO, duration_cast<std::chrono::milliseconds>(r_time_utils::iso_8601_to_tp(args["start_time"]).time_since_epoch()).count());

    uint32_t version = 0;
    auto bt = r_blob_tree::deserialize(&key_bt[0], key_bt.size(), version);

    auto video_codec_name = bt["video_codec_name"].get_string();
    auto video_codec_parameters = bt["video_codec_parameters"].get_string();

    if(bt["frames"].size() != 1)
        R_STHROW(r_http_500_exception, ("Expected exactly one frame in blob tree."));

    auto key = (bt["frames"][0]["key"].get_string() == "true");
    auto frame = bt["frames"][0]["data"].get();
    auto pts = r_string_utils::s_to_int64(bt["frames"][0]["pts"].get_string());

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

    R_STHROW(r_http_500_exception, ("Failed to decode video frame."));
}
