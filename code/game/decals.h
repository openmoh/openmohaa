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

// decals.h: Decal entities

#ifndef __DECAL_H__
#define __DECAL_H__

#include "g_local.h"
#include "entity.h"

class Decal : public Entity
	{       
   private:
      str            shader;

	public:
      CLASS_PROTOTYPE( Decal );

	                  Decal();
      void           setDirection( Vector dir );
      void           setShader( str shader );
      void           setOrientation( str deg );
      void           setRadius( float rad );
      void Archive( Archiver &arc ) override;
	};

inline void Decal::Archive
	(
	Archiver &arc
	)
   {
   Entity::Archive( arc );

   arc.ArchiveString( &shader );
   if ( arc.Loading() )
      {
      setShader( shader );
      }
   }

#endif // __DECAL_H__
