/*
===========================================================================
Copyright (C) 2025 the OpenMoHAA team

This file is part of OpenMoHAA source code.

OpenMoHAA source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

OpenMoHAA source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with OpenMoHAA source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include "../lz77.h"

#include <cstdio>
#include <cstring>
#include <iostream>

static unsigned char in[0x40000];
static unsigned char out[0x41013];
static size_t        out_len;

bool test_compression()
{
    cLZ77 lz77;

    memset(&in, 0, 0x40000);

    if (lz77.Compress(in, sizeof(in), out, &out_len)) {
        return false;
    }

    std::cout << "Compressed " << sizeof(in) << " bytes into " << out_len << " bytes" << std::endl;
    return true;
}

bool test_decompression()
{
    size_t in_len;
    size_t new_len;
    cLZ77  lz77;

    in_len = 0;
    lz77.Decompress(out, out_len, in, &in_len);
    new_len = in_len;

    if (in_len != 0x40000) {
        std::cerr << "Decompression got FuBar'd... " << sizeof(in) << " != " << new_len << std::endl;
        return false;
    }

    std::cout << "Decompressed " << out_len << " bytes into " << sizeof(in) << " bytes" << std::endl;

    return true;
}

int main(int argc, char *argv[])
{
    if (!test_compression()) {
        std::cerr << "Compression Failed!" << std::endl;
        return 1;
    }

    if (!test_decompression()) {
        std::cerr << "Decompression Failed!" << std::endl;
        return 2;
    }

    return 0;
}
