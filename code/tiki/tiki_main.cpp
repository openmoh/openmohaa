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
#include "tiki_shared.h"
#include "dbgheap.h"

int cache_numskel = 0;
int cache_maxskel = 0;
skelcache_t skelcache[ TIKI_MAX_SKELCACHE ];

/*
===============
TIKI_AddPointToBounds
===============
*/
void TIKI_AddPointToBounds( float *v, float *mins, float *maxs )
{
	int i;
	vec_t val;

	for( i = 0; i < 3; i++ )
	{
		val = v[ i ];

		if( val < mins[ i ] )
		{
			mins[ i ] = val;
		}

		if( val > maxs[ i ] )
		{
			maxs[ i ] = val;
		}
	}
}

/*
===============
TIKI_Message
===============
*/
void TIKI_Message( const char *fmt, ... )
{
	char msg[ 1024 ];
	va_list va;

	va_start( va, fmt );
	vsprintf( msg, fmt, va );
	va_end( va );
	Skel_DPrintf( msg );
}

/*
===============
TIKI_Warning
===============
*/
void TIKI_Warning( const char *fmt, ... )
{
	char msg[ 1024 ];
	va_list va;

	va_start( va, fmt );
	vsprintf( msg, fmt, va );
	va_end( va );
	Skel_DPrintf( msg );
}

/*
===============
TIKI_Error
===============
*/
void TIKI_Error( const char *fmt, ... )
{
	char msg[ 1024 ];
	va_list va;

	va_start( va, fmt );
	vsprintf( msg, fmt, va );
	va_end( va );
	Skel_DPrintf( msg );
}

/*
===============
TIKI_SetupIndividualSurface
===============
*/
void TIKI_SetupIndividualSurface( const char *filename, dtikisurface_t *surf, const char *name, dloadsurface_t *loadsurf )
{
	int j;

	surf->numskins = 0;
	for( j = 0; j < loadsurf->numskins; j++ )
	{
		if( surf->numskins >= MAX_TIKI_SHADER )
		{
			TIKI_Error( "TIKI_SetupIndividualSurface: Too many skins defined for surface %s in %s.\n", loadsurf->name, filename );
		}
		else
		{
			strncpy( surf->name, name, sizeof( surf->name ) );
			strncpy( surf->shader[ surf->numskins ], loadsurf->shader[ j ], sizeof( surf->shader[ surf->numskins ] ) );
			surf->numskins++;
		}
	}

	surf->flags = loadsurf->flags;
	surf->damage_multiplier = loadsurf->damage_multiplier;
}

/*
===============
TIKI_CalcAnimDefSize
===============
*/
size_t TIKI_CalcAnimDefSize( dloaddef_t *ld )
{
	int i, j, k;
	size_t defsize = ld->numclientinitcmds * sizeof( dtikicmd_t ) + ( ld->numserverinitcmds * 3 + ld->numanims ) + sizeof( dtikianimdef_t );

	for( i = 0; i < ld->numserverinitcmds; i++ )
	{
		defsize += sizeof( char * );

		for( j = 0; j < ld->loadserverinitcmds[ i ]->num_args; j++ )
		{
			defsize += strlen( ld->loadserverinitcmds[ i ]->args[ j ] ) + 1;
		}
	}

	for( i = 0; i < ld->numclientinitcmds; i++ )
	{
		defsize += sizeof( char * );

		for( j = 0; j < ld->loadclientinitcmds[ i ]->num_args; j++ )
		{
			defsize += strlen( ld->loadclientinitcmds[ i ]->args[ j ] ) + 1;
		}
	}

	for( i = 0; i < ld->numanims; i++ )
	{
		defsize += ld->loadanims[ i ]->num_server_cmds * sizeof( dtikicmd_t ) + sizeof( dtikianimdef_t );

		for( j = 0; j < ld->loadanims[ i ]->num_server_cmds; j++ )
		{
			defsize += ld->loadanims[ i ]->loadservercmds[ j ]->num_args * sizeof( char * );
			for( k = 0; k < ld->loadanims[ i ]->loadservercmds[ j ]->num_args; k++ )
			{
				defsize += strlen( ld->loadanims[ i ]->loadservercmds[ j ]->args[ k ] ) + 1;
			}
		}
	}

	defsize += ld->modelBuf->cursize + strlen( ld->headmodels ) + 1 + strlen( ld->headskins ) + 1;
	return defsize;
}

/*
===============
TIKI_FillTIKIStructureSkel
===============
*/
dtikianim_t *TIKI_FillTIKIStructureSkel( dloaddef_t *ld )
{
	size_t animdefsize;
	size_t defsize;

	animdefsize = TIKI_CalcAnimDefSize( ld );
	defsize = strlen( ld->path );

	// FIXME: what is 477 ?
	return TIKI_InitTiki( ld, animdefsize + defsize + sizeof( dloaddef_t ) + 477 );
}
