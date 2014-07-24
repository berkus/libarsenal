//
// Part of Metta OS. Check http://atta-metta.net for latest version.
//
// Copyright 2007 - 2014, Stanislav Karchebnyy <berkus@atta-metta.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#define BOOST_TEST_MODULE Test_proquint
#include <boost/test/unit_test.hpp>

#include "arsenal/proquint.h"

BOOST_AUTO_TEST_CASE(basic_encoding)
{
    std::string input{0x7f, 0x00, 0x00, 0x01};
    BOOST_CHECK(encode::to_proquint(input) == "lusab-babad");
    BOOST_CHECK(encode::from_proquint("lusab-babad") == input);
}

BOOST_AUTO_TEST_CASE(smallest_input)
{
    std::string input{0x1,0x2};
    BOOST_CHECK(encode::to_proquint(input) == "bahaf");
    BOOST_CHECK(encode::from_proquint("bahaf") == input);
}
