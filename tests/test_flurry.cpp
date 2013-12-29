//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2013, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#define BOOST_TEST_MODULE Test_flurry_serialization
#include <boost/test/unit_test.hpp>

#include "flurry.h"
#include "logging.h"
#include "byte_array_wrap.h"

using namespace std;

BOOST_AUTO_TEST_CASE(serialize_basic_types)
{
    byte_array data;
    int8_t out_i8_1, in_i8_1 = -122;
    int8_t out_i8_2, in_i8_2 = 42;
    int8_t out_i8_3, in_i8_3 = 140;
    int16_t out_i16_1, in_i16_1 = -122;
    int16_t out_i16_2, in_i16_2 = 42;
    int16_t out_i16_3, in_i16_3 = 140;
    int16_t out_i16_4, in_i16_4 = 16374;
    uint32_t out_u32_1, in_u32_1 = 42;
    uint32_t out_u32_2, in_u32_2 = 140;
    uint32_t out_u32_3, in_u32_3 = 16374;
    uint32_t out_u32_4, in_u32_4 = 0xdeadbeef;
    int64_t out_i64_1, in_i64_1 {0xdeadbeefabba};
    float out_pi_1, in_pi_1 {3.141592};
    double out_pi_2, in_pi_2 {3.1415926};
    byte_array out_ba, in_ba {'a','b','c','d','e'};
    string out_str, in_str {"Testing testing one two 3!"};
    std::vector<int> out_vec_1, in_vec_1 {99,98,97,96,95,94};
    enum class Testing : int {
        CHECK=1,
        UNCHECK=2
    } out_e_1, out_e_2, in_e_1 {Testing::CHECK}, in_e_2 {Testing::UNCHECK};
    bool t{false}, f{true};

    {
        byte_array_owrap<flurry::oarchive> write(data);

        write.archive() << true << false;
        write.archive() << in_i8_1 << in_i8_2 << in_i8_3;
        write.archive() << in_i16_1 << in_i16_2 << in_i16_3 << in_i16_4;
        write.archive() << in_u32_1 << in_u32_2 << in_u32_3 << in_u32_4;
        write.archive() << in_i64_1;
        write.archive() << in_pi_1 << in_pi_2;
        write.archive() << in_ba << in_str;
        write.archive() << in_e_1 << in_e_2;
        write.archive() << in_vec_1;
    }
    logger::file_dump(data, "flurry test");
    {
        byte_array_iwrap<flurry::iarchive> read(data);
        read.archive() >> t >> f;
        read.archive() >> out_i8_1 >> out_i8_2 >> out_i8_3;
        read.archive() >> out_i16_1 >> out_i16_2 >> out_i16_3 >> out_i16_4;
        read.archive() >> out_u32_1 >> out_u32_2 >> out_u32_3 >> out_u32_4;
        read.archive() >> out_i64_1;
        read.archive() >> out_pi_1 >> out_pi_2;
        read.archive() >> out_ba >> out_str;
        read.archive() >> out_e_1 >> out_e_2;
        read.archive() >> out_vec_1;
    }

    BOOST_CHECK(t == true);
    BOOST_CHECK(f == false);
    BOOST_CHECK(out_i8_1 == in_i8_1);
    BOOST_CHECK(out_i8_2 == in_i8_2);
    BOOST_CHECK(out_i8_3 == in_i8_3);
    BOOST_CHECK(out_i16_1 == in_i16_1);
    BOOST_CHECK(out_i16_2 == in_i16_2);
    BOOST_CHECK(out_i16_3 == in_i16_3);
    BOOST_CHECK(out_i16_4 == in_i16_4);
    BOOST_CHECK(out_u32_1 == in_u32_1);
    BOOST_CHECK(out_u32_2 == in_u32_2);
    BOOST_CHECK(out_u32_3 == in_u32_3);
    BOOST_CHECK(out_u32_4 == in_u32_4);
    BOOST_CHECK(out_i64_1 == in_i64_1);
    BOOST_CHECK(out_pi_1 == in_pi_1);
    BOOST_CHECK(out_pi_2 == in_pi_2);
    BOOST_CHECK(out_ba == in_ba);
    BOOST_CHECK(out_str == in_str);
    BOOST_CHECK(out_e_1 == in_e_1);
    BOOST_CHECK(out_e_2 == in_e_2);
    BOOST_CHECK(out_vec_1 == in_vec_1);
}

BOOST_AUTO_TEST_CASE(serialize_mismatch)
{
    byte_array data;
    int8_t in_i8_1 = -122;
    uint8_t out_u8_1;
    {
        byte_array_owrap<flurry::oarchive> write(data);
        write.archive() << in_i8_1;
    }
    BOOST_CHECK_THROW({
        byte_array_iwrap<flurry::iarchive> read(data);
        read.archive() >> out_u8_1;
    }, flurry::decode_error);
}
