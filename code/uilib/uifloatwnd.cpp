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

CLASS_DECLARATION( UIWidget, UIChildSpaceWidget, NULL )
{
	{ NULL, NULL }
};

UIChildSpaceWidget::UIChildSpaceWidget()
{
	// FIXME: stub
}

qboolean UIChildSpaceWidget::KeyEvent
	(
	int key,
	unsigned int time
	)

{
	// FIXME: stub
	return qfalse;
}

CLASS_DECLARATION( UIWidget, UIFloatingWindow, NULL )
{
	{ NULL, NULL }
};

Event UIFloatingWindow::W_ClosePressed;
Event UIFloatingWindow::W_MinimizePressed;

UIFloatingWindow::UIFloatingWindow()
{
	// FIXME: stub
}

void UIFloatingWindow::FrameInitialized
	(
	void
	)

{
	// FIXME: stub
}

void UIFloatingWindow::FrameInitialized
	(
	bool bHasDragBar
	)

{
	// FIXME: stub
}

void UIFloatingWindow::ClosePressed
	(
	Event *ev
	)

{
	// FIXME: stub
}

void UIFloatingWindow::MinimizePressed
	(
	Event *ev
	)

{
	// FIXME: stub
}

void UIFloatingWindow::Pressed
	(
	Event *ev
	)

{
	// FIXME: stub
}

void UIFloatingWindow::Released
	(
	Event *ev
	)

{
	// FIXME: stub
}

void UIFloatingWindow::Dragged
	(
	Event *ev
	)

{
	// FIXME: stub
}

void UIFloatingWindow::SizeChanged
	(
	Event *ev
	)

{
	// FIXME: stub
}

void UIFloatingWindow::OnActivated
	(
	Event *ev
	)

{
	// FIXME: stub
}

void UIFloatingWindow::OnDeactivated
	(
	Event *ev
	)

{
	// FIXME: stub
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
	// FIXME: stub
}

void UIFloatingWindow::Draw
	(
	void
	)

{
	// FIXME: stub
}

UIChildSpaceWidget *UIFloatingWindow::getChildSpace
	(
	void
	)

{
	// FIXME: stub
	return NULL;
}

bool UIFloatingWindow::IsMinimized
	(
	void
	)

{
	// FIXME: stub
	return false;
}
