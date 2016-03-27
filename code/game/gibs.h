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

// gibs.h: Gibs - nuff said

#ifndef __GIBS_H__
#define __GIBS_H__

#include "g_local.h"
#include "mover.h"

#define NO_FINAL_PITCH  -1000

class Gib : public Mover
   {
   private:
      int      sprayed;
      float    scale;
      Mover    *blood;
		str		blood_splat_name;
		float		blood_splat_size;
		str		blood_spurt_name;
		float		final_pitch;
		float		next_bleed_time;
   public:
      CLASS_PROTOTYPE( Gib );

      qboolean    fadesplat;
      Gib();
		~Gib();
      Gib( str name, qboolean blood_trail, str bloodtrailname="", str bloodspurtname="", str bloodsplatname="", 
				float blood_splat_size = 8, float pitch=NO_FINAL_PITCH );
      void     SetVelocity( float health );
      void     SprayBlood( Vector start );
      void     Throw( Event *ev );
      void     Splat( Event *ev );
      void     Stop( Event *ev );
		void     Damage( Event *ev );
      void     ClipGibVelocity( void );
	   virtual void Archive( Archiver &arc );
	};

inline void Gib::Archive
	(
	Archiver &arc
	)
   {
   Entity::Archive( arc );

   arc.ArchiveBoolean( &sprayed );
   arc.ArchiveFloat( &scale );
   arc.ArchiveObjectPointer( ( Class ** )&blood );
   arc.ArchiveString( &blood_splat_name );
   arc.ArchiveFloat( &blood_splat_size );
   arc.ArchiveString( &blood_spurt_name );
   arc.ArchiveFloat( &final_pitch );
   arc.ArchiveBoolean( &fadesplat );
	arc.ArchiveFloat( &next_bleed_time );
   }


void CreateGibs
   ( 
   Entity * ent, 
   float damage = -50, 
   float scale = 1.0f, 
   int num = 1, 
   const char * modelname = NULL
   );

extern Event EV_ThrowGib;

#endif // gibs.h
