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

#include "ui_local.h"
#include "../qcommon/localization.h"

Event EV_UILGL_ConnectToSelected
(
    "connecttoselected",
    EV_DEFAULT,
    NULL,
    NULL,
    "Makes the lan game list connect to the selected server"
);

CLASS_DECLARATION(UIWidget, UILanGameList, NULL) {
    {&EV_UILGL_ConnectToSelected, &UILanGameList::EventConnect},
    {NULL,                        NULL                        }
};

UILanGameList::UILanGameList()
{
    m_Vscroll = new UIVertScroll();
    m_created = false;

    AllowActivate(true);
    m_activerow  = 0;
    m_activeitem = 0;

    m_noservers_wid = NULL;
    m_noservers_mat = uWinMan.RegisterShader("textures/menu/noservers");
    m_fill_mat      = NULL;

    m_iPrevNumServers = 0;
    m_iCurrNumServers = 0;
}

UILanGameList::~UILanGameList()
{
    m_dying = true;
}

void UILanGameList::FrameInitialized(void) {}

void UILanGameList::AddColumn(str sName, UIReggedMaterial *pMaterial, int iWidth, Container<str> *csEntries)
{
    Event    evVirtualRes(EV_Layout_VirtualRes);
    int      i;
    qboolean iColorToggle = qfalse;
    UILabel *label;

    evVirtualRes.AddInteger(true);

    label = new UILabel();
    m_titlewidgets.AddObject(label);

    if (m_bVirtual) {
        label->ProcessEvent(evVirtualRes);
    }

    label->InitFrame(this, m_fCurColumnWidth, 4, iWidth, 24, 1);
    label->setFont("facfont-20");
    label->setFontHorizontalAlignment(FONT_JUSTHORZ_LEFT);
    label->setFontVerticalAlignment(FONT_JUSTVERT_CENTER);
    label->setName(sName);

    if (pMaterial) {
        label->setMaterial(pMaterial);
    } else {
        label->setTitle(sName);
    }

    label->Realign();
    label->setForegroundColor(UColor(0.7f, 0.6f, 0.05f, 1.f));
    label->setBackgroundColor(UColor(0, 0, 0, 0), true);

    for (i = 0; i < csEntries->NumObjects(); i++) {
        UILanGameListLabel *listLabel = new UILanGameListLabel(this);
        const str&          sEntry    = csEntries->ObjectAt(i);

        if (m_bVirtual) {
            listLabel->ProcessEvent(evVirtualRes);
        }

        listLabel->InitFrame(this, m_fCurColumnWidth, i * 16 + 28, iWidth, 16, 0);
        listLabel->setName(sEntry);
        listLabel->setTitle(sEntry);
        listLabel->setFontHorizontalAlignment(FONT_JUSTHORZ_LEFT);
        listLabel->setFontVerticalAlignment(FONT_JUSTVERT_CENTER);
        listLabel->setShow(true);

        iColorToggle ^= 1;
        if (iColorToggle) {
            listLabel->setForegroundColor(UColor(0.07f, 0.06f, 0.005f));
            listLabel->setBackgroundColor(UColor(0.07f, 0.06f, 0.005f), true);
        } else {
            listLabel->setForegroundColor(UColor(0.21f, 0.18f, 0.015f));
            listLabel->setBackgroundColor(UColor(0.21f, 0.18f, 0.015f), true);
        }

        m_widgetlist.AddObject(listLabel);
    }

    m_iNumColumns++;
    m_fCurColumnWidth += iWidth;
}

void UILanGameList::AddNoServer(void)
{
    if (!m_noservers_wid) {
        m_noservers_wid = new UILabel();
        m_miscwidgets.AddObject(m_noservers_wid);
        if (m_bVirtual) {
            Event event(EV_Layout_VirtualRes);
            event.AddInteger(1);
            m_noservers_wid->ProcessEvent(event);
        }

        m_noservers_wid->setBackgroundColor(UColor(0, 0, 0, 0), true);
        m_noservers_wid->setMaterial(m_noservers_mat);
        m_noservers_wid->InitFrame(this, 0, 0, 100, 100, 0);
    } else {
        m_noservers_wid->setFrame(UIRect2D(0, 0, 100, 100));
    }

    m_noservers_wid->setShow(false);
}

void UILanGameList::CreateServerWidgets(void)
{
    int i;

    Event evVirtualRes(EV_Layout_VirtualRes);
    evVirtualRes.AddInteger(true);

    if (m_created) {
        return;
    }

    Container<str> cServerNums;
    Container<str> cHostNames;
    Container<str> cMapNames;
    Container<str> cClients;
    Container<str> cPing;
    Container<str> cNetType;
    Container<str> cGameTypeString;
    float          fFrameWidth;

    for (i = 1; i <= m_iCurrNumServers; i++) {
        const serverInfo_t *info = m_servers.ObjectAt(i);

        cServerNums.AddObject(str(i));
        cHostNames.AddObject(info->hostName);
        cMapNames.AddObject(info->mapName);
        cClients.AddObject(info->clients);
        cPing.AddObject(info->ping);
        cNetType.AddObject(info->netType);
        cGameTypeString.AddObject(info->gameTypeString);
    }

    m_fCurColumnWidth = 4;
    m_iNumColumns     = 0;
    fFrameWidth       = m_frame.size.width / m_vVirtualScale[0] - 16.0 - 12.0;

    AddColumn(Sys_LV_CL_ConvertString("Server Name"), NULL, fFrameWidth * 0.4f, &cHostNames);
    AddColumn(Sys_LV_CL_ConvertString("Map"), NULL, fFrameWidth * 0.15f, &cMapNames);
    AddColumn(Sys_LV_CL_ConvertString("Players"), NULL, fFrameWidth * 0.165f, &cClients);
    AddColumn(Sys_LV_CL_ConvertString("GameType"), NULL, fFrameWidth * 0.22f, &cGameTypeString);
    AddColumn(Sys_LV_CL_ConvertString("Ping"), NULL, fFrameWidth * 0.064f, &cPing);
    AddNoServer();

    m_created = true;
}

void UILanGameList::DestroyServerWidgets(void)
{
    int i;

    if (!m_created) {
        return;
    }

    for (i = 1; i <= m_titlewidgets.NumObjects(); i++) {
        delete m_titlewidgets.ObjectAt(i);
    }
    m_titlewidgets.ClearObjectList();

    for (i = 1; i <= m_widgetlist.NumObjects(); i++) {
        delete m_widgetlist.ObjectAt(i);
    }
    m_widgetlist.ClearObjectList();

    for (i = 1; i <= m_miscwidgets.NumObjects(); i++) {
        delete m_miscwidgets.ObjectAt(i);
    }
    m_miscwidgets.ClearObjectList();

    m_noservers_wid = NULL;
    m_created       = false;
}

void UILanGameList::RepositionServerWidgets(void)
{
    DestroyServerWidgets();

    m_Vscroll->setPageHeight((getSize().height - 24) / 16.f);
    m_Vscroll->setNumItems(m_iCurrNumServers);
    m_Vscroll->InitFrameAlignRight(this, 4, 4);
    CreateServerWidgets();
}

qboolean UILanGameList::KeyEvent(int key, unsigned int time)
{
    switch (key) {
    case K_ENTER:
    case K_KP_END:
        Connect();
        return qtrue;
    case K_MWHEELUP:
        m_Vscroll->AttemptScrollTo(m_Vscroll->getTopItem() - 1);
        return qtrue;
    case K_MWHEELDOWN:
        m_Vscroll->AttemptScrollTo(m_Vscroll->getTopItem() + 1);
        return qtrue;
    case K_UPARROW:
        if (m_activerow != 0) {
            if (m_activerow == m_Vscroll->getTopItem()) {
                m_Vscroll->AttemptScrollTo(m_activerow - 1);
                m_activerow = m_Vscroll->getTopItem();
            } else {
                m_activerow--;
            }
        }
        return qtrue;
    case K_DOWNARROW:
        if (m_activerow + 1 < m_Vscroll->getNumItems()) {
            if (m_activerow - m_Vscroll->getTopItem() + 1 < m_Vscroll->getPageHeight()) {
                if (m_activerow == m_Vscroll->getPageHeight() + m_Vscroll->getTopItem()) {
                    m_Vscroll->AttemptScrollTo(m_Vscroll->getTopItem() + 1);
                    m_activerow = m_Vscroll->getTopItem() + m_Vscroll->getPageHeight();
                } else {
                    m_activerow++;
                }
                m_Vscroll->AttemptScrollTo(m_activerow - 1);
                m_activerow = m_Vscroll->getTopItem();
            } else if (m_Vscroll->AttemptScrollTo(m_activerow + 1)) {
                m_activerow++;
            }
        }
        return qtrue;
    default:
        break;
    }

    return qfalse;
}

void UILanGameList::DrawNoServers(UIRect2D frame)
{
    float x;
    float w, h;

    if (!m_noservers_mat) {
        return;
    }

    if (!m_noservers_wid) {
        AddNoServer();
    }

    w = uii.Rend_GetShaderWidth(m_noservers_mat->GetMaterial() * m_vVirtualScale[0]);
    h = uii.Rend_GetShaderHeight(m_noservers_mat->GetMaterial() * m_vVirtualScale[1]);

    UIRect2D newFrame    = uWinMan.getFrame();
    x                    = (newFrame.pos.x + newFrame.size.width - w) * 0.5f;
    newFrame.pos.x       = x;
    newFrame.pos.y       = (newFrame.pos.y + newFrame.size.height - h) * 0.5f;
    newFrame.size.width  = w;
    newFrame.size.height = h;

    m_noservers_wid->setFrame(newFrame);
    m_noservers_wid->setShow(true);
    m_noservers_wid->BringToFrontPropogated();
}

void UILanGameList::Highlight(UIWidget *wid)
{
    int i, j;
    int row = 0;

    for (i = 0; i < m_iCurrNumServers; i++, row++) {
        for (j = 0; j < m_iNumColumns; j++) {
            if (m_widgetlist.ObjectAt(i + j * m_iCurrNumServers + 1) == wid) {
                m_activerow  = row;
                m_activeitem = j;
            }
        }
    }
}

void UILanGameList::Connect(void)
{
    if (m_activerow < 0 || m_activerow >= m_iCurrNumServers) {
        return;
    }

    // FIXME: add IPv6 support
    uii.Connect(NET_AdrToString(m_servers.ObjectAt(m_activerow + 1)->adr), netadrtype_t::NA_IP);
}

void UILanGameList::EventConnect(Event *ev)
{
    Connect();
}

qboolean UILanGameList::SetActiveRow(UIWidget *w)
{
    int      i, k;
    int      row = 0;
    qboolean ret1, ret2;
    int      new_activerow  = -1;
    int      new_activeitem = -1;

    for (i = 0; i < m_iCurrNumServers; i++, row++) {
        for (k = 0; k < m_iNumColumns; k++) {
            if (m_widgetlist.ObjectAt(i + k * m_iCurrNumServers + 1) == w) {
                new_activerow  = row;
                new_activeitem = k;
            }
        }
    }

    ret1 = new_activerow == m_activerow;
    ret2 = new_activeitem == m_activeitem;

    if (new_activerow != -1) {
        m_activerow = new_activerow;
    }
    if (new_activeitem != -1) {
        m_activeitem = new_activeitem;
    }

    if (ret1) {
        return ret2;
    }
    return qfalse;
}

void UILanGameList::UpdateServers(void)
{
    int i;

    if (!m_noservers_wid) {
        AddNoServer();
    }

    setShow(false);

    if (m_iPrevNumServers != cls.numlocalservers) {
        m_iPrevNumServers = cls.numlocalservers;
        DestroyServerWidgets();
        m_iCurrNumServers = 0;

        for (i = 0; i < cls.numlocalservers; i++, m_iCurrNumServers++) {
            m_servers.AddUniqueObject(&cls.localServers[i]);
        }

        m_Vscroll->setPageHeight((getSize().height - 16) / 16.f);
        m_Vscroll->setNumItems(m_iCurrNumServers);
        m_Vscroll->InitFrameAlignRight(this, 0, 0);
        CreateServerWidgets();
    } else if (!m_iPrevNumServers) {
        DrawNoServers(getFrame());
    }
}

void UILanGameList::Draw(void)
{
    UIWidget *wid;
    int       i, j, k;
    int       row          = 0;
    qboolean  iColorToggle = qfalse;
    int       topRow;

    UpdateServers();

    if (!m_iCurrNumServers) {
        // nothing to draw
        return;
    }

    topRow = m_Vscroll->getTopItem();
    DrawNoServers(getFrame());

    for (i = 0; i < m_iCurrNumServers; i++, row++) {
        iColorToggle ^= 1;

        if (m_activerow == row) {
            for (k = 0; k < m_iNumColumns; k++) {
                wid = m_widgetlist.ObjectAt(m_activerow + k * m_iCurrNumServers + 1);

                wid->setForegroundColor(UColor(0.9f, 0.8f, 0.6f, 1.f));
                wid->setBackgroundColor(UColor(0.21f, 0.18f, 0.015f, 0.6f), true);

                if (iColorToggle) {
                    wid->setBorderColor(UColor(0, 0, 0, 0));
                } else {
                    wid->setBorderColor(UColor(0, 0, 0, 1));
                }
            }
        }

        for (j = 0; j < m_iNumColumns; j++) {
            UIRect2D frame;

            wid = m_widgetlist.ObjectAt(i + j * m_iCurrNumServers + 1);

            frame = wid->getFrame();

            if (m_activerow != row) {
                if (iColorToggle) {
                    wid->setForegroundColor(UColor(0.07f, 0.06f, 0.005f, 1));
                    wid->setBackgroundColor(UColor(0.07f, 0.06f, 0.005f, 1), true);
                } else {
                    wid->setForegroundColor(UColor(0.21f, 0.18f, 0.015f, 1));
                    wid->setBackgroundColor(UColor(0.21f, 0.18f, 0.015f, 1), true);
                }
            }

            if (row < topRow) {
                wid->setShow(false);
            } else {
                frame.pos.y = ((row - topRow) * 16 + 28) * m_vVirtualScale[1];
                wid->setShow(true);
            }

            if (getSize().height - m_vVirtualScale[1] * 12 <= frame.pos.y) {
                wid->setShow(false);
            }
            wid->setFrame(frame);
            wid->Realign();
        }
    }
}

void UILanGameList::Realign(void)
{
    qboolean bVirtualChanged = qfalse;

    if (m_bVirtual) {
        vec2_t vNewVirtualScale;

        vNewVirtualScale[0] = uid.vidWidth / 640.0;
        vNewVirtualScale[1] = uid.vidHeight / 480.0;
        bVirtualChanged     = VectorCompare2D(m_vVirtualScale, vNewVirtualScale) == 0;
    }

    UIWidget::Realign();

    if (m_bVirtual) {
        Event *event = new Event(EV_Layout_VirtualRes);
        event->AddInteger(1);
        m_Vscroll->ProcessEvent(event);
    }

    if (bVirtualChanged) {
        m_Vscroll->Realign();

        m_Vscroll->setPageHeight((getSize().height / m_vVirtualScale[1] - 24) / 16);
        m_Vscroll->InitFrameAlignRight(this, 4, 4);
        RepositionServerWidgets();
    }
}

void UILanGameList::EventScanNetwork(Event *ev) {}

void UILanGameList::EventScaningNetwork(Event *ev) {}

bool UILanGameList::isDying(void)
{
    return m_dying;
}

void UILanGameList::PlayEnterSound(void) {}

CLASS_DECLARATION(UILabel, UILanGameListLabel, NULL) {
    {&W_LeftMouseDown, &UILanGameListLabel::Pressed  },
    {&W_LeftMouseUp,   &UILanGameListLabel::Unpressed},
    {NULL,             NULL                          }
};

UILanGameListLabel::UILanGameListLabel()
{
    m_list             = NULL;
    m_iLastPressedTime = 0;
}

UILanGameListLabel::UILanGameListLabel(UILanGameList *list)
{
    m_list             = list;
    m_iLastPressedTime = 0;
}

void UILanGameListLabel::Pressed(Event *ev)
{
    if (m_list) {
        m_list->Highlight(this);

        if (uii.Sys_Milliseconds() - m_iLastPressedTime < 200) {
            m_list->Connect();
        }
    }
}

void UILanGameListLabel::Unpressed(Event *ev)
{
    if (m_list) {
        m_iLastPressedTime = uii.Sys_Milliseconds();
    }
}
