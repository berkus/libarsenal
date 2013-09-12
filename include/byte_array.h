//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2013, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <vector>
#include <utility>
#include <boost/array.hpp>

/**
 * Class mimicking Qt's QByteArray behavior using STL containers.
 */
class byte_array
{
    friend bool operator ==(const byte_array& a, const byte_array& b);
    std::vector<char> value; // XXX make implicitly shared cow?
public:
    typedef std::vector<char>::iterator iterator;
    typedef std::vector<char>::const_iterator const_iterator;

    byte_array();
    byte_array(const byte_array&);
    byte_array(const std::vector<char>&);
    byte_array(const char* str);
    byte_array(const char* data, size_t size);
    byte_array(std::initializer_list<uint8_t> data);
    explicit byte_array(size_t size) { resize(size); }

    template <typename T, size_t N>
    byte_array(const boost::array<T, N>& in) : value(in.begin(), in.begin() + N) {}

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

    /**
     * Fill entire array to char @a ch.
     * If the size is specified, resizes the array beforehand.
     */
    byte_array& fill(char ch, int size = -1);

    /**
     * Unlike Qt's fromRawData current implementation of
     * wrap does not actually wrap the data, it creates
     * its own copy. XXX fix it
     */
    static byte_array wrap(const char* data, size_t size);

    std::vector<char>& as_vector() { return value; }
    const std::vector<char>& as_vector() const { return value; }

    inline iterator begin() { return value.begin(); }
    inline const_iterator begin() const { return value.begin(); }
    inline iterator end() { return value.end(); }
    inline const_iterator end() const { return value.end(); }
};

// Hash specialization for byte_array
namespace std {

template<>
struct hash<byte_array> : public std::unary_function<byte_array, size_t>
{
    inline size_t operator()(const byte_array& a) const noexcept
    {
        // return std::hash_combine_range(a.begin(), a.end()); -- hopefully in c++1y
        // VEEERY bad implementation for now. @fixme
        size_t seed = 0xdeadbeef;
        for (auto x : a)
        {
            seed ^= x;
            seed <<= 3;
        }
        return seed;
    }
};

} // namespace std

bool operator ==(const byte_array& a, const byte_array& b);

std::ostream& operator << (std::ostream& os, const byte_array& a);

/**
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
