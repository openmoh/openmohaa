/*
===========================================================================
Copyright (C) 2024 the OpenMoHAA team

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

#define MAX_MARK_FRAGMENTS 128
#define MAX_MARK_POINTS    384

static vec3_t   cg_vEntAngles;
static vec3_t   cg_vEntOrigin;
static vec3_t   cg_fEntAxis[3];
static qboolean cg_bEntAnglesSet;

static int      cg_iLastEntIndex;
static int      cg_iLastEntTime;
static qboolean cg_bLastEntValid;

markPoly_t *cg_freeMarkPolys; // single linked list
markObj_t   cg_activeMarkObjs;
markObj_t  *cg_freeMarkObjs;
treadMark_t cg_treadMarks[MAX_TREAD_MARKS];

markPoly_t *cg_markPolys;
int         cg_iNumMarkPolys;
markObj_t  *cg_markObjs;
int         cg_iNumFreeMarkObjs;
int         cg_iMinFreeMarkObjs;
qboolean    cg_bMarksInitialized;
cvar_t     *cg_treadmark_test;

vec3_t vec_upwards = {0, 0, 1};

typedef struct cg_impactmarkinfo_s {
    vec3_t axis[3];
    vec3_t origin;
    float  fSCenter;
    float  fTCenter;
    float  texCoordScaleS;
    float  texCoordScaleT;
    byte   colors[4];
    int    leafnum;
} cg_impactmarkinfo_t;

typedef struct cg_treadmarkinfo_s {
    treadMark_t *pTread;
    vec3_t       vDirection;
    vec3_t       vRight;
    float        fStartDist;
    float        fStartTex;
    float        fStartAlpha;
    float        fLeftStartDist;
    float        fRightStartDist;
    float        fRightCenterDist;
    float        fLeftTexScale;
    float        fRightTexScale;
    float        fCenterTexScale;
    float        fLeftAlphaScale;
    float        fRightAlphaScale;
    float        fCenterAlphaScale;
    float        fOOWidth;
    float        fOODoubleWidth;
    byte         colors[4];
    int          leafnum;
} cg_treadmarkinfo_t;

int CG_GetMarkFragments(
    int             numVerts,
    const vec3_t   *pVerts,
    const vec3_t    vProjection,
    const vec3_t   *pPointBuffer,
    markFragment_t *pFragmentBuffer,
    float           fRadiusSquared
)
{
    int             i, j;
    int             iNumEnts;
    int             iCurrPoints, iCurrFragments;
    int             iCurrMaxPoints, iCurrMaxFragments;
    int             iNewPoints, iNewFragments;
    clipHandle_t    cmodel;
    vec3_t          vProjectionDir;
    vec3_t          vMins, vMaxs;
    vec3_t          vTemp;
    vec3_t          vAngles, vOrigin;
    const vec3_t   *pCurrPoint;
    centity_t      *pEntList[64];
    entityState_t  *pEnt;
    markFragment_t *pCurrFragment;
    markFragment_t *pFragment;

    iNewFragments = cgi.R_MarkFragments(
        numVerts,
        pVerts,
        vProjection,
        MAX_MARK_POINTS,
        (float *)pPointBuffer,
        MAX_MARK_FRAGMENTS,
        pFragmentBuffer,
        fRadiusSquared
    );

    if (iNewFragments > MAX_MARK_FRAGMENTS) {
        return iNewFragments;
    }

    iNewPoints = 0;
    for (i = 0, pFragment = pFragmentBuffer; i < iNewFragments; i++, pFragment++) {
        iNewPoints += pFragment->numPoints;
    }

    if (iNewPoints >= MAX_MARK_POINTS) {
        return iNewFragments;
    }

    iCurrPoints       = iNewPoints;
    iCurrFragments    = iNewFragments;
    iCurrMaxPoints    = MAX_MARK_POINTS - iNewPoints;
    iCurrMaxFragments = MAX_MARK_FRAGMENTS - iNewFragments;
    pCurrPoint        = &pPointBuffer[iNewPoints];
    pCurrFragment     = &pFragmentBuffer[iNewFragments];

    VectorNormalize2(vProjection, vProjectionDir);
    ClearBounds(vMins, vMaxs);

    for (i = 0; i < numVerts; i++) {
        AddPointToBounds(pVerts[i], vMins, vMaxs);
        VectorAdd(pVerts[i], vProjection, vTemp);
        AddPointToBounds(vTemp, vMins, vMaxs);
        VectorMA(pVerts[i], -20.0f, vProjectionDir, vTemp);
        AddPointToBounds(vTemp, vMins, vMaxs);
    }

    iNumEnts = CG_GetBrushEntitiesInBounds(ARRAY_LEN(pEntList), pEntList, vMins, vMaxs);
    if (!iNumEnts) {
        return iNewFragments;
    }

    for (i = 0; i < iNumEnts; i++) {
        pEnt = &pEntList[i]->currentState;
        VectorCopy(pEntList[i]->lerpAngles, vAngles);
        VectorCopy(pEntList[i]->lerpOrigin, vOrigin);

        cmodel = cgi.CM_InlineModel(pEnt->modelindex);

        iNewFragments = cgi.R_MarkFragmentsForInlineModel(
            cmodel,
            vAngles,
            vOrigin,
            numVerts,
            pVerts,
            vProjection,
            iCurrMaxPoints,
            (float *)pCurrPoint,
            iCurrMaxFragments,
            pCurrFragment,
            fRadiusSquared
        );

        iCurrFragments += iNewFragments;
        if (iCurrFragments >= MAX_MARK_FRAGMENTS) {
            break;
        }

        iNewPoints = 0;
        for (j = 0, pFragment = pCurrFragment; j < iNewFragments; j++, pFragment++) {
            pFragment->firstPoint += iCurrPoints;
            pFragment->iIndex = -pEnt->number;
            iNewPoints += pFragment->numPoints;
        }

        iCurrPoints += iNewPoints;
        if (iCurrPoints >= MAX_MARK_POINTS) {
            break;
        }

        pCurrPoint += iNewPoints;
        iCurrMaxPoints -= iNewPoints;
        iCurrMaxFragments -= iNewFragments;
        pCurrFragment += iNewFragments;
    }

    return iCurrFragments;
}

static qboolean CG_GetMarkInlineModelOrientation(int iIndex)
{
    centity_t *pCEnt;

    if (iIndex == cg_iLastEntIndex && cg_iLastEntTime == cg.time) {
        return cg_bLastEntValid;
    }

    pCEnt = &cg_entities[-iIndex];
    if (pCEnt->currentValid && pCEnt->currentState.modelindex < cgs.numInlineModels) {
        VectorCopy(pCEnt->lerpAngles, cg_vEntAngles);
        VectorCopy(pCEnt->lerpOrigin, cg_vEntOrigin);
        cg_bLastEntValid = qtrue;

        if (cg_vEntAngles[0] || cg_vEntAngles[1] || cg_vEntAngles[2]) {
            AngleVectorsLeft(cg_vEntAngles, cg_fEntAxis[0], cg_fEntAxis[1], cg_fEntAxis[2]);
            cg_bEntAnglesSet = qtrue;
        } else if (cg_bEntAnglesSet) {
            AxisClear(cg_fEntAxis);
            cg_bEntAnglesSet = qfalse;
        }

        cg_iLastEntIndex = iIndex;
        cg_iLastEntTime  = cg.time;
        return cg_bLastEntValid;
    }

    cg_bLastEntValid = qfalse;
    VectorClear(cg_vEntAngles);
    VectorClear(cg_vEntOrigin);

    if (cg_bEntAnglesSet) {
        AxisClear(cg_fEntAxis);
        cg_bEntAnglesSet = qfalse;
    }

    return qfalse;
}

static void CG_FragmentPosToWorldPos(const vec3_t vFrom, vec3_t vTo)
{
    if (cg_bEntAnglesSet) {
        VectorMA(cg_vEntOrigin, vFrom[0], cg_fEntAxis[0], vTo);
        VectorMA(vTo, vFrom[1], cg_fEntAxis[1], vTo);
        VectorMA(vTo, vFrom[2], cg_fEntAxis[2], vTo);
    } else {
        vTo[0] = cg_vEntOrigin[0] + *vFrom;
        vTo[1] = cg_vEntOrigin[1] + vFrom[1];
        vTo[2] = cg_vEntOrigin[2] + vFrom[2];
    }
}

qboolean CG_UpdateMarkPosition(markObj_t *pMark)
{
    vec3_t v;
    vec3_t pt;
    int    iIndex;

    iIndex = pMark->markPolys->iIndex;
    if (!CG_GetMarkInlineModelOrientation(iIndex)) {
        return qfalse;
    }

    VectorCopy(pMark->markPolys->verts[0].xyz, v);

    if (cg_bEntAnglesSet) {
        VectorScale(cg_fEntAxis[0], v[0], pt);
        VectorMA(pt, v[1], cg_fEntAxis[1], pt);
        VectorMA(pt, v[2], cg_fEntAxis[2], pt);
        VectorAdd(pt, cg_vEntOrigin, pt);
    } else {
        VectorAdd(v, cg_vEntOrigin, pt);
    }

    pMark->leafnum = cgi.CM_PointLeafnum(pt);

    return qtrue;
}

void CG_AddFragmentToScene(int iIndex, qhandle_t hShader, int iNumVerts, polyVert_t *pVerts)
{
    if (!iIndex) {
        cgi.R_AddPolyToScene(hShader, iNumVerts, pVerts, 0);
        return;
    }

    if (iIndex > 0) {
        cgi.R_AddTerrainMarkToScene(iIndex, hShader, iNumVerts, pVerts, 0);
        return;
    }

    if (cg_bEntAnglesSet) {
        int         i;
        vec3_t      vTmp;
        polyVert_t *pCurrVert;

        for (i = 0; i < iNumVerts; i++) {
            pCurrVert = &pVerts[i];

            vTmp[0] = cg_fEntAxis[0][0] * pCurrVert->xyz[0];
            vTmp[1] = cg_fEntAxis[0][1] * pCurrVert->xyz[0];
            vTmp[2] = cg_fEntAxis[0][2] * pCurrVert->xyz[0];
            VectorMA(vTmp, pCurrVert->xyz[1], cg_fEntAxis[1], vTmp);
            VectorMA(vTmp, pCurrVert->xyz[2], cg_fEntAxis[2], vTmp);
            VectorAdd(vTmp, cg_vEntOrigin, pCurrVert->xyz);
        }
    } else {
        int         i;
        polyVert_t *pCurrVert;

        for (i = 0; i < iNumVerts; i++) {
            pCurrVert = &pVerts[i];
            VectorAdd(pCurrVert->xyz, cg_vEntOrigin, pCurrVert->xyz);
        }
    }

    cgi.R_AddPolyToScene(hShader, iNumVerts, pVerts, 0);
}

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

    cg_markPolys = (markPoly_t *)cgi.Malloc(sizeof(markPoly_t) * cg_iNumMarkPolys);

    if (!cg_markPolys) {
        cgi.Error(ERR_DROP, "CG_InitMarks: Could not allocate array for mark polys");
    }

    if (cg_markObjs) {
        cgi.Free(cg_markObjs);
    }

    cg_markObjs = (markObj_t *)cgi.Malloc(sizeof(markObj_t) * iMaxMarks);

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

    cg_iNumFreeMarkObjs  = iMaxMarks;
    cg_bMarksInitialized = qtrue;
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

    le->nextPoly     = cg_freeMarkPolys;
    cg_freeMarkPolys = le;
}

/*
==================
CG_FreeMarkObj
==================
*/
void CG_FreeMarkObj(markObj_t *pMark)
{
    markPoly_t *pPoly;
    markPoly_t *pNextPoly;

    assert(pMark != &cg_activeMarkObjs);

    for (pPoly = pMark->markPolys; pPoly; pPoly = pNextPoly) {
        pNextPoly = pPoly->nextPoly;
        CG_FreeMarkPoly(pPoly);
    }

    pMark->prevMark->nextMark = pMark->nextMark;
    pMark->nextMark->prevMark = pMark->prevMark;
    pMark->nextMark           = cg_freeMarkObjs;
    cg_freeMarkObjs           = pMark;
    cg_iNumFreeMarkObjs++;
}

/*
==================
CG_FreeBestMarkObj
==================
*/
void CG_FreeBestMarkObj(qboolean bAllowFade)
{
    markObj_t *pMark;

    for (pMark = cg_activeMarkObjs.prevMark; pMark != &cg_activeMarkObjs; pMark = pMark->prevMark) {
        if (pMark->lastVisTime < cg.time - 250) {
            CG_FreeMarkObj(pMark);
            return;
        }
    }

    if (!cg_iNumFreeMarkObjs || !bAllowFade) {
        CG_FreeMarkObj(cg_activeMarkObjs.prevMark);
        return;
    }

    for (pMark = cg_activeMarkObjs.prevMark; pMark != &cg_activeMarkObjs; pMark = pMark->prevMark) {
        if (!pMark->alphaFade || pMark->time > cg.time - 9000) {
            break;
        }
    }

    pMark->time = cg.time - 9000;
    pMark->alphaFade = qtrue;
}

/*
===================
CG_AllocMark

Will allways succeed, even if it requires freeing an old active mark
===================
*/
markObj_t *CG_AllocMark(int iNumPolys)
{
    int         iPolyCount;
    markPoly_t *pPoly;
    markObj_t  *pMark;

    if (!cg_bMarksInitialized) {
        return NULL;
    }

    if (iNumPolys < 1) {
        return NULL;
    }

    if (iNumPolys > cg_iNumMarkPolys) {
        // Added in OPM
        //  Make sure to not over allocate polys
        return NULL;
    }

    if (cg_iNumFreeMarkObjs <= cg_iMinFreeMarkObjs) {
        CG_FreeBestMarkObj(1);
    }

    pMark           = cg_freeMarkObjs;
    cg_freeMarkObjs = cg_freeMarkObjs->nextMark;

    memset(pMark, 0, sizeof(markObj_t));
    pMark->lastVisTime = cg.time;

    for (iPolyCount = 0; iPolyCount < iNumPolys; iPolyCount++) {
        while (!cg_freeMarkPolys) {
            CG_FreeBestMarkObj(qfalse);
        }

        pPoly            = cg_freeMarkPolys;
        cg_freeMarkPolys = pPoly->nextPoly;

        memset(pPoly, 0, sizeof(*pPoly));
        pPoly->nextPoly  = pMark->markPolys;
        pMark->markPolys = pPoly;
    }

    // link into the active list
    pMark->nextMark                      = cg_activeMarkObjs.nextMark;
    pMark->prevMark                      = &cg_activeMarkObjs;
    cg_activeMarkObjs.nextMark->prevMark = pMark;
    cg_activeMarkObjs.nextMark           = pMark;

    cg_iNumFreeMarkObjs--;
    assert(cg_iNumFreeMarkObjs >= 0);

    return pMark;
}

int CG_ImpactMark_GetLeafCallback(markFragment_t *mf, void *pCustom)
{
    return ((cg_impactmarkinfo_t *)pCustom)->leafnum;
}

qboolean CG_ImpactMark_PerPolyCallback(const vec3_t *markPoints, markFragment_t *mf, polyVert_t *verts, void *pCustom)
{
    int                  j;
    polyVert_t          *v;
    cg_impactmarkinfo_t *pInfo;

    pInfo = (cg_impactmarkinfo_t *)pCustom;

    if (mf->iIndex >= 0) {
        for (j = 0; j < mf->numPoints; j++) {
            vec3_t delta;

            v = &verts[j];
            VectorCopy(markPoints[j + mf->firstPoint], v->xyz);
            VectorSubtract(v->xyz, pInfo->origin, delta);
            v->st[0]       = pInfo->fSCenter + DotProduct(delta, pInfo->axis[1]) * pInfo->texCoordScaleS;
            v->st[1]       = pInfo->fTCenter + DotProduct(delta, pInfo->axis[2]) * pInfo->texCoordScaleT;
            v->modulate[0] = pInfo->colors[0];
            v->modulate[1] = pInfo->colors[1];
            v->modulate[2] = pInfo->colors[2];
            v->modulate[3] = pInfo->colors[3];
        }
    } else {
        if (!CG_GetMarkInlineModelOrientation(mf->iIndex)) {
            return qfalse;
        }

        for (j = 0; j < mf->numPoints; j++) {
            vec3_t vWorldPos;
            vec3_t delta;

            v = &verts[j];
            VectorCopy(markPoints[j + mf->firstPoint], v->xyz);

            CG_FragmentPosToWorldPos(v->xyz, vWorldPos);
            VectorSubtract(vWorldPos, pInfo->origin, delta);
            v->st[0]       = pInfo->fSCenter + DotProduct(delta, pInfo->axis[1]) * pInfo->texCoordScaleS;
            v->st[1]       = pInfo->fTCenter + DotProduct(delta, pInfo->axis[2]) * pInfo->texCoordScaleT;
            v->modulate[0] = pInfo->colors[0];
            v->modulate[1] = pInfo->colors[1];
            v->modulate[2] = pInfo->colors[2];
            v->modulate[3] = pInfo->colors[3];
        }
    }

    return qtrue;
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
    int                 i;
    int                 numFragments;
    float               fSScale2, fTScale2;
    float               fSScale3, fTScale3;
    vec3_t              originalPoints[4];
    vec3_t              markPoints[MAX_MARK_POINTS];
    vec3_t              projection;
    markFragment_t      markFragments[MAX_MARK_FRAGMENTS], *mf;
    vec3_t              v;
    float               fRadiusSquared;
    cg_impactmarkinfo_t info;

    if (!cg_bMarksInitialized) {
        return;
    }

    if (!cg_addMarks->integer && markShader != cgs.media.shadowMarkShader
        && markShader != cgs.media.footShadowMarkShader) {
        return;
    }

    if (fSScale == 0.0) {
        fSScale = 1.0;
    }
    if (fTScale == 0.0) {
        fTScale = 1.0;
    }

    fRadiusSquared      = fSScale * fSScale + fTScale * fTScale;
    info.texCoordScaleS = 0.5 / fSScale;
    info.texCoordScaleT = 0.5 / fTScale;

    if (fSCenter < 0.0f || fSCenter > 1.0f) {
        fSCenter = 0.5f;
    }

    if (fTCenter < 0.0f || fTCenter > 1.0f) {
        fTCenter = 0.5f;
    }

    fSScale2 = (fSCenter + fSCenter) * fSScale;
    fTScale2 = (fTCenter + fTCenter) * fTScale;
    fSScale3 = (1.0 - fSCenter) * fSScale + (1.0 - fSCenter) * fSScale;
    fTScale3 = (1.0 - fTCenter) * fTScale + (1.0 - fTCenter) * fTScale;

    // create the texture axis

    if (orientation) {
        VectorNormalize2(dir, info.axis[0]);
        PerpendicularVector(info.axis[1], info.axis[0]);
        RotatePointAroundVector(info.axis[2], info.axis[0], info.axis[1], orientation);
        CrossProduct(info.axis[0], info.axis[2], info.axis[1]);
    } else {
        vec3_t angles;
        vec3_t tmp;

        VectorNormalize2(dir, info.axis[0]);
        VectorCopy(dir, tmp);
        vectoangles(tmp, angles);
        AnglesToAxis(angles, info.axis);
        VectorScale(info.axis[2], -1, info.axis[2]);
    }

    // create the full polygon
    for (i = 0; i < 3; i++) {
        originalPoints[0][i] = origin[i] - fSScale2 * info.axis[1][i] - fTScale2 * info.axis[2][i];
        originalPoints[1][i] = origin[i] + fSScale3 * info.axis[1][i] - fTScale2 * info.axis[2][i];
        originalPoints[2][i] = origin[i] + fSScale3 * info.axis[1][i] + fTScale3 * info.axis[2][i];
        originalPoints[3][i] = origin[i] - fSScale2 * info.axis[1][i] + fTScale3 * info.axis[2][i];
    }

    // get the fragments
    VectorScale(dir, -32, projection);

    numFragments = CG_GetMarkFragments(4, originalPoints, projection, markPoints, markFragments, fRadiusSquared);

    if (dolighting) {
        vec3_t vLight;
        cgi.R_GetLightingForDecal(vLight, dir, origin);

        info.colors[0] = (int)(red * vLight[0]);
        info.colors[1] = (int)(green * vLight[1]);
        info.colors[2] = (int)(blue * vLight[2]);
    } else {
        info.colors[0] = (int)(red * 255.0f);
        info.colors[1] = (int)(green * 255.0f);
        info.colors[2] = (int)(blue * 255.0f);
    }

    if (fadein) {
        info.colors[3] = 0;
    } else {
        info.colors[3] = (int)(alpha * 255.0);
    }

    info.fSCenter = fSCenter;
    info.fTCenter = fTCenter;
    VectorCopy(origin, info.origin);
    VectorAdd(origin, dir, v);
    info.leafnum = cgi.CM_PointLeafnum(v);

    if (temporary) {
        polyVert_t verts[8];

        for (i = 0, mf = markFragments; i < numFragments; i++, mf++) {
            if (mf->numPoints > 8) {
                mf->numPoints = 8;
            }

            if (CG_ImpactMark_PerPolyCallback(markPoints, mf, verts, (void *)&info)) {
                CG_AddFragmentToScene(mf->iIndex, markShader, mf->numPoints, verts);
            }
        }
    } else {
        CG_AssembleFinalMarks(
            markPoints,
            markFragments,
            numFragments,
            &CG_ImpactMark_PerPolyCallback,
            &CG_ImpactMark_GetLeafCallback,
            (void *)&info,
            info.origin,
            sqrt(fRadiusSquared),
            markShader,
            fadein,
            alphaFade
        );
    }
}

void CG_AssembleFinalMarks(
    vec3_t         *markPoints,
    markFragment_t *markFragments,
    int             numFragments,
    qboolean (*PerPolyCallback)(const vec3_t *markPoints, markFragment_t *mf, polyVert_t *verts, void *pCustom),
    int (*GetLeafCallback)(markFragment_t *mf, void *pCustom),
    void     *pCustom,
    vec3_t    pos,
    float     radius,
    qhandle_t markShader,
    qboolean  fadein,
    qboolean  alphaFade
)
{
    markObj_t      *pMark;
    markPoly_t     *pPoly;
    int             iGroup, iFirstNewGroup;
    int             numFragsForMark;
    int             i;
    markFragment_t *mf;

    iFirstNewGroup = 0;
    do {
        i               = iFirstNewGroup;
        mf              = &markFragments[iFirstNewGroup];
        iGroup          = mf->iIndex;
        numFragsForMark = 0;

        for (; i < numFragments; i++, mf++) {
            if (iGroup < 0 && mf->iIndex == iGroup) {
                numFragsForMark++;
            } else if (iGroup >= 0 && mf->iIndex >= 0) {
                numFragsForMark++;
            }
        }

        pMark = CG_AllocMark(numFragsForMark);
        if (!pMark) {
            break;
        }

        pMark->time       = cg.time;
        pMark->alphaFade  = alphaFade;
        pMark->markShader = markShader;
        pMark->fadein     = fadein;
        VectorCopy(pos, pMark->pos);
        pMark->radius = radius;

        i              = iFirstNewGroup;
        mf             = &markFragments[iFirstNewGroup];
        iFirstNewGroup = 0;

        if (mf->iIndex < 0) {
            centity_t *cent;
            vec3_t pos;

            cent = &cg_entities[-mf->iIndex];

            VectorSubtract(pMark->pos, cent->lerpOrigin, pos);

            if (cent->lerpAngles[0] || cent->lerpAngles[1] || cent->lerpAngles[2]) {
                vec3_t axis[3];

                // Fixed in OPM
                //  Make the position completely local to the entity
                AngleVectorsLeft(cent->lerpAngles, axis[0], axis[1], axis[2]);
                MatrixTransformVectorRight(axis, pos, pMark->pos);
            } else {
                VectorCopy(pos, pMark->pos);
            }
        }

        pPoly = pMark->markPolys;
        for (; i < numFragments; i++, mf++) {
            if (iGroup < 0 && mf->iIndex != iGroup) {
                if (!iFirstNewGroup) {
                    iFirstNewGroup = i;
                }
            } else if (iGroup >= 0 && mf->iIndex < 0) {
                if (!iFirstNewGroup) {
                    iFirstNewGroup = i;
                }
            } else {
                if (mf->numPoints > 8) {
                    mf->numPoints = 8;
                }

                if (PerPolyCallback(markPoints, mf, pPoly->verts, pCustom)) {
                    pPoly->numVerts = mf->numPoints;
                    pPoly->iIndex   = mf->iIndex;
                    pPoly           = pPoly->nextPoly;
                }
            }
        }

        pMark->leafnum = GetLeafCallback(mf, pCustom);
    } while (iFirstNewGroup);
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
    int         t;
    int         fade;
    markObj_t  *pMark;
    markObj_t  *pNext;
    markPoly_t *pPoly;
    polyVert_t *pVert;
    polyVert_t  tmpVerts[8];
    int         viewleafnum;

    if (!cg_bMarksInitialized) {
        return;
    }

    if (!cg_addMarks->integer) {
        return;
    }

    viewleafnum = cgi.CM_PointLeafnum(cg.refdef.vieworg);
    for (pMark = cg_activeMarkObjs.nextMark; pMark != &cg_activeMarkObjs; pMark = pNext) {
        // grab next now, so if the local entity is freed we
        // still have it
        pNext = pMark->nextMark;

        // see if it is time to completely remove it
        if (pMark->alphaFade && cg.time > pMark->time + MARK_TOTAL_TIME) {
            CG_FreeMarkObj(pMark);
            continue;
        }

        if (pMark->markPolys->iIndex < 0 && !CG_UpdateMarkPosition(pMark)) {
            if (pMark->lastVisTime < cg.time - 3000) {
                CG_FreeMarkObj(pMark);
            }
            continue;
        }

        if (!cgi.CM_LeafInPVS(viewleafnum, pMark->leafnum)) {
            continue;
        }

        if (pMark->markPolys->iIndex < 0) {
            vec3_t vWorldPos;
            CG_FragmentPosToWorldPos(pMark->pos, vWorldPos);
            if (CG_FrustumCullSphere(vWorldPos, pMark->radius)) {
                continue;
            }
        } else {
            if (CG_FrustumCullSphere(pMark->pos, pMark->radius)) {
                continue;
            }
        }

        pMark->lastVisTime = cg.time;
        if (pMark->fadein) {
            fade = 255 * (cg.time - pMark->time) / MARK_FADE_TIME;
            if (fade > 255) {
                fade          = 255;
                pMark->fadein = 0;
            }

            for (pPoly = pMark->markPolys; pPoly; pPoly = pPoly->nextPoly) {
                for (j = 0; j < pPoly->numVerts; j++) {
                    pPoly->verts[j].modulate[3] = fade;
                }
            }
        }

        if (pMark->alphaFade) {
            t = pMark->time + MARK_TOTAL_TIME - cg.time;
            if (t < MARK_FADE_TIME) {
                fade = 255 * t / MARK_FADE_TIME;
                for (pPoly = pMark->markPolys; pPoly; pPoly = pPoly->nextPoly) {
                    for (j = 0; j < pPoly->numVerts; j++) {
                        pPoly->verts[j].modulate[3] = fade;
                    }
                }
            }
        }

        for (pPoly = pMark->markPolys; pPoly; pPoly = pPoly->nextPoly) {
            if (pPoly->iIndex < 0) {
                memcpy(tmpVerts, pPoly->verts, 24 * pPoly->numVerts);
                pVert = tmpVerts;
            } else {
                pVert = pPoly->verts;
            }

            CG_AddFragmentToScene(pPoly->iIndex, pMark->markShader, pPoly->numVerts, pVert);
        }
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

    if (cg_entities[iEntIndex].currentState.modelindex < 0
        || cg_entities[iEntIndex].currentState.modelindex > cgi.CM_NumInlineModels()) {
        return qfalse;
    }

    return qtrue;
}

void CG_InitTestTreadMark()
{
    cg_treadmark_test = cgi.Cvar_Get("cg_treadmark_test", "0", 0);
}

int CG_StartTreadMark(int iReference, qhandle_t treadShader, const vec3_t vStartPos, float fWidth, float fAlpha)
{
    int          i;
    int          iTreadNum;
    treadMark_t *pTread;

    if (!cg_bMarksInitialized) {
        return -1;
    }

    if (!cg_addMarks->integer) {
        return -1;
    }

    iTreadNum = -1;
    for (i = 0; i < MAX_TREAD_MARKS; i++) {
        if (!cg_treadMarks[i].iState) {
            iTreadNum = i;
            pTread    = &cg_treadMarks[i];
            break;
        }
    }

    if (iTreadNum == -1) {
        return -1;
    }

    memset(pTread, 0, sizeof(*pTread));
    pTread->iState           = 1;
    pTread->iReferenceNumber = iReference;
    pTread->iLastTime        = cg.time;
    pTread->hTreadShader     = treadShader;
    pTread->fWidth           = fWidth * 0.5;
    VectorCopy(vStartPos, pTread->vMidPos);
    VectorCopy(vStartPos, pTread->vEndPos);

    if (fAlpha < 0.0) {
        pTread->fMidAlpha = 255.0;
    } else {
        pTread->fMidAlpha = fAlpha * 255.0;
    }

    pTread->fEndAlpha = pTread->fMidAlpha;
    return iTreadNum;
}

qboolean
CG_MakeTreadMarkDecal_PerPolyCallback(const vec3_t *markPoints, markFragment_t *mf, polyVert_t *verts, void *pCustom)
{
    int                 j;
    float               fDist;
    float               fSideDist;
    float               fSideAlpha;
    float               fCenterAlpha;
    float               fFrac;
    float               fTex, fAlpha;
    polyVert_t         *v;
    cg_treadmarkinfo_t *pInfo;

    pInfo = (cg_treadmarkinfo_t *)pCustom;

    if (mf->iIndex < 0) {
        vec3_t vWorldPos;

        if (!CG_GetMarkInlineModelOrientation(mf->iIndex)) {
            return qfalse;
        }

        for (j = 0; j < mf->numPoints; j++) {
            v = &verts[j];
            VectorCopy(markPoints[mf->firstPoint + j], v->xyz);
            v->modulate[0] = pInfo->colors[0];
            v->modulate[1] = pInfo->colors[1];
            v->modulate[2] = pInfo->colors[2];
            v->modulate[3] = pInfo->colors[3];

            CG_FragmentPosToWorldPos(v->xyz, vWorldPos);
            fSideDist = DotProduct(pInfo->vRight, vWorldPos) - pInfo->fRightCenterDist;
            v->st[0]  = (fSideDist + pInfo->pTread->fWidth) * pInfo->fOODoubleWidth;

            fSideAlpha = fSideDist * pInfo->fOOWidth;
            fDist      = DotProduct(vWorldPos, pInfo->vDirection);
            if (fSideAlpha < 0) {
                fSideAlpha = -fSideAlpha;
                v->st[1]   = pInfo->fStartTex + ((fDist - pInfo->fLeftStartDist) * pInfo->fLeftTexScale * fSideAlpha)
                         + ((fDist - pInfo->fStartDist) * pInfo->fCenterTexScale * (1.0 - fSideAlpha));
                v->modulate[3] = pInfo->fStartAlpha
                               + (1.0 - fSideAlpha)
                                     * ((fDist - pInfo->fStartDist) * pInfo->fCenterTexScale * pInfo->fCenterAlphaScale)
                               + (fDist - pInfo->fLeftStartDist) * pInfo->fLeftAlphaScale * fSideAlpha;
            } else {
                v->st[1] = pInfo->fStartTex + ((fDist - pInfo->fRightStartDist) * pInfo->fRightTexScale * fSideAlpha)
                         + ((fDist - pInfo->fStartDist) * pInfo->fCenterTexScale * (1.0 - fSideAlpha));
                v->modulate[3] = pInfo->fStartAlpha
                               + (1.0 - fSideAlpha)
                                     * ((fDist - pInfo->fStartDist) * pInfo->fCenterTexScale * pInfo->fCenterAlphaScale)
                               + (fDist - pInfo->fRightStartDist) * pInfo->fRightAlphaScale * fSideAlpha;
            }
        }
    } else {
        for (j = 0; j < mf->numPoints; j++) {
            v = &verts[j];
            VectorCopy(markPoints[mf->firstPoint + j], v->xyz);
            v->modulate[0] = pInfo->colors[0];
            v->modulate[1] = pInfo->colors[1];
            v->modulate[2] = pInfo->colors[2];
            v->modulate[3] = pInfo->colors[3];

            fSideDist = DotProduct(pInfo->vRight, v->xyz) - pInfo->fRightCenterDist;
            v->st[0]  = (fSideDist + pInfo->pTread->fWidth) * pInfo->fOODoubleWidth;

            fSideAlpha = fSideDist * pInfo->fOOWidth;
            fDist      = DotProduct(v->xyz, pInfo->vDirection);
            if (fSideAlpha < 0) {
                fSideAlpha = -fSideAlpha;
                fTex       = (fDist - pInfo->fLeftStartDist) * pInfo->fLeftTexScale;
                fAlpha     = (fDist - pInfo->fLeftStartDist) * pInfo->fLeftAlphaScale;
            } else {
                fTex   = (fDist - pInfo->fRightStartDist) * pInfo->fRightTexScale;
                fAlpha = (fDist - pInfo->fRightStartDist) * pInfo->fRightAlphaScale;
            }

            v->st[1] = pInfo->fStartTex + (fTex * fSideAlpha)
                     + (fDist - pInfo->fStartDist) * pInfo->fCenterTexScale * (1.0 - fSideAlpha);
            v->modulate[3] = pInfo->fStartAlpha
                           + ((1.0 - fSideAlpha) * ((fDist - pInfo->fStartDist) * pInfo->fCenterAlphaScale))
                           + (fAlpha * fSideAlpha);
        }
    }

    return qtrue;
}

int CG_MakeTreadMarkDecal_GetLeafCallback(markFragment_t *mf, void *pCustom)
{
    return ((cg_treadmarkinfo_t *)pCustom)->leafnum;
}

void CG_MakeTreadMarkDecal(treadMark_t *pTread, qboolean bStartSegment, qboolean bTemporary)
{
    int                i;
    int                numFragments;
    vec3_t             originalPoints[4];
    vec3_t             markPoints[MAX_MARK_POLYVERTS];
    vec3_t             projection;
    markFragment_t     markFragments[MAX_MARK_FRAGMENTS];
    markFragment_t    *mf;
    cg_treadmarkinfo_t info;
    float              fEndAlpha, fEndTex;
    float              fDist;
    vec3_t             vStartCenter;
    vec3_t             vEndCenter;
    vec3_t             vDelta;
    vec3_t             origin;
    float              fRadiusSquared;

    if (bStartSegment) {
        VectorCopy(pTread->vStartVerts[1], originalPoints[0]);
        VectorCopy(pTread->vStartVerts[0], originalPoints[1]);
        VectorCopy(pTread->vMidVerts[0], originalPoints[2]);
        VectorCopy(pTread->vMidVerts[1], originalPoints[3]);

        info.fStartAlpha = pTread->fStartAlpha;
        fEndAlpha        = pTread->fMidAlpha;
        info.fStartTex   = pTread->fStartTexCoord;
        fEndTex          = pTread->fMidTexCoord;

        // Calculate the start center
        VectorAdd(originalPoints[1], originalPoints[0], vStartCenter);
        VectorScale(vStartCenter, 0.5, vStartCenter);
        VectorCopy(pTread->vMidPos, vEndCenter);
        VectorCopy(pTread->vStartDir, info.vDirection);
    } else {
        VectorCopy(pTread->vMidVerts[1], originalPoints[0]);
        VectorCopy(pTread->vMidVerts[0], originalPoints[1]);
        VectorCopy(pTread->vEndVerts[0], originalPoints[2]);
        VectorCopy(pTread->vEndVerts[1], originalPoints[3]);

        info.fStartAlpha = pTread->fMidAlpha;
        fEndAlpha        = pTread->fEndAlpha;
        info.fStartTex   = pTread->fMidTexCoord;
        fEndTex          = pTread->fEndTexCoord;

        VectorCopy(pTread->vMidPos, vStartCenter);
        VectorCopy(pTread->vEndPos, vEndCenter);

        // Calculate the direction
        VectorSubtract(vEndCenter, vStartCenter, info.vDirection);
        VectorNormalizeFast(info.vDirection);
    }

    CrossProduct(vec_upwards, info.vDirection, info.vRight);
    info.fRightCenterDist = DotProduct(vEndCenter, info.vRight);
    info.fOODoubleWidth   = 0.5 / pTread->fWidth;
    info.fOOWidth         = info.fOODoubleWidth * 2;
    info.fStartDist       = DotProduct(info.vDirection, vStartCenter);
    info.fRightStartDist  = DotProduct(info.vDirection, originalPoints[1]);
    info.fLeftStartDist   = DotProduct(info.vDirection, originalPoints[0]);

    //
    // Center
    //

    VectorSubtract(vEndCenter, vStartCenter, vDelta);
    fDist = VectorLength(vDelta);

    fRadiusSquared = (Square(fDist) + Square(pTread->fWidth)) * 0.25;

    info.fCenterTexScale   = (fEndTex - info.fStartTex) / fDist;
    info.fCenterAlphaScale = (fEndAlpha - info.fStartAlpha) / fDist;

    //
    // Right
    //

    VectorSubtract(originalPoints[2], originalPoints[1], vDelta);
    fDist = VectorLength(vDelta);

    info.fRightTexScale   = (fEndTex - info.fStartTex) / fDist;
    info.fRightAlphaScale = (fEndAlpha - info.fStartAlpha) / fDist;

    //
    // Left
    //

    VectorSubtract(originalPoints[3], originalPoints[1], vDelta);
    fDist = VectorLength(vDelta);

    info.fLeftTexScale   = (fEndTex - info.fStartTex) / fDist;
    info.fLeftAlphaScale = (fEndAlpha - info.fStartAlpha) / fDist;

    VectorSet(projection, 0, 0, -32);
    info.colors[0] = info.colors[1] = info.colors[2] = info.colors[3] = -1;

    numFragments = CG_GetMarkFragments(4, originalPoints, projection, markPoints, markFragments, fRadiusSquared);
    VectorAdd(vStartCenter, vEndCenter, origin);
    VectorScale(origin, 0.5, origin);
    info.leafnum = cgi.CM_PointLeafnum(origin);
    info.pTread  = pTread;

    if (bTemporary) {
        for (i = 0; i < numFragments; i++) {
            polyVert_t verts[8];

            mf = &markFragments[i];
            if (mf->numPoints > 8) {
                mf->numPoints = 8;
            }
            if (CG_MakeTreadMarkDecal_PerPolyCallback(markPoints, mf, verts, &info)) {
                CG_AddFragmentToScene(mf->iIndex, pTread->hTreadShader, mf->numPoints, verts);
            }
        }
    } else {
        CG_AssembleFinalMarks(
            markPoints,
            markFragments,
            numFragments,
            &CG_MakeTreadMarkDecal_PerPolyCallback,
            &CG_MakeTreadMarkDecal_GetLeafCallback,
            &info,
            origin,
            sqrt(fRadiusSquared),
            pTread->hTreadShader,
            qfalse,
            qfalse
        );
    }
}

int CG_UpdateTreadMark(int iReference, const vec3_t vNewPos, float fAlpha)
{
    int          i;
    int          iTreadNum;
    float        fDist;
    float        fSplitLength;
    float        fNewLength;
    float        fTmp;
    qboolean     bDoSegmentation;
    vec3_t       vDelta, vDeltaNorm;
    vec3_t       vDir, vMidDir;
    vec3_t       vRight;
    treadMark_t *pTread;

    iTreadNum = -1;

    for (i = 0; i < MAX_TREAD_MARKS; i++) {
        if (cg_treadMarks[i].iReferenceNumber == iReference) {
            iTreadNum = i;
            pTread    = &cg_treadMarks[i];
        }
    }

    if (iTreadNum == -1 || !pTread->iState) {
        return -1;
    }

    pTread->iLastTime = cg.time;

    if (VectorCompare(pTread->vEndPos, vNewPos)) {
        if (fAlpha >= 0) {
            pTread->fEndAlpha = fAlpha * 255.0;
        }
        return 0;
    }

    VectorSubtract(vNewPos, pTread->vMidPos, vDelta);
    fDist = VectorNormalize2(vDelta, vDeltaNorm);
    if (pTread->iState == 1) {
        fSplitLength    = 0;
        bDoSegmentation = qfalse;
    } else {
        VectorAdd(vDeltaNorm, pTread->vStartDir, vMidDir);
        VectorScale(vMidDir, 0.5, vMidDir);
        VectorNormalizeFast(vMidDir);

        fTmp         = DotProduct(vMidDir, vDeltaNorm);
        fSplitLength = pTread->fWidth / fTmp;

        if (fTmp < -0.5) {
            fTmp = fTmp
                 * (fTmp
                        * (fTmp
                               * (fTmp * (fTmp * (fTmp * -337.31875783205 + -1237.54375255107) + -1802.11467325687)
                                  + -1303.19904613494)
                           + -471.347871690988)
                    + -70.0883838161826);
        } else if (fTmp > 0.5) {
            fTmp = fTmp
                     * (fTmp
                            * (fTmp
                                   * (fTmp * (fTmp * (fTmp * -1507.55394345521 + 6580.58002318442) + -11860.0735285953)
                                      + 11290.7510782536)
                               + -5986.89654545347)
                        + 1675.66417006387)
                 + -192.426950291139;
        } else {
            fTmp = fTmp * (fTmp * -0.531387674508458 + -2.11e-14) + 1.00086138065435;
        }

        fNewLength = fSplitLength * fTmp;
        if (fNewLength + 24 > fDist) {
            bDoSegmentation = qfalse;
        } else if (fSplitLength < -1) {
            fSplitLength    = -fSplitLength;
            bDoSegmentation = qtrue;
        } else if (fDist > 256) {
            bDoSegmentation = qtrue;
        } else {
            CrossProduct(vec_upwards, pTread->vStartDir, vRight);
            VectorNormalizeFast(vRight);
            bDoSegmentation = fabs(DotProduct(vRight, vDelta)) > 16;
        }
    }

    if (pTread->iState == 2 && bDoSegmentation) {
        VectorCopy(pTread->vMidVerts[0], pTread->vStartVerts[0]);
        VectorCopy(pTread->vMidVerts[1], pTread->vStartVerts[1]);
        pTread->fMidTexCoord = pTread->fEndTexCoord;
        pTread->fMidAlpha    = pTread->fEndAlpha;
        VectorCopy(vNewPos, pTread->vEndPos);

        if (fAlpha >= 0) {
            pTread->fEndAlpha = fAlpha * 255.0;
        }

        pTread->iState = 3;
    } else if (pTread->iState == 0 || pTread->iState == 1) {
        VectorCopy(vNewPos, pTread->vEndPos);

        if (fAlpha >= 0) {
            pTread->fEndAlpha = fAlpha * 255.0;
        }

        CrossProduct(vec_upwards, vDeltaNorm, vDir);
        VectorMA(pTread->vMidPos, pTread->fWidth, vDir, pTread->vMidVerts[0]);
        VectorMA(pTread->vMidPos, 0.0 - pTread->fWidth, vDir, pTread->vMidVerts[1]);
        VectorMA(pTread->vEndPos, pTread->fWidth, vDir, pTread->vEndVerts[0]);
        VectorMA(pTread->vEndPos, 0.0 - pTread->fWidth, vDir, pTread->vEndVerts[1]);

        pTread->fEndTexCoord = fDist / 32.0;

        if (pTread->iState == 1 && fDist > 8) {
            VectorCopy(vDelta, pTread->vStartDir);
            pTread->iState = 2;
        }
        return 0;
    } else {
        if (bDoSegmentation) {
            CG_MakeTreadMarkDecal(pTread, qtrue, qfalse);

            VectorCopy(pTread->vMidVerts[0], pTread->vStartVerts[0]);
            VectorCopy(pTread->vMidVerts[1], pTread->vStartVerts[1]);
            VectorSubtract(pTread->vEndPos, pTread->vMidPos, pTread->vStartDir);
            VectorNormalizeFast(pTread->vStartDir);
            VectorCopy(pTread->vEndPos, pTread->vMidPos);
            pTread->fMidTexCoord = pTread->fEndTexCoord;
            pTread->fMidAlpha    = pTread->fEndAlpha;

            if (pTread->fStartTexCoord >= 1.0) {
                pTread->fMidTexCoord -= (int)pTread->fStartTexCoord;
                pTread->fStartTexCoord -= (int)pTread->fStartTexCoord;
            }
        }

        VectorCopy(vNewPos, pTread->vEndPos);
        pTread->fEndTexCoord = pTread->fMidTexCoord + fDist / 32.0;

        if (fAlpha >= 0) {
            pTread->fEndAlpha = fAlpha * 255.0;
        }
    }

    CrossProduct(vec_upwards, vMidDir, vRight);
    VectorNormalizeFast(vRight);
    VectorMA(pTread->vMidPos, fSplitLength, vRight, pTread->vMidVerts[0]);
    VectorMA(pTread->vMidPos, 0.0 - fSplitLength, vRight, pTread->vMidVerts[1]);
    CrossProduct(vec_upwards, vDeltaNorm, vRight);
    VectorNormalizeFast(vRight);
    VectorMA(pTread->vEndPos, pTread->fWidth, vRight, pTread->vEndVerts[0]);
    VectorMA(pTread->vEndPos, 0.0 - pTread->fWidth, vRight, pTread->vEndVerts[1]);

    return 0;
}

void CG_AddTreadMarks()
{
    treadMark_t *pTread;
    trace_t      trace;
    vec3_t       vPos, vEnd;
    int          i;

    if (cg_treadmark_test->integer) {
        VectorCopy(cg.predicted_player_state.origin, vPos);
        VectorCopy(vPos, vEnd);
        vPos[2] += 32.0f;
        vEnd[2] -= 128.0f;

        CG_Trace(
            &trace,
            vPos,
            vec3_origin,
            vec3_origin,
            vEnd,
            cg.snap->ps.clientNum,
            MASK_SHOT,
            qfalse,
            qtrue,
            "CG_AddTreadMarks test"
        );

        if (trace.fraction < 1.0 && CG_UpdateTreadMark(1, trace.endpos, 1.0) == -1) {
            qhandle_t shader = cgi.R_RegisterShader("testtread");

            CG_StartTreadMark(1, shader, trace.endpos, cg_treadmark_test->integer, 1.0);
        }
    }

    for (i = 0; i < MAX_TREAD_MARKS; i++) {
        pTread = &cg_treadMarks[i];

        if (!pTread->iState) {
            continue;
        }

        if (cg.time - pTread->iLastTime > 500) {
            if (pTread->iState == 3) {
                CG_MakeTreadMarkDecal(pTread, qtrue, qfalse);
            }
            CG_MakeTreadMarkDecal(pTread, qfalse, qfalse);
            pTread->iState = 0;
        } else {
            if (pTread->iState == 3) {
                CG_MakeTreadMarkDecal(pTread, qtrue, qtrue);
            }
            CG_MakeTreadMarkDecal(pTread, qfalse, qtrue);
        }
    }
}

int CG_PermanentMark(
    vec3_t          origin,
    vec3_t          dir,
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
    byte            colors[4];
    int             i, j;
    int             numFragments;
    float           fSScale2, fTScale2;
    float           fSScale3, fTScale3;
    float           texCoordScaleS, texCoordScaleT;
    vec3_t          originalPoints[4];
    vec3_t          markPoints[MAX_MARK_POLYVERTS];
    vec3_t          projection;
    vec3_t          vLight;
    vec3_t          vTmp;
    vec3_t          axis[3];
    markFragment_t *mf;
    polyVert_t     *pPolyVerts;
    polyVert_t     *v;
    clipHandle_t    cmodel;
    trace_t         trace;
    float           fRadiusSquared;

    pPolyVerts = (polyVert_t *)pVoidPolyVerts;

    VectorMA(origin, -2048, dir, vTmp);
    VectorAdd(origin, dir, origin);
    CG_Trace(
        &trace, origin, vec3_origin, vec3_origin, vTmp, ENTITYNUM_NONE, MASK_MARK, qfalse, qtrue, "CG_PermanentMark"
    );

    if (trace.fraction == 1) {
        return 0;
    }

    VectorCopy(trace.endpos, origin);
    VectorCopy(trace.plane.normal, dir);

    if (!fSScale) {
        fSScale = 1.0;
    }
    if (!fTScale) {
        fTScale = 1.0;
    }
    texCoordScaleT = 0.5 / fTScale;
    texCoordScaleS = 0.5 / fSScale;
    if (fSCenter < 0.0 || fSCenter > 1.0) {
        fSCenter = 0.5;
    }
    if (fTCenter < 0.0 || fTCenter > 1.0) {
        fTCenter = 0.5;
    }
    fRadiusSquared = fSScale * fSScale;
    fRadiusSquared = fTScale * fTScale + fRadiusSquared;
    fSScale2       = (fSScale * (1.0 - fSCenter)) * 2;
    fTScale2       = (fTScale * (1.0 - fTCenter)) * 2;
    fSScale3       = fSCenter * 2 * fSScale;
    fTScale3       = fTCenter * 2 * fTScale;

    VectorNormalize2(dir, axis[0]);
    PerpendicularVector(axis[1], axis[0]);
    RotatePointAroundVector(axis[2], axis[0], axis[1], orientation);
    CrossProduct(axis[0], axis[2], axis[1]);

    // create the full polygon
    for (i = 0; i < 3; i++) {
        originalPoints[0][i] = origin[i] - fSScale3 * axis[1][i] - fTScale3 * axis[2][i];
        originalPoints[1][i] = origin[i] + fSScale2 * axis[1][i] - fTScale3 * axis[2][i];
        originalPoints[2][i] = origin[i] + fSScale2 * axis[1][i] + fTScale2 * axis[2][i];
        originalPoints[3][i] = origin[i] - fSScale3 * axis[1][i] + fTScale2 * axis[2][i];
    }

    VectorScale(dir, -32, projection);
    numFragments = CG_GetMarkFragments(
        ARRAY_LEN(originalPoints), originalPoints, projection, markPoints, pMarkFragments, fRadiusSquared
    );

    if (!dolighting) {
        colors[0] = (int)(red * 255.0f);
        colors[1] = (int)(green * 255.0f);
        colors[2] = (int)(blue * 255.0f);
    }

    colors[3] = (int)(alpha * 255.0f);

    for (i = 0, mf = pMarkFragments; i < numFragments; i++, mf++) {
        vec3_t vWorldPos;
        vec3_t delta;

        if (mf->numPoints > 8) {
            mf->numPoints = 8;
        }

        if (mf->iIndex >= 0) {
            for (j = 0; j < mf->numPoints; j++) {
                v = &pPolyVerts[mf->firstPoint + j];
                VectorCopy(markPoints[mf->firstPoint + j], v->xyz);

                if (dolighting) {
                    cgi.R_GetLightingForDecal(vLight, dir, v->xyz);

                    colors[0] = (int)(red * vLight[0]);
                    colors[1] = (int)(green * vLight[1]);
                    colors[2] = (int)(blue * vLight[2]);
                }

                v->modulate[0] = colors[0];
                v->modulate[1] = colors[1];
                v->modulate[2] = colors[2];
                v->modulate[3] = colors[3];

                VectorSubtract(v->xyz, origin, delta);
                v->st[0] = fSCenter + DotProduct(delta, axis[1]) * texCoordScaleS;
                v->st[1] = fTCenter + DotProduct(delta, axis[2]) * texCoordScaleT;
            }
        } else if (CG_GetMarkInlineModelOrientation(mf->iIndex)) {
            for (j = 0; j < mf->numPoints; j++) {
                v = &pPolyVerts[mf->firstPoint + j];
                VectorCopy(markPoints[mf->firstPoint + j], v->xyz);

                if (dolighting) {
                    CG_FragmentPosToWorldPos(v->xyz, vWorldPos);
                    cgi.R_GetLightingForDecal(vLight, dir, v->xyz);

                    colors[0] = (int)(red * vLight[0]);
                    colors[1] = (int)(green * vLight[1]);
                    colors[2] = (int)(blue * vLight[2]);
                }

                v->modulate[0] = colors[0];
                v->modulate[1] = colors[1];
                v->modulate[2] = colors[2];
                v->modulate[3] = colors[3];

                VectorSubtract(v->xyz, origin, delta);
                v->st[0] = fSCenter + DotProduct(delta, axis[1]) * texCoordScaleS;
                v->st[1] = fTCenter + DotProduct(delta, axis[2]) * texCoordScaleT;
            }

            mf->iIndex = -cgi.CM_InlineModel(cg_entities[-mf->iIndex].currentState.modelindex);
        } else {
            mf->numPoints = 0;
        }
    }

    return numFragments;
}

int CG_PermanentTreadMarkDecal(
    treadMark_t    *pTread,
    qboolean        bStartSegment,
    qboolean        dolighting,
    markFragment_t *pMarkFragments,
    void           *pVoidPolyVerts
)
{
    byte            colors[4];
    int             i, j;
    int             numFragments;
    vec3_t          originalPoints[4];
    vec3_t          markPoints[MAX_MARK_POLYVERTS];
    vec3_t          projection;
    vec3_t          vLight;
    markFragment_t *mf;
    polyVert_t     *pPolyVerts;
    polyVert_t     *v;
    clipHandle_t    cmodel;
    float           fStartAlpha, fEndAlpha;
    float           fStartTex, fEndTex;
    float           fRightCenterDist;
    float           fOOWidth, fOODoubleWidth;
    float           fDist, fSideDist;
    float           fFrac;
    float           fStartDist, fRightStartDist, fLeftStartDist;
    float           fCenterTexScale, fRightTexScale, fLeftTexScale;
    float           fCenterAlphaScale, fRightAlphaScale, fLeftAlphaScale;
    float           fSideAlpha, fCenterAlpha;
    vec3_t          vStartCenter, vEndCenter;
    vec3_t          vDirection;
    vec3_t          vRight;
    vec3_t          vDelta;
    float           fRadiusSquared;

    pPolyVerts = (polyVert_t *)pVoidPolyVerts;

    if (bStartSegment) {
        VectorCopy(pTread->vStartVerts[1], originalPoints[0]);
        VectorCopy(pTread->vStartVerts[0], originalPoints[1]);
        VectorCopy(pTread->vMidVerts[0], originalPoints[2]);
        VectorCopy(pTread->vMidVerts[1], originalPoints[3]);

        fStartAlpha = pTread->fStartAlpha;
        fEndAlpha   = pTread->fMidAlpha;
        fStartTex   = pTread->fStartTexCoord;
        fEndTex     = pTread->fMidTexCoord;

        // Calculate the start center
        vStartCenter[0] = (originalPoints[1][0] + originalPoints[0][0]) * 0.5;
        vStartCenter[1] = (originalPoints[1][1] + originalPoints[0][1]) * 0.5;
        vStartCenter[2] = (originalPoints[1][2] + originalPoints[0][2]) * 0.5;
        VectorCopy(pTread->vMidPos, vEndCenter);
        VectorCopy(pTread->vStartDir, vDirection);
    } else {
        VectorCopy(pTread->vMidVerts[1], originalPoints[0]);
        VectorCopy(pTread->vMidVerts[0], originalPoints[1]);
        VectorCopy(pTread->vEndVerts[0], originalPoints[2]);
        VectorCopy(pTread->vEndVerts[1], originalPoints[3]);

        fStartAlpha = pTread->fMidAlpha;
        fEndAlpha   = pTread->fEndAlpha;
        fStartTex   = pTread->fMidTexCoord;
        fEndTex     = pTread->fEndTexCoord;

        VectorCopy(pTread->vMidPos, vStartCenter);
        VectorCopy(pTread->vEndPos, vEndCenter);

        // Calculate the direction
        VectorSubtract(vEndCenter, vStartCenter, vDirection);
        VectorNormalizeFast(vDirection);
    }

    CrossProduct(vec_upwards, vDirection, vRight);
    fRightCenterDist = DotProduct(vEndCenter, vRight);
    fOOWidth         = 1.0 / pTread->fWidth;
    fOODoubleWidth   = 1.0 / (pTread->fWidth * 2);
    fStartDist       = DotProduct(vDirection, vStartCenter);
    fRightStartDist  = DotProduct(vDirection, originalPoints[1]);
    fLeftStartDist   = DotProduct(vDirection, originalPoints[1]);

    //
    // Calculate center
    //
    VectorSubtract(vEndCenter, vStartCenter, vDelta);
    fDist             = VectorLength(vDelta);
    fCenterTexScale   = (fEndTex - fStartTex) / fDist;
    fCenterAlphaScale = (fEndAlpha - fStartAlpha) / fDist;
    fSideDist         = fDist;

    //
    // Calculate right
    //
    VectorSubtract(originalPoints[2], originalPoints[1], vDelta);
    fDist            = VectorLength(vDelta);
    fRightTexScale   = (fEndTex - fStartTex) / fDist;
    fRightAlphaScale = (fEndAlpha - fStartAlpha) / fDist;

    //
    // Calculate left
    //
    VectorSubtract(originalPoints[3], originalPoints[0], vDelta);
    fDist           = VectorLength(vDelta);
    fLeftTexScale   = (fEndTex - fStartTex) / fDist;
    fLeftAlphaScale = (fEndAlpha - fStartAlpha) / fDist;

    VectorSet(projection, 0, 0, -32);
    colors[0] = colors[1] = colors[2] = colors[3] = 0xff;

    numFragments = CG_GetMarkFragments(
        4, originalPoints, projection, markPoints, pMarkFragments, (Square(pTread->fWidth) + Square(fSideDist)) * 0.25
    );

    for (i = 0; i < numFragments; i++) {
        vec3_t vWorldPos;

        mf = &pMarkFragments[i];

        if (mf->numPoints > 8) {
            mf->numPoints = 8;
        }

        if (mf->iIndex >= 0) {
            for (j = 0; j < mf->numPoints; j++) {
                v = &pPolyVerts[mf->firstPoint + j];
                VectorCopy(markPoints[mf->firstPoint + j], v->xyz);

                if (dolighting) {
                    cgi.R_GetLightingForDecal(vLight, projection, v->xyz);

                    colors[0] = vLight[0];
                    colors[1] = vLight[1];
                    colors[2] = vLight[2];
                }

                fSideDist  = DotProduct(v->xyz, vRight) - fRightCenterDist;
                fSideAlpha = fSideDist * fOOWidth;
                v->st[0]   = (fSideDist + pTread->fWidth) * fOODoubleWidth;

                if (fSideAlpha < 0) {
                    fSideAlpha = -fSideAlpha;
                    v->st[1] =
                        fStartTex + ((DotProduct(v->xyz, vDirection) - fLeftStartDist) * fLeftTexScale * fSideAlpha);
                } else {
                    v->st[1] =
                        fStartTex + ((DotProduct(v->xyz, vDirection) - fRightStartDist) * fRightTexScale * fSideAlpha);
                }

                v->st[1] += (DotProduct(v->xyz, vDirection) - fStartDist) * fCenterTexScale * (1.0 - fSideAlpha);
            }
        } else if (CG_GetMarkInlineModelOrientation(mf->iIndex)) {
            for (j = 0; j < mf->numPoints; j++) {
                v = &pPolyVerts[mf->firstPoint + j];
                VectorCopy(markPoints[mf->firstPoint + j], v->xyz);

                if (dolighting) {
                    cgi.R_GetLightingForDecal(vLight, projection, v->xyz);

                    colors[0] = vLight[0];
                    colors[1] = vLight[1];
                    colors[2] = vLight[2];
                }

                v->modulate[0] = colors[0];
                v->modulate[1] = colors[1];
                v->modulate[2] = colors[2];
                v->modulate[3] = colors[3];

                CG_FragmentPosToWorldPos(v->xyz, vWorldPos);

                fSideDist  = DotProduct(vWorldPos, vRight) - fRightCenterDist;
                fSideAlpha = fSideDist * fOOWidth;
                v->st[0]   = (fSideDist + pTread->fWidth) * fOODoubleWidth;

                if (fSideAlpha < 0) {
                    fSideAlpha = -fSideAlpha;
                    v->st[1] =
                        fStartTex + ((DotProduct(vWorldPos, vDirection) - fLeftStartDist) * fLeftTexScale * fSideAlpha);
                } else {
                    v->st[1] = fStartTex
                             + ((DotProduct(vWorldPos, vDirection) - fRightStartDist) * fRightTexScale * fSideAlpha);
                }

                v->st[1] += (DotProduct(vWorldPos, vDirection) - fStartDist) * fCenterTexScale * (1.0 - fSideAlpha);
            }

            mf->iIndex = -cgi.CM_InlineModel(cg_entities[-mf->iIndex].currentState.modelindex);
        } else {
            mf->numPoints = 0;
        }
    }

    return numFragments;
}

int CG_PermanentUpdateTreadMark(
    treadMark_t *pTread, float fAlpha, float fMinSegment, float fMaxSegment, float fMaxOffset, float fTexScale
)
{
    trace_t  trace;
    float    fDist;
    float    fSplitLength;
    float    fNewLength;
    float    fTmp;
    qboolean bDoSegmentation;
    vec3_t   vPos;
    vec3_t   vEnd;
    vec3_t   vNewPos;
    vec3_t   vDelta, vDeltaNorm;
    vec3_t   vDir, vMidDir;
    vec3_t   vRight;

    VectorCopy(cg.predicted_player_state.origin, vPos);
    VectorCopy(cg.predicted_player_state.origin, vEnd);

    vPos[2] += 32;
    vEnd[2] -= 256;

    CG_Trace(
        &trace,
        vPos,
        vec3_origin,
        vec3_origin,
        vEnd,
        cg.snap->ps.clientNum,
        MASK_TREADMARK,
        qfalse,
        qtrue,
        "CG_PermanentUpdateTreadMark"
    );

    VectorCopy(trace.endpos, vNewPos);

    if (cg.time - pTread->iLastTime > 500) {
        VectorClear(pTread->vStartDir);
        VectorClear(pTread->vStartVerts[0]);
        VectorClear(pTread->vStartVerts[1]);
        pTread->fStartTexCoord = 0;
        pTread->fStartAlpha    = 0;

        VectorClear(pTread->vMidPos);
        VectorClear(pTread->vMidVerts[0]);
        VectorClear(pTread->vMidVerts[1]);
        pTread->fMidTexCoord = 0;
        pTread->fMidAlpha    = 0;

        VectorClear(pTread->vEndPos);
        VectorClear(pTread->vEndVerts[0]);
        VectorClear(pTread->vEndVerts[1]);
        pTread->fEndTexCoord = 0;
        pTread->fEndAlpha    = 0;

        pTread->iState           = 1;
        pTread->iReferenceNumber = 0;
        pTread->iLastTime        = cg.time;
        VectorCopy(vNewPos, pTread->vMidPos);
        VectorCopy(vNewPos, pTread->vEndPos);

        if (fAlpha < 0) {
            pTread->fMidAlpha = 255.0;
        } else {
            pTread->fMidAlpha = fAlpha * 255.0;
        }
        pTread->fEndAlpha = pTread->fMidAlpha;

        return 0;
    }

    if (VectorCompare(pTread->vEndPos, vNewPos)) {
        if (fAlpha < 0) {
            return -1;
        }

        fTmp              = pTread->fEndAlpha;
        pTread->fEndAlpha = fAlpha * 255;

        //if (fabs(fTmp - pTread->fEndAlpha > 0.05)) {
        // Fixed in OPM
        //  Looks like this was accidental
        if (fabs(fTmp - pTread->fEndAlpha) > 0.05) {
            return -1;
        }

        return 0;
    }

    VectorSubtract(vNewPos, pTread->vMidPos, vDelta);
    fDist = VectorNormalize2(vDelta, vDeltaNorm);

    if (pTread->iState == 1) {
        fSplitLength    = 0.0;
        bDoSegmentation = qfalse;
    } else {
        VectorAdd(vDeltaNorm, pTread->vStartDir, vMidDir);
        VectorScale(vMidDir, 0.5, vMidDir);
        VectorNormalizeFast(vMidDir);

        fTmp         = DotProduct(vMidDir, vDeltaNorm);
        fSplitLength = pTread->fWidth / fTmp;

        if (fTmp < -0.5) {
            fNewLength =
                fTmp
                * (fTmp
                       * (fTmp
                              * (fTmp * (fTmp * (fTmp * -337.31875783205 + -1237.54375255107) + -1802.11467325687)
                                 + -1303.19904613494)
                          + -471.347871690988)
                   + -70.0883838161826);
        } else if (fTmp > 0.5) {
            fNewLength =
                fTmp
                    * (fTmp
                           * (fTmp
                                  * (fTmp * (fTmp * (fTmp * -1507.55394345521 + 6580.58002318442) + -11860.0735285953)
                                     + 11290.7510782536)
                              + -5986.89654545347)
                       + 1675.66417006387)
                + -192.426950291139;
        } else {
            fNewLength = fTmp * (fTmp * -0.531387674508458 + -2.11e-14) + 1.00086138065435;
        }

        fNewLength *= fSplitLength;
        if (fNewLength + fMinSegment > fDist) {
            bDoSegmentation = qfalse;
        } else if (fSplitLength < -1) {
            fSplitLength    = -fSplitLength;
            bDoSegmentation = qtrue;
        } else if (fDist > fMaxSegment) {
            bDoSegmentation = qtrue;
        } else {
            CrossProduct(vec_upwards, pTread->vStartDir, vRight);
            VectorNormalizeFast(vRight);

            if (fabs(DotProduct(vRight, vDelta)) > fMaxOffset) {
                bDoSegmentation = qtrue;
            } else {
                bDoSegmentation = qfalse;
            }
        }
    }

    if (pTread->iState == 2 && bDoSegmentation) {
        VectorCopy(pTread->vStartVerts[0], pTread->vMidVerts[0]);
        VectorCopy(pTread->vStartVerts[1], pTread->vMidVerts[1]);
        pTread->fMidTexCoord = pTread->fEndTexCoord;
        pTread->fMidAlpha    = pTread->fEndAlpha;
        VectorCopy(vNewPos, pTread->vEndPos);

        if (fAlpha >= 0) {
            pTread->fEndAlpha = fAlpha * 255.0;
        }

        pTread->iState = 3;

        return 0;
    } else if (pTread->iState == 1 || pTread->iState == 2) {
        VectorCopy(vNewPos, pTread->vEndPos);

        if (fAlpha >= 0.0) {
            pTread->fEndAlpha = fAlpha * 255.0;
        }

        CrossProduct(vec_upwards, vDeltaNorm, vDir);
        VectorMA(pTread->vMidPos, pTread->fWidth, vDir, pTread->vMidVerts[0]);
        VectorMA(pTread->vMidPos, -pTread->fWidth, vDir, pTread->vMidVerts[1]);
        VectorMA(pTread->vEndPos, pTread->fWidth, vDir, pTread->vEndVerts[0]);
        VectorMA(pTread->vEndPos, -pTread->fWidth, vDir, pTread->vEndVerts[1]);

        pTread->fEndTexCoord = fDist * fTexScale;

        if (pTread->iState == 1 && fDist > 8.0) {
            VectorCopy(vDelta, pTread->vStartDir);
            pTread->iState = 2;
        }

        return 0;
    } else if (bDoSegmentation) {
        if (!pTread->iReferenceNumber) {
            pTread->iReferenceNumber = 1;
            return 1;
        }

        pTread->iReferenceNumber = 0;
        VectorCopy(pTread->vStartVerts[0], pTread->vMidVerts[0]);
        VectorCopy(pTread->vStartVerts[1], pTread->vMidVerts[1]);
        VectorSubtract(pTread->vEndPos, pTread->vMidPos, pTread->vStartDir);
        VectorNormalizeFast(pTread->vStartDir);

        VectorCopy(pTread->vEndPos, pTread->vMidPos);
        pTread->fMidTexCoord = pTread->fEndTexCoord;
        pTread->fMidAlpha    = pTread->fEndAlpha;

        if (pTread->fStartTexCoord >= 1.0) {
            pTread->fStartTexCoord = pTread->fStartTexCoord - floor(pTread->fStartTexCoord);
            pTread->fMidTexCoord   = pTread->fMidTexCoord - floor(pTread->fStartTexCoord);
        }

        VectorCopy(vNewPos, pTread->vEndPos);
        pTread->fEndTexCoord = fDist * fTexScale + pTread->fMidTexCoord;
        if (fAlpha >= 0) {
            pTread->fEndAlpha = fAlpha * 255.0;
        }
    }

    CrossProduct(vec_upwards, vMidDir, vRight);
    VectorNormalizeFast(vRight);
    VectorMA(pTread->vMidPos, fSplitLength, vRight, pTread->vMidVerts[0]);
    VectorMA(pTread->vMidPos, -fSplitLength, vRight, pTread->vMidVerts[1]);
    CrossProduct(vec_upwards, vDeltaNorm, vRight);
    VectorNormalizeFast(vRight);
    VectorMA(pTread->vEndPos, pTread->fWidth, vRight, pTread->vEndVerts[0]);
    VectorMA(pTread->vEndPos, -pTread->fWidth, vRight, pTread->vEndVerts[1]);

    return 0;
}
