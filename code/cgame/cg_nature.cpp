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

cvar_t *cg_rain;
cvar_t *cg_rain_drawcoverage;

void RainTouch(ctempmodel_t *ct, trace_t *trace)
{
    Vector norm, neworg;

    ct->ent.hModel    = cgi.R_RegisterModel("splash_z.spr");
    ct->cgd.velocity  = vec_zero;
    ct->cgd.accel     = vec_zero;
    ct->killTime      = cg.time + 400;
    norm              = trace->plane.normal;
    norm.x            = -norm.x;
    norm.y            = -norm.y;
    ct->cgd.angles    = norm.toAngles();
    ct->ent.scale     = 0.3f;
    ct->cgd.scaleRate = 4.0;
    ct->cgd.flags |= T_FADE;

    neworg = trace->endpos + norm * 0.2f;
    VectorCopy(neworg, ct->ent.origin);
}

void ClientGameCommandManager::RainTouch(Event *ev)
{
    // FIXME: unimplemented
}

void ClientGameCommandManager::InitializeRainCvars()
{
    int i;

    cg_rain = cgi.Cvar_Get("cg_rain", "0", CVAR_ARCHIVE);
    cg_rain_drawcoverage =
        cgi.Cvar_Get("cg_rain_drawcoverage", "0", CVAR_SAVEGAME | CVAR_SERVER_CREATED | CVAR_SYSTEMINFO);

    cg.rain.density    = 0.0;
    cg.rain.speed      = 2048.0f;
    cg.rain.length     = 90.0f;
    cg.rain.min_dist   = 512.0f;
    cg.rain.width      = 1.0f;
    cg.rain.speed_vary = 512;
    cg.rain.slant      = 50;

    for (i = 0; i < MAX_RAIN_SHADERS; i++) {
        cg.rain.shader[i][0] = 0;
    }

    cg.rain.numshaders = 0;
}

void CG_Rain(centity_t *cent)
{
    int         iLife;
    vec3_t      mins, maxs;
    vec3_t      vOmins, vOmaxs, vOe;
    float       fcolor[4];
    vec3_t      vStart, vEnd;
    int         iNumSpawn;
    int         i;
    int         iRandom;
    float       fDensity;
    vec3_t      vLength;
    const char *shadername;

    fcolor[0] = 1.0;
    fcolor[1] = 1.0;
    fcolor[2] = 1.0;
    fcolor[3] = 1.0;

    if (!cg_rain->integer || paused->integer) {
        return;
    }

    cgi.R_ModelBounds(cgs.inlineDrawModel[cent->currentState.modelindex], mins, maxs);

    vOmins[0] = mins[0] + cent->lerpOrigin[0];
    if (vOmins[0] < cg.snap->ps.origin[0] - cg.rain.min_dist) {
        vOmins[0] = cg.snap->ps.origin[0] - cg.rain.min_dist;
    }

    vOmins[1] = mins[1] + cent->lerpOrigin[1];
    if (vOmins[1] < cg.snap->ps.origin[1] - cg.rain.min_dist) {
        vOmins[1] = cg.snap->ps.origin[1] - cg.rain.min_dist;
    }

    vOmins[2] = mins[2] + cent->lerpOrigin[2];

    vOmaxs[0] = maxs[0] + cent->lerpOrigin[0];
    if (vOmaxs[0] > cg.rain.min_dist + cg.snap->ps.origin[0]) {
        vOmaxs[0] = cg.rain.min_dist + cg.snap->ps.origin[0];
    }

    vOmaxs[1] = maxs[1] + cent->lerpOrigin[1];
    if (vOmaxs[1] > cg.rain.min_dist + cg.snap->ps.origin[1]) {
        vOmaxs[1] = cg.rain.min_dist + cg.snap->ps.origin[1];
    }

    vOmaxs[2] = maxs[2] + cent->lerpOrigin[2];

    if (vOmins[0] > vOmaxs[0]) {
        return;
    }

    if (vOmins[1] > vOmaxs[1]) {
        return;
    }

    if (cg_rain_drawcoverage->integer) {
        cgi.R_DebugLine(
            Vector(vOmins[0], vOmins[1], vOmins[2]), Vector(vOmaxs[0], vOmins[1], vOmins[2]), 1.0, 0.0, 0.0, 1.0
        );

        cgi.R_DebugLine(
            Vector(vOmaxs[0], vOmins[1], vOmins[2]), Vector(vOmaxs[0], vOmaxs[1], vOmins[2]), 1.0, 0.0, 0.0, 1.0
        );

        cgi.R_DebugLine(
            Vector(vOmaxs[0], vOmaxs[1], vOmins[2]), Vector(vOmins[0], vOmaxs[1], vOmins[2]), 1.0, 0.0, 0.0, 1.0
        );

        cgi.R_DebugLine(
            Vector(vOmins[0], vOmaxs[1], vOmins[2]), Vector(vOmins[0], vOmins[1], vOmins[2]), 1.0, 0.0, 0.0, 1.0
        );
    }

    VectorSubtract(vOmaxs, vOmins, vOe);
    fDensity  = cg.rain.density / 200.0;
    iNumSpawn = (int)(sqrt(vOe[0] * vOe[1]) * fDensity);
    if (iNumSpawn > MAX_BEAMS) {
        iNumSpawn = MAX_BEAMS;
    }

    iRandom = rand();

    if (cg.rain.numshaders) {
        shadername = cg.rain.shader[iRandom % cg.rain.numshaders];
    } else {
        shadername = cg.rain.shader[0];
    }

    for (i = 0; i < iNumSpawn; ++i) {
        iLife     = (int)(vOe[2] / ((float)(iRandom % cg.rain.speed_vary) + cg.rain.speed) * 1000.0);
        vStart[0] = (float)(iRandom % (int)(vOe[0] + 1.0)) + vOmins[0];
        iRandom   = ((214013 * iRandom + 2531011) >> 16) & 0x7FFF;
        vStart[1] = (float)(iRandom % (int)(vOe[1] + 1.0)) + vOmins[1];
        vStart[2] = vOmaxs[2];

        if (cg.snap) {
            vLength[0] = cg.snap->ps.origin[0] - vStart[0];
            vLength[1] = cg.snap->ps.origin[1] - vStart[1];
            vLength[2] = 0.0;
            if (cg.rain.min_dist * cg.rain.min_dist < VectorLengthSquared(vLength)) {
                continue;
            }
        }

        vEnd[0] = (float)(iRandom % cg.rain.slant) + vStart[0] + vss_wind_x->value;
        iRandom = ((214013 * iRandom + 2531011) >> 16) & 0x7FFF;
        vEnd[1] = (float)(iRandom % cg.rain.slant) + vStart[1] + vss_wind_y->value;
        vEnd[2] = vOmins[2];

        CG_CreateBeam(
            vStart,
            vec_zero,
            0,
            1,
            1.0,
            cg.rain.width,
            BEAM_INVERTED_FAST,
            1000.0,
            iLife,
            qtrue,
            vEnd,
            0,
            0,
            0,
            1,
            0,
            shadername,
            fcolor,
            0,
            0.0,
            cg.rain.length,
            1.0,
            0,
            "raineffect"
        );
    }
}
