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

static vec3_t cg_vEntAngles;
static vec3_t cg_vEntOrigin;
static vec3_t cg_fEntAxis[3];
static qboolean cg_bEntAnglesSet;
markPoly_t* cg_freeMarkPolys;   // single linked list
markObj_t cg_activeMarkObjs;
markObj_t* cg_freeMarkObjs;
treadMark_t cg_treadMarks[MAX_TREAD_MARKS];

markPoly_t* cg_markPolys;
int cg_iNumMarkPolys;
markObj_t* cg_markObjs;
int cg_iNumFreeMarkObjs;
int cg_iMinFreeMarkObjs;
qboolean cg_bMarksInitialized;
cvar_t* cg_treadmark_test;

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
    int iMaxMarks;

    iMaxMarks = cg_maxMarks->integer;
    if (iMaxMarks < 32) {
        iMaxMarks = 32;
    }

    cg_iMinFreeMarkObjs = iMaxMarks / 32;
    if (cg_iMinFreeMarkObjs < 8) {
        cg_iMinFreeMarkObjs = 8;
    }
    if (cg_iMinFreeMarkObjs > 32) {
        cg_iMinFreeMarkObjs = 32;
    }

    cg_iNumMarkPolys = 7 * iMaxMarks / 4;

    if (cg_markPolys) {
        cgi.Free(cg_markPolys);
    }

    cg_markPolys = (markPoly_t*)cgi.Malloc(sizeof(markPoly_t) * cg_iNumMarkPolys);

    if (!cg_markPolys) {
        cgi.Error(ERR_DROP, "CG_InitMarks: Could not allocate array for mark polys");
    }

    if (cg_markObjs) {
        cgi.Free(cg_markObjs);
    }

    cg_markObjs = (markObj_t*)cgi.Malloc(sizeof(markObj_t) * iMaxMarks);

    if (!cg_markObjs) {
        cgi.Error(ERR_DROP, "CG_InitMarks: Could not allocate array for mark objects");
    }

    memset(cg_markPolys, 0, sizeof(markPoly_t) * cg_iNumMarkPolys);

    cg_freeMarkPolys = cg_markPolys;
    for (i = 0; i < cg_iNumMarkPolys - 1; i++) {
        cg_markPolys[i].nextPoly = &cg_markPolys[i + 1];
    }
    cg_markPolys[i].nextPoly = NULL;

    cg_activeMarkObjs.nextMark = &cg_activeMarkObjs;
    cg_activeMarkObjs.prevMark = &cg_activeMarkObjs;
    memset(cg_markObjs, 0, sizeof(markObj_t) * iMaxMarks);
    cg_freeMarkObjs = cg_markObjs;

    for (i = 0; i < iMaxMarks - 1; i++) {
        cg_markObjs[i].nextMark = &cg_markObjs[i + 1];
    }
    cg_markObjs[i].nextMark = NULL;

    cg_iNumFreeMarkObjs = iMaxMarks;
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
    if (!cg_bMarksInitialized) {
        return;
    }

    le->nextPoly = cg_freeMarkPolys;
    cg_freeMarkPolys = le;
}

/*
==================
CG_FreeMarkObj
==================
*/
void CG_FreeMarkObj(markObj_t* pMark) {
    markPoly_t* pPoly;
    markPoly_t* pNextPoly;

    pPoly = pMark->markPolys;
    for (pPoly = pMark->markPolys; pPoly; pPoly = pNextPoly) {
        pNextPoly = pPoly->nextPoly;
        CG_FreeMarkPoly(pPoly);
    }

    pMark->prevMark->nextMark = pMark->nextMark;
    pMark->nextMark->prevMark = pMark->prevMark;
    pMark->nextMark = cg_freeMarkObjs;
    cg_freeMarkObjs = pMark;
    cg_iNumFreeMarkObjs++;
}

/*
==================
CG_FreeBestMarkObj
==================
*/
void CG_FreeBestMarkObj(qboolean bAllowFade) {
    markObj_t* pMark;

    for (pMark = cg_activeMarkObjs.prevMark; pMark != &cg_activeMarkObjs; pMark = pMark->prevMark)
    {
        if (pMark->lastVisTime < cg.time - 250)
        {
            CG_FreeMarkObj(pMark);
            return;
        }
    }

    if (!cg_iNumFreeMarkObjs || !bAllowFade) {
        CG_FreeMarkObj(cg_activeMarkObjs.prevMark);
        return;
    }

    for (pMark = cg_activeMarkObjs.prevMark; pMark != &cg_activeMarkObjs; pMark = pMark->prevMark)
    {
        if (!pMark->alphaFade || pMark->time > cg.time - 9000) {
            pMark->time = cg.time - 9000;
            pMark->alphaFade = 1;
        }
    }
}

/*
===================
CG_AllocMark

Will allways succeed, even if it requires freeing an old active mark
===================
*/
markObj_t* CG_AllocMark(int iNumPolys)
{
    int iPolyCount;
    markPoly_t* pPoly;
    markObj_t* pMark;

    if (!cg_bMarksInitialized) {
        return NULL;
    }

    if (iNumPolys < 1) {
        return NULL;
    }

    if (cg_iNumFreeMarkObjs <= cg_iMinFreeMarkObjs) {
        CG_FreeBestMarkObj(1);
    }

    pMark = cg_freeMarkObjs;
    cg_freeMarkObjs = cg_freeMarkObjs->nextMark;

    memset(pMark, 0, sizeof(markObj_t));
    pMark->lastVisTime = cg.time;

    for (iPolyCount = 0; iPolyCount < iNumPolys; iPolyCount++)
    {
        while (!cg_freeMarkPolys) {
            CG_FreeBestMarkObj(qfalse);
        }

        pPoly = cg_freeMarkPolys;
        cg_freeMarkPolys = pPoly->nextPoly;

        memset(pPoly, 0, sizeof(*pPoly));
        pPoly->nextPoly = pMark->markPolys;
        pMark->markPolys = pPoly;
    }


    // link into the active list
    pMark->nextMark = cg_activeMarkObjs.nextMark;
    pMark->prevMark = &cg_activeMarkObjs;
    cg_activeMarkObjs.nextMark->prevMark = pMark;
    cg_activeMarkObjs.nextMark = pMark;
    cg_iNumFreeMarkObjs--;
    return pMark;
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

    // FIXME: unimplemented
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
    int j;
    int t;
    int fade;
    markObj_t* pMark;
    markObj_t* pNext;
    markPoly_t* pPoly;
    polyVert_t* pVert;
    polyVert_t tmpVerts[8];
    int viewleafnum;

    if (!cg_bMarksInitialized) {
        return;
    }

    if (!cg_addMarks->integer) {
        return;
    }

    for (pMark = cg_activeMarkObjs.nextMark; pMark != &cg_activeMarkObjs; pMark = pNext) {
        // grab next now, so if the local entity is freed we
        // still have it
        pNext = pMark->nextMark;

        // see if it is time to completely remove it
        if (pMark->alphaFade && cg.time > pMark->time + MARK_TOTAL_TIME)
        {
            CG_FreeMarkObj(pMark);
            continue;
        }

        if (pMark->lastVisTime < cg.time - 3000) {
            CG_FreeMarkObj(pMark);
            continue;
        }

        // FIXME: unimplemented
    }

    CG_AddTreadMarks();
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
