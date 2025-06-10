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

// lz77.cpp: LZ77 Compression Algorithm

#include "lz77.h"
#include "q_shared.h"

#include <cstdio>
#include <cstring>

cLZ77 g_lz77;

unsigned int cLZ77::m_pDictionary[0xffff];

static void copy_bytes(unsigned char *dest, unsigned char *from, size_t length)
{
    size_t i;

    for (i = 0; i < length; i++) {
        dest[i] = from[i];
    }
}

cLZ77::cLZ77() {}

unsigned int cLZ77::CompressData(unsigned char *in, size_t in_len, unsigned char *out, size_t *out_len)
{
    this->in_end = &in[in_len];
    this->ip_end = &in[in_len - 13];
    this->op     = out;
    this->ip     = in;
    this->ii     = this->ip;
    this->ip += 4;

    while (this->ip < this->ip_end) {
        bool match_found = false;

        while (this->ip < this->ip_end) {
            // Primary hash index
            this->dindex =
                ((33 * (this->ip[0] ^ (32 * ((32 * ((this->ip[3] << 6) ^ this->ip[2])) ^ (unsigned int)this->ip[1]))))
                 >> 5)
                & 0x3FFF;
            this->m_off = cLZ77::m_pDictionary[this->dindex];

            if (this->ip - in <= this->m_off) {
                cLZ77::m_pDictionary[this->dindex] = this->ip - in;
                this->ip++;
                continue;
            }

            this->m_off = this->ip - in - this->m_off;
            if (this->m_off > 0xBFFF) {
                cLZ77::m_pDictionary[this->dindex] = this->ip - in;
                this->ip++;
                continue;
            }

            this->m_pos = this->ip - this->m_off;

            if (this->m_off <= 0x800 || (this->m_pos[3] == this->ip[3])) {
                if (*this->m_pos == *this->ip && this->m_pos[1] == this->ip[1] && this->m_pos[2] == this->ip[2]) {
                    match_found = true;
                    break;
                }
            }

            // Fallback hash index
            this->dindex = (this->dindex & 0x7FF) ^ 0x201F;
            this->m_off  = cLZ77::m_pDictionary[this->dindex];

            if (this->ip - in <= this->m_off) {
                cLZ77::m_pDictionary[this->dindex] = this->ip - in;
                this->ip++;
                continue;
            }

            this->m_off = this->ip - in - this->m_off;
            if (this->m_off > 0xBFFF) {
                cLZ77::m_pDictionary[this->dindex] = this->ip - in;
                this->ip++;
                continue;
            }

            this->m_pos = this->ip - this->m_off;

            if (this->m_off <= 0x800 || (this->m_pos[3] == this->ip[3])) {
                if (this->m_pos[0] == this->ip[0] && this->m_pos[1] == this->ip[1] && this->m_pos[2] == this->ip[2]) {
                    match_found = true;
                    break;
                }
            }

            cLZ77::m_pDictionary[this->dindex] = this->ip - in;
            this->ip++;
        }

        if (!match_found) {
            break;
        }

        cLZ77::m_pDictionary[this->dindex] = this->ip - in;

        unsigned int t = this->ip - this->ii;
        if (t > 0) {
            if (t <= 3) {
                *(this->op - 2) |= t;
            } else if (t <= 18) {
                *this->op++ = t - 3;
            } else {
                unsigned int tt = t - 18;

                *this->op++ = 0;
                while (tt > 255) {
                    tt -= 255;
                    *this->op++ = 0;
                }
                *this->op++ = tt;
            }

            copy_bytes(op, ii, t);
            ii += t;
            op += t;
        }

        this->ip += 3;

        for (t = 0; ip < in_end; t++, this->ip++) {
            if (this->m_pos[t + 3] != *this->ip) {
                break;
            }
        }

        this->m_len = this->ip - this->ii;

        if (this->m_off > 0x4000) {
            this->m_off -= 0x4000;
            if (this->m_len > 9) {
                this->m_len -= 9;
                *this->op++ = ((this->m_off & 0x4000) >> 11) | 0x10;
                while (this->m_len > 0xFF) {
                    this->m_len -= 255;
                    *this->op++ = 0;
                }
                *this->op++ = this->m_len;
            } else {
                *this->op++ = ((this->m_off & 0x4000) >> 11) | ((this->m_len & 0xFF) - 2) | 0x10;
            }
        } else {
            --this->m_off;
            if (this->m_len > 33) {
                this->m_len -= 33;
                *this->op++ = 32;
                while (this->m_len > 255) {
                    this->m_len -= 255;
                    *this->op++ = 0;
                }
                *this->op++ = this->m_len;
            } else {
                *this->op++ = ((this->m_len & 0xFF) - 2) | 0x20;
            }
        }

        *this->op++ = 4 * (this->m_off & 63);
        *this->op++ = this->m_off >> 6;
        this->ii    = this->ip;
    }

    *out_len = this->op - out;
    return this->in_end - this->ii;
}

int cLZ77::Compress(unsigned char *in, size_t in_len, unsigned char *out, size_t *out_len)
{
    byte  *op = out;
    size_t t  = 0;

    if (in_len == 0) {
        *out_len = 0;
        return 0;
    }

    if (in_len > 13) {
        t  = CompressData(in, in_len, out, out_len);
        op = out + *out_len;
    } else {
        t = in_len;
    }

    if (t) {
        if (op == out && t <= 238) {
            *op++ = t + 17;
        } else if (t <= 3) {
            *(op - 2) |= t;
        } else if (t <= 18) {
            *op++ = t - 3;
        } else {
            unsigned int tt;

            *op++ = 0;

            tt = t - 18;
            while (tt > 255) {
                tt -= 255;
                *op++ = 0;
            }

            *op++ = tt;
        }

        copy_bytes(op, &in[in_len - t], t);
        op += t;
    }

    *op++    = 17;
    *op++    = 0;
    *op++    = 0;
    *out_len = op - out;

    return 0;
}

static unsigned int decode_length(unsigned int base, unsigned char *& ip)
{
    unsigned int len = base;
    while (!*ip) {
        len += 255;
        ++ip;
    }
    return len + *ip++;
}

int cLZ77::Decompress(unsigned char *in, size_t in_len, unsigned char *out, size_t *out_len)
{
    unsigned int   t;
    unsigned short s;

    ip_end   = &in[in_len];
    ip       = in;
    op       = out;
    *out_len = 0;

    if (*ip > 17u) {
        t = *ip++ - 17;
        if (t <= 3) {
            copy_bytes(op, ip, t);
            op += t;
            ip += t;
            t = *ip++;
        } else {
            copy_bytes(op, ip, t);
            op += t;
            ip += t;
            t = *ip++;
        }
    } else {
        t = *ip++;
    }

    for (;;) {
        if (t <= 15) {
            if (t == 0) {
                t = decode_length(15, ip);
            }

            memcpy(op, ip, 4);
            op += 4;
            ip += 4;
            t--;

            if (t) {
                if (t <= 3) {
                    copy_bytes(op, ip, t);
                    op += t;
                    ip += t;
                } else {
                    while (t > 3) {
                        memcpy(op, ip, 4);
                        op += 4;
                        ip += 4;
                        t -= 4;
                    }
                    copy_bytes(op, ip, t);
                    op += t;
                    ip += t;
                }
            }

            t = *ip++;
            if (t <= 15) {
                m_pos = op - 2049 - (t >> 2) - 4 * *ip++;
                *op++ = *m_pos++;
                *op++ = *m_pos++;
                *op++ = *m_pos++;
                continue;
            }
        }

        while (true) {
            if (t > 63) {
                m_pos = op - 1 - ((t >> 2) & 7) - 8 * *ip++;
                t     = (t >> 5) - 1;
                *op++ = *m_pos++;
                *op++ = *m_pos++;
                copy_bytes(op, m_pos, t);
                op += t;
                break;
            }

            if (t > 31) {
                t &= 31;
                if (t == 0) {
                    t = decode_length(31, ip);
                }

                m_pos = op - 1;
                CopyLittleShort(&s, ip);
                ip += 2;
                m_pos -= (s >> 2);
            } else {
                if (t <= 15) {
                    m_pos = op - 1 - (t >> 2) - 4 * *ip++;
                    *op++ = *m_pos++;
                    *op++ = *m_pos++;
                    break;
                }

                m_pos = op - 2048 * (t & 8);
                t &= 7u;
                if (t == 0) {
                    t = decode_length(7, ip);
                }

                CopyLittleShort(&s, ip);
                ip += 2;
                m_pos -= (s >> 2);

                if (m_pos == op) {
                    *out_len = op - out;
                    return 0;
                }
                m_pos -= 0x4000;
            }

            if (t <= 5 || static_cast<size_t>(op - m_pos) <= 3) {
                *op++ = *m_pos++;
                *op++ = *m_pos++;
                copy_bytes(op, m_pos, t);
                op += t;
            } else {
                memcpy(op, m_pos, 4);
                op += 4;
                m_pos += 4;
                t -= 2;
                while (t > 3) {
                    memcpy(op, m_pos, 4);
                    op += 4;
                    m_pos += 4;
                    t -= 4;
                }
                copy_bytes(op, m_pos, t);
                op += t;
            }
            break;
        }

        t = *(ip - 2) & 3;
        if (t == 0) {
            t = *ip++;
            continue;
        }

        copy_bytes(op, ip, t);
        op += t;
        ip += t;
        t = *ip++;
    }

    *out_len = op - out;
    if (ip == ip_end) {
        return 0;
    }

    return (ip < ip_end) ? -1 : -2;
}

static unsigned char in[0x40000];
static unsigned char out[0x41013];

static void test_compression()
{
    size_t in_len;
    size_t out_len;
    size_t new_len;
    cLZ77  lz77;

    memset(&in, 0, 0x40000);

    if (lz77.Compress(in, sizeof(in), out, &out_len)) {
        puts("Compression Failed!");
        return;
    }

    printf("Compressed %i bytes into %zi bytes\n", 0x40000, out_len);

    if (lz77.Decompress(out, out_len, in, &in_len)) {
        new_len = in_len;
    } else {
        new_len = in_len;

        if (in_len == 0x40000) {
            printf("Decompressed %zi bytes into %i bytes\n", out_len, 0x40000);
            puts("Compression Test: Passed");
            return;
        }
    }

    printf("Decompression got FuBar'd... %i != %zi\n", 0x40000, new_len);
}
