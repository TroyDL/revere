
#include "test_r_utils_r_file_lock.h"
#include "r_utils/r_file_lock.h"
#include "r_utils/r_macro.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <thread>

#define RMDIR rmdir
#define UNLINK unlink
#define MKDIR(a) mkdir(a,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)

using namespace std;
using namespace r_utils;

REGISTER_TEST_FIXTURE(test_r_utils_r_file_lock);

void test_r_utils_r_file_lock::setup()
{
    lockFile = fopen( "lockfile", "w+b" );
}

void test_r_utils_r_file_lock::teardown()
{
    fclose(lockFile);
    UNLINK("lockfile");
}

void test_r_utils_r_file_lock::test_constructor()
{
    r_file_lock fileLock( fileno( lockFile ) );
}

void test_r_utils_r_file_lock::test_exclusive()
{
    FILE* otherFile = fopen( "lockfile", "w+b" );

    int state = 42;

    r_file_lock fileLock( fileno( lockFile ) );

    {
        r_file_lock_guard g( fileLock );

        thread t([&](){
                r_file_lock newLock( fileno( otherFile ) );
                r_file_lock_guard g( newLock );
                state = 43;
            });
        t.detach();

        usleep( 250000 );

        RTF_ASSERT( state == 42 );
    }

    usleep( 100000 );

    RTF_ASSERT( state == 43 );

    fclose( otherFile );
}

void test_r_utils_r_file_lock::test_shared()
{
    FILE* otherFileA = fopen( "lockfile", "w+b" );
    FILE* otherFileB = fopen( "lockfile", "w+b" );

    int state = 42;  // initially state is 42,

    // But then we fire up two threads with a shared lock

    thread t1([&](){
            r_file_lock newLock( fileno( otherFileA ) );
            r_file_lock_guard g( newLock, false );
            ++state;
            usleep( 500000 );
        });
    t1.detach();

    thread t2([&](){
            r_file_lock newLock( fileno( otherFileB ) );
            r_file_lock_guard g( newLock, false );
            ++state;
            usleep( 500000 );
        });
    t2.detach();

    usleep( 250000 ); // give the above threads a chance to start...

    {
        r_file_lock newLock( fileno( lockFile ) );
        r_file_lock_guard g( newLock );
        // since the above shared locks must be let go before an exclusive can be acquired, then we know at this point
        // state should be 44.
        RTF_ASSERT( state == 44 );
    }

    fclose( otherFileB );
    fclose( otherFileA );
}

void test_r_utils_r_file_lock::test_combo()
{
    // OK, so the interesting thing about file locks: It looks like everyone using a lock needs a
    // unique file handle.

    FILE* otherFileA = fopen( "lockfile", "w+b" );

    int num = 0;
    auto th = thread([&](){
        r_file_lock fileLock1( fileno( otherFileA ) );
        r_file_lock_guard g1( fileLock1, false );
        usleep(5000000);
        num = 42;
        FULL_MEM_BARRIER();
    });

    usleep(1000000); // give the above thread a chance to run...

    r_file_lock fileLock2( fileno( lockFile ) );
    r_file_lock_guard g2( fileLock2 );
    
    RTF_ASSERT(num==42);

    th.join();
}