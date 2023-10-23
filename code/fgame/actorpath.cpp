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
    m_fLookAhead       = 4096;
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
    m_startpathpos = NULL;
    m_pathpos      = NULL;
    m_Side         = false;
    m_Time         = -10000000;
    VectorClear2D(m_delta);
    m_TotalDist = PathManager.total_dist;
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
    vec2_t    normal;
    vec2_t    delta;
    Vector    pos;
    float     fallheight;
    PathInfo *current_path = m_pathpos;

    pos = current_path->point;

    while (1) {
        if (current_path == m_path) {
            end                    = current_path->point;
            m_HasCompleteLookahead = true;
            return area;
        }

        fallheight = current_path->point[2] - origin[2];

        if (fallheight > 94.0f || fallheight < -94.0f) {
            end                    = current_path->point;
            m_HasCompleteLookahead = false;
            return area;
        }

        current_path--;

        // calculate the normal
        normal[0] = current_path->point[1] - pos[1];
        normal[1] = pos[0] - current_path->point[0];
        VectorNormalize2D(normal);
        VectorSub2D(current_path->point, origin, delta);

        t = fabs(DotProduct2D(delta, normal)) * current_path->dist;

        if (t >= area) {
            break;
        }

        area -= t;
        pos = current_path->point;
    }

    t = area / t;
    s = 1.0f - t;

    end[0] = pos[0] * s + current_path->point[0] * t;
    end[1] = pos[1] * s + current_path->point[1] * t;
    end[2] = pos[2] * s + current_path->point[2] * t;

    m_HasCompleteLookahead = false;
    return 0;
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
    int depth;

    depth = PathManager.FindPath(start, end, ent, maxPath, vLeashHome, fLeashDistSquared, m_FallHeight);

    if (!depth) {
        Clear();
        return;
    }

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
    int depth;

    depth = PathManager.FindPathAway(
        start, avoid, vPreferredDir, ent, fMinSafeDist, vLeashHome, fLeashDistSquared, m_FallHeight
    );

    if (!depth) {
        Clear();
        return;
    }

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
    int depth;

    depth = PathManager.FindPathNear(
        start, nearby, ent, maxPath, fRadiusSquared, vLeashHome, fLeashDistSquared, m_FallHeight
    );

    if (!depth) {
        Clear();
        return;
    }

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
}

void ActorPath::ReFindPath(float *start, Entity *ent)
{
    int    depth;
    vec3_t point;
    // this is a critical bug in all versions of mohaa, it passes directly m_path->point
    // but m_path can be deleted afterwards, leaving a dangling pointer to the path_end
    // global variable
    VectorCopy(m_path->point, point);

    depth = PathManager.FindPath(start, point, ent, 0, NULL, 0, m_FallHeight);

    if (!depth) {
        Clear();
        return;
    }

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
        VectorSub2D(end, origin, m_delta);
        VectorNormalize2D2(m_delta, dir);
    } else if (m_fLookAhead >= 4096.0f) {
        if (PathLookAhead(m_fLookAhead, end, origin) <= (m_fLookAhead - 4096.0f)
            || G_SightTrace(
                origin + Vector(0, 0, 32),
                Vector(-15, -15, 0),
                Vector(15, 15, 60),
                end + Vector(0, 0, 32),
                g_entities[0].entity,
                NULL,
                MASK_ACTORPATH,
                false,
                "Actor::UpdatePos 2"
            )) {
            m_fLookAhead += 1024.0f;

            if (m_fLookAhead > Square(256)) {
                m_fLookAhead = Square(256);
            }

            m_bChangeLookAhead = true;
        } else {
            if (m_bChangeLookAhead) {
                m_fLookAhead -= 2048.0f;
                m_bChangeLookAhead = false;
            } else {
                m_fLookAhead /= 2.f;
            }

            if (m_fLookAhead < 4096.0f) {
                m_fLookAhead = 4096.0f;
            }

            PathLookAhead(m_fLookAhead, end, origin);
        }

        VectorSub2D(end, origin, m_delta);
        VectorNormalize2D2(m_delta, dir);
    } else if (PathLookAhead(4096.0f, end, origin) < 4096.0f - m_fLookAhead) {
        PathLookAhead(m_fLookAhead, end2, origin);

        if (!G_SightTrace(
                origin + Vector(0, 0, 32),
                Vector(-15, -15, 0),
                Vector(15, 15, 60),
                end2 + Vector(0, 0, 32),
                g_entities[0].entity,
                0,
                MASK_ACTORPATH,
                false,
                "Actor::UpdatePos 1"
            )) {
            m_fLookAhead -= 1024.0f;

            if (m_fLookAhead < 1024.0f) {
                m_fLookAhead = 1024.0f;
            }

            PathLookAhead(m_fLookAhead, end2, origin);
        } else {
            m_fLookAhead += 1024.0f;

            if (m_fLookAhead > 4096.0f) {
                m_fLookAhead = 4096.0f;
            }
        }

        VectorSub2D(end2, origin, delta);
        VectorNormalize2D2(delta, dir2);

        VectorSub2D(end, origin, delta);
        VectorNormalize2D2(m_delta, dir);

        if (DotProduct2D(dir, dir2) > 0.7f) {
            VectorCopy2D(delta, m_delta);
            VectorCopy2D(dir2, dir);
        }

        m_bChangeLookAhead = true;
    } else {
        m_fLookAhead -= 1024.0f;

        if (m_fLookAhead < 1024.0f) {
            m_fLookAhead = 1024.0f;
        }

        VectorSub2D(end, origin, m_delta);
        VectorNormalize2D2(m_delta, dir);

        m_bChangeLookAhead = true;
    }

    // Check added in OPM.
    //  Make sure to stop if it's the last node
    for (current_path = m_pathpos; current_path >= LastNode(); current_path--) {
        VectorSub2D(current_path->point, origin, delta2);
        //current_dot = DotProduct2D(delta2, dir) - fNodeRadius;
        // Removed in 2.0
        //  fNodeRadius is now unused?
        current_dot = DotProduct2D(delta2, dir);

        if (current_dot >= 0) {
            break;
        }

        previous_dot = current_dot;
    }

    if (current_path != m_pathpos) {
        m_pathpos = current_path + 1;

        t = previous_dot / (previous_dot - current_dot);
        s = 1.0f - t;

        m_pathpos->point[0] = m_pathpos->point[0] * s + current_path->point[0] * t;
        m_pathpos->point[1] = m_pathpos->point[1] * s + current_path->point[1] * t;
        m_pathpos->point[2] = m_pathpos->point[2] * s + current_path->point[2] * t;

        current_path->dist *= s;

        m_Side = true;
    } else {
        m_Side = false;
    }

    // Added in 2.0.
    //  Make sure to clear the delta if it's invalid (NaN, infinite...)
    if (!isfinite(m_delta[0]) || !isfinite(m_delta[1])) {
        VectorClear2D(m_delta);
    }
}

bool ActorPath::Complete(const float *origin) const
{
    if (!m_HasCompleteLookahead) {
        return false;
    }

    if (fabs(origin[0] - m_path->point[0]) < 16.0f && fabs(origin[1] - m_path->point[1]) < 16.0f) {
        return true;
    }

    return false;
}

void ActorPath::TrimPathFromEnd(int nNodesPop)
{
    int iLastPos;

    iLastPos = m_path - m_pathpos;
    if (iLastPos < 0) {
        Clear();
        return;
    }

    m_pathpos -= nNodesPop;
    for (int i = 0; i < iLastPos; i++) {
        m_path[i] = m_path[i + nNodesPop];
    }
}

void ActorPath::Shorten(float fDistRemove)
{
    while (m_path->dist >= fDistRemove) {
        fDistRemove -= m_path->dist;
        TrimPathFromEnd(1);
        if (!m_pathpos) {
            return;
        }
    }

    m_path->point[0] += m_path->dir[0] * -fDistRemove;
    m_path->point[1] += m_path->dir[1] * -fDistRemove;
    // Fixed in OPM.
    //  This is a bug in mohaa as it can write past the end of the class instance
    //m_path->point[2] += m_path->dir[2] * -fDistRemove;
    m_path->dist -= fDistRemove;
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
    return m_path->point;
}

int ActorPath::Time(void) const
{
    return m_Time;
}

const float *ActorPath::CurrentDelta(void) const
{
    return m_delta;
}

bool ActorPath::IsAccurate(void) const
{
    if (m_pathpos == m_path) {
        return false;
    }

    if (!m_pathpos[-1].bAccurate) {
        return false;
    }

    if (!m_Side) {
        return false;
    }

    return true;
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
