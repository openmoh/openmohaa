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
 * @file navigation_legacy_path.cpp
 * @brief Legacy navigation system, use an ActorPath.
 *
 * This implementation uses the old navigation system that use pathnodes and basic path-finding algorithm.
 * Nodes have to be placed manually.
 */

#include "navigation_legacy_path.h"

void LegacyPather::FindPath(const Vector& start, const Vector& end, const PathSearchParameter& parameters)
{
    path.SetFallHeight(parameters.fallHeight);
    path.FindPath((float *)(Vector&)start, (float *)(Vector&)end, parameters.entity, 0, NULL, 0);
}

void LegacyPather::FindPathNear(
    const Vector& start, const Vector& end, float radius, const PathSearchParameter& parameters
)
{
    path.SetFallHeight(parameters.fallHeight);
    path.FindPathNear((float *)(Vector&)start, (float *)(Vector&)end, parameters.entity, 0, radius, NULL, 0);
}

void LegacyPather::FindPathAway(
    const Vector&              start,
    const Vector&              avoid,
    const Vector&              preferredDir,
    float                      radius,
    const PathSearchParameter& parameters
)
{
    path.SetFallHeight(parameters.fallHeight);
    path.FindPathAway(
        (float *)(Vector&)start,
        (float *)(Vector&)avoid,
        (float *)(Vector&)preferredDir,
        parameters.entity,
        radius,
        NULL,
        0
    );
}

bool LegacyPather::TestPath(const Vector& start, const Vector& end, const PathSearchParameter& parameters)
{
    return PathSearch::FindPath(start, end, parameters.entity, 0, NULL, 0, parameters.fallHeight) != 0;
}

void LegacyPather::UpdatePos(const Vector& origin)
{
    path.UpdatePos((float *)(Vector&)origin, 8);
}

void LegacyPather::Clear()
{
    path.Clear();
    path.ForceShortLookahead();
}

PathNav LegacyPather::GetNode(unsigned int index) const
{
    PathNav nav;

    const PathInfo *currentNode = path.CurrentNode();
    if (!currentNode) {
        // no path
        return nav;
    }

    if (currentNode < path.LastNode() + index) {
        // out of range
        return nav;
    }

    if (currentNode > path.StartNode() + index) {
        // out of range
        return nav;
    }

    const PathInfo *node = currentNode - index;

    nav.dir[0] = node->dir[0];
    nav.dir[1] = node->dir[1];
    nav.dist   = node->dist;
    nav.origin = node->point;

    return nav;
}

int LegacyPather::GetNodeCount() const
{
    if (!path.CurrentNode()) {
        return 0;
    }

    return path.CurrentNode() - path.LastNode() + 1;
}

Vector LegacyPather::GetCurrentDelta() const
{
    const float *delta = path.CurrentDelta();
    return Vector(delta[0], delta[1], 0);
}

Vector LegacyPather::GetCurrentDirection() const
{
    const float *delta = path.CurrentDelta();
    return Vector(delta[0], delta[1], 0);
}

Vector LegacyPather::GetDestination() const
{
    if (!path.CurrentNode()) {
        return {};
    }

    return path.LastNode()->point;
}

bool LegacyPather::HasReachedGoal(const Vector& origin) const
{
    return path.Complete(origin);
}

bool LegacyPather::IsQuerying() const
{
    return false;
}
