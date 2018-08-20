/*
===========================================================================
Copyright (C) 2018 the OpenMoHAA team

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

#include "vehicle.h"
#include "player.h"

CLASS_DECLARATION(DrivableVehicle, VehicleHalfTrack, "VehicleHalfTrack")
{
	{ &EV_Damage, &VehicleHalfTrack::EventDamage },
	{ &EV_Killed, &VehicleHalfTrack::Killed },
	{ NULL, NULL }
};

VehicleHalfTrack::VehicleHalfTrack()
{
	// FIXME: STUB
}

void VehicleHalfTrack::Think()
{
	// FIXME: STUB
}

void VehicleHalfTrack::Postthink()
{
	// FIXME: STUB
}

void VehicleHalfTrack::Killed(Event* ev)
{
	// FIXME: STUB
}
