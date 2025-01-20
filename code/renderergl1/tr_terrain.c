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

// tr_terrain.c : Terrain rendering

#include "tr_local.h"

cvar_t *ter_maxlod;
cvar_t *ter_maxtris;
cvar_t *ter_cull;
cvar_t *ter_lock;
cvar_t *ter_error;
cvar_t *ter_cautiousframes;
cvar_t *ter_count;

#define TERRAIN_TABLE_SIZE 180

static float g_fDistanceTable[TERRAIN_TABLE_SIZE];
static float g_fClipDotProductTable[TERRAIN_TABLE_SIZE];
static float g_fClipDotSquaredTable[TERRAIN_TABLE_SIZE];
static float g_fFogDistance;
static float g_fClipDotProduct;
static float g_fClipDotSquared;

static vec3_t g_vClipOrigin;
static vec3_t g_vClipVector;
static vec3_t g_vViewOrigin;
static vec3_t g_vViewVector;

static int    g_terVisCount;
static float  g_fCheck;
static size_t g_nTris;
static size_t g_nVerts;
static int    g_nMerge;
static int    g_nSplit;
unsigned int  g_uiTerDist;
vec2_t        g_vTerOrg;
vec3_t        g_vTerFwd;

static const unsigned int MAX_TERRAIN_LOD       = 6;
static const float        TERRAIN_LIGHTMAP_SIZE = 128.0f;

typedef struct poolInfo_s {
    terraInt iFreeHead;
    terraInt iCur;
    size_t   nFree;
} poolInfo_t;

static int modeTable[8];

terraTri_t    *g_pTris;
terrainVert_t *g_pVert;

poolInfo_t g_tri;
poolInfo_t g_vert;

/*
================
R_ValidateHeightmapForVertex
================
*/
static void R_ValidateHeightmapForVertex(terraTri_t *pTri)
{
    for (terraInt i = 0; i < 3; i++) {
        terrainVert_t *pVert = &g_pVert[pTri->iPt[i]];
        if (pVert->pHgt < pTri->patch->heightmap || pVert->pHgt > &pTri->patch->heightmap[80]) {
            pVert->pHgt = pTri->patch->heightmap;
        }
    }
}

/*
================
R_AllocateVert
================
*/
static terraInt R_AllocateVert(cTerraPatchUnpacked_t *patch)
{
    terraInt iVert = g_vert.iFreeHead;

    g_vert.iFreeHead                = g_pVert[g_vert.iFreeHead].iNext;
    g_pVert[g_vert.iFreeHead].iPrev = 0;

    g_pVert[iVert].iPrev                     = 0;
    g_pVert[iVert].iNext                     = patch->drawinfo.iVertHead;
    g_pVert[patch->drawinfo.iVertHead].iPrev = iVert;

    patch->drawinfo.iVertHead = iVert;
    patch->drawinfo.nVerts++;

    assert(g_vert.nFree > 0);
    g_vert.nFree--;

    g_pVert[iVert].nRef         = 0;
    g_pVert[iVert].uiDistRecalc = 0;

    return iVert;
}

/*
================
R_InterpolateVert
================
*/
static void R_InterpolateVert(terraTri_t *pTri, terrainVert_t *pVert)
{
    const terrainVert_t         *pVert0 = &g_pVert[pTri->iPt[0]];
    const terrainVert_t         *pVert1 = &g_pVert[pTri->iPt[1]];
    const cTerraPatchUnpacked_t *pPatch = pTri->patch;
    const byte                  *pMinHeight, *pMaxHeight;

    // Interpolate texture coordinates
    pVert->texCoords[0][0] = (pVert0->texCoords[0][0] + pVert1->texCoords[0][0]) * 0.5f;
    pVert->texCoords[0][1] = (pVert0->texCoords[0][1] + pVert1->texCoords[0][1]) * 0.5f;
    pVert->texCoords[1][0] = (pVert0->texCoords[1][0] + pVert1->texCoords[1][0]) * 0.5f;
    pVert->texCoords[1][1] = (pVert0->texCoords[1][1] + pVert1->texCoords[1][1]) * 0.5f;

    // Fixed in OPM
    //  Use the delta of the two pointers instead of adding them + divide to get the average height,
    //  otherwise the resulting integer would overflow/warp and cause a segmentation fault.
    //  Although rare, the overflow issue can occur in the original game
    pMinHeight  = (byte *)Q_min((uintptr_t)pVert0->pHgt, (uintptr_t)pVert1->pHgt);
    pMaxHeight  = (byte *)Q_max((uintptr_t)pVert0->pHgt, (uintptr_t)pVert1->pHgt);
    pVert->pHgt = (byte *)(pMinHeight + ((pMaxHeight - pMinHeight) >> 1));
    assert(pVert->pHgt >= pMinHeight && pVert->pHgt < pMaxHeight);

    // Calculate the average Z
    pVert->fHgtAvg = (float)(*pVert0->pHgt + *pVert1->pHgt);
    pVert->fHgtAdd = (float)(*pVert->pHgt * 2) - pVert->fHgtAvg;
    pVert->fHgtAvg += pPatch->z0;
    // Calculate the average position
    pVert->xyz[0] = (pVert0->xyz[0] + pVert1->xyz[0]) * 0.5f;
    pVert->xyz[1] = (pVert0->xyz[1] + pVert1->xyz[1]) * 0.5f;
    pVert->xyz[2] = pVert->fHgtAvg;
}

/*
================
R_ReleaseVert
================
*/
static void R_ReleaseVert(cTerraPatchUnpacked_t *patch, int iVert)
{
    terrainVert_t *pVert = &g_pVert[iVert];

    terraInt iPrev       = pVert->iPrev;
    terraInt iNext       = pVert->iNext;
    g_pVert[iPrev].iNext = iNext;
    g_pVert[iNext].iPrev = iPrev;

    assert(patch->drawinfo.nVerts > 0);
    patch->drawinfo.nVerts--;

    if (patch->drawinfo.iVertHead == iVert) {
        patch->drawinfo.iVertHead = iNext;
    }

    pVert->iPrev                    = 0;
    pVert->iNext                    = g_vert.iFreeHead;
    g_pVert[g_vert.iFreeHead].iPrev = iVert;
    g_vert.iFreeHead                = iVert;
    g_vert.nFree++;
}

/*
================
R_AllocateTri
================
*/
terraInt R_AllocateTri(cTerraPatchUnpacked_t *patch, qboolean check)
{
    terraInt iTri = g_tri.iFreeHead;

    g_tri.iFreeHead                = g_pTris[iTri].iNext;
    g_pTris[g_tri.iFreeHead].iPrev = 0;

    g_pTris[iTri].iPrev                     = patch->drawinfo.iTriTail;
    g_pTris[iTri].iNext                     = 0;
    g_pTris[patch->drawinfo.iTriTail].iNext = iTri;
    patch->drawinfo.iTriTail                = iTri;

    if (!patch->drawinfo.iTriHead) {
        patch->drawinfo.iTriHead = iTri;
    }

    patch->drawinfo.nTris++;
    assert(g_tri.nFree > 0);
    g_tri.nFree--;

    g_pTris[iTri].byConstChecks = check ? 0 : 4;
    g_pTris[iTri].uiDistRecalc  = 0;
    patch->uiDistRecalc         = 0;

    return iTri;
}

/*
================
R_ReleaseTri
================
*/
static void R_ReleaseTri(cTerraPatchUnpacked_t *patch, terraInt iTri)
{
    terraTri_t *pTri = &g_pTris[iTri];

    terraInt iPrev       = pTri->iPrev;
    terraInt iNext       = pTri->iNext;
    g_pTris[iPrev].iNext = iNext;
    g_pTris[iNext].iPrev = iPrev;

    if (g_tri.iCur == iTri) {
        g_tri.iCur = iNext;
    }

    patch->uiDistRecalc = 0;

    assert(patch->drawinfo.nTris > 0);
    patch->drawinfo.nTris--;

    if (patch->drawinfo.iTriHead == iTri) {
        patch->drawinfo.iTriHead = iNext;
    }

    if (patch->drawinfo.iTriTail == iTri) {
        patch->drawinfo.iTriTail = iPrev;
    }

    pTri->iPrev                    = 0;
    pTri->iNext                    = g_tri.iFreeHead;
    g_pTris[g_tri.iFreeHead].iPrev = iTri;
    g_tri.iFreeHead                = iTri;
    g_tri.nFree++;

    for (int i = 0; i < 3; i++) {
        terraInt ptNum = pTri->iPt[i];

        g_pVert[ptNum].nRef--;
        if (!g_pVert[ptNum].nRef) {
            R_ReleaseVert(patch, ptNum);
        }
    }
}

/*
================
R_ConstChecksForTri
================
*/
static int R_ConstChecksForTri(terraTri_t *pTri)
{
    varnodeUnpacked_t vn;

    if (pTri->lod == MAX_TERRAIN_LOD) {
        return 2;
    }

    vn = *pTri->varnode;
    //vn.s.flags &= 0xF0;
    vn.flags &= 0xFFFFFFF0;

    if (vn.fVariance == 0.0 && !(pTri->varnode->flags & 8)) {
        return 2;
    } else if (pTri->varnode->flags & 8) {
        return 3;
    } else if ((pTri->byConstChecks & 4) && !(pTri->varnode->flags & 4) && pTri->lod < ter_maxlod->integer) {
        return 0;
    }

    return 2;
}

/*
================
R_DemoteInAncestry
================
*/
static void R_DemoteInAncestry(cTerraPatchUnpacked_t *patch, terraInt iTri)
{
    terraInt iPrev = g_pTris[iTri].iPrev;
    terraInt iNext = g_pTris[iTri].iNext;

    g_pTris[iPrev].iNext = iNext;
    g_pTris[iNext].iPrev = iPrev;

    if (g_tri.iCur == iTri) {
        g_tri.iCur = iNext;
    }

    assert(patch->drawinfo.nTris > 0);
    patch->drawinfo.nTris--;

    if (patch->drawinfo.iTriHead == iTri) {
        patch->drawinfo.iTriHead = iNext;
    }

    if (patch->drawinfo.iTriTail == iTri) {
        patch->drawinfo.iTriTail = iPrev;
    }

    g_pTris[iTri].iPrev                       = 0;
    g_pTris[iTri].iNext                       = patch->drawinfo.iMergeHead;
    g_pTris[patch->drawinfo.iMergeHead].iPrev = iTri;
    patch->drawinfo.iMergeHead                = iTri;
}

/*
================
R_TerrainHeapInit
================
*/
static void R_TerrainHeapInit()
{
    g_tri.iFreeHead  = 1;
    g_tri.nFree      = g_nTris - 1;
    g_vert.iFreeHead = 1;
    g_vert.nFree     = g_nVerts - 1;

    for (size_t i = 0; i < g_nTris; i++) {
        g_pTris[i].iPrev = (terraInt)i - 1;
        g_pTris[i].iNext = (terraInt)i + 1;
    }

    g_pTris[0].iPrev           = 0;
    g_pTris[g_nTris - 1].iNext = 0;

    for (size_t i = 0; i < g_nVerts; i++) {
        g_pVert[i].iPrev = (terraInt)i - 1;
        g_pVert[i].iNext = (terraInt)i + 1;
    }

    g_pVert[0].iPrev            = 0;
    g_pVert[g_nVerts - 1].iNext = 0;
}

/*
================
R_TerrainPatchesInit
================
*/
static void R_TerrainPatchesInit()
{
    int i;

    for (i = 0; i < tr.world->numTerraPatches; i++) {
        cTerraPatchUnpacked_t *patch = &tr.world->terraPatches[i];
        patch->drawinfo.iTriHead     = 0;
        patch->drawinfo.iTriTail     = 0;
        patch->drawinfo.iMergeHead   = 0;
        patch->drawinfo.iVertHead    = 0;
        patch->drawinfo.nTris        = 0;
        patch->drawinfo.nVerts       = 0;
    }
}

/*
================
R_SplitTri
================
*/

void R_SplitTri(
    terraInt iSplit, terraInt iNewPt, terraInt iLeft, terraInt iRight, terraInt iRightOfLeft, terraInt iLeftOfRight
)
{
    terraTri_t *pSplit = &g_pTris[iSplit];

    terraTri_t *pLeft;
    if (iLeft) {
        pLeft = &g_pTris[iLeft];
    } else {
        pLeft = NULL;
    }

    terraTri_t *pRight;
    if (iRight) {
        pRight = &g_pTris[iRight];
    } else {
        pRight = NULL;
    }

    int                iNextLod = pSplit->lod + 1;
    int                index    = pSplit->index;
    varnodeUnpacked_t *varnode  = &pSplit->varnode[index];

    if (pLeft) {
        pLeft->patch   = pSplit->patch;
        pLeft->index   = index * 2;
        pLeft->varnode = varnode;
        pLeft->lod     = iNextLod;
        pLeft->iLeft   = iRight;
        pLeft->iRight  = iRightOfLeft;
        pLeft->iBase   = pSplit->iLeft;
        pLeft->iPt[0]  = pSplit->iPt[1];
        pLeft->iPt[1]  = pSplit->iPt[2];
        pLeft->iPt[2]  = iNewPt;

        R_ValidateHeightmapForVertex(pLeft);
        pLeft->byConstChecks |= R_ConstChecksForTri(pLeft);

        g_pVert[pLeft->iPt[0]].nRef++;
        g_pVert[pLeft->iPt[1]].nRef++;
        g_pVert[pLeft->iPt[2]].nRef++;
        g_pTris[pSplit->iParent].nSplit++;
        pLeft->nSplit = 0;
    }

    if (pSplit->iLeft) {
        if (g_pTris[pSplit->iLeft].lod == iNextLod) {
            g_pTris[pSplit->iLeft].iBase = iLeft;
        } else {
            g_pTris[pSplit->iLeft].iRight = iLeft;
        }
    }

    if (pRight) {
        pRight->patch   = pSplit->patch;
        pRight->index   = index * 2 + 1;
        pRight->varnode = varnode + 1;
        pRight->lod     = iNextLod;
        pRight->iLeft   = iLeftOfRight;
        pRight->iRight  = iLeft;
        pRight->iBase   = pSplit->iRight;
        pRight->iPt[0]  = pSplit->iPt[2];
        pRight->iPt[1]  = pSplit->iPt[0];
        pRight->iPt[2]  = iNewPt;

        R_ValidateHeightmapForVertex(pRight);
        pRight->byConstChecks |= R_ConstChecksForTri(pRight);

        g_pVert[pRight->iPt[0]].nRef++;
        g_pVert[pRight->iPt[1]].nRef++;
        g_pVert[pRight->iPt[2]].nRef++;
        g_pTris[pSplit->iParent].nSplit++;
        pRight->nSplit = 0;
    }

    if (pSplit->iRight) {
        if (g_pTris[pSplit->iRight].lod == iNextLod) {
            g_pTris[pSplit->iRight].iBase = iRight;
        } else {
            g_pTris[pSplit->iRight].iLeft = iRight;
        }
    }
}

/*
================
R_ForceSplit
================
*/
static void R_ForceSplit(terraInt iTri)
{
    terraTri_t    *pTri = &g_pTris[iTri];
    terraTri_t    *pBase;
    terrainVert_t *pVert;
    terraInt       iBase;
    terraInt       iTriLeft, iTriRight;
    terraInt       iNewPt;
    terraInt       iBaseLeft = 0, iBaseRight = 0;
    terraInt       iNewBasePt;
    uint32_t       flags, flags2;

    g_nSplit++;

    iBase = pTri->iBase;
    pBase = &g_pTris[iBase];
    if (iBase && pBase->lod != pTri->lod) {
        assert(g_pTris[pTri->iBase].iBase != iTri);
        assert(g_tri.nFree >= 8);

        R_ForceSplit(iBase);

        assert(g_tri.nFree >= 4);

        iBase = pTri->iBase;
        pBase = &g_pTris[iBase];
    }

    flags = pTri->varnode->flags;

    iTriLeft  = R_AllocateTri(pTri->patch, (flags & 2));
    iTriRight = R_AllocateTri(pTri->patch, (flags & 1));

    iNewPt = R_AllocateVert(pTri->patch);
    R_InterpolateVert(pTri, &g_pVert[iNewPt]);

    g_pVert[iNewPt].fVariance = pTri->varnode->fVariance;

    iBaseLeft  = 0;
    iBaseRight = 0;

    if (iBase) {
        flags2 = pBase->varnode->flags;
        flags |= flags2;

        iBaseLeft  = R_AllocateTri(pBase->patch, (flags2 & 2));
        iBaseRight = R_AllocateTri(pBase->patch, (flags2 & 1));

        iNewBasePt = iNewPt;
        if (pBase->patch != pTri->patch) {
            iNewBasePt = R_AllocateVert(pBase->patch);
            pVert      = &g_pVert[iNewBasePt];
            R_InterpolateVert(pBase, pVert);

            pVert->fVariance = g_pVert[iNewPt].fVariance;
            if (flags & 8) {
                pVert->fHgtAvg += pVert->fHgtAdd;
                pVert->fHgtAdd   = 0.0;
                pVert->fVariance = 0.0;
                pVert->xyz[2]    = pVert->fHgtAvg;
            }
        }

        R_SplitTri(iBase, iNewBasePt, iBaseLeft, iBaseRight, iTriRight, iTriLeft);

        pBase->iLeftChild           = iBaseLeft;
        pBase->iRightChild          = iBaseRight;
        g_pTris[iBaseLeft].iParent  = iBase;
        g_pTris[iBaseRight].iParent = iBase;
        R_DemoteInAncestry(pBase->patch, iBase);
    }

    if (flags & 8) {
        pVert = &g_pVert[iNewPt];
        pVert->fHgtAvg += pVert->fHgtAdd;
        pVert->fHgtAdd   = 0.0;
        pVert->fVariance = 0.0;
        pVert->xyz[2]    = pVert->fHgtAvg;
    }

    R_SplitTri(iTri, iNewPt, iTriLeft, iTriRight, iBaseRight, iBaseLeft);

    pTri->iLeftChild           = iTriLeft;
    pTri->iRightChild          = iTriRight;
    g_pTris[iTriLeft].iParent  = iTri;
    g_pTris[iTriRight].iParent = iTri;
    R_DemoteInAncestry(pTri->patch, iTri);
}

/*
================
R_ForceMerge
================
*/
static void R_ForceMerge(terraInt iTri)
{
    terraTri_t            *pTri  = &g_pTris[iTri];
    cTerraPatchUnpacked_t *patch = pTri->patch;
    terraInt               iPrev = pTri->iPrev;
    terraInt               iNext = pTri->iNext;

    g_nMerge++;

    if (pTri->iLeftChild) {
        terraInt iLeft = g_pTris[pTri->iLeftChild].iBase;

        pTri->iLeft = iLeft;
        if (iLeft) {
            if (g_pTris[iLeft].lod == pTri->lod) {
                g_pTris[iLeft].iRight = iTri;
            } else {
                g_pTris[iLeft].iBase = iTri;
            }
        }

        R_ReleaseTri(pTri->patch, pTri->iLeftChild);

        pTri->iLeftChild = 0;
        g_pTris[pTri->iParent].nSplit--;
    }

    if (pTri->iRightChild) {
        terraInt iRight = g_pTris[pTri->iRightChild].iBase;

        pTri->iRight = iRight;
        if (iRight) {
            if (g_pTris[iRight].lod == pTri->lod) {
                g_pTris[iRight].iLeft = iTri;
            } else {
                g_pTris[iRight].iBase = iTri;
            }
        }

        R_ReleaseTri(pTri->patch, pTri->iRightChild);

        pTri->iLeftChild = 0;
        g_pTris[pTri->iParent].nSplit--;
    }

    g_pTris[iPrev].iNext = iNext;
    g_pTris[iNext].iPrev = iPrev;

    if (g_tri.iCur == iTri) {
        g_tri.iCur = iNext;
    }

    patch->drawinfo.nTris++;
    if (patch->drawinfo.iMergeHead == iTri) {
        patch->drawinfo.iMergeHead = iNext;
    }

    g_pTris[iTri].iPrev = patch->drawinfo.iTriTail;
    g_pTris[iTri].iNext = 0;

    g_pTris[patch->drawinfo.iTriTail].iNext = iTri;

    patch->drawinfo.iTriTail = iTri;
    if (!patch->drawinfo.iTriHead) {
        patch->drawinfo.iTriHead = iTri;
    }
}

/*
================
R_TerraTriNeighbor
================
*/
static int R_TerraTriNeighbor(cTerraPatchUnpacked_t *terraPatches, int iPatch, int dir)
{
    int iNeighbor;

    if (iPatch < 0) {
        return 0;
    }

    iNeighbor = 2 * iPatch + 1;

    switch (dir) {
    case 0:
        return iNeighbor;
    case 1:
        if (terraPatches[iPatch].flags & 0x80) {
            return iNeighbor;
        } else {
            return iNeighbor + 1;
        }
        break;
    case 2:
        return iNeighbor + 1;
    case 3:
        if (terraPatches[iPatch].flags & 0x80) {
            return iNeighbor + 1;
        } else {
            return iNeighbor;
        }
        break;
    }

    return 0;
}

/*
================
R_PreTessellateTerrain
================
*/
static void R_PreTessellateTerrain()
{
    size_t numTerrainPatches = tr.world->numTerraPatches;

    if (!numTerrainPatches) {
        return;
    }

    R_IssuePendingRenderCommands();

    if (ter_maxtris->integer < 4 * numTerrainPatches) {
        ri.Cvar_SetValue("ter_maxtris", 4 * numTerrainPatches);
    } else if (ter_maxtris->integer > 65535) {
        ri.Cvar_SetValue("ter_maxtris", 65535);
    }

    ri.Printf(PRINT_DEVELOPER, "Using ter_maxtris = %d\n", ter_maxtris->integer);

    g_nTris  = ter_maxtris->integer * 2 + 1;
    g_nVerts = ter_maxtris->integer + 1;
    g_pTris  = ri.Hunk_Alloc(g_nTris * sizeof(terraTri_t), h_dontcare);
    g_pVert  = ri.Hunk_Alloc(g_nVerts * sizeof(terrainVert_t), h_dontcare);

    // Init triangles & vertices
    R_TerrainHeapInit();
    R_TerrainPatchesInit();

    for (size_t i = 0; i < numTerrainPatches; i++) {
        cTerraPatchUnpacked_t *patch = &tr.world->terraPatches[i];

        patch->drawinfo.surfaceType = SF_TERRAIN_PATCH;
        patch->drawinfo.nTris       = 0;
        patch->drawinfo.nVerts      = 0;
        patch->drawinfo.iTriHead    = 0;
        patch->drawinfo.iTriTail    = 0;
        patch->drawinfo.iVertHead   = 0;

        float sMin, tMin;

        if (patch->texCoord[0][0][0] < patch->texCoord[0][1][0]) {
            sMin = patch->texCoord[0][0][0];
        } else {
            sMin = patch->texCoord[0][1][0];
        }

        float sMin2;
        if (patch->texCoord[1][0][0] < patch->texCoord[1][1][0]) {
            sMin2 = patch->texCoord[1][0][0];
        } else {
            sMin2 = patch->texCoord[1][1][0];
        }

        if (sMin >= sMin2) {
            if (patch->texCoord[1][0][0] >= patch->texCoord[1][1][0]) {
                sMin = floor(patch->texCoord[1][1][0]);
            } else {
                sMin = floor(patch->texCoord[1][0][0]);
            }
        } else if (patch->texCoord[0][0][0] >= patch->texCoord[0][1][0]) {
            sMin = floor(patch->texCoord[0][1][0]);
        } else {
            sMin = floor(patch->texCoord[0][0][0]);
        }

        if (patch->texCoord[0][0][1] < patch->texCoord[0][1][1]) {
            tMin = patch->texCoord[0][0][1];
        } else {
            tMin = patch->texCoord[0][1][1];
        }

        float tMin2;
        if (patch->texCoord[1][0][1] < patch->texCoord[1][1][1]) {
            tMin2 = patch->texCoord[1][0][1];
        } else {
            tMin2 = patch->texCoord[1][1][1];
        }

        if (tMin >= tMin2) {
            if (patch->texCoord[1][0][1] >= patch->texCoord[1][1][1]) {
                tMin = floor(patch->texCoord[1][1][1]);
            } else {
                tMin = floor(patch->texCoord[1][0][1]);
            }
        } else if (patch->texCoord[0][0][1] >= patch->texCoord[0][1][1]) {
            tMin = floor(patch->texCoord[0][1][1]);
        } else {
            tMin = floor(patch->texCoord[0][0][1]);
        }

        const float s00 = patch->texCoord[0][0][0] - sMin;
        const float s01 = patch->texCoord[0][1][0] - sMin;
        const float s10 = patch->texCoord[1][0][0] - sMin;
        const float s11 = patch->texCoord[1][1][0] - sMin;

        const float t00 = patch->texCoord[0][0][1] - tMin;
        const float t01 = patch->texCoord[0][1][1] - tMin;
        const float t10 = patch->texCoord[1][0][1] - tMin;
        const float t11 = patch->texCoord[1][1][1] - tMin;

        const float lmapSize = (float)(patch->drawinfo.lmapSize - 1) / TERRAIN_LIGHTMAP_SIZE;
        const float ls       = patch->s + lmapSize;
        const float lt       = patch->t + lmapSize;

        terraInt iTri0 = R_AllocateTri(patch, qfalse);
        terraInt iTri1 = R_AllocateTri(patch, qfalse);
        terraInt i00   = R_AllocateVert(patch);
        terraInt i01   = R_AllocateVert(patch);
        terraInt i10   = R_AllocateVert(patch);
        terraInt i11   = R_AllocateVert(patch);

        terrainVert_t *pVert;
        pVert                  = &g_pVert[i00];
        pVert->xyz[0]          = patch->x0;
        pVert->xyz[1]          = patch->y0;
        pVert->xyz[2]          = (float)(patch->heightmap[0] * 2) + patch->z0;
        pVert->pHgt            = &patch->heightmap[0];
        pVert->fHgtAvg         = pVert->xyz[2];
        pVert->texCoords[0][0] = s00;
        pVert->texCoords[0][1] = t00;
        pVert->texCoords[1][0] = patch->s;
        pVert->texCoords[1][1] = patch->t;
        pVert->fVariance       = 0.0f;
        pVert->nRef            = 4;

        pVert                  = &g_pVert[i01];
        pVert->xyz[0]          = patch->x0;
        pVert->xyz[1]          = patch->y0 + 512.0f;
        pVert->xyz[2]          = (float)(patch->heightmap[72] * 2) + patch->z0;
        pVert->pHgt            = &patch->heightmap[72];
        pVert->fHgtAvg         = pVert->xyz[2];
        pVert->texCoords[0][0] = s01;
        pVert->texCoords[0][1] = t01;
        pVert->texCoords[1][0] = patch->s;
        pVert->texCoords[1][1] = lt;
        pVert->fVariance       = 0.0f;
        pVert->nRef            = 4;

        pVert                  = &g_pVert[i10];
        pVert->xyz[0]          = patch->x0 + 512.0f;
        pVert->xyz[1]          = patch->y0;
        pVert->xyz[2]          = (float)(patch->heightmap[8] * 2) + patch->z0;
        pVert->pHgt            = &patch->heightmap[8];
        pVert->fHgtAvg         = pVert->xyz[2];
        pVert->texCoords[0][0] = s10;
        pVert->texCoords[0][1] = t10;
        pVert->texCoords[1][0] = ls;
        pVert->texCoords[1][1] = patch->t;
        pVert->fVariance       = 0.0f;
        pVert->nRef            = 4;

        pVert                  = &g_pVert[i11];
        pVert->xyz[0]          = patch->x0 + 512.0f;
        pVert->xyz[1]          = patch->y0 + 512.0f;
        pVert->xyz[2]          = (float)(patch->heightmap[80] * 2) + patch->z0;
        pVert->pHgt            = &patch->heightmap[80];
        pVert->fHgtAvg         = pVert->xyz[2];
        pVert->texCoords[0][0] = s11;
        pVert->texCoords[0][1] = t11;
        pVert->texCoords[1][0] = ls;
        pVert->texCoords[1][1] = lt;
        pVert->fVariance       = 0.0f;
        pVert->nRef            = 4;

        terraTri_t *pTri = &g_pTris[iTri0];
        pTri->patch      = patch;
        pTri->varnode    = &patch->varTree[0][0];
        pTri->index      = 1;
        pTri->lod        = 0;
        pTri->byConstChecks |= R_ConstChecksForTri(pTri);

        pTri->iBase = iTri1;
        if ((patch->flags & 0x80u) == 0) {
            pTri->iLeft  = R_TerraTriNeighbor(tr.world->terraPatches, patch->iWest, 1);
            pTri->iRight = R_TerraTriNeighbor(tr.world->terraPatches, patch->iNorth, 2);
            if (patch->flags & 0x40) {
                pTri->iPt[0] = i00;
                pTri->iPt[1] = i11;
            } else {
                pTri->iPt[0] = i11;
                pTri->iPt[1] = i00;
            }
            pTri->iPt[2] = i01;
        } else {
            pTri->iLeft  = R_TerraTriNeighbor(tr.world->terraPatches, patch->iNorth, 2);
            pTri->iRight = R_TerraTriNeighbor(tr.world->terraPatches, patch->iEast, 3);
            if (patch->flags & 0x40) {
                pTri->iPt[0] = i01;
                pTri->iPt[1] = i10;
            } else {
                pTri->iPt[0] = i10;
                pTri->iPt[1] = i01;
            }
            pTri->iPt[2] = i11;
        }

        R_ValidateHeightmapForVertex(pTri);

        pTri          = &g_pTris[iTri1];
        pTri->patch   = patch;
        pTri->varnode = &patch->varTree[1][0];
        pTri->index   = 1;
        pTri->lod     = 0;
        pTri->byConstChecks |= R_ConstChecksForTri(pTri);

        pTri->iBase = iTri0;
        if ((patch->flags & 0x80u) == 0) {
            pTri->iLeft  = R_TerraTriNeighbor(tr.world->terraPatches, patch->iEast, 3);
            pTri->iRight = R_TerraTriNeighbor(tr.world->terraPatches, patch->iSouth, 0);
            if (patch->flags & 0x40) {
                pTri->iPt[0] = i11;
                pTri->iPt[1] = i00;
            } else {
                pTri->iPt[0] = i00;
                pTri->iPt[1] = i11;
            }
            pTri->iPt[2] = i10;
        } else {
            pTri->iLeft  = R_TerraTriNeighbor(tr.world->terraPatches, patch->iSouth, 0);
            pTri->iRight = R_TerraTriNeighbor(tr.world->terraPatches, patch->iWest, 1);
            if (patch->flags & 0x40) {
                pTri->iPt[0] = i10;
                pTri->iPt[1] = i01;
            } else {
                pTri->iPt[0] = i01;
                pTri->iPt[1] = i10;
            }
            pTri->iPt[2] = i00;
        }

        R_ValidateHeightmapForVertex(pTri);
    }
}

/*
================
R_NeedSplitTri
================
*/
static qboolean R_NeedSplitTri(terraTri_t *pTri)
{
    uint8_t byConstChecks = pTri->byConstChecks;
    if (byConstChecks & 2) {
        return byConstChecks & 1;
    }

    if (pTri->uiDistRecalc > g_uiTerDist) {
        return qfalse;
    }

    float fRatio = ((g_pVert[pTri->iPt[0]].xyz[0] + g_pVert[pTri->iPt[1]].xyz[0]) * g_vViewVector[0]
                    + (g_pVert[pTri->iPt[0]].xyz[1] + g_pVert[pTri->iPt[1]].xyz[1]) * g_vViewVector[1])
                     * 0.5
                 + g_vViewVector[2] - pTri->varnode->fVariance * g_fCheck;

    if (fRatio > 0) {
        pTri->uiDistRecalc = floor(fRatio) + g_uiTerDist;
        return qfalse;
    }

    return qtrue;
}

/*
================
R_CalcVertMorphHeight
================
*/
static void R_CalcVertMorphHeight(terrainVert_t *pVert)
{
    float dot;

    pVert->xyz[2]       = pVert->fHgtAvg;
    pVert->uiDistRecalc = g_uiTerDist + 1;
    dot                 = (pVert->fVariance * g_fCheck)
        - (g_vViewVector[0] * pVert->xyz[0] + g_vViewVector[1] * pVert->xyz[1] + g_vViewVector[2]);

    if (dot > 0.0) {
        float calc = dot / 400.0;

        if (calc > 1.0) {
            pVert->uiDistRecalc = (unsigned int)((g_uiTerDist - 400) + floor(dot));
            calc                = 1.0;
        }

        pVert->xyz[2] += pVert->fHgtAdd * calc;
    } else {
        pVert->uiDistRecalc = g_uiTerDist - (int)ceil(dot);
    }
}

/*
================
R_UpdateVertMorphHeight
================
*/
static void R_UpdateVertMorphHeight(terrainVert_t *pVert)
{
    if (pVert->uiDistRecalc <= g_uiTerDist) {
        R_CalcVertMorphHeight(pVert);
    }
}

/*
================
R_DoTriSplitting
================
*/
static void R_DoTriSplitting()
{
    cTerraPatchUnpacked_t *patch;

    for (patch = tr.world->activeTerraPatches; patch; patch = patch->pNextActive) {
        if (patch->uiDistRecalc > g_uiTerDist) {
            continue;
        }

        patch->uiDistRecalc = -1;
        g_tri.iCur          = patch->drawinfo.iTriHead;
        while (g_tri.iCur != 0) {
            terraTri_t *pTri = &g_pTris[g_tri.iCur];

            if (R_NeedSplitTri(pTri)) {
                //
                // make sure there are sufficient number of tris
                //
                // Fixed in OPM
                //  Properly check the number of tris, which is double the number of verts
                if (g_tri.nFree < 14*2 || g_vert.nFree < 14) {
                    ri.Printf(PRINT_DEVELOPER, "WARNING: aborting terrain tessellation -- insufficient tris\n");
                    return;
                }

                patch->uiDistRecalc = 0;
                R_ForceSplit(g_tri.iCur);

                if (&g_pTris[g_tri.iCur] == pTri) {
                    g_tri.iCur = g_pTris[g_tri.iCur].iNext;
                }
            } else {
                if ((pTri->byConstChecks & 3) != 2) {
                    if (patch->uiDistRecalc > pTri->uiDistRecalc) {
                        patch->uiDistRecalc = pTri->uiDistRecalc;
                    }
                }

                g_tri.iCur = g_pTris[g_tri.iCur].iNext;
            }
        }
    }
}

/*
================
R_DoGeomorphs
================
*/
static void R_DoGeomorphs()
{
    for (size_t n = 0; n < tr.world->numTerraPatches; n++) {
        cTerraPatchUnpacked_t *patch = &tr.world->terraPatches[n];

        g_vert.iCur = patch->drawinfo.iVertHead;

        if (patch->visCountDraw == g_terVisCount) {
            if (patch->byDirty) {
                while (g_vert.iCur) {
                    terrainVert_t *pVert = &g_pVert[g_vert.iCur];
                    R_CalcVertMorphHeight(pVert);
                    g_vert.iCur = pVert->iNext;
                }

                patch->byDirty = qfalse;
            } else {
                while (g_vert.iCur) {
                    terrainVert_t *pVert = &g_pVert[g_vert.iCur];
                    R_UpdateVertMorphHeight(pVert);
                    g_vert.iCur = pVert->iNext;
                }
            }
        } else {
            if (!patch->byDirty) {
                patch->byDirty = qtrue;
                while (g_vert.iCur) {
                    terrainVert_t *pVert = &g_pVert[g_vert.iCur];
                    pVert->xyz[2]        = pVert->fHgtAvg;
                    g_vert.iCur          = pVert->iNext;
                }
            }
        }
    }
}

/*
================
R_MergeInternalAggressive
================
*/
static qboolean R_MergeInternalAggressive()
{
    terraTri_t *pTri = &g_pTris[g_tri.iCur];
    terraTri_t *pBase;

    if (pTri->nSplit) {
        return qfalse;
    }

    if (g_pVert[g_pTris[pTri->iLeftChild].iPt[2]].xyz[2] != g_pVert[g_pTris[pTri->iLeftChild].iPt[2]].fHgtAvg) {
        return qfalse;
    }

    if (pTri->iBase) {
        pBase = &g_pTris[pTri->iBase];
        if (pBase->nSplit) {
            return qfalse;
        }

        R_ForceMerge(pTri->iBase);
    }

    R_ForceMerge(g_tri.iCur);
    return qtrue;
}

/*
================
R_MergeInternalCautious
================
*/
static qboolean R_MergeInternalCautious()
{
    terraTri_t *pTri;
    terraTri_t *pBase;

    pTri = &g_pTris[g_tri.iCur];

    if (pTri->nSplit) {
        return qfalse;
    }

    if (pTri->varnode->flags & 8) {
        return qfalse;
    }

    if (g_pVert[g_pTris[pTri->iLeftChild].iPt[2]].xyz[2] != g_pVert[g_pTris[pTri->iLeftChild].iPt[2]].fHgtAvg) {
        return qfalse;
    }

    if (pTri->iBase) {
        pBase = &g_pTris[pTri->iBase];
        if (pBase->nSplit || (pBase->varnode->flags & 8)) {
            return qfalse;
        }

        R_ForceMerge(pTri->iBase);
    }

    R_ForceMerge(g_tri.iCur);
    return qtrue;
}

/*
================
R_DoTriMerging
================
*/
static void R_DoTriMerging()
{
    int iCautiousFrame = g_terVisCount - ter_cautiousframes->integer;

    for (size_t n = 0; n < tr.world->numTerraPatches; n++) {
        const cTerraPatchUnpacked_t *patch = &tr.world->terraPatches[n];

        g_tri.iCur = patch->drawinfo.iMergeHead;
        if (patch->visCountDraw >= iCautiousFrame) {
            while (g_tri.iCur) {
                if (!R_MergeInternalCautious()) {
                    g_tri.iCur = g_pTris[g_tri.iCur].iNext;
                }
            }
        } else if (patch->visCountDraw > iCautiousFrame - 10) {
            while (g_tri.iCur) {
                if (!R_MergeInternalAggressive()) {
                    g_tri.iCur = g_pTris[g_tri.iCur].iNext;
                }
            }
        }
    }
}

/*
================
R_TessellateTerrain
================
*/
void R_TessellateTerrain()
{
    R_DoTriSplitting();
    // Morph geometry according to the view
    R_DoGeomorphs();
    // Merge vertices
    R_DoTriMerging();
}

/*
================
R_TerrainPrepareFrame
================
*/
void R_TerrainPrepareFrame()
{
    float distance;
    int   index;
    float fFov;
    float fCheck;
    float fDistBound;

    if (ter_lock->integer) {
        return;
    }

    if (tr.viewParms.isPortalSky) {
        return;
    }

    g_terVisCount++;
    tr.world->activeTerraPatches = NULL;

    if (ter_error->value < 0.1) {
        ri.Cvar_Set("ter_error", "0.1");
    }

    distance = 1.0;
    fFov     = tr.refdef.fov_x;
    if (fFov < 1.0) {
        fFov = 1.0;
    }

    fCheck = 1.0 / (tan(fFov / 114.0) * ter_error->value / 320.0);
    if (g_terVisCount) {
        float fFarPlane = tr.viewParms.farplane_distance;
        if (fFarPlane == 0.0) {
            fFarPlane = 4096.0;
        }

        fDistBound = fabs((fCheck - g_fCheck) * 510.0) + fabs((tr.refdef.vieworg[0] - g_vTerOrg[0]) * g_vTerFwd[0])
                   + fabs((tr.refdef.vieworg[1] - g_vTerOrg[1]) * g_vTerFwd[1])
                   + fabs((tr.refdef.viewaxis[0][0] - g_vTerFwd[0]) * fFarPlane)
                   + fabs((tr.refdef.viewaxis[0][1] - g_vTerFwd[1]) * fFarPlane);

        g_uiTerDist = (ceil(fDistBound) + (float)g_uiTerDist);
        if (g_uiTerDist > 0xF0000000) {
            for (index = 0; index < tr.world->numTerraPatches; index++) {
                tr.world->terraPatches[index].uiDistRecalc = 0;
            }

            for (index = 0; index < g_nTris; index++) {
                g_pTris[index].uiDistRecalc = 0;
            }

            for (index = 0; index < g_nVerts; index++) {
                g_pVert[index].uiDistRecalc = 0;
            }

            g_uiTerDist = 0;
        }
    } else {
        fDistBound  = 0.0;
        g_uiTerDist = 0;
    }

    VectorCopy2D(tr.refdef.vieworg, g_vTerOrg);
    VectorCopy(tr.refdef.viewaxis[0], g_vTerFwd);
    g_fCheck = fCheck;

    if (fDistBound != 0.0) {
        index    = (int)tr.refdef.fov_x;
        distance = g_fDistanceTable[index];

        g_fClipDotSquared = g_fClipDotSquaredTable[index];
        g_fClipDotProduct = g_fClipDotProductTable[index];
        VectorCopy(tr.refdef.viewaxis[0], g_vClipVector);
        g_vClipOrigin[0] = tr.refdef.vieworg[0] - distance * tr.refdef.viewaxis[0][0] - 256.0;
        g_vClipOrigin[1] = tr.refdef.vieworg[1] - distance * tr.refdef.viewaxis[0][1] - 256.0;
        g_vClipOrigin[2] = tr.refdef.vieworg[2] - distance * tr.refdef.viewaxis[0][2] - 255.0;
        g_vViewVector[0] = tr.refdef.viewaxis[0][0];
        g_vViewVector[1] = tr.refdef.viewaxis[0][1];
        g_vViewVector[2] =
            -(tr.refdef.viewaxis[0][0] * tr.refdef.vieworg[0] + tr.refdef.viewaxis[0][1] * tr.refdef.vieworg[1]);
        VectorCopy(tr.refdef.vieworg, g_vViewOrigin);

        if (tr.viewParms.farplane_distance > 0) {
            g_fFogDistance = distance + tr.viewParms.farplane_distance + 768.0;
        } else {
            g_fFogDistance = 999999.0;
        }
    }
}

/*
================
R_MarkTerrainPatch
================
*/
void R_MarkTerrainPatch(cTerraPatchUnpacked_t *pPatch)
{
    if (pPatch->visCountCheck == g_terVisCount) {
        return;
    }

    pPatch->visCountCheck = g_terVisCount;
    if (ter_cull->integer) {
        vec3_t v;
        float  dot;

        v[0] = pPatch->x0 - g_vClipOrigin[0];
        v[1] = pPatch->y0 - g_vClipOrigin[1];
        v[2] = pPatch->z0 - g_vClipOrigin[2];

        dot = DotProduct(v, g_vClipVector);
        if (dot > g_fFogDistance) {
            return;
        }

        if (VectorLengthSquared(v) * g_fClipDotSquared > dot * dot) {
            return;
        }
    }

    pPatch->visCountDraw         = g_terVisCount;
    pPatch->pNextActive          = tr.world->activeTerraPatches;
    tr.world->activeTerraPatches = pPatch;
}

/*
================
R_AddTerrainSurfaces
================
*/
void R_AddTerrainSurfaces()
{
    int                    i;
    int                    dlight;
    cTerraPatchUnpacked_t *patch;

    if (tr.world->numTerraPatches < 0) {
        return;
    }

    if (!ter_lock->integer) {
        R_TessellateTerrain();
    }

    if (ter_count->integer || ter_lock->integer == 2) {
        for (i = 0; i < tr.world->numTerraPatches; i++) {
            patch = &tr.world->terraPatches[i];

            if (ter_lock->integer == 2 || patch->visCountDraw == g_terVisCount) {
                assert(patch->shader);

                dlight = R_CheckDlightTerrain(patch, (1 << (tr.refdef.num_dlights)) - 1);
                R_AddDrawSurf((surfaceType_t *)&patch->drawinfo, patch->shader, dlight);
            }

            if (ter_count->integer && (g_nSplit || g_nMerge)) {
                if (ter_count->integer == 1 || g_nSplit * 2 != g_nMerge) {
                    ri.Printf(
                        PRINT_DEVELOPER,
                        "%5zu tris / %5zu verts / %4d splits / %4d merges\n",
                        g_nTris - g_tri.nFree,
                        g_nVerts - g_vert.nFree,
                        g_nSplit,
                        g_nMerge
                    );
                }

                g_nSplit = 0;
                g_nMerge = 0;
            }
        }
    } else {
        for (patch = tr.world->activeTerraPatches; patch; patch = patch->pNextActive) {
            assert(patch->shader);

            dlight = R_CheckDlightTerrain(patch, (1 << (tr.refdef.num_dlights)) - 1);
            R_AddDrawSurf((surfaceType_t *)&patch->drawinfo, patch->shader, dlight);
        }
    }
}

/*
================
R_TerrainHeightForPoly

Calculates the height of a terrain polygon by interpolating the heights of its vertices.
The function finds the triangle containing the given point (x, y) within the terrain patch,
and then computes the height at each vertex of the polygon using barycentric interpolation.
Finally, it updates the z-coordinate of each vertex to store the calculated height.
Returns qtrue if the triangle was found and the height could be calculated and saved into its vertices.

ToDo: try to use the Vector*(?) macros for the math operations below from q_shared.h and q_math.h, not sure which ones would be a good fit
================
*/
qboolean R_TerrainHeightForPoly(cTerraPatchUnpacked_t *pPatch, polyVert_t *pVerts, int nVerts)
{
    float x0     = 0;
    float y0     = 0;
    float x1     = 0;
    float y1     = 0;
    float x2     = 0;
    float y2     = 0;
    float fKx[3] = {0}, fKy[3] = {0}, fKz[3] = {0}, fArea[3] = {0};
    float fAreaTotal = 0;

    float    x = pVerts->xyz[0];
    float    y = pVerts->xyz[1];
    terraInt iTri;

    // Calculate the average of the x and y coordinates of all vertices
    if (nVerts > 1) {
        for (int i = 1; i < nVerts; i++) {
            x += pVerts[i].xyz[0];
            y += pVerts[i].xyz[1];
        }

        // Use the averaged values from this point onwards
        x = x / nVerts;
        y = y / nVerts;
    }

    // Get the first valid triangle in the patch
    iTri = pPatch->drawinfo.iTriHead;

    // Find a triangle that contains the point (x, y)
    for (iTri = pPatch->drawinfo.iTriHead; iTri; iTri = g_pTris[iTri].iNext) {
        if (g_pTris[iTri].byConstChecks & 4) {
            // Get all three x-y coordinates of the current triangle's vertices
            x0 = g_pVert[g_pTris[iTri].iPt[0]].xyz[0];
            y0 = g_pVert[g_pTris[iTri].iPt[0]].xyz[1];
            x1 = g_pVert[g_pTris[iTri].iPt[1]].xyz[0];
            y1 = g_pVert[g_pTris[iTri].iPt[1]].xyz[1];
            x2 = g_pVert[g_pTris[iTri].iPt[2]].xyz[0];
            y2 = g_pVert[g_pTris[iTri].iPt[2]].xyz[1];

            // Calculate the signed areas of the three sub-triangles
            fArea[0] = (x - x1) * (y2 - y1) - (y - y1) * (x2 - x1);
            if (fArea[0] < -0.1) {
                // The point is outside the triangle
                continue;
            }

            fArea[1] = (x - x2) * (y0 - y2) - (y - y2) * (x0 - x2);
            if (fArea[1] < -0.1) {
                // The point is outside the triangle
                continue;
            }

            fArea[2] = (x - x0) * (y1 - y0) - (y - y0) * (x1 - x0);
            if (fArea[2] < -0.1) {
                // The point is outside the triangle
                continue;
            }

            fAreaTotal = fArea[0] + fArea[1] + fArea[2];
            if (fAreaTotal > 0.0) {
                //
                // Found it - the point is inside the triangle
                // Calculate the barycentric coordinates (fKx, fKy, fKz) of the triangle
                //

                float z0 = g_pVert[g_pTris[iTri].iPt[0]].xyz[2] / fAreaTotal;
                float z1 = g_pVert[g_pTris[iTri].iPt[1]].xyz[2] / fAreaTotal;
                float z2 = g_pVert[g_pTris[iTri].iPt[2]].xyz[2] / fAreaTotal;

                fKy[0] = z0 * (x2 - x1);
                fKy[1] = z1 * (x0 - x2);
                fKy[2] = z2 * (x1 - x0);

                fKx[0] = z0 * (y2 - y1);
                fKx[1] = z1 * (y0 - y2);
                fKx[2] = z2 * (y1 - y0);

                // Note: this could be done with the CrossProduct macro if everything were in a vector
                fKz[0] = fKx[0] * x1 - y1 * fKy[0];
                fKz[1] = fKx[1] * x2 - y2 * fKy[1];
                fKz[2] = fKx[2] * x0 - y0 * fKy[2];

                // Calculate the height for each vertex
                for (int i = 0; i < nVerts; i++) {
                    float fScaleX = pVerts[i].xyz[0] * (fKx[0] + fKx[1] + fKx[2]);
                    float fScaleY = pVerts[i].xyz[1] * (fKy[0] + fKy[1] + fKy[2]);
                    float fConstZ = fKz[0] + fKz[1] + fKz[2];

                    // Write back the calculated height into the vertex
                    pVerts[i].xyz[2] = fScaleX - fScaleY - fConstZ;
                }

                return qtrue;
            }
        }
    }

    assert(
        !pPatch->drawinfo.iTriHead
        && va("R_TerrainHeightForPoly: point(%f %f) not in patch(%f %f %f)\n", x, y, pPatch->x0, pPatch->y0, pPatch->z0)
    );
    return qfalse;
}

/*
================
R_TerrainRestart_f
================
*/
void R_TerrainRestart_f(void)
{
    if (tr.world->numTerraPatches < 0) {
        return;
    }

    R_TerrainFree();

    R_PreTessellateTerrain();
}

/*
================
R_CraterTerrain
================
*/
void R_CraterTerrain(const vec3_t pos, const vec3_t dir, float fDepth, float fRadius)
{
    int                    i, j, k;
    int                    index;
    qboolean               bMorphed;
    cTerraPatchUnpacked_t *pPatch;
    float                  x, y, z;
    float                  dzSquared;
    float                  heightmap[81];
    float                  fMinHeight, fMaxHeight;
    float                  fMaxOrig;
    vec3_t                 vArcCenter;
    float                  fArcRadius;
    float                  fArcSquared;

    fArcRadius  = (fRadius * fRadius / fDepth + fDepth) * 0.5;
    fArcSquared = Square(fArcRadius);
    VectorMA(pos, fDepth - fArcRadius, dir, vArcCenter);

    for (i = 0; i < tr.world->numTerraPatches; i++) {
        pPatch = &tr.world->terraPatches[i];

        bMorphed = qfalse;

        fMaxOrig   = pPatch->z0;
        fMaxHeight = pPatch->z0;
        fMinHeight = pPatch->z0;

        for (j = 0; j < 9; j++) {
            for (k = 0; k < 9; k++) {
                index = k + j * 9;

                x = pPatch->x0 + (k << 6);
                y = pPatch->y0 + (j << 6);
                z = pPatch->z0 + (pPatch->heightmap[index] * 2);

                heightmap[index] = z;

                if (fMaxHeight < z) {
                    fMaxHeight = z;
                }
                if (fMaxOrig < z) {
                    fMaxOrig = z;
                }

                dzSquared = fArcSquared - (Square(x - vArcCenter[0]) + Square(y - vArcCenter[1]));

                if (Square(z - vArcCenter[2]) > dzSquared) {
                    continue;
                }

                if (pPatch->flags & 0x40) {
                    heightmap[index] = vArcCenter[2] + sqrt(dzSquared);
                    if (fMaxHeight < heightmap[index]) {
                        fMaxHeight = heightmap[index];
                    }
                } else {
                    heightmap[index] = vArcCenter[2] - sqrt(dzSquared);
                    if (fMinHeight > heightmap[index]) {
                        fMinHeight = heightmap[index];
                    }
                }

                bMorphed = qtrue;
            }
        }

        if (!bMorphed) {
            continue;
        }

        if (fMinHeight < fMaxOrig - 510.0) {
            fMinHeight = fMaxOrig - 510.0;
        }

        pPatch->z0 = floor((fMinHeight + 1.0) * 0.5) * 2.0;

        for (j = 0; j < 9; j++) {
            for (k = 0; k < 9; k++) {
                index = k + j * 9;

                pPatch->heightmap[index] = Q_clamp_float(floor((heightmap[index] - pPatch->z0 + 1.0) * 0.5), 0, 255);
            }
        }
    }
}

/*
================
R_TerrainCrater_f
================
*/
void R_TerrainCrater_f(void)
{
    vec3_t dir;

    if (!ri.Cvar_VariableIntegerValue("cheats")) {
        //
        // Added in OPM
        //  This command may be used for debugging purposes.
        //  It also prevents executing this command accidentally
        //
        return;
    }

    VectorNegate(tr.refdef.viewaxis[2], dir);

    R_CraterTerrain(tr.refdef.vieworg, dir, 256.0, 256.0);
    R_TerrainRestart_f();
}

/*
================
R_InitTerrain
================
*/
void R_InitTerrain()
{
    int i;

    ter_maxlod = ri.Cvar_Get("ter_maxlod", "6", CVAR_ARCHIVE | CVAR_TERRAIN_LATCH);
    ri.Cvar_CheckRange(ter_maxlod, 3, 6, qtrue);

    ter_cull = ri.Cvar_Get("ter_cull", "1", 0);
    ter_lock = ri.Cvar_Get("ter_lock", "0", CVAR_CHEAT);

    ter_error = ri.Cvar_Get("ter_error", "4", CVAR_ARCHIVE);
    ri.Cvar_CheckRange(ter_error, 0.1, 16, qfalse);

    ter_cautiousframes = ri.Cvar_Get("ter_cautiousframes", "1", CVAR_ARCHIVE);

    ter_maxtris = ri.Cvar_Get("ter_maxtris", "24576", CVAR_TERRAIN_LATCH);
    ri.Cvar_CheckRange(ter_maxtris, 16384, 65536, qtrue);

    ter_count = ri.Cvar_Get("ter_count", "0", 0);

    ri.Cmd_AddCommand("ter_restart", R_TerrainRestart_f);
    R_PreTessellateTerrain();

    for (i = 0; i < TERRAIN_TABLE_SIZE; i++) {
        float dot                 = (1.0 / (0.82 - cos((float)i / 57.29) * 0.18) - 1.0) * (16.0 / 9.0);
        g_fClipDotSquaredTable[i] = dot;
        g_fClipDotProductTable[i] = sqrt(dot);
        g_fDistanceTable[i]       = 443.5 / sqrt(1.0 - dot);
    }
}

/*
================
R_ShutdownTerrain
================
*/
void R_ShutdownTerrain()
{
    ri.Cmd_RemoveCommand("ter_restart");

    R_TerrainFree();
}

/*
================
R_TerrainFree

Frees terrain memory
================
*/
void R_TerrainFree()
{
    if (g_pVert) {
        ri.Free(g_pVert);
        g_pVert = NULL;
    }
    if (g_pTris) {
        ri.Free(g_pTris);
        g_pTris = NULL;
    }
}

/*
====================
R_SwapTerraPatch

Swaps the patch on big-endian
====================
*/
void R_SwapTerraPatch(cTerraPatch_t *pPatch)
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
