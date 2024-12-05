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

// tr_vis.cpp: visibility culling

#include "tr_local.h"
#include "tr_vis.h"

#define MAX_POINTS_ON_WINDING 128
#define MAX_MAP_VISIBILITY    0x200000
#define MAX_VPORTALS          1024

typedef struct {
    int    numpoints;
    vec3_t p[24];
} viswinding_t;

typedef struct {
    int    numpoints;
    vec3_t p[4];
} winding_t;

typedef struct {
    vec3_t normal;
    float  dist;
} plane_t;

typedef struct {
    int           num;
    qboolean      hint;
    plane_t       plane;
    int           leaf;
    vec3_t        origin;
    float         radius;
    viswinding_t *winding;
} vportal_t;

typedef struct leaf_s {
    int        numportals;
    vportal_t *portals[MAX_VPORTALS];
} leaf_t;

int             portalclusters;
int             numportals;
int             numfaces;
int             leafbytes;
int             leaflongs;
int             portalbytes;
int             portallongs;
vportal_t      *portals;
leaf_t         *leafs;
int             numVisBytes;
static char     mapname[256];
static qboolean prev_showportal = qfalse;

winding_t *AllocWinding(int points)
{
    winding_t *w;
    int        s;

    if (points > MAX_POINTS_ON_WINDING) {
        ri.Error(ERR_DROP, "Exceeded MAX_POINTS_ON_WINDING");
        return NULL;
    }
    s = sizeof(vec_t) * 4 * points + sizeof(int) * 4;

    w = (winding_t *)malloc(s);
    if (!w) {
        ri.Error(ERR_DROP, "Couldn't allocate winding");
        return NULL;
    }

    if (s < 0) {
        s = sizeof(vec_t) * 4 * points + sizeof(int) * 4 + 3;
    }

    memset(w, 0, s);

    return w;
}

viswinding_t *AllocVisWinding(int points)
{
    return (viswinding_t *)AllocWinding(points);
}

void PlaneFromWinding(viswinding_t *w, plane_t *plane)
{
    vec3_t v1, v2;

    VectorSubtract(w->p[2], w->p[1], v1);
    VectorSubtract(w->p[2], w->p[1], v1);
    CrossProduct(v2, v1, plane->normal);
    VectorNormalize(plane->normal);

    plane->dist = DotProduct(w->p[0], plane->normal);
}

void SetPortalSphere(vportal_t *p)
{
    int           i;
    vec3_t        total;
    vec3_t        dist;
    viswinding_t *w;
    float         r;
    float         bestr;

    w = p->winding;
    VectorCopy(vec3_origin, total);

    for (i = 0; i < w->numpoints; i++) {
        VectorAdd(total, w->p[i], total);
    }

    for (i = 0; i < 3; i++) {
        total[i] /= w->numpoints;
    }

    bestr = 0;

    for (i = 0; i < w->numpoints; i++) {
        VectorSubtract(w->p[i], total, dist);
        r = VectorLength(dist);
        if (bestr < r) {
            bestr = r;
        }
    }

    VectorCopy(total, p->origin);
    p->radius = bestr;
}

void LoadPortals(const char *name)
{
    int         i, j;
    vportal_t  *p;
    leaf_t     *l;
    const char *magic;
    int         leafnums[2];
    plane_t     plane;
    char       *data;
    char       *f;
    int         numpoints;
    qboolean    bHint;

    if (ri.FS_ReadFile(name, (void **)&f) == -1) {
        ri.Error(ERR_DROP, "LoadPortals: couldn't read %s\n", name);
        return;
    }

    data  = f;
    magic = COM_Parse(&data);

    if (strcmp(magic, "PRT1")) {
        ri.Error(1, "LoadPortals: not a portal file");
    }

    portalclusters = atoi(COM_Parse(&data));
    numportals     = atoi(COM_Parse(&data));
    numfaces       = atoi(COM_Parse(&data));

    ri.Printf(PRINT_DEVELOPER, "%6i portalclusters\n", portalclusters);
    ri.Printf(PRINT_DEVELOPER, "%6i numportals\n", numportals);
    ri.Printf(PRINT_DEVELOPER, "%6i numfaces\n", numfaces);

    leafbytes   = ((portalclusters + 63) & 0xC0) >> 3;
    leaflongs   = leafbytes >> 2;
    portalbytes = ((numportals * 2 + 63) & 0xC0) >> 3;
    portallongs = portalbytes >> 2;

    //
    // Initialize portals
    //
    portals = (vportal_t *)malloc(numportals * sizeof(vportal_t));
    if (!portals) {
        ri.Error(ERR_DROP, "LoadPortals: portals allocation failed\n");
        return;
    }
    memset(portals, 0, numportals * sizeof(vportal_t));

    //
    // Initialize leafs
    //
    leafs = (leaf_t *)malloc(portalclusters * sizeof(leaf_t));
    if (!leafs) {
        ri.Error(ERR_DROP, "LoadPortals: leafs allocation failed\n");
        return;
    }
    memset(leafs, 0, portalclusters * sizeof(leaf_t));

    assert(leafbytes * portalclusters + sizeof(leafnums) <= MAX_MAP_VISIBILITY);
    if (numVisBytes > MAX_MAP_VISIBILITY) {
        ri.Error(ERR_DROP, "LoadPortals: NumVisBytes %d exceeds %d\n", numVisBytes, 0x200000);
    }

    for (i = 0; i < numportals; i++) {
        viswinding_t *w;

        p = &portals[i];

        numpoints   = atoi(COM_Parse(&data));
        leafnums[0] = atoi(COM_Parse(&data));
        leafnums[1] = atoi(COM_Parse(&data));

        if (numpoints > MAX_POINTS_ON_WINDING) {
            ri.Error(ERR_DROP, "LoadPortals: portal %i has too many points", i);
            return;
        }

        if (leafnums[0] > portalclusters || leafnums[1] > portalclusters) {
            ri.Error(ERR_DROP, "LoadPortals: reading portal %i", i);
            return;
        }

        bHint        = atoi(COM_Parse(&data));
        w            = AllocVisWinding(numpoints);
        w->numpoints = numpoints;
        p->winding   = w;

        for (j = 0; j < numpoints; j++) {
            magic = COM_Parse(&data);
            if (magic[0] != '(') {
                ri.Error(ERR_DROP, "LoadPortals: expecting '(' not '%s'", magic);
                return;
            }

            if (magic[1]) {
                w->p[j][0] = atof(magic + 1);
            } else {
                w->p[j][0] = atof(COM_Parse(&data));
            }

            w->p[j][1] = atof(COM_Parse(&data));
            w->p[j][2] = atof(COM_Parse(&data));

            magic = COM_Parse(&data);
            if (strcmp(")", magic)) {
                ri.Error(ERR_DROP, "LoadPortals: expecting ')' not '%s'", magic);
            }
        }

        PlaneFromWinding(w, &plane);

        //
        // Leafs
        //

        // First leaf

        l = &leafs[leafnums[0]];

        if (l->numportals == ARRAY_LEN(l->portals)) {
            ri.Error(ERR_DROP, "Leaf with too many portals");
            return;
        }

        l->portals[l->numportals++] = p;
        p->num                      = i + 1;
        p->hint                     = bHint;
        p->winding                  = w;
        VectorSubtract(vec3_origin, plane.normal, p->plane.normal);
        p->plane.dist = -plane.dist;
        p->leaf = leafnums[1];
        SetPortalSphere(p);

        // Second leaf

        p++;

        l = &leafs[leafnums[1]];
        if (l->numportals == ARRAY_LEN(l->portals)) {
            ri.Error(ERR_DROP, "Leaf with too many portals");
            return;
        }

        l->portals[l->numportals++] = p;
        p->num                      = i + 1;
        p->hint                     = bHint;
        p->winding                  = (viswinding_t *)AllocVisWinding(w->numpoints);
        p->winding->numpoints       = w->numpoints;

        for (j = 0; j < w->numpoints; j++) {
            VectorCopy(w->p[numpoints - (j + 1)], p->winding->p[j]);
        }

        p->plane = plane;
        p->leaf  = leafnums[0];
        SetPortalSphere(p);
    }

    ri.FS_FreeFile(f);
}

void R_VisDebug()
{
    int           leafnum;
    int           cluster;
    int           numportals;
    leaf_t       *leaf;
    vportal_t    *p;
    int           pnum;
    viswinding_t *w;
    int           numpoints;
    int           i;

    if (!r_showportal->integer) {
        return;
    }

    if (!prev_showportal) {
        char buffer[256];

        prev_showportal = qtrue;
        COM_StripExtension(mapname, buffer, sizeof(buffer));
        Q_strcat(buffer, sizeof(buffer), ".prt");
        LoadPortals(buffer);
    }

    leafnum = ri.CM_PointLeafnum(tr.refdef.vieworg);
    cluster = ri.CM_LeafCluster(leafnum);
    if (cluster < 0) {
        return;
    }

    leaf       = &leafs[cluster];
    numportals = leaf->numportals;
    for (pnum = 0; pnum < numportals; pnum++) {
        p = leaf->portals[pnum];
        w = p->winding;

        numpoints = w->numpoints;
        for (i = 0; i < numpoints; i++) {
            R_DebugLine(w->p[i], w->p[(i + 1) % numpoints], 0.0, 1.0, 0.0, 1.0);
        }
    }
}

void R_VisDebugLoad(const char *szBSPName)
{
    prev_showportal = qfalse;
    return Q_strncpyz(mapname, szBSPName, sizeof(mapname));
}
