/*
===========================================================================
Copyright (C) 2025 the OpenMoHAA team

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

static saved_bot_t *saved_bots     = NULL;
static unsigned int num_saved_bots = 0;
static unsigned int botId          = 0;
static float  botInitTime          = 0;

Container<str> alliedModelList;
Container<str> germanModelList;

saved_bot_t::saved_bot_t()
    : userinfo {0}
{}

static void G_ReadBotSessionData();
static unsigned int G_GetNumBotsToSpawn();

/*
===========
IsAlliedPlayerModel

Return whether or not the specified filename is for allies
============
*/
bool IsAlliedPlayerModel(const char *filename)
{
    return !Q_stricmpn(filename, "allied_", 7) || !Q_stricmpn(filename, "american_", 9);
}

/*
===========
IsGermanPlayerModel

Return whether or not the specified filename is for axis
============
*/
bool IsGermanPlayerModel(const char *filename)
{
    return !Q_stricmpn(filename, "german_", 7) || !Q_stricmpn(filename, "IT_", 3) || !Q_stricmpn(filename, "SC_", 3);
}

/*
===========
IsPlayerModel

Return whether or not the specified filename
is a player model that can be chosen
============
*/
bool IsPlayerModel(const char *filename)
{
    size_t len = strlen(filename);

    if (len >= 8 && !Q_stricmp(&filename[len - 8], "_fps.tik")) {
        return false;
    }

    if (!IsAlliedPlayerModel(filename) && !IsGermanPlayerModel(filename)) {
        return false;
    }

    return true;
}

/*
===========
ClearModelList

Clear the allied and axis model list
============
*/
void ClearModelList()
{
    alliedModelList.FreeObjectList();
    germanModelList.FreeObjectList();
}

/*
===========
InitModelList

Initialize the list of allied and axis player models
that bots can use
============
*/
void InitModelList()
{
    char **fileList;
    int    numFiles;
    int    i;
    size_t numAlliedModels = 0, numGermanModels = 0;
    byte  *p;

    ClearModelList();

    fileList = gi.FS_ListFiles("models/player", ".tik", qfalse, &numFiles);

    for (i = 0; i < numFiles; i++) {
        const char *filename = fileList[i];

        if (!IsPlayerModel(filename)) {
            continue;
        }

        if (IsAlliedPlayerModel(filename)) {
            numAlliedModels++;
        } else {
            numGermanModels++;
        }
    }

    alliedModelList.Resize(numAlliedModels);
    germanModelList.Resize(numGermanModels);

    for (i = 0; i < numFiles; i++) {
        const char *filename = fileList[i];
        size_t      len      = strlen(filename);

        if (!IsPlayerModel(filename)) {
            continue;
        }

        if (IsAlliedPlayerModel(filename)) {
            alliedModelList.AddObject(str(filename, 0, len - 4));
        } else {
            germanModelList.AddObject(str(filename, 0, len - 4));
        }
    }

    gi.FS_FreeFileList(fileList);
}

/*
===========
G_BotBegin

Begin spawning a new bot entity
============
*/
void G_BotBegin(gentity_t *ent)
{
    Player        *player;
    BotController *controller;

    level.spawn_entnum = ent->s.number;
    player             = new Player;

    G_ClientBegin(ent, NULL);

    controller = botManager.getControllerManager().createController(player);
    //player->setController(controller);
}

/*
===========
G_BotThink

Called each server frame to make bots think
============
*/
void G_BotThink(gentity_t *ent, int msec)
{
    /*
    usercmd_t  ucmd;
    usereyes_t eyeinfo;
    PlayerBot *bot;

    assert(ent);
    assert(ent->entity);
    assert(ent->entity->IsSubclassOfBot());

    bot = (PlayerBot *)ent->entity;

    bot->UpdateBotStates();
    bot->GetUsercmd(&ucmd);
    bot->GetEyeInfo(&eyeinfo);

    G_ClientThink(ent, &ucmd, &eyeinfo);
    */
}

/*
===========
G_FindFreeEntityForBot

Find a free client slot
============
*/
gentity_t *G_FindFreeEntityForBot()
{
    gentity_t *ent;
    int        minNum = 0;
    int        i;

    if (sv_sharedbots->integer) {
        minNum = 0;
    } else {
        minNum = maxclients->integer;
    }

    for (i = minNum; i < game.maxclients; i++) {
        ent = &g_entities[i];
        if (!ent->inuse && ent->client && !ent->client->pers.userinfo[0]) {
            return ent;
        }
    }

    return NULL;
}

/*
===========
G_ChangeParent

Fix parenting for entities that use the old number
============
*/
void G_ChangeParent(int oldNum, int newNum)
{
    gentity_t *ent;
    int        i;

    for (i = 0; i < game.maxentities; i++) {
        ent = &g_entities[i];
        if (!ent->inuse || !ent->entity) {
            continue;
        }

        if (ent->s.parent == oldNum) {
            ent->s.parent = newNum;
        }
        if (ent->r.ownerNum == oldNum) {
            ent->r.ownerNum = newNum;
        }
    }
}

/*
===========
G_BotShift

If the specified slot is used, the bot will be relocated
to the next free entity slot
============
*/
void G_BotShift(int clientNum)
{
    gentity_t *ent;
    gentity_t *newEnt;

    ent = &g_entities[clientNum];
    if (!ent->inuse || !ent->client || !ent->entity) {
        return;
    }

    if (!botManager.getControllerManager().findController(ent->entity)) {
        return;
    }

    newEnt = G_FindFreeEntityForBot();
    if (!newEnt) {
        G_RemoveBot(ent);
        return;
    }

    //
    // Allocate the new entity
    //
    level.spawn_entnum = newEnt - g_entities;
    level.AllocEdict(ent->entity);

    //
    // Copy all fields
    //
    newEnt->s        = ent->s;
    newEnt->s.number = newEnt - g_entities;
    memcpy(newEnt->client, ent->client, sizeof(*newEnt->client));
    newEnt->r     = ent->r;
    newEnt->solid = ent->solid;
    newEnt->tiki  = ent->tiki;
    AxisCopy(ent->mat, newEnt->mat);

    newEnt->freetime  = ent->freetime;
    newEnt->spawntime = ent->spawntime;
    newEnt->radius2   = ent->radius2;
    memcpy(newEnt->entname, ent->entname, sizeof(newEnt->entname));
    newEnt->clipmask             = ent->clipmask;
    newEnt->entity               = ent->entity;
    newEnt->entity->edict        = newEnt;
    newEnt->entity->client       = newEnt->client;
    newEnt->entity->entnum       = newEnt->s.number;
    newEnt->client->ps.clientNum = newEnt->s.number;

    G_ChangeParent(ent->s.number, newEnt->s.number);

    //
    // Free the old entity so the real client will use it
    //
    level.FreeEdict(ent);
    memset(ent->client, 0, sizeof(*ent->client));

    G_SetClientConfigString(newEnt);
}

/*
===========
G_GetFirstBot

Return the first bot
============
*/
gentity_t *G_GetFirstBot()
{
    gentity_t   *ent;
    unsigned int n;

    for (n = 0; n < game.maxclients; n++) {
        ent = &g_entities[n];
        if (G_IsBot(ent)) {
            return ent;
        }
    }

    return NULL;
}

/*
===========
G_IsBot

Return whether or not the gentity is a bot
============
*/
bool G_IsBot(gentity_t *ent)
{
    if (!ent->inuse || !ent->client) {
        return false;
    }

    if (!ent->entity || !botManager.getControllerManager().findController(ent->entity)) {
        return false;
    }

    return true;
}

/*
===========
G_IsPlayer

Return whether or not the gentity is a player
============
*/
bool G_IsPlayer(gentity_t *ent)
{
    if (!ent->inuse || !ent->client) {
        return false;
    }

    if (!ent->entity || botManager.getControllerManager().findController(ent->entity)) {
        return false;
    }

    return true;
}

/*
===========
G_GetRandomAlliedPlayerModel
============
*/
const char *G_GetRandomAlliedPlayerModel()
{
    if (!alliedModelList.NumObjects()) {
        return "";
    }

    const unsigned int index = rand() % alliedModelList.NumObjects();
    return alliedModelList[index];
}

/*
===========
G_GetRandomGermanPlayerModel
============
*/
const char *G_GetRandomGermanPlayerModel()
{
    if (!germanModelList.NumObjects()) {
        return "";
    }

    const unsigned int index = rand() % germanModelList.NumObjects();
    return germanModelList[index];
}

/*
===========
G_GetBotId
============
*/
unsigned int G_GetBotId(gentity_t *e) {
    const unsigned int clientNum = e - g_entities;

    return sv_sharedbots->integer ? clientNum : clientNum - maxclients->integer;
}

/*
===========
G_AddBot

Add the specified bot, optionally its saved state
============
*/
gentity_t *G_AddBot(const bot_info_t *info)
{
    int        clientNum;
    gentity_t *e;
    char       botName[MAX_NETNAME];
    char       userinfo[MAX_INFO_STRING] {0};

    e = G_FindFreeEntityForBot();
    if (!e) {
        gi.Printf("No free slot for a bot\n");
        return NULL;
    }

    clientNum = e - g_entities;

    // increase the unique ID
    botId++;

    if (info && info->name) {
        Q_strncpyz(botName, info->name, sizeof(botName));
    } else {
        const unsigned int num = sv_sharedbots->integer ? clientNum : clientNum - maxclients->integer;

        cvar_t *v = gi.Cvar_Find(va("g_bot%d_name", num));
        if (v && *v->string) {
            Q_strncpyz(botName, v->string, sizeof(botName));
        } else {
            Com_sprintf(botName, sizeof(botName), "bot%d", botId);
        }
    }

    Info_SetValueForKey(userinfo, "name", botName);

    //
    // Choose a random model
    //
    Info_SetValueForKey(userinfo, "dm_playermodel", G_GetRandomAlliedPlayerModel());
    Info_SetValueForKey(userinfo, "dm_playergermanmodel", G_GetRandomGermanPlayerModel());

    Info_SetValueForKey(userinfo, "fov", "80");
    Info_SetValueForKey(userinfo, "ip", "localhost");

    // Connect the bot for the first time
    // setup user info and stuff
    G_BotConnect(clientNum, qtrue, userinfo);
    G_BotBegin(e);

    return e;
}

/*
===========
G_RestoreBot

Restore the specified bot
============
*/
gentity_t *G_RestoreBot(const saved_bot_t& saved)
{
    gentity_t *e;
    char       userinfo[MAX_INFO_STRING] {0};

    e = G_FindFreeEntityForBot();
    if (!e) {
        gi.Printf("No free slot for a bot\n");
        return NULL;
    }

    G_BotConnect(e - g_entities, qfalse, saved.userinfo);
    G_BotBegin(e);

    return e;
}

/*
===========
G_AddBots

Add the specified number of bots
============
*/
void G_AddBots(unsigned int num)
{
    int n;

    for (n = 0; n < num; n++) {
        G_AddBot();
    }
}

/*
===========
G_RemoveBot

Remove the specified bot
============
*/
void G_RemoveBot(gentity_t *ent)
{
    if (ent->entity) {
        BotController *controller = botManager.getControllerManager().findController(ent->entity);

        botManager.getControllerManager().removeController(controller);
    }

    G_ClientDisconnect(ent);
}

/*
===========
G_RemoveBots

Remove the specified number of bots
============
*/
void G_RemoveBots(unsigned int num)
{
    unsigned int removed = 0;
    unsigned int n;
    unsigned int teamCount[2] {0};
    bool         bNoMoreToRemove = false;

    teamCount[0] = dmManager.GetTeamAllies()->m_players.NumObjects();
    teamCount[1] = dmManager.GetTeamAxis()->m_players.NumObjects();

    while (!bNoMoreToRemove) {
        bNoMoreToRemove = true;

        // First remove bots that are in the team
        // with the higest player count
        for (n = 0; n < game.maxclients && removed < num; n++) {
            gentity_t *e = &g_entities[n];
            if (!G_IsBot(e)) {
                continue;
            }

            Player *player = static_cast<Player *>(e->entity);
            if (player->GetTeam() == TEAM_ALLIES || player->GetTeam() == TEAM_AXIS) {
                unsigned int teamIndex = (player->GetTeam() - TEAM_ALLIES);
                if (teamCount[teamIndex] < teamCount[1 - teamIndex]) {
                    // Not enough players in that team, don't remove the bot
                    continue;
                }

                teamCount[teamIndex]--;
                bNoMoreToRemove = false;
            }

            G_RemoveBot(e);
            removed++;
        }
    }

    //
    // Remove all bots that haven't been removed earlier
    //
    for (n = 0; n < game.maxclients && removed < num; n++) {
        gentity_t *e = &g_entities[n];
        if (!G_IsBot(e)) {
            continue;
        }

        G_RemoveBot(e);
        removed++;
    }
}

/*
===========
G_GetNumBots
============
*/
unsigned int G_GetNumBots()
{
    return botManager.getControllerManager().getControllers().NumObjects();
}

/*
===========
G_GetBotSkill
============
*/
const char *G_GetBotSkill()
{
    return "Average";
}

/*
===========
G_SaveBots

Save bot persistent data
============
*/
void G_SaveBots()
{
    unsigned int count;
    unsigned int n;

    if (saved_bots) {
        delete[] saved_bots;
        saved_bots = NULL;
    }

    const BotControllerManager& manager        = botManager.getControllerManager();
    unsigned int                numSpawnedBots = manager.getControllers().NumObjects();

    if (!numSpawnedBots) {
        return;
    }

    saved_bots     = new saved_bot_t[numSpawnedBots];
    num_saved_bots = 0;

    count = manager.getControllers().NumObjects();
    assert(count <= numSpawnedBots);

    for (n = 1; n <= count; n++) {
        const BotController *controller = manager.getControllers().ObjectAt(n);
        Player              *player     = controller->getControlledEntity();
        if (!player) {
            // this shouldn't happen
            continue;
        }

        saved_bot_t& saved = saved_bots[num_saved_bots++];
        memcpy(saved.userinfo, player->client->pers.userinfo, sizeof(saved.userinfo));
    }
}

/*
===========
G_RestoreBots

Restore bot persistent data, such as their team
============
*/
void G_RestoreBots()
{
    unsigned int n;

    if (saved_bots) {
        if (g_gametype->integer != GT_SINGLE_PLAYER) {
            const unsigned int numToSpawn = G_GetNumBotsToSpawn();

            if (num_saved_bots > numToSpawn) {
                num_saved_bots = numToSpawn;
            }

            for (n = 0; n < num_saved_bots; n++) {
                const saved_bot_t& saved = saved_bots[n];

                G_RestoreBot(saved);
            }
        }

        delete[] saved_bots;
        saved_bots = NULL;
    }
}

/*
===========
G_CountPlayingClients

Count the number of real clients that are playing
============
*/
int G_CountPlayingClients()
{
    gentity_t   *other;
    unsigned int n;
    unsigned int count = 0;

    for (n = 0; n < game.maxclients; n++) {
        other = &g_entities[n];
        if (G_IsPlayer(other)) {
            Player *p = static_cast<Player *>(other->entity);
            // Ignore spectators
            if (p->GetTeam() != teamtype_t::TEAM_NONE && p->GetTeam() != teamtype_t::TEAM_SPECTATOR) {
                count++;
            }
        }
    }

    return count;
}

/*
===========
G_CountClients

Count the number of real clients
============
*/
int G_CountClients()
{
    gentity_t   *other;
    unsigned int n;
    unsigned int count = 0;

    for (n = 0; n < game.maxclients; n++) {
        other = &g_entities[n];
        if (G_IsBot(other)) {
            continue;
        }

        if (other->client && other->client->pers.userinfo[0]) {
            count++;
        }
    }

    return count;
}

static unsigned int G_GetNumBotsToSpawn()
{
    unsigned int numClients;
    unsigned int numBotsToSpawn;

    //
    // Check the minimum bot count
    //
    numClients = G_CountPlayingClients();
    if (numClients < sv_minPlayers->integer) {
        numBotsToSpawn = sv_minPlayers->integer - numClients + sv_numbots->integer;
    } else {
        numBotsToSpawn = sv_numbots->integer;
    }

    if (sv_sharedbots->integer) {
        numClients = G_CountClients();

        //
        // Cap to the maximum number of possible clients
        //
        numBotsToSpawn = Q_min(numBotsToSpawn, maxclients->integer - numClients + sv_maxbots->integer);
    } else {
        numBotsToSpawn = Q_min(numBotsToSpawn, sv_maxbots->integer);
    }

    return numBotsToSpawn;
}

/*
===========
G_RestartBots

Save bots
============
*/
void G_RestartBots()
{
    G_SaveBots();
}

static void G_InitBotSessionData()
{
    unsigned int n;

    gi.Cvar_Get("botsession", "", CVAR_ROM);

    for (n = 0; n < sv_maxbots->integer; n++) {
        gi.Cvar_Get(va("botsession%i", n), "", CVAR_ROM);
    }
}

/*
===========
G_ReadBotSessionData
============
*/
static void G_ReadBotSessionData()
{
    unsigned int n;
    unsigned int numSessions;

    if (saved_bots) {
        return;
    }

    cvar_t *v = gi.Cvar_Find("botsession");
    if (!v || !v->integer) {
        return;
    }

    // Unused for now, may be reserved for future use
#if 0
    numSessions = Q_min(v->integer, G_GetNumBotsToSpawn());

    // Spawn bots if there are any prepared
    for (n = 0; n < numSessions; n++) {
        v = gi.Cvar_Find(va("botsession%i", n));
        if (v) {
            gi.cvar_set(va("botsession%i", n), "");
        }
    }
#endif

    gi.cvar_set("botsession", "");
}

/*
===========
G_WriteBotSessionData
============
*/
void G_WriteBotSessionData()
{
    const BotControllerManager& manager        = botManager.getControllerManager();
    unsigned int                numSpawnedBots = manager.getControllers().NumObjects();
    unsigned int                count;
    unsigned int                n;
    unsigned int                current;

    if (!numSpawnedBots) {
        return;
    }

    if (saved_bots) {
        return;
    }

    // Unused for now, may be reserved for future use
    current = 0;
#if 0
    count = manager.getControllers().NumObjects();
    assert(count <= numSpawnedBots);

    for (n = 0; n < count; n++) {
        const BotController *controller = manager.getControllers().ObjectAt(n + 1);
        Player              *player     = controller->getControlledEntity();
        if (!player) {
            // this shouldn't happen
            continue;
        }

        gi.cvar_set(va("botsession%i", current), "");
        current++;
    }
#endif

    gi.cvar_set("botsession", va("%d", current));
}

/*
===========
G_ResetBots

Save and reset the bot count
============
*/
void G_ResetBots()
{
    G_WriteBotSessionData();

    botManager.Cleanup();

    botId = 0;
}

/*
===========
G_BotInit

Called to initialize bots
============
*/
void G_BotInit()
{
    InitModelList();
    botManager.Init();

    G_InitBotSessionData();
}

/*
===========
G_BotFrame

Called each frame to manage bots
============
*/
void G_BotFrame()
{
    botManager.Frame();
}

/*
===========
G_BotPostInit

Called after the server has spawned
============
*/
void G_BotPostInit()
{
    G_ReadBotSessionData();

    G_RestoreBots();

    if (g_bot_initial_spawn_delay->value <= 0) {
        G_SpawnBots();
    }

    botInitTime = level.time;
}

/*
===========
G_SpawnBots

Called each frame to manage bot spawning
============
*/
void G_SpawnBots()
{
    unsigned int numClients;
    unsigned int numBotsToSpawn;
    unsigned int numSpawnedBots;

    if (g_gametype->integer == GT_SINGLE_PLAYER) {
        // No bot on single-player
        return;
    }

    if (level.time - botInitTime < g_bot_initial_spawn_delay->value && !sv_numbots->modified) {
        // Wait before spawning all bots
        return;
    }

    numBotsToSpawn = G_GetNumBotsToSpawn();
    numSpawnedBots = botManager.getControllerManager().getControllers().NumObjects();

    //
    // Spawn bots
    //
    if (numBotsToSpawn > numSpawnedBots) {
        G_AddBots(numBotsToSpawn - numSpawnedBots);
    } else if (numBotsToSpawn < numSpawnedBots) {
        G_RemoveBots(numSpawnedBots - numBotsToSpawn);
    } else {
        sv_numbots->modified = false;
    }
}
