//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2013, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <algorithm>

// Return true if container c contains key k
// Container must be a collection with type Key for keys - @todo express constraint in code
template <typename Key, typename Container>
inline bool contains(Container const& c, Key const& k)
{
    return c.find(k) != end(c);
}
