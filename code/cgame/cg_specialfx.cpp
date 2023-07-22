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
        case SFX_BHIT_PAPER_LITE:
            szEffectModel = "models/fx/bh_paper_lite.tik";
            break;
        case SFX_BHIT_PAPER_HARD:
            szEffectModel = "models/fx/bh_paper_hard.tik";
            break;
        case SFX_BHIT_WOOD_LITE:
            szEffectModel = "models/fx/bh_wood_lite.tik";
            break;
        case SFX_BHIT_WOOD_HARD:
            szEffectModel = "models/fx/bh_wood_hard.tik";
            break;
        case SFX_BHIT_METAL_LITE:
            szEffectModel = "models/fx/bh_metal_lite.tik";
            break;
        case SFX_BHIT_METAL_HARD:
            szEffectModel = "models/fx/bh_metal_hard.tik";
            break;
        case SFX_BHIT_STONE_LITE:
            szEffectModel = "models/fx/bh_stone_lite.tik";
            break;
        case SFX_BHIT_STONE_HARD:
            szEffectModel = "models/fx/bh_stone_hard.tik";
            break;
        case SFX_BHIT_DIRT_LITE:
            szEffectModel = "models/fx/bh_dirt_lite.tik";
            break;
        case SFX_BHIT_DIRT_HARD:
            szEffectModel = "models/fx/bh_dirt_hard.tik";
            break;
        case SFX_BHIT_GRILL_LITE:
            szEffectModel = "models/fx/bh_metal_lite.tik";
            break;
        case SFX_BHIT_GRILL_HARD:
            szEffectModel = "models/fx/bh_metal_hard.tik";
            break;
        case SFX_BHIT_GRASS_LITE:
            szEffectModel = "models/fx/bh_grass_lite.tik";
            break;
        case SFX_BHIT_GRASS_HARD:
            szEffectModel = "models/fx/bh_grass_hard.tik";
            break;
        case SFX_BHIT_MUD_LITE:
            szEffectModel = "models/fx/bh_mud_lite.tik";
            break;
        case SFX_BHIT_MUD_HARD:
            szEffectModel = "models/fx/bh_mud_hard.tik";
            break;
        case SFX_BHIT_PUDDLE_LITE:
            szEffectModel = "models/fx/bh_water_lite.tik";
            break;
        case SFX_BHIT_PUDDLE_HARD:
            szEffectModel = "models/fx/bh_water_hard.tik";
            break;
        case SFX_BHIT_GLASS_LITE:
            szEffectModel = "models/fx/bh_glass_lite.tik";
            break;
        case SFX_BHIT_GLASS_HARD:
            szEffectModel = "models/fx/bh_glass_hard.tik";
            break;
        case SFX_BHIT_GRAVEL_LITE:
            szEffectModel = "models/fx/bh_stone_lite.tik";
            break;
        case SFX_BHIT_GRAVEL_HARD:
            szEffectModel = "models/fx/bh_stone_hard.tik";
            break;
        case SFX_BHIT_SAND_LITE:
            szEffectModel = "models/fx/bh_sand_lite.tik";
            break;
        case SFX_BHIT_SAND_HARD:
            szEffectModel = "models/fx/bh_sand_hard.tik";
            break;
        case SFX_BHIT_FOLIAGE_LITE:
            szEffectModel = "models/fx/bh_foliage_lite.tik";
            break;
        case SFX_BHIT_FOLIAGE_HARD:
            szEffectModel = "models/fx/bh_foliage_hard.tik";
            break;
        case SFX_BHIT_SNOW_LITE:
            szEffectModel = "models/fx/bh_snow_lite.tik";
            break;
        case SFX_BHIT_SNOW_HARD:
            szEffectModel = "models/fx/bh_snow_hard.tik";
            break;
        case SFX_BHIT_CARPET_LITE:
            szEffectModel = "models/fx/bh_carpet_lite.tik";
            break;
        case SFX_BHIT_CARPET_HARD:
            szEffectModel = "models/fx/bh_carpet_hard.tik";
            break;
        case SFX_BHIT_HUMAN_UNIFORM_LITE:
            szEffectModel = "models/fx/bh_human_uniform_lite.tik";
            break;
        case SFX_BHIT_HUMAN_UNIFORM_HARD:
            szEffectModel = "models/fx/bh_human_uniform_hard.tik";
            break;
        case SFX_WATER_TRAIL_BUBBLE:
            szEffectModel = "models/fx/water_trail_bubble.tik";
            break;
        case SFX_VFX_PAPER_LITE:
        case SFX_VFX_PAPER_HARD:
        case SFX_VFX_WOOD_LITE:
        case SFX_VFX_WOOD_HARD:
        case SFX_VFX_METAL_LITE:
        case SFX_VFX_METAL_HARD:
        case SFX_VFX_STONE_LITE:
        case SFX_VFX_STONE_HARD:
        case SFX_VFX_DIRT_LITE:
        case SFX_VFX_DIRT_HARD:
        case SFX_VFX_GRILL_LITE:
        case SFX_VFX_GRILL_HARD:
        case SFX_VFX_GRASS_LITE:
        case SFX_VFX_GRASS_HARD:
        case SFX_VFX_MUD_LITE:
        case SFX_VFX_MUD_HARD:
        case SFX_VFX_PUDDLE_LITE:
        case SFX_VFX_PUDDLE_HARD:
        case SFX_VFX_GLASS_LITE:
        case SFX_VFX_GLASS_HARD:
        case SFX_VFX_GRAVEL_LITE:
        case SFX_VFX_GRAVEL_HARD:
        case SFX_VFX_SAND_LITE:
        case SFX_VFX_SAND_HARD:
        case SFX_VFX_FOLIAGE_LITE:
        case SFX_VFX_FOLIAGE_HARD:
        case SFX_VFX_SNOW_LITE:
        case SFX_VFX_SNOW_HARD:
        case SFX_VFX_CARPET_LITE:
        case SFX_VFX_CARPET_HARD:
            szEffectModel = "models/fx/bh_stone_hard.tik";
            break;
        case SFX_EXP_GREN_BASE:
            szEffectModel = "models/fx/grenexp_base.tik";
            break;
        case SFX_EXP_BAZOOKA_BASE:
            szEffectModel = "models/fx/bazookaexp_base.tik";
            break;
        case SFX_EXP_HEAVYSHELL_BASE:
            szEffectModel = "models/fx/heavyshellexp_base.tik";
            break;
        case SFX_EXP_TANK_BASE:
            szEffectModel = "models/fx/tankexp_base.tik";
            break;
        case SFX_EXP_GREN_PAPER:
            szEffectModel = "models/fx/grenexp_paper.tik";
            break;
        case SFX_EXP_GREN_WOOD:
            szEffectModel = "models/fx/grenexp_wood.tik";
            break;
        case SFX_EXP_GREN_METAL:
            szEffectModel = "models/fx/grenexp_metal.tik";
            break;
        case SFX_EXP_GREN_STONE:
            szEffectModel = "models/fx/grenexp_stone.tik";
            break;
        case SFX_EXP_GREN_DIRT:
            szEffectModel = "models/fx/grenexp_dirt.tik";
            break;
        case SFX_EXP_GREN_GRILL:
            szEffectModel = "models/fx/grenexp_metal.tik";
            break;
        case SFX_EXP_GREN_GRASS:
            szEffectModel = "models/fx/grenexp_grass.tik";
            break;
        case SFX_EXP_GREN_MUD:
            szEffectModel = "models/fx/grenexp_mud.tik";
            break;
        case SFX_EXP_GREN_PUDDLE:
            szEffectModel = "models/fx/grenexp_water.tik";
            break;
        case SFX_EXP_GREN_GLASS:
        case SFX_TEST_EFFECT:
            continue;
        case SFX_EXP_GREN_GRAVEL:
            szEffectModel = "models/fx/grenexp_gravel.tik";
            break;
        case SFX_EXP_GREN_SAND:
            szEffectModel = "models/fx/grenexp_sand.tik";
            break;
        case SFX_EXP_GREN_FOLIAGE:
            szEffectModel = "models/fx/grenexp_foliage.tik";
            break;
        case SFX_EXP_GREN_SNOW:
            szEffectModel = "models/fx/grenexp_snow.tik";
            break;
        case SFX_EXP_GREN_CARPET:
            szEffectModel = "models/fx/grenexp_carpet.tik";
            break;
        case SFX_EXP_HEAVYSHELL_DIRT:
            szEffectModel = "models/fx/heavyshellexp_dirt.tik";
            break;
        case SFX_EXP_HEAVYSHELL_STONE:
            szEffectModel = "models/fx/heavyshellexp_stone.tik";
            break;
        case SFX_EXP_HEAVYSHELL_SNOW:
            szEffectModel = "models/fx/heavyshellexp_snow.tik";
            break;
        case SFX_EXP_TANK_DIRT:
            szEffectModel = "models/fx/tankexp_dirt.tik";
            break;
        case SFX_EXP_TANK_STONE:
            szEffectModel = "models/fx/tankexp_stone.tik";
            break;
        case SFX_EXP_TANK_SNOW:
            szEffectModel = "models/fx/tankexp_snow.tik";
            break;
        case SFX_EXP_BAZOOKA_DIRT:
            szEffectModel = "models/fx/bazookaexp_dirt.tik";
            break;
        case SFX_EXP_BAZOOKA_STONE:
            szEffectModel = "models/fx/bazookaexp_stone.tik";
            break;
        case SFX_EXP_BAZOOKA_SNOW:
            szEffectModel = "models/fx/bazookaexp_snow.tik";
            break;
        case SFX_WATER_RIPPLE_STILL:
            szEffectModel = "models/fx/water_ripple_still.tik";
            break;
        case SFX_WATER_RIPPLE_MOVING:
            szEffectModel = "models/fx/water_ripple_moving.tik";
            break;
        case SFX_OIL_LEAK_BIG:
            szEffectModel = "models/fx/barrel_oil_leak_big.tik";
            break;
        case SFX_OIL_LEAK_MEDIUM:
            szEffectModel = "models/fx/barrel_oil_leak_medium.tik";
            break;
        case SFX_OIL_LEAK_SMALL:
            szEffectModel = "models/fx/barrel_oil_leak_small.tik";
            break;
        case SFX_OIL_LEAK_SPLAT:
            szEffectModel = "models/fx/barrel_oil_leak_splat.tik";
            break;
        case SFX_WATER_LEAK_BIG:
            szEffectModel = "models/fx/barrel_water_leak_big.tik";
            break;
        case SFX_WATER_LEAK_MEDIUM:
            szEffectModel = "models/fx/barrel_water_leak_medium.tik";
            break;
        case SFX_WATER_LEAK_SMALL:
            szEffectModel = "models/fx/barrel_water_leak_small.tik";
            break;
        case SFX_WATER_LEAK_SPLAT:
            szEffectModel = "models/fx/barrel_water_leak_splat.tik";
            break;
        case SFX_FOOT_LIGHT_DUST:
            szEffectModel = "models/fx/fs_light_dust.tik";
            break;
        case SFX_FOOT_HEAVY_DUST:
            szEffectModel = "models/fx/fs_heavy_dust.tik";
            break;
        case SFX_FOOT_DIRT:
            szEffectModel = "models/fx/fs_dirt.tik";
            break;
        case SFX_FOOT_GRASS:
            szEffectModel = "models/fx/fs_grass.tik";
            break;
        case SFX_FOOT_MUD:
            szEffectModel = "models/fx/fs_mud.tik";
            break;
        case SFX_FOOT_PUDDLE:
            szEffectModel = "models/fx/fs_puddle.tik";
            break;
        case SFX_FOOT_SAND:
            szEffectModel = "models/fx/fs_sand.tik";
            break;
        case SFX_FOOT_SNOW:
            szEffectModel = "models/fx/fs_snow.tik";
            break;
        case SFX_FENCE_WOOD:
            szEffectModel = "models/fx/fx_fence_wood.tik";
            break;
        default:
            szEffectModel = "models/fx/bh_stone_hard.tik";
            break;
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
                m_iNumPendingEvents++;

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
            
                (commandManager.*pCommand->endfcn)();
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
            iEffectNum = SFX_FOOT_PUDDLE;
        }
    } else {
        surftype = trace->surfaceFlags & MASK_SURF_TYPE;
        switch (surftype) {
        case SURF_FOLIAGE:
            sSoundName += "foliage";
            iEffectNum = SFX_FOOT_GRASS;
            break;
        case SURF_SNOW:
            sSoundName += "snow";
            iEffectNum = SFX_FOOT_SNOW;
            break;
        case SURF_CARPET:
            sSoundName += "carpet";
            iEffectNum = SFX_FOOT_LIGHT_DUST;
            break;
        case SURF_SAND:
            sSoundName += "sand";
            iEffectNum = SFX_FOOT_SAND;
            break;
        case SURF_PUDDLE:
            sSoundName += "puddle";
            iEffectNum = SFX_FOOT_PUDDLE;
            break;
        case SURF_GLASS:
            sSoundName += "glass";
            iEffectNum = SFX_FOOT_LIGHT_DUST;
            break;
        case SURF_GRAVEL:
            sSoundName += "gravel";
            iEffectNum = SFX_FOOT_HEAVY_DUST;
            break;
        case SURF_MUD:
            sSoundName += "mud";
            iEffectNum = SFX_FOOT_MUD;
            break;
        case SURF_DIRT:
            sSoundName += "dirt";
            iEffectNum = SFX_FOOT_DIRT;
            break;
        case SURF_GRILL:
            sSoundName += "grill";
            iEffectNum = SFX_FOOT_LIGHT_DUST;
            break;
        case SURF_GRASS:
            sSoundName += "grass";
            iEffectNum = SFX_FOOT_GRASS;
            break;
        case SURF_ROCK:
            sSoundName += "stone";
            iEffectNum = SFX_FOOT_HEAVY_DUST;
            break;
        case SURF_PAPER:
            sSoundName += "paper";
            iEffectNum = SFX_FOOT_LIGHT_DUST;
            break;
        case SURF_WOOD:
            sSoundName += "wood";
            iEffectNum = SFX_FOOT_LIGHT_DUST;
            break;
        case SURF_METAL:
            sSoundName += "metal";
            iEffectNum = SFX_FOOT_LIGHT_DUST;
            break;
        default:
            sSoundName += "stone";
            iEffectNum = SFX_FOOT_HEAVY_DUST;
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
            iEffectNum = SFX_FOOT_PUDDLE;
        }
    } else {
        surftype = trace.surfaceFlags & MASK_SURF_TYPE;
        switch (surftype) {
        case SURF_FOLIAGE:
            sSoundName += "foliage";
            iEffectNum = SFX_FOOT_GRASS;
            break;
        case SURF_SNOW:
            sSoundName += "snow";
            iEffectNum = SFX_FOOT_SNOW;
            break;
        case SURF_CARPET:
            sSoundName += "carpet";
            break;
        case SURF_SAND:
            sSoundName += "sand";
            iEffectNum = SFX_FOOT_SAND;
            break;
        case SURF_PUDDLE:
            sSoundName += "puddle";
            iEffectNum = SFX_FOOT_PUDDLE;
            break;
        case SURF_GLASS:
            sSoundName += "glass";
            break;
        case SURF_GRAVEL:
            sSoundName += "gravel";
            iEffectNum = SFX_FOOT_HEAVY_DUST;
            break;
        case SURF_MUD:
            sSoundName += "mud";
            iEffectNum = SFX_FOOT_MUD;
            break;
        case SURF_DIRT:
            sSoundName += "dirt";
            iEffectNum = SFX_FOOT_DIRT;
            break;
        case SURF_GRILL:
            sSoundName += "grill";
            break;
        case SURF_GRASS:
            sSoundName += "grass";
            iEffectNum = SFX_FOOT_GRASS;
            break;
        case SURF_ROCK:
            sSoundName += "stone";
            iEffectNum = SFX_FOOT_LIGHT_DUST;
            break;
        case SURF_PAPER:
            sSoundName += "paper";
            break;
        case SURF_WOOD:
            sSoundName += "wood";
            break;
        case SURF_METAL:
            sSoundName += "metal";
            break;
        default:
            sSoundName += "stone";
            iEffectNum = SFX_FOOT_LIGHT_DUST;
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
    dist    = VectorNormalize(diff);

    // See if enough time has passed to add another ripple

    if (dist >= 1) {
        time_required = 60 - dist;
    } else {
        time_required = 175;
    }

    if (time_required < 5) {
        time_required = 5;
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
    start[2] += 72;

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

    if (dist > 0) {
        vectoangles(diff, start);
        start[0] = -90.0;

        dist /= cg.frametime;
        if (dist < 0) {
            dist = 0;
        } else if (dist > 8192) {
            dist = 8192;
        }

        VectorMA(trace.endpos, dist, diff, end);
        sfxManager.MakeEffect_Angles(81, end, start);
    } else {
        start[0] = 90;
        start[1] = 0;
        start[2] = 0;

        sfxManager.MakeEffect_Angles(80, trace.endpos, start);
    }
}

qboolean ClientSpecialEffectsManager::EffectsPending()
{
    return sfxManager.m_iNumPendingEvents > 0;
}

specialeffect_t *ClientSpecialEffectsManager::GetTestEffectPointer()
{
    return &m_effects[SPECIAL_EFFECT_TEST];
}
