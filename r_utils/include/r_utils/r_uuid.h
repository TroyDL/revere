
#ifndef r_utils_r_uuid_h
#define r_utils_r_uuid_h

#include <string>
#include <vector>
#ifdef IS_LINUX
#include <uuid/uuid.h>
#endif
#ifdef IS_WINDOWS
#include <Rpc.h>
#endif
namespace r_utils
{

namespace r_uuid
{

void generate(uint8_t* uuid);
std::string generate();
std::string uuid_to_s(const uint8_t* uuid);
void s_to_uuid(const std::string& uuidS, uint8_t* uuid);

}

}

#endif
