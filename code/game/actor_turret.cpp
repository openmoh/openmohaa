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

// actor_turret.cpp

#include "actor.h"

void Actor::InitTurret
	(
	GlobalFuncs_t *func
	)

{
	func->ThinkState					= &Actor::Think_Turret;
	func->BeginState					= &Actor::Begin_Turret;
	func->EndState						= &Actor::End_Turret;
	func->SuspendState					= &Actor::Suspend_Turret;
	func->ResumeState					= NULL;
	func->FinishedAnimation				= &Actor::FinishedAnimation_Turret;
	func->PassesTransitionConditions	= &Actor::PassesTransitionConditions_Attack;
	func->PostShoot						= &Actor::InterruptPoint_Turret;
	func->ReceiveAIEvent				= &Actor::ReceiveAIEvent_Turret;
	func->IsState						= &Actor::IsAttackState;
	func->PathnodeClaimRevoked			= &Actor::PathnodeClaimRevoked_Turret;
}

void Actor::Begin_Turret
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::End_Turret
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::Suspend_Turret
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::Think_Turret
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::FinishedAnimation_Turret
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::ReceiveAIEvent_Turret
	(
	vec3_t event_origin,
	int iType,
	Entity *originator,
	float fDistSquared,
	float fMaxDistSquared
	)

{
	// FIXME: stub
	STUB();
}

void Actor::InterruptPoint_Turret
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::PathnodeClaimRevoked_Turret
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

bool Actor::Turret_IsRetargeting
	(
	void
	) const

{
	// FIXME: stub
	STUB();
	return false;
}

bool Actor::Turret_DecideToSelectState
	(
	void
	)

{
	// FIXME: stub
	STUB();
	return false;
}

void Actor::Turret_SelectState
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

bool Actor::Turret_CheckRetarget
	(
	void
	)

{
	// FIXME: stub
	STUB();
	return false;
}

bool Actor::Turret_TryToBecomeCoverGuy
	(
	void
	)

{
	// FIXME: stub
	STUB();
	return false;
}

void Actor::Turret_BeginRetarget
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::Turret_NextRetarget
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::Turret_SideStep
	(
	int iStepSize,
	vec3_t vDir
	)

{
	// FIXME: stub
	STUB();
}

void Actor::State_Turret_Combat
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::State_Turret_Reacquire
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::State_Turret_TakeSniperNode
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::State_Turret_SniperNode
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

bool Actor::State_Turret_RunHome
	(
	bool bAttackOnFail
	)

{
	// FIXME: stub
	STUB();
	return false;
}

void Actor::State_Turret_RunAway
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::State_Turret_Charge
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::State_Turret_Grenade
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::State_Turret_FakeEnemy
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::State_Turret_Wait
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::State_Turret_Retarget_Sniper_Node
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::State_Turret_Retarget_Step_Side_Small
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::State_Turret_Retarget_Path_Exact
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::State_Turret_Retarget_Path_Near
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::State_Turret_Retarget_Step_Side_Medium
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::State_Turret_Retarget_Step_Side_Large
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::State_Turret_Retarget_Step_Face_Medium
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::State_Turret_Retarget_Step_Face_Large
	(
	void
	)

{
	// FIXME: stub
	STUB();
}
