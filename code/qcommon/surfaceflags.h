/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
//
// This file must be identical in the quake and utils directories

// contents flags are seperate bits
// a given brush can contribute multiple content bits

// these definitions also need to be in q_shared.h!
// IneQuation was here

#define	CONTENTS_SOLID			1		// an eye is never valid in a solid
#define CONTENTS_LADDER			2
#define	CONTENTS_LAVA			0x0008
#define	CONTENTS_SLIME			0x0010
#define	CONTENTS_WATER			0x0020
#define	CONTENTS_FOG			0x0040

#define CONTENTS_NOTTEAM1		0x0080
#define CONTENTS_NOTTEAM2		0x0100
#define CONTENTS_BBOX			0x0100
#define CONTENTS_NOBOTCLIP		0x0200
#define CONTENTS_UNKNOWN1		0x0400
#define CONTENTS_UNKNOWN2		0x0800

#define CONTENTS_UNKNOWN3		0x1000
#define CONTENTS_FENCE			0x2000
#define CONTENTS_UNKNOWN4		0x4000
#define	CONTENTS_AREAPORTAL		0x8000
// su44: this is only for bspc .aas file compiler (bot navigation info)
// I dont think its correct....
#define CONTENTS_CLUSTERPORTAL CONTENTS_AREAPORTAL

#define MASK_WOMBAT5				// su44: wtf is that?

#define	CONTENTS_PLAYERCLIP		0x10000
#define	CONTENTS_MONSTERCLIP	0x20000
#define	CONTENTS_WEAPONCLIP		0x40000
#define	CONTENTS_VEHICLECLIP	0x80000
#define MASK_CLIP				(CONTENTS_PLAYERCLIP|CONTENTS_MONSTERCLIP|CONTENTS_WEAPONCLIP|CONTENTS_VEHICLECLIP)

#define CONTENTS_SHOOTONLY		0x100000
#define CONTENTS_DONOTENTER		0x200000
#define CONTENTS_BOTCLIP		0x400000
#define CONTENTS_MOVER			0x800000
#define MASK_WOMBAT1			(CONTENTS_SHOOTONLY|CONTENTS_DONOTENTER|CONTENTS_BOTCLIP|CONTENTS_MOVER)

#define	CONTENTS_ORIGIN			0x1000000	// removed before bsping an entity
#define	CONTENTS_BODY			0x2000000	// should never be on a brush, only in game
#define	CONTENTS_CORPSE			0x4000000
#define	CONTENTS_DETAIL			0x8000000	// brushes not used for the bsp
#define MASK_WOMBAT2			(CONTENTS_ORIGIN|CONTENTS_BODY|CONTENTS_CORPSE|CONTENTS_DETAIL)

#define	CONTENTS_STRUCTURAL		0x10000000	// brushes used for the bsp
#define	CONTENTS_TRANSLUCENT	0x20000000	// don't consume surface fragments inside
#define	CONTENTS_TRIGGER		0x40000000
#define	CONTENTS_NODROP			0x80000000	// don't leave bodies or items (death fog, lava)

#define MASK_FOOTSTEP			0x42022901
#define MASK_TREADMARK			0x42012B01
#define MASK_WOMBAT				(MASK_WOMBAT2|MASK_WOMBAT1|MASK_CLIP|MASK_WOMBAT5)

#define	SURF_NODAMAGE			0x1		// never give falling damage
#define	SURF_SLICK				0x2		// effects game physics
#define	SURF_SKY				0x4		// lighting from environment map
#define	SURF_LADDER				0x8
#define	SURF_NOIMPACT			0x10	// don't make missile explosions
#define	SURF_NOMARKS			0x20	// don't leave missile marks
#define	SURF_CASTSHADOW			0x40
#define	SURF_NODRAW				0x80	// don't generate a drawsurface at all
#define SURF_HINT				0x100	// make a primary bsp splitter
#define	SURF_ALPHASHADOW		0x200	// do per-pixel light shadow casting in q3map
#define	SURF_NOSTEPS			0x400	// no footstep sounds
#define	SURF_NONSOLID			0x800	// don't collide against curves with this set
#define	SURF_UNKNOWN1			0x1000	// IneQuation: WTF?
#define	SURF_PAPER				0x2000	// paper effects
#define	SURF_WOOD				0x4000	// wood effects
#define	SURF_METAL				0x8000	// metal effects
#define	SURF_ROCK				0x10000	// rock effects
#define	SURF_DIRT				0x20000	// dirt effects
#define SURF_GRILL				0x40000 // metal grill (grate)
#define SURF_GRASS				0x80000	// grass effects
#define SURF_MUD				0x100000	// mud effects
#define SURF_PUDDLE				0x200000	// puddle effects
#define SURF_GLASS				0x400000	// glass effects
#define SURF_GRAVEL				0x800000	// gravel effects
#define SURF_SAND				0x1000000	// sand effects
#define SURF_FOLIAGE			0x2000000	// foliage effects
#define SURF_SNOW				0x4000000	// snow effects
#define SURF_CARPET				0x8000000	// carpet effects
#define SURF_BACKSIDE			0x10000000	// su44: backside
#define SURF_NODLIGHT			0x20000000	// don't dlight even if solid (solid lava, skies)
#define	SURF_NOLIGHTMAP			0x40000000	// surface doesn't need a lightmap
#define SURF_UNKNOWN3			0x80000000	// IneQuation: WTF?

#define	SURF_FLESH				0x40	// make flesh sounds and effects
#define	SURF_POINTLIGHT			0x800	// generate lighting info at vertexes
#define	SURF_METALSTEPS			0x1000	// clanking footsteps
#define	SURF_LIGHTFILTER		0x8000	// act as a light filter during q3map -light
#define SURF_DUST				0x40000 // leave a dust trail when walking on this surface

