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

void Actor::InitDogIdle(GlobalFuncs_t *func)
{
    func->BeginState                 = &Actor::Begin_Dog;
    func->EndState                   = &Actor::End_Dog;
    func->ThinkState                 = &Actor::Think_Dog_Idle;
    func->RestartState               = NULL;
    func->PassesTransitionConditions = &Actor::PassesTransitionConditions_Idle;
    func->IsState                    = &Actor::IsDogState;
}

void Actor::InitDogAttack(GlobalFuncs_t *func)
{
    func->BeginState                 = &Actor::Begin_Dog;
    func->EndState                   = &Actor::End_Dog;
    func->ThinkState                 = &Actor::Think_Dog_Attack;
    func->RestartState               = NULL;
    func->PassesTransitionConditions = &Actor::PassesTransitionConditions_Attack;
    func->IsState                    = &Actor::IsDogState;
}

void Actor::InitDogCurious(GlobalFuncs_t *func)
{
    func->BeginState                 = &Actor::Begin_Dog;
    func->EndState                   = &Actor::End_Dog;
    func->ThinkState                 = &Actor::Think_Dog_Curious;
    func->RestartState               = NULL;
    func->PassesTransitionConditions = &Actor::PassesTransitionConditions_Curious;
    func->IsState                    = &Actor::IsDogState;
}

bool Actor::IsDogState(int state)
{
    return true;
}

void Actor::Begin_Dog(void)
{
    m_bIsAnimal = true;
}

void Actor::End_Dog(void)
{
    ;
}

void Actor::Think_Dog_Idle(void)
{
    if (!RequireThink()) {
        return;
    }

    UpdateEyeOrigin();
    m_pszDebugState = "Dog_Idle";

    NoPoint();
    ForwardLook();

    DesiredAnimation(ANIM_MODE_NORMAL, STRING_ANIM_DOG_IDLE_SCR);

    CheckForThinkStateTransition();
    PostThink(false);
}

void Actor::Think_Dog_Attack(void)
{
    if (!RequireThink()) {
        return;
    }

    UpdateEyeOrigin();
    m_pszDebugState = "Dog_Attack";

    if (!m_Enemy || (m_Enemy->flags & FL_NOTARGET)) {
        DesiredAnimation(ANIM_MODE_NORMAL, STRING_ANIM_DOG_CURIOUS_SCR);
        TransitionState(20);

        CheckForThinkStateTransition();
        PostThink(false);
        return;
    }

    SetPath(m_Enemy->origin, NULL, 0, NULL, 0.0);

    if (PathExists()) {
        vec2_t delta;
        VectorSub2D(m_Enemy->origin, origin, delta);

        if (VectorLength2DSquared(delta) >= 8000) {
            FaceMotion();
            DesiredAnimation(ANIM_MODE_PATH, STRING_ANIM_DOG_CHASE_SCR);
        } else {
            SetDesiredYawDest(m_Enemy->origin);
            SetDesiredLookDir(m_Enemy->origin - origin);
            DesiredAnimation(ANIM_MODE_NORMAL, STRING_ANIM_DOG_ATTACK_SCR);
        }
    } else {
        SetDesiredYawDest(m_Enemy->origin);
        SetDesiredLookDir(m_Enemy->origin - origin);
        DesiredAnimation(ANIM_MODE_NORMAL, STRING_ANIM_DOG_CURIOUS_SCR);
        TransitionState(20);
    }

    CheckForThinkStateTransition();
    PostThink(false);
}

void Actor::Think_Dog_Curious(void)
{
    if (!RequireThink()) {
        return;
    }

    UpdateEyeOrigin();
    m_pszDebugState = "Dog_Curious";

    if (m_Enemy && !(m_Enemy->flags & FL_NOTARGET)) {
        SetDesiredYawDest(m_Enemy->origin);
        SetDesiredLookDir(m_Enemy->origin - origin);
    }

    DesiredAnimation(ANIM_MODE_NORMAL, STRING_ANIM_DOG_CURIOUS_SCR);

    CheckForThinkStateTransition();
    PostThink(false);
}
