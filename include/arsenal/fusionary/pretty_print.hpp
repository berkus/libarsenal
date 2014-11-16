#pragma once

#include <ostream>

//=================================================================================================
// Pretty printer
//=================================================================================================

namespace detail {
    struct value_writer {
        std::ostream& os_;

        value_writer(std::ostream& os)
            : os_(os)
        {}

        template <typename T>
        auto operator ()(T const& v) const ->
            typename std::enable_if<!boost::fusion::traits::is_sequence<T>::value, value_writer const&>::type
        {
            os_ << v;
            return *this;
        }

        template <class T>
        auto operator()(T const& v) const ->
            typename std::enable_if<boost::fusion::traits::is_sequence<T>::value, value_writer const&>::type
        {
            boost::fusion::for_each(v, *this);
            return *this;
        }

        template <typename T>
        value_writer const& operator()(varsize_field_flag<T>& v) const
        {
            os_ << v.value;
            return *this;
        }

        template <typename T, typename I, size_t M, size_t O>
        value_writer const& operator()(varsize_field_specification<T,I,M,O>& v) const
        {
            os_ << "varsize_field[" << I::value << "]";
            return *this;
        }
    };

    namespace mpl = boost::mpl;
    namespace f_ext = boost::fusion::extension;

    template <class T>
    using range_c = typename mpl::range_c<int, 0, mpl::size<T>::value>;

    template <class T>
    struct mpl_visitor {
        value_writer w_;
        T const& msg_;

        mpl_visitor(std::ostream& os, T const& msg)
            : w_(os)
            , msg_(msg)
        {}

        template <class N>
        void operator()(N idx)
        {
            w_(f_ext::struct_member_name<T, N::value>::call())(":")
              (boost::fusion::at<N>(msg_))((idx != mpl::size<T>::value ? ", " : ""));
        }
    };

    template <class T>
    struct printer
    {
        T const& msg_;

        printer(T const& struc) : msg_(struc) {}

        friend std::ostream& operator<<(std::ostream& os, printer<T> const& v)
        {
            using namespace detail;
            mpl::for_each<range_c<T>>(mpl_visitor<T>(os, v.msg_));
            return os;
        }
    };

} // detail namespace

template <class T>
detail::printer<T> pretty_print(T const& v)
{
    return detail::printer<T>(v);
}
