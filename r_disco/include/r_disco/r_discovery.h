
#ifndef r_discovery_r_discovery_h
#define r_discovery_r_discovery_h

#include "r_utils/r_socket.h"
#include "r_utils/r_socket_address.h"
#include "r_utils/r_server_threaded.h"
#include "r_utils/r_nullable.h"
#include "r_http/r_web_server.h"
#include "r_http/r_server_request.h"
#include "r_http/r_server_response.h"

#include <thread>
#include <map>
#include <future>
#include <mutex>
#include <functional>
#include <vector>

namespace r_disco
{

class r_discovery final
{
public:
    r_discovery(const std::string& deviceType,
                const std::string& friendlyName,
                const std::string& manufacturer,
                const std::string& manufacturerURL,
                const std::string& modelDescription,
                const std::string& modelURL,
                const std::string& modelName,
                const std::string& serialNumber,
                const std::string& packageVersion,
                const std::string& serverIP = r_utils::ip4_addr_any,
                int serverPort = 80);
    r_discovery(const r_discovery&) = delete;
    ~r_discovery() throw();

    r_discovery& operator = (const r_discovery&) = delete;

    // Use start_discovery to detect devices by periodically sending MSEARCHs and
    // listening for NOTIFYs.
    void start_discovery(std::function<void(const r_discovery& disco)> deviceChangedCB);
    std::vector<std::string> get_devices() const;

    // Use start_discoverable to respond to listen for MSEARCHs and respond by sending
    // NOTIFYs.
    void start_discoverable();

    void stop();

private:
    void _discovery_entry();
    int _cache_drop_old();
    void _update_cache(const std::string& loc);

    void _discoverable_entry();
    r_http::r_server_response _discoverable_get_device(const r_http::r_web_server<r_utils::r_socket>& ws, r_utils::r_buffered_socket<r_utils::r_socket>& conn, const r_http::r_server_request& request);
    r_http::r_server_response _discoverable_get_manifest(const r_http::r_web_server<r_utils::r_socket>& ws, r_utils::r_buffered_socket<r_utils::r_socket>& conn, const r_http::r_server_request& request);

    std::string _deviceType;
    std::string _friendlyName;
    std::string _manufacturer;
    std::string _manufacturerURL;
    std::string _modelDescription;
    std::string _modelURL;
    std::string _modelName;
    std::string _serialNumber;
    std::string _packageVersion;
    std::thread _worker;
    bool _running;
    int _sok;
    std::string _deviceID;

    mutable std::recursive_mutex _discoveryStateLock;

    std::map<std::string,std::chrono::steady_clock::time_point> _deviceCache;
    std::function<void(const r_discovery& disco)> _deviceChangedCB;

    std::string _serverIP;
    int _serverPort;

    r_http::r_web_server<r_utils::r_socket> _webServer;
};

}

#endif
