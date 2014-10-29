//
// Part of Metta OS. Check http://atta-metta.net for latest version.
//
// Copyright 2007 - 2014, Stanislav Karchebnyy <berkus@atta-metta.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include <array>
#include <iostream>
#include "arsenal/fusionary.h"
#include "arsenal/opaque_endian.h"

using usid_t = std::array<uint8_t, 24>;
using eckey_t = std::array<uint8_t, 32>;
using magic_t = std::integral_constant<uint64_t, 0xf00d>; // packet magic
//using version_t = std::integral_constant<uint64_t, 0xbeef>;

namespace sss {
namespace negotiation {

enum class msg_type_t {
    type_1,
    type_2,
    type_3
};

} // negotiation namespace
} // sss namespace

BOOST_FUSION_DEFINE_STRUCT(
    (sss)(negotiation), message_packet_header,
    (magic_t, magic)
    (eckey_t, public_key)
    (uint64_t, compresed_nonce)
)

// BOOST_FUSION_DEFINE_STRUCT(
//     (sss)(negotiation), stream_frame_header,
//     (uint8_t,        flags)
//     (big_uint32_t,   stream_id)
//     (sss::negotiation::msg_type_t,     msg_type)
//     // optionals
//     (big_uint32_t,   parent_stream_id)
//     (usid_t, usid)
//     (big_uint64_t,   offset)
//     (big_uint16_t,   data_length)
// )

int main()
{
    sss::negotiation::message_packet_header hdr;
    // std::cout << pretty_print(hdr) << std::endl;
    return 0;
}
