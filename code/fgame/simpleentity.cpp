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

// simpleentity.cpp : Simple entity

#include "glb_local.h"
#include "simpleentity.h"
#include "worldspawn.h"
#include "level.h"
#include "../script/scriptexception.h"

Event EV_SetTargetname
(
    "targetname",
    EV_DEFAULT,
    "s",
    "targetName",
    "set the targetname of the entity to targetName.",
    EV_NORMAL
);
Event EV_SetOrigin
(
    "origin",
    EV_DEFAULT,
    "v",
    "newOrigin",
    "Set the origin of the entity to newOrigin.",
    EV_NORMAL
);
Event EV_SetOrigin2
(
    "origin",
    EV_DEFAULT,
    "v",
    "newOrigin",
    "Set the origin of the entity to newOrigin.",
    EV_SETTER
);
Event EV_GetOrigin
(
    "origin",
    EV_DEFAULT,
    NULL,
    NULL,
    "entity's origin",
    EV_GETTER
);
Event EV_GetCentroid
(
    "centroid",
    EV_DEFAULT,
    NULL,
    NULL,
    "entity's centroid",
    EV_GETTER
);
Event EV_SetTargetname2
(
    "targetname",
    EV_DEFAULT,
    "s",
    "targetName",
    "set the targetname of the entity to targetName.",
    EV_SETTER
);
Event EV_GetTargetname
(
    "targetname",
    EV_DEFAULT,
    NULL,
    NULL,
    "entity's targetname",
    EV_GETTER
);
Event EV_SetTarget
(
    "target",
    EV_DEFAULT,
    "s",
    "targetname_to_target",
    "target another entity with targetname_to_target.",
    EV_NORMAL
);
Event EV_SetTarget2
(
    "target",
    EV_DEFAULT,
    "s",
    "targetname_to_target",
    "target another entity with targetname_to_target.",
    EV_SETTER
);
Event EV_GetTarget
(
    "target",
    EV_DEFAULT,
    NULL,
    NULL,
    "entity's target",
    EV_GETTER
);
Event EV_SetAngles
(
	"angles",
	EV_DEFAULT,
	"v[0,360][0,360][0,360]",
	"newAngles",
	"set the angles of the entity to newAngles.",
    EV_NORMAL
);
Event EV_SetAngles2
(
	"angles",
	EV_DEFAULT,
	"v[0,360][0,360][0,360]",
	"newAngles",
	"set the angles of the entity to newAngles.",
    EV_SETTER
);
Event EV_GetAngles
(
    "angles",
    EV_DEFAULT,
    NULL,
    NULL,
    "get the angles of the entity.",
    EV_GETTER
);
Event EV_SetAngle
(
    "angle",
    EV_DEFAULT,
    "f",
    "newAngle",
    "set the angles of the entity using just one value.\n"
    "Sets the yaw of the entity or an up and down\n"
    "direction if newAngle is[ 0 - 359 ] or - 1 or - 2",
    EV_NORMAL
);
Event EV_SetAngle2
(
    "angle",
    EV_DEFAULT,
    "f",
    "newAngle",
    "set the angles of the entity using just one value.\n"
    "Sets the yaw of the entity or an up and down\n"
    "direction if newAngle is[ 0 - 359 ] or - 1 or - 2",
    EV_SETTER
);
Event EV_GetAngle
(
    "angle",
    EV_DEFAULT,
    NULL,
    NULL,
    "get the angles of the entity using just one value.\n"
    "Gets the yaw of the entity or an up and down\n"
    "direction if newAngle is[ 0 - 359 ] or - 1 or - 2",
    EV_GETTER
);
Event EV_ForwardVector
(
    "forwardvector",
    EV_DEFAULT,
    NULL,
    NULL,
    "get the forward vector of angles",
    EV_GETTER
);
Event EV_LeftVector
(
    "leftvector",
    EV_DEFAULT,
    NULL,
    NULL,
    "get the left vector of angles",
    EV_GETTER
);
Event EV_RightVector
(
    "rightvector",
    EV_DEFAULT,
    NULL,
    NULL,
    "get the right vector of angles",
    EV_GETTER
);
Event EV_UpVector
(
    "upvector",
    EV_DEFAULT,
    NULL,
    NULL,
    "get the up vector of angles",
    EV_GETTER
);

CLASS_DECLARATION(Listener, SimpleEntity, NULL) {
    {&EV_SetOrigin,      &SimpleEntity::SetOrigin       },
    {&EV_SetOrigin2,     &SimpleEntity::SetOrigin       },
    {&EV_GetOrigin,      &SimpleEntity::GetOrigin       },
    {&EV_GetCentroid,    &SimpleEntity::GetCentroid     },
    {&EV_SetTargetname,  &SimpleEntity::SetTargetname   },
    {&EV_SetTargetname2, &SimpleEntity::SetTargetname   },
    {&EV_GetTargetname,  &SimpleEntity::GetTargetname   },
    {&EV_SetTarget,      &SimpleEntity::SetTarget       },
    {&EV_SetTarget2,     &SimpleEntity::SetTarget       },
    {&EV_GetTarget,      &SimpleEntity::GetTarget       },
    {&EV_SetAngles,      &SimpleEntity::SetAngles       },
    {&EV_SetAngles2,     &SimpleEntity::SetAngles       },
    {&EV_GetAngles,      &SimpleEntity::GetAngles       },
    {&EV_SetAngle,       &SimpleEntity::SetAngleEvent   },
    {&EV_SetAngle2,      &SimpleEntity::SetAngleEvent   },
    {&EV_GetAngle,       &SimpleEntity::GetAngleEvent   },
    {&EV_ForwardVector,  &SimpleEntity::GetForwardVector},
    {&EV_LeftVector,     &SimpleEntity::GetLeftVector   },
    {&EV_RightVector,    &SimpleEntity::GetRightVector  },
    {&EV_UpVector,       &SimpleEntity::GetUpVector     },
    {NULL,               NULL                           }
};

SimpleEntity::SimpleEntity()
{
    entflags = 0;
}

SimpleEntity::~SimpleEntity()
{
    if (world) {
        world->RemoveTargetEntity(this);
    }
}

int SimpleEntity::IsSubclassOfEntity(void) const
{
    return (entflags & ECF_ENTITY);
}

int SimpleEntity::IsSubclassOfAnimate(void) const
{
    return (entflags & ECF_ANIMATE);
}

int SimpleEntity::IsSubclassOfSentient(void) const
{
    return (entflags & ECF_SENTIENT);
}

int SimpleEntity::IsSubclassOfPlayer(void) const
{
    return (entflags & ECF_PLAYER);
}

int SimpleEntity::IsSubclassOfActor(void) const
{
    return (entflags & ECF_ACTOR);
}

int SimpleEntity::IsSubclassOfItem(void) const
{
    return (entflags & ECF_ITEM);
}

int SimpleEntity::IsSubclassOfInventoryItem(void) const
{
    return (entflags & ECF_INVENTORYITEM);
}

int SimpleEntity::IsSubclassOfWeapon(void) const
{
    return (entflags & ECF_WEAPON);
}

int SimpleEntity::IsSubclassOfProjectile(void) const
{
    return (entflags & ECF_PROJECTILE);
}

int SimpleEntity::IsSubclassOfDoor(void) const
{
    return (entflags & ECF_DOOR);
}

int SimpleEntity::IsSubclassOfCamera(void) const
{
    return (entflags & ECF_CAMERA);
}

int SimpleEntity::IsSubclassOfVehicle(void) const
{
    return (entflags & ECF_VEHICLE);
}

int SimpleEntity::IsSubclassOfVehicleTank(void) const
{
    return (entflags & ECF_VEHICLETANK);
}

int SimpleEntity::IsSubclassOfVehicleTurretGun(void) const
{
    return (entflags & ECF_VEHICLETURRET);
}

int SimpleEntity::IsSubclassOfTurretGun(void) const
{
    return (entflags & ECF_TURRET);
}

int SimpleEntity::IsSubclassOfPathNode(void) const
{
    return (entflags & ECF_PATHNODE);
}

int SimpleEntity::IsSubclassOfWaypoint(void) const
{
    return (entflags & ECF_WAYPOINT);
}

int SimpleEntity::IsSubclassOfTempWaypoint(void) const
{
    return (entflags & ECF_TEMPWAYPOINT);
}

int SimpleEntity::IsSubclassOfVehiclePoint(void) const
{
    return (entflags & ECF_VEHICLEPOINT);
}

int SimpleEntity::IsSubclassOfSplinePath(void) const
{
    return (entflags & ECF_SPLINEPATH);
}

int SimpleEntity::IsSubclassOfCrateObject(void) const
{
    return (entflags & ECF_CRATEOBJECT);
}

int SimpleEntity::IsSubclassOfBot(void) const
{
    return (entflags & ECF_BOT);
}

void SimpleEntity::SetTargetName(str targetname)
{
    if (!world) {
        Com_Error(ERR_FATAL, "world spawn entity does not exist. Blame Galactus.");
    }

    world->RemoveTargetEntity(this);

    this->targetname = targetname;

    world->AddTargetEntity(this);
}

void SimpleEntity::SetTargetname(Event *ev)
{
	if (this == world)
	{
		// not sure why this code is not directly handled by World
        // as SetTargetName can be overridden
		ScriptError("world was re-targeted with targetname '%s'", targetname.c_str());
	}

    SetTargetName(ev->GetString(1));
}

void SimpleEntity::GetTargetname(Event *ev)
{
    ev->AddString(TargetName());
}

void SimpleEntity::SetTarget(Event *ev)
{
    target = ev->GetString(1);
}

void SimpleEntity::GetTarget(Event *ev)
{
    ev->AddString(Target());
}

void SimpleEntity::GetOrigin(Event *ev)
{
    ev->AddVector(origin);
}

void SimpleEntity::GetCentroid(Event *ev)
{
    ev->AddVector(centroid);
}

str& SimpleEntity::Target()
{
    return target;
}

qboolean SimpleEntity::Targeted(void)

{
    if (!targetname.length()) {
        return false;
    }
    return true;
}

str& SimpleEntity::TargetName()
{
    return targetname;
}

void SimpleEntity::SetOrigin(Event *ev)
{
    setOriginEvent(ev->GetVector(1));
}

void SimpleEntity::setOrigin(Vector origin)
{
    this->origin   = origin;
    this->centroid = origin;
}

void SimpleEntity::setOriginEvent(Vector origin)
{
    setOrigin(origin);
}

void SimpleEntity::MPrintf(const char *fmt, ...)
{
    va_list argptr;
    char    msg[MAXPRINTMSG];
    if (!*g_monitor->string) {
        return;
    }

    if (targetname == g_monitor->string) {
        va_start(argptr, fmt);
        Q_vsnprintf(msg, sizeof(msg), fmt, argptr);
        va_end(argptr);
        Com_Printf("%s", msg);
    }
}

void SimpleEntity::SetAngles(Event *ev)
{
    Vector angles;

    if (ev->NumArgs() == 1) {
        angles = ev->GetVector(1);
    } else {
        angles = Vector(ev->GetFloat(1), ev->GetFloat(2), ev->GetFloat(3));
    }

    setAngles(angles);
}

void SimpleEntity::GetAngles(Event *ev)
{
    ev->AddVector(angles);
}

void SimpleEntity::SetAngleEvent(Event *ev)
{
    Vector dir;
    float  angle = ev->GetFloat(1);

    dir = G_GetMovedir(angle);
    setAngles(dir.toAngles());
}

void SimpleEntity::GetAngleEvent(Event *ev)
{
    ev->AddFloat(angles[1]);
}

void SimpleEntity::setAngles(Vector angles)
{
    this->angles = angles.AnglesMod();
}

void SimpleEntity::GetForwardVector(Event *ev)
{
    Vector fwd;

    AngleVectorsLeft(angles, fwd, NULL, NULL);
    ev->AddVector(fwd);
}

void SimpleEntity::GetLeftVector(Event *ev)
{
    Vector left;

    AngleVectorsLeft(angles, NULL, left, NULL);
    ev->AddVector(left);
}

void SimpleEntity::GetRightVector(Event *ev)
{
    Vector right;

    AngleVectors(angles, NULL, right, NULL);
    ev->AddVector(right);
}

void SimpleEntity::GetUpVector(Event *ev)
{
    Vector up;

    AngleVectorsLeft(angles, NULL, NULL, up);
    ev->AddVector(up);
}

SimpleEntity *SimpleEntity::Next(void)
{
    SimpleEntity *ent = world->GetTarget(target, true);

    if (!ent || !ent->isSubclassOf(SimpleEntity)) {
        return NULL;
    } else {
        return ent;
    }
}

void SimpleEntity::SimpleArchive(Archiver& arc)
{
    int index;

    Listener::Archive(arc);

    arc.ArchiveVector(&angles);

    arc.ArchiveString(&target);
    arc.ArchiveString(&targetname);

    if (targetname.length()) {
        if (arc.Loading()) {
            arc.ArchiveInteger(&index);
            world->AddTargetEntityAt(this, index);
        } else {
            index = world->GetTargetnameIndex(this);
            arc.ArchiveInteger(&index);
        }
    }
}

CLASS_DECLARATION(SimpleEntity, SimpleArchivedEntity, NULL) {
    {NULL, NULL}
};

void SimpleEntity::Archive(Archiver& arc)
{
    SimpleEntity::SimpleArchive(arc);

    arc.ArchiveVector(&origin);
    arc.ArchiveVector(&centroid);
}

SimpleArchivedEntity::SimpleArchivedEntity()
{
    level.m_SimpleArchivedEntities.AddObject(this);
}

SimpleArchivedEntity::~SimpleArchivedEntity()
{
    level.m_SimpleArchivedEntities.RemoveObject(this);
}
