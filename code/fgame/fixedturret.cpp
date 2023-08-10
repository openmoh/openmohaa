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

#include "fixedturret.h"

CLASS_DECLARATION(VehicleTank, FixedTurret, NULL) {
    {NULL, NULL}
};

FixedTurret::FixedTurret()
{
    // FIXME: unimplemented
}

void FixedTurret::Archive(Archiver& arc)
{
    // FIXME: unimplemented
}

qboolean FixedTurret::Drive(usercmd_t *ucmd)
{
    // FIXME: unimplemented
    return qfalse;
}

void FixedTurret::DetachRemoteOwner()
{
    // FIXME: unimplemented
}

void FixedTurret::EventDamage(Event *ev)
{
    // FIXME: unimplemented
}

void FixedTurret::AttachDriverSlot(int slot, Entity *ent, Vector vExitPosition)
{
    // FIXME: unimplemented
}

void FixedTurret::DetachDriverSlot(int slot, Vector vExitPosition, Vector *vExitAngles)
{
    // FIXME: unimplemented
}

void FixedTurret::EventSetCollisionModel(Event *ev)
{
    // FIXME: unimplemented
}
