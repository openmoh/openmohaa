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

// trigger.cpp: Environment based triggers.
//

#include "g_local.h"
#include "g_phys.h"
#include "g_spawn.h"
#include "entity.h"
#include "misc.h"
#include "specialfx.h"
#include "sentient.h"
#include "item.h"
#include "player.h"
#include "camera.h"
#include "actor.h"
#include "weaputils.h"
#include "VehicleCollisionEntity.h"
#include "scriptexception.h"

#include "trigger.h"
#include "scriptmaster.h"
#include "parm.h"
#include "worldspawn.h"
#include "barrels.h"

Event EV_Trigger_ActivateTargets
(
    "activatetrigger",
    EV_DEFAULT,
    "e",
    "triggering_entity",
    "Activates all of the targets for this trigger.",
    EV_NORMAL
);
Event EV_Trigger_SetWait
(
    "wait",
    EV_DEFAULT,
    "f",
    "wait_time",
    "Set the wait time (time bewteen triggerings) for this trigger",
    EV_NORMAL
);
Event EV_Trigger_SetDelay
(
    "delay",
    EV_DEFAULT,
    "f",
    "delay_time",
    "Set the delay time (time between triggering and firing) for this trigger",
    EV_NORMAL
);
Event EV_Trigger_SetCount
(
    "cnt",
    EV_DEFAULT,
    "i",
    "count",
    "Set the amount of times this trigger can be triggered",
    EV_NORMAL
);
Event EV_Trigger_SetMessage
(
    "message",
    EV_DEFAULT,
    "s",
    "message",
    "Set a message to be displayed when this trigger is activated",
    EV_NORMAL
);
Event EV_Trigger_SetNoise
(
    "noise",
    EV_DEFAULT,
    "s",
    "sound",
    "Set the sound to play when this trigger is activated",
    EV_NORMAL
);
Event EV_Trigger_SetSound
(
    "sound",
    EV_DEFAULT,
    "s",
    "sound",
    "Set the sound to play when this trigger is activated",
    EV_NORMAL
);
Event EV_Trigger_SetThread
(
    "setthread",
    EV_DEFAULT,
    "s",
    "thread",
    "Set the thread to execute when this trigger is activated",
    EV_NORMAL
);
Event EV_Trigger_Effect
(
    "triggereffect",
    EV_DEFAULT,
    "e",
    "triggering_entity",
    "Send event to owner of trigger.",
    EV_NORMAL
);
Event EV_Trigger_Effect_Alt
(
    "triggereffectalt",
    EV_DEFAULT,
    "e",
    "triggering_entity",
    "Send event to owner of trigger.  This event is only triggered when using a trigger\n"
    "as a multi-faceted edge trigger.",
    EV_NORMAL
);
Event EV_Trigger_StartThread
(
    "triggerthread",
    EV_DEFAULT,
    NULL,
    NULL,
    "Start the trigger thread.",
    EV_NORMAL
);
Event EV_Trigger_SetTriggerable
(
    "triggerable",
    EV_DEFAULT,
    NULL,
    NULL,
    "Turn this trigger back on",
    EV_NORMAL
);
Event EV_Trigger_SetNotTriggerable
(
    "nottriggerable",
    EV_DEFAULT,
    NULL,
    NULL,
    "Turn this trigger off",
    EV_NORMAL
);
Event EV_Trigger_SetMultiFaceted
(
    "multifaceted",
    EV_DEFAULT,
    "i",
    "facetDirection",
    "Make this trigger multifaceted.  If facet is 1, than trigger is North/South oriented.\n"
    "If facet is 2 than trigger is East/West oriented. If facet is 3 than trigger is Up/Down oriented.",
    EV_NORMAL
);
Event EV_Trigger_SetEdgeTriggered
(
    "edgetriggered",
    EV_DEFAULT,
    "b",
    "newEdgeTriggered",
    "If true, trigger will only trigger when object enters trigger, not when it is inside it.",
    EV_NORMAL
);
Event EV_Trigger_SetTriggerCone
(
    "cone",
    EV_DEFAULT,
    "f",
    "newTriggerCone",
    "Sets the cone in which directed triggers will trigger.",
    EV_NORMAL
);
Event EV_Trigger_GetActivator
(
    "activator",
    EV_DEFAULT,
    NULL,
    NULL,
    "Get's entity who last activated this trigger.",
    EV_GETTER
);

#define MULTI_ACTIVATE      1
#define INVISIBLE           2

#define VISIBLE             1

#define TRIGGER_PLAYERS     4
#define TRIGGER_MONSTERS    8
#define TRIGGER_PROJECTILES 16
#define TRIGGER_DAMAGE      128

/*****************************************************************************/
/*QUAKED trigger_multiple (1 0 0) ? x x NOT_PLAYERS MONSTERS PROJECTILES x x DAMAGE

Variable sized repeatable trigger.  Must be targeted at one or more entities.

If "health" is set, the trigger must be killed to activate each time.
If "delay" is set, the trigger waits some time after activating before firing.

"setthread" name of thread to trigger.  This can be in a different script file as well\
by using the '::' notation.

if "angle" is set, the trigger will only fire when someone is facing the
direction of the angle.
"cone" the cone in which a directed trigger can be triggered (default 60 degrees)

"wait" : Seconds between triggerings. (.2 default)
"cnt" how many times it can be triggered (infinite default)

"triggerable" turn trigger on
"nottriggerable" turn trigger off

If NOT_PLAYERS is set, the trigger does not respond to players
If MONSTERS is set, the trigger will respond to monsters
If PROJECTILES is set, the trigger will respond to projectiles (rockets, grenades, etc.)
If DAMAGE is set, the trigger will only respond to bullets

set "message" to text string

******************************************************************************/

CLASS_DECLARATION(Animate, Trigger, "trigger_multiple") {
    {&EV_Trigger_SetWait,           &Trigger::EventSetWait     },
    {&EV_Trigger_SetDelay,          &Trigger::EventSetDelay    },
    {&EV_Trigger_SetCount,          &Trigger::EventSetCount    },
    {&EV_Trigger_SetMessage,        &Trigger::EventSetMessage  },
    {&EV_Trigger_SetNoise,          &Trigger::EventSetNoise    },
    {&EV_Trigger_SetSound,          &Trigger::EventSetNoise    },
    {&EV_Trigger_SetThread,         &Trigger::EventSetThread   },
    {&EV_Touch,                     &Trigger::TriggerStuff     },
    {&EV_Killed,                    &Trigger::TriggerStuff     },
    {&EV_Activate,                  &Trigger::TriggerStuff     },
    {&EV_Trigger_ActivateTargets,   &Trigger::ActivateTargets  },
    {&EV_Trigger_StartThread,       &Trigger::StartThread      },
    {&EV_Model,                     &Trigger::SetModelEvent    },
    {&EV_SetAngle,                  &Trigger::SetTriggerDir    },
    {&EV_Trigger_SetTriggerable,    &Trigger::SetTriggerable   },
    {&EV_Trigger_SetNotTriggerable, &Trigger::SetNotTriggerable},
    {&EV_Trigger_SetMultiFaceted,   &Trigger::SetMultiFaceted  },
    {&EV_Trigger_SetEdgeTriggered,  &Trigger::SetEdgeTriggered },
    {&EV_Trigger_SetTriggerCone,    &Trigger::SetTriggerCone   },
    {&EV_Trigger_GetActivator,      &Trigger::EventGetActivator},
    {NULL,                          NULL                       }
};

Trigger::Trigger()
{
    AddWaitTill(STRING_TRIGGER);

    if (LoadingSavegame) {
        // Archive function will setup all necessary data
        return;
    }

    triggerActivated = false;
    activator        = NULL;
    trigger_time     = 0;

    setMoveType(MOVETYPE_NONE);

    health     = 0;
    max_health = 0;

    if (spawnflags & TRIGGER_DAMAGE) {
        takedamage = DAMAGE_YES;
        setContents(CONTENTS_CLAYPIDGEON);
        setSolidType(SOLID_BBOX);
    } else {
        setContents(0);
        setSolidType(SOLID_TRIGGER);
    }

    edict->r.svFlags |= SVF_NOCLIENT;

    delay       = 0;
    wait        = 0.2f;
    triggerCone = cos(DEG2RAD(60));

    useTriggerDir = qfalse;
    triggerDir    = G_GetMovedir(0);

    triggerable = qtrue;
    removable   = qtrue;

    trigger_time = 0;
    // normal trigger to begin with
    multiFaceted = 0;
    // normally, not edge triggered
    edgeTriggered = qfalse;

    count = -1;

    //noise = "environment/switch/switch2.wav";
    noise = STRING_EMPTY;

    respondto = spawnflags ^ TRIGGER_PLAYERS;

    message = STRING_EMPTY;
}

Trigger::~Trigger() {}

void Trigger::SetTriggerDir(float angle)

{
    triggerDirYaw = angle;
    triggerDir    = G_GetMovedir(angle);
    useTriggerDir = qtrue;
}

Vector Trigger::GetTriggerDir(void)

{
    return triggerDir;
}

void Trigger::SetTriggerCone(Event *ev)

{
    triggerCone = cos(DEG2RAD(ev->GetFloat(1)));
}

qboolean Trigger::UsingTriggerDir(void)
{
    return useTriggerDir;
}

void Trigger::SetTriggerDir(Event *ev)

{
    SetTriggerDir(ev->GetFloat(1));
}

void Trigger::SetModelEvent(Event *ev)
{
    Animate::SetModelEvent(ev);
    if (spawnflags & TRIGGER_DAMAGE) {
        setContents(CONTENTS_CLAYPIDGEON);
    } else {
        setContents(0);
    }
    edict->r.svFlags |= SVF_NOCLIENT;
    link();
}

void Trigger::StartThread(Event *ev)
{
    SafePtr<Trigger> This = this;

    parm.other = NULL;

    if (ev->NumArgs() > 0) {
        parm.other = ev->GetEntity(1);
    }

    parm.other = ev->NumArgs() > 0 ? ev->GetEntity(1) : NULL;
    parm.owner = ev->NumArgs() > 1 ? ev->GetEntity(2) : NULL;

    Unregister(STRING_TRIGGER);

    if (This) {
        m_Thread.Execute(this);
    }
}

qboolean Trigger::respondTo(Entity *other)

{
    return (
        ((respondto & TRIGGER_PLAYERS) && other->isClient())
        || ((respondto & TRIGGER_MONSTERS) && other->IsSubclassOfActor())
        || ((respondto & TRIGGER_PROJECTILES) && other->IsSubclassOfProjectile())
    );
}

Entity *Trigger::getActivator(Entity *other)

{
    return other;
}

void Trigger::TriggerStuff(Event *ev)
{
    Entity *other;
    Entity *activator;
    Event  *event;
    int     whatToTrigger;

    // if trigger is shut off return immediately
    if (!triggerable) {
        return;
    }

    if (isSubclassOf(TriggerUse) && *ev != EV_Use) {
        Com_Printf("^~^~^  Attempting to trigger TriggerUse with non-use trigger\n");
        return;
    }

    // Don't bother with testing anything if we can't trigger yet
    if ((level.time < trigger_time) || (trigger_time == -1)) {
        // if we are edgeTriggered, we reset our time until we leave the trigger
        if (edgeTriggered && trigger_time != -1) {
            trigger_time = level.time + wait;
        }
        return;
    }

    health = max_health;
    if (health && (*ev != EV_Killed) && (*ev != EV_Activate)) {
        // if health is set, we only respond to killed and activate messages
        return;
    }

    other = ev->GetEntity(1);

    assert(other != this);

    if (other == this) {
        ScriptError("trigger '%s' triggered by self", TargetName().c_str());
    }

    // Always respond to activate messages from the world since they're probably from
    // the "trigger" command
    if (!respondTo(other) && (other != world || *ev != EV_Activate)
        && (!other || (!other->isSubclassOf(Camera) && !other->isSubclassOf(BarrelObject)))) {
        return;
    }

    //
    // if we setup an angle for this trigger, only trigger if other is within ~60 degrees of the triggers origin
    // only test for this case if we were touched, activating or killed should never go through this code
    //
    if (useTriggerDir && (*ev == EV_Touch)) {
        Vector norm;
        float  dot;

        norm = origin - other->origin;
        norm.normalize();
        dot = norm * triggerDir;
        if (dot < triggerCone) {
            // don't retrigger for at least a second
            trigger_time = level.time + 1;
            return;
        }
    }

    activator = getActivator(other);

    if (multiFaceted) {
        Vector delta;

        delta = other->origin - origin;
        switch (multiFaceted) {
        case 1:
            if (delta[1] > 0) {
                whatToTrigger = 0;
            } else {
                whatToTrigger = 1;
            }
            break;
        case 2:
            if (delta[0] > 0) {
                whatToTrigger = 0;
            } else {
                whatToTrigger = 1;
            }
            break;
        case 3:
        default:
            if (delta[2] > 0) {
                whatToTrigger = 0;
            } else {
                whatToTrigger = 1;
            }
            break;
        }
    } else {
        whatToTrigger = 0;
    }

    if (takedamage == DAMAGE_NO) {
        trigger_time = level.time + wait;
    }

    if (!whatToTrigger) {
        event = new Event(EV_Trigger_Effect);
        event->AddEntity(activator);
        PostEvent(event, delay);
    } else {
        event = new Event(EV_Trigger_Effect_Alt);
        event->AddEntity(activator);
        PostEvent(event, delay);
    }

    event = new Event(EV_Trigger_ActivateTargets);
    event->AddEntity(activator);
    PostEvent(event, delay);

    // don't trigger the thread if we were triggered by the world touching us
    if ((activator != world) || (*ev != EV_Touch)) {
        event = new Event(EV_Trigger_StartThread);
        if (activator) {
            event->AddEntity(activator);
        }
        if (activator->IsSubclassOfProjectile()) {
            Projectile *proj = static_cast<Projectile *>(activator);
            event->AddEntity(G_GetEntity(proj->owner));
        }
        PostEvent(event, delay);
    }

    if (count > -1) {
        count--;
        if (count < 1) {
            //
            // Don't allow it to trigger anymore
            //
            trigger_time = -1;

            //
            // Make sure we wait until we're done triggering things before removing
            //
            if (removable) {
                PostEvent(EV_Remove, delay + FRAMETIME);
            }
        }
    }
}

//
//==============================
// ActivateTargets
//
// "other" should be set to the entity that initiated the firing.
//
// Centerprints any message to the activator.
//
// Removes all entities with a targetname that match killtarget,
// so some events can remove other triggers.
//
// Search in targetname of all entities that match target
// and send EVENT_ACTIVATE to there event handler
//==============================
//
void Trigger::ActivateTargets(Event *ev)

{
    Entity     *other;
    Entity     *ent;
    Event      *event;
    const char *name;

    other = ev->GetEntity(1);

    if (!other) {
        other = world;
    }

    if (triggerActivated) {
        //
        // Entity triggered itself.  Prevent an infinite loop
        //
        ScriptError("Entity targeting itself--Targetname '%s'", TargetName().c_str());
        return;
    }

    triggerActivated = true;
    activator        = other;

    //
    // print the message
    //
    if (message != STRING_EMPTY && other && (other->isClient() || other->isSubclassOf(Camera))) {
        // HACK HACK HACK
        // if it is a camera, pass in default player
        if (!other->isClient()) {
            gi.centerprintf(&g_entities[0], Director.GetString(message).c_str());
        } else {
            gi.centerprintf(other->edict, Director.GetString(message).c_str());
        }
        if (Noise().length()) {
            other->Sound(Director.GetString(noise).c_str(), CHAN_VOICE);
        }
    }

    //
    // kill the killtargets
    //
    name = KillTarget();
    if (name && strcmp(name, "")) {
        ent = NULL;
        do {
            ent = G_FindTarget(ent, name);
            if (!ent) {
                break;
            }
            ent->PostEvent(EV_Remove, 0);
        } while (1);
    }

    //
    // fire targets
    //
    name = Target();
    if (name && strcmp(name, "")) {
        ent = NULL;
        do {
            ent = G_FindTarget(ent, name);
            if (!ent) {
                break;
            }

            event = new Event(EV_Activate);
            event->AddEntity(other);
            ent->ProcessEvent(event);
        } while (1);
    }

    triggerActivated = false;
}

void Trigger::EventSetWait(Event *ev)

{
    wait = ev->GetFloat(1);
}

void Trigger::EventSetDelay(Event *ev)

{
    delay = ev->GetFloat(1);
}

void Trigger::EventSetThread(Event *ev)
{
    if (ev->IsFromScript()) {
        m_Thread.SetThread(ev->GetValue(1));
    } else {
        m_Thread.Set(ev->GetString(1));
    }
}

void Trigger::EventSetCount(Event *ev)

{
    count = ev->GetInteger(1);
}

void Trigger::EventSetMessage(Event *ev)

{
    SetMessage(ev->GetString(1));
}

void Trigger::SetMessage(const char *text)

{
    if (text) {
        message = Director.AddString(text);
    } else {
        message = STRING_EMPTY;
    }
}

str& Trigger::Message(void)

{
    return Director.GetString(message);
}

void Trigger::EventSetNoise(Event *ev)

{
    SetNoise(ev->GetString(1));
}

void Trigger::SetNoise(const char *text)

{
    if (text) {
        noise = Director.AddString(text);
        //
        // cache in the sound
        //
        CacheResource(Director.GetString(noise).c_str());
    }
}

str& Trigger::Noise(void)

{
    return Director.GetString(noise);
}

void Trigger::SetMultiFaceted(int newFacet)
{
    multiFaceted = newFacet;
}

void Trigger::SetEdgeTriggered(qboolean newEdge)
{
    edgeTriggered = newEdge;
}

int Trigger::GetMultiFaceted(void)
{
    return multiFaceted;
}

qboolean Trigger::GetEdgeTriggered(void)
{
    return edgeTriggered;
}

void Trigger::SetMultiFaceted(Event *ev)
{
    SetMultiFaceted(ev->GetInteger(1));
}

void Trigger::SetEdgeTriggered(Event *ev)
{
    SetEdgeTriggered(ev->GetBoolean(1));
}

void Trigger::SetTriggerable(Event *ev)
{
    triggerable = qtrue;
}

void Trigger::SetNotTriggerable(Event *ev)
{
    triggerable = qfalse;
}

void Trigger::DamageEvent(Event *ev)
{
    Entity *attacker;
    int     damage;
    Vector  dir;
    Vector  momentum;

    if (Immune(ev->GetInteger(9))) {
        // trigger is immune
        return;
    }

    attacker = ev->GetEntity(1);
    if (!attacker) {
        ScriptError("attacker is NULL");
    }

    if (max_health) {
        if (health <= 0) {
            return;
        }

        damage = ev->GetInteger(2);
        health -= damage;
    }

    if (health <= 0) {
        Event *event;

        event = new Event(EV_Activate, 1);
        event->AddEntity(attacker);
        ProcessEvent(event);
    }

    // notify scripts about the damage
    Unregister(STRING_DAMAGE);
}

void Trigger::EventGetActivator(Event *ev)
{
    ev->AddEntity(activator);
}

void Trigger::Archive(Archiver& arc)
{
    Animate::Archive(arc);

    arc.ArchiveFloat(&wait);
    arc.ArchiveFloat(&delay);
    arc.ArchiveFloat(&trigger_time);
    arc.ArchiveBoolean(&triggerActivated);
    arc.ArchiveInteger(&count);
    Director.ArchiveString(arc, noise);
    if (arc.Loading()) {
        SetNoise(Director.GetString(noise).c_str());
    }
    Director.ArchiveString(arc, message);
    m_Thread.Archive(arc);
    arc.ArchiveSafePointer(&activator);
    arc.ArchiveInteger(&respondto);
    arc.ArchiveBoolean(&useTriggerDir);
    arc.ArchiveFloat(&triggerCone);
    arc.ArchiveVector(&triggerDir);
    arc.ArchiveFloat(&triggerDirYaw);
    arc.ArchiveBoolean(&triggerable);
    arc.ArchiveBoolean(&removable);
    arc.ArchiveBoolean(&edgeTriggered);
    arc.ArchiveInteger(&multiFaceted);
}

/*QUAKED trigger_vehicle (0.25 0.5 0.35) ? x x x x x x x x
works exactly like a trigger_multiple but only responds to vehicles.
******************************************************************************/

CLASS_DECLARATION(Trigger, TriggerVehicle, "trigger_vehicle") {
    {NULL, NULL}
};

qboolean TriggerVehicle::respondTo(Entity *other)
{
    if ((other->IsSubclassOfVehicle()) || isSubclassOf(VehicleCollisionEntity)) {
        return qtrue;
    } else {
        return qfalse;
    }
}

/*QUAKED trigger_multipleall (1 0 0) ? x x NOT_PLAYERS MONSTERS PROJECTILES x x DAMAGE

Variable sized repeatable trigger.  Must be targeted at one or more entities.

If "health" is set, the trigger must be killed to activate each time.
If "delay" is set, the trigger waits some time after activating before firing.

"setthread" name of thread to trigger.  This can be in a different script file as well\
by using the '::' notation.

if "angle" is set, the trigger will only fire when someone is facing the
direction of the angle.
"cone" the cone in which a directed trigger can be triggered (default 60 degrees)

"wait" : Seconds between triggerings. (.2 default)

"triggerable" turn trigger on
"nottriggerable" turn trigger off

If NOT_PLAYERS is set, the trigger does not respond to players
If MONSTERS is set, the trigger will respond to monsters
If PROJECTILES is set, the trigger will respond to projectiles (rockets, grenades, etc.)
If DAMAGE is set, the trigger will only respond to bullets

set "message" to text string

******************************************************************************/
CLASS_DECLARATION(Trigger, TriggerAll, "trigger_multipleall") {
    {&EV_Touch,    &TriggerAll::TriggerStuff},
    {&EV_Activate, &TriggerAll::TriggerStuff},
    {NULL,         NULL                     }
};

void TriggerAll::TriggerStuff(Event *ev)
{
    Entity *other;
    Entity *activator;
    int     whatToTrigger;
    Event  *event;

    if (!triggerable) {
        return;
    }

    if (isSubclassOf(TriggerUse) && ev->eventnum != EV_Use.eventnum) {
        Com_Printf("^~^~^  Attempting to trigger TriggerUse with non-use trigger\n");
        return;
    }

    other = ev->GetEntity(1);

    if (other == this) {
        ScriptError("trigger '%s' triggered by self", TargetName().c_str());
    }

    // Always respond to activate messages from the world since they're probably from
    // the "trigger" command
    if (!respondTo(other) && !((other == world) && (*ev == EV_Activate))
        && (!other || !other->IsSubclassOfCamera() || !other->isSubclassOf(BarrelObject))) {
        return;
    }

    //
    // check if the other entity can trigger ourself
    //
    if (!IsTriggerable(other)) {
        if (edgeTriggered) {
            // don't retrigger for the entity for at least the specified wait time
            SetTriggerTime(other, level.time + wait);
        }

        return;
    }

    //
    // if we setup an angle for this trigger, only trigger if other is within ~60 degrees of the triggers origin
    // only test for this case if we were touched, activating or killed should never go through this code
    //
    if (useTriggerDir && (*ev == EV_Touch)) {
        Vector norm;
        float  dot;

        norm = origin - other->origin;
        norm.normalize();
        dot = norm * triggerDir;
        if (dot < triggerCone) {
            // don't retrigger for at least a second
            SetTriggerTime(other, level.time + 1);
            return;
        }
    }

    activator = getActivator(other);

    if (multiFaceted) {
        Vector delta;

        delta = other->origin - origin;
        switch (multiFaceted) {
        case 1:
            if (delta[1] > 0) {
                whatToTrigger = 0;
            } else {
                whatToTrigger = 1;
            }
            break;
        case 2:
            if (delta[0] > 0) {
                whatToTrigger = 0;
            } else {
                whatToTrigger = 1;
            }
            break;
        case 3:
        default:
            if (delta[2] > 0) {
                whatToTrigger = 0;
            } else {
                whatToTrigger = 1;
            }
            break;
        }
    } else {
        whatToTrigger = 0;
    }

    if (!whatToTrigger) {
        event = new Event(EV_Trigger_Effect);
        event->AddEntity(activator);
        PostEvent(event, delay);
    } else {
        event = new Event(EV_Trigger_Effect_Alt);
        event->AddEntity(activator);
        PostEvent(event, delay);
    }

    event = new Event(EV_Trigger_ActivateTargets);
    event->AddEntity(activator);
    PostEvent(event, delay);

    // don't trigger the thread if we were triggered by the world touching us
    if ((activator != world) || (ev->eventnum != EV_Touch.eventnum)) {
        event = new Event(EV_Trigger_StartThread);
        if (activator) {
            event->AddEntity(activator);
        }
        PostEvent(event, delay);
    }
}

bool TriggerAll::IsTriggerable(Entity *other)
{
    int              i;
    TriggerAllEntry *entry;

    for (i = entries.NumObjects(); i > entries.NumObjects(); i--) {
        entry = &entries.ObjectAt(i);

        if (!entry->ent || level.time >= entry->time) {
            // remove empty entities
            entries.RemoveObjectAt(i);
            continue;
        }

        if (entry->ent == other) {
            // not yet triggerable
            return false;
        }
    }

    return true;
}

void TriggerAll::SetTriggerTime(Entity *other, float time)
{
    int              i;
    TriggerAllEntry *entry;
    TriggerAllEntry  newEntry;

    for (i = entries.NumObjects(); i > entries.NumObjects(); i--) {
        entry = &entries.ObjectAt(i);

        if (!entry->ent || level.time >= entry->time) {
            // remove empty entities
            entries.RemoveObjectAt(i);
            continue;
        }

        if (entry->ent == other) {
            if (level.time >= time) {
                entries.RemoveObjectAt(i);
            } else {
                entry->time = time;
            }

            return;
        }
    }

    newEntry.ent  = other;
    newEntry.time = time;
    entries.AddObject(newEntry);
}

void TriggerAllEntry::Archive(Archiver& arc, TriggerAllEntry *obj)
{
    arc.ArchiveSafePointer(&obj->ent);
    arc.ArchiveFloat(&obj->time);
}

CLASS_DECLARATION(Trigger, TouchField, NULL) {
    {&EV_Trigger_Effect, &TouchField::SendEvent},
    {NULL,               NULL                  }
};

TouchField::TouchField()
{
    ontouch = NULL;
}

void TouchField::Setup(Entity *ownerentity, Event& touchevent, Vector min, Vector max, int respondto)

{
    assert(ownerentity);
    if (!ownerentity) {
        ScriptError("Null owner");
    }

    owner = ownerentity;
    if (ontouch) {
        delete ontouch;
    }

    ontouch = new Event(touchevent);
    setSize(min, max);

    setContents(0);
    setSolidType(SOLID_TRIGGER);
    link();

    this->respondto = respondto;
}

void TouchField::SendEvent(Event *ev)

{
    Event *event;

    // Check if our owner is still around
    if (owner) {
        event = new Event(*ontouch);
        event->AddEntity(ev->GetEntity(1));
        owner->PostEvent(event, delay);
    } else {
        // Our owner is gone!  The bastard didn't delete us!
        // Guess we're no longer needed, so remove ourself.
        PostEvent(EV_Remove, 0);
    }
}

/*****************************************************************************/
/*QUAKED trigger_once (1 0 0) ? NOTOUCH x NOT_PLAYERS MONSTERS PROJECTILES

Variable sized trigger. Triggers once, then removes itself.
You must set the key "target" to the name of another object in the
level that has a matching

If "health" is set, the trigger must be killed to activate it.
If "delay" is set, the trigger waits some time after activating before firing.

"targetname".  If "health" is set, the trigger must be killed to activate.

"setthread" name of thread to trigger.  This can be in a different script file as well\
by using the '::' notation.

if "killtarget" is set, any objects that have a matching "target" will be
removed when the trigger is fired.

if "angle" is set, the trigger will only fire when someone is facing the
direction of the angle.
"cone" the cone in which a directed trigger can be triggered (default 60 degrees)

"triggerable" turn trigger on
"nottriggerable" turn trigger off

If NOTOUCH is set, trigger will not respond to touch
If NOT_PLAYERS is set, the trigger does not respond to players
If MONSTERS is set, the trigger will respond to monsters
If PROJECTILES is set, the trigger will respond to projectiles (rockets, grenades, etc.)

set "message" to text string

******************************************************************************/

CLASS_DECLARATION(Trigger, TriggerOnce, "trigger_once") {
    {NULL, NULL}
};

TriggerOnce::TriggerOnce()
{
    if (LoadingSavegame) {
        // Archive function will setup all necessary data
        return;
    }
    //
    // no matter what, we only trigger once
    //
    count     = 1;
    respondto = spawnflags ^ TRIGGER_PLAYERS;

    //
    // if it's not supposed to be touchable, clear the trigger
    //
    if (spawnflags & 1) {
        setSolidType(SOLID_NOT);
    }
}

/*QUAKED trigger_save (1 0 0) ? NOTOUCH x NOT_PLAYERS MONSTERS PROJECTILES

Variable sized trigger. Triggers once, then removes itself.
You must set the key "target" to the name of another object in the
level that has a matching

"savename". The Save Name that is appended to the world.message.

If "health" is set, the trigger must be killed to activate it.
If "delay" is set, the trigger waits some time after activating before firing.

"targetname".  If "health" is set, the trigger must be killed to activate.

"setthread" name of thread to trigger.  This can be in a different script file as well\
by using the '::' notation.

if "killtarget" is set, any objects that have a matching "target" will be
removed when the trigger is fired.

if "angle" is set, the trigger will only fire when someone is facing the
direction of the angle.
"cone" the cone in which a directed trigger can be triggered (default 60 degrees)

"triggerable" turn trigger on
"nottriggerable" turn trigger off

If NOTOUCH is set, trigger will not respond to touch
If NOT_PLAYERS is set, the trigger does not respond to players
If MONSTERS is set, the trigger will respond to monsters
If PROJECTILES is set, the trigger will respond to projectiles (rockets, grenades, etc.)

set "message" to text string

******************************************************************************/

Event EV_TriggerSave_SaveName(
    "savename",
    EV_DEFAULT,
    "s",
    "name",
    "Sets the name which is appended to the world.message for this specific autosave"
);

TriggerSave::TriggerSave()
{
    if (LoadingSavegame) {
        return;
    }

    m_sSaveName = "";
    SetTargetName("autosave");
}

void TriggerSave::SaveGame(Event *ev)
{
    gi.setConfigstring(CS_SAVENAME, m_sSaveName);
    gi.SendConsoleCommand("savegame");
}

void TriggerSave::EventSaveName(Event *ev)
{
    m_sSaveName = ev->GetConstString(1);
}

void TriggerSave::Archive(Archiver& arc)
{
    Trigger::Archive(arc);
    arc.ArchiveString(&m_sSaveName);
}

CLASS_DECLARATION(Trigger, TriggerSave, "trigger_save") {
    {&EV_Trigger_Effect,       &TriggerSave::SaveGame     },
    {&EV_TriggerSave_SaveName, &TriggerSave::EventSaveName},
    {NULL,                     NULL                       }
};

/*****************************************************************************/
/*QUAKED trigger_relay (1 0 0) (-8 -8 -8) (8 8 8) x x NOT_PLAYERS MONSTERS PROJECTILES

This fixed size trigger cannot be touched, it can only be fired by other events.
It can contain killtargets, targets, delays, and messages.

If NOT_PLAYERS is set, the trigger does not respond to events triggered by players
If MONSTERS is set, the trigger will respond to events triggered by monsters
If PROJECTILES is set, the trigger will respond to events triggered by projectiles (rockets, grenades, etc.)

******************************************************************************/

CLASS_DECLARATION(Trigger, TriggerRelay, "trigger_relay") {
    {&EV_Touch, NULL},
    {NULL,      NULL}
};

TriggerRelay::TriggerRelay()
{
    setSolidType(SOLID_NOT);
}

/*****************************************************************************/
/*QUAKED trigger_secret (1 0 0) ? NOTOUCH x NOT_PLAYERS MONSTERS PROJECTILES
Secret counter trigger.  Automatically sets and increments script variables \
level.total_secrets and level.found_secrets.

set "message" to text string

if "angle" is set, the trigger will only fire when someone is facing the
direction of the angle.
"cone" the cone in which a directed trigger can be triggered (default 60 degrees)

"setthread" name of thread to trigger.  This can be in a different script file as well \
by using the '::' notation.  (defaults to "global/universal_script.scr::secret")

"triggerable" turn trigger on
"nottriggerable" turn trigger off

If NOTOUCH is set, trigger will not respond to touch
If NOT_PLAYERS is set, the trigger does not respond to players
If MONSTERS is set, the trigger will respond to monsters
If PROJECTILES is set, the trigger will respond to projectiles (rockets, grenades, etc.)

******************************************************************************/

CLASS_DECLARATION(TriggerOnce, TriggerSecret, "trigger_secret") {
    {&EV_Trigger_Effect, &TriggerSecret::FoundSecret},
    {NULL,               NULL                       }
};

TriggerSecret::TriggerSecret()
{
    if (LoadingSavegame) {
        // Archive function will setup all necessary data
        return;
    }
    level.total_secrets++;
}

void TriggerSecret::FoundSecret(Event *ev)

{
    //
    // anything that causes the trigger to fire increments the number
    // of secrets found.  This way, if the level designer triggers the
    // secret from the script, the player still gets credit for finding
    // it.  This is to prevent a secret from becoming undiscoverable.
    //
    level.found_secrets++;
}

/*****************************************************************************/
/*QUAKED trigger_push (1 0 0) ? x x NOT_PLAYERS MONSTERS PROJECTILES

Pushes entities as if they were caught in a heavy wind.

"speed" indicates the rate that entities are pushed (default 1000).

"angle" indicates the direction the wind is blowing (-1 is up, -2 is down)

"target" if target is set, then a velocity will be calculated based on speed

"triggerable" turn trigger on
"nottriggerable" turn trigger off

If NOT_PLAYERS is set, the trigger does not push players
If MONSTERS is set, the trigger will push monsters
If PROJECTILES is set, the trigger will push projectiles (rockets, grenades, etc.)

******************************************************************************/

Event EV_TriggerPush_SetPushSpeed
(
    "speed",
    EV_DEFAULT,
    "f",
    "speed",
    "Set the push speed of the TriggerPush",
    EV_NORMAL
);

CLASS_DECLARATION(Trigger, TriggerPush, "trigger_push") {
    {&EV_Trigger_Effect,           &TriggerPush::Push        },
    {&EV_SetAngle,                 &TriggerPush::SetPushDir  },
    {&EV_TriggerPush_SetPushSpeed, &TriggerPush::SetPushSpeed},
    {NULL,                         NULL                      }
};

void TriggerPush::Push(Event *ev)
{
    Entity *other;

    other = ev->GetEntity(1);
    if (other) {
        const char *targ;
        Entity     *ent;

        targ = Target();
        if (targ[0]) {
            ent = (Entity *)G_FindTarget(NULL, Target());
            if (ent) {
                other->velocity = G_CalculateImpulse(other->origin, ent->origin, speed, other->gravity);
            }
        } else {
            float dot;

            // find out how much velocity we have in this direction
            dot = triggerDir * other->velocity;
            // subtract it out and add in our velocity
            other->velocity += (speed - dot) * triggerDir;
        }

        other->VelocityModified();
    }
}

void TriggerPush::SetPushDir(Event *ev)
{
    float angle;

    angle = ev->GetFloat(1);
    // this is used, since we won't need elsewhere
    triggerDir = G_GetMovedir(angle);
}

void TriggerPush::SetPushSpeed(Event *ev)

{
    speed = ev->GetFloat(1);
}

TriggerPush::TriggerPush()
{
    if (LoadingSavegame) {
        // Archive function will setup all necessary data
        return;
    }
    speed = 1000;
}

/*****************************************************************************/
/*QUAKED trigger_pushany (1 0 0) ? x x NOT_PLAYERS MONSTERS PROJECTILES

Pushes entities as if they were caught in a heavy wind.

"speed" indicates the rate that entities are pushed (default 1000).
"angles" indicates the direction of the push
"target" if target is set, then a velocity will be calculated based on speed

"triggerable" turn trigger on
"nottriggerable" turn trigger off

If NOT_PLAYERS is set, the trigger does not push players
If MONSTERS is set, the trigger will push monsters
If PROJECTILES is set, the trigger will push projectiles (rockets, grenades, etc.)

******************************************************************************/

Event EV_TriggerPushAny_SetSpeed
(
    "speed",
    EV_DEFAULT,
    "f",
    "speed",
    "Set the speed.",
    EV_NORMAL
);

CLASS_DECLARATION(Trigger, TriggerPushAny, "trigger_pushany") {
    {&EV_TriggerPushAny_SetSpeed, &TriggerPushAny::SetSpeed},
    {&EV_Trigger_Effect,          &TriggerPushAny::Push    },
    {NULL,                        NULL                     }
};

TriggerPushAny::TriggerPushAny()
{
    if (LoadingSavegame) {
        // Archive function will setup all necessary data
        return;
    }
    speed = 1000;
}

void TriggerPushAny::Push(Event *ev)
{
    Entity *other;

    other = ev->GetEntity(1);
    if (other) {
        Entity *ent;

        if (target.length()) {
            ent = (Entity *)G_FindTarget(NULL, target.c_str());
            if (ent) {
                other->velocity = G_CalculateImpulse(other->origin, ent->origin, speed, other->gravity);
            }
        } else {
            other->velocity = Vector(orientation[0]) * speed;
        }
        other->VelocityModified();
    }
}

void TriggerPushAny::SetSpeed(Event *ev)
{
    speed = ev->GetFloat(1);
}

//================================================================================================
//
// TriggerSound stuff
//
//================================================================================================

Event EV_TriggerPlaySound_SetPitch
(
    "pitch",
    EV_DEFAULT,
    "f",
    "pitch",
    "Sets the pitch.",
    EV_NORMAL
);
Event EV_TriggerPlaySound_SetVolume
(
    "volume",
    EV_DEFAULT,
    "f",
    "volume",
    "Sets the volume.",
    EV_NORMAL
);
Event EV_TriggerPlaySound_SetMinDist
(
    "min_dist",
    EV_DEFAULT,
    "f",
    "min_dist",
    "Sets the minimum distance.",
    EV_NORMAL
);
Event EV_TriggerPlaySound_SetChannel
(
    "channel",
    EV_DEFAULT,
    "i",
    "channel",
    "Sets the sound channel to play on.",
    EV_NORMAL
);

CLASS_DECLARATION(Trigger, TriggerPlaySound, "play_sound_triggered") {
    {&EV_Trigger_Effect,              &TriggerPlaySound::ToggleSound},
    {&EV_TriggerPlaySound_SetVolume,  &TriggerPlaySound::SetVolume  },
    {&EV_TriggerPlaySound_SetMinDist, &TriggerPlaySound::SetMinDist },
    {&EV_TriggerPlaySound_SetChannel, &TriggerPlaySound::SetChannel },
    {&EV_Touch,                       NULL                          },
    {NULL,                            NULL                          }
};

void TriggerPlaySound::ToggleSound(Event *ev)

{
    if (!state) {
        // noise should already be initialized
        if (Noise().length()) {
            if (ambient || spawnflags & TOGGLESOUND) {
                state = 1;
            }

            if (ambient) {
                LoopSound(Noise().c_str(), volume, min_dist);
            } else {
                Sound(Noise(), channel, volume, min_dist);
            }
        }
    } else {
        state = 0;
        if (ambient) {
            StopLoopSound();
        } else {
            StopSound(channel);
        }
    }
}

void TriggerPlaySound::StartSound(void)

{
    // turn the current one off
    state = 1;
    ToggleSound(NULL);

    // start it up again
    state = 0;
    ToggleSound(NULL);
}

void TriggerPlaySound::SetVolume(float vol)

{
    volume = vol;
}

void TriggerPlaySound::SetVolume(Event *ev)

{
    volume = ev->GetFloat(1);
}

void TriggerPlaySound::SetMinDist(float dist)

{
    min_dist = dist;
}

void TriggerPlaySound::SetMinDist(Event *ev)

{
    min_dist = ev->GetFloat(1);
}

void TriggerPlaySound::SetChannel(Event *ev)

{
    channel = ev->GetInteger(1);
}

TriggerPlaySound::TriggerPlaySound()
{
    if (LoadingSavegame) {
        // Archive function will setup all necessary data
        return;
    }
    //
    // this is here so that it gets sent over at least once
    //
    PostEvent(EV_Show, EV_POSTSPAWN);

    ambient   = false;
    volume    = DEFAULT_VOL;
    channel   = CHAN_VOICE;
    state     = 0;
    respondto = spawnflags ^ TRIGGER_PLAYERS;
    min_dist  = DEFAULT_MIN_DIST;

    if (spawnflags & (AMBIENT_ON | AMBIENT_OFF)) {
        ambient = true;
        if (spawnflags & AMBIENT_ON) {
            PostEvent(EV_Trigger_Effect, EV_POSTSPAWN);
        }
    }
}

/*****************************************************************************/
/*QUAKED sound_speaker (0 0.75 0.75) (-8 -8 -8) (8 8 8) AMBIENT-ON AMBIENT-OFF NOT_PLAYERS MONSTERS PROJECTILES TOGGLE

play a sound when it is used

AMBIENT-ON specifies an ambient sound that starts on
AMBIENT-OFF specifies an ambient sound that starts off
TOGGLE specifies that the speaker toggles on triggering

if (AMBIENT-?) is not set, then the sound is sent over explicitly this creates more net traffic

"volume" how loud 0-4 (1 default full volume, ambients do not respond to volume)
"noise" sound to play
"channel" channel on which to play sound\
(0 auto, 1 weapon, 2 voice, 3 item, 4 body, 8 don't use PHS) (voice is default)
"setthread" name of thread to trigger.  This can be in a different script file as well\
by using the '::' notation.

Normal sounds play each time the target is used.

Ambient Looped sounds have a volume 1, and the use function toggles it on/off.

If NOT_PLAYERS is set, the trigger does not respond to players
If MONSTERS is set, the trigger will respond to monsters
If PROJECTILES is set, the trigger will respond to projectiles (rockets, grenades, etc.)

******************************************************************************/

CLASS_DECLARATION(TriggerPlaySound, TriggerSpeaker, "sound_speaker") {
    {&EV_Touch, NULL},
    {NULL,      NULL}
};

TriggerSpeaker::TriggerSpeaker() {}

/*****************************************************************************/
/*QUAKED sound_randomspeaker (0 0.75 0.75) (-8 -8 -8) (8 8 8) x x NOT_PLAYERS MONSTERS PROJECTILES x x

play a sound at random times

"mindelay" minimum delay between sound triggers (default 3)
"maxdelay" maximum delay between sound triggers (default 10)
"chance" chance that sound will play when fired (default 1)
"volume" how loud 0-4 (1 default full volume)
"noise" sound to play
"channel" channel on which to play sound\
(0 auto, 1 weapon, 2 voice, 3 item, 4 body, 8 don't use PHS) (voice is default)

Normal sounds play each time the target is used.

If NOT_PLAYERS is set, the trigger does not respond to players
If MONSTERS is set, the trigger will respond to monsters
If PROJECTILES is set, the trigger will respond to projectiles (rockets, grenades, etc.)

******************************************************************************/

Event EV_TriggerRandomSpeaker_TriggerSound
(
    "triggersound",
    EV_DEFAULT,
    NULL,
    NULL,
    "Triggers the sound to play and schedules the next time to play.",
    EV_NORMAL
);
Event EV_Trigger_SetMinDelay
(
    "mindelay",
    EV_DEFAULT,
    "f",
    "min_delay",
    "Sets the minimum time between playings.",
    EV_NORMAL
);
Event EV_Trigger_SetMaxDelay
(
    "maxdelay",
    EV_DEFAULT,
    "f",
    "max_delay",
    "Sets the maximum time between playings.",
    EV_NORMAL
);
Event EV_Trigger_SetChance
(
    "chance",
    EV_DEFAULT,
    "f[0,1]",
    "newChance",
    "Sets the chance that the sound will play when triggered.",
    EV_NORMAL
);

CLASS_DECLARATION(TriggerSpeaker, RandomSpeaker, "sound_randomspeaker") {
    {&EV_Trigger_Effect,      &RandomSpeaker::TriggerSound},
    {&EV_Trigger_SetMinDelay, &RandomSpeaker::SetMinDelay },
    {&EV_Trigger_SetMaxDelay, &RandomSpeaker::SetMaxDelay },
    {&EV_Trigger_SetChance,   &RandomSpeaker::SetChance   },
    {&EV_Touch,               NULL                        },
    {NULL,                    NULL                        }
};

void RandomSpeaker::TriggerSound(Event *ev)

{
    ScheduleSound();
    if (G_Random(1) <= chance) {
        TriggerPlaySound::ToggleSound(ev);
    }
}

void RandomSpeaker::ScheduleSound(void)

{
    CancelEventsOfType(EV_Trigger_Effect);
    PostEvent(EV_Trigger_Effect, mindelay + G_Random(maxdelay - mindelay));
}

void RandomSpeaker::SetMinDelay(Event *ev)

{
    mindelay = ev->GetFloat(1);
}

void RandomSpeaker::SetMaxDelay(Event *ev)

{
    maxdelay = ev->GetFloat(1);
}

void RandomSpeaker::SetChance(Event *ev)

{
    chance = ev->GetFloat(1);
}

void RandomSpeaker::SetMinDelay(float newMinDelay)

{
    mindelay = newMinDelay;
}

void RandomSpeaker::SetMaxDelay(float newMaxDelay)

{
    maxdelay = newMaxDelay;
}

void RandomSpeaker::SetChance(float newChance)

{
    chance = newChance;
}

RandomSpeaker::RandomSpeaker()
{
    if (LoadingSavegame) {
        // Archive function will setup all necessary data
        return;
    }
    ambient  = qfalse;
    chance   = 1.0f;
    mindelay = 3;
    maxdelay = 10;

    ScheduleSound();
}

/*****************************************************************************/
/*QUAKED trigger_changelevel (1 0 0) ? NO_INTERMISSION x NOT_PLAYERS MONSTERS PROJECTILES

When the player touches this, he gets sent to the map listed in the "map" variable.
Unless the NO_INTERMISSION flag is set, the view will go to the info_intermission
spot and display stats.

"spawnspot"  name of the spawn location to start at in next map.
"setthread" This defaults to "LevelComplete" and should point to a thread that is called just
before the level ends.

"triggerable" turn trigger on
"nottriggerable" turn trigger off

If NOT_PLAYERS is set, the trigger does not respond to players
If MONSTERS is set, the trigger will respond to monsters
If PROJECTILES is set, the trigger will respond to projectiles (rockets, grenades, etc.)

******************************************************************************/

Event EV_TriggerChangeLevel_Map
(
    "map",
    EV_DEFAULT,
    "s",
    "map_name",
    "Sets the map to change to when triggered.",
    EV_NORMAL
);
Event EV_TriggerChangeLevel_SpawnSpot
(
    "spawnspot",
    EV_DEFAULT,
    "s",
    "spawn_spot",
    "Sets the spawn spot to use.",
    EV_NORMAL
);

CLASS_DECLARATION(Trigger, TriggerChangeLevel, "trigger_changelevel") {
    {&EV_Trigger_Effect,               &TriggerChangeLevel::ChangeLevel },
    {&EV_TriggerChangeLevel_Map,       &TriggerChangeLevel::SetMap      },
    {&EV_TriggerChangeLevel_SpawnSpot, &TriggerChangeLevel::SetSpawnSpot},
    {NULL,                             NULL                             }
};

TriggerChangeLevel::TriggerChangeLevel()
{
    if (LoadingSavegame) {
        // Archive function will setup all necessary data
        return;
    }

    // default level change thread
    map       = STRING_EMPTY;
    spawnspot = STRING_EMPTY;
}

void TriggerChangeLevel::SetMap(Event *ev)

{
    map = ev->GetConstString(1);
}

void TriggerChangeLevel::SetSpawnSpot(Event *ev)
{
    spawnspot = ev->GetConstString(1);
}

void TriggerChangeLevel::ChangeLevel(Event *ev)
{
    SafePtr<Entity> safeThis = this;
    Entity         *other;

    other = ev->GetEntity(1);

    if (level.intermissiontime) {
        // already activated
        return;
    }

    // if noexit, do a ton of damage to other
    if (DM_FLAG(DF_SAME_LEVEL) && other != world) {
        other->Damage(this, other, 10 * other->max_health, other->origin, vec_zero, vec_zero, 1000, 0, MOD_CRUSH);
        return;
    }

    parm.other.Clear();
    parm.owner.Clear();

    Unregister(STRING_TRIGGER);

    // Scripts could potentially remove the trigger
    // So make sure it's safe to continue here
    if (!safeThis) {
        return;
    }

    //
    // make sure we execute the thread before changing
    //
    m_Thread.Execute();

    if (g_gametype->integer) {
        G_BeginIntermission2();
        return;
    }

    if (spawnflags & VISIBLE) {
        level.intermissiontime = level.time;
        G_FadeOut(1.0);
        G_FadeSound(1.0);
        // Set the next map
        level.nextmap = Director.GetString(map);
    }

    if (spawnspot != STRING_EMPTY) {
        G_BeginIntermission(Director.GetString(map) + "$" + Director.GetString(spawnspot), TRANS_BSP);
    } else {
        G_BeginIntermission(Director.GetString(map), TRANS_BSP);
    }
}

const char *TriggerChangeLevel::Map(void)
{
    return Director.GetString(map).c_str();
}

const char *TriggerChangeLevel::SpawnSpot(void)
{
    return Director.GetString(spawnspot).c_str();
}

void TriggerChangeLevel::Archive(Archiver& arc)
{
    Trigger::Archive(arc);

    Director.ArchiveString(arc, map);
    Director.ArchiveString(arc, spawnspot);
}

/*****************************************************************************/
/*QUAKED trigger_use (1 0 0) ? x x NOT_PLAYERS MONSTERS

Activates targets when 'used' by an entity
"setthread" name of thread to trigger.  This can be in a different script file as well\
by using the '::' notation.

"triggerable" turn trigger on
"nottriggerable" turn trigger off

If NOT_PLAYERS is set, the trigger does not respond to players
If MONSTERS is set, the trigger will respond to monsters

******************************************************************************/

CLASS_DECLARATION(Trigger, TriggerUse, "trigger_use") {
    {&EV_Use,   &TriggerUse::TriggerStuff},
    {&EV_Touch, NULL                     },
    {NULL,      NULL                     }
};

TriggerUse::TriggerUse() {}

/*****************************************************************************/
/*QUAKED trigger_useonce (1 0 0) ? x x NOT_PLAYERS MONSTERS

Activates targets when 'used' by an entity, but only once
"setthread" name of thread to trigger.  This can be in a different script file as well\
by using the '::' notation.

"triggerable" turn trigger on
"nottriggerable" turn trigger off

If NOT_PLAYERS is set, the trigger does not respond to players
If MONSTERS is set, the trigger will respond to monsters

******************************************************************************/

CLASS_DECLARATION(TriggerUse, TriggerUseOnce, "trigger_useonce") {
    {&EV_Touch, NULL},
    {NULL,      NULL}
};

TriggerUseOnce::TriggerUseOnce()
{
    if (LoadingSavegame) {
        // Archive function will setup all necessary data
        return;
    }

    // Only allow 1 use.
    count = 1;
}

/*****************************************************************************/
/*QUAKED trigger_hurt (1 0 0) ? x x NOT_PLAYERS MONSTERS PROJECTILES

"damage" amount of damage to cause. (default 10)
"damagetype" what kind of damage should be given to the player. (default CRUSH)

"triggerable" turn trigger on
"nottriggerable" turn trigger off

If NOT_PLAYERS is set, the trigger does not hurt players
If MONSTERS is set, the trigger hurts monsters
If PROJECTILES is set, the trigger will hurt projectiles (rockets, grenades, etc.)

******************************************************************************/

Event EV_TriggerHurt_SetDamage
(
    "damage",
    EV_DEFAULT,
    "i",
    "damage",
    "Sets the amount of damage to do.",
    EV_NORMAL
);

Event EV_TriggerHurt_SetDamageType
(
    "damagetype",
    EV_DEFAULT,
    "s",
    "damageType",
    "Sets the type of damage a TriggerHurt delivers.",
    EV_NORMAL
);

CLASS_DECLARATION(Trigger, TriggerHurt, "trigger_hurt") {
    {&EV_Trigger_Effect,            &TriggerHurt::Hurt      },
    {&EV_TriggerHurt_SetDamage,     &TriggerHurt::SetDamage },
    {&EV_TriggerHurt_SetDamageType, &TriggerHurt::DamageType},
    {&EV_Touch,                     &Trigger::TriggerStuff  },
    {NULL,                          NULL                    }
};

TriggerHurt::TriggerHurt()
{
    if (LoadingSavegame) {
        // Archive function will setup all necessary data
        return;
    }

    damage      = 10;
    damage_type = MOD_CRUSH;
}

void TriggerHurt::SetDamage(Event *ev)
{
    damage = ev->GetInteger(1);
}

void TriggerHurt::Hurt(Event *ev)
{
    Entity *other;

    other = ev->GetEntity(1);

    if ((damage != 0) && !other->deadflag && !(other->flags & FL_GODMODE)) {
        other->Damage(
            this, other, damage, other->origin, vec_zero, vec_zero, 0, DAMAGE_NO_ARMOR | DAMAGE_NO_SKILL, damage_type
        );
    }
}

/*****************************************************************************/
/*QUAKED trigger_damagetargets (1 0 0) ? SOLID x NOT_PLAYERS NOT_MONSTERS PROJECTILES

"damage" amount of damage to cause. If no damage is specified, objects\
are damaged by the current health+1

if a trigger_damagetargets is shot at and the SOLID flag is set,\
the damage is passed on to the targets

"triggerable" turn trigger on
"nottriggerable" turn trigger off

If NOT_PLAYERS is set, the trigger does not hurt players
If NOT_MONSTERS is set, the trigger does not hurt monsters
If PROJECTILES is set, the trigger will hurt projectiles (rockets, grenades, etc.)

******************************************************************************/

Event EV_TriggerDamageTargets_SetDamage
(
    "damage",
    EV_DEFAULT,
    "i",
    "damage",
    "Sets the amount of damage to do.",
    EV_NORMAL
);

CLASS_DECLARATION(Trigger, TriggerDamageTargets, "trigger_damagetargets") {
    {&EV_Trigger_ActivateTargets,        &TriggerDamageTargets::DamageTargets},
    {&EV_TriggerDamageTargets_SetDamage, &TriggerDamageTargets::SetDamage    },
    {&EV_Damage,                         &TriggerDamageTargets::PassDamage   },
    {&EV_Touch,                          NULL                                },
    {NULL,                               NULL                                }
};

TriggerDamageTargets::TriggerDamageTargets()
{
    if (LoadingSavegame) {
        // Archive function will setup all necessary data
        return;
    }

    damage = 0;

    if (spawnflags & 1) {
        health     = 60;
        max_health = health;
        deadflag   = DEAD_NO;
        takedamage = DAMAGE_YES;
        setSolidType(SOLID_BBOX);
    } else {
        takedamage = DAMAGE_NO;
        setSolidType(SOLID_NOT);
    }
}

void TriggerDamageTargets::SetDamage(Event *ev)

{
    damage = ev->GetInteger(1);
}

void TriggerDamageTargets::PassDamage(Event *ev)

{
    Entity     *attacker;
    int         dmg;
    Entity     *ent;
    const char *name;

    attacker = ev->GetEntity(1);
    dmg      = ev->GetInteger(2);

    //
    // damage the targets
    //
    name = Target();
    if (name && strcmp(name, "")) {
        ent = NULL;
        do {
            ent = G_FindTarget(ent, name);
            if (!ent) {
                break;
            }

            if (!ent->deadflag) {
                ent->Damage(this, attacker, dmg, ent->origin, vec_zero, vec_zero, 0, 0, MOD_CRUSH);
            }
        } while (1);
    }
}

//
//==============================
// DamageTargets
//==============================
//

void TriggerDamageTargets::DamageTargets(Event *ev)

{
    Entity     *other;
    Entity     *ent;
    const char *name;

    other = ev->GetEntity(1);

    if (triggerActivated) {
        //
        // Entity triggered itself.  Prevent an infinite loop
        //
        ScriptError("Entity targeting itself--Targetname '%s'", TargetName().c_str());
        return;
    }

    triggerActivated = true;
    activator        = other;

    //
    // print the message
    //
    if (message != STRING_EMPTY && other && other->isClient()) {
        gi.centerprintf(other->edict, Director.GetString(message).c_str());
        if (Noise().length()) {
            other->Sound(Noise().c_str(), CHAN_VOICE);
        }
    }

    //
    // damage the targets
    //
    name = Target();
    if (name && strcmp(name, "")) {
        ent = NULL;
        do {
            ent = G_FindTarget(ent, name);
            if (!ent) {
                break;
            }

            if (!ent->IsDead()) {
                if (damage) {
                    ent->Damage(this, other, damage, ent->origin, vec_zero, vec_zero, 0, 0, MOD_CRUSH);
                } else {
                    ent->Damage(this, other, ent->health + 1, ent->origin, vec_zero, vec_zero, 0, 0, MOD_CRUSH);
                }
            }
        } while (1);
    }

    triggerActivated = false;
}

/*****************************************************************************/
/*QUAKED trigger_camerause (1 0 0) ? VISIBLE x NOT_PLAYERS MONSTERS

Activates 'targeted' camera when 'used'
If activated, toggles through cameras
"setthread" name of thread to trigger.  This can be in a different script file as well\
by using the '::' notation.

"triggerable" turn trigger on
"nottriggerable" turn trigger off

If NOT_PLAYERS is set, the trigger does not respond to players
If MONSTERS is set, the trigger will respond to monsters

******************************************************************************/

CLASS_DECLARATION(TriggerUse, TriggerCameraUse, "trigger_camerause") {
    {&EV_Use,   &TriggerCameraUse::TriggerCamera},
    {&EV_Touch, NULL                            },
    {NULL,      NULL                            }
};

void TriggerCameraUse::TriggerCamera(Event *ev)
{
    ScriptThreadLabel camthread;
    Entity           *other;

    other = ev->GetEntity(1);
    if (other->isClient()) {
        Player *client;
        Entity *ent;
        Camera *cam;

        client = (Player *)other;
        cam    = client->CurrentCamera();
        if (cam != NULL) {
            str nextcam;

            nextcam = cam->NextCamera();
            if (nextcam.length()) {
                ent = (Entity *)G_FindTarget(NULL, nextcam.c_str());

                if (ent) {
                    if (ent->IsSubclassOfCamera()) {
                        cam = (Camera *)ent;
                        client->SetCamera(cam, CAMERA_SWITCHTIME);
                        ent->Unregister(STRING_TRIGGER);
                    }
                }
            }
        } else {
            ent = (Entity *)G_FindTarget(NULL, Target());
            if (ent) {
                if (ent->IsSubclassOfCamera()) {
                    cam = (Camera *)ent;
                    client->SetCamera(cam, CAMERA_SWITCHTIME);
                    ent->Unregister(STRING_TRIGGER);
                } else {
                    warning("TriggerCamera", "%s is not a camera", Target().c_str());
                }
            }
        }

        camthread.Execute(this);
    }
}

/*****************************************************************************/
/*QUAKED trigger_exit (1 0 0) ?

When the player touches this, an exit icon will be displayed in his hud.
This is to inform him that he is near an exit.

"triggerable" turn trigger on
"nottriggerable" turn trigger off

******************************************************************************/

Event EV_TriggerExit_TurnExitOff
(
    "_turnexitoff",
    EV_DEFAULT,
    NULL,
    NULL,
    "Internal event that turns the exit sign off.",
    EV_NORMAL
);

CLASS_DECLARATION(Trigger, TriggerExit, "trigger_exit") {
    {&EV_Trigger_Effect,          &TriggerExit::DisplayExitSign},
    {&EV_TriggerExit_TurnExitOff, &TriggerExit::TurnExitSignOff},
    {NULL,                        NULL                         }
};

TriggerExit::TriggerExit()
{
    wait = 1;
}

void TriggerExit::TurnExitSignOff(Event *ev)

{
    level.near_exit = false;
}

void TriggerExit::DisplayExitSign(Event *ev)

{
    level.near_exit = qtrue;

    CancelEventsOfType(EV_TriggerExit_TurnExitOff);
    PostEvent(EV_TriggerExit_TurnExitOff, 1.1f);
}

/*****************************************************************************/
/*       trigger_box (0.5 0.5 0.5) ? x x NOT_PLAYERS MONSTERS PROJECTILES

Variable sized repeatable trigger.  Must be targeted at one or more entities.  Made to
be spawned via script.

If "health" is set, the trigger must be killed to activate each time.
If "delay" is set, the trigger waits some time after activating before firing.

"setthread" name of thread to trigger.  This can be in a different script file as well\
by using the '::' notation.

"wait" : Seconds between triggerings. (.2 default)
"cnt" how many times it can be triggered (infinite default)

"triggerable" turn trigger on
"nottriggerable" turn trigger off

If NOT_PLAYERS is set, the trigger does not respond to players
If MONSTERS is set, the trigger will respond to monsters
If PROJECTILES is set, the trigger will respond to projectiles (rockets, grenades, etc.)

set "message" to text string

******************************************************************************/

Event EV_TriggerBox_SetMins
(
    "mins",
    EV_DEFAULT,
    "v",
    "mins",
    "Sets the minimum bounds of the trigger box.",
    EV_NORMAL
);
Event EV_TriggerBox_SetMaxs
(
    "maxs",
    EV_DEFAULT,
    "v",
    "maxs",
    "Sets the maximum bounds of the trigger box.",
    EV_NORMAL
);

CLASS_DECLARATION(Trigger, TriggerBox, "trigger_box") {
    {&EV_TriggerBox_SetMins, &TriggerBox::SetMins},
    {&EV_TriggerBox_SetMaxs, &TriggerBox::SetMaxs},
    {NULL,                   NULL                }
};

void TriggerBox::SetMins(Event *ev)
{
    Vector org;

    mins = ev->GetVector(1);
    org  = (mins + maxs) * 0.5;

    setSize(mins - org, maxs - org);
    setOrigin(org);
}

void TriggerBox::SetMaxs(Event *ev)
{
    Vector org;

    maxs = ev->GetVector(1);
    org  = (mins + maxs) * 0.5;

    setSize(mins - org, maxs - org);
    setOrigin(org);
}

/*****************************************************************************/
/*QUAKED trigger_music (1 0 0) ? NORMAL ACTION NOT_PLAYERS MONSTERS PROJECTILES SUSPENSE MYSTERY SURPRISE

Variable sized repeatable trigger to change the music mood.  

If "delay" is set, the trigger waits some time after activating before firing.
"current" can be used to set the current mood
"fallback" can be used to set the fallback mood
"altcurrent" can be used to set the current mood of the opposite face, if multiFaceted
"altfallback" can be used to set the fallback mood of the opposite face, if multiFaceted
"edgeTriggerable" trigger only fires when entering a trigger
"multiFaceted" if 1, then trigger is North/South separate triggerable\
if 2, then trigger East/West separate triggerable

"thread" name of thread to trigger.  This can be in a different script file as well\
by using the '::' notation.
"wait" : Seconds between triggerings. (1.0 default)
"cnt" how many times it can be triggered (infinite default)
"oneshot" make this a one time trigger

"triggerable" turn trigger on
"nottriggerable" turn trigger off

If NOT_PLAYERS is set, the trigger does not respond to players
If MONSTERS is set, the trigger will respond to monsters
If PROJECTILES is set, the trigger will respond to projectiles (rockets, grenades, etc.)

NORMAL, ACTION, SUSPENSE, MYSTERY, and SURPRISE are the moods that can be triggered

******************************************************************************/

Event EV_TriggerMusic_CurrentMood
(
    "current",
    EV_DEFAULT,
    "s",
    "current_mood",
    "Sets the current mood to use when triggered.",
    EV_NORMAL
);
Event EV_TriggerMusic_FallbackMood
(
    "fallback",
    EV_DEFAULT,
    "s",
    "fallback_mood",
    "Sets the fallback mood to use when triggered.",
    EV_NORMAL
);
Event EV_TriggerMusic_AltCurrentMood
(
    "altcurrent",
    EV_DEFAULT,
    "s",
    "alternate_current_mood",
    "Sets the alternate current mood to use when triggered.",
    EV_NORMAL
);
Event EV_TriggerMusic_AltFallbackMood
(
    "altfallback",
    EV_DEFAULT,
    "s",
    "alterante_fallback_mood",
    "Sets the alternate fallback mood to use when triggered.",
    EV_NORMAL
);
Event EV_TriggerMusic_OneShot
(
    "oneshot",
    EV_DEFAULT,
    NULL,
    NULL,
    "Make this a one time trigger.",
    EV_NORMAL
);

CLASS_DECLARATION(Trigger, TriggerMusic, "trigger_music") {
    {&EV_TriggerMusic_CurrentMood,     &TriggerMusic::SetCurrentMood    },
    {&EV_TriggerMusic_FallbackMood,    &TriggerMusic::SetFallbackMood   },
    {&EV_TriggerMusic_AltCurrentMood,  &TriggerMusic::SetAltCurrentMood },
    {&EV_TriggerMusic_AltFallbackMood, &TriggerMusic::SetAltFallbackMood},
    {&EV_TriggerMusic_OneShot,         &TriggerMusic::SetOneShot        },
    {&EV_Trigger_Effect,               &TriggerMusic::ChangeMood        },
    {&EV_Trigger_Effect_Alt,           &TriggerMusic::AltChangeMood     },
    {NULL,                             NULL                             }
};

TriggerMusic::TriggerMusic()
{
    if (LoadingSavegame) {
        // Archive function will setup all necessary data
        return;
    }
    removable        = false;
    triggerActivated = false;
    activator        = NULL;
    trigger_time     = (float)0;
    edgeTriggered    = qtrue;

    setMoveType(MOVETYPE_NONE);
    setSolidType(SOLID_TRIGGER);

    setContents(0);
    edict->r.svFlags |= SVF_NOCLIENT;

    delay      = 0;
    wait       = 1.0f;
    health     = 0;
    max_health = 0;

    trigger_time = (float)0;

    SetOneShot(false);

    noise = STRING_EMPTY;

    current  = STRING_NORMAL;
    fallback = STRING_NORMAL;

    altcurrent  = STRING_NORMAL;
    altfallback = STRING_NORMAL;

    // setup sound based on spawn flags
    if (spawnflags & 1) {
        current = STRING_NORMAL;
    } else if (spawnflags & 2) {
        current = STRING_ACTION;
    } else if (spawnflags & 32) {
        current = STRING_SUSPENSE;
    } else if (spawnflags & 64) {
        current = STRING_MYSTERY;
    } else if (spawnflags & 128) {
        current = STRING_SURPRISE;
    }
}

void TriggerMusic::SetMood(str crnt, str fback)
{
    current  = Director.AddString(crnt);
    fallback = Director.AddString(fback);
}

void TriggerMusic::SetAltMood(str crnt, str fback)
{
    altcurrent  = Director.AddString(crnt);
    altfallback = Director.AddString(fback);
}

void TriggerMusic::SetCurrentMood(Event *ev)
{
    current = ev->GetConstString(1);
}

void TriggerMusic::SetFallbackMood(Event *ev)
{
    fallback = ev->GetConstString(1);
}

void TriggerMusic::SetAltCurrentMood(Event *ev)
{
    altcurrent = ev->GetConstString(1);
}

void TriggerMusic::SetAltFallbackMood(Event *ev)
{
    altfallback = ev->GetConstString(1);
}

void TriggerMusic::ChangeMood(Event *ev)
{
    ChangeMusic(Director.GetString(current).c_str(), Director.GetString(fallback).c_str(), qfalse);
}

void TriggerMusic::AltChangeMood(Event *ev)
{
    ChangeMusic(Director.GetString(altcurrent).c_str(), Director.GetString(altfallback).c_str(), qfalse);
}

void TriggerMusic::SetOneShot(qboolean once)
{
    trigger_time = 0.0f;
    oneshot      = once;
    if (oneshot) {
        count = 1;
    } else {
        count = -1;
    }
}

void TriggerMusic::SetOneShot(Event *ev)
{
    SetOneShot(true);
}

void TriggerMusic::Archive(Archiver& arc)
{
    Trigger::Archive(arc);

    arc.ArchiveBoolean(&oneshot);
    Director.ArchiveString(arc, current);
    Director.ArchiveString(arc, fallback);
    Director.ArchiveString(arc, altcurrent);
    Director.ArchiveString(arc, altfallback);
}

/*****************************************************************************/
/*QUAKED trigger_reverb (1 0 0) ? x x NOT_PLAYERS MONSTERS PROJECTILES

Variable sized repeatable trigger to change the reverb level in the game

If "delay" is set, the trigger waits some time after activating before firing.
"reverbtype" what kind of reverb should be used
"reverblevel" how much of the reverb effect should be applied
"altreverbtype" what kind of reverb should be used
"altreverblevel" how much of the reverb effect should be applied
"edgeTriggerable" trigger only fires when entering a trigger
"multiFaceted" if 1, then trigger is North/South separate triggerable\
if 2, then trigger East/West separate triggerable

"setthread" name of thread to trigger.  This can be in a different script file as well\
by using the '::' notation.
"wait" : Seconds between triggerings. (1.0 default)
"cnt" how many times it can be triggered (infinite default)
"oneshot" make this a one time trigger

"triggerable" turn trigger on
"nottriggerable" turn trigger off

If NOT_PLAYERS is set, the trigger does not respond to players
If MONSTERS is set, the trigger will respond to monsters
If PROJECTILES is set, the trigger will respond to projectiles (rockets, grenades, etc.)

******************************************************************************/

Event EV_TriggerReverb_ReverbType
(
    "reverbtype",
    EV_DEFAULT,
    "i",
    "reverbType",
    "Sets the reverb type.",
    EV_NORMAL
);

Event EV_TriggerReverb_ReverbLevel
(
    "reverblevel",
    EV_DEFAULT,
    "f",
    "reverbLevel",
    "Sets the reverb level to be used when triggered.",
    EV_NORMAL
);

Event EV_TriggerReverb_AltReverbType
(
    "altreverbtype",
    EV_DEFAULT,
    "i",
    "reverbType",
    "Sets the reverb type.",
    EV_NORMAL
);

Event EV_TriggerReverb_AltReverbLevel
(
    "altreverblevel",
    EV_DEFAULT,
    "f",
    "reverbLevel",
    "Sets the reverb level to be used when triggered.",
    EV_NORMAL
);

Event EV_TriggerReverb_OneShot
(
    "oneshot",
    EV_DEFAULT,
    NULL,
    NULL,
    "Make this a one time trigger.",
    EV_NORMAL
);

CLASS_DECLARATION(Trigger, TriggerReverb, "trigger_music") {
    {&EV_TriggerReverb_ReverbType,     &TriggerReverb::SetReverbType    },
    {&EV_TriggerReverb_ReverbLevel,    &TriggerReverb::SetReverbLevel   },
    {&EV_TriggerReverb_AltReverbType,  &TriggerReverb::SetAltReverbType },
    {&EV_TriggerReverb_AltReverbLevel, &TriggerReverb::SetAltReverbLevel},
    {&EV_TriggerReverb_OneShot,        &TriggerReverb::SetOneShot       },
    {&EV_Trigger_Effect,               &TriggerReverb::ChangeReverb     },
    {&EV_Trigger_Effect_Alt,           &TriggerReverb::AltChangeReverb  },
    {NULL,                             NULL                             }
};

TriggerReverb::TriggerReverb()
{
    if (LoadingSavegame) {
        // Archive function will setup all necessary data
        return;
    }
    removable        = false;
    triggerActivated = false;
    activator        = NULL;
    trigger_time     = (float)0;
    edgeTriggered    = qtrue;

    setMoveType(MOVETYPE_NONE);
    setSolidType(SOLID_TRIGGER);

    setContents(0);
    edict->r.svFlags |= SVF_NOCLIENT;

    delay      = 0;
    wait       = 1.0f;
    health     = 0;
    max_health = 0;

    trigger_time = (float)0;

    SetOneShot(false);

    noise = STRING_EMPTY;

    reverbtype     = 0;
    altreverbtype  = 0;
    reverblevel    = 0.5f;
    altreverblevel = 0.5f;
}

void TriggerReverb::SetReverb(int type, float level)
{
    reverbtype  = type;
    reverblevel = level;
}

void TriggerReverb::SetAltReverb(int type, float level)
{
    altreverbtype  = type;
    altreverblevel = level;
}

void TriggerReverb::SetReverbType(Event *ev)
{
    reverbtype = ev->GetInteger(1);
}

void TriggerReverb::SetReverbLevel(Event *ev)
{
    reverblevel = ev->GetFloat(1);
}

void TriggerReverb::SetAltReverbType(Event *ev)
{
    altreverbtype = ev->GetInteger(1);
}

void TriggerReverb::SetAltReverbLevel(Event *ev)
{
    altreverblevel = ev->GetFloat(1);
}

void TriggerReverb::ChangeReverb(Event *ev)
{
    Entity *other;

    other = ev->GetEntity(1);
    if (other->isClient()) {
        Player *client;

        client = (Player *)other;
        client->SetReverb(reverbtype, reverblevel);
        gi.DPrintf("reverb set to %s at level %.2f\n", EAXMode_NumToName(reverbtype), reverblevel);
    }
}

void TriggerReverb::AltChangeReverb(Event *ev)
{
    Entity *other;

    other = ev->GetEntity(1);
    if (other->isClient()) {
        Player *client;

        client = (Player *)other;
        client->SetReverb(altreverbtype, altreverblevel);
        gi.DPrintf("reverb set to %s at level %.2f\n", EAXMode_NumToName(altreverbtype), altreverblevel);
    }
}

void TriggerReverb::SetOneShot(qboolean once)
{
    trigger_time = 0.0f;
    oneshot      = once;
    if (oneshot) {
        count = 1;
    } else {
        count = -1;
    }
}

void TriggerReverb::SetOneShot(Event *ev)
{
    SetOneShot(true);
}

/*****************************************************************************/
/*QUAKED trigger_pushobject (1 0 0) ?
Special trigger that can only be triggered by a push object.

"triggername" if set, trigger only responds to objects with a targetname the same as triggername.
"cnt" how many times it can be triggered (default 1, use -1 for infinite)
******************************************************************************/

Event EV_TriggerByPushObject_TriggerName
(
    "triggername",
    EV_DEFAULT,
    "s",
    "targetname_of_object",
    "If set, trigger will only respond to objects with specified name.",
    EV_NORMAL
);

CLASS_DECLARATION(TriggerOnce, TriggerByPushObject, "trigger_pushobject") {
    {&EV_TriggerByPushObject_TriggerName, &TriggerByPushObject::setTriggerName},
    {NULL,                                NULL                                }
};

void TriggerByPushObject::setTriggerName(Event *event)
{
    triggername = event->GetConstString(1);
}

qboolean TriggerByPushObject::respondTo(Entity *other)

{
    if (other->isSubclassOf(PushObject)) {
        if (triggername != STRING_EMPTY) {
            return (Director.GetString(triggername) == other->TargetName());
        }

        return qtrue;
    }

    return qfalse;
}

Entity *TriggerByPushObject::getActivator(Entity *other)

{
    Entity *owner;

    if (other->isSubclassOf(PushObject)) {
        owner = ((PushObject *)other)->getOwner();

        if (owner) {
            return owner;
        }
    }

    return other;
}

void TriggerByPushObject::Archive(Archiver& arc)
{
    TriggerOnce::Archive(arc);

    Director.ArchiveString(arc, triggername);
}

Event EV_TriggerGivePowerup_OneShot("oneshot", EV_DEFAULT, NULL, NULL, "Make this a one time trigger.");

Event EV_TriggerGivePowerup_PowerupName(
    "powerupname", EV_DEFAULT, "s", "powerup_name", "Specifies the powerup to give to the sentient."
);

CLASS_DECLARATION(Trigger, TriggerGivePowerup, "trigger_givepowerup") {
    {&EV_TriggerGivePowerup_OneShot,     &TriggerGivePowerup::SetOneShot    },
    {&EV_TriggerGivePowerup_PowerupName, &TriggerGivePowerup::SetPowerupName},
    {&EV_Trigger_Effect,                 &TriggerGivePowerup::GivePowerup   },
    {NULL,                               NULL                               }
};

TriggerGivePowerup::TriggerGivePowerup()
{
    if (LoadingSavegame) {
        // Archive function will setup all necessary data
        return;
    }
    removable        = false;
    triggerActivated = false;
    activator        = NULL;
    trigger_time     = (float)0;
    edgeTriggered    = qtrue;

    setMoveType(MOVETYPE_NONE);
    setSolidType(SOLID_TRIGGER);

    setContents(0);
    edict->r.svFlags |= SVF_NOCLIENT;

    delay      = 0;
    wait       = 1.0f;
    health     = 0;
    max_health = 0;

    trigger_time = (float)0;

    oneshot = false;

    count        = -1;
    noise        = STRING_EMPTY;
    powerup_name = STRING_EMPTY;
}

void TriggerGivePowerup::SetOneShot(Event *ev)
{
    trigger_time = 0.f;
    oneshot      = true;
    count        = 1;
}

void TriggerGivePowerup::SetPowerupName(Event *ev)
{
    powerup_name = ev->GetConstString(1);
}

void TriggerGivePowerup::GivePowerup(Event *ev)
{
    Entity   *other = ev->GetEntity(1);
    Sentient *sen;
    str       powerup_name_str;

    if (!other->IsSubclassOfSentient()) {
        // ignore non-sentient entities
        return;
    }

    sen              = static_cast<Sentient *>(other);
    powerup_name_str = Director.GetString(powerup_name);
    if (sen->FindItem(powerup_name_str)) {
        // sentient already has that item, so ignore
        return;
    }

    // NOTE:
    // Little bug found in original mohaa, even in 2.40!!
    // Sentient::giveItem() expects an str and powerup_name is a const_str
    // which is an int. It means that powerup_name is implicitly passed as an integer value
    // for the str(int) constructor, which will convert the number to string
    //sen->giveItem(powerup_name);

    // Pass the obtained str instead
    sen->giveItem(powerup_name_str);
}

void TriggerGivePowerup::Archive(Archiver& arc)
{
    Trigger::Archive(arc);

    arc.ArchiveBoolean(&oneshot);
    Director.ArchiveString(arc, powerup_name);
}

/*****************************************************************************
/*QUAKED trigger_clickitem (1 0 0) ?

Trigger that detects a hit by items like the camera of binoculars.

If "delay" is set, the trigger waits some time after activating before firing.
"reverbtype" what kind of reverb should be used
"reverblevel" how much of the reverb effect should be applied
"altreverbtype" what kind of reverb should be used
"altreverblevel" how much of the reverb effect should be applied
"edgeTriggerable" trigger only fires when entering a trigger
"multiFaceted" if 1, then trigger is North/South separate triggerable\
if 2, then trigger East/West separate triggerable

"setthread" name of thread to trigger.  This can be in a different script file as well\
by using the '::' notation.
"wait" : Seconds between triggerings. (1.0 default)
"cnt" how many times it can be triggered (infinite default)
"oneshot" make this a one time trigger

"triggerable" turn trigger on
"nottriggerable" turn trigger off

******************************************************************************/

CLASS_DECLARATION(Trigger, TriggerClickItem, "trigger_clickitem") {
    {&EV_Model, &TriggerClickItem::SetClickItemModelEvent},
    {&EV_Touch, NULL                                     },
    {NULL,      NULL                                     }
};

TriggerClickItem::TriggerClickItem()
{
    if (LoadingSavegame) {
        return;
    }

    setMoveType(MOVETYPE_NONE);
    setContents(CONTENTS_UNKNOWN3);
    setSolidType(SOLID_BBOX);
    edict->r.svFlags |= SVF_NOCLIENT;
}

void TriggerClickItem::SetClickItemModelEvent(Event *ev)
{
    SetModelEvent(ev);
    setContents(CONTENTS_UNKNOWN3);
    edict->r.svFlags |= SVF_NOCLIENT;
    link();
}

CLASS_DECLARATION(TriggerUse, TriggerNoDamage, "trigger_nodamage") {
    {&EV_Touch,          &TriggerNoDamage::TakeNoDamage},
    {&EV_Trigger_Effect, &TriggerNoDamage::TakeNoDamage},
    {NULL,               NULL                          }
};

void TriggerNoDamage::TakeNoDamage(Event *ev)
{
    Entity *ent = ev->GetEntity(1);
    Event  *newev;

    ent->ProcessEvent(EV_NoDamage);
    ent->CancelEventsOfType(EV_TakeDamage);

    // Make sure the entity will take damage
    // outside the trigger
    newev = new Event(EV_TakeDamage);
    ent->PostEvent(newev, level.frametime * 3);
}

CLASS_DECLARATION(Trigger, TriggerEntity, "trigger_entity") {
    {NULL, NULL}
};

Event EV_Trigger_IsAbandoned
(
    "isabandoned",
    EV_DEFAULT,
    "e",
    "owner",
    "Returns true if the owner has abandoned the mine",
    EV_RETURN
);

Event EV_Trigger_IsImmune
(
    "isimmune",
    EV_DEFAULT,
    "e",
    "owner",
    "Returns true if the given entity is immune to this landmine",
    EV_RETURN
);

Event EV_Trigger_SetDamageable
(
    "damageable",
    EV_DEFAULT,
    "b",
    "isDamageable",
    "Makes the trigger able to take damage, but it is up to the script to watch this damage",
    EV_RETURN
);

CLASS_DECLARATION(TriggerEntity, TriggerLandmine, "trigger_landmine") {
    {&EV_Trigger_IsAbandoned,   &TriggerLandmine::EventIsAbandoned  },
    {&EV_Trigger_IsImmune,      &TriggerLandmine::EventIsImmune     },
    {&EV_Trigger_SetDamageable, &TriggerLandmine::EventSetDamageable},
    {NULL,                      NULL                                }
};

TriggerLandmine::TriggerLandmine()
{
    setMoveType(MOVETYPE_TOSS);
    setSolidType(SOLID_TRIGGER);
    setContentsSolid();
    edict->clipmask = MASK_LANDMINE;
    setSize(mins, maxs);

    team       = -1;
    max_health = health;
    deadflag   = DEAD_NO;
    takedamage = DAMAGE_YES;
}

void TriggerLandmine::EventIsAbandoned(Event *ev)
{
    Entity *other = ev->GetEntity(1);
    Player *p;
    str     weapontype;

    if (!other) {
        ev->AddInteger(1);
        return;
    }

    if (!other->IsSubclassOfPlayer()) {
        ev->AddInteger(0);
        return;
    }

    p = static_cast<Player *>(other);

    if (team && team != p->GetTeam()) {
        ev->AddInteger(1);
    }

    weapontype = p->GetCurrentDMWeaponType();

    if (!str::icmp(weapontype, "landmine")) {
        ev->AddInteger(1);
    } else {
        ev->AddInteger(0);
    }
}

void TriggerLandmine::EventIsImmune(Event *ev)
{
    Entity *ent = ev->GetEntity(1);
    ev->AddInteger(IsImmune(ent));
}

void TriggerLandmine::EventSetDamageable(Event *ev)
{
    SetDamageable(ev->GetBoolean(1));
}

void TriggerLandmine::SetDamageable(qboolean damageable)
{
    if (damageable) {
        setContentsSolid();
    } else {
        setContents(0);
    }
}

qboolean TriggerLandmine::IsImmune(Entity *other) const
{
    if (!other) {
        return qtrue;
    }

    if (entnum == other->entnum) {
        return qtrue;
    }

    if (edict->r.ownerNum == other->entnum) {
        return qtrue;
    }

    if (!other->inheritsFrom(&Player::ClassInfo)) {
        return qfalse;
    }

    return static_cast<Player *>(other)->GetTeam() == team;
}

void TriggerLandmine::SetTeam(int team)
{
    this->team = team;
}
