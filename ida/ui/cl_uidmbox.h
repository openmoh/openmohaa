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

typedef enum { box_moving_out, box_moving_in, box_out, box_in } boxstate_t;

typedef struct UIDMBox_s {
	UIWidget baseClass;
	item_t m_items[ 10 ];
	int m_numitems;
	bool m_reallyshown;
	UIFont *m_fontbold;
	bool m_boxmovingout;
	boxstate_t m_boxstate;
	int m_boxtime;
	int m_movespeed;
	int m_iBeginDecay;
	int m_iEndDecay;
} UIDMBox;
