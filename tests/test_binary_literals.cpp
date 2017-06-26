//
// Part of Metta OS. Check https://metta.systems for latest version.
//
// Copyright 2007 - 2017, Stanislav Karchebnyy <berkus@metta.systems>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#define BOOST_TEST_MODULE Test_binary_literals
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(equivalence_check)
{
    BOOST_CHECK(0b00000001 == 1);
    BOOST_CHECK(0b10000000 == 128);
    BOOST_CHECK(0b1000000000000001 == 32769);
}
