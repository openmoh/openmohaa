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

// ammo.cpp: Base class for all ammunition for entities derived from the Weapon class.
//
// AmmoEntity is the Class which represents ammo that the player "sees" and "
// picks up" in the game
//
// Ammo is the Class which is used to keep track of how much ammo a player has
// in his inventory

#include "ammo.h"
#include "player.h"

CLASS_DECLARATION(Item, AmmoEntity, NULL) {
    {NULL, NULL}
};

AmmoEntity::AmmoEntity()

{
    if (LoadingSavegame) {
        // all data will be setup by the archive function
        return;
    }
    setName("UnknownAmmo");
    amount = 0;
}

Item *AmmoEntity::ItemPickup(Entity *other, qboolean add_to_inventory)

{
    Sentient *player;
    Weapon* pWeap;

    if (!other->IsSubclassOfPlayer()) {
        return NULL;
    }

    player = (Sentient *)other;

    if (player->AmmoCount(item_name.c_str()) == player->MaxAmmoCount(item_name.c_str())) {
        // don't pickup if the player already has max ammo
        return NULL;
    }

    // Play pickup sound
    player->Sound(sPickupSound, CHAN_ITEM);

    // Cancel some events
    CancelEventsOfType(EV_Item_DropToFloor);
    CancelEventsOfType(EV_Item_Respawn);
    CancelEventsOfType(EV_FadeOut);

    // Hide the model
    setSolidType(SOLID_NOT);
    hideModel();

    // Respawn?
    if (!Respawnable()) {
        PostEvent(EV_Remove, FRAMETIME);
    } else {
        PostEvent(EV_Item_Respawn, RespawnTime());
    }

    // Notify scripts about the pickup
    Unregister(STRING_PICKUP);

    // Tell the player about the ammo being picked
    gi.SendServerCommand(other->edict - g_entities, "print \"" HUD_MESSAGE_YELLOW "%s\"", gi.LV_ConvertString(va("Got %d %s Rounds", amount, item_name.c_str())));

    // Give the ammo to the player
    player->GiveAmmo(item_name, amount);

    // Make the weapon reload if the weapon has an empty clip
    pWeap = player->GetActiveWeapon(WEAPON_MAIN);
    if (pWeap && !pWeap->HasAmmoInClip(FIRE_PRIMARY) && pWeap->CheckReload(FIRE_PRIMARY)) {
        pWeap->SetShouldReload(qtrue);
    }

    return NULL; // This doesn't create any items
}

// This is the Class that is used to keep track of ammo in the player's inventory.
// It is not an entit, just a name and an amount.

CLASS_DECLARATION(Class, Ammo, NULL) {
    {NULL, NULL}
};

Ammo::Ammo()

{
    if (LoadingSavegame) {
        // all data will be setup by the archive function
        return;
    }
    setName("UnknownAmmo");
    setAmount(0);
    setMaxAmount(100);
}

void Ammo::setAmount(int a)

{
    amount = a;

    if ((maxamount > 0) && (amount > maxamount)) {
        amount = maxamount;
    }
}

int Ammo::getAmount(void)

{
    return amount;
}

void Ammo::setMaxAmount(int a)

{
    maxamount = a;
}

int Ammo::getMaxAmount(void)

{
    return maxamount;
}

void Ammo::setName(str n)

{
    name       = n;
    name_index = gi.itemindex(name) + CS_WEAPONS;
}

str Ammo::getName(void)

{
    return name;
}

int Ammo::getIndex(void)

{
    return name_index;
}
