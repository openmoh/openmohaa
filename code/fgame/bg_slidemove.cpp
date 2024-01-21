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
// bg_slidemove.c -- part of bg_pmove functionality

#include "../qcommon/q_shared.h"
#include "bg_public.h"
#include "bg_local.h"

/*

input: origin, velocity, bounds, groundPlane, trace function

output: origin, velocity, impacts, stairup boolean

*/

/*
==================
PM_SlideMove

Returns qtrue if the velocity was clipped in some way
==================
*/
#define	MAX_CLIP_PLANES	5
qboolean	PM_SlideMove( qboolean gravity )
{
	int			bumpcount, numbumps;
	vec3_t		dir;
	float		d;
	int			numplanes;
	vec3_t		planes[ MAX_CLIP_PLANES ];
	vec3_t		clipVelocity;
	int			i, j, k;
	trace_t	trace;
	vec3_t		end;
	float		time_left;
	float		into;
	vec3_t		endVelocity;
	vec3_t		endClipVelocity;

	numbumps = 4;

	VectorCopy( pm->ps->velocity, endVelocity );

	if( gravity ) {
		endVelocity[ 2 ] -= pm->ps->gravity * pml.frametime;
		pm->ps->velocity[ 2 ] = ( pm->ps->velocity[ 2 ] + endVelocity[ 2 ] ) * 0.5;
		if( pml.groundPlane ) {
			// slide along the ground plane
			PM_ClipVelocity( pm->ps->velocity, pml.groundTrace.plane.normal,
				pm->ps->velocity, OVERCLIP );
		}
	}

	time_left = pml.frametime;

	// never turn against the ground plane
	if( pml.groundPlane ) {
		numplanes = 1;
		VectorCopy( pml.groundTrace.plane.normal, planes[ 0 ] );
	}
	else {
		numplanes = 0;
	}

	// never turn against original velocity
	VectorNormalize2( pm->ps->velocity, planes[ numplanes ] );
	numplanes++;

	for( bumpcount = 0; bumpcount < numbumps; bumpcount++ ) {

		// calculate position we are trying to move to
		VectorMA( pm->ps->origin, time_left, pm->ps->velocity, end );

		// see if we can make it there
		pm->trace( &trace, pm->ps->origin, pm->mins, pm->maxs, end, pm->ps->clientNum, pm->tracemask, qtrue, qfalse );

		if( trace.allsolid ) {
			// entity is completely trapped in another solid
			pm->ps->velocity[ 2 ] = 0;	// don't build up falling damage, but allow sideways acceleration
			return qtrue;
		}

		if( trace.fraction > 0 ) {
			// actually covered some distance
			VectorCopy( trace.endpos, pm->ps->origin );
		}

		if( trace.fraction == 1 ) {
			break;		// moved the entire distance
		}

		if( ( trace.plane.normal[ 2 ] < MIN_WALK_NORMAL ) && ( trace.plane.normal[ 2 ] > 0 ) && DotProduct(trace.plane.normal, pm->ps->velocity) <= 0 )
		{
			// treat steep walls as vertical
			trace.plane.normal[ 2 ] = 0;
			VectorNormalizeFast( trace.plane.normal );
		}

		// save entity for contact
		PM_AddTouchEnt( trace.entityNum );

		time_left -= time_left * trace.fraction;

		if( numplanes >= MAX_CLIP_PLANES ) {
			// this shouldn't really happen
			VectorClear( pm->ps->velocity );
			return qtrue;
		}

		//
		// if this is the same plane we hit before, nudge velocity
		// out along it, which fixes some epsilon issues with
		// non-axial planes
		//
		for( i = 0; i < numplanes; i++ ) {
			if( DotProduct( trace.plane.normal, planes[ i ] ) > 0.99 ) {
				VectorAdd( trace.plane.normal, pm->ps->velocity, pm->ps->velocity );
				break;
			}
		}
		if( i < numplanes ) {
			continue;
		}
		VectorCopy( trace.plane.normal, planes[ numplanes ] );
		numplanes++;

		//
		// modify velocity so it parallels all of the clip planes
		//

		// find a plane that it enters
		for( i = 0; i < numplanes; i++ ) {
			into = DotProduct( pm->ps->velocity, planes[ i ] );
			if( into >= 0.1 ) {
				continue;		// move doesn't interact with the plane
			}

			// see how hard we are hitting things
			if( -into > pml.impactSpeed ) {
				pml.impactSpeed = -into;
			}

			// slide along the plane
			PM_ClipVelocity( pm->ps->velocity, planes[ i ], clipVelocity, OVERCLIP );

			// slide along the plane
			PM_ClipVelocity( endVelocity, planes[ i ], endClipVelocity, OVERCLIP );

			// see if there is a second plane that the new move enters
			for( j = 0; j < numplanes; j++ ) {
				if( j == i ) {
					continue;
				}
				if( DotProduct( clipVelocity, planes[ j ] ) >= 0.1 ) {
					continue;		// move doesn't interact with the plane
				}

				// try clipping the move to the plane
				PM_ClipVelocity( clipVelocity, planes[ j ], clipVelocity, OVERCLIP );
				PM_ClipVelocity( endClipVelocity, planes[ j ], endClipVelocity, OVERCLIP );

				// see if it goes back into the first clip plane
				if( DotProduct( clipVelocity, planes[ i ] ) >= 0 ) {
					continue;
				}

				// slide the original velocity along the crease
				CrossProduct( planes[ i ], planes[ j ], dir );
				VectorNormalize( dir );
				d = DotProduct( dir, pm->ps->velocity );
				VectorScale( dir, d, clipVelocity );

				CrossProduct( planes[ i ], planes[ j ], dir );
				VectorNormalize( dir );
				d = DotProduct( dir, endVelocity );
				VectorScale( dir, d, endClipVelocity );

				// see if there is a third plane the the new move enters
				for( k = 0; k < numplanes; k++ ) {
					if( k == i || k == j ) {
						continue;
					}
					if( DotProduct( clipVelocity, planes[ k ] ) >= 0.1 ) {
						continue;		// move doesn't interact with the plane
					}

					// stop dead at a tripple plane interaction
					VectorClear( pm->ps->velocity );
					return qtrue;
				}
			}

			// if we have fixed all interactions, try another move
			VectorCopy( clipVelocity, pm->ps->velocity );
			VectorCopy( endClipVelocity, endVelocity );
			break;
		}
	}

	if( gravity ) {
		VectorCopy( endVelocity, pm->ps->velocity );
	}

	return ( bumpcount != 0 );
}

/*
==================
PM_StepSlideMove

==================
*/
void PM_StepSlideMove( qboolean gravity )
{
	vec3_t start_o;
	vec3_t start_v;
	vec3_t nostep_o;
	vec3_t nostep_v;
	trace_t trace;
	qboolean bWasOnGoodGround;
	vec3_t up;
	vec3_t down;

	VectorCopy( pm->ps->origin, start_o );
	VectorCopy( pm->ps->velocity, start_v );

	if ( PM_SlideMove( gravity ) == 0 ) {
		return;		// we got exactly where we wanted to go first try	
	}

	VectorCopy( start_o, down );
	down[ 2 ] -= STEPSIZE;
	pm->trace( &trace, start_o, pm->mins, pm->maxs, down, pm->ps->clientNum, pm->tracemask, qtrue, qfalse );
	VectorSet( up, 0, 0, 1 );

	// never step up when you still have up velocity
	if( pm->ps->velocity[ 2 ] > 0 && ( trace.fraction == 1.0f || 
		DotProduct( trace.plane.normal, up ) < MIN_WALK_NORMAL ) ) {
		return;
	}

	if( pml.groundPlane && pml.groundTrace.plane.normal[ 2 ] >= MIN_WALK_NORMAL )
	{
		bWasOnGoodGround = true;
	}
	else
	{
		bWasOnGoodGround = false;
	}

	VectorCopy( start_o, up );
	up[ 2 ] += STEPSIZE;

	// test the player position if they were a stepheight higher
	pm->trace( &trace, up, pm->mins, pm->maxs, up, pm->ps->clientNum, pm->tracemask, qtrue, qfalse );
	if( trace.allsolid )
	{
		up[ 2 ] -= 9.0f;
		pm->trace( &trace, up, pm->mins, pm->maxs, up, pm->ps->clientNum, pm->tracemask, qtrue, qfalse );
		if( trace.allsolid )
		{
			return;
		}
	}

	VectorCopy( pm->ps->origin, nostep_o );
	VectorCopy( pm->ps->velocity, nostep_v );

	// try slidemove from this position
	VectorCopy( up, pm->ps->origin );
	VectorCopy( start_v, pm->ps->velocity );

	PM_SlideMove( gravity );

	// push down the final amount
	VectorCopy( pm->ps->origin, down );
	down[ 2 ] -= STEPSIZE;

	pm->trace( &trace, pm->ps->origin, pm->mins, pm->maxs, down, pm->ps->clientNum, pm->tracemask, qtrue, qfalse );
	if( !trace.allsolid )
	{
		if( bWasOnGoodGround && trace.fraction < 1.0 && trace.plane.normal[ 2 ] < MIN_WALK_NORMAL )
		{
			VectorCopy( nostep_o, pm->ps->origin );
			VectorCopy( nostep_v, pm->ps->velocity );
			return;
		}

		VectorCopy( trace.endpos, pm->ps->origin );
	}

	if ( trace.fraction < 1.0f ) {
		PM_ClipVelocity( pm->ps->velocity, trace.plane.normal, pm->ps->velocity, OVERCLIP );
	}

	pm->stepped = qtrue;
}

