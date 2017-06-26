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

    byte_array();
    byte_array(byte_array const&);
    byte_array(std::string const& str) : value(str) {}
    byte_array(std::vector<char> const& v) : byte_array(v.data(), v.size()) {}
    byte_array(char const* str);
    byte_array(char const* data, size_t size);
    byte_array(std::initializer_list<uint8_t> data);
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

    ~byte_array();
    byte_array& operator = (byte_array const& other);
    byte_array& operator = (byte_array&& other);

    inline bool is_empty() const { return size() == 0; }
    inline void clear() { value.clear(); }

    char* data();
    const char* data() const;
    const char* const_data() const;
    /**
     * @sa length(), capacity()
     */
    size_t size() const;
    /**
     * @sa size(), capacity()
     */
    inline size_t length() const {
        return size();
    }

    inline void resize(size_t size) {
        value.resize(size);
    }

    char at(int i) const;
    char operator[](int i) const;
    char& operator[](int i);

    inline void append(char c) {
        value.push_back(c);
    }

    inline void append(byte_array const& c) {
        value.insert(value.end(), c.begin(), c.end());
    }

    byte_array left(size_t size) const;
    byte_array mid(int pos, size_t size = ~0) const;
    byte_array right(size_t size) const;

    /**
     * Fill entire array to char @a ch.
     * If the size is specified, resizes the array beforehand.
     */
    byte_array& fill(char ch, int size = -1);

    /**
     * Interpret contents of byte_array buffer as if it was some arbitrary type.
     * Make sure byte_array has enough capacity to hold type T before calling this function.
     */
    template <typename T>
    T* as() {
        return reinterpret_cast<T*>(data());
    }

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
    static byte_array wrap(const char* data, size_t size);

    inline boost::string_ref
    string_view(size_t start_offset, size_t count = boost::string_ref::npos) const {
        return boost::string_ref(value).substr(start_offset, count);
    }

    container& as_vector() { return value; }
    container const& as_vector() const { return value; }
    std::string as_string() const { return std::string(value.begin(), value.end()); }

    inline iterator begin() { return value.begin(); }
    inline const_iterator begin() const { return value.begin(); }
    inline iterator end() { return value.end(); }
    inline const_iterator end() const { return value.end(); }
};

bool operator ==(const byte_array& a, const byte_array& b);
bool operator !=(const byte_array& a, const byte_array& b);

std::ostream& operator << (std::ostream& os, const byte_array& a);

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
