/*
===========================================================================
Copyright (C) 2015-2024 the OpenMoHAA team

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
#include "localization.h"

CLASS_DECLARATION(UIWidget, UIPopupMenu, NULL) {
    {&W_LeftMouseUp, &UIPopupMenu::MouseReleased},
    {&W_MouseMoved,  &UIPopupMenu::MouseMoved   },
    {&W_MouseExited, &UIPopupMenu::MouseExited  },
    {&W_Deactivated, &UIPopupMenu::DismissEvent },
    {NULL,           NULL                       }
};

uipopup_type UI_PopupTypeStringToInt(str type)
{
    if (type == "event_string") {
        return UIP_EVENT_STRING;
    } else if (type == "command") {
        return UIP_CMD;
    } else if (type == "cvar") {
        return UIP_CVAR;
    } else if (type == "separator") {
        return UIP_SEPARATOR;
    }

    return UIP_NONE;
}

UIPopupMenu::UIPopupMenu()
{
    m_selected    = -1;
    m_listener    = NULL;
    m_parentMenu  = NULL;
    m_submenu     = -1;
    m_marlett     = UIFont("marlett");
    m_autodismiss = qtrue;
    m_describe    = NULL;
    m_submenuptr  = NULL;

    m_highlightFGColor = UColor(0.9f, 0.8f, 0.6f, 1.0f);
    m_highlightBGColor = UColor(0.21f, 0.18f, 0.015f, 1.0f);
    m_background_color = UColor(0.02f, 0.07f, 0.005f, 1.0f);
    m_foreground_color = UHudColor;

    m_iFontAlignmentHorizontal = FONT_JUSTHORZ_RIGHT;
}

UIPopupMenu::~UIPopupMenu()
{
    if (m_parentMenu) {
        m_submenu = -1;
    }

    if (m_submenu != -1 && m_submenuptr) {
        m_submenuptr->m_parentMenu = NULL;
        delete m_submenuptr;
    }
}

uipopup_describe *UIPopupMenu::getDescribeFromPoint(const UIPoint2D& p)
{
    // FIXME: macro or inline for this somewhere?
    if (p.y < 4.0f || p.x < 2.0f || m_frame.size.width - 2.0f <= p.x) {
        return NULL;
    }

    float top = getVirtualScale()[1] * 4.0f;
    for (int i = 1; i <= m_describe->NumObjects(); i++) {
        uipopup_describe *desc   = m_describe->ObjectAt(i);
        float             height = getDescribeHeight(desc);
        if (p.y >= top && p.y < top + height) {
            return desc;
        }

        top += height;
    }

    return NULL;
}

void UIPopupMenu::MakeSubMenu(void)
{
    UIRect2D place;
    for (int i = 1; i <= m_submenu; i++) {
        place.pos.y += getDescribeHeight(m_describe->ObjectAt(i));
    }

    place.size.height = getDescribeHeight(m_describe->ObjectAt(m_submenu)) + 8.0f;
    place.size.width  = m_frame.size.width - 40.0f;

    place.pos.x = 20.0f;
    if (place.size.width < 0.0f) {
        place.pos.x -= place.size.width * 0.5f;
        place.size.width = 0.0f;
    }

    place.pos.x += m_screenframe.pos.x;
    place.pos.y += m_screenframe.pos.y;

    UIPopupMenu *me = new UIPopupMenu();
    me->Create(
        (Container<uipopup_describe *> *)m_describe->ObjectAt(m_submenu)->data,
        m_listener,
        place,
        UIP_WHERE_RIGHT,
        qfalse,
        -1.0f
    );

    me->m_parentMenu = this;
    m_submenuptr     = me;
}

float UIPopupMenu::getDescribeHeight(uipopup_describe *d)
{
    if (d->type == UIP_SEPARATOR) {
        return 8.0f * getVirtualScale()[0];
    }

    return m_font->getHeight(getVirtualScale());
}

float UIPopupMenu::getDescribeWidth(uipopup_describe *d)
{
    return m_font->getWidth(d->title, -1) * getVirtualScale()[0];
}

bool UIPopupMenu::MouseInSubmenus(void)
{
    if (m_submenu == -1) {
        return false;
    }

    if (m_submenuptr->m_screenframe.contains(uid.mouseX, uid.mouseY)) {
        return true;
    }

    return m_submenuptr->MouseInSubmenus();
}

void UIPopupMenu::Create(Container<uipopup_describe *> *describe, Listener *listener, const UIPoint2D& where)
{
    Create(describe, listener, UIRect2D(where.x, where.y, 0.0f, 0.0f), UIP_WHERE_RIGHT, qfalse, -1.0f);
}

void UIPopupMenu::Create(
    Container<uipopup_describe *> *describe,
    Listener                      *listener,
    const UIRect2D&                createRect,
    uipopup_where                  where,
    qboolean                       bVirtualSize,
    float                          width
)
{
    m_describe = describe;
    m_listener = listener;
    setFont("verdana-12");

    if (bVirtualSize) {
        m_bVirtual         = qtrue;
        m_vVirtualScale[0] = (float)uid.vidWidth / SCREEN_WIDTH;
        m_vVirtualScale[1] = (float)uid.vidHeight / SCREEN_HEIGHT;
    }

    if (!(m_describe && m_listener)) {
        return;
    }

    UISize2D totalSize;
    for (int i = 1; i <= m_describe->NumObjects(); i++) {
        uipopup_describe *desc = m_describe->ObjectAt(i);
        totalSize.height += getDescribeHeight(desc);
        if (desc->type != UIP_SEPARATOR) {
            float newWidth = getDescribeWidth(desc);
            if (newWidth > totalSize.width) {
                totalSize.width = newWidth;
            }
        }
    }

    totalSize.height /= getVirtualScale()[1];

    if (width != -1.0f) {
        totalSize.width = width;
    }

    if (totalSize.width == 0.0f || totalSize.height == 0.0f) {
        return;
    }

    totalSize.width += 24.0f;
    totalSize.height += 8.0f;

    //if (m_bVirtual)
    //{
    totalSize.width *= getVirtualScale()[0];
    totalSize.height *= getVirtualScale()[1];
    //}

    UIPoint2D place;
    place.x = (where) ? createRect.pos.x : createRect.size.width + createRect.pos.x;
    place.y = (where) ? createRect.size.height + createRect.pos.y : createRect.pos.y;

    // FIXME: avoid code repetition, extract these checks below to an inline func

    // Adjust position of menu rect if it goes beyond the screen width
    if (place.x + totalSize.width > uid.vidWidth) {
        place.x = (where) ? createRect.size.width + createRect.pos.x : createRect.pos.x;

        if (place.x < totalSize.width) {
            place.x = std::fmax(0.0f, uid.vidWidth - totalSize.width);
        } else {
            place.x -= totalSize.width;
        }
    }

    // Adjust position of menu rect if it goes beyond the screen height
    if (place.y + totalSize.height > uid.vidHeight) {
        place.y = (where) ? createRect.pos.y : createRect.size.height + createRect.pos.y;

        if (place.y < totalSize.height) {
            place.y = std::fmax(0.0f, uid.vidHeight - totalSize.height);
        } else {
            place.y -= totalSize.height;
        }
    }

    InitFrame(NULL, place.x, place.y, totalSize.width, totalSize.height, 1);
}

void UIPopupMenu::YouAreASubmenu(UIPopupMenu *me)
{
    m_parentMenu = me;
}

void UIPopupMenu::Draw(void)
{
    const float originX = getVirtualScale()[0] * 4.0f;
    const float originY = getVirtualScale()[1] * 4.0f;

    float fFontHeight = m_font->getHeight(getVirtualScale());
    float top         = originY;
    for (int i = 1; i <= m_describe->NumObjects(); i++) {
        uipopup_describe *desc = m_describe->ObjectAt(i);
        if (desc->type == UIP_SEPARATOR) {
            DrawBox(
                originX * 2.0f, originY + top, m_frame.size.width - (originX * 4.0f), 1.0f, m_foreground_color, 1.0f
            );
        } else {
            UColor textColor;
            if (m_selected == i || m_submenu == i) {
                DrawBox(
                    originX / 2.0f, top, m_frame.size.width - originX, fFontHeight, m_highlightBGColor, m_local_alpha
                );

                textColor = m_highlightFGColor;
            } else {
                textColor = m_foreground_color;
            }

            m_font->setColor(textColor);
            m_marlett.setColor(textColor);
            const char *text = Sys_LV_CL_ConvertString(desc->title);
            m_font->Print(4.0f, top / getVirtualScale()[1], text, -1, getVirtualScale());

            if (desc->type == UIP_SUBMENU) {
                m_marlett.PrintJustified(
                    UIRect2D(0.0f, top, m_frame.size.width - originX, getDescribeHeight(desc)),
                    m_iFontAlignmentHorizontal,
                    m_iFontAlignmentVertical,
                    "4",
                    getVirtualScale()
                );
            }
        }

        top += getDescribeHeight(desc);
    }
}

void UIPopupMenu::MouseReleased(Event *ev)
{
    UIPoint2D p = MouseEventToClientPoint(ev); // unused
    if (m_selected == -1 || m_selected > m_describe->NumObjects()) {
        return;
    }

    uipopup_describe *desc = m_describe->ObjectAt(m_selected);
    if (!desc || desc->type == UIP_SEPARATOR || desc->type == UIP_SUBMENU) {
        return;
    }

    // NOTE: Dismiss() below can delete this entire instance!
    // Save the m_listener pointer for later, as `this->m_listener`
    // cannot be used after the instance was freed.
    Listener *listener = m_listener;

    if (m_autodismiss) {
        Dismiss();
    }

    switch (desc->type) {
    case UIP_EVENT:
        // make sure it's passed in as an Event&, as ProcessEvent(Event*) would try to delete the static event!
        listener->ProcessEvent(*(Event *)desc->data);
        break;
    case UIP_EVENT_STRING:
        listener->ProcessEvent(new Event((const char *)desc->data));
        break;
    case UIP_CMD:
    case UIP_CVAR:
        Cbuf_AddText((const char *)desc->data);
        break;
    default:
        return;
    }
}

void UIPopupMenu::MouseMoved(Event *ev)
{
    UIPoint2D         point    = MouseEventToClientPoint(ev);
    uipopup_describe *describe = getDescribeFromPoint(point);
    if (describe) {
        int index = m_describe->IndexOfObject(describe);
        setSelection(index);
    } else {
        setSelection(-1);
    }
}

void UIPopupMenu::MouseExited(Event *ev)
{
    MouseMoved(ev);
}

void UIPopupMenu::Dismiss(void)
{
    // navigate to the topmost menu
    if (m_parentMenu) {
        m_parentMenu->Dismiss();
    } else {
        delete this;
    }
}

void UIPopupMenu::DismissEvent(Event *ev)
{
    Dismiss();
}

void UIPopupMenu::setAutoDismiss(bool b)
{
    m_autodismiss = b;
}

void UIPopupMenu::setNextSelection(void)
{
    if (!m_describe->NumObjects()) {
        return;
    }

    if (m_selected == -1) {
        setSelection(0);
        return;
    }

    int sel = m_selected + 1;
    if (sel > m_describe->NumObjects()) {
        // wrap around
        sel = 0;
    }

    setSelection(sel);
}

void UIPopupMenu::setPrevSelection(void)
{
    if (!m_describe->NumObjects()) {
        return;
    }

    if (m_selected == -1) {
        setSelection(0);
        return;
    }

    int sel;
    if (m_selected != 0) {
        sel = m_selected - 1;
    } else {
        // wrap around
        sel = m_describe->NumObjects();
    }

    setSelection(sel);
}

uipopup_describe *UIPopupMenu::getSelectedDescribe(void)
{
    if (m_selected == -1) {
        return NULL;
    }

    return m_describe->ObjectAt(m_selected);
}

void UIPopupMenu::setSelection(int sel)
{
    int maxSel = m_describe->NumObjects();
    if (sel > maxSel) {
        m_selected = maxSel;
        return;
    }

    int newSubMenu = -1;
    if (sel != -1) {
        uipopup_describe *desc = m_describe->ObjectAt(sel);
        if (desc && desc->type == UIP_SUBMENU) {
            newSubMenu = sel;
        }
    }

    m_selected = sel;

    if (newSubMenu == m_submenu) {
        return;
    }

    if (newSubMenu == -1) {
        if (m_submenuptr) {
            delete m_submenuptr;
        }

        m_submenu = -1;
    } else {
        if (m_submenu != -1 && m_submenuptr) {
            delete m_submenuptr;
        }

        m_submenu = newSubMenu;
        MakeSubMenu();
    }
}

void UIPopupMenu::setHighlightFGColor(UColor c)
{
    m_highlightFGColor = c;
}

void UIPopupMenu::setHighlightBGColor(UColor c)
{
    m_highlightBGColor = c;
}

void UIPopupMenu::setPopupHighlightBGColor(str menu, UColor c)
{
    // not present in disassembly
}

void UIPopupMenu::setPopupHighlightFGColor(str menu, UColor c)
{
    // not present in disassembly
}

void UIPopupMenu::getPulldown(str title)
{
    // not present in disassembly
}
