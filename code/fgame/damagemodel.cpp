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

#include "damagemodel.h"

static const unsigned int DAMAGEMODEL_NOTSOLID = 1;

Event EV_DamageModel_Setup
(
    "_setup",
    EV_DEFAULT,
    NULL,
    NULL,
    "Sets up an object.",
    EV_NORMAL
);
Event EV_DamageModel_KillTrace
(
    "killtrace",
    EV_DEFAULT,
    "vvff",
    "offset direction radius distance",
    "kills all objects along the trace\n"
    "offset - initial offset from origin\n"
    "direction - angular offset orientation for trace\n"
    "radius - thickness of trace\n"
    "distance - how far to trace",
    EV_NORMAL
);
Event EV_DamageModel_SpawnOrientedBoundingBox
(
    "orientedbbox",
    EV_DEFAULT,
    "vvf",
    "mins maxs yawoffset",
    "spawn an oriented bounding box with the given dimensions and an angular offset\n"
    "mins - min dimensions of box\n"
    "maxs - max dimensions of box\n"
    "yawoffset - angular offset orientation of box",
    EV_NORMAL
);
Event EV_DamageModel_KillThread
(
    "killthread",
    EV_DEFAULT,
    "s",
    "thread",
    "Set the thread to execute when this model is killed",
    EV_NORMAL
);

CLASS_DECLARATION(Animate, DamageModel, "DamageModel") {
    {&EV_Damage,                               &DamageModel::Damaged                 },
    {&EV_Killed,                               &DamageModel::Killed                  },
    {&EV_DamageModel_Setup,                    &DamageModel::Setup                   },
    {&EV_DamageModel_KillTrace,                &DamageModel::KillTrace               },
    {&EV_DamageModel_SpawnOrientedBoundingBox, &DamageModel::SpawnOrientedBoundingBox},
    {&EV_DamageModel_KillThread,               &DamageModel::EventSetKillThread      },
    {NULL,                                     NULL                                  }
};

DamageModel::DamageModel()
{
    if (LoadingSavegame) {
        return;
    }

    setSolidType(SOLID_BBOX);
    takedamage     = DAMAGE_YES;
    boundingBoxEnt = NULL;

    health = 50;
    flags |= FL_ROTATEDBOUNDS;

    PostEvent(EV_DamageModel_Setup, EV_POSTSPAWN);
}

DamageModel::~DamageModel()
{
    if (boundingBoxEnt) {
        boundingBoxEnt->PostEvent(EV_Remove, 0);
        boundingBoxEnt = NULL;
    }
}

void DamageModel::Setup(Event *ev)
{
    max_health = health;
    deadflag   = DEAD_NO;

    // set the animation
    NewAnim("idle");

    link();
}

void DamageModel::Damaged(Event *ev)
{
    Event *newev;
    int    damage;
    str    animname;

    newev = new Event(EV_SetAnim);
    newev->AddString("idle");

    damage = ev->GetInteger(2);
    if (damage < health * 0.25) {
        animname = "pain_small";
    } else if (damage < health * 0.66) {
        animname = "pain_medium";
    } else {
        animname = "pain_large";
    }

    switch (ev->GetInteger(9)) {
    case MOD_CRUSH:
    case MOD_CRUSH_EVERY_FRAME:
    case MOD_EXPLOSION:
    case MOD_EXPLODEWALL:
    case MOD_GRENADE:
    case MOD_ROCKET:
    case MOD_VEHICLE:
    case MOD_AAGUN:
        DamageEvent(ev);
        if (damage >= health) {
            return;
        }
        break;
    default:
        break;
    }

    if (!HasAnim(animname)) {
        animname = "pain";
    }

    NewAnim("pain", newev);
}

void DamageModel::Killed(Event *ev)
{
    Entity *inflictor;
    Vector  delta;
    float   yaw;
    int     num;
    str     anim;

    takedamage = DAMAGE_NO;
    deadflag   = DEAD_DEAD;
    setSolidType(SOLID_NOT);

    inflictor = ev->GetEntity(3);
    delta     = origin - inflictor->origin;
    yaw       = AngleSubtract(delta.toYaw(), angles.y);

    num  = (fmod(yaw + 360, 360) + 22.f) / 45.f;
    anim = "death_" + str(num);
    // execute the kill thread
    label.Execute(this);

    if (!HasAnim(anim)) {
        anim = "death";
    }

    if (spawnflags & DAMAGEMODEL_NOTSOLID) {
        NewAnim(anim, EV_BecomeNonSolid);
    } else {
        NewAnim(anim);
    }

    if (killtarget.c_str() && strcmp(killtarget, "")) {
        Entity *ent = NULL;

        // remove all entities with the kill target name
        for (ent = G_FindTarget(NULL, killtarget); ent; ent = G_FindTarget(ent, killtarget)) {
            ent->PostEvent(EV_Remove, 0);
        }
    }
}

void DamageModel::KillTrace(Event *ev)
{
    Vector  offset;
    Vector  direction;
    float   radius;
    float   distance;
    Vector  transformed;
    Vector  forward;
    Vector  localFwd;
    Vector  mins, maxs;
    Vector  end;
    trace_t trace;
    Entity *ent;
    int     i;

    if (spawnflags & DAMAGEMODEL_NOTSOLID) {
        return;
    }

    offset    = ev->GetVector(1);
    direction = ev->GetVector(2);
    radius    = ev->GetFloat(3);
    distance  = ev->GetFloat(4);

    MatrixTransformVector(offset, orientation, transformed);
    transformed += origin;

    direction.AngleVectorsLeft(&forward, NULL, NULL);
    MatrixTransformVector(forward, orientation, localFwd);

    end  = transformed + localFwd * distance;
    mins = Vector(-radius, -radius, -radius);
    maxs = Vector(radius, radius, radius);
    ent  = this;

    for (i = 0; i < 11; i++) {
        float damage;

        if (transformed == end) {
            break;
        }

        trace = G_Trace(transformed, mins, maxs, end, ent, MASK_SOLID, qfalse, "KillTrace");

        if (trace.fraction >= 1 || trace.entityNum == ENTITYNUM_WORLD) {
            break;
        }

        transformed = trace.endpos;
        if (!trace.ent) {
            continue;
        }

        ent = trace.ent->entity;
        if (!ent) {
            continue;
        }

        if (ent->takedamage == DAMAGE_NO) {
            continue;
        }

        if (ent->isSubclassOf(DamageModel)) {
            damage = health * 0.5;
            if (damage < 20) {
                damage = 20;
            }
        } else {
            damage = health + 1;
        }

        ent->Damage(this, this, damage, trace.endpos, localFwd, trace.plane.normal, 0, 0, MOD_CRUSH, HITLOC_GENERAL);
    }
}

void DamageModel::SpawnOrientedBoundingBox(Event *ev)
{
    Vector mins, maxs;
    float  yawoffset;

    if (spawnflags & DAMAGEMODEL_NOTSOLID) {
        return;
    }

    mins      = ev->GetVector(1) * edict->s.scale;
    maxs      = ev->GetVector(2) * edict->s.scale;
    yawoffset = ev->GetFloat(3);

    boundingBoxEnt = new Entity();
    boundingBoxEnt->edict->r.svFlags |= SVF_PORTAL;
    boundingBoxEnt->edict->s.eFlags |= EF_LINKANGLES;
    boundingBoxEnt->edict->r.contents = CONTENTS_SOLID;
    boundingBoxEnt->setSolidType(SOLID_BBOX);

    boundingBoxEnt->angles   = angles;
    boundingBoxEnt->angles.y = fmod(angles.y + yawoffset, 360);
    boundingBoxEnt->setAngles(boundingBoxEnt->angles);
    boundingBoxEnt->setSize(mins, maxs);
    boundingBoxEnt->setOrigin(origin);
    boundingBoxEnt->DisconnectPaths();
}

void DamageModel::EventSetKillThread(Event *ev)
{
    if (ev->IsFromScript()) {
        label.SetThread(ev->GetValue(1));
    } else {
        label.Set(ev->GetString(1));
    }
}

void DamageModel::Archive(Archiver& arc)
{
    Animate::Archive(arc);

    label.Archive(arc);
    arc.ArchiveSafePointer(&boundingBoxEnt);
}
