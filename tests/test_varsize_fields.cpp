//
// Part of Metta OS. Check http://atta-metta.net for latest version.
//
// Copyright 2007 - 2014, Stanislav Karchebnyy <berkus@atta-metta.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#define BOOST_TEST_MODULE Test_varsize_fields
#include <boost/test/unit_test.hpp>
#include <array>
#include <iostream>
#include "arsenal/fusionary.hpp"
#include "arsenal/fusionary/pretty_print.hpp"
#include "arsenal/hexdump.h"

using namespace std;
namespace asio = boost::asio;
namespace mpl = boost::mpl;

constexpr unsigned int operator"" _bits_mask (unsigned long long bits)
{
    return (1 << bits) - 1;
}

constexpr unsigned int operator"" _bits_shift (unsigned long long bits)
{
    return bits;
}

// User-defined mapping function (switcher bits value to type)
// based on the value of some field we must choose N-th value in this struct and read it
// index 0 - bits value 0, index 1 - bits value 1 and so on
// use a sentinel type nothing_t to read nothing.
// @todo When nothing_t is present we should be using optional<V> for output value?
BOOST_FUSION_DEFINE_STRUCT(
    (mapping), twobits,
    (nothing_t, no_value)
    (uint16_t, value1)
    (uint32_t, value2)
    (uint64_t, value3)
);

using flag_field_t = field_flag<uint8_t>;
using version_field_t = optional_field_specification<uint32_t, field_index<0>, 0_bits_shift>;
using packet_size_t = varsize_field_wrapper<mapping::twobits, uint64_t>;
using packet_field_t = varsize_field_specification<packet_size_t, field_index<0>, 2_bits_mask, 2_bits_shift>;
using packet_field2_t = varsize_field_specification<packet_size_t, field_index<3>, 2_bits_mask, 2_bits_shift>;

BOOST_FUSION_DEFINE_STRUCT(
    (actual), header_type,
    (flag_field_t, flags)
    (packet_field_t, packet_size)
    (version_field_t, version)
);

BOOST_FUSION_DEFINE_STRUCT(
    (actual), big_header_type,
    (flag_field_t, flags)
    (version_field_t, version)
    (packet_field_t, packet_size)
    (flag_field_t, flags2)
    (packet_field2_t, packet_size2)
);

BOOST_FUSION_DEFINE_STRUCT(
    (actual), packet_type,
    (actual::header_type, header1)
    (actual::big_header_type, header23)
    (actual::header_type, header4)
    (actual::header_type, header5)
    (rest_t, body)
);

std::array<uint8_t,36> buffer  = {{ // 2 byte packet size only
                                    0x04, 0xab, 0xcd,
                                    // version only
                                    0x01, 0x03, 0x02, 0x01, 0x00,
                                    // 4 byte packet size only
                                    0x08, 0xab, 0xcd, 0xef, 0x12,
                                    // 8 byte packet size only
                                    0x0c, 0xab, 0xcd, 0xef, 0x12, 0x34, 0x56, 0x78, 0x9a,
                                    // 4 byte packet size and version
                                    0x09, 0xef, 0xcd, 0xab, 0x01, 0x03, 0x02, 0x01, 0x00,
                                    'H', 'e', 'l', 'l', 'o' }};

BOOST_AUTO_TEST_CASE(basic_reader)
{
    asio::const_buffer buf(buffer.data(), buffer.size());
    reader read_(std::move(buf));
    actual::packet_type packet;

    read_(packet);

    BOOST_CHECK(packet.header1.packet_size.value.value() == 0xcdab);
    BOOST_CHECK(*packet.header23.version == 0x10203);
    BOOST_CHECK(packet.header23.packet_size.value.value() == 0);
    BOOST_CHECK(packet.header23.packet_size2.value.value() == 0x12efcdab);
    BOOST_CHECK(packet.header4.packet_size.value.value() == 0x9a78563412efcdab);
    BOOST_CHECK(packet.header5.packet_size.value.value() == 0x1abcdef);
    BOOST_CHECK(*packet.header5.version == 0x10203);
    BOOST_CHECK(packet.body.data == "Hello");
}
