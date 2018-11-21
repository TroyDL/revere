
#include "framework.h"

class test_r_av_r_demuxer : public test_fixture
{
public:

    RTF_FIXTURE(test_r_av_r_demuxer);
      TEST(test_r_av_r_demuxer::test_constructor);
      TEST(test_r_av_r_demuxer::test_examine_file);
      TEST(test_r_av_r_demuxer::test_file_from_memory);
      TEST(test_r_av_r_demuxer::test_get_container_statistics);
      TEST(test_r_av_r_demuxer::test_multi_stream_mp4);
      TEST(test_r_av_r_demuxer::test_move);
      TEST(test_r_av_r_demuxer::test_pts);
    RTF_FIXTURE_END();

    virtual ~test_r_av_r_demuxer() throw() {}

    virtual void setup();
    virtual void teardown();

    void test_constructor();
    void test_examine_file();
    void test_file_from_memory();
    void test_get_container_statistics();
    void test_multi_stream_mp4();
    void test_move();
    void test_pts();
};
