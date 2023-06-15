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

// viewthing.h: Actor code for the viewthing. 
//

#ifndef __VIEWTHING_H__
#define __VIEWTHING_H__

#include "animate.h"

class ViewMaster : public Listener
	{
	public:
      CLASS_PROTOTYPE( ViewMaster );

      EntityPtr current_viewthing;

           ViewMaster();
      void Init( void );
		void Next( Event *ev );
		void Prev( Event *ev );
		void DeleteAll( Event *ev );
		void Spawn( Event *ev );
		void SetModelEvent( Event *ev );
		void PassEvent( Event *ev );

      void Archive( Archiver &arc ) override;
	};

inline void ViewMaster::Archive
	(
	Archiver &arc
	)

   {
   Listener::Archive( arc );

   arc.ArchiveSafePointer( &current_viewthing );
   }

extern ViewMaster Viewmodel;

class Viewthing : public Animate
	{
	public:
      CLASS_PROTOTYPE( Viewthing );

      int      animstate;
      int      frame;
      int      lastframe;
      Vector   baseorigin;
      byte     origSurfaces[MAX_MODEL_SURFACES];

							   Viewthing();
		void					UpdateCvars( qboolean quiet = false );
		void					PrintTime( Event *ev );
		void					ThinkEvent( Event *ev );
		void					LastFrameEvent( Event *ev );
		void					ToggleAnimateEvent( Event *ev );
		void					SetModelEvent( Event *ev );
		void					NextFrameEvent( Event *ev );
		void					PrevFrameEvent( Event *ev );
		void					NextAnimEvent( Event *ev );
		void					PrevAnimEvent( Event *ev );
		void					ScaleUpEvent( Event *ev );
		void					ScaleDownEvent( Event *ev );
		void					SetScaleEvent( Event *ev );
		void					SetYawEvent( Event *ev );
		void					SetPitchEvent( Event *ev );
		void					SetRollEvent( Event *ev );
		void					SetAnglesEvent( Event *ev );
      void              AttachModel( Event *ev );
      void              Delete( Event *ev );
      void              DetachAll( Event *ev );
      void              ChangeOrigin( Event *ev );
      void              SaveSurfaces( Event *ev );
      void              SetAnim( Event *ev );

      void Archive( Archiver &arc ) override;
	};

inline void Viewthing::Archive
	(
	Archiver &arc
	)

   {
   Animate::Archive( arc );

   arc.ArchiveInteger( &animstate );
   arc.ArchiveInteger( &frame );
   arc.ArchiveInteger( &lastframe );
   arc.ArchiveVector( &baseorigin );
   arc.ArchiveRaw( origSurfaces, sizeof( origSurfaces ) );
   }

#endif /* viewthing.h */
