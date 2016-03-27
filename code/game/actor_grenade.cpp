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
	// FIXME: stub
	STUB();
}

void Actor::Grenade_Throw
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::Grenade_KickAcquire
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::Grenade_Kick
	(
	void
	)

{
	// FIXME: stub
	STUB();
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
	// FIXME: stub
	STUB();
}

void Actor::Grenade_NextThinkState
	(
	void
	)

{
	// FIXME: stub
	STUB();
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
	// FIXME: stub
	STUB();
}

void Actor::Resume_Grenade
	(
	void
	)

{
	// FIXME: stub
	STUB();
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
	// FIXME: stub
	STUB();
}
