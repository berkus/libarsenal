/**
 * MessagePack is a tagged format which is compact and rather efficient.
 * This file implements version v5 described at https://gist.github.com/frsyuki/5432559
 * It is byte-oriented and fits well for compact network serialization.
 */
#pragma once

#include <iostream>
#include "byte_array.h"

namespace flurry {

class unsupported_type : std::exception
{};

class iarchive
{
    std::istream& is_;
public:
    inline iarchive(std::istream& in) : is_(in) {}

    template <typename T>
    void load(T& value);
};

class oarchive
{
    std::ostream& os_;
public:
    inline oarchive(std::ostream& out) : os_(out) {}

    template <typename T>
    void save(T value);

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
};

//
// Specializations for flurry serializable types.
// 
// These types are basic building blocks for serializing other, more complex types.
//
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
