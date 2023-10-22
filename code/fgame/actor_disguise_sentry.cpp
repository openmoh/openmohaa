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

// actor.cpp:

#include "actor.h"

void Actor::InitDisguiseSentry(GlobalFuncs_t *func)
{
    func->ThinkState                 = &Actor::Think_DisguiseSentry;
    func->BeginState                 = &Actor::Begin_DisguiseSentry;
    func->EndState                   = &Actor::End_DisguiseSentry;
    func->ResumeState                = &Actor::Resume_DisguiseSentry;
    func->SuspendState               = &Actor::Suspend_DisguiseSentry;
    func->PassesTransitionConditions = &Actor::PassesTransitionConditions_Disguise;
    func->IsState                    = &Actor::IsDisguiseState;
}

void Actor::Begin_DisguiseSentry(void)
{
    m_csMood = STRING_BORED;
    assert(m_Enemy);

    if (!m_Enemy) {
        SetThinkState(THINKSTATE_IDLE, THINKLEVEL_IDLE);
        return;
    }

    if ((EnemyIsDisguised() || (m_Enemy->flags & FL_NOTARGET)) && level.m_bAlarm != qtrue) {
        SetDesiredYawDest(m_Enemy->origin);
        SetDesiredLookDir(m_Enemy->origin - origin);

        DesiredAnimation(ANIM_MODE_NORMAL, STRING_ANIM_DISGUISE_WAIT_SCR);
        m_iEnemyShowPapersTime = m_Enemy->m_ShowPapersTime;

        TransitionState(ACTOR_STATE_DISGUISE_WAIT);
    } else {
        SetThinkState(THINKSTATE_ATTACK, THINKLEVEL_IDLE);
    }
}

void Actor::End_DisguiseSentry(void)
{
    m_iNextDisguiseTime = level.inttime + (m_State ? m_iDisguisePeriod : 500);
}

void Actor::Resume_DisguiseSentry(void)
{
    Begin_DisguiseSentry();
}

void Actor::Suspend_DisguiseSentry(void)
{
    End_DisguiseSentry();
}

void Actor::Think_DisguiseSentry(void)
{
    if (!RequireThink()) {
        return;
    }

    UpdateEyeOrigin();
    NoPoint();
    ContinueAnimation();
    UpdateEnemy(1500);

    assert(m_Enemy != NULL);

    if (!m_Enemy) {
        SetThinkState(THINKSTATE_IDLE, THINKLEVEL_IDLE);
        return;
    }
    if (!EnemyIsDisguised() && !(m_Enemy->flags & FL_NOTARGET) && m_State != ACTOR_STATE_DISGUISE_ENEMY) {
        TransitionState(ACTOR_STATE_DISGUISE_ENEMY, 0);
    }

    if (level.m_bAlarm) {
        SetThinkState(THINKSTATE_ATTACK, THINKLEVEL_IDLE);
        return;
    }

    SetDesiredYawDest(m_Enemy->origin);
    SetDesiredLookDir(m_Enemy->origin - origin);

    switch (m_State) {
    case ACTOR_STATE_DISGUISE_WAIT:
        m_pszDebugState = "wait";
        State_Disguise_Wait();
        break;
    case ACTOR_STATE_DISGUISE_PAPERS:
        m_pszDebugState = "papers";
        State_Disguise_Papers();
        break;
    case ACTOR_STATE_DISGUISE_ACCEPT:
        m_pszDebugState = "accept";
        State_Disguise_Accept();
        break;
    case ACTOR_STATE_DISGUISE_ENEMY:
        m_pszDebugState = "enemy";
        State_Disguise_Enemy();
        break;
    case ACTOR_STATE_DISGUISE_HALT:
        m_pszDebugState = "halt";
        State_Disguise_Halt();
        break;
    case ACTOR_STATE_DISGUISE_DENY:
        m_pszDebugState = "deny";
        State_Disguise_Deny();
        break;
    default:
        Com_Printf("Actor::Think_DisguiseSentry: invalid think state %i\n", m_State);
        assert(!"invalid think state");
        break;
    }

    if (!CheckForTransition(THINKSTATE_GRENADE, THINKLEVEL_IDLE)) {
        CheckForTransition(THINKSTATE_GRENADE, THINKLEVEL_IDLE);
    }

    PostThink(true);
}
