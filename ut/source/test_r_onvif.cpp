
#include "test_r_onvif.h"
#include "r_onvif/r_onvif_session.h"
#include "r_utils/r_string_utils.h"
#include <string.h>
#include <map>

using namespace std;
using namespace r_onvif;
using namespace r_utils;

REGISTER_TEST_FIXTURE(test_r_onvif);

void test_r_onvif::setup()
{
}

void test_r_onvif::teardown()
{
}

void test_r_onvif::test_r_onvif_session_basic()
{
    struct keys
    {
        string username;
        string password;
    };

    map<string, keys> key_map;

    {
        // reolink rlc810a
        keys k;
        k.username = "admin";
        k.password = "emperor1";
        key_map.insert(make_pair("urn:uuid:2419d68a-2dd2-21b2-a205-ec:71:db:16:b1:12",k));
    }

    {
        // axis m-3075
        keys k;
        k.username = "root";
        k.password = "emperor1";
        key_map.insert(make_pair("urn:uuid:e58f6613-b3bf-4aa0-90da-250d77bb2fda",k));
    }

    {
        // Dahua 1A404XBNR
        keys k;
        k.username = "admin";
        k.password = "emperor1";
        key_map.insert(make_pair("uuid:b1fd91c3-9f24-b6e7-2a5c-b878404a9f24",k));
    }

    {
        // Hanhua Wisenet QNO-6082R 
        keys k;
        k.username = "admin";
        k.password = "nX760x2904SY";
        key_map.insert(make_pair("urn:uuid:5a4d5857-3656-3452-3330-3032304d4100",k));
    }

    r_onvif_session session;
    auto discovered = session.discover();

    bool foundSomething = false;
    for(auto& di : discovered)
    {
        printf("camera_name=%s\n",di.camera_name.c_str());
        printf("address=%s\n",di.address.c_str());
        printf("xaddrs=%s\n",di.xaddrs.c_str());
        printf("ipv4=%s\n",di.ipv4.c_str());
        fflush(stdout);
        auto keys = key_map.at(di.address);

        auto rdi = session.get_rtsp_url(di, keys.username, keys.password);

        if(!rdi.is_null())
        {
            foundSomething = true;
            RTF_ASSERT(rdi.value().rtsp_url.find("rtsp://") != string::npos);
            //printf("cameraName=%s\n",rdi.value().camera_name.c_str());
            //printf("address=%s\n",rdi.value().address.c_str());
            //printf("serial_number=%s\n",rdi.value().serial_number.c_str());
            //printf("model_number=%s\n",rdi.value().model_number.c_str());
            //printf("firmware_version=%s\n",rdi.value().firmware_version.c_str());
            //printf("manufacturer=%s\n",rdi.value().manufacturer.c_str());
            //printf("hardware_id=%s\n",rdi.value().hardware_id.c_str());
            printf("rtsp_url=%s\n",rdi.value().rtsp_url.c_str());
        }
    }

    RTF_ASSERT(foundSomething);
}
