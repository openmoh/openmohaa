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

#include "tr_types.h"

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
	void	(*BeginRegistration)( glconfig_t *config );
	qhandle_t (*RegisterModel)( const char *name );
	qhandle_t (*RegisterSkin)( const char *name );
	qhandle_t (*RegisterShader)( const char *name );
	qhandle_t (*RegisterShaderNoMip)( const char *name );
	void	(*LoadWorld)( const char *name );

	// the vis data is a large enough block of data that we go to the trouble
	// of sharing it with the clipmodel subsystem
	void	(*SetWorldVisData)( const byte *vis );

	// EndRegistration will draw a tiny polygon with each texture, forcing
	// them to be loaded into card memory
	void	(*EndRegistration)( void );

	// a scene is built up by calls to R_ClearScene and the various R_Add functions.
	// Nothing is drawn until R_RenderScene is called.
	void	(*ClearScene)( void );
	void	(*AddRefEntityToScene)( const refEntity_t *re, int parentEntityNumber );
	qboolean(*AddPolyToScene)( qhandle_t hShader , int numVerts, const polyVert_t *verts, int num );
	int		(*LightForPoint)( vec3_t point, vec3_t ambientLight, vec3_t directedLight, vec3_t lightDir );
	void	(*AddLightToScene)( const vec3_t org, float intensity, float r, float g, float b, int type );
	void	(*AddAdditiveLightToScene)( const vec3_t org, float intensity, float r, float g, float b );
	void	(*RenderScene)( const refdef_t *fd );

	void	(*SetColor)( const float *rgba );	// NULL = 1,1,1,1

	// Draw images for cinematic rendering, pass as 32 bit rgba
	void	(*DrawStretchRaw) (int x, int y, int w, int h, int cols, int rows, int components, const byte* data);
	void	(*UploadCinematic) (int w, int h, int cols, int rows, const byte *data, int client, qboolean dirty);

	void	(*BeginFrame)( stereoFrame_t stereoFrame );

	// if the pointers are not NULL, timing info will be returned
	void	(*EndFrame)( int *frontEndMsec, int *backEndMsec );


	int		(*MarkFragments)( int numPoints, const vec3_t *points, const vec3_t projection,
				   int maxPoints, vec3_t pointBuffer, int maxFragments, markFragment_t *fragmentBuffer, float fRadiusSquared );

	int		(*LerpTag)( orientation_t *tag,  qhandle_t model, int startFrame, int endFrame, 
					 float frac, const char *tagName );
	void	(*ModelBounds)( qhandle_t model, vec3_t mins, vec3_t maxs );

#ifdef __USEA3D
	void    (*A3D_RenderGeometry) (void *pVoidA3D, void *pVoidGeom, void *pVoidMat, void *pVoidGeomStatus);
#endif
	void	(*RegisterFont)(const char *fontName, int pointSize, fontInfo_t *font);
	void	(*RemapShader)(const char *oldShader, const char *newShader, const char *offsetTime);
	qboolean (*GetEntityToken)( char *buffer, int size );
	qboolean (*inPVS)( const vec3_t p1, const vec3_t p2 );

	void (*TakeVideoFrame)( int h, int w, byte* captureBuffer, byte *encodeBuffer, qboolean motionJpeg );

	//
	// non-ioq3
	//
    qhandle_t(*SpawnEffectModel)(const char* name, vec3_t pos, vec3_t axis[3]);
    qhandle_t(*RegisterServerModel)(const char* name);
    void (*UnregisterServerModel)(qhandle_t model);
    qhandle_t(*RefreshShaderNoMip)(const char* name);
    void (*FreeModels)();

    void (*PrintBSPFileSizes)();
    int (*MapVersion)();

    void (*AddRefSpriteToScene)(const refEntity_t* ent);
    void (*AddTerrainMarkToScene)(int terrainIndex, qhandle_t hShader, int numVerts, const polyVert_t* verts, int renderfx);

    refEntity_t* (*GetRenderEntity)(int entityNumber);
    void    (*SavePerformanceCounters)();

    void    (*Set2DWindow)(int x, int y, int w, int h, float left, float right, float bottom, float top, float n, float f);
	void	(*DrawStretchPic) ( float x, float y, float w, float h, 
		float s1, float t1, float s2, float t2, qhandle_t hShader );	// 0 = white
    void    (*DrawTilePic)(float x, float y, float w, float h, qhandle_t hShader);
    void    (*DrawTilePicOffset)(float x, float y, float w, float h, qhandle_t hShader, int offsetX, int offsetY);
    void    (*DrawTrianglePic)(vec2_t* points, vec2_t* texCoords, qhandle_t hShader);
    void    (*DrawBackground)(int cols, int rows, int bgr, uint8_t* data);

    void    (*DebugLine)(const vec3_t start, const vec3_t end, float r, float g, float b, float alpha);
    void    (*DrawBox)(float x, float y, float w, float h);
    void    (*AddBox)(float x, float y, float w, float h);

    void    (*Scissor)(int x, int y, int width, int height);
    void    (*DrawLineLoop)(const vec2_t* points, int count, int stippleFactor, int stippleMask);
   
    int     (*MarkFragmentsForInlineModel)(clipHandle_t bmodel, const vec3_t vAngles, const vec3_t vOrigin, int numPoints,
                const vec3_t* points, const vec3_t projection, int maxPoints, vec3_t pointBuffer,
                int maxFragments, markFragment_t* fragmentBuffer, float fRadiusSquared);

    void    (*GetInlineModelBounds)(int index, vec3_t mins, vec3_t maxs);
    void    (*GetLightingForDecal)(vec3_t light, const vec3_t facing, const vec3_t origin);
    void    (*GetLightingForSmoke)(vec3_t light, const vec3_t origin);
    int     (*R_GatherLightSources)(const vec3_t pos, vec3_t* lightPos, vec3_t* lightIntensity, int maxLights);

    float   (*ModelRadius)(qhandle_t handle);

    dtiki_t*        (*R_Model_GetHandle)(qhandle_t handle);
    void            (*DrawString)(fontheader_t* font, const char* text, float x, float y, int maxLen, qboolean virtualScreen);
    float           (*GetFontHeight)(const fontheader_t* font);
    float           (*GetFontStringWidth)(const fontheader_t* font, const char* string);
    fontheader_t*   (*LoadFont)(const char* name);
    void            (*SwipeBegin)(float thisTime, float life, qhandle_t hShader);
    void            (*SwipePoint)(vec3_t point1, vec3_t point2, float time);
    void            (*SwipeEnd)();
    void            (*SetRenderTime)(int t);
    float           (*Noise)(float x, float y, float z, float t);
    qboolean        (*SetMode)(int mode, const glconfig_t* glConfig);
    void            (*SetFullscreen)(qboolean fullScreen);
    int             (*GetShaderWidth)(qhandle_t hShader);
    int             (*GetShaderHeight)(qhandle_t hShader);
    const char*     (*GetGraphicsInfo)();
    void            (*ForceUpdatePose)(refEntity_t* model);
    orientation_t   (*TIKI_Orientation)(refEntity_t* model, int tagNum);
    qboolean        (*TIKI_IsOnGround)(refEntity_t* model, int tagNum, float threshold);
    void            (*SetFrameNumber)(int frameNumber);
} refexport_t;

//
// these are the functions imported by the refresh module
//
typedef struct {
	// print message on the local console
	void	(QDECL *Printf)( int printLevel, const char *fmt, ...) __attribute__ ((format (printf, 2, 3)));

	// abort the game
	void	(QDECL *Error)( int errorLevel, const char *fmt, ...) __attribute__ ((noreturn, format (printf, 2, 3)));

	// milliseconds should only be used for profiling, never
	// for anything game related.  Get time from the refdef
	int		(*Milliseconds)( void );

	// stack based memory allocation for per-level things that
	// won't be freed
#ifdef HUNK_DEBUG
	void	*(*Hunk_AllocDebug)( int size, ha_pref pref, char *label, char *file, int line );
#else
	void	*(*Hunk_Alloc)( int size, ha_pref pref );
#endif
	void	*(*Hunk_AllocateTempMemory)( int size );
	void	(*Hunk_FreeTempMemory)( void *block );

	// dynamic memory allocator for things that need to be freed
	void	*(*Malloc)( int bytes );
	void	(*Free)( void *buf );

	cvar_t	*(*Cvar_Get)( const char *name, const char *value, int flags );
	void	(*Cvar_Set)( const char *name, const char *value );
	void	(*Cvar_SetValue) (const char *name, float value);
	void	(*Cvar_CheckRange)( cvar_t *cv, float minVal, float maxVal, qboolean shouldBeIntegral );
	void	(*Cvar_SetDescription)( cvar_t *cv, const char *description );

	int		(*Cvar_VariableIntegerValue) (const char *var_name);

	void	(*Cmd_AddCommand)( const char *name, void(*cmd)(void) );
	void	(*Cmd_RemoveCommand)( const char *name );

	int		(*Cmd_Argc) (void);
	char	*(*Cmd_Argv) (int i);

	void	(*Cmd_ExecuteText) (int exec_when, const char *text);

	byte	*(*CM_ClusterPVS)(int cluster);

	// visualization for debugging collision detection
	void	(*CM_DrawDebugSurface)( void (*drawPoly)(int color, int numPoints, float *points) );

	// a -1 return means the file does not exist
	// NULL can be passed for buf to just determine existence
	int		(*FS_FileIsInPAK)( const char *name, int *pCheckSum );
	long		(*FS_ReadFile)( const char *name, void **buf );
	void	(*FS_FreeFile)( void *buf );
	char **	(*FS_ListFiles)( const char *name, const char *extension, int *numfilesfound );
	void	(*FS_FreeFileList)( char **filelist );
	void	(*FS_WriteFile)( const char *qpath, const void *buffer, int size );
	qboolean (*FS_FileExists)( const char *file );

	// cinematic stuff
	void	(*CIN_UploadCinematic)(int handle);
	int		(*CIN_PlayCinematic)( const char *arg0, int xpos, int ypos, int width, int height, int bits);
	e_status (*CIN_RunCinematic) (int handle);

	void	(*CL_WriteAVIVideoFrame)( const byte *buffer, int size );

	// input event handling
	void	(*IN_Init)( void *windowData );
	void	(*IN_Shutdown)( void );
	void	(*IN_Restart)( void );

	// math
	long    (*ftol)(float f);

	// system stuff
	void	(*Sys_SetEnv)( const char *name, const char *value );
	void	(*Sys_GLimpSafeInit)( void );
	void	(*Sys_GLimpInit)( void );
	qboolean (*Sys_LowPhysicalMemory)( void );

    //
    // non-ioq3
    //
    const char* (*LV_ConvertString)(const char* string);
    void (*Hunk_Clear)();
    void (*Clear)();
    void (*Cvar_SetDefault)(cvar_t* var, const char* varValue);

    long    (*FS_OpenFile)(const char* qpath, fileHandle_t *file, qboolean uniqueFILE, qboolean quiet);
    size_t  (*FS_Read)(void* buffer, size_t len, fileHandle_t fileHandle);
    void    (*FS_CloseFile)(fileHandle_t fileHandle);
    int     (*FS_Seek)(fileHandle_t fileHandle, long offset, fsOrigin_t origin);
    long     (*FS_ReadFileEx)(const char* qpath, void** buffer, qboolean quiet);

    void        (*CM_BoxTrace)(trace_t* results, const vec3_t start, const vec3_t end, const vec3_t mins, const vec3_t maxs, int model, int brushMask, int cylinder);
    int         (*CM_TerrainSquareType)(int terrainPatch, int i, int j);
    char*       (*CM_EntityString)();
	const char* (*CM_MapTime)();
    int         (*CG_PermanentMark)(const vec3_t origin, const vec3_t dir, float orientation, float sScale, float tScale, float red, float green, float blue, float alpha, qboolean doLighting, float sCenter, float tCenter, markFragment_t* markFragments, void* polyVerts);
    int         (*CG_PermanentTreadMarkDecal)(treadMark_t* treadMark, qboolean startSegment, qboolean doLighting, markFragment_t* markFragments, void* polyVerts);
    int         (*CG_PermanentUpdateTreadMark)(treadMark_t* treadMark, float alpha, float minSegment, float maxSegment, float maxOffset, float texScale);
    void        (*CG_ProcessInitCommands)(dtiki_t* tiki, refEntity_t* ent);
    void        (*CG_EndTiki)(dtiki_t* tiki);
    void        (*SetPerformanceCounters)(int totalTris, int totalVerts, int totalTexels, int worldTris, int worldVerts, int characterLights);

    debugline_t** DebugLines;
    int* numDebugLines;
    debugstring_t** DebugStrings;
    int* numDebugStrings;

    orientation_t   (*TIKI_OrientationInternal)(dtiki_t* tiki, int entNum, int tagNum, float scale);
    qboolean        (*TIKI_IsOnGroundInternal)(dtiki_t* tiki, int entNum, int tagNum, float thresHold);
    void            (*TIKI_SetPoseInternal)(void* skeletor, const frameInfo_t* frameInfo, const int* boneTag, const vec4_t *boneQuat, float actionWeight);
    void*           (*TIKI_Alloc)(size_t size);
    float           (*GetRadiusInternal)(dtiki_t* tiki, int entNum, float scale);
    float           (*GetCentroidRadiusInternal)(dtiki_t* tiki, int entNum, float scale, vec3_t centroid);
    void            (*GetFrameInternal)(dtiki_t* tiki, int entNum, skelAnimFrame_t* newFrame);
} refimport_t;


// this is the only function actually exported at the linker level
// If the module can't init to a valid rendering state, NULL will be
// returned.
#ifdef USE_RENDERER_DLOPEN
typedef	refexport_t* (QDECL *GetRefAPI_t) (int apiVersion, refimport_t * rimp);
#else
refexport_t*GetRefAPI( int apiVersion, refimport_t *rimp );
#endif

qboolean R_ImageExists(const char* name);

#ifdef __cplusplus
}
#endif

#endif	// __TR_PUBLIC_H
