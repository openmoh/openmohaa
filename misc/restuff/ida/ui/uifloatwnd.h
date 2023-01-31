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

typedef struct UIChildSpaceWidget_s {
	UIWidget baseClass;
} UIChildSpaceWidget;

typedef struct UIFloatingWindow_s {
	UIWidget baseClass;
	UIPoint2D m_clickOffset;
	bool m_isPressed;
	UColor m_titleColor;
	UColor m_textColor;
	UIChildSpaceWidget *m_childspace;
	bool m_minimized;
	float m_restoredHeight;
	UIPoint2D m_clickpoint;
	int m_clicktime;
	UIButton *m_closeButton;
	UIButton *m_minimizeButton;
} UIFloatingWindow;
