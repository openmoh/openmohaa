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

typedef enum { SLIDER_FLOAT, SLIDER_INTEGER } slidertype_t;

typedef struct UISlider_s {
	UIWidget baseClass;
	bool m_initialized;
	float m_minvalue;
	float m_maxvalue;
	int m_minx;
	int m_maxx;
	int m_arrow_width;
	float m_value;
	float m_oldvalue;
	float m_sliderwidth;
	UIRect2D m_sliderregion;
	UIRect2D m_prev_arrow_region;
	UIRect2D m_next_arrow_region;
	slidertype_t m_slidertype;
	float m_stepsize;
	bool m_prev_arrow_depressed;
	bool m_next_arrow_depressed;
	bool m_thumb_depressed;
	UIReggedMaterial *m_bar_material;
	UIReggedMaterial *m_prev_arrow_material;
	UIReggedMaterial *m_next_arrow_material;
	UIReggedMaterial *m_thumbmaterial;
	UIReggedMaterial *m_prev_arrow_material_pressed;
	UIReggedMaterial *m_next_arrow_material_pressed;
	UIReggedMaterial *m_thumbmaterial_pressed;
} UISlider;
