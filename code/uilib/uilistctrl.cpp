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

static UISize2D s_columnpadding;

CLASS_DECLARATION( UIListBase, UIListCtrl, NULL )
{
	{ &W_SizeChanged,		&UIListCtrl::OnSizeChanged },
	{ &W_LeftMouseDown,		&UIListCtrl::MousePressed },
	{ &W_LeftMouseUp,		&UIListCtrl::MouseReleased },
	{ &W_LeftMouseDragged,	&UIListCtrl::MouseDragged },
	{ &W_MouseEntered,		&UIListCtrl::MouseEntered },
	{ NULL, NULL }
};

UIListCtrl::UIListCtrl()
{
	m_clickState.point.x = m_clickState.point.y = 0;
	m_sizestate.column = 0;
	m_iLastSortColumn = 0;
	m_bDrawHeader = qtrue;
	m_headerfont = NULL;

	Connect(this, W_SizeChanged, W_SizeChanged);
	m_background_color = UColor(0.02f, 0.07f, 0.005f, 1.0f);
	m_foreground_color = UHudColor;
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
	if (m_bDrawHeader) {
		DrawColumns();
	}

	DrawContent();
}

int UIListCtrl::getHeaderHeight
	(
	void
	)

{
	if (!m_bDrawHeader) {
		return 0;
	}

	if (m_headerfont) {
		return (int)(m_headerfont->getHeight(m_bVirtual) + (s_columnpadding.height + s_columnpadding.height) * m_vVirtualScale[1]);
	}

	if (m_font) {
		return (int)(m_font->getHeight(m_bVirtual) + (s_columnpadding.height + s_columnpadding.height) * m_vVirtualScale[1]);
	}

	return (int)((s_columnpadding.height + s_columnpadding.height) * m_vVirtualScale[1]);
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
	m_sizestate.column = 0;

	if (uWinMan.getFirstResponder() == this) {
		uWinMan.setFirstResponder(NULL);
	}
}

void UIListCtrl::MouseEntered
	(
	Event *ev
	)

{
	uWinMan.ActivateControl(this);
}

void UIListCtrl::OnSizeChanged
	(
	Event *ev
	)

{
	if (!m_vertscroll) {
		return;
	}

	m_vertscroll->InitFrame(this, m_frame.size.width - 16.0, 0.0, 16.0, m_frame.size.height, -1);
	m_vertscroll->setPageHeight((m_frame.size.height - getHeaderHeight()) / m_font->getHeight(m_bVirtual));
	m_vertscroll->setNumItems(m_itemlist.NumObjects());
}

void UIListCtrl::DrawColumns
	(
	void
	)

{
	int atleft;
	int i;
	int height;
	UIFont* pFont;
	UColor columnColor, textColor;

	atleft = 0;
	columnColor = UColor(0.07f, 0.06f, 0.005f, 1.0f);
	textColor = UHudColor;

	pFont = m_headerfont;
	if (!pFont) pFont = m_font;

	height = (s_columnpadding.height + s_columnpadding.height) * m_vVirtualScale[1] + pFont->getHeight(m_bVirtual);
	pFont->setColor(textColor);

	for (i = 1; i <= m_columnlist.NumObjects(); i++)
	{
		const columndef_t& column = m_columnlist.ObjectAt(i);

		DrawBoxWithSolidBorder(
			UIRect2D(atleft, 0, column.width + m_vVirtualScale[1], height),
			columnColor,
			textColor,
			1,
			3,
			m_local_alpha
		);

		pFont->Print(
			atleft / m_vVirtualScale[0] + s_columnpadding.width,
			s_columnpadding.height,
			Sys_LV_CL_ConvertString(column.title.c_str()),
			-1,
			m_bVirtual
		);

		atleft += column.width;
	}

	if (m_frame.size.width > atleft)
	{
		DrawBoxWithSolidBorder(
			UIRect2D(atleft, 0, m_frame.size.width - atleft + 1.0, height),
			columnColor,
			textColor,
			1,
			3,
			m_local_alpha
		);
	}
}

void UIListCtrl::DrawContent
	(
	void
	)

{
	int item;
	int height, headerheight, iItemHeight;
	UIFont* pFont;
	int top;
	UColor selColor, selText;
	UColor backColor, textColor;

	height = m_font->getHeight(m_bVirtual);
	selColor = UColor(0.21f, 0.18f, 0.015f, 1.0f);
	selText = UColor(0.9f, 0.8f, 0.6f, 1.0f);
	backColor = m_background_color;
	textColor = m_foreground_color;

	if (m_headerfont) {
		headerheight = m_headerfont->getHeight(m_bVirtual);
	} else {
		headerheight = 0;
	}

	selColor.a = m_local_alpha;
	selText.a = m_local_alpha;
	backColor.a = m_local_alpha;
	textColor.a = m_local_alpha;
	top = getHeaderHeight();
	if (m_vertscroll) {
		item = m_vertscroll->getTopItem() + 1;
	} else {
		item = 1;
	}

	for (; item <= m_itemlist.NumObjects(); item++)
	{
		UIListCtrlItem* theitem;
		int col;
		int atleft;
		UColor* itemBg;
		UColor* itemText;

		theitem = m_itemlist.ObjectAt(item);
		atleft = 0;

		if (item == m_currentItem)
		{
			itemBg = &selColor;
			itemText = &selText;
		}
		else
		{
			itemBg = &backColor;
			itemText = &textColor;
		}

		if (theitem->IsHeaderEntry() && m_headerfont) {
			pFont = m_headerfont;
			iItemHeight = headerheight;
		} else {
			pFont = m_font;
			iItemHeight = height;
		}

		pFont->setColor(*itemText);
		for (col = 1; col <= m_columnlist.NumObjects(); col++)
		{
			columndef_t& column = m_columnlist.ObjectAt(col);
			UIRect2D drawRect(atleft, top, column.width, iItemHeight);
			griditemtype_t itemtype = theitem->getListItemType(column.name);

			switch (itemtype)
			{
			case TYPE_STRING:
				DrawBox(drawRect, *itemBg, m_local_alpha);
				pFont->Print(
					drawRect.pos.x / m_vVirtualScale[0] + 1.0,
					drawRect.pos.y / m_vVirtualScale[1],
					Sys_LV_CL_ConvertString(theitem->getListItemString(column.name)),
					-1,
					m_bVirtual
				);
				break;
			case TYPE_OWNERDRAW:
				theitem->DrawListItem(column.name, drawRect, item == m_currentItem, pFont);
				break;
			}

			atleft += column.width;
		}

		if (m_frame.size.width > atleft)
		{
			DrawBox(
				atleft,
				top,
				m_frame.size.width - atleft,
				height,
				*itemBg,
				m_local_alpha
			);
		}

		top += iItemHeight;
		if (iItemHeight + top > m_frame.size.height) {
			break;
		}
	}
}

void UIListCtrl::FrameInitialized
	(
	void
	)

{
	UIListBase::FrameInitialized();
	OnSizeChanged(NULL);
}

void UIListCtrl::SetDrawHeader
	(
	qboolean bDrawHeader
	)

{
	m_bDrawHeader = bDrawHeader;
}

void UIListCtrl::AddItem
	(
	UIListCtrlItem *item
	)

{
	m_itemlist.AddObject(item);
	if (m_vertscroll) {
		m_vertscroll->setNumItems(m_itemlist.NumObjects());
	}
}

void UIListCtrl::InsertItem
	(
	UIListCtrlItem *item, int where
	)

{
	if (where > 0 && where <= m_itemlist.NumObjects())
	{
		int i;

		m_itemlist.AddObject(NULL);

		for (i = m_itemlist.NumObjects(); i > where; i++) {
			m_itemlist.SetObjectAt(i, m_itemlist.ObjectAt(i - 1));
		}

		m_itemlist.SetObjectAt(where, item);
	}
	else
	{
		m_itemlist.AddObject(item);
	}

	if (m_vertscroll) {
		m_vertscroll->setNumItems(m_itemlist.NumObjects());
	}
}

int UIListCtrl::FindItem
	(
	UIListCtrlItem *item
	)

{
	return m_itemlist.IndexOfObject(item);
}

UIListCtrlItem *UIListCtrl::GetItem
	(
	int item
	)

{
	return m_itemlist.ObjectAt(item);
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
	columndef_t column;

	column.title = title;
	column.name = name;
	column.width = width;
	column.numeric = numeric;
	column.reverse_sort = reverse_sort;

	m_columnlist.AddObject(column);
}

void UIListCtrl::RemoveAllColumns
	(
	void
	)

{
	m_columnlist.ClearObjectList();
}

int UIListCtrl::getNumItems
	(
	void
	)

{
	return m_itemlist.NumObjects();
}

void UIListCtrl::DeleteAllItems
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

void UIListCtrl::DeleteItem
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
	SortByColumn(m_iLastSortColumn);
}

void UIListCtrl::setCompareFunction
	(
	int( *func ) ( const UIListCtrlItem *i1, const UIListCtrlItem *i2, int columnname )
	)

{
	m_comparefunction = func;
}

void UIListCtrl::setHeaderFont
	(
	const char *name
	)

{
	if (m_headerfont) {
		delete m_headerfont;
	}

	if (name) {
		m_headerfont = new UIFont(name);
	} else {
		m_headerfont = NULL;
	}
}
