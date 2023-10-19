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

// actor_alarm.cpp

#include "actor.h"

void Actor::InitAlarm(GlobalFuncs_t *func)
{
    func->BeginState                 = &Actor::Begin_Alarm;
    func->EndState                   = &Actor::End_Alarm;
    func->ThinkState                 = &Actor::Think_Alarm;
    func->PassesTransitionConditions = &Actor::PassesTransitionConditions_Attack;
    func->IsState                    = &Actor::IsAttackState;
}

void Actor::Begin_Alarm(void)
{
    DoForceActivate();

    m_csMood     = STRING_ALERT;
    m_csIdleMood = STRING_NERVOUS;

    if (!m_AlarmNode) {
        State_Alarm_StartThread();
        return;
    }

    SetPath(m_AlarmNode, "Actor::Begin_Alarm", 0);

    if (PathExists()) {
        TransitionState(ACTOR_STATE_ALARM_MOVE, 0);
    } else {
        TransitionState(ACTOR_STATE_ALARM_IDLE, 0);
        parm.movefail = true;
    }
}

void Actor::End_Alarm(void)
{
    parm.movefail = true;
}

void Actor::State_Alarm_StartThread(void)
{
    SetLeashHome(m_AlarmNode ? m_AlarmNode->origin : origin);
    // Added in 2.30.
    //  Execute the pre-alarm thread
    m_PreAlarmThread.Execute(this);

    TransitionState(ACTOR_STATE_ALARM_IDLE, 0);
    m_AlarmThread.Execute(this);
}

void Actor::State_Alarm_Move(void)
{
    if (!PathExists()) {
        TransitionState(ACTOR_STATE_ALARM_IDLE, 0);
        parm.movefail = true;

        Anim_Aim();
        AimAtTargetPos();
    } else if (PathComplete()) {
        Anim_Aim();
        AimAtTargetPos();
        State_Alarm_StartThread();
    } else {
        Anim_RunToAlarm(ANIM_MODE_PATH);
        FaceMotion();
    }
}

void Actor::State_Alarm_Idle(void)
{
    Anim_Aim();
    AimAtTargetPos();
    SetThink(THINKSTATE_ATTACK, THINK_TURRET);
}

void Actor::Think_Alarm(void)
{
    if (!RequireThink()) {
        return;
    }

    parm.movefail = false;

    UpdateEyeOrigin();
    NoPoint();

    switch (m_State) {
    case ACTOR_STATE_ALARM_IDLE:
        m_pszDebugState = "idle";
        State_Alarm_Idle();
        break;
    case ACTOR_STATE_ALARM_MOVE:
        m_pszDebugState = "move";
        State_Alarm_Move();
        break;
    default:
        Com_Printf("Actor::Think_Alarm: invalid think state %i\n", m_State);
        break;
    }
}

void Actor::FinishedAnimation_Alarm(void)
{
    // not needed
    return;
}
