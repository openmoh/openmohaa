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
// g_bot.cpp

#include "g_local.h"
#include "entity.h"
#include "playerbot.h"

static gentity_t* firstBot = NULL;

void G_BotBegin
	(
	gentity_t *ent
	)
{
	level.m_bSpawnBot = true;
	G_ClientBegin( ent, NULL );
}

void G_BotThink
	(
	gentity_t *ent,
	int msec
	)
{
	usercmd_t ucmd;
	usereyes_t eyeinfo;
	PlayerBot *bot;

	assert( ent );
	assert( ent->entity );
	assert( ent->entity->IsSubclassOfBot() );

	bot = ( PlayerBot * )ent->entity;

	bot->UpdateBotStates();
	bot->GetUsercmd( &ucmd );
	bot->GetEyeInfo( &eyeinfo );

	G_ClientThink( ent, &ucmd, &eyeinfo );
}

gentity_t* G_GetFirstBot()
{
	return firstBot;
}

void G_AddBot(unsigned int num)
{
	int n;
	int i;
	int clientNum = -1;
	gentity_t *e;
	char botName[ MAX_NETNAME ];
	char challenge[ MAX_STRING_TOKENS ];

	num = Q_min(num, maxbots->integer);
	for( n = 0; n < num; n++ )
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
			return;
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
}

void G_RemoveBot(unsigned int num)
{
	num = Q_min(atoi(gi.Argv(1)), maxbots->integer);

	for( int n = 0; n < num; n++ )
	{
		gentity_t *e = &g_entities[ game.maxclients - 1 - n ];
		if( e->inuse && e->client )
		{
			G_ClientDisconnect( e );
		}
	}
}
