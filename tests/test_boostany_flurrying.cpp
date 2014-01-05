//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2014, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#define BOOST_TEST_MODULE Test_boostany_flurrying
#include <boost/test/unit_test.hpp>

#include <boost/any.hpp>
#include "flurry.h"
#include "logging.h"
#include "byte_array_wrap.h"
#include "any_int_cast.h"

using namespace std;

BOOST_AUTO_TEST_CASE(serialize_boost_any)
{
    boost::any in_int32 = int32_t{-42}, out_int32;
    boost::any in_uint32 = uint32_t(42), out_uint32;
    boost::any in_int64 = int64_t(142), out_int64;
    boost::any in_uint64 = uint64_t(-1142), out_uint64;
    boost::any in_long = long(-142), out_long;
    boost::any in_ulong = (unsigned long){142}, out_ulong;
    boost::any in_short = short(-2), out_short;
    boost::any in_ushort = (unsigned short){2}, out_ushort;
    boost::any in_double = double(-2.81), out_double;
    boost::any in_float = float{2.81}, out_float;
    boost::any in_bool = bool{true}, out_bool;
    // map<string, boost::any> in_map;
    // if (save_any<map<string, boost::any>>(value, *this)) return; // "map"
    // if (save_any<string>(value, *this)) return;
    // if (save_any<vector<boost::any>>(value, *this)) return; // "array"
    // if (save_any<byte_array>(value, *this)) return; // "byte_array"

    byte_array data;
    {
        byte_array_owrap<flurry::oarchive> write(data);
        write.archive() << in_int32 << in_uint32 << in_int64 << in_uint64 << in_long << in_ulong
            << in_short << in_ushort << in_double << in_float << in_bool;
    }
    BOOST_CHECK(data.size() == 36);
    logger::file_dump(data, "serialize_boost_any");
    {
        byte_array_iwrap<flurry::iarchive> read(data);
        read.archive() >> out_int32 >> out_uint32 >> out_int64 >> out_uint64 >> out_long >> out_ulong
            >> out_short >> out_ushort >> out_double >> out_float >> out_bool;
    }

    BOOST_CHECK(boost::any_cast<double>(in_double) == boost::any_cast<double>(out_double));
    BOOST_CHECK(boost::any_cast<float>(in_float) == boost::any_cast<float>(out_float));
    BOOST_CHECK(boost::any_cast<bool>(in_bool) == boost::any_cast<bool>(out_bool));

    BOOST_CHECK(any_int_cast<int32_t>(in_int32) == any_int_cast<int64_t>(out_int32));
    BOOST_CHECK(any_int_cast<uint32_t>(in_uint32) == any_int_cast<uint64_t>(out_uint32));
    BOOST_CHECK(any_int_cast<int64_t>(in_int64) == any_int_cast<int64_t>(out_int64));
    BOOST_CHECK(any_int_cast<uint64_t>(in_uint64) == any_int_cast<uint64_t>(out_uint64));

    BOOST_CHECK(any_int_cast<long>(in_long) == any_int_cast<long>(out_long));
    BOOST_CHECK(any_int_cast<unsigned long>(in_ulong) == any_int_cast<unsigned long>(out_ulong));
    BOOST_CHECK(any_int_cast<short>(in_short) == any_int_cast<short>(out_short));
    BOOST_CHECK(any_int_cast<unsigned short>(in_ushort) == any_int_cast<unsigned short>(out_ushort));
}

