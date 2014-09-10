#pragma once

#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm_ext/overwrite.hpp>

template <typename T>
class subrange_impl
{
    T& container_;
    int start_offset_;
    int end_offset_;

    inline auto make_subrange() const
    {
        return boost::iterator_range<typename T::iterator>(
            container_.begin() + start_offset_,
            container_.begin() + start_offset_ + end_offset_);
    }

public:
    typedef typename T::iterator iterator;
    typedef typename T::const_iterator const_iterator;

    inline subrange_impl(T& base, int start, int end)
        : container_(base)
        , start_offset_(start)
        , end_offset_(end)
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
inline subrange_impl<T> subrange(T& base, int start, int end)
{
    return subrange_impl<T>(base, start, end);
}

