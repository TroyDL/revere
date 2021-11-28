#ifndef r_utils_r_md5_h
#define r_utils_r_md5_h

#include <cstdint>
#include <string>

namespace r_utils
{

class r_md5 final
{
public:
    r_md5();
    ~r_md5();

    void update(const uint8_t* src, size_t size );

    void finalize();

    void get(uint8_t* output);
    std::string get_as_string();
    std::string get_as_uuid();

private:
    const uint8_t* _body(const uint8_t* data, size_t size);

    uint32_t _lo, _hi;
    uint32_t _a, _b, _c, _d;
    unsigned char _buffer[64];
    uint32_t _block[16];
    bool _finalized;
    uint8_t _result[16];
};

}

#endif
