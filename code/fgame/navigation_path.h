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

#include "../qcommon/vector.h"
#include "../qcommon/container.h"
#include "../qcommon/lightclass.h"

struct PathNav {
    Vector origin;
    vec2_t dir;
    float  dist;
};

struct PathSearchParameter {
    Vector  leashHome;
    float   leashDist;
    int     fallHeight;
    Entity *entity;
};

class IPather : public LightClass
{
public:
    virtual ~IPather() = default;

    /**
     * @brief Create a new Pather object based on supported navigation features.
     * 
     * @return IPather*
     */
    static IPather *CreatePather();

    /**
     * @brief Find and set the path from start to end
     * 
     * @param start Start of the path.
     * @param end End of the path
     * @param parameters
     */
    virtual void FindPath(const Vector& start, const Vector& end, const PathSearchParameter& parameters) = 0;

    /**
     * @brief Find and set the path from start to end with the specified goal radius
     * 
     * @param start Start of the path
     * @param end End of the path
     * @param radius The radius at which to stop the path
     * @param parameters
     */
    virtual void
    FindPathNear(const Vector& start, const Vector& end, float radius, const PathSearchParameter& parameters) = 0;

    /**
     * @brief Find and set the path away from the specified origin
     * 
     * @param start Start of the path
     * @param avoid Location to avoid
     * @param preferredDir Preferred direction to use to escape from the location to avoid
     * @param radius Minimum safe distance to avoid
     * @param parameters
     */
    virtual void FindPathAway(
        const Vector&              start,
        const Vector&              avoid,
        const Vector&              preferredDir,
        float                      radius,
        const PathSearchParameter& parameters
    ) = 0;

    /**
     * @brief Returns true if the path exists, false otherwise.
     * 
     * @param start Start of the path.
     * @param end End of the path
     * @param parameters
     * @return true if the path exists
     */
    virtual bool TestPath(const Vector& start, const Vector& end, const PathSearchParameter& parameters) = 0;

    /**
     * @brief Update path movement
     * 
     * @param origin The origin to set
     */
    virtual void UpdatePos(const Vector& origin) = 0;

    /**
     * @brief Clear the path
     * 
     */
    virtual void Clear() = 0;

    /**
     * @brief Return the node at the specified index (0 = first node, and nodecount - 1 = last node).
     * 
     * @param index
     * @return A copy of the path information. 
     */
    virtual PathNav GetNode(unsigned int index = 0) const = 0;

    /**
     * @brief Return the number of nodes.
     * 
     * @return The number of nodes. If 0, then there is no path.
     */
    virtual int GetNodeCount() const = 0;

    /**
     * @brief Return the current move delta
     * 
     * @return Vector with the current move delta.
     */
    virtual Vector GetCurrentDelta() const = 0;

    /**
     * @brief Return the directional vector towards the path
     * 
     * @return Vector with the current directional vector.
     */
    virtual Vector GetCurrentDirection() const = 0;

    /**
     * @brief Return the final destination
     * 
     * @return Vector the destination.
     */
    virtual Vector GetDestination() const = 0;

    /**
     * @brief Return true if the origin is at the end of the goal
     * 
     * @param origin The current origin to test
     */
    virtual bool HasReachedGoal(const Vector& origin) const = 0;

    /**
     * @brief Return true if the path is currently being calculated.
     * 
     * @return true 
     * @return false 
     */
    virtual bool IsQuerying() const = 0;
};
