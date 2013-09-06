#include "flurry.h"
#include "opaque_endian.h"

namespace flurry {

namespace {

template<typename T>
const char* repr(T& val)
{
    return reinterpret_cast<const char*>(&val);
}

}

void oarchive::pack_nil()
{
    os_.write("\xc0", 1);
}

void oarchive::pack_true()
{
    os_.write("\xc3", 1);
}

void oarchive::pack_false()
{
    os_.write("\xc2", 1);
}

void oarchive::pack_int8(int8_t d)
{
    if (d < -(1<<5)) {
        // int8
        os_.write("\xd0", 1);
        os_.write(repr(d), 1);
    } else {
        // fixnum
        os_.write(repr(d), 1);
    }
}

void oarchive::pack_int16(int16_t d)
{

}

void oarchive::pack_int32(int32_t d)
{

}

void oarchive::pack_int64(int64_t d)
{

}

void oarchive::pack_uint8(uint8_t d)
{
    if (d < (1<<7)) {
        // fixnum
        os_.write(repr(d), 1);
    } else {
        // uint8
        os_.write("\xcc", 1);
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
        os_.write("\xcc", 1);
        os_.write(repr(d), 1);
    } else {
        // uint16
        os_.write("\xcd", 1);
        auto big = big_uint16_t(d);
        os_.write(repr(big), 2);
    }
}

void oarchive::pack_uint32(uint32_t d)
{
}

void oarchive::pack_uint64(uint64_t d)
{
}

void oarchive::pack_real(float d)
{

}

void oarchive::pack_real(double d)
{

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
