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

#pragma once

#include "../qcommon/q_shared.h"
#include "../qcommon/qfiles.h"
#include "../qcommon/qcommon.h"
#include "tr_public.h"
#include "../renderercommon/qgl.h"
#include "qgl.h"

#ifdef __cplusplus
#define GLE(ret, name, ...) extern "C" name##proc * qgl##name;
#else
#define GLE(ret, name, ...) extern name##proc * qgl##name;
#endif

QGL_1_1_PROCS;
QGL_1_1_FIXED_FUNCTION_PROCS;
QGL_DESKTOP_1_1_PROCS;
QGL_DESKTOP_1_1_FIXED_FUNCTION_PROCS;
QGL_3_0_PROCS;

#undef GLE

#ifdef __cplusplus
extern "C" {
#endif

#define GL_INDEX_TYPE		GL_UNSIGNED_INT
typedef unsigned int glIndex_t;

// fast float to int conversion
#if id386 && !( (defined __linux__ || defined __FreeBSD__ ) && (defined __i386__ ) ) // rb010123
long myftol( float f );
#else
#define	myftol(x) ((int)(x))
#endif


// everything that is needed by the backend needs
// to be double buffered to allow it to run in
// parallel on a dual cpu machine
#define	SMP_FRAMES		2

// 12 bits
// see QSORT_SHADERNUM_SHIFT
#define	MAX_SHADERS				16384

//#define MAX_SHADER_STATES 2048
#define MAX_STATES_PER_SHADER 32
#define MAX_STATE_NAME 32

// can't be increased without changing bit packing for drawsurfs

typedef struct skelSurfaceGame_s skelSurfaceGame_t;
typedef struct staticSurface_s staticSurface_t;

// any changes in surfaceType must be mirrored in rb_surfaceTable[]
typedef enum {
    SF_BAD,
    SF_SKIP,				// ignore
    SF_FACE,
    SF_GRID,
    SF_POLY,
    SF_MARK_FRAG,
    SF_FLARE,
    SF_ENTITY,				// beams, rails, lightning, etc that can be determined by entity
    SF_DISPLAY_LIST,
    SF_TIKI_SKEL,
    SF_TIKI_STATIC,
    SF_SWIPE,
    SF_SPRITE,
    SF_TERRAIN_PATCH,
    SF_TRIANGLES,
    SF_MD3,
    SF_MD4,

    SF_NUM_SURFACE_TYPES,
    SF_MAX = 0x7fffffff			// ensures that sizeof( surfaceType_t ) == sizeof( int )
} surfaceType_t;

typedef struct {
    struct mnode_s* cntNode;
    struct msurface_s* skySurfs[32];
    int numSurfs;
    vec3_t offset;
    vec3_t mins;
    vec3_t maxs;
} portalsky_t;

typedef struct {
    vec3_t transformed;
    int index;
} sphere_dlight_t;

typedef enum {
    LIGHT_POINT,
    LIGHT_DIRECTIONAL,
    LIGHT_SPOT,
    LIGHT_SPOT_FAST
} lighttype_t;

typedef struct reallightinfo_s {
    vec3_t color;
    lighttype_t eType;
    float fIntensity;
    float fDist;
    float fSpotSlope;
    float fSpotConst;
    float fSpotScale;
    vec3_t vOrigin;
    vec3_t vDirection;
} reallightinfo_t;

typedef float cube_entry_t[3][4];

typedef struct {
    vec3_t origin;
    vec3_t worldOrigin;
    vec3_t traceOrigin;
    float radius;
    struct mnode_s* leaves[8];
    void(*TessFunction) (unsigned char* dstColors);
    union {
        unsigned char level[4];
        int value;
    } ambient;
    int numRealLights;
    reallightinfo_t light[MAX_REAL_LIGHTS];
    int bUsesCubeMap;
    float cubemap[24][3][4];
} sphereor_t;

typedef struct spherel_s {
    vec3_t origin;
	vec3_t color;
    float intensity;
    struct mnode_s* leaf;
    int needs_trace;
    int spot_light;
    float spot_radiusbydistance;
	vec3_t spot_dir;
    int reference_count;
} spherel_t;

typedef struct dlight_s {
	vec3_t	origin;
	vec3_t	color;				// range from 0.0 to 1.0, should be color normalized
	float	radius;

	dlighttype_t type;
	vec3_t	transformed;		// origin in local coordinate system
} dlight_t;


// a trRefEntity_t has all the information passed in by
// the client game, as well as some locally derived info
typedef struct {
	refEntity_t	e;

	float		axisLength;		// compensate for non-normalized axis

	qboolean	needDlights;	// true for bmodels that touch a dlight
	qboolean	bLightGridCalculated;
	int			iGridLighting;
	float		lodpercentage[2];
	qboolean	sphereCalculated;
	int			lightingSphere;

	//
	// old lighting variables
	//
	vec3_t		lightDir;		// normalized direction towards light
	vec3_t		ambientLight;	// color normalized to 0-255
	int			ambientLightInt;	// 32 bit rgba packed
	vec3_t		directedLight;
} trRefEntity_t;

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

typedef struct {
	vec3_t		origin;			// in world coordinates
	vec3_t		axis[3];		// orientation in world
	vec3_t		viewOrigin;		// viewParms->or.origin in local coordinates
	float		modelMatrix[16];
} orientationr_t;

typedef struct image_s {
	char		imgName[MAX_QPATH];		// game path, including extension
	int			width, height;				// source image
	int			uploadWidth, uploadHeight;	// after power of two and picmip but not including clamp to MAX_TEXTURE_SIZE
	GLuint		texnum;					// gl texture binding

	int			frameUsed;			// for texture usage in frame statistics
	int			bytesUsed;

	int			internalFormat;
	int			TMU;				// only needed for voodoo2
	int			numMipmaps;

	qboolean	dynamicallyUpdated;
	qboolean	allowPicmip;
	qboolean	force32bit;
    int			wrapClampModeX;
    int			wrapClampModeY;
	int			r_sequence;

	struct image_s*	next;
} image_t;

//===============================================================================

typedef enum {
	SS_BAD,
	SS_PORTAL,			// mirrors, portals, viewscreens
	SS_PORTALSKY,
	SS_ENVIRONMENT,		// sky box
	SS_OPAQUE,			// opaque

	SS_DECAL,			// scorch marks, etc.
	SS_SEE_THROUGH,		// ladders, grates, grills that may have small blended edges
						// in addition to alpha test
	SS_BANNER,

	SS_UNDERWATER,		// for items that should be drawn in front of the water plane

	SS_BLEND0,			// regular transparency and filters
	SS_BLEND1,			// generally only used for additive type effects
	SS_BLEND2,
	SS_BLEND3,

	SS_BLEND6,
	SS_STENCIL_SHADOW,
	SS_ALMOST_NEAREST,	// gun smoke puffs

	SS_NEAREST			// blood blobs
} shaderSort_t;


#define MAX_SHADER_STAGES 8

typedef enum {
	GF_NONE,

	GF_SIN,
	GF_SQUARE,
	GF_TRIANGLE,
	GF_SAWTOOTH, 
	GF_INVERSE_SAWTOOTH, 

	GF_NOISE

} genFunc_t;

typedef enum {
	USE_S_COORDS,
	USE_T_COORDS
} texDirection_t;

typedef enum {
	DEFORM_NONE,
	DEFORM_WAVE,
	DEFORM_NORMALS,
	DEFORM_BULGE,
	DEFORM_MOVE,
	DEFORM_AUTOSPRITE,
	DEFORM_AUTOSPRITE2,
	DEFORM_LIGHTGLOW,
	DEFORM_FLAP_S,
	DEFORM_FLAP_T
} deform_t;

typedef enum {
	AGEN_IDENTITY,
	AGEN_SKIP,
	AGEN_ENTITY,
	AGEN_ONE_MINUS_ENTITY,
	AGEN_VERTEX,
	AGEN_ONE_MINUS_VERTEX,
	AGEN_LIGHTING_SPECULAR,
	AGEN_WAVEFORM,
	AGEN_PORTAL,
	AGEN_NOISE,
	AGEN_DOT,
	AGEN_ONE_MINUS_DOT,
	AGEN_CONSTANT,
	AGEN_GLOBAL_ALPHA,
	AGEN_SKYALPHA,
	AGEN_ONE_MINUS_SKYALPHA,
	AGEN_SCOORD,
	AGEN_TCOORD,
	AGEN_DIST_FADE,
    AGEN_ONE_MINUS_DIST_FADE,
    AGEN_TIKI_DIST_FADE,
    AGEN_ONE_MINUS_TIKI_DIST_FADE,
	AGEN_DOT_VIEW,
	AGEN_ONE_MINUS_DOT_VIEW,
	AGEN_HEIGHT_FADE,
} alphaGen_t;

typedef enum {
	CGEN_BAD,
	CGEN_IDENTITY_LIGHTING,	// tr.identityLight
	CGEN_IDENTITY,			// always (1,1,1,1)
	CGEN_ENTITY,			// grabbed from entity's modulate field
	CGEN_ONE_MINUS_ENTITY,	// grabbed from 1 - entity.modulate
	CGEN_EXACT_VERTEX,		// tess.vertexColors
	CGEN_VERTEX,			// tess.vertexColors * tr.identityLight
	CGEN_ONE_MINUS_VERTEX,
	CGEN_WAVEFORM,			// programmatically generated
	CGEN_MULTIPLY_BY_WAVEFORM,
	CGEN_LIGHTING_GRID,
	CGEN_LIGHTING_SPHERICAL,
	CGEN_CONSTANT,
	CGEN_NOISE,
	CGEN_GLOBAL_COLOR,
	CGEN_STATIC,
	CGEN_SCOORD,
	CGEN_TCOORD,
	CGEN_DOT,
	CGEN_ONE_MINUS_DOT
} colorGen_t;

typedef enum {
	TCGEN_BAD,
	TCGEN_IDENTITY,			// clear to 0,0
	TCGEN_LIGHTMAP,
	TCGEN_TEXTURE,
	TCGEN_ENVIRONMENT_MAPPED,
	TCGEN_VECTOR,			// S and T from world coordinates
	TCGEN_ENVIRONMENT_MAPPED2,
	TCGEN_SUN_REFLECTION,
	TCGEN_FOG
} texCoordGen_t;

typedef enum {
	ACFF_NONE,
	ACFF_MODULATE_RGB,
	ACFF_MODULATE_RGBA,
	ACFF_MODULATE_ALPHA
} acff_t;

typedef struct {
	genFunc_t	func;

	float base;
	float amplitude;
	float phase;
	float frequency;
} waveForm_t;

#define TR_MAX_TEXMODS 4

typedef enum {
	TMOD_NONE,
	TMOD_TRANSFORM,
	TMOD_TURBULENT,
	TMOD_SCROLL,
	TMOD_SCALE,
	TMOD_STRETCH,
	TMOD_ROTATE,
	TMOD_ENTITY_TRANSLATE
} texMod_t;

#define	MAX_SHADER_DEFORMS	3
typedef struct {
	deform_t	deformation;			// vertex coordinate modification type

	vec3_t		moveVector;
	waveForm_t	deformationWave;
	float		deformationSpread;

	float		bulgeWidth;
	float		bulgeHeight;
	float		bulgeSpeed;
} deformStage_t;


typedef struct {
	texMod_t		type;

	// used for TMOD_TURBULENT and TMOD_STRETCH
	waveForm_t		wave;

	// used for TMOD_TRANSFORM
	float			matrix[2][2];		// s' = s * m[0][0] + t * m[1][0] + trans[0]
	float			translate[2];		// t' = s * m[0][1] + t * m[0][1] + trans[1]

	// used for TMOD_SCALE
	float			scale[2];			// s *= scale[0]
	                                    // t *= scale[1]

	// used for TMOD_SCROLL
	float			scroll[2];			// s' = s + scroll[0] * time
										// t' = t + scroll[1] * time

	// + = clockwise
	// - = counterclockwise
	float			rotateSpeed;

} texModInfo_t;


#define	MAX_IMAGE_ANIMATIONS	8
#define BUNDLE_ANIMATE_ONCE		1

typedef struct {
	image_t			*image[MAX_IMAGE_ANIMATIONS];
	int				numImageAnimations;
	float			imageAnimationSpeed;
	float			imageAnimationPhase;

	texCoordGen_t	tcGen;
	vec3_t			tcGenVectors[2];

	int				numTexMods;
	texModInfo_t	*texMods;

	int				videoMapHandle;
	qboolean		isLightmap;
	qboolean		vertexLightmap;
	qboolean		isVideoMap;
	int				flags;
} textureBundle_t;

#define NUM_TEXTURE_BUNDLES 2

typedef struct {
	qboolean		active;
	qboolean		hasNormalMap;
	
	textureBundle_t	bundle[NUM_TEXTURE_BUNDLES];
	image_t			*normalMap;
	int				multitextureEnv;		// 0, GL_MODULATE, GL_ADD (FIXME: put in stage)

	waveForm_t		rgbWave;
	colorGen_t		rgbGen;

	waveForm_t		alphaWave;
    alphaGen_t		alphaGen;

    unsigned		stateBits;					// GLS_xxxx mask


	qboolean		noMipMaps;
    qboolean		noPicMip;
    qboolean		force32bit;

	float			alphaMin;
	float			alphaMax;
	vec3_t			specOrigin;

    byte			colorConst[4];			// for CGEN_CONST and AGEN_CONST
	byte			alphaConst;
	byte			alphaConstMin;
} shaderStage_t;

struct shaderCommands_s;

#define LIGHTMAP_2D			-4		// shader is for 2D rendering
#define LIGHTMAP_BY_VERTEX	-3		// pre-lit triangle models
#define LIGHTMAP_WHITEIMAGE	-2
#define	LIGHTMAP_NONE		-1

typedef enum {
	CT_FRONT_SIDED,
	CT_BACK_SIDED,
	CT_TWO_SIDED
} cullType_t;

typedef enum {
	FP_NONE,		// surface is translucent and will just be adjusted properly
	FP_EQUAL,		// surface is opaque but possibly alpha tested
	FP_LE			// surface is trnaslucent, but still needs a fog pass (fog surface)
} fogPass_t;

typedef struct {
	float		cloudHeight;
	image_t		*outerbox[6], *innerbox[6];
} skyParms_t;

typedef enum {
	SPRITE_PARALLEL,
	SPRITE_PARALLEL_ORIENTED,
	SPRITE_ORIENTED,
	SPRITE_PARALLEL_UPRIGHT
} spriteType_t;

typedef struct {
  spriteType_t type;
  float scale;
} spriteParms_t;

typedef struct {
	vec3_t	color;
	float	depthForOpaque;
} fogParms_t;


typedef struct shader_s {
	char		name[MAX_QPATH];		// game path, including extension
	int			lightmapIndex;			// for a shader to match, both name and lightmapIndex must match

	int			index;					// this shader == tr.shaders[index]
	int			sortedIndex;			// this shader == tr.sortedShaders[sortedIndex]

	float		sort;					// lower numbered shaders draw before higher numbered

	qboolean	defaultShader;			// we want to return index 0 if the shader failed to
										// load for some reason, but R_FindShader should
										// still keep a name allocated for it, so if
										// something calls RE_RegisterShader again with
										// the same name, we don't try looking for it again

	qboolean	explicitlyDefined;		// found in a .shader file

	int			surfaceFlags;			// if explicitlyDefined, this will have SURF_* flags
	int			contentFlags;

	qboolean	entityMergable;			// merge across entites optimizable (smoke, blood)

	qboolean	isSky;
	skyParms_t	sky;
	spriteParms_t sprite;
	qboolean	isPortalSky;
	float subdivisions;
	float fDistRange;
	float fDistNear;
	fogParms_t	fogParms;

	float		portalRange;			// distance to fog out at

	cullType_t	cullType;				// CT_FRONT_SIDED, CT_BACK_SIDED, or CT_TWO_SIDED
	qboolean	polygonOffset;			// set for decals and other items that must be offset 
	qboolean	noMipMaps;				// for console fonts, 2D elements, etc.
	qboolean	noPicMip;				// for images that must always be full resolution

	fogPass_t	fogPass;				// draw a blended pass, possibly with depth test equals

	qboolean	needsNormal;			// not all shaders will need all data to be gathered
	qboolean	needsST1;
	qboolean	needsST2;
	qboolean	needsColor;

	int			numDeforms;
	deformStage_t	deforms[MAX_SHADER_DEFORMS];

	int			numUnfoggedPasses;
	shaderStage_t	* unfoggedStages[MAX_SHADER_STAGES];

	int			needsLGrid;
	int			needsLSpherical;
	int			stagesWithAlphaFog;
	int			flags;

	void		(*optimalStageIteratorFunc)( void );

  float clampTime;                                  // time this shader is clamped to
  float timeOffset;                                 // current time offset for this shader

  int numStates;                                    // if non-zero this is a state shader
  struct shader_s *currentShader;                   // current state if this is a state shader
  struct shader_s *parentShader;                    // current state if this is a state shader
  int currentState;                                 // current state index for cycle purposes
  long expireTime;                                  // time in milliseconds this expires

  struct shader_s *remappedShader;                  // current shader this one is remapped too

  int shaderStates[MAX_STATES_PER_SHADER];          // index to valid shader states

	struct	shader_s	*next;
} shader_t;

typedef struct shaderState_s {
  char shaderName[MAX_QPATH];     // name of shader this state belongs to
  char name[MAX_STATE_NAME];      // name of this state
  char stateShader[MAX_QPATH];    // shader this name invokes
  int cycleTime;                  // time this cycle lasts, <= 0 is forever
  shader_t *shader;
} shaderState_t;


// trRefdef_t holds everything that comes in refdef_t,
// as well as the locally generated scene information
typedef struct {
	int			x, y, width, height;
	float		fov_x, fov_y;
	vec3_t		vieworg;
	vec3_t		viewaxis[3];		// transformation matrix

	int			time;				// time in milliseconds for shader effects and other time dependent rendering issues
	int			rdflags;			// RDF_NOWORLDMODEL, etc

	// 1 bits will prevent the associated area from rendering at all
	byte		areamask[MAX_MAP_AREA_BYTES];
	qboolean	areamaskModified;	// qtrue if areamask changed since last scene

	float		floatTime;			// tr.refdef.time / 1000.0

	// text messages for deform text shaders
	char		text[MAX_RENDER_STRINGS][MAX_RENDER_STRING_LENGTH];

	int			num_entities;
	trRefEntity_t	*entities;

    int			num_sprites;
    refSprite_t	*sprites;

	int			num_dlights;
	struct dlight_s	*dlights;

	int			numTerMarks;
	struct srfMarkFragment_s *terMarks;

	int			numPolys;
	struct srfPoly_s	*polys;

	int			numDrawSurfs;
	struct drawSurf_s	*drawSurfs;

	int			numSpriteSurfs;
	struct drawSurf_s	*spriteSurfs;

    int			numStaticModels;
    struct cStaticModelUnpacked_s *staticModels;

    int			numStaticModelData;
    unsigned char		*staticModelData;
    qboolean sky_portal;
    float sky_alpha;
    vec3_t sky_origin;
    vec3_t sky_axis[3];
	// added in 2.0
	//==
    qboolean skybox_farplane;
    qboolean render_terrain;
	//==

} trRefdef_t;


//=================================================================================

// skins allow models to be retextured without modifying the model file
typedef struct {
	char		name[MAX_QPATH];
	shader_t	*shader;
} skinSurface_t;

typedef struct skin_s {
	char		name[MAX_QPATH];		// game path, including extension
	int			numSurfaces;
	skinSurface_t	*surfaces[MD3_MAX_SURFACES];
} skin_t;


typedef struct {
	int			originalBrushNumber;
	vec3_t		bounds[2];

	unsigned	colorInt;				// in packed byte format
	float		tcScale;				// texture coordinate vector scales
	fogParms_t	parms;

	// for clipping distance in fog when outside
	qboolean	hasSurface;
	float		surface[4];
} fog_t;

typedef struct depthfog_s {
	float len;
	float oolen;
	int enabled;
	int extrafrustums;
} depthfog_t;

typedef struct {
	orientationr_t	ori;
	orientationr_t	world;
	vec3_t		pvsOrigin;			// may be different than or.origin for portals
	qboolean	isPortal;			// true if this view is through a portal
	qboolean	isMirror;			// the portal is a mirror, invert the face culling
    qboolean	isPortalSky;		// since 2.0 whether or not this view is a portal sky
	int			frameSceneNum;		// copied from tr.frameSceneNum
	int			frameCount;			// copied from tr.frameCount
	cplane_t	portalPlane;		// clip anything behind this if mirroring
	int			viewportX, viewportY, viewportWidth, viewportHeight;
	float		fovX, fovY;
	float		projectionMatrix[16];
	cplane_t	frustum[5];
	vec3_t		visBounds[2];
	float		zFar;
	depthfog_t	fog;
    float		farplane_distance;
    float		farplane_bias; // added in 2.0
	float		farplane_color[3];
    qboolean	farplane_cull;
    qboolean	renderTerrain; // added in 2.0
} viewParms_t;


/*
==============================================================================

SURFACES

==============================================================================
*/

typedef struct drawSurf_s {
	unsigned			sort;			// bit combination for fast compares
	surfaceType_t		*surface;		// any of surface*_t
} drawSurf_t;

#define	MAX_FACE_POINTS		64

#define	MAX_PATCH_SIZE		32			// max dimensions of a patch mesh in map file
#define	MAX_GRID_SIZE		65			// max dimensions of a grid mesh in memory

// when cgame directly specifies a polygon, it becomes a srfPoly_t
// as soon as it is called
typedef struct srfPoly_s {
	surfaceType_t	surfaceType;
	qhandle_t		hShader;
	int				numVerts;
	polyVert_t		*verts;
	int				renderfx;
} srfPoly_t;

typedef struct srfMarkFragment_s {
	surfaceType_t surfaceType;
	int iIndex;
	int numVerts;
	polyVert_t* verts;
} srfMarkFragment_t;

typedef struct srfDisplayList_s {
	surfaceType_t	surfaceType;
	int				listNum;
} srfDisplayList_t;


typedef struct srfFlare_s {
	surfaceType_t	surfaceType;
	vec3_t			origin;
	vec3_t			normal;
	vec3_t			color;
} srfFlare_t;

typedef struct srfGridMesh_s {
	surfaceType_t	surfaceType;

	// dynamic lighting information
    int				dlightBits[SMP_FRAMES];
    int				dlightMap[SMP_FRAMES];

	// culling information
	vec3_t			meshBounds[2];
	vec3_t			localOrigin;
	float			meshRadius;

	// lightmap data
    float			lightmapOffset[2];
    int				lmX;
    int				lmY;
    int				lmWidth;
    int				lmHeight;
    unsigned char	*lmData;

	// lod information, which may be different
	// than the culling information to allow for
	// groups of curves that LOD as a unit
	vec3_t			lodOrigin;
	float			lodRadius;
	int				lodFixed;
	int				lodStitched;

	// vertexes
	int				width, height;
	float			*widthLodError;
	float			*heightLodError;
	drawVert_t		verts[1];		// variable sized
} srfGridMesh_t;



#define	VERTEXSIZE	8
typedef struct {
	surfaceType_t	surfaceType;
	cplane_t	plane;

	// dynamic lighting information
    int			dlightBits[SMP_FRAMES];
    int			dlightMap[SMP_FRAMES];
    float		lightmapOffset[2];
    int			lmWidth;
    int			lmHeight;
    int			lmX;
    int			lmY;
    byte*		lmData;
    vec3_t		lmOrigin;
    vec3_t		lmVecs[2];
    vec3_t		lmInverseVecs[2];

	// triangle definitions (no normals at points)
	int			numPoints;
	int			numIndices;
	int			ofsIndices;
	float		points[1][VERTEXSIZE];	// variable sized
										// there is a variable length list of indices here also
} srfSurfaceFace_t;


// misc_models in maps are turned into direct geometry by q3map
typedef struct {
	surfaceType_t	surfaceType;

	// dynamic lighting information
	int				dlightBits[SMP_FRAMES];

	// culling information (FIXME: use this!)
	vec3_t			bounds[2];
	vec3_t			localOrigin;
	float			radius;

	// triangle definitions
	int				numIndexes;
	int				*indexes;

	int				numVerts;
	drawVert_t		*verts;
} srfTriangles_t;

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
    sphere_dlight_t dlights[MAX_DLIGHTS];
    int numdlights;
    float radius;
    float cull_radius;
    int iGridLighting;
    float lodpercentage[2];
} cStaticModelUnpacked_t;

extern	void (*rb_surfaceTable[SF_NUM_SURFACE_TYPES])(void *);

/*
==============================================================================

BRUSH MODELS

==============================================================================
*/


//
// in memory representation
//

#define	SIDE_FRONT	0
#define	SIDE_BACK	1
#define	SIDE_ON		2

typedef struct msurface_s {
	int					viewCount;		// if == tr.viewCount, already added
	int					frameCount;
	struct shader_s		*shader;
	int					fogIndex;

	surfaceType_t		*data;			// any of srf*_t
} msurface_t;



#define	CONTENTS_NODE		-1
typedef struct mnode_s {
	// common with leaf and node
	int			contents;		// -1 for nodes, to differentiate from leafs
	int			visframe;		// node needs to be traversed if current
	vec3_t		mins, maxs;		// for bounding box culling
	struct mnode_s	*parent;

	// node specific
	cplane_t	*plane;
	struct mnode_s	*children[2];	

	// leaf specific
	int			cluster;
    int			area;
    spherel_t**	lights;
    int			numlights;

	msurface_t	**firstmarksurface;
    int			nummarksurfaces;

    int			firstTerraPatch;
    int			numTerraPatches;
    int			firstStaticModel;
    int			numStaticModels;
    void**		pFirstMarkFragment;
    int			iNumMarkFragment;
} mnode_t;

typedef struct {
	vec3_t		bounds[2];		// for culling
	msurface_t	*firstSurface;
	int			numSurfaces;
    void** pFirstMarkFragment;
    int iNumMarkFragment;
    int frameCount;
    qboolean hasLightmap;
} bmodel_t;

typedef struct {
    float width;
    float height;
    float origin_x;
    float origin_y;
    float scale;
    shader_t* shader;
} sprite_t;

typedef struct {
	char		name[MAX_QPATH];		// ie: maps/tim_dm2.bsp
	char		baseName[MAX_QPATH];	// ie: tim_dm2

	int			dataSize;

	int			numShaders;
	dshader_t	*shaders;

	int			numBmodels;
	bmodel_t	*bmodels;

	int			numplanes;
	cplane_t	*planes;

	int			numnodes;		// includes leafs
	int			numDecisionNodes;
	mnode_t		*nodes;

	int			numsurfaces;
	msurface_t	*surfaces;

	int			nummarksurfaces;
	msurface_t	**marksurfaces;

	vec3_t		lightGridMins;
	vec3_t		lightGridSize;
	vec3_t		lightGridOOSize;
	int			lightGridBounds[3];
	unsigned short* lightGridOffsets;
	byte		*lightGridData;
	byte        lightGridPalette[768];

    int numTerraPatches;
    cTerraPatchUnpacked_t* terraPatches;
    cTerraPatchUnpacked_t* activeTerraPatches;

    int numVisTerraPatches;
    cTerraPatchUnpacked_t** visTerraPatches;

    int numStaticModelData;
    byte* staticModelData;

    int numStaticModels;
    cStaticModelUnpacked_t* staticModels;

    int numVisStaticModels;
    cStaticModelUnpacked_t** visStaticModels;

	int			numClusters;
	int			clusterBytes;
	const byte	*vis;			// may be passed in by CM_LoadMap to save space

	byte		*novis;			// clusterBytes of 0xff
	byte		*lighting;
} world_t;

//======================================================================

typedef enum {
	MOD_BAD,
	MOD_BRUSH,
	MOD_TIKI,
	MOD_SPRITE
} modtype_t;

typedef struct model_s {
	char		name[MAX_QPATH];
	modtype_t	type;
	int			index;				// model = tr.models[model->index]

    qboolean serveronly;
    union {
        bmodel_t* bmodel;
        dtiki_t* tiki;
        sprite_t* sprite;
    } d;
} model_t;


#define	MAX_MOD_KNOWN	1024

void		R_ModelInit (void);
model_t		*R_GetModelByHandle( qhandle_t hModel );
int			R_LerpTag( orientation_t *tag, qhandle_t handle, int startFrame, int endFrame, 
					 float frac, const char *tagName );

void		R_Modellist_f (void);

//====================================================
extern	refimport_t		ri;

#define	MAX_DRAWIMAGES			2048
#define	MAX_LIGHTMAPS			256
#define	MAX_SKINS				1024


#define	MAX_DRAWSURFS			0x10000
#define	MAX_SPRITESURFS			0x8000
#define	DRAWSURF_MASK			(MAX_DRAWSURFS-1)

#define MAX_SPRITE_DIST				16384.0f
#define MAX_SPRITE_DIST_SQUARED		(MAX_SPRITE_DIST * MAX_SPRITE_DIST)

/*

the drawsurf sort data is packed into a single 32 bit value so it can be
compared quickly during the qsorting process

the bits are allocated as follows:

21 - 31	: sorted shader index
11 - 20	: entity index
2 - 6	: fog index
//2		: used to be clipped flag REMOVED - 03.21.00 rad
0 - 1	: dlightmap index

	TTimo - 1.32
17-31 : sorted shader index
7-16  : entity index
2-6   : fog index
0-1   : dlightmap index
*/
#define	QSORT_SHADERNUM_SHIFT	21 // was 22, decreased in 2.0
#define	QSORT_ENTITYNUM_SHIFT	8
#define	QSORT_FOGNUM_SHIFT		2
#define	QSORT_REFENTITYNUM_SHIFT	7
#define	QSORT_STATICMODEL_SHIFT	20 // was 21, decreased in 2.0

extern	int			gl_filter_min, gl_filter_max;

/*
** performanceCounters_t
*/
typedef struct {
	int		c_sphere_cull_patch_in, c_sphere_cull_patch_clip, c_sphere_cull_patch_out;
	int		c_box_cull_patch_in, c_box_cull_patch_clip, c_box_cull_patch_out;
	int		c_sphere_cull_md3_in, c_sphere_cull_md3_clip, c_sphere_cull_md3_out;
	int		c_box_cull_md3_in, c_box_cull_md3_clip, c_box_cull_md3_out;

	int		c_leafs;
	int		c_dlightSurfaces;
	int		c_dlightSurfacesCulled;
	int		c_dlightMaps;
	int		c_dlightTexels;
} frontEndCounters_t;

#define	FOG_TABLE_SIZE		256
#define FUNCTABLE_SIZE		1024
#define FUNCTABLE_SIZE2		10
#define FUNCTABLE_MASK		(FUNCTABLE_SIZE-1)


// the renderer front end should never modify glstate_t
typedef struct {
	int			currenttextures[2];
	int			currenttmu;
	qboolean	finishCalled;
	int			texEnv[2];
	int			faceCulling;
	int			cntTexEnvExt;
	int			cntnvblendmode;
	long unsigned int glStateBits;
	long unsigned int externalSetState;
	vec4_t fFogColor;
} glstate_t;


typedef struct {
	int		c_surfaces, c_shaders, c_vertexes, c_indexes, c_totalIndexes, c_characterlights;
	float	c_overDraw;
	
	int		c_dlightVertexes;
	int		c_dlightIndexes;

	int		c_flareAdds;
	int		c_flareTests;
	int		c_flareRenders;

	int		msec;			// total msec for backend run
} backEndCounters_t;

// all state modified by the back end is seperated
// from the front end state
typedef struct {
	int			smpFrame;
	trRefdef_t	refdef;
	viewParms_t	viewParms;
	orientationr_t	ori;
	backEndCounters_t	pc;
	qboolean	isHyperspace;
	trRefEntity_t	*currentEntity;
    qboolean skyRenderedThisView;	// flag for drawing sun
    sphereor_t spheres[128];
    unsigned char numSpheresUsed;
    sphereor_t* currentSphere;
    sphereor_t spareSphere;
    sphereor_t hudSphere;
    cStaticModelUnpacked_t* currentStaticModel;
    int dsStreamVert;
	qboolean in2D;	// if qtrue, drawstretchpic doesn't need to change modes
	byte color2D[4];
	qboolean vertexes2D;		// shader needs to be finished
	trRefEntity_t entity2D;	// currentEntity will point at this when doing 2D rendering
    int backEndMsec;
    float shaderStartTime;
} backEndState_t;

/*
** trGlobals_t 
**
** Most renderer globals are defined here.
** backend functions should never modify any of these fields,
** but may read fields that aren't dynamically modified
** by the frontend.
*/
typedef struct {
	qboolean				registered;		// cleared at shutdown, set at beginRegistration

	int						visCount;		// incremented every time a new vis cluster is entered
	int						frameCount;		// incremented every frame
	int						sceneCount;		// incremented every scene
	int						viewCount;		// incremented every view (twice a scene if portaled)
											// and every R_MarkFragments call

	int						smpFrame;		// toggles from 0 to 1 every endFrame

	int						frameSceneNum;	// zeroed at RE_BeginFrame

	qboolean				worldMapLoaded;
	world_t					*world;

	const byte				*externalVisData;	// from RE_SetWorldVisData, shared with CM_Load

	image_t					*defaultImage;
	image_t					*scratchImage;
	image_t					*fogImage;
	image_t					*dlightImage;	// inverse-quare highlight for projective adding
	image_t					*flareImage;
	image_t					*whiteImage;			// full of 0xff
	image_t					*identityLightImage;	// full of tr.identityLightByte
	image_t					*dlightImages[15];

	shader_t				*defaultShader;
	shader_t				*shadowShader;
	shader_t				*projectionShadowShader;

	shader_t				*flareShader;
	shader_t				*sunShader;

	int						numLightmaps;
	image_t					*lightmaps[MAX_LIGHTMAPS];

	trRefEntity_t			*currentEntity;
	trRefEntity_t			worldEntity;		// point currentEntity at this when rendering world
	int						currentEntityNum;
	int						currentSpriteNum;
	int						shiftedEntityNum;	// currentEntityNum << QSORT_ENTITYNUM_SHIFT
	int                     shiftedIsStatic;
	model_t					*currentModel;

	viewParms_t				viewParms;

	float					identityLight;		// 1.0 / ( 1 << overbrightBits )
	int						identityLightByte;	// identityLight * 255
	int						overbrightBits;		// r_overbrightBits->integer, but set to 0 if no hw gamma
    int						overbrightShift;
    float					overbrightMult;
    int						needsLightScale;

	orientationr_t			ori;				// for current entity

	portalsky_t				portalsky;
	qboolean				skyRendered;
	qboolean				portalRendered;
	trRefdef_t				refdef;

	int						viewCluster;

	vec3_t					sunLight;			// from the sky shader for this level
	vec3_t					sunDirection;

	frontEndCounters_t		pc;
	int						frontEndMsec;		// not in pc due to clearing issue

	//
	// put large tables at the end, so most elements will be
	// within the +/32K indexed range on risc processors
	//
	model_t					models[MAX_MOD_KNOWN];
	int						numModels;

	int						numImages;
	image_t					images[MAX_DRAWIMAGES];

	// shader indexes from other modules will be looked up in tr.shaders[]
	// shader indexes from drawsurfs will be looked up in sortedShaders[]
	// lower indexed sortedShaders must be rendered first (opaque surfaces before translucent)
	int						numShaders;
	shader_t				*shaders[MAX_SHADERS];
	shader_t				*sortedShaders[MAX_SHADERS];

	int						numSkins;
	skin_t					*skins[MAX_SKINS];

	float					sinTable[FUNCTABLE_SIZE];
	float					squareTable[FUNCTABLE_SIZE];
	float					triangleTable[FUNCTABLE_SIZE];
	float					sawToothTable[FUNCTABLE_SIZE];
	float					inverseSawToothTable[FUNCTABLE_SIZE];

    spherel_t sSunLight;
    spherel_t sLights[1532];
    int numSLights;
    int rendererhandle;
    qboolean shadersParsed;
    int frame_skel_index;
    int skel_index[1024];
    fontheader_t* pFontDebugStrings;

	int farclip;
} trGlobals_t;

extern backEndState_t	backEnd;
extern trGlobals_t	tr;
extern glconfig_t	glConfig;		// outside of TR since it shouldn't be cleared during ref re-init
extern glstate_t	glState;		// outside of TR since it shouldn't be cleared during ref re-init
extern int r_sequencenumber;


//
// cvars
//
extern cvar_t	*r_flareSize;
extern cvar_t	*r_flareFade;

extern cvar_t	*r_ignore;				// used for debugging anything
extern cvar_t	*r_verbose;				// used for verbose debug spew
extern cvar_t	*r_ignoreFastPath;		// allows us to ignore our Tess fast paths

extern cvar_t	*r_znear;				// near Z clip plane

extern cvar_t	*r_stencilbits;			// number of desired stencil bits
extern cvar_t	*r_depthbits;			// number of desired depth bits
extern cvar_t	*r_colorbits;			// number of desired color bits, only relevant for fullscreen
extern cvar_t	*r_stereo;				// desired pixelformat stereo flag
extern cvar_t	*r_textureDetails;
extern cvar_t	*r_texturebits;			// number of desired texture bits
										// 0 = use framebuffer depth
										// 16 = use 16-bit textures
										// 32 = use 32-bit textures
										// all else = error

extern cvar_t	*r_measureOverdraw;		// enables stencil buffer overdraw measurement

extern cvar_t	*r_lodscale;

extern cvar_t	*r_primitives;			// "0" = based on compiled vertex array existance
										// "1" = glDrawElemet tristrips
										// "2" = glDrawElements triangles
										// "-1" = no drawing

extern cvar_t	*r_inGameVideo;				// controls whether in game video should be draw
extern cvar_t	*r_fastsky;				// controls whether sky should be cleared or drawn
extern cvar_t	*r_fastdlights;
extern cvar_t	*r_drawSun;				// controls drawing of sun quad
extern cvar_t	*r_dlightBacks;			// dlight non-facing surfaces for continuity

extern	cvar_t	*r_norefresh;			// bypasses the ref rendering
extern	cvar_t	*r_drawentities;		// disable/enable entity rendering
extern	cvar_t	*r_drawentitypoly;
extern	cvar_t	*r_drawstaticmodels;
extern	cvar_t	*r_drawstaticmodelpoly;
extern	cvar_t	*r_drawbrushes;
extern	cvar_t	*r_drawbrushmodels;
extern	cvar_t	*r_drawstaticdecals;
extern	cvar_t	*r_drawterrain;
extern	cvar_t	*r_drawsprites;
extern	cvar_t	*r_drawspherelights;
extern	cvar_t	*r_drawworld;			// disable/enable world rendering
extern	cvar_t	*r_speeds;				// various levels of information display
extern  cvar_t	*r_detailTextures;		// enables/disables detail texturing stages
extern	cvar_t	*r_novis;				// disable/enable usage of PVS
extern	cvar_t	*r_nocull;
extern	cvar_t	*r_showcull;
extern	cvar_t	*r_facePlaneCull;		// enables culling of planar surfaces with back side test
extern	cvar_t	*r_nocurves;
extern	cvar_t	*r_showcluster;

extern cvar_t	*r_mode;				// video mode
extern cvar_t	*r_fullscreen;
extern cvar_t	*r_gamma;
extern cvar_t	*r_displayRefresh;		// optional display refresh option
extern cvar_t	*r_ignorehwgamma;		// overrides hardware gamma capabilities

extern cvar_t	*r_allowExtensions;				// global enable/disable of OpenGL extensions
extern cvar_t	*r_ext_compressed_textures;		// these control use of specific extensions
extern cvar_t	*r_ext_gamma_control;
extern cvar_t	*r_ext_texenv_op;
extern cvar_t	*r_ext_multitexture;
extern cvar_t	*r_ext_compiled_vertex_array;
extern cvar_t	*r_ext_texture_env_add;
extern cvar_t	*r_ext_texture_env_combine;
extern cvar_t	*r_ext_aniso_filter;
extern cvar_t	*r_forceClampToEdge;
extern cvar_t	*r_geForce3WorkAround;
extern cvar_t	*r_reset_tc_array;

extern	cvar_t	*r_nobind;						// turns off binding to appropriate textures
extern	cvar_t	*r_singleShader;				// make most world faces use default shader
extern	cvar_t	*r_lerpmodels;
extern	cvar_t	*r_roundImagesDown;
extern	cvar_t	*r_colorMipLevels;				// development aid to see texture mip usage
extern	cvar_t	*r_picmip;						// controls picmip values
extern	cvar_t	*r_finish;
extern	cvar_t	*r_drawBuffer;
extern  cvar_t  *r_glDriver;
extern	cvar_t	*r_swapInterval;
extern	cvar_t	*r_textureMode;
extern	cvar_t	*r_offsetFactor;
extern	cvar_t	*r_offsetUnits;

extern	cvar_t	*r_fullbright;					// avoid lightmap pass
extern	cvar_t	*r_lightmap;					// render lightmaps only
extern	cvar_t	*r_vertexLight;					// vertex lighting mode for better performance

extern	cvar_t	*r_logFile;						// number of frames to emit GL logs
extern	cvar_t	*r_showtris;					// enables wireframe rendering of the world
extern	cvar_t	*r_showsky;						// forces sky in front of all surfaces
extern	cvar_t	*r_shownormals;					// draws wireframe normals
extern	cvar_t	*r_showhbox;
extern	cvar_t	*r_showstaticbboxes;
extern	cvar_t	*r_clear;						// force screen clear every frame

extern	cvar_t	*r_shadows;						// controls shadows: 0 = none, 1 = blur, 2 = stencil, 3 = black planar projection
extern	cvar_t	*r_entlight_scale;
extern	cvar_t	*r_entlight_errbound;
extern	cvar_t	*r_entlight_cubelevel;
extern	cvar_t	*r_entlight_cubefraction;
extern	cvar_t	*r_entlight_maxcalc;
extern	cvar_t	*r_flares;						// light flares

extern	cvar_t	*r_intensity;

extern	cvar_t	*r_lockpvs;
extern	cvar_t	*r_noportals;
extern	cvar_t	*r_entlightmap;
extern	cvar_t	*r_fastentlight;
extern	cvar_t	*r_portalOnly;

extern	cvar_t	*r_subdivisions;
extern	cvar_t	*r_lodCurveError;
extern	cvar_t	*r_smp;
extern	cvar_t	*r_showSmp;
extern	cvar_t	*r_skipBackEnd;

extern	cvar_t	*r_ignoreGLErrors;

extern	cvar_t	*r_overBrightBits;
extern	cvar_t	*r_mapOverBrightBits;

extern	cvar_t	*r_debugSurface;

extern	cvar_t	*r_showImages;
extern	cvar_t	*r_showlod;
extern	cvar_t	*r_showstaticlod;
extern	cvar_t	*r_debugSort;

extern	cvar_t	*r_printShaders;
extern	cvar_t	*r_saveFontData;

extern	cvar_t* r_staticlod;
extern	cvar_t* r_lodscale;
extern	cvar_t* r_lodcap;
extern	cvar_t* r_lodviewmodelcap;

extern  cvar_t* r_uselod;
extern  cvar_t* lod_LOD;
extern  cvar_t* lod_minLOD;
extern  cvar_t* lod_maxLOD;
extern  cvar_t* lod_LOD_slider;
extern  cvar_t* lod_curve_0_val;
extern  cvar_t* lod_curve_1_val;
extern  cvar_t* lod_curve_2_val;
extern  cvar_t* lod_curve_3_val;
extern  cvar_t* lod_curve_4_val;
extern  cvar_t* lod_edit_0;
extern  cvar_t* lod_edit_1;
extern  cvar_t* lod_edit_2;
extern  cvar_t* lod_edit_3;
extern  cvar_t* lod_edit_4;
extern  cvar_t* lod_curve_0_slider;
extern  cvar_t* lod_curve_1_slider;
extern  cvar_t* lod_curve_2_slider;
extern  cvar_t* lod_curve_3_slider;
extern  cvar_t* lod_curve_4_slider;
extern  cvar_t* lod_pitch_val;
extern  cvar_t* lod_zee_val;
extern  cvar_t* lod_mesh;
extern  cvar_t* lod_meshname;
extern  cvar_t* lod_tikiname;
extern  cvar_t* lod_metric;
extern  cvar_t* lod_tris;
extern  cvar_t* lod_position;
extern  cvar_t* lod_save;
extern  cvar_t* lod_tool;
extern	cvar_t* sys_cpuid;
extern	cvar_t* r_sse;
extern	cvar_t* r_static_shaderdata0;
extern	cvar_t* r_static_shaderdata1;
extern	cvar_t* r_static_shaderdata2;
extern	cvar_t* r_static_shaderdata3;
extern	cvar_t* r_static_shadermultiplier0;
extern	cvar_t* r_static_shadermultiplier1;
extern	cvar_t* r_static_shadermultiplier2;
extern	cvar_t* r_static_shadermultiplier3;

extern  cvar_t* r_numdebuglines;

extern	cvar_t* r_stipplelines;
extern	cvar_t* r_light_lines;
extern	cvar_t* r_light_sun_line;
extern	cvar_t* r_light_int_scale;
extern	cvar_t* r_light_nolight;
extern	cvar_t* r_light_showgrid;
extern	cvar_t* r_skyportal;
extern	cvar_t* r_skyportal_origin;
extern	cvar_t* r_farplane;
extern	cvar_t* r_farplane_bias;
extern	cvar_t* r_farplane_color;
extern	cvar_t* r_farplane_nocull;
extern	cvar_t* r_farplane_nofog;
extern	cvar_t* r_skybox_farplane;
extern	cvar_t* r_farclip;
extern	cvar_t* r_lightcoronasize;
extern	cvar_t* r_useglfog;
extern	cvar_t* r_debuglines_depthmask;
extern	cvar_t* r_smoothsmokelight;
extern	cvar_t* r_showportal;
extern	cvar_t* ter_minMarkRadius;
extern	cvar_t* ter_fastMarks;
extern	cvar_t* r_alpha_foliage1;
extern	cvar_t* r_alpha_foliage2;
extern	cvar_t* r_blendtrees;
extern	cvar_t* r_blendbushes;
extern	cvar_t* r_bumpmap;
extern	cvar_t* r_loadjpg;
extern	cvar_t* r_loadftx;

extern  cvar_t* r_showSkeleton;

//====================================================================

float R_NoiseGet4f( float x, float y, float z, float t );
void  R_NoiseInit( void );

void R_SwapBuffers( int );

void R_DebugCircle(const vec3_t org, float radius, float r, float g, float b, float alpha, qboolean horizontal);
void R_DebugLine(const vec3_t start, const vec3_t end, float r, float g, float b, float alpha);
void R_RenderView( viewParms_t *parms );

qboolean SurfIsOffscreen(const srfSurfaceFace_t* surface, shader_t* shader, int entityNum);

void R_AddMD3Surfaces( trRefEntity_t *e );
void R_AddNullModelSurfaces( trRefEntity_t *e );
void R_AddBeamSurfaces( trRefEntity_t *e );
void R_AddRailSurfaces( trRefEntity_t *e, qboolean isUnderwater );
void R_AddLightningBoltSurfaces( trRefEntity_t *e );

void R_AddPolygonSurfaces( void );

void R_DecomposeSort(unsigned int sort, int* entityNum, shader_t** shader, int* dlightMap, qboolean* bStaticModel);

void R_AddDrawSurf(surfaceType_t* surface, shader_t* shader, int dlightMap);


#define	CULL_IN		0		// completely unclipped
#define	CULL_CLIP	1		// clipped by one or more planes
#define	CULL_OUT	2		// completely outside the clipping planes
void R_LocalNormalToWorld (const vec3_t local, vec3_t world);
void R_LocalPointToWorld (const vec3_t local, vec3_t world);
int R_CullLocalBoxOffset(const vec3_t offset, vec3_t bounds[2]);
int R_CullLocalBox (vec3_t bounds[2]);
int R_CullPointAndRadius( vec3_t origin, float radius );
int R_CullLocalPointAndRadius( vec3_t origin, float radius );
int R_DistanceCullLocalPointAndRadius(float fDist, const vec3_t pt, float radius);
int R_DistanceCullPointAndRadius(float fDist, const vec3_t pt, float radius);

void R_RotateForEntity( const trRefEntity_t *ent, const viewParms_t *viewParms, orientationr_t *ori );
void R_RotateForStaticModel(cStaticModelUnpacked_t* SM, const viewParms_t* viewParms, orientationr_t* ori );
void R_RotateForViewer(void);
void R_SetupFrustum(void);

/*
** GL wrapper/helper functions
*/
void	GL_SetFogColor(const vec4_t fColor);
void	GL_Bind( image_t *image );
void	GL_SetDefaultState (void);
void	GL_SelectTexture( int unit );
void	GL_TextureMode( const char *string );
void	GL_CheckErrors( void );
void	GL_State( unsigned long stateVector );
void	GL_TexEnv( int env );
void	GL_Cull( int cullType );

#define GLS_SRCBLEND_ZERO						0x00000001
#define GLS_SRCBLEND_ONE						0x00000002
#define GLS_SRCBLEND_DST_COLOR					0x00000003
#define GLS_SRCBLEND_ONE_MINUS_DST_COLOR		0x00000004
#define GLS_SRCBLEND_SRC_ALPHA					0x00000005
#define GLS_SRCBLEND_ONE_MINUS_SRC_ALPHA		0x00000006
#define GLS_SRCBLEND_DST_ALPHA					0x00000007
#define GLS_SRCBLEND_ONE_MINUS_DST_ALPHA		0x00000008
#define GLS_SRCBLEND_ALPHA_SATURATE				0x00000009
#define		GLS_SRCBLEND_BITS					0x0000000f

#define GLS_DSTBLEND_ZERO						0x00000010
#define GLS_DSTBLEND_ONE						0x00000020
#define GLS_DSTBLEND_SRC_COLOR					0x00000030
#define GLS_DSTBLEND_ONE_MINUS_SRC_COLOR		0x00000040
#define GLS_DSTBLEND_SRC_ALPHA					0x00000050
#define GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA		0x00000060
#define GLS_DSTBLEND_DST_ALPHA					0x00000070
#define GLS_DSTBLEND_ONE_MINUS_DST_ALPHA		0x00000080
#define		GLS_DSTBLEND_BITS					0x000000f0

#define GLS_DEPTHMASK_TRUE						0x00000100
#define GLS_COLOR_MASK							0x00000200
#define GLS_POLYMODE_LINE						0x00000400
#define GLS_DEPTHTEST_DISABLE					0x00000800
#define GLS_DEPTHFUNC_EQUAL						0x00001000

#define GLS_CLAMP_EDGE							0x00002000
#define GLS_MULTITEXTURE						0x00004000
#define GLS_MULTITEXTURE_ENV					0x00008000
#define GLS_FOG									0x00010000

#define GLS_FOG_ENABLED							0x00020000
#define GLS_FOG_BLACK							0x00040000
#define GLS_FOG_WHITE							0x00080000
#define GLS_FOG_COLOR							(GLS_FOG_BLACK | GLS_FOG_WHITE)
#define GLS_FOG_BITS							(GLS_FOG_WHITE|GLS_FOG_BLACK|GLS_FOG_ENABLED)
#define GLS_COLOR_NOMASK						0x00100000

#define GLS_ATEST_GT_0							0x10000000
#define GLS_ATEST_LT_80							0x20000000
#define GLS_ATEST_GE_80							0x30000000
#define GLS_ATEST_LT_FOLIAGE1					0x40000000
#define GLS_ATEST_GE_FOLIAGE1					0x50000000
#define GLS_ATEST_LT_FOLIAGE2					0x60000000
#define GLS_ATEST_GE_FOLIAGE2					0x70000000
#define GLS_ATEST_BITS							0x70000000

#define GLS_DEFAULT			GLS_DEPTHMASK_TRUE

void Draw_SetColor(const vec4_t rgba);
void Draw_StretchPic(float x, float y, float w, float h, float s1, float t1, float s2, float t2, qhandle_t hShader);
void Draw_StretchPic2(float x, float y, float w, float h, float s1, float t1, float s2, float t2, float sx, float sy, qhandle_t hShader);
void Draw_TilePic(float x, float y, float w, float h, qhandle_t hShader);
void Draw_TilePicOffset(float x, float y, float w, float h, qhandle_t hShader, int offsetX, int offsetY);
void Draw_TrianglePic(const vec2_t vPoints[3], const vec2_t vTexCoords[3], qhandle_t hShader);
void DrawBox(float x, float y, float w, float h);
void AddBox(float x, float y, float w, float h);
void Set2DWindow(int x, int y, int w, int h, float left, float right, float bottom, float top, float n, float f);
void RE_Scissor(int x, int y, int width, int height);
void DrawLineLoop(const vec2_t* points, int count, int stipple_factor, int stipple_mask);
void RE_StretchRaw(int x, int y, int w, int h, int cols, int rows, int components, const byte* data);
void	RE_UploadCinematic (int w, int h, int cols, int rows, const byte *data, int client, qboolean dirty);

void		RE_BeginFrame( stereoFrame_t stereoFrame );
void		RE_BeginRegistration( glconfig_t *glconfig );
void		RE_LoadWorldMap( const char *mapname );
void		RE_PrintBSPFileSizes(void);
int			RE_MapVersion(void);
void		RE_SetWorldVisData( const byte *vis );
qhandle_t	RE_RegisterModel( const char *name );
qhandle_t	RE_RegisterSkin( const char *name );
void		RE_Shutdown( qboolean destroyWindow );

qboolean	R_GetEntityToken( char *buffer, int size );

model_t		*R_AllocModel( void );

void    	R_Init( void );
image_t		*R_FindImageFile(const char* name, qboolean mipmap, qboolean allowPicmip, qboolean force32bit, int glWrapClampModeX, int glWrapClampModeY);
image_t		*R_RefreshImageFile(const char* name, qboolean mipmap, qboolean allowPicmip, qboolean force32bit, int glWrapClampModeX, int glWrapClampModeY);

image_t* R_CreateImage(
	const char* name,
	byte* pic,
	int width,
	int height,
	int numMipmaps,
	int iMipmapsAvailable,
	qboolean allowPicmip,
	qboolean force32bit,
	qboolean hasAlpha,
	int glCompressMode,
	int glWrapClampModeX,
	int glWrapClampModeY
);
qboolean	R_GetModeInfo( int *width, int *height, float *windowAspect, int mode );

void		R_SetColorMappings( void );
void		R_GammaCorrect( byte *buffer, int bufSize );

void	R_ImageList_f( void );
void	R_SkinList_f( void );
// https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=516
const void *RB_TakeScreenshotCmd( const void *data );
void	R_ScreenShot_f( void );

void	R_InitImages( void );
void	R_FreeImage(image_t* image);
void	R_DeleteTextures( void );
int		R_SumOfUsedImages( void );
skin_t	*R_GetSkinByHandle( qhandle_t hSkin );


//
// tr_shader.c
//
qhandle_t		 RE_RegisterShader( const char *name );
qhandle_t		 RE_RegisterShaderNoMip( const char *name );
qhandle_t		RE_RefreshShaderNoMip(const char* name);
qhandle_t RE_RegisterShaderFromImage(const char *name, int lightmapIndex, image_t *image, qboolean mipRawImage);

shader_t* R_FindShader(const char* name, int lightmapIndex, qboolean mipRawImage, qboolean picmip, qboolean wrapx, qboolean wrapy);
shader_t	*R_GetShaderByHandle( qhandle_t hShader );
shader_t	*R_GetShaderByState( int index, long *cycleTime );
void R_StartupShaders();
void R_ShutdownShaders();
void R_SetupShaders();
void		R_ShaderList_f( void );
void    R_RemapShader(const char *oldShader, const char *newShader, const char *timeOffset);

/*
====================================================================

IMPLEMENTATION SPECIFIC FUNCTIONS

====================================================================
*/

void		GLimp_Init( qboolean fixedFunction );
void		GLimp_Shutdown( void );
void		GLimp_EndFrame( void );

qboolean	GLimp_SpawnRenderThread( void (*function)( void ) );
void		*GLimp_RendererSleep( void );
void		GLimp_FrontEndSleep( void );
void		GLimp_WakeRenderer( void *data );

void		GLimp_LogComment( char *comment );

// NOTE TTimo linux works with float gamma value, not the gamma table
//   the params won't be used, getting the r_gamma cvar directly
void		GLimp_SetGamma( unsigned char red[256], 
						    unsigned char green[256],
							unsigned char blue[256] );


/*
====================================================================

TESSELATOR/SHADER DECLARATIONS

====================================================================
*/
typedef byte color4ub_t[4];

typedef struct stageVars
{
	color4ub_t	colors[SHADER_MAX_VERTEXES];
	vec2_t		texcoords[NUM_TEXTURE_BUNDLES][SHADER_MAX_VERTEXES];
} stageVars_t;

typedef struct shaderCommands_s 
{
	glIndex_t	indexes[SHADER_MAX_INDEXES];
	vec4_t		xyz[SHADER_MAX_VERTEXES];
	vec4_t		normal[SHADER_MAX_VERTEXES];
	vec2_t		texCoords[SHADER_MAX_VERTEXES][2];
	color4ub_t	vertexColors[SHADER_MAX_VERTEXES];
	int			vertexDlightBits[SHADER_MAX_VERTEXES];

	stageVars_t	svars;

	color4ub_t	constantColor255[SHADER_MAX_VERTEXES];

	shader_t	*shader;
  float   shaderTime;
	int			fogNum;

	int			dlightBits;	// or together of all vertexDlightBits
	int			dlightMap;

	int			numIndexes;
	int			numVertexes;

	// info extracted from current shader
	int			numPasses;
	void		(*currentStageIteratorFunc)( void );
	shaderStage_t	**xstages;
    qboolean no_global_fog;
    qboolean vertexColorValid;
} shaderCommands_t;

extern	shaderCommands_t	tess;

void RB_BeginSurface(shader_t *shader );
void RB_EndSurface(void);
void RB_CheckOverflow( int verts, int indexes );
#define RB_CHECKOVERFLOW(v,i) if (tess.numVertexes + (v) >= SHADER_MAX_VERTEXES || tess.numIndexes + (i) >= SHADER_MAX_INDEXES ) {RB_CheckOverflow(v,i);}

void RB_StageIteratorGeneric( void );
void RB_StageIteratorSky( void );
void RB_StageIteratorVertexLitTextureUnfogged( void );
void RB_StageIteratorLightmappedMultitextureUnfogged( void );

void RB_AddQuadStamp( vec3_t origin, vec3_t left, vec3_t up, byte *color );
void RB_AddQuadStampExt( vec3_t origin, vec3_t left, vec3_t up, byte *color, float s1, float t1, float s2, float t2 );

void RB_ShowImages( void );


/*
============================================================

WORLD MAP

============================================================
*/

extern terraTri_t* g_pTris;
extern terrainVert_t* g_pVert;

void R_AddBrushModelSurfaces( trRefEntity_t *e );
void R_GetInlineModelBounds(int iIndex, vec3_t vMins, vec3_t vMaxs);
int R_SphereInLeafs(const vec3_t p, float r, mnode_t** nodes, int nMaxNodes);
mnode_t* R_PointInLeaf(const vec3_t p);
int R_DlightTerrain(cTerraPatchUnpacked_t* surf, int dlightBits);
int R_CheckDlightTerrain(cTerraPatchUnpacked_t* surf, int dlightBits);
void R_AddWorldSurfaces( void );
qboolean R_inPVS( const vec3_t p1, const vec3_t p2 );


/*
============================================================

FLARES

============================================================
*/

void R_ClearFlares( void );

void RB_AddFlare( void *surface, int fogNum, vec3_t point, vec3_t color, vec3_t normal );
void RB_AddDlightFlares( void );
void RB_RenderFlares (void);

/*
============================================================

LIGHTS

============================================================
*/

void R_DlightBmodel( bmodel_t *bmodel );
void R_GetLightingGridValue(const vec3_t vPos, vec3_t vLight);
void R_GetLightingForDecal(vec3_t vLight, const vec3_t vFacing, const vec3_t vOrigin);
void R_GetLightingForSmoke(vec3_t vLight, const vec3_t vOrigin);
void R_SetupEntityLighting( const trRefdef_t *refdef, trRefEntity_t *ent );
void RB_SetupEntityGridLighting();
void RB_SetupStaticModelGridLighting(trRefdef_t* refdef, cStaticModelUnpacked_t* ent, const vec3_t lightOrigin);
int R_RealDlightPatch(srfGridMesh_t* srf, int dlightBit);
int R_RealDlightFace(srfSurfaceFace_t* srf, int dlightBits);
int R_RealDlightTerrain(cTerraPatchUnpacked_t* srf, int dlightBits);
void R_TransformDlights( int count, dlight_t *dl, orientationr_t *ori );
void RB_Light_Real(unsigned char* colors);
void RB_Sphere_BuildDLights();
void RB_Sphere_SetupEntity();
void RB_Grid_SetupEntity();
void RB_Grid_SetupStaticModel();
void RB_Light_Fullbright(unsigned char* colors);
void R_Sphere_InitLights();
int R_GatherLightSources(const vec3_t vPos, vec3_t* pvLightPos, vec3_t* pvLightIntensity, int iMaxLights);
void R_ClearRealDlights();
void R_UploadDlights();


/*
============================================================

SHADOWS

============================================================
*/

void RB_ShadowTessEnd( void );
void RB_ComputeShadowVolume();
void RB_ShadowFinish( void );
void RB_ProjectionShadowDeform( void );

/*
============================================================

SKIES

============================================================
*/

void R_BuildCloudData( shaderCommands_t *shader );
void R_InitSkyTexCoords( float cloudLayerHeight );
void R_DrawSkyBox( shaderCommands_t *shader );
void RB_DrawSun( void );
void RB_ClipSkyPolygons( shaderCommands_t *shader );

/*
============================================================

CURVE TESSELATION

============================================================
*/

#define PATCH_STITCHING

srfGridMesh_t *R_SubdividePatchToGrid( int width, int height,
								drawVert_t points[MAX_PATCH_SIZE*MAX_PATCH_SIZE] );
srfGridMesh_t *R_GridInsertColumn( srfGridMesh_t *grid, int column, int row, vec3_t point, float loderror );
srfGridMesh_t *R_GridInsertRow( srfGridMesh_t *grid, int row, int column, vec3_t point, float loderror );
void R_FreeSurfaceGridMesh( srfGridMesh_t *grid );

/*
============================================================

MARKERS, POLYGON PROJECTION ON WORLD POLYGONS

============================================================
*/

int R_MarkFragments( int numPoints, const vec3_t *points, const vec3_t projection,
				   int maxPoints, vec3_t pointBuffer, int maxFragments, markFragment_t *fragmentBuffer, float fRadiusSquared);

int R_MarkFragmentsForInlineModel(clipHandle_t bmodel, const vec3_t vAngles, const vec3_t vOrigin, int numPoints,
	const vec3_t* points, const vec3_t projection, int maxPoints, vec3_t pointBuffer,
	int maxFragments, markFragment_t* fragmentBuffer, float fRadiusSquared);


/*
============================================================

SCENE GENERATION

============================================================
*/

void R_ToggleSmpFrame( void );

void RE_ClearScene( void );
void RE_AddRefEntityToScene( const refEntity_t *ent, int parentEntityNumber);
void RE_AddRefSpriteToScene(const refEntity_t* ent);
void RE_AddTerrainMarkToScene(int iTerrainIndex, qhandle_t hShader, int numVerts, const polyVert_t* verts, int renderfx);
refEntity_t* RE_GetRenderEntity(int entityNumber);
qboolean RE_AddPolyToScene(qhandle_t hShader, int numVerts, const polyVert_t* verts, int renderfx);
void RE_AddLightToScene( const vec3_t org, float intensity, float r, float g, float b, int type );
void RE_AddAdditiveLightToScene( const vec3_t org, float intensity, float r, float g, float b );
void RE_RenderScene( const refdef_t *fd );

/*
=============================================================

ANIMATED MODELS

=============================================================
*/

void R_MakeAnimModel( model_t *model );
void R_AddAnimSurfaces(trRefEntity_t* ent);

/*
=============================================================

FONT

=============================================================
*/
fontheader_t* R_LoadFont(const char* name);
void R_LoadFontShader(fontheader_sgl_t* font);
void R_DrawString(fontheader_t* font, const char* text, float x, float y, int maxlen, qboolean bVirtualScreen);
void R_DrawFloatingString(fontheader_t* font, const char* text, const vec3_t org, const vec4_t color, float scale, int maxlen);
float R_GetFontHeight(const fontheader_t* font);
float R_GetFontStringWidth(const fontheader_t* font, const char* s);

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

/*
=============================================================

SKY PORTALS

=============================================================
*/
void R_Sky_Init();
void R_Sky_Reset();
void R_Sky_AddSurf(msurface_t* surf);
void R_Sky_Render();

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
void R_AddSwipeSurfaces();

/*
=============================================================

TERRAIN

=============================================================
*/
void R_MarkTerrainPatch(cTerraPatchUnpacked_t* pPatch);
void R_AddTerrainSurfaces();
void R_AddTerrainMarkSurfaces();
void R_InitTerrain();
void R_TerrainPrepareFrame();
qboolean R_TerrainHeightForPoly(cTerraPatchUnpacked_t* pPatch, polyVert_t* pVerts, int nVerts);

/*
=============================================================

TIKI

=============================================================
*/
struct skelHeaderGame_s;
struct skelAnimFrame_s;

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
void R_CountTikiLodTris(dtiki_t* tiki, float lodpercentage, int* render_tris, int* total_tris);
float R_CalcLod(const vec3_t origin, float radius);
int GetLodCutoff(struct skelHeaderGame_s* skelmodel, float lod_val, int renderfx);
int GetToolLodCutoff(struct skelHeaderGame_s* skelmodel, float lod_val);
void R_PrintInfoWorldtris(void);

extern int g_nStaticSurfaces;
extern qboolean g_bInfostaticmodels;
extern qboolean g_bInfoworldtris;

/*
=============================================================

UTIL

=============================================================
*/
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
void R_DebugRotatedBBox(const vec3_t org, const vec3_t ang, const vec3_t mins, const vec3_t maxs, float r, float g, float b, float alpha);
int RE_GetShaderWidth(qhandle_t hShader);
int RE_GetShaderHeight(qhandle_t hShader);
const char* RE_GetShaderName(qhandle_t hShader);
const char* RE_GetModelName(qhandle_t hModel);

/*
=============================================================
=============================================================
*/
void	R_TransformModelToClip( const vec3_t src, const float *modelMatrix, const float *projectionMatrix,
							vec4_t eye, vec4_t dst );
void	R_TransformClipToWindow( const vec4_t clip, const viewParms_t *view, vec4_t normalized, vec4_t window );

void	RB_DeformTessGeometry( void );

void	RB_CalcEnvironmentTexCoords( float *dstTexCoords );
void	RB_CalcEnvironmentTexCoords2( float *dstTexCoords );
void	RB_CalcScrollTexCoords( const float scroll[2], float *dstTexCoords );
void	RB_CalcRotateTexCoords( float rotSpeed, float *dstTexCoords );
void	RB_CalcScaleTexCoords( const float scale[2], float *dstTexCoords );
void	RB_CalcTurbulentTexCoords( const waveForm_t *wf, float *dstTexCoords );
void	RB_CalcTransformTexCoords( const texModInfo_t *tmi, float *dstTexCoords );
void	RB_CalcWaveAlpha( const waveForm_t *wf, unsigned char *dstColors );
void	RB_CalcWaveColor(const waveForm_t* wf, unsigned char* dstColors, unsigned char* constantColor);
void	RB_CalcAlphaFromEntity( unsigned char *dstColors );
void	RB_CalcAlphaFromOneMinusEntity( unsigned char *dstColors );
void	RB_CalcStretchTexCoords( const waveForm_t *wf, float *texCoords );
void	RB_CalcColorFromEntity( unsigned char *dstColors );
void	RB_CalcColorFromOneMinusEntity( unsigned char *dstColors );
void	RB_CalcColorFromConstant(unsigned char* dstColors, unsigned char* constantColor);
void	RB_CalcRGBFromDot(unsigned char* colors, float alphaMin, float alphaMax);
void	RB_CalcRGBFromOneMinusDot(unsigned char* colors, float alphaMin, float alphaMax);
void	RB_CalcAlphaFromConstant(unsigned char* dstColors, int constantAlpha);
void	RB_CalcAlphaFromDot(unsigned char* colors, float alphaMin, float alphaMax);
void	RB_CalcAlphaFromHeightFade(unsigned char* colors, float alphaMin, float alphaMax);
void	RB_CalcAlphaFromOneMinusDot(unsigned char* colors, float alphaMin, float alphaMax);
void	RB_CalcAlphaFromTexCoords(unsigned char* colors, float alphaMin, float alphaMax, int alphaMinCap, int alphaCap, float sWeight, float tWeight, float* st);
void	RB_CalcRGBFromTexCoords(unsigned char* colors, float alphaMin, float alphaMax, int alphaMinCap, int alphaCap, float sWeight, float tWeight, float* st);
void	RB_CalcSpecularAlpha(unsigned char* alphas, float alphaMax, vec3_t lightOrigin);
void	RB_CalcLightGridColor(unsigned char* colors);
void	RB_CalcAlphaFromDotView(unsigned char* colors, float alphaMin, float alphaMax);
void	RB_CalcAlphaFromOneMinusDotView(unsigned char* colors, float alphaMin, float alphaMax);
void	RB_CalcDiffuseColor( unsigned char *colors );

/*
=============================================================

RENDERER BACK END FUNCTIONS

=============================================================
*/

void RB_RenderThread( void );
void RB_ExecuteRenderCommands( const void *data );

/*
=============================================================

RENDERER BACK END COMMAND QUEUE

=============================================================
*/

#define	MAX_RENDER_COMMANDS	0x40000

typedef struct suninfo_s {
	vec3_t color;
	vec3_t direction;
	vec3_t flaredirection;
	char szFlareName[64];
	qboolean exists;
} suninfo_t;

typedef struct {
	byte	cmds[MAX_RENDER_COMMANDS];
	int		used;
} renderCommandList_t;

typedef struct {
	int		commandId;
	float	color[4];
} setColorCommand_t;

typedef struct {
	int		commandId;
	int		buffer;
} drawBufferCommand_t;

typedef struct {
	int		commandId;
	image_t	*image;
	int		width;
	int		height;
	void	*data;
} subImageCommand_t;

typedef struct {
	int		commandId;
} swapBuffersCommand_t;

typedef struct {
	int		commandId;
	int		buffer;
} endFrameCommand_t;

typedef struct {
	int		commandId;
	shader_t	*shader;
	float	x, y;
	float	w, h;
	float	s1, t1;
	float	s2, t2;
} stretchPicCommand_t;

typedef struct {
	int		commandId;
	trRefdef_t	refdef;
	viewParms_t	viewParms;
	drawSurf_t *drawSurfs;
	int		numDrawSurfs;
} drawSurfsCommand_t;

typedef struct {
	int commandId;
	int x;
	int y;
	int width;
	int height;
	char *fileName;
	qboolean jpeg;
} screenshotCommand_t;

typedef enum {
	RC_END_OF_LIST,
	RC_SET_COLOR,
	RC_STRETCH_PIC,
	RC_DRAW_SURFS,
	RC_SPRITE_SURFS,
	RC_DRAW_BUFFER,
	RC_SWAP_BUFFERS,
	RC_SCREENSHOT
} renderCommand_t;


// these are sort of arbitrary limits.
// the limits apply to the sum of all scenes in a frame --
// the main view, all the 3D icons, etc
#define	MAX_POLYS		4096
#define	MAX_POLYVERTS	16384
#define	MAX_TERMARKS	1024

// all of the information needed by the back end must be
// contained in a backEndData_t.  This entire structure is
// duplicated so the front and back end can run in parallel
// on an SMP machine
typedef struct {
	drawSurf_t	drawSurfs[MAX_DRAWSURFS];
	drawSurf_t  spriteSurfs[MAX_SPRITESURFS];
	dlight_t	dlights[MAX_DLIGHTS];
	trRefEntity_t	entities[MAX_ENTITIES];
	srfMarkFragment_t* terMarks;
	srfPoly_t	*polys;
	polyVert_t	*polyVerts;
	refSprite_t sprites[2048];
	cStaticModelUnpacked_t* staticModels;
	byte* staticModelData;
	renderCommandList_t	commands;
} backEndData_t;

extern	int		max_polys;
extern	int		max_polyverts;

extern	backEndData_t	*backEndData[SMP_FRAMES];	// the second one may not be allocated

extern	volatile renderCommandList_t	*renderCommandList;

extern	volatile qboolean	renderThreadActive;


void *R_GetCommandBuffer( int bytes );
void R_AddDrawSurfCmd( drawSurf_t *drawSurfs, int numDrawSurfs );
void R_AddSpriteSurfCmd( drawSurf_t* drawSurfs, int numDrawSurfs );
void RB_ExecuteRenderCommands( const void *data );

void R_SavePerformanceCounters(void);
void R_InitCommandBuffers( void );
void R_ShutdownCommandBuffers( void );

void R_SyncRenderThread( void );

void R_AddDrawSurfCmd( drawSurf_t *drawSurfs, int numDrawSurfs );

void RE_SetColor( const float *rgba );
void RE_StretchPic ( float x, float y, float w, float h, 
					  float s1, float t1, float s2, float t2, qhandle_t hShader );
void RE_BeginFrame( stereoFrame_t stereoFrame );
void RE_EndFrame( int *frontEndMsec, int *backEndMsec );
void SaveJPG(char * filename, int quality, int image_width, int image_height, unsigned char *image_buffer);

void UI_LoadResource(const char* name);

#ifdef __cplusplus
}
#endif
