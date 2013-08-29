//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2013, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/endian/conversion2.hpp>
#include <functional> // for std::hash

/*
 * Idea based on sortix wrapper class.
 */
template <typename T, T (*reorder)(const T&)>
class __endian_conversion
{
    T representation;

public:
    inline __endian_conversion() : representation(reorder(T())) {}
    inline __endian_conversion(const T& t) : representation(reorder(t)) { }
    inline operator T() const { return reorder(representation); }

    inline __endian_conversion& operator=(const T& rhs)
    {
        representation = reorder(rhs);
        return *this;
    }

    inline __endian_conversion& operator=(const __endian_conversion& rhs)
    {
        representation = rhs.representation;
        return *this;
    }

    // boost.serialization support
    friend class boost::serialization::access;
    template<class Archive>
    void load(Archive &ar, const unsigned int) {
        ar >> representation;
    }
    template<class Archive>
    void save(Archive &ar, const unsigned int) const {
        ar << representation;
    }
    BOOST_SERIALIZATION_SPLIT_MEMBER()
} __attribute__((packed));

// Disable versioning for all these classes.
// from http://www.boost.org/doc/libs/1_54_0/libs/serialization/doc/traits.html 
// section Template Serialization Traits
namespace boost {
namespace serialization {

template <typename T, T (*reorder)(const T&)>
struct implementation_level<__endian_conversion<T, reorder>>
{
    typedef mpl::integral_c_tag tag;
    typedef mpl::int_<object_serializable> type;
    BOOST_STATIC_CONSTANT(
        int,
        value = implementation_level::type::value
    );
};

} // serialization namespace
} // boost namespace

/* Create big-endian versions of the stdint.h exact size data types. */
typedef int8_t big_int8_t;
typedef __endian_conversion<int16_t, boost::endian2::big> big_int16_t;
typedef __endian_conversion<int32_t, boost::endian2::big> big_int32_t;
typedef __endian_conversion<int64_t, boost::endian2::big> big_int64_t;

typedef uint8_t big_uint8_t;
typedef __endian_conversion<uint16_t, boost::endian2::big> big_uint16_t;
typedef __endian_conversion<uint32_t, boost::endian2::big> big_uint32_t;
typedef __endian_conversion<uint64_t, boost::endian2::big> big_uint64_t;

/* Create little-endian versions of the stdint.h exact size data types. */
typedef int8_t little_int8_t;
typedef __endian_conversion<int16_t, boost::endian2::little> little_int16_t;
typedef __endian_conversion<int32_t, boost::endian2::little> little_int32_t;
typedef __endian_conversion<int64_t, boost::endian2::little> little_int64_t;

typedef uint8_t little_uint8_t;
typedef __endian_conversion<uint16_t, boost::endian2::little> little_uint16_t;
typedef __endian_conversion<uint32_t, boost::endian2::little> little_uint32_t;
typedef __endian_conversion<uint64_t, boost::endian2::little> little_uint64_t;

// Hash function specialization to use endian-types in std collections.
namespace std {

template <>
struct hash<big_uint32_t>
{
    std::size_t operator()(big_uint32_t const& value) const
    {
        return std::hash<uint32_t>()(value.operator uint32_t());
    }
};

} // std namespace
