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

/**
 * @file navigation_bsp_utils.cpp
 * @brief Utility functions to create windings from brushes.
 * 
 */

#include "g_local.h"
#include "navigation_bsp.h"
#include "../qcommon/qfiles.h"
#include "../qcommon/container.h"
#include "../qcommon/vector.h"
#include "../qcommon/cm_polylib.h"
#include "../script/scriptexception.h"

/*
=============
AllocWinding
=============
*/
winding_t *AllocWinding(int points)
{
    winding_t *w;
    int        s;

    s = sizeof(vec_t) * 3 * points + sizeof(int);
    w = (winding_t *)gi.Malloc(s);
    Com_Memset(w, 0, s);
    return w;
}

void FreeWinding(winding_t *w)
{
    if (*(unsigned *)w == 0xdeaddead) {
        Com_Error(ERR_FATAL, "FreeWinding: freed a freed winding");
    }
    *(unsigned *)w = 0xdeaddead;

    gi.Free(w);
}

/*
=================
BaseWindingForPlane
=================
*/
winding_t *BaseWindingForPlane(vec3_t normal, vec_t dist)
{
    int        i, x;
    vec_t      max, v;
    vec3_t     org, vright, vup;
    winding_t *w;

    // find the major axis

    max = -MAP_SIZE;
    x   = -1;
    for (i = 0; i < 3; i++) {
        v = fabs(normal[i]);
        if (v > max) {
            x   = i;
            max = v;
        }
    }
    if (x == -1) {
        Com_Error(ERR_DROP, "BaseWindingForPlane: no axis found");
    }

    VectorCopy(vec3_origin, vup);
    switch (x) {
    case 0:
    case 1:
        vup[2] = 1;
        break;
    case 2:
        vup[0] = 1;
        break;
    }

    v = DotProduct(vup, normal);
    VectorMA(vup, -v, normal, vup);
    VectorNormalize(vup);

    VectorScale(normal, dist, org);

    CrossProduct(vup, normal, vright);

    VectorScale(vup, MAP_SIZE, vup);
    VectorScale(vright, MAP_SIZE, vright);

    // project a really big	axis aligned box onto the plane
    w = AllocWinding(4);

    VectorSubtract(org, vright, w->p[0]);
    VectorAdd(w->p[0], vup, w->p[0]);

    VectorAdd(org, vright, w->p[1]);
    VectorAdd(w->p[1], vup, w->p[1]);

    VectorAdd(org, vright, w->p[2]);
    VectorSubtract(w->p[2], vup, w->p[2]);

    VectorSubtract(org, vright, w->p[3]);
    VectorSubtract(w->p[3], vup, w->p[3]);

    w->numpoints = 4;

    return w;
}

/*
=============
ChopWindingInPlace
=============
*/
void ChopWindingInPlace(winding_t **inout, vec3_t normal, vec_t dist, vec_t epsilon)
{
    winding_t   *in;
    vec_t        dists[MAX_POINTS_ON_WINDING + 4];
    int          sides[MAX_POINTS_ON_WINDING + 4];
    int          counts[3];
    static vec_t dot; // VC 4.2 optimizer bug if not static
    int          i, j;
    vec_t       *p1, *p2;
    vec3_t       mid;
    winding_t   *f;
    int          maxpts;

    in        = *inout;
    counts[0] = counts[1] = counts[2] = 0;

    // determine sides for each point
    for (i = 0; i < in->numpoints; i++) {
        dot = DotProduct(in->p[i], normal);
        dot -= dist;
        dists[i] = dot;
        if (dot > epsilon) {
            sides[i] = SIDE_FRONT;
        } else if (dot < -epsilon) {
            sides[i] = SIDE_BACK;
        } else {
            sides[i] = SIDE_ON;
        }
        counts[sides[i]]++;
    }
    sides[i] = sides[0];
    dists[i] = dists[0];

    if (!counts[0]) {
        FreeWinding(in);
        *inout = NULL;
        return;
    }
    if (!counts[1]) {
        return; // inout stays the same
    }

    maxpts = in->numpoints + 4; // cant use counts[0]+2 because
    // of fp grouping errors

    f = AllocWinding(maxpts);

    for (i = 0; i < in->numpoints; i++) {
        p1 = in->p[i];

        if (sides[i] == SIDE_ON) {
            VectorCopy(p1, f->p[f->numpoints]);
            f->numpoints++;
            continue;
        }

        if (sides[i] == SIDE_FRONT) {
            VectorCopy(p1, f->p[f->numpoints]);
            f->numpoints++;
        }

        if (sides[i + 1] == SIDE_ON || sides[i + 1] == sides[i]) {
            continue;
        }

        // generate a split point
        p2 = in->p[(i + 1) % in->numpoints];

        dot = dists[i] / (dists[i] - dists[i + 1]);
        for (j = 0; j < 3; j++) { // avoid round off error when possible
            if (normal[j] == 1) {
                mid[j] = dist;
            } else if (normal[j] == -1) {
                mid[j] = -dist;
            } else {
                mid[j] = p1[j] + dot * (p2[j] - p1[j]);
            }
        }

        VectorCopy(mid, f->p[f->numpoints]);
        f->numpoints++;
    }

    if (f->numpoints > maxpts) {
        Com_Error(ERR_DROP, "ClipWinding: points exceeded estimate");
    }
    if (f->numpoints > MAX_POINTS_ON_WINDING) {
        Com_Error(ERR_DROP, "ClipWinding: MAX_POINTS_ON_WINDING");
    }

    FreeWinding(in);
    *inout = f;
}

#define SNAP_EPSILON 0.01

void SnapWeldVector(vec3_t a, vec3_t b, vec3_t out)
{
    int   i;
    vec_t ai, bi, outi;

    /* dummy check */
    if (a == NULL || b == NULL || out == NULL) {
        return;
    }

    /* do each element */
    for (i = 0; i < 3; i++) {
        /* round to integer */
        ai = Q_rint(a[i]);
        bi = Q_rint(a[i]);

        /* prefer exact integer */
        if (ai == a[i]) {
            out[i] = a[i];
        } else if (bi == b[i]) {
            out[i] = b[i];
        }

        /* use nearest */
        else if (fabs(ai - a[i]) < fabs(bi < b[i])) {
            out[i] = a[i];
        } else {
            out[i] = b[i];
        }

        /* snap */
        outi = Q_rint(out[i]);
        if (fabs(outi - out[i]) <= SNAP_EPSILON) {
            out[i] = outi;
        }
    }
}

#define DEGENERATE_EPSILON 0.1

qboolean FixWinding(winding_t *w)
{
    qboolean valid = qtrue;
    int      i, j, k;
    vec3_t   vec;
    float    dist;

    /* dummy check */
    if (!w) {
        return qfalse;
    }

    /* check all verts */
    for (i = 0; i < w->numpoints; i++) {
        /* don't remove points if winding is a triangle */
        if (w->numpoints == 3) {
            return valid;
        }

        /* get second point index */
        j = (i + 1) % w->numpoints;

        /* degenerate edge? */
        VectorSubtract(w->p[i], w->p[j], vec);
        dist = VectorLength(vec);
        if (dist < DEGENERATE_EPSILON) {
            valid = qfalse;
            //Sys_FPrintf( SYS_VRB, "WARNING: Degenerate winding edge found, fixing...\n" );

            /* create an average point (ydnar 2002-01-26: using nearest-integer weld preference) */
            SnapWeldVector(w->p[i], w->p[j], vec);
            VectorCopy(vec, w->p[i]);
            //VectorAdd( w->p[ i ], w->p[ j ], vec );
            //VectorScale( vec, 0.5, w->p[ i ] );

            /* move the remaining verts */
            for (k = i + 2; k < w->numpoints; k++) {
                VectorCopy(w->p[k], w->p[k - 1]);
            }
            w->numpoints--;
        }
    }

    /* one last check and return */
    if (w->numpoints < 3) {
        valid = qfalse;
    }
    return valid;
}

qboolean G_PlaneFromPoints(vec4_t plane, vec3_t a, vec3_t b, vec3_t c)
{
    vec3_t d1, d2;

    VectorSubtract(b, a, d1);
    VectorSubtract(c, a, d2);
    CrossProduct(d2, d1, plane);
    if (VectorNormalize(plane) == 0) {
        return qfalse;
    }

    plane[3] = DotProduct(a, plane);
    return qtrue;
}