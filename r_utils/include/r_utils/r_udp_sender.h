
#ifndef r_utils_r_udp_sender_h
#define r_utils_r_udp_sender_h

#include "r_utils/r_string_utils.h"
#include "r_utils/r_socket_address.h"
#include <vector>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>

typedef int SOCKET;

namespace r_utils
{

class r_udp_sender final
{
public:
    r_udp_sender() = delete;
    r_udp_sender( const r_udp_sender& ) = delete;
    r_udp_sender( r_udp_sender&& ) = delete;
    r_udp_sender( const std::string& targetIP,
                  int targetPort,
                  const std::string& localInterfaceIP = "",
                  int localPort = 0 );

    ~r_udp_sender() noexcept;

    r_udp_sender& operator = ( const r_udp_sender& ) = delete;
    r_udp_sender& operator = ( r_udp_sender&& ) = delete;

    void aim( const std::string& targetIP, int targetPort );

    int32_t send( void* buffer, size_t length );

    virtual size_t get_send_buffer_size();

    virtual void set_send_buffer_size(size_t size);

    SOCKET get_socket_fd() { return _sok; }

private:
    void _configure();
    void _close() throw();

    SOCKET _sok;
    r_socket_address _addr;
    std::string _localInterfaceIP;
    int _localPort;
};

}

#endif
