// Types needed:
// * buffer that runs from a point until the end of asio::buffer
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
#include <boost/mpl/range_c.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/size.hpp>
#include <boost/fusion/include/value_at.hpp>
#include <boost/fusion/include/at.hpp>
#include <boost/fusion/include/define_struct.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/include/is_sequence.hpp>
#include <boost/range/has_range_iterator.hpp>
#define BOOST_TEST_MODULE Test_opaque_endians
#include <boost/test/unit_test.hpp>


using namespace std;
namespace asio = boost::asio;
namespace mpl = boost::mpl;
namespace fusion = boost::fusion;

template <typename T, size_t M, size_t O>
struct varsize_field
{
    T value;
    constexpr static const size_t bit_mask = M;
    constexpr static const size_t bit_mask_offset = O;
};

// Optional bitmask position
// template <typename T, size_t N = CHAR_BIT * sizeof(T)>
// struct optional_field_set
// {
//     using value_type = T;
//     using bits_type = std::bitset<N>;
// };

// 0 = uint16_t, 1 = uint32_t (bit0)
std::array<uint8_t,5> buffer  = {{ 0x00, 0xab, 0xcd, 0xef, 0x12 }};
// std::array<uint8_t,5> buffer2 = {{ 0x01, 0xab, 0xcd, 0xef, 0x12 }};

struct reader
{
    mutable asio::const_buffer buf_;

    explicit reader(asio::const_buffer b) : buf_(std::move(b)) {}

    template <class T>
    auto operator()(T& val) const -> typename std::enable_if<std::is_integral<T>::value>::type
    {
        val = *asio::buffer_cast<T const*>(buf_);
        buf_ = buf_ + sizeof(T);
    }
};

// template <typename T>
// std::pair<T, const_buffer> read(const_buffer b)
// {
//     reader r(std::move(b));
//     T res;
//     fusion::for_each(res, r);
//     return std::make_pair(res, r.buf_);
// }

// - mapping function (bits to type)
// based on the value of some field we must choose N-th value in this struct and read it
// index 0 - bits value 0, index 1 - bits value 1 and so on
// use a sentinel type nothing_t to read nothing.
BOOST_FUSION_DEFINE_STRUCT(
    (mapping), twobits,
    (uint16_t, value1)
    (uint32_t, value2)
);

template <class T>
using range_c = typename mpl::range_c<int, 0, mpl::size<T>::value>;

template <typename T>
struct read_field_visitor
{
    T& output_;
    reader& read_;
    uint8_t value_;

    read_field_visitor(T& out, reader& r, uint8_t val) : output_(out), read_(r), value_(val) {}

    template <class N>
    void operator()(N idx)
    {
        if (N::value == value_) {
            read_(fusion::at<N>(output_));
        }
    }
};

struct read_fields
{
    template <typename T>
    void operator()(T& out, reader& r, uint8_t value)
    {
    // fusion::for_each(flags == x && r(fusion::at<mpl::int_<x>>(stru)));
        mpl::for_each<range_c<T>>(read_field_visitor<T>(out, r, value));
    }
};

BOOST_AUTO_TEST_CASE(basic_reader)
{
    asio::const_buffer buf(buffer.data(), buffer.size());
    reader r(std::move(buf));
    read_fields rf;

    uint8_t flags;
    mapping::twobits stru;

    r(flags);
    rf(stru, r, flags);

    BOOST_CHECK(flags == 0);
    BOOST_CHECK(stru.value1 == 0xcdab);

    cout << (int)flags << " / " << stru.value1 << endl;
}
