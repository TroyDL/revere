
#include "r_utils/r_uuid.h"
#include "r_utils/r_exception.h"
#include <string.h>

using namespace r_utils;
using namespace std;

void r_utils::r_uuid::generate(uint8_t* uuid)
{
    uuid_generate_random(uuid);
}

string r_utils::r_uuid::generate()
{
    uint8_t uuid[16];
    generate(&uuid[0]);
    return uuid_to_s(&uuid[0]);
}

string r_utils::r_uuid::uuid_to_s(const uint8_t* uuid)
{
    char str[37];
    uuid_unparse(uuid, str);
    return str;
}

void r_utils::r_uuid::s_to_uuid(const string& uuidS, uint8_t* uuid)
{
    if(uuid_parse(uuidS.c_str(), uuid) != 0)
        R_STHROW(r_invalid_argument_exception, ("Unable to parse uuid string."));
}
