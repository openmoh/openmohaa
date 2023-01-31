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

typedef enum { VS_NONE, VS_UP_ARROW, VS_DOWN_ARROW, VS_THUMB, VS_PAGE_DOWN, VS_PAGE_UP } whatspressed;

typedef struct UIVertScroll_s {
	UIWidget baseClass;
	int m_numitems;
	int m_pageheight;
	int m_topitem;
	UIFont m_marlett;
	whatspressed m_pressed;
	UIRect2D thumbRect;
	struct { 
		int itemOffset;
		int orgItem;
	} m_dragThumbState;
	bool m_frameinitted;
	UColor m_thumbcolor;
	UColor m_solidbordercolor;
} UIVertScroll;
