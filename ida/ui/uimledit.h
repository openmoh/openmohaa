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

typedef struct selectionpoint_s {
	int line;
	int column;
} selectionpoint_t;

typedef struct selection_s {
	selectionpoint_s begin;
	selectionpoint_s end;
} selection_t;

typedef struct dragState_s {
	UIPoint2D lastPos;
} dragState_t;

typedef struct UIMultiLineEdit_s {
	UIWidget baseClass;
	selection_s m_selection;
	dragState_s m_dragState;
	UList_str_ m_lines;
	UIVertScroll *m_vertscroll;
	mouseState_t m_mouseState;
	char m_shiftForcedDown;
	char m_edit;
	char m_changed;
	char pad1;
} UIMultiLineEdit;
