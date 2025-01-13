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

#include "g_local.h"
#include "../qcommon/qfiles.h"

void G_Navigation_LoadWorldMap(const char* mapname)
{
    dheader_t	    header;
    fileHandle_t	h;
    int				length;
    int             i;

    if (!sv_maxbots->integer) {
        return;
    }

    // load it
    length = gi.FS_FOpenFile(mapname, &h, qtrue, qtrue);
    if (length <= 0) {
        return;
    }

    gi.FS_Read(&header, sizeof(dheader_t), h);

    for (i=0 ; i<sizeof(dheader_t)/4 ; i++) {
        ((int *)&header)[i] = LittleLong ( ((int *)&header)[i]);
    }

    // FIXME: unimplemented
    //  1. Load surfaces and parse them
    //  2. Load terrains and render them into vertices

    gi.FS_FCloseFile(h);
}
