#pragma once

#include "arsenal/byte_array.h"

namespace arsenal::debug
{

/// octet_stride specifies number of bytes to print in one column
/// octet_split causes run of bytes to be separated by extra space in given column
/// setting it to 0 disables separation
void hexdump(byte_array data,
             size_t octet_stride = 16,
             size_t octet_split = 8,
             size_t indent_spaces = 0);

} // arsenal::debug namespace
