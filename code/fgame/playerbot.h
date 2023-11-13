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
// playerbot.h: Multiplayer bot system.

#ifndef __PLAYERBOT_H__
#define __PLAYERBOT_H__

#include "player.h"
#include "navigate.h"
#include "actorpath.h"

#define MAX_BOT_FUNCTIONS 5

typedef struct nodeAttract_s {
    float             m_fRespawnTime;
    AttractiveNodePtr m_pNode;
} nodeAttract_t;

class PlayerBot : public Player
{
    struct botfunc_t {
        bool (PlayerBot::*CheckCondition)(void);
        void (PlayerBot::*BeginState)(void);
        void (PlayerBot::*EndState)(void);
        void (PlayerBot::*ThinkState)(void);
    };

private:
    static botfunc_t botfuncs[];

    // Paths
    ActorPath                  m_Path;
    Vector                     m_vTargetPos;
    Vector                     m_vCurrentGoal;
    Vector                     m_vLastValidDir;
    Vector                     m_vLastValidGoal;
    bool                       m_bPathing;
    bool                       m_bTempAway;
    bool                       m_bAimPath;
    bool                       m_bDeltaMove;
    int                        m_iTempAwayTime;
    int                        m_iCheckPathTime;
    AttractiveNodePtr          m_pPrimaryAttract;
    float                      m_fAttractTime;
    Container<nodeAttract_t *> m_attractList;

    // States
    int               m_iCuriousTime;
    int               m_iAttackTime;
    Vector            m_vLastCuriousPos;
    Vector            m_vOldEnemyPos;
    Vector            m_vLastEnemyPos;
    Vector            m_vLastDeathPos;
    SafePtr<Sentient> m_pEnemy;

    // Input
    usercmd_t  m_botCmd;
    usereyes_t m_botEyes;

    // Direction
    Vector m_vTargetAng;
    Vector m_vCurrentAng;
    Vector m_vAngSpeed;
    float  m_fYawSpeedMult;

    // States
    int               m_StateCount;
    unsigned int      m_StateFlags;
    ScriptThreadLabel m_RunLabel;

private:
    void CheckAttractiveNodes(void);
    void MoveThink(void);
    void TurnThink(void);
    void CheckEndPos(void);
    void CheckJump(void);
    void CheckUse(void);

    void State_DefaultBegin(void);
    void State_DefaultEnd(void);
    void State_Reset(void);

    static void InitState_Idle(botfunc_t *func);
    bool        CheckCondition_Idle(void);
    void        State_BeginIdle(void);
    void        State_EndIdle(void);
    void        State_Idle(void);

    static void InitState_Curious(botfunc_t *func);
    bool        CheckCondition_Curious(void);
    void        State_BeginCurious(void);
    void        State_EndCurious(void);
    void        State_Curious(void);

    static void InitState_Attack(botfunc_t *func);
    bool        CheckCondition_Attack(void);
    void        State_BeginAttack(void);
    void        State_EndAttack(void);
    void        State_Attack(void);

    static void InitState_Grenade(botfunc_t *func);
    bool        CheckCondition_Grenade(void);
    void        State_BeginGrenade(void);
    void        State_EndGrenade(void);
    void        State_Grenade(void);

    static void InitState_Weapon(botfunc_t *func);
    bool        CheckCondition_Weapon(void);
    void        State_BeginWeapon(void);
    void        State_EndWeapon(void);
    void        State_Weapon(void);

    void CheckStates(void);

public:
    CLASS_PROTOTYPE(PlayerBot);

    PlayerBot();

    static void Init(void);

    void GetEyeInfo(usereyes_t *eyeinfo);
    void GetUsercmd(usercmd_t *ucmd);
    void SetTargetAngles(Vector vAngles);

    void UpdateBotStates(void);
    void CheckReload(void);

    void AimAt(Vector vPos);
    void AimAtAimNode(void);

    void AvoidPath(
        Vector vPos,
        float  fAvoidRadius,
        Vector vPreferredDir = vec_zero,
        float *vLeashHome    = NULL,
        float  fLeashRadius  = 0.0f
    );
    void MoveNear(Vector vNear, float fRadius, float *vLeashHome = NULL, float fLeashRadius = 0.0f);
    void MoveTo(Vector vPos, float *vLeashHome = NULL, float fLeashRadius = 0.0f);
    bool MoveToBestAttractivePoint(int iMinPriority = 0);

    bool CanMoveTo(Vector vPos);
    bool MoveDone(void);
    bool IsMoving(void);
    void ClearMove(void);

    void NoticeEvent(Vector vPos, int iType, Entity *pEnt, float fDistanceSquared, float fRadiusSquared);
    void ClearEnemy(void);

    void SendCommand(const char *text);

    void setAngles(Vector angles) override;
    void updateOrigin(void) override;

    void Spawned(void) override;

    void Killed(Event *ev);
    void GotKill(Event *ev);
    void EventStuffText(Event *ev);
};

#endif /* playerbot.h */
