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

/**
 * @file navigation_bsp_lump.cpp
 * @brief Provide functions for loading lumps from BSP.
 * 
 */

#include "g_local.h"
#include "navigation_bsp_lump.h"
#include "../script/scriptexception.h"

/*
============
gameLump_c::gameLump_c
============
*/
gameLump_c::gameLump_c(gameLump_c&& other) noexcept
{
    FreeLump();

    buffer       = other.buffer;
    length       = other.length;
    other.buffer = NULL;
}

/*
============
gameLump_c::operator=
============
*/
gameLump_c& gameLump_c::operator=(gameLump_c&& other) noexcept
{
    FreeLump();

    buffer       = other.buffer;
    length       = other.length;
    other.buffer = NULL;

    return *this;
}

/*
============
gameLump_c::~gameLump_c
============
*/
gameLump_c::~gameLump_c()
{
    FreeLump();
}

/*
============
gameLump_c::LoadLump
============
*/
gameLump_c gameLump_c::LoadLump(fileHandle_t handle, const lump_t& lump)
{
    if (lump.filelen) {
        void *buffer = gi.Malloc(lump.filelen);

        if (gi.FS_Seek(handle, lump.fileofs, FS_SEEK_SET) < 0) {
            throw ScriptException("CM_LoadLump: Error seeking to lump.");
        }

        gi.FS_Read(buffer, lump.filelen, handle);

        return gameLump_c(buffer, lump.filelen);
    }

    return gameLump_c();
}

/*
============
gameLump_c::FreeLump
============
*/
void gameLump_c::FreeLump()
{
    if (buffer) {
        gi.Free(buffer);
        buffer = NULL;
        length = 0;
    }
}

bspMap_c::bspMap_c()
    : mapname(NULL)
    , h(0)
    , length(0)
{}

bspMap_c::bspMap_c(const char *inMapName, fileHandle_t inHandle, int inLength)
    : mapname(inMapName)
    , h(inHandle)
    , length(inLength)
{}

bspMap_c::~bspMap_c()
{
    if (h) {
        gi.FS_FCloseFile(h);
    }
}

/*
============
bspMap_c::LoadLump
============
*/
gameLump_c bspMap_c::LoadLump(int index)
{
    const lump_t& lump = *Q_GetLumpByVersion(&header, index);

    if (lump.filelen) {
        void *buffer = gi.Malloc(lump.filelen);

        if (gi.FS_Seek(h, lump.fileofs, FS_SEEK_SET) < 0) {
            throw ScriptException("CM_LoadLump: Error seeking to lump.");
        }

        gi.FS_Read(buffer, lump.filelen, h);

        return gameLump_c(buffer, lump.filelen);
    }

    return gameLump_c();
}
