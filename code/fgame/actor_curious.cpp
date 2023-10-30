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

// actor_curious.cpp

#include "actor.h"

void Actor::InitCurious(GlobalFuncs_t *func)
{
    func->ThinkState                 = &Actor::Think_Curious;
    func->BeginState                 = &Actor::Begin_Curious;
    func->EndState                   = &Actor::End_Curious;
    func->ResumeState                = &Actor::Resume_Curious;
    func->SuspendState               = &Actor::Suspend_Curious;
    func->FinishedAnimation          = &Actor::FinishedAnimation_Cover;
    func->PassesTransitionConditions = &Actor::PassesTransitionConditions_Curious;
    func->IsState                    = &Actor::IsCuriousState;
}

void Actor::SetCuriousAnimHint(int iAnimHint)
{
    m_iCuriousAnimHint = iAnimHint;
}

void Actor::Begin_Curious(void)
{
    float  fDistSquared;
    Vector vDelta;

    DoForceActivate();
    m_csMood = STRING_CURIOUS;
    Anim_Emotion(EMOTION_CURIOUS);

    if (level.inttime > level.m_iCuriousVoiceTime + 3000) {
        if (m_iCuriousAnimHint == 5) {
            Anim_Say(STRING_ANIM_SAY_CURIOUS_SOUND_SCR, 8000, false);
            level.m_iCuriousVoiceTime = level.inttime;
        } else if (m_iCuriousAnimHint == 6) {
            Anim_Say(STRING_ANIM_SAY_CURIOUS_SIGHT_SCR, 8000, false);
            level.m_iCuriousVoiceTime = level.inttime;
        }
    }

    m_iCuriousTime = level.inttime;

    SetLeashHome(origin);
    if (m_bScriptGoalValid) {
        SetPath(m_vScriptGoal, NULL, 0, NULL, 0.0);
        ShortenPathToAvoidSquadMates();
        if (!PathExists()) {
            m_bScriptGoalValid = false;
        }
    }
    TransitionState(ACTOR_STATE_CURIOUS_BEGIN, 0);

    if (!m_bScriptGoalValid) {
        //check if last enemy pos is within leash area (distance to m_vHome is <= m_fLeash)
        vDelta       = m_vLastEnemyPos - m_vHome;
        fDistSquared = vDelta.lengthSquared();

        if (fDistSquared > m_fLeashSquared && (origin - m_vHome).lengthSquared() <= m_fLeashSquared) {
            //I'm inside leash area,
            //Try to go towards enemy as much as possible without leaving leash area.
            //vDest = vHome + U * leash
            //U = unit vector of vDelta.
            SetPath(m_vHome + sqrt(m_fLeashSquared / fDistSquared) * vDelta, NULL, 0, NULL, 0);
            ShortenPathToAvoidSquadMates();
        } else {
            //I'm outside leash area,
            //go to enemy, it doesn't matter.
            SetPath(m_vLastEnemyPos, NULL, 0, NULL, 0);
            ShortenPathToAvoidSquadMates();
        }

        if (m_iCuriousAnimHint <= 3 && m_iCuriousAnimHint >= 1) {
            StartAnimation(ANIM_MODE_NORMAL, STRING_ANIM_STANDFLINCH_SCR);
            TransitionState(ACTOR_STATE_CURIOUS_RUNNING);
            m_bLockThinkState = true;
        } else if (m_Enemy && m_PotentialEnemies.GetCurrentVisibility() < 0.1) {
            SetDesiredYawDest(m_vLastEnemyPos);
            StartAnimation(ANIM_MODE_NORMAL, STRING_ANIM_SURPRISE_SCR);
            TransitionState(ACTOR_STATE_CURIOUS_RUNNING);
            m_bLockThinkState = true;
        }
    }

    m_iNextWatchStepTime = level.inttime + (rand() & 0x1FF);
}

void Actor::End_Curious(void)
{
    m_iCuriousTime  = 0;
    m_iCuriousLevel = 0;
}

void Actor::Resume_Curious(void)
{
    Begin_Curious();
}

void Actor::Suspend_Curious(void)
{
    End_Curious();
}

void Actor::Think_Curious(void)
{
    if (!RequireThink()) {
        return;
    }

    UpdateEyeOrigin();
    NoPoint();
    UpdateEnemy(100);

    if (m_State == ACTOR_STATE_CURIOUS_RUNNING) {
        ContinueAnimation();
        CheckForThinkStateTransition();
        PostThink(true);
        return;
    }

    m_bLockThinkState = false;

    if (!PathExists() || PathComplete()) {
        ClearPath();
        Anim_Stand();
        LookAtCuriosity();
        TimeOutCurious();

        CheckForThinkStateTransition();
        PostThink(true);
        return;
    }

    if (m_Enemy && CanSeeEnemy(100)) {
        ClearPath();
        Anim_Stand();
        LookAtCuriosity();
        TimeOutCurious();

        CheckForThinkStateTransition();
        PostThink(true);
        return;
    }

    if (m_iCuriousLevel <= 5) {
        if (InFOV(m_vLastEnemyPos, m_fFov, m_fFovDot)) {
            Vector vEnd;

            vEnd = EyePosition() - m_vLastEnemyPos;
            vEnd.normalizefast();
            vEnd += m_vLastEnemyPos;

            if (G_SightTrace(
                    EyePosition(), vec_zero, vec_zero, vEnd, this, NULL, MASK_CANSEE, qfalse, "Actor::Think_Curious"
                )) {
                ClearPath();
                Anim_Stand();
                LookAtCuriosity();
                TimeOutCurious();

                CheckForThinkStateTransition();
                PostThink(true);
                return;
            }
        }
    } else if ((PathGoal() - origin).lengthSquared() < Square(48)) {
        ClearPath();
        Anim_Stand();
        LookAtCuriosity();
        TimeOutCurious();

        CheckForThinkStateTransition();
        PostThink(true);
        return;
    }

    if ((m_bScriptGoalValid || CanMovePathWithLeash()) && MoveOnPathWithSquad()) {
        if (PatrolNextNodeExists()) {
            DesiredAnimation(ANIM_MODE_PATH, GetRunAnim());
        } else {
            DesiredAnimation(ANIM_MODE_PATH_GOAL, GetRunAnim());
        }

        FaceMotion();
    } else {
        Anim_Stand();
    }

    if (level.inttime >= m_iNextWatchStepTime && velocity.lengthSquared() >= Square(2)) {
        SetDesiredLookDir(velocity);
        m_iNextWatchStepTime = level.inttime + (rand() & 0x1FF) + 500;
    } else {
        LookAtCuriosity();
    }

    CheckForThinkStateTransition();
    PostThink(true);
}

void Actor::LookAtCuriosity(void)
{
    unsigned int iSeed;
    float        fLookScale;
    vec3_t       vAngles;

    if (m_Enemy && !EnemyIsDisguised()) {
        SetDesiredLookDir(m_Enemy->origin - origin);
        fLookScale = 0.25;
    } else {
        Vector v;

        v = m_vLastEnemyPos - origin;
        if (v[0] < 15 && v[0] > -15 && v[1] < 15 && v[1] > -15) {
            SetDesiredLookAnglesRelative(vec_zero);
        } else {
            SetDesiredLookDir(v);
            if (velocity.x || velocity.y) {
                FaceMotion();
            } else {
                SetDesiredYawDir(v);
            }
        }
        fLookScale = 1;
    }

    iSeed = 0x19660D * (m_iCuriousTime - 0x2EB71B09 * ((level.inttime - m_iCuriousTime) & 0xFFFFFF00)) + 0x3C6EF35F;

    vAngles[0] = m_DesiredLookAngles[0] + (float)iSeed / 0x4444444 * fLookScale;
    vAngles[1] = m_DesiredLookAngles[1] + (float)iSeed / 0x2222222 * fLookScale;
    vAngles[2] = m_DesiredLookAngles[2];
    // clamp
    vAngles[0] = Q_clamp_float(vAngles[0], -90, 90);

    SetDesiredLookAnglesRelative(vAngles);
}

void Actor::TimeOutCurious(void)
{
    if (m_Enemy && !EnemyIsDisguised()) {
        return;
    }

    if (level.inttime <= m_iCuriousTime + 500) {
        return;
    }

    SetThinkState(THINKSTATE_IDLE, THINKLEVEL_IDLE);
    m_iCuriousTime = 0;
}

void Actor::FinishedAnimation_Curious(void)
{
    if (m_State == ACTOR_STATE_CURIOUS_RUNNING) {
        TransitionState(ACTOR_STATE_CURIOUS_BEGIN, 0);
    }
}
