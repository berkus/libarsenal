//
// Part of Metta OS. Check https://metta.systems for latest version.
//
// Copyright 2007 - 2017, Stanislav Karchebnyy <berkus@metta.systems>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#define BOOST_TEST_MODULE Test_key_message_serialization
#include <fstream>
#include <boost/test/unit_test.hpp>
#include <boost/log/trivial.hpp>
#include "arsenal/byte_array.h"
#include "arsenal/byte_array_wrap.h"
#include "arsenal/flurry.h"
#include "arsenal/file_dump.h"

using namespace std;
using namespace arsenal;

BOOST_AUTO_TEST_CASE(serialize_msgpack_types)
{
    byte_array data;
    {
        byte_array_owrap<flurry::oarchive> write(data);
        write.archive() << true << false << 42 << 0xdeadbeefabba
                        << byte_array({'a', 'b', 'c', 'd', 'e'});
    }
    logger::file_dump(data, "serialization test");
}

// @todo
BOOST_AUTO_TEST_CASE(serialize_and_deserialize)
{
    byte_array data;// = generate_dh1_chunk();

    logger::file_dump(data, "deserialization test");

    {
        // sss::negotiation::key_message m;
        // byte_array_iwrap<flurry::iarchive> read(data);

        // BOOST_CHECK(data.size() == 0xbc);

        // read.archive() >> m;

        // BOOST_CHECK(m.magic == sss::stream_protocol::magic_id);
        // BOOST_CHECK(m.chunks.size() == 2);
        // BOOST_CHECK(m.chunks[0].type == sss::negotiation::key_chunk_type::dh_init1);
        // BOOST_CHECK(m.chunks[0].dh_init1.is_initialized());
        // BOOST_CHECK(m.chunks[0].dh_init1->group ==
        // sss::negotiation::dh_group_type::dh_group_1024);
        // BOOST_CHECK(m.chunks[0].dh_init1->key_min_length = 0x10);

        // BOOST_CHECK(m.chunks[0].dh_init1->initiator_hashed_nonce.size() == 32);
        // BOOST_CHECK(m.chunks[0].dh_init1->initiator_dh_public_key.size() == 128);

        // for (int i = 0; i < 32; ++i) {
        //     BOOST_CHECK(uint8_t(m.chunks[0].dh_init1->initiator_hashed_nonce[i]) == i);
        // }

        // for (int i = 0; i < 128; ++i) {
        //     BOOST_CHECK(uint8_t(m.chunks[0].dh_init1->initiator_dh_public_key[i]) == 255 - i);
        // }
    }
}
