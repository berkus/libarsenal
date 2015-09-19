//
// Part of Metta OS. Check http://atta-metta.net for latest version.
//
// Copyright 2007 - 2015, Stanislav Karchebnyy <berkus@atta-metta.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include <array>
#include <iostream>
#include <type_traits>
#include "sodiumpp/sodiumpp.h" // @todo Remove sodium dep?
#include "arsenal/fusionary.hpp"
#include "arsenal/hexdump.h"
#include "arsenal/subrange.h"
#include "arsenal/opaque_endian.h"
#include "sss/framing/packet_format.h" // @fixme deps
#include "sss/framing/frame_format.h"  // @fixme deps
#include "sss/framing/frames_reader.h" // @fixme deps

using namespace std;
using namespace boost;
using namespace sodiumpp;

namespace bufferpool {

bool locked           = false;
char bufferPool[1280] = {0};

void
grabBuffer(asio::mutable_buffer& out)
{
    if (locked)
        throw logic_error("Grabbing already used buffer");
    locked = true;
    out    = asio::mutable_buffer(bufferPool, sizeof(bufferPool));
}

void
releaseBuffer(asio::mutable_buffer& in)
{
    locked = false;
}

} // bufferpool namespace

//======================
// Key exchange drivers
//======================

// Eh, clumsy.
std::string
string_cast(asio::mutable_buffer const& buf, asio::mutable_buffer const& end)
{
    return string(boost::asio::buffer_cast<char const*>(buf),
                  boost::asio::buffer_size(buf) - boost::asio::buffer_size(end));
}

template <typename T>
std::string
make_packet(T const& pkt)
{
    asio::mutable_buffer out;
    bufferpool::grabBuffer(out);
    asio::mutable_buffer end = fusionary::write(out, pkt);
    string result = string_cast(out, end);
    bufferpool::releaseBuffer(out);
    return result;
}

// Initiator sends Hello and subsequently Initiate
class kex_initiator
{
    secret_key long_term_key;
    secret_key short_term_key;
    struct server
    {
        string long_term_key; // == kex_responder.long_term_key.pk
        string short_term_key;
    } server;

public:
    kex_initiator() {}

    void set_peer_pk(string pk) { server.long_term_key = pk; }

    // Create and return a hello packet from the initiator
    string send_hello()
    {
        boxer<nonce64> seal(server.long_term_key, short_term_key, HELLO_NONCE_PREFIX);

        sss::channels::hello_packet_header pkt;
        pkt.initiator_shortterm_public_key = as_array<32>(short_term_key.pk.get());
        pkt.box                            = as_array<80>(seal.box(long_term_key.pk.get() + string(32, '\0')));
        pkt.nonce                          = as_array<8>(seal.nonce_sequential());

        return make_packet(pkt);
    }

    string got_cookie(string pkt, string reply)
    {
        sss::channels::cookie_packet_header cookie;
        asio::const_buffer buf(pkt.data(), pkt.size());
        fusionary::read(cookie, buf);

        // open cookie box
        string nonce = COOKIE_NONCE_PREFIX + as_string(cookie.nonce);

        unboxer<recv_nonce> unseal(server.long_term_key, short_term_key, nonce);
        string open = unseal.unbox(as_string(cookie.box));

        // cout << "Opened COOKIE box:" << endl;
        // hexdump(open);

        server.short_term_key = subrange(open, 0, 32);
        string cookie_buf     = subrange(open, 32, 96);

        return send_initiate(cookie_buf, reply);
    }

    string send_message(string payload)
    {
        boxer<nonce64> seal(server.short_term_key, short_term_key, MESSAGE_NONCE_PREFIX);

        sss::channels::message_packet_header pkt;
        pkt.shortterm_public_key = as_array<32>(short_term_key.pk.get());
        pkt.box                  = seal.box(payload);
        pkt.nonce                = as_array<8>(seal.nonce_sequential());

        return make_packet(pkt);
    }

    void got_message(string pkt)
    {
        sss::channels::message_packet_header msg;
        asio::const_buffer buf(pkt.data(), pkt.size());
        buf = fusionary::read(msg, buf);

        string nonce = MESSAGE_NONCE_PREFIX + as_string(msg.nonce);
        unboxer<recv_nonce> unseal(as_string(msg.shortterm_public_key), short_term_key, nonce);

        string payload = unseal.unbox(msg.box.data);
        cout << "Got from server:" << endl;
        hexdump(payload);
    }

private:
    string send_initiate(string cookie, string payload)
    {
        // Create vouch subpacket
        boxer<random_nonce<8>> vouchSeal(server.long_term_key, long_term_key, VOUCH_NONCE_PREFIX);
        string vouch = vouchSeal.box(short_term_key.pk.get());
        assert(vouch.size() == 48);

        // Assemble initiate packet
        sss::channels::initiate_packet_header pkt;
        pkt.initiator_shortterm_public_key = as_array<32>(short_term_key.pk.get());
        pkt.responder_cookie.nonce         = as_array<16>(subrange(cookie, 0, 16));
        pkt.responder_cookie.box           = as_array<80>(subrange(cookie, 16));

        boxer<nonce64> seal(server.short_term_key, short_term_key, INITIATE_NONCE_PREFIX);
        pkt.box = seal.box(long_term_key.pk.get() + vouchSeal.nonce_sequential() + vouch + payload);
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
    struct client
    {
        string short_term_key;
    } client;
    std::string fixmeNeedToRebuildSessionPk;

public:
    kex_responder() {}

    string long_term_pk() const { return long_term_key.pk.get(); }

    string got_hello(string pkt)
    {
        sss::channels::hello_packet_header hello;
        asio::const_buffer buf(pkt.data(), pkt.size());
        fusionary::read(hello, buf);

        string clientKey = as_string(hello.initiator_shortterm_public_key);
        string nonce     = HELLO_NONCE_PREFIX + as_string(hello.nonce);

        unboxer<recv_nonce> unseal(clientKey, long_term_key, nonce);
        string open = unseal.unbox(as_string(hello.box));

        // cout << "Opened HELLO box:" << endl;
        // hexdump(open);

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
        buf = fusionary::read(init, buf);

        // Try to open the cookie
        string nonce = MINUTEKEY_NONCE_PREFIX + as_string(init.responder_cookie.nonce);

        string cookie =
            crypto_secretbox_open(as_string(init.responder_cookie.box), nonce, minute_key.get());

        // cout << "Opened INITIATE cookie box:" << endl;
        // hexdump(cookie);

        // Check that cookie and client match
        assert(as_string(init.initiator_shortterm_public_key) == string(subrange(cookie, 0, 32)));

        // Extract server short-term secret key
        short_term_key =
            secret_key(public_key(fixmeNeedToRebuildSessionPk), subrange(cookie, 32, 32));
        // cout << "Constructed server short term key:" << endl
        //      << short_term_key << endl
        //      << "Client short term public key:" << endl
        //      << init.initiator_shortterm_public_key << endl;

        // Open the Initiate box using both short-term keys
        string initiateNonce = INITIATE_NONCE_PREFIX + as_string(init.nonce);

        unboxer<recv_nonce> unseal(
            as_string(init.initiator_shortterm_public_key), short_term_key, initiateNonce);
        string msg = unseal.unbox(as_string(init.box));

        // cout << "Opened INITIATE msg box:" << endl;
        // hexdump(msg);

        // Extract client long-term public key and check the vouch subpacket.
        string clientLongTermKey = subrange(msg, 0, 32);

        string vouchNonce = VOUCH_NONCE_PREFIX + string(subrange(msg, 32, 16));

        unboxer<recv_nonce> vouchUnseal(clientLongTermKey, long_term_key, vouchNonce);
        string vouch = vouchUnseal.unbox(subrange(msg, 48, 48));

        assert(vouch == as_string(init.initiator_shortterm_public_key));

        client.short_term_key = vouch;

        // All is good, what's in the payload?

        string payload = subrange(msg, 96);

        cout << "Opened INITIATE msg payload:" << endl;
        hexdump(payload);

        // @todo Read payload using framing layer.
        frames_reader r(payload);
        r.read_packet_header();
        r.read_frame_header();
    }

    string send_message(string payload)
    {
        boxer<nonce64> seal(client.short_term_key, short_term_key, MESSAGE_NONCE_PREFIX);

        sss::channels::message_packet_header pkt;
        pkt.shortterm_public_key = as_array<32>(fixmeNeedToRebuildSessionPk);
        pkt.box                  = seal.box(payload);
        pkt.nonce                = as_array<8>(seal.nonce_sequential());

        return make_packet(pkt);
    }

    void got_message(string pkt)
    {
        sss::channels::message_packet_header msg;
        asio::const_buffer buf(pkt.data(), pkt.size());
        buf = fusionary::read(msg, buf);

        string nonce = MESSAGE_NONCE_PREFIX + as_string(msg.nonce);
        unboxer<recv_nonce> unseal(as_string(msg.shortterm_public_key), short_term_key, nonce);

        string payload = unseal.unbox(msg.box.data);
        cout << "Got from client:" << endl;
        hexdump(payload);
    }

private:
    string send_cookie(string clientKey)
    {
        sss::channels::cookie_packet_header packet;
        sss::channels::responder_cookie cookie;
        secret_key sessionKey; // Generate short-term server key

        fixmeNeedToRebuildSessionPk = sessionKey.pk.get();

        // minute-key secretbox nonce
        random_nonce<8> minuteKeyNonce(MINUTEKEY_NONCE_PREFIX);
        // Client short-term public key + Server short-term secret key
        cookie.box = as_array<80>(
            crypto_secretbox(clientKey + sessionKey.get(), minuteKeyNonce.get(), minute_key.get()));

        // Compressed cookie nonce
        cookie.nonce = as_array<16>(minuteKeyNonce.sequential());

        boxer<random_nonce<8>> seal(clientKey, long_term_key, COOKIE_NONCE_PREFIX);

        // Server short-term public key + cookie
        // Box the cookies
        packet.box   = as_array<144>(seal.box(sessionKey.pk.get() + as_string(cookie)));
        packet.nonce = as_array<16>(seal.nonce_sequential());

        return make_packet(packet);
    }
};

int
main(int argc, const char** argv)
{
    kex_initiator client;
    kex_responder server;
    string msg;

    client.set_peer_pk(server.long_term_pk());

    try {
        cout << "Exchange start." << endl;
        msg = client.send_hello();
        // cout << "HELLO packet:" << endl;
        // hexdump(msg);
        msg = server.got_hello(msg);
        // cout << "COOKIE packet:" << endl;
        // hexdump(msg);

        sss::framing::packet_header hdr;
        hdr.version = 1; // should set flag bit
        // Specialize below type writing rules to pick shortest representation for size field.
        hdr.packet_sequence.value.choice_.size4 = 0x12345678; // should set flag type bits

        // write(hdr, buf);

        std::array<uint8_t, 5> frame = {{0b00000001, 0x12, 0x34, 0x56, 0x78}};
        msg                          = client.got_cookie(msg, as_string(frame));

        // cout << "INITIATE packet:" << endl;
        // hexdump(msg);
        server.got_initiate(msg);
        msg = client.send_message("Hello, server!");
        // cout << "Client MESSAGE packet:" << endl;
        // hexdump(msg);
        server.got_message(msg);
        msg = server.send_message("Good day to you, client!");
        // cout << "Server MESSAGE packet:" << endl;
        // hexdump(msg);
        client.got_message(msg);
        cout << "Exchange done." << endl;
    } catch (const char* e) {
        cout << "Exception: " << e << endl;
    } catch (std::exception& ex) {
        cout << "Exception: " << ex.what() << endl;
    }
}
