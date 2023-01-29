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

// decals.cpp: Decal entities

#include "decals.h"
#include "level.h"

CLASS_DECLARATION( Entity, Decal, NULL )
	{
		{ NULL, NULL }
	};

Decal::Decal
   (
   )

   {
   edict->s.eType = ET_DECAL;
   edict->s.modelindex  = 1;	      // must be non-zero
   PostEvent( EV_Remove, FRAMETIME );
   }

void Decal::setDirection
   (
   Vector dir
   )

   {
   edict->s.surfaces[0] = DirToByte( dir );
   }

void Decal::setShader
   (
   str decal_shader
   )

   {
   str temp_shader;

   shader = decal_shader;
   edict->s.tag_num = gi.imageindex( shader.c_str() );

	temp_shader = shader + ".spr";
	CacheResource( temp_shader );
   }

void Decal::setOrientation
   (
   str deg
   )

   {
   Vector ang;

   if ( !deg.icmp( "random" ) )
      ang[2] = random() * 360;
   else
      ang[2] = atof( deg );   

   setAngles( ang );
   }

void Decal::setRadius 
   (
   float rad
   )

   {
   edict->s.scale = rad;
   }

