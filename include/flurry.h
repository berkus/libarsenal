/**
 * MessagePack is a tagged format which is compact and rather efficient.
 * This file implements version v5 described at https://gist.github.com/frsyuki/5432559
 * It is byte-oriented and fits well for compact network serialization.
 */
#pragma once

#include <iostream>

namespace flurry {

class iarchive
{
public:
    iarchive(std::istream& in);

    template <typename T>
    void load(T& value);
};

class oarchive
{
    std::ostream& os_;
public:
    oarchive(std::ostream& out) : os_(out) {}

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
inline void oarchive::save<int>(int value)
{
    pack_int64(value);
}

template <>
inline void oarchive::save<long>(long value)
{
    pack_int64(value);
}

template <>
inline void oarchive::save<bool>(bool value)
{
    if (value) pack_true();
    else       pack_false();
}

template <>
inline void oarchive::save<std::nullptr_t>(std::nullptr_t)
{
    pack_nil();
}

// Deserializer conforms to this signature:
// template<class Input, typename T>
// Input& operator >> (Input& in, T& value)
// {
//     return in;
// }

// Default implementation for values supported out-of-the-box.
template <typename T>
inline iarchive& operator >> (iarchive& in, T& value)
{
    in.load<T>(value);
    return in;
}

template <typename T>
inline oarchive& operator << (oarchive& out, const T& value)
{
    out.save<T>(value);
    return out;
}

} // flurry namespace
