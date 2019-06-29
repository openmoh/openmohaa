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
	DesiredAnimation(1, m_AttackHandler);
}

void SimpleActor::Anim_Sniper
	(
	void
	)

{
	DesiredAnimation(1, m_SniperHandler);
}

void SimpleActor::Anim_Aim
	(
	void
	)

{
	DesiredAnimation(1, STRING_ANIM_AIM_SCR);
}

void SimpleActor::Anim_Shoot
	(
	void
	)

{
	DesiredAnimation(1, STRING_ANIM_SHOOT_SCR);
}

void SimpleActor::Anim_Idle
	(
	void
	)

{
	DesiredAnimation(1, STRING_ANIM_IDLE_SCR);
}

void SimpleActor::Anim_Crouch
	(
	void
	)

{
	DesiredAnimation(1, STRING_ANIM_CROUCH_SCR);
}

void SimpleActor::Anim_Prone
	(
	void
	)

{
	DesiredAnimation(1, STRING_ANIM_PRONE_SCR);
}

void SimpleActor::Anim_Stand
	(
	void
	)

{
	DesiredAnimation(1, STRING_ANIM_STAND_SCR);
}

void SimpleActor::Anim_Cower
	(
	void
	)

{
	DesiredAnimation(1, STRING_ANIM_COWER_SCR);
}

void SimpleActor::Anim_Killed
	(
	void
	)

{
	Com_Printf("m_eAnimMode Anim_Killed \n");
	DesiredAnimation(1, m_DeathHandler);
}

void SimpleActor::Anim_StartPain
	(
	void
	)

{
	Com_Printf("m_eAnimMode Anim_StartPain \n");
	StartAnimation(1, m_PainHandler);
	
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
	DesiredAnimation(eAnimMode, STRING_ANIM_CROUCH_RUN_SCR);
}

void SimpleActor::Anim_CrouchWalkTo
	(
	int eAnimMode
	)

{
	DesiredAnimation(eAnimMode, STRING_ANIM_CROUCH_WALK_SCR);
}

void SimpleActor::Anim_StandRunTo
	(
	int eAnimMode
	)

{
	DesiredAnimation(eAnimMode, STRING_ANIM_RUN_SCR);
}

void SimpleActor::Anim_StandWalkTo
	(
	int eAnimMode
	)

{
	DesiredAnimation(eAnimMode, STRING_ANIM_WALK_SCR);
}

void SimpleActor::Anim_RunTo
	(
	int eAnimMode
	)

{
	DesiredAnimation(eAnimMode, GetRunAnim());
}

void SimpleActor::Anim_WalkTo
	(
	int eAnimMode
	)

{
	DesiredAnimation(eAnimMode, GetWalkAnim());
}

void SimpleActor::Anim_RunAwayFiring
	(
	int eAnimMode
	)

{
	DesiredAnimation(eAnimMode, STRING_ANIM_RUNAWAYFIRING_SCR);
}

void SimpleActor::Anim_RunToShooting
	(
	int eAnimMode
	)

{
	DesiredAnimation(eAnimMode, STRING_ANIM_RUN_SHOOT_SCR);
}

void SimpleActor::Anim_RunToAlarm
	(
	int eAnimMode
	)

{
	DesiredAnimation(eAnimMode, STRING_ANIM_RUNTO_ALARM_SCR);
}

void SimpleActor::Anim_RunToCasual
	(
	int eAnimMode
	)

{
	DesiredAnimation(eAnimMode, STRING_ANIM_RUNTO_CASUAL_SCR);
}

void SimpleActor::Anim_RunToCover
	(
	int eAnimMode
	)

{
	DesiredAnimation(eAnimMode, STRING_ANIM_RUNTO_COVER_SCR);
}

void SimpleActor::Anim_RunToDanger
	(
	int eAnimMode
	)

{
	DesiredAnimation(eAnimMode, STRING_ANIM_RUNTO_DANGER_SCR);
}

void SimpleActor::Anim_RunToDive
	(
	int eAnimMode
	)

{
	DesiredAnimation(eAnimMode, STRING_ANIM_RUNTO_DIVE_SCR);
}

void SimpleActor::Anim_RunToFlee
	(
	int eAnimMode
	)

{
	DesiredAnimation(eAnimMode, STRING_ANIM_RUNTO_FLEE_SCR);
}

void SimpleActor::Anim_RunToInOpen
	(
	int eAnimMode
	)

{
	DesiredAnimation(eAnimMode, STRING_ANIM_RUNTO_INOPEN_SCR);
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
		DesiredAnimation(eAnimMode, STRING_ANIM_FULLBODY_SCR);
	}
	else
	{
		m_csAnimName = csFullBodyAnim;
		StartAnimation(eAnimMode, STRING_ANIM_FULLBODY_SCR);
	}
}
