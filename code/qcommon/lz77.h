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

// lz77.h: LZ77 Compression Algorithm

#pragma once

#include <cstddef>

class cLZ77
{
    static unsigned int m_pDictionary[];

    unsigned char *ip;
    unsigned char *op;
    unsigned char *in_end;
    unsigned char *ip_end;
    unsigned char *ii;
    unsigned char *m_pos;
    unsigned int   m_off;
    unsigned int   m_len;
    unsigned int   dindex;

public:
    cLZ77();

    /**
     * @brief Compress a block of data using an LZ77 coder.
     * 
     * @param in Input (uncompressed) buffer.
     * @param in_len Number of input bytes.
     * @param out Output (compressed) buffer. This buffer must be 0.4% larger than the input buffer, plus one byte.
     * @param out_len Output length.
     * @return Always return 0.
     */
    int Compress(unsigned char *in, size_t in_len, unsigned char *out, size_t *out_len);

    /**
     * @brief Uncompress a block of data using an LZ77 decoder.
     * 
     * @param in Input (compressed) buffer.
     * @param in_len Number of input bytes.
     * @param out Output (uncompressed) buffer. This buffer must be large enough to hold the uncompressed data.
     * @param out_len Output length.
     * @return 0 on success. -1 if not enough data was read, -2 if too much data was read.
     */
    int Decompress(unsigned char *in, size_t in_len, unsigned char *out, size_t *out_len);

private:
    unsigned int CompressData(unsigned char *in, size_t in_len, unsigned char *out, size_t *out_len);
};

extern cLZ77 g_lz77;
