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
// Volumetric smoke A.K.A VSS sources

#include "cg_local.h"
#include "cg_commands.h"

const char* cg_vsstypes[] =
{
    "default",
    "gun",
    "bulletimpact",
    "bulletdirtimpact",
    "heavy",
    "steam",
    "mist",
    "smokegrenade",
    "grenade",
    "fire",
    "greasefire",
    "debris"
};

static int lastVSSFrameTime;

cvar_t *vss_draw;
cvar_t *vss_physics_fps;
cvar_t *vss_repulsion_fps;
cvar_t *vss_maxcount;
cvar_t *vss_color;
cvar_t *vss_showsources;
cvar_t *vss_wind_x;
cvar_t *vss_wind_y;
cvar_t *vss_wind_z;
cvar_t *vss_wind_strength;
cvar_t *vss_movement_dampen;
cvar_t *vss_maxvisible;
cvar_t *vss_gridsize;
cvar_t *vss_default_r;
cvar_t *vss_default_g;
cvar_t *vss_default_b;
cvar_t *vss_lighting_fps;

void ClientGameCommandManager::ResetVSSSources()
{
    // FIXME: unimplemented
}

void CG_ResetVSSSources()
{
    commandManager.ResetVSSSources();
    lastVSSFrameTime = cg.time;
}

void ClientGameCommandManager::InitializeVSSSources()
{
    // FIXME: unimplemented
}

void ClientGameCommandManager::InitializeVSSCvars()
{
    vss_draw = cgi.Cvar_Get("vss_draw", "0", CVAR_ARCHIVE);
    vss_physics_fps = cgi.Cvar_Get("vss_physics_fps", "8", 0);
    vss_repulsion_fps = cgi.Cvar_Get("vss_repulsion_fps", "4", 0);
    vss_maxcount = cgi.Cvar_Get("vss_maxcount", "22", CVAR_ARCHIVE);
    vss_color = cgi.Cvar_Get("vss_color", "1", 0);
    vss_showsources = cgi.Cvar_Get("vss_showsources", "1", 0);
    vss_wind_x = cgi.Cvar_Get("vss_wind_x", "8", 0);
    vss_wind_y = cgi.Cvar_Get("vss_wind_y", "4", 0);
    vss_wind_z = cgi.Cvar_Get("vss_wind_z", "2", 0);
    vss_wind_strength = cgi.Cvar_Get("vss_wind_strength", "8", 0);
    vss_movement_dampen = cgi.Cvar_Get("vss_movement_dampen", "4", 0);
    vss_maxvisible = cgi.Cvar_Get("vss_maxvisible", "1024", 33);
    vss_gridsize = cgi.Cvar_Get("vss_gridsize", "12", 0);
    vss_default_r = cgi.Cvar_Get("vss_default_r", "0.5", 0);
    vss_default_g = cgi.Cvar_Get("vss_default_g", "0.45", 0);
    vss_default_b = cgi.Cvar_Get("vss_default_b", "0.4", 0);
    vss_lighting_fps = cgi.Cvar_Get("vss_lighting_fps", "15", 0);
}

void CG_AddVSSSources()
{
    commandManager.AddVSSSources();
}

void ClientGameCommandManager::AddVSSSources()
{
    // FIXME: unimplemented
}
