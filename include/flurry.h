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

    template <typename T>
    void load(T& value);

    template <typename T>
    inline void load(boost::optional<T>& value)
    {}

    template <typename T, T (*Func)(const T&)>
    inline void load(__endian_conversion<T,Func>& value)
    {}

protected:
    bool unpack_boolean();
    byte_array unpack_blob();
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
    save(T value)
    {
        *this << to_underlying(value);
    }

    // ...and the rest.
    template <typename T>
    typename std::enable_if<!std::is_enum<T>::value>::type save(T value);

    template <typename T>
    inline void save(boost::optional<T> value)
    {}

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
inline void oarchive::save(int8_t value)
{
    pack_int8(value);
}

template <>
inline void oarchive::save(int16_t value)
{
    pack_int16(value);
}

template <>
inline void oarchive::save(int32_t value)
{
    pack_int32(value);
}

template <>
inline void oarchive::save(int64_t value)
{
    pack_int64(value);
}

template <>
inline void oarchive::save(uint8_t value)
{
    pack_uint8(value);
}

template <>
inline void oarchive::save(uint16_t value)
{
    pack_uint16(value);
}

template <>
inline void oarchive::save(uint32_t value)
{
    pack_uint32(value);
}

template <>
inline void oarchive::save(uint64_t value)
{
    pack_uint64(value);
}

template <>
inline void oarchive::save(float value)
{
    pack_real(value);
}

template <>
inline void oarchive::save(double value)
{
    pack_real(value);
}

template <>
inline void oarchive::save(long value)
{
    pack_int64(value);
}

template <>
inline void oarchive::save(bool value)
{
    if (value) pack_true();
    else       pack_false();
}

template <>
inline void oarchive::save(std::nullptr_t)
{
    pack_nil();
}

template <>
inline void oarchive::save(byte_array value)
{
    pack_blob(value.data(), value.size());
}

template <>
inline void oarchive::save(std::string value)
{
    pack_string(value.data(), value.size());
}

// @todo generalize for STL container types...
// template <typename T>
// inline void oarchive::save(std::vector<T> value)
// {
//     pack_array_header(value.size());
//     for (auto x : value) {
//         *this << x;
//     }
// }

// Default deserializer implementation for types supported out-of-the-box.
template <typename T>
inline iarchive& operator >> (iarchive& in, T& value)
{
    in.load<T>(value);
    return in;
}

// Default serializer implementation for types supported out-of-the-box.
template <typename T>
inline oarchive& operator << (oarchive& out, const T& value)
{
    out.save<T>(value);
    return out;
}

} // flurry namespace
