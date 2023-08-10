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
// g_utils.c -- misc utility functions for game module

#include "g_local.h"
#include "actor.h"
#include "player.h"
#include "playerbot.h"
#include "debuglines.h"
#include <tiki.h>

typedef struct {
  char oldShader[MAX_QPATH];
  char newShader[MAX_QPATH];
  float timeOffset;
} shaderRemap_t;

#define MAX_SHADER_REMAPS 128

int remapCount = 0;
shaderRemap_t remappedShaders[MAX_SHADER_REMAPS];

const char *means_of_death_strings[ MOD_TOTAL_NUMBER ] =
{
	"none",
	"suicide",
	"crush",
	"crush_every_frame",
	"telefrag",
	"lava",
	"slime",
	"falling",
	"last_self_inflicted",
	"explosion",
	"explodewall",
	"electric",
	"electric_water",
	"thrown_object",
	"grenade",
	"beam",
	"rocket",
	"impact",
	"bullet",
	"fast_bullet",
	"vehicle",
	"fire",
	"flashbang",
	"on_fire",
	"gib",
	"impale",
	"bash",
	"shotgun",
};

// FIXME: OLD Q3 CODE
#if 0

/*
=============
G_PickTarget

Selects a random entity from among the targets
=============
*/
#define MAXCHOICES	32

gentity_t *G_PickTarget (char *targetname)
{
	gentity_t	*ent = NULL;
	int		num_choices = 0;
	gentity_t	*choice[MAXCHOICES];

	if (!targetname)
	{
		G_Printf("G_PickTarget called with NULL targetname\n");
		return NULL;
	}

	while(1)
	{
		ent = G_Find (ent, FOFS(targetname), targetname);
		if (!ent)
			break;
		choice[num_choices++] = ent;
		if (num_choices == MAXCHOICES)
			break;
	}

	if (!num_choices)
	{
		G_Printf("G_PickTarget: target %s not found\n", targetname);
		return NULL;
	}

	return choice[rand() % num_choices];
}


/*
==============================
G_UseTargets

"activator" should be set to the entity that initiated the firing.

Search for (string)targetname in all entities that
match (string)self.target and call their .use function

==============================
*/
void G_UseTargets( gentity_t *ent, gentity_t *activator ) {
	gentity_t		*t;
	
	if ( !ent ) {
		return;
	}

	//if (ent->targetShaderName && ent->targetShaderNewName) {
	//	float f = level.time * 0.001;
	//	AddRemap(ent->targetShaderName, ent->targetShaderNewName, f);
	//	gi.SetConfigstring(CS_SHADERSTATE, BuildShaderStateConfig());
	//}

	if ( !ent->target ) {
		return;
	}

	t = NULL;
	while ( (t = G_Find (t, FOFS(targetname), ent->target)) != NULL ) {
		if ( t == ent ) {
			G_Printf ("WARNING: Entity used itself.\n");
		} else {
			if ( t->use ) {
				t->use (t, ent, activator);
			}
		}
		if ( !ent->inuse ) {
			G_Printf("entity was removed while using targets\n");
			return;
		}
	}
}

#endif

int MOD_NameToNum( const str &meansOfDeath )
{
	int i;
	
	for ( i = 0 ; i < MOD_TOTAL_NUMBER ; i++ )
	{
		if ( !meansOfDeath.icmp( means_of_death_strings[ i ] ) )
			return i;
	}
	
	gi.DPrintf( "Unknown means of death - %s\n", meansOfDeath.c_str() );
	return -1;
}

const char *MOD_NumToName( int meansOfDeath )
{
	if ( ( meansOfDeath > MOD_TOTAL_NUMBER ) || ( meansOfDeath < 0 ) )
	{
		gi.DPrintf( "Unknown means of death num - %d\n", meansOfDeath );
		return "";
	}

	return means_of_death_strings[ meansOfDeath ];
}

qboolean MOD_matches( int incoming_damage, int damage_type )
{
	if ( damage_type == -1 )
	{
		return true;
	}
	else
	{
		return incoming_damage == damage_type;
	}
}

void AddRemap(const char *oldShader, const char *newShader, float timeOffset) {
	int i;

	for (i = 0; i < remapCount; i++) {
		if (Q_stricmp(oldShader, remappedShaders[i].oldShader) == 0) {
			// found it, just update this one
			strcpy(remappedShaders[i].newShader,newShader);
			remappedShaders[i].timeOffset = timeOffset;
			return;
		}
	}
	if (remapCount < MAX_SHADER_REMAPS) {
		strcpy(remappedShaders[remapCount].newShader,newShader);
		strcpy(remappedShaders[remapCount].oldShader,oldShader);
		remappedShaders[remapCount].timeOffset = timeOffset;
		remapCount++;
	}
}

const char *BuildShaderStateConfig(void) {
	static char	buff[MAX_STRING_CHARS*4];
	char out[(MAX_QPATH * 2) + 5];
	int i;
  
	memset(buff, 0, MAX_STRING_CHARS);
	for (i = 0; i < remapCount; i++) {
		Com_sprintf(out, (MAX_QPATH * 2) + 5, "%s=%s:%5.2f@", remappedShaders[i].oldShader, remappedShaders[i].newShader, remappedShaders[i].timeOffset);
		Q_strcat( buff, sizeof( buff ), out);
	}
	return buff;
}

/*
=================
G_Random

Returns a number from 0<= num < 1

random()
=================
*/

float G_Random
	(
	void
	)
{
	return ( ( float )( rand() & 0x7fff ) ) / ( ( float )0x8000 );
}

/*
=================
G_Random

Returns a number from 0 <= num < n

random()
=================
*/

float G_Random
	(
	float n
	)
{
	return G_Random() * n;
}

/*
=================
G_CRandom

Returns a number from -1 <= num < 1

crandom()
=================
*/

float G_CRandom
	(
	void
	)
{
	return G_Random( 2 ) - 1;
}

/*
=================
G_CRandom

Returns a number from -n <= num < n

crandom()
=================
*/

float G_CRandom
	(
	float n
	)
{
	return G_CRandom() * n;
}

/*
=========================================================================

model / sound configstring indexes

=========================================================================
*/

/*
================
G_FindConfigstringIndex

================
*/
int G_FindConfigstringIndex( const char *name, int start, int max, qboolean create ) {
	int		i;
	char	*s;

	if ( !name || !name[0] ) {
		return 0;
	}

	for ( i=1 ; i<max ; i++ ) {
		s = gi.GetConfigstring( start + i );

		if ( !s || !s[0] ) {
			break;
		}
		if ( !strcmp( s, name ) ) {
			return i;
		}
	}

	if ( !create ) {
		return 0;
	}

	if ( i == max ) {
		G_Error( "G_FindConfigstringIndex: overflow" );
	}

	gi.SetConfigstring( start + i, name );

	return i;
}


int G_ModelIndex( const char *name ) {
	return G_FindConfigstringIndex( name, CS_MODELS, MAX_MODELS, qtrue );
}

int G_SoundIndex( const char *name ) {
	return G_FindConfigstringIndex( name, CS_SOUNDS, MAX_SOUNDS, qtrue );
}

int G_WeaponIndex( const char *name ) {
	return G_FindConfigstringIndex( name, CS_WEAPONS, MAX_WEAPONS, qtrue );
}

/*
===============
G_SetTrajectory

Sets the pos trajectory for a fixed position
===============
*/
void G_SetTrajectory
(
gentity_t *ent,
vec3_t org
)
{
	ent->s.pos.trTime = 0;
	VectorClear( ent->s.pos.trDelta );

	VectorCopy( org, ent->s.origin );
}

/*
===============
G_SetConstantLight

Sets the encoded constant light parameter for entities
===============
*/
void G_SetConstantLight
(
int * constantlight,
float * red,
float * green,
float * blue,
float * radius,
int   * lightStyle
)
{
	int ir, ig, ib, iradius;

	if( !constantlight )
		return;

	ir = ( *constantlight ) & 255;
	ig = ( ( *constantlight ) >> 8 ) & 255;
	ib = ( ( *constantlight ) >> 16 ) & 255;
	iradius = ( ( *constantlight ) >> 24 ) & 255;

	if( red )
	{
		ir = *red * 255;
		if( ir > 255 )
			ir = 255;
	}

	if( green )
	{
		ig = *green * 255;
		if( ig > 255 )
			ig = 255;
	}

	if( blue )
	{
		ib = *blue * 255;
		if( ib > 255 )
			ib = 255;
	}

	if( radius )
	{
		iradius = *radius / CONSTANTLIGHT_RADIUS_SCALE;
		if( iradius > 255 )
			iradius = 255;
	}

	if( lightStyle )
	{
		ir = *lightStyle;
		if( ir > 255 )
			ir = 255;
	}
	*constantlight = ( ir )+( ig << 8 ) + ( ib << 16 ) + ( iradius << 24 );
}

//=====================================================================


/*
================
G_TeamCommand

Broadcasts a command to only a specific team
================
*/
void G_TeamCommand( teamtype_t team, char *cmd ) {
	int		i;

	for ( i = 0 ; i < game.maxclients ; i++ )
	{
		if ( game.clients[i].pers.enterTime != 0.0f )
		{
			if ( game.clients[i].pers.teamnum == team ) {
				gi.SendServerCommand( i, va("%s", cmd ));
			}
		}
	}
}

/*
=============
M_CheckBottom

Returns false if any part of the bottom of the entity is off an edge that
is not a staircase.

=============
*/
int c_yes, c_no;

qboolean M_CheckBottom( Entity *ent )
{
	Vector	mins, maxs, start, stop;
	trace_t	trace;
	int		x, y;
	float		mid, bottom;

	mins = ent->origin + ent->mins * 0.5;
	maxs = ent->origin + ent->maxs * 0.5;

	// if all of the points under the corners are solid world, don't bother
	// with the tougher checks
	// the corners must be within 16 of the midpoint
	start[ 2 ] = mins[ 2 ] - 1;

	for( x = 0; x <= 1; x++ )
	{
		for( y = 0; y <= 1; y++ )
		{
			start[ 0 ] = x ? maxs[ 0 ] : mins[ 0 ];
			start[ 1 ] = y ? maxs[ 1 ] : mins[ 1 ];
			if( gi.PointContents( start, 0 ) != CONTENTS_SOLID )
			{
				goto realcheck;
			}
		}
	}

	c_yes++;
	return true;		// we got out easy

realcheck:

	c_no++;

	//
	// check it for real...
	//
	start[ 2 ] = mins[ 2 ];

	// the midpoint must be within 16 of the bottom
	start[ 0 ] = stop[ 0 ] = ( mins[ 0 ] + maxs[ 0 ] ) * 0.5;
	start[ 1 ] = stop[ 1 ] = ( mins[ 1 ] + maxs[ 1 ] ) * 0.5;
	stop[ 2 ] = start[ 2 ] - 3 * STEPSIZE;//2 * STEPSIZE;

	trace = G_Trace( start, vec_zero, vec_zero, stop, ent, MASK_MONSTERSOLID, false, "M_CheckBottom 1" );

	if( trace.fraction == 1.0 )
	{
		return false;
	}

	mid = bottom = trace.endpos[ 2 ];

	// the corners must be within 16 of the midpoint
	for( x = 0; x <= 1; x++ )
	{
		for( y = 0; y <= 1; y++ )
		{
			start[ 0 ] = stop[ 0 ] = x ? maxs[ 0 ] : mins[ 0 ];
			start[ 1 ] = stop[ 1 ] = y ? maxs[ 1 ] : mins[ 1 ];

			trace = G_Trace( start, vec_zero, vec_zero, stop, ent, MASK_MONSTERSOLID, false, "M_CheckBottom 2" );

			if( trace.fraction != 1.0 && trace.endpos[ 2 ] > bottom )
			{
				bottom = trace.endpos[ 2 ];
			}

			if( trace.fraction == 1.0 || mid - trace.endpos[ 2 ] > STEPSIZE )
			{
				return false;
			}
		}
	}

	c_yes++;
	return true;
}

/*
=============
G_Find

Searches all active entities for the next one that holds
the matching string at fieldofs (use the FOFS() macro) in the structure.

Searches beginning at the entity after from, or the beginning if NULL
NULL will be returned if the end of the list is reached.

=============
*/
gentity_t *G_Find( gentity_t *from, int fieldofs, const char *match )
{
	char	*s;

	if( !from )
		from = g_entities;
	else
		from++;

	for( ; from < &g_entities[ globals.num_entities ]; from++ )
	{
		if( !from->inuse )
			continue;
		s = *( char ** )( ( byte * )from + fieldofs );
		if( !s )
			continue;
		if( !Q_stricmp( s, match ) )
			return from;
	}

	return NULL;
}

/*
=============
TempVector

This is just a convenience function
for making temporary vectors for function calls
=============
*/
float	*tv( float x, float y, float z ) {
	static	int		index;
	static	vec3_t	vecs[8];
	float	*v;

	// use an array so that multiple tempvectors won't collide
	// for a while
	v = vecs[index];
	index = (index + 1)&7;

	v[0] = x;
	v[1] = y;
	v[2] = z;

	return v;
}


/*
=============
VectorToString

This is just a convenience function
for printing vectors
=============
*/
char	*vtos( const vec3_t v ) {
	static	int		index;
	static	char	str[8][32];
	char	*s;

	// use an array so that multiple vtos won't collide
	s = str[index];
	index = (index + 1)&7;

	Com_sprintf (s, 32, "(%i %i %i)", (int)v[0], (int)v[1], (int)v[2]);

	return s;
}

Entity *G_NextEntity
(
Entity *ent
)
{
	gentity_t *from;

	if( !g_entities )
	{
		return NULL;
	}

	if( !ent )
	{
		from = g_entities;
	} else
	{
		from = ent->edict + 1;
	}

	if( !from )
	{
		return NULL;
	}

	for( ; from < &g_entities[ globals.num_entities ]; from++ )
	{
		if( !from->inuse || !from->entity )
		{
			continue;
		}

		return from->entity;
	}

	return NULL;
}

/*
===============
G_GetMovedir

QuakeEd only writes a single float for angles (bad idea), so up and down are
just constant angles.
===============
*/
Vector G_GetMovedir( float angle )
{
	if( angle == -1.0f )
	{
		return Vector( 0.0f, 0.0f, 1.0f );
	}
	else if( angle == -2.0f )
	{
		return Vector( 0.0f, 0.0f, -1.0f );
	}

	angle *= ( M_PI * 2.0f / 360.0f );
	return Vector( cos( angle ), sin( angle ), 0.0f );
}

/*
===============
G_SetMovedir

The editor only specifies a single value for angles (yaw),
but we have special constants to generate an up or down direction.
Angles will be cleared, because it is being used to represent a direction
instead of an orientation.
===============
*/
void G_SetMovedir( vec3_t angles, vec3_t movedir ) {
	static vec3_t VEC_UP		= {0, -1, 0};
	static vec3_t MOVEDIR_UP	= {0, 0, 1};
	static vec3_t VEC_DOWN		= {0, -2, 0};
	static vec3_t MOVEDIR_DOWN	= {0, 0, -1};

	if ( VectorCompare (angles, VEC_UP) ) {
		VectorCopy (MOVEDIR_UP, movedir);
	} else if ( VectorCompare (angles, VEC_DOWN) ) {
		VectorCopy (MOVEDIR_DOWN, movedir);
	} else {
		AngleVectors (angles, movedir, NULL, NULL);
	}
	VectorClear( angles );
}

float vectoyaw( const vec3_t vec ) {
	float	yaw;
	
	if (vec[YAW] == 0 && vec[PITCH] == 0) {
		yaw = 0;
	} else {
		//if (vec[PITCH]) {
			yaw = ( atan2( vec[YAW], vec[PITCH]) * 180 / M_PI );
		//} else if (vec[YAW] > 0) {
		//	yaw = 90;
		//} else {
		//	yaw = 270;
		//}
		if (yaw < 0) {
			yaw += 360;
		}
	}

	return yaw;
}

/*
=================
g_entitiesFree
=================
*/
qboolean g_entitiesFree( void ) {
	int			i;
	gentity_t	*e;

	e = &g_entities[MAX_CLIENTS];
	for ( i = MAX_CLIENTS; i < globals.num_entities; i++, e++) {
		if ( e->inuse ) {
			continue;
		}
		// slot available
		return qtrue;
	}
	return qfalse;
}


/*
=================
G_FreeEntity

Marks the entity as free
=================
*/
void G_FreeEntity( gentity_t *ed )
{
	gi.UnlinkEntity( ed ); // unlink from world

	memset( ed, 0, sizeof( *ed ) );

	ed->inuse = qfalse;
}

/*
==============================================================================

Kill box

==============================================================================
*/

/*
=================
G_KillBox

Kills all entities that would touch the proposed new positioning
of ent.  Ent should be unlinked before calling this!
=================
*/
void G_KillBox (gentity_t *ent) {
	int			i, num;
	int			touch[MAX_GENTITIES];
	gentity_t	*hit;
	vec3_t		mins, maxs;

	VectorAdd( ent->client->ps.origin, ent->r.mins, mins );
	VectorAdd( ent->client->ps.origin, ent->r.maxs, maxs );
	num = gi.AreaEntities( mins, maxs, touch, MAX_GENTITIES );

	for (i=0 ; i<num ; i++) {
		hit = &g_entities[touch[i]];
		if ( !hit->client ) {
			continue;
		}

		// nail it
		ent->entity->Damage( hit->entity, ent->entity, 100000, vec_zero, vec_zero,
			vec_zero, 0, DAMAGE_NO_PROTECTION, MOD_TELEFRAG );
	}

}

//==============================================================================

/*
=============
G_Sound
=============
*/
void G_Sound( gentity_t *ent, int channel, int soundIndex ) {

}


//==============================================================================


/*
=================
KillBox

Kills all entities that would touch the proposed new positioning
of ent.  Ent should be unlinked before calling this!
=================
*/
qboolean KillBox
(
Entity *ent
)
{
	int		   i;
	int	   	num;
	int         touch[ MAX_GENTITIES ];
	gentity_t   *hit;
	Vector      min;
	Vector      max;
	int         fail;

	fail = 0;

	min = ent->origin + ent->mins;
	max = ent->origin + ent->maxs;

	num = gi.AreaEntities( min, max, touch, MAX_GENTITIES );

	for( i = 0; i < num; i++ )
	{
		hit = &g_entities[ touch[ i ] ];

		if( !hit->inuse || ( hit->entity == ent ) || !hit->entity || ( hit->entity == world ) || ( !hit->entity->edict->solid ) )
		{
			continue;
		}

		hit->entity->Damage( ent, ent, hit->entity->health + 100000, ent->origin, vec_zero, vec_zero,
			0, DAMAGE_NO_PROTECTION, MOD_TELEFRAG );

		//
		// if we didn't kill it, fail
		//
		if( hit->entity->getSolidType() != SOLID_NOT )
		{
			fail++;
		}
	}

	//
	// all clear
	//
	return !fail;
}


/*
============
G_TouchSolids

Call after linking a new trigger in during gameplay
to force all entities it covers to immediately touch it
============
*/
void G_TouchSolids
(
Entity *ent
)
{
	int		i;
	int		num;
	int      touch[ MAX_GENTITIES ];
	gentity_t  *hit;
	Event		*ev;

	num = gi.AreaEntities( ent->absmin, ent->absmax, touch, MAX_GENTITIES );

	// be careful, it is possible to have an entity in this
	// list removed before we get to it (killtriggered)
	for( i = 0; i < num; i++ )
	{
		hit = &g_entities[ touch[ i ] ];
		if( !hit->inuse )
		{
			continue;
		}

		assert( hit->entity );

		//FIXME
		// should we post the events so that we don't have to worry about any entities going away
		ev = new Event( EV_Touch );
		ev->AddEntity( ent );
		hit->entity->ProcessEvent( ev );
		hit->entity->Unregister( "touch" );
	}
}

void G_ShowSightTrace
	(
	gentity_t *passent1,
	gentity_t *passent2,
	const char *reason
	)
{
	str text;
	str pass1;
	str pass2;

	assert( reason );

	if( passent1 )
	{
		pass1 = va( "'%s'(%d)", passent1->entname, passent1->s.number );
	}
	else
	{
		pass1 = "NULL";
	}

	if( passent2 )
	{
		pass2 = va( "'%s'(%d)", passent2->entname, passent2->s.number );
	}
	else
	{
		pass2 = "NULL";
	}

	text = va( "%0.2f : Pass1 %s Pass2 %s : '%s'\n",
		level.time, pass1.c_str(), pass2.c_str(), reason ? reason : "" );

	if( sv_traceinfo->integer == 3 )
	{
		gi.DebugPrintf( text.c_str() );
	}
	else
	{
		gi.DPrintf( "%s", text.c_str() );
	}
}

void G_ShowTrace
	(
	trace_t *trace,
	gentity_t *passent,
	const char *reason
	)
{
	str text;
	str pass;
	str hit;

	assert( reason );
	assert( trace );

	if( passent )
	{
		pass = va( "'%s'(%d)", passent->entname, passent->s.number );
	}
	else
	{
		pass = "NULL";
	}

	if( trace->ent )
	{
		hit = va( "'%s'(%d)", trace->ent->entname, trace->ent->s.number );
	}
	else
	{
		hit = "NULL";
	}

	text = va( "%0.2f : Pass %s Frac %f Hit %s : '%s'\n",
		level.time, pass.c_str(), trace->fraction, hit.c_str(), reason ? reason : "" );

	if( sv_traceinfo->integer == 3 )
	{
		gi.DebugPrintf( text.c_str() );
	}
	else
	{
		gi.DPrintf( "%s", text.c_str() );
	}
}

void G_CalcBoundsOfMove
(
Vector &start,
Vector &end,
Vector &mins,
Vector &maxs,
Vector *minbounds,
Vector *maxbounds
)
{
	Vector bmin;
	Vector bmax;

	ClearBounds( bmin, bmax );
	AddPointToBounds( start, bmin, bmax );
	AddPointToBounds( end, bmin, bmax );
	bmin += mins;
	bmax += maxs;

	if( minbounds )
	{
		*minbounds = bmin;
	}

	if( maxbounds )
	{
		*maxbounds = bmax;
	}
}

bool G_SightTrace
(
const Vector &start,
const Vector &mins,
const Vector &maxs,
const Vector &end,
gentity_t *passent,
gentity_t *passent2,
int contentmask,
qboolean cylinder,
const char *reason
)
{
	int entnum, entnum2;
	bool result;

	assert( reason );

	if( passent == NULL )
	{
		entnum = ENTITYNUM_NONE;
	}
	else
	{
		entnum = passent->s.number;
	}

	if( passent2 == NULL )
	{
		entnum2 = ENTITYNUM_NONE;
	}
	else
	{
		entnum2 = passent2->s.number;
	}

	result = gi.SightTrace( start, mins, maxs, end, entnum, entnum2, contentmask, cylinder ) ? true : false;

	if( sv_traceinfo->integer > 1 )
	{
		G_ShowSightTrace( passent, passent2, reason );
	}

	sv_numtraces++;

	if( sv_drawtrace->integer )
	{
		G_DebugLine( start, end, 1, 1, 0, 1 );
	}

	return result;
}

bool G_SightTrace
	(
	const Vector &start,
	const Vector &mins,
	const Vector &maxs,
	const Vector &end,
	Entity *passent,
	Entity *passent2,
	int contentmask,
	qboolean cylinder,
	const char *reason
	)
{
	gentity_t *ent, *ent2;
	int entnum, entnum2;
	bool result;

	assert( reason );

	if( passent == NULL || !passent->isSubclassOf( Entity ) )
	{
		ent = NULL;
		entnum = ENTITYNUM_NONE;
	}
	else
	{
		ent = passent->edict;
		entnum = ent->s.number;
	}

	if( passent2 == NULL || !passent2->isSubclassOf( Entity ) )
	{
		ent2 = NULL;
		entnum2 = ENTITYNUM_NONE;
	}
	else
	{
		ent2 = passent2->edict;
		entnum2 = ent2->s.number;
	}

	result = gi.SightTrace( start, mins, maxs, end, entnum, entnum2, contentmask, cylinder ) ? true : false;

	if( sv_traceinfo->integer > 1 )
	{
		G_ShowSightTrace( ent, ent2, reason );
	}

	sv_numtraces++;

	if( sv_drawtrace->integer )
	{
		G_DebugLine( start, end, 1, 1, 0, 1 );
	}

	return result;
}

trace_t G_Trace
	(
	vec3_t start,
	vec3_t mins,
	vec3_t maxs,
	vec3_t end,
	gentity_t *passent,
	int contentmask,
	qboolean cylinder,
	const char *reason,
	qboolean tracedeep
	)
{
	int entnum;
	trace_t trace;

	if( passent )
	{
		entnum = passent->s.number;
	}
	else
	{
		entnum = ENTITYNUM_NONE;
	}

	gi.Trace( &trace, start, mins, maxs, end, entnum, contentmask, cylinder, tracedeep );

	if( trace.entityNum == ENTITYNUM_NONE )
	{
		trace.ent = NULL;
	}
	else
	{
		trace.ent = &g_entities[ trace.entityNum ];
	}

	if( sv_traceinfo->integer > 1 )
	{
		G_ShowTrace( &trace, passent, reason );
	}

	sv_numtraces++;

	if( sv_drawtrace->integer )
	{
		G_DebugLine( Vector( start ), Vector( end ), 1, 1, 0, 1 );
	}

	return trace;
}

trace_t G_Trace
	(
	const Vector &start,
	const Vector &mins,
	const Vector &maxs,
	const Vector &end,
	const Entity *passent,
	int contentmask,
	qboolean cylinder,
	const char *reason,
	qboolean tracedeep
	)
{
	gentity_t *ent;
	int entnum;
	trace_t trace;

	assert( reason );

	if( passent == NULL || !passent->isSubclassOf( Entity ) )
	{
		ent = NULL;
		entnum = ENTITYNUM_NONE;
	}
	else
	{
		ent = passent->edict;
		entnum = ent->s.number;
	}

	gi.Trace( &trace, start, mins, maxs, end, entnum, contentmask, cylinder, tracedeep );

	if( trace.entityNum == ENTITYNUM_NONE )
	{
		trace.ent = NULL;
	}
	else
	{
		trace.ent = &g_entities[ trace.entityNum ];
	}

	if( sv_traceinfo->integer > 1 )
	{
		G_ShowTrace( &trace, ent, reason );
	}

	sv_numtraces++;

	if( sv_drawtrace->integer )
	{
		G_DebugLine( start, end, 1, 1, 0, 1 );
	}

	return trace;
}

void G_TraceEntities
	(
	Vector &start,
	Vector &mins,
	Vector &maxs,
	Vector &end,
	Container<Entity *>*victimlist,
	int contentmask,
	qboolean bIncludeTriggers
	)
{
	trace_t trace;
	vec3_t boxmins;
	vec3_t boxmaxs;
	int num;
	int touchlist[ MAX_GENTITIES ];
	gentity_t *touch;
	int i;


	// Find the bounding box

	for( i = 0; i<3; i++ )
	{
		if( end[ i ] > start[ i ] )
		{
			boxmins[ i ] = start[ i ] + mins[ i ] - 1;
			boxmaxs[ i ] = end[ i ] + maxs[ i ] + 1;
		}
		else
		{
			boxmins[ i ] = end[ i ] + mins[ i ] - 1;
			boxmaxs[ i ] = start[ i ] + maxs[ i ] + 1;
		}
	}

	// Find the list of entites

	num = gi.AreaEntities( boxmins, boxmaxs, touchlist, MAX_GENTITIES );

	for( i = 0; i<num; i++ )
	{
		touch = &g_entities[ touchlist[ i ] ];

		// see if we should ignore this entity
		if( touch->solid == SOLID_NOT )
			continue;
		if( touch->solid == SOLID_TRIGGER && !bIncludeTriggers )
			continue;

		gi.ClipToEntity( &trace, start, mins, maxs, end, touchlist[ i ], contentmask );

		if( trace.entityNum == touchlist[ i ] )
			victimlist->AddObject( touch->entity );
	}
}

float PlayersRangeFromSpot
	(
	Entity *ent
	)
{
	float bestplayerdistance = 9999999.0f;
	Vector vDist;
	int i;
	gentity_t *e;
	Player *player;

	if( g_gametype->integer > GT_FFA ) {
		return bestplayerdistance;
	}

	// find the nearest player from the post
	for( i = 0, e = g_entities; i < game.maxclients; i++, e++ )
	{
		if( !e->client || !e->entity ) {
			continue;
		}

		player = ( Player * )e->entity;
		if( !player->IsDead() && !player->IsSpectator() )
		{
			vDist = ent->origin - player->origin;
			if( bestplayerdistance > vDist.length() )
				bestplayerdistance = vDist.length();
		}
	}

	return bestplayerdistance;
}

/*
================
G_SetOrigin

Sets the pos trajectory for a fixed position
================
*/
void G_SetOrigin( gentity_t *ent, vec3_t origin ) {
	VectorCopy( origin, ent->s.origin );
	ent->s.pos.trTime = 0;
	VectorClear( ent->s.pos.trDelta );

	VectorCopy( origin, ent->r.currentOrigin );
}

/*
=================
findradius

Returns entities that have origins within a spherical area

findradius (org, radius)
=================
*/
Entity *findradius
(
Entity *startent,
Vector org,
float rad
)
{
	Vector		eorg;
	gentity_t	*from;
	float			r2, distance;

	if( !startent )
	{
		from = active_edicts.next;
	}
	else
	{
		from = startent->edict->next;
	}

	assert( from );
	if( !from )
	{
		return NULL;
	}

	assert( ( from == &active_edicts ) || ( from->inuse ) );

	// square the radius so that we don't have to do a square root
	r2 = rad * rad;

	for( ; from != &active_edicts; from = from->next )
	{
		assert( from->inuse );
		assert( from->entity );

		eorg = org - from->entity->centroid;

		// dot product returns length squared
		distance = eorg * eorg;

		if( distance <= r2 )
		{
			return from->entity;
		}
		else
		{
			// subtract the object's own radius from this distance
			distance -= from->radius2;
			if( distance <= r2 )
			{
				return from->entity;
			}
		}
	}

	return NULL;
}

/*
=================
findclientinradius

Returns clients that have origins within a spherical area

findclientinradius (org, radius)
=================
*/
Entity *findclientsinradius
(
Entity *startent,
Vector org,
float rad
)
{
	Vector	 eorg;
	gentity_t *ed;
	float		 r2;
	int       i;

	// square the radius so that we don't have to do a square root
	r2 = rad * rad;

	if( !startent )
	{
		i = 0;
	} else
	{
		i = startent->entnum + 1;
	}

	for( ; i < game.maxclients; i++ )
	{
		ed = &g_entities[ i ];

		if( !ed->inuse || !ed->entity )
		{
			continue;
		}

		eorg = org - ed->entity->centroid;

		// dot product returns length squared
		if( ( eorg * eorg ) <= r2 )
		{
			return ed->entity;
		}
	}

	return NULL;
}

Vector G_CalculateImpulse( const Vector &start, const Vector &end, float speed, float gravity )
{
	float traveltime, vertical_speed;
	Vector dir, xydir, velocity;

	dir = end - start;
	xydir = dir;
	xydir.z = 0;
	traveltime = xydir.length() / speed;
	vertical_speed = ( dir.z / traveltime ) + ( 0.5f * gravity * sv_gravity->value * traveltime );
	xydir.normalize();

	velocity = speed * xydir;
	velocity.z = vertical_speed;
	return velocity;
}

/*
================
DebugLine

  debug polygons only work when running a local game
  with r_debugSurface set to 2
================
*/
int DebugLine(vec3_t start, vec3_t end, int color) {
	vec3_t points[4], dir, cross, up = {0, 0, 1};
	float dot;

	VectorCopy(start, points[0]);
	VectorCopy(start, points[1]);
	//points[1][2] -= 2;
	VectorCopy(end, points[2]);
	//points[2][2] -= 2;
	VectorCopy(end, points[3]);


	VectorSubtract(end, start, dir);
	VectorNormalize(dir);
	dot = DotProduct(dir, up);
	if (dot > 0.99 || dot < -0.99) VectorSet(cross, 1, 0, 0);
	else CrossProduct(dir, up, cross);

	VectorNormalize(cross);

	VectorMA(points[0], 2, cross, points[0]);
	VectorMA(points[1], -2, cross, points[1]);
	VectorMA(points[2], -2, cross, points[2]);
	VectorMA(points[3], 2, cross, points[3]);

	//return gi.DebugPolygonCreate(color, 4, points);
	// FIXME
	return 0;
}

void G_DebugDamage
	(
	float damage,
	Entity *victim,
	Entity *attacker,
	Entity *inflictor
	)
{
	gi.DPrintf( "Victim:%s Attacker:%s Inflictor:%s Damage:%f\n", victim->getClassname(), attacker->getClassname(), inflictor->getClassname(), damage );
}


void G_DebugString
	(
	Vector pos,
	float scale,
	float r,
	float g,
	float b,
	const char *pszText,
	...
	)
{
	debugstring_t *string;
	va_list va;
	char szTemp[ 32768 ];

	if( !g_numdebugstrings ) {
		return;
	}

	if( *gi.numDebugStrings < g_numdebugstrings->integer )
	{
		string = ( debugstring_t * )&DebugStrings[ *gi.numDebugStrings ];

		( *gi.numDebugStrings )++;

		va_start( va, pszText );
		vsprintf( szTemp, pszText, va );
		va_end( va );

		VectorCopy( pos, string->pos );
		string->scale = scale;
		string->color[ 0 ] = r;
		string->color[ 1 ] = g;
		string->color[ 2 ] = b;
		string->color[ 3 ] = 1.0f;
		strncpy( string->szText, szTemp, sizeof( string->szText ) );
		string->szText[ sizeof( string->szText ) - 1 ] = 0;
	}
	else
	{
		gi.DPrintf( "G_DebugString: Exceeded g_numdebugstrings\n" );
	}
}

void G_FadeOut
	(
	float delaytime
	)
{
	// Fade the screen out
	level.m_fade_color = Vector( 0, 0, 0 );
	level.m_fade_alpha = 1.0f;
	level.m_fade_time = delaytime;
	level.m_fade_time_start = delaytime;
	level.m_fade_type = fadeout;
	level.m_fade_style = alphablend;
}

void G_AutoFadeIn
(
void
)
{
	level.m_fade_time_start = 1;
	level.m_fade_time = 1;
	level.m_fade_color[ 0 ] = 0;
	level.m_fade_color[ 1 ] = 0;
	level.m_fade_color[ 2 ] = 0;
	level.m_fade_alpha = 1;
	level.m_fade_type = fadein;
	level.m_fade_style = alphablend;
}

void G_ClearFade
(
void
)
{
	level.m_fade_time = -1;
	level.m_fade_type = fadein;
}

void G_FadeSound
(
float delaytime
)
{
	float time;

	// Fade the screen out
	time = delaytime * 1000;
	gi.SendServerCommand( 0, va( "fadesound %0.2f", time ) );
}

void G_MissionFailed
(
void
)
{
	// Make the music system play the failure music for this level
	ChangeMusic( "failure", "normal", true );

	//G_PlayerDied( 3 );

	// tell the player they f'd up
	gi.centerprintf( &g_entities[ 0 ], "@textures/menu/mission.tga" );

	level.mission_failed = true;
}

void G_StartCinematic
(
void
)
{
	level.cinematic = true;
	gi.Cvar_Set( "sv_cinematic", "1" );
}

void G_StopCinematic
	(
	void
	)
{
	// clear out the skip thread
	level.cinematic = false;
	gi.Cvar_Set( "sv_cinematic", "0" );
}

void G_CenterPrintToAllClients( const char *pszString )
{
	gentity_t *ent;
	int i;

	for( i = 0, ent = g_entities; i < game.maxclients; ent++, i++ )
	{
		if( !ent->inuse || !ent->entity ) {
			continue;
		}

		gi.centerprintf( ent, va( "%s\n", pszString ) );
	}
}

void G_PrintToAllClients( const char *pszString, qboolean bBold )
{
	if( bBold )
	{
		gi.SendServerCommand( -1, "print \"" HUD_MESSAGE_WHITE "%s\n\"", pszString );
	}
	else
	{
		gi.SendServerCommand( -1, "print \"" HUD_MESSAGE_YELLOW "%s\n\"", pszString );
	}
}

void G_PrintDeathMessage( const char *pszString )
{
	gentity_t *ent;
	int i;

	for( i = 0, ent = g_entities; i < game.maxclients; i++, ent++ )
	{
		if( !ent->inuse || !ent->entity ) {
			continue;
		}

		gi.SendServerCommand( ent - g_entities, "print \"" HUD_MESSAGE_CHAT_RED "%s\"", pszString );
	}
}

void G_WarnPlayer( Player *player, const char *format, ... )
{
	char buffer[ 4100 ];
	va_list va;

	va_start( va, format );
	vsprintf( buffer, format, va );
	va_end( va );

	gi.SendServerCommand( player->client->ps.clientNum, "print \"%s\"\n", buffer );
	gi.SendServerCommand( player->client->ps.clientNum, "print \"%c%s\"", HUD_MESSAGE_YELLOW, buffer );
}

dtiki_t *modelTiki( const char *mdl )
{
	str name;

	assert( mdl );

	if( !mdl )
	{
		return 0;
	}

	// Prepend 'models/' to make things easier
	if( !strchr( mdl, '*' ) && !strchr( mdl, '\\' ) && !strchr( mdl, '/' ) )
	{
		name = "models/";
	}

	name += mdl;

	return gi.modeltiki( name.c_str() );
}

void G_ProcessCacheInitCommands( dtiki_t *tiki )
{
	dtikicmd_t *pcmd;

	if( tiki->a->num_server_initcmds )
	{
		int i, j;
		Event *event;

		for( i = 0; i < tiki->a->num_server_initcmds; i++ )
		{
			pcmd = &tiki->a->server_initcmds[ i ];
			event = new Event( pcmd->args[ 0 ] );

			if( Director.GetFlags( event ) & EV_CACHE )
			{
				for( j = 1; j < pcmd->num_args; j++ )
				{
					event->AddToken( pcmd->args[ j ] );
				}

				if( !Director.ProcessEvent( event ) )
				{
					Com_Printf( "^~^~^ Entity::G_ProcessCacheInitCommands: Bad init server command '%s' in '%s'\n", pcmd->args[ 0 ], tiki->name );
				}
			}
			else
			{
				delete event;
			}
		}
	}
}

void CacheResource( const char *stuff )
{
	AliasListNode_t *ret;
	qboolean streamed = qfalse;
	char filename[ MAX_STRING_TOKENS ];

	if( !stuff )
		return;

	if( !strchr( stuff, '.' ) )
	{
		// must be a global alias
		stuff = gi.GlobalAlias_FindRandom( stuff, &ret );
		if( !stuff )
		{
			if( gi.fsDebug->integer )
				Com_Printf( "alias not found\n" );

			return;
		}
		streamed = ret->streamed;
	}

	strcpy( filename, stuff );
	gi.FS_CanonicalFilename( filename );

	if( strstr( filename, ".wav" ) )
	{
		gi.soundindex( filename, streamed );
	}
	else if( strstr( filename, ".mp3" ) )
	{
		gi.soundindex( filename, streamed );
	}
	else if( strstr( filename, ".tik" ) )
	{
		dtiki_t *tiki;

		if( *stuff && strncmp( "models/", stuff, 7 ) )
		{
			sprintf( filename, "models/%s", stuff );
		}
		else
		{
			strcpy( filename, stuff );
		}

		gi.FS_CanonicalFilename( filename );

		tiki = gi.TIKI_RegisterModel( filename );

		if( tiki )
			G_ProcessCacheInitCommands( tiki );
	}
	else if( strstr( filename, ".scr" ) )
	{
		Director.GetScript( filename );
	}
}

void ChangeMusic( const char *current, const char *fallback, qboolean force )
{
	int      j;
	gentity_t  *other;

	if( current || fallback )
	{
		for( j = 0; j < game.maxclients; j++ )
		{
			other = &g_entities[ j ];
			if( other->inuse && other->client )
			{
				Player *client;

				client = ( Player * )other->entity;
				client->ChangeMusic( current, fallback, force );
			}
		}

		if( current && fallback )
		{
			gi.DPrintf( "music set to %s with fallback %s\n", current, fallback );
		}
	}
}

void ChangeMusicVolume( float volume, float fade_time )
{
	int      j;
	gentity_t  *other;

	for( j = 0; j < game.maxclients; j++ )
	{
		other = &g_entities[ j ];
		if( other->inuse && other->client )
		{
			Player *client;

			client = ( Player * )other->entity;
			client->ChangeMusicVolume( volume, fade_time );
		}
	}
	gi.DPrintf( "music volume set to %.2f, fade time %.2f\n", volume, fade_time );
}

void RestoreMusicVolume( float fade_time )
{
	int      j;
	gentity_t  *other;

	for( j = 0; j < game.maxclients; j++ )
	{
		other = &g_entities[ j ];
		if( other->inuse && other->client )
		{
			Player *client;

			client = ( Player * )other->entity;
			client->RestoreMusicVolume( fade_time );
		}
	}
}

void ChangeSoundtrack( const char * soundtrack )
{
	level.saved_soundtrack = level.current_soundtrack;
	level.current_soundtrack = soundtrack;

	gi.SetConfigstring( CS_MUSIC, soundtrack );
	gi.DPrintf( "soundtrack switched to %s.\n", soundtrack );
}

void RestoreSoundtrack( void )
{
	if( level.saved_soundtrack.length() )
	{
		level.current_soundtrack = level.saved_soundtrack;
		level.saved_soundtrack = "";

		gi.SetConfigstring( CS_MUSIC, level.current_soundtrack.c_str() );
		gi.DPrintf( "soundtrack restored %s.\n", level.current_soundtrack.c_str() );
	}
}

const char * G_AIEventStringFromType(int iType)
{
	switch (iType)
	{
	case AI_EVENT_WEAPON_FIRE:
		return "weapon_fire";
		break;
	case AI_EVENT_WEAPON_IMPACT:
		return "weapon_impact";
		break;
	case AI_EVENT_EXPLOSION:
		return "explosion";
		break;
	case AI_EVENT_AMERICAN_VOICE:
		return "american_voice";
		break;
	case AI_EVENT_GERMAN_VOICE:
		return "german_voice";
		break;
	case AI_EVENT_AMERICAN_URGENT:
		return "american_urgent";
		break;
	case AI_EVENT_GERMAN_URGENT:
		return "german_urgent";
		break;
	case AI_EVENT_MISC:
		return "misc";
		break;
	case AI_EVENT_MISC_LOUD:
		return "misc_loud";
		break;
	case AI_EVENT_FOOTSTEP:
		return "footstep";
		break;
	case AI_EVENT_GRENADE:
		return "grenade";
		break;
	default:
		return "????";
		break;
	}
}

int G_AIEventTypeFromString
	(
	const char *pszType
	)
{
	if( !Q_stricmp( pszType, "weapon_fire" ) )
		return AI_EVENT_WEAPON_FIRE;
	else if( !Q_stricmp( pszType, "weapon_impact" ) )
		return AI_EVENT_WEAPON_IMPACT;
	else if( !Q_stricmp( pszType, "explosion" ) )
		return AI_EVENT_EXPLOSION;
	else if( !Q_stricmp( pszType, "american_voice" ) )
		return AI_EVENT_AMERICAN_VOICE;
	else if( !Q_stricmp( pszType, "german_voice" ) )
		return AI_EVENT_GERMAN_VOICE;
	else if( !Q_stricmp( pszType, "american_urgent" ) )
		return AI_EVENT_AMERICAN_URGENT;
	else if( !Q_stricmp( pszType, "german_urgent" ) )
		return AI_EVENT_GERMAN_URGENT;
	else if( !Q_stricmp( pszType, "misc" ) )
		return AI_EVENT_MISC;
	else if( !Q_stricmp( pszType, "misc_loud" ) )
		return AI_EVENT_MISC_LOUD;
	else if( !Q_stricmp( pszType, "footstep" ) )
		return AI_EVENT_FOOTSTEP;
	else if( !Q_stricmp( pszType, "grenade" ) )
		return AI_EVENT_GRENADE;
	else
		return AI_EVENT_NONE;
}

float G_AIEventRadius
	(
	int iType
	)
{
	static float fRadius[] =
	{
		2048.0f,
		384.0f,
		4096.0f,
		1024.0f,
		1024.0f,
		1536.0f,
		1536.0f,
		1500.0f,
		2250.0f,
		512.0f,
		384.0f,
		0,
		0,
		0,
		0
	};

	if( iType <= AI_EVENT_GRENADE)
	{
		return fRadius[ iType ];
	}
	else
	{
		Com_Printf( "G_AIEventRadius: invalid event type\n" );
		return 1500.0f;
	}
}

void G_BroadcastAIEvent
	(
	Entity *originator,
	Vector origin,
	char *pszType
	)
{
	G_BroadcastAIEvent( originator, origin, G_AIEventTypeFromString( pszType ), -1.0f );
}

void G_BroadcastAIEvent
	(
	Entity *originator,
	Vector origin,
	int iType,
	float radius
	)
{
	Sentient *ent;
	Vector delta;
	str name;
	float r2;
	float dist2;
	int i;
	int iNumSentients;
	int iAreaNum;
	
	if( iType < AI_EVENT_FOOTSTEP)
	{
		ent = ( Sentient * )G_GetEntity( 0 );

		if( ent && ent->m_bIsDisguised )
			return;
	}

	if( radius <= 0.0f )
		radius = G_AIEventRadius( iType );

	assert( originator );
	if( originator && !( originator->flags & FL_NOTARGET ) )
	{
		r2 = Square(radius);
		iNumSentients = SentientList.NumObjects();
		for( i = 1; i <= iNumSentients; i++ )
		{
			ent = SentientList.ObjectAt( i );
			if( ( ent == originator ) || ent->deadflag )
			{
				continue;
			}

			delta = origin - ent->centroid;

			// dot product returns length squared
			dist2 = Square(delta);

			if( originator )
				iAreaNum = originator->edict->r.areanum;
			else
				iAreaNum = gi.AreaForPoint( origin );

			if (
					( dist2 <= r2 ) &&
					(
						( iAreaNum == ent->edict->r.areanum ) ||
						( gi.AreasConnected( iAreaNum, ent->edict->r.areanum ) )
					)
				)

			{
				if( ent->IsSubclassOfActor() )
				{
					Actor *act = ( Actor * )ent;

					if( !act->IgnoreSound( iType ) )
					{
						act->ReceiveAIEvent( origin, iType, originator, dist2, r2 );
					}
				}
				else if( ent->IsSubclassOfBot() )
				{
					PlayerBot *bot = ( PlayerBot * )ent;

					bot->NoticeEvent( origin, iType, originator, dist2, r2 );
				}
			}
		}

#if 0
		gi.DPrintf( "Broadcast event %s to %d entities\n", ev->getName(), count );
#endif
	}
}

void CloneEntity
	(
	Entity *dest,
	Entity *src
	)
{
	int     i, num;

#if 0
	dest->setModel( src->model );
	// don't process our init commands
	//dest->CancelEventsOfType( EV_ProcessInitCommands );
	dest->setOrigin( src->origin );
	dest->setAngles( src->angles );
#endif
	dest->setScale( src->edict->s.scale );
	dest->setAlpha( src->edict->s.alpha );
	dest->health = src->health;
	// copy the surfaces
	memcpy( dest->edict->s.surfaces, src->edict->s.surfaces, sizeof( src->edict->s.surfaces ) );
	dest->edict->s.constantLight = src->edict->s.constantLight;
	//dest->edict->s.eFlags = src->edict->s.eFlags;
	dest->edict->s.renderfx = src->edict->s.renderfx;

	num = src->numchildren;
	for( i = 0; ( i < MAX_MODEL_CHILDREN ) && num; i++ )
	{
		Entity * clone;
		Entity * child;

		// duplicate the children
		if( !src->children[ i ] )
		{
			continue;
		}
		child = G_GetEntity( src->children[ i ] );
		if( child )
		{
			clone = new Animate;
			CloneEntity( clone, child );
			clone->attach( dest->entnum, child->edict->s.tag_num );
		}
		num--;
	}
}

weaponhand_t WeaponHandNameToNum
	(
	str side
	)
{
	if( !side.length() )
	{
		gi.DPrintf( "WeaponHandNameToNum : Weapon hand not specified\n" );
		return WEAPON_ERROR;
	}

	if( !side.icmp( "mainhand" ) || !side.icmp( "main" ) )
	{
		return WEAPON_MAIN;
	}
	else if( !side.icmp( "offhand" ) || !side.icmp( "off" ) )
	{
		return WEAPON_OFFHAND;
	}
	else
	{
		return ( weaponhand_t )atoi( side );
	}
}

const char *WeaponHandNumToName
(
weaponhand_t hand
)
{
	switch( hand )
	{
	case WEAPON_MAIN:
		return "mainhand";
	case WEAPON_OFFHAND:
		return "offhand";
	default:
		return "Invalid Hand";
	}
}

firemode_t WeaponModeNameToNum
(
str mode
)
{
	if( !mode.length() )
	{
		gi.DPrintf( "WeaponModeNameToNum : Weapon mode not specified\n" );
		assert( 0 );
		return FIRE_ERROR;
	}

	if( !mode.icmp( "primary" ) )
	{
		return FIRE_PRIMARY;
	}

	if( !mode.icmp( "secondary" ) )
	{
		return FIRE_SECONDARY;
	}
	else
	{
		return ( firemode_t )atoi( mode );
	}
}

// Used to tell the server about the edict pose, such as the player pose
// so that G_Trace with tracedeep will set the location
void G_UpdatePoseInternal( gentity_t *edict )
{
	if( edict->s.number == ENTITYNUM_NONE ||
		level.frame_skel_index != level.skel_index[ edict->s.number ] )
	{
		gi.TIKI_SetPoseInternal( edict->tiki, edict->s.number, edict->s.frameInfo, edict->s.bone_tag, edict->s.bone_quat, edict->s.actionWeight );
	}
}

orientation_t G_TIKI_Orientation( gentity_t *edict, int num )
{
	orientation_t orient;

	G_UpdatePoseInternal( edict );

	orient = gi.TIKI_OrientationInternal( edict->tiki, edict->s.number, num, edict->s.scale );

	return orient;
}

SkelMat4 *G_TIKI_Transform( gentity_t *edict, int num )
{
	G_UpdatePoseInternal( edict );
	return ( SkelMat4 *)gi.TIKI_TransformInternal( edict->tiki, edict->s.number, num );
}

qboolean G_TIKI_IsOnGround( gentity_t *edict, int num, float threshold )
{
	G_UpdatePoseInternal( edict );
	return gi.TIKI_IsOnGroundInternal( edict->tiki, edict->s.number, num, threshold );
}

char *G_TimeString( float fTime )
{
	float fTmp;
	static char szTime[ 32 ];

	fTmp = fTime / 3600.0f;

	if( fTmp >= 1.0f )
	{
		Com_sprintf( szTime, sizeof( szTime ), "%i:%02i:%02i",
			( int )( fTmp ),
			( int )( fmod( fTime / 60.0f, 60.0f ) ),
			( int )( fmod( fTime, 60.0f ) )
			);
	}
	else
	{
		Com_sprintf( szTime, sizeof( szTime ), "%i:%02i",
			( int )( fTime / 60.0f ),
			( int )( fmod( fTime, 60.0f ) )
			);
	}

	return szTime;
}

char *CanonicalTikiName( const char *szInName )
{
	static char filename[ 1024 ];

	if( *szInName && Q_stricmpn( "models/", szInName, 7 ) )
	{
		sprintf( filename, "models/%s", szInName );
	}
	else
	{
		strcpy( filename, szInName );
	}

	gi.FS_CanonicalFilename( filename );
	return filename;
}

qboolean G_Command_ProcessFile( const char * filename, qboolean quiet )
{
    char* buffer;
    const char* bufstart;
    const char* token;
	int numTokens = 0;

    if (gi.FS_ReadFile(filename, (void**)&buffer, quiet) == -1)
    {
        return qfalse;
    }

    if (!quiet)
        gi.DPrintf("G_Command_ProcessFile: %s\n", filename);

    bufstart = buffer;

    while (1)
    {
        // grab each line as we go
        token = COM_ParseExt(&buffer, qtrue);
        if (!token[0]) {
            break;
        }

        if (
            !Q_stricmp(token, "end") ||
            !Q_stricmp(token, "server")
            )
        {
            // skip the line
            while (1)
            {
                token = COM_ParseExt(&buffer, qfalse);
                if (!token[0]) {
                    break;
                }
            }
            continue;
        }

        // Create the event
		Event ev(token);

        // get the rest of the line
        while (1) {
            token = COM_ParseExt(&buffer, qfalse);
            if (!token[0]) {
                break;
            }
			
            ev.AddToken(token);
        }

        Director.ProcessEvent(ev);
    }

    gi.FS_FreeFile((void*)bufstart);

    return qtrue;
}

/*
==============
G_ArchiveEdict
==============
*/
void G_ArchiveEdict
(
Archiver &arc,
gentity_t *edict
)
{
	int    i;
	str    tempStr;

	assert( edict );

	//
	// this is written funny because it is used for both saving and loading
	//

	if( edict->client )
	{
		arc.ArchiveRaw( edict->client, sizeof( *edict->client ) );
	}

	arc.ArchiveInteger( &edict->s.beam_entnum );

	for( i = 0; i < MAX_FRAMEINFOS; i++ )
	{
		arc.ArchiveInteger( &edict->s.frameInfo[ i ].index );
		arc.ArchiveFloat( &edict->s.frameInfo[ i ].time );
		arc.ArchiveFloat( &edict->s.frameInfo[ i ].weight );
	}

	arc.ArchiveFloat( &edict->s.actionWeight );

	arc.ArchiveFloat( &edict->s.shader_data[ 0 ] );
	arc.ArchiveFloat( &edict->s.shader_data[ 1 ] );
	arc.ArchiveFloat( &edict->s.shader_time );

	arc.ArchiveVec3( edict->s.eyeVector );

	arc.ArchiveInteger( &edict->s.eType );
	arc.ArchiveInteger( &edict->s.eFlags );

	arc.ArchiveVec3( edict->s.netorigin );
	arc.ArchiveVec3( edict->s.origin );
	arc.ArchiveVec3( edict->s.origin2 );
	arc.ArchiveVec3( edict->s.netangles );
	arc.ArchiveVec3( edict->s.angles );

	arc.ArchiveInteger( &edict->s.constantLight );

	if( arc.Saving() )
	{
		if( edict->s.loopSound )
			tempStr = gi.GetConfigstring( CS_SOUNDS + edict->s.loopSound );
		else
			tempStr = "";

		arc.ArchiveString( &tempStr );
	}
	else
	{
		arc.ArchiveString( &tempStr );

		if( tempStr.length() )
			edict->s.loopSound = gi.soundindex( tempStr.c_str(), true );
		else
			edict->s.loopSound = 0;
	}

	arc.ArchiveFloat( &edict->s.loopSoundVolume );
	arc.ArchiveFloat( &edict->s.loopSoundMinDist );
	arc.ArchiveFloat( &edict->s.loopSoundMaxDist );
	arc.ArchiveFloat( &edict->s.loopSoundPitch );
	arc.ArchiveInteger( &edict->s.loopSoundFlags );

	arc.ArchiveInteger( &edict->s.parent );
	arc.ArchiveInteger( &edict->s.tag_num );
	arc.ArchiveBoolean( &edict->s.attach_use_angles );
	arc.ArchiveVec3( edict->s.attach_offset );

	arc.ArchiveInteger( &edict->s.skinNum );
	arc.ArchiveInteger( &edict->s.wasframe );

	for( i = 0; i < NUM_BONE_CONTROLLERS; i++ )
	{
		arc.ArchiveInteger( &edict->s.bone_tag[ i ] );
		arc.ArchiveVec3( edict->s.bone_angles[ i ] );
		arc.ArchiveVec4( edict->s.bone_quat[ i ] );
	}

	arc.ArchiveRaw( &edict->s.surfaces, sizeof( edict->s.surfaces ) );

	arc.ArchiveInteger( &edict->s.clientNum );
	arc.ArchiveInteger( &edict->s.groundEntityNum );
	arc.ArchiveInteger( &edict->s.solid );

	arc.ArchiveFloat( &edict->s.scale );
	arc.ArchiveFloat( &edict->s.alpha );
	arc.ArchiveInteger( &edict->s.renderfx );
	arc.ArchiveVec4( edict->s.quat );
	arc.ArchiveRaw( &edict->mat, sizeof( edict->mat ) );

	arc.ArchiveInteger( &edict->r.svFlags );

	arc.ArchiveVec3( edict->r.mins );
	arc.ArchiveVec3( edict->r.maxs );
	arc.ArchiveInteger( &edict->r.contents );
	arc.ArchiveVec3( edict->r.absmin );
	arc.ArchiveVec3( edict->r.absmax );
	arc.ArchiveFloat( &edict->r.radius );
	if( !arc.Saving() ) {
		edict->radius2 = edict->r.radius * edict->r.radius;
	}

	arc.ArchiveVec3( edict->r.currentOrigin );
	arc.ArchiveVec3( edict->r.currentAngles );

	arc.ArchiveInteger( &edict->r.ownerNum );
	ArchiveEnum( edict->solid, solid_t );
	arc.ArchiveFloat( &edict->freetime );
	arc.ArchiveFloat( &edict->spawntime );

	tempStr = str( edict->entname );
	arc.ArchiveString( &tempStr );
	strncpy( edict->entname, tempStr.c_str(), sizeof( edict->entname ) - 1 );

	arc.ArchiveInteger( &edict->clipmask );
	arc.ArchiveBoolean( &edict->r.bmodel );

	if( arc.Loading() )
	{
		gi.LinkEntity( edict );
	}

	arc.ArchiveInteger( &edict->r.lastNetTime );
}

const char *G_LocationNumToDispString( int iLocation )
{
	switch( iLocation )
	{
	case -2:
	case -1:
		return "";
	case 0:
		return "head";
	case 1:
		return "helmet";
	case 2:
		return "neck";
	case 3:
		return "upper torso";
	case 4:
		return "middle torso";
	case 5:
		return "lower torso";
	case 6:
		return "pelvis";
	case 7:
		return "upper right arm";
	case 8:
		return "upper left arm";
	case 9:
		return "upper right leg";
	case 10:
		return "upper left leg";
	case 11:
		return "lower right arm";
	case 12:
		return "lower left arm";
	case 13:
		return "lower right leg";
	case 14:
		return "lower left leg";
	case 15:
		return "right hand";
	case 16:
		return "left hand";
	case 17:
		return "right foot";
	case 18:
		return "left foot";
	default:
		return "";
	}
}

str G_WeaponClassNumToName
	(
	int num
	)
{
	if( num & WEAPON_CLASS_PISTOL )
	{
		return "pistol";
	}
	else if( num & WEAPON_CLASS_RIFLE )
	{
		return "rifle";
	}
	else if( num & WEAPON_CLASS_SMG )
	{
		return "smg";
	}
	else if( num & WEAPON_CLASS_MG )
	{
		return "mg";
	}
	else if( num & WEAPON_CLASS_GRENADE )
	{
		return "grenade";
	}
	else if( num & WEAPON_CLASS_HEAVY )
	{
		return "heavy";
	}
	else if( num & WEAPON_CLASS_CANNON )
	{
		return "cannon";
	}
	else if( num & WEAPON_CLASS_ITEM )
	{
		return "item";
	}
	else if( num & WEAPON_CLASS_ITEM1 )
	{
		return "item1";
	}
	else if( num & WEAPON_CLASS_ITEM2 )
	{
		return "item2";
	}
	else if( num & WEAPON_CLASS_ITEM3 )
	{
		return "item3";
	}
	else if( num & WEAPON_CLASS_ITEM4 )
	{
		return "item4";
	}
	else
	{
		assert( 0 );
		return "";
	}
}

int G_WeaponClassNameToNum
	(
	str name
	)
{
	int weaponindex = 0;

	if( !name.length() )
	{
		gi.DPrintf( "WeaponClassNameToNum: Weapon class not specified\n" );
		return 0;
	}

	if( !str::icmp( name, "pistol" ) )
	{
		weaponindex = WEAPON_CLASS_PISTOL;
	}
	else if( !str::icmp( name, "rifle" ) )
	{
		weaponindex = WEAPON_CLASS_RIFLE;
	}
	else if( !str::icmp( name, "smg" ) )
	{
		weaponindex = WEAPON_CLASS_SMG;
	}
	else if( !str::icmp( name, "mg" ) )
	{
		weaponindex = WEAPON_CLASS_MG;
	}
	else if( !str::icmp( name, "grenade" ) )
	{
		weaponindex = WEAPON_CLASS_GRENADE;
	}
	else if( !str::icmp( name, "heavy" ) )
	{
		weaponindex = WEAPON_CLASS_HEAVY;
	}
	else if( !str::icmp( name, "cannon" ) )
	{
		weaponindex = WEAPON_CLASS_CANNON;
	}
	else if( !str::icmp( name, "item" ) )
	{
		weaponindex = WEAPON_CLASS_ITEM;
	}
	else if( !str::icmp( name, "item1" ) )
	{
		weaponindex = WEAPON_CLASS_ITEM1;
	}
	else if( !str::icmp( name, "item2" ) )
	{
		weaponindex = WEAPON_CLASS_ITEM2;
	}
	else if( !str::icmp( name, "item3" ) )
	{
		weaponindex = WEAPON_CLASS_ITEM3;
	}
	else if( !str::icmp( name, "item4" ) )
	{
		weaponindex = WEAPON_CLASS_ITEM4;
	}
	else
	{
		gi.DPrintf( "WeaponClassNameToNum: Unknown Weapon class %s\n", name.c_str() );
		return 0;
	}

	return weaponindex;
}

void G_MoveClientToIntermission
	(
	Entity *ent
	)
{
	G_DisplayScores( ent );
	ent->flags |= FL_IMMOBILE;
}

void G_DisplayScores
	(
	Entity *ent
	)
{
	ent->client->ps.pm_flags |= PMF_INTERMISSION;
}

void G_HideScores
	(
	Entity *ent
	)
{
	ent->client->ps.pm_flags &= ~PMF_INTERMISSION;
}

void G_DisplayScoresToAllClients
	(
	void
	)
{
	gentity_t *ent;
	int i;

	for( i = 0, ent = g_entities; i < game.maxclients; ent++, i++ )
	{
		if( !ent->inuse || !ent->entity ) {
			continue;
		}

		G_DisplayScores( ent->entity );
	}
}

void G_HideScoresToAllClients
	(
	void
	)
{
	gentity_t *ent;
	int i;

	for( i = 0, ent = g_entities; i < game.maxclients; ent++, i++ )
	{
		if( !ent->inuse || !ent->entity ) {
			continue;
		}

		G_HideScores( ent->entity );
	}
}
