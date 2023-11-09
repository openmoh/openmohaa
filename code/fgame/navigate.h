/*
===========================================================================
Copyright (C) 2023 the OpenMoHAA team

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

// navigate.h:
// Potentially could be an C++ implementation of the A* search algorithm, but
// is currently unfinished.
//

#pragma once

#include "g_local.h"
#include "class.h"
#include "entity.h"
#include "stack.h"
#include "container.h"
#include "doors.h"
#include "sentient.h"
#include "../qcommon/qfiles.h"

extern Event EV_AI_SavePaths;
extern Event EV_AI_SaveNodes;
extern Event EV_AI_LoadNodes;
extern Event EV_AI_ClearNodes;
extern Event EV_AI_RecalcPaths;
extern Event EV_AI_CalcPath;
extern Event EV_AI_DisconnectPath;

extern cvar_t *ai_showroutes;
extern cvar_t *ai_showroutes_distance;
extern cvar_t *ai_shownodenums;
extern cvar_t *ai_shownode;
extern cvar_t *ai_showallnode;
extern cvar_t *ai_showpath;
extern cvar_t *ai_fallheight;
extern cvar_t *ai_debugpath;
extern cvar_t *ai_pathchecktime;
extern cvar_t *ai_pathcheckdist;

extern int ai_maxnode;

#define MAX_PATHCHECKSPERFRAME 4

extern int path_checksthisframe;

#define MAX_PATH_LENGTH  128 // should be more than plenty
#define NUM_PATHSPERNODE 48

class Path;
class PathNode;

#define NUM_WIDTH_VALUES 16
#define WIDTH_STEP       8
#define MAX_WIDTH        (WIDTH_STEP * NUM_WIDTH_VALUES)
#define MAX_HEIGHT       128

#define CHECK_PATH(path, width, height)                \
    ((((width) >= MAX_WIDTH) || ((width) < 0)) ? false \
                                               : ((int)(path)->maxheight[((width) / WIDTH_STEP) - 1] < (int)(height)))

class pathway_ref
{
public:
    short int from;
    short int to;
};

typedef struct {
    byte      numBlockers;
    byte      badPlaceTeam[2];
    short int node;
    short int fallheight;
    float     dist;
    float     dir[2];
    float     pos1[3];
    float     pos2[3];
} pathway_t;

class PathInfo
{
public:
    bool  bAccurate;
    float point[3];
    float dist;
    float dir[2];

public:
    void Archive(Archiver& arc);
};

inline void PathInfo::Archive(Archiver& arc)
{
    arc.ArchiveBool(&bAccurate);
    arc.ArchiveVec3(point);
    arc.ArchiveFloat(&dist);
    arc.ArchiveVec2(dir);
}

typedef enum {
    NOT_IN_LIST,
    IN_OPEN,
    IN_CLOSED
} pathlist_t;

#define PATH_DONT_LINK  1
#define AI_DUCK         2
#define AI_COVER        4
#define AI_CONCEALMENT  8
#define AI_CORNER_LEFT  16
#define AI_CORNER_RIGHT 32
#define AI_SNIPER       64
#define AI_CRATE        128
// Added in 2.0
#define AI_LOW_WALL_ARC      256

#define AI_COVERFLAGS        (AI_CRATE | AI_SNIPER | AI_CORNER_RIGHT | AI_CORNER_LEFT | AI_CONCEALMENT | AI_COVER | AI_LOW_WALL_ARC)
#define AI_COVERFLAGS2       (AI_SNIPER | AI_CORNER_RIGHT | AI_CORNER_LEFT | AI_CONCEALMENT)
#define AI_COVERFLAGS3       (AI_SNIPER | AI_CORNER_RIGHT | AI_CORNER_LEFT | AI_CONCEALMENT | AI_DUCK | AI_CONCEALMENT)
#define AI_SNIPERFLAGS       (AI_SNIPER | AI_CORNER_RIGHT | AI_CORNER_LEFT | AI_COVER)
#define AI_CRATEFLAGS        (AI_CRATE | AI_CORNER_LEFT | AI_CORNER_RIGHT | AI_CONCEALMENT | AI_COVER | AI_DUCK)
#define AI_COVER_LEFT_FLAGS  (AI_CRATE | AI_SNIPER | AI_CORNER_LEFT | AI_CONCEALMENT | AI_DUCK)
#define AI_COVER_RIGHT_FLAGS (AI_CRATE | AI_SNIPER | AI_CORNER_RIGHT | AI_CONCEALMENT | AI_DUCK)
#define AI_COVER_MASK        (AI_CRATE | AI_CORNER_RIGHT | AI_CORNER_LEFT | AI_CONCEALMENT | AI_COVER)
#define AI_CONCEALMENT_MASK  (AI_CRATE | AI_CORNER_RIGHT | AI_CONCEALMENT | AI_LOW_WALL_ARC)
#define AI_ALL               (AI_DUCK | AI_COVER | AI_CONCEALMENT | AI_CORNER_LEFT | AI_CORNER_RIGHT | AI_SNIPER | AI_CRATE)

void DrawNode(int iNodeCount);
void DrawAllConnections(void);

class PathNode : public SimpleEntity
{
public:
    int             findCount;
    pathway_t      *Child;
    int             numChildren;
    int             virtualNumChildren;
    float           f;
    float           h;
    float           g;
    class PathNode *Parent;
    bool            inopen;
    PathNode       *PrevNode;
    PathNode       *NextNode;
    short int       pathway;
    const vec_t    *m_PathPos;
    float           dist;
    float           dir[2];
    int             nodeflags;
    SafePtr<Entity> pLastClaimer;
    int             iAvailableTime;
    int             nodenum;
    short int       m_Depth;
    float           m_fLowWallArc;

    friend class PathSearch;
    friend void DrawAllConnections(void);

private:
    void ConnectTo(PathNode *node);
    void SetNodeFlags(Event *ev);
    void SetLowWallArc(Event *ev); // Added in 2.0
    void Remove(Event *ev);

public:
    CLASS_PROTOTYPE(PathNode);

    PathNode();
    virtual ~PathNode();

    void *operator new(size_t size);
    void  operator delete(void *ptr);

    void Archive(Archiver& arc) override;
    void ArchiveDynamic(Archiver& arc);
    void ArchiveStatic(Archiver& arc);

    bool     CheckPathTo(PathNode *node);
    void     CheckPathToDefault(PathNode *node, pathway_t *pathway);
    qboolean LadderTo(PathNode *node, pathway_t *pathway);

    void      DrawConnections(void);
    void      Claim(Entity *pClaimer);
    void      Relinquish(void);
    Entity   *GetClaimHolder(void) const;
    bool      IsClaimedByOther(Entity *pPossibleClaimer) const;
    void      MarkTemporarilyBad(void);
    void      ConnectChild(int i);
    void      DisconnectChild(int i);
    const_str GetSpecialAttack(class Actor *pActor);
    void      IsTouching(Event *ev);
    qboolean  IsTouching(Entity *e1);
    void      setOriginEvent(Vector org) override;
};

typedef SafePtr<PathNode> PathNodePtr;

#define PATHMAP_CELLSIZE 256
#define PATHMAP_GRIDSIZE (MAX_MAP_BOUNDS * 2 / PATHMAP_CELLSIZE)

#define PATHMAP_NODES    128 // 128 - sizeof( int ) / sizeof( short )

#define MAX_PATHNODES    4096

class MapCell : public Class
{
private:
    int    numnodes;
    short *nodes;

    friend class PathSearch;

public:
    MapCell();
    ~MapCell();

    qboolean AddNode(PathNode *node);
    int      NumNodes(void);
};

class PathSearch : public Listener
{
    friend class PathNode;

private:
    static MapCell   PathMap[PATHMAP_GRIDSIZE][PATHMAP_GRIDSIZE];
    static PathNode *open;
    static int       findFrame;
    static qboolean  m_bNodesloaded;
    static qboolean  m_NodeCheckFailed;
    static int       m_LoadIndex;

public:
    static PathNode   *pathnodes[4096];
    static int         nodecount;
    static float       total_dist;
    static const char *last_error;

private:
    static void     LoadAddToGrid(int x, int y);
    static void     LoadAddToGrid2(PathNode *node, int x, int y);
    static void     AddToGrid(PathNode *node, int x, int y);
    static bool     Connect(PathNode *node, int x, int y);
    static int      NodeCoordinate(float coord);
    static int      GridCoordinate(float coord);
    static qboolean ArchiveSaveNodes(void);
    static void     ArchiveLoadNodes(void);
    static void     Init(void);

public:
    CLASS_PROTOTYPE(PathSearch);

    PathSearch();
    virtual ~PathSearch();

    static void ArchiveStaticLoad(Archiver& arc);
    static void ArchiveStaticSave(Archiver& arc);
    static bool ArchiveDynamic(Archiver& arc);

    static void AddNode(PathNode *node);
    static void Connect(PathNode *node);
    static void UpdateNode(PathNode *node);

    static MapCell *GetNodesInCell(int x, int y);
    static MapCell *GetNodesInCell(const vec3_t pos);

    static class PathNode *DebugNearestStartNode(const vec3_t pos, Entity *ent = NULL);
    static class PathNode *NearestStartNode(const vec3_t pos, class SimpleActor *ent);
    static class PathNode *NearestEndNode(const vec3_t pos);
    static int             DebugNearestNodeList(const vec3_t pos, PathNode **nodelist, int iMaxNodes);
    static int             DebugNearestNodeList2(const vec3_t pos, PathNode **nodelist, int iMaxNodes);

    static void  ShowNodes(void);
    static void  LoadNodes(void);
    static void  CreatePaths(void);
    static void *AllocPathNode(void);
    static void  FreePathNode(void *);
    static void  ResetNodes(void);

    static void      UpdatePathwaysForBadPlace(const Vector     &origin, float radius, int dir, int team);
    static PathInfo *GeneratePath(PathInfo *path);
    static PathInfo *GeneratePathNear(PathInfo *path);
    static PathInfo *GeneratePathAway(PathInfo *path);

    static class PathNode *GetSpawnNode(ClassDef *cls);

    static int FindPath(
        const vec3_t start,
        const vec3_t end,
        Entity      *ent,
        float        maxPath,
        const vec3_t vLeashHome,
        float        fLeashDistSquared,
        int          fallheight
    );
    static int FindPathAway(
        const vec3_t start,
        const vec3_t avoid,
        const vec3_t vPreferredDir,
        Entity      *ent,
        float        fMinSafeDist,
        const vec3_t vLeashHome,
        float        fLeashDistSquared,
        int          fallheight
    );
    static int FindPathNear(
        const vec3_t start,
        const vec3_t end,
        Entity      *ent,
        float        maxPath,
        float        fRadiusSquared,
        const vec3_t vLeashHome,
        float        fLeashDistSquared,
        int          fallheight
    );
    static class PathNode *FindCornerNodeForWall(
        const vec3_t start, const vec3_t end, class SimpleActor *ent, float maxPath, const vec4_t plane
    );
    static class PathNode *FindCornerNodeForExactPath(class SimpleActor *self, Sentient *enemy, float fMaxPath);
    static int
    FindPotentialCover(class SimpleActor *pEnt, Vector& vPos, Entity *pEnemy, PathNode **ppFoundNodes, int iMaxFind);
    static void            PlayerCover(class Player *pPlayer);
    static class PathNode *FindNearestCover(class SimpleActor *pEnt, Vector& vPos, Entity *pEnemy);
    static class PathNode *FindNearestSniperNode(class SimpleActor *pEnt, Vector& vPos, Entity *pEnemy);

private:
    static int NearestNodeSetup(const vec3_t pos, MapCell *cell, int *nodes, vec3_t *deltas);
};

inline MapCell::~MapCell()
{
    numnodes = 0;
    nodes    = NULL;
}

extern PathSearch PathManager;

//===============
// Added in OPM
//===============

PathNode *AI_FindNode(const char *name);
void      AI_AddNode(PathNode *node);
void      AI_RemoveNode(PathNode *node);
void      AI_ResetNodes(void);

class AttractiveNode : public SimpleArchivedEntity
{
public:
    int       m_iPriority;
    bool      m_bUse;
    float     m_fMaxStayTime;
    float     m_fMaxDistance;
    float     m_fMaxDistanceSquared;
    float     m_fRespawnTime;
    const_str m_csTeam;
    int       m_iTeam;

private:
    //	Container< SafePtr< Sentient > >	m_pSentList;

public:
    CLASS_PROTOTYPE(AttractiveNode);

    AttractiveNode();
    ~AttractiveNode();

    bool CheckTeam(Sentient *sent);
    void setMaxDist(float dist);

    void GetPriority(Event *ev);
    void SetPriority(Event *ev);
    void GetDistance(Event *ev);
    void SetDistance(Event *ev);
    void GetStayTime(Event *ev);
    void SetStayTime(Event *ev);
    void GetRespawnTime(Event *ev);
    void SetRespawnTime(Event *ev);
    void GetTeam(Event *ev);
    void SetTeam(Event *ev);
    void SetUse(Event *ev);

    void Archive(Archiver& arc) override;
};

typedef SafePtr<AttractiveNode> AttractiveNodePtr;

inline void AttractiveNode::Archive(Archiver& arc)
{
    arc.ArchiveInteger(&m_iPriority);
    arc.ArchiveBool(&m_bUse);
    arc.ArchiveFloat(&m_fMaxStayTime);
    arc.ArchiveFloat(&m_fMaxDistanceSquared);
}

extern Container<AttractiveNode *> attractiveNodes;
