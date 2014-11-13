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
#include "arsenal/hexdump.h"
#include "arsenal/subrange.h"
#include "arsenal/fusionary.h"
#include "arsenal/opaque_endian.h"

using namespace std;
using namespace boost;
using namespace sodiumpp;

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
    (box96_t, responder_cookie)
    (cnonce8_t, nonce)
    //..... variable size box
);

BOOST_FUSION_DEFINE_STRUCT(
    (sss)(channels), initiate_packet_box,
    (eckey_t, initiator_longterm_public_key)
    (cnonce16_t, nonce)
    (box48_t, vouch)
    //..... variable size data
);

BOOST_FUSION_DEFINE_STRUCT(
    (sss)(channels), message_packet_header,
    (magic::message_packet, magic)
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
        pkt.initiator_shortterm_public_key = short_term_key.pk.get();
        pkt.box = seal.box(long_term_key.pk.get()+string(32, '\0'));
        pkt.nonce = seal.nonce_sequential();

        asio::mutable_buffer out;
        write(out, pkt);

        return packet;
    }

    string got_cookie(string pkt)
    {
        assert(pkt.size() == 168);
        assert(subrange(pkt, 0, 8) == cookiePacketMagic);

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
        string initiate(256+payload.size(), '\0');

        subrange(initiate, 0, 8) = initiatePacketMagic;
        subrange(initiate, 8, 32) = short_term_key.pk.get();
        subrange(initiate, 40, 96) = cookie;

        boxer<nonce64> seal(server.short_term_key, short_term_key, initiateNoncePrefix);
        subrange(initiate, 144, 112+payload.size())
            = seal.box(long_term_key.pk.get()+vouchSeal.nonce_sequential()+vouch+payload);
        // @todo Round payload size to next or second next multiple of 16..

        subrange(initiate, 136, 8) = seal.nonce_sequential();

        return initiate;
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
        hello_packet_header hello;
        packet_reader(pkt, hello);

        string clientKey = hello.initiator_shortterm_public_key;
        string nonce = helloNoncePrefix + hello.nonce;

        // assert(pkt.size() == 192);
        // assert(subrange(pkt, 0, 8) == helloPacketMagic);

        // string clientKey = subrange(pkt, 8, 32);

        // string nonce(24, '\0');
        // subrange(nonce, 0, 16) = helloNoncePrefix;
        // subrange(nonce, 16, 8) = subrange(pkt, 104, 8);

        unboxer<recv_nonce> unseal(clientKey, long_term_key, nonce);
        string open = unseal.unbox(hello.box);
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
        initiate_packet_header init;
        tie(init, pkt) = read<initiate_packet_header>(pkt);
        // vs.
        packet_reader(pkt, init);

        // assert(subrange(pkt, 0, 8) == initiatePacketMagic);

        // Try to open the cookie
        string nonce = minuteKeyNoncePrefix + init.nonce;

        // string nonce(24, '\0');
        // subrange(nonce, 0, 8) = minuteKeyNoncePrefix;
        // subrange(nonce, 8, 16) = subrange(pkt, 40, 16);

        string cookie = crypto_secretbox_open(init.cookie, nonce, minute_key.get());

        // string cookie = crypto_secretbox_open(subrange(pkt, 56, 80), nonce, minute_key.get());

        // Check that cookie and client match
        assert(init.clientShortTermKey == subrange(cookie, 0, 32));
        // assert(subrange(pkt, 8 ,32) == subrange(cookie, 0, 32));

        // Extract server short-term secret key
        short_term_key = secret_key(public_key(""), subrange(cookie, 32, 32));

        // Open the Initiate box using both short-term keys
        string initiateNonce = initiateNoncePrefix + init.nonce;
        // string initateNonce(24, '\0');
        // subrange(initateNonce, 0, 16) = initiateNoncePrefix;
        // subrange(initateNonce, 16, 8) = subrange(pkt, 136, 8);

        // string clientShortTermKey = subrange(pkt, 8, 32);

        unboxer<recv_nonce> unseal(init.clientShortTermKey, short_term_key, initateNonce);
        string msg = unseal.unbox(init.box);
        // string msg = unseal.unbox(subrange(pkt, 144));

        // Extract client long-term public key and check the vouch subpacket.
        string clientLongTermKey = subrange(msg, 0, 32);

        string vouchNonce(24, '\0');
        subrange(vouchNonce, 0, 8) = vouchNoncePrefix;
        subrange(vouchNonce, 8, 16) = subrange(msg, 32, 16);

        unboxer<recv_nonce> vouchUnseal(clientLongTermKey, long_term_key, vouchNonce);
        string vouch = vouchUnseal.unbox(subrange(msg, 48, 48));

        assert(vouch == clientShortTermKey);

        // All is good, what's in the payload?

        string payload = subrange(msg, 96);
        hexdump(payload);
    }

    string send_message(string pkt) { return ""s; }

private:
    string send_cookie(string clientKey)
    {
        string packet(8+16+144, '\0');
        string cookie(96, '\0');
        secret_key sessionKey; // Generate short-term server key

        // Client short-term public key
        subrange(cookie, 16, 32) = clientKey;
        // Server short-term secret key
        subrange(cookie, 48, 32) = sessionKey.get();

        // minute-key secretbox nonce
        random_nonce<8> minuteKeyNonce(minuteKeyNoncePrefix);
        subrange(cookie, 16, 80)
            = crypto_secretbox(subrange(cookie, 16, 64), minuteKeyNonce.get(), minute_key.get());

        // Compressed cookie nonce
        subrange(cookie, 0, 16) = minuteKeyNonce.sequential();

        boxer<random_nonce<8>> seal(clientKey, long_term_key, cookieNoncePrefix);

        // Server short-term public key + cookie
        // Box the cookies
        string box = seal.box(sessionKey.pk.get() + cookie);
        assert(box.size() == 96+32+16);

        subrange(packet, 0, 8) = cookiePacketMagic;
        subrange(packet, 8, 16) = seal.nonce_sequential();
        subrange(packet, 24, 144) = box;

        return packet;
    }
};

int main()
{
    sss::channels::message_packet_header hdr;
    // std::cout << pretty_print(hdr) << std::endl;
    return 0;
}
int main(int argc, const char ** argv)
{
    kex_initiator client;
    kex_responder server;
    string msg;

    client.set_peer_pk(server.long_term_pk());

    try {
        msg = client.send_hello();
        hexdump(msg);
        msg = server.got_hello(msg);
        hexdump(msg);
        msg = client.got_cookie(msg);
        hexdump(msg);
        server.got_initiate(msg);
        msg = client.send_message();
        // hexdump(msg);
        msg = server.send_message(msg);
        // hexdump(msg);
    } catch(const char* e) {
        cout << "Exception: " << e << endl;
    }
}
