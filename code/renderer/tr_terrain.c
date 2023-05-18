/*
===========================================================================
Copyright (C) 2015 the OpenMoHAA team

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

cvar_t* ter_maxlod;
cvar_t* ter_maxtris;
cvar_t* ter_cull;
cvar_t* ter_lock;
cvar_t* ter_error;
cvar_t* ter_cautiousframes;
cvar_t* ter_count;

#define TERRAIN_TABLE_SIZE	180

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

static int g_terVisCount;
static float g_fCheck;
static size_t g_nTris;
static size_t g_nVerts;
static int g_nMerge;
static int g_nSplit;
unsigned int g_uiTerDist;
vec2_t g_vTerOrg;
vec3_t g_vTerFwd;

static const unsigned int MAX_TERRAIN_LOD = 6;
static const float TERRAIN_LIGHTMAP_SIZE = 128.0f;

typedef struct poolInfo_s
{
	terraInt iFreeHead;
	terraInt iCur;
	size_t nFree;
} poolInfo_t;

static int modeTable[8];

terraTri_t* g_pTris;
terrainVert_t* g_pVert;

poolInfo_t g_tri;
poolInfo_t g_vert;

static int R_AllocateVert(cTerraPatchUnpacked_t* patch)
{
	terraInt iVert = g_vert.iFreeHead;

	g_vert.iFreeHead = g_pVert[g_vert.iFreeHead].iNext;
	g_pVert[g_vert.iFreeHead].iPrev = 0;

	g_pVert[iVert].iPrev = 0;
	g_pVert[iVert].iNext = patch->drawinfo.iVertHead;
	g_pVert[patch->drawinfo.iVertHead].iPrev = iVert;

	patch->drawinfo.iVertHead = iVert;
	patch->drawinfo.nVerts++;
	g_vert.nFree--;

	g_pVert[iVert].nRef = 0;
	g_pVert[iVert].uiDistRecalc = 0;

	return iVert;
}

static void R_InterpolateVert(terraTri_t* pTri, terrainVert_t* pVert)
{
	const terrainVert_t* pVert0 = &g_pVert[pTri->iPt[0]];
	const terrainVert_t* pVert1 = &g_pVert[pTri->iPt[1]];
	const cTerraPatchUnpacked_t* pPatch = pTri->patch;

	// Interpolate texture coordinates
	pVert->texCoords[0][0] = (pVert0->texCoords[0][0] + pVert1->texCoords[0][0]) * 0.5f;
	pVert->texCoords[0][1] = (pVert0->texCoords[0][1] + pVert1->texCoords[0][1]) * 0.5f;
	pVert->texCoords[1][0] = (pVert0->texCoords[1][0] + pVert1->texCoords[1][0]) * 0.5f;
	pVert->texCoords[1][1] = (pVert0->texCoords[1][1] + pVert1->texCoords[1][1]) * 0.5f;

	pVert->pHgt = (uint8_t*)(((size_t)pVert0->pHgt + (size_t)pVert1->pHgt) >> 1);
	pVert->fHgtAvg = (float)(*pVert0->pHgt + *pVert1->pHgt);
	pVert->fHgtAdd = (float)(*pVert->pHgt * 2) - pVert->fHgtAvg;
	pVert->fHgtAvg += pPatch->z0;
	pVert->xyz[0] = (pVert0->xyz[0] + pVert1->xyz[0]) * 0.5f;
	pVert->xyz[1] = (pVert0->xyz[1] + pVert1->xyz[1]) * 0.5f;
	pVert->xyz[2] = pVert->fHgtAvg;
}

static void R_ReleaseVert(cTerraPatchUnpacked_t* patch, int iVert)
{
	terrainVert_t* pVert = &g_pVert[iVert];

	terraInt iPrev = pVert->iPrev;
	terraInt iNext = pVert->iNext;
	g_pVert[iPrev].iNext = iNext;
	g_pVert[iNext].iPrev = iPrev;

	assert(patch->drawinfo.nVerts > 0);
	patch->drawinfo.nVerts--;

	if (patch->drawinfo.iVertHead == iVert)
	{
		patch->drawinfo.iVertHead = iNext;
	}

	pVert->iPrev = 0;
	pVert->iNext = g_vert.iFreeHead;
	g_pVert[g_vert.iFreeHead].iPrev = iVert;
	g_vert.iFreeHead = iVert;
	g_vert.nFree++;
}

terraInt R_AllocateTri(cTerraPatchUnpacked_t* patch, qboolean check)
{
	terraInt iTri = g_tri.iFreeHead;

	g_tri.iFreeHead = g_pTris[iTri].iNext;
	g_pTris[g_tri.iFreeHead].iPrev = 0;

	g_pTris[iTri].iPrev = patch->drawinfo.iTriTail;
	g_pTris[iTri].iNext = 0;
	g_pTris[patch->drawinfo.iTriTail].iNext = iTri;
	patch->drawinfo.iTriTail = iTri;

	if (!patch->drawinfo.iTriHead)
	{
		patch->drawinfo.iTriHead = iTri;
	}

	patch->drawinfo.nTris++;
	g_tri.nFree--;

	g_pTris[iTri].byConstChecks = ~((-check | check) >> 31) & 4;
	g_pTris[iTri].uiDistRecalc = 0;
	patch->uiDistRecalc = 0;

	return iTri;
}

static void R_ReleaseTri(cTerraPatchUnpacked_t* patch, int iTri)
{
	terraTri_t* pTri = &g_pTris[iTri];

	terraInt iPrev = pTri->iPrev;
	terraInt iNext = pTri->iNext;
	g_pTris[iPrev].iNext = iNext;
	g_pTris[iNext].iPrev = iPrev;

	if (g_tri.iCur == iTri)
	{
		g_tri.iCur = iNext;
	}

	patch->uiDistRecalc = 0;

	assert(patch->drawinfo.nTris > 0);
	patch->drawinfo.nTris--;

	if (patch->drawinfo.iTriHead == iTri)
	{
		patch->drawinfo.iTriHead = iNext;
	}

	if (patch->drawinfo.iTriTail == iTri)
	{
		patch->drawinfo.iTriTail = iPrev;
	}

	pTri->iPrev = 0;
	pTri->iNext = g_tri.iFreeHead;
	g_pTris[g_tri.iFreeHead].iPrev = iTri;
	g_tri.iFreeHead = iTri;
	g_tri.nFree++;

	for (int i = 0; i < 3; i++)
	{
		terraInt ptNum = pTri->iPt[i];

		g_pVert[ptNum].nRef--;
		if (g_pVert[ptNum].nRef-- == 1)
		{
			R_ReleaseVert(patch, ptNum);
		}
	}
}

void R_FixTriHeight(terraTri_t* pTri)
{
	for (terraInt i = 0; i < 3; i++)
	{
		terrainVert_t* pVert = &g_pVert[pTri->iPt[i]];
		if (pVert->pHgt < pTri->patch->heightmap || pVert->pHgt > &pTri->patch->heightmap[80])
		{
			pVert->pHgt = pTri->patch->heightmap;
		}
	}
}

static int R_ConstChecksForTri(terraTri_t* pTri)
{
	varnodeUnpacked_t vn = *pTri->varnode;

	if (pTri->lod == MAX_TERRAIN_LOD) {
		return 2;
	}

	vn.s.flags &= 0xF0u;

	if (vn.fVariance == 0.0 && !(pTri->varnode->s.flags & 8)) {
		return 2;
	} else if (pTri->varnode->s.flags & 8) {
		return 3;
	} else if ((pTri->byConstChecks & 4) && (pTri->varnode->s.flags & 4) && pTri->lod < ter_maxlod->integer) {
		return 0;
	}

	return 2;
}

static void R_DemoteInAncestry(cTerraPatchUnpacked_t* patch, int iTri)
{
	terraInt iPrev = g_pTris[iTri].iPrev;
	terraInt iNext = g_pTris[iTri].iNext;

	g_pTris[iPrev].iNext = iNext;
	g_pTris[iNext].iPrev = iPrev;

	if (g_tri.iCur == iTri)
	{
		g_tri.iCur = iNext;
	}

	assert(patch->drawinfo.nTris > 0);
	patch->drawinfo.nTris--;

	if (patch->drawinfo.iTriHead == iTri)
	{
		patch->drawinfo.iTriHead = iNext;
	}

	if (patch->drawinfo.iTriTail == iTri)
	{
		patch->drawinfo.iTriTail = iPrev;
	}

	g_pTris[iTri].iPrev = 0;
	g_pTris[iTri].iNext = patch->drawinfo.iMergeHead;
	g_pTris[patch->drawinfo.iMergeHead].iPrev = iTri;
	patch->drawinfo.iMergeHead = iTri;
}

static void R_TerrainHeapInit()
{
	g_tri.iFreeHead = 1;
	g_tri.nFree = g_nTris - 1;
	g_vert.iFreeHead = 1;
	g_vert.nFree = g_nVerts - 1;

	for (size_t i = 0; i < g_nTris; i++)
	{
		g_pTris[i].iPrev = (terraInt)i - 1;
		g_pTris[i].iNext = (terraInt)i + 1;
	}

	g_pTris[0].iPrev = 0;
	g_pTris[g_nTris - 1].iNext = 0;

	for (size_t i = 0; i < g_nVerts; i++)
	{
		g_pVert[i].iPrev = (terraInt)i - 1;
		g_pVert[i].iNext = (terraInt)i + 1;
	}

	g_pVert[0].iPrev = 0;
	g_pVert[g_nVerts - 1].iNext = 0;
}

static void R_TerrainPatchesInit()
{
	int i;

	for (i = 0; i < tr.world->numTerraPatches; i++)
	{
		cTerraPatchUnpacked_t* patch = &tr.world->terraPatches[i];
		patch->drawinfo.iTriHead = 0;
		patch->drawinfo.iTriTail = 0;
		patch->drawinfo.iMergeHead = 0;
		patch->drawinfo.iVertHead = 0;
		patch->drawinfo.nTris = 0;
		patch->drawinfo.nVerts = 0;
	}
}

void R_SplitTri(terraInt iSplit, terraInt iNewPt, terraInt iLeft, terraInt iRight, terraInt iRightOfLeft, terraInt iLeftOfRight)
{
	terraTri_t* pSplit = &g_pTris[iSplit];

	terraTri_t* pLeft;
	if (iRight)
	{
		pLeft = &g_pTris[iLeft];
	}
	else
	{
		pLeft = NULL;
	}

	terraTri_t* pRight;
	if (iRight)
	{
		pRight = &g_pTris[iRight];
	}
	else
	{
		pRight = NULL;
	}

	int iNextLod = pSplit->lod + 1;
	int index = pSplit->index;
	varnodeUnpacked_t* varnode = pSplit->varnode + index;

	if (pLeft)
	{
		pLeft->patch = pSplit->patch;
		pLeft->index = index * 2;
		pLeft->varnode = varnode;
		pLeft->lod = iNextLod;
		pLeft->iLeft = iRight;
		pLeft->iRight = iRightOfLeft;
		pLeft->iBase = pSplit->iLeft;
		pLeft->iPt[0] = pSplit->iPt[1];
		pLeft->iPt[1] = pSplit->iPt[2];
		pLeft->iPt[2] = iNewPt;

		R_FixTriHeight(pLeft);
		pLeft->byConstChecks |= R_ConstChecksForTri(pLeft);

		g_pVert[pLeft->iPt[0]].nRef++;
		g_pVert[pLeft->iPt[1]].nRef++;
		g_pVert[pLeft->iPt[2]].nRef++;
		g_pTris[pSplit->iParent].nSplit++;
		pLeft->nSplit = 0;
	}

	if (pSplit->iLeft)
	{
		if (g_pTris[pSplit->iLeft].lod == iNextLod)
		{
			g_pTris[pSplit->iLeft].iBase = iLeft;
		}
		else
		{
			g_pTris[pSplit->iLeft].iRight = iLeft;
		}
	}

	if (pRight)
	{
		pRight->patch = pSplit->patch;
		pRight->index = index * 2 + 1;
		pRight->varnode = varnode + 1;
		pRight->lod = iNextLod;
		pRight->iLeft = iLeftOfRight;
		pRight->iRight = iLeft;
		pRight->iBase = pSplit->iRight;
		pRight->iPt[0] = pSplit->iPt[2];
		pRight->iPt[1] = pSplit->iPt[0];
		pRight->iPt[2] = iNewPt;

		R_FixTriHeight(pRight);
		pRight->byConstChecks |= R_ConstChecksForTri(pRight);

		g_pVert[pRight->iPt[0]].nRef++;
		g_pVert[pRight->iPt[1]].nRef++;
		g_pVert[pRight->iPt[2]].nRef++;
		g_pTris[pSplit->iParent].nSplit++;
		pRight->nSplit = 0;
	}

	if (pSplit->iRight)
	{
		if (g_pTris[pSplit->iRight].lod == iNextLod)
		{
			g_pTris[pSplit->iRight].iBase = iRight;
		}
		else
		{
			g_pTris[pSplit->iRight].iLeft = iRight;
		}
	}

	pSplit->iLeftChild = iLeft;
	pSplit->iRightChild = iRight;
	g_pTris[iLeft].iParent = iSplit;
	g_pTris[iRight].iParent = iSplit;

	R_DemoteInAncestry(pSplit->patch, iSplit);
}

static void R_ForceSplit(int iTri)
{
	terraTri_t* pTri = &g_pTris[iTri];

	terraInt iBase = pTri->iBase;
	terraTri_t* pBase = &g_pTris[iBase];
	if (iBase && pBase->lod != pTri->lod)
	{
		R_ForceSplit(iBase);
		iBase = pTri->iBase;
		pBase = &g_pTris[iBase];
	}

	uint8_t flags = pTri->varnode->s.flags;

	terraInt iTriLeft = R_AllocateTri(pTri->patch, flags & 2);
	terraInt iTriRight = R_AllocateTri(pTri->patch, flags & 1);

	terraInt iNewPt = R_AllocateVert(pTri->patch);
	R_InterpolateVert(pTri, &g_pVert[iNewPt]);

	g_pVert[iNewPt].fVariance = pTri->varnode->fVariance;

	terraInt iBaseLeft = 0;
	terraInt iBaseRight = 0;

	if (iBase)
	{
		uint8_t flags2 = pBase->varnode->s.flags;
		flags |= flags2;

		iBaseLeft = R_AllocateTri(pBase->patch, flags2 & 2);
		iBaseRight = R_AllocateTri(pBase->patch, flags2 & 1);

		terraInt iNewBasePt = iNewPt;
		if (pBase->patch != pTri->patch)
		{
			iNewBasePt = R_AllocateVert(pBase->patch);
			terrainVert_t* pVert = &g_pVert[iNewBasePt];
			R_InterpolateVert(pBase, pVert);

			pVert->fVariance = g_pVert[iNewPt].fVariance;
			if (flags & 8)
			{
				pVert->fHgtAvg += pVert->fHgtAdd;
				pVert->fHgtAdd = 0.0;
				pVert->fVariance = 0.0;
				pVert->xyz[2] = pVert->fHgtAvg;
			}
		}

		R_SplitTri(iBase, iNewBasePt, iBaseLeft, iBaseRight, iTriRight, iTriLeft);
	}

	if (flags & 8)
	{
		terrainVert_t* pVert = &g_pVert[iNewPt];
		pVert->fHgtAvg += pVert->fHgtAdd;
		pVert->fHgtAdd = 0.0;
		pVert->fVariance = 0.0;
		pVert->xyz[2] = pVert->fHgtAvg;
	}

	R_SplitTri(iTri, iNewPt, iTriLeft, iTriRight, iBaseRight, iBaseLeft);
}

static void R_ForceMerge(int iTri)
{
	terraTri_t* pTri = &g_pTris[iTri];
	cTerraPatchUnpacked_t* patch = pTri->patch;
	terraInt iPrev = pTri->iPrev;
	terraInt iNext = pTri->iNext;

	g_nMerge++;

	if (pTri->iLeftChild)
	{
		terraInt iLeft = g_pTris[pTri->iLeftChild].iBase;

		g_pTris[iTri].iLeft = iLeft;
		if (iLeft)
		{
			if (g_pTris[iLeft].lod == pTri->lod)
			{
				g_pTris[iLeft].iRight = iTri;
			}
			else
			{
				g_pTris[iLeft].iBase = iTri;
			}
		}

		R_ReleaseTri(pTri->patch, pTri->iLeftChild);

		pTri->iLeftChild = 0;
		g_pTris[pTri->iParent].nSplit--;
	}

	if (pTri->iRightChild)
	{
		terraInt iRight = g_pTris[pTri->iRightChild].iBase;
		g_pTris[iTri].iRight = iRight;
		if (iRight)
		{
			if (g_pTris[iRight].lod == pTri->lod)
			{
				g_pTris[iRight].iLeft = iTri;
			}
			else
			{
				g_pTris[iRight].iBase = iTri;
			}
		}

		R_ReleaseTri(pTri->patch, pTri->iRightChild);

		pTri->iRightChild = 0;
		g_pTris[pTri->iParent].nSplit--;
	}

	g_pTris[iPrev].iNext = iNext;
	g_pTris[iNext].iPrev = iPrev;

	if (g_tri.iCur == iTri)
	{
		g_tri.iCur = iNext;
	}

	patch->drawinfo.nTris++;
	if (patch->drawinfo.iMergeHead == iTri)
	{
		patch->drawinfo.iMergeHead = iNext;
	}

	g_pTris[iTri].iPrev = patch->drawinfo.iTriTail;
	g_pTris[iTri].iNext = 0;

	g_pTris[patch->drawinfo.iTriTail].iNext = iTri;

	patch->drawinfo.iTriTail = iTri;
	if (!patch->drawinfo.iTriHead)
	{
		patch->drawinfo.iTriHead = iTri;
	}
}

static int R_TerraTriNeighbor(cTerraPatchUnpacked_t* terraPatches, int iPatch, int dir)
{
	if (iPatch >= 0)
	{
		int iNeighbor = 2 * iPatch + 1;

		if (dir == 1)
		{
			if (terraPatches[iPatch].flags & 0x80)
			{
				return iNeighbor;
			}
			else
			{
				return iNeighbor + 1;
			}
		}
		else if (dir > 1)
		{
			if (dir == 2)
			{
				return 2 * iPatch + 2;
			}
			if (dir == 3)
			{
				if (terraPatches[iPatch].flags & 0x80)
				{
					return iNeighbor + 1;
				}
				else
				{
					return iNeighbor;
				}
			}
		}
		else if (!dir)
		{
			return 2 * iPatch + 1;
		}
	}

	return 0;
}

static void R_PreTessellateTerrain()
{
	size_t numTerrainPatches = tr.world->numTerraPatches;

	if (!numTerrainPatches) {
		return;
	}

	R_SyncRenderThread();

	if (ter_maxtris->integer < 4 * numTerrainPatches) {
		Cvar_SetValue("ter_maxtris", 4 * numTerrainPatches);
	}
	else if (ter_maxtris->integer > 65535) {
		Cvar_SetValue("ter_maxtris", 65535);
	}

	Com_DPrintf("Using ter_maxtris = %d\n", ter_maxtris->integer);

	g_nTris = ter_maxtris->integer * 2 + 1;
	g_nVerts = ter_maxtris->integer + 1;
	g_pTris = ri.Hunk_Alloc(g_nTris * sizeof(terraTri_t));
	g_pVert = ri.Hunk_Alloc(g_nVerts * sizeof(terrainVert_t));

	// Init triangles & vertices
	R_TerrainHeapInit();
	R_TerrainPatchesInit();

	for (size_t i = 0; i < numTerrainPatches; i++)
	{
		cTerraPatchUnpacked_t* patch = &tr.world->terraPatches[i];

		patch->drawinfo.surfaceType = SF_TERRAIN_PATCH;
		patch->drawinfo.nTris = 0;
		patch->drawinfo.nVerts = 0;
		patch->drawinfo.iTriHead = 0;
		patch->drawinfo.iTriTail = 0;
		patch->drawinfo.iVertHead = 0;

		float sMin, tMin;

		if (patch->texCoord[0][0][0] < patch->texCoord[0][1][0])
		{
			sMin = patch->texCoord[0][0][0];
		}
		else
		{
			sMin = patch->texCoord[0][1][0];
		}

		float sMin2;
		if (patch->texCoord[1][0][0] < patch->texCoord[1][1][0])
		{
			sMin2 = patch->texCoord[1][0][0];
		}
		else
		{
			sMin2 = patch->texCoord[1][1][0];
		}

		if (sMin >= sMin2)
		{
			if (patch->texCoord[1][0][0] >= patch->texCoord[1][1][0])
			{
				sMin = floor(patch->texCoord[1][1][0]);
			}
			else
			{
				sMin = floor(patch->texCoord[1][0][0]);
			}
		}
		else if (patch->texCoord[0][0][0] >= patch->texCoord[0][1][0])
		{
			sMin = floor(patch->texCoord[0][1][0]);
		}
		else
		{
			sMin = floor(patch->texCoord[0][0][0]);
		}

		if (patch->texCoord[0][0][1] < patch->texCoord[0][1][1])
		{
			tMin = patch->texCoord[0][0][1];
		}
		else
		{
			tMin = patch->texCoord[0][1][1];
		}

		float tMin2;
		if (patch->texCoord[1][0][1] < patch->texCoord[1][1][1])
		{
			tMin2 = patch->texCoord[1][0][1];
		}
		else
		{
			tMin2 = patch->texCoord[1][1][1];
		}

		if (tMin >= tMin2)
		{
			if (patch->texCoord[1][0][1] >= patch->texCoord[1][1][1])
			{
				tMin = floor(patch->texCoord[1][1][1]);
			}
			else
			{
				tMin = floor(patch->texCoord[1][0][1]);
			}
		}
		else if (patch->texCoord[0][0][1] >= patch->texCoord[0][1][1])
		{
			tMin = floor(patch->texCoord[0][1][1]);
		}
		else
		{
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
		const float ls = patch->s * lmapSize;
		const float lt = patch->t * lmapSize;

		terraInt iTri0 = R_AllocateTri(patch, 0);
		terraInt iTri1 = R_AllocateTri(patch, 0);
		terraInt i00 = R_AllocateVert(patch);
		terraInt i01 = R_AllocateVert(patch);
		terraInt i10 = R_AllocateVert(patch);
		terraInt i11 = R_AllocateVert(patch);

		terrainVert_t* pVert;
		pVert = &g_pVert[i00];
		pVert->xyz[0] = patch->x0;
		pVert->xyz[1] = patch->y0;
		pVert->xyz[2] = (float)(patch->heightmap[0] * 2) + patch->z0;
		pVert->pHgt = &patch->heightmap[0];
		pVert->fHgtAvg = pVert->xyz[2];
		pVert->texCoords[0][0] = s00;
		pVert->texCoords[0][1] = t00;
		pVert->texCoords[1][0] = patch->s;
		pVert->texCoords[1][1] = patch->t;
		pVert->fVariance = 0.0f;
		pVert->nRef = 4;

		pVert = &g_pVert[i01];
		pVert->xyz[0] = patch->x0;
		pVert->xyz[1] = patch->y0 + 512.0f;
		pVert->xyz[2] = (float)(patch->heightmap[72] * 2) + patch->z0;
		pVert->pHgt = &patch->heightmap[72];
		pVert->fHgtAvg = pVert->xyz[2];
		pVert->texCoords[0][0] = s01;
		pVert->texCoords[0][1] = t01;
		pVert->texCoords[1][0] = patch->s;
		pVert->texCoords[1][1] = lt;
		pVert->fVariance = 0.0f;
		pVert->nRef = 4;

		pVert = &g_pVert[i10];
		pVert->xyz[0] = patch->x0 + 512.0f;
		pVert->xyz[1] = patch->y0;
		pVert->xyz[2] = (float)(patch->heightmap[8] * 2) + patch->z0;
		pVert->pHgt = &patch->heightmap[8];
		pVert->fHgtAvg = pVert->xyz[2];
		pVert->texCoords[0][0] = s10;
		pVert->texCoords[0][1] = t10;
		pVert->texCoords[1][0] = ls;
		pVert->texCoords[1][1] = patch->t;
		pVert->fVariance = 0.0f;
		pVert->nRef = 4;

		pVert = &g_pVert[i11];
		pVert->xyz[0] = patch->x0 + 512.0f;
		pVert->xyz[1] = patch->y0 + 512.0f;
		pVert->xyz[2] = (float)(patch->heightmap[80] * 2) + patch->z0;
		pVert->pHgt = &patch->heightmap[80];
		pVert->fHgtAvg = pVert->xyz[2];
		pVert->texCoords[0][0] = s11;
		pVert->texCoords[0][1] = t11;
		pVert->texCoords[1][0] = ls;
		pVert->texCoords[1][1] = lt;
		pVert->fVariance = 0.0f;
		pVert->nRef = 4;

		terraTri_t* pTri = &g_pTris[iTri0];
		pTri->patch = patch;
		pTri->varnode = &patch->varTree[0][0];
		pTri->index = 1;
		pTri->lod = 0;
		pTri->byConstChecks |= R_ConstChecksForTri(pTri);

		pTri->iBase = iTri1;
		if ((patch->flags & 0x80u) == 0)
		{
			pTri->iLeft = R_TerraTriNeighbor(tr.world->terraPatches, patch->iWest, 1);
			pTri->iRight = R_TerraTriNeighbor(tr.world->terraPatches, patch->iNorth, 2);
			if (patch->flags & 0x40)
			{
				pTri->iPt[0] = i00;
				pTri->iPt[1] = i11;
			}
			else
			{
				pTri->iPt[0] = i11;
				pTri->iPt[1] = i00;
			}
			pTri->iPt[2] = i01;
		}
		else
		{
			pTri->iLeft = R_TerraTriNeighbor(tr.world->terraPatches, patch->iNorth, 2);
			pTri->iRight = R_TerraTriNeighbor(tr.world->terraPatches, patch->iEast, 3);
			if (patch->flags & 0x40)
			{
				pTri->iPt[0] = i01;
				pTri->iPt[1] = i10;
			}
			else
			{
				pTri->iPt[0] = i10;
				pTri->iPt[1] = i01;
			}
			pTri->iPt[2] = i11;
		}

		R_FixTriHeight(pTri);

		pTri = &g_pTris[iTri1];
		pTri->patch = patch;
		pTri->varnode = &patch->varTree[1][0];
		pTri->index = 1;
		pTri->lod = 0;
		pTri->byConstChecks |= R_ConstChecksForTri(pTri);

		pTri->iBase = iTri0;
		if ((patch->flags & 0x80u) == 0)
		{
			pTri->iLeft = R_TerraTriNeighbor(tr.world->terraPatches, patch->iEast, 3);
			pTri->iRight = R_TerraTriNeighbor(tr.world->terraPatches, patch->iSouth, 0);
			if (patch->flags & 0x40)
			{
				pTri->iPt[0] = i11;
				pTri->iPt[1] = i00;
			}
			else
			{
				pTri->iPt[0] = i00;
				pTri->iPt[1] = i11;
			}
			pTri->iPt[2] = i10;
		}
		else
		{
			pTri->iLeft = R_TerraTriNeighbor(tr.world->terraPatches, patch->iSouth, 0);
			pTri->iRight = R_TerraTriNeighbor(tr.world->terraPatches, patch->iWest, 1);
			if (patch->flags & 0x40)
			{
				pTri->iPt[0] = i10;
				pTri->iPt[1] = i01;
			}
			else
			{
				pTri->iPt[0] = i01;
				pTri->iPt[1] = i10;
			}
			pTri->iPt[2] = i00;
		}

		R_FixTriHeight(pTri);
	}
}

static qboolean R_NeedSplitTri(terraTri_t * pTri)
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
		+ g_vViewVector[2]
		- *(float *)pTri->varnode * g_fCheck;

	if (fRatio <= 0)
	{
		return qtrue;
	}
	pTri->uiDistRecalc = floor(fRatio) + g_uiTerDist;

	return qtrue;
}

static void R_DoTriSplitting()
{
	cTerraPatchUnpacked_t* patch;

	for (patch = tr.world->activeTerraPatches; patch; patch = patch->pNextActive)
	{
		if (patch->uiDistRecalc > g_uiTerDist) {
			continue;
		}

		patch->uiDistRecalc = -1;
		g_tri.iCur = patch->drawinfo.iTriHead;
		while (g_tri.iCur != 0)
		{
			terraTri_t* pTri = &g_pTris[g_tri.iCur];

			if (R_NeedSplitTri(pTri))
			{
				//
				// make sure there are sufficient number of tris
				//
				if (g_tri.nFree <= 13 || g_vert.nFree <= 13) {
					Com_DPrintf("WARNING: aborting terrain tessellation -- insufficient tris\n");
					return;
				}

				patch->uiDistRecalc = 0;
				R_ForceSplit(g_tri.iCur);

				if (&g_pTris[g_tri.iCur] == pTri)
				{
					g_tri.iCur = g_pTris[g_tri.iCur].iNext;
				}
			}
			else
			{
				if ((pTri->byConstChecks & 3) != 2)
				{
					if (patch->uiDistRecalc > pTri->uiDistRecalc)
					{
						patch->uiDistRecalc = pTri->uiDistRecalc;
					}
				}

				g_tri.iCur = g_pTris[g_tri.iCur].iNext;
			}
		}
	}
}

static void R_DoGeomorphs()
{
	for (size_t n = 0; n < tr.world->numTerraPatches; n++)
	{
		cTerraPatchUnpacked_t* patch = &tr.world->terraPatches[n];

		g_vert.iCur = patch->drawinfo.iVertHead;

		if (patch->visCountDraw == g_terVisCount)
		{
			if (patch->byDirty)
			{
				while (g_vert.iCur)
				{
					terrainVert_t* pVert = &g_pVert[g_vert.iCur];
					float dot;

					pVert->xyz[2] = pVert->fHgtAvg;
					pVert->uiDistRecalc = g_uiTerDist + 1;
					dot = (pVert->fVariance * g_fCheck) - (g_vViewVector[0] * pVert->xyz[0] + g_vViewVector[1] * pVert->xyz[1] + g_vViewVector[2]);

					if (dot > 0.0)
					{
						if (dot / 400.0 < 1.0) {
							pVert->uiDistRecalc = (g_uiTerDist - 400) + (int)dot;
							pVert->xyz[2] += pVert->fHgtAdd * 1.0;
						} else {
							pVert->xyz[2] += pVert->fHgtAdd * (dot / 400.0);
						}
					}
					else
					{
						pVert->uiDistRecalc = g_uiTerDist - (int)dot;
					}

					g_vert.iCur = pVert->iNext;
				}

				patch->byDirty = qfalse;
			}
			else
			{
				while (g_vert.iCur)
				{
					terrainVert_t* pVert = &g_pVert[g_vert.iCur];
					float dot;

					pVert->xyz[2] = pVert->fHgtAvg;
					pVert->uiDistRecalc = g_uiTerDist + 1;
					dot = (pVert->fVariance * g_fCheck) - (g_vViewVector[0] * pVert->xyz[0] + g_vViewVector[1] * pVert->xyz[1] + g_vViewVector[2]);

					if (dot > 0.0)
					{
						if (dot / 400.0 < 1.0) {
							pVert->uiDistRecalc = (g_uiTerDist - 400) + (int)dot;
							pVert->xyz[2] += pVert->fHgtAdd * 1.0;
						}
						else {
							pVert->xyz[2] += pVert->fHgtAdd * (dot / 400.0);
						}
					}
					else
					{
						pVert->uiDistRecalc = g_uiTerDist - (int)dot;
					}

					g_vert.iCur = pVert->iNext;
				}
			}
		}
		else
		{
			if (!patch->byDirty)
			{
				patch->byDirty = qtrue;
				while (g_vert.iCur)
				{
					terrainVert_t* pVert = &g_pVert[g_vert.iCur];
					pVert->xyz[2] = pVert->fHgtAvg + pVert->fHgtAdd;
					g_vert.iCur = pVert->iNext;
				}
			}
		}
	}
}

static qboolean R_MergeInternalAggressive()
{
	terraTri_t* pTri = &g_pTris[g_tri.iCur];
	if (!pTri->nSplit
		&& g_pVert[g_pTris[pTri->iLeftChild].iPt[2]].xyz[2] == g_pVert[g_pTris[pTri->iLeftChild].iPt[2]].fHgtAvg)
	{
		if (!pTri->iBase)
		{
			R_ForceMerge(g_tri.iCur);
			return qtrue;
		}

		if (!g_pTris[pTri->iBase].nSplit)
		{
			R_ForceMerge(pTri->iBase);
			R_ForceMerge(g_tri.iCur);
			return qtrue;
		}
	}

	return qfalse;
}

static qboolean R_MergeInternalCautious()
{
	terraTri_t* pTri;
	terraTri_t* pBase;

	pTri = &g_pTris[g_tri.iCur];

	if (!pTri->nSplit
		&& !(pTri->varnode->s.flags & 8)
		&& g_pVert[g_pTris[pTri->iLeftChild].iPt[2]].xyz[2] == g_pVert[g_pTris[pTri->iLeftChild].iPt[2]].fHgtAvg)
	{
		if (!pTri->iBase)
		{
			R_ForceMerge(g_tri.iCur);
			return qtrue;
		}

		pBase = &g_pTris[pTri->iBase];
		if (!pBase->nSplit && !(pBase->varnode->s.flags & 8))
		{
			R_ForceMerge(pTri->iBase);
			R_ForceMerge(g_tri.iCur);
			return qtrue;
		}
	}

	return qfalse;
}

static void R_DoTriMerging()
{
	int iCautiousFrame = g_terVisCount - ter_cautiousframes->integer;

	for (size_t n = 0; n < tr.world->numTerraPatches; n++)
	{
		const cTerraPatchUnpacked_t* patch = &tr.world->terraPatches[n];

		g_tri.iCur = patch->drawinfo.iMergeHead;
		if (patch->visCountDraw >= iCautiousFrame)
		{
			while (g_tri.iCur)
			{
				if (!R_MergeInternalCautious())
				{
					g_tri.iCur = g_pTris[g_tri.iCur].iNext;
				}
			}
		}
		else if(patch->visCountDraw > iCautiousFrame - 10)
		{
			while (g_tri.iCur)
			{
				if (!R_MergeInternalAggressive())
				{
					g_tri.iCur = g_pTris[g_tri.iCur].iNext;
				}
			}
		}
	}
}

void R_TessellateTerrain()
{
	if (glConfig.smpActive) {
		R_SyncRenderThread();
	}

	R_DoTriSplitting();
	// Morph geometry according to the view
	R_DoGeomorphs();
	// Split vertices
	R_DoTriMerging();
}

void R_TerrainPrepareFrame()
{
	float distance;
	int index;
	float fFov;
	float fCheck;
	float fDistBound;

	if (ter_lock->integer) {
		return;
	}

	++g_terVisCount;
	tr.world->activeTerraPatches = NULL;

	distance = 1.0;
	fFov = tr.refdef.fov_x;
	if (fFov < 1.0) {
		fFov = 1.0;
	}

	fCheck = 1.0 / (tan(fFov / 114) * ter_error->value / 320.0);
	if (g_terVisCount)
	{
		float fFarPlane = tr.viewParms.farplane_distance;
		if (fFarPlane > 4096.0) {
			fFarPlane = 4096.0;
		}

		fDistBound = fabs((fCheck - g_fCheck) * 510.0)
			+ fabs((tr.refdef.vieworg[0] - g_vTerOrg[0]) * g_vTerFwd[0])
			+ fabs((tr.refdef.vieworg[1] - g_vTerOrg[1]) * g_vTerFwd[1])
			+ fabs((tr.refdef.viewaxis[0][0] - g_vTerFwd[0]) * fFarPlane)
			+ fabs((tr.refdef.viewaxis[0][1] - g_vTerFwd[1]) * fFarPlane);

		distance = fDistBound;

		g_uiTerDist = (ceil(fDistBound) + (float)g_uiTerDist);
		if (g_uiTerDist > 0xF0000000)
		{
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
	}
	else
	{
		g_uiTerDist = 0;
	}


	VectorCopy2D(tr.refdef.vieworg, g_vTerOrg);
	VectorCopy(tr.refdef.viewaxis[0], g_vTerFwd);
	g_fCheck = fCheck;

	if (distance != 0.0)
	{
		index = (int)tr.refdef.fov_x;
		g_fClipDotSquared = g_fClipDotSquaredTable[index];
		g_fClipDotProduct = g_fClipDotProductTable[index];
		VectorCopy(tr.refdef.viewaxis[0], g_vClipVector);
		g_vClipOrigin[0] = tr.refdef.vieworg[0] - g_fDistanceTable[index] * tr.refdef.viewaxis[0][0] - 256.0;
		g_vClipOrigin[1] = tr.refdef.vieworg[1] - g_fDistanceTable[index] * tr.refdef.viewaxis[0][1] - 256.0;
		g_vClipOrigin[2] = tr.refdef.vieworg[2] - g_fDistanceTable[index] * tr.refdef.viewaxis[0][2] - 255.0;
		g_vViewVector[0] = tr.refdef.viewaxis[0][0];
		g_vViewVector[1] = tr.refdef.viewaxis[0][1];
		g_vViewVector[2] = -(tr.refdef.viewaxis[0][0] * tr.refdef.vieworg[0] + tr.refdef.viewaxis[0][1] * tr.refdef.vieworg[1]);
		VectorCopy(tr.refdef.vieworg, g_vViewOrigin);

		if (tr.viewParms.farplane_distance > 0) {
			g_fFogDistance = distance + tr.viewParms.farplane_distance + 768.0;
		} else {
			g_fFogDistance = 999999.0;
		}
	}
}

void R_MarkTerrainPatch(cTerraPatchUnpacked_t* pPatch)
{
	if (pPatch->visCountCheck == g_terVisCount) {
		return;
	}

	pPatch->visCountCheck = g_terVisCount;
	if (ter_cull->integer)
	{
		vec3_t v;
		float dot;

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

	pPatch->visCountDraw = g_terVisCount;
	pPatch->pNextActive = tr.world->activeTerraPatches;
	tr.world->activeTerraPatches = pPatch;
}

void R_AddTerrainSurfaces()
{
	int i;
	int dlight;
	cTerraPatchUnpacked_t* patch;

	if (tr.world->numTerraPatches < 0) {
		return;
	}

	if (!ter_lock->integer) {
		R_TessellateTerrain();
	}

	if (ter_count->integer || ter_lock->integer == 2)
	{
		for (i = 0; i < tr.world->numTerraPatches; i++)
		{
			patch = &tr.world->terraPatches[i];

			if (ter_lock->integer == 2 || patch->visCountDraw == g_terVisCount)
			{
				assert(patch->shader);

				dlight = R_CheckDlightTerrain(patch, (1 << (tr.refdef.num_dlights)) - 1);
				R_AddDrawSurf((surfaceType_t*)&patch->drawinfo, patch->shader, dlight);
			}

			if (ter_count->integer && (g_nSplit || g_nMerge))
			{
				if (ter_count->integer == 1 || g_nSplit * 2 != g_nMerge)
				{
					Com_DPrintf(
						"%5d tris / %5d verts / %4d splits / %4d merges\n",
						g_nTris + ~g_tri.nFree,
						g_nVerts + ~g_vert.nFree,
						g_nSplit,
						g_nMerge);
				}

				g_nSplit = 0;
				g_nMerge = 0;
			}
		}
	}
	else
	{
		for (patch = tr.world->activeTerraPatches; patch; patch = patch->pNextActive)
		{
			assert(patch->shader);

			dlight = R_CheckDlightTerrain(patch, (1 << (tr.refdef.num_dlights)) - 1);
			R_AddDrawSurf((surfaceType_t*)&patch->drawinfo, patch->shader, dlight);
		}
	}
}

qboolean R_TerrainHeightForPoly(cTerraPatchUnpacked_t* pPatch, polyVert_t* pVerts, int nVerts)
{
	// FIXME: unimplemented
	return qfalse;
}

void R_TerrainRestart_f(void)
{
	if (tr.world->numTerraPatches < 0) {
		return;
	}

	ri.Free(g_pVert);
	g_pVert = NULL;
	ri.Free(g_pTris);
	g_pTris = NULL;

	R_PreTessellateTerrain();
}

void R_CraterTerrain(vec_t* pos /* 0x8 */, vec_t* dir, float fDepth, float fRadius)
{
	// FIXME: unimplemented
}

void R_TerrainCrater_f()
{
	vec3_t dir;
	VectorNegate(tr.refdef.viewaxis[2], dir);

	R_CraterTerrain(tr.refdef.vieworg, dir, 256.0, 256.0);
	R_TerrainRestart_f();
}

void R_InitTerrain()
{
	int i;

	ter_maxlod = ri.Cvar_Get("ter_maxlod", "3", CVAR_ARCHIVE | CVAR_TERRAIN_LATCH);
	ter_cull = ri.Cvar_Get("ter_cull", "1", 0);
	ter_lock = ri.Cvar_Get("ter_lock", "0", 0);
	ter_error = ri.Cvar_Get("ter_error", "10", CVAR_ARCHIVE);
	ter_cautiousframes = ri.Cvar_Get("ter_cautiousframes", "1", CVAR_ARCHIVE);
	ter_maxtris = ri.Cvar_Get("ter_maxtris", "16384", CVAR_TERRAIN_LATCH);
	ter_count = ri.Cvar_Get("ter_count", "0", 0);

	Cmd_AddCommand("ter_restart", R_TerrainRestart_f);
	R_PreTessellateTerrain();

	for (i = 0; i < TERRAIN_TABLE_SIZE; i++)
	{
		float dot = (1.0 / (0.82 - cos((float)i / 57.29) * 0.18) - 1.0) * (16.0 / 9.0);
		g_fClipDotSquaredTable[i] = dot;
		g_fClipDotProductTable[i] = sqrt(dot);
		g_fDistanceTable[i] = 443.5 / sqrt(1.0 - dot);
	}
}
