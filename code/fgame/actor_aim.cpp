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

// actor_aim.cpp

#include "actor.h"

void Actor::InitAim
	(
	GlobalFuncs_t *func
	)
{
	func->BeginState					= &Actor::Begin_Aim;
	func->ThinkState					= &Actor::Think_Aim;
	func->PassesTransitionConditions	= &Actor::PassesTransitionConditions_Idle;
	func->ShowInfo						= &Actor::ShowInfo_Aim;
	func->IsState						= &Actor::IsIdleState;
}
void Actor::Begin_Aim
	(
	void
	)
{
	m_csMood = STRING_ALERT;
	m_csIdleMood = STRING_NERVOUS;
	ClearPath();
}
void Actor::Think_Aim
	(
	void
	)
{
	if( !RequireThink() )
		return;

	UpdateEyeOrigin();
	NoPoint();
	m_pszDebugState = "";
	Anim_Aim();

	if( m_aimNode )
	{
		AimAtAimNode();
	}

	CheckForThinkStateTransition();
	PostThink( true );
}

void Actor::ShowInfo_Aim
	(
	void
	)
{
	ShowInfo_AimNode();
}
