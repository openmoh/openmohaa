#include "playerbot.h"
/*
===========================================================================
Copyright (C) 2024 the OpenMoHAA team

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
// playerbot_master.cpp: Multiplayer bot system.

#include "g_local.h"
#include "actor.h"
#include "playerbot.h"
#include "consoleevent.h"
#include "debuglines.h"
#include "scriptexception.h"
#include "vehicleturret.h"
#include "weaputils.h"

BotManager botManager;

CLASS_DECLARATION(Listener, BotControllerManager, NULL) {
    {NULL, NULL}
};

CLASS_DECLARATION(Listener, BotManager, NULL) {
    {NULL, NULL}
};

void BotManager::Init()
{
    botControllerManager.Init();
}

void BotManager::Cleanup()
{
    botControllerManager.Cleanup();
}

void BotManager::Frame()
{
    botControllerManager.ThinkControllers();
}

void BotManager::BroadcastEvent(Entity *originator, Vector origin, int iType, float radius)
{
    Sentient      *ent;
    Actor         *act;
    Vector         delta;
    str            name;
    float          r2;
    float          dist2;
    int            i;
    int            iNumSentients;
    int            iAreaNum;
    BotController *controller;

    if (radius <= 0.0f) {
        radius = G_AIEventRadius(iType);
    }

    assert(originator);

    r2 = Square(radius);

    const Container<BotController *>& controllers = getControllerManager().getControllers();
    for (i = 1; i <= controllers.NumObjects(); i++) {
        controller = controllers.ObjectAt(i);
        ent        = controller->getControlledEntity();
        if (!ent || ent == originator || ent->deadflag) {
            continue;
        }

        delta = origin - ent->centroid;

        // dot product returns length squared
        dist2 = Square(delta);

        if (originator) {
            iAreaNum = originator->edict->r.areanum;
        } else {
            iAreaNum = gi.AreaForPoint(origin);
        }

        if (dist2 > r2) {
            continue;
        }

        if (iAreaNum != ent->edict->r.areanum && !gi.AreasConnected(iAreaNum, ent->edict->r.areanum)) {
            continue;
        }

        controller->NoticeEvent(origin, iType, originator, dist2, r2);
    }
}

BotControllerManager& BotManager::getControllerManager()
{
    return botControllerManager;
}
