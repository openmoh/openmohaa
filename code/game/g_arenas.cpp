/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
//
//
// g_arenas.c
//

#include "g_local.h"


gentity_t	*podium1;
gentity_t	*podium2;
gentity_t	*podium3;


/*
==================
UpdateTournamentInfo
==================
*/
void UpdateTournamentInfo( void ) {

}


static gentity_t *SpawnModelOnVictoryPad( gentity_t *pad, vec3_t offset, gentity_t *ent, int place ) {
	

	return 0;
}


static void CelebrateStop( gentity_t *player ) {
}


#define	TIMER_GESTURE	(34*66+50)
static void CelebrateStart( gentity_t *player ) {

}


static vec3_t	offsetFirst  = {0, 0, 74};
static vec3_t	offsetSecond = {-10, 60, 54};
static vec3_t	offsetThird  = {-19, -60, 45};

static void PodiumPlacementThink( gentity_t *podium ) {

}


static gentity_t *SpawnPodium( void ) {
return 0;
}


/*
==================
SpawnModelsOnVictoryPads
==================
*/
void SpawnModelsOnVictoryPads( void ) {

}


/*
===============
Svcmd_AbortPodium_f
===============
*/
void Svcmd_AbortPodium_f( void ) {
	
}
