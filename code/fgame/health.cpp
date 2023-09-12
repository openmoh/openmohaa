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

// health.cpp: Health powerup
//

#include "g_local.h"
#include "item.h"
#include "inventoryitem.h"
#include "sentient.h"
#include "health.h"
#include "weaputils.h"
#include "player.h"
#include "g_phys.h"

Event EV_Health_PostSpawn("health_postspawn", EV_DEFAULT, NULL, NULL, "Health Post Spawn");

CLASS_DECLARATION(Item, Health, "health_020") {
    {&EV_Item_Pickup,      &Health::PickupHealth  },
    {&EV_Health_PostSpawn, &Health::EventPostSpawn},
    {NULL,                 NULL                   }
};

SafePtr<Health> Health::mHealthQueue[MAX_HEALTH_QUEUE];

Health::Health()
{
    if (DM_FLAG(DF_NO_HEALTH)) {
        PostEvent(EV_Remove, EV_REMOVE);
        return;
    }

    setAmount(20);
}

void Health::PickupHealth(Event *ev)
{
    Player *player;
    Entity *other;

    other = ev->GetEntity(1);
    if (!other || !other->IsSubclassOfPlayer()) {
        return;
    }

    player = (Player *)other;

    if (player->health >= player->max_health) {
        return;
    }

    if (!ItemPickup(other, qfalse)) {
        return;
    }

    if (g_healrate->value && other->IsSubclassOfPlayer()) {
        if (player->m_fHealRate + player->health >= player->max_health) {
            // will be healing to 100%
            return;
        }

        player->m_fHealRate += amount / 100.0 * player->max_health;
        if (player->m_fHealRate + player->health > player->max_health) {
            // make sure to not overflow
            player->m_fHealRate = player->max_health - player->health + 0.1f;
        }
    } else {
        player->health += amount / 100.0 * player->max_health;

        if (player->health > player->max_health) {
            player->health = player->max_health;
        }
    }

    gi.SendServerCommand(
        player->edict - g_entities,
        "print \"" HUD_MESSAGE_YELLOW "%s \"",
        gi.LV_ConvertString(va("Recovered %d Health", amount))
    );
}

void Health::EventPostSpawn(Event *ev)
{
    setMoveType(MOVETYPE_TOSS);

    if (edict->tiki) {
        vec3_t mins, maxs;

        gi.CalculateBounds(edict->tiki, edict->s.scale, mins, maxs);

        setSize(mins, maxs);
        link();
    }

    droptofloor(256);
}

void Health::CompressHealthQueue()
{
    int i, j;

    for (i = 0; i < MAX_HEALTH_QUEUE; i++) {
        if (!mHealthQueue[i]) {
            for (j = i + 1; j < MAX_HEALTH_QUEUE; j++) {
                if (mHealthQueue[j]) {
                    mHealthQueue[i] = mHealthQueue[j];
                    break;
                }
            }
        }
    }
}

void Health::AddToHealthQueue()
{
    CompressHealthQueue();

    if (mHealthQueue[MAX_HEALTH_QUEUE - 1]) {
        mHealthQueue[MAX_HEALTH_QUEUE - 1]->Delete();
        mHealthQueue[0] = NULL;
        CompressHealthQueue();
    }

    mHealthQueue[MAX_HEALTH_QUEUE - 1] = this;

    CompressHealthQueue();
}

void Health::ResetHealthQueue() {}

void Health::ArchiveStatic(Archiver& arc)
{
    int i;

    for (i = 0; i < MAX_HEALTH_QUEUE; i++) {
        arc.ArchiveSafePointer(&mHealthQueue[i]);
    }
}

void Health::DoRemoveProcess()
{
    if (g_gametype->integer != GT_SINGLE_PLAYER) {
        DoRemoveProcess();
    } else {
        AddToHealthQueue();
    }
}
