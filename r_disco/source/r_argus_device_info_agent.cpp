
#include "r_disco/r_argus_device_info_agent.h"
#include "r_utils/r_string_utils.h"
#include "r_http/r_utils.h"
#include "r_http/r_client_request.h"
#include "r_http/r_client_response.h"
#include "r_utils/r_socket.h"

using namespace r_disco;
using namespace r_utils;
using namespace r_http;
using namespace std;

r_argus_device_info_agent::r_argus_device_info_agent()
{
}

r_argus_device_info_agent::~r_argus_device_info_agent() noexcept
{
}

r_nullable<r_device_info> r_argus_device_info_agent::get_device_info(const string& ssdpNotifyMessage)
{
    r_nullable<r_device_info> output;

    if(r_string_utils::contains(r_string_utils::to_lower(ssdpNotifyMessage), "argus"))
    {
        auto locStart = r_string_utils::to_lower(ssdpNotifyMessage).find("location:");
        if(locStart != string::npos)
        {
            auto locNL = ssdpNotifyMessage.find('\n', locStart + 10);

            if(ssdpNotifyMessage[locNL-1] == '\r')
                --locNL;

            auto locURL = ssdpNotifyMessage.substr(locStart + 10, (locNL-(locStart+10)));

            string host, protocol, uri;
            int port;
            parse_url_parts(locURL, host, port, protocol, uri);

            if(r_string_utils::contains(protocol, "https"))
                R_THROW(("https unsupported protocol."));

            r_socket sok;
            sok.connect(host, port);

            r_client_request req(host, port);
            req.set_accept_type("application/xml");
            req.set_uri(uri);

            req.write_request(sok);

            r_client_response res;
            res.read_response(sok);

            auto responseBody = res.get_body_as_string();
            auto startUDN = responseBody.find("<UDN>uuid:");
            auto endUDN = responseBody.find("</UDN>");
            auto startSL = responseBody.find("<StreamLocation>");
            auto endSL = responseBody.find("</StreamLocation>");

            //<UDN>uuid:0800C007-8000-0001-0802-788602E20002</UDN>
            //<StreamLocation>rtsp://192.168.1.78/stream1</StreamLocation>
            auto udn = responseBody.substr(startUDN + 10, (endUDN - (startUDN + 10)));
            auto rtsp = responseBody.substr(startSL + 16, (endSL - (startSL + 16)));

            r_device_info di;
            di.unique_id = udn;
            di.rtsp_url = rtsp;
            di.type = "camera";

            output.set_value(di);
        }
    }

    return output;
}
