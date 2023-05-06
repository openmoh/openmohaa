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

class specialeffectcommand_t : public Class {
public:
    spawnthing_t* emitter;
    float fCommandTime;
    void (*endfcn)();
    Event *pEvent;

public:
    specialeffectcommand_t();
};

class specialeffect_t : public Class
{
public:
    int m_iCommandCount;
    specialeffectcommand_t* m_commands[MAX_SPECIAL_EFFECT_COMMANDS];

public:
    specialeffect_t();

    specialeffectcommand_t* AddNewCommand();
};

#define MAX_SPECIAL_EFFECTS 99
#define SPECIAL_EFFECT_TEST MAX_SPECIAL_EFFECTS - 1

class ClientSpecialEffectsManager : public Listener {
    specialeffect_t m_effects[99];
    qboolean m_bEffectsLoaded;
    int m_iNumPendingEvents;

public:
    CLASS_PROTOTYPE(ClientSpecialEffectsManager);

private:
    void ContinueEffectExecution(Event* ev);
    void ExecuteEffect(int iEffect, int iStartCommand, Vector vPos, Vector vAngles, float axis[3][3]);

public:
    ClientSpecialEffectsManager();

    void LoadEffects();
    qboolean EffectsPending();
    void MakeEffect_Normal(int iEffect, Vector vPos, Vector vNormal);
    void MakeEffect_Angles(int iEffect, Vector vPos, Vector vAngles);
    void MakeEffect_Axis(int iEffect, Vector vPos, float axis[3][3]);
    specialeffect_t* GetTestEffectPointer();
};

extern ClientSpecialEffectsManager sfxManager;
