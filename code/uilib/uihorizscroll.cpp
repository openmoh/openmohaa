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

Event W_HScrollbar_Positioned
(
    "hscrollbar_positioned",
    EV_DEFAULT,
    "i",
    "new_position",
    "Signaled when the user scrolls the view"
);
Event EV_HScrollbar_Scroll
(
    "_hscrollbar_scroll",
    EV_DEFAULT,
    "ii",
    "scrollrate autorepeat",
    "scroll the scrollbar, at the specified rate.\n"
    "The autorepeat is used for autoscrolling with the mouse button held down"
);

CLASS_DECLARATION(UIWidget, UIHorizScroll, NULL) {
    {&W_LeftMouseDown,      &UIHorizScroll::MouseDown   },
    {&W_LeftMouseUp,        &UIHorizScroll::MouseUp     },
    {&W_LeftMouseDragged,   &UIHorizScroll::MouseDragged},
    {&W_MouseEntered,       &UIHorizScroll::MouseEnter  },
    {&W_MouseExited,        &UIHorizScroll::MouseLeave  },
    {&EV_HScrollbar_Scroll, &UIHorizScroll::Scroll      },
    {NULL,                  NULL                        }
};

UIHorizScroll::UIHorizScroll()
    : m_numitems(0)
    , m_pagewidth(0)
    , m_topitem(0)
    , m_marlett("marlett")
    , thumbRect(0, 0, 0, 0)
    , m_pressed(VS_NONE)
    , m_frameinitted(false)
    , m_thumbcolor(UDarkGrey)
    , m_solidbordercolor(UBlack)
{}

void UIHorizScroll::DrawArrow(float top, const char *text, bool pressed)
{
    UIRect2D arrowRect(top, 0, m_frame.size.height, m_vVirtualScale[1] * 16.f);
    UColor   innerColor(m_background_color);

    m_marlett.setColor(m_foreground_color);

    if (pressed) {
        if (!m_background_color.r && !m_background_color.g && !m_background_color.b) {
            m_marlett.setColor(UGrey);
        } else {
            m_marlett.setColor(m_border_color.light);
        }
    }

    DrawBoxWithSolidBorder(arrowRect, innerColor, m_solidbordercolor, 1, 3, m_local_alpha);
    if (m_numitems > m_pagewidth) {
        DrawMac3DBox(arrowRect, pressed, m_border_color, 1, m_local_alpha);
    }

    m_marlett.PrintJustified(
        arrowRect, m_iFontAlignmentHorizontal, m_iFontAlignmentVertical, text, m_bVirtual ? m_vVirtualScale : NULL
    );
}

void UIHorizScroll::DrawThumb(void)
{
    UIRect2D inbarrect;
    float    thumbWidth;
    float    thumbdiff;
    UColor   thumbInside;

    thumbdiff             = m_frame.size.width - m_vVirtualScale[1] * 32.f;
    inbarrect.pos.x       = m_vVirtualScale[1] * 16.0;
    inbarrect.pos.y       = 0;
    inbarrect.size.width  = thumbdiff;
    inbarrect.size.height = m_frame.size.height;

    thumbRect.pos.x      = m_topitem * thumbdiff / m_numitems;
    thumbRect.pos.y      = 0.0;
    thumbRect.size.width = m_pagewidth * thumbdiff / m_numitems;
    if (thumbRect.size.width < 6.f) {
        thumbRect.size.width = 6.f;
    }
    thumbRect.size.height = m_frame.size.height;

    thumbInside = UDarkGrey;
    thumbWidth  = thumbRect.pos.x + thumbRect.size.width - thumbdiff;

    if (m_pressed == VS_THUMB) {
        thumbInside = UColor(-0.5f, -0.5f, -0.5f, thumbInside.a + 1.f);

        if (thumbInside.r >= 0) {
            thumbInside.r = 0;
        }

        thumbInside.g = Q_max(thumbInside.g, 0);
        thumbInside.b = Q_max(thumbInside.b, 0);
    }

    if (thumbWidth > 0) {
        thumbRect.pos.x -= thumbWidth;
    }
    thumbRect.pos.x += inbarrect.pos.x;

    DrawBoxWithSolidBorder(thumbRect, thumbInside, m_solidbordercolor, 1, 3, m_local_alpha);
    DrawMac3DBox(thumbRect, false, UBorderColor(thumbInside), true, m_local_alpha);

    inbarrect.pos.x += 1;
    inbarrect.pos.y += 1;
    inbarrect.size.width += -2;
    inbarrect.size.height += -2;

    thumbRect.pos.x += -1;
    thumbRect.pos.y += -1;
    thumbRect.size.width += -2;
    thumbRect.size.height += 2;

    switch (m_pressed) {
    case VS_PAGE_UP:
        DrawBox(
            inbarrect.pos.x,
            0,
            thumbRect.pos.x - inbarrect.pos.x,
            m_frame.size.height,
            m_border_color.dark,
            m_local_alpha
        );
        break;
    case VS_PAGE_DOWN:
        DrawBox(
            thumbRect.pos.x + thumbRect.size.width,
            0,
            inbarrect.pos.x + inbarrect.size.width - (thumbRect.pos.x + thumbRect.size.width),
            m_frame.size.height,
            m_border_color.dark,
            m_local_alpha
        );
        break;
    default:
        break;
    }
}

void UIHorizScroll::Draw(void)
{
    UIRect2D arrowRect;
    UColor   innerColor(0, 0, 0, 1);

    arrowRect = getClientFrame();
    DrawBoxWithSolidBorder(arrowRect, m_background_color, m_solidbordercolor, 1, 3, m_local_alpha);

    DrawArrow(0, "3", m_pressed == VS_UP_ARROW);
    DrawArrow(m_frame.size.width - m_vVirtualScale[1] * 16.f, "4", m_pressed = VS_DOWN_ARROW);
    if (m_numitems > m_pagewidth) {
        DrawThumb();
    }
}

void UIHorizScroll::Scroll(Event *ev)
{
    if (!AttemptScrollTo(m_topitem + ev->GetInteger(1))) {
        return;
    }

    float delay = 0.2f;
    if (ev->GetInteger(2)) {
        delay = 0.1f;
    }

    Event event(EV_HScrollbar_Scroll);
    event.AddInteger(ev->GetInteger(1));
    event.AddInteger(0);
    PostEvent(event, delay);
}

bool UIHorizScroll::AttemptScrollTo(int to)
{
    int besttopitem;

    if (to < 0) {
        to = 0;
    }

    besttopitem = m_numitems - m_pagewidth;
    if (besttopitem < 0) {
        besttopitem = 0;
    }

    if (to > besttopitem) {
        to = besttopitem;
    }

    if (to == m_topitem) {
        return false;
    }

    Event ev(W_HScrollbar_Positioned);
    ev.AddInteger(m_topitem);
    SendSignal(ev);

    return true;
}

void UIHorizScroll::MouseDown(Event *ev)
{
    UIPoint2D p;
    int       scrollrate = 0;

    p.x = ev->GetFloat(1);
    p.y = ev->GetFloat(2);
    p.x -= m_screenframe.pos.x;
    p.y -= m_screenframe.pos.y;

    if (p.x < 0 || p.y < 0) {
        // out of bounds
        return;
    }

    if (p.x > m_frame.size.width || p.y > m_frame.size.height) {
        // out of bounds
        return;
    }

    if (m_numitems <= m_pagewidth) {
        return;
    }

    if (p.x < 16.f) {
        scrollrate = -1;
        m_pressed  = VS_UP_ARROW;
    } else if (p.x > m_frame.size.width - 16.f) {
        scrollrate = 1;
        m_pressed  = VS_DOWN_ARROW;
    } else if (p.x < thumbRect.pos.x) {
        m_pressed  = VS_PAGE_UP;
        scrollrate = -m_pagewidth;
    } else if (p.x >= (thumbRect.pos.x + thumbRect.size.width)) {
        m_pressed  = VS_PAGE_DOWN;
        scrollrate = m_pagewidth;
    } else {
        m_pressed                   = VS_THUMB;
        m_dragThumbState.itemOffset = m_topitem - getItemFromWidth(p.x);
        m_dragThumbState.orgItem    = m_topitem;
        uWinMan.setFirstResponder(this);
    }

    if (scrollrate) {
        Event *event = new Event(EV_HScrollbar_Scroll);
        event->AddInteger(scrollrate);
        event->AddInteger(0);
        CancelEventsOfType(EV_HScrollbar_Scroll);
        PostEvent(event, 0);

        uWinMan.setFirstResponder(this);
    }
}

int UIHorizScroll::getItemFromWidth(float height)
{
    return (int)((height - 16.0) * m_numitems / (m_frame.size.width - 32.0));
}

void UIHorizScroll::MouseUp(Event *ev)
{
    CancelEventsOfType(EV_HScrollbar_Scroll);

    if (uWinMan.getFirstResponder() == this) {
        uWinMan.setFirstResponder(NULL);
    }

    m_pressed = VS_NONE;
}

void UIHorizScroll::MouseDragged(Event *ev)
{
    UIPoint2D p;

    if (m_pressed != VS_THUMB) {
        return;
    }

    p.x = ev->GetFloat(1);
    p.y = ev->GetFloat(2);

    if (p.y - m_screenframe.pos.y < -24 || m_frame.size.height + 24.0 < p.y - m_screenframe.pos.y) {
        AttemptScrollTo(m_dragThumbState.orgItem);
    } else {
        AttemptScrollTo(m_dragThumbState.itemOffset + getItemFromWidth(p.x - m_screenframe.pos.x));
    }
}

void UIHorizScroll::InitFrameAlignRight(UIWidget *parent)
{
    UIRect2D frame, frameOut;

    frame                = parent->getClientFrame();
    frameOut.pos.x       = 0;
    frameOut.pos.y       = frame.pos.y + frame.size.height - m_vVirtualScale[0] * 16.f;
    frameOut.size.width  = frame.size.width - m_vVirtualScale[0] * 16.f;
    frameOut.size.height = m_vVirtualScale[0] * 16.f;

    if (m_frameinitted) {
        setFrame(frameOut);
        return;
    }

    InitFrame(parent, frameOut, -1);
}

void UIHorizScroll::MouseEnter(Event *ev)
{
    uWinMan.ActivateControl(this);
}

void UIHorizScroll::MouseLeave(Event *ev) {}

bool UIHorizScroll::isEnoughItems(void)
{
    return m_numitems > m_pagewidth;
}

void UIHorizScroll::setNumItems(int i)
{
    m_numitems = i;
}

void UIHorizScroll::setPageWidth(int i)
{
    m_pagewidth = i;
}

void UIHorizScroll::setTopItem(int i)
{
    m_topitem = i;
}

int UIHorizScroll::getTopItem(void)
{
    return m_topitem;
}

int UIHorizScroll::getPageWidth(void)
{
    return m_pagewidth;
}

int UIHorizScroll::getNumItems(void)
{
    return m_numitems;
}

void UIHorizScroll::setThumbColor(const UColor& thumb)
{
    m_thumbcolor = thumb;
}

void UIHorizScroll::setSolidBorderColor(const UColor& col)
{
    m_solidbordercolor = col;
}
