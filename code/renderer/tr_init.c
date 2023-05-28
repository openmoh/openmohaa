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
// tr_init.c -- functions that are not called every frame

#include "tr_local.h"

glconfig_t	glConfig;
glstate_t	glState;
int r_sequencenumber;

static void GfxInfo_f( void );

cvar_t	*r_flareSize;
cvar_t	*r_flareFade;

cvar_t	*r_ignoreFastPath;

cvar_t	*r_verbose;
cvar_t	*r_ignore;

cvar_t	*r_displayRefresh;

cvar_t	*r_znear;

cvar_t	*r_smp;
cvar_t	*r_showSmp;
cvar_t	*r_skipBackEnd;

cvar_t	*r_ignorehwgamma;
cvar_t	*r_measureOverdraw;

cvar_t	*r_inGameVideo;
cvar_t	*r_fastsky;
cvar_t	*r_fastdlights;
cvar_t	*r_drawSun;
cvar_t	*r_dlightBacks;

cvar_t	*r_lodscale;

cvar_t	*r_norefresh;
cvar_t	*r_drawentities;
cvar_t	*r_drawentitypoly;
cvar_t	*r_drawstaticmodels;
cvar_t	*r_drawstaticmodelpoly;
cvar_t	*r_drawbrushes;
cvar_t	*r_drawbrushmodels;
cvar_t	*r_drawstaticdecals;
cvar_t	*r_drawterrain;
cvar_t	*r_drawsprites;
cvar_t	*r_drawspherelights;
cvar_t	*r_drawworld;
cvar_t	*r_speeds;
cvar_t	*r_fullbright;
cvar_t	*r_novis;
cvar_t	*r_nocull;
cvar_t	*r_showcull;
cvar_t	*r_facePlaneCull;
cvar_t	*r_showcluster;
cvar_t	*r_nocurves;

cvar_t	*r_allowExtensions;

cvar_t	*r_ext_compressed_textures;
cvar_t	*r_ext_gamma_control;
cvar_t	*r_ext_multitexture;
cvar_t	*r_ext_compiled_vertex_array;
cvar_t	*r_ext_texture_env_add;

cvar_t	*r_ext_texture_env_combine;
cvar_t	*r_ext_aniso_filter;
cvar_t	*r_forceClampToEdge;
cvar_t	*r_geForce3WorkAround;
cvar_t	*r_reset_tc_array;

cvar_t	*r_ignoreGLErrors;
cvar_t	*r_logFile;

cvar_t	*r_stencilbits;
cvar_t	*r_depthbits;
cvar_t	*r_colorbits;
cvar_t	*r_stereo;
cvar_t	*r_primitives;
cvar_t	*r_textureDetails;
cvar_t	*r_texturebits;

cvar_t	*r_drawBuffer;
cvar_t  *r_glDriver;
cvar_t	*r_lightmap;
cvar_t	*r_vertexLight;
cvar_t	*r_shadows;
cvar_t	*r_entlight_scale;
cvar_t	*r_entlight_errbound;
cvar_t	*r_entlight_cubelevel;
cvar_t	*r_entlight_cubefraction;
cvar_t	*r_entlight_maxcalc;
cvar_t	*r_flares;
cvar_t	*r_mode;
cvar_t	*r_nobind;
cvar_t	*r_singleShader;
cvar_t	*r_lerpmodels;
cvar_t	*r_roundImagesDown;
cvar_t	*r_colorMipLevels;
cvar_t	*r_picmip;
cvar_t	*r_showtris;
cvar_t	*r_showsky;
cvar_t	*r_shownormals;
cvar_t	*r_showhbox;
cvar_t	*r_showstaticbboxes;
cvar_t	*r_finish;
cvar_t	*r_clear;
cvar_t	*r_swapInterval;
cvar_t	*r_textureMode;
cvar_t	*r_offsetFactor;
cvar_t	*r_offsetUnits;
cvar_t	*r_gamma;
cvar_t	*r_intensity;
cvar_t	*r_lockpvs;
cvar_t	*r_noportals;
cvar_t	*r_entlightmap;
cvar_t	*r_fastentlight;
cvar_t	*r_portalOnly;

cvar_t	*r_subdivisions;
cvar_t	*r_lodCurveError;

cvar_t	*r_fullscreen;

cvar_t	*r_customwidth;
cvar_t	*r_customheight;
cvar_t	*r_customaspect;

cvar_t	*r_overBrightBits;
cvar_t	*r_mapOverBrightBits;

cvar_t	*r_debugSurface;

cvar_t	*r_showImages;
cvar_t	*r_showlod;
cvar_t	*r_showstaticlod;

cvar_t	*r_ambientScale;
cvar_t	*r_directedScale;
cvar_t	*r_debugSort;
cvar_t	*r_printShaders;
cvar_t	*r_saveFontData;

cvar_t	*r_maxpolys;
int		max_polys;
cvar_t	*r_maxpolyverts;
int		max_polyverts;
cvar_t* r_maxtermarks;
int		max_termarks;
cvar_t* r_precacheimages;

cvar_t* r_staticlod;
cvar_t* r_lodscale;
cvar_t* r_lodcap;
cvar_t* r_lodviewmodelcap;

cvar_t* r_uselod;
cvar_t* lod_LOD;
cvar_t* lod_minLOD;
cvar_t* lod_maxLOD;
cvar_t* lod_LOD_slider;
cvar_t* lod_curve_0_val;
cvar_t* lod_curve_1_val;
cvar_t* lod_curve_2_val;
cvar_t* lod_curve_3_val;
cvar_t* lod_curve_4_val;
cvar_t* lod_edit_0;
cvar_t* lod_edit_1;
cvar_t* lod_edit_2;
cvar_t* lod_edit_3;
cvar_t* lod_edit_4;
cvar_t* lod_curve_0_slider;
cvar_t* lod_curve_1_slider;
cvar_t* lod_curve_2_slider;
cvar_t* lod_curve_3_slider;
cvar_t* lod_curve_4_slider;
cvar_t* lod_pitch_val;
cvar_t* lod_zee_val;
cvar_t* lod_mesh;
cvar_t* lod_meshname;
cvar_t* lod_tikiname;
cvar_t* lod_metric;
cvar_t* lod_tris;
cvar_t* lod_position;
cvar_t* lod_save;
cvar_t* lod_tool;

cvar_t* sys_cpuid;
cvar_t* r_sse;
cvar_t* r_static_shaderdata0;
cvar_t* r_static_shaderdata1;
cvar_t* r_static_shaderdata2;
cvar_t* r_static_shaderdata3;
cvar_t* r_static_shadermultiplier0;
cvar_t* r_static_shadermultiplier1;
cvar_t* r_static_shadermultiplier2;
cvar_t* r_static_shadermultiplier3;

cvar_t* r_numdebuglines;


cvar_t* r_stipplelines;
cvar_t* r_light_lines;
cvar_t* r_light_sun_line;
cvar_t* r_light_int_scale;
cvar_t* r_light_nolight;
cvar_t* r_light_showgrid;
cvar_t* r_skyportal;
cvar_t* r_skyportal_origin;
cvar_t* r_farplane;
cvar_t* r_farplane_color;
cvar_t* r_farplane_nocull;
cvar_t* r_farplane_nofog;
cvar_t* r_lightcoronasize;
cvar_t* r_useglfog;
cvar_t* r_debuglines_depthmask;
cvar_t* r_smoothsmokelight;
cvar_t* r_showportal;
cvar_t* ter_minMarkRadius;
cvar_t* ter_fastMarks;
cvar_t* r_bumpmap;
cvar_t* r_loadjpg;
cvar_t* r_loadftx;

cvar_t* r_showSkeleton;

cvar_t* r_ext_multisample;
cvar_t* r_noborder;
cvar_t* r_ext_texture_filter_anisotropic;
cvar_t* r_stereoEnabled;

qboolean  textureFilterAnisotropic = qtrue;
int       maxAnisotropy = 16;
float     displayAspect = 16.f / 9.f;
qboolean  haveClampToEdge = qtrue;

static void AssertCvarRange( cvar_t *cv, float minVal, float maxVal, qboolean shouldBeIntegral )
{
	if ( shouldBeIntegral )
	{
		if ( ( int ) cv->value != cv->integer )
		{
			ri.Printf( PRINT_WARNING, "WARNING: cvar '%s' must be integral (%f)\n", cv->name, cv->value );
			ri.Cvar_Set( cv->name, va( "%d", cv->integer ) );
		}
	}

	if ( cv->value < minVal )
	{
		ri.Printf( PRINT_WARNING, "WARNING: cvar '%s' out of range (%f < %f)\n", cv->name, cv->value, minVal );
		ri.Cvar_Set( cv->name, va( "%f", minVal ) );
	}
	else if ( cv->value > maxVal )
	{
		ri.Printf( PRINT_WARNING, "WARNING: cvar '%s' out of range (%f > %f)\n", cv->name, cv->value, maxVal );
		ri.Cvar_Set( cv->name, va( "%f", maxVal ) );
	}
}


/*
** InitOpenGL
**
** This function is responsible for initializing a valid OpenGL subsystem.  This
** is done by calling GLimp_Init (which gives us a working OGL subsystem) then
** setting variables, checking GL constants, and reporting the gfx system config
** to the user.
*/
static void InitOpenGL( void )
{
	char renderer_buffer[1024];

	//
	// initialize OS specific portions of the renderer
	//
	// GLimp_Init directly or indirectly references the following cvars:
	//		- r_fullscreen
	//		- r_glDriver
	//		- r_mode
	//		- r_(color|depth|stencil)bits
	//		- r_ignorehwgamma
	//		- r_gamma
	//
	
	if ( glConfig.vidWidth == 0 )
	{
		GLint		temp;
		
		GLimp_Init(qtrue);

		strcpy( renderer_buffer, glConfig.renderer_string );
		Q_strlwr( renderer_buffer );

		// OpenGL driver constants
		qglGetIntegerv( GL_MAX_TEXTURE_SIZE, &temp );
		glConfig.maxTextureSize = temp;

		// stubbed or broken drivers may have reported 0...
		if ( glConfig.maxTextureSize <= 0 ) 
		{
			glConfig.maxTextureSize = 0;
		}
	}

	// init command buffers and SMP
	R_InitCommandBuffers();

	// print info
	GfxInfo_f();

	// set default state
	GL_SetDefaultState();
}

/*
==================
GL_CheckErrors
==================
*/
void GL_CheckErrors( void ) {
    int		err;
    char	s[64];

    err = qglGetError();
    if ( err == GL_NO_ERROR ) {
        return;
    }
    if ( r_ignoreGLErrors->integer ) {
        return;
    }
    switch( err ) {
        case GL_INVALID_ENUM:
            strcpy( s, "GL_INVALID_ENUM" );
            break;
        case GL_INVALID_VALUE:
            strcpy( s, "GL_INVALID_VALUE" );
            break;
        case GL_INVALID_OPERATION:
            strcpy( s, "GL_INVALID_OPERATION" );
            break;
        case GL_STACK_OVERFLOW:
            strcpy( s, "GL_STACK_OVERFLOW" );
            break;
        case GL_STACK_UNDERFLOW:
            strcpy( s, "GL_STACK_UNDERFLOW" );
            break;
        case GL_OUT_OF_MEMORY:
            strcpy( s, "GL_OUT_OF_MEMORY" );
            break;
        default:
            Com_sprintf( s, sizeof(s), "%i", err);
            break;
    }

    ri.Error( ERR_FATAL, "GL_CheckErrors: %s", s );
}


/*
** R_GetModeInfo
*/
typedef struct vidmode_s
{
    const char *description;
    int         width, height;
	float		pixelAspect;		// pixel width / height
} vidmode_t;

vidmode_t r_vidModes[] =
{
    { "Mode  0: 320x240",		320,	240,	1 },
    { "Mode  1: 400x300",		400,	300,	1 },
    { "Mode  2: 512x384",		512,	384,	1 },
    { "Mode  3: 640x480",		640,	480,	1 },
    { "Mode  4: 800x600",		800,	600,	1 },
    { "Mode  5: 960x720",		960,	720,	1 },
    { "Mode  6: 1024x768",		1024,	768,	1 },
    { "Mode  7: 1152x864",		1152,	864,	1 },
    { "Mode  8: 1280x1024",		1280,	1024,	1 },
    { "Mode  9: 1600x1200",		1600,	1200,	1 },
    { "Mode 10: 2048x1536",		2048,	1536,	1 },
    { "Mode 11: 856x480 (wide)",856,	480,	1 }
};
static int	s_numVidModes = ( sizeof( r_vidModes ) / sizeof( r_vidModes[0] ) );

qboolean R_GetModeInfo( int *width, int *height, float *windowAspect, int mode ) {
	vidmode_t	*vm;

    if ( mode < -1 ) {
        return qfalse;
	}
	if ( mode >= s_numVidModes ) {
		return qfalse;
	}

	if ( mode == -1 ) {
		*width = r_customwidth->integer;
		*height = r_customheight->integer;
		*windowAspect = r_customaspect->value;
		return qtrue;
	}

	vm = &r_vidModes[mode];

    *width  = vm->width;
    *height = vm->height;
    *windowAspect = (float)vm->width / ( vm->height * vm->pixelAspect );

    return qtrue;
}

/*
** R_ModeList_f
*/
static void R_ModeList_f( void )
{
	int i;

	ri.Printf( PRINT_ALL, "\n" );
	for ( i = 0; i < s_numVidModes; i++ )
	{
		ri.Printf( PRINT_ALL, "%s\n", r_vidModes[i].description );
	}
	ri.Printf( PRINT_ALL, "\n" );
}


/* 
============================================================================== 
 
						SCREEN SHOTS 

NOTE TTimo
some thoughts about the screenshots system:
screenshots get written in fs_homepath + fs_gamedir
vanilla q3 .. baseq3/screenshots/ *.tga
team arena .. missionpack/screenshots/ *.tga

two commands: "screenshot" and "screenshotJPEG"
we use statics to store a count and start writing the first screenshot/screenshot????.tga (.jpg) available
(with FS_FileExists / FS_FOpenFileWrite calls)
FIXME: the statics don't get a reinit between fs_game changes

============================================================================== 
*/ 

/* 
================== 
RB_TakeScreenshot
================== 
*/  
void RB_TakeScreenshot( int x, int y, int width, int height, char *fileName ) {
	byte		*buffer;
	int			i, c, temp;
		
	buffer = ri.Hunk_AllocateTempMemory(glConfig.vidWidth*glConfig.vidHeight*3+18);

	Com_Memset (buffer, 0, 18);
	buffer[2] = 2;		// uncompressed type
	buffer[12] = width & 255;
	buffer[13] = width >> 8;
	buffer[14] = height & 255;
	buffer[15] = height >> 8;
	buffer[16] = 24;	// pixel size

	qglReadPixels( x, y, width, height, GL_RGB, GL_UNSIGNED_BYTE, buffer+18 ); 

	// swap rgb to bgr
	c = 18 + width * height * 3;
	for (i=18 ; i<c ; i+=3) {
		temp = buffer[i];
		buffer[i] = buffer[i+2];
		buffer[i+2] = temp;
	}

	// gamma correct
	if ( ( tr.overbrightBits > 0 ) && glConfig.deviceSupportsGamma ) {
		R_GammaCorrect( buffer + 18, glConfig.vidWidth * glConfig.vidHeight * 3 );
	}

	ri.FS_WriteFile( fileName, buffer, c );

	ri.Hunk_FreeTempMemory( buffer );
}

/* 
================== 
RB_TakeScreenshotJPEG
================== 
*/  
void RB_TakeScreenshotJPEG( int x, int y, int width, int height, char *fileName ) {
	// FIXME: unimplemented
#if 0
	byte		*buffer;

	buffer = ri.Hunk_AllocateTempMemory(glConfig.vidWidth*glConfig.vidHeight*4);

	qglReadPixels( x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, buffer ); 

	// gamma correct
	if ( ( tr.overbrightBits > 0 ) && glConfig.deviceSupportsGamma ) {
		R_GammaCorrect( buffer, glConfig.vidWidth * glConfig.vidHeight * 4 );
	}

	ri.FS_WriteFile( fileName, buffer, 1 );		// create path
	SaveJPG( fileName, 95, glConfig.vidWidth, glConfig.vidHeight, buffer);

	ri.Hunk_FreeTempMemory( buffer );
#endif
}

/*
==================
RB_TakeScreenshotCmd
==================
*/
const void *RB_TakeScreenshotCmd( const void *data ) {
	const screenshotCommand_t	*cmd;
	
	cmd = (const screenshotCommand_t *)data;
	
	if (cmd->jpeg)
		RB_TakeScreenshotJPEG( cmd->x, cmd->y, cmd->width, cmd->height, cmd->fileName);
	else
		RB_TakeScreenshot( cmd->x, cmd->y, cmd->width, cmd->height, cmd->fileName);
	
	return (const void *)(cmd + 1);	
}

/*
==================
R_TakeScreenshot
==================
*/
void R_TakeScreenshot( int x, int y, int width, int height, char *name, qboolean jpeg ) {
	static char	fileName[MAX_OSPATH]; // bad things if two screenshots per frame?
	screenshotCommand_t	*cmd;

	cmd = R_GetCommandBuffer( sizeof( *cmd ) );
	if ( !cmd ) {
		return;
	}
	cmd->commandId = RC_SCREENSHOT;

	cmd->x = x;
	cmd->y = y;
	cmd->width = width;
	cmd->height = height;
	Q_strncpyz( fileName, name, sizeof(fileName) );
	cmd->fileName = fileName;
	cmd->jpeg = jpeg;
}

/* 
================== 
R_ScreenshotFilename
================== 
*/  
void R_ScreenshotFilename( int lastNumber, char *fileName ) {
	int		a,b,c,d;

	if ( lastNumber < 0 || lastNumber > 9999 ) {
		Com_sprintf( fileName, MAX_OSPATH, "screenshots/shot9999.tga" );
		return;
	}

	a = lastNumber / 1000;
	lastNumber -= a*1000;
	b = lastNumber / 100;
	lastNumber -= b*100;
	c = lastNumber / 10;
	lastNumber -= c*10;
	d = lastNumber;

	Com_sprintf( fileName, MAX_OSPATH, "screenshots/shot%i%i%i%i.tga"
		, a, b, c, d );
}

/* 
================== 
R_ScreenshotFilename
================== 
*/  
void R_ScreenshotFilenameJPEG( int lastNumber, char *fileName ) {
	int		a,b,c,d;

	if ( lastNumber < 0 || lastNumber > 9999 ) {
		Com_sprintf( fileName, MAX_OSPATH, "screenshots/shot9999.jpg" );
		return;
	}

	a = lastNumber / 1000;
	lastNumber -= a*1000;
	b = lastNumber / 100;
	lastNumber -= b*100;
	c = lastNumber / 10;
	lastNumber -= c*10;
	d = lastNumber;

	Com_sprintf( fileName, MAX_OSPATH, "screenshots/shot%i%i%i%i.jpg"
		, a, b, c, d );
}

/*
====================
R_LevelShot

levelshots are specialized 128*128 thumbnails for
the menu system, sampled down from full screen distorted images
====================
*/
void R_LevelShot( void ) {
	char		checkname[MAX_OSPATH];
	byte		*buffer;
	byte		*source;
	byte		*src, *dst;
	int			x, y;
	int			r, g, b;
	float		xScale, yScale;
	int			xx, yy;

	sprintf( checkname, "levelshots/%s.tga", tr.world->baseName );

	source = ri.Hunk_AllocateTempMemory( glConfig.vidWidth * glConfig.vidHeight * 3 );

	buffer = ri.Hunk_AllocateTempMemory( 128 * 128*3 + 18);
	Com_Memset (buffer, 0, 18);
	buffer[2] = 2;		// uncompressed type
	buffer[12] = 128;
	buffer[14] = 128;
	buffer[16] = 24;	// pixel size

	qglReadPixels( 0, 0, glConfig.vidWidth, glConfig.vidHeight, GL_RGB, GL_UNSIGNED_BYTE, source ); 

	// resample from source
	xScale = glConfig.vidWidth / 512.0f;
	yScale = glConfig.vidHeight / 384.0f;
	for ( y = 0 ; y < 128 ; y++ ) {
		for ( x = 0 ; x < 128 ; x++ ) {
			r = g = b = 0;
			for ( yy = 0 ; yy < 3 ; yy++ ) {
				for ( xx = 0 ; xx < 4 ; xx++ ) {
					src = source + 3 * ( glConfig.vidWidth * (int)( (y*3+yy)*yScale ) + (int)( (x*4+xx)*xScale ) );
					r += src[0];
					g += src[1];
					b += src[2];
				}
			}
			dst = buffer + 18 + 3 * ( y * 128 + x );
			dst[0] = b / 12;
			dst[1] = g / 12;
			dst[2] = r / 12;
		}
	}

	// gamma correct
	if ( ( tr.overbrightBits > 0 ) && glConfig.deviceSupportsGamma ) {
		R_GammaCorrect( buffer + 18, 128 * 128 * 3 );
	}

	ri.FS_WriteFile( checkname, buffer, 128 * 128*3 + 18 );

	ri.Hunk_FreeTempMemory( buffer );
	ri.Hunk_FreeTempMemory( source );

	ri.Printf( PRINT_ALL, "Wrote %s\n", checkname );
}

/* 
================== 
R_ScreenShot_f

screenshot
screenshot [silent]
screenshot [levelshot]
screenshot [filename]

Doesn't print the pacifier message if there is a second arg
================== 
*/  
void R_ScreenShot_f (void) {
	char	checkname[MAX_OSPATH];
	static	int	lastNumber = -1;
	qboolean	silent;

	if ( !strcmp( ri.Cmd_Argv(1), "levelshot" ) ) {
		R_LevelShot();
		return;
	}

	if ( !strcmp( ri.Cmd_Argv(1), "silent" ) ) {
		silent = qtrue;
	} else {
		silent = qfalse;
	}

	if ( ri.Cmd_Argc() == 2 && !silent ) {
		// explicit filename
		Com_sprintf( checkname, MAX_OSPATH, "screenshots/%s.tga", ri.Cmd_Argv( 1 ) );
	} else {
		// scan for a free filename

		// if we have saved a previous screenshot, don't scan
		// again, because recording demo avis can involve
		// thousands of shots
		if ( lastNumber == -1 ) {
			lastNumber = 0;
		}
		// scan for a free number
		for ( ; lastNumber <= 9999 ; lastNumber++ ) {
			R_ScreenshotFilename( lastNumber, checkname );

      if (!ri.FS_FileExists( checkname ))
      {
        break; // file doesn't exist
      }
		}

		if ( lastNumber >= 9999 ) {
			ri.Printf (PRINT_ALL, "ScreenShot: Couldn't create a file\n"); 
			return;
 		}

		lastNumber++;
	}

	R_TakeScreenshot( 0, 0, glConfig.vidWidth, glConfig.vidHeight, checkname, qfalse );

	if ( !silent ) {
		ri.Printf (PRINT_ALL, "Wrote %s\n", checkname);
	}
} 

void R_ScreenShotJPEG_f (void) {
	char		checkname[MAX_OSPATH];
	static	int	lastNumber = -1;
	qboolean	silent;

	if ( !strcmp( ri.Cmd_Argv(1), "levelshot" ) ) {
		R_LevelShot();
		return;
	}

	if ( !strcmp( ri.Cmd_Argv(1), "silent" ) ) {
		silent = qtrue;
	} else {
		silent = qfalse;
	}

	if ( ri.Cmd_Argc() == 2 && !silent ) {
		// explicit filename
		Com_sprintf( checkname, MAX_OSPATH, "screenshots/%s.jpg", ri.Cmd_Argv( 1 ) );
	} else {
		// scan for a free filename

		// if we have saved a previous screenshot, don't scan
		// again, because recording demo avis can involve
		// thousands of shots
		if ( lastNumber == -1 ) {
			lastNumber = 0;
		}
		// scan for a free number
		for ( ; lastNumber <= 9999 ; lastNumber++ ) {
			R_ScreenshotFilenameJPEG( lastNumber, checkname );

      if (!ri.FS_FileExists( checkname ))
      {
        break; // file doesn't exist
      }
		}

		if ( lastNumber == 10000 ) {
			ri.Printf (PRINT_ALL, "ScreenShot: Couldn't create a file\n"); 
			return;
 		}

		lastNumber++;
	}

	R_TakeScreenshot( 0, 0, glConfig.vidWidth, glConfig.vidHeight, checkname, qtrue );

	if ( !silent ) {
		ri.Printf (PRINT_ALL, "Wrote %s\n", checkname);
	}
} 

//============================================================================

/*
** GL_SetDefaultState
*/
void GL_SetDefaultState( void )
{
	qglClearDepth( 1.0f );

	qglCullFace(GL_FRONT);

	qglColor4f (1,1,1,1);

	// initialize downstream texture unit if we're running
	// in a multitexture environment
	if ( qglActiveTextureARB ) {
		GL_SelectTexture( 1 );
		GL_TextureMode( r_textureMode->string );
		GL_TexEnv( GL_MODULATE );
		qglDisable( GL_TEXTURE_2D );
		GL_SelectTexture( 0 );
	}

	qglEnable(GL_TEXTURE_2D);
	GL_TextureMode( r_textureMode->string );
	GL_TexEnv( GL_MODULATE );

	qglShadeModel( GL_SMOOTH );
	qglDepthFunc( GL_LEQUAL );

	// the vertex array is always enabled, but the color and texture
	// arrays are enabled and disabled around the compiled vertex array call
	qglEnableClientState (GL_VERTEX_ARRAY);

	//
	// make sure our GL state vector is set correctly
	//
	glState.glStateBits = GLS_DEPTHTEST_DISABLE | GLS_DEPTHMASK_TRUE;

	qglPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
	qglDepthMask( GL_TRUE );
	qglDisable( GL_DEPTH_TEST );
	qglEnable( GL_SCISSOR_TEST );
	qglDisable( GL_CULL_FACE );
	qglDisable( GL_BLEND );

	qglFogi(GL_FOG_MODE, GL_LINEAR);

	glState.fFogColor[0] = 0.0;
	glState.fFogColor[1] = 0.0;
	glState.fFogColor[2] = 0.0;
	glState.fFogColor[3] = 1.0;
}

const char* RE_GetGraphicsInfo() {
	// FIXME: unimplemented
	return NULL;
}

/*
================
GfxInfo_f
================
*/
void GfxInfo_f( void ) 
{
	cvar_t *sys_cpustring = ri.Cvar_Get( "sys_cpustring", "", 0 );
	const char *enablestrings[] =
	{
		"disabled",
		"enabled"
	};
	const char *fsstrings[] =
	{
		"windowed",
		"fullscreen"
	};

	ri.Printf( PRINT_ALL, "\nGL_VENDOR: %s\n", glConfig.vendor_string );
	ri.Printf( PRINT_ALL, "GL_RENDERER: %s\n", glConfig.renderer_string );
	ri.Printf( PRINT_ALL, "GL_VERSION: %s\n", glConfig.version_string );
	ri.Printf( PRINT_ALL, "GL_EXTENSIONS: %s\n", glConfig.extensions_string );
	ri.Printf( PRINT_ALL, "GL_MAX_TEXTURE_SIZE: %d\n", glConfig.maxTextureSize );
	//ri.Printf( PRINT_ALL, "GL_MAX_ACTIVE_TEXTURES_ARB: %d\n", glConfig.maxActiveTextures );
	ri.Printf( PRINT_ALL, "\nPIXELFORMAT: color(%d-bits) Z(%d-bit) stencil(%d-bits)\n", glConfig.colorBits, glConfig.depthBits, glConfig.stencilBits );
	ri.Printf( PRINT_ALL, "MODE: %d, %d x %d %s hz:", r_mode->integer, glConfig.vidWidth, glConfig.vidHeight, fsstrings[r_fullscreen->integer == 1] );
	if ( glConfig.displayFrequency )
	{
		ri.Printf( PRINT_ALL, "%d\n", glConfig.displayFrequency );
	}
	else
	{
		ri.Printf( PRINT_ALL, "N/A\n" );
	}
	if ( glConfig.deviceSupportsGamma )
	{
		ri.Printf( PRINT_ALL, "GAMMA: hardware w/ %d overbright bits\n", tr.overbrightBits );
	}
	else
	{
		ri.Printf( PRINT_ALL, "GAMMA: software w/ %d overbright bits\n", tr.overbrightBits );
	}
	ri.Printf( PRINT_ALL, "CPU: %s\n", sys_cpustring->string );

	// rendering primitives
	{
		int		primitives;

		// default is to use triangles if compiled vertex arrays are present
		ri.Printf( PRINT_ALL, "rendering primitives: " );
		primitives = r_primitives->integer;
		if ( primitives == 0 ) {
			if ( qglLockArraysEXT ) {
				primitives = 2;
			} else {
				primitives = 1;
			}
		}
		if ( primitives == -1 ) {
			ri.Printf( PRINT_ALL, "none\n" );
		} else if ( primitives == 2 ) {
			ri.Printf( PRINT_ALL, "single glDrawElements\n" );
		} else if ( primitives == 1 ) {
			ri.Printf( PRINT_ALL, "multiple glArrayElement\n" );
		} else if ( primitives == 3 ) {
			ri.Printf( PRINT_ALL, "multiple glColor4ubv + glTexCoord2fv + glVertex3fv\n" );
		}
	}

	ri.Printf( PRINT_ALL, "texturemode: %s\n", r_textureMode->string );
	ri.Printf( PRINT_ALL, "picmip: %d\n", r_picmip->integer );
	ri.Printf( PRINT_ALL, "texture bits: %d\n", r_texturebits->integer );
	ri.Printf( PRINT_ALL, "multitexture: %s\n", enablestrings[qglActiveTextureARB != 0] );
	ri.Printf( PRINT_ALL, "compiled vertex arrays: %s\n", enablestrings[qglLockArraysEXT != 0 ] );
	ri.Printf( PRINT_ALL, "texenv add: %s\n", enablestrings[glConfig.textureEnvAddAvailable != 0] );
	ri.Printf( PRINT_ALL, "compressed textures: %s\n", enablestrings[glConfig.textureCompression!=TC_NONE] );
	if ( r_vertexLight->integer || glConfig.hardwareType == GLHW_PERMEDIA2 )
	{
		ri.Printf( PRINT_ALL, "HACK: using vertex lightmap approximation\n" );
	}
	if ( glConfig.hardwareType == GLHW_RAGEPRO )
	{
		ri.Printf( PRINT_ALL, "HACK: ragePro approximations\n" );
	}
	if ( glConfig.hardwareType == GLHW_RIVA128 )
	{
		ri.Printf( PRINT_ALL, "HACK: riva128 approximations\n" );
	}
	if ( glConfig.smpActive ) {
		ri.Printf( PRINT_ALL, "Using dual processor acceleration\n" );
	}
	if ( r_finish->integer ) {
		ri.Printf( PRINT_ALL, "Forcing glFinish\n" );
	}
}

qboolean R_SetMode(int mode, const glconfig_t* glConfig) {
	// FIXME: unimplemented
	return qfalse;
}

void R_SetFullscreen(qboolean fullscreen) {
	// FIXME: unimplemented
}

/*
===============
R_Register
===============
*/
void R_Register( void ) 
{
	//
	// latched and archived variables
	//
	r_glDriver = ri.Cvar_Get( "r_glDriver", OPENGL_DRIVER_NAME, CVAR_ARCHIVE | CVAR_LATCH );
	r_allowExtensions = ri.Cvar_Get( "r_allowExtensions", "1", CVAR_ARCHIVE | CVAR_LATCH );
	r_ext_compressed_textures = ri.Cvar_Get( "r_ext_compressed_textures", "0", CVAR_ARCHIVE | CVAR_LATCH );
	r_ext_gamma_control = ri.Cvar_Get( "r_ext_gamma_control", "1", CVAR_ARCHIVE | CVAR_LATCH );
	r_ext_multitexture = ri.Cvar_Get( "r_ext_multitexture", "1", CVAR_ARCHIVE | CVAR_LATCH );
	r_ext_compiled_vertex_array = ri.Cvar_Get( "r_ext_compiled_vertex_array", "1", CVAR_ARCHIVE | CVAR_LATCH);
#ifdef __linux__ // broken on linux
	r_ext_texture_env_add = ri.Cvar_Get( "r_ext_texture_env_add", "0", CVAR_ARCHIVE | CVAR_LATCH);
#else
	r_ext_texture_env_add = ri.Cvar_Get( "r_ext_texture_env_add", "1", CVAR_ARCHIVE | CVAR_LATCH);
#endif
	r_ext_texture_env_combine = ri.Cvar_Get("r_ext_texture_env_combine", "0", CVAR_ARCHIVE | CVAR_LATCH);
    r_ext_aniso_filter = ri.Cvar_Get("r_ext_aniso_filter", "0", CVAR_ARCHIVE | CVAR_LATCH);
    r_forceClampToEdge = ri.Cvar_Get("r_forceClampToEdge", "0", CVAR_ARCHIVE | CVAR_LATCH);
    r_geForce3WorkAround = ri.Cvar_Get("r_geForce3WorkAround", "1", CVAR_ARCHIVE);
    r_reset_tc_array = ri.Cvar_Get("r_reset_tc_array", "1", CVAR_ARCHIVE);

	r_picmip = ri.Cvar_Get ("r_picmip", "1", CVAR_ARCHIVE | CVAR_LATCH );
	r_roundImagesDown = ri.Cvar_Get ("r_roundImagesDown", "1", CVAR_ARCHIVE | CVAR_LATCH );
	r_colorMipLevels = ri.Cvar_Get ("r_colorMipLevels", "0", CVAR_LATCH );
	AssertCvarRange( r_picmip, 0, 16, qtrue );
	r_textureDetails = ri.Cvar_Get("r_textureDetails", "1", 33);
	r_texturebits = ri.Cvar_Get( "r_texturebits", "0", CVAR_ARCHIVE | CVAR_LATCH );
	r_colorbits = ri.Cvar_Get( "r_colorbits", "0", CVAR_ARCHIVE | CVAR_LATCH );
	r_stereo = ri.Cvar_Get( "r_stereo", "0", CVAR_ARCHIVE | CVAR_LATCH );
#ifdef __linux__
	r_stencilbits = ri.Cvar_Get( "r_stencilbits", "0", CVAR_ARCHIVE | CVAR_LATCH );
#else
	r_stencilbits = ri.Cvar_Get( "r_stencilbits", "8", CVAR_ARCHIVE | CVAR_LATCH );
#endif
    r_depthbits = ri.Cvar_Get("r_depthbits", "0", CVAR_ARCHIVE | CVAR_LATCH);
	r_overBrightBits = ri.Cvar_Get ("r_overBrightBits", "0", CVAR_ARCHIVE | CVAR_LATCH );
	r_ignorehwgamma = ri.Cvar_Get( "r_ignorehwgamma", "0", CVAR_ARCHIVE | CVAR_LATCH);
	r_mode = ri.Cvar_Get( "r_mode", "3", CVAR_ARCHIVE | CVAR_LATCH );
    r_fullscreen = ri.Cvar_Get("r_fullscreen", "1", CVAR_ARCHIVE | CVAR_LATCH);
	r_customwidth = ri.Cvar_Get( "r_customwidth", "1600", CVAR_ARCHIVE | CVAR_LATCH );
	r_customheight = ri.Cvar_Get( "r_customheight", "1024", CVAR_ARCHIVE | CVAR_LATCH );
	r_customaspect = ri.Cvar_Get( "r_customaspect", "1", CVAR_ARCHIVE | CVAR_LATCH );
    r_vertexLight = ri.Cvar_Get("r_vertexLight", "0", CVAR_ARCHIVE | CVAR_LATCH);
#if (defined(MACOS_X) || defined(__linux__)) && defined(SMP)
  // Default to using SMP on Mac OS X or Linux if we have multiple processors
	r_smp = ri.Cvar_Get( "r_smp", Sys_ProcessorCount() > 1 ? "1" : "0", CVAR_ARCHIVE | CVAR_LATCH);
#else        
	r_smp = ri.Cvar_Get( "r_smp", "0", CVAR_ARCHIVE | CVAR_LATCH);
#endif
    r_ignoreFastPath = ri.Cvar_Get("r_ignoreFastPath", "0", CVAR_ARCHIVE | CVAR_LATCH);
    r_subdivisions = ri.Cvar_Get("r_subdivisions", "4", CVAR_ARCHIVE | CVAR_LATCH);

	//
	// temporary latched variables that can only change over a restart
	//
	r_displayRefresh = ri.Cvar_Get( "r_displayRefresh", "0", CVAR_LATCH );
	AssertCvarRange( r_displayRefresh, 0, 200, qtrue );
	r_fullbright = ri.Cvar_Get ("r_fullbright", "0", CVAR_LATCH|CVAR_CHEAT );
	r_mapOverBrightBits = ri.Cvar_Get ("r_mapOverBrightBits", "1", CVAR_LATCH );
	r_intensity = ri.Cvar_Get ("r_intensity", "1", CVAR_LATCH );
	r_numdebuglines = ri.Cvar_Get("g_numdebuglines", "4096", CVAR_LATCH);
	r_singleShader = ri.Cvar_Get ("r_singleShader", "0", CVAR_CHEAT | CVAR_LATCH );

	//
	// archived variables that can change at any time
	//
	r_lerpmodels = ri.Cvar_Get("r_lerpmodels", "1", 0);
	r_lodCurveError = ri.Cvar_Get( "r_lodCurveError", "250", CVAR_ARCHIVE|CVAR_CHEAT );
	r_flares = ri.Cvar_Get ("r_flares", "0", CVAR_ARCHIVE );
	r_znear = ri.Cvar_Get( "r_znear", "4", CVAR_CHEAT );
	AssertCvarRange( r_znear, 0.001f, 200, qtrue );
	r_ignoreGLErrors = ri.Cvar_Get( "r_ignoreGLErrors", "1", CVAR_ARCHIVE );
	r_fastsky = ri.Cvar_Get( "r_fastsky", "0", CVAR_ARCHIVE );
	r_fastdlights = ri.Cvar_Get("r_fastdlights", "1", CVAR_CHEAT | CVAR_LATCH);
	r_drawSun = ri.Cvar_Get( "r_drawSun", "0", CVAR_ARCHIVE );
	r_dlightBacks = ri.Cvar_Get( "r_dlightBacks", "1", CVAR_ARCHIVE );
	r_finish = ri.Cvar_Get ("r_finish", "0", CVAR_ARCHIVE);
	r_textureMode = ri.Cvar_Get( "r_textureMode", "GL_LINEAR_MIPMAP_NEAREST", CVAR_ARCHIVE );
	r_swapInterval = ri.Cvar_Get( "r_swapInterval", "0", CVAR_ARCHIVE );
#ifdef __MACOS__
	r_gamma = ri.Cvar_Get( "r_gamma", "1.2", CVAR_ARCHIVE );
#else
	r_gamma = ri.Cvar_Get( "r_gamma", "1", CVAR_ARCHIVE );
#endif
	r_facePlaneCull = ri.Cvar_Get ("r_facePlaneCull", "1", CVAR_ARCHIVE );

	r_primitives = ri.Cvar_Get( "r_primitives", "0", CVAR_ARCHIVE );

	r_ambientScale = ri.Cvar_Get( "r_ambientScale", "0.6", CVAR_CHEAT );
	r_directedScale = ri.Cvar_Get( "r_directedScale", "1", CVAR_CHEAT );

	//
	// temporary variables that can change at any time
	//
	r_showImages = ri.Cvar_Get( "r_showImages", "0", CVAR_TEMP );
    r_showlod = ri.Cvar_Get("r_showlod", "0", CVAR_TEMP);
    r_showstaticlod = ri.Cvar_Get("r_showstaticlod", "0", CVAR_TEMP);
    r_uselod = ri.Cvar_Get("r_uselod", "1", CVAR_TEMP);
    lod_LOD = ri.Cvar_Get("lod_LOD", "0", CVAR_TEMP);
    lod_minLOD = ri.Cvar_Get("lod_minLOD", "1.0", CVAR_TEMP);
    lod_maxLOD = ri.Cvar_Get("lod_maxLOD", "0.3", CVAR_TEMP);
    lod_LOD_slider = ri.Cvar_Get("lod_LOD_slider", "0.5", CVAR_TEMP);
    lod_edit_0 = ri.Cvar_Get("lod_edit_0", "0", CVAR_TEMP);
    lod_edit_1 = ri.Cvar_Get("lod_edit_1", "0", CVAR_TEMP);
    lod_edit_2 = ri.Cvar_Get("lod_edit_2", "0", CVAR_TEMP);
    lod_edit_3 = ri.Cvar_Get("lod_edit_3", "0", CVAR_TEMP);
    lod_edit_4 = ri.Cvar_Get("lod_edit_4", "0", CVAR_TEMP);
    lod_curve_0_val = ri.Cvar_Get("lod_curve_0_val", "0", CVAR_TEMP);
    lod_curve_1_val = ri.Cvar_Get("lod_curve_1_val", "0", CVAR_TEMP);
    lod_curve_2_val = ri.Cvar_Get("lod_curve_2_val", "0", CVAR_TEMP);
    lod_curve_3_val = ri.Cvar_Get("lod_curve_3_val", "0", CVAR_TEMP);
    lod_curve_4_val = ri.Cvar_Get("lod_curve_4_val", "0", CVAR_TEMP);
    lod_curve_0_slider = ri.Cvar_Get("lod_curve_0_slider", "0", CVAR_TEMP);
    lod_curve_1_slider = ri.Cvar_Get("lod_curve_1_slider", "0", CVAR_TEMP);
    lod_curve_2_slider = ri.Cvar_Get("lod_curve_2_slider", "0", CVAR_TEMP);
    lod_curve_3_slider = ri.Cvar_Get("lod_curve_3_slider", "0", CVAR_TEMP);
    lod_curve_4_slider = ri.Cvar_Get("lod_curve_4_slider", "0", CVAR_TEMP);
    lod_pitch_val = ri.Cvar_Get("lod_pitch_val", "0", CVAR_TEMP);
    lod_zee_val = ri.Cvar_Get("lod_zee_val", "0", CVAR_TEMP);
    lod_mesh = ri.Cvar_Get("lod_mesh", "0", CVAR_TEMP);
    lod_meshname = ri.Cvar_Get("lod_meshname", "", CVAR_TEMP);
    lod_tikiname = ri.Cvar_Get("lod_tikiname", "", CVAR_TEMP);
    lod_metric = ri.Cvar_Get("lod_metric", "0.0", CVAR_TEMP);
    lod_tris = ri.Cvar_Get("lod_tris", "", CVAR_TEMP);
    lod_save = ri.Cvar_Get("lod_save", "0", CVAR_TEMP);
    lod_position = ri.Cvar_Get("lod_position", "0 0 0", CVAR_TEMP);
    lod_tool = ri.Cvar_Get("lod_tool", "0", CVAR_TEMP);

	r_debugSort = ri.Cvar_Get( "r_debugSort", "0", CVAR_CHEAT );
	r_printShaders = ri.Cvar_Get( "r_printShaders", "0", 0 );

	r_nocurves = ri.Cvar_Get ("r_nocurves", "0", CVAR_CHEAT );
	r_drawworld = ri.Cvar_Get ("r_drawworld", "1", CVAR_CHEAT );
	r_lightmap = ri.Cvar_Get ("r_lightmap", "0", 0 );
	r_portalOnly = ri.Cvar_Get ("r_portalOnly", "0", CVAR_CHEAT );

	r_flareSize = ri.Cvar_Get ("r_flareSize", "40", CVAR_CHEAT);
	r_flareFade = ri.Cvar_Get ("r_flareFade", "7", CVAR_CHEAT);

	r_showSmp = ri.Cvar_Get ("r_showSmp", "0", CVAR_CHEAT);
	r_skipBackEnd = ri.Cvar_Get ("r_skipBackEnd", "0", CVAR_CHEAT);

	r_measureOverdraw = ri.Cvar_Get( "r_measureOverdraw", "0", CVAR_CHEAT );
    r_norefresh = ri.Cvar_Get("r_norefresh", "0", CVAR_CHEAT);
    r_drawentities = ri.Cvar_Get("r_drawentities", "1", CVAR_CHEAT);
    r_drawentitypoly = ri.Cvar_Get("r_drawentitypoly", "1", CVAR_CHEAT);
    r_drawstaticmodels = ri.Cvar_Get("r_drawstaticmodels", "1", CVAR_CHEAT);
    r_drawstaticmodelpoly = ri.Cvar_Get("r_drawstaticmodelpoly", "1", CVAR_CHEAT);
    r_drawbrushes = ri.Cvar_Get("r_drawbrushes", "1", CVAR_CHEAT);
    r_drawbrushmodels = ri.Cvar_Get("r_drawbrushmodels", "1", CVAR_CHEAT);
    r_drawstaticdecals = ri.Cvar_Get("r_drawstaticdecals", "0", 0);
    r_drawterrain = ri.Cvar_Get("r_drawterrain", "1", CVAR_CHEAT);
    r_drawsprites = ri.Cvar_Get("r_drawsprites", "1", CVAR_CHEAT);
    r_drawspherelights = ri.Cvar_Get("r_drawspherelights", "1", CVAR_CHEAT);
    r_staticlod = ri.Cvar_Get("r_staticlod", "1", CVAR_CHEAT);
    r_lodscale = ri.Cvar_Get("r_lodscale", "5", CVAR_ARCHIVE);
    r_lodcap = ri.Cvar_Get("r_lodcap", "0.35", CVAR_ARCHIVE);
    r_lodviewmodelcap = ri.Cvar_Get("r_lodviewmodelcap", "0.25", CVAR_ARCHIVE);
	r_ignore = ri.Cvar_Get( "r_ignore", "1", CVAR_CHEAT );
	r_nocull = ri.Cvar_Get ("r_nocull", "0", CVAR_CHEAT);
	r_showcull = ri.Cvar_Get("r_showcull", "0", CVAR_CHEAT);
	r_novis = ri.Cvar_Get ("r_novis", "0", CVAR_CHEAT);
	r_showcluster = ri.Cvar_Get ("r_showcluster", "0", CVAR_CHEAT);
	r_speeds = ri.Cvar_Get ("r_speeds", "0", CVAR_CHEAT);
	r_verbose = ri.Cvar_Get( "r_verbose", "0", CVAR_CHEAT );
	r_logFile = ri.Cvar_Get( "r_logFile", "0", CVAR_CHEAT );
	r_debugSurface = ri.Cvar_Get ("r_debugSurface", "0", CVAR_CHEAT);
	r_nobind = ri.Cvar_Get ("r_nobind", "0", CVAR_CHEAT);
	r_showtris = ri.Cvar_Get ("r_showtris", "0", CVAR_CHEAT);
	r_showsky = ri.Cvar_Get ("r_showsky", "0", CVAR_CHEAT);
	r_shownormals = ri.Cvar_Get ("r_shownormals", "0", CVAR_CHEAT);
    r_showhbox = ri.Cvar_Get("r_showhbox", "0", CVAR_CHEAT);
    r_showstaticbboxes = ri.Cvar_Get("r_showstaticbboxes", "0", CVAR_CHEAT);
	r_clear = ri.Cvar_Get ("r_clear", "0", CVAR_CHEAT);
	r_offsetFactor = ri.Cvar_Get( "r_offsetfactor", "-1", CVAR_CHEAT );
	r_offsetUnits = ri.Cvar_Get( "r_offsetunits", "-2", CVAR_CHEAT );
	r_drawBuffer = ri.Cvar_Get( "r_drawBuffer", "GL_BACK", CVAR_CHEAT );
	r_lockpvs = ri.Cvar_Get ("r_lockpvs", "0", CVAR_CHEAT);
	r_noportals = ri.Cvar_Get ("r_noportals", "0", CVAR_CHEAT);
    r_entlightmap = ri.Cvar_Get("r_entlightmap", "0", CVAR_CHEAT);
    r_fastentlight = ri.Cvar_Get("r_fastentlight", "1", CVAR_ARCHIVE);
    r_entlight_scale = ri.Cvar_Get("r_entlight_scale", "1.3", CVAR_CHEAT);
    r_entlight_errbound = ri.Cvar_Get("r_entlight_errbound", "6", CVAR_ARCHIVE);
    r_entlight_cubelevel = ri.Cvar_Get("r_entlight_cubelevel", "0", CVAR_ARCHIVE);
    r_entlight_cubefraction = ri.Cvar_Get("r_entlight_cubefraction", "0.5", CVAR_ARCHIVE);
    r_entlight_maxcalc = ri.Cvar_Get("r_entlight_maxcalc", "2", CVAR_ARCHIVE);
	r_shadows = ri.Cvar_Get( "cg_shadows", "1", 0 );

	r_maxpolys = ri.Cvar_Get( "r_maxpolys", va("%d", MAX_POLYS), 0);
	r_maxpolyverts = ri.Cvar_Get( "r_maxpolyverts", va("%d", MAX_POLYVERTS), 0);
	r_maxtermarks = ri.Cvar_Get("r_maxtermarks", va("%d", MAX_TERMARKS), 0);

    r_stipplelines = ri.Cvar_Get("r_stipplelines", "1", CVAR_ARCHIVE);
    r_light_lines = ri.Cvar_Get("r_light_lines", "0", CVAR_CHEAT);
    r_light_sun_line = ri.Cvar_Get("r_light_sun_line", "0", CVAR_CHEAT);
    r_light_int_scale = ri.Cvar_Get("r_light_int_scale", "0.05", CVAR_ARCHIVE);
    r_light_nolight = ri.Cvar_Get("r_light_nolight", "0", CVAR_ARCHIVE);
    r_light_showgrid = ri.Cvar_Get("r_light_showgrid", "0", CVAR_CHEAT);
    r_skyportal = ri.Cvar_Get("r_skyportal", "0", 0);
    r_skyportal_origin = ri.Cvar_Get("r_skyportal_origin", "0 0 0", 0);
    r_farplane = ri.Cvar_Get("r_farplane", "0", CVAR_CHEAT);
    r_farplane_color = ri.Cvar_Get("r_farplane_color", ".5 .5 .5", CVAR_CHEAT);
    r_farplane_nocull = ri.Cvar_Get("r_farplane_nocull", "0", CVAR_CHEAT);
    r_farplane_nofog = ri.Cvar_Get("r_farplane_nofog", "0", CVAR_CHEAT);
    r_skyportal = ri.Cvar_Get("r_skyportal", "0", 0);
    r_skyportal_origin = ri.Cvar_Get("r_skyportal_origin", "0 0 0", 0);
    r_lightcoronasize = ri.Cvar_Get("r_lightcoronasize", ".1", CVAR_ARCHIVE);
    r_useglfog = ri.Cvar_Get("r_useglfog", "1", 64);
    r_debuglines_depthmask = ri.Cvar_Get("r_debuglines_depthmask", "0", CVAR_ARCHIVE);
    r_smoothsmokelight = ri.Cvar_Get("vss_smoothsmokelight", "1", CVAR_ARCHIVE);
    r_showportal = ri.Cvar_Get("r_showportal", "0", 0);
    sys_cpuid = ri.Cvar_Get("sys_cpuid", "0", 0);
    r_sse = ri.Cvar_Get("r_sse", "0", CVAR_ARCHIVE);
    r_static_shaderdata0 = ri.Cvar_Get("r_static_shaderdata0", "0", CVAR_SYSTEMINFO);
    r_static_shaderdata1 = ri.Cvar_Get("r_static_shaderdata1", "0", CVAR_SYSTEMINFO);
    r_static_shaderdata2 = ri.Cvar_Get("r_static_shaderdata2", "0", CVAR_SYSTEMINFO);
    r_static_shaderdata3 = ri.Cvar_Get("r_static_shaderdata3", "0", CVAR_SYSTEMINFO);
    r_static_shadermultiplier0 = ri.Cvar_Get("r_static_shadermultiplier0", "1", CVAR_SYSTEMINFO);
    r_static_shadermultiplier1 = ri.Cvar_Get("r_static_shadermultiplier1", "1", CVAR_SYSTEMINFO);
    r_static_shadermultiplier2 = ri.Cvar_Get("r_static_shadermultiplier2", "1", CVAR_SYSTEMINFO);
    r_static_shadermultiplier3 = ri.Cvar_Get("r_static_shadermultiplier3", "1", CVAR_SYSTEMINFO);
    r_precacheimages = (cvar_t*)Cvar_Get("r_precacheimages", "0", 0);
    ter_minMarkRadius = ri.Cvar_Get("ter_minMarkRadius", "8", CVAR_ARCHIVE);
    ter_fastMarks = ri.Cvar_Get("ter_fastMarks", "1", CVAR_ARCHIVE);
    fps = ri.Cvar_Get("fps", "0", 0);
    r_loadjpg = ri.Cvar_Get("r_loadjpg", "1", CVAR_LATCH);
    r_loadftx = ri.Cvar_Get("r_loadftx", "0", CVAR_LATCH);

    r_ext_multisample = ri.Cvar_Get("r_ext_multisample", "0", CVAR_ARCHIVE | CVAR_LATCH);
    r_noborder = ri.Cvar_Get("r_noborder", "0", CVAR_ARCHIVE | CVAR_LATCH);
    r_ext_texture_filter_anisotropic = ri.Cvar_Get("r_ext_texture_filter_anisotropic",
        "0", CVAR_ARCHIVE | CVAR_LATCH);
    r_stereoEnabled = ri.Cvar_Get("r_stereoEnabled", "0", CVAR_ARCHIVE | CVAR_LATCH);

	// make sure all the commands added here are also
	// removed in R_Shutdown
	ri.Cmd_AddCommand( "imagelist", R_ImageList_f );
	ri.Cmd_AddCommand( "shaderlist", R_ShaderList_f );
	ri.Cmd_AddCommand( "skinlist", R_SkinList_f );
	ri.Cmd_AddCommand( "modellist", R_Modellist_f );
	ri.Cmd_AddCommand( "modelist", R_ModeList_f );
	ri.Cmd_AddCommand( "screenshot", R_ScreenShot_f );
	ri.Cmd_AddCommand( "screenshotJPEG", R_ScreenShotJPEG_f );
	ri.Cmd_AddCommand( "gfxinfo", GfxInfo_f );
}

void R_InitExtensions() {
	// FIXME: qglTextureEnvCombineExists
	glState.cntTexEnvExt = 0;
	glState.cntnvblendmode = 0;

	qglTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB, 34165.0);
	qglTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, 8448.0);
	qglTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_RGB, 0x47057700);
	qglTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_RGB, 0x44400000);
	qglTexEnvf(GL_TEXTURE_ENV, GL_SOURCE1_RGB, 5890.0);
	qglTexEnvf(GL_TEXTURE_ENV, GL_OPERAND1_RGB, 0x44400000);
	qglTexEnvf(GL_TEXTURE_ENV, GL_SOURCE2_RGB, 0x47057700);
	qglTexEnvf(GL_TEXTURE_ENV, GL_OPERAND2_RGB, 770.0);
}

/*
===============
R_Init
===============
*/
void R_Init( void ) {	
	int	err;
	int i;
	byte *ptr;

	ri.Printf( PRINT_ALL, "----- R_Init -----\n" );

	// clear all our internal state
	Com_Memset( &tr, 0, sizeof( tr ) );
	Com_Memset( &backEnd, 0, sizeof( backEnd ) );
	Com_Memset( &tess, 0, sizeof( tess ) );

//	Swap_Init();

	if ( (int)tess.xyz & 15 ) {
		Com_Printf( "WARNING: tess.xyz not 16 byte aligned\n" );
	}
	Com_Memset( tess.constantColor255, 255, sizeof( tess.constantColor255 ) );

	tr.worldEntity.e.shaderRGBA[0] = -1;
	tr.worldEntity.e.shaderRGBA[1] = 0;
	tr.worldEntity.e.shaderRGBA[2] = -1;
	tr.worldEntity.e.shaderRGBA[3] = -1;

	//
	// init function tables
	//
	for ( i = 0; i < FUNCTABLE_SIZE; i++ )
	{
		tr.sinTable[i]		= sin( DEG2RAD( i * 360.0f / ( ( float ) ( FUNCTABLE_SIZE - 1 ) ) ) );
		tr.squareTable[i]	= ( i < FUNCTABLE_SIZE/2 ) ? 1.0f : -1.0f;
		tr.sawToothTable[i] = (float)i / FUNCTABLE_SIZE;
		tr.inverseSawToothTable[i] = 1.0f - tr.sawToothTable[i];

		if ( i < FUNCTABLE_SIZE / 2 )
		{
			if ( i < FUNCTABLE_SIZE / 4 )
			{
				tr.triangleTable[i] = ( float ) i / ( FUNCTABLE_SIZE / 4 );
			}
			else
			{
				tr.triangleTable[i] = 1.0f - tr.triangleTable[i-FUNCTABLE_SIZE / 4];
			}
		}
		else
		{
			tr.triangleTable[i] = -tr.triangleTable[i-FUNCTABLE_SIZE/2];
		}
	}

	tr.rendererhandle = ri.Milliseconds();

	if (!tr.rendererhandle) {
		tr.rendererhandle = -1;
	}

	R_NoiseInit();

	R_Sky_Init();

	R_Register();

	max_polys = r_maxpolys->integer;
	if (max_polys < MAX_POLYS)
		max_polys = MAX_POLYS;

	max_polyverts = r_maxpolyverts->integer;
	if (max_polyverts < MAX_POLYVERTS)
        max_polyverts = MAX_POLYVERTS;

	max_termarks = r_maxtermarks->integer;
    if (max_termarks < MAX_TERMARKS)
		max_termarks = MAX_TERMARKS;

    ptr = ri.Malloc(sizeof(*backEndData[0]) + sizeof(srfPoly_t) * max_polys + sizeof(polyVert_t) * max_polyverts + sizeof(srfMarkFragment_t) * max_termarks);
    backEndData[0] = (backEndData_t*)ptr;
    backEndData[0]->polys = (srfPoly_t*)((char*)ptr + sizeof(*backEndData[0]));
    backEndData[0]->polyVerts = (polyVert_t*)((char*)ptr + sizeof(*backEndData[0]) + sizeof(srfPoly_t) * max_polys);
    backEndData[0]->terMarks = (srfMarkFragment_t*)((char*)ptr + sizeof(*backEndData[0]) + sizeof(srfPoly_t) * max_polys + sizeof(polyVert_t) * max_polyverts);
    backEndData[0]->staticModels = NULL;
    backEndData[0]->staticModelData = NULL;
    if (r_smp->integer) {
        ptr = ri.Malloc(sizeof(*backEndData[1]) + sizeof(srfPoly_t) * max_polys + sizeof(polyVert_t) * max_polyverts);
        backEndData[1] = (backEndData_t*)ptr;
        backEndData[1]->polys = (srfPoly_t*)((char*)ptr + sizeof(*backEndData[1]));
        backEndData[1]->polyVerts = (polyVert_t*)((char*)ptr + sizeof(*backEndData[1]) + sizeof(srfPoly_t) * max_polys);
        backEndData[1]->terMarks = (srfMarkFragment_t*)((char*)ptr + sizeof(*backEndData[1]) + sizeof(srfPoly_t) * max_polys + sizeof(polyVert_t) * max_polyverts);
        backEndData[1]->staticModels = NULL;
		backEndData[1]->staticModelData = NULL;
    }
    else {
        backEndData[1] = NULL;
    }
	R_ToggleSmpFrame();

	InitOpenGL();

	R_InitExtensions();

	R_InitImages();

	R_StartupShaders();

	R_ModelInit();

	R_LevelMarksInit();

	tr.pFontDebugStrings = R_LoadFont("verdana-14");
	g_bInfoworldtris = qfalse;

	err = qglGetError();
	if ( err != GL_NO_ERROR )
		ri.Printf (PRINT_ALL, "glGetError() = 0x%x\n", err);

	ri.Printf( PRINT_ALL, "----- finished R_Init -----\n" );
}

/*
===============
RE_Shutdown
===============
*/
void RE_Shutdown( qboolean destroyWindow ) {	

	ri.Printf( PRINT_ALL, "RE_Shutdown( %i )\n", destroyWindow );

	ri.Cmd_RemoveCommand ("modellist");
	ri.Cmd_RemoveCommand ("screenshotJPEG");
	ri.Cmd_RemoveCommand ("screenshot");
	ri.Cmd_RemoveCommand ("imagelist");
	ri.Cmd_RemoveCommand ("shaderlist");
	ri.Cmd_RemoveCommand ("skinlist");
	ri.Cmd_RemoveCommand ("gfxinfo");
	ri.Cmd_RemoveCommand( "modelist" );
	ri.Cmd_RemoveCommand( "shaderstate" );


	if ( tr.registered ) {
		R_SyncRenderThread();
		R_ShutdownCommandBuffers();
		R_DeleteTextures();
	}

	// shut down platform specific OpenGL stuff
	if ( destroyWindow ) {
		GLimp_Shutdown();
	}

	tr.registered = qfalse;
}

extern qboolean scr_initialized;

/*
** RE_BeginRegistration
*/
void RE_BeginRegistration(glconfig_t* glconfigOut) {

    UI_LoadResource("*123");
    scr_initialized = 0;

    R_SyncRenderThread();

    R_LevelMarksFree();
    ri.Hunk_Clear();

    *glconfigOut = glConfig;

    r_sequencenumber++;
    tr.viewCluster = -1;		// force markleafs to regenerate
    R_ClearFlares();
    RE_ClearScene();
    R_SetupShaders();
    R_InitLensFlare();
    R_LevelMarksInit();

    tr.registered = qtrue;

    scr_initialized = 1;
    UI_LoadResource("*124");
}

/*
=============
RE_EndRegistration

Touch all images to make sure they are resident
=============
*/
void RE_EndRegistration( void ) {
	R_SyncRenderThread();
	if (!Sys_LowPhysicalMemory()) {
		RB_ShowImages();
	}
}

void RE_SetRenderTime(int t) {
	// FIXME: unimplemented
}

/*
@@@@@@@@@@@@@@@@@@@@@
GetRefAPI

@@@@@@@@@@@@@@@@@@@@@
*/
refexport_t *GetRefAPI ( int apiVersion, refimport_t *rimp ) {
	static refexport_t	re;

	ri = *rimp;

	Com_Memset( &re, 0, sizeof( re ) );

	if ( apiVersion != REF_API_VERSION ) {
		ri.Printf(PRINT_ALL, "Mismatched REF_API_VERSION: expected %i, got %i\n", 
			REF_API_VERSION, apiVersion );
		return NULL;
	}

	R_Init();

	// the RE_ functions are Renderer Entry points

    re.Shutdown = RE_Shutdown;

    re.FreeModels = RE_FreeModels;
    re.BeginRegistration = RE_BeginRegistration;
    re.EndRegistration = RE_EndRegistration;
    re.RegisterModel = RE_RegisterModel;
    re.SpawnEffectModel = RE_SpawnEffectModel;
    re.RegisterServerModel = RE_RegisterServerModel;
    re.UnregisterServerModel = RE_UnregisterServerModel;
    re.RegisterShader = RE_RegisterShader;
    re.RegisterShaderNoMip = RE_RegisterShaderNoMip;
	re.RefreshShaderNoMip = RE_RefreshShaderNoMip;
    re.LoadWorld = RE_LoadWorldMap;
    re.PrintBSPFileSizes = RE_PrintBSPFileSizes;
    re.MapVersion = RE_MapVersion;
    re.LoadFont = R_LoadFont;
    re.SetWorldVisData = RE_SetWorldVisData;

    re.BeginFrame = RE_BeginFrame;
    re.EndFrame = RE_EndFrame;

    re.MarkFragments = R_MarkFragments;
    re.MarkFragmentsForInlineModel = R_MarkFragmentsForInlineModel;
    re.GetInlineModelBounds = R_GetInlineModelBounds;
    re.GetLightingForDecal = R_GetLightingForDecal;
    re.GetLightingForSmoke = R_GetLightingForSmoke;
    re.R_GatherLightSources = R_GatherLightSources;
    re.ModelBounds = R_ModelBounds;
    re.ModelRadius = R_ModelRadius;

    re.ClearScene = RE_ClearScene;
    re.AddRefEntityToScene = RE_AddRefEntityToScene;
    re.AddRefSpriteToScene = RE_AddRefSpriteToScene;
    re.AddPolyToScene = RE_AddPolyToScene;
    re.AddTerrainMarkToScene = RE_AddTerrainMarkToScene;
    re.AddLightToScene = RE_AddLightToScene;
    re.RenderScene = RE_RenderScene;
    re.GetRenderEntity = RE_GetRenderEntity;

    re.SavePerformanceCounters = R_SavePerformanceCounters;

    re.R_Model_GetHandle = R_Model_GetHandle;
    re.SetColor = Draw_SetColor;
    re.DrawStretchPic = Draw_StretchPic;
    re.DrawStretchRaw = RE_StretchRaw;
    re.DebugLine = R_DebugLine;
    re.DrawTilePic = Draw_TilePic;
    re.DrawTilePicOffset = Draw_TilePicOffset;
    re.DrawTrianglePic = Draw_TrianglePic;
    re.DrawBox = DrawBox;
    re.AddBox = AddBox;
    re.Set2DWindow = Set2DWindow;
    re.Scissor = RE_Scissor;
    re.DrawLineLoop = DrawLineLoop;
    re.DrawString = R_DrawString;
    re.GetFontHeight = R_GetFontHeight;
    re.GetFontStringWidth = R_GetFontStringWidth;
    re.SwipeBegin = RE_SwipeBegin;
    re.SwipeEnd = RE_SwipeEnd;
    re.SetRenderTime = RE_SetRenderTime;
    re.Noise = R_NoiseGet4f;

    re.SetMode = R_SetMode;
    re.SetFullscreen = R_SetFullscreen;

    re.GetShaderHeight = RE_GetShaderHeight;
    re.GetShaderWidth = RE_GetShaderWidth;
    re.GetGraphicsInfo = RE_GetGraphicsInfo;
    re.ForceUpdatePose = RE_ForceUpdatePose;
    re.TIKI_Orientation = RE_TIKI_Orientation;
    re.TIKI_IsOnGround = RE_TIKI_IsOnGround;
    re.SetFrameNumber = RE_SetFrameNumber;

	return &re;
}
