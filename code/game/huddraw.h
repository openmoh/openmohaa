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

// huddraw.cpp :	This is a HudDraw Functions Code section with huddraw functions reversed for 
//					altering clients HUD

#ifndef __HUDDRAW_H__
#define __HUDDRAW_H__

#include "g_local.h"

void HudDraw3d( int index, vec3_t vector, int ent_num, qboolean bAlwaysShow, qboolean depth );
void HudDrawShader( int info, const char *name );
void HudDrawAlign( int info, int horizontalAlign, int verticalAlign );
void HudDrawRect( int info, int x, int y, int width, int height );
void HudDrawVirtualSize( int info, int virtualScreen );
void HudDrawColor( int info, float *color );
void HudDrawAlpha( int info, float alpha );
void HudDrawString( int info, const char *string );
void HudDrawFont( int info, const char *fontName );
void HudDrawTimer( int index, float duration, float fade_out_time );

void iHudDraw3d( int cl_num, int index, vec3_t vector, int ent_num, qboolean bAlwaysShow, qboolean depth );
void iHudDrawShader( int cl_num, int info, const char *name );
void iHudDrawAlign( int cl_num, int info, int horizontalAlign, int verticalAlign );
void iHudDrawRect( int cl_num, int info, int x, int y, int width, int height );
void iHudDrawVirtualSize( int cl_num, int info, int virtualScreen );
void iHudDrawColor( int cl_num, int info, float *color );
void iHudDrawAlpha( int cl_num, int info, float alpha );
void iHudDrawString( int cl_num, int info, const char *string );
void iHudDrawFont( int cl_num, int info, const char *fontName );
void iHudDrawTimer( int cl_num, int index, float duration, float fade_out_time );

#endif // __HUDDRAW_H__
