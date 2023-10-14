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

// actor_turret.cpp

#include "actor.h"

void Actor::InitTurret(GlobalFuncs_t *func)
{
    func->ThinkState                 = &Actor::Think_Turret;
    func->BeginState                 = &Actor::Begin_Turret;
    func->EndState                   = &Actor::End_Turret;
    func->SuspendState               = &Actor::Suspend_Turret;
    func->RestartState               = NULL;
    func->FinishedAnimation          = &Actor::FinishedAnimation_Turret;
    func->PassesTransitionConditions = &Actor::PassesTransitionConditions_Attack;
    func->PostShoot                  = &Actor::InterruptPoint_Turret;
    func->ReceiveAIEvent             = &Actor::ReceiveAIEvent_Turret;
    func->IsState                    = &Actor::IsAttackState;
    func->PathnodeClaimRevoked       = &Actor::PathnodeClaimRevoked_Turret;
}

bool Actor::Turret_IsRetargeting(void) const
{
    return m_State <= 120;
}

bool Actor::Turret_DecideToSelectState(void)
{
    switch (m_State) {
    case 100:
        if (level.inttime > m_iStateTime + 5000) {
            InterruptPoint_Turret();
        }
        return false;
    case 101:
    case 107:
    case 102:
    case 109:
        return false;
    }
    return !Turret_IsRetargeting();
}

void Actor::Turret_SelectState(void)
{
    vec2_t vDelta;
    float  fDistSquared;
    if (!m_Enemy) {
        TransitionState(109, rand() & 0x7FF + 250);
        return;
    }

    VectorSub2D(origin, m_vHome, vDelta);
    fDistSquared = VectorLength2DSquared(vDelta);

    if (m_State == 104 && fDistSquared > m_fLeashSquared * 0.64 + 64.0) {
        if (PathExists() && !PathComplete()) {
            return;
        }
    } else if (fDistSquared <= m_fLeashSquared * 1.21 + 256.0) {
        m_iRunHomeTime = 0;
    } else if (m_iRunHomeTime == 0) {
        m_iRunHomeTime = level.inttime + (rand() & 0xFFF) + 1000;
    } else if (level.inttime >= m_iRunHomeTime) {
        m_iRunHomeTime = 0;

        ClearPath();
        SetPath(m_vHome, NULL, 0, NULL, 0.0);
        ShortenPathToAvoidSquadMates();

        if (PathExists() && !PathComplete()) {
            TransitionState(104, 0);
            return;
        } else {
            Com_Printf(
                "^~^~^ (entnum %i, radnum %d, targetname '%s') cannot reach his leash home\n",
                entnum,
                radnum,
                targetname.c_str()
            );
        }
    }

    VectorSub2D(origin, m_Enemy->origin, vDelta);
    fDistSquared = VectorLength2DSquared(vDelta);

    if (m_State == 105) {
        if (m_fMinDistanceSquared * 2.25 > fDistSquared) {
            return;
        }
    }

    if (m_fMinDistanceSquared > fDistSquared) {
        ClearPath();
        TransitionState(105, 0);
        return;
    }

    if (fDistSquared > m_fMaxDistanceSquared) {
        bool bSmthing = false;
        if (m_Team == TEAM_GERMAN) {
            if ((m_Enemy->origin - m_vHome).lengthSquared() >= Square(m_fLeash + m_fMaxDistance) && !CanSeeEnemy(200)) {
                bSmthing = true;
            }
        }
        if (bSmthing) {
            ClearPath();
            TransitionState(112, 0);
        } else {
            if (m_State == 106) {
                return;
            }
            ClearPath();
            TransitionState(106, 0);
        }
        return;
    }

    if (DecideToThrowGrenade(m_Enemy->velocity + m_vLastEnemyPos, &m_vGrenadeVel, &m_eGrenadeMode)) {
        m_bNextForceStart = false;
        SetDesiredYawDir(m_vGrenadeVel);

        m_eNextAnimMode = ANIM_MODE_NORMAL;
        m_csNextAnimString =
            (m_eGrenadeMode == AI_GREN_TOSS_ROLL) ? STRING_ANIM_GRENADETOSS_SCR : STRING_ANIM_GRENADETHROW_SCR;
        TransitionState(107, 0);
        return;
    }

    if (m_State != 100 && m_State != 103 && m_State != 112) {
        ClearPath();
        TransitionState(100, 0);
    }
}

bool Actor::Turret_CheckRetarget(void)
{
    if (level.inttime < m_iStateTime + 5000 || level.inttime < m_iLastHitTime + 5000) {
        return false;
    }

    Turret_BeginRetarget();

    return true;
}

void Actor::Begin_Turret(void)
{
    DoForceActivate();
    m_csMood = STRING_ALERT;

    ClearPath();

    if (m_Enemy) {
        TransitionState(110, 0);
    } else {
        TransitionState(109, (rand() & 0x7FF) + 250);
    }
}

void Actor::End_Turret(void)
{
    if (m_pCoverNode && m_State != 111) {
        m_pCoverNode->Relinquish();
        m_pCoverNode = NULL;
    }
    TransitionState(-1, 0);
}

void Actor::Suspend_Turret(void)
{
    if (!m_Enemy) {
        TransitionState(110, 0);
    } else {
        if (m_State <= 108) {
            SetEnemyPos(m_Enemy->origin);
            AimAtEnemyBehavior();
            TransitionState(113, 0);
        }
    }
}

void Actor::State_Turret_Combat(void)
{
    if (CanSeeEnemy(200)) {
        ClearPath();
        Anim_Attack();
        AimAtTargetPos();
        Turret_CheckRetarget();
        return;
    }
    if (!PathExists() || PathComplete() || !PathAvoidsSquadMates()) {
        SetPathWithLeash(m_vLastEnemyPos, NULL, 0);
        ShortenPathToAvoidSquadMates();
    }
    if (!PathExists() || PathComplete() || !PathAvoidsSquadMates()) {
        FindPathNearWithLeash(m_vLastEnemyPos, 4.0 * m_fMinDistanceSquared);
        if (!ShortenPathToAttack(0.0)) {
            ClearPath();
        }
        ShortenPathToAvoidSquadMates();
    }
    if (!PathExists() || PathComplete() || !PathAvoidsSquadMates()) {
        m_pszDebugState = "combat->chill";
        Turret_BeginRetarget();
    } else {
        m_pszDebugState = "combat->move";
        if (!MovePathWithLeash()) {
            m_pszDebugState = "combat->move->aim";
            Turret_BeginRetarget();
        } else {
            Turret_CheckRetarget();
        }
    }
}

void Actor::Turret_BeginRetarget(void)
{
    SetEnemyPos(m_Enemy->origin);
    AimAtEnemyBehavior();

    TransitionState(113, 0);
}

void Actor::Turret_NextRetarget(void)
{
    vec2_t vDelta;
    float  fDistSquared;

    m_State++;

    if (Turret_IsRetargeting()) {
        m_iStateTime = level.inttime;
        return;
    }

    VectorSub2D(origin, m_vHome, vDelta);
    fDistSquared = VectorLength2DSquared(vDelta);

    if (fDistSquared >= m_fLeashSquared) {
        SetPath(m_vHome, NULL, 0, NULL, 0.0);
        ShortenPathToAvoidSquadMates();
        if (PathExists() && !PathComplete()) {
            TransitionState(104, 0);
            State_Turret_RunHome(true);
            return;
        }
    }

    if (m_Team == TEAM_AMERICAN) {
        if (!CanSeeEnemy(200)) {
            m_PotentialEnemies.FlagBadEnemy(m_Enemy);
            UpdateEnemy(-1);
        }
        if (!m_Enemy) {
            Anim_Stand();
            return;
        }
        TransitionState(100, 0);
        State_Turret_Combat();

    } else {
        if (CanSeeEnemy(200)) {
            m_pszDebugState = "Retarget->Combat";
            TransitionState(100, 0);
            State_Turret_Combat();
        } else {
            TransitionState(112, 0);
            State_Turret_Wait();
        }
    }
}

void Actor::Turret_SideStep(int iStepSize, vec3_t vDir)
{
    AimAtEnemyBehavior();
    //v3 = iStepSize;
    StrafeToAttack(iStepSize, vDir);
    if ((PathExists() && !PathComplete() && PathAvoidsSquadMates())
        || ((StrafeToAttack(-iStepSize, vDir), PathExists()) && !PathComplete() && PathAvoidsSquadMates())) {
        TransitionState(101, 0);
    } else {
        Turret_NextRetarget();
    }
}

void Actor::State_Turret_Shoot(void)
{
    // FIXME: unimplemented
}

void Actor::State_Turret_Retarget_Suppress(void)
{
    // FIXME: unimplemented
}

void Actor::State_Turret_Retarget_Sniper_Node(void)
{
    PathNode *pSniperNode;
    bool      bTryAgain;

    AimAtEnemyBehavior();
    if (m_pCoverNode) {
        m_pCoverNode->Relinquish();
        m_pCoverNode = NULL;
    }
    pSniperNode = FindSniperNodeAndSetPath(&bTryAgain);
    if (pSniperNode) {
        m_pCoverNode = pSniperNode;
        pSniperNode->Claim(this);
        TransitionState(102, 0);

        State_Turret_TakeSniperNode();
    } else if (bTryAgain) {
        ContinueAnimation();
    } else {
        Turret_NextRetarget();
    }
}

void Actor::State_Turret_Retarget_Step_Side_Small(void)
{
    int iRand; // esi

    iRand = (rand() & 64) - 32;
    AimAtEnemyBehavior();
    StrafeToAttack(iRand, orientation[1]);
    if ((PathExists() && !PathComplete() && PathAvoidsSquadMates())
        || ((Actor::StrafeToAttack(-iRand, orientation[1]), PathExists()) && !PathComplete() && PathAvoidsSquadMates()
        )) {
        TransitionState(101, 0);
    } else {
        Turret_NextRetarget();
    }
}

void Actor::State_Turret_Retarget_Path_Exact(void)
{
    AimAtEnemyBehavior();
    SetPathWithLeash(m_vLastEnemyPos, NULL, 0);
    if (ShortenPathToAttack(128) && (ShortenPathToAvoidSquadMates(), PathExists())) {
        TransitionState(101, 0);
    } else {
        Turret_NextRetarget();
    }
}

void Actor::State_Turret_Retarget_Path_Near(void)
{
    AimAtEnemyBehavior();
    FindPathNearWithLeash(m_vLastEnemyPos, m_fMinDistanceSquared);
    if (ShortenPathToAttack(128)) {
        TransitionState(101, 0);
    } else {
        Turret_NextRetarget();
    }
}

void Actor::State_Turret_Retarget_Step_Side_Medium(void)
{
    int iRand; // esi

    iRand = (rand() & 256) - 128;
    AimAtEnemyBehavior();
    StrafeToAttack(iRand, orientation[1]);
    if ((PathExists() && !PathComplete() && PathAvoidsSquadMates())
        || ((Actor::StrafeToAttack(-iRand, orientation[1]), PathExists()) && !PathComplete() && PathAvoidsSquadMates()
        )) {
        TransitionState(101, 0);
    } else {
        Turret_NextRetarget();
    }
}

void Actor::State_Turret_Retarget_Step_Side_Large(void)
{
    int iRand; // esi

    iRand = (rand() & 512) - 256;
    AimAtEnemyBehavior();
    StrafeToAttack(iRand, orientation[1]);
    if ((PathExists() && !PathComplete() && PathAvoidsSquadMates())
        || ((Actor::StrafeToAttack(-iRand, orientation[1]), PathExists()) && !PathComplete() && PathAvoidsSquadMates()
        )) {
        TransitionState(101, 0);
    } else {
        Turret_NextRetarget();
    }
}

void Actor::State_Turret_Retarget_Step_Face_Medium(void)
{
    int iRand; // esi

    iRand = (rand() & 256) - 128;
    AimAtEnemyBehavior();
    StrafeToAttack(iRand, orientation[0]);
    if ((PathExists() && !PathComplete() && PathAvoidsSquadMates())
        || ((Actor::StrafeToAttack(-iRand, orientation[0]), PathExists()) && !PathComplete() && PathAvoidsSquadMates()
        )) {
        TransitionState(101, 0);
    } else {
        Turret_NextRetarget();
    }
}

void Actor::State_Turret_Retarget_Step_Face_Large(void)
{
    int iRand; // esi

    iRand = (rand() & 512) - 256;
    AimAtEnemyBehavior();
    StrafeToAttack(iRand, orientation[0]);
    if ((PathExists() && !PathComplete() && PathAvoidsSquadMates())
        || ((Actor::StrafeToAttack(-iRand, orientation[0]), PathExists()) && !PathComplete() && PathAvoidsSquadMates()
        )) {
        TransitionState(101, 0);
    } else {
        Turret_NextRetarget();
    }
}

void Actor::State_Turret_Reacquire(void)
{
    /*Sentient *v1; // ecx
	float v2; // ST08_4
	float v3; // ST0C_4
	Sentient *v4; // ecx
	float v5; // ST08_4
	float v6; // ST0C_4
	*/
    if (PathExists() && !PathComplete()) {
        if (CanMovePathWithLeash()) {
            Anim_RunToInOpen(3);
            FaceEnemyOrMotion(level.inttime - m_iStateTime);
        } else {
            Turret_BeginRetarget();
        }
    } else {
        m_pszDebugState = "Retarget->Cheat";
        SetEnemyPos(m_Enemy->origin);
        TransitionState(100, 0);
        State_Turret_Combat();
    }
}

void Actor::State_Turret_TakeSniperNode(void)
{
    if (PathExists() && !PathComplete()) {
        FaceMotion();
        Anim_RunToDanger(3);
    } else {
        AimAtEnemyBehavior();
        TransitionState(103, 0);
    }
}

void Actor::State_Turret_SniperNode(void)
{
    AimAtTargetPos();
    Anim_Sniper();
    if (Turret_CheckRetarget()) {
        m_pCoverNode->Relinquish();
        m_pCoverNode->MarkTemporarilyBad();
        m_pCoverNode = NULL;
    }
}

bool Actor::State_Turret_RunHome(bool bAttackOnFail)
{
    SetPath(m_vHome, NULL, 0, NULL, 0.0);
    ShortenPathToAvoidSquadMates();
    if (!PathExists() || PathComplete()) {
        Com_Printf(
            "^~^~^ (entnum %i, radnum %i, targetname '%s') cannot reach his leash home\n",
            entnum,
            radnum,
            targetname.c_str()
        );
        if (bAttackOnFail) {
            m_pszDebugState = "home->combat";
            State_Turret_Combat();
        }
        return false;
    } else {
        FaceMotion();
        Anim_RunToInOpen(2);
        return true;
    }
}

void Actor::State_Turret_RunAway(void)
{
    if (!PathExists() || PathComplete()) {
        FindPathAwayWithLeash(m_vLastEnemyPos, origin - m_Enemy->origin, 1.5 * m_fMinDistance);
    }
    if (!PathExists() || PathComplete()) {
        m_pszDebugState = "runaway->combat";
        State_Turret_Combat();
        return;
    }
    if (!CanMovePathWithLeash()) {
        m_pszDebugState = "runaway->leash->combat";
        State_Turret_Combat();
        return;
    }
    Anim_RunAwayFiring(2);
    FaceEnemyOrMotion(level.inttime - m_iStateTime);
}

void Actor::State_Turret_Charge(void)
{
    SetPathWithLeash(m_vLastEnemyPos, NULL, 0);
    ShortenPathToAvoidSquadMates();
    if (!PathExists()) {
        m_pszDebugState = "charge->near";
        FindPathNearWithLeash(m_vLastEnemyPos, m_fMaxDistanceSquared);
        if (!ShortenPathToAttack(0)) {
            ClearPath();
        }
    }
    if (!PathExists() || PathComplete() || !PathAvoidsSquadMates()) {
        ClearPath();
        if (CanSeeEnemy(500)) {
            m_pszDebugState = "charge->combat";
            State_Turret_Combat();
            return;
        }
        m_bHasDesiredLookAngles = false;
        m_pszDebugState         = "charge->chill";
        Anim_Idle();
        if (m_Team != TEAM_AMERICAN) {
            //v1 = &this->m_PotentialEnemies;
            if (!m_PotentialEnemies.HasAlternateEnemy()) {
                if (m_Enemy) {
                    Turret_CheckRetarget();
                }
                return;
            }
        }
        m_PotentialEnemies.FlagBadEnemy(m_Enemy);
        UpdateEnemy(-1);
        if (m_Enemy) {
            Turret_CheckRetarget();
        }
        return;
    }
    if (!MovePathWithLeash()) {
        m_pszDebugState = "charge->leash->combat";
        TransitionState(100, 0);
        State_Turret_Combat();
        return;
    }
}

void Actor::State_Turret_Grenade(void)
{
    GenericGrenadeTossThink();
}

void Actor::State_Turret_FakeEnemy(void)
{
    AimAtTargetPos();
    Anim_Aim();
    if (level.inttime >= m_iStateTime) {
        SetThinkState(THINKSTATE_IDLE, THINKLEVEL_NORMAL);
    }
}

void Actor::State_Turret_Wait(void)
{
    PathNode *pNode;
    if (CanSeeEnemy(500) || CanShootEnemy(500)) {
        bool bSmth;

        pNode = m_pCoverNode;
        Cover_FindCover(true);

        if (m_pCoverNode) {
            TransitionState(111, 0);
            SetThink(THINKSTATE_ATTACK, THINK_COVER);
            bSmth = true;
        } else {
            if (pNode) {
                m_pCoverNode = pNode;
                m_pCoverNode->Claim(this);
            }
            bSmth = false;
        }
        if (bSmth) {
            m_pszDebugState = "Wait->CoverInstead";
            ContinueAnimation();
        } else {
            m_pszDebugState = "Wait->Combat";
            TransitionState(100, 0);
            State_Turret_Combat();
        }
    } else {
        if (level.inttime >= m_iLastFaceDecideTime + 1500) {
            m_iLastFaceDecideTime = level.inttime + (rand() & 0x1FF);

            pNode = PathManager.FindCornerNodeForExactPath(this, m_Enemy, 0);

            if (pNode) {
                if (pNode->m_PathPos - origin != vec_zero) {
                    SetDesiredYawDir(pNode->m_PathPos - origin);
                }
                m_eDontFaceWallMode = 6;
            } else {
                AimAtTargetPos();
                DontFaceWall();
            }
        }
        if (m_eDontFaceWallMode > 8) {
            Anim_Aim();
        } else {
            Anim_Stand();
        }
    }
}

void Actor::Think_Turret(void)
{
    if (RequireThink()) {
        UpdateEyeOrigin();
        NoPoint();
        UpdateEnemy(200);

        if (m_Enemy && m_State == 110) {
            if (!m_bTurretNoInitialCover && Turret_TryToBecomeCoverGuy()) {
                m_pszDebugState = "CoverInstead";
                CheckUnregister();
                UpdateAngles();
                DoMove();
                UpdateBoneControllers();
                UpdateFootsteps();
                return;
            }

            m_bTurretNoInitialCover = false;

            Turret_SelectState();

            if (m_State == 100 && !CanSeeEnemy(0)) {
                Turret_BeginRetarget();
            }

            SetLeashHome(origin);

            if (level.inttime < m_iEnemyChangeTime + 200) {
                if (AttackEntryAnimation()) {
                    m_bLockThinkState = true;
                    TransitionState(108, 0);
                }
            }
        }

        if (level.inttime > m_iStateTime + 3000) {
            Turret_SelectState();
        }

        if (m_State == 108) {
            m_pszDebugState = "IntroAnim";
            AimAtTargetPos();
            ContinueAnimation();
        } else {
            m_bLockThinkState = false;
            if (!m_Enemy && m_State != 109 && m_State != 104) {
                TransitionState(109, ((rand() + 250) & 0x7FF));
            }
            if (!m_Enemy) {
                if (m_State != 109) {
                    if (m_State != 104 || (origin - m_vHome).lengthXYSquared() <= 0.64f * m_fLeashSquared + 64.0f
                        || !State_Turret_RunHome(false)) {
                        m_pszDebugState = "Idle";
                        SetThinkState(THINKSTATE_IDLE, THINKLEVEL_NORMAL);
                        IdleThink();
                    } else {
                        m_pszDebugState = "Idle->RunHome";
                        PostThink(true);
                    }
                    return;
                }
            }
            if (m_Enemy && m_State == 109) {
                Turret_BeginRetarget();
            }

            if (Turret_DecideToSelectState()) {
                Turret_SelectState();
            }

            switch (m_State) {
            case 100:
                m_pszDebugState = "Combat";
                State_Turret_Combat();
                break;
            case 101:
                m_pszDebugState = "Reacquire";
                State_Turret_Reacquire();
                break;
            case 102:
                m_pszDebugState = "TakeSniperNode";
                if (!PathExists() || PathComplete()) {
                    AimAtEnemyBehavior();
                    TransitionState(103, 0);
                } else {
                    FaceMotion();
                    Anim_RunToDanger(3);
                }
                break;
            case 103:
                m_pszDebugState = "SniperNode";
                State_Turret_SniperNode();
                break;
            case 104:
                m_pszDebugState = "RunHome";
                State_Turret_RunHome(true);
                break;
            case 105:
                m_pszDebugState = "RunAway";
                State_Turret_RunAway();
                break;
            case 106:
                m_pszDebugState = "Charge";
                State_Turret_Charge();
                break;
            case 107:
                m_pszDebugState = "Grenade";
                State_Turret_Grenade();
                break;
            case 109:
                m_pszDebugState = "FakeEnemy";
                State_Turret_FakeEnemy();
                break;
            case 111:
                m_pszDebugState = "BecomeCover";
                ContinueAnimation();
                break;
            case 112:
                m_pszDebugState = "Wait";
                State_Turret_Wait();
                break;
            case 113:
                m_pszDebugState = "Retarget_Sniper_Node";
                State_Turret_Retarget_Sniper_Node();
                break;
            case 114:
                m_pszDebugState = "Retarget_Step_Side_Small";
                State_Turret_Retarget_Step_Side_Small();
                break;
            case 115:
                m_pszDebugState = "Retarget_Path_Exact";
                State_Turret_Retarget_Path_Exact();
                break;
            case 116:
                m_pszDebugState = "Retarget_Path_Near";
                State_Turret_Retarget_Path_Near();
                break;
            case 117:
                m_pszDebugState = "Retarget_Step_Side_Medium";
                State_Turret_Retarget_Step_Side_Medium();
                break;
            case 118:
                m_pszDebugState = "Retarget_Step_Side_Large";
                State_Turret_Retarget_Step_Side_Large();
                break;
            case 119:
                m_pszDebugState = "Retarget_Step_Face_Medium";
                State_Turret_Retarget_Step_Face_Medium();
                break;
            case 120:
                m_pszDebugState = "Retarget_Step_Face_Large";
                State_Turret_Retarget_Step_Face_Large();
                break;
            default:
                Com_Printf("Actor::Think_Turret: invalid think state %i\n", m_State);
                assert(!"invalid think state");
                break;
            }
            CheckForTransition(THINKSTATE_GRENADE, THINKLEVEL_NORMAL);
        }
        if (m_State == 112) {
            PostThink(false);
        } else {
            PostThink(true);
        }
    }
}

void Actor::ReceiveAIEvent_Turret(
    vec3_t event_origin, int iType, Entity *originator, float fDistSquared, float fMaxDistSquared
)
{
    if (iType == AI_EVENT_WEAPON_IMPACT) {
        if (m_Enemy && fDistSquared <= Square(128)) {
            Turret_TryToBecomeCoverGuy();
        }
    } else {
        DefaultReceiveAIEvent(origin, iType, originator, fDistSquared, fMaxDistSquared);
    }
}

bool Actor::Turret_TryToBecomeCoverGuy(void)
{
    PathNode *pOldCover = m_pCoverNode;
    Cover_FindCover(true);
    if (m_pCoverNode) {
        TransitionState(111, 0);
        SetThink(THINKSTATE_ATTACK, THINK_COVER);
        return true;
    } else {
        if (pOldCover) {
            m_pCoverNode = pOldCover;
            m_pCoverNode->Claim(this);
        }
        return false;
    }
}

void Actor::FinishedAnimation_Turret(void)
{
    if (m_State <= 108) {
        Turret_SelectState();
    }
}

void Actor::InterruptPoint_Turret(void)
{
    if (m_Enemy && !Turret_TryToBecomeCoverGuy() && m_State == 100) {
        m_iStateTime = level.inttime;
        Turret_SelectState();
    }
}

void Actor::PathnodeClaimRevoked_Turret(void)
{
    if (m_Enemy == NULL) {
        TransitionState(110, 0);
    } else {
        Turret_BeginRetarget();
    }
}
