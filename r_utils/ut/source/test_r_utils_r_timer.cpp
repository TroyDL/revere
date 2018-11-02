
#include "test_r_utils_r_timer.h"
#include "r_utils/r_timer.h"
#include <unistd.h>

using namespace std;
using namespace r_utils;

REGISTER_TEST_FIXTURE(test_r_utils_r_timer);

void test_r_utils_r_timer::setup()
{
}

void test_r_utils_r_timer::teardown()
{
}

void test_r_utils_r_timer::test_repeating()
{
    int counter = 0;
    r_timer t( 250, [&](){ counter++; } );
    t.start();
    sleep( 1 );
    t.stop();
    RTF_ASSERT( counter >= 2 );
}
