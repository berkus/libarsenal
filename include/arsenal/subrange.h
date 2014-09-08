#pragma once

#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm_ext/overwrite.hpp>

template <typename T>
class subrange_impl
{
    T& container_;
    int start_offset_;
    int end_offset_;

    inline auto make_subrange()
    {
        return boost::iterator_range<typename T::iterator>(
            container_.begin() + start_offset_,
            container_.begin() + start_offset_ + end_offset_);
    }

public:
    inline subrange_impl(T& base, int start, int end)
        : container_(base)
        , start_offset_(start)
        , end_offset_(end)
    {}

    template <typename U>
    inline void operator = (U const& source) {
        boost::overwrite(source, make_subrange());
    }
};

template <typename T>
inline subrange_impl<T> subrange(T& base, int start, int end)
{
    return subrange_impl<T>(base, start, end);
}

