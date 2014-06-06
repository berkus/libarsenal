//
// Part of Metta OS. Check http://atta-metta.net for latest version.
//
// Copyright 2007 - 2014, Stanislav Karchebnyy <berkus@atta-metta.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#define BOOST_TEST_MODULE Test_opaque_endians
#include <boost/test/unit_test.hpp>

#include "arsenal/opaque_endian.h"

BOOST_AUTO_TEST_CASE(basic_assignments)
{
    little_int32_t little;
    big_int32_t big;
    int32_t native;

    little = 0xdeadbeef;
    big = 0xdeadbeef;
    native = 0xdeadbeef;

    BOOST_CHECK(little == big);
    BOOST_CHECK(little == native);
    BOOST_CHECK(big == native);
}

BOOST_AUTO_TEST_CASE(memory_layout)
{
    little_int32_t little;
    big_int32_t big;
    int32_t native;

    little = 0xdeadbeef;
    big = 0xdeadbeef;
    native = 0xdeadbeef;

    uint8_t* a = (uint8_t*)&big;
    uint8_t* b = (((uint8_t*)&big)+1);
    uint8_t* c = (((uint8_t*)&big)+2);
    uint8_t* d = (((uint8_t*)&big)+3);

    BOOST_CHECK(*a == 0xde);
    BOOST_CHECK(*b == 0xad);
    BOOST_CHECK(*c == 0xbe);
    BOOST_CHECK(*d == 0xef);

    a = (uint8_t*)&little;
    b = (((uint8_t*)&little)+1);
    c = (((uint8_t*)&little)+2);
    d = (((uint8_t*)&little)+3);

    BOOST_CHECK(*a == 0xef);
    BOOST_CHECK(*b == 0xbe);
    BOOST_CHECK(*c == 0xad);
    BOOST_CHECK(*d == 0xde);
}
