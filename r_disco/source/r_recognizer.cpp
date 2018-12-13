
#include "r_disco/r_recognizer.h"

using namespace r_disco;
using namespace r_utils;
using namespace std;

r_recognizer::r_recognizer() :
    _deviceInfoAgents()
{
}

r_recognizer::~r_recognizer() noexcept
{
}

r_nullable<r_device_info> r_recognizer::get_device_info(const string& ssdpNotifyMessage)
{
    r_nullable<r_device_info> output;

    for(auto dia : _deviceInfoAgents)
    {
        output = dia->get_device_info(ssdpNotifyMessage);
        if(!output.is_null())
            return output;
    }

    return output;
}
