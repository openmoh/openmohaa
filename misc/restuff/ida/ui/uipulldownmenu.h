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

typedef struct uipull_describe_s {
	str title;
	UColor highlightFGColor;
	UColor highlightBGColor;
	UColor FGColor;
	UColor BGColor;
	UIReggedMaterial *material;
	UIReggedMaterial *selected_material;
	Container desc;
} uipull_describe;

typedef struct UIPulldownMenu_s {
	UIWidget baseClass;
	Container m_desc;
	Listener_t *m_listener;
	int m_submenu;
	UIPopupMenu *m_submenuptr;
	UColor m_highlightBGColor;
	UColor m_highlightFGColor;
} UIPulldownMenu;
