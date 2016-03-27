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

// actorenemy.cpp:

#include "actorenemy.h"

ActorEnemy::ActorEnemy()
{

}

ActorEnemy::~ActorEnemy()
{

}

float ActorEnemy::UpdateVisibility
	(
	Actor *pSelf,
	bool *pbInFovAndRange,
	bool *pbVisible
	)

{
	return 0;
}

int ActorEnemy::UpdateThreat
	(
	Actor *pSelf
	)

{
	return NULL;
}

Sentient *ActorEnemy::GetEnemy
	(
	void
	) const

{
	// FIXME: stub
	STUB();
	return NULL;
}

float ActorEnemy::GetVisibility
	(
	void
	) const

{
	// FIXME: stub
	STUB();
	return 0;
}

int ActorEnemy::GetThreat
	(
	void
	) const

{
	// FIXME: stub
	STUB();
	return 0;
}

float ActorEnemy::GetRangeSquared
	(
	void
	) const

{
	// FIXME: stub
	STUB();
	return 0;
}

ActorEnemySet::ActorEnemySet()
{

}

ActorEnemySet::~ActorEnemySet()
{

}

ActorEnemy *ActorEnemySet::AddPotentialEnemy
	(
	Sentient *pEnemy
	)

{
	// FIXME: stub
	STUB();
	return NULL;
}

void ActorEnemySet::FlagBadEnemy
	(
	Sentient *pEnemy
	)

{
	// FIXME: stub
	STUB();
}

void ActorEnemySet::CheckEnemies
	(
	Actor *pSelf
	)

{
	// FIXME: stub
	STUB();
}

Sentient *ActorEnemySet::GetCurrentEnemy
	(
	void
	) const

{
	// FIXME: stub
	STUB();
	return NULL;
}

float ActorEnemySet::GetCurrentVisibility
	(
	void
	) const

{
	// FIXME: stub
	STUB();
	return 0;
}

int ActorEnemySet::GetCurrentThreat
	(
	void
	) const

{
	// FIXME: stub
	STUB();
	return 0;
}

qboolean ActorEnemySet::IsEnemyConfirmed
	(
	void
	) const

{
	// FIXME: stub
	STUB();
	return false;
}

bool ActorEnemySet::HasAlternateEnemy
	(
	void
	) const

{
	// FIXME: stub
	STUB();
	return false;
}

void ActorEnemySet::RemoveAll
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void ActorEnemySet::ConfirmEnemy
	(
	Actor *pSelf,
	Sentient *pEnemy
	)

{
	// FIXME: stub
	STUB();
}

void ActorEnemySet::ConfirmEnemyIfCanSeeSharerOrEnemy
	(
	Actor *pSelf,
	Actor *pSharer,
	Sentient *pEnemy
	)

{
	// FIXME: stub
	STUB();
}

bool ActorEnemySet::CaresAboutPerfectInfo
	(
	Sentient *pEnemy
	)

{
	// FIXME: stub
	STUB();
	return false;
}
