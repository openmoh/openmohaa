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

// surfaceflags.h: Surface flag parameters for q3map and game
//

#ifndef __SURFACEFLAGS_H__
#define __SURFACEFLAGS_H__

// This file must be identical in the quake and utils directories

// contents flags are seperate bits
// a given brush can contribute multiple content bits

// these definitions also need to be in q_shared.h!

#define	CONTENTS_SOLID			   (1<<0)		// an eye is never valid in a solid
#define	CONTENTS_LAVA			   (1<<3)
#define	CONTENTS_SLIME			   (1<<4)
#define	CONTENTS_WATER			   (1<<5)
#define	CONTENTS_FOG			   (1<<6)

#define  CONTENTS_AREAPORTAL     (1<<15)
#define	CONTENTS_PLAYERCLIP		(1<<16)
#define	CONTENTS_MONSTERCLIP	   (1<<17)
#define	CONTENTS_CAMERACLIP		(1<<18)
#define	CONTENTS_WEAPONCLIP		(1<<19)  // blocks projectiles and weapon attacks as well
#define  CONTENTS_SHOOTABLE_ONLY (1<<20)  // player can walk through this but can shoot it as well
#define	CONTENTS_ORIGIN			(1<<24) 	// removed before bsping an entity
#define	CONTENTS_BODY			   (1<<25)	// should never be on a brush, only in game
#define	CONTENTS_CORPSE			(1<<26)
#define	CONTENTS_DETAIL			(1<<27)	// brushes not used for the bsp
#define	CONTENTS_STRUCTURAL		(1<<28)	// brushes used for the bsp
#define	CONTENTS_TRANSLUCENT	   (1<<29)	// don't consume surface fragments inside
#define	CONTENTS_NODROP			(1<<31)	// don't leave bodies or items (death fog, lava)

#define  CONTENTS_KEEP  (CONTENTS_DETAIL)

#define	SURF_NODAMAGE			( 1<<0 )	// never give falling damage
#define	SURF_SLICK				( 1<<1 )	// effects game physics
#define	SURF_SKY				   ( 1<<2 )	// lighting from environment map
#define	SURF_LADDER				( 1<<3 ) // ladder surface
#define	SURF_NOIMPACT			( 1<<4 ) // don't make missile explosions
#define	SURF_NOMARKS			( 1<<5 ) // don't leave missile marks
#define  SURF_CASTSHADOW      ( 1<<6 ) // used in conjunction with nodraw allows surface to be not drawn but still cast shadows
#define  SURF_NODRAW          ( 1<<7 ) // don't generate a drawsurface at all
#define	SURF_NOLIGHTMAP		( 1<<10 )// surface doesn't need a lightmap
#define  SURF_ALPHASHADOW     ( 1<<11 )// do per-pixel shadow tests based on the texture
#define	SURF_NOSTEPS			( 1<<13 )// no footstep sounds
#define	SURF_NONSOLID			( 1<<14 )// don't collide against curves with this set
#define  SURF_RICOCHET        ( 1<<15 )// ricochet bullets


#define  SURF_TYPE_WOOD       ( 1<<16 )// wood surface
#define  SURF_TYPE_METAL      ( 1<<17 )// metal surface
#define  SURF_TYPE_ROCK       ( 1<<18 )// stone surface
#define  SURF_TYPE_DIRT       ( 1<<19 )// dirt surface
#define  SURF_TYPE_GRILL      ( 1<<20 )// metal grill surface
#define  SURF_TYPE_ORGANIC    ( 1<<21 )// oraganic (grass, loamy dirt)
#define  SURF_TYPE_SQUISHY    ( 1<<22 )// squishy (swamp dirt, flesh)

#define	SURF_NODLIGHT			( 1<<23 )// don't dlight even if solid (solid lava, skies)
#define	SURF_HINT   			( 1<<24 )// choose this plane as a partitioner

#define  SURF_PATCH           ( 1<<29 )
#define  SURF_KEEP            (SURF_PATCH)

#define MASK_SURF_TYPE        (SURF_TYPE_WOOD|SURF_TYPE_METAL|SURF_TYPE_ROCK|SURF_TYPE_DIRT|SURF_TYPE_GRILL|SURF_TYPE_ORGANIC|SURF_TYPE_SQUISHY)

void ParseSurfaceParm( char *token, int * flags, int * contents );

#endif

