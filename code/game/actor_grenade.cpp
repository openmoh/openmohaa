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

// actor_grenade.cpp

#include "actor.h"
#include "weaputils.h"

void Actor::InitGrenade
	(
	GlobalFuncs_t *func
	)

{
	func->ThinkState					= &Actor::Think_Grenade;
	func->BeginState					= &Actor::Begin_Grenade;
	func->EndState						= &Actor::End_Grenade;
	func->ResumeState					= &Actor::Resume_Grenade;
	func->SuspendState					= &Actor::End_Grenade;
	func->PassesTransitionConditions	= &Actor::PassesTransitionConditions_Grenade;
	func->FinishedAnimation				= &Actor::FinishedAnimation_Grenade;
	func->IsState						= &Actor::IsGrenadeState;
}

bool Actor::Grenade_Acquire
	(
	eGrenadeState eNextState,
	const_str csReturnAnim
	)

{
	bool bRetVal = false;
	Vector vDest;
	if (m_bGrenadeBounced)
	{
		bRetVal = true;
		vDest = origin - m_vGrenadePos;
		vDest = vDest * 16 + m_vGrenadePos;
		SetPath(vDest, NULL, 0, NULL, 0.0);
	}
	if (PathExists())
	{
		if (PathComplete())
		{
			m_bHasDesiredLookAngles = false;
			if (m_pGrenade->velocity.lengthXYSquared() >= 1024)
			{
				Anim_Stand();
			}
			else
			{
				m_pGrenade->velocity = vec_zero;

				m_eNextAnimMode = 1;
				m_eGrenadeState = eNextState;
				m_bNextForceStart = false;
				m_csNextAnimString = csReturnAnim;
			}
		}
		else
		{
			Anim_RunToCasual(3);

			vec2_t delta;
			VectorSub2D(origin, m_vGrenadePos, delta);

			if (VectorLength2DSquared(delta) > 1024.0)
			{
				FaceMotion();
			}
			else
			{
				{
					vec2_t facedir;
					facedir[0] = m_vGrenadePos[0] - origin[0];
					facedir[1] = m_vGrenadePos[1] - origin[1];
					if (facedir[0] != 0 || facedir[1] != 0)
					{
						SetDesiredYawDir(facedir);
					}

				}
			}
		}
	}
	else
	{
		m_bGrenadeBounced = true;
		m_eGrenadeState = AI_GRENSTATE_FLEE;
		Grenade_Flee();
		bRetVal = false;
	}

	return bRetVal;
}

void Actor::Grenade_Flee
	(
	void
	)

{
	//float origin_ratio;
	float fMinCloseDistSquared;
	float fCosAngle;
	float fSinAngle;
	float fAngle;
	vec2_t vDirAway;
	vec2_t vDirPreferred;

	if (m_bGrenadeBounced)
	{
		fAngle = (rand() - 0x3FFFFFFF) * -0.00000000146291807926716;

		fSinAngle = sin(fAngle);
		fCosAngle = cos(fAngle);

		VectorSub2D(origin, m_vGrenadePos, vDirAway);

		vDirPreferred[0] = vDirAway[0] * fCosAngle - fSinAngle * vDirAway[1];
		vDirPreferred[1] = vDirAway[0] * fSinAngle + vDirAway[1] * fCosAngle;

		FindPathAway(m_vGrenadePos, vDirPreferred, 512);

		if (PathExists() && !PathComplete())
		{
			fMinCloseDistSquared = VectorLength2DSquared(vDirAway) * 0.63999999;
		
			if (fMinCloseDistSquared < 1024)
				fMinCloseDistSquared = 0;

			vec2_t grenade_offset;
			for (auto current_node = CurrentPathNode(); current_node >= LastPathNode(); current_node--)
			{
				VectorSub2D(m_vGrenadePos, current_node->point, grenade_offset);
				if (current_node->dist > 0 && DotProduct2D(grenade_offset, current_node->dir) <= current_node->dist)
				{
					if (Square(CrossProduct2D(grenade_offset, current_node->dir)) < fMinCloseDistSquared)
					{
						ClearPath();
						break;
					}
				}
			}
		}
		m_bGrenadeBounced = false;
	}

	if (PathExists() && !PathComplete())
	{
		Sentient *pOwner = NULL;
		if (m_pGrenade && m_pGrenade->IsSubclassOfProjectile())
			pOwner = ((Projectile *)m_pGrenade.Pointer())->GetOwner();

		if (pOwner && pOwner->m_Team == m_Team)
			Anim_RunTo(3);
		else
			Anim_RunToFlee(3);
		FaceMotion();
	}
	else
	{

		if ((origin - m_vGrenadePos).lengthXYSquared() >= 100352 
			|| !G_SightTrace(
			centroid,
			vec_zero,
			vec_zero,
			m_vGrenadePos,
			this,
			m_pGrenade,
			33819417,
			0,
			"Actor::Grenade_Flee"))
		{
			m_eGrenadeState = AI_GRENSTATE_FLEE_SUCCESS;
			Anim_Attack();
			AimAtTargetPos();
		}
		else
		{
			m_bHasDesiredLookAngles = false;
			m_eGrenadeState = AI_GRENSTATE_FLEE_FAIL;
			Anim_Cower();
		}
	}
}

void Actor::Grenade_ThrowAcquire
	(
	void
	)

{
	if (!Grenade_Acquire(AI_GRENSTATE_THROW, STRING_ANIM_GRENADERETURN_SCR)
		&& !CanGetGrenadeFromAToB(
			m_vGrenadePos,
			m_vLastEnemyPos,
			true,
			&m_vGrenadeVel,
			&m_eGrenadeMode))
	{
		m_bGrenadeBounced = true;
		m_eGrenadeState = AI_GRENSTATE_FLEE;
		Grenade_Flee();
	}
}

void Actor::Grenade_Throw
	(
	void
	)

{
	m_bHasDesiredLookAngles = false;

	SetDesiredYawDir(m_vGrenadeVel);

	ContinueAnimation();
}

void Actor::Grenade_KickAcquire
	(
	void
	)

{
	Vector vFace = vec_zero;
	if (!Grenade_Acquire(AI_GRENSTATE_KICK, STRING_ANIM_GRENADEKICK_SCR))
	{
		VectorSub2D(m_vGrenadePos, origin, vFace);
		if (CanKickGrenade(m_vGrenadePos, m_vLastEnemyPos, vFace, &m_vGrenadeVel))
		{
			m_vKickDir = Vector(m_vGrenadeVel[0], m_vGrenadeVel[1], 0);
			
			VectorNormalizeFast(m_vKickDir);
		}
		else
		{
			m_bGrenadeBounced = true;
			m_eGrenadeState = AI_GRENSTATE_FLEE;
			Grenade_Flee();
		}
	}
}

void Actor::Grenade_Kick
	(
	void
	)

{
	m_bHasDesiredLookAngles = false;
	ContinueAnimation();
}

void Actor::Grenade_MartyrAcquire
	(
	void
	)

{
	Vector vDest;
	if (m_bGrenadeBounced)
	{
		m_bGrenadeBounced = false;

		vDest = origin - m_vGrenadePos;
		VectorNormalizeFast(vDest);

		vDest = vDest * 88 + m_vGrenadePos;

		SetPath(vDest, NULL, 0, NULL, 0.0);
	}

	if (PathExists())
	{
		if (PathComplete())
		{
			m_bHasDesiredLookAngles = false;

			vDest = vec_zero;
			m_pGrenade->velocity = vec_zero;

			//weird ? m_pGrenade->velocity is vec_zero ???
			if (m_pGrenade->velocity.lengthXYSquared() < 1024)
			{
				m_pGrenade->velocity = vec_zero;

				m_eGrenadeState = AI_GRENSTATE_MARTYR;
				m_iStateTime = level.inttime;
				Grenade_Martyr();
			}
		}
		else
		{
			Anim_RunToCasual(3);
			m_csPathGoalEndAnimScript = STRING_ANIM_GRENADEMARTYR_SCR;
			if ((origin-m_vGrenadePos).lengthXYSquared() > 16384)
			{
				FaceMotion();
			}
			else
			{
				if (m_vGrenadePos - origin != vec_zero)
				{
					SetDesiredYawDir(m_vGrenadePos - origin);
				}
			}
		}
	}
	else
	{
		m_bGrenadeBounced = true;
		m_eGrenadeState = AI_GRENSTATE_FLEE;
		Grenade_Flee();
	}
}

void Actor::Grenade_Martyr
	(
	void
	)

{
	if (m_pGrenade && level.inttime >= (m_iStateTime + 1000) - 0.5)
	{
		Projectile *m_pPGrenade = (Projectile *)m_pGrenade.Pointer();

		m_pPGrenade->m_bHurtOwnerOnly = true;
		m_pPGrenade->owner = entnum;
		
	}

	m_bHasDesiredLookAngles = false;
	ContinueAnimation();
}

void Actor::Grenade_Wait
	(
	void
	)

{
	if (rand() & 0xF)
	{
		Anim_Cower();
	}
	else
	{
		Anim_Stand();
		
		Grenade_NextThinkState();
	}
}

void Actor::Grenade_NextThinkState
	(
	void
	)

{
	if (m_Enemy && !(m_Enemy->IsSubclassOfActor()))
		SetThinkState(THINKSTATE_ATTACK, THINKLEVEL_NORMAL);
	else
		SetThinkState(THINKSTATE_IDLE, THINKLEVEL_NORMAL);
}

void Actor::Grenade_EventAttach
	(
	Event *ev
	)

{
	if (m_pGrenade)
	{
		eGrenadeTossMode eMode;
		Vector vVel;
		int tagnum = gi.Tag_NumForName(edict->tiki, "tag_weapon_right");
		if (tagnum >= 0)
		{
			vVel = vec_zero;
			m_pGrenade->attach(
				entnum,
				tagnum,
				qtrue,
				vec3_origin);
		}
		m_pGrenade->avelocity = vec3_origin;

		if (CanGetGrenadeFromAToB(
			origin,
			m_vLastEnemyPos,
			true,
			&vVel,
			&eMode))
		{
			m_vGrenadeVel = vVel;
			m_eGrenadeMode = eMode;
		}

		SetDesiredYawDir(m_vGrenadeVel);
	}
}

void Actor::Grenade_EventDetach
	(
	Event *ev
	)

{
	if (m_pGrenade)
	{
		m_pGrenade->detach();
		m_pGrenade->setOrigin(GrenadeThrowPoint(origin, orientation[0], m_eGrenadeMode == AI_GREN_KICK ? STRING_ANIM_GRENADEKICK_SCR : STRING_ANIM_GRENADERETURN_SCR));
		m_pGrenade->velocity = m_vGrenadeVel;

		m_pGrenade->edict->r.ownerNum = edict->s.number;
		m_pGrenade->groundentity = NULL;
		
	}
}

void Actor::Begin_Grenade
	(
	void
	)

{
	DoForceActivate();
	m_csMood = STRING_ALERT;
	m_csIdleMood = STRING_NERVOUS;

	if (m_pGrenade)
	{
		if (m_pGrenade->enemy)
		{
			m_eGrenadeState = AI_GRENSTATE_FLEE;
		//LABEL_4:
			Grenade_Flee();
			return;
		}

		if (m_pGrenade->edict->r.ownerNum == entnum)
		{
			m_eGrenadeState = AI_GRENSTATE_FLEE;
			Grenade_Flee();
			return;
		}
		bool bHasThrowTarget = true;
		float fDistSquared;
		Vector vDelta;
		if (!m_Enemy)
		{
			Sentient *pEnemy = (Sentient *)G_GetEntity(m_pGrenade->edict->r.ownerNum);

			if (pEnemy && pEnemy->m_Team != m_Team)
			{
				SetEnemyPos(pEnemy->origin);
			}
			else
			{
				bHasThrowTarget = false;
			}
		}
		if (!bHasThrowTarget)
		{
			m_eGrenadeState = AI_GRENSTATE_FLEE;
			Grenade_Flee();
			return;
		}
		vDelta = m_vGrenadePos - origin;
		vDelta.z = 0;
		fDistSquared = vDelta.lengthXYSquared();
		if (fDistSquared >= 65536)
		{
			m_eGrenadeState = AI_GRENSTATE_FLEE;
			Grenade_Flee();
			return;
		}

		if (fDistSquared > 16384 && GrenadeWillHurtTeamAt(m_vGrenadePos))
		{
			m_pGrenade->enemy = this;

			m_eGrenadeState = AI_GRENSTATE_MARTYR_ACQUIRE;
			Grenade_MartyrAcquire();
			return;
		}

		if (GrenadeWillHurtTeamAt(m_vLastEnemyPos))
		{
			m_eGrenadeState = AI_GRENSTATE_FLEE;
			Grenade_Flee();
			return;
		}

		if (CanKickGrenade(m_vGrenadePos, m_vLastEnemyPos, vDelta, &m_vGrenadeVel))
		{
			m_eGrenadeMode = AI_GREN_KICK;

			m_vKickDir = Vector(m_vGrenadeVel[0], m_vGrenadeVel[1], 0);
			VectorNormalizeFast(m_vKickDir);

			m_pGrenade->enemy = this; 
			
			m_eGrenadeState = AI_GRENSTATE_KICK_ACQUIRE;
			PostponeEvent(EV_Projectile_Explode, 0.25);
			Grenade_KickAcquire();
			return;
		}

		if (!CanGetGrenadeFromAToB(m_vGrenadePos, m_vLastEnemyPos, true, &m_vGrenadeVel, &m_eGrenadeMode))
		{
			m_eGrenadeState = AI_GRENSTATE_FLEE;
			Grenade_Flee();
			return;
		}

		m_pGrenade->enemy = this;

		m_eGrenadeState = AI_GRENSTATE_THROW_ACQUIRE;
		PostponeEvent(EV_Projectile_Explode, 0.75);
		if (!Grenade_Acquire(AI_GRENSTATE_THROW, STRING_ANIM_GRENADERETURN_SCR)
			&& !Actor::CanGetGrenadeFromAToB(m_vGrenadePos, m_vLastEnemyPos, true, &m_vGrenadeVel, &m_eGrenadeMode))
		{
			m_bGrenadeBounced = true;
			m_eGrenadeState = AI_GRENSTATE_FLEE;
			Grenade_Flee();
			return;
		}
	}
	else
	{
		if (m_Enemy && !m_Enemy->IsSubclassOfActor())
		{
			SetThinkState(THINKSTATE_ATTACK, THINKLEVEL_NORMAL);
		}
		else if (m_Team == TEAM_AMERICAN)
		{
			SetThinkState(THINKSTATE_IDLE, THINKLEVEL_NORMAL);
		}
		else
		{
			if (!IsTeamMate((Sentient*)G_GetEntity(0)))
				ForceAttackPlayer();
		}
	}
}

void Actor::End_Grenade
	(
	void
	)

{
	m_pszDebugState = "";
}

void Actor::Resume_Grenade
	(
	void
	)

{
	if (m_pGrenade)
		Begin_Grenade();
	else
		Grenade_NextThinkState();
}

void Actor::Think_Grenade
	(
	void
	)

{
	if (m_bEnableEnemy)
		UpdateEnemy(200);
	m_pszDebugState = "";
	NoPoint();

	if (level.inttime - m_iFirstGrenadeTime > 8000)
	{
		Anim_Stand();
		Grenade_NextThinkState();
		PostThink(false);
		return;
	}

	if (!m_pGrenade)
	{
		if (m_eGrenadeState && m_eGrenadeState != AI_GRENSTATE_FLEE_FAIL)
		{
			// weird ? no such thing as 8 ?
			// FIXME?
			if (m_eGrenadeMode == (eGrenadeTossMode)8)
				Anim_Attack();
			else
				Anim_Stand();
			Grenade_NextThinkState();
		}
		else
		{
			m_pszDebugState = "Wait";
			Grenade_Wait();
		}
		PostThink(false);
		return;
	}

	switch (m_eGrenadeState)
	{
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
		m_pszDebugState = "Kick";
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
		m_pszDebugState = "***Invalid***";
		char assertStr[16317] = { 0 };
		strcpy(assertStr, "\"invalid grenade state\"\n\tMessage: ");
		Q_strcat(assertStr, sizeof(assertStr), DumpCallTrace("thinkstate = %i", m_State));
		assert(!assertStr);
		break;
	}

	PostThink(false);
}

void Actor::FinishedAnimation_Grenade
	(
	void
	)

{
	switch (m_eGrenadeState)
	{
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
		char assertStr[16317] = { 0 };
		strcpy(assertStr, "\"invalid grenade state in FinishedAnimation()\"\n\tMessage: ");
		Q_strcat(assertStr, sizeof(assertStr), DumpCallTrace("state = %i", m_eGrenadeState));
		assert(!assertStr);
		break;
	}
}
