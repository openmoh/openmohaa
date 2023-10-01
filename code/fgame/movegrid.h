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

#pragma once

#include "g_local.h"
#include "../qcommon/class.h"
#include "../fgame/archive.h"

typedef struct vehicleState_s {
    vec3_t   origin;
    vec3_t   velocity;
    int      groundEntityNum;
    qboolean walking;
    qboolean groundPlane;
    trace_t  groundTrace;
    int      entityNum;
    vec2_t   desired_dir;
    qboolean hit_obstacle;
    float    hit_origin[3];
    vec3_t   obstacle_normal;
    qboolean useGravity;
} vehicleState_t;

typedef struct vmove_s {
    vehicleState_t *vs;
    float           frametime;
    float           desired_speed;
    int             tracemask;
    int             numtouch;
    int             touchents[32];
    vec3_t          mins;
    vec3_t          maxs;
} vmove_t;

typedef struct gridpoint_s {
    Vector         origin;
    Vector         neworigin;
    Vector         origindelta;
    Vector         changed;
    Vector         newvel;
    gentity_t     *groundentity;
    vmove_t        vm;
    vehicleState_t vs;
} gridpoint_t;

class cMoveGrid : public Class
{
public:
    gridpoint_t *GridPoints;

private:
    vmove_t v;
    vec3_t  orientation[3];
    int     m_iXRes;
    int     m_iYRes;
    int     m_iZRes;

public:
    cMoveGrid(int x, int y, int z);
    virtual ~cMoveGrid();

    void         SetOrientation(const vec3_t *v);
    void         SetMoveInfo(vmove_t *vm);
    void         CalculateBoxPoints(void);
    gridpoint_t *GetGridPoint(int x, int y, int z);
    void         Move(void);
    qboolean     CheckStuck(void);
    void         GetMoveInfo(vmove_t *vm);
    void         Archive(Archiver        &arc);
};
