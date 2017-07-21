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
#ifndef __QFILES_H__
#define __QFILES_H__

//
// qfiles.h: quake file formats
// This file must be identical in the quake and utils directories
//

//Ignore __attribute__ on non-gcc platforms
#ifndef __GNUC__
#ifndef __attribute__
#define __attribute__(x)
#endif
#endif

// surface geometry should not exceed these limits
#define	SHADER_MAX_VERTEXES		2048 // 1000 su44: I've increased it a bit...
#define	SHADER_MAX_INDEXES		(6*SHADER_MAX_VERTEXES)
#define	SHADER_MAX_TRIANGLES	(2*SHADER_MAX_VERTEXES)

// the maximum size of game relative pathnames
#define	MAX_QPATH		256

/*
========================================================================

QVM files

========================================================================
*/

#define	VM_MAGIC			0x12721444
#define	VM_MAGIC_VER2	0x12721445
typedef struct {
	int		vmMagic;

	int		instructionCount;

	int		codeOffset;
	int		codeLength;

	int		dataOffset;
	int		dataLength;
	int		litLength;			// ( dataLength - litLength ) should be byteswapped on load
	int		bssLength;			// zero filled memory appended to datalength

	//!!! below here is VM_MAGIC_VER2 !!!
	int		jtrgLength;			// number of jump table targets
} vmHeader_t;


/*
========================================================================

PCX files are used for 8 bit images

========================================================================
*/

typedef struct {
    char	manufacturer;
    char	version;
    char	encoding;
    char	bits_per_pixel;
    unsigned short	xmin,ymin,xmax,ymax;
    unsigned short	hres,vres;
    unsigned char	palette[48];
    char	reserved;
    char	color_planes;
    unsigned short	bytes_per_line;
    unsigned short	palette_type;
    char	filler[58];
    unsigned char	data[1];			// unbounded
} pcx_t;


/*
========================================================================

TGA files are used for 24/32 bit images

========================================================================
*/

typedef struct _TargaHeader {
	unsigned char 	id_length, colormap_type, image_type;
	unsigned short	colormap_index, colormap_length;
	unsigned char	colormap_size;
	unsigned short	x_origin, y_origin, width, height;
	unsigned char	pixel_size, attributes;
} TargaHeader;

/*
========================================================================

FTX files are pre mipmapped files for OpenMOHAA

========================================================================
*/
#define FENCEMASK_VERSION			2
#define FTX_EXTENSION				".ftx"

typedef struct ftx_s {
	int width;
	int height;
	int has_alpha;
	// data follows
} ftx_t;

/*
========================================================================

.MD3 triangle model file format

========================================================================
*/

#define MD3_IDENT			(('3'<<24)+('P'<<16)+('D'<<8)+'I')
#define MD3_VERSION			15

// limits
#define MD3_MAX_LODS		3
#define	MD3_MAX_TRIANGLES	8192	// per surface
#define MD3_MAX_VERTS		4096	// per surface
#define MD3_MAX_SHADERS		256		// per surface
#define MD3_MAX_FRAMES		1024	// per model
#define	MD3_MAX_SURFACES	32		// per model
#define MD3_MAX_TAGS		16		// per frame

// vertex scales
#define	MD3_XYZ_SCALE		(1.0/64)

#if 0
typedef struct md3Frame_s {
	vec3_t		bounds[2];
	vec3_t		localOrigin;
	float		radius;
	char		name[16];
} md3Frame_t;

typedef struct md3Tag_s {
	char		name[MAX_QPATH];	// tag name
	vec3_t		origin;
	vec3_t		axis[3];
} md3Tag_t;

/*
** md3Surface_t
**
** CHUNK			SIZE
** header			sizeof( md3Surface_t )
** shaders			sizeof( md3Shader_t ) * numShaders
** triangles[0]		sizeof( md3Triangle_t ) * numTriangles
** st				sizeof( md3St_t ) * numVerts
** XyzNormals		sizeof( md3XyzNormal_t ) * numVerts * numFrames
*/
typedef struct {
	int		ident;				//

	char	name[MAX_QPATH];	// polyset name

	int		flags;
	int		numFrames;			// all surfaces in a model should have the same

	int		numShaders;			// all surfaces in a model should have the same
	int		numVerts;

	int		numTriangles;
	int		ofsTriangles;

	int		ofsShaders;			// offset from start of md3Surface_t
	int		ofsSt;				// texture coords are common for all frames
	int		ofsXyzNormals;		// numVerts * numFrames

	int		ofsEnd;				// next surface follows
} md3Surface_t;

typedef struct {
	char			name[MAX_QPATH];
	int				shaderIndex;	// for in-game use
} md3Shader_t;

typedef struct {
	int			indexes[3];
} md3Triangle_t;

typedef struct {
	float		st[2];
} md3St_t;

typedef struct {
	short		xyz[3];
	short		normal;
} md3XyzNormal_t;

typedef struct {
	int			ident;
	int			version;

	char		name[MAX_QPATH];	// model name

	int			flags;

	int			numFrames;
	int			numTags;
	int			numSurfaces;

	int			numSkins;

	int			ofsFrames;			// offset for first frame
	int			ofsTags;			// numFrames * numTags
	int			ofsSurfaces;		// first surface, others follow

	int			ofsEnd;				// end of file
} md3Header_t;

/*
==============================================================================

MD4 file format

==============================================================================
*/

#define MD4_IDENT			(('4'<<24)+('P'<<16)+('D'<<8)+'I')
#define MD4_VERSION			1
#define	MD4_MAX_BONES		128

typedef struct {
	int			boneIndex;		// these are indexes into the boneReferences,
	float		   boneWeight;		// not the global per-frame bone list
	vec3_t		offset;
} md4Weight_t;

typedef struct {
	vec3_t		normal;
	vec2_t		texCoords;
	int			numWeights;
	md4Weight_t	weights[1];		// variable sized
} md4Vertex_t;

typedef struct {
	int			indexes[3];
} md4Triangle_t;

typedef struct {
	int			ident;

	char		name[MAX_QPATH];	// polyset name
	char		shader[MAX_QPATH];
	int			shaderIndex;		// for in-game use

	int			ofsHeader;			// this will be a negative number

	int			numVerts;
	int			ofsVerts;

	int			numTriangles;
	int			ofsTriangles;

	// Bone references are a set of ints representing all the bones
	// present in any vertex weights for this surface.  This is
	// needed because a model may have surfaces that need to be
	// drawn at different sort times, and we don't want to have
	// to re-interpolate all the bones for each surface.
	int			numBoneReferences;
	int			ofsBoneReferences;

	int			ofsEnd;				// next surface follows
} md4Surface_t;

typedef struct {
	float		matrix[3][4];
} md4Bone_t;

typedef struct {
	vec3_t		bounds[2];			// bounds of all surfaces of all LOD's for this frame
	vec3_t		localOrigin;		// midpoint of bounds, used for sphere cull
	float		radius;				// dist from localOrigin to corner
	md4Bone_t	bones[1];			// [numBones]
} md4Frame_t;

typedef struct {
	int			numSurfaces;
	int			ofsSurfaces;		// first surface, others follow
	int			ofsEnd;				// next lod follows
} md4LOD_t;

typedef struct {
	int			ident;
	int			version;

	char		name[MAX_QPATH];	// model name

	// frames and bones are shared by all levels of detail
	int			numFrames;
	int			numBones;
	int			ofsBoneNames;		// char	name[ MAX_QPATH ]
	int			ofsFrames;			// md4Frame_t[numFrames]

	// each level of detail has completely separate sets of surfaces
	int			numLODs;
	int			ofsLODs;

	int			ofsEnd;				// end of file
} md4Header_t;

/*
 * Here are the definitions for Ravensoft's model format of md4. Raven stores their
 * playermodels in .mdr files, in some games, which are pretty much like the md4
 * format implemented by ID soft. It seems like ID's original md4 stuff is not used at all.
 * MDR is being used in EliteForce, JediKnight2 and Soldiers of Fortune2 (I think).
 * So this comes in handy for anyone who wants to make it possible to load player
 * models from these games.
 * This format has bone tags, which is similar to the thing you have in md3 I suppose.
 * Raven has released their version of md3view under GPL enabling me to add support
 * to this codebase. Thanks to Steven Howes aka Skinner for helping with example
 * source code.
 *
 * - Thilo Schulz (arny@ats.s.bawue.de)
 */

// If you want to enable support for Raven's .mdr / md4 format, uncomment the next
// line.
//#define RAVENMD4

#ifdef RAVENMD4

#define MDR_IDENT	(('5'<<24)+('M'<<16)+('D'<<8)+'R')
#define MDR_VERSION	2
#define	MDR_MAX_BONES	128

typedef struct {
	int			boneIndex;	// these are indexes into the boneReferences,
	float		   boneWeight;		// not the global per-frame bone list
	vec3_t		offset;
} mdrWeight_t;

typedef struct {
	vec3_t		normal;
	vec2_t		texCoords;
	int			numWeights;
	mdrWeight_t	weights[1];		// variable sized
} mdrVertex_t;

typedef struct {
	int			indexes[3];
} mdrTriangle_t;

typedef struct {
	int			ident;

	char		name[MAX_QPATH];	// polyset name
	char		shader[MAX_QPATH];
	int			shaderIndex;	// for in-game use

	int			ofsHeader;	// this will be a negative number

	int			numVerts;
	int			ofsVerts;

	int			numTriangles;
	int			ofsTriangles;

	// Bone references are a set of ints representing all the bones
	// present in any vertex weights for this surface.  This is
	// needed because a model may have surfaces that need to be
	// drawn at different sort times, and we don't want to have
	// to re-interpolate all the bones for each surface.
	int			numBoneReferences;
	int			ofsBoneReferences;

	int			ofsEnd;		// next surface follows
} mdrSurface_t;

typedef struct {
	float		matrix[3][4];
} mdrBone_t;

typedef struct {
	vec3_t		bounds[2];		// bounds of all surfaces of all LOD's for this frame
	vec3_t		localOrigin;		// midpoint of bounds, used for sphere cull
	float		radius;			// dist from localOrigin to corner
	char		name[16];
	mdrBone_t	bones[1];		// [numBones]
} mdrFrame_t;

typedef struct {
        unsigned char Comp[24]; // MC_COMP_BYTES is in MatComp.h, but don't want to couple
} mdrCompBone_t;

typedef struct {
        vec3_t          bounds[2];		// bounds of all surfaces of all LOD's for this frame
        vec3_t          localOrigin;		// midpoint of bounds, used for sphere cull
        float           radius;			// dist from localOrigin to corner
        mdrCompBone_t   bones[1];		// [numBones]
} mdrCompFrame_t;

typedef struct {
	int			numSurfaces;
	int			ofsSurfaces;		// first surface, others follow
	int			ofsEnd;				// next lod follows
} mdrLOD_t;

typedef struct {
        int                     boneIndex;
        char            name[32];
} mdrTag_t;

typedef struct {
	int			ident;
	int			version;

	char		name[MAX_QPATH];	// model name

	// frames and bones are shared by all levels of detail
	int			numFrames;
	int			numBones;
	int			ofsFrames;			// mdrFrame_t[numFrames]

	// each level of detail has completely separate sets of surfaces
	int			numLODs;
	int			ofsLODs;

        int                     numTags;
        int                     ofsTags;

	int			ofsEnd;				// end of file
} mdrHeader_t;

#endif

#endif

/*
==============================================================================

  .BSP file format

==============================================================================
*/


#define BSP_IDENT	(('5'<<24)+('1'<<16)+('0'<<8)+'2')
		// little-endian "2015"

#define BSP_BETA_VERSION	18	// Beta Allied Assault
#define BSP_BASE_VERSION	19	// vanilla Allied Assault
#define BSP_VERSION			19	// current Allied Assault


// there shouldn't be any problem with increasing these values at the
// expense of more memory allocation in the utilities
#define	MAX_MAP_MODELS			0x400
#define	MAX_MAP_BRUSHES			0x8000
#define	MAX_MAP_ENTITIES		0x2000		// 0x800 // su44: increased for t2l1.map
#define	MAX_MAP_ENTSTRING		0x100000	//0x40000 // su44: increased for t2l1.map
#define	MAX_MAP_SHADERS			0x400

#define	MAX_MAP_AREAS			0x100		// MAX_MAP_AREA_BYTES in q_shared must match!
#define	MAX_MAP_FOGS			0x100
#define	MAX_MAP_PLANES			0x20000
#define	MAX_MAP_NODES			0x20000
#define	MAX_MAP_BRUSHSIDES		0x20000
#define	MAX_MAP_LEAFS			0x20000
#define	MAX_MAP_LEAFFACES		0x20000
#define	MAX_MAP_LEAFBRUSHES		0x40000
#define	MAX_MAP_PORTALS			0x20000
#define	MAX_MAP_LIGHTING		0x800000
#define	MAX_MAP_LIGHTGRID		0x800000
#define	MAX_MAP_VISIBILITY		0x200000

#define	MAX_MAP_DRAW_SURFS		0x20000
#define	MAX_MAP_DRAW_VERTS		0x80000
#define	MAX_MAP_DRAW_INDEXES	0x80000
#define MAX_MAP_SPHERE_L_SIZE	1532


#define MIN_MAP_SUBDIVISIONS	16


// key / value pair sizes in the entities lump
#define	MAX_KEY				32
#define	MAX_VALUE			1024

// the editor uses these predefined yaw angles to orient entities up or down
#define	ANGLE_UP			-1
#define	ANGLE_DOWN			-2

#define	LIGHTMAP_WIDTH		128
#define	LIGHTMAP_HEIGHT		128
#define	LIGHTMAP_SIZE		128	// IneQuation: moved it here, MUST MATCH THE FORMER TWO

#define MAX_WORLD_COORD		( 128*128 )
#define MIN_WORLD_COORD		( -128*128 )
#define WORLD_SIZE			( MAX_WORLD_COORD - MIN_WORLD_COORD )

//=============================================================================

typedef struct fcm_s {
	int iWidth;
	int iHeight;
} fcm_t;

typedef struct varnode_s {
	unsigned short flags;
} varnode_t;

typedef struct cTerraPatch_s {
	byte	flags;
	byte	lmapScale;
	byte	s;
	byte	t;

	float	texCoord[ 2 ][ 2 ][ 2 ];

	char	x;
	char	y;

	short			iBaseHeight;
	unsigned short	iShader;
	unsigned short	iLightMap;

	short	iNorth;
	short	iEast;
	short	iSouth;
	short	iWest;

	varnode_t		varTree[ 2 ][ 63 ];

	unsigned char	heightmap[ 9 * 9 ];
} cTerraPatch_t;

typedef struct cStaticModel_s {
	char model[ 128 ];
	float origin[ 3 ];
	float angles[ 3 ];
	float scale;
	int firstVertexData;
	int numVertexData;
} cStaticModel_t;

typedef struct {
	void	*buffer;
	int		length;
} gamelump_t;

typedef struct {
	int		fileofs, filelen;
} lump_t;


#define LUMP_FOGS				0
#define	LUMP_LIGHTGRID			0
// new lump defines. lump numbers are different in mohaa
#define LUMP_SHADERS			0
#define LUMP_PLANES				1
#define LUMP_LIGHTMAPS			2
#define LUMP_SURFACES			3
#define LUMP_DRAWVERTS			4
#define LUMP_DRAWINDEXES		5
#define LUMP_LEAFBRUSHES		6
#define LUMP_LEAFSURFACES		7
#define LUMP_LEAFS				8
#define LUMP_NODES				9
#define LUMP_SIDEEQUATIONS		10
#define LUMP_BRUSHSIDES			11
#define LUMP_BRUSHES			12
/*
// FOG seems to be handled differently in MOHAA - no fog lump found yet
#define LUMP_FOGS				0
*/
#define LUMP_MODELS				13
#define LUMP_ENTITIES			14
#define LUMP_VISIBILITY			15
#define LUMP_LIGHTGRIDPALETTE	16
#define LUMP_LIGHTGRIDOFFSETS	17
#define LUMP_LIGHTGRIDDATA		18
#define LUMP_SPHERELIGHTS		19
#define LUMP_SPHERELIGHTVIS		20
#define LUMP_LIGHTDEFS			21
#define LUMP_TERRAIN			22
#define LUMP_TERRAININDEXES		23
#define LUMP_STATICMODELDATA	24
#define LUMP_STATICMODELDEF		25
#define LUMP_STATICMODELINDEXES	26
#define LUMP_DUMMY10			27

#define	HEADER_LUMPS		28

typedef struct {
	int			ident;
	int			version;
	int			checksum;

	lump_t		lumps[ HEADER_LUMPS ];
} dheader_t;

typedef struct {
	float		mins[3], maxs[3];
	int			firstSurface, numSurfaces;
	int			firstBrush, numBrushes;
} dmodel_t;

typedef struct {
	char shader[ 64 ];
	int surfaceFlags;
	int contentFlags;
	int subdivisions;
	char fenceMaskImage[ 64 ];
} dshader_t;

typedef struct baseshader_s {
	char shader[ 64 ];
	int surfaceFlags;
	int contentFlags;
} baseshader_t;

// planes x^1 is allways the opposite of plane x

typedef struct {
	float		normal[3];
	float		dist;
} dplane_t;

typedef struct {
	int			planeNum;
	int			children[2];	// negative numbers are -(leafs+1), not nodes
	int			mins[3];		// for frustom culling
	int			maxs[3];
} dnode_t;

typedef struct {
	int			cluster;			// -1 = opaque cluster (do I still store these?)
	int			area;

	int			mins[3];			// for frustum culling
	int			maxs[3];

	int			firstLeafSurface;
	int			numLeafSurfaces;

	int			firstLeafBrush;
	int			numLeafBrushes;

	//added for mohaa
	int			firstTerraPatch;
	int			numTerraPatches;
	int			firstStaticModel;
	int			numStaticModels;
} dleaf_t;

// old leaf version
typedef struct {
	int		cluster;
	int		area;

	int		mins[ 3 ];
	int		maxs[ 3 ];

	int		firstLeafSurface;
	int		numLeafSurfaces;

	int		firstLeafBrush;
	int		numLeafBrushes;

	int		firstTerraPatch;
	int		numTerraPatches;
} dleaf_t_ver17;

// su44: It seems that sideEquations 
// are somehow related to fencemasks...
// MoHAA loads them only in CM (CM_LoadMap).
typedef struct {
	float fSeq[ 4 ];
	float fTeq[ 4 ];
} dsideequation_t;

typedef struct {
	int			planeNum;			// positive plane side faces out of the leaf
	int			shaderNum;

	//added for mohaa
	int			equationNum; // su44: that's a dsideEquation_t index
} dbrushside_t;

typedef struct {
	int			firstSide;
	int			numSides;
	int			shaderNum;		// the shader that determines the contents flags
} dbrush_t;

typedef struct {
	char		shader[MAX_QPATH];
	int			brushNum;
	int			visibleSide;	// the brush side that ray tests need to clip against (-1 == none)
} dfog_t;

typedef struct {
	vec3_t		xyz;
	float		st[2];
	float		lightmap[2];
	vec3_t		normal;
	byte		color[4];
} drawVert_t;

typedef struct {
	vec3_t		xyz;
	float		st[ 2 ];
	int			collapseMap;
	float		lodExtra; // depending on the vertexNumber, will be 0 - minLOD, 1 - lodScale or 2 - lodBias
	vec3_t		normal;
	byte		color[ 4 ];
} drawSoupVert_t;

#define drawVert_t_cleared(x) drawVert_t (x) = {{0, 0, 0}, {0, 0}, {0, 0}, {0, 0, 0}, {0, 0, 0, 0}}

typedef enum {
	MST_BAD,
	MST_PLANAR,
	MST_PATCH,
	MST_TRIANGLE_SOUP,
	MST_FLARE,
	MST_TERRAIN
} mapSurfaceType_t;

typedef struct {
	int			shaderNum;
	int			fogNum;
	int			surfaceType;

	int			firstVert;
	int			numVerts;

	int			firstIndex;
	int			numIndexes;

	int			lightmapNum;
	int			lightmapX, lightmapY;
	int			lightmapWidth, lightmapHeight;

	vec3_t		lightmapOrigin;
	vec3_t		lightmapVecs[ 3 ];	// for patches, [0] and [1] are lodbounds

	int			patchWidth;
	int			patchHeight;

	//added for mohaa
	float		subdivisions;
} dsurface_t;

// IneQuation was here
typedef struct dterPatch_s {
	byte			flags;
	byte			scale;
	byte			lmCoords[2];
	float			texCoords[8];
	char			x;
	char			y;
	short			baseZ;
	unsigned short	shader;
	short			lightmap;
	short			dummy[4];
	short			vertFlags[2][63];
	byte			heightmap[9][9];
} dterPatch_t;

// su44 was here
typedef struct dstaticModel_s {
	char model[128];
	vec3_t origin;
	vec3_t angles;
	float scale;
	int firstVertexData;
	short numVertexData;
} dstaticModel_t;

typedef struct {
	float origin[3];
	float color[3];
	float intensity;
	int leaf;
	qboolean needs_trace;
	qboolean spot_light;
	float spot_dir[3];
	float spot_radiusbydistance;
} dspherel_t;

typedef struct {
	float origin[3];
	float axis[3];
	int bounds[3];
} dlightGrid_t;

typedef struct {
	int lightIntensity;
	int lightAngle;
	int lightmapResolution;
	qboolean twoSided;
	qboolean lightLinear;
	vec3_t lightColor;
	float lightFalloff;
	float backsplashFraction;
	float backsplashDistance;
	float lightSubdivide;
	qboolean autosprite;
} dlightdef_t;

typedef struct {
	vec3_t origin;
	vec3_t color;
	float intensity;
	int leaf;
	qboolean needs_trace;
	qboolean spot_light;
	vec3_t spot_dir;
	float spot_radiusbydistance;
} mapspherel_t;

#endif
