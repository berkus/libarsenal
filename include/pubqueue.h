//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2013, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <queue>
#include <deque>

// Derive from queue and make c member public.
template <class T, class C = std::deque<T>>
struct pubqueue : std::queue<T, C>
{
    using std::queue<T,C>::c;

    static C& get_c(std::queue<T,C>& s) {
        return s.*&pubqueue::c;
    }
    static C const& get_c(std::queue<T,C> const& s) {
        return s.*&pubqueue::c;
    }
};

// Provide public access to the underlying container c of queue.
// Based on stackoverflow answer: http://stackoverflow.com/a/5877652/145434
template<class T, class C>
C& get_c(std::queue<T,C>& a) {
    return pubqueue<T,C>::get_c(a);
}

template<class T, class C>
C& get_c(std::queue<T, C> const& a) {
    return pubqueue<T,C>::get_c(a);
}
