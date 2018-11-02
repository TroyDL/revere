
#include "framework.h"

class test_r_utils_r_byte_ptr_rw : public test_fixture
{
public:

    RTF_FIXTURE(test_r_utils_r_byte_ptr_rw);
        TEST(test_r_utils_r_byte_ptr_rw::test_constructor);
        TEST(test_r_utils_r_byte_ptr_rw::test_get_set_ptr);
        TEST(test_r_utils_r_byte_ptr_rw::test_inc_dec);
        TEST(test_r_utils_r_byte_ptr_rw::test_pointer);
        TEST(test_r_utils_r_byte_ptr_rw::test_add_sub);
        TEST(test_r_utils_r_byte_ptr_rw::test_comparisons);
        TEST(test_r_utils_r_byte_ptr_rw::test_casting);
        TEST(test_r_utils_r_byte_ptr_rw::test_offset);
        TEST(test_r_utils_r_byte_ptr_rw::test_seek);
        TEST(test_r_utils_r_byte_ptr_rw::test_read_write);
        TEST(test_r_utils_r_byte_ptr_rw::test_bulk_write);
        TEST(test_r_utils_r_byte_ptr_rw::test_append);
    RTF_FIXTURE_END();

    virtual ~test_r_utils_r_byte_ptr_rw() throw() {}

    virtual void setup();
    virtual void teardown();

    void test_constructor();
    void test_get_set_ptr();
    void test_inc_dec();
    void test_pointer();
    void test_add_sub();
    void test_comparisons();
    void test_casting();
    void test_offset();
    void test_seek();
    void test_read_write();
    void test_bulk_write();
    void test_append();
};
