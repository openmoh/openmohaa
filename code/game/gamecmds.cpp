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

// gamecmds.cpp: Game commands
// 

#include "gamecmds.h"
#include "glb_local.h"
#include "camera.h"
#include "viewthing.h"
#include "soundman.h"
#include "navigate.h"
#include "lodthing.h"
#include "player.h"
#include <compiler.h>
#include "playerbot.h"

typedef struct
   {
   const char  *command;
   qboolean    ( *func )( gentity_t *ent );
   qboolean    allclients;
   } consolecmd_t;

consolecmd_t G_ConsoleCmds[] =
   {
   //   command name       function             available in multiplayer?
      { "say",				G_SayCmd,				qtrue },
      { "eventlist",		G_EventListCmd,			qfalse },
      { "pendingevents",	G_PendingEventsCmd,		qfalse },
      { "eventhelp",		G_EventHelpCmd,			qfalse },
      { "dumpevents",		G_DumpEventsCmd,		qfalse },
      { "classevents",		G_ClassEventsCmd,		qfalse },
      { "dumpclassevents",	G_DumpClassEventsCmd,	qfalse },
      { "dumpallclasses",	G_DumpAllClassesCmd,	qtrue },
      { "classlist",		G_ClassListCmd,			qfalse },
      { "classtree",		G_ClassTreeCmd,			qfalse },
      { "cam",				G_CameraCmd,			qfalse },
      { "snd",				G_SoundCmd,				qfalse },
      { "showvar",			G_ShowVarCmd,			qfalse },
      { "script",			G_ScriptCmd,			qfalse },
      { "levelvars",		G_LevelVarsCmd,			qfalse },
      { "gamevars",			G_GameVarsCmd,			qfalse },
	  { "compilescript",	G_CompileScript,		qfalse },
	  { "addbot",			G_AddBot,				qtrue },
	  { "removebot",		G_RemoveBot,			qtrue },
#ifdef _DEBUG
	  { "bot",				G_BotCommand,			qtrue },
#endif
      { NULL,				NULL,					NULL }
   };


void G_InitConsoleCommands
	(
	void
	)

{
	consolecmd_t *cmds;

	//
	// the game server will interpret these commands, which will be automatically
	// forwarded to the server after they are not recognized locally
	//
	gi.AddCommand( "give", NULL );
	gi.AddCommand( "god", NULL );
	gi.AddCommand( "notarget", NULL );
	gi.AddCommand( "noclip", NULL );
	gi.AddCommand( "kill", NULL );
	gi.AddCommand( "script", NULL );

	for( cmds = G_ConsoleCmds; cmds->command != NULL; cmds++ )
	{
		gi.AddCommand( cmds->command, NULL );
	}
}

qboolean G_ConsoleCommand ( void )
{
	gentity_t *ent;
	qboolean result;
	consolecmd_t *cmds;
	const char *cmd;

	result = qfalse;
	try
	{
		ent = &g_entities[ 0 ];

		cmd = gi.Argv( 0 );

		for( cmds = G_ConsoleCmds; cmds->command != NULL; cmds++ )
		{
			if( !Q_stricmp( cmd, cmds->command ) )
			{
				return cmds->func( ent );
			}
		}

		result = G_ProcessClientCommand( ent );
	}
	catch( const char *error )
	{
		G_ExitWithError( error );
	}

	return result;
}

void G_ClientCommand( gentity_t *ent )
{
	try
	{
		if ( ent && !G_ProcessClientCommand( ent ) )
		{
			// anything that doesn't match a command will be a chat
			//G_Say( ent, false, true );
		}
	}

	catch( const char *error )
	{
		G_ExitWithError( error );
	}
}

qboolean G_ProcessClientCommand
	(
	gentity_t *ent
	)

{
	const char		*cmd;
	consolecmd_t	*cmds;
	int				i;
	int				n;
	ConsoleEvent	*ev;
	Player			*player;

	if( !ent || !ent->client || !ent->entity )
	{
		// not fully in game yet
		return qfalse;
	}

	cmd = gi.Argv( 0 );

	player = ( Player * )ent->entity;
	player->m_lastcommand = cmd;

	for( cmds = G_ConsoleCmds; cmds->command != NULL; cmds++ )
	{
		// if we have multiple clients and this command isn't allowed by multiple clients, skip it
		if( ( game.maxclients > 1 ) && ( !cmds->allclients ) )
		{
			continue;
		}

		if( !Q_stricmp( cmd, cmds->command ) )
		{
			return cmds->func( ent );
		}
	}

	if( Event::FindEventNum( cmd ) )
	{
		ev = new ConsoleEvent( cmd );
		ev->SetConsoleEdict( ent );

		n = gi.Argc();

		for( i = 1; i < n; i++ )
		{
			ev->AddToken( gi.Argv( i ) );
		}

		if( !Q_stricmpn( cmd, "lod_", 4 ) )
		{
			return LODModel.ProcessEvent( ev );
		}
		else if( !Q_stricmpn( cmd, "view", 4 ) )
		{
			return Viewmodel.ProcessEvent( ev );
		}
		else if( ent->entity->CheckEventFlags( ev ) )
		{
			return ent->entity->ProcessEvent( ev );
		}
	}

	return qfalse;
}

/*
==================
Cmd_Say_f
==================
*/
void G_Say
	(
   gentity_t *ent,
	qboolean team,
	qboolean arg0
	)

	{
	int			j;
   gentity_t   *other;
	const char	*p;
	char			text[ 2048 ];

	if ( gi.Argc() < 2 && !arg0 )
		{
		return;
		}

   if ( !ent->entity )
      {
      // just in case we're not joined yet.
      team = false;
      }

	if ( !DM_FLAG( DF_MODELTEAMS | DF_SKINTEAMS ) )
		{
		team = false;
		}

	if ( team )
		{
		Com_sprintf( text, sizeof( text ), "(%s): ", ent->client->pers.netname );
		}
	else
		{
		Com_sprintf( text, sizeof( text ), "%s: ", ent->client->pers.netname );
		}

	if ( arg0 )
		{
		strcat( text, gi.Argv( 0 ) );
		strcat( text, " " );
		strcat( text, gi.Args() );
		}
	else
		{
		p = gi.Args();

		if ( *p == '"' )
			{
			p++;
			strcat( text, p );
			text[ strlen( text ) - 1 ] = 0;
			}
		else
			{
			strcat( text, p );
			}
		}

	// don't let text be too long for malicious reasons
	if ( strlen( text ) > 150 )
		{
		text[ 150 ] = 0;
		}

	strcat( text, "\n" );

   if ( dedicated->integer )
		{
      gi.SendServerCommand( NULL, "print \"%s\"", text );
		}

	for( j = 0; j < game.maxclients; j++ )
		{
      other = &g_entities[ j ];
		if ( !other->inuse || !other->client || !other->entity )
			{
			continue;
			}

      gi.SendServerCommand( NULL, "print \"%s\"", text );
		}
	}

qboolean G_CameraCmd
   (
   gentity_t *ent
   )

   {
   Event *ev;
	const char *cmd;
   int   i;
   int   n;

   n = gi.Argc();
   if ( !n )
      {
      gi.Printf( "Usage: cam [command] [arg 1]...[arg n]\n" );
      return qtrue;
      }

	cmd = gi.Argv( 1 );
  	if ( Event::GetEvent( cmd ) )
		{
		ev = new Event( cmd );

		for( i = 2; i < n; i++ )
			{
			ev->AddToken( gi.Argv( i ) );
			}

      CameraMan.ProcessEvent( ev );
      }
   else
      {
      gi.Printf( "Unknown camera command '%s'.\n", cmd );
      }

   return qtrue;
   }

qboolean G_SoundCmd
   (
   gentity_t *ent
   )

   {
   Event *ev;
	const char *cmd;
   int   i;
   int   n;

   n = gi.Argc();
   if ( !n )
      {
      gi.Printf( "Usage: snd [command] [arg 1]...[arg n]\n" );
      return qtrue;
      }

	cmd = gi.Argv( 1 );
  	if ( Event::GetEvent( cmd ) )
		{
		ev = new Event( cmd );

		for( i = 2; i < n; i++ )
			{
			ev->AddToken( gi.Argv( i ) );
			}

      SoundMan.ProcessEvent( ev );
      }
   else
      {
      gi.Printf( "Unknown sound command '%s'.\n", cmd );
      }

   return qtrue;
   }

qboolean G_SayCmd
   (
   gentity_t *ent
   )

   {
	G_Say( ent, false, false );

   return qtrue;
   }

qboolean G_EventListCmd
   (
   gentity_t *ent
   )

   {
   const char *mask;

   mask = NULL;
   if ( gi.Argc() > 1 )
      {
      mask = gi.Argv( 1 );
      }

	//Event::ListCommands( mask );

   return qtrue;
   }

qboolean G_PendingEventsCmd
   (
   gentity_t *ent
   )

   {
   const char *mask;

   mask = NULL;
   if ( gi.Argc() > 1 )
      {
      mask = gi.Argv( 1 );
      }

	//Event::PendingEvents( mask );

   return qtrue;
   }

qboolean G_EventHelpCmd
   (
   gentity_t *ent
   )

   {
   const char *mask;

   mask = NULL;
   if ( gi.Argc() > 1 )
      {
      mask = gi.Argv( 1 );
      }

	//Event::ListDocumentation( mask, false );

   return qtrue;
   }

qboolean G_DumpEventsCmd
   (
   gentity_t *ent
   )

   {
   const char *mask;

   mask = NULL;
   if ( gi.Argc() > 1 )
      {
      mask = gi.Argv( 1 );
      }

	//Event::ListDocumentation( mask, true );

   return qtrue;
   }

qboolean G_ClassEventsCmd
   (
   gentity_t *ent
   )

   {
   const char *className;

   className = NULL;
   if ( gi.Argc() < 2 )
      {
      gi.Printf( "Usage: classevents [className]\n" );
      className = gi.Argv( 1 );
      }
   else
      {
      className = gi.Argv( 1 );
	//ClassEvents( className );
      }
   return qtrue;
   }

qboolean G_DumpClassEventsCmd
   (
   gentity_t *ent
   )

   {
   const char *className;

   className = NULL;
   if ( gi.Argc() < 2 )
      {
      gi.Printf( "Usage: dumpclassevents [className]\n" );
      className = gi.Argv( 1 );
      }
   else
      {
      className = gi.Argv( 1 );
	//ClassEvents( className, qtrue );
      }
   return qtrue;
   }

qboolean G_DumpAllClassesCmd
   (
   gentity_t *ent
   )

   {
	DumpAllClasses();
   return qtrue;
   }

qboolean G_ClassListCmd
   (
   gentity_t *ent
   )

   {
	//listAllClasses();

   return qtrue;
   }

qboolean G_ClassTreeCmd
   (
   gentity_t *ent
   )

   {
   if ( gi.Argc() > 1 )
      {
	//listInheritanceOrder( gi.Argv( 1 ) );
      }
   else
      {
      gi.SendServerCommand( ent - g_entities, "print \"Syntax: classtree [classname].\n\"" );
      }

   return qtrue;
   }

qboolean G_ShowVarCmd
	(
	gentity_t *ent
	)

{
	return qtrue;
}

qboolean G_ScriptCmd( gentity_t *ent )
{
	if( gi.Argc() > 1 )
	{
		const char *script = gi.Argv( 1 );
		int recompile = false;

		if( gi.Argc() > 2 )
		{
			sscanf( gi.Argv( 2 ), "%d", &recompile );
		}

		Director.GetScript( script, recompile );
		Director.ExecuteThread( script );

		return qtrue;
	}
	else
	{
		gi.Printf( "Usage: script [filename] ([recompile])\n" );
		return qfalse;
	}
}

void PrintVariableList
	(
	ScriptVariableList * list
	)
{
}

qboolean G_LevelVarsCmd
   (
   gentity_t *ent
   )

   {
   gi.Printf( "Level Variables\n" );
   PrintVariableList( level.vars );

   return qtrue;
   }

qboolean G_GameVarsCmd
   (
   gentity_t *ent
   )

   {
   gi.Printf( "Game Variables\n" );
   PrintVariableList( game.vars );

   return qtrue;
   }

qboolean G_CompileScript
	(
	gentity_t *ent
	)

{
	if( gi.Argc() <= 2 )
	{
		gi.Printf( "Usage: compilescript [filename] [output file]\n" );
		return qfalse;
	}

	CompileAssemble( gi.Argv( 1 ), gi.Argv( 2 ) );
	return qtrue;
}

static gentity_t *firstBot = NULL;

qboolean G_AddBot
	(
	gentity_t *ent
	)

{
	int numbots;
	int n;
	int i;
	int clientNum = -1;
	gentity_t *e;
	char botName[ MAX_NETNAME ];
	char challenge[ MAX_STRING_TOKENS ];

	if( gi.Argc() <= 1 )
	{
		gi.Printf( "Usage: addbot [numbots] [optional botname]\n" );
		return qfalse;
	}

	numbots = atoi( gi.Argv( 1 ) );
	if( numbots <= 0 || numbots > maxbots->integer )
	{
		gi.Printf( "addbot must be between 1-%d\n", maxbots->integer );
		return qfalse;
	}

	for( n = 0; n < numbots; n++ )
	{
		for( i = maxclients->integer; i < game.maxclients; i++ )
		{
			e = &g_entities[ i ];

			if( !e->inuse && e->client )
			{
				clientNum = i;
				break;
			}
		}

		if( clientNum == -1 )
		{
			gi.Printf( "No free slot for a bot\n" );
			return qfalse;
		}

		if( gi.Argc() > 2 )
		{
			Q_strncpyz( botName, gi.Argv( 2 ), sizeof( botName ) );
		}
		else
		{
			sprintf( botName, "bot%d", clientNum - maxclients->integer + 1 );
		}

		sprintf( challenge, "%d", clientNum - maxclients->integer + 1 );

		e->s.clientNum = clientNum;
		e->s.number = clientNum;

		Info_SetValueForKey( e->client->pers.userinfo, "name", botName );
		Info_SetValueForKey( e->client->pers.userinfo, "dm_playermodel", "allied_pilot" );
		Info_SetValueForKey( e->client->pers.userinfo, "dm_playergermanmodel", "german_afrika_officer" );
		Info_SetValueForKey( e->client->pers.userinfo, "fov", "80" );
		Info_SetValueForKey( e->client->pers.userinfo, "protocol", "8" );
		Info_SetValueForKey( e->client->pers.userinfo, "ip", "0.0.0.0" );
		Info_SetValueForKey( e->client->pers.userinfo, "qport", "0" );
		Info_SetValueForKey( e->client->pers.userinfo, "challenge", challenge );
		Info_SetValueForKey( e->client->pers.userinfo, "snaps", "1" );
		Info_SetValueForKey( e->client->pers.userinfo, "rate", "1" );
		Info_SetValueForKey( e->client->pers.userinfo, "dmprimary", "smg" );

		G_BotConnect( clientNum );

		if( !firstBot )
			firstBot = e;

		G_BotBegin( e );

		e->entity->PostEvent( EV_Player_AutoJoinDMTeam, level.frametime );

		Event *ev = new Event( EV_Player_PrimaryDMWeapon );
		ev->AddString( "smg" );

		e->entity->PostEvent( ev, level.frametime );
	}

	return qtrue;
}

qboolean G_RemoveBot
	(
	gentity_t *ent
	)

{
	if( gi.Argc() <= 1 )
	{
		gi.Printf( "Usage: removebot [numbots]\n" );
		return qfalse;
	}

	for( int n = game.maxclients - 1; n >= maxclients->integer; n-- )
	{
		gentity_t *e = &g_entities[ n ];
		if( e->inuse && e->client )
		{
			G_ClientDisconnect( e );
		}
	}

	return qtrue;
}

#ifdef _DEBUG

qboolean G_BotCommand
	(
	gentity_t *ent
	)

{
	const char *command;
	PlayerBot *bot;

	if( !firstBot || !firstBot->entity )
	{
		gi.Printf( "No bot spawned.\n" );
		return qfalse;
	}

	if( gi.Argc() <= 1 )
	{
		gi.Printf( "Usage: bot [cmd] (arg1) (arg2) (arg3) ..." );
		return qfalse;
	}

	bot = ( PlayerBot * )firstBot->entity;

	command = gi.Argv( 1 );

	if( !Q_stricmp( command, "movehere" ) )
	{
		bot->MoveTo( ent->entity->origin );
	}
	else if( !Q_stricmp( command, "moveherenear" ) )
	{
		float rad = 256.0f;

		if( gi.Argc() > 2 )
		{
			rad = atof( gi.Argv( 2 ) );
		}

		bot->MoveNear( ent->entity->origin, rad );
	}
	else if( !Q_stricmp( command, "avoidhere" ) )
	{
		float rad = 256.0f;

		if( gi.Argc() > 2 )
		{
			rad = atof( gi.Argv( 2 ) );
		}

		bot->AvoidPath( ent->entity->origin, rad );
	}
	else if( !Q_stricmp( command, "telehere" ) )
	{
		bot->setOrigin( ent->s.origin );
	}

	return qtrue;
}

#endif
