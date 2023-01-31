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

typedef enum { FONT_JUSTHORZ_CENTER, FONT_JUSTHORZ_LEFT, FONT_JUSTHORZ_RIGHT } fonthorzjustify_t;
typedef enum { FONT_JUSTVERT_TOP, FONT_JUSTVERT_CENTER, FONT_JUSTVERT_BOTTOM } fontvertjustify_t;

typedef struct UIFont_s {
	unsigned int m_listbase;
	UColor color;
	fontheader_t *m_font;
} UIFont;
