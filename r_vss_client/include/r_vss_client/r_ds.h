
#ifndef _r_vss_client_r_ds_h
#define _r_vss_client_r_ds_h

#include <string>
#include <chrono>
#include <vector>

namespace r_vss_client
{

struct data_source
{
    std::string auth_password;
    std::string auth_username;
    std::string id;
    bool recording;
    std::string rtsp_url;
    std::string transport_pref;
    std::string type;
};

std::string fetch_data_sources_json();

std::vector<data_source> parse_data_sources(const std::string& ds);

}

#endif