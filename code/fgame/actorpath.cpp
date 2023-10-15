/*
===========================================================================
Copyright (C) 2015 the OpenMoHAA team

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

// actor.cpp:

#include "actor.h"

ActorPath::ActorPath()
{
    m_FallHeight       = 96;
    m_path             = NULL;
    m_pathlen          = 0;
    m_fLookAhead       = 4096.0f;
    m_bChangeLookAhead = true;

    Clear();
}

ActorPath::~ActorPath()
{
    if (m_path) {
        delete[] m_path;
    }
}

void ActorPath::Clear(void)
{
    m_startpathpos = 0;
    m_pathpos      = 0;
    m_Side         = false;
    m_Time         = -10000000;
    m_delta[0]     = 0;
    m_delta[1]     = 0;
}

void ActorPath::ForceShortLookahead(void)
{
    m_fLookAhead = 4096.0f;
}

float ActorPath::PathLookAhead(float total_area, Vector& end, float *origin)
{
    float     area = total_area;
    float     s;
    float     t;
    float     normal[2];
    float     delta[2];
    Vector    pos;
    float     fallheight;
    PathInfo *current_path = m_pathpos;

    while (1) {
        pos = current_path->point;

        if (current_path == m_path) {
            break;
        }

        fallheight = current_path->point[2] - origin[2];

        if (fallheight > 94.0f || fallheight < -94.0f) {
            VectorCopy(current_path->point, end);
            m_HasCompleteLookahead = false;
            return area;
        }

        current_path--;

        normal[0] = current_path->point[1] - pos[1];
        normal[1] = pos[0] - current_path->point[0];

        VectorNormalize2D(normal);

        delta[0] = current_path->point[0] - origin[0];
        delta[1] = current_path->point[1] - origin[1];

        t = fabs(DotProduct2D(delta, normal)) * current_path->dist;

        if (t >= area) {
            t = area / t;
            s = 1.0f - t;

            end[0]                 = current_path->point[0] * t + pos[0] * s;
            end[1]                 = current_path->point[1] * t + pos[1] * s;
            end[2]                 = current_path->point[2] * t + pos[2] * s;
            m_HasCompleteLookahead = false;
            return 0;
        }

        area -= t;
    }

    VectorCopy(current_path->point, end);
    m_HasCompleteLookahead = true;

    return area;
}

bool ActorPath::DoesTheoreticPathExist(
    float *start, float *end, class SimpleActor *ent, float maxPath, float *vLeashHome, float fLeashDistSquared
)
{
    return PathSearch::FindPath(start, end, ent, maxPath, NULL, 0, m_FallHeight) != 0;
}

void ActorPath::FindPath(
    float *start, float *end, Entity *ent, float maxPath, float *vLeashHome, float fLeashDistSquared
)
{
    int depth = PathManager.FindPath(start, end, ent, maxPath, vLeashHome, fLeashDistSquared, m_FallHeight);

    if (depth) {
        if (depth > m_pathlen) {
            if (m_path) {
                delete[] m_path;
            }

            m_pathlen = 10 * ((depth - 1) / 10) + 10;
            m_path    = new PathInfo[m_pathlen];
        }

        m_startpathpos = PathManager.GeneratePath(m_path);
        m_pathpos      = m_startpathpos;
        m_TotalDist    = PathManager.total_dist;
        m_Side         = false;
        m_Time         = level.inttime;
        UpdatePos(start);
    } else {
        Clear();
    }
}

void ActorPath::FindPathAway(
    float  *start,
    float  *avoid,
    float  *vPreferredDir,
    Entity *ent,
    float   fMinSafeDist,
    float  *vLeashHome,
    float   fLeashDistSquared
)
{
    int depth = PathManager.FindPathAway(
        start, avoid, vPreferredDir, ent, fMinSafeDist, vLeashHome, fLeashDistSquared, m_FallHeight
    );

    if (depth) {
        if (depth > m_pathlen) {
            if (m_path) {
                delete[] m_path;
            }

            m_pathlen = 10 * (depth - 1) / 10 + 10;
            m_path    = new PathInfo[m_pathlen];
        }

        m_startpathpos = PathManager.GeneratePathAway(m_path);
        m_pathpos      = m_startpathpos;
        m_TotalDist    = PathManager.total_dist;
        m_Side         = false;
        m_Time         = level.inttime;
        UpdatePos(start);
    } else {
        Clear();
    }
}

void ActorPath::FindPathNear(
    float  *start,
    float  *nearby,
    Entity *ent,
    float   maxPath,
    float   fRadiusSquared,
    float  *vLeashHome,
    float   fLeashDistSquared
)
{
    int depth = PathManager.FindPathNear(
        start, nearby, ent, maxPath, fRadiusSquared, vLeashHome, fLeashDistSquared, m_FallHeight
    );

    if (depth) {
        if (depth > m_pathlen) {
            if (m_path) {
                delete[] m_path;
            }

            m_pathlen = 10 * (depth - 1) / 10 + 10;
            m_path    = new PathInfo[m_pathlen];
        }

        m_startpathpos = PathManager.GeneratePathNear(m_path);
        m_pathpos      = m_startpathpos;
        m_TotalDist    = PathManager.total_dist;
        m_Side         = false;
        m_Time         = level.inttime;
        UpdatePos(start);
    } else {
        Clear();
    }
}

void ActorPath::ReFindPath(float *start, Entity *ent)
{
    vec3_t point;
    // this is a critical bug in all versions of mohaa, it passes directly m_path->point
    // but m_path can be deleted afterwards, leaving a dangling pointer to the path_end
    // global variable
    VectorCopy(m_path->point, point);
    int depth = PathManager.FindPath(start, point, ent, 0, NULL, 0, m_FallHeight);

    if (depth) {
        if (depth > m_pathlen) {
            if (m_path) {
                delete[] m_path;
            }

            m_pathlen = 10 * (depth - 1) / 10 + 10;
            m_path    = new PathInfo[m_pathlen];
        }

        m_startpathpos = PathManager.GeneratePath(m_path);
        m_pathpos      = m_startpathpos;
        m_TotalDist    = PathManager.total_dist;
        m_Side         = false;
        m_Time         = level.inttime;
        UpdatePos(start);
    } else {
        Clear();
    }
}

void ActorPath::UpdatePos(float *origin, float fNodeRadius)
{
    Vector    end;
    float     s = 0;
    float     t = 0;
    vec2_t    delta;
    float     current_dot  = 0;
    float     previous_dot = 0;
    Vector    pos;
    PathInfo *current_path = NULL;
    vec2_t    dir;
    Vector    end2;
    vec2_t    delta2;
    vec2_t    dir2;

    if (m_pathpos == m_path) {
        end                    = m_pathpos->point;
        m_bChangeLookAhead     = true;
        m_HasCompleteLookahead = true;
        m_delta[0]             = end[0] - origin[0];
        m_delta[1]             = end[1] - origin[1];
        VectorNormalize2D2(m_delta, dir);
    } else if (m_fLookAhead >= 4096.0f) {
        if (m_fLookAhead - 4096.0f >= PathLookAhead(m_fLookAhead, end, origin)) {
            Vector mins = Vector(-15, -15, 0);
            Vector maxs = Vector(15, 15, 60);
            Vector e    = end + Vector(0, 0, 32);

            pos = origin + Vector(0, 0, 32);

            if (G_SightTrace(
                    pos,
                    mins,
                    maxs,
                    e,
                    (gentity_t *)NULL, // g_entities[ 0 ].entity
                    0,
                    MASK_PLAYERSOLID,
                    false,
                    "Actor::UpdatePos 2"
                )
                != true) {
                if (m_bChangeLookAhead) {
                    m_fLookAhead -= 2048.0f;
                    m_bChangeLookAhead = false;
                } else {
                    m_fLookAhead *= 0.5f;
                }

                if (m_fLookAhead < 4096.0f) {
                    m_fLookAhead = 4096.0f;
                }

                PathLookAhead(4096.0f, end, origin);
                goto __setdelta;
            }
        }

        m_fLookAhead += 1024.0f;

        if (m_fLookAhead > 65536.0f) {
            m_fLookAhead = 65536.0f;
        }

        m_bChangeLookAhead = true;

    __setdelta:

        m_delta[0] = end[0] - origin[0];
        m_delta[1] = end[1] - origin[1];

        VectorNormalize2D2(m_delta, dir);
    } else if (PathLookAhead(4096.0f, end, origin) < 4096.0f - m_fLookAhead) {
        PathLookAhead(m_fLookAhead, end2, origin);

        Vector mins = Vector(-15, -15, 0);
        Vector maxs = Vector(15, 15, 60);
        Vector e    = end2 + Vector(0, 0, 32);

        pos = origin + Vector(0, 0, 32);

        if (G_SightTrace(pos, mins, maxs, e, (gentity_t *)NULL, 0, MASK_MONSTERSOLID, false, "Actor::UpdatePos 1")
            != true) {
            m_fLookAhead += 1024.0f;

            if (m_fLookAhead > 4096.0f) {
                m_fLookAhead = 4096.0f;
            }
        } else {
            m_fLookAhead -= 1024.0f;

            if (m_fLookAhead < 1024.0f) {
                m_fLookAhead = 1024.0f;
            }

            PathLookAhead(m_fLookAhead, end2, origin);
        }

        delta[0] = end2[0] - origin[0];
        delta[1] = end2[1] - origin[1];
        VectorNormalize2D2(delta, dir2);

        m_delta[0] = end[0] - origin[0];
        m_delta[1] = end[1] - origin[1];
        VectorNormalize2D2(m_delta, dir);

        if (DotProduct2D(dir, dir2) > 0.7f) {
            m_delta[0] = delta[0];
            m_delta[1] = delta[1];
        }

        m_bChangeLookAhead = true;
    } else {
        m_fLookAhead -= 1024.0f;

        if (m_fLookAhead < 1024.0f) {
            m_fLookAhead = 1024.0f;
        }

        m_delta[0] = end[0] - origin[0];
        m_delta[1] = end[1] - origin[1];
        VectorNormalize2D2(m_delta, dir);

        m_bChangeLookAhead = true;
    }

    current_path = m_pathpos;

    while (1) {
        delta2[0]   = current_path->point[0] - origin[0];
        delta2[1]   = current_path->point[1] - origin[1];
        current_dot = DotProduct2D(delta2, dir) - fNodeRadius;

        if (current_dot >= 0.0f) {
            break;
        }

        previous_dot = current_dot;

        if (current_path == LastNode()) {
            break;
        }

        current_path--;
    }

    if (current_path != m_pathpos) {
        m_pathpos = current_path + 1;

        t = previous_dot / (previous_dot - current_dot);
        s = 1.0f - t;

        /*m_pathpos->point[ 0 ] = m_pathpos->point[ 0 ] * s + current_path->point[ 0 ] * t;
		m_pathpos->point[ 1 ] = m_pathpos->point[ 1 ] * s + current_path->point[ 1 ] * t;
		m_pathpos->point[ 2 ] = m_pathpos->point[ 2 ] * s + current_path->point[ 2 ] * t;*/

        VectorCopy(current_path->point, m_pathpos->point);

        current_path->dist *= s;

        m_Side = true;
    } else {
        m_Side = false;
    }
}

bool ActorPath::Complete(const float *origin) const
{
    if (!m_HasCompleteLookahead) {
        return false;
    }

    if (!m_path) {
        return true;
    }

    if (fabs(origin[0] - m_path->point[0]) < 16.0f && fabs(origin[1] - m_path->point[1]) < 16.0f) {
        return true;
    }

    return false;
}

void ActorPath::TrimPathFromEnd(int nNodesPop)
{
    int iLastPos = m_path - m_pathpos;

    if (iLastPos - nNodesPop > 0) {
        for (int i = 0; i < iLastPos; i++) {
            m_path[i] = m_path[i + nNodesPop];
        }
    } else {
        Clear();
    }
}

void ActorPath::Shorten(float fDistRemove)
{
    if (m_path->dist > fDistRemove) {
        m_path->point[0] += m_path->dir[0] * -fDistRemove;
        m_path->point[1] += m_path->dir[1] * -fDistRemove;
        m_path->point[2] += m_path->point[2] * -fDistRemove;
        m_path->dist -= fDistRemove;
    } else {
        while (fDistRemove - m_path->dist > m_path->dist) {
            TrimPathFromEnd(1);

            if (!m_pathpos) {
                return;
            }
        }
    }
}

PathInfo *ActorPath::StartNode(void) const
{
    return m_startpathpos;
}

PathInfo *ActorPath::CurrentNode(void) const
{
    return m_pathpos;
}

int ActorPath::CurrentNodeIndex(void) const
{
    return m_pathpos ? m_pathpos - m_path : -1;
}

PathInfo *ActorPath::NextNode(void) const
{
    return m_pathpos == m_path ? NULL : m_pathpos - 1;
}

PathInfo *ActorPath::LastNode(void) const
{
    return m_path;
}

const float *ActorPath::CurrentPathDir(void) const
{
    if (m_pathpos == m_path) {
        return m_delta;
    } else {
        return m_pathpos[-1].dir;
    }
}

const float *ActorPath::CurrentPathGoal(void) const
{
    return m_pathpos->point;
}

int ActorPath::Time(void) const
{
    return m_Time;
}

Vector ActorPath::CurrentDelta(void) const
{
    return Vector(m_delta[0], m_delta[1], 0);
}

bool ActorPath::IsAccurate(void) const
{
    return m_pathpos->bAccurate;
}

void ActorPath::SetFallHeight(float fHeight)
{
    m_FallHeight = fHeight;
}

float ActorPath::GetFallHeight(void) const
{
    return m_FallHeight;
}

bool ActorPath::HasCompleteLookahead(void) const
{
    return m_HasCompleteLookahead;
}

float ActorPath::TotalDist(void) const
{
    return m_TotalDist;
}

bool ActorPath::IsSide(void) const
{
    return m_Side;
}
