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

// actor_badplace.cpp

#include "actor.h"

void Actor::InitBadPlace(GlobalFuncs_t *func)
{
    func->ThinkState = &Actor::Think_BadPlace;
    func->BeginState = &Actor::Begin_BadPlace;
    func->EndState = &Actor::End_BadPlace;
    func->ResumeState = &Actor::Begin_BadPlace;
    func->SuspendState = &Actor::End_BadPlace;
    func->PassesTransitionConditions = &Actor::PassesTransitionConditions_BadPlace;
    func->IsState = &Actor::IsBadPlaceState;
}

void Actor::Begin_BadPlace(void)
{
    DoForceActivate();

    m_csMood = STRING_ALERT;
    m_csIdleMood = STRING_NERVOUS;

    badplace_t& badplace = level.m_badPlaces.ObjectAt(m_iBadPlaceIndex);

    FindPathAway(badplace.m_vOrigin, origin - badplace.m_vOrigin, badplace.m_fRadius + 64);
}

void Actor::End_BadPlace(void)
{
    m_iBadPlaceIndex = 0;
}

void Actor::Think_BadPlace(void)
{
    if (m_bEnableEnemy) {
        UpdateEnemy(200);
    }

    m_pszDebugState = "";
    NoPoint();

    if (!PathExists() || PathComplete() || !m_iBadPlaceIndex) {
        m_iBadPlaceIndex = 0;
        if (!m_Enemy || (m_Enemy->flags & FL_NOTARGET)) {
            SetThinkState(THINKSTATE_IDLE, THINKLEVEL_NORMAL);
        } else {
            SetThinkState(THINKSTATE_ATTACK, THINKLEVEL_NORMAL);
        }
    }

    if (m_Enemy) {
        m_pszDebugState = "badplace_shoot";
        FaceEnemyOrMotion(m_iStateTime);
        Anim_RunAwayFiring(ANIM_MODE_PATH);
    } else {
        m_pszDebugState = "badplace_run";
        FaceMotion();
        Anim_RunToInOpen(ANIM_MODE_PATH);
    }

    PostThink(false);
}
