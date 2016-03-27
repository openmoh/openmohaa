#include "glb_local.h"

#include "renderer/qfx_library.h"
#include "renderer/qfx_log.h"
#include "renderer/qfx_settings.h"
#include "renderer/qfx_opengl.h"
#include "renderer/qfx_renderer.h"
#include "renderer/qfx_shader.h"
#include "renderer/qfx_glprogs.h"

#include "script/centity.h"
#include "script/cplayer.h"

#include "script.h"
#include "scriptmaster.h"
#include "scripttimer.h"

#include "script/vision.h"

#include "img/IMAGE.h"
#include "img/TORUS.h"

#define WANT_OPENGL
#include <ogl/opengl_api.h>

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")

effect_t *effect_list[MAX_EFFECTS];
qboolean render_inited = false;

GLuint m_pBloomBlurShader = 0;
GLuint m_pBloomCombineShader = 0;
GLuint m_pBloomDarkenShader = 0;
GLuint m_uiBlurTexture = 0;
GLuint m_uiDesaturationTexture = 0;
GLuint m_uiInvertTexture = 0;
GLuint m_uiDarkTexture = 0;
GLuint m_uiLightTexture = 0;

/*float vision_fadeTime = 0.0f;
str vision_current;
float vision_currentTime = 0.0f;
float vision_currentTime2 = 0.0f;*/

cvar_t	*r_debug;
cvar_t	*r_dof;
cvar_t	*r_texFilterAniso;
cvar_t	*r_tweakBlurX;
cvar_t	*r_tweakBlurY;
cvar_t	*r_tweakBlurPasses;
cvar_t	*r_glowEnable;
cvar_t	*r_glowQuality;
cvar_t	*r_glowSamples;
cvar_t	*r_ssao;
cvar_t	*r_autoluminance;
cvar_t	*r_fxaa;
cvar_t	*r_test_color;
cvar_t	*r_test_bump;

// Vision variables
// CANNOT BE MODIFIED FROM CONSOLE !

ScriptTimer m_blurTimer( TIMER_NORMAL );
ScriptTimer m_visionTimer( TIMER_GLIDE );

Container< VisionClass * > m_visionVar;

VisionActivator r_anaglyphTweakEnable
(
	"r_anaglyphTweakEnable",
	"0"
);

VisionActivator r_blur
(
	"r_blur",
	"1",
	NULL,
	&m_blurTimer
);

VisionActivator r_distortionEnable
(
	"r_distortionEnable",
	"0"
);

VisionActivator r_filmEnable
(
	"r_filmEnable",
	"0"
);

VisionActivator r_glow
(
	"r_glow",
	"0"
);

VisionVariable r_glowRadius0
(
	"r_glowRadius0",
	"0.0000",
	&r_glow
);

VisionVariable r_glowRadius1
(
	"r_glowRadius1",
	"0.0000",
	&r_glow
);

VisionVariable r_glowBloomCutoff
(
	"r_glowBloomCutoff",
	"0.0000",
	&r_glow
);

VisionVariable r_glowBloomDesaturation
(
	"r_glowBloomDesaturation",
	"0.0000",
	&r_glow
);

VisionVariable r_glowBloomIntensity0
(
	"r_glowBloomIntensity0",
	"0.0000",
	&r_glow
);

VisionVariable r_glowBloomIntensity1
(
	"r_glowBloomIntensity1",
	"0.0000",
	&r_glow
);

VisionVariable r_glowBloomStreakX
(
	"r_glowBloomStreakX",
	"0.0000",
	&r_glow
);

VisionVariable r_glowBloomStreakY
(
	"r_glowBloomStreakY",
	"0.0000",
	&r_glow
);

VisionVariable r_filmContrast
(
	"r_filmContrast",
	"1.0000 1.0000 1.0000",
	&r_filmEnable
);

VisionVariable r_filmBleach
(
	"r_filmBleach",
	"0.0000 0.0000 0.0000",
	&r_filmEnable
);

VisionVariable r_filmBrightness
(
	"r_filmBrightness",
	"0.0000",
	&r_filmEnable
);

VisionVariable r_filmHue
(
	"r_filmHue",
	"0.0000 0.0000 0.0000",
	&r_filmEnable
);

VisionVariable r_filmMidStart
(
	"r_filmMidStart",
	"0.2500",
	&r_filmEnable
);

VisionVariable r_filmMidEnd
(
	"r_filmMidEnd",
	"0.7500",
	&r_filmEnable
);

VisionVariable r_filmDarkTint
(
	"r_filmDarkTint",
	"1.0000 1.0000 1.0000",
	&r_filmEnable
);

VisionVariable r_filmMidTint
(
	"r_filmMidTint",
	"1.0000 1.0000 1.0000",
	&r_filmEnable
);

VisionVariable r_filmLightTint
(
	"r_filmLightTint",
	"1.0000 1.0000 1.0000",
	&r_filmEnable
);

VisionVariable r_filmSaturation
(
	"r_filmSaturation",
	"1.0000 1.0000 1.0000",
	&r_filmEnable
);

VisionVariable r_anaglyphAngleMult
(
	"r_anaglyphAngleMult",
	"0",
	&r_anaglyphTweakEnable
);

VisionVariable r_anaglyphOffsetMult
(
	"r_anaglyphOffsetMult",
	"0",
	&r_anaglyphTweakEnable
);

VisionVariable r_distortionRadius
(
	"r_distortionRadius",
	"1",
	&r_distortionEnable
);

VisionVariable r_distortionScale
(
	"r_distortionScale",
	"0",
	&r_distortionEnable
);

VisionVariable r_blurlevel
(
	"r_blurlevel",
	"0.0000",
	&r_blur
);

/*float m_blur_level = 0.0f;
float m_blur_currentlevel = 0.0f;
float m_blur_startlevel = 0.0f;
float m_blur_fadetime = 0.0f;
float m_blur_currentTime = 0.0f;*/

#if 0
vvar_t	r_glow;
vvar_t	r_glowRadius0;
vvar_t	r_glowRadius1;
vvar_t	r_glowBloomCutoff;
vvar_t	r_glowBloomDesaturation;
vvar_t	r_glowBloomIntensity0;
vvar_t	r_glowBloomIntensity1;
vvar_t	r_glowBloomStreakX;
vvar_t	r_glowBloomStreakY;

vvar_t	r_filmEnable;
vvar_t	r_filmContrast;
vvar_t	r_filmBrightness;
vvar_t	r_filmDesaturation;
vvar_t	r_filmSaturation;
vvar_t	r_filmHue;
vvar_t	r_filmBleach;
vvar_t	r_filmLightTint;
vvar_t	r_filmMidTint;
vvar_t	r_filmMidStart;
vvar_t	r_filmMidEnd;
vvar_t	r_filmDarkTint;

vvar_t	r_anaglyphTweakEnable;
vvar_t	r_anaglyphAngleMult;
vvar_t	r_anaglyphOffsetMult;

vvar_t	r_distortionEnable;
vvar_t	r_distortionRadius;
vvar_t	r_distortionScale;

regVis_t vvars[] =
{
	{ &r_glow,						"r_glow",					"0" },
	{ &r_glowRadius0,				"r_glowRadius0",			"0" },
	{ &r_glowRadius1,				"r_glowRadius1",			"0" },
	{ &r_glowBloomCutoff,			"r_glowBloomCutoff",		"0" },
	{ &r_glowBloomDesaturation,		"r_glowBloomDesaturation",	"0" },
	{ &r_glowBloomIntensity0,		"r_glowBloomIntensity0",	"0" },
	{ &r_glowBloomIntensity1,		"r_glowBloomIntensity1",	"0" },
	{ &r_glowBloomStreakX,			"r_glowBloomStreakX",		"0" },
	{ &r_glowBloomStreakY,			"r_glowBloomStreakY",		"0" },

	{ &r_filmEnable,				"r_filmEnable",				"0" },
	{ &r_filmContrast,				"r_filmContrast",			"1.0" },
	{ &r_filmBrightness,			"r_filmBrightness",			"0.0" },
	{ &r_filmDesaturation,			"r_filmDesaturation",		"0.0" },
	{ &r_filmHue,					"r_filmHue",				"0 0 0" },
	{ &r_filmSaturation,			"r_filmSaturation",			"1 1 1" },
	{ &r_filmBleach,				"r_filmBleach",				"0 0 0" },
	{ &r_filmLightTint,				"r_filmLightTint",			"1 1 1" },
	{ &r_filmMidStart,				"r_filmMidStart",			"0.25"	},
	{ &r_filmMidEnd,				"r_filmMidEnd",				"0.75"	},
	{ &r_filmMidTint,				"r_filmMidTint",			"1 1 1" },
	{ &r_filmDarkTint,				"r_filmDarkTint",			"1 1 1" },

	{ &r_anaglyphTweakEnable,		"r_anaglyphTweakEnable",	"0" },
	{ &r_anaglyphAngleMult,			"r_anaglyphAngleMult",		"0" },
	{ &r_anaglyphOffsetMult,		"r_anaglyphOffsetMult",		"0" },

	{ &r_distortionEnable,			"r_distortionEnable",		"0" },
	{ &r_distortionRadius,			"r_distortionRadius",		"1" },
	{ &r_distortionScale,			"r_distortionScale",		"0" },
};
#endif

regCvar_t r_cvars[] = {
	{ &r_debug,						"r_gfx_debug",				"0",					CVAR_ARCHIVE	},
	{ &r_dof,						"r_gfx_dof",				"0",					CVAR_ARCHIVE	},
	{ &r_texFilterAniso,			"r_gfx_texFilterAniso",		"8",					CVAR_ARCHIVE	},
	{ &r_ssao,						"r_gfx_ssao",				"0",					CVAR_ARCHIVE	},
	{ &r_tweakBlurX,				"r_gfx_tweakBlurX",			"0",					CVAR_ARCHIVE	},
	{ &r_tweakBlurY,				"r_gfx_tweakBlurY",			"0",					CVAR_ARCHIVE	},
	{ &r_tweakBlurPasses,			"r_gfx_tweakBlurPasses",	"0",					CVAR_ARCHIVE	},
	{ &r_glowEnable,				"r_gfx_glowEnable",			"1",					CVAR_ARCHIVE	},
	{ &r_glowQuality,				"r_gfx_glowQuality",		"8",					CVAR_ARCHIVE	},
	{ &r_glowSamples,				"r_gfx_glowSamples",		"8",					CVAR_ARCHIVE	},
	{ &r_autoluminance,				"r_gfx_autoluminance",		"1",					CVAR_ARCHIVE	},
	{ &r_fxaa,						"r_gfx_fxaa",				"1",					CVAR_ARCHIVE	},
	{ &r_test_bump,					"r_gfx_test_bump",			"1",					CVAR_ARCHIVE	},
	{ &r_test_color,				"r_gfx_test_color",			"1",					CVAR_ARCHIVE	},
	{ NULL,							NULL,						NULL,					NULL			},
};

/* Renderer stuff */
GL_State_f GL_State = ( GL_State_f )0x00498A90;
GL_Bind_f GL_Bind = ( GL_Bind_f )0x00498780;
GL_Setup_f GL_Setup = ( GL_Setup_f )0x004B0FC0;

typedef void( *R_Draw3D_f )( void );
R_Draw3D_f R_Draw3D_o;

DWORD ** ppWhite = ( DWORD ** )0x01313D98;

#define VISION_CVAR(name, default_value) name = cgi.Cvar_Get(#name, #default_value, CVAR_ROM)

void WINAPI glnDrawElements( GLuint returnAddress, GLenum mode, GLsizei count, GLenum type, const GLvoid *indices );

int R_CvarSize( void )
{
	return sizeof( r_cvars ) / sizeof( r_cvars[0] );
}

void EmptyTexture( GLuint txtnumber )
{
	unsigned int* data;											// Stored Data

	// Create Storage Space For Texture Data (128x128x4)
	data = (unsigned int*)new GLuint[ ( ( 512 * 512 )* 4 * sizeof(unsigned int) ) ];
	memset( data, 0, ( ( 512 * 512 ) * 4 * sizeof(unsigned int) ) );	// Clear Storage Memory

	glBindTexture(GL_TEXTURE_2D, txtnumber);					// Bind The Texture

	glTexImage2D( GL_TEXTURE_2D, 0, 4, 512, 512, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, data );						// Build Texture Using Information In data

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

	delete [] data;												// Release data
}

void R_RenderScene( refDef_t *fd );

int spamtime = 0;
int spamframetime = 0;
int spamprevioustime = 0;

//Our torus
TORUS torus;

//Normal map
GLuint normalMap;

//Decal texture
GLuint decalTexture;

//Normalisation cube map
GLuint normalisationCubeMap;

IMAGE normalMapImage;
IMAGE decalImage;

Vector worldLightPosition = Vector( 10.0f, 10.0f, 10.0f );

void RPrintf( int type, const char *fmt, ... )
{
	// Fix for this damn error spamming the console and sometimes this message eats more FPS than the model itself
	// This is a fix for CRAZY SERVER MODS in the map training
	// We leave at least 250 ms for this message
	if( _stricmp( "Exceeded MAX POLYS\n", fmt ) == 0 && spamtime <= 250 )
	{
		// We have a custom frametime here because RPrintf can be called 10 times in a row
		spamframetime = cgi.Milliseconds() - spamprevioustime;
		spamprevioustime = cgi.Milliseconds();

		spamtime += spamframetime;

		return;
	} else if( _stricmp( "Exceeded MAX POLYS\n", fmt ) == 0 && spamtime > 250 ) {
		spamtime = 0;
	}

	char buffer[4100];
	va_list va;

	va_start( va, fmt );
	vsprintf( buffer, fmt, va );
	va_end( va );

	ri.Printf( type, buffer );
}

bool GenerateNormalisationCubeMap()
{
	unsigned char * data = new unsigned char[ 32 * 32 * 3 ];
	if( !data )
	{
		printf( "Unable to allocate memory for texture data for cube map\n" );
		return false;
	}

	//some useful variables
	int size = 32;
	float offset = 0.5f;
	float halfSize = 16.0f;
	Vector tempVector;
	unsigned char * bytePtr;

	//positive x
	bytePtr = data;

	for( int j = 0; j<size; j++ )
	{
		for( int i = 0; i<size; i++ )
		{
			tempVector.setPitch( halfSize );
			tempVector.setYaw( -( j + offset - halfSize ) );
			tempVector.setRoll( -( i + offset - halfSize ) );

			tempVector.normalize();
			tempVector.PackTo01();

			bytePtr[ 0 ] = ( unsigned char )( tempVector.x * 255 );
			bytePtr[ 1 ] = ( unsigned char )( tempVector.y * 255 );
			bytePtr[ 2 ] = ( unsigned char )( tempVector.z * 255 );

			bytePtr += 3;
		}
	}
	glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB,
		0, GL_RGBA8, 32, 32, 0, GL_RGB, GL_UNSIGNED_BYTE, data );

	//negative x
	bytePtr = data;

	for( int j = 0; j<size; j++ )
	{
		for( int i = 0; i<size; i++ )
		{
			tempVector.setPitch( -halfSize );
			tempVector.setYaw( -( j + offset - halfSize ) );
			tempVector.setRoll( ( i + offset - halfSize ) );

			tempVector.normalize();
			tempVector.PackTo01();

			bytePtr[ 0 ] = ( unsigned char )( tempVector.x * 255 );
			bytePtr[ 1 ] = ( unsigned char )( tempVector.y * 255 );
			bytePtr[ 2 ] = ( unsigned char )( tempVector.z * 255 );

			bytePtr += 3;
		}
	}
	glTexImage2D( GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB,
		0, GL_RGBA8, 32, 32, 0, GL_RGB, GL_UNSIGNED_BYTE, data );

	//positive y
	bytePtr = data;

	for( int j = 0; j<size; j++ )
	{
		for( int i = 0; i<size; i++ )
		{
			tempVector.setPitch( i + offset - halfSize );
			tempVector.setYaw( halfSize );
			tempVector.setRoll( ( j + offset - halfSize ) );

			tempVector.normalize();
			tempVector.PackTo01();

			bytePtr[ 0 ] = ( unsigned char )( tempVector.x * 255 );
			bytePtr[ 1 ] = ( unsigned char )( tempVector.y * 255 );
			bytePtr[ 2 ] = ( unsigned char )( tempVector.z * 255 );

			bytePtr += 3;
		}
	}
	glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB,
		0, GL_RGBA8, 32, 32, 0, GL_RGB, GL_UNSIGNED_BYTE, data );

	//negative y
	bytePtr = data;

	for( int j = 0; j<size; j++ )
	{
		for( int i = 0; i<size; i++ )
		{
			tempVector.setPitch( i + offset - halfSize );
			tempVector.setYaw( -halfSize );
			tempVector.setRoll( -( j + offset - halfSize ) );

			tempVector.normalize();
			tempVector.PackTo01();

			bytePtr[ 0 ] = ( unsigned char )( tempVector.x * 255 );
			bytePtr[ 1 ] = ( unsigned char )( tempVector.y * 255 );
			bytePtr[ 2 ] = ( unsigned char )( tempVector.z * 255 );

			bytePtr += 3;
		}
	}
	glTexImage2D( GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB,
		0, GL_RGBA8, 32, 32, 0, GL_RGB, GL_UNSIGNED_BYTE, data );

	//positive z
	bytePtr = data;

	for( int j = 0; j<size; j++ )
	{
		for( int i = 0; i<size; i++ )
		{
			tempVector.setPitch( i + offset - halfSize );
			tempVector.setYaw( -( j + offset - halfSize ) );
			tempVector.setRoll( halfSize );

			tempVector.normalize();
			tempVector.PackTo01();

			bytePtr[ 0 ] = ( unsigned char )( tempVector.x * 255 );
			bytePtr[ 1 ] = ( unsigned char )( tempVector.y * 255 );
			bytePtr[ 2 ] = ( unsigned char )( tempVector.z * 255 );

			bytePtr += 3;
		}
	}
	glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB,
		0, GL_RGBA8, 32, 32, 0, GL_RGB, GL_UNSIGNED_BYTE, data );

	//negative z
	bytePtr = data;

	for( int j = 0; j<size; j++ )
	{
		for( int i = 0; i<size; i++ )
		{
			tempVector.setPitch( -( i + offset - halfSize ) );
			tempVector.setYaw( -( j + offset - halfSize ) );
			tempVector.setRoll( -halfSize );

			tempVector.normalize();
			tempVector.PackTo01();

			bytePtr[ 0 ] = ( unsigned char )( tempVector.x * 255 );
			bytePtr[ 1 ] = ( unsigned char )( tempVector.y * 255 );
			bytePtr[ 2 ] = ( unsigned char )( tempVector.z * 255 );

			bytePtr += 3;
		}
	}
	glTexImage2D( GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB,
		0, GL_RGBA8, 32, 32, 0, GL_RGB, GL_UNSIGNED_BYTE, data );

	delete[] data;

	return true;
}

void R_Init()
{
	DWORD dwExeBase = PtrToUlong( GetModuleHandle(NULL) );
	refImport_t *refimp = ( refImport_t * )( 0x01313C80 );
	refExport_t *refexp = ( refExport_t * )( 0x012E9940 );

	memcpy( &ri, refimp, sizeof( ri ) );
	memcpy( &re, refexp, sizeof( re ) );

	refimp->Printf = RPrintf;

	//refexp->RenderScene = R_RenderScene;

	//R_Draw3D_o = ( R_Draw3D_f )detour_function( ( PBYTE )0x0049A590, ( PBYTE )R_Draw3D, 6 );

	/*for( int i = 0; i < sizeof( vvars ) / sizeof( vvars[0] ); i++ )
	{
		vvars[i].vvar->name = ( char * )vvars[i].name;

		R_ProcessVisionVar( vvars[i].name, vvars[i].value );
	}*/

	glGenTextures( 1, &m_pBloomBlurShader );

	//EmptyTexture( m_pBloomBlurShader );

	gl::CheckInit();
	gl::InitializeExtensions();
	QFXRenderer::Instance().InitializeGL();

	glGenTextures( 1, &m_pBloomCombineShader );
	glGenTextures( 1, &m_pBloomDarkenShader );
	glGenTextures( 1, &m_uiBlurTexture );
	glGenTextures( 1, &m_uiInvertTexture );
	glGenTextures( 1, &m_uiDarkTexture );
	glGenTextures( 1, &m_uiLightTexture );

	render_inited = true;

	//refexp->DrawBox = DrawBox;
	//refimp->CM_EntityString = CM_EntityString;

	/*
	//Load identity modelview
	gl::qglMatrixMode( GL_MODELVIEW );
	gl::qglLoadIdentity();

	//Shading states
	gl::qglShadeModel( GL_SMOOTH );
	gl::qglClearColor( 0.2f, 0.4f, 0.2f, 0.0f );
	gl::qglColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
	gl::qglHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );

	//Depth states
	gl::qglClearDepth( 1.0f );
	gl::qglDepthFunc( GL_LEQUAL );
	gl::qglEnable( GL_DEPTH_TEST );

	gl::qglEnable( GL_CULL_FACE );

	//Load normal map
	normalMapImage.Load( "Normal map.bmp" );
	normalMapImage.ExpandPalette();

	//Convert normal map to texture
	gl::qglGenTextures( 1, &normalMap );
	gl::qglBindTexture( GL_TEXTURE_2D, normalMap );
	gl::qglTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, normalMapImage.width, normalMapImage.height,
		0, normalMapImage.format, GL_UNSIGNED_BYTE, normalMapImage.data );
	gl::qglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	gl::qglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	gl::qglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	gl::qglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

	//Load decal image
	decalImage.Load( "decal.bmp" );
	decalImage.ExpandPalette();

	//Convert decal image to texture
	gl::qglGenTextures( 1, &decalTexture );
	gl::qglBindTexture( GL_TEXTURE_2D, decalTexture );
	gl::qglTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, decalImage.width, decalImage.height,
		0, decalImage.format, GL_UNSIGNED_BYTE, decalImage.data );
	gl::qglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	gl::qglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	gl::qglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	gl::qglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );


	//Create normalisation cube map
	gl::qglGenTextures( 1, &normalisationCubeMap );
	gl::qglBindTexture( GL_TEXTURE_CUBE_MAP_ARB, normalisationCubeMap );
	GenerateNormalisationCubeMap();
	gl::qglTexParameteri( GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	gl::qglTexParameteri( GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	gl::qglTexParameteri( GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	gl::qglTexParameteri( GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	gl::qglTexParameteri( GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );
	*/
}

void R_Shutdown(void)
{
	int i;
	refImport_t *refimp = ( refImport_t * )( 0x01313C80 );

	refimp->Printf = ri.Printf;

	gl::Shutdown();

	/* Revert to the original function */
	/**exp_glBegin = (DWORD)orig_exp_glBegin;
	*exp_glDrawElements = (DWORD)orig_exp_glDrawElements;
	*exp_glViewport = (DWORD)orig_exp_glViewport;*/

	detour_remove( R_Draw3D, R_Draw3D_o );

	glDeleteTextures( 1, &m_pBloomBlurShader );
	glDeleteTextures( 1, &m_pBloomCombineShader );
	glDeleteTextures( 1, &m_pBloomDarkenShader );
	glDeleteTextures( 1, &m_uiBlurTexture );
	glDeleteTextures( 1, &m_uiInvertTexture );
	glDeleteTextures( 1, &m_uiDarkTexture );
	glDeleteTextures( 1, &m_uiLightTexture );

	for( i = 0; i < MAX_EFFECTS; i++ )
	{
		if( effect_list[ i ] == NULL )
			continue;

		R_RemoveEffect( effect_list[ i ] );
	}
}

void DrawBlur( float blur );
void DrawGBlur( int radius, float intensity, float cutoff, float desaturation );
void R_FadeOverTime();
void ProcessBloom( float radius );
void ViewOrtho();
void ViewPerspective();

void R_DrawModels()
{
	refEntity_t ref;
	Entity *ent;

	for( int i = 0; i < s_entities.NumObjects(); i++ )
	{
		ent = s_entities.ObjectAt( i + 1 );

		if( !ent->modelhandle || ( ent->renderFx & RF_INVISIBLE ) ) {
			continue;
		}

		memset( &ref, 0, sizeof( refEntity_t ) );

		if( ent->attached )
		{
			orientation_t orient;

			memset( &orient, 0, sizeof( orientation_t ) );

			ent->attached->GetTag( ent->attached_tagname, &orient );

			if( ent->attached_use_angles )
			{
				MatrixToEulerAngles( orient.axis, ent->angles );
			}

			ent->origin = ent->attached->origin + orient.origin + ent->attached_offset;
		}

		VectorCopy( ent->origin, ref.lightingOrigin );
		VectorCopy( ent->origin, ref.origin );
		VectorCopy( ent->origin, ref.oldorigin );

		//angles[ 1 ] += 90.f;
		//angles[ 2 ] += 90.f;

		AnglesToAxis( ent->angles, ref.axis );

		ref.reType = RT_MODEL;

		ref.scale = ent->scale;
		ref.hModel = ent->modelhandle;
		ref.hOldModel = ent->modelhandle;
		ref.tiki = ent->tiki;
		ref.entityNumber = 2048;
		ref.actionWeight = 1.0f;

		for( int i = 0; i < ent->GetNumFrames(); i++ )
		{
			frameInfo_t *frame = ent->GetFrameInfo( i );

			ref.frameInfo[ i ].index = frame->index;
			ref.frameInfo[ i ].time = frame->time;
			ref.frameInfo[ i ].weight = frame->weight;
		}

		cge.CG_ProcessInitCommands( ent->tiki, &ref );

		ref.renderfx = ent->renderFx;

		cgi.R_AddRefEntityToScene( &ref, 0 );
	}
}

Entity *R_SpawnModel( const char *model, vec3_t origin, vec3_t angles )
{
	Entity *ent = new Entity;

	ent->setModel( model );

	ent->setAngles( angles );
	ent->setOrigin( origin );

	//ent->renderFx = RF_ADDITIVEDLIGHT | RF_EXTRALIGHT | RF_LIGHTINGORIGIN | RF_SHADOW | RF_SHADOWPRECISE | RF_FIRST_PERSON;

	return ent;
}

int refEntityReadd = 0;
int refSpriteReadd = 0;
refEntity_t *refEntityList[ 1024 ];
refEntity_t *refSpriteList[ 1024 ];

model_t *models = ( model_t * )0x1314890;

void refEntityAdd()
{
	for( int i = 0; i < refSpriteReadd; i++ )
	{
		refEntity_t *ent = refSpriteList[ i ];

		cgi.R_AddRefSpriteToScene( ent );

		free( ent );

		refSpriteList[ i ] = NULL;
	}

	for( int i = 0; i < refEntityReadd; i++ )
	{
		refEntity_t *ent = refEntityList[ i ];
		model_t *pModel = &models[ ent->hModel ];

		pModel->bmodel->haslightmap = false;

		cgi.R_AddRefEntityToScene( ent, ent->parentEntity );

		free( ent );

		refEntityList[ i ] = NULL;
	}

	refEntityReadd = 0;
	refSpriteReadd = 0;
}

void R_RenderScene( refDef_t *fd )
{
	GL_Setup();
	GL_State( GLS_DEPTHMASK_TRUE );
	R_DrawModels();

	if( !r_anaglyph->integer && !r_anaglyphTweakEnable.isEnabled() )
	{
		//QFXRenderer::Instance().PreRenderScene( fd );
		cgi.R_RenderScene( fd );
		//QFXRenderer::Instance().PostRenderScene();
		GL_Setup();
		GL_State( GLS_DEPTHMASK_TRUE );
		R_Draw3D();

		return;
	}

	glDrawBuffer( GL_BACK );
	glReadBuffer( GL_BACK );

	/* Clear things */
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	//glClear( GL_ACCUM_BUFFER_BIT );

	glViewport( 0, 0, cgs->glConfig.vidWidth, cgs->glConfig.vidHeight );

	glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
	glColorMask( GL_TRUE, GL_FALSE, GL_FALSE, GL_TRUE );

	Vector angles;
	Vector leftvec;

	MatrixToEulerAngles( fd->viewAxis, angles );
	AngleVectors( angles, NULL, leftvec, NULL );

	Vector old_angles = angles;
	Vector old_origin = fd->viewOrg;

	/*if( r_anaglyph->integer )
	{
		if( r_anaglyphAngleMult.value_current < 1.0f && ( r_anaglyphAngleMult.value >= 1.0f || !r_anaglyphTweakEnable.integer_target ) ) {
			r_anaglyphAngleMult.value_current = 1.0f;
		}

		if( r_anaglyphOffsetMult.value_current < 1.0f && ( r_anaglyphOffsetMult.value >= 1.0f || !r_anaglyphTweakEnable.integer_target ) ) {
			r_anaglyphOffsetMult.value_current = 1.0f;
		}
	}*/

	if( r_anaglyphTweakEnable.isEnabled() ) {
		angles.y -= ( 1.0f * r_anaglyphAngleMult.floatValue() );
	} else {
		angles.y -= 1.0f;
	}

	AnglesToAxis( angles, fd->viewAxis );

	cgi.R_RenderScene( fd );
	GL_Setup();
	glFlush();

	glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );

	glDrawBuffer( GL_BACK );
	glClear( GL_DEPTH_BUFFER_BIT );

	if( r_anaglyphTweakEnable.isEnabled() )
	{
		fd->viewOrg[ 0 ] += leftvec[ 0 ] * ( 0.25f * r_anaglyphOffsetMult.floatValue() );
		fd->viewOrg[ 2 ] -= ( 0.25f * r_anaglyphOffsetMult.floatValue() );
		angles.y += ( 1.0f * r_anaglyphAngleMult.floatValue() );
	}
	else
	{
		fd->viewOrg[ 0 ] += leftvec[ 0 ] * 0.25f;
		fd->viewOrg[ 2 ] -= 0.25f;
		angles.y += 1.0f;
	}

	AnglesToAxis( angles, fd->viewAxis );

	glViewport( 0, 0, cgs->glConfig.vidWidth, cgs->glConfig.vidHeight );

	glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
	glColorMask( GL_FALSE, GL_TRUE, GL_TRUE, GL_TRUE );

	refEntityAdd();

	cgi.R_RenderScene( fd );
	GL_Setup();
	glFlush();

	glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );

	AnglesToAxis( old_angles, fd->viewAxis );
	AnglesToAxis( old_angles, cg->refdef.viewAxis );

	VectorCopy( old_origin, fd->viewOrg );
	VectorCopy( old_origin, cg->refdef.viewOrg );

	R_Draw3D();
}

void R_Draw3D()
{
	int blend_src = 0, blend_dst = 0;
	GLint viewport[4];

	glHint( GL_GENERATE_MIPMAP_HINT, GL_NICEST );
	glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
	glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );
	glHint( GL_POINT_SMOOTH_HINT, GL_NICEST );
	glHint( GL_POLYGON_SMOOTH_HINT, GL_NICEST );
	glHint( GL_FRAGMENT_SHADER_DERIVATIVE_HINT, GL_NICEST );

#if 0
	static float angle = 0.0f;
	angle += 0.1f;
	gl::qglRotatef( angle, 0.0f, 1.0f, 0.0f );

	matrix_t inverseModelMatrix;
	gl::qglPushMatrix();
	gl::qglLoadIdentity();
	gl::qglRotatef( -angle, 0.0f, 1.0f, 0.0f );
	gl::qglGetFloatv( GL_MODELVIEW_MATRIX, ( GLfloat * )inverseModelMatrix );
	gl::qglPopMatrix();

	vec4_t o;

	MatrixTransform4( inverseModelMatrix, worldLightPosition, o );

	Vector objectLightPosition;

	if( o[ 3 ] == 0.0f || o[ 3 ] == 1.0f )
	{
		objectLightPosition = Vector( o[ 0 ], o[ 1 ], o[ 2 ] );
	}
	else
	{
		objectLightPosition = Vector( o[ 0 ] / o[ 3 ], o[ 1 ] / o[ 3 ], o[ 2 ] / o[ 3 ] );
	}

	//Loop through vertices
	for( int i = 0; i<torus.numVertices; ++i )
	{
		Vector lightVector = objectLightPosition - torus.vertices[ i ].position;

		//Calculate tangent space light vector
		torus.vertices[ i ].tangentSpaceLight.x = DotProduct( torus.vertices[ i ].sTangent, lightVector );
		torus.vertices[ i ].tangentSpaceLight.y = DotProduct( torus.vertices[ i ].tTangent, lightVector );
		torus.vertices[ i ].tangentSpaceLight.z = DotProduct( torus.vertices[ i ].normal, lightVector );
	}

	if( r_test_bump->integer )
	{
		//Bind normal map to texture unit 0
		gl::qglBindTexture( GL_TEXTURE_2D, normalMap );
		gl::qglEnable( GL_TEXTURE_2D );

		//Bind normalisation cube map to texture unit 1
		gl::qglActiveTextureARB( GL_TEXTURE1_ARB );
		gl::qglBindTexture( GL_TEXTURE_CUBE_MAP_ARB, normalisationCubeMap );
		gl::qglEnable( GL_TEXTURE_CUBE_MAP_ARB );
		gl::qglActiveTextureARB( GL_TEXTURE0_ARB );

		//Send texture coords for normal map to unit 0
		gl::qglTexCoordPointer( 2, GL_FLOAT, sizeof( TORUS_VERTEX ), &torus.vertices[ 0 ].s );
		gl::qglEnableClientState( GL_TEXTURE_COORD_ARRAY );

		//Set vertex arrays for torus
		gl::qglVertexPointer( 3, GL_FLOAT, sizeof( TORUS_VERTEX ), &torus.vertices[ 0 ].position );
		gl::qglEnableClientState( GL_VERTEX_ARRAY );

		//Send tangent space light vectors for normalisation to unit 1
		gl::qglClientActiveTextureARB( GL_TEXTURE1_ARB );
		gl::qglTexCoordPointer( 3, GL_FLOAT, sizeof( TORUS_VERTEX ), &torus.vertices[ 0 ].tangentSpaceLight );
		gl::qglEnableClientState( GL_TEXTURE_COORD_ARRAY );
		gl::qglClientActiveTextureARB( GL_TEXTURE0_ARB );


		//Set up texture environment to do (tex0 dot tex1)*color
		gl::qglTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB );
		gl::qglTexEnvi( GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_TEXTURE );
		gl::qglTexEnvi( GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_REPLACE );

		gl::qglActiveTextureARB( GL_TEXTURE1_ARB );

		gl::qglTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB );
		gl::qglTexEnvi( GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_TEXTURE );
		gl::qglTexEnvi( GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_DOT3_RGB_ARB );
		gl::qglTexEnvi( GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_PREVIOUS_ARB );

		gl::qglActiveTextureARB( GL_TEXTURE0_ARB );

		//Draw torus
		gl::qglDrawElements( GL_TRIANGLES, torus.numIndices, GL_UNSIGNED_INT, torus.indices );

		//Disable textures
		gl::qglDisable( GL_TEXTURE_2D );

		gl::qglActiveTextureARB( GL_TEXTURE1_ARB );
		gl::qglDisable( GL_TEXTURE_CUBE_MAP_ARB );
		gl::qglActiveTextureARB( GL_TEXTURE0_ARB );

		//disable vertex arrays
		gl::qglDisableClientState( GL_VERTEX_ARRAY );

		gl::qglDisableClientState( GL_TEXTURE_COORD_ARRAY );

		gl::qglClientActiveTextureARB( GL_TEXTURE1_ARB );
		gl::qglDisableClientState( GL_TEXTURE_COORD_ARRAY );
		gl::qglClientActiveTextureARB( GL_TEXTURE0_ARB );

		//Return to standard modulate texenv
		gl::qglTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	}

	if( r_test_bump->integer && r_test_color->integer )
	{
		gl::qglBlendFunc( GL_DST_COLOR, GL_ZERO );
		gl::qglEnable( GL_BLEND );
	}

	if( r_test_color->integer )
	{
		if( !r_test_bump->integer )
		{
			gl::qglLightfv( GL_LIGHT1, GL_POSITION, ( const GLfloat * )&Quat( objectLightPosition ) );
			gl::qglLightfv( GL_LIGHT1, GL_DIFFUSE, ( const GLfloat * )&Quat( 1.0f, 1.0f, 1.0f, 1.0f ) );
			gl::qglLightfv( GL_LIGHT1, GL_AMBIENT, ( const GLfloat * )&Quat( 0.0f, 0.0f, 0.0f, 0.0f ) );
			gl::qglLightModelfv( GL_LIGHT_MODEL_AMBIENT, ( const GLfloat * )&Quat( 0.0f, 0.0f, 0.0f, 0.0f ) );
			gl::qglEnable( GL_LIGHT1 );
			gl::qglEnable( GL_LIGHTING );

			gl::qglMaterialfv( GL_FRONT, GL_DIFFUSE, ( const GLfloat * )&Quat( 1.0f, 1.0f, 1.0f, 1.0f ) );
		}

		//Bind decal texture
		gl::qglBindTexture( GL_TEXTURE_2D, decalTexture );
		gl::qglEnable( GL_TEXTURE_2D );

		//Set vertex arrays for torus
		gl::qglVertexPointer( 3, GL_FLOAT, sizeof( TORUS_VERTEX ), &torus.vertices[ 0 ].position );
		gl::qglEnableClientState( GL_VERTEX_ARRAY );

		gl::qglNormalPointer( GL_FLOAT, sizeof( TORUS_VERTEX ), &torus.vertices[ 0 ].normal );
		gl::qglEnableClientState( GL_NORMAL_ARRAY );

		gl::qglTexCoordPointer( 2, GL_FLOAT, sizeof( TORUS_VERTEX ), &torus.vertices[ 0 ].s );
		gl::qglEnableClientState( GL_TEXTURE_COORD_ARRAY );

		//Draw torus
		gl::qglDrawElements( GL_TRIANGLES, torus.numIndices, GL_UNSIGNED_INT, torus.indices );

		//Disable texture
		gl::qglDisable( GL_TEXTURE_2D );

		//disable vertex arrays
		gl::qglDisableClientState( GL_VERTEX_ARRAY );
		gl::qglDisableClientState( GL_NORMAL_ARRAY );
		gl::qglDisableClientState( GL_TEXTURE_COORD_ARRAY );
	}

	if( r_test_bump->integer && r_test_color->integer )
	{
		gl::qglDisable( GL_BLEND );
	}

	gl::qglFinish();
#endif

	QFXRenderer::Instance().GetVariables();
	QFXRenderer::Instance().RenderGL();

	// Post-processing effects are after Draw3D

	/*if( r_glow.integer )
	{
		DrawGBlur( r_glowRadius0.value_current,
				r_glowBloomIntensity0.value_current,
				r_glowBloomCutoff.value_current,
				r_glowBloomDesaturation.value_current );
	}*/

	DrawBlur( r_blurlevel.floatValue() );
#if 0

	ViewOrtho();

	glDepthRange( 0.0f, 0.0f );

	glGetIntegerv( GL_VIEWPORT, viewport );

	glGetIntegerv( GL_BLEND_SRC, &blend_src );
	glGetIntegerv( GL_BLEND_DST, &blend_dst );

	if( r_filmEnable.isEnabled() )
	{
		// This part is complex
		// It is doing a transition between inverted colors and normal colors
		/*if( r_filmInvert.integer )
		{
			float invert_value = r_filmInvert.value_current;

			if( invert_value != r_filmInvert.value ) {
				glBlendFunc( GL_ONE_MINUS_DST_COLOR, GL_SRC_ALPHA );
			} else {
				glBlendFunc( GL_ONE_MINUS_DST_COLOR, GL_ZERO );
			}

			// Generate the invert texture
			glBindTexture( GL_TEXTURE_2D, m_uiInvertTexture );

			glBegin( GL_QUADS );
				glColor4f( 1.0f, 1.0f, 1.0f, 0.1f );
				glVertex2f( -cgs->glConfig.vidWidth, -cgs->glConfig.vidHeight );
				glVertex2f( cgs->glConfig.vidWidth, -cgs->glConfig.vidHeight );
				glVertex2f( cgs->glConfig.vidWidth, cgs->glConfig.vidHeight );
				glVertex2f( -cgs->glConfig.vidWidth, cgs->glConfig.vidHeight );
			glEnd();

			if( invert_value != r_filmInvert.value )
			{
				glBindTexture( GL_TEXTURE_2D, m_uiInvertTexture );

				glBegin( GL_QUADS );
					glColor4f( 1.0f-invert_value, 1.0f-invert_value, 1.0f-invert_value, invert_value );
					glVertex2f( -cgs->glConfig.vidWidth, -cgs->glConfig.vidHeight );
					glVertex2f( cgs->glConfig.vidWidth, -cgs->glConfig.vidHeight );
					glVertex2f( cgs->glConfig.vidWidth, cgs->glConfig.vidHeight );
					glVertex2f( -cgs->glConfig.vidWidth, cgs->glConfig.vidHeight );
				glEnd();
			}
		}*/

		//R_SetSceneColor( r_filmLightTint.vector_current, r_filmDarkTint.vector_current );
	}

	//if( r_filmEnable.integer ) {
	//	R_SetSceneColor( r_filmLightTint.vector );
	//}

	/*memset( pixelData, 0, ( ( 512 * 512 ) * 4 * 4 ) );

	glBindTexture( GL_TEXTURE_2D, m_pBloomBlurShader );
	glTexImage2D( GL_TEXTURE_2D, 0, 4, 512, 512, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelData );

	glCopyTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, 512, 512, 0 );

	glClearColor( 0, 0, 0.5f, 0 );

	// Bloom/Glow effects
	glViewport( 0, 0, 512, 512 );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	//glReadPixels( 0, 0, 512, 512, GL_RGBA, GL_UNSIGNED_BYTE, pixelData );

	for( int i = 0; i < 512 * 512 ; i ++ ) {
		pixelData[i] = pixelData[i] / 64 * 64;
	}

	glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, 512, 512, GL_RGBA, GL_UNSIGNED_BYTE, pixelData );

	//glDrawPixels( 512, 512, GL_RGB, GL_UNSIGNED_BYTE, pixelData );

	free( pixelData );

	glViewport( viewport[0], viewport[1], viewport[2], viewport[3] );

	glBlendFunc( GL_SRC_ALPHA, GL_ONE );

	glBegin( GL_QUADS );
		glVertex2f( -cgs->glConfig.vidWidth, -cgs->glConfig.vidHeight );
		glVertex2f( cgs->glConfig.vidWidth, -cgs->glConfig.vidHeight );
		glVertex2f( cgs->glConfig.vidWidth, cgs->glConfig.vidHeight );
		glVertex2f( -cgs->glConfig.vidWidth, cgs->glConfig.vidHeight );
	glEnd();

	glFlush();

	orig_wglSwapBuffers( NULL );*/

	//glViewport( 0, 0, 512, 512 );
	//glViewport( viewport[0], viewport[1], viewport[2], viewport[3] );

	ViewPerspective();

	// 64, 0.55f
	glDepthRange( 0.0f, 1.0f );

	glBlendFunc( blend_src, blend_dst );
#endif

	for( int i = m_visionVar.NumObjects(); i > 0; i-- )
	{
		VisionClass *variable = m_visionVar.ObjectAt( i );

		if( !variable ) {
			continue;
		}

		variable->Think();
	}

	//R_FadeOverTime();
	//R_FadeBlurOverTime();
}

void ViewOrtho()
{
	glMatrixMode( GL_PROJECTION );
	glPushMatrix();
	glLoadIdentity();
	glOrtho( 0, cgs->glConfig.vidWidth , cgs->glConfig.vidHeight , 0, -1, 1 );
	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glLoadIdentity();
}

void ViewPerspective()
{
	glPopMatrix();
	glMatrixMode( GL_PROJECTION );
	glPopMatrix();
	glMatrixMode( GL_MODELVIEW );
}

void DrawBlur( float blur )
{
	if( blur <= 0.0f ) {
		return;
	}

	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	// Generate the blur texture
	glBindTexture( GL_TEXTURE_2D, m_uiBlurTexture );

	glBegin( GL_QUADS );
		glColor4f( 0.0f, 0.0f, 0.0f, 0.0f );
		glVertex2f( ( GLfloat )-cgs->glConfig.vidWidth, ( GLfloat )-cgs->glConfig.vidHeight );
		glVertex2f( ( GLfloat )cgs->glConfig.vidWidth, ( GLfloat )-cgs->glConfig.vidHeight );
		glVertex2f( ( GLfloat )cgs->glConfig.vidWidth, ( GLfloat )cgs->glConfig.vidHeight );
		glVertex2f( ( GLfloat )-cgs->glConfig.vidWidth, ( GLfloat )cgs->glConfig.vidHeight );
	glEnd();

	glAccum( GL_MULT, blur );
	glAccum( GL_ACCUM, 1.0f - blur );
	glAccum( GL_RETURN, 1.0f );

	glFlush();
}

void WINAPI glnDrawElements( GLuint returnAddress, GLenum mode,  GLsizei count,  GLenum type,  const GLvoid *indices )
{
#if 0
	int i;
	GLubyte colorR[5000];
	GLubyte colorG[5000];
	GLubyte colorB[5000];
	char * lpCurrentShader = *( ( char ** ) 0x01313900 );

	for ( i=0; i <= 4998; i = i+4 )
	{
		colorR[i+0]=0xFF;
		colorR[i+1]=0x00;
		colorR[i+2]=0x00;
		colorR[i+3]=0xFF;

		colorG[i+0]=0x00;
		colorG[i+1]=0xFF;
		colorG[i+2]=0x00;
		colorG[i+3]=0xFF;

		colorB[i+0]=0x00;
		colorB[i+1]=0x00;
		colorB[i+2]=0xFF;
		colorB[i+3]=0xFF;
	}

	//if( returnAddress != ELM_FLAT )
	//{
		if( !strstr( lpCurrentShader, "DAK_pants_tan" ) &&
			!strstr( lpCurrentShader, "DAK_private_tunic" ) &&
			!strstr( lpCurrentShader, "c_DAK_private_tunic") &&
			!strstr( lpCurrentShader, "facewrap" ) &&
			!strstr( lpCurrentShader, "german-helmet_inside" ) &&
			!strstr( lpCurrentShader, "dak_helmet" ) ) {
			return glDrawElements( mode, count, type, indices );
		}

		glDisable( GL_TEXTURE_2D );

		//glColorPointer( 4, GL_UNSIGNED_BYTE, 0, &colorG );
		glDrawElements( mode, count, type, indices );
		glColorPointer( 4, GL_UNSIGNED_BYTE, 0, &colorB );
		glColorPointer( 4, GL_UNSIGNED_BYTE, 0, &colorG );

		glEnable( GL_TEXTURE_2D );

	//}
#endif

	gl::qglDrawElements( mode, count, type, indices );
}

void WINAPI glDrawElements_h( GLenum mode,  GLsizei count,  GLenum type,  const GLvoid *indices )
{
	gl::qglDrawElements( mode, count, type, indices );
}

void WINAPI glTexCoordPointer_h( GLint size, GLenum type, GLsizei stride, const GLvoid *pointer )
{
	gl::qglTexCoordPointer( size, type, stride, pointer );
}

void R_SetShader( int ent_num, const char * shader, qboolean fDefault )
{
	if( !fDefault )
	{
		Entity *entity = CL_GetCEntity( ent_num );

		if( entity == NULL ) {
			entity = new Entity;
		}

		// Assign the number
		entity->AssignNumber( ent_num );

		entity->SetShader( shader );
	}
	else
	{
		Entity *entity = CL_GetCEntity( ent_num );

		if( entity != NULL ) {
			entity->SetShader( NULL );
		}
	}
}

void R_AddRefEntityToScene( refEntity_t *ent, int parentEntityNumber )
{
	centity_t *cent = CL_GetEntity( ent->entityNumber );
	Entity *entity = cent != NULL ? CL_GetCEntity( cent->currentState.number ) : NULL;

	if( entity != NULL && entity->fShader ) { // && ent->model != cg->playerFPSModelHandle && ent->entityNumber != cg->clientNum ) {
		ent->customShader = entity->customshader;
	}

	if( r_anaglyph->integer || r_anaglyphTweakEnable.isEnabled() )
	{
		refEntity_t *newEnt = ( refEntity_t * )malloc( sizeof( refEntity_t ) );

		memcpy( newEnt, ent, sizeof( refEntity_t ) );

		refEntityList[ refEntityReadd ] = newEnt;
		refEntityReadd++;
	}

	cgi.R_AddRefEntityToScene( ent, parentEntityNumber );
}

void R_AddRefSpriteToScene( refEntity_t *ent )
{
	if( r_anaglyph->integer || r_anaglyphTweakEnable.isEnabled() )
	{
		refEntity_t *newEnt = ( refEntity_t * )malloc( sizeof( refEntity_t ) );

		memcpy( newEnt, ent, sizeof( refEntity_t ) );

		refSpriteList[ refSpriteReadd ] = newEnt;
		refSpriteReadd++;
	}

	cgi.R_AddRefSpriteToScene( ent );
}

/*void WINAPI glViewport(GLint x,  GLint y,  GLsizei width,  GLsizei height)
{
	orig_exp_glViewport(x, y, width, height);
}*/

effect_t * R_GetEffectById(int ID)
{
	return effect_list[ID];
}

effect_t * R_GetEffectByName(const char * fxName)
{
	int i;

	if(!R_IsEffect(fxName))
	{
		cgi.DPrintf("R_GetEffectByName : unknown effect name %s\n", fxName);
		return NULL;
	}

	for(i=0;i<MAX_EFFECTS;i++)
	{
		/* Skip NULL effects to prevent crashes */
		if(effect_list[i] == NULL)
			continue;

		if(_strcmpi(fxName, effect_list[i]->effectName) == 0)
			return effect_list[i];
	}

	return NULL;
}

int R_GetEffectId(effect_t * effect)
{
	int i;

	if(effect == NULL)
		return -1;

	for(i=0;i<MAX_EFFECTS;i++)
	{
		if(effect_list[i] == effect)
			return i;
	}

	return -1;
}

void R_PostProcessEffect(const char * fxName, float intensity, vec3_t colors, float fadetime, qboolean bAddEffect, float parameters[2])
{
	int i;
	qboolean bFound = false;
	effect_t * effect;
	int nameLength;

	if(fxName == NULL)
		return;

	nameLength = strlen(fxName);

	if(!R_IsEffect(fxName))
	{
		cgi.DPrintf("R_PostProcessEffect : unknown effect name %s\n", fxName);
		return;
	}

	if( !bAddEffect || bAddEffect == 2 )
		bFound = false;

	effect = R_GetEffectByName(fxName);

	if( bAddEffect && effect != NULL )
	{
		cgi.DPrintf("R_PostProcessEffect : effect name %s already present in the effects list !\n", fxName);
		return;
	}
	else if( !bAddEffect || bAddEffect == 2 )
	{
		/* Error if the object is being removed and is not found */
		if(effect == NULL)
		{
			cgi.DPrintf("R_PostProcessEffect : can't find %s in the effect list !\n", fxName);
			return;
		}

		effect->fadetime = fadetime;

		if(!bAddEffect) {
			effect->bRemove = true;
		}

		/* Remove effects that doesn't/can't fade */
		if(effect->fadetime == 0)
		{
			if(!bAddEffect)
				R_RemoveEffect(effect);
		}
		else if(_strcmpi(effect->effectName, "invert") == 0)
		{
			if(!bAddEffect)
				R_RemoveEffect(effect);
		}

		if(bAddEffect == 2)
		{
			effect->intensity = intensity;

			effect->o_color[RED] = effect->t_color[RED];
			effect->o_color[GREEN] = effect->t_color[GREEN];
			effect->o_color[BLUE] = effect->t_color[BLUE];

			effect->t_color[RED] = colors[RED];
			effect->t_color[GREEN] = colors[GREEN];
			effect->t_color[BLUE] = colors[BLUE];

			memcpy(effect->param, parameters, sizeof(float));
		}

		return;
	}

	bFound = false;

	/* Loop until we find a free spot in the used effects list */
	for(i=0;i<MAX_EFFECTS;i++)
	{
		if(effect_list[i] == NULL)
		{
			bFound = true;
			break;
		}
	}

	/* Error if a spot wasn't found */
	if(!bFound)
	{
		cgi.DPrintf("R_PostProcessEffect : not enough memory to place the effect %s into the effect list !\n", fxName);
		return;
	}

	/* Allocate the effect */
	effect = (effect_t *)malloc(sizeof(effect_t));

	memset(effect, 0, sizeof(effect_t));

	if(effect == NULL)
	{
		cgi.DPrintf("R_PostProcessEffect : not enough memory to allocate the effect structure !\n");
		return;
	}

	/* Now copy the effect name to the effect structure */
	strcpy(effect->effectName, fxName);

	effect->intensity = intensity;

	effect->color[RED]= 1.0f;
	effect->color[GREEN]= 1.0f;
	effect->color[BLUE]= 1.0f;

	effect->t_color[RED] = colors[RED];
	effect->t_color[GREEN] = colors[GREEN];
	effect->t_color[BLUE] = colors[BLUE];

	effect->o_color[RED] = colors[RED];
	effect->o_color[GREEN] = colors[GREEN];
	effect->o_color[BLUE] = colors[BLUE];

	effect->fadetime = fadetime;

	memcpy(effect->param, parameters, sizeof(float));

	cgi.DPrintf("R_PostProcessEffect : effect '%s' (%p) added to the effect list (intensity=%f|red=%f|green=%f|blue=%f|param1=%f|param2=%f)\n", effect->effectName, effect, intensity, effect->t_color[RED], effect->t_color[GREEN], effect->t_color[BLUE], parameters[0], parameters[1]);

	/* Place the effect in the used effect list */
	effect_list[i] = effect;
}

void R_ProcessEffectFade(effect_t * effect, int frametime)
{
	float time;
	int reverted;

	if(effect == NULL)
		return;

	time = effect->fadetime * 1000.0f;

	/* Fade the RGB values over the specified time */

	reverted = 0;

	if(!effect->bRemove)
	{
		if(effect->color[RED] > effect->t_color[RED])
			effect->color[RED] -= (1.0f-effect->t_color[RED])/time * frametime;
		else if(effect->color[RED] < effect->t_color[RED])
			effect->color[RED] += (effect->t_color[RED]-effect->o_color[RED])/time * frametime;

		if(effect->color[GREEN] > effect->t_color[GREEN])
			effect->color[GREEN] -= (1.0f-effect->t_color[GREEN])/time * frametime;
		else if(effect->color[GREEN] < effect->t_color[GREEN])
			effect->color[GREEN] += (effect->t_color[GREEN]-effect->o_color[GREEN])/time * frametime;

		if(effect->color[BLUE] > effect->t_color[BLUE])
			effect->color[BLUE] -= (1.0f-effect->t_color[BLUE])/time * frametime;
		else if(effect->color[BLUE] < effect->t_color[BLUE])
			effect->color[BLUE] += (effect->t_color[BLUE]-effect->o_color[BLUE])/time * frametime;
	}
	else
	{
		/* If the effect is being removed, increase all RGB values up to 1.0 */

		effect->color[RED] += 1.0f/time * frametime;

		if(effect->color[RED] > 1.0f)
		{
			effect->color[RED] = 1.0f;
			reverted++;
		}

		effect->color[GREEN] += 1.0f/time * frametime;

		if(effect->color[GREEN] > 1.0f)
		{
			effect->color[GREEN] = 1.0f;
			reverted++;
		}

		effect->color[BLUE] += 1.0f/time * frametime;

		if(effect->color[BLUE] > 1.0f)
		{
			effect->color[BLUE] = 1.0f;
			reverted++;
		}

		/* If the effect did the transition (colors values are all 1.0) then we can now remove it */
		if(reverted >= 3)
			R_RemoveEffect(effect);
	}
}

void R_ProcessEffectsFade(int frametime)
{
	int i;

	/* Process the effects fading */

	for(i=0;i<MAX_EFFECTS;i++)
	{
		if(effect_list[i] != NULL)
			R_ProcessEffectFade(effect_list[i], frametime);
	}
}

void R_RemoveEffect(effect_t * effect)
{
	int index = 0;
	char *fxName;

	if(effect == NULL)
		return;

	/* Get an index for the effect from the effect list */
	index = R_GetEffectId(effect);

	effect->bRemove = true;

	/* Remove the effect from the list */
	if(index != -1)
		effect_list[index] = NULL;

	/* Just reallocate the effect name so that the debug message will show which effect 
	 * has been removed
	 */
	fxName = (char*)malloc(strlen(effect->effectName));

	strcpy(fxName, effect->effectName);

	memset(effect, 0, sizeof(effect_t));

	/* Free the effect from the memory */
	free(effect);

	cgi.DPrintf("R_RemoveEffect : effect '%s' removed from the effect list\n", fxName);

	/* Free the effect name */
	free(fxName);
}

#if 0

void R_SetSceneColor( vec3_t light, vec3_t dark )
{
#if 0
	GLfloat rgb[10010];
	int i;

	/* Set the RGB array values */
	for( i = 0; i <= 10007; i += 3 )
	{
		rgb[i] = color[0];
		rgb[i+1] = color[1];
		rgb[i+2] = color[2];
	}

	/* Apply the RGB values to the scene */
	glEnable( GL_BLEND );
	glColorPointer( 3, GL_FLOAT, 0, rgb );
#endif

	light[0] /= 2.0f;
	light[1] /= 2.0f;
	light[2] /= 2.0f;

	dark[0] /= 2.0f;
	dark[1] /= 2.0f;
	dark[2] /= 2.0f;

	GL_State( GLS_DEPTHMASK_TRUE );

	glEnableClientState( GL_COLOR_ARRAY );
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );

	glEnable( GL_BLEND );

	// Generate the light tint texture

	glBlendFunc( GL_DST_COLOR, GL_ONE );
	glBindTexture( GL_TEXTURE_2D, m_uiLightTexture );

	glBegin( GL_QUADS );
		glColor3fv( light );
		glVertex2f( -cgs->glConfig.vidWidth, -cgs->glConfig.vidHeight );
		glVertex2f( cgs->glConfig.vidWidth, -cgs->glConfig.vidHeight );
		glVertex2f( cgs->glConfig.vidWidth, cgs->glConfig.vidHeight );
		glVertex2f( -cgs->glConfig.vidWidth, cgs->glConfig.vidHeight );
	glEnd();

	// Generate the dark tint texture

	glBlendFunc( GL_DST_COLOR, GL_ZERO );

	glBindTexture( GL_TEXTURE_2D, m_uiDarkTexture );

	glBegin( GL_QUADS );
		glColor3fv( dark );
		glVertex2f( -cgs->glConfig.vidWidth, -cgs->glConfig.vidHeight );
		glVertex2f( cgs->glConfig.vidWidth, -cgs->glConfig.vidHeight );
		glVertex2f( cgs->glConfig.vidWidth, cgs->glConfig.vidHeight );
		glVertex2f( -cgs->glConfig.vidWidth, cgs->glConfig.vidHeight );
	glEnd();

	// Generate the desaturation texture

	GLint viewport[4];

	int HEIGHT = cgs->glConfig.vidHeight;
	int WIDTH = cgs->glConfig.vidWidth;
	float biggest = 0.0f;

	for( int i = 0; i < 3; i++ )
	{
		if( r_filmDesaturation.vector_current[i] > biggest ) {
			biggest = r_filmDesaturation.vector_current[i];
		}
	}

	// Optimize performances
	/*if( biggest > 0.0f )
	{
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		glBindTexture( GL_TEXTURE_2D, m_uiDesaturationTexture );

		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

		glCopyTexImage2D( GL_TEXTURE_2D, 0, GL_LUMINANCE, 0, 0, cgs->glConfig.vidWidth, cgs->glConfig.vidHeight, 0 );

		glColor4f( r_filmDesaturation.vector_current[0], r_filmDesaturation.vector_current[1], r_filmDesaturation.vector_current[2], biggest );
		glBegin( GL_QUADS );
			glTexCoord2f( 0, 1 ); glVertex2f( 0, 0 );
			glTexCoord2f( 1, 1 ); glVertex2f( cgs->glConfig.vidWidth, 0 );
			glTexCoord2f( 1, 0 ); glVertex2f( cgs->glConfig.vidWidth, cgs->glConfig.vidHeight );
			glTexCoord2f( 0, 0 ); glVertex2f( 0, cgs->glConfig.vidHeight );
		glEnd();

		glCopyTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, cgs->glConfig.vidWidth, cgs->glConfig.vidHeight, 0 );
	}*/

	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	glDisable( GL_BLEND );
}

#endif

void R_SetSkyColor( vec3_t color )
{
	glEnable( GL_BLEND );
	glColor3f( color[0], color[1], color[2] );
}

void R_SetSceneColor2(float intensity, vec3_t color)
{
	glColor3ub((int)(color[0]*255.0f),(int)(color[1]*255.0f),(int)(color[2]*255.0f));
	glVertex2f(-1,-1);
	glColor3ub((int)(color[0]*255.0f),(int)(color[1]*255.0f),(int)(color[2]*255.0f));
	glVertex2f(1,-1);
	glColor3ub((int)(color[0]*255.0f),(int)(color[1]*255.0f),(int)(color[2]*255.0f));
	glVertex2f(1,1);
}

/*
vvar_t * R_GetVisionVar( const char * name, regVis_t **vis = NULL )
{
	for( int i = 0; i < sizeof( vvars ) / sizeof( vvars[0] ); i++ )
	{
		if( _stricmp( vvars[i].name, name ) == 0 )
		{
			if( vis != NULL ) {
				*vis = &vvars[i];
			}

			return vvars[i].vvar;
		}
	}

	return NULL;
}


qboolean R_ProcessVisionVar( const char * variable, const char * value )
{
	vvar_t *var;
	const char *vec1, *vec2, *vec3;
	qboolean isVector = false;
	char buffer[MAX_STRINGTOKENS];
	regVis_t *vis = NULL;

	if( value == NULL ) {
		return false;
	}

	var = R_GetVisionVar( variable, &vis );

	if( var == NULL ) {
		return false;
	}

	strcpy( buffer, value );

	vec1 = strtok( ( char * )buffer, " " );

	if( vec1 != NULL && isdigit( vec1[0] ) )
	{
		vec2 = strtok( NULL, " " );

		if( vec2 != NULL && isdigit( vec2[0] ) )
		{
			vec3 = strtok( NULL, " " );

			if( vec3 != NULL && isdigit( vec3[0] ) ) {
				isVector = true;
			}
		}
	}

	if( !isVector )
	{
		var->integer_target = atoi( value );
		var->value = atof( value );

		var->vector[0] = var->value;
		var->vector[1] = var->value;
		var->vector[2] = var->value;
	}
	else
	{
		// Cast values into vector
		var->vector[0] = atof( vec1 );
		var->vector[1] = atof( vec2 );
		var->vector[2] = atof( vec3 );

		var->value = var->vector[0];
	}

	if( !render_inited )
	{
		VectorCopy( var->vector, var->vector_current );
		VectorCopy( var->vector, var->vector_start );

		var->value_current = var->value;
		var->value_start = var->value;
		var->integer = var->integer_target;
	}
	else
	{
		var->value_start = var->value_current;

		VectorCopy( var->vector_current, var->vector_start );
	}

	var->bProcessed = true;

	return true;
}
*/

char * R_LineParseComment( char * line )
{
	char * new_line = line;
	int i = 0;
	qboolean quotation = false;

	while( *new_line != '\n' && *new_line != '\0' )
	{
		if( *new_line == '"' ) {
			quotation = true;
		}

		if( *new_line == '/' && *( new_line + 1 ) == '/' )
		{
			if( i == 0 ) {
				*new_line = '\0';
			}

			return new_line;
		}

		//if( *new_line == '\t' ) {
		//	*new_line = ' ';
		//}

		if( *new_line == ' ' && !quotation ){
			*new_line = '\t';
		}

		new_line++;
		i++;
	}

	return line;
}

void R_ProcessBlur( float blur_level, float fade_time )
{
	/*m_blur_startlevel = m_blur_currentlevel;
	m_blur_level = blur_level;
	m_blur_fadetime = fade_time * 1000.0f;

	m_blur_currentTime = 0.0f;*/

	m_blurTimer.SetTime( fade_time );
	r_blurlevel.setFloatValue( blur_level );
}

void R_ProcessVision( str vision_name, float fade_time, float phase, str type )
{
	Script m_visionScript;
	VisionClass *visionVariable;
	int warnings;

	str filename = type + "/" + vision_name + "." + type;

	m_visionTimer.SetTime( fade_time );

	if( phase )
	{
		m_visionTimer.SetPhase( phase );
	}

	if( vision_name.icmp( vision_current ) == 0 )
	{
#ifdef DEBUG
		cgi.Printf( "DEBUG: Vision file '%s' was already processed!\n", filename.c_str() );
#endif
		return;
	}

	m_visionScript.LoadFile( filename );

	if( !m_visionScript.isValid() )
	{
		Com_Printf( "Couldn't find vision '%s'! Setting to the default vision\n", vision_name.c_str() );

		filename = type + "/" + "default" + "." + type;

		m_visionScript.LoadFile( filename );

		if( !m_visionScript.isValid() )
		{
			Com_Printf( "Couldn't find the default vision!\n" );
			return;
		}

		vision_name = "default";
	}

	warnings = 0;

	while( m_visionScript.TokenAvailable( true ) )
	{
		str variable = m_visionScript.GetToken( false );
		str value = m_visionScript.GetString( false );

		visionVariable = VisionClass::Get( variable );

		if( visionVariable != NULL && visionVariable->GetType() == "vision" )
		{
			visionVariable->setValue( value );
			visionVariable->fProcessed = true;
		}
		else
		{
			cgi.Printf( "WARNING : unknown var '%s' in file '%s'\n", variable.c_str(), filename.c_str() );
			warnings++;
		}
	}

	if( warnings ) {
		cgi.Printf( "Found %d warning(s) in file '%s'\n", warnings, filename.c_str() );
	}

	for( int i = 0; i < m_visionVar.NumObjects(); i++ )
	{
		visionVariable = m_visionVar[ i ];

		if( visionVariable->GetType() != "vision" ) {
			continue;
		}

		if( !visionVariable->fProcessed || !visionVariable->isNewEnabled() ) {
			visionVariable->setValue( visionVariable->defaultValue() );
		}

		visionVariable->fProcessed = false;
	}

	vision_current = vision_name;

#ifdef DEBUG
	cgi.Printf( "DEBUG: Processed vision file '%s' with %d warning(s)\n", filename.c_str(), warnings );
#endif
}

#if 0
void R_ProcessVision( str vision_name, float fade_time )
{
	char *content = NULL;
	char **line = NULL;
	char *variable, *value, *pch;
	int i = 0, num_lines = 0;
	int errors = 0;
	int length = 0;
	VisionClass *var;

	if( strlen( vision_name ) >= 64 )
	{
		cgi.Printf( "vision exceeded the maximum length!\n" );
		return;
	}

	str filename = "vision/" + vision_name + ".vision";

	if( vision_name.icmp( vision_current ) == 0 )
	{
#ifdef DEBUG
		cgi.Printf( "DEBUG: Vision file '%s' was already processed!\n", filename.c_str() );
#endif
		return;
	}

	length = cgi.FS_ReadFile( filename.c_str(), ( void ** )&content );

	if( length < 0 )
	{
		Com_Printf( "Couldn't find vision '%s'! Setting to the default vision\n", vision_name.c_str() );
		length = cgi.FS_ReadFile( "vision/default.vision", ( void ** )&content );

		if( length < 0 )
		{
			Com_Printf( "Couldn't find the default vision!\n" );
			return;
		}

		filename = "vision/default.vision";
		vision_name = "default";
	}

	//for( int i = 0; i < sizeof( vvars ) / sizeof( vvars[0] ); i++ ) {
	//	vvars[i].vvar->bProcessed = false;
	//}

	pch = strtok( content, "\n" );

	while( pch != NULL )
	{
		line = ( char ** )realloc( line, sizeof( DWORD ) * ( i + 1 ) );
		line[i] = R_LineParseComment( pch );
		i++;
		num_lines++;

		pch = strtok( NULL, "\n" );
	}

	i = 0;

	while( i < num_lines )
	{
		value = NULL;

		// This means we're at a comment
		if( line[i][0] == '\0' )
		{
			i++;
			continue;
		}

		variable = strtok( line[i], "\t" );

		if( variable != NULL ) {
			value = strtok( NULL, "\t\"" );
		}

		if( value != NULL )
		{
			// Process a vision variable
			/*if( !R_ProcessVisionVar( variable, value ) )
			{
				cgi.Printf( "WARNING : unknown var '%s' in file '%s'\n", variable, filename.c_str() );
				errors++;
			}*/

			var = VisionClass::Get( variable );

			if( var != NULL && var->GetType() == "vision" )
			{
				var->setValue( value );
				var->fProcessed = true;
			}
			else
			{
				cgi.Printf( "WARNING : unknown var '%s' in file '%s'\n", variable, filename.c_str() );
				errors++;
			}
		}

		i++;
	}

	if( errors ) {
		cgi.Printf( "Found %d warnings in file '%s'\n", errors, filename.c_str() );
	}

	for( int i = 0; i < m_visionVar.NumObjects(); i++ )
	{
		var = m_visionVar[ i ];

		if( var->GetType() != "vision" ) {
			continue;
		}

		if( !var->fProcessed || !var->isNewEnabled() ) {
			var->setValue( var->defaultValue() );
		}
		
		var->fProcessed = false;
	}

	/*for( int i = 0; i < sizeof( vvars ) / sizeof( vvars[0] ); i++ )
	{
		// ALL variables must be processed
		if( !vvars[i].vvar->bProcessed ) {
			R_ProcessVisionVar( vvars[i].name, vvars[i].value );
		}
	}*/

	vision_current = vision_name;

#ifdef DEBUG
	cgi.Printf( "DEBUG: Processed vision file '%s' with %d error(s)\n", filename.c_str(), errors );
#endif

	m_visionTimer.SetTime( fade_time );
}
#endif

// Vision class

VisionClass *VisionClass::Get( str n )
{
	for( int i = 0; i < m_visionVar.NumObjects(); i++ )
	{
		VisionClass *variable = m_visionVar[ i ];

		if( variable->GetName().icmp( n ) == 0 ) {
			return variable;
		}
	}

	return NULL;
}

void VisionClass::VisionInit( const char *n, const char *v, ScriptTimer *globalTimer )
{
	name = n;

	defaultval = v;

	fProcessed = false;

	if( globalTimer == NULL ) {
		globalTimer = &m_visionTimer;
	}

	m_timer = globalTimer;

	m_visionVar.AddObject( this );

	setValue( v );
}

VisionClass::VisionClass()
{
	value = 0.f;
	value_start = 0.f;
	value_target = 0.f;

	fProcessed = false;
}

VisionClass::VisionClass( const char *n, const char *v, ScriptTimer *globalTimer )
{
	VisionInit( n, v, globalTimer );
}

void VisionClass::GlobalArchive( Archiver &arc )
{
	for( int i = 0; i < m_visionVar.NumObjects(); i++ ) {
		arc.ArchiveObject( m_visionVar[ i ] );
	}
}

void VisionClass::Archive( Archiver &arc )
{
	Class::Archive( arc );

	arc.ArchiveString( &string );

	arc.ArchiveObject( m_timer );

	arc.ArchiveFloat( &value );
	arc.ArchiveFloat( &value_start );
	arc.ArchiveFloat( &value_target );

	arc.ArchiveBoolean( &fProcessed );
}

str VisionClass::GetName()
{
	return name;
}

str VisionClass::defaultValue()
{
	return defaultval;
}

float VisionClass::floatValue()
{
	return value;
}

str VisionClass::stringValue()
{
	return string;
}

ScriptTimer *VisionClass::GetTimer()
{
	return m_timer;
}

str VisionClass::GetType()
{
	return "";
}

qboolean VisionClass::isBasic()
{
	return true;
}

qboolean VisionClass::isEnabled()
{
	return true;
}

qboolean VisionClass::isNewEnabled()
{
	return true;
}

void VisionClass::setFloatValue( float v )
{
	value_target = v;

	if( !render_inited )
	{
		value = value_target;
		value_start = value_target;
	} else {
		value_start = value;
	}
}

void VisionClass::setValue( str v )
{
	float x;

	string = v;

	if( isdigit( *v ) ) {
		x = atof( v );
	} else {
		x = 0.f;
	}

	value_target = x;

	if( !render_inited )
	{
		value = value_target;
		value_start = value_target;
	} else {
		value_start = value;
	}
}

void VisionClass::Think()
{
	if( !m_timer->isEnabled() ) {
		m_timer->Enable();
	}

	value = m_timer->LerpValue( value_start, value_target );
}

VisionActivator::VisionActivator( const char *n, const char *v, const char *t, ScriptTimer *globalTimer )
{
	VisionInit( n, v, globalTimer );

	if( t == NULL ) {
		t = "none";
	}

	vistype = t;
}

VisionActivator::VisionActivator()
{

}

void VisionActivator::Archive( Archiver &arc )
{
	VisionClass::Archive( arc );
}

str VisionActivator::GetType()
{
	return vistype;
}

qboolean VisionActivator::isBasic()
{
	return false;
}

qboolean VisionActivator::isEnabled()
{
	return value > 0.0f || value_target > 0.0f;
}

qboolean VisionActivator::isNewEnabled()
{
	return value_target > 0.0f;
}

void VisionActivator::setValue( str v )
{
	float floatvalue = ( float )atoi( v );
	char newval[ MAX_STRING_TOKENS ];

	sprintf( newval, "%f", floatvalue );

	VisionClass::setValue( newval );
}

VisionVariable::VisionVariable( const char *n, const char *v, VisionActivator *a )
{
	VisionInit( n, v, a->GetTimer() );

	activator = a;
}

VisionVariable::VisionVariable()
{

}

void VisionVariable::Archive( Archiver &arc )
{
	VisionClass::Archive( arc );

	arc.ArchiveVector( &vector );

	arc.ArchiveVector( &vector_start );
	arc.ArchiveVector( &vector_target );
}

VisionActivator *VisionVariable::GetActivator()
{
	return activator;
}

ScriptTimer *VisionVariable::GetTimer()
{
	return activator->GetTimer();
}

str VisionVariable::GetType()
{
	return activator->GetType();
}

Vector VisionVariable::vectorValue()
{
	return vector;
}

qboolean VisionVariable::isBasic()
{
	return false;
}

qboolean VisionVariable::isEnabled()
{
	if( !activator ) {
		return true;
	}

	return activator->isEnabled();
}

qboolean VisionVariable::isNewEnabled()
{
	if( !activator ) {
		return true;
	}

	return activator->isNewEnabled();
}

void VisionVariable::setValue( str v )
{
	float x = 0.f, y = 0.f, z = 0.f;
	char buffer[ MAX_STRING_TOKENS ];
	qboolean isVector;

	if( sscanf( v, "%f %f %f", &x, &y, &z ) == 3 ) {
		isVector = true;
	} else {
		isVector = false;
	}

	string = v;

	value_target = x;

	if( !isVector ) {
		vector_target = Vector( x, x, x );
	} else {
		// Cast values into vector
		vector_target = Vector( x, y, z );
	}

	if( !render_inited )
	{
		vector = vector_target;
		vector_start = vector_target;

		value = value_target;
		value_start = value_target;
	}
	else
	{
		value_start = value;
		vector_start = vector;
	}
}

void VisionVariable::Think()
{
	if( activator == NULL ) {
		return;
	}

	ScriptTimer *t = GetTimer();

	if( !t->isEnabled() ) {
		t->Enable();
	}

	vector = t->LerpValue( vector_start, vector_target );
	value = t->LerpValue( value_start, value_target );
}

CLASS_DECLARATION( Class, VisionActivator, NULL )
{
	{ NULL, NULL }
};

CLASS_DECLARATION( Class, VisionClass, NULL )
{
	{ NULL, NULL }
};

CLASS_DECLARATION( Class, VisionVariable, NULL )
{
	{ NULL, NULL }
};
