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

#define MAX_MARK_FRAGMENTS 128
#define MAX_MARK_POINTS    384

static vec3_t cg_vEntAngles;
static vec3_t cg_vEntOrigin;
static vec3_t cg_fEntAxis[3];
static qboolean cg_bEntAnglesSet;

static int cg_iLastEntIndex;
static int cg_iLastEntTime;
static qboolean cg_bLastEntValid;

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

typedef struct cg_impactmarkinfo_s {
    vec3_t axis[3];
    vec3_t origin;
    float fSCenter;
    float fTCenter;
    float texCoordScaleS;
    float texCoordScaleT;
    byte colors[4];
    int leafnum;
} cg_impactmarkinfo_t;

typedef struct cg_treadmarkinfo_s {
    treadMark_t* pTread;
    vec3_t vDirection;
    vec3_t vRight;
    float fStartDist;
    float fStartTex;
    float fStartAlpha;
    float fLeftStartDist;
    float fRightStartDist;
    float fRightCenterDist;
    float fLeftTexScale;
    float fRightTexScale;
    float fCenterTexScale;
    float fLeftAlphaScale;
    float fRightAlphaScale;
    float fCenterAlphaScale;
    float fOOWidth;
    float fOODoubleWidth;
    byte colors[4];
    int leafnum;
} cg_treadmarkinfo_t;

int CG_GetMarkFragments(
    int numVerts,
    const vec3_t* pVerts,
    const vec3_t vProjection,
    const vec3_t* pPointBuffer,
    markFragment_t* pFragmentBuffer,
    float fRadiusSquared
) {
    int i, j;
    int iNumEnts;
    int iCurrPoints, iCurrFragments;
    int iCurrMaxPoints, iCurrMaxFragments;
    int iNewPoints, iNewFragments;
    clipHandle_t cmodel;
    vec3_t vProjectionDir;
    vec3_t vMins, vMaxs;
    vec3_t vTemp;
    vec3_t vAngles, vOrigin;
    const vec3_t* pCurrPoint;
    centity_t* pEntList[64];
    entityState_t* pEnt;
    markFragment_t* pCurrFragment;
    markFragment_t* pFragment;

    iNewFragments = cgi.R_MarkFragments(
        numVerts,
        pVerts,
        vProjection,
        MAX_MARK_POINTS,
        (float*)pPointBuffer,
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

    iCurrPoints = iNewPoints;
    iCurrFragments = iNewFragments;
    iCurrMaxPoints = MAX_MARK_POINTS - iNewPoints;
    iCurrMaxFragments = MAX_MARK_FRAGMENTS - iNewFragments;
    pCurrPoint = &pPointBuffer[iNewPoints];
    pCurrFragment = &pFragmentBuffer[iNewFragments];

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
            (float*)pCurrPoint,
            iCurrMaxFragments,
            pCurrFragment,
            fRadiusSquared
        );

        iCurrFragments += iNewFragments;
        if (iCurrFragments >= MAX_MARK_FRAGMENTS) {
            break;
        }

        iNewPoints = 0;
        for (j = 0, pFragment = pCurrFragment; j < iNewPoints; j++, pFragment++) {
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

static qboolean CG_GetMarkInlineModelOrientation(int iIndex) {
    centity_t* pCEnt;

    if (iIndex == cg_iLastEntIndex && cg_iLastEntTime == cg.time) {
        return cg_bLastEntValid;
    }

    pCEnt = &cg_entities[-iIndex];
    if (pCEnt->currentValid && pCEnt->currentState.modelindex < cgs.inlineDrawModel[ENTITYNUM_NONE])
    {
        VectorCopy(pCEnt->lerpAngles, cg_vEntAngles);
        VectorCopy(pCEnt->lerpOrigin, cg_vEntOrigin);

        if (cg_vEntAngles[0] || cg_vEntAngles[1] || cg_vEntAngles[2]) {
            AngleVectorsLeft(cg_vEntAngles, cg_fEntAxis[0], cg_fEntAxis[1], cg_fEntAxis[2]);
            cg_bEntAnglesSet = qtrue;
        } else if (cg_bEntAnglesSet) {
            AxisClear(cg_fEntAxis);
            cg_bEntAnglesSet = qfalse;
        }

        cg_iLastEntIndex = iIndex;
        cg_iLastEntTime = cg.time;
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

static void CG_FragmentPosToWorldPos(const vec3_t vFrom, vec3_t vTo) {
    if (cg_bEntAnglesSet)
    {
        VectorMA(cg_vEntOrigin, vFrom[0], cg_fEntAxis[0], vTo);
        VectorMA(vTo, vFrom[1], cg_fEntAxis[1], vTo);
        VectorMA(vTo, vFrom[2], cg_fEntAxis[2], vTo);
    }
    else
    {
        vTo[0] = cg_vEntOrigin[0] + *vFrom;
        vTo[1] = cg_vEntOrigin[1] + vFrom[1];
        vTo[2] = cg_vEntOrigin[2] + vFrom[2];
    }
}

qboolean CG_UpdateMarkPosition(markObj_t* pMark) {
    vec3_t v;
    vec3_t pt;
    int iIndex;

    iIndex = pMark->markPolys->iIndex;
    if (!CG_GetMarkInlineModelOrientation(iIndex)) {
        return qfalse;
    }

    VectorCopy(pMark->markPolys->verts[0].xyz, v);

    if (cg_bEntAnglesSet)
    {
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

void CG_AddFragmentToScene(
    int iIndex,
    qhandle_t hShader,
    int iNumVerts,
    polyVert_t* pVerts
) {
    if (!iIndex) {
        cgi.R_AddPolyToScene(hShader, iNumVerts, pVerts, 0);
        return;
    }

    if (iIndex > 0) {
        cgi.R_AddTerrainMarkToScene(iIndex, hShader, iNumVerts, pVerts, 0);
        return;
    }

    if (cg_bEntAnglesSet) {
        int i;
        vec3_t vTmp;
        polyVert_t* pCurrVert;

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
        int i;
        polyVert_t* pCurrVert;

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

int CG_ImpactMark_GetLeafCallback(markFragment_t* mf, void* pCustom) {
    return ((cg_impactmarkinfo_t*)pCustom)->leafnum;
}

qboolean CG_ImpactMark_PerPolyCallback(
    const vec3_t* markPoints,
    markFragment_t* mf,
    polyVert_t* verts,
    void* pCustom
) {
    int j;
    polyVert_t* v;
    cg_impactmarkinfo_t* pInfo;

    pInfo = (cg_impactmarkinfo_t*)pCustom;

    if (mf->iIndex >= 0)
    {
        for (j = 0; j < mf->numPoints; j++) {
            vec3_t delta;

            v = &verts[j];
            VectorCopy(markPoints[j + mf->firstPoint], v->xyz);
            VectorSubtract(v->xyz, pInfo->origin, delta);
            v->st[0] = pInfo->fSCenter + DotProduct(delta, pInfo->axis[1]) * pInfo->texCoordScaleS;
            v->st[1] = pInfo->fTCenter + DotProduct(delta, pInfo->axis[2]) * pInfo->texCoordScaleT;
            v->modulate[0] = pInfo->colors[0];
            v->modulate[1] = pInfo->colors[1];
            v->modulate[2] = pInfo->colors[2];
            v->modulate[3] = pInfo->colors[3];
        }
    }
    else
    {
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
            v->st[0] = pInfo->fSCenter + DotProduct(delta, pInfo->axis[1]) * pInfo->texCoordScaleS;
            v->st[1] = pInfo->fTCenter + DotProduct(delta, pInfo->axis[2]) * pInfo->texCoordScaleT;
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
    int i;
    int numFragments;
    float fSScale2, fTScale2;
    float fSScale3, fTScale3;
    vec3_t originalPoints[4];
    vec3_t markPoints[MAX_MARK_POINTS];
    vec3_t projection;
    markFragment_t markFragments[MAX_MARK_FRAGMENTS], *mf;
    vec3_t v;
    float fRadiusSquared;
    cg_impactmarkinfo_t info;

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

    fRadiusSquared = fSScale * fSScale + fTScale * fTScale;
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

    numFragments = CG_GetMarkFragments(
        4,
        originalPoints,
        projection,
        markPoints,
        markFragments,
        fRadiusSquared
    );

    if (dolighting)
    {
        vec3_t vLight;
        cgi.R_GetLightingForDecal(vLight, dir, origin);

        info.colors[0] = (int)(red * vLight[0]);
        info.colors[1] = (int)(green * vLight[1]);
        info.colors[2] = (int)(blue * vLight[2]);
    }
    else
    {
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

    if (temporary)
    {
        polyVert_t verts[8];

        for (i = 0, mf = markFragments; i < numFragments; i++, mf++) {
            if (mf->numPoints > 8) {
                mf->numPoints = 8;
            }

            if (CG_ImpactMark_PerPolyCallback(markPoints, mf, verts, (void*)&info)) {
                CG_AddFragmentToScene(mf->iIndex, markShader, mf->numPoints, verts);
            }
        }
    }
    else
    {
        CG_AssembleFinalMarks(
            markPoints,
            markFragments,
            numFragments,
            &CG_ImpactMark_PerPolyCallback,
            &CG_ImpactMark_GetLeafCallback,
            (void*)&info,
            info.origin,
            sqrt(fRadiusSquared),
            markShader,
            fadein,
            alphaFade
        );
    }
}

void CG_AssembleFinalMarks(
    vec3_t           *markPoints,
    markFragment_t   *markFragments,
    int              numFragments,
    qboolean         (*PerPolyCallback)(const vec3_t* markPoints, markFragment_t* mf, polyVert_t* verts, void* pCustom),
    int              (*GetLeafCallback)(markFragment_t* mf, void* pCustom),
    void             *pCustom,
    vec3_t           pos,
    float            radius,
    qhandle_t        markShader,
    qboolean         fadein,
    qboolean         alphaFade
)
{
    markObj_t* pMark;
    markPoly_t* pPoly;
    int iGroup, iFirstNewGroup;
    int numFragsForMark;
    int i;
    markFragment_t* mf;

    iFirstNewGroup = 0;
    do
    {
        i = iFirstNewGroup;
        mf = &markFragments[iFirstNewGroup];
        iGroup = mf->iIndex;
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

        pMark->time = cg.time;
        pMark->alphaFade = alphaFade;
        pMark->markShader = markShader;
        pMark->fadein = fadein;
        VectorCopy(pos, pMark->pos);
        pMark->radius = radius;

        i = iFirstNewGroup;
        mf = &markFragments[iFirstNewGroup];
        iFirstNewGroup = 0;

        if (mf->iIndex < 0) {
            VectorSubtract(pMark->pos, cg_entities[-mf->iIndex].lerpOrigin, pMark->pos);
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
                    pPoly->iIndex = mf->iIndex;
                    pPoly = pPoly->nextPoly;
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

    viewleafnum = cgi.CM_PointLeafnum(cg.refdef.vieworg);
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
            if (CG_FrustumCullSphere(pMark->pos, pMark->radius)) {
                continue;
            }
        } else {
            vec3_t vWorldPos;
            CG_FragmentPosToWorldPos(pMark->pos, vWorldPos);
            if (CG_FrustumCullSphere(vWorldPos, pMark->radius)) {
                continue;
            }
        }

        pMark->lastVisTime = cg.time;
        if (pMark->fadein) {
            fade = 255 * (cg.time - pMark->time) / MARK_FADE_TIME;
            if (fade > 255) {
                fade = 255;
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

        for (pPoly = pMark->markPolys; pPoly; pPoly = pPoly->nextPoly)
        {
            if (pPoly->iIndex < 0)
            {
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

    if (cg_entities[iEntIndex].currentState.modelindex < 0 || cg_entities[iEntIndex].currentState.modelindex > cgi.CM_NumInlineModels()) {
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
    int i;
    int iTreadNum;
    treadMark_t* pTread;

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
            pTread = &cg_treadMarks[i];
            break;
        }
    }

    if (iTreadNum == -1) {
        return -1;
    }

    memset(pTread, 0, sizeof(*pTread));
    pTread->iState = 1;
    pTread->iReferenceNumber = iReference;
    pTread->iLastTime = cg.time;
    pTread->hTreadShader = treadShader;
    pTread->fWidth = fWidth * 0.5;
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

qboolean CG_MakeTreadMarkDecal_PerPolyCallback(const vec3_t *markPoints, markFragment_t *mf, polyVert_t *verts, void *pCustom)
{
    // FIXME: unimplemented
    return qfalse;
}

int CG_MakeTreadMarkDecal_GetLeafCallback(markFragment_t *mf, void *pCustom)
{
    return ((cg_treadmarkinfo_t*)pCustom)->leafnum;
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
    trace_t trace;
    vec3_t vPos, vEnd;
    int i;

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

        if (trace.fraction < 1.0 && CG_UpdateTreadMark(1, trace.endpos, 0.f) == -1) {
            qhandle_t shader = cgi.R_RegisterShader("testtread");

            CG_StartTreadMark(1, shader, trace.endpos, cg_treadmark_test->integer, 1.0f);
        }
    }

    for (i = 0; i < MAX_TREAD_MARKS; i++) {
        if (!cg_treadMarks[i].iState) {
            continue;
        }

        if (cg.time - cg_treadMarks[i].iLastTime > 500) {
            CG_MakeTreadMarkDecal(
                &cg_treadMarks[i],
                cg_treadMarks[i].iState == 3 ? qtrue : qfalse,
                qfalse
            );
            cg_treadMarks[i].iState = 0;
        }
        else {
            CG_MakeTreadMarkDecal(
                &cg_treadMarks[i],
                cg_treadMarks[i].iState == 3 ? qtrue : qfalse,
                qtrue
            );
        }
    }
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
