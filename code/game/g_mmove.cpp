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

// g_mmove.cpp : AI/Path movement code.
//

#include "g_local.h"
#include "entity.h"

typedef struct {
	qboolean		validGroundTrace;
	trace_t			groundTrace;
	float			previous_origin[ 3 ];
	float			previous_velocity[ 3 ];
} mml_t;

mmove_t *mm;
mml_t mml;

void MM_ClipVelocity
	(
	float *in,
	float *normal,
	float *out,
	float overbounce
	)

{
	float backoff;
	float dir_z;
	float normal2[ 3 ];

	if( normal[ 2 ] >= 0.70f )
	{
		if( in[ 0 ] == 0.0f && in[ 1 ] == 0.0f )
		{
			VectorClear( out );
			return;
		}

		normal2[ 0 ] = in[ 0 ] + DotProduct2D( in, normal );
		normal2[ 1 ] = in[ 1 ] + DotProduct2D( in, normal );
		normal2[ 2 ] = normal[ 2 ] * DotProduct2D( in, in );

		VectorNormalize( normal2 );

		dir_z = -normal2[ 2 ];

		out[ 0 ] = in[ 0 ];
		out[ 1 ] = in[ 1 ];
		out[ 2 ] = DotProduct2D( in, normal2 ) / dir_z;
	}
	else
	{
		backoff = DotProduct( in, normal );

		if( backoff < 0 )
			backoff *= overbounce;
		else
			backoff /= overbounce;

		out[ 0 ] = in[ 0 ] - normal[ 0 ] * backoff;
		out[ 1 ] = in[ 1 ] - normal[ 1 ] * backoff;
		out[ 2 ] = in[ 2 ] - normal[ 2 ] * backoff;
	}
}

qboolean MM_AddTouchEnt
	(
	int entityNum
	)

{
	int			i;
	qboolean	blockEnt;
	Entity		*ent;

	if( entityNum == ENTITYNUM_NONE || entityNum == ENTITYNUM_WORLD ) {
		return qtrue;
	}

	ent = G_GetEntity( entityNum );

	blockEnt = ent->BlocksAIMovement();

	if( !blockEnt )
	{
		if( ent->IsSubclassOfPlayer() )
		{
			mm->hit_temp_obstacle |= 1;
		}
		else if( ent->IsSubclassOfDoor() )
		{
			mm->hit_temp_obstacle |= 2;
		}
	}

	// see if it is already added
	for( i = 0; i < mm->numtouch; i++ )
	{
		if( mm->touchents[ i ] == entityNum )
			return blockEnt;
	}

	// add it
	mm->touchents[ mm->numtouch ] = entityNum;
	mm->numtouch++;

	return blockEnt;
}

qboolean MM_SlideMove
	(
	qboolean gravity
	)

{
	int bumpcount;
	vec3_t dir;
	float d;
	int numplanes;
	vec3_t planes[ 5 ];
	vec3_t clipVelocity;
	int i;
	int j;
	int k;
	trace_t trace;
	vec3_t end;
	float time_left;
	qboolean bBlockEnt;

	if( gravity )
	{
		mm->velocity[ 2 ] = mm->velocity[ 2 ] - mm->frametime * sv_gravity->integer;
		if( mm->groundPlane )
			MM_ClipVelocity( mm->velocity, mm->groundPlaneNormal, mm->velocity, OVERCLIP );
	}

	time_left = mm->frametime;

	if( mm->groundPlane ) {
		numplanes = 1;
		VectorCopy( mm->groundPlaneNormal, planes[ 0 ] );
	} else {
		numplanes = 0;
	}

	// never turn against original velocity
	VectorNormalize2( mm->velocity, planes[ numplanes ] );
	numplanes++;

	for( bumpcount = 0; bumpcount < 4; bumpcount++ )
	{
		// calculate position we are trying to move to
		VectorMA( mm->origin, time_left, mm->velocity, end );

		// see if we can make it there
		gi.Trace( &trace, mm->origin, mm->mins, mm->maxs, end, mm->entityNum, mm->tracemask, qtrue, qfalse );

		if( trace.allsolid )
			break;

		if( trace.fraction > 0 ) {
			// actually covered some distance
			VectorCopy( trace.endpos, mm->origin );
		}

		if( trace.fraction == 1 )
			return bumpcount != 0;

		// save entity for contact
		bBlockEnt = MM_AddTouchEnt( trace.entityNum );

		if( trace.plane.normal[ 2 ] < MIN_WALK_NORMAL )
		{
			if( trace.plane.normal[ 2 ] > -0.999f && bBlockEnt && mm->groundPlane )
			{
				if( !mm->hit_obstacle )
				{
					mm->hit_obstacle = true;
					VectorCopy( mm->origin, mm->hit_origin );
				}

				VectorAdd( mm->obstacle_normal, trace.plane.normal, mm->obstacle_normal );
			}
		}
		else
		{
			memcpy( &mml.groundTrace, &trace, sizeof( mml.groundTrace ) );
			mml.validGroundTrace = true;
		}

		time_left -= time_left * trace.fraction;

		if( numplanes >= MAX_CLIP_PLANES )
		{
			VectorClear( mm->velocity );
			return qtrue;
		}

		//
		// if this is the same plane we hit before, nudge velocity
		// out along it, which fixes some epsilon issues with
		// non-axial planes
		//
		for( i = 0; i < numplanes; i++ )
		{
			if( DotProduct( trace.plane.normal, planes[ i ] ) > 0.99 )
			{
				VectorAdd( trace.plane.normal, mm->velocity, mm->velocity );
				break;
			}
		}

		if( i >= numplanes )
		{
			//
			// modify velocity so it parallels all of the clip planes
			//

			// find a plane that it enters
			for( i = 0; i < numplanes; i++ )
			{
				if( DotProduct( mm->velocity, planes[ i ] ) >= 0.1 ) {
					continue;		// move doesn't interact with the plane
				}

				// slide along the plane
				MM_ClipVelocity( mm->velocity, planes[ i ], clipVelocity, OVERCLIP );

				// see if there is a second plane that the new move enters
				for( j = 0; j < numplanes; j++ )
				{
					if( j == i ) {
						continue;
					}

					// slide along the plane
					MM_ClipVelocity( mm->velocity, planes[ j ], clipVelocity, OVERCLIP );

					if( DotProduct( clipVelocity, planes[ j ] ) >= 0.0f ) {
						continue;		// move doesn't interact with the plane
					}

					// slide the original velocity along the crease
					CrossProduct( planes[ i ], planes[ j ], dir );
					VectorNormalize( dir );
					d = DotProduct( dir, mm->velocity );
					VectorScale( dir, d, clipVelocity );

					// see if there is a third plane the the new move enters
					for( k = 0; k < numplanes; k++ )
					{
						if( k == i || k == j ) {
							continue;
						}

						if( DotProduct( clipVelocity, planes[ k ] ) >= 0.1f ) {
							continue;		// move doesn't interact with the plane
						}

						// stop dead at a tripple plane interaction
						VectorClear( mm->velocity );
						return qtrue;
					}
				}

				// if we have fixed all interactions, try another move
				VectorCopy( clipVelocity, mm->velocity );
				break;
			}
		}
	}

	if( mm->velocity[ 0 ] || mm->velocity[ 1 ] )
	{
		if( mm->groundPlane )
		{
			VectorCopy( mm->velocity, dir );
			VectorNegate( dir, dir );
			VectorNormalize( dir );

			if( MM_AddTouchEnt( trace.entityNum ) )
			{
				if( !mm->hit_obstacle )
				{
					mm->hit_obstacle = true;
					VectorCopy( mm->origin, mm->hit_origin );
				}

				VectorAdd( mm->obstacle_normal, dir, mm->obstacle_normal );
			}
		}

		VectorClear( mm->velocity );
		return true;
	}

	mm->velocity[ 2 ] = 0;
	return false;
}

void MM_GroundTraceInternal
	(
	void
	)

{
	if( mml.groundTrace.fraction == 1.0f )
	{
		mm->groundPlane = qfalse;
		mm->walking = qfalse;
		return;
	}

	if( mm->velocity[ 2 ] > 0.0f )
	{
		if( DotProduct( mm->velocity, mml.groundTrace.plane.normal ) > 10.0f )
		{
			mm->groundPlane = qfalse;
			mm->walking = qfalse;
			return;
		}
	}

	// slopes that are too steep will not be considered onground
	if( mml.groundTrace.plane.normal[ 2 ] < MIN_WALK_NORMAL )
	{
		vec3_t oldvel;
		float d;

		VectorCopy( mm->velocity, oldvel );
		VectorSet( mm->velocity, 0, 0, -1.0f / mm->frametime );
		MM_SlideMove( qfalse );

		d = VectorLength( mm->velocity );
		VectorCopy( oldvel, mm->velocity );

		if( d > ( 0.1f / mm->frametime ) )
		{
			mm->groundPlane = qtrue;
			mm->walking = qfalse;
			VectorCopy( mml.groundTrace.plane.normal, mm->groundPlaneNormal );
			return;
		}
	}

	mm->groundPlane = qtrue;
	mm->walking = qtrue;
	VectorCopy( mml.groundTrace.plane.normal, mm->groundPlaneNormal );

	MM_AddTouchEnt( mml.groundTrace.entityNum );
}

void MM_GroundTrace
	(
	void
	)

{
	float point[ 3 ];

	point[ 0 ] = mm->origin[ 0 ];
	point[ 1 ] = mm->origin[ 1 ];
	point[ 2 ] = mm->origin[ 2 ] - 0.25f;

	gi.Trace( &mml.groundTrace, mm->origin, mm->mins, mm->maxs, point, mm->entityNum, mm->tracemask, qtrue, qfalse );
	MM_GroundTraceInternal();
}

void MM_StepSlideMove
	(
	void
	)

{
	vec3_t start_o;
	vec3_t start_v;
	vec3_t nostep_o;
	vec3_t nostep_v;
	trace_t trace;
	qboolean bWasOnGoodGround;
	vec3_t up;
	vec3_t down;
	qboolean start_hit_wall;
	vec3_t start_wall_normal;
	qboolean first_hit_wall;
	vec3_t first_wall_normal;
	vec3_t start_hit_origin;
	vec3_t first_hit_origin;
	trace_t nostep_groundTrace;

	VectorCopy( mm->origin, start_o );
	VectorCopy( mm->velocity, start_v );
	start_hit_wall = mm->hit_obstacle;
	VectorCopy( mm->hit_origin, start_hit_origin );
	VectorCopy( mm->obstacle_normal, start_wall_normal );

	if( MM_SlideMove( qtrue ) == 0 )
	{
		if( !mml.validGroundTrace )
			MM_GroundTrace();

		return;
	}

	VectorCopy( start_o, down );
	down[ 2 ] -= STEPSIZE;
	gi.Trace( &trace, start_o, mm->mins, mm->maxs, down, mm->entityNum, mm->tracemask, qtrue, qfalse );
	VectorSet( up, 0, 0, 1 );

	// never step up when you still have up velocity
	if( mm->velocity[ 2 ] > 0 && ( trace.fraction == 1.0f ||
		DotProduct( trace.plane.normal, up ) < MIN_WALK_NORMAL ) )
	{
		if( !mml.validGroundTrace )
			MM_GroundTrace();
		else
			MM_GroundTraceInternal();

		return;
	}

	if( mm->groundPlane && mm->groundPlaneNormal[ 2 ] >= MIN_WALK_NORMAL )
		bWasOnGoodGround = qtrue;
	else
		bWasOnGoodGround = qfalse;

	VectorCopy( mm->origin, nostep_o );
	VectorCopy( mm->velocity, nostep_v );
	memcpy( &nostep_groundTrace, &mml.groundTrace, sizeof( trace_t ) );

	VectorCopy( start_o, mm->origin );
	VectorCopy( start_v, mm->velocity );

	first_hit_wall = mm->hit_obstacle;
	VectorCopy( mm->hit_origin, first_hit_origin );
	VectorCopy( mm->obstacle_normal, first_wall_normal );

	mm->hit_obstacle = start_hit_wall;
	VectorCopy( start_hit_origin, mm->hit_origin );
	VectorCopy( start_wall_normal, mm->obstacle_normal );
	MM_SlideMove( qtrue );

	VectorCopy( mm->origin, down );
	down[ 2 ] -= STEPSIZE * 2;

	// test the player position if they were a stepheight higher
	gi.Trace( &trace, up, mm->mins, mm->maxs, up, mm->entityNum, mm->tracemask, qtrue, qfalse );
	if( trace.entityNum > ENTITYNUM_NONE )
	{
		VectorCopy( nostep_o, mm->origin );
		VectorCopy( nostep_v, mm->velocity );
		memcpy( &mml.groundTrace, &nostep_groundTrace, sizeof( mml.groundTrace ) );
		mm->hit_obstacle = first_hit_wall;
		VectorCopy( first_hit_origin, mm->hit_origin );
		VectorCopy( first_wall_normal, mm->obstacle_normal );

		if( !mml.validGroundTrace )
			MM_GroundTrace();
		else
			MM_GroundTraceInternal();

		return;
	}
	
	if( !trace.allsolid )
	{
		memcpy( &mml.groundTrace, &trace, sizeof( mml.groundTrace ) );
		mml.validGroundTrace = qtrue;

		if( bWasOnGoodGround && trace.fraction && trace.plane.normal[ 2 ] < MIN_WALK_NORMAL )
		{
			VectorCopy( nostep_o, mm->origin );
			VectorCopy( nostep_v, mm->velocity );

			if( first_hit_wall )
			{
				mm->hit_obstacle = first_hit_wall;
				VectorCopy( first_hit_origin, mm->hit_origin );
				VectorCopy( first_wall_normal, mm->obstacle_normal );
			}

			MM_GroundTraceInternal();
			return;
		}

		VectorCopy( trace.endpos, mm->origin );
	}

	if( trace.fraction < 1.0f )
		MM_ClipVelocity( mm->velocity, trace.plane.normal, mm->velocity, OVERCLIP );

	if( !mml.validGroundTrace )
		MM_GroundTrace();
	else
		MM_GroundTraceInternal();
}

void MM_ClipVelocity2D
	(
	float *in,
	float *normal,
	float *out,
	float overbounce
	)

{
	float backoff;
	float dir_z;
	float normal2[ 3 ];

	if( normal[ 2 ] >= 0.70f )
	{
		if( in[ 0 ] == 0.0f && in[ 1 ] == 0.0f )
		{
			VectorClear( out );
			return;
		}

		normal2[ 0 ] = in[ 0 ] + DotProduct2D( in, normal );
		normal2[ 1 ] = in[ 1 ] + DotProduct2D( in, normal );
		normal2[ 2 ] = normal[ 2 ] * DotProduct2D( in, in );

		VectorNormalize( normal2 );

		dir_z = -normal2[ 2 ];

		out[ 0 ] = in[ 0 ];
		out[ 1 ] = in[ 1 ];
		out[ 2 ] = DotProduct2D( in, normal2 ) / -normal2[ 2 ];
	}
	else
	{
		backoff = DotProduct2D( in, normal );

		if( backoff < 0 )
			backoff *= overbounce;
		else
			backoff /= overbounce;

		out[ 0 ] = in[ 0 ] - normal[ 0 ] * backoff;
		out[ 1 ] = in[ 1 ] - normal[ 1 ] * backoff;
		out[ 2 ] = -( backoff * normal[ 2 ] );
	}
}

void MmoveSingle
	(
	mmove_t *mmove
	)

{
	float point[ 3 ];
	trace_t trace;

	mm = mmove;

	mmove->hit_obstacle = false;
	mmove->numtouch = false;
	VectorCopy( vec_origin, mmove->obstacle_normal );
	mmove->hit_temp_obstacle = false;

	memset( &mml, 0, sizeof( mml_t ) );

	VectorCopy( mmove->origin, mml.previous_origin );
	VectorCopy( mmove->velocity, mml.previous_velocity );

	if( mmove->walking )
	{
		if( mmove->desired_speed < 1.0f )
		{
			MM_GroundTrace();
			return;
		}

		float wishdir[ 3 ];

		MM_ClipVelocity2D( mm->desired_dir, mm->groundPlaneNormal, wishdir, OVERCLIP );
		VectorNormalize( wishdir );

		mm->velocity[ 0 ] = mm->desired_speed * wishdir[ 0 ];
		mm->velocity[ 1 ] = mm->desired_speed * wishdir[ 1 ];
	}
	else if( mmove->groundPlane )
	{
		MM_ClipVelocity( mmove->velocity, mmove->groundPlaneNormal, mmove->velocity, OVERCLIP );
	}

	MM_StepSlideMove();

	if( !mm->walking && mml.previous_velocity[ 2 ] >= 0.0f && mm->velocity[ 2 ] <= 0.0f )
	{
		point[ 0 ] = mmove->origin[ 0 ];
		point[ 1 ] = mmove->origin[ 1 ];
		point[ 2 ] = mmove->origin[ 2 ] - 18.0f;

		gi.Trace( &trace, mm->origin, mm->mins, mm->maxs, point, mm->entityNum, mm->tracemask, qtrue, qfalse );

		if( trace.fraction < 1.0f && !trace.allsolid )
		{
			MM_GroundTrace();
			return;
		}
	}
}
