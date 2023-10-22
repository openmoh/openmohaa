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

// actor_weaponless.cpp

#include "actor.h"

void Actor::InitWeaponless(GlobalFuncs_t *func)
{
    func->ThinkState                 = &Actor::Think_Weaponless;
    func->BeginState                 = &Actor::Begin_Weaponless;
    func->SuspendState               = &Actor::Suspend_Weaponless;
    func->PassesTransitionConditions = &Actor::PassesTransitionConditions_Attack;
    func->FinishedAnimation          = &Actor::FinishedAnimation_Weaponless;
    func->IsState                    = &Actor::IsAttackState;
}

void Actor::Begin_Weaponless(void)
{
    DoForceActivate();

    m_csMood     = STRING_ALERT;
    m_csIdleMood = STRING_NERVOUS;

    if (level.inttime < m_iEnemyChangeTime + 200) {
        SetLeashHome(origin);
        if (AttackEntryAnimation()) {
            m_bLockThinkState = true;

            TransitionState(ACTOR_STATE_WEAPONLESS_LOOP, 0);
        }
    }
    TransitionState(ACTOR_STATE_WEAPONLESS_START, 0);
}

void Actor::Suspend_Weaponless(void)
{
    if (m_State == ACTOR_STATE_WEAPONLESS_GRENADE || m_State == ACTOR_STATE_WEAPONLESS_LOOP) {
        TransitionState(ACTOR_STATE_WEAPONLESS_START, 0);
    }
}

void Actor::State_Weaponless_Normal(void)
{
    int iStateTime;
    if (m_bScriptGoalValid) {
        SetPath(m_vScriptGoal, NULL, 0, NULL, 0);
    }

    if (PathExists() && !PathComplete()) {
        FaceMotion();
        Anim_RunToDanger(ANIM_MODE_PATH_GOAL);
        return;
    }

    m_bScriptGoalValid = false;

    AimAtTargetPos();
    Anim_Stand();

    if (level.inttime < m_iStateTime) {
        return;
    }

    if (DecideToThrowGrenade(m_Enemy->origin + m_Enemy->velocity, &m_vGrenadeVel, &m_eGrenadeMode, false)) {
        SetDesiredYawDir(m_vGrenadeVel);
        DesiredAnimation(
            ANIM_MODE_NORMAL,
            m_eGrenadeMode == AI_GREN_TOSS_ROLL ? STRING_ANIM_GRENADETOSS_SCR : STRING_ANIM_GRENADETHROW_SCR
        );
        TransitionState(ACTOR_STATE_WEAPONLESS_GRENADE);
    } else {
        TransitionState(ACTOR_STATE_WEAPONLESS_START, 1000);
    }
}

void Actor::Think_Weaponless(void)
{
    if (!RequireThink()) {
        return;
    }

    UpdateEyeOrigin();
    NoPoint();
    UpdateEnemy(500);

    if (m_State == ACTOR_STATE_WEAPONLESS_LOOP) {
        ContinueAnimation();
    } else {
        m_bLockThinkState = false;

        if (!m_Enemy) {
            SetThinkState(THINKSTATE_IDLE, THINKLEVEL_IDLE);
            IdleThink();
            return;
        }

        if (m_State == ACTOR_STATE_WEAPONLESS_START) {
            State_Weaponless_Normal();
        } else if (m_State == ACTOR_STATE_WEAPONLESS_GRENADE) {
            State_Weaponless_Grenade();
        } else {
            Com_Printf("Think_Weaponless: invalid think state %i\n", m_State);
            char assertStr[16317] = {0};
            strcpy(assertStr, "\"invalid think state\"\n\tMessage: ");
            Q_strcat(assertStr, sizeof(assertStr), DumpCallTrace("thinkstate = %i", m_State));
            assert(!assertStr);
        }

        if (!CheckForTransition(THINKSTATE_GRENADE, THINKLEVEL_IDLE)) {
            CheckForTransition(THINKSTATE_GRENADE, THINKLEVEL_IDLE);
        }
    }

    PostThink(true);

    if (GetWeapon(WEAPON_MAIN)) {
        SetThink(THINKSTATE_ATTACK, THINK_TURRET);
    }
}

void Actor::FinishedAnimation_Weaponless(void)
{
    if (m_State == ACTOR_STATE_WEAPONLESS_GRENADE || m_State == ACTOR_STATE_WEAPONLESS_LOOP) {
        TransitionState(ACTOR_STATE_WEAPONLESS_NORMAL, 4000);
    }
}

void Actor::State_Weaponless_Grenade(void)
{
    GenericGrenadeTossThink();
}
