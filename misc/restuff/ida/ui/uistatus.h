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

typedef enum { M_NONE, M_DRAGGING } mouseState_t;

typedef struct UIWindowSizer_s {
	UIWidget baseClass;
	UIWidget *m_draggingwidget;
	mouseState_t m_mouseState;
	UIPoint2D m_screenDragPoint;
} UIWindowSizer;

typedef enum { WND_ALIGN_NONE, WND_ALIGN_BOTTOM } alignment_t;

typedef struct align_s {
	float dist;
	alignment_t alignment;
} align_t;

typedef struct UIStatusBar_s {
	align_t m_align;
	UIWidget *m_sizeenabled;
	bool m_created;
	UIWindowSizer *m_sizer;
} UIStatusBar;
