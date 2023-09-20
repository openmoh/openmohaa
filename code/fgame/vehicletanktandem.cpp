/*
===========================================================================
Copyright (C) 2023 the OpenMoHAA team

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

#include "vehicletanktandem.h"
#include "vehicleturret.h"

CLASS_DECLARATION(VehicleTank, VehicleTankTandem, "VehicleTankTandem") {
    {NULL, NULL}
};

VehicleTankTandem::VehicleTankTandem()
{
}

VehicleTankTandem::~VehicleTankTandem()
{
}

void VehicleTankTandem::SpawnTurret(Event *ev)
{
    VehicleTurretGunTandem* pTurret = new VehicleTurretGunTandem();
    int slot;

    pTurret->SetBaseOrientation(orientation, NULL);
    pTurret->setModel(ev->GetString(2));

    slot = ev->GetInteger(1);
    AttachTurretSlot(slot, pTurret, vec_zero, NULL);
    pTurret->SetVehicleOwner(this);

    PostEvent(EV_TakeDamage, EV_POSTSPAWN);

    UpdateTurretSlot(slot);
}

void VehicleTankTandem::UpdateTurretSlot(int iSlot)
{
    VehicleTank::UpdateTurretSlot(iSlot);

    if (Turrets[iSlot].ent && Turrets[iSlot].ent->isSubclassOf(VehicleTurretGunTandem)) {
        VehicleTurretGunTandem* pTurret = static_cast<VehicleTurretGunTandem*>(Turrets[iSlot].ent.Pointer());
        pTurret->UpdateLinkedTurret();
    }
}
