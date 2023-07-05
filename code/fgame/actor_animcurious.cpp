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

// actor_animcurious.cpp

#include "actor.h"

void Actor::InitAnimCurious
	(
	GlobalFuncs_t *func
	)
{
	func->ThinkState					= &Actor::Think_AnimCurious;
	func->BeginState					= &Actor::Begin_AnimCurious;
	func->PassesTransitionConditions	= &Actor::PassesTransitionConditions_Curious;
	func->FinishedAnimation				= &Actor::FinishedAnimation_Anim;
	func->ShowInfo						= &Actor::ShowInfo_Anim;
	func->IsState						= &Actor::IsCuriousState;
}

void Actor::Begin_AnimCurious
	(
	void
	)
{
	DoForceActivate();

	m_csMood = STRING_CURIOUS;
	StartAnimation(m_AnimMode, m_csAnimScript);
	StopTurning();
	m_iCuriousTime = level.inttime;
}

void Actor::Think_AnimCurious
	(
	void
	)
{
	if( !RequireThink() )
		return;

	UpdateEyeOrigin();
	m_pszDebugState = "";
	LookAtCuriosity();
	TimeOutCurious();
	DesiredAnimation(m_AnimMode, m_csAnimScript);
	CheckForThinkStateTransition();
	PostThink( false );
}

void Actor::FinishedAnimation_AnimCurious
	(
	void
	)
{
	if( m_State == 1101 )
	{
		TransitionState(1100, 0);
	}
}
