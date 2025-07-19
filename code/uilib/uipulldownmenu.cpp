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

#include "../qcommon/localization.h"
#include "ui_local.h"

Event EV_Pulldown_ChildKilled
(
    "_ev_pulldown_child_killed",
    EV_DEFAULT,
    NULL,
    NULL,
    "Signaled when the sub menu is killed for one reason or another"
);

Event EV_Pulldown_HighlightBGColor
(
    "highlight_bgcolor",
    EV_DEFAULT,
    "fff",
    "r g b",
    "Set the background color of the highlighted text when a pulldown is used"
);

Event EV_Pulldown_HighlightFGColor
(
    "highlight_fgcolor",
    EV_DEFAULT,
    "fff",
    "r g b",
    "Set the foreground color of the highlighted text when a pulldown is used"
);

CLASS_DECLARATION(UIWidget, UIPulldownMenu, NULL) {
    {&W_LeftMouseDown,              &UIPulldownMenu::MousePressed    },
    {&W_LeftMouseUp,                &UIPulldownMenu::MouseReleased   },
    {&W_LeftMouseDragged,           &UIPulldownMenu::MouseDragged    },
    {&EV_Pulldown_ChildKilled,      &UIPulldownMenu::ChildKilled     },
    {&EV_Pulldown_HighlightBGColor, &UIPulldownMenu::HighlightBGColor},
    {&EV_Pulldown_HighlightFGColor, &UIPulldownMenu::HighlightFGColor},
    {NULL,                          NULL                             }
};

static UIPopupMenu *MenuFromPoint(UIPoint2D& p)
{
    UIWidget *res = uWinMan.FindResponder(p);
    if (res && res->isSubclassOf(UIPopupMenu)) {
        return (UIPopupMenu *)res;
    }

    return NULL;
}

UIPulldownMenu::UIPulldownMenu()
{
    m_listener           = NULL;
    m_submenu            = -1;
    m_highlightBGColor.r = 0.02f;
    m_highlightBGColor.g = 0.07f;
    m_highlightBGColor.b = 0.005f;
    m_highlightBGColor.a = 1.0f;
    m_highlightFGColor   = (UColor)UHudColor;

    // uninitialized in original game
    m_submenuptr = NULL;

    // Added in OPM
    //  This is so the window manager knows it can be activated
    //  and appropriately handle this as the first responder
    m_canactivate = true;
}

UIPulldownMenu::~UIPulldownMenu()
{
    for (int i = m_desc.NumObjects(); i > 0; i--) {
        uipull_describe *updb = m_desc.ObjectAt(i);
        // FIXME: possible OG bug, memory leak? updb->desc is not cleared
        m_desc.RemoveObjectAt(i);
        delete updb;
    }
}

UIRect2D UIPulldownMenu::getAlignmentRect(UIWidget *parent)
{
    if (!parent) {
        parent = (m_parent == NULL) ? &uWinMan : m_parent;
    }

    UIRect2D parentRect = parent->getClientFrame();
    if (m_bVirtual) {
        m_vVirtualScale[0] = (float)uid.vidWidth / SCREEN_WIDTH;
        m_vVirtualScale[1] = (float)uid.vidHeight / SCREEN_HEIGHT;
    }

    int maxheight = m_font->getHeight(m_bVirtual ? m_vVirtualScale : NULL);
    for (int i = 1; i <= m_desc.NumObjects(); i++) {
        uipull_describe *uipd = m_desc.ObjectAt(i);
        if (uipd && uipd->material) {
            int h = uii.Rend_GetShaderHeight(uipd->material->GetMaterial());
            if (h > maxheight) {
                maxheight = h;
            }
        }
    }

    return UIRect2D(parentRect.pos.x, parentRect.pos.y, parentRect.size.width, (maxheight + 4) * m_vVirtualScale[1]);
}

float UIPulldownMenu::getDescWidth(uipull_describe *desc)
{
    UIReggedMaterial *mat = desc->material;
    if (mat) {
        return uii.Rend_GetShaderWidth(mat->GetMaterial());
    }

    return m_font->getWidth(desc->title, -1) + 8.0f;
}

float UIPulldownMenu::getDescHeight(uipull_describe *desc)
{
    UIReggedMaterial *mat = desc->material;
    if (mat) {
        return uii.Rend_GetShaderHeight(mat->GetMaterial());
    }

    return m_font->getHeight(m_bVirtual ? m_vVirtualScale : NULL);
}

uipull_describe *UIPulldownMenu::getPulldown(str title)
{
    for (int i = 1; i <= m_desc.NumObjects(); i++) {
        uipull_describe *pd = m_desc.ObjectAt(i);
        if (pd->title == title) {
            return pd;
        }
    }

    return NULL;
}

void UIPulldownMenu::HighlightBGColor(Event *ev)
{
    m_highlightBGColor.r = ev->GetFloat(1);
    m_highlightBGColor.g = ev->GetFloat(2);
    m_highlightBGColor.b = ev->GetFloat(3);
    m_highlightBGColor.a = 1.0f;
}

void UIPulldownMenu::HighlightFGColor(Event *ev)
{
    m_highlightFGColor.r = ev->GetFloat(1);
    m_highlightFGColor.g = ev->GetFloat(2);
    m_highlightFGColor.b = ev->GetFloat(3);
    m_highlightFGColor.a = 1.0f;
}

void UIPulldownMenu::Create(UIWidget *parent, Listener *listener, const UIRect2D& rect)
{
    m_listener = listener;
    InitFrame(parent, rect.pos.x, rect.pos.y, rect.size.width, rect.size.height, 0);
}

void UIPulldownMenu::CreateAligned(UIWidget *parent, Listener *listener)
{
    setFont("verdana-12");
    UIRect2D rect = getAlignmentRect(parent);
    Create(parent, listener, rect);
}

void UIPulldownMenu::MousePressed(Event *ev)
{
    uWinMan.setFirstResponder(this);
    MouseDragged(ev);
}

void UIPulldownMenu::MouseDragged(Event *ev)
{
    int              newSubMenu = -1;
    UIRect2D         subRect;
    uipull_describe *subdesc = NULL;

    UIPoint2D point = MouseEventToClientPoint(ev);
    if (getClientFrame().contains(point)) {
        float atx = m_vVirtualScale[0] * 2;
        for (int i = 1; i <= m_desc.NumObjects(); i++) {
            uipull_describe *desc  = m_desc.ObjectAt(i);
            float            width = getDescWidth(desc) * m_vVirtualScale[0];
            if (atx <= point.x && atx + width > point.x) {
                newSubMenu          = i;
                subdesc             = desc;
                subRect.pos.x       = atx;
                subRect.pos.y       = 0.0f;
                subRect.size.width  = width;
                subRect.size.height = m_frame.size.height;
                break;
            }
            atx += width;
        }
    }

    if (newSubMenu == m_submenu || newSubMenu == -1) {
        if (m_submenu == -1) {
            return;
        }

        UIPoint2D cursorLocation(point.x + m_screenframe.pos.x, point.y + m_screenframe.pos.y);

        UIPopupMenu *menu = MenuFromPoint(cursorLocation);
        if (!menu) {
            UIPopupMenu *submenu = m_submenuptr;
            while (submenu->m_submenu != -1) {
                submenu = submenu->m_submenuptr;
            }

            submenu->MouseMoved(ev);
        } else {
            while (menu->m_parentMenu) {
                menu = menu->m_parentMenu;
            }
            if (menu == m_submenuptr) {
                menu->MouseMoved(ev);
            }
        }

        return;
    }

    if (m_submenu != -1) {
        m_submenuptr->Disconnect(this, W_Destroyed);
        if (m_submenuptr) {
            delete m_submenuptr;
        }
    }

    m_submenu    = newSubMenu;
    m_submenuptr = new UIPopupMenu();
    subRect.pos.x += m_screenframe.pos.x;
    subRect.pos.y += m_screenframe.pos.y;
    m_submenuptr->Create(&subdesc->desc, m_listener, subRect, UIP_WHERE_DOWN, m_bVirtual, -1.0f);
    m_submenuptr->Connect(this, W_Destroyed, EV_Pulldown_ChildKilled);
}

void UIPulldownMenu::MouseReleased(Event *ev)
{
    if (uWinMan.getFirstResponder() == this) {
        uWinMan.setFirstResponder(NULL);
        UIPoint2D    point(ev->GetFloat(1), ev->GetFloat(2));
        UIPopupMenu *menu = MenuFromPoint(point);
        if (menu) {
            menu->MouseReleased(ev);
        }
    }

    if (m_submenu != -1) {
        m_submenuptr->Disconnect(this, W_Destroyed);
        delete m_submenuptr;
        m_submenu = -1;
    }
}

void UIPulldownMenu::ChildKilled(Event *ev)
{
    if (m_submenu != -1) {
        m_submenu = -1;
    }

    if (uWinMan.getFirstResponder() == this) {
        uWinMan.setFirstResponder(NULL);
    }
}

void UIPulldownMenu::AddUIPopupDescribe(const char *title, uipopup_describe *d)
{
    if (!title) {
        return;
    }

    uipull_describe *pd = getPulldown(title);
    if (!pd) {
        pd = new uipull_describe(title, NULL, NULL);
        m_desc.AddObject(pd);
    }

    pd->desc.AddObject(d);
}

void UIPulldownMenu::setHighlightFGColor(UColor c)
{
    m_highlightFGColor = c;
}

void UIPulldownMenu::setHighlightBGColor(UColor c)
{
    m_highlightBGColor = c;
}

void UIPulldownMenu::setPopupHighlightFGColor(str menu, UColor c)
{
    uipull_describe *up = getPulldown(menu);
    if (up) {
        up->highlightFGColor = c;
    }
}

void UIPulldownMenu::setPopupHighlightBGColor(str menu, UColor c)
{
    uipull_describe *up = getPulldown(menu);
    if (up) {
        up->highlightBGColor = c;
    }
}

void UIPulldownMenu::setPopupFGColor(str menu, UColor c)
{
    uipull_describe *up = getPulldown(menu);
    if (up) {
        up->FGColor = c;
    }
}

void UIPulldownMenu::setPopupBGColor(str menu, UColor c)
{
    uipull_describe *up = getPulldown(menu);
    if (up) {
        up->BGColor = c;
    }
}

void UIPulldownMenu::setPulldownShader(const char *title, UIReggedMaterial *mat)
{
    if (!title) {
        return;
    }

    uipull_describe *pd = getPulldown(title);
    if (!pd) {
        pd = new uipull_describe(title, NULL, NULL);
        m_desc.AddObject(pd);
    }

    pd->material = mat;
    Realign();
}

void UIPulldownMenu::setSelectedPulldownShader(const char *title, UIReggedMaterial *mat)
{
    if (!title) {
        return;
    }

    uipull_describe *pd = getPulldown(title);
    if (!pd) {
        pd = new uipull_describe(title, NULL, NULL);
        m_desc.AddObject(pd);
    }

    pd->selected_material = mat;
    Realign();
}

void UIPulldownMenu::Realign(void)
{
    UIRect2D rect = getAlignmentRect(NULL);
    setFrame(rect);
}

void UIPulldownMenu::Draw(void)
{
    float atx = 0.0f;
    for (int i = 1; i <= m_desc.NumObjects(); i++) {
        uipull_describe *desc   = m_desc.ObjectAt(i);
        float            width  = getDescWidth(desc) * m_vVirtualScale[0];
        float            height = getDescHeight(desc) * m_vVirtualScale[1];

        // Set font color based on whether the current item is selected
        if (m_submenu == i) {
            // Selected item
            m_font->setColor(m_highlightFGColor);
            if (desc->selected_material) {
                uii.Rend_DrawPicStretched(
                    atx, 0.0f, width, height, 0.0f, 0.0f, 1.0f, 1.0f, desc->selected_material->GetMaterial()
                );
            } else {
                DrawBox(atx, 0.0f, width, m_frame.size.height, m_highlightBGColor, 1.0f);
            }
        } else {
            // Unselected item
            m_font->setColor(m_foreground_color);
        }

        if (desc->material) {
            if (m_submenu != i) {
                uii.Rend_DrawPicStretched(
                    atx, 0.0f, width, height, 0.0f, 0.0f, 1.0f, 1.0f, desc->material->GetMaterial()
                );
            }
        } else {
            const char *text      = Sys_LV_CL_ConvertString(desc->title);
            float       text_xpos = m_vVirtualScale[0] * 4.0f + atx;
            float       text_ypos = m_vVirtualScale[1] * 2.0f;
            m_font->Print(text_xpos, text_ypos, text, -1, m_bVirtual ? m_vVirtualScale : NULL);
        }

        atx += width;
    }
}
