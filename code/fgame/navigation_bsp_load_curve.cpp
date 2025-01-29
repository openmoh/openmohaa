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

#include "g_local.h"
#include "navigation_bsp.h"

/*
============
LerpDrawVert
============
*/
static void LerpDrawVert(const Vector *a, const Vector *b, Vector *out)
{
    (*out) = 0.5f * (*a + *b);
}

/*
============
Transpose
============
*/
static void Transpose(int width, int height, Vector ctrl[MAX_GRID_SIZE][MAX_GRID_SIZE])
{
    int    i, j;
    Vector temp;

    if (width > height) {
        for (i = 0; i < height; i++) {
            for (j = i + 1; j < width; j++) {
                if (j < height) {
                    // swap the value
                    temp       = ctrl[j][i];
                    ctrl[j][i] = ctrl[i][j];
                    ctrl[i][j] = temp;
                } else {
                    // just copy
                    ctrl[j][i] = ctrl[i][j];
                }
            }
        }
    } else {
        for (i = 0; i < width; i++) {
            for (j = i + 1; j < height; j++) {
                if (j < width) {
                    // swap the value
                    temp       = ctrl[i][j];
                    ctrl[i][j] = ctrl[j][i];
                    ctrl[j][i] = temp;
                } else {
                    // just copy
                    ctrl[i][j] = ctrl[j][i];
                }
            }
        }
    }
}

/*
============
InvertCtrl
============
*/
static void InvertCtrl(int width, int height, Vector ctrl[MAX_GRID_SIZE][MAX_GRID_SIZE])
{
    int    i, j;
    Vector temp;

    for (i = 0; i < height; i++) {
        for (j = 0; j < width / 2; j++) {
            temp                   = ctrl[i][j];
            ctrl[i][j]             = ctrl[i][width - 1 - j];
            ctrl[i][width - 1 - j] = temp;
        }
    }
}

/*
=================
InvertErrorTable
=================
*/
static void InvertErrorTable(float errorTable[2][MAX_GRID_SIZE], int width, int height)
{
    int   i;
    float copy[2][MAX_GRID_SIZE];

    Com_Memcpy(copy, errorTable, sizeof(copy));

    for (i = 0; i < width; i++) {
        errorTable[1][i] = copy[0][i]; //[width-1-i];
    }

    for (i = 0; i < height; i++) {
        errorTable[0][i] = copy[1][height - 1 - i];
    }
}

/*
==================
PutPointsOnCurve
==================
*/
static void PutPointsOnCurve(Vector ctrl[MAX_GRID_SIZE][MAX_GRID_SIZE], int width, int height)
{
    int    i, j;
    Vector prev, next;

    for (i = 0; i < width; i++) {
        for (j = 1; j < height; j += 2) {
            LerpDrawVert(&ctrl[j][i], &ctrl[j + 1][i], &prev);
            LerpDrawVert(&ctrl[j][i], &ctrl[j - 1][i], &next);
            LerpDrawVert(&prev, &next, &ctrl[j][i]);
        }
    }

    for (j = 0; j < height; j++) {
        for (i = 1; i < width; i += 2) {
            LerpDrawVert(&ctrl[j][i], &ctrl[j][i + 1], &prev);
            LerpDrawVert(&ctrl[j][i], &ctrl[j][i - 1], &next);
            LerpDrawVert(&prev, &next, &ctrl[j][i]);
        }
    }
}

/*
=================
G_CreateSurfaceGridMesh
=================
*/
surfaceGrid_t *
G_CreateSurfaceGridMesh(int width, int height, Vector ctrl[MAX_GRID_SIZE][MAX_GRID_SIZE], int *indexes, int numIndexes)
{
    int            i, j, size;
    Vector        *vert;
    surfaceGrid_t *grid;

    // copy the results out to a grid
    size = sizeof(*grid) + sizeof(Vector) * (width * height) + sizeof(*indexes) * numIndexes;

    grid = (surfaceGrid_t *)gi.Malloc(size);
    Com_Memset(grid, 0, size);

    grid->width       = width;
    grid->height      = height;
    grid->numVertices = width * height;
    grid->numIndexes  = numIndexes;

    for (i = 0; i < width; i++) {
        for (j = 0; j < height; j++) {
            vert  = &grid->getVertices()[j * width + i];
            *vert = ctrl[j][i];
        }
    }

    for (i = 0; i < numIndexes; i++) {
        grid->getIndices()[i] = indexes[i];
    }

    return grid;
}

/*
=================
G_FreeSurfaceGridMesh
=================
*/
void G_FreeSurfaceGridMesh(surfaceGrid_t *grid)
{
    gi.Free(grid);
}

static int MakeMeshIndexes(int width, int height, int indexes[(MAX_GRID_SIZE - 1) * (MAX_GRID_SIZE - 1) * 2 * 3])
{
    int i, j;
    int numIndexes;
    int w, h;

    h          = height - 1;
    w          = width - 1;
    numIndexes = 0;
    for (i = 0; i < h; i++) {
        for (j = 0; j < w; j++) {
            int v1, v2, v3, v4;

            // vertex order to be recognized as tristrips
            v1 = i * width + j + 1;
            v2 = v1 - 1;
            v3 = v2 + width;
            v4 = v3 + 1;

            indexes[numIndexes++] = v2;
            indexes[numIndexes++] = v3;
            indexes[numIndexes++] = v1;

            indexes[numIndexes++] = v1;
            indexes[numIndexes++] = v3;
            indexes[numIndexes++] = v4;
        }
    }

    return numIndexes;
}

/*
=================
G_SubdividePatchToGrid
=================
*/
surfaceGrid_t *
G_SubdividePatchToGrid(int width, int height, float subdivide, Vector points[MAX_PATCH_SIZE * MAX_PATCH_SIZE])
{
    int               i, j, k, l;
    Vector            prev, next, mid;
    float             len, maxLen;
    int               dir;
    int               t;
    MAC_STATIC Vector ctrl[MAX_GRID_SIZE][MAX_GRID_SIZE];
    float             errorTable[2][MAX_GRID_SIZE];
    int               numIndexes;
    int               indexes[(MAX_GRID_SIZE - 1) * (MAX_GRID_SIZE - 1) * 2 * 3];

    for (i = 0; i < width; i++) {
        for (j = 0; j < height; j++) {
            ctrl[j][i] = points[j * width + i];
        }
    }

    for (dir = 0; dir < 2; dir++) {
        for (j = 0; j < MAX_GRID_SIZE; j++) {
            errorTable[dir][j] = 0;
        }

        // horizontal subdivisions
        for (j = 0; j + 2 < width; j += 2) {
            // check subdivided midpoints against control points

            // FIXME: also check midpoints of adjacent patches against the control points
            // this would basically stitch all patches in the same LOD group together.

            maxLen = 0;
            for (i = 0; i < height; i++) {
                vec3_t midxyz;
                vec3_t midxyz2;
                vec3_t dir;
                vec3_t projected;
                float  d;

                // calculate the point on the curve
                for (l = 0; l < 3; l++) {
                    midxyz[l] = (ctrl[i][j][l] + ctrl[i][j + 1][l] * 2 + ctrl[i][j + 2][l]) * 0.25f;
                }

                // see how far off the line it is
                // using dist-from-line will not account for internal
                // texture warping, but it gives a lot less polygons than
                // dist-from-midpoint
                VectorSubtract(midxyz, ctrl[i][j], midxyz);
                VectorSubtract(ctrl[i][j + 2], ctrl[i][j], dir);
                VectorNormalize(dir);

                d = DotProduct(midxyz, dir);
                VectorScale(dir, d, projected);
                VectorSubtract(midxyz, projected, midxyz2);
                len = VectorLengthSquared(midxyz2); // we will do the sqrt later
                if (len > maxLen) {
                    maxLen = len;
                }
            }

            maxLen = sqrt(maxLen);

            // if all the points are on the lines, remove the entire columns
            if (maxLen < 0.1f) {
                errorTable[dir][j + 1] = 999;
                continue;
            }

            // see if we want to insert subdivided columns
            if (width + 2 > MAX_GRID_SIZE) {
                errorTable[dir][j + 1] = 1.0f / maxLen;
                continue; // can't subdivide any more
            }

            if (maxLen <= subdivide) {
                errorTable[dir][j + 1] = 1.0f / maxLen;
                continue; // didn't need subdivision
            }

            errorTable[dir][j + 2] = 1.0f / maxLen;

            // insert two columns and replace the peak
            width += 2;
            for (i = 0; i < height; i++) {
                LerpDrawVert(&ctrl[i][j], &ctrl[i][j + 1], &prev);
                LerpDrawVert(&ctrl[i][j + 1], &ctrl[i][j + 2], &next);
                LerpDrawVert(&prev, &next, &mid);

                for (k = width - 1; k > j + 3; k--) {
                    ctrl[i][k] = ctrl[i][k - 2];
                }
                ctrl[i][j + 1] = prev;
                ctrl[i][j + 2] = mid;
                ctrl[i][j + 3] = next;
            }

            // back up and recheck this set again, it may need more subdivision
            j -= 2;
        }

        Transpose(width, height, ctrl);
        t      = width;
        width  = height;
        height = t;
    }

    // put all the aproximating points on the curve
    PutPointsOnCurve(ctrl, width, height);

    // cull out any rows or columns that are colinear
    for (i = 1; i < width - 1; i++) {
        if (errorTable[0][i] != 999) {
            continue;
        }
        for (j = i + 1; j < width; j++) {
            for (k = 0; k < height; k++) {
                ctrl[k][j - 1] = ctrl[k][j];
            }
            errorTable[0][j - 1] = errorTable[0][j];
        }
        width--;
    }

    for (i = 1; i < height - 1; i++) {
        if (errorTable[1][i] != 999) {
            continue;
        }
        for (j = i + 1; j < height; j++) {
            for (k = 0; k < width; k++) {
                ctrl[j - 1][k] = ctrl[j][k];
            }
            errorTable[1][j - 1] = errorTable[1][j];
        }
        height--;
    }

#if 1
    // flip for longest tristrips as an optimization
    // the results should be visually identical with or
    // without this step
    if (height > width) {
        Transpose(width, height, ctrl);
        InvertErrorTable(errorTable, width, height);
        t      = width;
        width  = height;
        height = t;
        InvertCtrl(width, height, ctrl);
    }
#endif

    // calculate indexes
    numIndexes = MakeMeshIndexes(width, height, indexes);

    return G_CreateSurfaceGridMesh(width, height, ctrl, indexes, numIndexes);
}
