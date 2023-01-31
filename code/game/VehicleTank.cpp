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
#include "vehicleturret.h"
#include "player.h"

CLASS_DECLARATION(DrivableVehicle, VehicleTank, "VehicleTank")
{
	{ &EV_Damage, &VehicleTank::EventDamage },
	{ NULL, NULL }
};

VehicleTank::VehicleTank()
{
	entflags |= FL_ROTATEDBOUNDS;
}

VehicleTank::~VehicleTank()
{
	entflags &= ~FL_ROTATEDBOUNDS;
}

qboolean VehicleTank::Drive(usercmd_t *ucmd)
{
	Vector i;
	Vector j;
	Vector k;
	Vector vTmp;

	vTmp = velocity;
	VectorNormalize(vTmp);

	if (!driver.ent || !driver.ent->client)
	{
		return qfalse;
	}

	if (!drivable)
	{
		driver.ent->client->ps.pm_flags |= PMF_FROZEN;
		ucmd->forwardmove = 0;
		ucmd->rightmove = 0;
		ucmd->upmove = 0;
		return qfalse;
	}

	moveimpulse = ucmd->forwardmove * (vTmp.length() + 1.0);
	
	m_bIsBraking = ucmd->forwardmove >> 31;

	m_fAccelerator += ucmd->forwardmove * 0.005;

	if (m_fAccelerator < 0)
	{
		m_fAccelerator = 0.0;
	}

	turnimpulse = -ucmd->rightmove;

	VehicleTurretGun *vtg = (VehicleTurretGun *)Turrets[0].ent.Pointer();

	if (vtg && vtg->IsSubclassOfVehicleTurretGun() && driver.ent->IsSubclassOfSentient())
	{
		vtg->RemoteControl(ucmd, (Sentient *)driver.ent.Pointer());

		if (ucmd->upmove)
			turnimpulse -= AngleSubtract(angles[1], vtg->angles[1]);
	}
	return qtrue;
}

void VehicleTank::EventDamage(Event *ev)
{
	int mod;
	mod = ev->GetInteger(9);

	if (mod > MOD_VEHICLE && mod != MOD_SHOTGUN && mod != MOD_GRENADE)
	{
		Vehicle::EventDamage(ev);
		if (g_gametype->integer == GT_SINGLE_PLAYER)
		{
			Player *p = (Player *)driver.ent.Pointer();
			if (p)
			{
				if (p->IsSubclassOfPlayer())
				{
					p->m_iNumHitsTaken++;
				}
			}
		}
	}
}

void VehicleTank::Think()
{
	flags |= FL_POSTTHINK;
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
	Vehicle::AttachDriverSlot(ev);

	VehicleTurretGun *vtg = (VehicleTurretGun *)Turrets[0].ent.Pointer();

	if (vtg && vtg->IsSubclassOfVehicleTurretGun() && driver.ent->IsSubclassOfSentient())
	{
		vtg->SetOwner((Sentient *)driver.ent.Pointer());
		Think();
		Postthink();
		vtg->Think();
	}

}

