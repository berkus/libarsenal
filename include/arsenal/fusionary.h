//
// Part of Metta OS. Check http://atta-metta.net for latest version.
//
// Copyright 2007 - 2014, Stanislav Karchebnyy <berkus@atta-metta.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <ostream> // temp
#include <bitset>
#include <cstddef>
#include <type_traits>
#include <unordered_map>
#include <boost/optional.hpp>
#include <boost/mpl/range_c.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/fusion/include/value_at.hpp>
#include <boost/fusion/include/at.hpp>
#include <boost/fusion/include/define_struct.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/include/is_sequence.hpp>
#include <boost/range/has_range_iterator.hpp>
#include <boost/asio/buffer.hpp>

// @todo Cleanup to make it a proper header
namespace asio = boost::asio;
using boost::optional;

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
std::pair<T, asio::const_buffer> read(asio::const_buffer b);

struct sizer
{
    mutable asio::const_buffer buf_;
    mutable size_t size_;

    explicit sizer(asio::const_buffer buf)
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
size_t get_size(asio::const_buffer buf) {
    sizer s(std::move(buf));
    T val;
    s(val);
    return s.size_;
}

template<class T>
struct lazy {
    asio::const_buffer buf_;

    lazy(asio::const_buffer const& buf)
        : buf_(asio::buffer_cast<void const*>(buf),
               get_size<T>(buf))
    {
        buf = buf + asio::buffer_size(buf_);
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
        typedef std::integral_constant<T, v> type;
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
        buf_ = buf + sizeof(T);
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
        typedef std::integral_constant<T, v> type;
        (*this)(type::value);
    }
    // longpascal string
    void operator()(std::string const& val) const
    {
        (*this)(static_cast<uint16_t>(val.length()));
        asio::buffer_copy(buf_, asio::buffer(val));
        buf_ = buf_ + length;
    }
    // vectors
    template <class T>
    void operator()(std::vector<T> const& vals)
    {
        (*this)(vals.length());//uint16_t cast?
        for(auto&& val : vals)
            (*this)(val);
    }
    // map
    template<class K, class V>
    void operator()(std::unordered_map<K, V> const& kvs)
    {
        (*this)(vals.length());
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
    auto operator()(T const& val) ->
        typename std::enable_if<boost::has_range_const_iterator<T>::value>::type
    {
        auto length = std::distance(std::begin(val), std::end(val));
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

//=================================================================================================
// Pretty printer
//=================================================================================================

namespace detail {
    namespace mpl = boost::mpl;
    template <class T>
    using typename range_c = mpl::range_c<int, 0, mpl::size<T>::value >;

    namespace f_ext = boost::fusion::extension;
    template <class T>
    struct mpl_visitor {
        value_writer w_;
        T& msg_;
        mpl_visitor(std::ostream& os, T& msg)
            : value_writer_(os)
            , msg_(msg)
        {}
        template <class N>
        void operator()(N idx)
        {
            w_(f_ext::struct_member_name<T, N::value>::call(), ":");
            w_(fusion::at<N>(msg_), (idx != mpl::size<T>::value ? "," : ""));
        }
    };

    template <class T>
    struct printer
    {
        T& msg_;

        friend std::ostream& operator<<(std::ostream& os, printer<T> const& v)
        {
            using namespace detail;
            boost::mpl::for_each<typename range_c<T>>(mpl_visitor<T>(os, v.msg_));
            return os;
        }
    };

} // detail namespace

template <class T>
detail::printer<T> pretty_print(T const& v)
{
    return detail::printer<T>(v);
}
