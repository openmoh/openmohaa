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
    // FIXME: stub
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
    m_maxlines = m_frame.size.height / m_font->getHeight(false);
}

void FakkMiniconsole::MoveInEvent(Event *ev)
{
    ChangeBoxState(boxstate_t::box_moving_in);
}

void FakkMiniconsole::HandleBoxMoving(void)
{
    // FIXME: stub
}

void FakkMiniconsole::Draw(void)
{
    // FIXME: stub
}

void FakkMiniconsole::Create(const UISize2D& size, const UColor& fore, const UColor& back, float alpha)
{
    // FIXME: stub
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
