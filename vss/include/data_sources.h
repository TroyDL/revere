
#ifndef _vss_data_sources_h
#define _vss_data_sources_h

#include "r_http/r_web_server.h"
#include <map>
#include <string>
#include <vector>

struct data_source
{
    std::string id;
    std::string type;
    std::string rtsp_url;
    bool recording;
    std::string transport_pref;
    std::string auth_username;
    std::string auth_password;
};

bool operator==(const data_source& a, const data_source& b);

class data_sources
{
public:
    data_sources(const std::string& dataSourcesPath);
    r_http::r_server_response handle_get(const r_http::r_web_server<r_utils::r_socket>& ws,
                                         r_utils::r_buffered_socket<r_utils::r_socket>& conn,
                                         const r_http::r_server_request& request);
    r_http::r_server_response handle_put(const r_http::r_web_server<r_utils::r_socket>& ws,
                                         r_utils::r_buffered_socket<r_utils::r_socket>& conn,
                                         const r_http::r_server_request& request);
    r_http::r_server_response handle_del(const r_http::r_web_server<r_utils::r_socket>& ws,
                                         r_utils::r_buffered_socket<r_utils::r_socket>& conn,
                                         const r_http::r_server_request& request);
    std::vector<data_source> recording_data_sources();
private:
    static void _upgrade_db(const std::string& dataSourcesPath);
    std::string _dataSourcesPath;
};

#endif
