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

// actor_runandshoot.cpp

#include "actor.h"

void Actor::InitRunAndShoot(GlobalFuncs_t *func)
{
    func->ThinkState                 = &Actor::Think_RunAndShoot;
    func->BeginState                 = &Actor::Begin_RunAndShoot;
    func->EndState                   = &Actor::End_RunAndShoot;
    func->ResumeState                = &Actor::Resume_RunAndShoot;
    func->PassesTransitionConditions = &Actor::PassesTransitionConditions_Attack;
    func->ShowInfo                   = &Actor::ShowInfo_RunAndShoot;
    func->IsState                    = &Actor::IsAttackState;
}

void Actor::Begin_RunAndShoot(void)
{
    m_State = ACTOR_STATE_RUN_AND_SHOOT_RUN;
    if (m_patrolCurrentNode) {
        m_State = ACTOR_STATE_RUN_AND_SHOOT_RUNNING;
    }
}

void Actor::End_RunAndShoot(void)
{
    m_vHome      = origin;
    m_pTetherEnt = NULL;
}

void Actor::Resume_RunAndShoot(void) {}

void Actor::Think_RunAndShoot(void)
{
    if (m_State == ACTOR_STATE_RUN_AND_SHOOT_RUNNING) {
        State_RunAndShoot_Running();
    } else if (m_State == ACTOR_STATE_RUN_AND_SHOOT_RUN) {
        SetThink(THINKSTATE_ATTACK, THINK_COVER);
    }
}

void Actor::ShowInfo_RunAndShoot(void) {}

void Actor::State_RunAndShoot_Running(void)
{
    Entity *player;
    bool    bMoveDone;

    if (!RequireThink()) {
        return;
    }

    parm.movefail = false;
    UpdateEyeOrigin();
    NoPoint();

    m_pszDebugState = "";
    bMoveDone       = RunAndShoot_MoveToPatrolCurrentNode();

    CheckForThinkStateTransition();
    player = G_GetEntity(0);
    if (player) {
        vec2_t delta;

        VectorSub2D(player->origin, origin, delta);

        if (VectorLength2DSquared(delta) < Square(150)) {
            ClearPatrolCurrentNode();
        }
    }

    if (!m_patrolCurrentNode) {
        if (m_Enemy) {
            SetThink(THINKSTATE_ATTACK, THINK_TURRET);
        } else {
            SetThinkIdle(THINK_IDLE);
        }

        m_bScriptGoalValid = false;
        parm.movedone      = true;

        Unregister(STRING_MOVEDONE);
    } else if (bMoveDone) {
        ClearPatrolCurrentNode();

        if (m_Enemy) {
            SetThink(THINKSTATE_ATTACK, THINK_TURRET);
        } else {
            SetThinkIdle(THINK_IDLE);
        }

        parm.movedone = true;

        Unregister(STRING_MOVEDONE);
    }

    Unregister(STRING_MOVE);

    CheckForTransition(THINKSTATE_BADPLACE, THINKLEVEL_IDLE);
    PostThink(true);
}

bool Actor::RunAndShoot_MoveToPatrolCurrentNode(void)
{
    UpdatePatrolCurrentNode();

    if (!m_patrolCurrentNode && m_bPatrolWaitTrigger) {
        IdleLook();
        Anim_Idle();
        return false;
    }

    SetPath(m_patrolCurrentNode->origin, "Actor::RunAndShoot_MoveToPatrolCurrentNode", 0, NULL, 0);

    if (!PathExists()) {
        IdleLook();
        Anim_Idle();

        parm.movefail = true;
        return false;
    }

    if (MoveOnPathWithSquad()) {
        if (m_Enemy) {
            m_eNextAnimMode    = ANIM_MODE_PATH;
            m_csNextAnimString = STRING_ANIM_RUNTO_INOPEN_SCR;
            m_bNextForceStart  = false;

            Vector delta;
            Vector lookDir;

            delta   = mTargetPos - EyePosition();
            lookDir = delta;
            lookDir[2] += 16;
            SetDesiredLookDir(lookDir);

            m_DesiredGunDir[0] = 360 - delta.toPitch();
            m_DesiredGunDir[1] = delta.toYaw();
            m_DesiredGunDir[2] = 0;
            SetDesiredYaw(m_DesiredGunDir[1]);
        } else {
            m_eNextAnimMode    = ANIM_MODE_PATH;
            m_csNextAnimString = STRING_ANIM_CROUCH_RUN_SCR;
            m_bNextForceStart  = false;
            FaceMotion();
        }
    } else if (m_Enemy) {
        Vector delta;
        Vector lookDir;

        delta   = mTargetPos - EyePosition();
        lookDir = delta;
        lookDir[2] += 16;
        SetDesiredLookDir(lookDir);

        m_DesiredGunDir[0] = 360 - delta.toPitch();
        m_DesiredGunDir[1] = delta.toYaw();
        m_DesiredGunDir[2] = 0;
        SetDesiredYaw(m_DesiredGunDir[1]);
        Anim_Attack();
    } else {
        Anim_Stand();
        IdleLook();
    }

    if (!m_fMoveDoneRadiusSquared) {
        return false;
    }

    if (PathComplete()) {
        return true;
    }

    if (!m_Path.HasCompleteLookahead() || m_patrolCurrentNode->Next()) {
        return false;
    }

    if (VectorLength2DSquared(PathDelta()) > m_fMoveDoneRadiusSquared) {
        return false;
    }

    return true;
}
