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
#include "player.h"
#include "playerbot.h"
#include "playerstart.h"
#include "scriptmaster.h"

// g_client.c -- client functions that don't happen every frame

static vec3_t playerMins = {-15, -15, -24};
static vec3_t playerMaxs = {15, 15, 32};

// FIXME: OLD Q3 CODE

#if 0

/*QUAKED info_player_deathmatch (1 0 1) (-16 -16 -24) (16 16 32) initial
potential spawning position for deathmatch games.
The first time a player enters the game, they will be at an 'initial' spot.
Targets will be fired when someone spawns in on them.
"nobots" will prevent bots from using this spot.
"nohumans" will prevent non-bots from using this spot.
*/
void SP_info_player_deathmatch( gentity_t *ent ) {
	int		i;

	G_SpawnInt( "nobots", "0", &i);
	if ( i ) {
		ent->flags |= FL_NO_BOTS;
	}
	G_SpawnInt( "nohumans", "0", &i );
	if ( i ) {
		ent->flags |= FL_NO_HUMANS;
	}
}

/*QUAKED info_player_start (1 0 0) (-16 -16 -24) (16 16 32)
equivelant to info_player_deathmatch
*/
void SP_info_player_start(gentity_t *ent) {
	ent->classname = "info_player_deathmatch";
	SP_info_player_deathmatch( ent );
}

/*QUAKED info_player_intermission (1 0 1) (-16 -16 -24) (16 16 32)
The intermission will be viewed from this point.  Target an info_notnull for the view direction.
*/
void SP_info_player_intermission( gentity_t *ent ) {

}

/*
=======================================================================

BODYQUE

=======================================================================
*/

/*
===============
InitBodyQue
===============
*/
void InitBodyQue( void ) {
	int		i;
	gentity_t	*ent;

	level.bodyQueIndex = 0;
	for( i = 0; i<BODY_QUEUE_SIZE; i++ ) {
		ent = G_Spawn();
		ent->classname = "bodyque";
		ent->neverFree = qtrue;
		level.bodyQue[ i ] = ent;
	}
}

/*
=============
BodySink

After sitting around for five seconds, fall into the ground and dissapear
=============
*/
void BodySink( gentity_t *ent ) {
	if( level.time - ent->timestamp > 6500 ) {
		// the body ques are never actually freed, they are just unlinked
		gi.UnlinkEntity( ent );
		ent->physicsObject = qfalse;
		return;
	}
	ent->nextthink = level.time + 100;
	ent->s.origin[ 2 ] -= 1;
}

/*
=============
CopyToBodyQue

A player is respawning, so make an entity that looks
just like the existing corpse to leave behind.
=============
*/
void CopyToBodyQue( gentity_t *ent ) {

}

/*
================
respawn
================
*/
void respawn( gentity_t *ent ) {
	//CopyToBodyQue (ent);
	ClientSpawn(ent);
}

/*
================
TeamCount

Returns number of players on a team
================
*/
teamtype_t TeamCount( int ignoreClientNum, int team ) {
	int		i;
	int		count = 0;

	for ( i = 0 ; i < game.maxclients ; i++ ) {
		if ( i == ignoreClientNum ) {
			continue;
		}
		if ( game.clients[i].pers.connected == CON_DISCONNECTED ) {
			continue;
		}
		if ( game.clients[i].sess.sessionTeam == team ) {
			count++;
		}
	}

	return ( teamtype_t )count;
}

/*
================
TeamLeader

Returns the client number of the team leader
================
*/
int TeamLeader( int team ) {
	int		i;

	for ( i = 0 ; i < game.maxclients ; i++ ) {
		if ( game.clients[i].pers.connected == CON_DISCONNECTED ) {
			continue;
		}
		if ( game.clients[i].sess.sessionTeam == team ) {
			if ( game.clients[i].sess.teamLeader )
				return i;
		}
	}

	return -1;
}


/*
================
PickTeam

================
*/
teamtype_t PickTeam( int ignoreClientNum ) {
	int		counts[TEAM_NUM_TEAMS];

	counts[ TEAM_ALLIES ] = TeamCount( ignoreClientNum, TEAM_ALLIES );
	counts[ TEAM_AXIS ] = TeamCount( ignoreClientNum, TEAM_AXIS );

	if( counts[ TEAM_ALLIES ] > counts[ TEAM_AXIS ] ) {
		return TEAM_AXIS;
	}
	if( counts[ TEAM_AXIS ] > counts[ TEAM_ALLIES ] ) {
		return TEAM_ALLIES;
	}
	// equal team count, so join the team with the lowest score
	if( level.teamScores[ TEAM_ALLIES ] > level.teamScores[ TEAM_AXIS ] ) {
		return TEAM_AXIS;
	}
	return TEAM_ALLIES;
}

/*
===========
ForceClientSkin

Forces a client's skin (for teamplay)
===========
*/
/*
static void ForceClientSkin( gclient_t *client, char *model, const char *skin ) {
	char *p;

	if ((p = Q_strrchr(model, '/')) != 0) {
		*p = 0;
	}

	Q_strcat(model, MAX_QPATH, "/");
	Q_strcat(model, MAX_QPATH, skin);
}
*/

/*
===========
ClientSpawn

Called every time a client is placed fresh in the world:
after the first ClientBegin, and after each respawn
Initializes all non-persistant parts of playerState
============
*/
void ClientSpawn(gentity_t *ent) {
	int		index;
	vec3_t	spawn_origin, spawn_angles;
	gclient_t	*client;
	int		i;
	clientPersistant_t	saved;
	clientSession_t		savedSess;
	int		persistant[MAX_PERSISTANT];
	Entity	*spawnPoint;
	int		flags;
	int		savedPing;
//	char	*savedAreaBits;
	int		accuracy_hits, accuracy_shots;
	int		eventSequence;
	char	userinfo[MAX_INFO_STRING];

	index = ent - g_entities;
	client = ent->client;

	// find a spawn point
	// do it before setting health back up, so farthest
	// ranging doesn't count this client
	if ( client->sess.sessionTeam == TEAM_SPECTATOR ) {
		spawnPoint = SelectSpectatorSpawnPoint ( 
						spawn_origin, spawn_angles);
	} else {
		do {
			// the first spawn should be at a good looking spot
			if ( !client->pers.initialSpawn && client->pers.localClient ) {
				client->pers.initialSpawn = qtrue;
				spawnPoint = SelectInitialSpawnPoint( spawn_origin, spawn_angles );
			} else {
				// don't spawn near existing origin if possible
				spawnPoint = SelectSpawnPoint ( 
					client->ps.origin, 
					spawn_origin, spawn_angles);
			}

			// Tim needs to prevent bots from spawning at the initial point
			// on q3dm0...
			if ( ( spawnPoint->flags & FL_NO_BOTS ) && ( ent->r.svFlags & SVF_BOT ) ) {
				continue;	// try again
			}
			// just to be symetric, we have a nohumans option...
			if ( ( spawnPoint->flags & FL_NO_HUMANS ) && !( ent->r.svFlags & SVF_BOT ) ) {
				continue;	// try again
			}

			break;

		} while ( 1 );
	}
	client->pers.teamState.state = TEAM_ACTIVE;

	// always clear the kamikaze flag
//	ent->s.eFlags &= ~EF_KAMIKAZE;

	// toggle the teleport bit so the client knows to not lerp
	// and never clear the voted flag
	flags = ent->s.eFlags & (EF_TELEPORT_BIT | EF_VOTED | EF_TEAMVOTED);
	flags ^= EF_TELEPORT_BIT;

	// clear everything but the persistant data

	saved = client->pers;
	savedSess = client->sess;
	savedPing = client->ps.ping;
//	savedAreaBits = client->areabits;
	accuracy_hits = client->accuracy_hits;
	accuracy_shots = client->accuracy_shots;

	Com_Memset (client, 0, sizeof(*client));

	client->ps.stats[STAT_ATTACKERCLIENT] = -1;
	client->ps.stats[STAT_INFOCLIENT] = -1;

	client->pers = saved;
	client->sess = savedSess;
	client->ps.ping = savedPing;
//	client->areabits = savedAreaBits;
	client->accuracy_hits = accuracy_hits;
	client->accuracy_shots = accuracy_shots;
	client->lastkilled_client = -1;

	client->airOutTime = level.time + 12000;

	gi.GetUserinfo( index, userinfo, sizeof(userinfo) );
	// set max health
	client->pers.maxHealth = atoi( Info_ValueForKey( userinfo, "handicap" ) );
	if ( client->pers.maxHealth < 1 || client->pers.maxHealth > 100 ) {
		client->pers.maxHealth = 100;
	}
	// clear entity values
	client->ps.stats[STAT_MAX_HEALTH] = client->pers.maxHealth;
	ent->s.eFlags = flags;

	ent->s.groundEntityNum = ENTITYNUM_NONE;
	ent->client = &game.clients[index];
	ent->takedamage = qtrue;
	ent->inuse = qtrue;
	ent->classname = "player";
	ent->r.contents = CONTENTS_BODY;
	ent->clipmask = MASK_PLAYERSOLID;
	ent->die = player_die;
	ent->waterlevel = 0;
	ent->watertype = 0;
	ent->flags = 0;
	
	VectorCopy (playerMins, ent->r.mins);
	VectorCopy (playerMaxs, ent->r.maxs);

	client->ps.clientNum = index;



	// health will count down towards max_health
	ent->health = client->ps.stats[STAT_HEALTH] = client->ps.stats[STAT_MAX_HEALTH] + 25;

	G_SetOrigin( ent, spawn_origin );
	VectorCopy( spawn_origin, client->ps.origin );

	// the respawned flag will be cleared after the attack and jump keys come up
	client->ps.pm_flags |= PMF_RESPAWNED;

	//gi.GetUsercmd( client - game.clients, &ent->client->pers.cmd );
	//FIXME

	SetClientViewAngle( ent, spawn_angles );

	if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {

	} else {
		G_KillBox( ent );
		gi.LinkEntity (ent);

	

	}

	// don't allow full run speed for a bit
	client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
	client->ps.pm_time = 100;

	client->respawnTime = level.time;
	client->inactivityTime = level.time + g_inactivity.integer * 1000;
	client->latched_buttons = 0;

	if ( level.intermissiontime ) {
		MoveClientToIntermission( ent );
	} else {
		// fire the targets of the spawn point
		//G_UseTargets( spawnPoint->edict, ent );

	}

	// run a client frame to drop exactly to the floor,
	// initialize animations and other things
	client->ps.commandTime = level.time - 100;
	ent->client->pers.cmd.serverTime = level.time;
	G_ClientThink( ent, &ent->client->pers.cmd );

	// positively link the client, even if the command times are weird
	if ( ent->client->sess.sessionTeam != TEAM_SPECTATOR ) {
		BG_PlayerStateToEntityState( &client->ps, &ent->s, qtrue );
		VectorCopy( ent->client->ps.origin, ent->r.currentOrigin );
		gi.LinkEntity( ent );
	}

	// run the presend to set anything else
	ClientEndFrame( ent );

	// clear entity state values
	BG_PlayerStateToEntityState( &client->ps, &ent->s, qtrue );

	ent->s.modelindex = G_ModelIndex("models/player/american_army.tik");
}

/*
==================
SetClientViewAngle

==================
*/
void SetClientViewAngle( gentity_t *ent, vec3_t angle ) {
	int			i;

	// set the delta angle
	for (i=0 ; i<3 ; i++) {
		int		cmdAngle;

		cmdAngle = ANGLE2SHORT(angle[i]);
		ent->client->ps.delta_angles[i] = cmdAngle - ent->client->pers.cmd.angles[i];
	}
	VectorCopy( angle, ent->s.angles );
	VectorCopy (ent->s.angles, ent->client->ps.viewangles);
}

#endif

/*
=======================================================================

  SelectSpawnPoint

=======================================================================
*/

/*
================
SelectNearestDeathmatchSpawnPoint

Find the spot that we DON'T want to use
================
*/
#define MAX_SPAWN_POINTS 128

Entity *SelectNearestDeathmatchSpawnPoint(vec3_t from)
{
    Entity *spot;
    vec3_t  delta;
    float   dist, nearestDist;
    Entity *nearestSpot;

    nearestDist = 999999;
    nearestSpot = NULL;
    spot        = NULL;

    while ((spot = (Entity *)G_FindClass(spot, "info_player_deathmatch")) != NULL) {
        VectorSubtract(spot->origin, from, delta);
        dist = VectorLength(delta);
        if (dist < nearestDist) {
            nearestDist = dist;
            nearestSpot = spot;
        }
    }

    return nearestSpot;
}

/*
================
SelectRandomDeathmatchSpawnPoint

go to a random point that doesn't telefrag
================
*/
Entity *SelectRandomDeathmatchSpawnPoint(str spawnpoint_type)
{
    Entity *spot  = NULL;
    Entity *spot1 = NULL;
    Entity *spot2 = NULL;
    int     count = 0;
    int     selection;
    float   range;
    float   range1 = 99999.0f;
    float   range2 = 99999.0f;

    while ((spot = (Entity *)G_FindClass(spot, spawnpoint_type))) {
        count++;

        range = PlayersRangeFromSpot(spot);
        if (range2 <= range) {
            if (range1 > range) {
                range1 = range;
                spot1  = spot;
            }
        } else {
            range2 = range;
            spot2  = spot;
        }
    }

    spot = NULL;

    if (!count) {
        return NULL;
    }

    if (count > 2) {
        count -= 2;
    } else {
        spot1 = NULL;
        spot2 = NULL;
    }

    for (selection = rand() % count; selection != -1; selection--) {
        spot = (Entity *)G_FindClass(spot, spawnpoint_type);
        if (!spot) {
            break;
        }

        if (spot == spot1 || spot == spot2) {
            selection++;
        }
    }

    return spot;
}

/*
===========
SelectRandomFurthestSpawnPoint

Chooses a player start, deathmatch start, etc
============
*/
Entity *SelectRandomFurthestSpawnPoint(void)
{
    Entity *spot               = NULL;
    Entity *bestspot           = NULL;
    float   bestplayerdistance = 0.0f;

    while ((spot = (Entity *)G_FindClass(spot, "info_player_deathmatch"))) {
        float dist = PlayersRangeFromSpot(spot);

        if (dist > bestplayerdistance) {
            bestplayerdistance = dist;
            bestspot           = spot;
        }
    }

    if (!bestspot) {
        bestspot = (Entity *)G_FindClass(NULL, "info_player_deathmatch");
    }

    return bestspot;
}

/*
===========
SelectSpawnPoint

Chooses a player start, deathmatch start, etc
============
*/
PlayerStart *SelectSpawnPoint(Player *player)
{
    PlayerStart *spot = NULL;

    switch (g_gametype->integer) {
    case GT_SINGLE_PLAYER:
        break;
    case GT_FFA:
        spot = player->GetDM_Team()->GetRandomFfaSpawnpoint(player);
        break;
    case GT_TEAM:
    case GT_TEAM_ROUNDS:
    case GT_LIBERATION:
        spot = player->GetDM_Team()->GetRandomTeamSpawnpoint(player);
        break;
    case GT_OBJECTIVE:
    case GT_TOW:
        spot = player->GetDM_Team()->GetRandomObjectiveSpawnpoint(player);
        break;
    default:
        Com_Printf("SelectSpawnPoint: unknown game type '%i'\n", g_gametype->integer);
        spot = player->GetDM_Team()->GetRandomFfaSpawnpoint(player);
        break;
    }

    // find a single player start spot
    if (!spot) {
        while ((spot = (PlayerStart *)G_FindArchivedClass(spot, "info_player_start")) != NULL) {
            if (level.spawnpoint.icmp(spot->TargetName()) == 0) {
                break;
            }
        }

        if (!spot && !level.spawnpoint.length()) {
            // there wasn't a spawnpoint without a target, so use any
            spot = (PlayerStart *)G_FindArchivedClass(NULL, "info_player_start");
        }

        if (!spot) {
            gi.Error(ERR_DROP, "No player spawn position named '%s'.  Can't spawn player.\n", level.spawnpoint.c_str());
        }
    }

    return spot;
}

//======================================================================

/*
===========
ClientCheckName
============
*/
static void ClientCleanName(const char *in, char *out, int outSize)
{
    int   len, colorlessLen;
    char  ch;
    char *p;
    int   spaces;

    //save room for trailing null byte
    outSize--;

    len          = 0;
    colorlessLen = 0;
    p            = out;
    *p           = 0;
    spaces       = 0;

    while (1) {
        ch = *in++;
        if (!ch) {
            break;
        }

        // don't allow leading spaces
        if (colorlessLen == 0 && ch == ' ') {
            continue;
        }

        // check colors
        if (ch == Q_COLOR_ESCAPE) {
            // solo trailing carat is not a color prefix
            if (!*in) {
                break;
            }

            // don't allow black in a name, period
            if (ColorIndex(*in) == 0) {
                in++;
                continue;
            }

            // make sure room in dest for both chars
            if (len > outSize - 2) {
                break;
            }

            *out++ = ch;
            *out++ = *in++;
            len += 2;
            continue;
        }

        // don't allow too many consecutive spaces
        // don't count spaces in colorlessLen
        if (ch == ' ') {
            spaces++;
            if (spaces > 3) {
                continue;
            }
            *out++ = ch;
            len++;
            continue;
        } else {
            spaces = 0;
        }

        if (len > outSize - 1) {
            break;
        }

        *out++ = ch;
        colorlessLen++;
        len++;
    }
    *out = 0;

    // don't allow empty names
    if (*p == 0 || colorlessLen == 0) {
        Q_strncpyz(p, "UnnamedPlayer", outSize);
    }
}

/*
===========
ClientUserInfoChanged

Called from ClientConnect when the player first connects and
directly by the server system when the player updates a userinfo variable.

The game can override any of the settings and call gi.SetUserinfo
if desired.
============
*/
void G_ClientUserinfoChanged(gentity_t *ent, const char *u)
{
    char      *s;
    gclient_t *client;
    int        clientnum;

    if (!ent) {
        return;
    }

    client = ent->client;

    s = Info_ValueForKey(u, "name");

    if (!s) {
        return;
    }

    clientnum = ent - g_entities;

    if (gi.SanitizeName(s, client->pers.netname)) {
        gi.Printf("WARNING: had to sanitize the name for client %i\n", clientnum);
    }

    s = Info_ValueForKey(u, "dm_playermodel");

    if (!s) {
        assert(0);
        return;
    }

    Q_strncpyz(client->pers.dm_playermodel, s, sizeof(client->pers.dm_playermodel));

    s = Info_ValueForKey(u, "dm_playergermanmodel");

    if (!s) {
        assert(0);
        return;
    }

    Q_strncpyz(client->pers.dm_playergermanmodel, s, sizeof(client->pers.dm_playergermanmodel));

    gi.SetConfigstring(CS_PLAYERS + clientnum, va("name\\%s", client->pers.netname));

    if (ent->entity) {
        float fov;

        s = Info_ValueForKey(u, "fov");

        fov = atof(s);

        if (fov < 80.0f) {
            fov = 80.0f;
        } else if (fov > 160.0f) {
            fov = 160.0f;
        }

        Event *ev = new Event(EV_Player_Fov);
        ev->AddFloat(fov);

        ent->entity->ProcessEvent(ev);
    }

    Q_strncpyz(client->pers.userinfo, u, sizeof(client->pers.userinfo));
}

void G_BotConnect(int clientNum)
{
    gclient_t *client;
    gentity_t *ent;
    char       userinfo[MAX_INFO_STRING];

    ent = &g_entities[clientNum];

    ent->client   = game.clients + clientNum;
    ent->s.number = clientNum;

    client = ent->client;

    Q_strncpyz(userinfo, client->pers.userinfo, sizeof(userinfo));

    // read the session data
    memset(client, 0, sizeof(*client));
    G_InitSessionData(client, userinfo);

    Q_strncpyz(client->pers.ip, "localhost", sizeof(client->pers.ip));
    client->pers.port = 0;

    G_ClientUserinfoChanged(ent, userinfo);
}

/*
===========
ClientConnect

Called when a player begins connecting to the server.
Called again for every map change or tournement restart.

The session information will be valid after exit.

Return NULL if the client should be allowed, otherwise return
a string with the reason for denial.

Otherwise, the client will be sent the current gamestate
and will eventually get to ClientBegin.

firstTime will be qtrue the very first time a client connects
to the server machine, but qfalse on map changes and tournement
restarts.
============
*/
const char *G_ClientConnect(int clientNum, qboolean firstTime)
{
    char      *ip, *port, *value;
    gclient_t *client;
    gentity_t *ent;
    char       userinfo[MAX_INFO_STRING];

    gi.DPrintf("TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT\n");
    if (!g_gametype->integer) {
        return NULL;
    }

    ent = &g_entities[clientNum];

    gi.GetUserinfo(clientNum, userinfo, sizeof(userinfo));

    // IP filtering
    // https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=500
    // recommanding PB based IP / GUID banning, the builtin system is pretty limited
    // check to see if they are on the banned IP list
    ip   = Info_ValueForKey(userinfo, "ip");
    port = Info_ValueForKey(userinfo, "port");

    // FIXME: what is fucking wrong with G_FilterPacket...
    //if ( G_FilterPacket( value ) ) {
    //	return "Banned IP";
    //}

    // we don't check password for bots and local client
    // NOTE: local client <-> "ip" "localhost"
    //   this means this client is not running in our current process
    if ((strcmp(ip, "localhost") != 0)) {
        // check for a password
        value = Info_ValueForKey(userinfo, "password");
        if (password->string[0] && Q_stricmp(password->string, "none") && strcmp(password->string, value) != 0) {
            return "Invalid password";
        }
    }

    ent->client = game.clients + clientNum;
    client      = ent->client;

    // read or initialize the session data
    if (firstTime) {
        memset(client, 0, sizeof(*client));
        G_InitSessionData(client, userinfo);
    } else {
        G_ReadSessionData(client);
    }

    Q_strncpyz(client->pers.ip, ip, sizeof(client->pers.ip));
    client->pers.port = atoi(port);

    G_ClientUserinfoChanged(ent, userinfo);

#if 0
	if( isBot )
	{
		ent->r.svFlags |= SVF_BOT;
		ent->inuse = qtrue;
		if( !G_BotConnect( clientNum, !firstTime ) ) {
			return "BotConnectfailed";
		}
	}
#endif

    // don't do the "xxx connected" messages if they were caried over from previous level
    if (firstTime && g_gametype->integer) {
        if (dedicated->integer) {
            gi.Printf("%s is preparing for deployment\n", client->pers.netname);
        }

        G_PrintToAllClients(va("%s is preparing for deployment\n", client->pers.netname));
    }
    return NULL;
}

/*
===========
ClientBegin

called when a client has finished connecting, and is ready
to be placed into the level.  This will happen every level load,
and on transition between teams, but doesn't happen on respawns
============
*/
void G_ClientBegin(gentity_t *ent, usercmd_t *cmd)
{
    try {
        assert(ent->s.number < game.maxclients);
        assert(ent->client != NULL);

        if (ent->inuse && ent->entity) {
            VectorClear(ent->client->cmd_angles);
            // the client has cleared the client side viewangles upon
            // connecting to the server, which is different than the
            // state when the game is saved, so we need to compensate
            // with deltaangles
            ent->entity->SetDeltaAngles();
        } else {
            // a spawn point will completely reinitialize the entity
            level.spawn_entnum = ent->s.number;

            if (level.m_bSpawnBot) {
                level.m_bSpawnBot = false;
                PlayerBot *player = new PlayerBot;
            } else {
                Player *player = new Player;
            }
        }

        if (level.intermissiontime && ent->entity) {
            G_MoveClientToIntermission(ent->entity);
        } else {
            ent->client->pers.enterTime = level.svsFloatTime;

            if (g_gametype->integer) {
                // send effect if in a multiplayer game
                if (dedicated->integer) {
                    gi.Printf("%s has entered the battle\n", ent->client->pers.netname);
                }

                G_PrintToAllClients(va("%s has entered the battle\n", ent->client->pers.netname), 2);
            }
        }

        // make sure all view stuff is valid
        if (ent->entity) {
            ent->entity->EndFrame();
        }
    } catch (const char *error) {
        G_ExitWithError(error);
    }
}

/*
===========
G_SetClientConfigString
============
*/
void G_SetClientConfigString(gentity_t *ent)
{
    int         num;
    const char *string;

    if (!ent) {
        return;
    }

    num = ent - g_entities;

    if (ent->entity) {
        Player *pEnt = static_cast<Player *>(ent->entity);
        string       = va("name\\%s\\team\\%i", ent->client->pers.netname, pEnt->GetTeam());
    } else {
        // No team
        string = va("name\\%s\\team\\0", ent->client->pers.netname);
    }

    gi.SetConfigstring(CS_PLAYERS + num, string);
}

/*
===========
ClientDisconnect

Called when a player drops from the server.
Will not be called between levels.

This should NOT be called directly by any game logic,
call gi.DropClient(), which will call this and do
server system housekeeping.
============
*/
void G_ClientDisconnect(gentity_t *ent)
{
    try {
        if (!ent || (!ent->client) || (!ent->entity)) {
            return;
        }

        G_PrintToAllClients(va("%s has left the battle\n", ent->client->pers.netname));

        ((Player *)ent->entity)->Disconnect();

        delete ent->entity;
        ent->entity = NULL;
    }

    catch (const char *error) {
        G_ExitWithError(error);
    }
#if 0
	int			i;

	// cleanup if we are kicking a bot that
	// hasn't spawned yet
	G_RemoveQueuedBotBegin( ent->client->ps.clientNum );

	if ( !ent->client ) {
		return;
	}

	// stop any following clients
	for ( i = 0 ; i < game.maxclients ; i++ ) {
		if ( game.clients[i].sess.sessionTeam == TEAM_SPECTATOR
			&& game.clients[i].sess.spectatorState == SPECTATOR_FOLLOW
			&& game.clients[i].sess.spectatorClient == ent->client->ps.clientNum ) {
			StopFollowing( &g_entities[i] );
		}
	}

	// send effect if they were completely connected
	if ( ent->client->pers.connected == CON_CONNECTED 
		&& ent->client->sess.sessionTeam != TEAM_SPECTATOR ) {

		// They don't get to take powerups with them!
		// Especially important for stuff like CTF flags
		TossClientItems( ent );
#    ifdef MISSIONPACK
		TossClientPersistantPowerups( ent );
		if( g_gametype->integer == GT_HARVESTER ) {
			TossClientCubes( ent );
		}
#    endif

	}

	G_LogPrintf( "ClientDisconnect: %i\n", ent->client->ps.clientNum );


	gi.UnlinkEntity (ent);
	ent->s.modelindex = 0;
	ent->inuse = qfalse;
	ent->classname = "disconnected";
	ent->client->pers.connected = CON_DISCONNECTED;
	ent->client->ps.stats[ STAT_TEAM ] = TEAM_NONE;
	ent->client->sess.sessionTeam = TEAM_NONE;

	gi.SetConfigstring( CS_PLAYERS + ent->client->ps.clientNum, "" );

	CalculateRanks();

	if ( ent->r.svFlags & SVF_BOT ) {
		BotAIShutdownClient( ent->client->ps.clientNum, qfalse );
	}
#endif
}
