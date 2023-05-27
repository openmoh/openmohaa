/*
===========================================================================
Copyright (C) 2023 the OpenMoHAA team

This file is part of OpenMoHAA source code.

OpenMoHAA source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

OpenMoHAA source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with OpenMoHAA source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include "../../cgame/tr_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct dtiki_s dtiki_t;
typedef struct skelAnimFrame_s skelAnimFrame_t;

typedef struct {
    qhandle_t(*SpawnEffectModel)(const char* name, vec3_t pos, vec3_t axis[3]);
    qhandle_t(*RegisterServerModel)(const char* name);
    void (*UnregisterServerModel)(qhandle_t model);
    qhandle_t(*RefreshShaderNoMip)(const char* name);
    void (*FreeModels)();

    void (*PrintBSPFileSizes)();
    int (*MapVersion)();

    void (*AddRefEntityToScene)(const refEntity_t* re, int parentEntityNumber);
    void (*AddRefSpriteToScene)(const refEntity_t* ent);
    qboolean(*AddPolyToScene)(qhandle_t hShader, int numVerts, const polyVert_t* verts, int renderfx);
    void (*AddTerrainMarkToScene)(int terrainIndex, qhandle_t hShader, int numVerts, const polyVert_t* verts, int renderfx);
    void (*AddLightToScene)(const vec3_t org, float intensity, float r, float g, float b, int type);
    void (*RenderScene)(const refdef_t* fd);

    refEntity_t* (*GetRenderEntity)(int entityNumber);
    void    (*SavePerformanceCounters)();

    void    (*Set2DWindow)(int x, int y, int w, int h, float left, float right, float bottom, float top, float n, float f);
    void    (*DrawTilePic)(float x, float y, float w, float h, qhandle_t hShader);
    void    (*DrawTilePicOffset)(float x, float y, float w, float h, qhandle_t hShader, int offsetX, int offsetY);
    void    (*DrawTrianglePic)(vec2_t* points, vec2_t* texCoords, qhandle_t hShader);
    void    (*DrawBackground)(int cols, int rows, int bgr, uint8_t* data);

    // Draw images for cinematic rendering, pass as 32 bit rgba
    void	(*DrawStretchRaw) (int x, int y, int w, int h, int cols, int rows, int components, const byte* data);
    void    (*DebugLine)(const vec3_t start, const vec3_t end, float r, float g, float b, float alpha);
    void    (*DrawBox)(float x, float y, float w, float h);
    void    (*AddBox)(float x, float y, float w, float h);

    void	(*BeginFrame)(stereoFrame_t stereoFrame);
    void    (*Scissor)(int x, int y, int width, int height);
    void    (*DrawLineLoop)(const vec2_t* points, int count, int stippleFactor, int stippleMask);
    
	int		(*MarkFragments)( int numPoints, const vec3_t *points, const vec3_t projection,
				   int maxPoints, vec3_t pointBuffer, int maxFragments, markFragment_t *fragmentBuffer, float fRadiusSquared);
    int (*MarkFragmentsForInlineModel)(clipHandle_t bmodel, const vec3_t angles, const vec3_t origin, int numPoints,
        const vec3_t* points, const vec3_t projection, int maxPoints, vec3_t pointBuffer,
        int maxFragments, markFragment_t* fragmentBuffer, float radiusSquared);

    void    (*GetInlineModelBounds)(int index, vec3_t mins, vec3_t maxs);
    void    (*GetLightingForDecal)(vec3_t light, vec3_t facing, vec3_t origin);
    void    (*GetLightingForSmoke)(vec3_t light, vec3_t origin);
    int     (*R_GatherLightSources)(const vec3_t pos, vec3_t* lightPos, vec3_t* lightIntensity, int maxLights);

    void	(*ModelBounds)(qhandle_t model, vec3_t mins, vec3_t maxs);
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

} refexport_new_t;

typedef struct {
    char* (*LV_ConvertString)(char* string);
    void (*Hunk_Clear)();
    void (*Clear)();
    void (*Cvar_SetDefault)(cvar_t* var, const char* varValue);
    byte* (*CM_ClusterPVS)(int cluster);

    
    int     (*FS_OpenFile)(const char* qpath, fileHandle_t *file, qboolean uniqueFILE, qboolean quiet);
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
} refimport_new_t;

qboolean R_ImageExists(const char* name);

#ifdef __cplusplus
}
#endif
