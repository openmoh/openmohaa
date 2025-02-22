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

#include "gcrypt.h"
#include "../qcommon/q_platform.h"
#include "../qcommon/q_shared.h"

static unsigned int rotl32(unsigned int value, unsigned int count)
{
    return (value << count) | (value >> (-count & 31));
}

static unsigned int crypt_seek(GCryptInfo *info, unsigned int n1, unsigned int n2)
{
    int part;
    int i;
    int keyIndex;
    int xorKey;
    int current;

    current  = 0x8000;
    xorKey   = n1;
    keyIndex = 0;
    i        = 1;

    info->offset = 0;

    while (current > 0) {
        xorKey += i;
        keyIndex += xorKey;
        xorKey += keyIndex;

        if (n2 & current) {
            part   = rotl32(~xorKey, 24);
            xorKey = rotl32(info->key[part & 0xff] ^ part, 24);
            part   = rotl32(info->key[keyIndex & 0xff] ^ keyIndex, 8);
            xorKey ^= info->key[xorKey & 0xff];
            keyIndex = rotl32(info->key[part & 0xff] ^ part, 8);

            i += 1 + i;
        } else {
            info->msg[info->offset]      = xorKey;
            info->msg[info->offset + 16] = keyIndex;
            info->msg[info->offset + 32] = i;
            info->offset++;

            part     = rotl32(keyIndex, 24);
            keyIndex = rotl32(info->key[part & 0xff] ^ part, 24);
            part     = rotl32(info->key[xorKey & 0xff] ^ xorKey, 8);
            keyIndex ^= info->key[keyIndex & 0xff];
            xorKey = rotl32(info->key[part & 0xff] ^ part, 8);

            i *= 2;
        }

        current >>= 1;
    }

    info->xorKey   = xorKey;
    info->keyIndex = keyIndex;
    info->i        = i;
    info->start    = n1;

    return keyIndex ^ xorKey;
}

static void crypt_encrypt(GCryptInfo *info, unsigned int *words, int len)
{
    int          part;
    unsigned int i;
    unsigned int w;
    int          keyIndex;
    int          xorKey;
    int          offset;

    offset   = info->offset;
    xorKey   = info->xorKey;
    keyIndex = info->keyIndex;
    i        = info->i;

    for (w = 0; w < len; w++) {
        while (i < 0x10000) {
            xorKey += i;
            keyIndex += xorKey;
            xorKey += keyIndex;

            info->msg[offset]      = xorKey;
            info->msg[offset + 16] = keyIndex;
            info->msg[offset + 32] = i;
            offset++;

            part     = rotl32(keyIndex, 24);
            keyIndex = rotl32(info->key[part & 0xff] ^ part, 24);
            part     = rotl32(info->key[xorKey & 0xff] ^ xorKey, 8);
            keyIndex ^= info->key[keyIndex & 0xff];
            xorKey = rotl32(info->key[part & 0xff] ^ part, 8);

            i *= 2;
        }

        words[w] = LittleLong(keyIndex ^ xorKey);
        offset--;
        if (offset < 0) {
            offset = 0;
        }

        part   = rotl32(~info->msg[offset], 24);
        xorKey = rotl32(info->key[part & 0xff] ^ part, 24);
        part   = rotl32(info->key[info->msg[offset + 16] & 0xff] ^ info->msg[offset + 16], 8);
        xorKey ^= info->key[xorKey & 0xff];
        keyIndex = rotl32(info->key[part & 0xff] ^ part, 8);

        i = info->msg[offset + 32] + 1 + info->msg[offset + 32];
    }

    info->offset   = offset;
    info->xorKey   = xorKey;
    info->keyIndex = keyIndex;
    info->i        = i;
}

void init_crypt_key(unsigned char *src, unsigned int len, GCryptInfo *info)
{
    int index;
    int i, j, k;
    int tmp;

    info->wordPtr = NULL;
    for (k = 0; k < 256; k++) {
        info->key[k] = 0;
    }

    for (i = 0; i < 4; i++) {
        for (k = 0; k < 256; k++) {
            info->key[k] = (info->key[k] << 8) + k;
        }

        index = i;

        for (j = 0; j < 2; j++) {
            for (k = 0; k < 256; k++) {
                index = ((info->key[k] & 0xff) + src[k % len] + index) & 0xff;

                tmp              = info->key[k];
                info->key[k]     = info->key[index];
                info->key[index] = tmp;
            }
        }
    }

    for (k = 0; k < 256; k++) {
        info->key[k] ^= k;
    }

    crypt_seek(info, 0, 0);
}

void crypt_docrypt(GCryptInfo *info, unsigned char *out, int len)
{
    int i;

    for (i = 0; i < len; i++) {
        if (!info->wordPtr || ((char *)info->wordPtr - (char *)info->words) >= (sizeof(info->words) - 1)) {
            info->wordPtr = (unsigned char *)info->words;
            crypt_encrypt(info, info->words, ARRAY_LEN(info->words));
        }

        out[i] ^= *info->wordPtr;
        info->wordPtr++;
    }
}
