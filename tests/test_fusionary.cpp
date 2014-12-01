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
#include <type_traits>
#include <sodiumpp/sodiumpp.h> // @todo Remove sodium dep?
#include "arsenal/fusionary.hpp"
#include "arsenal/hexdump.h"
#include "arsenal/subrange.h"
#include "arsenal/opaque_endian.h"

using namespace std;
using namespace boost;
using namespace sodiumpp;

namespace bufferpool {

bool locked = false;
char bufferPool[1280] = {0};

void grabBuffer(asio::mutable_buffer& out) {
    if (locked) throw logic_error("Grabbing already used buffer");
    locked = true;
    out = asio::mutable_buffer(bufferPool, sizeof(bufferPool));
}

void releaseBuffer(asio::mutable_buffer& in) {
    locked = false;
}

} // bufferpool namespace

// TODO: BIG ENDIAN!

using usid_t = std::array<uint8_t, 24>;
using eckey_t = std::array<uint8_t, 32>;
using cnonce8_t = std::array<uint8_t, 8>;
using cnonce16_t = std::array<uint8_t, 16>;
using box48_t = std::array<uint8_t, 48>;
using box64_t = std::array<uint8_t, 64>;
using box80_t = std::array<uint8_t, 80>;
using box96_t = std::array<uint8_t, 96>;
using box144_t = std::array<uint8_t, 144>;
using nonce64 = nonce<crypto_box_NONCEBYTES-8, 8>;
using nonce128 = nonce<crypto_box_NONCEBYTES-16, 16>;
using recv_nonce = source_nonce<crypto_box_NONCEBYTES>;

namespace magic {
using hello_packet = std::integral_constant<uint64_t, 0x71564e7135784c68>; // "qVNq5xLh"
using cookie_packet = std::integral_constant<uint64_t, 0x726c33416e6d786b>; // "rl3Anmxk"
using initiate_packet = std::integral_constant<uint64_t, 0x71564e7135784c69>; // "qVNq5xLi"
using message_packet = std::integral_constant<uint64_t, 0x726c337135784c6d>; // "rl3q5xLm"
}

const string helloNoncePrefix     = "cURVEcp-CLIENT-h";
const string minuteKeyNoncePrefix = "minute-k";
const string cookieNoncePrefix    = "cURVEcpk";
const string vouchNoncePrefix     = "cURVEcpv";
const string initiateNoncePrefix  = "cURVEcp-CLIENT-i";

constexpr unsigned int operator"" _bits_mask (unsigned long long bits)
{
    return (1 << bits) - 1;
}

constexpr unsigned int operator"" _bits_shift (unsigned long long bits)
{
    return bits;
}

//=================================================================================================
// Channel/packet layer
//=================================================================================================

BOOST_FUSION_DEFINE_STRUCT(
    (sss)(channels), responder_cookie,
    (cnonce16_t, nonce)
    (box80_t, box)
);

BOOST_FUSION_DEFINE_STRUCT(
    (sss)(channels), hello_packet_header,
    (magic::hello_packet, magic)
    (eckey_t, initiator_shortterm_public_key)
    (box64_t, zeros)
    (cnonce8_t, nonce)
    (box80_t, box)
);

BOOST_FUSION_DEFINE_STRUCT(
    (sss)(channels), cookie_packet_header,
    (magic::cookie_packet, magic)
    (cnonce16_t, nonce)
    (box144_t, box)
);

BOOST_FUSION_DEFINE_STRUCT(
    (sss)(channels), initiate_packet_header,
    (magic::initiate_packet, magic)
    (eckey_t, initiator_shortterm_public_key)
    (sss::channels::responder_cookie, responder_cookie)
    (cnonce8_t, nonce)
    (rest_t, box) // variable size box --v
);

BOOST_FUSION_DEFINE_STRUCT(
    (sss)(channels), initiate_packet_box,
    (eckey_t, initiator_longterm_public_key)
    (cnonce16_t, vouch_nonce)
    (box48_t, vouch)
    (rest_t, data) // variable size data containing initial frames
);

BOOST_FUSION_DEFINE_STRUCT(
    (sss)(channels), message_packet_header,
    (magic::message_packet, magic)
    (eckey_t, shortterm_public_key)
    (cnonce8_t, nonce)
    (rest_t, box) // variable size box containing message
);

//=================================================================================================
// Framing layer
//=================================================================================================

namespace sss { namespace framing {

struct uint48_t {
    uint32_t high;
    uint16_t low;
    operator uint64_t() { return uint64_t(high) << 16 | low; }
};

}}

BOOST_FUSION_ADAPT_STRUCT(
    sss::framing::uint48_t,
    (uint32_t, high)
    (uint16_t, low)
);

BOOST_FUSION_DEFINE_STRUCT(
    (sss)(framing), packet_sequence_number,
    (uint16_t, size2)
    (uint32_t, size4)
    (sss::framing::uint48_t, size6)
    (uint64_t, size8)
);

// namespace sss::framing { // -std=c++1z with SVN clang
namespace sss { namespace framing {

using packet_flag_field_t = field_flag<uint8_t>;
using version_field_t = optional_field_specification<uint16_t, field_index<0>, 0_bits_shift>;
using fec_field_t = optional_field_specification<uint8_t, field_index<0>, 1_bits_shift>;
using packet_size_t = varsize_field_wrapper<packet_sequence_number, uint64_t>;
using packet_field_t = varsize_field_specification<packet_size_t, field_index<0>,
    2_bits_mask, 2_bits_shift>;

}}

BOOST_FUSION_DEFINE_STRUCT(
    (sss)(framing), packet_header,
    (sss::framing::packet_flag_field_t, flags) // 000fssgv
    (sss::framing::version_field_t, version)
    (sss::framing::fec_field_t, fec_group)
    (sss::framing::packet_field_t, packet_sequence)
);

// BOOST_FUSION_DEFINE_STRUCT(
//     (sss)(framing), stream_frame_header,
//     (uint8_t, type)
//     (uint8_t, flags)
//     (uint32_t, stream_id)
//     (uint32_t, parent_stream_id)
//     (usid_t, usid)
//     (uint64_t, stream_offset)
//     (uint16_t, data_length)
//     // variable size data
// );

// BOOST_FUSION_DEFINE_STRUCT(
//     (sss)(framing), ack_frame_header,
//     (uint8_t, type)
//     (uint8_t, sent_entropy)
//     (uint8_t, received_entropy)
//     (uint8_t, missing_packets)
//     (uint64_t, least_unacked_packet)
//     (uint64_t, largest_observed_packet)
//     (uint32_t, largest_observed_delta_time)
//     (std::vector<uint64_t>, nacks)
// );

// BOOST_FUSION_DEFINE_STRUCT(
//     (sss)(framing), padding_frame_header,
//     (uint8_t, type)
//     (uint16_t, length)
//     // ... [length] padding data
// );

//======================
// Key exchange drivers
//======================

template <size_t N>
std::array<uint8_t, N> as_array(std::string const& s)
{
    assert(s.size() == N);
    std::array<uint8_t, N> ret;
    std::copy(s.begin(), s.end(), ret.begin());
    return ret;
}

template <size_t N>
std::string as_string(std::array<uint8_t, N> const& a)
{
    std::string ret;
    ret.resize(N);
    std::copy(a.begin(), a.end(), ret.begin());
    return ret;
}

std::string as_string(sss::channels::responder_cookie const& a)
{
    return as_string(a.nonce) + as_string(a.box);
}

std::string as_string(rest_t const& a)
{
    return a.data;
}

// Eh, clumsy.
std::string string_cast(asio::mutable_buffer const& buf, asio::mutable_buffer const& end)
{
    return string(boost::asio::buffer_cast<char const*>(buf),
        boost::asio::buffer_size(buf) - boost::asio::buffer_size(end));
}

template <typename T>
std::string make_packet(T const& pkt)
{
    asio::mutable_buffer out;
    bufferpool::grabBuffer(out);
    asio::mutable_buffer end = write(out, pkt);
    string result = string_cast(out, end);
    bufferpool::releaseBuffer(out);
    return result;
}

// Initiator sends Hello and subsequently Initiate
class kex_initiator
{
    secret_key long_term_key;
    secret_key short_term_key;
    struct server {
        string long_term_key; // == kex_responder.long_term_key.pk
        string short_term_key;
    } server;

public:
    kex_initiator()
    {}

    void set_peer_pk(string pk) { server.long_term_key = pk; }

    // Create and return a hello packet from the initiator
    string send_hello()
    {
        boxer<nonce64> seal(server.long_term_key, short_term_key, helloNoncePrefix);

        sss::channels::hello_packet_header pkt;
        pkt.initiator_shortterm_public_key = as_array<32>(short_term_key.pk.get());
        pkt.box = as_array<80>(seal.box(long_term_key.pk.get()+string(32, '\0')));
        pkt.nonce = as_array<8>(seal.nonce_sequential());

        return make_packet(pkt);
    }

    string got_cookie(string pkt)
    {
        assert(pkt.size() == 168);
        // assert(subrange(pkt, 0, 8) == cookiePacketMagic);

        // open cookie box
        string nonce(24, '\0');
        subrange(nonce, 0, 8) = cookieNoncePrefix;
        subrange(nonce, 8, 16) = subrange(pkt, 8, 16);

        unboxer<recv_nonce> unseal(server.long_term_key, short_term_key, nonce);
        string open = unseal.unbox(subrange(pkt, 24, 144));

        server.short_term_key = subrange(open, 0, 32);
        string cookie = subrange(open, 32, 96);

        // @todo Must get payload from client
        return send_initiate(cookie, "Hello, world!");
    }

    string send_message() { return ""s; } // must be in client

private:
    string send_initiate(string cookie, string payload)
    {
        // Create vouch subpacket
        boxer<random_nonce<8>> vouchSeal(server.long_term_key, long_term_key, vouchNoncePrefix);
        string vouch = vouchSeal.box(short_term_key.pk.get());
        assert(vouch.size() == 48);

        // Assemble initiate packet
        sss::channels::initiate_packet_header pkt;
        pkt.initiator_shortterm_public_key = as_array<32>(short_term_key.pk.get());
        pkt.responder_cookie.nonce = as_array<16>(subrange(cookie, 0, 16));
        pkt.responder_cookie.box = as_array<80>(subrange(cookie, 16));

        boxer<nonce64> seal(server.short_term_key, short_term_key, initiateNoncePrefix);
        pkt.box = seal.box(long_term_key.pk.get()+vouchSeal.nonce_sequential()+vouch+payload);
        // @todo Round payload size to next or second next multiple of 16..
        pkt.nonce = as_array<8>(seal.nonce_sequential());

        return make_packet(pkt);
    }
};

// Responder responds with Cookie and subsequently creates far-end session state.
class kex_responder
{
    secret_key long_term_key;
    secret_key short_term_key;
    secret_key minute_key;
    set<string> cookie_cache;

public:
    kex_responder()
    {}

    string long_term_pk() const { return long_term_key.pk.get(); }

    string got_hello(string pkt)
    {
        sss::channels::hello_packet_header hello;
        asio::const_buffer buf(pkt.data(), pkt.size());
        tie(hello, ignore) = read<sss::channels::hello_packet_header>(buf);

        string clientKey = as_string(hello.initiator_shortterm_public_key);
        string nonce = helloNoncePrefix + as_string(hello.nonce);

        // assert(pkt.size() == 192);
        // assert(subrange(pkt, 0, 8) == helloPacketMagic);

        // string clientKey = subrange(pkt, 8, 32);

        // string nonce(24, '\0');
        // subrange(nonce, 0, 16) = helloNoncePrefix;
        // subrange(nonce, 16, 8) = subrange(pkt, 104, 8);

        unboxer<recv_nonce> unseal(clientKey, long_term_key, nonce);
        string open = unseal.unbox(as_string(hello.box));
        // string open = unseal.unbox(subrange(pkt, 112, 80));

        // Open box contains client's long-term public key which we should check against:
        //  a) blacklist
        //  b) already initiated connection list

        // It could be beneficial to have asymmetric connection channels, it will ease
        // connection setup handling.
        // [2014/11/11] and we do!

        return send_cookie(clientKey);
    }

    void got_initiate(string pkt) // end of negotiation
    {
        sss::channels::initiate_packet_header init;
        asio::const_buffer buf(pkt.data(), pkt.size());
        tie(init, ignore) = read<sss::channels::initiate_packet_header>(buf);

        // assert(subrange(pkt, 0, 8) == initiatePacketMagic);

        // Try to open the cookie
        string nonce = minuteKeyNoncePrefix + as_string(init.responder_cookie.nonce);

        string cookie = crypto_secretbox_open(as_string(init.responder_cookie.box),
            nonce, minute_key.get());

        // string cookie = crypto_secretbox_open(subrange(pkt, 56, 80), nonce, minute_key.get());

        // Check that cookie and client match
        assert(as_string(init.initiator_shortterm_public_key) == string(subrange(cookie, 0, 32)));
        // assert(subrange(pkt, 8 ,32) == subrange(cookie, 0, 32));

        // Extract server short-term secret key
        short_term_key = secret_key(public_key(""), subrange(cookie, 32, 32));

        // Open the Initiate box using both short-term keys
        string initiateNonce = initiateNoncePrefix + as_string(init.nonce);

        // string clientShortTermKey = subrange(pkt, 8, 32);

        unboxer<recv_nonce> unseal(as_string(init.initiator_shortterm_public_key),
            short_term_key, initiateNonce);
        string msg = unseal.unbox(as_string(init.box));
        // string msg = unseal.unbox(subrange(pkt, 144));

        // Extract client long-term public key and check the vouch subpacket.
        string clientLongTermKey = subrange(msg, 0, 32);

        string vouchNonce = vouchNoncePrefix + string(subrange(msg, 32, 16));

        unboxer<recv_nonce> vouchUnseal(clientLongTermKey, long_term_key, vouchNonce);
        string vouch = vouchUnseal.unbox(subrange(msg, 48, 48));

        // assert(vouch == clientShortTermKey);

        // All is good, what's in the payload?

        string payload = subrange(msg, 96);
        hexdump(payload);
    }

    string send_message(string pkt) { return ""s; }

private:
    string send_cookie(string clientKey)
    {
        sss::channels::cookie_packet_header packet;
        sss::channels::responder_cookie cookie;
        secret_key sessionKey; // Generate short-term server key

        // minute-key secretbox nonce
        random_nonce<8> minuteKeyNonce(minuteKeyNoncePrefix);
        // Client short-term public key + Server short-term secret key
        cookie.box = as_array<80>(crypto_secretbox(clientKey + sessionKey.get(),
            minuteKeyNonce.get(), minute_key.get()));

        // Compressed cookie nonce
        cookie.nonce = as_array<16>(minuteKeyNonce.sequential());

        boxer<random_nonce<8>> seal(clientKey, long_term_key, cookieNoncePrefix);

        // Server short-term public key + cookie
        // Box the cookies
        string box = seal.box(sessionKey.pk.get() + as_string(cookie));
        assert(box.size() == 96+32+16);

        packet.nonce = as_array<16>(seal.nonce_sequential());
        packet.box = as_array<144>(box);

        return make_packet(packet);
    }
};

int main(int argc, const char ** argv)
{
    kex_initiator client;
    kex_responder server;
    string msg;

    client.set_peer_pk(server.long_term_pk());

    try {
        msg = client.send_hello();
        msg = server.got_hello(msg);
        msg = client.got_cookie(msg);
        server.got_initiate(msg);
        msg = client.send_message();
        msg = server.send_message(msg);
    } catch(const char* e) {
        cout << "Exception: " << e << endl;
    }
}
