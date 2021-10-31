#ifdef IS_WINDOWS
#define _CRT_RAND_S
#include <stdlib.h>
#endif
#include "utils.h"
#include "r_storage/r_dumbdex.h"
#include "r_storage/r_ind_block.h"
#include "r_storage/r_storage_file.h"
#include "r_storage/r_rel_block.h"
#include "r_utils/r_exception.h"
#include "r_utils/r_algorithms.h"
#include "r_utils/r_file.h"
#include "r_utils/r_blob_tree.h"
#include "r_pipeline/r_gst_source.h"
#include "r_pipeline/r_arg.h"
#include "r_pipeline/r_stream_info.h"
#include "r_mux/r_muxer.h"
#include "test_r_storage.h"

#include <vector>
#include <numeric>
#include <thread>
#include <vector>
#include <functional>
#include <algorithm>
#include <iterator>
#include <chrono>

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <sys/types.h>
#ifdef IS_WINDOWS
#pragma warning( push )
#pragma warning( disable : 4244 )
#endif
#include <gst/gst.h>
#ifdef IS_WINDOWS
#pragma warning( pop )
#endif

using namespace std;
using namespace std::chrono;
using namespace r_storage;
using namespace r_utils;
using namespace r_pipeline;
using namespace r_fakey;
using namespace r_mux;

REGISTER_TEST_FIXTURE(test_r_storage);

static void _whack_files()
{
    if(r_fs::file_exists("test_file"))
        r_fs::remove_file("test_file");
    if(r_fs::file_exists("ten_mb_file"))
        r_fs::remove_file("ten_mb_file");
}

void test_r_storage::setup()
{
    gst_init(NULL, NULL);

    _whack_files();
}

void test_r_storage::teardown()
{
    _whack_files();
}

static int _random()
{
#ifdef IS_WINDOWS
    unsigned int rand_val;
    rand_s(&rand_val);
    return abs((int)rand_val);
#endif
#ifdef IS_LINUX
    return random();
#endif
}

void test_r_storage::test_r_dumbdex_writing()
{
    vector<uint8_t> buffer(1024*1024);

    r_dumbdex d(&buffer[0], 1000);

    for(int i = 0; i < 1000; ++i)
        d._push_free_block(i);

    for(int i = 0; i < 10000; ++i)
        d.insert(_random() % 65535);
}

template<typename T1, typename T2>
pair<T1, T2> mp(T1 a, T2 b)
{
    return make_pair(a, b);
}

void test_r_storage::test_r_dumbdex_consistency()
{
    const size_t MAX_INDEXES = 8;
    vector<uint8_t> buffer(sizeof(uint32_t) + (MAX_INDEXES * INDEX_ELEMENT_SIZE) + sizeof(uint32_t) + (MAX_INDEXES * FREEDEX_ELEMENT_SIZE));

    r_dumbdex::allocate(&buffer[0], buffer.size(), MAX_INDEXES);

    r_dumbdex d(&buffer[0], 8);

    auto blk = d.insert(10);
    RTF_ASSERT(blk == 1);
    blk = d.insert(20);
    RTF_ASSERT(blk == 2);
    blk = d.insert(30);
    RTF_ASSERT(blk == 3);
    blk = d.insert(40);
    RTF_ASSERT(blk == 4);
    blk = d.insert(50);
    RTF_ASSERT(blk == 5);
    blk = d.insert(60);
    RTF_ASSERT(blk == 6);

    RTF_ASSERT(
        d._verify_index_state(
            {mp(10, 1),mp(20, 2),mp(30, 3),mp(40, 4),mp(50, 5),mp(60, 6)}
        ) == true
    );

    d.remove(10);

    RTF_ASSERT(
        d._verify_index_state(
            {mp(20, 2),mp(30, 3),mp(40, 4),mp(50, 5),mp(60, 6)}
        ) == true
    );

    d.insert(35);

    RTF_ASSERT(
        d._verify_index_state(
            {mp(20, 2),mp(30, 3),mp(35, 1),mp(40, 4),mp(50, 5),mp(60, 6)}
        ) == true
    );

    d.insert(70);

    RTF_ASSERT(
        d._verify_index_state(
            {mp(20, 2),mp(30, 3),mp(35, 1),mp(40, 4),mp(50, 5),mp(60, 6),mp(70, 7)}
        ) == true
    );

    d.insert(80);

    RTF_ASSERT(
        d._verify_index_state(
            {mp(20, 2),mp(30, 3),mp(35, 1),mp(40, 4),mp(50, 5),mp(60, 6),mp(70, 7),mp(80, 8)}
        ) == true
    );

    d.insert(75);

    RTF_ASSERT(
        d._verify_index_state(
            {mp(30, 3),mp(35, 1),mp(40, 4),mp(50, 5),mp(60, 6),mp(70, 7),mp(75,2),mp(80, 8)}
        ) == true
    );

    d.insert(10);

    RTF_ASSERT(
        d._verify_index_state(
            {mp(10, 3),mp(35, 1),mp(40, 4),mp(50, 5),mp(60, 6),mp(70, 7),mp(75,2),mp(80, 8)}
        ) == true
    );

    d.insert(90);

    RTF_ASSERT(
        d._verify_index_state(
            {mp(35, 1),mp(40, 4),mp(50, 5),mp(60, 6),mp(70, 7),mp(75, 2),mp(80, 8),mp(90, 3)}
        ) == true
    );

    d.remove(90);

    RTF_ASSERT(
        d._verify_index_state(
            {mp(35, 1),mp(40, 4),mp(50, 5),mp(60, 6),mp(70, 7),mp(75, 2),mp(80, 8)}
        ) == true
    );

    d.insert(72);

    RTF_ASSERT(
        d._verify_index_state(
            {mp(35, 1),mp(40, 4),mp(50, 5),mp(60, 6),mp(70, 7),mp(72, 3),mp(75, 2),mp(80, 8)}
        ) == true
    );

    {
        auto i = d.find_lower_bound(51);

        auto p = *i;
        RTF_ASSERT(p.first == 60);

        i.next();
        i.next();

        p = *i;
        RTF_ASSERT(p.first == 72);
        RTF_ASSERT(p.second == 3);
    }

    {
        auto i = d.begin();

        auto p = *i;
        RTF_ASSERT(p.first == 35);
        RTF_ASSERT(p.second == 1);

        RTF_ASSERT(d.find_lower_bound(81) == d.end());
    }

    d.remove(80);
    d.remove(75);
    d.remove(72);
    d.remove(70);
    d.remove(60);
    d.remove(50);
    d.remove(40);
    d.remove(35);

    // freedex should look like: 7, 2, 3, 8, 6, 5, 4, 1

    d.insert(1);

    RTF_ASSERT(
        d._verify_index_state(
            {mp(1, 1)}
        ) == true
    );
}

void test_r_storage::test_r_dumbdex_size_methods()
{
    RTF_ASSERT(r_dumbdex::_index_entry_size() == r_dumbdex::_freedex_entry_size());
    RTF_ASSERT(r_dumbdex::_index_entry_size() == 10);
    RTF_ASSERT(r_dumbdex::max_indexes_within(1024) == 84);
    RTF_ASSERT(r_dumbdex::max_indexes_within(1048576) == 87380);
}

void test_r_storage::test_r_ind_block_private_getters()
{    
    std::vector<uint8_t> buffer(8192);

    r_ind_block::initialize_block(&buffer[0], buffer.size(), 16, 123456789, "flv", "your mom", "voc", "your grandmom");

    r_ind_block blk(&buffer[0], buffer.size());

    RTF_ASSERT(blk._read_n_valid_entries() == 0);
    RTF_ASSERT(blk._read_n_entries() == 16);
    RTF_ASSERT(blk._read_base_time() == 123456789);
    RTF_ASSERT(blk._read_video_codec_name() == "flv");
    RTF_ASSERT(blk._read_video_codec_parameters() == "your mom");
    RTF_ASSERT(blk._read_audio_codec_name() == "voc");
    RTF_ASSERT(blk._read_audio_codec_parameters() == "your grandmom");
}

void test_r_storage::test_r_dumbdex_full()
{
    const size_t MAX_INDEXES = 8;
    vector<uint8_t> buffer(sizeof(uint32_t) + (MAX_INDEXES * INDEX_ELEMENT_SIZE) + sizeof(uint32_t) + (MAX_INDEXES * FREEDEX_ELEMENT_SIZE));

    r_dumbdex::allocate(&buffer[0], buffer.size(), MAX_INDEXES);

    r_dumbdex d(&buffer[0], 8);

    auto blk = d.insert(1000);
    RTF_ASSERT(blk == 1);

    blk = d.insert(1010);
    RTF_ASSERT(blk == 2);

    blk = d.insert(1020);
    RTF_ASSERT(blk == 3);

    blk = d.insert(1030);
    RTF_ASSERT(blk == 4);

    blk = d.insert(1040);
    RTF_ASSERT(blk == 5);

    blk = d.insert(1050);
    RTF_ASSERT(blk == 6);

    blk = d.insert(1060);
    RTF_ASSERT(blk == 7);

    blk = d.insert(1070);
    RTF_ASSERT(blk == 8);

    // full, so get from beginning
    blk = d.insert(1090);
    RTF_ASSERT(blk == 1);

    blk = d.insert(1100);
    RTF_ASSERT(blk == 2);

    blk = d.insert(1110);
    RTF_ASSERT(blk == 3);

    blk = d.insert(1120);
    RTF_ASSERT(blk == 4);

    blk = d.insert(1130);
    RTF_ASSERT(blk == 5);

    blk = d.insert(1140);
    RTF_ASSERT(blk == 6);

    blk = d.insert(1150);
    RTF_ASSERT(blk == 7);

    blk = d.insert(1160);
    RTF_ASSERT(blk == 8);

    blk = d.insert(1170);
    RTF_ASSERT(blk == 1);
}

void test_r_storage::test_r_ind_block_basic_iteration()
{
    vector<uint8_t> buffer(1024*1024);

    r_ind_block::initialize_block(&buffer[0], buffer.size(), 8, 0, "flv", "your mom", "voc", "your grandmom");

    r_ind_block blk(&buffer[0], buffer.size());

    vector<uint8_t> input_frame_1(1024);
    iota(begin(input_frame_1), end(input_frame_1), 0);

    vector<uint8_t> input_frame_2(1024);
    iota(rbegin(input_frame_2), rend(input_frame_2), 0);

    blk.append(&input_frame_1[0], input_frame_1.size(), 0, 0, 1);
    blk.append(&input_frame_2[0], input_frame_2.size(), 0, 0, 2);
    blk.append(&input_frame_1[0], input_frame_1.size(), 0, 0, 3);
    blk.append(&input_frame_2[0], input_frame_2.size(), 0, 0, 4);
    blk.append(&input_frame_1[0], input_frame_1.size(), 0, 0, 5);
    blk.append(&input_frame_2[0], input_frame_2.size(), 0, 0, 6);
    blk.append(&input_frame_1[0], input_frame_1.size(), 0, 0, 7);
    blk.append(&input_frame_2[0], input_frame_2.size(), 0, 0, 8);

    RTF_ASSERT_THROWS(blk.append(&input_frame_1[0], input_frame_1.size(), 0, 0, 9), r_exception);

    auto i = blk.begin();

    auto f1 = *i;

    RTF_ASSERT(f1.stream_id == 0);
    RTF_ASSERT(f1.ts == 1);
    RTF_ASSERT(f1.block_size == 1024);
    RTF_ASSERT(memcmp(f1.block, &input_frame_1[0], 1024) == 0);
    RTF_ASSERT(f1.video_codec_name == "flv");
    RTF_ASSERT(f1.video_codec_parameters == "your mom");
    RTF_ASSERT(f1.audio_codec_name == "voc");
    RTF_ASSERT(f1.audio_codec_parameters == "your grandmom");

    i.next();

    auto f2 = *i;

    RTF_ASSERT(f2.stream_id == 0);
    RTF_ASSERT(f2.ts == 2);
    RTF_ASSERT(f2.block_size == 1024);
    RTF_ASSERT(memcmp(f2.block, &input_frame_2[0], 1024) == 0);
    RTF_ASSERT(f2.video_codec_name == "flv");
    RTF_ASSERT(f2.video_codec_parameters == "your mom");
    RTF_ASSERT(f2.audio_codec_name == "voc");
    RTF_ASSERT(f2.audio_codec_parameters == "your grandmom");

    i.next();

    auto f3 = *i;

    RTF_ASSERT(f3.stream_id == 0);
    RTF_ASSERT(f3.ts == 3);
    RTF_ASSERT(f3.block_size == 1024);
    RTF_ASSERT(memcmp(f3.block, &input_frame_1[0], 1024) == 0);
    RTF_ASSERT(f3.video_codec_name == "flv");
    RTF_ASSERT(f3.video_codec_parameters == "your mom");
    RTF_ASSERT(f3.audio_codec_name == "voc");
    RTF_ASSERT(f3.audio_codec_parameters == "your grandmom");

    i.next();
    i.next();
    i.next();
    i.next();
    i.prev();

    auto f4 = *i;

    RTF_ASSERT(f4.stream_id == 0);
    RTF_ASSERT(f4.ts == 6);
    RTF_ASSERT(f4.block_size == 1024);
    RTF_ASSERT(memcmp(f4.block, &input_frame_2[0], 1024) == 0);
    RTF_ASSERT(f4.video_codec_name == "flv");
    RTF_ASSERT(f4.video_codec_parameters == "your mom");
    RTF_ASSERT(f4.audio_codec_name == "voc");
    RTF_ASSERT(f4.audio_codec_parameters == "your grandmom");

    i.next();
    i.next();
    RTF_ASSERT(i.valid());
    // Note: It is valid to move to the end sentinel, but once there you are in an invalid position.
    RTF_ASSERT(i.next());
    RTF_ASSERT(i.valid()==false);
    RTF_ASSERT(i.prev());
    RTF_ASSERT(i.valid()==true);

    i = blk.begin();

    RTF_ASSERT(i.valid());
    // Note: It is invalid to invalid to move before begin().
    RTF_ASSERT(i.prev()==false);
}

void test_r_storage::test_r_ind_block_find_lower_bound()
{
    vector<uint8_t> buffer(1024*1024);

    r_ind_block::initialize_block(&buffer[0], buffer.size(), 8, 0, "flv", "your mom", "voc", "your grandmom");

    r_ind_block blk(&buffer[0], buffer.size());

    vector<uint8_t> input_frame_1(1024);
    iota(begin(input_frame_1), end(input_frame_1), 0);

    vector<uint8_t> input_frame_2(1024);
    iota(rbegin(input_frame_2), rend(input_frame_2), 0);

    blk.append(&input_frame_1[0], input_frame_1.size(), 0, 0, 10);
    blk.append(&input_frame_2[0], input_frame_2.size(), 0, 0, 20);
    blk.append(&input_frame_1[0], input_frame_1.size(), 0, 0, 30);
    blk.append(&input_frame_2[0], input_frame_2.size(), 0, 0, 40);
    blk.append(&input_frame_1[0], input_frame_1.size(), 0, 0, 50);
    blk.append(&input_frame_2[0], input_frame_2.size(), 0, 0, 60);
    blk.append(&input_frame_1[0], input_frame_1.size(), 0, 0, 70);
    blk.append(&input_frame_2[0], input_frame_2.size(), 0, 0, 80);

    // find the first element that exists...
    auto i = blk.find_lower_bound(10);

    auto f1 = *i;

    RTF_ASSERT(f1.stream_id == 0);
    RTF_ASSERT(f1.ts == 10);
    RTF_ASSERT(f1.block_size == 1024);
    RTF_ASSERT(memcmp(f1.block, &input_frame_1[0], 1024) == 0);
    RTF_ASSERT(f1.video_codec_name == "flv");
    RTF_ASSERT(f1.video_codec_parameters == "your mom");
    RTF_ASSERT(f1.audio_codec_name == "voc");
    RTF_ASSERT(f1.audio_codec_parameters == "your grandmom");

    // find the last element that exists...
    i = blk.find_lower_bound(80);

    f1 = *i;

    RTF_ASSERT(f1.stream_id == 0);
    RTF_ASSERT(f1.ts == 80);
    RTF_ASSERT(f1.block_size == 1024);
    RTF_ASSERT(memcmp(f1.block, &input_frame_2[0], 1024) == 0);
    RTF_ASSERT(f1.video_codec_name == "flv");
    RTF_ASSERT(f1.video_codec_parameters == "your mom");
    RTF_ASSERT(f1.audio_codec_name == "voc");
    RTF_ASSERT(f1.audio_codec_parameters == "your grandmom");

    // find a middle element that exists...
    i = blk.find_lower_bound(50);

    f1 = *i;

    RTF_ASSERT(f1.stream_id == 0);
    RTF_ASSERT(f1.ts == 50);
    RTF_ASSERT(f1.block_size == 1024);
    RTF_ASSERT(memcmp(f1.block, &input_frame_1[0], 1024) == 0);
    RTF_ASSERT(f1.video_codec_name == "flv");
    RTF_ASSERT(f1.video_codec_parameters == "your mom");
    RTF_ASSERT(f1.audio_codec_name == "voc");
    RTF_ASSERT(f1.audio_codec_parameters == "your grandmom");

    // find a middle element that doesnt exist...
    i = blk.find_lower_bound(45);

    f1 = *i;

    RTF_ASSERT(f1.stream_id == 0);
    RTF_ASSERT(f1.ts == 50);
    RTF_ASSERT(f1.block_size == 1024);
    RTF_ASSERT(memcmp(f1.block, &input_frame_1[0], 1024) == 0);
    RTF_ASSERT(f1.video_codec_name == "flv");
    RTF_ASSERT(f1.video_codec_parameters == "your mom");
    RTF_ASSERT(f1.audio_codec_name == "voc");
    RTF_ASSERT(f1.audio_codec_parameters == "your grandmom");

    // find an element before the first...
    i = blk.find_lower_bound(9);

    f1 = *i;

    RTF_ASSERT(f1.stream_id == 0);
    RTF_ASSERT(f1.ts == 10);
    RTF_ASSERT(f1.block_size == 1024);
    RTF_ASSERT(memcmp(f1.block, &input_frame_1[0], 1024) == 0);
    RTF_ASSERT(f1.video_codec_name == "flv");
    RTF_ASSERT(f1.video_codec_parameters == "your mom");
    RTF_ASSERT(f1.audio_codec_name == "voc");
    RTF_ASSERT(f1.audio_codec_parameters == "your grandmom");

    // find an element after the last...
    i = blk.find_lower_bound(81);
    RTF_ASSERT(i.valid() == false);
}

void test_r_storage::test_r_rel_block_append()
{
    vector<uint8_t> src(1024);
    std::iota(begin(src), end(src), 0);

    vector<uint8_t> buffer(1024*1024);

    auto dst = buffer.data();

    auto after = r_rel_block::append(dst, src.data(), src.size(), 10, 1);

    RTF_ASSERT(after - dst == src.size() + 13);

    RTF_ASSERT(*(uint64_t*)dst == 10);

    RTF_ASSERT(*(dst + 8) == 1);

    dst = after;

    after = r_rel_block::append(dst, src.data(), src.size(), 20, 0);

    RTF_ASSERT(after - dst == src.size() + 13);

    RTF_ASSERT(*(uint64_t*)dst == 20);

    RTF_ASSERT(*(dst + 8) == 0);
}

void test_r_storage::test_r_rel_block_basic_iteration()
{
    vector<uint8_t> src(1024);
    std::iota(begin(src), end(src), 0);

    vector<uint8_t> buffer(1024*1024);

    auto dst = buffer.data();

    auto after = r_rel_block::append(dst, src.data(), src.size(), 10, 1);
    dst = after;
    after = r_rel_block::append(dst, src.data(), src.size(), 20, 0);
    dst = after;
    after = r_rel_block::append(dst, src.data(), src.size(), 30, 0);
    dst = after;
    after = r_rel_block::append(dst, src.data(), src.size(), 40, 0);
    dst = after;
    after = r_rel_block::append(dst, src.data(), src.size(), 50, 1);
    dst = after;
    after = r_rel_block::append(dst, src.data(), src.size(), 60, 0);
    dst = after;
    after = r_rel_block::append(dst, src.data(), src.size(), 70, 0);
    dst = after;
    after = r_rel_block::append(dst, src.data(), src.size(), 80, 0);

    // Note: the user of this class must keep track of the new top after appends and thus is
    // maintaining all knowledge of the size.
    r_rel_block blk(buffer.data(), after - buffer.data());

    auto i = blk.begin();
    RTF_ASSERT(i.valid());

    auto f1 = *i;
    RTF_ASSERT(f1.flags == 1);
    RTF_ASSERT(f1.pts == 10);
    RTF_ASSERT(f1.size = 1024);
    RTF_ASSERT(f1.data != nullptr);

    i.next();

    f1 = *i;
    RTF_ASSERT(f1.flags == 0);
    RTF_ASSERT(f1.pts == 20);
    RTF_ASSERT(f1.size = 1024);
    RTF_ASSERT(f1.data != nullptr);

    i.next();

    f1 = *i;
    RTF_ASSERT(f1.flags == 0);
    RTF_ASSERT(f1.pts == 30);
    RTF_ASSERT(f1.size = 1024);
    RTF_ASSERT(f1.data != nullptr);

    i.next();

    f1 = *i;
    RTF_ASSERT(f1.flags == 0);
    RTF_ASSERT(f1.pts == 40);
    RTF_ASSERT(f1.size = 1024);
    RTF_ASSERT(f1.data != nullptr);

    i.next();

    f1 = *i;
    RTF_ASSERT(f1.flags == 1);
    RTF_ASSERT(f1.pts == 50);
    RTF_ASSERT(f1.size = 1024);
    RTF_ASSERT(f1.data != nullptr);

    i.next();

    f1 = *i;
    RTF_ASSERT(f1.flags == 0);
    RTF_ASSERT(f1.pts == 60);
    RTF_ASSERT(f1.size = 1024);
    RTF_ASSERT(f1.data != nullptr);

    i.next();

    f1 = *i;
    RTF_ASSERT(f1.flags == 0);
    RTF_ASSERT(f1.pts == 70);
    RTF_ASSERT(f1.size = 1024);
    RTF_ASSERT(f1.data != nullptr);

    i.next();

    f1 = *i;
    RTF_ASSERT(f1.flags == 0);
    RTF_ASSERT(f1.pts == 80);
    RTF_ASSERT(f1.size = 1024);
    RTF_ASSERT(f1.data != nullptr);

    i.prev();

    f1 = *i;
    RTF_ASSERT(f1.flags == 0);
    RTF_ASSERT(f1.pts == 70);
    RTF_ASSERT(f1.size = 1024);
    RTF_ASSERT(f1.data != nullptr);

    i.prev();

    f1 = *i;
    RTF_ASSERT(f1.flags == 0);
    RTF_ASSERT(f1.pts == 60);
    RTF_ASSERT(f1.size = 1024);
    RTF_ASSERT(f1.data != nullptr);

    i.next();
    i.next();
    i.next();

    RTF_ASSERT(i.valid() == false);
}

void test_r_storage::test_r_storage_file_basic()
{
    r_storage_file::allocate("test_file", 65536, 10);

    r_storage_file sf("test_file");

    vector<uint8_t> frame(256);
    std::iota(begin(frame), end(frame), 0);

    r_nullable<string> vp = string("vparam"), ap = string("aparam");
    auto wc = sf.create_write_context("vname", vp, "aname", ap);

    int64_t ts = 100, pts = 10;
    for(int i = 0; i < 10000; ++i)
    {
        sf.write_frame(
            wc,
            R_STORAGE_MEDIA_TYPE_VIDEO,
            frame.data(),
            frame.size() - (_random() % 32),
            (ts % 50) == 0,
            ts,
            pts
        );

        ts += 10;
        pts += 10;
    }

    auto gop_starts = sf.key_frame_start_times(R_STORAGE_MEDIA_TYPE_VIDEO);

    auto first_ts = gop_starts.front();
    auto last_ts = gop_starts.back();

    auto result = sf.query(R_STORAGE_MEDIA_TYPE_VIDEO, first_ts + 50, last_ts - 50);

    RTF_ASSERT(result.size() > 0);
}

void test_r_storage::test_r_storage_file_fake_camera()
{
    r_storage_file::allocate("ten_mb_file", 65536, 160);

    int port = RTF_NEXT_PORT();

    auto fc = _create_fc(port);

    auto fct = thread([&](){
        fc->start();
    });
    fct.detach();

    vector<r_arg> arguments;
    add_argument(arguments, "url", r_string_utils::format("rtsp://127.0.0.1:%d/true_north_h264_aac.mp4", port));

    r_gst_source src;
    src.set_args(arguments);

    string video_codec_name, audio_codec_name;
    string video_codec_parameters, audio_codec_parameters;

    r_storage_file sf;
    r_storage_write_context ctx;

    src.set_sdp_media_cb([&](const map<string, r_sdp_media>& sdp_media){
        auto v_info = sdp_media_to_s(VIDEO_MEDIA, sdp_media);
        video_codec_name = v_info.first;
        video_codec_parameters = v_info.second;
        auto a_info = sdp_media_to_s(AUDIO_MEDIA, sdp_media);
        audio_codec_name = a_info.first;
        audio_codec_parameters = a_info.second;

        printf("video_codec_name=%s\n",video_codec_name.c_str());
        printf("video_codec_parameters=%s\n",video_codec_parameters.c_str());

        sf = r_storage_file("ten_mb_file");
        ctx = sf.create_write_context(video_codec_name, video_codec_parameters, audio_codec_name, audio_codec_parameters);
    });

    double framerate = 0.0;
    src.set_pad_added_cb([&](r_media type, const r_pad_info& pad_info){
        if(type == VIDEO_MEDIA)
            framerate = pad_info.framerate.value();
    });

    src.set_video_sample_cb(
        [&](const sample_context& sc, const uint8_t* p, size_t sz, bool key, int64_t pts){
            sf.write_frame(
                ctx,
                R_STORAGE_MEDIA_TYPE_VIDEO,
                p,
                sz,
                key,
                sc.video_stream_start_ts() + pts,
                pts
            );
        }
    );

    src.play();

    std::this_thread::sleep_for(std::chrono::seconds(10));

    src.stop();

    fc->quit();

    auto kfst = sf.key_frame_start_times(R_STORAGE_MEDIA_TYPE_ALL);
    auto result = sf.query(R_STORAGE_MEDIA_TYPE_ALL, kfst.front(), kfst.back());

    uint32_t version = 0;
    auto bt = r_blob_tree::deserialize(&result[0], result.size(), version);

    auto sps = get_h264_sps(video_codec_parameters);
    auto pps = get_h264_pps(video_codec_parameters);

    auto sps_info = parse_h264_sps(sps.value());

    r_muxer muxer("output.mp4");

    muxer.add_video_stream(
        av_d2q(framerate, 10000),
        r_mux::encoding_to_av_codec_id(video_codec_name),
        sps_info.width,
        sps_info.height,
        sps_info.profile_idc,
        sps_info.level_idc
    );

    //muxer.add_audio_stream(r_mux::encoding_to_av_codec_id(audio_codec_name), 

    muxer.set_video_extradata(make_h264_extradata(sps, pps));

    muxer.open();
    
    auto n_frames = bt["frames"].size();

    int64_t dts = 0;
    for(size_t fi = 0; fi < n_frames; ++fi)
    {
        auto key = (bt["frames"][fi]["key"].get_string() == "true");
        auto frame = bt["frames"][fi]["data"].get();
        auto pts = r_string_utils::s_to_int64(bt["frames"][fi]["pts"].get_string());
        
        muxer.write_video_frame(frame.data(), frame.size(), pts, dts, {1, 1000}, key);
        ++dts;
    }

    muxer.finalize();

}
