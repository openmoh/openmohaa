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
#include "tr_local.h"

// tr_shader.c -- this file deals with the parsing and definition of shaders


typedef struct shadertext_s {
    char name[64];
    char* text;
    shader_t* shader;
    struct shadertext_s* next;
} shadertext_t;

static char *s_shaderText;

// the shader is parsed into these global variables, then copied into
// dynamically allocated memory if it is valid.
static	shaderStage_t	unfoggedStages[MAX_SHADER_STAGES];		
static	shader_t		shader;
static	texModInfo_t	texMods[MAX_SHADER_STAGES][TR_MAX_TEXMODS];

#define FILE_HASH_SIZE		8192

static qboolean shader_noPicMip;
static qboolean shader_noMipMaps;
static qboolean shader_force32bit;
static shadertext_t* currentShader = NULL;
static shadertext_t* hashTable[FILE_HASH_SIZE];

#define GL_CLAMP_TO_EDGE 0x812F

/*
================
return a hash value for the filename
================
*/
static long generateHashValue( const char *fname ) {
	int		i;
	long	hash;
	char	letter;

	hash = 0;
	i = 0;
	while (fname[i] != '\0') {
		letter = tolower(fname[i]);
		if (letter =='.') break;				// don't include extension
		if (letter =='\\') letter = '/';		// damn path names
		if (letter == PATH_SEP) letter = '/';		// damn path names
		hash+=(long)(letter)*(i+119);
		i++;
	}
	hash = (hash ^ (hash >> 10) ^ (hash >> 20));
	hash &= (FILE_HASH_SIZE-1);
	return hash;
}

static shadertext_t* AddShaderTextToHash(const char* name, int hash)
{
	shadertext_t* shader;

    shader = (shadertext_t*)ri.Malloc(sizeof(shadertext_t));

    Com_Memset(shader, 0, sizeof(shadertext_t));
    strncpy(shader->name, name, sizeof(shader->name));
    shader->next = hashTable[hash];

    hashTable[hash] = shader;
    return shader;
}

static shadertext_t* AllocShaderText(const char* name)
{
    int hash;

    hash = generateHashValue(name);
    return AddShaderTextToHash(name, hash);
}

/*
====================
FindShaderText

Scans the combined text description of all the shader files for
the given shader name.

return NULL if not found

If found, it will return a valid shader
=====================
*/
static shadertext_t* FindShaderText(const char* name)
{
    shadertext_t* shader;
    int hash;

    hash = generateHashValue(name);
    shader = hashTable[hash];
    if (!shader) {
        return AddShaderTextToHash(name, hash);
    }

    while (Q_stricmp(shader->name, name))
    {
        shader = shader->next;
        if (!shader) {
            return AddShaderTextToHash(name, hash);
        }
    }

    return shader;
}

/*
===============
ParseVector
===============
*/
static qboolean ParseVector( char **text, int count, float *v ) {
	char	*token;
	int		i;

	for ( i = 0 ; i < count ; i++ ) {
		token = COM_ParseExt( text, qfalse );
		if ( !token[0] ) {
			ri.Printf( PRINT_WARNING, "WARNING: missing vector element in shader '%s'\n", shader.name );
			return qfalse;
		}
		v[i] = atof( token );
	}

	return qtrue;
}

void InitStaticShaders()
{
	char* text;
	const char* token;
	char* buf;
	char shadername[64];

    if (ri.FS_ReadFile("scripts/static_shaders.txt", (void**)&buf) == -1)
    {
        ri.Printf(3, "Couldn't find static shaders file: scripts/staticshaders.txt\n");
		return;
    }

    text = buf;
    while (1)
    {
        token = COM_ParseExt(&text, 1);
		if (!*token) {
			break;
		}

        strncpy(shadername, token, 0x40u);
		if (!R_FindShader(shadername, -1, 1, 1, 1, 1)) {
			ri.Printf(3, "InitStaticShaders: Couldn't find shader: %s\n", shadername);
		}
    }

    ri.FS_FreeFile(buf);
}

/*
===============
NameToAFunc
===============
*/
static unsigned NameToAFunc( const char *funcname )
{	
	if ( !Q_stricmp( funcname, "GT0" ) )
	{
		return GLS_ATEST_GT_0;
	}
	else if ( !Q_stricmp( funcname, "LT128" ) )
	{
		return GLS_ATEST_LT_80;
	}
	else if ( !Q_stricmp( funcname, "GE128" ) )
	{
		return GLS_ATEST_GE_80;
	}

	ri.Printf( PRINT_WARNING, "WARNING: invalid alphaFunc name '%s' in shader '%s'\n", funcname, shader.name );
	return 0;
}


/*
===============
NameToSrcBlendMode
===============
*/
static int NameToSrcBlendMode( const char *name )
{
	if ( !Q_stricmp( name, "GL_ONE" ) )
	{
		return GLS_SRCBLEND_ONE;
	}
	else if ( !Q_stricmp( name, "GL_ZERO" ) )
	{
		return GLS_SRCBLEND_ZERO;
	}
	else if ( !Q_stricmp( name, "GL_DST_COLOR" ) )
	{
		return GLS_SRCBLEND_DST_COLOR;
	}
	else if ( !Q_stricmp( name, "GL_ONE_MINUS_DST_COLOR" ) )
	{
		return GLS_SRCBLEND_ONE_MINUS_DST_COLOR;
	}
	else if ( !Q_stricmp( name, "GL_SRC_ALPHA" ) )
	{
		return GLS_SRCBLEND_SRC_ALPHA;
	}
	else if ( !Q_stricmp( name, "GL_ONE_MINUS_SRC_ALPHA" ) )
	{
		return GLS_SRCBLEND_ONE_MINUS_SRC_ALPHA;
	}
	else if ( !Q_stricmp( name, "GL_DST_ALPHA" ) )
	{
		return GLS_SRCBLEND_DST_ALPHA;
	}
	else if ( !Q_stricmp( name, "GL_ONE_MINUS_DST_ALPHA" ) )
	{
		return GLS_SRCBLEND_ONE_MINUS_DST_ALPHA;
	}
	else if ( !Q_stricmp( name, "GL_SRC_ALPHA_SATURATE" ) )
	{
		return GLS_SRCBLEND_ALPHA_SATURATE;
	}

	ri.Printf( PRINT_WARNING, "WARNING: unknown blend mode '%s' in shader '%s', substituting GL_ONE\n", name, shader.name );
	return GLS_SRCBLEND_ONE;
}

/*
===============
NameToDstBlendMode
===============
*/
static int NameToDstBlendMode( const char *name )
{
	if ( !Q_stricmp( name, "GL_ONE" ) )
	{
		return GLS_DSTBLEND_ONE;
	}
	else if ( !Q_stricmp( name, "GL_ZERO" ) )
	{
		return GLS_DSTBLEND_ZERO;
	}
	else if ( !Q_stricmp( name, "GL_SRC_ALPHA" ) )
	{
		return GLS_DSTBLEND_SRC_ALPHA;
	}
	else if ( !Q_stricmp( name, "GL_ONE_MINUS_SRC_ALPHA" ) )
	{
		return GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA;
	}
	else if ( !Q_stricmp( name, "GL_DST_ALPHA" ) )
	{
		return GLS_DSTBLEND_DST_ALPHA;
	}
	else if ( !Q_stricmp( name, "GL_ONE_MINUS_DST_ALPHA" ) )
	{
		return GLS_DSTBLEND_ONE_MINUS_DST_ALPHA;
	}
	else if ( !Q_stricmp( name, "GL_SRC_COLOR" ) )
	{
		return GLS_DSTBLEND_SRC_COLOR;
	}
	else if ( !Q_stricmp( name, "GL_ONE_MINUS_SRC_COLOR" ) )
	{
		return GLS_DSTBLEND_ONE_MINUS_SRC_COLOR;
	}

	ri.Printf( PRINT_WARNING, "WARNING: unknown blend mode '%s' in shader '%s', substituting GL_ONE\n", name, shader.name );
	return GLS_DSTBLEND_ONE;
}

/*
===============
NameToGenFunc
===============
*/
static genFunc_t NameToGenFunc( const char *funcname )
{
	if ( !Q_stricmp( funcname, "sin" ) )
	{
		return GF_SIN;
	}
	else if ( !Q_stricmp( funcname, "square" ) )
	{
		return GF_SQUARE;
	}
	else if ( !Q_stricmp( funcname, "triangle" ) )
	{
		return GF_TRIANGLE;
	}
	else if ( !Q_stricmp( funcname, "sawtooth" ) )
	{
		return GF_SAWTOOTH;
	}
	else if ( !Q_stricmp( funcname, "inversesawtooth" ) )
	{
		return GF_INVERSE_SAWTOOTH;
	}
	else if ( !Q_stricmp( funcname, "noise" ) )
	{
		return GF_NOISE;
	}

	ri.Printf( PRINT_WARNING, "WARNING: invalid genfunc name '%s' in shader '%s'\n", funcname, shader.name );
	return GF_SIN;
}


/*
===================
ParseWaveForm
===================
*/
static void ParseWaveForm( char **text, waveForm_t *wave )
{
	char *token;

	token = COM_ParseExt( text, qfalse );
	if ( token[0] == 0 )
	{
		ri.Printf( PRINT_WARNING, "WARNING: missing waveform parm in shader '%s'\n", shader.name );
		return;
	}
	wave->func = NameToGenFunc( token );

	// BASE, AMP, PHASE, FREQ
	token = COM_ParseExt( text, qfalse );
	if ( token[0] == 0 )
	{
		ri.Printf( PRINT_WARNING, "WARNING: missing waveform parm in shader '%s'\n", shader.name );
		return;
	}
	wave->base = atof( token );

	token = COM_ParseExt( text, qfalse );
	if ( token[0] == 0 )
	{
		ri.Printf( PRINT_WARNING, "WARNING: missing waveform parm in shader '%s'\n", shader.name );
		return;
	}
	wave->amplitude = atof( token );

	token = COM_ParseExt( text, qfalse );
	if ( token[0] == 0 )
	{
		ri.Printf( PRINT_WARNING, "WARNING: missing waveform parm in shader '%s'\n", shader.name );
		return;
	}
	wave->phase = atof( token );

	token = COM_ParseExt( text, qfalse );
	if ( token[0] == 0 )
	{
		ri.Printf( PRINT_WARNING, "WARNING: missing waveform parm in shader '%s'\n", shader.name );
		return;
	}
	wave->frequency = atof( token );
}


/*
===================
ParseTexMod
===================
*/
static void ParseTexMod( char *_text, shaderStage_t *stage, int cntBundle )
{
	const char *token;
	char **text = &_text;
	texModInfo_t *tmi;

	if ( stage->bundle[cntBundle].numTexMods == TR_MAX_TEXMODS ) {
		ri.Error( ERR_DROP, "ERROR: too many tcMod unfoggedStages in shader '%s'\n", shader.name );
		return;
	}

	tmi = &stage->bundle[cntBundle].texMods[stage->bundle[cntBundle].numTexMods];
	stage->bundle[cntBundle].numTexMods++;

	token = COM_ParseExt( text, qfalse );

	//
	// turb
	//
	if ( !Q_stricmp( token, "turb" ) )
	{
		token = COM_ParseExt( text, qfalse );
		if ( token[0] == 0 )
		{
			ri.Printf( PRINT_WARNING, "WARNING: missing tcMod turb parms in shader '%s'\n", shader.name );
			return;
		}
		tmi->wave.base = atof( token );
		token = COM_ParseExt( text, qfalse );
		if ( token[0] == 0 )
		{
			ri.Printf( PRINT_WARNING, "WARNING: missing tcMod turb in shader '%s'\n", shader.name );
			return;
		}
		tmi->wave.amplitude = atof( token );
		token = COM_ParseExt( text, qfalse );
		if ( token[0] == 0 )
		{
			ri.Printf( PRINT_WARNING, "WARNING: missing tcMod turb in shader '%s'\n", shader.name );
			return;
		}
		tmi->wave.phase = atof( token );
		token = COM_ParseExt( text, qfalse );
		if ( token[0] == 0 )
		{
			ri.Printf( PRINT_WARNING, "WARNING: missing tcMod turb in shader '%s'\n", shader.name );
			return;
		}
		tmi->wave.frequency = atof( token );

		tmi->type = TMOD_TURBULENT;
	}
	//
	// scale
	//
	else if ( !Q_stricmp( token, "scale" ) )
	{
		token = COM_ParseExt( text, qfalse );
		if ( token[0] == 0 )
		{
			ri.Printf( PRINT_WARNING, "WARNING: missing scale parms in shader '%s'\n", shader.name );
			return;
		}
		tmi->scale[0] = atof( token );

		token = COM_ParseExt( text, qfalse );
		if ( token[0] == 0 )
		{
			ri.Printf( PRINT_WARNING, "WARNING: missing scale parms in shader '%s'\n", shader.name );
			return;
		}
		tmi->scale[1] = atof( token );
		tmi->type = TMOD_SCALE;
	}
	//
	// scroll
	//
	else if ( !Q_stricmp( token, "scroll" ) )
	{
		token = COM_ParseExt( text, qfalse );
		if ( token[0] == 0 )
		{
			ri.Printf( PRINT_WARNING, "WARNING: missing scale scroll parms in shader '%s'\n", shader.name );
			return;
		}
		tmi->scroll[0] = atof( token );
		token = COM_ParseExt( text, qfalse );
		if ( token[0] == 0 )
		{
			ri.Printf( PRINT_WARNING, "WARNING: missing scale scroll parms in shader '%s'\n", shader.name );
			return;
		}
		tmi->scroll[1] = atof( token );
		tmi->type = TMOD_SCROLL;
	}
	//
	// stretch
	//
	else if ( !Q_stricmp( token, "stretch" ) )
	{
		token = COM_ParseExt( text, qfalse );
		if ( token[0] == 0 )
		{
			ri.Printf( PRINT_WARNING, "WARNING: missing stretch parms in shader '%s'\n", shader.name );
			return;
		}
		tmi->wave.func = NameToGenFunc( token );

		token = COM_ParseExt( text, qfalse );
		if ( token[0] == 0 )
		{
			ri.Printf( PRINT_WARNING, "WARNING: missing stretch parms in shader '%s'\n", shader.name );
			return;
		}
		tmi->wave.base = atof( token );

		token = COM_ParseExt( text, qfalse );
		if ( token[0] == 0 )
		{
			ri.Printf( PRINT_WARNING, "WARNING: missing stretch parms in shader '%s'\n", shader.name );
			return;
		}
		tmi->wave.amplitude = atof( token );

		token = COM_ParseExt( text, qfalse );
		if ( token[0] == 0 )
		{
			ri.Printf( PRINT_WARNING, "WARNING: missing stretch parms in shader '%s'\n", shader.name );
			return;
		}
		tmi->wave.phase = atof( token );

		token = COM_ParseExt( text, qfalse );
		if ( token[0] == 0 )
		{
			ri.Printf( PRINT_WARNING, "WARNING: missing stretch parms in shader '%s'\n", shader.name );
			return;
		}
		tmi->wave.frequency = atof( token );
		
		tmi->type = TMOD_STRETCH;
	}
	//
	// transform
	//
	else if ( !Q_stricmp( token, "transform" ) )
	{
		token = COM_ParseExt( text, qfalse );
		if ( token[0] == 0 )
		{
			ri.Printf( PRINT_WARNING, "WARNING: missing transform parms in shader '%s'\n", shader.name );
			return;
		}
		tmi->matrix[0][0] = atof( token );

		token = COM_ParseExt( text, qfalse );
		if ( token[0] == 0 )
		{
			ri.Printf( PRINT_WARNING, "WARNING: missing transform parms in shader '%s'\n", shader.name );
			return;
		}
		tmi->matrix[0][1] = atof( token );

		token = COM_ParseExt( text, qfalse );
		if ( token[0] == 0 )
		{
			ri.Printf( PRINT_WARNING, "WARNING: missing transform parms in shader '%s'\n", shader.name );
			return;
		}
		tmi->matrix[1][0] = atof( token );

		token = COM_ParseExt( text, qfalse );
		if ( token[0] == 0 )
		{
			ri.Printf( PRINT_WARNING, "WARNING: missing transform parms in shader '%s'\n", shader.name );
			return;
		}
		tmi->matrix[1][1] = atof( token );

		token = COM_ParseExt( text, qfalse );
		if ( token[0] == 0 )
		{
			ri.Printf( PRINT_WARNING, "WARNING: missing transform parms in shader '%s'\n", shader.name );
			return;
		}
		tmi->translate[0] = atof( token );

		token = COM_ParseExt( text, qfalse );
		if ( token[0] == 0 )
		{
			ri.Printf( PRINT_WARNING, "WARNING: missing transform parms in shader '%s'\n", shader.name );
			return;
		}
		tmi->translate[1] = atof( token );

		tmi->type = TMOD_TRANSFORM;
	}
	//
	// rotate
	//
	else if ( !Q_stricmp( token, "rotate" ) )
	{
		token = COM_ParseExt( text, qfalse );
		if ( token[0] == 0 )
		{
			ri.Printf( PRINT_WARNING, "WARNING: missing tcMod rotate parms in shader '%s'\n", shader.name );
			return;
		}
		tmi->rotateSpeed = atof( token );
		tmi->type = TMOD_ROTATE;
	}
	//
	// entityTranslate
	//
	else if ( !Q_stricmp( token, "entityTranslate" ) )
	{
		tmi->type = TMOD_ENTITY_TRANSLATE;
	}
	else
	{
		ri.Printf(PRINT_WARNING, "WARNING: unknown tcMod '%s' in shader '%s'\n", token, shader.name);
	}
}


/*
===================
ParseStage
===================
*/
static qboolean ParseStage(shaderStage_t* stage, char** text)
{
	char* token;
	int depthMaskBits = GLS_DEPTHMASK_TRUE, blendSrcBits = 0, blendDstBits = 0, atestBits = 0, depthFuncBits = 0;
	qboolean depthMaskExplicit = qfalse;
	int cntBundle = 0;
	int depthTestBits = 0;
	int fogBits = 0;

	stage->active = qtrue;
	stage->noMipMaps = shader_noMipMaps;
	stage->noPicMip = shader_noPicMip;
	stage->force32bit = shader_force32bit;

	while (1)
	{
		token = COM_ParseExt(text, qtrue);
		if (!token[0])
		{
			ri.Printf(PRINT_WARNING, "WARNING: no matching '}' found\n");
			return qfalse;
		}

		if (token[0] == '}')
		{
			break;
		}
		// no picmip adjustment
		else if (!Q_stricmp(token, "nomipmaps"))
		{
			stage->noMipMaps = qtrue;
			continue;
		}
		// no picmip adjustment
		else if (!Q_stricmp(token, "nopicmip"))
		{
			stage->noPicMip = qtrue;
			continue;
		}
		// no picmip adjustment
		else if (!Q_stricmp(token, "nofog"))
		{
			fogBits = GLS_FOG;
			continue;
		}
		//
		// map <name>
		//
		else if (!Q_stricmp(token, "map"))
		{
			token = COM_ParseExt(text, qfalse);
			if (!token[0])
			{
				ri.Printf(PRINT_WARNING, "WARNING: missing parameter for 'map' keyword in shader '%s'\n", shader.name);
				return qfalse;
			}

			if (!Q_stricmp(token, "$whiteimage"))
			{
				stage->bundle[cntBundle].image[0] = tr.whiteImage;
				continue;
			}
			else if (!Q_stricmp(token, "$lightmap"))
			{
				stage->bundle[cntBundle].isLightmap = qtrue;
				if (shader.lightmapIndex < 0) {
					stage->bundle[cntBundle].image[0] = tr.whiteImage;
				}
				else {
					stage->bundle[cntBundle].image[0] = tr.lightmaps[shader.lightmapIndex];
				}
				continue;
			}
			else
			{
				stage->bundle[cntBundle].image[0] = R_FindImageFile(token, !stage->noMipMaps, !stage->noPicMip, qfalse, GL_REPEAT, GL_REPEAT);
				if (!stage->bundle[cntBundle].image[0])
				{
					ri.Printf(PRINT_WARNING, "WARNING: R_FindImageFile could not find '%s' in shader '%s'\n", token, shader.name);
					return qfalse;
				}
			}
		}
		//
		// clampmap <name>
		//
		else if (!Q_stricmpn(token, "clampmap", 8))
		{
			int clampx, clampy;

			if (!token[8]) {
				if (!r_forceClampToEdge->value) {
					clampx = GL_CLAMP;
					clampy = GL_CLAMP;
				}
				else {
					clampx = GL_CLAMP_TO_EDGE;
					clampy = GL_CLAMP_TO_EDGE;
				}
			}
			else {
				if (token[8] == 'x')
				{
					if (!r_forceClampToEdge->value) {
						clampx = GL_CLAMP;
					}
					else {
						clampx = GL_CLAMP_TO_EDGE;
					}
					clampy = GL_CLAMP;
				}
				else if (token[8] == 'y')
				{
					clampx = GL_CLAMP;
					if (!r_forceClampToEdge->value) {
						clampy = GL_CLAMP;
					}
					else {
						clampy = GL_CLAMP_TO_EDGE;
					}
				}
				else {
					ri.Printf(PRINT_WARNING, "WARNING: Converting unknown clampmap type to clampmap\n");
					continue;
				}
			}

			token = COM_ParseExt(text, qfalse);
			if (!token[0])
			{
				ri.Printf(PRINT_WARNING, "WARNING: missing parameter for 'clampmap' keyword in shader '%s'\n", shader.name);
				return qfalse;
			}

			stage->bundle[cntBundle].image[0] = R_FindImageFile(token, !stage->noMipMaps, !stage->noPicMip, qfalse, clampx, clampy);
			if (!stage->bundle[cntBundle].image[0])
			{
				ri.Printf(PRINT_WARNING, "WARNING: R_FindImageFile could not find '%s' in shader '%s'\n", token, shader.name);
				return qfalse;
			}
		}
		//
		// animMap <frequency> <image1> .... <imageN>
		//
		else if ( !Q_stricmp( token, "animMap" ) || !Q_stricmp(token, "animMapOnce") || !Q_stricmp(token, "animMapPhase"))
		{
			qboolean phased;

			if (!Q_stricmp(token, "animMapOnce")) {
				stage->bundle[cntBundle].flags |= BUNDLE_ANIMATE_ONCE;
			}

			phased = !Q_stricmp(token, "animMapPhase");

			token = COM_ParseExt( text, qfalse );
			if ( !token[0] )
			{
				ri.Printf( PRINT_WARNING, "WARNING: missing parameter for 'animMmap' keyword in shader '%s'\n", shader.name );
				return qfalse;
			}
			stage->bundle[cntBundle].imageAnimationSpeed = atof( token );

			if (phased)
            {
                token = COM_ParseExt(text, qfalse);
                if (!token[0])
                {
                    ri.Printf(PRINT_WARNING, "WARNING: missing phase for 'animMapPhase' keyword in shader '%s'\n", shader.name);
                    return qfalse;
                }

				stage->bundle[cntBundle].imageAnimationPhase = atof(token);
			}
			// parse up to MAX_IMAGE_ANIMATIONS animations
			while ( 1 ) {
				int		num;

				token = COM_ParseExt( text, qfalse );
				if ( !token[0] ) {
					break;
				}
				num = stage->bundle[cntBundle].numImageAnimations;
				if ( num < MAX_IMAGE_ANIMATIONS ) {
                    stage->bundle[cntBundle].image[0] = R_FindImageFile(token, !stage->noMipMaps, !stage->noPicMip, qfalse, GL_REPEAT, GL_REPEAT );
					if ( !stage->bundle[cntBundle].image[num] )
					{
						ri.Printf( PRINT_WARNING, "WARNING: R_FindImageFile could not find '%s' in shader '%s'\n", token, shader.name );
						return qfalse;
					}
					stage->bundle[cntBundle].numImageAnimations++;
				}
			}
		}
		//
		// alphafunc <func>
		//
		else if ( !Q_stricmp( token, "alphaFunc" ) )
		{
			token = COM_ParseExt( text, qfalse );
			if ( !token[0] )
			{
				ri.Printf( PRINT_WARNING, "WARNING: missing parameter for 'alphaFunc' keyword in shader '%s'\n", shader.name );
				return qfalse;
			}

			atestBits = NameToAFunc( token );
		}
		//
		// depthFunc <func>
		//
		else if ( !Q_stricmp( token, "depthfunc" ) )
		{
			token = COM_ParseExt( text, qfalse );

			if ( !token[0] )
			{
				ri.Printf( PRINT_WARNING, "WARNING: missing parameter for 'depthfunc' keyword in shader '%s'\n", shader.name );
				return qfalse;
			}

			if ( !Q_stricmp( token, "lequal" ) )
			{
				depthTestBits = 0;
			}
			else if ( !Q_stricmp( token, "equal" ) )
			{
				depthTestBits = GLS_DEPTHFUNC_EQUAL;
			}
			else
			{
				ri.Printf( PRINT_WARNING, "WARNING: unknown depthfunc '%s' in shader '%s'\n", token, shader.name );
				continue;
			}
		}
		//
		// blendfunc <srcFactor> <dstFactor>
		// or blendfunc <add|filter|blend>
		//
		else if ( !Q_stricmp( token, "blendfunc" ) )
		{
			token = COM_ParseExt( text, qfalse );
			if ( token[0] == 0 )
			{
				ri.Printf( PRINT_WARNING, "WARNING: missing parm for blendFunc in shader '%s'\n", shader.name );
				continue;
			}
			// check for "simple" blends first
			if ( !Q_stricmp( token, "add" ) ) {
				blendSrcBits = GLS_SRCBLEND_ONE;
				blendDstBits = GLS_DSTBLEND_ONE;
			} else if ( !Q_stricmp( token, "filter" ) ) {
				blendSrcBits = GLS_SRCBLEND_DST_COLOR;
				blendDstBits = GLS_DSTBLEND_ZERO;
			} else if ( !Q_stricmp( token, "blend" ) ) {
				blendSrcBits = GLS_SRCBLEND_SRC_ALPHA;
				blendDstBits = GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA;
			} else if ( !Q_stricmp( token, "alphaadd" ) ) {
				blendSrcBits = GLS_SRCBLEND_SRC_ALPHA;
				blendDstBits = GLS_DSTBLEND_ONE;
			} else {
				// complex double blends
				blendSrcBits = NameToSrcBlendMode( token );

				token = COM_ParseExt( text, qfalse );
				if ( token[0] == 0 )
				{
					ri.Printf( PRINT_WARNING, "WARNING: missing parm for blendFunc in shader '%s'\n", shader.name );
					continue;
				}
				blendDstBits = NameToDstBlendMode( token );
			}

			// clear depth mask for blended surfaces
			if ( !depthMaskExplicit )
			{
				depthMaskBits = 0;
			}
		}
		//
		// rgbGen
		//
		else if ( !Q_stricmp( token, "rgbGen" ) )
		{
			token = COM_ParseExt( text, qfalse );
			if ( token[0] == 0 )
			{
				ri.Printf( PRINT_WARNING, "WARNING: missing parameters for rgbGen in shader '%s'\n", shader.name );
				continue;
			}

			if ( !Q_stricmp( token, "wave" ) )
			{
				ParseWaveForm( text, &stage->rgbWave );
				stage->rgbGen = CGEN_WAVEFORM;
			}
			else if (!Q_stricmp(token, "colorwave"))
			{
				vec3_t v;

				ParseVector(text, 3, v);
				stage->colorConst[0] = v[0] * 255.0;
				stage->colorConst[1] = v[1] * 255.0;
				stage->colorConst[2] = v[2] * 255.0;
				stage->rgbGen = CGEN_MULTIPLY_BY_WAVEFORM;
				shader.flags |= 2;
			}
			else if (!Q_stricmp(token, "identity"))
			{
				stage->rgbGen = CGEN_IDENTITY;
			}
			else if (!Q_stricmp(token, "identityLighting"))
			{
				stage->rgbGen = CGEN_IDENTITY_LIGHTING;
			}
			else if (!Q_stricmp(token, "global"))
			{
				stage->rgbGen = CGEN_GLOBAL_COLOR;
			}
			else if (!Q_stricmp(token, "entity") || !Q_stricmp(token, "fromentity"))
			{
				stage->rgbGen = CGEN_ENTITY;
			}
			else if (!Q_stricmp(token, "oneMinusEntity"))
			{
				stage->rgbGen = CGEN_ONE_MINUS_ENTITY;
			}
			else if (!Q_stricmp(token, "vertex") || !Q_stricmp(token, "fromclient"))
			{
				stage->rgbGen = CGEN_VERTEX;
				if (stage->alphaGen == 0) {
					stage->alphaGen = AGEN_VERTEX;
				}
			}
			else if (!Q_stricmp(token, "exactVertex"))
			{
				stage->rgbGen = CGEN_EXACT_VERTEX;
			}
			else if (!Q_stricmp(token, "lightingGrid"))
			{
				stage->rgbGen = CGEN_LIGHTING_GRID;
			}
			else if (!Q_stricmp(token, "lightingSpherical"))
			{
				stage->rgbGen = CGEN_LIGHTING_SPHERICAL;
			}
			else if (!Q_stricmp(token, "oneMinusVertex"))
			{
				stage->rgbGen = CGEN_ONE_MINUS_VERTEX;
			}
			else if ( !Q_stricmp( token, "const" ) || !Q_stricmp(token, "constant"))
			{
				vec3_t	color;

				ParseVector( text, 3, color );
				stage->colorConst[0] = 255 * color[0];
				stage->colorConst[1] = 255 * color[1];
				stage->colorConst[2] = 255 * color[2];

				stage->rgbGen = CGEN_CONSTANT;
			}
			else if (!Q_stricmp(token, "static"))
			{
				stage->rgbGen = CGEN_STATIC;
				if (stage->alphaGen == AGEN_IDENTITY) {
					stage->alphaGen = AGEN_VERTEX;
				}
			}
			else if (!Q_stricmp(token, "sCoord") || !Q_stricmp(token, "tCoord"))
			{
				if (!Q_stricmp(token, "sCoord")) {
					stage->rgbGen = CGEN_SCOORD;
				}
				else {
					stage->rgbGen = CGEN_TCOORD;
				}

				stage->alphaMin = 0.0;
				stage->alphaMax = 1.0;
				stage->alphaConstMin = 0;
				stage->alphaConst = -1;

				token = COM_ParseExt(text, qfalse);
				if (token[0] == 0) {
					continue;
				}

				stage->alphaMin = atof(token);

				token = COM_ParseExt(text, qfalse);
				if (token[0] == 0) {
					continue;
				}

				stage->alphaMax = atof(token);

				token = COM_ParseExt(text, qfalse);
				if (token[0] == 0) {
					continue;
				}

				stage->alphaConstMin = atof(token) * 255.0;

				token = COM_ParseExt(text, qfalse);
				if (token[0] == 0) {
					ri.Printf(PRINT_WARNING, "WARNING: missing rgbGen sCoord or tCoord parm 'max' in shader '%s'\n", shader.name);
					continue;
				}

				stage->alphaConst = atof(token) * 255.0;
			}
			else if (!Q_stricmp(token, "dot"))
			{
				stage->rgbGen = CGEN_DOT;
				stage->alphaMin = 0.0;
				stage->alphaMax = 1.0;

				token = COM_ParseExt(text, qfalse);
				if (token[0] == 0) {
					continue;
				}
				stage->alphaMin = atof(token);

				token = COM_ParseExt(text, qfalse);
				if (token[0] == 0) {
					continue;
				}
				stage->alphaMax = atof(token);
				}
			else if (!Q_stricmp(token, "oneminusdot"))
			{
				stage->rgbGen = CGEN_ONE_MINUS_DOT;
				stage->alphaMin = 0.0;
				stage->alphaMax = 1.0;

				token = COM_ParseExt(text, qfalse);
				if (token[0] == 0) {
					continue;
				}
				stage->alphaMin = atof(token);

				token = COM_ParseExt(text, qfalse);
				if (token[0] == 0) {
					continue;
				}
				stage->alphaMax = atof(token);
			}
			else
			{
				ri.Printf( PRINT_WARNING, "WARNING: unknown rgbGen parameter '%s' in shader '%s'\n", token, shader.name );
				continue;
			}
		}
		//
		// alphaGen 
		//
		else if ( !Q_stricmp( token, "alphaGen" ) )
		{
			token = COM_ParseExt( text, qfalse );
			if ( token[0] == 0 )
			{
				ri.Printf( PRINT_WARNING, "WARNING: missing parameters for alphaGen in shader '%s'\n", shader.name );
				continue;
			}

			if ( !Q_stricmp( token, "wave" ) )
			{
				ParseWaveForm( text, &stage->alphaWave );
				stage->alphaGen = AGEN_WAVEFORM;
			}
			else if (!Q_stricmp(token, "global"))
			{
				stage->alphaGen = AGEN_GLOBAL_ALPHA;
			}
			else if ( !Q_stricmp( token, "entity" ) || !Q_stricmp(token, "fromentity"))
			{
				stage->alphaGen = AGEN_ENTITY;
			}
			else if ( !Q_stricmp( token, "oneMinusEntity" ) )
			{
				stage->alphaGen = AGEN_ONE_MINUS_ENTITY;
			}
			else if ( !Q_stricmp( token, "vertex" ) || !Q_stricmp(token, "fromclient"))
			{
				stage->alphaGen = AGEN_VERTEX;
			}
			else if ( !Q_stricmp( token, "oneMinusVertex" ) || !Q_stricmp(token, "oneMinusFromClient"))
			{
				stage->alphaGen = AGEN_ONE_MINUS_VERTEX;
			}
			else if (!Q_stricmp(token, "lightingSpecular"))
			{
				stage->alphaGen = AGEN_LIGHTING_SPECULAR;
			}
			else if (!Q_stricmp(token, "distFade") || !Q_stricmp(token, "oneMinusDistFade")
				|| !Q_stricmp(token, "tikiDistFade") || !Q_stricmp(token, "oneMinusTikiDistFade"))
			{
				if (!Q_stricmp(token, "distFade")) {
					stage->alphaGen = AGEN_DIST_FADE;
				} else if (!Q_stricmp(token, "oneMinusDistFade")) {
					stage->alphaGen = AGEN_ONE_MINUS_DIST_FADE;
				} else if (!Q_stricmp(token, "tikiDistFade")) {
                    stage->alphaGen = AGEN_TIKI_DIST_FADE;
                } else {
                    stage->alphaGen = AGEN_ONE_MINUS_TIKI_DIST_FADE;
                }

				shader.fDistNear = 256.0;
				shader.fDistRange = 256.0;

				token = COM_ParseExt(text, qfalse);
				if (token[0] == 0) {
					continue;
				}
				shader.fDistNear = atof(token);

				token = COM_ParseExt(text, qfalse);
				if (token[0] == 0) {
					continue;
				}
				shader.fDistRange = atof(token);
			}
			else if (!Q_stricmp(token, "dot"))
			{
				shader.needsNormal = qtrue;
				stage->alphaMin = 0.0;
				stage->alphaMax = 1.0;
				stage->alphaGen = AGEN_DOT;

				token = COM_ParseExt(text, qfalse);
				if (token[0] == 0) {
					continue;
				}
				stage->alphaMin = atof(token);

				token = COM_ParseExt(text, qfalse);
				if (token[0] == 0) {
					continue;
				}
				stage->alphaMax = atof(token);
			}
			else if (!Q_stricmp(token, "heightFade"))
            {
                shader.needsNormal = qtrue;
                stage->alphaMin = 256.0f;
                stage->alphaMax = 512.0f;
                stage->alphaGen = AGEN_HEIGHT_FADE;

                token = COM_ParseExt(text, qfalse);
                if (token[0] == 0) {
                    continue;
                }
                stage->alphaMin = atof(token);

                token = COM_ParseExt(text, qfalse);
                if (token[0] == 0) {
                    continue;
                }
                stage->alphaMax = atof(token);
			}
			else if (!Q_stricmp(token, "oneMinusDot"))
			{
				shader.needsNormal = qtrue;
				stage->alphaMin = 0.0;
				stage->alphaMax = 1.0;
				stage->alphaGen = AGEN_ONE_MINUS_DOT;

				token = COM_ParseExt(text, qfalse);
				if (token[0] == 0) {
					continue;
				}
				stage->alphaMin = atof(token);

				token = COM_ParseExt(text, qfalse);
				if (token[0] == 0) {
					continue;
				}
				stage->alphaMax = atof(token);
			}
			else if (!Q_stricmp(token, "dotView"))
			{
				shader.needsNormal = qtrue;
				stage->alphaMin = 0.0;
				stage->alphaMax = 1.0;
				stage->alphaGen = AGEN_DOT_VIEW;

				token = COM_ParseExt(text, qfalse);
				if (token[0] == 0) {
					continue;
				}
				stage->alphaMin = atof(token);

				token = COM_ParseExt(text, qfalse);
				if (token[0] == 0) {
					continue;
				}
				stage->alphaMax = atof(token);
			}
			else if (!Q_stricmp(token, "oneMinusDotView"))
			{
				shader.needsNormal = qtrue;
				stage->alphaMin = 0.0;
				stage->alphaMax = 1.0;
				stage->alphaGen = AGEN_ONE_MINUS_DOT_VIEW;

				token = COM_ParseExt(text, qfalse);
				if (token[0] == 0) {
					continue;
				}
				stage->alphaMin = atof(token);

				token = COM_ParseExt(text, qfalse);
				if (token[0] == 0) {
					continue;
				}
				stage->alphaMax = atof(token);
			}
			else if (!Q_stricmp(token, "const") || !Q_stricmp(token, "constant"))
			{
				token = COM_ParseExt(text, qfalse);
				stage->colorConst[3] = 255 * atof(token);
				stage->alphaGen = AGEN_CONSTANT;
			}
			else if (!Q_stricmp(token, "skyAlpha"))
			{
				stage->alphaGen = AGEN_SKYALPHA;
			}
			else if (!Q_stricmp(token, "oneMinusSkyAlpha"))
			{
				stage->alphaGen = AGEN_ONE_MINUS_SKYALPHA;
			}
			else if (!Q_stricmp(token, "sCoord") || !Q_stricmp(token, "tCoord"))
			{
				if (!Q_stricmp(token, "sCoord")) {
					stage->alphaGen = AGEN_SCOORD;
				} else {
					stage->alphaGen = AGEN_TCOORD;
				}

				stage->alphaMin = 0.0;
				stage->alphaMax = 1.0;
				stage->alphaConstMin = 0;
				stage->alphaConst = -1;

				token = COM_ParseExt(text, qfalse);
				if (token[0] == 0) {
					continue;
				}
				stage->alphaMin = atof(token);

				token = COM_ParseExt(text, qfalse);
				if (token[0] == 0) {
					continue;
				}
				stage->alphaMax = atof(token);

				token = COM_ParseExt(text, qfalse);
				if (token[0] == 0) {
					continue;
				}
				stage->alphaConstMin = atof(token) * 255.0;

				token = COM_ParseExt(text, qfalse);
				if (token[0] == 0) {
					ri.Printf(PRINT_WARNING, "WARNING: missing alphaGen s or tCoord parm 'max' in shader '%s'\n", shader.name);
					continue;
				}
				stage->alphaConst = atof(token) * 255.0;
			}
			else if ( !Q_stricmp( token, "portal" ) )
			{
				stage->alphaGen = AGEN_PORTAL;
				token = COM_ParseExt( text, qfalse );
				if ( token[0] == 0 )
				{
					shader.portalRange = 256;
					ri.Printf( PRINT_WARNING, "WARNING: missing range parameter for alphaGen portal in shader '%s', defaulting to 256\n", shader.name );
				}
				else
				{
					shader.portalRange = atof( token );
				}
			}
			else
			{
				ri.Printf( PRINT_WARNING, "WARNING: unknown alphaGen parameter '%s' in shader '%s'\n", token, shader.name );
				continue;
			}
		}
		//
		// tcGen <function>
		//
		else if ( !Q_stricmp(token, "texgen") || !Q_stricmp( token, "tcGen" ) ) 
		{
			token = COM_ParseExt( text, qfalse );
			if ( token[0] == 0 )
			{
				ri.Printf( PRINT_WARNING, "WARNING: missing texgen parm in shader '%s'\n", shader.name );
				continue;
			}

			if ( !Q_stricmp( token, "environment" ) )
			{
				shader.needsNormal = qtrue;
				stage->bundle[cntBundle].tcGen = TCGEN_ENVIRONMENT_MAPPED;
            }
            else if (!Q_stricmp(token, "environmentmodel"))
            {
                shader.needsNormal = qtrue;
                stage->bundle[cntBundle].tcGen = TCGEN_ENVIRONMENT_MAPPED2;
            }
            else if (!Q_stricmp(token, "sunreflection"))
            {
                shader.needsNormal = qtrue;
                stage->bundle[cntBundle].tcGen = TCGEN_SUN_REFLECTION;
            }
			else if ( !Q_stricmp( token, "lightmap" ) )
			{
				stage->bundle[cntBundle].tcGen = TCGEN_LIGHTMAP;
			}
			else if ( !Q_stricmp( token, "texture" ) || !Q_stricmp( token, "base" ) )
			{
				stage->bundle[cntBundle].tcGen = TCGEN_TEXTURE;
			}
			else if ( !Q_stricmp( token, "vector" ) )
			{
				ParseVector( text, 3, stage->bundle[cntBundle].tcGenVectors[0] );
				ParseVector( text, 3, stage->bundle[cntBundle].tcGenVectors[1] );

				stage->bundle[cntBundle].tcGen = TCGEN_VECTOR;
			}
			else 
			{
				ri.Printf( PRINT_WARNING, "WARNING: unknown texgen parm in shader '%s'\n", shader.name );
			}
		}
		//
		// tcMod <type> <...>
		//
		else if ( !Q_stricmp( token, "tcMod" ) )
		{
			char buffer[1024] = "";

			while ( 1 )
			{
				token = COM_ParseExt( text, qfalse );
				if ( token[0] == 0 )
					break;
				strcat( buffer, token );
				strcat( buffer, " " );
			}

			ParseTexMod( buffer, stage, cntBundle );

			continue;
		}
		//
		// depthmask
		//
		else if ( !Q_stricmp( token, "depthwrite" ) || !Q_stricmp(token, "depthmask"))
		{
			depthMaskBits = GLS_DEPTHMASK_TRUE;
			depthMaskExplicit = qtrue;

			continue;
		}
		else if (!Q_stricmp(token, "noDepthTest"))
		{
			depthFuncBits = GLS_DEPTHTEST_DISABLE;
			continue;
        }
        else if (!Q_stricmp(token, "nextBundle"))
        {
			if (!qglActiveTextureARB) {
				ri.Printf(PRINT_ALL, "WARNING: " PRODUCT_NAME " requires a video card with multitexturing capability\n");
				return qfalse;
			}

			token = COM_ParseExt(text, qfalse);
			if (token[0] && !Q_stricmp(token, "add")) {
				stage->multitextureEnv = GL_ADD;
			} else {
				stage->multitextureEnv = GL_MODULATE;
			}

			cntBundle++;
			if (cntBundle > NUM_TEXTURE_BUNDLES) {
				ri.Printf(PRINT_WARNING, "WARNING: too many nextBundle commands in shader '%s'\n", shader.name);
				return qfalse;
			}
            continue;
        }
		else
		{
			ri.Printf( PRINT_WARNING, "WARNING: unknown parameter '%s' in shader '%s'\n", token, shader.name );
			return qfalse;
		}
	}

	//
	// if cgen isn't explicitly specified, use either identity or identitylighting
	//
	if ( stage->rgbGen == CGEN_BAD ) {
		if ( (!cntBundle && !stage->bundle[0].isLightmap) ||
			blendSrcBits == 0 ||
			blendSrcBits == GLS_SRCBLEND_ONE || 
			blendSrcBits == GLS_SRCBLEND_SRC_ALPHA ) {
			stage->rgbGen = CGEN_IDENTITY_LIGHTING;
		} else {
			stage->rgbGen = CGEN_IDENTITY;
		}
	}


	//
	// implicitly assume that a GL_ONE GL_ZERO blend mask disables blending
	//
	if ( ( blendSrcBits == GLS_SRCBLEND_ONE ) && 
		 ( blendDstBits == GLS_DSTBLEND_ZERO ) )
	{
		blendDstBits = blendSrcBits = 0;
		depthMaskBits = GLS_DEPTHMASK_TRUE;
	}

	if (depthFuncBits == GLS_DEPTHTEST_DISABLE) {
		depthMaskBits = 0;
	}

	// decide which agens we can skip
	if ( stage->alphaGen == CGEN_IDENTITY ) {
		if ( stage->rgbGen == CGEN_IDENTITY
			|| stage->rgbGen == CGEN_LIGHTING_GRID
			|| stage->rgbGen == CGEN_LIGHTING_SPHERICAL ) {
			stage->alphaGen = AGEN_SKIP;
		}
	}

	//
	// compute state bits
	//
	stage->stateBits = depthMaskBits | 
		               blendSrcBits | blendDstBits | 
					   atestBits | 
					   depthFuncBits |
					   depthTestBits |
					   fogBits;

	return qtrue;
}

/*
===============
ParseDeform

deformVertexes wave <spread> <waveform> <base> <amplitude> <phase> <frequency>
deformVertexes normal <frequency> <amplitude>
deformVertexes move <vector> <waveform> <base> <amplitude> <phase> <frequency>
deformVertexes bulge <bulgeWidth> <bulgeHeight> <bulgeSpeed>
deformVertexes projectionShadow
deformVertexes autoSprite
deformVertexes autoSprite2
deformVertexes text[0-7]
===============
*/
static void ParseDeform( char **text ) {
	char	*token;
	deformStage_t	*ds;

	token = COM_ParseExt( text, qfalse );
	if ( token[0] == 0 )
	{
		ri.Printf( PRINT_WARNING, "WARNING: missing deform parm in shader '%s'\n", shader.name );
		return;
	}

	if ( shader.numDeforms == MAX_SHADER_DEFORMS ) {
		ri.Printf( PRINT_WARNING, "WARNING: MAX_SHADER_DEFORMS in '%s'\n", shader.name );
		return;
	}

	ds = &shader.deforms[ shader.numDeforms ];
	shader.numDeforms++;

	if ( !Q_stricmp( token, "autosprite" ) ) {
		ds->deformation = DEFORM_AUTOSPRITE;
		return;
	}

	if ( !Q_stricmp( token, "autosprite2" ) ) {
		ds->deformation = DEFORM_AUTOSPRITE2;
		return;
	}

	if (!Q_stricmp(token, "lightglow")) {
		ds->deformation = DEFORM_LIGHTGLOW;
		return;
	}

	if ( !Q_stricmp( token, "bulge" ) )	{
		token = COM_ParseExt( text, qfalse );
		if ( token[0] == 0 )
		{
			ri.Printf( PRINT_WARNING, "WARNING: missing deformVertexes bulge parm in shader '%s'\n", shader.name );
			return;
		}
		ds->bulgeWidth = atof( token );

		token = COM_ParseExt( text, qfalse );
		if ( token[0] == 0 )
		{
			ri.Printf( PRINT_WARNING, "WARNING: missing deformVertexes bulge parm in shader '%s'\n", shader.name );
			return;
		}
		ds->bulgeHeight = atof( token );

		token = COM_ParseExt( text, qfalse );
		if ( token[0] == 0 )
		{
			ri.Printf( PRINT_WARNING, "WARNING: missing deformVertexes bulge parm in shader '%s'\n", shader.name );
			return;
		}
		ds->bulgeSpeed = atof( token );

		ds->deformation = DEFORM_BULGE;
		return;
	}

	if ( !Q_stricmp( token, "wave" ) )
	{
		token = COM_ParseExt( text, qfalse );
		if ( token[0] == 0 )
		{
			ri.Printf( PRINT_WARNING, "WARNING: missing deformVertexes parm in shader '%s'\n", shader.name );
			return;
		}

		if ( atof( token ) != 0 )
		{
			ds->deformationSpread = 1.0f / atof( token );
		}
		else
		{
			ds->deformationSpread = 100.0f;
			ri.Printf( PRINT_WARNING, "WARNING: illegal div value of 0 in deformVertexes command for shader '%s'\n", shader.name );
		}

		ParseWaveForm( text, &ds->deformationWave );
		ds->deformation = DEFORM_WAVE;
		return;
	}

	if ( !Q_stricmp( token, "normal" ) )
	{
		token = COM_ParseExt( text, qfalse );
		if ( token[0] == 0 )
		{
			ri.Printf( PRINT_WARNING, "WARNING: missing deformVertexes parm in shader '%s'\n", shader.name );
			return;
		}
		ds->deformationWave.amplitude = atof( token );

		token = COM_ParseExt( text, qfalse );
		if ( token[0] == 0 )
		{
			ri.Printf( PRINT_WARNING, "WARNING: missing deformVertexes parm in shader '%s'\n", shader.name );
			return;
		}
		ds->deformationWave.frequency = atof( token );

		ds->deformation = DEFORM_NORMALS;
		return;
	}

	if ( !Q_stricmp( token, "move" ) ) {
		int		i;

		for ( i = 0 ; i < 3 ; i++ ) {
			token = COM_ParseExt( text, qfalse );
			if ( token[0] == 0 ) {
				ri.Printf( PRINT_WARNING, "WARNING: missing deformVertexes parm in shader '%s'\n", shader.name );
				return;
			}
			ds->moveVector[i] = atof( token );
		}

		ParseWaveForm( text, &ds->deformationWave );
		ds->deformation = DEFORM_MOVE;
		return;
	}

	if (!Q_stricmp(token, "flap")) {
		texDirection_t coordDirection;

		token = COM_ParseExt(text, qfalse);
		if (token[0] == 's') {
			coordDirection = USE_S_COORDS;
		}
		else if (token[0] == 't') {
			coordDirection = USE_T_COORDS;
		}
		else {
			ri.Printf(PRINT_WARNING, "WARNING: deformVertexes flap requires 's' or 't' in shader '%s'\n", shader.name);
			return;
		}

		token = COM_ParseExt(text, qfalse);
		if (token[0] == 0) {
			ri.Printf(PRINT_WARNING, "WARNING: missing deformVertexes flap parm in shader '%s'\n", shader.name);
			return;
		}

		if (!atof(token)) {
			ds->deformationSpread = 100.0;
			ri.Printf(PRINT_WARNING, "WARNING: illegal div value of 0 in deformVertexes command for shader '%s'\n", shader.name);
		}
		else {
			ds->deformationSpread = 1.0 / atof(token);
		}

		ParseWaveForm(text, &ds->deformationWave);

		if (coordDirection == USE_T_COORDS) {
			ds->deformation = DEFORM_FLAP_T;
		}
		else {
			ds->deformation = DEFORM_FLAP_S;
		}

		shader.needsNormal = qtrue;

		token = COM_ParseExt(text, qfalse);
		if (token[0] == 0)
		{
			ds->bulgeWidth = 0.0;
			ds->bulgeHeight = 1.0;
			return;
		}
		ds->bulgeWidth = atof(token);

		token = COM_ParseExt(text, qfalse);
		if (token[0] == 0)
		{
			ri.Printf(PRINT_WARNING, "WARNING: missing deformVertexes parm 'max' in shader '%s'\n\n", shader.name);
			return;
		}
		ds->bulgeHeight = atof(token);
		return;
	}

	ri.Printf( PRINT_WARNING, "WARNING: unknown deformVertexes subtype '%s' found in shader '%s'\n", token, shader.name );
}


/*
===============
ParseSkyParms

skyParms <outerbox> <cloudheight> <innerbox>
===============
*/
static void ParseSkyParms( char **text ) {
	char		*token;
	static char	*suf[6] = {"rt", "bk", "lf", "ft", "up", "dn"};
	char		pathname[MAX_QPATH];
	int			i;

	// outerbox
	token = COM_ParseExt( text, qfalse );
	if ( token[0] == 0 ) {
		ri.Printf( PRINT_WARNING, "WARNING: 'skyParms' missing parameter in shader '%s'\n", shader.name );
		return;
	}
	if ( strcmp( token, "-" ) ) {
		for (i=0 ; i<6 ; i++) {
			Com_sprintf( pathname, sizeof(pathname), "%s_%s.tga"
				, token, suf[i] );
			if (!r_forceClampToEdge->integer) {
				shader.sky.outerbox[i] = R_FindImageFile((char*)pathname, qtrue, qtrue, shader_force32bit, GL_CLAMP, GL_CLAMP);
			}
			else {
				shader.sky.outerbox[i] = R_FindImageFile((char*)pathname, qtrue, qtrue, shader_force32bit, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
			}
			if ( !shader.sky.outerbox[i] ) {
				shader.sky.outerbox[i] = tr.defaultImage;
			}
		}
	}

	// cloudheight
	token = COM_ParseExt( text, qfalse );
	if ( token[0] == 0 ) {
		ri.Printf( PRINT_WARNING, "WARNING: 'skyParms' missing parameter in shader '%s'\n", shader.name );
		return;
	}
	shader.sky.cloudHeight = atof( token );
	if ( !shader.sky.cloudHeight ) {
		shader.sky.cloudHeight = 512;
	}
	R_InitSkyTexCoords( shader.sky.cloudHeight );


	// innerbox
	token = COM_ParseExt( text, qfalse );
	if ( token[0] == 0 ) {
		ri.Printf( PRINT_WARNING, "WARNING: 'skyParms' missing parameter in shader '%s'\n", shader.name );
		return;
	}
	if ( strcmp( token, "-" ) ) {
		for (i=0 ; i<6 ; i++) {
			Com_sprintf( pathname, sizeof(pathname), "%s_%s.tga"
				, token, suf[i] );
			shader.sky.outerbox[i] = R_FindImageFile( ( char * ) pathname, qtrue, qtrue, shader_force32bit, GL_REPEAT, GL_REPEAT );
			if ( !shader.sky.innerbox[i] ) {
				shader.sky.innerbox[i] = tr.defaultImage;
			}
		}
	}

	shader.isSky = qtrue;
}


/*
=================
ParseSort
=================
*/
void ParseSort( char **text ) {
	char	*token;

	token = COM_ParseExt( text, qfalse );
	if ( token[0] == 0 ) {
		ri.Printf( PRINT_WARNING, "WARNING: missing sort parameter in shader '%s'\n", shader.name );
		return;
	}

	if ( !Q_stricmp( token, "portal" ) ) {
		shader.sort = SS_PORTAL;
	} else if ( !Q_stricmp( token, "sky" ) ) {
		shader.sort = SS_ENVIRONMENT;
	} else if ( !Q_stricmp( token, "opaque" ) ) {
		shader.sort = SS_OPAQUE;
	}else if ( !Q_stricmp( token, "decal" ) ) {
		shader.sort = SS_DECAL;
	} else if ( !Q_stricmp( token, "seeThrough" ) ) {
		shader.sort = SS_SEE_THROUGH;
	} else if ( !Q_stricmp( token, "banner" ) ) {
		shader.sort = SS_BANNER;
	} else if ( !Q_stricmp( token, "additive" ) ) {
		shader.sort = SS_BLEND1;
	} else if ( !Q_stricmp( token, "nearest" ) ) {
		shader.sort = SS_NEAREST;
	} else if ( !Q_stricmp( token, "underwater" ) ) {
		shader.sort = SS_UNDERWATER;
	} else {
		shader.sort = atof( token );
	}
}



// this table is also present in q3map

typedef struct {
	char	*name;
	int		clearSolid, surfaceFlags, contents;
} infoParm_t;

infoParm_t	infoParms[] = {
	// server relevant contents
	{"water",		1,	0,	CONTENTS_WATER },
	{"slime",		1,	0,	CONTENTS_SLIME },		// mildly damaging
	{"lava",		1,	0,	CONTENTS_LAVA },		// very damaging
	{"playerclip",	1,	0,	CONTENTS_PLAYERCLIP },
	{"monsterclip",	1,	0,	CONTENTS_MONSTERCLIP },
	{"fence",		1,	0,	CONTENTS_FENCE },
	{"weaponclip",	1,	0,	CONTENTS_WEAPONCLIP },
	{"vehicleclip",	1,	0,	CONTENTS_VEHICLECLIP },
	{"nodrop",		1,	0,	CONTENTS_NODROP },		// don't drop items or leave bodies (death fog, lava, etc)
	{"nonsolid",	1,	SURF_NONSOLID,	0 },		// clears the solid flag

	// utility relevant attributes
	{"origin",		1,	0,	CONTENTS_ORIGIN },		// center of rotating brushes
	{"trans",		0,	0,	CONTENTS_TRANSLUCENT },	// don't eat contained surfaces
	{"detail",		0,	0,	CONTENTS_DETAIL },		// don't include in structural bsp
	{"structural",	0,	0,	CONTENTS_STRUCTURAL },	// force into structural bsp even if trnas
	{"areaportal",	1,	0,	CONTENTS_AREAPORTAL },	// divides areas
	{"fog",			1,	0,	CONTENTS_FOG},			// carves surfaces entering
	{"sky",			0,	SURF_SKY,			0 },	// emit light from an environment map
	{"alphashadow",	1,	SURF_ALPHASHADOW,	0 },	// carves surfaces entering

	// server attributes
	{"slick",		0,	SURF_SLICK,		0 },
	{"noimpact",	0,	SURF_NOIMPACT,	0 },		// don't make impact explosions or marks
	{"nomarks",		0,	SURF_NOMARKS,	0 },		// don't make impact marks, but still explode
	{"ladder",		0,	SURF_LADDER,	0 },
	{"nodamage",	0,	SURF_NODAMAGE,	0 },
	{"nosteps",		0,	SURF_NOSTEPS,	0 },
	{"paper",		0,	SURF_PAPER,		0 },
	{"wood",		0,	SURF_WOOD,		0 },
	{"metal",		0,	SURF_METAL,		0 },
	{"rock",		0,	SURF_ROCK,		0 },
	{"dirt",		0,	SURF_DIRT,		0 },
	{"grill",		0,	SURF_GRILL,		0 },
	{"grass",		0,	SURF_GRASS,		0 },
	{"mud",			0,	SURF_MUD,		0 },
	{"puddle",		0,	SURF_PUDDLE,	0 },
	{"glass",		0,	SURF_GLASS,		0 },
	{"gravel",		0,	SURF_GRAVEL,	0 },
	{"sand",		0,	SURF_SAND,		0 },
	{"foliage",		0,	SURF_FOLIAGE,	0 },
	{"snow",		0,	SURF_SNOW,		0 },
	{"carpet",		0,	SURF_CARPET,	0 },

	// drawsurf attributes
	{"nodraw",		0,	SURF_NODRAW,		0 },	// don't generate a drawsurface (or a lightmap)
	{"castshadow",	0,	SURF_CASTSHADOW,	0 },	// sample lighting at vertexes
	{"nolightmap",	0,	SURF_NOLIGHTMAP,	0 },	// don't generate a lightmap
	{"nodlight",	0,	SURF_NODLIGHT,		0 },	// don't ever add dynamic lights
	{"hint",		0,	SURF_HINT,		0}			// leave a dust trail when walking on this surface
};


/*
===============
ParseSurfaceParm

surfaceparm <name>
===============
*/
static void ParseSurfaceParm( char **text ) {
	char	*token;
	int		numInfoParms = sizeof(infoParms) / sizeof(infoParms[0]);
	int		i;

	token = COM_ParseExt( text, qfalse );
	for ( i = 0 ; i < numInfoParms ; i++ ) {
		if ( !Q_stricmp( token, infoParms[i].name ) ) {
			shader.surfaceFlags |= infoParms[i].surfaceFlags;
			shader.contentFlags |= infoParms[i].contents;
#if 0
			if ( infoParms[i].clearSolid ) {
				si->contents &= ~CONTENTS_SOLID;
			}
#endif
			break;
		}
	}
}

/*
=================
ParseShader

The current text pointer is at the explicit text definition of the
shader.  Parse it into the global shader variable.  Later functions
will optimize it.
=================
*/
static qboolean ParseShader( char **text )
{
	char *token;
	int s;

	s = 0;

	token = COM_ParseExt( text, qtrue );
	if ( token[0] != '{' )
	{
		ri.Printf( PRINT_WARNING, "WARNING: expecting '{', found '%s' instead in shader '%s'\n", token, shader.name );
		return qfalse;
	}

	while ( 1 )
	{
		token = COM_ParseExt( text, qtrue );
		if ( !token[0] )
		{
			ri.Printf( PRINT_WARNING, "WARNING: no concluding '}' in shader %s\n", shader.name );
			return qfalse;
		}

		// end of shader definition
		if ( token[0] == '}' )
		{
			break;
		}
		// stage definition
		else if ( token[0] == '{' )
		{
			if ( !ParseStage( &unfoggedStages[s], text ) )
			{
				return qfalse;
			}
			unfoggedStages[s].active = qtrue;
			s++;
			continue;
		}
		// skip stuff that only the QuakeEdRadient needs
		else if ( !Q_stricmpn( token, "qer", 3 ) ) {
			SkipRestOfLine( text );
			continue;
		}
		// sun parms
		else if ( !Q_stricmp( token, "q3map_sun" ) ) {
			float	a, b;

			token = COM_ParseExt( text, qfalse );
			tr.sunLight[0] = atof( token );
			token = COM_ParseExt( text, qfalse );
			tr.sunLight[1] = atof( token );
			token = COM_ParseExt( text, qfalse );
			tr.sunLight[2] = atof( token );
			
			VectorNormalize( tr.sunLight );

			token = COM_ParseExt( text, qfalse );
			a = atof( token );
			VectorScale( tr.sunLight, a, tr.sunLight);

			token = COM_ParseExt( text, qfalse );
			a = atof( token );
			a = a / 180 * M_PI;

			token = COM_ParseExt( text, qfalse );
			b = atof( token );
			b = b / 180 * M_PI;

			tr.sunDirection[0] = cos( a ) * cos( b );
			tr.sunDirection[1] = sin( a ) * cos( b );
			tr.sunDirection[2] = sin( b );
		}
		else if ( !Q_stricmp( token, "deformVertexes" ) ) {
			ParseDeform( text );
			continue;
		}
		else if ( !Q_stricmp( token, "tesssize" ) ) {
			SkipRestOfLine( text );
			continue;
		}
		else if ( !Q_stricmp( token, "clampTime" ) ) {
			token = COM_ParseExt( text, qfalse );
      if (token[0]) {
        shader.clampTime = atof(token);
      }
    }
		// skip stuff that only the q3map needs
		else if ( !Q_stricmpn( token, "q3map", 5 ) ) {
			SkipRestOfLine( text );
			continue;
		}
		// skip stuff that only q3map or the server needs
		else if ( !Q_stricmp( token, "surfaceParm" ) ) {
			ParseSurfaceParm( text );
			continue;
		}
		// no mip maps
		else if ( !Q_stricmp( token, "nomipmaps" ) )
		{
			shader_noMipMaps = qtrue;
			continue;
		}
		// no picmip adjustment
		else if ( !Q_stricmp( token, "nopicmip" ) )
		{
			shader_noPicMip = qtrue;
			continue;
		}
		// no picmip adjustment
		else if (!Q_stricmp(token, "force32bit"))
		{
			shader_force32bit = qtrue;
			continue;
		}
		// polygonOffset
		else if ( !Q_stricmp( token, "polygonOffset" ) )
		{
			shader.polygonOffset = qtrue;
			continue;
		}
		// entityMergable, allowing sprite surfaces from multiple entities
		// to be merged into one batch.  This is a savings for smoke
		// puffs and blood, but can't be used for anything where the
		// shader calcs (not the surface function) reference the entity color or scroll
		else if ( !Q_stricmp( token, "entityMergable" ) )
		{
			shader.entityMergable = qtrue;
			continue;
			}
		else if (!Q_stricmp(token, "noMerge"))
		{
			shader.flags |= 1;
			continue;
		}
		// portal
		else if ( !Q_stricmp(token, "portal") )
		{
			shader.sort = SS_PORTAL;
			continue;
		}
		// skyparms <cloudheight> <outerbox> <innerbox>
		else if ( !Q_stricmp( token, "skyparms" ) )
		{
			ParseSkyParms( text );
			continue;
		}
		// portal
		else if (!Q_stricmp(token, "portalsky"))
		{
			shader.sort = SS_PORTALSKY;
			shader.isPortalSky = qtrue;
			continue;
		}
		// light <value> determines flaring in q3map, not needed here
		else if ( !Q_stricmp(token, "light") ) 
		{
			token = COM_ParseExt( text, qfalse );
			continue;
		}
		else if (!Q_stricmp(token, "spritegen"))
		{
			token = COM_ParseExt(text, qfalse);
			if (token[0] == 0) {
				ri.Printf(PRINT_WARNING, "WARNING: missing spritegen parm in shader '%s'\n", shader.name);
				continue;
			}

			if (!Q_stricmp(token, "parallel")) {
				shader.sprite.type = SPRITE_PARALLEL;
			} else if (!Q_stricmp(token, "parallel_oriented")) {
				shader.sprite.type = SPRITE_PARALLEL_ORIENTED;
			} else if (!Q_stricmp(token, "parallel_upright")) {
				shader.sprite.type = SPRITE_PARALLEL_UPRIGHT;
			} else if (!Q_stricmp(token, "oriented")) {
				shader.sprite.type = SPRITE_ORIENTED;
			}

			shader.sprite.scale = 1.0;
			continue;
		}
		else if (!Q_stricmp(token, "spritescale"))
		{
			token = COM_ParseExt(text, qfalse);
			if (token[0] == 0) {
				ri.Printf(PRINT_WARNING, "WARNING: missing spritescale parm in shader '%s'\n", shader.name);
				continue;
			}

			shader.sprite.scale = atof(token);
			continue;
		}
		// cull <face>
		else if ( !Q_stricmp( token, "cull") ) 
		{
			token = COM_ParseExt( text, qfalse );
			if ( token[0] == 0 )
			{
				ri.Printf( PRINT_WARNING, "WARNING: missing cull parms in shader '%s'\n", shader.name );
				continue;
			}

			if ( !Q_stricmp( token, "none" ) || !Q_stricmp( token, "twosided" ) || !Q_stricmp( token, "disable" ) )
			{
				shader.cullType = CT_TWO_SIDED;
			}
			else if ( !Q_stricmp( token, "back" ) || !Q_stricmp( token, "backside" ) || !Q_stricmp( token, "backsided" ) )
			{
				shader.cullType = CT_BACK_SIDED;
			}
			else
			{
				ri.Printf( PRINT_WARNING, "WARNING: invalid cull parm '%s' in shader '%s'\n", token, shader.name );
			}
			continue;
		}
		// sort
		else if ( !Q_stricmp( token, "sort" ) )
		{
			ParseSort( text );
			continue;
		}
		else
		{
			ri.Printf( PRINT_WARNING, "WARNING: unknown general shader parameter '%s' in '%s'\n", token, shader.name );
			return qfalse;
		}
	}

	//
	// ignore shaders that don't have any unfoggedStages, unless it is a sky or fog
	//
	if ( s == 0 && !shader.isSky && !(shader.contentFlags & CONTENTS_FOG ) ) {
		return qfalse;
	}

	if (!(shader.flags & 2)) {
		shader.flags &= ~1;
	}

	if ((shader.contentFlags & CONTENTS_FENCE) != 0 && shader.cullType == CT_TWO_SIDED) {
		shader.cullType = CT_FRONT_SIDED;
	}

	shader.explicitlyDefined = qtrue;

	return qtrue;
}

/*
========================================================================================

SHADER OPTIMIZATION AND FOGGING

========================================================================================
*/

/*
===================
ComputeStageIteratorFunc

See if we can use on of the simple fastpath stage functions,
otherwise set to the generic stage function
===================
*/
static void ComputeStageIteratorFunc( void )
{
	shader.optimalStageIteratorFunc = RB_StageIteratorGeneric;

	//
	// see if this should go into the sky path
	//
	if ( shader.isSky )
	{
		shader.optimalStageIteratorFunc = RB_StageIteratorSky;
		goto done;
	}

	if ( r_ignoreFastPath->integer )
	{
		return;
	}

	//
	// see if this can go into the vertex lit fast path
	//
	if ( shader.numUnfoggedPasses == 1 )
	{
		if ( unfoggedStages[0].rgbGen == CGEN_LIGHTING_GRID || unfoggedStages[0].rgbGen == CGEN_LIGHTING_SPHERICAL )
		{
			if ( unfoggedStages[0].alphaGen == AGEN_IDENTITY || unfoggedStages[0].alphaGen == AGEN_SKIP)
			{
				if ( unfoggedStages[0].bundle[0].tcGen == TCGEN_TEXTURE && unfoggedStages[0].bundle[1].tcGen == TCGEN_LIGHTMAP)
				{
					if ( !shader.polygonOffset )
					{
						if ( !unfoggedStages[0].multitextureEnv )
						{
							if ( !shader.numDeforms )
							{
								shader.optimalStageIteratorFunc = RB_StageIteratorVertexLitTextureUnfogged;
								goto done;
							}
						}
					}
				}
			}
		}
	}

	//
	// see if this can go into an optimized LM, multitextured path
	//
	if ( shader.numUnfoggedPasses == 1 )
	{
		if ( ( unfoggedStages[0].rgbGen == CGEN_IDENTITY ) && ( unfoggedStages[0].alphaGen == AGEN_IDENTITY || unfoggedStages[0].alphaGen == AGEN_SKIP ) )
		{
			if ( unfoggedStages[0].bundle[0].tcGen == TCGEN_TEXTURE && 
				unfoggedStages[0].bundle[1].tcGen == TCGEN_LIGHTMAP )
			{
				if ( !shader.polygonOffset )
				{
					if ( !shader.numDeforms )
					{
						if (unfoggedStages[0].multitextureEnv )
						{
							shader.optimalStageIteratorFunc = RB_StageIteratorLightmappedMultitextureUnfogged;
							goto done;
						}
					}
				}
			}
		}
	}

done:
	return;
}

typedef struct {
	int		blendA;
	int		blendB;

	int		multitextureEnv;
	int		multitextureBlend;
} collapse_t;

static collapse_t	collapse[] = {
	{ 0, GLS_DSTBLEND_SRC_COLOR | GLS_SRCBLEND_ZERO,	
		GL_MODULATE, 0 },

	{ 0, GLS_DSTBLEND_ZERO | GLS_SRCBLEND_DST_COLOR,
		GL_MODULATE, 0 },

	{ GLS_DSTBLEND_ZERO | GLS_SRCBLEND_DST_COLOR, GLS_DSTBLEND_ZERO | GLS_SRCBLEND_DST_COLOR,
		GL_MODULATE, GLS_DSTBLEND_ZERO | GLS_SRCBLEND_DST_COLOR },

	{ GLS_DSTBLEND_SRC_COLOR | GLS_SRCBLEND_ZERO, GLS_DSTBLEND_ZERO | GLS_SRCBLEND_DST_COLOR,
		GL_MODULATE, GLS_DSTBLEND_ZERO | GLS_SRCBLEND_DST_COLOR },

	{ GLS_DSTBLEND_ZERO | GLS_SRCBLEND_DST_COLOR, GLS_DSTBLEND_SRC_COLOR | GLS_SRCBLEND_ZERO,
		GL_MODULATE, GLS_DSTBLEND_ZERO | GLS_SRCBLEND_DST_COLOR },

	{ GLS_DSTBLEND_SRC_COLOR | GLS_SRCBLEND_ZERO, GLS_DSTBLEND_SRC_COLOR | GLS_SRCBLEND_ZERO,
		GL_MODULATE, GLS_DSTBLEND_ZERO | GLS_SRCBLEND_DST_COLOR },

	{ 0, GLS_DSTBLEND_ONE | GLS_SRCBLEND_ONE,
		GL_ADD, 0 },

	{ GLS_DSTBLEND_ONE | GLS_SRCBLEND_ONE, GLS_DSTBLEND_ONE | GLS_SRCBLEND_ONE,
		GL_ADD, GLS_DSTBLEND_ONE | GLS_SRCBLEND_ONE },
#if 0
	{ 0, GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA | GLS_SRCBLEND_SRC_ALPHA,
		GL_DECAL, 0 },
#endif
	{ -1 }
};

/*
================
CollapseMultitexture

Attempt to combine two unfoggedStages into a single multitexture stage
FIXME: I think modulated add + modulated add collapses incorrectly
=================
*/
static void CollapseMultitexture(int *stagecounter) {
	int iUseCollapse;
	int stagenum;
	int abits, bbits;
	shaderStage_t* stage;
	textureBundle_t tmpBundle;

	for (stagenum = 0; stagenum < *stagecounter - 1; stagenum++) {
		stage = &unfoggedStages[stagenum];

		// make sure both unfoggedStages are active
		if (!stage->active || !unfoggedStages[stagenum + 1].active) {
			continue;
		}

		if (stage->multitextureEnv) {
			continue;
		}

		// on voodoo2, don't combine different tmus
		if (glConfig.driverType == GLDRV_VOODOO) {
			if (stage->bundle[0].image[0]->TMU ==
				unfoggedStages[stagenum + 1].bundle[0].image[0]->TMU) {
				continue;
			}
		}

		abits = stage->stateBits;
		bbits = unfoggedStages[stagenum + 1].stateBits;

		// make sure that both unfoggedStages have identical state other than blend modes
		if ((abits & ~(GLS_DSTBLEND_BITS | GLS_SRCBLEND_BITS | GLS_DEPTHMASK_TRUE)) !=
			(bbits & ~(GLS_DSTBLEND_BITS | GLS_SRCBLEND_BITS | GLS_DEPTHMASK_TRUE))) {
			continue;
		}

		abits &= (GLS_DSTBLEND_BITS | GLS_SRCBLEND_BITS);
		bbits &= (GLS_DSTBLEND_BITS | GLS_SRCBLEND_BITS);

		// search for a valid multitexture blend function
		for (iUseCollapse = 0; collapse[iUseCollapse].blendA != -1; iUseCollapse++) {
			if (abits == collapse[iUseCollapse].blendA
				&& bbits == collapse[iUseCollapse].blendB) {
				break;
			}
		}

		// nothing found
		if (collapse[iUseCollapse].blendA == -1) {
			continue;
		}

		// GL_ADD is a separate extension
		if (collapse[iUseCollapse].multitextureEnv == GL_ADD && !glConfig.textureEnvAddAvailable) {
			continue;
		}

		// make sure waveforms have identical parameters
		if ((stage->rgbGen != unfoggedStages[stagenum + 1].rgbGen) ||
			(stage->alphaGen != unfoggedStages[stagenum + 1].alphaGen)) {
			continue;
		}

		// an add collapse can only have identity colors
		if (collapse[iUseCollapse].multitextureEnv == GL_ADD && stage->rgbGen != CGEN_IDENTITY) {
			continue;
		}

		if (stage->rgbGen == CGEN_WAVEFORM)
		{
			if (memcmp(&stage->rgbWave,
				&unfoggedStages[stagenum + 1].rgbWave,
				sizeof(stage->rgbWave)))
			{
				continue;
			}
		}
		if (stage->alphaGen == CGEN_WAVEFORM)
		{
			if (memcmp(&stage->alphaWave,
				&unfoggedStages[stagenum + 1].alphaWave,
				sizeof(stage->alphaWave)))
			{
				continue;
			}
		}


		// make sure that lightmaps are in bundle 1 for 3dfx
		if (stage->bundle[0].isLightmap)
		{
			tmpBundle = stage->bundle[0];
			stage->bundle[0] = unfoggedStages[stagenum + 1].bundle[0];
			stage->bundle[1] = tmpBundle;
		}
		else
		{
			stage->bundle[1] = unfoggedStages[stagenum + 1].bundle[0];
		}

		// set the new blend state bits
		stage->multitextureEnv = collapse[iUseCollapse].multitextureEnv;
		stage->stateBits &= ~(GLS_DSTBLEND_BITS | GLS_SRCBLEND_BITS);
		stage->stateBits |= collapse[iUseCollapse].multitextureBlend;

		//
		// move down subsequent shaders
		//
		if (stagenum + 2 < MAX_SHADER_STAGES) {
			memmove(&unfoggedStages[stagenum + 1], &unfoggedStages[stagenum + 2], sizeof(unfoggedStages[0]) * (MAX_SHADER_STAGES - 2 - stagenum));
		}

		Com_Memset(&unfoggedStages[MAX_SHADER_STAGES - 1], 0, sizeof(unfoggedStages[0]));
		(*stagecounter)--;
	}
}

/*
=============

FixRenderCommandList
https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=493
Arnout: this is a nasty issue. Shaders can be registered after drawsurfaces are generated
but before the frame is rendered. This will, for the duration of one frame, cause drawsurfaces
to be rendered with bad shaders. To fix this, need to go through all render commands and fix
sortedIndex.
==============
*/
static void FixRenderCommandList( int newShader ) {
	renderCommandList_t	*cmdList = &backEndData[tr.smpFrame]->commands;

	if( cmdList ) {
		const void *curCmd = cmdList->cmds;

		while ( 1 ) {
			switch ( *(const int *)curCmd ) {
			case RC_SET_COLOR:
				{
				const setColorCommand_t *sc_cmd = (const setColorCommand_t *)curCmd;
				curCmd = (const void *)(sc_cmd + 1);
				break;
				}
			case RC_STRETCH_PIC:
				{
				const stretchPicCommand_t *sp_cmd = (const stretchPicCommand_t *)curCmd;
				curCmd = (const void *)(sp_cmd + 1);
				break;
				}
			case RC_DRAW_SURFS:
				{
				int i;
				drawSurf_t	*drawSurf;
				shader_t	*shader;
				int			fogNum;
				int			entityNum;
				int			dlightMap;
				int			sortedIndex;
				const drawSurfsCommand_t *ds_cmd =  (const drawSurfsCommand_t *)curCmd;

				for( i = 0, drawSurf = ds_cmd->drawSurfs; i < ds_cmd->numDrawSurfs; i++, drawSurf++ ) {
					R_DecomposeSort( drawSurf->sort, &entityNum, &shader, &fogNum, &dlightMap );
                    sortedIndex = (( drawSurf->sort >> QSORT_SHADERNUM_SHIFT ) & (MAX_SHADERS-1));
					if( sortedIndex >= newShader ) {
						sortedIndex++;
						drawSurf->sort = (sortedIndex << QSORT_SHADERNUM_SHIFT) | entityNum | ( fogNum << QSORT_FOGNUM_SHIFT ) | (int)dlightMap;
					}
				}
				curCmd = (const void *)(ds_cmd + 1);
				break;
				}
			case RC_DRAW_BUFFER:
				{
				const drawBufferCommand_t *db_cmd = (const drawBufferCommand_t *)curCmd;
				curCmd = (const void *)(db_cmd + 1);
				break;
				}
			case RC_SWAP_BUFFERS:
				{
				const swapBuffersCommand_t *sb_cmd = (const swapBuffersCommand_t *)curCmd;
				curCmd = (const void *)(sb_cmd + 1);
				break;
				}
			case RC_END_OF_LIST:
			default:
				return;
			}
		}
	}
}

/*
==============
SortNewShader

Positions the most recently created shader in the tr.sortedShaders[]
array so that the shader->sort key is sorted reletive to the other
shaders.

Sets shader->sortedIndex
==============
*/
static void SortNewShader( void ) {
	int		i;
	float	sort;
	shader_t	*newShader;

	newShader = tr.shaders[ tr.numShaders - 1 ];
	sort = newShader->sort;

	for ( i = tr.numShaders - 2 ; i >= 0 ; i-- ) {
		if ( tr.sortedShaders[ i ]->sort <= sort ) {
			break;
		}
		tr.sortedShaders[i+1] = tr.sortedShaders[i];
		tr.sortedShaders[i+1]->sortedIndex++;
	}

	// Arnout: fix rendercommandlist
	// https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=493
	FixRenderCommandList( i+1 );

	newShader->sortedIndex = i+1;
	tr.sortedShaders[i+1] = newShader;
}


/*
====================
GeneratePermanentShader
====================
*/
static shader_t *GeneratePermanentShader( void ) {
	shader_t	*newShader;
	int			i, b;
	int			size;

	if ( tr.numShaders == MAX_SHADERS ) {
		ri.Printf( PRINT_WARNING, "WARNING: GeneratePermanentShader - MAX_SHADERS hit\n");
		return tr.defaultShader;
	}

	newShader = ri.Hunk_Alloc( sizeof( shader_t ), h_dontcare);

	*newShader = shader;
	newShader->next = currentShader->shader;
	currentShader->shader = newShader;

	tr.shaders[ tr.numShaders ] = newShader;
	newShader->index = tr.numShaders;
	
	tr.sortedShaders[ tr.numShaders ] = newShader;
	newShader->sortedIndex = tr.numShaders;

	tr.numShaders++;

	for ( i = 0 ; i < newShader->numUnfoggedPasses ; i++ ) {
		if ( !unfoggedStages[i].active ) {
			break;
		}
		newShader->unfoggedStages[i] = ri.Hunk_Alloc( sizeof( unfoggedStages[i] ), h_dontcare );
		*newShader->unfoggedStages[i] = unfoggedStages[i];

		for ( b = 0 ; b < NUM_TEXTURE_BUNDLES ; b++ ) {
			size = newShader->unfoggedStages[i]->bundle[b].numTexMods * sizeof( texModInfo_t );
			if (size) {
				newShader->unfoggedStages[i]->bundle[b].texMods = ri.Hunk_Alloc(size, h_dontcare);
				Com_Memcpy(newShader->unfoggedStages[i]->bundle[b].texMods, unfoggedStages[i].bundle[b].texMods, size);
			}
		}
	}

	SortNewShader();

	currentShader = NULL;

	return newShader;
}

/*
=================
VertexLightingCollapse

If vertex lighting is enabled, only render a single
pass, trying to guess which is the correct one to best aproximate
what it is supposed to look like.
=================
*/
static void VertexLightingCollapse( void ) {
	int		stage;
	shaderStage_t	*bestStage;
	int		bestImageRank;
	int		rank;

	// if we aren't opaque, just use the first pass
	if ( shader.sort == SS_OPAQUE ) {

		// pick the best texture for the single pass
		bestStage = &unfoggedStages[0];
		bestImageRank = -999999;

		for ( stage = 0; stage < MAX_SHADER_STAGES; stage++ ) {
			shaderStage_t *pStage = &unfoggedStages[stage];

			if ( !pStage->active ) {
				break;
			}
			rank = 0;

			if ( pStage->bundle[0].isLightmap ) {
				rank -= 100;
			}
			if ( pStage->bundle[0].tcGen != TCGEN_TEXTURE ) {
				rank -= 5;
			}
			if ( pStage->bundle[0].numTexMods ) {
				rank -= 5;
			}
			if ( pStage->rgbGen != CGEN_IDENTITY && pStage->rgbGen != CGEN_IDENTITY_LIGHTING ) {
				rank -= 3;
			}

			if ( rank > bestImageRank  ) {
				bestImageRank = rank;
				bestStage = pStage;
			}
		}

		unfoggedStages[0].bundle[0] = bestStage->bundle[0];
		unfoggedStages[0].stateBits &= ~( GLS_DSTBLEND_BITS | GLS_SRCBLEND_BITS );
		unfoggedStages[0].stateBits |= GLS_DEPTHMASK_TRUE;
		if ( shader.lightmapIndex == LIGHTMAP_NONE ) {
			unfoggedStages[0].rgbGen = CGEN_LIGHTING_GRID;
		} else {
			unfoggedStages[0].rgbGen = CGEN_EXACT_VERTEX;
		}
		unfoggedStages[0].alphaGen = AGEN_SKIP;		
	} else {
		// don't use a lightmap (tesla coils)
		if ( unfoggedStages[0].bundle[0].isLightmap ) {
			unfoggedStages[0] = unfoggedStages[1];
		}

		// if we were in a cross-fade cgen, hack it to normal
		if ( unfoggedStages[0].rgbGen == CGEN_ONE_MINUS_ENTITY || unfoggedStages[1].rgbGen == CGEN_ONE_MINUS_ENTITY ) {
			unfoggedStages[0].rgbGen = CGEN_IDENTITY_LIGHTING;
		}
		if ( ( unfoggedStages[0].rgbGen == CGEN_WAVEFORM && unfoggedStages[0].rgbWave.func == GF_SAWTOOTH )
			&& ( unfoggedStages[1].rgbGen == CGEN_WAVEFORM && unfoggedStages[1].rgbWave.func == GF_INVERSE_SAWTOOTH ) ) {
			unfoggedStages[0].rgbGen = CGEN_IDENTITY_LIGHTING;
		}
		if ( ( unfoggedStages[0].rgbGen == CGEN_WAVEFORM && unfoggedStages[0].rgbWave.func == GF_INVERSE_SAWTOOTH )
			&& ( unfoggedStages[1].rgbGen == CGEN_WAVEFORM && unfoggedStages[1].rgbWave.func == GF_SAWTOOTH ) ) {
			unfoggedStages[0].rgbGen = CGEN_IDENTITY_LIGHTING;
		}
	}

	for ( stage = 1; stage < MAX_SHADER_STAGES; stage++ ) {
		shaderStage_t *pStage = &unfoggedStages[stage];

		if ( !pStage->active ) {
			break;
		}

		Com_Memset( pStage, 0, sizeof( *pStage ) );
	}
}

/*
=========================
FinishShader

Returns a freshly allocated shader with all the needed info
from the current global working shader
=========================
*/
static shader_t *FinishShader( void ) {
	int stage;
	int i;
	int bundle;
	qboolean hasLightmapStage;

	if (!currentShader) {
		currentShader = FindShaderText(shader.name);
	}

    if (shader.defaultShader)
    {
        currentShader->shader = tr.defaultShader;
        currentShader = NULL;
        return tr.defaultShader;
    }

	hasLightmapStage = qfalse;

	//
	// set sky stuff appropriate
	//
	if ( shader.isPortalSky ) {
		shader.sort = SS_PORTALSKY;
	}
	if ( shader.isSky ) {
		shader.sort = SS_ENVIRONMENT;
	}

	//
	// set polygon offset
	//
	if ( shader.polygonOffset && !shader.sort ) {
		shader.sort = SS_DECAL;
	}

	shader.needsLGrid = qfalse;
	shader.needsLSpherical = qfalse;

	//
	// set appropriate stage information
	//
	stage = 0;
	for (shaderStage_t* pStage = &unfoggedStages[0]; pStage->active; pStage++, stage++) {
		// check for a missing texture
		if ( !pStage->bundle[0].image[0] ) {
			ri.Printf( PRINT_WARNING, "Shader %s has a stage with no image\n", shader.name );
			pStage->active = qfalse;
			continue;
		}

		if (pStage->rgbGen == CGEN_LIGHTING_GRID) {
			shader.needsLGrid = 1;
		} else if (pStage->rgbGen == CGEN_LIGHTING_SPHERICAL || pStage->rgbGen == CGEN_STATIC) {
			shader.needsLSpherical = 1;
		}

		//
		// default texture coordinate generation
		//
		for (bundle = 0; bundle < 2; bundle++) {
			if (pStage->bundle[bundle].isLightmap) {
				if (pStage->bundle[bundle].tcGen == TCGEN_BAD) {
					pStage->bundle[bundle].tcGen = TCGEN_LIGHTMAP;
				}
				hasLightmapStage = qtrue;
			}
			else {
				if (pStage->bundle[bundle].tcGen == TCGEN_BAD) {
					pStage->bundle[bundle].tcGen = TCGEN_TEXTURE;
				}
			}
		}

		if (pStage->multitextureEnv && pStage->bundle[0].isLightmap) {
			//
			// exchange bundle
			//
			textureBundle_t tmpBundle = pStage->bundle[0];
			pStage->bundle[0] = pStage->bundle[1];
			pStage->bundle[1] = tmpBundle;
		}
	}

	//
	// if we are in r_vertexLight mode, never use a lightmap texture
	//
	if (stage > 1)
	{
		if (r_vertexLight->integer) {
			VertexLightingCollapse();
			stage = 1;
			hasLightmapStage = qfalse;
		}

		//
		// look for multitexture potential
		//
		if (qglActiveTextureARB) {
			CollapseMultitexture(&stage);
		};
	}

	for (i = 0; i < stage; i++) {
		int blendSrcBits, blendDstBits;

		shaderStage_t* pStage = &unfoggedStages[i];
		if (pStage->stateBits & (GLS_SRCBLEND_BITS | GLS_DSTBLEND_BITS))
		{
			blendSrcBits = pStage->stateBits & GLS_SRCBLEND_BITS;
			blendDstBits = pStage->stateBits & GLS_DSTBLEND_BITS;
			if (blendSrcBits == GLS_SRCBLEND_ONE && blendDstBits == GLS_DSTBLEND_ONE
				|| blendSrcBits == GLS_SRCBLEND_ZERO && blendDstBits == GLS_DSTBLEND_ONE_MINUS_SRC_COLOR
				|| blendSrcBits == GLS_SRCBLEND_SRC_ALPHA && blendDstBits == GLS_DSTBLEND_ONE
				|| blendSrcBits == GLS_SRCBLEND_DST_COLOR && blendDstBits == GLS_DSTBLEND_ONE
				|| blendSrcBits == GLS_SRCBLEND_ONE_MINUS_DST_COLOR && blendDstBits == GLS_DSTBLEND_ONE)
			{
				pStage->stateBits |= GLS_FOG_ENABLED | GLS_FOG_BLACK;
			}
			else if (blendSrcBits == GLS_SRCBLEND_DST_COLOR && blendDstBits == GLS_DSTBLEND_ZERO
					|| blendSrcBits == GLS_SRCBLEND_ZERO && blendDstBits == GLS_DSTBLEND_SRC_COLOR)
			{
				pStage->stateBits |= GLS_FOG_ENABLED | GLS_FOG_WHITE;
			}
			else if (blendSrcBits == GLS_SRCBLEND_SRC_ALPHA && blendDstBits == GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA
					|| blendSrcBits == GLS_SRCBLEND_ONE_MINUS_SRC_ALPHA && blendDstBits == GLS_DSTBLEND_SRC_ALPHA
					|| blendSrcBits == GLS_SRCBLEND_SRC_ALPHA && blendDstBits == GLS_DSTBLEND_ONE)
			{
				pStage->stateBits |= GLS_FOG_ENABLED;
			}
			else {
				ri.Printf(PRINT_WARNING, "Shader '%s' stage# %i is unfoggable\n", shader.name, i + 1);
			}

			if (shader.sort == SS_BAD)
			{
				if (pStage->stateBits & GLS_DEPTHMASK_TRUE) {
					shader.sort = SS_SEE_THROUGH;
				}
				else {
					shader.sort = SS_BLEND0;
				}
			}
		}
		else {
			pStage->stateBits |= GLS_FOG_ENABLED;
		}
	}

	// fogonly shaders don't have any normal passes
	if (!shader.sort) {
		shader.sort = SS_OPAQUE;
	}

	if (shader.lightmapIndex >= 0 && !hasLightmapStage) {
		ri.Printf(PRINT_DEVELOPER, "WARNING: shader '%s' has lightmap but no lightmap stage!\n", shader.name);
		shader.lightmapIndex = LIGHTMAP_NONE;
	}

	//
	// compute number of passes
	//
	shader.numUnfoggedPasses = 0;
	for (stage = 0; stage < MAX_SHADER_STAGES; stage++) {
		shaderStage_t* pStage = &unfoggedStages[stage];
		if (!pStage->active) {
			break;
		}

		shader.numUnfoggedPasses++;
	}

	// determine which stage iterator function is appropriate
	ComputeStageIteratorFunc();

	return GeneratePermanentShader();
}

//========================================================================================

/*
===============
R_FindShader

Will always return a valid shader, but it might be the
default shader if the real one can't be found.

In the interest of not requiring an explicit shader text entry to
be defined for every single image used in the game, three default
shader behaviors can be auto-created for any image:

If lightmapIndex == LIGHTMAP_NONE, then the image will have
dynamic diffuse lighting applied to it, as apropriate for most
entity skin surfaces.

If lightmapIndex == LIGHTMAP_2D, then the image will be used
for 2D rendering unless an explicit shader is found

If lightmapIndex == LIGHTMAP_BY_VERTEX, then the image will use
the vertex rgba modulate values, as apropriate for misc_model
pre-lit surfaces.

Other lightmapIndex values will have a lightmap stage created
and src*dest blending applied with the texture, as apropriate for
most world construction surfaces.

===============
*/
shader_t* R_FindShader(const char* name, int lightmapIndex, qboolean mipRawImage, qboolean picmip, qboolean wrapx, qboolean wrapy) {
	char		strippedName[MAX_QPATH];
	char		fileName[MAX_QPATH];
	int			i, j, hash;
	char		*shaderText;
	image_t		*image;
	shader_t	*sh;

	if ( name[0] == 0 ) {
		return tr.defaultShader;
	}

	// use (fullbright) vertex lighting if the bsp file doesn't have
	// lightmaps
	if ( lightmapIndex >= 0 && lightmapIndex >= tr.numLightmaps ) {
		lightmapIndex = LIGHTMAP_BY_VERTEX;
	}

	COM_StripExtension( name, strippedName, sizeof(strippedName));

	hash = generateHashValue(strippedName);

	//
	// see if the shader is already loaded
	//
	for (currentShader = hashTable[hash]; currentShader; currentShader = currentShader->next) {
		// NOTE: if there was no shader or image available with the name strippedName
		// then a default shader is created with lightmapIndex == LIGHTMAP_NONE, so we
		// have to check all default shaders otherwise for every call to R_FindShader
		// with that same strippedName a new default shader is created.
        if (!Q_stricmp(currentShader->name, strippedName))
		{
			// shader text found, but it has no shared assigned
			break;
		}
	}

	if (currentShader)
	{
		for (sh = currentShader->shader; sh; sh = sh->next)
		{
			if (sh->lightmapIndex == lightmapIndex || sh == tr.defaultShader) {
				return sh;
			}
		}
	}
	else
	{
		// create a new shader text
		currentShader = AddShaderTextToHash(strippedName, hash);
	}

	// make sure the render thread is stopped, because we are probably
	// going to have to upload an image
	if (r_smp->integer) {
		R_SyncRenderThread();
	}

	// clear the global shader
	Com_Memset( &shader, 0, sizeof( shader ) );
    Com_Memset(&unfoggedStages, 0, sizeof(unfoggedStages));
    Com_Memset(&texMods, 0, sizeof(texMods));

    shader.sprite.scale = 1.0;
    shader_noPicMip = qfalse;
    shader_noMipMaps = qfalse;
    shader_force32bit = qfalse;
	Q_strncpyz(shader.name, strippedName, sizeof(shader.name));
	shader.lightmapIndex = lightmapIndex;
	for ( i = 0 ; i < MAX_SHADER_STAGES ; i++ ) {
        for (j = 0; j < NUM_TEXTURE_BUNDLES; j++) {
            unfoggedStages[i].bundle[j].texMods = texMods[i];
		}
	}

	// FIXME: set these "need" values apropriately
	shader.needsNormal = qfalse;
	shader.needsST1 = qtrue;
	shader.needsST2 = qtrue;
	shader.needsColor = qtrue;

	//
	// attempt to define shader from an explicit parameter file
	//
	if ( currentShader->text ) {
		shaderText = currentShader->text;
		// enable this when building a pak file to get a global list
		// of all explicit shaders
		if ( r_printShaders->integer ) {
			ri.Printf( PRINT_ALL, "*SHADER* %s\n", name );
		}

		if ( !ParseShader( &shaderText ) ) {
			// had errors, so use default shader
			shader.defaultShader = qtrue;
		}

		if (shader.lightmapIndex == LIGHTMAP_BY_VERTEX && !(shader.surfaceFlags & SURF_HINT)) {
			unfoggedStages[0].rgbGen = CGEN_EXACT_VERTEX;
		}

		sh = FinishShader();
		return sh;
	}

	//
	// if not defined in the in-memory shader descriptions,
	// look for a single TGA, BMP, or PCX
	//
	Q_strncpyz( fileName, name, sizeof( fileName ) );
	COM_DefaultExtension( fileName, sizeof( fileName ), ".tga" );
    if (!r_forceClampToEdge->value) {
        image = R_FindImageFile(fileName, mipRawImage, picmip, qfalse, wrapx ? GL_REPEAT : GL_CLAMP, wrapy ? GL_REPEAT : GL_CLAMP);
	}
    else {
        image = R_FindImageFile(fileName, mipRawImage, picmip, qfalse, wrapx ? GL_REPEAT : GL_CLAMP_TO_EDGE, wrapy ? GL_REPEAT : GL_CLAMP_TO_EDGE);
	}

	if ( !image ) {
		ri.Printf( PRINT_DEVELOPER, "Couldn't find image for shader %s\n", name );
		shader.defaultShader = qtrue;
		return FinishShader();
	}

	//
	// create the default shading commands
	//
	if ( shader.lightmapIndex == LIGHTMAP_NONE ) {
		// dynamic colors at vertexes
		unfoggedStages[0].bundle[0].image[0] = image;
		unfoggedStages[0].active = qtrue;
		unfoggedStages[0].rgbGen = CGEN_LIGHTING_GRID;
		unfoggedStages[0].stateBits = GLS_DEFAULT;
	} else if ( shader.lightmapIndex == LIGHTMAP_BY_VERTEX ) {
		// explicit colors at vertexes
		unfoggedStages[0].bundle[0].image[0] = image;
		unfoggedStages[0].active = qtrue;
		unfoggedStages[0].rgbGen = CGEN_EXACT_VERTEX;
		unfoggedStages[0].alphaGen = AGEN_SKIP;
		unfoggedStages[0].stateBits = GLS_DEFAULT;
	} else if ( shader.lightmapIndex == LIGHTMAP_2D ) {
		// GUI elements
		unfoggedStages[0].bundle[0].image[0] = image;
        unfoggedStages[0].active = qtrue;
        unfoggedStages[0].force32bit = qtrue;
		unfoggedStages[0].rgbGen = CGEN_GLOBAL_COLOR;
		unfoggedStages[0].alphaGen = AGEN_GLOBAL_ALPHA;
		unfoggedStages[0].stateBits = GLS_DEPTHTEST_DISABLE |
			  GLS_SRCBLEND_SRC_ALPHA |
			  GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA;
	} else if ( shader.lightmapIndex == LIGHTMAP_WHITEIMAGE ) {
		// fullbright level
		unfoggedStages[0].bundle[0].image[0] = tr.whiteImage;
		unfoggedStages[0].active = qtrue;
		unfoggedStages[0].rgbGen = CGEN_IDENTITY_LIGHTING;
		unfoggedStages[0].stateBits = GLS_DEFAULT;

		unfoggedStages[1].bundle[0].image[0] = image;
		unfoggedStages[1].active = qtrue;
		unfoggedStages[1].rgbGen = CGEN_IDENTITY;
		unfoggedStages[1].stateBits |= GLS_SRCBLEND_DST_COLOR | GLS_DSTBLEND_ZERO;
	} else {
		// two pass lightmap
		unfoggedStages[0].bundle[0].image[0] = tr.lightmaps[shader.lightmapIndex];
		unfoggedStages[0].bundle[0].isLightmap = qtrue;
		unfoggedStages[0].active = qtrue;
		unfoggedStages[0].rgbGen = CGEN_IDENTITY;	// lightmaps are scaled on creation
													// for identitylight
		unfoggedStages[0].stateBits = GLS_DEFAULT;

		unfoggedStages[1].bundle[0].image[0] = image;
		unfoggedStages[1].active = qtrue;
		unfoggedStages[1].rgbGen = CGEN_IDENTITY;
		unfoggedStages[1].stateBits |= GLS_SRCBLEND_DST_COLOR | GLS_DSTBLEND_ZERO;
	}

	return FinishShader();
}

/* 
====================
RE_RegisterShader

This is the exported shader entry point for the rest of the system
It will always return an index that will be valid.

This should really only be used for explicit shaders, because there is no
way to ask for different implicit lighting modes (vertex, lightmap, etc)
====================
*/
qhandle_t RE_RegisterShader( const char *name ) {
	shader_t	*sh;

	if ( strlen( name ) >= MAX_QPATH ) {
		Com_Printf( "Shader name exceeds MAX_QPATH\n" );
		return 0;
	}

	sh = R_FindShader( name, LIGHTMAP_2D, qtrue, qtrue, qtrue, qtrue);

	// we want to return 0 if the shader failed to
	// load for some reason, but R_FindShader should
	// still keep a name allocated for it, so if
	// something calls RE_RegisterShader again with
	// the same name, we don't try looking for it again
	if ( sh->defaultShader ) {
		return 0;
	}

	return sh->index;
}


/*
====================
RE_RegisterShaderNoMip

For menu graphics that should never be picmiped
====================
*/
qhandle_t RE_RegisterShaderNoMip( const char *name ) {
	shader_t	*sh;

	if ( strlen( name ) >= MAX_QPATH ) {
		Com_Printf( "Shader name exceeds MAX_QPATH\n" );
		return 0;
	}

	sh = R_FindShader( name, LIGHTMAP_2D, qfalse, qfalse, qfalse, qfalse );

	// we want to return 0 if the shader failed to
	// load for some reason, but R_FindShader should
	// still keep a name allocated for it, so if
	// something calls RE_RegisterShader again with
	// the same name, we don't try looking for it again
	if ( sh->defaultShader ) {
		return 0;
	}

	return sh->index;
}

qhandle_t RE_RefreshShaderNoMip(const char* name) {
	// FIXME: unimplemented
	// Workaround
	return RE_RegisterShaderNoMip(name);
}

/*
====================
R_GetShaderByHandle

When a handle is passed in by another module, this range checks
it and returns a valid (possibly default) shader_t to be used internally.
====================
*/
shader_t *R_GetShaderByHandle( qhandle_t hShader ) {
	if ( hShader < 0 ) {
	  ri.Printf( PRINT_WARNING, "R_GetShaderByHandle: out of range hShader '%d'\n", hShader ); // bk: FIXME name
		return tr.defaultShader;
	}
	if ( hShader >= tr.numShaders ) {
		ri.Printf( PRINT_WARNING, "R_GetShaderByHandle: out of range hShader '%d'\n", hShader );
		return tr.defaultShader;
	}
	return tr.shaders[hShader];
}

/*
===============
R_ShaderList_f

Dump information on all valid shaders to the console
A second parameter will cause it to print in sorted order
===============
*/
void	R_ShaderList_f (void) {
	int			i;
	int			count;
	shader_t	*shader;

	ri.Printf (PRINT_ALL, "-----------------------\n");

	count = 0;
	for ( i = 0 ; i < tr.numShaders ; i++ ) {
		int stage;

		if ( ri.Cmd_Argc() > 1 ) {
			shader = tr.sortedShaders[i];
		} else {
			shader = tr.shaders[i];
		}

		ri.Printf( PRINT_ALL, "%i ", shader->numUnfoggedPasses );

		if (shader->lightmapIndex >= 0 ) {
			ri.Printf (PRINT_ALL, "L ");
		} else {
			ri.Printf (PRINT_ALL, "  ");
		}

		for (stage = 0; shader->unfoggedStages[stage] && shader->unfoggedStages[stage]->active; stage++) {
			if (shader->unfoggedStages[stage]->multitextureEnv == GL_ADD ) {
				ri.Printf( PRINT_ALL, "MT(a) " );
			} else if (shader->unfoggedStages[stage]->multitextureEnv == GL_MODULATE ) {
				ri.Printf( PRINT_ALL, "MT(m) " );
			} else if (shader->unfoggedStages[stage]->multitextureEnv == GL_DECAL ) {
				ri.Printf( PRINT_ALL, "MT(d) " );
			} else {
				ri.Printf( PRINT_ALL, "      " );
			}
		}

		if ( shader->explicitlyDefined ) {
			ri.Printf( PRINT_ALL, "E " );
		} else {
			ri.Printf( PRINT_ALL, "  " );
		}

		if ( shader->optimalStageIteratorFunc == RB_StageIteratorGeneric ) {
			ri.Printf( PRINT_ALL, "gen " );
		} else if ( shader->optimalStageIteratorFunc == RB_StageIteratorSky ) {
			ri.Printf( PRINT_ALL, "sky " );
		} else if ( shader->optimalStageIteratorFunc == RB_StageIteratorLightmappedMultitextureUnfogged ) {
			ri.Printf( PRINT_ALL, "lmmt" );
		} else if ( shader->optimalStageIteratorFunc == RB_StageIteratorVertexLitTextureUnfogged ) {
			ri.Printf( PRINT_ALL, "vlt " );
		} else {
			ri.Printf( PRINT_ALL, "    " );
		}

		if ( shader->defaultShader ) {
			ri.Printf (PRINT_ALL,  ": %s (DEFAULTED)\n", shader->name);
		} else {
			ri.Printf (PRINT_ALL,  ": %s\n", shader->name);
		}
		count++;
	}
	ri.Printf (PRINT_ALL, "%i total shaders\n", count);
	ri.Printf (PRINT_ALL, "------------------\n");
}


/*
====================
ScanAndLoadShaderFiles

Finds and loads all .shader files, combining them into
a single large text block that can be scanned for shader names
=====================
*/
#define	MAX_SHADER_FILES	4096
static void ScanAndLoadShaderFiles( void )
{
    char** shaderFiles;
    char* buffers[MAX_SHADER_FILES];
    char* p;
    int numShaders;
    int i;

    long sum = 0;
    // scan for shader files
    shaderFiles = ri.FS_ListFiles("scripts", ".shader", &numShaders);

    if (!shaderFiles || !numShaders)
    {
        ri.Printf(PRINT_WARNING, "WARNING: no shader files found\n");
        return;
    }

    if (numShaders > MAX_SHADER_FILES) {
        numShaders = MAX_SHADER_FILES;
    }

    // load and parse shader files
    for (i = 0; i < numShaders; i++)
    {
        char filename[MAX_QPATH];

        Com_sprintf(filename, sizeof(filename), "scripts/%s", shaderFiles[i]);
        ri.Printf(PRINT_ALL, "...loading '%s'\n", filename);
        sum += ri.FS_ReadFile(filename, (void**)&buffers[i]);
        if (!buffers[i]) {
            ri.Error(ERR_DROP, "Couldn't load %s", filename);
        }
    }

    // build single large buffer
    s_shaderText = ri.Malloc(sum + numShaders * 2);
    s_shaderText[0] = 0;

    // free in reverse order, so the temp files are all dumped
    for (i = numShaders - 1; i >= 0; i--) {
        strcat(s_shaderText, "\n");
        p = &s_shaderText[strlen(s_shaderText)];
        strcat(s_shaderText, buffers[i]);
        ri.FS_FreeFile(buffers[i]);
        buffers[i] = p;
    }

    COM_Compress(s_shaderText);
    // free up memory
    ri.FS_FreeFileList(shaderFiles);
}

static void FindShadersInShaderText()
{
	char* p;
	char* oldp;
	char* token;

	if (!s_shaderText) {
		return;
	}

	p = s_shaderText;
    // look for label
    while (1) {
        oldp = p;
        token = COM_ParseExt(&p, qtrue);
        if (token[0] == 0) {
            break;
        }

        if (*token == '{')
        {
            p = oldp;
            SkipBracedSection(&p);
        }
        else
        {
            currentShader = AllocShaderText(token);
            currentShader->text = p;
        }
    }
}

/*
====================
CreateInternalShaders
====================
*/
static void CreateInternalShaders( void ) {
	tr.numShaders = 0;

	// init the default shader
	Com_Memset( &shader, 0, sizeof( shader ) );
	Com_Memset( &unfoggedStages, 0, sizeof( unfoggedStages ) );

	Q_strncpyz( shader.name, "<default>", sizeof( shader.name ) );
	shader.lightmapIndex = LIGHTMAP_NONE;
	unfoggedStages[0].bundle[0].image[0] = tr.defaultImage;
	unfoggedStages[0].active = qtrue;
	unfoggedStages[0].stateBits = GLS_DEFAULT;
	currentShader = FindShaderText(shader.name);
    tr.defaultShader = FinishShader();

    // white shader
    Q_strncpyz(shader.name, "<white>", sizeof(shader.name));
    shader.lightmapIndex = LIGHTMAP_NONE;
    unfoggedStages[0].bundle[0].image[0] = tr.whiteImage;
    unfoggedStages[0].active = qtrue;
    unfoggedStages[0].stateBits = GLS_DEFAULT;
    currentShader = FindShaderText(shader.name);
	FinishShader();

	// shadow shader is just a marker
	Q_strncpyz( shader.name, "<stencil shadow>", sizeof( shader.name ) );
    shader.sort = SS_STENCIL_SHADOW;
    shader.lightmapIndex = LIGHTMAP_NONE;
	currentShader = FindShaderText(shader.name);
	tr.shadowShader = FinishShader();
}

static void CreateExternalShaders( void ) {
	tr.flareShader = R_FindShader( "flareShader", LIGHTMAP_NONE, qtrue, qtrue, qtrue, qtrue);
}

void R_StartupShaders()
{
    ri.Printf(PRINT_ALL, "Initializing Shaders\n");

    currentShader = NULL;
    s_shaderText = NULL;

    Com_Memset(hashTable, 0, sizeof(hashTable));

    ScanAndLoadShaderFiles();

    FindShadersInShaderText();

    R_SetupShaders();
}

void R_ShutdownShaders()
{
	shadertext_t* shader, * next;
	int hash;

	if (s_shaderText) {
		ri.Free(s_shaderText);
	}

    for (hash = 0; hash < FILE_HASH_SIZE; hash++) {
        for (shader = hashTable[hash]; shader; shader = next) {
            next = shader->next;
            ri.Free(shader);
        }
        hashTable[hash] = NULL;
	}
}

void R_SetupShaders()
{
	shadertext_t* shader;
	int hash;

    ri.Printf(0, "Setting up Shaders\n");
    currentShader = NULL;
    for (hash = 0; hash < FILE_HASH_SIZE; ++hash)
    {
		for (shader = hashTable[hash]; shader; shader = shader->next) {
			shader->shader = NULL;
		}
    }

    CreateInternalShaders();
    InitStaticShaders();
    CreateExternalShaders();
}
