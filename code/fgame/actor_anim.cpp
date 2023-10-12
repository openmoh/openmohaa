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

// actor_anim.cpp

#include "actor.h"

void Actor::InitAnim(GlobalFuncs_t *func)
{
    func->ThinkState                 = &Actor::Think_Anim;
    func->BeginState                 = &Actor::Begin_Anim;
    func->PassesTransitionConditions = &Actor::PassesTransitionConditions_Idle;
    func->FinishedAnimation          = &Actor::FinishedAnimation_Anim;
    func->ShowInfo                   = &Actor::ShowInfo_Anim;
    func->IsState                    = &Actor::IsIdleState;
}

void Actor::Begin_Anim(void)
{
    m_csMood = m_csIdleMood;
    ClearPath();
    StopTurning();
    TransitionState(1000, 0);
}

void Actor::Think_Anim(void)
{
    if (RequireThink()) {
        UpdateEyeOrigin();
        IdlePoint();
        IdleLook();
        if (m_State == 1000) {
            m_bNextForceStart = true;

            m_bAnimScriptSet = false;
            m_pszDebugState  = "initial";
            TransitionState(1001, 0);
        } else if (m_State == 1001) {
            m_bNextForceStart = false;
            m_pszDebugState   = "main";
        }
        m_eNextAnimMode    = m_AnimMode;
        m_csNextAnimString = m_csAnimScript;

        CheckForThinkStateTransition();
        IdleTurn();
        PostThink(false);
    }
}

void Actor::FinishedAnimation_Anim(void)
{
    if (!m_bAnimScriptSet) {
        if (m_bNoIdleAfterAnim) {
            m_csAnimScript = STRING_ANIM_CONTINUE_LAST_ANIM_SCR;
        } else {
            SetThinkIdle(THINK_IDLE);
        }
        Unregister(STRING_ANIMDONE);
    }
}

void Actor::ShowInfo_Anim(void)
{
    Com_Printf("anim script: %s, anim mode %d\n", Director.GetString(m_csAnimScript).c_str(), m_AnimMode);
}
