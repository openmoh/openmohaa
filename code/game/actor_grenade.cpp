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
	int eNextState,
	const_str csReturnAnim
	)

{
	// FIXME: stub
	STUB();
	return false;
}

void Actor::Grenade_Flee
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::Grenade_ThrowAcquire
	(
	void
	)

{
	if (!Grenade_Acquire(2, STRING_ANIM_GRENADERETURN_SCR)
		&& !CanGetGrenadeFromAToB(
			m_vGrenadePos,
			m_vLastEnemyPos,
			true,
			&m_vGrenadeVel,
			&m_eGrenadeMode))
	{
		m_bGrenadeBounced = true;
		m_eGrenadeState = 0;
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
	if (!Actor::Grenade_Acquire(4, STRING_ANIM_GRENADEKICK_SCR))
	{
		VectorSub2D(m_vGrenadePos, origin, vFace);
		if (CanKickGrenade(m_vGrenadePos, m_vLastEnemyPos, vFace, &m_vGrenadeVel))
		{

			m_vKickDir[0] = m_vGrenadeVel[0];
			m_vKickDir[1] = m_vGrenadeVel[1];
			m_vKickDir[2] = 0.0;
			
			VectorNormalizeFast(m_vKickDir);
		}
		else
		{
			m_bGrenadeBounced = true;
			m_eGrenadeState = 0;
			Grenade_Flee();
		}
	}
	// FIXME: stub
	STUB();
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
	// FIXME: stub
	STUB();
}

void Actor::Grenade_Martyr
	(
	void
	)

{
	// FIXME: stub
	STUB();
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
		SetThinkState(4, 0);
	else
		SetThinkState(1, 0);
}

void Actor::Grenade_EventAttach
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::Grenade_EventDetach
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::Begin_Grenade
	(
	void
	)

{
	// FIXME: stub
	STUB();
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
	// FIXME: stub
	STUB();
}

void Actor::FinishedAnimation_Grenade
	(
	void
	)

{
	switch (m_eGrenadeState)
	{
	case 0:
	case 1:
	case 3:
	case 5:
	case 6:
	case 8:
	case 9:
		return;
	case 2:
	case 4:
		Grenade_NextThinkState();
		break;
	default:
		/*
		 * useless assert
		if (!dword_39AC84)
		{
			strcpy(&v3, "\"invalid grenade state in FinishedAnimation()\"\n\tMessage: ");
			memset(&s, 0, 0x3FC6u);
			v1 = (*(this->baseSimpleActor.baseSentient.baseAnimate.baseEntity.baseSimple.baseListener.baseClass.vftable + 87))(
				this,
				"state = %i",
				this->m_eGrenadeState);
			Q_strcat(&v3, 0x4000, v1);
			v2 = MyAssertHandler(&v3, "fgame/actor_grenade.cpp", 663, 0);
			if (v2 < 0)
			{
				dword_39AC84 = 1;
			}
			else if (v2 > 0)
			{
				__debugbreak();
			}
		}*/
		break;
	}
}
