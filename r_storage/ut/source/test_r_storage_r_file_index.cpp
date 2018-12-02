
#include "test_r_storage_r_file_index.h"
#include "r_storage/r_file_index.h"
#include "r_utils/r_file.h"
#include "r_db/r_sqlite_conn.h"
#include "r_db/r_sqlite_pager.h"
#include <chrono>

using namespace r_storage;
using namespace r_utils;
using namespace r_db;
using namespace std;
using namespace std::chrono;

REGISTER_TEST_FIXTURE(test_r_storage_r_file_index);

void test_r_storage_r_file_index::setup()
{
    remove("index");

    r_file_index::allocate("index");
}

void test_r_storage_r_file_index::teardown()
{
    remove( "index" );
}

void test_r_storage_r_file_index::test_allocate()
{
    RTF_ASSERT(r_fs::file_exists("index"));
}

void test_r_storage_r_file_index::test_create_invalid()
{
    r_file_index db("index");

    uint64_t now = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

    r_sqlite_conn conn("index");
    db.create_invalid_segment_file(conn, "/mnt/fs1/1", "487cffdf-1cb4-4f0b-b220-b88502a6c096");
    db.create_invalid_segment_file(conn, "/mnt/fs1/2", "487cffdf-1cb4-4f0b-b220-b88502a6c096");
    db.create_invalid_segment_file(conn, "/mnt/fs1/3", "487cffdf-1cb4-4f0b-b220-b88502a6c096");
    db.create_invalid_segment_file(conn, "/mnt/fs1/4", "487cffdf-1cb4-4f0b-b220-b88502a6c096");
    db.create_invalid_segment_file(conn, "/mnt/fs1/5", "487cffdf-1cb4-4f0b-b220-b88502a6c096");
    db.create_invalid_segment_file(conn, "/mnt/fs1/6", "487cffdf-1cb4-4f0b-b220-b88502a6c096");
    db.create_invalid_segment_file(conn, "/mnt/fs1/7", "487cffdf-1cb4-4f0b-b220-b88502a6c096");
    db.create_invalid_segment_file(conn, "/mnt/fs1/8", "487cffdf-1cb4-4f0b-b220-b88502a6c096");
    db.create_invalid_segment_file(conn, "/mnt/fs1/9", "487cffdf-1cb4-4f0b-b220-b88502a6c096");
    db.create_invalid_segment_file(conn, "/mnt/fs1/10", "487cffdf-1cb4-4f0b-b220-b88502a6c096");

    r_sqlite_pager pager("*", "segment_files", "start_time", 10);

    pager.find(conn);

    while(pager.valid())
    {
        auto row = pager.current();
        RTF_ASSERT(r_string_utils::s_to_uint64(row["end_time"]) < now);
        RTF_ASSERT(row["valid"] == "0");
        RTF_ASSERT(row["data_source_id"] == "487cffdf-1cb4-4f0b-b220-b88502a6c096");
        pager.next(conn);
    }
}

size_t _num_valid(const r_sqlite_conn& conn)
{
    r_sqlite_pager pager("*", "segment_files", "start_time", 10);

    pager.find(conn);

    size_t numValid = 0;

    while(pager.valid())
    {
        auto row = pager.current();
        if(row["valid"] == "1")
            ++numValid;
        pager.next(conn);
    }

    return numValid;
}

void _print(const r_sqlite_conn& conn)
{
    r_sqlite_pager pager("*", "segment_files", "start_time", 10);

    pager.find(conn);

    while(pager.valid())
    {
        auto row = pager.current();
        printf("[");
        for(auto p : row)
            printf("%s: %s, ", p.first.c_str(), p.second.c_str());
        printf("]\n");
        pager.next(conn);
    }
}

void test_r_storage_r_file_index::test_recycle_append()
{
    r_file_index db("index");

    uint64_t now = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

    r_sqlite_conn conn("index");
    db.create_invalid_segment_file(conn, "/mnt/fs1/1", "487cffdf-1cb4-4f0b-b220-b88502a6c096");
    db.create_invalid_segment_file(conn, "/mnt/fs1/2", "487cffdf-1cb4-4f0b-b220-b88502a6c096");
    db.create_invalid_segment_file(conn, "/mnt/fs1/3", "487cffdf-1cb4-4f0b-b220-b88502a6c096");
    db.create_invalid_segment_file(conn, "/mnt/fs1/4", "487cffdf-1cb4-4f0b-b220-b88502a6c096");
    db.create_invalid_segment_file(conn, "/mnt/fs1/5", "487cffdf-1cb4-4f0b-b220-b88502a6c096");
    db.create_invalid_segment_file(conn, "/mnt/fs1/6", "487cffdf-1cb4-4f0b-b220-b88502a6c096");
    db.create_invalid_segment_file(conn, "/mnt/fs1/7", "487cffdf-1cb4-4f0b-b220-b88502a6c096");
    db.create_invalid_segment_file(conn, "/mnt/fs1/8", "487cffdf-1cb4-4f0b-b220-b88502a6c096");
    db.create_invalid_segment_file(conn, "/mnt/fs1/9", "487cffdf-1cb4-4f0b-b220-b88502a6c096");
    db.create_invalid_segment_file(conn, "/mnt/fs1/10", "487cffdf-1cb4-4f0b-b220-b88502a6c096");

    RTF_ASSERT(_num_valid(conn) == 0);

    auto sf = db.recycle_append(now, "9fac8f69-7b5a-43ef-afe5-e8830b0f7657", "video", "sdp", [](const segment_file& sf){});
    now += 1000;

    RTF_ASSERT(_num_valid(conn) == 1);

    RTF_ASSERT(sf.path == "/mnt/fs1/10");

    db.recycle_append(now, "9fac8f69-7b5a-43ef-afe5-e8830b0f7657", "video", "sdp", [](const segment_file& sf){});
    now += 1000;

    db.recycle_append(now, "9fac8f69-7b5a-43ef-afe5-e8830b0f7657", "video", "sdp", [](const segment_file& sf){});
    now += 1000;

    db.recycle_append(now, "9fac8f69-7b5a-43ef-afe5-e8830b0f7657", "video", "sdp", [](const segment_file& sf){});
    now += 1000;

    db.recycle_append(now, "9fac8f69-7b5a-43ef-afe5-e8830b0f7657", "video", "sdp", [](const segment_file& sf){});
    now += 1000;

    db.recycle_append(now, "9fac8f69-7b5a-43ef-afe5-e8830b0f7657", "video", "sdp", [](const segment_file& sf){});
    now += 1000;

    db.recycle_append(now, "9fac8f69-7b5a-43ef-afe5-e8830b0f7657", "video", "sdp", [](const segment_file& sf){});
    now += 1000;

    db.recycle_append(now, "9fac8f69-7b5a-43ef-afe5-e8830b0f7657", "video", "sdp", [](const segment_file& sf){});
    now += 1000;

    db.recycle_append(now, "9fac8f69-7b5a-43ef-afe5-e8830b0f7657", "video", "sdp", [](const segment_file& sf){});
    now += 1000;

    db.recycle_append(now, "9fac8f69-7b5a-43ef-afe5-e8830b0f7657", "video", "sdp", [](const segment_file& sf){});
    now += 1000;

    RTF_ASSERT(_num_valid(conn) == 10);

    sf = db.recycle_append(now, "9fac8f69-7b5a-43ef-afe5-e8830b0f7657", "video", "sdp", [](const segment_file& sf){});
    now += 1000;
    
    RTF_ASSERT(sf.path == "/mnt/fs1/10");

    RTF_ASSERT(_num_valid(conn) == 10);

    sf = db.recycle_append(now, "9fac8f69-7b5a-43ef-afe5-e8830b0f7657", "video", "sdp", [](const segment_file& sf){});
    now += 1000;

    RTF_ASSERT(sf.path == "/mnt/fs1/9");

    RTF_ASSERT(_num_valid(conn) == 10);
}

void test_r_storage_r_file_index::test_update_end_time()
{
    r_file_index db("index");

    uint64_t now = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

    r_sqlite_conn conn("index");
    db.create_invalid_segment_file(conn, "/mnt/fs1/1", "487cffdf-1cb4-4f0b-b220-b88502a6c096");
    db.create_invalid_segment_file(conn, "/mnt/fs1/2", "487cffdf-1cb4-4f0b-b220-b88502a6c096");
    db.create_invalid_segment_file(conn, "/mnt/fs1/3", "487cffdf-1cb4-4f0b-b220-b88502a6c096");
    db.create_invalid_segment_file(conn, "/mnt/fs1/4", "487cffdf-1cb4-4f0b-b220-b88502a6c096");
    db.create_invalid_segment_file(conn, "/mnt/fs1/5", "487cffdf-1cb4-4f0b-b220-b88502a6c096");
    db.create_invalid_segment_file(conn, "/mnt/fs1/6", "487cffdf-1cb4-4f0b-b220-b88502a6c096");
    db.create_invalid_segment_file(conn, "/mnt/fs1/7", "487cffdf-1cb4-4f0b-b220-b88502a6c096");
    db.create_invalid_segment_file(conn, "/mnt/fs1/8", "487cffdf-1cb4-4f0b-b220-b88502a6c096");
    db.create_invalid_segment_file(conn, "/mnt/fs1/9", "487cffdf-1cb4-4f0b-b220-b88502a6c096");
    db.create_invalid_segment_file(conn, "/mnt/fs1/10", "487cffdf-1cb4-4f0b-b220-b88502a6c096");

    RTF_ASSERT(_num_valid(conn) == 0);

    auto sf = db.recycle_append(now, "9fac8f69-7b5a-43ef-afe5-e8830b0f7657", "video", "sdp", [](const segment_file& sf){});

    auto result = conn.exec("SELECT * FROM segment_files ORDER BY start_time DESC LIMIT 1;");

    RTF_ASSERT(result.front()["end_time"] == "0");

    now = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

    db.update_end_time(sf, now);

    result = conn.exec("SELECT * FROM segment_files ORDER BY start_time DESC LIMIT 1;");
    
    RTF_ASSERT(r_string_utils::s_to_uint64(result.front()["end_time"]) == now);    
}

void test_r_storage_r_file_index::test_free()
{
    r_file_index db("index");

    r_sqlite_conn conn("index");
    db.create_invalid_segment_file(conn, "/mnt/fs1/1", "487cffdf-1cb4-4f0b-b220-b88502a6c096");
    db.create_invalid_segment_file(conn, "/mnt/fs1/2", "487cffdf-1cb4-4f0b-b220-b88502a6c096");
    db.create_invalid_segment_file(conn, "/mnt/fs1/3", "487cffdf-1cb4-4f0b-b220-b88502a6c096");
    db.create_invalid_segment_file(conn, "/mnt/fs1/4", "487cffdf-1cb4-4f0b-b220-b88502a6c096");
    db.create_invalid_segment_file(conn, "/mnt/fs1/5", "487cffdf-1cb4-4f0b-b220-b88502a6c096");
    db.create_invalid_segment_file(conn, "/mnt/fs1/6", "487cffdf-1cb4-4f0b-b220-b88502a6c096");
    db.create_invalid_segment_file(conn, "/mnt/fs1/7", "487cffdf-1cb4-4f0b-b220-b88502a6c096");
    db.create_invalid_segment_file(conn, "/mnt/fs1/8", "487cffdf-1cb4-4f0b-b220-b88502a6c096");
    db.create_invalid_segment_file(conn, "/mnt/fs1/9", "487cffdf-1cb4-4f0b-b220-b88502a6c096");
    db.create_invalid_segment_file(conn, "/mnt/fs1/10", "487cffdf-1cb4-4f0b-b220-b88502a6c096");

    db.recycle_append(1551976959065, "9fac8f69-7b5a-43ef-afe5-e8830b0f7657", "video", "sdp", [](const segment_file& sf){});
    db.recycle_append(1551976960065, "9fac8f69-7b5a-43ef-afe5-e8830b0f7657", "video", "sdp", [](const segment_file& sf){});
    db.recycle_append(1551976961065, "9fac8f69-7b5a-43ef-afe5-e8830b0f7657", "video", "sdp", [](const segment_file& sf){});
    db.recycle_append(1551976962065, "9fac8f69-7b5a-43ef-afe5-e8830b0f7657", "video", "sdp", [](const segment_file& sf){});
    db.recycle_append(1551976963065, "9fac8f69-7b5a-43ef-afe5-e8830b0f7657", "video", "sdp", [](const segment_file& sf){});
    db.recycle_append(1551976964065, "9fac8f69-7b5a-43ef-afe5-e8830b0f7657", "video", "sdp", [](const segment_file& sf){});
    db.recycle_append(1551976965065, "9fac8f69-7b5a-43ef-afe5-e8830b0f7657", "video", "sdp", [](const segment_file& sf){});
    db.recycle_append(1551976966065, "9fac8f69-7b5a-43ef-afe5-e8830b0f7657", "video", "sdp", [](const segment_file& sf){});
    db.recycle_append(1551976967065, "9fac8f69-7b5a-43ef-afe5-e8830b0f7657", "video", "sdp", [](const segment_file& sf){});
    db.recycle_append(1551976968065, "9fac8f69-7b5a-43ef-afe5-e8830b0f7657", "video", "sdp", [](const segment_file& sf){});

#if 0
    _print(conn);
    printf("**************************************************************************\n");
    fflush(stdout);
#endif

    db.free(1551976962065, 1551976965065, "video");
    RTF_ASSERT(_num_valid(conn) == 7);

    r_sqlite_pager pager("*", "segment_files", "start_time", 10);
    pager.find(conn);

#if 0
    _print(conn);
#endif
}

void test_r_storage_r_file_index::test_basic_iteration()
{
    r_file_index db("index");

    r_sqlite_conn conn("index");
    db.create_invalid_segment_file(conn, "/mnt/fs1/1", "487cffdf-1cb4-4f0b-b220-b88502a6c096");
    db.create_invalid_segment_file(conn, "/mnt/fs1/2", "487cffdf-1cb4-4f0b-b220-b88502a6c096");
    db.create_invalid_segment_file(conn, "/mnt/fs1/3", "487cffdf-1cb4-4f0b-b220-b88502a6c096");
    db.create_invalid_segment_file(conn, "/mnt/fs1/4", "487cffdf-1cb4-4f0b-b220-b88502a6c096");
    db.create_invalid_segment_file(conn, "/mnt/fs1/5", "487cffdf-1cb4-4f0b-b220-b88502a6c096");
    db.create_invalid_segment_file(conn, "/mnt/fs1/6", "487cffdf-1cb4-4f0b-b220-b88502a6c096");
    db.create_invalid_segment_file(conn, "/mnt/fs1/7", "487cffdf-1cb4-4f0b-b220-b88502a6c096");
    db.create_invalid_segment_file(conn, "/mnt/fs1/8", "487cffdf-1cb4-4f0b-b220-b88502a6c096");
    db.create_invalid_segment_file(conn, "/mnt/fs1/9", "487cffdf-1cb4-4f0b-b220-b88502a6c096");
    db.create_invalid_segment_file(conn, "/mnt/fs1/10", "487cffdf-1cb4-4f0b-b220-b88502a6c096");

    // 1551976959065 this time is about 10 years in the future...

    auto sf = db.recycle_append(1551976959065, "9fac8f69-7b5a-43ef-afe5-e8830b0f7657", "video", "sdp", [](const segment_file& sf){});
    db.update_end_time(sf, 1551976959565);
    sf = db.recycle_append(1551976960065, "9fac8f69-7b5a-43ef-afe5-e8830b0f7657", "video", "sdp", [](const segment_file& sf){});
    db.update_end_time(sf, 1551976960565);
    sf = db.recycle_append(1551976961065, "9fac8f69-7b5a-43ef-afe5-e8830b0f7657", "video", "sdp", [](const segment_file& sf){});
    db.update_end_time(sf, 1551976961565);
    sf = db.recycle_append(1551976962065, "0c080d75-96e1-4d75-bdf1-ec68152fd081", "video", "sdp", [](const segment_file& sf){});
    db.update_end_time(sf, 1551976962565);
    sf = db.recycle_append(1551976963065, "0c080d75-96e1-4d75-bdf1-ec68152fd081", "md", "sdp", [](const segment_file& sf){});
    db.update_end_time(sf, 1551976963565);
    sf = db.recycle_append(1551976964065, "9fac8f69-7b5a-43ef-afe5-e8830b0f7657", "video", "sdp", [](const segment_file& sf){});
    db.update_end_time(sf, 1551976964565);
    sf = db.recycle_append(1551976965065, "9fac8f69-7b5a-43ef-afe5-e8830b0f7657", "video", "sdp", [](const segment_file& sf){});
    db.update_end_time(sf, 1551976965565);
    sf = db.recycle_append(1551976966065, "9fac8f69-7b5a-43ef-afe5-e8830b0f7657", "video", "sdp", [](const segment_file& sf){});
    db.update_end_time(sf, 1551976966565);
    sf = db.recycle_append(1551976967065, "0c080d75-96e1-4d75-bdf1-ec68152fd081", "video", "sdp", [](const segment_file& sf){});
    db.update_end_time(sf, 1551976967565);
    sf = db.recycle_append(1551976968065, "9fac8f69-7b5a-43ef-afe5-e8830b0f7657", "video", "sdp", [](const segment_file& sf){});
    db.update_end_time(sf, 1551976968565);

    auto iter = db.get_iterator("start_time", "0c080d75-96e1-4d75-bdf1-ec68152fd081", "video");
    
    iter.begin();
    sf = iter.current_data();
    RTF_ASSERT(sf.start_time == 1551976962065);

    iter.next();
    sf = iter.current_data();
    RTF_ASSERT(sf.start_time == 1551976967065);

    iter.next();
    RTF_ASSERT(!iter.valid());
}
