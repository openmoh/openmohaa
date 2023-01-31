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

typedef struct {
	str itemname;
	str itemalias;
} UIListItem;

typedef struct UIList_s {
	UIWidget baseClass;
	Container m_itemlist;
	int m_currentItem;
	float m_arrow_width;
	UIRect2D *m_next_arrow_region;
	UIRect2D *m_prev_arrow_region;
	char m_depressed;
	char m_held;
	short pad1;
	UIReggedMaterial *m_prev_arrow;
	UIReggedMaterial *m_next_arrow;
	char m_prev_arrow_depressed;
	char m_next_arrow_depressed;
	short pad2;
} UIList;

typedef struct UIListIndex_s {
	UIList baseClass;
} UIListIndex;
