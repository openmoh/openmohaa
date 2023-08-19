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
// g_bot.cpp

#include "g_local.h"
#include "entity.h"
#include "playerbot.h"
#include "g_bot.h"

static gentity_t* firstBot = NULL;
static saved_bot_t* saved_bots = NULL;
static unsigned int current_bot_count = 0;

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

void G_AddBot(unsigned int num, saved_bot_t* saved)
{
	int n;
	int i;
	int clientNum = -1;
	gentity_t *e;
	char botName[ MAX_NETNAME ];
	char challenge[ MAX_STRING_TOKENS ];
	char userinfo[MAX_INFO_STRING]{ 0 };
	Event* teamEv;

	num = Q_min(num, sv_maxbots->integer);
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

        if (saved) {
			strncpy(userinfo, saved->pers.userinfo, ARRAY_LEN(userinfo));
		} else {
            Info_SetValueForKey(userinfo, "name", botName);
            Info_SetValueForKey(userinfo, "dm_playermodel", "allied_pilot");
            Info_SetValueForKey(userinfo, "dm_playergermanmodel", "german_afrika_officer");
            Info_SetValueForKey(userinfo, "fov", "80");
            Info_SetValueForKey(userinfo, "protocol", "8");
            Info_SetValueForKey(userinfo, "ip", "0.0.0.0");
            Info_SetValueForKey(userinfo, "qport", "0");
            Info_SetValueForKey(userinfo, "challenge", challenge);
            Info_SetValueForKey(userinfo, "snaps", "1");
            Info_SetValueForKey(userinfo, "rate", "1");
            Info_SetValueForKey(userinfo, "dmprimary", "smg");
		}

		current_bot_count++;

		G_BotConnect( clientNum, userinfo );

		if (saved) {
			e->client->pers = saved->pers;
		}

		if( !firstBot )
			firstBot = e;

		G_BotBegin( e );

		if (saved) {
			/*
			switch (saved->team)
			{
            case TEAM_ALLIES:
                teamEv = new Event(EV_Player_JoinDMTeam);
				teamEv->AddString("allies");
				break;
            case TEAM_AXIS:
                teamEv = new Event(EV_Player_JoinDMTeam);
                teamEv->AddString("axis");
				break;
            default:
				teamEv = new Event(EV_Player_AutoJoinDMTeam);
				break;
			}
			*/
		} else {
            teamEv = new Event(EV_Player_AutoJoinDMTeam);
            e->entity->PostEvent(teamEv, level.frametime);

            Event* ev = new Event(EV_Player_PrimaryDMWeapon);
            ev->AddString("smg");

            e->entity->PostEvent(ev, level.frametime);
        }
	}
}

void G_RemoveBot(unsigned int num)
{
	num = Q_min(atoi(gi.Argv(1)), sv_maxbots->integer);

	for( int n = 0; n < num; n++ )
	{
		gentity_t *e = &g_entities[ game.maxclients - 1 - n ];
		if( e->inuse && e->client )
		{
			G_ClientDisconnect( e );
			current_bot_count--;
		}
	}
}

void G_SaveBots() {
	unsigned int n;

    if (saved_bots) {
        delete[] saved_bots;
		saved_bots = NULL;
    }

	if (!current_bot_count) {
		return;
	}

	saved_bots = new saved_bot_t[current_bot_count];
    for (n = 0; n < current_bot_count; n++) {
        gentity_t* e = &g_entities[game.maxclients - sv_maxbots->integer + n];
		saved_bot_t& saved = saved_bots[n];

		if (e->inuse && e->client)
		{
			Player* player = static_cast<Player*>(e->entity);

			saved.bValid = true;
			//saved.team = player->GetTeam();
			saved.pers = player->client->pers;
		}
	}
}

void G_RestoreBots() {
	unsigned int n;

    if (!saved_bots) {
		return;
    }

    for (n = 0; n < sv_numbots->integer; n++) {
        saved_bot_t& saved = saved_bots[n];

		G_AddBot(1, &saved);
	}

	delete[] saved_bots;
	saved_bots = NULL;
}

void G_ResetBots() {
	G_SaveBots();

	current_bot_count = 0;
}

void G_SpawnBots() {
	if (saved_bots) {
		G_RestoreBots();
	}

	if (sv_numbots->integer > current_bot_count) {
		G_AddBot(sv_numbots->integer - current_bot_count);
	} else if (sv_numbots->integer < current_bot_count) {
		G_RemoveBot(current_bot_count - sv_numbots->integer);
	}
}
