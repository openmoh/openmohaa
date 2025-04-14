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
    {&EV_UIInstantAction_Refresh,      NULL                           },
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
    doneList[0]     = false;
    doneList[1]     = false;
    servers         = 0;
    serverList[0]   = NULL;
    serverList[1]   = NULL;

    ReadIniFile();
    EnableServerInfo(false);

    menuManager.PassEventToWidget("ia_cancel_button", &EV_Widget_Disable);
    menuManager.PassEventToWidget("ia_refresh_button", &EV_Widget_Disable);
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

    menuManager.PassEventToWidget("ia_cancel_button", &EV_Widget_Disable);
    menuManager.PassEventToWidget("ia_refresh_button", &EV_Widget_Disable);
    menuManager.PassEventToWidget("ia_noserverfound", &EV_Widget_Disable);

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

    menuManager.PassEventToWidget("ia_cancel_button", &EV_Widget_Enable);
    menuManager.PassEventToWidget("searchstatus", &EV_Widget_Enable);
    menuManager.PassEventToWidget("searchstatuslable", &EV_Widget_Enable);
}

int UIInstantAction::GetServerIndex(int maxPing, int gameType)
{
    // FIXME: unimplemented
    return 0;
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
        if (!sscanf(p, "%32s", value)) {
            break;
        }

        pVal = strstr(p, "=");
        if (!pVal) {
            break;
        }

        pVal++;

        if (!sscanf(pVal, "%d", &intValue)) {
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
    // FIXME: unimplemented
}

void UIInstantAction::Connect(Event *ev)
{
    // FIXME: unimplemented
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
    // FIXME: unimplemented
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

void UIInstantAction::EnableServerInfo(bool enable)
{
    if (enable) {
        menuManager.PassEventToWidget("iaservername_label", &EV_Widget_Enable);
        menuManager.PassEventToWidget("ia_servername_field", &EV_Widget_Enable);
        menuManager.PassEventToWidget("ia_ping_label", &EV_Widget_Enable);
        menuManager.PassEventToWidget("ia_ping_field", &EV_Widget_Enable);
        menuManager.PassEventToWidget("ia_gametype_label", &EV_Widget_Enable);
        menuManager.PassEventToWidget("ia_gametype_field", &EV_Widget_Enable);
        menuManager.PassEventToWidget("ia_players_label", &EV_Widget_Enable);
        menuManager.PassEventToWidget("ia_players_field", &EV_Widget_Enable);
        menuManager.PassEventToWidget("ia_maxplayers_label", &EV_Widget_Enable);
        menuManager.PassEventToWidget("ia_maxplayers_field", &EV_Widget_Enable);
        menuManager.PassEventToWidget("acceptserver", &EV_Widget_Enable);
        menuManager.PassEventToWidget("rejectserver", &EV_Widget_Enable);
    } else {
        menuManager.PassEventToWidget("iaservername_label", &EV_Widget_Disable);
        menuManager.PassEventToWidget("ia_servername_field", &EV_Widget_Disable);
        menuManager.PassEventToWidget("ia_ping_label", &EV_Widget_Disable);
        menuManager.PassEventToWidget("ia_ping_field", &EV_Widget_Disable);
        menuManager.PassEventToWidget("ia_gametype_label", &EV_Widget_Disable);
        menuManager.PassEventToWidget("ia_gametype_field", &EV_Widget_Disable);
        menuManager.PassEventToWidget("ia_players_label", &EV_Widget_Disable);
        menuManager.PassEventToWidget("ia_players_field", &EV_Widget_Disable);
        menuManager.PassEventToWidget("ia_maxplayers_label", &EV_Widget_Disable);
        menuManager.PassEventToWidget("ia_maxplayers_field", &EV_Widget_Disable);
        menuManager.PassEventToWidget("acceptserver", &EV_Widget_Disable);
        menuManager.PassEventToWidget("rejectserver", &EV_Widget_Disable);
    }
}

void UIInstantAction::IAServerListCallBack(GServerList serverlist, int msg, void *instance, void *param1, void *param2)
{
    const ServerListInstance *pInstance   = (const ServerListInstance *)instance;
    UIInstantAction          *pServerList = pInstance->pServerList;

    if (msg == LIST_PROGRESS) {
        if (pServerList->state == IA_WAITING) {
            if (pInstance->iServerType == 2) {
                pServerList->doneList[0] = 1;
            }
            if (pInstance->iServerType == 1) {
                pServerList->doneList[1] = 1;
            }
            if (pServerList->doneList[0] && pServerList->doneList[1]) {
                pServerList->state = IA_UPDATE;
            }
        } else if (pServerList->state == IA_SEARCHING) {
            ServerGame_t serverGame;
            serverGame.serverType = pInstance->iServerType;
            const int serverIndex = pServerList->AddServer((GServer)param1, serverGame);

            Cvar_Set("ia_search_percentage", va("%d %%", 100 * serverIndex / pServerList->numServers));

            if (pServerList->maxServers >= 0 && serverIndex >= pServerList->maxServers) {
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
