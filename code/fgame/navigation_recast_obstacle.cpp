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

#include "g_local.h"
#include "navigation_recast_obstacle.h"
#include "navigation_recast_load.h"
#include "navigation_recast_config.h"
#include "navigation_recast_helpers.h"
#include "entity.h"
#include "trigger.h"

#include "DetourNavMeshQuery.h"

NavigationObstacleMap navigationObstacleMap;

/**
 * @brief Iterate through polys and set flags
 * 
 */
class NavigationObstacleQuery : public dtPolyQuery
{
private:
    const dtNavMesh         *navMesh;
    NavigationObstacleTiles& tiles;
    bool                     isOccupied;

public:
    float maxRadiusSqr;

public:
    NavigationObstacleQuery(const dtNavMesh *inNavMesh, NavigationObstacleTiles& inTiles, bool inIsOccupied)
        : navMesh(inNavMesh)
        , tiles(inTiles)
        , isOccupied(inIsOccupied)
    {}

    float getPolySize(const dtMeshTile *tile, const dtPoly *poly) const
    {
        Vector rcBounds[2];
        Vector size;
        int    i;

        //
        // Calculate the polys bounds
        //

        ClearBounds(rcBounds[0], rcBounds[1]);

        for (i = 0; i < poly->vertCount; i++) {
            const float *pv = &tile->verts[poly->verts[i] * 3];

            AddPointToBounds(pv, rcBounds[0], rcBounds[1]);
        }

        size = rcBounds[1] - rcBounds[0];

        return size.lengthSquared();
    }

    void process(const dtMeshTile *tile, dtPoly **polys, dtPolyRef *refs, int count) override
    {
        const int tileNum = tile - navMesh->getTile(0);
        int       i, j;

        if (isOccupied) {
            for (i = 0; i < count; i++) {
                const int   polyNum = polys[i] - tile->polys;
                const float radius  = getPolySize(tile, polys[i]);

                if (radius > maxRadiusSqr) {
                    continue;
                }

                // Mark as busy as long as the poly size is below the allowed radius
                polys[i]->flags |= RECAST_POLYFLAG_BUSY;

                tiles.polysRef[tileNum][polyNum]++;
            }

            tiles.tilesRef[tileNum]++;
        } else {
            for (i = 0; i < count; i++) {
                const int   polyNum = polys[i] - tile->polys;
                const float radius  = getPolySize(tile, polys[i]);

                if (radius > maxRadiusSqr) {
                    continue;
                }

                // Last poly, release it
                if (tiles.polysRef[tileNum][polyNum] == 1) {
                    polys[i]->flags &= ~RECAST_POLYFLAG_BUSY;
                }

                assert(tiles.polysRef[tileNum][polyNum] > 0);
                if (tiles.polysRef[tileNum][polyNum] > 0) {
                    tiles.polysRef[tileNum][polyNum]--;
                }
            }

            assert(tiles.tilesRef[tileNum] > 0);
            if (tiles.tilesRef[tileNum] > 0) {
                tiles.tilesRef[tileNum]--;
            }
        }
    }
};

const Vector& NavigationObstacleEntities::GetMin(unsigned int entnum) const
{
    return bounds[0][entnum];
}

const Vector& NavigationObstacleEntities::GetMax(unsigned int entnum) const
{
    return bounds[1][entnum];
}

void NavigationObstacleEntities::SetBounds(unsigned int entnum, const Vector& min, const Vector& max)
{
    bounds[0][entnum] = min;
    bounds[1][entnum] = max;
}

bool NavigationObstacleEntities::IsActive(unsigned int entnum) const
{
    return flag[entnum] & NAVOBS_FLAG_ACTIVE;
}

void NavigationObstacleEntities::Add(unsigned int entnum)
{
    flag[entnum] = NAVOBS_FLAG_ACTIVE;
}

void NavigationObstacleEntities::Set(unsigned int entnum, const Vector& position)
{
    flag[entnum] = NAVOBS_FLAG_ACTIVE;
}

void NavigationObstacleEntities::Remove(unsigned int entnum)
{
    bounds[0][entnum] = vec_zero;
    bounds[1][entnum] = vec_zero;
    flag[entnum]      = 0;
}

NavigationObstacleTiles::NavigationObstacleTiles()
{
    tilesRef = NULL;
    polysRef = NULL;
}

NavigationObstacleTiles::~NavigationObstacleTiles()
{
    Clear();
}

void NavigationObstacleTiles::Init()
{
    const dtNavMesh *navMesh = navigationMap.GetNavMesh();
    int              numTotalPolys;
    int              i;
    int              totalSize;
    byte            *start;
    byte            *buffer;

    Clear();

    numTiles      = navMesh->getMaxTiles();
    numTotalPolys = 0;

    //
    // Calculate the total number of polys from all tiles
    //

    for (i = 0; i < numTiles; i++) {
        const dtMeshTile *tile = navMesh->getTile(i);
        if (tile && tile->header) {
            numTotalPolys += tile->header->polyCount;
        }
    }

    //
    // Allocate one giant big chunk of memory to hold all that information
    //

    totalSize =
        sizeof(unsigned int) * numTiles + sizeof(unsigned int *) * numTiles + sizeof(unsigned int) * numTotalPolys;
    buffer = start = (byte *)gi.Malloc(totalSize);

    tilesRef = new (buffer) unsigned int[numTiles]();
    buffer += sizeof(unsigned int) * numTiles;

    polysRef = new (buffer) unsigned int *[numTiles];
    buffer += sizeof(unsigned int *) * numTiles;

    for (i = 0; i < numTiles; i++) {
        const dtMeshTile *tile = navMesh->getTile(i);
        if (tile && tile->header) {
            polysRef[i] = new (buffer) unsigned int[tile->header->polyCount]();
            buffer += sizeof(unsigned int) * tile->header->polyCount;
        }
    }

    assert(buffer - start <= totalSize);
}

void NavigationObstacleTiles::Clear()
{
    // Single block of memory
    if (tilesRef) {
        gi.Free(tilesRef);
        tilesRef = NULL;
    }
}

NavigationObstacleEntities::NavigationObstacleEntities()
{
    int i;

    for (i = 0; i < ARRAY_LEN(flag); i++) {
        flag[i] = 0;
    }
}

NavigationObstacleMap::NavigationObstacleMap()
{
    ents = NULL;
}

NavigationObstacleMap::~NavigationObstacleMap()
{
    Clear();
}

void NavigationObstacleMap::Clear()
{
    if (ents) {
        delete ents;
        ents = NULL;
    }

    tiles.Clear();
}

void NavigationObstacleMap::Init()
{
    Clear();

    ents = new NavigationObstacleEntities();
    tiles.Init();
}

void NavigationObstacleMap::Update()
{
    gentity_t *ent;
    int        i;

    if (!ents) {
        return;
    }

    for (i = 0; i < MAX_GENTITIES; i++) {
        ent = &g_entities[i];

        if (i == ENTITYNUM_WORLD) {
            continue;
        }

        if (IsValidEntity(ent)) {
            if (!ents->IsActive(i)) {
                EntityAdded(ent);
            } else if (ents->GetMin(i) != ent->r.absmin || ents->GetMax(i) != ent->r.absmax) {
                EntityChanged(ent);
            }
        } else if (ents->IsActive(i)) {
            EntityRemoved(ent);
        }
    }
}

bool NavigationObstacleMap::IsValidEntity(gentity_t *ent)
{
    if (!ent->inuse || !ent->entity) {
        return false;
    }

    if (!IsSpecialEntity(ent)) {
        if (ent->s.solid == SOLID_NOT || ent->s.solid == SOLID_TRIGGER) {
            return false;
        }

        // Same contents as player's clipmask
        if (!(ent->r.contents & MASK_PLAYERSOLID)) {
            return false;
        }
    }

    // Ignore other sentients
    if (ent->entity->IsSubclassOfSentient()) {
        return false;
    }

    // Ignore doors as they can be interacted
    if (ent->entity->IsSubclassOfDoor()) {
        return false;
    }

    return true;
}

bool NavigationObstacleMap::IsSpecialEntity(gentity_t *ent)
{
    const Trigger *trig;

    //
    // FIXME: Use an alternative solution other than hardcoding
    //

    if (!ent->entity->isSubclassOf(Trigger)) {
        return false;
    }

    trig = static_cast<Trigger*>(ent->entity);

    if (str::icmp(ent->entity->targetname, "minefield")) {
        return false;
    }

    return true;
}

void NavigationObstacleMap::EntityAdded(gentity_t *ent)
{
    int entnum = ent - g_entities;

    ents->Add(entnum);
    ents->SetBounds(entnum, ent->r.absmin, ent->r.absmax);

    // Mark the poly as occupied
    EngagePolysAt(ent->r.absmin, ent->r.absmax);
}

void NavigationObstacleMap::EntityRemoved(gentity_t *ent)
{
    int entnum = ent - g_entities;

    // Release the poly
    ReleasePolysAt(ents->GetMin(entnum), ents->GetMax(entnum));

    ents->Remove(entnum);
}

void NavigationObstacleMap::EntityChanged(gentity_t *ent)
{
    int entnum = ent - g_entities;

    // Release polys at the old location
    ReleasePolysAt(ents->GetMin(entnum), ents->GetMax(entnum));

    // Mark polys as occupied at the new location
    EngagePolysAt(ent->r.absmin, ent->r.absmax);
    ents->SetBounds(entnum, ent->r.absmin, ent->r.absmax);
}

void NavigationObstacleMap::EngagePolysAt(const Vector& min, const Vector& max)
{
    dtNavMeshQuery         *navMeshQuery = navigationMap.GetNavMeshQuery();
    NavigationObstacleQuery query(navigationMap.GetNavMesh(), tiles, true);

    Vector center;
    Vector halfExtents;
    Vector size;
    float  radiusSqr;
    center      = (max + min) * 0.5;
    size        = max - min;
    halfExtents = size * 0.5;

    radiusSqr = halfExtents.lengthXYSquared();
    // Minimum size of 100 units (sphere)
    // So objects like barrels that the bot can get around are ignored.
    if (radiusSqr < Square(100)) {
        return;
    }

    // Allow the object to cover polygons up to twice its size
    query.maxRadiusSqr = radiusSqr * Square(1.5);

    float rcCenter[3];
    float rcHalfExtents[3];
    ConvertGameToRecastCoord(center, rcCenter);
    ConvertGameToRecastExtents(halfExtents, rcHalfExtents);

    dtQueryFilter filter;
    navMeshQuery->queryPolygons(rcCenter, rcHalfExtents, &filter, &query);
}

void NavigationObstacleMap::ReleasePolysAt(const Vector& min, const Vector& max)
{
    dtNavMeshQuery         *navMeshQuery = navigationMap.GetNavMeshQuery();
    NavigationObstacleQuery query(navigationMap.GetNavMesh(), tiles, false);

    Vector center;
    Vector halfExtents;
    Vector size;
    float  radiusSqr;
    center      = (max + min) * 0.5;
    size        = max - min;
    halfExtents = size * 0.5;

    radiusSqr = halfExtents.lengthXYSquared();
    if (radiusSqr < Square(100)) {
        return;
    }

    query.maxRadiusSqr = radiusSqr * Square(1.5);

    float rcCenter[3];
    float rcHalfExtents[3];
    ConvertGameToRecastCoord(center, rcCenter);
    ConvertGameToRecastExtents(halfExtents, rcHalfExtents);

    dtQueryFilter filter;
    navMeshQuery->queryPolygons(rcCenter, rcHalfExtents, &filter, &query);
}
