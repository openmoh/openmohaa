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

#pragma once

extern int r_sequencenumber;

void GetRefAPI_new(refimport_t* ri, refexport_t* re);

extern cvar_t* r_reset_tc_array;
extern  cvar_t* r_numdebuglines;

extern cvar_t* r_staticlod;
extern cvar_t* r_lodscale;
extern cvar_t* r_lodcap;
extern cvar_t* r_lodviewmodelcap;

extern cvar_t* r_uselod;
extern cvar_t* lod_LOD;
extern cvar_t* lod_minLOD;
extern cvar_t* lod_maxLOD;
extern cvar_t* lod_LOD_slider;
extern cvar_t* lod_curve_0_val;
extern cvar_t* lod_curve_1_val;
extern cvar_t* lod_curve_2_val;
extern cvar_t* lod_curve_3_val;
extern cvar_t* lod_curve_4_val;
extern cvar_t* lod_edit_0;
extern cvar_t* lod_edit_1;
extern cvar_t* lod_edit_2;
extern cvar_t* lod_edit_3;
extern cvar_t* lod_edit_4;
extern cvar_t* lod_curve_0_slider;
extern cvar_t* lod_curve_1_slider;
extern cvar_t* lod_curve_2_slider;
extern cvar_t* lod_curve_3_slider;
extern cvar_t* lod_curve_4_slider;
extern cvar_t* lod_pitch_val;
extern cvar_t* lod_zee_val;
extern cvar_t* lod_mesh;
extern cvar_t* lod_meshname;
extern cvar_t* lod_tikiname;
extern cvar_t* lod_metric;
extern cvar_t* lod_tris;
extern cvar_t* lod_position;
extern cvar_t* lod_save;
extern cvar_t* lod_tool;
extern cvar_t* r_showSkeleton;

typedef union varnodeUnpacked_u {
    float fVariance;
    struct {
        byte flags;
        unsigned char unused[3];
    } s;
} varnodeUnpacked_t;

typedef unsigned short terraInt;

typedef struct terrainVert_s {
    vec3_t xyz;
    vec2_t texCoords[2];
    float fVariance;
    float fHgtAvg;
    float fHgtAdd;
    unsigned int uiDistRecalc;
    terraInt nRef;
    terraInt iVertArray;
    byte* pHgt;
    terraInt iNext;
    terraInt iPrev;
} terrainVert_t;

typedef struct terraTri_s {
    unsigned short iPt[3];
    terraInt nSplit;
    unsigned int uiDistRecalc;
    struct cTerraPatchUnpacked_s* patch;
    varnodeUnpacked_t* varnode;
    terraInt index;
    byte lod;
    byte byConstChecks;
    terraInt iLeft;
    terraInt iRight;
    terraInt iBase;
    terraInt iLeftChild;
    terraInt iRightChild;
    terraInt iParent;
    terraInt iPrev;
    terraInt iNext;
} terraTri_t;

typedef struct srfTerrain_s {
    surfaceType_t surfaceType;
    terraInt iVertHead;
    terraInt iTriHead;
    terraInt iTriTail;
    terraInt iMergeHead;
    int nVerts;
    int nTris;
    int lmapSize;
    int dlightBits[2];
    float lmapStep;
    int dlightMap[2];
    byte* lmData;
    float lmapX;
    float lmapY;
} srfTerrain_t;

typedef struct cTerraPatchUnpacked_s {
    srfTerrain_t drawinfo;
    int viewCount;
    int visCountCheck;
    int visCountDraw;
    int frameCount;
    unsigned int uiDistRecalc;
    float s;
    float t;
    vec2_t texCoord[2][2];
    float x0;
    float y0;
    float z0;
    float zmax;
    shader_t* shader;
    short int iNorth;
    short int iEast;
    short int iSouth;
    short int iWest;
    struct cTerraPatchUnpacked_s* pNextActive;
    varnodeUnpacked_t varTree[2][63];
    unsigned char heightmap[81];
    byte flags;
    byte byDirty;
} cTerraPatchUnpacked_t;

typedef struct srfStaticModel_s {
    surfaceType_t surfaceType;
    struct cStaticModelUnpacked_s* parent;
} srfStaticModel_t;

typedef struct cStaticModelUnpacked_s {
    qboolean useSpecialLighting;
    qboolean bLightGridCalculated;
    qboolean bRendered;
    char model[128];
    vec3_t origin;
    vec3_t angles;
    vec3_t axis[3];
    float scale;
    int firstVertexData;
    int numVertexData;
    int visCount;
    dtiki_t* tiki;
    sphere_dlight_t dlights[32];
    int numdlights;
    float radius;
    float cull_radius;
    int iGridLighting;
    float lodpercentage[2];
} cStaticModelUnpacked_t;

typedef struct refSprite_s {
    surfaceType_t surftype;
    int hModel;
    int shaderNum;
    float origin[3];
    float scale;
    float axis[3][3];
    unsigned char shaderRGBA[4];
    int renderfx;
    float shaderTime;
} refSprite_t;

//
// tr_shader.c
//
qhandle_t		RE_RefreshShaderNoMip(const char* name);

//
// tr_bsp.c
//
void		RE_PrintBSPFileSizes(void);
int			RE_MapVersion(void);

typedef struct skelSurfaceGame_s skelSurfaceGame_t;
typedef struct staticSurface_s staticSurface_t;

/*
============================================================

DRAWING

============================================================
*/
void Draw_TilePic(float x, float y, float w, float h, qhandle_t hShader);
void Draw_TilePicOffset(float x, float y, float w, float h, qhandle_t hShader, int offsetX, int offsetY);
void Draw_TrianglePic(const vec2_t vPoints[3], const vec2_t vTexCoords[3], qhandle_t hShader);
void DrawBox(float x, float y, float w, float h);
void AddBox(float x, float y, float w, float h);
void Set2DWindow(int x, int y, int w, int h, float left, float right, float bottom, float top, float n, float f);
void RE_Scissor(int x, int y, int width, int height);
void DrawLineLoop(const vec2_t* points, int count, int stipple_factor, int stipple_mask);

/*
============================================================

FLARES

============================================================
*/

void R_ClearFlares(void);

void RB_AddFlare(void* surface, int fogNum, vec3_t point, vec3_t color, vec3_t normal);
void RB_AddDlightFlares(void);
void RB_RenderFlares(void);

/*
=============================================================

FONT

=============================================================
*/
fontheader_t* R_LoadFont(const char* name);
void R_LoadFontShader(fontheader_t* font);
void R_DrawString(fontheader_t* font, const char* text, float x, float y, int maxlen, qboolean bVirtualScreen);
float R_GetFontHeight(const fontheader_t* font);
float R_GetFontStringWidth(const fontheader_t* font, const char* s);

/*
============================================================

LIGHTS

============================================================
*/

void R_DlightBmodel(bmodel_t* bmodel);
void R_GetLightingGridValue(const vec3_t vPos, vec3_t vLight);
void R_GetLightingForDecal(vec3_t vLight, vec3_t vFacing, vec3_t vOrigin);
void R_GetLightingForSmoke(vec3_t vLight, vec3_t vOrigin);
void R_SetupEntityLighting(const trRefdef_t* refdef, trRefEntity_t* ent);
void RB_SetupEntityGridLighting();
void RB_SetupStaticModelGridLighting(trRefdef_t* refdef, cStaticModelUnpacked_t* ent, const vec3_t lightOrigin);
void R_TransformDlights(int count, dlight_t* dl, orientationr_t* ori);
void RB_Light_Real(unsigned char* colors);
void RB_Sphere_BuildDLights();
void RB_Sphere_SetupEntity();
void RB_Grid_SetupEntity();
void RB_Grid_SetupStaticModel();
void RB_Light_Fullbright(unsigned char* colors);
void R_Sphere_InitLights();
int R_GatherLightSources(const vec3_t vPos, vec3_t* pvLightPos, vec3_t* pvLightIntensity, int iMaxLights);
void R_UploadDlights();

/*
=============================================================

MARKS

=============================================================
*/
void R_LevelMarksLoad(const char* szBSPName);
void R_LevelMarksInit();
void R_LevelMarksFree();
void R_UpdateLevelMarksSystem();
void R_AddPermanentMarkFragmentSurfaces(void** pFirstMarkFragment, int iNumMarkFragment);

int R_MarkFragments_New(int numPoints, const vec3_t* points, const vec3_t projection,
    int maxPoints, vec3_t pointBuffer, int maxFragments, markFragment_t* fragmentBuffer, float fRadiusSquared);

int R_MarkFragmentsForInlineModel(clipHandle_t bmodel, const vec3_t angles, const vec3_t origin, int numPoints,
    const vec3_t* points, const vec3_t projection, int maxPoints, vec3_t pointBuffer,
    int maxFragments, markFragment_t* fragmentBuffer, float radiusSquared);

/*
=============================================================

SKY PORTALS

=============================================================
*/
void R_Sky_Init();
void R_Sky_Reset();
void R_Sky_AddSurf(msurface_t* surf);

/*
=============================================================

SPRITE

=============================================================
*/
sprite_t* SPR_RegisterSprite(const char* name);
void RB_DrawSprite(const refSprite_t* spr);


/*
=============================================================

SUN FLARE

=============================================================
*/
void R_InitLensFlare();
void R_DrawLensFlares();

/*
=============================================================

SWIPE

=============================================================
*/
void RB_DrawSwipeSurface(surfaceType_t* pswipe);
void RE_SwipeBegin(float thistime, float life, qhandle_t shader);
void RE_SwipeEnd();

/*
=============================================================

TERRAIN

=============================================================
*/
void R_MarkTerrainPatch(cTerraPatchUnpacked_t* pPatch);
void R_AddTerrainSurfaces();
void R_InitTerrain();
void R_TerrainPrepareFrame();

/*
=============================================================

SCENE GENERATION

=============================================================
*/
void RE_AddRefSpriteToScene(const refEntity_t* ent);
void RE_AddTerrainMarkToScene(int iTerrainIndex, qhandle_t hShader, int numVerts, const polyVert_t* verts, int renderfx);
refEntity_t* RE_GetRenderEntity(int entityNumber);

/*
=============================================================

TIKI

=============================================================
*/
void R_InitStaticModels(void);
void RE_FreeModels(void);
qhandle_t RE_SpawnEffectModel(const char* szModel, vec3_t vPos, vec3_t* axis);
qhandle_t RE_RegisterServerModel(const char* name);
void RE_UnregisterServerModel(qhandle_t hModel);
orientation_t RE_TIKI_Orientation(refEntity_t* model, int tagnum);
qboolean RE_TIKI_IsOnGround(refEntity_t* model, int tagnum, float threshold);
float R_ModelRadius(qhandle_t handle);
void R_ModelBounds(qhandle_t handle, vec3_t mins, vec3_t maxs);
dtiki_t* R_Model_GetHandle(qhandle_t handle);

float R_GetRadius(refEntity_t* model);
void R_GetFrame(refEntity_t* model, struct skelAnimFrame_s* newFrame);
void RE_ForceUpdatePose(refEntity_t* model);
void RE_SetFrameNumber(int frameNumber);
void R_UpdatePoseInternal(refEntity_t* model);
void RB_SkelMesh(skelSurfaceGame_t* sf);
void RB_StaticMesh(staticSurface_t* staticSurf);
void RB_Static_BuildDLights();
void R_PrintInfoStaticModels();
void R_AddSkelSurfaces(trRefEntity_t* ent);
void R_AddStaticModelSurfaces(void);
float R_CalcLod(const vec3_t origin, float radius);
int R_LerpTag(orientation_t* tag, qhandle_t handle, int startFrame, int endFrame,
    float frac, const char* tagName);
void R_PrintInfoWorldtris(void);

/*
=============================================================

UTIL

=============================================================
*/
int RE_GetShaderHeight(qhandle_t hShader);
int RE_GetShaderWidth(qhandle_t hShader);
void RB_StreamBegin(shader_t* shader);
void RB_StreamEnd(void);
void RB_StreamBeginDrawSurf(void);
void RB_StreamEndDrawSurf(void);
static void addTriangle(void);
void RB_Vertex3fv(vec3_t v);
void RB_Vertex3f(vec_t x, vec_t y, vec_t z);
void RB_Vertex2f(vec_t x, vec_t y);
void RB_Color4f(vec_t r, vec_t g, vec_t b, vec_t a);
void RB_Color3f(vec_t r, vec_t g, vec_t b);
void RB_Color3fv(vec3_t col);
void RB_Color4bv(unsigned char* colors);
void RB_Texcoord2f(float s, float t);
void RB_Texcoord2fv(vec2_t st);
void R_DrawDebugNumber(const vec3_t org, float number, float scale, float r, float g, float b, int precision);
void R_DebugRotatedBBox(const vec3_t org, vec3_t ang, vec3_t mins, vec3_t maxs, float r, float g, float b, float alpha);
const char* RE_GetGraphicsInfo();

/*
=============================================================

WORLD MAP

=============================================================
*/
void R_GetInlineModelBounds(int iIndex, vec3_t vMins, vec3_t vMaxs);

extern int g_nStaticSurfaces;
extern qboolean g_bInfostaticmodels;
extern qboolean g_bInfoworldtris;

//
//
//

void R_DebugCircle(const vec3_t org, float radius, float r, float g, float b, float alpha, qboolean horizontal);
void R_DebugLine(const vec3_t start, const vec3_t end, float r, float g, float b, float alpha);

void RE_SetRenderTime(int t);
qboolean R_SetMode(int mode, const glconfig_t* glConfig);
void R_SetFullscreen(qboolean fullscreen);
void R_SavePerformanceCounters(void);
void R_SyncRenderThread(void);
