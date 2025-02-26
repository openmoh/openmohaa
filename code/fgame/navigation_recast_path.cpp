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

// navigation -- Modern navigation system using Recast and Detour

#include "navigation_recast_path.h"

void RecastPather::FindPath(const Vector& start, const Vector& end, const PathSearchParameter& parameters) {}

RecastPather::RecastPather() {}

RecastPather::~RecastPather() {}

void RecastPather::FindPathNear(
    const Vector& start, const Vector& end, float radius, const PathSearchParameter& parameters
)
{}

void RecastPather::FindPathAway(
    const Vector&              start,
    const Vector&              avoid,
    const Vector&              preferredDir,
    float                      radius,
    const PathSearchParameter& parameters
)
{}

bool RecastPather::TestPath(const Vector& start, const Vector& end, const PathSearchParameter& parameters)
{
    return false;
}

void RecastPather::UpdatePos(const Vector& origin) {}

void RecastPather::Clear() {}

PathNav RecastPather::GetNode(unsigned int index) const
{
    return {};
}

int RecastPather::GetNodeCount() const
{
    return 0;
}

Vector RecastPather::GetCurrentDelta() const
{
    return Vector();
}

bool RecastPather::HasReachedGoal(const Vector& origin) const
{
    return false;
}
