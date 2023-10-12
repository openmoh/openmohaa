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

// actor_disguise_officier.cpp

#include "actor.h"

void Actor::InitDisguiseOfficer(GlobalFuncs_t *func)
{
    func->ThinkState                 = &Actor::Think_DisguiseOfficer;
    func->BeginState                 = &Actor::Begin_DisguiseOfficer;
    func->EndState                   = &Actor::End_DisguiseOfficer;
    func->ResumeState                = &Actor::Resume_DisguiseOfficer;
    func->SuspendState               = &Actor::Suspend_DisguiseOfficer;
    func->PassesTransitionConditions = &Actor::PassesTransitionConditions_Disguise;
    func->IsState                    = &Actor::IsDisguiseState;
}

void Actor::Begin_DisguiseOfficer(void)
{
    vec2_t vDelta;
    m_csMood = STRING_BORED;
    assert(m_Enemy);

    if (m_Enemy) {
        if ((EnemyIsDisguised() || m_Enemy->IsSubclassOfActor()) && !level.m_bAlarm) {
            VectorSub2D(m_Enemy->origin, origin, vDelta);

            if (vDelta[0] != 0 || vDelta[1] != 0) {
                SetDesiredYawDir(vDelta);
            }

            SetDesiredLookDir(m_Enemy->origin - origin);

            m_eNextAnimMode    = 1;
            m_csNextAnimString = STRING_ANIM_DISGUISE_PAPERS_SCR;
            m_bNextForceStart  = false;

            m_iEnemyShowPapersTime = m_Enemy->m_ShowPapersTime;
            TransitionState(1, 0);
        } else {
            SetThinkState(THINKSTATE_ATTACK, THINKLEVEL_NORMAL);
        }
    } else {
        SetThinkState(THINKSTATE_IDLE, THINKLEVEL_NORMAL);
    }
}

void Actor::End_DisguiseOfficer(void)
{
    m_iNextDisguiseTime = level.inttime + (m_State ? m_iDisguisePeriod : 500);
}

void Actor::Resume_DisguiseOfficer(void)
{
    Begin_DisguiseOfficer();
}

void Actor::Suspend_DisguiseOfficer(void)
{
    End_DisguiseOfficer();
}

void Actor::Think_DisguiseOfficer(void)
{
    if (RequireThink()) {
        UpdateEyeOrigin();
        NoPoint();
        ContinueAnimation();
        UpdateEnemy(1500);
        assert(m_Enemy);

        if (!m_Enemy) {
            SetThinkState(THINKSTATE_IDLE, THINKLEVEL_NORMAL);
            return;
        }
        if (!EnemyIsDisguised() && !(m_Enemy->IsSubclassOfActor()) && m_State != 3) {
            TransitionState(3, 0);
        }
        if (level.m_bAlarm) {
            SetThinkState(THINKSTATE_ATTACK, THINKLEVEL_NORMAL);
            return;
        }
        vec2_t vDelta;

        VectorSub2D(m_Enemy->origin, origin, vDelta);

        if (vDelta[0] != 0 || vDelta[1] != 0) {
            SetDesiredYawDir(vDelta);
        }

        SetDesiredLookDir(m_Enemy->origin - origin);

        if (m_State == 3) {
            m_pszDebugState = "enemy";
            State_Disguise_Enemy();
        } else if (m_State > 3) {
            if (m_State != 4) {
                Com_Printf("Actor::Think_DisguiseOfficer: invalid think state %i\n", m_State);
                char assertStr[16317] = {0};
                strcpy(assertStr, "\"invalid think state\"\n\tMessage: ");
                Q_strcat(assertStr, sizeof(assertStr), DumpCallTrace("thinkstate = %i", m_State));
                assert(!assertStr);
            } else {
                m_pszDebugState = "halt";
                Actor::State_Disguise_Halt();
            }
        } else {
            if (m_State == 1) {
                m_pszDebugState = "papers";
                State_Disguise_Fake_Papers();
            } else {
                char assertStr[16317] = {0};
                strcpy(assertStr, "\"invalid think state\"\n\tMessage: ");
                Q_strcat(assertStr, sizeof(assertStr), DumpCallTrace("thinkstate = %i", m_State));
                assert(!assertStr);
            }
        }
        CheckForTransition(THINKSTATE_GRENADE, THINKLEVEL_NORMAL);
        PostThink(true);
    }
}
