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

// actor.cpp:

#include "actor.h"

void Actor::InitDogIdle
	(
	GlobalFuncs_t *func
	)

{
	func->BeginState					= &Actor::Begin_Dog;
	func->EndState						= &Actor::End_Dog;
	func->ThinkState					= &Actor::Think_Dog_Idle;
	func->PassesTransitionConditions	= &Actor::PassesTransitionConditions_Idle;
	func->IsState						= &Actor::IsDogState;
}

void Actor::InitDogAttack
	(
	GlobalFuncs_t *func
	)

{
	func->BeginState					= &Actor::Begin_Dog;
	func->EndState						= &Actor::End_Dog;
	func->ThinkState					= &Actor::Think_Dog_Attack;
	func->PassesTransitionConditions	= &Actor::PassesTransitionConditions_Attack;
	func->IsState						= &Actor::IsDogState;
}

void Actor::InitDogCurious
	(
	GlobalFuncs_t *func
	)

{
	func->BeginState					= &Actor::Begin_Dog;
	func->EndState						= &Actor::End_Dog;
	func->ThinkState					= &Actor::Think_Dog_Curious;
	func->PassesTransitionConditions	= &Actor::PassesTransitionConditions_Curious;
	func->IsState						= &Actor::IsDogState;
}

void Actor::Begin_Dog
	(
	void
	)

{
	m_bAnimating = true;
}

void Actor::End_Dog
	(
	void
	)

{
}

void Actor::Think_Dog_Idle
	(
	void
	)

{
	if( !RequireThink() )
	{
		return;
	}

	UpdateEyeOrigin();
	m_pszDebugState = "Dog_Idle";
	m_bHasDesiredLookAngles = false;
	m_eNextAnimMode = 1;
	m_csNextAnimString = STRING_ANIM_DOG_IDLE_SCR;
	CheckForThinkStateTransition();
	PostThink( false );
}

void Actor::Think_Dog_Attack
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::Think_Dog_Curious
	(
	void
	)

{
	if (RequireThink())
	{
		UpdateEyeOrigin();
		m_pszDebugState = "Dog_Curious";

		if (m_Enemy && !m_Enemy->IsSubclassOfActor())
		{
			vec2_t vDelta;
			VectorSub2D(m_Enemy->origin, origin, vDelta);

			if (vDelta[0] != 0 || vDelta[1] != 0)
			{
				m_YawAchieved = false;
				m_DesiredYaw = vectoyaw(vDelta);
			}

			SetDesiredLookDir(m_Enemy->origin - origin);

		}
		m_bNextForceStart = false;
		m_eNextAnimMode = 1;
		m_csNextAnimString = STRING_ANIM_DOG_CURIOUS_SCR;

		CheckForThinkStateTransition();
		PostThink(false);
	}
}
