/*
===========================================================================
Copyright (C) 2024 the OpenMoHAA team

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

#if USE_SOUND_NEW

#    include "snd_local.h"

byte *data_p;
byte *iff_end;
byte *last_chunk;
byte *iff_data;
int   iff_chunk_len;

qboolean S_LoadMP3(const char *fileName, sfx_t *sfx);

short int GetLittleShort()
{
    union {
        short value;
        byte  bytes[2];
    } val;

#    ifdef Q3_LITTLE_ENDIAN
    val.bytes[1] = data_p[0];
    val.bytes[0] = data_p[1];
#    else
    val.bytes[0] = data_p[0];
    val.bytes[1] = data_p[1];
#    endif

    data_p += sizeof(short);
    return val.value;
}

int GetLittleLong()
{
    union {
        int  value;
        byte bytes[4];
    } val;

#    ifdef Q3_LITTLE_ENDIAN
    val.bytes[3] = data_p[0];
    val.bytes[2] = data_p[1];
    val.bytes[1] = data_p[2];
    val.bytes[0] = data_p[3];
#    else
    val.bytes[0] = data_p[0];
    val.bytes[1] = data_p[1];
    val.bytes[2] = data_p[2];
    val.bytes[3] = data_p[3];
#    endif

    data_p += sizeof(int);
    return val.value;
}

void SetLittleShort(int i)
{
    union {
        short value;
        byte  bytes[2];
    } val;

    val.value = i;

#    ifdef Q3_LITTLE_ENDIAN
    data_p[0] = val.bytes[1];
    data_p[1] = val.bytes[0];
#    else
    data_p[0] = val.bytes[0];
    data_p[1] = val.bytes[1];
#    endif

    data_p += sizeof(short);
}

void SetLittleLong(int i)
{
    union {
        int  value;
        byte bytes[2];
    } val;

    val.value = i;

#    ifdef Q3_LITTLE_ENDIAN
    data_p[0] = val.bytes[3];
    data_p[1] = val.bytes[2];
    data_p[2] = val.bytes[1];
    data_p[3] = val.bytes[0];
#    else
    data_p[0] = val.bytes[0];
    data_p[1] = val.bytes[1];
    data_p[2] = val.bytes[2];
    data_p[3] = val.bytes[3];
#    endif

    data_p += sizeof(int);
}

void FindNextChunk(const char *name)
{
    // FIXME: unimplemented
}

void FindChunk(const char *name)
{
    // FIXME: unimplemented
}

void DumpChunks()
{
    // FIXME: unimplemented
}

wavinfo_t GetWavinfo(const char *name, byte *wav, int wavlength)
{
    wavinfo_t info;
    int       samples;

    memset(&info, 0, sizeof(wavinfo_t));

    // FIXME: unimplemented
    return info;
}

int DownSampleWav(wavinfo_t *info, byte *wav, int wavlength, int newkhz, byte **newdata)
{
    // FIXME: unimplemented
    return 0;
}

int DownSampleWav_MILES(wavinfo_t *info, byte *wav, int wavlength, int newkhz, byte **newdata)
{
    // FIXME: unimplemented
    return 0;
}

qboolean S_LoadSound(const char *fileName, sfx_t *sfx, int streamed, qboolean force_load)
{
    // FIXME: unimplemented
    return qfalse;
}

qboolean S_LoadMP3(const char *fileName, sfx_t *sfx)
{
    // FIXME: unimplemented
    return qfalse;
}

#endif
