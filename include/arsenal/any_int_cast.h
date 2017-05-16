#pragma once

#include <boost/any.hpp>

namespace arsenal
{

/**
 * Cast boost::any from any contained integer type to specified integer type T.
 * T may be not even an integer type, but contained value must be, otherwise a bad_any_cast
 * is thrown.
 */
template <typename T>
T any_int_cast(boost::any const& v)
{
    if (v.type() == typeid(T)) {
        return boost::any_cast<T>(v);
    }
    if (v.type() == typeid(int16_t)) {
        return boost::any_cast<int16_t>(v);
    }
    if (v.type() == typeid(uint16_t)) {
        return boost::any_cast<uint16_t>(v);
    }
    if (v.type() == typeid(int32_t)) {
        return boost::any_cast<int32_t>(v);
    }
    if (v.type() == typeid(uint32_t)) {
        return boost::any_cast<uint32_t>(v);
    }
    if (v.type() == typeid(int64_t)) {
        return boost::any_cast<int64_t>(v);
    }
    if (v.type() == typeid(uint64_t)) {
        return boost::any_cast<uint64_t>(v);
    }
    throw boost::bad_any_cast();
}

} // arsenal namespace
