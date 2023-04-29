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

// actor_balcony.cpp

#include "actor.h"
#include "scriptexception.h"

void Actor::InitBalconyIdle
	(
	GlobalFuncs_t *func
	)

{
	func->ThinkState					= &Actor::Think_Idle;
	func->PassesTransitionConditions	= &Actor::PassesTransitionConditions_Idle;
	func->Pain							= &Actor::Pain_Balcony;
	func->Killed						= &Actor::Killed_Balcony;
	func->IsState						= &Actor::IsIdleState;
}

void Actor::InitBalconyCurious
	(
	GlobalFuncs_t *func
	)

{
	func->ThinkState					= &Actor::Think_Curious;
	func->BeginState					= &Actor::Begin_Curious;
	func->EndState						= &Actor::End_Curious;
	func->ResumeState					= &Actor::Resume_Curious;
	func->SuspendState					= &Actor::Suspend_Curious;
	func->FinishedAnimation				= &Actor::FinishedAnimation_Curious;
	func->PassesTransitionConditions	= &Actor::PassesTransitionConditions_Curious;
	func->IsState						= &Actor::IsCuriousState;
	func->Pain							= &Actor::Pain_Balcony;
	func->Killed						= &Actor::Killed_Balcony;
}

void Actor::InitBalconyAttack
	(
	GlobalFuncs_t *func
	)

{
	func->ThinkState					= &Actor::Think_BalconyAttack;
	func->PassesTransitionConditions	= &Actor::PassesTransitionConditions_Attack;
	func->BeginState					= &Actor::Begin_BalconyAttack;
	func->FinishedAnimation				= &Actor::FinishedAnimation_BalconyAttack;
	func->Pain							= &Actor::Pain_Balcony;
	func->Killed						= &Actor::Killed_Balcony;
	func->IsState						= &Actor::IsAttackState;
	func->PostShoot						= &Actor::State_Balcony_PostShoot;
}

void Actor::InitBalconyDisguise
	(
	GlobalFuncs_t *func
	)

{
	func->IsState = &Actor::IsDisguiseState;
}

void Actor::InitBalconyGrenade
	(
	GlobalFuncs_t *func
	)

{
	func->IsState = &Actor::IsGrenadeState;
}

void Actor::InitBalconyPain
	(
	GlobalFuncs_t *func
	)

{
	func->BeginState				= &Actor::Begin_Pain;
	func->ThinkState				= &Actor::Think_Pain;
	func->FinishedAnimation			= &Actor::FinishedAnimation_Pain;
	func->Pain						= &Actor::Pain_Balcony;
	func->Killed					= &Actor::Killed_Balcony;
	func->IsState					= &Actor::IsPainState;
}

void Actor::InitBalconyKilled
	(
	GlobalFuncs_t *func
	)

{
	func->BeginState				= &Actor::Begin_BalconyKilled;
	func->EndState					= &Actor::End_BalconyKilled;
	func->ThinkState				= &Actor::Think_BalconyKilled;
	func->FinishedAnimation			= &Actor::FinishedAnimation_BalconyKilled;
	func->IsState					= &Actor::IsKilledState;
}

void Actor::Pain_Balcony
	(
	Event *ev
	)

{
	SetThink( THINKSTATE_PAIN, THINK_BALCONY_PAIN);
	HandlePain( ev );
}

void Actor::Killed_Balcony
	(
	Event *ev,
	bool bPlayDeathAnim
	)

{
	ClearStates();
	SetThink( THINKSTATE_KILLED, THINK_BALCONY_KILLED);
	HandleKilled( ev, true );

	if( !bPlayDeathAnim )
		ScriptError( "cannot do 'bedead' on balcony guys" );
}

void Actor::Begin_BalconyAttack
	(
	void
	)

{
	TransitionState(200, 0);
}

void Actor::State_Balcony_PostShoot
	(
	void
	)

{
	if( m_Enemy )
	{
		TransitionState(201, 0);
	}
}

void Actor::State_Balcony_FindEnemy
	(
	void
	)

{
	m_bHasDesiredLookAngles = false;
	Anim_Aim();

	if( CanSeeEnemy( 200 ) )
	{
		TransitionState(201, 0);
	}
}

void Actor::State_Balcony_Target
	(
	void
	)

{
	Anim_Aim();
	AimAtTargetPos();

	if( level.inttime > m_iStateTime + 1000 )
	{
		if( CanSeeEnemy( 0 ) && CanShootEnemy( 0 ) )
		{
			TransitionState(202, 0);
		}
		else
		{
			ClearPath();
			TransitionState(200, 0);
		}
	}
}

void Actor::State_Balcony_Shoot
	(
	void
	)

{
	Anim_Shoot();
	AimAtTargetPos();
}

void Actor::Think_BalconyAttack
	(
	void
	)

{
	if( !RequireThink() )
		return;

	UpdateEyeOrigin();
	UpdateEnemy( 500 );

	if( !m_Enemy )
	{
		SetThinkState( THINKSTATE_IDLE, THINKLEVEL_NORMAL);
		IdleThink();
		return;
	}

	NoPoint();

	if( m_State == 201 )
	{
		m_pszDebugState = "target";
		State_Balcony_Target();
	}
	else if( m_State == 202 )
	{
		m_pszDebugState = "shoot";
		State_Balcony_Shoot();
	}
	else if( m_State == 200 )
	{
		m_pszDebugState = "findenemy";
		State_Balcony_FindEnemy();
	}
	else
	{
		Com_Printf( "Actor::Think_BalconyAttack: invalid think state %i\n", m_State );
		assert( 0 );
	}

	PostThink( true );
}

void Actor::FinishedAnimation_BalconyAttack
	(
	void
	)

{
	if( m_State == 202 )
		State_Balcony_PostShoot();
}

void Actor::Begin_BalconyKilled
	(
	void
	)

{
	ClearPath();
	ResetBoneControllers();

	PostEvent( EV_Actor_DeathEmbalm, 0.05f );

	if( CalcFallPath() )
		TransitionState(800, 0);
	else
		TransitionState(806, 0);
}

void Actor::End_BalconyKilled
	(
	void
	)

{
	if( m_pFallPath )
	{
		gi.Free(m_pFallPath);
		m_pFallPath = NULL;
	}
}

void Actor::Think_BalconyKilled
	(
	void
	)

{
	int animnum;

	Unregister( STRING_ANIMDONE );

	if( m_State == 805 )
	{
		m_pszDebugState = "end";
	}
	else
	{
		NoPoint();
		m_bHasDesiredLookAngles = false;
		StopTurning();

		switch( m_State )
		{
		case 800:
			m_bNextForceStart = true;
			m_eNextAnimMode = 7;
			m_pszDebugState = "begin";
			m_csNextAnimString = STRING_ANIM_NO_KILLED_SCR;

			animnum = gi.Anim_NumForName(edict->tiki, "death_balcony_intro");

			ChangeMotionAnim();

			m_bMotionAnimSet = true;
			m_iMotionSlot = GetMotionSlot(0);
			m_weightType[m_iMotionSlot] = 1;
			m_weightCrossBlend[m_iMotionSlot] = 0.0;
			m_weightBase[m_iMotionSlot] = 1.0;

			NewAnim( animnum, m_iMotionSlot );
			SetTime( m_iMotionSlot, m_pFallPath->startTime );
			UpdateNormalAnimSlot( m_iMotionSlot );
			TransitionState(801, 0);
			break;
		case 801:
			m_bNextForceStart = false;
			m_pszDebugState = "intro";
			m_eNextAnimMode = 7;
			m_csNextAnimString = STRING_ANIM_NO_KILLED_SCR;
			break;
		case 802:
			m_pszDebugState = "loop";
			Anim_FullBody( STRING_DEATH_BALCONY_LOOP, 7 );
			break;
		case 803:
			TransitionState(804, 0);
			StopAllAnimating();
		case 804:
			m_pszDebugState = "outtro";
			Anim_FullBody( STRING_DEATH_BALCONY_OUTTRO, 1 );
			break;
		case 806:
			m_pszDebugState = "normal";
			Anim_Killed();
			break;
		default:
			Com_Printf( "Actor::Think_BalconyKilled: invalid think state %i\n", m_State );
			assert( 0 );
		}

		PostThink( false );

		if( m_State >= 800 )
		{
			if( m_State == 801 )
			{
				if( m_pFallPath->currentPos >= m_pFallPath->length )
				{
					TransitionState(803, 0);
				}
				else if (m_pFallPath->currentPos >= m_pFallPath->loop)
				{
					TransitionState(802, 0);
				}
			}
			else if( m_State == 802 )
			{
				if (m_pFallPath->currentPos >= m_pFallPath->length)
				{
					TransitionState(803, 0);
				}
			}
		}
	}
}

void Actor::FinishedAnimation_BalconyKilled
	(
	void
	)

{
	if( m_State == 804 || m_State == 806 )
	{
		BecomeCorpse();
		TransitionState(805, 0);
	}
	else if( m_State == 801 )
	{
		TransitionState(802, 0);
		StopAllAnimating();
	}
}
