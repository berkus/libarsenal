/*
 * Copyright (C) 2001-2005 Jacek Sieka, arnetheduck on gmail point com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
//from http://sourcecodebrowser.com/dc-qt/0.2.0.alpha/_encoder_8cpp_source.html
// tables adjusted according to base32x for better readability

#include "arsenal/base32x.h"
#include <cassert>

namespace {
const int8_t base32xTable[] = {
       -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
       -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
       -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
       -1,-1,24,25,26,27,28,29,30,31,-1,-1,-1,-1,-1,-1,//'0','1','2','3','4','5','6','7','8','9',':',';','<','=','>','?'
       -1, 0, 1, 2, 3, 4, 5, 6, 7,-1, 8, 9,10,11,12,-1,//'@','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O'
       13,14,15,16,17,18,19,20,21,22,23,-1,-1,-1,-1,-1,//'P','Q','R','S','T','U','V','W','X','Y','Z','[','\',']','^','_'
       -1, 0, 1, 2, 3, 4, 5, 6, 7,-1, 8, 9,10,11,12,-1,//'`','a','b','c','d','e','f','g','h','i','j','k','l','m','n','o'
       13,14,15,16,17,18,19,20,21,22,23,-1,-1,-1,-1,-1,//'p','q','r','s','t','u','v','w','x','y','z','{','|','}','~','DEL'
       -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
       -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
       -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
       -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
       -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
       -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
       -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
       -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
};

const char base32xAlphabet[] = "ABCDEFGHJKLMNPQRSTUVWXYZ23456789";
}

namespace encode {

// @todo Use transform_width<const char*,5,8> from boost::archive?
std::string to_base32x(const std::string& src)
{
    // Code snagged from the bitzi bitcollider
    size_t i, index;
    uint8_t word;
    size_t len = src.length();
    std::string dst;
    dst.reserve(((len * 8) / 5) + 1);

    for(i = 0, index = 0; i < len;)
    {
        /* Is the current word going to span a byte boundary? */
        if (index > 3) {
            word = ((uint8_t)src.at(i) & (0xFF >> index));
            index = (index + 5) % 8;
            word <<= index;
            if ((i + 1) < len) {
                word |= ((uint8_t)src.at(i + 1) >> (8 - index));
            }
            i++;
        } else {
            word = ((uint8_t)src.at(i) >> (8 - (index + 5))) & 0x1F;
            index = (index + 5) % 8;
            if (index == 0) {
                i++;
            }
        }
        assert(word < 32);
        dst += base32xAlphabet[word];
    }
    return dst;
}

std::string from_base32x(const std::string& src)
{
    size_t i, index;
    size_t offset;
    size_t len = src.length();
    std::string dst(0, ((len * 5 + 7) / 8));

    for(i = 0, index = 0, offset = 0; i < len; i++)
    {
        // Skip what we don't recognise
        int8_t tmp = base32xTable[(uint8_t)src.at(i)];

        if(tmp == -1)
            continue;

        if (index <= 3) {
            index = (index + 5) % 8;
            if (index == 0) {
                dst[offset] = dst[offset] | tmp;
                offset++;
                if(offset == len)
                    break;
            } else {
                dst[offset] = dst[offset] | tmp << (8 - index);
            }
        } else {
            index = (index + 5) % 8;
            dst[offset] = dst[offset] | (tmp >> index);
            offset++;
            if(offset == len)
                break;
            dst[offset] = dst[offset] | tmp << (8 - index);
        }
    }
    return dst;
}

}
