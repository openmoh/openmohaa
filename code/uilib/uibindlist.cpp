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

Event EV_UIFakkBindList_Filename
	(
	"filename",
	EV_DEFAULT,
	"s",
	"filename",
	"Filename that holds bind definitions"
	);

Event EV_UIFakkBindList_StopBind
	(
	"stopbind",
	EV_DEFAULT,
	NULL,
	NULL,
	"stops trying to bind a key to a command"
	);

CLASS_DECLARATION( UIWidget, UIFakkBindList, NULL )
{
	{ &EV_UIFakkBindList_Filename,		&UIFakkBindList::Filename },
	{ &EV_UIFakkBindList_StopBind,		&UIFakkBindList::StopBind },
	{ NULL, NULL }
};


UIFakkBindList::UIFakkBindList()
{
	// FIXME: stub
}

void UIFakkBindList::CreateBindWidgets
	(
	void
	)

{
	// FIXME: stub
}

void UIFakkBindList::DestroyBindWidgets
	(
	void
	)

{
	// FIXME: stub
}

void UIFakkBindList::RepositionBindWidgets
	(
	void
	)

{
	// FIXME: stub
}

void UIFakkBindList::DrawPressKey
	(
	UIRect2D frame
	)

{
	// FIXME: stub
}

void UIFakkBindList::FrameInitialized
	(
	void
	)

{
	// FIXME: stub
}

void UIFakkBindList::Filename
	(
	Event *ev
	)

{
	// FIXME: stub
}

void UIFakkBindList::StopBind
	(
	Event *ev
	)

{
	// FIXME: stub
}

void UIFakkBindList::setBind
	(
	bind_t *b
	)

{
	// FIXME: stub
}

void UIFakkBindList::Draw
	(
	void
	)

{
	// FIXME: stub
}

bool UIFakkBindList::isDying
	(
	void
	)

{
	// FIXME: stub
	return false;
}

qboolean UIFakkBindList::KeyEvent
	(
	int key,
	unsigned int time
	)

{
	// FIXME: stub
	return qfalse;
}

void UIFakkBindList::Highlight
	(
	UIWidget *wid
	)

{
	// FIXME: stub
}

void UIFakkBindList::PlayEnterSound
	(
	void
	)

{
	// FIXME: stub
}

qboolean UIFakkBindList::SetActiveRow
	(
	UIWidget *w
	)

{
	// FIXME: stub
	return qfalse;
}

void UIFakkBindList::Realign
	(
	void
	)

{
	// FIXME: stub
}

CLASS_DECLARATION( UILabel, UIFakkBindListLabel, NULL )
{
	{ NULL, NULL }
};

UIFakkBindListLabel::UIFakkBindListLabel()
{
	// FIXME: stub
}

UIFakkBindListLabel::UIFakkBindListLabel
	(
	UIFakkBindList *list
	)

{
	// FIXME: stub
}

void UIFakkBindListLabel::Pressed
	(
	Event *ev
	)

{
	// FIXME: stub
}

void UIFakkBindListLabel::Draw
	(
	void
	)

{
	// FIXME: stub
}
