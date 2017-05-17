#define BOOST_TEST_MODULE Test_fusionary_types
#include <boost/test/unit_test.hpp>

#include <boost/endian/arithmetic.hpp>
#include "arsenal/fusionary.hpp"
#include "arsenal/optional_field_specification.hpp"

#include <iostream>

using namespace boost::asio;
using namespace boost::endian;
using namespace arsenal;
using optional_test_field = arsenal::optional_field_specification<uint32_t, field_index<0>, 0_bits_shift>;

BOOST_FUSION_DEFINE_STRUCT(
    (), test_optional_field_struct,
    (field_flag<uint8_t>, flags)
    (optional_test_field, test_field)
);

BOOST_AUTO_TEST_CASE(optional_types_check)
{
    char b[5000];
    mutable_buffer mbuf(b, 5000);
    const_buffer cbuf(b, 5000);
    {
        test_optional_field_struct ss;
        ss.flags.value = 1;
        ss.test_field = 100;
        mbuf = fusionary::write(mbuf, ss);
        test_optional_field_struct ssr;
        cbuf = fusionary::read(ssr, cbuf);
        BOOST_REQUIRE_EQUAL(ss.flags.value, ssr.flags.value);
        BOOST_REQUIRE_EQUAL(ss.test_field.get(), ssr.test_field.get());
    }
    {
        test_optional_field_struct ss;
        ss.flags.value = 0;
        ss.test_field = boost::none;
        mbuf = fusionary::write(mbuf, ss);
        test_optional_field_struct ssr;
        ssr.flags.value = 1;
        ssr.test_field = 100;
        cbuf = fusionary::read(ssr, cbuf);
        BOOST_REQUIRE_EQUAL(ss.flags.value, ssr.flags.value);
        BOOST_REQUIRE(!ssr.test_field);
    }
}

BOOST_AUTO_TEST_CASE(endians_check)
{
    char b[5000];
    mutable_buffer mbuf(b, 5000);
    const_buffer cbuf(b, 5000);
    {
        big_uint64_t i = 64; 
        big_uint64_t j = 0;
        mbuf = fusionary::write(mbuf, i);
        cbuf = fusionary::read(j, cbuf);
        BOOST_REQUIRE_EQUAL(i, j);
    }
    {
        big_uint32_t i = 32; 
        big_uint32_t j = 0;
        mbuf = fusionary::write(mbuf, i);
        cbuf = fusionary::read(j, cbuf);
        BOOST_REQUIRE_EQUAL(i, j);
    }
    {
        big_uint16_t i = 16; 
        big_uint16_t j = 0;
        mbuf = fusionary::write(mbuf, i);
        cbuf = fusionary::read(j, cbuf);
        BOOST_REQUIRE_EQUAL(i, j);
    }
    {
        big_uint8_t i = 8; 
        big_uint8_t j = 0;
        mbuf = fusionary::write(mbuf, i);
        cbuf = fusionary::read(j, cbuf);
        BOOST_REQUIRE_EQUAL(i, j);
    }
    {
        big_int64_t i = 64; 
        big_int64_t j = 0;
        mbuf = fusionary::write(mbuf, i);
        cbuf = fusionary::read(j, cbuf);
        BOOST_REQUIRE_EQUAL(i, j);
    }
    {
        big_int32_t i = 32; 
        big_int32_t j = 0;
        mbuf = fusionary::write(mbuf, i);
        cbuf = fusionary::read(j, cbuf);
        BOOST_REQUIRE_EQUAL(i, j);
    }
    {
        big_int16_t i = 16; 
        big_int16_t j = 0;
        mbuf = fusionary::write(mbuf, i);
        cbuf = fusionary::read(j, cbuf);
        BOOST_REQUIRE_EQUAL(i, j);
    }
    {
        big_int8_t i = 8; 
        big_int8_t j = 0;
        mbuf = fusionary::write(mbuf, i);
        cbuf = fusionary::read(j, cbuf);
        BOOST_REQUIRE_EQUAL(i, j);
    }
    {
        little_uint64_t i = 64; 
        little_uint64_t j = 0;
        mbuf = fusionary::write(mbuf, i);
        cbuf = fusionary::read(j, cbuf);
        BOOST_REQUIRE_EQUAL(i, j);
    }
    {
        little_uint32_t i = 32; 
        little_uint32_t j = 0;
        mbuf = fusionary::write(mbuf, i);
        cbuf = fusionary::read(j, cbuf);
        BOOST_REQUIRE_EQUAL(i, j);
    }
    {
        little_uint16_t i = 16; 
        little_uint16_t j = 0;
        mbuf = fusionary::write(mbuf, i);
        cbuf = fusionary::read(j, cbuf);
        BOOST_REQUIRE_EQUAL(i, j);
    }
    {
        little_uint8_t i = 8; 
        little_uint8_t j = 0;
        mbuf = fusionary::write(mbuf, i);
        cbuf = fusionary::read(j, cbuf);
        BOOST_REQUIRE_EQUAL(i, j);
    }
    {
        little_int64_t i = 64; 
        little_int64_t j = 0;
        mbuf = fusionary::write(mbuf, i);
        cbuf = fusionary::read(j, cbuf);
        BOOST_REQUIRE_EQUAL(i, j);
    }
    {
        little_int32_t i = 32; 
        little_int32_t j = 0;
        mbuf = fusionary::write(mbuf, i);
        cbuf = fusionary::read(j, cbuf);
        BOOST_REQUIRE_EQUAL(i, j);
    }
    {
        little_int16_t i = 16; 
        little_int16_t j = 0;
        mbuf = fusionary::write(mbuf, i);
        cbuf = fusionary::read(j, cbuf);
        BOOST_REQUIRE_EQUAL(i, j);
    }
    {
        little_int8_t i = 8; 
        little_int8_t j = 0;
        mbuf = fusionary::write(mbuf, i);
        cbuf = fusionary::read(j, cbuf);
        BOOST_REQUIRE_EQUAL(i, j);
    }
}
