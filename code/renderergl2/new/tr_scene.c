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

#include "../tr_local.h"

extern int			r_firstSceneDrawSurf;

extern int			r_numdlights;
extern int			r_firstSceneDlight;

extern int			r_numentities;
extern int			r_firstSceneEntity;

extern int			r_numpolys;
extern int			r_firstScenePoly;

extern int			r_numpolyverts;

void RE_AddRefSpriteToScene(const refEntity_t* ent) {
    // FIXME: unimplemented
}

/*
=====================
R_AddTerrainMarkSurfaces
=====================
*/
void R_AddTerrainMarkSurfaces(void) {
    // FIXME: unimplemented
}

/*
=====================
RE_AddTerrainMarkToScene
=====================
*/
void RE_AddTerrainMarkToScene(int iTerrainIndex, qhandle_t hShader, int numVerts, const polyVert_t* verts, int renderfx) {
    // FIXME: unimplemented
}

//=================================================================================

/*
=====================
RE_GetRenderEntity
=====================
*/
refEntity_t* RE_GetRenderEntity(int entityNumber) {
    int i;

    for (i = 0; i < r_numentities; i++) {
        if (backEndData->entities[i].e.entityNumber == entityNumber) {
            return &backEndData->entities[i].e;
        }
    }

    return NULL;
}
