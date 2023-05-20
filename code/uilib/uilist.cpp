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

CLASS_DECLARATION( UIWidget, UIList, NULL )
{
	{ &W_LeftMouseDown,			&UIList::Pressed },
	{ &W_LeftMouseUp,			&UIList::Released },
	{ &EV_Layout_AddListItem,	&UIList::LayoutAddListItem },
	{ NULL, NULL }
};

UIList::UIList()
{
	m_arrow_width = 15.0;
	m_arrow_width = 0.0;
	m_currentItem = 0;
	m_next_arrow_region = 0;
	m_prev_arrow_region = 0;
	m_prev_arrow_depressed = 0;
	m_next_arrow_depressed = 0;

	AllowActivate(true);

	m_prev_arrow = uWinMan.RegisterShader("gfx/2d/arrow_left.tga");
	if (!m_prev_arrow) {
		uii.Sys_Printf("UIList::Ulist : Could not register shader gfx/2d/arrow_left.tga");
	}



	m_next_arrow = uWinMan.RegisterShader("gfx/2d/arrow_right.tga");
	if (!m_next_arrow) {
		uii.Sys_Printf("UIList::Ulist : Could not register shader gfx/2d/arrow_right.tga");
	}
}

UIList::~UIList()
{
	for (int i = m_itemlist.NumObjects(); i > 0; i--)
	{
		UIListItem* item = m_itemlist.ObjectAt(i);
		m_itemlist.RemoveObjectAt(i);
		delete item;
	}

	if (m_prev_arrow_region) delete m_prev_arrow_region;
	if (m_next_arrow_region) delete m_next_arrow_region;
}

void UIList::Draw
	(
	void
	)

{
	// FIXME: stub
}

qboolean UIList::KeyEvent
	(
	int key,
	unsigned int time
	)

{
	switch (key)
	{
	case K_RIGHTARROW:
		ScrollNext();
		UpdateData();
		if (m_commandhandler) {
			m_commandhandler(m_itemlist.ObjectAt(m_currentItem)->itemname.c_str(), NULL);
		}
		return qtrue;
	case K_LEFTARROW:
		ScrollPrev();
		UpdateData();
		if (m_commandhandler) {
			m_commandhandler(m_itemlist.ObjectAt(m_currentItem)->itemname.c_str(), NULL);
		}
		return qtrue;
	default:
		return qfalse;
	}

}

void UIList::CharEvent
	(
	int ch
	)

{
}

void UIList::Pressed
	(
	Event *ev
	)

{
	// FIXME: stub
}

void UIList::Released
	(
	Event *ev
	)

{
	// FIXME: stub
}

void UIList::ScrollNext
	(
	void
	)

{
	m_currentItem++;
	if (m_currentItem > m_itemlist.NumObjects()) {
		m_currentItem = m_itemlist.NumObjects();
	}
}

void UIList::ScrollPrev
	(
	void
	)

{
	m_currentItem--;
	if (m_currentItem < 1) {
		m_currentItem = 1;
	}
}

void UIList::FrameInitialized
	(
	void
	)

{
	m_prev_arrow_region = new UIRect2D(m_clippedframe.pos.x, m_clippedframe.pos.y, m_arrow_width, m_frame.size.height);
	m_next_arrow_region = new UIRect2D(m_clippedframe.size.width - m_arrow_width + m_clippedframe.pos.x, m_clippedframe.pos.y, m_arrow_width, m_frame.size.height);
}

void UIList::LayoutAddListItem
	(
	Event *ev
	)

{
	// FIXME: stub
}

void UIList::AddItem
(
str item,
str alias
)

{
	// FIXME: stub
}

void UIList::UpdateUIElement
	(
	void
	)

{
	// FIXME: stub
}

void UIList::UpdateData
	(
	void
	)

{
	// FIXME: stub
}

CLASS_DECLARATION( UIList, UIListIndex, NULL )
{
	{ NULL, NULL }
};

qboolean UIListIndex::KeyEvent
	(
	int key,
	unsigned int time
	)

{
	switch (key)
	{
	case K_RIGHTARROW:
		ScrollNext();
		UpdateData();
		return qtrue;
	case K_LEFTARROW:
		ScrollPrev();
		UpdateData();
		return qtrue;
	default:
		return qfalse;
	}
}
