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

typedef enum { UIP_NONE, UIP_EVENT, UIP_EVENT_STRING, UIP_SUBMENU, UIP_CMD, UIP_CVAR, UIP_SEPARATOR } uipopup_type;
typedef enum { UIP_WHERE_RIGHT, UIP_WHERE_DOWN } uipopup_where;

typedef struct uipopup_describe_s {
	str title;
	UIReggedMaterial *material;
	uipopup_type type;
	void *data;
} uipopup_describe;

typedef struct UIPopupMenu_s {
	UIWidget baseClass;
	Container *m_describe;
	int m_selected;
	Listener_t *m_listener;
	struct UIPopupMenu_s *m_parentMenu;
	struct UIPopupMenu_s *m_submenuptr;
	int m_submenu;
	UIFont m_marlett;
	bool m_autodismiss;
	UColor m_highlightFGColor;
	UColor m_highlightBGColor;
} UIPopupMenu;

