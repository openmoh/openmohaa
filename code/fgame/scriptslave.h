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

// scriptslave.h: Standard scripted objects.  Controlled by scriptmaster.  These objects
// are bmodel objects created in the editor and controlled by an external
// text based script.  Commands are interpretted on by one and executed
// upon a signal from the script master.  The base script object can
// perform several different relative and specific rotations and translations
// and can cause other parts of the script to be executed when touched, damaged,
// touched, or used.
//

#pragma once

#include "g_local.h"
#include "entity.h"
#include "trigger.h"
#include "mover.h"
#include "script.h"
#include "scriptmaster.h"
#include "misc.h"
#include "bspline.h"
#include "spline.h"

class ScriptSlave : public Mover
{
protected:
    float attack_finished;

    int dmg;
    int dmg_means_of_death;

public:
    qboolean commandswaiting;

    Vector TotalRotation;
    Vector NewAngles;
    Vector NewPos;
    Vector ForwardDir;
    float  speed;

    Waypoint *waypoint;
    float     traveltime;
    BSpline  *splinePath;
    float     splineTime;

    cSpline<4, 512> *m_pCurPath;
    int              m_iCurNode;

    float  m_fCurSpeed;
    float  m_fIdealSpeed;
    Vector m_vIdealPosition;
    Vector m_vIdealDir;
    float  m_fIdealAccel;
    float  m_fIdealDistance;
    float  m_fLookAhead;

    qboolean splineangles;
    qboolean ignoreangles;
    qboolean ignorevelocity;
    qboolean moving; // is the script object currently moving?

protected:
    void CheckNewOrders(void);
    void NewMove(void);

    CLASS_PROTOTYPE(ScriptSlave);

    ScriptSlave();
    ~ScriptSlave();

    void BindEvent(Event *ev);
    void EventUnbind(Event *ev);
    void DoMove(Event *ev);
    void WaitMove(Event *ev);
    void MoveEnd(Event *ev);
    void SetAnglesEvent(Event *ev);
    void SetAngleEvent(Event *ev);
    void SetModelEvent(Event *ev);
    void TriggerEvent(Event *ev) override;
    void GotoNextWaypoint(Event *ev);
    void JumpTo(Event *ev);
    void MoveToEvent(Event *ev);
    void SetSpeed(Event *ev);
    void SetTime(Event *ev);
    void MoveUp(Event *ev);
    void MoveDown(Event *ev);
    void MoveNorth(Event *ev);
    void MoveSouth(Event *ev);
    void MoveEast(Event *ev);
    void MoveWest(Event *ev);
    void MoveForward(Event *ev);
    void MoveBackward(Event *ev);
    void MoveLeft(Event *ev);
    void MoveRight(Event *ev);
    void RotateXdownto(Event *ev);
    void RotateYdownto(Event *ev);
    void RotateZdownto(Event *ev);
    void RotateAxisdownto(Event *ev);
    void RotateXupto(Event *ev);
    void RotateYupto(Event *ev);
    void RotateZupto(Event *ev);
    void RotateAxisupto(Event *ev);
    void Rotateupto(Event *ev);
    void Rotatedownto(Event *ev);
    void Rotateto(Event *ev);
    void RotateXdown(Event *ev);
    void RotateYdown(Event *ev);
    void RotateZdown(Event *ev);
    void RotateAxisdown(Event *ev);
    void RotateXup(Event *ev);
    void RotateYup(Event *ev);
    void RotateZup(Event *ev);
    void RotateAxisup(Event *ev);
    void RotateX(Event *ev);
    void RotateY(Event *ev);
    void RotateZ(Event *ev);
    void RotateAxis(Event *ev);
    void SetDamage(Event *ev);
    void SetMeansOfDeath(Event *ev);
    void FollowPath(Event *ev);
    void EndPath(Event *ev);
    void FollowingPath(Event *ev);
    void CreatePath(SplinePath *path, splinetype_t type);
    void Explode(Event *ev);
    void NotShootable(Event *ev);
    void OpenPortal(Event *ev);
    void ClosePortal(Event *ev);
    void PhysicsOn(Event *ev);
    void PhysicsOff(Event *ev);
    void PhysicsVelocity(Event *ev);
    void DamageFunc(Event *ev);
    void EventFlyPath(Event *ev);
    void EventModifyFlyPath(Event *ev);
    void SetupPath(cSpline<4, 512> *pPath, SimpleEntity *se);

    void Archive(Archiver& arc) override;
};

inline void ScriptSlave::Archive(Archiver& arc)
{
    int tempInt;

    Mover::Archive(arc);

    arc.ArchiveFloat(&attack_finished);
    arc.ArchiveInteger(&dmg);
    arc.ArchiveInteger(&dmg_means_of_death);

    arc.ArchiveBoolean(&commandswaiting);
    arc.ArchiveVector(&TotalRotation);
    arc.ArchiveVector(&NewAngles);
    arc.ArchiveVector(&NewPos);
    arc.ArchiveVector(&ForwardDir);
    arc.ArchiveFloat(&speed);
    arc.ArchiveObjectPointer((Class **)&waypoint);
    arc.ArchiveFloat(&traveltime);

    if (arc.Saving()) {
        // if it exists, archive it, otherwise place a special NULL ptr tag
        if (splinePath) {
            tempInt = ARCHIVE_POINTER_VALID;
        } else {
            tempInt = ARCHIVE_POINTER_NULL;
        }
        arc.ArchiveInteger(&tempInt);
        if (tempInt == ARCHIVE_POINTER_VALID) {
            splinePath->Archive(arc);
        }
    } else {
        arc.ArchiveInteger(&tempInt);
        if (tempInt == ARCHIVE_POINTER_VALID) {
            splinePath = new BSpline;
            splinePath->Archive(arc);
        } else {
            splinePath = NULL;
        }
    }
    arc.ArchiveFloat(&splineTime);
    arc.ArchiveBoolean(&splineangles);
    arc.ArchiveBoolean(&ignoreangles);
    arc.ArchiveBoolean(&moving);

    if (arc.Saving()) {
        // if it exists, archive it, otherwise place a special NULL ptr tag
        if (m_pCurPath) {
            tempInt = ARCHIVE_POINTER_VALID;
        } else {
            tempInt = ARCHIVE_POINTER_NULL;
        }
        arc.ArchiveInteger(&tempInt);
        if (tempInt == ARCHIVE_POINTER_VALID) {
            m_pCurPath->Archive(arc);
        }
    } else {
        arc.ArchiveInteger(&tempInt);
        if (tempInt == ARCHIVE_POINTER_VALID) {
            m_pCurPath = new cSpline<4, 512>;
            m_pCurPath->Archive(arc);
        } else {
            m_pCurPath = NULL;
        }
    }

    arc.ArchiveInteger(&m_iCurNode);
    arc.ArchiveFloat(&m_fCurSpeed);
    arc.ArchiveFloat(&m_fIdealSpeed);
    arc.ArchiveVector(&m_vIdealPosition);
    arc.ArchiveVector(&m_vIdealDir);
    arc.ArchiveFloat(&m_fIdealAccel);
    arc.ArchiveFloat(&m_fIdealDistance);
    arc.ArchiveFloat(&m_fLookAhead);
}

class ScriptModel : public ScriptSlave
{
private:
    void GibEvent(Event *ev);

public:
    CLASS_PROTOTYPE(ScriptModel);

    ScriptModel();
    void SetAngleEvent(Event *ev);
    void SetModelEvent(Event *ev);
    void SetAnimEvent(Event *ev);
    void AnimDoneEvent(Event *ev);
    void MoveAnimEvent(Event *ev);
    void MovingFromAnimEvent(Event *ev);

    void Archive(Archiver& arc) override;
};

inline void ScriptModel::Archive(Archiver& arc)
{
    ScriptSlave::Archive(arc);
}

class ScriptModelRealDamage : public ScriptModel
{
public:
    CLASS_PROTOTYPE(ScriptModelRealDamage);

    ScriptModelRealDamage();
    void EventDamage(Event *ev);

    void Archive(Archiver& arc) override;
};

inline void ScriptModelRealDamage::Archive(Archiver& arc)
{
    ScriptModel::Archive(arc);
}

class ScriptOrigin : public ScriptSlave
{
public:
    CLASS_PROTOTYPE(ScriptOrigin);
    ScriptOrigin();
};

class ScriptSkyOrigin : public ScriptSlave
{
public:
    CLASS_PROTOTYPE(ScriptSkyOrigin);
    ScriptSkyOrigin();
};

class ScriptSimpleStrafingGunfire : public ScriptSlave
{
protected:
    qboolean isOn;
    float    fireDelay;
    float    range;
    Vector   spread;
    float    damage;
    float    knockback;
    float    throughWood;
    float    throughMetal;
    int      bulletCount;
    int      tracerCount;
    int      tracerFrequency;
    str      projectileModel;

public:
    CLASS_PROTOTYPE(ScriptSimpleStrafingGunfire);

    ScriptSimpleStrafingGunfire();

    void Archive(Archiver& arc) override;

protected:
    void GunFire(Event *ev);

private:
    void GunOn(Event *ev);
    void GunOff(Event *ev);
    void SetFireDelay(Event *ev);
    void SetRange(Event *ev);
    void SetSpread(Event *ev);
    void SetDamage(Event *ev);
    void SetKnockback(Event *ev);
    void SetThroughWood(Event *ev);
    void SetThroughMetal(Event *ev);
    void SetBulletCount(Event *ev);
    void SetTracerFreq(Event *ev);
    void SetProjectileModel(Event *ev);
};

inline void ScriptSimpleStrafingGunfire::SetFireDelay(Event *ev)
{
    fireDelay = ev->GetFloat(1);
}

inline void ScriptSimpleStrafingGunfire::SetRange(Event *ev)
{
    range = ev->GetFloat(1);
}

inline void ScriptSimpleStrafingGunfire::SetSpread(Event *ev)
{
    spread.x = ev->GetFloat(1);
    spread.y = ev->GetFloat(2);
}

inline void ScriptSimpleStrafingGunfire::SetDamage(Event *ev)
{
    damage = ev->GetFloat(1);
}

inline void ScriptSimpleStrafingGunfire::SetKnockback(Event *ev)
{
    knockback = ev->GetFloat(1);
}

inline void ScriptSimpleStrafingGunfire::SetThroughWood(Event *ev)
{
    throughWood = ev->GetFloat(1);
}

inline void ScriptSimpleStrafingGunfire::SetThroughMetal(Event *ev)
{
    throughMetal = ev->GetFloat(1);
}

inline void ScriptSimpleStrafingGunfire::SetBulletCount(Event *ev)
{
    bulletCount = ev->GetInteger(1);
}

inline void ScriptSimpleStrafingGunfire::SetTracerFreq(Event *ev)
{
    tracerFrequency = ev->GetInteger(1);
    tracerCount     = 0;
}

inline void ScriptSimpleStrafingGunfire::SetProjectileModel(Event *ev)
{
    projectileModel = ev->GetString(1);
}

class ScriptAimedStrafingGunfire : public ScriptSimpleStrafingGunfire
{
private:
    Entity *aimTarget;

public:
    CLASS_PROTOTYPE(ScriptAimedStrafingGunfire);

    ScriptAimedStrafingGunfire();

    void Archive(Archiver& arc) override;

    void GunFire(Event *ev);
    void SetAimTarget(Event *ev);
};

inline void ScriptAimedStrafingGunfire::SetAimTarget(Event *ev)
{
    aimTarget = ev->GetEntity(1);
}
