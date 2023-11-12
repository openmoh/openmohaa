/*
===========================================================================
Copyright (C) 2023 the OpenMoHAA team

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
	UpdateUIElement();

	// draw the item text
	if (m_currentItem)
	{
		str itemText;
		UIListItem *item = m_itemlist.ObjectAt(m_currentItem);
		if (item->itemalias)
		{
			itemText = item->itemalias;
		}
		else if (item->itemname)
		{
			itemText = item->itemname;
		}

		float textX = 0.5f * m_frame.size.width - 0.5f * m_font->getWidth(itemText, -1);
		float textY = 0.5f * (m_frame.size.height - m_font->getHeight(m_bVirtual)) - 1.0f;
		m_font->setColor(m_foreground_color);
		const char *text = Sys_LV_CL_ConvertString(itemText);
		m_font->Print(textX, textY, text, -1, m_bVirtual);
	}

	// draw the previous arrow
	Draw3DBox(
		0.0f,
		0.0f,
		m_arrow_width,
		m_frame.size.height,
		m_prev_arrow_depressed,
		m_border_color,
		m_local_alpha);

	m_prev_arrow->ReregisterMaterial();

	float height = m_frame.size.height - 4.0;
	float width = m_arrow_width - 4.0;
	uii.Rend_DrawPicStretched(
		2.0f,
		2.0f,
		width,
		height,
		0.0f,
		0.0f,
		1.0f,
		1.0f,
		m_prev_arrow->GetMaterial());

	// draw the next arrow
	float x = m_frame.size.width - m_arrow_width;
	float y = 0.0f;
	Draw3DBox(
		x,
		y,
		m_arrow_width,
		m_frame.size.height,
		m_next_arrow_depressed,
		m_border_color,
		m_local_alpha);

	m_next_arrow->ReregisterMaterial();

	uii.Rend_DrawPicStretched(
		x + 2.0f,
		y + 2.0f,
		width,
		height,
		0.0f,
		0.0f,
		1.0f,
		1.0f,
		m_next_arrow->GetMaterial());
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
	m_held = qtrue;
	m_depressed = qtrue;

	float mouse_xpos = ev->GetFloat(1);
	float mouse_ypos = ev->GetFloat(2);
	int buttons = ev->GetInteger(3); // unused

	FrameInitialized();

	// previous arrow is clicked
	if (m_prev_arrow_region->contains(mouse_xpos, mouse_ypos))
	{
		m_prev_arrow_depressed = qtrue;
		ScrollPrev();
		UpdateData();
		return;
	}

	// next arrow is clicked
	if (m_next_arrow_region->contains(mouse_xpos, mouse_ypos))
	{
		m_next_arrow_depressed = qtrue;
		ScrollNext();
		UpdateData();
	}
}

void UIList::Released
	(
	Event *ev
	)

{
	m_held = qfalse;
	m_depressed = qfalse;
	m_prev_arrow_depressed = qfalse;
	m_next_arrow_depressed = qfalse;
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
	str item = ev->GetString(1);
	str alias;
	if (ev->NumArgs() > 1)
	{
		alias = ev->GetString(2);
	}

	AddItem(item, alias);
}

void UIList::AddItem
(
str item,
str alias
)

{
	UIListItem *listItem = new UIListItem();
	listItem->itemname = item;
	listItem->itemalias = alias;

	m_itemlist.AddObject(listItem);
	m_currentItem = 1;
}

void UIList::UpdateUIElement
	(
	void
	)

{
	if (!m_cvarname)
	{
		return;
	}

	const char *cvarstring = UI_GetCvarString(m_cvarname, NULL);
	if (!cvarstring)
	{
		m_currentItem = 1;
		UpdateData();
		return;
	}

	for (int i = 1; i <= m_itemlist.NumObjects(); i++)
	{
		UIListItem *obj = m_itemlist.ObjectAt(i);
		if (obj->itemname == cvarstring)
		{
			m_currentItem = i;
			break;
		}
	}
}

void UIList::UpdateData
	(
	void
	)

{
	if (!m_currentItem)
	{
		return;
	}

	if (m_cvarname)
	{
		UIListItem *item = m_itemlist.ObjectAt(m_currentItem);
		if (item->itemname)
		{
			uii.Cvar_Set(m_cvarname, item->itemname);
		}
	}

	if (m_command)
	{
		UIListItem *item = m_itemlist.ObjectAt(m_currentItem);
		m_command.append(' ');
		m_command.append(item->itemname);
		m_command.append('\n');
		Cbuf_AddText(m_command.c_str());
	}
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
