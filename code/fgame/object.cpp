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

// object.cpp : Object (used by common TIKIs)

#include "g_local.h"
#include "object.h"
#include "sentient.h"
#include "misc.h"
#include "explosion.h"
#include "gibs.h"
#include "specialfx.h"
#include "g_phys.h"
#include "game.h"

Event EV_Object_HandleSetModel
(
    "handlespawn",
    EV_DEFAULT,
    NULL,
    NULL,
    "Internal usage",
    EV_NORMAL
);

CLASS_DECLARATION(Animate, Object, NULL) {
    {NULL, NULL}
};

Event EV_InteractObject_Setup("_setup", EV_DEFAULT, NULL, NULL, "Sets up an object.");

Event EV_InteractObject_KilledEffect
(
    "killedeffect",
    EV_DEFAULT,
    NULL,
    NULL,
    "Sets the tiki it will spawn when it's destroyed",
    EV_NORMAL
);

Event EV_InteractObject_HitEffect
(
    "hiteffect",
    EV_DEFAULT,
    NULL,
    NULL,
    "Sets the tiki it will spawn when it's hit",
    EV_NORMAL
);

CLASS_DECLARATION(Animate, InteractObject, "interactobject") {
    {&EV_Damage,                      &InteractObject::Damaged          },
    {&EV_Killed,                      &InteractObject::Killed           },
    {&EV_InteractObject_Setup,        &InteractObject::Setup            },
    {&EV_InteractObject_HitEffect,    &InteractObject::EventHitEffect   },
    {&EV_InteractObject_KilledEffect, &InteractObject::EventKilledEffect},
    {NULL,                            NULL                              }
};

InteractObject::InteractObject() {}

void InteractObject::Setup(Event *ev)
{
    if (!health) {
        // Set the bounding box as health
        health = (maxs - mins).length();
    }

    max_health = health;
    deadflag   = DEAD_NO;

    NewAnim("idle");
    link();
}

void InteractObject::EventHitEffect(Event *ev)
{
    m_sHitEffect = ev->GetString(1);
}

void InteractObject::EventKilledEffect(Event *ev)
{
    m_sKilledEffect = ev->GetString(1);
}

void InteractObject::Damaged(Event *ev)
{
    if (m_sHitEffect.length()) {
        // Spawn a temporary hit effect
        Animate *temp = new Animate();
        temp->PostEvent(EV_Remove, 1.f);
        temp->setModel(m_sHitEffect);
        temp->NewAnim("idle");
    }

    Entity::DamageEvent(ev);
}

void InteractObject::Killed(Event *ev)
{
    SimpleEntity *ent;
    Entity       *attacker;
    Vector        dir;
    const char   *name;

    takedamage = DAMAGE_NO;
    deadflag   = DEAD_NO;
    setSolidType(SOLID_NOT);
    edict->s.renderfx |= RF_DONTDRAW;

    if (edict->solid == SOLID_NOT || edict->solid == SOLID_TRIGGER) {
        edict->r.svFlags |= SVF_NOCLIENT;
    }

    if (m_sKilledEffect.length()) {
        // Spawn a temporary killed effect
        Animate *temp = new Animate();
        temp->PostEvent(EV_Remove, 1.f);
        temp->setModel(m_sKilledEffect);
        temp->NewAnim("idle");
    }

    attacker = ev->GetEntity(1);
    if (killtarget.c_str() && killtarget[0]) {
        // Kill all targets
        for (ent = G_FindTarget(NULL, killtarget.c_str()); ent; ent = G_FindTarget(ent, killtarget.c_str())) {
            ent->PostEvent(EV_Remove, 0);
        }
    }

    if (target.c_str() && target[0]) {
        // Activate all targets
        for (ent = G_FindTarget(NULL, target.c_str()); ent; ent = G_FindTarget(ent, target.c_str())) {
            Event *event = new Event(EV_Activate);
            event->AddEntity(attacker);

            ent->ProcessEvent(event);
        }
    }

    // Remove ourself
    PostEvent(EV_Remove, 0);
}

void InteractObject::Archive(Archiver& arc)
{
    Animate::Archive(arc);

    arc.ArchiveString(&m_sHitEffect);
    arc.ArchiveString(&m_sKilledEffect);
}

/*****************************************************************************/
/*QUAKED func_throwobject (0 0.25 0.5) (-16 -16 0) (16 16 32)

This is an object you can pickup and throw at people
******************************************************************************/

Event EV_ThrowObject_Pickup
(
    "pickup",
    EV_DEFAULT,
    "es",
    "entity tag_name",
    "Picks up this throw object and attaches it to the entity.",
    EV_NORMAL
);
Event EV_ThrowObject_Throw
(
    "throw",
    EV_DEFAULT,
    "efeF",
    "owner speed targetent grav",
    "Throw this throw object.",
    EV_NORMAL
);
Event EV_ThrowObject_PickupOffset
(
    "pickupoffset",
    EV_DEFAULT,
    "v",
    "pickup_offset",
    "Sets the pickup_offset.",
    EV_NORMAL
);
Event EV_ThrowObject_ThrowSound
(
    "throwsound",
    EV_DEFAULT,
    "s",
    "throw_sound",
    "Sets the sound to play when object is thrown.",
    EV_NORMAL
);

CLASS_DECLARATION(Object, ThrowObject, "func_throwobject") {
    {&EV_Touch,                    &ThrowObject::Touch       },
    {&EV_ThrowObject_Pickup,       &ThrowObject::Pickup      },
    {&EV_ThrowObject_Throw,        &ThrowObject::Throw       },
    {&EV_ThrowObject_PickupOffset, &ThrowObject::PickupOffset},
    {&EV_ThrowObject_ThrowSound,   &ThrowObject::ThrowSound  },
    {NULL,                         NULL                      }
};

ThrowObject::ThrowObject()
{
    if (LoadingSavegame) {
        // Archive function will setup all necessary data
        return;
    }
    pickup_offset = vec_zero;
}

void ThrowObject::PickupOffset(Event *ev)
{
    pickup_offset = edict->s.scale * ev->GetVector(1);
}

void ThrowObject::ThrowSound(Event *ev)
{
    throw_sound = ev->GetString(1);
}

void ThrowObject::Touch(Event *ev)

{
    Entity *other;

    if (movetype != MOVETYPE_BOUNCE) {
        return;
    }

    other = ev->GetEntity(1);
    assert(other);

    if (other->isSubclassOf(Teleporter)) {
        return;
    }

    if (other->entnum == owner) {
        return;
    }

    if (throw_sound.length()) {
        StopLoopSound();
    }

    if (other->takedamage) {
        other->Damage(
            this,
            G_GetEntity(owner),
            size.length() * velocity.length() / 400,
            origin,
            velocity,
            level.impact_trace.plane.normal,
            32,
            0,
            MOD_THROWNOBJECT
        );
    }

    Damage(this, this, max_health, origin, velocity, level.impact_trace.plane.normal, 32, 0, MOD_THROWNOBJECT);
}

void ThrowObject::Throw(Event *ev)

{
    Entity   *owner;
    Sentient *targetent;
    float     speed;
    float     traveltime;
    float     vertical_speed;
    Vector    target;
    Vector    dir;
    float     grav;
    Vector    xydir;
    Event    *e;

    owner = ev->GetEntity(1);
    assert(owner);

    if (!owner) {
        return;
    }

    speed = ev->GetFloat(2);
    if (!speed) {
        speed = 1;
    }

    targetent = (Sentient *)ev->GetEntity(3);
    assert(targetent);
    if (!targetent) {
        return;
    }

    if (ev->NumArgs() == 4) {
        grav = ev->GetFloat(4);
    } else {
        grav = 1;
    }

    e = new Event(EV_Detach);
    ProcessEvent(e);

    this->owner       = owner->entnum;
    edict->r.ownerNum = owner->entnum;

    gravity = grav;

    target = targetent->origin;
    target.z += targetent->viewheight;

    setMoveType(MOVETYPE_BOUNCE);
    setSolidType(SOLID_BBOX);
    edict->clipmask = MASK_PROJECTILE;

    dir            = target - origin;
    xydir          = dir;
    xydir.z        = 0;
    traveltime     = xydir.length() / speed;
    vertical_speed = (dir.z / traveltime) + (0.5f * gravity * sv_gravity->value * traveltime);
    xydir.normalize();

    // setup ambient flying sound
    if (throw_sound.length()) {
        LoopSound(throw_sound.c_str());
    }

    velocity   = speed * xydir;
    velocity.z = vertical_speed;

    angles = velocity.toAngles();
    setAngles(angles);

    avelocity.x = crandom() * 200;
    avelocity.y = crandom() * 200;
    takedamage  = DAMAGE_YES;
}

void ThrowObject::Pickup(Event *ev)

{
    Entity *ent;
    Event  *e;
    str     bone;

    ent = ev->GetEntity(1);

    assert(ent);
    if (!ent) {
        return;
    }
    bone = ev->GetString(2);

    setOrigin(pickup_offset);

    e = new Event(EV_Attach);
    e->AddEntity(ent);
    e->AddString(bone);
    ProcessEvent(e);

    edict->s.renderfx &= ~RF_FRAMELERP;
}

void ThrowObject::Archive(Archiver& arc)
{
    Object::Archive(arc);

    arc.ArchiveInteger(&owner);
    arc.ArchiveVector(&pickup_offset);
    arc.ArchiveString(&throw_sound);
}

CLASS_DECLARATION(Entity, HelmetObject, "helmetobject") {
    {NULL, NULL}
};

HelmetObject::HelmetObject()
{
    if (LoadingSavegame) {
        return;
    }

    setSolidType(SOLID_NOT);
    setMoveType(MOVETYPE_TOSS);
    setSize(Vector(-2, -2, -2), Vector(2, 2, 2));
    edict->clipmask = MASK_VIEWSOLID;

    // Remove the object automatically after 5 seconds
    PostEvent(EV_Remove, 5);
}

void HelmetObject::HelmetTouch(Event *ev)
{
    avelocity = vec_zero;

    angles.x = 0;
    angles.z = 0;
    setAngles(angles);
    // Stop moving
    setMoveType(MOVETYPE_NONE);
}
