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
#include "localization.h"

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
	DrawBoxWithSolidBorder(
		getClientFrame(),
		getBackgroundColor(),
		m_solidbordercolor,
		1,
		3,
		m_local_alpha
	);

	DrawArrow(0.0, "5", m_pressed == VS_UP_ARROW);
	DrawArrow(m_frame.size.height - m_vVirtualScale[1] * 16.0, "6", m_pressed == VS_DOWN_ARROW);

	if (m_numitems > m_pageheight) {
		DrawThumb();
	}
}

void UIVertScroll::DrawArrow
	(
	float top,
	const char *text,
	bool pressed
	)

{
	UIRect2D arrowRect;
	UColor innerColor;

	arrowRect.pos.x = 0.0;
	arrowRect.pos.y = top;
	arrowRect.size.width = m_frame.size.width;
	arrowRect.size.height = m_vVirtualScale[1] * 16.0;

	innerColor = getBackgroundColor();
	m_marlett.setColor(getForegroundColor());

	if (pressed)
	{
		innerColor = m_border_color.light;

		if (m_background_color.r != 0.0 || m_background_color.g || m_background_color.b) {
			m_marlett.setColor(m_border_color.light);
		} else {
			m_marlett.setColor(UColor(0.15, 0.196, 0.278, 1.0));
		}
	}

	DrawBoxWithSolidBorder(arrowRect, innerColor, m_solidbordercolor, 1, 3, m_local_alpha);
	if (m_numitems > m_pageheight) {
		DrawMac3DBox(arrowRect, pressed, m_border_color, 1, m_local_alpha);
	}

	m_marlett.setAlpha(m_local_alpha);
	m_marlett.PrintJustified(
		arrowRect,
		m_iFontAlignmentHorizontal,
		m_iFontAlignmentVertical,
		Sys_LV_CL_ConvertString(text),
		m_bVirtual ? m_vVirtualScale : NULL
	);
}

void UIVertScroll::DrawThumb
	(
	void
	)

{
	UIRect2D inbarrect;
	float thumbHeight, thumbdiff;
	UColor thumbInside;

	thumbdiff = m_frame.size.height - m_vVirtualScale[1] * 32.0;

	inbarrect.pos.x = 0.0;
	inbarrect.pos.y = m_vVirtualScale[1] * 16.0;
	inbarrect.size.width = m_frame.size.width;
	inbarrect.size.height = m_frame.size.height - thumbdiff;

	thumbHeight = m_pageheight * thumbdiff;

	thumbRect.pos.x = 0.0;
	thumbRect.pos.y = m_topitem * thumbdiff / (float)m_numitems;
	thumbRect.size.width = m_frame.size.width;
	thumbRect.size.height = thumbHeight / (float)m_numitems;
	if (thumbRect.size.height < 6.0) {
		thumbRect.size.height = 6.0;
	}

	thumbInside = UColor(0.15, 0.196, 0.278, 1.0);

	if (thumbRect.pos.y + thumbRect.size.height - thumbdiff > 0.0) {
		thumbRect.pos.y -= thumbRect.pos.y + thumbRect.size.height - thumbdiff;
	}
	thumbRect.pos.y = thumbRect.pos.y + inbarrect.pos.y;

	DrawBoxWithSolidBorder(thumbRect, thumbInside, m_solidbordercolor, 1, 3, m_local_alpha);
	DrawMac3DBox(thumbRect, m_pressed == VS_THUMB, thumbInside, true, m_local_alpha);

	inbarrect.pos.x += 1.0;
	inbarrect.pos.y += 1.0;
	inbarrect.size.width -= 2.0;
	inbarrect.size.height -= 2.0;

	thumbRect.pos.x -= 1.0;
	thumbRect.pos.y -= 1.0;
	thumbRect.size.width -= 2.0;
	thumbRect.size.height -= 2.0;

	if (m_pressed == VS_PAGE_UP)
	{
		DrawBox(
			0.0,
			inbarrect.pos.y,
			m_frame.size.width,
			thumbRect.pos.y - inbarrect.pos.y,
			m_border_color.light,
			m_local_alpha
		);
	}
	else
	{
		DrawBox(
			0.0,
			thumbRect.pos.y + thumbRect.size.height,
			m_frame.size.width,
			inbarrect.pos.y + inbarrect.size.height - (thumbRect.pos.y + thumbRect.size.height),
			m_border_color.light,
			m_local_alpha
		);
	}
}

void UIVertScroll::MouseDown
	(
	Event *ev
	)

{
	UIPoint2D p;
	int scrollrate;

	scrollrate = 0;

	p.x = ev->GetFloat(1) - m_screenframe.pos.x;
	p.y = ev->GetFloat(2) - m_screenframe.pos.y;
	m_pressed = VS_NONE;

	if (p.x < 0 || p.y < 0) {
		return;
	}

	if (p.x > m_frame.size.width || p.y > m_frame.size.height) {
		return;
	}

	if (m_numitems <= m_pageheight) {
		return;
	}

	if (p.y >= 16.0)
	{
		if (m_frame.size.height - 16.0 >= p.y)
		{
			if (thumbRect.pos.y > p.y)
			{
				m_pressed = VS_PAGE_UP;
				scrollrate = -m_pageheight;
			}
			else
			{
				if (thumbRect.pos.y + thumbRect.size.height > p.y)
				{
					m_pressed = VS_THUMB;
					m_dragThumbState.itemOffset = m_topitem - getItemFromHeight(p.y);
					m_dragThumbState.orgItem = m_topitem;
					uWinMan.setFirstResponder(this);
				}
				else
				{
					m_pressed = VS_PAGE_DOWN;
					scrollrate = m_pageheight;
				}
			}

			if (!scrollrate) {
				return;
			}
		}
		else
		{
			m_pressed = VS_DOWN_ARROW;
			scrollrate = 1;
		}
	}

	Event* newev = new Event(EV_Scrollbar_Scroll);
	newev->AddInteger(scrollrate);
	newev->AddInteger(0);
	CancelEventsOfType(EV_Scrollbar_Scroll);
	PostEvent(newev, 0);

	uWinMan.setFirstResponder(this);
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
}

void UIVertScroll::MouseDragged
	(
	Event *ev
	)

{
	UIPoint2D p;

	if (m_pressed != VS_THUMB) {
		return;
	}

	p.x = ev->GetFloat(1) - m_screenframe.pos.x;
	p.y = ev->GetFloat(2);

	if (p.x < -24.0 || p.x > m_frame.size.width + 24.0) {
		AttemptScrollTo(m_dragThumbState.orgItem);
	} else {
		AttemptScrollTo(m_dragThumbState.itemOffset + getItemFromHeight(p.y - m_screenframe.pos.y));
	}
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
	if (AttemptScrollTo(m_topitem + ev->GetInteger(1)))
	{
		float delay = 0.2;
		if (ev->GetInteger(2)) {
			delay = 0.1;
		}

		Event *newev = new Event(EV_Scrollbar_Scroll);
		newev->AddInteger(ev->GetInteger(1));
		newev->AddInteger(0);
		PostEvent(newev, delay);
	}
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
	m_numitems = i;
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
