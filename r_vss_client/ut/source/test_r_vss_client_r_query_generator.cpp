
#include "test_r_vss_client_r_query_generator.h"
#include "r_vss_client/r_query_generator.h"
#include "r_utils/r_time_utils.h"
#include "r_utils/r_string.h"

using namespace r_vss_client;
using namespace r_utils;
using namespace std::chrono;
using namespace std;

REGISTER_TEST_FIXTURE(_test_r_vss_client_r_query_generator);

void _test_r_vss_client_r_query_generator::setup()
{
}

void _test_r_vss_client_r_query_generator::teardown()
{
}

void _test_r_vss_client_r_query_generator::test_basic()
{
    auto start = system_clock::from_time_t(1514793600);
    auto end = start + hours(1);

    r_query_generator g("8dcf6919-f0d3-4b99-ad09-2bbeb45ce638",
                        "video",
                        60000,
                        start,
                        end);

    bool first = true;

    size_t numQ = 0;

    while( auto q = g.next() )
    {
        if(first)
            RTF_ASSERT(r_string::contains(q.value(), "previous_playable=true"));
        else RTF_ASSERT(r_string::contains(q.value(), "previous_playable=false"));

        ++numQ;

        first = false;
    }

    RTF_ASSERT(numQ == 60);
}
