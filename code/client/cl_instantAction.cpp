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

#include "cl_ui.h"
#include "cl_instantAction.h"
#include "cl_uiserverlist.h"
#include "../gamespy/sv_gamespy.h"

Event EV_UIInstantAction_AcceptServer
(
    "acceptserver",
    EV_DEFAULT,
    NULL,
    NULL,
    "Connect to the current server"
);

Event EV_UIInstantAction_RejectServer
(
    "rejectserver",
    EV_DEFAULT,
    NULL,
    NULL,
    "Reject the current server"
);

Event EV_UIInstantAction_Cancel
(
    "ia_cancel",
    EV_DEFAULT,
    NULL,
    NULL,
    "cancel the server update"
);

Event EV_UIInstantAction_Refresh
(
    "ia_refresh",
    EV_DEFAULT,
    NULL,
    NULL,
    "Refresh the server list"
);

CLASS_DECLARATION(UIWidget, UIInstantAction, NULL) {
    {&EV_UIInstantAction_AcceptServer, &UIInstantAction::Connect      },
    {&EV_UIInstantAction_RejectServer, &UIInstantAction::Reject       },
    {&EV_UIInstantAction_Cancel,       &UIInstantAction::CancelRefresh},
    {&EV_UIInstantAction_Refresh,      &UIInstantAction::Refresh      },
    {NULL,                             NULL                           }
};

struct ServerListInstance {
    int              iServerType;
    UIInstantAction *pServerList;
};

ServerListInstance g_IAServerListInst[2];

UIInstantAction::UIInstantAction()
{
    state           = IA_INITIALIZE;
    numFoundServers = 0;
    numServers      = 0;
    minPlayers      = 3;
    startingMaxPing = 100;
    endingMaxPing   = 1500;
    maxServers      = -1;
    servers         = NULL;
    doneList[0]     = false;
    doneList[1]     = false;
    serverList[0]   = NULL;
    serverList[1]   = NULL;

    ReadIniFile();
    EnableServerInfo(false);

    menuManager.PassEventToWidget("ia_cancel_button", new Event(EV_Widget_Disable));
    menuManager.PassEventToWidget("ia_refresh_button", new Event(EV_Widget_Disable));
}

UIInstantAction::~UIInstantAction()
{
    CleanUp();
}

void UIInstantAction::CleanUp()
{
    if (serverList[0]) {
        ServerListFree(serverList[0]);
        serverList[0] = NULL;
    }

    if (serverList[1]) {
        ServerListFree(serverList[1]);
        serverList[1] = NULL;
    }

    if (servers) {
        delete[] servers;
        servers = NULL;
    }
}

void UIInstantAction::Init()
{
    const char *secret_key;
    const char *game_name;

    static const unsigned int iNumConcurrent = 10;

    numFoundServers = 0;
    numServers      = 0;
    doneList[0]     = false;
    doneList[1]     = false;

    EnableServerInfo(false);

    menuManager.PassEventToWidget("ia_cancel_button", new Event(EV_Widget_Disable));
    menuManager.PassEventToWidget("ia_refresh_button", new Event(EV_Widget_Disable));
    menuManager.PassEventToWidget("ia_noserverfound", new Event(EV_Widget_Disable));

    Cvar_Set("ia_search_percentage", va("%d %%", 0));

    if (com_target_game->integer < target_game_e::TG_MOHTT) {
        g_IAServerListInst[0].iServerType = com_target_game->integer;
        g_IAServerListInst[0].pServerList = this;

        game_name  = GS_GetGameName(com_target_game->integer);
        secret_key = GS_GetGameKey(com_target_game->integer);

        serverList[0] = ServerListNew(
            game_name,
            game_name,
            secret_key,
            iNumConcurrent,
            (void *)&IAServerListCallBack,
            1,
            (void *)&g_IAServerListInst[0]
        );

        ServerListClear(serverList[0]);
    } else {
        g_IAServerListInst[0].iServerType = target_game_e::TG_MOHTT;
        g_IAServerListInst[0].pServerList = this;

        game_name  = GS_GetGameName(target_game_e::TG_MOHTT);
        secret_key = GS_GetGameKey(target_game_e::TG_MOHTT);

        serverList[0] = ServerListNew(
            game_name,
            game_name,
            secret_key,
            iNumConcurrent,
            (void *)&IAServerListCallBack,
            1,
            (void *)&g_IAServerListInst[0]
        );

        ServerListClear(serverList[0]);

        g_IAServerListInst[1].iServerType = target_game_e::TG_MOHTA;
        g_IAServerListInst[1].pServerList = this;

        game_name  = GS_GetGameName(target_game_e::TG_MOHTA);
        secret_key = GS_GetGameKey(target_game_e::TG_MOHTA);

        serverList[1] = ServerListNew(
            game_name,
            game_name,
            secret_key,
            iNumConcurrent,
            (void *)&IAServerListCallBack,
            1,
            (void *)&g_IAServerListInst[1]
        );

        ServerListClear(serverList[1]);
    }

    state           = IA_WAITING;
    numFoundServers = 0;

    ServerListUpdate(serverList[0], true);

    if (serverList[1]) {
        ServerListUpdate(serverList[1], true);
    }

    menuManager.PassEventToWidget("ia_cancel_button", new Event(EV_Widget_Enable));
    menuManager.PassEventToWidget("searchstatus", new Event(EV_Widget_Enable));
    menuManager.PassEventToWidget("searchstatuslable", new Event(EV_Widget_Enable));
}

int UIInstantAction::GetServerIndex(int maxPing, int gameType)
{
    int bestPing   = 1500;
    int bestServer = -1;
    int i;

    for (i = 0; i < numFoundServers; i++) {
        const IAServer_t& IAServer = servers[i];

        char *gameVer;
        float fGameVer;
        int   ping;
        int   numPlayers;

        if (IAServer.rejected) {
            continue;
        }

        // Skip servers that don't match the provided game type
        if (ServerGetIntValue(IAServer.server, "g_gametype_i", 1) != gameType) {
            continue;
        }

        // Skip servers with high ping
        ping = ServerGetPing(IAServer.server);
        if (ping > maxPing) {
            continue;
        }

        gameVer = ServerGetStringValue(IAServer.server, "gamever", "1.00");
        if (com_target_demo->integer && *gameVer != 'd') {
            // Skip retail servers on demo game
            continue;
        } else if (!com_target_demo->integer && *gameVer == 'd') {
            // Skip demo servers on retail game
            continue;
        }

        // Skip incompatible servers
        fGameVer = atof(gameVer);
        if (com_target_game->integer >= target_game_e::TG_MOHTT) {
            if (IAServer.serverGame.serverType == target_game_e::TG_MOHTT) {
                if (fabs(fGameVer) < 2.3f) {
                    continue;
                }
            } else {
                if (fabs(fGameVer) < 2.1f) {
                    continue;
                }
            }
        } else {
            if (fabs(fGameVer - com_target_shortversion->value) > 0.1f) {
                continue;
            }
        }

        // Skip servers with a password
        if (ServerGetIntValue(IAServer.server, "password", 0)) {
            continue;
        }

        // Skip servers that don't match the minimum number of players
        numPlayers = ServerGetIntValue(IAServer.server, "numplayers", 0);
        if (numPlayers < minPlayers) {
            continue;
        }

        // Skip full servers
        if (numPlayers == ServerGetIntValue(IAServer.server, "maxplayers", 0)) {
            continue;
        }

        // Skip servers with an higher ping than the best one
        if (ping >= bestPing) {
            continue;
        }

        //
        // Found a potential server
        //

        bestPing   = ping;
        bestServer = i;
    }

    return bestServer;
}

void UIInstantAction::ReadIniFile()
{
    char       *buffer;
    const char *p;
    const char *pVal;
    int         intValue;
    char        value[32];

    if (!FS_ReadFileEx("iaction.ini", (void **)&buffer, qtrue)) {
        return;
    }

    for (p = buffer; p; p = strstr(pVal, "\n")) {
        if (sscanf(p, "%31s", value) != 1) {
            break;
        }

        pVal = strstr(p, "=");
        if (!pVal) {
            break;
        }

        pVal++;

        if (sscanf(pVal, "%d", &intValue) != 1) {
            break;
        }

        if (!Q_stricmpn(value, "MinPlayers", 10)) {
            minPlayers = intValue;
        }

        if (!Q_stricmpn(value, "StartingMaxPing", 15)) {
            startingMaxPing = intValue;
        }

        if (!Q_stricmpn(value, "EndingMaxPing", 13)) {
            endingMaxPing = intValue;
        }

        if (!Q_stricmpn(value, "MaxServers", 10)) {
            maxServers = intValue;
        }
    }
}

void UIInstantAction::FindServer()
{
    int ping;
    int i;

    currentServer = -1;
    state         = IA_NONE;

    for (ping = startingMaxPing; ping < endingMaxPing; ping += 100) {
        //
        // Find the best server starting from FFA gametype first
        //
        for (i = 1; i < 7; i++) {
            currentServer = GetServerIndex(ping, i);
            if (currentServer >= 0) {
                break;
            }
        }

        if (currentServer >= 0) {
            break;
        }
    }

    menuManager.PassEventToWidget("ia_refresh_button", new Event(EV_Widget_Enable));
    menuManager.PassEventToWidget("ia_cancel_button", new Event(EV_Widget_Disable));
    menuManager.PassEventToWidget("searchstatus", new Event(EV_Widget_Disable));
    menuManager.PassEventToWidget("searchstatuslable", new Event(EV_Widget_Disable));

    if (currentServer < 0) {
        EnableServerInfo(false);

        menuManager.PassEventToWidget("ia_noserverfound", new Event(EV_Widget_Enable));
        return;
    }

    const IAServer_t& IAServer   = servers[currentServer];
    const char       *hostname   = ServerGetStringValue(IAServer.server, "hostname", "(NONE)");
    const char       *gametype   = ServerGetStringValue(IAServer.server, "gametype", "(NONE)");
    int               numplayers = ServerGetIntValue(IAServer.server, "numplayers", 0);
    int               maxplayers = ServerGetIntValue(IAServer.server, "maxplayers", 0);
    ping                         = ServerGetPing(IAServer.server);

    Cvar_Set("ia_servername", va("  %s", hostname));
    Cvar_Set("ia_ping", va("%d", ping));
    Cvar_Set("ia_gametype", va("%s", gametype));
    Cvar_Set("ia_players", va("%d", numplayers));
    Cvar_Set("ia_maxplayers", va("%d", maxplayers));

    EnableServerInfo(true);
}

void UIInstantAction::Connect(Event *ev)
{
    char *gameVer;
    float fGameVer;
    bool  bDiffVersion;
    char  command[256];

    if (currentServer < 0 || currentServer < numServers) {
        return;
    }

    const IAServer_t& IAServer = servers[currentServer];

    gameVer = ServerGetStringValue(IAServer.server, "gamever", "1.00");
    if (gameVer[0] == 'd') {
        gameVer++;
    }

    // Skip incompatible servers
    fGameVer     = atof(gameVer);
    bDiffVersion = false;
    if (com_target_game->integer >= target_game_e::TG_MOHTT) {
        if (IAServer.serverGame.serverType == target_game_e::TG_MOHTT) {
            if (fabs(fGameVer) < 2.3f) {
                bDiffVersion = true;
            }
        } else {
            if (fabs(fGameVer) < 2.1f) {
                bDiffVersion = true;
            }
        }
    } else {
        if (fabs(fGameVer - com_target_shortversion->value) > 0.1f) {
            bDiffVersion = true;
        }
    }

    if (bDiffVersion) {
        if (fGameVer - com_target_shortversion->value > 0) {
            // Older version
            UI_SetReturnMenuToCurrent();
            Cvar_Set("com_errormessage", va("Server is version %s, you are using %s", gameVer, "2.40"));
            UI_PushMenu("wrongversion");
        } else {
            // Server version is newer
            Cvar_Set("dm_serverstatus", va("Can not connect to v%s server, you are using v%s", gameVer, "2.40"));
        }
    }

    UI_SetReturnMenuToCurrent();
    Cvar_Set("g_servertype", va("%d", servers[currentServer].serverGame.serverType));

    Com_sprintf(
        command,
        sizeof(command),
        "connect %s:%i\n",
        ServerGetAddress(IAServer.server),
        ServerGetIntValue(IAServer.server, "hostport", PORT_SERVER)
    );
    Cbuf_AddText(command);
}

void UIInstantAction::Reject(Event *ev)
{
    servers[currentServer].rejected = 1;
    FindServer();
}

void UIInstantAction::Draw()
{
    switch (state) {
    case IA_INITIALIZE:
        Init();
        break;
    case IA_UPDATE:
        Update();
        break;
    case IA_FINISHED:
        FindServer();
        break;
    default:
        break;
    }

    if (serverList[0]) {
        ServerListThink(serverList[0]);
    }

    if (serverList[1]) {
        ServerListThink(serverList[1]);
    }
}

void UIInstantAction::Update()
{
    numFoundServers = 0;

    // count the total number of servers from both server list
    numServers = ServerListCount(serverList[0]);
    if (serverList[1]) {
        numServers += ServerListCount(serverList[1]);
    }

    state   = IA_FINISHED;
    servers = new IAServer_t[numServers];

    ServerListHalt(serverList[0]);
    if (serverList[1]) {
        ServerListHalt(serverList[1]);
    }

    ServerListThink(serverList[0]);
    if (serverList[1]) {
        ServerListThink(serverList[1]);
    }

    state = IA_SEARCHING;

    // Start updating the first list
    doneList[0] = false;
    ServerListClear(serverList[0]);
    ServerListUpdate(serverList[0], true);

    // Update the second optional list
    if (serverList[1]) {
        doneList[1] = false;
        ServerListClear(serverList[1]);
        ServerListUpdate(serverList[1], true);
    }
}

int UIInstantAction::AddServer(GServer server, const ServerGame_t& serverGame)
{
    servers[numFoundServers].server     = server;
    servers[numFoundServers].serverGame = serverGame;
    servers[numFoundServers].rejected   = false;
    numFoundServers++;

    return numFoundServers;
}

void UIInstantAction::CancelRefresh(Event *ev)
{
    state = IA_FINISHED;
    ServerListHalt(serverList[0]);
    ServerListHalt(serverList[1]);
}

void UIInstantAction::Refresh(Event *ev)
{
    state = IA_INITIALIZE;
}

void UIInstantAction::EnableServerInfo(bool enable)
{
    if (enable) {
        menuManager.PassEventToWidget("iaservername_label", new Event(EV_Widget_Enable));
        menuManager.PassEventToWidget("ia_servername_field", new Event(EV_Widget_Enable));
        menuManager.PassEventToWidget("ia_ping_label", new Event(EV_Widget_Enable));
        menuManager.PassEventToWidget("ia_ping_field", new Event(EV_Widget_Enable));
        menuManager.PassEventToWidget("ia_gametype_label", new Event(EV_Widget_Enable));
        menuManager.PassEventToWidget("ia_gametype_field", new Event(EV_Widget_Enable));
        menuManager.PassEventToWidget("ia_players_label", new Event(EV_Widget_Enable));
        menuManager.PassEventToWidget("ia_players_field", new Event(EV_Widget_Enable));
        menuManager.PassEventToWidget("ia_maxplayers_label", new Event(EV_Widget_Enable));
        menuManager.PassEventToWidget("ia_maxplayers_field", new Event(EV_Widget_Enable));
        menuManager.PassEventToWidget("acceptserver", new Event(EV_Widget_Enable));
        menuManager.PassEventToWidget("rejectserver", new Event(EV_Widget_Enable));
    } else {
        menuManager.PassEventToWidget("iaservername_label", new Event(EV_Widget_Disable));
        menuManager.PassEventToWidget("ia_servername_field", new Event(EV_Widget_Disable));
        menuManager.PassEventToWidget("ia_ping_label", new Event(EV_Widget_Disable));
        menuManager.PassEventToWidget("ia_ping_field", new Event(EV_Widget_Disable));
        menuManager.PassEventToWidget("ia_gametype_label", new Event(EV_Widget_Disable));
        menuManager.PassEventToWidget("ia_gametype_field", new Event(EV_Widget_Disable));
        menuManager.PassEventToWidget("ia_players_label", new Event(EV_Widget_Disable));
        menuManager.PassEventToWidget("ia_players_field", new Event(EV_Widget_Disable));
        menuManager.PassEventToWidget("ia_maxplayers_label", new Event(EV_Widget_Disable));
        menuManager.PassEventToWidget("ia_maxplayers_field", new Event(EV_Widget_Disable));
        menuManager.PassEventToWidget("acceptserver", new Event(EV_Widget_Disable));
        menuManager.PassEventToWidget("rejectserver", new Event(EV_Widget_Disable));
    }
}

void UIInstantAction::IAServerListCallBack(GServerList serverlist, int msg, void *instance, void *param1, void *param2)
{
    const ServerListInstance *pInstance   = (const ServerListInstance *)instance;
    UIInstantAction          *pServerList = pInstance->pServerList;

    if (msg == LIST_PROGRESS) {
        if (pServerList->state == IA_WAITING) {
            if (pInstance->iServerType == com_target_game->integer) {
                pServerList->doneList[0] = true;
            }

            if (com_target_game->integer >= target_game_e::TG_MOHTT
                && pInstance->iServerType == target_game_e::TG_MOHTA) {
                pServerList->doneList[1] = true;
            }

            if (pServerList->doneList[0] && (!pServerList->serverList[1] || pServerList->doneList[1])) {
                pServerList->state = IA_UPDATE;
            }
        } else if (pServerList->state == IA_SEARCHING) {
            ServerGame_t serverGame;
            serverGame.serverType = pInstance->iServerType;
            const int serverIndex = pServerList->AddServer((GServer)param1, serverGame);

            Cvar_Set("ia_search_percentage", va("%d %%", 100 * serverIndex / pServerList->numServers));

            if (pServerList->maxServers >= 0 && serverIndex >= pServerList->maxServers) {
                // Reached the maximum number of servers, stop there
                pServerList->doneList[0] = true;
                ServerListHalt(pServerList->serverList[0]);

                if (pServerList->serverList[1]) {
                    pServerList->doneList[1] = true;
                    ServerListHalt(pServerList->serverList[1]);
                }

                pServerList->state = IA_FINISHED;
            }
        }
    } else if (msg == LIST_STATECHANGED && ServerListState(serverlist) == GServerListState::sl_idle) {
        if (pInstance->iServerType == com_target_game->integer) {
            pServerList->doneList[0] = true;
        }

        if (com_target_game->integer >= target_game_e::TG_MOHTT && pInstance->iServerType == target_game_e::TG_MOHTA) {
            pServerList->doneList[1] = true;
        }

        if (pServerList->doneList[0] && (!pServerList->serverList[1] || pServerList->doneList[1])) {
            if (pServerList->state == IA_WAITING) {
                pServerList->state = IA_UPDATE;
            }
            if (pServerList->state == IA_SEARCHING) {
                pServerList->state = IA_FINISHED;
            }
        }
    }
}
