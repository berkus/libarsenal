//
// Part of Metta OS. Check http://atta-metta.net for latest version.
//
// Copyright 2007 - 2014, Stanislav Karchebnyy <berkus@atta-metta.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <string>

namespace encode
{

/**
 * Encode a binary buffer to Proquint[1] format for human-readability.
 * [1]: http://arxiv.org/html/0901.4016
 * @param  buffer A binary buffer of even size.
 * @return        Readable representation of the given buffer.
 */
std::string to_proquint(std::string buffer)
{
    auto consonants = "bdfghjklmnprstvz";
    auto vowels = "aiou";
    std::string out;
    out.reserve(buffer.size() * 3);

    assert(buffer.size() % 2 == 0);
    for (size_t i = 0; i < buffer.size() / 2; ++i)
    {
        uint16_t n = (buffer[i*2] << 8) + buffer[i*2+1];
        out.append(1, consonants[(n >> 12) & 0xf]);
        out.append(1, vowels[(n >> 10) & 0x3]);
        out.append(1, consonants[(n >> 6) & 0xf]);
        out.append(1, vowels[(n >> 4) & 0x3]);
        out.append(1, consonants[n & 0xf]);
        if (i != buffer.size() / 2 - 1) {
            out.append("-");
        }
    }
    return out;
}

/**
 * Decode a Proquint[1] format string to a binary buffer.
 * Minimal possible input string length is 5 characters.
 * Decoder does not enforce following Proquint word structure, so for example,
 * consonant-only strings are possible (bbbb is equivalent to babab).
 * [1]: http://arxiv.org/html/0901.4016
 * @param  text A human-readable string. At least 5 characters long.
 * @return      Binary representation of the given text.
 */
std::string from_proquint(std::string text)
{
    assert(text.size() >= 5);

    std::string buffer;
    buffer.resize(text.size()/3+1);

    size_t i{0};
    uint16_t n = 0;
    for (auto c : text)
    {
        switch (c)
        {
            case 'b': n <<= 4; n +=  0; break;
            case 'd': n <<= 4; n +=  1; break;
            case 'f': n <<= 4; n +=  2; break;
            case 'g': n <<= 4; n +=  3; break;
            case 'h': n <<= 4; n +=  4; break;
            case 'j': n <<= 4; n +=  5; break;
            case 'k': n <<= 4; n +=  6; break;
            case 'l': n <<= 4; n +=  7; break;
            case 'm': n <<= 4; n +=  8; break;
            case 'n': n <<= 4; n +=  9; break;
            case 'p': n <<= 4; n += 10; break;
            case 'r': n <<= 4; n += 11; break;
            case 's': n <<= 4; n += 12; break;
            case 't': n <<= 4; n += 13; break;
            case 'v': n <<= 4; n += 14; break;
            case 'z': n <<= 4; n += 15; break;
            case 'a': n <<= 2; n +=  0; break;
            case 'i': n <<= 2; n +=  1; break;
            case 'o': n <<= 2; n +=  2; break;
            case 'u': n <<= 2; n +=  3; break;
            case '-':
                buffer[i*2] = (n >> 8);
                buffer[i*2+1] = n & 0xff;
                ++i;
                n = 0;
                break;
            default:
                assert(false);
        }
    }
    buffer[i*2] = (n >> 8);
    buffer[i*2+1] = n & 0xff;

    return buffer;
}

} // encode namespace
