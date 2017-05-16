//
// Part of Metta OS. Check http://atta-metta.net for latest version.
//
// Copyright 2007 - 2014, Stanislav Karchebnyy <berkus@atta-metta.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm_ext/overwrite.hpp>

namespace arsenal
{

template <typename T>
class subrange_impl // @todo use std::string_view and ranges ts
{
    T& container_;
    int start_offset_;
    int size_;

    inline auto make_subrange() const
    {
        return boost::iterator_range<typename T::iterator>(
            container_.begin() + start_offset_,
            container_.begin() + start_offset_ + size_);
    }

public:
    using iterator = typename T::iterator;
    using const_iterator = typename T::const_iterator;

    inline subrange_impl(T& base, int start, int size)
        : container_(base)
        , start_offset_(start)
        , size_(size)
    {}

    template <typename U>
    inline void operator = (U const& source) {
        boost::overwrite(source, make_subrange());
    }

    inline void operator = (subrange_impl<T> const& source) {
        boost::overwrite(source.make_subrange(), make_subrange());
    }

    template <typename U>
    inline bool operator == (U const& source) const {
        return boost::equal(source, make_subrange());
    }

    typename boost::range_iterator<T>::type begin() { return make_subrange().begin(); }
    typename boost::range_iterator<T>::type end() { return make_subrange().end(); }
    typename boost::range_iterator<T>::type begin() const { return make_subrange().begin(); }
    typename boost::range_iterator<T>::type end() const { return make_subrange().end(); }

    operator T() const {
        return T(begin(), end());
    }
};

template <typename T>
inline subrange_impl<T> subrange(T& base, int start, int size)
{
    return subrange_impl<T>(base, start, size);
}

template <typename T>
inline subrange_impl<T> subrange(T& base, int start)
{
    return subrange_impl<T>(base, start, base.size() - start);
}

} // arsenal namespace
