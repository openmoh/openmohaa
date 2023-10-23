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
    TransitionState(ACTOR_STATE_BALCONY_ATTACK_FIND_ENEMY, 0);
}

void Actor::State_Balcony_PostShoot(void)
{
    if (m_Enemy) {
        TransitionState(ACTOR_STATE_BALCONY_ATTACK_TARGET, 0);
    }
}

void Actor::State_Balcony_FindEnemy(void)
{
    m_bHasDesiredLookAngles = false;
    Anim_Aim();

    if (CanSeeEnemy(200)) {
        TransitionState(ACTOR_STATE_BALCONY_ATTACK_TARGET, 0);
    }
}

void Actor::State_Balcony_Target(void)
{
    Anim_Aim();
    AimAtTargetPos();

    if (level.inttime > m_iStateTime + 1000) {
        if (CanSeeEnemy(0) && CanShootEnemy(0)) {
            TransitionState(ACTOR_STATE_BALCONY_ATTACK_SHOOT, 0);
        } else {
            ClearPath();
            TransitionState(ACTOR_STATE_BALCONY_ATTACK_FIND_ENEMY, 0);
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
        SetThinkState(THINKSTATE_IDLE, THINKLEVEL_IDLE);
        IdleThink();
        return;
    }

    NoPoint();

    switch (m_State) {
    case ACTOR_STATE_BALCONY_ATTACK_TARGET:
        m_pszDebugState = "target";
        State_Balcony_Target();
        break;
    case ACTOR_STATE_BALCONY_ATTACK_SHOOT:
        m_pszDebugState = "shoot";
        State_Balcony_Shoot();
        break;
    case ACTOR_STATE_BALCONY_ATTACK_FIND_ENEMY:
        m_pszDebugState = "findenemy";
        State_Balcony_FindEnemy();
        break;
    default:
        Com_Printf("Actor::Think_BalconyAttack: invalid think state %i\n", m_State);
        assert(0);
        break;
    }

    PostThink(true);
}

void Actor::FinishedAnimation_BalconyAttack(void)
{
    if (m_State == ACTOR_STATE_BALCONY_ATTACK_SHOOT) {
        State_Balcony_PostShoot();
    }
}

bool Actor::CalcFallPath(void)
{
    mmove_t mm;
    int currentPos = 0;
    int loop;
    vec3_t pos[200];
    float startDeltaTime;
    float nextTime;
    vec3_t vRelDelta;
    vec3_t vAbsDelta;
    float animTime;
    int anim;
    int i;
    float startTime;

    SetMoveInfo(&mm);

    mm.tracemask &= ~(CONTENTS_BODY | CONTENTS_UNKNOWN2 | CONTENTS_NOBOTCLIP | CONTENTS_BBOX);
    mm.desired_speed = 80;
    VectorCopy2D(orientation[0], mm.desired_dir);

    anim     = gi.Anim_NumForName(edict->tiki, "death_balcony_intro");
    animTime = gi.Anim_Time(edict->tiki, anim);
    startTime = 0.65f;

    for (;;) {
        MmoveSingle(&mm);
        VectorCopy(mm.origin, pos[currentPos]);

        currentPos++;
        if (currentPos >= ARRAY_LEN(pos)) {
            return false;
        }

        if (mm.hit_obstacle) {
            break;
        }

        if (!mm.groundPlane) {
            return false;
        }

        startTime -= level.frametime;
        if (startTime < 0) {
            return false;
        }
    }

    for (startDeltaTime = 0.65f; startDeltaTime < animTime; startDeltaTime = nextTime) {
        nextTime = startDeltaTime + level.frametime;
        if (nextTime >= animTime - 0.01f) {
            nextTime = animTime;
        }

        gi.Anim_DeltaOverTime(edict->tiki, anim, startDeltaTime, nextTime, vRelDelta);
        MatrixTransformVector(vRelDelta, orientation, vAbsDelta);

        VectorAdd(mm.origin, vAbsDelta, mm.origin);
        VectorCopy(mm.origin, pos[currentPos]);

        currentPos++;
        if (currentPos >= ARRAY_LEN(pos)) {
            return false;
        }
    }

    loop = currentPos;

    mm.desired_speed = 0;
    mm.groundPlane = NULL;
    mm.walking = false;
    VectorSet(mm.velocity, 0, 0, -171);

    do {
        MmoveSingle(&mm);
        VectorCopy(mm.origin, pos[currentPos]);

        currentPos++;
        if (currentPos >= ARRAY_LEN(pos)) {
            return false;
        }

        if (mm.hit_obstacle) {
            return false;
        }
    } while (!mm.groundPlane);

    if (origin[2] - pos[currentPos - 1][2] < m_fBalconyHeight) {
        return false;
    }

    m_pFallPath = (FallPath*)gi.Malloc(sizeof(FallPath) + sizeof(FallPath::pos) * (currentPos - 1));
    m_pFallPath->length = currentPos;
    m_pFallPath->currentPos = 0;
    m_pFallPath->startTime = startTime;
    m_pFallPath->loop = loop;

    for (i = 0; i < currentPos; i++) {
        VectorCopy(pos[i], m_pFallPath->pos[i]);
    }

    return true;
}

void Actor::Begin_BalconyKilled(void)
{
    ClearPath();
    ResetBoneControllers();

    PostEvent(EV_Actor_DeathEmbalm, 0.05f);

    if (CalcFallPath()) {
        TransitionState(ACTOR_STATE_BALCONY_KILLED_BEGIN, 0);
    } else {
        TransitionState(ACTOR_STATE_BALCONY_KILLED_NORMAL, 0);
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

    if (m_State == ACTOR_STATE_BALCONY_KILLED_END) {
        m_pszDebugState = "";
        return;
    }

    NoPoint();
    ForwardLook();

    switch (m_State) {
    case ACTOR_STATE_BALCONY_KILLED_BEGIN:
        m_bNextForceStart = true;
        m_eNextAnimMode = ANIM_MODE_FALLING_PATH;
        m_pszDebugState = "begin";
        m_csNextAnimString = STRING_ANIM_NO_KILLED_SCR;

        animnum = gi.Anim_NumForName(edict->tiki, "death_balcony_intro");

        ChangeMotionAnim();

        m_bMotionAnimSet = true;
        m_iMotionSlot = GetMotionSlot(0);
        m_weightType[m_iMotionSlot] = ANIM_WEIGHT_MOTION;
        m_weightCrossBlend[m_iMotionSlot] = 0.0;
        m_weightBase[m_iMotionSlot] = 1.0;

        NewAnim(animnum, m_iMotionSlot);
        SetTime(m_iMotionSlot, m_pFallPath->startTime);
        UpdateNormalAnimSlot(m_iMotionSlot);
        TransitionState(ACTOR_STATE_BALCONY_KILLED_INTRO, 0);
        break;
    case ACTOR_STATE_BALCONY_KILLED_INTRO:
        m_bNextForceStart = false;
        m_pszDebugState = "intro";
        m_eNextAnimMode = ANIM_MODE_FALLING_PATH;
        m_csNextAnimString = STRING_ANIM_NO_KILLED_SCR;
        break;
    case ACTOR_STATE_BALCONY_KILLED_LOOP:
        m_pszDebugState = "loop";
        Anim_FullBody(STRING_DEATH_BALCONY_LOOP, 7);
        break;
    case ACTOR_STATE_BALCONY_KILLED_LOOP_END:
        TransitionState(804, 0);
        StopAllAnimating();
    case ACTOR_STATE_BALCONY_KILLED_OUTTRO:
        m_pszDebugState = "outtro";
        Anim_FullBody(STRING_DEATH_BALCONY_OUTTRO, 1);
        break;
    case ACTOR_STATE_BALCONY_KILLED_NORMAL:
        m_pszDebugState = "normal";
        Anim_Killed();
        break;
    default:
        Com_Printf("Actor::Think_BalconyKilled: invalid think state %i\n", m_State);
        assert(0);
    }

    PostThink(false);

    if (m_State >= ACTOR_STATE_BALCONY_KILLED_BEGIN) {
        if (m_State == ACTOR_STATE_BALCONY_KILLED_INTRO) {
            if (m_pFallPath->currentPos >= m_pFallPath->length) {
                TransitionState(ACTOR_STATE_BALCONY_KILLED_LOOP_END, 0);
            }
            else if (m_pFallPath->currentPos >= m_pFallPath->loop) {
                TransitionState(ACTOR_STATE_BALCONY_KILLED_LOOP, 0);
            }
        } else if (m_State == ACTOR_STATE_BALCONY_KILLED_LOOP) {
            if (m_pFallPath->currentPos >= m_pFallPath->length) {
                TransitionState(ACTOR_STATE_BALCONY_KILLED_LOOP_END, 0);
            }
        }
    }
}

void Actor::FinishedAnimation_BalconyKilled(void)
{
    switch (m_State) {
    case ACTOR_STATE_BALCONY_KILLED_OUTTRO:
    case ACTOR_STATE_BALCONY_KILLED_NORMAL:
        BecomeCorpse();
        TransitionState(ACTOR_STATE_BALCONY_KILLED_END, 0);
        break;
    case ACTOR_STATE_BALCONY_KILLED_INTRO:
        TransitionState(ACTOR_STATE_BALCONY_KILLED_LOOP, 0);
        StopAllAnimating();
        break;
    }
}
