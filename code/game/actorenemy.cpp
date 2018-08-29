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
#include "world.h"
#include "sentient.h"
#include "actor.h"

ActorEnemy::ActorEnemy()
{
	//no need
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
	static float fRangeThreatSquared[15];
	static int iWeaponThreat[7][5];

	m_iThreat = 0;
	m_fCurrentRangeSquared = 1e38;

	if (m_pEnemy->m_bIsDisguised || m_fTotalVisibility < 1)
		return m_iThreat;

	m_iThreat = 10000;
	if (m_bVisible == true)
		m_iThreat = 10500;


	// FIXME: stub
	STUB();
	return NULL;
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
	float fFarPlane, fLMRF = 8, fMinSightTime, fFovScale, fForward, fNormalizedRange, fRangeScale, fRange, fMaxRange;
	float fTmp1, fTmp2, fTmp3;

	*pbInFovAndRange = false;
	*pbVisible = false;

	vDelta = pSelf->origin - GetEnemy()->origin;

	fFarPlane = world->farplane_distance;
	
	fRange = pSelf->m_fSight;

	if (fFarPlane > 0)
	{
		fRange = fFarPlane * 0.828;
		if (pSelf->m_fSight <= fRange)
			fRange = pSelf->m_fSight;
	}


	if (Square(fRange) < vDelta.lengthXY(true))
	{
		return fLMRF;
	}

	fForward = vDelta.lengthXY();

	if (-DotProduct2D(vDelta, pSelf->m_vEyeDir) < 0)
	{
		return fLMRF;
	}

	fTmp2 = 128.0 - DotProduct2D(vDelta, pSelf->m_vEyeDir);
	
	if (fForward * pSelf->m_fFovDot > fTmp2)
	{
		return fLMRF;
	}

	*pbInFovAndRange = true;

	if (!pSelf->CanSee(m_pEnemy, 0, 0))
	{
		return fLMRF;
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

}

ActorEnemySet::~ActorEnemySet()
{

}

ActorEnemy *ActorEnemySet::AddPotentialEnemy
	(
	Sentient *pEnemy
	)

{
	// FIXME: stub
	STUB();
	return NULL;
}

void ActorEnemySet::FlagBadEnemy
	(
	Sentient *pEnemy
	)

{
	// FIXME: stub
	STUB();
}

void ActorEnemySet::CheckEnemies
	(
	Actor *pSelf
	)

{
	// FIXME: stub
	STUB();
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
	// FIXME: stub
	STUB();
	return false;
}

bool ActorEnemySet::HasAlternateEnemy
	(
	void
	) const

{
	// FIXME: stub
	STUB();
	return false;
}

void ActorEnemySet::RemoveAll
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void ActorEnemySet::ConfirmEnemy
	(
	Actor *pSelf,
	Sentient *pEnemy
	)

{
	// FIXME: stub
	STUB();
}

void ActorEnemySet::ConfirmEnemyIfCanSeeSharerOrEnemy
	(
	Actor *pSelf,
	Actor *pSharer,
	Sentient *pEnemy
	)

{
	// FIXME: stub
	STUB();
}

bool ActorEnemySet::CaresAboutPerfectInfo
	(
	Sentient *pEnemy
	)

{
	// FIXME: stub
	STUB();
	return false;
}
