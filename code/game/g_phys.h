/*
===========================================================================
Copyright (C) 2008 the OpenMoHAA team

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

// g_phys.h: Header for g_phys.cpp

#ifndef __G_PHYS_H__
#define __G_PHYS_H__

#include "g_local.h"

class Entity;

typedef enum
{
	STEPMOVE_OK,
	STEPMOVE_BLOCKED_BY_ENTITY,
	STEPMOVE_BLOCKED_BY_WORLD,
	STEPMOVE_BLOCKED_BY_WATER,
	STEPMOVE_BLOCKED_BY_FALL,
	STEPMOVE_BLOCKED_BY_DOOR,
	STEPMOVE_STUCK
} stepmoveresult_t;

// movetype values
typedef enum
{
	MOVETYPE_NONE,			// never moves
	MOVETYPE_STATIONARY, // never moves but does collide agains push objects
	MOVETYPE_NOCLIP,		// origin and angles change with no interaction
	MOVETYPE_PUSH,			// no clip to world, push on box contact
	MOVETYPE_STOP,			// no clip to world, stops on box contact
	MOVETYPE_WALK,			// gravity
	MOVETYPE_FLY,
	MOVETYPE_TOSS,			// gravity
	MOVETYPE_FLYMISSILE,	// extra size to monsters
	MOVETYPE_BOUNCE,
	MOVETYPE_SLIDE,
	MOVETYPE_GIB,
	MOVETYPE_VEHICLE,
	MOVETYPE_TURRET
} movetype_t;

typedef struct
{
	Entity  	*ent;
	Vector   localorigin;
	Vector   origin;
	Vector   localangles;
	Vector	angles;
	float		deltayaw;
} pushed_t;

extern pushed_t pushed[];
extern pushed_t *pushed_p;

void		G_RunEntity( Entity *ent );
void		G_Impact( Entity *e1, trace_t *trace );
qboolean	G_PushMove( Entity *pusher, Vector move, Vector amove );
void		G_CheckWater( Entity *ent );
Entity		*G_TestEntityPosition( Entity *ent, Vector vOrg );

#endif /* g_phys.h */
