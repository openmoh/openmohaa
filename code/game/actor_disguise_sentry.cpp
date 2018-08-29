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

void Actor::InitDisguiseSentry
	(
	GlobalFuncs_t *func
	)

{
	func->ThinkState						= &Actor::Think_DisguiseSentry;
	func->BeginState						= &Actor::Begin_DisguiseSentry;
	func->EndState							= &Actor::End_DisguiseSentry;
	func->ResumeState						= &Actor::Resume_DisguiseSentry;
	func->SuspendState						= &Actor::Suspend_DisguiseSentry;
	func->PassesTransitionConditions		= &Actor::PassesTransitionConditions_Disguise;
	func->IsState							= &Actor::IsDisguiseState;
}

void Actor::Begin_DisguiseSentry
	(
	void
	)

{
	vec2_t vDelta;


	m_csMood = STRING_BORED;
	/*
		useless assert
	  if ( !dword_39AC34 )
	  {
		if ( !this->baseSimpleActor.baseSentient.m_Enemy._vptr$ )
		{
		  v15 = MyAssertHandler("m_Enemy", "fgame/actor_disguise_rover.cpp", 38, 1);
		  if ( v15 < 0 )
		  {
			dword_39AC34 = 1;
		  }
		  else if ( v15 > 0 )
		  {
			__debugbreak();
		  }
		}
	  }
	 */

	if (m_Enemy)
	{
		if ((EnemyIsDisguised() || m_Enemy->IsSubclassOfActor()) && !level.m_bAlarm)
		{
			VectorSub2D(m_Enemy->origin, origin, vDelta);

			if (vDelta[0] != 0 || vDelta[1] != 0)
			{
				m_YawAchieved = false;
				m_DesiredYaw = vectoyaw(vDelta);
			}

			SetDesiredLookDir(m_Enemy->origin - origin);

			m_eNextAnimMode = 1;
			m_csNextAnimString = STRING_ANIM_DISGUISE_WAIT_SCR;
			m_bNextForceStart = false;

			m_State = 1;

			m_iEnemyShowPapersTime = m_Enemy->m_ShowPapersTime;
			m_iStateTime = level.inttime;
		}
		else
		{
			SetThinkState(4, 0);
		}
	}
	else
	{
		SetThinkState(1, 0);
	}
}

void Actor::End_DisguiseSentry
	(
	void
	)

{
	m_iNextDisguiseTime = level.inttime + (m_State ? m_iDisguisePeriod : 500);
}

void Actor::Resume_DisguiseSentry
	(
	void
	)

{
	Begin_DisguiseSentry();
}

void Actor::Suspend_DisguiseSentry
	(
	void
	)

{
	End_DisguiseSentry();
}

void Actor::Think_DisguiseSentry
	(
	void
	)

{
	// FIXME: stub
	STUB();
}
