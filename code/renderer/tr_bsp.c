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
// tr_map.c

#include "tr_local.h"
#include "tr_vis.h"

/*

Loads and prepares a map file for scene rendering.

A single entry point:

void RE_LoadWorldMap( const char *name );

*/

static	world_t		s_worldData;
static	byte		*fileBase;

int			c_subdivisions;
int			c_gridVerts;

static int map_length;
static int nummodels;
static int numShaders;
static int numbrushes;
static int numbrushsides;
static int numFogs;
static int numplanes;
static int num_entities;
static int numnodes;
static int numleafs;
static int numleafsurfaces;
static int numDrawVerts;
static int numDrawIndexes;
static int numDrawSurfaces;
static int numLightDefs;
static int numLightBytes;
static int numVisBytes;
static int numSLights;
static int entLightVisSize;
static int g_iGridArraySize;
static int g_iGridDataSize;
static int g_iGridPaletteBytes;
static int g_iGridOffsets;
static int g_nStaticModelData;
static int map_version;
static int g_nStaticModelIndices;
static int g_nStaticModels;
static int g_nTerPatchIndices;
static int g_nTerraPatches;

//===============================================================================

static void HSVtoRGB( float h, float s, float v, float rgb[3] )
{
	int i;
	float f;
	float p, q, t;

	h *= 5;

	i = floor( h );
	f = h - i;

	p = v * ( 1 - s );
	q = v * ( 1 - s * f );
	t = v * ( 1 - s * ( 1 - f ) );

	switch ( i )
	{
	case 0:
		rgb[0] = v;
		rgb[1] = t;
		rgb[2] = p;
		break;
	case 1:
		rgb[0] = q;
		rgb[1] = v;
		rgb[2] = p;
		break;
	case 2:
		rgb[0] = p;
		rgb[1] = v;
		rgb[2] = t;
		break;
	case 3:
		rgb[0] = p;
		rgb[1] = q;
		rgb[2] = v;
		break;
	case 4:
		rgb[0] = t;
		rgb[1] = p;
		rgb[2] = v;
		break;
	case 5:
		rgb[0] = v;
		rgb[1] = p;
		rgb[2] = q;
		break;
	}
}

/*
===============
R_ColorShiftLightingBytes

===============
*/
static	void R_ColorShiftLightingBytes( byte in[4], byte out[4] ) {
	int		shift, r, g, b;

	// shift the color data based on overbright range
	shift = r_mapOverBrightBits->integer - tr.overbrightBits;

	// shift the data based on overbright range
	r = in[0] << shift;
	g = in[1] << shift;
	b = in[2] << shift;
	
	// normalize by color instead of saturating to white
	if ( ( r | g | b ) > 255 ) {
		int		max;

		max = r > g ? r : g;
		max = max > b ? max : b;
		r = r * 255 / max;
		g = g * 255 / max;
		b = b * 255 / max;
	}

	out[0] = r;
	out[1] = g;
	out[2] = b;
	out[3] = in[3];
}

/*
===============
R_LoadLightmaps

===============
*/
#define	LIGHTMAP_SIZE	128
static	void R_LoadLightmaps(gamelump_t* l, gamelump_t* surfs) {
	byte		*buf, *buf_p;
	int			len;
	MAC_STATIC byte		image[LIGHTMAP_SIZE*LIGHTMAP_SIZE*4];
	int			i, j;
	float maxIntensity = 0;
	double sumIntensity = 0;

    len = l->length;
    if (!len) {
        return;
    }
    buf = l->buffer;

	// we are about to upload textures
	R_SyncRenderThread();

	// create all the lightmaps
	tr.numLightmaps = len / (LIGHTMAP_SIZE * LIGHTMAP_SIZE * 3);
	if ( tr.numLightmaps == 1 ) {
		//FIXME: HACK: maps with only one lightmap turn up fullbright for some reason.
		//this avoids this, but isn't the correct solution.
		tr.numLightmaps++;
	}

	// if we are in r_vertexLight mode, we don't need the lightmaps at all
	if ( r_vertexLight->integer || glConfig.hardwareType == GLHW_PERMEDIA2 ) {
		return;
	}

	for ( i = 0 ; i < tr.numLightmaps ; i++ ) {
		// expand the 24 bit on-disk to 32 bit
		buf_p = buf + i * LIGHTMAP_SIZE*LIGHTMAP_SIZE * 3;

		if ( r_lightmap->integer == 2 )
		{	// color code by intensity as development tool	(FIXME: check range)
			for ( j = 0; j < LIGHTMAP_SIZE * LIGHTMAP_SIZE; j++ )
			{
				float r = buf_p[j*3+0];
				float g = buf_p[j*3+1];
				float b = buf_p[j*3+2];
				float intensity;
				float out[3];

				intensity = 0.33f * r + 0.685f * g + 0.063f * b;

				if ( intensity > 255 )
					intensity = 1.0f;
				else
					intensity /= 255.0f;

				if ( intensity > maxIntensity )
					maxIntensity = intensity;

				HSVtoRGB( intensity, 1.00, 0.50, out );

				image[j*4+0] = out[0] * 255;
				image[j*4+1] = out[1] * 255;
				image[j*4+2] = out[2] * 255;
				image[j*4+3] = 255;

				sumIntensity += intensity;
			}
		} else {
			for ( j = 0 ; j < LIGHTMAP_SIZE * LIGHTMAP_SIZE; j++ ) {
				R_ColorShiftLightingBytes( &buf_p[j*3], &image[j*4] );
				image[j*4+3] = 255;
			}
		}
		tr.lightmaps[i] = R_CreateImage( va("*lightmap%d",i), image, 
			LIGHTMAP_SIZE, LIGHTMAP_SIZE, qfalse, qfalse, GL_CLAMP );
	}

	if ( r_lightmap->integer == 2 )	{
		ri.Printf( PRINT_ALL, "Brightest lightmap value: %d\n", ( int ) ( maxIntensity * 255 ) );
	}
}


/*
=================
RE_SetWorldVisData

This is called by the clipmodel subsystem so we can share the 1.8 megs of
space in big maps...
=================
*/
void		RE_SetWorldVisData( const byte *vis ) {
	tr.externalVisData = vis;
}


/*
=================
R_LoadVisibility
=================
*/
static	void R_LoadVisibility(gamelump_t* l) {
	int		len;
	byte	*buf;

	len = ( s_worldData.numClusters + 63 ) & ~63;
	s_worldData.novis = ri.Hunk_Alloc( len );
	Com_Memset( s_worldData.novis, 0xff, len );

    len = l->length;
    if (!len) {
        return;
    }
    buf = l->buffer;

	s_worldData.numClusters = LittleLong( ((int *)buf)[0] );
	s_worldData.clusterBytes = LittleLong( ((int *)buf)[1] );

	// CM_Load should have given us the vis data to share, so
	// we don't need to allocate another copy
	if ( tr.externalVisData ) {
		s_worldData.vis = tr.externalVisData;
	} else {
		byte	*dest;

		dest = ri.Hunk_Alloc( len - 8 );
		Com_Memcpy( dest, buf + 8, len - 8 );
		s_worldData.vis = dest;
	}
}

/*
================
R_LoadSphereLights
================
*/
void R_LoadSphereLights(gamelump_t* l) {
    int				i, j;
    mapspherel_t* at;
    spherel_t* light;

    if (l->length % sizeof(mapspherel_t)) {
        Com_Error(ERR_DROP, "LoadMap: funny lump size in spherelight data for %s\n", s_worldData.name);
    }

    tr.numSLights = l->length / sizeof(mapspherel_t);

    if (tr.numSLights >= MAX_MAP_SPHERE_L_SIZE) {
        Com_Error(ERR_DROP, "LoadMap: Too many spherelights on map %s, limit is %d\n", s_worldData.name, MAX_MAP_SPHERE_L_SIZE);
    }

    at = l->buffer;
    light = tr.sLights;

    for (i = 0; i < tr.numSLights; at++, light++, i++) {
        VectorCopy(at->origin, light->origin);
        VectorCopy(at->color, light->color);
        VectorCopy(at->spot_dir, light->spot_dir);

        for (j = 0; j < 3; j++) {
            light->origin[j] = LittleFloat(light->origin[j]);
            light->color[j] = LittleFloat(light->color[j]);
            light->spot_dir[j] = LittleFloat(light->spot_dir[j]);
        }

        light->spot_radiusbydistance = LittleFloat(at->spot_radiusbydistance);
        light->intensity = LittleFloat(at->intensity);
        light->spot_light = LittleLong(at->spot_light);
        light->needs_trace = LittleLong(at->needs_trace);
        light->leaf = &s_worldData.nodes[s_worldData.numDecisionNodes + LittleLong(at->leaf)];
    }
}

/*
================
R_LoadSphereLightVis
================
*/
void R_LoadSphereLightVis(gamelump_t* l) {
    int i;
    int j;
    int h;
    mnode_t* node;
    int numentries;
    int* entries;

    memset(&tr.sSunLight, 0, sizeof(spherel_t));

    if (l->length % 4) {
        ri.Error(ERR_DROP, "LoadMap: funny lump size in spherelight vis data for %s\n",
            s_worldData.name);
    }
    else {
        entries = l->buffer;
        numentries = l->length / sizeof(int);

        if (numentries) {
            if (numentries != s_worldData.numnodes - s_worldData.numDecisionNodes) {
                for (i = 0; i < numentries; ++i)
                    entries[i] = LittleLong(entries[i]);

                for (node = &s_worldData.nodes[s_worldData.numDecisionNodes]; numentries; node++) {
                    j = 0;
                    if (entries[0] != -1) {
                        do
                            ++j;
                        while (entries[j] != -1);
                    }
                    if (j) {
                        node->lights = ri.Hunk_Alloc(j * sizeof(spherel_t*));
                        node->numlights = j;
                        h = 0;
                        if (entries[0] == -2) {
                            node->lights[0] = &tr.sSunLight;
                            tr.sSunLight.leaf = (mnode_t*)-1;
                            h = 1;
                        }
                        while (entries[h] != -1) {
                            node->lights[h] = &tr.sLights[entries[h]];
                            h++;
                        }
                        entries += h + 1;
                        numentries = numentries - 1 - h;
                    }
                    else {
                        numentries--;
                        entries++;
                    }
                }
            }
        }
    }
}

//===============================================================================


/*
===============
ShaderForShaderNum
===============
*/
static shader_t *ShaderForShaderNum( int shaderNum, int lightmapNum ) {
	shader_t	*shader;
	dshader_t	*dsh;

	shaderNum = LittleLong( shaderNum );
	if ( shaderNum < 0 || shaderNum >= s_worldData.numShaders ) {
		ri.Error( ERR_DROP, "ShaderForShaderNum: bad num %i", shaderNum );
	}
	dsh = &s_worldData.shaders[ shaderNum ];

	if ( r_vertexLight->integer || glConfig.hardwareType == GLHW_PERMEDIA2 ) {
		lightmapNum = LIGHTMAP_BY_VERTEX;
	}

	if ( r_fullbright->integer ) {
		lightmapNum = LIGHTMAP_WHITEIMAGE;
	}

	shader = R_FindShader( dsh->shader, lightmapNum, qtrue, qtrue, qtrue, qtrue);

	// if the shader had errors, just use default shader
	if ( shader->defaultShader ) {
		return tr.defaultShader;
	}

	return shader;
}

/*
================
R_UnpackTerraPatch
================
*/
void R_UnpackTerraPatch(cTerraPatch_t* pPacked, cTerraPatchUnpacked_t* pUnpacked) {
    int			i;

    pUnpacked->byDirty = qfalse;
    pUnpacked->visCountCheck = 0;
    pUnpacked->visCountDraw = 0;
    pUnpacked->uiDistRecalc = 0;

    if (pPacked->lmapScale <= 0) {
        Com_Error(ERR_DROP, "invalid map: terrain has lmapScale <= 0");
    }

    pUnpacked->drawinfo.lmapStep = (pPacked->lmapScale / 64);
    pUnpacked->drawinfo.lmapSize = (pPacked->lmapScale * 8) | 1;
    pUnpacked->s = ((float)pPacked->s + 0.5) * 0.0078125;
    pUnpacked->t = ((float)pPacked->t + 0.5) * 0.0078125;

    //if( s_worldData.lighting ) {
    //	pUnpacked->drawinfo.lmData = &s_worldData.lighting[ 3 * ( ( pPacked->t << 7 ) + pPacked->s + ( pPacked->iLightMap << 14 ) ) ];
    //} else {
    //	pUnpacked->drawinfo.lmData = NULL;
    //}

    // fixme: is it important to have a lightmap data?
    pUnpacked->drawinfo.lmData = NULL;

    memcpy(pUnpacked->texCoord, pPacked->texCoord, sizeof(pUnpacked->texCoord));
    pUnpacked->x0 = ((int)pPacked->x << 6);
    pUnpacked->y0 = ((int)pPacked->y << 6);
    pUnpacked->z0 = pPacked->iBaseHeight;
    pUnpacked->shader = ShaderForShaderNum(pPacked->iShader, pPacked->iLightMap);
    pUnpacked->iNorth = pPacked->iNorth;
    pUnpacked->iEast = pPacked->iEast;
    pUnpacked->iSouth = pPacked->iSouth;
    pUnpacked->iWest = pPacked->iWest;

    for (i = 0; i < 63; i++)
    {
        pUnpacked->varTree[0][i / 4].fVariance = pPacked->varTree[0][i].flags >> 12;
        pUnpacked->varTree[1][i / 4].fVariance = pPacked->varTree[1][i].flags >> 12;
    }

    memcpy(pUnpacked->heightmap, pPacked->heightmap, sizeof(pUnpacked->heightmap));
    pUnpacked->zmax = 0;
    pUnpacked->flags = pPacked->flags;

    for (i = 0; i < sizeof(pUnpacked->heightmap); i++)
    {
        if (pUnpacked->zmax < pUnpacked->heightmap[i]) {
            pUnpacked->zmax = pUnpacked->heightmap[i];
        }
    }

    pUnpacked->frameCount = 0;
    pUnpacked->zmax += pUnpacked->zmax;
}

/*
================
R_LoadTerrain
================
IneQuation was here
*/
void R_LoadTerrain(gamelump_t* lump) {
    int		i;
    cTerraPatch_t* in;
    cTerraPatchUnpacked_t* out;

    if (!lump->length) {
        s_worldData.numTerraPatches = 0;
        s_worldData.terraPatches = NULL;
        return;
    }

    if (lump->length % sizeof(cTerraPatch_t)) {
        Com_Error(ERR_DROP, "R_LoadTerrain: funny lump size");
    }

    s_worldData.numTerraPatches = lump->length / sizeof(cTerraPatch_t);
    s_worldData.terraPatches = ri.Hunk_Alloc(s_worldData.numTerraPatches * sizeof(cTerraPatchUnpacked_t));

    in = lump->buffer;
    out = s_worldData.terraPatches;

    for (i = 0; i < s_worldData.numTerraPatches; in++, out++, i++) {
        R_UnpackTerraPatch(in, out);
    }
}

/*
================
R_LoadTerrainIndexes
================
*/
void R_LoadTerrainIndexes(gamelump_t* lump) {
    int		i;
    short* in;
    cTerraPatchUnpacked_t** out;

    if (!lump->length) {
        s_worldData.numVisTerraPatches = 0;
        s_worldData.visTerraPatches = NULL;
        return;
    }

    if (lump->length % sizeof(short)) {
        Com_Error(ERR_DROP, "R_LoadTerrainIndexes: funny lump size");
    }

    s_worldData.numVisTerraPatches = lump->length / sizeof(short);
    s_worldData.visTerraPatches = ri.Hunk_Alloc(s_worldData.numVisTerraPatches * sizeof(cTerraPatchUnpacked_t*));

    in = lump->buffer;
    out = s_worldData.visTerraPatches;

    for (i = 0; i < s_worldData.numVisTerraPatches; in++, out++, i++) {
        *out = &s_worldData.terraPatches[*in];
    }
}

/*
===============
ParseFace
===============
*/
static void ParseFace( dsurface_t *ds, drawVert_t *verts, msurface_t *surf, int *indexes  ) {
	int			i, j;
	srfSurfaceFace_t	*cv;
	int			numPoints, numIndexes;
	int			lightmapNum;
	int			sfaceSize, ofsIndexes;

	lightmapNum = LittleLong( ds->lightmapNum );

	// get fog volume
	surf->fogIndex = LittleLong( ds->fogNum ) + 1;

	// get shader value
	surf->shader = ShaderForShaderNum( ds->shaderNum, lightmapNum );
	if ( r_singleShader->integer && !surf->shader->isSky ) {
		surf->shader = tr.defaultShader;
	}

	numPoints = LittleLong( ds->numVerts );
	if (numPoints > MAX_FACE_POINTS) {
		ri.Printf( PRINT_WARNING, "WARNING: MAX_FACE_POINTS exceeded: %i\n", numPoints);
    numPoints = MAX_FACE_POINTS;
    surf->shader = tr.defaultShader;
	}

	numIndexes = LittleLong( ds->numIndexes );

	// create the srfSurfaceFace_t
	sfaceSize = ( int ) &((srfSurfaceFace_t *)0)->points[numPoints];
	ofsIndexes = sfaceSize;
	sfaceSize += sizeof( int ) * numIndexes;

	cv = ri.Hunk_Alloc( sfaceSize );
	cv->surfaceType = SF_FACE;
	cv->numPoints = numPoints;
	cv->numIndices = numIndexes;
	cv->ofsIndices = ofsIndexes;

	verts += LittleLong( ds->firstVert );
	for ( i = 0 ; i < numPoints ; i++ ) {
		for ( j = 0 ; j < 3 ; j++ ) {
			cv->points[i][j] = LittleFloat( verts[i].xyz[j] );
		}
		for ( j = 0 ; j < 2 ; j++ ) {
			cv->points[i][3+j] = LittleFloat( verts[i].st[j] );
			cv->points[i][5+j] = LittleFloat( verts[i].lightmap[j] );
		}
		R_ColorShiftLightingBytes( verts[i].color, (byte *)&cv->points[i][7] );
	}

	indexes += LittleLong( ds->firstIndex );
	for ( i = 0 ; i < numIndexes ; i++ ) {
		((int *)((byte *)cv + cv->ofsIndices ))[i] = LittleLong( indexes[ i ] );
	}

	// take the plane information from the lightmap vector
	for ( i = 0 ; i < 3 ; i++ ) {
		cv->plane.normal[i] = LittleFloat( ds->lightmapVecs[2][i] );
	}
	cv->plane.dist = DotProduct( cv->points[0], cv->plane.normal );
	SetPlaneSignbits( &cv->plane );
	cv->plane.type = PlaneTypeForNormal( cv->plane.normal );

	surf->data = (surfaceType_t *)cv;
}


/*
===============
ParseMesh
===============
*/
static void ParseMesh ( dsurface_t *ds, drawVert_t *verts, msurface_t *surf ) {
	srfGridMesh_t	*grid;
	int				i, j;
	int				width, height, numPoints;
	MAC_STATIC drawVert_t points[MAX_PATCH_SIZE*MAX_PATCH_SIZE];
	int				lightmapNum;
	vec3_t			bounds[2];
	vec3_t			tmpVec;
	static surfaceType_t	skipData = SF_SKIP;

	lightmapNum = LittleLong( ds->lightmapNum );

	// get fog volume
	surf->fogIndex = LittleLong( ds->fogNum ) + 1;

	// get shader value
	surf->shader = ShaderForShaderNum( ds->shaderNum, lightmapNum );
	if ( r_singleShader->integer && !surf->shader->isSky ) {
		surf->shader = tr.defaultShader;
	}

	// we may have a nodraw surface, because they might still need to
	// be around for movement clipping
	if ( s_worldData.shaders[ LittleLong( ds->shaderNum ) ].surfaceFlags & SURF_NODRAW ) {
		surf->data = &skipData;
		return;
	}

	width = LittleLong( ds->patchWidth );
	height = LittleLong( ds->patchHeight );

	verts += LittleLong( ds->firstVert );
	numPoints = width * height;
	for ( i = 0 ; i < numPoints ; i++ ) {
		for ( j = 0 ; j < 3 ; j++ ) {
			points[i].xyz[j] = LittleFloat( verts[i].xyz[j] );
			points[i].normal[j] = LittleFloat( verts[i].normal[j] );
		}
		for ( j = 0 ; j < 2 ; j++ ) {
			points[i].st[j] = LittleFloat( verts[i].st[j] );
			points[i].lightmap[j] = LittleFloat( verts[i].lightmap[j] );
		}
		R_ColorShiftLightingBytes( verts[i].color, points[i].color );
	}

	// pre-tesseleate
	grid = R_SubdividePatchToGrid( width, height, points );
	surf->data = (surfaceType_t *)grid;

	// copy the level of detail origin, which is the center
	// of the group of all curves that must subdivide the same
	// to avoid cracking
	for ( i = 0 ; i < 3 ; i++ ) {
		bounds[0][i] = LittleFloat( ds->lightmapVecs[0][i] );
		bounds[1][i] = LittleFloat( ds->lightmapVecs[1][i] );
	}
	VectorAdd( bounds[0], bounds[1], bounds[1] );
	VectorScale( bounds[1], 0.5f, grid->lodOrigin );
	VectorSubtract( bounds[0], grid->lodOrigin, tmpVec );
	grid->lodRadius = VectorLength( tmpVec );
}

/*
===============
ParseTriSurf
===============
*/
static void ParseTriSurf( dsurface_t *ds, drawVert_t *verts, msurface_t *surf, int *indexes ) {
	srfTriangles_t	*tri;
	int				i, j;
	int				numVerts, numIndexes;

	// get fog volume
	surf->fogIndex = LittleLong( ds->fogNum ) + 1;

	// get shader
	surf->shader = ShaderForShaderNum( ds->shaderNum, LIGHTMAP_BY_VERTEX );
	if ( r_singleShader->integer && !surf->shader->isSky ) {
		surf->shader = tr.defaultShader;
	}

	numVerts = LittleLong( ds->numVerts );
	numIndexes = LittleLong( ds->numIndexes );

	tri = ri.Hunk_Alloc( sizeof( *tri ) + numVerts * sizeof( tri->verts[0] ) 
		+ numIndexes * sizeof( tri->indexes[0] ) );
	tri->surfaceType = SF_TRIANGLES;
	tri->numVerts = numVerts;
	tri->numIndexes = numIndexes;
	tri->verts = (drawVert_t *)(tri + 1);
	tri->indexes = (int *)(tri->verts + tri->numVerts );

	surf->data = (surfaceType_t *)tri;

	// copy vertexes
	ClearBounds( tri->bounds[0], tri->bounds[1] );
	verts += LittleLong( ds->firstVert );
	for ( i = 0 ; i < numVerts ; i++ ) {
		for ( j = 0 ; j < 3 ; j++ ) {
			tri->verts[i].xyz[j] = LittleFloat( verts[i].xyz[j] );
			tri->verts[i].normal[j] = LittleFloat( verts[i].normal[j] );
		}
		AddPointToBounds( tri->verts[i].xyz, tri->bounds[0], tri->bounds[1] );
		for ( j = 0 ; j < 2 ; j++ ) {
			tri->verts[i].st[j] = LittleFloat( verts[i].st[j] );
			tri->verts[i].lightmap[j] = LittleFloat( verts[i].lightmap[j] );
		}

		R_ColorShiftLightingBytes( verts[i].color, tri->verts[i].color );
	}

	// copy indexes
	indexes += LittleLong( ds->firstIndex );
	for ( i = 0 ; i < numIndexes ; i++ ) {
		tri->indexes[i] = LittleLong( indexes[i] );
		if ( tri->indexes[i] < 0 || tri->indexes[i] >= numVerts ) {
			ri.Error( ERR_DROP, "Bad index in triangle surface" );
		}
	}
}

/*
===============
ParseFlare
===============
*/
static void ParseFlare( dsurface_t *ds, drawVert_t *verts, msurface_t *surf, int *indexes ) {
	srfFlare_t		*flare;
	int				i;

	// get fog volume
	surf->fogIndex = LittleLong( ds->fogNum ) + 1;

	// get shader
	surf->shader = ShaderForShaderNum( ds->shaderNum, LIGHTMAP_BY_VERTEX );
	if ( r_singleShader->integer && !surf->shader->isSky ) {
		surf->shader = tr.defaultShader;
	}

	flare = ri.Hunk_Alloc( sizeof( *flare ) );
	flare->surfaceType = SF_FLARE;

	surf->data = (surfaceType_t *)flare;

	for ( i = 0 ; i < 3 ; i++ ) {
		flare->origin[i] = LittleFloat( ds->lightmapOrigin[i] );
		flare->color[i] = LittleFloat( ds->lightmapVecs[0][i] );
		flare->normal[i] = LittleFloat( ds->lightmapVecs[2][i] );
	}
}


/*
=================
R_MergedWidthPoints

returns true if there are grid points merged on a width edge
=================
*/
int R_MergedWidthPoints(srfGridMesh_t *grid, int offset) {
	int i, j;

	for (i = 1; i < grid->width-1; i++) {
		for (j = i + 1; j < grid->width-1; j++) {
			if ( fabs(grid->verts[i + offset].xyz[0] - grid->verts[j + offset].xyz[0]) > .1) continue;
			if ( fabs(grid->verts[i + offset].xyz[1] - grid->verts[j + offset].xyz[1]) > .1) continue;
			if ( fabs(grid->verts[i + offset].xyz[2] - grid->verts[j + offset].xyz[2]) > .1) continue;
			return qtrue;
		}
	}
	return qfalse;
}

/*
=================
R_MergedHeightPoints

returns true if there are grid points merged on a height edge
=================
*/
int R_MergedHeightPoints(srfGridMesh_t *grid, int offset) {
	int i, j;

	for (i = 1; i < grid->height-1; i++) {
		for (j = i + 1; j < grid->height-1; j++) {
			if ( fabs(grid->verts[grid->width * i + offset].xyz[0] - grid->verts[grid->width * j + offset].xyz[0]) > .1) continue;
			if ( fabs(grid->verts[grid->width * i + offset].xyz[1] - grid->verts[grid->width * j + offset].xyz[1]) > .1) continue;
			if ( fabs(grid->verts[grid->width * i + offset].xyz[2] - grid->verts[grid->width * j + offset].xyz[2]) > .1) continue;
			return qtrue;
		}
	}
	return qfalse;
}

/*
=================
R_FixSharedVertexLodError_r

NOTE: never sync LoD through grid edges with merged points!

FIXME: write generalized version that also avoids cracks between a patch and one that meets half way?
=================
*/
void R_FixSharedVertexLodError_r( int start, srfGridMesh_t *grid1 ) {
	int j, k, l, m, n, offset1, offset2, touch;
	srfGridMesh_t *grid2;

	for ( j = start; j < s_worldData.numsurfaces; j++ ) {
		//
		grid2 = (srfGridMesh_t *) s_worldData.surfaces[j].data;
		// if this surface is not a grid
		if ( grid2->surfaceType != SF_GRID ) continue;
		// if the LOD errors are already fixed for this patch
		if ( grid2->lodFixed == 2 ) continue;
		// grids in the same LOD group should have the exact same lod radius
		if ( grid1->lodRadius != grid2->lodRadius ) continue;
		// grids in the same LOD group should have the exact same lod origin
		if ( grid1->lodOrigin[0] != grid2->lodOrigin[0] ) continue;
		if ( grid1->lodOrigin[1] != grid2->lodOrigin[1] ) continue;
		if ( grid1->lodOrigin[2] != grid2->lodOrigin[2] ) continue;
		//
		touch = qfalse;
		for (n = 0; n < 2; n++) {
			//
			if (n) offset1 = (grid1->height-1) * grid1->width;
			else offset1 = 0;
			if (R_MergedWidthPoints(grid1, offset1)) continue;
			for (k = 1; k < grid1->width-1; k++) {
				for (m = 0; m < 2; m++) {

					if (m) offset2 = (grid2->height-1) * grid2->width;
					else offset2 = 0;
					if (R_MergedWidthPoints(grid2, offset2)) continue;
					for ( l = 1; l < grid2->width-1; l++) {
					//
						if ( fabs(grid1->verts[k + offset1].xyz[0] - grid2->verts[l + offset2].xyz[0]) > .1) continue;
						if ( fabs(grid1->verts[k + offset1].xyz[1] - grid2->verts[l + offset2].xyz[1]) > .1) continue;
						if ( fabs(grid1->verts[k + offset1].xyz[2] - grid2->verts[l + offset2].xyz[2]) > .1) continue;
						// ok the points are equal and should have the same lod error
						grid2->widthLodError[l] = grid1->widthLodError[k];
						touch = qtrue;
					}
				}
				for (m = 0; m < 2; m++) {

					if (m) offset2 = grid2->width-1;
					else offset2 = 0;
					if (R_MergedHeightPoints(grid2, offset2)) continue;
					for ( l = 1; l < grid2->height-1; l++) {
					//
						if ( fabs(grid1->verts[k + offset1].xyz[0] - grid2->verts[grid2->width * l + offset2].xyz[0]) > .1) continue;
						if ( fabs(grid1->verts[k + offset1].xyz[1] - grid2->verts[grid2->width * l + offset2].xyz[1]) > .1) continue;
						if ( fabs(grid1->verts[k + offset1].xyz[2] - grid2->verts[grid2->width * l + offset2].xyz[2]) > .1) continue;
						// ok the points are equal and should have the same lod error
						grid2->heightLodError[l] = grid1->widthLodError[k];
						touch = qtrue;
					}
				}
			}
		}
		for (n = 0; n < 2; n++) {
			//
			if (n) offset1 = grid1->width-1;
			else offset1 = 0;
			if (R_MergedHeightPoints(grid1, offset1)) continue;
			for (k = 1; k < grid1->height-1; k++) {
				for (m = 0; m < 2; m++) {

					if (m) offset2 = (grid2->height-1) * grid2->width;
					else offset2 = 0;
					if (R_MergedWidthPoints(grid2, offset2)) continue;
					for ( l = 1; l < grid2->width-1; l++) {
					//
						if ( fabs(grid1->verts[grid1->width * k + offset1].xyz[0] - grid2->verts[l + offset2].xyz[0]) > .1) continue;
						if ( fabs(grid1->verts[grid1->width * k + offset1].xyz[1] - grid2->verts[l + offset2].xyz[1]) > .1) continue;
						if ( fabs(grid1->verts[grid1->width * k + offset1].xyz[2] - grid2->verts[l + offset2].xyz[2]) > .1) continue;
						// ok the points are equal and should have the same lod error
						grid2->widthLodError[l] = grid1->heightLodError[k];
						touch = qtrue;
					}
				}
				for (m = 0; m < 2; m++) {

					if (m) offset2 = grid2->width-1;
					else offset2 = 0;
					if (R_MergedHeightPoints(grid2, offset2)) continue;
					for ( l = 1; l < grid2->height-1; l++) {
					//
						if ( fabs(grid1->verts[grid1->width * k + offset1].xyz[0] - grid2->verts[grid2->width * l + offset2].xyz[0]) > .1) continue;
						if ( fabs(grid1->verts[grid1->width * k + offset1].xyz[1] - grid2->verts[grid2->width * l + offset2].xyz[1]) > .1) continue;
						if ( fabs(grid1->verts[grid1->width * k + offset1].xyz[2] - grid2->verts[grid2->width * l + offset2].xyz[2]) > .1) continue;
						// ok the points are equal and should have the same lod error
						grid2->heightLodError[l] = grid1->heightLodError[k];
						touch = qtrue;
					}
				}
			}
		}
		if (touch) {
			grid2->lodFixed = 2;
			R_FixSharedVertexLodError_r ( start, grid2 );
			//NOTE: this would be correct but makes things really slow
			//grid2->lodFixed = 1;
		}
	}
}

/*
=================
R_FixSharedVertexLodError

This function assumes that all patches in one group are nicely stitched together for the highest LoD.
If this is not the case this function will still do its job but won't fix the highest LoD cracks.
=================
*/
void R_FixSharedVertexLodError( void ) {
	int i;
	srfGridMesh_t *grid1;

	for ( i = 0; i < s_worldData.numsurfaces; i++ ) {
		//
		grid1 = (srfGridMesh_t *) s_worldData.surfaces[i].data;
		// if this surface is not a grid
		if ( grid1->surfaceType != SF_GRID )
			continue;
		//
		if ( grid1->lodFixed )
			continue;
		//
		grid1->lodFixed = 2;
		// recursively fix other patches in the same LOD group
		R_FixSharedVertexLodError_r( i + 1, grid1);
	}
}


/*
===============
R_StitchPatches
===============
*/
int R_StitchPatches( int grid1num, int grid2num ) {
	float *v1, *v2;
	srfGridMesh_t *grid1, *grid2;
	int k, l, m, n, offset1, offset2, row, column;

	grid1 = (srfGridMesh_t *) s_worldData.surfaces[grid1num].data;
	grid2 = (srfGridMesh_t *) s_worldData.surfaces[grid2num].data;
	for (n = 0; n < 2; n++) {
		//
		if (n) offset1 = (grid1->height-1) * grid1->width;
		else offset1 = 0;
		if (R_MergedWidthPoints(grid1, offset1))
			continue;
		for (k = 0; k < grid1->width-2; k += 2) {

			for (m = 0; m < 2; m++) {

				if ( grid2->width >= MAX_GRID_SIZE )
					break;
				if (m) offset2 = (grid2->height-1) * grid2->width;
				else offset2 = 0;
				for ( l = 0; l < grid2->width-1; l++) {
				//
					v1 = grid1->verts[k + offset1].xyz;
					v2 = grid2->verts[l + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) > .1)
						continue;
					if ( fabs(v1[1] - v2[1]) > .1)
						continue;
					if ( fabs(v1[2] - v2[2]) > .1)
						continue;

					v1 = grid1->verts[k + 2 + offset1].xyz;
					v2 = grid2->verts[l + 1 + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) > .1)
						continue;
					if ( fabs(v1[1] - v2[1]) > .1)
						continue;
					if ( fabs(v1[2] - v2[2]) > .1)
						continue;
					//
					v1 = grid2->verts[l + offset2].xyz;
					v2 = grid2->verts[l + 1 + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) < .01 &&
							fabs(v1[1] - v2[1]) < .01 &&
							fabs(v1[2] - v2[2]) < .01)
						continue;
					//
					//ri.Printf( PRINT_ALL, "found highest LoD crack between two patches\n" );
					// insert column into grid2 right after after column l
					if (m) row = grid2->height-1;
					else row = 0;
					grid2 = R_GridInsertColumn( grid2, l+1, row,
									grid1->verts[k + 1 + offset1].xyz, grid1->widthLodError[k+1]);
					grid2->lodStitched = qfalse;
					s_worldData.surfaces[grid2num].data = (void *) grid2;
					return qtrue;
				}
			}
			for (m = 0; m < 2; m++) {

				if (grid2->height >= MAX_GRID_SIZE)
					break;
				if (m) offset2 = grid2->width-1;
				else offset2 = 0;
				for ( l = 0; l < grid2->height-1; l++) {
					//
					v1 = grid1->verts[k + offset1].xyz;
					v2 = grid2->verts[grid2->width * l + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) > .1)
						continue;
					if ( fabs(v1[1] - v2[1]) > .1)
						continue;
					if ( fabs(v1[2] - v2[2]) > .1)
						continue;

					v1 = grid1->verts[k + 2 + offset1].xyz;
					v2 = grid2->verts[grid2->width * (l + 1) + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) > .1)
						continue;
					if ( fabs(v1[1] - v2[1]) > .1)
						continue;
					if ( fabs(v1[2] - v2[2]) > .1)
						continue;
					//
					v1 = grid2->verts[grid2->width * l + offset2].xyz;
					v2 = grid2->verts[grid2->width * (l + 1) + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) < .01 &&
							fabs(v1[1] - v2[1]) < .01 &&
							fabs(v1[2] - v2[2]) < .01)
						continue;
					//
					//ri.Printf( PRINT_ALL, "found highest LoD crack between two patches\n" );
					// insert row into grid2 right after after row l
					if (m) column = grid2->width-1;
					else column = 0;
					grid2 = R_GridInsertRow( grid2, l+1, column,
										grid1->verts[k + 1 + offset1].xyz, grid1->widthLodError[k+1]);
					grid2->lodStitched = qfalse;
					s_worldData.surfaces[grid2num].data = (void *) grid2;
					return qtrue;
				}
			}
		}
	}
	for (n = 0; n < 2; n++) {
		//
		if (n) offset1 = grid1->width-1;
		else offset1 = 0;
		if (R_MergedHeightPoints(grid1, offset1))
			continue;
		for (k = 0; k < grid1->height-2; k += 2) {
			for (m = 0; m < 2; m++) {

				if ( grid2->width >= MAX_GRID_SIZE )
					break;
				if (m) offset2 = (grid2->height-1) * grid2->width;
				else offset2 = 0;
				for ( l = 0; l < grid2->width-1; l++) {
				//
					v1 = grid1->verts[grid1->width * k + offset1].xyz;
					v2 = grid2->verts[l + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) > .1)
						continue;
					if ( fabs(v1[1] - v2[1]) > .1)
						continue;
					if ( fabs(v1[2] - v2[2]) > .1)
						continue;

					v1 = grid1->verts[grid1->width * (k + 2) + offset1].xyz;
					v2 = grid2->verts[l + 1 + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) > .1)
						continue;
					if ( fabs(v1[1] - v2[1]) > .1)
						continue;
					if ( fabs(v1[2] - v2[2]) > .1)
						continue;
					//
					v1 = grid2->verts[l + offset2].xyz;
					v2 = grid2->verts[(l + 1) + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) < .01 &&
							fabs(v1[1] - v2[1]) < .01 &&
							fabs(v1[2] - v2[2]) < .01)
						continue;
					//
					//ri.Printf( PRINT_ALL, "found highest LoD crack between two patches\n" );
					// insert column into grid2 right after after column l
					if (m) row = grid2->height-1;
					else row = 0;
					grid2 = R_GridInsertColumn( grid2, l+1, row,
									grid1->verts[grid1->width * (k + 1) + offset1].xyz, grid1->heightLodError[k+1]);
					grid2->lodStitched = qfalse;
					s_worldData.surfaces[grid2num].data = (void *) grid2;
					return qtrue;
				}
			}
			for (m = 0; m < 2; m++) {

				if (grid2->height >= MAX_GRID_SIZE)
					break;
				if (m) offset2 = grid2->width-1;
				else offset2 = 0;
				for ( l = 0; l < grid2->height-1; l++) {
				//
					v1 = grid1->verts[grid1->width * k + offset1].xyz;
					v2 = grid2->verts[grid2->width * l + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) > .1)
						continue;
					if ( fabs(v1[1] - v2[1]) > .1)
						continue;
					if ( fabs(v1[2] - v2[2]) > .1)
						continue;

					v1 = grid1->verts[grid1->width * (k + 2) + offset1].xyz;
					v2 = grid2->verts[grid2->width * (l + 1) + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) > .1)
						continue;
					if ( fabs(v1[1] - v2[1]) > .1)
						continue;
					if ( fabs(v1[2] - v2[2]) > .1)
						continue;
					//
					v1 = grid2->verts[grid2->width * l + offset2].xyz;
					v2 = grid2->verts[grid2->width * (l + 1) + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) < .01 &&
							fabs(v1[1] - v2[1]) < .01 &&
							fabs(v1[2] - v2[2]) < .01)
						continue;
					//
					//ri.Printf( PRINT_ALL, "found highest LoD crack between two patches\n" );
					// insert row into grid2 right after after row l
					if (m) column = grid2->width-1;
					else column = 0;
					grid2 = R_GridInsertRow( grid2, l+1, column,
									grid1->verts[grid1->width * (k + 1) + offset1].xyz, grid1->heightLodError[k+1]);
					grid2->lodStitched = qfalse;
					s_worldData.surfaces[grid2num].data = (void *) grid2;
					return qtrue;
				}
			}
		}
	}
	for (n = 0; n < 2; n++) {
		//
		if (n) offset1 = (grid1->height-1) * grid1->width;
		else offset1 = 0;
		if (R_MergedWidthPoints(grid1, offset1))
			continue;
		for (k = grid1->width-1; k > 1; k -= 2) {

			for (m = 0; m < 2; m++) {

				if ( grid2->width >= MAX_GRID_SIZE )
					break;
				if (m) offset2 = (grid2->height-1) * grid2->width;
				else offset2 = 0;
				for ( l = 0; l < grid2->width-1; l++) {
				//
					v1 = grid1->verts[k + offset1].xyz;
					v2 = grid2->verts[l + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) > .1)
						continue;
					if ( fabs(v1[1] - v2[1]) > .1)
						continue;
					if ( fabs(v1[2] - v2[2]) > .1)
						continue;

					v1 = grid1->verts[k - 2 + offset1].xyz;
					v2 = grid2->verts[l + 1 + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) > .1)
						continue;
					if ( fabs(v1[1] - v2[1]) > .1)
						continue;
					if ( fabs(v1[2] - v2[2]) > .1)
						continue;
					//
					v1 = grid2->verts[l + offset2].xyz;
					v2 = grid2->verts[(l + 1) + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) < .01 &&
							fabs(v1[1] - v2[1]) < .01 &&
							fabs(v1[2] - v2[2]) < .01)
						continue;
					//
					//ri.Printf( PRINT_ALL, "found highest LoD crack between two patches\n" );
					// insert column into grid2 right after after column l
					if (m) row = grid2->height-1;
					else row = 0;
					grid2 = R_GridInsertColumn( grid2, l+1, row,
										grid1->verts[k - 1 + offset1].xyz, grid1->widthLodError[k+1]);
					grid2->lodStitched = qfalse;
					s_worldData.surfaces[grid2num].data = (void *) grid2;
					return qtrue;
				}
			}
			for (m = 0; m < 2; m++) {

				if (grid2->height >= MAX_GRID_SIZE)
					break;
				if (m) offset2 = grid2->width-1;
				else offset2 = 0;
				for ( l = 0; l < grid2->height-1; l++) {
				//
					v1 = grid1->verts[k + offset1].xyz;
					v2 = grid2->verts[grid2->width * l + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) > .1)
						continue;
					if ( fabs(v1[1] - v2[1]) > .1)
						continue;
					if ( fabs(v1[2] - v2[2]) > .1)
						continue;

					v1 = grid1->verts[k - 2 + offset1].xyz;
					v2 = grid2->verts[grid2->width * (l + 1) + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) > .1)
						continue;
					if ( fabs(v1[1] - v2[1]) > .1)
						continue;
					if ( fabs(v1[2] - v2[2]) > .1)
						continue;
					//
					v1 = grid2->verts[grid2->width * l + offset2].xyz;
					v2 = grid2->verts[grid2->width * (l + 1) + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) < .01 &&
							fabs(v1[1] - v2[1]) < .01 &&
							fabs(v1[2] - v2[2]) < .01)
						continue;
					//
					//ri.Printf( PRINT_ALL, "found highest LoD crack between two patches\n" );
					// insert row into grid2 right after after row l
					if (m) column = grid2->width-1;
					else column = 0;
					grid2 = R_GridInsertRow( grid2, l+1, column,
										grid1->verts[k - 1 + offset1].xyz, grid1->widthLodError[k+1]);
					if (!grid2)
						break;
					grid2->lodStitched = qfalse;
					s_worldData.surfaces[grid2num].data = (void *) grid2;
					return qtrue;
				}
			}
		}
	}
	for (n = 0; n < 2; n++) {
		//
		if (n) offset1 = grid1->width-1;
		else offset1 = 0;
		if (R_MergedHeightPoints(grid1, offset1))
			continue;
		for (k = grid1->height-1; k > 1; k -= 2) {
			for (m = 0; m < 2; m++) {

				if ( grid2->width >= MAX_GRID_SIZE )
					break;
				if (m) offset2 = (grid2->height-1) * grid2->width;
				else offset2 = 0;
				for ( l = 0; l < grid2->width-1; l++) {
				//
					v1 = grid1->verts[grid1->width * k + offset1].xyz;
					v2 = grid2->verts[l + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) > .1)
						continue;
					if ( fabs(v1[1] - v2[1]) > .1)
						continue;
					if ( fabs(v1[2] - v2[2]) > .1)
						continue;

					v1 = grid1->verts[grid1->width * (k - 2) + offset1].xyz;
					v2 = grid2->verts[l + 1 + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) > .1)
						continue;
					if ( fabs(v1[1] - v2[1]) > .1)
						continue;
					if ( fabs(v1[2] - v2[2]) > .1)
						continue;
					//
					v1 = grid2->verts[l + offset2].xyz;
					v2 = grid2->verts[(l + 1) + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) < .01 &&
							fabs(v1[1] - v2[1]) < .01 &&
							fabs(v1[2] - v2[2]) < .01)
						continue;
					//
					//ri.Printf( PRINT_ALL, "found highest LoD crack between two patches\n" );
					// insert column into grid2 right after after column l
					if (m) row = grid2->height-1;
					else row = 0;
					grid2 = R_GridInsertColumn( grid2, l+1, row,
										grid1->verts[grid1->width * (k - 1) + offset1].xyz, grid1->heightLodError[k+1]);
					grid2->lodStitched = qfalse;
					s_worldData.surfaces[grid2num].data = (void *) grid2;
					return qtrue;
				}
			}
			for (m = 0; m < 2; m++) {

				if (grid2->height >= MAX_GRID_SIZE)
					break;
				if (m) offset2 = grid2->width-1;
				else offset2 = 0;
				for ( l = 0; l < grid2->height-1; l++) {
				//
					v1 = grid1->verts[grid1->width * k + offset1].xyz;
					v2 = grid2->verts[grid2->width * l + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) > .1)
						continue;
					if ( fabs(v1[1] - v2[1]) > .1)
						continue;
					if ( fabs(v1[2] - v2[2]) > .1)
						continue;

					v1 = grid1->verts[grid1->width * (k - 2) + offset1].xyz;
					v2 = grid2->verts[grid2->width * (l + 1) + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) > .1)
						continue;
					if ( fabs(v1[1] - v2[1]) > .1)
						continue;
					if ( fabs(v1[2] - v2[2]) > .1)
						continue;
					//
					v1 = grid2->verts[grid2->width * l + offset2].xyz;
					v2 = grid2->verts[grid2->width * (l + 1) + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) < .01 &&
							fabs(v1[1] - v2[1]) < .01 &&
							fabs(v1[2] - v2[2]) < .01)
						continue;
					//
					//ri.Printf( PRINT_ALL, "found highest LoD crack between two patches\n" );
					// insert row into grid2 right after after row l
					if (m) column = grid2->width-1;
					else column = 0;
					grid2 = R_GridInsertRow( grid2, l+1, column,
										grid1->verts[grid1->width * (k - 1) + offset1].xyz, grid1->heightLodError[k+1]);
					grid2->lodStitched = qfalse;
					s_worldData.surfaces[grid2num].data = (void *) grid2;
					return qtrue;
				}
			}
		}
	}
	return qfalse;
}

/*
===============
R_TryStitchPatch

This function will try to stitch patches in the same LoD group together for the highest LoD.

Only single missing vertice cracks will be fixed.

Vertices will be joined at the patch side a crack is first found, at the other side
of the patch (on the same row or column) the vertices will not be joined and cracks
might still appear at that side.
===============
*/
int R_TryStitchingPatch( int grid1num ) {
	int j, numstitches;
	srfGridMesh_t *grid1, *grid2;

	numstitches = 0;
	grid1 = (srfGridMesh_t *) s_worldData.surfaces[grid1num].data;
	for ( j = 0; j < s_worldData.numsurfaces; j++ ) {
		//
		grid2 = (srfGridMesh_t *) s_worldData.surfaces[j].data;
		// if this surface is not a grid
		if ( grid2->surfaceType != SF_GRID ) continue;
		// grids in the same LOD group should have the exact same lod radius
		if ( grid1->lodRadius != grid2->lodRadius ) continue;
		// grids in the same LOD group should have the exact same lod origin
		if ( grid1->lodOrigin[0] != grid2->lodOrigin[0] ) continue;
		if ( grid1->lodOrigin[1] != grid2->lodOrigin[1] ) continue;
		if ( grid1->lodOrigin[2] != grid2->lodOrigin[2] ) continue;
		//
		while (R_StitchPatches(grid1num, j))
		{
			numstitches++;
		}
	}
	return numstitches;
}

/*
===============
R_StitchAllPatches
===============
*/
void R_StitchAllPatches( void ) {
	int i, stitched, numstitches;
	srfGridMesh_t *grid1;

	numstitches = 0;
	do
	{
		stitched = qfalse;
		for ( i = 0; i < s_worldData.numsurfaces; i++ ) {
			//
			grid1 = (srfGridMesh_t *) s_worldData.surfaces[i].data;
			// if this surface is not a grid
			if ( grid1->surfaceType != SF_GRID )
				continue;
			//
			if ( grid1->lodStitched )
				continue;
			//
			grid1->lodStitched = qtrue;
			stitched = qtrue;
			//
			numstitches += R_TryStitchingPatch( i );
		}
	}
	while (stitched);
	ri.Printf( PRINT_ALL, "stitched %d LoD cracks\n", numstitches );
}

/*
===============
R_MovePatchSurfacesToHunk
===============
*/
void R_MovePatchSurfacesToHunk(void) {
	int i, size;
	srfGridMesh_t *grid, *hunkgrid;

	for ( i = 0; i < s_worldData.numsurfaces; i++ ) {
		//
		grid = (srfGridMesh_t *) s_worldData.surfaces[i].data;
		// if this surface is not a grid
		if ( grid->surfaceType != SF_GRID )
			continue;
		//
		size = (grid->width * grid->height - 1) * sizeof( drawVert_t ) + sizeof( *grid );
		hunkgrid = ri.Hunk_Alloc( size );
		Com_Memcpy(hunkgrid, grid, size);

		hunkgrid->widthLodError = ri.Hunk_Alloc( grid->width * 4 );
		Com_Memcpy( hunkgrid->widthLodError, grid->widthLodError, grid->width * 4 );

		hunkgrid->heightLodError = ri.Hunk_Alloc( grid->height * 4 );
		Com_Memcpy( grid->heightLodError, grid->heightLodError, grid->height * 4 );

		R_FreeSurfaceGridMesh( grid );

		s_worldData.surfaces[i].data = (void *) hunkgrid;
	}
}

/*
===============
R_LoadSurfaces
===============
*/
static	void R_LoadSurfaces(gamelump_t* surfs, gamelump_t* verts, gamelump_t* indexLump) {
	dsurface_t	*in;
	msurface_t	*out;
	drawVert_t	*dv;
	int			*indexes;
	int			count;
	int			numFaces, numMeshes, numTriSurfs, numFlares;
	int			i;

	numFaces = 0;
	numMeshes = 0;
	numTriSurfs = 0;
	numFlares = 0;

    if (surfs->length % sizeof(*in))
        ri.Error(ERR_DROP, "LoadMap: funny lump size in %s", s_worldData.name);
    count = surfs->length / sizeof(*in);

    dv = (void*)verts->buffer;
    if (verts->length % sizeof(*dv))
        ri.Error(ERR_DROP, "LoadMap: funny lump size in %s", s_worldData.name);

    indexes = (void*)indexLump->buffer;
    if (indexLump->length % sizeof(*indexes))
        ri.Error(ERR_DROP, "LoadMap: funny lump size in %s", s_worldData.name);

    in = (void*)surfs->buffer;
	out = ri.Hunk_Alloc ( count * sizeof(*out) );	

	s_worldData.surfaces = out;
	s_worldData.numsurfaces = count;

	for ( i = 0 ; i < count ; i++, in++, out++ ) {
		switch ( LittleLong( in->surfaceType ) ) {
		case MST_PATCH:
			ParseMesh ( in, dv, out );
			numMeshes++;
			break;
		case MST_TRIANGLE_SOUP:
			ParseTriSurf( in, dv, out, indexes );
			numTriSurfs++;
			break;
		case MST_PLANAR:
			ParseFace( in, dv, out, indexes );
			numFaces++;
			break;
		case MST_FLARE:
			ParseFlare( in, dv, out, indexes );
			numFlares++;
			break;
		default:
			ri.Error( ERR_DROP, "Bad surfaceType" );
		}
	}

#ifdef PATCH_STITCHING
	R_StitchAllPatches();
#endif

	R_FixSharedVertexLodError();

#ifdef PATCH_STITCHING
	R_MovePatchSurfacesToHunk();
#endif

	ri.Printf( PRINT_ALL, "...loaded %d faces, %i meshes, %i trisurfs, %i flares\n", 
		numFaces, numMeshes, numTriSurfs, numFlares );
}



/*
=================
R_LoadSubmodels
=================
*/
static	void R_LoadSubmodels(gamelump_t* l) {
	dmodel_t	*in;
	bmodel_t	*out;
	int			i, j, count;

    in = (void*)l->buffer;
    if (l->length % sizeof(*in))
        ri.Error(ERR_DROP, "LoadMap: funny lump size in %s", s_worldData.name);
    count = l->length / sizeof(*in);

	s_worldData.bmodels = out = ri.Hunk_Alloc( count * sizeof(*out) );

	for ( i=0 ; i<count ; i++, in++, out++ ) {
		model_t *model;

		model = R_AllocModel();

		assert( model != NULL );			// this should never happen

		model->type = MOD_BRUSH;
		model->d.bmodel = out;
		Com_sprintf( model->name, sizeof( model->name ), "*%d", i );

		for (j=0 ; j<3 ; j++) {
			out->bounds[0][j] = LittleFloat (in->mins[j]);
			out->bounds[1][j] = LittleFloat (in->maxs[j]);
		}

		out->firstSurface = s_worldData.surfaces + LittleLong( in->firstSurface );
		out->numSurfaces = LittleLong( in->numSurfaces );
	}
}



//==================================================================

/*
=================
R_SetParent
=================
*/
static	void R_SetParent (mnode_t *node, mnode_t *parent)
{
	node->parent = parent;
	if (node->contents != -1)
		return;
	R_SetParent (node->children[0], node);
	R_SetParent (node->children[1], node);
}

/*
=================
R_LoadNodesAndLeafs
=================
*/
static	void R_LoadNodesAndLeafs(gamelump_t* nodeLump, gamelump_t* leafLump) {
    int			i, j, p;
    dnode_t* in;
    dleaf_t* inLeaf;
    mnode_t* out;
    int			numNodes, numLeafs;

    in = (void*)nodeLump->buffer;
    if (nodeLump->length % sizeof(dnode_t) ||
        leafLump->length % sizeof(dleaf_t)) {
        ri.Error(ERR_DROP, "LoadMap: funny lump size in %s", s_worldData.name);
    }
    numNodes = nodeLump->length / sizeof(dnode_t);
    numLeafs = leafLump->length / sizeof(dleaf_t);

    out = ri.Hunk_Alloc((numNodes + numLeafs) * sizeof(*out));

    s_worldData.nodes = out;
    s_worldData.numnodes = numNodes + numLeafs;
    s_worldData.numDecisionNodes = numNodes;

    // load nodes
    for (i = 0; i < numNodes; i++, in++, out++)
    {
        for (j = 0; j < 3; j++)
        {
            out->mins[j] = LittleLong(in->mins[j]);
            out->maxs[j] = LittleLong(in->maxs[j]);
        }

        p = LittleLong(in->planeNum);
        out->plane = s_worldData.planes + p;

        out->contents = CONTENTS_NODE;	// differentiate from leafs

        for (j = 0; j < 2; j++)
        {
            p = LittleLong(in->children[j]);
            if (p >= 0)
                out->children[j] = s_worldData.nodes + p;
            else
                out->children[j] = s_worldData.nodes + numNodes + (-1 - p);
        }
    }

    // load leafs
    inLeaf = (void*)leafLump->buffer;
    for (i = 0; i < numLeafs; i++, inLeaf++, out++)
    {
        for (j = 0; j < 3; j++)
        {
            out->mins[j] = LittleLong(inLeaf->mins[j]);
            out->maxs[j] = LittleLong(inLeaf->maxs[j]);
        }

        out->cluster = LittleLong(inLeaf->cluster);
        out->area = LittleLong(inLeaf->area);

        if (out->cluster >= s_worldData.numClusters) {
            s_worldData.numClusters = out->cluster + 1;
        }

        out->firstmarksurface = s_worldData.marksurfaces +
            LittleLong(inLeaf->firstLeafSurface);
        out->nummarksurfaces = LittleLong(inLeaf->numLeafSurfaces);

        out->firstTerraPatch = LittleLong(inLeaf->firstTerraPatch);
        out->numTerraPatches = LittleLong(inLeaf->numTerraPatches);

        out->firstStaticModel = LittleLong(inLeaf->firstStaticModel);
        out->numStaticModels = LittleLong(inLeaf->numStaticModels);
    }

    // chain decendants
    R_SetParent(s_worldData.nodes, NULL);
}

/*
=================
R_LoadNodesAndLeafsOld
=================
*/
static	void R_LoadNodesAndLeafsOld(gamelump_t* nodeLump, gamelump_t* leafLump) {
    int			i, j, p;
    dnode_t* in;
    dleaf_t_ver17* inLeaf;
    mnode_t* out;
    int			numNodes, numLeafs;

    in = nodeLump->buffer;
    if (nodeLump->length % sizeof(dnode_t) ||
        leafLump->length % sizeof(dleaf_t)) {
        ri.Error(ERR_DROP, "LoadMap: funny lump size in %s", s_worldData.name);
    }
    numNodes = nodeLump->length / sizeof(dnode_t);
    numLeafs = leafLump->length / sizeof(dleaf_t);

    out = ri.Hunk_Alloc((numNodes + numLeafs) * sizeof(*out));

    s_worldData.nodes = out;
    s_worldData.numnodes = numNodes + numLeafs;
    s_worldData.numDecisionNodes = numNodes;

    // load nodes
    for (i = 0; i < numNodes; i++, in++, out++)
    {
        for (j = 0; j < 3; j++)
        {
            out->mins[j] = LittleLong(in->mins[j]);
            out->maxs[j] = LittleLong(in->maxs[j]);
        }

        p = LittleLong(in->planeNum);
        out->plane = s_worldData.planes + p;

        out->contents = CONTENTS_NODE;	// differentiate from leafs

        for (j = 0; j < 2; j++)
        {
            p = LittleLong(in->children[j]);
            if (p >= 0)
                out->children[j] = s_worldData.nodes + p;
            else
                out->children[j] = s_worldData.nodes + numNodes + (-1 - p);
        }
    }

    // load leafs
    inLeaf = leafLump->buffer;
    for (i = 0; i < numLeafs; i++, inLeaf++, out++)
    {
        for (j = 0; j < 3; j++)
        {
            out->mins[j] = LittleLong(inLeaf->mins[j]);
            out->maxs[j] = LittleLong(inLeaf->maxs[j]);
        }

        out->cluster = LittleLong(inLeaf->cluster);
        out->area = LittleLong(inLeaf->area);

        if (out->cluster >= s_worldData.numClusters) {
            s_worldData.numClusters = out->cluster + 1;
        }

        out->firstmarksurface = s_worldData.marksurfaces +
            LittleLong(inLeaf->firstLeafSurface);
        out->nummarksurfaces = LittleLong(inLeaf->numLeafSurfaces);

        out->firstTerraPatch = LittleLong(inLeaf->firstTerraPatch);
        out->numTerraPatches = LittleLong(inLeaf->numTerraPatches);
    }

    // chain decendants
    R_SetParent(s_worldData.nodes, NULL);
}

//=============================================================================

/*
=================
R_LoadShaders
=================
*/
static	void R_LoadShaders(gamelump_t* l) {
    int		i, count;
    dshader_t* in, * out;

    in = l->buffer;
    if (l->length % sizeof(*in))
        ri.Error(ERR_DROP, "LoadMap: funny lump size in %s", s_worldData.name);
    count = l->length / sizeof(*in);
    out = ri.Hunk_Alloc(count * sizeof(*out));

    s_worldData.shaders = out;
    s_worldData.numShaders = count;

    Com_Memcpy(out, in, count * sizeof(*out));

    for (i = 0; i < count; i++) {
        out[i].surfaceFlags = LittleLong(out[i].surfaceFlags);
        out[i].contentFlags = LittleLong(out[i].contentFlags);
    }
}


/*
=================
R_LoadMarksurfaces
=================
*/
static	void R_LoadMarksurfaces(gamelump_t* l)
{
    int		i, j, count;
    int* in;
    msurface_t** out;

    in = l->buffer;
    if (l->length % sizeof(*in))
        ri.Error(ERR_DROP, "LoadMap: funny lump size in %s", s_worldData.name);
    count = l->length / sizeof(*in);
    out = (msurface_t**)ri.Hunk_Alloc(count * sizeof(*out));

    s_worldData.marksurfaces = out;
    s_worldData.nummarksurfaces = count;

    for (i = 0; i < count; i++)
    {
        j = LittleLong(in[i]);
        out[i] = &s_worldData.surfaces[j];
    }
}


/*
=================
R_LoadPlanes
=================
*/
static	void R_LoadPlanes(gamelump_t* l) {
    int			i, j;
    cplane_t* out;
    dplane_t* in;
    int			count;
    int			bits;

    in = l->buffer;
    if (l->length % sizeof(*in))
        ri.Error(ERR_DROP, "LoadMap: funny lump size in %s", s_worldData.name);
    count = l->length / sizeof(*in);
    out = ri.Hunk_Alloc(count * 2 * sizeof(*out));

    s_worldData.planes = out;
    s_worldData.numplanes = count;

    for (i = 0; i < count; i++, in++, out++) {
        bits = 0;
        for (j = 0; j < 3; j++) {
            out->normal[j] = LittleFloat(in->normal[j]);
            if (out->normal[j] < 0) {
                bits |= 1 << j;
            }
        }

        out->dist = LittleFloat(in->dist);
        out->type = PlaneTypeForNormal(out->normal);
        out->signbits = bits;
    }
}

/*
=================
R_LoadFogs

=================
*/
static	void R_LoadFogs(gamelump_t* l, gamelump_t* brushesLump, gamelump_t* sidesLump) {
    int			i;
    fog_t* out;
    dfog_t* fogs;
    dbrush_t* brushes, * brush;
    dbrushside_t* sides;
    int			count, brushesCount, sidesCount;
    int			sideNum;
    int			planeNum;
    shader_t* shader;
    float		d;
    int			firstSide;

    fogs = l->buffer;
    if (l->length % sizeof(*fogs)) {
        ri.Error(ERR_DROP, "LoadMap: funny lump size in %s", s_worldData.name);
    }
    count = l->length / sizeof(*fogs);

    // create fog strucutres for them
    s_worldData.numfogs = count + 1;
    s_worldData.fogs = ri.Hunk_Alloc(s_worldData.numfogs * sizeof(*out));
    out = s_worldData.fogs + 1;

    if (!count) {
        return;
    }

    brushes = brushesLump->buffer;
    if (brushesLump->length % sizeof(*brushes)) {
        ri.Error(ERR_DROP, "LoadMap: funny lump size in %s", s_worldData.name);
    }
    brushesCount = brushesLump->length / sizeof(*brushes);

    sides = sidesLump->buffer;
    if (sidesLump->length % sizeof(*sides)) {
        ri.Error(ERR_DROP, "LoadMap: funny lump size in %s", s_worldData.name);
    }
    sidesCount = sidesLump->length / sizeof(*sides);

    for (i = 0; i < count; i++, fogs++) {
        out->originalBrushNumber = LittleLong(fogs->brushNum);

        if ((unsigned)out->originalBrushNumber >= brushesCount) {
            ri.Error(ERR_DROP, "fog brushNumber out of range");
        }
        brush = brushes + out->originalBrushNumber;

        firstSide = LittleLong(brush->firstSide);

        if ((unsigned)firstSide > sidesCount - 6) {
            ri.Error(ERR_DROP, "fog brush sideNumber out of range");
        }

        // brushes are always sorted with the axial sides first
        sideNum = firstSide + 0;
        planeNum = LittleLong(sides[sideNum].planeNum);
        out->bounds[0][0] = -s_worldData.planes[planeNum].dist;

        sideNum = firstSide + 1;
        planeNum = LittleLong(sides[sideNum].planeNum);
        out->bounds[1][0] = s_worldData.planes[planeNum].dist;

        sideNum = firstSide + 2;
        planeNum = LittleLong(sides[sideNum].planeNum);
        out->bounds[0][1] = -s_worldData.planes[planeNum].dist;

        sideNum = firstSide + 3;
        planeNum = LittleLong(sides[sideNum].planeNum);
        out->bounds[1][1] = s_worldData.planes[planeNum].dist;

        sideNum = firstSide + 4;
        planeNum = LittleLong(sides[sideNum].planeNum);
        out->bounds[0][2] = -s_worldData.planes[planeNum].dist;

        sideNum = firstSide + 5;
        planeNum = LittleLong(sides[sideNum].planeNum);
        out->bounds[1][2] = s_worldData.planes[planeNum].dist;

        // get information from the shader for fog parameters
        shader = R_FindShader(fogs->shader, LIGHTMAP_NONE, qtrue, qtrue, qtrue, qtrue);

        out->parms = shader->fogParms;

        out->colorInt = ColorBytes4(shader->fogParms.color[0] * tr.identityLight,
            shader->fogParms.color[1] * tr.identityLight,
            shader->fogParms.color[2] * tr.identityLight, 1.0);

        d = shader->fogParms.depthForOpaque < 1 ? 1 : shader->fogParms.depthForOpaque;
        out->tcScale = 1.0f / (d * 8);

        // set the gradient vector
        sideNum = LittleLong(fogs->visibleSide);

        if (sideNum == -1) {
            out->hasSurface = qfalse;
        }
        else {
            out->hasSurface = qtrue;
            planeNum = LittleLong(sides[firstSide + sideNum].planeNum);
            VectorSubtract(vec3_origin, s_worldData.planes[planeNum].normal, out->surface);
            out->surface[3] = -s_worldData.planes[planeNum].dist;
        }

        out++;
    }

}


/*
================
R_LoadLightGrid

================
*/
void R_LoadLightGrid(gamelump_t* l) {
	int		i;
	vec3_t	maxs;
	int		numGridPoints;
	world_t	*w;
	float	*wMins, *wMaxs;

	w = &s_worldData;

	w->lightGridInverseSize[0] = 1.0f / w->lightGridSize[0];
	w->lightGridInverseSize[1] = 1.0f / w->lightGridSize[1];
	w->lightGridInverseSize[2] = 1.0f / w->lightGridSize[2];

	wMins = w->bmodels[0].bounds[0];
	wMaxs = w->bmodels[0].bounds[1];

	for ( i = 0 ; i < 3 ; i++ ) {
		w->lightGridOrigin[i] = w->lightGridSize[i] * ceil( wMins[i] / w->lightGridSize[i] );
		maxs[i] = w->lightGridSize[i] * floor( wMaxs[i] / w->lightGridSize[i] );
		w->lightGridBounds[i] = (maxs[i] - w->lightGridOrigin[i])/w->lightGridSize[i] + 1;
	}

	numGridPoints = w->lightGridBounds[0] * w->lightGridBounds[1] * w->lightGridBounds[2];

    if (l->length != numGridPoints * 8) {
        ri.Printf(PRINT_WARNING, "WARNING: light grid mismatch\n");
        w->lightGridData = NULL;
        return;
    }

    w->lightGridData = ri.Hunk_Alloc(l->length);
    Com_Memcpy(w->lightGridData, (void*)l->buffer, l->length);

	// deal with overbright bits
	for ( i = 0 ; i < numGridPoints ; i++ ) {
		R_ColorShiftLightingBytes( &w->lightGridData[i*8], &w->lightGridData[i*8] );
		R_ColorShiftLightingBytes( &w->lightGridData[i*8+3], &w->lightGridData[i*8+3] );
	}
}

/*
================
R_LoadStaticModelData
================
*/
void R_LoadStaticModelData(gamelump_t* lump) {
    int		i;
    byte* pDstColors;
    byte* pSrcColors;

    if (!lump->length) {
        s_worldData.numStaticModelData = 0;
        s_worldData.staticModelData = 0;
        return;
    }


    if (lump->length % (sizeof(byte) * 3)) {
        Com_Error(1, "R_LoadStaticModelData: funny lump size");
    }

    s_worldData.numStaticModelData = lump->length / (sizeof(byte) * 3);
    s_worldData.staticModelData = ri.Hunk_Alloc(s_worldData.numStaticModelData * (sizeof(byte) * 4));

    pSrcColors = lump->buffer;
    pDstColors = s_worldData.staticModelData;

    for (i = 0; i < lump->length; i += 3)
    {
        pDstColors[0] = pSrcColors[0];
        pDstColors[1] = pSrcColors[1];
        pDstColors[2] = pSrcColors[2];
        pDstColors[3] = -1;

        pSrcColors += sizeof(byte) * 3;
        pDstColors += sizeof(byte) * 4;
    }
}

/*
================
R_CopyStaticModel
================
*/
void R_CopyStaticModel(cStaticModel_t* pSM, cStaticModelUnpacked_t* pUnpackedSM) {
    pUnpackedSM->visCount = 0;
    VectorCopy(pSM->angles, pUnpackedSM->angles);
    VectorCopy(pSM->origin, pUnpackedSM->origin);
    pUnpackedSM->scale = pSM->scale;
    pUnpackedSM->firstVertexData = pSM->firstVertexData * 4 / 3;
    pUnpackedSM->numVertexData = pSM->numVertexData;
    memcpy(pUnpackedSM->model, pSM->model, sizeof(pUnpackedSM->model));
}

/*
================
R_LoadStaticModelDefs
================
*/
void R_LoadStaticModelDefs(gamelump_t* lump) {
    int		i;
    cStaticModel_t* in;
    cStaticModelUnpacked_t* out;

    if (!lump->length) {
        s_worldData.numStaticModels = 0;
        s_worldData.staticModels = NULL;
        return;
    }

    if (lump->length % sizeof(cStaticModel_t)) {
        Com_Error(ERR_DROP, "R_LoadStaticModelDefs: funny lump size");
    }

    s_worldData.numStaticModels = lump->length / sizeof(cStaticModel_t);
    s_worldData.staticModels = ri.Hunk_Alloc(s_worldData.numStaticModels * sizeof(cStaticModelUnpacked_t));

    in = lump->buffer;
    out = s_worldData.staticModels;

    for (i = 0; i < s_worldData.numStaticModels; in++, out++, i++) {
        R_CopyStaticModel(in, out);
    }
}

/*
================
R_LoadStaticModelIndexes
================
*/
void R_LoadStaticModelIndexes(gamelump_t* lump) {
    int		i;
    short* in;
    cStaticModelUnpacked_t** out;

    if (lump->length % sizeof(short)) {
        Com_Error(ERR_DROP, "R_LoadStaticModelDefs: funny lump size");
    }

    s_worldData.numVisStaticModels = lump->length / sizeof(short);
    s_worldData.visStaticModels = ri.Hunk_Alloc(s_worldData.numVisStaticModels * sizeof(cStaticModelUnpacked_t*));

    in = lump->buffer;
    out = s_worldData.visStaticModels;

    for (i = 0; i < s_worldData.numVisStaticModels; in++, out++, i++) {
        *out = &s_worldData.staticModels[*in];
    }
}

/*
==================
R_LoadLump

Loads a lump from the BSP file
==================
*/
int R_LoadLump(fileHandle_t handle, lump_t* lump, gamelump_t* glump, int size)
{
    glump->buffer = NULL;
    glump->length = lump->filelen;

    if (lump->filelen) {
        glump->buffer = Hunk_AllocateTempMemory(lump->filelen);

        if (FS_Seek(handle, lump->fileofs, FS_SEEK_SET)) {
            Com_Error(ERR_DROP, "R_LoadLump: Error seeking to lump.");
        }

        FS_Read(glump->buffer, lump->filelen, handle);

        if (size) {
            return lump->filelen / size;
        }
    }

    return 0;
}

/*
==================
R_FreeLump

Free a previously allocated lump
==================
*/
void R_FreeLump(gamelump_t* lump)
{
    if (lump->buffer)
    {
        Hunk_FreeTempMemory(lump->buffer);
        lump->buffer = NULL;
        lump->length = 0;
    }
}

#define _R( id ) UI_LoadResource( "*" #id )

/*
=================
RE_LoadWorldMap

Called directly from cgame
=================
*/
void RE_LoadWorldMap( const char *name ) {
	int			i;
    dheader_t	header;
    fileHandle_t	h;
    int				length;
	vec3_t			vDefSundir;
    gamelump_t		lump, lump2, lump3;

    vDefSundir[0] = 0.45f;
    vDefSundir[1] = 0.3f;
    vDefSundir[2] = 0.9f;

	_R(52);

	// set default sun direction to be used if it isn't
	// overridden by a shader
	VectorCopy(vDefSundir, tr.sunDirection);
	VectorNormalize( tr.sunDirection );

	tr.worldMapLoaded = qtrue;

    // load it
    length = ri.FS_OpenFile(name, &h, qtrue, qtrue);
    if (length <= 0) {
        ri.Error(ERR_DROP, "RE_LoadWorldMap: %s not found", name);
    }

    ri.FS_Read(&header, sizeof(dheader_t), h);
    map_length = length;
    map_version = header.version;

    header.ident = LittleLong(header.ident);

	// clear tr.world so if the level fails to load, the next
	// try will not look at the partially loaded version
	tr.world = NULL;

	Com_Memset( &s_worldData, 0, sizeof( s_worldData ) );
	Q_strncpyz( s_worldData.name, name, sizeof( s_worldData.name ) );

	Q_strncpyz( s_worldData.baseName, COM_SkipPath( s_worldData.name ), sizeof( s_worldData.name ) );
	COM_StripExtension( s_worldData.baseName, s_worldData.baseName, sizeof(s_worldData.baseName));

	c_gridVerts = 0;

    i = LittleLong(header.version);
    if (i < BSP_BASE_VERSION || i > BSP_VERSION) {
        ri.Error(ERR_DROP, "RE_LoadWorldMap: %s has wrong version number (%i should be between %i and %i)",
            name, i, BSP_BASE_VERSION, BSP_VERSION);
    }

	// load into heap
    // load into heap
    _R(53);
    numShaders = R_LoadLump(h, &header.lumps[LUMP_SHADERS], &lump, sizeof(dshader_t));
    _R(54);
    R_LoadShaders(&lump);
    _R(55);
    R_FreeLump(&lump);
    _R(56);
    numplanes = R_LoadLump(h, &header.lumps[LUMP_PLANES], &lump, sizeof(dplane_t));
    R_LoadPlanes(&lump);
    _R(57);
    R_FreeLump(&lump);
    _R(58);
    numLightBytes = R_LoadLump(h, &header.lumps[LUMP_LIGHTMAPS], &lump2, sizeof(byte));
    _R(59);
    numDrawSurfaces = R_LoadLump(h, &header.lumps[LUMP_SURFACES], &lump, sizeof(dsurface_t));
    _R(60);
    R_LoadLightmaps(&lump2, &lump);
    _R(61);
    R_FreeLump(&lump2);
    _R(62);
    numDrawVerts = R_LoadLump(h, &header.lumps[LUMP_DRAWVERTS], &lump2, sizeof(drawVert_t));
    _R(63);
    numDrawIndexes = R_LoadLump(h, &header.lumps[LUMP_DRAWINDEXES], &lump3, sizeof(int));
    _R(64);
    R_LoadSurfaces(&lump, &lump2, &lump3);
    _R(65);
    R_FreeLump(&lump3);
    _R(66);
    R_FreeLump(&lump2);
    _R(67);
    R_FreeLump(&lump);
    _R(68);
    numleafsurfaces = R_LoadLump(h, &header.lumps[LUMP_LEAFSURFACES], &lump, sizeof(int));
    _R(69);
    R_LoadMarksurfaces(&lump);
    _R(70);
    R_FreeLump(&lump);
    _R(71);
    numleafs = R_LoadLump(h, &header.lumps[LUMP_LEAFS], &lump2, sizeof(dleaf_t));
    _R(72);
    numnodes = R_LoadLump(h, &header.lumps[LUMP_NODES], &lump, sizeof(mnode_t));
    _R(73);
    if (header.version > BSP_BETA_VERSION) {
        R_LoadNodesAndLeafs(&lump, &lump2);
    }
    else {
        R_LoadNodesAndLeafsOld(&lump, &lump2);
    }
    _R(74);
    R_FreeLump(&lump);
    _R(75);
    R_FreeLump(&lump2);
    _R(76);
    numbrushsides = R_LoadLump(h, &header.lumps[LUMP_BRUSHSIDES], &lump2, sizeof(dbrushside_t));
    numbrushes = R_LoadLump(h, &header.lumps[LUMP_BRUSHES], &lump, sizeof(dbrush_t));
    numFogs = R_LoadLump(h, &header.lumps[LUMP_DUMMY10], &lump3, sizeof(dfog_t));
    R_LoadFogs(&lump3, &lump, &lump2);
    R_FreeLump(&lump);
    R_FreeLump(&lump2);
    R_FreeLump(&lump3);
    nummodels = R_LoadLump(h, &header.lumps[LUMP_MODELS], &lump, sizeof(bmodel_t));
    _R(77);
    R_LoadSubmodels(&lump);
    _R(78);
    R_FreeLump(&lump);
    _R(79);
    numVisBytes = R_LoadLump(h, &header.lumps[LUMP_VISIBILITY], &lump, sizeof(byte));
    _R(80);
    R_LoadVisibility(&lump);
    _R(81);
    R_FreeLump(&lump);
    _R(82);
    g_iGridPaletteBytes = R_LoadLump(h, &header.lumps[LUMP_LIGHTGRIDPALETTE], &lump, sizeof(byte));
    _R(83);
    g_iGridOffsets = R_LoadLump(h, &header.lumps[LUMP_LIGHTGRIDOFFSETS], &lump2, sizeof(short));
    _R(84);
    g_iGridDataSize = R_LoadLump(h, &header.lumps[LUMP_LIGHTGRIDDATA], &lump3, sizeof(byte));
    _R(85);
    //R_LoadLightGrid( &lump, &lump2, &lump3 );
    R_LoadLightGrid(&lump3);
    _R(86);
    R_FreeLump(&lump);
    _R(87);
    R_FreeLump(&lump2);
    _R(88);
    R_FreeLump(&lump3);
    _R(89);
    numSLights = R_LoadLump(h, &header.lumps[LUMP_SPHERELIGHTS], &lump, sizeof(mapspherel_t));
    _R(90);
    R_LoadSphereLights(&lump);
    _R(91);
    R_FreeLump(&lump);
    _R(92);
    entLightVisSize = R_LoadLump(h, &header.lumps[LUMP_SPHERELIGHTVIS], &lump, sizeof(byte));
    _R(93);
    R_LoadSphereLightVis(&lump);
    _R(94);
    R_FreeLump(&lump);
    _R(95);
    g_nTerraPatches = R_LoadLump(h, &header.lumps[LUMP_TERRAIN], &lump, sizeof(cTerraPatch_t));
    _R(96);
    R_LoadTerrain(&lump);
    _R(97);
    R_FreeLump(&lump);
    _R(98);
    g_nTerPatchIndices = R_LoadLump(h, &header.lumps[LUMP_TERRAININDEXES], &lump, sizeof(short));
    _R(99);
    R_LoadTerrainIndexes(&lump);
    _R(100);
    R_FreeLump(&lump);
    _R(101);
    if (header.version > BSP_BETA_VERSION)
    {
        g_nStaticModelData = R_LoadLump(h, &header.lumps[LUMP_STATICMODELDATA], &lump, sizeof(byte));
        _R(102);
        R_LoadStaticModelData(&lump);
        _R(103);
        R_FreeLump(&lump);
        _R(104);
        g_nStaticModels = R_LoadLump(h, &header.lumps[LUMP_STATICMODELDEF], &lump, sizeof(cStaticModel_t));
        _R(105);
        R_LoadStaticModelDefs(&lump);
        _R(106);
        R_FreeLump(&lump);
        _R(107);
        g_nStaticModelIndices = R_LoadLump(h, &header.lumps[LUMP_STATICMODELINDEXES], &lump, sizeof(byte));
        _R(108);
        R_LoadStaticModelIndexes(&lump);
        _R(109);
        R_FreeLump(&lump);
        _R(110);
    }
    else
    {
        g_nStaticModelData = 0;
        g_nStaticModels = 0;
        g_nStaticModelIndices = 0;
    }

	// only set tr.world now that we know the entire level has loaded properly
	tr.world = &s_worldData;

    ri.FS_CloseFile(h);

    UI_LoadResource("*111");
    R_Sphere_InitLights();
    UI_LoadResource("*112");
    R_InitTerrain();
    UI_LoadResource("*113");
    R_InitStaticModels();
    UI_LoadResource("*114");
    R_LevelMarksLoad(name);
    UI_LoadResource("*115");
    R_VisDebugLoad(name);
    UI_LoadResource("*116");
}

int RE_MapVersion(void)
{
    // FIXME: unimplemented
    return 0;
}

void RE_PrintBSPFileSizes(void)
{
    // FIXME: unimplemented
}
