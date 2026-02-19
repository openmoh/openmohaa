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

// cl_uiserverlist.cpp -- UI Server list
// Modifications that were made in OPM:
//  1) Move most global variables inside UIFAKKServerList
//  2) Made a global RefreshStatus() function so the status is consistent
//     when there are multiple server list instances

#include "cl_ui.h"
#include "../gamespy/goaceng.h"
#include "../gamespy/sv_gamespy.h"
#include "../gamespy/common/gsPlatformSocket.h"

Event EV_FAKKServerList_Connect("connect", EV_DEFAULT, NULL, NULL, "Connect to the specified server");

Event EV_FAKKServerList_RefreshServerList("refreshserverlist", EV_DEFAULT, NULL, NULL, "Refresh the serverlist");

Event EV_FAKKServerList_RefreshLANServerList(
    "refreshlanserverlist", EV_DEFAULT, NULL, NULL, "Refresh the LAN serverlist"
);

Event EV_FAKKServerList_CancelRefresh("cancelrefresh", EV_DEFAULT, NULL, NULL, "Cancel serverlist Refresh");

Event EV_FAKKServerList_LANListing(
    "lanlisting",
    EV_DEFAULT,
    NULL,
    NULL,
    "Makes this server list to LAN stuff when there's a choice between Internet & LAN servers"
);

Event EV_FAKKServerList_UpdateServer("updateserver", EV_DEFAULT, NULL, NULL, "Update the selected server");

class FAKKServerListItem : public UIListCtrlItem
{
    UIFAKKServerList *m_parent; // Added in OPM
    str               m_strings[6];
    str               m_sVersion;
    bool              m_bDifferentVersion;
    bool              m_bFavorite;
    bool              m_bQueryDone;
    bool              m_bQueryFailed;
    int               m_iNumPlayers;

public:
    str          m_sIP;
    unsigned int m_uiRealIP;
    int          m_iPort;
    int          m_iGameSpyPort;

public:
    FAKKServerListItem(
        UIFAKKServerList *parent, str string1, str string2, str string3, str string4, str string5, str string6, str ver
    );

    griditemtype_t getListItemType(int index) const override;
    int            getListItemValue(int i) const override;
    str            getListItemString(int i) const override;
    void           setListItemString(int i, str sNewString);
    void           DrawListItem(int iColumn, const UIRect2D& drawRect, bool bSelected, UIFont *pFont) override;
    qboolean       IsHeaderEntry() const override;
    bool           IsQueried() const;
    void           SetQueried(bool bIsQueried);
    bool           IfQueryFailed() const;
    void           SetQueryFailed(bool bFailed);
    void           SetNumPlayers(int iNum);
    bool           IsFavorite() const;
    void           SetFavorite(bool bIsFavorite);
    str            GetListItemVersion() const;
    void           SetListItemVersion(str sNewVer);
    void           SetDifferentVersion(bool bIsDifferentVersion);
    bool           IsDifferentVersion() const;
    int            GetNumPlayers() const;
};

//static int         g_iTotalNumPlayers;
qboolean g_bNumericSort = qfalse;
qboolean g_bReverseSort = qfalse;
//qboolean           g_NeedAdditionalLANSearch = qfalse;
//qboolean           g_bDoneUpdating[2];
//ServerListInstance g_ServerListInst[2];

// Fixed in OPM
//  It was a static vaariable inside UpdateServerListCallBack
//  that was set to 0 when the mode changed. This caused some issues

static void AddFilter(char *filter, const char *value);
static void AddFilter(char *filter, const char *value, size_t maxsize);

FAKKServerListItem::FAKKServerListItem(
    UIFAKKServerList *parent, str string1, str string2, str string3, str string4, str string5, str string6, str ver
)
{
    m_parent     = parent;
    m_strings[0] = string1;
    m_strings[1] = string2;
    m_strings[2] = string3;
    m_strings[3] = string4;
    m_strings[4] = string5;
    m_strings[5] = string6;
    m_sVersion   = ver;

    m_bFavorite    = false;
    m_bQueryDone   = false;
    m_bQueryFailed = false;
    m_iNumPlayers  = 0;
}

griditemtype_t FAKKServerListItem::getListItemType(int index) const
{
    return griditemtype_t::TYPE_OWNERDRAW;
}

int FAKKServerListItem::getListItemValue(int i) const
{
    return atoi(m_strings[i]);
}

str FAKKServerListItem::getListItemString(int i) const
{
    return m_strings[i];
}

void FAKKServerListItem::setListItemString(int i, str sNewString)
{
    m_strings[i] = sNewString;
}

void FAKKServerListItem::DrawListItem(int iColumn, const UIRect2D& drawRect, bool bSelected, UIFont *pFont)
{
    static cvar_t *pColoringType = Cvar_Get("cl_browserdetailedcolors", "0", CVAR_ARCHIVE);
    UIRect2D       newRect       = drawRect;
    vec2_t         virtualScale;

    virtualScale[0] = m_parent->getHighResScale()[0];
    virtualScale[1] = m_parent->getHighResScale()[1];

    if (!pColoringType->integer) {
        if (IfQueryFailed() || (IsDifferentVersion() && IsQueried())) {
            if (bSelected) {
                DrawBox(newRect, UColor(0.2f, 0.0f, 0.0f), 1.0);
                pFont->setColor(UColor(0.9f, 0.0f, 0.0f));
            } else {
                DrawBox(newRect, UColor(0.1f, 0.0f, 0.0f), 1.0);
                pFont->setColor(UColor(0.55f, 0.0f, 0.0f));
            }
        } else if (IsQueried()) {
            if (bSelected) {
                DrawBox(newRect, UColor(0.2f, 0.18f, 0.015f), 1.0);
                pFont->setColor(UColor(0.9f, 0.8f, 0.6f));
            } else {
                DrawBox(newRect, UColor(0.02f, 0.07f, 0.004f), 1.0);
                pFont->setColor(UHudColor);
            }
        } else {
            if (bSelected) {
                DrawBox(newRect, UColor(0.15f, 0.18f, 0.18f), 1.0);
                pFont->setColor(UColor(0.6f, 0.7f, 0.8f));
            } else {
                DrawBox(newRect, UColor(0.005f, 0.07f, 0.02f), 1.0);
                pFont->setColor(UColor(0.05f, 0.5f, 0.6f));
            }
        }

        pFont->Print(
            newRect.pos.x / virtualScale[0] + 1.0,
            newRect.pos.y / virtualScale[1],
            getListItemString(iColumn).c_str(),
            -1,
            virtualScale
        );
    } else {
        if (IfQueryFailed() || (IsDifferentVersion() && IsQueried())) {
            if (bSelected) {
                DrawBox(newRect, UColor(0.2f, 0.0f, 0.0f), 1.0);
                pFont->setColor(UColor(0.9f, 0.0f, 0.0f));
            } else {
                DrawBox(newRect, UColor(0.1f, 0.0f, 0.0f), 1.0);
                pFont->setColor(UColor(0.55f, 0.0f, 0.0f));
            }
        } else if (IsQueried()) {
            if (bSelected) {
                DrawBox(newRect, UColor(0.2f, 0.18f, 0.015f), 1.0);
                pFont->setColor(UColor(0.9f, 0.8f, 0.6f));
            } else {
                DrawBox(newRect, UColor(0.02f, 0.07f, 0.005f), 1.0);
                pFont->setColor(UHudColor);
            }
        } else {
            if (bSelected) {
                DrawBox(newRect, UColor(0.15f, 0.18f, 0.18f), 1.0);
                pFont->setColor(UColor(0.6f, 0.7f, 0.8f));
            } else {
                DrawBox(newRect, UColor(0.005f, 0.07f, 0.02f), 1.0);
                pFont->setColor(UColor(0.05f, 0.5f, 0.6f));
            }
        }

        pFont->Print(
            newRect.pos.x / virtualScale[0] + 1.0,
            newRect.pos.y / virtualScale[1],
            getListItemString(iColumn).c_str(),
            -1,
            virtualScale
        );
    }
}

qboolean FAKKServerListItem::IsHeaderEntry() const
{
    return m_bFavorite;
}

bool FAKKServerListItem::IsQueried() const
{
    return m_bQueryDone;
}

void FAKKServerListItem::SetQueried(bool bIsQueried)
{
    m_bQueryDone = bIsQueried;
    if (m_bQueryDone) {
        m_bQueryFailed = false;
    } else {
        SetNumPlayers(0);
    }
}

bool FAKKServerListItem::IfQueryFailed() const
{
    return m_bQueryFailed;
}

void FAKKServerListItem::SetQueryFailed(bool bFailed)
{
    m_bQueryFailed = bFailed;
}

void FAKKServerListItem::SetNumPlayers(int iNum)
{
    // Removed in OPM
    //if (m_iNumPlayers) {
    //    g_iTotalNumPlayers -= m_iNumPlayers;
    //}
    //g_iTotalNumPlayers += iNum;
    //Cvar_Set("dm_playercount", va("%d", g_iTotalNumPlayers));

    m_iNumPlayers = iNum;
}

bool FAKKServerListItem::IsFavorite() const
{
    return m_bFavorite;
}

void FAKKServerListItem::SetFavorite(bool bIsFavorite)
{
    m_bFavorite = bIsFavorite;
}

str FAKKServerListItem::GetListItemVersion() const
{
    return m_sVersion;
}

void FAKKServerListItem::SetListItemVersion(str sNewVer)
{
    m_sVersion = sNewVer;
}

void FAKKServerListItem::SetDifferentVersion(bool bIsDifferentVersion)
{
    m_bDifferentVersion = bIsDifferentVersion;
}

bool FAKKServerListItem::IsDifferentVersion() const
{
    return m_bDifferentVersion;
}

int FAKKServerListItem::GetNumPlayers() const
{
    return m_iNumPlayers;
}

CLASS_DECLARATION(UIListCtrl, UIFAKKServerList, NULL) {
    {&EV_UIListBase_ItemSelected,             &UIFAKKServerList::SelectServer        },
    {&EV_UIListBase_ItemDoubleClicked,        &UIFAKKServerList::ConnectServer       },
    {&EV_FAKKServerList_RefreshServerList,    &UIFAKKServerList::RefreshServerList   },
    {&EV_FAKKServerList_RefreshLANServerList, &UIFAKKServerList::RefreshLANServerList},
    {&EV_FAKKServerList_CancelRefresh,        &UIFAKKServerList::CancelRefresh       },
    {&EV_FAKKServerList_Connect,              &UIFAKKServerList::ConnectServer       },
    {&EV_FAKKServerList_LANListing,           &UIFAKKServerList::MakeLANListing      },
    {&EV_FAKKServerList_UpdateServer,         &UIFAKKServerList::UpdateServer        },
    {NULL,                                    NULL                                   }
};

UIFAKKServerList::UIFAKKServerList()
{
    setBackgroundColor(UWhite, true);
    Connect(this, EV_UIListBase_ItemDoubleClicked, EV_UIListBase_ItemDoubleClicked);
    Connect(this, EV_UIListBase_ItemSelected, EV_UIListBase_ItemSelected);

    AllowActivate(true);
    setHeaderFont("facfont-20");
    m_serverList[0]   = NULL;
    m_serverList[1]   = NULL;
    m_bHasList        = false;
    m_bGettingList[0] = false;
    m_bGettingList[1] = false;
    m_bUpdatingList   = false;
    m_bLANListing     = false;
    m_iLastSortColumn = 2;
}

void UIFAKKServerList::SelectServer(Event *ev) {}

void UIFAKKServerList::ConnectServer(Event *ev)
{
    const FAKKServerListItem *pItem;

    if (getCurrentItem() <= 0) {
        return;
    }

    pItem = static_cast<const FAKKServerListItem *>(GetItem(getCurrentItem()));
    if (pItem->IsDifferentVersion()) {
        const char *message;
        float       neededVersion = com_target_shortversion->value;
        float       serverVersion = atof(pItem->GetListItemVersion().c_str());

        // Tolerate patch version
        if (fabs(neededVersion - serverVersion) >= 0.1) {
            UI_SetReturnMenuToCurrent();

            message =
                va("Server is version %s, you are targeting %s",
                   pItem->GetListItemVersion().c_str(),
                   com_target_shortversion->string);
            Cvar_Set("com_errormessage", message);

            UI_PushMenu("wrongversion");
        } else {
            message =
                va("Can not connect to v%s server, you are targeting v%s",
                   pItem->GetListItemVersion().c_str(),
                   com_target_shortversion->string);

            Cvar_Set("dm_serverstatus", message);
        }
    } else {
        char cmdString[256];

        UI_SetReturnMenuToCurrent();

        Com_sprintf(cmdString, sizeof(cmdString), "connect %s\n", pItem->getListItemString(1).c_str());
        Cbuf_AddText(cmdString);
        ServerListHalt(m_serverList[0]);
    }
}

qboolean UIFAKKServerList::KeyEvent(int key, unsigned int time)
{
    switch (key) {
    case K_ENTER:
    case K_KP_ENTER:
        ConnectServer(NULL);
        return qtrue;
    case K_UPARROW:
        if (getCurrentItem() > 1) {
            TrySelectItem(getCurrentItem() - 1);
            SelectServer(NULL);

            return qtrue;
        } else {
            return qfalse;
        }
        break;
    case K_DOWNARROW:
        if (getCurrentItem() < getNumItems()) {
            TrySelectItem(getCurrentItem() + 1);
            SelectServer(NULL);

            return qtrue;
        } else {
            return qfalse;
        }
        break;
    case 'u':
    case 'U':
        UpdateServer(NULL);
        return UIListCtrl::KeyEvent(key, time);
    case 'c':
    case 'C':
        CancelRefresh(NULL);
        return qtrue;
    case 'i':
    case 'I':
        {
            int                       i, j;
            const FAKKServerListItem *pServerItem1;
            const FAKKServerListItem *pServerItem2;
            int                       iNumErrors = 0;

            for (i = 1; i <= getNumItems(); i++) {
                pServerItem1 = static_cast<const FAKKServerListItem *>(GetItem(i));

                for (j = i + 1; j <= getNumItems(); j++) {
                    pServerItem2 = static_cast<const FAKKServerListItem *>(GetItem(j));

                    if (!str::icmp(pServerItem1->m_sIP.c_str(), pServerItem2->m_sIP.c_str())) {
                        if (pServerItem1->m_iPort == pServerItem2->m_iPort) {
                            Com_DPrintf(
                                "*#*#* Duplicate server address: %s:%i\n",
                                pServerItem1->m_sIP.c_str(),
                                pServerItem1->m_iPort
                            );
                            iNumErrors++;
                        }

                        if (pServerItem1->m_iGameSpyPort == pServerItem2->m_iGameSpyPort) {
                            Com_DPrintf(
                                "*#*#* servers at IP %s sharing GameSpy port %i\n",
                                pServerItem1->m_sIP.c_str(),
                                pServerItem1->m_iGameSpyPort
                            );
                            iNumErrors++;
                        }
                    }
                }
            }

            Com_DPrintf("*#*#* %i problems detected\n", iNumErrors);
        }
        return qtrue;
    default:
        return UIListCtrl::KeyEvent(key, time);
    }
}

void UIFAKKServerList::UpdateUIElement(void)
{
    float width;

    RemoveAllColumns();

    width = getClientFrame().size.width;

    AddColumn("Server Name", 0, width * 0.31f, false, false);
    AddColumn("Map", 5, width * 0.135f, false, false);
    AddColumn("Players", 3, width * 0.085f, true, true);
    AddColumn("Gametype", 4, width * 0.18f, false, false);
    AddColumn("Ping", 2, width * 0.05f, true, false);
    AddColumn("IP", 1, width * 0.23f, false, false);

    RefreshStatus();
    uWinMan.ActivateControl(this);
}

void UIFAKKServerList::RefreshServerList(Event *ev)
{
    int                 i;
    FAKKServerListItem *pNewServerItem;

    if (m_serverList[0] && ServerListState(m_serverList[0]) != sl_idle) {
        // Fixed in OPM
        //  Only free the server list if it isn't currently being queried.
        //  The server list must not be queried
        return;
    }

    if (m_serverList[1] && ServerListState(m_serverList[1]) != sl_idle) {
        return;
    }

    for (i = 1; i <= getNumItems(); i++) {
        pNewServerItem = static_cast<FAKKServerListItem *>(GetItem(i));
        m_iTotalNumPlayers -= pNewServerItem->GetNumPlayers();
        pNewServerItem->SetQueried(false);
        pNewServerItem->SetNumPlayers(0);
        pNewServerItem->SetQueryFailed(false);
    }

    if (m_serverList[0]) {
        ServerListClear(m_serverList[0]);
        // Added in 2.0: Free the server list
        //  Since this version, the UI no longer refreshes the server list
        //  when clicking "Browse Internet Servers"
        ServerListFree(m_serverList[0]);
        m_serverList[0] = NULL;
    }

    if (m_serverList[1]) {
        ServerListClear(m_serverList[1]);
        ServerListFree(m_serverList[1]);
        m_serverList[1] = NULL;
    }

    if (!m_serverList[0] && (com_target_game->integer < target_game_e::TG_MOHTT || !m_serverList[1])) {
        NewServerList();
    }

    m_bDoneUpdating[0] = false;
    m_bDoneUpdating[1] = false;

    Cvar_Set("dm_playercount", "0");
    {
        char           filter[256]          = {0};
        static cvar_t *dm_max_players       = Cvar_Get("dm_max_players", "0", CVAR_ARCHIVE);
        static cvar_t *dm_min_players       = Cvar_Get("dm_min_players", "0", CVAR_ARCHIVE);
        static cvar_t *dm_show_demo_servers = Cvar_Get("dm_show_demo_servers", "1", CVAR_ARCHIVE);
        static cvar_t *dm_realism_mode      = Cvar_Get("dm_realism_mode", "0", CVAR_ARCHIVE);
        static cvar_t *dm_filter_listen     = Cvar_Get("dm_filter_listen", "1", CVAR_ARCHIVE);
        static cvar_t *dm_filter_empty      = Cvar_Get("dm_filter_empty", "0", CVAR_ARCHIVE);
        static cvar_t *dm_filter_full       = Cvar_Get("dm_filter_full", "0", CVAR_ARCHIVE);

        if (dm_min_players->integer) {
            AddFilter(filter, va("numplayers >= %d", dm_min_players->integer), sizeof(filter));
        }

        if (dm_max_players->integer) {
            AddFilter(filter, va("numplayers <= %d", dm_max_players->integer), sizeof(filter));
        }

        if (dm_show_demo_servers && !dm_show_demo_servers->integer) {
            AddFilter(filter, "gamever not like 'd%'", sizeof(filter));
        }

        if (dm_realism_mode && dm_realism_mode->integer == 1) {
            AddFilter(filter, "realism=1", sizeof(filter));
        }

        if (dm_filter_listen->integer == 1) {
            AddFilter(filter, "dedicated=1", sizeof(filter));
        }

        if (dm_filter_empty && dm_filter_empty->integer) {
            AddFilter(filter, "numplayers > 0", sizeof(filter));
        }

        if (dm_filter_full && dm_filter_full->integer == 1) {
            AddFilter(filter, "numplayers < maxplayers", sizeof(filter));
        }

        ServerListUpdate2(m_serverList[0], true, filter, GQueryType::qt_status);

        if (m_serverList[1]) {
            ServerListUpdate2(m_serverList[1], true, filter, GQueryType::qt_status);
        }
    }

    m_bUpdatingList = true;

    RefreshStatus();
}

void UIFAKKServerList::RefreshLANServerList(Event *ev)
{
    int                 i;
    FAKKServerListItem *pNewServerItem;

    for (i = 1; i <= getNumItems(); i++) {
        pNewServerItem = static_cast<FAKKServerListItem *>(GetItem(i));
        m_iTotalNumPlayers -= pNewServerItem->GetNumPlayers();
        pNewServerItem->SetQueried(false);
        pNewServerItem->SetNumPlayers(0);
        pNewServerItem->SetQueryFailed(false);
    }

    if (m_serverList[0]) {
        ServerListClear(m_serverList[0]);
    }

    if (m_serverList[1]) {
        ServerListClear(m_serverList[1]);
    }

    if (!m_serverList[0] && (com_target_game->integer < target_game_e::TG_MOHTT || !m_serverList[1])) {
        NewServerList();
    }

    m_bDoneUpdating[0]        = false;
    m_bDoneUpdating[1]        = false;
    m_NeedAdditionalLANSearch = true;

    Cvar_Set("dm_playercount", "0");
    // Search all LAN servers from port 12300 to 12316
    ServerListLANUpdate(m_serverList[0], true, 12300, 12316, 1);

    if (m_serverList[1]) {
        // If another game is supported search for it
        ServerListLANUpdate(m_serverList[1], true, 12300, 12316, 1);
    }
}

static void AddFilter(char *filter, const char *value)
{
    if (*filter) {
        strcat(filter, va(" and %s", value));
    } else {
        strcpy(filter, value);
    }
}

//
// Added in OPM
//  Filter with safety checks
//
static void AddFilter(char *filter, const char *value, size_t maxsize)
{
    const char *newval;
    size_t      valuelen;
    size_t      filterlen;

    if (*filter) {
        newval = va(" and %s", value);
    } else {
        newval = value;
    }

    valuelen  = strlen(newval);
    filterlen = strlen(filter);
    if (filterlen + valuelen >= maxsize) {
        return;
    }

    strncpy(filter + filterlen, newval, maxsize - filterlen);
}

void UIFAKKServerList::CancelRefresh(Event *ev)
{
    ServerListHalt(m_serverList[0]);
    if (m_serverList[1]) {
        ServerListHalt(m_serverList[1]);
    }
}

void UIFAKKServerList::NewServerList(void)
{
    int         iNumConcurrent;
    const char *secret_key;
    const char *game_name;
    cvar_t     *pRateCvar = Cvar_Get("rate", "5000", CVAR_ARCHIVE | CVAR_USERINFO);

    if (pRateCvar->integer > 25000) {
        iNumConcurrent = 15;
    } else if (pRateCvar->integer > 5000) {
        iNumConcurrent = 10;
    } else if (pRateCvar->integer > 3000) {
        iNumConcurrent = 6;
    } else {
        iNumConcurrent = 4;
    }

    m_iServerQueryCount = 0;
    m_iServerTotalCount = 0;

    if (com_target_game->integer < target_game_e::TG_MOHTT) {
        game_name  = GS_GetCurrentGameName();
        secret_key = GS_GetCurrentGameKey();

        // standard mohaa server
        m_ServerListInst[0].iServerType = com_target_game->integer;
        m_ServerListInst[0].serverList  = this;

        m_serverList[0] = ServerListNew(
            game_name,
            game_name,
            secret_key,
            iNumConcurrent,
            (void *)&UpdateServerListCallBack,
            1,
            (void *)&m_ServerListInst[0]
        );

        m_serverList[1] = NULL;
    } else {
        static cvar_t *dm_omit_spearhead = Cvar_Get("dm_omit_spearhead", "0", 1);

        game_name  = GS_GetGameName(target_game_e::TG_MOHTT);
        secret_key = GS_GetGameKey(target_game_e::TG_MOHTT);

        m_ServerListInst[0].iServerType = target_game_e::TG_MOHTT;
        m_ServerListInst[0].serverList  = this;

        // As there are 2 server lists it's better to balance the number of parallel requests
        iNumConcurrent = iNumConcurrent * 4 / 5;

        m_serverList[0] = ServerListNew(
            game_name,
            game_name,
            secret_key,
            iNumConcurrent,
            (void *)&UpdateServerListCallBack,
            1,
            (void *)&m_ServerListInst[0]
        );

        if (!dm_omit_spearhead->integer) {
            // Since mohtt is compatible with mohta
            // Search for both type of servers
            game_name  = GS_GetGameName(target_game_e::TG_MOHTA);
            secret_key = GS_GetGameKey(target_game_e::TG_MOHTA);

            m_ServerListInst[1].iServerType = target_game_e::TG_MOHTA;
            m_ServerListInst[1].serverList  = this;

            m_serverList[1] = ServerListNew(
                game_name,
                game_name,
                secret_key,
                iNumConcurrent,
                (void *)&UpdateServerListCallBack,
                1,
                (void *)&m_ServerListInst[1]
            );
        }
    }
}

void UIFAKKServerList::MakeLANListing(Event *ev)
{
    m_bLANListing = true;
}

void UIFAKKServerList::UpdateServer(Event *ev)
{
    if (getCurrentItem() <= 0) {
        return;
    }

    FAKKServerListItem *item = (FAKKServerListItem *)GetItem(getCurrentItem());
    ServerListAuxUpdate(m_serverList[0], item->m_sIP.c_str(), item->m_iGameSpyPort, true, GQueryType::qt_status);

    if (com_target_game->integer >= target_game_e::TG_MOHTT) {
        const cvar_t *dm_omit_spearhead = Cvar_Get("dm_omit_spearhead", "0", CVAR_ARCHIVE);
        // check for Spearhead
        if (!dm_omit_spearhead->integer) {
            ServerListAuxUpdate(
                m_serverList[1], item->m_sIP.c_str(), item->m_iGameSpyPort, true, GQueryType::qt_status
            );
        }
    }
}

int UIFAKKServerList::ServerCompareFunction(const UIListCtrlItem *i1, const UIListCtrlItem *i2, int columnname)
{
    int                       iCompResult;
    int                       val1, val2;
    const FAKKServerListItem *fi1 = static_cast<const FAKKServerListItem *>(i1);
    const FAKKServerListItem *fi2 = static_cast<const FAKKServerListItem *>(i2);

    if (fi1->IsFavorite() != fi2->IsFavorite()) {
        if (fi1->IsFavorite()) {
            iCompResult = -1;
        } else {
            iCompResult = 1;
        }

        if (g_bReverseSort) {
            iCompResult = -iCompResult;
        }
    } else if (fi1->IsQueried() != fi2->IsQueried()) {
        if (fi1->IsQueried()) {
            iCompResult = -1;
        } else {
            iCompResult = 1;
        }

        if (g_bReverseSort) {
            iCompResult = -iCompResult;
        }
    } else if (fi1->IsDifferentVersion() != fi2->IsDifferentVersion()) {
        if (fi1->IsDifferentVersion()) {
            iCompResult = 1;
        } else {
            iCompResult = -1;
        }

        if (g_bReverseSort) {
            iCompResult = -iCompResult;
        }
    } else if (fi1->IfQueryFailed() != fi2->IfQueryFailed()) {
        if (fi1->IfQueryFailed()) {
            iCompResult = 1;
        } else {
            iCompResult = -1;
        }

        if (g_bReverseSort) {
            iCompResult = -iCompResult;
        }
    } else if (g_bNumericSort) {
        val1 = fi1->getListItemValue(columnname);
        val2 = fi2->getListItemValue(columnname);

        if (val1 < val2) {
            iCompResult = -1;
        } else if (val1 > val2) {
            iCompResult = 1;
        } else {
            iCompResult = 0;
        }
    } else {
        iCompResult = str::icmp(fi1->getListItemString(columnname), fi2->getListItemString(columnname));
    }

    if (!iCompResult) {
        if (columnname == 3) {
            //
            // Added in OPM
            //
            //  Sort by bot count
            const str   s1 = fi1->getListItemString(columnname);
            const str   s2 = fi2->getListItemString(columnname);
            const char *c1 = strchr(s1, '(');
            const char *c2 = strchr(s2, '(');

            if (c1 || c2) {
                if (c1 && !c2) {
                    iCompResult = -1;
                } else if (!c1 && c2) {
                    iCompResult = 1;
                } else {
                    // Both have bots
                    val1 = atoi(c1 + 1);
                    val2 = atoi(c2 + 1);

                    if (val1 > val2) {
                        iCompResult = -1;
                    } else if (val1 < val2) {
                        iCompResult = 1;
                    } else {
                        iCompResult = 0;
                    }
                }
            }
        }

        if (!iCompResult) {
            if (columnname != 2) {
                val1 = fi1->getListItemValue(2);
                val2 = fi2->getListItemValue(2);

                if (val1 < val2) {
                    iCompResult = -1;
                } else if (val1 > val2) {
                    iCompResult = 1;
                } else {
                    iCompResult = 0;
                }
            }
        }

        if (!iCompResult) {
            if (columnname != 4) {
                iCompResult = str::icmp(fi1->getListItemString(4), fi2->getListItemString(4));
            }
        }

        if (!iCompResult) {
            if (columnname != 5) {
                iCompResult = str::icmp(fi1->getListItemString(5), fi2->getListItemString(5));
            }
        }

        if (!iCompResult) {
            if (columnname != 3) {
                val1 = fi1->getListItemValue(3);
                val2 = fi2->getListItemValue(3);

                if (val1 < val2) {
                    iCompResult = 1;
                } else if (val1 > val2) {
                    iCompResult = -1;
                } else {
                    iCompResult = 0;
                }
            }
        }

        if (!iCompResult) {
            if (columnname != 0) {
                iCompResult = str::icmp(fi1->getListItemString(0), fi2->getListItemString(0));
            }
        }

        if (!iCompResult) {
            if (columnname != -1) {
                iCompResult = str::icmp(fi1->getListItemString(1), fi2->getListItemString(1));
            }
        }

        if (g_bReverseSort) {
            iCompResult = -iCompResult;
        }
    }

    return iCompResult;
}

void UIFAKKServerList::Draw(void)
{
    if (m_serverList[0]) {
        GServerListState listState[2];

        ServerListThink(m_serverList[0]);
        if (m_serverList[1]) {
            ServerListThink(m_serverList[1]);
        }

        listState[0] = ServerListState(m_serverList[0]);
        listState[1] = m_serverList[1] ? ServerListState(m_serverList[1]) : GServerListState::sl_idle;
        if (listState[0] != GServerListState::sl_idle || listState[1] != GServerListState::sl_idle) {
            menuManager.PassEventToWidget("refresh", new Event(EV_Widget_Disable));
            menuManager.PassEventToWidget("cancelrefresh", new Event(EV_Widget_Enable));
        } else {
            menuManager.PassEventToWidget("refresh", new Event(EV_Widget_Enable));
            menuManager.PassEventToWidget("cancelrefresh", new Event(EV_Widget_Disable));
        }
    } else if (!m_bHasList) {
        if (m_bLANListing) {
            RefreshLANServerList(NULL);
        } else {
            RefreshServerList(NULL);
        }

        m_bHasList = true;

        menuManager.PassEventToWidget("refresh", new Event(EV_Widget_Enable));
        menuManager.PassEventToWidget("cancelrefresh", new Event(EV_Widget_Disable));

        if (m_NeedAdditionalLANSearch) {
            m_NeedAdditionalLANSearch = false;
            ServerListLANUpdate(m_serverList[0], true, 12201, 12233, 1);

            if (m_serverList[1]) {
                ServerListLANUpdate(m_serverList[1], true, 12201, 12233, 1);
            }
        }
    }

    UIListCtrl::Draw();
}

void UIFAKKServerList::SortByColumn(int column)
{
    int             i;
    bool            exists     = 0;
    bool            numeric    = 0;
    bool            reverse    = 0;
    UIListCtrlItem *selected   = NULL;
    bool            selvisible = false;

    for (i = 1; i <= m_columnlist.NumObjects(); i++) {
        const columndef_t& def = m_columnlist.ObjectAt(i);
        if (def.name == column) {
            // found one
            numeric = def.numeric;
            reverse = def.reverse_sort;
            exists  = true;
            break;
        }
    }

    if (!exists) {
        m_iLastSortColumn = 0;
        return;
    }

    m_iLastSortColumn = column;
    s_qsortcolumn     = column;
    s_qsortobject     = this;
    s_qsortreverse    = reverse;
    g_bNumericSort    = numeric;
    g_bReverseSort    = reverse;

    if (getCurrentItem()) {
        selected = m_itemlist.ObjectAt(getCurrentItem());
        if (GetScrollBar()) {
            selvisible = getCurrentItem() >= GetScrollBar()->getTopItem() + 1
                      && getCurrentItem() <= GetScrollBar()->getPageHeight() + GetScrollBar()->getTopItem();
        } else {
            selvisible = getCurrentItem() > 0;
        }
    }

    setCompareFunction(&UIFAKKServerList::ServerCompareFunction);
    if (m_itemlist.NumObjects()) {
        m_itemlist.Sort(&UIListCtrl::QsortCompare);
    }

    if (selected) {
        if (selvisible) {
            TrySelectItem(FindItem(selected));
        } else {
            m_currentItem = FindItem(selected);
        }
    }
}

void UIFAKKServerList::UpdateServerListCallBack(
    GServerList serverlist, int msg, void *instance, void *param1, void *param2
)
{
    int                 i, j;
    int                 iPort, iGameSpyPort;
    unsigned int        iRealIP;
    str                 sAddress;
    GServer             server;
    FAKKServerListItem *pNewServerItem;
    UIFAKKServerList   *uiServerList;
    int                 iServerType;
    // filters
    static cvar_t *dm_filter_empty      = Cvar_Get("dm_filter_empty", "0", CVAR_ARCHIVE);
    static cvar_t *dm_filter_full       = Cvar_Get("dm_filter_full", "0", CVAR_ARCHIVE);
    static cvar_t *dm_filter_pure       = Cvar_Get("dm_filter_pure", "0", CVAR_ARCHIVE);
    static cvar_t *dm_max_ping          = Cvar_Get("dm_max_ping", "0", CVAR_ARCHIVE);
    static cvar_t *dm_free_for_all      = Cvar_Get("dm_free_for_all", "1", CVAR_ARCHIVE);
    static cvar_t *dm_objective_match   = Cvar_Get("dm_objective_match", "1", CVAR_ARCHIVE);
    static cvar_t *dm_round_based_match = Cvar_Get("dm_round_based_match", "1", CVAR_ARCHIVE);
    static cvar_t *dm_team_match        = Cvar_Get("dm_team_match", "1", CVAR_ARCHIVE);
    static cvar_t *dm_tow_match         = Cvar_Get("dm_tow_match", "1", CVAR_ARCHIVE);
    static cvar_t *dm_liberation_match  = Cvar_Get("dm_liberation_match", "1", CVAR_ARCHIVE);
    static cvar_t *dm_run_fast          = Cvar_Get("dm_run_fast", "1", CVAR_ARCHIVE);
    static cvar_t *dm_run_normal        = Cvar_Get("dm_run_normal", "1", CVAR_ARCHIVE);
    static cvar_t *dm_omit_spearhead    = Cvar_Get("dm_omit_spearhead", "0", CVAR_ARCHIVE);

    iServerType    = ((FAKKServerListInstance *)instance)->iServerType;
    uiServerList   = ((FAKKServerListInstance *)instance)->serverList;
    pNewServerItem = NULL;
    server         = (GServer)param1;

    // Changed in OPM
    //  Instead of calling Cvar_Set() for each list
    //  RefreshStatus() is called to combine results of all lists

    if (param2) {
        if (msg == LIST_PROGRESS && param2 == (void *)-1) {
            iRealIP = inet_addr(ServerGetAddress(server));
            ServerGetIntValue(server, "hostport", PORT_SERVER);
            iGameSpyPort = ServerGetQueryPort(server);

            for (i = 1; i <= uiServerList->getNumItems(); i++) {
                pNewServerItem = static_cast<FAKKServerListItem *>(uiServerList->GetItem(i));
                if (pNewServerItem->m_uiRealIP == iRealIP && pNewServerItem->m_iGameSpyPort == iGameSpyPort) {
                    break;
                }
            }

            if (i <= uiServerList->getNumItems() && pNewServerItem) {
                pNewServerItem->SetQueryFailed(true);
            }

            return;
        }

        // Removed in OPM
        //Cvar_Set("dm_serverstatusbar", va("%i", (int)(uintptr_t)param2));
        // Fixed in OPM
        //  As both lists are combined, show the correct percentage
        // Removed in OPM
        //Cvar_Set("dm_serverstatusbar", va("%i", 100 * m_iServerQueryCount / m_iServerTotalCount));
    }

    if (msg == LIST_PROGRESS) {
        const char *pszHostName;
        bool        bDiffVersion;
        bool        bIsDemo;
        str         sServerName;
        str         sPlayers;
        const char *pszGameVer;
        const char *pszGameVerNumber;
        float       fGameVer;
        int         iNumPlayers, iMaxPlayers;
        int         iMinPlayers;

        pszHostName      = ServerGetStringValue(server, "hostname", "(NONE)");
        bDiffVersion     = false;
        bIsDemo          = false;
        pszGameVer       = ServerGetStringValue(server, "gamever", "1.00");
        pszGameVerNumber = pszGameVer;

        if (pszGameVerNumber[0] == 'd') {
            // demo server
            pszGameVerNumber++;
            bIsDemo = true;
        }

        fGameVer = atof(pszGameVerNumber);

        if (com_target_game->integer >= target_game_e::TG_MOHTT) {
            if (iServerType == target_game_e::TG_MOHTT) {
                //if (fabs(fGameVer - com_target_version->value) > 0.1f) {
                //    bDiffVersion = true;
                //}
                if (fabs(fGameVer) < 2.3f) {
                    bDiffVersion = true;
                }
            } else {
                //if (fabs(fGameVer - com_target_version->value) > 0.3f) {
                //    bDiffVersion = true;
                //}
                if (fabs(fGameVer) < 2.1f) {
                    bDiffVersion = true;
                }
            }
        } else {
            if (fabs(fGameVer - com_target_shortversion->value) > 0.1f) {
                bDiffVersion = true;
            }
        }
        // always show the version
        if (!bIsDemo) {
            sServerName = va(" (%s) %s", pszGameVerNumber, pszHostName);
        } else {
            sServerName = va(" (d%s) %s", pszGameVerNumber, pszHostName);
        }

        iRealIP      = inet_addr(ServerGetAddress(server));
        iPort        = ServerGetIntValue(server, "hostport", PORT_SERVER);
        iGameSpyPort = ServerGetQueryPort(server);
        sAddress     = va("%s:%i", ServerGetAddress(server), iPort);

        iNumPlayers = ServerGetIntValue(server, "numplayers", 0);
        iMaxPlayers = ServerGetIntValue(server, "maxplayers", 0);

        // Added in OPM
        //  Try to get and display the number of bots
        iMinPlayers = ServerGetIntValue(server, "minplayers", 0);
        if (iMinPlayers > iNumPlayers) {
            sPlayers = va("%d(%d)/%d", iNumPlayers, iMinPlayers - iNumPlayers, iMaxPlayers);
        } else {
            sPlayers = va("%d/%d", iNumPlayers, iMaxPlayers);
        }

        for (i = 1; i <= uiServerList->getNumItems(); i++) {
            pNewServerItem = static_cast<FAKKServerListItem *>(uiServerList->GetItem(i));
            if (pNewServerItem->m_uiRealIP == iRealIP && pNewServerItem->m_iGameSpyPort == iGameSpyPort) {
                break;
            }
        }

        if (i > uiServerList->getNumItems() || !pNewServerItem) {
            pNewServerItem             = new FAKKServerListItem(uiServerList, "?", sAddress, "?", "?/?", "?", "?", "?");
            pNewServerItem->m_sIP      = ServerGetAddress(server);
            pNewServerItem->m_uiRealIP = iRealIP;
            pNewServerItem->m_iGameSpyPort = iGameSpyPort;

            uiServerList->AddItem(pNewServerItem);
        }

        uiServerList->m_iTotalNumPlayers -= pNewServerItem->GetNumPlayers();

        pNewServerItem->m_iPort = iPort;

        pNewServerItem->setListItemString(0, sServerName);
        pNewServerItem->setListItemString(1, sAddress);
        pNewServerItem->setListItemString(2, va("%d", ServerGetPing(server)));
        pNewServerItem->setListItemString(3, sPlayers.c_str());
        pNewServerItem->setListItemString(4, ServerGetStringValue(server, "gametype", "(NONE)"));
        pNewServerItem->setListItemString(5, ServerGetStringValue(server, "mapname", "(NONE)"));
        pNewServerItem->SetListItemVersion(pszGameVer);
        pNewServerItem->SetDifferentVersion(bDiffVersion);
        pNewServerItem->SetQueried(true);
        pNewServerItem->SetNumPlayers(ServerGetIntValue(server, "numplayers", 0));
        uiServerList->m_iTotalNumPlayers += pNewServerItem->GetNumPlayers();
        // Removed in OPM
        //Cvar_Set("dm_servercount", va("%d/%d", m_iServerQueryCount, m_iServerTotalCount));

        uiServerList->SortByLastSortColumn();

        uiServerList->m_iServerQueryCount = uiServerList->getNumItems();
    } else if (msg == LIST_STATECHANGED) {
        switch (ServerListState(serverlist)) {
        case GServerListState::sl_idle:
            if (com_target_game->integer >= target_game_e::TG_MOHTT) {
                if (iServerType == target_game_e::TG_MOHTT) {
                    uiServerList->m_bDoneUpdating[0] = true;
                } else if (iServerType == target_game_e::TG_MOHTA || dm_omit_spearhead->integer) {
                    uiServerList->m_bDoneUpdating[1] = true;
                }
            } else {
                uiServerList->m_bDoneUpdating[0] = true;
                uiServerList->m_bDoneUpdating[1] = true;
            }

            if (uiServerList->m_bDoneUpdating[0] && uiServerList->m_bDoneUpdating[1]) {
                // Removed in OPM
                //Cvar_Set("dm_serverstatus", "Done Updating.");
                //Cvar_Set("dm_serverstatusbar", "0");
                //Cvar_Set("dm_servercount", va("%d", uiServerList->getNumItems()));

                uiServerList->m_bUpdatingList = false;
                uiServerList->SortByLastSortColumn();
            }
            break;
        case GServerListState::sl_listxfer:
            // Removed in OPM
            //Cvar_Set("dm_serverstatus", "Getting List.");

            if (com_target_game->integer >= target_game_e::TG_MOHTT) {
                if (iServerType == target_game_e::TG_MOHTT) {
                    uiServerList->m_bGettingList[0] = true;
                }
                if (iServerType == target_game_e::TG_MOHTA) {
                    uiServerList->m_bGettingList[1] = true;
                }
            } else {
                uiServerList->m_bGettingList[0] = true;
                uiServerList->m_bGettingList[1] = false;
            }
            uiServerList->m_bUpdatingList = true;
            return;
        case GServerListState::sl_lanlist:
            // Removed in OPM
            //Cvar_Set("dm_serverstatus", "Searching LAN.");

            uiServerList->m_bUpdatingList = true;
            break;
        case GServerListState::sl_querying:
            // Removed in OPM
            //Cvar_Set("dm_serverstatus", "Querying Servers.");

            uiServerList->m_bUpdatingList = true;
            break;
        default:
            break;
        }

        //if (!uiServerList->m_bGettingList[0] && !uiServerList->m_bGettingList[1]) {
        //    return;
        //}

        //
        // Rebuild the number of servers
        //

        uiServerList->m_iServerTotalCount = 0;
        for (i = 0; i < ARRAY_LEN(uiServerList->m_serverList); i++) {
            if (uiServerList->m_bGettingList[i] && uiServerList->m_serverList[i]) {
                uiServerList->m_iServerTotalCount += ServerListCount(uiServerList->m_serverList[i]);
            }
        }

        // Removed in 2.0
        //  Only add entries for servers that are queried successfully
        //  it avoids unnecessary entries
#if 0
        for (j = 0; j < ServerListCount(serverlist); j++) {
            GServer arrayServer = ServerListGetServer(serverlist, j);
        
            iRealIP = inet_addr(ServerGetAddress(arrayServer));
            iGameSpyPort = ServerGetQueryPort(arrayServer);

            for (i = 1; i <= uiServerList->getNumItems(); i++) {
                pNewServerItem = static_cast<FAKKServerListItem*>(uiServerList->GetItem(i));
                if (pNewServerItem->m_uiRealIP == iRealIP && pNewServerItem->m_iGameSpyPort == iGameSpyPort) {
                    break;
                }
            }

            if (i <= uiServerList->getNumItems() && pNewServerItem) {
                continue;
            }

            pNewServerItem = new FAKKServerListItem(uiServerList, "?", sAddress, "?", "?/?", "?", "?", "?");
            pNewServerItem->m_sIP = ServerGetAddress(arrayServer);
            pNewServerItem->m_uiRealIP = iRealIP;
            pNewServerItem->m_iPort = PORT_SERVER;
            pNewServerItem->m_iGameSpyPort = iGameSpyPort;
            pNewServerItem->SetDifferentVersion(false);
            pNewServerItem->SetQueried(false);

            uiServerList->AddItem(pNewServerItem);
        }
#endif

        /*
        for (i = 1; i <= uiServerList->getNumItems(); i++)
        {
            pNewServerItem = static_cast<FAKKServerListItem*>(uiServerList->GetItem(i));

            if (!pNewServerItem->IsFavorite())
            {
                for (j = 0; j < ServerListCount(serverlist); j++) {
                    GServer arrayServer = ServerListGetServer(serverlist, j);

                    iRealIP = inet_addr(ServerGetAddress(arrayServer));
                    iGameSpyPort = ServerGetQueryPort(arrayServer);

                    if (pNewServerItem->m_uiRealIP == iRealIP && pNewServerItem->m_iGameSpyPort == iGameSpyPort) {
                        break;
                    }
                }

                if (j == ServerListCount(serverlist)) {
                    uiServerList->DeleteItem(j);
                    j--;
                }
            }
        }
        */
    }

    // Changed in OPM
    uiServerList->RefreshStatus();
}

void UIFAKKServerList::RefreshStatus()
{
    bool doneUpdating;
    int  i;

    Cvar_Set("dm_servercount", va("%d", getNumItems()));
    Cvar_Set("dm_playercount", va("%d", m_iTotalNumPlayers));
    if (m_iServerTotalCount) {
        Cvar_Set("dm_serverstatusbar", va("%i", 100 * m_iServerQueryCount / m_iServerTotalCount));
    }

    doneUpdating = true;

    for (i = 0; i < NUM_SERVERLISTS; i++) {
        if (!m_serverList[i]) {
            continue;
        }

        if (ServerListState(m_serverList[i]) != GServerListState::sl_idle || !m_bDoneUpdating[i]) {
            doneUpdating = false;
            break;
        }
    }

    for (i = 0; i < NUM_SERVERLISTS; i++) {
        if (!m_serverList[i]) {
            continue;
        }

        switch (ServerListState(m_serverList[i])) {
        case GServerListState::sl_idle:
            if (doneUpdating) {
                Cvar_Set("dm_serverstatus", "Done Updating.");
                Cvar_Set("dm_serverstatusbar", "0");
            } else {
                Cvar_Set("dm_serverstatus", "Querying Servers.");
            }
            break;
        case GServerListState::sl_listxfer:
            Cvar_Set("dm_serverstatus", "Getting List.");
            break;
        case GServerListState::sl_lanlist:
            Cvar_Set("dm_serverstatus", "Searching LAN.");
            Cvar_Set("dm_servercount", va("%d/%d", m_iServerQueryCount, m_iServerTotalCount));
            return;
        case GServerListState::sl_querying:
            Cvar_Set("dm_serverstatus", "Querying Servers.");
            Cvar_Set("dm_servercount", va("%d/%d", m_iServerQueryCount, m_iServerTotalCount));
            return;
        default:
            break;
        }
    }
}
