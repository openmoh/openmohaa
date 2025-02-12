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

// Legacy path system

#include "navigation_legacy_path.h"

void LegacyPather::FindPath(const Vector& start, const Vector& end, const PathSearchParameter& parameters) {}

void LegacyPather::FindPathNear(
    const Vector& start, const Vector& end, float radius, const PathSearchParameter& parameters
)
{}

void LegacyPather::FindPathAway(
    const Vector&              start,
    const Vector&              avoid,
    const Vector&              preferredDir,
    float                      radius,
    const PathSearchParameter& parameters
)
{}

void LegacyPather::UpdatePos(const Vector& origin) {}

void LegacyPather::Clear() {}

PathNav LegacyPather::GetNode(int index) const
{
    return {};
}

int LegacyPather::GetNodeCount() const
{
    return 0;
}
