/*
===========================================================================
Copyright (C) 2025 the OpenMoHAA team

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

#include "navigation_recast_helpers.h"
#include "Recast.h"

void ConvertGameToRecastCoord(const float *in, float *out)
{
#if 0
    vec3_t mat[3];
    vec3_t angles = { 0, 0, -90 };
    AnglesToAxis(angles, mat);

    MatrixTransformVector(in, mat, out);
#endif

    // Same as above
    out[0] = in[0];
    out[1] = in[2];
    out[2] = -in[1];
}

void ConvertGameToRecastExtents(const float *in, float *out)
{
    out[0] = in[0];
    out[1] = in[2];
    out[2] = in[1];
}

void ConvertRecastToGameCoord(const float *in, float *out)
{
#if 0
    vec3_t mat[3];
    vec3_t angles = { 0, 0, 90 };
    AnglesToAxis(angles, mat);

    MatrixTransformVector(in, mat, out);
#endif

    out[0] = in[0];
    out[1] = -in[2];
    out[2] = in[1];
}

void ConvertRecastToGameExtents(const float *in, float *out)
{
    out[0] = in[0];
    out[1] = in[2];
    out[2] = in[1];
}

void GetPolyMeshVertPosition(const rcPolyMesh *polyMesh, int vertNum, vec3_t out)
{
    const unsigned short *v;
    vec3_t tmp;

    v = &polyMesh->verts[vertNum * 3];
    tmp[0] = polyMesh->bmin[0] + v[0] * polyMesh->cs;
    tmp[1] = polyMesh->bmin[1] + (v[1] + 1) * polyMesh->ch;
    tmp[2] = polyMesh->bmin[2] + v[2] * polyMesh->cs;
    ConvertRecastToGameCoord(tmp, out);
}
