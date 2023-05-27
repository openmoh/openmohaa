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
int R_MarkFragments_New(int numPoints, const vec3_t* points, const vec3_t projection,
    int maxPoints, vec3_t pointBuffer, int maxFragments, markFragment_t* fragmentBuffer, float fRadiusSquared) {
    return R_MarkFragments(numPoints, points, projection, maxPoints, pointBuffer, maxFragments, fragmentBuffer);
}

int R_MarkFragmentsForInlineModel(clipHandle_t bmodel, const vec3_t angles, const vec3_t origin, int numPoints,
    const vec3_t* points, const vec3_t projection, int maxPoints, vec3_t pointBuffer,
    int maxFragments, markFragment_t* fragmentBuffer, float radiusSquared)
{
    // FIXME: unimplemented
    return 0;
}