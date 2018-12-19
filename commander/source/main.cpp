
#include "r_http/r_web_server.h"
#include "r_http/r_http_exception.h"
#include "r_disco/r_discovery.h"
#include "r_disco/r_device_info.h"
#include "r_utils/3rdparty/json/json.h"
#include "r_utils/r_functional.h"
#include "r_utils/r_socket.h"
#include "r_http/r_utils.h"
#include "r_http/r_client_request.h"
#include "r_http/r_client_response.h"
#include "r_http/r_server_request.h"
#include "r_http/r_server_response.h"
#include "r_http/r_methods.h"
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

static const int SERVER_PORT = 11006;

bool running = true;

void handle_sigterm(int sig)
{
    if(sig == SIGTERM)
        running = false;
}

void start_vss_recording(const string& uniqueID, const string& rtspURL, const string& type, const string& authPassword, const string& authUserName, const string& transportPref)
{
    try
    {
        r_socket sok;
        sok.connect("127.0.0.1", 11002);

        r_client_request req("127.0.0.1", 11002);
        req.set_uri("/data_sources");
        req.set_method(r_http::METHOD_PUT);
        auto body = r_string_utils::format("{ \"id\": \"%s\", "
                                         "\"type\": \"%s\", "
                                         "\"rtsp_url\": \"%s\", "
                                         "\"recording\": \"%s\", "
                                         "\"transport_pref\": \"%s\", "
                                         "\"auth_username\": \"%s\", "
                                         "\"auth_password\": \"%s\" }",
                                         uniqueID.c_str(),
                                         type.c_str(),
                                         rtspURL.c_str(),
                                         "1",
                                         transportPref.c_str(),
                                         authUserName.c_str(),
                                         authPassword.c_str());

        req.set_body(body);
        req.write_request(sok);

        r_client_response res;
        res.read_response(sok);
    }
    catch(exception& ex)
    {
        printf("ex = %s\n",ex.what());
    }
}

int main(int argc, char* argv[])
{
    r_logger::install_terminate();

    signal(SIGTERM, handle_sigterm);

    string firstArgument = (argc > 1)?argv[1]:string();

    if( firstArgument != "--interactive" )
        daemon( 1, 0 );

    auto serverIP = r_utils::ip4_addr_any;

    r_discovery discovery("1234");

    vector<r_device_info> devices;
    std::mutex devicesLock;
    discovery.start_discovery("upnp:rootdevice", 5, [&](const r_discovery& disco){ 
        std::lock_guard<std::mutex> g(devicesLock);
        devices = disco.get_devices();
    });

    assigned_devices ad("/data/commander/assigned_devices");

    r_web_server<r_socket> ws(SERVER_PORT);

    // Example PUT of devices to /assigned_devices
    //     curl --header "Content-Type: application/json" --request PUT --data '{ "id": "ea1767e6-f7e9-0d1d-c548-1d21beb28964", "type": "camera", "rtsp_url": "rtsp://127.0.0.1:11005/video/the_white_wolf_1280x720.mp4" }' http://127.0.0.1:11006/assigned_devices
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
        if(devices.size() > 0)
        {
            json j;
            for(auto d : devices)
                j["data"]["discovered_devices"] += {{"type", d.type},{"rtsp_url", d.rtsp_url},{"unique_id", d.unique_id}};
            response.set_body(j.dump());
        }
        else response.set_body("{ \"data\": { \"discovered_devices\": [] } }");
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

            for(auto sid : startingIDs)
            {
                for(auto d : devices)
                {
                    if(sid == d.unique_id)
                    {
                        start_vss_recording(d.unique_id, d.rtsp_url, d.type, d.auth_password, d.auth_user, d.transport_pref);
                    }
                }
            }

            printf("starting.size() = %lu\n",startingIDs.size());
            printf("stopping.size() = %lu\n",stoppingIDs.size());
        }

        usleep(5000000);
    }

    return 0;
}
