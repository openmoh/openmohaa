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

// actor_disguise_salute.cpp

#include "actor.h"

void Actor::InitDisguiseSalute
	(
	GlobalFuncs_t *func
	)

{
	func->ThinkState						= &Actor::Think_DisguiseSalute;
	func->BeginState						= &Actor::Begin_DisguiseSalute;
	func->EndState							= &Actor::End_DisguiseSalute;
	func->ResumeState						= &Actor::Resume_DisguiseSalute;
	func->SuspendState						= &Actor::Suspend_DisguiseSalute;
	func->PassesTransitionConditions		= &Actor::PassesTransitionConditions_Disguise;
	func->IsState							= &Actor::IsDisguiseState;
}

void Actor::Begin_DisguiseSalute
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::End_DisguiseSalute
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::Resume_DisguiseSalute
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::Suspend_DisguiseSalute
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::Think_DisguiseSalute
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::FinishedAnimation_DisguiseSalute
	(
	void
	)

{
	// FIXME: stub
	STUB();
}
