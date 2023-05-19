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

// tr_marks_pernanent.c: permanent marks

#include "tr_local.h"

typedef struct lmEditPoly_s {
	srfMarkFragment_t surf;
	shader_t* shader;
	int viewCount;
	polyVert_t verts[8];
	mnode_t* pLeafs[8];
	int iNumLeafs;
} lmEditPoly_t;

typedef struct lmEditMarkDef_s {
	struct lmEditMarkDef_s* pNextMark;
	struct lmEditMarkDef_s* pPrevMark;
	shader_t* markShader;
	vec3_t vPos;
	vec3_t vProjection;
	float fRadius;
	float fHeightScale;
	float fWidthScale;
	float fRotation;
	float color[4];
	qboolean bDoLighting;
	lmEditPoly_t* pMarkEditPolys;
	int iNumEditPolys;
	vec3_t vPathCorners[4];
} lmEditMarkDef_t;

typedef struct lmPoly_s {
	srfMarkFragment_t surf;
	shader_t* shader;
	int viewCount;
} lmPoly_t;

typedef struct {
	qboolean bLevelMarksLoaded;
	char szDCLFilename[MAX_QPATH];
	lmPoly_t* pMarkFragments;
	qboolean bAutoApplySettings;
	lmEditPoly_t* pFreeEditPolys;
	lmEditMarkDef_t activeMarkDefs;
	lmEditMarkDef_t* pFreeMarkDefs;
	lmEditMarkDef_t* pCurrentMark;
	qboolean bPathLayingMode;
	treadMark_t treadMark;
	lmEditMarkDef_t* pTreadMarkStartDecal;
	lmEditMarkDef_t* pTreadMarkEndDecal;
} lmGlobals_t;

typedef struct {
  int ident;
  int version;
  int checksum;
  int iNumDecals;
  int iNumFragments;
} dclHeader_t;

typedef struct {
  char shader[MAX_RES_NAME];
  vec3_t vPos;
  vec3_t vProjection;
  float fRadius;
  float fHeightScale;
  float fWidthScale;
  float fRotation;
  float color[4];
  qboolean bDoLighting;
} dclSavedMarkDef_t;

typedef struct {
  char shader[MAX_RES_NAME];
  int fogIndex;
  int iIndex;
  int iNumVerts;
} dclSavedMarkPoly_t;

cvar_t* dcl_editmode;
cvar_t* dcl_showcurrent;
cvar_t* dcl_autogetinfo;
cvar_t* dcl_shiftstep;
cvar_t* dcl_shader;
cvar_t* dcl_radius;
cvar_t* dcl_heightscale;
cvar_t* dcl_widthscale;
cvar_t* dcl_rotation;
cvar_t* dcl_r;
cvar_t* dcl_g;
cvar_t* dcl_b;
cvar_t* dcl_alpha;
cvar_t* dcl_dolighting;
cvar_t* dcl_doworld;
cvar_t* dcl_doterrain;
cvar_t* dcl_dobmodels;
cvar_t* dcl_dostring;
cvar_t* dcl_pathmode;
cvar_t* dcl_maxsegment;
cvar_t* dcl_minsegment;
cvar_t* dcl_maxoffset;
cvar_t* dcl_texturescale;
lmGlobals_t lm;
vec3_t vec_upwards;

void DCLC_Save(void)
{
	// FIXME: unimplemented
}

void R_LevelMarksLoad(const char* szBSPName)
{
    // FIXME: unimplemented
}

void R_RemoveEditPolyFromPointerList(lmEditPoly_t* pPoly, void** pFragmentList, int* piNumFragments)
{
	// FIXME: unimplemented
}

void R_FreeMarkFragments(lmEditMarkDef_t* pMark)
{
	// FIXME: unimplemented
}

void R_InitMarkDef(lmEditMarkDef_t* pMark)
{
	// FIXME: unimplemented
}

lmEditMarkDef_t* R_AllocateMarkDef(void)
{
	// FIXME: unimplemented
	return NULL;
}

lmEditPoly_t* R_AllocateEditPoly(void)
{
	// FIXME: unimplemented
	return NULL;
}

qboolean R_AddEditPolyToPointerList(lmEditPoly_t* pPoly, void*** pFragmentList, int* piNumFragments)
{
	// FIXME: unimplemented
	return qfalse;
}

lmEditMarkDef_t* R_ApplyLevelDecal(const vec3_t vPos, const vec3_t vDir, qboolean bUseCurrent, qboolean bUseCurrentSettings, int iPathIndex)
{
	// FIXME: unimplemented
	return NULL;
}

void DCLC_Make(void)
{
	// FIXME: unimplemented
}

void DCLC_Reapply(void)
{
	// FIXME: unimplemented
}

void DCLC_Delete(void)
{
	// FIXME: unimplemented
}

void DCLC_SelectNext(void)
{
	// FIXME: unimplemented
}

void DCLC_SelectPrev(void)
{
	// FIXME: unimplemented
}

void DCLC_SelectCh(void)
{
	// FIXME: unimplemented
}

void DCLC_SelectChNxt(void)
{
	// FIXME: unimplemented
}

void DCLC_GetInfo(void)
{
	// FIXME: unimplemented
}

void DCLC_ShiftUp(void)
{
	// FIXME: unimplemented
}

void DCLC_ShiftDown(void)
{
	// FIXME: unimplemented
}

void DCLC_ShiftLeft(void)
{
	// FIXME: unimplemented
}

void DCLC_ShiftRight(void)
{
	// FIXME: unimplemented
}

void DCLC_RandomRoll(void)
{
	// FIXME: unimplemented
}

void R_LevelMarksInit()
{
	memset(&lm, 0, sizeof(lm));

	dcl_editmode = ri.Cvar_Get("dcl_editmode", "0", CVAR_LATCH);
	dcl_showcurrent = ri.Cvar_Get("dcl_showcurrent", "1", 0);
	dcl_autogetinfo = ri.Cvar_Get("dcl_autogetinfo", "1", 0);
	dcl_shiftstep = ri.Cvar_Get("dcl_shiftstep", "4", 0);
	dcl_shader = ri.Cvar_Get("dcl_shader", "blastmark", 0);
	dcl_radius = ri.Cvar_Get("dcl_radius", "16", 0);
	dcl_heightscale = ri.Cvar_Get("dcl_heightscale", "1", 0);
	dcl_widthscale = ri.Cvar_Get("dcl_widthscale", "1", 0);
	dcl_rotation = ri.Cvar_Get("dcl_rotation", "0", 0);
	dcl_r = ri.Cvar_Get("dcl_r", "1", 0);
	dcl_g = ri.Cvar_Get("dcl_g", "1", 0);
	dcl_b = ri.Cvar_Get("dcl_b", "1", 0);
	dcl_alpha = ri.Cvar_Get("dcl_alpha", "1", 0);
	dcl_dolighting = ri.Cvar_Get("dcl_dolighting", "1", 0);
	dcl_doworld = ri.Cvar_Get("dcl_doworld", "1", 0);
	dcl_doterrain = ri.Cvar_Get("dcl_doterrain", "1", 0);
	dcl_dobmodels = ri.Cvar_Get("dcl_dobmodels", "1", 0);
	dcl_dostring = ri.Cvar_Get("dcl_dostring", "apply to all", 0);
	dcl_pathmode = ri.Cvar_Get("dcl_pathmode", "0", 0);
	dcl_maxsegment = ri.Cvar_Get("dcl_maxsegment", "512", 0);
	dcl_minsegment = ri.Cvar_Get("dcl_minsegment", "24", 0);
	dcl_maxoffset = ri.Cvar_Get("dcl_maxoffset", "10", 0);
	dcl_texturescale = ri.Cvar_Get("dcl_texturescale", "32", 0);

	if (dcl_editmode->integer)
	{
		ri.Cmd_AddCommand("dclc_make", DCLC_Make);
		ri.Cmd_AddCommand("dclc_reapply", DCLC_Reapply);
		ri.Cmd_AddCommand("dclc_delete", DCLC_Delete);
		ri.Cmd_AddCommand("dclc_selectnext", DCLC_SelectNext);
		ri.Cmd_AddCommand("dclc_selectprev", DCLC_SelectPrev);
		ri.Cmd_AddCommand("dclc_selectch", DCLC_SelectCh);
		ri.Cmd_AddCommand("dclc_selectchnxt", DCLC_SelectChNxt);
		ri.Cmd_AddCommand("dclc_getinfo", DCLC_GetInfo);
		ri.Cmd_AddCommand("dclc_shiftup", DCLC_ShiftUp);
		ri.Cmd_AddCommand("dclc_shiftdown", DCLC_ShiftDown);
		ri.Cmd_AddCommand("dclc_shiftleft", DCLC_ShiftLeft);
		ri.Cmd_AddCommand("dclc_shiftright", DCLC_ShiftRight);
		ri.Cmd_AddCommand("dclc_randomroll", DCLC_RandomRoll);
		ri.Cmd_AddCommand("dclc_save", DCLC_Save);

		lm.activeMarkDefs.pNextMark = &lm.activeMarkDefs;
		lm.activeMarkDefs.pPrevMark = &lm.activeMarkDefs;
	}
}

void R_LevelMarksFree()
{
	int i;

	if (!dcl_editmode->integer) {
		return;
	}

	ri.Cmd_RemoveCommand("dclc_make");
	ri.Cmd_RemoveCommand("dclc_reapply");
	ri.Cmd_RemoveCommand("dclc_delete");
	ri.Cmd_RemoveCommand("dclc_selectnext");
	ri.Cmd_RemoveCommand("dclc_selectprev");
	ri.Cmd_RemoveCommand("dclc_selectch");
	ri.Cmd_RemoveCommand("dclc_selectchnxt");
	ri.Cmd_RemoveCommand("dclc_getinfo");
	ri.Cmd_RemoveCommand("dclc_shiftup");
	ri.Cmd_RemoveCommand("dclc_shiftdown");
	ri.Cmd_RemoveCommand("dclc_shiftleft");
	ri.Cmd_RemoveCommand("dclc_shiftright");
	ri.Cmd_RemoveCommand("dclc_randomroll");
	ri.Cmd_RemoveCommand("dclc_save");

	for (i = 0; i < tr.world->numBmodels; i++)
	{
		bmodel_t* pBmodel = &tr.world->bmodels[i];
		if (pBmodel->iNumMarkFragment && pBmodel->pFirstMarkFragment)
		{
			ri.Free(pBmodel->pFirstMarkFragment);
			pBmodel->pFirstMarkFragment = NULL;
			pBmodel->iNumMarkFragment = 0;
		}
	}

	for (i = tr.world->numDecisionNodes; i < tr.world->numnodes; i++)
	{
		mnode_t* pLeaf = &tr.world->nodes[i];

		if (pLeaf->iNumMarkFragment)
		{
			if (pLeaf->pFirstMarkFragment)
			{
				ri.Free(pLeaf->pFirstMarkFragment);
				pLeaf->pFirstMarkFragment = NULL;
				pLeaf->iNumMarkFragment = 0;
			}
		}
	}
}

void R_UpdateLevelMarksSystem()
{
    // FIXME: unimplemented
}
