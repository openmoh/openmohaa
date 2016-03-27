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

// actor_disguise_common.cpp

#include "actor.h"


void Actor::InitDisguiseNone
	(
	GlobalFuncs_t *func
	)

{
	func->IsState = &Actor::IsDisguiseState;
}

void Actor::State_Disguise_Wait
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::State_Disguise_Papers
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::State_Disguise_Fake_Papers
	(
	void
	)

{
	// FIXME: stub
}

void Actor::State_Disguise_Enemy
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::State_Disguise_Halt
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::State_Disguise_Accept
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::State_Disguise_Deny
	(
	void
	)

{
	// FIXME: stub
	STUB();
}
