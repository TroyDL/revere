
#ifndef __revere_ws_h
#define __revere_ws_h

#include "r_http/r_web_server.h"
#include "r_http/r_http_exception.h"
#include "r_utils/r_socket.h"
#include "r_disco/r_devices.h"

class ws final
{
public:
    ws(const std::string& top_dir, r_disco::r_devices& devices);
    ~ws();

private:
    r_http::r_server_response _get_jpg(const r_http::r_web_server<r_utils::r_socket>& ws,
                                       r_utils::r_buffered_socket<r_utils::r_socket>& conn,
                                       const r_http::r_server_request& request);

    std::string _top_dir;
    r_disco::r_devices& _devices;
    r_http::r_web_server<r_utils::r_socket> _server;
};

#endif
