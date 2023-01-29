/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
//
#include "g_local.h"
#include "game.h"


/*
=======================================================================

  SESSION DATA

Session data is the only data that stays persistant across level loads
and tournament restarts.
=======================================================================
*/

bool g_bNewSession = false;

/*
================
G_WriteClientSessionData

Called on game shutdown
================
*/
void G_WriteClientSessionData( gclient_t *client )
{
	gi.Cvar_Set( va( "session%i", ( int )( client - game.clients ) ),
		va( "%s %i %i", client->pers.weapon[ 0 ] ? client->pers.weapon : "-",
		client->pers.team,
		client->pers.kills ) );
}

/*
================
G_ReadSessionData

Called on a reconnect
================
*/
void G_ReadSessionData( gclient_t *client )
{
	cvar_t *session;

	if( g_bNewSession ) {
		return;
	}

	session = gi.Cvar_Get( va( "session%i", client - game.clients ), "", 0 );
	
	sscanf( session->string, "%s %i %i", &client->pers.weapon, &client->pers.team, &client->pers.kills );
	if( client->pers.weapon[ 0 ] == '-' )
	{
		client->pers.weapon[ 0 ] = 0;
	}
}


/*
================
G_InitSessionData

Called on a first-time connect
================
*/
void G_InitSessionData( gclient_t *client, char *userinfo )
{
	G_WriteClientSessionData( client );
}


/*
==================
G_InitWorldSession

==================
*/
void G_InitWorldSession( void ) {
	char	s[MAX_STRING_CHARS];
	int			gt;

	cvar_t *v = gi.Cvar_Get( "session", "", 0 );

	gt = atoi( s );
	
	// if the gametype changed since the last session, don't use any
	// client sessions
	if ( g_gametype->integer != gt ) {
		G_Printf( "Gametype changed, clearing session data.\n" );
	}
}

/*
==================
G_WriteSessionData

==================
*/
void G_WriteSessionData( void ) {
	int		i;

	gi.Cvar_Set( "session", va( "%i", g_gametype->integer ) );

	for ( i = 0 ; i < game.maxclients ; i++ )
	{
		if ( game.clients[i].pers.enterTime != 0.0f )
		{
			G_WriteClientSessionData( &game.clients[i] );
		}
	}
}
