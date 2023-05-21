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
// CGM buffer parser

#include "cg_local.h"
#include "cg_parsemsg.h"
#include "cg_specialfx.h"

#include "str.h"

extern int current_entity_number;

typedef struct {
    vec3_t i_vBarrel;
    vec3_t i_vStart;
    vec3_t* i_vEnd;
    int i_iNumBullets;
    qboolean iLarge;
    int iTracerVisible;
    qboolean bIgnoreEntities;
} bullet_tracer_t;

#define MAX_BULLET_TRACERS 32
#define MAX_BULLET_TRACE_BULLETS 1024
#define MAX_IMPACTS 64

static int bullet_tracers_count;
static int bullet_tracer_bullets_count;
static int wall_impact_count;
static int flesh_impact_count;
static bullet_tracer_t bullet_tracers[MAX_BULLET_TRACERS];
static vec3_t bullet_tracer_bullets[MAX_BULLET_TRACE_BULLETS];
static vec3_t wall_impact_pos[MAX_IMPACTS];
static vec3_t wall_impact_norm[MAX_IMPACTS];
static int wall_impact_large[MAX_IMPACTS];
static int wall_impact_type[MAX_IMPACTS];
static vec3_t flesh_impact_pos[MAX_IMPACTS];
static vec3_t flesh_impact_norm[MAX_IMPACTS];
static int flesh_impact_large[MAX_IMPACTS];

static void CG_MakeBulletHole(vec3_t i_vPos, vec3_t i_vNorm, int iLarge, trace_t* pPreTrace, qboolean bMakeSound)
{
    // FIXME: unimplemented
}

static void CG_MakeBubbleTrail(vec3_t i_vStart, vec3_t i_vEnd, int iLarge, float alpha = 1.0f)
{
    // FIXME: unimplemented
}

static void CG_BulletTracerEffect(vec3_t i_vStart, vec3_t i_vEnd)
{
    // FIXME: unimplemented
}

static void CG_MakeBulletTracerInternal(vec3_t i_vBarrel, vec3_t i_vStart, vec3_t *i_vEnd, int i_iNumBullets, qboolean iLarge, int iTracerVisible, qboolean bIgnoreEntities)
{
    // FIXME: unimplemented
}

static void CG_MakeBulletTracer(vec3_t i_vBarrel, vec3_t i_vStart, vec3_t* i_vEnd, int i_iNumBullets, qboolean iLarge, int iTracerVisible, qboolean bIgnoreEntities, float alpha = 1.0f) {
    bullet_tracer_t* bullet_tracer;
    int i;

    if (bullet_tracer_bullets_count >= MAX_BULLET_TRACERS) {
        Com_Printf("CG_MakeBulletTracer: MAX_BULLET_TRACERS exceeded\n");
        return;
    }

    if (i_iNumBullets + bullet_tracers_count >= MAX_BULLET_TRACE_BULLETS) {
        Com_Printf("CG_MakeBulletTracerInternal: MAX_BULLET_TRACE_BULLETS exceeded\n");
        return;
    }

    bullet_tracer = &bullet_tracers[bullet_tracer_bullets_count++];
    VectorCopy(i_vBarrel, bullet_tracer->i_vBarrel);
    VectorCopy(i_vStart, bullet_tracer->i_vStart);
    bullet_tracer->i_vEnd = &bullet_tracer_bullets[bullet_tracers_count];
    bullet_tracer->i_iNumBullets = i_iNumBullets;

    for (i = 0; i < i_iNumBullets; ++i)
    {
        bullet_tracer_bullets[bullet_tracers_count][0] = (*i_vEnd)[i];
        bullet_tracer_bullets[bullet_tracers_count][1] = (*i_vEnd)[i + 1];
        bullet_tracer_bullets[bullet_tracers_count++][2] = (*i_vEnd)[i + 2];
    }

    bullet_tracer->iLarge = iLarge;
    bullet_tracer->iTracerVisible = iTracerVisible;
    bullet_tracer->bIgnoreEntities = bIgnoreEntities;
}

void CG_AddBulletTracers()
{
    int i;

    for (i = 0; i < bullet_tracer_bullets_count; ++i) {
        CG_MakeBulletTracerInternal(
            bullet_tracers[i].i_vBarrel,
            bullet_tracers[i].i_vStart,
            bullet_tracers[i].i_vEnd,
            bullet_tracers[i].i_iNumBullets,
            bullet_tracers[i].iLarge,
            bullet_tracers[i].iTracerVisible,
            bullet_tracers[i].bIgnoreEntities
        );
    }

    bullet_tracer_bullets_count = 0;
    bullet_tracers_count = 0;
}

void CG_AddBulletImpacts()
{
    // FIXME: unimplemented
}

void CG_MakeExplosionEffect(vec3_t vPos, int iType) {
    int iSurfType;
    int iBaseEffect;
    int iSurfEffect;
    float fRadius;
    vec3_t vEnd;
    str sMark;
    trace_t trace;
    qhandle_t shader;

    vEnd[0] = vPos[0];
    vEnd[1] = vPos[1];
    vEnd[2] = vPos[2] - 64.0;
    fRadius = 64.0;

    if ((CG_PointContents(vPos, 0)) & MASK_WATER)
    {
        iBaseEffect = 73;
        sfxManager.MakeEffect_Normal(iBaseEffect, vPos, Vector(0, 0, 1));
        return;
    }

    switch (iType)
    {
    case 12:
        iBaseEffect = 63;
        break;
    case 13:
        iBaseEffect = 64;
        break;
    default:
        iBaseEffect = 63;
        break;
    }

    CG_Trace(
        &trace,
        vPos,
        vec_zero,
        vec_zero,
        vEnd,
        ENTITYNUM_NONE,
        MASK_EXPLOSION,
        qfalse,
        qtrue,
        "CG_MakeExplosionEffect"
    );

    if (trace.fraction == 1.0 || trace.startsolid)
    {
        // exploded in the air
        sfxManager.MakeEffect_Normal(iBaseEffect, vPos, Vector(0, 0, 1));
        return;
    }

    VectorMA(trace.endpos, 32.0, trace.plane.normal, vPos);

    iSurfType = trace.surfaceFlags & MASK_SURF_TYPE;
    switch (iSurfType)
    {
    case SURF_FOLIAGE:
        iSurfEffect = 77;
        fRadius = 0;
        break;
    case SURF_SNOW:
        iSurfEffect = 78;
        fRadius = 0;
        break;
    case SURF_CARPET:
        iSurfEffect = 79;
        break;
    case SURF_SAND:
        iSurfEffect = 76;
        fRadius = 0;
        break;
    case SURF_PUDDLE:
        iSurfEffect = -1;
        fRadius = 0;
        break;
    case SURF_GLASS:
        iSurfEffect = -1;
        fRadius = 0;
        break;
    case SURF_GRAVEL:
        iSurfEffect = 75;
        break;
    case SURF_MUD:
        iSurfEffect = 72;
        fRadius = 0;
        break;
    case SURF_DIRT:
        iSurfEffect = 69;
        fRadius = 0;
        break;
    case SURF_GRILL:
        iSurfEffect = 70;
        fRadius = 0;
        break;
    case SURF_GRASS:
        iSurfEffect = 71;
        break;
    case SURF_ROCK:
        iSurfEffect = 68;
        break;
    case SURF_PAPER:
        iSurfEffect = 65;
        break;
    case SURF_WOOD:
        iSurfEffect = 66;
        break;
    case SURF_METAL:
        iSurfEffect = 67;
        break;
    default:
        iSurfEffect = -1;
        break;
    }

    sMark = "blastmark";
    if (fRadius)
    {
        float value = (rand() & 0x7FFF) / 32767.0 - 0.5;
        fRadius *= (value + value) * 0.1 + 1.0;
        shader = cgi.R_RegisterShader(sMark.c_str());

        CG_ImpactMarkSimple(
            shader,
            trace.endpos,
            trace.plane.normal,
            random() * 360.0,
            fRadius,
            1, 1, 1, 1,
            qfalse,
            qfalse,
            qtrue,
            qfalse
        );
    }

    VectorMA(vPos, 1.0, trace.plane.normal, vPos);

    if (iSurfEffect != -1)
    {
        sfxManager.MakeEffect_Normal(
            iSurfEffect,
            Vector(trace.endpos) + Vector(trace.plane.normal),
            trace.plane.normal
        );
    }

    sfxManager.MakeEffect_Normal(
        iSurfEffect,
        vPos,
        trace.plane.normal
    );
}

void CG_MakeVehicleEffect(vec3_t i_vStart, vec3_t i_vEnd , vec3_t i_vDir) {
    vec3_t vDir, vFrom, vDest;
    trace_t trace;

    VectorSubtract(i_vEnd, i_vStart, vDir);
    VectorNormalizeFast(vDir);
    VectorMA(i_vEnd, -16.0, vDir, vFrom);
    VectorMA(i_vEnd, 16.0, vDir, vDest);

    CG_Trace(&trace, vFrom, vec_zero, vec_zero, vDest, ENTITYNUM_NONE, MASK_PLAYERSOLID, qfalse, qtrue, "CG_MakeBulletHole");
    cgi.R_DebugLine(vFrom, trace.endpos, 1.0, 1.0, 1.0, 1.0);
}

#if TARGET_GAME_PROTOCOL >= 15

void CG_ParseCGMessage()
{
	int i;
	int iType;
	int iLarge;
	int iInfo;
	int iCount;
	char* szTmp;
	vec3_t vStart, vEnd, vTmp;
	vec3_t vEndArray[MAX_IMPACTS];
    float alpha;
    int value;

	qboolean bMoreCGameMessages = qtrue;
	while (bMoreCGameMessages) {
		iType = cgi.MSG_ReadBits(6);

		switch (iType)
		{
		case 1:
		case 2:
		case 5:
			if (iType == 1)
			{
				vTmp[0] = cgi.MSG_ReadCoord();
				vTmp[1] = cgi.MSG_ReadCoord();
				vTmp[2] = cgi.MSG_ReadCoord();
			}
			vStart[0] = cgi.MSG_ReadCoord();
			vStart[1] = cgi.MSG_ReadCoord();
			vStart[2] = cgi.MSG_ReadCoord();

			if (iType != 1)
			{
				vTmp[0] = vStart[0];
				vTmp[1] = vStart[1];
				vTmp[2] = vStart[2];
			}

			vEndArray[0][0] = cgi.MSG_ReadCoord();
			vEndArray[0][1] = cgi.MSG_ReadCoord();
			vEndArray[0][2] = cgi.MSG_ReadCoord();
            iLarge = cgi.MSG_ReadBits(2);
            if (cgi.MSG_ReadBits(1)) {
                int iAlpha = cgi.MSG_ReadBits(10);
                alpha = (float)iAlpha / 512.0;
                if (alpha < 0.002) {
                    alpha = 0.002;
                }
			} else {
				alpha = 1.0f;
			}

			if (iType == 1) {
				CG_MakeBulletTracer(vTmp, vStart, vEndArray, 1, iLarge, qfalse, qtrue, alpha);
			}
			else if (iType == 2) {
				CG_MakeBulletTracer(vTmp, vStart, vEndArray, 1, iLarge, qfalse, qtrue, alpha);
			}
			else {
				CG_MakeBubbleTrail(vStart, vEndArray[0], iLarge, alpha);
			}

			break;
		case 3:
		case 4:
			if (iType == 3)
			{
				vTmp[0] = cgi.MSG_ReadCoord();
				vTmp[1] = cgi.MSG_ReadCoord();
				vTmp[2] = cgi.MSG_ReadCoord();
				iInfo = cgi.MSG_ReadBits(6);
			}
			else
			{
				iInfo = 0;
			}

			vStart[0] = cgi.MSG_ReadCoord();
			vStart[1] = cgi.MSG_ReadCoord();
			vStart[2] = cgi.MSG_ReadCoord();
			iLarge = cgi.MSG_ReadBits(2);
			if (cgi.MSG_ReadBits(1)) {
				int iAlpha = cgi.MSG_ReadBits(10);
				alpha = (float)iAlpha / 512.0;
				if (alpha < 0.002) {
					alpha = 0.002;
				}
            } else {
                alpha = 1.0f;
            }

			iCount = cgi.MSG_ReadBits(6);
			for (i = 0; i < iCount; ++i)
			{
				vEndArray[i][0] = cgi.MSG_ReadCoord();
				vEndArray[i][1] = cgi.MSG_ReadCoord();
				vEndArray[i][2] = cgi.MSG_ReadCoord();
			}

			if (iCount) {
				CG_MakeBulletTracer(vTmp, vStart, vEndArray, iCount, iLarge, iInfo, qtrue, alpha);
			}
			break;
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
        case 11:
			vStart[0] = cgi.MSG_ReadCoord();
			vStart[1] = cgi.MSG_ReadCoord();
			vStart[2] = cgi.MSG_ReadCoord();
			cgi.MSG_ReadDir(vEnd);
			iLarge = cgi.MSG_ReadBits(2);

			switch (iType)
			{
			case 6:
				if (wall_impact_count < MAX_IMPACTS)
				{
					VectorCopy(vStart, wall_impact_pos[wall_impact_count]);
					VectorCopy(vEnd, wall_impact_norm[wall_impact_count]);
					wall_impact_large[wall_impact_count] = iLarge;
					wall_impact_type[wall_impact_count] = -1;
					wall_impact_count++;
				}
				break;
			case 7:
				if (wall_impact_count < MAX_IMPACTS)
				{
					VectorCopy(vStart, wall_impact_pos[wall_impact_count]);
					VectorCopy(vEnd, wall_impact_norm[wall_impact_count]);
					wall_impact_large[wall_impact_count] = iLarge;
					wall_impact_type[wall_impact_count] = 6;
					wall_impact_count++;
				}
				break;
			case 8:
				if (flesh_impact_count < MAX_IMPACTS)
				{
					// negative
					VectorNegate(vEnd, vEnd);
					VectorCopy(vStart, flesh_impact_pos[flesh_impact_count]);
					VectorCopy(vEnd, flesh_impact_norm[flesh_impact_count]);
					flesh_impact_large[flesh_impact_count] = iLarge;
					flesh_impact_count++;
				}
				break;
			case 9:
				if (flesh_impact_count < MAX_IMPACTS)
				{
					// negative
					VectorNegate(vEnd, vEnd);
					VectorCopy(vStart, flesh_impact_pos[flesh_impact_count]);
					VectorCopy(vEnd, flesh_impact_norm[flesh_impact_count]);
					flesh_impact_large[flesh_impact_count] = iLarge;
					flesh_impact_count++;
				}
				break;
			case 10:
				if (wall_impact_count < MAX_IMPACTS)
				{
					VectorCopy(vStart, wall_impact_pos[wall_impact_count]);
					VectorCopy(vEnd, wall_impact_norm[wall_impact_count]);
					wall_impact_large[wall_impact_count] = iLarge;
					wall_impact_type[wall_impact_count] = 2;
					wall_impact_count++;
				}
				break;
			case 11:
				if (wall_impact_count < MAX_IMPACTS)
				{
					VectorCopy(vStart, wall_impact_pos[wall_impact_count]);
					VectorCopy(vEnd, wall_impact_norm[wall_impact_count]);
					wall_impact_large[wall_impact_count] = iLarge;
					wall_impact_type[wall_impact_count] = 4;
					wall_impact_count++;
				}
				break;
			default:
				continue;
			}
			break;

		case 12:
			vStart[0] = cgi.MSG_ReadCoord();
			vStart[1] = cgi.MSG_ReadCoord();
			vStart[2] = cgi.MSG_ReadCoord();
			vEnd[0] = cgi.MSG_ReadCoord();
			vEnd[1] = cgi.MSG_ReadCoord();
			vEnd[2] = cgi.MSG_ReadCoord();
			CG_MeleeImpact(vStart, vEnd);
			break;
		case 13:
		case 14:
		case 15:
		case 16:
			vStart[0] = cgi.MSG_ReadCoord();
			vStart[1] = cgi.MSG_ReadCoord();
			vStart[2] = cgi.MSG_ReadCoord();
			CG_MakeExplosionEffect(vStart, iType);
			break;
		case 18:
		case 19:
		case 20:
		case 21:
		case 22:
		case 23:
		case 24:
		case 25:
			vStart[0] = cgi.MSG_ReadCoord();
			vStart[1] = cgi.MSG_ReadCoord();
			vStart[2] = cgi.MSG_ReadCoord();
			cgi.MSG_ReadDir(vEnd);

			sfxManager.MakeEffect_Normal(iType + 67, vStart, vEnd);
			break;

		case 26:
		case 27:
		{
			str sEffect;
			char cTmp[8];
			vec3_t axis[3];

			vStart[0] = cgi.MSG_ReadCoord();
			vStart[1] = cgi.MSG_ReadCoord();
			vStart[2] = cgi.MSG_ReadCoord();
			iLarge = cgi.MSG_ReadByte();
			// get the integer as string
			snprintf(cTmp, sizeof(cTmp), "%d", iLarge);

			if (iType == 23) {
				sEffect = "models/fx/crates/debris_";
			}
			else {
				sEffect = "models/fx/windows/debris_";
			}

			sEffect += cTmp;
			sEffect += ".tik";

			VectorSet(axis[0], 0, 0, 1);
			VectorSet(axis[1], 0, 1, 0);
			VectorSet(axis[2], 1, 0, 0);

			cgi.R_SpawnEffectModel(sEffect.c_str(), vStart, axis);
		}
		break;

		case 28:
			vTmp[0] = cgi.MSG_ReadCoord();
			vTmp[1] = cgi.MSG_ReadCoord();
			vTmp[2] = cgi.MSG_ReadCoord();
			vStart[0] = cgi.MSG_ReadCoord();
			vStart[1] = cgi.MSG_ReadCoord();
			vStart[2] = cgi.MSG_ReadCoord();
			vEndArray[0][0] = cgi.MSG_ReadCoord();
			vEndArray[0][1] = cgi.MSG_ReadCoord();
			vEndArray[0][2] = cgi.MSG_ReadCoord();
			iLarge = cgi.MSG_ReadBits(2);
			if (cgi.MSG_ReadBits(1)) {
				int iAlpha = cgi.MSG_ReadBits(10);
				alpha = (float)iAlpha / 512.0;
				if (alpha < 0.002) {
					alpha = 0.002;
				}
            } else {
                alpha = 1.0f;
            }

			CG_MakeBulletTracer(vTmp, vStart, vEndArray, 1, iLarge, qtrue, qtrue, alpha);
			break;

		case 29:
			memset(vTmp, 0, sizeof(vTmp));
			vStart[0] = cgi.MSG_ReadCoord();
			vStart[1] = cgi.MSG_ReadCoord();
			vStart[2] = cgi.MSG_ReadCoord();
			vEndArray[0][0] = cgi.MSG_ReadCoord();
			vEndArray[0][1] = cgi.MSG_ReadCoord();
			vEndArray[0][2] = cgi.MSG_ReadCoord();
			iLarge = cgi.MSG_ReadBits(1);
			if (cgi.MSG_ReadBits(1)) {
				int iAlpha = cgi.MSG_ReadBits(10);
				alpha = (float)iAlpha / 512.0;
				if (alpha < 0.002) {
					alpha = 0.002;
				}
            } else {
                alpha = 1.0f;
            }

			CG_MakeBulletTracer(vTmp, vStart, vEndArray, 1, iLarge, qfalse, qtrue, alpha);
			break;

		case 30:
			iInfo = cgi.MSG_ReadByte();
			strcpy(cgi.HudDrawElements[iInfo].shaderName, cgi.MSG_ReadString());
			cgi.HudDrawElements[iInfo].string[0] = 0;
			cgi.HudDrawElements[iInfo].pFont = NULL;
			cgi.HudDrawElements[iInfo].fontName[0] = 0;
			// set the shader
			CG_HudDrawShader(iInfo);
			break;

		case 31:
			iInfo = cgi.MSG_ReadByte();
			cgi.HudDrawElements[iInfo].iHorizontalAlign = cgi.MSG_ReadBits(2);
			cgi.HudDrawElements[iInfo].iVerticalAlign = cgi.MSG_ReadBits(2);
			break;

		case 32:
			iInfo = cgi.MSG_ReadByte();
			cgi.HudDrawElements[iInfo].iX = cgi.MSG_ReadShort();
			cgi.HudDrawElements[iInfo].iY = cgi.MSG_ReadShort();
			cgi.HudDrawElements[iInfo].iWidth = cgi.MSG_ReadShort();
			cgi.HudDrawElements[iInfo].iHeight = cgi.MSG_ReadShort();
			break;

		case 33:
			iInfo = cgi.MSG_ReadByte();
			cgi.HudDrawElements[iInfo].bVirtualScreen = cgi.MSG_ReadBits(1);
			break;

		case 34:
			iInfo = cgi.MSG_ReadByte();
			cgi.HudDrawElements[iInfo].vColor[0] = cgi.MSG_ReadByte() / 255.0;
			cgi.HudDrawElements[iInfo].vColor[1] = cgi.MSG_ReadByte() / 255.0;
			cgi.HudDrawElements[iInfo].vColor[2] = cgi.MSG_ReadByte() / 255.0;
			break;

		case 35:
			iInfo = cgi.MSG_ReadByte();
			cgi.HudDrawElements[iInfo].vColor[3] = cgi.MSG_ReadByte() / 255.0;
			break;

		case 36:
			iInfo = cgi.MSG_ReadByte();
			cgi.HudDrawElements[iInfo].hShader = 0;
			strcpy(cgi.HudDrawElements[iInfo].string, cgi.MSG_ReadString());
			break;

		case 37:
			iInfo = cgi.MSG_ReadByte();
			strcpy(cgi.HudDrawElements[iInfo].fontName, cgi.MSG_ReadString());
			cgi.HudDrawElements[iInfo].hShader = 0;
			cgi.HudDrawElements[iInfo].shaderName[0] = 0;
			// load the font
			CG_HudDrawFont(iInfo);
			break;

		case 38:
		case 39:
		{
			int iOldEnt;

			iOldEnt = current_entity_number;
			current_entity_number = cg.snap->ps.clientNum;
			if (iType == 36) {
				commandManager.PlaySound(
					"dm_kill_notify",
					NULL,
					CHAN_LOCAL,
					2.0,
					-1,
					-1,
					1
				);
			}
			else {
				commandManager.PlaySound(
					"dm_hit_notify",
					NULL,
					CHAN_LOCAL,
					2.0,
					-1,
					-1,
					1
				);
			}

			current_entity_number = iOldEnt;
		}
		break;

		case 40:
		{
			int iOldEnt;

			vStart[0] = cgi.MSG_ReadCoord();
			vStart[1] = cgi.MSG_ReadCoord();
			vStart[2] = cgi.MSG_ReadCoord();
			iLarge = cgi.MSG_ReadBits(1);
			iInfo = cgi.MSG_ReadBits(6);
			szTmp = cgi.MSG_ReadString();

			iOldEnt = current_entity_number;

			if (iLarge) {
				current_entity_number = iInfo;

				commandManager.PlaySound(
					szTmp,
					vStart,
					CHAN_LOCAL,
					-1,
					-1,
					-1,
					0
				);
			}
			else {
				current_entity_number = cg.snap->ps.clientNum;

				commandManager.PlaySound(
					szTmp,
					vStart,
					CHAN_AUTO,
					-1,
					-1,
					-1,
					1
				);
			}

			current_entity_number = iOldEnt;
		}
		break;
        case 41:
			vStart[0] = cgi.MSG_ReadCoord();
			vStart[1] = cgi.MSG_ReadCoord();
			vStart[2] = cgi.MSG_ReadCoord();
			vEnd[0] = cgi.MSG_ReadCoord();
			vEnd[1] = cgi.MSG_ReadCoord();
			vEnd[2] = cgi.MSG_ReadCoord();
            cgi.MSG_ReadByte();
            cgi.MSG_ReadByte();
            VectorSubtract(vEnd, vStart, vTmp);

            // FIXME: unimplemented
            // ?? can't figure out what is this
			break;
		default:
			cgi.Error(ERR_DROP, "CG_ParseCGMessage: Unknown CGM message type");
			break;
		}

		bMoreCGameMessages = cgi.MSG_ReadBits(1);
	}
}

#else

void CG_ParseCGMessage()
{
    int i;
    int iType;
    int iLarge;
    int iInfo;
    int iCount;
    char* szTmp;
    vec3_t vStart, vEnd, vTmp;
    vec3_t vEndArray[MAX_IMPACTS];

    qboolean bMoreCGameMessages = qtrue;
    while (bMoreCGameMessages) {
        iType = cgi.MSG_ReadBits(6);

        switch (iType)
        {
        case 1:
        case 2:
        case 5:
            if (iType == 1)
            {
                vTmp[0] = cgi.MSG_ReadCoord();
                vTmp[1] = cgi.MSG_ReadCoord();
                vTmp[2] = cgi.MSG_ReadCoord();
            }
            vStart[0] = cgi.MSG_ReadCoord();
            vStart[1] = cgi.MSG_ReadCoord();
            vStart[2] = cgi.MSG_ReadCoord();

            if (iType != 1)
            {
                vTmp[0] = vStart[0];
                vTmp[1] = vStart[1];
                vTmp[2] = vStart[2];
            }

            vEndArray[0][0] = cgi.MSG_ReadCoord();
            vEndArray[0][1] = cgi.MSG_ReadCoord();
            vEndArray[0][2] = cgi.MSG_ReadCoord();
            iLarge = cgi.MSG_ReadBits(1);

            if (iType == 1) {
                CG_MakeBulletTracer(vTmp, vStart, vEndArray, 1, iLarge, qfalse, qtrue);
            }
            else if (iType == 2) {
                CG_MakeBulletTracer(vTmp, vStart, vEndArray, 1, iLarge, qfalse, qtrue);
            }
            else {
                CG_MakeBubbleTrail(vStart, vEndArray[0], iLarge);
            }

            break;
        case 3:
        case 4:
            if (iType == 3)
            {
                vTmp[0] = cgi.MSG_ReadCoord();
                vTmp[1] = cgi.MSG_ReadCoord();
                vTmp[2] = cgi.MSG_ReadCoord();
                iInfo = cgi.MSG_ReadBits(6);
            }
            else
            {
                iInfo = 0;
            }

            vStart[0] = cgi.MSG_ReadCoord();
            vStart[1] = cgi.MSG_ReadCoord();
            vStart[2] = cgi.MSG_ReadCoord();
            iLarge = cgi.MSG_ReadBits(1);
            iCount = cgi.MSG_ReadBits(6);
            for (i = 0; i < iCount; ++i)
            {
                vEndArray[i][0] = cgi.MSG_ReadCoord();
                vEndArray[i][1] = cgi.MSG_ReadCoord();
                vEndArray[i][2] = cgi.MSG_ReadCoord();
            }

            if (iCount) {
                CG_MakeBulletTracer(vTmp, vStart, vEndArray, iCount, iLarge, iInfo, qtrue);
            }
            break;
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
            vStart[0] = cgi.MSG_ReadCoord();
            vStart[1] = cgi.MSG_ReadCoord();
            vStart[2] = cgi.MSG_ReadCoord();
            cgi.MSG_ReadDir(vEnd);
            iLarge = cgi.MSG_ReadBits(1);

            switch (iType)
            {
            case 6:
                if (wall_impact_count < MAX_IMPACTS)
                {
                    VectorCopy(vStart, wall_impact_pos[wall_impact_count]);
                    VectorCopy(vEnd, wall_impact_norm[wall_impact_count]);
                    wall_impact_large[wall_impact_count] = iLarge;
                    wall_impact_type[wall_impact_count] = 0;
                    wall_impact_count++;
                }
                break;
            case 7:
                if (flesh_impact_count < MAX_IMPACTS)
                {
                    // negative
                    VectorNegate(vEnd, vEnd);
                    VectorCopy(vStart, flesh_impact_pos[flesh_impact_count]);
                    VectorCopy(vEnd, flesh_impact_norm[flesh_impact_count]);
                    flesh_impact_large[flesh_impact_count] = iLarge;
                    flesh_impact_count++;
                }
                break;
            case 8:
                if (flesh_impact_count < MAX_IMPACTS)
                {
                    // negative
                    VectorNegate(vEnd, vEnd);
                    VectorCopy(vStart, flesh_impact_pos[flesh_impact_count]);
                    VectorCopy(vEnd, flesh_impact_norm[flesh_impact_count]);
                    flesh_impact_large[flesh_impact_count] = iLarge;
                    flesh_impact_count++;
                }
                break;
            case 9:
                if (wall_impact_count < MAX_IMPACTS)
                {
                    VectorCopy(vStart, wall_impact_pos[wall_impact_count]);
                    VectorCopy(vEnd, wall_impact_norm[wall_impact_count]);
                    wall_impact_large[wall_impact_count] = iLarge;
                    wall_impact_type[wall_impact_count] = (iLarge != 0) + 2;
                    wall_impact_count++;
                }
                break;
            case 10:
                if (wall_impact_count < MAX_IMPACTS)
                {
                    VectorCopy(vStart, wall_impact_pos[wall_impact_count]);
                    VectorCopy(vEnd, wall_impact_norm[wall_impact_count]);
                    wall_impact_large[wall_impact_count] = iLarge;
                    wall_impact_type[wall_impact_count] = (iLarge != 0) + 4;
                    wall_impact_count++;
                }
                break;
            default:
                continue;
            }
            break;

        case 11:
            vStart[0] = cgi.MSG_ReadCoord();
            vStart[1] = cgi.MSG_ReadCoord();
            vStart[2] = cgi.MSG_ReadCoord();
            vEnd[0] = cgi.MSG_ReadCoord();
            vEnd[1] = cgi.MSG_ReadCoord();
            vEnd[2] = cgi.MSG_ReadCoord();
            CG_MeleeImpact(vStart, vEnd);
            break;
        case 12:
        case 13:
            vStart[0] = cgi.MSG_ReadCoord();
            vStart[1] = cgi.MSG_ReadCoord();
            vStart[2] = cgi.MSG_ReadCoord();
            CG_MakeExplosionEffect(vStart, iType);
            break;
        case 15:
        case 16:
        case 17:
        case 18:
        case 19:
        case 20:
        case 21:
        case 22:
            vStart[0] = cgi.MSG_ReadCoord();
            vStart[1] = cgi.MSG_ReadCoord();
            vStart[2] = cgi.MSG_ReadCoord();
            cgi.MSG_ReadDir(vEnd);

            sfxManager.MakeEffect_Normal(iType + 67, vStart, vEnd);
            break;

        case 23:
        case 24:
        {
            str sEffect;
            char cTmp[8];
            vec3_t axis[3];

            vStart[0] = cgi.MSG_ReadCoord();
            vStart[1] = cgi.MSG_ReadCoord();
            vStart[2] = cgi.MSG_ReadCoord();
            iLarge = cgi.MSG_ReadByte();
            // get the integer as string
            snprintf(cTmp, sizeof(cTmp), "%d", iLarge);

            if (iType == 23) {
                sEffect = "models/fx/crates/debris_";
            }
            else {
                sEffect = "models/fx/windows/debris_";
            }

            sEffect += cTmp;
            sEffect += ".tik";

            VectorSet(axis[0], 0, 0, 1);
            VectorSet(axis[1], 0, 1, 0);
            VectorSet(axis[2], 1, 0, 0);

            cgi.R_SpawnEffectModel(sEffect.c_str(), vStart, axis);
        }
        break;

        case 25:
            vTmp[0] = cgi.MSG_ReadCoord();
            vTmp[1] = cgi.MSG_ReadCoord();
            vTmp[2] = cgi.MSG_ReadCoord();
            vStart[0] = cgi.MSG_ReadCoord();
            vStart[1] = cgi.MSG_ReadCoord();
            vStart[2] = cgi.MSG_ReadCoord();
            vEndArray[0][0] = cgi.MSG_ReadCoord();
            vEndArray[0][1] = cgi.MSG_ReadCoord();
            vEndArray[0][2] = cgi.MSG_ReadCoord();
            iLarge = cgi.MSG_ReadBits(1);

            CG_MakeBulletTracer(vTmp, vStart, vEndArray, 1, iLarge, qtrue, qtrue);
            break;

        case 26:
            memset(vTmp, 0, sizeof(vTmp));
            vStart[0] = cgi.MSG_ReadCoord();
            vStart[1] = cgi.MSG_ReadCoord();
            vStart[2] = cgi.MSG_ReadCoord();
            vEndArray[0][0] = cgi.MSG_ReadCoord();
            vEndArray[0][1] = cgi.MSG_ReadCoord();
            vEndArray[0][2] = cgi.MSG_ReadCoord();
            iLarge = cgi.MSG_ReadBits(1);

            CG_MakeBulletTracer(vTmp, vStart, vEndArray, 1, iLarge, qfalse, qtrue);
            break;

        case 27:
            iInfo = cgi.MSG_ReadByte();
            strcpy(cgi.HudDrawElements[iInfo].shaderName, cgi.MSG_ReadString());
            cgi.HudDrawElements[iInfo].string[0] = 0;
            cgi.HudDrawElements[iInfo].pFont = NULL;
            cgi.HudDrawElements[iInfo].fontName[0] = 0;
            // set the shader
            CG_HudDrawShader(iInfo);
            break;

        case 28:
            iInfo = cgi.MSG_ReadByte();
            cgi.HudDrawElements[iInfo].iHorizontalAlign = cgi.MSG_ReadBits(2);
            cgi.HudDrawElements[iInfo].iVerticalAlign = cgi.MSG_ReadBits(2);
            break;

        case 29:
            iInfo = cgi.MSG_ReadByte();
            cgi.HudDrawElements[iInfo].iX = cgi.MSG_ReadShort();
            cgi.HudDrawElements[iInfo].iY = cgi.MSG_ReadShort();
            cgi.HudDrawElements[iInfo].iWidth = cgi.MSG_ReadShort();
            cgi.HudDrawElements[iInfo].iHeight = cgi.MSG_ReadShort();
            break;

        case 30:
            iInfo = cgi.MSG_ReadByte();
            cgi.HudDrawElements[iInfo].bVirtualScreen = cgi.MSG_ReadBits(1);
            break;

		case 31:
			iInfo = cgi.MSG_ReadByte();
            cgi.HudDrawElements[iInfo].vColor[0] = cgi.MSG_ReadByte() / 255.0;
            cgi.HudDrawElements[iInfo].vColor[1] = cgi.MSG_ReadByte() / 255.0;
            cgi.HudDrawElements[iInfo].vColor[2] = cgi.MSG_ReadByte() / 255.0;
            break;

		case 32:
			iInfo = cgi.MSG_ReadByte();
            cgi.HudDrawElements[iInfo].vColor[3] = cgi.MSG_ReadByte() / 255.0;
            break;

        case 33:
            iInfo = cgi.MSG_ReadByte();
            cgi.HudDrawElements[iInfo].hShader = 0;
            strcpy(cgi.HudDrawElements[iInfo].string, cgi.MSG_ReadString());
            break;

        case 34:
            iInfo = cgi.MSG_ReadByte();
            strcpy(cgi.HudDrawElements[iInfo].fontName, cgi.MSG_ReadString());
            cgi.HudDrawElements[iInfo].hShader = 0;
            cgi.HudDrawElements[iInfo].shaderName[0] = 0;
            // load the font
            CG_HudDrawFont(iInfo);
            break;

        case 35:
        case 36:
        {
            int iOldEnt;

            iOldEnt = current_entity_number;
            current_entity_number = cg.snap->ps.clientNum;
            if (iType == 36) {
                commandManager.PlaySound(
                    "dm_kill_notify",
                    NULL,
                    CHAN_LOCAL,
                    2.0,
                    -1,
                    -1,
                    1
                );
            }
            else {
                commandManager.PlaySound(
                    "dm_hit_notify",
                    NULL,
                    CHAN_LOCAL,
                    2.0,
                    -1,
                    -1,
                    1
                );
            }

            current_entity_number = iOldEnt;
        }
        break;

        case 37:
        {
            int iOldEnt;

            vStart[0] = cgi.MSG_ReadCoord();
            vStart[1] = cgi.MSG_ReadCoord();
            vStart[2] = cgi.MSG_ReadCoord();
            iLarge = cgi.MSG_ReadBits(1);
            iInfo = cgi.MSG_ReadBits(6);
            szTmp = cgi.MSG_ReadString();

            iOldEnt = current_entity_number;

            if (iLarge) {
                current_entity_number = iInfo;

                commandManager.PlaySound(
                    szTmp,
                    vStart,
                    CHAN_LOCAL,
                    -1,
                    -1,
                    -1,
                    0
                );
            }
            else {
                current_entity_number = cg.snap->ps.clientNum;

                commandManager.PlaySound(
                    szTmp,
                    vStart,
                    CHAN_AUTO,
                    -1,
                    -1,
                    -1,
                    1
                );
            }

            current_entity_number = iOldEnt;
        }
            break;
        default:
            cgi.Error(ERR_DROP, "CG_ParseCGMessage: Unknown CGM message type");
            break;
        }

        bMoreCGameMessages = cgi.MSG_ReadBits(1);
    }
}
#endif
