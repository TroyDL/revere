#include "data_sources.h"
#include "stream_host.h"
#include "r_utils/r_logger.h"
#include "r_utils/r_functional.h"
#include "r_utils/r_time_utils.h"
#include "r_utils/r_file.h"
#include "r_utils/3rdparty/json/json.h"
#include "r_http/r_web_server.h"
#include "r_http/r_http_exception.h"
#include "r_storage/r_storage_engine.h"
#include "r_storage/r_queries.h"
#include "r_pipe/plugins/rtsp_source/r_rtsp_source.h"
#include "r_pipe/plugins/storage_sink/r_storage_sink.h"

#include <functional>
#include <algorithm>
#include <string>
#include <signal.h>
#include <unistd.h>

using namespace r_utils;
using json = nlohmann::json;
using namespace r_http;
using namespace r_pipe;
using namespace std;
using namespace std::placeholders;

static const int SERVER_PORT = 11002;

bool running = true;

void handle_sigterm(int sig)
{
    if(sig == SIGTERM)
        running = false;
}

unique_ptr<r_control> _create_stream(const data_source& ds)
{
    auto control = make_unique<r_control>();
    control->add_source("source", make_shared<r_rtsp_source>());
    if(r_string_utils::to_lower(ds.transport_pref) != "tcp")
        control->set_param("source", "prefer_tcp", "false");
    control->set_param("source", "rtsp_url", ds.rtsp_url);
    control->set_param("source", "type", "video"); // XXX this needs to be based on the type of data source it is.
    control->commit_params("source");

    control->add_filter("sink", make_shared<r_storage_sink>("/data/vss/index", ds.id, ds.type));;

    control->connect("source", "sink");

    control->run();

    return control;
}
void printv(const string& pfx, const vector<data_source>& v)
{
    for(auto s : v)
        printf("%s %s\n",pfx.c_str(),s.id.c_str());
}
int main(int argc, char* argv[])
{
    // XXX TODO
    // - When populating drives with files, randomly place them in write dirs... so that when we are pulling
    //   them out and using them they come out equally often from each drive.
    // - Need multiple frames API
    // - Discovery
    // - we need camera uuid cross check mechanism (/Device.xml)
    // - Need event log
    // - Need retention API (by media type)
    // - Need connects api (query event log)
    // - drop_counts api
    // - restreaming api

    r_logger::install_terminate();

    signal(SIGTERM, handle_sigterm);

    string firstArgument = (argc > 1) ? argv[1] : string();

    if( firstArgument != "--interactive" )
        daemon( 1, 0 );

    auto cfgBuffer = r_fs::read_file("/data/vss/config.json");
    auto config = string((char*)&cfgBuffer[0], cfgBuffer.size());

    auto cfg = json::parse(config)["storage_config"];

    r_storage::r_storage_engine::configure_storage(config);

    r_web_server<r_socket> ws(SERVER_PORT);

    data_sources::upgrade_db(cfg["data_sources_path"].get<string>());

    data_sources ds(cfg["data_sources_path"].get<string>());
    ws.add_route(METHOD_GET, "/data_sources", std::bind(&data_sources::handle_get, &ds, _1, _2, _3));
    ws.add_route(METHOD_PUT, "/data_sources", std::bind(&data_sources::handle_put, &ds, _1, _2, _3));
    ws.add_route(METHOD_DELETE, "/data_sources", std::bind(&data_sources::handle_del, &ds, _1, _2, _3));

    ws.add_route(METHOD_GET, "/status", [](const r_web_server<r_socket>& ws, r_utils::r_buffered_socket<r_utils::r_socket>& conn, const r_server_request& request)->r_server_response {
        r_server_response response;
        response.set_body("{ \"status\": \"healthy\" }");
        return response;
    });

    ws.add_route(METHOD_GET, "/contents", [&](const r_web_server<r_socket>& ws, r_utils::r_buffered_socket<r_utils::r_socket>& conn, const r_server_request& request)->r_server_response {
        r_server_response response;
        auto args = request.get_uri().get_get_args();
        response.set_body(r_storage::contents(cfg["storage_config"]["index_path"].get<string>(),
                                              args["data_source_id"],
                                              r_time_utils::iso_8601_to_tp(args["start_time"]),
                                              r_time_utils::iso_8601_to_tp(args["end_time"]),
                                              r_string_utils::ends_with(r_string_utils::to_lower(args["start_time"]), "z")));
        return response;
    });

    ws.add_route(METHOD_GET, "/key_before", [&](const r_web_server<r_socket>& ws, r_utils::r_buffered_socket<r_utils::r_socket>& conn, const r_server_request& request)->r_server_response {
        r_server_response response;
        auto args = request.get_uri().get_get_args(); 

        auto dsID = args["data_source_id"];
        auto tm = args["time"];

        printf("before key_before()\n");
        fflush(stdout);
        auto frame = r_storage::key_before(cfg["storage_config"]["index_path"].get<string>(),
                                           args["data_source_id"],
                                           r_time_utils::iso_8601_to_tp(args["time"]));
        printf("after key_before()\n");
        fflush(stdout);
        response.set_content_type("application/octet-stream");
        response.set_body(std::move(frame));
        return response;
    });

    ws.add_route(METHOD_GET, "/query", [&](const r_web_server<r_socket>& ws, r_utils::r_buffered_socket<r_utils::r_socket>& conn, const r_server_request& request)->r_server_response {
        r_server_response response;
        auto args = request.get_uri().get_get_args();
        bool previousPlayable = true;
        if(args.find("previous_playable") != args.end())
            previousPlayable = (args["previous_playable"] == "false")?false:true;
        bool keyOnly = false;
        if(args.find("key_only") != args.end())
            keyOnly = (args["key_only"] == "true")?true:false;
        auto result = r_storage::query(cfg["storage_config"]["index_path"].get<string>(),
                                       args["data_source_id"],
                                       args["type"],
                                       previousPlayable,
                                       r_time_utils::iso_8601_to_tp(args["start_time"]),
                                       r_time_utils::iso_8601_to_tp(args["end_time"]),
                                       keyOnly);
        response.set_content_type("application/octet-stream");
        response.set_body(std::move(result));
        return response;
    });

    ws.add_route(METHOD_GET, "/sdp_before", [&](const r_web_server<r_socket>& ws, r_utils::r_buffered_socket<r_utils::r_socket>& conn, const r_server_request& request)->r_server_response {
        r_server_response response;
        auto args = request.get_uri().get_get_args();
        auto sdp = r_storage::sdp_before(cfg["storage_config"]["index_path"].get<string>(),
                                         args["data_source_id"],
                                         args["type"],
                                         r_time_utils::iso_8601_to_tp(args["time"]));
        response.set_content_type("application/sdp");
        response.set_body(std::move(sdp));
        return response;
    });

    ws.start();

    vector<data_source> current;
    map<std::string, stream_host> streams;

    while(running)
    {
        auto recordingInDB = ds.recording_data_sources();
        // returns records in recordingInDB but not in current
        auto starting = r_funky::set_diff(recordingInDB, current);
        printv("starting", starting);
        // returns records in current but not in recordingInDB
        auto stopping = r_funky::set_diff(current, recordingInDB);
        printv("stopping", stopping);

        // XXX If camera goes UNHEALTHY its never restarted.

        for(auto s : stopping)
        {
            // NOTE: stopping should also remove from current
            streams[s.id].controls.erase(s.type);
        }
        
        for(auto s : current)
        {
            auto id = s.id;
            auto type = s.type;
            if(streams[id].controls[type] && !streams[id].controls[type]->healthy())
            {
                printf("FOUND UNHEALTHY: %s\n", s.id.c_str());
                current.erase(remove_if(begin(current), end(current), [id, type](const data_source& ds){return ((ds.id == id) && (ds.type == type));}), current.end());
                streams[s.id].controls.erase(s.type);
            }
        }

        for(auto s : starting)
        {
            current.push_back(s);
            streams[s.id].controls[s.type] = _create_stream(s);
        }

        usleep(5000000);
    }

    return 0;
}
