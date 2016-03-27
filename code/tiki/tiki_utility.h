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

// tiki_utility.h : TIKI Utility

#ifndef __TIKI_UTILITY_H__
#define __TIKI_UTILITY_H__

#ifdef __cplusplus
extern "C" {
#endif

const char *TIKI_Name( dtiki_t *pmdl );
int TIKI_NumAnims( dtiki_t *pmdl );
int TIKI_NumSurfaces( dtiki_t *pmdl );
int TIKI_NumTags( dtiki_t *pmdl );
void TIKI_CalculateBounds( dtiki_t *pmdl, float scale, vec3_t mins, vec3_t maxs );
float TIKI_GlobalRadius( dtiki_t *pmdl );
int TIKI_NumHeadModels( dtikianim_t *tiki );
void TIKI_GetHeadModel( dtikianim_t *tiki, int num, char *name );
int TIKI_NumHeadSkins( dtikianim_t *tiki );
void TIKI_GetHeadSkin( dtikianim_t *tiki, int num, char *name );

#ifdef __cplusplus
}
#endif

#endif // __TIKI_TIKI_H__
