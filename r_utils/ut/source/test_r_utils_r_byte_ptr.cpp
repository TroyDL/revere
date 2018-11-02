
#include "test_r_utils_r_byte_ptr.h"
#include "r_utils/r_byte_ptr.h"
#include "r_utils/r_sha_256.h"

using namespace std;
using namespace r_utils;

REGISTER_TEST_FIXTURE(test_r_utils_r_byte_ptr_rw);

static uint8_t test_buffer[]= {
    0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
};

static const size_t test_buffer_len = sizeof(test_buffer)/sizeof(test_buffer[0]);

void test_r_utils_r_byte_ptr_rw::setup()
{
}

void test_r_utils_r_byte_ptr_rw::teardown()
{
}

void test_r_utils_r_byte_ptr_rw::test_constructor()
{
    {
        // Default ctor
        RTF_ASSERT_NO_THROW(r_byte_ptr_rw());
        r_byte_ptr_rw p;
        RTF_ASSERT( p.length() == 0 );
        RTF_ASSERT( p.get_ptr() == 0 );
        RTF_ASSERT( p.original_ptr() == 0 );
    }
    {
        RTF_ASSERT_NO_THROW( r_byte_ptr_rw(test_buffer, test_buffer_len) );
        r_byte_ptr_rw p(test_buffer, test_buffer_len);
        RTF_ASSERT( p.length() == test_buffer_len );
        RTF_ASSERT( p.get_ptr() == test_buffer );
        RTF_ASSERT( p.original_ptr() == test_buffer );
    }
    {
        // Copy ctor
        r_byte_ptr_rw p(test_buffer, test_buffer_len);
        RTF_ASSERT_NO_THROW( ++p );
        RTF_ASSERT_NO_THROW( r_byte_ptr_rw(p) );
        r_byte_ptr_rw copy(p);
        RTF_ASSERT( copy.length() == test_buffer_len );
        RTF_ASSERT( copy.get_ptr() == test_buffer + 1 );
        RTF_ASSERT( copy.original_ptr() == test_buffer );
    }
}

void test_r_utils_r_byte_ptr_rw::test_get_set_ptr()
{
    {
        r_byte_ptr_rw p;
        RTF_ASSERT_NO_THROW( p.set_ptr(test_buffer, test_buffer_len) );
        RTF_ASSERT( p.length() == test_buffer_len );
        RTF_ASSERT( p.get_ptr() == test_buffer );
        RTF_ASSERT( p.original_ptr() == test_buffer );
    }
    {
        // Test the reference returned by set_ptr()
        r_byte_ptr_rw p;
        RTF_ASSERT_NO_THROW( p.set_ptr(test_buffer, test_buffer_len)++ );
        RTF_ASSERT( p.length() == test_buffer_len );
        RTF_ASSERT( p.get_ptr() == test_buffer + 1 );
        RTF_ASSERT( p.original_ptr() == test_buffer );
    }
}

void test_r_utils_r_byte_ptr_rw::test_inc_dec()
{
    {
        // Test prefix/postfix
        r_byte_ptr_rw p(test_buffer, test_buffer_len);
        RTF_ASSERT_NO_THROW( ++p );
        RTF_ASSERT( p.length() == test_buffer_len );
        RTF_ASSERT( p.get_ptr() == test_buffer + 1 );
        RTF_ASSERT( p.original_ptr() == test_buffer );

        RTF_ASSERT_NO_THROW( p++ );
        RTF_ASSERT( p.length() == test_buffer_len );
        RTF_ASSERT( p.get_ptr() == test_buffer + 2 );
        RTF_ASSERT( p.original_ptr() == test_buffer );

        RTF_ASSERT_NO_THROW( --p );
        RTF_ASSERT( p.length() == test_buffer_len );
        RTF_ASSERT( p.get_ptr() == test_buffer + 1 );
        RTF_ASSERT( p.original_ptr() == test_buffer );

        RTF_ASSERT_NO_THROW( p-- );
        RTF_ASSERT( p.length() == test_buffer_len );
        RTF_ASSERT( p.get_ptr() == test_buffer );
        RTF_ASSERT( p.original_ptr() == test_buffer );

        // Pointer back to original position.

        // Make sure pointer operations occur in the correct order.
        uint8_t *curPtr = p.get_ptr();
        RTF_ASSERT( ++p == curPtr + 1 );
        curPtr = p.get_ptr();
        RTF_ASSERT( p++ == curPtr );
        curPtr = p.get_ptr();
        RTF_ASSERT( --p == curPtr - 1 );
        curPtr = p.get_ptr();
        RTF_ASSERT( p-- == curPtr );
    }
}

void test_r_utils_r_byte_ptr_rw::test_pointer()
{
    r_byte_ptr_rw p(test_buffer, test_buffer_len);
    for (size_t ii=0; ii<test_buffer_len; ++ii)
    {
        RTF_ASSERT_NO_THROW( *p );
        RTF_ASSERT_NO_THROW( p[ii] );
        RTF_ASSERT( *p == test_buffer[ii] );
        RTF_ASSERT( p[ii] == test_buffer[ii] );

        // Test all sorts of weird pointer usage
        RTF_ASSERT( &p[ii] == &test_buffer[ii] );
        RTF_ASSERT( memcmp(&p[ii], &test_buffer[ii], test_buffer_len-ii) == 0 );
        if (ii < test_buffer_len-1)
        {
            int16_t raw = *(int16_t*)&test_buffer[ii];
            int16_t ptr = *(int16_t*)&p[ii];
            RTF_ASSERT( raw == ptr );
        }

        ++p;
    }
}

void test_r_utils_r_byte_ptr_rw::test_add_sub()
{
    {
        r_byte_ptr_rw p(test_buffer, test_buffer_len);
        //RTF_ASSERT_NO_THROW( p + 3 );
        //RTF_ASSERT_NO_THROW( p - 7 );
        RTF_ASSERT_NO_THROW( p += 5 );
        RTF_ASSERT_NO_THROW( p -= 2 );
    }
    {
        // Test +
        r_byte_ptr_rw p(test_buffer, test_buffer_len);
	//        RTF_ASSERT( p + 3 == test_buffer + 3 );
        RTF_ASSERT( p.length() == test_buffer_len );
        RTF_ASSERT( p.get_ptr() == test_buffer );
        RTF_ASSERT( p.original_ptr() == test_buffer );
    }
    {
        // Test -
        r_byte_ptr_rw p(test_buffer, test_buffer_len);
        //RTF_ASSERT( p - 3 == test_buffer - 3 );
        RTF_ASSERT( p.length() == test_buffer_len );
        RTF_ASSERT( p.get_ptr() == test_buffer );
        RTF_ASSERT( p.original_ptr() == test_buffer );
    }
    {
        // Test +=
        r_byte_ptr_rw p(test_buffer, test_buffer_len);
        RTF_ASSERT_NO_THROW( p += 6 );
        RTF_ASSERT( p.length() == test_buffer_len );
        RTF_ASSERT( p.get_ptr() == test_buffer + 6 );
        RTF_ASSERT( p.original_ptr() == test_buffer );
    }
    {
        // Test -=
        r_byte_ptr_rw p(test_buffer, test_buffer_len);
        RTF_ASSERT_NO_THROW( p += 6 );
        RTF_ASSERT_NO_THROW( p -= 3 );
        RTF_ASSERT( p.length() == test_buffer_len );
        RTF_ASSERT( p.get_ptr() == test_buffer + 3 );
        RTF_ASSERT( p.original_ptr() == test_buffer );
    }
}

void test_r_utils_r_byte_ptr_rw::test_comparisons()
{
    r_byte_ptr_rw begin(test_buffer, test_buffer_len);
    r_byte_ptr_rw middle(test_buffer + (test_buffer_len / 2), test_buffer_len / 2);
    r_byte_ptr_rw end(test_buffer + test_buffer_len - 1, 1);

    {
        RTF_ASSERT_NO_THROW( begin == begin );
        RTF_ASSERT_NO_THROW( begin == r_byte_ptr_rw(test_buffer, test_buffer_len) );
        RTF_ASSERT_NO_THROW( begin != begin );
        RTF_ASSERT_NO_THROW( begin != r_byte_ptr_rw(test_buffer, test_buffer_len) );
        RTF_ASSERT_NO_THROW( begin <= begin );
        RTF_ASSERT_NO_THROW( begin <= r_byte_ptr_rw(test_buffer, test_buffer_len) );
        RTF_ASSERT_NO_THROW( begin >= begin );
        RTF_ASSERT_NO_THROW( begin >= r_byte_ptr_rw(test_buffer, test_buffer_len) );
        RTF_ASSERT_NO_THROW( begin < begin );
        RTF_ASSERT_NO_THROW( begin < r_byte_ptr_rw(test_buffer, test_buffer_len) );
        RTF_ASSERT_NO_THROW( begin > begin );
        RTF_ASSERT_NO_THROW( begin > r_byte_ptr_rw(test_buffer, test_buffer_len) );
    }
    {
        // Test ==
        RTF_ASSERT( begin == begin );
        RTF_ASSERT( (begin == end) == false );
    }
    {
        // Test !=
        RTF_ASSERT( begin != end );
        RTF_ASSERT( (begin != begin) == false );
    }
    {
        // Test <=
        RTF_ASSERT( begin <= end );
        RTF_ASSERT( begin <= begin );
        RTF_ASSERT( (end <= begin) == false );
    }
    {
        // Test >=
        RTF_ASSERT( end >= begin );
        RTF_ASSERT( begin >= begin );
        RTF_ASSERT( (begin >= end) == false );
    }
    {
        // Test <
        RTF_ASSERT( begin < end );
        RTF_ASSERT( (end < begin) == false );
    }
    {
        // Test >
        RTF_ASSERT( end > begin );
        RTF_ASSERT( (begin > end) == false );
    }
}

void test_r_utils_r_byte_ptr_rw::test_casting()
{
    {
        r_byte_ptr_rw p(test_buffer, test_buffer_len);
        RTF_ASSERT_NO_THROW( (char*)p );
        RTF_ASSERT_NO_THROW( (unsigned char*)p );
        RTF_ASSERT_NO_THROW( (short*)p );
        RTF_ASSERT_NO_THROW( (unsigned short*)p );
        RTF_ASSERT_NO_THROW( (int*)p );
        RTF_ASSERT_NO_THROW( (unsigned int*)p );
        RTF_ASSERT_NO_THROW( (long*)p );
        RTF_ASSERT_NO_THROW( (unsigned long*)p );
        RTF_ASSERT_NO_THROW( (long long*)p );
        RTF_ASSERT_NO_THROW( (unsigned long long*)p );
        RTF_ASSERT_NO_THROW( (float*)p );
        RTF_ASSERT_NO_THROW( (double*)p );

        RTF_ASSERT( *(char*)p == (char)test_buffer[0] );
        RTF_ASSERT( *(unsigned char*)p == (unsigned char)test_buffer[0] );
        RTF_ASSERT( *(short*)p == *(short*)&test_buffer );
        RTF_ASSERT( *(unsigned short*)p == *(unsigned short*)&test_buffer );
        RTF_ASSERT( *(int*)p == *(int*)&test_buffer );
        RTF_ASSERT( *(unsigned int*)p == *(unsigned int*)&test_buffer );
        RTF_ASSERT( *(long*)p == *(long*)&test_buffer );
        RTF_ASSERT( *(unsigned long*)p == *(unsigned long*)&test_buffer );
        RTF_ASSERT( *(long long*)p == *(long long*)&test_buffer );
        RTF_ASSERT( *(unsigned long long*)p == *(unsigned long long*)&test_buffer );
        RTF_ASSERT( *(float*)p == *(float*)&test_buffer );
        RTF_ASSERT( *(double*)p == *(double*)&test_buffer );

        // Test bounds checking
        p += test_buffer_len - 1; // Point to last byte in buffer
        RTF_ASSERT_NO_THROW( (char*)p );
        RTF_ASSERT_NO_THROW( (unsigned char*)p );
        RTF_ASSERT_THROWS( (short*)p, r_exception ); // Casting and dereferencing would cause out-of-bounds access.
        RTF_ASSERT_THROWS( (unsigned short*)p, r_exception );
        RTF_ASSERT_THROWS( (int*)p, r_exception );
        RTF_ASSERT_THROWS( (unsigned int*)p, r_exception );
        RTF_ASSERT_THROWS( (long*)p, r_exception );
        RTF_ASSERT_THROWS( (unsigned long*)p, r_exception );
        RTF_ASSERT_THROWS( (long long*)p, r_exception );
        RTF_ASSERT_THROWS( (unsigned long long*)p, r_exception );
        RTF_ASSERT_THROWS( (float*)p, r_exception );
        RTF_ASSERT_THROWS( (double*)p, r_exception );
    }
    {
        // Test bounds checking when pointer points BEFORE start of buffer
        r_byte_ptr_rw p(test_buffer, test_buffer_len);
        RTF_ASSERT_NO_THROW( --p ); // pointer before head of buffer
        RTF_ASSERT_THROWS( (char*)p, r_exception );
        RTF_ASSERT_THROWS( (unsigned char*)p, r_exception );
        RTF_ASSERT_THROWS( (short*)p, r_exception );
        RTF_ASSERT_THROWS( (unsigned short*)p, r_exception );
        RTF_ASSERT_THROWS( (int*)p, r_exception );
        RTF_ASSERT_THROWS( (unsigned int*)p, r_exception );
        RTF_ASSERT_THROWS( (long*)p, r_exception );
        RTF_ASSERT_THROWS( (unsigned long*)p, r_exception );
        RTF_ASSERT_THROWS( (long long*)p, r_exception );
        RTF_ASSERT_THROWS( (unsigned long long*)p, r_exception );
        RTF_ASSERT_THROWS( (float*)p, r_exception );
        RTF_ASSERT_THROWS( (double*)p, r_exception );
    }
}

void test_r_utils_r_byte_ptr_rw::test_offset()
{
    {
        r_byte_ptr_rw p(test_buffer, test_buffer_len);
        RTF_ASSERT_NO_THROW( p.offset() );
        RTF_ASSERT_NO_THROW( p.remaining() );
    }
    {
        r_byte_ptr_rw p(test_buffer, test_buffer_len);
        p += 5;
        RTF_ASSERT( p.offset() == 5 );
        RTF_ASSERT( p.remaining() == 3 );
    }
    {
        r_byte_ptr_rw p(test_buffer, test_buffer_len);
        p--;
        RTF_ASSERT( p.offset() == -1 );
        RTF_ASSERT( p.in_bounds() == false );
        p++;
        p += test_buffer_len;
        RTF_ASSERT( p.remaining() == 0 );
        RTF_ASSERT( p.in_bounds() == false );
        p++;
        RTF_ASSERT( p.remaining() == -1 );
        RTF_ASSERT( p.in_bounds() == false );
    }
}

void test_r_utils_r_byte_ptr_rw::test_seek()
{
    {
        r_byte_ptr_rw p(test_buffer, test_buffer_len);
        RTF_ASSERT_NO_THROW( p.seek(1) );
        RTF_ASSERT_THROWS( p.seek(test_buffer_len), r_exception );
        RTF_ASSERT_THROWS( p.seek(-1), r_exception );
    }
    {
        r_byte_ptr_rw p(test_buffer, test_buffer_len);
        RTF_ASSERT( p.seek(3).offset() == 3 );
        RTF_ASSERT( p.remaining() == (ptrdiff_t)test_buffer_len - p.offset() );
    }
}

void test_r_utils_r_byte_ptr_rw::test_read_write()
{
    {
        // read
        r_byte_ptr_rw p(test_buffer, test_buffer_len);
        RTF_ASSERT( p.read<uint32_t>() == *(uint32_t*)&test_buffer[0] );
        RTF_ASSERT( p.consume<uint32_t>() == *(uint32_t*)&test_buffer[0] ); // Advances pointer
        RTF_ASSERT( p.consume<uint32_t>() == *(uint32_t*)&test_buffer[sizeof(uint32_t)] );

        // Test bounds
        p.seek(test_buffer_len-1);
        RTF_ASSERT_NO_THROW( p.read<uint8_t>() );
        RTF_ASSERT_THROWS( p.read<uint16_t>(), r_exception );
    }
    {
        // write

        // Make a copy of the buffer for validation later
        vector<uint8_t> cpy(test_buffer_len);
        memcpy(&cpy[0], test_buffer, test_buffer_len);

        r_byte_ptr_rw p(test_buffer, test_buffer_len);
        int16_t temp = *(int16_t*)p;
        RTF_ASSERT( p.write<int16_t>(-42) == -42 );
        RTF_ASSERT( p.offset() == sizeof(int16_t) ); // Make sure ptr moved forward

        // Make sure we wrote the data
        p.seek(0);
        RTF_ASSERT( memcmp(test_buffer, p, test_buffer_len) == 0 );
        RTF_ASSERT( memcmp(test_buffer, &cpy[0], test_buffer_len) != 0 );
        RTF_ASSERT( *(int16_t*)p == -42 );
        RTF_ASSERT( *(int16_t*)test_buffer == -42 );

        // Now put the data back
        p.write<int16_t>(temp);
        p.seek(0);
        RTF_ASSERT( memcmp(test_buffer, p, test_buffer_len) == 0 );
        RTF_ASSERT( memcmp(test_buffer, &cpy[0], test_buffer_len) == 0 );

        // Test bounds
        p.seek(test_buffer_len-1);
        RTF_ASSERT_NO_THROW( p.write<uint8_t>( test_buffer[test_buffer_len-1] ) );
        p.seek(test_buffer_len-1);
        RTF_ASSERT_THROWS( p.write<uint16_t>( 0xa5f0 ), r_exception );
    }
}

void test_r_utils_r_byte_ptr_rw::test_bulk_write()
{
    // Make a copy of the buffer for validation later
    vector<uint8_t> new_buffer(test_buffer_len);
    memset( &new_buffer[0], 0, test_buffer_len );

    {
        r_byte_ptr_rw p(&new_buffer[0], test_buffer_len);
        RTF_ASSERT_NO_THROW(p.bulk_write( test_buffer, test_buffer_len ));
        RTF_ASSERT_THROWS(p.bulk_write( test_buffer, test_buffer_len ), r_exception);
    }

    RTF_ASSERT(memcmp(&new_buffer[0], test_buffer, test_buffer_len) == 0);
}

void test_r_utils_r_byte_ptr_rw::test_append()
{
    vector<uint8_t> srcBuffer = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20};
    string srcHash;

    {
        r_sha_256 hasher;
        hasher.update(&srcBuffer[0], 21);
        hasher.finalize();
        srcHash = hasher.get_as_string();
    }

    uint8_t* src = &srcBuffer[0];

    vector<uint8_t> buffer;

    auto p = r_byte_ptr_rw::append(buffer, 5);
    p.bulk_write(src, 5);
    src+=5;

    p = r_byte_ptr_rw::append(buffer, 5);
    p.bulk_write(src, 5);
    src+=5;

    p = r_byte_ptr_rw::append(buffer, 5);
    p.bulk_write(src, 5);
    src+=5;

    p = r_byte_ptr_rw::append(buffer, 5);
    p.bulk_write(src, 5);
    src+=5;

    p = r_byte_ptr_rw::append(buffer, 1);
    p.bulk_write(src, 1);
    src+=1;

    RTF_ASSERT(buffer.capacity() == 40);

    RTF_ASSERT(srcBuffer.size() == buffer.size());

    string outHash;
    {
        r_sha_256 hasher;
        hasher.update(&buffer[0], 21);
        hasher.finalize();
        outHash == hasher.get_as_string();
    }

    RTF_ASSERT(buffer.size() == srcBuffer.size());
}
