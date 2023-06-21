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

// skeletor_animation_file_format.h : Skeletor animation file format

#ifndef __SKELETOR_ANIMATION_FILE_FORMAT_H__
#define __SKELETOR_ANIMATION_FILE_FORMAT_H__

typedef float skelAnimChannel_t[ 4 ];

typedef struct {
	SkelVec3 bounds[ 2 ];
	float radius;
	SkelVec3 delta;
	float angleDelta;
	int iOfsChannels; 
} skelAnimFileFrame_t;

typedef struct {
	int ident;
	int version;
	int flags;
	int nBytesUsed;
	float frameTime;
	SkelVec3 totalDelta;
	float totalAngleDelta;
	int numChannels;
	int ofsChannelNames;
	int numFrames;
	skelAnimFileFrame_t frame[ 1 ];
} skelAnimDataFileHeader_t;

typedef struct {
	SkelVec3 bounds[ 2 ];
	float radius;
	SkelVec3 delta;
	float angleDelta;
	vec4_t *pChannels;
} skelAnimGameFrame_t;

typedef struct {
	short int nFrameNum;
	short int nPrevFrameIndex;
	vec4_t pChannelData;
} skanGameFrame;

typedef struct {
	short int nFramesInChannel;
	skanGameFrame *ary_frames;
} skanChannelHdr;

typedef struct {
	short	nFrameNum;
	vec3_t	pos;
	vec3_t	rot;
} skanAnimFrame;

typedef struct skelAnimDataGameHeader_s skelAnimDataGameHeader_t;

#ifdef __cplusplus

typedef struct skelAnimDataGameHeader_s {
	int flags;
	int nBytesUsed;
	bool bHasDelta;
	bool bHasMorph;
	bool bHasUpper;
	int numFrames;
	SkelVec3 totalDelta;
	float totalAngleDelta;
	float frameTime;
	skelChannelList_c channelList;
	SkelVec3 bounds[ 2 ];
	skelAnimGameFrame_t *m_frame;
	short int nTotalChannels;
	skanChannelHdr ary_channels[ 1 ];

	skelAnimDataGameHeader_s( const skelAnimDataGameHeader_t& );
	skelAnimDataGameHeader_s();

	static skelAnimDataGameHeader_t		*AllocRLEChannelData(size_t numChannels);
	int									GetFrameNums( float timeSeconds, float timeTolerance, int *beforeFrame, int *afterFrame, float *beforeWeight, float *afterWeight );
	SkelVec3							GetDeltaOverTime( float, float );
	static skelAnimDataGameHeader_t		*AllocAnimData(size_t numFrames, size_t numChannels );
	static void							DeallocAnimData( skelAnimDataGameHeader_t *data );
} skelAnimDataGameHeader_t;

#endif

#endif // __SKELETOR_ANIMATION_FILE_FORMAT_H__
