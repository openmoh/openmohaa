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

CLASS_DECLARATION( UIWidget, UILanGameList, NULL )
{
	{ NULL, NULL }
};

UILanGameList::UILanGameList()
{
	// FIXME: stub
}

void UILanGameList::CreateServerWidgets
	(
	void
	)

{
	// FIXME: stub
}

void UILanGameList::DestroyServerWidgets
	(
	void
	)

{
	// FIXME: stub
}

void UILanGameList::RepositionServerWidgets
	(
	void
	)

{
	// FIXME: stub
}

void UILanGameList::DrawNoServers
	(
	UIRect2D frame
	)

{
	// FIXME: stub
}

void UILanGameList::AddColumn
	(
	str sName,
	UIReggedMaterial *pMaterial,
	int iWidth,
	Container<str> *csEntries
	)

{
	// FIXME: stub
}

void UILanGameList::AddNoServer
	(
	void
	)

{
	// FIXME: stub
}

void UILanGameList::UpdateServers
	(
	void
	)

{
	// FIXME: stub
}


void UILanGameList::FrameInitialized
	(
	void
	)

{
	// FIXME: stub
}

void UILanGameList::EventScanNetwork
	(
	Event *ev
	)

{
	// FIXME: stub
}

void UILanGameList::EventScaningNetwork
	(
	Event *ev
	)

{
	// FIXME: stub
}

bool UILanGameList::isDying
	(
	void
	)

{
	// FIXME: stub
	return false;
}

void UILanGameList::Draw
	(
	void
	)

{
	// FIXME: stub
}

qboolean UILanGameList::KeyEvent
	(
	int key,
	unsigned int time
	)

{
	// FIXME: stub
	return qfalse;
}

void UILanGameList::Highlight
	(
	UIWidget *wid
	)

{
	// FIXME: stub
}

void UILanGameList::Connect
	(
	void
	)

{
	// FIXME: stub
}

void UILanGameList::EventConnect
	(
	Event *ev
	)

{
	// FIXME: stub
}

void UILanGameList::PlayEnterSound
	(
	void
	)

{
	// FIXME: stub
}

qboolean UILanGameList::SetActiveRow
	(
	UIWidget *w
	)

{
	// FIXME: stub
	return qfalse;
}

void UILanGameList::Realign
	(
	void
	)

{
	// FIXME: stub
}

CLASS_DECLARATION( UILabel, UILanGameListLabel, NULL )
{
	{ NULL, NULL }
};

UILanGameListLabel::UILanGameListLabel()
{

}

UILanGameListLabel::UILanGameListLabel
	(
	UILanGameList *list
	)

{

}

void UILanGameListLabel::Pressed
	(
	Event *ev
	)

{

}

void UILanGameListLabel::Unpressed
	(
	Event *ev
	)

{

}
