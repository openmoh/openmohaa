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

/**
 * @brief Utility functions for Recast navigation
 * 
 */

#pragma once

#include "../qcommon/q_shared.h"

struct rcPolyMesh;

/**
 * @brief Convert Recast coordinates to game coordinates.
 * 
 * @param in XYZ Recast coordinates.
 * @param out XYZ Game coordinates.
 */
void ConvertRecastToGameCoord(const float *in, float *out);

/**
 * @brief Convert Recast coordinates to game coordinates.
 * 
 * @param in XYZ Recast coordinates.
 * @param out XYZ Game coordinates.
 */
void ConvertRecastToGameExtents(const float *in, float *out);

/**
 * @brief Convert game coordinates to Recast coordinates.
 * 
 * @param in XYZ Game coordinates 
 * @param out XYZ Recast coordinates.
 */
void ConvertGameToRecastCoord(const float *in, float *out);

/**
 * @brief Convert game coordinates to Recast extents.
 * 
 * @param in XYZ Game extents 
 * @param out XYZ Recast extents.
 */
void ConvertGameToRecastExtents(const float *in, float *out);

/**
 * @brief Get the game position of a vertice in the poly mesh
 * 
 * @param polyMesh The poly mesh 
 * @param vertNum The vertice number
 * @param out world coordinate of the vertice in the mesh
 */
void GetPolyMeshVertPosition(const rcPolyMesh *polyMesh, int vertNum, vec3_t out);
