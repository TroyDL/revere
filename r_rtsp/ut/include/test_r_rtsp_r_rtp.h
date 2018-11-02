
#include "framework.h"

class test_r_rtsp_r_rtp : public test_fixture
{
public:

    RTF_FIXTURE(test_r_rtsp_r_rtp);
      TEST(test_r_rtsp_r_rtp::test_parse_sequence);
      TEST(test_r_rtsp_r_rtp::test_parse_marker);
      TEST(test_r_rtsp_r_rtp::test_parse_timestamp);
      TEST(test_r_rtsp_r_rtp::test_parse_payload);
    RTF_FIXTURE_END();

    virtual ~test_r_rtsp_r_rtp() throw() {}

    virtual void setup();
    virtual void teardown();

    void test_parse_sequence();
    void test_parse_marker();
    void test_parse_timestamp();
    void test_parse_payload();
};
