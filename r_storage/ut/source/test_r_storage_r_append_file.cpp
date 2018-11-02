
#include "test_r_storage_r_append_file.h"
#include "r_storage/r_append_file.h"
#include "r_utils/r_file.h"

using namespace r_storage;
using namespace r_utils;
using namespace std;

REGISTER_TEST_FIXTURE(test_r_storage_r_append_file);

void test_r_storage_r_append_file::setup()
{
    r_append_file::allocate("1024_5mb", (1024*1024) * 5, 1024, "ddf3f926-1eac-43c9-913d-c6668283171c");

    r_file outFile = r_file::open("scratch", "w+b");

    r_fs::fallocate( outFile, (1024*1024) * 5 );
}

void test_r_storage_r_append_file::teardown()
{
    remove( "1024_5mb" );
    remove( "scratch" );
}

void test_r_storage_r_append_file::test_allocate()
{
    r_append_file f( "1024_5mb" );

    uint16_t version = 0;
    uint64_t maxEntries = 0;
    uint64_t timeOfFirst = 0;
    f._read_header( version, maxEntries, timeOfFirst );

    RTF_ASSERT( version == 1 );
    RTF_ASSERT( maxEntries == 1024 );
    RTF_ASSERT( timeOfFirst == 0 );
}

void test_r_storage_r_append_file::test_append_side_effects()
{
    r_append_file f( "1024_5mb" );

    uint8_t frame[2048];
    memset( frame, 42, 2048 );

    f.append( 1000, 0, frame, 2048 );

    uint16_t version = 0;
    uint64_t maxEntries = 0;
    uint64_t timeOfFirst = 0;
    f._read_header( version, maxEntries, timeOfFirst );

    RTF_ASSERT( timeOfFirst == 1000 );

    uint64_t numIndexes = 0;
    f._read_num_indexes( numIndexes );

    RTF_ASSERT( numIndexes == 1 );

    uint64_t timeOfs = 0;
    uint64_t frameOfs = 0;
    f._read_index( 0, timeOfs, frameOfs );

    RTF_ASSERT( timeOfs == 0 );

    uint32_t flags;
    uint32_t payloadLen = 0;

    f._read_frame_md( frameOfs, timeOfs, flags, payloadLen );

    RTF_ASSERT( timeOfs == 0 );
    RTF_ASSERT( payloadLen == 2048 );

    f.append( 1010, 0, frame, 1024 );

    f._read_header( version, maxEntries, timeOfFirst );
    RTF_ASSERT( timeOfFirst == 1000 );

    f._read_num_indexes( numIndexes );
    RTF_ASSERT( numIndexes == 2 );

    f._read_index( 1, timeOfs, frameOfs );
    RTF_ASSERT( timeOfs == 10 );

    f._read_frame_md( frameOfs, timeOfs, flags, payloadLen );
    RTF_ASSERT( timeOfs == 10 );
    RTF_ASSERT( payloadLen == 1024 );
}

void test_r_storage_r_append_file::test_boundry_times()
{
    r_append_file f( "1024_5mb" );

    uint32_t TWO_K = 2048;

    uint8_t frame[TWO_K];
    memset( frame, 42, TWO_K );

    for( int i = 0; i < 100; i++ )
    {
        f.append( 1000 + (i*10), 0, frame, random() % TWO_K );
    }

    uint64_t startTime = f.first_key();   // ts + (0*10)
    uint64_t lastTime = f.last_key();     // ts + (99*10)

    RTF_ASSERT( startTime == 1000 );
    RTF_ASSERT( lastTime == 1990 );
}

void test_r_storage_r_append_file::test_iteration()
{
    r_append_file f( "1024_5mb" );

    uint32_t TWO_K = 2048;

    uint8_t frame[TWO_K];
    memset( frame, 42, TWO_K );

    for( int i = 0; i < 100; i++ )
    {
        f.append( 1000 + (i*10), 0, frame, random() % TWO_K );
    }

    uint64_t ts = 0;
    uint32_t payloadLen = 0;
    const uint8_t* payload = nullptr;

    r_append_file::iterator i = f.get_iterator();

    i.begin();

    uint32_t flags;
    i.current_data( ts, flags, payloadLen, payload );
    RTF_ASSERT( ts == 1000 );

    i.next();                           // step forward

    i.current_data( ts, flags, payloadLen, payload );
    RTF_ASSERT( ts == 1010 );

    i.next();                           // step forward

    i.current_data( ts, flags, payloadLen, payload );
    RTF_ASSERT( ts == 1020 );

    i.begin();                          // jump back to start

    i.current_data( ts, flags, payloadLen, payload );
    RTF_ASSERT( ts == 1000 );

    i.end();                            // end puts us on the last valid entry

    RTF_ASSERT( i.valid() == false );

    i.prev();

    i.current_data( ts, flags, payloadLen, payload );
    RTF_ASSERT( ts == 1990 );

    RTF_ASSERT( i.next() == true );      // we're allowed to move into the iter_valid() == false state
    RTF_ASSERT( i.next() == false );     // an iterator @ the end sentinal cannot be further incremented

    i.end();                            // Now, jump back to the last valid

    i.prev();                           // backup
    i.prev();                           // backup
    i.current_data( ts, flags, payloadLen, payload );
    RTF_ASSERT( ts == 1980 );

    int idx = 0;
    for( i.begin(); i.valid(); i.next() )
    {
        i.current_data( ts, flags, payloadLen, payload );
        RTF_ASSERT( ts == (uint64_t)1000 + (idx * 10) );
        idx++;
    }
}

void test_r_storage_r_append_file::test_payload()
{
    r_append_file f( "1024_5mb" );

    uint32_t TWO_K = 2048;

    uint8_t frame[TWO_K];

    for( int i = 0; i < 100; i++ )
    {
        memset( frame, i, TWO_K );
        f.append( 1000 + (i*10), 0, frame, TWO_K );
    }

//    uint8_t outputFrame[TWO_K];

    r_append_file::iterator i = f.get_iterator();

    int idx = 0;
    for( i.begin(); i.valid(); i.next() )
    {
        uint64_t ts;
        uint32_t payloadLen;
        const uint8_t* payload;
        uint32_t flags;
        i.current_data( ts, flags, payloadLen, payload );

        memset( frame, idx, TWO_K );

        RTF_ASSERT( memcmp( payload, frame, TWO_K ) == 0 );

        idx++;
    }
}

void test_r_storage_r_append_file::test_find()
{
    r_append_file f( "1024_5mb" );

    uint32_t TWO_K = 2048;

    uint8_t frame[TWO_K];
    memset( frame, 42, TWO_K );

    for( int i = 0; i < 100; i++ )
    {
        f.append( 1000 + (i*10), 0, frame, TWO_K );
    }

    // find users std::lower_bound(), which returns the first element that does
    // not compare less than the key.

    r_append_file::iterator i = f.get_iterator();

    i.find( 1500 ); // exact find...

    uint64_t ts = 0;
    uint32_t payloadLen = 0;
    const uint8_t* payload = nullptr;

    uint32_t flags;
    i.current_data( ts, flags, payloadLen, payload );
    RTF_ASSERT( ts == 1500 );
    RTF_ASSERT( payloadLen == TWO_K );

    i.find( 1491 );

    i.current_data( ts, flags, payloadLen, payload );

    RTF_ASSERT( ts == 1500 );
    RTF_ASSERT( payloadLen == TWO_K );
}

void test_r_storage_r_append_file::test_file_full()
{
    r_append_file f( "1024_5mb" );

    uint32_t TWO_K = 2048;

    uint8_t frame[TWO_K];
    memset( frame, 42, TWO_K );

    for( int i = 0; i < 1024; i++ )
    {
        uint32_t size = random() % TWO_K;
        RTF_ASSERT( f.fits( size ) );
        f.append( 1000 + (i*10), 0, frame, size );
    }

    RTF_ASSERT( !f.fits( TWO_K ) );
}
