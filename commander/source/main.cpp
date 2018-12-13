
#include "r_http/r_web_server.h"
#include "r_http/r_http_exception.h"
#include "r_disco/r_discovery.h"
#include "r_disco/r_device_info.h"
#include "r_utils/3rdparty/json/json.h"
#include "r_utils/r_functional.h"
#include "r_utils/r_socket.h"
#include "r_http/r_utils.h"
#include "r_http/r_server_request.h"
#include "r_http/r_server_response.h"
#include "assigned_devices.h"
#include "utils.h"
#include <signal.h>
#include <unistd.h>
#include <mutex>
#include <functional>
#include <range/v3/all.hpp>

using namespace r_utils;
using namespace r_http;
using namespace r_disco;
using json = nlohmann::json;
using namespace std;
using namespace std::placeholders;

static const int SERVER_PORT = 11004;

bool running = true;

void handle_sigterm(int sig)
{
    if(sig == SIGTERM)
        running = false;
}

string get_vss_data_sources()
{
    r_socket sok;
    sok.connect("127.0.0.1", 11002);

    r_client_request req("127.0.0.1", 11002);
    req.set_uri("/data_sources");
    req.write_request(sok);

    r_client_response res;
    res.read_response(sok);

    return res.get_body_as_string();
}

void stop_vss_stream(const string& id)
{
// Query data_sources
//    iterate over sources find id=camera_id match
//    delete data_source id
    r_socket sok;
    sok.connect("127.0.0.1", 11002);

    r_client_request req("127.0.0.1", 11002);
    req.set_uri("/data_sources");
    req.set_method(r_http::delete_request);
    req.set_body(r_string_utils::format("{ \"data_source_id\": \"%s\" }", id.c_str());
    req.write_request(sok);

    r_client_response res;
    res.read_response(sok);
}

void start_vss_stream(const string& id, const 
{
    r_socket sok;
    sok.connect("127.0.0.1", 11002);

    r_client_request req("127.0.0.1", 11002);
    req.set_uri("/data_sources");
    req.set_method(r_http::put_request);
    req.set_body(r_string_utils::format("{ \"data_source_id\": \"%s\" }", id.c_str());
    req.write_request(sok);

    r_client_response res;
    res.read_response(sok);
}

int main(int argc, char* argv[])
{
    r_logger::install_terminate();

    signal(SIGTERM, handle_sigterm);

    string firstArgument = (argc > 1)?argv[1]:string();

    if( firstArgument != "--interactive" )
        daemon( 1, 0 );

    auto serverIP = r_utils::ip4_addr_any;

    r_discovery discovery("svms-gateway",
                          "OpenSVMS Gateway",
                          "opensvms",
                          "opensvms.org",
                          "open source on site video management system.",
                          "opensvms.org",
                          "svms-gateway",
                          "0001",
                          "0001",
                          r_utils::ip4_addr_any,
                          80);

    vector<r_device_info> devices;
    std::mutex devicesLock;
    discovery.start_discovery([&](const r_discovery& disco){ 
        std::lock_guard<std::mutex> g(devicesLock);
        devices = disco.get_devices(); 
    });

    assigned_devices ad("/data/commander/assigned_devices");

    r_web_server<r_socket> ws(SERVER_PORT);

    ws.add_route(METHOD_GET, "/assigned_devices", std::bind(&assigned_devices::handle_get, &ad, _1, _2, _3));
    ws.add_route(METHOD_PUT, "/assigned_devices", std::bind(&assigned_devices::handle_put, &ad, _1, _2, _3));
    ws.add_route(METHOD_DELETE, "/assigned_devices", std::bind(&assigned_devices::handle_del, &ad, _1, _2, _3));

    ws.add_route(METHOD_GET, "/status", [](const r_web_server<r_socket>& ws, r_utils::r_buffered_socket<r_utils::r_socket>& conn, const r_server_request& request)->r_server_response {
        r_server_response response;
        response.set_body("{ \"status\": \"healthy\" }");
        return response;
    });

    ws.add_route(METHOD_GET, "/discovered_devices", [&](const r_web_server<r_socket>& ws, r_utils::r_buffered_socket<r_utils::r_socket>& conn, const r_server_request& request)->r_server_response {
        std::lock_guard<std::mutex> g(devicesLock);
        r_server_response response;
        json j;
        for(auto d : devices)
            j["data"]["discovered_devices"] += {{"type", d.type},{"rtsp_url", d.rtsp_url},{"udn", d.unique_id}};
        response.set_body(j.dump());
        return response;
    });

    ws.start();

    while(running)
    {
        {
            lock_guard<mutex> g(devicesLock);
            auto assignedDevices = ad.get();
            vector<string> assignedIDs = assignedDevices | ranges::view::transform([](const pair<string, assigned_device>& p){return p.second.id;});

            auto recordingCameraIDs = get_recording_camera_ids();

            auto startingIDs = r_funky::set_diff(assignedIDs, recordingCameraIDs);
            auto stoppingIDs = r_funky::set_diff(recordingCameraIDs, assignedIDs);

            printf("starting.size() = %lu\n",startingIDs.size());
            printf("stopping.size() = %lu\n",stoppingIDs.size());
        }

        usleep(5000000);
    }

    return 0;
}
