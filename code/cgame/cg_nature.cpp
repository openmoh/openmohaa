/*
===========================================================================
Copyright (C) 2023 the OpenMoHAA team

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

// DESCRIPTION:
// Nature effects

#include "cg_local.h"
#include "cg_commands.h"

cvar_t* cg_rain;
cvar_t* cg_rain_drawcoverage;

void ClientGameCommandManager::InitializeRainCvars()
{
    int i;

    cg_rain = cgi.Cvar_Get("cg_rain", "0", CVAR_ARCHIVE);
    cg_rain_drawcoverage = cgi.Cvar_Get("cg_rain_drawcoverage", "0", CVAR_SAVEGAME | CVAR_SERVER_CREATED | CVAR_SYSTEMINFO);

    cg.rain.density = 0.0;
    cg.rain.speed = 2048.0f;
    cg.rain.length = 90.0f;
    cg.rain.min_dist = 512.0f;
    cg.rain.width = 1.0f;
    cg.rain.speed_vary = 512;
    cg.rain.slant = 50;

    for (i = 0; i < MAX_RAIN_SHADERS; i++) {
        cg.rain.shader[i][0] = 0;
    }

    cg.rain.numshaders = 0;
}
