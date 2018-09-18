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

// actor_disguise_rover.cpp

#include "actor.h"

void Actor::InitDisguiseRover
	(
	GlobalFuncs_t *func
	)

{
	func->ThinkState						= &Actor::Think_DisguiseRover;
	func->BeginState						= &Actor::Begin_DisguiseRover;
	func->EndState							= &Actor::End_DisguiseRover;
	func->ResumeState						= &Actor::Resume_DisguiseRover;
	func->SuspendState						= &Actor::Suspend_DisguiseRover;
	func->PassesTransitionConditions		= &Actor::PassesTransitionConditions_Disguise;
	func->IsState							= &Actor::IsDisguiseState;
}

void Actor::Begin_DisguiseRover
	(
	void
	)

{

	vec2_t vDelta;
	m_csMood = STRING_BORED;
	assert(m_Enemy);

	if (m_Enemy)
	{
		if ((EnemyIsDisguised() || m_Enemy->IsSubclassOfActor()) && !level.m_bAlarm)
		{
			VectorSub2D(m_Enemy->origin, origin, vDelta);

			if (vDelta[0] != 0 || vDelta[1] != 0)
			{
				SetDesiredYawDir(vDelta);
			}

			SetDesiredLookDir(m_Enemy->origin - origin);

			m_eNextAnimMode = 1;
			m_csNextAnimString = STRING_ANIM_DISGUISE_PAPERS_SCR;
			m_bNextForceStart = false;

			m_State = 1;

			m_iEnemyShowPapersTime = m_Enemy->m_ShowPapersTime;
			m_iStateTime = level.inttime;
		}
		else
		{
			SetThinkState(THINKSTATE_ATTACK, THINKLEVEL_NORMAL);
		}
	}
	else
	{
		SetThinkState(THINKSTATE_IDLE, THINKLEVEL_NORMAL);
	}
}

void Actor::End_DisguiseRover
	(
	void
	)

{
	m_iNextDisguiseTime = level.inttime + (m_State ? m_iDisguisePeriod : 500);
}

void Actor::Resume_DisguiseRover
	(
	void
	)

{
	Begin_DisguiseRover();
}

void Actor::Suspend_DisguiseRover
	(
	void
	)

{
	End_DisguiseRover();
}

void Actor::Think_DisguiseRover
	(
	void
	)

{
	if (!RequireThink())
	{
		return;
	}
	UpdateEyeOrigin();
	NoPoint();
	ContinueAnimation();
	UpdateEnemy(1500);

	assert(m_Enemy != NULL);

	if (!m_Enemy)
	{
		SetThinkState(THINKSTATE_IDLE, THINKLEVEL_NORMAL);
		return;
	}

	if (!EnemyIsDisguised() && !m_Enemy->IsSubclassOfActor() && m_State != 3)
	{
		m_State = 3;
		m_iStateTime = level.inttime;
	}

	if (level.m_bAlarm)
	{
		SetThinkState(THINKSTATE_ATTACK, THINKLEVEL_NORMAL);
		return;
	}
	
	{
		vec2_t facedir;
		facedir[0] = m_Enemy->origin[0] - origin[0];
		facedir[1] = m_Enemy->origin[1] - origin[1];
		if (facedir[0] != 0 || facedir[1] != 0)
		{
			SetDesiredYawDir(facedir);
		}

	}
	
	SetDesiredLookDir(m_Enemy->origin - origin);

	if (m_State == 2)
	{
		m_pszDebugState = "accept";
		State_Disguise_Accept();
	}
	else if(m_State > 2)
	{
		if (m_State == 3)
		{
			m_pszDebugState = "enemy";
			State_Disguise_Enemy();
		}
		else if (m_State == 4)
		{
			m_pszDebugState = "halt";
			State_Disguise_Halt();
		}
		else
		{
			assert(!"invalid think state");
		}
	}
	else if (m_State == 1)
	{
		m_pszDebugState = "papers";
		State_Disguise_Papers();
	}
	else
	{
		assert(!"invalid think state");
	}

	CheckForTransition(THINKSTATE_GRENADE, 0);
	PostThink(true);
}
