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
    {
        byte_array_owrap<flurry::oarchive> write(data);
        write.archive() << nullptr << true << false;
        // << 42 << 0xdeadbeefabba << {'a','b','c','d','e'};

        int8_t i8_1 = -122;
        int8_t i8_2 = 42;
        int8_t i8_3 = 125;
        write.archive() << i8_1 << i8_2 << i8_3;

        int16_t i16_1 = -122;
        int16_t i16_2 = 42;
        int16_t i16_3 = 125;
        write.archive() << i16_1 << i16_2 << i16_3;

        uint32_t u32_1 = 42;
        uint32_t u32_2 = 140;
        uint32_t u32_3 = 16374;
        uint32_t u32_4 = 0xdeadbeef;
        write.archive() << u32_1 << u32_2 << u32_3 << u32_4;

        float pi = 3.141592;
        double pi_2 = 3.1415926;
        write.archive() << pi << pi_2;
    }
    logger::file_dump out(data);
    {
        byte_array_iwrap<flurry::iarchive> read(data);
    }
}
