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

#pragma once

/**
 * @file navigation_recast_debug.h
 * @brief Debugging module for the navigation system
 *
 * ai_showroutes allow to visualize the navigation mesh.
 * There are different type of colors:
 * - red: Cannot use this route
 * - green: Walkable route
 * - cyan: Jump route
 * - violet: Fall route
 * - blue: Ladder route
 */

#include "../qcommon/vector.h"

void G_Navigation_DebugDraw();

class dtCrowd;
struct navModel_t;

class NavigationMapDebug
{
public:
    NavigationMapDebug();
    ~NavigationMapDebug();

    void DebugDraw();

private:
    void TestAgent(const Vector& start, const Vector& end, Vector *paths, int *numPaths, int maxPaths);

    void
    DrawModel(const Vector& origin, const navModel_t& model, float maxDistSquared, const Vector& offset = vec_zero);

public:
    dtCrowd *crowd;
    int      agentId;

    Vector ai_startpath;
    Vector ai_endpath;
    Vector ai_pathlist[256];
    int    ai_numPaths;
    int    ai_lastpath;
};
