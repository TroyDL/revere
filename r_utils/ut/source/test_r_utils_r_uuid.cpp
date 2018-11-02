
#include "test_r_utils_r_uuid.h"
#include "r_utils/r_uuid.h"
#include <numeric>
#include <string.h>

using namespace std;
using namespace r_utils;

REGISTER_TEST_FIXTURE(test_r_utils_r_uuid);

void test_r_utils_r_uuid::setup()
{
}

void test_r_utils_r_uuid::teardown()
{
}

void test_r_utils_r_uuid::test_generate()
{
    uint8_t buffer[16];
    memset(&buffer[0], 0, 16);
    r_uuid::generate(&buffer[0]);
    RTF_ASSERT(std::accumulate(&buffer[0], &buffer[16], 0) != 0);

    RTF_ASSERT(r_uuid::generate().length() == 36);
}

void test_r_utils_r_uuid::test_uuid_to_s()
{
    uint8_t buffer[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    RTF_ASSERT(r_uuid::uuid_to_s(&buffer[0]) == "00010203-0405-0607-0809-0a0b0c0d0e0f");
}

void test_r_utils_r_uuid::test_s_to_uuid()
{
    uint8_t buffer[16];
    memset(&buffer[0], 0, 16);
    r_uuid::s_to_uuid("e158d3a3-bacd-45cb-92ea-4db749fe026c", buffer);
    RTF_ASSERT(buffer[0] == 0xe1);
    RTF_ASSERT(buffer[1] == 0x58);
    RTF_ASSERT(buffer[2] == 0xd3);
    RTF_ASSERT(buffer[3] == 0xa3);
    RTF_ASSERT(buffer[4] == 0xba);
    RTF_ASSERT(buffer[5] == 0xcd);
    RTF_ASSERT(buffer[6] == 0x45);
    RTF_ASSERT(buffer[7] == 0xcb);
    RTF_ASSERT(buffer[8] == 0x92);
    RTF_ASSERT(buffer[9] == 0xea);
    RTF_ASSERT(buffer[10] == 0x4d);
    RTF_ASSERT(buffer[11] == 0xb7);
    RTF_ASSERT(buffer[12] == 0x49);
    RTF_ASSERT(buffer[13] == 0xfe);
    RTF_ASSERT(buffer[14] == 0x02);
    RTF_ASSERT(buffer[15] == 0x6c);    
}
