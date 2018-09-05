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

// actor_weaponless.cpp

#include "actor.h"

void Actor::InitWeaponless
	(
	GlobalFuncs_t *func
	)

{
	func->ThinkState					= &Actor::Think_Weaponless;
	func->BeginState					= &Actor::Begin_Weaponless;
	func->SuspendState					= &Actor::Suspend_Weaponless;
	func->PassesTransitionConditions	= &Actor::PassesTransitionConditions_Attack;
	func->FinishedAnimation				= &Actor::FinishedAnimation_Weaponless;
	func->IsState						= &Actor::IsAttackState;
}

void Actor::Begin_Weaponless
	(
	void
	)

{

	DoForceActivate();

	m_csMood = STRING_ALERT;
	m_csIdleMood = STRING_NERVOUS;

	if (level.inttime < m_iEnemyChangeTime + 200)
	{
		SetLeashHome(origin);
		if (AttackEntryAnimation())
		{
			m_State = 902;
			m_bLockThinkState = true;
			m_iStateTime = level.inttime;
		}
	}
	m_State = 900;
	m_iStateTime = level.inttime;
}

void Actor::Suspend_Weaponless
	(
	void
	)

{
	if (m_State <= 902)
	{
		m_State = 900;
		m_iStateTime = level.inttime;
	}

}

void Actor::Think_Weaponless
	(
	void
	)

{

	if (RequireThink())
	{
		UpdateEyeOrigin();
		NoPoint();
		UpdateEnemy(500);


		if (m_State == 902)
		{
			ContinueAnimation();
		}
		else
		{
			m_bLockThinkState = false;
			if (!m_Enemy)
			{
				SetThinkState(THINKSTATE_IDLE, THINKLEVEL_NORMAL);
				IdleThink();
				return;
			}
			if (m_State == 900)
			{
				State_Weaponless_Normal();
			}
			else if (m_State == 901)
			{
				GenericGrenadeTossThink();
			}
			else
			{
				Com_Printf("Think_Weaponless: invalid think state %i\n", m_State);
				/* useless assert
				if ( !dword_39ACFC )
				{
				  strcpy(v4, "\"invalid think state\"\n\tMessage: ");
				  memset(&s, 0, 0x3FDFu);
				  v2 = DumpCallTrace(
						 "thinkstate = %i",
						 m_State);
				  Q_strcat(v4, 0x4000, v2);
				  v3 = MyAssertHandler(v4, "fgame/actor_weaponless.cpp", 155, 0);
				  if ( v3 < 0 )
				  {
					dword_39ACFC = 1;
				  }
				  else if ( v3 > 0 )
				  {
					__debugbreak();
				  }
				}
				*/
			}
			CheckForTransition(THINKSTATE_GRENADE, 0);
		}
		PostThink(true);
		if (GetWeapon(WEAPON_MAIN))
			SetThink(THINKSTATE_ATTACK, THINK_TURRET);
	}
}

void Actor::FinishedAnimation_Weaponless
	(
	void
	)

{
	if (m_State <= 902)
	{
		m_State = 900;
		m_iStateTime = level.inttime + 4000;
	}
}

void Actor::State_Weaponless_Normal
	(
	void
	)

{
	int iStateTime;
	if (m_bScriptGoalValid)
		SetPath(
			m_vScriptGoal,
			"",
			0,
			NULL,
			0); 
	if (PathExists() && !PathComplete())
	{
		FaceMotion();
		Anim_RunToDanger(3);
	}
	else
	{
		m_bScriptGoalValid = false;

		AimAtAimNode();
		
		Anim_Stand();
		if (level.inttime >= m_iStateTime)
		{
			if (DecideToThrowGrenade(m_Enemy->velocity + m_Enemy->origin, &m_vGrenadeVel, &m_eGrenadeMode))
			{
				SetDesiredYawDir(m_vGrenadeVel);

				m_State = 901;
				m_eNextAnimMode = 1;
				m_bNextForceStart = false;
				m_csNextAnimString = (m_eGrenadeMode == AI_GREN_TOSS_ROLL) ? STRING_ANIM_GRENADETOSS_SCR : STRING_ANIM_GRENADETHROW_SCR;
				iStateTime = level.inttime;
			}
			else
			{
				m_State = 900;
				iStateTime = level.inttime + 1000;
			}
			m_iStateTime = iStateTime;
		}
	}
}
