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

typedef enum {
	L_STATBAR_NONE,
	L_STATBAR_HORIZONTAL,
	L_STATBAR_VERTICAL,
	L_STATBAR_VERTICAL_REVERSE,
	L_STATBAR_VERTICAL_STAGGER_EVEN,
	L_STATBAR_VERTICAL_STAGGER_ODD,
	L_STATBAR_CIRCULAR,
	L_STATBAR_NEEDLE,
	L_STATBAR_ROTATOR,
	L_STATBAR_COMPASS,
	L_STATBAR_SPINNER,
	L_STATBAR_HEADING_SPINNER
} statbar_orientation_t;

typedef struct UIFakkLabel_s {
	UILabel baseClass;
	SafePtr2_t m_lastitem;
	int m_lastitemindex;
	int m_stat;
	int m_stat_configstring;
	int m_maxstat;
	int m_itemindex;
	int m_inventoryrendermodelindex;
	str m_sDrawModelName;
	qboolean m_rendermodel;
	statbar_orientation_t m_statbar_or;
	float m_statbar_min;
	float m_statbar_max;
	float m_lastfrac;
	float m_flashtime;
	vec3_t m_offset;
	vec3_t m_rotateoffset;
	vec3_t m_angles;
	float m_scale;
	str m_anim;
	UIReggedMaterial *m_statbar_material;
	UIReggedMaterial *m_statbar_material_flash;
} UIFakkLabel;
