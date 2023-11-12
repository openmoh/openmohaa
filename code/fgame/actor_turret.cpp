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
    return m_State > ACTOR_STATE_TURRET_SHOOT && m_State < ACTOR_STATE_TURRET_NUM_STATES;
}

bool Actor::Turret_DecideToSelectState(void)
{
    switch (m_State) {
    case ACTOR_STATE_TURRET_COMBAT:
        if (level.inttime > m_iStateTime + 5000) {
            InterruptPoint_Turret();
        }
        return false;
    case ACTOR_STATE_TURRET_REACQUIRE:
    case ACTOR_STATE_TURRET_GRENADE:
    case ACTOR_STATE_TURRET_TAKE_SNIPER_NODE:
    case ACTOR_STATE_TURRET_FAKE_ENEMY:
    case ACTOR_STATE_TURRET_SHOOT:
        return false;
    }

    return !Turret_IsRetargeting();
}

void Actor::Turret_SelectState(void)
{
    float  fDistSquared;
    vec2_t vDelta;

    if (!m_Enemy) {
        TransitionState(ACTOR_STATE_TURRET_FAKE_ENEMY, (rand() & 0x7FF) + 250);
        return;
    }

    VectorSub2D(origin, m_vHome, vDelta);
    fDistSquared = VectorLength2DSquared(vDelta);

    if (m_State == ACTOR_STATE_TURRET_RUN_HOME && fDistSquared > (m_fLeashSquared * 0.64 + 64.0)) {
        if (PathExists() && !PathComplete()) {
            return;
        }
    } else if (fDistSquared <= m_fLeashSquared * 1.21 + 256.0) {
        m_iRunHomeTime = 0;
    } else if (!m_iRunHomeTime) {
        m_iRunHomeTime = level.inttime + (rand() & 0xFFF) + 1000;
    } else if (level.inttime >= m_iRunHomeTime) {
        m_iRunHomeTime = 0;

        ClearPath();
        SetPath(m_vHome, NULL, 0, NULL, 0.0);
        ShortenPathToAvoidSquadMates();

        if (!PathExists()) {
            Com_Printf(
                "^~^~^ (entnum %i, radnum %d, targetname '%s') cannot reach his leash home\n",
                entnum,
                radnum,
                targetname.c_str()
            );
        } else if (!PathComplete()) {
            TransitionState(ACTOR_STATE_TURRET_RUN_HOME, 0);
            return;
        }
    }

    VectorSub2D(origin, m_Enemy->origin, vDelta);
    fDistSquared = VectorLength2DSquared(vDelta);

    if (m_State == ACTOR_STATE_TURRET_RUN_AWAY && fDistSquared < m_fMinDistanceSquared * 2.25) {
        return;
    }

    if (fDistSquared < m_fMinDistanceSquared) {
        ClearPath();
        TransitionState(ACTOR_STATE_TURRET_RUN_AWAY, 0);
        return;
    }

    if (fDistSquared > m_fMaxDistanceSquared) {
        if (m_Team == TEAM_GERMAN && (m_Enemy->origin - m_vHome).lengthSquared() >= Square(m_fLeash + m_fMaxDistance)
            && !CanSeeEnemy(200)) {
            ClearPath();
            TransitionState(ACTOR_STATE_TURRET_WAIT, 0);
        } else if (m_State != ACTOR_STATE_TURRET_CHARGE) {
            ClearPath();
            TransitionState(ACTOR_STATE_TURRET_CHARGE, 0);
        }
    } else {
        if (DecideToThrowGrenade(m_vLastEnemyPos + m_Enemy->velocity, &m_vGrenadeVel, &m_eGrenadeMode, false)) {
            SetDesiredYawDir(m_vGrenadeVel);

            DesiredAnimation(
                ANIM_MODE_NORMAL,
                m_eGrenadeMode == AI_GREN_TOSS_ROLL ? STRING_ANIM_GRENADETOSS_SCR : STRING_ANIM_GRENADETHROW_SCR
            );
            TransitionState(ACTOR_STATE_TURRET_GRENADE, 0);
            return;
        }

        if (m_State != ACTOR_STATE_TURRET_COMBAT && m_State != ACTOR_STATE_TURRET_SNIPER_NODE
            && m_State != ACTOR_STATE_TURRET_WAIT && m_State != ACTOR_STATE_TURRET_SHOOT) {
            ClearPath();
            TransitionState(ACTOR_STATE_TURRET_COMBAT);
        }
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
        TransitionState(ACTOR_STATE_TURRET_COVER_INSTEAD, 0);
    } else {
        TransitionState(ACTOR_STATE_TURRET_FAKE_ENEMY, (rand() & 0x7FF) + 250);
    }
}

void Actor::End_Turret(void)
{
    if (m_pCoverNode && m_State != ACTOR_STATE_TURRET_BECOME_COVER) {
        m_pCoverNode->Relinquish();
        m_pCoverNode = NULL;
    }
    TransitionState(-1, 0);
}

void Actor::Suspend_Turret(void)
{
    if (!m_Enemy) {
        TransitionState(ACTOR_STATE_TURRET_COVER_INSTEAD, 0);
    } else if (m_State == ACTOR_STATE_TURRET_GRENADE || m_State == ACTOR_STATE_TURRET_INTRO_AIM) {
        Turret_BeginRetarget();
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
        FindPathNearWithLeash(m_vLastEnemyPos, m_fMinDistanceSquared * 4);
        if (!ShortenPathToAttack(0.0)) {
            ClearPath();
        }
        ShortenPathToAvoidSquadMates();
    }

    if (!PathExists() || PathComplete() || !PathAvoidsSquadMates()) {
        m_pszDebugState = "combat->chill";
        Turret_BeginRetarget();
        return;
    }

    m_pszDebugState = "combat->move";
    if (!MovePathWithLeash()) {
        m_pszDebugState = "combat->move->aim";
        Turret_BeginRetarget();
        return;
    }

    Turret_CheckRetarget();
}

void Actor::Turret_BeginRetarget(void)
{
    SetEnemyPos(m_Enemy->origin);
    AimAtEnemyBehavior();

    TransitionState(ACTOR_STATE_TURRET_RETARGET_SUPPRESS, 0);
}

void Actor::Turret_NextRetarget(void)
{
    vec2_t vDelta;

    m_State++;
    if (m_State < ACTOR_STATE_TURRET_NUM_STATES) {
        TransitionState(m_State);
        return;
    }

    VectorSub2D(origin, m_vHome, vDelta);

    if (VectorLength2DSquared(vDelta) >= m_fLeashSquared) {
        SetPath(m_vHome, NULL, 0, NULL, 0.0);
        ShortenPathToAvoidSquadMates();

        if (PathExists() && !PathComplete()) {
            TransitionState(ACTOR_STATE_TURRET_RUN_HOME, 0);
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

        TransitionState(ACTOR_STATE_TURRET_COMBAT, 0);
        State_Turret_Combat();
    } else if (CanSeeEnemy(200)) {
        m_pszDebugState = "Retarget->Combat";
        TransitionState(ACTOR_STATE_TURRET_COMBAT, 0);
        State_Turret_Combat();
    } else {
        TransitionState(ACTOR_STATE_TURRET_WAIT, 0);
        State_Turret_Wait();
    }
}

void Actor::Turret_SideStep(int iStepSize, vec3_t vDir)
{
    AimAtEnemyBehavior();
    StrafeToAttack(iStepSize, vDir);

    if (!PathExists() || PathComplete() || !PathAvoidsSquadMates()) {
        StrafeToAttack(-iStepSize, vDir);
    }

    if (PathExists() && !PathComplete() && PathAvoidsSquadMates()) {
        TransitionState(ACTOR_STATE_TURRET_REACQUIRE);
    } else {
        Turret_NextRetarget();
    }
}

void Actor::State_Turret_Shoot(void)
{
    if (CanSeeEnemy(200) || FriendlyInLineOfFire(m_Enemy)) {
        TransitionState(ACTOR_STATE_TURRET_COMBAT);
        State_Turret_Combat();
        return;
    }

    AimAtTargetPos();
    Anim_Suppress();

    if (level.inttime >= m_iStateTime + 15000) {
        Turret_SelectState();
        if (m_State == ACTOR_STATE_TURRET_SHOOT) {
            Turret_BeginRetarget();
        }
    }
}

void Actor::State_Turret_Retarget_Suppress(void)
{
    trace_t trace;

    if (rand() % 100 >= m_iSuppressChance) {
        AimAtEnemyBehavior();
        Turret_NextRetarget();
        return;
    }

    if (level.inttime >= m_iLastEnemyVisibleTime + 15000) {
        AimAtEnemyBehavior();
        Turret_NextRetarget();
        return;
    }

    if (FriendlyInLineOfFire(m_Enemy)) {
        AimAtEnemyBehavior();
        Turret_NextRetarget();
        return;
    }

    trace = G_Trace(
        EyePosition(),
        vec_zero,
        vec_zero,
        m_Enemy->EyePosition(),
        this,
        MASK_CANSEE,
        qfalse,
        "Actor::State_Turret_Retarget_Suppress"
    );
    if (trace.fraction <= 0.5f) {
        AimAtEnemyBehavior();
        Turret_NextRetarget();
        return;
    }

    if (trace.fraction != 1.f && trace.plane.normal[2] >= 0.7f) {
        AimAtEnemyBehavior();
        Turret_NextRetarget();
        return;
    }

    TransitionState(ACTOR_STATE_TURRET_SHOOT);
    State_Turret_Shoot();
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
        TransitionState(ACTOR_STATE_TURRET_TAKE_SNIPER_NODE);
        State_Turret_TakeSniperNode();
    } else if (bTryAgain) {
        ContinueAnimation();
    } else {
        Turret_NextRetarget();
    }
}

void Actor::State_Turret_Retarget_Step_Side_Small(void)
{
    Turret_SideStep((rand() & 64) - 32, orientation[1]);
}

void Actor::State_Turret_Retarget_Path_Exact(void)
{
    AimAtEnemyBehavior();
    SetPathWithLeash(m_vLastEnemyPos, NULL, 0);

    if (!ShortenPathToAttack(128)) {
        Turret_NextRetarget();
        return;
    }

    ShortenPathToAvoidSquadMates();

    if (!PathExists()) {
        Turret_NextRetarget();
        return;
    }

    TransitionState(ACTOR_STATE_TURRET_REACQUIRE, 0);
}

void Actor::State_Turret_Retarget_Path_Near(void)
{
    AimAtEnemyBehavior();
    FindPathNearWithLeash(m_vLastEnemyPos, m_fMinDistanceSquared);

    if (ShortenPathToAttack(128)) {
        TransitionState(ACTOR_STATE_TURRET_REACQUIRE, 0);
    } else {
        Turret_NextRetarget();
    }
}

void Actor::State_Turret_Retarget_Step_Side_Medium(void)
{
    Turret_SideStep((rand() & 256) - 128, orientation[1]);
}

void Actor::State_Turret_Retarget_Step_Side_Large(void)
{
    Turret_SideStep((rand() & 512) - 256, orientation[1]);
}

void Actor::State_Turret_Retarget_Step_Face_Medium(void)
{
    Turret_SideStep((rand() & 256) - 128, orientation[0]);
}

void Actor::State_Turret_Retarget_Step_Face_Large(void)
{
    Turret_SideStep((rand() & 512) - 256, orientation[0]);
}

void Actor::State_Turret_Reacquire(void)
{
    if (!PathExists() || PathComplete()) {
        m_pszDebugState = "Retarget->Cheat";
        SetEnemyPos(m_Enemy->origin);
        TransitionState(ACTOR_STATE_TURRET_COMBAT, 0);
        State_Turret_Combat();
        return;
    }

    if (CanMovePathWithLeash()) {
        Anim_RunToInOpen(ANIM_MODE_PATH_GOAL);
        FaceEnemyOrMotion(level.inttime - m_iStateTime);
    } else {
        Turret_BeginRetarget();
    }
}

void Actor::State_Turret_TakeSniperNode(void)
{
    if (!PathExists() || PathComplete()) {
        AimAtEnemyBehavior();
        TransitionState(ACTOR_STATE_TURRET_SNIPER_NODE, 0);
        return;
    }

    FaceMotion();
    Anim_RunToDanger(ANIM_MODE_PATH_GOAL);
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
    }

    FaceMotion();
    Anim_RunToInOpen(ANIM_MODE_PATH);
    return true;
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

    Anim_RunAwayFiring(ANIM_MODE_PATH);
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

        m_pszDebugState = "charge->chill";

        ForwardLook();
        Anim_Idle();

        if (m_Team == TEAM_AMERICAN || m_PotentialEnemies.HasAlternateEnemy()) {
            m_PotentialEnemies.FlagBadEnemy(m_Enemy);
            UpdateEnemy(-1);
        }

        if (m_Enemy) {
            Turret_CheckRetarget();
        }
    } else if (!MovePathWithLeash()) {
        m_pszDebugState = "charge->leash->combat";
        TransitionState(ACTOR_STATE_TURRET_COMBAT, 0);
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
        SetThinkState(THINKSTATE_IDLE, THINKLEVEL_IDLE);
    }
}

void Actor::State_Turret_Wait(void)
{
    PathNode *pNode;

    if (CanSeeEnemy(500) || CanShootEnemy(500)) {
        if (Turret_TryToBecomeCoverGuy()) {
            m_pszDebugState = "Wait->CoverInstead";
            ContinueAnimation();
        } else {
            m_pszDebugState = "Wait->Combat";
            TransitionState(ACTOR_STATE_TURRET_COMBAT);
            State_Turret_Combat();
        }
        return;
    }

    if (level.inttime >= m_iLastEnemyVisibleTime + 25000) {
        m_iLastEnemyVisibleTime = level.inttime;
        m_vLastEnemyPos         = m_Enemy->origin;
        Turret_BeginRetarget();
    }

    if (level.inttime >= m_iLastFaceDecideTime + 1500) {
        m_iLastFaceDecideTime = level.inttime + (rand() & 0x1FF);

        pNode = PathManager.FindCornerNodeForExactPath(this, m_Enemy, 0);

        if (pNode) {
            SetDesiredYawDest(pNode->m_PathPos);
            m_eDontFaceWallMode = 6;
        } else {
            AimAtTargetPos();
            DontFaceWall();
        }
    }

    if (m_eDontFaceWallMode == 7 || m_eDontFaceWallMode == 8) {
        Anim_Stand();
    } else {
        Anim_Aim();
    }
}

void Actor::Think_Turret(void)
{
    if (!RequireThink()) {
        return;
    }

    UpdateEyeOrigin();
    NoPoint();
    UpdateEnemy(200);

    if (m_Enemy && m_State == ACTOR_STATE_TURRET_COVER_INSTEAD) {
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

        if (m_State == ACTOR_STATE_TURRET_COMBAT && !CanSeeEnemy(0)) {
            Turret_BeginRetarget();
        }

        SetLeashHome(origin);

        if (level.inttime < m_iEnemyChangeTime + 200 && AttackEntryAnimation()) {
            TransitionState(ACTOR_STATE_TURRET_INTRO_AIM);
            m_bLockThinkState = true;
        }
    }

    if (level.inttime > m_iStateTime + 3000) {
        Turret_SelectState();
    }

    if (m_State == ACTOR_STATE_TURRET_INTRO_AIM) {
        m_pszDebugState = "IntroAnim";
        AimAtTargetPos();
        ContinueAnimation();

        if (m_State == ACTOR_STATE_TURRET_WAIT) {
            PostThink(false);
        } else {
            PostThink(true);
        }
        return;
    }

    m_bLockThinkState = false;

    if (!m_Enemy && m_State != ACTOR_STATE_TURRET_FAKE_ENEMY && m_State != ACTOR_STATE_TURRET_RUN_HOME) {
        TransitionState(ACTOR_STATE_TURRET_FAKE_ENEMY, (rand() + 250) & 0x7FF);
    }

    if (!m_Enemy && m_State != ACTOR_STATE_TURRET_FAKE_ENEMY) {
        if (m_State != ACTOR_STATE_TURRET_RUN_HOME
            || (origin - m_vHome).lengthXYSquared() <= (m_fLeashSquared * 0.64f + 64.0f)
            || !State_Turret_RunHome(false)) {
            m_pszDebugState = "Idle";
            SetThinkState(THINKSTATE_IDLE, THINKLEVEL_IDLE);
            IdleThink();
        } else {
            m_pszDebugState = "Idle->RunHome";
            PostThink(true);
        }
        return;
    }

    if (m_Enemy && m_State == ACTOR_STATE_TURRET_FAKE_ENEMY) {
        Turret_BeginRetarget();
    }

    if (Turret_DecideToSelectState()) {
        Turret_SelectState();
    }

    switch (m_State) {
    case ACTOR_STATE_TURRET_COMBAT:
        m_pszDebugState = "Combat";
        State_Turret_Combat();
        break;
    case ACTOR_STATE_TURRET_REACQUIRE:
        m_pszDebugState = "Reacquire";
        State_Turret_Reacquire();
        break;
    case ACTOR_STATE_TURRET_TAKE_SNIPER_NODE:
        m_pszDebugState = "TakeSniperNode";
        State_Turret_TakeSniperNode();
        break;
    case ACTOR_STATE_TURRET_SNIPER_NODE:
        m_pszDebugState = "SniperNode";
        State_Turret_SniperNode();
        break;
    case ACTOR_STATE_TURRET_RUN_HOME:
        m_pszDebugState = "RunHome";
        State_Turret_RunHome(true);
        break;
    case ACTOR_STATE_TURRET_RUN_AWAY:
        m_pszDebugState = "RunAway";
        State_Turret_RunAway();
        break;
    case ACTOR_STATE_TURRET_CHARGE:
        m_pszDebugState = "Charge";
        State_Turret_Charge();
        break;
    case ACTOR_STATE_TURRET_GRENADE:
        m_pszDebugState = "Grenade";
        State_Turret_Grenade();
        break;
    case ACTOR_STATE_TURRET_FAKE_ENEMY:
        m_pszDebugState = "FakeEnemy";
        State_Turret_FakeEnemy();
        break;
    case ACTOR_STATE_TURRET_BECOME_COVER:
        m_pszDebugState = "BecomeCover";
        ContinueAnimation();
        break;
    case ACTOR_STATE_TURRET_WAIT:
        m_pszDebugState = "Wait";
        State_Turret_Wait();
        break;
    case ACTOR_STATE_TURRET_SHOOT:
        m_pszDebugState = "Shoot";
        State_Turret_Shoot();
        break;
    case ACTOR_STATE_TURRET_RETARGET_SUPPRESS:
        m_pszDebugState = "Retarget_Suppress";
        State_Turret_Retarget_Suppress();
        break;
    case ACTOR_STATE_TURRET_RETARGET_SNIPER_NODE:
        m_pszDebugState = "Retarget_Sniper_Node";
        State_Turret_Retarget_Sniper_Node();
        break;
    case ACTOR_STATE_TURRET_RETARGET_STEP_SIDE_SMALL:
        m_pszDebugState = "Retarget_Step_Side_Small";
        State_Turret_Retarget_Step_Side_Small();
        break;
    case ACTOR_STATE_TURRET_RETARGET_PATH_EXACT:
        m_pszDebugState = "Retarget_Path_Exact";
        State_Turret_Retarget_Path_Exact();
        break;
    case ACTOR_STATE_TURRET_RETARGET_PATH_NEAR:
        m_pszDebugState = "Retarget_Path_Near";
        State_Turret_Retarget_Path_Near();
        break;
    case ACTOR_STATE_TURRET_RETARGET_STEP_SIDE_MEDIUM:
        m_pszDebugState = "Retarget_Step_Side_Medium";
        State_Turret_Retarget_Step_Side_Medium();
        break;
    case ACTOR_STATE_TURRET_RETARGET_STEP_SIDE_LARGE:
        m_pszDebugState = "Retarget_Step_Side_Large";
        State_Turret_Retarget_Step_Side_Large();
        break;
    case ACTOR_STATE_TURRET_RETARGET_STEP_FACE_MEDIUM:
        m_pszDebugState = "Retarget_Step_Face_Medium";
        State_Turret_Retarget_Step_Face_Medium();
        break;
    case ACTOR_STATE_TURRET_RETARGET_STEP_FACE_LARGE:
        m_pszDebugState = "Retarget_Step_Face_Large";
        State_Turret_Retarget_Step_Face_Large();
        break;
    default:
        Com_Printf("Actor::Think_Turret: invalid think state %i\n", m_State);
        assert(!"invalid think state");
        break;
    }

    if (!CheckForTransition(THINKSTATE_GRENADE, THINKLEVEL_IDLE)) {
        CheckForTransition(THINKSTATE_BADPLACE, THINKLEVEL_IDLE);
    }

    if (m_State == ACTOR_STATE_TURRET_WAIT) {
        PostThink(false);
    } else {
        PostThink(true);
    }
}

void Actor::ReceiveAIEvent_Turret(
    vec3_t event_origin, int iType, Entity *originator, float fDistSquared, float fMaxDistSquared
)
{
    if (iType == AI_EVENT_WEAPON_IMPACT && m_Enemy && fDistSquared <= Square(128)) {
        Turret_TryToBecomeCoverGuy();
        return;
    }

    DefaultReceiveAIEvent(origin, iType, originator, fDistSquared, fMaxDistSquared);
}

bool Actor::Turret_TryToBecomeCoverGuy(void)
{
    PathNode *pOldCover = m_pCoverNode;

    Cover_FindCover(true);

    if (m_pCoverNode) {
        TransitionState(ACTOR_STATE_TURRET_BECOME_COVER, 0);
        SetThink(THINKSTATE_ATTACK, THINK_COVER);
        return true;
    }

    if (pOldCover) {
        m_pCoverNode = pOldCover;
        m_pCoverNode->Claim(this);
    }

    return false;
}

void Actor::FinishedAnimation_Turret(void)
{
    if (m_State == ACTOR_STATE_TURRET_GRENADE || m_State == ACTOR_STATE_TURRET_INTRO_AIM
        || m_State == ACTOR_STATE_TURRET_SHOOT) {
        Turret_SelectState();
    }
}

void Actor::InterruptPoint_Turret(void)
{
    if (m_Enemy && !Turret_TryToBecomeCoverGuy() && m_State == ACTOR_STATE_TURRET_COMBAT) {
        m_iStateTime = level.inttime;
        Turret_SelectState();
    }
}

void Actor::PathnodeClaimRevoked_Turret(void)
{
    if (!m_Enemy) {
        TransitionState(ACTOR_STATE_TURRET_COVER_INSTEAD, 0);
    } else {
        Turret_BeginRetarget();
    }
}
