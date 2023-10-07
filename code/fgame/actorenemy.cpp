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

// actorenemy.cpp:

#include "actorenemy.h"
#include "worldspawn.h"
#include "sentient.h"
#include "actor.h"

ActorEnemy::ActorEnemy()
{
	m_pEnemy = NULL;
	m_vLastKnownPos = vec_zero;
}

ActorEnemy::~ActorEnemy()
{
	//no need
}

float ActorEnemy::UpdateVisibility
	(
	Actor *pSelf,
	bool *pbInFovAndRange,
	bool *pbVisible
	)
{
	float fLMRF, fFrameTime;

	fFrameTime = level.time - m_fLastLookTime;

	m_fLastLookTime = level.time;

	fLMRF = UpdateLMRF(pSelf, pbInFovAndRange, pbVisible);
	if (fLMRF < 8.0)
	{
		m_fVisibility += fFrameTime / fLMRF;
	}
	else
	{
		if (m_fVisibility < 1.0)
		{
			m_fVisibility -= fFrameTime * 0.25;
			if (m_fVisibility < 0.0)
				m_fVisibility = 0.0;
		}
	}

	m_fTotalVisibility = m_fVisibility + GetEnemy()->m_fPlayerSightLevel;
	if (m_fTotalVisibility > 1)
	{
		m_fVisibility = 1;
		m_fTotalVisibility = 1;
	}
	return m_fTotalVisibility;
}

int ActorEnemy::UpdateThreat
	(
	Actor *pSelf
	)
{
	//FIXME: macro
	static float fRangeThreatSquared[15];
	static int iWeaponThreat[7][5];
	int iWeapon;
	int iZone;
	Vector vDelta;

	m_iThreat = 0;
	m_fCurrentRangeSquared = 1e38f;

	if (m_pEnemy->m_bIsDisguised || m_fTotalVisibility < 1)
		return m_iThreat;

	m_iThreat = 10000;
	if (m_bVisible == true)
		m_iThreat = 10500;

	vDelta = m_pEnemy->origin - pSelf->origin;
	m_fCurrentRangeSquared = vDelta.lengthSquared();

	if (m_fCurrentRangeSquared >= 65536.0)
	{
		if (m_fCurrentRangeSquared >= 589824.0)
		{
			if (m_fCurrentRangeSquared >= 1638400.0)
			{
				if (m_fCurrentRangeSquared >= 4194304.0)
					iZone = 4;
				else
					iZone = 3;
			}
			else
			{
				iZone = 2;
			}
		}
		else
		{
			iZone = 1;
		}
	}
	else
	{
		iZone = 0;
	}

	Weapon *pEnemyWeapon = m_pEnemy->GetActiveWeapon(WEAPON_MAIN);
	iWeapon = 0;

	if (pEnemyWeapon)
	{
		int iWeapClass = pEnemyWeapon->GetWeaponClass();
		if (iWeapClass & WEAPON_CLASS_PISTOL)
		{
			iWeapon = 1;
		}
		else if (iWeapClass & WEAPON_CLASS_RIFLE)
		{
			iWeapon = 2;
		}
		else if (iWeapClass & WEAPON_CLASS_SMG)
		{
			iWeapon = 3;
		}
		else if (iWeapClass & WEAPON_CLASS_MG)
		{
			iWeapon = 4;
		}
		else if(iWeapClass & WEAPON_CLASS_GRENADE)
		{
			iWeapon = 5;
		}
		else
		{
			iWeapon = 6;
		}
	}

	int i=0;
	if (m_fCurrentRangeSquared > 4194304)
	{
		for (; i < 15 && fRangeThreatSquared[i] > m_fCurrentRangeSquared; i++)
		{
		}
	}

	m_iThreat += iWeaponThreat[iZone][iWeapon] + m_pEnemy->m_iThreatBias + i;

	float fMinSafeDistSquared = 1.21 * pSelf->m_fMinDistanceSquared + 16384;

	if (m_fCurrentRangeSquared < fMinSafeDistSquared)
	{
		m_iThreat -= (sqrt(m_fCurrentRangeSquared / fMinSafeDistSquared) * 500) + 500;
	}

	Vector vLine = m_vLastKnownPos - pSelf->origin;
	float fDot = DotProduct2D(vLine, m_vLastKnownPos);
	for (Sentient  *pSquadMate = m_pEnemy->m_pNextSquadMate; pSquadMate != m_pEnemy; pSquadMate = pSquadMate->m_pNextSquadMate)
	{
		if (fDot > DotProduct2D(vLine, pSquadMate->origin))
		{
			m_iThreat -= 4;
		}
	}

	Actor *pAEnemy = (Actor *)m_pEnemy.Pointer();
	if (pAEnemy->IsSubclassOfActor() && pAEnemy->m_ThinkState == THINKSTATE_PAIN)
	{
		m_iThreat -= 2;
	}

	fDot = Vector::Dot(vDelta, pSelf->orientation[0]);
	if (fDot <= 0 )
	{
		if (m_fCurrentRangeSquared * 0.5 > Square(fDot))
		{
			m_iThreat++;
		}
	}
	else
	{
		if (m_fCurrentRangeSquared * 0.5 >= Square(fDot))
		{
			if (m_fCurrentRangeSquared * 0.5 > Square(fDot))
			{
				m_iThreat++;
			}
		}
	}
	int iEnemyDiscount = m_pEnemy->m_iAttackerCount;

	if (m_pEnemy == pSelf->m_Enemy)
	{
		iEnemyDiscount -= 2;
		if (level.inttime < pSelf->m_iEnemyChangeTime + 1000)
			m_iThreat += 5;
	}

	if (iEnemyDiscount > 4)
		iEnemyDiscount = 4;
	m_iThreat -= iEnemyDiscount;

	if (m_pEnemy == pSelf->m_pLastAttacker)
	{
		m_iThreat += 5;
	}
	if (m_pEnemy == pSelf->m_FavoriteEnemy)
		m_iThreat += 250;
	return m_iThreat;
}

Sentient *ActorEnemy::GetEnemy
	(
	void
	) const
{
	return m_pEnemy;
}

float ActorEnemy::GetVisibility
	(
	void
	) const
{
	return m_fVisibility;
}

int ActorEnemy::GetThreat
	(
	void
	) const
{
	return m_iThreat;
}

float ActorEnemy::GetRangeSquared
	(
	void
	) const
{
	return m_fCurrentRangeSquared;
}

float ActorEnemy::UpdateLMRF
	(
	Actor *pSelf,
	bool *pbInFovAndRange,
	bool *pbVisible
	)
{
	//FIXME: variable names, I did my best
	Vector vDelta;
	float fFarPlane, fLMRF, /*fMinSightTime,*/ fFovScale, fForward, /*fNormalizedRange,*/ fRangeScale, fRange/*, fMaxRange*/ ;
	float fTmp1, fTmp2, fTmp3;

	*pbInFovAndRange = false;
	*pbVisible = false;

	vDelta = pSelf->EyePosition() - GetEnemy()->origin;

	fFarPlane = world->farplane_distance;
	
	fRange = pSelf->m_fSight;

	if (fFarPlane > 0)
	{
		fRange = fFarPlane * 0.828;
		if (pSelf->m_fSight <= fRange)
			fRange = pSelf->m_fSight;
	}


	if (vDelta.lengthXYSquared() > Square(fRange))
	{
		return 8.0;
	}

	fForward = vDelta.lengthXY();

	if (-DotProduct2D(vDelta, pSelf->m_vEyeDir) < 0)
	{
		return 8.0;
	}

	fTmp2 = 128.0 - DotProduct2D(vDelta, pSelf->m_vEyeDir);
	
	if (fForward * pSelf->m_fFovDot > fTmp2)
	{
		return 8.0;
	}

	*pbInFovAndRange = true;

	if (!pSelf->CanSee(m_pEnemy, 0, 0, false))
	{
		return 8.0;
	}
	
	*pbVisible = true;
	fTmp1 = fForward + 128.0;
	*pbVisible = true;
	fTmp3 = fTmp1 / fTmp2;
	fRangeScale = fForward
		/ fRange
		* (((fForward / fRange * 16.0 + -16.0) * (fForward / fRange) + -1.0) * (fForward / fRange) + 7.0)
		/ 3.0
		* GetEnemy()->stealthMovementScale;
	fFovScale = (1 / fTmp3 * -1.3 - (pSelf->m_fFovDot * 0.2 - 1.5)) / (1.0 - pSelf->m_fFovDot);
	fLMRF = g_ai_noticescale->value * pSelf->m_fNoticeTimeScale * (fTmp3 * fRangeScale + fTmp3 * fRangeScale);
	if (fFovScale > fLMRF)
		fLMRF = fFovScale;
	return fLMRF;
}


ActorEnemySet::ActorEnemySet()
{
	m_pCurrentEnemy = NULL;
	m_iCheckCount = 0;

	if (m_pCurrentEnemy)
	{
		//delete m_pCurrentEnemy;
		m_pCurrentEnemy = NULL;
	}
	
	m_iCurrentThreat = 0;
	m_fCurrentVisibility = 0.0;
}

ActorEnemySet::~ActorEnemySet()
{

}

ActorEnemy *ActorEnemySet::AddPotentialEnemy
	(
	Sentient *pEnemy
	)
{
	ActorEnemy NewEnemy;
	
	if (pEnemy->IsDead() || pEnemy->m_iThreatBias == THREATBIAS_IGNOREME)
	{
		return NULL;
	}

	for (int i = 0; i < m_Enemies.NumObjects(); i++)
	{
		ActorEnemy *pActorEnemy = &m_Enemies[i];
		if (pActorEnemy->m_pEnemy == pEnemy)
		{
			pActorEnemy->m_iAddTime = level.inttime;
			return pActorEnemy;
		}
	}

	NewEnemy.m_fVisibility = 0.0;
	NewEnemy.m_fTotalVisibility = 0.0;
	NewEnemy.m_iAddTime = level.inttime;
	NewEnemy.m_fLastLookTime = level.time;
	NewEnemy.m_iThreat = 0;

	NewEnemy.m_pEnemy = pEnemy;

	NewEnemy.m_fCurrentRangeSquared = 1e38f;

	NewEnemy.m_iLastSightChangeTime = 0;
	NewEnemy.m_vLastKnownPos = vec_zero;

	NewEnemy.m_bVisible = false;



	return &m_Enemies[m_Enemies.AddObject(NewEnemy)-1];
}

void ActorEnemySet::FlagBadEnemy
	(
	Sentient *pEnemy
	)
{
	ActorEnemy *pActorEnemy;
	for (int i = 0; i < m_Enemies.NumObjects(); i++)
	{
		pActorEnemy = &m_Enemies[i];
		if (pActorEnemy->m_pEnemy == pEnemy)
		{
			break;
		}

		if (i+1 == m_Enemies.NumObjects())
			return;
	}

	pActorEnemy->m_iThreat = 0;
	pActorEnemy->m_fVisibility = 0.0;
	pActorEnemy->m_fTotalVisibility = 0.0;
	pActorEnemy->m_fLastLookTime = level.time;

	pActorEnemy->m_bVisible = false;
	pActorEnemy->m_iLastSightChangeTime = level.inttime;

	if (pEnemy ==  m_pCurrentEnemy)
	{
		if (m_pCurrentEnemy)
		{
			//delete m_pCurrentEnemy;
			m_pCurrentEnemy = NULL;
		}
		m_fCurrentVisibility = 0.0;
		m_iCurrentThreat = 0;
	}
}

void ActorEnemySet::CheckEnemies
	(
	Actor *pSelf
	)
{
	float fRangeSquared;
	bool bVisible;
	bool bInFovAndRange;
	//int nChecked;
	//int iThreat;
	float fVisibility;

	ActorEnemy *pActorEnemy;


	for (int i = 1; i <= m_Enemies.NumObjects();i++)
	{
		pActorEnemy = &m_Enemies[i-1];
		if (!pActorEnemy->m_pEnemy 
			|| pActorEnemy->m_pEnemy->m_Team == pSelf->m_Team 
			|| pActorEnemy->m_pEnemy->IsDead() 
			|| level.inttime > pActorEnemy->m_iAddTime + 10000
			|| pActorEnemy->m_pEnemy->m_iThreatBias == THREATBIAS_IGNOREME)
		{
			m_Enemies.RemoveObjectAt(i);
			i--;//decrease i in order to not miss next object in container.
		}
	}

	if (!m_Enemies.NumObjects())
	{
		m_iCurrentThreat = 0;
		if (m_pCurrentEnemy)
		{
			//delete m_pCurrentEnemy;
			m_pCurrentEnemy = NULL;
		}
	}
	else
	{
		for (int i = 0; i < m_Enemies.NumObjects(); i++)
		{
			m_iCheckCount++;
			if (m_iCheckCount > m_Enemies.NumObjects())
				m_iCheckCount = 1;

			pActorEnemy = &m_Enemies[m_iCheckCount-1];

			fVisibility = pActorEnemy->UpdateVisibility(pSelf, &bInFovAndRange, &bVisible);
			if (fVisibility <= 0.0)
			{
				if (pActorEnemy->m_pEnemy == m_pCurrentEnemy)
				{
					m_iCurrentThreat = 0;
					m_pCurrentEnemy = NULL;
					m_fCurrentVisibility = 0.0;
				}
			}
			else
			{
				if (fVisibility < m_fCurrentVisibility)
				{
					if (pActorEnemy->m_pEnemy == m_pCurrentEnemy)
					{
						m_fCurrentVisibility = fVisibility;
					}
				}
				else
				{
					m_pCurrentEnemy = pActorEnemy->m_pEnemy;
					m_fCurrentVisibility = fVisibility;
				}

				if (g_showawareness->integer)
				{
					Com_Printf(
						"ent #%3i: enemy #%i: awareness = %5.1f%%, threat = %i\n",
						pSelf->entnum,
						pActorEnemy->m_pEnemy->entnum,
						(fVisibility * 100.0),
						0);
				}
			}

			if (bVisible)
			{
				if (!pActorEnemy->m_bVisible)
				{
					pActorEnemy->m_bVisible = true;
					pActorEnemy->m_iLastSightChangeTime = level.inttime;
				}
				pActorEnemy->m_vLastKnownPos = pActorEnemy->m_pEnemy->origin;
			}
			else if (pActorEnemy->m_bVisible)
			{
				pActorEnemy->m_bVisible = false;
				pActorEnemy->m_iLastSightChangeTime = level.inttime;
			}

			if (bInFovAndRange)
			{
				break;
			}
		}

		if (m_pCurrentEnemy && m_pCurrentEnemy->IsDead())
		{
			m_pCurrentEnemy = NULL;
			m_iCurrentThreat = 0;
			m_fCurrentVisibility = 0.0;
		}

		m_iCurrentThreat = 0;

		fRangeSquared = 1e37f;

		if (m_fCurrentVisibility >= 1)
		{
			for (int i = 1;i <= m_Enemies.NumObjects(); i++)
			{
				pActorEnemy = &m_Enemies[i-1];
				pActorEnemy->UpdateThreat(pSelf);
				if (m_iCurrentThreat < pActorEnemy->m_iThreat || (m_iCheckCount == pActorEnemy->m_iThreat && fRangeSquared > pActorEnemy->m_fCurrentRangeSquared))
				{
					m_iCurrentThreat = pActorEnemy->m_iThreat;
					m_pCurrentEnemy = pActorEnemy->m_pEnemy;
					fRangeSquared = pActorEnemy->m_fCurrentRangeSquared;
				}
			}
		}

		if ((!m_pCurrentEnemy || !m_pCurrentEnemy->m_bIsDisguised) && m_iCurrentThreat <= 0)
		{
			m_pCurrentEnemy = NULL;
			m_iCurrentThreat = 0;
			m_fCurrentVisibility = 0.0;
		}
	}
}

Sentient *ActorEnemySet::GetCurrentEnemy
	(
	void
	) const
{
	return m_pCurrentEnemy;
}

float ActorEnemySet::GetCurrentVisibility
	(
	void
	) const
{
	return m_fCurrentVisibility;
}

int ActorEnemySet::GetCurrentThreat
	(
	void
	) const
{
	return m_iCurrentThreat;
}

qboolean ActorEnemySet::IsEnemyConfirmed
	(
	void
	) const
{
	// not found in ida
	return false;
}

bool ActorEnemySet::HasAlternateEnemy
	(
	void
	) const
{
	ActorEnemy *pActorEnemy;
	for (int i = 0; i < m_Enemies.NumObjects(); i++)
	{
		pActorEnemy = &m_Enemies[i];
		if (pActorEnemy->m_pEnemy != m_pCurrentEnemy)
		{
			return true;
		}
	}
	return false;
}

void ActorEnemySet::RemoveAll
	(
	void
	)
{
	m_Enemies.ClearObjectList();
	m_iCheckCount = 0;
	if (m_pCurrentEnemy)
	{
		//delete m_pCurrentEnemy;
		m_pCurrentEnemy = NULL;
	}
	m_fCurrentVisibility = 0.0;
	m_iCurrentThreat = 0;
}

void ActorEnemySet::ConfirmEnemy
	(
	Actor *pSelf,
	Sentient *pEnemy
	)
{
	pSelf->m_bEnemyIsDisguised = false;
	ActorEnemy *pActorEnemy = ActorEnemySet::AddPotentialEnemy(pEnemy);
	if (pActorEnemy)
	{
		pActorEnemy->m_fVisibility = 1.0;
		pActorEnemy->m_fTotalVisibility = 1.0;
		pActorEnemy->m_vLastKnownPos = pEnemy->origin;
		if (m_fCurrentVisibility < 1.0)
		{
			m_iCurrentThreat = pActorEnemy->UpdateThreat(pSelf);
			m_fCurrentVisibility = 1.0;
			m_pCurrentEnemy = pEnemy;
		}
	}
}

void ActorEnemySet::ConfirmEnemyIfCanSeeSharerOrEnemy
	(
	Actor *pSelf,
	Actor *pSharer,
	Sentient *pEnemy
	)
{
	pSelf->m_bEnemyIsDisguised = false;
	ActorEnemy *pActorEnemy = ActorEnemySet::AddPotentialEnemy(pEnemy);
	if (pActorEnemy)
	{
		if (pActorEnemy->m_fTotalVisibility > 0)
		{
			pActorEnemy->m_vLastKnownPos = pEnemy->origin;
			return;
		}
		if (!pActorEnemy->m_bVisible)
		{
			if (!pSelf->CanSee(
				pSharer,
				pSelf->m_bSilent ? 90.0f : 0,
				0.828 * world->farplane_distance, false))
				return;
		}
		pActorEnemy->m_fVisibility = 1.0;
		pActorEnemy->m_fTotalVisibility = 1.0;
		pActorEnemy->m_vLastKnownPos = pEnemy->origin;
		if (m_fCurrentVisibility < 1.0)
		{
			m_iCurrentThreat = pActorEnemy->UpdateThreat(pSelf);
			m_fCurrentVisibility = 1.0;
			m_pCurrentEnemy = pEnemy;
		}
	}
}

bool ActorEnemySet::CaresAboutPerfectInfo
	(
	Sentient *pEnemy
	)
{
	ActorEnemy *pActorEnemy = ActorEnemySet::AddPotentialEnemy(pEnemy);
	if (pActorEnemy)
	{
		if (pActorEnemy->m_fTotalVisibility < 1.0 || (pEnemy->origin - pActorEnemy->m_vLastKnownPos).lengthSquared() > 262144)
		{
			return true;
		}
	}
	return false;
}
