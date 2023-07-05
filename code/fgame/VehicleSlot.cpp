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

// VehicleSlot.cpp
//

#include "VehicleSlot.h"
#include "weapturret.h"

cVehicleSlot::cVehicleSlot()
{
	prev_takedamage = ( damage_t )-1;
	prev_solid = ( solid_t )-1;
	prev_contents = -1;
}

void cVehicleSlot::NotSolid
	(
	void
	)
{
	if( !( flags & FL_SWIM ) )
	{
		return;
	}

	if( !ent )
	{
		return;
	}

	// Save previous values
	prev_takedamage = ent->takedamage;
	prev_solid = ent->edict->solid;
	prev_contents = ent->edict->r.contents;
	
	ent->takedamage = DAMAGE_NO;
	ent->setSolidType( SOLID_NOT );
}

void cVehicleSlot::Solid
	(
	void
	)
{
	if( !( flags & FL_SWIM ) )
	{
		return;
	}

	if( !ent )
	{
		return;
	}

	// Restore previous values
	ent->takedamage = prev_takedamage;
	ent->setSolidType( prev_solid );
	ent->edict->r.contents = prev_contents;

	// Save previous values
	prev_takedamage = ( damage_t )-1;
	prev_solid = ( solid_t )-1;
	prev_contents = -1;
}

cTurretSlot::cTurretSlot()
{
	owner_prev_takedamage = ( damage_t )-1;
	owner_prev_solid = ( solid_t )-1;
	owner_prev_contents = -1;
}

void cTurretSlot::NotSolid
	(
	void
	)
{
	if( !( flags & FL_SWIM ) )
	{
		return;
	}

	if( !ent )
	{
		return;
	}

	cVehicleSlot::NotSolid();

	if( ent->IsSubclassOfTurretGun() )
	{
		TurretGun *turret = ( TurretGun * )ent.Pointer();
		Entity *owner = turret->GetOwner();

		if( owner )
		{
			owner_prev_takedamage = owner->takedamage;
			owner_prev_solid = owner->edict->solid;
			owner_prev_contents = owner->edict->r.contents;

			owner->takedamage = DAMAGE_NO;
			owner->setSolidType( SOLID_NOT );
		}
	}
}

void cTurretSlot::Solid
	(
	void
	)
{
	if( !( flags & FL_SWIM ) )
	{
		return;
	}

	if( !ent )
	{
		return;
	}

	cVehicleSlot::NotSolid();

	if( ent->IsSubclassOfTurretGun() )
	{
		TurretGun *turret = ( TurretGun * )ent.Pointer();
		Entity *owner = turret->GetOwner();

		if( owner )
		{
			owner->takedamage = owner_prev_takedamage;
			owner->setSolidType( owner_prev_solid );

			owner_prev_takedamage = ( damage_t )-1;
			owner_prev_solid = ( solid_t )-1;
		}
	}
}
