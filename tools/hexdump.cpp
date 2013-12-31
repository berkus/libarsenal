#include <boost/format.hpp>
#include "hexdump.h"

using namespace std;

inline char printable(char c)
{
    if (c < 32) return '.';
    if (c > 126) return '.'; // 127 = DEL
    return c;
}

// @todo Add formatting width
// @todo Add lead indent printing
void hexdump(byte_array data/*int octet_stride, int octet_split, int indent_spaces*/)
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

