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

// actor_disguise_common.cpp

#include "actor.h"


void Actor::InitDisguiseNone
	(
	GlobalFuncs_t *func
	)

{
	func->IsState = &Actor::IsDisguiseState;
}

void Actor::State_Disguise_Wait
	(
	void
	)

{
	vec2_t vDelta;
	float fDistSquared;
	VectorSub2D(origin, m_Enemy->origin, vDelta);

	m_eNextAnimMode = 1;
	m_csNextAnimString = STRING_ANIM_DISGUISE_WAIT_SCR;
	m_bNextForceStart = false;

	fDistSquared =  VectorLength2DSquared(vDelta);

	if (m_fMaxDisguiseDistSquared > fDistSquared * 4)
	{
		m_State = 1;
		m_iStateTime = level.inttime;
	}
	else
	{
		if (level.inttime > m_iStateTime + 3000)
		{
			if (fDistSquared <= 65536)
			{
				m_State = 1;
				m_iStateTime = level.inttime;
			}
			else
			{
				SetThinkState(1, 0);
			}
		}
	}
}

void Actor::State_Disguise_Papers
	(
	void
	)

{
	vec2_t vDelta;

	m_csNextAnimString = STRING_ANIM_DISGUISE_PAPERS_SCR;
	m_eNextAnimMode = 1;
	m_bNextForceStart = false;
	if (m_iEnemyShowPapersTime < m_Enemy->m_ShowPapersTime)
	{
		if (level.m_iPapersLevel < this->m_iDisguiseLevel)
		{
			m_State = 5;
		}
		else
		{
			if (m_DisguiseAcceptThread.IsSet())
				m_DisguiseAcceptThread.Execute(this);
			m_State = 2;
		}
		m_iStateTime = level.inttime;
	}
	else
	{
		if (level.inttime > m_iStateTime + 12000)
		{
			m_State = 3;
			m_iStateTime = level.inttime;
		}
		else
		{
			VectorSub2D(origin, m_Enemy->origin, vDelta);

			if (VectorLength2DSquared(vDelta) > 65536)
			{
				m_State = 4;
				m_iStateTime = level.inttime;
			}
		}
	}
}

void Actor::State_Disguise_Fake_Papers
	(
	void
	)

{
	vec2_t vDelta;

	m_csNextAnimString = STRING_ANIM_DISGUISE_PAPERS_SCR;
	m_eNextAnimMode = 1;
	m_bNextForceStart = false;
	if (m_iEnemyShowPapersTime < m_Enemy->m_ShowPapersTime || level.inttime > m_iStateTime + 12000)
	{
		m_State = 3;
		m_iStateTime = level.inttime;
	}
	else
	{
		VectorSub2D(origin, m_Enemy->origin, vDelta);

		if (VectorLength2DSquared(vDelta) > 65536)
		{
			m_State = 4;
			m_iStateTime = level.inttime;
		}
	}

}

void Actor::State_Disguise_Enemy
	(
	void
	)

{
	m_eNextAnimMode = 1;
	m_csNextAnimString = STRING_ANIM_DISGUISE_ENEMY_SCR;
	m_bNextForceStart = false;

	if (level.inttime > m_iStateTime + 3000 && !m_Enemy->IsSubclassOfActor())
	{
		SetThinkState(4, 0);
	}
}

void Actor::State_Disguise_Halt
	(
	void
	)

{
	m_eNextAnimMode = 1;
	m_csNextAnimString = STRING_ANIM_DISGUISE_HALT_SCR;
	m_bNextForceStart = false;

	if (level.inttime > m_iStateTime + 1500 && !m_Enemy->IsSubclassOfActor())
	{
		SetThinkState(4, 0);
	}
}

void Actor::State_Disguise_Accept
	(
	void
	)

{
	m_eNextAnimMode = 1;
	m_csNextAnimString = STRING_ANIM_DISGUISE_ACCEPT_SCR;
	m_bNextForceStart = false;

	if (level.inttime > m_iStateTime + 3000 )
	{
		SetThinkState(1, 0);
		SetThink(6, 10);
	}
}

void Actor::State_Disguise_Deny
	(
	void
	)

{
	m_eNextAnimMode = 1;
	m_csNextAnimString = STRING_ANIM_DISGUISE_DENY_SCR;
	m_bNextForceStart = false;

	if (level.inttime > m_iStateTime + 3000)
	{
		SetThinkState(1, 0);
	}
}
