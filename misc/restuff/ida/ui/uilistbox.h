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

typedef struct UIListBase_s {
	UIWidget baseClass;
	int m_currentItem;
	UIVertScroll *m_vertscroll;
	qboolean m_bUseVertScroll;
} UIListBase;

typedef struct ListItem_s {
	Class_t baseClass;
	str string;
	str command;
	int index;
} ListItem;

typedef struct UIListBox_s {
	UIListBase baseClass;
	Container m_itemlist;
	struct {
		int time;
		int selected;
		UIPoint2D point;
	} m_clickState;
} UIListBox;
