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
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

// su44: use qfiles.h from OpenMoHAA codebase
#include "../../../qcommon/qfiles.h"
#include "../../../qcommon/surfaceflags.h"

// su44: q3map-only limits added for MoHAA bsp structures
// These values are exacly the same as in MoHAA's q3map
#define MAX_MAP_SIDEEQATIONS 131072
#define MAX_TERRAIN_PATCHES 2048
#define MAX_LEAF_STATICMODELS 0x8000
#define MAX_STATIC_MODELS 4096
// these are from MoHlight
#define MAX_MAP_LIGHTGRIDDATA 4194304
// added by me...
#define MAX_MAP_SPHERELIGHTS 8192
#define MAX_MAP_STATICMODELVERTCOLORS 0x80000
// Note: there are 6163 lightdefs on mohdm1
#define MAX_LIGHTDEFS 16384

extern	int			nummodels;
extern	dmodel_t	dmodels[MAX_MAP_MODELS];

extern	int			numShaders;
extern	dshader_t	dshaders[MAX_MAP_MODELS];

extern	int			entdatasize;
extern	char		dentdata[MAX_MAP_ENTSTRING];

extern	int			numleafs;
extern	dleaf_t		dleafs[MAX_MAP_LEAFS];

extern	int			numplanes;
extern	dplane_t	dplanes[MAX_MAP_PLANES];

extern	int			numnodes;
extern	dnode_t		dnodes[MAX_MAP_NODES];

extern	int			numleafsurfaces;
extern	int			dleafsurfaces[MAX_MAP_LEAFFACES];

extern	int			numleafbrushes;
extern	int			dleafbrushes[MAX_MAP_LEAFBRUSHES];

extern	int			numbrushes;
extern	dbrush_t	dbrushes[MAX_MAP_BRUSHES];

extern	int			numbrushsides;
extern	dbrushside_t	dbrushsides[MAX_MAP_BRUSHSIDES];

extern	int			numLightBytes;
extern	byte		lightBytes[MAX_MAP_LIGHTING];

extern	int			numGridPoints;
extern	byte		gridData[MAX_MAP_LIGHTGRID];

extern	int			numVisBytes;
extern	byte		visBytes[MAX_MAP_VISIBILITY];

extern	int			numDrawVerts;
extern	drawVert_t	drawVerts[MAX_MAP_DRAW_VERTS];

extern	int			numDrawIndexes;
extern	int			drawIndexes[MAX_MAP_DRAW_INDEXES];

extern	int			numDrawSurfaces;
extern	dsurface_t	drawSurfaces[MAX_MAP_DRAW_SURFS];

extern	int			numFogs;
extern	dfog_t		dfogs[MAX_MAP_FOGS];

extern	int			numLeafStaticModels;
extern	short			dleafStaticModels[MAX_LEAF_STATICMODELS];

extern	int			numStaticModels;
extern	dstaticModel_t staticModels[MAX_STATIC_MODELS];

extern	int			numStaticModelVertColors;
extern	byte		staticModelVertColors[MAX_MAP_STATICMODELVERTCOLORS];

extern	int			numSideEquations;
extern	dsideEquation_t sideEquations[MAX_MAP_SIDEEQATIONS];

extern	int			numSphereLights;
extern	dspherel_t	sphereLights[MAX_MAP_SPHERELIGHTS];

extern	int			numTerraPatches;
extern	dterPatch_t	terraPatches[MAX_TERRAIN_PATCHES];

extern	int			numLightDefs;
extern	dlightdef_t	lightDefs[MAX_LIGHTDEFS];

void	LoadBSPFile( const char *filename );
void	WriteBSPFile( const char *filename );
void	PrintBSPFileSizes( void );

//===============


typedef struct epair_s {
	struct epair_s	*next;
	char	*key;
	char	*value;
} epair_t;

typedef struct {
	vec3_t		origin;
	struct bspbrush_s	*brushes;
	struct parseMesh_s	*patches;
	int			firstDrawSurf;
	epair_t		*epairs;
} entity_t;

extern	int			num_entities;
extern	entity_t	entities[MAX_MAP_ENTITIES];

void	ParseEntities( void );
void	UnparseEntities( void );

void 	SetKeyValue( entity_t *ent, const char *key, const char *value );
const char 	*ValueForKey( const entity_t *ent, const char *key );
// will return "" if not present

vec_t	FloatForKey( const entity_t *ent, const char *key );
void 	GetVectorForKey( const entity_t *ent, const char *key, vec3_t vec );

epair_t *ParseEpair( void );

void	PrintEntity( const entity_t *ent );

