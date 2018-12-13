
#ifndef _vss_assigned_devices_h
#define _vss_assigned_devices_h

#include "r_http/r_web_server.h"
#include <map>
#include <string>
#include <vector>

struct assigned_device
{
    std::string id;
    std::string type;
    std::string rtsp_url;
};

bool operator==(const assigned_device& a, const assigned_device& b);

class assigned_devices
{
public:
    assigned_devices(const std::string& devicesPath);
    r_http::r_server_response handle_get(const r_http::r_web_server<r_utils::r_socket>& ws,
                                         r_utils::r_buffered_socket<r_utils::r_socket>& conn,
                                         const r_http::r_server_request& request);
    r_http::r_server_response handle_put(const r_http::r_web_server<r_utils::r_socket>& ws,
                                         r_utils::r_buffered_socket<r_utils::r_socket>& conn,
                                         const r_http::r_server_request& request);
    r_http::r_server_response handle_del(const r_http::r_web_server<r_utils::r_socket>& ws,
                                         r_utils::r_buffered_socket<r_utils::r_socket>& conn,
                                         const r_http::r_server_request& request);

    std::map<std::string, assigned_device> get();
private:
    static void _upgrade_db(const std::string& devicesPath);
    std::string _devicesPath;
};

#endif
