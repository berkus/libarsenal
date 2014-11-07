arsenal
=======

Support tools, classes and templates for C++11 projects.

This is a very small library of classes that are reused between several projects
and do not find their place anywhere in the bigger frameworks.

* algorithm - extended algorithms,
* any_int_cast - cast integer `boost::any` to integer type regardless of exact storage type,
* base32 - conversion between byte arrays and base32-encoded strings,
* base32x - conversion between byte arrays and base32x-encoded strings,
* base64 - conversion between byte arrays and base64-encoded strings,
* proquint - conversion between byte arrays and proquint-encoded strings,
* binary_literal - binary literal strings like `10011010_b`,
* byte_array - a convenient wrapper around `vector<char>`,
* flurry - msgpackv5-based (de-)serialization,
* stdext::hash_combine - implement C++14 `hash_combine`,
* logging - flexible lightweight logging,
* stdext::make_unique - drop-in replacement for `std::make_unique` missing from libc++,
* opaque_endian - opaque endianness-aware types `little_intXX`, `big_intXX` etc.,
* pubqueue - C++ trick to access underlying container of `std::queue`,
* settings_provider - a settings storage class abstracting platform-specific details,
* underlying - convert an enum class type to its underlying storage type.

[![Build Status](https://travis-ci.org/berkus/libarsenal.svg?branch=master)](https://travis-ci.org/berkus/libarsenal)
 [![Coverage Status](https://coveralls.io/repos/berkus/libarsenal/badge.png?branch=master)](https://coveralls.io/r/berkus/libarsenal?branch=master) [![Bitdeli Badge](https://d2weczhvl823v0.cloudfront.net/berkus/libarsenal/trend.png)](https://bitdeli.com/free "Bitdeli Badge")
