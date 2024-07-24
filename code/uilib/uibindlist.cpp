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

#include "ui_local.h"
#include "../qcommon/localization.h"

Event EV_UIFakkBindList_Filename
(
    "filename",
    EV_DEFAULT,
    "s",
    "filename",
    "Filename that holds bind definitions"
);

Event EV_UIFakkBindList_StopBind
(
    "stopbind",
    EV_DEFAULT,
    NULL,
    NULL,
    "stops trying to bind a key to a command"
);

CLASS_DECLARATION(UIWidget, UIFakkBindList, NULL) {
    {&EV_UIFakkBindList_Filename, &UIFakkBindList::Filename},
    {&EV_UIFakkBindList_StopBind, &UIFakkBindList::StopBind},
    {NULL,                        NULL                     }
};

UIFakkBindList::UIFakkBindList()
{
    m_scroll  = new UIVertScroll();
    m_bind    = NULL;
    m_created = false;
    AllowActivate(true);
    m_activerow    = 0;
    m_activeitem   = 2;
    m_presskey_wid = NULL;
    m_presskey_mat = uWinMan.RegisterShader("textures/menu/presskey");
}

UIFakkBindList::~UIFakkBindList()
{
    m_dying = true;
    if (m_bind) {
        delete m_bind;
    }
}

void UIFakkBindList::Filename(Event *ev)
{
    m_bind = new bind_t();
    CL_LoadBind(ev->GetString(1), m_bind);

    m_scroll->setPageHeight((getSize().height - m_bind->commandheight) / m_bind->commandheight);
    m_scroll->setNumItems(m_bind->binds.NumObjects());
    m_scroll->InitFrameAlignRight(this, 0, 0);

    CreateBindWidgets();
}

void UIFakkBindList::StopBind(Event *ev)
{
    if (uWinMan.BindActive()) {
        uWinMan.BindActive()->KeyEvent(K_ESCAPE, 0);
    }

    m_presskey_wid->setShow(false);
}

void UIFakkBindList::FrameInitialized(void) {}

void UIFakkBindList::CreateBindWidgets(void)
{
    UIFakkBindListLabel *listlabel;
    UILabel             *label;
    int                  i;

    if (m_created || !m_bind) {
        return;
    }

    Event evVirtualRes(EV_Layout_VirtualRes);
    evVirtualRes.AddInteger(1);

    //
    // List label 1
    //
    listlabel = new UIFakkBindListLabel(NULL);
    m_miscwidgets.AddObject(listlabel);
    if (m_bVirtual) {
        listlabel->ProcessEvent(evVirtualRes);
    }

    listlabel->InitFrame(
        this, 0, 0, m_bind->commandwidth * m_vVirtualScale[0], m_bind->commandheight * m_vVirtualScale[1], 1
    );
    listlabel->setName(m_bind->headers[0]);
    listlabel->setMaterial(uWinMan.RegisterShader("textures/bind/emptylabel"));
    listlabel->setTitle(m_bind->headers[0]);
    listlabel->setForegroundColor(m_bind->titlefgcolor);
    listlabel->setBackgroundColor(m_bind->titlebgcolor, true);

    //
    // List label 2
    //
    listlabel = new UIFakkBindListLabel(NULL);
    m_miscwidgets.AddObject(listlabel);
    if (m_bVirtual) {
        listlabel->ProcessEvent(evVirtualRes);
    }

    listlabel->InitFrame(
        this,
        m_bind->primarykeywidth * m_vVirtualScale[0],
        0,
        m_bind->primarykeywidth * m_vVirtualScale[0],
        m_bind->commandheight * m_vVirtualScale[1],
        1
    );
    listlabel->setName(m_bind->headers[1]);
    listlabel->setMaterial(uWinMan.RegisterShader("textures/bind/emptykey"));
    listlabel->setTitle(m_bind->headers[1]);
    listlabel->setForegroundColor(m_bind->titlefgcolor);
    listlabel->setBackgroundColor(m_bind->titlebgcolor, true);

    //
    // List label 3
    //
    listlabel = new UIFakkBindListLabel(NULL);
    m_miscwidgets.AddObject(listlabel);
    if (m_bVirtual) {
        listlabel->ProcessEvent(evVirtualRes);
    }

    listlabel->InitFrame(
        this,
        (m_bind->fillwidth * 2 + m_bind->commandwidth + m_bind->primarykeywidth) * m_vVirtualScale[0],
        0,
        m_bind->alternatekeywidth * m_vVirtualScale[0],
        m_bind->commandheight * m_vVirtualScale[1],
        1
    );
    listlabel->setName(m_bind->headers[2]);
    listlabel->setMaterial(uWinMan.RegisterShader("textures/bind/emptykey"));
    listlabel->setTitle(m_bind->headers[2]);
    listlabel->setForegroundColor(m_bind->titlefgcolor);
    listlabel->setBackgroundColor(m_bind->titlebgcolor, true);

    if (m_bind->fillwidth) {
        //
        // Fill label 1
        //
        label = new UILabel();
        m_miscwidgets.AddObject(label);

        if (m_bVirtual) {
            label->ProcessEvent(evVirtualRes);
        }

        label->InitFrame(
            this,
            (m_bind->commandwidth) * m_vVirtualScale[0],
            0,
            m_bind->fillwidth * m_vVirtualScale[0],
            m_frame.size.height * m_vVirtualScale[1],
            0
        );
        label->setMaterial(m_bind->fillmaterial);
        label->setShow(true);

        //
        // Fill label 2
        //
        label = new UILabel();
        m_miscwidgets.AddObject(label);

        if (m_bVirtual) {
            label->ProcessEvent(evVirtualRes);
        }

        label->InitFrame(
            this,
            (m_bind->fillwidth + m_bind->commandwidth + m_bind->primarykeywidth) * m_vVirtualScale[0],
            0,
            m_bind->fillwidth * m_vVirtualScale[0],
            m_frame.size.height * m_vVirtualScale[1],
            0
        );
        label->setMaterial(m_bind->fillmaterial);
        label->setShow(true);

        //
        // Fill label 3
        //
        label = new UILabel();
        m_miscwidgets.AddObject(label);

        if (m_bVirtual) {
            label->ProcessEvent(evVirtualRes);
        }

        label->InitFrame(
            this,
            (m_bind->fillwidth * 2 + m_bind->alternatekeywidth + m_bind->commandwidth + m_bind->primarykeywidth)
                * m_vVirtualScale[0],
            0,
            m_bind->fillwidth * m_vVirtualScale[0],
            m_frame.size.height * m_vVirtualScale[1],
            0
        );
        label->setMaterial(m_bind->fillmaterial);
        label->setShow(true);
    }

    for (i = 1; i <= m_bind->binds.NumObjects(); i++) {
        UIBindButton *button;
        bind_item_t  *bi;

        //
        // Label
        //
        listlabel = new UIFakkBindListLabel(this);
        bi        = m_bind->binds.ObjectAt(i);

        if (m_bVirtual) {
            listlabel->ProcessEvent(evVirtualRes);
        }

        listlabel->InitFrame(
            this,
            0,
            (m_bind->commandheight * i) * m_vVirtualScale[1],
            m_bind->commandwidth * m_vVirtualScale[0],
            m_bind->commandheight * m_vVirtualScale[1],
            0
        );
        listlabel->setName(bi->name);
        listlabel->setForegroundColor(m_bind->inactivefgcolor);
        listlabel->setBackgroundColor(m_bind->inactivebgcolor, true);
        listlabel->setMaterial(uWinMan.RegisterShader("textures/bind/emptylabel"));
        listlabel->setTitle(bi->name);
        m_widgetlist.AddObject(listlabel);

        //
        // Button - bind
        //
        button = new UIBindButton(m_bind->entersound, m_bind->activesound);
        if (m_bVirtual) {
            button->ProcessEvent(evVirtualRes);
        }
        button->SetCommand(bi->command);

        button->InitFrame(
            this,
            (m_bind->commandwidth + m_bind->fillwidth) * m_vVirtualScale[0],
            (m_bind->commandheight * i) * m_vVirtualScale[1],
            m_bind->primarykeywidth * m_vVirtualScale[0],
            m_bind->primarykeyheight * m_vVirtualScale[1],
            0
        );
        button->setForegroundColor(m_bind->inactivefgcolor);
        button->setBackgroundColor(m_bind->inactivebgcolor, true);
        m_widgetlist.AddObject(button);

        //
        // Button - alternate bind
        //
        button = new UIBindButton(m_bind->entersound, m_bind->activesound);
        if (m_bVirtual) {
            button->ProcessEvent(evVirtualRes);
        }
        button->SetCommand(bi->command);
        button->SetAlternate(true);

        button->InitFrame(
            this,
            (m_bind->fillwidth * 2 + m_bind->commandwidth + m_bind->primarykeywidth) * m_vVirtualScale[0],
            (m_bind->commandheight * i) * m_vVirtualScale[1],
            m_bind->alternatekeywidth * m_vVirtualScale[0],
            m_bind->alternatekeyheight * m_vVirtualScale[1],
            0
        );
        button->setForegroundColor(m_bind->inactivefgcolor);
        button->setBackgroundColor(m_bind->inactivebgcolor, true);
        m_widgetlist.AddObject(button);
    }

    m_presskey_wid = new UILabel();
    m_miscwidgets.AddObject(m_presskey_wid);
    if (m_bVirtual) {
        m_presskey_wid->ProcessEvent(evVirtualRes);
    }

    m_presskey_wid->setMaterial(m_presskey_mat);
    m_presskey_wid->InitFrame(NULL, 0, 0, 100, 100, 0);
    m_presskey_wid->setShow(false);

    // Finished creation
    m_created = true;
}

void UIFakkBindList::DestroyBindWidgets(void)
{
    int       i;
    UIWidget *pWidget;

    if (!m_created) {
        return;
    }

    for (i = 1; i <= m_widgetlist.NumObjects(); i++) {
        pWidget = m_widgetlist.ObjectAt(i);
        if (pWidget) {
            delete pWidget;
        }
    }

    m_widgetlist.ClearObjectList();

    for (i = 1; i <= m_miscwidgets.NumObjects(); i++) {
        pWidget = m_miscwidgets.ObjectAt(i);
        if (pWidget) {
            delete pWidget;
        }
    }

    m_miscwidgets.ClearObjectList();

    m_presskey_wid = NULL;
    m_created      = false;
}

void UIFakkBindList::RepositionBindWidgets(void)
{
    int       i;
    UIWidget *pWidget;

    Event evVirtualRes(EV_Layout_VirtualRes);
    evVirtualRes.AddInteger(1);

    //
    // Index 1
    //
    pWidget = m_miscwidgets.ObjectAt(1);
    if (m_bVirtual) {
        pWidget->ProcessEvent(evVirtualRes);
        pWidget->Realign();
    }

    pWidget->setFrame(
        UIRect2D(0, 0, m_bind->commandwidth * m_vVirtualScale[0], m_bind->commandheight * m_vVirtualScale[1])
    );

    //
    // Index 2
    //
    pWidget = m_miscwidgets.ObjectAt(2);
    if (m_bVirtual) {
        pWidget->ProcessEvent(evVirtualRes);
        pWidget->Realign();
    }

    pWidget->setFrame(UIRect2D(
        (m_bind->commandwidth + m_bind->fillwidth) * m_vVirtualScale[0],
        0,
        m_bind->primarykeywidth * m_vVirtualScale[0],
        m_bind->commandheight * m_vVirtualScale[1]
    ));

    //
    // Index 3
    //
    pWidget = m_miscwidgets.ObjectAt(3);
    if (m_bVirtual) {
        pWidget->ProcessEvent(evVirtualRes);
        pWidget->Realign();
    }
    pWidget->setFrame(UIRect2D(
        (m_bind->fillwidth * 2 + m_bind->commandwidth + m_bind->primarykeywidth) * m_vVirtualScale[0],
        0,
        m_bind->alternatekeywidth * m_vVirtualScale[0],
        m_bind->commandheight * m_vVirtualScale[1]
    ));

    if (m_bind->fillwidth) {
        //
        // Index 4
        //
        pWidget = m_miscwidgets.ObjectAt(4);
        if (m_bVirtual) {
            pWidget->ProcessEvent(evVirtualRes);
            pWidget->Realign();
        }

        pWidget->setFrame(UIRect2D(
            m_bind->commandwidth * m_vVirtualScale[0], 0, m_bind->fillwidth * m_vVirtualScale[0], m_frame.size.height
        ));

        //
        // Index 5
        //
        pWidget = m_miscwidgets.ObjectAt(5);
        if (m_bVirtual) {
            pWidget->ProcessEvent(evVirtualRes);
            pWidget->Realign();
        }

        pWidget->setFrame(UIRect2D(
            (m_bind->fillwidth + m_bind->commandwidth + m_bind->primarykeywidth) * m_vVirtualScale[0],
            0,
            m_bind->fillwidth * m_vVirtualScale[0],
            m_frame.size.height
        ));

        //
        // Index 6
        //
        pWidget = m_miscwidgets.ObjectAt(6);
        if (m_bVirtual) {
            pWidget->ProcessEvent(evVirtualRes);
            pWidget->Realign();
        }

        pWidget->setFrame(UIRect2D(
            (m_bind->fillwidth * 2 + m_bind->alternatekeywidth + m_bind->commandwidth + m_bind->primarykeywidth)
                * m_vVirtualScale[0],
            0,
            m_bind->fillwidth * m_vVirtualScale[0],
            m_frame.size.height
        ));
    }

    for (i = 0; i < m_bind->binds.NumObjects(); i++) {
        //
        // Subindex 1
        //
        pWidget = m_widgetlist.ObjectAt(i * 3 + 1);
        if (m_bVirtual) {
            pWidget->ProcessEvent(evVirtualRes);
            pWidget->Realign();
        }

        pWidget->setFrame(UIRect2D(
            0,
            (m_bind->commandheight * i) * m_vVirtualScale[1],
            m_bind->commandwidth * m_vVirtualScale[0],
            m_bind->commandheight * m_vVirtualScale[1]
        ));

        //
        // Subindex 2
        //
        pWidget = m_widgetlist.ObjectAt(i * 3 + 2);
        if (m_bVirtual) {
            pWidget->ProcessEvent(evVirtualRes);
            pWidget->Realign();
        }

        pWidget->setFrame(UIRect2D(
            (m_bind->commandwidth + m_bind->fillwidth) * m_vVirtualScale[0],
            (m_bind->commandheight * i) * m_vVirtualScale[1],
            m_bind->primarykeywidth * m_vVirtualScale[0],
            m_bind->primarykeyheight * m_vVirtualScale[1]
        ));

        //
        // Subindex 3
        //
        pWidget = m_widgetlist.ObjectAt(i * 3 + 3);
        if (m_bVirtual) {
            pWidget->ProcessEvent(evVirtualRes);
            pWidget->Realign();
        }

        pWidget->setFrame(UIRect2D(
            (m_bind->fillwidth * 2 + m_bind->commandwidth + m_bind->primarykeywidth) * m_vVirtualScale[0],
            (m_bind->commandheight * i) * m_vVirtualScale[1],
            m_bind->alternatekeywidth * m_vVirtualScale[0],
            m_bind->alternatekeyheight * m_vVirtualScale[1]
        ));
    }
}

qboolean UIFakkBindList::KeyEvent(int key, unsigned int time)
{
    UIWidget *wid;

    switch (key) {
    case K_ENTER:
    case K_KP_ENTER:
        wid = m_widgetlist.ObjectAt(m_activeitem + m_activerow * 3);
        if (wid->isSubclassOf(UIBindButton)) {
            static_cast<UIBindButton *>(wid)->Pressed();
            uii.Snd_PlaySound(m_bind->activesound);
        }
        return true;
    case K_BACKSPACE:
    case K_DEL:
        wid = m_widgetlist.ObjectAt(m_activeitem + m_activerow * 3);
        if (wid->isSubclassOf(UIBindButton)) {
            static_cast<UIBindButton *>(wid)->Clear();
            uii.Snd_PlaySound(m_bind->activesound);
        }
        return true;
    case K_MWHEELUP:
        m_scroll->AttemptScrollTo(m_scroll->getTopItem() - 1);
        uii.Snd_PlaySound(m_bind->changesound);
        return true;
    case K_MWHEELDOWN:
        m_scroll->AttemptScrollTo(m_scroll->getTopItem() + 1);
        uii.Snd_PlaySound(m_bind->changesound);
        return true;
    case K_RIGHTARROW:
        m_activeitem++;
        if (m_activeitem > 3) {
            m_activeitem = 3;
        }
        uii.Snd_PlaySound(m_bind->changesound);
        return true;
    case K_LEFTARROW:
        m_activeitem--;
        if (m_activeitem < 2) {
            m_activeitem = 2;
        }
        uii.Snd_PlaySound(m_bind->changesound);
        return true;
    case K_UPARROW:
        if (!m_activerow) {
            return true;
        }
        if (m_activerow == m_scroll->getTopItem()) {
            m_scroll->AttemptScrollTo(m_activerow - 1);
            m_activerow = m_scroll->getTopItem();
        } else {
            m_activerow--;
        }
        uii.Snd_PlaySound(m_bind->changesound);
        return true;
    case K_DOWNARROW:
        if (m_activerow + 1 >= m_scroll->getNumItems()) {
            return true;
        }

        if (m_activerow - m_scroll->getTopItem() + 1 >= m_scroll->getPageHeight()) {
            if (m_scroll->AttemptScrollTo(m_activerow + 1)) {
                m_activerow++;
            }
            return true;
        }

        if (m_activerow == m_scroll->getPageHeight() + m_scroll->getTopItem()) {
            m_scroll->AttemptScrollTo(m_scroll->getTopItem() + 1);
            m_activerow = m_scroll->getTopItem() + m_scroll->getPageHeight();
        } else {
            m_activerow++;
        }

        uii.Snd_PlaySound(m_bind->changesound);
        return true;
    }

    return false;
}

void UIFakkBindList::DrawPressKey(UIRect2D frame)
{
    float x;
    float w, h;

    if (!m_presskey_mat) {
        return;
    }

    w = uii.Rend_GetShaderWidth(m_presskey_mat->GetMaterial()) * m_vVirtualScale[0];
    h = uii.Rend_GetShaderHeight(m_presskey_mat->GetMaterial()) * m_vVirtualScale[1];

    UIRect2D newFrame    = uWinMan.getFrame();
    x                    = (newFrame.pos.x + newFrame.size.width - w) * 0.5;
    newFrame.pos.x       = x;
    newFrame.pos.y       = (newFrame.pos.y + newFrame.size.height - h) * 0.5;
    newFrame.size.width  = w;
    newFrame.size.height = h;

    m_presskey_wid->setFrame(newFrame);
    m_presskey_wid->setShow(true);
    m_presskey_wid->BringToFrontPropogated();
}

void UIFakkBindList::Highlight(UIWidget *wid)
{
    int i, j;
    int row = 0;

    for (i = 0; i < m_bind->binds.NumObjects(); i++, row++) {
        for (j = 1; j <= 3; j++) {
            if (m_widgetlist.ObjectAt(j + i * 3) == wid) {
                m_activerow  = row;
                m_activeitem = j;
            }
        }
    }

    uii.Snd_PlaySound(m_bind->changesound);
}

qboolean UIFakkBindList::SetActiveRow(UIWidget *w)
{
    int      i, k;
    int      row = 0;
    qboolean ret1, ret2;
    int      new_activerow = -1, new_activeitem = -1;

    for (i = 0; i < m_bind->binds.NumObjects(); i++, row++) {
        for (k = 1; k <= 3; k++) {
            if (m_widgetlist.ObjectAt(k + i * 3) == w) {
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

void UIFakkBindList::Draw(void)
{
    UIWidget *wid;
    UIWidget *bindwid;
    int       i, j, k;
    int       row = 0, topRow;

    if (!m_bind) {
        return;
    }

    topRow = m_scroll->getTopItem();
    if (m_presskey_wid) {
        m_presskey_wid->setShow(false);
    }

    bindwid = uWinMan.BindActive();
    if (bindwid) {
        for (i = 0; i < m_bind->binds.NumObjects(); i++, row++) {
            for (k = 1; k <= 3; k++) {
                if (m_widgetlist.ObjectAt(k + i * 3) == bindwid) {
                    m_activerow  = row;
                    m_activeitem = k;
                    DrawPressKey(bindwid->getFrame());
                }
            }
        }
    }

    for (i = 0, row = 0; i < m_bind->binds.NumObjects(); i++, row++) {
        if (m_activerow == row) {
            for (k = 1; k <= 3; k++) {
                UIWidget *hwid = m_widgetlist.ObjectAt(k + m_activerow * 3);

                hwid->setForegroundColor(m_bind->highlightfgcolor);
                hwid->setBackgroundColor(m_bind->highlightbgcolor, true);

                if (m_activeitem != k) {
                    hwid->setBorderColor(m_bind->inactivebgcolor);
                } else if (!uWinMan.BindActive()) {
                    hwid->setForegroundColor(m_bind->selectfgcolor);
                    hwid->setBackgroundColor(m_bind->selectbgcolor, true);
                } else {
                    hwid->setForegroundColor(m_bind->activefgcolor);
                    hwid->setBackgroundColor(m_bind->activebgcolor, true);
                    hwid->setBorderColor(m_bind->activebordercolor);
                }
            }
        }

        for (j = 1; j <= 3; j++) {
            wid = m_widgetlist.ObjectAt(j + i * 3);

            UIRect2D rect = wid->getFrame();

            if (m_activerow != row) {
                wid->setForegroundColor(m_bind->inactivefgcolor);
                wid->setBackgroundColor(m_bind->inactivebgcolor, true);
            }

            if (row < topRow) {
                wid->setShow(false);
            } else {
                rect.pos.y = m_bind->commandheight * (row - topRow);
                rect.pos.y = (rect.pos.y + m_bind->commandheight) * m_vVirtualScale[1];
                wid->setShow(true);
            }

            if (rect.pos.y >= getSize().height) {
                wid->setShow(false);
            }

            wid->setFrame(rect);
        }
    }
}

void UIFakkBindList::Realign(void)
{
    qboolean bVirtualChanged = qfalse;

    if (m_bVirtual) {
        float vNewVirtualScale[2];

        vNewVirtualScale[0] = uid.vidWidth / 640.f;
        vNewVirtualScale[1] = uid.vidHeight / 480.f;
        bVirtualChanged     = VectorCompare2D(m_vVirtualScale, vNewVirtualScale) == 0;
    }

    UIWidget::Realign();

    if (m_bVirtual) {
        Event *event = new Event(EV_Layout_VirtualRes);
        event->AddInteger(1);
        m_scroll->ProcessEvent(event);
    }

    if (bVirtualChanged) {
        m_scroll->Realign();
    }

    m_scroll->setPageHeight((getSize().height / m_vVirtualScale[1] - m_bind->commandheight) / m_bind->commandheight);

    m_scroll->InitFrameAlignRight(this, 0, 0);
    RepositionBindWidgets();
}

void UIFakkBindList::setBind(bind_t *b)
{
    m_bind = b;
}

bool UIFakkBindList::isDying(void)
{
    return m_dying;
}

void UIFakkBindList::PlayEnterSound(void)
{
    uii.Snd_PlaySound(m_bind->entersound);
}

CLASS_DECLARATION(UILabel, UIFakkBindListLabel, NULL) {
    {&W_LeftMouseDown, &UIFakkBindListLabel::Pressed},
    {NULL,             NULL                         }
};

UIFakkBindListLabel::UIFakkBindListLabel()
{
    m_list = NULL;
}

UIFakkBindListLabel::UIFakkBindListLabel(UIFakkBindList *list)
{
    m_list = list;
}

void UIFakkBindListLabel::Pressed(Event *ev)
{
    if (m_list) {
        m_list->Highlight(this);
    }
}

void UIFakkBindListLabel::Draw(void)
{
    if (!m_title.length()) {
        return;
    }

    m_font->setColor(m_background_color);

    UIRect2D frame = getClientFrame();
    m_font->PrintJustified(
        getClientFrame(),
        m_iFontAlignmentHorizontal,
        m_iFontAlignmentVertical,
        Sys_LV_CL_ConvertString(m_title),
        m_bVirtual ? m_vVirtualScale : NULL
    );
}
