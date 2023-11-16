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

// crateobject.cpp : Crates

#include "crateobject.h"
#include "player.h"
#include "g_phys.h"

/*****************************************************************************
/*QUAKED func_crate (0 0.25 0.5) ? INDESTRUCTABLE NOTSTACKEDON
Brush model crate object
"spawnitems" is a list of items to spawn when the crate is destroyed.
INDESTRUCTABLE makes the crate, well, indestructable.
Since the same effect can be achieved by not making it an entity, this should only
be used when stacking indestructable crates on top of destructable crates.
Otherwise, they should just be brushes to save on entity count.
NOTSTACKED prevents the crate from causing crates above it to fall. Great for crates on shelves.
"health" sets the crate's health. Default is 300
"debristype" sets the type of debris to spawn from the crate. Valid values are from 0 - ?. Default is 0
0: wooden crate 32 units to a side
1: wooden crate 64 units to a side
2: cardboard crate 16 units to a side
3: cardboard crate 32 units to a side
******************************************************************************/

Event EV_Crate_Think
(
    "_crate_think",
    EV_DEFAULT,
    NULL,
    NULL,
    "think function for a crate."
);

Event EV_Crate_Falling
(
    "_crate_falling",
    EV_DEFAULT,
    NULL,
    NULL,
    "crate's falling."
);

Event EV_Crate_Setup
(
    "_cratesetup",
    EV_DEFAULT,
    NULL,
    NULL,
    "Does the post spawn setup of the crate"
);

Event EV_Crate_Start_Falling
(
    "_cratestartfalling",
    EV_DEFAULT,
    NULL,
    NULL,
    "Makes a crate start falling"
);

Event EV_Crate_SetDebris
(
    "debristype",
    EV_DEFAULT,
    "i",
    "type",
    "Sets the debris type of the crate"
);

CLASS_DECLARATION(Entity, CrateObject, "func_crate") {
    {&EV_Crate_Setup,         &CrateObject::CrateSetup     },
    {&EV_Crate_Falling,       &CrateObject::CrateFalling   },
    {&EV_Damage,              &CrateObject::CrateDamaged   },
    {&EV_Killed,              &CrateObject::CrateKilled    },
    {&EV_Crate_Start_Falling, &CrateObject::StartFalling   },
    {&EV_Crate_Think,         &CrateObject::CrateThink     },
    {&EV_Crate_SetDebris,     &CrateObject::CrateDebrisType},
    {NULL,                    NULL                         }
};

CrateObject::CrateObject()
{
    entflags |= ECF_CRATEOBJECT;

    AddWaitTill(STRING_DEATH);

    if (LoadingSavegame) {
        return;
    }

    edict->s.eType = ET_GENERAL;

    m_fMoveTime = 0.0f;

    health     = 100.0f;
    max_health = 100.0f;
    deadflag   = 0;
    takedamage = DAMAGE_YES;

    mass = 1200;

    m_iDebrisType = 0;

    PostEvent(EV_Crate_Setup, EV_POSTSPAWN);
}

void CrateObject::TellNeighborsToFall(void)
{
    Entity *pEnt;
    Entity *pNext;
    Vector  vMins;
    Vector  vMaxs;

    vMins[0] = mins[0] + origin[0] + 6.0f;
    vMins[1] = mins[1] + origin[1] + 6.0f;
    vMins[2] = mins[2] + origin[2] + 6.0f;

    vMaxs[0] = maxs[0] + origin[0] - 6.0f;
    vMaxs[1] = maxs[1] + origin[1] - 6.0f;
    vMaxs[2] = maxs[2] + origin[2] + 12.0f;

    for (pEnt = G_NextEntity(NULL); pEnt != NULL; pEnt = pNext) {
        pNext = G_NextEntity(pEnt);

        for (pEnt = G_NextEntity(NULL); pEnt != NULL; pEnt = pNext) {
            pNext = G_NextEntity(pEnt);

            if (pEnt != this && pEnt->IsSubclassOfCrateObject()) {
                if (vMins[0] <= absmax[0] && vMins[1] <= absmax[1] && vMins[2] <= absmax[2] && absmin[0] <= vMaxs[0]
                    && absmin[1] <= vMaxs[1] && absmin[2] <= vMaxs[2]) {
                    if (!pEnt->EventPending(EV_Crate_Start_Falling)) {
                        float time = level.frametime + level.frametime;
                        pEnt->PostEvent(EV_Crate_Start_Falling, time);
                    }
                }
            }
        }
    }
}

void CrateObject::TellNeighborsToJitter(Vector vJitterAdd)
{
    Entity *pEnt;
    Entity *pNext;
    Vector  vMins;
    Vector  vMaxs;

    vMins[0] = mins[0] + origin[0] + 6.0f;
    vMins[1] = mins[1] + origin[1] + 6.0f;
    vMins[2] = mins[2] + origin[2] + 6.0f;

    vMaxs[0] = maxs[0] + origin[0] - 6.0f;
    vMaxs[1] = maxs[1] + origin[1] - 6.0f;
    vMaxs[2] = maxs[2] + origin[2] + 12.0f;

    for (pEnt = G_NextEntity(NULL); pEnt != NULL; pEnt = pNext) {
        CrateObject *crate = (CrateObject *)pEnt;

        pNext = G_NextEntity(pEnt);

        if (pEnt == this) {
            continue;
        }

        if (!pEnt->IsSubclassOfCrateObject()) {
            continue;
        }

        if (vMins[0] > absmax[0] || vMins[1] > absmax[1] || vMins[2] > absmax[2] || absmin[0] > vMaxs[0]
            || absmin[1] > vMaxs[1] || absmin[2] > vMaxs[2]) {
            continue;
        }

        crate->m_vJitterAngles[0] = m_vJitterAngles[0] + vJitterAdd[0];

        if (crate->m_vJitterAngles[0] > m_fJitterScale * 1.25f
            || -(m_fJitterScale * 1.25f) <= crate->m_vJitterAngles[0]) {
            crate->m_vJitterAngles[0] = m_fJitterScale * 1.25f;
        }

        crate->m_vJitterAngles[2] += vJitterAdd[2];

        if (crate->m_vJitterAngles[2] > m_fJitterScale * 1.25f) {
            break;
        }

        if (-(m_fJitterScale * 1.25f) <= crate->m_vJitterAngles[2]) {
            m_vJitterAngles[2] = -(m_fJitterScale * 1.25f);
        }

        if (!crate->EventPending(EV_Crate_Think)) {
            crate->ProcessEvent(EV_Crate_Think);
        }
    }
}

void CrateObject::CrateSetup(Event *ev)
{
    Vector  vMins;
    Vector  vMaxs;
    Entity *pEnt;
    Entity *pNext;

    mass = 20 * ((size[0] / 16) * (size[1] / 16) * (size[2] / 16));

    m_vStartAngles = angles;
    m_fJitterScale = 64.0f / size[2];

    setMoveType(MOVETYPE_PUSH);
    setSolidType(SOLID_BSP);

    vMins = origin + mins + Vector(12, 12, 18);
    vMaxs = origin + maxs - Vector(12, 12, 18);

    for (pEnt = G_NextEntity(NULL); pEnt != NULL; pEnt = pNext) {
        pNext = G_NextEntity(pEnt);

        if (pEnt->entnum <= entnum) {
            continue;
        }

        if (!pEnt->IsSubclassOfCrateObject()) {
            continue;
        }

        if (vMins[0] <= pEnt->absmax[0]
            && vMins[1] <= pEnt->absmax[1]
            && vMins[2] <= pEnt->absmax[2]
            && vMaxs[0] >= pEnt->absmin[0]
            && vMaxs[1] >= pEnt->absmin[1]
            && vMaxs[2] >= pEnt->absmin[2]) {
            Com_Printf(
                "^~^~^ WARNING: func_crate entities %i and %i have overlapping volumes near (%g %g %g) to (%g %g %g)\n",
                entnum,
                pEnt->entnum,
                vMins[0],
                vMins[1],
                vMins[2],
                vMaxs[0],
                vMaxs[1],
                vMaxs[2]
            );
        }
    }
}

void CrateObject::CrateDebrisType(Event *ev)
{
    m_iDebrisType = ev->GetInteger(1);

    if (m_iDebrisType >= 0) {
        CacheResource(va("models/fx/crates/debris_%i.tik", m_iDebrisType));
    }
}

void CrateObject::StartFalling(Event *ev)
{
    setMoveType(MOVETYPE_FLY);
    m_fMoveTime = 0;

    PostEvent(EV_Crate_Think, level.frametime);

    if (!(spawnflags & CRATE_NOTSTACKEDON)) {
        TellNeighborsToFall();
    }
}

void CrateObject::CrateFalling(Event *ev)
{
    if (velocity != vec_zero || !groundentity || edict->solid != SOLID_BSP) {
        m_fMoveTime = level.time + 2.0f;
    }

    if (level.time <= m_fMoveTime) {
        PostEvent(EV_Crate_Think, level.frametime);
    } else {
        setMoveType(MOVETYPE_PUSH);
        setSolidType(SOLID_BSP);
    }
}

void CrateObject::CrateDamaged(Event *ev)
{
    Vector vDir;
    Vector vForward;
    Vector vRight;
    Vector vJitterAdd;
    int    iDamage;
    int    iMeansOfDeath;
    Vector vHitPos;
    Vector vHitDirection;
    Vector vHitNormal;

    if (!takedamage) {
        return;
    }

    iDamage       = ev->GetInteger(2);
    iMeansOfDeath = ev->GetInteger(9);
    vHitPos       = ev->GetVector(4);
    vHitDirection = ev->GetVector(5);
    vHitNormal    = ev->GetVector(6);

    vDir = (vHitDirection - vHitNormal) * 0.5f;
    AngleVectors(angles, vForward, vRight, NULL);

    vJitterAdd[0] = DotProduct(vDir, vForward) * m_fJitterScale * 0.025f * (float)iDamage;
    vJitterAdd[2] = DotProduct(vDir, vRight) * m_fJitterScale * 0.025f * (float)iDamage;

    m_vJitterAngles += Vector(vJitterAdd[0], 0, vJitterAdd[2]);

    if (m_vJitterAngles[0] > m_fJitterScale * 1.25f || -(m_fJitterScale * 1.25f) > m_vJitterAngles[0]) {
        m_vJitterAngles[0] = m_fJitterScale * 1.25f;
    }

    if (m_vJitterAngles[2] > m_fJitterScale * 1.25f) {
        m_vJitterAngles[2] = m_fJitterScale * 1.25f;
    } else if (m_vJitterAngles[2] >= -(m_fJitterScale * 1.25f)) {
        m_vJitterAngles[2] = -(m_fJitterScale * 1.25f);
    }

    vJitterAdd[1] = 0.0f;
    vJitterAdd *= -0.5f;

    TellNeighborsToJitter(vJitterAdd);

    if (!EventPending(EV_Crate_Think)) {
        ProcessEvent(EV_Crate_Think);
    }

    if (spawnflags & CRATE_INDESTRUCTABLE) {
        return;
    }

    if (iMeansOfDeath >= MOD_VEHICLE) {
        if (iMeansOfDeath == MOD_BASH) {
            iDamage *= 2;
        }
    } else {
        iDamage /= 2;
    }

    health -= iDamage;

    if (health <= 0.0f) {
        Event *event = new Event(EV_Killed);
        event->AddEntity(ev->GetEntity(1));
        event->AddInteger(iDamage);
        ev->AddEntity(ev->GetEntity(3));

        ProcessEvent(event);
    }
}

void CrateObject::CrateKilled(Event *ev)
{
    Vector  vCenter;
    Entity *attacker = ev->GetEntity(1);

    if (attacker->IsSubclassOfPlayer()) {
        Player *p = (Player *)attacker;
        p->m_iNumObjectsDestroyed++;
    }

    setSolidType(SOLID_NOT);
    hideModel();

    takedamage = DAMAGE_NO;

    if (!(spawnflags & CRATE_NOTSTACKEDON)) {
        TellNeighborsToFall();
    }

    vCenter = origin + (maxs + mins) * 0.5f;

    // Tell clients around that a crate is destroyed
    gi.SetBroadcastVisible(origin, origin);
    gi.MSG_StartCGM(BG_MapCGMToProtocol(g_protocol, CGM_MAKE_CRATE_DEBRIS));
    gi.MSG_WriteCoord(vCenter[0]);
    gi.MSG_WriteCoord(vCenter[1]);
    gi.MSG_WriteCoord(vCenter[2]);
    gi.MSG_WriteByte(m_iDebrisType);
    gi.MSG_EndCGM();

    health   = 0;
    deadflag = DEAD_DEAD;

    PostEvent(EV_Remove, 0);

    // Notify scripts
    Unregister(STRING_DEATH);
}

void CrateObject::CrateThink(Event *ev)
{
    // Shake the create if it's on fire

    if (m_vJitterAngles[0] || m_vJitterAngles[2]) {
        setAngles(Vector(m_vStartAngles[0] * 2.0f, m_vStartAngles[1], m_vStartAngles[2] * 2.0f));

        if (m_vJitterAngles[0] > 0.0f) {
            m_vJitterAngles[0] -= 1 / 3 * m_fJitterScale;

            if (m_vJitterAngles[0] > 0.0f) {
                m_vJitterAngles[0] = 0.0f;
            }
        } else if (m_vJitterAngles[0] < 0.0f) {
            m_vJitterAngles[0] += 1 / 3 * m_fJitterScale;

            if (m_vJitterAngles[0] < 0.0f) {
                m_vJitterAngles[0] = 0.0f;
            }
        }

        m_vJitterAngles[0] = -m_vJitterAngles[0];

        if (m_vJitterAngles[2] > 0.0f) {
            m_vJitterAngles[2] -= 1 / 3 * m_fJitterScale;

            if (m_vJitterAngles[2] > 0.0f) {
                m_vJitterAngles[2] = 0.0f;
            }
        } else if (m_vJitterAngles[2] < 0.0f) {
            m_vJitterAngles[2] += 1 / 3 * m_fJitterScale;

            if (m_vJitterAngles[2] < 0.0f) {
                m_vJitterAngles[2] = 0.0f;
            }
        }

        m_vJitterAngles[2] = -m_vJitterAngles[2];

        PostEvent(EV_Crate_Think, 0.05f);
    } else if (!VectorCompare(angles, m_vStartAngles)) {
        setAngles(m_vStartAngles);
    }
}
