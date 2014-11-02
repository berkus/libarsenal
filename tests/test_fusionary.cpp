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
using cnonce8_t = std::array<uint8_t, 8>;
using cnonce16_t = std::array<uint8_t, 16>;

BOOST_FUSION_DEFINE_STRUCT(
    (sss)(channels), responder_cookie,
    (cnonce16_t, nonce)
    (std::array<uint8_t, 80>, box)
);

BOOST_FUSION_DEFINE_STRUCT(
    (sss)(channels), hello_packet_header,
    (magic_t, magic)
    (eckey_t, initiator_shortterm_public_key)
    (std::array<uint8_t, 64>, zeros)
    (cnonce8_t, nonce)
    (std::array<uint8_t, 80>, box)
);

BOOST_FUSION_DEFINE_STRUCT(
    (sss)(channels), cookie_packet_header,
    (magic_t, magic)
    (cnonce16_t, nonce)
    (std::array<uint8_t, 144>, box)
);

BOOST_FUSION_DEFINE_STRUCT(
    (sss)(channels), initiate_packet_header,
    (magic_t, magic)
    (eckey_t, initiator_shortterm_public_key)
    (std::array<uint8_t, 96>, responder_cookie)
    (cnonce8_t, nonce)
    //..... variable size box
);

BOOST_FUSION_DEFINE_STRUCT(
    (sss)(channels), initiate_packet_box,
    (eckey_t, initiator_longterm_public_key)
    (cnonce16_t, nonce)
    (std::array<uint8_t, 48>, vouch)
    //..... variable size data
);

BOOST_FUSION_DEFINE_STRUCT(
    (sss)(channels), message_packet_header,
    (magic_t, magic)
    (eckey_t, public_key)
    (cnonce8_t, nonce)
    //..... variable size box
);

BOOST_FUSION_DEFINE_STRUCT(
    (sss)(channels), packet_header,
    (uint8_t, flags)
    (uint16_t, version) // @todo mark optional
    (uint8_t, fec_group) // @todo mark optional
    (uint64_t, packet_sequence) // @todo mark optional, variable size
);

BOOST_FUSION_DEFINE_STRUCT(
    (sss)(channels), stream_frame_header,
    (uint8_t, type)
    (uint8_t, flags)
    (uint32_t, stream_id)
    (uint32_t, parent_stream_id)
    (usid_t, usid)
    (uint64_t, stream_offset)
    (uint16_t, data_length)
    // variable size data
);

BOOST_FUSION_DEFINE_STRUCT(
    (sss)(channels), ack_frame_header,
    (uint8_t, type)
    (uint8_t, sent_entropy)
    (uint8_t, received_entropy)
    (uint8_t, missing_packets)
    (uint64_t, least_unacked_packet)
    (uint64_t, largest_observed_packet)
    (uint32_t, largest_observed_delta_time)
    (std::vector<uint64_t>, nacks)
);

BOOST_FUSION_DEFINE_STRUCT(
    (sss)(channels), padding_frame_header,
    (uint8_t, type)
    (uint16_t, length)
    // ... [length] padding data
);

int main()
{
    sss::channels::message_packet_header hdr;
    // std::cout << pretty_print(hdr) << std::endl;
    return 0;
}
