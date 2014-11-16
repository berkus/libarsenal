//
// Part of Metta OS. Check http://atta-metta.net for latest version.
//
// Copyright 2007 - 2014, Stanislav Karchebnyy <berkus@atta-metta.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <bitset>
#include <cstddef>
#include <type_traits>
#include <unordered_map>
#include <boost/bind.hpp>
#include <boost/optional.hpp>
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
/*
// Optional value
template <typename T, size_t N>
struct optional_field : boost::optional<T>
{
    using boost::optional<T>::optional;
    constexpr static const size_t bit = N;
};

// Optional bitmask position
template <typename T, size_t N = CHAR_BIT * sizeof(T)>
struct optional_field_set
{
    using value_type = T;
    using bits_type = std::bitset<N>;
};

using opt_fields = optional_field_set<uint8_t>;

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

//=================================================================================================
// Reader
//=================================================================================================

struct reader
{
    mutable boost::optional<opt_fields::bits_type> opts_;
    mutable asio::const_buffer buf_;

    explicit reader(asio::const_buffer buf)
        : buf_(std::move(buf))
    {}

    // ints
    template <class T>
    auto operator()(T & val) const -> typename std::enable_if<std::is_integral<T>::value>::type
    {
        val = *asio::buffer_cast<T const*>(buf_);
        buf_ = buf_ + sizeof(T);
    }
    // enums
    template <class T>
    auto operator()(T & val) const -> typename std::enable_if<std::is_enum<T>::value>::type
    {
        typename std::underlying_type<T>::type v;
        (*this)(v);
        val = static_cast<T>(v);
    }
    // constant markers
    template <class T, T v>
    void operator()(std::integral_constant<T, v>) const
    {
        using type = std::integral_constant<T, v>;
        typename type::value_type val;
        (*this)(val);
        if (val != type::value)
            throw std::exception();
    }
    // longpascal string (uint16_t length)
    void operator()(std::string& val) const
    {
        uint16_t length = 0;
        (*this)(length);
        val = std::string(asio::buffer_cast<char const*>(buf_), length);
        buf_ = buf_ + length;
    }
    void operator()(boost::string_ref& val) const
    {
        uint16_t length = 0;
        (*this)(length);
        val = boost::string_ref(asio::buffer_cast<char const*>(buf_), length);
        buf_ = buf_ + length;
    }
    // vector
    template <class T>
    void operator()(std::vector<T>& vals)
    {
        uint16_t length;
        (*this)(length);
        for (; length; --length)
        {
            T val;
            (*this)(val);
            vals.emplace_back(std::move(val));
        }
    }
    // map
    template<class K, class V>
    void operator()(std::unordered_map<K,V> & kvs)
    {
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
    // fusion structs
    // To read custom structs, wrap them into BOOST_FUSION_ADOPT_STRUCT and use this instead
    // of extra overloads.
    template <class T>
    auto operator()(T & val) const ->
        typename std::enable_if<boost::fusion::traits::is_sequence<T>::value>::type
    {
        boost::fusion::for_each(val, *this);
    }
    // lazy
    template<class T>
    void operator()(lazy<T> & val) const {
        val = lazy<T>(buf_);
        buf_ = buf_ + val.size();
    }
};

template <typename T>
std::pair<T, asio::const_buffer> read(asio::const_buffer b)
{
    reader r(std::move(b));
    T res;
    boost::fusion::for_each(res, r);
    return std::make_pair(res, r.buf_);
}

//=================================================================================================
// Writer
//=================================================================================================

struct writer
{
    mutable asio::mutable_buffer buf_;

    explicit writer(asio::mutable_buffer buf) : buf_(std::move(buf))
    {}
    // ints
    template <class T>
    auto operator()(T const& val) const -> typename std::enable_if<std::is_integral<T>::value>::type
    {
        asio::buffer_copy(buf_, asio::buffer(&val, sizeof(T)));
        buf_ = buf_ + sizeof(T);
    }
    // enums
    template <class T>
    auto operator()(T const& val) const -> typename std::enable_if<std::is_enum<T>::value>::type
    {
        using utype = typename std::underlying_type<T>::type;
        (*this)(static_cast<utype>(val));
    }
    // constant markers
    template<class T, T v>
    void operator()(std::integral_constant<T, v>) const
    {
        using type = std::integral_constant<T, v>;
        (*this)(type::value);
    }
    // longpascal string
    void operator()(std::string const& val) const
    {
        (*this)(static_cast<uint16_t>(val.length()));
        asio::buffer_copy(buf_, asio::buffer(val));
        buf_ = buf_ + val.length();
    }
    // vectors
    template <class T>
    void operator()(std::vector<T> const& vals)
    {
        (*this)(static_cast<uint16_t>(vals.length()));
        for(auto&& val : vals)
            (*this)(val);
    }
    // map
    template<class K, class V>
    void operator()(std::unordered_map<K, V> const& kvs)
    {
        (*this)(static_cast<uint16_t>(kvs.length()));
        for(auto& kv : kvs) {
            (*this)(kv.first);
            (*this)(kv.second);
        }
    }
    // any ForwardRange
    template<class T, class U>
    void operator()(std::pair<T, U> const& val)
    {
        (*this)(val.first);
        (*this)(val.second);
    }

    template<typename T>
    auto operator()(T const& vals) ->
        typename std::enable_if<boost::has_range_const_iterator<T>::value>::type
    {
        auto length = std::distance(std::begin(vals), std::end(vals));
        if (length > std::numeric_limits<uint16_t>::max())
            throw std::exception();
        (*this)(static_cast<uint16_t>(length));
        for (auto& val : vals)
            (*this)(val);
    }
    // fusion structs
    template <class T>
    auto operator()(T const& val) const ->
        typename std::enable_if<boost::fusion::traits::is_sequence<T>::value>::type
    {
        boost::fusion::for_each(val, *this);
    }

};

template <typename T>
asio::mutable_buffer write(asio::mutable_buffer b, T const& val)
{
    writer w(std::move(b));
    boost::fusion::for_each(val, w);
    return w.buf_;
}

// BOOST_FUSION_ADAPT_STRUCT(
//     example::decimal_t,
//     (int8_t, exponent_)
//     (uint32_t, mantissa_)
// )
// now decimal_t can be read and written using usual fusion overloads in reader and writer
*/

// Variable sized field bitmask position
template <typename T, size_t N = CHAR_BIT * sizeof(T)>
struct varsize_field_flag
{
    using value_type = T;
    using bits_type = std::bitset<N>;
    value_type value;
};

// * a variable sized field, where size is controlled by external bitfield with certain mapping
//   - where this bitfield is
//   - offset of bits
//   - mask of bits
//   - mapping function (bits to type)
template <typename T, typename I, size_t M, size_t O>
struct varsize_field_specification
{
    T value; // varsized_field_wrapper
    using index = I;
    constexpr static const size_t bit_mask = M;
    constexpr static const size_t bit_mask_offset = O;
};

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
};

// from above switcher struct we need to copy the appropriate field into output struct:
template <typename SwitchType, typename FinalType>
struct varsize_field_wrapper
{
    SwitchType choice_;
    FinalType output_;

    inline FinalType value() const { return output_; }
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

        template <class N>
        void operator()(N idx)
        {
            if (N::value == value_) {
                read_(boost::fusion::at<N>(output_), output_);
                result_ = boost::fusion::at<N>(output_);
            }
        }
    };

    template <typename T, typename V>
    void operator()(varsize_field_wrapper<T,V>& w, reader const& r, uint8_t value)
    {
        boost::mpl::for_each<range_c<T>>(read_field_visitor<T,V>(w.choice_, w.output_, r, value));
    }
};

struct reader
{
    mutable boost::asio::const_buffer buf_;
    using result_type = void;

    explicit reader(boost::asio::const_buffer b)
        : buf_(std::move(b))
    {}

    // Read integral values

    template <class T>
    auto operator()(T& val) const -> typename std::enable_if<std::is_integral<T>::value>::type
    {
        val = *boost::asio::buffer_cast<T const*>(buf_);
        buf_ = buf_ + sizeof(T);
    }

    template <class T, typename P>
    auto operator()(T& val, P&) const -> typename std::enable_if<std::is_integral<T>::value>::type
    {
        val = *boost::asio::buffer_cast<T const*>(buf_);
        buf_ = buf_ + sizeof(T);
    }

    // Read varsized fields flags

    template <typename T, typename P>
    void operator()(varsize_field_flag<T>& val, P& parent) const {
        (*this)(val.value, parent);
    }

    // Read varsized field

    template <typename T, typename I, size_t M, size_t O, typename P>
    void operator()(varsize_field_specification<T,I,M,O>& val, P& parent) const
    {
        // We need to extract the flags value out of parent sequence
        auto vflag = boost::fusion::at<I>(parent).value; // do we need bits_type at all?
        vflag = (vflag >> O) & M;
        (*this)(val.value, vflag);
    }

    // This is called from the function above and does not need the parent.
    // Read the actual varsized field with proper flags.
    template <typename T, typename V, typename F>
    void operator()(varsize_field_wrapper<T,V>& val, F flag_value) const
    {
        read_fields()(val, *this, flag_value);
    }

    // Sequence doesn't usually need parent.
    template <class T>
    auto operator()(T & val) const ->
        typename std::enable_if<boost::fusion::traits::is_sequence<T>::value>::type
    {
        boost::fusion::for_each(val, boost::bind(boost::ref(*this), _1, boost::ref(val)));
    }
    // Parent version for sequence member of a sequence.
    template <class T, typename P>
    auto operator()(T & val, P&) const ->
        typename std::enable_if<boost::fusion::traits::is_sequence<T>::value>::type
    {
        boost::fusion::for_each(val, boost::bind(boost::ref(*this), _1, boost::ref(val)));
    }

    // Read nothing

    template <typename P>
    void operator()(nothing_t&, P&) const
    {
        // Do nothing!
    }

    // Read until the end of the buffer

    // @todo Simply return the buf_ in remaining, to reduce copying etc.
    template <typename P>
    void operator()(rest_t& rest, P&) const
    {
        rest.data = std::string(boost::asio::buffer_cast<char const*>(buf_), boost::asio::buffer_size(buf_));
        buf_ = buf_ + boost::asio::buffer_size(buf_);
    }
};

