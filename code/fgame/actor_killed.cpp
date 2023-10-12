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

// actor_killed.cpp

#include "actor.h"

void Actor::InitDead(GlobalFuncs_t *func)
{
    func->IsState = &Actor::IsKilledState;
}

void Actor::InitKilled(GlobalFuncs_t *func)
{
    func->BeginState        = &Actor::Begin_Killed;
    func->ThinkState        = &Actor::Think_Killed;
    func->FinishedAnimation = &Actor::FinishedAnimation_Killed;
    func->IsState           = &Actor::IsKilledState;
}

void Actor::Begin_Killed(void)
{
    Event e1(EV_Actor_DeathEmbalm); // ebx

    ClearPath();
    ResetBoneControllers();

    PostEvent(e1, 0.05f);
    TransitionState(700, 0);
}

void Actor::Think_Killed(void)
{
    Unregister(STRING_ANIMDONE);
    if (m_State == 700) {
        m_pszDebugState = "begin";
        NoPoint();
        m_bHasDesiredLookAngles = false;
        StopTurning();
        Anim_Killed();
        PostThink(false);
    } else {
        m_pszDebugState = "end";
    }
}

void Actor::FinishedAnimation_Killed(void)
{
    BecomeCorpse();
    TransitionState(701, 0);
}
