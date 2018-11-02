
#include "framework.h"
#include "r_utils/r_udp_receiver.h"
#include <memory>

class test_r_utils_r_udp_receiver : public test_fixture
{
public:

    RTF_FIXTURE(test_r_utils_r_udp_receiver);
        TEST(test_r_utils_r_udp_receiver::test_send);
        TEST(test_r_utils_r_udp_receiver::test_send_multicast);
        TEST(test_r_utils_r_udp_receiver::test_associated_send);
        TEST(test_r_utils_r_udp_receiver::test_get_set_recv_buffer_size);
        TEST(test_r_utils_r_udp_receiver::test_shutdown_close_while_blocked);
        TEST(test_r_utils_r_udp_receiver::test_raw_recv);
        TEST(test_r_utils_r_udp_receiver::test_shutdown_close_while_blocked_in_raw_recv);
    RTF_FIXTURE_END();

    virtual ~test_r_utils_r_udp_receiver() throw() {}

    virtual void setup();
    virtual void teardown();

    void test_send();
    void test_send_multicast();
    void test_associated_send();
    void test_get_set_recv_buffer_size();
    void test_shutdown_close_while_blocked();
    void test_raw_recv();
    void test_shutdown_close_while_blocked_in_raw_recv();

private:
    int _val;
    std::string _recvAddress;
    std::string _interfaceAddress;
    std::shared_ptr<r_utils::r_udp_receiver> _receiver;
};
