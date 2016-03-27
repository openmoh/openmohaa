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

CLASS_DECLARATION( UIWidget, UIListBase, NULL )
{
	{ NULL, NULL }
};

UIListBase::UIListBase()
{
	// FIXME: stub
}

void UIListBase::TrySelectItem
	(
	int which
	)

{
	// FIXME: stub
}

qboolean UIListBase::KeyEvent
	(
	int key,
	unsigned int time
	)

{
	// FIXME: stub
	return qfalse;
}

void UIListBase::FrameInitialized
	(
	void
	)

{
	// FIXME: stub
}

int UIListBase::getCurrentItem
	(
	void
	)

{
	// FIXME: stub
	return 0;
}

int UIListBase::getNumItems
	(
	void
	)

{
	// FIXME: stub
	return 0;
}

void  UIListBase::DeleteAllItems
	(
	void
	)

{
	// FIXME: stub
}

void UIListBase::DeleteItem
	(
	int which
	)

{
	// FIXME: stub
}

UIVertScroll *UIListBase::GetScrollBar
	(
	void
	)

{
	// FIXME: stub
	return NULL;
}

void UIListBase::SetUseScrollBar
	(
	qboolean bUse
	)

{
	// FIXME: stub
}

ListItem::ListItem()
{
	// FIXME: stub
}

ListItem::ListItem
	(
	str string,
	int index,
	str command
	)

{
	// FIXME: stub
}

CLASS_DECLARATION( UIListBase, UIListBox, NULL )
{
	{ NULL, NULL }
};

UIListBox::UIListBox()
{
	// FIXME: stub
}

void UIListBox::Draw
	(
	void
	)

{
	// FIXME: stub
}

void UIListBox::MousePressed
	(
	Event *ev
	)

{
	// FIXME: stub
}

void UIListBox::MouseReleased
	(
	Event *ev
	)

{
	// FIXME: stub
}

void UIListBox::DeleteAllItems
	(
	Event *ev
	)

{
	// FIXME: stub
}

void UIListBox::SetListFont
	(
	Event *ev
	)

{
	// FIXME: stub
}

void UIListBox::TrySelectItem
	(
	int which
	)

{
	// FIXME: stub
}

void UIListBox::AddItem
	(
	const char *item,
	const char *command
	)

{
	// FIXME: stub
}

void UIListBox::AddItem
	(
	int index,
	const char *command
	)

{
	// FIXME: stub
}

void UIListBox::FrameInitialized
	(
	void
	)

{
	// FIXME: stub
}

void UIListBox::LayoutAddListItem
	(
	Event *ev
	)

{
	// FIXME: stub
}

void UIListBox::LayoutAddConfigstringListItem
	(
	Event *ev
	)

{
	// FIXME: stub
}

str UIListBox::getItemText
	(
	int which
	)

{
	// FIXME: stub
	return "";
}

int UIListBox::getNumItems
	(
	void
	)

{
	// FIXME: stub
	return 0;
}

void UIListBox::DeleteAllItems
	(
	void
	)

{
	// FIXME: stub
}

void UIListBox::DeleteItem
	(
	int which
	)

{
	// FIXME: stub
}
