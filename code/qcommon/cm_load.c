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
// cmodel.c -- model loading

#include "cm_local.h"
#include "../client/client.h"

#ifdef BSPC

#include "../bspc/l_qfiles.h"

void SetPlaneSignbits (cplane_t *out) {
	int	bits, j;

	// for fast box on planeside test
	bits = 0;
	for (j=0 ; j<3 ; j++) {
		if (out->normal[j] < 0) {
			bits |= 1<<j;
		}
	}
	out->signbits = bits;
}
#endif //BSPC

// to allow boxes to be treated as brush models, we allocate
// some extra indexes along with those needed by the map
#define	BOX_BRUSHES		1
#define	BOX_SIDES		6
#define	BOX_LEAFS		2
#define	BOX_PLANES		12

#define	LL(x) x=LittleLong(x)


clipMap_t	cm;
int			c_pointcontents;
int			c_traces, c_brush_traces, c_patch_traces, c_terrain_patch_traces;


byte		*cmod_base;

#ifndef BSPC
cvar_t		*cm_noAreas;
cvar_t		*cm_noCurves;
cvar_t		*cm_playerCurveClip;
cvar_t		*cm_FCMcacheall;
cvar_t		*cm_FCMdebug;
cvar_t		*cm_ter_usesphere;
#endif

cmodel_t	box_model;
cplane_t	*box_planes;
cbrush_t	*box_brush;

static int numleafbrushes;
static int g_iNumSideEquations;



void	CM_InitBoxHull (void);
void	CM_FloodAreaConnections (void);


/*
===============================================================================

					MAP LOADING

===============================================================================
*/

/*
=================
CMod_LoadShaders
=================
*/
void CMod_LoadShaders( gamelump_t *l, int **shaderSubdivisions ) {
	dshader_t		*in;
	cshader_t		*out;
	int				i, count;

	*shaderSubdivisions = NULL;

	in = ( dshader_t * )l->buffer;
	if ( l->length % sizeof( dshader_t ) ) {
		Com_Error( ERR_DROP, "CM_LoadMap: funny lump size in %s", cm.name );
	}
	count = l->length / sizeof( dshader_t );

	if (count < 1) {
		Com_Error (ERR_DROP, "Map with no shaders");
	}

	cm.shaders = Hunk_Alloc( count * sizeof( *cm.shaders ) );
	cm.numShaders = count;
	cm.fencemasks = NULL;

	*shaderSubdivisions = Hunk_AllocateTempMemory( count * sizeof( *shaderSubdivisions ) );

	out = cm.shaders;
	for ( i=0 ; i<count ; i++, in++, out++ ) {
		strcpy( out->shader, in->shader );
		out->contentFlags = LittleLong( in->contentFlags );
		out->surfaceFlags = LittleLong( in->surfaceFlags );
		out->mask = CM_GetFenceMask( in->fenceMaskImage );
		( *shaderSubdivisions )[ i ] = LittleLong( in->subdivisions );
	}
}


/*
=================
CMod_LoadSubmodels
=================
*/
void CMod_LoadSubmodels( gamelump_t *l ) {
	dmodel_t	*in;
	cmodel_t	*out;
	int			i, j, count;
	int			*indexes;

	in = ( dmodel_t * )l->buffer;
	if (l->length % sizeof(*in))
		Com_Error( ERR_DROP, "CM_LoadMap: funny lump size in %s", cm.name );
	count = l->length / sizeof(*in);

	if (count < 1)
		Com_Error (ERR_DROP, "Map with no models");
	cm.cmodels = Hunk_Alloc( count * sizeof( *cm.cmodels ) );
	cm.numSubModels = count;

	if ( count > MAX_SUBMODELS ) {
		Com_Error( ERR_DROP, "MAX_SUBMODELS exceeded" );
	}

	for ( i=0 ; i<count ; i++, in++, out++)
	{
		out = &cm.cmodels[i];

		for (j=0 ; j<3 ; j++)
		{	// spread the mins / maxs by a pixel
			out->mins[j] = LittleFloat (in->mins[j]) - 1;
			out->maxs[j] = LittleFloat (in->maxs[j]) + 1;
		}

		if ( i == 0 ) {
			continue;	// world model doesn't need other info
		}

		// make a "leaf" just to hold the model's brushes and surfaces
		out->leaf.numLeafBrushes = LittleLong( in->numBrushes );
		if( out->leaf.numLeafBrushes )
		{
			indexes = Hunk_Alloc( out->leaf.numLeafBrushes * 4 );
			out->leaf.firstLeafBrush = indexes - cm.leafbrushes;
			for( j = 0; j < out->leaf.numLeafBrushes; j++ ) {
				indexes[ j ] = LittleLong( in->firstBrush ) + j;
			}
		}
		else
		{
			out->leaf.firstLeafBrush = -1;
		}

		out->leaf.numLeafSurfaces = LittleLong( in->numSurfaces );
		if( out->leaf.numLeafSurfaces )
		{
			indexes = Hunk_Alloc( out->leaf.numLeafSurfaces * 4 );
			out->leaf.firstLeafSurface = indexes - cm.leafsurfaces;
			for( j = 0; j < out->leaf.numLeafSurfaces; j++ ) {
				indexes[ j ] = LittleLong( in->firstSurface ) + j;
			}
		}
		else
		{
			out->leaf.numLeafSurfaces = -1;
		}
	}
}


/*
=================
CMod_LoadNodes

=================
*/
void CMod_LoadNodes( gamelump_t *l ) {
	dnode_t		*in;
	int			child;
	cNode_t		*out;
	int			i, j, count;

	in = ( dnode_t * )l->buffer;
	if (l->length % sizeof(*in))
		Com_Error( ERR_DROP, "CM_LoadMap: funny lump size in %s", cm.name );
	count = l->length / sizeof(*in);

	if (count < 1)
		Com_Error (ERR_DROP, "Map has no nodes");
	cm.nodes = Hunk_Alloc( count * sizeof( *cm.nodes ) );
	cm.numNodes = count;

	out = cm.nodes;

	for (i=0 ; i<count ; i++, out++, in++)
	{
		out->plane = cm.planes + LittleLong( in->planeNum );
		for (j=0 ; j<2 ; j++)
		{
			child = LittleLong (in->children[j]);
			out->children[j] = child;
		}
	}
}

/*
=================
CM_BoundBrush

=================
*/
void CM_BoundBrush( cbrush_t *b ) {
	b->bounds[0][0] = -b->sides[0].plane->dist;
	b->bounds[1][0] = b->sides[1].plane->dist;

	b->bounds[0][1] = -b->sides[2].plane->dist;
	b->bounds[1][1] = b->sides[3].plane->dist;

	b->bounds[0][2] = -b->sides[4].plane->dist;
	b->bounds[1][2] = b->sides[5].plane->dist;
}


/*
=================
CMod_LoadBrushes

=================
*/
void CMod_LoadBrushes( gamelump_t *l ) {
	dbrush_t	*in;
	cbrush_t	*out;
	int			i, count;

	in = ( dbrush_t * )l->buffer;
	if (l->length % sizeof(*in)) {
		Com_Error( ERR_DROP, "CM_LoadMap: funny lump size in %s", cm.name );
	}
	count = l->length / sizeof(*in);

	cm.brushes = Hunk_Alloc( ( BOX_BRUSHES + count ) * sizeof( *cm.brushes ) );
	cm.numBrushes = count;

	out = cm.brushes;

	for ( i=0 ; i<count ; i++, out++, in++ ) {
		out->sides = cm.brushsides + LittleLong(in->firstSide);
		out->numsides = LittleLong(in->numSides);

		out->shaderNum = LittleLong( in->shaderNum );
		if ( out->shaderNum < 0 || out->shaderNum >= cm.numShaders ) {
			Com_Error( ERR_DROP, "CMod_LoadBrushes: bad shaderNum: %i", out->shaderNum );
		}
		out->contents = cm.shaders[out->shaderNum].contentFlags;

		CM_BoundBrush( out );
	}

}

/*
=================
CMod_LoadLeafs
=================
*/
void CMod_LoadLeafs( gamelump_t *l )
{
	int			i;
	cLeaf_t		*out;
	dleaf_t 	*in;
	int			count;

	in = ( dleaf_t * )l->buffer;
	if( l->length % sizeof( *in ) )
		Com_Error( ERR_DROP, "CM_LoadMap: funny lump size in %s", cm.name );
	count = l->length / sizeof(*in);

	if (count < 1)
		Com_Error (ERR_DROP, "Map with no leafs");

	cm.leafs = Hunk_Alloc( ( BOX_LEAFS + count ) * sizeof( *cm.leafs ) );
	cm.numLeafs = count;

	out = cm.leafs;
	for( i = 0; i<count; i++, in++, out++ )
	{
		out->cluster = LittleLong( in->cluster );
		out->area = LittleLong( in->area );
		out->firstLeafBrush = LittleLong( in->firstLeafBrush );
		out->numLeafBrushes = LittleLong( in->numLeafBrushes );
		out->firstLeafSurface = LittleLong( in->firstLeafSurface );
		out->numLeafSurfaces = LittleLong( in->numLeafSurfaces );
		out->firstLeafTerrain = LittleLong( in->firstTerraPatch );
		out->numLeafTerrains = LittleLong( in->numTerraPatches );

		if( out->cluster >= cm.numClusters )
			cm.numClusters = out->cluster + 1;
		if( out->area >= cm.numAreas )
			cm.numAreas = out->area + 1;
	}

	cm.areas = Hunk_Alloc( cm.numAreas * sizeof( *cm.areas ) );
	cm.areaPortals = Hunk_Alloc( cm.numAreas * cm.numAreas * sizeof( *cm.areaPortals ) );
}

/*
=================
CMod_LoadLeafsOld
=================
*/
void CMod_LoadLeafsOld( gamelump_t *l )
{
	int				i;
	cLeaf_t			*out;
	dleaf_t_ver17 	*in;
	int				count;

	in = ( dleaf_t_ver17 * )l->buffer;
	if( l->length % sizeof( *in ) )
		Com_Error( ERR_DROP, "CM_LoadMap: funny lump size in %s", cm.name );
	count = l->length / sizeof( *in );

	if( count < 1 )
		Com_Error( ERR_DROP, "Map with no leafs" );

	cm.leafs = Hunk_Alloc( ( BOX_LEAFS + count ) * sizeof( *cm.leafs ) );
	cm.numLeafs = count;

	out = cm.leafs;
	for( i = 0; i<count; i++, in++, out++ )
	{
		out->cluster = LittleLong( in->cluster );
		out->area = LittleLong( in->area );
		out->firstLeafBrush = LittleLong( in->firstLeafBrush );
		out->numLeafBrushes = LittleLong( in->numLeafBrushes );
		out->firstLeafSurface = LittleLong( in->firstLeafSurface );
		out->numLeafSurfaces = LittleLong( in->numLeafSurfaces );

		if( out->cluster >= cm.numClusters )
			cm.numClusters = out->cluster + 1;
		if( out->area >= cm.numAreas )
			cm.numAreas = out->area + 1;
	}

	cm.areas = Hunk_Alloc( cm.numAreas * sizeof( *cm.areas ) );
	cm.areaPortals = Hunk_Alloc( cm.numAreas * cm.numAreas * sizeof( *cm.areaPortals ) );
}

/*
=================
CMod_LoadPlanes
=================
*/
void CMod_LoadPlanes( gamelump_t *l )
{
	int			i, j;
	cplane_t	*out;
	dplane_t 	*in;
	int			count;
	int			bits;

	in = ( dplane_t * )l->buffer;
	if( l->length % sizeof( *in ) )
		Com_Error( ERR_DROP, "CM_LoadMap: funny lump size in %s", cm.name );
	count = l->length / sizeof( *in );

	if (count < 1)
		Com_Error (ERR_DROP, "Map with no planes");
	cm.planes = Hunk_Alloc( ( BOX_PLANES + count ) * sizeof( *cm.planes ) );
	cm.numPlanes = count;

	out = cm.planes;

	for ( i=0 ; i<count ; i++, in++, out++)
	{
		bits = 0;
		for (j=0 ; j<3 ; j++)
		{
			out->normal[j] = LittleFloat (in->normal[j]);
			if (out->normal[j] < 0)
				bits |= 1<<j;
		}

		out->dist = LittleFloat (in->dist);
		out->type = PlaneTypeForNormal( out->normal );
		out->signbits = bits;
	}
}

/*
=================
CMod_LoadLeafBrushes
=================
*/
void CMod_LoadLeafBrushes( gamelump_t *l )
{
	int			i;
	int			*out;
	int		 	*in;
	int			count;

	in = ( int * )l->buffer;
	if( l->length % sizeof( *in ) )
		Com_Error( ERR_DROP, "CM_LoadMap: funny lump size in %s", cm.name );
	count = l->length / sizeof(*in);

	cm.leafbrushes = Hunk_Alloc( (count + BOX_BRUSHES) * sizeof( *cm.leafbrushes ) );
	cm.numLeafBrushes = count;

	out = cm.leafbrushes;

	for ( i=0 ; i<count ; i++, in++, out++) {
		*out = LittleLong( *in );
	}
}

/*
=================
CMod_LoadLeafSurfaces
=================
*/
void CMod_LoadLeafSurfaces( gamelump_t *l )
{
	int			i;
	int			*out;
	int		 	*in;
	int			count;

	in = ( int * )l->buffer;
	if( l->length % sizeof( *in ) )
		Com_Error( ERR_DROP, "CM_LoadMap: funny lump size in %s", cm.name );
	count = l->length / sizeof(*in);

	cm.leafsurfaces = Hunk_Alloc( count * sizeof( *cm.leafsurfaces ) );
	cm.numLeafSurfaces = count;

	out = cm.leafsurfaces;

	for ( i=0 ; i<count ; i++, in++, out++) {
		*out = LittleLong (*in);
	}
}

/*
=================
CMod_LoadSideEquations
=================
*/
void CMod_LoadSideEquations( gamelump_t *l )
{
	int					i, j;
	int					count;
	dsideequation_t		*out;
	dsideequation_t		*in;

	in = ( dsideequation_t * )l->buffer;
	if( l->length % sizeof( *in ) ) {
		Com_Error( ERR_DROP, "CM_LoadMap: funny lump size in %s", cm.name );
	}
	count = l->length / sizeof( *in );

	if( !count )
	{
		cm.numSideEquations = 0;
		cm.sideequations = NULL;
		return;
	}

	cm.sideequations = Hunk_Alloc( count * sizeof( *cm.sideequations ) );
	cm.numSideEquations = count;

	out = cm.sideequations;

	for( i = 0; i<count; i++, in++, out++ ) {
		for( j = 0; j < 4; j++ ) {
			out->fSeq[ j ] = LittleFloat( in->fSeq[ j ] );
			out->fTeq[ j ] = LittleFloat( in->fTeq[ j ] );
		}
	}
}

/*
=================
CMod_LoadBrushSides
=================
*/
void CMod_LoadBrushSides( gamelump_t *l )
{
	int				i;
	cbrushside_t	*out;
	dbrushside_t 	*in;
	int				count;
	int				num;

	in = ( dbrushside_t * )l->buffer;
	if( l->length % sizeof( *in ) ) {
		Com_Error( ERR_DROP, "CM_LoadMap: funny lump size in %s", cm.name );
	}
	count = l->length / sizeof(*in);

	cm.brushsides = Hunk_Alloc( ( BOX_SIDES + count ) * sizeof( *cm.brushsides ) );
	cm.numBrushSides = count;

	out = cm.brushsides;

	for ( i=0 ; i<count ; i++, in++, out++) {
		num = LittleLong( in->planeNum );
		out->plane = &cm.planes[num];
		out->shaderNum = LittleLong( in->shaderNum );
		if ( out->shaderNum < 0 || out->shaderNum >= cm.numShaders ) {
			Com_Error( ERR_DROP, "CMod_LoadBrushSides: bad shaderNum: %i", out->shaderNum );
		}
		out->surfaceFlags = cm.shaders[out->shaderNum].surfaceFlags;

		num = LittleLong( in->equationNum );
		if( num ) {
			out->pEq = &cm.sideequations[ num ];
		} else {
			out->pEq = NULL;
		}
	}
}


/*
=================
CMod_LoadEntityString
=================
*/
void CMod_LoadEntityString( gamelump_t *l ) {
	cm.entityString = Hunk_Alloc( l->length );
	cm.numEntityChars = l->length;
	Com_Memcpy( cm.entityString, l->buffer, l->length );
}

/*
=================
CMod_LoadVisibility
=================
*/
#define	VIS_HEADER	8
void CMod_LoadVisibility( gamelump_t *l ) {
	int		len;
	byte	*buf;

    len = l->length;
	if ( !len ) {
		cm.clusterBytes = ( cm.numClusters + 31 ) & ~31;
		cm.visibility = Hunk_Alloc( cm.clusterBytes );
		Com_Memset( cm.visibility, 255, cm.clusterBytes );
		return;
	}
	buf = ( byte * )l->buffer;

	cm.vised = qtrue;
	cm.visibility = Hunk_Alloc( len );
	cm.numClusters = LittleLong( ( ( int * )buf )[ 0 ] );
	cm.clusterBytes = LittleLong( ( ( int * )buf )[ 1 ] );
	Com_Memcpy( cm.visibility, buf + VIS_HEADER, len - VIS_HEADER );
}

//==================================================================


/*
=================
CMod_LoadPatches
=================
*/
#define	MAX_PATCH_VERTS		1024
void CMod_LoadPatches( gamelump_t *surfs, gamelump_t *verts, int *shaderSubdivisions ) {
	drawVert_t	*dv, *dv_p;
	dsurface_t	*in;
	int			count;
	int			i, j;
	int			c;
	cPatch_t	*patch;
	vec3_t		points[MAX_PATCH_VERTS];
	int			width, height;
	int			shaderNum;

	in = ( dsurface_t * )surfs->buffer;
	if( surfs->length % sizeof( *in ) )
		Com_Error( ERR_DROP, "CM_LoadMap: funny lump size in %s", cm.name );
	cm.numSurfaces = count = surfs->length / sizeof(*in);
	cm.surfaces = Hunk_Alloc( cm.numSurfaces * sizeof( cm.surfaces[0] ) );
	memset( cm.surfaces, 0, cm.numSurfaces * sizeof( cm.surfaces[ 0 ] ) );

	dv = ( drawVert_t * )verts->buffer;
	if( verts->length % sizeof( *dv ) )
		Com_Error( ERR_DROP, "CM_LoadMap: funny lump size in %s", cm.name );

	// scan through all the surfaces, but only load patches,
	// not planar faces
	for ( i = 0 ; i < count ; i++, in++ ) {
		if ( LittleLong( in->surfaceType ) != MST_PATCH ) {
			assert(cm.surfaces[ i ] == 0);
			continue;		// ignore other surfaces
		}

		shaderNum = LittleLong( in->shaderNum );
		if( !( cm.shaders[ shaderNum ].contentFlags & CONTENTS_SOLID ) &&
			!( cm.shaders[ shaderNum ].contentFlags & MASK_CLICK ) )
		{
			cm.surfaces[ i ] = NULL;
			continue;
		}

		cm.surfaces[ i ] = patch = Hunk_Alloc( sizeof( *patch ) );
		assert( cm.surfaces[ i ] != ( cPatch_t * )0 );
		assert( cm.surfaces[ i ] != ( cPatch_t * )0x4 );

		// load the full drawverts onto the stack
		width = LittleLong( in->patchWidth );
		height = LittleLong( in->patchHeight );
		c = width * height;
		if ( c > MAX_PATCH_VERTS ) {
			Com_Error( ERR_DROP, "ParseMesh: MAX_PATCH_VERTS" );
		}

		dv_p = dv + LittleLong( in->firstVert );
		for ( j = 0 ; j < c ; j++, dv_p++ ) {
			points[j][0] = LittleFloat( dv_p->xyz[0] );
			points[j][1] = LittleFloat( dv_p->xyz[1] );
			points[j][2] = LittleFloat( dv_p->xyz[2] );
		}

		shaderNum = LittleLong( in->shaderNum );
		patch->shaderNum = shaderNum;
		patch->contents = cm.shaders[ shaderNum ].contentFlags;
		patch->surfaceFlags = cm.shaders[ shaderNum ].surfaceFlags;

		patch->subdivisions = shaderSubdivisions[ shaderNum ];
		if( patch->subdivisions < MIN_MAP_SUBDIVISIONS ) {
			patch->subdivisions = MIN_MAP_SUBDIVISIONS;
		}

		// create the internal facet structure
		patch->pc = CM_GeneratePatchCollide( width, height, points, patch->subdivisions );
	}

	Hunk_FreeTempMemory( shaderSubdivisions );
}

/*
=================
CMod_LoadTerrain
=================
*/
void CMod_LoadTerrain( gamelump_t *lump ) {
	int				numTerraPatches;
	int				i;
	cTerraPatch_t	*terraPatches;

	if( !lump->length )
		return;

	terraPatches = ( cTerraPatch_t * )lump->buffer;
	if( lump->length % sizeof( *terraPatches ) )
		Com_Error( ERR_DROP, "CM_LoadMap: funny lump size in %s", cm.name );
	numTerraPatches = lump->length / sizeof( *terraPatches );
	cm.numTerrain = numTerraPatches;
	cm.terrain = ( cTerrain_t * )Hunk_Alloc( cm.numTerrain * sizeof( cm.terrain[ 0 ] ) );

	// Prepare collision
	CM_PrepareGenerateTerrainCollide();

	for( i = 0; i < numTerraPatches; i++ ) {
		// Generate collision
		CM_GenerateTerrainCollide( &terraPatches[ i ], &cm.terrain[ i ].tc );
		cm.terrain[ i ].contents = cm.shaders[ terraPatches[ i ].iShader ].contentFlags;
		cm.terrain[ i ].surfaceFlags = cm.shaders[ terraPatches[ i ].iShader ].surfaceFlags;
		cm.terrain[ i ].shaderNum = terraPatches[ i ].iShader;
	}
}

/*
=================
CMod_LoadTerrainIndexes
=================
*/
void CMod_LoadTerrainIndexes( gamelump_t *lump ) {
	int		i;
	short	*in;

	if( !lump->length ) {
		cm.numLeafTerrains = 0;
		cm.leafterrains = NULL;
		return;
	}

	if( lump->length % sizeof( short ) ) {
		Com_Error( ERR_DROP, "CMod_LoadTerrainIndexes: funny lump size" );
	}

	cm.numLeafTerrains = lump->length / sizeof( short );
	cm.leafterrains = ( cTerrain_t ** )Hunk_Alloc( cm.numLeafTerrains * sizeof( cTerrain_t * ) );

	in = ( short * )lump->buffer;

	for( i = 0; i < cm.numLeafTerrains; i++, in++ )
	{
		cm.leafterrains[ i ] = &cm.terrain[ *in ];
	}
}

//==================================================================

unsigned CM_Checksum( dheader_t *header ) {
	/*
	unsigned checksums[16];
	checksums[0] = CM_LumpChecksum(&header->lumps[LUMP_SHADERS]);
	checksums[1] = CM_LumpChecksum(&header->lumps[LUMP_LEAFS]);
	checksums[2] = CM_LumpChecksum(&header->lumps[LUMP_LEAFBRUSHES]);
	checksums[3] = CM_LumpChecksum(&header->lumps[LUMP_LEAFSURFACES]);
	checksums[4] = CM_LumpChecksum(&header->lumps[LUMP_PLANES]);
	checksums[5] = CM_LumpChecksum(&header->lumps[LUMP_BRUSHSIDES]);
	checksums[6] = CM_LumpChecksum(&header->lumps[LUMP_BRUSHES]);
	checksums[7] = CM_LumpChecksum(&header->lumps[LUMP_MODELS]);
	checksums[8] = CM_LumpChecksum(&header->lumps[LUMP_NODES]);
	checksums[9] = CM_LumpChecksum(&header->lumps[LUMP_SURFACES]);
	checksums[10] = CM_LumpChecksum(&header->lumps[LUMP_DRAWVERTS]);

	return LittleLong(Com_BlockChecksum(checksums, 11 * 4));
	*/
	return header->checksum;
}

/*
==================
CM_LoadLump

Loads a lump from the BSP file
==================
*/
int CM_LoadLump( fileHandle_t handle, lump_t *lump, gamelump_t *glump, int size )
{
	glump->buffer = NULL;
	glump->length = lump->filelen;

	if( lump->filelen ) {
		glump->buffer = Hunk_AllocateTempMemory( lump->filelen );

		if( FS_Seek( handle, lump->fileofs, FS_SEEK_SET ) ) {
			Com_Error( ERR_DROP, "CM_LoadLump: Error seeking to lump." );
		}

		FS_Read( glump->buffer, lump->filelen, handle );

		if( size ) {
			return lump->filelen / size;
		}
	}

	return 0;
}

/*
==================
CM_FreeLump

Free a previously allocated lump
==================
*/
void CM_FreeLump( gamelump_t *lump )
{
	if( lump->buffer )
	{
		Hunk_FreeTempMemory( lump->buffer );
		lump->buffer = NULL;
		lump->length = 0;
	}
}

#define _R( id ) UI_LoadResource( "*" #id )

/*
==================
CM_LoadMap

Loads in the map and all submodels
==================
*/
void CM_LoadMap( const char *name, qboolean clientload, int *checksum ) {
	gamelump_t		lump, lump2;
	int				*shaderSubdivisions;
	int				i;
	dheader_t		header;
	int				length;
	fileHandle_t	h;
	static unsigned	last_checksum;

	if ( !name || !name[0] ) {
		Com_Error( ERR_DROP, "CM_LoadMap: NULL name" );
	}

#ifndef BSPC
	cm_noAreas = Cvar_Get( "cm_noAreas", "0", CVAR_CHEAT );
	cm_noCurves = Cvar_Get( "cm_noCurves", "0", CVAR_CHEAT );
	cm_playerCurveClip = Cvar_Get( "cm_playerCurveClip", "1", CVAR_ARCHIVE | CVAR_CHEAT );
	cm_FCMcacheall = Cvar_Get( "cm_FCMcacheall", "0", CVAR_CHEAT );
	cm_FCMdebug = Cvar_Get( "cm_FCMdebug", "0", CVAR_CHEAT );
	cm_ter_usesphere = Cvar_Get( "cm_ter_usesphere", "1", CVAR_CHEAT );
#endif
	Com_DPrintf( "CM_LoadMap( %s, %i )\n", name, clientload );

	if ( !strcmp( cm.name, name ) && clientload ) {
		*checksum = last_checksum;
		return;
	}

	// free old stuff
	Com_Memset( &cm, 0, sizeof( cm ) );
	CM_ClearLevelPatches();

	if ( !name[0] ) {
		cm.numLeafs = 1;
		cm.numClusters = 1;
		cm.numAreas = 1;
		cm.cmodels = Hunk_Alloc( sizeof( *cm.cmodels ) );
		*checksum = 0;
		return;
	}

	// clear aliases
	Alias_Clear();

	//
	// load the file
	//
#ifndef BSPC
	length = FS_FOpenFileRead( name, &h, qtrue, qtrue );
#else
	length = LoadQuakeFile((quakefile_t *) name, (void **)&buf);
#endif

	if( length <= 0 ) {
		Com_Error( ERR_DROP, "Couldn't load %s", name );
	}

	FS_Read( &header, sizeof( dheader_t ), h );

	last_checksum = header.checksum;
	*checksum = last_checksum;

	for (i=0 ; i<sizeof(dheader_t)/4 ; i++) {
		((int *)&header)[i] = LittleLong ( ((int *)&header)[i]);
	}

	if ( header.version != BSP_VERSION ) {
		Com_Error (ERR_DROP, "CM_LoadMap: %s has wrong version number (%i should be %i)"
		, name, header.version, BSP_VERSION );
	}

	cmod_base = ( byte * )&header;

	// load into heap
	_R( 0 );
	CM_LoadLump( h, &header.lumps[ LUMP_SHADERS ], &lump, 0 );
	_R( 1 );
	CMod_LoadShaders( &lump, &shaderSubdivisions );
	_R( 2 );
	CM_FreeLump( &lump );
	_R( 3 );
	CM_LoadLump( h, &header.lumps[ LUMP_PLANES ], &lump, 0 );
	_R( 4 );
	CMod_LoadPlanes( &lump );
	_R( 5 );
	CM_FreeLump( &lump );
	_R( 6 );
	CM_LoadLump( h, &header.lumps[ LUMP_SURFACES ], &lump, 0 );
	_R( 7 );
	CM_LoadLump( h, &header.lumps[ LUMP_DRAWVERTS ], &lump2, 0 );
	_R( 8 );
	CMod_LoadPatches( &lump, &lump2, shaderSubdivisions );
	_R( 9 );
	CM_FreeLump( &lump2 );
	_R( 10 );
	CM_FreeLump( &lump );
	_R( 11 );
	cm.numLeafBrushes = CM_LoadLump( h, &header.lumps[ LUMP_LEAFBRUSHES ], &lump, sizeof( int ) );
	_R( 12 );
	CMod_LoadLeafBrushes( &lump );
	_R( 13 );
	CM_FreeLump( &lump );
	_R( 14 );
	CM_LoadLump( h, &header.lumps[ LUMP_LEAFSURFACES ], &lump, 0 );
	_R( 15 );
	CMod_LoadLeafSurfaces( &lump );
	_R( 16 );
	CM_FreeLump( &lump );
	_R( 17 );
	CM_LoadLump( h, &header.lumps[ LUMP_LEAFS ], &lump, 0 );
	_R( 18 );
	if( header.version > 18 ) {
		CMod_LoadLeafs( &lump );
	} else {
		CMod_LoadLeafsOld( &lump );
	}
	_R( 19 );
	CM_FreeLump( &lump );
	_R( 20 );
	CM_LoadLump( h, &header.lumps[ LUMP_NODES ], &lump, 0 );
	_R( 21 );
	CMod_LoadNodes( &lump );
	_R( 22 );
	CM_FreeLump( &lump );
	_R( 23 );
	g_iNumSideEquations = CM_LoadLump( h, &header.lumps[ LUMP_SIDEEQUATIONS ], &lump, 32 );
	_R( 24 );
	CMod_LoadSideEquations( &lump );
	_R( 25 );
	CM_FreeLump( &lump );
	_R( 26 );
	CM_LoadLump( h, &header.lumps[ LUMP_BRUSHSIDES ], &lump, 0 );
	_R( 27 );
	CMod_LoadBrushSides( &lump );
	_R( 28 );
	CM_FreeLump( &lump );
	_R( 29 );
	CM_LoadLump( h, &header.lumps[ LUMP_BRUSHES ], &lump, 0 );
	_R( 30 );
	CMod_LoadBrushes( &lump );
	_R( 31 );
	CM_FreeLump( &lump );
	_R( 32 );
	CM_LoadLump( h, &header.lumps[ LUMP_MODELS ], &lump, 0 );
	_R( 33 );
	CMod_LoadSubmodels( &lump );
	_R( 34 );
	CM_FreeLump( &lump );
	_R( 35 );
	CM_LoadLump( h, &header.lumps[ LUMP_ENTITIES ], &lump, 0 );
	_R( 36 );
	CMod_LoadEntityString( &lump );
	_R( 37 );
	CM_FreeLump( &lump );
	_R( 38 );
	CM_LoadLump( h, &header.lumps[ LUMP_VISIBILITY ], &lump, 0 );
	_R( 39 );
	CMod_LoadVisibility( &lump );
	_R( 40 );
	CM_FreeLump( &lump );
	_R( 41 );
	CM_LoadLump( h, &header.lumps[ LUMP_TERRAIN ], &lump, 0 );
	_R( 42 );
	CMod_LoadTerrain( &lump );
	_R( 43 );
	CM_FreeLump( &lump );
	_R( 44 );
	CM_LoadLump( h, &header.lumps[ LUMP_TERRAININDEXES ], &lump, 0 );
	_R( 45 );
	CMod_LoadTerrainIndexes( &lump );
	_R( 46 );
	CM_FreeLump( &lump );
	_R( 47 );
	FS_FCloseFile( h );
	_R( 48 );
	CM_InitBoxHull();
	_R( 49 );
	CM_FloodAreaConnections();
	_R( 50 );

	// allow this to be cached if it is loaded by the server
	if ( !clientload ) {
		Q_strncpyz( cm.name, name, sizeof( cm.name ) );
	}

	_R( 51 );
}

/*
==================
CM_ClearMap
==================
*/
void CM_ClearMap( void ) {
	Com_Memset( &cm, 0, sizeof( cm ) );
	CM_ClearLevelPatches();
}

/*
==================
CM_ClipHandleToModel
==================
*/
cmodel_t	*CM_ClipHandleToModel( clipHandle_t handle ) {
	if ( handle < 0 ) {
		Com_Error( ERR_DROP, "CM_ClipHandleToModel: bad handle %i", handle );
	}
	if ( handle < cm.numSubModels ) {
		return &cm.cmodels[handle];
	}
	if ( handle == BOX_MODEL_HANDLE ) {
		return &box_model;
	}
	if ( handle < MAX_SUBMODELS ) {
		Com_Error( ERR_DROP, "CM_ClipHandleToModel: bad handle %i < %i < %i",
			cm.numSubModels, handle, MAX_SUBMODELS );
	}
	Com_Error( ERR_DROP, "CM_ClipHandleToModel: bad handle %i", handle + MAX_SUBMODELS );

	return NULL;

}

/*
==================
CM_InlineModel
==================
*/
clipHandle_t	CM_InlineModel( int index ) {
	if ( index < 0 || index >= cm.numSubModels ) {
		Com_Error (ERR_DROP, "CM_InlineModel: bad number");
	}
	return index;
}

int		CM_NumClusters( void ) {
	return cm.numClusters;
}

int		CM_NumInlineModels( void ) {
	return cm.numSubModels;
}

char	*CM_EntityString( void ) {
	return cm.entityString;
}

const char	*CM_MapTime( void ) {
	const char *com_token;
	char *data;

	data = CM_EntityString();
	if( !data || *data != '{' ) {
		return NULL;
	}

	while( 1 )
	{
		com_token = COM_Parse( &data );
		if( !data || *com_token == '}' ) {
			break;
		}

		if( !strcmp( com_token, "map_time" ) )
		{
			com_token = COM_Parse( &data );
			if( com_token && *com_token != '}' ) {
				return com_token;
			} else {
				return "";
			}
		}
	}

	return "";
}

int		CM_LeafCluster( int leafnum ) {
	if (leafnum < 0 || leafnum >= cm.numLeafs) {
		Com_Error (ERR_DROP, "CM_LeafCluster: bad number");
	}
	return cm.leafs[leafnum].cluster;
}

int		CM_LeafArea( int leafnum ) {
	if ( leafnum < 0 || leafnum >= cm.numLeafs ) {
		Com_Error (ERR_DROP, "CM_LeafArea: bad number");
	}
	return cm.leafs[leafnum].area;
}

//=======================================================================


/*
===================
CM_InitBoxHull

Set up the planes and nodes so that the six floats of a bounding box
can just be stored out and get a proper clipping hull structure.
===================
*/
void CM_InitBoxHull (void)
{
	int			i;
	int			side;
	cplane_t	*p;
	cbrushside_t	*s;

	box_planes = &cm.planes[cm.numPlanes];

	box_brush = &cm.brushes[cm.numBrushes];
	box_brush->numsides = 6;
	box_brush->sides = cm.brushsides + cm.numBrushSides;
	box_brush->contents = CONTENTS_BBOX;

	box_model.leaf.numLeafBrushes = 1;
//	box_model.leaf.firstLeafBrush = cm.numBrushes;
	box_model.leaf.firstLeafBrush = cm.numLeafBrushes;
	cm.leafbrushes[cm.numLeafBrushes] = cm.numBrushes;

	for (i=0 ; i<6 ; i++)
	{
		side = i&1;

		// brush sides
		s = &cm.brushsides[cm.numBrushSides+i];
		s->plane = 	cm.planes + (cm.numPlanes+i*2+side);
		s->surfaceFlags = 0;

		// planes
		p = &box_planes[i*2];
		p->type = i>>1;
		p->signbits = 0;
		VectorClear (p->normal);
		p->normal[i>>1] = 1;

		p = &box_planes[i*2+1];
		p->type = 3 + (i>>1);
		p->signbits = 0;
		VectorClear (p->normal);
		p->normal[i>>1] = -1;

		SetPlaneSignbits( p );
	}
}

/*
===================
CM_TempBoxModel

To keep everything totally uniform, bounding boxes are turned into small
BSP trees instead of being compared directly.
Capsules are handled differently though.
===================
*/
clipHandle_t CM_TempBoxModel( const vec3_t mins, const vec3_t maxs, int contents ) {
	box_planes[0].dist = maxs[0];
	box_planes[1].dist = -maxs[0];
	box_planes[2].dist = mins[0];
	box_planes[3].dist = -mins[0];
	box_planes[4].dist = maxs[1];
	box_planes[5].dist = -maxs[1];
	box_planes[6].dist = mins[1];
	box_planes[7].dist = -mins[1];
	box_planes[8].dist = maxs[2];
	box_planes[9].dist = -maxs[2];
	box_planes[10].dist = mins[2];
	box_planes[11].dist = -mins[2];

	VectorCopy( mins, box_brush->bounds[0] );
	VectorCopy( maxs, box_brush->bounds[1] );
	box_brush->contents = contents;

	return BOX_MODEL_HANDLE;
}

/*
===================
CM_ModelBounds
===================
*/
void CM_ModelBounds( clipHandle_t model, vec3_t mins, vec3_t maxs ) {
	cmodel_t	*cmod;

	cmod = CM_ClipHandleToModel( model );
	VectorCopy( cmod->mins, mins );
	VectorCopy( cmod->maxs, maxs );
}

/*
===================
CM_ModelBoundsFromName
===================
*/
void CM_ModelBoundsFromName( const char *name, vec3_t mins, vec3_t maxs ) {
	clipHandle_t h;

	if( !name ) {
		Com_Error( ERR_DROP, "SV_SetBrushModel: NULL" );
	}

	if( *name != '*' ) {
		Com_Error( ERR_DROP, "SV_SetBrushModel: %s isn't a brush model", name );
	}

	h = CM_InlineModel( atoi( name + 1 ) );
	CM_ModelBounds( h, mins, maxs );
}

/*
===================
CM_VisibilityPointer
===================
*/
byte *CM_VisibilityPointer( void )
{
	if( cm.vised ) {
		return cm.visibility;
	} else {
		return NULL;
	}
}

/*
===================
CM_PrintBSPFileSizes
===================
*/
void CM_PrintBSPFileSizes( void )
{
	Com_Printf( "%6i   sideequations         %7i\n", g_iNumSideEquations, g_iNumSideEquations * sizeof( dsideequation_t ) );
	Com_Printf( "%6i   leafbrushes           %7i\n", numleafbrushes, numleafbrushes * sizeof( int ) );
}
