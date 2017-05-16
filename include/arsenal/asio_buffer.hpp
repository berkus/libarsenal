//
// Part of Metta OS. Check http://atta-metta.net for latest version.
//
// Copyright 2007 - 2015, Stanislav Karchebnyy <berkus@atta-metta.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <boost/asio/buffer.hpp>
#include <boost/utility/string_ref.hpp>
#include <iomanip>
#include <ostream>

namespace boost::asio {

inline std::ostream& operator << (std::ostream& os, const_buffer const& b)
{
    os << '[';
    auto r = boost::asio::buffer_cast<unsigned char const*>(b);
    auto s = boost::basic_string_ref<unsigned char>(r, boost::asio::buffer_size(b));
    bool first = true;
    for (auto c : s) { 
        os << (first ? "" : " ") << std::setw(2) << std::setfill('0') << std::hex << static_cast<unsigned int>(c);
        first = false;
    }
    os << ']';
    return os;
}

inline std::ostream& operator << (std::ostream& os, mutable_buffer const& b)
{
    os << '[';
    auto r = boost::asio::buffer_cast<unsigned char const*>(b);
    auto s = boost::basic_string_ref<unsigned char>(r, boost::asio::buffer_size(b));
    bool first = true;
    for (auto c : s) { 
        os << (first ? "" : " ") << std::setw(2) << std::setfill('0') << std::hex << static_cast<unsigned int>(c);
        first = false;
    }
    os << ']';
    return os;
}

} // boost::asio namespace
