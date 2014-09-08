#include <iostream>
#include <boost/format.hpp>
#include "arsenal/hexdump.h"

using namespace std;

inline char printable(char c)
{
    if (c < 32) return '.';
    if (c > 126) return '.'; // 127 = DEL
    return c;
}

// @todo Add formatting width
// @todo Add lead indent printing
void hexdump(byte_array data, size_t octet_stride, size_t octet_split, size_t indent_spaces)
{
    size_t offset = 0;
    size_t remain = data.size();
    string spaces(indent_spaces, ' ');

    while (remain > 0)
    {
        cout << spaces << boost::format("%08x  ") % offset;
        size_t stride = remain < octet_stride ? remain : octet_stride;

        for(size_t i = 0; i < stride; ++i)
        {
            cout << boost::format("%02x ") % (int)(unsigned char)(data[i+offset]);
            if (i == octet_split - 1)
                cout << ' ';
        }
        if (stride < octet_stride)
        {
            if(stride < octet_split)
                cout << ' ';
            for(size_t i = 0; i < octet_stride - stride; ++i)
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
    cout << spaces << boost::format("%08x") % offset << endl;
}

