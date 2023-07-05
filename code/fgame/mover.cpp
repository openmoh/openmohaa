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

// mover.cpp: Base class for any object that needs to move to specific locations over a
// period of time.  This class is kept separate from most entities to keep
// class size down for objects that don't need such behavior.
//

#include "g_local.h"
#include "entity.h"
#include "trigger.h"
#include "mover.h"
#include "level.h"
#include "g_phys.h"

#define MOVE_ANGLES 1
#define MOVE_ORIGIN 2

CLASS_DECLARATION( Trigger, Mover, "mover" )
	{
	   { &EV_MoveDone,         &Mover::MoveDone },
		{ NULL, NULL }
	};


Mover::Mover()
   {
   endevent = NULL;
   }

Mover::~Mover()
	{
	}

void Mover::MoveDone
	(
	Event *ev
	)
{
	Event * event;
	Vector move;
	Vector amove;

	// zero out the movement
	if( moveflags & MOVE_ANGLES )
	{
		avelocity = vec_zero;
		amove = angledest - localangles;
	}
	else
	{
		amove = vec_zero;
	}

	if( moveflags & MOVE_ORIGIN )
	{
		velocity = vec_zero;
		move = finaldest - localorigin;
	}
	else
	{
		move = vec_zero;
	}

	if( !G_PushMove( this, move, amove ) )
	{
		// Delay finish till we can move into the final position
		PostEvent( EV_MoveDone, FRAMETIME );
		return;
	}

	//
	// After moving, set origin to exact final destination
	//
	if( moveflags & MOVE_ORIGIN )
	{
		setLocalOrigin( finaldest );
	}

	if( moveflags & MOVE_ANGLES )
	{
		localangles = angledest;

		if( ( localangles.x >= 360 ) || ( localangles.x < 0 ) )
		{
			localangles.x -= ( ( int )localangles.x / 360 ) * 360;
		}
		if( ( localangles.y >= 360 ) || ( localangles.y < 0 ) )
		{
			localangles.y -= ( ( int )localangles.y / 360 ) * 360;
		}
		if( ( localangles.z >= 360 ) || ( localangles.z < 0 ) )
		{
			localangles.z -= ( ( int )localangles.z / 360 ) * 360;
		}
	}

	event = endevent;
	endevent = NULL;

	if( event )
		ProcessEvent( event );
}

/*
=============
MoveTo

calculate self.velocity and self.nextthink to reach dest from
self.origin traveling at speed
===============
*/
void Mover::MoveTo( Vector tdest, Vector angdest, float tspeed, Event& event )
{
	Vector vdestdelta;
	Vector angdestdelta;
	float  len;
	float  traveltime;

	assert( tspeed >= 0.0f );

	if( !tspeed )
	{
		error( "MoveTo", "No speed is defined!" );
	}

	if( tspeed < 0.0f )
	{
		error( "MoveTo", "Speed is negative!" );
	}

	// Cancel previous moves
	CancelEventsOfType( EV_MoveDone );

	moveflags = 0;

	if( endevent )
	{
		delete endevent;
	}

	endevent = new Event( event );

	finaldest = tdest;
	angledest = angdest;

	if( finaldest != localorigin )
	{
		moveflags |= MOVE_ORIGIN;
	}
	if( angledest != localangles )
	{
		moveflags |= MOVE_ANGLES;
	}

	if( !moveflags )
	{
		// stop the object from moving
		velocity = vec_zero;
		avelocity = vec_zero;

		// post the event so we don't wait forever
		PostEvent( EV_MoveDone, FRAMETIME );
		return;
	}

	// set destdelta to the vector needed to move
	vdestdelta = tdest - localorigin;
	angdestdelta[ 0 ] = angledist( angdest[ 0 ] - localangles[ 0 ] );
	angdestdelta[ 1 ] = angledist( angdest[ 1 ] - localangles[ 1 ] );
	angdestdelta[ 2 ] = angledist( angdest[ 2 ] - localangles[ 2 ] );

	if( tdest == localorigin )
	{
		// calculate length of vector based on angles
		len = angdestdelta.length();
	}
	else
	{
		// calculate length of vector based on distance
		len = vdestdelta.length();
	}

	// divide by speed to get time to reach dest
	traveltime = len / tspeed;

	if( traveltime < level.frametime )
	{
		traveltime = level.frametime;
		vdestdelta = vec_zero;
		angdestdelta = vec_zero;
	}

	// scale the destdelta vector by the time spent traveling to get velocity
	if( moveflags & MOVE_ORIGIN )
	{
		velocity = vdestdelta * ( 1.0f / traveltime );
	}

	if( moveflags & MOVE_ANGLES )
	{
		avelocity = angdestdelta * ( 1.0f / traveltime );
	}

	PostEvent( EV_MoveDone, traveltime );
}

/*
=============
LinearInterpolate
===============
*/
void Mover::LinearInterpolate
	(
	Vector tdest,
	Vector angdest,
	float time,
	Event &event
	)
{
	Vector vdestdelta;
	Vector angdestdelta;
	float t;

	if( endevent )
	{
		delete endevent;
	}
	endevent = new Event( event );
	finaldest = tdest;
	angledest = angdest;

	// Cancel previous moves
	CancelEventsOfType( EV_MoveDone );

	// Quantize to FRAMETIME
	if( time < FRAMETIME )
	{
		time = FRAMETIME;
	}

	moveflags = 0;
	t = 1 / time;
	// scale the destdelta vector by the time spent traveling to get velocity
	if( finaldest != localorigin )
	{
		vdestdelta = tdest - localorigin;
		velocity = vdestdelta * t;
		moveflags |= MOVE_ORIGIN;
	}

	if( angledest != localangles )
	{
		angdestdelta = angdest - localangles;
		avelocity = angdestdelta * t;
		moveflags |= MOVE_ANGLES;
	}

	if( g_bBeforeThinks )
		time -= FRAMETIME;

	PostEvent( EV_MoveDone, time );
}

