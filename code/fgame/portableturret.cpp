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

#include "portableturret.h"
#include "g_spawn.h"

Event EV_CarryableTurret_Drop
(
    "dropturret",
    EV_DEFAULT,
    NULL,
    NULL,
    "Makes the carryable turret have itself dropped",
    EV_NORMAL
);
Event EV_CarryableTurret_TurretTik
(
    "turrettik",
    EV_DEFAULT,
    NULL,
    NULL,
    "Tiki of the turret to place",
    EV_NORMAL
);
Event EV_CarryableTurret_Place
(
    "placeturret",
    EV_DEFAULT,
    NULL,
    NULL,
    "Try to place the carryable turret",
    EV_NORMAL
);
Event EV_CarryableTurret_EnablePickup
(
    "_internalenablepickup",
    EV_DEFAULT,
    NULL,
    NULL,
    NULL,
    EV_NORMAL
);

CLASS_DECLARATION(InventoryItem, CarryableTurret, NULL) {
    {&EV_Touch,                        NULL                              },
    {&EV_Item_Pickup,                  &CarryableTurret::EventPickup     },
    {&EV_CarryableTurret_Drop,         &CarryableTurret::DropTurret      },
    {&EV_CarryableTurret_TurretTik,    &CarryableTurret::EventTurretTik  },
    {&EV_CarryableTurret_Place,        &CarryableTurret::EventPlaceTurret},
    {&EV_CarryableTurret_EnablePickup, &CarryableTurret::EventPickup     },
    {NULL,                             NULL                              }
};

CarryableTurret::CarryableTurret()
{
    // FIXME: unimplemented
}

void CarryableTurret::EventPickup(Event *ev)
{
    // FIXME: unimplemented
}

qboolean CarryableTurret::Removable(void)
{
    return qtrue;
}

void CarryableTurret::DetachFromOwner(void)
{
    // FIXME: unimplemented
}

void CarryableTurret::AttachToHolster(weaponhand_t hand)
{
    // FIXME: unimplemented
}

void CarryableTurret::DropTurret(Event *ev)
{
    // FIXME: unimplemented
}

qboolean CarryableTurret::Drop(void)
{
    qboolean result = Weapon::Drop();
    if (result) {
        spawnflags &= ~SPAWNFLAG_NOPC;
        CancelEventsOfType(EV_Remove);
    }

    return result;
}

void CarryableTurret::EventTurretTik(Event *ev)
{
    // FIXME: unimplemented
}

void CarryableTurret::EventPlaceTurret(Event *ev)
{
    // FIXME: unimplemented
}

void CarryableTurret::EventEnablePickup(Event *ev)
{
    // FIXME: unimplemented
}

CLASS_DECLARATION(TurretGun, PortableTurret, NULL) {
    {&EV_Item_DropToFloor,       &PortableTurret::PortablePlaceTurret},
    {&EV_Weapon_SetAmmoClipSize, &PortableTurret::EventSetClipSize   },
    {NULL,                       NULL                                }
};

PortableTurret::PortableTurret()
{
    // FIXME: unimplemented
}

void PortableTurret::SetGroundPitch(float pitch)
{
    // FIXME: unimplemented
}

void PortableTurret::SetOwnerPosition(const Vector& pos)
{
    // FIXME: unimplemented
}

void PortableTurret::PortablePlaceTurret(Event *ev)
{
    // FIXME: unimplemented
}

void PortableTurret::MakeBaseEntity()
{
    // FIXME: unimplemented
}

void PortableTurret::EventSetClipSize(Event *ev)
{
    // FIXME: unimplemented
}

void PortableTurret::AbortTurretSetup()
{
    // FIXME: unimplemented
}

void PortableTurret::P_UserAim(usercmd_t *cmd)
{
    // FIXME: unimplemented
}

void PortableTurret::P_TurretEndUsed()
{
    // FIXME: unimplemented
}

void PortableTurret::P_TurretBeginUsed(Player *pEnt)
{
    // FIXME: unimplemented
}

void PortableTurret::P_ThinkActive()
{
    // FIXME: unimplemented
}

void PortableTurret::P_TurretUsed(Player *player)
{
    // FIXME: unimplemented
}
