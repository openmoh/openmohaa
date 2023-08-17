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

#include "q_shared.h"
#include "qcommon.h"
#include "cm_polylib.h"
#include "cm_terrain.h"

#define	MAX_SUBMODELS			1024
#define	BOX_MODEL_HANDLE		(MAX_SUBMODELS-1)
#define CAPSULE_MODEL_HANDLE	510


typedef struct {
	cplane_t	*plane;
	int			children[2];		// negative numbers are leafs
} cNode_t;

typedef struct {
	int			cluster;
	int			area;

	intptr_t	firstLeafBrush;
	int			numLeafBrushes;

	intptr_t	firstLeafSurface;
	int			numLeafSurfaces;

	//added for mohaa
	int			firstLeafTerrain;
	int			numLeafTerrains;
} cLeaf_t;

typedef struct cfencemask_s {
	char name[ 64 ];
	int iWidth;
	int iHeight;
	byte *pData;
	struct cfencemask_s *pNext;
} cfencemask_t;

typedef struct {
	char shader[ 64 ];
	int surfaceFlags;
	int contentFlags;
	cfencemask_t *mask;
} cshader_t;

typedef struct cmodel_s {
	vec3_t		mins, maxs;
	cLeaf_t		leaf;			// submodels don't reference the main tree
} cmodel_t;

typedef struct {
	cplane_t			*plane;
	int					surfaceFlags;
	int					shaderNum;

	dsideequation_t		*pEq;
} cbrushside_t;

typedef struct {
	int			shaderNum;		// the shader that determined the contents
	int			contents;
	vec3_t		bounds[2];
	int			numsides;
	cbrushside_t	*sides;
	int			checkcount;		// to avoid repeated testings
} cbrush_t;


typedef struct {
	int			checkcount;				// to avoid repeated testings
	int			surfaceFlags;
	int			contents;

	int			shaderNum;
	int			subdivisions;

	struct patchCollide_s	*pc;
} cPatch_t;

typedef struct {
	int checkcount;
	int surfaceFlags;
	int contents;
	int shaderNum;
	terrainCollide_t tc;
} cTerrain_t;

typedef struct {
	int			floodnum;
	int			floodvalid;
} cArea_t;

// IneQuation
typedef struct {
	int			checkcount;				// to avoid repeated testings

	struct terPatchCollide_s	*tc;
} cterPatch_t;

typedef struct {
	char			name[ MAX_QPATH ];

	int				numShaders;
	cfencemask_t	*fencemasks;
	cshader_t		*shaders;

	int				numSideEquations;
	dsideequation_t	*sideequations;

	int				numBrushSides;
	cbrushside_t	*brushsides;

	int				numPlanes;
	cplane_t		*planes;

	int				numNodes;
	cNode_t			*nodes;

	int				numLeafs;
	cLeaf_t			*leafs;

	int				numLeafBrushes;
	int				*leafbrushes;

	int				numLeafSurfaces;
	int				*leafsurfaces;

	int				numLeafTerrains;
	cTerrain_t		**leafterrains;

	int				numSubModels;
	cmodel_t		*cmodels;

	int				numBrushes;
	cbrush_t		*brushes;

	int				numClusters;
	int				clusterBytes;
	byte			*visibility;
	qboolean		vised;			// if false, visibility is just a single cluster of ffs

	int				numEntityChars;
	char			*entityString;

	int				numAreas;
	cArea_t			*areas;
	int				*areaPortals;	// [ numAreas*numAreas ] reference counts

	int				numSurfaces;
	cPatch_t		**surfaces;			// non-patches will be NULL

	// IneQuation
	int				numTerrain;
	cTerrain_t		*terrain;

	int				floodvalid;
	int				checkcount;					// incremented on each trace
} clipMap_t;


// keep 1/8 unit away to keep the position valid before network snapping
// and to avoid various numeric issues
#define	SURFACE_CLIP_EPSILON	(0.125)

// cm_test.c

// Used for oriented capsule collision detection
typedef struct
{
	qboolean	use;
	float		radius;
	vec3_t		offset;
} sphere_t;

typedef struct {
	vec3_t		start;
	vec3_t		end;
	vec3_t		size[2];	// size of the box being swept through the model
	vec3_t		offsets[8];	// [signbits][x] = either size[0][x] or size[1][x]
	float		maxOffset;	// longest corner length from origin
	vec3_t		extents;	// greatest of abs(size[0]) and abs(size[1])
	vec3_t		bounds[2];	// enclosing box of start and end surrounding by size
	float		height;
	float		radius;
	int			contents;	// ored contents of the model tracing through
	qboolean	isPoint;	// optimized case
	trace_t		trace;		// returned from trace call
} traceWork_t;

typedef struct leafList_s {
	int		count;
	int		maxcount;
	qboolean	overflowed;
	int		*list;
	vec3_t	bounds[2];
	int		lastLeaf;		// for overflows where each leaf can't be stored individually
	void	(*storeLeafs)( struct leafList_s *ll, int nodenum );
} leafList_t;

extern	clipMap_t	cm;
extern	int			c_pointcontents;
extern	int			c_traces, c_brush_traces, c_patch_traces, c_terrain_patch_traces;
extern	cvar_t		*cm_noAreas;
extern	cvar_t		*cm_noCurves;
extern	cvar_t		*cm_playerCurveClip;
extern	cvar_t		*cm_FCMcacheall;
extern	cvar_t		*cm_FCMdebug;
extern	cvar_t		*cm_ter_usesphere;
extern	sphere_t	sphere;


int CM_BoxBrushes( const vec3_t mins, const vec3_t maxs, cbrush_t **list, int listsize );

void CM_StoreLeafs( leafList_t *ll, int nodenum );
void CM_StoreBrushes( leafList_t *ll, int nodenum );

void CM_BoxLeafnums_r( leafList_t *ll, int nodenum );

cmodel_t	*CM_ClipHandleToModel( clipHandle_t handle );
qboolean CM_BoundsIntersect( const vec3_t mins, const vec3_t maxs, const vec3_t mins2, const vec3_t maxs2 );
qboolean CM_BoundsIntersectPoint( const vec3_t mins, const vec3_t maxs, const vec3_t point );

// cm_patch.c

qboolean CM_PlaneFromPoints( vec4_t plane, vec3_t a, vec3_t b, vec3_t c );
struct patchCollide_s	*CM_GeneratePatchCollide( int width, int height, vec3_t *points, float subdivisions );
void CM_TraceThroughPatchCollide( traceWork_t *tw, const struct patchCollide_s *pc );
qboolean CM_PositionTestInPatchCollide( traceWork_t *tw, const struct patchCollide_s *pc );
void CM_ClearLevelPatches( void );

// cm_terrain.c
struct terPatchCollide_s *CM_GenerateTerPatchCollide(vec3_t origin, byte heightmap[9][9], baseshader_t *shader);
void CM_TraceThroughTerrainCollide(traceWork_t *tw, terrainCollide_t *tc);
qboolean CM_PositionTestInTerrainCollide( traceWork_t *tw, terrainCollide_t *tc );
qboolean CM_SightTracePointThroughTerrainCollide( void );
qboolean CM_SightTraceThroughTerrainCollide( traceWork_t *tw, terrainCollide_t *tc );

// cm_fencemask.c
cfencemask_t *CM_GetFenceMask( const char *szMaskName );
qboolean CM_TraceThroughFence( traceWork_t *tw, cbrush_t *brush, cbrushside_t *side, float fTraceFraction );
