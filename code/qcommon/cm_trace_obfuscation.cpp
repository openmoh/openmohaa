/*
===========================================================================
Copyright (C) 2023 the OpenMoHAA team

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

#include "cm_local.h"

static char com_token[MAX_TOKEN_CHARS];

/*
================
CM_NextCsvToken
================
*/
char *CM_NextCsvToken(char **text, qboolean crossline)
{
    char    *p;
    char    *out;
    char     c;
    int      i;
    qboolean newline;
    qboolean comma;

    com_token[0] = 0;
    p            = *text;
    if (!p) {
        return com_token;
    }

    for (c = *p; isspace(c); p++, c = *p) {
        if (c == '\n' && !crossline) {
            break;
        }
    }

    out = com_token;

    for (i = 0;; i++, p++) {
        c = *p;

        newline = qfalse;
        comma   = qfalse;

        if (*p && c != ',') {
            comma = qtrue;
        }

        if (comma && c != '\n') {
            newline = qtrue;
        }

        if (!newline) {
            break;
        }

        *out = c;
        out++;
    }

    if (c == ',') {
        p++;
    }

    while (i > 0) {
        if (!isspace(com_token[i - 1])) {
            break;
        }

        i--;
    }

    com_token[i] = 0;
    *text        = p;

    if (!p[0]) {
        *text = NULL;
    }

    return com_token;
}

/*
================
CM_SetupObfuscationMapping
================
*/
obfuscation_t *CM_SetupObfuscationMapping()
{
    obfuscation_t *list;
    obfuscation_t *obfuscation;
    char         **files;
    int            numFiles;
    int            numObfuscations;
    int            i, j;

    list = (obfuscation_t *)Hunk_AllocateTempMemory(sizeof(obfuscation_t) * MAX_OBFUSCATIONS);
    for (i = 0; i < MAX_OBFUSCATIONS; i++) {
        list[i].name[0]       = 0;
        list[i].heightDensity = 0;
        list[i].widthDensity  = 0;
    }

    files           = FS_ListFiles("scripts/", ".csv", qfalse, &numFiles);
    numObfuscations = 0;

    for (i = 0; i < numFiles; ++i) {
        const char *filename = va("scripts/%s", files[i]);
        void       *buffer;
        char       *text;
        char       *token;

        if (FS_ReadFile(filename, &buffer) < 0) {
            continue;
        }

        text = (char *)buffer;
        while (text) {
            token = CM_NextCsvToken(&text, qtrue);
            if (!token[0]) {
                break;
            }

            for (j = 0; j < numObfuscations; ++j) {
                if (!Q_stricmp(token, list[j].name)) {
                    Com_Printf("WARNING: using redefinition of obfuscation for '%s' in '%s'\n", token, files[i]);
                    break;
                }
            }

            obfuscation = &list[j];
            Q_strncpyz(obfuscation->name, token, sizeof(obfuscation->name));

            token = CM_NextCsvToken(&text, qfalse);
            if (!text) {
                Com_Printf(
                    "WARNING: unexpected EOF in definition of obfuscation for '%s' in '%s'; skipping\n",
                    obfuscation->name,
                    files[i]
                );
                break;
            }

            if (token[0]) {
                float maxNumVolumes = atof(token);

                if (maxNumVolumes > 0) {
                    obfuscation->widthDensity = 0.5f / maxNumVolumes;
                } else {
                    obfuscation->widthDensity = 0;
                }

                token = CM_NextCsvToken(&text, 0);
                if (!text) {
                    Com_Printf(
                        "WARNING: unexpected EOF in definition of obfuscation for '%s' in '%s'; skipping\n",
                        obfuscation->name,
                        files[i]
                    );
                    break;
                }

                if (token[0]) {
                    float maxDist = atof(token);

                    if (maxDist > 0) {
                        obfuscation->heightDensity = 1.f / maxDist;
                    } else {
                        obfuscation->heightDensity = 0;
                    }

                    if (numObfuscations == MAX_OBFUSCATIONS) {
                        Com_Printf("WARNING: exceeded MAX_OBFUSCATIONS (%i)", numObfuscations);
                    } else {
                        numObfuscations++;
                    }
                } else {
                    Com_Printf(
                        "WARNING: missing max distance thorugh obscuring volumes for '%s' in '%s'; skipping\n",
                        obfuscation->name,
                        files[i]
                    );
                    SkipRestOfLine(&text);
                }
            } else {
                Com_Printf(
                    "WARNING: missing max number of obscuring volumes for '%s' in '%s'; skipping\n",
                    obfuscation->name,
                    files[i]
                );
                SkipRestOfLine(&text);
            }
        }

        FS_FreeFile(buffer);
    }

    list[numObfuscations].name[0] = 0;

    return list;
}

/*
================
CM_ReleaseObfuscationMapping
================
*/
void CM_ReleaseObfuscationMapping(obfuscation_t *obfuscation)
{
    Hunk_FreeTempMemory(obfuscation);
}

/*
================
CM_ObfuscationForShader
================
*/
void CM_ObfuscationForShader(obfuscation_t *list, const char *shaderName, float *widthDensity, float *heightDensity)
{
    obfuscation_t *current;

    for (current = list; current->name[0]; current++) {
        if (!Q_stricmp(shaderName, current->name)) {
            *widthDensity  = current->widthDensity;
            *heightDensity = current->heightDensity;
        }
    }

    Com_Printf("WARNING: using default obfuscation for shader %s\n", shaderName);
    *widthDensity  = 1.f / 16.f;
    *heightDensity = 1.f / 1024.f;
}

/*
================
CM_ObfuscationTraceThroughBrush
================
*/
float CM_ObfuscationTraceThroughBrush(traceWork_t *tw, cbrush_t *brush)
{
    int           i;
    cplane_t     *plane;
    float         dist;
    float         enterFrac, leaveFrac;
    float         enterDensity, leaveDensity;
    float         delta;
    float         d1, d2;
    float         f;
    cbrushside_t *side;

    enterFrac    = 0;
    enterDensity = 0;
    leaveFrac    = 1.0;
    leaveDensity = 0;

    //
    // compare the trace against all planes of the brush
    // find the latest time the trace crosses a plane towards the interior
    // and the earliest time the trace crosses a plane towards the exterior
    //
    for (i = 0; i < brush->numsides; i++) {
        side  = brush->sides + i;
        plane = side->plane;

        // adjust the plane distance apropriately for mins/maxs
        //dist = plane->dist - DotProduct( tw->offsets[ plane->signbits ], plane->normal );
        dist = plane->dist;

        d1 = DotProduct(tw->start, plane->normal) - dist;
        d2 = DotProduct(tw->end, plane->normal) - dist;

        // if it doesn't cross the plane, the plane isn't relevent
        if (d1 >= 0 && d2 >= 0) {
            return 0;
        }

        if (d1 < 0 && d2 > 0) {
            f = d1 / (d1 - d2);
            if (leaveFrac > f) {
                leaveFrac = f;
            }
            leaveDensity = cm.shaders[brush->shaderNum].obfuscationWidthDensity;
        } else if (d1 > 0 && d2 < 0) {
            f = d1 / (d1 - d2);
            if (enterFrac < f) {
                enterFrac = f;
            }
            enterDensity = cm.shaders[brush->shaderNum].obfuscationWidthDensity;
        }
    }

    delta = leaveFrac - enterFrac;
    if (delta <= 0) {
        return 0;
    }

    return enterDensity + cm.shaders[brush->shaderNum].obfuscationHeightDensity * tw->radius * delta + leaveDensity;
}

/*
================
CM_ObfuscationTraceToLeaf
================
*/
float CM_ObfuscationTraceToLeaf(traceWork_t *tw, cLeaf_t *leaf)
{
    int       k;
    cbrush_t *b;
    float     total;

    total = 0;
    // test box position against all brushes in the leaf
    for (k = 0; k < leaf->numLeafBrushes; k++) {
        b = &cm.brushes[cm.leafbrushes[leaf->firstLeafBrush + k]];
        if (b->checkcount == cm.checkcount) {
            continue; // already checked this brush in another leaf
        }
        b->checkcount = cm.checkcount;

        if (!(b->contents & CONTENTS_DONOTENTER)) {
            continue;
        }

        total += CM_ObfuscationTraceThroughBrush(tw, b);
    }

    return total;
}

/*
==================
CM_ObfuscationTraceThroughTree

Traverse all the contacted leafs from the start to the end position.
If the trace is a point, they will be exactly in order, but for larger
trace volumes it is possible to hit something in a later leaf with
a smaller intercept fraction.
==================
*/
float CM_ObfuscationTraceThroughTree(traceWork_t *tw, int num, float p1f, float p2f, vec3_t p1, vec3_t p2)
{
    cNode_t  *node;
    cplane_t *plane;
    float     t1, t2;
    float     frac, frac2;
    float     idist;
    vec3_t    mid;
    int       side;
    float     midf;

    // if < 0, we are in a leaf node
    if (num < 0) {
        return CM_ObfuscationTraceToLeaf(tw, &cm.leafs[-1 - num]);
    }

    //
    // find the point distances to the seperating plane
    // and the offset for the size of the box
    //
    node  = cm.nodes + num;
    plane = node->plane;

    // adjust the plane distance apropriately for mins/maxs
    if (plane->type < 3) {
        t1 = p1[plane->type] - plane->dist;
        t2 = p2[plane->type] - plane->dist;
    } else {
        t1 = DotProduct(plane->normal, p1) - plane->dist;
        t2 = DotProduct(plane->normal, p2) - plane->dist;
    }

    // see which sides we need to consider
    if (t1 >= SURFACE_CLIP_EPSILON && t2 >= SURFACE_CLIP_EPSILON) {
        return CM_ObfuscationTraceThroughTree(tw, node->children[0], p1f, p2f, p1, p2);
    }
    if (t1 < -SURFACE_CLIP_EPSILON && t2 < -SURFACE_CLIP_EPSILON) {
        return CM_ObfuscationTraceThroughTree(tw, node->children[1], p1f, p2f, p1, p2);
    }

    // put the crosspoint SURFACE_CLIP_EPSILON pixels on the near side
    if (t1 < t2) {
        idist = 1.0 / (t1 - t2);
        side  = 1;
        frac2 = (t1 + SURFACE_CLIP_EPSILON) * idist;
        frac  = (t1 + SURFACE_CLIP_EPSILON) * idist;
    } else if (t1 > t2) {
        idist = 1.0 / (t1 - t2);
        side  = 0;
        frac2 = (t1 - SURFACE_CLIP_EPSILON) * idist;
        frac  = (t1 + SURFACE_CLIP_EPSILON) * idist;
    } else {
        side  = 0;
        frac  = 1;
        frac2 = 0;
    }

    // move up to the node
    if (frac < 0) {
        frac = 0;
    } else if (frac > 1) {
        frac = 1;
    }

    midf = p1f + (p2f - p1f) * frac;

    mid[0] = p1[0] + frac * (p2[0] - p1[0]);
    mid[1] = p1[1] + frac * (p2[1] - p1[1]);
    mid[2] = p1[2] + frac * (p2[2] - p1[2]);

    CM_ObfuscationTraceThroughTree(tw, node->children[side], p1f, midf, p1, mid);

    // go past the node
    if (frac2 < 0) {
        frac2 = 0;
    } else if (frac2 > 1) {
        frac2 = 1;
    }

    midf = p1f + (p2f - p1f) * frac2;

    mid[0] = p1[0] + frac2 * (p2[0] - p1[0]);
    mid[1] = p1[1] + frac2 * (p2[1] - p1[1]);
    mid[2] = p1[2] + frac2 * (p2[2] - p1[2]);

    return CM_ObfuscationTraceThroughTree(tw, node->children[side ^ 1], midf, p2f, mid, p2);
}

/*
================
CM_ObfuscationTrace
================
*/
float CM_ObfuscationTrace(const vec3_t start, const vec3_t end, clipHandle_t handle)
{
    cmodel_t   *model;
    vec3_t      delta;
    traceWork_t tw;

    model = CM_ClipHandleToModel(handle);

    c_traces++;
    cm.checkcount++;

    VectorCopy(start, tw.start);
    VectorCopy(end, tw.end);
    VectorSubtract(tw.end, tw.start, delta);
    tw.radius = VectorLength(delta);

    if (handle) {
        return CM_ObfuscationTraceToLeaf(&tw, &model->leaf);
    } else {
        return CM_ObfuscationTraceThroughTree(&tw, 0, 0.0, 1.0, tw.start, tw.end);
    }
}

/*
================
CM_VisualObfuscation
================
*/
float CM_VisualObfuscation(const vec3_t start, const vec3_t end)
{
    return CM_ObfuscationTrace(start, end, 0);
}
