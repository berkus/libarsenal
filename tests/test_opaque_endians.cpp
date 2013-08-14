//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2013, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#define BOOST_TEST_MODULE Test_base32
#include <boost/test/unit_test.hpp>

#include "opaque_endian.h"

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
