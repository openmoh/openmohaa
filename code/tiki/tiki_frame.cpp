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

// tiki_frame.cpp : TIKI Frame

#include "q_shared.h"
#include "qcommon.h"
#include "../skeletor/skeletor.h"
#include "../skeletor/tokenizer.h"
#include "../client/client.h"
#include <tiki.h>
#include "dbgheap.h"

/*
===============
TIKI_Frame_Commands_Server
===============
*/
qboolean TIKI_Frame_Commands_Server( dtiki_t *pmdl, int animnum, int framenum, tiki_cmd_t *tiki_cmd )
{
	dtikicmd_t *pcmd;
	dtikianimdef_t *panimdef;
	qboolean found;
	int i;

	tiki_cmd->num_cmds = 0;
	panimdef = pmdl->a->animdefs[ animnum ];
	found = false;

	for( i = 0; i < panimdef->num_server_cmds; i++ )
	{
		pcmd = &panimdef->server_cmds[ i ];

		if( ( pcmd->frame_num == framenum ) || ( pcmd->frame_num == TIKI_FRAME_EVERY && framenum >= 0 ) )
		{
			if( tiki_cmd->num_cmds >= TIKI_MAX_COMMANDS )
			{
				TIKI_Error( "TIKI_Frame_Commands_Client: Too many cmds for anim %d frame %d for %s.\n", animnum, framenum, pmdl->a->name );
				return false;
			}

			tiki_cmd->cmds[ tiki_cmd->num_cmds ].num_args = pcmd->num_args;
			tiki_cmd->cmds[ tiki_cmd->num_cmds ].args = pcmd->args;
			tiki_cmd->num_cmds++;

			found = true;
		}
	}

	return found;
}

/*
===============
TIKI_Frame_Commands_Client
===============
*/
qboolean TIKI_Frame_Commands_Client( dtiki_t *pmdl, int animnum, int framenum, tiki_cmd_t *tiki_cmd )
{
	dtikicmd_t *pcmd;
	dtikianimdef_t *panimdef;
	qboolean found;
	int i;

	tiki_cmd->num_cmds = 0;
	panimdef = pmdl->a->animdefs[ animnum ];
	found = false;

	for( i = 0; i < panimdef->num_client_cmds; i++ )
	{
		pcmd = &panimdef->client_cmds[ i ];

		if( ( pcmd->frame_num == framenum ) || ( pcmd->frame_num == TIKI_FRAME_EVERY && framenum >= 0 ) )
		{
			if( tiki_cmd->num_cmds >= TIKI_MAX_COMMANDS )
			{
				TIKI_Error( "TIKI_Frame_Commands_Client: Too many cmds for anim %d frame %d for %s.\n", animnum, framenum, pmdl->a->name );
				return false;
			}

			tiki_cmd->cmds[ tiki_cmd->num_cmds ].num_args = pcmd->num_args;
			tiki_cmd->cmds[ tiki_cmd->num_cmds ].args = pcmd->args;
			tiki_cmd->num_cmds++;

			found = true;
		}
	}

	return found;
}

/*
===============
TIKI_Frame_Commands_ClientTime
===============
*/
qboolean TIKI_Frame_Commands_ClientTime( dtiki_t *pmdl, int animnum, float start, float end, tiki_cmd_t *tiki_cmd )
{
	dtikicmd_t *pcmd;
	dtikianimdef_t *panimdef;
	qboolean found;
	int i;
	float cmdTime;

	tiki_cmd->num_cmds = 0;
	panimdef = pmdl->a->animdefs[ animnum ];
	found = false;

	for( i = 0; i < panimdef->num_client_cmds; i++ )
	{
		pcmd = &panimdef->client_cmds[ i ];
		cmdTime = pcmd->frame_num / 1000.0f;

		if( cmdTime >= start && cmdTime <= end )
		{
			if( tiki_cmd->num_cmds >= TIKI_MAX_COMMANDS )
			{
				TIKI_Error( "TIKI_Frame_Commands_Client: Too many cmds for anim %d times %f %f for %s.\n", animnum, start );
				return false;
			}

			tiki_cmd->cmds[ tiki_cmd->num_cmds ].num_args = pcmd->num_args;
			tiki_cmd->cmds[ tiki_cmd->num_cmds ].args = pcmd->args;
			tiki_cmd->num_cmds++;

			found = true;
		}
	}

	return true;
}
