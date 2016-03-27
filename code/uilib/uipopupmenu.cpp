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

CLASS_DECLARATION( UIWidget, UIPopupMenu, NULL )
{
	{ NULL, NULL }
};

UIPopupMenu::UIPopupMenu()
{
	// FIXME: stub
}

uipopup_describe *UIPopupMenu::getDescribeFromPoint
	(
	const UIPoint2D& p
	)

{
	// FIXME: stub
	return NULL;
}

void UIPopupMenu::MakeSubMenu
	(
	void
	)

{
	// FIXME: stub
}

float UIPopupMenu::getDescribeHeight
	(
	uipopup_describe *d
	)

{
	// FIXME: stub
	return 0.0f;
}

float UIPopupMenu::getDescribeWidth
	(
	uipopup_describe *d
	)

{
	// FIXME: stub
	return 0.0f;
}

bool UIPopupMenu::MouseInSubmenus
	(
	void
	)

{
	// FIXME: stub
	return false;
}


void UIPopupMenu::Create
	(
	Container<uipopup_describe*> *describe,
	Listener *listener,
	const UIPoint2D& where
	)

{
	// FIXME: stub
}

void UIPopupMenu::Create
	(
	Container<uipopup_describe*> *describe,
	Listener *listener,
	const UIRect2D& createRect,
	uipopup_where where,
	qboolean bVirtualSize,
	float width
	)

{
	// FIXME: stub
}

void UIPopupMenu::YouAreASubmenu
	(
	UIPopupMenu *me
	)

{
	// FIXME: stub
}

void UIPopupMenu::Draw
	(
	void
	)

{
	// FIXME: stub
}

void UIPopupMenu::MouseReleased
	(
	Event *ev
	)

{
	// FIXME: stub
}

void UIPopupMenu::MouseMoved
	(
	Event *ev
	)

{
	// FIXME: stub
}

void UIPopupMenu::MouseExited
	(
	Event *ev
	)

{
	// FIXME: stub
}

void UIPopupMenu::Dismiss
	(
	void
	)

{
	// FIXME: stub
}

void UIPopupMenu::DismissEvent
	(
	Event *ev
	)

{
	// FIXME: stub
}

void UIPopupMenu::setAutoDismiss
	(
	bool b
	)

{
	// FIXME: stub
}

void UIPopupMenu::setNextSelection
	(
	void
	)

{
	// FIXME: stub
}

void UIPopupMenu::setPrevSelection
	(
	void
	)

{
	// FIXME: stub
}

uipopup_describe *UIPopupMenu::getSelectedDescribe
	(
	void
	)

{
	// FIXME: stub
	return NULL;
}

void UIPopupMenu::setSelection
	(
	int sel
	)

{
	// FIXME: stub
}

void UIPopupMenu::setHighlightFGColor
	(
	UColor c
	)

{
	// FIXME: stub
}

void UIPopupMenu::setHighlightBGColor
	(
	UColor c
	)

{
	// FIXME: stub
}

void UIPopupMenu::setPopupHighlightBGColor
	(
	str menu,
	UColor c
	)

{
	// FIXME: stub
}

void UIPopupMenu::setPopupHighlightFGColor
	(
	str menu,
	UColor c
	)

{
	// FIXME: stub
}

void UIPopupMenu::getPulldown
	(
	str title
	)

{
	// FIXME: stub
}

