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

// actor_cover.cpp

#include "actor.h"

static int Cover_HideTime(int iTeam)
{
    if (iTeam == TEAM_AMERICAN) {
        return rand() % 2001 + 2000;
    } else {
        return rand() % 11001 + 4000;
    }
}

bool Actor::Cover_IsValid(PathNode *node)
{
    Vector sight_origin = node->origin + eyeposition;

    if (node->IsClaimedByOther(this)) {
        return false;
    }

    if (node->nodeflags & (AI_CONCEALMENT | AI_LOW_WALL_ARC)) {
        return true;
    }

    if (!CanSeeFrom(sight_origin, m_Enemy)) {
        return true;
    }

    if (!(node->nodeflags & AI_DUCK)) {
        return false;
    }

    if (CanSee(sight_origin - Vector(0, 0, 32), 0, 0, false)) {
        return false;
    }

    return true;
}

bool Actor::Cover_SetPath(PathNode *node)
{
    float     origin_ratio;
    Vector    enemy_offset;
    PathInfo *current_node;
    Vector    enemy_origin;
    vec2_t    vDelta;
    float     fMinDistSquared;
    float     fPathDist;

    SetPathWithLeash(node, NULL, 0);

    if (!PathExists()) {
        return false;
    }

    fPathDist       = PathDist();
    fMinDistSquared = Square(fPathDist);

    if ((node->origin - origin).lengthSquared() <= fMinDistSquared * 4.0f && fPathDist > 128.0f) {
        return false;
    }

    if (PathComplete()) {
        return true;
    }

    enemy_origin = m_Enemy->origin;
    VectorSub2D(enemy_origin, origin, vDelta);

    origin_ratio = VectorLength2DSquared(vDelta) * 0.64f;
    if (origin_ratio > Square(192)) {
        origin_ratio = Square(192);
    }

    for (current_node = CurrentPathNode() - 1; current_node >= LastPathNode(); current_node--) {
        VectorSub2D(enemy_origin, current_node->point, enemy_offset);

        if (VectorLength2DSquared(enemy_offset) <= origin_ratio) {
            return false;
        }

        if (DotProduct2D(enemy_offset, current_node->dir) >= 0) {
            continue;
        }

        if (DotProduct2D(enemy_offset, current_node->dir) < -current_node->dist) {
            continue;
        }

        if (Square(CrossProduct2D(enemy_offset, current_node->dir)) <= origin_ratio) {
            return false;
        }
    }

    return PathAvoidsSquadMates() != false;
}

void Actor::Cover_FindCover(bool bCheckAll)
{
    if (m_pCoverNode) {
        if (Cover_IsValid(m_pCoverNode) && Cover_SetPath(m_pCoverNode)) {
            return;
        }

        m_pCoverNode->Relinquish();
        m_pCoverNode = NULL;
    }

    if (!m_iPotentialCoverCount) {
        m_iPotentialCoverCount =
            PathManager.FindPotentialCover(this, origin, m_Enemy, m_pPotentialCoverNode, MAX_COVER_NODES);
    }

    if (!m_iPotentialCoverCount) {
        return;
    }

    PathNode *pNode;

    while (m_iPotentialCoverCount) {
        m_iPotentialCoverCount--;
        pNode                                         = m_pPotentialCoverNode[m_iPotentialCoverCount];
        m_pPotentialCoverNode[m_iPotentialCoverCount] = NULL;

        if (Cover_IsValid(pNode) && Cover_SetPath(pNode)) {
            break;
        }

        if (!bCheckAll || !m_iPotentialCoverCount) {
            return;
        }
    }

    m_pCoverNode = pNode;
    m_pCoverNode->Claim(this);
    memset(m_pPotentialCoverNode, 0, sizeof(m_pPotentialCoverNode));
    m_iPotentialCoverCount = 0;
}

void Actor::InitCover(GlobalFuncs_t *func)
{
    func->ThinkState                 = &Actor::Think_Cover;
    func->BeginState                 = &Actor::Begin_Cover;
    func->EndState                   = &Actor::End_Cover;
    func->SuspendState               = &Actor::Suspend_Cover;
    func->FinishedAnimation          = &Actor::FinishedAnimation_Cover;
    func->PassesTransitionConditions = &Actor::PassesTransitionConditions_Attack;
    func->IsState                    = &Actor::IsAttackState;
    func->PathnodeClaimRevoked       = &Actor::PathnodeClaimRevoked_Cover;
}

void Actor::Begin_Cover(void)
{
    DoForceActivate();
    m_csIdleMood = STRING_NERVOUS;
    m_csMood     = STRING_ALERT;

    if (m_pCoverNode) {
        TransitionState(ACTOR_STATE_COVER_TAKE_COVER, 0);
        return;
    }

    TransitionState(ACTOR_STATE_COVER_NEW_ENEMY, 0);

    if (level.inttime < m_iEnemyChangeTime + 200) {
        SetLeashHome(origin);

        if (AttackEntryAnimation()) {
            m_bLockThinkState = true;
            TransitionState(ACTOR_STATE_COVER_LOOP, 0);
        }
    }
}

void Actor::End_Cover(void)
{
    m_pszDebugState = "";

    if (m_pCoverNode) {
        m_pCoverNode->Relinquish();
        m_pCoverNode = NULL;
    }

    TransitionState(-1, 0);
}

void Actor::Suspend_Cover(void)
{
    if (m_pCoverNode) {
        m_pCoverNode->Relinquish();
        m_pCoverNode->MarkTemporarilyBad();
        m_pCoverNode = NULL;
    }

    TransitionState(ACTOR_STATE_COVER_FIND_COVER, 0);
}

void Actor::State_Cover_NewEnemy(void)
{
    ForwardLook();
    Cover_FindCover(true);

    if (m_pCoverNode && PathExists() && !PathComplete()) {
        Anim_RunToCover(ANIM_MODE_PATH_GOAL);
        TransitionState(ACTOR_STATE_COVER_TAKE_COVER, 0);
    } else {
        Anim_Aim();
        AimAtTargetPos();
        TransitionState(ACTOR_STATE_COVER_TARGET, 0);
    }
}

void Actor::State_Cover_FindCover(void)
{
    Anim_Aim();
    AimAtTargetPos();
    Cover_FindCover(false);

    if (m_pCoverNode) {
        if (PathExists() && !PathComplete()) {
            Anim_RunToCover(ANIM_MODE_PATH_GOAL);
            TransitionState(ACTOR_STATE_COVER_TAKE_COVER, 0);
        } else {
            TransitionState(ACTOR_STATE_COVER_TARGET, 0);
        }
    } else if (!m_iPotentialCoverCount) {
        SetThink(THINKSTATE_ATTACK, THINK_TURRET);
    }
}

void Actor::State_Cover_TakeCover(void)
{
    if (PathExists() && !PathComplete()) {
        FaceEnemyOrMotion(level.inttime - m_iStateTime);
        Anim_RunToCover(ANIM_MODE_PATH_GOAL);
        SetPathGoalEndAnim(m_bInReload ? STRING_ANIM_RUNTO_COVER_SCR : STRING_ANIM_IDLE_SCR);
    } else {
        ClearPath();
        m_eAnimMode = ANIM_MODE_NORMAL;
        TransitionState(ACTOR_STATE_COVER_FINISH_RELOADING, 0);
        State_Cover_FinishReloading();
    }
}

void Actor::State_Cover_FinishReloading(void)
{
    Weapon    *pWeapon;
    firetype_t eFireType;

    if (m_bInReload) {
        ContinueAnimation();
        AimAtTargetPos();
        return;
    }

    pWeapon = GetWeapon(WEAPON_MAIN);
    if (pWeapon) {
        eFireType = pWeapon->GetFireType(FIRE_PRIMARY);
    }

    if (pWeapon && eFireType != FT_PROJECTILE && eFireType != FT_SPECIAL_PROJECTILE
        && (m_csSpecialAttack = m_pCoverNode->GetSpecialAttack(this)) != 0) {
        SetDesiredYaw(m_pCoverNode->angles.yaw());
        SafeSetOrigin(m_pCoverNode->origin);
        DesiredAnimation(ANIM_MODE_NORMAL, m_csSpecialAttack);
        TransitionState(ACTOR_STATE_COVER_SPECIAL_ATTACK, 0);
    } else {
        if (m_pCoverNode->nodeflags & AI_DUCK) {
            Anim_Crouch();
        } else {
            Anim_Stand();
        }

        TransitionState(ACTOR_STATE_COVER_HIDE, Cover_HideTime(m_Team));

        Anim_Aim();
        AimAtTargetPos();
    }
}

void Actor::State_Cover_Target(void)
{
    DontFaceWall();

    if (AvoidingFacingWall()) {
        TransitionState(ACTOR_STATE_COVER_FIND_ENEMY, 0);
        State_Cover_FindEnemy();
        return;
    }

    Anim_Aim();
    AimAtTargetPos();

    if (level.inttime <= m_iStateTime + 300) {
        return;
    }

    if (fabs(m_DesiredYaw - angles[1]) >= 0.001f) {
        return;
    }

    if (DecideToThrowGrenade(m_vLastEnemyPos + velocity, &m_vGrenadeVel, &m_eGrenadeMode, false)) {
        SetDesiredYawDir(m_vGrenadeVel);
        DesiredAnimation(
            ANIM_MODE_NORMAL,
            m_eGrenadeMode == AI_GREN_TOSS_ROLL ? STRING_ANIM_GRENADETOSS_SCR : STRING_ANIM_GRENADETHROW_SCR
        );
        TransitionState(ACTOR_STATE_COVER_GRENADE);
    } else if (CanSeeEnemy(500) && CanShootEnemy(500)) {
        TransitionState(ACTOR_STATE_COVER_SHOOT, 0);
    } else {
        TransitionState(ACTOR_STATE_COVER_HIDE, Cover_HideTime(m_Team));
    }
}

void Actor::State_Cover_Hide(void)
{
    PathNode *pNode;
    trace_t   trace;
    bool      bCanShoot, bCanSee;

    if (m_Enemy) {
        SetEnemyPos(origin);
    }

    if (!m_pCoverNode) {
        TransitionState(ACTOR_STATE_COVER_FIND_COVER, 0);
        State_Cover_FindCover();
        return;
    }

    m_csSpecialAttack = m_pCoverNode->GetSpecialAttack(this);
    MPrintf("special: %d", m_csSpecialAttack);

    if (m_csSpecialAttack) {
        SetDesiredYaw(m_pCoverNode->angles.yaw());
        SafeSetOrigin(m_pCoverNode->origin);
        DesiredAnimation(ANIM_MODE_NORMAL, m_csSpecialAttack);
        TransitionState(ACTOR_STATE_COVER_SPECIAL_ATTACK, 0);
        return;
    }

    bCanSee   = CanSeeEnemy(500);
    bCanShoot = CanShootEnemy(500);

    if (bCanSee && bCanShoot) {
        vec2_t vDelta;

        VectorSub2D(m_Enemy->origin, origin, vDelta);
        if (VectorLength2DSquared(vDelta) * 0.75f > Square(DotProduct2D(vDelta, orientation[0]))) {
            TransitionState(ACTOR_STATE_COVER_SHOOT);
        } else {
            TransitionState(ACTOR_STATE_COVER_TARGET);
        }

        State_Cover_Shoot();
        return;
    }

    if (bCanShoot || bCanShoot) {
        m_pCoverNode->Relinquish();
        m_pCoverNode->MarkTemporarilyBad();
        m_pCoverNode = NULL;

        TransitionState(ACTOR_STATE_COVER_FIND_ENEMY);
        State_Cover_FindEnemy();
        return;
    }

    if (m_Team == TEAM_AMERICAN) {
        if (level.inttime >= m_iLastFaceDecideTime + 1000) {
            m_iLastFaceDecideTime = level.inttime;
            pNode                 = PathManager.FindCornerNodeForExactPath(this, m_Enemy, m_fLeash + m_fMaxDistance);

            if (!pNode) {
                ForwardLook();
                Anim_Stand();
                m_PotentialEnemies.FlagBadEnemy(m_Enemy);
                UpdateEnemy(-1);
                return;
            }

            SetDesiredYawDest(pNode->m_PathPos);
        }

        Anim_Aim();
    } else {
        if (level.inttime >= m_iLastFaceDecideTime + 1000) {
            m_iLastFaceDecideTime = level.inttime;

            trace = G_Trace(
                EyePosition(), vec_zero, vec_zero, m_vLastEnemyPos + eyeposition, this, MASK_LOOK, false, "State_Cover"
            );

            pNode = PathManager.FindCornerNodeForWall(origin, m_vLastEnemyPos, this, 0.0f, trace.plane.normal);

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

    if (level.inttime <= m_iStateTime) {
        return;
    }

    pNode = (PathNode *)G_FindRandomSimpleTarget(m_pCoverNode->Target());

    m_pCoverNode->Relinquish();
    m_pCoverNode = NULL;

    if (!pNode) {
        Anim_Stand();
        TransitionState(ACTOR_STATE_COVER_FIND_ENEMY, rand() & 0x7FF);
        return;
    }

    assert(pNode->IsSubclassOfPathNode());

    if (pNode->IsSubclassOfPathNode() && !pNode->IsClaimedByOther(this)) {
        SetPath(pNode, "Actor::State_Cover_Target", 0);

        if (PathExists()) {
            m_pCoverNode = pNode;
            pNode->Claim(this);
            Anim_RunToDanger(ANIM_MODE_PATH_GOAL);
            TransitionState(ACTOR_STATE_COVER_SEARCH_NODE, 0);
        }
    }
}

void Actor::State_Cover_Shoot(void)
{
    if (m_bNeedReload) {
        Cover_FindCover(true);

        if (m_pCoverNode) {
            Anim_RunToCover(ANIM_MODE_PATH_GOAL);
            FaceEnemyOrMotion(0);
            TransitionState(ACTOR_STATE_COVER_TAKE_COVER, 0);
            return;
        }
    }

    Anim_Shoot();
    AimAtTargetPos();

    // FIXME: debug build only?
    if (level.inttime > m_iStateTime + 10000) {
        gi.cvar_set("g_monitornum", va("%i", entnum));
        assert(!"anim/shoot.scr took over 10 seconds");
        Com_Error(
            ERR_DROP, "anim/shoot.scr took over 10 seconds, entnum = %i, targetname = %s", entnum, TargetName().c_str()
        );
    }
}

void Actor::State_Cover_Grenade(void)
{
    GenericGrenadeTossThink();
}

void Actor::State_Cover_SpecialAttack(void)
{
    ForwardLook();

    assert(m_pCoverNode);
    if (!m_pCoverNode) {
        TransitionState(ACTOR_STATE_COVER_FIND_ENEMY, 0);
        State_Cover_FindEnemy();
        return;
    }

    if (mbBreakSpecialAttack) {
        if (m_pCoverNode) {
            m_pCoverNode->Relinquish();
            m_pCoverNode = NULL;
        }

        TransitionState(ACTOR_STATE_COVER_FIND_ENEMY, 0);
        State_Cover_FindEnemy();
        return;
    }

    if (level.inttime >= m_iLastEnemyPosChangeTime + level.intframetime || !m_csSpecialAttack) {
        m_csSpecialAttack = m_pCoverNode->GetSpecialAttack(this);
    }

    if (!m_csSpecialAttack) {
        TransitionState(ACTOR_STATE_COVER_FIND_ENEMY, 0);
        State_Cover_FindEnemy();
        return;
    }

    if (m_csSpecialAttack > STRING_ANIM_HIGHWALL_SCR || m_csSpecialAttack < STRING_ANIM_LOWWALL_SCR) {
        SetDesiredYaw(m_pCoverNode->angles.yaw());
    } else {
        AimAtTargetPos();
    }

    DesiredAnimation(ANIM_MODE_NORMAL, m_csSpecialAttack);
}

void Actor::State_Cover_FindEnemy(void)
{
    if (m_Team == TEAM_AMERICAN) {
        ForwardLook();
        Anim_Stand();
    } else {
        AimAtTargetPos();
        Anim_Aim();
        DontFaceWall();

        if (CanSeeEnemy(200) && !AvoidingFacingWall()) {
            TransitionState(ACTOR_STATE_COVER_TARGET);
        } else if (!AvoidingFacingWall() && level.inttime <= m_iStateTime + 500) {
            return;
        }
    }

    SetPathWithLeash(m_vLastEnemyPos, NULL, 0);
    ShortenPathToAvoidSquadMates();

    if (PathExists() && !PathComplete() && PathAvoidsSquadMates()) {
        TransitionState(ACTOR_STATE_COVER_HUNT_ENEMY);
    } else {
        m_bTurretNoInitialCover = true;
        SetThink(THINKSTATE_ATTACK, THINK_TURRET);
    }
}

void Actor::State_Cover_SearchNode(void)
{
    ForwardLook();

    if (CanSeeEnemy(200)) {
        Anim_Aim();
        AimAtTargetPos();
        TransitionState(ACTOR_STATE_COVER_TARGET, 0);
        return;
    }

    if (PathExists() && !PathComplete()) {
        FaceEnemyOrMotion(level.inttime - m_iStateTime);
        Anim_RunToDanger(ANIM_MODE_PATH_GOAL);
        return;
    }

    Anim_Aim();
    AimAtTargetPos();

    if (level.inttime > m_iStateTime + 3000) {
        TransitionState(ACTOR_STATE_COVER_FIND_COVER, 0);
    }
}

void Actor::State_Cover_HuntEnemy(void)
{
    FaceEnemyOrMotion(level.inttime - m_iStateTime);
    MovePathWithLeash();

    if (PathExists() && !PathComplete()) {
        if (CanSeeEnemy(300)) {
            TransitionState(ACTOR_STATE_COVER_TARGET, 0);
        }
        return;
    }

    TransitionState(ACTOR_STATE_COVER_FIND_ENEMY, rand() & 0x7FF);

    if (m_pCoverNode) {
        m_pCoverNode->Relinquish();
        m_pCoverNode = NULL;
    }
}

void Actor::State_Cover_FakeEnemy(void)
{
    AimAtTargetPos();
    Anim_Aim();

    if (level.inttime >= m_iStateTime) {
        SetThinkState(THINKSTATE_IDLE, THINKLEVEL_IDLE);
    }
}

void Actor::Think_Cover(void)
{
    if (!RequireThink()) {
        return;
    }

    UpdateEyeOrigin();
    NoPoint();
    UpdateEnemy(500);

    if (m_State == ACTOR_STATE_COVER_LOOP) {
        ContinueAnimation();
    } else {
        m_bLockThinkState = false;
        if (m_Enemy) {
            if (m_State == ACTOR_STATE_COVER_FAKE_ENEMY) {
                TransitionState(ACTOR_STATE_COVER_NEW_ENEMY, 0);
            }
        } else {
            if (m_State != ACTOR_STATE_COVER_FAKE_ENEMY) {
                TransitionState(ACTOR_STATE_COVER_FAKE_ENEMY, (rand() & 0x7FF) + 1000);
            }
        }

        switch (m_State) {
        case ACTOR_STATE_COVER_START:
            m_pszDebugState = "NewEnemy";
            State_Cover_NewEnemy();
            break;
        case ACTOR_STATE_COVER_FIND_COVER:
            m_pszDebugState = "FindCover";
            State_Cover_FindCover();
            break;
        case ACTOR_STATE_COVER_TAKE_COVER:
            m_pszDebugState = "TakeCover";
            State_Cover_TakeCover();
            break;
        case ACTOR_STATE_COVER_FINISH_RELOADING:
            m_pszDebugState = "FinishReloading";
            State_Cover_FinishReloading();
            break;
        case ACTOR_STATE_COVER_SPECIAL_ATTACK:
            m_pszDebugState = "SpecialAttack";
            State_Cover_SpecialAttack();
            break;
        case ACTOR_STATE_COVER_FIND_ENEMY:
            m_pszDebugState = "FindEnemy";
            State_Cover_FindEnemy();
            break;
        case ACTOR_STATE_COVER_SEARCH_NODE:
            m_pszDebugState = "SearchNode";
            State_Cover_SearchNode();
            break;
        case ACTOR_STATE_COVER_TARGET:
            m_pszDebugState = "Target";
            State_Cover_Target();
            break;
        case ACTOR_STATE_COVER_HIDE:
            m_pszDebugState = "Hide";
            State_Cover_Hide();
            break;
        case ACTOR_STATE_COVER_SHOOT:
            m_pszDebugState = "Shoot";
            State_Cover_Shoot();
            break;
        case ACTOR_STATE_COVER_GRENADE:
            m_pszDebugState = "Grenade";
            State_Cover_Grenade();
            break;
        case ACTOR_STATE_COVER_HUNT_ENEMY:
            m_pszDebugState = "HuntEnemy";
            State_Cover_HuntEnemy();
            break;
        case ACTOR_STATE_COVER_FAKE_ENEMY:
            m_pszDebugState = "FakeEnemy";
            State_Cover_FakeEnemy();
            break;
        default:
            Com_Printf("Actor::Think_Cover: invalid think state %i\n", m_State);
            assert(0);
            break;
        }

        if (!CheckForTransition(THINKSTATE_GRENADE, THINKLEVEL_IDLE)) {
            CheckForTransition(THINKSTATE_BADPLACE, THINKLEVEL_IDLE);
        }
    }

    if (m_State == ACTOR_STATE_COVER_HIDE || m_State == ACTOR_STATE_COVER_FIND_ENEMY
        || m_State == ACTOR_STATE_COVER_TARGET || m_State == ACTOR_STATE_COVER_SHOOT) {
        PostThink(false);
    } else {
        PostThink(true);
    }
}

void Actor::FinishedAnimation_Cover(void)
{
    if (m_State == ACTOR_STATE_COVER_SHOOT) {
        if (m_Enemy && !m_Enemy->IsDead() && CanSeeEnemy(500) && CanShootEnemy(500)) {
            TransitionState(ACTOR_STATE_COVER_SHOOT, 0);
        } else {
            TransitionState(ACTOR_STATE_COVER_FIND_COVER, 0);
        }
    } else if (m_State == ACTOR_STATE_COVER_GRENADE || m_State == ACTOR_STATE_COVER_LOOP) {
        TransitionState(ACTOR_STATE_COVER_FIND_COVER, 0);
    }
}

void Actor::PathnodeClaimRevoked_Cover(void)
{
    TransitionState(ACTOR_STATE_COVER_FIND_COVER, 0);
}
