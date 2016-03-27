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

griditemtype_t UIListCtrlItem::getListItemType
	(
	int which
	) const

{
	// FIXME: stub
	return TYPE_STRING;
}

str UIListCtrlItem::getListItemString
	(
	int which
	) const

{
	// FIXME: stub
	return "";
}

int UIListCtrlItem::getListItemValue
	(
	int which
	) const

{
	// FIXME: stub
	return 0;
}

void UIListCtrlItem::DrawListItem
	(
	int,
	UIRect2D const &,
	bool, UIFont *
	)

{
	// FIXME: stub
}

qboolean UIListCtrlItem::IsHeaderEntry
	(
	void
	) const

{
	// FIXME: stub
	return qfalse;
}

CLASS_DECLARATION( UIListBase, UIListCtrl, NULL )
{
	{ NULL, NULL }
};

UIListCtrl::UIListCtrl()
{
	// FIXME: stub
}

int UIListCtrl::StringCompareFunction
	(
	const UIListCtrlItem *i1,
	const UIListCtrlItem *i2,
	int columnname
	)

{
	// FIXME: stub
	return 0;
}

int UIListCtrl::StringNumberCompareFunction
	(
	const UIListCtrlItem *i1,
	const UIListCtrlItem *i2,
	int columnname
	)

{
	// FIXME: stub
	return 0;
}

int UIListCtrl::QsortCompare
	(
	const void *e1,
	const void *e2
	)

{
	// FIXME: stub
	return 0;
}

void UIListCtrl::Draw
	(
	void
	)

{
	// FIXME: stub
}

int UIListCtrl::getHeaderHeight
	(
	void
	)

{
	// FIXME: stub
	return 0;
}

void UIListCtrl::MousePressed
	(
	Event *ev
	)

{
	// FIXME: stub
}

void UIListCtrl::MouseDragged
	(
	Event *ev
	)

{
	// FIXME: stub
}

void UIListCtrl::MouseReleased
	(
	Event *ev
	)

{
	// FIXME: stub
}

void UIListCtrl::MouseEntered
	(
	Event *ev
	)

{
	// FIXME: stub
}

void UIListCtrl::OnSizeChanged
	(
	Event *ev
	)

{
	// FIXME: stub
}

void UIListCtrl::DrawColumns
	(
	void
	)

{
	// FIXME: stub
}

void UIListCtrl::DrawContent
	(
	void
	)

{
	// FIXME: stub
}

void UIListCtrl::FrameInitialized
	(
	void
	)

{
	// FIXME: stub
}

void UIListCtrl::SetDrawHeader
	(
	qboolean bDrawHeader
	)

{
	// FIXME: stub
}

void UIListCtrl::AddItem
	(
	UIListCtrlItem *item
	)

{
	// FIXME: stub
}

void UIListCtrl::InsertItem
	(
	UIListCtrlItem *item, int where
	)

{
	// FIXME: stub
}

int UIListCtrl::FindItem
	(
	UIListCtrlItem *item
	)

{
	// FIXME: stub
	return 0;
}

UIListCtrlItem *UIListCtrl::GetItem
	(
	int item
	)

{
	// FIXME: stub
	return NULL;
}

void UIListCtrl::AddColumn
	(
	str title,
	int name,
	int width,
	bool numeric,
	bool reverse_sort
	)

{
	// FIXME: stub
}

void UIListCtrl::RemoveAllColumns
	(
	void
	)

{
	// FIXME: stub
}

int UIListCtrl::getNumItems
	(
	void
	)

{
	// FIXME: stub
	return 0;
}

void UIListCtrl::DeleteAllItems
	(
	void
	)

{
	// FIXME: stub
}

void UIListCtrl::DeleteItem
	(
	int which
	)

{
	// FIXME: stub
}

void UIListCtrl::SortByColumn
	(
	int column
	)

{
	// FIXME: stub
}

void UIListCtrl::SortByLastSortColumn
	(
	void
	)

{
	// FIXME: stub
}

void UIListCtrl::setCompareFunction
	(
	int( *func ) ( const UIListCtrlItem *i1, const UIListCtrlItem *i2, int columnname )
	)

{
	// FIXME: stub
}

void UIListCtrl::setHeaderFont
	(
	const char *name
	)

{
	// FIXME: stub
}
