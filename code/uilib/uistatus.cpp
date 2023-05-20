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

CLASS_DECLARATION( UIWidget, UIWindowSizer, NULL )
{
	{ &W_LeftMouseDown,			&UIWindowSizer::MouseDown },
	{ &W_LeftMouseUp,			&UIWindowSizer::MouseUp },
	{ &W_LeftMouseDragged,		&UIWindowSizer::MouseDragged },
	{ NULL, NULL }
};

UIWindowSizer::UIWindowSizer()
{
	m_draggingwidget = NULL;
	m_mouseState = M_NONE;
}


UIWindowSizer::UIWindowSizer(UIWidget* w)
	: UIWindowSizer()
{
	m_draggingwidget = w;
}

void UIWindowSizer::Draw
	(
	void
	)

{
	m_font->setColor(UWhite);
	m_font->Print(0, 0, "o", -1, m_bVirtual);

	m_font->setColor(UBlack);
	m_font->Print(0, 0, "p", -1, m_bVirtual);
}

void UIWindowSizer::FrameInitialized
	(
	void
	)

{
	setFont("marlett");
}

void UIWindowSizer::MouseDown
	(
	Event *ev
	)

{
	m_screenDragPoint.x = ev->GetFloat(1);
	m_screenDragPoint.y = ev->GetFloat(2);
	m_mouseState = M_DRAGGING;
	uWinMan.setFirstResponder(this);
}

void UIWindowSizer::MouseUp
	(
	Event *ev
	)

{
	if (uWinMan.getFirstResponder() == this) {
		uWinMan.setFirstResponder(NULL);
	}

	m_mouseState = M_NONE;
}

void UIWindowSizer::MouseDragged
	(
	Event *ev
	)

{
	UIPoint2D newpoint, delta;
	UISize2D newsize;

	if (m_mouseState != M_DRAGGING) {
		return;
	}

	newpoint.x = ev->GetFloat(1);
	newpoint.y = ev->GetFloat(2);
	delta = UIPoint2D(newpoint.x - m_screenDragPoint.x, newpoint.y - m_screenDragPoint.y);

	if (delta.x || delta.y)
	{
		m_screenDragPoint = newpoint;
		newsize = m_draggingwidget->getSize();
		newsize.width += delta.x;
		newsize.height += delta.y;

		if (newsize.width < 32) newsize.width = 32;
		if (newsize.height < 16) newsize.height = 16;

		m_draggingwidget->setSize(newsize);
	}
}

void UIWindowSizer::setDraggingWidget
	(
	UIWidget *w
	)

{
	m_draggingwidget = w;
}

UIWidget *UIWindowSizer::getDraggingWidget
	(
	void
	)

{
	return m_draggingwidget;
}

CLASS_DECLARATION( UIWidget, UIStatusBar, NULL )
{
	{ &W_RealignWidget,	&UIStatusBar::ParentSized },
	{ &W_SizeChanged,	&UIStatusBar::SelfSized },
	{ NULL, NULL }
};

UIStatusBar::UIStatusBar()
{
	m_sizeenabled = false;
	m_created = false;
	m_sizer = NULL;
	m_align.alignment = WND_ALIGN_NONE;
	m_iFontAlignmentHorizontal = FONT_JUSTHORZ_LEFT;
}

UIStatusBar::UIStatusBar
	(
	alignment_t align,
	float height
	)
	: UIStatusBar()

{
	AlignBar(align, height);
}

void UIStatusBar::FrameInitialized
	(
	void
	)

{
	m_created = true;
	Connect(this, W_SizeChanged, W_SizeChanged);
	if (m_parent) {
		m_parent->Connect(this, W_SizeChanged, W_RealignWidget);
	}

	SelfSized(NULL);
	ParentSized(NULL);
}

void UIStatusBar::Draw
	(
	void
	)

{
	if (m_align.alignment == WND_ALIGN_BOTTOM) {
		DrawBox(0.0, 0.0, m_frame.size.width, 1.0, m_border_color.dark, 1.0);
	}

	m_font->setColor(m_foreground_color);
	m_font->PrintJustified(
		getClientFrame(),
		m_iFontAlignmentHorizontal,
		m_iFontAlignmentVertical,
		Sys_LV_CL_ConvertString(m_title.c_str()),
		m_bVirtual ? m_vVirtualScale : NULL
	);

	if (m_sizer)
	{
		m_sizer->setForegroundColor(m_foreground_color);
		m_sizer->setBackgroundColor(m_background_color, true);
		m_sizer->setBackgroundAlpha(m_local_alpha);
	}
}

void UIStatusBar::AlignBar
	(
	alignment_t align,
	float height
	)

{
	m_align.alignment = align;
	m_align.dist = height;
}

void UIStatusBar::DontAlignBar
	(
	void
	)

{
	m_align.alignment = WND_ALIGN_NONE;
}

void UIStatusBar::EnableSizeBox
	(
	UIWidget *which
	)

{
	m_sizeenabled = which;
	if (m_created) {
		SelfSized(NULL);
	}
}

void UIStatusBar::ParentSized
	(
	Event *ev
	)

{
	if (m_align.alignment == WND_ALIGN_BOTTOM)
	{
		UISize2D parentSize = m_parent->getSize();

		setFrame(UIRect2D(0.0, parentSize.height - m_align.dist, parentSize.width, m_align.dist));
	}
}

void UIStatusBar::SelfSized
	(
	Event *ev
	)

{
	if (m_sizeenabled)
	{
		if (m_sizer) {
			if (m_sizer->getDraggingWidget() != m_sizeenabled) {
				m_sizer->setDraggingWidget(m_sizeenabled);
			}
		}
		else {
			m_sizer = new UIWindowSizer(m_sizeenabled);
			m_sizer->InitFrame(this, 0, 0, 16.0, 16.0, 0);
		}


		UISize2D sizerFrame = m_sizer->getSize();

		m_sizer->setFrame(UIRect2D(
			UIPoint2D(m_frame.size.width - sizerFrame.width, m_frame.size.height - sizerFrame.height),
			sizerFrame
		));
	}
	else if (m_sizer)
	{
		delete m_sizer;
		m_sizer = NULL;
	}
}

