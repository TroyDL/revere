
#include "r_disco/r_discovery.h"
#include "r_utils/r_string_utils.h"
#include "r_utils/r_nullable.h"
#include "r_utils/r_time_utils.h"
#include "r_utils/r_file.h"
#include "r_utils/r_socket.h"
#include "r_http/r_server_request.h"
#include "r_http/r_server_response.h"
#include "r_disco/r_cirrus_device_info_agent.h"

#include <algorithm>
#include <functional>

using namespace std;
using namespace std::placeholders;
using namespace std::chrono;
using namespace r_utils;
using namespace r_disco;
using namespace r_http;

static const int SSDP_PORT = 1900;
static const char* SSDP_ADDRESS = "239.255.255.250";

static const int DEVICE_EXPIRATION_SECONDS = (60 * 10);
static const int QUARTER_SECONDS_BETWEEN_MSEARCH = 1200;
static const int PACKET_SIZE = 1500;

static void _send_message(int sok,
                          const string& destIP,
                          const string& message)
{
    vector<uint8_t> notifyPacket(1500);
    memcpy(&notifyPacket[0], message.c_str(), message.length());

    sockaddr_in addr;

    // Now, create _addr to point at the desired target.
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SSDP_PORT);

    inet_pton(AF_INET, destIP.c_str(), &addr.sin_addr);

    auto p = &notifyPacket[0];

    sendto(sok,
           (char*)p,
           message.length(),
           0,
           (sockaddr*)&addr,
           sizeof(addr));

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SSDP_PORT);

    inet_pton(AF_INET, "255.255.255.255", &addr.sin_addr);

    sendto(sok,
           (char*)p,
           message.length(),
           0,
           (sockaddr*)&addr,
           sizeof( addr ));
}

static void _send_msearch(int sok, const string& serviceType, int maxLatency)
{
    string msearch = r_string_utils::format("M-SEARCH * HTTP/1.1\r\n"
                                            "ST: %s\r\n"
                                            "MX: %d\r\n"
                                            "MAN: \"ssdp:discover\"\r\n"
                                            "HOST: %s:%d\r\n"
                                            "\r\n",
                                            serviceType.c_str(),
                                            maxLatency,
                                            SSDP_ADDRESS,
                                            SSDP_PORT);

    _send_message(sok, SSDP_ADDRESS, msearch);
}

static void _send_msearch_response(const string& destIP,
                                   const string& deviceID,
                                   const string& locationURL,
                                   const string& server,
                                   const string& serviceType)

{
    int clientSok = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(clientSok < 0)
        R_THROW(("Unable to create a datagram socket."));

    int on = 1;
    int err = (int)::setsockopt(clientSok, SOL_SOCKET, SO_REUSEADDR, (const char *)&on, sizeof(int));
    if( err < 0 )
    {
        ::close(clientSok);
        R_THROW(("Unable to configure socket."));
    }

    try
    {
        string response = r_string_utils::format("HTTP/1.1 200 OK\r\n"
                                                 "CACHE-CONTROL: max-age=1800\r\n"
                                                 "LOCATION: %s\r\n"
                                                 "SERVER: %s\r\n"
                                                 "ST: %s\r\n"
                                                 "USN: %s\r\n\r\n",
                                                 locationURL.c_str(),
                                                 server.c_str(),
                                                 serviceType.c_str(),
                                                 r_string_utils::format("uuid:%s",deviceID.c_str()).c_str());

        _send_message(clientSok, destIP, response);
    }
    catch(r_exception& ex)
    {
        R_LOG_NOTICE("%s",ex.what());
        ::close(clientSok);
        throw ex;
    }

    ::close(clientSok);
}

static void _send_notify(const string& locationURL,
                         const string& server,
                         const string& deviceID)
{
    int clientSok = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(clientSok < 0)
        R_THROW(("Unable to create a datagram socket."));

    int on = 1;
    int err = (int)::setsockopt(clientSok, SOL_SOCKET, SO_REUSEADDR, (const char *)&on, sizeof(int));
    if( err < 0 )
    {
        ::close(clientSok);
        R_THROW(("Unable to configure socket."));
    }

    try
    {
        string notify = r_string_utils::format("NOTIFY * HTTP/1.1\r\n"
                                               "CACHE-CONTROL: max-age=1800\r\n"
                                               "HOST: 239.255.255.250:1900\r\n"
                                               "LOCATION: %s\r\n"
                                               "NT: upnp:rootdevice\r\n"
                                               "NTS: ssdp:alive\r\n"
                                               "SERVER: %s\r\n"
                                               "USN: %s\r\n\r\n",
                                               locationURL.c_str(),
                                               server.c_str(),
                                               r_string_utils::format("uuid:%s",deviceID.c_str()).c_str());

        _send_message(clientSok, SSDP_ADDRESS, notify);
    }
    catch(r_exception& ex)
    {
        R_LOG_NOTICE("%s",ex.what());
        ::close(clientSok);
        throw ex;
    }

    ::close(clientSok);
}

static void _set_socket_options(int sok)
{
    // We apply these options repeatedly while running. Ignore error if it has already
    // been set.

    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = inet_addr(SSDP_ADDRESS);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);

    if(setsockopt(sok, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq, sizeof(mreq)) < 0)
    {
        if( errno != EADDRINUSE )
            R_LOG_ERROR("setsockopt IP_ADD_MEMBERSHIP: %s.", strerror(errno));
    }

    uint8_t loop=0;
    if( setsockopt(sok, IPPROTO_IP, IP_MULTICAST_LOOP, (char*)&loop, sizeof(loop)) < 0)
        R_LOG_ERROR("setsockopt IP_MULTICAST_LOOP: %s.", strerror(errno));

    uint8_t ttl=5;
    if( setsockopt(sok, IPPROTO_IP, IP_MULTICAST_TTL, (char*)&ttl, sizeof(ttl)) < 0)
        R_LOG_ERROR("setsockopt IP_MULTICAST_TTL: %s.", strerror(errno));

    int broadcast=1;
    if( setsockopt(sok, SOL_SOCKET, SO_BROADCAST, (char*)&broadcast, sizeof(broadcast)) < 0)
        R_LOG_ERROR("setsockopt SO_BROADCAST: %s.", strerror(errno));
}

r_discovery::r_discovery( const string& deviceID,
                          const string& serverIP,
                          int serverPort) :
    _worker(),
    _running(false),
    _sok(-1),
    _deviceID(deviceID),
    _discoveryStateLock(),
    _deviceCache(),
    _deviceChangedCB(),
    _serverIP(serverIP),
    _serverPort(serverPort),
    _recognizer()
{
    _sok = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(_sok < 0)
        R_THROW(("Unable to create a datagram socket."));

    int on = 1;
    int err = (int)::setsockopt((int)_sok, SOL_SOCKET, SO_REUSEADDR, (const char *)&on, sizeof(int));
    if( err < 0 )
        R_THROW(("Unable to configure socket."));

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(SSDP_PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(::bind(_sok, (struct sockaddr*)&addr, sizeof(addr)) < 0)
        R_THROW(("Unable to bind to interface."));

    _recognizer.register_device_info_agent(make_shared<r_cirrus_device_info_agent>());
}

r_discovery::~r_discovery() throw()
{
    if(_sok > -1)
        ::close(_sok);

    _sok = -1;

    stop();
}

void r_discovery::start_discovery(const string& serviceType, int maxLatency, std::function<void(const r_discovery& disco)> deviceChangedCB)
{
    _deviceChangedCB = deviceChangedCB;

    _running = true;

    _worker = thread(&r_discovery::_discovery_entry, this, serviceType, maxLatency);
}

vector<r_device_info> r_discovery::get_devices() const
{
    vector<r_device_info> devices;

    {
        unique_lock<recursive_mutex> g(_discoveryStateLock);
        for(auto i = _deviceCache.begin(), e = _deviceCache.end(); i != e; ++i)
            devices.push_back(i->first);
    }

    return devices;
}

void r_discovery::start_discoverable(const string& locationURL, const string& server, const string& serviceType)
{
    _running = true;
    _worker = thread(&r_discovery::_discoverable_entry, this, locationURL, server, serviceType);
}

void r_discovery::stop()
{
    if( _running )
    {
        _running = false;
        _worker.join();
    }
}

void r_discovery::_discovery_entry(const string& serviceType, int maxLatency)
{
    char buffer[PACKET_SIZE];

    int searchCountdown = 0;

    sleep(2);

    while(_running)
    {
        if(searchCountdown <= 0)
        {
            _set_socket_options(_sok);
            _send_msearch(_sok, serviceType, maxLatency);
            searchCountdown = QUARTER_SECONDS_BETWEEN_MSEARCH;
        }

        --searchCountdown;

        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(_sok, &rfds);

        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 250000;

        int sret = select(_sok+1, &rfds, NULL, NULL, &timeout);

        if(sret > 0)
        {
            struct sockaddr_storage their_addr;
            socklen_t addr_len = sizeof(their_addr);

            int bytesReceived = recvfrom(_sok,
                                         buffer,
                                         PACKET_SIZE,
                                         0,
                                         (struct sockaddr*)&their_addr,
                                         &addr_len);

            if(bytesReceived <= 0)
                continue;

            buffer[bytesReceived] = '\0';

            string msg((char*)&buffer[0],strlen((char*)&buffer[0]));

            auto di = _recognizer.get_device_info(msg);

            if(!di.is_null())
                _update_cache(di.value());
        }
    }
}

int r_discovery::_cache_drop_old()
{
    int dropped = 0;

    auto now = steady_clock::now();

    for(auto i = _deviceCache.begin(), e = _deviceCache.end(); i != e; ++i)
    {
        if(duration_cast<seconds>(now - i->second).count() > DEVICE_EXPIRATION_SECONDS)
        {
            i = _deviceCache.erase(i);
            ++dropped;
        }
    }

    return dropped;
}

void r_discovery::_update_cache(const r_device_info& di)
{
    int expired = 0, erased = 0;

    {
        unique_lock<recursive_mutex> g(_discoveryStateLock);

        // Drop anything from cache older than DEVICE_EXPIRATION_SECONDS...
        expired = _cache_drop_old();

        erased = _deviceCache.erase(di);

        _deviceCache.insert(make_pair(di, steady_clock::now()));
    }

    if((expired > 0) || (erased == 0))
        _deviceChangedCB(*this);
}

void r_discovery::_discoverable_entry(const string& locationURL, const string& server, const string& serviceType)
{
    char buffer[PACKET_SIZE];

    sleep(2);

    _send_notify(locationURL, server, _deviceID);

    while(_running)
    {
        _set_socket_options(_sok);

        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(_sok, &rfds);

        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        int sret = select(_sok+1, &rfds, NULL, NULL, &timeout);

        if(sret > 0)
        {
            struct sockaddr_storage their_addr;
            socklen_t addr_len = sizeof(their_addr);

            int bytesReceived = recvfrom(_sok,
                                         buffer,
                                         PACKET_SIZE,
                                         0,
                                         (struct sockaddr*)&their_addr,
                                         &addr_len);

            if(bytesReceived <= 0)
                continue;

            buffer[bytesReceived] = '\0';

            string msg((char*)&buffer[0],strlen((char*)&buffer[0]));

            if(r_string_utils::contains(msg, "M-SEARCH") && r_string_utils::contains(msg, "rootdevice") && their_addr.ss_family == AF_INET)
            {
                char ipstr[INET6_ADDRSTRLEN];

                inet_ntop(their_addr.ss_family,
                          &((struct sockaddr_in *)&their_addr)->sin_addr,
                          ipstr,
                          sizeof(ipstr));

                string destIP = ipstr;

                _send_msearch_response(ipstr,
                                       _deviceID,
                                       locationURL,
                                       server,
                                       serviceType);
            }
        }
    }
}
