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

#include "cl_ui.h"

Event EV_Minicon_Goin("_minicon_goin", EV_DEFAULT, NULL, NULL, "Event to make the miniconsole disappear");

CLASS_DECLARATION(UIWidget, FakkMiniconsole, NULL) {
    {&W_SizeChanged,   &FakkMiniconsole::OnSizeChanged},
    {&EV_Minicon_Goin, &FakkMiniconsole::MoveInEvent  },
    {NULL,             NULL                           }
};

FakkMiniconsole::FakkMiniconsole()
{
    m_maxlines    = 0;
    m_reallyshown = true;
    m_boxstate    = boxstate_t::box_in;
    m_boxtime     = uid.time;
    m_movespeed   = 500;
}

void FakkMiniconsole::setShowState(void)
{
    if (m_reallyshown) {
        setShow(m_boxstate != boxstate_t::box_in);
    } else {
        setShow(false);
    }
}

void FakkMiniconsole::setRealShow(bool b)
{
    m_reallyshown = b;
    setShowState();
}

void FakkMiniconsole::VerifyBoxOut(void)
{
    PostMoveinEvent();

    if (m_boxstate && m_boxstate != boxstate_t::box_out) {
        ChangeBoxState(box_moving_out);
    }
}

void FakkMiniconsole::Print(const char *text)
{
    str        *lastline;
    const char *p;

    if (!m_lines.NumObjects()) {
        m_lines.AddObject({});
    }

    lastline = &m_lines.ObjectAt(m_lines.NumObjects());

    if (*text) {
        for (p = text; *p; p++) {
            if (*p != '\n') {
                *lastline += *p;
            } else {
                m_lines.AddObject({});
                lastline = &m_lines.ObjectAt(m_lines.NumObjects());
            }
        }
    }

    if (lastline && lastline->length() > 128) {
        Print("\n");
    }

    if (m_maxlines < 0) {
        m_maxlines = 1;
    }

    while (m_lines.NumObjects() > this->m_maxlines) {
        m_lines.RemoveObjectAt(1);
    }

    VerifyBoxOut();
}

void FakkMiniconsole::PostMoveinEvent(void)
{
    if (m_boxstate != boxstate_t::box_out) {
        return;
    }

    CancelEventsOfType(EV_Minicon_Goin);
    PostEvent(new Event(EV_Minicon_Goin), 5.f);
}

void FakkMiniconsole::OnSizeChanged(Event *ev)
{
    m_maxlines = m_frame.size.height / m_font->getHeight(getHighResScale());
}

void FakkMiniconsole::MoveInEvent(Event *ev)
{
    ChangeBoxState(boxstate_t::box_moving_in);
}

void FakkMiniconsole::HandleBoxMoving(void)
{
    int      delta;
    UIRect2D newRect;

    if (m_boxstate == boxstate_t::box_out || m_boxstate == boxstate_t::box_in) {
        return;
    }

    delta     = m_movespeed * (uid.time - m_boxtime) / 1000;
    m_boxtime = 1000 * delta / m_movespeed + m_boxtime;

    switch (m_boxstate) {
    case boxstate_t::box_moving_in:
        newRect = UIRect2D(delta + m_frame.pos.x, m_frame.pos.y, m_frame.size.width, m_frame.size.height);
        if (newRect.pos.x >= uid.vidWidth) {
            newRect.pos.x = uid.vidWidth;
            ChangeBoxState(boxstate_t::box_in);
        }
        break;
    case boxstate_t::box_moving_out:
        newRect = UIRect2D(-delta + m_frame.pos.x, m_frame.pos.y, m_frame.size.width, m_frame.size.height);
        if (newRect.pos.x <= uid.vidWidth - newRect.size.width) {
            newRect.pos.x = uid.vidWidth - newRect.size.width;
            ChangeBoxState(boxstate_t::box_out);
        }
        break;
    default:
        ChangeBoxState(boxstate_t::box_in);
        break;
    }

    setFrame(newRect);
}

void FakkMiniconsole::Draw(void)
{
    float aty;
    int   i;

    HandleBoxMoving();

    m_font->setColor(m_foreground_color);
    aty = m_frame.size.height - m_font->getHeight(getHighResScale());
    for (i = m_lines.NumObjects(); i > 0; i--) {
        if (-m_font->getHeight(getHighResScale()) >= aty) {
            break;
        }

        m_font->Print(0, aty / getHighResScale()[1], m_lines.ObjectAt(i), -1, getHighResScale());
        aty -= m_font->getHeight(getHighResScale());
    }
}

void FakkMiniconsole::Create(const UISize2D& size, const UColor& fore, const UColor& back, float alpha)
{
    InitFrame(NULL, uid.vidWidth, 0, size.width, size.height, 0);

    setBackgroundColor(back, true);
    setForegroundColor(fore);
    setBackgroundAlpha(alpha);
    Connect(this, W_SizeChanged, W_SizeChanged);
    OnSizeChanged(NULL);

    m_movespeed = size.width * 3;

    setShowState();
}

void FakkMiniconsole::ChangeBoxState(boxstate_t state)
{
    m_boxstate = state;
    m_boxtime  = uid.time;
    setShowState();
    if (state == boxstate_t::box_out) {
        PostMoveinEvent();
    }
}
