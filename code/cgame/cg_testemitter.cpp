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
// testemitter functions. Used for designing emitters in game

#include "cg_commands.h"

cvar_t* cg_te_xangles;
cvar_t* cg_te_yangles;
cvar_t* cg_te_zangles;
cvar_t* cg_te_alpha;
cvar_t* cg_te_dietouch;
cvar_t* cg_te_bouncefactor;
cvar_t* cg_te_scale;
cvar_t* cg_te_scalemin;
cvar_t* cg_te_scalemax;
cvar_t* cg_te_model;
cvar_t* cg_te_life;
cvar_t* cg_te_color_r;
cvar_t* cg_te_color_g;
cvar_t* cg_te_color_b;
cvar_t* cg_te_startcolor_r;
cvar_t* cg_te_startcolor_g;
cvar_t* cg_te_startcolor_b;
cvar_t* cg_te_endcolor_r;
cvar_t* cg_te_endcolor_g;
cvar_t* cg_te_endcolor_b;
cvar_t* cg_te_accel_x;
cvar_t* cg_te_accel_y;
cvar_t* cg_te_accel_z;
cvar_t* cg_te_count;
cvar_t* cg_te_fade;
cvar_t* cg_te_fadein;
cvar_t* cg_te_spawnrate;
cvar_t* cg_te_orgoffset_x;
cvar_t* cg_te_orgoffset_y;
cvar_t* cg_te_orgoffset_z;
cvar_t* cg_te_rorgx;
cvar_t* cg_te_rorgy;
cvar_t* cg_te_rorgz;
cvar_t* cg_te_scalerate;
cvar_t* cg_te_circle;
cvar_t* cg_te_sphere;
cvar_t* cg_te_insphere;
cvar_t* cg_te_radius;
cvar_t* cg_te_swarm;
cvar_t* cg_te_wavy;
cvar_t* cg_te_align;
cvar_t* cg_te_flickeralpha;
cvar_t* cg_te_collision;
cvar_t* cg_te_globalfade;
cvar_t* cg_te_randomroll;
cvar_t* cg_te_angles_pitch;
cvar_t* cg_te_angles_yaw;
cvar_t* cg_te_angles_roll;
cvar_t* cg_te_randpitch;
cvar_t* cg_te_randyaw;
cvar_t* cg_te_randroll;
cvar_t* cg_te_forwardvelocity;
cvar_t* cg_te_randvelx;
cvar_t* cg_te_randvely;
cvar_t* cg_te_randvelz;
cvar_t* cg_te_rvxtype;
cvar_t* cg_te_rvytype;
cvar_t* cg_te_rvztype;

spawnthing_t* testspawnthing = NULL;

void CG_TestEmitter_f(void)
{
    // FIXME: TODO
}

void CG_DumpEmitter_f(void)
{
    // FIXME: TODO
}

void CG_UpdateTestEmitter(void)
{
    // FIXME: TODO
}

void CG_InitTestEmitter(void)
{
    cg_te_xangles = cgi.Cvar_Get("cg_te_xangles", "0", 0);
    cg_te_yangles = cgi.Cvar_Get("cg_te_yangles", "0", 0);
    cg_te_zangles = cgi.Cvar_Get("cg_te_zangles", "0", 0);
    cg_te_life = cgi.Cvar_Get("cg_te_life", "1", 0);
    cg_te_alpha = cgi.Cvar_Get("cg_te_alpha", "1", 0);
    cg_te_dietouch = cgi.Cvar_Get("cg_te_dietouch", "0", 0);
    cg_te_bouncefactor = cgi.Cvar_Get("cg_te_bouncefactor", "1", 0);
    cg_te_scale = cgi.Cvar_Get("cg_te_scale", "1", 0);
    cg_te_scalemin = cgi.Cvar_Get("cg_te_scalemin", "0", 0);
    cg_te_scalemax = cgi.Cvar_Get("cg_te_scalemax", "0", 0);
    cg_te_model = cgi.Cvar_Get("cg_te_model", "none", 0);
    cg_te_life = cgi.Cvar_Get("cg_te_life", "1", 0);
    cg_te_color_r = cgi.Cvar_Get("cg_te_color_r", "1", 0);
    cg_te_color_g = cgi.Cvar_Get("cg_te_color_g", "1", 0);
    cg_te_color_b = cgi.Cvar_Get("cg_te_color_b", "1", 0);
    cg_te_startcolor_r = cgi.Cvar_Get("cg_te_startcolor_r", "1", 0);
    cg_te_startcolor_g = cgi.Cvar_Get("cg_te_startcolor_g", "1", 0);
    cg_te_startcolor_b = cgi.Cvar_Get("cg_te_startcolor_b", "1", 0);
    cg_te_endcolor_r = cgi.Cvar_Get("cg_te_endcolor_r", "1", 0);
    cg_te_endcolor_g = cgi.Cvar_Get("cg_te_endcolor_g", "1", 0);
    cg_te_endcolor_b = cgi.Cvar_Get("cg_te_endcolor_b", "1", 0);
    cg_te_accel_x = cgi.Cvar_Get("cg_te_accel_x", "0", 0);
    cg_te_accel_y = cgi.Cvar_Get("cg_te_accel_y", "0", 0);
    cg_te_accel_z = cgi.Cvar_Get("cg_te_accel_z", "0", 0);
    cg_te_count = cgi.Cvar_Get("cg_te_count", "1", 0);
    cg_te_fade = cgi.Cvar_Get("cg_te_fade", "0", 0);
    cg_te_fadein = cgi.Cvar_Get("cg_te_fadein", "0", 0);
    cg_te_spawnrate = cgi.Cvar_Get("cg_te_spawnrate", "1", 0);
    cg_te_scalerate = cgi.Cvar_Get("cg_te_scalerate", "0", 0);
    cg_te_circle = cgi.Cvar_Get("cg_te_circle", "0", 0);
    cg_te_sphere = cgi.Cvar_Get("cg_te_sphere", "0", 0);
    cg_te_insphere = cgi.Cvar_Get("cg_te_insphere", "0", 0);
    cg_te_radius = cgi.Cvar_Get("cg_te_radius", "0", 0);
    cg_te_swarm = cgi.Cvar_Get("cg_te_swarm", "0", 0);
    cg_te_wavy = cgi.Cvar_Get("cg_te_wavy", "0", 0);
    cg_te_align = cgi.Cvar_Get("cg_te_align", "0", 0);
    cg_te_flickeralpha = cgi.Cvar_Get("cg_te_flickeralpha", "0", 0);
    cg_te_collision = cgi.Cvar_Get("cg_te_collision", "0", 0);
    cg_te_globalfade = cgi.Cvar_Get("cg_te_globalfade", "0", 0);
    cg_te_randomroll = cgi.Cvar_Get("cg_te_randomroll", "0", 0);

    cg_te_orgoffset_x = cgi.Cvar_Get("cg_te_orgoffset_x", "0", 0);
    cg_te_orgoffset_y = cgi.Cvar_Get("cg_te_orgoffset_y", "0", 0);
    cg_te_orgoffset_z = cgi.Cvar_Get("cg_te_orgoffset_z", "0", 0);
    cg_te_rorgx = cgi.Cvar_Get("cg_te_rorgx", "No", 0);
    cg_te_rorgy = cgi.Cvar_Get("cg_te_rorgy", "No", 0);
    cg_te_rorgz = cgi.Cvar_Get("cg_te_rorgz", "No", 0);

    cg_te_angles_pitch = cgi.Cvar_Get("cg_te_angles_pitch", "0", 0);
    cg_te_angles_yaw = cgi.Cvar_Get("cg_te_angles_yaw", "0", 0);
    cg_te_angles_roll = cgi.Cvar_Get("cg_te_angles_roll", "0", 0);
    cg_te_randpitch = cgi.Cvar_Get("cg_te_randpitch", "No", 0);
    cg_te_randyaw = cgi.Cvar_Get("cg_te_randyaw", "No", 0);
    cg_te_randroll = cgi.Cvar_Get("cg_te_randroll", "No", 0);

    cg_te_randvelx = cgi.Cvar_Get("cg_te_randvelx", "0", 0);
    cg_te_randvely = cgi.Cvar_Get("cg_te_randvely", "0", 0);
    cg_te_randvelz = cgi.Cvar_Get("cg_te_randvelz", "0", 0);
    cg_te_rvxtype = cgi.Cvar_Get("cg_te_rvxtype", "No", 0);
    cg_te_rvytype = cgi.Cvar_Get("cg_te_rvytype", "No", 0);
    cg_te_rvztype = cgi.Cvar_Get("cg_te_rvztype", "No", 0);

    cg_te_forwardvelocity = cgi.Cvar_Get("cg_te_forwardvel", "0", 0);
}
