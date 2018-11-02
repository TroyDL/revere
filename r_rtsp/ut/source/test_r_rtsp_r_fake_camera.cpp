
#include "test_r_rtsp_r_fake_camera.h"
#include "r_rtsp/r_fake_camera.h"
#include "r_rtsp/r_rtsp_server.h"
#include "r_rtsp/r_client_connection.h"
#include "r_rtsp/r_client_request.h"
#include "r_av/r_locky.h"
#include <thread>
#include <unistd.h>

using namespace std;
using namespace r_utils;
using namespace r_rtsp;

REGISTER_TEST_FIXTURE(test_r_rtsp_r_fake_camera);

void test_r_rtsp_r_fake_camera::setup()
{
    r_av::r_locky::register_ffmpeg();
}

void test_r_rtsp_r_fake_camera::teardown()
{
    r_av::r_locky::unregister_ffmpeg();
}

void test_r_rtsp_r_fake_camera::test_start_stop()
{
    int port = RTF_NEXT_PORT();

    printf("PORT = %d\n",port);
    fflush(stdout);

    r_fake_camera fc(".", port);
    fc.start();
    usleep(1000000);
    fc.stop();
}
