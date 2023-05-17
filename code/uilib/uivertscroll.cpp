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

Event W_Scrollbar_Positioned
(
	"scrollbar_positioned",
	EV_DEFAULT,
	"i",
	"new_position",
	"Signaled when the user scrolls the view"
);

Event EV_Scrollbar_Scroll
(
	"_scrollbar_scroll",
	EV_DEFAULT,
	"ii",
	"scrollrate autorepeat",
	"scroll the scrollbar, at the specified rate.\n"
	"The autorepeat is used for autoscrolling with the mouse button held down"
);

CLASS_DECLARATION( UIWidget, UIVertScroll, NULL )
{
	{ &W_LeftMouseDown,			&UIVertScroll::MouseDown },
	{ &W_LeftMouseUp,			&UIVertScroll::MouseUp },
	{ &W_LeftMouseDragged,		&UIVertScroll::MouseDragged },
	{ &W_MouseEntered,			&UIVertScroll::MouseEnter },
	{ &W_MouseExited,			&UIVertScroll::MouseLeave },
	{ &EV_Scrollbar_Scroll,		&UIVertScroll::Scroll },
	{ NULL, NULL }
};

UIVertScroll::UIVertScroll()
	: m_numitems(0)
	, m_pageheight(0)
	, m_topitem(0)
	, m_marlett("marlett")
	, thumbRect(0, 0, 0, 0)
	, m_thumbcolor(0.15, 0.196, 0.278, 1.0)
	, m_solidbordercolor(0.075, 0.098, 0.139, 1.0)
	, m_pressed(VS_NONE)
	, m_frameinitted(false)
{
	m_background_color = UColor(0.15, 0.196, 0.278, 1.0);
	m_foreground_color = UHudColor;
}

int UIVertScroll::getItemFromHeight
	(
	float height
	)

{
	return (int)((height - 16.0) * (float)this->m_numitems / (m_frame.size.height - 32.0));
}

bool UIVertScroll::isEnoughItems
	(
	void
	)

{
	return m_numitems > m_pageheight;
}

void UIVertScroll::Draw
	(
	void
	)

{
	// FIXME: stub
}

void UIVertScroll::DrawArrow
	(
	float top,
	const char *text,
	bool pressed
	)

{
	// FIXME: stub
}

void UIVertScroll::DrawThumb
	(
	void
	)

{
	// FIXME: stub
}

void UIVertScroll::MouseDown
	(
	Event *ev
	)

{
	// FIXME: stub
}

void UIVertScroll::MouseUp
	(
	Event *ev
	)

{
	CancelEventsOfType(EV_Scrollbar_Scroll);
	if (uWinMan.getFirstResponder() == this) {
		uWinMan.setFirstResponder(NULL);
	}

	m_pressed = VS_NONE;
	// FIXME: stub
}

void UIVertScroll::MouseDragged
	(
	Event *ev
	)

{
	// FIXME: stub
}

void UIVertScroll::MouseEnter
	(
	Event *ev
	)

{
	uWinMan.ActivateControl(this);
}

void UIVertScroll::MouseLeave
	(
	Event *ev
	)

{
}

void UIVertScroll::Scroll
	(
	Event *ev
	)

{
	// FIXME: stub
}

bool UIVertScroll::AttemptScrollTo
	(
	int to
	)

{
	int besttopitem;

	if (to < 0) {
		to = 0;
	}

	besttopitem = m_numitems - m_pageheight;
	if (besttopitem < 0) {
		besttopitem = 0;
	}

	if (to > besttopitem) {
		to = besttopitem;
	}

	if (to == m_topitem) {
		return false;
	}

	m_topitem = to;

	Event ev(W_Scrollbar_Positioned);
	ev.AddInteger(m_topitem);
	SendSignal(ev);

	return true;
}

void UIVertScroll::setNumItems
	(
	int i
	)

{
	m_numitems = 0;
}

void UIVertScroll::setPageHeight
	(
	int i
	)

{
	m_pageheight = i;
}

void UIVertScroll::setTopItem
	(
	int i
	)

{
	m_topitem = i;
}

int UIVertScroll::getTopItem
	(
	void
	)

{
	return m_topitem;
}

int UIVertScroll::getPageHeight
	(
	void
	)

{
	return m_pageheight;
}

int UIVertScroll::getNumItems
	(
	void
	)

{
	return m_numitems;
}

void UIVertScroll::setThumbColor
	(
	const UColor& thumb
	)

{
	m_thumbcolor = thumb;
}

void UIVertScroll::setSolidBorderColor
	(
	const UColor& col
	)

{
	m_solidbordercolor = col;
}

void UIVertScroll::InitFrameAlignRight
	(
	UIWidget *parent,
	float fWidthPadding,
	float fHeightPadding
	)

{
	UIRect2D frame, frameOut;

	frame = parent->getClientFrame();
	frameOut.pos.x = frame.pos.x + frame.size.width - (fWidthPadding + 16.0) * m_vVirtualScale[0];
	frameOut.pos.y = fHeightPadding * m_vVirtualScale[1];
	frameOut.size.width = m_vVirtualScale[0] * 16.0;
	frameOut.size.height = frame.size.height - (fHeightPadding * 2) * m_vVirtualScale[1];

	if (!m_frameinitted)
	{
		InitFrame(parent, frameOut, -1, "verdana-12");
		m_frameinitted = true;
	}
	else {
		setFrame(frameOut);
	}
}
