//
// Part of Metta OS. Check http://atta-metta.net for latest version.
//
// Copyright 2007 - 2014, Stanislav Karchebnyy <berkus@atta-metta.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/endian/arithmetic.hpp>

// @fixme: use boost/endian/buffer.hpp for fusionary i/o

template <typename T>
struct is_endian
{
    static constexpr bool const value = false;
};

template <boost::endian::order O, typename T, std::size_t N, boost::endian::align A>
struct is_endian<boost::endian::endian_arithmetic<O, T, N, A>>
{
    static constexpr bool const value = true;
};
