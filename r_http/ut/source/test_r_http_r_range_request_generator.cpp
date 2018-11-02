
#include "test_r_http_r_range_request_generator.h"
#include "r_http/r_range_request_generator.h"
#include "r_utils/r_string.h"

using namespace std;
using namespace r_utils;
using namespace r_http;

REGISTER_TEST_FIXTURE(test_r_http_r_range_request_generator);

void test_r_http_r_range_request_generator::setup()
{
}

void test_r_http_r_range_request_generator::teardown()
{
}

void test_r_http_r_range_request_generator::test_basic()
{
    r_range_request_generator rrg("127.0.0.1", 8080, "/foo", 15, 4, 0);

    //range = bytes=0-3
    //range = bytes=4-7
    //range = bytes=8-11
    //range = bytes=12-14

    int vals[] = {0,3,4,7,8,11,12,14};
    int vi=0;

    while(rrg.valid())
    {
        auto req = rrg.get();
        auto uri = req._uri.get_full_raw_uri();
        auto outerParts = r_string::split(req._headerParts["range"], "=");
        auto innerParts = r_string::split(outerParts[1], "-");
        RTF_ASSERT(vals[vi] == r_string::s_to_int(innerParts[0]));
        ++vi;
        RTF_ASSERT(vals[vi] == r_string::s_to_int(innerParts[1]));
        ++vi;
        rrg.next();
    }
}

void test_r_http_r_range_request_generator::test_even()
{
    r_range_request_generator rrg("127.0.0.1", 8080, "/foo", 10, 2, 0);

    //range = bytes=0-1
    //range = bytes=2-3
    //range = bytes=4-5
    //range = bytes=6-7
    //range = bytes=8-9

    int vals[] = {0,1,2,3,4,5,6,7,8,9};
    int vi=0;

    while(rrg.valid())
    {
        auto req = rrg.get();
        auto uri = req._uri.get_full_raw_uri();
        auto outerParts = r_string::split(req._headerParts["range"], "=");
        auto innerParts = r_string::split(outerParts[1], "-");
        RTF_ASSERT(vals[vi] == r_string::s_to_int(innerParts[0]));
        ++vi;
        RTF_ASSERT(vals[vi] == r_string::s_to_int(innerParts[1]));
        ++vi;
        rrg.next();
    }
}
