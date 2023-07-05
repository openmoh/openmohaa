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
// Special Effects code

#include "cg_local.h"
#include "cg_commands.h"
#include "surfaceflags.h"
#include "cg_specialfx.h"

extern refEntity_t *current_entity;
extern dtiki_t     *current_tiki;

static vec3_t g_vFootstepMins = {-4, -4, 0};
static vec3_t g_vFootstepMaxs = {4, 4, 2};
static vec3_t g_vLadderstepMins;
static vec3_t g_vLadderstepMaxs;

ClientSpecialEffectsManager sfxManager;

Event EV_SFX_EffectDelay(
    "effectdelay",
    EV_DEFAULT,
    "iivvvvv",
    "iEffect iCurrEmitter vPos vAngles vAxisA vAxisB vAxisC",
    "Resumes the execution of iEffect effect from its iCurrEmitter emitter."
);

CLASS_DECLARATION(Listener, ClientSpecialEffectsManager, NULL) {
    {&EV_SFX_EffectDelay, &ClientSpecialEffectsManager::ContinueEffectExecution},
    {NULL,                NULL                                                 }
};

specialeffectcommand_t::specialeffectcommand_t()
{
    emitter      = NULL;
    fCommandTime = 0.0f;
    endfcn       = NULL;
}

specialeffect_t::specialeffect_t()
{
    m_iCommandCount = 0;
}

specialeffectcommand_t *specialeffect_t::AddNewCommand()
{
    if (m_iCommandCount == MAX_SPECIAL_EFFECT_COMMANDS - 1) {
        return NULL;
    }

    m_commands[m_iCommandCount] = new specialeffectcommand_t;
    return m_commands[m_iCommandCount++];
}

ClientSpecialEffectsManager::ClientSpecialEffectsManager()
{
    m_bEffectsLoaded    = 0;
    m_iNumPendingEvents = 0;
}

void ClientSpecialEffectsManager::LoadEffects()
{
    int              i, j, k;
    const char      *szEffectModel;
    float            axis[3][3];
    specialeffect_t *pEffect;

    if (m_bEffectsLoaded) {
        return;
    }

    cgi.DPrintf("Loading Special Effects...\n");
    AxisClear(axis);

    for (i = 0; i < MAX_SPECIAL_EFFECTS; i++) {
        switch (i) {
        case 0:
            szEffectModel = "models/fx/bh_paper_lite.tik";
            break;
        case 1:
            szEffectModel = "models/fx/bh_paper_hard.tik";
            break;
        case 2:
            szEffectModel = "models/fx/bh_wood_lite.tik";
            break;
        case 3:
            szEffectModel = "models/fx/bh_wood_hard.tik";
            break;
        case 4:
            szEffectModel = "models/fx/bh_metal_lite.tik";
            break;
        case 5:
            szEffectModel = "models/fx/bh_metal_hard.tik";
            break;
        case 6:
            szEffectModel = "models/fx/bh_stone_lite.tik";
            break;
        case 7:
            szEffectModel = "models/fx/bh_stone_hard.tik";
            break;
        case 8:
            szEffectModel = "models/fx/bh_dirt_lite.tik";
            break;
        case 9:
            szEffectModel = "models/fx/bh_dirt_hard.tik";
            break;
        case 10:
            szEffectModel = "models/fx/bh_metal_lite.tik";
            break;
        case 11:
            szEffectModel = "models/fx/bh_metal_hard.tik";
            break;
        case 12:
            szEffectModel = "models/fx/bh_grass_lite.tik";
            break;
        case 13:
            szEffectModel = "models/fx/bh_grass_hard.tik";
            break;
        case 14:
            szEffectModel = "models/fx/bh_mud_lite.tik";
            break;
        case 15:
            szEffectModel = "models/fx/bh_mud_hard.tik";
            break;
        case 16:
            szEffectModel = "models/fx/bh_water_lite.tik";
            break;
        case 17:
            szEffectModel = "models/fx/bh_water_hard.tik";
            break;
        case 18:
            szEffectModel = "models/fx/bh_glass_lite.tik";
            break;
        case 19:
            szEffectModel = "models/fx/bh_glass_hard.tik";
            break;
        case 20:
            szEffectModel = "models/fx/bh_stone_lite.tik";
            break;
        case 21:
            szEffectModel = "models/fx/bh_stone_hard.tik";
            break;
        case 22:
            szEffectModel = "models/fx/bh_sand_lite.tik";
            break;
        case 23:
            szEffectModel = "models/fx/bh_sand_hard.tik";
            break;
        case 24:
            szEffectModel = "models/fx/bh_foliage_lite.tik";
            break;
        case 25:
            szEffectModel = "models/fx/bh_foliage_hard.tik";
            break;
        case 26:
            szEffectModel = "models/fx/bh_snow_lite.tik";
            break;
        case 27:
            szEffectModel = "models/fx/bh_snow_hard.tik";
            break;
        case 28:
            szEffectModel = "models/fx/bh_carpet_lite.tik";
            break;
        case 29:
            szEffectModel = "models/fx/bh_carpet_hard.tik";
            break;
        case 30:
            szEffectModel = "models/fx/bh_human_uniform_lite.tik";
            break;
        case 31:
            szEffectModel = "models/fx/bh_human_uniform_hard.tik";
            break;
        case 32:
            szEffectModel = "models/fx/water_trail_bubble.tik";
            break;
        case 33:
        case 34:
        case 35:
        case 36:
        case 37:
        case 38:
        case 39:
        case 40:
        case 41:
        case 42:
        case 43:
        case 44:
        case 45:
        case 46:
        case 47:
        case 48:
        case 49:
        case 50:
        case 51:
        case 52:
        case 53:
        case 54:
        case 55:
        case 56:
        case 57:
        case 58:
        case 59:
        case 60:
        case 61:
        case 62:
            szEffectModel = "models/fx/bh_stone_hard.tik";
            break;
        case 63:
            szEffectModel = "models/fx/grenexp_base.tik";
            break;
        case 64:
            szEffectModel = "models/fx/bazookaexp_base.tik";
            break;
        case 65:
            szEffectModel = "models/fx/grenexp_paper.tik";
            break;
        case 66:
            szEffectModel = "models/fx/grenexp_wood.tik";
            break;
        case 67:
            szEffectModel = "models/fx/grenexp_metal.tik";
            break;
        case 68:
            szEffectModel = "models/fx/grenexp_stone.tik";
            break;
        case 69:
            szEffectModel = "models/fx/grenexp_dirt.tik";
            break;
        case 70:
            szEffectModel = "models/fx/grenexp_metal.tik";
            break;
        case 71:
            szEffectModel = "models/fx/grenexp_grass.tik";
            break;
        case 72:
            szEffectModel = "models/fx/grenexp_mud.tik";
            break;
        case 73:
            szEffectModel = "models/fx/grenexp_water.tik";
            break;
        case 74:
        case 98:
            continue;
        case 75:
            szEffectModel = "models/fx/grenexp_gravel.tik";
            break;
        case 76:
            szEffectModel = "models/fx/grenexp_sand.tik";
            break;
        case 77:
            szEffectModel = "models/fx/grenexp_foliage.tik";
            break;
        case 78:
            szEffectModel = "models/fx/grenexp_snow.tik";
            break;
        case 79:
            szEffectModel = "models/fx/grenexp_carpet.tik";
            break;
        case 80:
            szEffectModel = "models/fx/water_ripple_still.tik";
            break;
        case 81:
            szEffectModel = "models/fx/water_ripple_moving.tik";
            break;
        case 82:
            szEffectModel = "models/fx/barrel_oil_leak_big.tik";
            break;
        case 83:
            szEffectModel = "models/fx/barrel_oil_leak_medium.tik";
            break;
        case 84:
            szEffectModel = "models/fx/barrel_oil_leak_small.tik";
            break;
        case 85:
            szEffectModel = "models/fx/barrel_oil_leak_splat.tik";
            break;
        case 86:
            szEffectModel = "models/fx/barrel_water_leak_big.tik";
            break;
        case 87:
            szEffectModel = "models/fx/barrel_water_leak_medium.tik";
            break;
        case 88:
            szEffectModel = "models/fx/barrel_water_leak_small.tik";
            break;
        case 89:
            szEffectModel = "models/fx/barrel_water_leak_splat.tik";
            break;
        case 90:
            szEffectModel = "models/fx/fs_light_dust.tik";
            break;
        case 91:
            szEffectModel = "models/fx/fs_heavy_dust.tik";
            break;
        case 92:
            szEffectModel = "models/fx/fs_dirt.tik";
            break;
        case 93:
            szEffectModel = "models/fx/fs_grass.tik";
            break;
        case 94:
            szEffectModel = "models/fx/fs_mud.tik";
            break;
        case 95:
            szEffectModel = "models/fx/fs_puddle.tik";
            break;
        case 96:
            szEffectModel = "models/fx/fs_sand.tik";
            break;
        case 97:
            szEffectModel = "models/fx/fs_snow.tik";
            break;
        default:
            szEffectModel = "models/fx/bh_stone_hard.tik";
        }

        pEffect = &m_effects[i];
        commandManager.SetCurrentSFX(pEffect);
        cgi.R_SpawnEffectModel(szEffectModel, vec_zero, axis);
        commandManager.ClearCurrentSFX();

        for (j = 0; j < pEffect->m_iCommandCount - 1; j++) {
            for (k = 0; k < j; k++) {
                if (pEffect->m_commands[k]->fCommandTime > pEffect->m_commands[k + 1]->fCommandTime) {
                    specialeffectcommand_t *pCur = pEffect->m_commands[k];
                    pEffect->m_commands[k]       = pEffect->m_commands[k + 1];
                    pEffect->m_commands[k + 1]   = pCur;
                }
            }
        }
    }

    m_bEffectsLoaded = qtrue;
}

void CG_InitializeSpecialEffectsManager()
{
    sfxManager.LoadEffects();
}

void CG_AddPendingEffects()
{
    if (sfxManager.EffectsPending()) {
        sfxManager.ProcessPendingEvents();
    }
}

void ClientSpecialEffectsManager::ContinueEffectExecution(Event *ev)
{
    Vector norm;
    float  axis[3][3];

    norm = ev->GetVector(5);
    VectorCopy(norm, axis[0]);

    norm = ev->GetVector(6);
    VectorCopy(norm, axis[1]);

    norm = ev->GetVector(7);
    VectorCopy(norm, axis[2]);

    ExecuteEffect(ev->GetInteger(1), ev->GetInteger(2), ev->GetVector(3), ev->GetVector(4), axis);
}

void ClientSpecialEffectsManager::ExecuteEffect(
    int iEffect, int iStartCommand, Vector vPos, Vector vAngles, float axis[3][3]
)
{
    int                     i;
    int                     iCommandCount;
    float                   fStartCommandTime;
    specialeffect_t        *pEffect;
    specialeffectcommand_t *pCommand;
    refEntity_t            *old_entity;
    dtiki_t                *old_tiki;
    refEntity_t             tmpEntity;

    memset(&tmpEntity, 0, sizeof(tmpEntity));
    VectorCopy(((const float *)vPos), tmpEntity.origin);
    tmpEntity.scale         = 1.0;
    tmpEntity.renderfx      = 0;
    tmpEntity.shaderRGBA[3] = -1;

    pEffect       = &this->m_effects[iEffect];
    iCommandCount = pEffect->m_iCommandCount;
    if (pEffect->m_iCommandCount) {
        old_entity     = current_entity;
        old_tiki       = current_tiki;
        current_entity = NULL;
        current_tiki   = NULL;
        pCommand       = pEffect->m_commands[iStartCommand];

        fStartCommandTime = pCommand->fCommandTime;
        for (i = iStartCommand; i < iCommandCount; i++) {
            pCommand = pEffect->m_commands[i];
            if (pCommand->fCommandTime > fStartCommandTime) {
                Event ev1(EV_SFX_EffectDelay);
                ev1.AddInteger(iEffect);
                ev1.AddInteger(i);
                ev1.AddVector(vPos);
                ev1.AddVector(vAngles);
                ev1.AddVector(axis[0]);
                ev1.AddVector(axis[1]);
                ev1.AddVector(axis[2]);

                PostEvent(ev1, pCommand->fCommandTime - fStartCommandTime);
                ++m_iNumPendingEvents;

                break;
            }

            if (pCommand->pEvent) {
                current_entity = &tmpEntity;
                current_tiki   = tmpEntity.tiki;
                // give a reference to it so the event doesn't get deleted
                commandManager.ProcessEvent(*pCommand->pEvent);
            } else if (pCommand->emitter && pCommand->endfcn) {
                current_entity = NULL;
                current_tiki   = NULL;

                pCommand->emitter->cgd.origin = vPos;
                if (pCommand->emitter->cgd.flags & T_ANGLES) {
                    pCommand->emitter->cgd.angles = vAngles;
                }

                AxisCopy(axis, pCommand->emitter->axis);
                AxisCopy(axis, pCommand->emitter->tag_axis);
                pCommand->emitter->cgd.createTime = cg.time;
                commandManager.SetSpawnthing(pCommand->emitter);
            }
        }

        current_entity = old_entity;
        current_tiki   = old_tiki;
    }
}

void ClientSpecialEffectsManager::MakeEffect_Normal(int iEffect, Vector vPos, Vector vNormal)
{
    Vector vAngles;
    float  axis[3][3];

    vAngles = vNormal.toAngles();
    AnglesToAxis(vAngles, axis);
    ClientSpecialEffectsManager::ExecuteEffect(iEffect, 0, vPos, vAngles, axis);
}

void ClientSpecialEffectsManager::MakeEffect_Angles(int iEffect, Vector vPos, Vector vAngles)
{
    float axis[3][3];

    AnglesToAxis((const float *)vAngles, axis);
    ClientSpecialEffectsManager::ExecuteEffect(iEffect, 0, vPos, vAngles, axis);
}

void ClientSpecialEffectsManager::MakeEffect_Axis(int iEffect, Vector vPos, float axis[3][3])
{
    Vector vAngles;

    MatrixToEulerAngles(axis, (float *)vAngles);
    ClientSpecialEffectsManager::ExecuteEffect(iEffect, 0, vPos, vAngles, axis);
}

/*
==============================================================

FOOTSTEP CODE

==============================================================
*/

#define GROUND_DISTANCE        8
#define WATER_NO_SPLASH_HEIGHT 16

static void CG_FootstepMain(trace_t *trace, int iRunning, int iEquipment)
{
    int    contents;
    int    surftype;
    int    iEffectNum;
    float  fVolume;
    vec3_t vPos;
    vec3_t midlegs;
    str    sSoundName;

    iEffectNum = -1;

    VectorCopy(trace->endpos, vPos);
    sSoundName = "snd_step_";

    contents = CG_PointContents(trace->endpos, -1);
    if (contents & MASK_WATER) {
        // take our ground position and trace upwards
        VectorCopy(trace->endpos, midlegs);
        midlegs[2] += WATER_NO_SPLASH_HEIGHT;
        contents = CG_PointContents(midlegs, -1);
        if (contents & MASK_WATER) {
            sSoundName += "wade";
        } else {
            sSoundName += "puddle";
            iEffectNum = 95;
        }
    } else {
        surftype = trace->surfaceFlags & MASK_SURF_TYPE;
        switch (surftype) {
        case SURF_FOLIAGE:
            sSoundName += "foliage";
            iEffectNum = 93;
            break;
        case SURF_SNOW:
            sSoundName += "snow";
            iEffectNum = 97;
            break;
        case SURF_CARPET:
            sSoundName += "carpet";
            iEffectNum = 90;
            break;
        case SURF_SAND:
            sSoundName += "sand";
            iEffectNum = 96;
            break;
        case SURF_PUDDLE:
            sSoundName += "puddle";
            iEffectNum = 95;
            break;
        case SURF_GLASS:
            sSoundName += "glass";
            iEffectNum = 90;
            break;
        case SURF_GRAVEL:
            sSoundName += "gravel";
            iEffectNum = 91;
            break;
        case SURF_MUD:
            sSoundName += "mud";
            iEffectNum = 94;
            break;
        case SURF_DIRT:
            sSoundName += "dirt";
            iEffectNum = 92;
            break;
        case SURF_GRILL:
            sSoundName += "grill";
            iEffectNum = 90;
            break;
        case SURF_GRASS:
            sSoundName += "grass";
            iEffectNum = 93;
            break;
        case SURF_ROCK:
            sSoundName += "stone";
            iEffectNum = 91;
            break;
        case SURF_PAPER:
            sSoundName += "paper";
            iEffectNum = 90;
            break;
        case SURF_WOOD:
            sSoundName += "wood";
            iEffectNum = 90;
            break;
        case SURF_METAL:
            sSoundName += "metal";
            iEffectNum = 90;
            break;
        default:
            sSoundName += "stone";
            iEffectNum = 91;
            break;
        }
    }

    if (cg_debugFootsteps->integer) {
        cgi.DPrintf("Footstep: %s  running = %i  effect = %i\n", sSoundName.c_str(), iRunning, surftype);
    }

    if (iRunning) {
        if (iRunning == -1) {
            fVolume = 0.5;
        } else {
            fVolume = 1.0;
        }
    } else {
        fVolume = 0.25;
    }

    if (!iRunning && cgs.gametype == GT_SINGLE_PLAYER) {
        return;
    }

    commandManager.PlaySound(sSoundName, vPos, -1, fVolume, -1, -1, 1);

    if (iEquipment && random() < 0.3) {
        // also play equipment sound

        commandManager.PlaySound("snd_step_equipment", vPos, -1, fVolume, -1, -1, 1);
    }
}

void CG_Footstep(const char *szTagName, centity_t *ent, refEntity_t *pREnt, int iRunning, int iEquipment)
{
    int           i;
    int           iTagNum;
    vec3_t        vStart, vEnd;
    vec3_t        midlegs;
    vec3_t        vMins, vMaxs;
    str           sSoundName;
    trace_t       trace;
    orientation_t oTag;

    // send a trace down from the player to the ground
    VectorCopy(ent->lerpOrigin, vStart);
    vStart[2] += GROUND_DISTANCE;

    if (szTagName) {
        iTagNum = cgi.Tag_NumForName(pREnt->tiki, szTagName);
        if (iTagNum != -1) {
            oTag = cgi.TIKI_Orientation(pREnt, iTagNum);

            for (i = 0; i < 2; i++) {
                VectorMA(vStart, oTag.origin[i], pREnt->axis[i], vStart);
            }
        }
    }

    if (iRunning == -1) {
        AngleVectors(ent->lerpAngles, midlegs, NULL, NULL);
        VectorMA(vStart, -16, midlegs, vStart);
        VectorMA(vStart, 64, midlegs, vEnd);

        VectorSet(vMins, -2, -2, -8);
        VectorSet(vMaxs, 2, 2, 8);
    } else {
        VectorSet(vMins, -4, -4, 0);
        VectorSet(vMaxs, 4, 4, 2);

        // add 16 units above feets
        vStart[2] += 16.0;
        VectorCopy(vStart, vEnd);
        vEnd[2] -= 64.0;
    }

    if (ent->currentState.eType == ET_PLAYER) {
        CG_Trace(
            &trace,
            vStart,
            vMins,
            vMaxs,
            vEnd,
            ent->currentState.number,
            MASK_PLAYERSOLID,
            qtrue,
            qtrue,
            "Player Footsteps"
        );
    } else {
        CG_Trace(
            &trace,
            vStart,
            vMins,
            vMaxs,
            vEnd,
            ent->currentState.number,
            MASK_MONSTERSOLID,
            qfalse,
            qfalse,
            "Monster Footsteps"
        );
    }

    if (trace.fraction == 1.0f) {
        if (cg_debugFootsteps->integer) {
            cgi.DPrintf("Footstep: missed floor\n");
        }

        return;
    }

    CG_FootstepMain(&trace, iRunning, iEquipment);
}

void CG_MeleeImpact(vec3_t vStart, vec3_t vEnd)
{
    vec3_t  vMins, vMaxs;
    trace_t trace;

    VectorSet(vMins, -4, -4, 0);
    VectorSet(vMaxs, 4, 4, 2);
    CG_Trace(&trace, vStart, vMins, vMaxs, vEnd, ENTITYNUM_NONE, MASK_PLAYERSOLID, qtrue, qtrue, "CG_MeleeImpact");

    if (trace.fraction != 1.0) {
        CG_FootstepMain(&trace, qtrue, qfalse);
    }
}

void CG_LandingSound(centity_t *ent, refEntity_t *pREnt, float volume, int iEquipment)
{
    int     contents;
    int     surftype;
    int     iEffectNum;
    vec3_t  vStart, vEnd;
    vec3_t  midlegs;
    str     sSoundName;
    trace_t trace;

    iEffectNum = -1;

    if (ent->iNextLandTime > cg.time) {
        ent->iNextLandTime = cg.time + 200;
        return;
    }

    ent->iNextLandTime = cg.time + 200;
    VectorCopy(ent->lerpOrigin, vStart);
    vStart[2] += GROUND_DISTANCE;

    VectorCopy(vStart, vEnd);
    vEnd[2] -= 64.0;

    if (ent->currentState.eType == ET_PLAYER) {
        CG_Trace(
            &trace,
            vStart,
            g_vFootstepMins,
            g_vFootstepMaxs,
            vEnd,
            ent->currentState.number,
            MASK_PLAYERSOLID,
            qtrue,
            qtrue,
            "Player Footsteps"
        );
    } else {
        CG_Trace(
            &trace,
            vStart,
            g_vFootstepMins,
            g_vFootstepMaxs,
            vEnd,
            ent->currentState.number,
            MASK_MONSTERSOLID,
            qfalse,
            qfalse,
            "Monster Footsteps"
        );
    }

    if (trace.fraction == 1.0) {
        return;
    }

    sSoundName += "snd_landing_";

    contents = CG_PointContents(trace.endpos, -1);
    if (contents & MASK_WATER) {
        // take our ground position and trace upwards
        VectorCopy(trace.endpos, midlegs);
        midlegs[2] += WATER_NO_SPLASH_HEIGHT;
        contents = CG_PointContents(midlegs, -1);
        if (contents & MASK_WATER) {
            sSoundName += "wade";
        } else {
            sSoundName += "puddle";
            iEffectNum = 95;
        }
    } else {
        surftype = trace.surfaceFlags & MASK_SURF_TYPE;
        switch (surftype) {
        case SURF_FOLIAGE:
            sSoundName += "foliage";
            iEffectNum = 93;
            break;
        case SURF_SNOW:
            sSoundName += "snow";
            iEffectNum = 97;
            break;
        case SURF_CARPET:
            sSoundName += "carpet";
            iEffectNum = 90;
            break;
        case SURF_SAND:
            sSoundName += "sand";
            iEffectNum = 96;
            break;
        case SURF_PUDDLE:
            sSoundName += "puddle";
            iEffectNum = 95;
            break;
        case SURF_GLASS:
            sSoundName += "glass";
            iEffectNum = 90;
            break;
        case SURF_GRAVEL:
            sSoundName += "gravel";
            iEffectNum = 91;
            break;
        case SURF_MUD:
            sSoundName += "mud";
            iEffectNum = 94;
            break;
        case SURF_DIRT:
            sSoundName += "dirt";
            iEffectNum = 92;
            break;
        case SURF_GRILL:
            sSoundName += "grill";
            iEffectNum = 90;
            break;
        case SURF_GRASS:
            sSoundName += "grass";
            iEffectNum = 93;
            break;
        case SURF_ROCK:
            sSoundName += "stone";
            iEffectNum = 91;
            break;
        case SURF_PAPER:
            sSoundName += "paper";
            iEffectNum = 90;
            break;
        case SURF_WOOD:
            sSoundName += "wood";
            iEffectNum = 90;
            break;
        case SURF_METAL:
            sSoundName += "metal";
            iEffectNum = 90;
            break;
        default:
            sSoundName += "stone";
            iEffectNum = 91;
            break;
        }
    }

    if (cg_debugFootsteps->integer) {
        cgi.DPrintf("Landing: %s    volume: %.2f   effect = %i\n", sSoundName.c_str(), volume, contents);
    }

    commandManager.PlaySound(sSoundName, trace.endpos, -1, volume, -1, -1, 1);

    if (iEquipment && random() < 0.5) {
        commandManager.PlaySound("snd_step_equipment", ent->lerpOrigin, -1, volume, -1, -1, 1);
    }

    if (iEffectNum != -1) {
        sfxManager.MakeEffect_Angles(iEffectNum, trace.endpos, Vector(270, 0, 0));
    }
}

void CG_BodyFallSound(centity_t *ent, refEntity_t *pREnt, float volume)
{
    // FIXME: unimplemented
}

/*
===============
CG_Splash

Draw a mark at the water surface
===============
*/
void CG_Splash(centity_t *cent)
{
    vec3_t  start, end, diff;
    trace_t trace;
    int     contents;
    float   dist;
    float   time_required;

    spawnthing_t m_ripple;

    if (!cg_shadows->integer) {
        return;
    }

    VectorSubtract(cent->currentState.origin, cent->nextState.origin, diff);
    diff[2] = 0;
    dist    = VectorLength(diff);

    // See if enough time has passed to add another ripple

    if (dist >= 1) {
        time_required = 100 - dist;
    } else {
        time_required = 200;
    }

    if (time_required < 10) {
        time_required = 10;
    }

    if (cent->splash_last_spawn_time + time_required > cg.time) {
        return;
    }

    // Save the current time

    cent->splash_last_spawn_time = cg.time;

    // Make sure the entity is moving
    if (dist < 1) {
        if (cent->splash_still_count >= 0) {
            cent->splash_still_count++;

            if (cent->splash_still_count > 2) {
                cent->splash_still_count = 0;
            } else {
                return;
            }
        } else {
            return;
        }
    }

    VectorCopy(cent->lerpOrigin, end);

    // if the feet aren't in liquid, don't make a mark
    // this won't handle moving water brushes, but they wouldn't draw right anyway...
    contents = cgi.CM_PointContents(end, 0);
    if (!(contents & (CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA))) {
        return;
    }

    VectorCopy(cent->lerpOrigin, start);
    start[2] += 88;

    // if the head isn't out of liquid, don't make a mark
    contents = cgi.CM_PointContents(start, 0);
    if (contents & (CONTENTS_SOLID | CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA)) {
        return;
    }

    // trace down to find the surface
    cgi.CM_BoxTrace(
        &trace, start, end, vec3_origin, vec3_origin, 0, (CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA), qfalse
    );

    if (trace.fraction == 1.0) {
        return;
    }

    // FIXME
    // TODO: Make effect
}

qboolean ClientSpecialEffectsManager::EffectsPending()
{
    return sfxManager.m_iNumPendingEvents > 0;
}

specialeffect_t *ClientSpecialEffectsManager::GetTestEffectPointer()
{
    return &m_effects[SPECIAL_EFFECT_TEST];
}
