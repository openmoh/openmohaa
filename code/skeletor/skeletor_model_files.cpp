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

// skeletor_model_files.cpp : Skeletor model files

#include "q_shared.h"
#include "qcommon.h"
#include "skeletor.h"
#include "dbgheap.h"

int CreateRotationBoneFileData( const char *newBoneName, const char *newBoneParentName, SkelVec3 basePos, boneFileData_t *fileData )
{
	char *saveChannelName;
	char *rotChannelName;
	boneData_t *boneData;

	strncpy( fileData->name, newBoneName, sizeof( fileData->name ) );
	strncpy( fileData->parent, newBoneParentName, sizeof( fileData->parent ) );
	fileData->boneType = SKELBONE_ROTATION;
	fileData->ofsBaseData = sizeof( boneFileData_t );
	boneData = ( boneData_t * )( ( char * )fileData + fileData->ofsBaseData );
	boneData->offset[ 0 ] = basePos[ 0 ];
	boneData->offset[ 1 ] = basePos[ 1 ];
	boneData->offset[ 2 ] = basePos[ 2 ];
	boneData->length = 1.0f;
	boneData->weight = 1.0f;
	boneData->bendRatio = 1.0f;
	fileData->ofsChannelNames = fileData->ofsBaseData + 24;
	saveChannelName = ( char * )( ( char * )fileData + fileData->ofsChannelNames );

	rotChannelName = ( char * )Skel_Alloc( strlen( newBoneName ) + 5 );
	ConvertToRotationName( newBoneName, rotChannelName );
	strcpy( saveChannelName, rotChannelName );
	fileData->ofsBoneNames = fileData->ofsChannelNames + ( int )strlen( rotChannelName ) + 1;
	fileData->ofsEnd = fileData->ofsBoneNames;
	return fileData->ofsEnd;
}

int CreatePosRotBoneFileData( char *newBoneName, char *newBoneParentName, boneFileData_t *fileData )
{
	int channelNamesLength;
	char *saveChannelName;
	char *rotChannelName;
	char *posChannelName;
	boneData_t *boneData;

	strncpy( fileData->name, newBoneName, sizeof( fileData->name ) );
	strncpy( fileData->parent, newBoneParentName, sizeof( fileData->parent ) );
	fileData->boneType = SKELBONE_POSROT;
	fileData->ofsBaseData = sizeof( boneFileData_t );
	boneData = ( boneData_t * )( ( char * )fileData + fileData->ofsBaseData );
	boneData->offset[ 0 ] = 1.0f;
	boneData->offset[ 1 ] = 1.0f;
	boneData->offset[ 2 ] = 1.0f;
	fileData->ofsChannelNames = fileData->ofsBaseData + 12;
	saveChannelName = ( char * )( ( char * )fileData + fileData->ofsChannelNames );

	rotChannelName = ( char * )Skel_Alloc( strlen( newBoneName ) + 5 );
	ConvertToRotationName( newBoneName, rotChannelName );
	strcpy( saveChannelName, rotChannelName );
	Skel_Free( rotChannelName );

	channelNamesLength = ( int )strlen( saveChannelName );
	saveChannelName = ( char * )( ( char * )fileData + fileData->ofsChannelNames + channelNamesLength );
	posChannelName = ( char * )Skel_Alloc( strlen( newBoneName ) + 5 );
	ConvertToPositionName( newBoneName, posChannelName );
	strcpy( saveChannelName, posChannelName );
	Skel_Free( posChannelName );

	channelNamesLength += ( int )strlen( saveChannelName );

	fileData->ofsBoneNames = fileData->ofsChannelNames + channelNamesLength + 2;
	fileData->ofsEnd = fileData->ofsBoneNames;
	return fileData->ofsEnd;
}

void CreatePosRotBoneData( const char *newBoneName, const char *newBoneParentName, boneData_t *boneData )
{
	char *rotChannelName;
	char *posChannelName;

	boneData->channel = skeletor_c::m_boneNames.RegisterChannel( newBoneName );

	if( !strcmp( newBoneParentName, "worldbone" ) )
	{
		boneData->parent = -1;
	}
	else
	{
		boneData->parent = skeletor_c::m_boneNames.RegisterChannel( newBoneParentName );
		assert( boneData->parent >= 0 );
	}

	boneData->boneType = SKELBONE_POSROT;

	rotChannelName = ( char * )Skel_Alloc( strlen( newBoneName ) + 5 );
	ConvertToRotationName( newBoneName, rotChannelName );
	boneData->channelIndex[ 0 ] = skeletor_c::m_channelNames.RegisterChannel( rotChannelName );
	if( boneData->channelIndex[ 0 ] )
	{
		SKEL_Warning( "Channel named %s not added. (Bone will not work without it)\n", rotChannelName );
		boneData->boneType = SKELBONE_ZERO;
	}
	Skel_Free( rotChannelName );

	posChannelName = ( char * )Skel_Alloc( strlen( newBoneName ) + 5 );
	ConvertToPositionName( newBoneName, posChannelName );
	boneData->channelIndex[ 1 ] = skeletor_c::m_channelNames.RegisterChannel( posChannelName );
	if( boneData->channelIndex[ 0 ] )
	{
		SKEL_Warning( "Channel named %s not added. (Bone will not work without it)\n", posChannelName );
		boneData->boneType = SKELBONE_ZERO;
	}
	Skel_Free( posChannelName );

	boneData->numChannels = 2;
	boneData->numRefs = 0;
}

int CreateIKShoulderBoneFileData( const char *newBoneName, const char *newBoneParentName, SkelQuat baseOrient, SkelVec3 basePos, boneData_t *boneData )
{
	// FIXME: stub
	return 0;
}

int CreateIKElbowBoneFileData( const char *newBoneName, const char *newBoneParentName, SkelVec3 basePos, boneData_t *boneData )
{
	// FIXME: stub
	return 0;
}

int CreateIKWristBoneFileData( const char *newBoneName, const char *newBoneParentName, const char *shoulderBoneName, SkelVec3 basePos, boneFileData_t *fileData )
{
	// FIXME: stub
	return 0;
}

int CreateHoseRotBoneFileData( char *newBoneName, char *newBoneParentName, char *targetBoneName, float bendRatio, float bendMax, float spinRatio,
	hoseRotType_t hoseRotType, SkelVec3 basePos, boneFileData_t *fileData )
{
	// FIXME: stub
	return 0;
}

int CreateAvRotBoneFileData( char *newBoneName, char *newBoneParentName, char *baseBoneName, char *targetBoneName, float rotRatio,
	SkelVec3 basePos, boneFileData_t *fileData )
{
	// FIXME: stub
	return 0;
}

void skeletor_c::LoadMorphTargetNames( skelHeaderGame_t *modelHeader )
{
	int numTargets;
	char *newTargetName;
	int i;
	int newChannel;
	int morphTargetIndex;

	numTargets = modelHeader->numMorphTargets;
	newTargetName = modelHeader->pMorphTargets;

	for( i = 0; i < numTargets; i++ )
	{
		newChannel = m_channelNames.RegisterChannel( newTargetName );
		morphTargetIndex = m_morphTargetList.AddChannel( newChannel );

		if( !strncmp( newTargetName, "EYES_left", 9 ) )
		{
			m_targetLookLeft = morphTargetIndex;
		}
		else if( !strncmp( newTargetName, "EYES_right", 10 ) )
		{
			m_targetLookRight = morphTargetIndex;
		}
		else if( !strncmp( newTargetName, "EYES_up",70 ) )
		{
			m_targetLookUp = morphTargetIndex;
		}
		else if( !strncmp( newTargetName, "EYES_down", 9 ) )
		{
			m_targetLookDown = morphTargetIndex;
		}
		else if( !strncmp( newTargetName, "EYES_blink", 9 ) )
		{
			m_targetBlink = morphTargetIndex;
		}

		newTargetName += strlen( newTargetName + 1 );
	}
}
