
#ifndef r_utils_r_sha_256_h
#define r_utils_r_sha_256_h

#include "r_utils/r_string_utils.h"
#include "openssl/sha.h"

namespace r_utils
{

class r_sha_256 final
{
public:
    r_sha_256();
    r_sha_256(const r_sha_256&) = delete;
    r_sha_256(r_sha_256&&) = delete;
    ~r_sha_256() noexcept;

    r_sha_256& operator=(const r_sha_256&) = delete;
    r_sha_256& operator=(r_sha_256&&) = delete;

    void update( const uint8_t* src, size_t size );
    void finalize();

    // returns 32 byte sha256
    void get( uint8_t* output );
    std::string get_as_string();

private:
    uint8_t _digest[32];
    bool _finalized;
    SHA256_CTX _sha256;
};

}

#endif
