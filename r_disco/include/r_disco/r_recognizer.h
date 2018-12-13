
#ifndef r_discovery_r_recognizer_h
#define r_discovery_r_recognizer_h

#include "r_disco/r_device_info.h"
#include "r_disco/interfaces/r_query_device_info.h"
#include "r_utils/r_nullable.h"
#include <memory>
#include <vector>

namespace r_disco
{

class r_recognizer : public r_query_device_info
{
public:
    r_recognizer();
    r_recognizer(const r_recognizer&) = delete;
    r_recognizer(r_recognizer&&) = delete;
    ~r_recognizer() noexcept;

    r_recognizer operator=(const r_recognizer&) = delete;
    r_recognizer operator=(r_recognizer&&) = delete;

    virtual r_utils::r_nullable<r_device_info> get_device_info(const std::string& ssdpNotifyMessage);

    void register_device_info_agent(std::shared_ptr<r_query_device_info> dia) {_deviceInfoAgents.push_back(dia);}

private:
    std::vector<std::shared_ptr<r_query_device_info>> _deviceInfoAgents;
};

}

#endif
