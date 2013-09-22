//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2013, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/format.hpp>
#include "flurry.h"
#include "logging.h"
#include "byte_array_wrap.h"

using namespace std;

inline char printable(char c)
{
    if (c < 32) return '.';
    if (c > 126) return '.'; // 127 = DEL
    return c;
}

void hexdump(byte_array data)
{
    size_t offset = 0;
    size_t remain = data.size();

    while (remain > 0)
    {
        cout << boost::format("%08x  ") % offset;
        size_t stride = remain < 16 ? remain : 16;

        for(size_t i = 0; i < stride; ++i)
        {
            cout << boost::format("%02x ") % (int)(unsigned char)(data[i+offset]);
            if (i == 7)
                cout << ' ';
        }
        if (stride < 16)
        {
            if(stride < 8)
                cout << ' ';
            for(size_t i = 0; i < 16 - stride; ++i)
                cout << "   ";
        }
        cout << " |";
        for(size_t i = 0; i < stride; ++i)
        {
            cout << boost::format("%c") % printable(data[i+offset]);
        }
        cout << "|" << endl;

        remain -= stride;
        offset += stride;
    }
    cout << boost::format("%08x") % offset << endl;
}

int main()
{
    byte_array data;
    std::ifstream in("dump.bin", std::ios::in|std::ios::binary);
    flurry::iarchive ia(in);

    while (ia >> data) {
        cout << "*** BLOB " << data.size() << " bytes ***" << endl;
        hexdump(data);
    }
}
