/*
===========================================================================
Copyright (C) 2008 Leszek Godlewski

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

// cm_terain.c : LOD Terrain support
// so many headaches

#include "cm_local.h"
#include "cm_terrain.h"

typedef struct varnodeIndex_s {
    short unsigned int iTreeAndMask;
    short unsigned int iNode;
} varnodeIndex_t;

typedef struct worknode_s {
    int i0;
    int j0;
    int i1;
    int j1;
    int i2;
    int j2;
} worknode_t;

typedef struct pointtrace_s {
    traceWork_t      *tw;
    terrainCollide_t *tc;
    vec3_t            vStart;
    vec3_t            vEnd;
    int               i;
    int               j;
    float             fSurfaceClipEpsilon;
} pointtrace_t;

varnodeIndex_t g_vni[2][8][8][2];
// Changed in OPM
//  Use stack instead of storing it globally
//static pointtrace_t g_trace;

static int modeTable[] = {2, 2, 5, 6, 4, 3, 0, 0};

/*
=================
CM_SignbitsForNormal
=================
Copied over from cm_patch.c
*/
static int CM_SignbitsForNormal(vec3_t normal)
{
    int bits, j;

    bits = 0;
    for (j = 0; j < 3; j++) {
        if (normal[j] < 0) {
            bits |= 1 << j;
        }
    }
    return bits;
}

/*
====================
CM_CalculateTerrainIndices
====================
*/
void CM_CalculateTerrainIndices(worknode_t *worknode, int iDiagonal, int iTree)
{
    int             i;
    int             i2;
    int             j2;
    varnodeIndex_t *vni;

    for (i = 0; i <= 30; i++) {
        i2                         = worknode[i + 1].i0 + worknode[i + 1].i1;
        j2                         = worknode[i + 1].j0 + worknode[i + 1].j1;
        worknode[i * 2 + 2].i0     = worknode[i + 1].i1;
        worknode[i * 2 + 2].j0     = worknode[i + 1].j1;
        worknode[i * 2 + 2].i1     = worknode[i + 1].i2;
        worknode[i * 2 + 2].j1     = worknode[i + 1].j2;
        worknode[i * 2 + 2].i2     = i2 >> 1;
        worknode[i * 2 + 2].j2     = j2 >> 1;
        worknode[i * 2 + 2 + 1].i0 = worknode[i + 1].i2;
        worknode[i * 2 + 2 + 1].j0 = worknode[i + 1].j2;
        worknode[i * 2 + 2 + 1].i1 = worknode[i + 1].i0;
        worknode[i * 2 + 2 + 1].j1 = worknode[i + 1].j0;
        worknode[i * 2 + 2 + 1].i2 = i2 >> 1;
        worknode[i * 2 + 2 + 1].j2 = j2 >> 1;
    }

    for (i = 32; i < 64; i++) {
        i2 = (worknode[i].i0 + worknode[i].i1) >> 1;
        j2 = (worknode[i].j0 + worknode[i].j1) >> 1;

        if (worknode[i].i0 == worknode[i].i1) {
            if (worknode[i].j0 <= worknode[i].j1) {
                vni               = &g_vni[iDiagonal][i2][j2][1];
                vni->iNode        = i - 1;
                vni->iTreeAndMask = iTree | 0x2000;

                vni               = &g_vni[iDiagonal][i2][j2 - 1][0];
                vni->iNode        = i - 1;
                vni->iTreeAndMask = iTree | 0x1000;
            } else {
                vni               = &g_vni[iDiagonal][i2 - 1][j2][1];
                vni->iNode        = i - 1;
                vni->iTreeAndMask = iTree | 0x1000;

                vni               = &g_vni[iDiagonal][i2 - 1][j2 - 1][0];
                vni->iNode        = i - 1;
                vni->iTreeAndMask = iTree | 0x2000;
            }
        } else {
            if (worknode[i].i0 <= worknode[i].i1) {
                vni               = &g_vni[iDiagonal][i2][j2 - 1][0];
                vni->iNode        = i - 1;
                vni->iTreeAndMask = iTree | 0x2000;

                vni               = &g_vni[iDiagonal][i2 - 1][j2 - 1][0];
                vni->iNode        = i - 1;
                vni->iTreeAndMask = iTree | 0x1000;
            } else {
                vni               = &g_vni[iDiagonal][i2][j2][1];
                vni->iNode        = i - 1;
                vni->iTreeAndMask = iTree | 0x1000;

                vni               = &g_vni[iDiagonal][i2 - 1][j2][1];
                vni->iNode        = i - 1;
                vni->iTreeAndMask = iTree | 0x2000;
            }
        }
    }
}

/*
====================
CM_PrepareGenerateTerrainCollide
====================
*/
void CM_PrepareGenerateTerrainCollide(void)
{
    worknode_t worknode[64];

    memset(&g_vni, 0, sizeof(g_vni));

    worknode[1].i0 = 8;
    worknode[1].j0 = 8;
    worknode[1].i1 = 0;
    worknode[1].j1 = 0;
    worknode[1].i2 = 0;
    worknode[1].j2 = 8;

    CM_CalculateTerrainIndices(worknode, 0, 0);

    worknode[1].i0 = 0;
    worknode[1].j0 = 0;
    worknode[1].i1 = 8;
    worknode[1].j1 = 8;
    worknode[1].i2 = 8;
    worknode[1].j2 = 0;

    CM_CalculateTerrainIndices(worknode, 0, 1);

    worknode[1].i0 = 8;
    worknode[1].j0 = 0;
    worknode[1].i1 = 0;
    worknode[1].j1 = 8;
    worknode[1].i2 = 8;
    worknode[1].j2 = 8;

    CM_CalculateTerrainIndices(worknode, 1, 0);

    worknode[1].i0 = 0;
    worknode[1].j0 = 8;
    worknode[1].i1 = 8;
    worknode[1].j1 = 0;
    worknode[1].i2 = 0;
    worknode[1].j2 = 0;

    CM_CalculateTerrainIndices(worknode, 1, 1);
}

void CM_PickTerrainSquareMode(terrainCollideSquare_t *square, vec3_t vTest, int i, int j, cTerraPatch_t *patch)
{
    int             flags0, flags1;
    varnodeIndex_t *vni;

    if (patch->flags & TERPATCH_NEIGHBOR) {
        vni = g_vni[1][i][j];
    } else {
        vni = g_vni[0][i][j];
    }

    if ((vni[0].iTreeAndMask & patch->varTree[vni[0].iTreeAndMask & 1][vni[0].iNode].flags & 0xFFFE) != 0) {
        flags0 = 2;
    } else {
        flags0 = 0;
    }

    if ((vni[1].iTreeAndMask & patch->varTree[vni[1].iTreeAndMask & 1][vni[1].iNode].flags & 0xFFFFFFFE) != 0) {
        flags1 = 4;
    } else {
        flags1 = 0;
    }

    square->eMode = modeTable[(j + i) & 1 | flags0 | flags1];

    if (square->eMode == 2) {
        if (DotProduct(vTest, square->plane[0]) < square->plane[0][3]) {
            square->eMode = 1;
        }
    } else if (square->eMode == 5 || square->eMode == 6) {
        VectorCopy(square->plane[1], square->plane[0]);
        square->plane[0][3] = square->plane[1][3];
    }
}

/*
====================
CM_GenerateTerPatchCollide
====================
*/
void CM_GenerateTerrainCollide(cTerraPatch_t *patch, terrainCollide_t *tc)
{
    int                     i;
    int                     j;
    int                     x0, y0, z0;
    float                   fMaxHeight;
    float                   heightmap[9][9];
    terrainCollideSquare_t *square;
    vec3_t                  v1;
    vec3_t                  v2;
    vec3_t                  v3;
    vec3_t                  v4;

    x0 = (patch->x << 6);
    y0 = (patch->y << 6);
    z0 = (patch->iBaseHeight);

    fMaxHeight = z0;

    for (j = 0; j < 9; j++) {
        for (i = 0; i < 9; i++) {
            heightmap[i][j] = (float)(z0 + 2 * patch->heightmap[j * 9 + i]);
        }
    }

    for (j = 0; j < 8; j++) {
        for (i = 0; i < 8; i++) {
            v1[0] = ((i << 6) + x0);
            v1[1] = ((j << 6) + y0);
            v1[2] = heightmap[i][j];

            v2[0] = ((i << 6) + x0) + 64;
            v2[1] = ((j << 6) + y0);
            v2[2] = heightmap[i + 1][j];

            v3[0] = ((i << 6) + x0) + 64;
            v3[1] = ((j << 6) + y0) + 64;
            v3[2] = heightmap[i + 1][j + 1];

            v4[0] = ((i << 6) + x0);
            v4[1] = ((j << 6) + y0) + 64;
            v4[2] = heightmap[i][j + 1];

            if (fMaxHeight < v1[2]) {
                fMaxHeight = v1[2];
            }

            if (fMaxHeight < v2[2]) {
                fMaxHeight = v2[2];
            }

            if (fMaxHeight < v3[2]) {
                fMaxHeight = v3[2];
            }

            if (fMaxHeight < v4[2]) {
                fMaxHeight = v4[2];
            }

            square = &tc->squares[i][j];

            if ((i + j) & 1) {
                if (patch->flags & TERPATCH_FLIP) {
                    CM_PlaneFromPoints(square->plane[0], v4, v2, v3);
                    CM_PlaneFromPoints(square->plane[1], v2, v4, v1);
                } else {
                    CM_PlaneFromPoints(square->plane[0], v2, v4, v3);
                    CM_PlaneFromPoints(square->plane[1], v4, v2, v1);
                }
                CM_PickTerrainSquareMode(square, v1, i, j, patch);
            } else {
                if (patch->flags & TERPATCH_FLIP) {
                    CM_PlaneFromPoints(square->plane[0], v1, v3, v4);
                    CM_PlaneFromPoints(square->plane[1], v3, v1, v2);
                } else {
                    CM_PlaneFromPoints(square->plane[0], v3, v1, v4);
                    CM_PlaneFromPoints(square->plane[1], v1, v3, v2);
                }
                CM_PickTerrainSquareMode(square, v2, i, j, patch);
            }
        }
    }

    tc->vBounds[0][0] = x0;
    tc->vBounds[0][1] = y0;
    tc->vBounds[0][2] = z0;
    tc->vBounds[1][0] = (x0 + 512);
    tc->vBounds[1][1] = (y0 + 512);
    tc->vBounds[1][2] = fMaxHeight;
}

/*
====================
CM_CheckTerrainPlane
====================
*/
float CM_CheckTerrainPlane(const pointtrace_t *pt, vec4_t plane)
{
    float d1, d2;
    float f;

    d1 = DotProduct(pt->vStart, plane) - plane[3];
    d2 = DotProduct(pt->vEnd, plane) - plane[3];

    // if completely in front of face, no intersection with the entire brush
    if (d1 > 0 && (d2 >= SURFACE_CLIP_EPSILON || d2 >= d1)) {
        return 1;
    }

    if (d1 <= 0 && d2 <= 0) {
        if (d1 >= -32) {
            return 0;
        }

        if (d2 >= -32) {
            return 0;
        }
        return 1;
    }

#if 0
    if (d1 <= d2) {
        return 1;
    }

    f = (d1 - SURFACE_CLIP_EPSILON) / (d1 - d2);
    if (f < 0) {
        f = 0;
    }

    return f;
#endif

    // Fixed in OPM
    //  Also check when leaving the terrain (usually from below).
    // crosses face
    if (d1 > d2) { // enter
        f = (d1 - SURFACE_CLIP_EPSILON) / (d1 - d2);
        if (f < 0) {
            f = 0;
        }
    } else { // leave
        f = (d1 + SURFACE_CLIP_EPSILON) / (d1 - d2);
        if (f > 1) {
            f = 1;
        }
    }

    return f;
}

/*
====================
CM_CheckTerrainTriSpherePoint
====================
*/
float CM_CheckTerrainTriSpherePoint(const pointtrace_t *pt, vec3_t v)
{
    vec3_t vDelta, vDir;
    float  fLenSq;
    float  fRadSq;
    float  fA, fB;
    float  fDiscr;
    float  fFrac;
    float  fSq;
    float  f;

    VectorSubtract(pt->vStart, v, vDir);

    fRadSq = Square(sphere.radius);
    fLenSq = VectorLengthSquared(vDir);

    if (fLenSq <= fRadSq) {
        pt->tw->trace.startsolid = qtrue;
        pt->tw->trace.allsolid   = qtrue;
        return 0;
    }

    VectorSubtract(pt->vEnd, pt->vStart, vDelta);

    fA     = VectorLengthSquared(vDelta);
    fB     = DotProduct(vDelta, vDir);
    fDiscr = fB * fB - (fLenSq - fRadSq) * fA;

    if (fDiscr <= 0.0f) {
        return pt->tw->trace.fraction;
    }

    fSq = sqrt(fDiscr);

    if (fA > 0) {
        fFrac = (-fB - fSq) / fA - pt->fSurfaceClipEpsilon;

        if (fFrac >= 0.0f && fFrac <= pt->tw->trace.fraction) {
            return fFrac;
        }

        fFrac = -fB + fSq;
    } else {
        fFrac = (-fB + fSq) / fA - pt->fSurfaceClipEpsilon;

        if (fFrac >= 0.0f && fFrac <= pt->tw->trace.fraction) {
            return fFrac;
        }

        fFrac = -fB - fSq;
    }

    f = fFrac / fA - pt->fSurfaceClipEpsilon;
    if (f < 0 || f > pt->tw->trace.fraction) {
        f = pt->tw->trace.fraction;
    }

    return f;
}

/*
====================
CM_CheckTerrainTriSphereCorner
====================
*/
float CM_CheckTerrainTriSphereCorner(const pointtrace_t *pt, vec4_t plane, float x0, float y0, int i, int j)
{
    vec3_t v;

    v[0] = ((i << 6) + x0);
    v[1] = ((j << 6) + y0);
    v[2] = (plane[3] - (v[1] * plane[1] + v[0] * plane[0])) / plane[2];

    return CM_CheckTerrainTriSpherePoint(pt, v);
}

/*
====================
CM_CheckTerrainTriSphereEdge
====================
*/
float CM_CheckTerrainTriSphereEdge(
    const pointtrace_t *pt, float *plane, float x0, float y0, int i0, int j0, int i1, int j1
)
{
    vec3_t v0, v1;
    float  fScale;
    float  fRadSq;
    float  S, T;
    vec3_t vDeltaStart;
    vec3_t vDirEdge;
    vec3_t vDirTrace;
    float  fSFromT_Const;
    float  fSFromT_Scale;
    vec3_t vRFromT_Const;
    vec3_t vRFromT_Scale;

    // junk variable(s) as usual
    float fLengthSq, fDot;
    float fFrac, fFracClip;

    fScale = 1.0 / plane[2];

    v0[0] = (i0 << 6) + x0;
    v0[1] = (j0 << 6) + y0;
    v0[2] = (plane[3] - DotProduct2D(v0, plane)) * fScale;

    v1[0] = (i1 << 6) + x0;
    v1[1] = (j1 << 6) + y0;
    v1[2] = (plane[3] - DotProduct2D(v1, plane)) * fScale;

    VectorSubtract(pt->vStart, v0, vDirTrace);
    VectorSubtract(v1, v0, vDirEdge);
    VectorSubtract(pt->vEnd, pt->vStart, vDeltaStart);

    fScale = 1.0 / VectorLengthSquared(vDirEdge);
    S      = DotProduct(vDirTrace, vDirEdge) * fScale;
    T      = DotProduct(vDeltaStart, vDirEdge) * fScale;

    VectorMA(vDirTrace, -S, vDirEdge, vRFromT_Const);
    VectorMA(vDeltaStart, -T, vDirEdge, vRFromT_Scale);

    fRadSq    = Square(sphere.radius);
    fLengthSq = VectorLengthSquared(vRFromT_Const);

    if (fLengthSq <= fRadSq) {
        if (S < 0 || S > 1) {
            return CM_CheckTerrainTriSpherePoint(pt, v0);
        }

        pt->tw->trace.startsolid = qtrue;
        pt->tw->trace.allsolid   = qtrue;
        return 1;
    }

    fDot          = DotProduct(vRFromT_Scale, vRFromT_Const);
    fSFromT_Scale = VectorLengthSquared(vRFromT_Scale);
    fSFromT_Const = Square(fDot) - (fLengthSq - fRadSq) * fSFromT_Scale;

    if (fSFromT_Const <= 0) {
        return pt->tw->trace.fraction;
    }

    if (fSFromT_Scale > 0) {
        fFrac = (-fDot - sqrt(fSFromT_Const)) / fSFromT_Scale;
    } else {
        fFrac = (-fDot + sqrt(fSFromT_Const)) / fSFromT_Scale;
    }

    if (fFrac <= 0) {
        return pt->tw->trace.fraction;
    }

    fFracClip = fFrac - pt->fSurfaceClipEpsilon;
    if (fFracClip >= pt->tw->trace.fraction) {
        return pt->tw->trace.fraction;
    }

    fFrac = fFrac * T + S;

    if (fFrac < 0) {
        return CM_CheckTerrainTriSpherePoint(pt, v0);
    }

    if (fFrac > 1) {
        return CM_CheckTerrainTriSpherePoint(pt, v1);
    }

    if (fFracClip < 0) {
        fFracClip = 0;
    }
    return fFracClip;
}

/*
====================
CM_CheckTerrainTriSphere
====================
*/
float CM_CheckTerrainTriSphere(pointtrace_t *pt, float x0, float y0, int iPlane)
{
    float   *plane;
    float    fMaxFraction;
    float    d1, d2;
    float    fSpherePlane;
    int      eMode;
    qboolean bFitsX, bFitsY;
    qboolean bFitsDiag;
    int      iX[3];
    int      iY[3];

    plane = pt->tc->squares[pt->i][pt->j].plane[iPlane];
    d1    = DotProduct(pt->vStart, plane) - plane[3];
    d2    = DotProduct(pt->vEnd, plane) - plane[3];

    // if completely in front of face, no intersection with the entire brush
    if (d1 > sphere.radius && (d2 >= sphere.radius + SURFACE_CLIP_EPSILON || d2 >= d1)) {
        return pt->tw->trace.fraction;
    }

    // if it doesn't cross the plane, the plane isn't relevent
    if (d1 <= -sphere.radius && d2 <= -sphere.radius) {
        return pt->tw->trace.fraction;
    }

#if 0
    if (d1 <= d2) {
        return pt->tw->trace.fraction;
    }

    fMaxFraction            = SURFACE_CLIP_EPSILON / (d1 - d2);
    pt->fSurfaceClipEpsilon = fMaxFraction;
    fSpherePlane            = (d1 - sphere.radius) / (d1 - d2) - fMaxFraction;

    if (fSpherePlane < 0) {
        fSpherePlane = 0;
    }
#endif

    fMaxFraction            = SURFACE_CLIP_EPSILON / (d1 - d2);
    pt->fSurfaceClipEpsilon = fMaxFraction;

    // Fixed in OPM
    //  Also check when leaving the terrain (usually from below).
    // crosses face
    if (d1 > d2) { // enter
        fSpherePlane = (d1 - sphere.radius) / (d1 - d2) - fMaxFraction;

        if (fSpherePlane < 0) {
            fSpherePlane = 0;
        }
    } else {
        fSpherePlane = (d1 + sphere.radius) / (d1 - d2) - fMaxFraction;

        if (fSpherePlane > 1) {
            fSpherePlane = 1;
        }
    }

    if (fSpherePlane >= pt->tw->trace.fraction) {
        return pt->tw->trace.fraction;
    }

    d1 = (pt->vEnd[0] - pt->vStart[0]) * fSpherePlane + pt->vEnd[0] - sphere.radius * plane[0] - x0;
    d2 = (pt->vEnd[1] - pt->vStart[1]) * fSpherePlane + pt->vEnd[1] - sphere.radius * plane[1] - y0;

    eMode = pt->tc->squares[pt->i][pt->j].eMode;

    if (eMode == 1 || eMode == 2) {
        if ((pt->i + pt->j) & 1) {
            eMode = iPlane ? 6 : 3;
        } else {
            eMode = iPlane ? 5 : 4;
        }
    }

    switch (eMode) {
    case 3:
        bFitsX    = d1 <= 64;
        bFitsY    = d2 <= 64;
        bFitsDiag = d1 >= 64 - d2;

        iX[0] = 1;
        iX[1] = 0;
        iX[2] = 1;
        iY[0] = 1;
        iY[1] = 1;
        iY[2] = 0;
        break;
    case 4:
        bFitsX    = d1 >= 0;
        bFitsY    = d2 <= 64;
        bFitsDiag = d1 <= d2;

        iX[0] = 0;
        iX[1] = 1;
        iX[2] = 0;
        iY[0] = 1;
        iY[1] = 1;
        iY[2] = 0;
        break;
    case 5:
        bFitsX    = d1 <= 64;
        bFitsY    = d2 >= 0;
        bFitsDiag = d1 >= d2;

        iX[0] = 1;
        iX[1] = 0;
        iX[2] = 1;
        iY[0] = 0;
        iY[1] = 0;
        iY[2] = 1;
        break;
    case 6:
        bFitsX    = d1 >= 0;
        bFitsY    = d2 >= 0;
        bFitsDiag = d1 <= 64 - d2;

        iX[0] = 0;
        iX[1] = 1;
        iX[2] = 0;
        iY[0] = 0;
        iY[1] = 0;
        iY[2] = 1;
        break;
    default:
        return 0;
    }

    if (bFitsX && bFitsY) {
        if (bFitsDiag) {
            return fSpherePlane;
        }

        return CM_CheckTerrainTriSphereEdge(pt, plane, x0, y0, iX[1], iY[1], iX[2], iY[2]);
    }

    if (bFitsX && !bFitsY) {
        if (bFitsDiag) {
            return CM_CheckTerrainTriSphereEdge(pt, plane, x0, y0, iX[0], iY[0], iX[1], iY[1]);
        }

        return CM_CheckTerrainTriSphereCorner(pt, plane, x0, y0, iX[1], iY[1]);
    }

    if (!bFitsX && bFitsY) {
        if (bFitsDiag) {
            return CM_CheckTerrainTriSphereEdge(pt, plane, x0, y0, iX[0], iY[0], iX[2], iY[2]);
        }

        return CM_CheckTerrainTriSphereCorner(pt, plane, x0, y0, iX[2], iY[2]);
    }

    if (!bFitsX && !bFitsY) {
        if (bFitsDiag) {
            return CM_CheckTerrainTriSphereCorner(pt, plane, x0, y0, iX[0], iY[0]);
        }
    }

    return pt->tw->trace.fraction;
}

/*
====================
CM_ValidateTerrainCollidePointSquare
====================
*/
qboolean CM_ValidateTerrainCollidePointSquare(const pointtrace_t *pt, float frac)
{
    float f;

    f = pt->vStart[0] + frac * (pt->vEnd[0] - pt->vStart[0]) - ((pt->i << 6) + pt->tc->vBounds[0][0]);

    if (f >= 0 && f <= 64) {
        f = pt->vStart[1] + frac * (pt->vEnd[1] - pt->vStart[1]) - ((pt->j << 6) + pt->tc->vBounds[0][1]);

        if (f >= 0 && f <= 64) {
            return qtrue;
        }
    }

    return qfalse;
}

/*
====================
CM_ValidateTerrainCollidePointTri
====================
*/
qboolean CM_ValidateTerrainCollidePointTri(const pointtrace_t *pt, int eMode, float frac)
{
    float x0, y0;
    float x, y;
    float dx, dy;

    x0 = (pt->i << 6) + pt->tc->vBounds[0][0];
    dx = pt->vStart[0] + (pt->vEnd[0] - pt->vStart[0]) * frac;
    x  = x0 + 64;

    if (x0 > dx) {
        return qfalse;
    }

    if (x < dx) {
        return qfalse;
    }

    y0 = (pt->j << 6) + pt->tc->vBounds[0][1];
    dy = pt->vStart[1] + (pt->vEnd[1] - pt->vStart[1]) * frac;
    y  = y0 + 64;

    if (y0 > dy) {
        return qfalse;
    }

    if (y < dy) {
        return qfalse;
    }

    switch (eMode) {
    case 3:
        return (dx - x0) >= (64 - (dy - y0));
    case 4:
        return (dx - x0) <= (dy - y0);
    case 5:
        return (dx - x0) >= (dy - y0);
    case 6:
        return (dx - x0) <= (64 - (dy - y0));
    default:
        return qtrue;
    }
}

/*
====================
CM_TestTerrainCollideSquare
====================
*/
qboolean CM_TestTerrainCollideSquare(const pointtrace_t *pt)
{
    float *plane;
    float  frac0;
    float  enterFrac;
    int    eMode;

    eMode = pt->tc->squares[pt->i][pt->j].eMode;

    if (!eMode) {
        return qfalse;
    }

    if (eMode >= 0 && eMode <= 2) {
        enterFrac = CM_CheckTerrainPlane(pt, pt->tc->squares[pt->i][pt->j].plane[0]);

        plane = pt->tc->squares[pt->i][pt->j].plane[1];
        frac0 = CM_CheckTerrainPlane(pt, plane);

        if (eMode == 2) {
            if (enterFrac > frac0) {
                enterFrac = frac0;
            }
        } else {
            if (enterFrac < frac0) {
                enterFrac = frac0;
            }
        }

        if (enterFrac < pt->tw->trace.fraction && CM_ValidateTerrainCollidePointSquare(pt, enterFrac)) {
            pt->tw->trace.fraction = enterFrac;
            VectorCopy(plane, pt->tw->trace.plane.normal);
            pt->tw->trace.plane.dist = plane[3];
            return qtrue;
        }
    } else {
        plane     = pt->tc->squares[pt->i][pt->j].plane[0];
        enterFrac = CM_CheckTerrainPlane(pt, plane);

        if (enterFrac < pt->tw->trace.fraction
            && CM_ValidateTerrainCollidePointTri(pt, pt->tc->squares[pt->i][pt->j].eMode, enterFrac)) {
            pt->tw->trace.fraction = enterFrac;
            VectorCopy(plane, pt->tw->trace.plane.normal);
            pt->tw->trace.plane.dist = plane[3];
            return qtrue;
        }
    }

    return qfalse;
}

/*
====================
CM_CheckStartInsideTerrain
====================
*/
static qboolean CM_CheckStartInsideTerrain(const pointtrace_t *pt, int i, int j, float fx, float fy)
{
    float *plane;
    float  fDot;

    if (i < 0 || i > 7) {
        return qfalse;
    }
    if (j < 0 || j > 7) {
        return qfalse;
    }

    if (!pt->tc->squares[i][j].eMode) {
        return qfalse;
    }

    if ((i + j) & 1) {
        if (fx + fy >= 1) {
            if (pt->tc->squares[i][j].eMode == 6) {
                return qfalse;
            }
            plane = pt->tc->squares[i][j].plane[0];
        } else {
            if (pt->tc->squares[i][j].eMode == 3) {
                return qfalse;
            }
            plane = pt->tc->squares[i][j].plane[1];
        }
    } else {
        if (fy >= fx) {
            if (pt->tc->squares[i][j].eMode == 5) {
                return qfalse;
            }
            plane = pt->tc->squares[i][j].plane[0];
        } else {
            if (pt->tc->squares[i][j].eMode == 4) {
                return qfalse;
            }
            plane = pt->tc->squares[i][j].plane[1];
        }
    }

    fDot = DotProduct(pt->vStart, plane);
    if (fDot <= plane[3] && fDot + 32 >= plane[3]) {
        return qtrue;
    }

    return qfalse;
}

/*
====================
CM_PositionTestPointInTerrainCollide
====================
*/
qboolean CM_PositionTestPointInTerrainCollide(const pointtrace_t *pt)
{
    int   i0, j0;
    float fx, fy;

    fx = (pt->vStart[0] - pt->tc->vBounds[0][0]) * (SURFACE_CLIP_EPSILON / 8);
    fy = (pt->vStart[1] - pt->tc->vBounds[0][1]) * (SURFACE_CLIP_EPSILON / 8);

    i0 = (int)floor(fx);
    j0 = (int)floor(fy);

    return CM_CheckStartInsideTerrain(pt, i0, j0, fx - i0, fy - j0);
}

/*
====================
CM_TracePointThroughTerrainCollide
====================
*/
void CM_TracePointThroughTerrainCollide(pointtrace_t *pt)
{
    int i0, j0, i1, j1;
    int di, dj;
    int d1, d2;
    //int nTotal;
    float fx, fy;
    float dx, dy, dx2, dy2;

    fx = (pt->vStart[0] - pt->tc->vBounds[0][0]) * (SURFACE_CLIP_EPSILON / 8);
    fy = (pt->vStart[1] - pt->tc->vBounds[0][1]) * (SURFACE_CLIP_EPSILON / 8);
    i0 = (int64_t)floor(fx);
    j0 = (int64_t)floor(fy);
    i1 = (int64_t)floor((pt->vEnd[0] - pt->tc->vBounds[0][0]) * (SURFACE_CLIP_EPSILON / 8));
    j1 = (int64_t)floor((pt->vEnd[1] - pt->tc->vBounds[0][1]) * (SURFACE_CLIP_EPSILON / 8));

    const float dfx = fx - i0;
    const float dfy = fy - j0;

    if (CM_CheckStartInsideTerrain(pt, i0, j0, dfx, dfy)) {
        pt->tw->trace.startsolid = qtrue;
        pt->tw->trace.allsolid   = qtrue;
        pt->tw->trace.fraction   = 0;
        return;
    }

    if (i0 == i1) {
        if (i0 < 0 || i0 > 7) {
            return;
        }

        if (j0 == j1) {
            if (j0 < 0 || j0 > 7) {
                return;
            }

            pt->i = i0;
            pt->j = j0;
            CM_TestTerrainCollideSquare(pt);
        } else if (j0 >= j1) {
            if (j0 > 7) {
                j0 = 7;
            }
            if (j1 < 0) {
                j1 = 0;
            }

            pt->i = i0;
            for (pt->j = j0; pt->j >= j1; pt->j--) {
                if (CM_TestTerrainCollideSquare(pt)) {
                    return;
                }
            }
        } else {
            if (j0 < 0) {
                j0 = 0;
            }
            if (j1 > 7) {
                j1 = 7;
            }

            pt->i = i0;
            for (pt->j = j0; pt->j <= j1; pt->j++) {
                if (CM_TestTerrainCollideSquare(pt)) {
                    return;
                }
            }
        }
    } else if (j0 == j1) {
        if (j0 < 0 || j0 > 7) {
            return;
        }

        if (i0 >= i1) {
            if (i0 > 7) {
                i0 = 7;
            }
            if (i1 < 0) {
                i1 = 0;
            }

            pt->j = j0;
            for (pt->i = i0; pt->i >= i1; pt->i--) {
                if (CM_TestTerrainCollideSquare(pt)) {
                    break;
                }
            }
        } else {
            if (i0 < 0) {
                i0 = 0;
            }
            if (i1 > 7) {
                i1 = 7;
            }

            pt->j = j0;
            for (pt->i = i0; pt->i <= i1; pt->i++) {
                if (CM_TestTerrainCollideSquare(pt)) {
                    break;
                }
            }
        }
    } else {
        dx = pt->vEnd[0] - pt->vStart[0];
        dy = pt->vEnd[1] - pt->vStart[1];

        // Fix
        //==
        // The original compares if delta float is zero
        // not only it's slower, but it can be problematic if those floats are NaN
        //==
        if (i1 > i0) {
            // dx positive
            d1  = 1;
            di  = i1 - i0;
            dx2 = (i0 + 1 - fx) * dy;
        } else {
            d1  = -1;
            di  = i0 - i1;
            dx  = -dx;
            dx2 = dfx * dy;
        }

        if (j1 > j0) {
            // dy positive
            d2  = 1;
            dj  = di + j1 - j0 + 1;
            dy2 = (j0 + 1 - fy) * dx;
        } else {
            d2  = -1;
            dy  = -dy;
            dj  = di + j0 - j1 + 1;
            dy2 = dfy * dx;
            dx2 = -dx2;
        }

        pt->i = i0;
        pt->j = j0;

        while (1) {
            if (pt->i >= 0 && pt->i <= 7 && pt->j >= 0 && pt->j <= 7) {
                if (CM_TestTerrainCollideSquare(pt)) {
                    return;
                }
            }

            dj--;
            if (!dj) {
                break;
            }

            if (dx2 < dy2) {
                dy2 -= dx2;
                dx2 = dy;
                pt->i += d1;
            } else {
                dx2 -= dy2;
                dy2 = dx;
                pt->j += d2;
            }
        }
    }
}

/*
====================
CM_TraceCylinderThroughTerrainCollide
====================
*/
void CM_TraceCylinderThroughTerrainCollide(pointtrace_t *pt, traceWork_t *tw, const terrainCollide_t *tc)
{
    int   i0, j0, i1, j1;
    float x0, y0;
    float enterFrac;

    i0 = (int)(floor(tw->bounds[0][0] - tc->vBounds[0][0]) * (SURFACE_CLIP_EPSILON / 8));
    i1 = (int)(floor(tw->bounds[1][0] - tc->vBounds[0][0]) * (SURFACE_CLIP_EPSILON / 8));
    j0 = (int)(floor(tw->bounds[0][1] - tc->vBounds[0][1]) * (SURFACE_CLIP_EPSILON / 8));
    j1 = (int)(floor(tw->bounds[1][1] - tc->vBounds[0][1]) * (SURFACE_CLIP_EPSILON / 8));

    if (i0 < 0) {
        i0 = 0;
    }
    if (j0 < 0) {
        j0 = 0;
    }
    if (i1 > 7) {
        i1 = 7;
    }
    if (j1 > 7) {
        j1 = 7;
    }

    y0 = (j0 << 6) + tc->vBounds[0][1];
    for (pt->j = j0; pt->j <= j1; pt->j++) {
        x0 = (i0 << 6) + tc->vBounds[0][0];
        for (pt->i = i0; pt->i <= i1; pt->i++) {
            switch (tc->squares[pt->i][pt->j].eMode) {
            case 1:
            case 2:
                enterFrac = CM_CheckTerrainTriSphere(pt, x0, y0, 0);
                if (enterFrac < 0) {
                    enterFrac = 0;
                }
                if (enterFrac < pt->tw->trace.fraction) {
                    pt->tw->trace.fraction = enterFrac;
                    VectorCopy(pt->tc->squares[pt->i][pt->j].plane[0], pt->tw->trace.plane.normal);
                    pt->tw->trace.plane.dist = pt->tc->squares[pt->i][pt->j].plane[0][3];
                }
                enterFrac = CM_CheckTerrainTriSphere(pt, x0, y0, 1);
                if (enterFrac < 0) {
                    enterFrac = 0;
                }
                if (enterFrac < pt->tw->trace.fraction) {
                    pt->tw->trace.fraction = enterFrac;
                    VectorCopy(pt->tc->squares[pt->i][pt->j].plane[1], pt->tw->trace.plane.normal);
                    pt->tw->trace.plane.dist = pt->tc->squares[pt->i][pt->j].plane[1][3];
                }
                break;
            case 3:
            case 4:
                enterFrac = CM_CheckTerrainTriSphere(pt, x0, y0, 0);
                if (enterFrac < 0) {
                    enterFrac = 0;
                }
                if (enterFrac < pt->tw->trace.fraction) {
                    pt->tw->trace.fraction = enterFrac;
                    VectorCopy(pt->tc->squares[pt->i][pt->j].plane[0], pt->tw->trace.plane.normal);
                    pt->tw->trace.plane.dist = pt->tc->squares[pt->i][pt->j].plane[0][3];
                }
                break;
            case 5:
            case 6:
                enterFrac = CM_CheckTerrainTriSphere(pt, x0, y0, 1);
                if (enterFrac < 0) {
                    enterFrac = 0;
                }
                if (enterFrac < pt->tw->trace.fraction) {
                    pt->tw->trace.fraction = enterFrac;
                    VectorCopy(pt->tc->squares[pt->i][pt->j].plane[1], pt->tw->trace.plane.normal);
                    pt->tw->trace.plane.dist = pt->tc->squares[pt->i][pt->j].plane[1][3];
                }
                break;
            default:
                break;
            }
            x0 += 64;
        }

        y0 += 64;
    }
}

/*
====================
CM_TraceThroughTerrainCollide
====================
*/
void CM_TraceThroughTerrainCollide(traceWork_t *tw, terrainCollide_t *tc)
{
    int          i;
    pointtrace_t pt;

    if (tw->bounds[0][0] >= tc->vBounds[1][0] || tw->bounds[1][0] <= tc->vBounds[0][0]) {
        return;
    }

    if (tw->bounds[0][1] >= tc->vBounds[1][1] || tw->bounds[1][1] <= tc->vBounds[0][1]) {
        return;
    }

    if (tw->bounds[0][2] >= tc->vBounds[1][2] || tw->bounds[1][2] <= tc->vBounds[0][2]) {
        return;
    }

    pt.tw = tw;
    pt.tc = tc;
    VectorCopy(tw->start, pt.vStart);
    VectorCopy(tw->end, pt.vEnd);

    if (sphere.use && cm_ter_usesphere->integer) {
        VectorSubtract(tw->start, sphere.offset, pt.vStart);
        VectorSubtract(tw->end, sphere.offset, pt.vEnd);
        CM_TraceCylinderThroughTerrainCollide(&pt, tw, tc);
    } else if (tw->isPoint) {
        VectorCopy(tw->start, pt.vStart);
        VectorCopy(tw->end, pt.vEnd);
        CM_TracePointThroughTerrainCollide(&pt);
    } else {
        if (tc->squares[0][0].plane[0][2] >= 0) {
            for (i = 0; i < 4; i++) {
                VectorAdd(tw->start, tw->offsets[i], pt.vStart);
                VectorAdd(tw->end, tw->offsets[i], pt.vEnd);

                CM_TracePointThroughTerrainCollide(&pt);
                if (tw->trace.allsolid) {
                    return;
                }
            }
        } else {
            for (i = 4; i < 8; i++) {
                VectorAdd(tw->start, tw->offsets[i], pt.vStart);
                VectorAdd(tw->end, tw->offsets[i], pt.vEnd);

                CM_TracePointThroughTerrainCollide(&pt);
                if (tw->trace.allsolid) {
                    return;
                }
            }
        }
    }
}

/*
====================
CM_PositionTestInTerrainCollide
====================
*/
qboolean CM_PositionTestInTerrainCollide(traceWork_t *tw, terrainCollide_t *tc)
{
    int          i;
    pointtrace_t pt;

    if (tw->bounds[0][0] >= tc->vBounds[1][0] || tw->bounds[1][0] <= tc->vBounds[0][0]) {
        return qfalse;
    }

    if (tw->bounds[0][1] >= tc->vBounds[1][1] || tw->bounds[1][1] <= tc->vBounds[0][1]) {
        return qfalse;
    }

    if (tw->bounds[0][2] >= tc->vBounds[1][2] || tw->bounds[1][2] <= tc->vBounds[0][2]) {
        return qfalse;
    }

    pt.tw = tw;
    pt.tc = tc;
    VectorCopy(tw->start, pt.vStart);
    VectorCopy(tw->end, pt.vEnd);

    if (sphere.use && cm_ter_usesphere->integer) {
        VectorSubtract(tw->start, sphere.offset, pt.vStart);
        VectorSubtract(tw->end, sphere.offset, pt.vEnd);
        CM_TraceCylinderThroughTerrainCollide(&pt, tw, tc);
        return tw->trace.startsolid;
    } else if (tw->isPoint) {
        VectorCopy(tw->start, pt.vStart);
        VectorCopy(tw->end, pt.vEnd);
        return CM_PositionTestPointInTerrainCollide(&pt);
    } else {
        if (tc->squares[0][0].plane[0][2] >= 0) {
            for (i = 0; i < 4; i++) {
                VectorAdd(tw->start, tw->offsets[i], pt.vStart);
                VectorAdd(tw->end, tw->offsets[i], pt.vEnd);

                if (CM_PositionTestPointInTerrainCollide(&pt)) {
                    return qtrue;
                }
            }
        } else {
            for (i = 4; i < 8; i++) {
                VectorAdd(tw->start, tw->offsets[i], pt.vStart);
                VectorAdd(tw->end, tw->offsets[i], pt.vEnd);

                if (CM_PositionTestPointInTerrainCollide(&pt)) {
                    return qtrue;
                }
            }
        }
    }

    return qfalse;
}

/*
====================
CM_SightTracePointThroughTerrainCollide
====================
*/
qboolean CM_SightTracePointThroughTerrainCollide(pointtrace_t *pt)
{
    int   i0, j0;
    int   i1, j1;
    int   di, dj;
    float fx, fy;
    float dx, dy, dx2, dy2;
    float d1, d2;

    fx = (pt->vStart[0] - pt->tc->vBounds[0][0]) * (SURFACE_CLIP_EPSILON / 8);
    fy = (pt->vStart[1] - pt->tc->vBounds[0][1]) * (SURFACE_CLIP_EPSILON / 8);
    i0 = (int)floor(fx);
    j0 = (int)floor(fy);
    i1 = (int)floor((pt->vEnd[0] - pt->tc->vBounds[0][0]) * (SURFACE_CLIP_EPSILON / 8));
    j1 = (int)floor((pt->vEnd[1] - pt->tc->vBounds[0][1]) * (SURFACE_CLIP_EPSILON / 8));

    if (CM_CheckStartInsideTerrain(pt, i0, j0, fx - i0, fy - j0)) {
        return qfalse;
    }

    if (i0 == i1) {
        if (i0 < 0 || i0 > 7) {
            return qtrue;
        }

        if (j0 == j1) {
            if (j0 < 0 || j0 > 7) {
                return qtrue;
            }

            pt->i = i0;
            pt->j = j0;
            return !CM_TestTerrainCollideSquare(pt);
        } else if (j0 >= j1) {
            if (j0 > 7) {
                j0 = 7;
            }
            if (j1 < 0) {
                j1 = 0;
            }

            pt->i = i0;
            for (pt->j = j0; pt->j >= j1; pt->j--) {
                if (CM_TestTerrainCollideSquare(pt)) {
                    return qfalse;
                }
            }
        } else {
            if (j0 < 0) {
                j0 = 0;
            }
            if (j1 > 7) {
                j1 = 7;
            }

            pt->i = i0;
            for (pt->j = j0; pt->j <= j1; pt->j++) {
                if (CM_TestTerrainCollideSquare(pt)) {
                    return qfalse;
                }
            }
        }
    } else if (j0 == j1) {
        if (j0 < 0 || j0 > 7) {
            return qtrue;
        }

        if (i0 >= i1) {
            if (i0 > 7) {
                i0 = 7;
            }
            if (i1 < 0) {
                i1 = 0;
            }

            pt->j = j0;
            for (pt->i = i0; pt->i >= i1; pt->i--) {
                if (CM_TestTerrainCollideSquare(pt)) {
                    return qfalse;
                }
            }
        } else {
            if (i0 < 0) {
                i0 = 0;
            }
            if (i1 > 7) {
                i1 = 7;
            }

            pt->j = j0;
            for (pt->i = i0; pt->i <= i1; pt->i++) {
                if (CM_TestTerrainCollideSquare(pt)) {
                    return qfalse;
                }
            }
        }
    } else {
        dx = pt->vEnd[0] - pt->vStart[0];
        dy = pt->vEnd[1] - pt->vStart[1];

        if (dx > 0) {
            d1  = 1;
            di  = i1 - i0;
            dx2 = (i0 + 1 - fx) * dy;
        } else {
            d1  = -1;
            di  = i0 - i1;
            dx  = -dx;
            dx2 = (fx - i0) * dy;
        }

        if (dy > 0) {
            d2  = 1;
            dj  = di + j1 - j0 + 1;
            dy2 = (j0 + 1 - fy) * dx;
        } else {
            d2  = -1;
            dy  = -dy;
            dj  = di + j0 - j1 + 1;
            dy2 = (fy - j0) * dx;
            dx2 = -dx2;
        }

        pt->i = i0;
        pt->j = j0;

        while (1) {
            if (pt->i >= 0 && pt->i <= 7 && pt->j >= 0 && pt->j <= 7) {
                if (CM_TestTerrainCollideSquare(pt)) {
                    return qfalse;
                }
            }

            dj--;
            if (!dj) {
                break;
            }

            if (dx2 < dy2) {
                dy2 -= dx2;
                dx2 = dy;
                pt->i += d1;
            } else {
                dx2 -= dy2;
                dy2 = dx;
                pt->j += d2;
            }
        }
    }

    return qtrue;
}

/*
====================
CM_SightTraceThroughTerrainCollide
====================
*/
qboolean CM_SightTraceThroughTerrainCollide(traceWork_t *tw, terrainCollide_t *tc)
{
    int          i;
    pointtrace_t pt;

    if (tw->bounds[0][0] >= tc->vBounds[1][0] || tw->bounds[1][0] <= tc->vBounds[0][0]) {
        return qfalse;
    }

    if (tw->bounds[0][1] >= tc->vBounds[1][1] || tw->bounds[1][1] <= tc->vBounds[0][1]) {
        return qfalse;
    }

    if (tw->bounds[0][2] >= tc->vBounds[1][2] || tw->bounds[1][2] <= tc->vBounds[0][2]) {
        return qfalse;
    }

    pt.tw = tw;
    pt.tc = tc;
    VectorCopy(tw->start, pt.vStart);
    VectorCopy(tw->end, pt.vEnd);

    if (tw->isPoint) {
        VectorCopy(tw->start, pt.vStart);
        VectorCopy(tw->end, pt.vEnd);
        return CM_SightTracePointThroughTerrainCollide(&pt);
    } else {
        if (tc->squares[0][0].plane[0][2] >= 0) {
            for (i = 0; i < 4; i++) {
                VectorAdd(tw->start, tw->offsets[i], pt.vStart);
                VectorAdd(tw->end, tw->offsets[i], pt.vEnd);

                if (!CM_SightTracePointThroughTerrainCollide(&pt)) {
                    return qfalse;
                }
            }
        } else {
            for (i = 4; i < 8; i++) {
                VectorAdd(tw->start, tw->offsets[i], pt.vStart);
                VectorAdd(tw->end, tw->offsets[i], pt.vEnd);

                if (!CM_SightTracePointThroughTerrainCollide(&pt)) {
                    return qfalse;
                }
            }
        }
    }

    return qtrue;
}

/*
====================
CM_TerrainSquareType
====================
*/
int CM_TerrainSquareType(int iTerrainPatch, int i, int j)
{
    return cm.terrain[iTerrainPatch].tc.squares[i][j].eMode;
}

/*
====================
CM_SwapTerraPatch

Swaps the patch on big-endian
====================
*/
void CM_SwapTerraPatch(cTerraPatch_t *pPatch)
{
#ifdef Q3_BIG_ENDIAN
    int i;

    pPatch->texCoord[0][0][0] = LittleFloat(pPatch->texCoord[0][0][0]);
    pPatch->texCoord[0][0][1] = LittleFloat(pPatch->texCoord[0][0][1]);
    pPatch->texCoord[0][1][0] = LittleFloat(pPatch->texCoord[0][1][0]);
    pPatch->texCoord[0][1][1] = LittleFloat(pPatch->texCoord[0][1][1]);
    pPatch->texCoord[1][0][0] = LittleFloat(pPatch->texCoord[1][0][0]);
    pPatch->texCoord[1][0][1] = LittleFloat(pPatch->texCoord[1][0][1]);
    pPatch->texCoord[1][1][0] = LittleFloat(pPatch->texCoord[1][1][0]);
    pPatch->texCoord[1][1][1] = LittleFloat(pPatch->texCoord[1][1][1]);
    pPatch->iBaseHeight       = LittleShort(pPatch->iBaseHeight);
    pPatch->iShader           = LittleUnsignedShort(pPatch->iShader);
    pPatch->iLightMap         = LittleUnsignedShort(pPatch->iLightMap);
    pPatch->iNorth            = LittleShort(pPatch->iNorth);
    pPatch->iEast             = LittleShort(pPatch->iEast);
    pPatch->iSouth            = LittleShort(pPatch->iSouth);
    pPatch->iWest             = LittleShort(pPatch->iWest);

    for (i = 0; i < MAX_TERRAIN_VARNODES; i++) {
        pPatch->varTree[0][i].flags = LittleUnsignedShort(pPatch->varTree[0][i].flags);
        pPatch->varTree[1][i].flags = LittleUnsignedShort(pPatch->varTree[1][i].flags);
    }
#endif
}
