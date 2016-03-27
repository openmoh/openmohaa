/*
===========================================================================
Copyright (C) 2008 Leszek Godlewski

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

/*
This file does not reference any globals, and has these entry points:

struct terPatchCollide_s *CM_GenerateTerPatchCollide(vec3_t origin, byte heightmap[9][9]);
void CM_TraceThroughTerPatchCollide(traceWork_t *tw, const struct terPatchCollide_s *tc);
qboolean CM_PositionTestInTerPatchCollide(traceWork_t *tw, const struct terPatchCollide_s *tc);
*/

#ifndef CM_TERRAIN_H
#define CM_TERRAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#define TER_QUADS_PER_ROW	8
#define TER_TRIS_PER_PATCH	(TER_QUADS_PER_ROW * TER_QUADS_PER_ROW * 2)
#define TER_PLANES_PER_TRI	5

typedef struct terTriangle_s {
	cplane_t		planes[TER_PLANES_PER_TRI];	// 0 is the surface plane, 3 border planes follow and a cap to give it some finite volume
} terTriangle_t;

typedef struct terPatchCollide_s {
	vec3_t			bounds[2];

	baseshader_t	*shader;

	terTriangle_t	tris[TER_TRIS_PER_PATCH];
} terPatchCollide_t;

typedef struct terrainCollideSquare_s {
	vec4_t plane[ 2 ];
	int eMode;
} terrainCollideSquare_t;

typedef struct terrainCollide_s {
	vec3_t vBounds[ 2 ];
	terrainCollideSquare_t squares[ 8 ][ 8 ];
} terrainCollide_t;

void CM_PrepareGenerateTerrainCollide( void );
void CM_GenerateTerrainCollide( cTerraPatch_t *patch, terrainCollide_t *tc );
int CM_TerrainSquareType( int iTerrainPatch, int i, int j );

#ifdef __cplusplus
}
#endif

#endif // CM_TERRAIN_H
