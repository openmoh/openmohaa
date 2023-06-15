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

// actor_pain.cpp

#include "actor.h"

void Actor::InitPain
	(
	GlobalFuncs_t *func
	)

{
	func->BeginState					= &Actor::Begin_Pain;
	func->ThinkState					= &Actor::Think_Pain;
	func->FinishedAnimation				= &Actor::FinishedAnimation_Pain;
	func->IsState						= &Actor::IsPainState;
}

void Actor::Begin_Pain
	(
	void
	)

{
	m_PainState = 500;
}

void Actor::Think_Pain
	(
	void
	)

{
	NoPoint();

	m_bHasDesiredLookAngles = false;

	UpdateEyeOrigin();

	if (m_PainState == 500)
	{
		m_pszDebugState = "initial";
		Anim_StartPain();
		m_PainState = 501;
	}
	else if (m_PainState == 501)
	{
		m_pszDebugState = "main";
		Anim_Pain();
	}
	PostThink(false);
}

void Actor::FinishedAnimation_Pain
	(
	void
	)

{
	if (m_PainState != 500)
		EndCurrentThinkState();
}
