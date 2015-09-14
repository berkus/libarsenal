//
// Part of Metta OS. Check http://atta-metta.net for latest version.
//
// Copyright 2007 - 2014, Stanislav Karchebnyy <berkus@atta-metta.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "arsenal/optional_field_specification.hpp"

#include <cstddef>
#include <type_traits>
#include <unordered_map>
#include <boost/mpl/size.hpp>
#include <boost/mpl/range_c.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/fusion/include/value_at.hpp>
#include <boost/fusion/include/at.hpp>
#include <boost/fusion/include/define_struct.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/include/is_sequence.hpp>
#include <boost/range/has_range_iterator.hpp>
#include <boost/utility/string_ref.hpp>
#include <boost/asio/buffer.hpp>

// #include "arsenal/hexdump.h"//temp DEBUG

/*
//=================================================================================================
// Lazy reading
//=================================================================================================

template <typename T>
std::pair<T, boost::asio::const_buffer> read(boost::asio::const_buffer b);

struct sizer
{
    mutable boost::asio::const_buffer buf_;
    mutable size_t size_;

    explicit sizer(boost::asio::const_buffer buf)
        : buf_(std::move(buf))
        , size_(0)
    { }

    template <class T>
    auto operator()(T &) const -> typename std::enable_if<std::is_integral<T>::value>::type
    {
        size_ += sizeof(T);
        buf_ = buf_ + sizeof(T);
    }

    // ...
};

template<class T>
size_t get_size(boost::asio::const_buffer buf) {
    sizer s(std::move(buf));
    T val;
    s(val);
    return s.size_;
}

template<class T>
struct lazy {
    boost::asio::const_buffer buf_;

    lazy(boost::asio::const_buffer const& buf)
        : buf_(boost::asio::buffer_cast<void const*>(buf),
               get_size<T>(buf))
    {
        buf = buf + boost::asio::buffer_size(buf_);
    }

    T get() const { return read<T>(buf_); }

    size_t size() const { return asio::buffer_size(buf_); }
};

struct reader
{
    // lazy
    template<class T>
    void operator()(lazy<T> & val) const {
        val = lazy<T>(buf_);
        buf_ = buf_ + val.size();
    }
};
*/

constexpr unsigned int operator"" _bits_mask (unsigned long long bits)
{
    return (1 << bits) - 1;
}

constexpr unsigned int operator"" _bits_shift (unsigned long long bits)
{
    return bits;
}

struct nothing_t
{
    template <typename T>
    operator T() { return T(); } // convert to anything - should actually return empty optional?
};

// * buffer that runs from current point until the end of asio::buffer
//   (should be easy - when we see it in types, just consume the rest of the buffer)
//   sentinel type rest_t to grab until the rest of buffer... must be last in struct
struct rest_t
{
    std::string data;

    void operator = (std::string const& other) {
        data = other;
    }

    bool operator==(rest_t const& o) const
    {
        return data == o.data;
    }
};

template <class T>
using range_c = typename boost::mpl::range_c<int, 0, boost::mpl::size<T>::value>;

struct reader;

struct read_fields
{
    template <typename T, typename V>
    struct read_field_visitor
    {
        T& output_;
        V& result_;
        reader const& read_;
        uint8_t value_;

        read_field_visitor(T& out, V& result, reader const& r, uint8_t val)
            : output_(out)
            , result_(result)
            , read_(r)
            , value_(val)
        {}

        template <class Idx>
        void operator()(Idx)
        {
            if (Idx::value == value_) {
                read_(boost::fusion::at<Idx>(output_), &output_);
                result_ = boost::fusion::at<Idx>(output_);
            }
        }
    };

    template <typename T, typename V>
    void operator()(varsize_field_wrapper<T,V>& w, reader const& r, uint8_t value)
    {
        // std::cout << "r(varsize field wrapper)" << std::endl;
        boost::mpl::for_each<range_c<T>>(read_field_visitor<T,V>(w.choice_, w.output_, r, value));
    }
};

//=================================================================================================
// Reader
//=================================================================================================

struct reader
{
    mutable boost::asio::const_buffer buf_;
    // using result_type = void;

    explicit reader(boost::asio::const_buffer b)
        : buf_(std::move(b))
    {}

    // Read integral values

    template <typename T, typename P = void>
    auto operator()(T& val, P* = nullptr) const
        -> typename std::enable_if<std::is_integral<T>::value>::type
    {
        // std::cout << "r(integral value)" << std::endl;
        val = *boost::asio::buffer_cast<T const*>(buf_);
        buf_ = buf_ + sizeof(T);
    }

    // Read enums

    template <typename T, typename P = void>
    auto operator()(T& val, P* = nullptr) const
        -> typename std::enable_if<std::is_enum<T>::value>::type
    {
        // std::cout << "r(enum)" << std::endl;
        typename std::underlying_type<T>::type v;
        (*this)(v);
        val = static_cast<T>(v);
    }

    // Read constant markers

    template <typename T, T v, typename P = void>
    void operator()(std::integral_constant<T, v>, P* = nullptr) const
    {
        // std::cout << "r(integral constant) " << std::showbase << std::hex << v << std::endl;
        using type = std::integral_constant<T, v>;
        typename type::value_type val;
        (*this)(val);
        if (val != type::value) {
            throw std::exception();
        }
    }

    // Read longpascal string (uint16_t length)

    template <typename P = void>
    void operator()(std::string& val, P* = nullptr) const
    {
        // std::cout << "r(longpascal string)" << std::endl;
        uint16_t length = 0;
        (*this)(length);
        val = std::string(boost::asio::buffer_cast<char const*>(buf_), length);
        buf_ = buf_ + length;
    }

    template <typename P = void>
    void operator()(boost::string_ref& val, P* = nullptr) const
    {
        // std::cout << "r(longpascal string ref)" << std::endl;
        uint16_t length = 0;
        (*this)(length);
        val = boost::string_ref(boost::asio::buffer_cast<char const*>(buf_), length);
        buf_ = buf_ + length;
    }

    // Read vector

    template <class T, typename P = void>
    void operator()(std::vector<T>& vals, P* = nullptr)
    {
        // std::cout << "r(vector)" << std::endl;
        uint16_t length;
        (*this)(length);
        for (; length; --length)
        {
            T val;
            (*this)(val);
            vals.emplace_back(std::move(val));
        }
    }

    // Read map

    template<class K, class V, typename P = void>
    void operator()(std::unordered_map<K,V>& kvs, P* = nullptr)
    {
        // std::cout << "r(map)" << std::endl;
        uint16_t length;
        (*this)(length);
        for (; length; --length) {
            K key;
            (*this)(key);
            V val;
            (*this)(val);
            kvs.emplace(key, val);
        }
    }

    // Read varsized fields flags

    template <typename T, typename P = void>
    void operator()(field_flag<T>& val, P* = nullptr) const {
        // std::cout << "r(field flag)" << std::endl;
        (*this)(val.value);
    }

    // Read varsized field
    // Must have a parent type otherwise we couldn't read it.

    template <typename Type, typename Index, size_t Mask, size_t Offset, typename P>
    auto operator()(varsize_field_specification<Type,Index,Mask,Offset>& val, P* parent) const
        -> typename std::enable_if<boost::fusion::traits::is_sequence<P>::value>::type
    {
        // std::cout << "r(varsize field spec)" << std::endl;
        assert(parent);
        // We need to extract the flags value out of parent sequence
        auto vflag = boost::fusion::at<Index>(*parent).value; // do we need bits_type at all?
        vflag = (vflag >> Offset) & Mask;
        (*this)(val.value, vflag);
    }

    // Read the actual varsized field with proper flags.
    // This is called from the function above and does not need the parent.

    template <typename T, typename V, typename F>
    void operator()(varsize_field_wrapper<T,V>& val, F flag_value) const
    {
        read_fields()(val, *this, flag_value);
    }

    // Read optional field
    // Must have a parent type otherwise we couldn't read it.

    template <typename Type, typename Index, size_t N, typename P>
    auto operator()(optional_field_specification<Type,Index,N>& val, P* parent) const
        -> typename std::enable_if<boost::fusion::traits::is_sequence<P>::value>::type
    {
        // std::cout << "r(optional field spec)" << std::endl;
        assert(parent);
        auto flag = boost::fusion::at<Index>(*parent).value; // do we need bits_type at all?
        if (flag & (1 << N)) {
            Type v;
            (*this)(v);
            val = optional_field_specification<Type,Index,N>(std::move(v));
        } else {
            val = boost::none;
        }
    }

    // To read custom structs, wrap them into BOOST_FUSION_ADOPT_STRUCT and use this instead
    // of extra overloads.
    //
    // Sequence doesn't usually need parent.
    // Version with parent for sequence member of a sequence.
    //
    template <class T, typename P = void>
    auto operator()(T& val, P* = nullptr) const
        -> typename std::enable_if<boost::fusion::traits::is_sequence<T>::value>::type
    {
        // std::cout << "r(sequence)" << std::endl;
        boost::fusion::for_each(val, [this, &val](auto& arg) { (*this)(arg, &val); });
    }

    // Read fixed-size array

    template <typename T, size_t N, typename P = void>
    void operator()(std::array<T,N>& arr, P* = nullptr) const
    {
        // std::cout << "r(fixarray)" << std::endl;
        for (auto& val : arr)
            (*this)(val);
    }

    // Read nothing

    template <typename P = void>
    void operator()(nothing_t&, P* = nullptr) const
    {
        // std::cout << "r(nothing)" << std::endl;
        // Do nothing!
    }

    // Read until the end of the buffer
    // @todo Simply return buf_ to reduce copying?

    template <typename P = void>
    void operator()(rest_t& rest, P* = nullptr) const
    {
        // std::cout << "Grabbing rest of the packet, size = " << boost::asio::buffer_size(buf_) << std::endl;
        rest.data = std::string(boost::asio::buffer_cast<char const*>(buf_), boost::asio::buffer_size(buf_));
        // hexdump(rest.data);
        buf_ = buf_ + boost::asio::buffer_size(buf_);
    }
};

namespace fusionary {

// @todo Return only remaining buf and pass T in by reference (we do anyway)?
template <typename T>
std::pair<T, boost::asio::const_buffer> read(T const&, boost::asio::const_buffer b)
{
    reader r(std::move(b));
    T res;
    r(res);
    // std::cout << "Remaining buffer space after read " << boost::asio::buffer_size(r.buf_) << " bytes" << std::endl;
    return std::make_pair(res, r.buf_);
}

} // fusionary namespace

//=================================================================================================
// Writer
//=================================================================================================

struct writer
{
    mutable boost::asio::mutable_buffer buf_;

    explicit writer(boost::asio::mutable_buffer buf)
        : buf_(std::move(buf))
    {}

    // Write integral values

    template <class T>
    auto operator()(T const& val) const
        -> typename std::enable_if<std::is_integral<T>::value>::type
    {
        boost::asio::buffer_copy(buf_, boost::asio::buffer(&val, sizeof(T)));
        buf_ = buf_ + sizeof(T);
    }

    // Write enums

    template <class T>
    auto operator()(T const& val) const
        -> typename std::enable_if<std::is_enum<T>::value>::type
    {
        using utype = typename std::underlying_type<T>::type;
        (*this)(static_cast<utype>(val));
    }

    // Write constant markers

    template<class T, T v>
    void operator()(std::integral_constant<T, v>) const
    {
        using type = std::integral_constant<T, v>;
        (*this)(type::value);
    }

    // Write longpascal string

    void operator()(std::string const& val) const
    {
        (*this)(static_cast<uint16_t>(val.length()));
        boost::asio::buffer_copy(buf_, boost::asio::buffer(val));
        buf_ = buf_ + val.length();
    }

    // Write vectors

    template <class T>
    void operator()(std::vector<T> const& vals) const
    {
        (*this)(static_cast<uint16_t>(vals.length()));
        for(auto&& val : vals)
            (*this)(val);
    }

    // Write map

    template<class K, class V>
    void operator()(std::unordered_map<K, V> const& kvs) const
    {
        (*this)(static_cast<uint16_t>(kvs.length()));
        for(auto& kv : kvs) {
            (*this)(kv.first);
            (*this)(kv.second);
        }
    }

    // Write any ForwardRange

    template<class T, class U>
    void operator()(std::pair<T, U> const& val) const
    {
        (*this)(val.first);
        (*this)(val.second);
    }

    template<typename T>
    auto operator()(T const& vals) const ->
        typename std::enable_if<boost::has_range_const_iterator<T>::value>::type
    {
        auto length = std::distance(std::begin(vals), std::end(vals));
        if (length > std::numeric_limits<uint16_t>::max())
            throw std::exception();
        (*this)(static_cast<uint16_t>(length));
        for (auto& val : vals)
            (*this)(val);
    }

    // Write fusion structs

    template <class T>
    auto operator()(T const& val) const ->
        typename std::enable_if<boost::fusion::traits::is_sequence<T>::value>::type
    {
        boost::fusion::for_each(val, *this);
    }

    // Write fixed-size array

    template <typename T, size_t N>
    void operator()(std::array<T,N> const& arr) const
    {
        for (auto& val : arr)
            (*this)(val);
    }

    // Write the final remainder of the buffer

    void operator()(rest_t const& rest) const
    {
        boost::asio::buffer_copy(buf_, boost::asio::buffer(rest.data));
        buf_ = buf_ + rest.data.length();
    }
};

namespace fusionary {

template <typename T>
boost::asio::mutable_buffer write(boost::asio::mutable_buffer b, T const& val)
{
    writer w(std::move(b));
    w(val);
    return w.buf_;
}

} // fusionary namespace
