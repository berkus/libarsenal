//
// Part of Metta OS. Check https://metta.systems for latest version.
//
// Copyright 2007 - 2017, Stanislav Karchebnyy <berkus@metta.systems>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <vector>
#include <string>
#include <utility>
#include <iomanip>
#include <iostream>
#include <functional>
#include <boost/tr1/array.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/utility/string_ref.hpp>
#include "hash_combine.h"

namespace arsenal
{

/**
 * Class mimicking Qt's QByteArray behavior using STL containers.
 */
class byte_array
{
    friend bool operator ==(const byte_array& a, const byte_array& b);
    friend bool operator !=(const byte_array& a, const byte_array& b);
    std::string value;

public:
    using container = std::string;
    using iterator = container::iterator;
    using const_iterator = container::const_iterator;
    using value_type = container::value_type;

    // Constructors

    byte_array() : value() {}

    byte_array(byte_array const& other) : value(other.value) {}

    byte_array(byte_array&&) = default;
    byte_array(std::string const& str) : value(str) {}
    byte_array(std::vector<char> const& v) : byte_array(v.data(), v.size()) {}

    byte_array(char const* str) : value(str, str+strlen(str)+1) {}

    byte_array(char const* data, size_t size) : value(data, data+size) {}

    byte_array(std::initializer_list<uint8_t> data)
    {
        resize(data.size());
        uninitialized_copy(data.begin(), data.end(), value.begin());
    }

    byte_array(boost::asio::const_buffer const& buf)
        : byte_array(boost::asio::buffer_cast<char const*>(buf), boost::asio::buffer_size(buf))
    {}

    byte_array(boost::asio::mutable_buffer const& buf)
        : byte_array(boost::asio::buffer_cast<char const*>(buf), boost::asio::buffer_size(buf))
    {}

    explicit byte_array(size_t size) { resize(size); }

    template <typename T, size_t N>
    byte_array(boost::array<T, N> const& in) : value(in.begin(), in.begin() + N) {}

    template <typename T, size_t N>
    byte_array(std::array<T, N> const& in) : value(in.begin(), in.begin() + N) {}

    // Destructor

    ~byte_array() {}

    // Assignment operators

    byte_array& operator = (byte_array const& other)
    {
        if (&other != this) {
            value = other.value;
        }
        return *this;
    }

    byte_array& operator = (byte_array&& other)
    {
        if (&other != this) {
            value = std::move(other.value);
        }
        return *this;
    }

    inline bool is_empty() const { return size() == 0; }
    inline void clear() { value.clear(); }

    char* data() {
        return &value[0];
    }
    const char* data() const {
        return &value[0];
    }
    const char* const_data() const {
        return &value[0];
    }
    /**
     * @sa length(), capacity()
     */
    size_t size() const {
        return value.size();
    }

    /**
     * @sa size(), capacity()
     */
    inline size_t length() const {
        return size();
    }

    inline void resize(size_t size) {
        value.resize(size);
    }

    char at(int i) const {
        return value.at(i);
    }

    char operator[](int i) const {
        return value[i];
    }

    char& operator[](int i) {
        return value[i];
    }

    inline void append(char c) {
        value.push_back(c);
    }

    inline void append(byte_array const& c) {
        value.insert(value.end(), c.begin(), c.end());
    }

    template<class InputIt>
    iterator insert( const_iterator pos, InputIt first, InputIt last )
    {
        return value.insert(pos, first, last);
    }

    byte_array left(size_t new_size) const
    {
        new_size = std::min(new_size, size());
        return byte_array(const_data(), new_size);
    }

    byte_array mid(int pos, size_t new_size = ~0) const
    {
        new_size = std::min(new_size, size() - pos);
        return byte_array(const_data() + pos, new_size);
    }

    byte_array right(size_t new_size) const
    {
        new_size = std::min(new_size, size());
        return byte_array(const_data() + size() - new_size, new_size);
    }

    /**
     * Fill entire array to char @a ch.
     * If the size is specified, resizes the array beforehand.
     */
    byte_array& fill(char ch, int size = -1)
    {
        if (size != -1) {
            value.resize(size);
        }
        std::fill(value.begin(), value.end(), ch);
        return *this;
    }

    /**
     * Interpret contents of byte_array buffer as holding an array of n items of type T.
     * Will ensure byte_array has enough capacity before returning the pointer to array start.
     */
    template <typename T>
    T* as() {
        if (sizeof(T) > size()) {
            resize(sizeof(T));
        }
        return reinterpret_cast<T*>(data());
    }

    /**
     * Interpret contents of byte_array buffer as if it was some arbitrary type.
     * Make sure byte_array has enough capacity to hold type T before calling this function.
     */
    template <typename T>
    T const* as() const {
        return reinterpret_cast<T const*>(data());
    }

    /**
     * Interpret contents of byte_array buffer as holding an array of n items of type T.
     * Will ensure byte_array has enough capacity before returning the pointer to array start.
     */
    template <typename T>
    T* as(size_t n) {
        if (sizeof(T)*n > size()) {
            resize(sizeof(T)*n);
        }
        return reinterpret_cast<T*>(data());
    }
    // There is no const variant of this function, as it may change the vector.

    /**
     * Unlike Qt's fromRawData current implementation of
     * wrap does not actually wrap the data, it creates
     * its own copy. XXX fix it
     */
    static byte_array wrap(const char* data, size_t size) {
        return byte_array(data, size);
    }

    inline boost::string_ref
    string_view(size_t start_offset, size_t count = boost::string_ref::npos) const {
        return boost::string_ref(value).substr(start_offset, count);
    }

    std::string as_string() const { return value; }
    std::vector<char> as_vector() const { return std::vector<char>(value.begin(), value.end()); }

    inline iterator begin() { return value.begin(); }
    inline const_iterator begin() const { return value.begin(); }
    inline iterator end() { return value.end(); }
    inline const_iterator end() const { return value.end(); }
};

inline
bool operator ==(const byte_array& a, const byte_array& b) {
    return a.value == b.value;
}

inline
bool operator !=(const byte_array& a, const byte_array& b) {
    return a.value != b.value;
}

inline
std::ostream& operator << (std::ostream& os, const byte_array& a)
{
    for (size_t s = 0; s < a.size(); ++s) {
        os << std::setfill('0') << std::hex << std::setw(2) << (int)(unsigned char)(a.at(s)) << ' ';
    }
    return os;
}

} // arsenal namespace

namespace std {

/**
 * Hash specialization for byte_array
 */
template<>
struct hash<arsenal::byte_array> : public std::unary_function<arsenal::byte_array, size_t>
{
    inline size_t operator()(arsenal::byte_array const& a) const noexcept
    {
        // return std::hash_combine_range(a.begin(), a.end()); -- hopefully in c++1y
        // VEEERY bad implementation for now. @fixme
        size_t seed = 0xdeadbeef;
        for (auto x : a) {
            stdext::hash_combine(seed, x);
        }
        return seed;
    }
};

} // std namespace

/**
// Copy-on-write implementation:
struct BigArray{
   BigArray():m_data(new int[10000000]){}
   int  operator[](size_t i)const{return (*m_data)[i];}
   int& operator[](size_t i){
        if(!m_data.unique()){//"detach"
            shared_ptr<int> _tmp(new int[10000000]);
            memcpy(_tmp.get(),m_data.get(),10000000);
            m_data=_tmp;
        }
        return (*m_data)[i];
    }
private:
    shared_ptr<int> m_data;
};
*/
