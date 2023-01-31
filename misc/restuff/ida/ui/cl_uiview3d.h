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

typedef struct View3D_s {
	UIWidget baseClass;
	float m_printfadetime;
	float m_printalpha;
	str m_printstring;
	UIReggedMaterial *m_print_mat;
	qboolean m_locationprint;
	int m_x_coord;
	int m_y_coord;
	qboolean m_letterbox_active;
} View3D;

typedef struct ConsoleView_s {
	UIWidget baseClass;
} ConsoleView;
