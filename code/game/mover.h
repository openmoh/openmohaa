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

// mover.h: DESCRIPTION:
// Base class for any object that needs to move to specific locations over a
// period of time.  This class is kept separate from most entities to keep
// class size down for objects that don't need such behavior.
// 

#ifndef __MOVER_H__
#define __MOVER_H__

#include "g_local.h"
#include "entity.h"
#include "trigger.h"

class Mover : public Trigger
	{		
	private:
      Vector				finaldest;
		Vector				angledest;
		Event					*endevent;
		int					moveflags;

	public:
		CLASS_PROTOTYPE( Mover );

								Mover();
		virtual				~Mover();
		void					MoveDone( Event *ev );
		void					MoveTo( Vector tdest, Vector angdest, float tspeed, Event &event );
		void					LinearInterpolate( Vector tdest, Vector angdest, float time, Event &event );
	   virtual void      Archive( Archiver &arc );
   };

inline void Mover::Archive
	(
	Archiver &arc
	)
   {
   Trigger::Archive( arc );

   arc.ArchiveVector( &finaldest );
   arc.ArchiveVector( &angledest );
   arc.ArchiveEventPointer( &endevent );
   arc.ArchiveInteger( &moveflags );
   }

#endif
