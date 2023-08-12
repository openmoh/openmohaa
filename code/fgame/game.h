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

// game.h: General Game Info

#pragma once

#include "level.h"
#include "listener.h"
#include "g_public.h"

class Game : public Listener
{
public:
	gclient_t		*clients;
	qboolean		autosaved;
	int				maxclients;
	int				maxentities;

public:
	CLASS_PROTOTYPE( Game );

	void Archive( Archiver &arc ) override;

	Game();
	~Game();
};

class SimpleArchivedEntity;

/*
* Functions prototypes
*/


/* G_BeginIntermission
*
* Change the current map to the specified map with the possibility to fade the screen with fadetime.
*/
void G_BeginIntermission(
	const char *map_name,
	INTTYPE_e transtype = TRANS_BSP,
	bool no_fade = false
);

/* G_BeginIntermission2
*
* Called when a restart/change was issued
*/
void G_BeginIntermission2( void );

/* G_ExitIntermission
*
* Exit the intermission screen.
*/
void G_ExitIntermission( void );

/* G_CheckIntermissionExit
*
* Checks for the intermission to exit.
*/
void G_CheckIntermissionExit( void );

/* G_ExitLevel
*
* Exits the level.
*/
void G_ExitLevel( void );

/* G_RestartLevelWithDelay
*
* Restarts the level.
*/
void G_RestartLevelWithDelay( float delaytime );

/* G_UpdateMatchEndTime
*
* Update the timelimit
*/
void G_UpdateMatchEndTime( void );

/* G_FadeSound
*
* Fades the sound out over the given time.
*/
void G_FadeSound(
	float time
);

/* G_FindArchivedClass
*
* Finds an entity of the specified class.
* Gets the next entity if necessary.
*/
SimpleArchivedEntity *G_FindArchivedClass(
	SimpleArchivedEntity *ent,
	const char *classname
);

/* G_FindClass
*
* Finds an entity of the specified class.
* Gets the next entity if necessary.
*/
Entity *G_FindClass(
	Entity *ent,
	const char *classname
);

/* G_FindTarget
*
* Find an entity with the specified targetname.
*/
SimpleEntity *G_FindTarget(
	SimpleEntity *ent,
	const char *name
);

/* G_FindRandomSimpleTarget
*
* Find a random simple entity with the specified targetname.
*/
SimpleEntity *G_FindRandomSimpleTarget(
	const char *name
);

/* G_FindRandomSimpleTarget
*
* Find a random entity with the specified targetname.
*/
Entity *G_FindRandomTarget(
	const char *name
);

/* G_GetClientNumber
*
* Gets the specified client number from an entity.
* Returns -1 on failure.
*/
int G_GetClientNumber(
	gentity_t * ent
);

/* G_GetClientNumber
*
* Gets the specified client number from an entity.
* Returns -1 on failure.
*/
int G_GetClientNumber(
	Entity * ent
);

/* G_GetEntity
*
* Gets the specified entity number from an entity.
* Returns -1 on failure.
*/
int G_GetEntity(
	Entity * entity
);

/* G_GetEntity
*
* Gets the specified entity
*/
Entity * G_GetEntity(
	int id
);

/* G_GetEntityByClient
*
* Gets the specified entity from a client number
*/
Entity * G_GetEntityByClient(
	int clientNum
);

/* G_GetEntityIndex
*
* Gets the specified index from an entity number
*/
int G_GetEntityIndex(
		int number
);

/* G_GetEntityIndex
*
* Gets the specified index from a gentity
*/
int G_GetEntityIndex(
		gentity_t * ent
);

/* G_GetGEntity
*
* Gets the specified gentity.
*/
gentity_t * G_GetGEntity(
	int id
);

/* G_StartCinematic
*
* Turns on cinematic.
*/
void G_StartCinematic( void );


/* G_StopCinematic
*
* Turns off cinematic.
*/
void G_StopCinematic( void );

/* G_TouchTriggers
*
* Makes the entity touch triggers
*/
void G_TouchTriggers(
	Entity *entity
);

extern Game game;
