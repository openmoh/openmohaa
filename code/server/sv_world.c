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
// world.c -- world query functions

#include "server.h"
#include "../qcommon/tiki.h"

/*
================
SV_ClipHandleForEntity

Returns a headnode that can be used for testing or clipping to a
given entity.  If the entity is a bsp model, the headnode will
be returned, otherwise a custom box tree will be constructed.
================
*/
clipHandle_t SV_ClipHandleForEntity( const gentity_t *ent ) {
	if ( ent->r.bmodel && ent->solid != SOLID_BBOX ) {
		// explicit hulls in the BSP model
		return CM_InlineModel( ent->s.modelindex );
	}

	// create a temp tree from bounding box sizes
	return CM_TempBoxModel( ent->r.mins, ent->r.maxs, ent->r.contents );
}



/*
===============================================================================

ENTITY CHECKING

To avoid linearly searching through lists of entities during environment testing,
the world is carved up with an evenly spaced, axially aligned bsp tree.  Entities
are kept in chains either at the final leafs, or at the first node that splits
them, which prevents having to deal with multiple fragments of a single entity.

===============================================================================
*/

typedef struct worldSector_s {
	int		axis;		// -1 = leaf node
	float	dist;
	struct worldSector_s	*children[2];
	svEntity_t	*entities;
} worldSector_t;

#define	AREA_DEPTH	4
#define	AREA_NODES	64

worldSector_t	sv_worldSectors[AREA_NODES];
int			sv_numworldSectors;


/*
===============
SV_SectorList_f
===============
*/
void SV_SectorList_f( void ) {
	int				i, c;
	worldSector_t	*sec;
	svEntity_t		*ent;

	for ( i = 0 ; i < AREA_NODES ; i++ ) {
		sec = &sv_worldSectors[i];

		c = 0;
		for ( ent = sec->entities ; ent ; ent = ent->nextEntityInWorldSector ) {
			c++;
		}
		Com_Printf( "sector %i: %i entities\n", i, c );
	}
}

/*
===============
SV_CreateworldSector

Builds a uniformly subdivided tree for the given world size
===============
*/
worldSector_t *SV_CreateworldSector( int depth, vec3_t mins, vec3_t maxs ) {
	worldSector_t	*anode;
	vec3_t		size;
	vec3_t		mins1, maxs1, mins2, maxs2;

	anode = &sv_worldSectors[sv_numworldSectors];
	sv_numworldSectors++;

	if (depth == AREA_DEPTH) {
		anode->axis = -1;
		anode->children[0] = anode->children[1] = NULL;
		return anode;
	}
	
	VectorSubtract (maxs, mins, size);
	if (size[0] > size[1]) {
		anode->axis = 0;
	} else {
		anode->axis = 1;
	}

	anode->dist = 0.5 * (maxs[anode->axis] + mins[anode->axis]);
	VectorCopy (mins, mins1);	
	VectorCopy (mins, mins2);	
	VectorCopy (maxs, maxs1);	
	VectorCopy (maxs, maxs2);	
	
	maxs1[anode->axis] = mins2[anode->axis] = anode->dist;
	
	anode->children[0] = SV_CreateworldSector (depth+1, mins2, maxs2);
	anode->children[1] = SV_CreateworldSector (depth+1, mins1, maxs1);

	return anode;
}

/*
===============
SV_ClearWorld

===============
*/
void SV_ClearWorld( void ) {
	clipHandle_t	h;
	vec3_t			mins, maxs;
	int				i;
	int				num;
	char			name[ 10 ];

	Com_Memset( sv_worldSectors, 0, sizeof(sv_worldSectors) );
	sv_numworldSectors = 0;

	// get world map bounds
	h = CM_InlineModel( 0 );
	CM_ModelBounds( h, mins, maxs );
	SV_CreateworldSector( 0, mins, maxs );

	// set inline models
	num = CM_NumInlineModels();
	for( i = 1; i < num; i++ )
	{
		sprintf( name, "*%i", i );
		SV_ModelIndex( name );
	}
}


/*
===============
SV_UnlinkEntity

===============
*/
void SV_UnlinkEntity( gentity_t *gEnt ) {
	svEntity_t		*ent;
	svEntity_t		*scan;
	worldSector_t	*ws;

	ent = SV_SvEntityForGentity( gEnt );

	gEnt->r.linked = qfalse;

	ws = ent->worldSector;
	if ( !ws ) {
		return;		// not linked in anywhere
	}
	ent->worldSector = NULL;

	if ( ws->entities == ent ) {
		ws->entities = ent->nextEntityInWorldSector;
		return;
	}

	for ( scan = ws->entities ; scan ; scan = scan->nextEntityInWorldSector ) {
		if ( scan->nextEntityInWorldSector == ent ) {
			scan->nextEntityInWorldSector = ent->nextEntityInWorldSector;
			return;
		}
	}

	Com_Printf( "WARNING: SV_UnlinkEntity: not found in worldSector\n" );
}


/*
===============
SV_LinkEntity

===============
*/
#define MAX_TOTAL_ENT_LEAFS		128
void SV_LinkEntity( gentity_t *gEnt ) {
	worldSector_t	*node;
	int			leafs[MAX_TOTAL_ENT_LEAFS];
	int			cluster;
	int			num_leafs;
	int			i;
	int			area;
	int			lastLeaf;
	float		*origin;
	svEntity_t	*ent;

	ent = SV_SvEntityForGentity( gEnt );

	if ( ent->worldSector ) {
		SV_UnlinkEntity( gEnt );	// unlink from old position
	}

	switch( gEnt->solid )
	{
	case SOLID_TRIGGER:
		gEnt->s.solid = 0;
		break;
	case SOLID_BBOX:
		if( !gEnt->r.contents || ( gEnt->r.contents == CONTENTS_SHOOTONLY ) )
		{
			gEnt->s.solid = 0;
		}
		else
		{
			// encode the size into the entityState_t for client prediction
			gEnt->s.solid = BoundingBoxToInteger( gEnt->r.mins, gEnt->r.maxs );
		}
		break;
	case SOLID_BSP:
		if( gEnt->r.contents == -1 || !( gEnt->r.contents & CONTENTS_SHOOTONLY ) )
		{
			gEnt->s.solid = SOLID_BMODEL;
		}
		else
		{
			gEnt->s.solid = 0;
		}
		break;
	default:
		gEnt->s.solid = gEnt->solid;
		break;

	}

	// get the position
	origin = gEnt->s.origin;

	// set the abs box
	if( gEnt->r.currentAngles[ 0 ] || gEnt->r.currentAngles[ 1 ] || gEnt->r.currentAngles[ 2 ] ) {
		// expand for rotation
		VectorCopy( gEnt->r.mins, gEnt->r.absmin );
		VectorCopy( gEnt->r.maxs, gEnt->r.absmax );
		CalculateRotatedBounds2( gEnt->mat, gEnt->r.absmin, gEnt->r.absmax );
		VectorAdd( origin, gEnt->r.absmin, gEnt->r.absmin );
		VectorAdd( origin, gEnt->r.absmax, gEnt->r.absmax );
	} else {
		// normal
		VectorAdd( origin, gEnt->r.mins, gEnt->r.absmin );
		VectorAdd( origin, gEnt->r.maxs, gEnt->r.absmax );
	}

	// because movement is clipped an epsilon away from an actual edge,
	// we must fully check even when bounding boxes don't quite touch
	gEnt->r.absmin[0] -= 1;
	gEnt->r.absmin[1] -= 1;
	gEnt->r.absmin[2] -= 1;
	gEnt->r.absmax[0] += 1;
	gEnt->r.absmax[1] += 1;
	gEnt->r.absmax[2] += 1;

	// link to PVS leafs
	ent->numClusters = 0;
	ent->lastCluster = 0;
	ent->areanum = -1;
	ent->areanum2 = -1;

	//get all leafs, including solids
	num_leafs = CM_BoxLeafnums( gEnt->r.absmin, gEnt->r.absmax,
		leafs, MAX_TOTAL_ENT_LEAFS, &lastLeaf );

	// if none of the leafs were inside the map, the
	// entity is outside the world and can be considered unlinked
	if ( !num_leafs ) {
		return;
	}

	// set areas, even from clusters that don't fit in the entity array
	for (i=0 ; i<num_leafs ; i++) {
		area = CM_LeafArea (leafs[i]);
		if (area != -1) {
			// doors may legally straggle two areas,
			// but nothing should evern need more than that
			if (ent->areanum != -1 && ent->areanum != area) {
				if (ent->areanum2 != -1 && ent->areanum2 != area && sv.state == SS_LOADING) {
					Com_DPrintf ("Object %i touching 3 areas at %f %f %f\n",
						gEnt->s.number,
						gEnt->r.absmin[ 0 ], gEnt->r.absmin[ 1 ], gEnt->r.absmin[ 2 ] );
				}
				ent->areanum2 = area;
			} else {
				ent->areanum = area;
			}
		}
	}

	// store the areanum to gentity
	gEnt->r.areanum = ent->areanum;

	// store as many explicit clusters as we can
	ent->numClusters = 0;
	for (i=0 ; i < num_leafs ; i++) {
		cluster = CM_LeafCluster( leafs[i] );
		if ( cluster != -1 ) {
			ent->clusternums[ent->numClusters++] = cluster;
			if ( ent->numClusters == MAX_ENT_CLUSTERS ) {
				break;
			}
		}
	}

	// store off a last cluster if we need to
	if ( i != num_leafs ) {
		ent->lastCluster = CM_LeafCluster( lastLeaf );
	}

	gEnt->r.linkcount++;

	// find the first world sector node that the ent's box crosses
	node = sv_worldSectors;
	while (1)
	{
		if (node->axis == -1)
			break;
		if( gEnt->r.absmin[ node->axis ] > node->dist )
			node = node->children[0];
		else if( gEnt->r.absmax[ node->axis ] < node->dist )
			node = node->children[1];
		else
			break;		// crosses the node
	}
	
	// link it in
	ent->worldSector = node;
	ent->nextEntityInWorldSector = node->entities;
	node->entities = ent;

	gEnt->r.linked = qtrue;
}

/*
============================================================================

AREA QUERY

Fills in a list of all entities who's absmin / absmax intersects the given
bounds.  This does NOT mean that they actually touch in the case of bmodels.
============================================================================
*/

typedef struct {
	const float	*mins;
	const float	*maxs;
	int			*list;
	int			count, maxcount;
} areaParms_t;


/*
====================
SV_AreaEntities_r

====================
*/
void SV_AreaEntities_r( worldSector_t *node, areaParms_t *ap ) {
	svEntity_t		*check, *next;
	gentity_t		*gcheck;
	worldSector_t	*nodestack[ 8 ];
	int				iStackPos;

	iStackPos = 0;

	while( 1 )
	{
		while( 1 )
		{
			for( check = node->entities; check; check = next ) {
				next = check->nextEntityInWorldSector;

				gcheck = SV_GEntityForSvEntity( check );

				if( gcheck->r.absmin[ 0 ] > ap->maxs[ 0 ]
					|| gcheck->r.absmin[ 1 ] > ap->maxs[ 1 ]
					|| gcheck->r.absmin[ 2 ] > ap->maxs[ 2 ]
					|| gcheck->r.absmax[ 0 ] < ap->mins[ 0 ]
					|| gcheck->r.absmax[ 1 ] < ap->mins[ 1 ]
					|| gcheck->r.absmax[ 2 ] < ap->mins[ 2 ] ) {
					continue;
				}

				if( ap->count == ap->maxcount ) {
					Com_Printf( "SV_AreaEntities: MAXCOUNT\n" );
					return;
				}

				ap->list[ ap->count ] = check - sv.svEntities;
				ap->count++;
			}

			if( node->axis == -1 ) {
				break;		// terminal node
			}

			// recurse down both sides
			if( ap->maxs[ node->axis ] > node->dist ) {
				nodestack[ iStackPos++ ] = node->children[ 0 ];
			}

			if( ap->mins[ node->axis ] < node->dist ) {
				node = node->children[ 1 ];
			} else {
				break;
			}
		}

		if( !iStackPos ) {
			return;
		}

		iStackPos--;

		node = nodestack[ iStackPos ];
	}
}

/*
================
SV_AreaEntities
================
*/
int SV_AreaEntities( const vec3_t mins, const vec3_t maxs, int *entityList, int maxcount ) {
	areaParms_t		ap;

	ap.mins = mins;
	ap.maxs = maxs;
	ap.list = entityList;
	ap.count = 0;
	ap.maxcount = maxcount;

	SV_AreaEntities_r( sv_worldSectors, &ap );

	return ap.count;
}



//===========================================================================


typedef struct {
	vec3_t		boxmins, boxmaxs;// enclose the test object along entire move
	const float	*mins;
	const float *maxs;	// size of the moving object
	const float	*start;
	vec3_t		end;
	trace_t		trace;
	int			passEntityNum;
	int			contentmask;
	qboolean	cylinder;
	qboolean	traceDeep;
} moveclip_t;


/*
====================
SV_ClipToEntity

====================
*/
void SV_ClipToEntity( trace_t *trace, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int entityNum, int contentmask ) {
	gentity_t		*touch;
	clipHandle_t	clipHandle;

	touch = SV_GentityNum( entityNum );

	Com_Memset( trace, 0, sizeof( trace_t ) );

	// if it doesn't have any brushes of a type we
	// are looking for, ignore it
	if ( ! ( contentmask & touch->r.contents ) ) {
		trace->fraction = 1.0;
		return;
	}

	// might intersect, so do an exact clip
	clipHandle = SV_ClipHandleForEntity (touch);

	CM_TransformedBoxTrace ( trace, start, end,
		mins, maxs, clipHandle,  contentmask,
		touch->s.origin, touch->r.currentAngles, qfalse );

	if ( trace->fraction < 1 ) {
		trace->entityNum = touch->s.number;
	}
}


/*
====================
SV_ClipMoveToEntities

====================
*/
void SV_ClipMoveToEntities( moveclip_t *clip ) {
	int			i, num;
	int			touchlist[MAX_GENTITIES];
	gentity_t	*touch;
	int			passOwnerNum;
	trace_t		trace;
	clipHandle_t	clipHandle;

	num = SV_AreaEntities( clip->boxmins, clip->boxmaxs, touchlist, MAX_GENTITIES );

	if ( clip->passEntityNum != ENTITYNUM_NONE ) {
		passOwnerNum = ( SV_GentityNum( clip->passEntityNum ) )->r.ownerNum;
		if ( passOwnerNum == ENTITYNUM_NONE ) {
			passOwnerNum = -1;
		}
	} else {
		passOwnerNum = -1;
	}

	for ( i=0 ; i<num ; i++ ) {
		if ( clip->trace.allsolid ) {
			return;
		}
		touch = SV_GentityNum( touchlist[i] );

		// skip non-solids and triggers
		if( touch->solid == SOLID_NOT || touch->solid == SOLID_TRIGGER ) {
			continue;
		}

		// see if we should ignore this entity
		if ( clip->passEntityNum != ENTITYNUM_NONE ) {
			if ( touchlist[i] == clip->passEntityNum ) {
				continue;	// don't clip against the pass entity
			}
			if (touchlist[i] == passOwnerNum) {
				continue; // don't clip against the owner of this entity
			}
			if ( touch->r.ownerNum == clip->passEntityNum ) {
				continue;	// don't clip against own missiles
			}
			if ( touch->r.ownerNum == passOwnerNum ) {
				continue;	// don't clip against other missiles from our owner
			}
		}

		// if it doesn't have any brushes of a type we
		// are looking for, ignore it
		if ( ! ( clip->contentmask & touch->r.contents ) ) {
			continue;
		}

		if( clip->traceDeep && touch->tiki != NULL && touch->tiki->a->bIsCharacter )
		{
			// do an extended trace with an output location
			SV_TraceDeep(
				&trace,
				clip->start,
				clip->end,
				clip->contentmask,
				touch );
		}
		else
		{
			// might intersect, so do an exact clip
			clipHandle = SV_ClipHandleForEntity( touch );

			CM_TransformedBoxTrace( &trace, clip->start, clip->end,
				clip->mins, clip->maxs, clipHandle, clip->contentmask,
				touch->s.origin, touch->r.currentAngles, clip->cylinder );
		}

		if ( trace.allsolid ) {
			clip->trace.allsolid = qtrue;
			clip->trace.entityNum = touch->s.number;
			trace.entityNum = touch->s.number;
		} else if ( trace.startsolid ) {
			clip->trace.startsolid = qtrue;
			clip->trace.entityNum = touch->s.number;
			trace.entityNum = touch->s.number;
		}

		if ( trace.fraction < clip->trace.fraction ) {
			qboolean	oldStart;

			// make sure we keep a startsolid from a previous trace
			oldStart = clip->trace.startsolid;

			trace.entityNum = touch->s.number;
			clip->trace = trace;
			clip->trace.startsolid |= oldStart;
		}
	}
}

/*
====================
SV_ClipSightToEntities
====================
*/
qboolean SV_ClipSightToEntities( moveclip_t *clip, int passEntityNum2 )
{
	int			i, num;
	int			touchlist[ MAX_GENTITIES ];
	gentity_t	*touch;
	int			passOwnerNum;
	int			passOwnerNum2;
	clipHandle_t	clipHandle;

	num = SV_AreaEntities( clip->boxmins, clip->boxmaxs, touchlist, MAX_GENTITIES );

	if( clip->passEntityNum != ENTITYNUM_NONE ) {
		passOwnerNum = ( SV_GentityNum( clip->passEntityNum ) )->r.ownerNum;
		if( passOwnerNum == ENTITYNUM_NONE ) {
			passOwnerNum = -1;
		}
	} else {
		passOwnerNum = -1;
	}

	if( passEntityNum2 != ENTITYNUM_NONE ) {
		passOwnerNum2 = ( SV_GentityNum( passEntityNum2 ) )->r.ownerNum;
		if( passOwnerNum2 == ENTITYNUM_NONE ) {
			passOwnerNum2 = -1;
		}
	} else {
		passOwnerNum2 = -1;
	}

	for( i = 0; i<num; i++ ) {
		touch = SV_GentityNum( touchlist[ i ] );

		// skip non-solids and triggers
		if( touch->solid == SOLID_NOT || touch->solid == SOLID_TRIGGER ) {
			continue;
		}

		// see if we should ignore this entity
		if( clip->passEntityNum != ENTITYNUM_NONE ) {
			if( touchlist[ i ] == clip->passEntityNum ) {
				continue;	// don't clip against the pass entity
			}
			if( touch->r.ownerNum == clip->passEntityNum ) {
				continue;	// don't clip against own missiles
			}
			if( touch->r.ownerNum == passOwnerNum ) {
				continue;	// don't clip against other missiles from our owner
			}
		}

		if( passEntityNum2 != ENTITYNUM_NONE ) {
			if( touchlist[ i ] == passEntityNum2 ) {
				continue;	// don't clip against the pass entity
			}
			if( touch->r.ownerNum == passEntityNum2 ) {
				continue;	// don't clip against own missiles
			}
			if( touch->r.ownerNum == passOwnerNum2 ) {
				continue;	// don't clip against other missiles from our owner
			}
		}

		// if it doesn't have any brushes of a type we
		// are looking for, ignore it
		if( !( clip->contentmask & touch->r.contents ) ) {
			continue;
		}

		// might intersect, so do an exact clip
		clipHandle = SV_ClipHandleForEntity( touch );

		if( !CM_TransformedBoxSightTrace( clip->start, clip->end,
			clip->mins, clip->maxs, clipHandle, clip->contentmask,
			touch->s.origin, touch->r.currentAngles, clip->cylinder ) ) {
			return qfalse;
		}
	}

	return qtrue;
}

/*
==================
SV_SightTraceEntity

Returns false if something was hit.
==================
*/
qboolean SV_SightTraceEntity( gentity_t *touch, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int contentmask, qboolean cylinder ) {
	clipHandle_t clipHandle = SV_ClipHandleForEntity( touch );
	return CM_TransformedBoxSightTrace( start, end, mins, maxs, clipHandle, contentmask, touch->s.origin, touch->r.currentAngles, cylinder );
}

/*
==================
SV_SightTrace

Returns false if something was hit.
==================
*/
qboolean SV_SightTrace( const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int passEntityNum2, int contentmask, qboolean cylinder ) {
	moveclip_t clip;
	int i;

	if( !CM_BoxSightTrace( start, end, mins, maxs, 0, contentmask, cylinder ) ) {
		return qfalse;
	}

	clip.contentmask = contentmask;
	clip.start = start;
	VectorCopy( end, clip.end );
	clip.mins = mins;
	clip.maxs = maxs;
	clip.passEntityNum = passEntityNum;
	clip.cylinder = cylinder;

	// create the bounding box of the entire move
	// we can limit it to the part of the move not
	// already clipped off by the world, which can be
	// a significant savings for line of sight and shot traces
	for( i = 0; i<3; i++ ) {
		if( end[ i ] > start[ i ] ) {
			clip.boxmins[ i ] = clip.start[ i ] + clip.mins[ i ] - 1;
			clip.boxmaxs[ i ] = clip.end[ i ] + clip.maxs[ i ] + 1;
		}
		else {
			clip.boxmins[ i ] = clip.end[ i ] + clip.mins[ i ] - 1;
			clip.boxmaxs[ i ] = clip.start[ i ] + clip.maxs[ i ] + 1;
		}
	}

	// clip to other solid entities
	return SV_ClipSightToEntities( &clip, passEntityNum2 );
}
/*
==================
SV_HitEntity

This functions checks if an entity is inside another entity.
==================
*/
qboolean SV_HitEntity(gentity_t* pEnt, gentity_t* pOther)
{
	clipHandle_t handle = SV_ClipHandleForEntity(pOther);

	return !CM_TransformedBoxSightTrace(
		pEnt->s.origin,
		pEnt->s.origin,
		pEnt->r.mins,
		pEnt->r.maxs,
		handle,
		CONTENTS_SOLID,
		pOther->s.origin,
		pOther->r.currentAngles,
		qfalse
	);
}

/*
==================
SV_Trace

Moves the given mins/maxs volume through the world from start to end.
passEntityNum and entities owned by passEntityNum are explicitly not checked.
==================
*/
void SV_Trace( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask, qboolean cylinder, qboolean traceDeep ) {
	moveclip_t	clip;
	int			i;

	Com_Memset( &clip, 0, sizeof( moveclip_t ) );

	// clip to world
	CM_BoxTrace( &clip.trace, start, end, mins, maxs, 0, contentmask, cylinder );
	clip.trace.entityNum = clip.trace.fraction != 1.0 ? ENTITYNUM_WORLD : ENTITYNUM_NONE;
	if ( clip.trace.fraction == 0 ) {
		*results = clip.trace;
		return;		// blocked immediately by the world
	}

	clip.contentmask = contentmask;
	clip.start = start;
//	VectorCopy( clip.trace.endpos, clip.end );
	VectorCopy( end, clip.end );
	clip.mins = mins;
	clip.maxs = maxs;
	clip.passEntityNum = passEntityNum;
	clip.cylinder = cylinder;
	clip.traceDeep = traceDeep;

	// create the bounding box of the entire move
	// we can limit it to the part of the move not
	// already clipped off by the world, which can be
	// a significant savings for line of sight and shot traces
	for ( i=0 ; i<3 ; i++ ) {
		if ( end[i] > start[i] ) {
			clip.boxmins[i] = clip.start[i] + clip.mins[i] - 1;
			clip.boxmaxs[i] = clip.end[i] + clip.maxs[i] + 1;
		} else {
			clip.boxmins[i] = clip.end[i] + clip.mins[i] - 1;
			clip.boxmaxs[i] = clip.start[i] + clip.maxs[i] + 1;
		}
	}

	// clip to other solid entities
	SV_ClipMoveToEntities( &clip );

	*results = clip.trace;
}

/*
=============
SV_GetShaderPointer
=============
*/
baseshader_t *SV_GetShaderPointer( int iShaderNum )
{
	return CM_ShaderPointer( iShaderNum );
}

/*
=============
SV_PointContents
=============
*/
int SV_PointContents( const vec3_t p, int passEntityNum ) {
	int			touch[MAX_GENTITIES];
	gentity_t	*hit;
	int			i, num;
	int			contents, c2;
	clipHandle_t	clipHandle;
	float		*angles;

	// get base contents from world
	contents = CM_PointContents( p, 0 );

	// or in contents from all the other entities
	num = SV_AreaEntities( p, p, touch, MAX_GENTITIES );

	for ( i=0 ; i<num ; i++ ) {
		if ( touch[i] == passEntityNum ) {
			continue;
		}
		hit = SV_GentityNum( touch[i] );
		// might intersect, so do an exact clip
		clipHandle = SV_ClipHandleForEntity( hit );
		angles = hit->s.angles;
		if ( !hit->r.bmodel ) {
			angles = vec3_origin;	// boxes don't rotate
		}

		c2 = CM_TransformedPointContents (p, clipHandle, hit->s.origin, hit->s.angles);

		contents |= c2;
	}

	return contents;
}


