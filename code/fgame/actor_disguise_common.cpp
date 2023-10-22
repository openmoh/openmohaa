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
    float  fDistSquared;
    vec2_t vDelta;

    VectorSub2D(origin, m_Enemy->origin, vDelta);
    fDistSquared = VectorLength2DSquared(vDelta);

    DesiredAnimation(ANIM_MODE_NORMAL, STRING_ANIM_DISGUISE_WAIT_SCR);

    if (fDistSquared * 4 < m_fMaxDisguiseDistSquared) {
        TransitionState(ACTOR_STATE_DISGUISE_PAPERS);
        return;
    }

    if (level.inttime <= m_iStateTime + 3000) {
        return;
    }

    if (fDistSquared > Square(256)) {
        SetThinkState(THINKSTATE_IDLE, THINKLEVEL_IDLE);
    } else {
        TransitionState(ACTOR_STATE_DISGUISE_PAPERS);
    }
}

void Actor::State_Disguise_Papers(void)
{
    vec2_t vDelta;

    DesiredAnimation(ANIM_MODE_NORMAL, STRING_ANIM_DISGUISE_PAPERS_SCR);

    if (m_iEnemyShowPapersTime >= m_Enemy->m_ShowPapersTime) {
        if (level.inttime > m_iStateTime + 12000) {
            TransitionState(ACTOR_STATE_DISGUISE_ENEMY);
        } else {
            VectorSub2D(origin, m_Enemy->origin, vDelta);

            if (VectorLength2DSquared(vDelta) > Square(256)) {
                TransitionState(ACTOR_STATE_DISGUISE_HALT, 0);
            }
        }
    } else if (level.m_iPapersLevel < m_iDisguiseLevel) {
        TransitionState(ACTOR_STATE_DISGUISE_DENY);
    } else {
        if (m_DisguiseAcceptThread.IsSet()) {
            m_DisguiseAcceptThread.Execute(this);
        }

        TransitionState(ACTOR_STATE_DISGUISE_ACCEPT);
    }
}

void Actor::State_Disguise_Fake_Papers(void)
{
    vec2_t vDelta;

    DesiredAnimation(ANIM_MODE_NORMAL, STRING_ANIM_DISGUISE_PAPERS_SCR);

    if (m_iEnemyShowPapersTime < m_Enemy->m_ShowPapersTime || level.inttime > m_iStateTime + 12000) {
        TransitionState(ACTOR_STATE_DISGUISE_ENEMY);
        return;
    }

    VectorSub2D(origin, m_Enemy->origin, vDelta);

    if (VectorLength2DSquared(vDelta) > Square(256)) {
        TransitionState(ACTOR_STATE_DISGUISE_HALT, 0);
    }
}

void Actor::State_Disguise_Enemy(void)
{
    DesiredAnimation(ANIM_MODE_NORMAL, STRING_ANIM_DISGUISE_ENEMY_SCR);

    if (level.inttime > m_iStateTime + 3000 && !(m_Enemy->flags & FL_NOTARGET)) {
        SetThinkState(THINKSTATE_ATTACK, THINKLEVEL_IDLE);
    }
}

void Actor::State_Disguise_Halt(void)
{
    DesiredAnimation(ANIM_MODE_NORMAL, STRING_ANIM_DISGUISE_HALT_SCR);

    if (level.inttime > m_iStateTime + 1500 && !(m_Enemy->flags & FL_NOTARGET)) {
        SetThinkState(THINKSTATE_ATTACK, THINKLEVEL_IDLE);
    }
}

void Actor::State_Disguise_Accept(void)
{
    DesiredAnimation(ANIM_MODE_NORMAL, STRING_ANIM_DISGUISE_ACCEPT_SCR);

    if (level.inttime > m_iStateTime + 3000) {
        SetThinkState(THINKSTATE_IDLE, THINKLEVEL_IDLE);
        SetThink(THINKSTATE_DISGUISE, THINK_DISGUISE_SALUTE);
    }
}

void Actor::State_Disguise_Deny(void)
{
    DesiredAnimation(ANIM_MODE_NORMAL, STRING_ANIM_DISGUISE_DENY_SCR);

    if (level.inttime > m_iStateTime + 3000) {
        SetThinkState(THINKSTATE_IDLE, THINKLEVEL_IDLE);
    }
}
