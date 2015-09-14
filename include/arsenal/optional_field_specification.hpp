#pragma once

#define BOOST_OPTIONAL_NO_INPLACE_FACTORY_SUPPORT // hmm without this breaks using optional ctor
#include <boost/optional.hpp>

#include <bitset>
#include <climits>

// Variable size field or optional field flags
template <typename T, size_t N = CHAR_BIT * sizeof(T)>
struct field_flag
{
    using value_type = T;
    using bits_type = std::bitset<N>;
    value_type value;

    bool operator==(field_flag const& o) const
    {
        return value == o.value;
    }
};


// Type for Index type below
template <int N>
using field_index = boost::mpl::int_<N>;

// Optional value
template <typename Type, typename Index, size_t Num>
struct optional_field_specification : boost::optional<Type>
{
    using boost::optional<Type>::optional; // @todo Requires BOOST_OPTIONAL_NO_INPLACE_FACTORY_SUPPORT
    using boost::optional<Type>::value;
    using boost::optional<Type>::is_initialized;
    using index = Index;
    constexpr static const size_t bit = Num;

    bool operator==(optional_field_specification const& o) const
    {
        return is_initialized() ? o.is_initialized() && value() == o.value()
            : !o.is_initialized();
    }
};

// from above switcher struct we need to copy the appropriate field into output struct:
template <typename SwitchType, typename FinalType>
struct varsize_field_wrapper
{
    SwitchType choice_;
    FinalType output_;

    inline FinalType operator()(void) const { return output_; }
    inline operator FinalType() const { return output_; }
    inline FinalType value() const { return output_; }

    inline bool operator==(varsize_field_wrapper const& o) const
    {
        return output_ == o.output_;
    }
};

// * a variable sized field, where size is controlled by external bitfield with certain mapping
//   - where this bitfield is
//   - offset of bits
//   - mask of bits
//   - mapping function (bits to type)
template <typename Type, typename Index, size_t Mask, size_t Offset>
struct varsize_field_specification
{
    Type value; // varsized_field_wrapper
    using index = Index;
    constexpr static const size_t bit_mask = Mask;
    constexpr static const size_t bit_mask_offset = Offset;

    bool operator==(varsize_field_specification const& o) const
    {
        return value == o.value;
    }
};

