/*
===========================================================================
Copyright (C) 2023-2024 the OpenMoHAA team

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
#include "../qcommon/localization.h"

class LANGameItem : public UIListCtrlItem
{
    str strings[6];

public:
    LANGameItem();
    LANGameItem(
        const str& hostName,
        const str& mapName,
        const str& players,
        const str& gameTypeString,
        const str& ping,
        const str& ipAddress
    );

    int            getListItemValue(int which) const override;
    griditemtype_t getListItemType(int which) const override;
    str            getListItemString(int which) const override;
    void           DrawListItem(int iColumn, const UIRect2D& drawRect, bool bSelected, UIFont *pFont) override;
    qboolean       IsHeaderEntry() const override;
};

Event EV_LANGame_JoinGame
(
    "joingame",
    EV_DEFAULT,
    NULL,
    NULL,
    "Join the currently selected server"
);

CLASS_DECLARATION(UIListCtrl, UILANGameClass, NULL) {
    {&EV_UIListBase_ItemSelected,      &UILANGameClass::SelectServer},
    {&EV_UIListBase_ItemDoubleClicked, &UILANGameClass::JoinServer  },
    {&EV_LANGame_JoinGame,             &UILANGameClass::JoinServer  },
    {NULL,                             NULL                         }
};

UILANGameClass::UILANGameClass()
{
    Connect(this, EV_UIListBase_ItemDoubleClicked, EV_UIListBase_ItemDoubleClicked);
    Connect(this, EV_UIListBase_ItemSelected, EV_UIListBase_ItemSelected);
    AllowActivate(true);
    setHeaderFont("facfont-20");

    m_iLastUpdateTime = 0;
    m_noservers_mat   = uWinMan.RegisterShader("textures/menu/noservers");

    //
    // Added in OPM
    //
    m_bVirtual = false;
}

void UILANGameClass::Draw(void)
{
    if (cls.bNewLocalServerInfo) {
        SetupServers();
    }

    UIListCtrl::Draw();

    if (!getNumItems() && m_noservers_mat) {
        float x, y, w, h;

        w = re.GetShaderWidth(m_noservers_mat->GetMaterial()) * m_vVirtualScale[0];
        h = re.GetShaderHeight(m_noservers_mat->GetMaterial()) * m_vVirtualScale[1];

        UIRect2D frame = getClientFrame();
        x              = (frame.size.width - w) * 0.5f;
        y              = (frame.size.height - h) * 0.5f;

        re.DrawStretchPic(x, y, w, h, 0, 0, 1, 1, m_noservers_mat->GetMaterial());
    }

    if (m_iLastUpdateTime && m_iLastUpdateTime < cls.realtime) {
        Cvar_Set("cl_langamerefreshstatus", "Ready");
        m_iLastUpdateTime = 0;
    }
}

void UILANGameClass::UpdateUIElement(void)
{
    float width;

    RemoveAllColumns();

    width = getClientFrame().size.width - 16.f;

    AddColumn(Sys_LV_CL_ConvertString("Server Name"), 0, width * 0.27f, false, false); // was 0.4
    AddColumn(Sys_LV_CL_ConvertString("Map"), 1, width * 0.12f, false, false);         // was 0.15
    AddColumn(Sys_LV_CL_ConvertString("Players"), 2, width * 0.08f, true, true);       // was 0.165
    AddColumn(Sys_LV_CL_ConvertString("GameType"), 3, width * 0.118f, false, false);   // was 0.22
    AddColumn(Sys_LV_CL_ConvertString("Ping"), 4, width * 0.052f, true, false);        // was 0.065
    AddColumn(Sys_LV_CL_ConvertString("IP"), 5, width * 0.36f, false, false);          // Added in OPM

    uWinMan.ActivateControl(this);

    SetupServers();
}

void UILANGameClass::SetupServers(void)
{
    int                 i;
    LANGameItem        *pItem;
    const serverInfo_t *pServerInfo;

    for (i = 1; i <= cls.numlocalservers; i++) {
        pServerInfo = &cls.localServers[i - 1];

        if (i > getNumItems()) {
            pItem = new LANGameItem();
            AddItem(pItem);
        } else {
            pItem = static_cast<LANGameItem *>(GetItem(i));
        }

        *pItem = LANGameItem(
            pServerInfo->hostName,
            pServerInfo->mapName,
            va("%02i/%02i", pServerInfo->clients, pServerInfo->maxClients),
            pServerInfo->gameTypeString,
            str(pServerInfo->ping),
            // Added in OPM
            NET_AdrToStringwPort(pServerInfo->adr)
        );
    }

    while (cls.numlocalservers < getNumItems()) {
        DeleteItem(getNumItems());
    }

    cls.bNewLocalServerInfo = false;
    m_iLastUpdateTime       = 0;
}

void UILANGameClass::SelectServer(Event *ev)
{
    const serverInfo_t& serverInfo = cls.localServers[getCurrentItem() - 1];
    static const char   info[]     = "\xFF\xFF\xFF\xFF\x02getinfo xxx";

    Com_Printf("Requesting updated info from server...\n");
    Cvar_Set("cl_langamerefreshstatus", "Requesting updated info from server...");

    NET_SendPacket(NS_CLIENT, sizeof(info), info, serverInfo.adr);
    m_iLastUpdateTime = cls.realtime + 3000;
}

void UILANGameClass::JoinServer(Event *ev)
{
    if (!getCurrentItem()) {
        return;
    }

    const serverInfo_t& serverInfo = cls.localServers[getCurrentItem() - 1];

    UI_SetReturnMenuToCurrent();
    CL_Connect(NET_AdrToStringwPort(serverInfo.adr), netadrtype_t::NA_UNSPEC);
}

qboolean UILANGameClass::KeyEvent(int key, unsigned int time)
{
    switch (key) {
    case K_ENTER:
    case K_KP_ENTER:
        JoinServer(NULL);
        return true;
    case K_UPARROW:
        if (getCurrentItem() > 1) {
            TrySelectItem(getCurrentItem() - 1);
            SelectServer(NULL);
        }
        return false;
    case K_DOWNARROW:
        if (getCurrentItem() < getNumItems()) {
            TrySelectItem(getCurrentItem() + 1);
            SelectServer(NULL);
        }
        return false;
    }

    return UIListBase::KeyEvent(key, time);
}

LANGameItem::LANGameItem() {}

LANGameItem::LANGameItem(
    const str& hostName,
    const str& mapName,
    const str& players,
    const str& gameTypeString,
    const str& ping,
    const str& ipAddress
)
{
    strings[0] = hostName;
    strings[1] = mapName;
    strings[2] = players;
    strings[3] = gameTypeString;
    strings[4] = ping;

    //
    // Added in OPM
    //

    strings[5] = ipAddress;
}

int LANGameItem::getListItemValue(int which) const
{
    return atoi(strings[which]);
}

griditemtype_t LANGameItem::getListItemType(int which) const
{
    return griditemtype_t::TYPE_STRING;
}

str LANGameItem::getListItemString(int which) const
{
    return strings[which];
}

void LANGameItem::DrawListItem(int iColumn, const UIRect2D& drawRect, bool bSelected, UIFont *pFont) {}

qboolean LANGameItem::IsHeaderEntry() const
{
    return qfalse;
}
