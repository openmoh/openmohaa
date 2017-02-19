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

// tiki_files.cpp : TIKI File Loader

#include "q_shared.h"
#include "qcommon.h"
#include "../skeletor/skeletor.h"
#include "../skeletor/tokenizer.h"
#include "../client/client.h"
#include <tiki.h>
#include <mem_tempalloc.h>
#include "dbgheap.h"

qboolean	tiki_loading;
cvar_t		*dumploadedanims;
cvar_t		*low_anim_memory;
cvar_t		*showLoad;
cvar_t		*convertAnims;

typedef struct {
	char path[ 100 ];
	skelAnimDataGameHeader_t *data;
	int numusers;
	int lookup;
} skeletorCacheEntry_t;

static int m_numInCache;
class InitSkelCache {
public:
	static class InitSkelCache init;

	InitSkelCache();
};

static int m_cachedDataLookup[ 4095 ];
static skeletorCacheEntry_t m_cachedData[ 4095 ];
InitSkelCache InitSkelCache::init;
MEM_TempAlloc TIKI_allocator;
dloaddef_t loaddef;

/*
===============
InitSkelCache::InitSkelCache
===============
*/
InitSkelCache::InitSkelCache()
{
	int i;

	for( i = 0; i < 4095; i++ )
	{
		m_cachedData[ i ].lookup = -1;
	}
}

/*
===============
TIKI_FreeStorage
===============
*/
void TIKI_FreeStorage( dloaddef_t *ld )
{
	TIKI_allocator.FreeAll();
	ld->tikiFile.Close();
}

/*
===============
TIKI_AllocateLoadData
===============
*/
void *TIKI_AllocateLoadData( size_t length )
{
	return TIKI_allocator.Alloc( length );
}

/*
===============
TIKI_CopyString
===============
*/
char *TIKI_CopyString( const char *s )
{
	char *result = ( char * )TIKI_AllocateLoadData( strlen( s ) + 1 );
	strcpy( result, s );
	return result;
}

/*
===============
TIKI_LoadTikiAnim
===============
*/
qboolean loadtikicommands = true;
dtikianim_t *TIKI_LoadTikiAnim( const char *path )
{
	dtikianim_t *tiki = NULL;
	const char *token;
	float tempVec[ 3 ];
	msg_t modelBuf;
	str s;
	char tempName[ 257 ];

	memset( &loaddef, 0, sizeof( dloaddef_t ) );
	loaddef.modelBuf = &modelBuf;

	TIKI_InitSetup( &loaddef );

	if( loaddef.tikiFile.LoadFile( path, qfalse ) )
	{
		loaddef.path = path;

		token = loaddef.tikiFile.GetToken( true );
		if( strcmp( token, "TIKI" ) )
		{
			TIKI_Error( "TIKI_LoadTIKIfile: def file %s has wrong header (%s should be TIKI)\n", loaddef.tikiFile.Filename(), token );
			loaddef.tikiFile.Close();
			return NULL;
		}

		loaddef.numanims = 0;
		loaddef.numserverinitcmds = 0;
		loaddef.numclientinitcmds = 0;
		loaddef.bInIncludesSection = false;

		while( loaddef.tikiFile.TokenAvailable( true ) )
		{
			token = loaddef.tikiFile.GetToken( true );

			if( !stricmp( token, "setup" ) )
			{
				if( !TIKI_ParseSetup( &loaddef ) )
				{
					TIKI_FreeStorage( &loaddef );
					return NULL;
				}
			}
			else if( !stricmp( token, "init" ) )
			{
				TIKI_ParseInit( &loaddef );
			}
			else if( !stricmp( token, "animations" ) )
			{
				TIKI_ParseAnimations( &loaddef );
			}
			else if( !stricmp( token, "includes" ) )
			{
				if( !loaddef.bInIncludesSection )
				{
					loaddef.bInIncludesSection = TIKI_ParseIncludes( &loaddef );
				}
				else
				{
					TIKI_Error( "TIKI_LoadTIKIfile: Nested Includes section in %s on line %d, the animations will be fubar\n",
						token,
						loaddef.tikiFile.GetLineNumber(),
						loaddef.tikiFile.Filename() );
				}
			}
			else if( !stricmp( token, "}" ) && loaddef.bInIncludesSection )
			{
				loaddef.bInIncludesSection = false;
			}
			else
			{
				TIKI_Error( "TIKI_LoadTIKIfile: unknown section %s in %s online %d, skipping line.\n", token, loaddef.tikiFile.Filename(), loaddef.tikiFile.GetLineNumber() );

				// skip the current line
				while( loaddef.tikiFile.TokenAvailable( false ) ) {
					loaddef.tikiFile.GetToken( false );
				}
			}
		}

		if( loaddef.bInIncludesSection )
		{
			TIKI_Error( "TIKI_LoadTIKIfile: Include section in %s did not terminate\n", loaddef.tikiFile.Filename() );
		}

		if( loaddef.numanims )
		{
			sprintf( tempName, "a%s", path );
			UI_LoadResource( tempName );

			tiki = TIKI_FillTIKIStructureSkel( &loaddef );
			if( tiki )
			{
				sprintf( tempName, "b%s", path );
				UI_LoadResource( tempName );
				sprintf( tempName, "c%s", path );
				UI_LoadResource( tempName );

				VectorSubtract( tiki->maxs, tiki->mins, tempVec );
				if( VectorLength( tempVec ) > 100000.0f )
				{
					VectorSet( tiki->mins, -4.0f, -4.0f, -4.0f );
					VectorSet( tiki->maxs, 4.0f, 4.0f, 4.0f );
				}

				TIKI_FreeStorage( &loaddef );
				sprintf( tempName, "d%s", path );
				UI_LoadResource( tempName );
			}
			else
			{
				TIKI_FreeStorage( &loaddef );
			}
		}
		else
		{
			TIKI_Error( "TIKI_LoadTIKIfile: No valid animations found in %s.\n", loaddef.tikiFile.Filename() );
			TIKI_FreeStorage( &loaddef );
		}
	}
	else
	{
		loaddef.tikiFile.Close();
	}

	return tiki;
}

/*
===============
TIKI_LoadTikiModel
===============
*/
dtiki_t *TIKI_LoadTikiModel( dtikianim_t *tikianim, const char *name, con_map<str, str> *keyValues )
{
	dtiki_t *tiki;
	//byte *start_ptr;
	//byte *ptr;
	int i, j, k;
	char *strptr;
	size_t defsize;
	struct {
		dtiki_t tiki;
		short int buffer[ 1000 ];
	} temp;
	//int skel;
	dloadsurface_t loadsurfaces[ 24 ];
	int numSurfacesSetUp;
	dtikisurface_t *tikiSurf;
	dloadsurface_t *loadsurf;
	int mesh;
	skelHeaderGame_t *skelmodel;
	skelSurfaceGame_t *surf;
	int surfOffset;
	qboolean found;

	TIKI_LoadSetup( &temp.tiki, tikianim->name, loadsurfaces, &numSurfacesSetUp, tikianim->modelData, tikianim->modelDataSize, keyValues );
	if( !temp.tiki.numMeshes )
	{
		Com_Printf( "^~^~^ Model '%s' has no skelmodel\n", tikianim->name );
		return NULL;
	}

	defsize = sizeof( dtiki_t ) + strlen( name ) + 1 + temp.tiki.num_surfaces * sizeof( dtikisurface_t ) + temp.tiki.numMeshes * sizeof( short );
	tiki = ( dtiki_t * )TIKI_Alloc( defsize );
	memset( tiki, 0, defsize );
	tiki->a = tikianim;
	tiki->m_boneList.InitChannels();
	tiki->skeletor = NULL;
	tiki->load_scale = temp.tiki.load_scale;
	tiki->lod_scale = temp.tiki.lod_scale;
	tiki->lod_bias = temp.tiki.lod_bias;
	tiki->num_surfaces = temp.tiki.num_surfaces;
	tiki->numMeshes = temp.tiki.numMeshes;
	tiki->radius = temp.tiki.radius;
	tiki->name = ( char * )( ( char * )tiki + sizeof( dtiki_t ) + ( temp.tiki.numMeshes - 1 ) * sizeof( short ) );
	strcpy( tiki->name, name );
	tikiSurf = ( dtikisurface_t * )( ( byte * )tiki->name + strlen( tiki->name ) + 1 );
	tiki->m_boneList.ZeroChannels();

	for( i = 0; i < temp.tiki.numMeshes; i++ )
	{
		mesh = temp.tiki.mesh[ i ];
		tiki->mesh[ i ] = mesh;
		skelmodel = skelcache[ mesh ].skel;
		skelcache[ mesh ].numuses++;

		for( j = 0; j < skelmodel->numBones; j++ )
		{
			tiki->m_boneList.AddChannel( skelmodel->pBones[ j ].channel );
		}
	}

	tiki->m_boneList.PackChannels();
	VectorCopy( temp.tiki.light_offset, tiki->light_offset );
	VectorCopy( temp.tiki.load_origin, tiki->load_origin );
	tiki->surfaces = tikiSurf;

	for( i = 0; i < numSurfacesSetUp; i++ )
	{
		loadsurf = &loadsurfaces[ i ];
		found = false;
		strptr = strchr( loadsurf->name, '*' );

		surfOffset = 0;

		if( strptr || !stricmp( loadsurf->name, "all" ) )
		{
			for( j = 0; j < temp.tiki.numMeshes; j++ )
			{
				mesh = temp.tiki.mesh[ j ];
				skelmodel = TIKI_GetSkel( mesh );
				surf = skelmodel->pSurfaces;

				tikiSurf = &tiki->surfaces[ surfOffset ];

				for( k = 0; k < skelmodel->numSurfaces; k++ )
				{
					tikiSurf = &tiki->surfaces[ surfOffset ];

					if( ( strptr
						&& strptr != loadsurf->name
						&& !strnicmp( loadsurf->name, surf->name, strptr - loadsurf->name ) )
						|| !stricmp( loadsurf->name, "all" ) )
					{
						TIKI_SetupIndividualSurface( tikianim->name, tikiSurf, surf->name, loadsurf );
						found = true;
					}

					surf = surf->pNext;
					tikiSurf++;
				}

				surfOffset += skelmodel->numSurfaces;
			}
		}
		else
		{
			for( j = 0; j < temp.tiki.numMeshes; j++ )
			{
				mesh = temp.tiki.mesh[ j ];
				skelmodel = TIKI_GetSkel( mesh );
				surf = skelmodel->pSurfaces;

				tikiSurf = &tiki->surfaces[ surfOffset ];

				for( k = 0; k < skelmodel->numSurfaces; k++ )
				{
					if( !stricmp( loadsurf->name, surf->name ) )
					{
						TIKI_SetupIndividualSurface( tikianim->name, tikiSurf, surf->name, loadsurf );
						if( !tikiSurf->name[ 0 ] )
						{
							TIKI_Warning( "TIKI_InitTiki: Surface %i in %s(referenced in %s) has no name!  Please investigate and fix\n", k, skelmodel->name, name );
						}
						found = true;
					}

					surf = surf->pNext;
					tikiSurf++;
				}

				surfOffset += skelmodel->numSurfaces;
			}
		}

		if( !found )
		{
			TIKI_Warning( "TIKI_InitTiki: could not find surface '%s' in '%s' (check referenced skb/skd files).\n", loadsurf, tikianim->name );
		}
	}

	if( tiki->radius != 0.0f )
	{
		TIKI_CalcRadius( tiki );
	}

	return tiki;
}

/*
===============
TIKI_CalcRadius
===============
*/
void TIKI_CalcRadius( dtiki_t *tiki )
{
	int j;
	float radius;
	float tmpVec[ 3 ];
	float *bounds[ 2 ];

	tiki->radius = 0.0f;

	bounds[ 0 ] = &tiki->a->mins[ 0 ];
	bounds[ 1 ] = &tiki->a->maxs[ 0 ];

	for( j = 0; j < 4; j++ )
	{
		tmpVec[ 0 ] = bounds[ j & 1 ][ 0 ];
		tmpVec[ 1 ] = bounds[ j & 1 ][ 1 ];
		tmpVec[ 2 ] = bounds[ j & 1 ][ 2 ];

		radius = VectorLength( tmpVec );
		if( radius > tiki->radius ) {
			tiki->radius = radius;
		}
	}

	radius = tiki->radius * 0.7f;
	tiki->radius = radius * tiki->lod_scale;
}

/*
===============
SkeletorCacheFileCallback
===============
*/
skelAnimDataGameHeader_t *SkeletorCacheFileCallback( const char *path )
{
	skelAnimDataFileHeader_t *pHeader;
	int iBuffLength;
	char tempName[ 100 ];
	char extension[ 100 ];
	skelAnimDataGameHeader_t *finishedHeader;
	char *buffer;
	char npath[ 256 ];

	Skel_ExtractFileExtension( path, extension );

	if( strcmp( extension, "skc" ) )
	{
		Com_Printf( "Skeletor CacheAnimSkel: %s: File extension unknown.  Attempting to open as skc file\n", path );
	}

	strcpy( npath, "newanim/" );
	strcat( npath, path );

	iBuffLength = TIKI_ReadFileEx( npath, ( void ** )&buffer, qtrue );
	if( iBuffLength > 0 )
	{
		finishedHeader = skeletor_c::LoadProcessedAnim( npath, buffer, iBuffLength, path );
		TIKI_FreeFile( buffer );
	}
	else
	{
		iBuffLength = TIKI_ReadFileEx( path, ( void ** )&pHeader, qtrue );
		if( iBuffLength <= 0 )
		{
			Com_Printf( "Skeletor CacheAnimSkel: Could not open binary file %s\n", path );
			return NULL;
		}

		if( pHeader->ident != TIKI_SKC_HEADER_IDENT || ( pHeader->version != TIKI_SKC_HEADER_OLD_VERSION && pHeader->version != TIKI_SKC_HEADER_VERSION ) )
		{
			Com_Printf( "Skeletor CacheAnimSkel: anim %s has wrong header ([ident,version] = [%i,%i] should be [%i,%i])\n", path,
				pHeader->ident, pHeader->version,
				TIKI_SKC_HEADER_IDENT, TIKI_SKC_HEADER_VERSION );
			TIKI_FreeFile( pHeader );
			return NULL;
		}

		if( pHeader->version == TIKI_SKC_HEADER_OLD_VERSION )
		{
			Com_Printf( "WARNING- DOWNGRADING TO OLD ANIMATION FORMAT FOR FILE: %s\n", path );
			finishedHeader = skeletor_c::ConvertSkelFileToGame( pHeader, iBuffLength, path );
			if( convertAnims && convertAnims->integer )
			{
				skeletor_c::SaveProcessedAnim( finishedHeader, path, pHeader );
			}
		}
		else
		{
			// looks like SKC version 14 and above are processed animations

			// points the buffer to the animation data
			buffer = ( char * )pHeader + sizeof( int ) + sizeof( int );
			iBuffLength -= sizeof( int ) + sizeof( int );

			// loads the processed animation
			finishedHeader = skeletor_c::LoadProcessedAnimEx( path, buffer, iBuffLength, path );
		}

		TIKI_FreeFile( pHeader );
	}

	if( dumploadedanims && dumploadedanims->integer )
	{
		Com_Printf( "+loadanim: %s\n", path );
	}

	sprintf( tempName, "g%s", path );
	UI_LoadResource( tempName );

	return finishedHeader;
}

/*
===============
SkeletorCacheGetData
===============
*/
skelAnimDataGameHeader_t *SkeletorCacheGetData( int index )
{
	if( index < 0 )
	{
		return NULL;
	}

	skelAnimDataGameHeader_t *data = m_cachedData[ index ].data;
	if( !data )
	{
		data = SkeletorCacheFileCallback( m_cachedData[ index ].path );
		m_cachedData[ index ].data = data;
	}

	return data;
}

/*
===============
SkeletorCacheFindFilename
===============
*/
bool SkeletorCacheFindFilename( const char *path, int *indexPtr )
{
	int sortValue;
	int lowerBound;
	int upperBound;
	int index;

	lowerBound = 0;
	upperBound = m_numInCache - 1;
	while( lowerBound <= upperBound )
	{
		index = ( lowerBound + upperBound ) / 2;
		sortValue = stricmp( path, m_cachedData[ m_cachedDataLookup[ index ] ].path );
		if( !sortValue )
		{
			if( indexPtr )
				*indexPtr = index;
			return true;
		}
		if( sortValue < 0 )
		{
			upperBound = index - 1;
		}
		else
		{
			lowerBound = index + 1;
		}
	}

	if( indexPtr )
		*indexPtr = lowerBound;
	return false;
}

/*
===============
SkeletorCacheLoadData
===============
*/
bool SkeletorCacheLoadData( const char *path, bool precache, int newIndex )
{
	int i;
	skelAnimDataGameHeader_t *data;
	int lookup;

	if( m_numInCache >= 4095 )
	{
		Com_Printf( "Skeletor CacheData, Cache full, can't load %s\n", path );
		return false;
	}

	if( strlen( path ) >= 100 )
	{
		Com_Printf( "^~^~^ SkeletorCache: File name over %i characters will be ignored.\n(%s)\n", 99, path );
		return false;
	}

	if( precache )
	{
		data = SkeletorCacheFileCallback( path );
		if( !data ) {
			return false;
		}
	}
	else
	{
		data = 0;
	}

	for( lookup = 0; lookup < 4095; lookup++ )
	{
		if( m_cachedData[ lookup ].lookup == -1 ) {
			break;
		}
	}

	for( i = m_numInCache - 1; i >= newIndex; i-- )
	{
		m_cachedData[ m_cachedDataLookup[ i ] ].lookup = i + 1;
		m_cachedDataLookup[ i + 1 ] = m_cachedDataLookup[ i ];
	}

	m_cachedDataLookup[ newIndex ] = lookup;
	m_cachedData[ lookup ].lookup = newIndex;
	m_cachedData[ lookup ].data = data;
	strcpy( m_cachedData[ lookup ].path, path );
	m_cachedData[ lookup ].numusers = 0;
	m_numInCache++;

	return true;
}

/*
===============
SkeletorCacheUnloadData
===============
*/
void SkeletorCacheUnloadData( int index )
{
	int i;

	m_numInCache--;

	if( dumploadedanims && dumploadedanims->integer )
	{
		Com_Printf( "-loadanim: %s\n", m_cachedData[ m_cachedDataLookup[ index ] ] );
	}

	if( m_cachedData[ m_cachedDataLookup[ index ] ].data ) {
		skelAnimDataGameHeader_s::DeallocAnimData( m_cachedData[ m_cachedDataLookup[ index ] ].data );
		m_cachedData[ m_cachedDataLookup[ index ] ].data = NULL;
	}

	m_cachedData[ m_cachedDataLookup[ index ] ].lookup = -1;
	for( i = index + 1; i < m_numInCache; i++ )
	{
		m_cachedDataLookup[ i - 1 ] = m_cachedDataLookup[ i ];
		m_cachedData[ m_cachedDataLookup[ i ] ].lookup = i - 1;
	}
}

/*
===============
SkeletorCacheCleanCache
===============
*/
void SkeletorCacheCleanCache()
{
	int i;

	for( i = m_numInCache - 1; i >= 0; i-- )
	{
		if( !m_cachedData[ m_cachedDataLookup[ i ] ].numusers ) {
			SkeletorCacheUnloadData( i );
		}
	}
}

/*
===============
TikiAddToBounds
===============
*/
void TikiAddToBounds( dtikianim_t *tiki, SkelVec3 *newBounds )
{
	int i;

	for( i = 0; i < 3; i++ )
	{
		if( newBounds[ 0 ].val[ i ] < tiki->mins[ i ] ) {
			tiki->mins[ i ] = newBounds[ 0 ].val[ i ];
		}

		if( newBounds[ 1 ].val[ i ] > tiki->maxs[ i ] ) {
			tiki->maxs[ i ] = newBounds[ 1 ].val[ i ];
		}
	}
}

/*
===============
TIKI_AnimList_f
===============
*/
void TIKI_AnimList_f()
{
	skeletorCacheEntry_t *entry;
	int i;

	Com_Printf( "\nanimlist:\n" );
	for( i = 0; i < m_numInCache; i++ )
	{
		entry = &m_cachedData[ m_cachedDataLookup[ i ] ];
		if( !entry ) {
			Com_Printf( "*** NOT CACHED: " );
		}

		if( m_cachedData[ i ].path[ 0 ] )
		{
			Com_Printf( "%s\n", m_cachedData[ i ].path );
		}
		else
		{
			Com_Printf( "*** EMPTY PATH ERROR\n" );
		}
	}

	for( ; i < 4095; i++ )
	{
		if( m_cachedData[ m_cachedDataLookup[ i ] ].path[ 0 ] ) {
			Com_Printf( "*** CORRUPTED ENTRY\n" );
		}
	}
}

/*
===============
TIKI_FixFrameNum
===============
*/
void TIKI_FixFrameNum( dtikianim_t *ptiki, skelAnimDataGameHeader_t *animData, dtikicmd_t *cmd, const char *alias )
{
	if( cmd->frame_num >= TIKI_FRAME_LAST && cmd->frame_num < animData->numFrames )
	{
		if( cmd->frame_num <= TIKI_FRAME_END ) {
			cmd->frame_num = animData->numFrames - 1;
		}
	}
	else
	{
		TIKI_Error( "TIKI_FixFrameNum: illegal frame number %d (total: %d) in anim '%s' in '%s'\n", cmd->frame_num, animData->numFrames, alias, ptiki->name );
		cmd->frame_num = 0;
	}
}

/*
===============
TIKI_LoadAnim
===============
*/
void TIKI_LoadAnim( dtikianim_t *ptiki )
{
	int i, j;
	dtikianimdef_t *panim;
	skelAnimDataGameHeader_t *animData;

	for( i = 0; i < ptiki->num_anims; i++ )
	{
		animData = SkeletorCacheGetData( ptiki->m_aliases[ i ] );
		if( animData )
		{
			panim = ptiki->animdefs[ i ];
			for( j = 0; j < panim->num_server_cmds; j++ )
			{
				TIKI_FixFrameNum( ptiki, animData, &ptiki->animdefs[ i ]->server_cmds[ j ], ptiki->animdefs[ i ]->alias );
			}

			for( j = 0; j < panim->num_client_cmds; j++ )
			{
				TIKI_FixFrameNum( ptiki, animData, &ptiki->animdefs[ i ]->client_cmds[ j ], ptiki->animdefs[ i ]->alias );
			}
		}
	}
}

/*
===============
TIKI_InitTiki
===============
*/
dtikianim_t *TIKI_InitTiki( dloaddef_t *ld, size_t defsize )
{
	byte *ptr;
	byte *start_ptr;
	dtikicmd_t *pcmds;
	int i, k;
	size_t j;
	size_t size;
	//int anim_index;
	int alias_index;
	dtikianim_t *panim;
	dtikianimdef_t *panimdef;
	dloadanim_t *anim;
	skelAnimDataGameHeader_t *data;
	qboolean bModelBoundsSet = false;
	bool bPrecache;
	int index;
	char tempName[ 257 ];
	int order[ 4095 ];
	short temp_aliases[ 4095 ];

	panim = ( dtikianim_t * )TIKI_Alloc( defsize );
	memset( panim, 0, defsize );
	ClearBounds( panim->mins, panim->maxs );
	panim->num_client_initcmds = ld->numclientinitcmds;
	panim->num_server_initcmds = ld->numserverinitcmds;
	panim->bIsCharacter = ld->bIsCharacter;
	panim->name = ( char * )( ( char * )panim + sizeof( dtikianim_t ) + ( ld->numanims - 1 ) * sizeof( dtikianimdef_t * ) );
	strcpy( panim->name, ld->path );
	panim->server_initcmds = ( dtikicmd_t * )( ( char * )panim->name + strlen( ld->path ) + 1 );

	start_ptr = ( byte * )( ( char * )panim->server_initcmds + ld->numserverinitcmds * sizeof( dtikicmd_t ) );
	ptr = start_ptr;

	// Process server init commands
	for( i = 0; i < ld->numserverinitcmds; i++ )
	{
		pcmds = &panim->server_initcmds[ i ];
		pcmds->num_args = ld->loadserverinitcmds[ i ]->num_args;
		pcmds->args = ( char ** )ptr;

		ptr += pcmds->num_args * sizeof( char ** );

		for( j = 0; j < ld->loadserverinitcmds[ i ]->num_args; j++ )
		{
			pcmds->args[ j ] = ( char * )ptr;
			size = strlen( ld->loadserverinitcmds[ i ]->args[ j ] ) + 1;
			memcpy( pcmds->args[ j ], ld->loadserverinitcmds[ i ]->args[ j ], size );

			ptr += size;
		}
	}

	panim->client_initcmds = ( dtikicmd_t * )ptr;

	start_ptr = ( byte * )( ( char * )panim->client_initcmds + ld->numclientinitcmds * sizeof( dtikicmd_t ) );
	ptr = start_ptr;

	// Process client init commands
	for( i = 0; i < ld->numclientinitcmds; i++ )
	{
		pcmds = &panim->client_initcmds[ i ];
		pcmds->num_args = ld->loadclientinitcmds[ i ]->num_args;
		pcmds->args = ( char ** )ptr;

		ptr += pcmds->num_args * sizeof( char ** );

		for( j = 0; j < ld->loadclientinitcmds[ i ]->num_args; j++ )
		{
			pcmds->args[ j ] = ( char * )ptr;
			size = strlen( ld->loadclientinitcmds[ i ]->args[ j ] ) + 1;
			memcpy( pcmds->args[ j ], ld->loadclientinitcmds[ i ]->args[ j ], size );

			ptr += size;
		}
	}

	TIKI_GetAnimOrder( ld, order );
	sprintf( tempName, "e%s", ld->path );
	UI_LoadResource( tempName );

	panim->m_aliases = temp_aliases;

	// Process anim commands
	for( i = 0; i < ld->numanims; i++ )
	{
		anim = ld->loadanims[ order[ i ] ];
		if( !SkeletorCacheFindFilename( anim->name, &index ) ) 
		{
			bPrecache = false;

			if( ( !low_anim_memory && ld->numanims <= 49 ) || !low_anim_memory || !low_anim_memory->integer )
			{
				bPrecache = true;
			}

			if( !SkeletorCacheLoadData( anim->name, bPrecache, index ) )
			{
				TIKI_Error( "TIKI_InitTiki: Failed to load animation '%s' at %s\n", anim->name, anim->location );
				panim->m_aliases[ i ] = -1;
				continue;
			}
		}

		alias_index = m_cachedDataLookup[ index ];
		m_cachedData[ alias_index ].numusers++;
		panimdef = ( dtikianimdef_t * )ptr;
		ptr += sizeof( dtikianimdef_t );
		panim->animdefs[ i ] = panimdef;
		panim->m_aliases[ i ] = alias_index;
		strcpy( panimdef->alias, anim->alias );
		panimdef->weight = anim->weight;
		panimdef->flags = anim->flags;

		if( !Q_stricmp( panimdef->alias, "idle" ) )
		{
			data = SkeletorCacheGetData( alias_index );
			if( data )
			{
				VectorCopy( data->bounds[ 0 ].val, panim->mins );
				VectorCopy( data->bounds[ 1 ].val, panim->maxs );
				bModelBoundsSet = true;
			}
		}


		if( anim->flags & TAF_RANDOM )
		{
			j = strlen( panimdef->alias );
			if( isdigit( panimdef->alias[ j - 1 ] ) )
			{
				do
				{
					j--;
				} while( isdigit( panimdef->alias[ j - 1 ] ) );

				panimdef->alias[ j ] = 0;
			}
			else
			{
				TIKI_DPrintf( "TIKI_InitTiki: Random animation name '%s' should end with a number\n", panimdef->alias );
			}
		}

		panimdef->blendtime = anim->blendtime;
		if( loadtikicommands )
		{
			panimdef->num_server_cmds = anim->num_server_cmds;
			panimdef->num_client_cmds = anim->num_client_cmds;
		}
		else
		{
			panimdef->num_server_cmds = 0;
			panimdef->num_client_cmds = 0;
		}

		panimdef->server_cmds = ( dtikicmd_t * )ptr;
		ptr += anim->num_server_cmds * sizeof( dtikicmd_t );

		// Process server anim commands
		for( j = 0; j < anim->num_server_cmds; j++ )
		{
			pcmds = &panimdef->server_cmds[ j ];
			pcmds->num_args = anim->loadservercmds[ j ]->num_args;
			pcmds->frame_num = anim->loadservercmds[ j ]->frame_num;
			pcmds->args = ( char ** )ptr;

			ptr += pcmds->num_args * sizeof( char * );

			for( k = 0; k < anim->loadservercmds[ j ]->num_args; k++ )
			{
				pcmds->args[ k ] = ( char * )ptr;
				size = strlen( anim->loadservercmds[ j ]->args[ k ] ) + 1;
				memcpy( pcmds->args[ k ], anim->loadservercmds[ j ]->args[ k ], size );

				ptr += size;
			}
		}

		panimdef->client_cmds = ( dtikicmd_t * )ptr;
		ptr += anim->num_client_cmds * sizeof( dtikicmd_t );

		// Process client anim commands
		for( j = 0; j < anim->num_client_cmds; j++ )
		{
			pcmds = &panimdef->client_cmds[ j ];
			pcmds->num_args = anim->loadclientcmds[ j ]->num_args;
			pcmds->frame_num = anim->loadclientcmds[ j ]->frame_num;
			pcmds->args = ( char ** )ptr;

			ptr += pcmds->num_args * sizeof( char * );

			for( k = 0; k < anim->loadclientcmds[ j ]->num_args; k++ )
			{
				pcmds->args[ k ] = ( char * )ptr;
				size = strlen( anim->loadclientcmds[ j ]->args[ k ] ) + 1;
				memcpy( pcmds->args[ k ], anim->loadclientcmds[ j ]->args[ k ], size );

				ptr += size;
			}
		}
	}

	panim->m_aliases = NULL;
	if( i )
	{
		if( !bModelBoundsSet )
		{
			TIKI_DPrintf( "TIKI_InitTiki: no 'idle' animation found, model bounds not set for %s\n", ld->path );
		}

		panim->num_anims = i;
		panim->m_aliases = ( short * )TIKI_Alloc( panim->num_anims * sizeof( short ) );
		memcpy( panim->m_aliases, temp_aliases, panim->num_anims * sizeof( short ) );
		panim->modelData = ptr;
		panim->modelDataSize = ld->modelBuf->cursize;
		memcpy( panim->modelData, ld->modelData, panim->modelDataSize );
		ptr += panim->modelDataSize;

		size = strlen( ld->headmodels ) + 1;
		panim->headmodels = ( char * )ptr;
		memcpy( panim->headmodels, ld->headmodels, size );
		ptr += size;

		size = strlen( ld->headskins ) + 1;
		panim->headskins = ( char * )ptr;
		memcpy( panim->headskins, ld->headskins, size );
		ptr += size;

		sprintf( tempName, "h%s", ld->path );
		UI_LoadResource( tempName );

		if( low_anim_memory && ( !low_anim_memory->integer || !tiki_loading ) )
		{
			TIKI_LoadAnim( panim );
		}
	}
	else
	{
		TIKI_Error( "TIKI_InitTiki: No valid animations found in %s.\n", ld->path );
		panim = NULL;
	}

	return panim;
}

/*
===============
TIKI_RemoveTiki
===============
*/
void TIKI_RemoveTiki( dtikianim_t *ptiki )
{
	int i;
	int alias_index;

	for( i = 0; i < ptiki->num_anims; i++ )
	{
		alias_index = ptiki->m_aliases[ i ];
		m_cachedData[ alias_index ].numusers--;
	}
}
