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

#include "../qcommon/q_shared.h"
#include "../qcommon/vector.h"
#include "g_public.h"

static constexpr unsigned int NAVOBS_FLAG_ACTIVE = (1 << 0);

/**
 * @brief Store a list of entities and their last bounds.
 * 
 */
struct NavigationObstacleEntities {
public:
    NavigationObstacleEntities();

    const Vector& GetMin(unsigned int entnum) const;
    const Vector& GetMax(unsigned int entnum) const;
    int           GetContents(unsigned int entnum) const;
    void          SetContents(unsigned int entnum, int contents);
    solid_t       GetSolidType(unsigned int entnum) const;
    void          SetSolidType(unsigned int entnum, solid_t type);
    void          SetBounds(unsigned int entnum, const Vector& min, const Vector& max);
    bool          IsActive(unsigned int entnum) const;
    void          Add(unsigned int entnum);
    void          Set(unsigned int entnum, const Vector& position);
    void          Remove(unsigned int entnum);

public:
    int     contents[MAX_GENTITIES];
    Vector  bounds[2][MAX_GENTITIES];
    byte    flag[MAX_GENTITIES];
    solid_t solid[MAX_GENTITIES];
};

/**
 * @brief Store a list of tiles only containing a number of references,
 * used to keep track of the number of entities occupying space.
 * 
 */
struct NavigationObstacleTiles {
public:
    NavigationObstacleTiles();
    ~NavigationObstacleTiles();

    void Init();
    void Clear();

public:
    unsigned int  *tilesRef;
    unsigned int **polysRef;
    unsigned int   numTiles;
};

/**
 * @brief Manages obstacle on the map.
 * 
 */
class NavigationObstacleMap
{
public:
    NavigationObstacleMap();
    ~NavigationObstacleMap();

    void Clear();
    void Init();
    void Update();

private:
    bool IsValidEntity(gentity_t *ent) const;
    bool IsSpecialEntity(gentity_t *ent) const;
    bool HasChanged(gentity_t *ent) const;

    void EntityAdded(gentity_t *ent);
    void EntityRemoved(gentity_t *ent);
    void EntityChanged(gentity_t *ent);

    void EngagePolysAt(gentity_t *ent, const Vector& min, const Vector& max);
    void ReleasePolysAt(gentity_t *ent, const Vector& min, const Vector& max);

private:
    // This instance is quite big so only allocate it when used
    NavigationObstacleEntities *ents;
    NavigationObstacleTiles     tiles;
};

extern NavigationObstacleMap navigationObstacleMap;
