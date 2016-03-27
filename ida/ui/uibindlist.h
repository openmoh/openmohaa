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

typedef struct UIFakkBindList_s {
	UIWidget baseWidget;
	bool m_created;
	UIVertScroll *m_scroll;
	UIReggedMaterial *m_presskey_mat;
	UILabel *m_presskey_wid;
	Container_t m_widgetlist;
	Container_t m_miscwidgets;
	int m_activerow;
	int m_activeitem;
	bind_t *m_bind;
} UIFakkBindList;

typedef struct UIFakkBindListLabel_s {
	UILabel baseClass;
	UIFakkBindList *m_list;
} UIFakkBindListLabel;

