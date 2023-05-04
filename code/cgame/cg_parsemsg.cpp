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

#define MAX_TRACERS 32
#define MAX_BULLETS 1024
#define MAX_IMPACTS 64

static int bullet_tracers_count;
static int bullet_tracer_bullets_count;
static int wall_impact_count;
static int flesh_impact_count;
static bullet_tracer_t bullet_tracers[MAX_TRACERS];
static vec3_t bullet_tracer_bullets[MAX_BULLETS];
static vec3_t wall_impact_pos[MAX_IMPACTS];
static vec3_t wall_impact_norm[MAX_IMPACTS];
static int wall_impact_large[MAX_IMPACTS];
static int wall_impact_type[MAX_IMPACTS];
static vec3_t flesh_impact_pos[MAX_IMPACTS];
static vec3_t flesh_impact_norm[MAX_IMPACTS];
static int flesh_impact_large[MAX_IMPACTS];

static void CG_MakeBulletTracer(vec3_t i_vBarrel, vec3_t i_vStart, vec3_t* i_vEnd, int i_iNumBullets, qboolean iLarge, int iTracerVisible, qboolean bIgnoreEntities) {
    // FIXME: unimplemented
}

static void CG_MakeBubbleTrail(vec3_t i_vStart, vec3_t i_vEnd, int iLarge) {
    // FIXME: unimplemented
}

void CG_AddBulletTracers()
{
    // FIXME: unimplemented
}

void CG_AddBulletImpacts()
{
    // FIXME: unimplemented
}

void CG_MakeExplosionEffect(float* vPos, int iType) {
    // FIXME: unimplemented
}

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
            // get the integer as string
            itoa(iLarge, cTmp, 10);

            iLarge = cgi.MSG_ReadByte();

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
            cgi.HudDrawElements[iInfo].vColor[0] = cgi.MSG_ReadByte() / 255.0;
            cgi.HudDrawElements[iInfo].vColor[1] = cgi.MSG_ReadByte() / 255.0;
            cgi.HudDrawElements[iInfo].vColor[2] = cgi.MSG_ReadByte() / 255.0;
            break;

        case 32:
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
        }
    }

    // FIXME: unimplemented
}
