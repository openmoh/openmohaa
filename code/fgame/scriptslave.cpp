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

// scriptslave.cpp: Standard scripted objects.  Controlled by ScriptThread.  These objects
// are bmodel objects created in the editor and controlled by an external
// text based script.  Commands are interpretted on by one and executed
// upon a signal from the script master.  The base script object can
// perform several different relative and specific rotations and translations
// and can cause other parts of the script to be executed when touched, damaged,
// touched, or used.
//

#include "g_local.h"
#include "g_phys.h"
#include "class.h"
#include "mover.h"
#include "scriptmaster.h"
#include "scriptthread.h"
#include "scriptslave.h"
#include "scriptexception.h"
#include "sentient.h"
#include "weapon.h"
#include "gibs.h"
#include "explosion.h"
#include "game.h"
#include "debuglines.h"
#include "weaputils.h"

/*****************************************************************************/
/*QUAKED script_object (0 0.5 1) ? NOT_SOLID

******************************************************************************/

Event EV_ScriptSlave_DoMove
(
    "move",
    EV_DEFAULT,
    NULL,
    NULL,
    "Move the script slave.",
    EV_NORMAL
);
Event EV_ScriptSlave_WaitMove
(
    "waitmove",
    EV_DEFAULT,
    NULL,
    NULL,
    "Move the script slave and wait until finished.",
    EV_NORMAL
);
Event EV_ScriptSlave_Angles
(
    "angles",
    EV_DEFAULT,
    "v",
    "angles",
    "Sets the angles.",
    EV_NORMAL
);
Event EV_ScriptSlave_Trigger
(
    "trigger",
    EV_DEFAULT,
    "e",
    "ent",
    "Trigger entities target.",
    EV_NORMAL
);
Event EV_ScriptSlave_Next
(
    "next",
    EV_DEFAULT,
    NULL,
    NULL,
    "Goto the next waypoint.",
    EV_NORMAL
);
Event EV_ScriptSlave_JumpTo
(
    "jumpto",
    EV_DEFAULT,
    "s",
    "vector_or_entity",
    "Jump to specified vector or entity.",
    EV_NORMAL
);
Event EV_ScriptSlave_MoveTo
(
    "moveto",
    EV_DEFAULT,
    "s",
    "vector_or_entity",
    "Move to the specified vector or entity.",
    EV_NORMAL
);
Event EV_ScriptSlave_Speed
(
    "speed",
    EV_DEFAULT,
    "f",
    "speed",
    "Sets the speed.",
    EV_NORMAL
);
Event EV_ScriptSlave_Time
(
    "time",
    EV_DEFAULT,
    "f",
    "travel_time",
    "Sets the travel time.",
    EV_NORMAL
);
Event EV_ScriptSlave_MoveUp
(
    "moveUp",
    EV_DEFAULT,
    "f",
    "dist",
    "Move the position up.",
    EV_NORMAL
);
Event EV_ScriptSlave_MoveDown
(
    "moveDown",
    EV_DEFAULT,
    "f",
    "dist",
    "Move the position down.",
    EV_NORMAL
);
Event EV_ScriptSlave_MoveNorth
(
    "moveNorth",
    EV_DEFAULT,
    "f",
    "dist",
    "Move the position north.",
    EV_NORMAL
);
Event EV_ScriptSlave_MoveSouth
(
    "moveSouth",
    EV_DEFAULT,
    "f",
    "dist",
    "Move the position south.",
    EV_NORMAL
);
Event EV_ScriptSlave_MoveEast
(
    "moveEast",
    EV_DEFAULT,
    "f",
    "dist",
    "Move the position east.",
    EV_NORMAL
);
Event EV_ScriptSlave_MoveWest
(
    "moveWest",
    EV_DEFAULT,
    "f",
    "dist",
    "Move the position west.",
    EV_NORMAL
);
Event EV_ScriptSlave_MoveForward
(
    "moveForward",
    EV_DEFAULT,
    "f",
    "dist",
    "Move the position forward.",
    EV_NORMAL
);
Event EV_ScriptSlave_MoveBackward
(
    "moveBackward",
    EV_DEFAULT,
    "f",
    "dist",
    "Move the position backward.",
    EV_NORMAL
);
Event EV_ScriptSlave_MoveLeft
(
    "moveLeft",
    EV_DEFAULT,
    "f",
    "dist",
    "Move the position left.",
    EV_NORMAL
);
Event EV_ScriptSlave_MoveRight
(
    "moveRight",
    EV_DEFAULT,
    "f",
    "dist",
    "Move the position right.",
    EV_NORMAL
);
Event EV_ScriptSlave_RotateXDownTo
(
    "rotateXdownto",
    EV_DEFAULT,
    "f",
    "angle",
    "Rotate the x down to angle.",
    EV_NORMAL
);
Event EV_ScriptSlave_RotateYDownTo
(
    "rotateYdownto",
    EV_DEFAULT,
    "f",
    "angle",
    "Rotate the y down to angle.",
    EV_NORMAL
);
Event EV_ScriptSlave_RotateZDownTo
(
    "rotateZdownto",
    EV_DEFAULT,
    "f",
    "angle",
    "Rotate the z down to angle.",
    EV_NORMAL
);
Event EV_ScriptSlave_RotateAxisDownTo
(
    "rotateaxisdownto",
    EV_DEFAULT,
    "if",
    "axis angle",
    "Rotate the specified axis down to angle.",
    EV_NORMAL
);
Event EV_ScriptSlave_RotateXUpTo
(
    "rotateXupto",
    EV_DEFAULT,
    "f",
    "angle",
    "Rotate the x up to angle.",
    EV_NORMAL
);
Event EV_ScriptSlave_RotateYUpTo
(
    "rotateYupto",
    EV_DEFAULT,
    "f",
    "angle",
    "Rotate the y up to angle.",
    EV_NORMAL
);
Event EV_ScriptSlave_RotateZUpTo
(
    "rotateZupto",
    EV_DEFAULT,
    "f",
    "angle",
    "Rotate the z up to angle.",
    EV_NORMAL
);
Event EV_ScriptSlave_RotateAxisUpTo
(
    "rotateaxisupto",
    EV_DEFAULT,
    "if",
    "axis angle",
    "Rotate the specified axis up to angle.",
    EV_NORMAL
);
Event EV_ScriptSlave_RotateXDown
(
    "rotateXdown",
    EV_DEFAULT,
    "f",
    "angle",
    "Rotate the x down by the specified amount.",
    EV_NORMAL
);
Event EV_ScriptSlave_RotateYDown
(
    "rotateYdown",
    EV_DEFAULT,
    "f",
    "angle",
    "Rotate the y down by the specified amount.",
    EV_NORMAL
);
Event EV_ScriptSlave_RotateZDown
(
    "rotateZdown",
    EV_DEFAULT,
    "f",
    "angle",
    "Rotate the z down by the specified amount.",
    EV_NORMAL
);
Event EV_ScriptSlave_RotateAxisDown
(
    "rotateaxisdown",
    EV_DEFAULT,
    "if",
    "axis angle",
    "Rotate the specified axis down by the specified amount.",
    EV_NORMAL
);
Event EV_ScriptSlave_RotateXUp
(
    "rotateXup",
    EV_DEFAULT,
    "f",
    "angle",
    "Rotate the x up by the specified amount.",
    EV_NORMAL
);
Event EV_ScriptSlave_RotateYUp
(
    "rotateYup",
    EV_DEFAULT,
    "f",
    "angle",
    "Rotate the y up by the specified amount.",
    EV_NORMAL
);
Event EV_ScriptSlave_RotateZUp
(
    "rotateZup",
    EV_DEFAULT,
    "f",
    "angle",
    "Rotate the z up by the specified amount.",
    EV_NORMAL
);
Event EV_ScriptSlave_RotateAxisUp
(
    "rotateaxisup",
    EV_DEFAULT,
    "if",
    "axis angle",
    "Rotate the specified axis up by the specified amount.",
    EV_NORMAL
);
Event EV_ScriptSlave_RotateX
(
    "rotateX",
    EV_DEFAULT,
    "f",
    "avelocity",
    "Rotate about the x axis at the specified angular velocity.",
    EV_NORMAL
);
Event EV_ScriptSlave_RotateY
(
    "rotateY",
    EV_DEFAULT,
    "f",
    "avelocity",
    "Rotate about the y axis at the specified angular velocity.",
    EV_NORMAL
);
Event EV_ScriptSlave_RotateZ
(
    "rotateZ",
    EV_DEFAULT,
    "f",
    "avelocity",
    "Rotate about the z axis at the specified angular velocity.",
    EV_NORMAL
);
Event EV_ScriptSlave_RotateAxis
(
    "rotateaxis",
    EV_DEFAULT,
    "if",
    "axis avelocity",
    "Rotate about the specified axis at the specified angular velocity.",
    EV_NORMAL
);
Event EV_ScriptSlave_RotateDownTo
(
    "rotatedownto",
    EV_DEFAULT,
    "v",
    "direction",
    "Rotate down to the specified direction.",
    EV_NORMAL
);
Event EV_ScriptSlave_RotateUpTo
(
    "rotateupto",
    EV_DEFAULT,
    "v",
    "direction",
    "Rotate up to the specified direction.",
    EV_NORMAL
);
Event EV_ScriptSlave_RotateTo
(
    "rotateto",
    EV_DEFAULT,
    "v",
    "direction",
    "Rotate to the specified direction.",
    EV_NORMAL
);
Event EV_ScriptSlave_SetDamage
(
    "setdamage",
    EV_DEFAULT,
    "i",
    "damage",
    "Set the damage.",
    EV_NORMAL
);
Event EV_ScriptSlave_SetMeansOfDeath
(
    "setmeansofdeath",
    EV_DEFAULT,
    "s",
    "means_of_death",
    "Set the damage means of death.",
    EV_NORMAL
);
Event EV_ScriptSlave_SetDamageSpawn
(
    "dmg",
    EV_DEFAULT,
    "i",
    "damage",
    "Set the damage.",
    EV_NORMAL
);
Event EV_ScriptSlave_FollowPath
(
    "followpath",
    EV_DEFAULT,
    "eSSSSSS",
    "path arg1 arg2 arg3 arg4 arg5 arg6",
    "Makes the script slave follow the specified path.  The allowable arguments are ignoreangles,\n"
    "normalangles, loop, and a number specifying the start time.",
    EV_NORMAL
);
Event EV_ScriptSlave_EndPath
(
    "endpath",
    EV_DEFAULT,
    NULL,
    NULL,
    "Stop following the path",
    EV_NORMAL
);
Event EV_ScriptSlave_MoveDone
(
    "scriptslave_movedone",
    EV_DEFAULT,
    NULL,
    NULL,
    "Called when the script slave is doen moving",
    EV_NORMAL
);
Event EV_ScriptSlave_FollowingPath
(
    "scriptslave_followingpath",
    EV_DEFAULT,
    NULL,
    NULL,
    "Called every frame to actually follow the path",
    EV_NORMAL
);
Event EV_ScriptSlave_Explode
(
    "explode",
    EV_DEFAULT,
    "f",
    "damage",
    "Creates an explosion at the script slave's position",
    EV_NORMAL
);
Event EV_ScriptSlave_NotShootable
(
    "notshootable",
    EV_DEFAULT,
    NULL,
    NULL,
    "Makes the script slave not shootable",
    EV_NORMAL
);

Event EV_ScriptSlave_OpenAreaPortal
(
    "openportal",
    EV_DEFAULT,
    NULL,
    NULL,
    "Open the area portal enclosed in this object",
    EV_NORMAL
);

Event EV_ScriptSlave_CloseAreaPortal
(
    "closeportal",
    EV_DEFAULT,
    NULL,
    NULL,
    "Close the area portal enclosed in this object",
    EV_NORMAL
);

Event EV_ScriptSlave_PhysicsOn
(
    "physics_on",
    EV_DEFAULT,
    "I",
    "no_collide_entity",
    "Turn physics on this script object on\n"
    "If no_collide_entity is set to 1 then the script slave will not collide with other entities",
    EV_NORMAL
);

Event EV_ScriptSlave_PhysicsOff
(
    "physics_off",
    EV_DEFAULT,
    NULL,
    NULL,
    "Turn physics off this script object on",
    EV_NORMAL
);

Event EV_ScriptSlave_PhysicsVelocity
(
    "physics_velocity",
    EV_DEFAULT,
    "v",
    "impulseVector",
    "Add a physical impulse to an object when it is being physically simulated",
    EV_NORMAL
);

Event EV_ScriptSlave_FlyPath
(
    "flypath",
    EV_DEFAULT,
    "efff",
    "array speed acceleration look_ahead",
    "Makes the script slave fly the specified path with speed and acceleration until reached_distance close to "
    "position",
    EV_NORMAL
);

Event EV_ScriptSlave_ModifyFlyPath
(
    "modifyflypath",
    EV_DEFAULT,
    "efff",
    "array speed acceleration look_ahead",
    "Makes the script slave fly the specified path with speed and acceleration until reached_distance close to "
    "position",
    EV_NORMAL
);

CLASS_DECLARATION(Mover, ScriptSlave, "script_object") {
    {&EV_Bind,                         &ScriptSlave::BindEvent         },
    {&EV_Unbind,                       &ScriptSlave::EventUnbind       },
    {&EV_ScriptSlave_DoMove,           &ScriptSlave::DoMove            },
    {&EV_ScriptSlave_WaitMove,         &ScriptSlave::WaitMove          },
    {&EV_ScriptSlave_Angles,           &ScriptSlave::SetAnglesEvent    },
    {&EV_SetAngle,                     &ScriptSlave::SetAngleEvent     },
    {&EV_Model,                        &ScriptSlave::SetModelEvent     },
    {&EV_ScriptSlave_Trigger,          &ScriptSlave::TriggerEvent      },
    {&EV_ScriptSlave_Next,             &ScriptSlave::GotoNextWaypoint  },
    {&EV_ScriptSlave_JumpTo,           &ScriptSlave::JumpTo            },
    {&EV_ScriptSlave_MoveTo,           &ScriptSlave::MoveToEvent       },
    {&EV_ScriptSlave_Speed,            &ScriptSlave::SetSpeed          },
    {&EV_ScriptSlave_Time,             &ScriptSlave::SetTime           },
    {&EV_ScriptSlave_MoveUp,           &ScriptSlave::MoveUp            },
    {&EV_ScriptSlave_MoveDown,         &ScriptSlave::MoveDown          },
    {&EV_ScriptSlave_MoveNorth,        &ScriptSlave::MoveNorth         },
    {&EV_ScriptSlave_MoveSouth,        &ScriptSlave::MoveSouth         },
    {&EV_ScriptSlave_MoveEast,         &ScriptSlave::MoveEast          },
    {&EV_ScriptSlave_MoveWest,         &ScriptSlave::MoveWest          },
    {&EV_ScriptSlave_MoveForward,      &ScriptSlave::MoveForward       },
    {&EV_ScriptSlave_MoveBackward,     &ScriptSlave::MoveBackward      },
    {&EV_ScriptSlave_MoveLeft,         &ScriptSlave::MoveLeft          },
    {&EV_ScriptSlave_MoveRight,        &ScriptSlave::MoveRight         },
    {&EV_ScriptSlave_RotateXDownTo,    &ScriptSlave::RotateXdownto     },
    {&EV_ScriptSlave_RotateYDownTo,    &ScriptSlave::RotateYdownto     },
    {&EV_ScriptSlave_RotateZDownTo,    &ScriptSlave::RotateZdownto     },
    {&EV_ScriptSlave_RotateXUpTo,      &ScriptSlave::RotateXupto       },
    {&EV_ScriptSlave_RotateYUpTo,      &ScriptSlave::RotateYupto       },
    {&EV_ScriptSlave_RotateZUpTo,      &ScriptSlave::RotateZupto       },
    {&EV_ScriptSlave_RotateXDown,      &ScriptSlave::RotateXdown       },
    {&EV_ScriptSlave_RotateYDown,      &ScriptSlave::RotateYdown       },
    {&EV_ScriptSlave_RotateZDown,      &ScriptSlave::RotateZdown       },
    {&EV_ScriptSlave_RotateXUp,        &ScriptSlave::RotateXup         },
    {&EV_ScriptSlave_RotateYUp,        &ScriptSlave::RotateYup         },
    {&EV_ScriptSlave_RotateZUp,        &ScriptSlave::RotateZup         },
    {&EV_ScriptSlave_RotateX,          &ScriptSlave::RotateX           },
    {&EV_ScriptSlave_RotateY,          &ScriptSlave::RotateY           },
    {&EV_ScriptSlave_RotateZ,          &ScriptSlave::RotateZ           },
    {&EV_ScriptSlave_RotateAxisDownTo, &ScriptSlave::RotateAxisdownto  },
    {&EV_ScriptSlave_RotateAxisUpTo,   &ScriptSlave::RotateAxisupto    },
    {&EV_ScriptSlave_RotateAxisDown,   &ScriptSlave::RotateAxisdown    },
    {&EV_ScriptSlave_RotateAxisUp,     &ScriptSlave::RotateAxisup      },
    {&EV_ScriptSlave_RotateAxis,       &ScriptSlave::RotateZ           },
    {&EV_ScriptSlave_SetDamage,        &ScriptSlave::SetDamage         },
    {&EV_ScriptSlave_SetMeansOfDeath,  &ScriptSlave::SetMeansOfDeath   },
    {&EV_ScriptSlave_SetDamageSpawn,   &ScriptSlave::SetDamage         },
    {&EV_ScriptSlave_FollowPath,       &ScriptSlave::FollowPath        },
    {&EV_ScriptSlave_EndPath,          &ScriptSlave::EndPath           },
    {&EV_ScriptSlave_FollowingPath,    &ScriptSlave::FollowingPath     },
    {&EV_ScriptSlave_MoveDone,         &ScriptSlave::MoveEnd           },
    {&EV_Damage,                       &ScriptSlave::DamageFunc        },
    {&EV_ScriptSlave_RotateDownTo,     &ScriptSlave::Rotatedownto      },
    {&EV_ScriptSlave_RotateUpTo,       &ScriptSlave::Rotateupto        },
    {&EV_ScriptSlave_RotateTo,         &ScriptSlave::Rotateto          },
    {&EV_ScriptSlave_Explode,          &ScriptSlave::Explode           },
    {&EV_ScriptSlave_NotShootable,     &ScriptSlave::NotShootable      },
    {&EV_ScriptSlave_OpenAreaPortal,   &ScriptSlave::OpenPortal        },
    {&EV_ScriptSlave_CloseAreaPortal,  &ScriptSlave::ClosePortal       },
    {&EV_ScriptSlave_PhysicsOn,        &ScriptSlave::PhysicsOn         },
    {&EV_ScriptSlave_PhysicsOff,       &ScriptSlave::PhysicsOff        },
    {&EV_ScriptSlave_PhysicsVelocity,  &ScriptSlave::PhysicsVelocity   },
    {&EV_ScriptSlave_FlyPath,          &ScriptSlave::EventFlyPath      },
    {&EV_ScriptSlave_ModifyFlyPath,    &ScriptSlave::EventModifyFlyPath},

    {NULL,                             NULL                            }
};

cvar_t *g_showflypath;

ScriptSlave::ScriptSlave()
{
    g_showflypath = gi.Cvar_Get("g_showflypath", "0", 0);

    AddWaitTill(STRING_TOUCH);
    AddWaitTill(STRING_BLOCK);
    AddWaitTill(STRING_TRIGGER);
    AddWaitTill(STRING_USE);
    AddWaitTill(STRING_DAMAGE);

    if (LoadingSavegame) {
        // Archive function will setup all necessary data
        return;
    }

    // this is a normal entity
    edict->s.eType = ET_GENERAL;

    speed           = 100;
    takedamage      = DAMAGE_YES;
    waypoint        = NULL;
    NewAngles       = localangles;
    NewPos          = localorigin;
    traveltime      = 0;
    commandswaiting = false;

    dmg                = 2;
    dmg_means_of_death = MOD_CRUSH;

    setMoveType(MOVETYPE_PUSH);

    if (spawnflags & 2) {
        edict->s.renderfx = RF_ALWAYSDRAW;
    }

    splinePath       = NULL;
    m_pCurPath       = NULL;
    m_iCurNode       = 0;
    m_fLookAhead     = 0;
    m_fCurSpeed      = 0;
    m_fIdealSpeed    = 0;
    m_fIdealAccel    = 0;
    m_fIdealDistance = 100;
    splineangles     = false;
    attack_finished  = 0;

    setSolidType(SOLID_NOT);

    if (spawnflags & 1) {
        PostEvent(EV_BecomeNonSolid, EV_POSTSPAWN);
    }

    edict->s.eFlags |= EF_LINKANGLES;
}

ScriptSlave::~ScriptSlave()
{
    if (splinePath) {
        delete splinePath;
        splinePath = NULL;
    }
}

void ScriptSlave::CheckNewOrders(void)
{
    // make sure position and angles are current
    if (!commandswaiting) {
        commandswaiting = true;
        NewAngles       = localangles;
        NewPos          = localorigin;
    }
}

void ScriptSlave::NewMove(void)
{
    float dist;

    CheckNewOrders();

    commandswaiting = false;

    if (RegisterSize(0)) {
        Event ev = Event(EV_DelayThrow);
        ev.AddConstString(STRING_FAIL);
        BroadcastEvent(0, ev);
    }

    if (m_pCurPath) {
        PostEvent(EV_ScriptSlave_FollowingPath, 0);
    } else if (splinePath) {
        moving = true;
        PostEvent(EV_ScriptSlave_FollowingPath, 0);
    } else {
        float t = traveltime;
        if (t == 0) {
            dist = Vector(NewPos - localorigin).length();
            t    = dist / speed;
        }
        moving = true;
        LinearInterpolate(NewPos, NewAngles, t, EV_ScriptSlave_MoveDone);
    }
}

void ScriptSlave::BindEvent(Event *ev)

{
    Entity *ent;

    ent = ev->GetEntity(1);
    if (ent) {
        bind(ent);
    }

    // make sure position and angles are current
    NewAngles = localangles;
    NewPos    = localorigin;
}

void ScriptSlave::EventUnbind(Event *ev)

{
    unbind();

    // make sure position and angles are current
    NewAngles = localangles;
    NewPos    = localorigin;
}

void ScriptSlave::DoMove(Event *ev)
{
    NewMove();
}

void ScriptSlave::WaitMove(Event *ev)
{
    NewMove();
    Register(0, Director.CurrentThread());
}

void ScriptSlave::MoveEnd(Event *ev)
{
    Unregister(0);
}

void ScriptSlave::SetAnglesEvent(Event *ev)

{
    commandswaiting = true;
    SetAngles(ev);
    NewAngles = localangles;
}

void ScriptSlave::SetAngleEvent(Event *ev)

{
    float angle;

    angle = ev->GetFloat(1);
    if (angle == -1) {
        ForwardDir = Vector(0, 0, 90);
    } else if (angle == -2) {
        ForwardDir = Vector(0, 0, -90);
    } else {
        ForwardDir = Vector(0, angle, 0);
    }
}

void ScriptSlave::SetModelEvent(Event *ev)
{
    str m;

    m = ev->GetString(1);

    edict->r.svFlags &= ~SVF_NOCLIENT;

    if (!m || strstr(m, ".tik")) {
        setSolidType(SOLID_BBOX);
        setModel(m);
        if (!edict->s.modelindex) {
            hideModel();
            setSolidType(SOLID_NOT);
        }
    } else if (strstr(m, ".spr")) {
        setModel(m);
        if (!edict->s.modelindex) {
            hideModel();
        }
        setSolidType(SOLID_NOT);
    } else {
        setModel(m);
        if (edict->s.modelindex) {
            setSolidType(SOLID_BSP);
        } else {
            hideModel();
            setSolidType(SOLID_NOT);
        }
    }

    if (!edict->s.modelindex) {
        hideModel();
        setSolidType(SOLID_NOT);
    }
}

void ScriptSlave::TriggerEvent(Event *ev)

{
    Entity *ent;
    Event  *e;

    ent = ev->GetEntity(1);
    if (ent) {
        target = ent->TargetName();

        e = new Event(EV_Trigger_ActivateTargets);
        //fixme
        //get "other"
        e->AddEntity(world);
        ProcessEvent(e);
    }
}

void ScriptSlave::GotoNextWaypoint(Event *ev)

{
    commandswaiting = true;

    if (!waypoint) {
        ScriptError("%s is currently not at a waypoint", TargetName().c_str());
        return;
    }

    waypoint = (Waypoint *)G_FindTarget(NULL, waypoint->Target());
    if (!waypoint) {
        ScriptError("%s could not find waypoint %s", TargetName().c_str(), waypoint->Target().c_str());
        return;
    } else {
        NewPos = waypoint->origin;
    }
}

void ScriptSlave::JumpTo(Event *ev)
{
    Entity *part;

    //
    // see if it is a vector
    //
    if (ev->IsVectorAt(1)) {
        NewPos = ev->GetVector(1);
        if (bindmaster) {
            localorigin = bindmaster->getLocalVector(NewPos - bindmaster->origin);
        } else {
            localorigin = NewPos;
        }

        for (part = this; part; part = part->teamchain) {
            part->setOrigin();
            part->origin.copyTo(part->edict->s.origin2);
            part->edict->s.renderfx |= RF_FRAMELERP;
        }
    } else {
        waypoint = ev->GetWaypoint(1);
        if (waypoint) {
            NewPos = waypoint->origin;
            if (bindmaster) {
                localorigin = bindmaster->getLocalVector(NewPos - bindmaster->origin);
            } else {
                localorigin = NewPos;
            }

            for (part = this; part; part = part->teamchain) {
                part->setOrigin();
                part->origin.copyTo(part->edict->s.origin2);
                part->edict->s.renderfx |= RF_FRAMELERP;
            }
        }
    }
}

void ScriptSlave::MoveToEvent(Event *ev)

{
    commandswaiting = true;

    //
    // see if it is a vector
    //
    if (ev->IsVectorAt(1)) {
        NewPos = ev->GetVector(1);
    } else {
        waypoint = (Waypoint *)ev->GetEntity(1);
        if (waypoint) {
            NewPos = waypoint->origin;
        }
    }
}

void ScriptSlave::SetSpeed(Event *ev)

{
    speed      = ev->GetFloat(1);
    traveltime = 0;
}

void ScriptSlave::SetTime(Event *ev)

{
    traveltime = ev->GetFloat(1);
}

// Relative move commands

void ScriptSlave::MoveUp(Event *ev)
{
    CheckNewOrders();
    NewPos[2] += ev->GetFloat(1);
}

void ScriptSlave::MoveDown(Event *ev)
{
    CheckNewOrders();
    NewPos[2] -= ev->GetFloat(1);
}

void ScriptSlave::MoveNorth(Event *ev)
{
    CheckNewOrders();
    NewPos[1] += ev->GetFloat(1);
}

void ScriptSlave::MoveSouth(Event *ev)
{
    CheckNewOrders();
    NewPos[1] -= ev->GetFloat(1);
}

void ScriptSlave::MoveEast(Event *ev)
{
    CheckNewOrders();
    NewPos[0] += ev->GetFloat(1);
}

void ScriptSlave::MoveWest(Event *ev)
{
    CheckNewOrders();
    NewPos[0] -= ev->GetFloat(1);
}

void ScriptSlave::MoveForward(Event *ev)
{
    Vector v;
    Vector t;

    CheckNewOrders();

    t = NewAngles + ForwardDir;
    t.AngleVectorsLeft(&v, NULL, NULL);

    NewPos += v * ev->GetFloat(1);
}

void ScriptSlave::MoveBackward(Event *ev)
{
    Vector v;
    Vector t;

    CheckNewOrders();

    t = NewAngles + ForwardDir;
    t.AngleVectorsLeft(&v, NULL, NULL);

    NewPos -= v * ev->GetFloat(1);
}

void ScriptSlave::MoveLeft(Event *ev)
{
    Vector v;
    Vector t;

    CheckNewOrders();

    t = NewAngles + ForwardDir;
    t.AngleVectorsLeft(NULL, &v, NULL);

    NewPos += v * ev->GetFloat(1);
}

void ScriptSlave::MoveRight(Event *ev)
{
    Vector t;
    Vector v;

    CheckNewOrders();

    t = NewAngles + ForwardDir;
    t.AngleVectorsLeft(NULL, &v, NULL);

    NewPos -= v * ev->GetFloat(1);
}

// exact rotate commands

void ScriptSlave::RotateXdownto(Event *ev)
{
    CheckNewOrders();

    NewAngles[0] = ev->GetFloat(1);
    if (NewAngles[0] > localangles[0]) {
        NewAngles[0] -= 360;
    }
}

void ScriptSlave::RotateYdownto(Event *ev)
{
    CheckNewOrders();

    NewAngles[1] = ev->GetFloat(1);
    if (NewAngles[1] > localangles[1]) {
        NewAngles[1] -= 360;
    }
}

void ScriptSlave::RotateZdownto(Event *ev)
{
    CheckNewOrders();

    NewAngles[2] = ev->GetFloat(1);
    if (NewAngles[2] > localangles[2]) {
        NewAngles[2] -= 360;
    }
}

void ScriptSlave::RotateAxisdownto(Event *ev)
{
    int axis;
    CheckNewOrders();

    axis            = ev->GetInteger(1);
    NewAngles[axis] = ev->GetFloat(2);
    if (NewAngles[axis] > localangles[axis]) {
        NewAngles[axis] -= 360;
    }
}

void ScriptSlave::RotateXupto(Event *ev)
{
    CheckNewOrders();

    NewAngles[0] = ev->GetFloat(1);
    if (NewAngles[0] < localangles[0]) {
        NewAngles[0] += 360;
    }
}

void ScriptSlave::RotateYupto(Event *ev)
{
    CheckNewOrders();

    NewAngles[1] = ev->GetFloat(1);
    if (NewAngles[1] < localangles[1]) {
        NewAngles[1] += 360;
    }
}

void ScriptSlave::RotateZupto(Event *ev)
{
    CheckNewOrders();

    NewAngles[2] = ev->GetFloat(1);
    if (NewAngles[2] < localangles[2]) {
        NewAngles[2] += 360;
    }
}

void ScriptSlave::RotateAxisupto(Event *ev)
{
    int axis;
    CheckNewOrders();

    axis            = ev->GetInteger(1);
    NewAngles[axis] = ev->GetFloat(2);
    if (NewAngles[axis] < localangles[axis]) {
        NewAngles[axis] += 360;
    }
}

// full vector rotation

void ScriptSlave::Rotatedownto(Event *ev)
{
    Vector ang;

    CheckNewOrders();

    ang = ev->GetVector(1);

    NewAngles[0] = ang[0];
    if (NewAngles[0] > localangles[0]) {
        NewAngles[0] -= 360;
    }
    NewAngles[1] = ang[1];
    if (NewAngles[1] > localangles[1]) {
        NewAngles[1] -= 360;
    }
    NewAngles[2] = ang[2];
    if (NewAngles[2] > localangles[2]) {
        NewAngles[2] -= 360;
    }
}

void ScriptSlave::Rotateupto(Event *ev)
{
    Vector ang;

    CheckNewOrders();

    ang = ev->GetVector(1);

    NewAngles[0] = ang[0];
    if (NewAngles[0] < localangles[0]) {
        NewAngles[0] += 360;
    }
    NewAngles[1] = ang[1];
    if (NewAngles[1] < localangles[1]) {
        NewAngles[1] += 360;
    }
    NewAngles[2] = ang[2];
    if (NewAngles[2] < localangles[2]) {
        NewAngles[2] += 360;
    }
}

void ScriptSlave::Rotateto(Event *ev)
{
    Vector ang;

    CheckNewOrders();

    ang = ev->GetVector(1);

    NewAngles = ang;
}

// Relative rotate commands

void ScriptSlave::RotateXdown(Event *ev)
{
    CheckNewOrders();
    NewAngles[0] = localangles[0] - ev->GetFloat(1);
}

void ScriptSlave::RotateYdown(Event *ev)
{
    CheckNewOrders();
    NewAngles[1] = localangles[1] - ev->GetFloat(1);
}

void ScriptSlave::RotateZdown(Event *ev)
{
    CheckNewOrders();
    NewAngles[2] = localangles[2] - ev->GetFloat(1);
}

void ScriptSlave::RotateAxisdown(Event *ev)
{
    int axis;
    CheckNewOrders();

    axis            = ev->GetInteger(1);
    NewAngles[axis] = localangles[axis] - ev->GetFloat(2);
}

void ScriptSlave::RotateXup(Event *ev)
{
    CheckNewOrders();
    NewAngles[0] = localangles[0] + ev->GetFloat(1);
}

void ScriptSlave::RotateYup(Event *ev)
{
    CheckNewOrders();
    NewAngles[1] = localangles[1] + ev->GetFloat(1);
}

void ScriptSlave::RotateZup(Event *ev)
{
    CheckNewOrders();
    NewAngles[2] = localangles[2] + ev->GetFloat(1);
}

void ScriptSlave::RotateAxisup(Event *ev)
{
    int axis;
    CheckNewOrders();

    axis            = ev->GetInteger(1);
    NewAngles[axis] = localangles[axis] + ev->GetFloat(2);
}

void ScriptSlave::RotateX(Event *ev)

{
    avelocity[0] = ev->GetFloat(1);
}

void ScriptSlave::RotateY(Event *ev)

{
    avelocity[1] = ev->GetFloat(1);
}

void ScriptSlave::RotateZ(Event *ev)

{
    avelocity[2] = ev->GetFloat(1);
}

void ScriptSlave::RotateAxis(Event *ev)

{
    int axis;

    axis            = ev->GetInteger(1);
    avelocity[axis] = ev->GetFloat(2);
}

void ScriptSlave::DamageFunc(Event *ev)
{
    Unregister(STRING_DAMAGE);
}

void ScriptSlave::SetDamage(Event *ev)

{
    dmg = ev->GetInteger(1);
}

void ScriptSlave::SetMeansOfDeath(Event *ev)

{
    dmg_means_of_death = MOD_string_to_int(ev->GetString(1));
}

void ScriptSlave::CreatePath(SplinePath *path, splinetype_t type)

{
    SplinePath *node;

    if (!splinePath) {
        splinePath = new BSpline;
    }

    splinePath->Clear();
    splinePath->SetType(type);

    node = path;
    while (node != NULL) {
        splinePath->AppendControlPoint(node->origin, node->angles, node->speed);
        node = node->GetNext();

        if (node == path) {
            break;
        }
    }
}

void ScriptSlave::FollowPath(Event *ev)
{
    int         i, argnum;
    Entity     *ent;
    str         token;
    SplinePath *path;
    qboolean    clamp;
    float       starttime;

    ent          = ev->GetEntity(1);
    argnum       = 2;
    starttime    = -2;
    clamp        = true;
    ignoreangles = false;
    splineangles = true;
    for (i = argnum; i <= ev->NumArgs(); i++) {
        token = ev->GetString(i);
        if (!Q_stricmp(token, "ignoreangles")) {
            ignoreangles = true;
        } else if (!Q_stricmp(token, "normalangles")) {
            splineangles = false;
        } else if (!Q_stricmp(token, "loop")) {
            clamp = false;
        } else if (IsNumeric(token)) {
            starttime = atof(token);
        } else {
            ScriptError("Unknown followpath command %s.", token.c_str());
        }
    }
    if (ent && ent->IsSubclassOfSplinePath()) {
        commandswaiting = true;
        path            = (SplinePath *)ent;
        if (clamp) {
            CreatePath(path, SPLINE_CLAMP);
        } else {
            CreatePath(path, SPLINE_LOOP);
        }
        splineTime = starttime;
        CancelEventsOfType(EV_ScriptSlave_FollowingPath);
        if (!ignoreangles) {
            avelocity = vec_zero;
        }
        velocity = vec_zero;
    }
}

void ScriptSlave::EndPath(Event *ev)
{
    if (!splinePath) {
        return;
    }

    delete splinePath;
    splinePath = NULL;
    velocity   = vec_zero;
    if (!ignoreangles) {
        avelocity = vec_zero;
    }
}

void ScriptSlave::FollowingPath(Event *ev)
{
    Vector pos;
    Vector orient;
    float  speed_multiplier;

    if (m_pCurPath && m_pCurPath->m_iPoints) {
        Vector vAngles;
        Vector vDeltaAngles;
        Vector vDelta;
        float *vTmp;
        Vector vPrev;
        Vector vCur;
        Vector vTotal;
        float  fCoef;
        Vector vWishPosition;
        Vector vWishAngles;
        Vector vNextWishAngles;
        Vector primal_angles;
        Vector n_angles;

        if (g_showflypath && g_showflypath->integer) {
            for (int i = 0; i < m_pCurPath->m_iPoints; i++) {
                vTmp = m_pCurPath->GetByNode(i, NULL);
                G_DebugBBox((vTmp + 1), Vector(-32, -32, -32), Vector(32, 32, 32), 0.0f, 1.0f, 1.0f, 1.0f);

                for (int ii = 0; ii <= 8; ii++) {
                    G_DebugLine(
                        (m_pCurPath->GetByNode((ii / 10.0f + (i + 1)), NULL) + 1),
                        (m_pCurPath->GetByNode(m_fLookAhead + (ii / 10.0f + (i + 1)), NULL) + 1),
                        0.0f,
                        1.0f,
                        1.0f,
                        1.0f
                    );
                }
            }
        }

        if (m_iCurNode <= 0) {
            vTmp          = m_pCurPath->GetByNode(m_iCurNode, NULL);
            vWishPosition = (vTmp + 1);
            vDelta        = vWishPosition - origin;

            if (vDelta.length() <= 32.0f) {
                m_iCurNode++;

                if (m_iCurNode >= m_pCurPath->m_iPoints) {
                    velocity  = vec_zero;
                    avelocity = vec_zero;

                    delete m_pCurPath;
                    m_pCurPath = NULL;
                    m_iCurNode = 0;
                    moving     = false;
                    ProcessEvent(EV_ScriptSlave_MoveDone);
                    return;
                }
            }
        } else {
            vTmp  = m_pCurPath->GetByNode(m_iCurNode - 1, NULL);
            vPrev = (vTmp + 1);
            vTmp  = m_pCurPath->GetByNode(m_iCurNode, NULL);
            vCur  = (vTmp + 1);

            m_vIdealDir = vCur - vPrev;
            VectorNormalize(m_vIdealDir);
            angles.AngleVectorsLeft(&vWishAngles);

            fCoef = ProjectLineOnPlane(vWishAngles, DotProduct(origin, vWishAngles), vPrev, vCur, NULL);

            vTmp          = m_pCurPath->GetByNode((m_iCurNode - (1.0f - fCoef)), NULL);
            vTmp          = m_pCurPath->Get(vTmp[0] + m_fLookAhead, NULL);
            vWishPosition = (vTmp + 1);

            if (fCoef > 1.0f) {
                m_iCurNode++;

                if (m_iCurNode >= m_pCurPath->m_iPoints) {
                    velocity  = vec_zero;
                    avelocity = vec_zero;

                    delete m_pCurPath;
                    m_pCurPath = NULL;
                    m_iCurNode = 0;
                    moving     = false;
                    ProcessEvent(EV_ScriptSlave_MoveDone);
                    return;
                }
            }
        }

        vWishAngles = vWishPosition - origin;

        if (vWishAngles.length() > 0.0f) {
            VectorNormalize(vWishAngles);
            VectorToAngles(vWishAngles, vNextWishAngles);
        } else {
            AngleVectorsLeft(angles, vWishAngles, NULL, NULL);
            vNextWishAngles = angles;
        }

        vAngles = angles;

        for (int i = 0; i < 3; i++) {
            n_angles[i] = vNextWishAngles[i] - angles[i];

            if (n_angles[i] <= 180.0f) {
                if (n_angles[i] < -180.0f) {
                    n_angles[i] += 360.0f;
                }
            } else {
                n_angles[i] -= 360.0f;
            }

            float change = level.frametime * 360.0f;
            float error  = 0.33f * n_angles[i];

            if (-change > error) {
                error = -change;
            } else if (error <= change) {
                change = error;
            }

            primal_angles[i] = change + angles[i];
        }

        setAngles(primal_angles);

        vDeltaAngles = (angles - vAngles) * level.frametime;

        if (vDeltaAngles[0] > 180.0f || vDeltaAngles[0] <= -180.0f) {
            vDeltaAngles[0] = 0.0f;
        }

        if (vDeltaAngles[1] > 180.0f || vDeltaAngles[1] <= -180.0f) {
            vDeltaAngles[1] = 0.0f;
        }

        if (vDeltaAngles[2] > 180.0f || vDeltaAngles[2] <= -180.0f) {
            vDeltaAngles[2] = 0.0f;
        }

        if (vDeltaAngles[0] > -1.0f || vDeltaAngles[0] < 1.0f) {
            vDeltaAngles[0] = 0.0f;
        }

        if (vDeltaAngles[1] > -1.0f || vDeltaAngles[1] < 1.0f) {
            vDeltaAngles[1] = 0.0f;
        }

        if (vDeltaAngles[2] > -1.0f || vDeltaAngles[2] < 1.0f) {
            vDeltaAngles[2] = 0.0f;
        }

        avelocity = vDeltaAngles;
        velocity  = vWishAngles * m_fCurSpeed;

        if (m_fCurSpeed < m_fIdealSpeed) {
            m_fCurSpeed += m_fIdealAccel * level.frametime;

            if (m_fCurSpeed > m_fIdealSpeed) {
                m_fCurSpeed = m_fIdealSpeed;
            }
        } else if (m_fCurSpeed > m_fIdealSpeed) {
            m_fCurSpeed -= m_fIdealAccel * level.frametime;

            if (m_fCurSpeed < m_fIdealSpeed) {
                m_fCurSpeed = m_fIdealSpeed;
            }
        }
    } else {
        if (!splinePath) {
            return;
        }

        if ((splinePath->GetType() == SPLINE_CLAMP) && (splineTime > (splinePath->EndPoint() - 2))) {
            delete splinePath;
            splinePath = NULL;
            velocity   = vec_zero;
            if (!ignoreangles) {
                avelocity = vec_zero;
            }
            moving = false;
            ProcessEvent(EV_ScriptSlave_MoveDone);
            return;
        }

        speed_multiplier = splinePath->Eval(splineTime, pos, orient);

        splineTime += level.frametime * speed_multiplier;

        velocity = (pos - origin) * (1 / level.frametime);
        if (!ignoreangles) {
            if (splineangles) {
                avelocity = (orient - angles) * (1 / level.frametime);
            } else {
                float len;

                len = velocity.length();
                if (len > 0.05) {
                    Vector ang;
                    Vector dir;
                    float  aroll;

                    aroll           = avelocity[ROLL];
                    dir             = velocity * (1 / len);
                    ang             = dir.toAngles();
                    avelocity       = (ang - angles) * (1 / level.frametime);
                    avelocity[ROLL] = aroll;
                } else {
                    avelocity = vec_zero;
                }
            }
        }
    }

    PostEvent(EV_ScriptSlave_FollowingPath, level.frametime);
}

void ScriptSlave::Explode(Event *ev)

{
    float damage;

    if (ev->NumArgs()) {
        damage = ev->GetFloat(1);
    } else {
        damage = 120.0f;
    }

    CreateExplosion(origin, damage, this, this, this);
}

void ScriptSlave::NotShootable(Event *ev)
{
    setContents(0);
}

void ScriptSlave::OpenPortal(Event *ev)
{
    gi.AdjustAreaPortalState(this->edict, true);
}

void ScriptSlave::ClosePortal(Event *ev)
{
    gi.AdjustAreaPortalState(this->edict, false);
}

void ScriptSlave::PhysicsOn(Event *ev)
{
    commandswaiting = false;
    setMoveType(MOVETYPE_BOUNCE);
    setSolidType(SOLID_BBOX);
    velocity        = Vector(0, 0, 1);
    edict->clipmask = MASK_SOLID | CONTENTS_BODY;
    if (ev->NumArgs() == 1 && ev->GetInteger(1)) {
        edict->clipmask &= ~MASK_SCRIPT_SLAVE;
    }
}

void ScriptSlave::PhysicsOff(Event *ev)
{
    Event *event;

    commandswaiting = false;
    setMoveType(MOVETYPE_PUSH);
    edict->clipmask = 0;
    // become solid again
    event = new Event(EV_Model);
    event->AddString(model);
    PostEvent(event, 0);
}

void ScriptSlave::PhysicsVelocity(Event *ev)
{
    velocity += ev->GetVector(1);
}

void ScriptSlave::EventFlyPath(Event *ev)
{
    SimpleEntity *path;

    m_fIdealDistance = 100.0f;
    m_fLookAhead     = 256.0f;
    m_fIdealAccel    = 35.0f;
    m_fIdealSpeed    = 250.0f;

    if (ev->NumArgs() != 1 && ev->NumArgs() != 2 && ev->NumArgs() != 3 && ev->NumArgs() != 4) {
        ScriptError("wrong number of arguments");
    }

    if (ev->NumArgs() > 1) {
        m_fIdealSpeed = ev->GetFloat(2);
    }

    if (ev->NumArgs() > 2) {
        m_fIdealAccel = ev->GetFloat(3);
    }

    if (ev->NumArgs() > 3) {
        m_fLookAhead = ev->GetFloat(4);
    }

    path = ev->GetSimpleEntity(1);

    if (!path) {
        ScriptError("ScriptSlave Given FlyPath Command with NULL path.");
    }

    if (m_pCurPath) {
        delete m_pCurPath;
    }

    m_pCurPath = new cSpline<4, 512>;

    SetupPath(m_pCurPath, path);
    m_iCurNode = 0;
    CancelEventsOfType(EV_ScriptSlave_FollowingPath);
}

void ScriptSlave::EventModifyFlyPath(Event *ev)
{
    m_fIdealDistance = 100.0f;

    if (ev->NumArgs() != 1 && ev->NumArgs() != 2 && ev->NumArgs() != 3) {
        ScriptError("wrong number of arguments");
    }

    if (ev->NumArgs() > 0) {
        m_fIdealSpeed = ev->GetFloat(1);
    }

    if (ev->NumArgs() > 1) {
        m_fIdealAccel = ev->GetFloat(2);
    }

    if (ev->NumArgs() > 2) {
        m_fLookAhead = ev->GetFloat(3);
    }
}

void ScriptSlave::SetupPath(cSpline<4, 512> *pPath, SimpleEntity *se)
{
    str name;
    //int iObjNum;
    Vector                  vLastOrigin;
    SimpleEntity           *ent;
    int                     i;
    static cSpline<4, 512> *pTmpPath = NULL;

    if (!pTmpPath) {
        pTmpPath = new cSpline<4, 512>;
    }

    if (!pPath) {
        return;
    }

    pPath->Reset();
    pTmpPath->Reset();

    vLastOrigin = se->origin;

    name = se->Target();

    if (name.c_str()) {
        Vector vDelta;
        vec4_t origin;
        float  fCurLength;

        i          = 0;
        fCurLength = 0;
        ent        = se;
        while (ent) {
            if (vLastOrigin.length() == 0.0f && i > 1) {
                Com_Printf("^~^~^Warning: ScriptSlave Flying with a Path that contains 2 equal points\n");
            } else {
                origin[0] = fCurLength;
                origin[1] = ent->origin[0];
                origin[2] = ent->origin[1];
                origin[3] = ent->origin[2];
                pTmpPath->Add(origin, 0);
                vLastOrigin = ent->origin;
                fCurLength++;
            }

            if (ent == se && i > 1) {
                break;
            }

            i++;
            ent = ent->Next();
        }

        if (pTmpPath->m_iPoints > 2) {
            float *vTmp;

            pPath->Reset();
            vTmp = pTmpPath->GetByNode(0.0f, 0);

            vLastOrigin[0] = vTmp[1];
            vLastOrigin[1] = vTmp[2];
            vLastOrigin[2] = vTmp[3];

            fCurLength = 0;

            for (i = 0; i < pTmpPath->m_iPoints; i++) {
                vTmp = pTmpPath->GetByNode(i, 0);

                vDelta = (vTmp + 1) - vLastOrigin;

                fCurLength += vDelta.length();
                origin[0] = fCurLength;
                origin[1] = vTmp[1];
                origin[2] = vTmp[2];
                origin[3] = vTmp[3];

                pPath->Add(origin, 0);
                vLastOrigin = (vTmp + 1);
            }
        }
    }
}

/*****************************************************************************/
/*QUAKED script_model (0 0.5 1) (0 0 0) (0 0 0) NOT_SOLID

******************************************************************************/

Event EV_ScriptModel_SetAnim
(
    "anim",
    EV_DEFAULT,
    "s",
    "anim_name",
    "Sets the script model's animation",
    EV_NORMAL
);
Event EV_ScriptModel_AnimDone
(
    "animdone",
    EV_ZERO,
    NULL,
    NULL,
    "Script model animation has finished.",
    EV_NORMAL
);
Event EV_ScriptModel_MoveAnim
(
    "moveanim",
    EV_DEFAULT,
    "s",
    "animName",
    "Makes the script model play an animation and move with the deltas contained in the animation",
    EV_NORMAL
);
Event EV_ScriptModel_MovingAnim
(
    "moving_from_anim",
    EV_DEFAULT,
    NULL,
    NULL,
    "The script model is moving based on an animation",
    EV_NORMAL
);

CLASS_DECLARATION(ScriptSlave, ScriptModel, "script_model") {
    {&EV_SetAngle,               &ScriptModel::SetAngleEvent      },
    {&EV_ScriptModel_SetAnim,    &ScriptModel::SetAnimEvent       },
    {&EV_Model,                  &ScriptModel::SetModelEvent      },
    {&EV_ScriptModel_AnimDone,   &ScriptModel::AnimDoneEvent      },
    {&EV_ScriptModel_MoveAnim,   &ScriptModel::MoveAnimEvent      },
    {&EV_ScriptModel_MovingAnim, &ScriptModel::MovingFromAnimEvent},
    {NULL,                       NULL                             },
};

ScriptModel::ScriptModel()
{
    // this is a tiki model
    edict->s.eType = ET_MODELANIM;

    AddWaitTill(STRING_ANIMDONE);
}

void ScriptModel::SetModelEvent(Event *ev)
{
    ScriptSlave::SetModelEvent(ev);

    if (edict->tiki && !mins.length() && !maxs.length()) {
        gi.TIKI_CalculateBounds(edict->tiki, edict->s.scale, mins, maxs);
    }
}

void ScriptModel::SetAnimEvent(Event *ev)
{
    str animname;

    animname = ev->GetString(1);
    if (animname.length() && edict->tiki) {
        int animnum;

        animnum = gi.Anim_NumForName(edict->tiki, animname);
        if (animnum >= 0) {
            NewAnim(animnum);
        }
    }
}

void ScriptModel::AnimDoneEvent(Event *ev)
{
    CancelEventsOfType(EV_ScriptModel_MovingAnim);
    Unregister(STRING_ANIMDONE);
}

void ScriptModel::MoveAnimEvent(Event *ev)
{
    str animName;
    int animNum;

    animName = ev->GetString(1);

    if (!animName.length()) {
        return;
    }

    animNum = gi.Anim_NumForName(edict->tiki, animName.c_str());
    if (animNum < 0) {
        ScriptError("ScriptModel could not find animation %s.", animName.c_str());
    }

    NewAnim(animNum, EV_ScriptModel_AnimDone);
    RestartAnimSlot(0);
    PostEvent(EV_ScriptModel_MovingAnim, 0);
}

void ScriptModel::MovingFromAnimEvent(Event *ev)
{
    Vector newOrigin;
    Vector newAngles;

    // calculate velocity
    newOrigin = origin + frame_delta;
    velocity  = (newOrigin - origin) / level.frametime;

    // calculate angular velocity
    newAngles = angles + Vector(0, angular_delta, 0);
    avelocity = (newAngles - angles) / level.frametime;

    PostEvent(EV_ScriptModel_MovingAnim, level.frametime);
}

void ScriptModel::SetAngleEvent(Event *ev)
{
    float angle;

    angle = ev->GetFloat(1);
    if (angle == -1) {
        ForwardDir  = Vector(0, 0, 90);
        localangles = Vector(-90, 0, 0);
    } else if (angle == -2) {
        ForwardDir  = Vector(0, 0, -90);
        localangles = Vector(90, 0, 0);
    } else {
        ForwardDir  = Vector(0, angle, 0);
        localangles = Vector(0, angle, 0);
    }

    setAngles(localangles);
}

void ScriptModel::GibEvent(Event *ev)
{
    int   num, power;
    float scale;

    setSolidType(SOLID_NOT);
    hideModel();

    if (!com_blood->integer) {
        PostEvent(EV_Remove, 0);
        return;
    }

    num   = ev->GetInteger(1);
    power = ev->GetInteger(2);
    scale = ev->GetFloat(3);

    power = -power;

    if (ev->NumArgs() > 3) {
        CreateGibs(this, power, scale, num, ev->GetString(4));
    } else {
        CreateGibs(this, power, scale, num);
    }

    PostEvent(EV_Remove, 0);
}

/*****************************************************************************/
/*QUAKED script_model_realdamage (0 0.5 1) (0 0 0) (0 0 0) NOT_SOLID ALWAYS_DRAW

******************************************************************************/
/*****************************************************************************/

CLASS_DECLARATION(ScriptModel, ScriptModelRealDamage, "script_model_realdamage") {
    {&EV_Damage, &ScriptModelRealDamage::EventDamage},
    {NULL,       NULL                               }
};

ScriptModelRealDamage::ScriptModelRealDamage()
{
    RemoveWaitTill(STRING_DAMAGE);
}

void ScriptModelRealDamage::EventDamage(Event *ev)
{
    Entity::DamageEvent(ev);
}

/*****************************************************************************/
/*QUAKED script_origin (1.0 0 0) (-8 -8 -8) (8 8 8)

Used as an alternate origin for objects.  Bind the object to the script_origin
in order to simulate changing that object's origin.
******************************************************************************/

CLASS_DECLARATION(ScriptSlave, ScriptOrigin, "script_origin") {
    {&EV_Model, &ScriptOrigin::SetModelEvent},
    {NULL,      NULL                        }
};

ScriptOrigin::ScriptOrigin()
{
    setContents(0);
    setSolidType(SOLID_NOT);
}

/*****************************************************************************/
/*QUAKED script_skyorigin (1.0 0 0) ?

Used to specify the origin of a portal sky
******************************************************************************/

CLASS_DECLARATION(ScriptSlave, ScriptSkyOrigin, "script_skyorigin") {
    {NULL, NULL}
};

ScriptSkyOrigin::ScriptSkyOrigin()
{
    edict->s.renderfx |= RF_SKYORIGIN;
    setContents(0);
    setSolidType(SOLID_NOT);
}

Event EV_ScriptSimpleStrafingGunfire_On
(
    "on",
    EV_DEFAULT,
    NULL,
    NULL,
    "Turn the gunfire on.",
    EV_NORMAL
);
Event EV_ScriptSimpleStrafingGunfire_Off
(
    "off",
    EV_DEFAULT,
    NULL,
    NULL,
    "Turn the gunfire off.",
    EV_NORMAL
);
Event EV_ScriptSimpleStrafingGunfire_Fire
(
    "fire",
    EV_DEFAULT,
    NULL,
    NULL,
    "Fire.",
    EV_NORMAL
);
Event EV_ScriptSimpleStrafingGunfire_TracerFreq
(
    "tracerfreq",
    EV_DEFAULT,
    "f",
    NULL,
    "Set the frequency of the tracers",
    EV_NORMAL
);
Event EV_ScriptSimpleStrafingGunfire_ProjectileModel
(
    "projectile",
    EV_DEFAULT,
    "s",
    NULL,
    "Set the projectile model",
    EV_NORMAL
);

CLASS_DECLARATION(ScriptSlave, ScriptSimpleStrafingGunfire, "script_simplestrafinggunfire") {
    {&EV_ScriptSimpleStrafingGunfire_On,              &ScriptSimpleStrafingGunfire::GunOn             },
    {&EV_ScriptSimpleStrafingGunfire_Off,             &ScriptSimpleStrafingGunfire::GunOff            },
    {&EV_ScriptSimpleStrafingGunfire_Fire,            &ScriptSimpleStrafingGunfire::GunFire           },
    {&EV_Weapon_FireDelay,                            &ScriptSimpleStrafingGunfire::SetFireDelay      },
    {&EV_Weapon_SetBulletRange,                       &ScriptSimpleStrafingGunfire::SetRange          },
    {&EV_Weapon_SetBulletSpread,                      &ScriptSimpleStrafingGunfire::SetSpread         },
    {&EV_Weapon_SetBulletDamage,                      &ScriptSimpleStrafingGunfire::SetDamage         },
    {&EV_Weapon_SetBulletKnockback,                   &ScriptSimpleStrafingGunfire::SetKnockback      },
    {&EV_Weapon_SetBulletThroughWood,                 &ScriptSimpleStrafingGunfire::SetThroughWood    },
    {&EV_Weapon_SetBulletThroughMetal,                &ScriptSimpleStrafingGunfire::SetThroughMetal   },
    {&EV_Weapon_SetBulletCount,                       &ScriptSimpleStrafingGunfire::SetBulletCount    },
    {&EV_ScriptSimpleStrafingGunfire_TracerFreq,      &ScriptSimpleStrafingGunfire::SetTracerFreq     },
    {&EV_ScriptSimpleStrafingGunfire_ProjectileModel, &ScriptSimpleStrafingGunfire::SetProjectileModel},
    {NULL,                                            NULL                                            }
};

ScriptSimpleStrafingGunfire::ScriptSimpleStrafingGunfire()
{
    isOn            = false;
    fireDelay       = 0.05f;
    range           = 4000;
    spread          = Vector(100, 100, 0);
    damage          = 100;
    knockback       = 0;
    throughWood     = 0;
    throughMetal    = 0;
    bulletCount     = 1;
    tracerCount     = 0;
    tracerFrequency = 0;
    projectileModel = "models/projectiles/stukaround.tik";
}

void ScriptSimpleStrafingGunfire::GunOn(Event *ev)
{
    isOn = true;

    CancelEventsOfType(&EV_ScriptSimpleStrafingGunfire_Fire);
    PostEvent(EV_ScriptSimpleStrafingGunfire_Fire, 0.05f);
}

void ScriptSimpleStrafingGunfire::GunOff(Event *ev)
{
    isOn = false;

    CancelEventsOfType(&EV_ScriptSimpleStrafingGunfire_Fire);
}

void ScriptSimpleStrafingGunfire::GunFire(Event *ev)
{
    Vector dir, right, up;
    Vector horzAngles;

    AngleVectors(angles, NULL, NULL, up);
    dir = -1 * up;

    VectorToAngles(dir, horzAngles);
    AngleVectors(horzAngles, NULL, right, up);

    dir = dir * range + right * grandom() * spread.x;
    dir += up * grandom() * spread.y;
    dir.normalize();

    ProjectileAttack(origin, dir, this, projectileModel, 1, 0, NULL);
    // continue firing
    PostEvent(EV_ScriptSimpleStrafingGunfire_Fire, fireDelay);
}

void ScriptSimpleStrafingGunfire::Archive(Archiver& arc)
{
    ScriptSlave::Archive(arc);

    arc.ArchiveBoolean(&isOn);
    arc.ArchiveFloat(&fireDelay);
    arc.ArchiveFloat(&range);
    arc.ArchiveVector(&spread);
    arc.ArchiveFloat(&damage);
    arc.ArchiveFloat(&knockback);
    arc.ArchiveFloat(&throughWood);
    arc.ArchiveFloat(&throughMetal);
    arc.ArchiveInteger(&bulletCount);
    arc.ArchiveInteger(&tracerCount);
    arc.ArchiveInteger(&tracerFrequency);
    arc.ArchiveString(&projectileModel);
}

Event EV_ScriptAimedStrafingGunfire_AimTarget
(
    "aimtarget",
    EV_DEFAULT,
    "e",
    NULL,
    "Set the aim target.",
    EV_NORMAL
);

CLASS_DECLARATION(ScriptSimpleStrafingGunfire, ScriptAimedStrafingGunfire, "script_aimedstrafinggunfire") {
    {&EV_ScriptSimpleStrafingGunfire_Fire, &ScriptAimedStrafingGunfire::GunFire},
    {NULL,                                 NULL                                }
};

ScriptAimedStrafingGunfire::ScriptAimedStrafingGunfire()
{
    aimTarget = NULL;
}

void ScriptAimedStrafingGunfire::GunFire(Event *ev)
{
    if (!aimTarget) {
        ScriptSimpleStrafingGunfire::GunFire(ev);
        return;
    }

    Vector dir, right, up;
    Vector horzAngles;

    AngleVectors(angles, NULL, NULL, up);
    dir = -1 * up;

    VectorToAngles(dir, horzAngles);
    AngleVectors(horzAngles, NULL, right, up);

    dir = dir * range + right * grandom() * spread.x;
    dir += up * grandom() * spread.y;
    dir.normalize();

    ProjectileAttack(origin, dir, this, projectileModel, 1, 0, NULL);
    // continue firing
    PostEvent(EV_ScriptSimpleStrafingGunfire_Fire, fireDelay);
}

void ScriptAimedStrafingGunfire::Archive(Archiver& arc)
{
    ScriptSimpleStrafingGunfire::Archive(arc);

    arc.ArchiveObjectPointer((Class **)&aimTarget);
}
