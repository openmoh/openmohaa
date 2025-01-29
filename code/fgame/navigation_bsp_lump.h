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

#pragma once

#include "g_local.h"
#include "../qcommon/qfiles.h"

class gameLump_c
{
public:
    gameLump_c()
        : buffer(NULL)
        , length(0)
    {}

    gameLump_c(void *inBuffer, int inLength)
        : buffer(inBuffer)
        , length(inLength)
    {}

    gameLump_c(const gameLump_c&)            = delete;
    gameLump_c& operator=(const gameLump_c&) = delete;
    gameLump_c(gameLump_c&& other) noexcept;
    gameLump_c& operator=(gameLump_c&& other) noexcept;
    ~gameLump_c();

    static gameLump_c LoadLump(fileHandle_t handle, const lump_t& lump);
    void              FreeLump();

public:
    void *buffer;
    int   length;
};

class bspMap_c {
public:
    bspMap_c();
    ~bspMap_c();

    bspMap_c(const char* inMapName, fileHandle_t inHandle, int inLength);
    gameLump_c LoadLump(int index);

public:
    const char* mapname;
    fileHandle_t h;
    dheader_t header;
    int length;
};
