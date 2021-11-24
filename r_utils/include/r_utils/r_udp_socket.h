
#ifndef r_utils_r_timer_h
#define r_utils_r_timer_h

#include "r_utils/r_socket_address.h"
#include "r_utils/r_socket.h"

namespace r_utils
{

class r_udp_socket
{
public:
    r_udp_socket();
    r_udp_socket(const r_udp_socket&) = delete;
    r_udp_socket(r_udp_socket&& obj) noexcept;
    ~r_udp_socket() noexcept;

    r_udp_socket& operator = (const r_udp_socket& obj);
    r_udp_socket& operator = (r_udp_socket&& obj) noexcept;

    int sendto(const uint8_t* buffer, size_t size, r_socket_address& address);
    int recvfrom(uint8_t* buffer, size_t size, r_socket_address& address);

    int fd() const {return _sok;}

private:
    void _clear() noexcept;
    SOK _sok;
};

}

#endif
