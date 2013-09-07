//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2013, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
/**
 * MessagePack is a tagged format which is compact and rather efficient.
 * This file implements version v5 described at https://gist.github.com/frsyuki/5432559
 * It is byte-oriented and fits well for compact network serialization.
 */
#pragma once

#include <boost/serialization/serialization.hpp> // for constructing optionals
#include <boost/serialization/detail/stack_constructor.hpp> // for constructing optionals
#include <boost/optional/optional.hpp>
#include <type_traits>
#include <iostream>
#include "byte_array.h"
#include "underlying.h"
#include "opaque_endian.h"

namespace flurry {

class encode_error : public std::exception
{};

class decode_error : public std::exception
{};

class unsupported_type : public std::exception
{};

class iarchive
{
    std::istream& is_;
public:
    inline iarchive(std::istream& in) : is_(in) {}
    explicit inline operator bool() const { return (bool)is_; }

    // For enums...
    template <typename T>
    inline typename std::enable_if<std::is_enum<T>::value>::type
    load(T& value)
    {
        int read; // @fixme This is not very robust.
        load(read);
        value = T(read);
    }

    // ...and the rest.
    template <typename T>
    typename std::enable_if<!std::is_enum<T>::value>::type load(T& value);

    template <typename T>
    inline void load(boost::optional<T>& value)
    {
        bool empty = unpack_nil();
        if (empty) {
            value.reset();
        } else {
            boost::serialization::detail::stack_construct<iarchive, T> aux(*this, 0);
            *this >> aux.reference();
            value.reset(aux.reference());
        }
    }

    template <typename T, size_t N>
    inline void load(boost::array<T,N>& value)
    {
        assert(0 and "Loading boost::array fail");
    }

    template <typename T, T (*Func)(const T&)>
    inline void load(__endian_conversion<T,Func>& value)
    {
        assert(0 and "Loading __endian_conversion fail - this call is not needed, remove!");
    }

    template <typename T>
    inline void load(std::vector<T>& value)
    {
        size_t size = unpack_array_header();
        value.resize(size);
        for (auto it = value.begin(); it != value.end(); ++it)
        {
            *this >> *it;
        }
    }

protected:
    /**
     * Semantics of unpack_nil are a bit different.
     * It peeks to see if the next byte denotes nil type, and if so consumes it and returns true;
     * otherwise it returns false and leaves the stream untouched.
     */
    bool unpack_nil();
    bool unpack_boolean();

    int8_t  unpack_int8();
    int16_t unpack_int16();
    int32_t unpack_int32();
    int64_t unpack_int64();

    uint8_t  unpack_uint8();
    uint16_t unpack_uint16();
    uint32_t unpack_uint32();
    uint64_t unpack_uint64();

    float unpack_float();
    double unpack_double();

    byte_array unpack_blob();
    std::string unpack_string();

    size_t unpack_array_header();
    size_t unpack_map_header();
    size_t unpack_ext_header(uint8_t& type);

    void unpack_raw_data(byte_array& buf);
};

class oarchive
{
    std::ostream& os_;
public:
    inline oarchive(std::ostream& out) : os_(out) {}

    // For enums...
    template <typename T>
    inline typename std::enable_if<std::is_enum<T>::value>::type
    save(T const& value)
    {
        *this << to_underlying(value);
    }

    // ...and the rest.
    template <typename T>
    typename std::enable_if<!std::is_enum<T>::value>::type save(T const& value);

    template <typename T>
    inline void save(boost::optional<T> const& value);

    template <typename T, size_t N>
    inline void save(boost::array<T,N> const& value);

    template <typename T>
    inline void save(std::vector<T> const& value);

protected:
    // Actual serialization functions.
    void pack_nil();
    void pack_true();
    void pack_false();

    void pack_int8(int8_t d);
    void pack_int16(int16_t d);
    void pack_int32(int32_t d);
    void pack_int64(int64_t d);

    void pack_uint8(uint8_t d);
    void pack_uint16(uint16_t d);
    void pack_uint32(uint32_t d);
    void pack_uint64(uint64_t d);

    void pack_real(float d);
    void pack_real(double d);

    void pack_blob(const char* data, size_t size);
    void pack_string(const char* data, size_t size);

    void pack_array_header(size_t size);
    void pack_map_header(size_t size);
    void pack_ext_header(uint8_t type, size_t size);

    inline void pack_raw_data(const char* data, size_t bytes) {
        os_.write(data, bytes);
    }
};

//
// Specializations for flurry serializable types.
// 
// These types are basic building blocks for serializing other, more complex types.
//
template <>
inline void iarchive::load(int8_t& value)
{
    value = unpack_int8();
}

template <>
inline void iarchive::load(int16_t& value)
{
    value = unpack_int16();
}

template <>
inline void iarchive::load(int32_t& value)
{
    value = unpack_int32();
}

template <>
inline void iarchive::load(int64_t& value)
{
    value = unpack_int64();
}

template <>
inline void iarchive::load(uint8_t& value)
{
    value = unpack_uint8();
}

template <>
inline void iarchive::load(uint16_t& value)
{
    value = unpack_uint16();
}

template <>
inline void iarchive::load(uint32_t& value)
{
    value = unpack_uint32();
}

template <>
inline void iarchive::load(uint64_t& value)
{
    value = unpack_uint64();
}

template <>
inline void iarchive::load(float& value)
{
    value = unpack_float();
}

template <>
inline void iarchive::load(double& value)
{
    value = unpack_double();
}

template <>
inline void iarchive::load(long& value)
{
    value = unpack_int64();
}

template <>
inline void iarchive::load(bool& value)
{
    value = unpack_boolean();
}

template <>
inline void iarchive::load(byte_array& value)
{
    value = unpack_blob();
}

template <>
inline void oarchive::save(int8_t const& value)
{
    pack_int8(value);
}

template <>
inline void oarchive::save(int16_t const& value)
{
    pack_int16(value);
}

template <>
inline void oarchive::save(int32_t const& value)
{
    pack_int32(value);
}

template <>
inline void oarchive::save(int64_t const& value)
{
    pack_int64(value);
}

template <>
inline void oarchive::save(uint8_t const& value)
{
    pack_uint8(value);
}

template <>
inline void oarchive::save(uint16_t const& value)
{
    pack_uint16(value);
}

template <>
inline void oarchive::save(uint32_t const& value)
{
    pack_uint32(value);
}

template <>
inline void oarchive::save(uint64_t const& value)
{
    pack_uint64(value);
}

template <>
inline void oarchive::save(float const& value)
{
    pack_real(value);
}

template <>
inline void oarchive::save(double const& value)
{
    pack_real(value);
}

template <>
inline void oarchive::save(long const& value)
{
    pack_int64(value);
}

template <>
inline void oarchive::save(bool const& value)
{
    if (value) pack_true();
    else       pack_false();
}

template <>
inline void oarchive::save(std::nullptr_t const&)
{
    pack_nil();
}

template <>
inline void oarchive::save(byte_array const& value)
{
    pack_blob(value.data(), value.size());
}

template <>
inline void oarchive::save(std::string const& value)
{
    pack_string(value.data(), value.size());
}

template <typename T>
inline void oarchive::save(boost::optional<T> const& value)
{
    if (value.is_initialized()) {
        *this << *value;
    } else {
        pack_nil();
    }
}

template <typename T, size_t N>
inline void oarchive::save(boost::array<T,N> const& value)
{
    pack_array_header(N);
    for (auto x : value) {
        *this << x;
    }
}

// @todo generalize for STL container types...
template <typename T>
inline void oarchive::save(std::vector<T> const& value)
{
    pack_array_header(value.size());
    for (auto x : value) {
        *this << x;
    }
}

// Default deserializer implementation for types supported out-of-the-box.
template <typename T>
inline iarchive& operator >> (iarchive& in, T& value)
{
    in.load(value);
    return in;
}

// Default serializer implementation for types supported out-of-the-box.
template <typename T>
inline oarchive& operator << (oarchive& out, T const& value)
{
    out.save(value);
    return out;
}

template <typename T, size_t N>
inline oarchive& operator << (oarchive& out, boost::array<T,N> const& value)
{
    out.save(value);
    return out;
}

} // flurry namespace
