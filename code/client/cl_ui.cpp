/*
===========================================================================
Copyright (C) 2024 the OpenMoHAA team

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

#include "client.h"

#include "../server/server.h"
#include "../renderercommon/tr_public.h"
#include "../corepp/tiki.h"
#include "../qcommon/localization.h"
#include "../qcommon/q_version.h"

#include "cl_ui.h"
#include "cl_uigamespy.h"

#include <ctime>

typedef struct {
    float             fadetime;
    float             starttime;
    float             endtime;
    float             alpha_start;
    float             alpha_end;
    UIReggedMaterial *material;
} intro_stage_t;

static qboolean             ui_hud;
static class UIFont        *globalFont;
static UIFloatingConsole   *fakk_console;
static UIFloatingDMConsole *dm_console;
static UIFloatingConsole   *developer_console;
static FakkMiniconsole     *mini_console;
static UIGMBox             *gmbox;
static UIDMBox             *dmbox;
static View3D              *view3d;
static Menu                *mainmenu;
static Menu                *hud_weapons;
static Menu                *hud_items;
static Menu                *hud_health;
static Menu                *hud_ammo;
static Menu                *hud_compass;
static Menu                *hud_boss;
static Menu                *crosshairhud;
static Menu                *missionLog;
qboolean                    server_loading;
static qboolean             server_loading_waiting;
static str                 *s_intermediateconsole;
static int                  ui_lastWeapHudState_Owned;
static int                  ui_lastWeapHudState_Equipped = 0xFFFF;
static int                  ui_weapHudTime;
static int                  ui_itemHudTime;
Menu                       *ui_pLoadingMenu;
static Menu                *ui_pConnectingMenu;

static_media_t ui_static_materials;
cvar_t        *cl_greenfps;
cvar_t        *ui_GunneryEvaluation;
cvar_t        *ui_GroinShots;
cvar_t        *ui_RightArmShots;
cvar_t        *ui_LeftArmShots;
cvar_t        *ui_RightLegShots;
cvar_t        *ui_LeftLegShots;
cvar_t        *ui_TorsoShots;
cvar_t        *ui_HeadShots;
cvar_t        *ui_NumEnemysKilled;
cvar_t        *ui_NumObjectsDestroyed;
cvar_t        *ui_NumHitsTaken;
cvar_t        *ui_PreferredWeapon;
cvar_t        *ui_Accuracy;
cvar_t        *ui_NumObjectives;
cvar_t        *ui_NumComplete;
cvar_t        *ui_NumHits;
cvar_t        *ui_NumShotsFired;
cvar_t        *ui_gotmedal;
cvar_t        *ui_success;
cvar_t        *ui_failed;
cvar_t        *ui_returnmenu;
cvar_t        *ui_skip_eamovie;
cvar_t        *ui_skip_titlescreen;
cvar_t        *ui_skip_legalscreen;
cvar_t        *ui_titlescreen_fadein;
cvar_t        *ui_titlescreen_fadeout;
cvar_t        *ui_titlescreen_stay;
cvar_t        *ui_legalscreen_fadein;
cvar_t        *ui_legalscreen_fadeout;
cvar_t        *ui_legalscreen_stay;
cvar_t        *ui_drawcoords;
cvar_t        *ui_minicon;
cvar_t        *ui_gmbox;
cvar_t        *ui_consoleposition;
cvar_t        *ui_inventoryfile;
cvar_t        *ui_console;
cvar_t        *ui_newvidmode;
cvar_t        *ui_crosshair;
cvar_t        *ui_compass;
cvar_t        *ui_weaponsbar;
cvar_t        *ui_weaponsbartime;
cvar_t        *ui_itemsbar;
cvar_t        *ui_health_start;
cvar_t        *ui_health_end;
cvar_t        *ui_gmboxspam;
cvar_t        *ui_debugload;
cvar_t        *sound_overlay;
cvar_t        *ui_compass_scale;

static intro_stage_t intro_stage;
static char          server_mapname[64];
static UIListCtrl   *scoreboardlist;
static Menu         *scoreboard_menu;
static float         scoreboard_x;
static float         scoreboard_y;
static float         scoreboard_w;
static float         scoreboard_h;
static qboolean      scoreboard_header;
cvar_t              *cl_playintro;
cvar_t              *cl_movieaudio;
static unsigned int  startCountLow;
static unsigned int  startCountHigh;
static unsigned int  loadCountLow;
static unsigned int  loadCountHigh;
static unsigned int  loadCount;
static unsigned int  lastTime   = 0;
static unsigned int  updateTime = 0;
static unsigned int  loadNumber;
static unsigned int  totalLoadTime;
static unsigned int  currentLoadTime;
unsigned char        UIListCtrlItem[8];

static const float maxWidthRes  = 1920;
static const float maxHeightRes = 1080;

inventory_t              client_inv;
bind_t                   client_bind;
static str               scoreboard_menuname;
static str               ui_sCurrentLoadingMenu;
static Container<Menu *> hudList;

const UColor UWhiteChatMessageColor(0.75, 0.75, 0.75);
const UColor URedChatMessageColor(1.0, 0.25, 0.25);
const UColor UGreenChatMessageColor(0.0, 1.0, 0.25, 1.0);

void UI_MultiplayerMenuWidgetsUpdate(void);
void UI_MultiplayerMainMenuWidgetsUpdate(void);
void UI_MainMenuWidgetsUpdate(void);

static UIRect2D getDefaultGMBoxRectangle(void);
static UIRect2D getDefaultDMBoxRectangle(void);

class ConsoleHider : public Listener
{
public:
    CLASS_PROTOTYPE(ConsoleHider);

    void HideConsole(Event *ev);
    void DeadConsole(Event *ev);
};

CLASS_DECLARATION(Listener, ConsoleHider, NULL) {
    {&UIFloatingWindow::W_ClosePressed, &ConsoleHider::HideConsole},
    {&W_Destroyed,                      &ConsoleHider::DeadConsole},
    {NULL,                              NULL                      }
};

void ConsoleHider::HideConsole(Event *ev)
{
    UI_CloseConsole();
}

void ConsoleHider::DeadConsole(Event *ev)
{
    if (fakk_console && !fakk_console->IsDying()) {
        delete fakk_console;
        fakk_console = NULL;
    }
}

static ConsoleHider s_consolehider;

/*
====================
ConsoleCommandHandler
====================
*/
static void ConsoleCommandHandler(const char *txt)
{
    Cbuf_AddText(txt);
}

/*
====================
getDefaultConsoleRectangle
====================
*/
static UIRect2D getDefaultConsoleRectangle(void)
{
    float    f[4];
    int      i;
    UIRect2D rect;

    if (sscanf(ui_consoleposition->string, "%f %f %f %f", &f[0], &f[1], &f[2], &f[3]) == 4) {
        for (i = 0; i < 4; i++) {
            f[i] = floor(f[i]);
        }

        rect.pos.x       = f[0] - uid.vidWidth;
        rect.pos.y       = f[1] - uid.vidHeight;
        rect.size.width  = f[2] + 50.0;
        rect.size.height = f[3] + 50.0;
    } else {
        rect.pos.x       = 25.0;
        rect.pos.y       = 25.0;
        rect.size.width  = (uid.vidWidth - 50);
        rect.size.height = (uid.vidHeight / 2);
    }

    return rect;
}

class DMConsoleHider : public Listener
{
public:
    CLASS_PROTOTYPE(DMConsoleHider);

    void HideDMConsole(Event *ev);
    void DeadDMConsole(Event *ev);
};

CLASS_DECLARATION(Listener, DMConsoleHider, NULL) {
    {&UIFloatingWindow::W_ClosePressed, &DMConsoleHider::HideDMConsole},
    {&W_Deactivated,                    &DMConsoleHider::HideDMConsole},
    {&W_Destroyed,                      &DMConsoleHider::DeadDMConsole},
    {NULL,                              NULL                          }
};

void DMConsoleHider::HideDMConsole(Event *ev)
{
    UI_CloseDMConsole();
}

void DMConsoleHider::DeadDMConsole(Event *ev)
{
    if (dm_console && !dm_console->IsDying()) {
        delete dm_console;
        dm_console = NULL;
    }
}

static DMConsoleHider s_dmconsolehider;

/*
====================
GetClientState
====================
*/
static void GetClientState(uiClientState_t *state)
{
    state->connectPacketCount = clc.connectPacketCount;
    state->connState          = clc.state;
    Q_strncpyz(state->servername, clc.servername, sizeof(state->servername));
    Q_strncpyz(state->updateInfoString, cls.updateInfoString, sizeof(state->updateInfoString));
    Q_strncpyz(state->messageString, clc.serverMessage, sizeof(state->messageString));
    state->clientNum = cl.snap.ps.clientNum;
}

/*
====================
LAN_LoadCachedServers
====================
*/
void LAN_LoadCachedServers(void)
{
    int          size;
    fileHandle_t fileIn;
    cls.numglobalservers = cls.nummplayerservers = cls.numfavoriteservers = 0;
    cls.numGlobalServerAddresses                                          = 0;
    if (FS_BaseDir_FOpenFileRead("servercache.dat", &fileIn)) {
        FS_Read(&cls.numglobalservers, sizeof(int), fileIn);
        FS_Read(&cls.nummplayerservers, sizeof(int), fileIn);
        FS_Read(&cls.numfavoriteservers, sizeof(int), fileIn);
        FS_Read(&size, sizeof(int), fileIn);
        if (size == sizeof(cls.globalServers) + sizeof(cls.favoriteServers) + sizeof(cls.mplayerServers)) {
            FS_Read(&cls.globalServers, sizeof(cls.globalServers), fileIn);
            FS_Read(&cls.mplayerServers, sizeof(cls.mplayerServers), fileIn);
            FS_Read(&cls.favoriteServers, sizeof(cls.favoriteServers), fileIn);
        } else {
            cls.numglobalservers = cls.nummplayerservers = cls.numfavoriteservers = 0;
            cls.numGlobalServerAddresses                                          = 0;
        }
        FS_FCloseFile(fileIn);
    }
}

/*
====================
LAN_SaveServersToCache
====================
*/
void LAN_SaveServersToCache(void)
{
    int          size;
    fileHandle_t fileOut = FS_BaseDir_FOpenFileWrite("servercache.dat");
    FS_Write(&cls.numglobalservers, sizeof(int), fileOut);
    FS_Write(&cls.nummplayerservers, sizeof(int), fileOut);
    FS_Write(&cls.numfavoriteservers, sizeof(int), fileOut);
    size = sizeof(cls.globalServers) + sizeof(cls.favoriteServers) + sizeof(cls.mplayerServers);
    FS_Write(&size, sizeof(int), fileOut);
    FS_Write(&cls.globalServers, sizeof(cls.globalServers), fileOut);
    FS_Write(&cls.mplayerServers, sizeof(cls.mplayerServers), fileOut);
    FS_Write(&cls.favoriteServers, sizeof(cls.favoriteServers), fileOut);
    FS_FCloseFile(fileOut);
}

/*
====================
LAN_ResetPings
====================
*/
static void LAN_ResetPings(int source)
{
    int           count, i;
    serverInfo_t *servers = NULL;
    count                 = 0;

    switch (source) {
    case AS_LOCAL:
        servers = &cls.localServers[0];
        count   = MAX_OTHER_SERVERS;
        break;
    case AS_MPLAYER:
        servers = &cls.mplayerServers[0];
        count   = MAX_OTHER_SERVERS;
        break;
    case AS_GLOBAL:
        servers = &cls.globalServers[0];
        count   = MAX_GLOBAL_SERVERS;
        break;
    case AS_FAVORITES:
        servers = &cls.favoriteServers[0];
        count   = MAX_OTHER_SERVERS;
        break;
    }
    if (servers) {
        for (i = 0; i < count; i++) {
            servers[i].ping = -1;
        }
    }
}

/*
====================
LAN_AddServer
====================
*/
static int LAN_AddServer(int source, const char *name, const char *address)
{
    int           max, *count, i;
    netadr_t      adr;
    serverInfo_t *servers = NULL;
    max                   = MAX_OTHER_SERVERS;
    count                 = NULL;

    switch (source) {
    case AS_LOCAL:
        count   = &cls.numlocalservers;
        servers = &cls.localServers[0];
        break;
    case AS_MPLAYER:
        count   = &cls.nummplayerservers;
        servers = &cls.mplayerServers[0];
        break;
    case AS_GLOBAL:
        max     = MAX_GLOBAL_SERVERS;
        count   = &cls.numglobalservers;
        servers = &cls.globalServers[0];
        break;
    case AS_FAVORITES:
        count   = &cls.numfavoriteservers;
        servers = &cls.favoriteServers[0];
        break;
    }
    if (servers && *count < max) {
        NET_StringToAdr(address, &adr, NA_IP);
        for (i = 0; i < *count; i++) {
            if (NET_CompareAdr(servers[i].adr, adr)) {
                break;
            }
        }
        if (i >= *count) {
            servers[*count].adr = adr;
            Q_strncpyz(servers[*count].hostName, name, sizeof(servers[*count].hostName));
            servers[*count].visible = qtrue;
            (*count)++;
            return 1;
        }
        return 0;
    }
    return -1;
}

/*
====================
LAN_RemoveServer
====================
*/
static void LAN_RemoveServer(int source, const char *addr)
{
    int          *count, i;
    serverInfo_t *servers = NULL;
    count                 = NULL;
    switch (source) {
    case AS_LOCAL:
        count   = &cls.numlocalservers;
        servers = &cls.localServers[0];
        break;
    case AS_MPLAYER:
        count   = &cls.nummplayerservers;
        servers = &cls.mplayerServers[0];
        break;
    case AS_GLOBAL:
        count   = &cls.numglobalservers;
        servers = &cls.globalServers[0];
        break;
    case AS_FAVORITES:
        count   = &cls.numfavoriteservers;
        servers = &cls.favoriteServers[0];
        break;
    }
    if (servers) {
        netadr_t comp;
        NET_StringToAdr(addr, &comp, NA_IP);
        for (i = 0; i < *count; i++) {
            if (NET_CompareAdr(comp, servers[i].adr)) {
                int j = i;
                while (j < *count - 1) {
                    Com_Memcpy(&servers[j], &servers[j + 1], sizeof(servers[j]));
                    j++;
                }
                (*count)--;
                break;
            }
        }
    }
}

/*
====================
LAN_GetServerCount
====================
*/
static int LAN_GetServerCount(int source)
{
    switch (source) {
    case AS_LOCAL:
        return cls.numlocalservers;
        break;
    case AS_MPLAYER:
        return cls.nummplayerservers;
        break;
    case AS_GLOBAL:
        return cls.numglobalservers;
        break;
    case AS_FAVORITES:
        return cls.numfavoriteservers;
        break;
    }
    return 0;
}

/*
====================
LAN_GetLocalServerAddressString
====================
*/
static void LAN_GetServerAddressString(int source, int n, char *buf, int buflen)
{
    switch (source) {
    case AS_LOCAL:
        if (n >= 0 && n < MAX_OTHER_SERVERS) {
            Q_strncpyz(buf, NET_AdrToString(cls.localServers[n].adr), buflen);
            return;
        }
        break;
    case AS_MPLAYER:
        if (n >= 0 && n < MAX_OTHER_SERVERS) {
            Q_strncpyz(buf, NET_AdrToString(cls.mplayerServers[n].adr), buflen);
            return;
        }
        break;
    case AS_GLOBAL:
        if (n >= 0 && n < MAX_GLOBAL_SERVERS) {
            Q_strncpyz(buf, NET_AdrToString(cls.globalServers[n].adr), buflen);
            return;
        }
        break;
    case AS_FAVORITES:
        if (n >= 0 && n < MAX_OTHER_SERVERS) {
            Q_strncpyz(buf, NET_AdrToString(cls.favoriteServers[n].adr), buflen);
            return;
        }
        break;
    }
    buf[0] = '\0';
}

/*
====================
LAN_GetServerInfo
====================
*/
static void LAN_GetServerInfo(int source, int n, char *buf, int buflen)
{
    char          info[MAX_STRING_CHARS];
    serverInfo_t *server = NULL;
    info[0]              = '\0';
    switch (source) {
    case AS_LOCAL:
        if (n >= 0 && n < MAX_OTHER_SERVERS) {
            server = &cls.localServers[n];
        }
        break;
    case AS_MPLAYER:
        if (n >= 0 && n < MAX_OTHER_SERVERS) {
            server = &cls.mplayerServers[n];
        }
        break;
    case AS_GLOBAL:
        if (n >= 0 && n < MAX_GLOBAL_SERVERS) {
            server = &cls.globalServers[n];
        }
        break;
    case AS_FAVORITES:
        if (n >= 0 && n < MAX_OTHER_SERVERS) {
            server = &cls.favoriteServers[n];
        }
        break;
    }
    if (server && buf) {
        buf[0] = '\0';
        Info_SetValueForKey(info, "hostname", server->hostName);
        Info_SetValueForKey(info, "mapname", server->mapName);
        Info_SetValueForKey(info, "clients", va("%i", server->clients));
        Info_SetValueForKey(info, "sv_maxclients", va("%i", server->maxClients));
        Info_SetValueForKey(info, "ping", va("%i", server->ping));
        Info_SetValueForKey(info, "minping", va("%i", server->minPing));
        Info_SetValueForKey(info, "maxping", va("%i", server->maxPing));
        Info_SetValueForKey(info, "game", server->game);
        Info_SetValueForKey(info, "gametype", va("%i", server->gameType));
        Info_SetValueForKey(info, "nettype", va("%i", server->netType));
        Info_SetValueForKey(info, "addr", NET_AdrToString(server->adr));
        Q_strncpyz(buf, info, buflen);
    } else {
        if (buf) {
            buf[0] = '\0';
        }
    }
}

/*
====================
LAN_GetServerPing
====================
*/
static int LAN_GetServerPing(int source, int n)
{
    serverInfo_t *server = NULL;
    switch (source) {
    case AS_LOCAL:
        if (n >= 0 && n < MAX_OTHER_SERVERS) {
            server = &cls.localServers[n];
        }
        break;
    case AS_MPLAYER:
        if (n >= 0 && n < MAX_OTHER_SERVERS) {
            server = &cls.mplayerServers[n];
        }
        break;
    case AS_GLOBAL:
        if (n >= 0 && n < MAX_GLOBAL_SERVERS) {
            server = &cls.globalServers[n];
        }
        break;
    case AS_FAVORITES:
        if (n >= 0 && n < MAX_OTHER_SERVERS) {
            server = &cls.favoriteServers[n];
        }
        break;
    }
    if (server) {
        return server->ping;
    }
    return -1;
}

/*
====================
LAN_GetServerPtr
====================
*/
static serverInfo_t *LAN_GetServerPtr(int source, int n)
{
    switch (source) {
    case AS_LOCAL:
        if (n >= 0 && n < MAX_OTHER_SERVERS) {
            return &cls.localServers[n];
        }
        break;
    case AS_MPLAYER:
        if (n >= 0 && n < MAX_OTHER_SERVERS) {
            return &cls.mplayerServers[n];
        }
        break;
    case AS_GLOBAL:
        if (n >= 0 && n < MAX_GLOBAL_SERVERS) {
            return &cls.globalServers[n];
        }
        break;
    case AS_FAVORITES:
        if (n >= 0 && n < MAX_OTHER_SERVERS) {
            return &cls.favoriteServers[n];
        }
        break;
    }
    return NULL;
}

/*
====================
LAN_CompareServers
====================
*/
static int LAN_CompareServers(int source, int sortKey, int sortDir, int s1, int s2)
{
    int           res;
    serverInfo_t *server1, *server2;

    server1 = LAN_GetServerPtr(source, s1);
    server2 = LAN_GetServerPtr(source, s2);
    if (!server1 || !server2) {
        return 0;
    }

    res = 0;
    switch (sortKey) {
    case SORT_HOST:
        res = Q_stricmp(server1->hostName, server2->hostName);
        break;

    case SORT_MAP:
        res = Q_stricmp(server1->mapName, server2->mapName);
        break;
    case SORT_CLIENTS:
        if (server1->clients < server2->clients) {
            res = -1;
        } else if (server1->clients > server2->clients) {
            res = 1;
        } else {
            res = 0;
        }
        break;
    case SORT_GAME:
        if (server1->gameType < server2->gameType) {
            res = -1;
        } else if (server1->gameType > server2->gameType) {
            res = 1;
        } else {
            res = 0;
        }
        break;
    case SORT_PING:
        if (server1->ping < server2->ping) {
            res = -1;
        } else if (server1->ping > server2->ping) {
            res = 1;
        } else {
            res = 0;
        }
        break;
    }

    if (sortDir) {
        if (res < 0) {
            return 1;
        }
        if (res > 0) {
            return -1;
        }
        return 0;
    }
    return res;
}

/*
====================
LAN_GetPingQueueCount
====================
*/
static int LAN_GetPingQueueCount(void)
{
    return (CL_GetPingQueueCount());
}

/*
====================
LAN_ClearPing
====================
*/
static void LAN_ClearPing(int n)
{
    CL_ClearPing(n);
}

/*
====================
LAN_GetPing
====================
*/
static void LAN_GetPing(int n, char *buf, int buflen, int *pingtime)
{
    CL_GetPing(n, buf, buflen, pingtime);
}

/*
====================
LAN_GetPingInfo
====================
*/
static void LAN_GetPingInfo(int n, char *buf, int buflen)
{
    CL_GetPingInfo(n, buf, buflen);
}

/*
====================
LAN_MarkServerVisible
====================
*/
static void LAN_MarkServerVisible(int source, int n, qboolean visible)
{
    if (n == -1) {
        int           count  = MAX_OTHER_SERVERS;
        serverInfo_t *server = NULL;
        switch (source) {
        case AS_LOCAL:
            server = &cls.localServers[0];
            break;
        case AS_MPLAYER:
            server = &cls.mplayerServers[0];
            break;
        case AS_GLOBAL:
            server = &cls.globalServers[0];
            count  = MAX_GLOBAL_SERVERS;
            break;
        case AS_FAVORITES:
            server = &cls.favoriteServers[0];
            break;
        }
        if (server) {
            for (n = 0; n < count; n++) {
                server[n].visible = visible;
            }
        }

    } else {
        switch (source) {
        case AS_LOCAL:
            if (n >= 0 && n < MAX_OTHER_SERVERS) {
                cls.localServers[n].visible = visible;
            }
            break;
        case AS_MPLAYER:
            if (n >= 0 && n < MAX_OTHER_SERVERS) {
                cls.mplayerServers[n].visible = visible;
            }
            break;
        case AS_GLOBAL:
            if (n >= 0 && n < MAX_GLOBAL_SERVERS) {
                cls.globalServers[n].visible = visible;
            }
            break;
        case AS_FAVORITES:
            if (n >= 0 && n < MAX_OTHER_SERVERS) {
                cls.favoriteServers[n].visible = visible;
            }
            break;
        }
    }
}

/*
=======================
LAN_ServerIsVisible
=======================
*/
static int LAN_ServerIsVisible(int source, int n)
{
    switch (source) {
    case AS_LOCAL:
        if (n >= 0 && n < MAX_OTHER_SERVERS) {
            return cls.localServers[n].visible;
        }
        break;
    case AS_MPLAYER:
        if (n >= 0 && n < MAX_OTHER_SERVERS) {
            return cls.mplayerServers[n].visible;
        }
        break;
    case AS_GLOBAL:
        if (n >= 0 && n < MAX_GLOBAL_SERVERS) {
            return cls.globalServers[n].visible;
        }
        break;
    case AS_FAVORITES:
        if (n >= 0 && n < MAX_OTHER_SERVERS) {
            return cls.favoriteServers[n].visible;
        }
        break;
    }
    return qfalse;
}

/*
=======================
LAN_UpdateVisiblePings
=======================
*/
qboolean LAN_UpdateVisiblePings(int source)
{
    return CL_UpdateVisiblePings_f(source);
}

/*
====================
LAN_GetServerStatus
====================
*/
int LAN_GetServerStatus(const char *serverAddress, char *serverStatus, int maxLen)
{
    return CL_ServerStatus(serverAddress, serverStatus, maxLen);
}

/*
====================
CL_GetGlConfig
====================
*/
static void CL_GetGlconfig(glconfig_t *config)
{
    *config = cls.glconfig;
}

/*
====================
CL_GetClipboardData
====================
*/
static void CL_GetClipboardData(char *buf, int buflen)
{
    char *cbd;

    cbd = Sys_GetClipboardData();

    if (!cbd) {
        *buf = 0;
        return;
    }

    Q_strncpyz(buf, cbd, buflen);

    Z_Free(cbd);
}

/*
====================
Key_KeynumToStringBuf
====================
*/
static void Key_KeynumToStringBuf(int keynum, char *buf, int buflen)
{
    Q_strncpyz(buf, Key_KeynumToString(keynum), buflen);
}

/*
====================
Key_GetBindingBuf
====================
*/
static void Key_GetBindingBuf(int keynum, char *buf, int buflen)
{
    const char *value;

    value = Key_GetBinding(keynum);
    if (value) {
        Q_strncpyz(buf, value, buflen);
    } else {
        *buf = 0;
    }
}

/*
====================
GetConfigString
====================
*/
static int GetConfigString(int index, char *buf, int size)
{
    int offset;

    if (index < 0 || index >= MAX_CONFIGSTRINGS) {
        return qfalse;
    }

    offset = cl.gameState.stringOffsets[index];
    if (!offset) {
        if (size) {
            buf[0] = 0;
        }
        return qfalse;
    }

    Q_strncpyz(buf, cl.gameState.stringData + offset, size);

    return qtrue;
}

/*
====================
FloatAsInt
====================
*/
static int FloatAsInt(float f)
{
    int temp;

    *(float *)&temp = f;

    return temp;
}

qboolean UI_usesUniqueCDKey(void)
{
    return qfalse;
}

/*
====================
getDefaultConsoleRectangle
====================
*/
static UIRect2D getDefaultDMConsoleRectangle(void)
{
    float    f[4];
    int      i;
    UIRect2D rect;

    if (sscanf(ui_consoleposition->string, "%f %f %f %f", &f[0], &f[1], &f[2], &f[3]) == 4) {
        for (i = 0; i < 4; i++) {
            f[i] = floor(f[i]);
        }

        rect.pos.x       = f[0] - uid.vidWidth;
        rect.pos.y       = f[1] - uid.vidHeight;
        rect.size.width  = f[2] + 50.0;
        rect.size.height = f[3] + 50.0;
    } else {
        rect.pos.x       = 0;
        rect.pos.y       = uid.vidHeight * 0.58;
        rect.size.width  = uid.vidWidth;
        rect.size.height = uid.vidHeight * 0.415;
    }

    return rect;
}

/*
====================
getDefaultConsoleRectangle
====================
*/
static UIRect2D getQuickMessageDMConsoleRectangle(void)
{
    float    f[4];
    int      i;
    UIRect2D rect;

    if (sscanf(ui_consoleposition->string, "%f %f %f %f", &f[0], &f[1], &f[2], &f[3]) == 4) {
        for (i = 0; i < 4; i++) {
            f[i] = floor(f[i]);
        }

        rect.pos.x       = f[0] - uid.vidWidth;
        rect.pos.y       = f[1] - uid.vidHeight;
        rect.size.width  = f[2] + 50.0;
        rect.size.height = f[3] + 50.0;
    } else {
        rect.pos.x      = 0;
        rect.pos.y      = uid.vidHeight * 0.66;
        rect.size.width = uid.vidWidth;
        // Fixed in 2.0
        //  Was 38.0 in 1.11 and below
        //  This prevents characters to be seen from the DM console
        //  in the quick message console
        rect.size.height = 36.0 * uid.scaleRes[1];
    }

    return rect;
}

/*
====================
UI_DMMessageModesMatch
====================
*/
static qboolean UI_DMMessageModesMatch(int iMode)
{
    qboolean bQuickMessage = qfalse;

    if (iMode && (iMode != 300 || dm_console->GetQuickMessageMode())) {
        if (iMode < 0) {
            bQuickMessage = qtrue;
            iMode         = -iMode;
        }

        if (dm_console->GetQuickMessageMode() == bQuickMessage) {
            return dm_console->GetMessageMode() == iMode;
        } else {
            return qfalse;
        }
    } else {
        return qtrue;
    }
}

/*
====================
UI_SetDMConsoleMode
====================
*/
static void UI_SetDMConsoleMode(int iMode)
{
    qboolean bQuickMessage;

    if (UI_DMMessageModesMatch(iMode)) {
        return;
    }

    bQuickMessage = qfalse;

    if (iMode < 0) {
        bQuickMessage = qtrue;
        iMode         = -iMode;
    }

    if (iMode == 300) {
        iMode = dm_console->GetMessageMode();
    }

    if (dm_console->GetQuickMessageMode()) {
        if (!bQuickMessage) {
            dm_console->setFrame(getDefaultDMConsoleRectangle());
            dm_console->SetQuickMessageMode(qfalse);
        }
    } else {
        if (bQuickMessage) {
            dm_console->setFrame(getQuickMessageDMConsoleRectangle());
            dm_console->SetQuickMessageMode(bQuickMessage);
        }
    }

    dm_console->SetMessageMode(iMode);

    if (bQuickMessage) {
        if (iMode == 100) {
            dm_console->setTitle(Sys_LV_CL_ConvertString("Quick Chat (Press Enter to send) : Messaging to All"));
        } else if (iMode == 200) {
            dm_console->setTitle(Sys_LV_CL_ConvertString("Quick Chat (Press Enter to send) : Messaging to Team"));
        } else {
            dm_console->setTitle(
                va("%s %i", Sys_LV_CL_ConvertString("Quick Chat (Press Enter to send) : Messaging to Client"), iMode)
            );
        }
    } else {
        if (iMode == 100) {
            dm_console->setTitle(Sys_LV_CL_ConvertString(
                "Chat Window (Enter to send/close) (all|team|private to change mode) : Messaging to All"
            ));
        } else if (iMode == 200) {
            dm_console->setTitle(Sys_LV_CL_ConvertString(
                "Chat Window (Enter to send/close) (all|team|private to change mode) : Messaging to Team"
            ));
        } else {
            dm_console->setTitle(
                va("%s %i",
                   Sys_LV_CL_ConvertString(
                       "Chat Window (Enter to send/close) (all|team|private to change mode) : Messaging to Client"
                   ),
                   iMode)
            );
        }
    }
}

static void DMConsoleCommandHandler(const char *txt)
{
    int  iMode;
    char szStringOut[1024];

    iMode = 0;

    if (dm_console->GetMessageMode() != 100) {
        if (dm_console->GetMessageMode() == 200) {
            iMode = -1;
        } else {
            iMode = dm_console->GetMessageMode();
        }
    }

    Com_sprintf(szStringOut, sizeof(szStringOut), "dmmessage %i %s\n", iMode, txt);
    CL_AddReliableCommand(szStringOut, qfalse);
}

/*
====================
getScreenWidth
====================
*/
static float getScreenWidth()
{
    if (uid.bHighResScaling) {
        return maxWidthRes;
    } else {
        return uid.vidWidth;
    }
}

/*
====================
getNewConsole
====================
*/
UIFloatingConsole *getNewConsole()
{
    const char *consoleName = va("%s console version %s", PRODUCT_NAME, PRODUCT_VERSION);

    UIFloatingConsole *console = new UIFloatingConsole;
    console->Create(NULL, getDefaultConsoleRectangle(), consoleName, UWindowColor, UHudColor);
    console->setConsoleHandler(ConsoleCommandHandler);
    console->setConsoleBackground(UBlack, 0.8f);
    console->setShow(false);
    console->Connect(&s_consolehider, UIFloatingWindow::W_ClosePressed, UIFloatingWindow::W_ClosePressed);
    console->Connect(&s_consolehider, W_Destroyed, W_Destroyed);

    return console;
}

/*
====================
getNewDMConsole
====================
*/
UIFloatingDMConsole *getNewDMConsole()
{
    UIFloatingDMConsole *console = new UIFloatingDMConsole;
    Event               *event;

    console->Create(
        NULL,
        getDefaultDMConsoleRectangle(),
        "Chat Window (Enter to send/close) (all|team|private to change message mode) : Messaging to All",
        UWindowColor,
        UHudColor
    );

    console->setConsoleHandler(DMConsoleCommandHandler);
    console->setConsoleBackground(UBlack, 0.8f);
    console->setShow(false);
    console->Connect(&s_dmconsolehider, UIFloatingWindow::W_ClosePressed, UIFloatingWindow::W_ClosePressed);
    console->Connect(&s_dmconsolehider, W_Destroyed, W_Destroyed);

    event = new Event(EV_Widget_Disable);
    console->PassEventToWidget("closebutton", event);

    event = new Event(EV_Widget_Disable);
    console->PassEventToWidget("minimizebutton", event);

    return console;
}

/*
====================
getDefaultGMBoxRectangle
====================
*/
static UIRect2D getDefaultGMBoxRectangle(void)
{
    UIRect2D dmRect = getDefaultDMBoxRectangle();
    float    height = uid.vidHeight * ui_compass_scale->value * 0.25f;
    float    y      = dmRect.size.height + dmRect.pos.y;

    if (height < y) {
        height = y;
    }

    return UIRect2D(20.0f, height, (getScreenWidth() - 20) * uid.scaleRes[0], 128.0f * uid.scaleRes[1]);
}

/*
====================
getDefaultDMBoxRectangle
====================
*/
static UIRect2D getDefaultDMBoxRectangle(void)
{
    float width;
    float screenWidth = getScreenWidth();

    width = screenWidth * uid.scaleRes[0] * ui_compass_scale->value * 0.2f;

    return UIRect2D(width, 0, (screenWidth - (width + 192.0f)) * uid.scaleRes[0], 120.0f * uid.scaleRes[1]);
}

/*
====================
UI_GetObjectivesTop
====================
*/
float UI_GetObjectivesTop(void)
{
    return getDefaultGMBoxRectangle().pos.y;
}

/*
====================
UI_GetObjectivesTop
====================
*/
void UI_GetHighResolutionScale(vec2_t scale)
{
    if (uid.bHighResScaling) {
        scale[0] = uid.scaleRes[0];
        scale[1] = uid.scaleRes[1];
    } else {
        scale[0] = scale[1] = 1.0;
    }
}

/*
====================
UI_ShowHudList
====================
*/
void UI_ShowHudList(void)
{
    for (int i = 1; i <= hudList.NumObjects(); i++) {
        hudList.ObjectAt(i)->ForceShow();
    }
}

/*
====================
UI_HideHudList
====================
*/
void UI_HideHudList(void)
{
    for (int i = 1; i <= hudList.NumObjects(); i++) {
        hudList.ObjectAt(i)->ForceHide();
    }
}

/*
====================
UI_DisplayHudList
====================
*/
void UI_DisplayHudList(void)
{
    for (int i = 1; i <= hudList.NumObjects(); i++) {
        Menu     *pMenu   = hudList.ObjectAt(i);
        UIWidget *pWidget = pMenu->GetContainerWidget();

        pMenu->ForceShow();
        pWidget->Display(uWinMan.getFrame(), 1.0);
    }
}

/*
====================
UI_PrintConsole
====================
*/
void UI_PrintConsole(const char *msg)
{
    const UColor *pColor = NULL;
    const char   *pszString;
    char          szBlah[1024];
    qboolean      bPrintedDMBox = qfalse;

    pszString = msg;

    if (*pszString > 0 && (unsigned char)*pszString < MESSAGE_MAX) {
        qboolean bNormalMessage = qfalse;
        qboolean bDMMessage     = qfalse;
        qboolean bBold          = qfalse;
        qboolean bDeathMessage  = qfalse;

        switch (*pszString) {
        case MESSAGE_YELLOW:
            bNormalMessage = qtrue;
            pColor         = &UHudColor;
            break;
        case MESSAGE_CHAT_WHITE:
            bDMMessage = qtrue;
            pColor     = &UWhiteChatMessageColor;
            break;
        case MESSAGE_WHITE:
            bBold  = qtrue;
            pColor = &UWhite;
            break;
        case MESSAGE_CHAT_RED:
            bDeathMessage = MESSAGE_CHAT_RED;
            pColor        = &URedChatMessageColor;
            break;
        case MESSAGE_CHAT_GREEN:
            bDeathMessage = MESSAGE_CHAT_GREEN;
            pColor        = &UGreenChatMessageColor;
            break;
        }

        pszString++;

        //
        // print to the deathmatch console
        //
        if (dm_console && !bNormalMessage) {
            if (bDMMessage) {
                dm_console->AddDMMessageText(pszString, pColor);
            } else {
                dm_console->AddText(pszString, pColor);
            }
        }

        //
        // print to the deathmatch message box
        //
        if (dmbox) {
            // Changed in OPM
            //  Avoid touching/copying buffers
            /*
            if (bDMMessage) {
                *szString = MESSAGE_CHAT_WHITE;
                dmbox->Print(szString);
            } else if (bDeathMessage) {
                *szString = bDeathMessage;
                dmbox->Print(szString);
            }
            */

            if (bDMMessage || bDeathMessage) {
                dmbox->Print(msg);
                bPrintedDMBox = qtrue;
            }
        }

        //
        // print to the game message box
        //
        if (gmbox && !bPrintedDMBox) {
            if (bBold) {
                // Changed in OPM
                //  Avoid touching/copying buffers
                //*szString = MESSAGE_WHITE;
                //gmbox->Print(szString);

                gmbox->Print(msg);
                uii.Snd_PlaySound("objective_text");
            } else {
                gmbox->Print(pszString);
            }

            if (!ui_gmboxspam->integer) {
                return;
            }

            if (!bDMMessage) {
                Q_strncpyz(szBlah, "Game Message: ", sizeof(szBlah));
                Q_strcat(szBlah, sizeof(szBlah), pszString);
                pszString = szBlah;
            }
        }
    }

    if (*pszString == '\a') {
        pszString++;
        if (mini_console) {
            mini_console->Print(pszString);
            return;
        }
    }

    if (fakk_console) {
        if (s_intermediateconsole) {
            fakk_console->AddText(*s_intermediateconsole, NULL);
            delete s_intermediateconsole;
            s_intermediateconsole = NULL;
        }

        fakk_console->AddText(pszString, pColor);

        if (mini_console) {
            mini_console->Print(pszString);
        }
    } else {
        if (!s_intermediateconsole) {
            s_intermediateconsole = new str;
        }

        *s_intermediateconsole = msg;
    }
}

/*
====================
UI_PrintDeveloperConsole
====================
*/
void UI_PrintDeveloperConsole(const char *msg)
{
    if (!developer_console) {
        return;
    }

    developer_console->AddText(msg, NULL);
}

/*
====================
UI_UpdateContinueGame
====================
*/
void UI_UpdateContinueGame(void)
{
    cvar_t     *var;
    const char *name;
    const char *archive_name;
    int         length;
    Event      *event;

    var  = Cvar_Get("g_lastsave", "", 0);
    name = var->string;

    if (*name) {
        archive_name = Com_GetArchiveFileName(name, "sav");
        length       = FS_ReadFileEx(archive_name, NULL, qtrue);

        if (length != -1) {
            event = new Event(EV_Widget_Enable);
            menuManager.PassEventToWidget("continue game", event);

            event = new Event(EV_Widget_Disable);
            menuManager.PassEventToWidget("new game", event);
            return;
        }

        Cvar_Set("g_lastsave", "");
    }

    event = new Event(EV_Widget_Disable);
    menuManager.PassEventToWidget("continue game", event);

    event = new Event(EV_Widget_Enable);
    menuManager.PassEventToWidget("new game", event);
}

/*
====================
UI_MenuActive
====================
*/
qboolean UI_MenuActive(void)
{
    return menuManager.CurrentMenu() != NULL;
}

/*
====================
UI_FocusMenuIfExists
====================
*/
void UI_FocusMenuIfExists(void)
{
    Menu *currentMenu;
    int   r_mode;

    currentMenu = menuManager.CurrentMenu();
    r_mode      = Cvar_VariableValue("r_mode");

    if (currentMenu) {
        IN_MouseOn();

        if (view3d->IsActive()) {
            uWinMan.DeactivateCurrentControl();
        }

        currentMenu->ActivateMenu();

        if (ui_newvidmode->integer == -1) {
            Cvar_Get("ui_newvidmode", va("%d", r_mode), CVAR_RESETSTRING);
            Cvar_SetValue("ui_newvidmode", r_mode);
        }
    } else {
        UI_ActivateView3D();
        IN_MouseOff();
        Cvar_SetValue("ui_newvidmode", -1);
    }
}

/*
====================
UI_OpenConsole
====================
*/
void UI_OpenConsole(void)
{
    if (!fakk_console) {
        return;
    }

    fakk_console->setShow(true);
    uWinMan.ActivateControl(fakk_console);
    IN_MouseOn();
}

/*
====================
UI_OpenConsole
====================
*/
qboolean UI_ConsoleIsVisible(void)
{
    return fakk_console && fakk_console->IsVisible();
}

/*
====================
UI_ConsoleIsOpen
====================
*/
qboolean UI_ConsoleIsOpen(void)
{
    return fakk_console && fakk_console->IsVisible() && fakk_console->IsActive();
}

/*
====================
UI_CloseConsole
====================
*/
void UI_CloseConsole(void)
{
    if (fakk_console && fakk_console->getShow()) {
        fakk_console->setShow(false);
    }
    UI_FocusMenuIfExists();
}

/*
====================
UI_ToggleConsole
====================
*/
void UI_ToggleConsole(void)
{
    if (!fakk_console) {
        return;
    }

    if (fakk_console->IsVisible()) {
        UI_CloseConsole();
    } else {
        UI_OpenConsole();
    }
}

/*
====================
UI_OpenDMConsole
====================
*/
void UI_OpenDMConsole(int iMode)
{
    if (!dm_console) {
        return;
    }

    UI_SetDMConsoleMode(iMode);
    dm_console->setShow(true);
    uWinMan.ActivateControl(dm_console);
    IN_MouseOff();
}

/*
====================
UI_CloseDMConsole
====================
*/
void UI_CloseDMConsole(void)
{
    if (dm_console && dm_console->getShow()) {
        dm_console->setShow(false);
    }
    UI_FocusMenuIfExists();
}

/*
====================
UI_ToggleDMConsole
====================
*/
void UI_ToggleDMConsole(int iMode)
{
    if (!dm_console) {
        return;
    }

    if (dm_console->IsVisible()) {
        if (UI_DMMessageModesMatch(iMode)) {
            UI_CloseDMConsole();
        } else {
            UI_OpenDMConsole(iMode);
        }
    } else {
        UI_OpenDMConsole(iMode);
    }
}

/*
====================
UI_OpenDeveloperConsole
====================
*/
void UI_OpenDeveloperConsole(void)
{
    if (!developer_console) {
        return;
    }

    developer_console->setShow(true);
    uWinMan.ActivateControl(developer_console);
    IN_MouseOn();
}

/*
====================
UI_CloseDeveloperConsole
====================
*/
void UI_CloseDeveloperConsole(void)
{
    if (developer_console && developer_console->getShow()) {
        developer_console->setShow(false);
    }
    UI_FocusMenuIfExists();
}

/*
====================
UI_ToggleDeveloperConsole_f
====================
*/
void UI_ToggleDeveloperConsole_f(void)
{
    if (!developer_console) {
        return;
    }

    if (developer_console->IsThisOrChildActive()) {
        UI_CloseDeveloperConsole();
    } else {
        UI_OpenDeveloperConsole();
    }
}

/*
====================
UI_KeyEvent
====================
*/
void UI_KeyEvent(int key, unsigned int time)
{
    uWinMan.KeyEvent(key, time);
}

/*
====================
UI_CharEvent
====================
*/
void UI_CharEvent(int ch)
{
    uWinMan.CharEvent(ch);
}

/*
====================
UI_ClearBackground
====================
*/
void UI_ClearBackground(void)
{
    re.Set2DWindow(0, 0, uid.vidWidth, uid.vidHeight, 0, uid.vidWidth, uid.vidHeight, 0, -1, 1);
    re.Scissor(0, 0, uid.vidWidth, uid.vidHeight);
    re.SetColor(g_color_table[0]);
    re.DrawBox(0, 0, uid.vidWidth, uid.vidHeight);
    re.SetColor(NULL);
}

/*
====================
UI_ActivateView3D
====================
*/
void UI_ActivateView3D(void)
{
    if (!view3d) {
        return;
    }

    view3d->setShow(true);
    uWinMan.ActivateControl(view3d);
}

/*
====================
UI_DrawIntro
====================
*/
void UI_DrawIntro(void)
{
    if (clc.state == CA_CINEMATIC) {
        view3d->setShow(true);
        return;
    }

    view3d->setShow(false);
    UI_ClearBackground();

    if (cls.startStage < 3 || cls.startStage >= 12) {
        return;
    }

    if (intro_stage.material->GetMaterial()) {
        float  swidth;
        float  sheight;
        vec4_t color;

        VectorSet4(color, 1, 1, 1, 1);
        if (intro_stage.fadetime) {
            float frac;

            frac     = Q_clamp_float((cls.realtime - intro_stage.starttime) / intro_stage.fadetime, 0, 1);
            color[0] = color[1] = color[2] =
                intro_stage.alpha_start + frac * (intro_stage.alpha_end - intro_stage.alpha_start);
        }

        swidth  = view3d->getFrame().getMaxX();
        sheight = view3d->getFrame().getMaxY();

        re.SetColor(color);

        re.DrawStretchPic(0.0, 0.0, swidth, sheight, 0.0, 0.0, 1.0, 1.0, intro_stage.material->GetMaterial());
        re.SetColor(NULL);
    }

    if (cls.realtime >= intro_stage.endtime) {
        CL_FinishedStartStage();
    }
}

/*
====================
UI_MenuUp
====================
*/
qboolean UI_MenuUp()
{
    return menuManager.CurrentMenu() != NULL;
}

/*
====================
UI_FullscreenMenuUp
====================
*/
qboolean UI_FullscreenMenuUp()
{
    Menu *currentMenu = menuManager.CurrentMenu();

    if (currentMenu) {
        return currentMenu->isFullscreen();
    }

    return qfalse;
}

/*
====================
UI_Update

Updates the UI.
====================
*/
void UI_Update(void)
{
    Menu    *currentMenu;
    UIRect2D frame;

    re.SetRenderTime(cls.realtime);
    CL_FillUIDef();
    uWinMan.ServiceEvents();

    //
    // draw the base HUD when in-game
    //
    if (cls.no_menus && clc.state == CA_ACTIVE) {
        view3d->setShow(true);
        frame = uWinMan.getFrame();
        view3d->Display(frame, 1.0);

        if (ui_hud && !view3d->LetterboxActive()) {
            // draw the health hud
            if (hud_health) {
                hud_health->ForceShow();
                frame = uWinMan.getFrame();
                hud_health->GetContainerWidget()->Display(frame, 1.0);
            }

            // draw the ammo hud
            if (hud_ammo) {
                hud_ammo->ForceShow();
                frame = uWinMan.getFrame();
                hud_ammo->GetContainerWidget()->Display(frame, 1.0);
            }

            // draw the compass hud
            if (hud_compass) {
                hud_compass->ForceShow();
                frame = uWinMan.getFrame();
                hud_compass->GetContainerWidget()->Display(frame, 1.0);
            }
        }

        return;
    }

    if (fakk_console) {
        if (ui_minicon->integer) {
            // toggle the mini-console
            if (mini_console) {
                mini_console->setRealShow(fakk_console->getShow() ^ 1);
            }
        } else if (mini_console) {
            mini_console->setRealShow(false);
        }
    }

    if (gmbox) {
        gmbox->setRealShow(true);
    }

    if (dmbox) {
        dmbox->setRealShow(true);
    }

    currentMenu = menuManager.CurrentMenu();

    if (currentMenu == menuManager.FindMenu("main")) {
        UI_MainMenuWidgetsUpdate();
    } else if (currentMenu == menuManager.FindMenu("dm_main")) {
        UI_MultiplayerMainMenuWidgetsUpdate();
    }

    // don't care about the intro
    if (!CL_FinishedIntro()) {
        UI_DrawIntro();
    } else if (!server_loading && (clc.state == CA_CONNECTING || clc.state == CA_CHALLENGING) && ui_pConnectingMenu) {
        view3d->setShow(false);
        UI_ClearBackground();

        if (UI_BindActive()) {
            Menu *bindMenu = menuManager.FindMenu("controls");

            if (bindMenu) {
                Event *event = new Event(EV_UIFakkBindList_StopBind);
                bindMenu->PassEventToWidget("bindlist", event);
            } else {
                uWinMan.SetBindActive(NULL);
            }
        }

        if (clc.connectStartTime >= cls.realtime - 1000 * cl_connect_timeout->integer) {
            if (currentMenu != ui_pConnectingMenu) {
                UI_ForceMenuOff(true);
                UI_DeactiveFloatingWindows();
                UI_ForceMenu("connecting");
                uWinMan.showCursor(true);
            }
        } else {
            Com_Printf("\nConnect to server timed out\n");
            UI_ForceMenuOff(true);
            Cbuf_AddText("disconnect;pushmenu servertimeout");
        }
    } else {
        if (currentMenu && currentMenu->isFullscreen() && (!server_loading || !ui_pLoadingMenu)) {
            if (com_sv_running->integer && clc.state == CA_ACTIVE) {
                Com_FakePause();
            }

            view3d->setShow(false);
        } else if (!server_loading) {
            if (clc.state <= CA_PRIMED) {
                view3d->setShow(false);
                UI_ClearBackground();
            } else if (clc.state == CA_ACTIVE || clc.state == CA_CINEMATIC) {
                Com_FakeUnpause();
                view3d->setShow(true);
            } else {
                UI_ClearBackground();
                view3d->setShow(false);
            }
        } else {
            view3d->setShow(false);
            UI_ClearBackground();

            if (UI_BindActive()) {
                Menu *bindMenu = menuManager.FindMenu("controls");

                if (bindMenu) {
                    Event *event = new Event(EV_UIFakkBindList_StopBind);
                    bindMenu->PassEventToWidget("bindlist", event);
                }
            } else {
                uWinMan.SetBindActive(NULL);
            }

            if (ui_pLoadingMenu) {
                if (currentMenu != ui_pLoadingMenu) {
                    UI_ForceMenuOff(true);
                    UI_DeactiveFloatingWindows();
                    UI_ForceMenu(ui_sCurrentLoadingMenu);
                    uWinMan.showCursor(false);
                }

                if (!developer->integer && UI_ConsoleIsVisible()) {
                    UI_CloseConsole();
                }
            } else if (ui_static_materials.loading) {
                ui_static_materials.loading->GetMaterial();
            }
        }
    }

    // Hide the HUD when necessary
    if (!ui_hud || clc.state != CA_ACTIVE || view3d->LetterboxActive() || (currentMenu && currentMenu->isFullscreen())
        || server_loading || ((cl.snap.ps.pm_flags & PMF_NO_HUD) || (cl.snap.ps.pm_flags & PMF_INTERMISSION))) {
        if (crosshairhud) {
            crosshairhud->ForceHide();
        }
        if (hud_weapons) {
            hud_weapons->ForceHide();
            ui_weapHudTime = 0;
        }
        if (hud_items) {
            hud_items->ForceHide();
            ui_itemHudTime = 0;
        }
        if (hud_health) {
            hud_health->ForceHide();
        }
        if (hud_ammo) {
            hud_ammo->ForceHide();
        }
        if (hud_compass) {
            hud_compass->ForceHide();
        }
        if (hud_boss) {
            hud_boss->ForceHide();
        }

        UI_HideHudList();
    } else {
        if (crosshairhud) {
            if (ui_crosshair->integer && cl.snap.ps.stats[STAT_CROSSHAIR]) {
                crosshairhud->ForceShow();
            } else {
                crosshairhud->ForceHide();
            }
        }

        //
        // show and highlight all weapons that the player holds
        //

        //
        // try to show the weapons bar
        //
        if (hud_weapons) {
            if (ui_weaponsbar->integer && !(cl.snap.ps.pm_flags & PMF_NO_WEAPONBAR)) {
                int iEquippedDiff = 0;
                int iOwnedDiff    = 0;

                if (ui_weaponsbar->integer == 2) {
                    ui_weapHudTime = cls.realtime + ui_weaponsbartime->value;
                } else if (ui_weaponsbar->integer != 3 && ui_itemHudTime && hud_items->isVisible()) {
                    ui_weapHudTime = 0;
                }

                if (ui_lastWeapHudState_Owned != cl.snap.ps.stats[STAT_WEAPONS]
                    || ui_lastWeapHudState_Equipped != cl.snap.ps.stats[STAT_EQUIPPED_WEAPON]) {
                    iOwnedDiff    = cl.snap.ps.stats[STAT_WEAPONS] ^ ui_lastWeapHudState_Owned & 0x3F;
                    iEquippedDiff = (ui_lastWeapHudState_Equipped ^ cl.snap.ps.stats[STAT_EQUIPPED_WEAPON]) & 0x3F;

                    // if we have different equipment, reset the weapons hud time
                    if (iOwnedDiff || iEquippedDiff) {
                        ui_weapHudTime = cls.realtime + ui_weaponsbartime->integer;
                    }
                }

                //
                // show weapons that the player holds
                //
                if (iOwnedDiff) {
                    if (iOwnedDiff & 1) {
                        if (cl.snap.ps.stats[STAT_WEAPONS] & 1) {
                            Event *event = new Event(EV_Widget_Disable);
                            hud_weapons->PassEventToWidget("pistol_empty", event);
                        } else {
                            Event *event = new Event(EV_Widget_Enable);
                            hud_weapons->PassEventToWidget("pistol_empty", event);
                        }
                    }
                    if (iOwnedDiff & 2) {
                        if (cl.snap.ps.stats[STAT_WEAPONS] & 2) {
                            Event *event = new Event(EV_Widget_Disable);
                            hud_weapons->PassEventToWidget("rifle_empty", event);
                        } else {
                            Event *event = new Event(EV_Widget_Enable);
                            hud_weapons->PassEventToWidget("rifle_empty", event);
                        }
                    }
                    if (iOwnedDiff & 4) {
                        if (cl.snap.ps.stats[STAT_WEAPONS] & 4) {
                            Event *event = new Event(EV_Widget_Disable);
                            hud_weapons->PassEventToWidget("smg_empty", event);
                        } else {
                            Event *event = new Event(EV_Widget_Enable);
                            hud_weapons->PassEventToWidget("smg_empty", event);
                        }
                    }
                    if (iOwnedDiff & 8) {
                        if (cl.snap.ps.stats[STAT_WEAPONS] & 8) {
                            Event *event = new Event(EV_Widget_Disable);
                            hud_weapons->PassEventToWidget("mg_empty", event);
                        } else {
                            Event *event = new Event(EV_Widget_Enable);
                            hud_weapons->PassEventToWidget("mg_empty", event);
                        }
                    }
                    if (iOwnedDiff & 16) {
                        if (cl.snap.ps.stats[STAT_WEAPONS] & 16) {
                            Event *event = new Event(EV_Widget_Disable);
                            hud_weapons->PassEventToWidget("grenade_empty", event);
                        } else {
                            Event *event = new Event(EV_Widget_Enable);
                            hud_weapons->PassEventToWidget("grenade_empty", event);
                        }
                    }
                    if (iOwnedDiff & 32) {
                        if (cl.snap.ps.stats[STAT_WEAPONS] & 32) {
                            Event *event = new Event(EV_Widget_Disable);
                            hud_weapons->PassEventToWidget("heavy_empty", event);
                        } else {
                            Event *event = new Event(EV_Widget_Enable);
                            hud_weapons->PassEventToWidget("heavy_empty", event);
                        }
                    }
                    ui_lastWeapHudState_Owned =
                        cl.snap.ps.stats[STAT_WEAPONS] & 0x3F | ui_lastWeapHudState_Owned & ~0x3F;
                }
                //
                // highlight currently equipped weapons
                //
                if (iEquippedDiff) {
                    if (iEquippedDiff & 1) {
                        if (cl.snap.ps.stats[STAT_EQUIPPED_WEAPON] & 1) {
                            Event *event = new Event(EV_Widget_Enable);
                            hud_weapons->PassEventToWidget("pistol_equipped", event);
                        } else {
                            Event *event = new Event(EV_Widget_Disable);
                            hud_weapons->PassEventToWidget("pistol_equipped", event);
                        }
                    }
                    if (iEquippedDiff & 2) {
                        if (cl.snap.ps.stats[STAT_EQUIPPED_WEAPON] & 2) {
                            Event *event = new Event(EV_Widget_Enable);
                            hud_weapons->PassEventToWidget("rifle_equipped", event);
                        } else {
                            Event *event = new Event(EV_Widget_Disable);
                            hud_weapons->PassEventToWidget("rifle_equipped", event);
                        }
                    }
                    if (iEquippedDiff & 4) {
                        if (cl.snap.ps.stats[STAT_EQUIPPED_WEAPON] & 4) {
                            Event *event = new Event(EV_Widget_Enable);
                            hud_weapons->PassEventToWidget("smg_equipped", event);
                        } else {
                            Event *event = new Event(EV_Widget_Disable);
                            hud_weapons->PassEventToWidget("smg_equipped", event);
                        }
                    }
                    if (iEquippedDiff & 8) {
                        if (cl.snap.ps.stats[STAT_EQUIPPED_WEAPON] & 8) {
                            Event *event = new Event(EV_Widget_Enable);
                            hud_weapons->PassEventToWidget("mg_equipped", event);
                        } else {
                            Event *event = new Event(EV_Widget_Disable);
                            hud_weapons->PassEventToWidget("mg_equipped", event);
                        }
                    }
                    if (iEquippedDiff & 16) {
                        if (cl.snap.ps.stats[STAT_EQUIPPED_WEAPON] & 16) {
                            Event *event = new Event(EV_Widget_Enable);
                            hud_weapons->PassEventToWidget("grenade_equipped", event);
                        } else {
                            Event *event = new Event(EV_Widget_Disable);
                            hud_weapons->PassEventToWidget("grenade_equipped", event);
                        }
                    }
                    if (iEquippedDiff & 32) {
                        if (cl.snap.ps.stats[STAT_EQUIPPED_WEAPON] & 32) {
                            Event *event = new Event(EV_Widget_Enable);
                            hud_weapons->PassEventToWidget("heavy_equipped", event);
                        } else {
                            Event *event = new Event(EV_Widget_Disable);
                            hud_weapons->PassEventToWidget("heavy_equipped", event);
                        }
                    }
                    ui_lastWeapHudState_Equipped =
                        cl.snap.ps.stats[STAT_EQUIPPED_WEAPON] & 0x3F | ui_lastWeapHudState_Equipped & ~0x3F;
                }

                if (!ui_weapHudTime) {
                    if (hud_weapons->isVisible()) {
                        hud_weapons->ProcessEvent(EV_HideMenu);
                    }
                } else if (ui_weapHudTime < cls.realtime || ui_itemHudTime > ui_weapHudTime) {
                    ui_weapHudTime = 0;

                    if (hud_weapons->isVisible()) {
                        hud_weapons->ProcessEvent(EV_HideMenu);
                    }
                } else if (!hud_weapons->isVisible()) {
                    Event *event = new Event(EV_ShowMenu);
                    event->AddInteger(false);
                    hud_weapons->ProcessEvent(event);

                    if (hud_items->isVisible()) {
                        hud_items->ProcessEvent(EV_HideMenu);
                    }
                }
            } else {
                hud_weapons->ForceHide();
                ui_weapHudTime = 0;
            }
        }

        //
        // try to show the item bar
        //
        if (hud_items) {
            if (ui_weaponsbar->integer && ui_itemsbar->integer) {
                int iEquippedDiff = 0;
                int iOwnedDiff    = 0;

                if (ui_weaponsbar->integer == 3) {
                    ui_itemHudTime = cls.realtime + ui_weaponsbartime->integer;

                    if (ui_weapHudTime && hud_weapons->isVisible()) {
                        ui_itemHudTime = 0;
                    }
                }

                if (ui_lastWeapHudState_Owned != cl.snap.ps.stats[STAT_WEAPONS]
                    || ui_lastWeapHudState_Equipped != cl.snap.ps.stats[STAT_EQUIPPED_WEAPON]) {
                    iOwnedDiff    = cl.snap.ps.stats[STAT_WEAPONS] ^ ui_lastWeapHudState_Owned & 0xF00;
                    iEquippedDiff = (ui_lastWeapHudState_Equipped ^ cl.snap.ps.stats[STAT_EQUIPPED_WEAPON]) & 0xF00;

                    // if we have different equipment, reset the weapons hud time
                    if (iOwnedDiff || iEquippedDiff) {
                        ui_weapHudTime = cls.realtime + ui_weaponsbartime->integer;
                    }
                }

                //
                // show items that the player holds
                //
                if (iOwnedDiff) {
                    if (iOwnedDiff & 0x100) {
                        if (cl.snap.ps.stats[STAT_WEAPONS] & 1) {
                            Event *event = new Event(EV_Widget_Disable);
                            hud_weapons->PassEventToWidget("slot1_icon", event);
                        } else {
                            Event *event = new Event(EV_Widget_Enable);
                            hud_weapons->PassEventToWidget("slot1_icon", event);
                        }
                    }
                    if (iOwnedDiff & 0x200) {
                        if (cl.snap.ps.stats[STAT_WEAPONS] & 2) {
                            Event *event = new Event(EV_Widget_Disable);
                            hud_weapons->PassEventToWidget("slot2_icon", event);
                        } else {
                            Event *event = new Event(EV_Widget_Enable);
                            hud_weapons->PassEventToWidget("slot2_icon", event);
                        }
                    }
                    if (iOwnedDiff & 0x400) {
                        if (cl.snap.ps.stats[STAT_WEAPONS] & 4) {
                            Event *event = new Event(EV_Widget_Disable);
                            hud_weapons->PassEventToWidget("slot3_icon", event);
                        } else {
                            Event *event = new Event(EV_Widget_Enable);
                            hud_weapons->PassEventToWidget("slot3_icon", event);
                        }
                    }
                    if (iOwnedDiff & 0x800) {
                        if (cl.snap.ps.stats[STAT_WEAPONS] & 8) {
                            Event *event = new Event(EV_Widget_Disable);
                            hud_weapons->PassEventToWidget("slot4_icon", event);
                        } else {
                            Event *event = new Event(EV_Widget_Enable);
                            hud_weapons->PassEventToWidget("slot4_icon", event);
                        }
                    }
                    ui_lastWeapHudState_Owned =
                        cl.snap.ps.stats[STAT_WEAPONS] & 0xF00 | (ui_lastWeapHudState_Owned & 0xF0);
                }
                //
                // highlight currently equipped weapons
                //
                else if (iEquippedDiff) {
                    if (iEquippedDiff & 0x100) {
                        if (cl.snap.ps.stats[STAT_EQUIPPED_WEAPON] & 1) {
                            Event *event = new Event(EV_Widget_Disable);
                            hud_weapons->PassEventToWidget("slot1_highlight", event);
                        } else {
                            Event *event = new Event(EV_Widget_Enable);
                            hud_weapons->PassEventToWidget("slot1_highlight", event);
                        }
                    } else if (iEquippedDiff & 0x200) {
                        if (cl.snap.ps.stats[STAT_EQUIPPED_WEAPON] & 2) {
                            Event *event = new Event(EV_Widget_Disable);
                            hud_weapons->PassEventToWidget("slot2_highlight", event);
                        } else {
                            Event *event = new Event(EV_Widget_Enable);
                            hud_weapons->PassEventToWidget("slot2_highlight", event);
                        }
                    } else if (iEquippedDiff & 0x400) {
                        if (cl.snap.ps.stats[STAT_EQUIPPED_WEAPON] & 4) {
                            Event *event = new Event(EV_Widget_Disable);
                            hud_weapons->PassEventToWidget("slot3_highlight", event);
                        } else {
                            Event *event = new Event(EV_Widget_Enable);
                            hud_weapons->PassEventToWidget("slot3_highlight", event);
                        }
                    } else if (iEquippedDiff & 0x800) {
                        if (cl.snap.ps.stats[STAT_EQUIPPED_WEAPON] & 8) {
                            Event *event = new Event(EV_Widget_Disable);
                            hud_weapons->PassEventToWidget("slot4_highlight", event);
                        } else {
                            Event *event = new Event(EV_Widget_Enable);
                            hud_weapons->PassEventToWidget("slot4_highlight", event);
                        }
                    } else {
                        ui_lastWeapHudState_Equipped =
                            cl.snap.ps.stats[STAT_EQUIPPED_WEAPON] & 0xF00 | (ui_lastWeapHudState_Equipped & 0xF0);
                    }
                }

                if (!ui_itemHudTime) {
                    if (hud_items->isVisible()) {
                        hud_items->ProcessEvent(EV_HideMenu);
                    }
                } else if (ui_itemHudTime < cls.realtime || ui_weapHudTime > ui_itemHudTime) {
                    ui_itemHudTime = 0;

                    if (hud_items->isVisible()) {
                        hud_items->ProcessEvent(EV_HideMenu);
                    }
                } else if (!hud_items->isVisible()) {
                    Event *event = new Event(EV_ShowMenu);
                    event->AddInteger(false);
                    hud_items->ProcessEvent(event);

                    if (hud_weapons->isVisible()) {
                        hud_weapons->ProcessEvent(EV_HideMenu);
                    }
                }
            } else {
                hud_items->ForceHide();
                ui_itemHudTime = 0;
            }
        }

        if (hud_health) {
            hud_health->ForceShow();
        }

        //
        // show the ammo hud
        //
        str ammo = "hud_ammo_";
        ammo += CL_ConfigString(CS_WEAPONS + cl.snap.ps.activeItems[ITEM_WEAPON]);

        if (!hud_ammo || hud_ammo->m_name.icmp(ammo)) {
            Menu *ammoMenu = menuManager.FindMenu(ammo);
            if (ammoMenu) {
                if (ammoMenu != hud_ammo) {
                    if (hud_ammo) {
                        hud_ammo->ForceHide();
                    }
                    hud_ammo = ammoMenu;
                }
            } else {
                ammoMenu = menuManager.FindMenu("hud_ammo_");
                if (ammoMenu) {
                    if (ammoMenu != hud_ammo) {
                        if (hud_ammo) {
                            hud_ammo->ForceHide();
                        }
                        hud_ammo = ammoMenu;
                    }
                }
            }
        }

        if (hud_ammo) {
            hud_ammo->ForceShow();
        }

        //
        // show the compass
        //
        if (hud_compass) {
            if (ui_compass->integer) {
                hud_compass->ForceShow();
            } else {
                hud_compass->ForceHide();
            }
        }

        //
        // show the boss health
        //
        if (hud_boss) {
            if (cl.snap.ps.stats[STAT_BOSSHEALTH] > 0 && !hud_boss->isVisible()) {
                Event *event = new Event(EV_ShowMenu);
                event->AddInteger(false);
                hud_boss->ProcessEvent(event);
            } else if (hud_boss->isVisible()) {
                hud_boss->ProcessEvent(EV_HideMenu);
            }
        }

        UI_ShowHudList();
    }

    //
    // show the scoreboard
    //
    if (scoreboard_menu) {
        if (scoreboardlist && scoreboardlist->IsVisible()) {
            scoreboard_menu->ForceShow();
        } else {
            scoreboard_menu->ForceHide();
        }
    }

    uWinMan.UpdateViews();
}

/*
====================
UI_MultiplayerMenuWidgetsUpdate
====================
*/
void UI_MultiplayerMenuWidgetsUpdate(void)
{
    Event *event;

    // allow the map change widget when running in-game
    if (com_sv_running->integer) {
        event = new Event(EV_Widget_Disable);
        menuManager.PassEventToWidget("startnew", event);

        if (clc.state > CA_PRIMED && !cg_gametype->integer) {
            event = new Event(EV_Widget_Disable);
            menuManager.PassEventToWidget("changemap", event);
        } else {
            event = new Event(EV_Widget_Enable);
            menuManager.PassEventToWidget("changemap", event);
        }
    } else {
        event = new Event(EV_Widget_Disable);
        menuManager.PassEventToWidget("changemap", event);

        event = new Event(EV_Widget_Enable);
        menuManager.PassEventToWidget("startnew", event);
    }

    // allow the disconnect widget when in-game
    if (clc.state > CA_PRIMED) {
        event = new Event(EV_Widget_Enable);
        menuManager.PassEventToWidget("disconnect", event);
    } else {
        event = new Event(EV_Widget_Disable);
        menuManager.PassEventToWidget("disconnect", event);
    }

    // allow the join widget when in-game
    if (com_sv_running->integer || clc.state > CA_PRIMED) {
        event = new Event(EV_Widget_Disable);
        menuManager.PassEventToWidget("joinlan", event);

        event = new Event(EV_Widget_Disable);
        menuManager.PassEventToWidget("joininternet", event);
    } else {
        event = new Event(EV_Widget_Enable);
        menuManager.PassEventToWidget("joinlan", event);

        event = new Event(EV_Widget_Enable);
        menuManager.PassEventToWidget("joininternet", event);
    }
}

/*
====================
UI_MainMenuWidgetsUpdate
====================
*/
void UI_MainMenuWidgetsUpdate(void)
{
    if (clc.state > CA_PRIMED) {
        Event *event = new Event(EV_Widget_Enable);
        menuManager.PassEventToWidget("backtogame", event);
    } else {
        Event *event = new Event(EV_Widget_Disable);
        menuManager.PassEventToWidget("backtogame", event);
    }

    UI_UpdateContinueGame();
}

/*
====================
UI_MultiplayerMainMenuWidgetsUpdate
====================
*/
// Added in 2.0
void UI_MultiplayerMainMenuWidgetsUpdate(void)
{
    static const cvar_t *cg_allowvote = Cvar_Get("cg_allowvote", "1", 0);

    if (cg_allowvote->integer) {
        menuManager.PassEventToWidget("cantvote", new Event(EV_Widget_Disable));

        if (atoi(CL_ConfigString(CS_VOTE_TIME))) {
            menuManager.PassEventToWidget("callvotebutton", new Event(EV_Widget_Disable));

            if (cl.snap.ps.voted) {
                menuManager.PassEventToWidget("votebutton", new Event(EV_Widget_Disable));
                menuManager.PassEventToWidget("alreadyvoted", new Event(EV_Widget_Enable));
            } else {
                menuManager.PassEventToWidget("votebutton", new Event(EV_Widget_Enable));
                menuManager.PassEventToWidget("alreadyvoted", new Event(EV_Widget_Disable));
            }
        } else {
            menuManager.PassEventToWidget("callvotebutton", new Event(EV_Widget_Enable));
            menuManager.PassEventToWidget("votebutton", new Event(EV_Widget_Disable));
            menuManager.PassEventToWidget("alreadyvoted", new Event(EV_Widget_Disable));
        }
    } else {
        menuManager.PassEventToWidget("callvotebutton", new Event(EV_Widget_Disable));
        menuManager.PassEventToWidget("votebutton", new Event(EV_Widget_Disable));
        menuManager.PassEventToWidget("alreadyvoted", new Event(EV_Widget_Disable));
        menuManager.PassEventToWidget("cantvote", new Event(EV_Widget_Enable));
    }

    UI_UpdateContinueGame();
}

/*
====================
UI_ToggleMenu

Toggle a menu
====================
*/
void UI_ToggleMenu(str name)
{
    Menu *menu;

    menu = menuManager.CurrentMenu();

    if (menu) {
        UI_ForceMenuOff(false);

        if (menu->m_name != name) {
            UI_PushMenu(name);
        }
    } else {
        UI_PushMenu(name);
    }

    UI_FocusMenuIfExists();
}

/*
====================
UI_ToggleMenu_f
====================
*/
void UI_ToggleMenu_f()
{
    if (Cmd_Argc() <= 1) {
        Com_Printf("Usage: togglemenu <menuname>\n");
        return;
    }

    UI_ToggleMenu(Cmd_Argv(1));
}

/*
====================
UI_PopMenu
====================
*/
void UI_PopMenu(qboolean restore_cvars)
{
    Menu *menu;

    //
    //  Added in OPM
    //   Close any active floating windows (e.g. maplist)
    //
    UI_DeactiveFloatingWindows();

    if (uWinMan.DialogExists()) {
        uWinMan.RemoveAllDialogBoxes();
        return;
    }

    if (menuManager.CurrentMenu()) {
        menuManager.PopMenu(restore_cvars);
    }

    UI_FocusMenuIfExists();

    menu = menuManager.CurrentMenu();

    if (menu) {
        if (!str::cmp(menu->m_name, "main")) {
            UI_MainMenuWidgetsUpdate();
        } else if (!str::cmp(menu->m_name, "dm_main")) {
            UI_MultiplayerMainMenuWidgetsUpdate();
        } else if (!str::cmp(menu->m_name, "multiplayer")) {
            UI_MultiplayerMenuWidgetsUpdate();
        }
    }
}

/*
====================
UI_DeactiveFloatingWindows
====================
*/
void UI_DeactiveFloatingWindows(void)
{
    uWinMan.DeactiveFloatingWindows();
}

/*
====================
UI_PushMenu
====================
*/
void UI_PushMenu(const char *name)
{
    Menu    *menu  = menuManager.CurrentMenu();
    qboolean bDiff = qfalse;

    if (menu) {
        bDiff = strcmp(menu->m_name, name) != 0;
    }

    if (!bDiff || !ui_pLoadingMenu || menu != ui_pLoadingMenu) {
        menuManager.PushMenu(name);
    }

    UI_FocusMenuIfExists();

    if (!str::cmp(name, "main")) {
        UI_MainMenuWidgetsUpdate();
    } else if (!str::cmp(name, "dm_main")) {
        UI_MultiplayerMainMenuWidgetsUpdate();
    } else if (!str::cmp(name, "multiplayer")) {
        UI_MultiplayerMenuWidgetsUpdate();
    }
}

/*
====================
UI_ForceMenu
====================
*/
void UI_ForceMenu(const char *name)
{
    Menu    *menu  = menuManager.CurrentMenu();
    qboolean bDiff = qfalse;

    if (menu) {
        bDiff = strcmp(menu->m_name, name) != 0;
    }

    if (!menu || bDiff) {
        menuManager.ForceMenu(name);
    }

    UI_FocusMenuIfExists();
}

/*
====================
UI_ShowMenu
====================
*/
void UI_ShowMenu(const char *name, qboolean bForce)
{
    Menu *pMenu;

    pMenu = menuManager.FindMenu(name);

    if (pMenu) {
        if (bForce) {
            pMenu->ForceShow();
        } else if (!pMenu->isVisible()) {
            Event *event = new Event(EV_ShowMenu);
            event->AddInteger(false);
            pMenu->ProcessEvent(event);
        }
    }
}

/*
====================
UI_HideMenu
====================
*/
void UI_HideMenu(const char *name, qboolean bForce)
{
    Menu *pMenu;

    pMenu = menuManager.FindMenu(name);

    if (pMenu) {
        if (bForce) {
            pMenu->ForceHide();
        } else if (pMenu->isVisible()) {
            pMenu->ProcessEvent(EV_HideMenu);
        }
    }
}

/*
====================
UI_PushMenu_f
====================
*/
void UI_PushMenu_f(void)
{
    if (Cmd_Argc() <= 1) {
        Com_Printf("Usage: pushmenu <menuname>\n");
        return;
    }

    UI_PushMenu(Cmd_Argv(1));
}

/*
====================
UI_PushMenuSP_f
====================
*/
void UI_PushMenuSP_f(void)
{
    if (Cmd_Argc() <= 1) {
        Com_Printf("Usage: pushmenu_sp <menuname>\n");
        return;
    }

    if ((!com_cl_running || !com_cl_running->integer || clc.state == CA_DISCONNECTED || !cg_gametype->integer)
        && (!com_sv_running || !com_sv_running->integer || g_gametype->integer == GT_SINGLE_PLAYER)) {
        UI_PushMenu(Cmd_Argv(1));
    }
}

/*

====================
UI_PushMenuMP_f
====================
*/
void UI_PushMenuMP_f(void)
{
    const char *cmd;

    if (Cmd_Argc() <= 1) {
        Com_Printf("Usage: pushmenu_mp <menuname>\n");
        return;
    }

    if (com_cl_running && com_cl_running->integer && clc.state != CA_DISCONNECTED && cg_gametype->integer
        && com_sv_running && com_sv_running->integer && g_gametype->integer != GT_SINGLE_PLAYER) {
        cmd = Cmd_Argv(1);

        //
        // Push the right menu depending on the gametype
        //
        if (!Q_stricmp(cmd, "SelectTeam")) {
            switch (cg_gametype->integer) {
            case GT_FFA:
                UI_PushMenu("SelectFFAModel");
                break;
            case GT_OBJECTIVE:
                UI_PushMenu("ObjSelectTeam");
                break;
            default:
                UI_PushMenu("SelectTeam");
            }
        } else {
            UI_PushMenu(cmd);
        }
    }
}

/*
====================
UI_ForceMenu_f
====================
*/
void UI_ForceMenu_f(void)
{
    if (Cmd_Argc() <= 1) {
        Com_Printf("Usage: forcemenu <menuname>\n");
        return;
    }

    UI_ForceMenu(Cmd_Argv(1));
}

/*
====================
UI_PopMenu_f
====================
*/
void UI_PopMenu_f(void)
{
    if (Cmd_Argc() <= 1) {
        Com_Printf("Usage: popmenu <restore_cvars - set to 1 if you want all cvars restored to their original values>");
        return;
    }

    UI_PopMenu(atoi(Cmd_Argv(1)));
}

/*
====================
UI_ShowMenu_f
====================
*/
void UI_ShowMenu_f(void)
{
    qboolean    bForce;
    const char *name;

    if (Cmd_Argc() <= 1) {
        Com_Printf("Usage: showmenu <menuname>\n");
        return;
    }

    name = Cmd_Argv(1);

    if (Cmd_Argc() > 2) {
        bForce = atoi(Cmd_Argv(2));
    } else {
        bForce = qfalse;
    }

    UI_ShowMenu(name, bForce);
}

/*
====================
UI_HideMenu_f
====================
*/
void UI_HideMenu_f(void)
{
    qboolean    bForce;
    const char *name;

    if (Cmd_Argc() <= 1) {
        Com_Printf("Usage: hidemenu <menuname>\n");
        return;
    }

    name = Cmd_Argv(1);

    if (Cmd_Argc() > 2) {
        bForce = atoi(Cmd_Argv(2));
    } else {
        bForce = qfalse;
    }

    UI_HideMenu(name, bForce);
}

/*
====================
UI_WidgetCommand_f
====================
*/
void UI_WidgetCommand_f(void)
{
    str name;
    str commandstring;

    if (Cmd_Argc() <= 2) {
        Com_Printf("Usage: widgetcommand <name> <args1...argsN>\n");
        return;
    }

    name          = Cmd_Argv(1);
    commandstring = Cmd_Argv(2);
    Event *event  = new Event(commandstring);

    for (int i = 3; i < Cmd_Argc(); i++) {
        event->AddToken(Cmd_Argv(i));
    }

    menuManager.PassEventToWidget(name, event);
}

/*
====================
UI_GlobalWidgetCommand_f
====================
*/
void UI_GlobalWidgetCommand_f(void)
{
    str name;
    str commandstring;

    if (Cmd_Argc() <= 2) {
        Com_Printf("Usage: globalwidgetcommand <name> <args1...argsN>\n");
        return;
    }

    name          = Cmd_Argv(1);
    commandstring = Cmd_Argv(2);
    Event *event  = new Event(commandstring);

    for (int i = 3; i < Cmd_Argc(); i++) {
        event->AddToken(Cmd_Argv(i));
    }

    uWinMan.PassEventToWidget(name, event);
}

/*
====================
UI_ListMenus_f
====================
*/
void UI_ListMenus_f()
{
    menuManager.ListMenus();
}

/*
====================
UI_BindActive
====================
*/
qboolean UI_BindActive(void)
{
    return uWinMan.BindActive() != NULL;
}

/*
====================
UI_SetReturnMenuToCurrent
====================
*/
void UI_SetReturnMenuToCurrent(void)
{
    Menu *currentMenu = menuManager.CurrentMenu();

    if (currentMenu) {
        Cvar_Set("ui_returnmenu", currentMenu->m_name);
    } else {
        Cvar_Set("ui_returnmenu", "");
    }
}

/*
====================
UI_PushReturnMenu
====================
*/
qboolean UI_PushReturnMenu()
{
    str sMenuName;

    if (!ui_returnmenu->string || !*ui_returnmenu->string) {
        return qfalse;
    }

    sMenuName = ui_returnmenu->string;

    UI_PushMenu(sMenuName);
    Cvar_Set("ui_returnmenu", "");
    UI_FocusMenuIfExists();

    if (!menuManager.CurrentMenu()) {
        return qfalse;
    }

    if (!str::cmp(sMenuName, "main")) {
        UI_MainMenuWidgetsUpdate();
    } else if (!str::cmp(sMenuName, "dm_main")) {
        UI_MultiplayerMainMenuWidgetsUpdate();
    } else if (!str::cmp(sMenuName, "multiplayer")) {
        UI_MultiplayerMenuWidgetsUpdate();
    }

    return qtrue;
}

/*
====================
UI_PushReturnMenu_f
====================
*/
void UI_PushReturnMenu_f(void)
{
    UI_PushReturnMenu();
}

/*
====================
UI_MenuEscape
====================
*/
void UI_MenuEscape(const char *name)
{
    if (server_loading) {
        return;
    }

    if (uWinMan.BindActive()) {
        UI_KeyEvent(K_ESCAPE, qfalse);
        return;
    }

    if (menuManager.CurrentMenu() == mainmenu && clc.state == CA_DISCONNECTED) {
        if (developer->integer) {
            if (UI_ConsoleIsVisible()) {
                UI_CloseConsole();
            } else {
                UI_OpenConsole();
            }
        }
        return;
    }

    if (uWinMan.DialogExists()) {
        uWinMan.RemoveAllDialogBoxes();
        return;
    }

    if (menuManager.CurrentMenu()) {
        menuManager.PopMenu(qtrue);
    } else if (!Q_stricmp(name, "main") && clc.state > CA_PRIMED && cg_gametype->integer > 0) {
        // multiplayer
        UI_PushMenu("dm_main");
    } else {
        // single-player
        UI_PushMenu(name);
    }

    UI_FocusMenuIfExists();

    if (menuManager.CurrentMenu()) {
        if (!str::cmp(name, "main")) {
            UI_MainMenuWidgetsUpdate();
        } else if (!str::cmp(name, "dm_main")) {
            UI_MultiplayerMainMenuWidgetsUpdate();
        } else if (!str::cmp(name, "multiplayer")) {
            UI_MultiplayerMenuWidgetsUpdate();
        }
    }
}

/*
====================
UI_ForceMenuOff
====================
*/
void UI_ForceMenuOff(bool force)
{
    menuManager.ClearMenus(force);
    UI_FocusMenuIfExists();
}

/*
====================
UI_UpdateConnectionString
====================
*/
void UI_UpdateConnectionString(void) {}

/*
====================
UI_ParseServerInfoMessage
====================
*/
void UI_ParseServerInfoMessage(msg_t *msg) {}

/*
====================
UI_DrawConnectText
====================
*/
void UI_DrawConnectText(void) {}

/*
====================
UI_DrawConnect
====================
*/
void UI_DrawConnect(void)
{
    Com_Printf("UI_DrawConnect called\n");
}

/*
====================
CL_PingServers_f
====================
*/
void CL_PingServers_f(void)
{
    int         i;
    netadr_t    adr;
    char        name[32];
    const char *adrstring;
    cvar_t     *noudp;
    cvar_t     *noipx;

    Com_Printf("pinging broadcast...\n");

    noudp = Cvar_Get("noudp", "0", CVAR_INIT);
    if (!noudp->integer) {
        adr.type = NA_BROADCAST;
        adr.port = BigShort(12203);
        CL_NET_OutOfBandPrint(adr, "info %i", 8);
    }

    noipx = Cvar_Get("noipx", "0", CVAR_INIT);
    if (!noipx->integer) {
        adr.type = NA_BROADCAST_IPX;
        adr.port = BigShort(12203);
        CL_NET_OutOfBandPrint(adr, "info %i", 8);
    }

    for (i = 0; i < 16; i++) {
        Com_sprintf(name, sizeof(name), "adr%i", i);
        adrstring = Cvar_VariableString(name);

        if (adrstring && *adrstring) {
            Com_Printf("pinging %s...\n", adrstring);

            if (NET_StringToAdr(adrstring, &adr, NA_IP)) {
                if (!adr.port) {
                    adr.port = BigShort(12203);
                }

                CL_NET_OutOfBandPrint(adr, "info %i", 8);
            } else {
                Com_Printf("Bad address: %s\n", adrstring);
            }
        }
    }
}

/*
====================
UI_MapList_f
====================
*/
void UI_MapList_f(void)
{
    str mappath = "maps";

    if (Cmd_Argc() == 2) {
        mappath += "/";
        mappath += Cmd_Argv(1);
    }

    MapRunnerClass *map = new MapRunnerClass;
    map->Setup("maps", mappath, ".bsp", "_sml");

    CL_SetMousePos(uid.vidWidth / 2, uid.vidHeight / 2);
}

/*
====================
UI_DMMapSelect_f
====================
*/
void UI_DMMapSelect_f(void)
{
    str basepath = "maps";
    str mappath;
    str gametype;

    if (Cmd_Argc() > 1) {
        const char *path;

        path = Cmd_Argv(1);

        if (strcmp(path, ".")) {
            basepath += "/";
            basepath += path;
        }

        if (Cmd_Argc() > 2) {
            path = Cmd_Argv(2);
            if (strcmp(path, ".")) {
                mappath += "maps/";
                mappath += path;
            }
        }

        if (Cmd_Argc() > 3) {
            gametype = Cmd_Argv(3);
        }
    }

    MpMapPickerClass *map = new MpMapPickerClass;
    map->Setup(basepath, mappath, gametype);

    CL_SetMousePos(uid.vidWidth / 2, uid.vidHeight / 2);
}

/*
====================
UI_StartDMMap_f
====================
*/
void UI_StartDMMap_f(void)
{
    int         iDedicated;
    int         iMaxClients;
    int         iUseGameSpy;
    int         iGameType = GT_FFA;
    int         iFragLimit;
    int         iTimeLimit;
    int         iTeamDamage;
    int         iInactiveSpectate;
    int         iInactiveKick;
    const char *pszTemp;
    const char *pszGameTypeString = "Free-For-All";
    const char *pszMapName;
    const char *pszMapListCvar = "sv_maplist";
    const char *pszMapListString;
    char        szHostName[32];

    if (Cmd_Argc() > 1) {
        iGameType = atoi(Cmd_Argv(1));
        if (Cmd_Argc() > 2) {
            pszGameTypeString = Cmd_Argv(2);

            if (Cmd_Argc() > 3) {
                pszMapListCvar = Cmd_Argv(3);
            }
        }
    }

    pszMapName = CvarGetForUI("ui_dmmap", "");

    if (!*pszMapName) {
        return;
    }

    iDedicated = atoi(CvarGetForUI("ui_dedicated", "0"));

    if (iDedicated < 0) {
        iDedicated = 0;
    } else if (iDedicated > 1) {
        iDedicated = 1;
    }

    Cvar_SetValue("ui_dedicated", iDedicated);

    iMaxClients = atoi(CvarGetForUI("ui_maxclients", "0"));

    if (iMaxClients < 1) {
        iMaxClients = 1;
    } else if (iMaxClients > MAX_CLIENTS) {
        iMaxClients = MAX_CLIENTS;
    }

    Cvar_SetValue("ui_maxclients", iMaxClients);

    iUseGameSpy = atoi(CvarGetForUI("ui_gamespy", "0"));

    if (iUseGameSpy < 0) {
        iUseGameSpy = 0;
    } else if (iUseGameSpy > 1) {
        iUseGameSpy = 1;
    }

    Cvar_SetValue("ui_gamespy", iUseGameSpy);

    iFragLimit = atoi(CvarGetForUI("ui_fraglimit", "0"));

    if (iFragLimit < 0) {
        iFragLimit = 0;
    }

    Cvar_SetValue("ui_fraglimit", iFragLimit);

    iTimeLimit = atoi(CvarGetForUI("ui_timelimit", "0"));

    if (iTimeLimit < 0) {
        iTimeLimit = 0;
    }

    Cvar_SetValue("ui_timelimit", iTimeLimit);

    iTeamDamage = atoi(CvarGetForUI("ui_teamdamage", "0"));

    if (iTeamDamage < 0) {
        iTeamDamage = 0;
    } else if (iTeamDamage > 1) {
        iTeamDamage = 1;
    }

    Cvar_SetValue("ui_teamdamage", iTeamDamage);

    iInactiveSpectate = atoi(CvarGetForUI("ui_inactivespectate", "0"));

    if (iInactiveSpectate < 0) {
        iInactiveSpectate = 0;
    }

    Cvar_SetValue("ui_inactivespectate", iInactiveSpectate);

    iInactiveKick = atoi(CvarGetForUI("ui_inactivekick", "0"));

    if (iInactiveKick < 0) {
        iInactiveKick = 0;
    }

    Cvar_SetValue("ui_inactivekick", iInactiveKick);

    pszMapListString = CvarGetForUI(pszMapListCvar, "");

    pszTemp = CvarGetForUI("ui_hostname", "Nameless Battle");
    Q_strncpyz(szHostName, pszTemp, sizeof(szHostName));

    for (int i = 0; i < sizeof(szHostName); i++) {
        if (szHostName[i] == ';' || szHostName[i] == '"' || szHostName[i] == '\\') {
            szHostName[i] = '_';
        }
    }

    Cvar_Set("ui_hostname", szHostName);

    Cbuf_AddText(
        va("set dedicated %i;"
           "set sv_maxclients %i;"
           "set sv_gamespy %i;"
           "set g_gametype %i;"
           "set g_gametypestring \"%s\";"
           "set fraglimit %i;"
           "set timelimit %i;"
           "set g_teamdamage %i;"
           "set g_inactivespectate %i;"
           "set g_inactivekick %i;"
           "set sv_maplist \"%s\";"
           "set sv_hostname \"%s\";"
           "set cheats 0;"
           "wait;"
           "map \"%s\"\n",
           iDedicated,
           iMaxClients,
           iUseGameSpy,
           iGameType,
           pszGameTypeString,
           iFragLimit,
           iTimeLimit,
           iTeamDamage,
           iInactiveSpectate,
           iInactiveKick,
           pszMapListString,
           szHostName,
           pszMapName)
    );
}

/*
====================
UI_PlayerModel_f
====================
*/
void UI_PlayerModel_f(void)
{
    qboolean bGermanModel;
    str      modelpath = "models/player";

    bGermanModel = qfalse;

    if (Cmd_Argc() > 1) {
        bGermanModel = atoi(Cmd_Argv(1)) ? qtrue : qfalse;

        if (Cmd_Argc() == 3) {
            modelpath += "/";
            modelpath += Cmd_Argv(2);
        }
    }

    PlayerModelPickerClass *picker = new PlayerModelPickerClass;
    picker->Setup("models/player", modelpath, bGermanModel);

    CL_SetMousePos(uid.vidWidth / 2, uid.vidHeight / 2);
}

/*
====================
UI_ApplyPlayerModel_f
====================
*/
void UI_ApplyPlayerModel_f(void)
{
    const char *pszUIPlayerModel;
    char        donotshowssindeorfr[64];

    pszUIPlayerModel = CvarGetForUI("ui_dm_playermodel_set", "american_army");
    Cvar_Set("dm_playermodel", pszUIPlayerModel);

    pszUIPlayerModel = CvarGetForUI("ui_dm_playergermanmodel_set", "german_wehrmacht_soldier");
    if (!Q_stricmpn(pszUIPlayerModel, "german_waffen_", 14)) {
        Q_strncpyz(donotshowssindeorfr, "german_waffenss_", sizeof(donotshowssindeorfr));
        Q_strcat(donotshowssindeorfr, sizeof(donotshowssindeorfr), pszUIPlayerModel + 14);
        Cvar_Set("dm_playergermanmodel", donotshowssindeorfr);
    } else {
        Cvar_Set("dm_playergermanmodel", pszUIPlayerModel);
    }

    Cvar_Set("name", CvarGetForUI("ui_name", "UnnamedSoldier"));
}

/*
====================
UI_GetPlayerModel_f
====================
*/
void UI_GetPlayerModel_f(void)
{
    const char *pszUIPlayerModel;
    char        donotshowssindeorfr[64];

    //
    // Allies
    //
    pszUIPlayerModel = CvarGetForUI("dm_playermodel", "american_army");
    Cvar_Set("ui_dm_playermodel", PM_FilenameToDisplayname(pszUIPlayerModel));
    Cvar_Set("ui_dm_playermodel_set", pszUIPlayerModel);
    Cvar_Set("ui_disp_playermodel", va("models/player/%s.tik", pszUIPlayerModel));

    //
    // Axis
    //
    pszUIPlayerModel = CvarGetForUI("dm_playergermanmodel", "german_wehrmacht_soldier");
    Cvar_Set("ui_dm_playergermanmodel", PM_FilenameToDisplayname(pszUIPlayerModel));
    Cvar_Set("ui_dm_playergermanmodel_set", pszUIPlayerModel);

    if (!Q_stricmpn(pszUIPlayerModel, "german_waffen_", 14)) {
        Q_strncpyz(donotshowssindeorfr, "german_waffenss_", sizeof(donotshowssindeorfr));
        Q_strcat(donotshowssindeorfr, sizeof(donotshowssindeorfr), pszUIPlayerModel + 14);
        Cvar_Set("ui_disp_playergermanmodel", va("models/player/%s.tik", donotshowssindeorfr));
    } else {
        Cvar_Set("ui_disp_playergermanmodel", va("models/player/%s.tik", pszUIPlayerModel));
    }

    Cvar_Set("ui_name", CvarGetForUI("name", "UnnamedSoldier"));
}

/*
====================
UI_SoundPicker_f
====================
*/
void UI_SoundPicker_f(void)
{
    new SoundPickerClass;
}

/*
====================
UI_ViewSpawnList_f
====================
*/
void UI_ViewSpawnList_f(void)
{
    str modelpath = "models";

    ViewSpawnerClass *picker = new ViewSpawnerClass;
    picker->Setup("models", modelpath, ".tik");
}

/*
====================
UI_LODSpawnList_f
====================
*/
void UI_LODSpawnList_f(void)
{
    str modelpath = "models";

    LODSpawnerClass *picker = new LODSpawnerClass;
    picker->Setup("models", modelpath, ".tik");
}

/*
====================
UI_Notepad_f
====================
*/
void UI_Notepad_f(void)
{
    if (Cmd_Argc() > 1) {
        UI_LoadNotepadFile(Cmd_Argv(1));
    } else {
        UI_LoadNotepadFile(NULL);
    }
}

/*
====================
UI_EditScript_f
====================
*/
void UI_EditScript_f(void)
{
    const char *info;
    const char *mname;
    str         mapname;
    str         mappath;

    // editscript only works in-game
    if (clc.state != CA_ACTIVE) {
        Com_Printf("You need to load a map to edit its script\n");
        return;
    }

    mname   = Info_ValueForKey(&cl.gameState.stringData[cl.gameState.stringOffsets[CS_SERVERINFO]], "mapname");
    mapname = mname;

    // remove the spawnpoint name
    if (strchr(mname, '$')) {
        info                  = strchr(mname, '$');
        mapname[info - mname] = 0;
    }

    // make sure the map is loaded
    if (!strcmp(mapname, "")) {
        Com_Printf("No map loaded?\n");
        return;
    }

    mappath = "maps/" + mapname + ".scr";

    if (!UI_LoadNotepadFile(mappath)) {
        Com_Printf("Couldn't load/find script file for %s\n", mappath.c_str());
    }
}

/*
====================
UI_EditShader_f
====================
*/
void UI_EditShader_f(void)
{
    str shaderpath;

    if (Cmd_Argc() != 2) {
        Com_Printf("Usage: Editshader <file>\n");
        return;
    }

    shaderpath = "scripts/" + str(Cmd_Argv(1)) + ".shader";

    if (!UI_LoadNotepadFile(shaderpath)) {
        Com_Printf("Couldn't open shader named %s\n", shaderpath.c_str());
    }
}

/*
====================
UI_EditSpecificShader_f
====================
*/
void UI_EditSpecificShader_f(void) {}

/*
====================
UI_LoadMenu
====================
*/
void UI_LoadMenu(const char *name)
{
    char buffer[256];

    Com_sprintf(buffer, sizeof(buffer), "ui/%s", name);
    COM_StripExtension(buffer, buffer, sizeof(buffer));
    Q_strcat(buffer, 256, ".urc");

    new UILayout(buffer);
    uWinMan.CreateMenus();
    UI_FocusMenuIfExists();
}

/*
====================
UI_LoadMenu_f
====================
*/
void UI_LoadMenu_f(void)
{
    UI_LoadMenu(Cmd_Argv(1));
}

/*
====================
CvarGetForUI
====================
*/
const char *CvarGetForUI(const char *name, const char *defval)
{
    cvar_t *cvar = Cvar_FindVar(name);

    if (cvar) {
        if (cvar->latchedString && *cvar->latchedString) {
            return cvar->latchedString;
        } else {
            return cvar->string;
        }
    } else {
        return defval;
    }
}

/*
====================
ListFilesForUI
====================
*/
void ListFilesForUI(const char *filespec)
{
    int    nfiles;
    char **filenames;

    filenames = FS_ListFiles("", filespec, qfalse, &nfiles);

    for (int i = 0; i < nfiles; i++) {
        uie.AddFileToList(filenames[i]);
    }

    FS_FreeFileList(filenames);
}

/*
====================
IsKeyDown
====================
*/
int IsKeyDown(int key)
{
    return keys[key].down;
}

/*
====================
UI_WantsKeyboard
====================
*/
void UI_WantsKeyboard()
{
    Key_SetCatcher(Key_GetCatcher() | KEYCATCH_UI);
}

struct widgettrans_s {
    const char *src;
    const char *dst;
};

static widgettrans_s s_widgettrans[2] = {
    {"Label", "FakkLabel"},
    {NULL,    NULL       }
};

/*
====================
TranslateWidgetName
====================
*/
const char *TranslateWidgetName(const char *widget)
{
    widgettrans_s *trans = &s_widgettrans[0];

    for (int i = 0; trans->src != NULL; i++, trans++) {
        if (!strcmp(trans->src, widget)) {
            return trans->dst;
        }
    }

    return NULL;
}

/*
====================
UI_Cvar_Set
====================
*/
void UI_Cvar_Set(const char *var_name, const char *value)
{
    Cvar_Set2(var_name, value, qfalse);
}

/*
====================
CL_FillUIDef
====================
*/
void CL_FillUIDef(void)
{
    CL_GetMouseState(&uid.mouseX, &uid.mouseY, &uid.mouseFlags);
    uid.time       = cls.realtime;
    uid.vidHeight  = cls.glconfig.vidHeight;
    uid.vidWidth   = cls.glconfig.vidWidth;
    uid.uiHasMouse = in_guimouse != qfalse;
}

/*
====================
UI_RegisterSound
====================
*/
sfxHandle_t UI_RegisterSound(const char *sample, qboolean streamed)
{
    return S_RegisterSound(sample, streamed, qfalse);
}

/*
====================
UI_FadeSound
====================
*/
void UI_FadeSound(float fTime)
{
    S_FadeSound(fTime);
}

/*
====================
UI_StopAll
====================
*/
void UI_StopAll()
{
    S_StopAllSounds2(qtrue);
}

/*
====================
UI_StartLocalSound
====================
*/
void UI_StartLocalSound(const char *sound_name)
{
    S_StartLocalSound(sound_name, qtrue);
}

/*
====================
UI_StartLocalSoundDialog
====================
*/
void UI_StartLocalSoundDialog(const char *sound_name)
{
    S_StartLocalSoundChannel(sound_name, qtrue, CHAN_DIALOG);
}

/*
====================
CL_FillUIImports
====================
*/
void CL_FillUIImports(void)
{
    uii.Rend_DrawBox           = re.DrawBox;
    uii.Rend_DrawPicStretched  = re.DrawStretchPic;
    uii.Rend_DrawPicStretched2 = re.DrawStretchPic2;
    uii.Rend_DrawPicTiled      = re.DrawTilePic;
    uii.Rend_GetShaderHeight   = re.GetShaderHeight;
    uii.Rend_GetShaderWidth    = re.GetShaderWidth;
    uii.Rend_DrawString        = re.DrawString;
    uii.Rend_LoadFont          = re.LoadFont;
    uii.Rend_RegisterMaterial  = re.RegisterShaderNoMip;
    uii.Rend_RefreshMaterial   = re.RefreshShaderNoMip;
    uii.Rend_Scissor           = re.Scissor;
    uii.Rend_Set2D             = re.Set2DWindow;
    uii.Rend_SetColor          = re.SetColor;
    uii.Rend_ImageExists       = re.ImageExists;

    uii.Cmd_Stuff                   = Cbuf_AddText;
    uii.Cvar_GetString              = CvarGetForUI;
    uii.Cvar_Reset                  = Cvar_Reset;
    uii.Cvar_Find                   = Cvar_FindVar;
    uii.Cvar_Set                    = UI_Cvar_Set;
    uii.File_PickFile               = PickFile;
    uii.File_FreeFile               = FS_FreeFile;
    uii.File_ListFiles              = ListFilesForUI;
    uii.File_OpenFile               = FS_ReadFile;
    uii.File_WriteFile              = FS_WriteTextFile;
    uii.Snd_PlaySound               = UI_StartLocalSound;
    uii.Snd_PlaySoundDialog         = UI_StartLocalSoundDialog;
    uii.Snd_RegisterSound           = UI_RegisterSound;
    uii.Snd_FadeSound               = UI_FadeSound;
    uii.Snd_StopAllSound            = UI_StopAll;
    uii.Alias_Add                   = Alias_Add;
    uii.Alias_FindRandom            = Alias_FindRandom;
    uii.Sys_Error                   = Com_Error;
    uii.Sys_IsKeyDown               = IsKeyDown;
    uii.Sys_Milliseconds            = Sys_Milliseconds;
    uii.Sys_Printf                  = Com_Printf;
    uii.Sys_DPrintf                 = Com_DPrintf;
    uii.Sys_GetClipboard            = Sys_GetWholeClipboard;
    uii.Sys_SetClipboard            = Sys_SetClipboard;
    uii.Cmd_CompleteCommandByNumber = Cmd_CompleteCommandByNumber;
    uii.Cvar_CompleteCvarByNumber   = Cvar_CompleteVariableByNumber;
    uii.UI_WantsKeyboard            = UI_WantsKeyboard;
    uii.Client_TranslateWidgetName  = TranslateWidgetName;
    uii.Connect                     = CL_Connect;
    uii.Key_GetKeynameForCommand    = Key_GetKeynameForCommand;
    uii.Key_GetCommandForKey        = Key_GetBinding;
    uii.Key_SetBinding              = Key_SetBinding;
    uii.Key_GetKeysForCommand       = Key_GetKeysForCommand;
    uii.Key_KeynumToString          = Key_KeynumToBindString;

    uii.GetConfigstring   = CL_ConfigString;
    uii.UI_CloseDMConsole = UI_CloseDMConsole;

    uii.GetRefSequence    = CL_GetRefSequence;
    uii.IsRendererLoaded  = CL_IsRendererLoaded;
    uii.Rend_LoadRawImage = re.LoadRawImage;
    uii.Rend_FreeRawImage = re.FreeRawImage;
}

/*
====================
CL_BeginRegistration
====================
*/
void CL_BeginRegistration(void)
{
    // init console stuff
    re.BeginRegistration(&cls.glconfig);
    uWinMan.CleanupShadersFromList();
}

/*
====================
CL_EndRegistration
====================
*/
void CL_EndRegistration(void)
{
    int start, end;

    start = Sys_Milliseconds();

    S_EndRegistration();
    re.EndRegistration();

    if (!Sys_LowPhysicalMemory()) {
        Com_TouchMemory();
    }

    end = Sys_Milliseconds();

    Com_Printf("CL_EndRegistration: %5.2f seconds\n", (float)(start - end) / 1000.0);
}

/*
====================
UI_CreateDialog
====================
*/
void UI_CreateDialog(
    const char *title,
    char       *cvarname,
    const char *command,
    const char *cancelCommand,
    int         width,
    int         height,
    const char *shader,
    const char *okshader,
    const char *cancelshader
)
{
    UIDialog *dlg     = new UIDialog;
    UIRect2D  rect    = UIRect2D((uid.vidWidth - width) / 2, (uid.vidHeight - height) / 2, width, height);
    UColor    bgColor = UWindowColor;

    dlg->Create(NULL, rect, title, bgColor, UHudColor);
    dlg->LinkCvar(cvarname);
    dlg->SetOKCommand(command);
    dlg->SetCancelCommand(cancelCommand);
    dlg->SetLabelMaterial(uWinMan.RegisterShader(shader));
    dlg->SetOkMaterial(uWinMan.RegisterShader(okshader));

    if (cancelshader) {
        dlg->SetCancelMaterial(uWinMan.RegisterShader(cancelshader));
    }

    uWinMan.ActivateControl(dlg);
}

/*
====================
UI_ResolutionChange
====================
*/
void UI_ResolutionChange(void)
{
    UIRect2D frame;

    if (com_target_game->integer >= TG_MOHTA) {
        ui_compass_scale = Cvar_Get("ui_compass_scale", "0.75", CVAR_ARCHIVE | CVAR_LATCH);
    } else {
        // Older version doesn't have an adjustable compass, so assume 0.5 by default
        ui_compass_scale = Cvar_Get("ui_compass_scale", "0.55", CVAR_ARCHIVE | CVAR_LATCH);
    }

    CL_FillUIImports();
    CL_FillUIDef();

    // Added in OPM
    //  Scaling for high resolutions
    if (uid.vidWidth > maxWidthRes && uid.vidHeight > maxHeightRes) {
        const float vidRatio = (float)uid.vidWidth / (float)uid.vidHeight;

        uid.scaleRes[0] = (float)uid.vidWidth / (maxHeightRes * vidRatio);
        uid.scaleRes[1] = (float)uid.vidHeight / maxHeightRes;
        //uid.scaleRes[0] = (float)uid.vidWidth / maxWidthRes;
        //uid.scaleRes[1] = (float)uid.vidHeight / maxHeightRes;
        uid.bHighResScaling = qtrue;
    } else {
        uid.scaleRes[0]     = 1;
        uid.scaleRes[1]     = 1;
        uid.bHighResScaling = qfalse;
    }

    if (!uie.ResolutionChange) {
        return;
    }

    if (fakk_console) {
        frame = getDefaultConsoleRectangle();
        fakk_console->setFrame(frame);
    }

    if (dm_console) {
        if (dm_console->GetQuickMessageMode()) {
            frame = getQuickMessageDMConsoleRectangle();
        } else {
            frame = getDefaultDMConsoleRectangle();
        }

        dm_console->setFrame(frame);
    }

    if (developer_console) {
        frame = getDefaultConsoleRectangle();
        developer_console->setFrame(frame);
    }

    if (scoreboardlist) {
        delete scoreboardlist;
        scoreboardlist = NULL;
        UI_CreateScoreboard();
    }

    uie.ResolutionChange();
    menuManager.RealignMenus();

    if (view3d) {
        frame = UIRect2D(0, 0, uid.vidWidth, uid.vidHeight);
        view3d->setFrame(frame);
    }

    if (gmbox) {
        frame = getDefaultGMBoxRectangle();
        gmbox->setFrame(frame);
    }

    if (dmbox) {
        frame = getDefaultDMBoxRectangle();
        dmbox->setFrame(frame);
    }
}

/*
====================
UI_FinishLoadingScreen_f
====================
*/
void UI_FinishLoadingScreen_f(void)
{
    Com_Unpause();
    UI_ForceMenuOff(qtrue);

    ui_pLoadingMenu        = NULL;
    ui_sCurrentLoadingMenu = "";
    server_loading         = qfalse;
    server_loading_waiting = qfalse;

    UI_ActivateView3D();
}

/*
====================
S_ServerLoaded
====================
*/
void S_ServerLoaded(void)
{
    if (!svs.soundsNeedLoad) {
        return;
    }
    svs.soundsNeedLoad = qfalse;

    Com_DPrintf("Loading Previous Sound State.\n");
    S_StopAllSounds2(qfalse);

    S_TriggeredMusic_Stop();
    s_bSoundPaused = qtrue;
    S_ReLoad(&svs.soundSystem);

    if (svs.tm_filename[0]) {
        S_TriggeredMusic_SetupHandle(svs.tm_filename, svs.tm_loopcount, svs.tm_offset, 0);
    }
}

/*
====================
UI_ServerLoaded
====================
*/
void UI_ServerLoaded(void)
{
    Menu   *pCurrentMenu;
    cvar_t *pMaxClients;
    Event  *event;

    CL_UpdateSnapFlags();
    SV_ServerLoaded();
    S_FadeSound(0);
    S_ServerLoaded();

    if (!server_loading) {
        return;
    }

    UI_EndLoad();
    pMaxClients = Cvar_Get("sv_maxclients", "1", 0);

    if (com_sv_running->integer && pMaxClients->integer > 1) {
        SV_Heartbeat_f();
    }

    uWinMan.showCursor(true);

    if (!ui_pLoadingMenu) {
        UI_FinishLoadingScreen_f();
        return;
    }

    pCurrentMenu = menuManager.CurrentMenu();

    if (pCurrentMenu != ui_pLoadingMenu || !pCurrentMenu->GetNamedWidget("continuebutton") || !com_sv_running->integer
        || pMaxClients->integer > 1) {
        UI_FinishLoadingScreen_f();
        return;
    }

    if (ui_sCurrentLoadingMenu == "loading_default") {
        UI_FinishLoadingScreen_f();
        return;
    }

    server_loading_waiting = qtrue;

    event = new Event(EV_Widget_Enable);
    ui_pLoadingMenu->PassEventToWidget("continuebutton", event);

    event = new Event(EV_Widget_Disable);
    ui_pLoadingMenu->PassEventToWidget("loadingflasher", event);

    event = new Event(EV_Widget_Disable);
    ui_pLoadingMenu->PassEventToWidget("loadingbar", event);

    event = new Event(EV_Widget_Disable);
    ui_pLoadingMenu->PassEventToWidget("loadingbar_border", event);

    event = new Event(EV_Widget_Activate);
    ui_pLoadingMenu->PassEventToWidget("continuebutton", event);

    Com_FakePause();
}

/*
====================
UI_ClearCenterPrint
====================
*/
void UI_ClearCenterPrint(void)
{
    if (view3d) {
        view3d->ClearCenterPrint();
    }
}

/*
====================
UI_UpdateCenterPrint
====================
*/
void UI_UpdateCenterPrint(const char *s, float alpha)
{
    view3d->UpdateCenterPrint(s, alpha);
}

/*
====================
UI_UpdateLocationPrint
====================
*/
void UI_UpdateLocationPrint(int x, int y, const char *s, float alpha)
{
    view3d->UpdateLocationPrint(x, y, s, alpha);
}

/*
====================
UI_CenterPrint_f
====================
*/
void UI_CenterPrint_f(void)
{
    const char *s;
    float       alpha;

    if (Cmd_Argc() <= 1) {
        Com_Printf("Usage: centerprintf string [alpha]\n");
        return;
    }

    s = Cmd_Argv(1);

    if (Cmd_Argc() > 2) {
        alpha = atof(Cmd_Argv(2));
    } else {
        alpha = 1.0;
    }

    UI_UpdateCenterPrint(s, alpha);
}

/*
====================
UI_LocationPrint_f
====================
*/
void UI_LocationPrint_f(void)
{
    int         x, y;
    const char *s;
    float       alpha;

    if (Cmd_Argc() <= 3) {
        Com_Printf("Usage: locatinprint x y string [alpha]\n");
        return;
    }

    x = atoi(Cmd_Argv(1));
    y = atoi(Cmd_Argv(2));

    s = Cmd_Argv(3);

    if (Cmd_Argc() > 4) {
        alpha = atof(Cmd_Argv(4));
    } else {
        alpha = 1.0;
    }

    UI_UpdateLocationPrint(x, y, s, alpha);
}

/*
====================
UI_LoadInventory_f
====================
*/
void UI_LoadInventory_f(void)
{
    UI_CloseInventory();
    CL_LoadInventory(ui_inventoryfile->string, &client_inv);
}

/*
====================
UI_ClearState
====================
*/
void UI_ClearState(void)
{
    S_FadeSound(0.0);
    UI_ClearCenterPrint();

    if (gmbox) {
        gmbox->Clear();
    }

    if (dmbox) {
        dmbox->Clear();
    }

    // Added in OPM
    SCR_StopCinematic();
}

/*
====================
UI_CheckRestart
====================
*/
void UI_CheckRestart(void)
{
    if (ui_console->integer && !fakk_console) {
        fakk_console = getNewConsole();
    }

    if (!dm_console) {
        dm_console = getNewDMConsole();
    }

    if (!ui_console->integer && fakk_console) {
        fakk_console->SendSignal(W_Destroyed);
    }

    if (ui_minicon->integer) {
        if (!mini_console) {
            UISize2D size;
            UColor   back = UColor(0.0, 0.5, 1.0, 1.0);

            size.width  = 500;
            size.height = 100;

            mini_console = new FakkMiniconsole;
            mini_console->Create(size, UHudColor, back, 0.3f);
            mini_console->setAlwaysOnBottom(true);
            mini_console->setBorderStyle(border_none);
        }
    } else if (mini_console) {
        delete mini_console;
        mini_console = NULL;
    }

    if (ui_gmbox->integer) {
        if (!gmbox) {
            UIRect2D frame;

            gmbox = new UIGMBox;
            frame = getDefaultGMBoxRectangle();

            gmbox->Create(frame, UHudColor, UHudColor, 0);
            gmbox->setAlwaysOnBottom(true);
            gmbox->setBorderStyle(border_none);
        }
    } else if (gmbox) {
        delete gmbox;
        gmbox = NULL;
    }

    if (!dmbox) {
        UIRect2D frame;

        dmbox = new UIDMBox;
        frame = getDefaultDMBoxRectangle();

        dmbox->Create(frame, UHudColor, UHudColor, 0);
        dmbox->setAlwaysOnBottom(true);
        dmbox->setBorderStyle(border_outline);
    }

    menuManager.CheckRestart();
}

/*
====================
UI_ResetCvars
====================
*/
void UI_ResetCvars(void)
{
    menuManager.ResetCVars();
}

/*
====================
UI_ShowMouse_f
====================
*/
void UI_ShowMouse_f(void)
{
    if (view3d->IsActive()) {
        uWinMan.DeactivateCurrentControl();
    }
}

/*
====================
UI_HideMouse_f
====================
*/
void UI_HideMouse_f(void)
{
    if (!view3d->IsActive()) {
        uWinMan.ActivateControl(view3d);
        IN_MouseOff();
    }
}

class TestListItem : public UIListCtrlItem
{
    str strings[3];

public:
    virtual griditemtype_t getListItemType(int which) const;
    virtual str            getListItemString(int i) const;
    virtual int            getListItemValue(int which) const;
    virtual void           DrawListItem(int iColumn, const UIRect2D& drawRect, bool bSelected, UIFont *pFont);
    virtual qboolean       IsHeaderEntry(void) const;
};

/*
====================
UI_TestListCtrl_f
====================
*/
void UI_TestListCtrl_f(void)
{
    UIFloatingWindow *wnd = new UIFloatingWindow;
    UIListCtrl       *control;
    UIRect2D          frame = UIRect2D(20, 20, 400, 300);
    TestListItem     *i1, *i2, *i3;

    wnd->Create(NULL, frame, "Test list control", UWindowColor, UHudColor);

    control = new UIListCtrl;
    control->InitFrame(
        wnd->getChildSpace(), frame.pos.x, frame.pos.y, frame.size.width, frame.size.height, border_none, "verdana-12"
    );
    control->setBackgroundColor(UWhite, true);
    control->AddColumn("name", 0, 100, false, false);
    control->AddColumn("ping", 1, 64, false, false);
    control->AddColumn("IP", 2, 64, false, false);

    for (int i = 0; i < 100; i++) {
        i1 = new TestListItem;
        control->AddItem(i1);

        i2 = new TestListItem;
        control->AddItem(i2);

        i3 = new TestListItem;
        control->AddItem(i3);
    }
}

class StatsUpdater : public Listener
{
public:
    CLASS_PROTOTYPE(StatsUpdater);

    void UpdateStats(Event *ev);
};

Event EV_StatsUpdater_UpdateStats("updatestats", EV_DEFAULT, NULL, NULL, "Update the stats on the missionLog");

CLASS_DECLARATION(Listener, StatsUpdater, NULL) {
    {NULL, NULL}
};

static StatsUpdater statsUpdater;
static str          loadName;

void StatsUpdater::UpdateStats(Event *ev)
{
    cvar_t *pMaxClients = Cvar_Get("sv_maxclients", "1", 0);

    if (!com_sv_running->integer || pMaxClients->integer > 1) {
        return;
    }

    if (paused && !paused->integer) {
        CL_AddReliableCommand("stats", qfalse);
    }

    Event *event = new Event(EV_StatsUpdater_UpdateStats);
    PostEvent(event, 2.0);
}

static int  statsRequestTime;
static bool intermission_stats_up;
static bool isMissionLogVisible;

/*
====================
UI_ShowScoreboard_f
====================
*/
void UI_ShowScoreboard_f(const char *pszMenuName)
{
    if (pszMenuName) {
        if (scoreboard_menuname.length() && str::icmp(scoreboard_menuname, pszMenuName) && scoreboard_menu) {
            scoreboard_menu->ForceHide();
        }

        scoreboard_menuname = pszMenuName;
    }

    if (UI_MenuActive()) {
        if (scoreboard_menuname.length()) {
            scoreboard_menu = menuManager.FindMenu(scoreboard_menuname);

            if (scoreboard_menu) {
                scoreboard_menu->ForceHide();
            }
        }

        if (scoreboardlist && scoreboardlist->IsVisible()) {
            scoreboardlist->setShow(false);
        }
    } else {
        if (scoreboard_menuname.length()) {
            scoreboard_menu = menuManager.FindMenu(scoreboard_menuname);

            if (scoreboard_menu) {
                UIWidget *widget = scoreboard_menu->GetContainerWidget();
                if (widget) {
                    widget->BringToFrontPropogated();
                }
                scoreboard_menu->ForceShow();
            }
        }

        if (scoreboardlist && !scoreboardlist->IsVisible()) {
            scoreboardlist->setShow(true);
        }
    }
}

/*
====================
UI_HideScoreboard_f
====================
*/
void UI_HideScoreboard_f(void)
{
    if (scoreboardlist) {
        scoreboardlist->setShow(false);
    }

    if (scoreboard_menuname.length()) {
        // Fixed in 2.30 (scoreboard_menu check)
        if (scoreboard_menu) {
            scoreboard_menu->ForceHide();
        }
    }
}

class ScoreboardListItem : public UIListCtrlItem
{
    str      strings[8];
    qboolean bColorSet;
    UColor   backColor;
    UColor   textColor;
    qboolean bTitleItem;

public:
    ScoreboardListItem();

    void SetListItemStrings(
        const char *string1,
        const char *string2,
        const char *string3,
        const char *string4,
        const char *string5,
        const char *string6,
        const char *string7,
        const char *string8
    );
    virtual griditemtype_t getListItemType(int which) const;
    virtual str            getListItemString(int i) const;
    virtual int            getListItemValue(int which) const;
    virtual void           DrawListItem(int iColumn, const UIRect2D& drawRect, bool bSelected, UIFont *pFont);
    void                   SetColors(const UColor& newTextColor, const UColor& newBackColor);
    void                   ClearColors(void);
    void                   SetTitleItem(qboolean bSet);
    virtual qboolean       IsHeaderEntry(void) const;
};

ScoreboardListItem::ScoreboardListItem()
{
    bColorSet  = false;
    bTitleItem = false;
}

void ScoreboardListItem::DrawListItem(int iColumn, const UIRect2D& drawRect, bool bSelected, UIFont *pFont)
{
    DrawBox(drawRect, backColor, 1.0);
    pFont->setColor(textColor);
    pFont->Print(
        (drawRect.pos.x + 1) / uid.scaleRes[0],
        drawRect.pos.y / uid.scaleRes[1],
        Sys_LV_CL_ConvertString(getListItemString(iColumn)),
        -1,
        uid.scaleRes
    );

    if (bTitleItem) {
        UIRect2D lineRect;
        UColor   lineColor = backColor;

        lineRect = drawRect;
        lineRect.pos.y += lineRect.size.height - 2.0;
        lineRect.size.height = 2.0;

        DrawBox(lineRect, lineColor, 1.0);
    }
}

void ScoreboardListItem::SetColors(const UColor& newTextColor, const UColor& newBackColor)
{
    textColor = newTextColor;
    backColor = newBackColor;
    bColorSet = true;
}

void ScoreboardListItem::ClearColors(void)
{
    bColorSet = false;
}

/*
====================
UI_CreateScoreboard
====================
*/
void UI_CreateScoreboard(void)
{
    int         i;
    int         iColumnWidth;
    float       w, h, x, y;
    float       fColumnScale;
    float       fR, fG, fB, fA;
    float       fFontR, fFontG, fFontB, fFontA;
    const char *pszColumnName;

    if (!cge) {
        return;
    }

    scoreboardlist = new UIListCtrl;
    cge->CG_GetScoreBoardPosition(&x, &y, &w, &h);
    scoreboard_x = x;
    scoreboard_y = y;
    scoreboard_w = w;
    scoreboard_h = h;

    SCR_AdjustFrom640(&x, &y, &w, &h);
    fColumnScale = uid.vidWidth / 640.0;
    cge->CG_GetScoreBoardColor(&fR, &fG, &fB, &fA);
    cge->CG_GetScoreBoardFontColor(&fFontR, &fFontG, &fFontB, &fFontA);

    scoreboardlist->InitFrame(NULL, x, y, w, h, -1, "verdana-12");
    scoreboardlist->setBackgroundColor(UColor(fR, fG, fB, fA), true);
    scoreboardlist->setForegroundColor(UColor(fFontR, fFontG, fFontB, fFontA));

    for (i = 0; i < 8; i++) {
        pszColumnName = cge->CG_GetColumnName(i, &iColumnWidth);
        if (!pszColumnName) {
            break;
        }

        scoreboardlist->AddColumn(pszColumnName, i, iColumnWidth * fColumnScale + 0.5, false, false);
    }

    scoreboard_header = cge->CG_GetScoreBoardDrawHeader();
    scoreboardlist->setShow(false);
    scoreboardlist->AllowActivate(false);
    scoreboardlist->setAlwaysOnBottom(true);
    scoreboardlist->SetUseScrollBar(false);
    scoreboardlist->SetDrawHeader(scoreboard_header);
    scoreboardlist->setHeaderFont("facfont-20");
}

/*
====================
UI_SetScoreBoardItem
====================
*/
void UI_SetScoreBoardItem(
    int          iItemNumber,
    const char  *pszData1,
    const char  *pszData2,
    const char  *pszData3,
    const char  *pszData4,
    const char  *pszData5,
    const char  *pszData6,
    const char  *pszData7,
    const char  *pszData8,
    const vec4_t pTextColor,
    const vec4_t pBackColor,
    qboolean     bIsHeader
)
{
    int                 i;
    float               x, y, w, h;
    ScoreboardListItem *pItem;

    if (iItemNumber >= 64) {
        return;
    }

    if (scoreboardlist) {
        cge->CG_GetScoreBoardPosition(&x, &y, &w, &h);

        // Recreate the scoreboard if it has different rect
        if (scoreboard_x != x || scoreboard_y != y || scoreboard_w != w || scoreboard_h != h
            || scoreboard_header != cge->CG_GetScoreBoardDrawHeader()) {
            delete scoreboardlist;
            scoreboardlist = NULL;
            UI_CreateScoreboard();
        }
    } else {
        UI_CreateScoreboard();
    }

    if (scoreboardlist) {
        if (iItemNumber + 1 > scoreboardlist->getNumItems()) {
            pItem = new ScoreboardListItem;
            scoreboardlist->AddItem(pItem);
        } else {
            pItem = (ScoreboardListItem *)scoreboardlist->GetItem(iItemNumber + 1);
        }

        if (pItem) {
            if (!pszData1) {
                pszData1 = "";
            }
            if (!pszData2) {
                pszData2 = "";
            }
            if (!pszData3) {
                pszData3 = "";
            }
            if (!pszData4) {
                pszData4 = "";
            }
            if (!pszData5) {
                pszData5 = "";
            }
            if (!pszData6) {
                pszData6 = "";
            }
            if (!pszData7) {
                pszData7 = "";
            }
            if (!pszData8) {
                pszData8 = "";
            }

            pItem->SetListItemStrings(pszData1, pszData2, pszData3, pszData4, pszData5, pszData6, pszData7, pszData8);

            if (pTextColor && pBackColor) {
                UColor textColor, backColor;

                textColor = UColor(pTextColor[0], pTextColor[1], pTextColor[2], pTextColor[3]);
                backColor = UColor(pBackColor[0], pBackColor[1], pBackColor[2], pBackColor[3]);
                pItem->SetColors(textColor, backColor);
            } else {
                pItem->ClearColors();
            }

            for (i = 0; i < 8; i++) {
                pItem->getListItemString(i);
            }

            pItem->SetTitleItem(bIsHeader);
        }
    }
}

/*
====================
UI_DeleteScoreBoardItems
====================
*/
void UI_DeleteScoreBoardItems(int iMaxIndex)
{
    while (iMaxIndex + 1 <= scoreboardlist->getNumItems()) {
        scoreboardlist->DeleteItem(scoreboardlist->getNumItems());
    }
}

/*
====================
UI_Hud_f
====================
*/
void UI_Hud_f(void)
{
    qboolean hide;

    if (Cmd_Argc() != 2) {
        Com_Printf("Usage: ui_hud [1|0]");
        return;
    }

    ui_hud = atoi(Cmd_Argv(1));
    hide   = !ui_hud;

    if (hud_weapons && hide) {
        hud_weapons->ForceHide();
        ui_weapHudTime = 0;
    }

    if (hud_items && hide) {
        hud_items->ForceHide();
        ui_itemHudTime = 0;
    }

    if (hud_health) {
        if (hide) {
            hud_health->ForceHide();
        } else {
            hud_health->ForceShow();
        }
    }
    if (hud_ammo) {
        if (hide) {
            hud_ammo->ForceHide();
        } else {
            hud_ammo->ForceShow();
        }
    }
    if (hud_compass) {
        if (hide) {
            hud_compass->ForceHide();
        } else {
            hud_compass->ForceShow();
        }
    }

    if (hide) {
        UI_HideHudList();
    } else {
        UI_ShowHudList();
    }

    if (ui_hud) {
        Cvar_Set("cg_hud", "1");
    } else {
        Cvar_Set("cg_hud", "0");
    }
}

/*
====================
UI_ClearConsole_f
====================
*/
void UI_ClearConsole_f(void)
{
    if (fakk_console) {
        fakk_console->Clear();
    }

    if (dm_console) {
        dm_console->Clear();
    }

    if (developer_console) {
        developer_console->Clear();
    }
}

/*
====================
CL_FinishedIntro
====================
*/
qboolean CL_FinishedIntro(void)
{
    return cls.startStage == 0;
}

/*
====================
CL_FinishedStartStage
====================
*/
void CL_FinishedStartStage(void)
{
    int wait;

    switch (cls.startStage++) {
    case 1:
        Cbuf_ExecuteText(EXEC_NOW, "cinematic EAlogo.RoQ\n");
        break;
    case 2:
        intro_stage.alpha_start = 0.0;
        intro_stage.alpha_end   = 1.0;

        wait                  = ui_titlescreen_fadein->value * 1000.0;
        intro_stage.fadetime  = wait;
        intro_stage.starttime = cls.realtime;
        intro_stage.endtime   = cls.realtime + wait;

        intro_stage.material = uWinMan.RegisterShader("mohaa_title");
        break;
    case 3:
        intro_stage.alpha_start = 1.0;
        intro_stage.alpha_end   = 1.0;

        wait                  = ui_legalscreen_stay->value * 1000.0;
        intro_stage.fadetime  = 0.0;
        intro_stage.starttime = cls.realtime;
        intro_stage.endtime   = cls.realtime + wait;
        break;
    case 4:
        intro_stage.alpha_start = 1.0;
        intro_stage.alpha_end   = 0.0;

        wait                  = ui_titlescreen_fadeout->value * 1000.0;
        intro_stage.fadetime  = wait;
        intro_stage.starttime = cls.realtime;
        intro_stage.endtime   = cls.realtime + wait;
        break;
    case 5:
        intro_stage.alpha_start = 0.0;
        intro_stage.alpha_end   = 1.0;

        wait                  = ui_legalscreen_fadein->value * 1000.0;
        intro_stage.fadetime  = wait;
        intro_stage.starttime = cls.realtime;
        intro_stage.endtime   = cls.realtime + wait;

        intro_stage.material = uWinMan.RegisterShader("legal");
        break;
    case 6:
        intro_stage.alpha_start = 1.0;
        intro_stage.alpha_end   = 1.0;

        wait                  = ui_legalscreen_stay->value * 1000.0;
        intro_stage.fadetime  = 0.0;
        intro_stage.starttime = cls.realtime;
        intro_stage.endtime   = cls.realtime + wait;
        break;
    case 7:
        intro_stage.alpha_start = 1.0;
        intro_stage.alpha_end   = 0.0;

        wait                  = ui_legalscreen_fadeout->value * 1000.0;
        intro_stage.fadetime  = wait;
        intro_stage.starttime = cls.realtime;
        intro_stage.endtime   = cls.realtime + wait;
        break;
    case 8:
        if (com_target_game->integer >= target_game_e::TG_MOHTA) {
            cls.startStage = 0;
            return;
        }

        Cbuf_ExecuteText(EXEC_NOW, "cinematic 2015intro.RoQ\n");
        break;
    case 9:
        Cbuf_ExecuteText(EXEC_NOW, "cinematic intro.RoQ\n");
        break;
    case 10:
        cls.startStage = 0;
        break;
    default:
        break;
    }
}

/*
====================
UI_StartStageKeyEvent
====================
*/
void UI_StartStageKeyEvent(void)
{
    switch (cls.startStage) {
    case 2:
        if (ui_skip_eamovie->integer) {
            SCR_StopCinematic();
        }
        break;
    case 3:
    case 4:
    case 5:
        if (ui_skip_titlescreen->integer) {
            cls.startStage = 5;
            CL_FinishedStartStage();
        }
        break;
    case 6:
    case 7:
    case 8:
        if (ui_skip_legalscreen->integer) {
            cls.startStage = 8;
            CL_FinishedStartStage();
        }
        break;
    case 9:
    case 10:
        SCR_StopCinematic();
        break;
    default:
        break;
    }
}

/*
====================
UI_StartIntro_f
====================
*/
void UI_StartIntro_f(void)
{
    cls.startStage = 1;
    IN_MouseOff();
    CL_FinishedStartStage();
}

/*
====================
CL_TryStartIntro
====================
*/
void CL_TryStartIntro(void)
{
    if (developer->integer || !cl_playintro->integer) {
        UI_ToggleConsole();
    } else {
        // FIXME: no intro from now
        Cvar_Set(cl_playintro->name, "0");
        UI_StartIntro_f();
    }
}

/*
====================
SaveConsoleRectangle
====================
*/
static void SaveConsoleRectangle(void)
{
    bool     wasMinimized;
    UIRect2D r;

    if (!fakk_console) {
        return;
    }

    wasMinimized = fakk_console->IsMinimized();

    if (wasMinimized) {
        fakk_console->ProcessEvent(UIFloatingWindow::W_MinimizePressed);
    }

    if (!fakk_console->IsMinimized()) {
        r = fakk_console->getFrame();

        Cvar_Set(
            "ui_consoleposition", va("%d %d %d %d", (int)r.pos.x, (int)r.pos.y, (int)r.size.width, (int)r.size.height)
        );

        if (wasMinimized) {
            fakk_console->ProcessEvent(UIFloatingWindow::W_MinimizePressed);
        }
    }
}

/*
====================
LoadConsoleRectangle
====================
*/
static void LoadConsoleRectangle(void)
{
    bool wasMinimized;

    if (!fakk_console) {
        return;
    }

    wasMinimized = fakk_console->IsMinimized();

    if (wasMinimized) {
        fakk_console->ProcessEvent(UIFloatingWindow::W_MinimizePressed);
    }

    if (!fakk_console->IsMinimized() && wasMinimized) {
        fakk_console->ProcessEvent(UIFloatingWindow::W_MinimizePressed);
    }
}

/*
====================
UI_AddHud_f
====================
*/
void UI_AddHud_f(void)
{
    Menu *hud = menuManager.FindMenu(Cmd_Argv(1));

    if (hud) {
        if (!hudList.ObjectInList(hud)) {
            hudList.AddObject(hud);
        }

        hud->ShowMenu(NULL);
    } else {
        Com_DPrintf("Hud %s not found in menu system.\n", Cmd_Argv(1));
    }
}

/*
====================
UI_RemoveHud_f
====================
*/
void UI_RemoveHud_f(void)
{
    Menu *hud = menuManager.FindMenu(Cmd_Argv(1));

    if (hud && hudList.ObjectInList(hud)) {
        hud->ForceHide();
        hudList.RemoveObject(hud);
    }
}

/*
====================
UI_StartServer_f
====================
*/
void UI_StartServer_f(void)
{
    const char *map = Cmd_Argv(1);

    if (strlen(map)) {
        Cbuf_AddText(va("map %s\n", map));
    }
}

/*
====================
UI_ShowStatistics_f
====================
*/
void UI_ShowStatistics_f()
{
    cvar_t *pMaxClients = Cvar_Get("sv_maxclients", "1", 0);

    if (!com_sv_running->integer || pMaxClients->integer > 1) {
        return;
    }

    if (missionLog && !isMissionLogVisible) {
        isMissionLogVisible = true;
        missionLog->ShowMenu(NULL);
        IN_MouseOn();
    }

    if (statsRequestTime + 2000 <= cls.realtime) {
        if (paused && !paused->integer) {
            statsRequestTime = cls.realtime;
            CL_AddReliableCommand("stats", qfalse);
            Event *event = new Event(EV_StatsUpdater_UpdateStats);
            statsUpdater.PostEvent(event, 2.0);
        }
    }
}

/*
====================
UI_HideStatistics_f
====================
*/
void UI_HideStatistics_f()
{
    cvar_t *pMaxClients = Cvar_Get("sv_maxclients", "1", 0);

    if (!com_sv_running->integer || pMaxClients->integer > 1) {
        return;
    }

    if (missionLog) {
        isMissionLogVisible = false;
        missionLog->HideMenu(&EV_HideMenu);
        IN_MouseOff();
    }

    statsUpdater.CancelEventsOfType(EV_StatsUpdater_UpdateStats);
}

/*
====================
UI_SalesScreen_f
====================
*/
void UI_SalesScreen_f()
{
    if (random() > 0.5) {
        Cbuf_AddText("pushmenu sales_splash1\n");
    } else {
        Cbuf_AddText("pushmenu sales_splash2\n");
    }
}

/*
====================
CL_ShutdownUI
====================
*/
void CL_ShutdownUI(void)
{
    if (!cls.uiStarted) {
        return;
    }

    if (s_intermediateconsole) {
        delete s_intermediateconsole;
        s_intermediateconsole = NULL;
    }

    // remove all UI commands
    Cmd_RemoveCommand("pushmenu");
    Cmd_RemoveCommand("forcemenu");
    Cmd_RemoveCommand("popmenu");
    Cmd_RemoveCommand("showmenu");
    Cmd_RemoveCommand("widgetcommand");
    Cmd_RemoveCommand("listmenus");
    Cmd_RemoveCommand("togglemenu");
    Cmd_RemoveCommand("loadmenu");
    Cmd_RemoveCommand("maplist");
    Cmd_RemoveCommand("dmmapselect");
    Cmd_RemoveCommand("ui_startdmmap");
    Cmd_RemoveCommand("viewspawnlist");
    Cmd_RemoveCommand("lod_spawnlist");
    Cmd_RemoveCommand("soundpicker");
    Cmd_RemoveCommand("notepad");
    Cmd_RemoveCommand("editscript");
    Cmd_RemoveCommand("editshader");
    Cmd_RemoveCommand("inv_restart");
    Cmd_RemoveCommand("ui_showmouse");
    Cmd_RemoveCommand("ui_hidemouse");
    Cmd_RemoveCommand("ui_saveconsolepos");
    Cmd_RemoveCommand("ui_loadconsolepos");
    Cmd_RemoveCommand("ui_testlist");
    Cmd_RemoveCommand("clear");
    Cmd_RemoveCommand("ui_hud");
    Cmd_RemoveCommand("ui_resetcvars");
    Cmd_RemoveCommand("ui_checkrestart");
    Cmd_RemoveCommand("ui_addhud");
    Cmd_RemoveCommand("ui_removehud");
    Cmd_RemoveCommand("centerprint");
    Cmd_RemoveCommand("locationprint");
    Cmd_RemoveCommand("startserver");
    Cmd_RemoveCommand("finishloadingscreen");
    Cmd_RemoveCommand("playermodel");
    Cmd_RemoveCommand("ui_applyplayermodel");
    Cmd_RemoveCommand("ui_getplayermodel");
    Cmd_RemoveCommand("devcon");
    Cmd_RemoveCommand("+statistics");
    Cmd_RemoveCommand("-statistics");

    for (int i = 0; i < hudList.NumObjects(); i++) {
        hudList.RemoveObjectAt(i);
    }

    // Removed in 2.0
    //  Crosshair is now handled by the cgame module
    //crosshairhud       = menuManager.FindMenu("crosshair");
    hud_weapons        = menuManager.FindMenu("hud_weapons");
    hud_items          = menuManager.FindMenu("hud_items");
    hud_health         = menuManager.FindMenu("hud_health");
    hud_compass        = menuManager.FindMenu("hud_compass");
    hud_boss           = menuManager.FindMenu("hud_boss");
    ui_pConnectingMenu = menuManager.FindMenu("connecting");

    // delete base hud
    if (view3d) {
        delete view3d;
        view3d = NULL;
    }
    if (fakk_console) {
        delete fakk_console;
        fakk_console = NULL;
    }
    if (dm_console) {
        delete dm_console;
        dm_console = NULL;
    }
    if (developer_console) {
        delete developer_console;
        developer_console = NULL;
    }
    if (mini_console) {
        delete mini_console;
        mini_console = NULL;
    }

    // delete game hud
    if (crosshairhud) {
        crosshairhud = NULL;
    }
    if (hud_weapons) {
        hud_weapons = NULL;
    }
    if (hud_items) {
        hud_items = NULL;
    }
    if (hud_health) {
        hud_health = NULL;
    }
    if (hud_ammo) {
        hud_ammo = NULL;
    }
    if (hud_compass) {
        hud_compass = NULL;
    }
    if (hud_boss) {
        hud_boss = NULL;
    }

    // delete informations hud
    if (scoreboardlist) {
        delete scoreboardlist;
        scoreboardlist = NULL;
    }
    if (gmbox) {
        delete gmbox;
        gmbox = NULL;
    }
    if (dmbox) {
        delete dmbox;
        dmbox = NULL;
    }

    // clear inventory
    client_inv.Clear();

    // shutdown the UI
    uie.Shutdown();

    // delete all menus
    menuManager.DeleteAllMenus();

    cls.uiStarted = false;
}

/*
====================
CL_InitUI
====================
*/
#define UI_OLD_API_VERSION 4

void CL_InitializeUI(void)
{
    int    nfiles;
    char **filenames;
    int    i;

    // Register all variables
    ui_minicon         = Cvar_Get("ui_minicon", "0", 1);
    ui_gmbox           = Cvar_Get("ui_gmbox", "1", 1);
    ui_consoleposition = Cvar_Get("ui_consoleposition", "", 1);
    ui_console         = Cvar_Get("ui_console", "0", 1);
    ui_crosshair       = Cvar_Get("ui_crosshair", "1", 1);
    ui_weaponsbar      = Cvar_Get("ui_weaponsbar", "1", 1);
    ui_weaponsbartime  = Cvar_Get("ui_weaponsbartime", "2500", 1);
    ui_itemsbar        = Cvar_Get("ui_itemsbar", "0", 1);
    sound_overlay      = Cvar_Get("soundoverlay", "0", 0);
    ui_debugload       = Cvar_Get("ui_debugload", "0", 0);
    Cvar_Get("ui_signshader", "", 0);
    ui_compass             = Cvar_Get("ui_compass", "1", 0);
    ui_newvidmode          = Cvar_Get("ui_newvidmode", "-1", 0);
    ui_inventoryfile       = Cvar_Get("ui_inventoryfile", "global/inventory.txt", 0);
    ui_drawcoords          = Cvar_Get("ui_drawcoords", "0", 0);
    ui_health_start        = Cvar_Get("ui_health_start", "0", 0);
    ui_health_end          = Cvar_Get("ui_health_end", "0", 0);
    ui_GunneryEvaluation   = Cvar_Get("ui_GunneryEvaluation", "0", 0);
    ui_GroinShots          = Cvar_Get("ui_GroinShots", "0", 0);
    ui_RightArmShots       = Cvar_Get("ui_RightArmShots", "0", 0);
    ui_LeftArmShots        = Cvar_Get("ui_LeftArmShots", "0", 0);
    ui_RightLegShots       = Cvar_Get("ui_RightLegShots", "0", 0);
    ui_LeftLegShots        = Cvar_Get("ui_LeftLegShots", "0", 0);
    ui_TorsoShots          = Cvar_Get("ui_TorsoShots", "0", 0);
    ui_HeadShots           = Cvar_Get("ui_HeadShots", "0", 0);
    ui_NumEnemysKilled     = Cvar_Get("ui_NumEnemysKilled", "0", 0);
    ui_NumObjectsDestroyed = Cvar_Get("ui_NumObjectsDestroyed", "0", 0);
    ui_NumHitsTaken        = Cvar_Get("ui_NumHitsTaken", "0", 0);
    ui_PreferredWeapon     = Cvar_Get("ui_PreferredWeapon", "none", 0);
    ui_Accuracy            = Cvar_Get("ui_Accuracy", "0", 0);
    ui_NumObjectives       = Cvar_Get("ui_NumObjectives", "0", 0);
    ui_NumComplete         = Cvar_Get("ui_NumComplete", "0", 0);
    ui_NumHits             = Cvar_Get("ui_NumHits", "0", 0);
    ui_NumShotsFired       = Cvar_Get("ui_NumShotsFired", "0", 0);
    ui_gmboxspam           = Cvar_Get("ui_gmboxspam", "1", 0);
    ui_gotmedal            = Cvar_Get("ui_gotmedal", "0", 0);
    ui_success             = Cvar_Get("ui_success", "0", 0);
    ui_failed              = Cvar_Get("ui_failed", "0", 0);
    ui_returnmenu          = Cvar_Get("ui_returnmenu", "0", 0);
    ui_skip_eamovie        = Cvar_Get("ui_skip_eamovie", "1", 0);
    ui_skip_titlescreen    = Cvar_Get("ui_skip_titlescreen", "1", 0);
    ui_skip_legalscreen    = Cvar_Get("ui_skip_legalscreen", "1", 0);
    ui_titlescreen_fadein  = Cvar_Get("ui_titlescreen_fadein", "1", 0);
    ui_titlescreen_fadeout = Cvar_Get("ui_titlescreen_fadeout", "1", 0);
    ui_titlescreen_stay    = Cvar_Get("ui_titlescreen_stay", "3", 0);
    ui_legalscreen_fadein  = Cvar_Get("ui_legalscreen_fadein", "1", 0);
    ui_legalscreen_fadeout = Cvar_Get("ui_legalscreen_fadeout", "1", 0);
    ui_legalscreen_stay    = Cvar_Get("ui_legalscreen_stay", "3", 0);
    cl_greenfps            = Cvar_Get("cl_greenfps", "0", 1);
    cl_playintro           = Cvar_Get("cl_playintro", "1", 0);
    cl_movieaudio          = Cvar_Get("cl_movieaudio", "1", 0);
    Cvar_Get("ui_startmap", "", 1);
    Cvar_Get("dlg_badsave", "This save game is invalid", 0);

    Cvar_Set("cg_hud", ui_hud ? "1" : "0");

    CL_FillUIDef();
    CL_FillUIImports();
    UI_InitExports();
    uie.Init();

    UI_LoadInventory_f();
    UI_CreateScoreboard();

    // New since mohta
    // Version number
    Cvar_Set("game_version", va("v%s", com_target_shortversion->string));

    // Add all commands
    Cmd_AddCommand("pushmenu", UI_PushMenu_f);
    Cmd_AddCommand("pushmenu_sp", UI_PushMenuSP_f);
    Cmd_AddCommand("pushmenu_dm", UI_PushMenuMP_f);
    Cmd_AddCommand("forcemenu", UI_ForceMenu_f);
    Cmd_AddCommand("popmenu", UI_PopMenu_f);
    Cmd_AddCommand("showmenu", UI_ShowMenu_f);
    Cmd_AddCommand("hidemenu", UI_HideMenu_f);
    Cmd_AddCommand("widgetcommand", UI_WidgetCommand_f);
    Cmd_AddCommand("globalwidgetcommand", UI_GlobalWidgetCommand_f);
    Cmd_AddCommand("listmenus", UI_ListMenus_f);
    Cmd_AddCommand("togglemenu", UI_ToggleMenu_f);
    Cmd_AddCommand("loadmenu", UI_LoadMenu_f);
    Cmd_AddCommand("maplist", UI_MapList_f);
    Cmd_AddCommand("dmmapselect", UI_DMMapSelect_f);
    Cmd_AddCommand("ui_startdmmap", UI_StartDMMap_f);
    Cmd_AddCommand("viewspawnlist", UI_ViewSpawnList_f);
    Cmd_AddCommand("lod_spawnlist", UI_LODSpawnList_f);
    Cmd_AddCommand("soundpicker", UI_SoundPicker_f);
    Cmd_AddCommand("notepad", UI_Notepad_f);
    Cmd_AddCommand("editscript", UI_EditScript_f);
    Cmd_AddCommand("editshader", UI_EditShader_f);
    Cmd_AddCommand("editspecificshader", UI_EditSpecificShader_f);
    Cmd_AddCommand("inv_restart", UI_LoadInventory_f);
    Cmd_AddCommand("ui_showmouse", UI_ShowMouse_f);
    Cmd_AddCommand("ui_hidemouse", UI_HideMouse_f);
    Cmd_AddCommand("ui_saveconsolepos", SaveConsoleRectangle);
    Cmd_AddCommand("ui_loadconsolepos", LoadConsoleRectangle);
    Cmd_AddCommand("ui_testlist", UI_TestListCtrl_f);
    Cmd_AddCommand("clear", UI_ClearConsole_f);
    Cmd_AddCommand("ui_hud", UI_Hud_f);
    Cmd_AddCommand("ui_resetcvars", UI_ResetCvars);
    Cmd_AddCommand("ui_checkrestart", UI_CheckRestart);
    Cmd_AddCommand("centerprint", UI_CenterPrint_f);
    Cmd_AddCommand("locationprint", UI_LocationPrint_f);
    Cmd_AddCommand("ui_addhud", UI_AddHud_f);
    Cmd_AddCommand("ui_removehud", UI_RemoveHud_f);
    Cmd_AddCommand("startserver", UI_StartServer_f);
    Cmd_AddCommand("finishloadingscreen", UI_FinishLoadingScreen_f);
    Cmd_AddCommand("playermodel", UI_PlayerModel_f);
    Cmd_AddCommand("ui_applyplayermodel", UI_ApplyPlayerModel_f);
    Cmd_AddCommand("ui_getplayermodel", UI_GetPlayerModel_f);
    Cmd_AddCommand("+statistics", UI_ShowStatistics_f);
    Cmd_AddCommand("-statistics", UI_HideStatistics_f);
    Cmd_AddCommand("setreturnmenu", UI_SetReturnMenuToCurrent);
    Cmd_AddCommand("gotoreturnmenu", UI_PushReturnMenu_f);
    Cmd_AddCommand("salesscreen", UI_SalesScreen_f);
    Cmd_AddCommand("launchgamespy", UI_LaunchGameSpy_f);

    if (developer->integer) {
        UColor bgColor;

        Cmd_AddCommand("devcon", UI_ToggleDeveloperConsole_f);

        developer_console = new UIFloatingConsole;

        bgColor = UWindowColor;

        developer_console->Create(NULL, getDefaultConsoleRectangle(), "Developer Console", bgColor, UHudColor);
        developer_console->setConsoleHandler(ConsoleCommandHandler);
        developer_console->setConsoleBackground(UBlack, 0.800000011920929);
        developer_console->setShow(false);
        developer_console->Connect(&s_consolehider, UIFloatingWindow::W_ClosePressed, UIFloatingWindow::W_ClosePressed);
        developer_console->Connect(&s_consolehider, W_Destroyed, W_Destroyed);
    }

    // Create the 3D view
    view3d = new View3D;
    view3d->setAlwaysOnBottom(true);
    view3d->InitFrame(NULL, 0, 0, uid.vidWidth, uid.vidHeight, -1, "facfont-20");
    view3d->setName("view3d");
    view3d->InitSubtitle();

    memset(&intro_stage, 0, sizeof(intro_stage));

    // Register the loading material
    ui_static_materials.loading = uWinMan.RegisterShader("textures/menu/loading");

    if (ui_console->integer || developer->integer > 0) {
        UColor bgColor = UWindowColor;

        // Create the console
        fakk_console = getNewConsole();

        bgColor = UColor(0.0, 0.5, 1.0, 1.0);

        // Create the mini console
        mini_console = new FakkMiniconsole;
        mini_console->Create(UISize2D(500, 100), UWhite, bgColor, 0.2f);
        mini_console->setAlwaysOnBottom(true);
        mini_console->setBorderStyle(border_none);
    }

    // Create the dm console
    if (!dm_console) {
        dm_console = getNewDMConsole();
    }

    // Create the game message box
    if (ui_gmbox->integer && !gmbox) {
        gmbox = new UIGMBox;
        gmbox->Create(getDefaultGMBoxRectangle(), UHudColor, UHudColor, 0.0);
        gmbox->setAlwaysOnBottom(true);
        gmbox->setBorderStyle(border_none);
    }

    // Create the deathmatch message box
    if (!dmbox) {
        dmbox = new UIDMBox;
        dmbox->Create(getDefaultDMBoxRectangle(), UHudColor, UHudColor, 0.0);
        dmbox->setAlwaysOnBottom(true);
        dmbox->setBorderStyle(border_outline);
    }

    // Load all urc files
    filenames = FS_ListFiles("ui/", "urc", qfalse, &nfiles);

    for (i = 0; i < nfiles; i++) {
        char szFilename[MAX_QPATH];

        Com_sprintf(szFilename, sizeof(szFilename), "ui/%s", filenames[i]);
        new UILayout(szFilename);
    }

    FS_FreeFileList(filenames);

    uWinMan.CreateMenus();

    // find the crosshair
    // Removed in 2.0
    //  Crosshair is now handled by the cgame module
    //crosshairhud = menuManager.FindMenu("crosshair");
    // find weapons hud
    hud_weapons = menuManager.FindMenu("hud_weapons");
    // find items hud
    hud_items = menuManager.FindMenu("hud_items");
    // find the health hud
    hud_health = menuManager.FindMenu("hud_health");
    // find the compass hud
    hud_compass = menuManager.FindMenu("hud_compass");
    // find the boss health hud
    hud_boss = menuManager.FindMenu("hud_boss");
    // find the stats screen
    missionLog = menuManager.FindMenu("StatsScreen");
    // find the connection menu
    ui_pConnectingMenu = menuManager.FindMenu("connecting");

    if (crosshairhud) {
        crosshairhud->ShowMenu(NULL);
    }
    if (hud_health) {
        hud_health->ShowMenu(NULL);
    }
    if (hud_compass) {
        hud_compass->ShowMenu(NULL);
    }

    // find the main menu
    mainmenu = menuManager.FindMenu("main");

    IN_MouseOn();

    // realign menus
    menuManager.RealignMenus();

    // clear input
    CL_ClearButtons();

    cls.uiStarted = qtrue;

    if (!com_dedicated->integer) {
        CL_TryStartIntro();
    }
}

static char        **loadStrings;
static unsigned int *loadTimes;
static char         *loadStringsBuffer;

class LoadResourceInfo
{
public:
    class LoadResourceInfo *next;
    unsigned int            loadCount;
    char                    name[1];

public:
    LoadResourceInfo();
};

static LoadResourceInfo *loadHead;

/*
====================
UI_BeginLoadResource
====================
*/
void UI_BeginLoadResource(void)
{
    clock_t time = clock();

    startCountHigh = time >> 32;
    startCountLow  = time;
}

/*
====================
UI_EndLoadResource
====================
*/
void UI_EndLoadResource(void)
{
    clock_t time;

    time =
        clock() - (((clock_t)startCountHigh << 32) | startCountLow) + (((clock_t)loadCountHigh << 32) | loadCountLow);

    loadCountHigh = time >> 32;
    loadCountLow  = time;
    loadCount     = time >> 25;
}

/*
====================
UI_EndLoadResource
====================
*/
void UI_EndLoadResource(const char *name)
{
    LoadResourceInfo *newLoadHead;

    if (!loadCount) {
        return;
    }

    newLoadHead            = (LoadResourceInfo *)Z_Malloc(sizeof(LoadResourceInfo) + strlen(name));
    newLoadHead->next      = loadHead;
    newLoadHead->loadCount = loadCount;
    strcpy(newLoadHead->name, name);
    loadHead = newLoadHead;

    loadNumber++;
    loadCountLow  = 0;
    loadCountHigh = 0;
}

/*
====================
UI_IsResourceLoaded
====================
*/
qboolean UI_IsResourceLoaded(const char *name)
{
    switch (*name) {
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 104:
        return TIKI_FindTikiAnim(name + 1) != NULL;
    case 103:
        return SkeletorCacheFindFilename(name + 1, NULL);
    case 107:
        return S_IsSoundRegistered(name + 1);
    case 110:
        return uii.Rend_ImageExists(name + 1);
    default:
        return qfalse;
    }
}

/*
====================
UI_RegisterLoadResource
====================
*/
void UI_RegisterLoadResource(const char *name)
{
    int i, j;
    int low, high;

    low  = 0;
    high = loadNumber - 1;

    while (low <= high) {
        i = (low + high) / 2;
        j = strcmp(name, loadStrings[i]);

        if (j < 0) {
            high = i - 1;
        } else if (j > 0) {
            low = i + 1;
        } else {
            currentLoadTime += loadTimes[i];
            loadTimes[i] = 0;
            break;
        }
    }
}

/*
====================
UI_TestUpdateScreen
====================
*/
void UI_TestUpdateScreen(unsigned int timeout)
{
    unsigned int newTime = Sys_Milliseconds();
    unsigned int startRenderTime, endRenderTime;

    if (timeout > 0 && (newTime - lastTime) < (timeout + updateTime)) {
        return;
    }

    startRenderTime = Sys_Milliseconds();
    Sys_PumpMessageLoop();
    SCR_UpdateScreen();
    endRenderTime = Sys_Milliseconds();

    updateTime = Q_min(endRenderTime - startRenderTime, 1000);
    lastTime   = endRenderTime;
}

/*
====================
UI_ClearResource
====================
*/
void UI_ClearResource(void)
{
    if (cls.loading == SS_DEAD || cls.loading != SS_GAME) {
        return;
    }

    UI_BeginLoadResource();
    UI_TestUpdateScreen(0);
}

/*
====================
UI_LoadResource
====================
*/
void UI_LoadResource(const char *name)
{
    if (cls.loading == SS_DEAD) {
        return;
    }

    if (cls.loading == SS_GAME) {
        UI_EndLoadResource();
        UI_EndLoadResource(name);
        UI_BeginLoadResource();
    } else if (cls.loading == SS_LOADING2) {
        UI_RegisterLoadResource(name);
        Cvar_SetValue("loadingbar", (float)currentLoadTime / (float)totalLoadTime);
    }

    UI_TestUpdateScreen(33);
}

/*
====================
UI_FreeLoadStrings
====================
*/
void UI_FreeLoadStrings()
{
    if (loadStrings) {
        Z_Free(loadStrings);
        loadStrings = NULL;
    }
}

/*
====================
UI_DeleteLoadInfo
====================
*/
void UI_DeleteLoadInfo()
{
    LoadResourceInfo *ptr;
    LoadResourceInfo *nextPtr;

    for (ptr = loadHead; ptr != NULL; ptr = nextPtr) {
        nextPtr = ptr->next;
        Z_Free(ptr);
    }
}

/*
====================
UI_ParseLoadMapinfo
====================
*/
bool UI_ParseLoadMapinfo(Script *scr)
{
    int         size, index;
    int         allocLen;
    char       *pos;
    const char *token;
    char       *newPos;
    int         time;

    loadNumber = scr->GetInteger(false);
    size       = scr->GetInteger(false);
    scr->SkipWhiteSpace(true);

    allocLen          = size + sizeof(loadStrings[0]) * loadNumber + sizeof(loadTimes[0]) * loadNumber;
    loadStrings       = (char **)Z_Malloc(allocLen);
    loadTimes         = (unsigned int *)(loadStrings + loadNumber);
    loadStringsBuffer = (char *)(loadTimes + loadNumber);
    pos               = loadStringsBuffer;

    totalLoadTime = 0;
    index         = 0;

    do {
        token  = scr->GetToken(true);
        newPos = &pos[strlen(token) + 1];
        strcpy(pos, token);
        loadStrings[index] = pos;

        time = scr->GetInteger(true);

        if (UI_IsResourceLoaded(pos)) {
            loadTimes[index] = 0;
        } else {
            loadTimes[index] = time;
            totalLoadTime += time;
        }

        pos = newPos;
        index++;
    } while (index != loadNumber);

    assert(pos <= (char *)loadStrings + allocLen);

    if (newPos != loadStringsBuffer + size) {
        UI_FreeLoadStrings();
        return false;
    }

    return true;
}

/*
====================
UI_ArchiveLoadMapinfo
====================
*/
bool UI_ArchiveLoadMapinfo(const char *mapname)
{
    Script scr;
    int    version;
    byte  *tempbuf;
    int    length;
    bool   success;

    length = FS_ReadFile(loadName, (void **)&tempbuf);

    if (length <= 0) {
        return false;
    }

    scr.LoadFile(loadName, length, (const char *)tempbuf);
    FS_FreeFile(tempbuf);

    version = scr.GetInteger(false);
    scr.SkipWhiteSpace(true);

    if (version == 3) {
        success = UI_ParseLoadMapinfo(&scr);
    } else {
        Com_Printf("Expecting version %d map info file.  Map info is version %d.\n", 3, version);
        success = false;
    }

    scr.Close();
    return success;
}

/*
====================
UI_BeginLoad
====================
*/
void UI_BeginLoad(const char *pszMapName)
{
    str mapfile;

    if (cls.loading) {
        if (cls.loading == SS_GAME) {
            UI_DeleteLoadInfo();
        }

        UI_FreeLoadStrings();
        cls.loading = SS_DEAD;
    }

    server_loading         = qtrue;
    server_loading_waiting = qfalse;
    strcpy(server_mapname, pszMapName);

    if (str::icmp(ui_sCurrentLoadingMenu, server_mapname)) {
        ui_sCurrentLoadingMenu = server_mapname;
        ui_pLoadingMenu        = menuManager.FindMenu(ui_sCurrentLoadingMenu);
    }

    if (!ui_pLoadingMenu) {
        ui_sCurrentLoadingMenu = "loading_default";
        ui_pLoadingMenu        = menuManager.FindMenu(ui_sCurrentLoadingMenu);
    }

    UI_ForceMenuOff(true);
    UI_DeactiveFloatingWindows();

    if (ui_pLoadingMenu) {
        if (!developer->integer && UI_ConsoleIsVisible()) {
            UI_CloseConsole();
        }

        ui_pLoadingMenu->PassEventToWidget("continuebutton", new Event(EV_Widget_Disable));

        loadName = "maps/";
        loadName += pszMapName;
        mapfile = loadName;
        loadName += ".min";

        if (CL_UseLargeLightmap(pszMapName)) {
            mapfile += ".bsp";
        } else {
            // Added in 2.0
            mapfile += "_sml.bsp";
        }

        if (UI_ArchiveLoadMapinfo(mapfile)) {
            cls.loading = SS_LOADING2;
        } else {
            cls.loading = SS_GAME;
        }

        if (cls.loading == SS_LOADING2) {
            ui_pLoadingMenu->PassEventToWidget("loadingflasher", new Event(EV_Widget_Disable));
            ui_pLoadingMenu->PassEventToWidget("loadingbar", new Event(EV_Widget_Enable));
            ui_pLoadingMenu->PassEventToWidget("loadingbar_border", new Event(EV_Widget_Enable));

            currentLoadTime = 0;
            Cvar_SetValue("loadingbar", 0);

            L_ProcessPendingEvents();

            lastTime = Sys_Milliseconds();
            if (com_sv_running->integer) {
                SCR_UpdateScreen();
            }
        } else {
            ui_pLoadingMenu->PassEventToWidget("loadingflasher", new Event(EV_Widget_Enable));
            ui_pLoadingMenu->PassEventToWidget("loadingbar", new Event(EV_Widget_Disable));
            ui_pLoadingMenu->PassEventToWidget("loadingbar_border", new Event(EV_Widget_Disable));

            UI_BeginLoadResource();
            loadCountLow  = 0;
            loadCountHigh = 0;
            loadHead      = NULL;
            loadNumber    = 0;

            cls.loading = SS_LOADING;

            L_ProcessPendingEvents();
            lastTime = Sys_Milliseconds();

            if (com_sv_running->integer) {
                SCR_UpdateScreen();
            }
        }
    } else {
        cls.loading = SS_LOADING;

        // process all events
        L_ProcessPendingEvents();

        lastTime = Sys_Milliseconds();

        if (com_sv_running->integer) {
            SCR_UpdateScreen();
        }
    }
}

/*
====================
UI_CompareLoadResources
====================
*/
int UI_CompareLoadResources(const void *elem1, const void *elem2)
{
    LoadResourceInfo *e1 = *(LoadResourceInfo **)elem1;
    LoadResourceInfo *e2 = *(LoadResourceInfo **)elem2;

    return strcmp(e1->name, e2->name);
}

/*
====================
UI_EndLoad
====================
*/
void UI_EndLoad(void)
{
    LoadResourceInfo  *ptr;
    LoadResourceInfo **base;
    int                i;
    fileHandle_t       file;
    int                size;
    char               buf[1024];

    if (!cls.loading) {
        return;
    }

    UI_LoadResource("*end");

    if (cls.loading == SS_GAME) {
        base = (LoadResourceInfo **)Z_Malloc(loadNumber * sizeof(LoadResourceInfo *));

        i = 0;
        for (ptr = loadHead; ptr != NULL; ptr = ptr->next) {
            base[i] = ptr;
            i++;
        }

        qsort(base, loadNumber, sizeof(LoadResourceInfo *), UI_CompareLoadResources);

        size = 0;
        for (i = 0; i < loadNumber; i++) {
            ptr = base[i];
            size += strlen(ptr->name) + 1;
        }

        file = FS_FOpenFileWrite(loadName);
        Com_sprintf(buf, sizeof(buf), "%d\n%d %d\n", 3, loadNumber, size);

        FS_Write(buf, strlen(buf), file);

        for (i = 0; i < loadNumber; i++) {
            Com_sprintf(buf, sizeof(buf), "%s\n%d\n", base[i]->name, base[i]->loadCount);
            FS_Write(buf, strlen(buf), file);
        }

        FS_FCloseFile(file);
        Z_Free(base);
        UI_DeleteLoadInfo();
    } else if (cls.loading == SS_LOADING2) {
        if (ui_debugload->integer) {
            Com_Printf("Following resources not accounted for on load:\n");
        }

        for (i = 0; i < loadNumber; i++) {
            if (loadTimes[i]) {
                if (ui_debugload->integer) {
                    Com_Printf("%s\n", loadStrings[i]);
                }

                loadTimes[i] = 0;
            }
        }

        currentLoadTime = totalLoadTime;
    }

    UI_FreeLoadStrings();

    cls.loading = SS_DEAD;
}

/*
====================
UI_AbortLoad
====================
*/
void UI_AbortLoad(void)
{
    if (cls.loading) {
        if (cls.loading == SS_GAME) {
            UI_DeleteLoadInfo();
        }
        cls.loading = SS_DEAD;
    }
}

/*
====================
CL_FontStringWidth
====================
*/
int CL_FontStringWidth(fontheader_t *pFont, const char *pszString, int iMaxLen)
{
    return uie.FontStringWidth(pFont, pszString, iMaxLen);
}

griditemtype_t TestListItem::getListItemType(int which) const
{
    return TYPE_STRING;
}

str TestListItem::getListItemString(int i) const
{
    return strings[i];
}

int TestListItem::getListItemValue(int i) const
{
    return atoi(strings[i]);
}

void TestListItem::DrawListItem(int iColumn, const UIRect2D& drawRect, bool bSelected, UIFont *pFont) {}

qboolean TestListItem::IsHeaderEntry(void) const
{
    return qfalse;
}

void ScoreboardListItem::SetListItemStrings(
    const char *string1,
    const char *string2,
    const char *string3,
    const char *string4,
    const char *string5,
    const char *string6,
    const char *string7,
    const char *string8
)
{
    strings[0] = string1;
    strings[1] = string2;
    strings[2] = string3;
    strings[3] = string4;
    strings[4] = string5;
    strings[5] = string6;
    strings[6] = string7;
    strings[7] = string8;
}

griditemtype_t ScoreboardListItem::getListItemType(int i) const
{
    return bColorSet != 0 ? TYPE_OWNERDRAW : TYPE_STRING;
}

str ScoreboardListItem::getListItemString(int i) const
{
    return strings[i];
}

int ScoreboardListItem::getListItemValue(int i) const
{
    return atoi(strings[i]);
}

void ScoreboardListItem::SetTitleItem(qboolean bSet)
{
    bTitleItem = bSet;
}

qboolean ScoreboardListItem::IsHeaderEntry(void) const
{
    return bTitleItem;
}
