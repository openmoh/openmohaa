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
 * @file navigation_bsp_load_brush.cpp
 * @brief Provide utility functions to create windings from brushes, so surface
 * can be created from them.
 * 
 */

#include "g_local.h"
#include "navigation_bsp.h"

/*
=================
cbrushside_t::cbrushside_t
=================
*/
cbrushside_t::cbrushside_t()
    : winding(NULL)
{}

/*
=================
cbrushside_t::~cbrushside_t
=================
*/
cbrushside_t::~cbrushside_t()
{
    if (winding) {
        FreeWinding(winding);
    }
}

/*
============
BoundBrush
============
*/
qboolean BoundBrush(cbrush_t& brush)
{
    int        i, j;
    winding_t *w;

    ClearBounds(brush.bounds[0], brush.bounds[1]);
    for (i = 0; i < brush.numsides; i++) {
        w = brush.sides[i].winding;
        if (w == NULL) {
            continue;
        }
        for (j = 0; j < w->numpoints; j++) {
            AddPointToBounds(w->p[j], brush.bounds[0], brush.bounds[1]);
        }
    }

    for (i = 0; i < 3; i++) {
        if (brush.bounds[0][i] < MIN_WORLD_COORD || brush.bounds[1][i] > MAX_WORLD_COORD
            || brush.bounds[0][i] >= brush.bounds[1][i]) {
            return qfalse;
        }
    }

    return qtrue;
}

/*
============
CreateBrushWindings
============
*/
qboolean CreateBrushWindings(const Container<cplane_t>& planes, cbrush_t& brush)
{
    int           i, j;
    winding_t    *w;
    cbrushside_t *side;
    cplane_t     *plane;

    /* walk the list of brush sides */
    for (i = 0; i < brush.numsides; i++) {
        /* get side and plane */
        side  = &brush.sides[i];
        plane = side->plane;

        /* make huge winding */
        w = BaseWindingForPlane(plane->normal, plane->dist);

        /* walk the list of brush sides */
        for (j = 0; j < brush.numsides && w != NULL; j++) {
            if (i == j) {
                continue;
            }
            if (brush.sides[j].planenum == (brush.sides[i].planenum ^ 1)) {
                continue; /* back side clipaway */
            }
            plane = (cplane_t *)&planes[brush.sides[j].planenum ^ 1];
            ChopWindingInPlace(&w, plane->normal, plane->dist, 0); // CLIP_EPSILON );

            FixWinding(w);
        }

        if (side->winding) {
            FreeWinding(side->winding);
        }
        /* set side winding */
        side->winding = w;
    }

    /* find brush bounds */
    return BoundBrush(brush);
}

/*
============
FanFaceSurface
============
*/
void FanFaceSurface(navSurface_t& surface, const winding_t *winding, size_t baseVertex)
{
    int           i, j, k, a, b, c;
    navVertice_t *verts, centroid, *dv;
    double        iv;
    size_t        numVerts;

    for (i = 0; i < winding->numpoints; i++) {
        dv = &surface.vertices.ObjectAt(baseVertex + i + 1);
        centroid.xyz += dv->xyz;
    }

    iv = 1.0f / winding->numpoints;
    centroid.xyz *= iv;

    surface.vertices.InsertObjectAt(1, centroid);
    numVerts = surface.vertices.NumObjects() - baseVertex;

    for (i = 1; i < numVerts; i++) {
        a = 0;
        b = i;
        c = (i + 1) % numVerts;
        c = c ? c : 1;

        surface.AddIndex(baseVertex + a);
        surface.AddIndex(baseVertex + b);
        surface.AddIndex(baseVertex + c);
    }
}

#define COLINEAR_AREA 10

/*
============
IsTriangleDegenerate
============
*/
qboolean IsTriangleDegenerate(const vec3_t *points, int a, int b, int c)
{
    vec3_t v1, v2, v3;
    float  d;

    VectorSubtract(points[b], points[a], v1);
    VectorSubtract(points[c], points[a], v2);
    CrossProduct(v1, v2, v3);
    d = VectorLength(v3);

    // assume all very small or backwards triangles will cause problems
    if (d < COLINEAR_AREA) {
        return qtrue;
    }

    return qfalse;
}

#define MAX_INDEXES 1024

/*
============
G_StripFaceSurface
============
*/
void G_StripFaceSurface(navSurface_t& surface, const winding_t *winding)
{
    int          i, r, least, rotate, ni;
    int          numIndexes;
    int          a, b, c;
    const vec_t *v1, *v2;
    int          indexes[MAX_INDEXES];
    size_t       baseVertex;

    baseVertex = surface.vertices.NumObjects();

    for (i = 0; i < winding->numpoints; i++) {
        surface.AddVertice(winding->p[i]);
    }

    if (winding->numpoints == 3) {
        surface.AddIndex(baseVertex + 0);
        surface.AddIndex(baseVertex + 1);
        surface.AddIndex(baseVertex + 2);
        return;
    }

    least = 0;
    for (i = 0; i < winding->numpoints; i++) {
        /* get points */
        v1 = winding->p[i];
        v2 = winding->p[least];

        /* compare */
        if (v1[0] < v2[0] || (v1[0] == v2[0] && v1[1] < v2[1]) || (v1[0] == v2[0] && v1[1] == v2[1] && v1[2] < v2[2])) {
            least = i;
        }
    }

    /* determine the triangle strip order */
    numIndexes = (winding->numpoints - 2) * 3;

    ni = 0;
    /* try all possible orderings of the points looking for a non-degenerate strip order */
    for (r = 0; r < winding->numpoints; r++) {
        /* set rotation */
        rotate = (r + least) % winding->numpoints;

        /* walk the winding in both directions */
        for (ni = 0, i = 0; i < winding->numpoints - 2 - i; i++) {
            /* make indexes */
            a = (winding->numpoints - 1 - i + rotate) % winding->numpoints;
            b = (i + rotate) % winding->numpoints;
            c = (winding->numpoints - 2 - i + rotate) % winding->numpoints;

            /* test this triangle */
            if (winding->numpoints > 4 && IsTriangleDegenerate(winding->p, a, b, c)) {
                break;
            }
            indexes[ni++] = a;
            indexes[ni++] = b;
            indexes[ni++] = c;

            /* handle end case */
            if (i + 1 != winding->numpoints - 1 - i) {
                /* make indexes */
                a = (winding->numpoints - 2 - i + rotate) % winding->numpoints;
                b = (i + rotate) % winding->numpoints;
                c = (i + 1 + rotate) % winding->numpoints;

                /* test triangle */
                if (winding->numpoints > 4 && IsTriangleDegenerate(winding->p, a, b, c)) {
                    break;
                }
                indexes[ni++] = a;
                indexes[ni++] = b;
                indexes[ni++] = c;
            }
        }

        /* valid strip? */
        if (ni == numIndexes) {
            break;
        }
    }

    /* if any triangle in the strip is degenerate, render from a centered fan point instead */
    if (ni < numIndexes) {
        FanFaceSurface(surface, winding, baseVertex);
        return;
    }

    for (i = 0; i < numIndexes; i++) {
        surface.AddIndex(baseVertex + indexes[i]);
    }
}
