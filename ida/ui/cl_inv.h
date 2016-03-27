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

typedef enum { INV_MOVE_NONE, INV_MOVE_BOB, INV_MOVE_SPIN } inv_move_type;
typedef enum { INV_CASCADE_LEFT, INV_CASCADE_RIGHT } inv_cascade_type;
typedef enum { INV_HUDANGLES_BASE, INV_HUDANGLES_COMPASS, INV_HUDANGLES_COMPASS_NEEDLE } inv_hudangles_type;

typedef struct item_properties_s {
	Class baseClass;
	float scale;
	vec3_t angles;
	vec3_t angledeltas;
	vec3_t rotateoffset;
	vec3_t offset;
	inv_move_type move;
	str model;
	str anim;
} item_properties_t;

typedef struct inventory_item_s {
	Class baseClass;
	str name;
	str ammoname;
	int equip;
	int width;
	int height;
	int barwidth;
	int barheight;
	int baroffsetY;
	int baroffsetX;
	float modelWindowX;
	float modelWindowY;
	float modelWindowWidth;
	float modelWindowHeight;
	char selShaderOnTop;
	char checkammo;
	short pad2;
	str command;
	inv_hudangles_type anglesType;
	item_properties_t hudprops;
	item_properties_t invprops;
	UIReggedMaterial *bgshader;
	UIReggedMaterial *barshader;
	UIReggedMaterial *selshader;
} inventory_item_t;

typedef struct inventory_type_s {
	str name;
	bool bg_tile;
	UIReggedMaterial *texture;
	UIReggedMaterial *bg;
	UIReggedMaterial *hoverTexture;
	UIReggedMaterial *selTexture;
	Container items;
} inventory_type_t;

typedef struct inventory_s {
	int typewidth;
	int typeheight;
	int horizoffset;
	int vertoffset;
	int align;
	inv_cascade_type cascade;
	str selectsound;
	str rejectsound;
	str changesound;
	Container types;
} inventory_t;

typedef struct invlistener_s {
	Listener2 invlistener;
	inventory_t *inv;
	inventory_type_t *curtype;
	inventory_item_t *curitem;
	int defaultWidth;
	int defaultHeight;
	int defaultBarWidth;
	int defaultBarHeight;
	int defaultBarOffsetX;
	int defaultBarOffsetY;
} invlistener;
