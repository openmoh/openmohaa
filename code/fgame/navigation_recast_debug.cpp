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

#include "navigation_recast_debug.h"
#include "navigation_recast_helpers.h"
#include "navigation_recast_config.h"
#include "navigation_recast_config_ext.h"
#include "navigation_recast_load.h"
#include "navigation_bsp.h"
#include "../qcommon/vector.h"
#include "navigate.h"
#include "debuglines.h"
#include "entity.h"
#include "level.h"

#include "DetourCrowd.h"
#include "DetourNavMesh.h"
#include "DetourNavMeshQuery.h"
#include "Recast.h"

NavigationMapDebug navigationMapDebug;

NavigationMapDebug::NavigationMapDebug()
{
    crowd       = NULL;
    agentId     = -1;
    ai_numPaths = 0;
    ai_lastpath = 0;
}

NavigationMapDebug::~NavigationMapDebug()
{
    if (crowd) {
        dtFreeCrowd(crowd);
    }
}

void NavigationMapDebug::TestAgent(const Vector& start, const Vector& end, Vector *paths, int *numPaths, int maxPaths)
{
    vec3_t        half = {64, 64, 64};
    vec3_t        startNav;
    vec3_t        endNav;
    dtQueryFilter filter;
    dtPolyRef     nearestStartRef, nearestEndRef;
    vec3_t        nearestStartPt, nearestEndPt;

    ConvertGameToRecastCoord(start, startNav);
    ConvertGameToRecastCoord(end, endNav);

    if (!crowd) {
        crowd = dtAllocCrowd();
        crowd->init(MAX_CLIENTS, NavigationMapConfiguration::agentRadius, navigationMap.GetNavMesh());
    }

    if (agentId != -1) {
        crowd->removeAgent(agentId);
    }

    dtCrowdAgentParams ap {0};

    ap.radius                = NavigationMapConfiguration::agentRadius;
    ap.height                = NavigationMapConfiguration::agentHeight;
    ap.maxAcceleration       = 8.0f;
    ap.maxSpeed              = 3.5f;
    ap.collisionQueryRange   = ap.radius * 12.0f;
    ap.pathOptimizationRange = ap.radius * 30.0f;
    ap.updateFlags           = 0;
    agentId                  = crowd->addAgent(startNav, &ap);

    navigationMap.GetNavMeshQuery()->findNearestPoly(startNav, half, &filter, &nearestStartRef, nearestStartPt);
    navigationMap.GetNavMeshQuery()->findNearestPoly(endNav, half, &filter, &nearestEndRef, nearestEndPt);

    crowd->requestMoveTarget(agentId, nearestEndRef, nearestEndPt);

    dtPolyRef polys[256];
    int       nPolys;
    navigationMap.GetNavMeshQuery()->findPath(
        nearestStartRef, nearestEndRef, nearestStartPt, nearestEndPt, &filter, polys, &nPolys, 256
    );
    navigationMap.GetNavMeshQuery()->findStraightPath(
        nearestStartPt, nearestEndPt, polys, nPolys, (float *)paths, NULL, NULL, numPaths, maxPaths
    );

    for (int i = 0; i < *numPaths; i++) {
        Vector converted;

        ConvertRecastToGameCoord(paths[i], converted);
        paths[i] = converted;
    }
}

void NavigationMapDebug::DrawModel(
    const Vector& origin, const navModel_t& model, float maxDistSquared, const Vector& offset
)
{
    int i, j;

    for (i = 1; i <= model.surfaces.NumObjects(); i++) {
        const navSurface_t& surface = model.surfaces.ObjectAt(i);

        for (j = 0; j < surface.indices.NumObjects(); j += 3) {
            const navVertice_t& v1 = surface.vertices[surface.indices[j + 0]];
            const navVertice_t& v2 = surface.vertices[surface.indices[j + 1]];
            const navVertice_t& v3 = surface.vertices[surface.indices[j + 2]];

            for (int k = 0; k < 3; ++k) {
                const Vector delta = (offset + surface.vertices[surface.indices[j + k]].xyz) - origin;

                if (delta.lengthSquared() >= maxDistSquared) {
                    continue;
                }

                G_DebugLine(
                    offset + surface.vertices[surface.indices[j + k]].xyz,
                    offset + surface.vertices[surface.indices[j + ((k + 1) % 3)]].xyz,
                    0,
                    1,
                    0,
                    1
                );
            }
        }
    }
}

/*
============
DebugDraw
============
*/
void NavigationMapDebug::DebugDraw()
{
    Entity               *ent            = g_entities[0].entity;
    const dtNavMesh      *navMeshDt      = navigationMap.GetNavMesh();
    const dtNavMeshQuery *navMeshQuery   = navigationMap.GetNavMeshQuery();
    const dtQueryFilter  *filter         = navigationMap.GetQueryFilter();
    const navMap_t&       navigationData = navigationMap.GetNavigationData();
    const navModel_t&     worldMap       = navigationData.GetWorldMap();

    if (!navMeshDt) {
        return;
    }

    if (!ent) {
        return;
    }

    if (ai_showallnode->integer) {
        for (int i = 0; i < navMeshDt->getMaxTiles(); i++) {
            const dtMeshTile *tile = static_cast<const dtNavMesh *>(navMeshDt)->getTile(i);
            if (!tile || !tile->header) {
                continue;
            }

            for (int j = 0; j < tile->header->vertCount; j++) {
                const float *pvert = &tile->verts[j * 3];

                Vector org;
                ConvertRecastToGameCoord(pvert, org);

                org.z += 16;

                if (org.z < ent->origin.z - 94 || org.z > ent->origin.z + 94) {
                    continue;
                }

                G_DebugBBox(org, Vector(-8, -8, -8), Vector(8, 8, 8), 1.0, 0.0, 0.5, 1.0);
            }
        }

#if 0
        for (int i = 0; i < polyMesh->nverts; ++i) {
            GetPolyMeshVertPosition(polyMesh, i, org);

            org.z += 16;

            if (org.z < ent->origin.z - 94 || org.z > ent->origin.z + 94) {
                continue;
            }

            G_DebugBBox(org, Vector(-8, -8, -8), Vector(8, 8, 8), 1.0, 0.0, 0.5, 1.0);
        }
#endif
    }

    switch (ai_showroutes->integer) {
    case 0:
    default:
        break;
    case 1:
        {
            const float maxDistSquared = Square(ai_showroutes_distance->integer);

            for (int i = 0; i < navMeshDt->getMaxTiles(); i++) {
                const dtMeshTile *tile = static_cast<const dtNavMesh *>(navMeshDt)->getTile(i);
                if (!tile || !tile->header) {
                    continue;
                }

                for (int j = 0; j < tile->header->polyCount; j++) {
                    const dtPoly *poly = &tile->polys[j];
                    const bool pass = navMeshQuery->isValidPolyRef(navMeshDt->encodePolyId(tile->salt, i, j), filter);

                    for (int k = 0; k < poly->vertCount; ++k) {
                        const float *pv1 = &tile->verts[poly->verts[k] * 3];
                        const float *pv2 = &tile->verts[poly->verts[(k + 1) % poly->vertCount] * 3];

                        Vector v1, v2;
                        ConvertRecastToGameCoord(pv1, v1);
                        ConvertRecastToGameCoord(pv2, v2);

                        const Vector delta = v1 - ent->origin;

                        if (delta.lengthSquared() >= maxDistSquared) {
                            continue;
                        }

                        if (pass) {
                            switch(poly->getArea() & 0x3f) {
                                case RC_WALKABLE_AREA:
                                default:
                                    G_DebugLine(v1, v2, 0, 1, 0, 1);
                                    break;
                                case RECAST_AREA_JUMP:
                                    G_DebugLine(v1, v2, 0, 1, 1, 1);
                                    break;
                                case RECAST_AREA_FALL:
                                case RECAST_AREA_MEDIUM_FALL:
                                case RECAST_AREA_HIGH_FALL:
                                    G_DebugLine(v1, v2, 1, 0, 1, 1);
                                    break;
                                case RECAST_AREA_LADDER:
                                    G_DebugLine(v1, v2, 0, 0, 1, 1);
                                    break;
                            }
                        } else {
                            G_DebugLine(v1, v2, 1, 0, 0, 1);
                        }
                    }
                }

                /*
                for (int j = 0; j < tile->header->offMeshConCount; j++) {
                    const dtOffMeshConnection* offMeshCon = &tile->offMeshCons[j];
                    const float* pv1 = &offMeshCon->pos[0];
                    const float* pv2 = &offMeshCon->pos[3];

                    Vector v1, v2;
                    ConvertRecastToGameCoord(pv1, v1);
                    ConvertRecastToGameCoord(pv2, v2);

                    const Vector delta = v1 - ent->origin;

                    if (delta.lengthSquared() >= maxDistSquared) {
                        continue;
                    }

                    G_DebugLine(v1, v2, 0, 1, 1, 1);
                }
                */
            }
#if 0
        for (int i = 0; i < polyMesh->npolys; ++i) {
            const unsigned short *p    = &polyMesh->polys[i * polyMesh->nvp * 2];
            const unsigned char   area = polyMesh->areas[i];

            if (area != RC_WALKABLE_AREA) {
                continue;
            }

            unsigned short vi[3];
            for (int j = 2; j < polyMesh->nvp; ++j) {
                if (p[j] == RC_MESH_NULL_IDX) {
                    break;
                }
                vi[0] = p[0];
                vi[1] = p[j - 1];
                vi[2] = p[j];

                Vector vertices[3];

                for (int k = 0; k < 3; ++k) {
                    GetPolyMeshVertPosition(polyMesh, vi[k], vertices[k]);
                }

                for (int k = 0; k < 3; ++k) {
                    const Vector delta = vertices[k] - ent->origin;

                    if (delta.lengthSquared() >= maxDistSquared) {
                        continue;
                    }

                    G_DebugLine(vertices[k], vertices[(k + 1) % 3], 0, 1, 0, 1);
                }
            }
        }
#endif
        }
        break;
    case 2:
        {
            const float maxDistSquared = Square(ai_showroutes_distance->integer);
            gentity_t  *edict;

            DrawModel(ent->origin, worldMap, maxDistSquared);

            for (edict = active_edicts.next; edict != &active_edicts; edict = edict->next) {
                if (!edict->entity || edict->entity == world) {
                    continue;
                }

                if (edict->s.modelindex < 1 || edict->s.modelindex > navigationData.GetNumSubmodels()) {
                    continue;
                }

                const navModel_t& submodel = navigationData.GetSubmodel(edict->s.modelindex - 1);
                if (!submodel.surfaces.NumObjects()) {
                    // Could be a trigger
                    continue;
                }

                DrawModel(ent->origin, submodel, maxDistSquared, edict->entity->origin);
            }
        }
        break;
    }

    if (ai_showpath->integer && navMeshDt) {
        switch (ai_showpath->integer) {
        default:
        case 0:
            ai_startpath = ai_endpath = vec_zero;
            break;
        case 1:
            ai_startpath = ent->origin;
            break;
        case 2:
            ai_endpath = ent->origin;
            break;
        case 3:
            if (ai_lastpath != ai_showpath->integer) {
                TestAgent(ai_startpath, ai_endpath, ai_pathlist, &ai_numPaths, ARRAY_LEN(ai_pathlist));
            }
            break;
        }

        ai_lastpath = ai_showpath->integer;

        if (crowd) {
            crowd->update(level.frametime, NULL);
        }

        for (int i = 0; i < ai_numPaths - 1; i++) {
            const Vector v1 = ai_pathlist[i] + Vector(0, 0, 16);
            const Vector v2 = ai_pathlist[i + 1] + Vector(0, 0, 16);

            G_DebugLine(v1, v2, 1.0, 0.0, 1.0, 1.0);
        }

#if 0
        if (agentId != -1) {
            const dtCrowdAgent* agent = crowd->getAgent(agentId);

            const dtPolyRef* path = agent->corridor.getPath();
            const int npath = agent->corridor.getPathCount();
            for (int i = 0; i < npath; ++i) {
                const dtMeshTile* tile;
                const dtPoly* poly;
                navMeshDt->getTileAndPolyByRef(path[i], &tile, &poly);

                const unsigned int tileId = (unsigned int)(poly - tile->polys);

                const dtPolyDetail* dm = &tile->detailMeshes[tileId];

                for (int j = 0; j < dm->triCount; ++j)
                {
                    const unsigned char* t = &tile->detailTris[(dm->triBase + i) * 4];
                    for (int k = 0; k < 3; ++k)
                    {
                        const float* pv1;
                        const float* pv2;

                        if (t[k] < poly->vertCount) {
                            pv1 = &tile->verts[poly->verts[t[k]] * 3];
                            pv2 = &tile->verts[poly->verts[t[(k + 1) % 3]] * 3];
                        } else {
                            pv1 = &tile->detailVerts[(dm->vertBase + t[k] - poly->vertCount) * 3];
                            pv2 = &tile->detailVerts[(dm->vertBase + t[(k + 1) % 3] - poly->vertCount) * 3];
                        }

                        Vector v1, v2;
                        ConvertRecastToGameCoord(pv1, v1);
                        ConvertRecastToGameCoord(pv2, v2);

                        G_DebugLine(v1, v2, 0.0, 1.0, 0.0, 1.0);
                    }
                }
            }
        }
#endif

#if 0
        const dtNodePool* pool = navMeshQuery->getNodePool();

        if (pool) {
            for (int i = 0; i < pool->getHashSize(); ++i)
            {
                for (dtNodeIndex j = pool->getFirst(i); j != DT_NULL_IDX; j = pool->getNext(j))
                {
                    const dtNode* node = pool->getNodeAtIdx(j + 1);
                    if (!node) continue;

                    Vector pos;
                    ConvertRecastToGameCoord(node->pos, pos);

                    G_DebugBBox(pos, Vector(-8, -8, -8), Vector(8, 8, 8), 0.0, 1.0, 0.0, 1.0);
                }
            }
        }
#endif
    }
}

/*
============
G_Navigation_DebugDraw
============
*/
void G_Navigation_DebugDraw()
{
    navigationMapDebug.DebugDraw();
}
