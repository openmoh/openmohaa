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

// game.cpp : General Game Info.

#include "game.h"
#include "scriptmaster.h"
#include "world.h"

#include "camera.h"
#include "entity.h"
#include "player.h"
#include "dm_manager.h"

Game game;

void G_BeginIntermission
	(
	const char *map_name,
	INTTYPE_e transtype,
	bool no_fade 
	)
{
	Entity *camera;
	Entity *node;
	Event *ev;
	gentity_t *client;
	int i;

	if( level.intermissiontime || g_gametype->integer ) {
		return;
	}

	level.intermissiontime = level.time;
	level.intermissiontype = transtype;

	if (!no_fade) {
		G_FadeOut(2.0f);
	}

	G_FadeSound( 4.0f );

	level.nextmap = map_name;

	camera = ( Entity * )G_FindClass( NULL, "info_player_intermission" );
	if( camera )
	{
		SetCamera( camera, 0.5f );

		ev = new Event( EV_Camera_Orbit );

		node = ( Entity * )G_FindTarget( NULL, "endnode1" );
		if( node && node->IsSubclassOfEntity() )
		{
			ev->AddEntity( node );
			camera->ProcessEvent( ev );
			camera->ProcessEvent( EV_Camera_Cut );
		}
	}

	for( i = 0, client = g_entities; i < game.maxclients; client++, i++ )
	{
		if( !client->inuse || !client->entity ) {
			continue;
		}

		client->entity->flags |= FL_IMMOBILE;
		client->entity->PostEvent( EV_Player_EnterIntermission, 3.0f );
	}
}

void G_BeginIntermission2( void )
{
	gentity_t *client;
	Entity *ent;
	int i;

	if( level.intermissiontime ) {
		return;
	}

	level.playerfrozen = qtrue;
	level.intermissiontime = level.time;

	ent = ( Entity * )G_FindClass( NULL, "info_player_intermission" );

	G_FadeSound( 4.0f );

	if( ent )
	{
		SetCamera( ent, 0.5f );
	}
	else
	{
		G_FadeOut( 2.0f );
	}

	for( i = 0, client = g_entities; i < game.maxclients; i++, client++ )
	{
		if( !client->inuse || !client->entity || !client->client ) {
			continue;
		}

		ent = client->entity;

		G_DisplayScores( ent );
		ent->flags |= FL_IMMOBILE;
	}
}

void G_ExitIntermission( void )
{
	level.exitintermission = qtrue;
}

void G_CheckIntermissionExit( void )
{
	if( !level.exitintermission && g_maxintermission->value > level.time - level.intermissiontime ) {
		return;
	}

	if( level.nextmap != level.current_map )
	{
		G_ExitLevel();
	}
	else
	{
		G_RestartLevelWithDelay( 0.1f );

		level.nextmap = "";
		level.intermissiontime = 0;
		level.exitintermission = qfalse;
	}
}

void ClosePlayerLogFile( void );

void G_ExitLevel
	(
	void
	)
{
	static const char *seps = " ,\n\r";
	char command[ 256 ];
	int j;
	gentity_t *ent;


	// Don't allow exit level if the mission was failed

	if( level.mission_failed )
		return;

	// close the player log file if necessary
	ClosePlayerLogFile();

	// kill the sounds
	Com_sprintf( command, sizeof( command ), "stopsound\n" );
	gi.SendConsoleCommand( command );

	if( g_gametype->integer )
	{
		if( strlen( sv_nextmap->string ) )
		{
			// The nextmap cvar was set (possibly by a vote - so go ahead and use it)
			level.nextmap = sv_nextmap->string;
			gi.cvar_set( "nextmap", "" );
		}
		else // Use the next map in the maplist
		{
			char *s, *f, *t;

			f = NULL;
			s = strdup( sv_maplist->string );
			t = strtok( s, seps );
			while( t != NULL )
			{
				if( !Q_stricmp( t, level.mapname.c_str() ) )
				{
					// it's in the list, go to the next one
					t = strtok( NULL, seps );
					if( t == NULL ) // end of list, go to first one
					{
						if( f == NULL ) // there isn't a first one, same level
						{
							level.nextmap = level.mapname;
						}
						else
						{
							level.nextmap = f;
						}
					}
					else
					{
						level.nextmap = t;
					}
					free( s );
					goto out;
				}

				// set the first map
				if( !f )
				{
					f = t;
				}
				t = strtok( NULL, seps );
			}
			free( s );
		}
out:
		// level.nextmap should be set now, but if it isn't use the same map
		if( level.nextmap.length() == 0 )
		{
			// Stay on the same map since no nextmap was set
			Com_sprintf( command, sizeof( command ), "restart\n" );
			gi.SendConsoleCommand( command );
		}
		else // use the level.nextmap variable
		{
			Com_sprintf( command, sizeof( command ), "gamemap \"%s\"\n", level.nextmap.c_str() );
			gi.SendConsoleCommand( command );
		}
	}
	else
	{
		Com_sprintf( command, sizeof( command ), "gamemap \"%s\"\n", level.nextmap.c_str() );
		gi.SendConsoleCommand( command );
	}

	// Tell all the clients that the level is done
	for( j = 0; j < game.maxclients; j++ )
	{
		ent = &g_entities[ j ];
		if( !ent->inuse || !ent->entity )
		{
			continue;
		}

		ent->entity->ProcessEvent( EV_Player_EndLevel );
	}

	level.nextmap = "";

	level.exitintermission = 0;
	level.intermissiontime = 0;

	G_ClientEndServerFrames();
}

void G_RestartLevelWithDelay( float delaytime )
{
	int i;

	if( level.died_already ) {
		return;
	}

	level.died_already = true;

	// Restart the level soon
	for( i = 0; i < game.maxclients; i++ )
	{
		if( g_entities[ i ].inuse )
		{
			if( g_entities[ i ].entity )
			{
				g_entities[ i ].entity->PostEvent( EV_Player_Respawn, delaytime );
			}
		}
	}
}

void G_UpdateMatchEndTime( void )
{
	int endtime = 0;

	if( dmManager.GameHasRounds() && dmManager.GetRoundLimit() )
	{
		endtime = dmManager.GetMatchStartTime() * 1000.0f + ( level.svsStartTime + 60000 * dmManager.GetRoundLimit() );
	}
	else if( timelimit->integer )
	{
		endtime = level.svsStartTime + 60000 * timelimit->integer;
	}

	if( level.svsEndTime != endtime )
	{
		level.svsEndTime = endtime;
		gi.setConfigstring( CS_MATCHEND, va( "%i", endtime ) );
	}
}

int G_GetClientNumber( gentity_t *ent )
{
	return ent->client->ps.clientNum;
}

int G_GetClientNumber( Entity *entity )
{
	return 0;
}

SimpleArchivedEntity *G_FindArchivedClass( SimpleArchivedEntity *ent, const char *classname )
{
	SimpleArchivedEntity *arcent;
	int			i;

	if( ent )
	{
		i = level.m_SimpleArchivedEntities.IndexOfObject( ( SimpleArchivedEntity * )ent ) - 1;
	}
	else
	{
		i = level.m_SimpleArchivedEntities.NumObjects();
	}

	for( ; i > 0; i-- )
	{
		arcent = level.m_SimpleArchivedEntities.ObjectAt( i );

		if( !Q_stricmp( arcent->getClassID(), classname ) )
		{
			return arcent;
		}
	}

	return NULL;
}

Entity *G_GetEntityByClient( int clientNum )
{
	gentity_t * ent = globals.gentities;
	if ( clientNum < 0 || clientNum > globals.max_entities )
	{
		gi.DPrintf( "G_GetEntity: %d out of valid range.", clientNum );
		return NULL;
	}

	for( int i = 0; i < globals.num_entities; i++, ent++ )
	{
		if ( ent->s.clientNum == clientNum )
			break;
	}

	return ent->entity;
}

int G_GetEntityIndex( int number )
{
	gentity_t * ent = globals.gentities;

	if( number < 0 || number > globals.max_entities ) {
		return -1;
	}

	for( int i = 0; i < globals.num_entities; i++, ent++ )
	{
		if( ent->s.number == number ) {
			return i;
		}
	}

	return -1;
}

int G_GetEntityIndex( gentity_t * ent )
{
	return G_GetEntityIndex( ent->s.number );
}

gentity_t * G_GetGEntity( int ent_num )
{
	gentity_t * ent = &globals.gentities[ ent_num ];

	if( ent_num < 0 || ent_num > globals.max_entities )
	{
		gi.DPrintf( "G_GetEntity: %d out of valid range.\n", ent_num );
		return NULL;
	}

	return ent;
}

SimpleEntity *G_FindTarget( SimpleEntity *ent, const char *name )
{
	SimpleEntity      *next;

	if( name && name[ 0 ] )
	{
		if( name[ 0 ] == '$' )
			name++;

		next = world->GetNextEntity( name, ent );

		if( next )
		{
			return next;
		}
	}

	return NULL;
}

SimpleEntity *G_FindRandomSimpleTarget( const char *name )
{
	SimpleEntity *found = NULL, *ent = world;
	int nFound = 0;

	if (name && *name)
	{
		while (true)
		{
			ent = world->GetNextEntity(name, ent);
			if (!ent)
			{
				break;
			}

			if (++nFound * rand() <= 0x7FFF)
			{
				found = ent;
			}
		}
	}
	return found;
}

Entity *G_FindRandomTarget( const char *name )
{
	SimpleEntity *found = NULL, *ent = world;
	int nFound = 0;

	if (name && *name)
	{
		while (true)
		{
			ent = world->GetNextEntity(name, ent);
			if (!ent)
			{
				break;
			}

			if (ent->IsSubclassOfEntity() && ++nFound * rand() <= 0x7FFF)
			{
				found = ent;
			}
		}
	}
	return (Entity *)found;
}

Game::Game()
{
	clients = NULL;
	maxclients = 0;
}

Game::~Game()
{

}

void Game::Archive( Archiver& arc )
{

}

CLASS_DECLARATION( Listener, Game, NULL )
{
	{ NULL, NULL }
};
