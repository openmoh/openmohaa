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

// nature.h:

#include "g_local.h"
#include "trigger.h"

class Emitter : public Entity
   {
   private:
      str  emitterName;
      void setEmitter( str name );
      void EmitterName( Event *ev );
   public:
      CLASS_PROTOTYPE( Emitter );
      Emitter();
      void Archive( Archiver &arc ) override;
   };

inline void Emitter::Archive
	(
	Archiver &arc
	)
   {
   Entity::Archive( arc );

   arc.ArchiveString( &emitterName );
   if ( arc.Loading() )
      {
      setEmitter( emitterName );
      }
   }


class Rain : public Emitter
   {
   private:
      str  rainName;
      void setRainName( str name );
   public:
      CLASS_PROTOTYPE( Rain );
      Rain();
      void Archive( Archiver &arc ) override;
   };

inline void Rain::Archive
	(
	Archiver &arc
	)
   {
   Entity::Archive( arc );

   arc.ArchiveString( &rainName );
   if ( arc.Loading() )
      {
      setRainName( rainName );
      }
   }


class PuffDaddy : public Animate
   {
   private:
      void Touch( Event *ev );
      void Idle( Event *ev );

   public:
      CLASS_PROTOTYPE( PuffDaddy );
      PuffDaddy();
   };

