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

// tiki_skel.cpp : TIKI skeletor loader

#include "q_shared.h"
#include "qcommon.h"
#include "../skeletor/skeletor.h"
#include "../client/client.h"
#include "dbgheap.h"

/*
===============
LoadBoneFromBuffer2
===============
*/
void LoadBoneFromBuffer2( boneFileData_t *fileData, boneData_t *boneData )
{
	char *newChannelName;
	char *newBoneRefName;
	int i;

	memset( boneData, 0, sizeof( boneData_t ) );
	boneData->channel = skeletor_c::m_boneNames.RegisterChannel( fileData->name );
	boneData->boneType = fileData->boneType;

	if( boneData->boneType == SKELBONE_HOSEROT )
	{
		i = *( int * )&fileData->parent[ fileData->ofsBaseData + 4 ];

		if( i == 1 )
		{
			boneData->boneType = SKELBONE_HOSEROTPARENT;
		}
		else if( i == 2 )
		{
			boneData->boneType = SKELBONE_HOSEROTBOTH;
		}
	}

	newChannelName = ( char * )fileData + fileData->ofsChannelNames;
	boneData->numChannels = skelBone_Base::GetNumChannels( boneData->boneType );

	for( i = 0; i < boneData->numChannels; i++ )
	{
		boneData->channelIndex[ i ] = skeletor_c::m_channelNames.RegisterChannel( newChannelName );
		if( boneData->channelIndex[ i ] < 0 )
		{
			SKEL_Warning( "Channel named %s not added. (Bone will not work without it)\n", newChannelName );
			boneData->boneType = SKELBONE_ZERO;
		}

		newChannelName += strlen( newChannelName ) + 1;
	}

	newBoneRefName = ( char * )fileData + fileData->ofsBoneNames;
	boneData->numRefs = skelBone_Base::GetNumBoneRefs( boneData->boneType );

	for( i = 0; i < boneData->numRefs; i++ )
	{
		boneData->refIndex[ i ] = skeletor_c::m_boneNames.RegisterChannel( newBoneRefName );
		newBoneRefName += strlen( newBoneRefName ) + 1;
	}

	if( !strcmp( fileData->parent, SKEL_BONENAME_WORLD ) )
	{
		boneData->parent = -1;
	}
	else
	{
		boneData->parent = skeletor_c::m_boneNames.RegisterChannel( fileData->parent );
	}

	switch( boneData->boneType )
	{
	case SKELBONE_ROTATION:
	{
		float *baseData = ( float * )( ( char * )fileData + fileData->ofsBaseData );
		boneData->offset[ 0 ] = baseData[ 0 ];
		boneData->offset[ 1 ] = baseData[ 1 ];
		boneData->offset[ 2 ] = baseData[ 2 ];
		break;
	}
	case SKELBONE_IKSHOULDER:
	{
		float *baseData = ( float * )( ( char * )fileData + fileData->ofsBaseData );
		boneData->offset[ 0 ] = baseData[ 4 ];
		boneData->offset[ 1 ] = baseData[ 5 ];
		boneData->offset[ 2 ] = baseData[ 6 ];
		break;
	}
	case SKELBONE_IKELBOW:
	case SKELBONE_IKWRIST:
	{
		float *baseData = ( float * )( ( char * )fileData + fileData->ofsBaseData );
		boneData->length = VectorLength( baseData );
		break;
	}
	case SKELBONE_AVROT:
	{
		float *baseData = ( float * )( ( char * )fileData + fileData->ofsBaseData );
		boneData->length = baseData[ 0 ];
		boneData->offset[ 0 ] = baseData[ 1 ];
		boneData->offset[ 1 ] = baseData[ 2 ];
		boneData->offset[ 2 ] = baseData[ 3 ];
		break;
	}
	case SKELBONE_HOSEROT:
	case SKELBONE_HOSEROTBOTH:
	case SKELBONE_HOSEROTPARENT:
	{
		float *baseData = ( float * )( ( char * )fileData + fileData->ofsBaseData );
		boneData->offset[ 0 ] = baseData[ 3 ];
		boneData->offset[ 1 ] = baseData[ 4 ];
		boneData->offset[ 2 ] = baseData[ 5 ];
		boneData->bendRatio = baseData[ 0 ];
		boneData->bendMax = baseData[ 1 ];
		boneData->spinRatio = baseData[ 2 ];
		break;
	}
	default:
		break;
	}
}

/*
===============
TIKI_CacheFileSkel
===============
*/
void TIKI_CacheFileSkel( skelHeader_t *pHeader, skelcache_t *cache, int length )
{
	skelHeaderGame_t *pSkel;
	skelSurfaceGame_t *pGameSurf;
	skelSurface_t *pSurf;
	int i;
	size_t nSurfBytes;
	size_t nBoneBytes;
	size_t nBoxBytes;
	size_t nMorphBytes;
	size_t nVertBytes;
	size_t nTriBytes;
	int j;

	pSurf = ( skelSurface_t * )( ( byte * )pHeader + pHeader->ofsSurfaces );
	nSurfBytes = 0;

	for( i = 0; i < pHeader->numSurfaces; i++ )
	{
		skeletorVertex_t *pVert = ( skeletorVertex_t * )( ( byte * )pSurf + pSurf->ofsVerts );

		nVertBytes = 0;
		for( j = 0; j < pSurf->numVerts; j++ )
		{
			int iOffset = sizeof( skeletorMorph_t ) * pVert->numMorphs + sizeof( skelWeight_t ) * pVert->numWeights + sizeof( skeletorVertex_t );
			nVertBytes += iOffset;
			pVert = ( skeletorVertex_t * )( ( byte * )pVert + iOffset );
		}

		nSurfBytes += sizeof( skelSurfaceGame_t ) + sizeof( skelIndex_t ) * pSurf->numVerts + nVertBytes + pSurf->numTriangles * sizeof( skelIndex_t ) * 3;
		if( pHeader->version > TIKI_SKB_HEADER_VER_3 )
		{
			nSurfBytes += sizeof( skelIndex_t ) * pSurf->numVerts;
		}

		pSurf = ( skelSurface_t * )( ( byte * )pSurf + pSurf->ofsEnd );
	}

	nBoneBytes = pHeader->numBones * sizeof( boneData_t );
	nBoxBytes = 0;
	nMorphBytes = 0;

	if( pHeader->version > TIKI_SKB_HEADER_VERSION )
	{
		char *pMorphTargets;
		intptr_t nLen;

		nBoxBytes = pHeader->numBoxes * sizeof( skelHitBox_t );
		pMorphTargets = ( char * )pHeader + pHeader->ofsMorphTargets;

		if( pHeader->ofsMorphTargets > 0 || ( pHeader->ofsMorphTargets + pHeader->numMorphTargets ) < length )
		{
			for( i = 0; i < pHeader->numMorphTargets; i++ )
			{
				nLen = strlen( pMorphTargets ) + 1;
				nMorphBytes += nLen;
				pMorphTargets += nLen;
			}
		}
		else
		{
			nMorphBytes = pHeader->numMorphTargets;
		}
	}
	else if( pHeader->version == TIKI_SKB_HEADER_VERSION )
	{
		nBoxBytes = pHeader->numBoxes * sizeof( skelHitBox_t );
	}

	cache->size = sizeof( skelHeaderGame_t ) + nMorphBytes + nBoneBytes + nBoxBytes + nSurfBytes;
	cache->skel = pSkel = ( skelHeaderGame_t * )TIKI_Alloc( cache->size );
	pSkel->version = pHeader->version;
	pSkel->numSurfaces = pHeader->numSurfaces;
	pSkel->numBones = pHeader->numBones;
	pSkel->pSurfaces = ( skelSurfaceGame_t * )( ( byte * )pSkel + sizeof( skelHeaderGame_t ) );
	pSkel->pBones = ( boneData_t * )( ( byte * )pSkel->pSurfaces + nSurfBytes );
	pSkel->numBoxes = pHeader->numBoxes;
	pSkel->pBoxes = ( skelHitBox_t * )( ( byte * )pSkel->pBones + nBoneBytes );
	pSkel->pLOD = NULL;
	pSkel->numMorphTargets = pHeader->numMorphTargets;
	pSkel->pMorphTargets = ( char * )( ( byte * )pSkel->pBoxes + nBoxBytes );
	memcpy( pSkel->name, pHeader->name, sizeof( pSkel->name ) );
	memcpy( pSkel->lodIndex, pHeader->lodIndex, sizeof( pSkel->lodIndex ) );

	pSurf = ( skelSurface_t * )( ( byte * )pHeader + pHeader->ofsSurfaces );
	pGameSurf = pSkel->pSurfaces;

	i = 0;
	while( 1 )
	{
		size_t nBytesUsed;
		skeletorVertex_t *pVert;

		nTriBytes = pSurf->numTriangles * sizeof( skelIndex_t ) * 3;
		nSurfBytes = 0;
		if( pHeader->version > TIKI_SKB_HEADER_VER_3 ) {
			nSurfBytes = pSurf->numVerts * sizeof( skelIndex_t );
		}

		pVert = ( skeletorVertex_t * )( ( byte * )pSurf + pSurf->ofsVerts );

		nVertBytes = 0;
		for( j = 0; j < pSurf->numVerts; j++ )
		{
			int iOffset = sizeof( skeletorMorph_t ) * pVert->numMorphs + sizeof( skelWeight_t ) * pVert->numWeights + sizeof( skeletorVertex_t );
			nVertBytes += iOffset;
			pVert = ( skeletorVertex_t * )( ( byte * )pVert + iOffset );
		}

		nBytesUsed = sizeof( skelSurfaceGame_t ) + nSurfBytes + nVertBytes + nTriBytes;
		pGameSurf->ident = pSurf->ident;
		pGameSurf->numTriangles = pSurf->numTriangles;
		pGameSurf->numVerts = pSurf->numVerts;
		pGameSurf->pStaticXyz = NULL;
		pGameSurf->pStaticNormal = NULL;
		pGameSurf->pStaticTexCoords = NULL;
		pGameSurf->pTriangles = ( skelIndex_t * )( ( byte * )pGameSurf + sizeof( skelSurfaceGame_t ) );
		pGameSurf->pVerts = ( skeletorVertex_t * )( ( byte * )pGameSurf->pTriangles + nTriBytes );
		pGameSurf->pCollapse = ( skelIndex_t * )( ( byte * )pGameSurf->pVerts + nVertBytes );
		pGameSurf->pCollapseIndex = ( skelIndex_t * )( ( byte * )pGameSurf->pCollapse + sizeof( skelIndex_t ) * pSurf->numVerts );
		memcpy( pGameSurf->name, pSurf->name, sizeof( pGameSurf->name ) );

		if( pGameSurf->numTriangles )
		{
			for( j = 0; j < pSurf->numTriangles * 3; j++ )
			{
				pGameSurf->pTriangles[ j ] = *( ( int * )( ( byte * )pSurf + pSurf->ofsTriangles ) + j );
			}
		}
		else
		{
			pGameSurf->pTriangles = NULL;
		}

		if( pGameSurf->numVerts )
		{
			/*pVert = ( skeletorVertex_t * )( ( char * )pSurf + pSurf->ofsVerts );

			for( j = 0; j < pGameSurf->numVerts; j++ )
			{
				int iOffset = sizeof( skeletorMorph_t ) * pVert->numMorphs + sizeof( skelWeight_t ) * pVert->numWeights + sizeof( skeletorVertex_t );
				pGameSurf->pVerts[ j ].normal[ 0 ] = pVert->normal[ 0 ];
				pGameSurf->pVerts[ j ].normal[ 1 ] = pVert->normal[ 1 ];
				pGameSurf->pVerts[ j ].normal[ 2 ] = pVert->normal[ 2 ];
				pVert = ( skeletorVertex_t * )( ( char * )pVert + iOffset );
			}*/

			memcpy( pGameSurf->pVerts, ( ( byte * )pSurf + pSurf->ofsVerts ), nVertBytes );

			for( j = 0; j < pSurf->numVerts; j++ )
			{
				pGameSurf->pCollapse[ j ] = *( ( int * )( ( byte * )pSurf + pSurf->ofsCollapse ) + j );
			}

			if( pHeader->version > TIKI_SKB_HEADER_VER_3 )
			{
				for( j = 0; j < pSurf->numVerts; j++ )
				{
					pGameSurf->pCollapseIndex[ j ] = *( ( int * )( ( byte * )pSurf + pSurf->ofsCollapseIndex ) + j );
				}
			}
			else
			{
				pGameSurf->pCollapseIndex = NULL;
			}
		}
		else
		{
			pGameSurf->pVerts = NULL;
			pGameSurf->pCollapse = NULL;
			pGameSurf->pCollapseIndex = NULL;
		}

		i++;
		if( i >= pHeader->numSurfaces )
			break;

		pGameSurf->pNext = ( skelSurfaceGame_t * )( ( byte * )pGameSurf + nBytesUsed );
		pGameSurf = pGameSurf->pNext;
		pSurf = ( skelSurface_t * )( ( byte * )pSurf + pSurf->ofsEnd );
	}

	pGameSurf->pNext = NULL;

	if( nBoneBytes )
	{
		if( pHeader->version <= TIKI_SKB_HEADER_VERSION )
		{
			skelBoneName_t *TIKI_bones = ( skelBoneName_t * )( ( byte * )pHeader + pHeader->ofsBones );
			for( i = 0; i < pSkel->numBones; i++ )
			{
				const char *boneName;

				if( TIKI_bones->parent == -1 )
				{
					boneName = SKEL_BONENAME_WORLD;
				}
				else
				{
					boneName = TIKI_bones[ TIKI_bones->parent ].name;
				}

				CreatePosRotBoneData( TIKI_bones->name, boneName, &pSkel->pBones[ i ] );
				TIKI_bones++;
			}
		}
		else
		{
			boneFileData_t *boneBuffer = ( boneFileData_t * )( ( byte * )pHeader + pHeader->ofsBones );
			for( i = 0; i < pSkel->numBones; i++ )
			{
				LoadBoneFromBuffer2( boneBuffer, &pSkel->pBones[ i ] );
				boneBuffer = ( boneFileData_t * )( ( byte * )boneBuffer + boneBuffer->ofsEnd );
			}
		}
	}
	else
	{
		pSkel->numBones = 0;
		pSkel->pBones = NULL;
	}

	if( pHeader->version <= TIKI_SKB_HEADER_VER_3 )
	{
		pSkel->numBoxes = 0;
		pSkel->pBoxes = NULL;
		pSkel->numMorphTargets = 0;
		pSkel->pMorphTargets = NULL;
		return;
	}

	if( nBoxBytes )
	{
		if( pHeader->ofsBoxes <= 0 || ( nBoxBytes + pHeader->ofsBoxes ) >= length )
		{
			Com_Printf( "^~^~^ Box data is corrupted for '%s'\n", cache->path );
			pSkel->numMorphTargets = 0;
			pSkel->pMorphTargets = NULL;
		}
		else
		{
			memcpy( pSkel->pBoxes, ( ( byte * )pHeader + pHeader->ofsBoxes ), nBoxBytes );
		}
	}
	else
	{
		pSkel->numBoxes = 0;
		pSkel->pBoxes = NULL;
	}

	if( pHeader->version <= TIKI_SKB_HEADER_VERSION )
	{
		pSkel->numMorphTargets = 0;
		pSkel->pMorphTargets = NULL;
		return;
	}

	if( nMorphBytes )
	{
		if( pHeader->ofsMorphTargets <= 0 || ( nMorphBytes + pHeader->ofsMorphTargets ) >= length )
		{
			Com_Printf( "^~^~^ Morph targets data is corrupted for '%s'\n", cache->path );
			pSkel->numMorphTargets = 0;
			pSkel->pMorphTargets = NULL;
		}
		else
		{
			memcpy( pSkel->pMorphTargets, ( ( byte * )pHeader + pHeader->ofsMorphTargets ), nMorphBytes );
		}
	}
	else
	{
		pSkel->numMorphTargets = 0;
		pSkel->pMorphTargets = NULL;
	}
}

/*
===============
TIKI_SortLOD
===============
*/
void TIKI_SortLOD( skelHeaderGame_t *skelmodel )
{
	skelSurfaceGame_t *surf;
	int render_count;
	int nTriVerts;
	int i;
	int collapse[ TIKI_MAX_VERTEXES ];
	int iTemp;

	for( surf = skelmodel->pSurfaces; surf != NULL; surf = surf->pNext )
	{
		for( i = 0; i < surf->numVerts; i++ )
			collapse[ i ] = i;

		render_count = surf->numTriangles * 3;

		nTriVerts = surf->numVerts - 1;
		while( nTriVerts > 1 )
		{
			for( i = nTriVerts; i > 1; i-- )
			{
				if( surf->pCollapseIndex[ i - 1 ] != surf->pCollapseIndex[ i ] )
					break;
			}

			nTriVerts = i - 1;
			for( ; i < surf->numVerts; i++ )
			{
				collapse[ i ] = collapse[ surf->pCollapse[ i ] ];
			}

			i = 0;
			while( i < render_count )
			{
				if( collapse[ surf->pTriangles[ i ] ] != collapse[ surf->pTriangles[ i + 1 ] ] &&
					collapse[ surf->pTriangles[ i + 1 ] ] != collapse[ surf->pTriangles[ i + 2 ] ] &&
					collapse[ surf->pTriangles[ i + 2 ] ] != collapse[ surf->pTriangles[ i ] ] )
				{
					i += 3;
				}
				else
				{
					render_count -= 3;

					iTemp = surf->pTriangles[ i ];
					surf->pTriangles[ i ] = surf->pTriangles[ render_count ];
					surf->pTriangles[ render_count ] = iTemp;

					iTemp = surf->pTriangles[ i + 1 ];
					surf->pTriangles[ i + 1 ] = surf->pTriangles[ render_count + 1 ];
					surf->pTriangles[ render_count + 1 ] = iTemp;

					iTemp = surf->pTriangles[ i + 2 ];
					surf->pTriangles[ i + 2 ] = surf->pTriangles[ render_count + 2 ];
					surf->pTriangles[ render_count + 2 ] = iTemp;
				}
			}
		}
	}
}

/*
===============
TIKI_LoadSKB
===============
*/
qboolean TIKI_LoadSKB( const char *path, skelcache_t *cache )
{
	int i, j, k;
	skelHeader_t *pheader;
	skelSurface_t *surf;
	int version;
	unsigned int header;
	int length;
	char *buf;
	int totalVerts;
	int newLength;
	skelHeader_t *newHeader;
	skelSurface_t *oldSurf;
	skelSurface_t *newSurf;

	length = TIKI_ReadFileEx( path, ( void ** )&buf, true );
	if( length < 0 )
	{
		TIKI_DPrintf( "Tiki:LoadAnim Couldn't load %s\n", path );
		return qfalse;
	}

	pheader = ( skelHeader_t * )TIKI_Alloc( length );
	memcpy( pheader, buf, length );
	TIKI_FreeFile( buf );
	memset( cache, 0, sizeof( skelcache_t ) );
	strncpy( cache->path, path, sizeof( cache->path ) );

	header = LittleLong( pheader->ident );
	if( header != TIKI_SKB_HEADER_IDENT )
	{
		TIKI_Error( "TIKI_LoadSKB: Tried to load '%s' as a skeletal base frame (File has invalid header)\n", path );
		TIKI_Free( pheader );
		return qfalse;
	}

	version = LittleLong( pheader->version );
	if( version != TIKI_SKB_HEADER_VER_3 && version != TIKI_SKB_HEADER_VERSION )
	{
		TIKI_Error( "TIKI_LoadSKB: %s has wrong version (%i should be %i or %i)\n", path, version, TIKI_SKB_HEADER_VER_3, TIKI_SKB_HEADER_VERSION );
		TIKI_Free( pheader );
		return qfalse;
	}

	surf = ( skelSurface_t * )( ( char * )pheader + pheader->ofsSurfaces );
	for( i = 0; i < pheader->numSurfaces; i++ )
	{
		if( surf->numVerts > TIKI_MAX_VERTEXES )
		{
			TIKI_Error( "TIKI_LoadSKB: %s has more than %i verts on a surface (%i)", path, TIKI_MAX_VERTEXES, surf->numVerts );
			TIKI_Free( pheader );
			return qfalse;
		}

		if( surf->numTriangles > TIKI_MAX_TRIANGLES )
		{
			TIKI_Error( "TIKI_LoadSKB: %s has more than %i triangles on a surface (%i)", path, TIKI_MAX_TRIANGLES, surf->numTriangles );
			TIKI_Free( pheader );
			return qfalse;
		}

		surf = ( skelSurface_t * )( ( char * )surf + surf->ofsEnd );
	}

	if( pheader->numBones > TIKI_MAX_BONES )
	{
		TIKI_Error( "TIKI_LoadSKB: %s has more than %i bones (%i)\n", path, TIKI_MAX_BONES, pheader->numBones );
		TIKI_Free( pheader );
		return qfalse;
	}

	totalVerts = 0;
	surf = ( skelSurface_t * )( ( byte * )pheader + pheader->ofsSurfaces );
	for( i = 0; i < pheader->numSurfaces; i++ )
	{
		totalVerts += surf->numVerts;
		surf = ( skelSurface_t * )( ( byte * )surf + surf->ofsEnd );
	}

	newLength = totalVerts * sizeof( unsigned int );
	newHeader = ( skelHeader_t * )TIKI_Alloc( length + newLength );
	memcpy( newHeader, pheader, pheader->ofsSurfaces );

	if( newHeader->ofsBones > newHeader->ofsSurfaces ) newHeader->ofsBones += newLength;
	if( newHeader->ofsBoxes > newHeader->ofsSurfaces ) newHeader->ofsBoxes += newLength;
	if( newHeader->ofsEnd > newHeader->ofsSurfaces ) newHeader->ofsEnd += newLength;

	oldSurf = ( skelSurface_t * )( ( byte * )pheader + pheader->ofsSurfaces );
	newSurf = ( skelSurface_t * )( ( byte * )newHeader + newHeader->ofsSurfaces );

	for( i = 0; i < pheader->numSurfaces; i++ )
	{
		skeletorVertex_t *newVerts;
		skelVertex_t *oldVerts;

		memcpy( newSurf, oldSurf, oldSurf->ofsVerts );
		if( newSurf->ofsCollapse > newSurf->ofsVerts ) newSurf->ofsCollapse += sizeof( unsigned int ) * newSurf->numVerts;
		if( newSurf->ofsCollapseIndex > newSurf->ofsVerts ) newSurf->ofsCollapse += sizeof( unsigned int ) * newSurf->numVerts;
		if( newSurf->ofsTriangles > newSurf->ofsVerts ) newSurf->ofsTriangles += sizeof( unsigned int ) * newSurf->numVerts;
		if( newSurf->ofsEnd > newSurf->ofsVerts ) newSurf->ofsEnd += sizeof( unsigned int ) * newSurf->numVerts;

		oldVerts = ( skelVertex_t * )( ( byte * )oldSurf + oldSurf->ofsVerts );
		newVerts = ( skeletorVertex_t * )( ( byte * )newSurf + newSurf->ofsVerts );

		for( j = 0; j < oldSurf->numVerts; j++ )
		{
			VectorCopy( oldVerts->normal, newVerts->normal );
			newVerts->texCoords[ 0 ] = oldVerts->texCoords[ 0 ];
			newVerts->texCoords[ 1 ] = oldVerts->texCoords[ 1 ];
			newVerts->numMorphs = 0;
			newVerts->numWeights = oldVerts->numWeights;

			skelWeight_t *newWeights = ( skelWeight_t * )( ( byte * )newVerts + sizeof( skeletorVertex_t ) );

			for( k = 0; k < oldVerts->numWeights; k++ )
			{
				memcpy( newWeights, &oldVerts->weights[ k ], sizeof( skelWeight_t ) );
				newWeights++;
			}

			oldVerts = ( skelVertex_t * )( ( byte * )oldVerts + sizeof( skelWeight_t ) * oldVerts->numWeights + ( sizeof( skelVertex_t ) - sizeof( skelWeight_t ) ) );
			newVerts = ( skeletorVertex_t * )( ( byte * )newVerts + sizeof( skeletorVertex_t ) + sizeof( skelWeight_t ) * newVerts->numWeights );
		}

		memcpy( newVerts, oldVerts, oldSurf->ofsEnd - ( ( byte * )oldVerts - ( byte * )oldSurf ) );
		oldSurf = ( skelSurface_t * )( ( byte * )oldSurf + oldSurf->ofsEnd );
		newSurf = ( skelSurface_t * )( ( byte * )newSurf + newSurf->ofsEnd );
	}

	memcpy( newSurf, oldSurf, pheader->ofsEnd - ( ( char * )oldSurf - ( char * )pheader ) );
	TIKI_Free( pheader );
	TIKI_CacheFileSkel( newHeader, cache, length );
	TIKI_Free( newHeader );
	cache_numskel++;

	return qtrue;
}

/*
===============
SaveLODFile
===============
*/
void SaveLODFile( const char *path, lodControl_t *LOD )
{
	fileHandle_t file = FS_FOpenFileWrite( path );
	if( !file )
	{
		TIKI_Warning( "SaveLODFile: Failed to open file %s\n", path );
		return;
	}

	FS_Write( LOD, sizeof( lodControl_t ), file );
}

/*
===============
GetLODFile
===============
*/
void GetLODFile( skelcache_t *cache )
{
	lodControl_t *LOD;
	char pathLOD[ 256 ];
	char *buf;
	int i;
	skelSurfaceGame_t *pSurf;
	bool bCanLod = false;
	char *ext;
	int length;

	pSurf = cache->skel->pSurfaces;

	for( i = 0; i < cache->skel->numSurfaces; i++ )
	{
		if( pSurf->pCollapseIndex[ 0 ] != pSurf->pCollapseIndex[ pSurf->numVerts - 1 ] )
		{
			bCanLod = true;
			break;
		}

		pSurf = pSurf->pNext;
	}

	if( !bCanLod )
	{
		return;
	}

	strcpy( pathLOD, cache->path );
	ext = strstr( pathLOD, "skd" );
	strcpy( ext, "lod" );

	length = TIKI_ReadFileEx( pathLOD, ( void ** )&buf, true );
	if( length >= 0 )
	{
		LOD = ( lodControl_t * )TIKI_Alloc( sizeof( lodControl_t ) );
		memcpy( LOD, buf, length );
		TIKI_FreeFile( buf );
	}
	else
	{
		LOD = ( lodControl_t * )TIKI_Alloc( sizeof( lodControl_t ) );
		LOD->minMetric = 1.0f;
		LOD->maxMetric = 0.2f;
		LOD->curve[ 0 ].pos = 0.0f;
		LOD->curve[ 0 ].val = 0.0f;
		LOD->curve[ 1 ].pos = 0.5f;
		LOD->curve[ 1 ].val = ( float )cache->skel->lodIndex[ 1 ];
		LOD->curve[ 2 ].pos = 0.8f;
		LOD->curve[ 2 ].val = ( float )cache->skel->lodIndex[ 1 ];
		LOD->curve[ 3 ].pos = 0.95f;
		LOD->curve[ 3 ].val = ( float )cache->skel->lodIndex[ 1 ];
		LOD->curve[ 4 ].pos = 1.0f;

		for( i = 9; cache->skel->lodIndex[ i ] > cache->skel->lodIndex[ 3 ]; i-- )
		{
			if( i <= 2 )
				break;
		}

		LOD->curve[ 4 ].val = ( float )cache->skel->lodIndex[ i ];

		for( i = 8; i >= 0; i-- )
		{
			if( LOD->curve[ i - 1 ].pos > LOD->curve[ i ].pos )
				LOD->curve[ i ].val = LOD->curve[ i - 1 ].pos;
		}
	}

	TIKI_CalcLodConsts( LOD );
	cache->skel->pLOD = LOD;
}

/*
===============
TIKI_CalcLodConsts
===============
*/
void TIKI_CalcLodConsts( lodControl_t *LOD )
{
	int i;

	for( i = 0; i < 4; i++ )
	{
		float common = ( LOD->curve[ i + 1 ].val - LOD->curve[ i + 0 ].val ) / ( LOD->curve[ i + 1 ].pos - LOD->curve[ i + 0 ].pos );

		LOD->consts[ i ].base = LOD->curve[ i + 0 ].val + ( LOD->minMetric / ( LOD->minMetric - LOD->maxMetric ) - LOD->curve[ i + 0 ].pos ) * common;
		LOD->consts[ i ].scale = common / ( LOD->maxMetric - LOD->minMetric );
		LOD->consts[ i ].cutoff = LOD->minMetric + ( LOD->maxMetric - LOD->minMetric ) * LOD->curve[ i + 0 ].pos;
	}
}

/*
===============
TIKI_LoadSKD
===============
*/
qboolean TIKI_LoadSKD( const char *path, skelcache_t *cache )
{
	int i;
	skelHeader_t *pheader;
	skelSurface_t *surf;
	int version;
	unsigned int header;
	int length;

	length = TIKI_ReadFileEx( path, ( void ** )&pheader, qtrue );
	if( length < 0 )
	{
		TIKI_DPrintf( "Tiki:LoadAnim Couldn't load %s\n", path );
		return qfalse;
	}

	// The SKD must have at least one surface
	if( pheader->numSurfaces <= 0 )
	{
		TIKI_Error( "^~^~^ TIKI_LoadSKD: %s has no surfaces\n", path );
		TIKI_FreeFile( pheader );
		return qfalse;
	}

	memset( cache, 0, sizeof( skelcache_t ) );
	strncpy( cache->path, path, sizeof( cache->path ) );

	// Check the signature
	header = LittleLong( pheader->ident );
	if( header != TIKI_SKD_HEADER_IDENT )
	{
		TIKI_Error( "TIKI_LoadSKD: Tried to load '%s' as a skeletal base frame (File has invalid header)\n", path );
		TIKI_FreeFile( pheader );
		return qfalse;
	}

	// Check the version
	version = LittleLong( pheader->version );
	if( version != TIKI_SKD_HEADER_OLD_VERSION && version != TIKI_SKD_HEADER_VERSION )
	{
		TIKI_Error( "TIKI_LoadSKD: %s has wrong version (%i should be %i)\n", path, version, TIKI_SKD_HEADER_VERSION );
		TIKI_FreeFile( pheader );
		return qfalse;
	}

	surf = ( skelSurface_t * )( ( byte * )pheader + pheader->ofsSurfaces );

	for( i = 0; i < pheader->numSurfaces; i++ )
	{
		if( surf->numVerts > TIKI_MAX_VERTEXES )
		{
			TIKI_Error( "TIKI_LoadSKD: %s has more than %i verts on a surface (%i)", path, TIKI_MAX_VERTEXES, surf->numVerts );
			TIKI_FreeFile( pheader );
			return qfalse;
		}

		if( surf->numTriangles > TIKI_MAX_TRIANGLES )
		{
			TIKI_Error( "TIKI_LoadSKD: %s has more than %i triangles on a surface (%i)", path, TIKI_MAX_TRIANGLES, surf->numTriangles );
			TIKI_FreeFile( pheader );
			return qfalse;
		}

		surf = ( skelSurface_t * )( ( byte * )surf + surf->ofsEnd );
	}

	if( pheader->numBones > TIKI_MAX_BONES )
	{
		TIKI_FreeFile( pheader );
		TIKI_Error( "TIKI_LoadSKD: %s has more than %i bones (%i)\n", path, TIKI_MAX_BONES, pheader->numBones );
		return qfalse;
	}

	// Cache the skeleton
	TIKI_CacheFileSkel( pheader, cache, length );
	TIKI_FreeFile( pheader );
	cache_numskel++;

	// Get the lod file associated with the skeleton
	GetLODFile( cache );
	TIKI_SortLOD( cache->skel );
	return 1;
}

/*
===============
TIKI_GetSkel
===============
*/
skelHeaderGame_t *TIKI_GetSkel( int index )
{
	return skelcache[ index ].skel;
}

/*
===============
TIKI_GetSkelCache
===============
*/
int TIKI_GetSkelCache( skelHeaderGame_t *pSkel )
{
	for( int index = 0; index < cache_maxskel; index++ )
	{
		if( skelcache[ index ].skel == pSkel )
		{
			return index;
		}
	}

	return -1;
}

/*
===============
TIKI_FreeSkel
===============
*/
void TIKI_FreeSkel( int index )
{
	if( index < 0 || index > cache_maxskel )
	{
		TIKI_Error( "TIKI_FreeSkel: handle %d out of range\n", index );
		return;
	}

	if( !skelcache[ index ].skel )
	{
		TIKI_Error( "TIKI_FreeSkel: NULL pointer for %s\n", skelcache[ index ].path );
		return;
	}

	TIKI_FreeSkelCache( &skelcache[ index ] );
	cache_numskel--;
}

/*
===============
TIKI_FreeSkelCache
===============
*/
void TIKI_FreeSkelCache( skelcache_t *cache )
{
	skelSurfaceGame_t *pSurf;

	if( cache->skel == NULL ) {
		return;
	}

	for( pSurf = cache->skel->pSurfaces; pSurf != NULL; pSurf = pSurf->pNext )
	{
		if( pSurf->pStaticXyz ) {
			TIKI_Free( pSurf->pStaticXyz );
		}
	}

	if( cache->skel->pLOD ) {
		TIKI_Free( cache->skel->pLOD );
	}

	TIKI_Free( cache->skel );
	cache->skel = NULL;
	cache->size = 0;
	cache->path[0] = 0;
	cache->numuses = 0;
}

/*
===============
TIKI_FindSkel
===============
*/
skelcache_t *TIKI_FindSkel( const char *path )
{
	int i;
	skelcache_t *cache;

	for( i = 0; i < TIKI_MAX_SKELCACHE; i++ )
	{
		cache = &skelcache[ i ];

		if( !cache->skel )
			continue;

		if( !strcmp( path, cache->path ) )
			return cache;
	}

	return NULL;
}

/*
===============
TIKI_FindFreeSkel
===============
*/
skelcache_t *TIKI_FindFreeSkel( void )
{
	int i;
	skelcache_t *cache;

	for( i = 0; i < TIKI_MAX_SKELCACHE; i++ )
	{
		cache = &skelcache[ i ];

		if( !cache->skel )
			return cache;
	}

	return NULL;
}

/*
===============
TIKI_RegisterSkel
===============
*/
int TIKI_RegisterSkel( const char *path, dtiki_t *tiki )
{
	char tempName[ 257 ];
	skelcache_t *cache;
	const char *extension;

	// Find a skel with the same name
	cache = TIKI_FindSkel( path );
	if( cache )
	{
		return cache - skelcache;
	}

	// Find a free skel
	cache = TIKI_FindFreeSkel();
	if( !cache )
	{
		TIKI_Error( "TIKI_RegisterSkel: No free spots open in skel cache for %s\n", path );
		return -1;
	}

	if( cache - skelcache + 1 > cache_maxskel )
	{
		cache_maxskel = cache - skelcache + 1;
	}

	extension = TIKI_FileExtension( path );
	if( !strcmp( extension, "skb" ) )
	{
		if( !TIKI_LoadSKB( path, cache ) )
			return -1;
	}
	else if( !strcmp( extension, "skd" ) )
	{
		if( !TIKI_LoadSKD( path, cache ) )
			return -1;
	}

	sprintf( tempName, "l%s", path );
	UI_LoadResource( tempName );

	return cache - skelcache;
}
