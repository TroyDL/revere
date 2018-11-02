
#ifndef _r_rtsp_sdp_h
#define _r_rtsp_sdp_h

#include <string>
#include <chrono>
#include <vector>
#include <list>
#include <algorithm>
#include <assert.h>
#include "r_utils/r_string.h"
#include "r_utils/r_exception.h"

namespace r_rtsp
{

struct r_sdp_origin
{
    std::string username;
    std::string session_id;
    std::string session_version;
    std::string nettype;
    std::string addrtype;
    std::string unicast_address;
};

struct r_connection
{
    std::string nettype;
    std::string address_type;
    std::string connection_address;
};

struct r_bandwidth
{
    std::string type;
    std::string value;
};

struct r_encryption
{
    std::string method;
    std::string key;
};

struct r_attributes
{
    std::vector<std::string> property_attributes; // a=foo
    std::vector<std::string> value_attributes;
};

struct r_media_description
{
    std::string type;                  // m=
    int port;
    std::string proto;
    std::string fmt;

    std::string codec;
    uint64_t time_base;
    std::string encoded_sps;
    std::string encoded_pps;
    std::string control;

    std::string info;                    // i=
    r_connection conn;                   // c=
    r_bandwidth bw;                      // b=
    r_encryption enc;                    // k=
    r_attributes attr;                   // a=
};

struct r_session_description
{
    std::string protocol_version {"0"}; // v=
    r_sdp_origin origin;                // o=
    std::string session_name;           // s=
    std::string info;                   // i=
    std::string uri;                    // u=
    std::string email_address;          // e=
    std::string phone_number;           // p=
    r_connection conn;                  // c=
    r_bandwidth bw;                     // b=
    r_encryption enc;                   // k=
    r_attributes attr;                  // a=foo
    std::vector<r_media_description> media_descriptions;
};

r_session_description parse_sdp(const std::string& sdp);

std::vector<uint8_t> create_annexb_extradata(const std::string& encodedSPS, const std::string& encodedPPS);

}

#endif
