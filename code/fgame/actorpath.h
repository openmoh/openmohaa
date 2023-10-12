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

// actorpath.h: Actor path

#pragma once

#define MIN_FALLHEIGHT 18
#define MAX_FALLHEIGHT 1024

class ActorPath
{
    // path list
    PathInfo *m_path;
    int       m_pathlen;

    // path position
    PathInfo *m_pathpos;

    // starting path position
    PathInfo *m_startpathpos;
    vec2_t    m_delta;
    bool      m_Side;
    int       m_Time;
    float     m_TotalDist;
    short int m_FallHeight;
    bool      m_HasCompleteLookahead;
    float     m_fLookAhead;
    bool      m_bChangeLookAhead;

private:
    float PathLookAhead(float total_area, Vector& end, float *origin);

public:
    ActorPath();
    ~ActorPath();

    void Clear(void);
    bool DoesTheoreticPathExist(
        float *start, float *end, class SimpleActor *ent, float maxPath, float *vLeashHome, float fLeashDistSquared
    );
    void FindPath(float *start, float *end, Entity *ent, float maxPath, float *vLeashHome, float fLeashDistSquared);
    void FindPathAway(
        float  *start,
        float  *avoid,
        float  *vPreferredDir,
        Entity *ent,
        float   fMinSafeDist,
        float  *vLeashHome,
        float   fLeashDistSquared
    );
    void FindPathNear(
        float  *start,
        float  *nearby,
        Entity *ent,
        float   maxPath,
        float   fRadiusSquared,
        float  *vLeashHome,
        float   fLeashDistSquared
    );
    void      ReFindPath(float *start, Entity *ent);
    void      UpdatePos(float *origin, float fNodeRadius = 0.0f);
    bool      Complete(const float *origin) const;
    PathInfo *StartNode(void) const;
    PathInfo *CurrentNode(void) const;
    int       CurrentNodeIndex(void) const;
    PathInfo *NextNode(void) const;
    PathInfo *LastNode(void) const;
    Vector    CurrentPathDir(void) const;
    float    *CurrentPathGoal(void) const;
    int       Time(void) const;
    Vector    CurrentDelta(void) const;
    bool      IsAccurate(void) const;
    float     TotalDist(void) const;
    void      SetFallHeight(float fHeight);
    float     GetFallHeight(void) const;
    void      TrimPathFromEnd(int nNodesPop);
    void      Shorten(float fDistRemove);
    bool      HasCompleteLookahead(void) const;
    bool      IsSide(void) const;
    void      ForceShortLookahead(void);

    virtual void Archive(Archiver& arc);
};

inline void ActorPath::Archive(Archiver& arc)
{
    int pos;
    int startpathpos;

    arc.ArchiveInteger(&m_pathlen);

    if (m_pathlen) {
        if (arc.Loading()) {
            m_path = new PathInfo[m_pathlen];
        }

        if (m_pathlen > 0) {
            for (int i = 0; i < m_pathlen; i++) {
                m_path->Archive(arc);
            }
        }

        if (!arc.Saving()) {
            arc.ArchiveInteger(&pos);
            if (pos == -1) {
                m_pathpos = NULL;
            } else {
                m_pathpos = &m_path[pos];
            }

            arc.ArchiveInteger(&startpathpos);
            if (startpathpos == -1) {
                m_startpathpos = NULL;
            } else {
                m_startpathpos = &m_path[pos];
            }
        } else {
            if (m_pathpos) {
                pos = m_pathpos - m_path;
            } else {
                pos = -1;
            }

            arc.ArchiveInteger(&pos);

            if (m_startpathpos) {
                pos = m_startpathpos - m_path;
            } else {
                pos = -1;
            }

            arc.ArchiveInteger(&pos);
        }
    } else if (arc.Loading()) {
        m_pathpos = 0;
    }

    arc.ArchiveVec2(m_delta);
    arc.ArchiveBool(&m_Side);
    arc.ArchiveInteger(&m_Time);
    arc.ArchiveFloat(&m_TotalDist);
    arc.ArchiveShort(&m_FallHeight);
    arc.ArchiveShort(&m_FallHeight);
    arc.ArchiveBool(&m_HasCompleteLookahead);
    arc.ArchiveFloat(&m_fLookAhead);
    arc.ArchiveBool(&m_bChangeLookAhead);
}
