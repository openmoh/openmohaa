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

#pragma once

#include "cg_local.h"
#include "cg_commands.h"
#include "class.h"

#define MAX_SPECIAL_EFFECT_COMMANDS 32

typedef enum {
    SFX_BHIT_PAPER_LITE,
    SFX_BHIT_PAPER_HARD,
    SFX_BHIT_WOOD_LITE,
    SFX_BHIT_WOOD_HARD,
    SFX_BHIT_METAL_LITE,
    SFX_BHIT_METAL_HARD,
    SFX_BHIT_STONE_LITE,
    SFX_BHIT_STONE_HARD,
    SFX_BHIT_DIRT_LITE,
    SFX_BHIT_DIRT_HARD,
    SFX_BHIT_GRILL_LITE,
    SFX_BHIT_GRILL_HARD,
    SFX_BHIT_GRASS_LITE,
    SFX_BHIT_GRASS_HARD,
    SFX_BHIT_MUD_LITE,
    SFX_BHIT_MUD_HARD,
    SFX_BHIT_PUDDLE_LITE,
    SFX_BHIT_PUDDLE_HARD,
    SFX_BHIT_GLASS_LITE,
    SFX_BHIT_GLASS_HARD,
    SFX_BHIT_GRAVEL_LITE,
    SFX_BHIT_GRAVEL_HARD,
    SFX_BHIT_SAND_LITE,
    SFX_BHIT_SAND_HARD,
    SFX_BHIT_FOLIAGE_LITE,
    SFX_BHIT_FOLIAGE_HARD,
    SFX_BHIT_SNOW_LITE,
    SFX_BHIT_SNOW_HARD,
    SFX_BHIT_CARPET_LITE,
    SFX_BHIT_CARPET_HARD,
    SFX_BHIT_HUMAN_UNIFORM_LITE,
    SFX_BHIT_HUMAN_UNIFORM_HARD,
    SFX_WATER_TRAIL_BUBBLE,
    SFX_VFX_PAPER_LITE,
    SFX_VFX_PAPER_HARD,
    SFX_VFX_WOOD_LITE,
    SFX_VFX_WOOD_HARD,
    SFX_VFX_METAL_LITE,
    SFX_VFX_METAL_HARD,
    SFX_VFX_STONE_LITE,
    SFX_VFX_STONE_HARD,
    SFX_VFX_DIRT_LITE,
    SFX_VFX_DIRT_HARD,
    SFX_VFX_GRILL_LITE,
    SFX_VFX_GRILL_HARD,
    SFX_VFX_GRASS_LITE,
    SFX_VFX_GRASS_HARD,
    SFX_VFX_MUD_LITE,
    SFX_VFX_MUD_HARD,
    SFX_VFX_PUDDLE_LITE,
    SFX_VFX_PUDDLE_HARD,
    SFX_VFX_GLASS_LITE,
    SFX_VFX_GLASS_HARD,
    SFX_VFX_GRAVEL_LITE,
    SFX_VFX_GRAVEL_HARD,
    SFX_VFX_SAND_LITE,
    SFX_VFX_SAND_HARD,
    SFX_VFX_FOLIAGE_LITE,
    SFX_VFX_FOLIAGE_HARD,
    SFX_VFX_SNOW_LITE,
    SFX_VFX_SNOW_HARD,
    SFX_VFX_CARPET_LITE,
    SFX_VFX_CARPET_HARD,
    SFX_EXP_GREN_BASE,
    SFX_EXP_BAZOOKA_BASE,
    SFX_EXP_HEAVYSHELL_BASE,
    SFX_EXP_TANK_BASE,
    SFX_EXP_GREN_PAPER,
    SFX_EXP_GREN_WOOD,
    SFX_EXP_GREN_METAL,
    SFX_EXP_GREN_STONE,
    SFX_EXP_GREN_DIRT,
    SFX_EXP_GREN_GRILL,
    SFX_EXP_GREN_GRASS,
    SFX_EXP_GREN_MUD,
    SFX_EXP_GREN_PUDDLE,
    SFX_EXP_GREN_GLASS,
    SFX_EXP_GREN_GRAVEL,
    SFX_EXP_GREN_SAND,
    SFX_EXP_GREN_FOLIAGE,
    SFX_EXP_GREN_SNOW,
    SFX_EXP_GREN_CARPET,
    SFX_EXP_HEAVYSHELL_DIRT,
    SFX_EXP_HEAVYSHELL_STONE,
    SFX_EXP_HEAVYSHELL_SNOW,
    SFX_EXP_TANK_DIRT,
    SFX_EXP_TANK_STONE,
    SFX_EXP_TANK_SNOW,
    SFX_EXP_BAZOOKA_DIRT,
    SFX_EXP_BAZOOKA_STONE,
    SFX_EXP_BAZOOKA_SNOW,
    SFX_WATER_RIPPLE_STILL,
    SFX_WATER_RIPPLE_MOVING,
    SFX_OIL_LEAK_BIG,
    SFX_OIL_LEAK_MEDIUM,
    SFX_OIL_LEAK_SMALL,
    SFX_OIL_LEAK_SPLAT,
    SFX_WATER_LEAK_BIG,
    SFX_WATER_LEAK_MEDIUM,
    SFX_WATER_LEAK_SMALL,
    SFX_WATER_LEAK_SPLAT,
    SFX_FOOT_LIGHT_DUST,
    SFX_FOOT_HEAVY_DUST,
    SFX_FOOT_DIRT,
    SFX_FOOT_GRASS,
    SFX_FOOT_MUD,
    SFX_FOOT_PUDDLE,
    SFX_FOOT_SAND,
    SFX_FOOT_SNOW,
    SFX_FENCE_WOOD,
    SFX_TEST_EFFECT,
    SFX_COUNT
} SpecialEffects;

class specialeffectcommand_t : public Class
{
public:
    spawnthing_t *emitter;
    float         fCommandTime;
    void (ClientGameCommandManager::*endfcn)();
    Event *pEvent;

public:
    specialeffectcommand_t();
};

class specialeffect_t : public Class
{
public:
    int                     m_iCommandCount;
    specialeffectcommand_t *m_commands[MAX_SPECIAL_EFFECT_COMMANDS];

public:
    specialeffect_t();

    specialeffectcommand_t *AddNewCommand();
};

#define MAX_SPECIAL_EFFECTS 99
#define SPECIAL_EFFECT_TEST MAX_SPECIAL_EFFECTS - 1

class ClientSpecialEffectsManager : public Listener
{
    specialeffect_t m_effects[99];
    qboolean        m_bEffectsLoaded;
    int             m_iNumPendingEvents;

public:
    CLASS_PROTOTYPE(ClientSpecialEffectsManager);

private:
    void ContinueEffectExecution(Event *ev);
    void ExecuteEffect(int iEffect, int iStartCommand, Vector vPos, Vector vAngles, float axis[3][3]);

public:
    ClientSpecialEffectsManager();

    void             LoadEffects();
    qboolean         EffectsPending();
    void             MakeEffect_Normal(int iEffect, Vector vPos, Vector vNormal);
    void             MakeEffect_Angles(int iEffect, Vector vPos, Vector vAngles);
    void             MakeEffect_Axis(int iEffect, Vector vPos, float axis[3][3]);
    specialeffect_t *GetTestEffectPointer();
};

extern ClientSpecialEffectsManager sfxManager;
