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

// DESCRIPTION:
// wall marks

#include "cg_local.h"

/*
===================================================================

MARK POLYS

===================================================================
*/

markPoly_t  cg_activeMarkPolys; // double linked list
markPoly_t *cg_freeMarkPolys;   // single linked list
markPoly_t  cg_markPolys[MAX_MARK_POLYS];
markObj_t* cg_markObjs;
markObj_t cg_activeMarkObjs;
markObj_t* cg_freeMarkObjs;
treadMark_t cg_treadMarks[MAX_TREAD_MARKS];
cvar_t* cg_treadmark_test;
int cg_iNumFreeMarkObjs;
int cg_iMinFreeMarkObjs;
qboolean cg_bMarksInitialized;

vec3_t vec_upwards;

/*
===================
CG_InitMarkPolys

This is called at startup and for tournement restarts
===================
*/
void CG_InitMarks(void)
{
    int i;

    memset(cg_markPolys, 0, sizeof(cg_markPolys));

    cg_activeMarkPolys.nextMark = &cg_activeMarkPolys;
    cg_activeMarkPolys.prevMark = &cg_activeMarkPolys;
    cg_freeMarkPolys            = cg_markPolys;
    for (i = 0; i < MAX_MARK_POLYS - 1; i++) {
        cg_markPolys[i].nextMark = &cg_markPolys[i + 1];
    }

    cg_bMarksInitialized = qtrue;

    // FIXME: partially implemented
}

/*
==================
CG_FreeMarkPoly
==================
*/
void CG_FreeMarkPoly(markPoly_t *le)
{
    if (!le->prevMark) {
        cgi.Error(ERR_DROP, "CG_FreeLocalEntity: not active");
    }

    // remove from the doubly linked active list
    le->prevMark->nextMark = le->nextMark;
    le->nextMark->prevMark = le->prevMark;

    // the free list is only singly linked
    le->nextMark     = cg_freeMarkPolys;
    cg_freeMarkPolys = le;
}

/*
===================
CG_AllocMark

Will allways succeed, even if it requires freeing an old active mark
===================
*/
markPoly_t *CG_AllocMark(void)
{
    markPoly_t *le;

    if (!cg_freeMarkPolys) {
        int time;
        // no free entities, so free the one at the end of the chain
        // remove the oldest active entity
        time = cg_activeMarkPolys.prevMark->time;
        while (cg_activeMarkPolys.prevMark && time == cg_activeMarkPolys.prevMark->time) {
            CG_FreeMarkPoly(cg_activeMarkPolys.prevMark);
        }
    }

    le               = cg_freeMarkPolys;
    cg_freeMarkPolys = cg_freeMarkPolys->nextMark;

    memset(le, 0, sizeof(*le));

    // link into the active list
    le->nextMark                          = cg_activeMarkPolys.nextMark;
    le->prevMark                          = &cg_activeMarkPolys;
    cg_activeMarkPolys.nextMark->prevMark = le;
    cg_activeMarkPolys.nextMark           = le;
    return le;
}

void CG_AssembleFinalMarks(
    vec3_t           *markPoints,
    markFragment_t   *markFragments,
    int              numFragments,
    qboolean         (*PerPolyCallback)(const vec3_t* markPoints, markFragment_t* mf, polyVert_t* verts, void* pCustom),
    int              (*GetLeafCallback)(markFragment_t* mf, void* pCustom),
    void             *pCustom,
    qhandle_t        markShader,
    qboolean         fadein,
    qboolean         alphaFade
)
{
    // FIXME: unimplemented
}

/*
=================
CG_ImpactMark

origin should be a point within a unit of the plane
dir should be the plane normal

temporary marks will not be stored or randomly oriented, but immediately
passed to the renderer.
=================
*/
#define MAX_MARK_FRAGMENTS 128
#define MAX_MARK_POINTS    384

void CG_ImpactMark(
    qhandle_t    markShader,
    const vec3_t origin,
    const vec3_t dir,
    float        orientation,
    float        fSScale,
    float        fTScale,
    float        red,
    float        green,
    float        blue,
    float        alpha,
    qboolean     alphaFade,
    qboolean     temporary,
    qboolean     dolighting,
    qboolean     fadein,
    float        fSCenter,
    float        fTCenter
)
{
    vec3_t         axis[3];
    float          texCoordScale;
    vec3_t         originalPoints[4];
    byte           colors[4];
    int            i, j;
    int            numFragments;
    markFragment_t markFragments[MAX_MARK_FRAGMENTS], *mf;
    vec3_t         markPoints[MAX_MARK_POINTS];
    vec3_t         projection;
    float          radius = fSScale * fTScale;

    if (!cg_bMarksInitialized) {
        return;
    }

    if (!cg_addMarks->integer
        && markShader != cgs.media.shadowMarkShader
        && markShader != cgs.media.footShadowMarkShader) {
        return;
    }

    if (fSScale == 0.0) {
        fSScale = 1.0;
    }
    if (fTScale == 0.0) {
        fTScale = 1.0;
    }

    if (radius <= 0) {
        cgi.Error(ERR_DROP, "CG_ImpactMark called with <= 0 radius");
    }

    // create the texture axis

    if (orientation) {
        VectorNormalize2(dir, axis[0]);
        PerpendicularVector(axis[1], axis[0]);
        RotatePointAroundVector(axis[2], axis[0], axis[1], orientation);
        CrossProduct(axis[0], axis[2], axis[1]);
    } else {
        vec3_t angles;
        vec3_t tmp;

        VectorNormalize2(dir, axis[0]);
        VectorCopy(dir, tmp);
        tmp[0] = -tmp[0];
        tmp[1] = -tmp[1];
        vectoangles(tmp, angles);
        AnglesToMat(angles, axis);
        VectorScale(axis[2], -1, axis[2]);
    }

    texCoordScale = 0.5 * 1.0 / radius;

    // create the full polygon
    for (i = 0; i < 3; i++) {
        originalPoints[0][i] = origin[i] - radius * axis[1][i] - radius * axis[2][i];
        originalPoints[1][i] = origin[i] + radius * axis[1][i] - radius * axis[2][i];
        originalPoints[2][i] = origin[i] + radius * axis[1][i] + radius * axis[2][i];
        originalPoints[3][i] = origin[i] - radius * axis[1][i] + radius * axis[2][i];
    }

    // get the fragments
    VectorScale(dir, -32, projection);

    // FIXME: fRadiusSquared
    numFragments = cgi.R_MarkFragments(
        4, (void *)originalPoints, projection, MAX_MARK_POINTS, markPoints[0], MAX_MARK_FRAGMENTS, markFragments, 0.f
    );

    if (fadein) {
        colors[0] = 0;
        colors[1] = 0;
        colors[2] = 0;
        colors[3] = 0;
    } else {
        colors[0] = red * 255;
        colors[1] = green * 255;
        colors[2] = blue * 255;
        colors[3] = alpha * 255;
    }

    for (i = 0, mf = markFragments; i < numFragments; i++, mf++) {
        polyVert_t *v;
        polyVert_t  verts[MAX_VERTS_ON_POLY];
        markPoly_t *mark;

        // we have an upper limit on the complexity of polygons
        // that we store persistantly
        if (mf->numPoints > MAX_VERTS_ON_POLY) {
            mf->numPoints = MAX_VERTS_ON_POLY;
        }
        for (j = 0, v = verts; j < mf->numPoints; j++, v++) {
            vec3_t delta;

            VectorCopy(markPoints[mf->firstPoint + j], v->xyz);

            VectorSubtract(v->xyz, origin, delta);
            v->st[0]            = 0.5 + DotProduct(delta, axis[1]) * texCoordScale;
            v->st[1]            = 0.5 + DotProduct(delta, axis[2]) * texCoordScale;
            *(int *)v->modulate = *(int *)colors;
        }

        // if it is a temporary (shadow) mark, add it immediately and forget
        // about it
        if (temporary) {
            cgi.R_AddPolyToScene(markShader, mf->numPoints, verts, 0);
            continue;
        }

        // otherwise save it persistantly
        mark                = CG_AllocMark();
        mark->time          = cg.time;
        mark->alphaFade     = alphaFade;
        mark->markShader    = markShader;
        mark->poly.numVerts = mf->numPoints;
        mark->color[0]      = red;
        mark->color[1]      = green;
        mark->color[2]      = blue;
        mark->color[3]      = alpha;
        mark->fadein        = fadein;
        memcpy(mark->verts, verts, mf->numPoints * sizeof(verts[0]));
    }
}

void CG_ImpactMarkSimple(
    qhandle_t    markShader,
    const vec3_t origin,
    const vec3_t dir,
    float        orientation,
    float        fRadius,
    float        red,
    float        green,
    float        blue,
    float        alpha,
    qboolean     alphaFade,
    qboolean     temporary,
    qboolean     dolighting,
    qboolean     fadein
)
{
    if (cg_bMarksInitialized) {
        CG_ImpactMark(
            markShader,
            origin,
            dir,
            orientation,
            fRadius,
            fRadius,
            red,
            green,
            blue,
            alpha,
            alphaFade,
            temporary,
            dolighting,
            fadein,
            0.5f,
            0.5f
        );
    }
    // FIXME: unimplemented
}

/*
===============
CG_AddMarks
===============
*/
#define MARK_TOTAL_TIME 10000
#define MARK_FADE_TIME  1000

void CG_AddMarks(void)
{
    int         j;
    markPoly_t *mp, *next;
    int         t;
    int         fade;

    if (!cg_addMarks->integer) {
        return;
    }

    mp = cg_activeMarkPolys.nextMark;
    for (; mp != &cg_activeMarkPolys; mp = next) {
        // grab next now, so if the local entity is freed we
        // still have it
        next = mp->nextMark;

        // see if it is time to completely remove it
        if (cg.time > mp->time + MARK_TOTAL_TIME) {
            CG_FreeMarkPoly(mp);
            continue;
        }

        // fade all marks out with time
        t = mp->time + MARK_TOTAL_TIME - cg.time;

        if (mp->lightstyle > 0) {
            CG_LightStyleColor(mp->lightstyle, t, mp->color, qtrue);

            if (mp->color[3] <= 0) {
                CG_FreeMarkPoly(mp);
                continue;
            }

            for (j = 0; j < mp->poly.numVerts; j++) {
                mp->verts[j].modulate[0] = mp->color[0] * 255;
                mp->verts[j].modulate[1] = mp->color[1] * 255;
                mp->verts[j].modulate[2] = mp->color[2] * 255;
                mp->verts[j].modulate[3] = mp->color[3] * 255;
            }
        }

        // Fade in marks
        if (mp->fadein) {
            fade = 255 * (cg.time - mp->time) / MARK_FADE_TIME;

            if (fade > 255) {
                fade = 255;
            }

            if (mp->alphaFade) {
                for (j = 0; j < mp->poly.numVerts; j++) {
                    mp->verts[j].modulate[3] = fade;
                }
            } else {
                for (j = 0; j < mp->poly.numVerts; j++) {
                    mp->verts[j].modulate[0] = mp->color[0] * fade;
                    mp->verts[j].modulate[1] = mp->color[1] * fade;
                    mp->verts[j].modulate[2] = mp->color[2] * fade;
                }
            }
        }

        // Fade out marks
        if (t < MARK_FADE_TIME) {
            fade = 255 * t / MARK_FADE_TIME;
            if (mp->alphaFade) {
                for (j = 0; j < mp->poly.numVerts; j++) {
                    mp->verts[j].modulate[3] = fade;
                }
            } else {
                for (j = 0; j < mp->poly.numVerts; j++) {
                    mp->verts[j].modulate[0] = mp->color[0] * fade;
                    mp->verts[j].modulate[1] = mp->color[1] * fade;
                    mp->verts[j].modulate[2] = mp->color[2] * fade;
                }
            }
        }

        cgi.R_AddPolyToScene(mp->markShader, mp->poly.numVerts, mp->verts, 0);
    }
}

qboolean CG_CheckMakeMarkOnEntity(int iEntIndex)
{
    if (iEntIndex == ENTITYNUM_WORLD) {
        return qtrue;
    }

    if (iEntIndex == ENTITYNUM_NONE) {
        return qfalse;
    }

    if (cg_entities[iEntIndex].currentState.solid != SOLID_BMODEL) {
        return qfalse;
    }

    if (cg_entities[iEntIndex].currentState.modelindex < 0 || cg_entities[iEntIndex].currentState.modelindex > cgi.CM_NumInlineModels()) {
        return qfalse;
    }

    return qtrue;
}

void CG_InitTestTreadMark()
{
    cg_treadmark_test = cgi.Cvar_Get("cg_treadmark_test", "0", 0);
}

int CG_StartTreadMark(int iReference, qhandle_t treadShader, vec_t *vStartPos, float fWidth, float fAlpha)
{
    // FIXME: unimplemented
    return 0;
}

qboolean CG_MakeTreadMarkDecal_PerPolyCallback(const vec3_t *markPoints, markFragment_t *mf, polyVert_t *verts, void *pCustom)
{
    // FIXME: unimplemented
    return qfalse;
}

int CG_MakeTreadMarkDecal_GetLeafCallback(markFragment_t *mf, void *pCustom)
{
    // FIXME: unimplemented
    return 0;
}

void CG_MakeTreadMarkDecal(treadMark_t *pTread, qboolean bStartSegment, qboolean bTemporary)
{
    // FIXME: unimplemented
}

int CG_UpdateTreadMark(int iReference, vec_t *vNewPos, float fAlpha)
{
    // FIXME: unimplemented
    return 0;
}

void CG_AddTreadMarks()
{
    // FIXME: unimplemented
}

int CG_PermanentMark(
    const vec3_t    origin,
    const vec3_t    dir,
    float           orientation,
    float           fSScale,
    float           fTScale,
    float           red,
    float           green,
    float           blue,
    float           alpha,
    qboolean        dolighting,
    float           fSCenter,
    float           fTCenter,
    markFragment_t *pMarkFragments,
    void           *pVoidPolyVerts
)
{
    // FIXME: unimplemented
    return 0;
}

int CG_PermanentTreadMarkDecal(
    treadMark_t    *pTread,
    qboolean        bStartSegment,
    qboolean        dolighting,
    markFragment_t *pMarkFragments,
    void           *pVoidPolyVerts
)
{
    // FIXME: unimplemented
    return 0;
}

int CG_PermanentUpdateTreadMark(
    treadMark_t *pTread, float fAlpha, float fMinSegment, float fMaxSegment, float fMaxOffset, float fTexScale
)
{
    // FIXME: unimplemented
    return 0;
}
