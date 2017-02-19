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

// tiki_parse.cpp : TIKI Parser

#include "q_shared.h"
#include "qcommon.h"
#include "../server/server.h"
#include "../skeletor/skeletor.h"
#include <tiki.h>
#include "dbgheap.h"

/*
===============
TIKI_FileExtension
===============
*/
const char *TIKI_FileExtension( const char *in )
{
	static char exten[ 8 ];
	int i;

	for( i = 0; in[ i ] != 0; i++ )
	{
		if( in[ i ] == '.' )
		{
			i++;
			break;
		}
	}

	if( !in[ i ] )
	{
		return "";
	}

	strncpy( exten, &in[ i ], sizeof( exten ) );
	return exten;
}

/*
===============
TIKI_ParseFrameCommands
===============
*/
void TIKI_ParseFrameCommands( dloaddef_t *ld, dloadframecmd_t **cmdlist, int maxcmds, int *numcmds )
{
	qboolean usecurrentframe = false;
	const char *token;
	dloadframecmd_t *cmds;
	int framenum = 0;
	int i;
	char *pszArgs[ 256 ];

	ld->tikiFile.GetToken( true );

	while( ld->tikiFile.TokenAvailable( true ) )
	{
		token = ld->tikiFile.GetToken( true );
		if( !stricmp( token, "}" ) )
		{
			break;
		}
	
		if( *numcmds < maxcmds )
		{
			cmds = ( dloadframecmd_t * )TIKI_AllocateLoadData( sizeof( dloadframecmd_t ) );
			cmdlist[ *numcmds ] = cmds;
			if( !cmds )
			{
				TIKI_Error( "TIKI_ParseFrameCommands: could not allocate storage for dloadframecmd_t in %s on line %d.\n", ld->tikiFile.Filename(), ld->tikiFile.GetLineNumber() );
				continue;
			}

			cmds->num_args = 0;
			( *numcmds )++;

			if( !stricmp( token, "start" ) || !stricmp( token, "first" ) )
			{
				framenum = TIKI_FRAME_FIRST;
			}
			else if( !stricmp( token, "end" ) )
			{
				framenum = TIKI_FRAME_END;
			}
			else if( !stricmp( token, "last" ) )
			{
				framenum = TIKI_FRAME_LAST;
			}
			else if( !stricmp( token, "every" ) )
			{
				framenum = TIKI_FRAME_EVERY;
			}
			else if( !stricmp( token, "exit" ) )
			{
				framenum = TIKI_FRAME_EXIT;
			}
			else if( !stricmp( token, "entry" ) || !stricmp( token, "enter" ) )
			{
				framenum = TIKI_FRAME_ENTRY;
			}
			else if( !stricmp( token, "(" ) )
			{
				usecurrentframe = true;
				ld->tikiFile.UnGetToken();
			}
			else if( !stricmp( token, ")" ) )
			{
				usecurrentframe = false;
				ld->tikiFile.UnGetToken();
			}
			else if( !usecurrentframe )
			{
				framenum = atoi( token );
			}
			else
			{
				ld->tikiFile.UnGetToken();
			}

			if( framenum < TIKI_FRAME_LAST )
			{
				TIKI_Error( "TIKI_ParseFrameCommands: illegal frame number %d on line %d in %s\n", framenum, ld->tikiFile.Filename(), ld->tikiFile.GetLineNumber() );
				while( ld->tikiFile.TokenAvailable( false ) )
					ld->tikiFile.GetToken( false );
				( *numcmds )--;
				continue;
			}

			cmds->frame_num = framenum;
			if( ld->tikiFile.currentScript )
			{
				sprintf( cmds->location, "%s, line: %d", ld->tikiFile.Filename(), ld->tikiFile.GetLineNumber() );
			}

			while( ld->tikiFile.TokenAvailable( false ) )
			{
				token = ld->tikiFile.GetToken( false );

				if( cmds->num_args > 255 )
				{
					TIKI_Error( "TIKI_ParseFrameCommands: too many args in anim commands in %s.\n", ld->tikiFile.Filename() );
					continue;
				}

				pszArgs[ cmds->num_args ] = TIKI_CopyString( token );
				cmds->num_args++;
			}

			cmds->args = ( char ** )TIKI_AllocateLoadData( cmds->num_args * sizeof( char * ) );
			for( i = 0; i < cmds->num_args; i++ )
				cmds->args[ i ] = pszArgs[ i ];
		}
		else
		{
			TIKI_Error( "TIKI_ParseFrameCommands: too many anim commands in %s starting on line %i.\n", ld->tikiFile.Filename(), ld->tikiFile.GetLineNumber() );
			while( ld->tikiFile.TokenAvailable( false ) )
				ld->tikiFile.GetToken( false );
		}
	}
}

/*
===============
TIKI_ParseAnimationCommands
===============
*/
void TIKI_ParseAnimationCommands( dloaddef_t *ld, dloadanim_t *anim )
{
	const char *token;

	while( ld->tikiFile.TokenAvailable( true ) )
	{
		token = ld->tikiFile.GetToken( true );
		if( !stricmp( token, "client" ) )
		{
			TIKI_ParseFrameCommands( ld, anim->loadclientcmds, sizeof( anim->loadclientcmds ) / sizeof( anim->loadclientcmds[ 0 ] ), &anim->num_client_cmds );
		}
		else if( !stricmp( token, "server" ) )
		{
			TIKI_ParseFrameCommands( ld, anim->loadservercmds, sizeof( anim->loadservercmds ) / sizeof( anim->loadservercmds[ 0 ] ), &anim->num_server_cmds );
		}
		else if( !stricmp( token, "}" ) )
		{
			break;
		}
		else
		{
			TIKI_Error( "TIKI_ParseAnimationCommands: unknown anim command '%s' in '%s' on line %d, skipping line.\n", token, ld->tikiFile.Filename(), ld->tikiFile.GetLineNumber() );
			while( ld->tikiFile.TokenAvailable( false ) )
				token = ld->tikiFile.GetToken( false );
		}
	}
}

/*
===============
TIKI_ParseAnimationFlags
===============
*/
void TIKI_ParseAnimationFlags( dloaddef_t *ld, dloadanim_t *anim )
{
	const char *token;

	anim->weight = 1.0f;
	anim->blendtime = 0.2f;
	anim->flags = 0;

	while( ld->tikiFile.TokenAvailable( false ) )
	{
		token = ld->tikiFile.GetToken( false );

		if( !stricmp( token, "weight" ) )
		{
			anim->weight = ld->tikiFile.GetFloat( false );
			anim->flags |= TAF_RANDOM;
		}
		else if( !stricmp( token, "deltadriven" ) )
		{
			anim->flags |= TAF_DELTADRIVEN;
		}
		else if( !stricmp( token, "default_angles" ) )
		{
			anim->flags |= TAF_DEFAULT_ANGLES;
		}
		else if( !stricmp( token, "notimecheck" ) )
		{
			anim->flags |= TAF_NOTIMECHECK;
		}
		else if( !stricmp( token, "crossblend" ) )
		{
			anim->blendtime = ld->tikiFile.GetFloat( false );
		}
		else if( !stricmp( token, "dontrepeate" ) )
		{
			anim->flags |= TAF_NOREPEAT;
		}
		else if( !stricmp( token, "random" ) )
		{
			anim->flags |= TAF_RANDOM;
		}
		else if( !stricmp( token, "autosteps_run" ) )
		{
			anim->flags |= TAF_AUTOSTEPS_RUN;
		}
		else if( !stricmp( token, "autosteps_walk" ) )
		{
			anim->flags |= TAF_AUTOSTEPS_WALK;
		}
		else if( !stricmp( token, "autosteps_dog" ) )
		{
			anim->flags |= TAF_AUTOSTEPS_DOG;
		}
		else
		{
			TIKI_Error( "Unknown Animation flag %s for anim '%s' in %s\n", token, anim->alias, TikiScript::currentScript->Filename() );
		}
	}
}

/*
===============
TIKI_ParseAnimationsFail
===============
*/
void TIKI_ParseAnimationsFail( dloaddef_t *ld )
{
	int nestcount = 0;
	const char *token;

	while( ld->tikiFile.TokenAvailable( false ) )
	{
		ld->tikiFile.GetToken( false );
	}

	if( !ld->tikiFile.TokenAvailable( true ) )
	{
		return;
	}

	token = ld->tikiFile.GetToken( true );
	if( stricmp( token, "{" ) )
	{
		ld->tikiFile.UnGetToken();
		return;
	}

	ld->tikiFile.UnGetToken();

	while( ld->tikiFile.TokenAvailable( true ) )
	{
		token = ld->tikiFile.GetToken( true );
		if( !stricmp( token, "{" ) )
		{
			nestcount++;
		}
		else if( !stricmp( token, "}" ) )
		{
			nestcount--;
			if( !nestcount )
				break;
		}
	}
}

/*
===============
TIKI_ParseIncludes
===============
*/
qboolean TIKI_ParseIncludes( dloaddef_t *ld )
{
	const char *token;
	qboolean b_incl = false;
	const char *mapname;
	int depth = 0;

	token = ld->tikiFile.GetToken( true );
	if( sv_mapname )
	{
		mapname = sv_mapname->string;
	}
	else
	{
		mapname = "utils";
	}

	while( 1 )
	{
		if( !strncmp( token, mapname, strlen( token ) )
			|| !strncmp( token, "spearheadserver", strlen( token ) )
			|| !strncmp( token, "breakthroughserver", strlen( token ) ) )
		{
			b_incl = true;
		}
		else if( ( !stricmp( token, "{" ) || !ld->tikiFile.TokenAvailable( true ) ) )
		{
			break;
		}

		token = ld->tikiFile.GetToken( true );
	}

	if( b_incl )
	{
		return true;
	}

	while( ld->tikiFile.TokenAvailable( true ) )
	{
		token = ld->tikiFile.GetAndIgnoreLine( false );
		if( strstr( token, "{" ) )
		{
			depth++;
		}

		if( strstr( token, "}" ) )
		{
			if( !depth )
			{
				break;
			}

			depth--;
		}
	}

	return false;
}

/*
===============
TIKI_ParseAnimations
===============
*/
void TIKI_ParseAnimations( dloaddef_t *ld )
{
	const char *token;
	dloadanim_t *anim;
	qboolean b_mapspec = false;
	const char *mapname;
	size_t depth = 0;

	ld->tikiFile.GetToken( true );

	while( ld->tikiFile.TokenAvailable( true ) )
	{
		token = ld->tikiFile.GetToken( true );
		if( !stricmp( token, "}" ) )
		{
			return;
		}
		else if( !stricmp( token, "$mapspec" ) )
		{
			token = ld->tikiFile.GetToken( true );
			if( sv_mapname )
			{
				mapname = sv_mapname->string;
			}
			else
			{
				mapname = "utils";
			}

			while( ld->tikiFile.TokenAvailable( true ) )
			{
				if( !strncmp( token, mapname, strlen( token ) ) )
				{
					b_mapspec = true;
				}
				else if( !stricmp(token, "{" ) )
				{
					break;
				}

				token = ld->tikiFile.GetToken( true );
			}

			if( !b_mapspec )
			{
				while( ld->tikiFile.TokenAvailable( true ) )
				{
					token = ld->tikiFile.GetToken( true );
					if( strstr( token, "{" ) )
					{
						depth++;
					}

					if( strstr( token, "}" ) )
					{
						if( !depth )
						{
							continue;
						}

						depth--;
					}
				}

				return;
			}
		}
		else
		{
			if( ld->numanims > 4094 )
			{
				TIKI_Error( "TIKI_ParseAnimations: Too many animations in '%s'.\n", ld->path );
				continue;
			}

			anim = ( dloadanim_t * )TIKI_AllocateLoadData( sizeof( dloadanim_t ) );
			ld->loadanims[ ld->numanims ] = anim;
			if( !anim )
			{
				TIKI_Error( "TIKI_ParseAnimations: Could not allocate storage for anim alias name %s in %s.\n", token, ld->tikiFile.Filename() );
				TIKI_ParseAnimationsFail( ld );
				continue;
			}

			depth = strlen( token );
			if( depth < 48 )
			{
				anim->alias = ( char * )TIKI_CopyString( token );

				token = ld->tikiFile.GetToken( false );
				strcpy( anim->name, TikiScript::currentScript->path );
				strcat( anim->name, token );

				anim->location[ 0 ] = 0;
				if( ld->tikiFile.currentScript )
				{
					sprintf( anim->location, "%s, line: %d", ld->tikiFile.currentScript->Filename(), ld->tikiFile.currentScript->GetLineNumber() );
				}

				anim->num_client_cmds = 0;
				anim->num_server_cmds = 0;
				TIKI_ParseAnimationFlags( ld, anim );
				ld->numanims++;

				if( ld->tikiFile.TokenAvailable( true ) )
				{
					token = ld->tikiFile.GetToken( true );
					if( !stricmp( token, "{" ) )
					{
						TIKI_ParseAnimationCommands( ld, anim );
					}
					else
					{
						ld->tikiFile.UnGetToken();
					}
				}
			}
			else
			{
				TIKI_Error( "TIKI_ParseAnimations: Anim alias name %s is too long in %s.\n", token, ld->tikiFile.Filename() );
				TIKI_ParseAnimationsFail( ld );
			}
		}
	}
}

/*
===============
TIKI_ParseSurfaceFlag
===============
*/
int TIKI_ParseSurfaceFlag( const char *token )
{
	int flags = 0;

	if( !stricmp( token, "skin1" ) )
	{
		flags = TIKI_SURF_SKIN1;
	}
	else if( !stricmp( token, "skin2" ) )
	{
		flags = TIKI_SURF_SKIN2;
	}
	else if( !stricmp( token, "skin3" ) )
	{
		flags = TIKI_SURF_SKIN3;
	}
	else if( !stricmp( token, "nodraw" ) )
	{
		flags = TIKI_SURF_NODRAW;
	}
	else if( !stricmp( token, "nodamage" ) )
	{
		flags = TIKI_SURF_NODAMAGE;
	}
	else if( !stricmp( token, "crossfade" ) )
	{
		flags = TIKI_SURF_CROSSFADE;
	}
	else if( !stricmp( token, "nomipmaps" ) )
	{
		flags = TIKI_SURF_NOMIPMAPS;
	}
	else if( !stricmp( token, "nopicmip" ) )
	{
		flags = TIKI_SURF_NOPICMIP;
	}
	else
	{
		TIKI_Error( "Unknown surface flag %s\n", token );
	}

	return flags;
}

/*
===============
WriteScale
===============
*/
static void WriteScale( dloaddef_t *ld, float scale )
{
	MSG_WriteByte( ld->modelBuf, 0 );
	MSG_WriteFloat( ld->modelBuf, scale );
}

/*
===============
WriteLoadScale
===============
*/
static void WriteLoadScale( dloaddef_t *ld, float lod_scale )
{
	MSG_WriteByte( ld->modelBuf, 1 );
	MSG_WriteFloat( ld->modelBuf, lod_scale );
}

/*
===============
WriteLodBias
===============
*/
static void WriteLodBias( dloaddef_t *ld, float lod_bias )
{
	MSG_WriteByte( ld->modelBuf, 2 );
	MSG_WriteFloat( ld->modelBuf, lod_bias );
}

/*
===============
WriteSkelmodel
===============
*/
static void WriteSkelmodel( dloaddef_t *ld, const char *name )
{
	MSG_WriteByte( ld->modelBuf, 3 );
	MSG_WriteString( ld->modelBuf, name );
}

/*
===============
WriteOrigin
===============
*/
static void WriteOrigin( dloaddef_t *ld, float origin_x, float origin_y, float origin_z )
{
	MSG_WriteByte( ld->modelBuf, 4 );
	MSG_WriteFloat( ld->modelBuf, origin_x );
	MSG_WriteFloat( ld->modelBuf, origin_y );
	MSG_WriteFloat( ld->modelBuf, origin_z );
}

/*
===============
WriteLightOffset
===============
*/
static void WriteLightOffset( dloaddef_t *ld, float light_offset_x, float light_offset_y, float light_offset_z )
{
	MSG_WriteByte( ld->modelBuf, 5 );
	MSG_WriteFloat( ld->modelBuf, light_offset_x );
	MSG_WriteFloat( ld->modelBuf, light_offset_y );
	MSG_WriteFloat( ld->modelBuf, light_offset_z );
}

/*
===============
WriteRadius
===============
*/
static void WriteRadius( dloaddef_t *ld, float radius )
{
	MSG_WriteByte( ld->modelBuf, 6 );
	MSG_WriteFloat( ld->modelBuf, radius );
}

/*
===============
WriteSurface
===============
*/
static void WriteSurface( dloaddef_t *ld, const char *surface )
{
	MSG_WriteByte( ld->modelBuf, 7 );
	MSG_WriteString( ld->modelBuf, surface );
}

/*
===============
WriteFlags
===============
*/
static void WriteFlags( dloaddef_t *ld, int flags )
{
	MSG_WriteByte( ld->modelBuf, 8 );
	MSG_WriteLong( ld->modelBuf, flags );
}

/*
===============
WriteDamage
===============
*/
static void WriteDamage( dloaddef_t *ld, float damage )
{
	MSG_WriteByte( ld->modelBuf, 9 );
	MSG_WriteFloat( ld->modelBuf, damage );
}

/*
===============
WriteShader
===============
*/
static void WriteShader( dloaddef_t *ld, const char *shader )
{
	MSG_WriteByte( ld->modelBuf, 10 );
	MSG_WriteString( ld->modelBuf, shader );
}

/*
===============
WriteBeginCase
===============
*/
static void WriteBeginCase( dloaddef_t *ld )
{
	MSG_WriteByte( ld->modelBuf, 13 );
}

/*
===============
WriteCaseKey
===============
*/
static void WriteCaseKey( dloaddef_t *ld, const char *key )
{
	MSG_WriteByte( ld->modelBuf, 11 );
	MSG_WriteString( ld->modelBuf, key );
}

/*
===============
WriteCaseValue
===============
*/
static void WriteCaseValue( dloaddef_t *ld, const char *value )
{
	MSG_WriteByte( ld->modelBuf, 12 );
	MSG_WriteString( ld->modelBuf, value );
}

/*
===============
WriteBeginCaseBody
===============
*/
static void WriteBeginCaseBody( dloaddef_t *ld )
{
	MSG_WriteByte( ld->modelBuf, 14 );
}

/*
===============
WriteEndCase
===============
*/
static void WriteEndCase( dloaddef_t *ld )
{
	MSG_WriteByte( ld->modelBuf, 15 );
}

/*
===============
TIKI_InitSetup
===============
*/
void TIKI_InitSetup( dloaddef_t *ld )
{
	MSG_Init( ld->modelBuf, ld->modelData, 8192 );
}

/*
===============
TIKI_LoadSetupCaseHeader
===============
*/
qboolean TIKI_LoadSetupCaseHeader( dtiki_t *tiki, const char *filename, dloadsurface_t *loadsurfaces, int *numSurfacesSetUp, msg_t *msg, qboolean skip, con_map<str, str> *keyValues )
{
	int c = 0;
	str key;
	str *val;
	const char *value;
	qboolean match = false;

	while( c != 14 )
	{
		c = MSG_ReadByte( msg );
		switch( c )
		{
		case 11:
			value = MSG_ReadString( msg );
			key = value;
			break;
		case 12:
			value = MSG_ReadString( msg );
			if( skip || match || !keyValues )
				break;

			val = keyValues->find( key );
			if( val && !stricmp( key.c_str(), val->c_str() ) )
			{
				match = true;
			}
			break;
		case 14:
		default:
			break;
		}
	}

	return TIKI_LoadSetupCase( tiki, filename, loadsurfaces, numSurfacesSetUp, msg, skip || !match, keyValues );
}

/*
===============
TIKI_LoadSetupCase
===============
*/
qboolean TIKI_LoadSetupCase( dtiki_t *tiki, const char *filename, dloadsurface_t *loadsurfaces, int *numSurfacesSetUp, msg_t *msg, qboolean skip, con_map<str, str> *keyValues )
{
	int c;
	const char *name;
	int currentSurface = -1;
	int mesh;
	skelHeaderGame_t *skelmodel;
	float load_scale;
	float lod_scale;
	float lod_bias;
	float load_origin[ 3 ];
	float light_offset[ 3 ];
	float radius;
	int flags;
	float damage_multiplier;

	while( 1 )
	{
		c = MSG_ReadByte( msg ) + 1;
		switch( c )
		{
		default:
			break;
		case 1:
			load_scale = MSG_ReadFloat( msg );
			if( skip )
				break;
			tiki->load_scale = load_scale;
			break;
		case 2:
			lod_scale = MSG_ReadFloat( msg );
			if( skip )
				break;
			tiki->lod_scale = lod_scale;
			break;
		case 3:
			lod_bias = MSG_ReadFloat( msg );
			if( skip )
				break;
			tiki->lod_bias = lod_bias;
			break;
		case 4:
			name = MSG_ReadString( msg );
			if( skip )
				break;

			if( tiki->numMeshes >= 12 )
			{
				TIKI_Error( "^~^~^ TIKI_LoadSetup: too many skelmodels in %s.\n", filename );
				return false;
			}

			mesh = TIKI_RegisterSkel( name, tiki );
			if( mesh < 0 )
				return false;

			tiki->mesh[ tiki->numMeshes ] = mesh;
			skelmodel = TIKI_GetSkel( mesh );
			tiki->num_surfaces += skelmodel->numSurfaces;
			tiki->numMeshes++;
			break;
		case 5:
			load_origin[ 0 ] = MSG_ReadFloat( msg );
			load_origin[ 1 ] = MSG_ReadFloat( msg );
			load_origin[ 2 ] = MSG_ReadFloat( msg );
			if( skip )
				break;
			VectorCopy( load_origin, tiki->load_origin );
			break;
		case 6:
			light_offset[ 0 ] = MSG_ReadFloat( msg );
			light_offset[ 1 ] = MSG_ReadFloat( msg );
			light_offset[ 2 ] = MSG_ReadFloat( msg );
			if( skip )
				break;
			VectorCopy( light_offset, tiki->load_origin );
			break;
		case 7:
			radius = MSG_ReadFloat( msg );
			if( skip )
				break;
			tiki->radius = radius;
			break;
		case 8:
			name = MSG_ReadString( msg );
			if( skip )
				break;

			for( currentSurface = 0; currentSurface < *numSurfacesSetUp; currentSurface++ )
			{
				if( !stricmp( loadsurfaces[ currentSurface ].name, name ) )
					break;
			}

			if( currentSurface == *numSurfacesSetUp )
			{
				loadsurfaces[ currentSurface ].flags = 0;
				( *numSurfacesSetUp )++;
			}
			strcpy( loadsurfaces[ currentSurface ].name, name );
			break;
		case 9:
			flags = MSG_ReadLong( msg );
			if( skip )
				break;
			loadsurfaces[ currentSurface ].flags |= flags;
			break;
		case 10:
			damage_multiplier = MSG_ReadFloat( msg );
			if( skip )
				break;
			loadsurfaces[ currentSurface ].damage_multiplier = damage_multiplier;
			break;
		case 11:
			name = MSG_ReadString( msg );
			if( skip )
				break;

			if( loadsurfaces[ currentSurface ].numskins > 3 )
			{
				TIKI_Error( "TIKI_ParseSetup: Too many skins defined for surface %s in %s.\n", loadsurfaces[ currentSurface ].name, filename );
				break;
			}

			strncpy( loadsurfaces[ currentSurface ].shader[ loadsurfaces[ currentSurface ].numskins ], name, sizeof( loadsurfaces[ currentSurface ].shader[ loadsurfaces[ currentSurface ].numskins ] ) );
			loadsurfaces[ currentSurface ].numskins++;
			break;
		case 14:
			if( !TIKI_LoadSetupCaseHeader( tiki, filename, loadsurfaces, numSurfacesSetUp, msg, skip, keyValues ) )
				return false;
			break;
		case 0:
		case 16:
			return true;
		}
	}
}

/*
===============
TIKI_LoadSetup
===============
*/
qboolean TIKI_LoadSetup( dtiki_t *tiki, const char *filename, dloadsurface_t *loadsurfaces, int *numSurfacesSetUp, byte *modelData, size_t modelDataSize, con_map<str, str> *keyValues )
{
	msg_t msg;

	MSG_Init( &msg, modelData, modelDataSize );
	msg.cursize = modelDataSize;
	MSG_BeginReading( &msg );

	memset( tiki, 0, sizeof( dtiki_t ) );
	memset( loadsurfaces, 0, sizeof( dloadsurface_t ) * 24 );
	*numSurfacesSetUp = 0;

	VectorCopy( vec3_origin, tiki->load_origin );
	VectorCopy( vec3_origin, tiki->light_offset );
	tiki->load_scale = 1.0f;
	tiki->lod_scale = 1.0f;
	tiki->lod_bias = 0.0f;
	tiki->numMeshes = 0;

	return TIKI_LoadSetupCase( tiki, filename, loadsurfaces, numSurfacesSetUp, &msg, false, keyValues );
}

/*
===============
TIKI_strstr
===============
*/
bool TIKI_strstr( const char *s, const char *substring )
{
	const char *t = strstr( s, substring );
	const char *w;

	if( !t || ( t != s && *( t - 1 ) != '\n' ) )
	{
		return false;
	}

	w = strstr( t, "\n" );
	if( w != ( t + strlen( substring ) ) )
	{
		return false;
	}

	return true;
}

/*
===============
TIKI_ParseSetup
===============
*/
qboolean TIKI_ParseSetup( dloaddef_t *ld )
{
	const char *token;
	float load_scale;
	float lod_scale;
	float lod_bias;
	float tmpFloat;
	float tmpVec[ 3 ];
	int tmpInt;
	size_t length;
	char name[ 128 ];

	// Skip the setup token
	ld->tikiFile.GetToken( true );

	while( ld->tikiFile.TokenAvailable( true ) )
	{
		token = ld->tikiFile.GetToken( true );

		if( !stricmp( token, "scale" ) )
		{
			load_scale = ld->tikiFile.GetFloat( false );
			WriteScale( ld, load_scale );
		}
		else if( !stricmp( token, "lod_scale" ) )
		{
			lod_scale = ld->tikiFile.GetFloat( false ) / 5.0f;
			WriteLoadScale( ld, lod_scale );
		}
		else if( !stricmp( token, "lod_bias" ) )
		{
			lod_bias = ld->tikiFile.GetFloat( false );
			WriteLodBias( ld, lod_bias );
		}
		else if( !stricmp( token, "skelmodel" ) )
		{
			token = ld->tikiFile.GetToken( false );
			strcpy( name, ld->tikiFile.currentScript->path );
			strcat( name, token );
			WriteSkelmodel( ld, name );
		}
		else if( !stricmp( token, "path" ) )
		{
			token = ld->tikiFile.GetToken( false );
			strcpy( ld->tikiFile.currentScript->path, token );
			length = strlen( ld->tikiFile.currentScript->path );
			token = ld->tikiFile.currentScript->path + length - 1;

			if( *token != '/' && *token != '\\' )
			{
				strcat( ld->tikiFile.currentScript->path, "/" );
			}
		}
		else if( !stricmp( token, "orgin" ) )
		{
			tmpVec[ 0 ] = ld->tikiFile.GetFloat( false );
			tmpVec[ 1 ] = ld->tikiFile.GetFloat( false );
			tmpVec[ 2 ] = ld->tikiFile.GetFloat( false );
			WriteOrigin( ld, tmpVec[ 0 ], tmpVec[ 1 ], tmpVec[ 2 ] );
		}
		else if( !stricmp( token, "lightoffset" ) )
		{
			tmpVec[ 0 ] = ld->tikiFile.GetFloat( false );
			tmpVec[ 1 ] = ld->tikiFile.GetFloat( false );
			tmpVec[ 2 ] = ld->tikiFile.GetFloat( false );
			WriteLightOffset( ld, tmpVec[ 0 ], tmpVec[ 1 ], tmpVec[ 2 ] );
		}
		else if( !stricmp( token, "radius" ) )
		{
			tmpFloat = ld->tikiFile.GetFloat( false );
			WriteRadius( ld, tmpFloat );
		}
		else if( !stricmp( token, "surface" ) )
		{
			token = ld->tikiFile.GetToken( false );
			WriteSurface( ld, token );
			while( ld->tikiFile.TokenAvailable( false ) )
			{
				token = ld->tikiFile.GetToken( false );
				if( !stricmp( token, "flags" ) )
				{
					token = ld->tikiFile.GetToken( false );
					tmpInt = TIKI_ParseSurfaceFlag( token );
					WriteFlags( ld, tmpInt );
				}
				else if( !stricmp( token, "damage" ) )
				{
					tmpFloat = ld->tikiFile.GetFloat( false );
					WriteDamage( ld, tmpFloat );
				}
				else if( !stricmp( token, "shader" ) )
				{
					token = ld->tikiFile.GetToken( false );
					if( strstr( token, "." ) )
					{
						strcpy( name, ld->tikiFile.currentScript->path );
						strcat( name, token );
						WriteShader( ld, name );
					}
					else
					{
						WriteShader( ld, token );
					}
				}
			}
		}
		else if( !stricmp( token, "ischaracter" ) )
		{
			ld->bIsCharacter = true;
		}
		else if( !stricmp( token, "case" ) )
		{
			if( !TIKI_ParseCase( ld ) )
				return false;
		}
		else if( !stricmp( token, "}" ) )
		{
			return true;
		}
		else
		{
			TIKI_Error( "TIKI_ParseSetup: unknown setup command '%s' in '%s' on line %d, skipping line.\n", token, ld->tikiFile.Filename(), ld->tikiFile.GetLineNumber() );
			while( ld->tikiFile.TokenAvailable( false ) )
				ld->tikiFile.GetToken( false );
		}
	}

	return true;
}

/*
===============
TIKI_ParseInitCommands
===============
*/
void TIKI_ParseInitCommands( dloaddef_t *ld, dloadinitcmd_t **cmdlist, int maxcmds, int *numcmds )
{
	int i;
	const char *token;
	char *pszArgs[ 256 ];
	dloadinitcmd_t *cmd;

	// Skip current token
	ld->tikiFile.GetToken( true );

	while( ld->tikiFile.TokenAvailable( true ) )
	{
		token = ld->tikiFile.GetToken( true );

		if( !stricmp( token, "}" ) )
		{
			break;
		}

		if( *numcmds < maxcmds )
		{
			cmd = ( dloadinitcmd_t * )TIKI_AllocateLoadData( sizeof( dloadinitcmd_t ) );
			cmdlist[ *numcmds ] = cmd;
			if( cmd )
			{
				( *numcmds )++;

				cmd->num_args = 0;
				cmd->args = NULL;

				ld->tikiFile.UnGetToken();
				while( ld->tikiFile.TokenAvailable( false ) )
				{
					token = ld->tikiFile.GetToken( false );
					if( cmd->num_args < 255 )
					{
						pszArgs[ cmd->num_args ] = TIKI_CopyString( token );
						cmd->num_args++;
					}
					else
					{
						TIKI_Error( "TIKI_ParseInitCommands: too many args in anim commands in %s.\n", ld->tikiFile.Filename() );
					}
				}

				cmd->args = ( char ** )TIKI_AllocateLoadData( cmd->num_args * sizeof( char * ) );
				for( i = 0; i < cmd->num_args; i++ )
				{
					cmd->args[ i ] = pszArgs[ i ];
				}
			}
			else
			{
				TIKI_Error( "TIKI_ParseInitCommands: could not allocate storage for dloadinitcmd_t in %s on line %d.\n", ld->tikiFile.Filename(), ld->tikiFile.GetLineNumber() );
			}
		}
	}
}

/*
===============
TIKI_ParseInit
===============
*/
void TIKI_ParseInit( dloaddef_t *ld )
{
	const char *token;

	// Skip the init token
	ld->tikiFile.GetToken( true );

	while( ld->tikiFile.TokenAvailable( true ) )
	{
		token = ld->tikiFile.GetToken( true );
		if( !stricmp( token, "client" ) )
		{
			TIKI_ParseInitCommands( ld, ld->loadclientinitcmds, 160, &ld->numclientinitcmds );
		}
		else if( !stricmp( token, "server" ) )
		{
			TIKI_ParseInitCommands( ld, ld->loadserverinitcmds, 160, &ld->numserverinitcmds );
		}
		else if( !stricmp( token, "}" ) )
		{
			break;
		}
		else
		{
			TIKI_Error( "TIKI_ParseInit: unknown init command %s in %s on line %d, skipping line.\n", token, ld->tikiFile.Filename(), ld->tikiFile.GetLineNumber() );

			// Skip the current line
			while( ld->tikiFile.TokenAvailable( false ) )
				ld->tikiFile.GetToken( false );
		}
	}

}

/*
===============
TIKI_ParseCase
===============
*/
qboolean TIKI_ParseCase( dloaddef_t *ld )
{
	const char *token;
	bool isheadmodel;
	bool isheadskin;

	WriteBeginCase( ld );

__newcase:
	token = ld->tikiFile.GetToken( false );
	WriteCaseKey( ld, token );
	isheadmodel = ( !stricmp( token, "headmodel" ) ) ? true : false;
	isheadskin = ( !stricmp( token, "headskin" ) ) ? true : false;

	while( 1 )
	{
		if( !ld->tikiFile.TokenAvailable( true ) )
		{
			TIKI_Error( "TIKI_ParseSetup: unexpected end of file while parsing 'case' switch in %s on line %d.\n", ld->tikiFile.Filename() , ld->tikiFile.GetLineNumber() );
			return 0;
		}

		token = ld->tikiFile.GetToken( true );
		if( !stricmp( token, "case" ) )
		{
			goto __newcase;
		}
		else if( !stricmp( token, "{" ) )
		{
			break;
		}

		WriteCaseValue( ld, token );

		if( isheadmodel && TIKI_strstr( ld->headmodels, token ) )
		{
			strcat( ld->headmodels, token );
			strcat( ld->headmodels, "\n" );
		}
	
		if( isheadskin && TIKI_strstr( ld->headskins, token ) )
		{
			strcat( ld->headskins, token );
			strcat( ld->headskins, "\n" );
		}
	}

	WriteBeginCaseBody( ld );
	ld->tikiFile.UnGetToken();
	if( !TIKI_ParseSetup( ld ) )
	{
		return false;
	}

	WriteEndCase( ld );
	return true;
}
