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

// actor_grenade.cpp

#include "actor.h"
#include "weaputils.h"

bool Actor::Grenade_Acquire(eGrenadeState eNextState, const_str csReturnAnim)
{
    vec3_t vDest;
    bool   bRetVal = false;

    if (m_bGrenadeBounced) {
        m_bGrenadeBounced = false;
        bRetVal           = true;

        VectorSubtract(origin, m_vGrenadePos, vDest);
        VectorNormalizeFast(vDest);

        VectorMA(m_vGrenadePos, 16, vDest, vDest);

        SetPath(vDest, NULL, 0, NULL, 0);
    }

    if (!PathExists()) {
        m_bGrenadeBounced = true;
        m_eGrenadeState   = AI_GRENSTATE_FLEE;
        Grenade_Flee();
        return false;
    }

    if (PathComplete()) {
        ForwardLook();

        if (VectorLength2DSquared(m_pGrenade->velocity) >= Square(32)) {
            Anim_Stand();
        } else {
            m_pGrenade->velocity = vec_zero;
            m_eGrenadeState      = eNextState;
            DesiredAnimation(ANIM_MODE_NORMAL, csReturnAnim);
        }
    } else {
        Anim_RunToCasual(ANIM_MODE_PATH_GOAL);
        VectorSub2D(origin, m_vGrenadePos, vDest);

        if (VectorLength2DSquared(vDest) > Square(32)) {
            FaceMotion();
        } else {
            SetDesiredYawDest(m_vGrenadePos);
        }
    }

    return bRetVal;
}

void Actor::InitGrenade(GlobalFuncs_t *func)
{
    func->ThinkState                 = &Actor::Think_Grenade;
    func->BeginState                 = &Actor::Begin_Grenade;
    func->EndState                   = &Actor::End_Grenade;
    func->ResumeState                = &Actor::Resume_Grenade;
    func->SuspendState               = &Actor::End_Grenade;
    func->PassesTransitionConditions = &Actor::PassesTransitionConditions_Grenade;
    func->FinishedAnimation          = &Actor::FinishedAnimation_Grenade;
    func->IsState                    = &Actor::IsGrenadeState;
}

void Actor::Grenade_Flee(void)
{
    vec2_t    vDirPreferred;
    vec2_t    vDirAway;
    float     fAngle, fSinAngle, fCosAngle;
    float     fMinCloseDistSquared;
    vec2_t    grenade_offset;
    float     origin_ratio;
    PathInfo *current_node;

    if (m_bGrenadeBounced) {
        fAngle    = (rand() - 0x3FFFFFFF) / 683565275.f;
        fSinAngle = sin(fAngle);
        fCosAngle = cos(fAngle);

        VectorSub2D(origin, m_vGrenadePos, vDirAway);

        vDirPreferred[0] = vDirAway[0] * fCosAngle - vDirAway[1] * fSinAngle;
        vDirPreferred[1] = vDirAway[0] * fSinAngle + vDirAway[1] * fCosAngle;

        FindPathAway(m_vGrenadePos, vDirPreferred, 512);

        if (PathExists() && !PathComplete()) {
            fMinCloseDistSquared = VectorLength2DSquared(vDirAway) * 0.64f;

            if (fMinCloseDistSquared < Square(64)) {
                fMinCloseDistSquared = 0;
            }

            for (current_node = CurrentPathNode(); current_node >= LastPathNode(); current_node--) {
                VectorSub2D(m_vGrenadePos, current_node->point, grenade_offset);

                origin_ratio = DotProduct2D(grenade_offset, current_node->dir);
                if (origin_ratio <= 0) {
                    continue;
                }

                if (origin_ratio > current_node->dist) {
                    continue;
                }

                if (Square(CrossProduct2D(grenade_offset, current_node->dir)) < fMinCloseDistSquared) {
                    ClearPath();
                    break;
                }
            }
        }

        m_bGrenadeBounced = false;
    }

    if (PathExists() && !PathComplete()) {
        Sentient *pOwner = NULL;

        if (m_pGrenade && m_pGrenade->IsSubclassOfProjectile()) {
            pOwner = static_cast<Projectile *>(m_pGrenade.Pointer())->GetOwner();
        }

        if (pOwner && pOwner->m_Team == m_Team) {
            Anim_RunTo(ANIM_MODE_PATH_GOAL);
        } else {
            Anim_RunToFlee(ANIM_MODE_PATH_GOAL);
        }

        FaceMotion();
    }
    else if ((origin - m_vGrenadePos).lengthXYSquared() >= 100352
        || !G_SightTrace(
            centroid, vec_zero, vec_zero, m_vGrenadePos, this, m_pGrenade, MASK_CANSEE, 0, "Actor::Grenade_Flee"
        )) {
        m_eGrenadeState = AI_GRENSTATE_FLEE_SUCCESS;
        Anim_Attack();
        AimAtTargetPos();
    } else {
        m_eGrenadeState = AI_GRENSTATE_FLEE_FAIL;
        ForwardLook();
        Anim_Cower();
    }
}

void Actor::Grenade_ThrowAcquire(void)
{
    if (Grenade_Acquire(AI_GRENSTATE_THROW, STRING_ANIM_GRENADERETURN_SCR)) {
        return;
    }

    if (CanGetGrenadeFromAToB(m_vGrenadePos, m_vLastEnemyPos, true, &m_vGrenadeVel, &m_eGrenadeMode)) {
        return;
    }

    m_eGrenadeState   = AI_GRENSTATE_FLEE;
    m_bGrenadeBounced = true;
    Grenade_Flee();
}

void Actor::Grenade_Throw(void)
{
    ForwardLook();
    SetDesiredYawDir(m_vGrenadeVel);
    ContinueAnimation();
}

void Actor::Grenade_KickAcquire(void)
{
    Vector vDelta;

    if (Grenade_Acquire(AI_GRENSTATE_KICK, STRING_ANIM_GRENADEKICK_SCR)) {
        return;
    }

    vDelta   = m_vGrenadePos - origin;
    vDelta.z = 0;

    if (CanKickGrenade(m_vGrenadePos, m_vLastEnemyPos, vDelta, &m_vGrenadeVel)) {
        m_vKickDir   = m_vGrenadeVel;
        m_vKickDir.z = 0;
        m_vKickDir.normalizefast();
    } else {
        m_eGrenadeState   = AI_GRENSTATE_FLEE;
        m_bGrenadeBounced = true;
        Grenade_Flee();
    }
}

void Actor::Grenade_Kick(void)
{
    ForwardLook();
    ContinueAnimation();
}

void Actor::Grenade_MartyrAcquire(void)
{
    vec3_t vDest;

    if (m_bGrenadeBounced) {
        m_bGrenadeBounced = false;

        VectorSubtract(origin, m_vGrenadePos, vDest);
        VectorNormalizeFast(vDest);
        VectorMA(m_vGrenadePos, 88, vDest, vDest);

        SetPath(vDest, NULL, 0, NULL, 0.0);
    }

    if (!PathExists()) {
        m_bGrenadeBounced = true;
        m_eGrenadeState   = AI_GRENSTATE_FLEE;
        Grenade_Flee();
        return;
    }

    if (PathComplete()) {
        ForwardLook();
        m_pGrenade->velocity = vec_zero;

        if (m_pGrenade->velocity.lengthXYSquared() < Square(32)) {
            m_pGrenade->velocity = vec_zero;

            m_eGrenadeState = AI_GRENSTATE_MARTYR;
            m_iStateTime    = level.inttime;
            Grenade_Martyr();
        }
    } else {
        Anim_RunToCasual(ANIM_MODE_PATH_GOAL);
        SetPathGoalEndAnim(STRING_ANIM_GRENADEMARTYR_SCR);

        VectorSub2D(origin, m_vGrenadePos, vDest);
        if (VectorLength2DSquared(vDest) > Square(128)) {
            FaceMotion();
        } else {
            SetDesiredYawDest(m_vGrenadePos);
        }
    }
}

void Actor::Grenade_Martyr(void)
{
    if (m_pGrenade && level.inttime >= (m_iStateTime + 1000) - 0.5) {
        static_cast<Projectile *>(m_pGrenade.Pointer())->SetMartyr(entnum);
    }

    ForwardLook();
    ContinueAnimation();
}

void Actor::Grenade_Wait(void)
{
    if (rand() & 0xF) {
        Anim_Cower();
    } else {
        Anim_Stand();
        Grenade_NextThinkState();
    }
}

void Actor::Grenade_NextThinkState(void)
{
    if (!m_Enemy || (m_Enemy->flags & FL_NOTARGET)) {
        SetThinkState(THINKSTATE_IDLE, THINKLEVEL_IDLE);
        return;
    }

    SetThinkState(THINKSTATE_ATTACK, THINKLEVEL_IDLE);
}

void Actor::Grenade_EventAttach(Event *ev)
{
    if (!m_pGrenade) {
        return;
    }

    int              tagnum;
    Vector           vVel;
    eGrenadeTossMode eMode;

    tagnum = gi.Tag_NumForName(edict->tiki, "tag_weapon_right");
    if (tagnum >= 0) {
        m_pGrenade->attach(entnum, tagnum, qtrue, vec3_origin);
    }

    m_pGrenade->avelocity = vec3_origin;

    if (CanGetGrenadeFromAToB(origin, m_vLastEnemyPos, true, &vVel, &eMode)) {
        m_vGrenadeVel  = vVel;
        m_eGrenadeMode = eMode;
    }

    SetDesiredYawDir(m_vGrenadeVel);
}

void Actor::Grenade_EventDetach(Event *ev)
{
    if (!m_pGrenade) {
        return;
    }

    const_str csAnim;

    m_pGrenade->detach();
    csAnim = m_eGrenadeMode == AI_GREN_KICK ? STRING_ANIM_GRENADEKICK_SCR : STRING_ANIM_GRENADERETURN_SCR;

    m_pGrenade->setOrigin(GrenadeThrowPoint(origin, orientation[0], csAnim));
    m_pGrenade->velocity = m_vGrenadeVel;

    m_pGrenade->edict->r.ownerNum = edict->s.number;
    m_pGrenade->groundentity      = NULL;
}

void Actor::Begin_Grenade(void)
{
    DoForceActivate();
    m_csMood     = STRING_ALERT;
    m_csIdleMood = STRING_NERVOUS;

    if (!m_pGrenade) {
        if (m_Enemy && (m_Enemy->flags & FL_NOTARGET)) {
            SetThinkState(THINKSTATE_ATTACK, THINKLEVEL_IDLE);
        } else if (m_Team != TEAM_GERMAN) {
            SetThinkState(THINKSTATE_IDLE, THINKLEVEL_IDLE);
        } else {
            if (!IsTeamMate(static_cast<Sentient *>(G_GetEntity(0)))) {
                ForceAttackPlayer();
            }
        }

        return;
    }

    if (m_pGrenade->enemy || m_pGrenade->edict->r.ownerNum == entnum) {
        m_eGrenadeState = AI_GRENSTATE_FLEE;
        Grenade_Flee();
        return;
    }

    if (m_pGrenade) {
        vec3_t vDelta;
        float  fDistSquared;
        bool   bHasThrowTarget;

        bHasThrowTarget = true;
        if (!m_Enemy) {
            Sentient *pEnemy = static_cast<Sentient *>(G_GetEntity(m_pGrenade->edict->r.ownerNum));

            if (pEnemy && pEnemy->m_Team != m_Team) {
                SetEnemyPos(pEnemy->origin);
            } else {
                bHasThrowTarget = false;
            }
        }

        if (!bHasThrowTarget) {
            m_eGrenadeState = AI_GRENSTATE_FLEE;
            Grenade_Flee();
            return;
        }

        VectorSub2D(m_vGrenadePos, origin, vDelta);
        fDistSquared = VectorLength2DSquared(vDelta);

        if (fDistSquared >= Square(256)) {
            m_eGrenadeState = AI_GRENSTATE_FLEE;
            Grenade_Flee();
            return;
        }

        if (fDistSquared > 16384 && GrenadeWillHurtTeamAt(m_vGrenadePos)) {
            m_pGrenade->enemy = this;

            m_eGrenadeState = AI_GRENSTATE_MARTYR_ACQUIRE;
            Grenade_MartyrAcquire();
            return;
        }

        if (GrenadeWillHurtTeamAt(m_vLastEnemyPos)) {
            m_eGrenadeState = AI_GRENSTATE_FLEE;
            Grenade_Flee();
            return;
        }

        Vector vFace = vDelta;
        if (CanKickGrenade(m_vGrenadePos, m_vLastEnemyPos, vFace, &m_vGrenadeVel)) {
            m_eGrenadeMode = AI_GREN_KICK;

            m_vKickDir   = m_vGrenadeVel;
            m_vKickDir.z = 0;
            m_vKickDir.normalizefast();

            m_pGrenade->enemy = this;
            m_eGrenadeState   = AI_GRENSTATE_KICK_ACQUIRE;

            PostponeEvent(EV_Projectile_Explode, 0.25);
            Grenade_KickAcquire();
            return;
        }

        if (!CanGetGrenadeFromAToB(m_vGrenadePos, m_vLastEnemyPos, true, &m_vGrenadeVel, &m_eGrenadeMode)) {
            m_eGrenadeState = AI_GRENSTATE_FLEE;
            Grenade_Flee();
            return;
        }

        m_pGrenade->enemy = this;

        m_eGrenadeState = AI_GRENSTATE_THROW_ACQUIRE;
        PostponeEvent(EV_Projectile_Explode, 0.75);

        Grenade_ThrowAcquire();
    } else if (m_Enemy && !m_Enemy->IsSubclassOfActor()) {
        SetThinkState(THINKSTATE_ATTACK, THINKLEVEL_IDLE);
    } else if (m_Team == TEAM_AMERICAN) {
        SetThinkState(THINKSTATE_IDLE, THINKLEVEL_IDLE);
    } else {
        if (!IsTeamMate((Sentient *)G_GetEntity(0))) {
            ForceAttackPlayer();
        }
    }
}

void Actor::End_Grenade(void)
{
    m_pszDebugState = "";
}

void Actor::Resume_Grenade(void)
{
    if (m_pGrenade) {
        Begin_Grenade();
    } else {
        Grenade_NextThinkState();
    }
}

void Actor::FinishedAnimation_Grenade(void)
{
    switch (m_eGrenadeState) {
    case AI_GRENSTATE_FLEE:
    case AI_GRENSTATE_THROW_ACQUIRE:
    case AI_GRENSTATE_KICK_ACQUIRE:
    case AI_GRENSTATE_MARTYR_ACQUIRE:
    case AI_GRENSTATE_MARTYR:
    case AI_GRENSTATE_FLEE_SUCCESS:
    case AI_GRENSTATE_FLEE_FAIL:
        return;
    case AI_GRENSTATE_THROW:
    case AI_GRENSTATE_KICK:
        Grenade_NextThinkState();
        break;
    default:
        char assertStr[16317] = {0};
        strcpy(assertStr, "\"invalid grenade state in FinishedAnimation()\"\n\tMessage: ");
        Q_strcat(assertStr, sizeof(assertStr), DumpCallTrace("state = %i", m_eGrenadeState));
        assert(!assertStr);
        break;
    }
}

void Actor::Think_Grenade(void)
{
    if (m_bEnableEnemy) {
        UpdateEnemy(200);
    }

    m_pszDebugState = "";
    NoPoint();

    if (level.inttime - m_iFirstGrenadeTime > 8000) {
        Anim_Stand();
        Grenade_NextThinkState();
        PostThink(false);
        return;
    }

    if (m_pGrenade) {
        switch (m_eGrenadeState) {
        case AI_GRENSTATE_FLEE:
        case AI_GRENSTATE_FLEE_SUCCESS:
        case AI_GRENSTATE_FLEE_FAIL:
            m_pszDebugState = "RunAway";
            Grenade_Flee();
            break;
        case AI_GRENSTATE_THROW_ACQUIRE:
            m_pszDebugState = "ThrowAcquire";
            Grenade_ThrowAcquire();
            break;
        case AI_GRENSTATE_THROW:
            m_pszDebugState = "Throw";
            Grenade_Throw();
            break;
        case AI_GRENSTATE_KICK_ACQUIRE:
            m_pszDebugState = "KickAcquire";
            Grenade_KickAcquire();
            break;
        case AI_GRENSTATE_KICK:
            m_pszDebugState         = "Kick";
            m_bHasDesiredLookAngles = false;
            ContinueAnimation();
            break;
        case AI_GRENSTATE_MARTYR_ACQUIRE:
            m_pszDebugState = "MartyrAcquire";
            Grenade_MartyrAcquire();
            break;
        case AI_GRENSTATE_MARTYR:
            m_pszDebugState = "Martyr";
            Grenade_Martyr();
            break;
        default:
            m_pszDebugState       = "***Invalid***";
            char assertStr[16317] = {0};
            strcpy(assertStr, "\"invalid grenade state\"\n\tMessage: ");
            Q_strcat(assertStr, sizeof(assertStr), DumpCallTrace("thinkstate = %i", m_State));
            assert(!assertStr);
            break;
        }
    } else if (m_eGrenadeState && m_eGrenadeState != AI_GRENSTATE_FLEE_FAIL) {
        if (m_eGrenadeMode == AI_GRENSTATE_FLEE_SUCCESS) {
            Anim_Attack();
        } else {
            Anim_Stand();
        }
        Grenade_NextThinkState();
    } else {
        m_pszDebugState = "Wait";
        Grenade_Wait();
    }

    PostThink(false);
}
