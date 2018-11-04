#include <string>
#include <chrono>
#include <vector>
#include <list>
#include <algorithm>
#include <assert.h>
#include "r_utils/r_string_utils.h"
#include "r_utils/r_exception.h"

using namespace std;
using namespace r_utils;

struct sdp_origin
{
    std::string username;
    std::string session_id;
    std::string session_version;
    std::string nettype;
    std::string addrtype;
    std::string unicast_address;
};

struct connection
{
    std::string nettype;
    std::string address_type;
    std::string connection_address;
};

struct bandwidth
{
    std::string type;
    std::string value;
};

struct encryption
{
    std::string method;
    std::string key;
};

struct attributes
{
    std::vector<std::string> property_attributes; // a=foo
    std::vector<std::string> value_attributes;
};

struct media_description
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

    std::string info;                  // i=
    connection conn;                   // c=
    bandwidth bw;                      // b=
    encryption enc;                    // k=
    attributes attr;                   // a=
};

struct session_description
{
    std::string protocol_version {"0"}; // v=
    sdp_origin origin;                  // o=
    std::string session_name;           // s=
    std::string info;                   // i=
    std::string uri;                    // u=
    std::string email_address;          // e=
    std::string phone_number;           // p=
    connection conn;                    // c=
    bandwidth bw;                       // b=
    encryption enc;                     // k=
    attributes attr;                    // a=foo
    std::vector<media_description> media_descriptions;
};

connection _parse_connection(const std::string& c)
{
    auto connParts = r_string_utils::split(c, " ");
    if(connParts.size() < 3)
        R_THROW(("Unable to parse connection line in sdp."));
    connection cd;
    cd.nettype = connParts[0];
    cd.address_type = connParts[1];
    cd.connection_address = connParts[2];
    return cd;
}

bandwidth _parse_bandwidth(const std::string& b)
{
    auto bwParts = r_string_utils::split(b, ":");
    if(bwParts.size() != 2)
        R_THROW(("Unable to parse bandwidth."));
    bandwidth bw;
    bw.type = bwParts[0];
    bw.value = bwParts[1];
    return bw;
}

encryption _parse_encryption(const std::string& k)
{
    encryption enc;
    if(r_string_utils::contains(k, ":"))
    {
        auto keyParts = r_string_utils::split(k, ":");
        if(keyParts.size() != 2)
            R_THROW(("Unable to parse encryption line in sdp."));
        enc.method = keyParts[0];
        enc.key = keyParts[1];
    }
    else enc.method = k;
    return enc;
}

void _parse_media_description(media_description& md, const std::string& m)
{
    auto mparts = r_string_utils::split(m, " ");
    if(mparts.size() < 4)
        R_THROW(("Unable to parse m= line in sdp"));
    md.type = mparts[0];
    md.port = r_string_utils::s_to_int(mparts[1]);
    md.proto = mparts[2];
    md.fmt = mparts[3];
}

session_description parse_sdp(const std::string& sdp)
{
    session_description sdesc;

    auto lines = r_string_utils::split(sdp, "\n");

    auto md_it = find_if(lines.begin(), lines.end(), [](const string& s)->bool{return r_string_utils::contains(s, "m=");});

    list<string> globalLines;

    auto i = lines.begin();
    for(; i != md_it; ++i)
        globalLines.push_back(*i);

    list<list<string>> mds;

    while(md_it != lines.end())
    {
        md_it = find_if(md_it+1, lines.end(), [](const string& s)->bool{return r_string_utils::contains(s, "m=");});
        list<string> md;
        for(; i != md_it; ++i)
            md.push_back(*i);
        mds.push_back(md);
    }

    for(auto l : globalLines)
    {
        auto lt = l.substr(0, 2);
        auto lv = l.substr(2);

        if(lt == "v=")
            sdesc.protocol_version = lv;
        else if(lt == "o=")
        {
            auto originParts = r_string_utils::split(lv, " ");
            size_t numParts = originParts.size();

            if(numParts>0)
                sdesc.origin.username = originParts[0];
            if(numParts>1)
                sdesc.origin.session_id = originParts[1];
            if(numParts>2)
                sdesc.origin.session_version = originParts[2];
            if(numParts>3)
                sdesc.origin.nettype = originParts[3];
            if(numParts>4)
                sdesc.origin.addrtype = originParts[4];
            if(numParts>5)
                sdesc.origin.unicast_address = originParts[5];
        }
        else if(lt == "s=")
            sdesc.session_name = lv;
        else if(lt == "i=")
            sdesc.info = lv;
        else if(lt == "u=")
            sdesc.uri = lv;
        else if(lt == "e=")
            sdesc.email_address = lv;
        else if(lt == "p=")
            sdesc.phone_number = lv;
        else if(lt == "c=")
            sdesc.conn = _parse_connection(lv);
        else if(lt == "b=")
            sdesc.bw = _parse_bandwidth(lv);
        else if(lt == "k=")
            sdesc.enc = _parse_encryption(lv);
        else if(lt == "a=")
        {
            if(r_string_utils::contains(lv, ":"))
                sdesc.attr.value_attributes.push_back(lv);
            else sdesc.attr.property_attributes.push_back(lv);
        }
    }

    for(auto& md : mds)
    {
        media_description mdesc;

        for(auto l : md)
        {
            auto lt = l.substr(0, 2);
            auto lv = l.substr(2);

            if(lt == "m=")
                _parse_media_description(mdesc, lv);
            else if(lt == "i=")
                mdesc.info = lv;
            else if(lt == "c=")
                mdesc.conn = _parse_connection(lv);
            else if(lt == "b=")
                mdesc.bw = _parse_bandwidth(lv);
            else if(lt == "k=")
                mdesc.enc = _parse_encryption(lv);
            else if(lt == "a=")
            {
                if(r_string_utils::contains(lv, ":"))
                {
                    mdesc.attr.value_attributes.push_back(lv);
                    
                    if(mdesc.codec.empty())
                    {
                        if(r_string_utils::starts_with(lv, "rtpmap"))
                        {
                            auto rtpMapParts = r_string_utils::split(lv, " ");

                            if(rtpMapParts.size() < 2)
                                R_THROW(("Unable to parse rtpmap line in sdp."));

                            if(r_string_utils::contains(rtpMapParts[1], "/"))
                            {
                                auto codecParts = r_string_utils::split(rtpMapParts[1], "/");

                                if(codecParts.size() < 2)
                                    R_THROW(("Unable to parse codec in sdp."));

                                mdesc.codec = codecParts[0];
                                mdesc.time_base = r_string_utils::s_to_uint64(codecParts[1]);
                            }
                            else mdesc.codec = rtpMapParts[1];
                        }
                    }

                    if(mdesc.encoded_sps.empty())
                    {
                        if(r_string_utils::contains(lv, "sprop-parameter-sets"))
                        {
                            auto mainParts = r_string_utils::split(lv, ";");
                            for(auto mp : mainParts)
                            {
                                if(r_string_utils::contains(mp, "sprop-parameter-sets"))
                                {
                                    auto spv = mp.substr(mp.find("=")+1);

                                    if(r_string_utils::contains(spv, ","))
                                    {
                                        auto pparts = r_string_utils::split(spv, ",");
                                        mdesc.encoded_sps = pparts[0];

                                        if(r_string_utils::contains(pparts[1], ";"))
                                            mdesc.encoded_pps = pparts[1].substr(0, pparts[1].length()-1);
                                        else mdesc.encoded_pps = pparts[1];
                                    }
                                    else mdesc.encoded_sps = spv;
                                }
                            }
                        }
                    }

                    if(mdesc.control.empty())
                    {
                        if(r_string_utils::contains(lv, "control"))
                            mdesc.control = lv.substr(lv.find("control:") + 8);
                    }
                }
                else mdesc.attr.property_attributes.push_back(lv);
            }
        }

        sdesc.media_descriptions.push_back(mdesc);
    }

    return sdesc;
}

int main(int argc, char* argv[])
{
    {
        auto desc = parse_sdp("o=- 0 0 IN IP4 127.0.0.1\n"
                              "s=No Name\n"
                              "t=0 0\n"
                              "a=tool:libavformat 57.83.100\n"
                              "m=video 0 RTP/AVP 96\n"
                              "b=AS:1766\n"
                              "a=rtpmap:96 H264/90000\n"
                              "a=fmtp:96 packetization-mode=1; sprop-parameter-sets=Z2QAH6yyAKALdCAAAAMAIAAAAwPB4wZJ,aOvDyyLA; profile-level-id=64001F\n"
                              "a=control:streamid=0\n"
                              "m=audio 49230 RTP/AVP 96 97 98\n"
                              "b=AS:1766\n"
                              "a=rtpmap:96 L8/8000\n"
                              "a=rtpmap:97 L16/8000\n"
                              "a=rtpmap:98 L16/11025/2\n");

        assert(desc.media_descriptions.size() == 2);

        assert(desc.media_descriptions[0].type == "video");
        assert(desc.media_descriptions[0].codec == "H264");
        assert(desc.media_descriptions[0].time_base == 90000);
        assert(desc.media_descriptions[0].encoded_sps == "Z2QAH6yyAKALdCAAAAMAIAAAAwPB4wZJ");
        assert(desc.media_descriptions[0].encoded_pps == "aOvDyyLA");
        assert(desc.media_descriptions[0].control == "streamid=0");

        assert(desc.media_descriptions[1].type == "audio");
        assert(desc.media_descriptions[1].codec == "L8");
        assert(desc.media_descriptions[1].time_base == 8000);
    }

    {
        auto desc = parse_sdp("o=- 0 0 IN IP4 127.0.0.1\n"
                              "s=No Name\n"
                              "t=0 0\n"
                              "a=tool:libavformat 57.83.100\n"
                              "m=video 0 RTP/AVP 96\n"
                              "b=AS:1766\n"
                              "a=rtpmap:96 H264/90000\n"
                              "a=fmtp:96 packetization-mode=1; sprop-parameter-sets=Z2QAH6yyAKALdCAAAAMAIAAAAwPB4wZJ,aOvDyyLA; profile-level-id=64001F\n"
                              "a=control:streamid=42\n");

        assert(desc.media_descriptions.size() == 1);
        assert(desc.media_descriptions[0].type == "video");
        assert(desc.media_descriptions[0].codec == "H264");
        assert(desc.media_descriptions[0].time_base == 90000);
        assert(desc.media_descriptions[0].encoded_sps == "Z2QAH6yyAKALdCAAAAMAIAAAAwPB4wZJ");
        assert(desc.media_descriptions[0].encoded_pps == "aOvDyyLA");
        assert(desc.media_descriptions[0].control == "streamid=42");
    }

    {
        // HIKVision MP4V
        auto desc = parse_sdp("v=0\n"
                              "o=- 1109162014219182 1109162014219192 IN IP4 x.y.z.w\n"
                              "s=Media Presentation\n"
                              "e=NONE\n"
                              "c=IN IP4 0.0.0.0\n"
                              "t=0 0\n"
                              "m=video 0 RTP/AVP 96\n"
                              "a=rtpmap:96 MP4V-ES/90000\n"
                              "a=fmtp:96 profile-level-id=8;config=000001B0F5000001B50900000100000001200886C400670CA0116851;\n"
                              "a=range:ntp=now\n"
                              "a=control:trackID=1\n"
                              "m=audio 0 RTP/AVP 0\n"
                              "a=rtpmap:0 PCMU/8000\n"
                              "a=control:trackID=2\n"
                              "a=Media_header:MEDIAINFO=494D4B48010100000400010010710110401F000000FA000000000000000000000000000000000000;\n"
                              "a=appversion:1.0\n");

        assert(desc.media_descriptions.size() == 2);
        assert(desc.media_descriptions[0].type == "video");
        assert(desc.media_descriptions[0].codec == "MP4V-ES");
        assert(desc.media_descriptions[0].time_base == 90000);
        assert(desc.media_descriptions[0].encoded_sps == "");
        assert(desc.media_descriptions[0].encoded_pps == "");
        assert(desc.media_descriptions[0].control == "trackID=1");
    }

    {
        // HIKVision h.264
        auto desc = parse_sdp("v=0\n"
                              "o=- 1109162014219182 1109162014219192 IN IP4 x.y.z.w\n"
                              "s=Media Presentation\n"
                              "e=NONE\n"
                              "c=IN IP4 0.0.0.0\n"
                              "t=0 0\n"
                              "m=video 0 RTP/AVP 96\n"
                              "a=rtpmap:96 H264/90000\n"
                              "a=control:trackID=1\n"
                              "a=fmtp:96;profile-level-id=4D0014;packetization-mode=0;sprop-parameter-sets=Z0LAH4iLUCgC3QgAADhAAAr8gBA=,aM44gA==\n"
                              "m=audio 0 RTP/AVP 0\n"
                              "a=rtpmap:0 PCMU/8000\n"
                              "a=control:trackID=2\n"
                              "a=Media_header:MEDIAINFO=494D4B48010100000400010010710110401F000000FA000000000000000000000000000000000000;\n"
                              "a=appversion:1.0\n");

        assert(desc.media_descriptions.size() == 2);
        assert(desc.media_descriptions[0].type == "video");
        assert(desc.media_descriptions[0].codec == "H264");
        assert(desc.media_descriptions[0].time_base == 90000);
        assert(desc.media_descriptions[0].encoded_sps == "Z0LAH4iLUCgC3QgAADhAAAr8gBA=");
        assert(desc.media_descriptions[0].encoded_pps == "aM44gA==");
        assert(desc.media_descriptions[0].control == "trackID=1");

        assert(desc.media_descriptions[1].type == "audio");
        assert(desc.media_descriptions[1].codec == "PCMU");
        assert(desc.media_descriptions[1].time_base == 8000);
        assert(desc.media_descriptions[1].control == "trackID=2");
    }

    {
        // Axis h.264
        auto desc = parse_sdp("v=0\n"
                              "o=- 1216212527554872 1216212527554872 IN IP4 myserver\n"
                              "s=Media Presentation\n"
                              "e=NONE\n"
                              "c=IN IP4 0.0.0.0\n"
                              "b=AS:50064\n"
                              "t=0 0\n"
                              "a=control:rtsp://myserver/axis-media/media.amp?videocodec=h264&resolution=640x480\n"
                              "a=range:npt=0.000000-\n"
                              "m=video 0 RTP/AVP 96\n"
                              "b=AS:50000\n"
                              "a=framerate:30.0\n"
                              "a=transform:1,0,0;0,1,0;0,0,1\n"
                              "a=control:rtsp://myserver/axis-media/media.amp/trackID=1?videocodec=h264&resolution=640x480\n"
                              "a=rtpmap:96 H264/90000\n"
                              "a=fmtp:96 packetization-mode=1; profile-level-id=420029; sprop-parameter-sets=Z0IAKeKQFAe2AtwEBAaQeJEV,aM48gA==\n"
                              "m=audio 0 RTP/AVP 97\n"
                              "b=AS:64\n"
                              "a=control:rtsp://myserver/axis-media/media.amp/trackID=2?videocodec=h264&resolution=640x480\n"
                              "a=rtpmap:97 mpeg4-generic/16000/1\n"
                              "a=fmtp:97 profile-level-id=15; mode=AAC-hbr;config=1408; SizeLength=13; IndexLength=3;IndexDeltaLength=3; Profile=1; bitrate=64000;\n");

        assert(desc.media_descriptions.size() == 2);
        assert(desc.media_descriptions[0].type == "video");
        assert(desc.media_descriptions[0].codec == "H264");
        assert(desc.media_descriptions[0].time_base == 90000);
        assert(desc.media_descriptions[0].encoded_sps == "Z0IAKeKQFAe2AtwEBAaQeJEV");
        assert(desc.media_descriptions[0].encoded_pps == "aM48gA==");
        assert(desc.media_descriptions[0].control == "rtsp://myserver/axis-media/media.amp/trackID=1?videocodec=h264&resolution=640x480");

        assert(desc.media_descriptions[1].type == "audio");
        assert(desc.media_descriptions[1].codec == "mpeg4-generic");
        assert(desc.media_descriptions[1].time_base == 16000);
        assert(desc.media_descriptions[1].control == "rtsp://myserver/axis-media/media.amp/trackID=2?videocodec=h264&resolution=640x480");
   }

    return 0;
}
