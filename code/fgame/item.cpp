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

// item.cpp: Base class for respawnable, carryable objects.
//

#include "g_local.h"
#include "g_phys.h"
#include "entity.h"
#include "trigger.h"
#include "item.h"
#include "inventoryitem.h"
#include "scriptmaster.h"
#include "health.h"
#include "game.h"

typedef struct {
    str  name;
    str  prefix;
    bool mohprefix;
} prefix_t;

static prefix_t r_prefixlist[256] = {
    {"Unarmed",                "unarmed",          true},
    {"Binoculars",             "binoculars",       true},
    {"Papers",                 "papers",           true},
    {"Packed MG42 Turret",     "mg42portable",     true},
    {"Colt 45",                "colt45",           true},
    {"Walther P38",            "p38",              true},
    {"Hi-Standard Silenced",   "histandard",       true},
    {"Webley Revolver",        "webley",           true},
    {"Nagant Revolver",        "nagantrev",        true},
    {"Beretta",                "beretta",          true},
    {"M1 Garand",              "garand",           true},
    {"Mauser KAR 98K",         "kar98",            true},
    {"KAR98 - Sniper",         "kar98sniper",      true},
    {"Springfield '03 Sniper", "springfield",      true},
    {"Lee-Enfield",            "enfield",          true},
    {"SVT 40",                 "svt",              true},
    {"Mosin Nagant Rifle",     "mosin",            true},
    {"G 43",                   "g43",              true},
    {"Enfield L42A1",          "enfieldl42a1",     true},
    {"Carcano",                "carcano",          true},
    {"DeLisle",                "delisle",          true},
    {"Thompson",               "thompson",         true},
    {"MP40",                   "mp40",             true},
    {"Sten Mark II",           "sten",             true},
    {"PPSH SMG",               "ppsh",             true},
    {"Moschetto",              "moschetto",        true},
    {"BAR",                    "bar",              true},
    {"StG 44",                 "mp44",             true},
    {"FG 42",                  "fg42",             true},
    {"Vickers-Berthier",       "vickers",          true},
    {"Breda",                  "breda",            true},
    {"Frag Grenade",           "fraggrenade",      true},
    {"Stielhandgranate",       "stielhandgranate", true},
    {"F1 Grenade",             "f1grenade",        true},
    {"Mills Grenade",          "millsgrenade",     true},
    {"Nebelhandgranate",       "nebelhandgranate", true},
    {"M18 Smoke Grenade",      "m18smokegrenade",  true},
    {"RDG-1 Smoke Grenade",    "rdg1smokegrenade", true},
    {"Bomba A Mano",           "bomba",            true},
    {"Bomba A Mano Breda",     "bombabreda",       true},
    {"Landmine",               "mine",             true},
    {"LandmineAllies",         "minedetector",     true},
    {"LandmineAxis",           "minedetectoraxis", true},
    {"Unarmed",                "detonator",        true},
    {"Bazooka",                "bazooka",          true},
    {"Panzerschreck",          "panzerschreck",    true},
    {"Gewehrgranate",          "kar98mortar",      true},
    {"Shotgun",                "shotgun",          true},
    {"PIAT",                   "PIAT",             true},
};

void AddItemToList(const char *name, const char *prefix)
{
    for (int i = 0; i < sizeof(r_prefixlist) / sizeof(r_prefixlist[0]); i++) {
        if (!r_prefixlist[i].name.length()) {
            r_prefixlist[i].name      = name;
            r_prefixlist[i].prefix    = prefix;
            r_prefixlist[i].mohprefix = false;
            return;
        }
    }
}

const char *GetItemName(const char *prefix, qboolean *mohprefix)
{
    for (int i = 0; i < sizeof(r_prefixlist) / sizeof(r_prefixlist[0]); i++) {
        if (!r_prefixlist[i].prefix.c_str()) {
            continue;
        }

        if (r_prefixlist[i].prefix == prefix) {
            if (mohprefix) {
                *mohprefix = r_prefixlist[i].mohprefix;
            }

            return r_prefixlist[i].name;
        }
    }

    if (mohprefix) {
        *mohprefix = false;
    }

    return "Unarmed";
}

const char *GetItemPrefix(const char *name, qboolean *mohprefix)
{
    for (int i = 0; i < sizeof(r_prefixlist) / sizeof(r_prefixlist[0]); i++) {
        if (r_prefixlist[i].name == name) {
            if (mohprefix) {
                *mohprefix = r_prefixlist[i].mohprefix;
            }

            return r_prefixlist[i].prefix;
        }
    }

    if (mohprefix) {
        *mohprefix = false;
    }

    return "unarmed";
}

Event EV_Item_Pickup
(
    "item_pickup",
    EV_DEFAULT,
    "e",
    "item",
    "Pickup the specified item.",
    EV_NORMAL
);
Event EV_Item_DropToFloor
(
    "item_droptofloor",
    EV_DEFAULT,
    NULL,
    NULL,
    "Drops the item to the ground.",
    EV_NORMAL
);
Event EV_Item_Respawn
(
    "respawn",
    EV_DEFAULT,
    NULL,
    NULL,
    "Respawns the item.",
    EV_NORMAL
);
Event EV_Item_SetRespawn
(
    "set_respawn",
    EV_DEFAULT,
    "i",
    "respawn",
    "Turns respawn on or off.",
    EV_NORMAL
);
Event EV_Item_SetRespawnTime
(
    "set_respawn_time",
    EV_DEFAULT,
    "f",
    "respawn_time",
    "Sets the respawn time.",
    EV_NORMAL
);
Event EV_Item_SetAmount
(
    "amount",
    EV_DEFAULT,
    "i",
    "amount",
    "Sets the amount of the item.",
    EV_NORMAL
);
Event EV_Item_SetMaxAmount
(
    "maxamount",
    EV_DEFAULT,
    "i",
    "max_amount",
    "Sets the max amount of the item.",
    EV_NORMAL
);
Event EV_Item_SetDMAmount
(
    "dmamount",
    EV_DEFAULT,
    "i",
    "amount",
    "Sets the amount of the item for DM.",
    EV_NORMAL
);
Event EV_Item_SetDMMaxAmount
(
    "dmmaxamount",
    EV_DEFAULT,
    "i",
    "max_amount",
    "Sets the max amount of the item fmr DM.",
    EV_NORMAL
);
Event EV_Item_SetItemName
(
    "name",
    EV_DEFAULT,
    "s",
    "item_name",
    "Sets the item name.",
    EV_NORMAL
);
Event EV_Item_RespawnSound
(
    "respawnsound",
    EV_DEFAULT,
    NULL,
    NULL,
    "Turns on the respawn sound for this item.",
    EV_NORMAL
);
Event EV_Item_DialogNeeded
(
    "dialogneeded",
    EV_DEFAULT,
    "s",
    "dialog_needed",
    "Sets the dialog needed string.",
    EV_NORMAL
);
Event EV_Item_NoRemove
(
    "no_remove",
    EV_DEFAULT,
    NULL,
    NULL,
    "Makes it so the item is not removed from the world when it is picked up.",
    EV_NORMAL
);
Event EV_Item_RespawnDone
(
    "respawn_done",
    EV_DEFAULT,
    NULL,
    NULL,
    "Called when the item respawn is done.",
    EV_NORMAL
);
Event EV_Item_PickupDone
(
    "pickup_done",
    EV_DEFAULT,
    NULL,
    NULL,
    "Called when the item pickup is done.",
    EV_NORMAL
);

Event EV_Item_SetPickupSound
(
    "pickupsound",
    EV_DEFAULT,
    "s",
    "name",
    "sets the item's pickup sound alias",
    EV_NORMAL
);

Event EV_Item_ViewModelPrefix
(
    "viewmodelprefix",
    EV_DEFAULT,
    "s",
    "prefix",
    "Sets the item's prefix for viewmodelanim.",
    EV_NORMAL
);

Event EV_Item_UpdatePrefix
(
    "_updateprefix",
    EV_CODEONLY,
    NULL,
    NULL,
    "internal event - update the custom viewmodel prefix",
    EV_NORMAL
);

CLASS_DECLARATION(Trigger, Item, NULL) {
    {&EV_Trigger_Effect,       &Item::ItemTouch            },
    {&EV_Item_DropToFloor,     &Item::DropToFloor          },
    {&EV_Item_Respawn,         &Item::Respawn              },
    {&EV_Item_SetAmount,       &Item::SetAmountEvent       },
    {&EV_Item_SetMaxAmount,    &Item::SetMaxAmount         },
    {&EV_Item_SetItemName,     &Item::SetItemName          },
    {&EV_Item_Pickup,          &Item::Pickup               },
    {&EV_Use,                  &Item::TriggerStuff         },
    {&EV_Item_RespawnSound,    &Item::RespawnSound         },
    {&EV_Item_DialogNeeded,    &Item::DialogNeeded         },
    {&EV_Item_NoRemove,        &Item::SetNoRemove          },
    {&EV_Item_RespawnDone,     &Item::RespawnDone          },
    {&EV_Item_PickupDone,      &Item::PickupDone           },
    {&EV_Item_SetRespawn,      &Item::setRespawn           },
    {&EV_Item_SetRespawnTime,  &Item::setRespawnTime       },
    {&EV_Stop,                 &Item::Landed               },
    {&EV_SetAngle,             &SimpleEntity::SetAngleEvent},
    {&EV_Item_SetDMAmount,     &Item::SetDMAmountEvent     },
    {&EV_Item_SetDMMaxAmount,  &Item::SetDMMaxAmount       },
    {&EV_Item_SetPickupSound,  &Item::SetPickupSound       },
    {&EV_Item_ViewModelPrefix, &Item::EventViewModelPrefix },
    {&EV_Item_UpdatePrefix,    &Item::updatePrefix         },
    {NULL,                     NULL                        }
};

Item::Item()
{
    str fullname;

    entflags |= ECF_ITEM;

    AddWaitTill(STRING_PICKUP);

    if (LoadingSavegame) {
        return;
    }

    setSolidType(SOLID_NOT);

    // Set default respawn behavior
    // Derived classes should use setRespawn
    // if they want to override the default behavior
    setRespawn(deathmatch->integer ? true : false);
    setRespawnTime(20);

    //
    // set a minimum mins and maxs for the model
    //
    if (size.length() < 10) {
        mins = "-10 -10 0";
        maxs = "10 10 20";
    }

    //
    // reset the mins and maxs to pickup the FL_ROTATEDBOUNDS flag
    //
    setSize(mins, maxs);

    // Items can't be immediately dropped to floor, because they might
    // be on an entity that hasn't spawned yet.
    PostEvent(EV_Item_DropToFloor, EV_POSTSPAWN);

    respondto = TRIGGER_PLAYERS;

    // items should collide with everything that the player does
    edict->clipmask = MASK_ITEM;

    item_index     = 0;
    maximum_amount = 1;
    playrespawn    = false;

    // this is an item entity
    edict->s.eType = ET_ITEM;

    amount    = 1;
    no_remove = false;
    setName("Unknown Item");

    sPickupSound = "snd_pickup";

    m_sVMprefix  = "Unarmed";
    m_bMOHPrefix = true;
}

Item::~Item()
{
    if (owner) {
        owner->RemoveItem(this);
        owner = NULL;
    }

    entflags &= ~ECF_ITEM;
}

void Item::RemoveFromOwner(void)
{
    owner->RemoveItem(this);
    owner = NULL;
}

void Item::Delete(void)
{
    if (g_iInThinks) {
        if (owner) {
            RemoveFromOwner();
        }

        PostEvent(EV_Remove, 0);
    } else {
        delete this;
    }
}

void Item::SetNoRemove(Event *ev)
{
    no_remove = true;
}

/*
============
PlaceItem

Puts an item back in the world
============
*/
void Item::PlaceItem(void)
{
    setSolidType(SOLID_TRIGGER);
    setMoveType(MOVETYPE_TOSS);
    showModel();

    groundentity = NULL;

    setSize(Vector(-12, -12, -2), Vector(12, 12, 12));
}

/*
============
DropToFloor

plants the object on the floor
============
*/
void Item::DropToFloor(Event *ev)
{
    str    fullname;
    Vector save;

    PlaceItem();
    setMoveType(MOVETYPE_NONE);

    /*
	addOrigin( Vector( "0 0 1" ) );

	save = origin;
	if( !droptofloor( 8192 ) )
	{
		gi.DPrintf( "%s (%d) stuck in world at '%5.1f %5.1f %5.1f'\n",
			getClassID(), entnum, origin.x, origin.y, origin.z );
		setOrigin( save );
		setMoveType( MOVETYPE_NONE );
	}
	else
	{
		setMoveType( MOVETYPE_NONE );
	}
	//
	// if the our global variable doesn't exist, lets zero it out
	//
	fullname = str( "playeritem_" ) + getName();

	game.vars->GetOrCreateVariable( fullname.c_str() );
	level.vars->GetOrCreateVariable( fullname.c_str() );
	*/
}

qboolean Item::Drop(void)
{
    if (!owner) {
        return false;
    }

    setOrigin(owner->origin + Vector("0 0 40"));

    // drop the item
    PlaceItem();
    velocity = owner->velocity * 0.5 + Vector(G_CRandom(50), G_CRandom(50), 100);
    setAngles(owner->angles);
    avelocity = Vector(0, G_CRandom(360), 0);

    trigger_time = level.time + 1;

    if (owner->isClient()) {
        spawnflags |= DROPPED_PLAYER_ITEM;
    } else {
        spawnflags |= DROPPED_ITEM;
    }

    // Remove this from the owner's item list
    RemoveFromOwner();

    PostEvent(EV_Remove, g_droppeditemlife->value);

    return true;
}

void Item::ItemTouch(Event *ev)

{
    Entity *other;
    Event  *e;

    if (owner) {
        // Don't respond to trigger events after item is picked up.
        // we really don't need to see this.
        //gi.DPrintf( "%s with targetname of %s was triggered unexpectedly.\n", getClassID(), TargetName() );
        return;
    }

    other = ev->GetEntity(1);

    e = new Event(EV_Item_Pickup);
    e->AddEntity(other);
    ProcessEvent(e);
}

void Item::SetOwner(Sentient *ent)
{
    assert(ent);
    if (!ent) {
        // return to avoid any buggy behaviour
        return;
    }

    owner = ent;
    setRespawn(false);

    setSolidType(SOLID_NOT);
    hideModel();
    CancelEventsOfType(EV_Touch);
    CancelEventsOfType(EV_Item_DropToFloor);
    CancelEventsOfType(EV_Remove);

    Event *ev = new Event(EV_Item_UpdatePrefix);
    ev->AddEntity(ent);

    PostEvent(ev, EV_POSTSPAWN);
}

Sentient *Item::GetOwner(void)
{
    return owner;
}

Item *Item::ItemPickup(Entity *other, qboolean add_to_inventory)
{
    Sentient *sent;
    Item     *item = NULL;
    str       realname;

    if (!Pickupable(other)) {
        return NULL;
    }

    sent = (Sentient *)other;

    if (add_to_inventory) {
        item = sent->giveItem(model, getAmount());

        if (!item) {
            return NULL;
        }
    } else {
        item = this;
    }

    //
    // let our sent know they received it
    // we put this here so we can transfer information from the original item we picked up
    //
    sent->ReceivedItem(item);

    Sound(sPickupSound);

    if (!Removable()) {
        // leave the item for others to pickup
        return item;
    }

    CancelEventsOfType(EV_Item_DropToFloor);
    CancelEventsOfType(EV_Item_Respawn);
    CancelEventsOfType(EV_FadeOut);

    setSolidType(SOLID_NOT);

    if (HasAnim("pickup")) {
        NewAnim("pickup", EV_Item_PickupDone);
    } else {
        if (!no_remove) {
            hideModel();

            if (!Respawnable()) {
                PostEvent(EV_Remove, FRAMETIME);
            }
        }
    }

    if (Respawnable()) {
        PostEvent(EV_Item_Respawn, RespawnTime());
    }

    return item;
}

void Item::Respawn(Event *ev)
{
    showModel();

    // allow it to be touched again
    setSolidType(SOLID_TRIGGER);

    // play respawn sound
    if (playrespawn) {
        Sound("snd_itemspawn");
    }

    setOrigin();

    if (HasAnim("respawn")) {
        NewAnim("respawn", EV_Item_RespawnDone);
    }
}

void Item::setRespawn(Event *ev)

{
    if (ev->NumArgs() < 1) {
        return;
    }

    setRespawn(ev->GetInteger(1));
}

void Item::setRespawnTime(Event *ev)

{
    if (ev->NumArgs() < 1) {
        return;
    }

    setRespawnTime(ev->GetFloat(1));
}

void Item::RespawnDone(Event *ev)
{
    NewAnim("idle");
}

void Item::PickupDone(Event *ev)
{
    if (!no_remove) {
        hideModel();

        if (!Respawnable()) {
            PostEvent(EV_Remove, FRAMETIME);
        }
    } else {
        if (HasAnim("pickup_idle")) {
            NewAnim("pickup_idle");
        } else {
            NewAnim("pickup");
        }
    }
}

void Item::setRespawn(qboolean flag)
{
    respawnable = flag;
}

qboolean Item::Respawnable(void)

{
    return respawnable;
}

void Item::setRespawnTime(float time)
{
    respawntime = time;
}

float Item::RespawnTime(void)

{
    return respawntime;
}

int Item::getAmount(void)

{
    return amount;
}

int Item::MaxAmount(void)

{
    return maximum_amount;
}

qboolean Item::Pickupable(Entity *other)
{
    if (!other->IsSubclassOfSentient()) {
        return false;
    } else {
        Sentient *sent;
        Item     *item;

        sent = (Sentient *)other;

        item = sent->FindItem(getName());

        if (item && (item->getAmount() >= item->MaxAmount())) {
            return false;
        }
    }
    return true;
}

void Item::Pickup(Event *ev)
{
    ItemPickup(ev->GetEntity(1));
}

void Item::setName(const char *i)
{
    const char *prefix;

    item_name  = i;
    item_index = gi.itemindex(i);
    strcpy(edict->entname, i);

    prefix = GetItemPrefix(item_name);
    if (prefix) {
        m_sVMprefix  = prefix;
        m_bMOHPrefix = true;
    }
}

str Item::getName(void)

{
    return (item_name);
}

int Item::getIndex(void)

{
    return item_index;
}

void Item::setAmount(int startamount)

{
    amount = startamount;
    if (amount >= MaxAmount()) {
        SetMax(amount);
    }
}

void Item::SetMax(int maxamount)

{
    maximum_amount = maxamount;
}

void Item::SetAmountEvent(Event *ev)
{
    if (g_protocol <= protocol_e::PROTOCOL_MOH && g_gametype->integer != GT_SINGLE_PLAYER) {
        return;
    }
    setAmount(ev->GetInteger(1));
}

void Item::SetMaxAmount(Event *ev)

{
    if (g_protocol <= protocol_e::PROTOCOL_MOH && g_gametype->integer != GT_SINGLE_PLAYER) {
        return;
    }
    SetMax(ev->GetInteger(1));
}

void Item::SetDMAmountEvent(Event *ev)
{
    if (!g_gametype->integer) {
        return;
    }
    setAmount(ev->GetInteger(1));
}

void Item::SetDMMaxAmount(Event *ev)
{
    if (!g_gametype->integer) {
        return;
    }
    setAmount(ev->GetInteger(1));
}

void Item::SetPickupSound(Event *ev)
{
    sPickupSound = ev->GetString(1);
}

void Item::SetItemName(Event *ev)

{
    setName(ev->GetString(1));
}

void Item::Add(int num)

{
    amount += num;
    if (amount >= MaxAmount()) {
        amount = MaxAmount();
    }
}

void Item::Remove(int num)
{
    amount -= num;
    if (amount < 0) {
        amount = 0;
    }
}

qboolean Item::Use(int num)

{
    if (num > amount) {
        return false;
    }

    amount -= num;
    return true;
}

qboolean Item::Removable(void)

{
    return true;
}

void Item::RespawnSound(Event *ev)

{
    playrespawn = true;
}

void Item::DialogNeeded(Event *ev)

{
    //
    // if this item is needed for a trigger, play this dialog
    //
    dialog_needed = ev->GetString(1);
}

str Item::GetDialogNeeded(void)

{
    return dialog_needed;
}

//
// once item has landed on the floor, go to movetype none
//
void Item::Landed(Event *ev)
{
    if (groundentity && (groundentity->entity != world)) {
        warning("Item::Landed", "Item %d has landed on an entity that might move\n", entnum);
    }
    setMoveType(MOVETYPE_NONE);
}

void Item::EventViewModelPrefix(Event *ev)
{
    int        i;
    gentity_t *ent;

    m_sVMprefix = ev->GetString(1);

    if (!GetItemPrefix(item_name, &m_bMOHPrefix)) {
        AddItemToList(item_name, m_sVMprefix);
    }

    for (i = 0, ent = g_entities; i < game.maxclients; i++, ent++) {
        if (!ent->inuse || !ent->entity) {
            continue;
        }

        Event *ev = new Event(EV_Item_UpdatePrefix);
        ev->AddEntity(ent->entity);

        PostEvent(ev, EV_POSTSPAWN);
    }
}

void Item::updatePrefix(Event *ev)
{
    if (!level.specialgame) {
        return;
    }

    if (m_bMOHPrefix) {
        return;
    }

    // FIXME: delete
    /*
	Entity *ent = ev->GetEntity( 1 );

	gi.MSG_SetClient( ent->edict - g_entities );

	gi.MSG_StartCGM( CGM_VIEWMODELPREFIX );
		gi.MSG_WriteString( item_name.c_str() );
		gi.MSG_WriteString( m_sVMprefix.c_str() );
	gi.MSG_EndCGM();
	*/
}

CLASS_DECLARATION(Item, DynItem, NULL) {
    {&EV_Kill,   &DynItem::UnlinkItem},
    {&EV_Damage, &DynItem::UnlinkItem},
    {NULL,       NULL                }
};

DynItem::DynItem()
{
    if (LoadingSavegame) {
        return;
    }

    setSolidType(SOLID_BBOX);
    setMoveType(MOVETYPE_BOUNCE);
    takedamage = DAMAGE_YES;
}

void DynItem::UnlinkItem(Event *ev)
{
    if (!owner) {
        return;
    }

    setOrigin(owner->origin + Vector(0, 0, 40));
    PlaceItem();

    velocity = owner->velocity * 0.5 + Vector(G_CRandom(50), G_CRandom(50), 100);
    setAngles(owner->angles);
    avelocity = Vector(0, G_CRandom(360), 0);

    trigger_time = level.time + 1;

    if (owner->isClient()) {
        spawnflags |= DROPPED_PLAYER_ITEM;
    } else {
        spawnflags |= DROPPED_ITEM;
    }

    // Remove this from the owner's item list
    RemoveFromOwner();
}

void DynItem::DynItemTouched(Event *ev)
{
    // Empty
}

void DynItem::DynItemUse(Event *ev)
{
    // Empty
}

void DynItem::Archive(Archiver& arc)
{
    Item::Archive(arc);

    arc.ArchiveString(&m_attachPrime);
    arc.ArchiveString(&m_attachSec);
    arc.ArchiveString(&m_dynItemName);

    if (arc.Loading()) {
        setName(m_dynItemName.c_str());
    }
}
