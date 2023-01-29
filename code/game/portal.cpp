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

// portal.cpp: Portals - surfaces that are mirrors or cameras

#include "portal.h"
#include "game.h"

/*QUAKED portal_surface (1 0 1) (-8 -8 -8) (8 8 8)
The portal surface nearest this entity will show a view from the targeted portal_camera, or a mirror view if untargeted.
*/
   
Event EV_Portal_LocateCamera
	( 
	"locatecamera",
	EV_DEFAULT,
   NULL,
   NULL,
   "Locates the camera position.",
   EV_NORMAL
	);

CLASS_DECLARATION( Entity, PortalSurface, "portal_surface" )
	{
      { &EV_Portal_LocateCamera,     &PortalSurface::LocateCamera },
		{ NULL, NULL }
	};

void PortalSurface::LocateCamera
   (
   Event *ev 
   )

   {
	Entity   *owner;
	Entity   *target;
   Vector   dir;

	owner = ( Entity * )G_FindTarget( NULL, Target() );

   if ( !owner )
      {
      // No target, just a mirror
      VectorCopy( edict->s.origin, edict->s.origin2 );
      return;
      }

	// skinNum holds the rotate offset
	edict->s.skinNum = owner->edict->s.skinNum;

	VectorCopy( owner->origin, edict->s.origin2 );

	// see if the portal_camera has a target
	target = ( Entity * )G_FindTarget( NULL, owner->Target() );

	if ( target )
      {
		dir = target->origin - owner->origin;
		dir.normalize();
      setAngles( dir.toAngles() );
	   } 
   else
      {
      setAngles( owner->angles );
		dir = owner->orientation[ 0 ];
	   }
   }

PortalSurface::PortalSurface
   (
   )

   {
   if ( LoadingSavegame )
      {
      // Archive function will setup all necessary data
      return;
      }

   VectorClear( edict->r.mins );
   VectorClear( edict->r.maxs );

	gi.LinkEntity( edict );

	edict->r.svFlags = SVF_PORTAL | SVF_SENDPVS;
	edict->s.eType = ET_PORTAL;

   PostEvent( EV_Portal_LocateCamera, EV_POSTSPAWN );
   }


/*QUAKED portal_camera (1 0 1) (-8 -8 -8) (8 8 8) slowrotate fastrotate
The target for a portal_surface.  You can set either angles or target another entity to determine the direction of view.
"roll" an angle modifier to orient the camera around the target vector;
*/
   
Event EV_Portal_Roll
	( 
	"roll",
	EV_DEFAULT,
   "f",
   "roll",
   "Sets the portal camera's roll.",
   EV_NORMAL
	);

CLASS_DECLARATION( Entity, PortalCamera, "portal_camera" )
	{
      { &EV_Portal_Roll,         &PortalCamera::Roll },
		{ NULL, NULL }
	};

void PortalCamera::Roll   
   (
   Event *ev
   )

   {
   float roll = ev->GetFloat( 1 );

   // skinNum holds the roll
   edict->s.skinNum = roll/360.0f * 256;
   }

PortalCamera::PortalCamera
   (
   )

   {
   if ( LoadingSavegame )
      {
      // Archive function will setup all necessary data
      return;
      }

   VectorClear( edict->r.mins );
   VectorClear( edict->r.maxs );

   // No roll on the camera by default
   edict->s.skinNum = 0;
   
   gi.LinkEntity( edict );
   }
