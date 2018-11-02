
#include "framework.h"
#include "test_r_utils_r_server_threaded.h"
#include "r_utils/r_server_threaded.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <thread>
#include <mutex>
#include <condition_variable>

using namespace std;
using namespace r_utils;

REGISTER_TEST_FIXTURE(test_r_utils_r_server_threaded);

void test_r_utils_r_server_threaded::setup()
{
}

void test_r_utils_r_server_threaded::teardown()
{
}

void test_r_utils_r_server_threaded::test_basic()
{
    int port = RTF_NEXT_PORT();

    r_server_threaded<r_socket> s( port, [](r_buffered_socket<r_socket>& conn){
        uint32_t inv;
        conn.recv( &inv, 4 );
        ++inv;
        conn.send( &inv, 4 );

        conn.close();
    } );

    thread t([&](){
        s.start();
    });

    try
    {
        usleep(1000000);
        r_socket socket;

        RTF_ASSERT_NO_THROW( socket.connect( "127.0.0.1", port ) );

        uint32_t val = 41;
        socket.send( &val, 4 );
        socket.recv( &val, 4 );

        RTF_ASSERT( val == 42 );

        socket.close();

        s.stop();
        t.join();
    }
    catch(r_exception& ex)
    {
        printf("%s\n",ex.what());
    }
    catch(exception& ex)
    {
        printf("%s\n",ex.what());
    }
}