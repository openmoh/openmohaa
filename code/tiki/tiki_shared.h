/*
===========================================================================
Copyright (C) 2015 the OpenMoHAA team

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

// tiki_shared.h : Skeletor

#ifndef __TIKI_SHARED_H__
#define __TIKI_SHARED_H__

#ifdef __cplusplus
#include "../skeletor/skeletor_name_lists.h"
#endif

// animation flags
#define TAF_RANDOM					0x1		// random
#define TAF_NOREPEAT				0x2		// no loop
#define TAF_DONTREPEAT				( TAF_RANDOM | TAF_NOREPEAT )	// don't repeat
#define TAF_AUTOSTEPS_DOG			0x4		// footsteps for dogs
#define TAF_DEFAULT_ANGLES			0x8		// ?
#define TAF_NOTIMECHECK				0x10	// don't check for timing
#define TAF_AUTOSTEPS_WALK			( TAF_NOTIMECHECK | TAF_AUTOSTEPS_DOG )	// walk
#define TAF_AUTOSTEPS_RUN			( TAF_NOTIMECHECK | TAF_AUTOSTEPS_DOG | TAF_DEFAULT_ANGLES )	// run
#define TAF_DELTADRIVEN				0x20	// animation is based on skeletor animation's delta

// skeletor animation flags
#define TAF_HASDELTA			0x40
#define TAF_HASMORPH			0x80

// tiki anim command frames
#define TIKI_FRAME_LAST				-5			// exec at last
#define TIKI_FRAME_END				-4			// exec at end
#define TIKI_FRAME_ENTRY			-3			// exec at entry
#define TIKI_FRAME_EXIT				-2			// exec at exit
#define TIKI_FRAME_EVERY			-1			// exec each frame
#define TIKI_FRAME_FIRST			0			// exec at first frame
#define TIKI_FRAME_MAXFRAMERATE		60

// skb skeletor
#define TIKI_SKB_HEADER_IDENT		( *( int * )"SKL " )
#define TIKI_SKB_HEADER_VER_3		3
#define TIKI_SKB_HEADER_VERSION		4

// skd skeletor
#define TIKI_SKD_HEADER_IDENT		( *( int * )"SKMD" )
#define TIKI_SKD_HEADER_OLD_VERSION	5
#define TIKI_SKD_HEADER_VERSION		6

// skc skeletor animations
#define TIKI_SKC_HEADER_IDENT		( *( int * )"SKAN" )
#define TIKI_SKC_HEADER_OLD_VERSION	13
#define TIKI_SKC_HEADER_VERSION		14

// tiki limitations
#define TIKI_MAX_BONES				100
#define TIKI_MAX_VERTEXES			1000
#define TIKI_MAX_TRIANGLES			2000
#define TIKI_MAX_SKELCACHE			1024
#define TIKI_MAX_COMMANDS			128

// tiki surface flags
#define TIKI_SURF_SKIN1				( 1 << 0 )
#define TIKI_SURF_SKIN2				( 1 << 1 )
#define TIKI_SURF_SKIN3				( TIKI_SURF_SKIN1 | TIKI_SURF_SKIN2 )
#define TIKI_SURF_NODRAW			( 1 << 2 )
#define TIKI_SURF_CROSSFADE			( 1 << 6 )
#define TIKI_SURF_NODAMAGE			( 1 << 7 )
#define TIKI_SURF_NOMIPMAPS			( 1 << 8 )
#define TIKI_SURF_NOPICMIP			( 1 << 9 )

typedef struct AliasList_s AliasList_t;
typedef struct AliasListNode_s AliasListNode_t;

typedef struct tiki_singlecmd_s {
	int num_args;
	char **args;
} tiki_singlecmd_t;

typedef struct tiki_cmd_s {
	int num_cmds;
	tiki_singlecmd_t cmds[ TIKI_MAX_COMMANDS ];
} tiki_cmd_t;

#ifdef __cplusplus

typedef class skelChannelList_s {
public:
	short int	m_numChannels;

private:
	short int	m_numLocalFromGlobal;
	short int	*m_chanLocalFromGlobal;

public:
	short int	m_chanGlobalFromLocal[ 200 ];

	int					NumChannels( void ) const;
	void				ZeroChannels( void );
	void				PackChannels( void );
	void				InitChannels( void );
	void				CleanUpChannels();
	int					GlobalChannel( int localchannel ) const;
	int					LocalChannel( int channel ) const;
	int					GetLocalFromGlobal( int globalChannel ) const;
	void				SetLocalFromGlobal( int channel, int localchannel );
	qboolean			HasChannel( ChannelNameTable *nameTable, const char *channelName ) const;
	qboolean			HasChannel( int channel ) const;
	int					AddChannel( int newGlobalChannelNum );
	const char			*ChannelName( ChannelNameTable *nameTable, int localChannelNum ) const;
} skelChannelList_c;

#else

typedef struct skelChannelList_s {
	short int	m_numChannels;
	short int	m_numLocalFromGlobal;
	short int	*m_chanLocalFromGlobal;
	short int	m_chanGlobalFromLocal[ 200 ];
} skelChannelList_c;

#endif

typedef struct dtikianim_s {
	char *name;
	int num_anims;
	void *alias_list;
	int num_client_initcmds;
	struct dtikicmd_s *client_initcmds;
	int num_server_initcmds;
	struct dtikicmd_s *server_initcmds;
	byte *modelData;
	size_t modelDataSize;
	float mins[ 3 ];
	float maxs[ 3 ];
	short int *m_aliases;
	char *headmodels;
	char *headskins;
	qboolean bIsCharacter;
	struct dtikianimdef_s *animdefs[ 1 ];
} dtikianim_t;

typedef struct dtiki_s {
	char *name;
	dtikianim_t *a;
	void *skeletor;
	int num_surfaces;
	struct dtikisurface_s *surfaces;
	float load_scale;
	float lod_scale;
	float lod_bias;
	float light_offset[ 3 ];
	float load_origin[ 3 ];
	float radius;
	skelChannelList_c m_boneList;
	int numMeshes;
	short int mesh[ 1 ];

#ifdef __cplusplus
	int			GetBoneNumFromName( const char *name );
	const char	*GetBoneNameFromNum( int num ) const;
#endif
} dtiki_t;

typedef struct {
	int ident;
	int version;
	char name[ 64 ];
	int numSurfaces;
	int numBones;
	int ofsBones;
	int ofsSurfaces;
	int ofsEnd;
	int lodIndex[ 10 ];
	int numBoxes;
	int ofsBoxes;
	int numMorphTargets;
	int ofsMorphTargets;
} skelHeader_t;

typedef struct {
	short int parent;
	short int boxIndex;
	int flags;
	char name[ 64 ];
} skelBoneName_t;

typedef struct {
	int ident;
	char name[ 64 ];
	int numTriangles;
	int numVerts;
	int staticSurfProcessed;
	int ofsTriangles;
	int ofsVerts;
	int ofsCollapse;
	int ofsEnd;
	int ofsCollapseIndex;
} skelSurface_t;

typedef short int skelIndex_t;

typedef struct  skelSurfaceGame_s {
	int ident;
	char name[ MAX_QPATH ];
	int numTriangles;
	int numVerts;
	vec4_t *pStaticXyz;
	vec4_t *pStaticNormal;
	vec2_t ( *pStaticTexCoords )[ 2 ];
	skelIndex_t *pTriangles;
	struct skeletorVertex_s *pVerts;
	skelIndex_t *pCollapse;
	struct skelSurfaceGame_s *pNext;
	skelIndex_t *pCollapseIndex;

#ifdef __cplusplus
	skelSurfaceGame_s();
#endif
} skelSurfaceGame_t;

typedef struct skelHeaderGame_s {
	int version;
	char name[ 64 ];
	int numSurfaces;
	int numBones;
	struct boneData_s *pBones;
	skelSurfaceGame_t *pSurfaces;
	int lodIndex[ 10 ];
	int numBoxes;
	struct skelHitBox_s *pBoxes;
	struct lodControl_s *pLOD;
	int numMorphTargets;
	char *pMorphTargets;
} skelHeaderGame_t;

typedef struct boneData_s {
	short int channel;
	enum boneType_e boneType;
	short int parent;
	short int numChannels;
	short int numRefs;
	short int channelIndex[ 2 ];
	short int refIndex[ 2 ];
	vec3_t offset;
	float length;
	float weight;
	float bendRatio;
	float bendMax;
	float spinRatio;
} boneData_t;

typedef struct tikiSurface_s {
	int ident;
	char name[ 64 ];
	int numFrames;
	int numVerts;
	int minLod;
	int numTriangles;
	int ofsTriangles;
	int ofsCollapseMap;
	int ofsSt;
	int ofsXyzNormals;
	int ofsEnd;
} tikiSurface_t;

#define MAX_TIKI_SHADER		4

typedef struct dtikisurface_s {
	char name[ MAX_QPATH ];
	char shader[ MAX_TIKI_SHADER ][ MAX_QPATH ];
	int hShader[ MAX_TIKI_SHADER ];
	int numskins;
	int flags;
	float damage_multiplier;
} dtikisurface_t;

/*typedef struct tikiFrame_s {
	vec3_t bounds[ 2 ];
	vec3_t scale;
	vec3_t offset;
	vec3_t delta;
	float radius;
	float frametime;
} tikiFrame_t;*/

typedef struct tiki_tess_data_s {
	int ident;
	tikiSurface_t *newsurface;
	tikiSurface_t *oldsurface;
} tiki_tess_data_t;

typedef struct skelWeight_s {
	int boneIndex;
	float boneWeight;
	vec3_t offset;
} skelWeight_t;

typedef struct skelVertex_s {
	vec3_t normal;
	vec2_t texCoords;
	int numWeights;
	skelWeight_t weights[ 1 ];
} skelVertex_t;

typedef struct skeletorVertex_s {
	vec3_t normal;
	vec2_t texCoords;
	int numWeights;
	int numMorphs;
} skeletorVertex_t;

typedef struct skeletorVertexGame_s {
	vec3_t normal;
	vec2_t texCoords;
	int numWeights;
	int numMorphs;
} skeletorVertexGame_t;

typedef struct {
	int ident;
	int ofsStaticData;
	skelSurfaceGame_t *surface;
	int meshNum;
} staticSurface_t;

typedef struct {
	int morphIndex;
	vec3_t offset;
} skeletorMorph_t;

typedef struct dtikianimdef_s {
	char alias[ 48 ];
	float weight;
	float blendtime;
	int flags;
	int num_client_cmds;
	struct dtikicmd_s *client_cmds;
	int num_server_cmds;
	struct dtikicmd_s *server_cmds;
} dtikianimdef_t;

typedef struct {
	short int shortQuat[ 4 ];
	short int shortOffset[ 3 ];
	short int pad;
} skelBone_t;

typedef struct {
	float bounds[ 2 ][ 3 ];
	float radius;
	float delta[ 3 ];
	skelBone_t bones[ 1 ];
} skelFrame_t;

typedef struct {
	int ident;
	int version;
	char name[ 64 ];
	int type;
	int numFrames;
	int numbones;
	float totaltime;
	float frametime;
	float totaldelta[ 3 ];
	int ofsFrames;
} skelAnimHeader_t;

typedef struct {
	float offset[ 4 ];
	float matrix[ 3 ][ 4 ];
} skelBoneCache_t;

typedef struct {
	float pos;
	float val;
} lodCurvePoint_t;

typedef struct {
	float base;
	float scale;
	float cutoff;
} lodCurvePointConstants_t;

typedef struct lodControl_s {
	float minMetric;
	float maxMetric;
	lodCurvePoint_t curve[ 5 ];
	lodCurvePointConstants_t consts[ 4 ];
} lodControl_t;

typedef struct {
	char path[ 96 ];
	skelHeaderGame_t *skel;
	size_t size;
	int numuses;
} skelcache_t;

#endif // __TIKI_SHARED_H__
