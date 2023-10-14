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

// actor_balcony.cpp

#include "actor.h"
#include "scriptexception.h"

void Actor::InitBalconyIdle(GlobalFuncs_t *func)
{
    func->ThinkState                 = &Actor::Think_Idle;
    func->PassesTransitionConditions = &Actor::PassesTransitionConditions_Idle;
    func->Pain                       = &Actor::Pain_Balcony;
    func->Killed                     = &Actor::Killed_Balcony;
    func->IsState                    = &Actor::IsIdleState;
}

void Actor::InitBalconyCurious(GlobalFuncs_t *func)
{
    func->ThinkState                 = &Actor::Think_Curious;
    func->BeginState                 = &Actor::Begin_Curious;
    func->EndState                   = &Actor::End_Curious;
    func->ResumeState                = &Actor::Resume_Curious;
    func->SuspendState               = &Actor::Suspend_Curious;
    func->FinishedAnimation          = &Actor::FinishedAnimation_Curious;
    func->PassesTransitionConditions = &Actor::PassesTransitionConditions_Curious;
    func->IsState                    = &Actor::IsCuriousState;
    func->Pain                       = &Actor::Pain_Balcony;
    func->Killed                     = &Actor::Killed_Balcony;
}

void Actor::InitBalconyAttack(GlobalFuncs_t *func)
{
    func->ThinkState                 = &Actor::Think_BalconyAttack;
    func->PassesTransitionConditions = &Actor::PassesTransitionConditions_Attack;
    func->BeginState                 = &Actor::Begin_BalconyAttack;
    func->FinishedAnimation          = &Actor::FinishedAnimation_BalconyAttack;
    func->Pain                       = &Actor::Pain_Balcony;
    func->Killed                     = &Actor::Killed_Balcony;
    func->IsState                    = &Actor::IsAttackState;
    func->PostShoot                  = &Actor::State_Balcony_PostShoot;
}

void Actor::InitBalconyDisguise(GlobalFuncs_t *func)
{
    func->IsState = &Actor::IsDisguiseState;
}

void Actor::InitBalconyGrenade(GlobalFuncs_t *func)
{
    func->IsState = &Actor::IsGrenadeState;
}

void Actor::InitBalconyPain(GlobalFuncs_t *func)
{
    func->BeginState        = &Actor::Begin_Pain;
    func->ThinkState        = &Actor::Think_Pain;
    func->FinishedAnimation = &Actor::FinishedAnimation_Pain;
    func->Pain              = &Actor::Pain_Balcony;
    func->Killed            = &Actor::Killed_Balcony;
    func->IsState           = &Actor::IsPainState;
}

void Actor::InitBalconyKilled(GlobalFuncs_t *func)
{
    func->BeginState        = &Actor::Begin_BalconyKilled;
    func->EndState          = &Actor::End_BalconyKilled;
    func->ThinkState        = &Actor::Think_BalconyKilled;
    func->FinishedAnimation = &Actor::FinishedAnimation_BalconyKilled;
    func->IsState           = &Actor::IsKilledState;
}

void Actor::Pain_Balcony(Event *ev)
{
    SetThink(THINKSTATE_PAIN, THINK_BALCONY_PAIN);
    HandlePain(ev);
}

void Actor::Killed_Balcony(Event *ev, bool bPlayDeathAnim)
{
    ClearStates();
    SetThink(THINKSTATE_KILLED, THINK_BALCONY_KILLED);
    HandleKilled(ev, true);

    if (!bPlayDeathAnim) {
        ScriptError("cannot do 'bedead' on balcony guys");
    }
}

void Actor::Begin_BalconyAttack(void)
{
    TransitionState(200, 0);
}

void Actor::State_Balcony_PostShoot(void)
{
    if (m_Enemy) {
        TransitionState(201, 0);
    }
}

void Actor::State_Balcony_FindEnemy(void)
{
    m_bHasDesiredLookAngles = false;
    Anim_Aim();

    if (CanSeeEnemy(200)) {
        TransitionState(201, 0);
    }
}

void Actor::State_Balcony_Target(void)
{
    Anim_Aim();
    AimAtTargetPos();

    if (level.inttime > m_iStateTime + 1000) {
        if (CanSeeEnemy(0) && CanShootEnemy(0)) {
            TransitionState(202, 0);
        } else {
            ClearPath();
            TransitionState(200, 0);
        }
    }
}

void Actor::State_Balcony_Shoot(void)
{
    Anim_Shoot();
    AimAtTargetPos();
}

void Actor::Think_BalconyAttack(void)
{
    if (!RequireThink()) {
        return;
    }

    UpdateEyeOrigin();
    UpdateEnemy(500);

    if (!m_Enemy) {
        SetThinkState(THINKSTATE_IDLE, THINKLEVEL_NORMAL);
        IdleThink();
        return;
    }

    NoPoint();

    if (m_State == 201) {
        m_pszDebugState = "target";
        State_Balcony_Target();
    } else if (m_State == 202) {
        m_pszDebugState = "shoot";
        State_Balcony_Shoot();
    } else if (m_State == 200) {
        m_pszDebugState = "findenemy";
        State_Balcony_FindEnemy();
    } else {
        Com_Printf("Actor::Think_BalconyAttack: invalid think state %i\n", m_State);
        assert(0);
    }

    PostThink(true);
}

void Actor::FinishedAnimation_BalconyAttack(void)
{
    if (m_State == 202) {
        State_Balcony_PostShoot();
    }
}

bool Actor::CalcFallPath(void)
{
    float   startTime, animTime, startDeltaTime, nextTime;
    vec3_t  vAbsDelta, vRelDelta, pos[200];
    int     anim, loop, /*currentPos,*/ i;
    mmove_t mm;

    SetMoveInfo(&mm);

    mm.desired_speed = 80;
    mm.tracemask &= 0xFDFFF4FF;

    VectorCopy2D(orientation[0], mm.desired_dir);

    anim     = gi.Anim_NumForName(edict->tiki, "death_balcony_intro");
    animTime = gi.Anim_Time(edict->tiki, anim);

    startTime = 0.65F;

    i = 0;
    while (true) {
        MmoveSingle(&mm);

        i++;
        VectorCopy(mm.origin, pos[i]);

        if (i >= 200) {
            break;
        }

        if (mm.hit_obstacle) {
            for (float j = 0.65f; j < animTime; j = nextTime) {
                nextTime = j + level.frametime;
                if (nextTime >= animTime - 0.01f) {
                    nextTime = animTime;
                }
                startDeltaTime = j;
                gi.Anim_DeltaOverTime(edict->tiki, anim, startDeltaTime, nextTime, vAbsDelta);
                MatrixTransformVector(vAbsDelta, orientation, vRelDelta);

                i++;

                VectorAdd(vRelDelta, mm.origin, mm.origin);
                VectorCopy(mm.origin, pos[i]);

                if (i >= 200) {
                    return false;
                }
            }
            mm.desired_speed = 0;
            mm.groundPlane   = qfalse;
            mm.walking       = qfalse;
            mm.velocity[0]   = 0;
            mm.velocity[1]   = 0;
            mm.velocity[2]   = -171;

            loop = i;

            while (true) {
                MmoveSingle(&mm);

                i++;
                VectorCopy(mm.origin, pos[i]);

                if (i >= 200) {
                    break;
                }

                if (mm.hit_obstacle) {
                    return false;
                }

                if (mm.groundPlane) {
                    if (m_fBalconyHeight > origin[2] - pos[i][2]) {
                        return false;
                    }

                    m_pFallPath =
                        (FallPath *)gi.Malloc((sizeof(FallPath::pos)) * i + (sizeof(FallPath) - sizeof(FallPath::pos)));

                    m_pFallPath->length = i;

                    m_pFallPath->currentPos = 0;
                    m_pFallPath->startTime  = startTime;
                    m_pFallPath->loop       = loop;

                    if (i > 0) {
                        for (int j = i; j; j--) {
                            VectorCopy(pos[j], m_pFallPath->pos[j]);
                        }
                    }
                    return true;
                }
            }

            return false;
        }

        if (mm.groundPlane) {
            startTime -= level.frametime;
            if (startTime >= 0) {
                continue;
            }
        }
        return false;
    }
    return false;
}

void Actor::Begin_BalconyKilled(void)
{
    ClearPath();
    ResetBoneControllers();

    PostEvent(EV_Actor_DeathEmbalm, 0.05f);

    if (CalcFallPath()) {
        TransitionState(800, 0);
    } else {
        TransitionState(806, 0);
    }
}

void Actor::End_BalconyKilled(void)
{
    if (m_pFallPath) {
        gi.Free(m_pFallPath);
        m_pFallPath = NULL;
    }
}

void Actor::Think_BalconyKilled(void)
{
    int animnum;

    Unregister(STRING_ANIMDONE);

    if (m_State == 805) {
        m_pszDebugState = "end";
    } else {
        NoPoint();
        m_bHasDesiredLookAngles = false;
        StopTurning();

        switch (m_State) {
        case 800:
            m_bNextForceStart  = true;
            m_eNextAnimMode    = ANIM_MODE_FALLING_PATH;
            m_pszDebugState    = "begin";
            m_csNextAnimString = STRING_ANIM_NO_KILLED_SCR;

            animnum = gi.Anim_NumForName(edict->tiki, "death_balcony_intro");

            ChangeMotionAnim();

            m_bMotionAnimSet                  = true;
            m_iMotionSlot                     = GetMotionSlot(0);
            m_weightType[m_iMotionSlot]       = 1;
            m_weightCrossBlend[m_iMotionSlot] = 0.0;
            m_weightBase[m_iMotionSlot]       = 1.0;

            NewAnim(animnum, m_iMotionSlot);
            SetTime(m_iMotionSlot, m_pFallPath->startTime);
            UpdateNormalAnimSlot(m_iMotionSlot);
            TransitionState(801, 0);
            break;
        case 801:
            m_bNextForceStart  = false;
            m_pszDebugState    = "intro";
            m_eNextAnimMode    = ANIM_MODE_FALLING_PATH;
            m_csNextAnimString = STRING_ANIM_NO_KILLED_SCR;
            break;
        case 802:
            m_pszDebugState = "loop";
            Anim_FullBody(STRING_DEATH_BALCONY_LOOP, 7);
            break;
        case 803:
            TransitionState(804, 0);
            StopAllAnimating();
        case 804:
            m_pszDebugState = "outtro";
            Anim_FullBody(STRING_DEATH_BALCONY_OUTTRO, 1);
            break;
        case 806:
            m_pszDebugState = "normal";
            Anim_Killed();
            break;
        default:
            Com_Printf("Actor::Think_BalconyKilled: invalid think state %i\n", m_State);
            assert(0);
        }

        PostThink(false);

        if (m_State >= 800) {
            if (m_State == 801) {
                if (m_pFallPath->currentPos >= m_pFallPath->length) {
                    TransitionState(803, 0);
                } else if (m_pFallPath->currentPos >= m_pFallPath->loop) {
                    TransitionState(802, 0);
                }
            } else if (m_State == 802) {
                if (m_pFallPath->currentPos >= m_pFallPath->length) {
                    TransitionState(803, 0);
                }
            }
        }
    }
}

void Actor::FinishedAnimation_BalconyKilled(void)
{
    if (m_State == 804 || m_State == 806) {
        BecomeCorpse();
        TransitionState(805, 0);
    } else if (m_State == 801) {
        TransitionState(802, 0);
        StopAllAnimating();
    }
}
