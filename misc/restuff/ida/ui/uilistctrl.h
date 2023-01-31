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

typedef enum { TYPE_STRING, TYPE_OWNERDRAW } griditemtype_t;

typedef struct UIListCtrlItem_s {
	void *_vptr;
} UIListCtrlItem;

typedef struct m_clickState_s {
	int time;
	int selected;
	UIPoint2D point;
} m_clickState_t;

typedef struct UIListCtrl_s {
	UIListBase baseClass;
	struct columndef_t {
		str title;
		int name;
		int width;
		bool numeric;
		bool reverse_sort;
	};

	int m_iLastSortColumn;
	UIFont *m_headerfont;
	Container m_itemlist;
	Container m_columnlist;

	qboolean m_bDrawHeader;
	struct {
		int column;
		int min;
	} m_sizestate;
	m_clickState_s m_clickState;

	int( *m_comparefunction ) ();
} UIListCtrl;
