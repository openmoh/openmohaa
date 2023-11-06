/*
===========================================================================
Copyright (C) 2023 the OpenMoHAA team

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

// tiki_mesh.cpp : Mesh handling

#include "tiki.h"

int             TIKI_Skel_Bones_Index;
int             skeletorMorphCacheIndex;
skelBoneCache_t TIKI_Skel_Bones[MAX_SKELBONES];
int             skeletorMorphCache[MAX_SKELMORPH];

void TIKI_Reset_Caches()
{
    TIKI_Skel_Bones_Index   = 0;
    skeletorMorphCacheIndex = 0;
}
