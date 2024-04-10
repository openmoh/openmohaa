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
// VehicleCollisionEntity.cpp : MOHAA Vehicle Collision

#include "VehicleCollisionEntity.h"
#include "g_phys.h"

Event EV_Owner
(
    "owner",
    EV_DEFAULT,
    NULL,
    NULL,
    "owner",
    EV_GETTER
);

CLASS_DECLARATION(Entity, VehicleCollisionEntity, NULL) {
    {&EV_Damage, &VehicleCollisionEntity::EventDamage},
    {&EV_Owner,  &VehicleCollisionEntity::GetOwner   },
    {NULL,       NULL                                }
};

VehicleCollisionEntity::VehicleCollisionEntity(Entity *ent)
{
    if (LoadingSavegame) {
        return;
    }

    edict->s.eType = ET_GENERAL;

    m_pOwner   = ent;
    takedamage = DAMAGE_YES;

    showModel();
    setMoveType(MOVETYPE_PUSH);
    setSolidType(SOLID_NOT);

    edict->clipmask |= MASK_VEHICLE;
    edict->s.eFlags |= EF_LINKANGLES;
}

VehicleCollisionEntity::VehicleCollisionEntity(void)
{
    if (LoadingSavegame) {
        return;
    }

    gi.Error(ERR_DROP, "VehicleCollisionEntity Created with no parameters!\n");
}

void VehicleCollisionEntity::GetOwner(Event *ev)
{
    ev->AddListener(m_pOwner);
}

void VehicleCollisionEntity::Solid(void)
{
    setContents(CONTENTS_SOLID);
    setSolidType(SOLID_BSP);
}

void VehicleCollisionEntity::NotSolid(void)
{
    setSolidType(SOLID_NOT);
}

void VehicleCollisionEntity::EventDamage(Event *ev)
{
    m_pOwner->ProcessEvent(*ev);
}

void VehicleCollisionEntity::Used(Event *ev)
{
    m_pOwner->ProcessEvent(*ev);
}

Entity *VehicleCollisionEntity::GetOwner() const
{
    return m_pOwner;
}
