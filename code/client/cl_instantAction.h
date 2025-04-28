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

// Added in 2.30
//  Instantly find a server matching common criterias

#pragma once

#include "../gamespy/goaceng.h"

typedef struct {
    int serverType;
} ServerGame_t;

typedef struct {
    GServer      server;
    ServerGame_t serverGame;
    bool         rejected;
} IAServer_t;

enum IAState_e {
    IA_NONE,
    IA_INITIALIZE,
    IA_WAITING,
    IA_UPDATE,
    IA_SEARCHING,
    IA_FINISHED
};

class UIInstantAction : public UIWidget
{
public:
    CLASS_PROTOTYPE(UIInstantAction);

public:
    UIInstantAction();
    ~UIInstantAction() override;

    void CleanUp();
    void Init();

    int  GetServerIndex(int maxPing, int gameType);
    void ReadIniFile();
    void FindServer();
    void Connect(Event *ev);
    void Reject(Event *ev);
    void Draw();
    void Update();
    int  AddServer(GServer server, const ServerGame_t& serverGame);
    void CancelRefresh(Event *ev);
    void Refresh(Event *ev);
    void EnableServerInfo(bool enable);

private:
    static void IAServerListCallBack(GServerList serverlist, int msg, void *instance, void *param1, void *param2);

private:
    //
    // List
    //
    bool        doneList[2];
    GServerList serverList[2];
    int         maxServers;

    //
    // Current states
    //
    IAState_e state;
    int       numServers;
    int       numFoundServers;

    //
    // Filters
    //
    int minPlayers;
    int startingMaxPing;
    int endingMaxPing;

    //
    // Servers
    //
    IAServer_t *servers;
    int         currentServer;
};
