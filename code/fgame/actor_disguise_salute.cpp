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

// actor_disguise_salute.cpp

#include "actor.h"

void Actor::InitDisguiseSalute(GlobalFuncs_t *func)
{
    func->ThinkState                 = &Actor::Think_DisguiseSalute;
    func->BeginState                 = &Actor::Begin_DisguiseSalute;
    func->EndState                   = &Actor::End_DisguiseSalute;
    func->ResumeState                = &Actor::Resume_DisguiseSalute;
    func->SuspendState               = &Actor::Suspend_DisguiseSalute;
    func->PassesTransitionConditions = &Actor::PassesTransitionConditions_Disguise;
    func->IsState                    = &Actor::IsDisguiseState;
}

void Actor::Begin_DisguiseSalute(void)
{
    vec2_t vDelta;

    Com_Printf("Saluting guy....\n");

    m_csMood = STRING_BORED;
    assert(m_Enemy);

    if (m_Enemy) {
        if ((EnemyIsDisguised() || m_Enemy->IsSubclassOfActor()) && !level.m_bAlarm) {
            VectorSub2D(m_Enemy->origin, origin, vDelta);

            if (vDelta[0] != 0 || vDelta[1] != 0) {
                SetDesiredYawDir(vDelta);
            }

            SetDesiredLookDir(m_Enemy->origin - origin);

            m_eNextAnimMode    = ANIM_MODE_NORMAL;
            m_csNextAnimString = STRING_ANIM_DISGUISE_SALUTE_SCR;
            m_bNextForceStart  = false;

            m_iEnemyShowPapersTime = m_Enemy->m_ShowPapersTime;
            TransitionState(1, 0);
        } else {
            SetThinkState(THINKSTATE_ATTACK, THINKLEVEL_IDLE);
        }
    } else {
        SetThinkState(THINKSTATE_IDLE, THINKLEVEL_IDLE);
    }
}

void Actor::End_DisguiseSalute(void)
{
    m_iNextDisguiseTime = level.inttime + m_iDisguisePeriod;
}

void Actor::Resume_DisguiseSalute(void)
{
    Begin_DisguiseSalute();
}

void Actor::Suspend_DisguiseSalute(void)
{
    End_DisguiseSalute();
}

void Actor::Think_DisguiseSalute(void)
{
    NoPoint();
    ContinueAnimation();
    UpdateEnemy(2000);

    assert(m_Enemy != NULL);

    if (m_Enemy) {
        if (!EnemyIsDisguised() && !m_Enemy->IsSubclassOfActor() && level.m_bAlarm) {
            {
                vec2_t facedir;
                facedir[0] = m_Enemy->origin[0] - origin[0];
                facedir[1] = m_Enemy->origin[1] - origin[1];
                if (facedir[0] != 0 || facedir[1] != 0) {
                    SetDesiredYawDir(facedir);
                }
            }
            SetDesiredLookDir(m_Enemy->origin - origin);

            PostThink(true);
        } else {
            SetThinkState(THINKSTATE_ATTACK, THINKLEVEL_IDLE);
        }
    } else {
        SetThinkState(THINKSTATE_IDLE, THINKLEVEL_IDLE);
    }
}

void Actor::FinishedAnimation_DisguiseSalute(void)
{
    SetThinkState(THINKSTATE_IDLE, THINKLEVEL_IDLE);
}
