//
// Part of Metta OS. Check http://atta-metta.net for latest version.
//
// Copyright 2007 - 2014, Stanislav Karchebnyy <berkus@atta-metta.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <algorithm>
#include <type_traits>
#include <map>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <boost/tr1/array.hpp>

// The map types have find() member function for finding by key.
template<class T> struct is_map_type : public std::false_type {};

template<class K, class T, class Comp, class Alloc>
struct is_map_type<std::map<K, T, Comp, Alloc>> : public std::true_type {};

template<class K, class T, class Hash, class KeyEqual, class Alloc>
struct is_map_type<std::unordered_map<K, T, Hash, KeyEqual, Alloc>> : public std::true_type {};

// Return true if container c contains key k
// Container must be a collection with type Key for keys - @todo express constraint in code
template <typename Key, typename Container>
inline typename std::enable_if<is_map_type<Container>::value, bool>::type
contains(Container const& c, Key const& k)
{
    return c.find(k) != std::end(c);
}

template <typename Key, typename Container>
inline typename std::enable_if<!is_map_type<Container>::value, bool>::type
contains(Container const& c, Key const& k)
{
    return std::find(std::begin(c), std::end(c), k) != std::end(c);
}

// Convert an unordered set into a vector of the same types
template <typename T>
inline std::vector<T>
set_to_vector(std::unordered_set<T> const& source)
{
    std::vector<T> result;
    for (auto c : source) {
        result.emplace_back(c);
    }
    return result;
}

// Specialize tuple_size for boost::array<T,N>
namespace std {

template<class T, size_t N>
class tuple_size<boost::array<T, N>> : public integral_constant<size_t, N>
{};

} // std namespace
