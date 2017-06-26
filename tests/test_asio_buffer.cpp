//
// Part of Metta OS. Check https://metta.systems for latest version.
//
// Copyright 2007 - 2017, Stanislav Karchebnyy <berkus@metta.systems>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#define BOOST_TEST_MODULE Test_asio_buffer
#include <boost/test/unit_test.hpp>

#include "arsenal/asio_buffer.hpp"

#include <sstream>

unsigned char data1[] = {0x00, 0xa0, 0xfe, 0x04, 0xbc, 0xad, 0x14};
unsigned char data2[] = {0x76, 0xff, 0x1f, 0x4a, 0x51, 0xda, 0xcc, 0x21, 0x1b, 0x1c, 0xee};

BOOST_AUTO_TEST_CASE(asio_output_check)
{
    {
        std::ostringstream ss;
        boost::asio::const_buffer b{data1, sizeof(data1)};
        ss << b;
        BOOST_REQUIRE_EQUAL(ss.str(), "[00 a0 fe 04 bc ad 14]");
    }
    {
        std::ostringstream ss;
        boost::asio::const_buffer b{data2, sizeof(data2)};
        ss << b;
        BOOST_REQUIRE_EQUAL(ss.str(), "[76 ff 1f 4a 51 da cc 21 1b 1c ee]");
    }
    {
        std::ostringstream ss;
        boost::asio::const_buffer b{};
        ss << b;
        BOOST_REQUIRE_EQUAL(ss.str(), "[]");
    }
    {
        std::ostringstream ss;
        boost::asio::const_buffer b{data1, sizeof(data1)};
        ss << b;
        BOOST_REQUIRE_EQUAL(ss.str(), "[00 a0 fe 04 bc ad 14]");
    }
    {
        std::ostringstream ss;
        boost::asio::mutable_buffer b{data2, sizeof(data2)};
        ss << b;
        BOOST_REQUIRE_EQUAL(ss.str(), "[76 ff 1f 4a 51 da cc 21 1b 1c ee]");
    }
    {
        std::ostringstream ss;
        boost::asio::mutable_buffer b{};
        ss << b;
        BOOST_REQUIRE_EQUAL(ss.str(), "[]");
    }
}
