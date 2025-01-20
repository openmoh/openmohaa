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

// navigation -- Modern navigation system using Recast and Detour

#include "g_local.h"
#include "navigation_recast.h"
#include "../script/scriptexception.h"

/*
============
G_Navigation_BuildRecastMesh
============
*/
void G_Navigation_BuildRecastMesh(navMap_t& navigationMap)
{

}

/*
============
G_Navigation_LoadWorldMap
============
*/
void G_Navigation_LoadWorldMap(const char *mapname)
{
    navMap_t navigationMap;
    int start, end;

    gi.Printf("---- Recast Navigation ----\n");

    if (!sv_maxbots->integer) {
        gi.Printf("No bots, skipping navigation\n");
        return;
    }

    try {
        start = gi.Milliseconds();

        G_Navigation_ProcessBSPForNavigation(mapname, navigationMap);
    } catch (const ScriptException& e) {
        gi.Printf("Failed to load BSP for navigation: %s\n", e.string.c_str());
        return;
    }

    end = gi.Milliseconds();

    gi.Printf("BSP file loaded and parsed in %.03f seconds\n", (float)((end - start) / 1000.0));

    try {
        start = gi.Milliseconds();

        G_Navigation_BuildRecastMesh(navigationMap);
    } catch (const ScriptException& e) {
        gi.Printf("Couldn't build recast navigation mesh: %s\n", e.string.c_str());
        return;
    }

    end = gi.Milliseconds();

    gi.Printf("Recast navigation mesh generated in %.03f seconds\n", (float)((end - start) / 1000.0));
}
