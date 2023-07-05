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

// PlayerStart.cpp: Player start location entity declarations
//

#include "g_local.h"
#include "entity.h"
#include "trigger.h"
#include "playerstart.h"

/*****************************************************************************/
/*QUAKED info_player_start (0.75 0.75 0) (-16 -16 0) (16 16 96)

The normal starting point for a level.

"angle" - the direction the player should face
"thread" - the thread that should be called when spawned at this position

******************************************************************************/

Event EV_PlayerStart_EnableSpawn
(
	"enablespawn",
	EV_DEFAULT,
	NULL,
	NULL,
	"allows spawning from this spawnpoint"
);

Event EV_PlayerStart_DisableSpawn
(
	"disablespawn",
	EV_DEFAULT,
	NULL,
	NULL,
	"forbids spawning from this spawnpoint"
);

Event EV_PlayerStart_DeleteOnSpawn
(
	"deletespawn",
	EV_DEFAULT,
	NULL,
	NULL,
	"delete this spawnpoint when spawning from this spawnpoint"
);

Event EV_PlayerStart_KeepOnSpawn
(
	"keepspawn",
	EV_DEFAULT,
	NULL,
	NULL,
	"keep this spawnpoint when spawning from this spawnpoint"
);

CLASS_DECLARATION( SimpleArchivedEntity, PlayerStart, "info_player_start" )
{
	{ &EV_SetAngle,							&PlayerStart::SetAngle },
	{ &EV_PlayerStart_EnableSpawn,			&PlayerStart::EventEnableSpawn },
	{ &EV_PlayerStart_DisableSpawn,			&PlayerStart::EventDisableSpawn },
	{ &EV_PlayerStart_DeleteOnSpawn,		&PlayerStart::EventDeleteOnSpawn },
	{ &EV_PlayerStart_KeepOnSpawn,			&PlayerStart::EventKeepOnSpawn },
	{ NULL, NULL }
};

PlayerStart::PlayerStart()
{
	m_bForbidSpawns = false;
	m_bDeleteOnSpawn = false;
}

void PlayerStart::SetAngle
   (
   Event *ev
   )
{
	angles = Vector( 0, ev->GetFloat( 1 ), 0 );
}

void PlayerStart::EventEnableSpawn
	(
	Event *ev
	)
{
	m_bForbidSpawns = false;
}

void PlayerStart::EventDisableSpawn
	(
	Event *ev
	)
{
	m_bForbidSpawns = true;
}

void PlayerStart::EventDeleteOnSpawn
	(
	Event *ev
	)
{
	m_bDeleteOnSpawn = true;
}

void PlayerStart::EventKeepOnSpawn
	(
	Event *ev
	)
{
	m_bDeleteOnSpawn = false;
}

/*****************************************************************************/
/*  saved out by quaked in region mode

******************************************************************************/

CLASS_DECLARATION( PlayerStart, TestPlayerStart, "testplayerstart" )
	{
		{ NULL, NULL }
	};

/*****************************************************************************/
/*QUAKED info_player_deathmatch (0.75 0.75 1) (-16 -16 0) (16 16 96)

potential spawning position for deathmatch games

"angle" - the direction the player should face
"thread" - the thread that should be called when spawned at this position
"arena" - the arena that this start point is located in

******************************************************************************/

CLASS_DECLARATION( PlayerStart, PlayerDeathmatchStart, "info_player_deathmatch" )
	{
		{ NULL, NULL }
	};

/*****************************************************************************/
/*QUAKED info_player_allied (0.75 0.75 1) (-16 -16 0) (16 16 96)

potential spawning position for an allied player in a team games... ignored for FFA.

"angle" - the direction the player should face
"thread" - the thread that should be called when spawned at this position
"arena" - the arena that this start point is located in

******************************************************************************/

CLASS_DECLARATION( PlayerDeathmatchStart, PlayerAlliedDeathmatchStart, "info_player_allied" )
{
	{ NULL, NULL }
};

/*****************************************************************************/
/*QUAKED info_player_axis (0.75 0.75 1) (-16 -16 0) (16 16 96)

potential spawning position for an axis player in a team games... ignored for FFA.

"angle" - the direction the player should face
"thread" - the thread that should be called when spawned at this position
"arena" - the arena that this start point is located in

******************************************************************************/

CLASS_DECLARATION( PlayerDeathmatchStart, PlayerAxisDeathmatchStart, "info_player_axis" )
{
	{ NULL, NULL }
};

/*****************************************************************************/
/*QUAKED info_player_intermission (0.75 0.75 0) (-16 -16 0) (16 16 96)

viewing point in between deathmatch levels

******************************************************************************/

CLASS_DECLARATION( Camera, PlayerIntermission, "info_player_intermission" )
	{
		{ NULL, NULL }
	};

PlayerIntermission::PlayerIntermission
   (
   )

   {
   currentstate.watch.watchPath = false;
   }

