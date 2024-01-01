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
{
    // FIXME: stub
}

void UIHorizScroll::DrawArrow(float top, const char *text, bool pressed)
{
    // FIXME: stub
}

void UIHorizScroll::DrawThumb(void)
{
    // FIXME: stub
}

void UIHorizScroll::Draw(void)
{
    // FIXME: stub
}

void UIHorizScroll::Scroll(Event *ev)
{
    // FIXME: stub
}

bool UIHorizScroll::AttemptScrollTo(int to)
{
    // FIXME: stub
    return false;
}

void UIHorizScroll::MouseDown(Event *ev)
{
    // FIXME: stub
}

int UIHorizScroll::getItemFromWidth(float height)
{
    // FIXME: stub
    return 0;
}

void UIHorizScroll::MouseUp(Event *ev)
{
    // FIXME: stub
}

void UIHorizScroll::MouseDragged(Event *ev)
{
    // FIXME: stub
}

void UIHorizScroll::InitFrameAlignRight(UIWidget *parent)
{
    // FIXME: stub
}

void UIHorizScroll::MouseEnter(Event *ev)
{
    // FIXME: stub
}

void UIHorizScroll::MouseLeave(Event *ev)
{
    // FIXME: stub
}

bool UIHorizScroll::isEnoughItems(void)
{
    // FIXME: stub
    return false;
}

void UIHorizScroll::setNumItems(int i)
{
    // FIXME: stub
}

void UIHorizScroll::setPageWidth(int i)
{
    // FIXME: stub
}

void UIHorizScroll::setTopItem(int i)
{
    // FIXME: stub
}

int UIHorizScroll::getTopItem(void)
{
    // FIXME: stub
    return 0;
}

int UIHorizScroll::getPageWidth(void)
{
    // FIXME: stub
    return 0;
}

int UIHorizScroll::getNumItems(void)
{
    // FIXME: stub
    return 0;
}

void UIHorizScroll::setThumbColor(const UColor& thumb)
{
    // FIXME: stub
}

void UIHorizScroll::setSolidBorderColor(const UColor& col)
{
    // FIXME: stub
}
