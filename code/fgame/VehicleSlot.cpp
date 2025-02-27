/*
===========================================================================
Copyright (C) 2024 the OpenMoHAA team

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
    prev_takedamage = (damage_t)-1;
    prev_solid      = (solid_t)-1;
    prev_contents   = -1;

    prev_num_children = 0;
}

void cVehicleSlot::NotSolid(void)
{
    int i;

    if (!(flags & SLOT_BUSY)) {
        return;
    }

    if (!ent) {
        return;
    }

    // Save previous values
    prev_takedamage = ent->takedamage;
    prev_solid      = ent->edict->solid;
    prev_contents   = ent->edict->r.contents;

    ent->takedamage = DAMAGE_NO;

    prev_num_children = 0;

    for (i = 0; i < ent->numchildren; i++) {
        Entity *sub = G_GetEntity(ent->children[i]);
        // Fixed in OPM
        //  Check for solidity
        if (sub && !sub->IsSubclassOfWeapon() && sub->getSolidType() != SOLID_NOT) {
            // Fixed in OPM
            //  Save children solidity so it can properly be restored later
            prev_children_ent[prev_num_children]   = sub;
            prev_children_solid[prev_num_children] = (solid_t)sub->getSolidType();
            prev_num_children++;

            sub->setSolidType(SOLID_NOT);
        }
    }

    // Added in 2.0
    // Commented in OPM
    //  Not sure why since 2.0, players aren't made non-solid in slots
    //if (!ent->IsSubclassOfPlayer()) {
    //    ent->setSolidType(SOLID_NOT);
    //}

    ent->setSolidType(SOLID_NOT);
}

void cVehicleSlot::Solid(void)
{
    int i, j;

    if (!(flags & FL_SWIM)) {
        return;
    }

    if (!ent) {
        return;
    }

    // Restore previous values
    ent->takedamage = prev_takedamage;
    ent->setSolidType(prev_solid);
    ent->setContents(prev_contents);

    // Save previous values
    prev_takedamage = (damage_t)-1;
    prev_solid      = (solid_t)-1;
    prev_contents   = -1;

    //
    // Fixed in OPM
    //  Restore children solidity
    //
    for (i = 0; i < ent->numchildren; i++) {
        Entity *sub = G_GetEntity(ent->children[i]);
        if (sub && !sub->IsSubclassOfWeapon()) {
            for (j = 0; j < prev_num_children; j++) {
                if (prev_children_ent[j] == sub) {
                    sub->setSolidType(prev_children_solid[j]);
                }
            }

            //sub->setSolidType(SOLID_NOT);
        }
    }

    prev_num_children = 0;
}

cTurretSlot::cTurretSlot()
{
    owner_prev_takedamage = (damage_t)-1;
    owner_prev_solid      = (solid_t)-1;
    owner_prev_contents   = -1;
}

void cTurretSlot::NotSolid(void)
{
    int i;

    if (!(flags & FL_SWIM)) {
        return;
    }

    if (!ent) {
        return;
    }

    // Save previous values
    prev_takedamage = ent->takedamage;
    prev_solid      = ent->edict->solid;
    prev_contents   = ent->edict->r.contents;

    ent->takedamage = DAMAGE_NO;
    ent->setSolidType(SOLID_NOT);

    if (ent->IsSubclassOfTurretGun()) {
        TurretGun *turret = static_cast<TurretGun *>(ent.Pointer());
        Entity    *owner  = turret->GetOwner();

        if (owner) {
            owner_prev_takedamage = owner->takedamage;
            owner_prev_solid      = owner->edict->solid;
            owner_prev_contents   = owner->edict->r.contents;

            owner->takedamage = DAMAGE_NO;
            owner->setSolidType(SOLID_NOT);
        }
    }

    prev_num_children = 0;

    for (i = 0; i < ent->numchildren; i++) {
        Entity *sub = G_GetEntity(ent->children[i]);
        // Fixed in OPM
        //  Check for solidity
        if (sub && !sub->IsSubclassOfWeapon() && sub->getSolidType() != SOLID_NOT) {
            // Fixed in OPM
            //  Save children solidity so it can properly be restored later
            prev_children_ent[prev_num_children]   = sub;
            prev_children_solid[prev_num_children] = (solid_t)sub->getSolidType();
            prev_num_children++;

            sub->setSolidType(SOLID_NOT);
        }
    }
}

void cTurretSlot::Solid(void)
{
    int i, j;

    if (!(flags & FL_SWIM)) {
        return;
    }

    if (!ent) {
        return;
    }

    // Restore previous values
    ent->takedamage = prev_takedamage;
    ent->setSolidType(prev_solid);

    // Save previous values
    prev_takedamage = (damage_t)-1;
    prev_solid      = (solid_t)-1;

    if (ent->IsSubclassOfTurretGun()) {
        TurretGun *turret = static_cast<TurretGun *>(ent.Pointer());
        Entity    *owner  = turret->GetOwner();

        if (owner) {
            owner->takedamage = owner_prev_takedamage;
            owner->setSolidType(owner_prev_solid);

            owner_prev_takedamage = (damage_t)-1;
            owner_prev_solid      = (solid_t)-1;
        }
    }

    //
    // Fixed in OPM
    //  Restore children solidity
    //
    for (i = 0; i < ent->numchildren; i++) {
        Entity *sub = G_GetEntity(ent->children[i]);
        if (sub && !sub->IsSubclassOfWeapon()) {
            for (j = 0; j < prev_num_children; j++) {
                if (prev_children_ent[j] == sub) {
                    sub->setSolidType(prev_children_solid[j]);
                }
            }

            //sub->setSolidType(SOLID_NOT);
        }
    }

    prev_num_children = 0;
}
