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
    Vector vDelta;
    //float fDistSquared;

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
        SetPath(m_vScriptGoal, "", 0, NULL, 0.0);
        ShortenPathToAvoidSquadMates();
        if (!PathExists()) {
            m_bScriptGoalValid = false;
        }
    }
    TransitionState(1100, 0);

    if (!m_bScriptGoalValid) {
        //check if last enemy pos is within leash area (distance to m_vHome is <= m_fLeash)
        vDelta = m_vLastEnemyPos - m_vHome;
        if (vDelta.lengthSquared() <= m_fLeashSquared) {
            //it's within leash area, go check it.
            SetPath(m_vLastEnemyPos, NULL, 0, NULL, 0.0);
        } else {
            //it's NOT within leash area,

            //am I within leash area ?
            if ((origin - m_vHome).lengthSquared() <= m_fLeashSquared) {
                //I'm inside leash area,
                //Try to go towards enemy as much as possible without leaving leash area.
                //vDest = vHome + U * leash
                //U = unit vector of vDelta.
                SetPath(vDelta * sqrt(m_fLeashSquared / vDelta.lengthSquared()) + m_vHome, NULL, 0, NULL, 0.0);
            } else {
                //I'm outside leash area,
                //go to enemy, it doesn't matter.
                SetPath(m_vLastEnemyPos, NULL, 0, NULL, 0.0);
            }
        }

        ShortenPathToAvoidSquadMates();
        if (m_iCuriousAnimHint <= 3) {
            m_eNextAnimMode    = ANIM_MODE_NORMAL;
            m_csNextAnimString = STRING_ANIM_STANDFLINCH_SCR;

            m_bNextForceStart = true;
            m_bLockThinkState = true;
            TransitionState(1101, 0);
        } else if (m_Enemy && m_PotentialEnemies.GetCurrentVisibility() < 0.1) {
            vec2_t vDel;
            VectorSub2D(m_vLastEnemyPos, origin, vDel);
            if (vDel[0] != 0 || vDel[1] != 0) {
                SetDesiredYawDir(vDel);
            }
            m_eNextAnimMode    = ANIM_MODE_NORMAL;
            m_csNextAnimString = STRING_ANIM_SURPRISE_SCR;

            m_bNextForceStart = true;
            m_bLockThinkState = true;
            TransitionState(1101, 0);
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
    //horrible function!
    if (RequireThink()) {
        UpdateEyeOrigin();
        NoPoint();
        UpdateEnemy(100);

        if (m_State == 1101) {
            ContinueAnimation();
            //LABEL_16:
            CheckForThinkStateTransition();
            glbs.Printf("Think_Curious CheckForThinkStateTransition 1\n");
            PostThink(true);
            return;
        }

        m_bLockThinkState = false;

        if (!PathExists() || PathComplete()) {
            //v1 = &this->baseSimpleActor.baseSentient.m_Enemy;
            //goto LABEL_6;

            ClearPath();
            Anim_Stand();
            LookAtCuriosity();

            if (!m_Enemy || EnemyIsDisguised()) {
                if (level.inttime > m_iCuriousTime + 500) {
                    SetThinkState(THINKSTATE_IDLE, THINKLEVEL_IDLE);
                    m_iCuriousTime = 0;
                }
            }
            CheckForThinkStateTransition();
            glbs.Printf("Think_Curious CheckForThinkStateTransition 2\n");
            PostThink(true);
            return;
        }

        if (!m_Enemy || CanSeeEnemy(100)) {
            ClearPath();
            Anim_Stand();
            LookAtCuriosity();
            if (!m_Enemy || EnemyIsDisguised()) {
                if (level.inttime > m_iCuriousTime + 500) {
                    SetThinkState(THINKSTATE_IDLE, THINKLEVEL_IDLE);
                    m_iCuriousTime = 0;
                }
            }
            CheckForThinkStateTransition();
            glbs.Printf("Think_Curious CheckForThinkStateTransition 3\n");
            PostThink(true);
            return;
        } else {
            if (m_iCuriousLevel <= 5) {
                if (!InFOV(m_vLastEnemyPos, m_fFov, m_fFovDot)) {
                    if ((m_bScriptGoalValid || CanMovePathWithLeash()) && MoveOnPathWithSquad()) {
                        if (PatrolNextNodeExists()) {
                            m_eNextAnimMode = ANIM_MODE_PATH;
                        } else {
                            //v11 = SimpleActor::GetRunAnim(this);
                            m_eNextAnimMode = ANIM_MODE_PATH_GOAL;
                        }
                        m_csNextAnimString = GetRunAnim();
                        m_bNextForceStart  = false;

                        FaceMotion();
                    } else {
                        Anim_Stand();
                    }

                    if (level.inttime >= m_iNextWatchStepTime && velocity.lengthSquared() >= 4.0) {
                        SetDesiredLookDir(velocity);

                        m_iNextWatchStepTime = level.inttime + (rand() & 0x1FF) + 500;
                    } else {
                        LookAtCuriosity();
                    }
                    CheckForThinkStateTransition();
                    glbs.Printf("Think_Curious CheckForThinkStateTransition 4\n");
                    PostThink(true);
                    return;
                } else {
                    Vector vEnd = EyePosition() - m_vLastEnemyPos;
                    VectorNormalizeFast(vEnd);

                    vEnd += m_vLastEnemyPos;
                    if (!G_SightTrace(
                            EyePosition(),
                            vec_zero,
                            vec_zero,
                            vEnd,
                            this,
                            NULL,
                            33819417,
                            qfalse,
                            "Actor::Think_Curious"
                        )) {
                        if ((m_bScriptGoalValid || CanMovePathWithLeash()) && MoveOnPathWithSquad()) {
                            if (PatrolNextNodeExists()) {
                                m_eNextAnimMode = ANIM_MODE_PATH;
                            } else {
                                //v11 = SimpleActor::GetRunAnim(this);
                                m_eNextAnimMode = ANIM_MODE_PATH_GOAL;
                            }
                            m_csNextAnimString = GetRunAnim();
                            m_bNextForceStart  = false;

                            FaceMotion();
                        } else {
                            Anim_Stand();
                        }

                        if (level.inttime >= m_iNextWatchStepTime && velocity.lengthSquared() >= 4.0) {
                            SetDesiredLookDir(velocity);

                            m_iNextWatchStepTime = level.inttime + (rand() & 0x1FF) + 500;
                        } else {
                            LookAtCuriosity();
                        }
                        CheckForThinkStateTransition();
                        glbs.Printf("Think_Curious CheckForThinkStateTransition 5\n");
                        PostThink(true);
                        return;
                    } else {
                        ClearPath();
                        Anim_Stand();
                        LookAtCuriosity();
                        if (!m_Enemy || EnemyIsDisguised()) {
                            if (level.inttime > m_iCuriousTime + 500) {
                                SetThinkState(THINKSTATE_IDLE, THINKLEVEL_IDLE);
                                m_iCuriousTime = 0;
                            }
                        }
                        CheckForThinkStateTransition();
                        glbs.Printf("Think_Curious CheckForThinkStateTransition 6\n");
                        PostThink(true);
                        return;
                    }
                }
            } else {
                if ((PathGoal() - origin).lengthSquared() >= 2304) {
                    if ((m_bScriptGoalValid || CanMovePathWithLeash()) && MoveOnPathWithSquad()) {
                        if (PatrolNextNodeExists()) {
                            m_eNextAnimMode = ANIM_MODE_PATH;
                        } else {
                            //v11 = SimpleActor::GetRunAnim(this);
                            m_eNextAnimMode = ANIM_MODE_PATH_GOAL;
                        }
                        m_csNextAnimString = GetRunAnim();
                        m_bNextForceStart  = false;

                        FaceMotion();
                    } else {
                        Anim_Stand();
                    }

                    if (level.inttime >= m_iNextWatchStepTime && velocity.lengthSquared() >= 4.0) {
                        SetDesiredLookDir(velocity);

                        m_iNextWatchStepTime = level.inttime + (rand() & 0x1FF) + 500;
                    } else {
                        LookAtCuriosity();
                    }
                    CheckForThinkStateTransition();
                    glbs.Printf("Think_Curious CheckForThinkStateTransition 7\n");
                    PostThink(true);
                    return;
                }
            }
            ClearPath();
            Anim_Stand();
            LookAtCuriosity();
            if (!m_Enemy || EnemyIsDisguised()) {
                if (level.inttime > m_iCuriousTime + 500) {
                    SetThinkState(THINKSTATE_IDLE, THINKLEVEL_IDLE);
                    m_iCuriousTime = 0;
                }
            }
            CheckForThinkStateTransition();
            glbs.Printf("Think_Curious CheckForThinkStateTransition 8\n");
            PostThink(true);
            return;
        }
    }
}

void Actor::LookAtCuriosity(void)
{
    Vector       vAngles;
    float        fLookScale;
    unsigned int iSeed;
    if (m_Enemy && EnemyIsDisguised()) {
        SetDesiredLookDir(m_Enemy->origin - origin);
        fLookScale = 0.25;
    } else {
        vAngles = m_vLastEnemyPos - origin;
        if (vAngles.x < 15.0 && vAngles.x > -15.0 && vAngles.y < 15.0 && vAngles.y > -15.0) {
            SetDesiredLookAnglesRelative(vec_zero);
        } else {
            SetDesiredLookDir(vAngles);
            if (velocity.x == 0.0 && velocity.y == 0.0) {
                SetDesiredYawDir(vAngles);
            } else {
                FaceMotion();
            }
        }
        fLookScale = 1;
    }

    //FIXME: wth is this ! @_@
    iSeed = 1664525 * (-783751945 * ((level.inttime - m_iCuriousTime) & 0xFFFFFF00) + m_iCuriousTime);

    vAngles.x =
        fLookScale * ((1664525 * (iSeed + 1013904223) + 1013904223) * 0.000000013969839) + m_DesiredLookAngles[0];
    vAngles.y = (iSeed + 1013904223) * 0.000000027939677 * fLookScale + m_DesiredLookAngles[1];
    vAngles.z = m_DesiredLookAngles[2];

    if (vAngles.x >= -90.0) {
        if (vAngles.x > 90.0) {
            vAngles.x = 90.0;
        }
    } else {
        vAngles.x = -90.0;
    }

    SetDesiredLookAnglesRelative(vAngles);
}

void Actor::TimeOutCurious(void)
{
    //FIXME: macros
    if (!m_Enemy || EnemyIsDisguised()) {
        if (level.inttime > m_iCuriousTime + 500) {
            SetThinkState(THINKSTATE_IDLE, THINKLEVEL_IDLE);
            m_iCuriousTime = 0;
        }
    }
}

void Actor::FinishedAnimation_Curious(void)
{
    FinishedAnimation_AnimCurious();
}
