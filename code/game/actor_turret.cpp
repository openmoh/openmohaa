/*
===========================================================================
Copyright (C) 2015 the OpenMoHAA team

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

void Actor::InitTurret
	(
	GlobalFuncs_t *func
	)

{
	func->ThinkState					= &Actor::Think_Turret;
	func->BeginState					= &Actor::Begin_Turret;
	func->EndState						= &Actor::End_Turret;
	func->SuspendState					= &Actor::Suspend_Turret;
	func->RestartState					= NULL;
	func->FinishedAnimation				= &Actor::FinishedAnimation_Turret;
	func->PassesTransitionConditions	= &Actor::PassesTransitionConditions_Attack;
	func->PostShoot						= &Actor::InterruptPoint_Turret;
	func->ReceiveAIEvent				= &Actor::ReceiveAIEvent_Turret;
	func->IsState						= &Actor::IsAttackState;
	func->PathnodeClaimRevoked			= &Actor::PathnodeClaimRevoked_Turret;
}

void Actor::Begin_Turret
	(
	void
	)

{
	DoForceActivate();
	m_csMood = STRING_ALERT;

	ClearPath();

	if (m_Enemy)
	{
		m_State = 110;
		m_iStateTime = level.inttime;
	}
	else
	{
		m_State = 109;
		m_iStateTime = level.inttime + (rand() & 0x7FF) + 250;
	}
}

void Actor::End_Turret
	(
	void
	)

{
	if (m_pCoverNode && m_State != 111)
	{
		m_pCoverNode->Relinquish();
		m_pCoverNode = NULL;
	}
	m_State = -1;
	m_iStateTime = level.inttime;
}

void Actor::Suspend_Turret
	(
	void
	)

{
	if (!m_Enemy)
	{
		m_State = 110;
		m_iStateTime = level.inttime;
	}
	else
	{
		if (m_State <= 108)
		{
			SetEnemyPos(m_Enemy->origin);
			AimAtEnemyBehavior();
			m_State = 113;
			m_iStateTime = level.inttime;
		}
	}
}

void Actor::Think_Turret
	(
	void
	)

{

	//FIXME: weird, appears to be looping here ? O.o
	if (!RequireThink())
	{
		return;
	}

	UpdateEyeOrigin();
	NoPoint();
	UpdateEnemy(200);

	// FIXME: stub
	STUB();
}

void Actor::FinishedAnimation_Turret
	(
	void
	)

{
	if (m_State <= 108)
	{
		Turret_SelectState();
	}
}

void Actor::ReceiveAIEvent_Turret
	(
	vec3_t event_origin,
	int iType,
	Entity *originator,
	float fDistSquared,
	float fMaxDistSquared
	)

{
	if (iType == 2)
	{
		if (m_Enemy && fDistSquared <= 16384)
		{
			auto pCoverNode = m_pCoverNode;
			Cover_FindCover(true);
			if (m_pCoverNode)
			{
				m_State = 111;
				m_iStateTime = level.inttime;
				SetThink(4, 2);
			}
			else if (pCoverNode)
			{
				m_pCoverNode = pCoverNode;
				m_pCoverNode->Claim(this);
			}
		}
	}
	else
	{
		DefaultReceiveAIEvent(
			origin,
			iType,
			originator,
			fDistSquared,
			fMaxDistSquared);
	}
}

void Actor::InterruptPoint_Turret
	(
	void
	)

{
	if (m_Enemy && !Turret_TryToBecomeCoverGuy() && m_State == 100)
	{
		m_iStateTime = level.inttime;
		Turret_SelectState();
	}
}

void Actor::PathnodeClaimRevoked_Turret
	(
	void
	)

{
	if (m_Enemy == NULL)
	{
		m_State = 110;
	}
	else
	{
		SetEnemyPos(m_Enemy->origin);
		AimAtEnemyBehavior();
		m_State = 113;
	}
	m_iStateTime = level.inttime;
}

bool Actor::Turret_IsRetargeting
	(
	void
	) const

{
	return m_State <= 120;
}

bool Actor::Turret_DecideToSelectState
	(
	void
	)

{
	switch (m_State)
	{
	case 100:
		if (level.inttime > m_iStateTime + 5000)
		{
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

void Actor::Turret_SelectState
	(
	void
	)

{
	vec2_t vDelta;
	float fDistSquared;
	if (!m_Enemy)
	{
		m_State = 109;
		m_iStateTime = level.inttime + rand() & 0x7fff + 250;
		return;
	}

	VectorSub2D(origin, m_vHome, vDelta);
	fDistSquared = VectorLength2DSquared(vDelta);
	
	if (m_State == 104 && fDistSquared > m_fLeashSquared * 0.64 + 64.0)
	{
		if (PathExists() && !PathComplete())
			return;
	}
	else if (fDistSquared <= m_fLeashSquared * 1.21 + 256.0)
	{
		m_iRunHomeTime = 0;
	}
	else if (!m_iRunHomeTime)
	{
		m_iRunHomeTime = level.inttime + (rand() & 0xFFF) + 1000;
	}
	else if (level.inttime >= m_iRunHomeTime)
	{
		m_iRunHomeTime = 0;

		ClearPath();
		SetPath(m_vHome, "", 0, NULL, 0.0);
		ShortenPathToAvoidSquadMates();

		if (PathExists())
		{
			if (!PathComplete())
			{
				m_State = 104;
				m_iStateTime = level.inttime;
				return;
			}
		}
		else
		{
			Com_Printf(
				"^~^~^ (entnum %i, radnum %d, targetname '%s') cannot reach his leash home\n",
				entnum,
				radnum,
				targetname.c_str());
		}
	}

	VectorSub2D(origin, m_vHome, vDelta);
	fDistSquared = VectorLength2DSquared(vDelta);

	if (m_State == 105)
	{
		if (m_fMinDistanceSquared * 2.25 > fDistSquared)
			return;
	}

	if (m_fMinDistanceSquared > fDistSquared)
	{
		ClearPath();
		m_State = 105;
		m_iStateTime = level.inttime;
		return;
	}

	if (fDistSquared > m_fMaxDistanceSquared)
	{
		bool bSmthing = false;
		if (m_Team == TEAM_GERMAN)
		{
			if ((m_Enemy->origin-m_vHome).lengthSquared() >= Square(m_fLeash + m_fMaxDistance) && !Actor::CanSeeEnemy(200))
				bSmthing = true;
		}
		if (bSmthing)
		{
			ClearPath();
			m_State = 112;
		}
		else
		{
			if (m_State == 106)
				return;
			ClearPath();
			m_State = 106;
		}
		m_iStateTime = level.inttime;
		return;
	}

	if (DecideToThrowGrenade(m_Enemy->velocity + m_vLastEnemyPos, &m_vGrenadeVel, &m_eGrenadeMode))
	{
		m_bNextForceStart = false;
		m_YawAchieved = false;
		m_DesiredYaw = vectoyaw(this->m_vGrenadeVel);
		m_eNextAnimMode = 1;
		m_State = 107;
		m_csNextAnimString = (m_eGrenadeMode == AI_GREN_TOSS_ROLL) ? STRING_ANIM_GRENADETOSS_SCR : STRING_ANIM_GRENADETHROW_SCR;
		m_iStateTime = level.inttime;
		return;
	}

	if (m_State != 100 && m_State != 103 && m_State != 112)
	{
		ClearPath();
		m_State = 100;
		m_iStateTime = level.inttime;
	}
}

bool Actor::Turret_CheckRetarget
	(
	void
	)

{
	if (level.inttime < m_iStateTime + 5000 || level.inttime < m_iLastHitTime + 5000)
		return false;

	Turret_BeginRetarget();

	return true;
}

bool Actor::Turret_TryToBecomeCoverGuy
	(
	void
	)

{
	auto pCoverNode = this->m_pCoverNode;
	Cover_FindCover(true);
	if (m_pCoverNode)
	{
		m_State = 111;
		m_iStateTime = level.inttime;
		SetThink(4, 2);
		return true;
	}
	else
	{
		if (pCoverNode)
		{
			m_pCoverNode = pCoverNode;
			m_pCoverNode->Claim(this);
		}
		return false;
	}
}

void Actor::Turret_BeginRetarget
	(
	void
	)

{
	SetEnemyPos(m_Enemy->origin);
	AimAtEnemyBehavior();

	m_State = 113;
	m_iStateTime = level.inttime;

}

void Actor::Turret_NextRetarget
	(
	void
	)

{
	vec2_t vDelta;
	float fDistSquared;


	m_State++;
	if (m_State > 120)
	{
		VectorSub2D(origin, m_vHome, vDelta);
		fDistSquared = VectorLength2DSquared(vDelta);
		if (fDistSquared < m_fLeashSquared
			|| (SetPath(
				m_vHome,
				"",
				0,
				NULL,
				0.0),
				ShortenPathToAvoidSquadMates(),
				!PathExists())
			|| PathComplete())
		{
			if (m_Team == TEAM_AMERICAN)
			{
				if (!CanSeeEnemy(200))
				{
					m_PotentialEnemies.FlagBadEnemy(m_Enemy);
					UpdateEnemy(-1);
				}
				if (!m_Enemy)
				{
					Anim_Stand();
					return;
				}
			}
			else
			{
				if (!CanSeeEnemy(200))
				{
					m_State = 112;
					m_iStateTime = level.inttime;
					State_Turret_Wait();
					return;
				}
				m_pszDebugState = "Retarget->Combat";
			}
			m_State = 100;
			m_iStateTime = level.inttime;
			State_Turret_Combat();
			return;
		}
		m_State = 104;
		m_iStateTime = level.inttime;
		SetPath(m_vHome, "", 0, NULL, 0.0);
		ShortenPathToAvoidSquadMates();
		if (!PathExists() || PathComplete())
		{
			Com_Printf(
				"^~^~^ (entnum %i, radnum %i, targetname '%s') cannot reach his leash home\n",
				entnum,
				radnum,
				targetname.c_str());
			m_pszDebugState = "home->combat";
			State_Turret_Combat();
		}
		else
		{
			FaceMotion();
			Anim_RunToInOpen(2);
		}
	}
	else
	{
		m_iStateTime = level.inttime;
	}
}

void Actor::Turret_SideStep
	(
	int iStepSize,
	vec3_t vDir
	)

{
	AimAtEnemyBehavior();
	//v3 = iStepSize;
	StrafeToAttack(iStepSize, vDir);
	if (PathExists() && !PathComplete() && PathAvoidsSquadMates()
		|| (Actor::StrafeToAttack(-iStepSize, vDir), SimpleActor::PathExists())
		&& !PathComplete()
		&& PathAvoidsSquadMates())
	{
		m_State = 101;
		m_iStateTime = level.inttime;
	}
	else
	{
		Turret_NextRetarget();
	}
}

void Actor::State_Turret_Combat
	(
	void
	)

{
	if (CanSeeEnemy(200))
	{
		ClearPath();
		Anim_Attack();
		SetDesiredLookDir(mTargetPos - EyePosition());
		AimAtAimNode();
		Turret_CheckRetarget();
		return;
	}
	if (!PathExists() || PathComplete() || !PathAvoidsSquadMates())
	{
		SetPathWithLeash(m_vLastEnemyPos, "", 0);
		ShortenPathToAvoidSquadMates();
	}
	if (!PathExists() || PathComplete() || !PathAvoidsSquadMates())
	{
		FindPathNearWithLeash(m_vLastEnemyPos, 4.0 * m_fMinDistanceSquared);
		if (!ShortenPathToAttack(0.0))
			ClearPath();
		ShortenPathToAvoidSquadMates();
	}
	if (PathExists() && !PathComplete() && PathAvoidsSquadMates())
	{
		m_pszDebugState = "combat->move";
		if (MovePathWithLeash())
		{
			Turret_CheckRetarget();
			return;
		}
		m_pszDebugState = "combat->move->aim";
	}
	else
	{
		m_pszDebugState = "combat->chill";
	}

	SetEnemyPos(m_Enemy->origin);
	AimAtEnemyBehavior();
	m_State = 113;
	m_iStateTime = level.inttime;
}

void Actor::State_Turret_Reacquire
	(
	void
	)

{
	Sentient *v1; // ecx
	float v2; // ST08_4
	float v3; // ST0C_4
	Sentient *v4; // ecx
	float v5; // ST08_4
	float v6; // ST0C_4

	if (PathExists() && !PathComplete())
	{
		if (CanMovePathWithLeash())
		{
			Anim_RunToInOpen(3);
			FaceEnemyOrMotion(level.inttime - m_iStateTime);
		}
		else
		{
			Turret_BeginRetarget();
		}
	}
	else
	{
		m_pszDebugState = "Retarget->Cheat";
		SetEnemyPos(m_Enemy->origin);
		m_State = 100;
		m_iStateTime = level.inttime;
		State_Turret_Combat();
	}
}

void Actor::State_Turret_TakeSniperNode
	(
	void
	)

{
	if (PathExists() && !PathComplete())
	{
		FaceMotion();
		Anim_RunToDanger(3);
	}
	else
	{
		AimAtEnemyBehavior();
		m_State = 103;
		m_iStateTime = level.inttime;
	}
}

void Actor::State_Turret_SniperNode
	(
	void
	)

{
	AimAtAimNode();
	Anim_Sniper();
	if (Turret_CheckRetarget())
	{
		m_pCoverNode->Relinquish();
		m_pCoverNode->MarkTemporarilyBad();
		m_pCoverNode = NULL;
	}
}

bool Actor::State_Turret_RunHome
	(
	bool bAttackOnFail
	)

{

	SetPath(this->m_vHome, "", 0, NULL, 0.0);
	ShortenPathToAvoidSquadMates();
	if (!PathExists() || PathComplete())
	{
		Com_Printf(
			"^~^~^ (entnum %i, radnum %i, targetname '%s') cannot reach his leash home\n",
			entnum,
			radnum,
			targetname.c_str());
		if (bAttackOnFail)
		{
			m_pszDebugState = "home->combat";
			State_Turret_Combat();
		}
		return false;
	}
	else
	{
		FaceMotion();
		Anim_RunToInOpen(2);
		return true;
	}
}

void Actor::State_Turret_RunAway
	(
	void
	)

{
	if (!PathExists() || PathComplete())
	{
		FindPathAwayWithLeash(m_vLastEnemyPos, origin-m_Enemy->origin, 1.5 * m_fMinDistance);
	}
	if (!PathExists() || PathComplete())
	{
		m_pszDebugState = "runaway->combat";
		State_Turret_Combat();
		return;
	}
	if (!CanMovePathWithLeash())
	{
		m_pszDebugState = "runaway->leash->combat";
		State_Turret_Combat();
		return;
	}
	Anim_RunAwayFiring(2);
	FaceEnemyOrMotion(level.inttime - m_iStateTime);
}

void Actor::State_Turret_Charge
	(
	void
	)

{
	SetPathWithLeash(this->m_vLastEnemyPos, "", 0);
	ShortenPathToAvoidSquadMates();
	if (!PathExists())
	{
		m_pszDebugState = "charge->near";
		FindPathNearWithLeash(m_vLastEnemyPos, m_fMaxDistanceSquared);
		if (!ShortenPathToAttack(0))
			ClearPath();
	}
	if (!PathExists() || PathComplete() || !PathAvoidsSquadMates())
	{
		ClearPath();
		if (CanSeeEnemy(500))
		{
			m_pszDebugState = "charge->combat";
			State_Turret_Combat();
			return;
		}
		m_bHasDesiredLookAngles = false;
		m_pszDebugState = "charge->chill";
		Anim_Idle();
		if (m_Team != TEAM_AMERICAN)
		{
			//v1 = &this->m_PotentialEnemies;
			if (!m_PotentialEnemies.HasAlternateEnemy())
			{

				if (m_Enemy)
					Turret_CheckRetarget();
				return;
			}
		}
		m_PotentialEnemies.FlagBadEnemy(m_Enemy);
		UpdateEnemy(-1);
		if (m_Enemy)
			Turret_CheckRetarget();
		return;
	}
	if (!MovePathWithLeash())
	{
		m_pszDebugState = "charge->leash->combat";
		m_State = 100;
		m_iStateTime = level.inttime;
		State_Turret_Combat();
		return;
	}
}

void Actor::State_Turret_Grenade
	(
	void
	)

{
	GenericGrenadeTossThink();
}

void Actor::State_Turret_FakeEnemy
	(
	void
	)

{
	AimAtAimNode();
	Anim_Aim();
	if (level.inttime >= m_iStateTime)
		SetThinkState(THINKSTATE_IDLE, 0);
}

void Actor::State_Turret_Wait
	(
	void
	)

{
	PathNode *pNode;
	if (CanSeeEnemy(500) || CanShootEnemy(500))
	{
		bool bSmth;

		pNode = m_pCoverNode;
		Cover_FindCover(true);

		if (m_pCoverNode)
		{
			m_State = 111;
			m_iStateTime = level.inttime;
			SetThink(THINKSTATE_ATTACK, 2);
			bSmth = true;
		}
		else
		{
			if (pNode)
			{
				m_pCoverNode = pNode;
				m_pCoverNode->Claim(this);
			}
			bSmth = false;
		}
		if (bSmth)
		{
			m_pszDebugState = "Wait->CoverInstead";
			ContinueAnimation();
		}
		else
		{
			m_pszDebugState = "Wait->Combat";
			m_State = 100;
			m_iStateTime = level.inttime;
			State_Turret_Combat();
		}
	}
	else
	{
		if (level.inttime >= m_iLastFaceDecideTime + 1500)
		{
			this->m_iLastFaceDecideTime = level.inttime + (rand() & 0x1FF);
			
			
			
			
			
			
			pNode = PathManager.FindCornerNodeForExactPath(this, m_Enemy, 0);

			if (pNode)
			{
				if (pNode->m_PathPos - origin != vec_zero)
				{
					m_YawAchieved = false;
					m_DesiredYaw = vectoyaw(pNode->m_PathPos - origin);
				}
				m_eDontFaceWallMode = 6;
			}
			else
			{
				AimAtAimNode();
				DontFaceWall();
			}
		}
		if (m_eDontFaceWallMode > 8)
			Anim_Aim();
		else
			Anim_Stand();
	}
}

void Actor::State_Turret_Retarget_Sniper_Node
	(
	void
	)

{
	PathNode *pSniperNode;
	bool bTryAgain;

	AimAtEnemyBehavior();
	if (m_pCoverNode)
	{
		m_pCoverNode->Relinquish();
		m_pCoverNode = NULL;
	}
	pSniperNode = FindSniperNodeAndSetPath(&bTryAgain);
	if (pSniperNode)
	{
		m_pCoverNode = pSniperNode;
		pSniperNode->Claim(this);
		m_State = 102;
		m_iStateTime = level.inttime;
		if (PathExists() && !PathComplete())
		{
			FaceMotion();
			Anim_RunToDanger(3);
		}
		else
		{
			AimAtEnemyBehavior();
			m_State = 103;
			m_iStateTime = level.inttime;
		}
	}
	else if (bTryAgain)
	{
		ContinueAnimation();
	}
	else
	{
		Turret_NextRetarget();
	}
}

void Actor::State_Turret_Retarget_Path_Exact
	(
	void
	)

{
	AimAtEnemyBehavior();
	SetPathWithLeash(m_vLastEnemyPos, "", 0);
	if (ShortenPathToAttack(128)
		&& (ShortenPathToAvoidSquadMates(), PathExists()))
	{
		m_State = 101;
		m_iStateTime = level.inttime;
	}
	else
	{
		Turret_NextRetarget();
	}
}

void Actor::State_Turret_Retarget_Path_Near
	(
	void
	)

{
	AimAtEnemyBehavior();
	FindPathNearWithLeash(m_vLastEnemyPos, m_fMinDistanceSquared);
	if (ShortenPathToAttack(128))
	{
		m_State = 101;
		m_iStateTime = level.inttime;
	}
	else
	{
		Turret_NextRetarget();
	}
}

void Actor::State_Turret_Retarget_Step_Side_Small
(
	void
)

{
	int iRand; // esi

	iRand = (rand() & 64) - 32;
	AimAtEnemyBehavior();
	StrafeToAttack(iRand, orientation[1]);
	if (PathExists() && !PathComplete() && PathAvoidsSquadMates()
		|| (Actor::StrafeToAttack(-iRand, orientation[1]),
			PathExists())
		&& !PathComplete()
		&& PathAvoidsSquadMates())
	{
		m_State = 101;
		m_iStateTime = level.inttime;
	}
	else
	{
		Turret_NextRetarget();
	}
}

void Actor::State_Turret_Retarget_Step_Side_Medium
	(
	void
	)

{
	int iRand; // esi

	iRand = (rand() & 256) - 128;
	AimAtEnemyBehavior();
	StrafeToAttack(iRand, orientation[1]);
	if (PathExists() && !PathComplete() && PathAvoidsSquadMates()
		|| (Actor::StrafeToAttack(-iRand, orientation[1]),
			PathExists())
		&& !PathComplete()
		&& PathAvoidsSquadMates())
	{
		m_State = 101;
		m_iStateTime = level.inttime;
	}
	else
	{
		Turret_NextRetarget();
	}
}

void Actor::State_Turret_Retarget_Step_Side_Large
	(
	void
	)

{
	int iRand; // esi

	iRand = (rand() & 512) - 256;
	AimAtEnemyBehavior();
	StrafeToAttack(iRand, orientation[1]);
	if (PathExists() && !PathComplete() && PathAvoidsSquadMates()
		|| (Actor::StrafeToAttack(-iRand, orientation[1]),
			PathExists())
		&& !PathComplete()
		&& PathAvoidsSquadMates())
	{
		m_State = 101;
		m_iStateTime = level.inttime;
	}
	else
	{
		Turret_NextRetarget();
	}
}

void Actor::State_Turret_Retarget_Step_Face_Medium
	(
	void
	)

{
	int iRand; // esi

	iRand = (rand() & 256) - 128;
	AimAtEnemyBehavior();
	StrafeToAttack(iRand, orientation[0]);
	if (PathExists() && !PathComplete() && PathAvoidsSquadMates()
		|| (Actor::StrafeToAttack(-iRand, orientation[0]),
			PathExists())
		&& !PathComplete()
		&& PathAvoidsSquadMates())
	{
		m_State = 101;
		m_iStateTime = level.inttime;
	}
	else
	{
		Turret_NextRetarget();
	}
}

void Actor::State_Turret_Retarget_Step_Face_Large
	(
	void
	)

{
	int iRand; // esi

	iRand = (rand() & 512) - 256;
	AimAtEnemyBehavior();
	StrafeToAttack(iRand, orientation[0]);
	if (PathExists() && !PathComplete() && PathAvoidsSquadMates()
		|| (Actor::StrafeToAttack(-iRand, orientation[0]),
			PathExists())
		&& !PathComplete()
		&& PathAvoidsSquadMates())
	{
		m_State = 101;
		m_iStateTime = level.inttime;
	}
	else
	{
		Turret_NextRetarget();
	}
}
