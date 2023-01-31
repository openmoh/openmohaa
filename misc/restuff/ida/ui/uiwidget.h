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

typedef struct item_s {
	
} item_t;

typedef struct UIReggedMaterial_s {
	uihandle_t hMat;
	str name;
	char isSet;
	char isGot;
} UIReggedMaterial;

typedef struct UIReggedMaterial_s UIReggedMaterial;
typedef enum { border_none, border_3D, border_indent, border_outline } borderstyle_t;
typedef enum { D_NONE, D_FROM_LEFT, D_FROM_RIGHT, D_FROM_BOTTOM, D_FROM_TOP } direction_t;
typedef enum { MOTION_IN, MOTION_OUT } motion_t;

typedef struct UIWidget_s {
	USignal baseClass;
	UIRect2D m_frame;
	float m_vVirtualScale[ 2 ];
	UIPoint2D m_startingpos;
	UIPoint2D m_origin;
	UIRect2D m_screenframe;
	UIPoint2D m_screenorigin;
	UIRect2D m_clippedframe;
	UIPoint2D m_clippedorigin;
	UColor m_background_color;
	UColor m_foreground_color;
	UBorderColor m_border_color;
	borderstyle_t m_borderStyle;
	char m_visible;
	char m_enabled;
	short pad1;
	UIFont *m_font;
	struct UIWidget_s *m_parent;
	Container_t m_children;
	str m_name;
	int m_canactivate;
	str m_cvarname;
	str m_cvarvalue;
	str m_command;
	Container_t m_showcommands;
	Container_t m_hidecommands;
	str m_title;
	float m_indent;
	float m_bottomindent;
	UIReggedMaterial *m_material;
	UIReggedMaterial *m_hovermaterial;
	UIReggedMaterial *m_pressedmaterial;
	char m_hovermaterial_active;
	char m_pressedmaterial_active;
	short pad2;
	direction_t m_direction;
	direction_t m_direction_orig;
	motion_t m_motiontype;
	float m_starttime;
	float m_fadetime;
	float m_alpha;
	float m_local_alpha;
	float m_motiontime;
	str m_stopsound;
	str m_clicksound;
	int m_align;
	int m_flags;
	int m_dying;
	int m_ordernum;
	int m_configstring_index;
	fonthorzjustify_t m_iFontAlignmentHorizontal;
	fontvertjustify_t m_iFontAlignmentVertical;
	qboolean m_bVirtual;
	str m_enabledCvar;
	void *m_commandhandler;
} UIWidget;

typedef struct UILayout_s UILayout;

typedef struct UIWidgetContainer_s {
	UIWidget baseClass;
	UColor m_bgfill;
	qboolean m_fullscreen;
	int m_vidmode;
	int m_currentwidnum;
	int m_maxordernum;
	UILayout *m_layout;
} UIWidgetContainer;
