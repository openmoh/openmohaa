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
#include "tiki.h"
#include "../tiki/tiki_shared.h"

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
size_t TIKI_CalcAnimDefSize(dloaddef_t* ld)
{
	int i, j, k;

	// initial cmds
	size_t defsize = 0;
	// other animations
	defsize += sizeof(dtikianimdef_t) * ld->numanims;
	defsize += ld->numclientinitcmds * sizeof(dtikicmd_t);
	defsize += ld->numserverinitcmds * sizeof(dtikicmd_t);

	for (i = 0; i < ld->numserverinitcmds; i++)
	{
		const dloadinitcmd_t* initcmd = ld->loadserverinitcmds[i];
		defsize += sizeof(*initcmd->args) * initcmd->num_args;

		for (j = 0; j < initcmd->num_args; j++)
		{
			defsize += strlen(initcmd->args[j]) + 1;
		}

		defsize = PAD(defsize, sizeof(void*));
	}

	for (i = 0; i < ld->numclientinitcmds; i++)
	{
		const dloadinitcmd_t* initcmd = ld->loadclientinitcmds[i];
		defsize += sizeof(*initcmd->args) * initcmd->num_args;

		for (j = 0; j < initcmd->num_args; j++)
		{
			defsize += strlen(initcmd->args[j]) + 1;
		}

		defsize = PAD(defsize, sizeof(void*));
	}

	for (i = 0; i < ld->numanims; i++)
	{
		const dloadanim_t* loadanim = ld->loadanims[i];

		defsize += sizeof(dtikianimdef_t);
		defsize += loadanim->num_server_cmds * sizeof(dtikicmd_t);
		defsize += loadanim->num_client_cmds * sizeof(dtikianimdef_t);

		for (j = 0; j < loadanim->num_server_cmds; j++)
		{
			const dloadframecmd_t* loadframecmd = loadanim->loadservercmds[j];
			defsize += sizeof(*loadframecmd->args) * loadframecmd->num_args;

			for (k = 0; k < loadframecmd->num_args; k++)
			{
				defsize += strlen(loadframecmd->args[k]) + 1;
			}

			defsize = PAD(defsize, sizeof(void*));
		}

		for (j = 0; j < loadanim->num_client_cmds; j++)
		{
			const dloadframecmd_t* loadframecmd = loadanim->loadclientcmds[j];
			defsize += sizeof(*loadframecmd->args) * loadframecmd->num_args;

			for (k = 0; k < loadframecmd->num_args; k++)
			{
				defsize += strlen(loadframecmd->args[k]) + 1;
			}

			defsize = PAD(defsize, sizeof(void*));
		}

		defsize = PAD(defsize, sizeof(void*));
	}

	defsize += ld->modelBuf->cursize + strlen(ld->headmodels) + 1 + strlen(ld->headskins) + 1;
	defsize = PAD(defsize, sizeof(void*));
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

	animdefsize = TIKI_CalcAnimDefSize(ld);
	defsize = sizeof(dtikianim_t);
	// anim name
	defsize += strlen(ld->path) + 1;

	return TIKI_InitTiki(ld, PAD(animdefsize + defsize, sizeof(void*)));
}
