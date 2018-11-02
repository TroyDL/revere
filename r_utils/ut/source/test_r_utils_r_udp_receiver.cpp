
#include "test_r_utils_r_udp_receiver.h"
#include "r_utils/r_udp_receiver.h"
#include "r_utils/r_udp_sender.h"
#include "r_utils/r_byte_ptr.h"
#include <thread>
#include <unistd.h>

using namespace std;
using namespace r_utils;

REGISTER_TEST_FIXTURE(test_r_utils_r_udp_receiver);

void test_r_utils_r_udp_receiver::setup()
{
    _recvAddress.clear();
}

void test_r_utils_r_udp_receiver::teardown()
{
}

void test_r_utils_r_udp_receiver::test_send()
{
    const char addrs[][32] = { "127.0.0.1", "::1" };

    for( int ii=0; ii<2; ++ii )
    {
        _recvAddress = r_socket_address::get_address_family( addrs[ii] ) == AF_INET ? ip4_addr_any : ip6_addr_any;
        _val = 0;

        thread t([&](){
            vector<uint8_t> buffer;

            r_udp_receiver server( 8484, DEFAULT_UDP_RECV_BUF_SIZE, _recvAddress );

            int destinationPort = 0;

            server.receive( destinationPort, buffer );

            r_byte_ptr_rw reader(&buffer[0], buffer.size());

            _val = reader.consume<int>();
        });
        t.detach();

        usleep( 500000 );

        vector<uint8_t> buffer(4);

        r_byte_ptr_rw writer(&buffer[0], 4);

        writer.write<int>(42);

        r_udp_sender client( addrs[ii], 8484 );

        client.send( &buffer[0], buffer.size() );

        usleep( 250000 );

        RTF_ASSERT( _val == 42 );
    }
}

void test_r_utils_r_udp_receiver::test_send_multicast()
{
}

void test_r_utils_r_udp_receiver::test_associated_send()
{
    const char addrs[][32] = { "127.0.0.1", "::1" };

    for( int ii=0; ii<2; ++ii )
    {
        _recvAddress = r_socket_address::get_address_family( addrs[ii] ) == AF_INET ? ip4_addr_any : ip6_addr_any;
        _val = 0;

        thread t([&](){
            vector<uint8_t> buffer;

            // Bind to a specific interface
            shared_ptr<r_udp_receiver> serverA = make_shared<r_udp_receiver>( 8787, DEFAULT_UDP_RECV_BUF_SIZE, _recvAddress );
            shared_ptr<r_udp_receiver> serverB = make_shared<r_udp_receiver>( 8888, DEFAULT_UDP_RECV_BUF_SIZE, _recvAddress );

            serverA->associate( serverB );

            int destinationPort = 0;

            serverA->receive( destinationPort, buffer );

            RTF_ASSERT( destinationPort == 8787 );

            r_byte_ptr_rw reader(&buffer[0], buffer.size());

            _val += reader.consume<int>();

            serverA->receive( destinationPort, buffer );

            reader = r_byte_ptr_rw(&buffer[0], buffer.size());

            _val += reader.consume<int>();

            RTF_ASSERT(destinationPort == 8888);
        });
        t.detach();

        usleep(500000);

        vector<uint8_t> buffer(4);
        r_byte_ptr_rw writer(&buffer[0], buffer.size());
        writer.write<int>( 42 );

        r_udp_sender clientA( addrs[ii], 8787 );

        clientA.send( &buffer[0], buffer.size() );

        r_udp_sender clientB( addrs[ii], 8888 );

        clientB.send( &buffer[0], buffer.size() );

        usleep(100000);

        RTF_ASSERT( _val == 84 );
    }
}

void test_r_utils_r_udp_receiver::test_get_set_recv_buffer_size()
{
    {
        r_udp_receiver r;
        RTF_ASSERT_NO_THROW( r.get_recv_buffer_size() );
        RTF_ASSERT_NO_THROW( r.set_recv_buffer_size(256 * 1024) );
    }
    {
        r_udp_receiver r;
        size_t sizeOrig, sizeFinal;
        RTF_ASSERT_NO_THROW( sizeOrig = r.get_recv_buffer_size() );
        RTF_ASSERT_NO_THROW( r.set_recv_buffer_size( sizeOrig * 2 ) );
        RTF_ASSERT_NO_THROW( sizeFinal = r.get_recv_buffer_size() );
        RTF_ASSERT( sizeFinal > sizeOrig );
    }
}

void test_r_utils_r_udp_receiver::test_shutdown_close_while_blocked()
{
    const char addrs[][32] = { "127.0.0.1", "::1" };

    for( int ii=0; ii<2; ++ii )
    {
        _recvAddress = r_socket_address::get_address_family( addrs[ii] ) == AF_INET ? ip4_addr_any : ip6_addr_any;
        _val = 0;

        _receiver = make_shared<r_udp_receiver>( 8989, DEFAULT_UDP_RECV_BUF_SIZE, addrs[ii] );

        thread t([&](){
            vector<uint8_t> buffer(1600);

            int destinationPort = 0;

            _receiver->receive( destinationPort, buffer );

            _val = 1234;
        });
        t.detach();

        usleep(1000000);

        _receiver->shutdown();
        _receiver->close();

        usleep( 100000 );

        RTF_ASSERT( _val == 1234 );
    }
}

void test_r_utils_r_udp_receiver::test_raw_recv()
{
    const char addrs[][32] = { "127.0.0.1", "::1" };

    for( int ii=0; ii<2; ++ii )
    {
        _recvAddress = r_socket_address::get_address_family( addrs[ii] ) == AF_INET ? ip4_addr_any : ip6_addr_any;
        _val = 0;

        thread t([&](){
            vector<uint8_t> buffer;

            r_udp_receiver server( 9090, DEFAULT_UDP_RECV_BUF_SIZE, _recvAddress );

            int destinationPort = 0;

            server.raw_receive( destinationPort, buffer );

            r_byte_ptr_rw reader(&buffer[0], buffer.size());

            _val = reader.consume<int>();
        });
        t.detach();

        usleep( 500000 );

        vector<uint8_t> buffer(4);
        r_byte_ptr_rw writer(&buffer[0], buffer.size());
        writer.write<int>( 24 );

        r_udp_sender client( addrs[ii], 9090 );

        client.send( &buffer[0], buffer.size() );

        usleep(100000);

        RTF_ASSERT( _val == 24 );
    }
}

void test_r_utils_r_udp_receiver::test_shutdown_close_while_blocked_in_raw_recv()
{
    const char addrs[][32] = { "127.0.0.1", "::1" };

    for( int ii=0; ii<2; ++ii )
    {
        _recvAddress = r_socket_address::get_address_family( addrs[ii] ) == AF_INET ? ip4_addr_any : ip6_addr_any;
        _val = 0;

        _receiver = make_shared<r_udp_receiver>( 9191, DEFAULT_UDP_RECV_BUF_SIZE, addrs[ii] );

        thread t([&](){
            vector<uint8_t> buffer(1600);

            int destinationPort = 0;

            _receiver->raw_receive( destinationPort, buffer );

            _val = 4321;
        });
        t.detach();

        usleep( 500000 );

        _receiver->close();

        usleep( 1000000 );

        RTF_ASSERT( _val == 4321 );
    }
}
