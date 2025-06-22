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
 * @brief Extension for recast-based navigation
 * 
 */

#pragma once

#include "../qcommon/class.h"
#include "../qcommon/vector.h"
#include "../qcommon/container.h"

struct rcPolyMesh;

/**
 * @brief An offmesh point that the navigation system will use to find path.
 *
 */
struct offMeshNavigationPoint {
    Vector         start;
    Vector         end;
    float          radius;
    unsigned short flags;
    unsigned char  area;
    bool           bidirectional;
    int            id;

    offMeshNavigationPoint()
        : radius(0)
        , flags(0)
        , area(0)
        , bidirectional(true)
        , id(0)
    {}

    bool operator==(const offMeshNavigationPoint& other) const { return start == other.start && end == other.end; }

    bool operator!=(const offMeshNavigationPoint& other) const { return !(*this == other); }
};

struct ExtensionArea {
    float         cost;
    unsigned char number;

    ExtensionArea(unsigned char number, float cost)
    {
        this->number = number;
        this->cost   = cost;
    }
};

class INavigationMapExtension : public Class
{
private:
    CLASS_PROTOTYPE(INavigationMapExtension);

public:
    virtual void Handle(Container<offMeshNavigationPoint>& points, const rcPolyMesh *polyMesh) {}

    virtual Container<ExtensionArea> GetSupportedAreas() const { return Container<ExtensionArea>(); }
};

class NavigationMapExtension_Ladders : public INavigationMapExtension
{
private:
    CLASS_PROTOTYPE(NavigationMapExtension_Ladders);

public:
    void                     Handle(Container<offMeshNavigationPoint>& points, const rcPolyMesh *polyMesh) override;
    Container<ExtensionArea> GetSupportedAreas() const override;
};

class NavigationMapExtension_JumpFall : public INavigationMapExtension
{
private:
    CLASS_PROTOTYPE(NavigationMapExtension_JumpFall);

public:
    void                     Handle(Container<offMeshNavigationPoint>& points, const rcPolyMesh *polyMesh) override;
    Container<ExtensionArea> GetSupportedAreas() const override;

private:
    void                   FixupPoint(vec3_t pos);
    bool                   AddPoint(Container<offMeshNavigationPoint>& points, const offMeshNavigationPoint& point);
    bool                   AreVertsValid(const vec3_t pos1, const vec3_t pos2) const;
    offMeshNavigationPoint CanConnectFallPoint(const rcPolyMesh *polyMesh, const Vector& pos1, const Vector& pos2);
    offMeshNavigationPoint CanConnectJumpPoint(const rcPolyMesh *polyMesh, const Vector& pos1, const Vector& pos2);
    offMeshNavigationPoint CanConnectJumpOverLedgePoint(const rcPolyMesh *polyMesh, const Vector& pos1, const Vector& pos2);
    offMeshNavigationPoint CanConnectStraightPoint(const rcPolyMesh *polyMesh, const Vector& pos1, const Vector& pos2);
};
