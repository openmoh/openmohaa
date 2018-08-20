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

CLASS_DECLARATION(DrivableVehicle, VehicleTank, "VehicleTank")
{
	{ &EV_Damage, &VehicleTank::EventDamage },
	{ NULL, NULL }
};

VehicleTank::VehicleTank()
{
	// FIXME: STUB
}

VehicleTank::~VehicleTank()
{
	// FIXME: STUB
}

qboolean VehicleTank::Drive(usercmd_t *ucmd)
{
	// FIXME: STUB
	return false;
}

void VehicleTank::EventDamage(Event *ev)
{
	// FIXME: STUB
}

void VehicleTank::Think()
{
	// FIXME: STUB
}

void VehicleTank::Postthink()
{
	// FIXME: STUB
}

void VehicleTank::CalculateOriginOffset()
{
	// FIXME: STUB
}

void VehicleTank::UpdateSound()
{
	// FIXME: STUB
}

void VehicleTank::AttachDriverSlot(Event* ev)
{
	// FIXME: STUB
}

