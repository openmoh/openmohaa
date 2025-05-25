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
 * @file navigation_bsp_load_terrain.cpp
 * @brief Render LOD terrain from BSP into triangles and vertices.
 * 
 */

#include "g_local.h"
#include "navigation_bsp.h"

static const unsigned int MAX_TERRAIN_LOD = 6;

static size_t  g_nTris;
static size_t  g_nVerts;
static int     g_nMerge;
static int     g_nSplit;
terraTri_t    *g_pTris = NULL;
terrainVert_t *g_pVert = NULL;

poolInfo_t g_tri;
poolInfo_t g_vert;

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

varnodeIndex_t g_vni[2][8][8][2];

static int modeTable[] = {2, 2, 5, 6, 4, 3, 0, 0};

/*
================
G_ValidateHeightmapForVertex
================
*/
static void G_ValidateHeightmapForVertex(terraTri_t *pTri)
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
G_AllocateVert
================
*/
static terraInt G_AllocateVert(cTerraPatchUnpacked_t *patch)
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

    g_pVert[iVert].nRef = 0;

    return iVert;
}

/*
================
G_InterpolateVert
================
*/
static void G_InterpolateVert(terraTri_t *pTri, terrainVert_t *pVert)
{
    const terrainVert_t         *pVert0 = &g_pVert[pTri->iPt[0]];
    const terrainVert_t         *pVert1 = &g_pVert[pTri->iPt[1]];
    const cTerraPatchUnpacked_t *pPatch = pTri->patch;
    const byte                  *pMinHeight, *pMaxHeight;

    // Fixed in OPM
    //  Use the delta of the two pointers instead of adding them + divide to get the average height,
    //  otherwise the resulting integer would overflow/warp and cause a segmentation fault.
    //  Although rare, the overflow issue can occur in the original game
    pMinHeight  = (byte *)Q_min((uintptr_t)pVert0->pHgt, (uintptr_t)pVert1->pHgt);
    pMaxHeight  = (byte *)Q_max((uintptr_t)pVert0->pHgt, (uintptr_t)pVert1->pHgt);
    pVert->pHgt = (byte *)(pMinHeight + ((pMaxHeight - pMinHeight) >> 1));
    assert(pVert->pHgt >= pMinHeight && pVert->pHgt < pMaxHeight || pMinHeight == pMaxHeight);

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
G_ReleaseVert
================
*/
static void G_ReleaseVert(cTerraPatchUnpacked_t *patch, int iVert)
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
G_AllocateTri
================
*/
terraInt G_AllocateTri(cTerraPatchUnpacked_t *patch, qboolean check)
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
    g_pTris[iTri].iParent       = 0;

    return iTri;
}

/*
================
R_ReleaseTri
================
*/
static void G_ReleaseTri(cTerraPatchUnpacked_t *patch, terraInt iTri)
{
    terraTri_t *pTri = &g_pTris[iTri];

    terraInt iPrev       = pTri->iPrev;
    terraInt iNext       = pTri->iNext;
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

    pTri->iPrev                    = 0;
    pTri->iNext                    = g_tri.iFreeHead;
    g_pTris[g_tri.iFreeHead].iPrev = iTri;
    g_tri.iFreeHead                = iTri;
    g_tri.nFree++;

    for (int i = 0; i < 3; i++) {
        terraInt ptNum = pTri->iPt[i];

        g_pVert[ptNum].nRef--;
        if (!g_pVert[ptNum].nRef) {
            G_ReleaseVert(patch, ptNum);
        }
    }
}

/*
================
G_ConstChecksForTri
================
*/
static int G_ConstChecksForTri(terraTri_t *pTri)
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
    } else if ((pTri->byConstChecks & 4) && !(pTri->varnode->flags & 4) && pTri->lod < MAX_TERRAIN_LOD) {
        return 0;
    }

    return 2;
}

/*
================
G_DemoteInAncestry
================
*/
static void G_DemoteInAncestry(cTerraPatchUnpacked_t *patch, terraInt iTri)
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
static void G_TerrainHeapInit()
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
static void G_TerrainPatchesInit(cTerraPatchUnpacked_t *terraPatches, size_t numTerraPatches)
{
    int i;

    for (i = 0; i < numTerraPatches; i++) {
        cTerraPatchUnpacked_t *patch = &terraPatches[i];
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
G_SplitTri
================
*/

void G_SplitTri(
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

        G_ValidateHeightmapForVertex(pLeft);
        pLeft->byConstChecks |= G_ConstChecksForTri(pLeft);

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

        G_ValidateHeightmapForVertex(pRight);
        pRight->byConstChecks |= G_ConstChecksForTri(pRight);

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
G_ForceSplit
================
*/
static void G_ForceSplit(terraInt iTri)
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

        G_ForceSplit(iBase);

        assert(g_tri.nFree >= 4);

        iBase = pTri->iBase;
        pBase = &g_pTris[iBase];
    }

    flags = pTri->varnode->flags;

    iTriLeft  = G_AllocateTri(pTri->patch, (flags & 2));
    iTriRight = G_AllocateTri(pTri->patch, (flags & 1));

    iNewPt = G_AllocateVert(pTri->patch);
    G_InterpolateVert(pTri, &g_pVert[iNewPt]);

    g_pVert[iNewPt].fVariance = pTri->varnode->fVariance;

    iBaseLeft  = 0;
    iBaseRight = 0;

    if (iBase) {
        flags2 = pBase->varnode->flags;
        flags |= flags2;

        iBaseLeft  = G_AllocateTri(pBase->patch, (flags2 & 2));
        iBaseRight = G_AllocateTri(pBase->patch, (flags2 & 1));

        iNewBasePt = iNewPt;
        if (pBase->patch != pTri->patch) {
            iNewBasePt = G_AllocateVert(pBase->patch);
            pVert      = &g_pVert[iNewBasePt];
            G_InterpolateVert(pBase, pVert);

            pVert->fVariance = g_pVert[iNewPt].fVariance;
            if (flags & 8) {
                pVert->fHgtAvg += pVert->fHgtAdd;
                pVert->fHgtAdd   = 0.0;
                pVert->fVariance = 0.0;
                pVert->xyz[2]    = pVert->fHgtAvg;
            }
        }

        G_SplitTri(iBase, iNewBasePt, iBaseLeft, iBaseRight, iTriRight, iTriLeft);

        pBase->iLeftChild           = iBaseLeft;
        pBase->iRightChild          = iBaseRight;
        g_pTris[iBaseLeft].iParent  = iBase;
        g_pTris[iBaseRight].iParent = iBase;
        G_DemoteInAncestry(pBase->patch, iBase);
    }

    if (flags & 8) {
        pVert = &g_pVert[iNewPt];
        pVert->fHgtAvg += pVert->fHgtAdd;
        pVert->fHgtAdd   = 0.0;
        pVert->fVariance = 0.0;
        pVert->xyz[2]    = pVert->fHgtAvg;
    }

    G_SplitTri(iTri, iNewPt, iTriLeft, iTriRight, iBaseRight, iBaseLeft);

    pTri->iLeftChild           = iTriLeft;
    pTri->iRightChild          = iTriRight;
    g_pTris[iTriLeft].iParent  = iTri;
    g_pTris[iTriRight].iParent = iTri;
    G_DemoteInAncestry(pTri->patch, iTri);
}

/*
================
G_ForceMerge
================
*/
static void G_ForceMerge(terraInt iTri)
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

        G_ReleaseTri(pTri->patch, pTri->iLeftChild);

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

        G_ReleaseTri(pTri->patch, pTri->iRightChild);

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
G_TerraTriNeighbor
================
*/
static int G_TerraTriNeighbor(cTerraPatchUnpacked_t *terraPatches, int iPatch, int dir)
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
G_PreTessellateTerrain
================
*/
void G_PreTessellateTerrain(cTerraPatchUnpacked_t *terraPatches, size_t numTerraPatches)
{
    if (!numTerraPatches) {
        return;
    }

    g_nTris  = numTerraPatches * 8 * 8 * 6 * MAX_TERRAIN_LOD + 1;
    g_nVerts = g_nTris;
    g_pTris  = (terraTri_t *)gi.Malloc(g_nTris * sizeof(terraTri_t));
    g_pVert  = (terrainVert_t *)gi.Malloc(g_nVerts * sizeof(terrainVert_t));

    // Init triangles & vertices
    G_TerrainHeapInit();
    G_TerrainPatchesInit(terraPatches, numTerraPatches);

    for (size_t i = 0; i < numTerraPatches; i++) {
        cTerraPatchUnpacked_t *patch = &terraPatches[i];

        patch->drawinfo.nTris     = 0;
        patch->drawinfo.nVerts    = 0;
        patch->drawinfo.iTriHead  = 0;
        patch->drawinfo.iTriTail  = 0;
        patch->drawinfo.iVertHead = 0;

        terraInt iTri0 = G_AllocateTri(patch, qfalse);
        terraInt iTri1 = G_AllocateTri(patch, qfalse);
        terraInt i00   = G_AllocateVert(patch);
        terraInt i01   = G_AllocateVert(patch);
        terraInt i10   = G_AllocateVert(patch);
        terraInt i11   = G_AllocateVert(patch);

        terrainVert_t *pVert;
        pVert            = &g_pVert[i00];
        pVert->xyz[0]    = patch->x0;
        pVert->xyz[1]    = patch->y0;
        pVert->xyz[2]    = (float)(patch->heightmap[0] * 2) + patch->z0;
        pVert->pHgt      = &patch->heightmap[0];
        pVert->fHgtAvg   = pVert->xyz[2];
        pVert->fVariance = 0.0f;
        pVert->nRef      = 4;

        pVert            = &g_pVert[i01];
        pVert->xyz[0]    = patch->x0;
        pVert->xyz[1]    = patch->y0 + 512.0f;
        pVert->xyz[2]    = (float)(patch->heightmap[72] * 2) + patch->z0;
        pVert->pHgt      = &patch->heightmap[72];
        pVert->fHgtAvg   = pVert->xyz[2];
        pVert->fVariance = 0.0f;
        pVert->nRef      = 4;

        pVert            = &g_pVert[i10];
        pVert->xyz[0]    = patch->x0 + 512.0f;
        pVert->xyz[1]    = patch->y0;
        pVert->xyz[2]    = (float)(patch->heightmap[8] * 2) + patch->z0;
        pVert->pHgt      = &patch->heightmap[8];
        pVert->fHgtAvg   = pVert->xyz[2];
        pVert->fVariance = 0.0f;
        pVert->nRef      = 4;

        pVert            = &g_pVert[i11];
        pVert->xyz[0]    = patch->x0 + 512.0f;
        pVert->xyz[1]    = patch->y0 + 512.0f;
        pVert->xyz[2]    = (float)(patch->heightmap[80] * 2) + patch->z0;
        pVert->pHgt      = &patch->heightmap[80];
        pVert->fHgtAvg   = pVert->xyz[2];
        pVert->fVariance = 0.0f;
        pVert->nRef      = 4;

        terraTri_t *pTri = &g_pTris[iTri0];
        pTri->patch      = patch;
        pTri->varnode    = &patch->varTree[0][0];
        pTri->index      = 1;
        pTri->lod        = 0;
        pTri->byConstChecks |= G_ConstChecksForTri(pTri);

        pTri->iBase = iTri1;
        if ((patch->flags & 0x80u) == 0) {
            pTri->iLeft  = G_TerraTriNeighbor(terraPatches, patch->iWest, 1);
            pTri->iRight = G_TerraTriNeighbor(terraPatches, patch->iNorth, 2);
            if (patch->flags & 0x40) {
                pTri->iPt[0] = i00;
                pTri->iPt[1] = i11;
            } else {
                pTri->iPt[0] = i11;
                pTri->iPt[1] = i00;
            }
            pTri->iPt[2] = i01;
        } else {
            pTri->iLeft  = G_TerraTriNeighbor(terraPatches, patch->iNorth, 2);
            pTri->iRight = G_TerraTriNeighbor(terraPatches, patch->iEast, 3);
            if (patch->flags & 0x40) {
                pTri->iPt[0] = i01;
                pTri->iPt[1] = i10;
            } else {
                pTri->iPt[0] = i10;
                pTri->iPt[1] = i01;
            }
            pTri->iPt[2] = i11;
        }

        G_ValidateHeightmapForVertex(pTri);

        pTri          = &g_pTris[iTri1];
        pTri->patch   = patch;
        pTri->varnode = &patch->varTree[1][0];
        pTri->index   = 1;
        pTri->lod     = 0;
        pTri->byConstChecks |= G_ConstChecksForTri(pTri);

        pTri->iBase = iTri0;
        if ((patch->flags & 0x80u) == 0) {
            pTri->iLeft  = G_TerraTriNeighbor(terraPatches, patch->iEast, 3);
            pTri->iRight = G_TerraTriNeighbor(terraPatches, patch->iSouth, 0);
            if (patch->flags & 0x40) {
                pTri->iPt[0] = i11;
                pTri->iPt[1] = i00;
            } else {
                pTri->iPt[0] = i00;
                pTri->iPt[1] = i11;
            }
            pTri->iPt[2] = i10;
        } else {
            pTri->iLeft  = G_TerraTriNeighbor(terraPatches, patch->iSouth, 0);
            pTri->iRight = G_TerraTriNeighbor(terraPatches, patch->iWest, 1);
            if (patch->flags & 0x40) {
                pTri->iPt[0] = i10;
                pTri->iPt[1] = i01;
            } else {
                pTri->iPt[0] = i01;
                pTri->iPt[1] = i10;
            }
            pTri->iPt[2] = i00;
        }

        G_ValidateHeightmapForVertex(pTri);
    }
}

/*
================
G_NeedSplitTri
================
*/
static qboolean G_NeedSplitTri(terraTri_t *pTri)
{
    uint8_t byConstChecks = pTri->byConstChecks;
    if (byConstChecks & 2) {
        return byConstChecks & 1;
    }

    return qtrue;
}

/*
================
G_CalcVertMorphHeight
================
*/
static void G_CalcVertMorphHeight(terrainVert_t *pVert)
{
    pVert->xyz[2] = pVert->fHgtAvg;
    if (pVert->fVariance > 0) {
        pVert->xyz[2] += pVert->fHgtAdd;
    }
}

/*
================
G_UpdateVertMorphHeight
================
*/
static void G_UpdateVertMorphHeight(terrainVert_t *pVert)
{
    G_CalcVertMorphHeight(pVert);
}

/*
================
G_DoTriSplitting
================
*/
void G_DoTriSplitting(cTerraPatchUnpacked_t *terraPatches, size_t numTerraPatches)
{
    cTerraPatchUnpacked_t *patch;
    size_t                 i;

    for (i = 0; i < numTerraPatches; i++) {
        patch = &terraPatches[i];

        g_tri.iCur = patch->drawinfo.iTriHead;
        while (g_tri.iCur != 0) {
            terraTri_t *pTri = &g_pTris[g_tri.iCur];

            if (G_NeedSplitTri(pTri)) {
                if (g_tri.nFree < 14 * 2 || g_vert.nFree < 14) {
                    // this shouldn't happen
                    throw("aborting terrain tessellation -- insufficient tris\n");
                }

                G_ForceSplit(g_tri.iCur);

                if (&g_pTris[g_tri.iCur] == pTri) {
                    g_tri.iCur = g_pTris[g_tri.iCur].iNext;
                }
            } else {
                g_tri.iCur = g_pTris[g_tri.iCur].iNext;
            }
        }
    }
}

/*
================
G_DoGeomorphs
================
*/
void G_DoGeomorphs(cTerraPatchUnpacked_t *terraPatches, size_t numTerraPatches)
{
    for (size_t n = 0; n < numTerraPatches; n++) {
        cTerraPatchUnpacked_t *patch = &terraPatches[n];

        g_vert.iCur = patch->drawinfo.iVertHead;

        while (g_vert.iCur) {
            terrainVert_t *pVert = &g_pVert[g_vert.iCur];
            G_UpdateVertMorphHeight(pVert);
            g_vert.iCur = pVert->iNext;
        }
    }
}

/*
================
G_TerrainFree
================
*/
void G_TerrainFree()
{
    if (g_pVert) {
        gi.Free(g_pVert);
        g_pVert = NULL;
    }
    if (g_pTris) {
        gi.Free(g_pTris);
        g_pTris = NULL;
    }
}

/*
====================
G_CalculateTerrainIndices
====================
*/
void G_CalculateTerrainIndices(worknode_t *worknode, int iDiagonal, int iTree)
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
G_PrepareGenerateTerrainCollide
====================
*/
void G_PrepareGenerateTerrainCollide(void)
{
    worknode_t worknode[64];

    memset(&g_vni, 0, sizeof(g_vni));

    worknode[1].i0 = 8;
    worknode[1].j0 = 8;
    worknode[1].i1 = 0;
    worknode[1].j1 = 0;
    worknode[1].i2 = 0;
    worknode[1].j2 = 8;

    G_CalculateTerrainIndices(worknode, 0, 0);

    worknode[1].i0 = 0;
    worknode[1].j0 = 0;
    worknode[1].i1 = 8;
    worknode[1].j1 = 8;
    worknode[1].i2 = 8;
    worknode[1].j2 = 0;

    G_CalculateTerrainIndices(worknode, 0, 1);

    worknode[1].i0 = 8;
    worknode[1].j0 = 0;
    worknode[1].i1 = 0;
    worknode[1].j1 = 8;
    worknode[1].i2 = 8;
    worknode[1].j2 = 8;

    G_CalculateTerrainIndices(worknode, 1, 0);

    worknode[1].i0 = 0;
    worknode[1].j0 = 8;
    worknode[1].i1 = 8;
    worknode[1].j1 = 0;
    worknode[1].i2 = 0;
    worknode[1].j2 = 0;

    G_CalculateTerrainIndices(worknode, 1, 1);
}

void G_PickTerrainSquareMode(terrainCollideSquare_t *square, vec3_t vTest, int i, int j, cTerraPatch_t *patch)
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
G_GenerateTerrainCollide
====================
*/
void G_GenerateTerrainCollide(cTerraPatch_t *patch, terrainCollide_t *tc)
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
                    G_PlaneFromPoints(square->plane[0], v4, v2, v3);
                    G_PlaneFromPoints(square->plane[1], v2, v4, v1);
                } else {
                    G_PlaneFromPoints(square->plane[0], v2, v4, v3);
                    G_PlaneFromPoints(square->plane[1], v4, v2, v1);
                }
                G_PickTerrainSquareMode(square, v1, i, j, patch);
            } else {
                if (patch->flags & TERPATCH_FLIP) {
                    G_PlaneFromPoints(square->plane[0], v1, v3, v4);
                    G_PlaneFromPoints(square->plane[1], v3, v1, v2);
                } else {
                    G_PlaneFromPoints(square->plane[0], v3, v1, v4);
                    G_PlaneFromPoints(square->plane[1], v1, v3, v2);
                }
                G_PickTerrainSquareMode(square, v2, i, j, patch);
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
============
BoundTerPatch
============
*/
static qboolean BoundTerPatch(const terrainCollide_t& tc)
{
    int        i, j, k;
    vec3_t     bounds[2];
    winding_t *w;

    ClearBounds(bounds[0], bounds[1]);
    for (i = 0; i < ARRAY_LEN(tc.squares); i++) {
        for (j = 0; j < ARRAY_LEN(tc.squares[i]); j++) {
            w = tc.squares[i][j].w;
            if (w == NULL) {
                continue;
            }
            for (k = 0; k < w->numpoints; k++) {
                AddPointToBounds(w->p[k], bounds[0], bounds[1]);
            }
        }
    }

    for (i = 0; i < 3; i++) {
        if (bounds[0][i] < MIN_WORLD_COORD || bounds[1][i] > MAX_WORLD_COORD || bounds[0][i] >= bounds[1][i]) {
            return qfalse;
        }
    }

    return qtrue;
}

/*
============
G_CreateTerPatchWindings
============
*/
qboolean G_CreateTerPatchWindings(terrainCollide_t& tc)
{
    int                     i, j, k, l;
    winding_t              *w;
    terrainCollideSquare_t *side, *side2;
    cplane_t                plane;

    /* walk the list of brush sides */
    for (i = 0; i < ARRAY_LEN(tc.squares); i++) {
        for (j = 0; j < ARRAY_LEN(tc.squares[i]); j++) {
            /* get side and plane */
            side = &tc.squares[i][j];

            /* make huge winding */
            w = BaseWindingForPlane(side->plane[0], side->plane[0][3]);
            /* walk the list of brush sides */
            for (k = 0; k < ARRAY_LEN(tc.squares) && w; k++) {
                for (l = 0; l < ARRAY_LEN(tc.squares[k]) && w; l++) {
                    if (i == k && j == l) {
                        continue;
                    }

                    side2 = &tc.squares[k][l];
                    ChopWindingInPlace(&w, side2->plane[1], side2->plane[1][3], 0); // CLIP_EPSILON );

                    FixWinding(w);
                }
            }
#if 0
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
#endif

            if (side->w) {
                FreeWinding(side->w);
            }
            /* set side winding */
            side->w = w;
        }
    }

    /* find brush bounds */
    return BoundTerPatch(tc);
}

/*
============
terrainCollideSquare_t::terrainCollideSquare_t
============
*/
terrainCollideSquare_t::terrainCollideSquare_t()
{
    w = NULL;
}

/*
============
terrainCollideSquare_t::~terrainCollideSquare_t
============
*/
terrainCollideSquare_t::~terrainCollideSquare_t()
{
    if (w) {
        FreeWinding(w);
    }
}
