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

// skeletor.cpp : Skeletor

#include "q_shared.h"
#include "qcommon.h"
#include "skeletor.h"
#include "dbgheap.h"

#define EPSILON		0.000000000001f


int skelAnimDataGameHeader_s::GetFrameNums( float timeSeconds, float timeTolerance, int *beforeFrame, int *afterFrame, float *beforeWeight, float *afterWeight )
{
	int frameNum1;
	int frameNum2;

	frameNum1 = ( int )( timeSeconds / frameTime );
	frameNum2 = frameNum1 + 1;

	*afterWeight = ( timeSeconds / frameTime ) - ( float )frameNum1;

	if( ( 1.0 - *afterWeight ) * frameTime < timeTolerance ||
		*afterWeight * frameTime < timeTolerance )
	{
		if( *afterWeight > 0.5 )
			frameNum1++;

		if( frameNum1 >= numFrames )
		{
			if( ( flags & TAF_DELTADRIVEN ) ) {
				frameNum1 %= numFrames;
			} else {
				frameNum1 = numFrames - 1;
			}
		}

		*beforeFrame = frameNum1;
		*beforeWeight = 1.0;
		*afterFrame = 0;
		*afterWeight = 0.0;

		return 1;
	}
	else if( frameNum2 >= numFrames )
	{
		if( ( flags & TAF_DELTADRIVEN ) ) {
			frameNum2 %= numFrames;
		} else {
			frameNum2 = numFrames - 1;
		}

		if( frameNum1 >= numFrames )
		{
			if( ( flags & TAF_DELTADRIVEN ) )
			{
				frameNum1 %= numFrames;
			}
			else
			{
				*beforeFrame = numFrames - 1;
				*beforeWeight = 1.0;
				return 1;
			}
		}
	}

	*beforeFrame = frameNum1;
	*afterFrame = frameNum2;
	*beforeWeight = 1.0 - *afterWeight;

	return 2;
}

SkelVec3 skelAnimDataGameHeader_s::GetDeltaOverTime( float time1, float time2 )
{
	float deltaWeight1;
	int frameNum1;
	float deltaWeight2;
	int frameNum2;
	SkelVec3 delta;
	int currFrame;
	float s, d;

	deltaWeight1 = time1 / frameTime;
	deltaWeight2 = time2 / frameTime;
	frameNum1 = ( int )( deltaWeight1 + 1.0 );
	frameNum2 = ( int )( deltaWeight2 + 1.0 );

	d = frameNum1 - ( time1 / frameTime );
	s = 1.0 - ( frameNum2 - ( time2 / frameTime ) );

	if( frameNum1 < frameNum2 )
	{
		delta = m_frame[ frameNum1 % numFrames ].delta;

		for( currFrame = frameNum1 + 1; currFrame < frameNum2; currFrame++ )
		{
			delta += m_frame[ currFrame % numFrames ].delta;
		}
	}
	else
	{
		s = s - ( 1.0 - d );
	}

	delta.x += m_frame[ frameNum2 % numFrames ].delta.x * s;
	delta.y += m_frame[ frameNum2 % numFrames ].delta.y * s;
	delta.z += m_frame[ frameNum2 % numFrames ].delta.z * s;

	if( delta.x > -0.001f && delta.x < 0.001f ) {
		delta.x = 0;
	}
	if( delta.y > -0.001f && delta.y < 0.001f ) {
		delta.y = 0;
	}
	if( delta.z > -0.001f && delta.z < 0.001f ) {
		delta.z = 0;
	}

	return delta;
}

skelAnimDataGameHeader_t *skelAnimDataGameHeader_s::AllocAnimData( int numFrames, int numChannels )
{
	skelAnimDataGameHeader_t *data;
	int animSize;

	animSize = sizeof( skelAnimDataGameHeader_t ) + ( 16 * ( 3 * numFrames - 3 ) ) + 16 * numChannels * numFrames;

	data = ( skelAnimDataGameHeader_t * )Skel_Alloc( animSize );
	data->flags = 0;
	data->channelList.InitChannels();
	data->nBytesUsed = animSize;
	data->numFrames = numFrames;
	return data;
}


skelAnimDataGameHeader_s *skelAnimDataGameHeader_s::AllocRLEChannelData( int numChannels )
{
	int animSize;
	skelAnimDataGameHeader_t *data;

	animSize = sizeof( skelAnimDataGameHeader_t ) + ( numChannels - 1 ) * sizeof( skanChannelHdr );
	data = ( skelAnimDataGameHeader_t * )Skel_Alloc( animSize );
	data->flags = 0;
	data->nTotalChannels = numChannels;
	data->channelList.InitChannels();
	data->nBytesUsed = animSize;
	return data;
}

skeletor_c::skeletor_c( dtiki_t *tiki )
{
	int numBones;
	int mesh;
	skelHeaderGame_t *skelmodel;

	m_Tiki = tiki;

	m_morphTargetList.InitChannels();
	m_morphTargetList.ZeroChannels();

	m_frameBounds[ 0 ].set( -32, -32, 0 );
	m_frameBounds[ 1 ].set( 32, 32, 28 );
	m_frameRadius = 64;
	m_frameList.numActionFrames = 0;
	m_frameList.numMovementFrames = 0;
	m_targetLookLeft = 0;
	m_targetLookRight = 0;
	m_targetLookUp = 0;
	m_targetLookDown = 0;
	m_targetLookCrossed = 0;
	m_targetBlink = 0;

	VectorClear( m_eyeTargetPos );
	VectorClear( m_eyePrevTargetPos );

	m_timeNextBlink = Sys_Milliseconds();
	numBones = m_Tiki->m_boneList.NumChannels();
	m_bone = ( skelBone_Base ** )Skel_Alloc( numBones * sizeof( skelBone_Base * ) );
	memset( m_bone, 0, numBones * sizeof( skelBone_Base * ) );

	for( mesh = 0; mesh < m_Tiki->numMeshes; mesh++ )
	{
		skelmodel = TIKI_GetSkel( m_Tiki->mesh[ mesh ] );
		SkeletorLoadBonesFromBuffer( &m_Tiki->m_boneList, skelmodel, m_bone );
		LoadMorphTargetNames( skelmodel );
	}

	m_morphTargetList.PackChannels();
	m_headBoneIndex = m_Tiki->GetBoneNumFromName( "Bip01 Head" );
}

skeletor_c::~skeletor_c()
{
	int i;

	for( i = 0; i < m_Tiki->m_boneList.m_numChannels; i++ ) {
		delete m_bone[ i ];
	}

	m_morphTargetList.CleanUpChannels();
	Skel_Free( m_bone );
	m_bone = NULL;
}

void skelAnimDataGameHeader_s::DeallocAnimData( skelAnimDataGameHeader_t *data )
{
	skanChannelHdr *pChannel;
	int i;

	if( !data || data == ( skelAnimDataGameHeader_t * )-476 )
	{
		return;
	}

	for( i = 0; i < data->nTotalChannels; i++ )
	{
		pChannel = &data->ary_channels[ i ];

		if( pChannel->ary_frames )
		{
			Skel_Free( pChannel->ary_frames );
		}
	}

	data->channelList.CleanUpChannels();

	if( data->m_frame ) {
		Skel_Free( data->m_frame );
	}

	Skel_Free( data );
}

void ConvertToRotationName( const char *boneName, char *rotChannelName )
{
	strcpy( rotChannelName, boneName );
	strcat( rotChannelName, " rot" );
}

void ConvertToPositionName( const char *boneName, char *posChannelName )
{
	strcpy( posChannelName, boneName );
	strcat( posChannelName, " rot" );
}

void ConvertToFKRotationName( const char *boneName, char *rotChannelName )
{
	strcpy( rotChannelName, boneName );
	strcat( rotChannelName, " rot" );
	strcat( rotChannelName, "FK" );
}

void ConvertToFKPositionName( const char *boneName, char *rotChannelName )
{
	strcpy( rotChannelName, boneName );
	strcat( rotChannelName, " pos" );
	strcat( rotChannelName, "FK" );
}

int GetHighestFloat( float *selection )
{
	float currentHighest;
	int currentHighestIndex;
	int i;

	currentHighest = selection[ 0 ];
	currentHighestIndex = 0;

	for( i = 0; i < 8; i++ )
	{
		if( selection[ i ] > currentHighest )
		{
			currentHighest = selection[ i ];
			currentHighestIndex = i;
		}
	}

	return currentHighestIndex;
}
void AddToBounds( SkelVec3 *bounds, SkelVec3 *newBounds )
{
	int i;

	for( i = 0; i < 3; i++ )
	{
		if( bounds[ 0 ][ i ] > newBounds[ 0 ][ i ] )
		{
			bounds[ 0 ][ i ] = newBounds[ 0 ][ i ];
		}

		if( bounds[ 1 ][ i ] > newBounds[ 1 ][ i ] )
		{
			bounds[ 1 ][ i ] = newBounds[ 1 ][ i ];
		}
	}
}

void skeletor_c::SetPose( const frameInfo_t *frameInfo, const int *contIndices, const vec4_t *contValues, float actionWeight )
{
	skelAnimDataGameHeader_t *animData;
	short int *aliases;
	//int boneNum;
	int blendNum;
	int i;
	int blendFrame;
	int movementBlendFrame;
	int actionBlendFrame;
	//int realAnimIndex;
	int beforeFrame;
	int afterFrame;
	float beforeWeight;
	float afterWeight;
	int numFramesAdded;
	float cutoff_weight;
	int contNum;
	float animWeight;
	skanBlendInfo *frame1, *frame2;

	for( i = 0; i < m_Tiki->m_boneList.NumChannels(); i++ )
	{
		m_bone[ i ]->m_controller = NULL;
		m_bone[ i ]->m_isDirty = true;
	}

	if( contIndices && contValues )
	{
		for( i = 0; i < 5; i++ )
		{
			contNum = contIndices[ i ];
			if( contNum != -1 )
			{
				cutoff_weight = ( contValues[ i ][ 3 ] - 1.0 ) * ( contValues[ i ][ 3 ] - 1.0 );
				if( cutoff_weight >= EPSILON )
				{
					m_bone[ contNum ]->m_controller = ( float * )contValues[ i ];
				}
			}
		}
	}

	for( i = 0; i < 3; i++ )
	{
		m_frameBounds[ 0 ][ i ] = -2.0f;
		m_frameBounds[ 1 ][ i ] = 2.0f;
	}

	m_frameRadius = 2.0f;
	animWeight = 0.0f;
	movementBlendFrame = 0;
	actionBlendFrame = 32;
	aliases = m_Tiki->a->m_aliases;

	for( i = 0; i < MAX_FRAMEINFOS; i++ )
	{
		if( animWeight < frameInfo[ i ].weight ) {
			animWeight = frameInfo[ i ].weight;
		}
	}

	cutoff_weight = animWeight * 0.01f;

	for( i = 0; i < MAX_FRAMEINFOS; i++ )
	{
		if( frameInfo[ i ].weight > cutoff_weight )
		{
			animData = SkeletorCacheGetData( aliases[ frameInfo[ i ].index ] );
			if( animData->bHasDelta ) {
				blendFrame = movementBlendFrame;
			} else {
				blendFrame = actionBlendFrame;
			}

			beforeWeight = 0.0;
			afterWeight = 0.0;
			beforeFrame = 0.0;
			afterFrame = 0.0;
			
			numFramesAdded = animData->GetFrameNums( frameInfo[ i ].time, 0.01f, &beforeFrame, &afterFrame, &beforeWeight, &afterWeight );

			frame1 = &m_frameList.m_blendInfo[ blendFrame ];
			frame1->frame = beforeFrame;
			frame1->pAnimationData = animData;
			frame1->weight = beforeWeight * frameInfo[ i ].weight;

			AddToBounds( m_frameBounds, animData->bounds );

			if( frame1->pAnimationData->m_frame[ frame1->frame ].radius > m_frameRadius )
				m_frameRadius = frame1->pAnimationData->m_frame[ frame1->frame ].radius;

			if( numFramesAdded == 2 )
			{
				frame2 = &m_frameList.m_blendInfo[ blendFrame + 1 ];
				frame2->frame = afterFrame;
				frame2->pAnimationData = animData;
				frame2->weight = afterWeight * frameInfo[ i ].weight;
				AddToBounds( m_frameBounds, animData->bounds );

				if( frame2->pAnimationData->m_frame[ frame2->frame ].radius > m_frameRadius )
					m_frameRadius = frame2->pAnimationData->m_frame[ frame2->frame ].radius;

			}

			blendNum = blendFrame + numFramesAdded;

			if( animData->bHasDelta ) {
				movementBlendFrame = blendNum;
			} else {
				actionBlendFrame = blendNum;
			}
		}
	}

	for( i = 0; i < 3; i++ )
	{
		m_frameBounds[ 0 ][ i ] += -7.0f;
		m_frameBounds[ 1 ][ i ] += 7.0f;
	}

	m_frameList.numMovementFrames = movementBlendFrame;
	m_frameList.numActionFrames = actionBlendFrame - 32;
	m_frameList.actionWeight = actionWeight;
}

static SkelMat4 GetGlobalDefaultPosition( skelBone_Base *bone )
{
	SkelMat4 lLocalPosition;
	SkelMat4 lGlobalPosition;
	SkelMat4 lParentGlobalPosition;

	lLocalPosition = bone->GetTransform( NULL );

	if( bone->Parent() )
	{
		//lParentGlobalPosition = GetGlobalDefaultPosition( bone->Parent() );
		//lGlobalPosition.Multiply( lParentGlobalPosition, lLocalPosition );
		lGlobalPosition = lLocalPosition;
	}
	else
	{
		lGlobalPosition = lLocalPosition;
	}

	return lGlobalPosition;
}

SkelMat4 GlobalToLocal( skelBone_Base *bone, SkelMat4 pGlobalPosition )
{
	SkelMat4 lLocalPosition;
	SkelMat4 lParentGlobalPosition;

	if( bone->Parent() )
	{
		lParentGlobalPosition = GetGlobalDefaultPosition( bone->Parent() );
		lParentGlobalPosition.Inverse();
		lLocalPosition.Multiply( lParentGlobalPosition, pGlobalPosition );
	}
	else
	{
		lLocalPosition = pGlobalPosition;
	}

	return lLocalPosition;
}

void BoneGetFrames( skelHeaderGame_t *skelmodel, skelAnimDataGameHeader_t *animData, skelChannelList_c *boneList, int boneNum, Container< skanAnimFrame >& outFrames )
{
	int numBones;
	skelBone_Base **bone;
	int i;
	skanAnimFrame frame;
	int localChannelNum;

	outFrames.FreeObjectList();

	numBones = boneList->NumChannels();

	bone = ( skelBone_Base ** )Skel_Alloc( sizeof( skelBone_Base * ) * numBones );
	memset( bone, 0, sizeof( skelBone_Base * ) * numBones );

	SkeletorLoadBonesFromBuffer( boneList, skelmodel, bone );

	for( i = 0; i < numBones; i++ )
	{
		bone[ i ]->m_controller = NULL;
		bone[ i ]->m_isDirty = true;
	}

	// process the rot channel
	localChannelNum = animData->channelList.GetLocalFromGlobal( bone[ boneNum ]->GetChannelIndex( 1 ) );
	if( localChannelNum >= 0 )
	{
		skanChannelHdr *channel = &animData->ary_channels[ localChannelNum ];

		for( i = 0; i < channel->nFramesInChannel; i++ )
		{
			skanGameFrame *pFrame = &channel->ary_frames[ i ];

			frame.nFrameNum = pFrame->nFrameNum;
			VectorClear( frame.pos );
			QuatToAngles( pFrame->pChannelData, frame.rot );

			outFrames.AddObject( frame );
		}
	}

	if( bone[ i ]->GetChannelIndex( 0 ) != bone[ i ]->GetChannelIndex( 1 ) )
	{
		// process the pos channel
		localChannelNum = animData->channelList.GetLocalFromGlobal( bone[ boneNum ]->GetChannelIndex( 0 ) );
		if( localChannelNum >= 0 )
		{
			skanChannelHdr *channel = &animData->ary_channels[ localChannelNum ];

			for( i = 0; i < channel->nFramesInChannel; i++ )
			{
				skanGameFrame *pFrame = &channel->ary_frames[ i ];
				skanAnimFrame *pOutFrame = &outFrames[ i ];

				VectorCopy( pFrame->pChannelData, pOutFrame->pos );
			}
		}
	}
}

void SkeletorGetAnimFrame2(skelHeaderGame_t *skelmodel, skelChannelList_c *boneList, skelBoneCache_t *bones, skelAnimStoreFrameList_c *frameList, float *radius, vec3_t *mins, vec3_t *maxes)
{
	int numBones;
	skelBone_Base **bone;
	int i;
	skelAnimFrame_t *newFrame;

	numBones = skelmodel->numBones;

	bone = (skelBone_Base **)Skel_Alloc(sizeof(skelBone_Base *) * numBones);
	memset(bone, 0, sizeof(skelBone_Base *) * numBones);

	SkeletorLoadBonesFromBuffer(boneList, skelmodel, bone);

	for (i = 0; i < numBones; i++)
	{
		bone[i]->m_controller = NULL;
		bone[i]->m_isDirty = true;
	}

	newFrame = (skelAnimFrame_t *)Skel_Alloc(sizeof(skelAnimFrame_t) + sizeof(SkelMat4) * numBones);
	newFrame->radius = 0;
	newFrame->bounds[0] = SkelVec3();
	newFrame->bounds[1] = SkelVec3();

	for (i = 0; i < numBones; i++)
	{
		//skelBone_Base *Parent = bone[ i ]->Parent();
		//bone[ i ]->SetParent( &skeletor_c::m_worldBone );
		newFrame->bones[i] = bone[i]->GetTransform(frameList);
		//bone[ i ]->SetParent( Parent );
	}

	for (i = 0; i < numBones; i++)
	{
		VectorCopy(newFrame->bones[i][3], bones[i].offset);
		bones[i].matrix[0][0] = newFrame->bones[i][0][0];
		bones[i].matrix[0][1] = newFrame->bones[i][0][1];
		bones[i].matrix[0][2] = newFrame->bones[i][0][2];
		bones[i].matrix[0][3] = 0;
		bones[i].matrix[1][0] = newFrame->bones[i][1][0];
		bones[i].matrix[1][1] = newFrame->bones[i][1][1];
		bones[i].matrix[1][2] = newFrame->bones[i][1][2];
		bones[i].matrix[1][3] = 0;
		bones[i].matrix[2][0] = newFrame->bones[i][2][0];
		bones[i].matrix[2][1] = newFrame->bones[i][2][1];
		bones[i].matrix[2][2] = newFrame->bones[i][2][2];
		bones[i].matrix[2][3] = 0;
	}

	for (i = 0; i < numBones; i++)
	{
		delete bone[i];
	}

	Skel_Free(bone);

	if (radius) {
		*radius = newFrame->radius;
	}

	if (mins || maxes)
	{
		for (i = 0; i < 3; i++)
		{
			if (mins) {
				(*mins)[i] = newFrame->bounds[0][i];
			}
			if (maxes) {
				(*maxes)[i] = newFrame->bounds[1][i];
			}
		}
	}

	Skel_Free(newFrame);
}

void SkeletorGetAnimFrame( skelHeaderGame_t *skelmodel, skelAnimDataGameHeader_t *animData, skelChannelList_c *boneList, skelBoneCache_t *bones, int frame, float *radius, vec3_t *mins, vec3_t *maxes )
{
	int numBones;
	skelBone_Base **bone;
	int i;
	skelAnimStoreFrameList_c frameList;
	skelAnimFrame_t *newFrame;

	frameList.actionWeight = animData ? 1.0 : 0;

	if (!animData->bHasDelta)
	{
		frameList.numMovementFrames = 0;
		frameList.numActionFrames = 1;
		frameList.m_blendInfo[32].weight = 1.0;
		frameList.m_blendInfo[32].pAnimationData = animData;
		frameList.m_blendInfo[32].frame = frame;
	}
	else
	{
		frameList.numMovementFrames = 1;
		frameList.numActionFrames = 0;
		frameList.m_blendInfo[0].weight = 1.0;
		frameList.m_blendInfo[0].pAnimationData = animData;
		frameList.m_blendInfo[0].frame = frame;
	}
	numBones = skelmodel->numBones;

	bone = ( skelBone_Base ** )Skel_Alloc( sizeof( skelBone_Base * ) * numBones );
	memset( bone, 0, sizeof( skelBone_Base * ) * numBones );

	SkeletorLoadBonesFromBuffer( boneList, skelmodel, bone );

	for( i = 0; i < numBones; i++ )
	{
		bone[ i ]->m_controller = NULL;
		bone[ i ]->m_isDirty = true;
	}

	newFrame = ( skelAnimFrame_t * )Skel_Alloc( sizeof( skelAnimFrame_t ) + sizeof( SkelMat4 ) * numBones );

	if( animData )
	{
		if( animData->m_frame )
		{
			newFrame->radius = animData->m_frame->radius;
		}
		else
		{
			newFrame->radius = 0;
		}

		newFrame->bounds[ 0 ] = animData->bounds[ 0 ];
		newFrame->bounds[ 1 ] = animData->bounds[ 1 ];
	}
	else
	{
		newFrame->radius = 0;
		newFrame->bounds[ 0 ] = SkelVec3();
		newFrame->bounds[ 1 ] = SkelVec3();
	}

	for( i = 0; i < numBones; i++ )
	{
		//skelBone_Base *Parent = bone[ i ]->Parent();
		//bone[ i ]->SetParent( &skeletor_c::m_worldBone );
		newFrame->bones[ i ] = bone[ i ]->GetTransform( &frameList );
		//bone[ i ]->SetParent( Parent );
	}

	for( i = 0; i < numBones; i++ )
	{
		VectorCopy( newFrame->bones[ i ][ 3 ], bones[ i ].offset );
		bones[ i ].matrix[ 0 ][ 0 ] = newFrame->bones[ i ][ 0 ][ 0 ];
		bones[ i ].matrix[ 0 ][ 1 ] = newFrame->bones[ i ][ 0 ][ 1 ];
		bones[ i ].matrix[ 0 ][ 2 ] = newFrame->bones[ i ][ 0 ][ 2 ];
		bones[ i ].matrix[ 0 ][ 3 ] = 0;
		bones[ i ].matrix[ 1 ][ 0 ] = newFrame->bones[ i ][ 1 ][ 0 ];
		bones[ i ].matrix[ 1 ][ 1 ] = newFrame->bones[ i ][ 1 ][ 1 ];
		bones[ i ].matrix[ 1 ][ 2 ] = newFrame->bones[ i ][ 1 ][ 2 ];
		bones[ i ].matrix[ 1 ][ 3 ] = 0;
		bones[ i ].matrix[ 2 ][ 0 ] = newFrame->bones[ i ][ 2 ][ 0 ];
		bones[ i ].matrix[ 2 ][ 1 ] = newFrame->bones[ i ][ 2 ][ 1 ];
		bones[ i ].matrix[ 2 ][ 2 ] = newFrame->bones[ i ][ 2 ][ 2 ];
		bones[ i ].matrix[ 2 ][ 3 ] = 0;
	}

	for( i = 0; i < numBones; i++ )
	{
		delete bone[ i ];
	}

	Skel_Free( bone );

	if( radius ) {
		*radius = newFrame->radius;
	}

	if( mins || maxes )
	{
		for( i = 0; i < 3; i++ )
		{
			if( mins ) {
				( *mins )[ i ] = newFrame->bounds[ 0 ][ i ];
			}
			if( maxes ) {
				( *maxes )[ i ] = newFrame->bounds[ 1 ][ i ];
			}
		}
	}

	Skel_Free( newFrame );
}

void TIKI_GetSkelAnimFrameInternal2(dtiki_t *tiki, skelBoneCache_t *bones, skelAnimStoreFrameList_c* frameList, float *radius, vec3_t *mins, vec3_t *maxes)
{
	//int boneNum;
	int numBones;
	skelBone_Base **bone;
	int i;
	skelAnimFrame_t *newFrame;
	//int realAnimIndex;
	//skanBlendInfo *frame;
	skelHeaderGame_t *skelmodel;

	numBones = tiki->m_boneList.NumChannels();

	bone = (skelBone_Base **)Skel_Alloc(sizeof(skelBone_Base *) * numBones);
	memset(bone, 0, sizeof(skelBone_Base *) * numBones);

	for (i = 0; i < tiki->numMeshes; i++)
	{
		skelmodel = TIKI_GetSkel(tiki->mesh[i]);
		SkeletorLoadBonesFromBuffer(&tiki->m_boneList, skelmodel, bone);
	}

	for (i = 0; i < numBones; i++)
	{
		bone[i]->m_controller = NULL;
		bone[i]->m_isDirty = true;
	}

	newFrame = (skelAnimFrame_t *)Skel_Alloc(sizeof(skelAnimFrame_t) + sizeof(SkelMat4) * numBones);

	/*
	if (animData)
	{
		if (animData->m_frame)
		{
			newFrame->radius = animData->m_frame->radius;
		}
		else
		{
			newFrame->radius = 0;
		}

		newFrame->bounds[0] = animData->bounds[0];
		newFrame->bounds[1] = animData->bounds[1];
	}
	else
	*/
	{
		newFrame->radius = 0;
		newFrame->bounds[0] = SkelVec3();
		newFrame->bounds[1] = SkelVec3();
	}

	for (i = 0; i < numBones; i++)
	{
		newFrame->bones[i] = bone[i]->GetTransform(frameList);
	}

	for (i = 0; i < numBones; i++)
	{
		delete bone[i];
	}

	Skel_Free(bone);

	for (i = 0; i < numBones; i++)
	{
		VectorCopy(newFrame->bones[i][3], bones[i].offset);
		bones[i].matrix[0][0] = newFrame->bones[i][0][0];
		bones[i].matrix[0][1] = newFrame->bones[i][0][1];
		bones[i].matrix[0][2] = newFrame->bones[i][0][2];
		bones[i].matrix[0][3] = 0;
		bones[i].matrix[1][0] = newFrame->bones[i][1][0];
		bones[i].matrix[1][1] = newFrame->bones[i][1][1];
		bones[i].matrix[1][2] = newFrame->bones[i][1][2];
		bones[i].matrix[1][3] = 0;
		bones[i].matrix[2][0] = newFrame->bones[i][2][0];
		bones[i].matrix[2][1] = newFrame->bones[i][2][1];
		bones[i].matrix[2][2] = newFrame->bones[i][2][2];
		bones[i].matrix[2][3] = 0;
	}

	if (radius) {
		*radius = newFrame->radius;
	}

	if (mins && maxes)
	{
		for (i = 0; i < 3; i++)
		{
			(*mins)[i] = newFrame->bounds[0][i];
			(*maxes)[i] = newFrame->bounds[1][i];
		}
	}

	Skel_Free(newFrame);
}

void TIKI_GetSkelAnimFrameInternal( dtiki_t *tiki, skelBoneCache_t *bones, skelAnimDataGameHeader_t *animData, int frame, float *radius, vec3_t *mins, vec3_t *maxes )
{
	//int boneNum;
	int numBones;
	skelBone_Base **bone;
	int i;
	skelAnimStoreFrameList_c frameList;
	skelAnimFrame_t *newFrame;
	//int realAnimIndex;
	//skanBlendInfo *frame;
	skelHeaderGame_t *skelmodel;

	frameList.actionWeight = animData ? 1.0 : 0;
	if (!animData || !animData->bHasDelta)
	{
		frameList.numMovementFrames = 0;
		frameList.numActionFrames = 1;
		frameList.m_blendInfo[32].weight = 1.0;
		frameList.m_blendInfo[32].pAnimationData = animData;
		frameList.m_blendInfo[32].frame = frame;
	}
	else
	{
		frameList.numMovementFrames = 1;
		frameList.numActionFrames = 0;
		frameList.m_blendInfo[0].weight = 1.0;
		frameList.m_blendInfo[0].pAnimationData = animData;
		frameList.m_blendInfo[0].frame = frame;
	}
	numBones = tiki->m_boneList.NumChannels();

	bone = ( skelBone_Base ** )Skel_Alloc( sizeof( skelBone_Base * ) * numBones );
	memset( bone, 0, sizeof( skelBone_Base * ) * numBones );

	for( i = 0; i < tiki->numMeshes; i++ )
	{
		skelmodel = TIKI_GetSkel( tiki->mesh[ i ] );
		SkeletorLoadBonesFromBuffer( &tiki->m_boneList, skelmodel, bone );
	}

	for( i = 0; i < numBones; i++ )
	{
		bone[ i ]->m_controller = NULL;
		bone[ i ]->m_isDirty = true;
	}

	newFrame = ( skelAnimFrame_t * )Skel_Alloc( sizeof( skelAnimFrame_t ) + sizeof( SkelMat4 ) * numBones );

	if( animData )
	{
		if( animData->m_frame )
		{
			newFrame->radius = animData->m_frame->radius;
		}
		else
		{
			newFrame->radius = 0;
		}

		newFrame->bounds[ 0 ] = animData->bounds[ 0 ];
		newFrame->bounds[ 1 ] = animData->bounds[ 1 ];
	}
	else
	{
		newFrame->radius = 0;
		newFrame->bounds[ 0 ] = SkelVec3();
		newFrame->bounds[ 1 ] = SkelVec3();
	}

	for( i = 0; i < numBones; i++ )
	{
		newFrame->bones[ i ] = bone[ i ]->GetTransform( &frameList );
	}

	for( i = 0; i < numBones; i++ )
	{
		delete bone[ i ];
	}

	Skel_Free( bone );

	for( i = 0; i < numBones; i++ )
	{
		VectorCopy( newFrame->bones[ i ][ 3 ], bones[ i ].offset );
		bones[ i ].matrix[ 0 ][ 0 ] = newFrame->bones[ i ][ 0 ][ 0 ];
		bones[ i ].matrix[ 0 ][ 1 ] = newFrame->bones[ i ][ 0 ][ 1 ];
		bones[ i ].matrix[ 0 ][ 2 ] = newFrame->bones[ i ][ 0 ][ 2 ];
		bones[ i ].matrix[ 0 ][ 3 ] = 0;
		bones[ i ].matrix[ 1 ][ 0 ] = newFrame->bones[ i ][ 1 ][ 0 ];
		bones[ i ].matrix[ 1 ][ 1 ] = newFrame->bones[ i ][ 1 ][ 1 ];
		bones[ i ].matrix[ 1 ][ 2 ] = newFrame->bones[ i ][ 1 ][ 2 ];
		bones[ i ].matrix[ 1 ][ 3 ] = 0;
		bones[ i ].matrix[ 2 ][ 0 ] = newFrame->bones[ i ][ 2 ][ 0 ];
		bones[ i ].matrix[ 2 ][ 1 ] = newFrame->bones[ i ][ 2 ][ 1 ];
		bones[ i ].matrix[ 2 ][ 2 ] = newFrame->bones[ i ][ 2 ][ 2 ];
		bones[ i ].matrix[ 2 ][ 3 ] = 0;
	}

	if( radius ) {
		*radius = newFrame->radius;
	}

	if( mins && maxes )
	{
		for( i = 0; i < 3; i++ )
		{
			( *mins )[ i ] = newFrame->bounds[ 0 ][ i ];
			( *maxes )[ i ] = newFrame->bounds[ 1 ][ i ];
		}
	}

	Skel_Free( newFrame );
}

void TIKI_GetSkelAnimFrame2( dtiki_t *tiki, skelBoneCache_t *bones, int anim, int frame, float *radius, vec3_t *mins, vec3_t *maxes )
{
	short *aliases;
	skelAnimDataGameHeader_t *animData;

	aliases = tiki->a->m_aliases;
	if( *aliases == -1 )
	{
		SKEL_Warning( "TIKI_GetSkelAnimFrame: Bad anim in static model %s, couldn't generate pose properly.\n", tiki->name );
		return;
	}

	animData = SkeletorCacheGetData( aliases[ anim ] );

	TIKI_GetSkelAnimFrameInternal( tiki, bones, animData, frame, NULL, NULL, NULL );
}

void TIKI_GetSkelAnimFrame( dtiki_t *tiki, skelBoneCache_t *bones, float *radius, vec3_t *mins, vec3_t *maxes )
{
	TIKI_GetSkelAnimFrame2( tiki, bones, 0, 0, radius, mins, maxes );
}

void skeletor_c::GetFrame( skelAnimFrame_t *newFrame )
{
	int boneNum;
	int numBones;

	numBones = m_Tiki->m_boneList.NumChannels();

	for( boneNum = 0; boneNum < numBones; boneNum++ )
	{
		newFrame->bones[ boneNum ] = GetBoneFrame( boneNum );
	}

	newFrame->bounds[ 0 ] = m_frameBounds[ 0 ];
	newFrame->bounds[ 1 ] = m_frameBounds[ 1 ];
	newFrame->radius = m_frameRadius;
}


SkelMat4 skeletor_c::GetBoneFrame( int boneIndex )
{
	return m_bone[ boneIndex ]->GetTransform( &m_frameList );
}

bool skeletor_c::IsBoneOnGround( int boneIndex, float threshold )
{
	return GetBoneFrame( boneIndex ).val[ 3 ][ 2 ] < threshold;
}


float skeletor_c::GetRadius()
{
	return m_frameRadius;
}


float skeletor_c::GetCentroidRadius( float *centroid )
{
	centroid[ 0 ] = ( m_frameBounds[ 0 ][ 0 ] + m_frameBounds[ 1 ][ 0 ] ) * 0.5f;
	centroid[ 1 ] = ( m_frameBounds[ 0 ][ 1 ] + m_frameBounds[ 1 ][ 1 ] ) * 0.5f;
	centroid[ 2 ] = ( m_frameBounds[ 0 ][ 2 ] + m_frameBounds[ 1 ][ 2 ] ) * 0.5f;
	return m_frameRadius;
}


int skeletor_c::GetMorphWeightFrame( int index, float time, int *data )
{
	return GetMorphWeightFrame( data );
}

vec4_t *DecodeFrameValue( skanChannelHdr *channelFrames, int desiredFrameNum )
{
	skanGameFrame *foundFrame;
	int i;

	foundFrame = channelFrames->ary_frames;

	for( i = 0; i < channelFrames->nFramesInChannel; i++ )
	{
		if( channelFrames->ary_frames[ i ].nFrameNum >= desiredFrameNum )
		{
			foundFrame = &channelFrames->ary_frames[ i ];
			break;
		}
	}

	return &foundFrame->pChannelData;
}


int skeletor_c::GetMorphWeightFrame( int *data )
{
	int numTargets;
	int animChannelNum;
	int blendNum;
	float weight;
	int modelChannelNum;
	vec4_t *channelData;

	numTargets = m_morphTargetList.NumChannels();

	if( !numTargets ) {
		return 0;
	}

	memset( data, 0, sizeof( *data ) * numTargets );

	for( blendNum = 0; blendNum < m_frameList.numMovementFrames; blendNum++ )
	{
		weight = m_frameList.m_blendInfo[ blendNum ].weight;

		if( weight > 0.001 )
		{
			for( modelChannelNum = 0; modelChannelNum < m_morphTargetList.NumChannels(); modelChannelNum++ )
			{
				animChannelNum = m_morphTargetList.m_chanGlobalFromLocal[ modelChannelNum ];
				animChannelNum = m_morphTargetList.GetLocalFromGlobal( animChannelNum );

				if( animChannelNum >= 0 )
				{
					channelData = DecodeFrameValue( &m_frameList.m_blendInfo[ blendNum ].pAnimationData->ary_channels[ animChannelNum ], m_frameList.m_blendInfo[ blendNum ].frame );
					data[ modelChannelNum ] += ( int )( ( *channelData )[ 0 ] * weight );
				}
			}
		}
	}

	for( blendNum = 32; blendNum < m_frameList.numActionFrames + 32; blendNum++ )
	{
		weight = m_frameList.m_blendInfo[ blendNum ].weight;

		if( weight > 0.001 )
		{
			for( modelChannelNum = 0; modelChannelNum < m_morphTargetList.NumChannels(); modelChannelNum++ )
			{
				animChannelNum = m_morphTargetList.m_chanGlobalFromLocal[ modelChannelNum ];
				animChannelNum = m_morphTargetList.GetLocalFromGlobal( animChannelNum );

				if( animChannelNum >= 0 )
				{
					channelData = DecodeFrameValue( &m_frameList.m_blendInfo[ blendNum ].pAnimationData->ary_channels[ animChannelNum ], m_frameList.m_blendInfo[ blendNum ].frame );
					data[ modelChannelNum ] += ( int )( ( *channelData )[ 0 ] * weight );
				}
			}
		}
	}

	if( m_headBoneIndex >= 0 && !VectorCompareEpsilon( m_eyeTargetPos, vec3_origin, EPSILON ) )
	{
		SkelVec3 lookPos;
		SkelVec3 temp;
		SkelMat4 headOrient;
		SkelMat4 invHeadOrient;
		float lookUpAmount;
		float lookLeftAmount;
		float l;
		float s;

		lookPos = m_eyeTargetPos;
		headOrient = GetBoneFrame( m_headBoneIndex );
		invHeadOrient.TransposeRotOf( headOrient );

		temp = lookPos;
		lookPos[ 0 ] = temp[ 0 ] * invHeadOrient[ 0 ][ 0 ] * temp[ 1 ] * invHeadOrient[ 1 ][ 0 ] + temp[ 2 ] * invHeadOrient[ 2 ][ 0 ];
		lookPos[ 1 ] = temp[ 0 ] * invHeadOrient[ 0 ][ 1 ] * temp[ 1 ] * invHeadOrient[ 1 ][ 1 ] + temp[ 2 ] * invHeadOrient[ 2 ][ 1 ];
		lookPos[ 2 ] = temp[ 0 ] * invHeadOrient[ 0 ][ 2 ] * temp[ 1 ] * invHeadOrient[ 1 ][ 2 ] + temp[ 2 ] * invHeadOrient[ 2 ][ 2 ];

		lookLeftAmount = lookPos[ 2 ] * 100 + data[ m_targetLookLeft ] - data[ m_targetLookRight ];
		lookUpAmount = lookPos[ 0 ] * 100 + data[ m_targetLookUp ] - data[ m_targetLookDown ];

		s = VectorLengthSquared( lookPos );

		if( s == 0.0 ) {
			lookPos[0 ] = 1.0;
		} else if( s != 1.0 ) {
			l = 1.0 / sqrt( s );
			VectorScale( lookPos, l, lookPos );
		}

		if( m_targetLookLeft >= 0 && m_targetLookRight >= 0
			&& m_targetLookUp >= 0 && m_targetLookDown >= 0 )
		{
			if( lookLeftAmount > 0.0 )
			{
				if( lookLeftAmount > 100.0 ) {
					lookLeftAmount = 100.0;
				}

				data[ m_targetLookLeft ] = lookLeftAmount;
				data[ m_targetLookRight ] = 0;
			}
			else
			{
				if( lookLeftAmount < -100.0 ) {
					lookLeftAmount = -100.0;
				}

				data[ m_targetLookLeft ] = 0;
				data[ m_targetLookRight ] = -lookLeftAmount;
			}

			if( m_targetLookUp > 0.0 )
			{
				if( lookUpAmount > 100.0 ) {
					lookUpAmount = 100.0;
				}

				data[ m_targetLookUp ] = lookUpAmount;
				data[ m_targetLookDown ] = 0;
			}
			else
			{
				if( lookUpAmount < -133.0 ) {
					lookUpAmount = -133.0;
				}

				data[ m_targetLookUp ] = 0.0;
				data[ m_targetLookDown ] = ( -lookUpAmount * 0.75 );
			}
		}
	}

	// check for blink
	if( m_targetBlink >= 0 )
	{
		int sysMilliseconds;
		int blinkAmount;

		sysMilliseconds = Sys_Milliseconds();

		if( sysMilliseconds > m_timeNextBlink )
		{
			if( sysMilliseconds <= m_timeNextBlink + 250 )
			{
				blinkAmount = sysMilliseconds - m_timeNextBlink;

				if( blinkAmount > 100 )
				{
					blinkAmount = 250 - blinkAmount;

					if( blinkAmount > 100 ) {
						blinkAmount = 100;
					}
				}

				if( data[ m_targetBlink ] < blinkAmount ) {
					data[ m_targetBlink ] = blinkAmount;
				}
			}
			else
			{
				m_timeNextBlink = rand() / 5 + sysMilliseconds - 1000;
			}
		}
	}

	return numTargets;
}

void skeletor_c::SetEyeTargetPos( const float *pEyeTargetPos )
{
	VectorCopy( pEyeTargetPos, m_eyeTargetPos );
}


int skeletor_c::GetBoneParent( int boneIndex ) {
	int iBoneNum;
	skelBone_Base *pBoneParent = m_bone[ boneIndex ]->Parent();

	for( iBoneNum = 0; iBoneNum < m_Tiki->m_boneList.NumChannels(); iBoneNum++ )
	{
		if( m_bone[ iBoneNum ] == pBoneParent ) {
			return iBoneNum;
		}
	}

	return -1;
	//return m_bone[ boneIndex ] - m_bone[ boneIndex ]->Parent();
}

const char *dtiki_s::GetBoneNameFromNum( int num ) const
{
	return m_boneList.ChannelName( &skeletor_c::m_boneNames, num );
}


int dtiki_s::GetBoneNumFromName( const char *name )
{
	int iGlobalChannel;

	iGlobalChannel = skeletor_c::m_boneNames.FindNameLookup( name );

	if( iGlobalChannel < 0 )
	{
		return -1;
	}

	return m_boneList.GetLocalFromGlobal( iGlobalChannel );
}
