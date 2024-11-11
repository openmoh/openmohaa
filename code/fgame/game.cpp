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

// game.cpp : General Game Info.

#include "game.h"
#include "scriptmaster.h"
#include "worldspawn.h"

#include "camera.h"
#include "entity.h"
#include "player.h"
#include "dm_manager.h"

Event EV_Game_Detail
(
    "skill",
    EV_DEFAULT,
    NULL,
    NULL,
    "game.skill",   
    EV_GETTER   
);

Event EV_Game_Skill
(
    "detail",
    EV_DEFAULT,
    NULL,
    NULL,
    "game.detail",
    EV_GETTER
);

Game game;

void Game::GetSkill(Event *ev)
{
    switch (skill->integer) {
    case 0:
    case 1:
        ev->AddString("easy");
        break;
    case 2:
        ev->AddString("hard");
        break;
    default:
        ev->AddString("unknown");
        break;
    }
}

void Game::GetDetail(Event *ev)
{
    ev->AddFloat(detail->value);
}

void Game::Init()
{
    clients     = NULL;
    autosaved   = qfalse;
    maxentities = 0;
    maxclients  = 0;
}

void Game::Archive(Archiver& arc)
{
    static cvar_t *g_maxplayerhealth = gi.Cvar_Get("g_maxplayerhealth", "250", 0);
    int            i;

    if (arc.Saving()) {
        Vars()->MakePrimitive();
    }

    Listener::Archive(arc);

    if (arc.Saving()) {
        float fTmp;
        int   iTmp;

        fTmp = skill->value;
        arc.ArchiveFloat(&fTmp);

        iTmp = g_maxplayerhealth->integer;
        arc.ArchiveInteger(&iTmp);
    } else {
        float fTmp;
        int   iTmp;

        arc.ArchiveFloat(&fTmp);
        gi.cvar_set("skill", va("%f", fTmp));
        arc.ArchiveInteger(&iTmp);
        gi.cvar_set("g_maxplayerhealth", va("%d", iTmp));
    }

    arc.ArchiveBoolean(&autosaved);
    arc.ArchiveInteger(&maxentities);
    arc.ArchiveInteger(&maxclients);

    if (arc.Loading()) {
        G_AllocGameData();
    }

    for (i = 0; i < maxclients; i++) {
        G_ArchiveClient(arc, &clients[i]);
    }
}

Game::Game()
{
    Init();
}

Game::~Game() {}

CLASS_DECLARATION(Listener, Game, NULL) {
    {&EV_Game_Skill,  &Game::GetSkill },
    {&EV_Game_Detail, &Game::GetDetail},
    {NULL,            NULL            }
};
