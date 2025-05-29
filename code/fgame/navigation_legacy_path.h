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

#include "navigation_path.h"
#include "actorpath.h"

/**
 * @brief Legacy navigation system
 *
 * It uses the legacy navigate algorithm from the old game,
 * which uses manually placed PathNode. It's a wrapper to ActorPath.
 */
class LegacyPather : public IPather
{
public:
    virtual void FindPath(const Vector& start, const Vector& end, const PathSearchParameter& parameters) override;
    virtual void
    FindPathNear(const Vector& start, const Vector& end, float radius, const PathSearchParameter& parameters) override;
    virtual void FindPathAway(
        const Vector&              start,
        const Vector&              avoid,
        const Vector&              preferredDir,
        float                      radius,
        const PathSearchParameter& parameters
    ) override;
    virtual bool TestPath(const Vector& start, const Vector& end, const PathSearchParameter& parameters) override;

    virtual void UpdatePos(const Vector& origin) override;
    virtual void Clear() override;

    virtual PathNav GetNode(unsigned int index) const override;
    virtual int     GetNodeCount() const override;
    virtual Vector  GetCurrentDelta() const override;
    virtual Vector  GetCurrentDirection() const override;
    virtual Vector  GetDestination() const override;
    virtual bool    HasReachedGoal(const Vector& origin) const override;
    virtual bool    IsQuerying() const override;

private:
    ActorPath path;
};
