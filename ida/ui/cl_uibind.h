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

typedef struct bind_item_s {
	str name;
	str command;
	UIReggedMaterial *nameMaterial;
} bind_item_t;

typedef struct bind_s {
	int width;
	int height;
	int fillwidth;
	int commandwidth;
	int commandheight;
	int primarykeywidth;
	int primarykeyheight;
	int alternatekeywidth;
	int alternatekeyheight;
	int align;
	UColor titlebgcolor;
	UColor titlefgcolor;
	UColor activefgcolor;
	UColor activebgcolor;
	UColor activebordercolor;
	UColor inactivefgcolor;
	UColor inactivebgcolor;
	UColor highlightfgcolor;
	UColor highlightbgcolor;
	UColor selectfgcolor;
	UColor selectbgcolor;
	str changesound;
	str activesound;
	str entersound;
	str headers[ 3 ];
	UIReggedMaterial *headermats[ 3 ];
	UIReggedMaterial *fillmaterial;
	Container binds;
} bind_t;

typedef struct bindlistener_s {
	Listener2 baseClass;
	bind_t *bind;
} bindlistener;
