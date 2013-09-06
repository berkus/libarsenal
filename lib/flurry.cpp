#include "flurry.h"
#include "opaque_endian.h"
#include "underlying.h"

namespace flurry {

namespace {

template<typename T>
const char* repr(T& val)
{
    return reinterpret_cast<const char*>(&val);
}

enum class TAGS : uint8_t {
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
    MAP32 = 0xdf
};

}

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
            os_ << to_underlying(TAGS::INT8) << d;
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
            os_ << to_underlying(TAGS::INT8) << d;
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
                os_ << to_underlying(TAGS::INT8) << d;
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

void oarchive::pack_blob(const char* data, size_t size)
{

}

void oarchive::pack_string(const char* data, size_t size)
{

}

void oarchive::pack_array_header(size_t size)
{

}

void oarchive::pack_map_header(size_t size)
{

}

void oarchive::pack_ext_header(uint8_t type, size_t size)
{

}


} // flurry namespace
