// Types needed:
// * buffer that runs from current point until the end of asio::buffer
//   (should be easy - when we see it in types, just consume the rest of the buffer)
//   sentinel type rest_t to grab until the rest of buffer... must be last in struct
// * a variable sized field, where size is controlled by external bitfield with certain mapping
//   - where this bitfield is
//   - offset of bits
//   - mask of bits
//   - mapping function (bits to type)
#include <array>
#include <iostream>
#include <boost/asio/buffer.hpp>
#include <boost/optional.hpp>
#include <boost/mpl/range_c.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/size.hpp>
#include <boost/fusion/include/value_at.hpp>
#include <boost/fusion/include/at.hpp>
#include <boost/fusion/include/define_struct.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/include/is_sequence.hpp>
#include <boost/range/has_range_iterator.hpp>
#define BOOST_TEST_MODULE Test_varsize_fields
#include <boost/test/unit_test.hpp>

//=================================================================================================
// Templated part
//=================================================================================================

using namespace std;
using boost::optional;
namespace asio = boost::asio;
namespace mpl = boost::mpl;
namespace fusion = boost::fusion;

constexpr unsigned int operator"" _bits_mask (unsigned long long bits)
{
    return (1 << bits) - 1;
}

// Optional bitmask position
template <typename T, size_t N = CHAR_BIT * sizeof(T)>
struct varsize_field_flag
{
    using value_type = T;
    using bits_type = std::bitset<N>;
};

template <typename T, size_t M, size_t O>
struct varsize_field_specification
{
    T value; // varsized_field_wrapper
    constexpr static const size_t bit_mask = M;
    constexpr static const size_t bit_mask_offset = O;
};

struct nothing_t
{
    template <typename T>
    operator T() { return T(); } // convert to anything - should actually return empty optional?
};

struct rest_t
{
    std::string data;
};

// from above switcher struct we need to copy the appropriate field into output struct:
template <typename SwitchType, typename FinalType>
struct varsize_field_wrapper
{
    SwitchType choice_;
    FinalType output_;

    inline FinalType value() const { return output_; }
};

template <class T>
using range_c = typename mpl::range_c<int, 0, mpl::size<T>::value>;

struct reader;

struct read_fields
{
    template <typename T, typename V>
    struct read_field_visitor
    {
        T& output_;
        V& result_;
        reader const& read_;
        uint8_t value_;

        read_field_visitor(T& out, V& result, reader const& r, uint8_t val)
            : output_(out)
            , result_(result)
            , read_(r)
            , value_(val)
        {}

        template <class N>
        void operator()(N idx)
        {
            if (N::value == value_) {
                read_(fusion::at<N>(output_));
                result_ = fusion::at<N>(output_);
            }
        }
    };

    template <typename T, typename V>
    void operator()(varsize_field_wrapper<T,V>& w, reader const& r, uint8_t value)
    {
        mpl::for_each<range_c<T>>(read_field_visitor<T,V>(w.choice_, w.output_, r, value));
    }
};

struct reader
{
    mutable asio::const_buffer buf_;
    // temp until i figure how to pass this state:
    mutable optional<varsize_field_flag<uint8_t>::bits_type> vars_;

    explicit reader(asio::const_buffer b) : buf_(std::move(b)) {}

    template <class T>
    auto operator()(T& val) const -> typename std::enable_if<std::is_integral<T>::value>::type
    {
        val = *asio::buffer_cast<T const*>(buf_);
        buf_ = buf_ + sizeof(T);
    }

    void operator()(varsize_field_flag<uint8_t>) const { // Temp: this also depends on type of parsed struct
        varsize_field_flag<uint8_t>::value_type val;
        (*this)(val);
        vars_ = varsize_field_flag<uint8_t>::bits_type(val);
    }

    template <typename T, size_t M, size_t O>
    void operator()(varsize_field_specification<T,M,O>& val) const
    {
        if (!vars_) {
            throw logic_error("Variable value comes before the defining flags");
        }
        varsize_field_flag<uint8_t>::value_type vflag = vars_->to_ulong();
        vflag = (vflag >> O) & M;
        (*this)(val.value, vflag);
    }

    template <typename T, typename V>
    void operator()(varsize_field_wrapper<T,V>& val, varsize_field_flag<uint8_t>::value_type flag_value) const // @todo
    {
        read_fields read_field_;
        read_field_(val, *this, flag_value);
    }

    template <class T>
    auto operator()(T & val) const ->
        typename std::enable_if<boost::fusion::traits::is_sequence<T>::value>::type
    {
        boost::fusion::for_each(val, *this);
    }

    void operator()(nothing_t&) const
    {
        // Do nothing!
    }

    // @todo Simply return the buf_ in remaining, to reduce copying etc.
    void operator()(rest_t& rest) const
    {
        rest.data = std::string(asio::buffer_cast<char const*>(buf_), asio::buffer_size(buf_));
        buf_ = buf_ + asio::buffer_size(buf_);
    }
};

//=================================================================================================
// User-defined part
//=================================================================================================

// User-defined mapping function (switcher bits value to type)
// based on the value of some field we must choose N-th value in this struct and read it
// index 0 - bits value 0, index 1 - bits value 1 and so on
// use a sentinel type nothing_t to read nothing.
// @todo When nothing_t is present we should be using optional<V> for output value?
BOOST_FUSION_DEFINE_STRUCT(
    (mapping), twobits,
    (nothing_t, no_value)
    (uint16_t, value1)
    (uint32_t, value2)
    (uint64_t, value3)
);

using flag_field_t = varsize_field_flag<uint8_t>;
using packet_size_t = varsize_field_wrapper<mapping::twobits, uint64_t>;
using packet_field_t = varsize_field_specification<packet_size_t, 2_bits_mask, 2>;

BOOST_FUSION_DEFINE_STRUCT(
    (actual), header_type,
    (flag_field_t, flags)
    (packet_field_t, packet_size)
);

BOOST_FUSION_DEFINE_STRUCT(
    (actual), packet_type,
    (actual::header_type, header1)
    (actual::header_type, header2)
    (actual::header_type, header3)
    (actual::header_type, header4)
    (rest_t, body)
);

std::array<uint8_t,23> buffer  = {{ 0x00,
                                    0x04, 0xab, 0xcd,
                                    0x08, 0xab, 0xcd, 0xef, 0x12,
                                    0x0c, 0xab, 0xcd, 0xef, 0x12, 0x34, 0x56, 0x78, 0x9a,
                                    'H', 'e', 'l', 'l', 'o' }};

BOOST_AUTO_TEST_CASE(basic_reader)
{
    asio::const_buffer buf(buffer.data(), buffer.size());
    reader read_(std::move(buf));
    actual::packet_type packet;

    read_(packet);

    BOOST_CHECK(packet.header1.packet_size.value.value() == 0);
    BOOST_CHECK(packet.header2.packet_size.value.value() == 0xcdab);
    BOOST_CHECK(packet.header3.packet_size.value.value() == 0x12efcdab);
    BOOST_CHECK(packet.header4.packet_size.value.value() == 0x9a78563412efcdab);
    BOOST_CHECK(packet.body.data == "Hello");
}
