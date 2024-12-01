/*
===========================================================================
Copyright (C) 2023-2024 the OpenMoHAA team

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

// tr_swipe.cpp -- swipe rendering

#include "tr_local.h"

#define MAX_SWIPE_POINTS 1024
#define MAX_SWIPES       32

typedef struct {
    vec3_t points[2];
    float  time;
} rswipepoint_t;

typedef struct {
    surfaceType_t surftype;
    float         life;
    float         time;
    int           lastrendtime;
    qhandle_t     shader;
    int           numswipes;
    rswipepoint_t swipes[MAX_SWIPE_POINTS];
} rendswipe_t;

static int         lastswipeframe;
static int         numswipes;
static rendswipe_t swipes[MAX_SWIPES];

/*
======================
RE_SwipeBegin
======================
*/
void RE_SwipeBegin(float thistime, float life, qhandle_t shader)
{
    rendswipe_t *swipe;

    if (tr.frameCount != lastswipeframe) {
        swipe     = &swipes[0];
        numswipes = 0;
    }

    if (numswipes >= MAX_SWIPES) {
        return;
    }

    swipe = &swipes[numswipes++];

    swipe->life         = life;
    swipe->time         = thistime;
    swipe->surftype     = SF_SWIPE;
    swipe->numswipes    = 0;
    swipe->lastrendtime = 0;
    swipe->shader       = shader;

    if (shader < 0 && life) {
        ri.Printf(PRINT_DEVELOPER, "RE_SwipeBegin: Invalid shader handle\n");
    }

    lastswipeframe = tr.frameCount;
}

/*
======================
RE_SwipePoint
======================
*/
void RE_SwipePoint(vec3_t point1, vec3_t point2, float time)
{
    rendswipe_t *swipe = &swipes[numswipes - 1];

    if (swipe->numswipes >= MAX_SWIPE_POINTS) {
        return;
    }

    VectorCopy(point1, swipe->swipes[swipe->numswipes - 1].points[0]);
    VectorCopy(point2, swipe->swipes[swipe->numswipes - 1].points[1]);
}

/*
======================
RE_SwipeEnd
======================
*/
void RE_SwipeEnd() {}

/*
======================
R_AddSwipeSurfaces
======================
*/
void R_AddSwipeSurfaces()
{
    shader_t *shader;
    int       at;

    if (!numswipes) {
        return;
    }

    tr.currentEntityNum = ENTITYNUM_WORLD;
    tr.shiftedEntityNum = tr.currentEntityNum << QSORT_ENTITYNUM_SHIFT;

    for (at = 0; at < numswipes; at++) {
        rendswipe_t *swipe     = &swipes[at];
        int          timedelta = 0;

        if (!swipe->numswipes) {
            continue;
        }

        if (swipe->lastrendtime) {
            timedelta = tr.refdef.time - swipe->lastrendtime;
        }
        swipe->lastrendtime = tr.refdef.time;

        if (timedelta) {
            swipe->time += timedelta;
        }

        shader = R_GetShaderByHandle(swipe->shader);
        R_AddDrawSurf(&swipe->surftype, shader, 0);
    }
}

/*
======================
RB_DrawSwipeSurface
======================
*/
void RB_DrawSwipeSurface(surfaceType_t *pswipe)
{
    int          i;
    float        oolife;
    rendswipe_t *swipe = (rendswipe_t *)pswipe;

    oolife = 1.0 / swipe->life;

    RB_CHECKOVERFLOW(swipe->numswipes * 2, swipe->numswipes * 6);

    RB_StreamBeginDrawSurf();

    for (i = 0; i < swipe->numswipes; i++) {
        rswipepoint_t *swipepoint = &swipe->swipes[i];
        float          alpha;
        float          f;

        f = 1.0 - (swipe->time - swipepoint->time) * oolife;
        if (f > 0 && swipe->time >= swipepoint->time) {
            alpha = f - Q_max(f - 1.0, 0);

            RB_Color4f(alpha, alpha, alpha, alpha);
            RB_Texcoord2f(i / (float)swipe->numswipes, 1.0);
            RB_Vertex3fv(swipepoint->points[0]);
            RB_Texcoord2f(i / (float)swipe->numswipes, 1.0);
            RB_Vertex3fv(swipepoint->points[1]);
        } else if (i == swipe->numswipes - 1) {
            swipe->numswipes = 0;
        }
    }

    RB_StreamEndDrawSurf();
}
