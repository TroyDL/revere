
#ifndef r_utils_r_udp_receiver_h
#define r_utils_r_udp_receiver_h

#include "r_utils/r_socket.h"
#include "r_utils/r_string_utils.h"
#include "r_utils/r_socket_address.h"
#include <memory>
#include <list>

namespace r_utils
{

const int DEFAULT_UDP_RECV_BUF_SIZE = 0;

class r_udp_receiver final
{
public:
    r_udp_receiver( int destinationPort = 0,
                    int recvBufferSize = DEFAULT_UDP_RECV_BUF_SIZE,
                    const std::string& destinationAddress = ip4_addr_any );

    r_udp_receiver( const r_udp_receiver& ) = delete;
    r_udp_receiver( r_udp_receiver&& obj ) = delete;

    ~r_udp_receiver() noexcept {}

    r_udp_receiver& operator = ( const r_udp_receiver& ) = delete;
    r_udp_receiver& operator = ( r_udp_receiver&& obj ) = delete;

    void connect( const std::string& ipAddress, int port );

    virtual bool receive( int& port, std::vector<uint8_t>& buffer );

    virtual bool receive( int& port, std::vector<uint8_t>& buffer, int waitMillis );

    virtual bool non_blocking_receive( int& port, std::vector<uint8_t>& buffer );

    virtual bool raw_receive( int& port, std::vector<uint8_t>& buffer );

    virtual void shutdown();

    virtual void close();

    virtual void associate( std::shared_ptr<r_udp_receiver> receiver );

    virtual void clear_associations();

    virtual int get_bound_port();

    virtual size_t get_recv_buffer_size();

    virtual void set_recv_buffer_size( size_t size );

    SOCKET get_socket_fd() { return _sok; }

private:

    bool _receive( int& port, std::vector<uint8_t>& buffer, bool block, int waitMillis );

    SOCKET _sok;
    r_socket_address _addr;
    std::list<std::shared_ptr<r_udp_receiver> > _associatedReceivers;
};

}

#endif
