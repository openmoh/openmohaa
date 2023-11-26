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

// windows.cpp: Window class
//

#include "g_phys.h"
#include "windows.h"
#include "game.h"

Event EV_Window_Setup
(
    "_Windowsetup",
    EV_CODEONLY,
    NULL,
    NULL,
    "Does the post spawn setup of the Window"
);

Event EV_Window_SetDebris
(
    "debristype",
    EV_DEFAULT,
    "i",
    "type",
    "Sets the debris type of the Window"
);

CLASS_DECLARATION(Entity, WindowObject, "func_window") {
    {&EV_Window_Setup,     &WindowObject::WindowSetup     },
    {&EV_Damage,           &WindowObject::WindowDamaged   },
    {&EV_Killed,           &WindowObject::WindowKilled    },
    {&EV_Window_SetDebris, &WindowObject::WindowDebrisType},
    {NULL,                 NULL                           }
};

WindowObject::WindowObject()
{
    m_iDebrisType = WINDOW_GLASS;

    PostEvent(EV_Window_Setup, EV_POSTSPAWN);
}

void WindowObject::Archive(Archiver& arc)
{
    Entity::Archive(arc);

    arc.ArchiveInteger(&m_iDebrisType);
    arc.ArchiveString(&m_sBrokenModel);
}

void WindowObject::WindowSetup(Event *ev)
{
    Entity *pEnt;

    setSolidType(SOLID_BSP);

    if (Target().length()) {
        pEnt = (Entity *)G_FindTarget(NULL, Target());

        if (pEnt) {
            // set the broken model to the target model
            m_sBrokenModel = pEnt->model;

            pEnt->PostEvent(EV_Remove, 0);
        }
    }

    takedamage = DAMAGE_YES;

    if (health <= 0.1f) {
        health = 100.0f;
    }
}

void WindowObject::WindowDebrisType(Event *ev)
{
    m_iDebrisType = ev->GetInteger(1);
}

void WindowObject::WindowDamaged(Event *ev)
{
    int    iMeansOfDeath;
    int    iDamage;
    Vector vRight;
    Vector vForward;
    Vector vDir;
    Event *event;

    if (!takedamage) {
        return;
    }

    iDamage       = ev->GetInteger(2);
    iMeansOfDeath = ev->GetInteger(9);

    if (iMeansOfDeath == MOD_BASH) {
        // double damage for bash
        iDamage *= 2;
    } else if (iMeansOfDeath == MOD_EXPLOSION) {
        // instant break
        iDamage = health;
    }

    health -= iDamage;

    if (health <= 0.0f) {
        event = new Event(EV_Killed);

        event->AddEntity(ev->GetEntity(1));
        event->AddInteger(ev->GetInteger(2));
        event->AddEntity(ev->GetEntity(3));

        ProcessEvent(event);
    }
}

void WindowObject::WindowKilled(Event *ev)
{
    Vector vCenter;

    setSolidType(SOLID_NOT);

    hideModel();
    takedamage = DAMAGE_NO;

    vCenter = origin + mins + maxs;

    gi.SetBroadcastVisible(vCenter, vCenter);
    gi.MSG_StartCGM(BG_MapCGMToProtocol(g_protocol, CGM_MAKE_WINDOW_DEBRIS));
    gi.MSG_WriteCoord(vCenter[0]);
    gi.MSG_WriteCoord(vCenter[1]);
    gi.MSG_WriteCoord(vCenter[2]);
    gi.MSG_WriteByte(m_iDebrisType);
    gi.MSG_EndCGM();

    if (m_sBrokenModel.length()) {
        Entity *ent = new Entity;

        ent->takedamage = DAMAGE_NO;
        ent->setModel(m_sBrokenModel);

        ent->setMoveType(MOVETYPE_NONE);
        ent->setSolidType(SOLID_BSP);

        setModel(m_sBrokenModel);

        if (spawnflags & WINDOW_BROKEN_BLOCK) {
            ent->setContents(CONTENTS_MONSTERCLIP | CONTENTS_PLAYERCLIP);
        } else {
            ent->setContents(CONTENTS_SHOOTONLY);
        }

        ent->setOrigin(origin);
        ent->showModel();
    }

    deadflag = DEAD_DEAD;

    // remove the window
    PostEvent(EV_Remove, 0);
}
