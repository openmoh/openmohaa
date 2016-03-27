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

typedef struct UColor_s {
	float r;
	float g;
	float b;
	float a;
} UColor;

typedef struct UColorHSV_s {
	float h;
	float s;
	float v;
	float a;
} UColorHSV;

typedef enum { DARK, REALLYDARK, LIGHT, NORMAL } colorType_t;

typedef struct UBorderColor_s {
	UColor dark;
	UColor reallydark;
	UColor light;
	UColor original;
} UBorderColor;
