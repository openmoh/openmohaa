/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2024 the OpenMoHAA team

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
static	void R_ColorShiftLightingBytes( byte in[3], byte out[3] ) {
    int		r, g, b;

    // shift the data based on overbright range
    r = in[0] << tr.overbrightShift;
    g = in[1] << tr.overbrightShift;
    b = in[2] << tr.overbrightShift;
    
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
}

/*
===============
R_ColorShiftLightingBytesAlpha

===============
*/
static	void R_ColorShiftLightingBytesAlpha( byte in[4], byte out[4] ) {
    int		r, g, b;

    // shift the data based on overbright range
    r = in[0] << tr.overbrightShift;
    g = in[1] << tr.overbrightShift;
    b = in[2] << tr.overbrightShift;
    
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
static	void R_LoadLightmaps(gamelump_t* l) {
    byte		*buf, *buf_p;
    int			len;
    MAC_STATIC byte		image[LIGHTMAP_SIZE*LIGHTMAP_SIZE*4];
    int			i, j;
    float maxIntensity = 0;
    double sumIntensity = 0;

    tr.numLightmaps = 0;

    len = l->length;
    if (!len) {
        return;
    }
    buf = l->buffer;

    // we are about to upload textures
    R_IssuePendingRenderCommands();

    // create all the lightmaps
    tr.numLightmaps = len / (LIGHTMAP_SIZE * LIGHTMAP_SIZE * 3);
    
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
        tr.lightmaps[i] = R_CreateImageOld(va("*lightmap%d", i), image,
            LIGHTMAP_SIZE, LIGHTMAP_SIZE, 0, 1, qfalse, qfalse, qfalse, qfalse, GL_CLAMP, GL_CLAMP);
    }

    if ( r_lightmap->integer == 2 )	{
        ri.Printf( PRINT_ALL, "Brightest lightmap value: %d\n", ( int ) ( maxIntensity * 255 ) );
    }

    if (r_fastdlights->integer) {
        s_worldData.lighting = NULL;
    }
    else {
        s_worldData.lighting = ri.Hunk_Alloc(l->length, h_dontcare);
        Com_Memcpy(s_worldData.lighting, l->buffer, l->length);
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
    s_worldData.novis = ri.Hunk_Alloc( len, h_dontcare );
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

        dest = ri.Hunk_Alloc( len - 8, h_dontcare );
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
                        node->lights = ri.Hunk_Alloc(j * sizeof(spherel_t*), h_dontcare);
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

    if ( shaderNum < 0 || shaderNum >= s_worldData.numShaders ) {
        ri.Error( ERR_DROP, "ShaderForShaderNum: bad num %i", shaderNum );
    }
    dsh = &s_worldData.shaders[ shaderNum ];

    if ( r_vertexLight->integer || glConfig.hardwareType == GLHW_PERMEDIA2 ) {
        lightmapNum = LIGHTMAP_BY_VERTEX;
    }

    if ( r_fullbright->integer || !tr.numLightmaps ) {
        lightmapNum = LIGHTMAP_WHITEIMAGE;
    }

    shader = R_FindShader( dsh->shader, lightmapNum, qtrue, r_picmip->integer, qtrue, qtrue);

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
    int i, j;

    pUnpacked->byDirty = qfalse;
    pUnpacked->visCountCheck = 0;
    pUnpacked->visCountDraw = 0;
    pUnpacked->uiDistRecalc = 0;

    if (pPacked->lmapScale <= 0) {
        Com_Error(ERR_DROP, "invalid map: terrain has lmapScale <= 0");
    }

    pUnpacked->drawinfo.lmapStep = (float)(64 / pPacked->lmapScale);
    pUnpacked->drawinfo.lmapSize = pPacked->lmapScale * 8 + 1;
    pUnpacked->s = ((float)pPacked->s + 0.5) / LIGHTMAP_SIZE;
    pUnpacked->t = ((float)pPacked->t + 0.5) / LIGHTMAP_SIZE;

    if( s_worldData.lighting ) {
        pUnpacked->drawinfo.lmData = &s_worldData.lighting[pPacked->iLightMap * (LIGHTMAP_SIZE * LIGHTMAP_SIZE * 3) + 3 * pPacked->s + 3 * LIGHTMAP_SIZE * pPacked->t];
    } else {
        pUnpacked->drawinfo.lmData = NULL;
    }

    for (i = 0; i < 2; i++) {
        for (j = 0; j < 2; j++) {
            pUnpacked->texCoord[i][j][0] = pPacked->texCoord[i][j][0];
            pUnpacked->texCoord[i][j][1] = pPacked->texCoord[i][j][1];
        }
    }

    pUnpacked->x0 = ((int)pPacked->x << 6);
    pUnpacked->y0 = ((int)pPacked->y << 6);
    pUnpacked->z0 = pPacked->iBaseHeight;
    pUnpacked->shader = ShaderForShaderNum(pPacked->iShader, pPacked->iLightMap);
    pUnpacked->iNorth = pPacked->iNorth;
    pUnpacked->iEast = pPacked->iEast;
    pUnpacked->iSouth = pPacked->iSouth;
    pUnpacked->iWest = pPacked->iWest;

    for (i = 0; i < MAX_TERRAIN_VARNODES; i++)
    {
        varnode_t *packedVarTree;

        packedVarTree = &pPacked->varTree[0][i];
        pUnpacked->varTree[0][i].fVariance = packedVarTree->flags & 0x7FF;
        pUnpacked->varTree[0][i].flags &= ~0xFF;
        pUnpacked->varTree[0][i].flags |= (packedVarTree->flags >> 12) & 0xFF;

        packedVarTree = &pPacked->varTree[1][i];
        pUnpacked->varTree[1][i].fVariance = packedVarTree->flags & 0x7FF;
        pUnpacked->varTree[1][i].flags &= ~0xFF;
        pUnpacked->varTree[1][i].flags |= (packedVarTree->flags >> 12) & 0xFF;
    }

    for (i = 0; i < ARRAY_LEN(pUnpacked->heightmap); i++) {
        pUnpacked->heightmap[i] = pPacked->heightmap[i];
    }

    pUnpacked->zmax = 0;
    pUnpacked->flags = pPacked->flags;

    for (i = 0; i < ARRAY_LEN(pUnpacked->heightmap); i++)
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
    s_worldData.terraPatches = ri.Hunk_Alloc(s_worldData.numTerraPatches * sizeof(cTerraPatchUnpacked_t), h_dontcare);

    in = lump->buffer;
    out = s_worldData.terraPatches;

    for (i = 0; i < s_worldData.numTerraPatches; in++, out++, i++) {
        R_SwapTerraPatch(in);
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
    s_worldData.visTerraPatches = ri.Hunk_Alloc(s_worldData.numVisTerraPatches * sizeof(cTerraPatchUnpacked_t*), h_dontcare);

    in = lump->buffer;
    out = s_worldData.visTerraPatches;

    for (i = 0; i < s_worldData.numVisTerraPatches; in++, out++, i++) {
        *out = &s_worldData.terraPatches[LittleShort(*in)];
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
    static surfaceType_t skipData = SF_SKIP;

    lightmapNum = LittleLong( ds->lightmapNum );

    // get shader value
    surf->shader = ShaderForShaderNum(LittleLong(ds->shaderNum), lightmapNum);
    if ( r_singleShader->integer && !surf->shader->isSky ) {
        surf->shader = tr.defaultShader;
    }

    if (surf->shader->surfaceFlags & SURF_NODRAW) {
        // Nodraw surface doesn't need any processing
        surf->data = &skipData;
        return;
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

    cv = ri.Hunk_Alloc( sfaceSize, h_dontcare );
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
        R_ColorShiftLightingBytesAlpha( verts[i].color, (byte *)&cv->points[i][7] );
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

    if (tr.numLightmaps && lightmapNum != -1 && surf->shader->lightmapIndex >= 0)
    {
        float inv;

        cv->lmX = LittleLong(ds->lightmapX);
        cv->lmY = LittleLong(ds->lightmapY);
        cv->lmWidth = LittleLong(ds->lightmapWidth);
        cv->lmHeight = LittleLong(ds->lightmapHeight);

        if (s_worldData.lighting) {
            cv->lmData = &s_worldData.lighting[lightmapNum * (LIGHTMAP_SIZE * LIGHTMAP_SIZE * 3) + 3 * cv->lmX + 3 * LIGHTMAP_SIZE * cv->lmY];
        } else {
            cv->lmData = NULL;
        }

        for (i = 0; i < 3; i++)
        {
            cv->lmOrigin[i] = LittleFloat(ds->lightmapOrigin[i]);
            cv->lmVecs[0][i] = LittleFloat(ds->lightmapVecs[0][i]);
            cv->lmVecs[1][i] = LittleFloat(ds->lightmapVecs[1][i]);
        }

        inv = VectorNormalize2(cv->lmVecs[0], cv->lmInverseVecs[0]);
        cv->lmInverseVecs[0][0] *= 1.0 / inv;
        cv->lmInverseVecs[0][1] *= 1.0 / inv;
        cv->lmInverseVecs[0][2] *= 1.0 / inv;

        inv = VectorNormalize2(cv->lmVecs[1], cv->lmInverseVecs[1]);
        cv->lmInverseVecs[1][0] *= 1.0 / inv;
        cv->lmInverseVecs[1][1] *= 1.0 / inv;
        cv->lmInverseVecs[1][2] *= 1.0 / inv;
    }
    else
    {
        cv->lmData = NULL;
    }
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
    float           subdivisions;
    static surfaceType_t	skipData = SF_SKIP;

    lightmapNum = LittleLong( ds->lightmapNum );

    // get fog volume
    surf->fogIndex = LittleLong( ds->fogNum ) + 1;

    // get shader value
    surf->shader = ShaderForShaderNum(LittleLong(ds->shaderNum), lightmapNum);
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
        R_ColorShiftLightingBytesAlpha( verts[i].color, points[i].color );
    }

    // pre-tesseleate
    subdivisions = LittleFloat(ds->subdivisions);
    if (subdivisions) {
        grid = R_SubdividePatchToGrid(width, height, subdivisions * (r_subdivisions->value / 10.0), points);
    } else if (surf->shader->subdivisions) {
        grid = R_SubdividePatchToGrid(width, height, surf->shader->subdivisions * (r_subdivisions->value / 10.0), points);
    } else {
        grid = R_SubdividePatchToGrid(width, height, r_subdivisions->value, points);
    }
    surf->data = (surfaceType_t *)grid;

    if (tr.numLightmaps && lightmapNum != -1 && surf->shader->lightmapIndex >= 0)
    {
        grid->lmX = LittleLong(ds->lightmapX);
        grid->lmY = LittleLong(ds->lightmapY);
        grid->lmWidth = LittleLong(ds->lightmapWidth);
        grid->lmHeight = LittleLong(ds->lightmapHeight);
        if (s_worldData.lighting) {
            grid->lmData = &s_worldData.lighting[lightmapNum * (LIGHTMAP_SIZE * LIGHTMAP_SIZE * 3) + 3 * grid->lmX + 3 * LIGHTMAP_SIZE * grid->lmY];
        } else {
            grid->lmData = NULL;
        }
    }

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
    surf->shader = ShaderForShaderNum(LittleLong(ds->shaderNum), LIGHTMAP_BY_VERTEX);
    if ( r_singleShader->integer && !surf->shader->isSky ) {
        surf->shader = tr.defaultShader;
    }

    numVerts = LittleLong( ds->numVerts );
    numIndexes = LittleLong( ds->numIndexes );

    tri = ri.Hunk_Alloc( sizeof( *tri ) + numVerts * sizeof( tri->verts[0] ) 
        + numIndexes * sizeof( tri->indexes[0] ), h_dontcare );
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

        R_ColorShiftLightingBytesAlpha( verts[i].color, tri->verts[i].color );
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
    surf->shader = ShaderForShaderNum(LittleLong(ds->shaderNum), LIGHTMAP_BY_VERTEX);
    if ( r_singleShader->integer && !surf->shader->isSky ) {
        surf->shader = tr.defaultShader;
    }

    flare = ri.Hunk_Alloc( sizeof( *flare ), h_dontcare );
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

        // the grid1 could be reallocated again
        grid1 = (srfGridMesh_t *) s_worldData.surfaces[grid1num].data;
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
        hunkgrid = ri.Hunk_Alloc( size, h_dontcare );
        Com_Memcpy(hunkgrid, grid, size);

        hunkgrid->widthLodError = ri.Hunk_Alloc( grid->width * 4, h_dontcare );
        Com_Memcpy( hunkgrid->widthLodError, grid->widthLodError, grid->width * 4 );

        hunkgrid->heightLodError = ri.Hunk_Alloc( grid->height * 4, h_dontcare );
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
    out = ri.Hunk_Alloc ( count * sizeof(*out), h_dontcare );	

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

    s_worldData.bmodels = out = ri.Hunk_Alloc( count * sizeof(*out), h_dontcare );
    s_worldData.numBmodels = count;

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

    out = ri.Hunk_Alloc((numNodes + numLeafs) * sizeof(*out), h_dontcare);

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

    out = ri.Hunk_Alloc((numNodes + numLeafs) * sizeof(*out), h_dontcare);

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
    out = ri.Hunk_Alloc(count * sizeof(*out), h_dontcare);

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
    out = (msurface_t**)ri.Hunk_Alloc(count * sizeof(*out), h_dontcare);

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
    out = ri.Hunk_Alloc(count * 2 * sizeof(*out), h_dontcare);

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
================
R_LoadLightGrid

================
*/
void R_LoadLightGrid(gamelump_t* plPal, gamelump_t* plOffsets, gamelump_t* plData) {
    int		i;
    vec3_t	maxs;
    int		numGridPoints;
    world_t	*w;
    float	*wMins, *wMaxs;

    w = &s_worldData;

    if (!plPal->buffer || !plOffsets->buffer || !plData->buffer) {
        ri.Printf(PRINT_WARNING, "WARNING: No light grid data present\n");
        w->lightGridOffsets = 0;
        w->lightGridData = 0;
        return;
    }

    switch (map_version)
    {
    case 21:
        w->lightGridSize[0] = 80.0;
        w->lightGridSize[1] = 80.0;
        w->lightGridSize[2] = 80.0;
        break;
    case 20:
        w->lightGridSize[0] = 48.0;
        w->lightGridSize[1] = 48.0;
        w->lightGridSize[2] = 64.0;
        break;
    default:
        w->lightGridSize[0] = 32.0;
        w->lightGridSize[1] = 32.0;
        w->lightGridSize[2] = 32.0;
        break;
    }

    w->lightGridOOSize[0] = 1.0f / w->lightGridSize[0];
    w->lightGridOOSize[1] = 1.0f / w->lightGridSize[1];
    w->lightGridOOSize[2] = 1.0f / w->lightGridSize[2];

    wMins = w->bmodels[0].bounds[0];
    wMaxs = w->bmodels[0].bounds[1];

    for ( i = 0 ; i < 3 ; i++ ) {
        w->lightGridMins[i] = w->lightGridSize[i] * ceil( wMins[i] / w->lightGridSize[i] );
        maxs[i] = w->lightGridSize[i] * floor( wMaxs[i] / w->lightGridSize[i] );
        w->lightGridBounds[i] = (maxs[i] - w->lightGridMins[i])/w->lightGridSize[i] + 1;
    }

    numGridPoints = w->lightGridBounds[0] * w->lightGridBounds[1] + w->lightGridBounds[0];

    if (plOffsets->length != numGridPoints * 2) {
        ri.Printf(PRINT_WARNING, "WARNING: light grid offset size mismatch\n");
        w->lightGridOffsets = NULL;
        w->lightGridData = NULL;
        return;
    }

    if (plPal->length != 768) {
        ri.Printf(PRINT_WARNING, "WARNING: light grid palette size mismatch\n");
        w->lightGridOffsets = NULL;
        w->lightGridData = NULL;
        return;
    }

    w->lightGridOffsets = ri.Hunk_Alloc(plOffsets->length, h_dontcare);
    for (i = 0; i < plOffsets->length / 2; i++) {
        w->lightGridOffsets[i] = LittleUnsignedShort(((short*)plOffsets->buffer)[i]);
    }
    Com_Memcpy(w->lightGridPalette, plPal->buffer, sizeof(s_worldData.lightGridPalette));

    w->lightGridData = ri.Hunk_Alloc(plData->length, h_dontcare);
    Com_Memcpy(w->lightGridData, plData->buffer, plData->length);
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
    s_worldData.staticModelData = (byte*)ri.Hunk_Alloc(s_worldData.numStaticModelData * sizeof(color4ub_t), h_dontcare);

    pSrcColors = lump->buffer;
    pDstColors = s_worldData.staticModelData;

    for (i = 0; i < lump->length; i += sizeof(byte) * 3)
    { 
        // Colors are stored as integers
        pDstColors[0] = pSrcColors[0];
        pDstColors[1] = pSrcColors[1];
        pDstColors[2] = pSrcColors[2];
        pDstColors[3] = 0xFF;

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
    pUnpackedSM->angles[0] = LittleFloat(pSM->angles[0]);
    pUnpackedSM->angles[1] = LittleFloat(pSM->angles[1]);
    pUnpackedSM->angles[2] = LittleFloat(pSM->angles[2]);
    pUnpackedSM->origin[0] = LittleFloat(pSM->origin[0]);
    pUnpackedSM->origin[1] = LittleFloat(pSM->origin[1]);
    pUnpackedSM->origin[2] = LittleFloat(pSM->origin[2]);
    pUnpackedSM->scale = LittleFloat(pSM->scale);
    pUnpackedSM->firstVertexData = LittleLong(pSM->firstVertexData) * sizeof(color4ub_t) / (sizeof(byte) * 3);
    pUnpackedSM->numVertexData = LittleLong(pSM->numVertexData);
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
    s_worldData.staticModels = ri.Hunk_Alloc(s_worldData.numStaticModels * sizeof(cStaticModelUnpacked_t), h_dontcare);

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
    int i;
    unsigned short* in;
    cStaticModelUnpacked_t** out;

    if (lump->length % sizeof(short)) {
        Com_Error(ERR_DROP, "R_LoadStaticModelDefs: funny lump size");
    }

    s_worldData.numVisStaticModels = lump->length / sizeof(short);
    s_worldData.visStaticModels = ri.Hunk_Alloc(s_worldData.numVisStaticModels * sizeof(cStaticModelUnpacked_t*), h_dontcare);

    in = lump->buffer;
    out = s_worldData.visStaticModels;

    for (i = 0; i < s_worldData.numVisStaticModels; in++, out++, i++) {
        unsigned short index = LittleUnsignedShort(*in);
        if (index >= s_worldData.numStaticModels) {
            // Added in OPM
            Com_Error(ERR_DROP, "R_LoadStaticModelIndexes: bad static model index %d", index);
        }

        *out = &s_worldData.staticModels[index];
    }
}

/*
=================
R_GetEntityToken
=================
*/
qboolean R_GetEntityToken( char *buffer, int size ) {
    // Stub
    return qfalse;
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
        glump->buffer = ri.Hunk_AllocateTempMemory(lump->filelen);

        if (ri.FS_Seek(handle, lump->fileofs, FS_SEEK_SET) < 0) {
            Com_Error(ERR_DROP, "R_LoadLump: Error seeking to lump.");
        }

        ri.FS_Read(glump->buffer, lump->filelen, handle);

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
        ri.Hunk_FreeTempMemory(lump->buffer);
        lump->buffer = NULL;
        lump->length = 0;
    }
}

#define _R( id ) ri.UI_LoadResource( "*" #id )

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

    for (i=0 ; i<sizeof(dheader_t)/4 ; i++) {
        ((int *)&header)[i] = LittleLong ( ((int *)&header)[i]);
    }

    map_length = length;
    map_version = header.version;

    // clear tr.world so if the level fails to load, the next
    // try will not look at the partially loaded version
    tr.world = NULL;

    Com_Memset( &s_worldData, 0, sizeof( s_worldData ) );
    Q_strncpyz( s_worldData.name, name, sizeof( s_worldData.name ) );

    Q_strncpyz( s_worldData.baseName, COM_SkipPath( s_worldData.name ), sizeof( s_worldData.name ) );
    COM_StripExtension( s_worldData.baseName, s_worldData.baseName, sizeof(s_worldData.baseName));

    c_gridVerts = 0;

    i = header.version;
    if (i < BSP_MIN_VERSION || i > BSP_MAX_VERSION) {
        ri.Error(ERR_DROP, "RE_LoadWorldMap: %s has wrong version number (%i should be between %i and %i)",
            name, i, BSP_MIN_VERSION, BSP_MAX_VERSION);
    }

    // load into heap
    // load into heap
    _R(53);
    numShaders = R_LoadLump(h, Q_GetLumpByVersion(&header, LUMP_SHADERS), &lump, sizeof(dshader_t));
    _R(54);
    R_LoadShaders(&lump);
    _R(55);
    R_FreeLump(&lump);
    _R(56);
    numplanes = R_LoadLump(h, Q_GetLumpByVersion(&header, LUMP_PLANES), &lump, sizeof(dplane_t));
    R_LoadPlanes(&lump);
    _R(57);
    R_FreeLump(&lump);
    _R(58);
    numLightBytes = R_LoadLump(h, Q_GetLumpByVersion(&header, LUMP_LIGHTMAPS), &lump, sizeof(byte));
    _R(59);
    R_LoadLightmaps(&lump);
    _R(60);
    R_FreeLump(&lump);
    _R(61);
    numDrawSurfaces = R_LoadLump(h, Q_GetLumpByVersion(&header, LUMP_SURFACES), &lump, sizeof(dsurface_t));
    _R(62);
    numDrawVerts = R_LoadLump(h, Q_GetLumpByVersion(&header, LUMP_DRAWVERTS), &lump2, sizeof(drawVert_t));
    _R(63);
    numDrawIndexes = R_LoadLump(h, Q_GetLumpByVersion(&header, LUMP_DRAWINDEXES), &lump3, sizeof(int));
    _R(64);
    R_LoadSurfaces(&lump, &lump2, &lump3);
    _R(65);
    R_FreeLump(&lump3);
    _R(66);
    R_FreeLump(&lump2);
    _R(67);
    R_FreeLump(&lump);
    _R(68);
    numleafsurfaces = R_LoadLump(h, Q_GetLumpByVersion(&header, LUMP_LEAFSURFACES), &lump, sizeof(int));
    _R(69);
    R_LoadMarksurfaces(&lump);
    _R(70);
    R_FreeLump(&lump);
    _R(71);
    numleafs = R_LoadLump(h, Q_GetLumpByVersion(&header, LUMP_LEAFS), &lump2, sizeof(dleaf_t));
    _R(72);
    numnodes = R_LoadLump(h, Q_GetLumpByVersion(&header, LUMP_NODES), &lump, sizeof(mnode_t));
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
    numFogs = 0;
    nummodels = R_LoadLump(h, Q_GetLumpByVersion(&header, LUMP_MODELS), &lump, sizeof(bmodel_t));
    _R(77);
    R_LoadSubmodels(&lump);
    _R(78);
    R_FreeLump(&lump);
    _R(79);
    numVisBytes = R_LoadLump(h, Q_GetLumpByVersion(&header, LUMP_VISIBILITY), &lump, sizeof(byte));
    _R(80);
    R_LoadVisibility(&lump);
    _R(81);
    R_FreeLump(&lump);
    _R(82);
    g_iGridPaletteBytes = R_LoadLump(h, Q_GetLumpByVersion(&header, LUMP_LIGHTGRIDPALETTE), &lump, sizeof(byte));
    _R(83);
    g_iGridOffsets = R_LoadLump(h, Q_GetLumpByVersion(&header, LUMP_LIGHTGRIDOFFSETS), &lump2, sizeof(short));
    _R(84);
    g_iGridDataSize = R_LoadLump(h, Q_GetLumpByVersion(&header, LUMP_LIGHTGRIDDATA), &lump3, sizeof(byte));
    _R(85);
    R_LoadLightGrid( &lump, &lump2, &lump3 );
    _R(86);
    R_FreeLump(&lump);
    _R(87);
    R_FreeLump(&lump2);
    _R(88);
    R_FreeLump(&lump3);
    _R(89);
    numSLights = R_LoadLump(h, Q_GetLumpByVersion(&header, LUMP_SPHERELIGHTS), &lump, sizeof(mapspherel_t));
    _R(90);
    R_LoadSphereLights(&lump);
    _R(91);
    R_FreeLump(&lump);
    _R(92);
    entLightVisSize = R_LoadLump(h, Q_GetLumpByVersion(&header, LUMP_SPHERELIGHTVIS), &lump, sizeof(byte));
    _R(93);
    R_LoadSphereLightVis(&lump);
    _R(94);
    R_FreeLump(&lump);
    _R(95);
    g_nTerraPatches = R_LoadLump(h, Q_GetLumpByVersion(&header, LUMP_TERRAIN), &lump, sizeof(cTerraPatch_t));
    _R(96);
    R_LoadTerrain(&lump);
    _R(97);
    R_FreeLump(&lump);
    _R(98);
    g_nTerPatchIndices = R_LoadLump(h, Q_GetLumpByVersion(&header, LUMP_TERRAININDEXES), &lump, sizeof(short));
    _R(99);
    R_LoadTerrainIndexes(&lump);
    _R(100);
    R_FreeLump(&lump);
    _R(101);
    if (header.version > BSP_BETA_VERSION)
    {
        g_nStaticModelData = R_LoadLump(h, Q_GetLumpByVersion(&header, LUMP_STATICMODELDATA), &lump, sizeof(byte));
        _R(102);
        R_LoadStaticModelData(&lump);
        _R(103);
        R_FreeLump(&lump);
        _R(104);
        g_nStaticModels = R_LoadLump(h, Q_GetLumpByVersion(&header, LUMP_STATICMODELDEF), &lump, sizeof(cStaticModel_t));
        _R(105);
        R_LoadStaticModelDefs(&lump);
        _R(106);
        R_FreeLump(&lump);
        _R(107);
        g_nStaticModelIndices = R_LoadLump(h, Q_GetLumpByVersion(&header, LUMP_STATICMODELINDEXES), &lump, sizeof(byte));
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

    ri.UI_LoadResource("*111");
    R_Sphere_InitLights();
    ri.UI_LoadResource("*112");
    R_InitTerrain();
    ri.UI_LoadResource("*113");
    R_InitStaticModels();
    ri.UI_LoadResource("*114");
    R_LevelMarksLoad(name);
    ri.UI_LoadResource("*115");
    R_VisDebugLoad(name);
    ri.UI_LoadResource("*116");
}

/*
=================
RE_MapVersion

=================
*/
int RE_MapVersion(void)
{
	return map_version;
}

/*
=================
RE_PrintBSPFileSizes

=================
*/
void RE_PrintBSPFileSizes(void)
{
  ri.Printf(PRINT_ALL, "%s: %i\n", s_worldData.name, map_length);
  ri.Printf(PRINT_ALL, "%6i   models                %7i\n", nummodels, 40 * nummodels);
  ri.Printf(PRINT_ALL, "%6i   shaders               %7i\n", numShaders, 140 * numShaders);
  ri.Printf(PRINT_ALL, "%6i   brushes               %7i\n", numbrushes, 12 * numbrushes);
  ri.Printf(PRINT_ALL, "%6i   brushsides            %7i\n", numbrushsides, 12 * numbrushsides);
  ri.Printf(PRINT_ALL, "%6i   fogs                  %7i\n", numFogs, 72 * numFogs);
  ri.Printf(PRINT_ALL, "%6i   planes                %7i\n", numplanes, 16 * numplanes);
  ri.Printf(PRINT_ALL, "%6i   nodes                 %7i\n", numnodes, 36 * numnodes);
  ri.Printf(PRINT_ALL, "%6i   leafs                 %7i\n", numleafs, numleafs << 6);
  ri.Printf(PRINT_ALL, "%6i   leafsurfaces          %7i\n", numleafsurfaces, 4 * numleafsurfaces);
  ri.Printf(PRINT_ALL, "%6i   drawverts             %7i\n", numDrawVerts, 44 * numDrawVerts);
  ri.Printf(PRINT_ALL, "%6i   drawindexes           %7i\n", numDrawIndexes, 4 * numDrawIndexes);
  ri.Printf(PRINT_ALL, "%6i   drawsurfaces          %7i\n", numDrawSurfaces, 108 * numDrawSurfaces);
  ri.Printf(PRINT_ALL, "%6i   lightdefs             %7i\n", numLightDefs, 52 * numLightDefs);
  ri.Printf(PRINT_ALL, "%6i   lightmaps             %7i\n", numLightBytes / 49152, numLightBytes);
  ri.Printf(PRINT_ALL, "         visibility            %7i\n", numVisBytes);
  ri.Printf(PRINT_ALL, "%6i   entitylights          %7i\n", numSLights, 56 * numSLights);
  ri.Printf(PRINT_ALL, "         entitylightvis        %7i\n", entLightVisSize);
  ri.Printf(PRINT_ALL, "         light grid palette    %7i\n", g_iGridPaletteBytes);
  ri.Printf(PRINT_ALL, "%6i   light grid offsets    %7i\n", g_iGridOffsets, 2 * g_iGridOffsets);
  ri.Printf(PRINT_ALL, "         light grid data       %7i\n", g_iGridDataSize);
  ri.Printf(PRINT_ALL, "%6i   terrain               %7i\n", g_nTerraPatches, 388 * g_nTerraPatches);
  ri.Printf(PRINT_ALL, "%6i   terrain indexes       %7i\n", g_nTerPatchIndices, 2 * g_nTerPatchIndices);
  ri.Printf(PRINT_ALL, "         static model data     %7i\n", g_nStaticModelData);
  ri.Printf(PRINT_ALL, "%6i   static models defs    %7i\n", g_nStaticModels, 164 * g_nStaticModels);
  ri.Printf(PRINT_ALL, "         static model indexes  %7i\n", g_nStaticModelIndices);
}

/*
=================
R_ClearWorld

Set the world to NULL to prevent anyone from accessing
freed world data
=================
*/
void R_ClearWorld(void) {
    tr.world = NULL;
}
