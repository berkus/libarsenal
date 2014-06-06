//
// Part of Metta OS. Check http://atta-metta.net for latest version.
//
// Copyright 2007 - 2014, Stanislav Karchebnyy <berkus@atta-metta.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#define BOOST_TEST_MODULE Test_binary_literals
#include <boost/test/unit_test.hpp>

#include "arsenal/binary_literal.h"

BOOST_AUTO_TEST_CASE(equivalence_check)
{
    BOOST_CHECK(00000001_b == 1);
    BOOST_CHECK(10000000_b == 128);
    BOOST_CHECK(1000000000000001_b == 32769);
}
