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

CLASS_DECLARATION(DrivableVehicle, VehicleWheelsX4, "VehicleWheelsX4")
{
	{ &EV_Damage, &VehicleWheelsX4::EventDamage },
	{ &EV_Killed, &VehicleWheelsX4::Killed },
	{ NULL, NULL }
};

VehicleWheelsX4::VehicleWheelsX4()
{
	// FIXME: STUB
}

void VehicleWheelsX4::UpdateVariables(Vector *acceleration, Vector *vpn, Vector *vup, Vector *vright, Vector *t_vpn, Vector *t_vup, Vector *t_vright)
{
	// FIXME: STUB
}

float VehicleWheelsX4::TorqueLookup(int rpm)
{
	// FIXME: STUB
	return 0.f;
}

void VehicleWheelsX4::Think()
{
	// FIXME: STUB
}

void VehicleWheelsX4::Postthink()
{
	// FIXME: STUB
}

void VehicleWheelsX4::Killed(Event* ev)
{
	// FIXME: STUB
}
