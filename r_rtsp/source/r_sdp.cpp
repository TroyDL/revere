
#include "r_rtsp/r_sdp.h"
#include <string.h>

using namespace r_rtsp;
using namespace r_utils;
using namespace std;

static r_connection _parse_connection(const std::string& c)
{
    auto connParts = r_string_utils::split(c, " ");
    if(connParts.size() < 3)
        R_THROW(("Unable to parse connection line in sdp."));
    r_connection cd;
    cd.nettype = connParts[0];
    cd.address_type = connParts[1];
    cd.connection_address = r_string_utils::strip(connParts[2]);
    return cd;
}

static r_bandwidth _parse_bandwidth(const std::string& b)
{
    auto bwParts = r_string_utils::split(b, ":");
    if(bwParts.size() != 2)
        R_THROW(("Unable to parse bandwidth."));
    r_bandwidth bw;
    bw.type = bwParts[0];
    bw.value = r_string_utils::strip(bwParts[1]);
    return bw;
}

static r_encryption _parse_encryption(const std::string& k)
{
    r_encryption enc;
    if(r_string_utils::contains(k, ":"))
    {
        auto keyParts = r_string_utils::split(k, ":");
        if(keyParts.size() != 2)
            R_THROW(("Unable to parse encryption line in sdp."));
        enc.method = keyParts[0];
        enc.key = r_string_utils::strip(keyParts[1]);
    }
    else enc.method = k;
    return enc;
}

static void _parse_media_description(r_media_description& md, const std::string& m)
{
    auto mparts = r_string_utils::split(m, " ");
    if(mparts.size() < 4)
        R_THROW(("Unable to parse m= line in sdp"));
    md.type = mparts[0];
    md.port = r_string_utils::s_to_int(mparts[1]);
    md.proto = mparts[2];
    md.fmt = r_string_utils::strip(mparts[3]);
}

r_session_description r_rtsp::parse_sdp(const std::string& sdp)
{
    r_session_description sdesc;

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
            sdesc.protocol_version = r_string_utils::strip(lv);
        else if(lt == "o=")
        {
            auto originParts = r_string_utils::split(lv, " ");
            size_t numParts = originParts.size();

            if(numParts>0)
                sdesc.origin.username = r_string_utils::strip(originParts[0]);
            if(numParts>1)
                sdesc.origin.session_id = r_string_utils::strip(originParts[1]);
            if(numParts>2)
                sdesc.origin.session_version = r_string_utils::strip(originParts[2]);
            if(numParts>3)
                sdesc.origin.nettype = r_string_utils::strip(originParts[3]);
            if(numParts>4)
                sdesc.origin.addrtype = r_string_utils::strip(originParts[4]);
            if(numParts>5)
                sdesc.origin.unicast_address = r_string_utils::strip(originParts[5]);
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
        r_media_description mdesc;

        for(auto l : md)
        {
            auto lt = l.substr(0, 2);
            auto lv = l.substr(2);

            if(lt == "m=")
                _parse_media_description(mdesc, lv);
            else if(lt == "i=")
                mdesc.info = r_string_utils::strip(lv);
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
                            mdesc.control = r_string_utils::strip(lv.substr(lv.find("control:") + 8));
                    }
                }
                else mdesc.attr.property_attributes.push_back(lv);
            }
        }

        sdesc.media_descriptions.push_back(mdesc);
    }

    return sdesc;
}

std::vector<uint8_t> r_rtsp::create_annexb_extradata(const std::string& encodedSPS, const std::string& encodedPPS)
{
    auto sps = r_string_utils::from_base64(encodedSPS);
    auto pps = r_string_utils::from_base64(encodedPPS);
    vector<uint8_t> output(4 + sps.size() + 4 + pps.size());
    uint8_t startCode[] = {0, 0, 0, 1};
    uint8_t* dst = &output[0];
    memcpy(dst, startCode, 4);
    dst+=4;
    memcpy(dst, &sps[0], sps.size());
    dst+=sps.size();
    memcpy(dst, startCode, 4);
    dst+=4;
    memcpy(dst, &pps[0], pps.size());
    return output;
}