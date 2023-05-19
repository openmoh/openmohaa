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
	m_currentItem = 0;
	m_vertscroll = 0;
	m_bUseVertScroll = 1;
	AllowActivate(true);
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
	if (m_vertscroll) {
		delete m_vertscroll;
	}

	m_vertscroll = new UIVertScroll();
	m_vertscroll->InitFrame(this, m_frame.size.width - 16.0, 0.0, 16.0, m_frame.size.height, -1);
	m_vertscroll->setTopItem(0);
	// FIXME: stub
}

int UIListBase::getCurrentItem
	(
	void
	)

{
	return m_currentItem;
}

int UIListBase::getNumItems
	(
	void
	)

{
	return -1;
}

void  UIListBase::DeleteAllItems
	(
	void
	)

{
}

void UIListBase::DeleteItem
	(
	int which
	)

{
}

UIVertScroll *UIListBase::GetScrollBar
	(
	void
	)

{
	return m_vertscroll;
}

void UIListBase::SetUseScrollBar
	(
	qboolean bUse
	)

{
	m_bUseVertScroll = bUse;

	if (bUse)
	{
		if (!m_vertscroll) {
			m_vertscroll = new UIVertScroll();
		}

		m_vertscroll->InitFrame(this, m_frame.size.width - 16.0, 0.0, 16.0, m_frame.size.height, -1);
	}
	else
	{
		if (m_vertscroll) {
			delete m_vertscroll;
			m_vertscroll = NULL;
		}
	}
}

ListItem::ListItem()
{
	index = -1;
}

ListItem::ListItem
	(
	str string,
	int index,
	str command
	)

{
	this->string = string;
	this->index = index;
	this->command = command;
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
	ListItem* li;

	li = new ListItem();
	if (!li) {
		uii.Sys_Error(ERR_DROP, "Couldn't create list item\n");
	}

	li->string = item;
	if (command) {
		li->command = command;
	}

	m_itemlist.AddObject(li);

	if (!m_currentItem) {
		m_currentItem = 1;
	}

	if (m_vertscroll) {
		m_vertscroll->setNumItems(m_itemlist.NumObjects());
	}
}

void UIListBox::AddItem
	(
	int index,
	const char *command
	)

{
	ListItem* li;

	li = new ListItem();
	if (!li) {
		uii.Sys_Error(ERR_DROP, "Couldn't create list item\n");
	}

	li->index = index;
	if (command) {
		li->command = command;
	}

	m_itemlist.AddObject(li);

	if (!m_currentItem) {
		m_currentItem = 1;
	}

	if (m_vertscroll) {
		m_vertscroll->setNumItems(m_itemlist.NumObjects());
	}
}

void UIListBox::FrameInitialized
	(
	void
	)

{
	UIListBase::FrameInitialized();

	if (m_vertscroll) {
		m_vertscroll->setPageHeight(m_frame.size.height / m_font->getHeight(m_bVirtual));
		m_vertscroll->setNumItems(m_itemlist.NumObjects());
	}
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
	return m_itemlist.ObjectAt(which)->string;
}

int UIListBox::getNumItems
	(
	void
	)

{
	return m_itemlist.NumObjects();
}

void UIListBox::DeleteAllItems
	(
	void
	)

{
	m_itemlist.ClearObjectList();
	m_currentItem = 0;

	if (m_vertscroll)
	{
		m_vertscroll->setNumItems(0);
		m_vertscroll->setTopItem(0);
	}
}

void UIListBox::DeleteItem
	(
	int which
	)

{
	m_itemlist.RemoveObjectAt(which);

	if (m_vertscroll) {
		m_vertscroll->setNumItems(m_itemlist.NumObjects());
	}

	if (m_currentItem > getNumItems()) {
		TrySelectItem(getNumItems());
	}
}
