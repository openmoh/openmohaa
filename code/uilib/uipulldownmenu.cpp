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

CLASS_DECLARATION( UIWidget, UIPulldownMenu, NULL )
{
	{ NULL, NULL }
};

UIPulldownMenu::UIPulldownMenu()
{
	// FIXME: stub
}

UIRect2D UIPulldownMenu::getAlignmentRect
	(
	UIWidget *parent
	)

{
	// FIXME: stub
	return UIRect2D();
}

float UIPulldownMenu::getDescWidth
	(
	uipull_describe *desc
	)

{
	// FIXME: stub
	return 0.0f;
}

float UIPulldownMenu::getDescHeight
	(
	uipull_describe *desc
	)

{
	// FIXME: stub
	return 0.0f;
}

uipull_describe *UIPulldownMenu::getPulldown
	(
	str title
	)

{
	// FIXME: stub
	return NULL;
}

void UIPulldownMenu::HighlightBGColor
	(
	Event *ev
	)

{
	// FIXME: stub
}

void UIPulldownMenu::HighlightFGColor
	(
	Event *ev
	)

{
	// FIXME: stub
}


void UIPulldownMenu::Create
	(
	UIWidget *parent,
	Listener *listener,
	const UIRect2D& rect
	)

{
	// FIXME: stub
}

void UIPulldownMenu::CreateAligned
	(
	UIWidget *parent,
	Listener *listener
	)

{
	// FIXME: stub
}

void UIPulldownMenu::MousePressed
	(
	Event *ev
	)

{
	// FIXME: stub
}

void UIPulldownMenu::MouseDragged
	(
	Event *ev
	)

{
	// FIXME: stub
}

void UIPulldownMenu::MouseReleased
	(
	Event *ev
	)

{
	// FIXME: stub
}

void UIPulldownMenu::ChildKilled
	(
	Event *ev
	)

{
	// FIXME: stub
}

void UIPulldownMenu::AddUIPopupDescribe
	(
	const char *title,
	uipopup_describe *d
	)

{
	// FIXME: stub
}

void UIPulldownMenu::setHighlightFGColor
	(
	UColor c
	)

{
	// FIXME: stub
}

void UIPulldownMenu::setHighlightBGColor
	(
	UColor c
	)

{
	// FIXME: stub
}

void UIPulldownMenu::setPopupHighlightFGColor
	(
	str menu,
	UColor c
	)

{
	// FIXME: stub
}

void UIPulldownMenu::setPopupHighlightBGColor
	(
	str menu,
	UColor c
	)

{
	// FIXME: stub
}

void UIPulldownMenu::setPopupFGColor
	(
	str menu,
	UColor c
	)

{
	// FIXME: stub
}

void UIPulldownMenu::setPopupBGColor
	(
	str menu,
	UColor c
	)

{
	// FIXME: stub
}

void UIPulldownMenu::setPulldownShader
	(
	const char *title,
	UIReggedMaterial *mat
	)

{
	// FIXME: stub
}

void UIPulldownMenu::setSelectedPulldownShader
	(
	const char *title,
	UIReggedMaterial *mat
	)

{
	// FIXME: stub
}

void UIPulldownMenu::Realign
	(
	void
	)

{
	// FIXME: stub
}

void UIPulldownMenu::Draw
	(
	void
	)

{
	// FIXME: stub
}
