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

// skeletor.h : General skeletor structures

#ifndef __SKELETOR__
#define __SKELETOR__

#include "../tiki/tiki_shared.h"
#include "SkelVec3.h"
#include "SkelVec4.h"
#include "SkelMat3.h"
#include "SkelMat4.h"
#include "SkelQuat.h"
#include "skeletor_animation_file_format.h"
#include "skeletor_model_file_format.h"
#include "skeletor_name_lists.h"
#include "skeletor_internal.h"

#ifdef __cplusplus
#include "container.h"
#endif

typedef struct skelAnimFrame_s {
	float radius;
	SkelVec3 bounds[ 2 ];
	SkelMat4 bones[ 1 ];
} skelAnimFrame_t;

typedef struct {
	float seconds;
} skelAnimTime_t;

typedef struct {
	float weight;
	skelAnimDataGameHeader_t *pAnimationData;
	skelAnimGameFrame_t *frame;
} skelAnimBlendFrame_t;

typedef struct {
	float weight;
	skelAnimDataGameHeader_t *pAnimationData;
	int frame;
} skanBlendInfo;

#ifdef __cplusplus

class skelAnimStoreFrameList_c {
public:
	short int numMovementFrames;
	short int numActionFrames;
	float actionWeight;
	skanBlendInfo m_blendInfo[ 64 ];

public:
	SkelQuat	GetSlerpValue( int globalChannelNum ) const;
	void		GetLerpValue3( int globalChannelNum, SkelVec3 *outVec ) const;
};

class skeletor_c {
public:
	dtiki_t						*m_Tiki;
	static ChannelNameTable		m_boneNames;
	static ChannelNameTable		m_channelNames;
	static skelBone_World		m_worldBone;

private:
	SkelVec3 m_frameBounds[ 2 ];
	float m_frameRadius;
	skelAnimStoreFrameList_c m_frameList;
	short int m_targetLookLeft;
	short int m_targetLookRight;
	short int m_targetLookUp;
	short int m_targetLookDown;
	short int m_targetLookCrossed;
	short int m_targetBlink;
	short int m_timeNextBlink;
	short int m_headBoneIndex;
	vec3_t m_eyeTargetPos;
	vec3_t m_eyePrevTargetPos;
	class skelBone_Base *m_leftFoot;
	class skelBone_Base *m_rightFoot;
	skelChannelList_c m_morphTargetList;
	class skelBone_Base **m_bone;

public:
	skeletor_c( dtiki_t *tiki );
	~skeletor_c();

	static skelAnimDataGameHeader_t		*ConvertSkelFileToGame( skelAnimDataFileHeader_t *pHeader, int iBuffLength, const char *path );
	static void							SaveProcessedAnim( skelAnimDataGameHeader_t *enAnim, const char *path, skelAnimDataFileHeader_t *pHeader );
	static skelAnimDataGameHeader_t		*LoadProcessedAnim( const char *path, void *buffer, int len, const char *name );
	static skelAnimDataGameHeader_t		*LoadProcessedAnimEx( const char *path, void *buffer, int len, const char *name );
	void								PrintBoneCacheList();
	void								PrintBoneList();
	void								LoadMorphTargetNames( skelHeaderGame_t *modelHeader );
	void								GetFrame( skelAnimFrame_t *newFrame );
	int									GetMorphWeightFrame( int *data );
	SkelMat4							GetBoneFrame( int boneIndex );
	void								GetFrameBounds( SkelVec3 *, SkelVec3 * );
	float								GetModelLODRadius();
	bool								IsBoneOnGround( int boneIndex, float threshold );
	int									GetMorphWeightFrame( int index, float time, int *data );
	qboolean							LoadSKB( const char * );
	float								GetRadius();
	float								GetCentroidRadius( float *centroid );
	void								SetPose( const frameInfo_t *frameInfo, const int *contIndices, const vec4_t *contValues, float actionWeight );
	void								SetEyeTargetPos( const float *pEyeTargetPos );
	int									GetBoneParent( int boneIndex );

private:
	void Init();
	static class ChannelNameTable *ChannelNames();
	SkelMat4 *BoneTransformation( int, int *, float( *)[ 4 ] );
};

#endif

#ifdef __cplusplus
extern "C" {
#endif

//
// skeletor.cpp
//

void ConvertToRotationName( const char *boneName, char *rotChannelName );
void ConvertToPositionName( const char *boneName, char *posChannelName );
void ConvertToFKRotationName( const char *boneName, char *rotChannelName );
void ConvertToFKPositionName( const char *boneName, char *rotChannelName );
void AddToBounds( SkelVec3 *bounds, SkelVec3 *newBounds );
#ifdef __cplusplus
void BoneGetFrames( skelHeaderGame_t *skelmodel, skelAnimDataGameHeader_t *animData, skelChannelList_c *boneList, int boneNum, Container< skanAnimFrame >& outFrames );
void TIKI_GetSkelAnimFrameInternal2(dtiki_t *tiki, skelBoneCache_t *bones, skelAnimStoreFrameList_c *frameList, float *radius, vec3_t *mins, vec3_t *maxes);
void SkeletorGetAnimFrame2(skelHeaderGame_t *skelmodel, skelChannelList_c *boneList, skelBoneCache_t *bones, skelAnimStoreFrameList_c *frameList, float *radius, vec3_t *mins, vec3_t *maxes);
#endif
void SkeletorGetAnimFrame( skelHeaderGame_t *skelmodel, skelAnimDataGameHeader_t *animData, skelChannelList_c *boneList, skelBoneCache_t *bones, int frame, float *radius, vec3_t *mins, vec3_t *maxes );
void TIKI_GetSkelAnimFrame( dtiki_t *tiki, skelBoneCache_t *bones, float *radius, vec3_t *mins, vec3_t *maxes );
void TIKI_GetSkelAnimFrame2( dtiki_t *tiki, skelBoneCache_t *bones, int anim, int frame, float *radius, vec3_t *mins, vec3_t *maxes );
void TIKI_GetSkelAnimFrameInternal( dtiki_t *tiki, skelBoneCache_t *bones, skelAnimDataGameHeader_t *animData, int frame, float *radius, vec3_t *mins, vec3_t *maxes );

//
// skeletor_imports.cpp
//

void Skel_DPrintf( const char *fmt, ... );

#ifndef _DEBUG_MEM
void Skel_Free( void *ptr );
void *Skel_Alloc( size_t size );
#else
#define Skel_Free(ptr) free(ptr)
#define Skel_Alloc(size) malloc(size)
#endif

void Skel_FreeFile( void *buffer );
int Skel_ReadFileEx( const char *qpath, void **buffer, qboolean quiet );

//
// skeletor_utilities.cpp
//

void SKEL_Message( const char *fmt, ... );
void SKEL_Warning( const char *fmt, ... );
void SKEL_Error( const char *fmt, ... );
void Skel_ExtractFilePath( const char *path, char *dest );
void Skel_ExtractFileBase( const char *path, char *dest );
void Skel_ExtractFileExtension( const char *path, char *dest );
//const char *Skel_ExtractFileExtension( const char *in );
void Skel_ExtractFileName( const char *path, char *dest );
int FileLength( FILE *pFile );

//
// skeletorbones.cpp
//

void SkeletorLoadBoneFromBuffer( skelChannelList_c *boneList, boneData_t *boneData, skelBone_Base **bone );
void SkeletorLoadBonesFromBuffer( skelChannelList_c *boneList, skelHeaderGame_t *buffer, skelBone_Base **bone );

#ifdef __cplusplus
}
#endif

#endif // __SKELETOR__
