
#include "test_r_vss_client_r_media_parser.h"
#include "r_vss_client/r_media_parser.h"
#include "r_utils/r_time_utils.h"

#include "query.cpp"

using namespace r_vss_client;
using namespace r_utils;
using namespace std::chrono;
using namespace std;

REGISTER_TEST_FIXTURE(test_r_vss_client_r_media_parser);

void test_r_vss_client_r_media_parser::setup()
{
}

void test_r_vss_client_r_media_parser::teardown()
{
}

void test_r_vss_client_r_media_parser::test_basic()
{
    // query and query_len
    std::vector<uint8_t> media(query_len);
    memcpy(&media[0], query, query_len);
    r_media_parser parser(media);
    parser.begin();
    RTF_ASSERT(parser.valid());
    bool firstFrame = true;
    for(; parser.valid(); parser.next())
    {
        system_clock::time_point time;
        size_t frameSize;
        const uint8_t* frame;
        uint32_t flags;
        parser.current_data(time, frameSize, frame, flags);
        if(firstFrame)
            RTF_ASSERT(flags != 0);
        firstFrame=false;
    }
    RTF_ASSERT(parser.valid() == false);
}
