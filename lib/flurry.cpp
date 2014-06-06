//
// Part of Metta OS. Check http://atta-metta.net for latest version.
//
// Copyright 2007 - 2014, Stanislav Karchebnyy <berkus@atta-metta.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "arsenal/flurry.h"
#include "arsenal/opaque_endian.h"
#include "arsenal/underlying.h"

using namespace std;

namespace flurry {

namespace {

// For writing.
template<typename T>
char const* repr(const T& val)
{
    return reinterpret_cast<char const*>(&val);
}

// For reading.
template<typename T>
char* repr(T& val)
{
    return reinterpret_cast<char*>(&val);
}

enum class TAGS : uint8_t {
    POSITIVE_INT_FIRST = 0x00,
    POSITIVE_INT_LAST = 0x7f,
    FIXMAP_FIRST = 0x80,
    FIXMAP_LAST = 0x8f,
    FIXARRAY_FIRST = 0x90,
    FIXARRAY_LAST = 0x9f,
    FIXSTR_FIRST = 0xa0,
    FIXSTR_LAST = 0xbf,
    NIL = 0xc0,
    // 0xc1 not used
    BOOLEAN_FALSE = 0xc2,
    BOOLEAN_TRUE = 0xc3,
    BLOB8 = 0xc4,
    BLOB16 = 0xc5,
    BLOB32 = 0xc6,
    EXT8 = 0xc7,
    EXT16 = 0xc8,
    EXT32 = 0xc9,
    FLOAT = 0xca,
    DOUBLE = 0xcb,
    UINT8 = 0xcc,
    UINT16 = 0xcd,
    UINT32 = 0xce,
    UINT64 = 0xcf,
    INT8 = 0xd0,
    INT16 = 0xd1,
    INT32 = 0xd2,
    INT64 = 0xd3,
    FIXEXT1 = 0xd4,
    FIXEXT2 = 0xd5,
    FIXEXT4 = 0xd6,
    FIXEXT8 = 0xd7,
    FIXEXT16 = 0xd8,
    STR8 = 0xd9,
    STR16 = 0xda,
    STR32 = 0xdb,
    ARRAY16 = 0xdc,
    ARRAY32 = 0xdd,
    MAP16 = 0xde,
    MAP32 = 0xdf,
    NEGATIVE_INT_FIRST = 0xe0,
    NEGATIVE_INT_LAST = 0xff
};

} // anonymous namespace

//=================================================================================================
// flurry::oarchive
//=================================================================================================

void oarchive::pack_nil()
{
    os_ << to_underlying(TAGS::NIL);
}

void oarchive::pack_true()
{
    os_ << to_underlying(TAGS::BOOLEAN_TRUE);
}

void oarchive::pack_false()
{
    os_ << to_underlying(TAGS::BOOLEAN_FALSE);
}

//=================================================================================================
// integer types
//=================================================================================================

void oarchive::pack_int8(int8_t d)
{
    if (d < -(1<<5)) {
        /* signed 8 */
        os_ << to_underlying(TAGS::INT8) << d;
    } else {
        /* fixnum */
        os_ << d;
    }
}

void oarchive::pack_int16(int16_t d)
{
    if(d < -(1<<5)) {
        if(d < -(1<<7)) {
            /* signed 16 */
            os_ << to_underlying(TAGS::INT16);
            auto big = big_int16_t(d);
            os_.write(repr(big), 2);
        } else {
            /* signed 8 */
            os_ << to_underlying(TAGS::INT8) << int8_t(d);
        }
    } else if(d < (1<<7)) {
        /* fixnum */
        os_ << int8_t(d);
    } else {
        if(d < (1<<8)) {
            /* unsigned 8 */
            os_ << to_underlying(TAGS::UINT8) << uint8_t(d);
        } else {
            /* unsigned 16 */
            os_ << to_underlying(TAGS::UINT16);
            auto big = big_uint16_t(d);
            os_.write(repr(big), 2);
        }
    }
}

void oarchive::pack_int32(int32_t d)
{
    if(d < -(1<<5)) {
        if(d < -(1<<15)) {
            /* signed 32 */
            os_ << to_underlying(TAGS::INT32);
            auto big = big_int32_t(d);
            os_.write(repr(big), 4);
        } else if(d < -(1<<7)) {
            /* signed 16 */
            os_ << to_underlying(TAGS::INT16);
            auto big = big_int16_t(d);
            os_.write(repr(big), 2);
        } else {
            /* signed 8 */
            os_ << to_underlying(TAGS::INT8) << int8_t(d);
        }
    } else if(d < (1<<7)) {
        /* fixnum */
        os_ << int8_t(d);
    } else {
        if(d < (1<<8)) {
            /* unsigned 8 */
            os_ << to_underlying(TAGS::UINT8) << uint8_t(d);
        } else if(d < (1<<16)) {
            /* unsigned 16 */
            os_ << to_underlying(TAGS::UINT16);
            auto big = big_uint16_t(d);
            os_.write(repr(big), 2);
        } else {
            /* unsigned 32 */
            os_ << to_underlying(TAGS::UINT32);
            auto big = big_uint32_t(d);
            os_.write(repr(big), 4);
        }
    }
}

void oarchive::pack_int64(int64_t d)
{
    if(d < -(1LL<<5)) {
        if(d < -(1LL<<15)) {
            if(d < -(1LL<<31)) {
                /* signed 64 */
                os_ << to_underlying(TAGS::INT64);
                auto big = big_int64_t(d);
                os_.write(repr(big), 8);
            } else {
                /* signed 32 */
                os_ << to_underlying(TAGS::INT32);
                auto big = big_int32_t(d);
                os_.write(repr(big), 4);
            }
        } else {
            if(d < -(1<<7)) {
                /* signed 16 */
                os_ << to_underlying(TAGS::INT16);
                auto big = big_int16_t(d);
                os_.write(repr(big), 2);
            } else {
                /* signed 8 */
                os_ << to_underlying(TAGS::INT8) << int8_t(d);
            }
        }
    } else if(d < (1<<7)) {
        /* fixnum */
        os_ << int8_t(d);
    } else {
        if(d < (1LL<<16)) {
            if(d < (1<<8)) {
                /* unsigned 8 */
                os_ << to_underlying(TAGS::UINT8) << uint8_t(d);
            } else {
                /* unsigned 16 */
                os_ << to_underlying(TAGS::UINT16);
                auto big = big_uint16_t(d);
                os_.write(repr(big), 2);
            }
        } else {
            if(d < (1LL<<32)) {
                /* unsigned 32 */
                os_ << to_underlying(TAGS::UINT32);
                auto big = big_uint32_t(d);
                os_.write(repr(big), 4);
            } else {
                /* unsigned 64 */
                os_ << to_underlying(TAGS::UINT64);
                auto big = big_uint64_t(d);
                os_.write(repr(big), 8);
            }
        }
    }
}

void oarchive::pack_uint8(uint8_t d)
{
    if (d < (1<<7)) {
        // fixnum
        os_.write(repr(d), 1);
    } else {
        // uint8
        os_ << to_underlying(TAGS::UINT8);
        os_.write(repr(d), 1);
    }
}

void oarchive::pack_uint16(uint16_t d)
{
    if (d < (1<<7)) {
        // fixnum
        os_.write(repr(d), 1);
    } else if (d < (1<<8)) {
        // uint8
        os_ << to_underlying(TAGS::UINT8);
        os_.write(repr(d), 1);
    } else {
        // uint16
        os_ << to_underlying(TAGS::UINT16);
        auto big = big_uint16_t(d);
        os_.write(repr(big), 2);
    }
}

void oarchive::pack_uint32(uint32_t d)
{
    if(d < (1<<8)) {
        if(d < (1<<7)) {
            /* fixnum */
            os_ << uint8_t(d & 0xff);
        } else {
            /* unsigned 8 */
            os_ << to_underlying(TAGS::UINT8) << uint8_t(d & 0xff);
        }
    } else {
        if(d < (1<<16)) {
            /* unsigned 16 */
            os_ << to_underlying(TAGS::UINT16);
            auto big = big_uint16_t(d);
            os_.write(repr(big), 2);
        } else {
            /* unsigned 32 */
            os_ << to_underlying(TAGS::UINT32);
            auto big = big_uint32_t(d);
            os_.write(repr(big), 4);
        }
    }
}

void oarchive::pack_uint64(uint64_t d)
{
    if(d < (1ULL<<8)) {
        if(d < (1ULL<<7)) {
            /* fixnum */
            os_ << uint8_t(d);
        } else {
            /* unsigned 8 */
            os_ << to_underlying(TAGS::UINT8) << uint8_t(d);
        }
    } else {
        if(d < (1ULL<<16)) {
            /* unsigned 16 */
            os_ << to_underlying(TAGS::UINT16);
            auto big = big_uint16_t(d);
            os_.write(repr(big), 2);
        } else if(d < (1ULL<<32)) {
            /* unsigned 32 */
            os_ << to_underlying(TAGS::UINT32);
            auto big = big_uint32_t(d);
            os_.write(repr(big), 4);
        } else {
            /* unsigned 64 */
            os_ << to_underlying(TAGS::UINT64);
            auto big = big_uint64_t(d);
            os_.write(repr(big), 8);
        }
    }
}

//=================================================================================================
// floating-point types
//=================================================================================================

void oarchive::pack_real(float d)
{
    union { float f; uint32_t i; } mem;
    mem.f = d;
    auto big = big_uint32_t(mem.i);
    os_ << to_underlying(TAGS::FLOAT);
    os_.write(repr(big), 4);
}

void oarchive::pack_real(double d)
{
    union { double f; uint64_t i; } mem;
    mem.f = d;
    auto big = big_uint64_t(mem.i);
    os_ << to_underlying(TAGS::DOUBLE);
    os_.write(repr(big), 8);
}

//=================================================================================================
// array and blob types
//=================================================================================================

void oarchive::pack_blob(const char* data, uint64_t bytes)
{
    if (bytes < 32) {
        // Since we use blob and str interchangeably, is there any need for such differentiation?
        os_ << uint8_t(to_underlying(TAGS::FIXSTR_FIRST) | bytes);
    } else if (bytes < 256) {
        os_ << to_underlying(TAGS::BLOB8) << uint8_t(bytes);
    } else if (bytes < 65536) {
        os_ << to_underlying(TAGS::BLOB16);
        auto big = big_uint16_t(bytes);
        os_.write(repr(big), 2);
    } else if (bytes < (1ULL<<32)) {
        os_ << to_underlying(TAGS::BLOB32);
        auto big = big_uint32_t(bytes);
        os_.write(repr(big), 4);
    } else {
        throw unsupported_type("blob size too big (over 4Gib) " + to_string(bytes));
    }
    pack_raw_data(data, bytes);
}

// Since we use blob and str interchangeably, is there any need for such differentiation?
void oarchive::pack_string(const char* data, uint64_t bytes)
{
    if (bytes < 32) {
        os_ << uint8_t(to_underlying(TAGS::FIXSTR_FIRST) | bytes);
    } else if (bytes < 256) {
        os_ << to_underlying(TAGS::STR8) << uint8_t(bytes);
    } else if (bytes < 65536) {
        os_ << to_underlying(TAGS::STR16);
        auto big = big_uint16_t(bytes);
        os_.write(repr(big), 2);
    } else if (bytes < (1ULL<<32)) {
        os_ << to_underlying(TAGS::STR32);
        auto big = big_uint32_t(bytes);
        os_.write(repr(big), 4);
    } else {
        throw unsupported_type("string size too big (over 4Gib) " + to_string(bytes));
    }
    pack_raw_data(data, bytes);
}

void oarchive::pack_array_header(uint64_t count)
{
    if (count < 16) {
        os_ << uint8_t(to_underlying(TAGS::FIXARRAY_FIRST) | count);
    } else if (count < 65536) {
        os_ << to_underlying(TAGS::ARRAY16);
        auto big = big_uint16_t(count);
        os_.write(repr(big), 2);
    } else if (count < (1ULL<<32)) {
        os_ << to_underlying(TAGS::ARRAY32);
        auto big = big_uint32_t(count);
        os_.write(repr(big), 4);
    } else {
        throw unsupported_type("array size too big (over 4Gib) " + to_string(count));
    }
}

void oarchive::pack_map_header(uint64_t count)
{
    if (count < 16) {
        os_ << uint8_t(to_underlying(TAGS::FIXMAP_FIRST) | count);
    } else if (count < 65536) {
        os_ << to_underlying(TAGS::MAP16);
        auto big = big_uint16_t(count);
        os_.write(repr(big), 2);
    } else if (count < (1ULL<<32)) {
        os_ << to_underlying(TAGS::MAP32);
        auto big = big_uint32_t(count);
        os_.write(repr(big), 4);
    } else {
        throw unsupported_type("map size too big (over 4Gib) " + to_string(count));
    }
}

void oarchive::pack_ext_header(uint8_t type, size_t bytes)
{
    if (bytes == 1) {
        os_ << to_underlying(TAGS::FIXEXT1);
    } else if (bytes == 2) {
        os_ << to_underlying(TAGS::FIXEXT2);
    } else if (bytes == 4) {
        os_ << to_underlying(TAGS::FIXEXT4);
    } else if (bytes == 8) {
        os_ << to_underlying(TAGS::FIXEXT8);
    } else if (bytes == 16) {
        os_ << to_underlying(TAGS::FIXEXT16);
    } else if (bytes < 256) {
        os_ << to_underlying(TAGS::EXT8);
        os_ << uint8_t(bytes);
    } else if (bytes < 65536) {
        os_ << to_underlying(TAGS::EXT16);
        auto big = big_uint16_t(bytes);
        os_.write(repr(big), 2);
    } else {
        os_ << to_underlying(TAGS::EXT32);
        auto big = big_uint32_t(bytes);
        os_.write(repr(big), 4);
    }
    os_ << type;
    // Client should write the data using pack_raw_data(),
    // the inline type-specific wrappers handle that.
}

//=================================================================================================
// flurry::iarchive
//=================================================================================================

bool iarchive::maybe_unpack_nil()
{
    uint8_t type{0};
    type = peek();
    if (type == to_underlying(TAGS::NIL))
    {
        is_ >> type;
        return true;
    }
    return false;
}

bool iarchive::unpack_boolean()
{
    uint8_t type{0};
     if (!(is_ >> type)) {
        throw decode_error("sudden eof in unpack_boolean");
    }
    if (type == to_underlying(TAGS::BOOLEAN_TRUE))
        return true;
    if (type == to_underlying(TAGS::BOOLEAN_FALSE))
        return false;
    throw decode_error("invalid boolean tag " + to_string(type));
}

//=================================================================================================
// integer types
//=================================================================================================

int8_t iarchive::unpack_int8()
{
    uint8_t type{0};
    if (!(is_ >> type)) {
        throw decode_error("sudden eof in unpack_int8");
    }
    switch (type) {
        case to_underlying(TAGS::POSITIVE_INT_FIRST) ... to_underlying(TAGS::POSITIVE_INT_LAST): {
            int8_t value = type;
            return value;
        }
        case to_underlying(TAGS::NEGATIVE_INT_FIRST) ... to_underlying(TAGS::NEGATIVE_INT_LAST): {
            int8_t value = type;
            return value;
        }
        case to_underlying(TAGS::INT8): {
            int8_t value{0};
            is_ >> value;
            return value;
        }
    }
    throw decode_error("invalid int8 tag " + to_string(type));
}

int16_t iarchive::unpack_int16()
{
    uint8_t type{0};
    if (!(is_ >> type)) {
        throw decode_error("sudden eof in unpack_int16");
    }
    switch (type) {
        case to_underlying(TAGS::POSITIVE_INT_FIRST) ... to_underlying(TAGS::POSITIVE_INT_LAST): {
            int16_t value = type;
            return value;
        }
        case to_underlying(TAGS::NEGATIVE_INT_FIRST) ... to_underlying(TAGS::NEGATIVE_INT_LAST): {
            int16_t value = type; // FIXME: bad negative unpack?
            return value;
        }
        case to_underlying(TAGS::UINT8): {
            uint8_t value{0};
            is_ >> value;
            return value;
        }
        case to_underlying(TAGS::UINT16): {
            big_uint16_t value{0};
            is_.read(repr(value), 2);
            if (value > 0x7fff)
                throw out_of_range("int16 representation invalid " + to_string(value));
            return value;
        }
        case to_underlying(TAGS::INT8): {
            int8_t value{0};
            is_ >> value;
            return value;
        }
        case to_underlying(TAGS::INT16): {
            big_int16_t value{0};
            is_.read(repr(value), 2);
            return value;
        }
    }
    throw decode_error("invalid int16 tag " + to_string(type));
}

int32_t iarchive::unpack_int32()
{
    uint8_t type{0};
    if (!(is_ >> type)) {
        throw decode_error("sudden eof in unpack_int32");
    }
    switch (type) {
        case to_underlying(TAGS::POSITIVE_INT_FIRST) ... to_underlying(TAGS::POSITIVE_INT_LAST): {
            int8_t value = type;
            return value;
        }
        case to_underlying(TAGS::NEGATIVE_INT_FIRST) ... to_underlying(TAGS::NEGATIVE_INT_LAST): {
            int8_t value = type;
            return value;
        }
        case to_underlying(TAGS::UINT8): {
            uint8_t value{0};
            is_ >> value;
            return value;
        }
        case to_underlying(TAGS::UINT16): {
            big_uint16_t value{0};
            is_.read(repr(value), 2);
            return value;
        }
        case to_underlying(TAGS::UINT32): {
            big_uint32_t value{0};
            is_.read(repr(value), 4);
            if (value > 0x7fffffff)
                throw out_of_range("int32 representation invalid " + to_string(value));
            return value;
        }
        case to_underlying(TAGS::INT8): {
            int8_t value{0};
            is_ >> value;
            return value;
        }
        case to_underlying(TAGS::INT16): {
            big_int16_t value{0};
            is_.read(repr(value), 2);
            return value;
        }
        case to_underlying(TAGS::INT32): {
            big_int32_t value{0};
            is_.read(repr(value), 4);
            return value;
        }
    }
    throw decode_error("invalid int32 tag " + to_string(type));
}

int64_t iarchive::unpack_int64()
{
    uint8_t type{0};
    if (!(is_ >> type)) {
        throw decode_error("sudden eof in unpack_int64");
    }
    switch (type) {
        case to_underlying(TAGS::POSITIVE_INT_FIRST) ... to_underlying(TAGS::POSITIVE_INT_LAST): {
            int8_t value = type;
            return value;
        }
        case to_underlying(TAGS::NEGATIVE_INT_FIRST) ... to_underlying(TAGS::NEGATIVE_INT_LAST): {
            int8_t value = type;
            return value;
        }
        case to_underlying(TAGS::UINT8): {
            uint8_t value{0};
            is_ >> value;
            return value;
        }
        case to_underlying(TAGS::UINT16): {
            big_uint16_t value{0};
            is_.read(repr(value), 2);
            return value;
        }
        case to_underlying(TAGS::UINT32): {
            big_uint32_t value{0};
            is_.read(repr(value), 4);
            return value;
        }
        case to_underlying(TAGS::UINT64): {
            big_uint64_t value{0};
            is_.read(repr(value), 8);
            if (value > 0x7fffffffffffffff)
                throw out_of_range("int64 representation invalid " + to_string(value));
            return value;
        }
        case to_underlying(TAGS::INT8): {
            int8_t value{0};
            is_ >> value;
            return value;
        }
        case to_underlying(TAGS::INT16): {
            big_int16_t value{0};
            is_.read(repr(value), 2);
            return value;
        }
        case to_underlying(TAGS::INT32): {
            big_int32_t value{0};
            is_.read(repr(value), 4);
            return value;
        }
        case to_underlying(TAGS::INT64): {
            big_int64_t value{0};
            is_.read(repr(value), 8);
            return value;
        }
    }
    throw decode_error("invalid int64 tag " + to_string(type));
}

uint8_t iarchive::unpack_uint8()
{
    uint8_t type{0};
    if (!(is_ >> type)) {
        throw decode_error("sudden eof in unpack_uint8");
    }
    switch (type) {
        case to_underlying(TAGS::POSITIVE_INT_FIRST) ... to_underlying(TAGS::POSITIVE_INT_LAST): {
            return type;
        }
        case to_underlying(TAGS::UINT8): {
            uint8_t value{0};
            is_ >> value;
            return value;
        }
    }
    throw decode_error("invalid uint8 tag " + to_string(type));
}

uint16_t iarchive::unpack_uint16()
{
    uint8_t type{0};
    if (!(is_ >> type)) {
        throw decode_error("sudden eof in unpack_uint16");
    }
    switch (type) {
        case to_underlying(TAGS::POSITIVE_INT_FIRST) ... to_underlying(TAGS::POSITIVE_INT_LAST): {
            uint8_t value = type & 0x7f;
            return value;
        }
        case to_underlying(TAGS::UINT8): {
            uint8_t value{0};
            is_ >> value;
            return value;
        }
        case to_underlying(TAGS::UINT16): {
            big_uint16_t value{0};
            is_.read(repr(value), 2);
            return value;
        }
    }
    throw decode_error("invalid uint16 tag " + to_string(type));
}

uint32_t iarchive::unpack_uint32()
{
    uint8_t type{0};
    if (!(is_ >> type)) {
        throw decode_error("sudden eof in unpack_uint32");
    }
    switch (type) {
        case to_underlying(TAGS::POSITIVE_INT_FIRST) ... to_underlying(TAGS::POSITIVE_INT_LAST): {
            uint8_t value = type & 0x7f;
            return value;
        }
        case to_underlying(TAGS::UINT8): {
            uint8_t value{0};
            is_ >> value;
            return value;
        }
        case to_underlying(TAGS::UINT16): {
            big_uint16_t value{0};
            is_.read(repr(value), 2);
            return value;
        }
        case to_underlying(TAGS::UINT32): {
            big_uint32_t value{0};
            is_.read(repr(value), 4);
            return value;
        }
    }
    throw decode_error("invalid uint32 tag " + to_string(type));
}

uint64_t iarchive::unpack_uint64()
{
    uint8_t type{0};
    if (!(is_ >> type)) {
        throw decode_error("sudden eof in unpack_uint64");
    }
    switch (type) {
        case to_underlying(TAGS::POSITIVE_INT_FIRST) ... to_underlying(TAGS::POSITIVE_INT_LAST): {
            uint8_t value = type & 0x7f;
            return value;
        }
        case to_underlying(TAGS::UINT8): {
            uint8_t value{0};
            is_ >> value;
            return value;
        }
        case to_underlying(TAGS::UINT16): {
            big_uint16_t value{0};
            is_.read(repr(value), 2);
            return value;
        }
        case to_underlying(TAGS::UINT32): {
            big_uint32_t value{0};
            is_.read(repr(value), 4);
            return value;
        }
        case to_underlying(TAGS::UINT64): {
            big_uint64_t value{0};
            is_.read(repr(value), 8);
            return value;
        }
    }
    throw decode_error("invalid uint64 tag " + to_string(type));
}

//=================================================================================================
// floating-point types
//=================================================================================================

float iarchive::unpack_float()
{
    uint8_t type{0};
    if (!(is_ >> type)) {
        throw decode_error("sudden eof in unpack_float");
    }
    if (type != to_underlying(TAGS::FLOAT))
        throw decode_error("invalid float tag " + to_string(type));

    big_uint32_t value{0};
    is_.read(repr(value), 4);

    union { float f; uint32_t i; } mem;
    mem.i = value;
    return mem.f;
}

double iarchive::unpack_double()
{
    uint8_t type{0};
    if (!(is_ >> type)) {
        throw decode_error("sudden eof in unpack_double");
    }
    if (type != to_underlying(TAGS::DOUBLE))
        throw decode_error("invalid double tag " + to_string(type));

    big_uint64_t value{0};
    is_.read(repr(value), 8);

    union { double f; uint64_t i; } mem;
    mem.i = value;
    return mem.f;
}

//=================================================================================================
// array and blob types
//=================================================================================================

byte_array iarchive::unpack_blob()
{
    uint8_t type{0};
    size_t bytes{0};

    if (!(is_ >> type)) {
        return byte_array();
        // throw eof?
        // throw decode_error("sudden eof in unpack_blob");
    }
    switch (type) {
        case to_underlying(TAGS::FIXSTR_FIRST) ... to_underlying(TAGS::FIXSTR_LAST):
            bytes = type & 0x1f;
            break;

        case to_underlying(TAGS::BLOB8): {
            uint8_t size{0};
            is_.read(repr(size), 1);
            bytes = size;
            break;
        }

        case to_underlying(TAGS::BLOB16): {
            big_uint16_t size{0};
            is_.read(repr(size), 2);
            bytes = size;
            break;
        }

        case to_underlying(TAGS::BLOB32): {
            big_uint32_t size{0};
            is_.read(repr(size), 4);
            bytes = size;
            break;
        }

        default:
            if (is_.eof())
                return byte_array();

            throw decode_error("invalid blob tag " + to_string(type));
    }

    byte_array out;
    out.resize(bytes);
    unpack_raw_data(out);

    return out;
}

string iarchive::unpack_string()
{
    uint8_t type{0};
    size_t bytes{0};

    if (!(is_ >> type)) {
        throw decode_error("sudden eof in unpack_string");
    }
    switch (type) {
        case to_underlying(TAGS::FIXSTR_FIRST) ... to_underlying(TAGS::FIXSTR_LAST):
            bytes = type & 0x1f;
            break;

        case to_underlying(TAGS::STR8): {
            uint8_t size{0};
            is_.read(repr(size), 1);
            bytes = size;
            break;
        }

        case to_underlying(TAGS::STR16): {
            big_uint16_t size{0};
            is_.read(repr(size), 2);
            bytes = size;
            break;
        }

        case to_underlying(TAGS::STR32): {
            big_uint32_t size{0};
            is_.read(repr(size), 4);
            bytes = size;
            break;
        }

        default:
            if (is_.eof())
                return string();

            throw decode_error("invalid string tag " + to_string(type));
    }

    byte_array out;
    out.resize(bytes);
    unpack_raw_data(out);

    return string(out.begin(), out.end());
}

size_t iarchive::unpack_array_header()
{
    uint8_t type{0};
    size_t count{0};

    if (!(is_ >> type)) {
        throw decode_error("sudden eof in unpack_array_header");
    }
    switch (type) {
        case to_underlying(TAGS::FIXARRAY_FIRST) ... to_underlying(TAGS::FIXARRAY_LAST):
            count = type & 0x0f;
            break;
        case to_underlying(TAGS::ARRAY16): {
            big_uint16_t size{0};
            is_.read(repr(size), 2);
            count = size;
            break;
        }
        case to_underlying(TAGS::ARRAY32): {
            big_uint32_t size{0};
            is_.read(repr(size), 4);
            count = size;
            break;
        }
        default:
            throw decode_error("invalid array tag " + to_string(type));
    }

    return count;
}

size_t iarchive::unpack_map_header()
{
    uint8_t type{0};
    size_t count{0};

    if (!(is_ >> type)) {
        throw decode_error("sudden eof in unpack_map_header");
    }
    switch (type) {
        case to_underlying(TAGS::FIXMAP_FIRST) ... to_underlying(TAGS::FIXMAP_LAST):
            count = type & 0x0f;
            break;
        case to_underlying(TAGS::MAP16): {
            big_uint16_t size{0};
            is_.read(repr(size), 2);
            count = size;
            break;
        }
        case to_underlying(TAGS::MAP32): {
            big_uint32_t size{0};
            is_.read(repr(size), 4);
            count = size;
            break;
        }
        default:
            throw decode_error("invalid map tag " + to_string(type));
    }

    return count;
}

// Read as many bytes as buf has in capacity.
// This makes sure we never read into unallocated memory.
void iarchive::unpack_raw_data(byte_array& buf)
{
    is_.read(buf.data(), buf.size());//hmm, what about using capacity()?
}

// Read and discard given number of bytes
void iarchive::skip_raw_data(size_t bytes)
{
    char buf[512];
    while (bytes > 512) {
        is_.read(buf, 512);
        bytes -= 512;
    }
    is_.read(buf, bytes);
}

//=================================================================================================
// boost::any serialization
//=================================================================================================

template <typename T>
bool save_any(boost::any const& v, oarchive& oa)
{
    static boost::any type = T();
    if (v.type() == type.type()) {
        oa << boost::any_cast<T const&>(v);
        return true;
    }
    return false;
}

template <>
void oarchive::save(boost::any const& value)
{
    // Problem:
    // Serialized boost::anys use most compact wire encoding, so it's not possible to restore
    // to exactly the same type as boost::any had before serialization.
    //
    // @todo For integer types save EXACTLY the type that was passed in...
    // This would mean that deserialized type will be matching.
    // This is a limitation of boost::any and while inefficient, I don't see any reasonable
    // way around it.
    if (save_any<int32_t>(value, *this)) return;
    if (save_any<uint32_t>(value, *this)) return;
    if (save_any<int64_t>(value, *this)) return;
    if (save_any<uint64_t>(value, *this)) return;
    if (save_any<long>(value, *this)) return;
    if (save_any<unsigned long>(value, *this)) return;
    if (save_any<short>(value, *this)) return;
    if (save_any<unsigned short>(value, *this)) return;
    if (save_any<map<string, boost::any>>(value, *this)) return; // "map"
    if (save_any<string>(value, *this)) return;
    if (save_any<vector<boost::any>>(value, *this)) return; // "array"
    if (save_any<vector<char>>(value, *this)) return; // "byte_array"
    if (save_any<byte_array>(value, *this)) return; // "byte_array"
    if (save_any<double>(value, *this)) return;
    if (save_any<float>(value, *this)) return;
    if (save_any<bool>(value, *this)) return;
    throw encode_error(string("unsupported boost::any type ") + value.type().name());
}

// @todo
// boost::any reader expands all read integers to int64_t or uint64_t types.
// this is a limitation of current implementation, in the future the implementation of save()
// would preserve the boost::any actual type and therefore loading will return the same type.
template <>
void iarchive::load(boost::any& value)
{
    uint8_t type = peek();
    switch (type)
    {
        case to_underlying(TAGS::NEGATIVE_INT_FIRST) ... to_underlying(TAGS::NEGATIVE_INT_LAST):
        case to_underlying(TAGS::INT8):
        case to_underlying(TAGS::INT16):
        case to_underlying(TAGS::INT32):
        case to_underlying(TAGS::INT64): {
            value = int64_t(unpack_int64());
            return;
        }
        case to_underlying(TAGS::POSITIVE_INT_FIRST) ... to_underlying(TAGS::POSITIVE_INT_LAST):
        case to_underlying(TAGS::UINT8):
        case to_underlying(TAGS::UINT16):
        case to_underlying(TAGS::UINT32):
        case to_underlying(TAGS::UINT64): {
            value = uint64_t(unpack_uint64());
            return;
        }
        case to_underlying(TAGS::FLOAT): {
            value = unpack_float();
            return;
        }
        case to_underlying(TAGS::DOUBLE): {
            value = unpack_double();
            return;
        }
        case to_underlying(TAGS::BOOLEAN_FALSE):
        case to_underlying(TAGS::BOOLEAN_TRUE): {
            value = unpack_boolean();
            return;
        }
        case to_underlying(TAGS::NIL): {
            value = boost::any();
            return;
        }
        case to_underlying(TAGS::FIXSTR_FIRST) ... to_underlying(TAGS::FIXSTR_LAST):
        case to_underlying(TAGS::STR8):
        case to_underlying(TAGS::STR16):
        case to_underlying(TAGS::STR32): {
            value = unpack_string();
            return;
        }
        case to_underlying(TAGS::FIXMAP_FIRST) ... to_underlying(TAGS::FIXMAP_LAST):
        case to_underlying(TAGS::MAP16):
        case to_underlying(TAGS::MAP32): {
            map<string, boost::any> m;
            size_t size = unpack_map_header();
            for (size_t x = 0; x < size; ++x) {
                string key;
                boost::any v;
                key = unpack_string();
                load(v);
                m[key] = v;
            }
            value = m;
            return;
        }
        case to_underlying(TAGS::FIXARRAY_FIRST) ... to_underlying(TAGS::FIXARRAY_LAST):
        case to_underlying(TAGS::ARRAY16):
        case to_underlying(TAGS::ARRAY32): {
            vector<boost::any> v;
            size_t size = unpack_array_header();
            v.reserve(size);
            for (size_t x = 0; x < size; ++x) {
                load(v[x]);
            }
            value = v;
            return;
        }
        case to_underlying(TAGS::BLOB8):
        case to_underlying(TAGS::BLOB16):
        case to_underlying(TAGS::BLOB32): {
            value = unpack_blob();
            return;
        }
    // unsupported
        // EXT8 = 0xc7,
        // EXT16 = 0xc8,
        // EXT32 = 0xc9,
        // FIXEXT1 = 0xd4,
        // FIXEXT2 = 0xd5,
        // FIXEXT4 = 0xd6,
        // FIXEXT8 = 0xd7,
        // FIXEXT16 = 0xd8,
    }
    throw decode_error("invalid tag " + to_string(type));
}

} // flurry namespace

