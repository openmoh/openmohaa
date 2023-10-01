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

// movegrid.h: Move grid
//

#include "movegrid.h"
#include "debuglines.h"

cMoveGrid::cMoveGrid(int x, int y, int z)
{
    m_iXRes = x;
    m_iYRes = y;
    m_iZRes = z;

    GridPoints = (gridpoint_t *)gi.Malloc(sizeof(gridpoint_t) * x * y * z);
}

cMoveGrid::~cMoveGrid()
{
    gi.Free(GridPoints);
}

void cMoveGrid::SetMoveInfo(vmove_t *vm)
{
    v = *vm;
}

void cMoveGrid::GetMoveInfo(vmove_t *vm)
{
    *vm = v;
}

gridpoint_t *cMoveGrid::GetGridPoint(int x, int y, int z)
{
    return &GridPoints[x * m_iZRes + y * m_iXRes + z];
}

void cMoveGrid::SetOrientation(const vec3_t *v)
{
    for (int i = 0; i < 3; i++) {
        VectorCopy(v[i], orientation[i]);
    }
}

void cMoveGrid::CalculateBoxPoints(void)
{
    int    x, y, z;
    int    rx, ry, rz;
    int    xdelta, ydelta, zdelta;
    Vector vBoxSize;

    vBoxSize = Vector(v.maxs) - Vector(v.mins);

    rx = (int)(vBoxSize.x / (float)m_iXRes);
    ry = (int)(vBoxSize.y / (float)m_iYRes);
    rz = (int)(vBoxSize.z / (float)m_iZRes);

    xdelta = ydelta = zdelta = 0;

    for (x = 0; x < m_iXRes; x++) {
        for (y = 0; y < m_iYRes; y++) {
            for (z = 0; z < m_iZRes; z++) {
                gridpoint_t *gridPoint = GetGridPoint(x, y, z);

                gridPoint->origin = Vector(xdelta - rx, ydelta - ry, zdelta + rz / 2);
                zdelta += rz;
            }
            zdelta = 0;
            ydelta += ry;
        }
        ydelta = 0;
        xdelta += rx;
    }

    for (x = 0; x < m_iXRes; x++) {
        for (y = 0; y < m_iYRes; y++) {
            for (z = 0; z < m_iZRes; z++) {
                Vector       vTmp;
                gridpoint_t *gridPoint = GetGridPoint(x, y, z);

                MatrixTransformVector(gridPoint->origin, orientation, vTmp);

                gridPoint->origin = vTmp;
                gridPoint->vm     = v;
                gridPoint->vm.vs  = &gridPoint->vs;

                if (rx >= ry) {
                    Vector vec;

                    vec = Vector(-ry / 2, -ry / 2, -rz / 2);
                    vec.copyTo(gridPoint->vm.mins);

                    vec = Vector(ry / 2, ry / 2, rz / 2);
                    vec.copyTo(gridPoint->vm.maxs);
                } else {
                    Vector vec;

                    vec = Vector(-rx / 2, -rx / 2, -rz / 2);
                    vec.copyTo(gridPoint->vm.mins);

                    vec = Vector(rx / 2, rx / 2, rz / 2);
                    vec.copyTo(gridPoint->vm.maxs);
                }
            }
        }
    }
}

void cMoveGrid::Move(void)
{
    gridpoint_t *gridPoint;
    int          x, y, z;
    Vector       vBoxSize;
    Vector       old_origin;
    float        best;
    float        bx, by, bz;
    Vector       bestdelta;

    old_origin = v.vs->origin;
    vBoxSize   = Vector(v.maxs) - Vector(v.mins);

    CalculateBoxPoints();

    for (x = 0; x < m_iXRes; x++) {
        for (y = 0; y < m_iYRes; y++) {
            for (z = 0; z < m_iZRes; z++) {
                gridPoint = GetGridPoint(x, y, z);

                gridPoint->origindelta = gridPoint->origin;
                gridPoint->origin += old_origin;
                gridPoint->origin.copyTo(gridPoint->vm.vs->origin);

                G_DebugBBox(gridPoint->origin, gridPoint->vm.mins, gridPoint->vm.maxs, 1, 1, 0, 1);

                VmoveSingle(&gridPoint->vm);

                gridPoint->neworigin = gridPoint->vm.vs->origin;
                gridPoint->changed   = gridPoint->neworigin - gridPoint->origin;
                gridPoint->newvel    = gridPoint->vm.vs->velocity;
                // set the ground entity
                gridPoint->groundentity = NULL;
                if (gridPoint->vm.vs->groundEntityNum != ENTITYNUM_NONE) {
                    gridPoint->groundentity = &g_entities[gridPoint->vm.vs->groundEntityNum];
                }
            }
        }
    }

    best = 99999;
    bx = by = bz = 0;

    for (x = 0; x < m_iXRes; x++) {
        for (y = 0; y < m_iYRes; y++) {
            for (z = 0; z < m_iZRes; z++) {
                gridPoint = GetGridPoint(x, y, z);
                float speed;

                speed = (gridPoint->changed - gridPoint->origindelta).length();
                if (speed < best) {
                    best = gridPoint->changed.length();
                    bx   = x;
                    by   = y;
                    bz   = z;
                }
            }
        }
    }

    gridPoint = GetGridPoint(bx, by, bz);
    bestdelta = gridPoint->neworigin - gridPoint->origindelta;
    bestdelta.copyTo(v.vs->origin);
    gridPoint->newvel.copyTo(v.vs->velocity);
}

qboolean cMoveGrid::CheckStuck(void)
{
    int     x;
    int     y;
    int     z;
    trace_t trace;

    for (x = 0; x < m_iXRes; x++) {
        for (y = 0; y < m_iYRes; y++) {
            for (z = 0; z < m_iZRes; z++) {
                gi.trace(
                    &trace, v.vs->origin, v.mins, v.maxs, v.vs->origin, v.vs->entityNum, v.tracemask, false, false
                );

                if (trace.allsolid || trace.startsolid || trace.fraction == 0.0f) {
                    return true;
                }
            }
        }
    }

    return false;
}

void cMoveGrid::Archive(Archiver& arc)
{
    arc.ArchiveInteger(&m_iXRes);
    arc.ArchiveInteger(&m_iYRes);
    arc.ArchiveInteger(&m_iZRes);
    arc.ArchiveRaw(&v, sizeof(vmove_t));
    arc.ArchiveVec3(orientation[0]);
    arc.ArchiveVec3(orientation[1]);
    arc.ArchiveVec3(orientation[2]);

    if (arc.Loading()) {
        if (GridPoints) {
            if (m_iXRes != 3 && m_iYRes != 3 && m_iZRes != 1) {
                gi.Free(GridPoints);
                GridPoints = (gridpoint_t *)gi.Malloc(sizeof(gridpoint_t) * m_iXRes * m_iYRes * m_iZRes);
            }
        }
    }
}
