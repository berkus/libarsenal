//
// Part of Metta OS. Check http://atta-metta.net for latest version.
//
// Copyright 2007 - 2014, Stanislav Karchebnyy <berkus@atta-metta.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include <iostream>
#include <iomanip>
#include "arsenal/byte_array.h"

namespace arsenal
{

byte_array::byte_array()
    : value()
{}

byte_array::byte_array(byte_array const& other)
    : value(other.value)
{}

byte_array::byte_array(const char* str)
    : value(str, str+strlen(str)+1)
{}

byte_array::byte_array(const char* data, size_t size)
    : value(data, data+size)
{}

byte_array::byte_array(std::initializer_list<uint8_t> data)
{
    resize(data.size());
    uninitialized_copy(data.begin(), data.end(), value.begin());
}

byte_array::~byte_array()
{}

byte_array& byte_array::operator = (const byte_array& other)
{
    if (&other != this) {
        value = other.value;
    }
    return *this;
}

byte_array& byte_array::operator = (byte_array&& other)
{
    if (&other != this) {
        value = std::move(other.value);
    }
    return *this;
}

char* byte_array::data() {
    return &value[0];
}

const char* byte_array::data() const {
    return &value[0];
}

const char* byte_array::const_data() const {
    return &value[0];
}

size_t byte_array::size() const {
    return value.size();
}

char byte_array::at(int i) const {
    return value.at(i);
}

char byte_array::operator[](int i) const {
    return value[i];
}

char& byte_array::operator[](int i) {
    return value[i];
}

byte_array byte_array::left(size_t new_size) const
{
    new_size = std::min(new_size, size());
    return byte_array(const_data(), new_size);
}

byte_array byte_array::mid(int pos, size_t new_size) const
{
    new_size = std::min(new_size, size() - pos);
    return byte_array(const_data() + pos, new_size);
}

byte_array byte_array::right(size_t new_size) const
{
    new_size = std::min(new_size, size());
    return byte_array(const_data() + size() - new_size, new_size);
}

byte_array& byte_array::fill(char ch, int size)
{
    if (size != -1) {
        value.resize(size);
    }
    std::fill(value.begin(), value.end(), ch);
    return *this;
}

byte_array byte_array::wrap(const char* data, size_t size) {
    return byte_array(data, size);
}

bool operator == (const byte_array& a, const byte_array& b) {
    return a.value == b.value;
}

bool operator != (const byte_array& a, const byte_array& b) {
    return a.value != b.value;
}

std::ostream& operator << (std::ostream& os, byte_array const& a)
{
    for (size_t s = 0; s < a.size(); ++s) {
        os << std::setfill('0') << std::hex << std::setw(2) << (int)(unsigned char)(a.at(s)) << ' ';
    }
    return os;
}

} // arsenal namespace
