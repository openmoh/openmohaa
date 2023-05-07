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
#ifndef __TR_PUBLIC_H
#define __TR_PUBLIC_H

#include "../cgame/tr_types.h"
#include "q_shared.h"

#ifdef __cplusplus
extern "C" {
#endif

#define	REF_API_VERSION		14


typedef struct dtiki_s dtiki_t;
typedef struct skelAnimFrame_s skelAnimFrame_t;

//
// these are the functions exported by the refresh module
//
typedef struct {
	// called before the library is unloaded
	// if the system is just reconfiguring, pass destroyWindow = qfalse,
	// which will keep the screen from flashing to the desktop.
	void	(*Shutdown)( qboolean destroyWindow );

	// All data that will be used in a level should be
	// registered before rendering any frames to prevent disk hits,
	// but they can still be registered at a later time
	// if necessary.
	//
	// BeginRegistration makes any existing media pointers invalid
	// and returns the current gl configuration, including screen width
	// and height, which can be used by the client to intelligently
	// size display elements
	void		( *BeginRegistration )( glconfig_t *glConfigOut );
	qhandle_t	( *RegisterModel )( const char *name );
	qhandle_t	( *SpawnEffectModel )( const char *name, vec3_t pos, vec3_t axis[3] );
	qhandle_t	( *RegisterServerModel )( const char *name );
	void		( *UnregisterServerModel )( qhandle_t model );
	qhandle_t	( *RegisterShader )( const char *name );
	qhandle_t	( *RegisterShaderNoMip )( const char *name );
	qhandle_t	( *RefreshShaderNoMip )( const char *name );

	// EndRegistration will draw a tiny polygon with each texture, forcing
	// them to be loaded into card memory
	void	( *EndRegistration )( void );
	void	( *FreeModels )( void );

	// the vis data is a large enough block of data that we go to the trouble
	// of sharing it with the clipmodel subsystem
	void	( *SetWorldVisData )( const byte *vis );

	void	( *LoadWorld )( const char *name );
	void	( *PrintBSPFileSizes )( void );
	int		( *MapVersion )( void );

	// a scene is built up by calls to R_ClearScene and the various R_Add functions.
	// Nothing is drawn until R_RenderScene is called.
	void	(*ClearScene)( void );
	void	(*AddRefEntityToScene)( const refEntity_t *re, int parentEntityNumber );
	void	(*AddRefSpriteToScene )( const refEntity_t *ent );
	qboolean	(*AddPolyToScene)( qhandle_t hShader , int numVerts, const polyVert_t *verts, int renderFx );
	void	(*AddTerrainMarkToScene)( int terrainIndex, qhandle_t hShader, int numVerts, polyVert_t *verts, int renderfx );
	void	(*AddLightToScene)( const vec3_t org, float intensity, float r, float g, float b, int type );
	void	(*AddAdditiveLightToScene)( const vec3_t org, float intensity, float r, float g, float b );
	void	(*RenderScene)( const refdef_t *fd );

	refEntity_t		*(*GetRenderEntity )( int entityNumber );
	void			(*SavePerformanceCounters)( void );

	void	(*SetColor)( const vec4_t rgba );	// NULL = 1,1,1,1
	void	(*Set2DWindow)( int x, int y, int w, int h, float left, float right, float bottom, float top, float n, float f );
	void	(*DrawStretchPic) ( float x, float y, float w, float h,
		float s1, float t1, float s2, float t2, qhandle_t hShader );	// 0 = white

	void	(*DrawTilePic)( float x, float y, float w, float h, qhandle_t hShader );
	void	(*DrawTilePicOffset)( float x, float y, float w, float h, qhandle_t hShader, int offsetX, int offsetY );
	void	(*DrawTrianglePic)( vec2_t *points, vec2_t *texCoords, qhandle_t hShader );
	void	(*DrawBackground)( int cols, int rows, int bgr, unsigned char *data );

	// Draw images for cinematic rendering, pass as 32 bit rgba
	void	(*DrawStretchRaw) (int x, int y, int w, int h, int cols, int rows, int components, const byte *data);

	void	(*DebugLine)( vec3_t start, vec3_t end, float r, float g, float b, float alpha );
	void	(*DrawBox)( float x, float y, float w, float h );
	void	(*AddBox)( float x, float y, float w, float h );

	void	(*BeginFrame)( stereoFrame_t stereoFrame );

	void	(*Scissor)( int x, int y, int width, int height );
	void	(*DrawLineLoop)( vec2_t *points, int count, int stippleFactor, int stippleMask );

	// if the pointers are not NULL, timing info will be returned
	void	(*EndFrame)( int *frontEndMsec, int *backEndMsec );


	int		(*MarkFragments)( int numPoints, const vec3_t *points, const vec3_t projection,
					int maxPoints, vec3_t pointBuffer, int maxFragments, markFragment_t *fragmentBuffer, float fRadiusSquared );

	int		(*MarkFragmentsForInlineModel)( clipHandle_t bmodel, vec3_t vAngles, vec3_t vOrigin,
                                        int numPoints, const vec3_t *points, const vec3_t projection, 
                                        int maxPoints, vec3_t pointBuffer,
                                        int maxFragments, markFragment_t *fragmentBuffer, float fRadiusSquared );

	void	( *GetInlineModelBounds )( int index, vec3_t mins, vec3_t maxs );
	void	( *GetLightingForDecal )( vec3_t light, vec3_t facing, vec3_t origin );
	void	( *GetLightingForSmoke )( vec3_t light, vec3_t origin );
	int		( *R_GatherLightSources )( vec3_t pos, vec3_t *lightPos, vec3_t *lightIntensity, int maxLights );
	void	( *ModelBounds )( qhandle_t handle, vec3_t mins, vec3_t maxs );
	float	( *ModelRadius )( qhandle_t handle );
	dtiki_t	*( *R_Model_GetHandle )( qhandle_t handle );
	void	( *DrawString )( fontheader_t *font, const char *text, float x, float y, int maxLen, qboolean virtualScreen );
	float	( *GetFontHeight )( fontheader_t *font );
	float	( *GetFontStringWidth )( fontheader_t *font, const char *string );

	fontheader_t	*(*LoadFont)( const char *name );

	void			(*SwipeBegin)( float thisTime, float life, qhandle_t hShader );
	void			(*SwipePoint)( vec3_t point1, vec3_t point2, float time );
	void			(*SwipeEnd)( );
	void			(*SetRenderTime)( int t );
	float			(*Noise)( float x, float y, float z, float t );
	qboolean		(*SetMode)( int mode, glconfig_t *glConfig );
	void			(*SetFullscreen)( qboolean fullscreen, glconfig_t *config );

#ifdef __USEA3D
	void    (*A3D_RenderGeometry) (void *pVoidA3D, void *pVoidGeom, void *pVoidMat, void *pVoidGeomStatus);
#endif

	// IneQuation
	int		(*Text_Width)(fontInfo_t *font, const char *text, int limit, qboolean useColourCodes);
	int		(*Text_Height)(fontInfo_t *font, const char *text, int limit, qboolean useColourCodes);
	// Paints a string. The alpha value will be ignored unless useColourCodes is qtrue.
	void	(*Text_Paint)(fontInfo_t *font, float x, float y, float scale, float alpha, const char *text, float adjust, int limit, qboolean useColourCodes, qboolean is640);
	void	(*Text_PaintChar)(fontInfo_t *font, float x, float y, float scale, int c, qboolean is640);
	// su44
	int		(*GetShaderWidth)(qhandle_t shader);
	int		(*GetShaderHeight)(qhandle_t shader);

	const char		*(*GetGraphicsInfo)( );
	void			(*ForceUpdatePose)( refEntity_t *model );
	orientation_t	(*TIKI_Orientation)( refEntity_t *model, int tagNum );
	qboolean		(*TIKI_IsOnGround)( refEntity_t *model, int tagNum, float threshold );
	void			(*SetFrameNumber)( int frameNumber );

	// obsolete functions
	//void		(*TakeVideoFrame)( int h, int w, byte* captureBuffer, byte *encodeBuffer, qboolean motionJpeg );
	const char *(*GetShaderName)(qhandle_t shader);
	void		(*RegisterFont)(const char *fontName, int pointSize, fontInfo_t *font);
	qboolean	(*inPVS)( const vec3_t p1, const vec3_t p2 );
	void		(*UploadCinematic) (int w, int h, int cols, int rows, const byte *data, int client, qboolean dirty);
} refexport_t;

//
// these are the functions imported by the refresh module
//
typedef struct {
	// print message on the local console
	void	(QDECL *Printf)( int printLevel, const char *fmt, ...);

	// abort the game
	void	(QDECL *Error)( int errorLevel, const char *fmt, ...);

	// milliseconds should only be used for profiling, never
	// for anything game related.  Get time from the refdef
	int		(*Milliseconds)( void );

	// transform the text into its localized version
	const char *(*LV_ConvertString)( const char *text );

	// stack based memory allocation for per-level things that
	// won't be freed
	void	(*Hunk_Clear )( void );
	void	*(*Hunk_Alloc)(size_t size );
	void	*(*Hunk_AllocateTempMemory)(size_t size );
	void	(*Hunk_FreeTempMemory)( void *block );

	// dynamic memory allocator for things that need to be freed
	void	*(*Malloc)(size_t bytes );
	void	(*Free)( void *buf );
	void	(*Clear)( void );

	cvar_t	*(*Cvar_Get)( const char *name, const char *value, int flags );
	void	(*Cvar_Set)( const char *name, const char *value );
	void	(*Cvar_SetDefault)( cvar_t *var, const char *value );

	void	(*Cmd_AddCommand)( const char *name, void(*cmd)(void) );
	void	(*Cmd_RemoveCommand)( const char *name );

	int		(*Cmd_Argc) (void);
	char	*(*Cmd_Argv) (int i);

	void	(*Cmd_ExecuteText) (cbufExec_t exec_when, const char *text);

	// visualization for debugging collision detection
	void	(*CM_DrawDebugSurface)( void (*drawPoly)(int color, int numPoints, float *points) );

	// a -1 return means the file does not exist
	// NULL can be passed for buf to just determine existance
	int		(*FS_FOpenFile)( const char *filename, fileHandle_t *file, qboolean uniqueFILE, qboolean quiet );
	size_t  (*FS_Read)( void *buffer, size_t len, fileHandle_t f );
	void	(*FS_CloseFile)( fileHandle_t f );
	int		(*FS_Seek)( fileHandle_t f, long offset, fsOrigin_t origin );
	int		(*FS_FileIsInPAK)( const char *filename, int *pChecksum );
	int		(*FS_ReadFile)( const char *name, void **buf );
	int		(*FS_ReadFileEx)( const char *name, void **buf, qboolean quiet );
	void	(*FS_FreeFile)( void *buf );
	char **	(*FS_ListFiles)( const char *name, const char *extension, qboolean wantStubs, int *numfilesfound );
	void	(*FS_FreeFileList)( char **filelist );
	int		(*FS_WriteFile)( const char *qpath, const void *buffer, int size );
	qboolean (*FS_FileExists)( const char *file );

	// cm stuff
	void (*CM_BoxTrace)( trace_t *results, const vec3_t start, const vec3_t end,
		const vec3_t mins, const vec3_t maxs,
		clipHandle_t model, int brushmask, int cylinder );
	int			(*CM_TerrainSquareType)( int iTerrainPatch, int i, int j );
	char		*(*CM_EntityString)( void );
	const char	*(*CM_MapTime)( void );

	// client game stuff
	int (*CG_PermanentMark)( vec3_t origin, vec3_t dir, float orientation,
		float fSScale, float fTScale, float red, float green, float blue, float alpha,
		qboolean dolighting, float fSCenter, float fTCenter,
		markFragment_t *pMarkFragments, void *pPolyVerts );
	int	(*CG_PermanentTreadMarkDecal)( treadMark_t *pTread, qboolean bStartSegment, qboolean dolighting,
		markFragment_t *pMarkFragments, void *pVoidPolyVerts );
	int		(*CG_PermanentUpdateTreadMark)( treadMark_t *pTread, float fAlpha, float fMinSegment, float fMaxSegment, float fMaxOffset, float fTexScale );
	void	(*CG_ProcessInitCommands)( dtiki_t *tiki, refEntity_t *ent );
	void	(*CG_EndTiki)( dtiki_t *tiki );


	void	(*SetPerformanceCounters)( int total_tris, int total_verts, int total_texels, int world_tris, int world_verts, int character_lights );

	// debugging stuff
	debugline_t **DebugLines;
	int *numDebugLines;
	debugstring_t **DebugStrings;
	int *numDebugStrings;

	// TIKI stuff
	orientation_t	(*TIKI_OrientationInternal)( dtiki_t *tiki, int entnum, int tagnum, float scale );
	qboolean		(*TIKI_IsOnGroundInternal) (dtiki_t *tiki, int entnum, int tagnum, float threshold );
	void			(*TIKI_SetPoseInternal)( void *skeletor, const frameInfo_t *frameInfo, int *bone_tag, vec4_t *bone_quat, float actionWeight );
	void			*(*TIKI_Alloc)(size_t size );
	float			(*GetRadiusInternal)( dtiki_t *tiki, int entnum, float scale );
	float			(*GetCentroidRadiusInternal)( dtiki_t *tiki, int entnum, float scale, float *centroid );
	void			(*GetFrameInternal)( dtiki_t *tiki, int entnum, skelAnimFrame_t *newFrame );

	// cinematic stuff
	void	(*CIN_UploadCinematic)(int handle);
	int		(*CIN_PlayCinematic)( const char *arg0, int xpos, int ypos, int width, int height, int bits);
	e_status (*CIN_RunCinematic) (int handle);

	void	(*CL_WriteAVIVideoFrame)( const byte *buffer, int size );

	// su44 - plugable renderer architecture
	byte	*(*CM_ClusterPVS)(int cluster);

	// input event handling
	void	(*IN_Init)(void *windowData);
	void	(*IN_Shutdown)(void);
    void	(*IN_Restart)(void);

    // system stuff
    void	(*Sys_SetEnv)(const char* name, const char* value);
    void	(*Sys_GLimpSafeInit)(void);
    void	(*Sys_GLimpInit)(void);
    qboolean(*Sys_LowPhysicalMemory)(void);
} refimport_t;


// this is the only function actually exported at the linker level
// If the module can't init to a valid rendering state, NULL will be
// returned.
#ifdef USE_RENDERER_DLOPEN
typedef	refexport_t* (QDECL *GetRefAPI_t) (int apiVersion, refimport_t * rimp);
#else
refexport_t*GetRefAPI( int apiVersion, refimport_t *rimp );
#endif

#ifdef __cplusplus
}
#endif

#endif	// __TR_PUBLIC_H
