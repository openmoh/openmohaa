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

// actor_animapi.cpp: Base actor animation script

#include "actor.h"

const_str SimpleActor::GetRunAnim
	(
	void
	)

{
	if( m_csCurrentPosition != STRING_PRONE && ( m_csCurrentPosition < STRING_PRONE || m_csCurrentPosition > STRING_CROUCHRUN ) )
		return STRING_ANIM_RUN_SCR;
	else
		return STRING_ANIM_CROUCH_RUN_SCR;
}

const_str SimpleActor::GetWalkAnim
	(
	void
	)

{
	if( m_csCurrentPosition != STRING_PRONE && ( m_csCurrentPosition < STRING_PRONE || m_csCurrentPosition > STRING_CROUCHRUN ) )
		return STRING_ANIM_WALK_SCR;
	else
		return STRING_ANIM_CROUCH_WALK_SCR;
}

void SimpleActor::Anim_Attack
	(
	void
	)

{
	m_eNextAnimMode = 1;
	m_csNextAnimString = NULL;
	m_NextAnimLabel = m_AttackHandler;
	m_bNextForceStart = false;
}

void SimpleActor::Anim_Sniper
	(
	void
	)

{
	m_eNextAnimMode = 1;
	m_csNextAnimString = 0;
	m_NextAnimLabel = m_SniperHandler;
	m_bNextForceStart = false;
}

void SimpleActor::Anim_Aim
	(
	void
	)

{
	m_eNextAnimMode = 1;
	m_csNextAnimString = STRING_ANIM_AIM_SCR;
	m_bNextForceStart = false;
}

void SimpleActor::Anim_Shoot
	(
	void
	)

{
	m_eNextAnimMode = 1;
	m_csNextAnimString = STRING_ANIM_SHOOT_SCR;
	m_bNextForceStart = false;
}

void SimpleActor::Anim_Idle
	(
	void
	)

{
	m_eNextAnimMode = 1;
	m_csNextAnimString = STRING_ANIM_IDLE_SCR;
	m_bNextForceStart = false;
}

void SimpleActor::Anim_Crouch
	(
	void
	)

{
	m_eNextAnimMode = 1;
	m_csNextAnimString = STRING_ANIM_CROUCH_SCR;
	m_bNextForceStart = false;
}

void SimpleActor::Anim_Prone
	(
	void
	)

{
	m_eNextAnimMode = 1;
	m_csNextAnimString = STRING_ANIM_PRONE_SCR;
	m_bNextForceStart = false;
}

void SimpleActor::Anim_Stand
	(
	void
	)

{
	m_eNextAnimMode = 1;
	m_csNextAnimString = STRING_ANIM_STAND_SCR;
	m_bNextForceStart = false;
}

void SimpleActor::Anim_Cower
	(
	void
	)

{
	m_eNextAnimMode = 1;
	m_csNextAnimString = STRING_ANIM_COWER_SCR;
	m_bNextForceStart = false;
}

void SimpleActor::Anim_Killed
	(
	void
	)

{
	m_eAnimMode = 1;
	m_csNextAnimString = 0;
	m_NextAnimLabel = m_DeathHandler;
	m_bNextForceStart = false;
}

void SimpleActor::Anim_StartPain
	(
	void
	)

{
	m_eAnimMode = 1;
	m_csNextAnimString = 0;
	m_NextAnimLabel = m_PainHandler;
	m_bNextForceStart = true;
}

void SimpleActor::Anim_Pain
	(
	void
	)

{
	ContinueAnimation();
}


void SimpleActor::Anim_CrouchRunTo
	(
	int eAnimMode
	)

{
	m_csNextAnimString = STRING_ANIM_CROUCH_RUN_SCR;
	m_eNextAnimMode = eAnimMode;
	m_bNextForceStart = false;
}

void SimpleActor::Anim_CrouchWalkTo
	(
	int eAnimMode
	)

{
	m_csNextAnimString = STRING_ANIM_CROUCH_WALK_SCR;
	m_eNextAnimMode = eAnimMode;
	m_bNextForceStart = false;
}

void SimpleActor::Anim_StandRunTo
	(
	int eAnimMode
	)

{
	m_csNextAnimString = STRING_ANIM_RUN_SCR;
	m_eNextAnimMode = eAnimMode;
	m_bNextForceStart = false;
}

void SimpleActor::Anim_StandWalkTo
	(
	int eAnimMode
	)

{
	m_csNextAnimString = STRING_ANIM_WALK_SCR;
	m_eNextAnimMode = eAnimMode;
	m_bNextForceStart = false;
}

void SimpleActor::Anim_RunTo
	(
	int eAnimMode
	)

{
	m_bNextForceStart = false;
	m_csNextAnimString = GetRunAnim();
	m_eNextAnimMode = eAnimMode;
}

void SimpleActor::Anim_WalkTo
	(
	int eAnimMode
	)

{
	m_bNextForceStart = false;
	m_csNextAnimString = GetWalkAnim();
	m_eNextAnimMode = eAnimMode;
}

void SimpleActor::Anim_RunAwayFiring
	(
	int eAnimMode
	)

{
	m_csNextAnimString = STRING_ANIM_RUNAWAYFIRING_SCR;
	m_eNextAnimMode = eAnimMode;
	m_bNextForceStart = false;
}

void SimpleActor::Anim_RunToShooting
	(
	int eAnimMode
	)

{
	m_csNextAnimString = STRING_ANIM_RUN_SHOOT_SCR;
	m_eNextAnimMode = eAnimMode;
	m_bNextForceStart = false;
}

void SimpleActor::Anim_RunToAlarm
	(
	int eAnimMode
	)

{
	m_csNextAnimString = STRING_ANIM_RUNTO_ALARM_SCR;
	m_eNextAnimMode = eAnimMode;
	m_bNextForceStart = false;
}

void SimpleActor::Anim_RunToCasual
	(
	int eAnimMode
	)

{
	m_csNextAnimString = STRING_ANIM_RUNTO_CASUAL_SCR;
	m_eNextAnimMode = eAnimMode;
	m_bNextForceStart = false;
}

void SimpleActor::Anim_RunToCover
	(
	int eAnimMode
	)

{
	m_csNextAnimString = STRING_ANIM_RUNTO_COVER_SCR;
	m_eNextAnimMode = eAnimMode;
	m_bNextForceStart = false;
}

void SimpleActor::Anim_RunToDanger
	(
	int eAnimMode
	)

{
	m_csNextAnimString = STRING_ANIM_RUNTO_DANGER_SCR;
	m_eNextAnimMode = eAnimMode;
	m_bNextForceStart = false;
}

void SimpleActor::Anim_RunToDive
	(
	int eAnimMode
	)

{
	m_csNextAnimString = STRING_ANIM_RUNTO_DIVE_SCR;
	m_eNextAnimMode = eAnimMode;
	m_bNextForceStart = false;
}

void SimpleActor::Anim_RunToFlee
	(
	int eAnimMode
	)

{
	m_eNextAnimMode = eAnimMode;
	m_csNextAnimString = STRING_ANIM_RUNTO_FLEE_SCR;
	m_bNextForceStart = false;
}

void SimpleActor::Anim_RunToInOpen
	(
	int eAnimMode
	)

{
	m_eNextAnimMode = eAnimMode;
	m_csNextAnimString = STRING_ANIM_RUNTO_INOPEN_SCR;
	m_bNextForceStart = false;
}

void SimpleActor::Anim_Emotion
	(
	eEmotionMode eEmotMode
	)

{
	m_eEmotionMode = eEmotMode;
}

void SimpleActor::Anim_Say
	(
	const_str csSayAnimScript,
	int iMinTimeSinceLastSay,
	bool bCanInterrupt
	)

{
	if( m_bSayAnimSet && !bCanInterrupt )
		return;

	if( level.inttime > m_iVoiceTime + iMinTimeSinceLastSay )
	{
		ScriptThreadLabel label;

		label.TrySetScript( csSayAnimScript );
		label.Execute( this );
	}
}

void SimpleActor::Anim_FullBody
	(
	const_str csFullBodyAnim,
	int eAnimMode
	)

{
	if( m_csAnimName == csFullBodyAnim )
	{
		m_eNextAnimMode = eAnimMode;
		m_csNextAnimString = STRING_ANIM_FULLBODY_SCR;
		m_bNextForceStart = false;
	}
	else
	{
		m_csAnimName = csFullBodyAnim;
		m_eNextAnimMode = eAnimMode;
		m_csNextAnimString = STRING_ANIM_FULLBODY_SCR;
		m_bNextForceStart = true;
	}
}
