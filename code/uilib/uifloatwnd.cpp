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

#include "ui_local.h"

CLASS_DECLARATION( UIWidget, UIChildSpaceWidget, NULL )
{
	{ NULL, NULL }
};

UIChildSpaceWidget::UIChildSpaceWidget()
{
}

qboolean UIChildSpaceWidget::KeyEvent
	(
	int key,
	unsigned int time
	)

{
	if (m_parent)
	{
		//
		// Transmit the key event to the parent
		//
		return m_parent->KeyEvent(key, 0);
	}

	return qfalse;
}

CLASS_DECLARATION( UIWidget, UIFloatingWindow, NULL )
{
	{ &W_LeftMouseDown,							&UIFloatingWindow::Pressed },
	{ &W_LeftMouseDragged,						&UIFloatingWindow::Dragged },
	{ &W_LeftMouseUp,							&UIFloatingWindow::Released },
	{ &UIFloatingWindow::W_ClosePressed,		&UIFloatingWindow::ClosePressed },
	{ &UIFloatingWindow::W_MinimizePressed,		&UIFloatingWindow::MinimizePressed },
	{ &W_SizeChanged,							&UIFloatingWindow::SizeChanged },
	{ &W_Activated,								&UIFloatingWindow::OnActivated },
	{ NULL, NULL }
};

Event UIFloatingWindow::W_ClosePressed
(
	"floatingwindow_close_pressed",
	EV_DEFAULT,
	NULL,
	NULL,
	"Signal that the window was closed"
);
Event UIFloatingWindow::W_MinimizePressed
(
	"floatingwindow_minimize_pressed",
	EV_DEFAULT,
	NULL,
	NULL,
	"Signal that the window was minimized"
);

UIFloatingWindow::UIFloatingWindow()
{
	m_clickpoint = UIPoint2D(-20.0, -20.0);
	m_isPressed = false;
	m_minimized = false;
	m_canactivate = true;
	m_clicktime = -20000;
	// create buttons that compose a window
	m_closeButton = new UIButton();
	m_minimizeButton = new UIButton();
	m_childspace = new UIChildSpaceWidget();
}

UIFloatingWindow::~UIFloatingWindow()
{
}

void UIFloatingWindow::FrameInitialized
	(
	void
	)

{
	FrameInitialized(true);
}

void UIFloatingWindow::FrameInitialized
	(
	bool bHasDragBar
	)

{
	//
	// Initialize the close button
	//
	m_closeButton->InitFrame(
		this,
		m_frame.size.width - 18.0,
		2.0,
		16.0,
		16.0,
		-1,
		"marlett"
	);

	m_closeButton->setTitle("r");
	m_closeButton->setName("closebutton");

	//
	// Initialize the minimize button
	//
	m_minimizeButton->InitFrame(
		this,
		m_frame.size.width - 36.0,
		2.0,
		16.0,
		16.0,
		-1,
		"marlett"
	);
	m_minimizeButton->setTitle("0");
	m_minimizeButton->setName("minimizebutton");

	m_closeButton->Connect(this, W_Button_Pressed, UIFloatingWindow::W_ClosePressed);
	m_minimizeButton->Connect(this, W_Button_Pressed, UIFloatingWindow::W_MinimizePressed);
	//
	// Size event
	//
	Connect(this, W_SizeChanged, W_SizeChanged);
	Connect(this, W_Activated, W_Activated);

	//
	// Child space
	//
	if (bHasDragBar)
	{
		m_childspace->InitFrame(
			this,
			UIRect2D(2.0, 20.0, m_frame.size.width - 4.0, m_frame.size.height - 22.0),
			0,
			"verdana-12"
		);
	}
	else
	{
		m_childspace->InitFrame(
			this,
			UIRect2D(UIPoint2D(0, 0), m_frame.size),
			0,
			"verdana-12"
		);
	}

	m_childspace->setTitle("Child space");
	m_childspace->AllowActivate(true);
}

void UIFloatingWindow::ClosePressed
	(
	Event *ev
	)

{
	PostEvent(EV_Remove, 0.0);
}

void UIFloatingWindow::MinimizePressed
	(
	Event *ev
	)

{
	UISize2D size = getSize();

	if (!m_minimized)
	{
		m_minimized = true;
		// save the hold height
		// so the window can be restored later
		m_restoredHeight = size.height;
		size.height = 20.0;
		m_minimizeButton->setTitle("1");
	}
	else
	{
		m_minimized = false;
		size.height = m_restoredHeight;
		m_minimizeButton->setTitle("0");
	}

	setSize(size);
	uWinMan.ActivateControl(this);
}

void UIFloatingWindow::Pressed
	(
	Event *ev
	)

{
	UIPoint2D p = MouseEventToClientPoint(ev);

	m_clickOffset.x = p.x;
	m_clickOffset.y = p.y;

	if (uid.time - 500 >= m_clicktime) {
		m_clicktime = uid.time;
	}
	else
	{
		if (fabs(m_clickpoint.x - m_clickOffset.x) > 2.0 || fabs(m_clickpoint.y - m_clickOffset.y) > 2.0) {
			m_clicktime = uid.time;
		} else {
			ProcessEvent(W_MinimizePressed);
			m_clicktime = -20000;
		}
	}

	m_clickpoint = m_clickOffset;
	if (m_clickOffset.y < 18.0)
	{
		uWinMan.setFirstResponder(this);
		m_isPressed = true;
	}
}

void UIFloatingWindow::Released
	(
	Event *ev
	)

{
	if (!m_isPressed) {
		return;
	}

	uWinMan.setFirstResponder(NULL);
	m_isPressed = false;
	OnActivated(NULL);
}

void UIFloatingWindow::Dragged
	(
	Event *ev
	)

{
	if (!m_isPressed) {
		return;
	}

	UIPoint2D point = MouseEventToClientPoint(ev);
	UIPoint2D newpos;

	newpos.x = point.x - m_clickOffset.x;
	newpos.y = point.y - m_clickOffset.y;
	newpos.x += m_frame.pos.x;
	newpos.y += m_frame.pos.y;

	setFrame(UIRect2D(newpos, getSize()));
}

void UIFloatingWindow::SizeChanged
	(
	Event *ev
	)

{
	UIRect2D childRect;
	childRect.pos.x = 2.0;
	childRect.pos.y = 20.0;
	childRect.size.width = m_frame.size.width - 4.0;
	childRect.size.height = m_frame.size.height - 22.0;

	if (childRect.size.width <= 0.0 || childRect.size.height <= 0.0) {
		return;
	}

	m_childspace->setFrame(childRect);
	m_closeButton->setFrame(UIRect2D(m_frame.size.width - 18.0, 2.0, 16.0, 16.0));
	m_minimizeButton->setFrame(UIRect2D(m_frame.size.width - 36.0, 2.0, 16.0, 16.0));
}

void UIFloatingWindow::OnActivated
	(
	Event *ev
	)

{
	UIWidget* next;

	if (!IsActive()) {
		return;
	}

	for (next = m_childspace->getFirstChild(); next; next = m_childspace->getNextChild(next))
	{
		if (next->CanActivate())
		{
			uWinMan.ActivateControl(next);
			break;
		}
	}
}

void UIFloatingWindow::OnDeactivated
	(
	Event *ev
	)

{
	PostEvent(EV_Remove, 0.0);
}

void UIFloatingWindow::Create
	(
	UIWidget *parent,
	const UIRect2D& rect,
	const char *title,
	const UColor& bgColor,
	const UColor& fgColor
	)

{
	m_titleColor = bgColor;
	m_textColor = fgColor;
	setTitle(title);
	// actually create the window
	InitFrame(parent, rect.pos.x, rect.pos.y, rect.size.width, rect.size.height, 0, "verdana-14");
}

void UIFloatingWindow::Draw
	(
	void
	)

{
	UColor titleBar;
	UColor textColor;
	UBorderColor titleBorder;
	UBorderColor windowBorder;

	if (IsThisOrChildActive())
	{
		UColor newColor;

		titleBar = m_titleColor;
		textColor = m_textColor;

		UColorHSV tmp(titleBar);
		tmp.s *= 0.75;
		tmp.v *= 1.333;
		newColor = tmp;

		titleBorder.dark = newColor;
		titleBorder.reallydark = newColor;
		titleBorder.light = newColor;

	}
	else
	{
		UColor newColor(0.1125, 0.147, 0.2085, 1.0);

		titleBar = newColor;
		textColor = UBlack;

		titleBorder.CreateSolidBorder(newColor, DARK);
		titleBorder.reallydark = newColor;
		titleBorder.light = newColor;
	}

	UColor newColor(0.15, 0.196, 0.278, 1.0);
	newColor.ScaleColor(0.666f);

	windowBorder.dark = newColor;
	windowBorder.reallydark = newColor;
	windowBorder.light = newColor;

	setForegroundColor(textColor);
	m_closeButton->setForegroundColor(textColor);
	m_closeButton->setBackgroundColor(titleBar, true);
	m_minimizeButton->setForegroundColor(textColor);
	m_minimizeButton->setBackgroundColor(titleBar, true);

	if (!IsMinimized())
	{
		DrawBoxWithSolidBorder(
			getClientFrame(),
			UColor(0.15, 0.195, 0.278, 1.0),
			UColor(0.075, 0.097, 0.139, 1.0),
			2,
			2,
			1.0
		);
	}

	DrawBox(
		0.0,
		0.0,
		m_frame.size.width,
		m_frame.size.height > 20.0 ? 20.0 : m_frame.size.height,
		titleBar,
		1.0
	);

	Draw3DBox(
		0.0,
		0.0,
		m_frame.size.width,
		m_frame.size.height > 20.0 ? 20.0 : m_frame.size.height,
		false,
		titleBorder,
		1.0
	);

	DrawTitle(3.0, 3.0);
}

UIChildSpaceWidget *UIFloatingWindow::getChildSpace
	(
	void
	)

{
	return m_childspace;
}

bool UIFloatingWindow::IsMinimized
	(
	void
	)

{
	return m_minimized;
}
