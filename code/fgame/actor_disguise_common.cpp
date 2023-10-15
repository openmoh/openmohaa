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

// actor_disguise_common.cpp

#include "actor.h"

void Actor::InitDisguiseNone(GlobalFuncs_t *func)
{
    func->IsState = &Actor::IsDisguiseState;
}

void Actor::State_Disguise_Wait(void)
{
    vec2_t vDelta;
    float  fDistSquared;
    VectorSub2D(origin, m_Enemy->origin, vDelta);

    m_eNextAnimMode    = ANIM_MODE_NORMAL;
    m_csNextAnimString = STRING_ANIM_DISGUISE_WAIT_SCR;
    m_bNextForceStart  = false;

    fDistSquared = VectorLength2DSquared(vDelta);

    if (m_fMaxDisguiseDistSquared > fDistSquared * 4) {
        TransitionState(1, 0);
    } else {
        if (level.inttime > m_iStateTime + 3000) {
            if (fDistSquared <= 65536) {
                TransitionState(1, 0);
            } else {
                SetThinkState(THINKSTATE_IDLE, THINKLEVEL_IDLE);
            }
        }
    }
}

void Actor::State_Disguise_Papers(void)
{
    vec2_t vDelta;

    m_csNextAnimString = STRING_ANIM_DISGUISE_PAPERS_SCR;
    m_eNextAnimMode    = ANIM_MODE_NORMAL;
    m_bNextForceStart  = false;
    if (m_iEnemyShowPapersTime < m_Enemy->m_ShowPapersTime) {
        if (level.m_iPapersLevel < m_iDisguiseLevel) {
            m_State = 5;
        } else {
            if (m_DisguiseAcceptThread.IsSet()) {
                m_DisguiseAcceptThread.Execute(this);
            }
            TransitionState(2, 0);
        }
    } else {
        if (level.inttime > m_iStateTime + 12000) {
            TransitionState(3, 0);
        } else {
            VectorSub2D(origin, m_Enemy->origin, vDelta);

            if (VectorLength2DSquared(vDelta) > 65536) {
                TransitionState(4, 0);
            }
        }
    }
}

void Actor::State_Disguise_Fake_Papers(void)
{
    vec2_t vDelta;

    m_csNextAnimString = STRING_ANIM_DISGUISE_PAPERS_SCR;
    m_eNextAnimMode    = ANIM_MODE_NORMAL;
    m_bNextForceStart  = false;
    if (m_iEnemyShowPapersTime < m_Enemy->m_ShowPapersTime || level.inttime > m_iStateTime + 12000) {
        TransitionState(3, 0);
    } else {
        VectorSub2D(origin, m_Enemy->origin, vDelta);

        if (VectorLength2DSquared(vDelta) > 65536) {
            TransitionState(4, 0);
        }
    }
}

void Actor::State_Disguise_Enemy(void)
{
    m_eNextAnimMode    = ANIM_MODE_NORMAL;
    m_csNextAnimString = STRING_ANIM_DISGUISE_ENEMY_SCR;
    m_bNextForceStart  = false;

    if (level.inttime > m_iStateTime + 3000 && !m_Enemy->IsSubclassOfActor()) {
        SetThinkState(THINKSTATE_ATTACK, THINKLEVEL_IDLE);
    }
}

void Actor::State_Disguise_Halt(void)
{
    m_eNextAnimMode    = ANIM_MODE_NORMAL;
    m_csNextAnimString = STRING_ANIM_DISGUISE_HALT_SCR;
    m_bNextForceStart  = false;

    if (level.inttime > m_iStateTime + 1500 && !m_Enemy->IsSubclassOfActor()) {
        SetThinkState(THINKSTATE_ATTACK, THINKLEVEL_IDLE);
    }
}

void Actor::State_Disguise_Accept(void)
{
    m_eNextAnimMode    = ANIM_MODE_NORMAL;
    m_csNextAnimString = STRING_ANIM_DISGUISE_ACCEPT_SCR;
    m_bNextForceStart  = false;

    if (level.inttime > m_iStateTime + 3000) {
        SetThinkState(THINKSTATE_IDLE, THINKLEVEL_IDLE);
        SetThink(THINKSTATE_DISGUISE, THINK_DISGUISE_SALUTE);
    }
}

void Actor::State_Disguise_Deny(void)
{
    m_eNextAnimMode    = ANIM_MODE_NORMAL;
    m_csNextAnimString = STRING_ANIM_DISGUISE_DENY_SCR;
    m_bNextForceStart  = false;

    if (level.inttime > m_iStateTime + 3000) {
        SetThinkState(THINKSTATE_IDLE, THINKLEVEL_IDLE);
    }
}
