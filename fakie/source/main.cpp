
#include "r_rtsp/r_fake_camera.h"
#include "r_av/r_locky.h"
#include "r_utils/r_logger.h"
#include "r_utils/r_file.h"
#include "r_utils/r_args.h"
#include "r_http/r_web_server.h"
#include "r_disco/r_discovery.h"
#include <signal.h>
#include <unistd.h>
#include <vector>
#include <string>
#include <algorithm>
#include <sys/utsname.h>

using namespace r_utils;
using namespace r_http;
using namespace r_disco;
using namespace std;

const int SERVER_PORT = 11004;
const int RTSP_PORT = 11005;

bool running = true;

void handle_sigterm(int sig)
{
    if(sig == SIGTERM)
        running = false;
}

string _get_kernel_name_version()
{
    struct utsname uinfo;
    uname(&uinfo);

    return r_string_utils::format("%s/%s", uinfo.sysname, uinfo.release);
}

string _get_ssdp_server()
{
    return r_string_utils::format("%s UPnP/1.0 Fakie/1.0.0", _get_kernel_name_version().c_str());
}

int main(int argc, char* argv[])
{
    auto args = r_args::parse_arguments(argc, argv);

    r_logger::install_terminate();

    r_av::r_locky::register_ffmpeg();

    signal(SIGTERM, handle_sigterm);

    string firstArgument = (argc > 1) ? argv[1] : string();

    auto ifname = r_args::get_required_argument(args, "--interface");
    auto interactive = r_args::get_optional_argument(args, "--interactive");

    if(interactive.is_null())
        daemon( 1, 0 );

    auto addresses = r_networking::r_get_interface_addresses(AF_INET);

    string serverIP = ip4_addr_any;
    if(addresses.find(ifname) != addresses.end())
    {
        auto address = addresses[ifname];

        if(address.size() > 0)
            serverIP = address[0];
    }
    printf("ifname = %s\n",ifname.c_str());
    printf("serverIP = %s\n",serverIP.c_str());

    r_discovery disco(r_networking::r_get_device_uuid(ifname),
                      serverIP,
                      SERVER_PORT);

    disco.start_discoverable(r_string_utils::format("http://%s:%d/Device.xml", serverIP.c_str(), SERVER_PORT),
                             _get_ssdp_server(),
                             "cirrus-fakie");

    r_rtsp::r_fake_camera fc(".", RTSP_PORT);
    fc.start();

    r_web_server<r_socket> ws(SERVER_PORT);

    ws.add_route(METHOD_GET, "/status", [](const r_web_server<r_socket>& ws, r_utils::r_buffered_socket<r_utils::r_socket>& conn, const r_server_request& request)->r_server_response {
        r_server_response response;
        response.set_body("{ \"status\": \"healthy\" }");
        return response;
    });

    ws.add_route(METHOD_GET, "/Device.xml", [](const r_web_server<r_socket>& ws, r_utils::r_buffered_socket<r_utils::r_socket>& conn, const r_server_request& request)->r_server_response {
        r_server_response response;
        response.set_content_type("application/xml");
        response.set_body( "<root xmlns=\"urn:schemas-upnp-org:device-1-0\"> "
                            "<specVersion> "
                            "<major>1</major> "
                            "<minor>0</minor> "
                            "</specVersion> "
                            "<device> "
                            "<deviceType>urn:schemas-upnp-org:device:Camera:1</deviceType> "
                            "<friendlyName>fakie</friendlyName> "
                            "<manufacturer>fakie</manufacturer> "
                            "<manufacturerURL>fakie.com</manufacturerURL> "
                            "<modelDescription>Fake camera</modelDescription> "
                            "<modelURL>fakie.com</modelURL> "
                            "<modelName>fakie</modelName> "
                            "<serialNumber>D370650</serialNumber> "
                            "<UDN>uuid:0800C006-8000-0001-0802-788602E24444</UDN> "
                            "<StreamLocation>rtsp://127.0.0.1:11005/video/the_white_wolf_1280x720.mp4</StreamLocation> "
                            "<HasVideo>yes</HasVideo> "
                            "<HasMetadata>no</HasMetadata> "
                            "<HasObjectTracker>no</HasObjectTracker> "
                            "</device> "
                            "</root>" );

        return response;
    });

    ws.start();

    while(running)
    {
        usleep(500000);
    }

    fc.stop();

    r_av::r_locky::unregister_ffmpeg();

    return 0;
}
