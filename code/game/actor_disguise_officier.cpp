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

// actor_disguise_officier.cpp

#include "actor.h"


void Actor::InitDisguiseOfficer
	(
	GlobalFuncs_t *func
	)

{
	func->ThinkState						= &Actor::Think_DisguiseOfficer;
	func->BeginState						= &Actor::Begin_DisguiseOfficer;
	func->EndState							= &Actor::End_DisguiseOfficer;
	func->ResumeState						= &Actor::Resume_DisguiseOfficer;
	func->SuspendState						= &Actor::Suspend_DisguiseOfficer;
	func->PassesTransitionConditions		= &Actor::PassesTransitionConditions_Disguise;
	func->IsState							= &Actor::IsDisguiseState;
}

void Actor::Begin_DisguiseOfficer
	(
	void
	)

{
	vec2_t vDelta;
	m_csMood = STRING_BORED;
	/*
		useless assert
	  if ( !dword_39AC1C )
	  {
		if ( !this->baseSimpleActor.baseSentient.m_Enemy._vptr$ )
		{
		  v15 = MyAssertHandler("m_Enemy", "fgame/actor_disguise_officer.cpp", 38, 1);
		  if ( v15 < 0 )
		  {
			dword_39AC1C = 1;
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
		if ( (EnemyIsDisguised() || m_Enemy->IsSubclassOfActor())  && !level.m_bAlarm)
		{
			VectorSub2D(m_Enemy->origin, origin, vDelta);

			if (vDelta[0] != 0 || vDelta[1] != 0)
			{
				m_YawAchieved = false;
				m_DesiredYaw = vectoyaw(vDelta);
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
			SetThinkState(4, 0);
		}
	}
	else
	{
		SetThinkState(1, 0);
	}
}

void Actor::End_DisguiseOfficer
	(
	void
	)

{
	m_iNextDisguiseTime = level.inttime + (m_State ? m_iDisguisePeriod : 500);
}

void Actor::Resume_DisguiseOfficer
	(
	void
	)

{
	Begin_DisguiseOfficer();
}

void Actor::Suspend_DisguiseOfficer
	(
	void
	)

{
	End_DisguiseOfficer();
}

void Actor::Think_DisguiseOfficer
	(
	void
	)

{
	if (RequireThink())
	{
		UpdateEyeOrigin();
		NoPoint();
		ContinueAnimation();
		UpdateEnemy(1500);
		/*
		 * useless assert
		if ( !dword_39AC20 )
		{
			if ( !this->baseSimpleActor.baseSentient.m_Enemy._vptr$ )
			{
			v18 = MyAssertHandler("m_Enemy", "fgame/actor_disguise_officer.cpp", 139, 1);
			if ( v18 < 0 )
			{
				dword_39AC20 = 1;
			}
			else if ( v18 > 0 )
			{
				__debugbreak();
			}
			}
		}
		*/

		if (!m_Enemy)
		{
			SetThinkState(1, 0);
			return;
		}
		if (!EnemyIsDisguised() && !(m_Enemy->IsSubclassOfActor()) && m_State != 3)
		{
			m_State = 3;
			m_iStateTime = level.inttime;
		}
		if (level.m_bAlarm)
		{
			SetThinkState(4, 0);
			return;
		}
		vec2_t vDelta;

		VectorSub2D(m_Enemy->origin, origin, vDelta);

		if (vDelta[0] != 0 || vDelta[1] != 0)
		{
			m_YawAchieved = false;
			m_DesiredYaw = vectoyaw(vDelta);
		}

		SetDesiredLookDir(m_Enemy->origin - origin);

		if (m_State == 3)
		{
			m_pszDebugState = "enemy";
			State_Disguise_Enemy();
		}
		else if (m_State > 3)
		{
			if (m_State != 4)
			{
				Com_Printf("Actor::Think_DisguiseOfficer: invalid think state %i\n", m_State);
				/*
				 * useless assert
				 *if ( !dword_39AC24 )
				{
				  strcpy(v20, "\"invalid think state\"\n\tMessage: ");
				  memset(&s, 0, 0x3FDFu);
				  v16 = (*(this->baseSimpleActor.baseSentient.baseAnimate.baseEntity.baseSimple.baseListener.baseClass.vftable
						 + 87))(
						  this,
						  "thinkstate = %i",
						  this->m_State);
				  Q_strcat(v20, 0x4000, v16);
				  v17 = MyAssertHandler(v20, "fgame/actor_disguise_officer.cpp", 182, 0);
				  if ( v17 < 0 )
				  {
					dword_39AC24 = 1;
				  }
				  else if ( v17 > 0 )
				  {
					__debugbreak();
				  }
				}
				 **/
			}
			else
			{
				m_pszDebugState = "halt";
				Actor::State_Disguise_Halt();
			}
		}
		else
		{
			if (m_State != 1)

			{
				/*
				 * useless assert
				 *if ( !dword_39AC24 )
				{
				  strcpy(v20, "\"invalid think state\"\n\tMessage: ");
				  memset(&s, 0, 0x3FDFu);
				  v16 = (*(this->baseSimpleActor.baseSentient.baseAnimate.baseEntity.baseSimple.baseListener.baseClass.vftable
						 + 87))(
						  this,
						  "thinkstate = %i",
						  this->m_State);
				  Q_strcat(v20, 0x4000, v16);
				  v17 = MyAssertHandler(v20, "fgame/actor_disguise_officer.cpp", 182, 0);
				  if ( v17 < 0 )
				  {
					dword_39AC24 = 1;
				  }
				  else if ( v17 > 0 )
				  {
					__debugbreak();
				  }
				}
				 **/
			}
			else
			{
				m_pszDebugState = "papers";
				State_Disguise_Fake_Papers();
			}
		}
		CheckForTransition(7, 0);
		PostThink(true);
	}
}
