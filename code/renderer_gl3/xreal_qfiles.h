#ifndef __XREAL_QFILES_H__
#define __XREAL_QFILES_H__
// su44: things from Xreal's qfiles.h, totally irrelevant to OpenMoHAA.
// Should be removed soon.

// Ridah, mesh compression
/*
==============================================================================

MDC file format

==============================================================================
*/

#define MDC_IDENT           ( ( 'C' << 24 ) + ( 'P' << 16 ) + ( 'D' << 8 ) + 'I' )
#define MDC_VERSION         2

// version history:
// 1 - original
// 2 - changed tag structure so it only lists the names once

typedef struct
{
	unsigned int    ofsVec;		// offset direction from the last base frame
//  unsigned short  ofsVec;
} mdcXyzCompressed_t;

typedef struct
{
	char            name[64];	// tag name
} mdcTagName_t;

#define MDC_TAG_ANGLE_SCALE ( 360.0 / 32700.0 )

typedef struct
{
	short           xyz[3];
	short           angles[3];
} mdcTag_t;

/*
** mdcSurface_t
**
** CHUNK			SIZE
** header			sizeof( md3Surface_t )
** shaders			sizeof( md3Shader_t ) * numShaders
** triangles[0]		sizeof( md3Triangle_t ) * numTriangles
** st				sizeof( md3St_t ) * numVerts
** XyzNormals		sizeof( md3XyzNormal_t ) * numVerts * numBaseFrames
** XyzCompressed	sizeof( mdcXyzCompressed ) * numVerts * numCompFrames
** frameBaseFrames	sizeof( short ) * numFrames
** frameCompFrames	sizeof( short ) * numFrames (-1 if frame is a baseFrame)
*/
typedef struct
{
	int             ident;		//

	char            name[64];	// polyset name

	int             flags;
	int             numCompFrames;	// all surfaces in a model should have the same
	int             numBaseFrames;	// ditto

	int             numShaders;	// all surfaces in a model should have the same
	int             numVerts;

	int             numTriangles;
	int             ofsTriangles;

	int             ofsShaders;	// offset from start of md3Surface_t
	int             ofsSt;		// texture coords are common for all frames
	int             ofsXyzNormals;	// numVerts * numBaseFrames
	int             ofsXyzCompressed;	// numVerts * numCompFrames

	int             ofsFrameBaseFrames;	// numFrames
	int             ofsFrameCompFrames;	// numFrames

	int             ofsEnd;		// next surface follows
} mdcSurface_t;

typedef struct
{
	int             ident;
	int             version;

	char            name[64];	// model name

	int             flags;

	int             numFrames;
	int             numTags;
	int             numSurfaces;

	int             numSkins;

	int             ofsFrames;	// offset for first frame, stores the bounds and localOrigin
	int             ofsTagNames;	// numTags
	int             ofsTags;	// numFrames * numTags
	int             ofsSurfaces;	// first surface, others follow

	int             ofsEnd;		// end of file
} mdcHeader_t;

// done.

/*
==============================================================================

MDS file format (Wolfenstein Skeletal Format)

==============================================================================
*/

#define MDS_IDENT           ( ( 'W' << 24 ) + ( 'S' << 16 ) + ( 'D' << 8 ) + 'M' )
#define MDS_VERSION         4
#define MDS_MAX_VERTS       6000
#define MDS_MAX_TRIANGLES   8192
#define MDS_MAX_BONES       128
#define MDS_MAX_SURFACES    32
#define MDS_MAX_TAGS        128

#define MDS_TRANSLATION_SCALE   ( 1.0 / 64 )

typedef struct
{
	int             boneIndex;	// these are indexes into the boneReferences,
	float           boneWeight;	// not the global per-frame bone list
	vec3_t          offset;
} mdsWeight_t;

typedef struct
{
	vec3_t          normal;
	vec2_t          texCoords;
	int             numWeights;
	int             fixedParent;	// stay equi-distant from this parent
	float           fixedDist;
	mdsWeight_t     weights[1];	// variable sized
} mdsVertex_t;

typedef struct
{
	int             indexes[3];
} mdsTriangle_t;

typedef struct
{
	int             ident;

	char            name[64];	// polyset name
	char            shader[64];
	int             shaderIndex;	// for in-game use

	int             minLod;

	int             ofsHeader;	// this will be a negative number

	int             numVerts;
	int             ofsVerts;

	int             numTriangles;
	int             ofsTriangles;

	int             ofsCollapseMap;	// numVerts * int

	// Bone references are a set of ints representing all the bones
	// present in any vertex weights for this surface.  This is
	// needed because a model may have surfaces that need to be
	// drawn at different sort times, and we don't want to have
	// to re-interpolate all the bones for each surface.
	int             numBoneReferences;
	int             ofsBoneReferences;

	int             ofsEnd;		// next surface follows
} mdsSurface_t;

typedef struct
{
	//float     angles[3];
	//float     ofsAngles[2];
	short           angles[4];	// to be converted to axis at run-time (this is also better for lerping)
	short           ofsAngles[2];	// PITCH/YAW, head in this direction from parent to go to the offset position
} mdsBoneFrameCompressed_t;

// NOTE: this only used at run-time
typedef struct
{
	float           matrix[3][3];	// 3x3 rotation
	vec3_t          translation;	// translation vector
} mdsBoneFrame_t;

typedef struct
{
	vec3_t          bounds[2];	// bounds of all surfaces of all LOD's for this frame
	vec3_t          localOrigin;	// midpoint of bounds, used for sphere cull
	float           radius;		// dist from localOrigin to corner
	vec3_t          parentOffset;	// one bone is an ascendant of all other bones, it starts the hierachy at this position
	mdsBoneFrameCompressed_t bones[1];	// [numBones]
} mdsFrame_t;

typedef struct
{
	int             numSurfaces;
	int             ofsSurfaces;	// first surface, others follow
	int             ofsEnd;		// next lod follows
} mdsLOD_t;

typedef struct
{
	char            name[64];	// name of tag
	float           torsoWeight;
	int             boneIndex;	// our index in the bones
} mdsTag_t;

#define BONEFLAG_TAG        1	// this bone is actually a tag

typedef struct
{
	char            name[64];	// name of bone
	int             parent;		// not sure if this is required, no harm throwing it in
	float           torsoWeight;	// scale torso rotation about torsoParent by this
	float           parentDist;
	int             flags;
} mdsBoneInfo_t;

typedef struct
{
	int             ident;
	int             version;

	char            name[64];	// model name

	float           lodScale;
	float           lodBias;

	// frames and bones are shared by all levels of detail
	int             numFrames;
	int             numBones;
	int             ofsFrames;	// md4Frame_t[numFrames]
	int             ofsBones;	// mdsBoneInfo_t[numBones]
	int             torsoParent;	// index of bone that is the parent of the torso

	int             numSurfaces;
	int             ofsSurfaces;

	// tag data
	int             numTags;
	int             ofsTags;	// mdsTag_t[numTags]

	int             ofsEnd;		// end of file
} mdsHeader_t;

/*
==============================================================================

MDM file format (Wolfenstein Skeletal Mesh)

version history:
	2 - initial version
	3 - removed all frame data, this format is pure mesh and bone references now

==============================================================================
*/

#define MDM_IDENT           ( ( 'W' << 24 ) + ( 'M' << 16 ) + ( 'D' << 8 ) + 'M' )
#define MDM_VERSION         3
#define MDM_MAX_VERTS       6000
#define MDM_MAX_TRIANGLES   8192
#define MDM_MAX_SURFACES    32
#define MDM_MAX_TAGS        128

#define MDM_TRANSLATION_SCALE   ( 1.0 / 64 )

typedef struct
{
	int             boneIndex;	// these are indexes into the boneReferences,
	float           boneWeight;	// not the global per-frame bone list
	vec3_t          offset;
} mdmWeight_t;

typedef struct
{
	vec3_t          normal;
	vec2_t          texCoords;
	int             numWeights;
	mdmWeight_t     weights[1];	// variable sized
} mdmVertex_t;

typedef struct
{
	int             indexes[3];
} mdmTriangle_t;

typedef struct
{
	int             ident;

	char            name[64];	// polyset name
	char            shader[64];
	int             shaderIndex;	// for in-game use

	int             minLod;

	int             ofsHeader;	// this will be a negative number

	int             numVerts;
	int             ofsVerts;

	int             numTriangles;
	int             ofsTriangles;

	int             ofsCollapseMap;	// numVerts * int

	// Bone references are a set of ints representing all the bones
	// present in any vertex weights for this surface.  This is
	// needed because a model may have surfaces that need to be
	// drawn at different sort times, and we don't want to have
	// to re-interpolate all the bones for each surface.
	int             numBoneReferences;
	int             ofsBoneReferences;

	int             ofsEnd;		// next surface follows
} mdmSurface_t;

/*typedef struct {
	vec3_t		bounds[2];			// bounds of all surfaces of all LOD's for this frame
	vec3_t		localOrigin;		// midpoint of bounds, used for sphere cull
	float		radius;				// dist from localOrigin to corner
	vec3_t		parentOffset;		// one bone is an ascendant of all other bones, it starts the hierachy at this position
} mdmFrame_t;*/

typedef struct
{
	int             numSurfaces;
	int             ofsSurfaces;	// first surface, others follow
	int             ofsEnd;		// next lod follows
} mdmLOD_t;

/*typedef struct {
	char		name[64];	// name of tag
	float		torsoWeight;
	int			boneIndex;			// our index in the bones

	int			numBoneReferences;
	int			ofsBoneReferences;

	int			ofsEnd;				// next tag follows
} mdmTag_t;*/

// Tags always only have one parent bone
typedef struct
{
	char            name[64];	// name of tag
	vec3_t          axis[3];

	int             boneIndex;
	vec3_t          offset;

	int             numBoneReferences;
	int             ofsBoneReferences;

	int             ofsEnd;		// next tag follows
} mdmTag_t;

typedef struct
{
	int             ident;
	int             version;

	char            name[64];	// model name
/*	char		bonesfile[64];	// bone file

#ifdef UTILS
	int			skel;
#else
	// dummy in file, set on load to link to MDX
	qhandle_t	skel;
#endif // UTILS
*/
	float           lodScale;
	float           lodBias;

	// frames and bones are shared by all levels of detail
/*	int			numFrames;
	int			ofsFrames;			// mdmFrame_t[numFrames]
*/
	int             numSurfaces;
	int             ofsSurfaces;

	// tag data
	int             numTags;
	int             ofsTags;

	int             ofsEnd;		// end of file
} mdmHeader_t;

/*
==============================================================================

MDX file format (Wolfenstein Skeletal Data)

version history:
	1 - initial version
	2 - moved parentOffset from the mesh to the skeletal data file

==============================================================================
*/

#define MDX_IDENT           ( ( 'W' << 24 ) + ( 'X' << 16 ) + ( 'D' << 8 ) + 'M' )
#define MDX_VERSION         2
#define MDX_MAX_BONES       128

typedef struct
{
	vec3_t          bounds[2];	// bounds of this frame
	vec3_t          localOrigin;	// midpoint of bounds, used for sphere cull
	float           radius;		// dist from localOrigin to corner
	vec3_t          parentOffset;	// one bone is an ascendant of all other bones, it starts the hierachy at this position
} mdxFrame_t;

typedef struct
{
	//float     angles[3];
	//float     ofsAngles[2];
	short           angles[4];	// to be converted to axis at run-time (this is also better for lerping)
	short           ofsAngles[2];	// PITCH/YAW, head in this direction from parent to go to the offset position
} mdxBoneFrameCompressed_t;

// NOTE: this only used at run-time
// FIXME: do we really need this?
typedef struct
{
	float           matrix[3][3];	// 3x3 rotation
	vec3_t          translation;	// translation vector
} mdxBoneFrame_t;

typedef struct
{
	char            name[64];	// name of bone
	int             parent;		// not sure if this is required, no harm throwing it in
	float           torsoWeight;	// scale torso rotation about torsoParent by this
	float           parentDist;
	int             flags;
} mdxBoneInfo_t;

typedef struct
{
	int             ident;
	int             version;

	char            name[64];	// model name

	// bones are shared by all levels of detail
	int             numFrames;
	int             numBones;
	int             ofsFrames;	// (mdxFrame_t + mdxBoneFrameCompressed_t[numBones]) * numframes
	int             ofsBones;	// mdxBoneInfo_t[numBones]
	int             torsoParent;	// index of bone that is the parent of the torso

	int             ofsEnd;		// end of file
} mdxHeader_t;


/*
========================================================================

Actor X - .PSK / .PSA skeletal triangle model file format

========================================================================
*/

#define PSK_IDENTSTRING		"ACTRHEAD"
#define PSK_IDENTLEN		8
#define PSK_VERSION			1


typedef struct
{
	char            ident[20];
	int             flags;

	int				dataSize;	// sizeof(struct)
	int				numData;	// number of structs put into this data chunk
} axChunkHeader_t;

typedef struct
{
	float			point[3];
} axPoint_t;

typedef struct
{
	unsigned short	pointIndex;
	unsigned short	unknownA;
	float			st[2];
	byte			materialIndex;
	byte			reserved;		// we don't care about this one
	unsigned short	unknownB;
} axVertex_t;

typedef struct
{
	unsigned short	indexes[3];
	byte			materialIndex;
	byte			materialIndex2;
	unsigned int	smoothingGroups;
} axTriangle_t;

typedef struct
{
	char            name[64];
	int             shaderIndex;	// for in-game use
	unsigned int	polyFlags;
	int				auxMaterial;
	unsigned int	auxFlags;
	int				lodBias;
	int				lodStyle;
} axMaterial_t;

typedef struct
{
	float			quat[4];		// x y z w
	float			position[3];	// x y z

	float			length;
	float			xSize;
	float			ySize;
	float			zSize;
} axBone_t;

typedef struct
{
	char			name[64];
	unsigned int	flags;
	int				numChildren;
	int				parentIndex;
	axBone_t		bone;
} axReferenceBone_t;

typedef struct
{
	float			weight;
	unsigned int	pointIndex;
	unsigned int	boneIndex;
} axBoneWeight_t;

typedef struct
{
	char			name[64];
	char			group[64];

	int				numBones;		// same as numChannels
	int				rootInclude;

	int				keyCompressionStyle;
	int				keyQuotum;
	float			keyReduction;

	float			trackTime;

	float			frameRate;

	int				startBoneIndex;

	int				firstRawFrame;
	int				numRawFrames;
} axAnimationInfo_t;

typedef struct
{
	float			position[3];
	float			quat[4];
	float			time;
} axAnimationKey_t;

#endif // __XREAL_QFILES_H__