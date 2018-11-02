
#include "framework.h"

class test_r_rtsp_r_fake_camera : public test_fixture
{
public:

    RTF_FIXTURE(test_r_rtsp_r_fake_camera);
      TEST(test_r_rtsp_r_fake_camera::test_start_stop);
    RTF_FIXTURE_END();

    virtual ~test_r_rtsp_r_fake_camera() throw() {}

    virtual void setup();
    virtual void teardown();

    void test_start_stop();
};
