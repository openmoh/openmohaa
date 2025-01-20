/*
===========================================================================
Copyright (C) 2024 the OpenMoHAA team

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

// navigate.cpp: C++ implementation of the A* search algorithm.
//

#include "g_local.h"
#include "navigate.h"
#include "misc.h"
#include "doors.h"
#include "actor.h"
#include "player.h"
#include "debuglines.h"
#include "scriptexception.h"
#include "gamecmds.h"

#define PATHFILE_VERSION 103

int     path_checkthisframe;
cvar_t *ai_showroutes;
cvar_t *ai_showroutes_distance;
cvar_t *ai_shownodenums;
cvar_t *ai_shownode;
cvar_t *ai_showallnode;
cvar_t *ai_showpath;
cvar_t *ai_fallheight;
cvar_t *ai_debugpath;
cvar_t *ai_pathchecktime;
cvar_t *ai_pathcheckdist;
cvar_t *ai_editmode; // Added in OPM

static const vec_t *path_start;
static const vec_t *path_end;
static PathNode    *Node;
static vec2_t       path_totaldir;
static vec3_t       path_p;
static vec2_t       path_startdir;

static Entity *IgnoreObjects[MAX_GENTITIES];
static int     NumIgnoreObjects;

static qboolean pathnodesinitialized = false;
static qboolean loadingarchive       = false;
static qboolean pathnodescalculated  = false;
int             ai_maxnode;

MapCell   PathSearch::PathMap[PATHMAP_GRIDSIZE][PATHMAP_GRIDSIZE];
PathNode *PathSearch::open;
int       PathSearch::findFrame;
qboolean  PathSearch::m_bNodesloaded;
qboolean  PathSearch::m_NodeCheckFailed;
int       PathSearch::m_LoadIndex;

PathNode   *PathSearch::pathnodes[MAX_PATHNODES];
int         PathSearch::nodecount;
float       PathSearch::total_dist;
const char *PathSearch::last_error;

byte *bulkNavMemory      = NULL;
byte *startBulkNavMemory = NULL;

Vector PLAYER_BASE_MIN(-15.5f, -15.5f, 0);
Vector PLAYER_BASE_MAX(15.5f, 15.5f, 0);
Vector testpos[200];
Vector ai_startpath;
Vector ai_endpath;

float NODE_MINS[3]                   = {-15, -15, 0};
float NODE_MAXS[3]                   = {15, 15, 94};
float COLOR_PATHNODE_ERROR[3]        = {0, 0, 0};
float COLOR_PATHNODE_COVER[3]        = {0, 1, 0};
float COLOR_PATHNODE_CORNER_LEFT[3]  = {1, 1, 0};
float COLOR_PATHNODE_CORNER_RIGHT[3] = {0.7f, 1, 0};
float COLOR_PATHNODE_SNIPER[3]       = {1, 0, 0};
float COLOR_PATHNODE_CRATE[3]        = {3, 0, 0};
float COLOR_PATHNODE_CONCEALMENT[3]  = {0, 0, 1};
float COLOR_PATHNODE_DUCK[3]         = {0, 1, 1};
float COLOR_PATHNODE_DEFAULT[3]      = {1, 0, 1};

int               testcount = 0;
static ActorPath *test_path = NULL;

struct {
    float fMinRangeSquared;
    float fMaxRangeSquared;
    float fMinAngle;
    float fMaxAngle;
} g_AttackParms[] = {
    {Square(64), Square(2048), 150.0f, 210.0f},
    {Square(64), Square(2048), 150.0f, 210.0f},
    {Square(96), Square(2048), 320.0f, 40.0f },
    {Square(96), Square(4096), 0.0f,   0.0f  },
};

PathSearch PathManager;

int path_checksthisframe;

PathInfo *PathSearch::GeneratePath(PathInfo *path)
{
    PathNode  *ParentNode;
    pathway_t *pathway;
    float      dist;
    float      dir[2];
    PathInfo  *current_path;

    current_path = path;

    dir[0] = path_end[0] - Node->m_PathPos[0];
    dir[1] = path_end[1] - Node->m_PathPos[1];

    dist = VectorNormalize2D(dir);

    total_dist = dist + Node->g;

    VectorCopy(path_end, current_path->point);

    ParentNode = Node->Parent;
    if (ParentNode) {
        pathway = &ParentNode->Child[Node->pathway];
        VectorSub2D(path_end, pathway->pos2, current_path->dir);
        current_path->dist = VectorNormalize2D(current_path->dir);

        if (path->dist) {
            path->bAccurate = false;
            current_path++;
        }

        if (pathway->dist) {
            VectorCopy(pathway->pos2, current_path->point);
            VectorCopy2D(pathway->dir, current_path->dir);
            current_path->dist      = pathway->dist;
            current_path->bAccurate = true;
            assert(current_path->dist > -1e+07 && current_path->dist < 1e+07);
            current_path++;
        }

        for (Node = ParentNode, ParentNode = ParentNode->Parent; ParentNode != NULL;
             Node = ParentNode, ParentNode = ParentNode->Parent) {
            pathway = &ParentNode->Child[Node->pathway];
            if (pathway->dist) {
                VectorCopy(pathway->pos2, current_path->point);
                VectorCopy2D(pathway->dir, current_path->dir);
                current_path->dist = pathway->dist;
                assert(current_path->dist > -1e+07 && current_path->dist < 1e+07);
                current_path->bAccurate = true;
                current_path++;
            }
        }

        VectorCopy(pathway->pos1, current_path->point);
        VectorCopy2D(path_startdir, current_path->dir);
        current_path->dist = Node->g;
        assert(current_path->dist > -1e+07 && current_path->dist < 1e+07);
    } else {
        VectorCopy2D(path_totaldir, current_path->dir);
        path->dist = Node->h;
    }

    if (current_path->dist) {
        current_path->bAccurate = false;
        current_path++;

        VectorCopy(path_start, current_path->point);
        current_path->dist = 0;
        VectorClear2D(current_path->dir);
    }

    current_path->bAccurate = false;
    return current_path;
}

PathInfo *PathSearch::GeneratePathNear(PathInfo *path)
{
    PathInfo  *current_path = path;
    pathway_t *pathway;
    PathNode  *ParentNode;

    total_dist = Node->g;
    VectorCopy(Node->m_PathPos, path->point);

    ParentNode = Node->Parent;
    if (ParentNode) {
        pathway = &ParentNode->Child[Node->pathway];

        if (pathway->dist) {
            VectorCopy(pathway->pos2, path->point);
            VectorCopy2D(pathway->dir, path->dir);
            path->dist = pathway->dist;

            current_path->bAccurate = true;
            current_path++;
        }

        for (Node = ParentNode, ParentNode = ParentNode->Parent; ParentNode != NULL;
             Node = ParentNode, ParentNode = ParentNode->Parent) {
            pathway = &ParentNode->Child[Node->pathway];
            if (pathway->dist) {
                VectorCopy(pathway->pos2, current_path->point);
                VectorCopy2D(pathway->dir, current_path->dir);
                current_path->dist      = pathway->dist;
                current_path->bAccurate = true;
                assert(current_path->dist > -1e+07 && current_path->dist < 1e+07);
                current_path++;
            }
        }

        VectorCopy(pathway->pos1, current_path->point);
        VectorCopy2D(path_startdir, current_path->dir);
        current_path->dist = Node->g;
    } else {
        VectorCopy2D(path_totaldir, current_path->dir);
        path->dist = Node->h;
    }

    if (current_path->dist) {
        current_path->bAccurate = false;
        current_path++;

        VectorCopy(path_start, current_path->point);
        VectorClear2D(current_path->dir);
        current_path->dist = 0;
    }

    current_path->bAccurate = false;
    return current_path;
}

PathInfo *PathSearch::GeneratePathAway(PathInfo *path)
{
    PathInfo  *current_path = path;
    pathway_t *pathway;
    PathNode  *ParentNode;

    VectorCopy(Node->m_PathPos, path->point);

    ParentNode = Node->Parent;
    if (ParentNode) {
        pathway = &ParentNode->Child[Node->pathway];

        if (pathway->dist) {
            VectorCopy(pathway->pos2, current_path->point);
            VectorCopy2D(pathway->dir, current_path->dir);
            current_path->dist = pathway->dist;

            current_path->bAccurate = true;
            assert(current_path->dist > -1e+07 && current_path->dist < 1e+07);
            current_path++;
        }

        for (Node = ParentNode, ParentNode = ParentNode->Parent; ParentNode != NULL;
             Node = ParentNode, ParentNode = ParentNode->Parent) {
            pathway = &ParentNode->Child[Node->pathway];
            if (pathway->dist) {
                VectorCopy(pathway->pos2, current_path->point);
                VectorCopy2D(pathway->dir, current_path->dir);
                current_path->dist      = pathway->dist;
                current_path->bAccurate = true;
                assert(current_path->dist > -1e+07 && current_path->dist < 1e+07);
                current_path++;
            }
        }

        VectorCopy(pathway->pos1, current_path->point);
        VectorCopy2D(pathway->pos1, current_path->point);

        current_path->dist = Node->g;

        if (Node->g) {
            current_path->bAccurate = false;
            current_path++;
            VectorCopy(path_start, current_path->point);
            VectorClear2D(current_path->dir);
            current_path->dist = 0;
        }
    } else {
        VectorClear2D(path->dir);
        path->dist = 0;
    }

    current_path->bAccurate = false;
    return current_path;
}

int PathSearch::FindPath(
    const vec3_t start,
    const vec3_t end,
    Entity      *ent,
    float        maxPath,
    const vec3_t vLeashHome,
    float        fLeashDistSquared,
    int          fallheight
)
{
    int        i;
    int        g;
    PathNode  *NewNode;
    pathway_t *pathway;
    PathNode  *prev;
    PathNode  *next;
    int        f;
    vec2_t     delta;
    PathNode  *to;

    if (ent) {
        // Added in OPM
        //  Check for simple actor
        if (ent->IsSubclassOfActor()) {
            Node = NearestStartNode(start, static_cast<SimpleActor*>(ent));
        } else {
            Node = DebugNearestStartNode(start, ent);
        }
    } else {
        Node = DebugNearestStartNode(start);
    }

    if (!Node) {
        last_error = "couldn't find start node";
        return 0;
    }

    to = NearestEndNode(end);
    if (!to) {
        last_error = "couldn't find end node";
        return 0;
    }

    total_dist = 1e+12f;

    if (!maxPath) {
        maxPath = 1e+12f;
    }

    findFrame++;
    open = NULL;

    VectorSub2D(Node->origin, start, path_startdir);
    Node->g = VectorNormalize2D(path_startdir);

    VectorSub2D(end, start, path_totaldir);
    Node->h = VectorNormalize2D(path_totaldir);

    Node->Parent    = NULL;
    Node->m_Depth   = 3;
    Node->findCount = findFrame;
    Node->inopen    = true;
    Node->PrevNode  = NULL;
    Node->NextNode  = NULL;
    Node->m_PathPos = start;

    open = Node;

    while (open) {
        Node         = open;
        open->inopen = false;
        open         = Node->NextNode;

        if (open) {
            open->PrevNode = NULL;
        }

        if (Node == to) {
            path_start = start;
            path_end   = end;
            return Node->m_Depth;
        }

        for (i = Node->numChildren - 1; i >= 0; i--) {
            vec2_t vDist;

            pathway = &Node->Child[i];

            NewNode = pathnodes[pathway->node];
            if (!NewNode) {
                continue;
            }

            if (vLeashHome) {
                VectorSub2D(pathway->pos2, vLeashHome, vDist);
                if (VectorLength2DSquared(vDist) > fLeashDistSquared) {
                    continue;
                }
            }

            g = (int)(pathway->dist + Node->g + 1.0f);

            if (NewNode->findCount == findFrame) {
                if (NewNode->g <= g) {
                    continue;
                }

                if (NewNode->inopen) {
                    NewNode->inopen = false;
                    next            = NewNode->NextNode;
                    prev            = NewNode->PrevNode;

                    if (next) {
                        next->PrevNode = prev;
                    }

                    if (prev) {
                        prev->NextNode = next;
                    } else {
                        open = next;
                    }
                }
            }

            VectorSub2D(end, pathway->pos2, delta);
            NewNode->h = VectorLength2D(delta);

            f = (int)((float)g + NewNode->h);

            if (f >= maxPath) {
                last_error = "specified path distance exceeded";
                return 0;
            }

            if (pathway->fallheight <= fallheight
                && (!ent || !ent->IsSubclassOfSentient() || !pathway->badPlaceTeam[static_cast<Sentient *>(ent)->m_Team]
                )) {
                NewNode->m_Depth   = Node->m_Depth + 1;
                NewNode->Parent    = Node;
                NewNode->pathway   = i;
                NewNode->g         = (float)g;
                NewNode->f         = (float)f;
                NewNode->m_PathPos = pathway->pos2;
                NewNode->findCount = findFrame;
                NewNode->inopen    = true;

                if (!open) {
                    NewNode->NextNode = NULL;
                    NewNode->PrevNode = NULL;
                    open              = NewNode;
                    continue;
                }

                if (open->f >= f) {
                    NewNode->NextNode = open;
                    NewNode->PrevNode = NULL;

                    open->PrevNode = NewNode;
                    open           = NewNode;
                    continue;
                }

                prev = open;
                for (next = open->NextNode; next; next = next->NextNode) {
                    if (next->f >= f) {
                        break;
                    }
                    prev = next;
                }

                NewNode->NextNode = next;
                if (next) {
                    next->PrevNode = NewNode;
                }
                prev->NextNode    = NewNode;
                NewNode->PrevNode = prev;
            }
        }
    }

    last_error = "unreachable path";
    return 0;
}

int PathSearch::FindPathNear(
    const vec3_t start,
    const vec3_t end,
    Entity      *ent,
    float        maxPath,
    float        fRadiusSquared,
    const vec3_t vLeashHome,
    float        fLeashDistSquared,
    int          fallheight
)
{
    int        i;
    int        g;
    PathNode  *NewNode;
    pathway_t *pathway;
    PathNode  *prev;
    PathNode  *next;
    int        f;
    vec2_t     dir;
    vec2_t     delta;

    if (ent) {
        // Added in OPM
        //  Check for simple actor
        if (ent->IsSubclassOfActor()) {
            Node = NearestStartNode(start, static_cast<SimpleActor*>(ent));
        } else {
            Node = DebugNearestStartNode(start, ent);
        }
    } else {
        Node = DebugNearestStartNode(start);
    }

    if (!Node) {
        last_error = "no start node";
        return 0;
    }

    total_dist = 1e12f;

    if (!maxPath) {
        maxPath = 1e12f;
    }

    findFrame++;
    open = NULL;

    VectorSub2D(Node->origin, start, path_startdir);
    VectorSub2D(end, start, delta);
    VectorCopy2D(delta, dir);

    Node->inopen    = true;
    Node->g         = VectorNormalize2D(path_startdir);
    Node->h         = VectorNormalize2D(dir);
    Node->Parent    = NULL;
    Node->m_Depth   = 3;
    Node->findCount = findFrame;
    Node->PrevNode  = NULL;
    Node->NextNode  = NULL;
    Node->m_PathPos = start;

    open = Node;

    while (open) {
        Node         = open;
        open->inopen = false;
        open         = Node->NextNode;

        if (open) {
            open->PrevNode = NULL;
        }

        VectorSub2D(end, Node->m_PathPos, delta);

        if (fRadiusSquared >= VectorLength2DSquared(delta)) {
            path_start = start;
            path_end   = end;
            return Node->m_Depth;
        }

        for (i = Node->numChildren - 1; i >= 0; i--) {
            pathway = &Node->Child[i];

            NewNode = pathnodes[pathway->node];
            if (!NewNode) {
                continue;
            }

            g = (int)(pathway->dist + Node->g + 1.0f);

            if (NewNode->findCount == findFrame) {
                if (NewNode->g <= g) {
                    continue;
                }

                if (NewNode->inopen) {
                    NewNode->inopen = false;
                    next            = NewNode->NextNode;
                    prev            = NewNode->PrevNode;

                    if (next) {
                        next->PrevNode = prev;
                    }

                    if (prev) {
                        prev->NextNode = next;
                    } else {
                        open = next;
                    }
                }
            }

            VectorSub2D(end, pathway->pos2, delta);
            NewNode->h = VectorLength2D(delta);

            f = (int)((float)g + NewNode->h);

            if (f >= maxPath) {
                last_error = "specified path distance exceeded";
                return 0;
            }

            if (pathway->fallheight <= fallheight
                && (!ent || !ent->IsSubclassOfSentient() || !pathway->badPlaceTeam[static_cast<Sentient *>(ent)->m_Team]
                )) {
                NewNode->m_Depth   = Node->m_Depth + 1;
                NewNode->Parent    = Node;
                NewNode->pathway   = i;
                NewNode->g         = (float)g;
                NewNode->f         = (float)f;
                NewNode->m_PathPos = pathway->pos2;
                NewNode->findCount = findFrame;
                NewNode->inopen    = true;

                if (!open) {
                    NewNode->NextNode = NULL;
                    NewNode->PrevNode = NULL;
                    open              = NewNode;
                    continue;
                }

                if (open->f >= f) {
                    NewNode->NextNode = open;
                    NewNode->PrevNode = NULL;

                    open->PrevNode = NewNode;
                    open           = NewNode;
                    continue;
                }

                prev = open;
                for (next = open->NextNode; next; next = next->NextNode) {
                    if (next->f >= f) {
                        break;
                    }
                    prev = next;
                }

                NewNode->NextNode = next;
                if (next) {
                    next->PrevNode = NewNode;
                }
                prev->NextNode    = NewNode;
                NewNode->PrevNode = prev;
            }
        }
    }

    last_error = "unreachable path";
    return 0;
}

int PathSearch::FindPathAway(
    const vec3_t start,
    const vec3_t avoid,
    const vec3_t vPreferredDir,
    Entity      *ent,
    float        fMinSafeDist,
    const vec3_t vLeashHome,
    float        fLeashDistSquared,
    int          fallheight
)
{
    int        i;
    int        g;
    PathNode  *NewNode;
    pathway_t *pathway;
    PathNode  *prev;
    PathNode  *next;
    int        f;
    float      fBias;
    vec2_t     delta;
    float      fMinSafeDistSquared;

    fMinSafeDistSquared = fMinSafeDist * fMinSafeDist;

    if (ent) {
        // Added in OPM
        //  Check for simple actor
        if (ent->IsSubclassOfActor()) {
            Node = NearestStartNode(start, static_cast<SimpleActor*>(ent));
        } else {
            Node = DebugNearestStartNode(start, ent);
        }
    } else {
        Node = DebugNearestStartNode(start);
    }

    if (!Node) {
        last_error = "couldn't find start node";
        return 0;
    }

    findFrame++;
    open = NULL;

    VectorSub2D(Node->origin, start, path_startdir);
    VectorSub2D(start, avoid, delta);

    fBias = VectorLength2D(vPreferredDir);

    Node->inopen = true;
    Node->g      = VectorNormalize2D(path_startdir);
    Node->h      = fMinSafeDist - VectorNormalize2D(delta);
    Node->h += fBias - DotProduct2D(vPreferredDir, delta);
    Node->Parent    = NULL;
    Node->m_Depth   = 2;
    Node->findCount = findFrame;
    Node->PrevNode  = NULL;
    Node->NextNode  = NULL;
    Node->m_PathPos = start;

    open = Node;

    while (open) {
        Node         = open;
        open->inopen = false;
        open         = Node->NextNode;

        if (open) {
            open->PrevNode = NULL;
        }

        VectorSub2D(Node->m_PathPos, avoid, delta);

        if (VectorLength2DSquared(delta) >= fMinSafeDistSquared) {
            path_start = start;
            return Node->m_Depth;
        }

        for (i = Node->numChildren - 1; i >= 0; i--) {
            vec2_t vDist;

            pathway = &Node->Child[i];

            NewNode = pathnodes[pathway->node];
            if (!NewNode) {
                continue;
            }

            if (vLeashHome) {
                VectorSub2D(pathway->pos2, vLeashHome, vDist);
                if (VectorLength2DSquared(vDist) > fLeashDistSquared) {
                    continue;
                }
            }

            g = (int)(pathway->dist + Node->g + 1.0f);

            if (NewNode->findCount == findFrame) {
                if (NewNode->g <= g) {
                    continue;
                }

                if (NewNode->inopen) {
                    NewNode->inopen = false;
                    next            = NewNode->NextNode;
                    prev            = NewNode->PrevNode;

                    if (next) {
                        next->PrevNode = prev;
                    }

                    if (prev) {
                        prev->NextNode = next;
                    } else {
                        open = next;
                    }
                }
            }

            VectorSub2D(pathway->pos2, avoid, delta);
            NewNode->h = VectorNormalize2D(delta);
            NewNode->h += fBias - DotProduct2D(delta, vPreferredDir);

            f = (int)((float)g + NewNode->h);

            if (pathway->fallheight <= fallheight) {
                NewNode->m_Depth   = Node->m_Depth + 1;
                NewNode->Parent    = Node;
                NewNode->pathway   = i;
                NewNode->g         = (float)g;
                NewNode->f         = (float)f;
                NewNode->m_PathPos = pathway->pos2;
                NewNode->findCount = findFrame;
                NewNode->inopen    = true;

                if (!open) {
                    NewNode->NextNode = NULL;
                    NewNode->PrevNode = NULL;
                    open              = NewNode;
                    continue;
                }

                if (open->f >= f) {
                    NewNode->NextNode = open;
                    NewNode->PrevNode = NULL;

                    open->PrevNode = NewNode;
                    open           = NewNode;
                    continue;
                }

                prev = open;
                for (next = open->NextNode; next; next = next->NextNode) {
                    if (next->f >= f) {
                        break;
                    }
                    prev = next;
                }

                NewNode->NextNode = next;
                if (next) {
                    next->PrevNode = NewNode;
                }
                prev->NextNode    = NewNode;
                NewNode->PrevNode = prev;
            }
        }
    }

    last_error = "unreachable path";
    return 0;
}

PathNode *PathSearch::FindCornerNodeForWall(
    const vec3_t start, const vec3_t end, Entity *ent, float maxPath, const vec4_t plane
)
{
    int        i, g;
    PathNode  *NewNode;
    pathway_t *pathway;
    PathNode  *prev, *next;
    int        f;
    vec2_t     delta;
    vec2_t     dir;

    if (ent) {
        // Added in OPM
        //  Check for simple actor
        if (ent->IsSubclassOfActor()) {
            Node = NearestStartNode(start, static_cast<SimpleActor*>(ent));
        } else {
            Node = DebugNearestStartNode(start, ent);
        }
    } else {
        Node = DebugNearestStartNode(start);
    }

    if (!Node) {
        last_error = "couldn't find start node";
        return NULL;
    }

    if (DotProduct(start, plane) - plane[3] < 0.0) {
        last_error = "starting point is already behind the wall";
        return NULL;
    }

    if (DotProduct(plane, end) - plane[3] > 0.0) {
        last_error = "end point is in front of the wall";
        return NULL;
    }

    total_dist = 1e12f;

    if (maxPath == 0.0) {
        maxPath = 1e12f;
    }

    findFrame++;
    open = NULL;

    VectorSub2D(Node->origin, start, path_startdir);
    Node->g = VectorNormalize2D(path_startdir);

    VectorSub2D(end, start, path_totaldir);
    Node->h         = VectorNormalize2D(path_totaldir);
    Node->inopen    = true;
    Node->Parent    = NULL;
    Node->m_Depth   = 3;
    Node->findCount = findFrame;
    Node->PrevNode  = 0;
    Node->NextNode  = 0;
    Node->m_PathPos = start;

    open = Node;

    while (open) {
        Node         = open;
        open->inopen = false;
        open         = Node->NextNode;

        if (open) {
            open->PrevNode = NULL;
        }

        if (Node->Parent && DotProduct(Node->m_PathPos, plane) - plane[3] < 0) {
            VectorSub2D(Node->m_PathPos, start, delta);

            if (VectorLength2DSquared(delta) >= 256) {
                return Node->Parent;
            }
            return Node;
        }

        for (i = Node->numChildren - 1; i >= 0; i--) {
            pathway = &Node->Child[i];

            NewNode = pathnodes[pathway->node];
            if (!NewNode) {
                continue;
            }

            g = (int)(pathway->dist + Node->g + 1.0f);

            if (NewNode->findCount == findFrame) {
                if (NewNode->g <= g) {
                    continue;
                }

                if (NewNode->inopen) {
                    NewNode->inopen = false;
                    next            = NewNode->NextNode;
                    prev            = NewNode->PrevNode;

                    if (next) {
                        next->PrevNode = prev;
                    }

                    if (prev) {
                        prev->NextNode = next;
                    } else {
                        open = next;
                    }
                }
            }

            VectorSub2D(end, pathway->pos2, dir);
            NewNode->h = VectorNormalize2D(dir);

            f = (int)((float)g + NewNode->h);

            if (f >= maxPath) {
                last_error = "specified path distance exceeded";
                return 0;
            }

            NewNode->m_Depth   = Node->m_Depth + 1;
            NewNode->Parent    = Node;
            NewNode->pathway   = i;
            NewNode->g         = (float)g;
            NewNode->f         = (float)f;
            NewNode->m_PathPos = pathway->pos2;
            NewNode->findCount = findFrame;
            NewNode->inopen    = true;

            if (!open) {
                NewNode->NextNode = NULL;
                NewNode->PrevNode = NULL;
                open              = NewNode;
                continue;
            }

            if (open->f >= f) {
                NewNode->NextNode = open;
                NewNode->PrevNode = NULL;

                open->PrevNode = NewNode;
                open           = NewNode;
                continue;
            }

            prev = open;
            for (next = open->NextNode; next; next = next->NextNode) {
                if (next->f >= f) {
                    break;
                }
                prev = next;
            }

            NewNode->NextNode = next;
            if (next) {
                next->PrevNode = NewNode;
            }
            prev->NextNode    = NewNode;
            NewNode->PrevNode = prev;
        }
    }

    last_error = "unreachable path";
    return NULL;
}

PathNode *PathSearch::FindCornerNodeForExactPath(Entity *pSelf, Sentient *enemy, float fMaxPath)
{
    PathNode *pPathNode[4096];
    PathNode *pParentNode;
    size_t    i, iDepth;
    Vector    vEnd;
    Vector    vEyeDelta;
    Vector    vEyePos;

    iDepth = PathSearch::FindPath(enemy->origin, pSelf->origin, pSelf, fMaxPath, 0, 0.0, 100);
    if (!iDepth) {
        return NULL;
    }

    if (pSelf->IsSubclassOfActor()) {
        vEyePos = static_cast<SimpleActor*>(pSelf)->EyePosition();
    } else {
        vEyePos = pSelf->origin + Vector(0, 0, pSelf->maxs.z);
    }

    vEyeDelta = vEyePos - pSelf->origin;

    for (pParentNode = Node->Parent, i = 0; pParentNode; pParentNode = pParentNode->Parent, i++) {
        Node         = pParentNode;
        pPathNode[i] = pParentNode;
    }

    iDepth = i;
    if (!iDepth) {
        return NULL;
    }

    for (i = 1; i < iDepth; i += 2) {
        vEnd = vEyeDelta + pPathNode[i]->m_PathPos;

        if (!G_SightTrace(
                vEyePos, vec_zero, vec_zero, vEnd, pSelf, enemy, MASK_CORNER_NODE, qfalse, "FindCornerNodeFoExactPath 1"
            )) {
            break;
        }
    }

    i--;
    if (i >= iDepth) {
        i = iDepth - 1;
        return pPathNode[i];
    }

    if (i) {
        vEnd = vEyeDelta + pPathNode[i]->m_PathPos;

        if (!G_SightTrace(
                vEyePos, vec_zero, vec_zero, vEnd, pSelf, enemy, MASK_CORNER_NODE, qfalse, "FindCornerNodeFoExactPath 2"
            )) {
            i--;
        }
    }

    return pPathNode[i];
}

void PathSearch::ResetNodes(void)
{
    int i;
    int x;
    int y;

    m_bNodesloaded = false;
    m_LoadIndex    = -1;

    if (!startBulkNavMemory && nodecount) {
        for (x = 0; x < PATHMAP_GRIDSIZE; x++) {
            for (y = 0; y < PATHMAP_GRIDSIZE; y++) {
                if (PathMap[x][y].nodes) {
                    gi.Free(PathMap[x][y].nodes);
                }
            }
        }

        for (i = 0; i < nodecount; i++) {
            if (pathnodes[i] && pathnodes[i]->Child) {
                gi.Free(pathnodes[i]->Child);
            }
        }
    }

    for (x = 0; x < PATHMAP_GRIDSIZE; x++) {
        for (y = 0; y < PATHMAP_GRIDSIZE; y++) {
            PathMap[x][y] = MapCell();
        }
    }

    for (i = 0; i < nodecount; i++) {
        if (pathnodes[i]) {
            delete pathnodes[i];
            pathnodes[i] = NULL;
        }
    }

    nodecount = 0;

    // Free the bulk nav' memory
    if (startBulkNavMemory) {
        gi.Free(startBulkNavMemory);
        bulkNavMemory      = NULL;
        startBulkNavMemory = NULL;
    }
}

void PathSearch::ClearNodes(void)
{
    int i, j;
    int x;
    int y;

    m_bNodesloaded = false;
    m_LoadIndex    = -1;

    if (!startBulkNavMemory && nodecount) {
        for (x = 0; x < PATHMAP_GRIDSIZE; x++) {
            for (y = 0; y < PATHMAP_GRIDSIZE; y++) {
                if (PathMap[x][y].nodes) {
                    gi.Free(PathMap[x][y].nodes);
                }
            }
        }

        for (i = 0; i < nodecount; i++) {
            if (pathnodes[i] && pathnodes[i]->Child) {
                gi.Free(pathnodes[i]->Child);
            }
        }
    }

    for (x = 0; x < PATHMAP_GRIDSIZE; x++) {
        for (y = 0; y < PATHMAP_GRIDSIZE; y++) {
            PathMap[x][y] = MapCell();
        }
    }

    for (i = 0; i < nodecount; i++) {
        if (pathnodes[i]) {
            pathnodes[i]->Child              = NULL;
            pathnodes[i]->virtualNumChildren = 0;
            pathnodes[i]->numChildren        = 0;
            pathnodes[i]->findCount          = 0;
        }
    }

    // Rebuild node list
    for (i = 0; i < nodecount; i++) {
        PathNode *node1 = pathnodes[i];
        if (node1) {
            continue;
        }

        for (j = i + 1; j < nodecount; j++) {
            PathNode *node2 = pathnodes[j];
            if (node2) {
                pathnodes[i]   = node2;
                pathnodes[j]   = NULL;
                node2->nodenum = i;
                break;
            }
        }
    }

    while (nodecount > 0 && !pathnodes[nodecount - 1]) {
        nodecount--;
    }

    // Free the bulk nav' memory
    if (startBulkNavMemory) {
        gi.Free(startBulkNavMemory);
        bulkNavMemory      = NULL;
        startBulkNavMemory = NULL;
    }
}

void PathSearch::UpdatePathwaysForBadPlace(const Vector& origin, float radius, int dir, int team)
{
    float radiusSqr;
    int   i, j, k;

    radiusSqr = radius * radius;

    for (i = 0; i < nodecount; i++) {
        PathNode *node = pathnodes[i];
        if (!node) {
            continue;
        }

        for (j = node->virtualNumChildren; j > 0; j--) {
            pathway_t& pathway = node->Child[j - 1];
            if (PointToSegmentDistanceSquared(origin, pathway.pos1, pathway.pos2) < radiusSqr) {
                for (k = 0; k < 2; k++) {
                    if ((1 << k) & team) {
                        pathway.badPlaceTeam[k] += dir;
                    }
                }
            }
        }
    }
}

void AI_AddNode(PathNode *node)
{
    int i;

    assert(node);

    //
    // Find a free slot
    //
    for (i = 0; i < PathSearch::nodecount; i++) {
        if (!PathSearch::pathnodes[i]) {
            PathSearch::pathnodes[i] = node;
            node->nodenum            = i;
            return;
        }
    }

    if (i > ai_maxnode) {
        ai_maxnode = i;
    }

    if (i >= MAX_PATHNODES) {
        gi.Error(ERR_DROP, "Exceeded MAX_PATHNODES!\n");
        return;
    }

    PathSearch::pathnodes[i] = node;
    node->nodenum            = i;

    PathSearch::nodecount++;
}

void AI_RemoveNode(PathNode *node)
{
    int num;

    num = node->nodenum;
    delete node;

    PathSearch::pathnodes[num] = NULL;
    if (num == PathSearch::nodecount) {
        PathSearch::nodecount--;
    }
}

/*****************************************************************************/
/*QUAKED info_pathnode (1 0 0) (-24 -24 0) (24 24 32) FLEE DUCK COVER DOOR JUMP LADDER

FLEE marks the node as a safe place to flee to.  Actor will be removed when it reaches a flee node and is not visible to a player.

DUCK marks the node as a good place to duck behind during weapon fire.

COVER marks the node as a good place to hide behind during weapon fire.

DOOR marks the node as a door node.  If an adjacent node has DOOR marked as well, the actor will only use the path if the door in between them is unlocked.

JUMP marks the node as one to jump from when going to the node specified by target.
"target" the pathnode to jump to.

******************************************************************************/

Event EV_Path_SetNodeFlags
(
    "spawnflags",
    EV_DEFAULT,
    "i",
    "node_flags",
    "Sets the path nodes flags.",
    EV_NORMAL
);

// Added in 2.0
Event EV_Path_SetLowWallArc
(
    "low_wall_arc",
    EV_DEFAULT,
    "f",
    "arc_half_angle",
    "Marks this node as good for low-wall behavior"
    "and gives the arc"
);

CLASS_DECLARATION(SimpleEntity, PathNode, "info_pathnode") {
    {&EV_Path_SetNodeFlags,  &PathNode::SetNodeFlags },
    {&EV_IsTouching,         &PathNode::IsTouching   },
    {&EV_Delete,             &PathNode::Remove       },
    {&EV_Remove,             &PathNode::Remove       },
    {&EV_Path_SetLowWallArc, &PathNode::SetLowWallArc},
    {NULL,                   NULL                    }
};

static Vector pathNodesChecksum;
static int    numLoadNodes = 0;
static int    numNodes     = 0;

void *PathNode::operator new(size_t size)
{
    return PathManager.AllocPathNode();
}

void PathNode::operator delete(void *ptr)
{
    return PathManager.FreePathNode(ptr);
}

PathNode::PathNode()
{
    entflags |= ECF_PATHNODE;
    findCount      = 0;
    pLastClaimer   = NULL;
    numChildren    = 0;
    iAvailableTime = -1;

    if (!loadingarchive) {
        // our archive function will take care of this stuff
        AI_AddNode(this);
        nodeflags          = 0;
        m_fLowWallArc      = 0;
        pLastClaimer       = NULL;
        iAvailableTime     = -1;
        virtualNumChildren = 0;
        Child              = NULL;
    }
}

PathNode::~PathNode()
{
    entflags &= ~ECF_PATHNODE;
}

void PathNode::Remove(Event *ev)
{
    // Pathnodes mustn't be removed
    ScriptError("Not allowed to delete a path node");
}

void PathNode::SetNodeFlags(Event *ev)
{
    nodeflags = ev->GetInteger(1);
}

void PathNode::SetLowWallArc(Event *ev)
{
    float value = ev->GetFloat(1);
    if (value < 0 || value >= 180) {
        ScriptError("low_wall_arc must be >= 0 and < 180");
    }

    m_fLowWallArc = value;

    if (!value) {
        nodeflags &= ~AI_LOW_WALL_ARC;
    } else {
        nodeflags |= AI_LOW_WALL_ARC;
    }
}

void PathNode::ConnectTo(PathNode *node)
{
    Child[virtualNumChildren].node            = node->nodenum;
    Child[virtualNumChildren].numBlockers     = 0;
    Child[virtualNumChildren].badPlaceTeam[0] = 0;
    Child[virtualNumChildren].badPlaceTeam[1] = 0;
    virtualNumChildren++;
    numChildren++;
}

void PathNode::ConnectChild(int i)
{
    int       j;
    pathway_t child = Child[i];

    for (j = i - 1; j >= numChildren; j--) {
        Child[j + 1] = Child[j];
    }

    Child[numChildren] = child;
    numChildren++;
}

void PathNode::DisconnectChild(int i)
{
    int       j;
    pathway_t child = Child[i];

    for (j = i + 1; j < numChildren; j++) {
        Child[j - 1] = Child[j];
    }

    numChildren--;
    Child[numChildren] = child;
}

qboolean PathNode::IsTouching(Entity *e1)
{
    return e1->absmin[0] <= origin[0] + 15.5f && e1->absmin[1] <= origin[1] + 15.5f
        && e1->absmin[0] <= origin[2] + 94.0f && origin[0] - 15.5f <= e1->absmax[0]
        && origin[1] - 15.5f <= e1->absmax[1] && origin[2] + 0.0f <= e1->absmax[2];
}

void PathNode::IsTouching(Event *ev)
{
    Entity *ent = ev->GetEntity(1);

    if (!ent) {
        ScriptError("IsTouching used with a NULL entity.\n");
    }

    ev->AddInteger(IsTouching(ev->GetEntity(1)));
}

void PathNode::setOriginEvent(Vector org)
{
    if (!PathManager.m_bNodesloaded) {
        origin   = org;
        centroid = org;
    }
}

void PathNode::DrawConnections(void)
{
    int        i;
    pathway_t *path;
    PathNode  *node;

    for (i = 0; i < numChildren; i++) {
        path = &Child[i];
        node = PathSearch::pathnodes[path->node];
        if (!node) {
            continue;
        }

        G_DebugLine(origin + Vector("0 0 24"), node->origin + Vector("0 0 24"), 0.7f, 0.7f, 0, 1);
    }
}

static void droptofloor(Vector& vec, PathNode *node)
{
    Vector  start, end;
    trace_t trace;

    start = vec;
    start[2] += 36;
    end = start;
    end[2] -= 2048;

    trace = G_Trace(start, PLAYER_BASE_MIN, PLAYER_BASE_MAX, end, NULL, MASK_PATHSOLID, qfalse, "droptofloor");
    vec.z = trace.endpos[2];
}

static bool IsValidPathnode(int spawnflags)
{
    if ((spawnflags & AI_DUCK)
        && (spawnflags & (AI_CONCEALMENT | AI_CORNER_LEFT | AI_CORNER_RIGHT | AI_SNIPER | AI_CRATE))) {
        return false;
    }

    if ((spawnflags & AI_CONCEALMENT) && (spawnflags & (AI_COVER | AI_CORNER_LEFT | AI_CORNER_RIGHT | AI_SNIPER))) {
        return false;
    }

    if ((spawnflags & AI_CORNER_LEFT)
        && (spawnflags & (AI_DUCK | AI_CONCEALMENT | AI_CORNER_RIGHT | AI_SNIPER | AI_CRATE))) {
        return false;
    }

    if ((spawnflags & AI_CORNER_RIGHT)
        && (spawnflags & (AI_DUCK | AI_CONCEALMENT | AI_CORNER_LEFT | AI_SNIPER | AI_CRATE))) {
        return false;
    }

    if ((spawnflags & AI_SNIPER)
        && (spawnflags & (AI_DUCK | AI_COVER | AI_CONCEALMENT | AI_CORNER_LEFT | AI_CORNER_RIGHT | AI_CRATE))) {
        return false;
    }

    if ((spawnflags & AI_CRATE)
        && (spawnflags & (AI_DUCK | AI_CONCEALMENT | AI_CORNER_LEFT | AI_CORNER_RIGHT | AI_SNIPER))) {
        return false;
    }

    return true;
}

static void GetPathnodeColor(int spawnflags, vec3_t color)
{
    if (!IsValidPathnode(spawnflags)) {
        VectorCopy(COLOR_PATHNODE_ERROR, color);
        return;
    }

    if (spawnflags & AI_CORNER_LEFT) {
        VectorCopy(COLOR_PATHNODE_CORNER_LEFT, color);
    } else if (spawnflags & AI_CORNER_RIGHT) {
        VectorCopy(COLOR_PATHNODE_CORNER_RIGHT, color);
    } else if (spawnflags & AI_DUCK) {
        VectorCopy(COLOR_PATHNODE_DUCK, color);
    } else if (spawnflags & AI_SNIPER) {
        VectorCopy(COLOR_PATHNODE_SNIPER, color);
    } else if (spawnflags & AI_CONCEALMENT) {
        VectorCopy(COLOR_PATHNODE_CONCEALMENT, color);
    } else if (spawnflags & AI_COVER) {
        VectorCopy(COLOR_PATHNODE_COVER, color);
    } else if (spawnflags & AI_CRATE) {
        VectorCopy(COLOR_PATHNODE_CRATE, color);
    } else {
        VectorCopy(COLOR_PATHNODE_DEFAULT, color);
    }
}

static int GetNodeTypeFromName(const char *name)
{
    if (!str::icmp(name, "corner_left")) {
        return AI_CORNER_LEFT;
    } else if (!str::icmp(name, "corner_right")) {
        return AI_CORNER_RIGHT;
    } else if (!str::icmp(name, "duck")) {
        return AI_DUCK;
    } else if (!str::icmp(name, "sniper")) {
        return AI_SNIPER;
    } else if (!str::icmp(name, "concealment")) {
        return AI_CONCEALMENT;
    } else if (!str::icmp(name, "cover")) {
        return AI_COVER;
    } else if (!str::icmp(name, "crate")) {
        return AI_CRATE;
    } else if (!str::icmp(name, "none")) {
        return 0;
    }

    return -1;
}

void DrawNode(int iNodeCount)
{
    Vector    down;
    Vector    up;
    Vector    dir;
    Vector    p1;
    Vector    p2;
    Vector    p3;
    Vector    p4;
    Vector    q1;
    Vector    q2;
    Vector    q3;
    Vector    q4;
    Vector    playerorigin;
    Vector    aStart;
    Vector    aEnd;
    PathNode *node;
    PathNode *nodelist[4096];
    Vector    end;
    Vector    start;
    Vector    p;
    vec3_t    color;

    playerorigin = g_entities[0].client->ps.origin;

    if (iNodeCount > 4096) {
        iNodeCount = 4096;
    }

    if (ai_showallnode->integer) {
        iNodeCount = PathSearch::DebugNearestNodeList2(playerorigin, nodelist, iNodeCount);
    } else {
        iNodeCount = PathSearch::DebugNearestNodeList(playerorigin, nodelist, iNodeCount);
    }

    if (iNodeCount) {
        for (int i = 0; i < iNodeCount; i++) {
            node = nodelist[i];
            GetPathnodeColor(node->nodeflags, color);

            p1.x = PLAYER_BASE_MAX.x + node->origin.x;
            p1.y = PLAYER_BASE_MAX.y + node->origin.y;

            p2.x = PLAYER_BASE_MAX.x + node->origin.x;
            p2.y = PLAYER_BASE_MIN.y + node->origin.y;

            p3.x = PLAYER_BASE_MIN.x + node->origin.x;
            p3.y = PLAYER_BASE_MIN.y + node->origin.y;

            p4.x = PLAYER_BASE_MIN.x + node->origin.x;
            p4.y = PLAYER_BASE_MAX.y + node->origin.y;

            start = node->origin + Vector(0, 0, 18);
            end   = node->origin + Vector(0, 0, 18);

            aStart  = start;
            aEnd[0] = node->origin[0] + cos(M_PI / 180.0f * node->angles[1]) * 16.0f;
            aEnd[1] = node->origin[1] + sin(M_PI / 180.0f * node->angles[1]) * 16.0f;
            aEnd[2] = end[2];

            G_DebugLine(aStart, aEnd, 1, 1, 1, 1);

            p1.z = node->origin.z + 36.0f;
            p2.z = node->origin.z + 36.0f;
            p3.z = node->origin.z + 36.0f;
            p4.z = node->origin.z + 36.0f;

            G_DebugLine(p1, p2, color[0], color[1], color[2], 1);
            G_DebugLine(p2, p3, color[0], color[1], color[2], 1);
            G_DebugLine(p3, p4, color[0], color[1], color[2], 1);
            G_DebugLine(p4, p1, color[0], color[1], color[2], 1);

            q1 = p1;
            q2 = p2;
            q3 = p3;
            q4 = p4;

            q1.z = node->origin.z;
            q2.z = node->origin.z;
            q3.z = node->origin.z;
            q4.z = node->origin.z;

            G_DebugLine(q1, q2, color[0], color[1], color[2], 1);
            G_DebugLine(q2, q3, color[0], color[1], color[2], 1);
            G_DebugLine(q3, q4, color[0], color[1], color[2], 1);
            G_DebugLine(q4, q1, color[0], color[1], color[2], 1);

            G_DebugLine(p1, q1, color[0], color[1], color[2], 1);
            G_DebugLine(p2, q2, color[0], color[1], color[2], 1);
            G_DebugLine(p3, q3, color[0], color[1], color[2], 1);
            G_DebugLine(p4, q4, color[0], color[1], color[2], 1);
        }
    } else {
        G_DebugCircle(playerorigin + Vector(0, 0, 48), 128, 1, 0, 0, 1, true);
    }
}

void DrawAllConnections(void)
{
    pathway_t *path;
    pathway_t *path2;
    PathNode  *node;
    PathNode  *to;
    Vector     down;
    Vector     up;
    Vector     dir;
    Vector     p1;
    Vector     p2;
    Vector     p3;
    Vector     p4;
    Vector     playerorigin;
    qboolean   showroutes;
    qboolean   shownums;
    bool       reverse;

    showroutes = (ai_showroutes->integer != 0);
    shownums   = (ai_shownodenums->integer != 0);

    // Figure out where the camera is

    if (!g_entities[0].client) {
        return;
    }

    playerorigin.x = g_entities[0].client->ps.origin[0];
    playerorigin.y = g_entities[0].client->ps.origin[1];
    playerorigin.z = g_entities[0].client->ps.origin[2];

    playerorigin[2] += g_entities[0].client->ps.viewheight;

    for (int i = 0; i < PathSearch::nodecount; i++) {
        node = PathSearch::pathnodes[i];
        if (!node) {
            continue;
        }

        if (Vector(node->origin - playerorigin).length() > ai_showroutes_distance->integer) {
            continue;
        }

        if (shownums) {
            G_DrawDebugNumber(node->origin + Vector(0, 0, 14), node->nodenum, 1.5, 1, 1, 0);
        }

        for (int j = 0; j < node->numChildren; j++) {
            path = &node->Child[j];

            if (path->fallheight > ai_fallheight->integer) {
                continue;
            }

            reverse = false;

            to = PathSearch::pathnodes[path->node];
            if (!to) {
                continue;
            }

            for (int k = to->numChildren - 1; k >= 0; k--) {
                path2 = &to->Child[k];

                if (path2->fallheight < ai_fallheight->integer && PathSearch::pathnodes[path2->node] == node) {
                    reverse = true;
                    break;
                }
            }

            p1 = path->pos1 + Vector(0, 0, 36);
            p2 = path->pos2 + Vector(0, 0, 36);

            if (node->nodenum < to->nodenum || !reverse) {
                // draw connected lines in green
                G_DebugLine(p1, p2, 0, 1, 0, 1);

                if (!reverse) {
                    dir   = Vector(path->pos2) - Vector(path->pos1);
                    dir.z = 0;
                    VectorNormalize(dir);

                    p3 = dir * 8.0f + dir * 8.0f;
                    p4 = dir * 8.0f;
                    p4.z += 8.0f;

                    G_DebugLine(p1 + p3 + up, p1 + p3 + up - p4, 1, 0, 0, 1);

                    p4.z -= 16.0f;

                    G_DebugLine(p1 + p3 + down, p1 + p3 + down - p4, 1, 0, 0, 1);
                }
            }
        }

        if (!node->numChildren) {
            // Put a little X where the node is to show that it had no connections
            p1 = node->origin;
            p1.z += 2;

            if (node->nodeflags & PATH_DONT_LINK) {
                G_DebugCircle(p1, 12, 0, 0, 1, 1, true);
            } else {
                p2 = Vector(12, 12, 0);
                G_DebugLine(p1 - p2, p1 + p2, 1, 0, 0, 1);

                p2.x = -12;
                G_DebugLine(p1 - p2, p1 + p2, 1, 0, 0, 1);
            }
        }
    }
}

MapCell::MapCell()
{
    numnodes = 0;
    nodes    = NULL;
}

int MapCell::NumNodes(void)
{
    return numnodes;
}

/*                         All
                     work and no play
                 makes Jim a dull boy. All
               work and no  play makes Jim a
             dull boy. All  work and no  play
           makes Jim a dull boy. All work and no
         play makes Jim a dull  boy. All work and
        no play makes Jim a dull boy. All work and
       no play makes Jim a dull boy. All work and no
      play makes Jim a dull boy. All work and no play
     makes Jim a dull boy. All work and no play makes
    Jim a dull boy.  All work and no  play makes Jim a
   dull boy. All work and no play makes Jim a dull boy.
   All work and no play makes  Jim a dull boy. All work
  and no play makes Jim a dull boy. All work and no play
  makes Jim a dull boy. All work and no play makes Jim a
 dull boy. All work and no play makes Jim a dull boy. All
 work and no play makes  Jim a dull boy. All  work and no
 play makes Jim a dull boy. All work and no play makes Jim
 a dull boy. All work  and no play makes Jim  a dull boy.
 All work and no play makes Jim  a dull boy. All work and
 no play makes Jim a dull boy. All work and no play makes
 Jim a dull boy.  All work and no  play makes Jim a  dull
 boy. All work and no play makes Jim a dull boy. All work
 and no play makes Jim  a dull boy. All work  and no play
 makes Jim a dull boy.  All work and no play  makes Jim a
 dull boy. All work and no play makes Jim a dull boy. All
  work and no play makes Jim a dull boy. All work and no
  play makes Jim a dull boy.  All work and no play makes
   Jim a dull boy. All work and no play makes Jim a dull
   boy. All work and no play  makes Jim a dull boy. All
    work and no play makes Jim a dull boy. All work and
     no play makes  Jim a dull  boy. All work  and no
      play makes Jim a dull boy. All work and no play
       makes Jim a dull  boy. All work and  no play
        makes Jim a dull boy. All work and no play
         makes Jim a  dull boy. All  work and no
           play makes Jim a  dull boy. All work
             and no play makes Jim a dull boy.
               All work  and no  play makes
                 Jim a dull boy. All work
                     and no play makes
                          Jim  a
*/

CLASS_DECLARATION(Class, PathSearch, NULL) {
    {NULL, NULL}
};

PathSearch::PathSearch()
{
    memset(pathnodes, 0, sizeof(pathnodes));
    open      = 0;
    findFrame = 0;
}

PathSearch::~PathSearch()
{
    ResetNodes();
}

void PathSearch::LoadAddToGrid(int x, int y)
{
    MapCell *cell;

    cell = GetNodesInCell(x, y);
    if (cell) {
        cell->numnodes++;
    }
}

int PathSearch::NodeCoordinate(float coord)
{
    float c;

    //return ( ( int )coord + MAX_MAP_BOUNDS - ( PATHMAP_CELLSIZE / 2 ) ) / PATHMAP_CELLSIZE;

    c = coord + MAX_MAP_BOUNDS - (PATHMAP_CELLSIZE / 2);

    if (c < 0) {
        c = coord + MAX_MAP_BOUNDS + (PATHMAP_CELLSIZE / 2) - 1;
    }

    return (int)c >> 8;
}

int PathSearch::GridCoordinate(float coord)
{
    float c;

    //return ( ( int )coord + MAX_MAP_BOUNDS ) / PATHMAP_CELLSIZE;

    c = coord + MAX_MAP_BOUNDS;

    if (c < 0) {
        c = coord + MAX_MAP_BOUNDS + PATHMAP_CELLSIZE - 1;
    }

    return (int)c >> 8;
}

MapCell *PathSearch::GetNodesInCell(int x, int y)
{
    if ((x < 0) || (x >= PATHMAP_GRIDSIZE) || (y < 0) || (y >= PATHMAP_GRIDSIZE)) {
        return NULL;
    }

    return &PathMap[x][y];
}

MapCell *PathSearch::GetNodesInCell(const vec3_t pos)
{
    int x;
    int y;

    x = GridCoordinate(pos[0]);
    y = GridCoordinate(pos[1]);

    return GetNodesInCell(x, y);
}

int PathSearch::DebugNearestNodeList(const vec3_t pos, PathNode **nodelist, int iMaxNodes)
{
    PathNode *node;
    int       i;
    MapCell  *cell;
    int       nodes[128];
    vec3_t    deltas[128];
    vec3_t    start;
    vec3_t    end;

    cell = GetNodesInCell(pos);

    if (!cell) {
        return 0;
    }

    int node_count = NearestNodeSetup(pos, cell, nodes, deltas);
    int n          = 0;

    for (i = 0; i < node_count && n < iMaxNodes; i++) {
        node = pathnodes[cell->nodes[nodes[i]]];
        if (!node) {
            continue;
        }

        VectorCopy(pos, start);
        VectorCopy(pos, end);

        VectorAdd(end, deltas[i], end);

        Vector vStart = start;
        Vector vMins  = Vector(-15, -15, 0);
        Vector vMaxs  = Vector(15, 15, 62);
        Vector vEnd   = end;

        if (G_SightTrace(
                vStart,
                vMins,
                vMaxs,
                vEnd,
                (gentity_t *)NULL,
                (gentity_t *)NULL,
                MASK_PATHSOLID,
                qtrue,
                "PathSearch::DebugNearestNodeList"
            )) {
            nodelist[n] = node;
            n++;
        }
    }

    if (!n && node_count) {
        nodelist[0] = pathnodes[cell->nodes[nodes[0]]];
        return 1;
    } else {
        return n;
    }

    return 0;
}

int PathSearch::DebugNearestNodeList2(const vec3_t pos, PathNode **nodelist, int iMaxNodes)
{
    vec3_t       delta;
    PathNode    *node;
    float        dist;
    int          n = 0;
    int          i;
    int          j;
    static float node_dist[MAX_PATHNODES];
    int          node_count;

    node_count = nodecount;

    for (i = 0; i < node_count; i++) {
        node = pathnodes[i];
        if (!node) {
            continue;
        }

        if (pos[2] > node->origin[2] + 94.0f) {
            continue;
        }

        if (node->origin[2] > pos[2] + 94.0f) {
            continue;
        }

        delta[0] = node->origin[0] - pos[0];
        delta[1] = node->origin[1] - pos[1];
        delta[2] = node->origin[2] - pos[2];

        dist = VectorLengthSquared(delta);

        for (j = n; j > 0; j--) {
            if (dist >= node_dist[j - 1]) {
                break;
            }

            node_dist[j] = node_dist[j - 1];
            nodelist[j]  = nodelist[j - 1];
        }

        n++;
        nodelist[j]  = node;
        node_dist[j] = dist;
    }

    return n;
}

PathNode *PathSearch::DebugNearestStartNode(const vec3_t pos, Entity *ent)
{
    PathNode *node = NULL;
    int       i;
    MapCell  *cell;
    int       nodes[128];
    vec3_t    deltas[128];
    vec3_t    start;
    vec3_t    end;
    int       node_count;

    cell = GetNodesInCell(pos);

    if (!cell) {
        return NULL;
    }

    node_count = NearestNodeSetup(pos, cell, nodes, deltas);

    VectorCopy(pos, start);
    start[2] += 32.0f;

    for (i = 0; i < node_count; i++) {
        node = pathnodes[cell->nodes[nodes[i]]];
        if (!node) {
            continue;
        }

        VectorCopy(start, end);
        VectorAdd(end, deltas[nodes[i]], end);

        if (G_SightTrace(
                start,
                Vector(-15, -15, 0),
                Vector(15, 15, 62),
                end,
                ent,
                NULL,
                MASK_TARGETPATH,
                qtrue,
                "PathSearch::DebugNearestStartNode"
            )) {
            return node;
        }
    }

    if (node_count > 0) {
        return pathnodes[cell->nodes[nodes[0]]];
    }

    return NULL;
}

PathNode *PathSearch::NearestStartNode(const vec3_t pos, SimpleActor *ent)
{
    PathNode *node = NULL;
    int       i;
    MapCell  *cell;
    int       nodes[128];
    vec3_t    deltas[128];
    vec3_t    start;
    vec3_t    end;
    int       node_count;

    cell = GetNodesInCell(pos);

    if (!cell) {
        return NULL;
    }

    node_count = NearestNodeSetup(pos, cell, nodes, deltas);

    VectorCopy(pos, start);
    start[2] += 32.0f;

    for (i = 0; i < node_count; i++) {
        node = pathnodes[cell->nodes[nodes[i]]];
        if (!node) {
            continue;
        }

        VectorAdd(start, deltas[nodes[i]], end);

        if (G_SightTrace(
                start,
                Vector(-15, -15, 0),
                Vector(15, 15, 62),
                end,
                ent,
                NULL,
                MASK_PATHSOLID,
                qtrue,
                "PathSearch::NearestStartNode 1"
            )) {
            ent->m_NearestNode = node;
            VectorCopy(end, ent->m_vNearestNodePos);
            return node;
        }
    }

    if (ent->m_NearestNode) {
        if (G_SightTrace(
                start,
                Vector(-15, -15, 0),
                Vector(15, 15, 62),
                ent->m_vNearestNodePos,
                ent,
                NULL,
                MASK_TARGETPATH,
                qtrue,
                "PathSearch::NearestStartNode 2"
            )) {
            return ent->m_NearestNode;
        }
    }

    if (node_count > 0) {
        return pathnodes[cell->nodes[nodes[0]]];
    }

    return ent->m_NearestNode;
}

PathNode *PathSearch::NearestEndNode(const vec3_t pos)
{
    PathNode *node = NULL;
    int       i;
    MapCell  *cell;
    int       nodes[128];
    vec3_t    deltas[128];
    vec3_t    start;
    vec3_t    end;
    int       node_count;

    cell = GetNodesInCell(pos);

    if (!cell) {
        return NULL;
    }

    node_count = NearestNodeSetup(pos, cell, nodes, deltas);

    VectorCopy(pos, start);
    start[2] += 32.0f;

    for (i = 0; i < node_count; i++) {
        node = pathnodes[cell->nodes[nodes[i]]];
        if (!node) {
            continue;
        }

        VectorAdd(start, deltas[nodes[i]], end);

        if (G_SightTrace(
                start,
                Vector(-15, -15, 0),
                Vector(15, 15, 62),
                end,
                (Entity *)nullptr,
                (Entity *)nullptr,
                MASK_TARGETPATH,
                qtrue,
                "PathSearch::NearestEndNode"
            )) {
            return node;
        }
    }

    return NULL;
}

void PathSearch::ShowNodes(void)
{
    if (g_entities->client) {
        if (ai_shownode->integer) {
            DrawNode(ai_shownode->integer);
        }
        if (ai_showroutes->integer || ai_shownodenums->integer) {
            DrawAllConnections();
        }
    }

    if (ai_showpath->integer) {
        if (!test_path) {
            test_path = new ActorPath;
        }

        if (ai_showpath->integer == 1) {
            ai_startpath = g_entities[0].entity->origin;
        }
        if (ai_showpath->integer == 2) {
            ai_endpath = g_entities[0].entity->origin;
        }
        if (ai_showpath->integer <= 2) {
            test_path->SetFallHeight(ai_fallheight->integer);
            test_path->FindPath(ai_startpath, ai_endpath, NULL, 0, NULL, 0);
        }
        if (ai_showpath->integer == 3) {
            if (test_path->CurrentNode()) {
                test_path->UpdatePos(g_entities[0].entity->origin);

                Vector vStart = g_entities[0].entity->origin + Vector(0, 0, 32);
                Vector vEnd   = g_entities[0].entity->origin + test_path->CurrentDelta() + Vector(0, 0, 32);

                G_DebugLine(vStart, vEnd, 1, 1, 0, 1);
            }
        }

        G_DebugLine(ai_startpath, ai_endpath, 0, 0, 1, 1);

        if (test_path->CurrentNode()) {
            PathInfo *pos = test_path->CurrentNode();

            while (pos != test_path->LastNode()) {
                Vector vStart = pos->point + Vector(0, 0, 32);

                pos--;

                Vector vEnd = pos->point + Vector(0, 0, 32);

                G_DebugLine(vStart, vEnd, 1, 0, 0, 1);
            }
        }
    }

    navMaster.Frame();
}

qboolean PathSearch::ArchiveSaveNodes(void)
{
    Archiver arc;
    str      maptime;
    int      tempInt;

    if (!arc.Create(level.m_pathfile)) {
        return qfalse;
    }

    tempInt = PATHFILE_VERSION;
    arc.ArchiveInteger(&tempInt);

    maptime = gi.MapTime();
    arc.ArchiveString(&maptime);

    arc.ArchiveInteger(&m_NodeCheckFailed);
    ArchiveStaticSave(arc);
    arc.Close();

    return true;
}

void PathSearch::ArchiveLoadNodes(void)
{
    Archiver arc;

    m_LoadIndex = 0;
    if (arc.Read(level.m_pathfile, false)) {
        int file_version;
        str maptime;

        // get file values
        arc.ArchiveInteger(&file_version);
        if (file_version != PATHFILE_VERSION) {
            Com_Printf("Expecting version %d path file.  Path file is version %d.\n", PATHFILE_VERSION, file_version);
            arc.Close();
            return;
        }

        arc.ArchiveString(&maptime);
        if (gi.MapTime() == maptime && gi.FS_FileNewer(level.m_mapfile.c_str(), level.m_pathfile.c_str()) <= 0) {
            arc.ArchiveInteger(&m_NodeCheckFailed);

            if (!g_nodecheck->integer || !m_NodeCheckFailed) {
                ArchiveStaticLoad(arc);
                m_bNodesloaded = arc.NoErrors();
            } else {
                gi.Printf("Rebuilding pathnodes to view node errors.\n");
            }
        } else {
            gi.Printf("Pathnodes have changed, rebuilding.\n");
        }
    }

    arc.Close();
}

void PathSearch::Init(void)
{
    ai_showroutes          = gi.Cvar_Get("ai_showroutes", "0", 0);
    ai_showroutes_distance = gi.Cvar_Get("ai_showroutes_distance", "1000", 0);
    ai_shownodenums        = gi.Cvar_Get("ai_shownodenums", "0", 0);
    ai_shownode            = gi.Cvar_Get("ai_shownode", "0", 0);
    ai_showallnode         = gi.Cvar_Get("ai_showallnode", "0", 0);
    ai_showpath            = gi.Cvar_Get("ai_showpath", "0", 0);
    ai_fallheight          = gi.Cvar_Get("ai_fallheight", "96", 0);
    ai_debugpath           = gi.Cvar_Get("ai_debugpath", "0", 0);
    ai_pathchecktime       = gi.Cvar_Get("ai_pathchecktime", "1.5", CVAR_CHEAT);
    ai_pathcheckdist       = gi.Cvar_Get("ai_pathcheckdist", "4096", CVAR_CHEAT);

    //
    // Added in OPM
    //
    ai_editmode = gi.Cvar_Get("ai_editmode", "0", CVAR_LATCH);

    navMaster.Init();
}

void *PathSearch::AllocPathNode(void)
{
    if (!bulkNavMemory) {
        return gi.Malloc(sizeof(PathNode));
    } else {
        bulkNavMemory -= sizeof(PathNode);
        if (ai_editmode->integer) {
            return gi.Malloc(sizeof(PathNode));
        }
    }

    return bulkNavMemory;
}

void PathSearch::FreePathNode(void *ptr)
{
    if (!bulkNavMemory || ai_editmode->integer) {
        gi.Free(ptr);
    }
}

void PathSearch::PlayerCover(Player *pPlayer)
{
    int       i;
    PathNode *node;
    Vector    delta;
    Entity   *pOwner;

    for (i = 0; i < nodecount; i++) {
        node = pathnodes[i];

        if (!node || !(node->nodeflags & AI_MOVE_MASK)) {
            continue;
        }

        pOwner = node->GetClaimHolder();

        delta = node->origin - pPlayer->origin;

        // Check if we need to cover
        if (VectorLengthSquared(delta) > Square(48)) {
            if (pOwner == pPlayer) {
                node->Relinquish();
            }
            continue;
        }

        if (pOwner != pPlayer) {
            if (pOwner) {
                pOwner->PathnodeClaimRevoked(node);
            }
            // Player claim the node
            node->Claim(pPlayer);
        }
    }
}

class nodeinfo
{
public:
    PathNode *pNode;
    float     fDistSquared;
};

int node_compare(const void *pe1, const void *pe2)
{
    nodeinfo *Pe1 = (nodeinfo *)pe1;
    nodeinfo *Pe2 = (nodeinfo *)pe2;
    int       iConcealment;

    iConcealment = (Pe1->pNode->nodeflags & AI_CONCEALMENT_MASK) ? 1 : 0;
    if (Pe2->pNode->nodeflags & AI_CONCEALMENT_MASK) {
        iConcealment--;
    }

    return (
        *(unsigned int*)&Pe1->fDistSquared + (iConcealment << 23)
        + (((Pe1->pNode->nodeflags & AI_CONCEALMENT) - (Pe2->pNode->nodeflags & AI_CONCEALMENT)) << 21)
        - *(unsigned int*)&Pe2->fDistSquared
    );
}

int PathSearch::FindPotentialCover(
    Entity *pEnt, Vector& vPos, Entity *pEnemy, PathNode **ppFoundNodes, int iMaxFind
)
{
    nodeinfo  nodes[MAX_PATHNODES];
    int       nNodes = 0;
    int       i;
    Vector    delta;
    PathNode *node;

    Actor *pActor = static_cast<Actor *>(pEnt);

    for (i = 0; i < nodecount; i++) {
        node = pathnodes[i];
        if (!node) {
            continue;
        }

        if (!(node->nodeflags & AI_COVER_MASK)) {
            continue;
        }

        if (node->IsClaimedByOther(static_cast<Entity *>(pEnt))) {
            continue;
        }

        delta = node->origin - pActor->m_vHome;
        if (delta.lengthSquared() > pActor->m_fLeashSquared) {
            continue;
        }

        delta = node->origin - pEnemy->origin;
        if (delta.lengthSquared() < pActor->m_fMinDistanceSquared
            || delta.lengthSquared() > pActor->m_fMaxDistanceSquared) {
            continue;
        }

        delta                      = node->origin - pEnt->origin;
        nodes[nNodes].pNode        = node;
        nodes[nNodes].fDistSquared = delta.lengthSquared();
        nNodes++;
    }

    if (nNodes) {
        qsort(nodes, nNodes, sizeof(nodeinfo), node_compare);

        if (nNodes > iMaxFind) {
            nNodes = iMaxFind;
        }

        for (i = 0; i < nNodes; i++) {
            ppFoundNodes[nNodes - i - 1] = nodes[i].pNode;
        }
    }
    return nNodes;
}

PathNode *PathSearch::FindNearestSniperNode(Entity*pEnt, Vector& vPos, Entity *pEnemy)
{
    Actor    *pSelf = (Actor *)pEnt;
    PathNode *pNode;
    Vector    delta;
    int       nNodes = 0;
    int       i;
    nodeinfo  nodes[MAX_PATHNODES];

    for (i = 0; i < nodecount; i++) {
        pNode = pathnodes[i];
        if (!pNode) {
            continue;
        }

        if (!(pNode->nodeflags & AI_SNIPER)) {
            continue;
        }
        if (pNode->IsClaimedByOther(pEnt)) {
            continue;
        }

        delta = pNode->origin - pSelf->m_vHome;

        if (delta.lengthSquared() > pSelf->m_fLeashSquared) {
            continue;
        }

        delta = pNode->origin - pEnemy->origin;
        if (delta.lengthSquared() < pSelf->m_fMinDistanceSquared
            || delta.lengthSquared() > pSelf->m_fMaxDistanceSquared) {
            continue;
        }

        delta                      = pNode->origin - pSelf->origin;
        nodes[nNodes].fDistSquared = delta.lengthSquared();
        nodes[nNodes].pNode        = pNode;
        nNodes++;
    }

    if (nNodes == 0) {
        return NULL;
    }

    qsort(nodes, nNodes, sizeof(nodeinfo), node_compare);

    if (nNodes <= 0) {
        return NULL;
    }

    for (i = 0; i < nNodes; i++) {
        pNode = nodes[i].pNode;
        if (pSelf->CanSeeFrom(pSelf->eyeposition + pNode->origin, pEnemy)) {
            return pNode;
        }

        pNode->MarkTemporarilyBad();
    }

    return NULL;
}

PathNode *PathSearch::GetSpawnNode(ClassDef *cls)
{
    if (m_bNodesloaded
        // Fixed in OPM
        && m_LoadIndex < nodecount
        ) {
        return pathnodes[m_LoadIndex++];
    } else {
        // Otherwise create a new node
        return static_cast<PathNode *>(cls->newInstance());
    }
}

void PathSearch::LoadNodes(void)
{
    Init();

    ArchiveLoadNodes();
}

void PathSearch::CreatePaths(void)
{
    int        i;
    int        j;
    int        x;
    int        y;
    PathNode  *node;
    Vector     start;
    Vector     end;
    gentity_t *ent;
    int        t1;
    int        t2;

    if (m_bNodesloaded) {
        return;
    }

    if (!nodecount) {
        m_bNodesloaded = true;
        return;
    }

    m_NodeCheckFailed = false;

    gi.DPrintf(
        "***********************************\n"
        "***********************************\n"
        "\n"
        "Creating paths...\n"
        "\n"
        "***********************************\n"
        "***********************************\n"
    );

    gi.ClearResource();

    t1 = gi.Milliseconds();

    for (i = 0, ent = g_entities; i < game.maxentities; i++, ent++) {
        if (ent->entity && ent->entity->IsSubclassOfDoor()) {
            ent->entity->unlink();
        }
    }

    for (x = 0; x < PATHMAP_GRIDSIZE; x++) {
        for (y = 0; y < PATHMAP_GRIDSIZE; y++) {
            MapCell *cell = &PathMap[x][y];

            cell->nodes    = (short *)gi.Malloc(PATHMAP_CELLSIZE);
            cell->numnodes = 0;
            memset(cell->nodes, 0, PATHMAP_CELLSIZE);
        }
    }

    for (i = 0; i < nodecount; i++) {
        node = pathnodes[i];
        if (!node) {
            continue;
        }

        droptofloor(node->origin, node);
        node->centroid = node->origin;

        if (node->nodeflags & PATH_DONT_LINK) {
            continue;
        }

        for (j = i - 1; j >= 0; j--) {
            PathNode *node2 = pathnodes[j];
            if (!node2) {
                continue;
            }

            if (node->origin == node2->origin) {
                Com_Printf(
                    "^~^~^ Duplicate node at (%.2f %.2f %.2f) not linked\n",
                    node->origin[0],
                    node->origin[1],
                    node->origin[2]
                );
                node->nodeflags |= PATH_DONT_LINK;
                break;
            }
        }

        if (!(node->nodeflags & PATH_DONT_LINK)) {
            node->Child = (pathway_t *)gi.Malloc(sizeof(pathway_t) * PATHMAP_NODES);
        }
    }

    for (i = 0; i < nodecount; i++) {
        node = pathnodes[i];

        if (!node || (node->nodeflags & PATH_DONT_LINK)) {
            continue;
        }

        AddNode(node);
    }

    for (i = 0; i < nodecount; i++) {
        node = pathnodes[i];

        if (!node || (node->nodeflags & PATH_DONT_LINK)) {
            continue;
        }

        Connect(node);
    }

    for (i = 0, ent = g_entities; i < game.maxentities; i++, ent++) {
        if (ent->entity && ent->entity->IsSubclassOfDoor()) {
            ent->entity->link();
        }
    }

    gi.DPrintf("\nSaving path nodes to '%s'\n", level.m_pathfile.c_str());
    Com_Printf("Archiving\n");
    ArchiveSaveNodes();
    m_bNodesloaded = true;
    Com_Printf("done.\n");

    t2 = gi.Milliseconds();
    Com_Printf("Path connection: %5.2f seconds\n", (t2 - t1) / 1000.0f);
    Com_Printf("Number of nodes: %d\n", nodecount);
    gi.ClearResource();

    if (g_nodecheck->integer && m_NodeCheckFailed) {
        gi.Error(ERR_DROP, "Node check failed");
    }
}

void PathSearch::LoadAddToGrid2(PathNode *node, int x, int y)
{
    MapCell *cell;

    cell = GetNodesInCell(x, y);

    if (cell) {
        cell->AddNode(node);
    }
}

void PathSearch::ArchiveStaticLoad(Archiver& arc)
{
    int       i;
    PathNode *node;
    int       total_nodes;
    int       total_children;
    int       x;
    int       y;
    int       size;

    loadingarchive = true;

    arc.ArchiveInteger(&nodecount);
    arc.ArchiveInteger(&total_nodes);
    arc.ArchiveInteger(&total_children);

    size = total_nodes + total_children * (sizeof(pathway_t) * 2) + nodecount * (sizeof(PathNode) / 2);
    size *= sizeof(void *) / 2;

    gi.DPrintf("%d memory allocated for navigation.\n", size);

    if (size) {
        startBulkNavMemory = (byte *)gi.Malloc(size);
    } else {
        startBulkNavMemory = NULL;
    }

    bulkNavMemory = startBulkNavMemory + size;

    for (i = 0; i < nodecount; i++) {
        node = new PathNode;

        arc.ArchiveObjectPosition(node);
        node->ArchiveStatic(arc);
        node->nodenum = i;

        pathnodes[i] = node;

        if (!(node->nodeflags & PATH_DONT_LINK)) {
            x = NodeCoordinate(node->origin[0]);
            y = NodeCoordinate(node->origin[1]);

            LoadAddToGrid(x, y);
            LoadAddToGrid(x + 1, y);
            LoadAddToGrid(x, y + 1);
            LoadAddToGrid(x + 1, y + 1);
        }
    }

    for (x = 0; x < PATHMAP_GRIDSIZE; x++) {
        for (y = 0; y < PATHMAP_GRIDSIZE; y++) {
            bulkNavMemory -= PathMap[x][y].numnodes * sizeof(short);

            PathMap[x][y].nodes    = PathMap[x][y].numnodes ? (short *)bulkNavMemory : NULL;
            PathMap[x][y].numnodes = 0;
        }
    }

    for (i = 0; i < nodecount; i++) {
        node = pathnodes[i];

        if (!(node->nodeflags & PATH_DONT_LINK)) {
            x = NodeCoordinate(node->origin[0]);
            y = NodeCoordinate(node->origin[1]);

            LoadAddToGrid2(node, x, y);
            LoadAddToGrid2(node, x + 1, y);
            LoadAddToGrid2(node, x, y + 1);
            LoadAddToGrid2(node, x + 1, y + 1);
        }
    }

    loadingarchive = false;
}

void PathSearch::ArchiveStaticSave(Archiver& arc)
{
    int       i;
    PathNode *node;
    int       total_nodes    = 0;
    int       total_children = 0;
    int       x              = 0;
    int       y              = 0;

    for (x = 0; x < PATHMAP_GRIDSIZE; x++) {
        for (y = 0; y < PATHMAP_GRIDSIZE; y++) {
            total_nodes += PathMap[x][y].NumNodes();
        }
    }

    for (i = 0; i < nodecount; i++) {
        node = pathnodes[i];
        total_children += node->virtualNumChildren;
    }

    arc.ArchiveInteger(&nodecount);
    arc.ArchiveInteger(&total_nodes);
    arc.ArchiveInteger(&total_children);

    for (i = 0; i < nodecount; i++) {
        node = pathnodes[i];
        arc.ArchiveObjectPosition(node);
        node->ArchiveStatic(arc);
    }
}

bool PathSearch::ArchiveDynamic(Archiver& arc)
{
    PathNode *node;
    int       i;
    int       count;

    if (arc.Saving()) {
        arc.ArchiveInteger(&nodecount);
    } else {
        arc.ArchiveInteger(&count);
        if (count != nodecount) {
            Com_Printf("Path file invalid - cannot load save game\n");
            return false;
        }
    }

    for (i = 0; i < nodecount; i++) {
        node = PathSearch::pathnodes[i];
        node->ArchiveDynamic(arc);
    }

    return true;
}

void PathSearch::AddToGrid(PathNode *node, int x, int y)
{
    MapCell *cell;

    cell = GetNodesInCell(x, y);

    if (!cell) {
        return;
    }

    if (cell->NumNodes() >= PATHMAP_NODES) {
        Com_Printf("^~^~^ PathSearch::AddToGrid: Node overflow at ( %d, %d )\n", x, y);
        return;
    }

    cell->AddNode(node);
}

bool PathSearch::Connect(PathNode *node, int x, int y)
{
    MapCell  *cell;
    int       i;
    PathNode *node2;

    cell = GetNodesInCell(x, y);

    if (!cell) {
        return true;
    }

    for (i = 0; i < cell->numnodes; i++) {
        node2 = pathnodes[cell->nodes[i]];
        if (!node2) {
            continue;
        }

        if (node2->findCount != findFrame) {
            node2->findCount = findFrame;

            if (!node->CheckPathTo(node2)) {
                return false;
            }
        }
    }

    return true;
}

bool PathNode::CheckPathTo(PathNode *node)
{
    if (virtualNumChildren >= NUM_PATHSPERNODE) {
        Com_Printf(
            "^~^~^ %d paths per node at (%.2f %.2f %.2f) exceeded\n - use DONT_LINK on some nodes to conserve cpu and "
            "memory usage\n",
            NUM_PATHSPERNODE,
            node->origin[0],
            node->origin[1],
            node->origin[2]
        );
        PathSearch::m_NodeCheckFailed = true;
        return false;
    }

    CheckPathToDefault(node, &Child[virtualNumChildren]);
    return true;
}

qboolean CheckMove(Vector& origin, Vector& pos, short int *path_fallheight, float size)
{
    mmove_t mm;
    int     i;
    float   air_z;
    float   fallheight;
    float   test_fallheight;
    float   error;
    trace_t trace;
    vec3_t  dir;
    vec3_t  end;

    memset(&mm, 0, sizeof(mmove_t));

    VectorClear(mm.velocity);
    VectorCopy(origin, mm.origin);
    mm.desired_speed  = 150.0f;
    mm.entityNum      = ENTITYNUM_NONE;
    mm.tracemask      = MASK_PATHSOLID;
    mm.frametime      = 0.1f;
    mm.desired_dir[0] = pos[0] - origin[0];
    mm.desired_dir[1] = pos[1] - origin[1];
    VectorNormalize2D(mm.desired_dir);

    mm.groundPlane = qfalse;
    mm.walking     = qfalse;

    mm.mins[0] = -size;
    mm.mins[1] = -size;
    mm.mins[2] = 0;
    mm.maxs[0] = size;
    mm.maxs[1] = size;
    mm.maxs[2] = MAXS_Z;

    testcount  = 0;
    fallheight = 0.0f;
    air_z      = mm.origin[2];

    for (i = 0; i < 200; i++) {
        testpos[testcount++] = mm.origin;

        MmoveSingle(&mm);

        if (mm.groundPlane) {
            test_fallheight = air_z - mm.origin[2];

            if (test_fallheight > fallheight) {
                if (test_fallheight > MAXS_Z) {
                    Vector start;

                    //
                    // Added in OPM
                    //  Check to make sure that the target is reachable enough
                    //  so that AIs don't block trying to move to the target position

                    start = origin;
                    start.z += MAXS_Z;

                    if (!G_SightTrace(
                            start,
                            vec_zero,
                            vec_zero,
                            pos,
                            (gentity_t *)NULL,
                            (gentity_t *)NULL,
                            MASK_PATHSOLID,
                            qfalse,
                            "CheckMove"
                        )) {
                        return false;
                    }
                }

                if (test_fallheight > 1024.0f) {
                    return false;
                }

                fallheight = test_fallheight;
            }

            air_z = mm.origin[2];
        }

        VectorSub2D(pos, mm.origin, dir);

        if (DotProduct2D(dir, mm.desired_dir) <= 0.1f) {
            error = mm.origin[2] - pos[2];

            *path_fallheight = (short)fallheight;
            if (fabs(error) <= MAXS_Z) {
                if (error <= 0.0f || mm.groundPlane) {
                    return true;
                }

                end[0] = mm.origin[0];
                end[1] = mm.origin[1];
                end[2] = pos[2];

                trace = G_Trace(mm.origin, mm.mins, mm.maxs, end, NULL, MASK_PATHSOLID, true, "CheckMove");

                test_fallheight = mm.origin[2] - trace.endpos[2];

                if (test_fallheight <= 18.0f) {
                    *path_fallheight = (short)test_fallheight + fallheight;
                    return test_fallheight + fallheight <= 1024.0f;
                }
            }

            if (mm.groundPlane) {
                return false;
            }

            VectorCopy2D(dir, mm.desired_dir);
            VectorNormalize2D(mm.desired_dir);
        }

        if (mm.hit_obstacle) {
            break;
        }
    }

    return false;
}

void PathNode::CheckPathToDefault(PathNode *node, pathway_t *pathway)
{
    float  dist;
    vec2_t delta;
    Vector start;
    Vector end;

    VectorSub2D(node->origin, origin, delta);
    dist = VectorNormalize2D(delta);

    if (dist >= 384.0f) {
        return;
    }

    start = origin;
    end   = node->origin;

    droptofloor(start, this);
    droptofloor(end, node);

    if (CheckMove(start, end, &pathway->fallheight, 15.5f)) {
        pathway->dist = dist;
        VectorCopy2D(delta, pathway->dir);
        start.copyTo(pathway->pos1);
        end.copyTo(pathway->pos2);
        ConnectTo(node);
    }
}

void MapCell::AddNode(PathNode *node)
{
    nodes[numnodes] = node->nodenum;
    numnodes++;
}

void PathSearch::AddNode(PathNode *node)
{
    int x;
    int y;

    assert(node);

    x = NodeCoordinate(node->origin[0]);
    y = NodeCoordinate(node->origin[1]);

    AddToGrid(node, x, y);
    AddToGrid(node, x + 1, y);
    AddToGrid(node, x, y + 1);
    AddToGrid(node, x + 1, y + 1);
}

void PathSearch::Connect(PathNode *node)
{
    int x;
    int y;

    findFrame++;
    node->findCount = findFrame;

    x = GridCoordinate(node->origin[0]);
    y = GridCoordinate(node->origin[1]);

    if (!Connect(node, x - 1, y - 1)) {
        return;
    }

    if (!Connect(node, x - 1, y)) {
        return;
    }

    if (!Connect(node, x - 1, y + 1)) {
        return;
    }

    if (!Connect(node, x, y - 1)) {
        return;
    }

    if (!Connect(node, x, y)) {
        return;
    }

    if (!Connect(node, x, y + 1)) {
        return;
    }

    if (!Connect(node, x + 1, y - 1)) {
        return;
    }

    if (!Connect(node, x + 1, y)) {
        return;
    }

    Connect(node, x + 1, y + 1);
}

const_str PathNode::GetSpecialAttack(Actor *pActor)
{
    int       iSpecialAttack;
    const_str csAnimation;
    float     fRangeSquared;
    vec2_t    vDelta;
    float     fAngle;
    float     fMinRangeSquared;
    float     fMaxRangeSquared;
    float     fMinAngle;
    float     fMaxAngle;

    if (nodeflags & AI_CORNER_LEFT) {
        iSpecialAttack   = 0;
        csAnimation      = STRING_ANIM_CORNERLEFT_SCR;
        fMinRangeSquared = g_AttackParms[iSpecialAttack].fMinRangeSquared;
        fMaxRangeSquared = g_AttackParms[iSpecialAttack].fMaxRangeSquared;
        fMinAngle        = g_AttackParms[iSpecialAttack].fMinAngle;
        fMaxAngle        = g_AttackParms[iSpecialAttack].fMaxAngle;
    } else if (nodeflags & AI_CORNER_RIGHT) {
        iSpecialAttack   = 1;
        csAnimation      = STRING_ANIM_CORNERRIGHT_SCR;
        fMinRangeSquared = g_AttackParms[iSpecialAttack].fMinRangeSquared;
        fMaxRangeSquared = g_AttackParms[iSpecialAttack].fMaxRangeSquared;
        fMinAngle        = g_AttackParms[iSpecialAttack].fMinAngle;
        fMaxAngle        = g_AttackParms[iSpecialAttack].fMaxAngle;
    } else if (nodeflags & AI_CRATE) {
        iSpecialAttack   = 2;
        csAnimation      = STRING_ANIM_OVERATTACK_SCR;
        fMinRangeSquared = g_AttackParms[iSpecialAttack].fMinRangeSquared;
        fMaxRangeSquared = g_AttackParms[iSpecialAttack].fMaxRangeSquared;
        fMinAngle        = g_AttackParms[iSpecialAttack].fMinAngle;
        fMaxAngle        = g_AttackParms[iSpecialAttack].fMaxAngle;
    } else if (nodeflags & AI_LOW_WALL_ARC) {
        if (nodeflags & AI_DUCK) {
            csAnimation = STRING_ANIM_LOWWALL_SCR;
        } else {
            csAnimation = STRING_ANIM_HIGHWALL_SCR;
        }

        iSpecialAttack   = 3;
        fMinRangeSquared = g_AttackParms[iSpecialAttack].fMinRangeSquared;
        fMaxRangeSquared = g_AttackParms[iSpecialAttack].fMaxRangeSquared;
        fMinAngle        = 360 - m_fLowWallArc;
        fMaxAngle        = m_fLowWallArc;
    } else {
        return STRING_NULL;
    }

    if (pActor->m_Enemy) {
        VectorSub2D(pActor->m_Enemy->centroid, origin, vDelta);
    } else {
        VectorSub2D(pActor->m_vLastEnemyPos, origin, vDelta);
    }

    fRangeSquared = VectorLength2DSquared(vDelta);

    if (fRangeSquared < fMinRangeSquared || fRangeSquared > fMaxRangeSquared) {
        return STRING_NULL;
    }

    fAngle = RAD2DEG(atan2(vDelta[1], vDelta[0])) - angles[1];

    if (fAngle <= -360) {
        fAngle = fAngle + 720.0f;
    } else if (fAngle < 0) {
        fAngle = fAngle + 360.0f;
    } else if (fAngle >= 720) {
        fAngle = fAngle - 720.0f;
    } else if (fAngle >= 360) {
        fAngle = fAngle - 360.0f;
    } else {
        fAngle = fAngle;
    }

    if (fMinAngle > fMaxAngle) {
        if (fAngle < fMinAngle && fAngle > fMaxAngle) {
            return STRING_NULL;
        }
    } else {
        if (fAngle < fMinAngle || fAngle > fMaxAngle) {
            return STRING_NULL;
        }
    }

    return csAnimation;
}

void PathNode::Claim(Entity *pClaimer)
{
    pLastClaimer   = pClaimer;
    iAvailableTime = 0;
}

Entity *PathNode::GetClaimHolder(void) const
{
    if (iAvailableTime) {
        return NULL;
    } else {
        return pLastClaimer;
    }
}

void PathNode::Relinquish(void)
{
    iAvailableTime = level.inttime + 4000;
}

bool PathNode::IsClaimedByOther(Entity *pPossibleClaimer) const
{
    if (pLastClaimer == pPossibleClaimer) {
        return false;
    }

    if (iAvailableTime) {
        return (level.inttime < iAvailableTime);
    } else {
        return (pLastClaimer != NULL);
    }
}

void PathNode::MarkTemporarilyBad(void)
{
    iAvailableTime = level.inttime + 5000;
    pLastClaimer   = NULL;
}

void PathNode::Archive(Archiver& arc) {}

void PathNode::ArchiveStatic(Archiver& arc)
{
    arc.ArchiveVector(&origin);
    arc.ArchiveVector(&centroid);
    arc.ArchiveInteger(&nodeflags);
    arc.ArchiveInteger(&virtualNumChildren);

    numChildren = virtualNumChildren;

    if (arc.Loading()) {
        bulkNavMemory -= virtualNumChildren * sizeof(pathway_t) * sizeof(pathway_t *);
        Child = virtualNumChildren ? (pathway_t *)bulkNavMemory : NULL;
    }

    for (int i = 0; i < virtualNumChildren; i++) {
        arc.ArchiveShort(&Child[i].node);
        arc.ArchiveShort(&Child[i].fallheight);
        arc.ArchiveFloat(&Child[i].dist);
        arc.ArchiveVec2(Child[i].dir);
        arc.ArchiveVec3(Child[i].pos1);
        arc.ArchiveVec3(Child[i].pos2);

        if (arc.Loading()) {
            Child[i].numBlockers = 0;

            for (int j = 0; j < ARRAY_LEN(Child[i].badPlaceTeam); j++) {
                Child[i].badPlaceTeam[j] = 0;
            }
        }
    }
}

void PathNode::ArchiveDynamic(Archiver& arc)
{
    SimpleEntity::SimpleArchive(arc);

    arc.ArchiveObjectPosition(this);
    arc.ArchiveSafePointer(&pLastClaimer);
    arc.ArchiveInteger(&iAvailableTime);
    arc.ArchiveInteger(&numChildren);

    if (numChildren != virtualNumChildren) {
        for (int i = 0; i < virtualNumChildren; i++) {
            arc.ArchiveByte(&Child[i].numBlockers);
            arc.ArchiveShort(&Child[i].node);
            arc.ArchiveShort(&Child[i].fallheight);
            arc.ArchiveFloat(&Child[i].dist);
            arc.ArchiveVec2(Child[i].dir);
            arc.ArchiveVec3(Child[i].pos1);
            arc.ArchiveVec3(Child[i].pos2);
        }
    }
}

int PathSearch::NearestNodeSetup(const vec3_t pos, MapCell *cell, int *nodes, vec3_t *deltas)
{
    vec3_t    delta;
    PathNode *node;
    float     dist;
    int       n = 0;
    int       i;
    int       j;
    float     node_dist[128];
    int       node_count;

    node_count = cell->numnodes;

    for (i = 0; i < node_count; i++) {
        node = pathnodes[cell->nodes[i]];
        if (!node) {
            continue;
        }

        if (pos[2] > node->origin[2] + 94.0f) {
            continue;
        }

        if (pos[2] + 94.0f < node->origin[2]) {
            continue;
        }

        VectorSubtract(node->origin, pos, delta);
        VectorCopy(delta, deltas[i]);

        dist = VectorLengthSquared(delta);

        for (j = n; j > 0; j--) {
            if (dist >= node_dist[j - 1]) {
                break;
            }

            node_dist[j] = node_dist[j - 1];
            nodes[j]     = nodes[j - 1];
        }

        n++;
        nodes[j]     = i;
        node_dist[j] = dist;
    }

    return n;
}

PathNode *PathSearch::FindNearestCover(Entity *pEnt, Vector& vPos, Entity *pEnemy)
{
    // not found in ida
    return NULL;
}

//===============
// Added in OPM
//===============

Event EV_NavMaster_CreatePaths
(
    "nav_build",
    EV_CHEAT,
    NULL,
    NULL,
    "Build navigation path"
);
Event EV_NavMaster_SpawnNode
(
    "nav_create",
    EV_CHEAT,
    "S",
    "type",
    "Create a new node at the player's origin"
);
Event EV_NavMaster_SetNodeFlags
(
    "nav_node_set_type",
    EV_CHEAT,
    "sSS",
    "type1 type2 ...",
    "Set the selected node type. Type can be one of the following values:\n"
    "- corner_left\n"
    "- corner_right\n"
    "- duck\n"
    "- sniper\n"
    "- concealment\n"
    "- cover\n"
    "- crate\n"
    "- none\n"
);
Event EV_NavMaster_SetNodeTargetName
(
    "nav_node_set_targetname",
    EV_CHEAT,
    "S",
    "targetname",
    "Set the selected node target name"
);
Event EV_NavMaster_SetNodeTarget
(
    "nav_node_set_target",
    EV_CHEAT,
    "S",
    "target",
    "Set the selected node target"
);
Event EV_NavMaster_RemoveNode
(
    "nav_node_remove",
    EV_CHEAT,
    NULL,
    NULL,
    "Delete the selected node"
);

CLASS_DECLARATION(Listener, NavMaster, NULL) {
    {&EV_NavMaster_CreatePaths,       &NavMaster::CreatePaths      },
    {&EV_NavMaster_SpawnNode,         &NavMaster::CreateNode       },
    {&EV_NavMaster_SetNodeFlags,      &NavMaster::SetNodeFlags     },
    {&EV_NavMaster_SetNodeTargetName, &NavMaster::SetNodeTargetName},
    {&EV_NavMaster_SetNodeTarget,     &NavMaster::SetNodeTarget    },
    {&EV_NavMaster_RemoveNode,        &NavMaster::RemoveNode       },
    {NULL,                            NULL                         }
};

NavMaster navMaster;

NavMaster::NavMaster() {}

void NavMaster::Init()
{
    G_CreateMaster("nav", this);

    if (ai_editmode->integer) {
        // show routes by default
        gi.cvar_set("ai_showroutes", "1");
        gi.cvar_set("ai_shownode", "30");
    }
}

void NavMaster::CreatePaths(Event *ev)
{
    if (!ai_editmode->integer) {
        return;
    }
    PathSearch::ClearNodes();
    PathSearch::CreatePaths();
}

void NavMaster::CreateNode(Event *ev)
{
    str       type;
    int       spawnflags = 0;
    PathNode *node;
    Entity   *ent;

    if (ev->NumArgs() > 0) {
        type       = ev->GetString(1);
        spawnflags = GetNodeTypeFromName(type);
        if (spawnflags == -1) {
            ScriptError("Unrecognized node type '%s'", type.c_str());
        }
    }

    ent = g_entities->entity;
    if (!ent) {
        return;
    }

    node = new PathNode;

    node->nodeflags = spawnflags;
    node->setOrigin(ent->origin);
}

void NavMaster::SetNodeFlags(Event *ev)
{
    str type;
    str typelist;
    int spawnflags;
    int i;

    CheckNodeSelected();

    if (ev->NumArgs() < 1) {
        ScriptError("A type is required");
        return;
    }

    spawnflags = 0;

    for (i = 1; i <= ev->NumArgs(); i++) {
        int flags;

        type = ev->GetString(i);

        flags = GetNodeTypeFromName(type);
        if (flags == -1) {
            ScriptError("Unrecognized node type '%s'", type.c_str());
        }

        spawnflags |= flags;

        if (i != 1) {
            typelist += ", ";
        }
        typelist += "'" + type + "'";
    }

    selectedNode->nodeflags = spawnflags;

    gi.Printf("Node %d set type %s\n", selectedNode->nodenum, typelist.c_str());
}

void NavMaster::SetNodeTargetName(Event *ev)
{
    CheckNodeSelected();

    selectedNode->targetname = ev->GetString(1);

    gi.Printf("Node %d got new targetname '%s'\n", selectedNode->nodenum, selectedNode->targetname.c_str());
}

void NavMaster::SetNodeTarget(Event *ev)
{
    CheckNodeSelected();

    selectedNode->target = ev->GetString(1);

    gi.Printf("Node %d got new target '%s'\n", selectedNode->nodenum, selectedNode->target.c_str());
}

void NavMaster::RemoveNode(Event *ev)
{
    str targetname;
    int num;

    CheckNodeSelected();

    num = selectedNode->nodenum;
    targetname = selectedNode->targetname;
    AI_RemoveNode(selectedNode);

    gi.Printf("Node %d removed\n", num, targetname.c_str());
}

void NavMaster::CheckNodeSelected()
{
    if (!selectedNode) {
        ScriptError("No node selected");
    }
}

void NavMaster::Frame()
{
    float  phase;
    vec3_t color;

    if (!ai_editmode->integer) {
        return;
    }

    phase = fmod(level.time * 2, 2.0);
    if (phase > 1.0) {
        phase = 2.0 - phase;
    }
    //phase = (sin(level.time * 5) + 1) / 2.0;

    selectedNode = DetermineCurrentNode();
    if (selectedNode) {
        Vector mins, maxs;
        Vector org;
        vec3_t colorPhase;

        GetPathnodeColor(selectedNode->nodeflags, color);

        colorPhase[0] = 0;
        colorPhase[1] = (1.0 - phase) * 0.5;
        colorPhase[2] = phase;
        color[1]      = Q_max(color[1], colorPhase[1]) - Q_min(color[1], colorPhase[1]);
        color[2]      = Q_max(color[2], colorPhase[2]) - Q_min(color[2], colorPhase[2]);

        mins = Vector(-24, -24, -24);
        maxs = Vector(24, 24, 24);
        org  = selectedNode->centroid;
        org.z += 24;

        G_DebugBBox(org, mins, maxs, color[0], color[1], color[2], 1.0);
    }
}

PathNode *NavMaster::DetermineCurrentNode() const
{
    Entity   *ent;
    PathNode *bestnode;
    float     bestdist;
    Vector    delta;
    Vector    dir;
    int       i;

    ent = g_entities->entity;
    if (!ent) {
        return NULL;
    }

    if (ent->IsSubclassOfSentient()) {
        Sentient    *sent       = static_cast<Sentient *>(ent);
        const Vector viewAngles = sent->GetViewAngles();

        Vector forward;
        viewAngles.AngleVectorsLeft(&dir);
    } else {
        dir = ent->orientation[0];
    }

    bestdist = 1e+12f;
    bestnode = NULL;

    for (i = 0; i < PathSearch::nodecount; i++) {
        PathNode *node = PathSearch::pathnodes[i];
        float     dist;

        if (!node) {
            continue;
        }

        delta = node->centroid - ent->centroid;
        if (abs(delta.z) > (MAXS_Z * 2)) {
            continue;
        }

        if (!FovCheck(dir, delta, 45)) {
            continue;
        }

        dist = delta.lengthSquared();
        if (dist < bestdist) {
            bestdist = dist;
            bestnode = node;
        }
    }

    return bestnode;
}

bool NavMaster::FovCheck(const Vector& dir, const Vector& delta, float fov) const
{
    float fovdot   = cos(DEG2RAD(fov / 2.f));
    float deltadot = DotProduct(delta, dir);

    if (fovdot < 0.0f) {
        if (deltadot >= 0.0f) {
            return true;
        }

        return VectorLengthSquared(delta) * Square(fovdot) > Square(deltadot);
    } else {
        if (deltadot < 0.0f) {
            return false;
        }

        return VectorLengthSquared(delta) * Square(fovdot) < Square(deltadot);
    }
}

Event EV_AttractiveNode_GetPriority
(
    "priority",
    EV_DEFAULT,
    NULL,
    NULL,
    "Get the node priority",
    EV_GETTER
);
Event EV_AttractiveNode_SetPriority
(
    "priority",
    EV_DEFAULT,
    "i",
    "priority",
    "Set the node priority",
    EV_SETTER
);
Event EV_AttractiveNode_GetDistance
(
    "max_dist",
    EV_DEFAULT,
    NULL,
    NULL,
    "Get the max distance for this node",
    EV_GETTER
);
Event EV_AttractiveNode_SetDistance
(
    "max_dist",
    EV_DEFAULT,
    "f",
    "max_dist",
    "Set the max distance for this node to be attracted, -1 for unlimited distance.",
    EV_SETTER
);
Event EV_AttractiveNode_GetStayTime
(
    "stay_time",
    EV_DEFAULT,
    NULL,
    NULL,
    "Get the max stay time for this node",
    EV_GETTER
);
Event EV_AttractiveNode_SetStayTime
(
    "stay_time",
    EV_DEFAULT,
    "f",
    "stay_time",
    "Set the maximum stay time AI will stay on this node",
    EV_SETTER
);
Event EV_AttractiveNode_GetRespawnTime
(
    "respawn_time",
    EV_DEFAULT,
    NULL,
    NULL,
    "Get the how much time will this node re-attract already attracted AIs",
    EV_GETTER
);
Event EV_AttractiveNode_SetRespawnTime
(
    "respawn_time",
    EV_DEFAULT,
    "f",
    "respawn_time",
    "Set the how much time will this node re-attract already attracted AIs. The minimum required value is 1, otherwise "
    "AI will get stuck.",
    EV_SETTER
);
Event EV_AttractiveNode_GetTeam
(
    "team",
    EV_DEFAULT,
    NULL,
    NULL,
    "Get the attractive node team. 'none' for no team.",
    EV_GETTER
);
Event EV_AttractiveNode_SetTeam
(
    "team",
    EV_DEFAULT,
    "s",
    "team",
    "Set the attractive node team. 'none' for no team.",
    EV_SETTER
);
Event EV_AttractiveNode_SetUse
(
    "setuse",
    EV_DEFAULT,
    "b",
    "use",
    "Set if AI should use or not"
);

CLASS_DECLARATION(SimpleArchivedEntity, AttractiveNode, NULL) {
    {&EV_AttractiveNode_GetPriority,    &AttractiveNode::GetPriority   },
    {&EV_AttractiveNode_SetPriority,    &AttractiveNode::SetPriority   },
    {&EV_AttractiveNode_GetDistance,    &AttractiveNode::GetDistance   },
    {&EV_AttractiveNode_SetDistance,    &AttractiveNode::SetDistance   },
    {&EV_AttractiveNode_GetStayTime,    &AttractiveNode::GetStayTime   },
    {&EV_AttractiveNode_SetStayTime,    &AttractiveNode::SetStayTime   },
    {&EV_AttractiveNode_GetRespawnTime, &AttractiveNode::GetRespawnTime},
    {&EV_AttractiveNode_SetRespawnTime, &AttractiveNode::SetRespawnTime},
    {&EV_AttractiveNode_GetTeam,        &AttractiveNode::GetTeam       },
    {&EV_AttractiveNode_SetTeam,        &AttractiveNode::SetTeam       },
    {&EV_AttractiveNode_SetUse,         &AttractiveNode::SetUse        },
    {NULL,                              NULL                           }
};

Container<AttractiveNode *> attractiveNodes;

AttractiveNode::AttractiveNode()
{
    m_iPriority           = 0; // set to default 0
    m_fMaxStayTime        = 0; // set to default 0, could be just a pickup
    m_fMaxDistance        = 1024;
    m_fMaxDistanceSquared = m_fMaxDistance * m_fMaxDistance;
    m_fRespawnTime        = 15.0f; // set to default 15 seconds
    m_bUse                = false;
    m_csTeam              = STRING_EMPTY;
    m_iTeam               = TEAM_NONE;

    attractiveNodes.AddObject(this);
}

AttractiveNode::~AttractiveNode()
{
    attractiveNodes.RemoveObject(this);
}

bool AttractiveNode::CheckTeam(Sentient *sent)
{
    if (!m_iTeam) {
        return true;
    }

    if (sent->IsSubclassOfPlayer()) {
        Player *p = (Player *)sent;

        if ((m_iTeam == TEAM_FREEFORALL && g_gametype->integer >= GT_TEAM) || p->GetTeam() != m_iTeam) {
            return false;
        }
    } else {
        if (m_iTeam == TEAM_ALLIES && sent->m_Team != TEAM_AMERICAN) {
            return false;
        } else if (m_iTeam == TEAM_AXIS && sent->m_Team != TEAM_GERMAN) {
            return false;
        }
    }

    return true;
}

void AttractiveNode::setMaxDist(float dist)
{
    m_fMaxDistance = dist;

    if (dist < 0) {
        m_fMaxDistanceSquared = -1;
    } else {
        m_fMaxDistanceSquared = dist * dist;
    }
}

void AttractiveNode::GetPriority(Event *ev)
{
    ev->AddInteger(m_iPriority);
}

void AttractiveNode::SetPriority(Event *ev)
{
    m_iPriority = ev->GetInteger(1);
}

void AttractiveNode::GetDistance(Event *ev)
{
    ev->AddFloat(m_fMaxDistance);
}

void AttractiveNode::SetDistance(Event *ev)
{
    setMaxDist(ev->GetFloat(1));
}

void AttractiveNode::GetStayTime(Event *ev)
{
    ev->AddFloat(m_fMaxStayTime);
}

void AttractiveNode::SetStayTime(Event *ev)
{
    m_fMaxStayTime = ev->GetFloat(1);
}

void AttractiveNode::GetRespawnTime(Event *ev)
{
    ev->AddFloat(m_fRespawnTime);
}

void AttractiveNode::SetRespawnTime(Event *ev)
{
    m_fRespawnTime = ev->GetFloat(1);
    if (m_fRespawnTime < 1.0f) {
        m_fRespawnTime = 1.0f;
    }
}

void AttractiveNode::GetTeam(Event *ev)
{
    ev->AddConstString(m_csTeam);
}

void AttractiveNode::SetTeam(Event *ev)
{
    if (ev->IsNilAt(1)) {
        m_csTeam = STRING_EMPTY;
        m_iTeam  = TEAM_NONE;
        return;
    }

    m_csTeam = ev->GetConstString(1);

    switch (m_csTeam) {
    case STRING_EMPTY:
        m_iTeam = TEAM_NONE;
        break;
    case STRING_SPECTATOR:
        m_iTeam = TEAM_SPECTATOR;
        break;
    case STRING_FREEFORALL:
        m_iTeam = TEAM_FREEFORALL;
        break;
    case STRING_ALLIES:
    case STRING_AMERICAN:
        m_iTeam = TEAM_ALLIES;
        break;
    case STRING_AXIS:
    case STRING_GERMAN:
        m_iTeam = TEAM_AXIS;
        break;
    default:
        m_iTeam = TEAM_NONE;
        ScriptError("Invalid team %s\n", ev->GetString(1).c_str());
    }
}

void AttractiveNode::SetUse(Event *ev)
{
    m_bUse = ev->GetBoolean(1);
}
