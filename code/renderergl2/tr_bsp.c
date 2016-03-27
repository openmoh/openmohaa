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
// tr_map.c

#include "../client/client.h"
#include "tr_local.h"

/*

Loads and prepares a map file for scene rendering.

A single entry point:

void RE_LoadWorldMap( const char *name );

*/

static	world_t		s_worldData;
static	byte		*fileBase;

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

int			c_subdivisions;
int			c_gridVerts;

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
R_ColorShiftLightingFloats

===============
*/
static void R_ColorShiftLightingFloats(float in[4], float out[4], float scale )
{
	float	r, g, b;

	scale *= pow(2.0f, r_mapOverBrightBits->integer - tr.overbrightBits);

	r = in[0] * scale;
	g = in[1] * scale;
	b = in[2] * scale;

	// normalize by color instead of saturating to white
	if ( r > 1 || g > 1 || b > 1 ) {
		float	max;

		max = r > g ? r : g;
		max = max > b ? max : b;
		r = r / max;
		g = g / max;
		b = b / max;
	}

	out[0] = r;
	out[1] = g;
	out[2] = b;
	out[3] = in[3];
}

// Modified from http://graphicrants.blogspot.jp/2009/04/rgbm-color-encoding.html
void ColorToRGBM(const vec3_t color, unsigned char rgbm[4])
{
	vec3_t          sample;
	float			maxComponent;

	VectorCopy(color, sample);

	maxComponent = MAX(sample[0], sample[1]);
	maxComponent = MAX(maxComponent, sample[2]);
	maxComponent = CLAMP(maxComponent, 1.0f/255.0f, 1.0f);

	rgbm[3] = (unsigned char) ceil(maxComponent * 255.0f);
	maxComponent = 255.0f / rgbm[3];

	VectorScale(sample, maxComponent, sample);

	rgbm[0] = (unsigned char) (sample[0] * 255);
	rgbm[1] = (unsigned char) (sample[1] * 255);
	rgbm[2] = (unsigned char) (sample[2] * 255);
}

void ColorToRGBA16F(const vec3_t color, unsigned short rgba16f[4])
{
	rgba16f[0] = FloatToHalf(color[0]);
	rgba16f[1] = FloatToHalf(color[1]);
	rgba16f[2] = FloatToHalf(color[2]);
	rgba16f[3] = FloatToHalf(1.0f);
}


/*
===============
R_LoadLightmaps

===============
*/
#define	DEFAULT_LIGHTMAP_SIZE	128
#define MAX_LIGHTMAP_PAGES 2
static	void R_LoadLightmaps( gamelump_t *l, gamelump_t *surfs ) {
	byte		*buf, *buf_p;
	dsurface_t  *surf;
	int			len;
	byte		*image;
	int			i, j, numLightmaps, textureInternalFormat = 0;
	float maxIntensity = 0;
	double sumIntensity = 0;

	len = l->length;
	if ( !len ) {
		return;
	}
	buf = l->buffer;

	// we are about to upload textures
	R_IssuePendingRenderCommands();

	tr.lightmapSize = DEFAULT_LIGHTMAP_SIZE;
	numLightmaps = len / (tr.lightmapSize * tr.lightmapSize * 3);

	// check for deluxe mapping
	if (numLightmaps <= 1)
	{
		tr.worldDeluxeMapping = qfalse;
	}
	else
	{
		tr.worldDeluxeMapping = qtrue;
		for( i = 0, surf = (dsurface_t *)l->buffer;
			i < surfs->length / sizeof(dsurface_t); i++, surf++ ) {
			int lightmapNum = LittleLong( surf->lightmapNum );

			if ( lightmapNum >= 0 && (lightmapNum & 1) != 0 ) {
				tr.worldDeluxeMapping = qfalse;
				break;
			}
		}
	}

	image = ri.Malloc(tr.lightmapSize * tr.lightmapSize * 4 * 2);

	if (tr.worldDeluxeMapping)
		numLightmaps >>= 1;

	if(numLightmaps == 1)
	{
		//FIXME: HACK: maps with only one lightmap turn up fullbright for some reason.
		//this avoids this, but isn't the correct solution.
		numLightmaps++;
	}
	else if (r_mergeLightmaps->integer && numLightmaps >= 1024 )
	{
		// FIXME: fat light maps don't support more than 1024 light maps
		ri.Printf(PRINT_WARNING, "WARNING: number of lightmaps > 1024\n");
		numLightmaps = 1024;
	}

	// use fat lightmaps of an appropriate size
	if (r_mergeLightmaps->integer)
	{
		tr.fatLightmapSize = 512;
		tr.fatLightmapStep = tr.fatLightmapSize / tr.lightmapSize;

		// at most MAX_LIGHTMAP_PAGES
		while (tr.fatLightmapStep * tr.fatLightmapStep * MAX_LIGHTMAP_PAGES < numLightmaps && tr.fatLightmapSize != glConfig.maxTextureSize )
		{
			tr.fatLightmapSize <<= 1;
			tr.fatLightmapStep = tr.fatLightmapSize / tr.lightmapSize;
		}

		tr.numLightmaps = numLightmaps / (tr.fatLightmapStep * tr.fatLightmapStep);

		if (numLightmaps % (tr.fatLightmapStep * tr.fatLightmapStep) != 0)
			tr.numLightmaps++;
	}
	else
	{
		tr.numLightmaps = numLightmaps;
	}

	tr.lightmaps = ri.Hunk_Alloc( tr.numLightmaps * sizeof(image_t *) );

	if (tr.worldDeluxeMapping)
	{
		tr.deluxemaps = ri.Hunk_Alloc( tr.numLightmaps * sizeof(image_t *) );
	}

	if (glRefConfig.floatLightmap)
		textureInternalFormat = GL_RGBA16F_ARB;
	else
		textureInternalFormat = GL_RGBA8;

	if (r_mergeLightmaps->integer)
	{
		for (i = 0; i < tr.numLightmaps; i++)
		{
			tr.lightmaps[i] = R_CreateImage(va("_fatlightmap%d", i), NULL, tr.fatLightmapSize, tr.fatLightmapSize, IMGTYPE_COLORALPHA, IMGFLAG_NOLIGHTSCALE | IMGFLAG_NO_COMPRESSION | IMGFLAG_CLAMPTOEDGE, textureInternalFormat );

			if (tr.worldDeluxeMapping)
			{
				tr.deluxemaps[i] = R_CreateImage(va("_fatdeluxemap%d", i), NULL, tr.fatLightmapSize, tr.fatLightmapSize, IMGTYPE_DELUXE, IMGFLAG_NOLIGHTSCALE | IMGFLAG_NO_COMPRESSION | IMGFLAG_CLAMPTOEDGE, 0 );
			}
		}
	}

	for(i = 0; i < numLightmaps; i++)
	{
		int xoff = 0, yoff = 0;
		int lightmapnum = i;
		// expand the 24 bit on-disk to 32 bit

		if (r_mergeLightmaps->integer)
		{
			int lightmaponpage = i % (tr.fatLightmapStep * tr.fatLightmapStep);
			xoff = (lightmaponpage % tr.fatLightmapStep) * tr.lightmapSize;
			yoff = (lightmaponpage / tr.fatLightmapStep) * tr.lightmapSize;

			lightmapnum /= (tr.fatLightmapStep * tr.fatLightmapStep);
		}

		// if (tr.worldLightmapping)
		{
			char filename[MAX_QPATH];
			byte *hdrLightmap = NULL;
			int size = 0;

			// look for hdr lightmaps
			if (r_hdr->integer)
			{
				Com_sprintf( filename, sizeof( filename ), "maps/%s/lm_%04d.hdr", s_worldData.baseName, i * (tr.worldDeluxeMapping ? 2 : 1) );
				//ri.Printf(PRINT_ALL, "looking for %s\n", filename);

				size = ri.FS_ReadFile(filename, (void **)&hdrLightmap);
			}

			if (hdrLightmap)
			{
				byte *p = hdrLightmap;
				//ri.Printf(PRINT_ALL, "found!\n");
				
				/* FIXME: don't just skip over this header and actually parse it */
				while (size && !(*p == '\n' && *(p+1) == '\n'))
				{
					size--;
					p++;
				}

				if (!size)
					ri.Error(ERR_DROP, "Bad header for %s!", filename);

				size -= 2;
				p += 2;
				
				while (size && !(*p == '\n'))
				{
					size--;
					p++;
				}

				size--;
				p++;

				buf_p = (byte *)p;

#if 0 // HDRFILE_RGBE
				if (size != tr.lightmapSize * tr.lightmapSize * 4)
					ri.Error(ERR_DROP, "Bad size for %s (%i)!", filename, size);
#else // HDRFILE_FLOAT
				if (size != tr.lightmapSize * tr.lightmapSize * 12)
					ri.Error(ERR_DROP, "Bad size for %s (%i)!", filename, size);
#endif
			}
			else
			{
				if (tr.worldDeluxeMapping)
					buf_p = buf + (i * 2) * tr.lightmapSize * tr.lightmapSize * 3;
				else
					buf_p = buf + i * tr.lightmapSize * tr.lightmapSize * 3;
			}

			for ( j = 0 ; j < tr.lightmapSize * tr.lightmapSize; j++ ) 
			{
				if (hdrLightmap)
				{
					vec4_t color;

#if 0 // HDRFILE_RGBE
					float exponent = exp2(buf_p[j*4+3] - 128);

					color[0] = buf_p[j*4+0] * exponent;
					color[1] = buf_p[j*4+1] * exponent;
					color[2] = buf_p[j*4+2] * exponent;
#else // HDRFILE_FLOAT
					memcpy(color, &buf_p[j*12], 12);

					color[0] = LittleFloat(color[0]);
					color[1] = LittleFloat(color[1]);
					color[2] = LittleFloat(color[2]);
#endif
					color[3] = 1.0f;

					R_ColorShiftLightingFloats(color, color, 1.0f/255.0f);

					if (glRefConfig.floatLightmap)
						ColorToRGBA16F(color, (unsigned short *)(&image[j*8]));
					else
						ColorToRGBM(color, &image[j*4]);
				}
				else if (glRefConfig.floatLightmap)
				{
					vec4_t color;

					//hack: convert LDR lightmap to HDR one
					color[0] = MAX(buf_p[j*3+0], 0.499f);
					color[1] = MAX(buf_p[j*3+1], 0.499f);
					color[2] = MAX(buf_p[j*3+2], 0.499f);

					// if under an arbitrary value (say 12) grey it out
					// this prevents weird splotches in dimly lit areas
					if (color[0] + color[1] + color[2] < 12.0f)
					{
						float avg = (color[0] + color[1] + color[2]) * 0.3333f;
						color[0] = avg;
						color[1] = avg;
						color[2] = avg;
					}
					color[3] = 1.0f;

					R_ColorShiftLightingFloats(color, color, 1.0f/255.0f);

					ColorToRGBA16F(color, (unsigned short *)(&image[j*8]));
				}
				else
				{
					if ( r_lightmap->integer == 2 )
					{	// color code by intensity as development tool	(FIXME: check range)
						float r = buf_p[j*3+0];
						float g = buf_p[j*3+1];
						float b = buf_p[j*3+2];
						float intensity;
						float out[3] = {0.0, 0.0, 0.0};

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
					else
					{
						R_ColorShiftLightingBytes( &buf_p[j*3], &image[j*4] );
						image[j*4+3] = 255;
					}
				}
			}

			if (r_mergeLightmaps->integer)
				R_UpdateSubImage(tr.lightmaps[lightmapnum], image, xoff, yoff, tr.lightmapSize, tr.lightmapSize);
			else
				tr.lightmaps[i] = R_CreateImage(va("*lightmap%d", i), image, tr.lightmapSize, tr.lightmapSize, IMGTYPE_COLORALPHA, IMGFLAG_NOLIGHTSCALE | IMGFLAG_NO_COMPRESSION | IMGFLAG_CLAMPTOEDGE, textureInternalFormat );

			if (hdrLightmap)
				ri.FS_FreeFile(hdrLightmap);
		}

		if (tr.worldDeluxeMapping)
		{
			buf_p = buf + (i * 2 + 1) * tr.lightmapSize * tr.lightmapSize * 3;

			for ( j = 0 ; j < tr.lightmapSize * tr.lightmapSize; j++ ) {
				image[j*4+0] = buf_p[j*3+0];
				image[j*4+1] = buf_p[j*3+1];
				image[j*4+2] = buf_p[j*3+2];

				// make 0,0,0 into 127,127,127
				if ((image[j*4+0] == 0) && (image[j*4+1] == 0) && (image[j*4+2] == 0))
				{
					image[j*4+0] =
					image[j*4+1] =
					image[j*4+2] = 127;
				}

				image[j*4+3] = 255;
			}

			if (r_mergeLightmaps->integer)
			{
				R_UpdateSubImage(tr.deluxemaps[lightmapnum], image, xoff, yoff, tr.lightmapSize, tr.lightmapSize );
			}
			else
			{
				tr.deluxemaps[i] = R_CreateImage(va("*deluxemap%d", i), image, tr.lightmapSize, tr.lightmapSize, IMGTYPE_DELUXE, IMGFLAG_NOLIGHTSCALE | IMGFLAG_NO_COMPRESSION | IMGFLAG_CLAMPTOEDGE, 0 );
			}
		}
	}

	if ( r_lightmap->integer == 2 )	{
		ri.Printf( PRINT_ALL, "Brightest lightmap value: %d\n", ( int ) ( maxIntensity * 255 ) );
	}

	ri.Free(image);
}


static float FatPackU(float input, int lightmapnum)
{
	if (lightmapnum < 0)
		return input;

	if (tr.worldDeluxeMapping)
		lightmapnum >>= 1;

	if(tr.fatLightmapSize > 0)
	{
		int             x;

		lightmapnum %= (tr.fatLightmapStep * tr.fatLightmapStep);

		x = lightmapnum % tr.fatLightmapStep;

		return (input / ((float)tr.fatLightmapStep)) + ((1.0 / ((float)tr.fatLightmapStep)) * (float)x);
	}

	return input;
}

static float FatPackV(float input, int lightmapnum)
{
	if (lightmapnum < 0)
		return input;

	if (tr.worldDeluxeMapping)
		lightmapnum >>= 1;

	if(tr.fatLightmapSize > 0)
	{
		int             y;

		lightmapnum %= (tr.fatLightmapStep * tr.fatLightmapStep);

		y = lightmapnum / tr.fatLightmapStep;

		return (input / ((float)tr.fatLightmapStep)) + ((1.0 / ((float)tr.fatLightmapStep)) * (float)y);
	}

	return input;
}


static int FatLightmap(int lightmapnum)
{
	if (lightmapnum < 0)
		return lightmapnum;

	if (tr.worldDeluxeMapping)
		lightmapnum >>= 1;

	if (tr.fatLightmapSize > 0)
	{
		return lightmapnum / (tr.fatLightmapStep * tr.fatLightmapStep);
	}
	
	return lightmapnum;
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
static	void R_LoadVisibility( gamelump_t *l ) {
	int		len;
	byte	*buf;

	len = l->length;
	if ( !len ) {
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
void R_LoadSphereLights(gamelump_t *l) {
	int				i, j;
	mapspherel_t	*at;
	spherel_t		*light;

	if( l->length % sizeof( mapspherel_t ) ) {
		Com_Error( ERR_DROP, "LoadMap: funny lump size in spherelight data for %s\n", s_worldData.name );
	}

	tr.numSLights = l->length / sizeof( mapspherel_t );

	if( tr.numSLights >= MAX_MAP_SPHERE_L_SIZE ) {
		Com_Error( ERR_DROP, "LoadMap: Too many spherelights on map %s, limit is %d\n", s_worldData.name, MAX_MAP_SPHERE_L_SIZE );
	}

	at = l->buffer;
	light = tr.sLights;

	for( i = 0; i < tr.numSLights; at++, light++, i++ ) {
		VectorCopy( at->origin, light->origin );
		VectorCopy( at->color, light->color );
		VectorCopy( at->spot_dir, light->spot_dir );

		for( j = 0; j < 3; j++ ) {
			light->origin[ j ] = LittleFloat( light->origin[ j ] );
			light->color[ j ] = LittleFloat( light->color[ j ] );
			light->spot_dir[ j ] = LittleFloat( light->spot_dir[ j ] );
		}

		light->spot_radiusbydistance = LittleFloat( at->spot_radiusbydistance );
		light->intensity = LittleFloat( at->intensity );
		light->spot_light = LittleLong( at->spot_light );
		light->needs_trace = LittleLong( at->needs_trace );
		light->leaf = &s_worldData.nodes[ s_worldData.numDecisionNodes + LittleLong( at->leaf ) ];
	}
}

/*
================
R_LoadSphereLightVis
================
*/
void R_LoadSphereLightVis(gamelump_t *l) {
	int i;
	int j;
	int h;
	mnode_t* node;
	int numentries;
	int *entries;

	memset( &tr.sSunLight, 0, sizeof( spherel_t ) );

	if ( l->length % 4 ) {
		ri.Error (ERR_DROP, "LoadMap: funny lump size in spherelight vis data for %s\n",
			s_worldData.name);
	} else {
		entries = l->buffer;
		numentries = l->length / sizeof(int);

		if ( numentries ) {
			if ( numentries != s_worldData.numnodes - s_worldData.numDecisionNodes ) {
				for ( i = 0; i < numentries; ++i )
					entries[i] = LittleLong(entries[i]);

				for ( node = &s_worldData.nodes[s_worldData.numDecisionNodes]; numentries; node ++ ) {
					j = 0;
					if ( entries[0] != -1 ) {
						do
							++j;
						while ( entries[j] != -1 );
					}
					if ( j ) {
						node->lights = ri.Hunk_Alloc(j*sizeof(spherel_t*));
						node->numlights = j;
						h = 0;
						if ( entries[0] == -2 ) {
							node->lights[0] = &tr.sSunLight;
							tr.sSunLight.leaf = ( mnode_t * )-1;
							h = 1;
						}
						while ( entries[h] != -1 ) {
							node->lights[h] = &tr.sLights[entries[h]];
							h++;
						}
						entries += h + 1;
						numentries = numentries - 1 - h;
					} else {
						numentries --;
						entries ++;
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

	int _shaderNum = LittleLong( shaderNum );
	if ( _shaderNum < 0 || _shaderNum >= s_worldData.numShaders ) {
		ri.Error( ERR_DROP, "ShaderForShaderNum: bad num %i", _shaderNum );
	}
	dsh = &s_worldData.shaders[ _shaderNum ];

	if ( r_vertexLight->integer || glConfig.hardwareType == GLHW_PERMEDIA2 ) {
		lightmapNum = LIGHTMAP_BY_VERTEX;
	}

	if ( r_fullbright->integer ) {
		lightmapNum = LIGHTMAP_WHITEIMAGE;
	}

	shader = R_FindShader( dsh->shader, lightmapNum, qtrue );

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
void R_UnpackTerraPatch( cTerraPatch_t *pPacked, cTerraPatchUnpacked_t *pUnpacked ) {
	int			i;

	pUnpacked->byDirty = qfalse;
	pUnpacked->visCountCheck = 0;
	pUnpacked->visCountDraw = 0;
	pUnpacked->uiDistRecalc = 0;

	if( pPacked->lmapScale <= 0 ) {
		Com_Error( ERR_DROP, "invalid map: terrain has lmapScale <= 0" );
	}

	pUnpacked->drawinfo.lmapStep = ( pPacked->lmapScale / 64 );
	pUnpacked->drawinfo.lmapSize = ( pPacked->lmapScale * 8 ) | 1;
	pUnpacked->s = ( ( float )pPacked->s + 0.5 ) * 0.0078125;
	pUnpacked->t = ( ( float )pPacked->t + 0.5 ) * 0.0078125;

	//if( s_worldData.lighting ) {
	//	pUnpacked->drawinfo.lmData = &s_worldData.lighting[ 3 * ( ( pPacked->t << 7 ) + pPacked->s + ( pPacked->iLightMap << 14 ) ) ];
	//} else {
	//	pUnpacked->drawinfo.lmData = NULL;
	//}

	// fixme: is it important to have a lightmap data?
	pUnpacked->drawinfo.lmData = NULL;

	memcpy( pUnpacked->texCoord, pPacked->texCoord, sizeof( pUnpacked->texCoord ) );
	pUnpacked->x0 = ( ( int )pPacked->x << 6 );
	pUnpacked->y0 = ( ( int )pPacked->y << 6 );
	pUnpacked->z0 = pPacked->iBaseHeight;
	pUnpacked->shader = ShaderForShaderNum( pPacked->iShader, pPacked->iLightMap );
	pUnpacked->iNorth = pPacked->iNorth;
	pUnpacked->iEast = pPacked->iEast;
	pUnpacked->iSouth = pPacked->iSouth;
	pUnpacked->iWest = pPacked->iWest;

	for( i = 0; i < 63; i++ )
	{
		pUnpacked->varTree[ 0 ][ i / 4 ].fVariance = pPacked->varTree[ 0 ][ i ].flags >> 12;
		pUnpacked->varTree[ 1 ][ i / 4 ].fVariance = pPacked->varTree[ 1 ][ i ].flags >> 12;
	}

	memcpy( pUnpacked->heightmap, pPacked->heightmap, sizeof( pUnpacked->heightmap ) );
	pUnpacked->zmax = 0;
	pUnpacked->flags = pPacked->flags;

	for( i = 0; i < sizeof( pUnpacked->heightmap ); i++ )
	{
		if( pUnpacked->zmax < pUnpacked->heightmap[ i ] ) {
			pUnpacked->zmax = pUnpacked->heightmap[ i ];
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
void R_LoadTerrain( gamelump_t *lump ) {
	int		i;
	cTerraPatch_t			*in;
	cTerraPatchUnpacked_t	*out;

	if( !lump->length ) {
		s_worldData.numTerraPatches = 0;
		s_worldData.terraPatches = NULL;
		return;
	}

	if( lump->length % sizeof( cTerraPatch_t ) ) {
		Com_Error( ERR_DROP, "R_LoadTerrain: funny lump size" );
	}

	s_worldData.numTerraPatches = lump->length / sizeof( cTerraPatch_t );
	s_worldData.terraPatches = ri.Hunk_Alloc( s_worldData.numTerraPatches * sizeof( cTerraPatchUnpacked_t ) );

	in = lump->buffer;
	out = s_worldData.terraPatches;

	for( i = 0; i < s_worldData.numTerraPatches; in++, out++, i++ ) {
		R_UnpackTerraPatch( in, out );
	}
}

/*
================
R_LoadTerrainIndexes
================
*/
void R_LoadTerrainIndexes( gamelump_t *lump ) {
	int		i;
	short					*in;
	cTerraPatchUnpacked_t	**out;

	if( !lump->length ) {
		s_worldData.numVisTerraPatches = 0;
		s_worldData.visTerraPatches = NULL;
		return;
	}

	if( lump->length % sizeof( short ) ) {
		Com_Error( ERR_DROP, "R_LoadTerrainIndexes: funny lump size" );
	}

	s_worldData.numVisTerraPatches = lump->length / sizeof( short );
	s_worldData.visTerraPatches = ri.Hunk_Alloc( s_worldData.numVisTerraPatches * sizeof( cTerraPatchUnpacked_t * ) );

	in = lump->buffer;
	out = s_worldData.visTerraPatches;

	for( i = 0; i < s_worldData.numVisTerraPatches; in++, out++, i++ ) {
		*out = &s_worldData.terraPatches[ *in ];
	}
}

/*
===============
ParseFace
===============
*/
static void ParseFace( dsurface_t *ds, drawVert_t *verts, float *hdrVertColors, msurface_t *surf, int *indexes  ) {
	int			i, j;
	srfBspSurface_t	*cv;
	glIndex_t  *tri;
	int			numVerts, numIndexes, badTriangles;
	int realLightmapNum;

	realLightmapNum = LittleLong( ds->lightmapNum );

	// get fog volume
	surf->fogIndex = LittleLong( ds->fogNum ) + 1;

	// get shader value
	surf->shader = ShaderForShaderNum( ds->shaderNum, FatLightmap(realLightmapNum) );
	if ( r_singleShader->integer && !surf->shader->isSky ) {
		surf->shader = tr.defaultShader;
	}

	numVerts = LittleLong(ds->numVerts);
	if (numVerts > MAX_FACE_POINTS) {
		ri.Printf( PRINT_WARNING, "WARNING: MAX_FACE_POINTS exceeded: %i\n", numVerts);
		numVerts = MAX_FACE_POINTS;
		surf->shader = tr.defaultShader;
	}

	numIndexes = LittleLong(ds->numIndexes);

	//cv = ri.Hunk_Alloc(sizeof(*cv));
	cv = (void *)surf->data;
	cv->surfaceType = SF_FACE;

	cv->numIndexes = numIndexes;
	cv->indexes = ri.Hunk_Alloc(numIndexes * sizeof(cv->indexes[0]));

	cv->numVerts = numVerts;
	cv->verts = ri.Hunk_Alloc(numVerts * sizeof(cv->verts[0]));

	// copy vertexes
	surf->cullinfo.type = CULLINFO_PLANE | CULLINFO_BOX;
	ClearBounds(surf->cullinfo.bounds[0], surf->cullinfo.bounds[1]);
	verts += LittleLong(ds->firstVert);
	for(i = 0; i < numVerts; i++)
	{
		vec4_t color;

		for(j = 0; j < 3; j++)
		{
			cv->verts[i].xyz[j] = LittleFloat(verts[i].xyz[j]);
			cv->verts[i].normal[j] = LittleFloat(verts[i].normal[j]);
		}
		AddPointToBounds(cv->verts[i].xyz, surf->cullinfo.bounds[0], surf->cullinfo.bounds[1]);
		for(j = 0; j < 2; j++)
		{
			cv->verts[i].st[j] = LittleFloat(verts[i].st[j]);
			//cv->verts[i].lightmap[j] = LittleFloat(verts[i].lightmap[j]);
		}
		cv->verts[i].lightmap[0] = FatPackU(LittleFloat(verts[i].lightmap[0]), realLightmapNum);
		cv->verts[i].lightmap[1] = FatPackV(LittleFloat(verts[i].lightmap[1]), realLightmapNum);

		if (hdrVertColors)
		{
			color[0] = hdrVertColors[(ds->firstVert + i) * 3    ];
			color[1] = hdrVertColors[(ds->firstVert + i) * 3 + 1];
			color[2] = hdrVertColors[(ds->firstVert + i) * 3 + 2];
		}
		else
		{
			//hack: convert LDR vertex colors to HDR
			if (r_hdr->integer)
			{
				color[0] = MAX(verts[i].color[0], 0.499f);
				color[1] = MAX(verts[i].color[1], 0.499f);
				color[2] = MAX(verts[i].color[2], 0.499f);
			}
			else
			{
				color[0] = verts[i].color[0];
				color[1] = verts[i].color[1];
				color[2] = verts[i].color[2];
			}

		}
		color[3] = verts[i].color[3] / 255.0f;

		R_ColorShiftLightingFloats( color, cv->verts[i].vertexColors, 1.0f / 255.0f );
	}

	// copy triangles
	badTriangles = 0;
	indexes += LittleLong(ds->firstIndex);
	for(i = 0, tri = cv->indexes; i < numIndexes; i += 3, tri += 3)
	{
		for(j = 0; j < 3; j++)
		{
			tri[j] = LittleLong(indexes[i + j]);

			if(tri[j] >= numVerts)
			{
				ri.Error(ERR_DROP, "Bad index in face surface");
			}
		}

		if ((tri[0] == tri[1]) || (tri[1] == tri[2]) || (tri[0] == tri[2]))
		{
			tri -= 3;
			badTriangles++;
		}
	}

	if (badTriangles)
	{
		ri.Printf(PRINT_WARNING, "Face has bad triangles, originally shader %s %d tris %d verts, now %d tris\n", surf->shader->name, numIndexes / 3, numVerts, numIndexes / 3 - badTriangles);
		cv->numIndexes -= badTriangles * 3;
	}

	// take the plane information from the lightmap vector
	for ( i = 0 ; i < 3 ; i++ ) {
		cv->cullPlane.normal[i] = LittleFloat( ds->lightmapVecs[2][i] );
	}
	cv->cullPlane.dist = DotProduct( cv->verts[0].xyz, cv->cullPlane.normal );
	SetPlaneSignbits( &cv->cullPlane );
	cv->cullPlane.type = PlaneTypeForNormal( cv->cullPlane.normal );
	surf->cullinfo.plane = cv->cullPlane;

	surf->data = (surfaceType_t *)cv;

#ifdef USE_VERT_TANGENT_SPACE
	// Calculate tangent spaces
	{
		srfVert_t      *dv[3];

		for(i = 0, tri = cv->indexes; i < numIndexes; i += 3, tri += 3)
		{
			dv[0] = &cv->verts[tri[0]];
			dv[1] = &cv->verts[tri[1]];
			dv[2] = &cv->verts[tri[2]];

			R_CalcTangentVectors(dv);
		}
	}
#endif
}


/*
===============
ParseMesh
===============
*/
static void ParseMesh ( dsurface_t *ds, drawVert_t *verts, float *hdrVertColors, msurface_t *surf ) {
	srfBspSurface_t	*grid;
	int				i, j;
	int				width, height, numPoints;
	srfVert_t points[MAX_PATCH_SIZE*MAX_PATCH_SIZE];
	vec3_t			bounds[2];
	vec3_t			tmpVec;
	static surfaceType_t	skipData = SF_SKIP;
	int realLightmapNum;

	realLightmapNum = LittleLong( ds->lightmapNum );

	// get fog volume
	surf->fogIndex = LittleLong( ds->fogNum ) + 1;

	// get shader value
	surf->shader = ShaderForShaderNum( ds->shaderNum, FatLightmap(realLightmapNum) );
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

	if(width < 0 || width > MAX_PATCH_SIZE || height < 0 || height > MAX_PATCH_SIZE)
		ri.Error(ERR_DROP, "ParseMesh: bad size");

	verts += LittleLong( ds->firstVert );
	numPoints = width * height;
	for(i = 0; i < numPoints; i++)
	{
		vec4_t color;

		for(j = 0; j < 3; j++)
		{
			points[i].xyz[j] = LittleFloat(verts[i].xyz[j]);
			points[i].normal[j] = LittleFloat(verts[i].normal[j]);
		}

		for(j = 0; j < 2; j++)
		{
			points[i].st[j] = LittleFloat(verts[i].st[j]);
			//points[i].lightmap[j] = LittleFloat(verts[i].lightmap[j]);
		}
		points[i].lightmap[0] = FatPackU(LittleFloat(verts[i].lightmap[0]), realLightmapNum);
		points[i].lightmap[1] = FatPackV(LittleFloat(verts[i].lightmap[1]), realLightmapNum);

		if (hdrVertColors)
		{
			color[0] = hdrVertColors[(ds->firstVert + i) * 3    ];
			color[1] = hdrVertColors[(ds->firstVert + i) * 3 + 1];
			color[2] = hdrVertColors[(ds->firstVert + i) * 3 + 2];
		}
		else
		{
			//hack: convert LDR vertex colors to HDR
			if (r_hdr->integer)
			{
				color[0] = MAX(verts[i].color[0], 0.499f);
				color[1] = MAX(verts[i].color[1], 0.499f);
				color[2] = MAX(verts[i].color[2], 0.499f);
			}
			else
			{
				color[0] = verts[i].color[0];
				color[1] = verts[i].color[1];
				color[2] = verts[i].color[2];
			}
		}
		color[3] = verts[i].color[3] / 255.0f;

		R_ColorShiftLightingFloats( color, points[i].vertexColors, 1.0f / 255.0f );
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
static void ParseTriSurf( dsurface_t *ds, drawVert_t *verts, float *hdrVertColors, msurface_t *surf, int *indexes ) {
	srfBspSurface_t *cv;
	glIndex_t  *tri;
	int             i, j;
	int             numVerts, numIndexes, badTriangles;

	// get fog volume
	surf->fogIndex = LittleLong( ds->fogNum ) + 1;

	// get shader
	//surf->shader = ShaderForShaderNum( ds->shaderNum, LIGHTMAP_BY_VERTEX );
	//if ( r_singleShader->integer && !surf->shader->isSky ) {
	//	surf->shader = tr.defaultShader;
	//}

	numVerts = LittleLong(ds->numVerts);
	numIndexes = LittleLong(ds->numIndexes);

	//cv = ri.Hunk_Alloc(sizeof(*cv));
	cv = (void *)surf->data;
	cv->surfaceType = SF_TRIANGLES;

	cv->numIndexes = numIndexes;
	cv->indexes = ri.Hunk_Alloc(numIndexes * sizeof(cv->indexes[0]));

	cv->numVerts = numVerts;
	cv->verts = ri.Hunk_Alloc(numVerts * sizeof(cv->verts[0]));

	surf->data = (surfaceType_t *) cv;

	// copy vertexes
	surf->cullinfo.type = CULLINFO_BOX;
	ClearBounds(surf->cullinfo.bounds[0], surf->cullinfo.bounds[1]);
	verts += LittleLong(ds->firstVert);
	for(i = 0; i < numVerts; i++)
	{
		vec4_t color;

		for(j = 0; j < 3; j++)
		{
			cv->verts[i].xyz[j] = LittleFloat(verts[i].xyz[j]);
			cv->verts[i].normal[j] = LittleFloat(verts[i].normal[j]);
		}

		AddPointToBounds( cv->verts[i].xyz, surf->cullinfo.bounds[0], surf->cullinfo.bounds[1] );

		for(j = 0; j < 2; j++)
		{
			cv->verts[i].st[j] = LittleFloat(verts[i].st[j]);
			cv->verts[i].lightmap[j] = LittleFloat(verts[i].lightmap[j]);
		}

		if (hdrVertColors)
		{
			color[0] = hdrVertColors[(ds->firstVert + i) * 3    ];
			color[1] = hdrVertColors[(ds->firstVert + i) * 3 + 1];
			color[2] = hdrVertColors[(ds->firstVert + i) * 3 + 2];
		}
		else
		{
			//hack: convert LDR vertex colors to HDR
			if (r_hdr->integer)
			{
				color[0] = MAX(verts[i].color[0], 0.499f);
				color[1] = MAX(verts[i].color[1], 0.499f);
				color[2] = MAX(verts[i].color[2], 0.499f);
			}
			else
			{
				color[0] = verts[i].color[0];
				color[1] = verts[i].color[1];
				color[2] = verts[i].color[2];
			}
		}
		color[3] = verts[i].color[3] / 255.0f;

		R_ColorShiftLightingFloats( color, cv->verts[i].vertexColors, 1.0f / 255.0f );
	}

	// copy triangles
	badTriangles = 0;
	indexes += LittleLong(ds->firstIndex);
	for(i = 0, tri = cv->indexes; i < numIndexes; i += 3, tri += 3)
	{
		for(j = 0; j < 3; j++)
		{
			tri[j] = LittleLong(indexes[i + j]);

			if(tri[j] >= numVerts)
			{
				ri.Error(ERR_DROP, "Bad index in face surface");
			}
		}

		if ((tri[0] == tri[1]) || (tri[1] == tri[2]) || (tri[0] == tri[2]))
		{
			tri -= 3;
			badTriangles++;
		}
	}

	if (badTriangles)
	{
		ri.Printf(PRINT_WARNING, "Trisurf has bad triangles, originally shader %s %d tris %d verts, now %d tris\n", surf->shader->name, numIndexes / 3, numVerts, numIndexes / 3 - badTriangles);
		cv->numIndexes -= badTriangles * 3;
	}

#ifdef USE_VERT_TANGENT_SPACE
	// Calculate tangent spaces
	{
		srfVert_t      *dv[3];

		for(i = 0, tri = cv->indexes; i < numIndexes; i += 3, tri += 3)
		{
			dv[0] = &cv->verts[tri[0]];
			dv[1] = &cv->verts[tri[1]];
			dv[2] = &cv->verts[tri[2]];

			R_CalcTangentVectors(dv);
		}
	}
#endif
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

	//flare = ri.Hunk_Alloc( sizeof( *flare ) );
	flare = (void *)surf->data;
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
int R_MergedWidthPoints(srfBspSurface_t *grid, int offset) {
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
int R_MergedHeightPoints(srfBspSurface_t *grid, int offset) {
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
void R_FixSharedVertexLodError_r( int start, srfBspSurface_t *grid1 ) {
	int j, k, l, m, n, offset1, offset2, touch;
	srfBspSurface_t *grid2;

	for ( j = start; j < s_worldData.numsurfaces; j++ ) {
		//
		grid2 = (srfBspSurface_t *) s_worldData.surfaces[j].data;
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
	srfBspSurface_t *grid1;

	for ( i = 0; i < s_worldData.numsurfaces; i++ ) {
		//
		grid1 = (srfBspSurface_t *) s_worldData.surfaces[i].data;
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
	srfBspSurface_t *grid1, *grid2;
	int k, l, m, n, offset1, offset2, row, column;

	grid1 = (srfBspSurface_t *) s_worldData.surfaces[grid1num].data;
	grid2 = (srfBspSurface_t *) s_worldData.surfaces[grid2num].data;
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

				if ( !grid2 || grid2->width >= MAX_GRID_SIZE )
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

				if (!grid2 || grid2->height >= MAX_GRID_SIZE)
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

				if (!grid2 || grid2->width >= MAX_GRID_SIZE )
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

				if (!grid2 || grid2->height >= MAX_GRID_SIZE)
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
	srfBspSurface_t *grid1, *grid2;

	numstitches = 0;
	grid1 = (srfBspSurface_t *) s_worldData.surfaces[grid1num].data;
	for ( j = 0; j < s_worldData.numsurfaces; j++ ) {
		//
		grid2 = (srfBspSurface_t *) s_worldData.surfaces[j].data;
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
	srfBspSurface_t *grid1;

	numstitches = 0;
	do
	{
		stitched = qfalse;
		for ( i = 0; i < s_worldData.numsurfaces; i++ ) {
			//
			grid1 = (srfBspSurface_t *) s_worldData.surfaces[i].data;
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
	srfBspSurface_t *grid, *hunkgrid;

	for ( i = 0; i < s_worldData.numsurfaces; i++ ) {
		//
		grid = (srfBspSurface_t *) s_worldData.surfaces[i].data;
		// if this surface is not a grid
		if ( grid->surfaceType != SF_GRID )
			continue;
		//
		size = sizeof(*grid);
		hunkgrid = ri.Hunk_Alloc(size);
		Com_Memcpy(hunkgrid, grid, size);

		hunkgrid->widthLodError = ri.Hunk_Alloc( grid->width * 4 );
		Com_Memcpy( hunkgrid->widthLodError, grid->widthLodError, grid->width * 4 );

		hunkgrid->heightLodError = ri.Hunk_Alloc( grid->height * 4 );
		Com_Memcpy( hunkgrid->heightLodError, grid->heightLodError, grid->height * 4 );

		hunkgrid->numIndexes = grid->numIndexes;
		hunkgrid->indexes = ri.Hunk_Alloc(grid->numIndexes * sizeof(glIndex_t));
		Com_Memcpy(hunkgrid->indexes, grid->indexes, grid->numIndexes * sizeof(glIndex_t));

		hunkgrid->numVerts = grid->numVerts;
		hunkgrid->verts = ri.Hunk_Alloc(grid->numVerts * sizeof(srfVert_t));
		Com_Memcpy(hunkgrid->verts, grid->verts, grid->numVerts * sizeof(srfVert_t));

		R_FreeSurfaceGridMesh( grid );

		s_worldData.surfaces[i].data = (void *) hunkgrid;
	}
}


/*
=================
BSPSurfaceCompare
compare function for qsort()
=================
*/
static int BSPSurfaceCompare(const void *a, const void *b)
{
	msurface_t   *aa, *bb;

	aa = *(msurface_t **) a;
	bb = *(msurface_t **) b;

	// shader first
	if(aa->shader->sortedIndex < bb->shader->sortedIndex)
		return -1;

	else if(aa->shader->sortedIndex > bb->shader->sortedIndex)
		return 1;

	// by fogIndex
	if(aa->fogIndex < bb->fogIndex)
		return -1;

	else if(aa->fogIndex > bb->fogIndex)
		return 1;

	// by cubemapIndex
	if(aa->cubemapIndex < bb->cubemapIndex)
		return -1;

	else if(aa->cubemapIndex > bb->cubemapIndex)
		return 1;

	// by leaf
	if (s_worldData.surfacesViewCount[aa - s_worldData.surfaces] < s_worldData.surfacesViewCount[bb - s_worldData.surfaces])
		return -1;

	else if (s_worldData.surfacesViewCount[aa - s_worldData.surfaces] > s_worldData.surfacesViewCount[bb - s_worldData.surfaces])
		return 1;

	// by surface number
	if (aa < bb)
		return -1;

	else if (aa > bb)
		return 1;

	return 0;
}


static void CopyVert(const srfVert_t * in, srfVert_t * out)
{
	VectorCopy(in->xyz,      out->xyz);
#ifdef USE_VERT_TANGENT_SPACE
	VectorCopy4(in->tangent, out->tangent);
#endif
	VectorCopy(in->normal,   out->normal);
	VectorCopy(in->lightdir, out->lightdir);

	VectorCopy2(in->st,       out->st);
	VectorCopy2(in->lightmap, out->lightmap);

	VectorCopy4(in->vertexColors, out->vertexColors);
}


/*
===============
R_CreateWorldVaos
===============
*/
static void R_CreateWorldVaos(void)
{
	int             i, j, k;

	int             numVerts;
	srfVert_t      *verts;

	int             numIndexes;
	glIndex_t      *indexes;

    int             numSortedSurfaces, numSurfaces;
	msurface_t   *surface, **firstSurf, **lastSurf, **currSurf;
	msurface_t  **surfacesSorted;

	vao_t *vao;

	int maxVboSize = 4 * 1024 * 1024;

	int             startTime, endTime;

	startTime = ri.Milliseconds();

	// mark surfaces with best matching leaf, using overlapping bounds
	// using surfaceViewCount[] as leaf number, and surfacesDlightBits[] as coverage * 256
	for (i = 0; i < s_worldData.numWorldSurfaces; i++)
	{
		s_worldData.surfacesViewCount[i] = -1;
	}

	for (i = 0; i < s_worldData.numWorldSurfaces; i++)
	{
		s_worldData.surfacesDlightBits[i] = 0;
	}

	for (i = s_worldData.numDecisionNodes; i < s_worldData.numnodes; i++)
	{
		mnode_t *leaf = s_worldData.nodes + i;

		for (j = leaf->firstmarksurface; j < leaf->firstmarksurface + leaf->nummarksurfaces; j++)
		{
			int surfaceNum = s_worldData.marksurfaces[j];
			msurface_t *surface = s_worldData.surfaces + surfaceNum;
			float coverage = 1.0f;
			int iCoverage;

			for (k = 0; k < 3; k++)
			{
				float left, right;

				if (leaf->mins[k] > surface->cullinfo.bounds[1][k] || surface->cullinfo.bounds[0][k] > leaf->maxs[k])
				{
					coverage = 0.0f;
					break;
				}

				left  = MAX(leaf->mins[k], surface->cullinfo.bounds[0][k]);
				right = MIN(leaf->maxs[k], surface->cullinfo.bounds[1][k]);

				// nudge a bit in case this is an axis aligned wall
				coverage *= right - left + 1.0f/256.0f;
			}

			iCoverage = coverage * 256;

			if (iCoverage > s_worldData.surfacesDlightBits[surfaceNum])
			{
				s_worldData.surfacesDlightBits[surfaceNum] = iCoverage;
				s_worldData.surfacesViewCount[surfaceNum] = i - s_worldData.numDecisionNodes;
			}
		}
	}

	for (i = 0; i < s_worldData.numWorldSurfaces; i++)
	{
		s_worldData.surfacesDlightBits[i] = 0;
	}

	// count surfaces
	numSortedSurfaces = 0;
	for(surface = s_worldData.surfaces; surface < s_worldData.surfaces + s_worldData.numWorldSurfaces; surface++)
	{
		srfBspSurface_t *bspSurf;
		shader_t *shader = surface->shader;

		if (shader->isPortal || shader->isSky || ShaderRequiresCPUDeforms(shader))
			continue;

		// check for this now so we can use srfBspSurface_t* universally in the rest of the function
		if (!(*surface->data == SF_FACE || *surface->data == SF_GRID || *surface->data == SF_TRIANGLES))
			continue;

		bspSurf = (srfBspSurface_t *) surface->data;

		if (!bspSurf->numIndexes || !bspSurf->numVerts)
			continue;

		numSortedSurfaces++;
	}

	// presort surfaces
	surfacesSorted = ri.Malloc(numSortedSurfaces * sizeof(*surfacesSorted));

	j = 0;
	for(surface = s_worldData.surfaces; surface < s_worldData.surfaces + s_worldData.numWorldSurfaces; surface++)
	{
		srfBspSurface_t *bspSurf;
		shader_t *shader = surface->shader;

		if (shader->isPortal || shader->isSky || ShaderRequiresCPUDeforms(shader))
			continue;

		// check for this now so we can use srfBspSurface_t* universally in the rest of the function
		if (!(*surface->data == SF_FACE || *surface->data == SF_GRID || *surface->data == SF_TRIANGLES))
			continue;

		bspSurf = (srfBspSurface_t *) surface->data;

		if (!bspSurf->numIndexes || !bspSurf->numVerts)
			continue;

		surfacesSorted[j++] = surface;
	}

	qsort(surfacesSorted, numSortedSurfaces, sizeof(*surfacesSorted), BSPSurfaceCompare);

	k = 0;
	for(firstSurf = lastSurf = surfacesSorted; firstSurf < surfacesSorted + numSortedSurfaces; firstSurf = lastSurf)
	{
		int currVboSize;

		// Find range of surfaces to place in a VAO by:
		// - Collecting a number of surfaces which fit under maxVboSize, or
		// - All the surfaces with a single shader which go over maxVboSize
		currVboSize = 0;
		while (currVboSize < maxVboSize && lastSurf < surfacesSorted + numSortedSurfaces)
		{
			int addVboSize, currShaderIndex;

			addVboSize = 0;
			currShaderIndex = (*lastSurf)->shader->sortedIndex;

			for(currSurf = lastSurf; currSurf < surfacesSorted + numSortedSurfaces && (*currSurf)->shader->sortedIndex == currShaderIndex; currSurf++)
			{
				srfBspSurface_t *bspSurf = (srfBspSurface_t *) (*currSurf)->data;

				addVboSize += bspSurf->numVerts * sizeof(srfVert_t);
			}

			if (currVboSize != 0 && addVboSize + currVboSize > maxVboSize)
				break;

			lastSurf = currSurf;

			currVboSize += addVboSize;
		}

		// count verts/indexes/surfaces
		numVerts = 0;
		numIndexes = 0;
		numSurfaces = 0;
		for (currSurf = firstSurf; currSurf < lastSurf; currSurf++)
		{
			srfBspSurface_t *bspSurf = (srfBspSurface_t *) (*currSurf)->data;

			numVerts += bspSurf->numVerts;
			numIndexes += bspSurf->numIndexes;
			numSurfaces++;
		}

		ri.Printf(PRINT_ALL, "...calculating world VAO %d ( %i verts %i tris )\n", k, numVerts, numIndexes / 3);

		// create arrays
		verts = ri.Hunk_AllocateTempMemory(numVerts * sizeof(srfVert_t));
		indexes = ri.Hunk_AllocateTempMemory(numIndexes * sizeof(glIndex_t));

		// set up indices and copy vertices
		numVerts = 0;
		numIndexes = 0;
		for (currSurf = firstSurf; currSurf < lastSurf; currSurf++)
		{
			srfBspSurface_t *bspSurf = (srfBspSurface_t *) (*currSurf)->data;
			glIndex_t *surfIndex;

			bspSurf->firstIndex = numIndexes;
			bspSurf->minIndex = numVerts + bspSurf->indexes[0];
			bspSurf->maxIndex = numVerts + bspSurf->indexes[0];

			for(i = 0, surfIndex = bspSurf->indexes; i < bspSurf->numIndexes; i++, surfIndex++)
			{
				indexes[numIndexes++] = numVerts + *surfIndex;
				bspSurf->minIndex = MIN(bspSurf->minIndex, numVerts + *surfIndex);
				bspSurf->maxIndex = MAX(bspSurf->maxIndex, numVerts + *surfIndex);
			}

			bspSurf->firstVert = numVerts;

			for(i = 0; i < bspSurf->numVerts; i++)
			{
				CopyVert(&bspSurf->verts[i], &verts[numVerts++]);
			}
		}

		vao = R_CreateVao2(va("staticBspModel%i_VAO", k), numVerts, verts, numIndexes, indexes);

		// point bsp surfaces to VAO
		for (currSurf = firstSurf; currSurf < lastSurf; currSurf++)
		{
			srfBspSurface_t *bspSurf = (srfBspSurface_t *) (*currSurf)->data;

			bspSurf->vao = vao;
		}

		ri.Hunk_FreeTempMemory(indexes);
		ri.Hunk_FreeTempMemory(verts);

		k++;
	}

	if (r_mergeLeafSurfaces->integer)
	{
		msurface_t *mergedSurf;

		// count merged surfaces
		int numMergedSurfaces = 0, numUnmergedSurfaces = 0;
		for(firstSurf = lastSurf = surfacesSorted; firstSurf < surfacesSorted + numSortedSurfaces; firstSurf = lastSurf)
		{
			for (lastSurf++ ; lastSurf < surfacesSorted + numSortedSurfaces; lastSurf++)
			{
				int lastSurfLeafIndex, firstSurfLeafIndex;

				if ((*lastSurf)->shader         != (*firstSurf)->shader
				 || (*lastSurf)->fogIndex       != (*firstSurf)->fogIndex
				 || (*lastSurf)->cubemapIndex   != (*firstSurf)->cubemapIndex)
					break;

				lastSurfLeafIndex  = s_worldData.surfacesViewCount[*lastSurf  - s_worldData.surfaces];
				firstSurfLeafIndex = s_worldData.surfacesViewCount[*firstSurf - s_worldData.surfaces];

				if (lastSurfLeafIndex != firstSurfLeafIndex)
					break;
			}

			// don't merge single surfaces
			if (firstSurf + 1 == lastSurf)
			{
				numUnmergedSurfaces++;
				continue;
			}

			numMergedSurfaces++;
		}

		// Allocate merged surfaces
		s_worldData.mergedSurfaces = ri.Hunk_Alloc(sizeof(*s_worldData.mergedSurfaces) * numMergedSurfaces);
		s_worldData.mergedSurfacesViewCount = ri.Hunk_Alloc(sizeof(*s_worldData.mergedSurfacesViewCount) * numMergedSurfaces);
		s_worldData.mergedSurfacesDlightBits = ri.Hunk_Alloc(sizeof(*s_worldData.mergedSurfacesDlightBits) * numMergedSurfaces);
		s_worldData.mergedSurfacesPshadowBits = ri.Hunk_Alloc(sizeof(*s_worldData.mergedSurfacesPshadowBits) * numMergedSurfaces);
		s_worldData.numMergedSurfaces = numMergedSurfaces;
		
		// view surfaces are like mark surfaces, except negative ones represent merged surfaces
		// -1 represents 0, -2 represents 1, and so on
		s_worldData.viewSurfaces = ri.Hunk_Alloc(sizeof(*s_worldData.viewSurfaces) * s_worldData.nummarksurfaces);

		// copy view surfaces into mark surfaces
		for (i = 0; i < s_worldData.nummarksurfaces; i++)
		{
			s_worldData.viewSurfaces[i] = s_worldData.marksurfaces[i];
		}

		// actually merge surfaces
		mergedSurf = s_worldData.mergedSurfaces;
		for(firstSurf = lastSurf = surfacesSorted; firstSurf < surfacesSorted + numSortedSurfaces; firstSurf = lastSurf)
		{
			srfBspSurface_t *bspSurf, *vaoSurf;

			for ( lastSurf++ ; lastSurf < surfacesSorted + numSortedSurfaces; lastSurf++)
			{
				int lastSurfLeafIndex, firstSurfLeafIndex;

				if ((*lastSurf)->shader         != (*firstSurf)->shader
				 || (*lastSurf)->fogIndex       != (*firstSurf)->fogIndex
				 || (*lastSurf)->cubemapIndex   != (*firstSurf)->cubemapIndex)
					break;

				lastSurfLeafIndex  = s_worldData.surfacesViewCount[*lastSurf  - s_worldData.surfaces];
				firstSurfLeafIndex = s_worldData.surfacesViewCount[*firstSurf - s_worldData.surfaces];

				if (lastSurfLeafIndex != firstSurfLeafIndex)
					break;
			}

			// don't merge single surfaces
			if (firstSurf + 1 == lastSurf)
				continue;

			bspSurf = (srfBspSurface_t *)(*firstSurf)->data;

			vaoSurf = ri.Hunk_Alloc(sizeof(*vaoSurf));
			memset(vaoSurf, 0, sizeof(*vaoSurf));
			vaoSurf->surfaceType = SF_VAO_MESH;

			vaoSurf->vao = bspSurf->vao;

			vaoSurf->firstIndex = bspSurf->firstIndex;
			vaoSurf->minIndex = bspSurf->minIndex;
			vaoSurf->maxIndex = bspSurf->maxIndex;

			ClearBounds(vaoSurf->cullBounds[0], vaoSurf->cullBounds[1]);
			for (currSurf = firstSurf; currSurf < lastSurf; currSurf++)
			{
				srfBspSurface_t *currBspSurf = (srfBspSurface_t *)(*currSurf)->data;

				vaoSurf->numVerts   += currBspSurf->numVerts;
				vaoSurf->numIndexes += currBspSurf->numIndexes;
				vaoSurf->minIndex = MIN(vaoSurf->minIndex, currBspSurf->minIndex);
				vaoSurf->maxIndex = MAX(vaoSurf->maxIndex, currBspSurf->maxIndex);
				AddPointToBounds((*currSurf)->cullinfo.bounds[0], vaoSurf->cullBounds[0], vaoSurf->cullBounds[1]);
				AddPointToBounds((*currSurf)->cullinfo.bounds[1], vaoSurf->cullBounds[0], vaoSurf->cullBounds[1]);
			}

			VectorCopy(vaoSurf->cullBounds[0], mergedSurf->cullinfo.bounds[0]);
			VectorCopy(vaoSurf->cullBounds[1], mergedSurf->cullinfo.bounds[1]);

			mergedSurf->cullinfo.type =  CULLINFO_BOX;
			mergedSurf->data          =  (surfaceType_t *)vaoSurf;
			mergedSurf->fogIndex      =  (*firstSurf)->fogIndex;
			mergedSurf->cubemapIndex  =  (*firstSurf)->cubemapIndex;
			mergedSurf->shader        =  (*firstSurf)->shader;

			// redirect view surfaces to this surf
			for (currSurf = firstSurf; currSurf < lastSurf; currSurf++)
				s_worldData.surfacesViewCount[*currSurf - s_worldData.surfaces] = -2;

			for (k = 0; k < s_worldData.nummarksurfaces; k++)
			{
				if (s_worldData.surfacesViewCount[s_worldData.marksurfaces[k]] == -2)
					s_worldData.viewSurfaces[k] = -((int)(mergedSurf - s_worldData.mergedSurfaces) + 1);
			}

			for (currSurf = firstSurf; currSurf < lastSurf; currSurf++)
				s_worldData.surfacesViewCount[*currSurf - s_worldData.surfaces] = -1;

			mergedSurf++;
		}

		ri.Printf(PRINT_ALL, "Processed %d mergeable surfaces into %d merged, %d unmerged\n", 
			numSortedSurfaces, numMergedSurfaces, numUnmergedSurfaces);
	}

	for (i = 0; i < s_worldData.numWorldSurfaces; i++)
		s_worldData.surfacesViewCount[i] = -1;

	ri.Free(surfacesSorted);

	endTime = ri.Milliseconds();
	ri.Printf(PRINT_ALL, "world VAOs calculation time = %5.2f seconds\n", (endTime - startTime) / 1000.0);
}

/*
===============
R_LoadSurfaces
===============
*/
static	void R_LoadSurfaces( gamelump_t *surfs, gamelump_t *verts, gamelump_t *indexLump ) {
	dsurface_t	*in;
	msurface_t	*out;
	drawVert_t	*dv;
	int			*indexes;
	int			count;
	int			numFaces, numMeshes, numTriSurfs, numFlares;
	int			i;
	float *hdrVertColors = NULL;
	//int				j;
	//char			buffer[ 65535 ];
	//char			buffer2[ 32 ];

	numFaces = 0;
	numMeshes = 0;
	numTriSurfs = 0;
	numFlares = 0;

	if (surfs->length % sizeof(*in))
		ri.Error (ERR_DROP, "LoadMap: funny lump size in %s",s_worldData.name);
	count = surfs->length / sizeof(*in);

	dv = (void *)verts->buffer;
	if (verts->length % sizeof(*dv))
		ri.Error (ERR_DROP, "LoadMap: funny lump size in %s",s_worldData.name);

	indexes = (void *)indexLump->buffer;
	if ( indexLump->length % sizeof(*indexes))
		ri.Error (ERR_DROP, "LoadMap: funny lump size in %s",s_worldData.name);

	out = ri.Hunk_Alloc ( count * sizeof(*out) );	

	s_worldData.surfaces = out;
	s_worldData.numsurfaces = count;
	s_worldData.surfacesViewCount = ri.Hunk_Alloc ( count * sizeof(*s_worldData.surfacesViewCount) );
	s_worldData.surfacesDlightBits = ri.Hunk_Alloc ( count * sizeof(*s_worldData.surfacesDlightBits) );
	s_worldData.surfacesPshadowBits = ri.Hunk_Alloc ( count * sizeof(*s_worldData.surfacesPshadowBits) );

	// load hdr vertex colors
	if (r_hdr->integer)
	{
		char filename[MAX_QPATH];
		int size;

		Com_sprintf( filename, sizeof( filename ), "maps/%s/vertlight.raw", s_worldData.baseName);
		//ri.Printf(PRINT_ALL, "looking for %s\n", filename);

		size = ri.FS_ReadFile(filename, (void **)&hdrVertColors);

		if (hdrVertColors)
		{
			//ri.Printf(PRINT_ALL, "Found!\n");
			if (size != sizeof(float) * 3 * (verts->length / sizeof(*dv)))
				ri.Error(ERR_DROP, "Bad size for %s (%i, expected %i)!", filename, size, (int)((sizeof(float)) * 3 * (verts->length / sizeof(*dv))));
		}
	}


	// Two passes, allocate surfaces first, then load them full of data
	// This ensures surfaces are close together to reduce L2 cache misses when using VAOs,
	// which don't actually use the verts and indexes
	in = (void *)surfs->buffer;
	out = s_worldData.surfaces;
	for ( i = 0 ; i < count ; i++, in++, out++ ) {
		switch ( LittleLong( in->surfaceType ) ) {
			case MST_PATCH:
				// FIXME: do this
				break;
			case MST_TRIANGLE_SOUP:
				out->data = ri.Hunk_Alloc( sizeof(srfBspSurface_t));
				break;
			case MST_PLANAR:
				out->data = ri.Hunk_Alloc( sizeof(srfBspSurface_t));
				break;
			case MST_FLARE:
				out->data = ri.Hunk_Alloc( sizeof(srfFlare_t));
				break;
			default:
				break;
		}
	}

	in = (void *)surfs->buffer;
	out = s_worldData.surfaces;
	for ( i = 0 ; i < count ; i++, in++, out++ ) {
		switch ( LittleLong( in->surfaceType ) ) {
		case MST_PATCH:
			ParseMesh ( in, dv, hdrVertColors, out );
			{
				srfBspSurface_t *surface = (srfBspSurface_t *)out->data;

				out->cullinfo.type = CULLINFO_BOX | CULLINFO_SPHERE;
				VectorCopy(surface->cullBounds[0], out->cullinfo.bounds[0]);
				VectorCopy(surface->cullBounds[1], out->cullinfo.bounds[1]);
				VectorCopy(surface->cullOrigin, out->cullinfo.localOrigin);
				out->cullinfo.radius = surface->cullRadius;
			}
			numMeshes++;
			break;
		case MST_TRIANGLE_SOUP:
			ParseTriSurf( in, dv, hdrVertColors, out, indexes );
			numTriSurfs++;
			break;
		case MST_PLANAR:
			ParseFace( in, dv, hdrVertColors, out, indexes );
			numFaces++;
			break;
		case MST_FLARE:
			ParseFlare( in, dv, out, indexes );
			{
				out->cullinfo.type = CULLINFO_NONE;
			}
			numFlares++;
			break;
		default:
			ri.Error( ERR_DROP, "Bad surfaceType" );
		}
	}

	/*memset( buffer, 0, sizeof( buffer ) );
	memset( buffer2, 0, sizeof( buffer ) );

	strcat( buffer, "Faces: " );
	sprintf( buffer2, "%d", count );
	strcat( buffer, buffer2 );
	strcat( buffer, "\r\n\r\n" );

	in = ( void * )surfs->buffer;
	out = s_worldData.surfaces;
	for( i = 0; i < count; i++, in++, out++ ) {
		srfBspSurface_t *cv = ( srfBspSurface_t * )out->data;

		for( j = 0; j < cv->numVerts; j++ )
		{
			strcat( buffer, "Face: " );
			sprintf( buffer2, "%d", i );
			strcat( buffer, buffer2 );
			strcat( buffer, "\r\n" );
			strcat( buffer, "Vert: " );
			sprintf( buffer2, "%d", j );
			strcat( buffer, buffer2 );
			strcat( buffer, "\r\n" );
			strcat( buffer, "Indice: " );
			sprintf( buffer2, "%d", cv->indexes[ j ] );
			strcat( buffer, buffer2 );
			strcat( buffer, "\r\n" );
			strcat( buffer, "Pos: " );
			sprintf( buffer2, "%f, %f, %f", cv->verts[ j ].xyz[ 0 ], cv->verts[ j ].xyz[ 1 ], cv->verts[ j ].xyz[ 2 ] );
			strcat( buffer, buffer2 );
			strcat( buffer, "\r\n" );
		}
	}*/

	if (hdrVertColors)
	{
		ri.FS_FreeFile(hdrVertColors);
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


	//Com_Printf( buffer );
	//ri.FS_WriteFile( "dmp.txt", buffer, strlen( buffer ) + 1 );
}



/*
=================
R_LoadSubmodels
=================
*/
static	void R_LoadSubmodels( gamelump_t *l ) {
	dmodel_t	*in;
	bmodel_t	*out;
	int			i, j, count;

	in = (void *)l->buffer;
	if (l->length % sizeof(*in))
		ri.Error (ERR_DROP, "LoadMap: funny lump size in %s",s_worldData.name);
	count = l->length / sizeof(*in);

	s_worldData.numBModels = count;
	s_worldData.bmodels = out = ri.Hunk_Alloc( count * sizeof(*out) );

	for ( i=0 ; i<count ; i++, in++, out++ ) {
		model_t *model;

		model = R_AllocModel();

		assert( model != NULL );			// this should never happen
		if ( model == NULL ) {
			ri.Error(ERR_DROP, "R_LoadSubmodels: R_AllocModel() failed");
		}

		model->type = MOD_BRUSH;
		model->d.bmodel = out;
		Com_sprintf( model->name, sizeof( model->name ), "*%d", i );

		for (j=0 ; j<3 ; j++) {
			out->bounds[0][j] = LittleFloat (in->mins[j]);
			out->bounds[1][j] = LittleFloat (in->maxs[j]);
		}

		out->firstSurface = LittleLong( in->firstSurface );
		out->numSurfaces = LittleLong( in->numSurfaces );

		if(i == 0)
		{
			// Add this for limiting VAO surface creation
			s_worldData.numWorldSurfaces = out->numSurfaces;
		}
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
static	void R_LoadNodesAndLeafs (gamelump_t *nodeLump, gamelump_t *leafLump) {
	int			i, j, p;
	dnode_t		*in;
	dleaf_t		*inLeaf;
	mnode_t 	*out;
	int			numNodes, numLeafs;

	in = (void *)nodeLump->buffer;
	if (nodeLump->length % sizeof(dnode_t) ||
		leafLump->length % sizeof(dleaf_t) ) {
		ri.Error (ERR_DROP, "LoadMap: funny lump size in %s",s_worldData.name);
	}
	numNodes = nodeLump->length / sizeof(dnode_t);
	numLeafs = leafLump->length / sizeof(dleaf_t);

	out = ri.Hunk_Alloc ( (numNodes + numLeafs) * sizeof(*out));	

	s_worldData.nodes = out;
	s_worldData.numnodes = numNodes + numLeafs;
	s_worldData.numDecisionNodes = numNodes;

	// load nodes
	for ( i=0 ; i<numNodes; i++, in++, out++)
	{
		for (j=0 ; j<3 ; j++)
		{
			out->mins[j] = LittleLong (in->mins[j]);
			out->maxs[j] = LittleLong (in->maxs[j]);
		}
	
		p = LittleLong(in->planeNum);
		out->plane = s_worldData.planes + p;

		out->contents = CONTENTS_NODE;	// differentiate from leafs

		for (j=0 ; j<2 ; j++)
		{
			p = LittleLong (in->children[j]);
			if (p >= 0)
				out->children[j] = s_worldData.nodes + p;
			else
				out->children[j] = s_worldData.nodes + numNodes + (-1 - p);
		}
	}
	
	// load leafs
	inLeaf = (void *)leafLump->buffer;
	for ( i=0 ; i<numLeafs ; i++, inLeaf++, out++)
	{
		for (j=0 ; j<3 ; j++)
		{
			out->mins[j] = LittleLong (inLeaf->mins[j]);
			out->maxs[j] = LittleLong (inLeaf->maxs[j]);
		}

		out->cluster = LittleLong(inLeaf->cluster);
		out->area = LittleLong(inLeaf->area);

		if ( out->cluster >= s_worldData.numClusters ) {
			s_worldData.numClusters = out->cluster + 1;
		}

		out->firstmarksurface = LittleLong(inLeaf->firstLeafSurface);
		out->nummarksurfaces = LittleLong(inLeaf->numLeafSurfaces);

		out->firstTerraPatch = LittleLong( inLeaf->firstTerraPatch );
		out->numTerraPatches = LittleLong( inLeaf->numTerraPatches );

		out->firstStaticModel = LittleLong(inLeaf->firstStaticModel);
		out->numStaticModels = LittleLong(inLeaf->numStaticModels);
	}	

	// chain decendants
	R_SetParent (s_worldData.nodes, NULL);
}

/*
=================
R_LoadNodesAndLeafsOld
=================
*/
static	void R_LoadNodesAndLeafsOld (gamelump_t *nodeLump, gamelump_t *leafLump) {
	int			i, j, p;
	dnode_t		*in;
	dleaf_t_ver17	*inLeaf;
	mnode_t 	*out;
	int			numNodes, numLeafs;

	in = nodeLump->buffer;
	if (nodeLump->length % sizeof(dnode_t) ||
		leafLump->length % sizeof(dleaf_t) ) {
		ri.Error (ERR_DROP, "LoadMap: funny lump size in %s",s_worldData.name);
	}
	numNodes = nodeLump->length / sizeof(dnode_t);
	numLeafs = leafLump->length / sizeof(dleaf_t);

	out = ri.Hunk_Alloc ( (numNodes + numLeafs) * sizeof(*out) );

	s_worldData.nodes = out;
	s_worldData.numnodes = numNodes + numLeafs;
	s_worldData.numDecisionNodes = numNodes;

	// load nodes
	for ( i=0 ; i<numNodes; i++, in++, out++)
	{
		for (j=0 ; j<3 ; j++)
		{
			out->mins[j] = LittleLong (in->mins[j]);
			out->maxs[j] = LittleLong (in->maxs[j]);
		}

		p = LittleLong(in->planeNum);
		out->plane = s_worldData.planes + p;

		out->contents = CONTENTS_NODE;	// differentiate from leafs

		for (j=0 ; j<2 ; j++)
		{
			p = LittleLong (in->children[j]);
			if (p >= 0)
				out->children[j] = s_worldData.nodes + p;
			else
				out->children[j] = s_worldData.nodes + numNodes + (-1 - p);
		}
	}

	// load leafs
	inLeaf = leafLump->buffer;
	for ( i=0 ; i<numLeafs ; i++, inLeaf++, out++)
	{
		for (j=0 ; j<3 ; j++)
		{
			out->mins[j] = LittleLong (inLeaf->mins[j]);
			out->maxs[j] = LittleLong (inLeaf->maxs[j]);
		}

		out->cluster = LittleLong(inLeaf->cluster);
		out->area = LittleLong(inLeaf->area);

		if ( out->cluster >= s_worldData.numClusters ) {
			s_worldData.numClusters = out->cluster + 1;
		}

		out->firstmarksurface = ( int )( s_worldData.marksurfaces +
			LittleLong(inLeaf->firstLeafSurface) );
		out->nummarksurfaces = LittleLong(inLeaf->numLeafSurfaces);

		out->firstTerraPatch = LittleLong( inLeaf->firstTerraPatch );
		out->numTerraPatches = LittleLong( inLeaf->numTerraPatches );
	}

	// chain decendants
	R_SetParent (s_worldData.nodes, NULL);
}

//=============================================================================

/*
=================
R_LoadShaders
=================
*/
static	void R_LoadShaders( gamelump_t *l ) {	
	int		i, count;
	dshader_t	*in, *out;
	
	in = l->buffer;
	if (l->length % sizeof(*in))
		ri.Error (ERR_DROP, "LoadMap: funny lump size in %s",s_worldData.name);
	count = l->length / sizeof(*in);
	out = ri.Hunk_Alloc ( count*sizeof(*out) );

	s_worldData.shaders = out;
	s_worldData.numShaders = count;

	Com_Memcpy( out, in, count*sizeof(*out) );

	for ( i=0 ; i<count ; i++ ) {
		out[i].surfaceFlags = LittleLong( out[i].surfaceFlags );
		out[i].contentFlags = LittleLong( out[i].contentFlags );
	}
}


/*
=================
R_LoadMarksurfaces
=================
*/
static	void R_LoadMarksurfaces (gamelump_t *l)
{	
	int		i, j, count;
	int		*in;
	int     *out;
	
	in = l->buffer;
	if (l->length % sizeof(*in))
		ri.Error (ERR_DROP, "LoadMap: funny lump size in %s",s_worldData.name);
	count = l->length / sizeof(*in);
	out = ri.Hunk_Alloc ( count*sizeof(*out));	

	s_worldData.marksurfaces = out;
	s_worldData.nummarksurfaces = count;

	for ( i=0 ; i<count ; i++)
	{
		j = LittleLong(in[i]);
		out[i] = j;
	}
}


/*
=================
R_LoadPlanes
=================
*/
static	void R_LoadPlanes( gamelump_t *l ) {
	int			i, j;
	cplane_t	*out;
	dplane_t 	*in;
	int			count;
	int			bits;
	
	in = l->buffer;
	if (l->length % sizeof(*in))
		ri.Error (ERR_DROP, "LoadMap: funny lump size in %s",s_worldData.name);
	count = l->length / sizeof(*in);
	out = ri.Hunk_Alloc ( count*2*sizeof(*out));	
	
	s_worldData.planes = out;
	s_worldData.numplanes = count;

	for ( i=0 ; i<count ; i++, in++, out++) {
		bits = 0;
		for (j=0 ; j<3 ; j++) {
			out->normal[j] = LittleFloat (in->normal[j]);
			if (out->normal[j] < 0) {
				bits |= 1<<j;
			}
		}

		out->dist = LittleFloat (in->dist);
		out->type = PlaneTypeForNormal( out->normal );
		out->signbits = bits;
	}
}

/*
=================
R_LoadFogs

=================
*/
static	void R_LoadFogs( gamelump_t *l, gamelump_t *brushesLump, gamelump_t *sidesLump ) {
	int			i;
	fog_t		*out;
	dfog_t		*fogs;
	dbrush_t 	*brushes, *brush;
	dbrushside_t	*sides;
	int			count, brushesCount, sidesCount;
	int			sideNum;
	int			planeNum;
	shader_t	*shader;
	float		d;
	int			firstSide;

	fogs = l->buffer;
	if (l->length % sizeof(*fogs)) {
		ri.Error (ERR_DROP, "LoadMap: funny lump size in %s",s_worldData.name);
	}
	count = l->length / sizeof(*fogs);

	// create fog strucutres for them
	s_worldData.numfogs = count + 1;
	s_worldData.fogs = ri.Hunk_Alloc ( s_worldData.numfogs*sizeof(*out));
	out = s_worldData.fogs + 1;

	if ( !count ) {
		return;
	}

	brushes = brushesLump->buffer;
	if (brushesLump->length % sizeof(*brushes)) {
		ri.Error (ERR_DROP, "LoadMap: funny lump size in %s",s_worldData.name);
	}
	brushesCount = brushesLump->length / sizeof(*brushes);

	sides = sidesLump->buffer;
	if (sidesLump->length % sizeof(*sides)) {
		ri.Error (ERR_DROP, "LoadMap: funny lump size in %s",s_worldData.name);
	}
	sidesCount = sidesLump->length / sizeof(*sides);

	for ( i=0 ; i<count ; i++, fogs++) {
		out->originalBrushNumber = LittleLong( fogs->brushNum );

		if ( (unsigned)out->originalBrushNumber >= brushesCount ) {
			ri.Error( ERR_DROP, "fog brushNumber out of range" );
		}
		brush = brushes + out->originalBrushNumber;

		firstSide = LittleLong( brush->firstSide );

			if ( (unsigned)firstSide > sidesCount - 6 ) {
			ri.Error( ERR_DROP, "fog brush sideNumber out of range" );
		}

		// brushes are always sorted with the axial sides first
		sideNum = firstSide + 0;
		planeNum = LittleLong( sides[ sideNum ].planeNum );
		out->bounds[0][0] = -s_worldData.planes[ planeNum ].dist;

		sideNum = firstSide + 1;
		planeNum = LittleLong( sides[ sideNum ].planeNum );
		out->bounds[1][0] = s_worldData.planes[ planeNum ].dist;

		sideNum = firstSide + 2;
		planeNum = LittleLong( sides[ sideNum ].planeNum );
		out->bounds[0][1] = -s_worldData.planes[ planeNum ].dist;

		sideNum = firstSide + 3;
		planeNum = LittleLong( sides[ sideNum ].planeNum );
		out->bounds[1][1] = s_worldData.planes[ planeNum ].dist;

		sideNum = firstSide + 4;
		planeNum = LittleLong( sides[ sideNum ].planeNum );
		out->bounds[0][2] = -s_worldData.planes[ planeNum ].dist;

		sideNum = firstSide + 5;
		planeNum = LittleLong( sides[ sideNum ].planeNum );
		out->bounds[1][2] = s_worldData.planes[ planeNum ].dist;

		// get information from the shader for fog parameters
		shader = R_FindShader( fogs->shader, LIGHTMAP_NONE, qtrue );

		out->parms = shader->fogParms;

		out->colorInt = ColorBytes4 ( shader->fogParms.color[0] * tr.identityLight, 
			                          shader->fogParms.color[1] * tr.identityLight, 
			                          shader->fogParms.color[2] * tr.identityLight, 1.0 );

		d = shader->fogParms.depthForOpaque < 1 ? 1 : shader->fogParms.depthForOpaque;
		out->tcScale = 1.0f / ( d * 8 );

		// set the gradient vector
		sideNum = LittleLong( fogs->visibleSide );

		if ( sideNum == -1 ) {
			out->hasSurface = qfalse;
		} else {
			out->hasSurface = qtrue;
			planeNum = LittleLong( sides[ firstSide + sideNum ].planeNum );
			VectorSubtract( vec3_origin, s_worldData.planes[ planeNum ].normal, out->surface );
			out->surface[3] = -s_worldData.planes[ planeNum ].dist;
		}

		out++;
	}

}


/*
================
R_LoadLightGrid

================
*/
void R_LoadLightGrid( gamelump_t *l ) {
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

	if ( l->length != numGridPoints * 8 ) {
		ri.Printf( PRINT_WARNING, "WARNING: light grid mismatch\n" );
		w->lightGridData = NULL;
		return;
	}

	w->lightGridData = ri.Hunk_Alloc( l->length );
	Com_Memcpy( w->lightGridData, (void *)l->buffer, l->length );

	// deal with overbright bits
	for ( i = 0 ; i < numGridPoints ; i++ ) {
		R_ColorShiftLightingBytes( &w->lightGridData[i*8], &w->lightGridData[i*8] );
		R_ColorShiftLightingBytes( &w->lightGridData[i*8+3], &w->lightGridData[i*8+3] );
	}

	// load hdr lightgrid
	if (r_hdr->integer)
	{
		char filename[MAX_QPATH];
		float *hdrLightGrid;
		int size;

		Com_sprintf( filename, sizeof( filename ), "maps/%s/lightgrid.raw", s_worldData.baseName);
		//ri.Printf(PRINT_ALL, "looking for %s\n", filename);

		size = ri.FS_ReadFile(filename, (void **)&hdrLightGrid);

		if (hdrLightGrid)
		{
			float lightScale = pow(2, r_mapOverBrightBits->integer - tr.overbrightBits);

			//ri.Printf(PRINT_ALL, "found!\n");

			if (size != sizeof(float) * 6 * numGridPoints)
			{
				ri.Error(ERR_DROP, "Bad size for %s (%i, expected %i)!", filename, size, (int)(sizeof(float)) * 6 * numGridPoints);
			}

			w->hdrLightGrid = ri.Hunk_Alloc(size);

			for (i = 0; i < numGridPoints ; i++)
			{
				w->hdrLightGrid[i * 6    ] = hdrLightGrid[i * 6    ] * lightScale;
				w->hdrLightGrid[i * 6 + 1] = hdrLightGrid[i * 6 + 1] * lightScale;
				w->hdrLightGrid[i * 6 + 2] = hdrLightGrid[i * 6 + 2] * lightScale;
				w->hdrLightGrid[i * 6 + 3] = hdrLightGrid[i * 6 + 3] * lightScale;
				w->hdrLightGrid[i * 6 + 4] = hdrLightGrid[i * 6 + 4] * lightScale;
				w->hdrLightGrid[i * 6 + 5] = hdrLightGrid[i * 6 + 5] * lightScale;
			}
		}

		if (hdrLightGrid)
			ri.FS_FreeFile(hdrLightGrid);
	}
}

/*
================
R_LoadEntities
================
*/
void R_LoadEntities( lump_t *l ) {
	char *p, *token, *s;
	char keyname[MAX_TOKEN_CHARS];
	char value[MAX_TOKEN_CHARS];
	world_t	*w;

	w = &s_worldData;
	w->lightGridSize[0] = 64;
	w->lightGridSize[1] = 64;
	w->lightGridSize[2] = 128;

	p = (char *)(fileBase + l->fileofs);

	// store for reference by the cgame
	w->entityString = ri.Hunk_Alloc( l->filelen + 1 );
	strcpy( w->entityString, p );
	w->entityParsePoint = w->entityString;

	token = COM_ParseExt( &p, qtrue );
	if (!*token || *token != '{') {
		return;
	}

	// only parse the world spawn
	while ( 1 ) {	
		// parse key
		token = COM_ParseExt( &p, qtrue );

		if ( !*token || *token == '}' ) {
			break;
		}
		Q_strncpyz(keyname, token, sizeof(keyname));

		// parse value
		token = COM_ParseExt( &p, qtrue );

		if ( !*token || *token == '}' ) {
			break;
		}
		Q_strncpyz(value, token, sizeof(value));

		// check for remapping of shaders for vertex lighting
		s = "vertexremapshader";
		if (!Q_strncmp(keyname, s, strlen(s)) ) {
			s = strchr(value, ';');
			if (!s) {
				ri.Printf( PRINT_WARNING, "WARNING: no semi colon in vertexshaderremap '%s'\n", value );
				break;
			}
			*s++ = 0;
			if (r_vertexLight->integer) {
				R_RemapShader(value, s, "0");
			}
			continue;
		}
		// check for remapping of shaders
		s = "remapshader";
		if (!Q_strncmp(keyname, s, strlen(s)) ) {
			s = strchr(value, ';');
			if (!s) {
				ri.Printf( PRINT_WARNING, "WARNING: no semi colon in shaderremap '%s'\n", value );
				break;
			}
			*s++ = 0;
			R_RemapShader(value, s, "0");
			continue;
		}
		// check for a different grid size
		if (!Q_stricmp(keyname, "gridsize")) {
			sscanf(value, "%f %f %f", &w->lightGridSize[0], &w->lightGridSize[1], &w->lightGridSize[2] );
			continue;
		}

		// check for auto exposure
		if (!Q_stricmp(keyname, "autoExposureMinMax")) {
			sscanf(value, "%f %f", &tr.autoExposureMinMax[0], &tr.autoExposureMinMax[1]);
			continue;
		}
	}
}

/*
=================
R_GetEntityToken
=================
*/
qboolean R_GetEntityToken( char *buffer, int size ) {
	const char	*s;

	s = COM_Parse( &s_worldData.entityParsePoint );
	Q_strncpyz( buffer, s, size );
	if ( !s_worldData.entityParsePoint && !s[0] ) {
		s_worldData.entityParsePoint = s_worldData.entityString;
		return qfalse;
	} else {
		return qtrue;
	}
}

#ifndef MAX_SPAWN_VARS
#define MAX_SPAWN_VARS 64
#endif

// derived from G_ParseSpawnVars() in g_spawn.c
qboolean R_ParseSpawnVars( char *spawnVarChars, int maxSpawnVarChars, int *numSpawnVars, char *spawnVars[MAX_SPAWN_VARS][2] )
{
	char		keyname[MAX_TOKEN_CHARS];
	char		com_token[MAX_TOKEN_CHARS];
	int			numSpawnVarChars = 0;

	*numSpawnVars = 0;

	// parse the opening brace
	if ( !R_GetEntityToken( com_token, sizeof( com_token ) ) ) {
		// end of spawn string
		return qfalse;
	}
	if ( com_token[0] != '{' ) {
		ri.Printf( PRINT_ALL, "R_ParseSpawnVars: found %s when expecting {\n",com_token );
		return qfalse;
	}

	// go through all the key / value pairs
	while ( 1 ) {	
		int keyLength, tokenLength;

		// parse key
		if ( !R_GetEntityToken( keyname, sizeof( keyname ) ) ) {
			ri.Printf( PRINT_ALL, "R_ParseSpawnVars: EOF without closing brace\n" );
			return qfalse;
		}

		if ( keyname[0] == '}' ) {
			break;
		}
		
		// parse value	
		if ( !R_GetEntityToken( com_token, sizeof( com_token ) ) ) {
			ri.Printf( PRINT_ALL, "R_ParseSpawnVars: EOF without closing brace\n" );
			return qfalse;
		}

		if ( com_token[0] == '}' ) {
			ri.Printf( PRINT_ALL, "R_ParseSpawnVars: closing brace without data\n" );
			return qfalse;
		}

		if ( *numSpawnVars == MAX_SPAWN_VARS ) {
			ri.Printf( PRINT_ALL, "R_ParseSpawnVars: MAX_SPAWN_VARS\n" );
			return qfalse;
		}

		keyLength = strlen(keyname) + 1;
		tokenLength = strlen(com_token) + 1;

		if (numSpawnVarChars + keyLength + tokenLength > maxSpawnVarChars)
		{
			ri.Printf( PRINT_ALL, "R_ParseSpawnVars: MAX_SPAWN_VAR_CHARS\n" );
			return qfalse;
		}

		strcpy(spawnVarChars + numSpawnVarChars, keyname);
		spawnVars[ *numSpawnVars ][0] = spawnVarChars + numSpawnVarChars;
		numSpawnVarChars += keyLength;

		strcpy(spawnVarChars + numSpawnVarChars, com_token);
		spawnVars[ *numSpawnVars ][1] = spawnVarChars + numSpawnVarChars;
		numSpawnVarChars += tokenLength;

		(*numSpawnVars)++;
	}

	return qtrue;
}

void R_LoadCubemapEntities(char *cubemapEntityName)
{
	char spawnVarChars[2048];
	int numSpawnVars;
	char *spawnVars[MAX_SPAWN_VARS][2];
	int numCubemaps = 0;

	// count cubemaps
	numCubemaps = 0;
	while(R_ParseSpawnVars(spawnVarChars, sizeof(spawnVarChars), &numSpawnVars, spawnVars))
	{
		int i;

		for (i = 0; i < numSpawnVars; i++)
		{
			if (!Q_stricmp(spawnVars[i][0], "classname") && !Q_stricmp(spawnVars[i][1], cubemapEntityName))
				numCubemaps++;
		}
	}

	if (!numCubemaps)
		return;

	tr.numCubemaps = numCubemaps;
	tr.cubemapOrigins = ri.Hunk_Alloc( tr.numCubemaps * sizeof(*tr.cubemapOrigins) );
	tr.cubemaps = ri.Hunk_Alloc( tr.numCubemaps * sizeof(*tr.cubemaps));

	numCubemaps = 0;
	while(R_ParseSpawnVars(spawnVarChars, sizeof(spawnVarChars), &numSpawnVars, spawnVars))
	{
		int i;
		qboolean isCubemap = qfalse;
		qboolean positionSet = qfalse;
		vec3_t origin;

		for (i = 0; i < numSpawnVars; i++)
		{
			if (!Q_stricmp(spawnVars[i][0], "classname") && !Q_stricmp(spawnVars[i][1], cubemapEntityName))
				isCubemap = qtrue;

			if (!Q_stricmp(spawnVars[i][0], "origin"))
			{
				sscanf(spawnVars[i][1], "%f %f %f", &origin[0], &origin[1], &origin[2]);
				positionSet = qtrue;
			}
		}

		if (isCubemap && positionSet)
		{
			//ri.Printf(PRINT_ALL, "cubemap at %f %f %f\n", origin[0], origin[1], origin[2]);
			VectorCopy(origin, tr.cubemapOrigins[numCubemaps]);
			numCubemaps++;
		}
	}
}


void R_AssignCubemapsToWorldSurfaces(void)
{
	world_t	*w;
	int i;

	w = &s_worldData;

	for (i = 0; i < w->numsurfaces; i++)
	{
		msurface_t *surf = &w->surfaces[i];
		vec3_t surfOrigin;

		if (surf->cullinfo.type & CULLINFO_SPHERE)
		{
			VectorCopy(surf->cullinfo.localOrigin, surfOrigin);
		}
		else if (surf->cullinfo.type & CULLINFO_BOX)
		{
			surfOrigin[0] = (surf->cullinfo.bounds[0][0] + surf->cullinfo.bounds[1][0]) * 0.5f;
			surfOrigin[1] = (surf->cullinfo.bounds[0][1] + surf->cullinfo.bounds[1][1]) * 0.5f;
			surfOrigin[2] = (surf->cullinfo.bounds[0][2] + surf->cullinfo.bounds[1][2]) * 0.5f;
		}
		else
		{
			//ri.Printf(PRINT_ALL, "surface %d has no cubemap\n", i);
			continue;
		}

		surf->cubemapIndex = R_CubemapForPoint(surfOrigin);
		//ri.Printf(PRINT_ALL, "surface %d has cubemap %d\n", i, surf->cubemapIndex);
	}
}


void R_RenderAllCubemaps(void)
{
	int i, j;

	for (i = 0; i < tr.numCubemaps; i++)
	{
		tr.cubemaps[i] = R_CreateImage(va("*cubeMap%d", i), NULL, CUBE_MAP_SIZE, CUBE_MAP_SIZE, IMGTYPE_COLORALPHA, IMGFLAG_NO_COMPRESSION | IMGFLAG_CLAMPTOEDGE | IMGFLAG_MIPMAP | IMGFLAG_CUBEMAP, GL_RGBA8);
	}
	
	for (i = 0; i < tr.numCubemaps; i++)
	{
		for (j = 0; j < 6; j++)
		{
			RE_ClearScene();
			R_RenderCubemapSide(i, j, qfalse);
			R_IssuePendingRenderCommands();
			R_InitNextFrame();
		}
	}
}


void R_CalcVertexLightDirs( void )
{
	int i, k;
	msurface_t *surface;

	for(k = 0, surface = &s_worldData.surfaces[0]; k < s_worldData.numsurfaces /* s_worldData.numWorldSurfaces */; k++, surface++)
	{
		srfBspSurface_t *bspSurf = (srfBspSurface_t *) surface->data;

		switch(bspSurf->surfaceType)
		{
			case SF_FACE:
			case SF_GRID:
			case SF_TRIANGLES:
				for(i = 0; i < bspSurf->numVerts; i++)
					R_LightDirForPoint( bspSurf->verts[i].xyz, bspSurf->verts[i].lightdir, bspSurf->verts[i].normal, &s_worldData );

				break;

			default:
				break;
		}
	}
}

/*
================
R_LoadStaticModelData
================
*/
void R_LoadStaticModelData( gamelump_t *lump ) {
	int		i;
	byte	*pDstColors;
	byte	*pSrcColors;

	if( !lump->length ) {
		s_worldData.numStaticModelData = 0;
		s_worldData.staticModelData = 0;
		return;
	}


	if( lump->length % ( sizeof( byte ) * 3 ) ) {
		Com_Error( 1, "R_LoadStaticModelData: funny lump size" );
	}

	s_worldData.numStaticModelData = lump->length / ( sizeof( byte ) * 3 );
	s_worldData.staticModelData = ri.Hunk_Alloc( s_worldData.numStaticModelData * ( sizeof( byte ) * 4 ) );

	pSrcColors = lump->buffer;
	pDstColors = s_worldData.staticModelData;

	for( i = 0; i < lump->length; i += 3 )
	{
		pDstColors[ 0 ] = pSrcColors[ 0 ];
		pDstColors[ 1 ] = pSrcColors[ 1 ];
		pDstColors[ 2 ] = pSrcColors[ 2 ];
		pDstColors[ 3 ] = -1;

		pSrcColors += sizeof( byte ) * 3;
		pDstColors += sizeof( byte ) * 4;
	}
}

/*
================
R_CopyStaticModel
================
*/
void R_CopyStaticModel( cStaticModel_t *pSM, cStaticModelUnpacked_t *pUnpackedSM ) {
	pUnpackedSM->visCount = 0;
	VectorCopy( pSM->angles, pUnpackedSM->angles );
	VectorCopy( pSM->origin, pUnpackedSM->origin );
	pUnpackedSM->scale = pSM->scale;
	pUnpackedSM->firstVertexData = pSM->firstVertexData * 4 / 3;
	pUnpackedSM->numVertexData = pSM->numVertexData;
	memcpy( pUnpackedSM->model, pSM->model, sizeof( pUnpackedSM->model ) );
}

/*
================
R_LoadStaticModelDefs
================
*/
void R_LoadStaticModelDefs( gamelump_t *lump ) {
	int		i;
	cStaticModel_t			*in;
	cStaticModelUnpacked_t	*out;

	if( !lump->length ) {
		s_worldData.numStaticModels = 0;
		s_worldData.staticModels = NULL;
		return;
	}

	if( lump->length % sizeof( cStaticModel_t ) ) {
		Com_Error( ERR_DROP, "R_LoadStaticModelDefs: funny lump size" );
	}

	s_worldData.numStaticModels = lump->length / sizeof( cStaticModel_t );
	s_worldData.staticModels = ri.Hunk_Alloc( s_worldData.numStaticModels * sizeof( cStaticModelUnpacked_t ) );

	in = lump->buffer;
	out = s_worldData.staticModels;

	for( i = 0; i < s_worldData.numStaticModels; in++, out++, i++ ) {
		R_CopyStaticModel( in, out );
	}
}

/*
================
R_LoadStaticModelIndexes
================
*/
void R_LoadStaticModelIndexes( gamelump_t *lump ) {
	int		i;
	short	*in;
	cStaticModelUnpacked_t **out;

	if( lump->length % sizeof( short ) ) {
		Com_Error( ERR_DROP, "R_LoadStaticModelDefs: funny lump size" );
	}

	s_worldData.numVisStaticModels = lump->length / sizeof( short );
	s_worldData.visStaticModels = ri.Hunk_Alloc( s_worldData.numVisStaticModels * sizeof( cStaticModelUnpacked_t * ) );

	in = lump->buffer;
	out = s_worldData.visStaticModels;

	for( i = 0; i < s_worldData.numVisStaticModels; in++, out++, i++ ) {
		*out = &s_worldData.staticModels[ *in ];
	}
}

/*
==================
R_LoadLump

Loads a lump from the BSP file
==================
*/
int R_LoadLump( fileHandle_t handle, lump_t *lump, gamelump_t *glump, int size )
{
	glump->buffer = NULL;
	glump->length = lump->filelen;

	if( lump->filelen ) {
		glump->buffer = Hunk_AllocateTempMemory( lump->filelen );

		if( FS_Seek( handle, lump->fileofs, FS_SEEK_SET ) ) {
			Com_Error( ERR_DROP, "R_LoadLump: Error seeking to lump." );
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
R_FreeLump

Free a previously allocated lump
==================
*/
void R_FreeLump( gamelump_t *lump )
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
	gamelump_t		lump, lump2, lump3;

	if ( tr.worldMapLoaded ) {
		ri.Error( ERR_DROP, "ERROR: attempted to redundantly load world map" );
	}

	// set default map light scale
	tr.mapLightScale  = 1.0f;
	tr.sunShadowScale = 0.5f;

	// set default sun direction to be used if it isn't
	// overridden by a shader
	tr.sunDirection[0] = 0.45f;
	tr.sunDirection[1] = 0.3f;
	tr.sunDirection[2] = 0.9f;

	VectorNormalize( tr.sunDirection );

	// set default autoexposure settings
	tr.autoExposureMinMax[0] = -2.0f;
	tr.autoExposureMinMax[1] = 2.0f;

	// set default tone mapping settings
	tr.toneMinAvgMaxLevel[0] = -8.0f;
	tr.toneMinAvgMaxLevel[1] = -2.0f;
	tr.toneMinAvgMaxLevel[2] = 0.0f;

	// reset last cascade sun direction so last shadow cascade is rerendered
	VectorClear(tr.lastCascadeSunDirection);

	tr.worldMapLoaded = qtrue;

	// load it
	length = ri.FS_FOpenFile( name, &h, qtrue, qtrue );
	if( length <= 0 ) {
		ri.Error( ERR_DROP, "RE_LoadWorldMap: %s not found", name );
	}

	ri.FS_Read( &header, sizeof( dheader_t ), h );
	map_length = length;
	map_version = header.version;

	header.ident = LittleLong( header.ident );

	// clear tr.world so if the level fails to load, the next
	// try will not look at the partially loaded version
	tr.world = NULL;

	Com_Memset( &s_worldData, 0, sizeof( s_worldData ) );
	Q_strncpyz( s_worldData.name, name, sizeof( s_worldData.name ) );

	Q_strncpyz( s_worldData.baseName, COM_SkipPath( s_worldData.name ), sizeof( s_worldData.name ) );
	COM_StripExtension(s_worldData.baseName, s_worldData.baseName, sizeof(s_worldData.baseName));

	c_gridVerts = 0;

	i = LittleLong( header.version );
	if( i < BSP_BASE_VERSION || i > BSP_VERSION ) {
		ri.Error( ERR_DROP, "RE_LoadWorldMap: %s has wrong version number (%i should be between %i and %i)",
			name, i, BSP_BASE_VERSION, BSP_VERSION );
	}

	// load into heap
	_R( 53 );
	numShaders = R_LoadLump( h, &header.lumps[ LUMP_SHADERS ], &lump, sizeof( dshader_t ) );
	_R( 54 );
	R_LoadShaders( &lump );
	_R( 55 );
	R_FreeLump( &lump );
	_R( 56 );
	numplanes = R_LoadLump( h, &header.lumps[ LUMP_PLANES ], &lump, sizeof( dplane_t ) );
	R_LoadPlanes( &lump );
	_R( 57 );
	R_FreeLump( &lump );
	_R( 58 );
	numLightBytes = R_LoadLump( h, &header.lumps[ LUMP_LIGHTMAPS ], &lump2, sizeof( byte ) );
	_R( 59 );
	numDrawSurfaces = R_LoadLump( h, &header.lumps[ LUMP_SURFACES ], &lump, sizeof( dsurface_t ) );
	_R( 60 );
	R_LoadLightmaps( &lump2, &lump );
	_R( 61 );
	R_FreeLump( &lump2 );
	_R( 62 );
	numDrawVerts = R_LoadLump( h, &header.lumps[ LUMP_DRAWVERTS ], &lump2, sizeof( drawVert_t ) );
	_R( 63 );
	numDrawIndexes = R_LoadLump( h, &header.lumps[ LUMP_DRAWINDEXES ], &lump3, sizeof( int ) );
	_R( 64 );
	R_LoadSurfaces( &lump, &lump2, &lump3 );
	_R( 65 );
	R_FreeLump( &lump3 );
	_R( 66 );
	R_FreeLump( &lump2 );
	_R( 67 );
	R_FreeLump( &lump );
	_R( 68 );
	numleafsurfaces = R_LoadLump( h, &header.lumps[ LUMP_LEAFSURFACES ], &lump, sizeof( int ) );
	_R( 69 );
	R_LoadMarksurfaces( &lump );
	_R( 70 );
	R_FreeLump( &lump );
	_R( 71 );
	numleafs = R_LoadLump( h, &header.lumps[ LUMP_LEAFS ], &lump2, sizeof( dleaf_t ) );
	_R( 72 );
	numnodes = R_LoadLump( h, &header.lumps[ LUMP_NODES ], &lump, sizeof( mnode_t ) );
	_R( 73 );
	if( header.version > BSP_BETA_VERSION ) {
		R_LoadNodesAndLeafs( &lump, &lump2 );
	} else {
		R_LoadNodesAndLeafsOld( &lump, &lump2 );
	}
	_R( 74 );
	R_FreeLump( &lump );
	_R( 75 );
	R_FreeLump( &lump2 );
	_R( 76 );
	numbrushsides = R_LoadLump( h, &header.lumps[ LUMP_BRUSHSIDES ], &lump2, sizeof( dbrushside_t ) );
	numbrushes = R_LoadLump( h, &header.lumps[ LUMP_BRUSHES ], &lump, sizeof( dbrush_t ) );
	numFogs = R_LoadLump( h, &header.lumps[ LUMP_DUMMY10 ], &lump3, sizeof( dfog_t ) );
	R_LoadFogs( &lump3, &lump, &lump2 );
	R_FreeLump( &lump );
	R_FreeLump( &lump2 );
	R_FreeLump( &lump3 );
	nummodels = R_LoadLump( h, &header.lumps[ LUMP_MODELS ], &lump, sizeof( bmodel_t ) );
	_R( 77 );
	R_LoadSubmodels( &lump );
	_R( 78 );
	R_FreeLump( &lump );
	_R( 79 );
	numVisBytes = R_LoadLump( h, &header.lumps[ LUMP_VISIBILITY ], &lump, sizeof( byte ) );
	_R( 80 );
	R_LoadVisibility( &lump );
	_R( 81 );
	R_FreeLump( &lump );
	_R( 82 );
	g_iGridPaletteBytes = R_LoadLump( h, &header.lumps[ LUMP_LIGHTGRIDPALETTE ], &lump, sizeof( byte ) );
	_R( 83 );
	g_iGridOffsets = R_LoadLump( h, &header.lumps[ LUMP_LIGHTGRIDOFFSETS ], &lump2, sizeof( short ) );
	_R( 84 );
	g_iGridDataSize = R_LoadLump( h, &header.lumps[ LUMP_LIGHTGRIDDATA ], &lump3, sizeof( byte ) );
	_R( 85 );
	//R_LoadLightGrid( &lump, &lump2, &lump3 );
	R_LoadLightGrid( &lump3 );
	_R( 86 );
	R_FreeLump( &lump );
	_R( 87 );
	R_FreeLump( &lump2 );
	_R( 88 );
	R_FreeLump( &lump3 );
	_R( 89 );
	numSLights = R_LoadLump( h, &header.lumps[ LUMP_SPHERELIGHTS ], &lump, sizeof( mapspherel_t ) );
	_R( 90 );
	R_LoadSphereLights( &lump );
	_R( 91 );
	R_FreeLump( &lump );
	_R( 92 );
	entLightVisSize = R_LoadLump( h, &header.lumps[ LUMP_SPHERELIGHTVIS ], &lump, sizeof( byte ) );
	_R( 93 );
	R_LoadSphereLightVis( &lump );
	_R( 94 );
	R_FreeLump( &lump );
	_R( 95 );
	g_nTerraPatches = R_LoadLump( h, &header.lumps[ LUMP_TERRAIN ], &lump, sizeof( cTerraPatch_t ) );
	_R( 96 );
	R_LoadTerrain( &lump );
	_R( 97 );
	R_FreeLump( &lump );
	_R( 98 );
	g_nTerPatchIndices = R_LoadLump( h, &header.lumps[ LUMP_TERRAININDEXES ], &lump, sizeof( short ) );
	_R( 99 );
	R_LoadTerrainIndexes( &lump );
	_R( 100 );
	R_FreeLump( &lump );
	_R( 101 );
	if( header.version > BSP_BETA_VERSION )
	{
		g_nStaticModelData = R_LoadLump( h, &header.lumps[ LUMP_STATICMODELDATA ], &lump, sizeof( byte ) );
		_R( 102 );
		R_LoadStaticModelData( &lump );
		_R( 103 );
		R_FreeLump( &lump );
		_R( 104 );
		g_nStaticModels = R_LoadLump( h, &header.lumps[ LUMP_STATICMODELDEF ], &lump, sizeof( cStaticModel_t ) );
		_R( 105 );
		R_LoadStaticModelDefs( &lump );
		_R( 106 );
		R_FreeLump( &lump );
		_R( 107 );
		g_nStaticModelIndices = R_LoadLump( h, &header.lumps[ LUMP_STATICMODELINDEXES ], &lump, sizeof( byte ) );
		_R( 108 );
		R_LoadStaticModelIndexes( &lump );
		_R( 109 );
		R_FreeLump( &lump );
		_R( 110 );
	}
	else
	{
		g_nStaticModelData = 0;
		g_nStaticModels = 0;
		g_nStaticModelIndices = 0;
	}

	// determine vertex light directions
	R_CalcVertexLightDirs();

	// determine which parts of the map are in sunlight
	if (0)
	{
		world_t	*w;
		uint8_t *primaryLightGrid, *data;
		int lightGridSize;
		int i;

		w = &s_worldData;

		lightGridSize = w->lightGridBounds[0] * w->lightGridBounds[1] * w->lightGridBounds[2];
		primaryLightGrid = ri.Malloc(lightGridSize * sizeof(*primaryLightGrid));

		memset(primaryLightGrid, 0, lightGridSize * sizeof(*primaryLightGrid));

		data = w->lightGridData;
		for (i = 0; i < lightGridSize; i++, data += 8)
		{
			int lat, lng;
			vec3_t gridLightDir, gridLightCol;

			// skip samples in wall
			if (!(data[0]+data[1]+data[2]+data[3]+data[4]+data[5]) )
				continue;

			gridLightCol[0] = ByteToFloat(data[3]);
			gridLightCol[1] = ByteToFloat(data[4]);
			gridLightCol[2] = ByteToFloat(data[5]);
			(void)gridLightCol; // Suppress unused-but-set-variable warning

			lat = data[7];
			lng = data[6];
			lat *= (FUNCTABLE_SIZE/256);
			lng *= (FUNCTABLE_SIZE/256);

			// decode X as cos( lat ) * sin( long )
			// decode Y as sin( lat ) * sin( long )
			// decode Z as cos( long )

			gridLightDir[0] = tr.sinTable[(lat+(FUNCTABLE_SIZE/4))&FUNCTABLE_MASK] * tr.sinTable[lng];
			gridLightDir[1] = tr.sinTable[lat] * tr.sinTable[lng];
			gridLightDir[2] = tr.sinTable[(lng+(FUNCTABLE_SIZE/4))&FUNCTABLE_MASK];

			// FIXME: magic number for determining if light direction is close enough to sunlight
			if (DotProduct(gridLightDir, tr.sunDirection) > 0.75f)
			{
				primaryLightGrid[i] = 1;
			}
			else
			{
				primaryLightGrid[i] = 255;
			}
		}

		if (0)
		{
			int i;
			byte *buffer = ri.Malloc(w->lightGridBounds[0] * w->lightGridBounds[1] * 3 + 18);
			byte *out;
			uint8_t *in;
			char fileName[MAX_QPATH];
			
			Com_Memset (buffer, 0, 18);
			buffer[2] = 2;		// uncompressed type
			buffer[12] = w->lightGridBounds[0] & 255;
			buffer[13] = w->lightGridBounds[0] >> 8;
			buffer[14] = w->lightGridBounds[1] & 255;
			buffer[15] = w->lightGridBounds[1] >> 8;
			buffer[16] = 24;	// pixel size

			in = primaryLightGrid;
			for (i = 0; i < w->lightGridBounds[2]; i++)
			{
				int j;

				sprintf(fileName, "primarylg%d.tga", i);

				out = buffer + 18;
				for (j = 0; j < w->lightGridBounds[0] * w->lightGridBounds[1]; j++)
				{
					if (*in == 1)
					{
						*out++ = 255;
						*out++ = 255;
						*out++ = 255;
					}
					else if (*in == 255)
					{
						*out++ = 64;
						*out++ = 64;
						*out++ = 64;
					}
					else
					{
						*out++ = 0;
						*out++ = 0;
						*out++ = 0;
					}
					in++;
				}

				ri.FS_WriteFile(fileName, buffer, w->lightGridBounds[0] * w->lightGridBounds[1] * 3 + 18);
			}

			ri.Free(buffer);
		}

		for (i = 0; i < w->numWorldSurfaces; i++)
		{
			msurface_t *surf = w->surfaces + i;
			cullinfo_t *ci = &surf->cullinfo;

			if(ci->type & CULLINFO_PLANE)
			{
				if (DotProduct(ci->plane.normal, tr.sunDirection) <= 0.0f)
				{
					//ri.Printf(PRINT_ALL, "surface %d is not oriented towards sunlight\n", i);
					continue;
				}
			}

			if(ci->type & CULLINFO_BOX)
			{
				int ibounds[2][3], x, y, z, goodSamples, numSamples;
				vec3_t lightOrigin;

				VectorSubtract( ci->bounds[0], w->lightGridOrigin, lightOrigin );

				ibounds[0][0] = floor(lightOrigin[0] * w->lightGridInverseSize[0]);
				ibounds[0][1] = floor(lightOrigin[1] * w->lightGridInverseSize[1]);
				ibounds[0][2] = floor(lightOrigin[2] * w->lightGridInverseSize[2]);

				VectorSubtract( ci->bounds[1], w->lightGridOrigin, lightOrigin );

				ibounds[1][0] = ceil(lightOrigin[0] * w->lightGridInverseSize[0]);
				ibounds[1][1] = ceil(lightOrigin[1] * w->lightGridInverseSize[1]);
				ibounds[1][2] = ceil(lightOrigin[2] * w->lightGridInverseSize[2]);

				ibounds[0][0] = CLAMP(ibounds[0][0], 0, w->lightGridSize[0]);
				ibounds[0][1] = CLAMP(ibounds[0][1], 0, w->lightGridSize[1]);
				ibounds[0][2] = CLAMP(ibounds[0][2], 0, w->lightGridSize[2]);

				ibounds[1][0] = CLAMP(ibounds[1][0], 0, w->lightGridSize[0]);
				ibounds[1][1] = CLAMP(ibounds[1][1], 0, w->lightGridSize[1]);
				ibounds[1][2] = CLAMP(ibounds[1][2], 0, w->lightGridSize[2]);

				/*
				ri.Printf(PRINT_ALL, "surf %d bounds (%f %f %f)-(%f %f %f) ibounds (%d %d %d)-(%d %d %d)\n", i,
					ci->bounds[0][0], ci->bounds[0][1], ci->bounds[0][2],
					ci->bounds[1][0], ci->bounds[1][1], ci->bounds[1][2],
					ibounds[0][0], ibounds[0][1], ibounds[0][2],
					ibounds[1][0], ibounds[1][1], ibounds[1][2]);
				*/

				goodSamples = 0;
				numSamples = 0;
				for (x = ibounds[0][0]; x <= ibounds[1][0]; x++)
				{
					for (y = ibounds[0][1]; y <= ibounds[1][1]; y++)
					{
						for (z = ibounds[0][2]; z <= ibounds[1][2]; z++)
						{
							uint8_t primaryLight = primaryLightGrid[x * 8 + y * 8 * w->lightGridBounds[0] + z * 8 * w->lightGridBounds[0] * w->lightGridBounds[2]];

							if (primaryLight == 0)
								continue;

							numSamples++;

							if (primaryLight == 1)
								goodSamples++;
						}
					}
				}

				// FIXME: magic number for determining whether object is mostly in sunlight
				if (goodSamples > numSamples * 0.75f)
				{
					//ri.Printf(PRINT_ALL, "surface %d is in sunlight\n", i);
					//surf->primaryLight = 1;
				}
			}
		}

		ri.Free(primaryLightGrid);
	}

	// load cubemaps
	if (r_cubeMapping->integer)
	{
		R_LoadCubemapEntities("misc_cubemap");
		if (!tr.numCubemaps)
		{
			// use deathmatch spawn points as cubemaps
			R_LoadCubemapEntities("info_player_deathmatch");
		}

		if (tr.numCubemaps)
		{
			R_AssignCubemapsToWorldSurfaces();
		}
	}

	// create static VAOS from the world
	R_CreateWorldVaos();

	s_worldData.dataSize = 0;

	// only set tr.world now that we know the entire level has loaded properly
	tr.world = &s_worldData;

	// make sure the VAO glState entry is safe
	R_BindNullVao();

	// Render all cubemaps
	if (r_cubeMapping->integer && tr.numCubemaps)
	{
		R_RenderAllCubemaps();
	}

	ri.FS_CloseFile( h );

	R_InitStaticModels();

	/*
	_R( 111 );
	R_Sphere_InitLights();
	_R( 112 );
	R_InitTerrain();
	_R( 113 );
	R_InitStaticModels();
	_R( 114 );
	R_LevelMarksLoad( name );
	_R( 115 );
	R_VisDebugLoad( name );
	_R( 116 );
	*/
}

/*
=================
RE_MapVersion
=================
*/
int RE_MapVersion( void )
{
	return map_version;
}

/*
=================
RE_PrintBSPFileSizes
=================
*/
void RE_PrintBSPFileSizes( void )
{
	ri.Printf( PRINT_ALL, "%s: %i\n", s_worldData.name, map_length );
	ri.Printf( PRINT_ALL, "%6i   models                %7i\n", nummodels, nummodels * sizeof( bmodel_t ) );
	ri.Printf( PRINT_ALL, "%6i   shaders               %7i\n", numShaders, numShaders * sizeof( dshader_t ) );
	ri.Printf( PRINT_ALL, "%6i   brushes               %7i\n", numbrushes, numbrushes * sizeof( dbrush_t ) );
	ri.Printf( PRINT_ALL, "%6i   brushsides            %7i\n", numbrushsides, numbrushsides * sizeof( dbrushside_t ) );
	ri.Printf( PRINT_ALL, "%6i   fogs                  %7i\n", numFogs, numFogs * sizeof( dfog_t ) );
	ri.Printf( PRINT_ALL, "%6i   planes                %7i\n", numplanes, numplanes * sizeof( dplane_t ) );
	ri.Printf( PRINT_ALL, "%6i   nodes                 %7i\n", numnodes, numnodes * sizeof( mnode_t ) );
	ri.Printf( PRINT_ALL, "%6i   leafs                 %7i\n", numleafs, numleafs * sizeof( dleaf_t ) );
	ri.Printf( PRINT_ALL, "%6i   leafsurfaces          %7i\n", numleafsurfaces, numleafsurfaces * sizeof( int ) );
	ri.Printf( PRINT_ALL, "%6i   drawverts             %7i\n", numDrawVerts, numDrawVerts * sizeof( drawVert_t ) );
	ri.Printf( PRINT_ALL, "%6i   drawindexes           %7i\n", numDrawIndexes, numDrawIndexes * sizeof( int ) );
	ri.Printf( PRINT_ALL, "%6i   drawsurfaces          %7i\n", numDrawSurfaces, numDrawSurfaces * sizeof( drawSurf_t ) );
	ri.Printf( PRINT_ALL, "%6i   lightdefs             %7i\n", numLightDefs, numLightDefs * sizeof( dlightdef_t ) );
	ri.Printf( PRINT_ALL, "%6i   lightmaps             %7i\n", numLightBytes / 49152, numLightBytes );
	ri.Printf( PRINT_ALL, "         visibility            %7i\n", numVisBytes );
	ri.Printf( PRINT_ALL, "%6i   entitylights          %7i\n", numSLights, numSLights * sizeof( mapspherel_t ) );
	ri.Printf( PRINT_ALL, "         entitylightvis        %7i\n", entLightVisSize );
	ri.Printf( PRINT_ALL, "         light grid palette    %7i\n", g_iGridPaletteBytes );
	ri.Printf( PRINT_ALL, "%6i   light grid offsets    %7i\n", g_iGridOffsets, g_iGridOffsets * sizeof( short ) );
	ri.Printf( PRINT_ALL, "         light grid data       %7i\n", g_iGridDataSize );
	ri.Printf( PRINT_ALL, "%6i   terrain               %7i\n", g_nTerraPatches, g_nTerraPatches * sizeof( cTerraPatch_t ) );
	ri.Printf( PRINT_ALL, "%6i   terrain indexes       %7i\n", g_nTerPatchIndices, g_nTerPatchIndices * sizeof( short ) );
	ri.Printf( PRINT_ALL, "         static model data     %7i\n", g_nStaticModelData );
	ri.Printf( PRINT_ALL, "%6i   static models defs    %7i\n", g_nStaticModels, g_nStaticModels * sizeof( cStaticModel_t ) );
	ri.Printf( PRINT_ALL, "         static model indexes  %7i\n", g_nStaticModelIndices );
}
