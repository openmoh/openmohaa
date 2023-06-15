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

// debuglines.h:

#ifndef __DEBUGLINES_H__
#define __DEBUGLINES_H__

#include "g_local.h"

void G_InitDebugLines( void );
void G_InitDebugStrings( void );
void G_DebugLine( Vector start, Vector end, float r, float g, float b, float alpha );
void G_LineStipple( int factor, unsigned short pattern );
void G_LineWidth( float width );
void G_Color3f( float r, float g, float b	);
void G_Color3v( Vector color );
void G_Color4f( float r, float g,	float b,	float alpha	);
void G_Color3vf( Vector color, float alpha );
void G_BeginLine( void );
void G_Vertex( Vector v );
void G_EndLine( void );
void G_DebugBBox( Vector org, Vector mins, Vector maxs, float r, float g, float b, float alpha );
void G_DrawDebugNumber( Vector org, float number, float scale, float r, float g, float b, int precision = 0 );
void G_DebugCircle( float *org, float radius, float r, float g, float b, float alpha, qboolean horizontal = qfalse );
void G_DebugOrientedCircle( Vector org, float radius, float r, float g, float b, float alpha, Vector angles );
void G_DebugPyramid( Vector org, float radius, float r, float g, float b, float alpha );
void G_DrawCoordSystem( Vector pos, Vector f, Vector r, Vector u, int len );
void G_DebugArrow( Vector org, Vector dir, float  length, float r, float g, float b, float alpha );
void G_DrawCSystem( Vector pos, Vector forward, Vector right, Vector up, int length );
void G_AllocDebugLines( void );
void G_AllocDebugStrings( void );
void G_DeAllocDebugLines( void );
void G_DeAllocDebugStrings( void );
void G_ShiftDebugLines( void );

typedef enum
   {
   north,
   south,
   east,
   west,
   up,
   down
   } facet_t;

void G_DebugHighlightFacet( Vector org, Vector mins, Vector maxs, facet_t facet, float r, float g, float b, float alpha );

extern debugline_t *DebugLines;
extern debugstring_t *DebugStrings;

#endif /* !__DEBUGLINES_H__ */
