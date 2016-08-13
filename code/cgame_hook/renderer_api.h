#ifndef _renderer_api_h
#define _renderer_api_h

#include <stdio.h>

#ifdef _CGAME_DLL
#include "cgamex86.h"
#else
#include "gamex86.h"
#endif

#ifdef WANT_OPENGL
#include "ogl\opengl_api.h"
#endif

#define QUAT_EPSILON 0.00001

#ifndef M_PI
#define M_PI					3.1415926535897932384626433832795f
#endif

#define random()				((rand () & 0x7fff) / ((float)0x7fff))
#define crandom()				(2.0 * (random() - 0.5))

#define DEG2RAD( a )			( ( (a) * M_PI ) / 180.0F )
#define RAD2DEG( a )			( ( (a) * 180.0f ) / M_PI )

#define	ANGLE2SHORT(x)			((int)((x)*65536/360) & 65535)
#define	SHORT2ANGLE(x)			((x)*(360.0/65536))

#define SUBMAX( a, b )			( a > b ? ( a - b ) : ( b - a ) )
#define SUBMIN( a, b )			( a < b ? ( a - b ) : ( b - a ) )

/*#define DotProduct(x,y)			((x)[0]*(y)[0]+(x)[1]*(y)[1]+(x)[2]*(y)[2])
#define VectorSubtract(a,b,c)	((c)[0]=(a)[0]-(b)[0],(c)[1]=(a)[1]-(b)[1],(c)[2]=(a)[2]-(b)[2])
#define VectorAdd(a,b,c)		((c)[0]=(a)[0]+(b)[0],(c)[1]=(a)[1]+(b)[1],(c)[2]=(a)[2]+(b)[2])
#define VectorCopy(a,b)			((b)[0]=(a)[0],(b)[1]=(a)[1],(b)[2]=(a)[2])
#define	VectorScale(v, s, o)	((o)[0]=(v)[0]*(s),(o)[1]=(v)[1]*(s),(o)[2]=(v)[2]*(s))
#define	VectorMA(v, s, b, o)	((o)[0]=(v)[0]+(b)[0]*(s),(o)[1]=(v)[1]+(b)[1]*(s),(o)[2]=(v)[2]+(b)[2]*(s))
#define VectorCompare(a,b)		(((a)[0]==(b)[0])&&((a)[1]==(b)[1])&&((a)[2]==(b)[2]))

#define VectorClear(a)			((a)[0]=(a)[1]=(a)[2]=0)
#define VectorNegate(a,b)		((b)[0]=-(a)[0],(b)[1]=-(a)[1],(b)[2]=-(a)[2])
#define VectorSet(v, x, y, z)	((v)[0]=(x), (v)[1]=(y), (v)[2]=(z))
#define Vector4Copy(a,b)		((b)[0]=(a)[0],(b)[1]=(a)[1],(b)[2]=(a)[2],(b)[3]=(a)[3])*/

#define VectorInverse(a)		((a)[0]=-(a)[0],(a)[1]=-(a)[1],(a)[2]=-(a)[2])

#define PITCH					0
#define YAW						1
#define ROLL					2

#define RED						0
#define GREEN					1
#define BLUE					2

#define MAX_EFFECTS				10
#define MAX_EFFECTS_LENGTH		64

/* Animation flags */
#define ANIM_RANDOM				0x01
#define ANIM_DELTADRIVEN		0x02
#define ANIM_DONTREPEATE		0x03
#define ANIM_AUTOSTEPS_DOG		0x04
#define ANIM_DEFAULTANGLES		0x08
#define ANIM_NOTIMECHECK		0x10
#define ANIM_AUTOSTEPS_WALK		0x14
#define ANIM_AUTOSTEPS_RUN		0x1C

#define GLS_SRCBLEND_ZERO						0x00000001
#define GLS_SRCBLEND_ONE						0x00000002
#define GLS_SRCBLEND_DST_COLOR					0x00000003
#define GLS_SRCBLEND_ONE_MINUS_DST_COLOR		0x00000004
#define GLS_SRCBLEND_SRC_ALPHA					0x00000005
#define GLS_SRCBLEND_ONE_MINUS_SRC_ALPHA		0x00000006
#define GLS_SRCBLEND_DST_ALPHA					0x00000007
#define GLS_SRCBLEND_ONE_MINUS_DST_ALPHA		0x00000008
#define GLS_SRCBLEND_ALPHA_SATURATE				0x00000009
#define	GLS_SRCBLEND_BITS						0x0000000f

#define GLS_DSTBLEND_ZERO						0x00000010
#define GLS_DSTBLEND_ONE						0x00000020
#define GLS_DSTBLEND_SRC_COLOR					0x00000030
#define GLS_DSTBLEND_ONE_MINUS_SRC_COLOR		0x00000040
#define GLS_DSTBLEND_SRC_ALPHA					0x00000050
#define GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA		0x00000060
#define GLS_DSTBLEND_DST_ALPHA					0x00000070
#define GLS_DSTBLEND_ONE_MINUS_DST_ALPHA		0x00000080
#define	GLS_DSTBLEND_BITS						0x000000f0

#define GLS_DEPTHMASK_TRUE						0x00000100

#define	GLS_NODEPTH								0x00000200
#define GLS_DEPTH								0x00000300

#define GLS_POLYMODE_LINE						0x00001000

#define GLS_DEPTHTEST_DISABLE					0x00010000
#define GLS_DEPTHFUNC_EQUAL						0x00020000

#define GLS_ATEST_GT_0							0x10000000
#define GLS_ATEST_LT_80							0x20000000
#define GLS_ATEST_GE_80							0x40000000
#define	GLS_ATEST_BITS							0x70000000

#define GLS_DEFAULT								GLS_DEPTHMASK_TRUE

#define ELM_MODELS								0x004b56b3
#define ELM_WALLS								0x004b598e

#define ELM_FLAT								0x004b3993
#define ELM_ENTS								0x004b56b3
#define ELM_WORLD								0x004b598e

static const char *anim_cmd[] =
{
	"weight",
	"deltadriven",
	"default_angles",
	"notimecheck",
	"crossblend",
	"dontrepeate",
	"random",
	"autosteps_run",
	"autosteps_walk",
	"autosteps_dog",
};

static const char *effectName[MAX_EFFECTS] =
{
	"bloom",
	"blur",
	"darkwhite",
	"dof",
	"invert",
	"scenecolor",
	"thermal",
	"wallhack",
	"wave",
};

qboolean __inline R_IsEffect(const char * fxName)
{
	int i;

	/* Check if the name is a valid effect name */
	for(i=0;i<MAX_EFFECTS;i++)
	{
		if( _stricmp( fxName, effectName[ i ] ) == 0 )
			return true;
	}

	return false;
}

typedef struct vvar_s
{
	char	*name;
	int		integer;
	int		integer_target;
	float	value;
	float	value_current;
	float	value_start;
	vec3_t	vector;
	vec3_t	vector_current;
	vec3_t	vector_start;
	qboolean bProcessed;
} vvar_t;

typedef struct {
	vvar_t		*vvar;
	const char	*name;
	const char	*value;
	vec4_t		vector;
} regVis_t;

typedef struct effect_s
{
	char effectName[MAX_EFFECTS_LENGTH];
	float intensity;
	vec3_t color;
	vec3_t t_color;
	vec3_t o_color;
	float fadetime;
	qboolean bRemove;
	float param[2];
} effect_t;

typedef struct tikiFrame_s
{
	vec3_t bounds[ 2 ];
	vec3_t scale;
	vec3_t offset;
	vec3_t delta;
	float radius;
	float frameTime;

} tikiFrame_t;

typedef struct refDef_s
{
	int x;
	int y;
	int width;
	int height;
	float fovX;
	float fovY;
	vec3_t viewOrg;
	vec3_t viewAxis[3];
	int time;
	int rdFlags;
	unsigned char areaMask[32];
	float farplaneDistance;
	float farplaneColor[3];
	qboolean farplaneCull;
	qboolean skyPortal;
	float skyAlpha;
	vec3_t skyOrigin;
	vec3_t skyAxis[3];

} refDef_t;

typedef struct animation_s
{
	unsigned char	un1[0x184];		// 0x000

	float	weight;			// 0x184
	float	crossblend;		// 0x188
	int		animFlags;		// 0x18C

} animation_t;

typedef struct bmodel_s
{
	char		un1[ 40 ];
	void		*lightmap;
	bool		haslightmap;
} bmodel_t;

typedef struct model_s
{
	char		un1[ 140 ];
	bmodel_t	*bmodel;
} model_t;

typedef struct debugLine_s
{
	vec3_t start;
	vec3_t end;
	float color[ 3 ];
	float alpha;
	float width;
	short unsigned int factor;
	short unsigned int pattern;

} debugLine_t;

typedef struct debugString_s
{
	char text[ MAX_QPATH ];
	vec3_t pos;
	float scale;
	vec4_t color;

} debugString_t;

typedef struct polyVert_s
{
	float xyz[ 3 ];
	float st[ 2 ];
	unsigned char modulate[ 4 ];

} polyVert_t;

typedef enum dlightType_e
{
	LENSFLARE = 1,
	VIEWLENSFLARE,
	ADDITIVE = 4

} dlightType_t;

typedef enum stereoFrame_e
{
	STEREO_CENTER,
	STEREO_LEFT,
	STEREO_RIGHT

} stereoFrame_t;

typedef struct letterLoc_s
{
	vec2_t pos;
	vec2_t size;

} letterLoc_t;

typedef struct fontHeader_s
{
	int indirection[ 256 ];
	letterLoc_t locations[ 256 ];
	char name[ MAX_QPATH ];
	float height;
	float aspectRatio;
	void *shader;
	int trHandle;

} fontHeader_t;

typedef struct media_s
{
	qhandle_t backTileShader;
	qhandle_t lagometerShader;
	qhandle_t shadowMarkShader;
	qhandle_t footShadowMarkShader;
	qhandle_t wakeMarkShader;
	qhandle_t pausedShader;
	qhandle_t levelExitShader;
	qhandle_t zoomOverlayShader;
	qhandle_t kar98TopOverlayShader;
	qhandle_t kar98BottomOverlayShader;
	qhandle_t binocularsOverlayShader;
	fontHeader_t *hudDrawFont;
	fontHeader_t *attackerFont;
	fontHeader_t *objectiveFont;
	qhandle_t objectivesBackShader;
	qhandle_t checkedBoxShader;
	qhandle_t uncheckedBoxShader;

} media_t;

typedef void( *GL_State_f )( int state );
extern GL_State_f GL_State;

typedef void( *GL_Bind_f )( void * image );
extern GL_Bind_f GL_Bind;

typedef void( *GL_Setup_f )( void );
extern GL_Setup_f GL_Setup;

typedef struct refImport_s
{
	void ( *Printf )( int type, const char *format, ... );
	void ( *Error )( errorParm_t errorCode, const char *format, ... );
	int ( *Milliseconds )( );
	char * ( *LV_ConvertString )( const char *string );
	void ( *Hunk_Clear )( );
	void * ( *Hunk_Alloc )( size_t size );
	void * ( *Hunk_AllocateTempMemory )( size_t size );
	void ( *Hunk_FreeTempMemory )( void *buffer );
	void * ( *Malloc )( size_t size );
	void ( *Free )( void *ptr );
	void ( *Clear )( );
	cvar_t * ( *Cvar_Get )( char *varName, char *varValue, int varFlags );
	void ( *Cvar_Set )( char *varName, char *varValue );
	void ( *Cvar_SetDefault )( cvar_t *var, char *varValue );
	void ( *Cmd_AddCommand )( char *cmdName, xcommand_t cmdFunction );
	void ( *Cmd_RemoveCommand )( char *cmdName );
	int ( *Argc )( );
	char * ( *Argv )( int arg );
	void ( *Cmd_ExecuteText )( cbufExec_t execWhen, char *text );
	void ( *CM_DrawDebugSurface )( void ( *drawPoly )( ) );
	int ( *FS_OpenFile )( char *qpath, fsMode_t mode );
	int ( *FS_Read )( void *buffer, int len, fileHandle_t fileHandle );
	void ( *FS_CloseFile )( fileHandle_t fileHandle );
	int ( *FS_Seek )( fileHandle_t fileHandle, long int offset, fsOrigin_t origin );
	int ( *FS_FileIsInPAK )( char *fileName, int *checksum );
	int ( *FS_ReadFile )( char *qpath, void **buffer );
	int ( *FS_ReadFileEx )( char *qpath, void **buffer, qboolean quiet );
	void ( *FS_FreeFile )( void *buffer );
	const char ** ( *FS_ListFiles )( const char *qpath, const char *extension, qboolean wantSubs, int *numFiles );
	void ( *FS_FreeFileList )( char **list );
	int ( *FS_WriteFile )( char *qpath, void *buffer, int size );
	qboolean ( *FS_FileExists )( char *file );
	void ( *CM_BoxTrace )( trace_t *results, vec3_t start, vec3_t end, vec3_t mins, vec3_t maxs, int model, int brushMask, int cylinder );
	int ( *CM_TerrainSquareType )( int terrainPatch, int i, int j );
	char * ( *CM_EntityString )( );
	char * ( *CM_MapTime )( );
	int ( *CG_PermanentMark )( vec3_t origin, vec3_t dir, float orientation, float sScale, float tScale, float red, float green, float blue, float alpha, qboolean doLighting, float sCenter, float tCenter, markFragment_t *markFragments, void *polyVerts );
	int ( *CG_PermanentTreadMarkDecal )( treadMark_t *treadMark, qboolean startSegment, qboolean doLighting, markFragment_t *markFragments, void *polyVerts );
	int ( *CG_PermanentUpdateTreadMark )( treadMark_t *treadMark, float alpha, float minSegment, float maxSegment, float maxOffset, float texScale );
	void ( *CG_ProcessInitCommands )( dtiki_t *tiki, refEntity_t *ent );
	void ( *CG_EndTiki )( dtiki_t *tiki );
	void ( *SetPerformanceCounters )( int totalTris, int totalVerts, int totalTexels, int worldTris, int worldVerts, int characterLights );

	debugLine_t **debugLines;
	int *numDebugLines;
	debugString_t **debugStrings;
	int *numDebugStrings;

	orientation_t ( *TIKI_OrientationInternal )( dtiki_t *tiki, int entNum, int tagNum, float scale );
	qboolean ( *TIKI_IsOnGroundInternal )( dtiki_t *tiki, int entNum, int tagNum, float thresHold );
	void ( *TIKI_SetPoseInternal )( void *skeletor, frameInfo_t *frameInfo, int *boneTag, vec4_t boneQuat[4], float actionWeight );
	void * ( *TIKI_Alloc )( int size );
	float ( *GetRadiusInternal )( dtiki_t *tiki, int entNum, float scale );
	float ( *GetCentroidRadiusInternal )( dtiki_t *tiki, int entNum, float scale, vec3_t centroid );
	void ( *GetFrameInternal )( dtiki_t *tiki, int entNum, skelAnimFrame_t *newFrame );

} refImport_t;

typedef struct refExport_s
{
	void ( *Shutdown )( );
	void ( *BeginRegistration )( glconfig_t *glConfigOut );
	qhandle_t ( *RegisterModel )( const char *name );
	qhandle_t ( *SpawnEffectModel )( const char *name, vec3_t pos, vec3_t axis[3] );
	qhandle_t ( *RegisterServerModel )( const char *name );
	void ( *UnregisterServerModel )( qhandle_t model );
	qhandle_t ( *RegisterShader )( const char *name );
	qhandle_t ( *RegisterShaderNoMip )( const char *name );
	qhandle_t ( *RefreshShaderNoMip )( const char *name );
	void ( *EndRegistration )( );
	void ( *FreeModels )( );
	void ( *SetWorldVisData )( unsigned char *vis );
	void ( *LoadWorld )( char *name );
	void ( *PrintBSPFileSizes )( );
	int ( *MapVersion )( );
	void ( *ClearScene )( );
	void ( *AddRefEntityToScene )( refEntity_t *ent, int parentEntityNumber );
	void ( *AddRefSpriteToScene )( refEntity_t *ent );
	qboolean ( *AddPolyToScene )( qhandle_t hShader, int numVerts, polyVert_t *verts, int renderFx );
	void ( *AddTerrainMarkToScene )( int terrainIndex, qhandle_t hShader, int numVerts, polyVert_t *verts, int renderfx );
	void ( *AddLightToScene )( vec3_t org, float intensity, float r, float g, float b, dlightType_t type );
	void ( *RenderScene )( refDef_t *fd );
	refEntity_t * ( *GetRenderEntity )( int entityNumber );
	void ( *SavePerformanceCounters )( );
	void ( *SetColor )( vec4_t color );
	void ( *Set2DWindow )( int x, int y, int w, int h, float left, float right, float bottom, float top, float n, float f );
	void ( *DrawStretchPic )( float x, float y, float w, float h, float s1, float t1, float s2, float t2, qhandle_t hShader );
	void ( *DrawTilePic )( float x, float y, float w, float h, qhandle_t hShader );
	void ( *DrawTilePicOffset )( float x, float y, float w, float h, qhandle_t hShader, int offsetX, int offsetY );
	void ( *DrawTrianglePic )( vec2_t *points, vec2_t *texCoords, qhandle_t hShader );
	void ( *DrawBackground )( int cols, int rows, int bgr, unsigned char *data );
	void ( *DrawStretchRaw )( int x, int y, int w, int h, int cols, int rows, int components, unsigned char *data );
	void ( *DebugLine )( vec3_t start, vec3_t end, float r, float g, float b, float alpha );
	void ( *DrawBox )( float x, float y, float w, float h );
	void ( *AddBox )( float x, float y, float w, float h );
	void ( *BeginFrame )( stereoFrame_t stereoFrame );
	void ( *Scissor )( int x, int y, int width, int height );
	void ( *DrawLineLoop )( vec2_t *points, int count, int stippleFactor, int stippleMask );
	void ( *EndFrame )( int *frontEndMsec, int *backEndMsec );
	int ( *MarkFragments )( int numPoints, vec3_t *points, vec3_t projection, int maxPoints, float *pointBuffer, int maxFragments, markFragment_t *fragmentBuffer, float radiusSquared );
	int ( *MarkFragmentsForInlineModel )( clipHandle_t bmodel, vec3_t angles, vec3_t origin, int numPoints, vec3_t *points, vec3_t projection, int maxPoints, float *pointBuffer, int maxFragments, markFragment_t *fragmentBuffer, float radiusSquared );
	void ( *GetInlineModelBounds )( int index, vec3_t mins, vec3_t maxs );
	void ( *GetLightingForDecal )( vec3_t light, vec3_t facing, vec3_t origin );
	void ( *GetLightingForSmoke )( vec3_t light, vec3_t origin );
	int ( *R_GatherLightSources )( vec3_t pos, vec3_t *lightPos, vec3_t *lightIntensity, int maxLights );
	void ( *ModelBounds )( qhandle_t handle, vec3_t mins, vec3_t maxs );
	float ( *ModelRadius )( qhandle_t handle );
	dtiki_t * ( *R_Model_GetHandle )( qhandle_t handle );
	void ( *DrawString )( fontHeader_t *font, char *text, float x, float y, int maxLen, qboolean virtualScreen );
	float ( *GetFontHeight )( fontHeader_t *font );
	float ( *GetFontStringWidth )( fontHeader_t *font, const char *string );
	fontHeader_t * ( *LoadFont )( char *name );
	void ( *SwipeBegin )( float thisTime, float life, qhandle_t hShader );
	void ( *SwipePoint )( vec3_t point1, vec3_t point2, float time );
	void ( *SwipeEnd )( );
	void ( *SetRenderTime )( int t );
	float ( *Noise )( vec3_t vec, int len );
	qboolean ( *SetMode )( int mode, glconfig_t *glConfig );
	void ( *SetFullscreen )( qboolean fullScreen );
	int ( *GetShaderWidth )( qhandle_t hShader );
	int ( *GetShaderHeight )( qhandle_t hShader );
	char * ( *GetGraphicsInfo )( );
	void ( *ForceUpdatePose )( refEntity_t *model );
	orientation_t ( *TIKI_Orientation )( refEntity_t *model, int tagNum );
	qboolean ( *TIKI_IsOnGround )( refEntity_t *model );
	void ( *SetFrameNumber )( int frameNumber );

} refExport_t;

extern refImport_t ri;
extern refExport_t re;

extern effect_t * effect_list[MAX_EFFECTS];

extern cvar_t	*r_debug;

extern cvar_t	*r_dof;

extern cvar_t	*r_texFilterAniso;

extern cvar_t	*r_tweakBlurX;
extern cvar_t	*r_tweakBlurY;
extern cvar_t	*r_tweakBlurPasses;

extern cvar_t	*r_glowEnable;
extern cvar_t	*r_glowQuality;
extern cvar_t	*r_glowSamples;

extern cvar_t	*r_ssao;

extern cvar_t	*r_autoluminance;
extern cvar_t	*r_fxaa;
extern cvar_t	*r_test_color;
extern cvar_t	*r_test_bump;

#ifdef _CGAME_DLL

#if 0
// vision-specific variables
extern vvar_t	r_glow;
extern vvar_t	r_glowRadius0;
extern vvar_t	r_glowRadius1;
extern vvar_t	r_glowBloomCutoff;
extern vvar_t	r_glowBloomDesaturation;
extern vvar_t	r_glowBloomIntensity0;
extern vvar_t	r_glowBloomIntensity1;
extern vvar_t	r_glowBloomStreakX;
extern vvar_t	r_glowBloomStreakY;

extern vvar_t	r_filmEnable;
extern vvar_t	r_filmContrast;
extern vvar_t	r_filmBrightness;
extern vvar_t	r_filmDesaturation;
extern vvar_t	r_filmSaturation;
extern vvar_t	r_filmHue;
extern vvar_t	r_filmBleach;

extern vvar_t	r_filmMidStart;
extern vvar_t	r_filmMidEnd;

extern vvar_t	r_filmLightTint;
extern vvar_t	r_filmMidTint;
extern vvar_t	r_filmDarkTint;

extern vvar_t	r_distortionEnable;
extern vvar_t	r_distortionRadius;
extern vvar_t	r_distortionScale;
#endif

//void R_DoPostProcessing(refDef_t *fd);
#ifdef WANT_OPENGL
void WINAPI _glBegin(GLenum mode);
void WINAPI _glDrawElements(GLenum mode,  GLsizei count,  GLenum type,  const GLvoid *indices);
void WINAPI _glViewport(GLint x,  GLint y,  GLsizei width,  GLsizei height);
#endif

effect_t * R_GetEffectById(int ID);
effect_t * R_GetEffectByName(const char * fxName);

void R_Draw3D();

void R_SetShader( int ent_num, const char * shader, qboolean fDefault );
void R_AddRefEntityToScene( refEntity_t *ent, int parentEntityNumber );
void R_AddRefSpriteToScene( refEntity_t *ent );

void R_PostProcessEffect(const char * effectName, float intensity, vec3_t colors, float fadetime, qboolean bAddEffect, float parameters[2]);
void R_ProcessEffectsFade(int frametime);
void R_RemoveEffect(effect_t * effect);
void R_Shutdown(void);
void R_SetSceneColor( vec3_t light, vec3_t dark );
void R_SetSceneColor2(float intensity, vec3_t color);
void R_SetSkyColor( vec3_t color );
qboolean R_ProcessVisionVar( const char * variable, const char * value );
void R_ProcessBlur( float blur_level, float fade_time );
void R_ProcessVision( str vision_name, float fade_time, float phase = 0.0f, str type = "vision" );

#endif

#endif
