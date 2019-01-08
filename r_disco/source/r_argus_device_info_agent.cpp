
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

    try
    {
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
                auto startSL = responseBody.find("<StreamLocation>");
                auto endSL = responseBody.find("</StreamLocation>");

                //<StreamLocation>rtsp://192.168.1.78/stream1</StreamLocation>
                auto rtsp = responseBody.substr(startSL + 16, (endSL - (startSL + 16)));

                auto startUSN = ssdpNotifyMessage.find("USN: ");
                if(startUSN == string::npos)
                    R_THROW(("USN not found"));

                auto uuidStart = startUSN + 5; //+5 skip past "USN: "

                r_device_info di;
                di.unique_id = ssdpNotifyMessage.substr(uuidStart, 36); //+5 start past "uuid:"
                di.rtsp_url = rtsp;
                di.type = "video";
                di.transport_pref = "tcp";

                output.set_value(di);
            }
        }
    }
    catch(exception& ex)
    {
        R_LOG_NOTICE("%s", ex.what());
    }
    
    return output;
}
