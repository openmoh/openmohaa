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

// actor_idle.cpp

#include "actor.h"

void Actor::InitIdle(GlobalFuncs_t *func)
{
    func->BeginState                 = &Actor::Begin_Idle;
    func->ThinkState                 = &Actor::Think_Idle;
    func->PassesTransitionConditions = &Actor::PassesTransitionConditions_Idle;
    func->IsState                    = &Actor::IsIdleState;
}

void Actor::Begin_Idle(void)
{
    m_csMood = m_csIdleMood;
    ClearPath();
}

void Actor::IdleThink(void)
{
    IdlePoint();
    IdleLook();

    if (PathExists() && PathComplete()) {
        ClearPath();
    }

    if (m_bAutoAvoidPlayer && !PathExists()) {
        SetPathToNotBlockSentient(static_cast<Sentient *>(G_GetEntity(0)));
    }

    if (PathExists()) {
        Anim_WalkTo(ANIM_MODE_PATH);
        if (PathDist() > 128.0) {
            FaceMotion();
        } else {
            IdleTurn();
        }
    } else {
        Anim_Idle();
        IdleTurn();
    }

    PostThink(true);
}

void Actor::Think_Idle(void)
{
    if (!RequireThink()) {
        return;
    }

    UpdateEyeOrigin();
    m_pszDebugState = "";

    CheckForThinkStateTransition();
    IdleThink();
}