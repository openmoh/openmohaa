/*
===========================================================================
Copyright (C) 2023 the OpenMoHAA team

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

// DESCRIPTION: 
// cg_servercmds.c -- text commands sent by the server

#include "cg_local.h"


/*
================
CG_ParseServerinfo

This is called explicitly when the gamestate is first received,
and whenever the server updates any serverinfo flagged cvars
================
*/
void CG_ParseServerinfo( void ) {
	const char	*info;
	const char	*mapname;
   char        map[ MAX_QPATH ];
   char        *spawnpos;

	info = CG_ConfigString( CS_SERVERINFO );
	cgs.gametype = atoi( Info_ValueForKey( info, "g_gametype" ) );
	cgs.dmflags = atoi( Info_ValueForKey( info, "dmflags" ) );
	cgs.teamflags = atoi( Info_ValueForKey( info, "teamflags" ) );
	cgs.fraglimit = atoi( Info_ValueForKey( info, "fraglimit" ) );
	cgs.timelimit = atoi( Info_ValueForKey( info, "timelimit" ) );
	cgs.maxclients = atoi( Info_ValueForKey( info, "sv_maxclients" ) );

	mapname = Info_ValueForKey( info, "mapname" );

   spawnpos = strchr( mapname, '$' );
   if ( spawnpos )
      {
      Q_strncpyz( map, mapname, spawnpos - mapname + 1 );
      }
   else
      {
      strcpy( map, mapname );
      }

	Com_sprintf( cgs.mapname, sizeof( cgs.mapname ), "maps/%s.bsp", map );
}


/*
================
CG_ConfigStringModified

================
*/
static void CG_ConfigStringModified( void )
   {	
	int		num;

	num = atoi( cgi.Argv( 1 ) );

	// get the gamestate from the client system, which will have the
	// new configstring already integrated
	cgi.GetGameState( &cgs.gameState );

   CG_ProcessConfigString( num );
   }

/*
===============
CG_MapRestart
===============
*/
static void CG_MapRestart( void ) {
	if ( cg_showmiss->integer ) {
		cgi.Printf( "CG_MapRestart\n" );
	}
   CG_Shutdown();
   CG_Init( &cgi, cgs.processedSnapshotNum, cgs.serverCommandSequence );
}

/*
=================
CG_ServerCommand

The string has been tokenized and can be retrieved with
Cmd_Argc() / Cmd_Argv()
=================
*/
static void CG_ServerCommand( void )
   {
	const char	*cmd;

	cmd = cgi.Argv(0);

	if ( !cmd[0] ) 
      {
		// server claimed the command
		return;
	   }

	if ( !strcmp( cmd, "cs" ) )
      {
		CG_ConfigStringModified();
		return;
	   }

	if ( !strcmp( cmd, "print" ) )
      {
		cgi.Printf( "%s", cgi.Argv(1) );
		return;
	   }

	if ( !strcmp( cmd, "map_restart" ) ) 
      {
		CG_MapRestart();
		return;
	   }

	cgi.Printf( "Unknown client game command: %s\n", cmd );
   }


/*
====================
CG_ExecuteNewServerCommands

Execute all of the server commands that were received along
with this this snapshot.
====================
*/
void CG_ExecuteNewServerCommands( int latestSequence ) {
	while ( cgs.serverCommandSequence < latestSequence ) {
		if ( cgi.GetServerCommand( ++cgs.serverCommandSequence ) ) {
			CG_ServerCommand();
		}
	}
}
