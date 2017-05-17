//
// Part of Metta OS. Check http://atta-metta.net for latest version.
//
// Copyright 2007 - 2014, Stanislav Karchebnyy <berkus@atta-metta.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/range/iterator_range.hpp>
#include "byte_array.h"

namespace arsenal
{

/**
 * Wrap byte array in an input wrapping for boost.serialization or msgpack archives.
 * Archive type must accept an istream as constructor argument.
 */
template <class Archive>
class byte_array_iwrap
{
    boost::iostreams::filtering_istream in;
    Archive ia;

public:
    byte_array_iwrap(byte_array const& data)
        : in(boost::make_iterator_range(data.as_vector()))
        , ia(in)
    {}

    Archive& archive() { return ia; }
};

/**
 * Wrap byte array in an output wrapping for boost.serialization or msgpack archives.
 * Archive type must accept an ostream as constructor argument.
 */
template <class Archive>
class byte_array_owrap
{
    boost::iostreams::filtering_ostream out;
    Archive oa;

public:
    byte_array_owrap(byte_array& data)
        : out(boost::iostreams::back_inserter(data.as_vector()))
        , oa(out)
    {}

    Archive& archive() { return oa; }
};

} // arsenal namespace
