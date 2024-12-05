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

// tr_marks_pernanent.c: permanent marks

#include "tr_local.h"

#define DCL_FILE_SIGNATURE   *(int *)"DCL "
#define DCL_FILE_OLD_VERSION 1
#define DCL_FILE_VERSION     2

typedef struct lmEditPoly_s {
    srfMarkFragment_t    surf;
    shader_t            *shader;
    int                  viewCount;
    polyVert_t           verts[8];
    struct lmEditPoly_s *pNextPoly;
    mnode_t             *pLeafs[8];
    int                  iNumLeafs;
} lmEditPoly_t;

typedef struct lmEditMarkDef_s {
    struct lmEditMarkDef_s *pNextMark;
    struct lmEditMarkDef_s *pPrevMark;
    shader_t               *markShader;
    vec3_t                  vPos;
    vec3_t                  vProjection;
    float                   fRadius;
    float                   fHeightScale;
    float                   fWidthScale;
    float                   fRotation;
    vec4_t                  color;
    qboolean                bDoLighting;
    lmEditPoly_t           *pMarkEditPolys;
    int                     iNumEditPolys;
    vec3_t                  vPathCorners[4];
} lmEditMarkDef_t;

typedef struct lmPoly_s {
    srfMarkFragment_t surf;
    shader_t         *shader;
    int               viewCount;
} lmPoly_t;

typedef struct {
    qboolean         bLevelMarksLoaded;
    char             szDCLFilename[MAX_QPATH];
    lmPoly_t        *pMarkFragments;
    qboolean         bAutoApplySettings;
    lmEditPoly_t    *pFreeEditPolys;
    lmEditMarkDef_t  activeMarkDefs;
    lmEditMarkDef_t *pFreeMarkDefs;
    lmEditMarkDef_t *pCurrentMark;
    qboolean         bPathLayingMode;
    treadMark_t      treadMark;
    lmEditMarkDef_t *pTreadMarkStartDecal;
    lmEditMarkDef_t *pTreadMarkEndDecal;
} lmGlobals_t;

typedef struct {
    int ident;
    int version;
    int checksum;
    int iNumDecals;
    int iNumFragments;
} dclHeader_t;

typedef struct {
    char     shader[MAX_RES_NAME];
    vec3_t   vPos;
    vec3_t   vProjection;
    float    fRadius;
    float    fHeightScale;
    float    fWidthScale;
    float    fRotation;
    vec4_t   color;
    qboolean bDoLighting;
} dclSavedMarkDef_t;

typedef struct {
    char shader[MAX_RES_NAME];
    int  fogIndex;
    int  iIndex;
    int  iNumVerts;
} dclSavedMarkPoly_t;

cvar_t *dcl_editmode;
cvar_t *dcl_showcurrent;
cvar_t *dcl_autogetinfo;
cvar_t *dcl_shiftstep;
cvar_t *dcl_shader;
cvar_t *dcl_radius;
cvar_t *dcl_heightscale;
cvar_t *dcl_widthscale;
cvar_t *dcl_rotation;
cvar_t *dcl_r;
cvar_t *dcl_g;
cvar_t *dcl_b;
cvar_t *dcl_alpha;
cvar_t *dcl_dolighting;
cvar_t *dcl_doworld;
cvar_t *dcl_doterrain;
cvar_t *dcl_dobmodels;
cvar_t *dcl_dostring;
cvar_t *dcl_pathmode;
cvar_t *dcl_maxsegment;
cvar_t *dcl_minsegment;
cvar_t *dcl_maxoffset;
cvar_t *dcl_texturescale;

static lmGlobals_t lm;

vec3_t vec_upwards;

lmEditMarkDef_t *R_AllocateMarkDef(void);
lmEditMarkDef_t *R_ApplyLevelDecal(
    const vec3_t vPos, const vec3_t vDir, qboolean bUseCurrent, qboolean bUseCurrentSettings, int iPathIndex
);
void DCLC_Delete(void);
void DCLC_GetInfo(void);

/*
=============
DCLC_Save

Save level marks into a DCL file
=============
*/
void DCLC_Save(void)
{
    fileHandle_t       hFile;
    dclHeader_t        header;
    int                i, j;
    int                iNumDecals, iNumFragments;
    lmEditPoly_t      *pPoly;
    lmEditMarkDef_t   *pMark;
    bmodel_t          *pBmodel;
    mnode_t           *pLeaf;
    dclSavedMarkDef_t  saveMark;
    dclSavedMarkPoly_t savePoly;
    char               map_time[32];
    int                littleValue;

    hFile = ri.FS_OpenFileWrite(lm.szDCLFilename);
    if (!hFile) {
        ri.Printf(PRINT_ALL, "R_SaveDCLFile: couldn't write to %s\n", lm.szDCLFilename);
    }

    pMark         = lm.activeMarkDefs.pNextMark;
    iNumDecals    = 0;
    iNumFragments = 0;

    for (pMark = lm.activeMarkDefs.pNextMark; pMark != &lm.activeMarkDefs; pMark = pMark->pNextMark, iNumDecals++) {
        for (pPoly = pMark->pMarkEditPolys; pPoly; iNumFragments++) {
            pPoly->viewCount = iNumFragments;
            pPoly            = pPoly->pNextPoly;
        }
    }

    header.ident         = DCL_FILE_SIGNATURE;
    header.version       = LittleLong(DCL_FILE_VERSION);
    header.checksum      = 0;
    header.iNumDecals    = LittleLong(iNumDecals);
    header.iNumFragments = LittleLong(iNumFragments);

    ri.FS_Write(&header, sizeof(header), hFile);

    //
    // Write the map time
    //
    memset(map_time, 0, sizeof(map_time));
    Q_strncpyz(map_time, ri.CM_MapTime(), sizeof(map_time));
    ri.FS_Write(map_time, sizeof(map_time), hFile);

    if (!iNumDecals) {
        //
        // Nothing to write
        //
        ri.FS_CloseFile(hFile);
        return;
    }

    for (pMark = lm.activeMarkDefs.pNextMark; pMark != &lm.activeMarkDefs; pMark = pMark->pNextMark) {
        Q_strncpyz(saveMark.shader, pMark->markShader->name, sizeof(saveMark.shader));

        saveMark.vPos[0]        = LittleFloat(pMark->vPos[0]);
        saveMark.vPos[1]        = LittleFloat(pMark->vPos[1]);
        saveMark.vPos[2]        = LittleFloat(pMark->vPos[2]);
        saveMark.vProjection[0] = LittleFloat(pMark->vProjection[0]);
        saveMark.vProjection[1] = LittleFloat(pMark->vProjection[1]);
        saveMark.vProjection[2] = LittleFloat(pMark->vProjection[2]);

        saveMark.fRadius      = LittleFloat(pMark->fRadius);
        saveMark.fHeightScale = LittleFloat(pMark->fHeightScale);
        saveMark.fWidthScale  = LittleFloat(pMark->fWidthScale);
        saveMark.fRotation    = LittleFloat(pMark->fRotation);

        saveMark.color[0] = LittleFloat(pMark->color[0]);
        saveMark.color[1] = LittleFloat(pMark->color[1]);
        saveMark.color[2] = LittleFloat(pMark->color[2]);
        saveMark.color[3] = LittleFloat(pMark->color[3]);

        saveMark.bDoLighting = LittleLong(pMark->bDoLighting);

        ri.FS_Write(&saveMark, sizeof(saveMark), hFile);
    }

    for (pMark = lm.activeMarkDefs.pNextMark; pMark != &lm.activeMarkDefs; pMark = pMark->pNextMark) {
        for (pPoly = pMark->pMarkEditPolys; pPoly; pPoly = pPoly->pNextPoly) {
            Q_strncpyz(savePoly.shader, pMark->markShader->name, sizeof(savePoly.shader));

            savePoly.fogIndex  = 0;
            savePoly.iIndex    = LittleLong(pPoly->surf.iIndex);
            savePoly.iNumVerts = LittleLong(pPoly->surf.numVerts);

            ri.FS_Write(&savePoly, sizeof(savePoly), hFile);
            ri.FS_Write(pPoly->verts, sizeof(pPoly->verts[0]) * savePoly.iNumVerts, hFile);
        }
    }

    for (i = 0; i < tr.world->numBmodels; i++) {
        pBmodel = &tr.world->bmodels[i];

        if (!pBmodel->iNumMarkFragment) {
            //
            // No fragment to save
            //
            littleValue = 0;
            ri.FS_Write(&littleValue, sizeof(littleValue), hFile);
            continue;
        }

        littleValue = LittleLong(pBmodel->iNumMarkFragment);
        ri.FS_Write(&littleValue, sizeof(int), hFile);

        for (j = 0; j < pBmodel->iNumMarkFragment; i++) {
            littleValue = ((lmPoly_t *)pBmodel->pFirstMarkFragment[j])->viewCount;
            ri.FS_Write(&littleValue, sizeof(int), hFile);
        }
    }

    for (i = tr.world->numDecisionNodes; i < tr.world->numnodes; i++) {
        pLeaf = &tr.world->nodes[i];

        if (!pLeaf->iNumMarkFragment) {
            //
            // No fragment to save
            //
            littleValue = 0;
            ri.FS_Write(&littleValue, sizeof(littleValue), hFile);
            continue;
        }

        littleValue = LittleLong(pLeaf->iNumMarkFragment);
        ri.FS_Write(&littleValue, sizeof(int), hFile);

        for (j = 0; j < pLeaf->iNumMarkFragment; j++) {
            littleValue = ((lmPoly_t *)pLeaf->pFirstMarkFragment[j])->viewCount;
            ri.FS_Write(&littleValue, sizeof(int), hFile);
        }
    }

    ri.FS_CloseFile(hFile);
}

/*
=============
R_LevelMarksLoad

Load level marks from a DCL file
=============
*/
void R_LevelMarksLoad(const char *szBSPName)
{
    int          i;
    int          iLength;
    fileHandle_t hFile;
    dclHeader_t  dclHeader;
    const char  *real_map_time;
    char         map_time[33];

    if (lm.bLevelMarksLoaded) {
        //
        // Destroy and reallocate level marks
        //

        R_LevelMarksFree();
        R_LevelMarksInit();
    }

    COM_StripExtension(szBSPName, lm.szDCLFilename, sizeof(lm.szDCLFilename));
    Q_strcat(lm.szDCLFilename, sizeof(lm.szDCLFilename), ".dcl");

    if (ri.FS_OpenFile(lm.szDCLFilename, &hFile, qtrue, qtrue) <= 0) {
        ri.Printf(PRINT_ALL, "R_LevelMarksLoad: %s not found\n", lm.szDCLFilename);
        return;
    }

    lm.bLevelMarksLoaded = qtrue;

    //
    // Read the DCL file header
    //

    iLength = ri.FS_Read(&dclHeader, sizeof(dclHeader), hFile);
    if (iLength < sizeof(dclHeader)) {
        ri.FS_CloseFile(hFile);
        ri.Printf(PRINT_ALL, "------ Finished loading DCL file %s -----\n", lm.szDCLFilename);
        return;
    }

    //
    // Signature and version checks
    //

    if (dclHeader.ident != DCL_FILE_SIGNATURE) {
        ri.Printf(PRINT_ALL, "R_LevelMarksLoad: %s Seems to be an invalid DCL file\n", lm.szDCLFilename);
        ri.FS_CloseFile(hFile);
        return;
    }

    for (i = 0; i < sizeof(dclHeader) / sizeof(int); i++) {
        ((int *)&dclHeader)[i] = LittleLong(((int *)&dclHeader)[i]);
    }

    if (dclHeader.version != DCL_FILE_OLD_VERSION && dclHeader.version != DCL_FILE_VERSION) {
        ri.Printf(
            PRINT_ALL,
            "R_LevelMarksLoad: %s is wrong version (%i should be %i)\n",
            lm.szDCLFilename,
            dclHeader.version,
            DCL_FILE_VERSION
        );
        ri.FS_CloseFile(hFile);
        return;
    }

    if (dclHeader.version == DCL_FILE_OLD_VERSION) {
        //
        // The old version has no map time
        //
        map_time[0] = 0;
    } else {
        map_time[32] = 0;
        ri.FS_Read(map_time, sizeof(map_time) - 1, hFile);
    }

    ri.Printf(PRINT_ALL, "----------- Loading DCL file %s ---------\n", lm.szDCLFilename);

    if (dcl_editmode->integer) {
        dclSavedMarkDef_t saveMark;
        lmEditMarkDef_t  *pMark;
        qhandle_t         hShader;

        //
        // Load all editor-mode decals
        //

        for (i = 0; i < dclHeader.iNumDecals; i++) {
            ri.FS_Read(&saveMark, sizeof(saveMark), hFile);

            saveMark.vPos[0]        = LittleFloat(saveMark.vPos[0]);
            saveMark.vPos[1]        = LittleFloat(saveMark.vPos[1]);
            saveMark.vPos[2]        = LittleFloat(saveMark.vPos[2]);
            saveMark.vProjection[0] = LittleFloat(saveMark.vProjection[0]);
            saveMark.vProjection[1] = LittleFloat(saveMark.vProjection[1]);
            saveMark.vProjection[2] = LittleFloat(saveMark.vProjection[2]);

            saveMark.fRadius      = LittleFloat(saveMark.fRadius);
            saveMark.fHeightScale = LittleFloat(saveMark.fHeightScale);
            saveMark.fWidthScale  = LittleFloat(saveMark.fWidthScale);
            saveMark.fRotation    = LittleFloat(saveMark.fRotation);

            saveMark.color[0] = LittleFloat(saveMark.color[0]);
            saveMark.color[1] = LittleFloat(saveMark.color[1]);
            saveMark.color[2] = LittleFloat(saveMark.color[2]);
            saveMark.color[3] = LittleFloat(saveMark.color[3]);

            saveMark.bDoLighting = LittleLong(saveMark.bDoLighting);

            pMark = R_AllocateMarkDef();
            if (!pMark) {
                break;
            }

            hShader = RE_RegisterShader(saveMark.shader);
            if (hShader) {
                pMark->markShader = R_GetShaderByHandle(hShader);
            } else {
                ri.Printf(PRINT_ALL, "Bad decal shader %s\n", saveMark.shader);
                pMark->markShader = tr.defaultShader;
            }

            //
            // Copy settings from the saved mark
            // to the newly allocated mark def
            //

            VectorCopy(saveMark.vPos, pMark->vPos);
            VectorCopy(saveMark.vProjection, pMark->vProjection);

            pMark->fRadius      = saveMark.fRadius;
            pMark->fWidthScale  = saveMark.fWidthScale;
            pMark->fHeightScale = saveMark.fHeightScale;
            pMark->fRotation    = saveMark.fRotation;

            Vector4Copy(saveMark.color, pMark->color);
            pMark->bDoLighting = saveMark.bDoLighting;

            lm.pCurrentMark = pMark;

            //
            // Now add the loaded mark
            //

            if (!R_ApplyLevelDecal(pMark->vPos, pMark->vProjection, qtrue, qtrue, 0)) {
                DCLC_Delete();
            }
        }
    } else {
        int                j;
        int                iIndex;
        dclSavedMarkPoly_t savePoly;
        lmPoly_t          *pPoly;
        shader_t          *shader;
        qhandle_t          hShader;
        char               szLastShaderName[MAX_RES_NAME];
        bmodel_t          *pBmodel;
        mnode_t           *pLeaf;

        szLastShaderName[0] = 0;
        real_map_time       = ri.CM_MapTime();

        if (!*real_map_time || strcmp(real_map_time, map_time)) {
            ri.Printf(PRINT_ALL, "!!! BSP file is newer than DCL file.\n");
            ri.Printf(PRINT_ALL, "!!! The DCL file needs to be updated.\n");
            ri.FS_CloseFile(hFile);
            ri.Printf(PRINT_ALL, "------ Finished loading DCL file %s -----\n", lm.szDCLFilename);
            return;
        }

        if (!dclHeader.iNumDecals) {
            //
            // Nothing to load
            //
            ri.FS_CloseFile(hFile);
            ri.Printf(PRINT_ALL, "------ Finished loading DCL file %s -----\n", lm.szDCLFilename);
            return;
        }

        //
        // Skip editor stuff
        //
        ri.FS_Seek(hFile, sizeof(dclSavedMarkDef_t) * dclHeader.iNumDecals, FS_SEEK_CUR);

        //
        // Load all saved polys
        //

        lm.pMarkFragments = (lmPoly_t *)ri.Hunk_Alloc(sizeof(lmPoly_t) * dclHeader.iNumFragments, h_low);

        for (i = 0; i < dclHeader.iNumFragments; i++) {
            polyVert_t *pv;

            ri.FS_Read(&savePoly, sizeof(savePoly), hFile);

            savePoly.fogIndex  = LittleLong(savePoly.fogIndex);
            savePoly.iIndex    = LittleLong(savePoly.iIndex);
            savePoly.iNumVerts = LittleLong(savePoly.iNumVerts);

            pPoly = &lm.pMarkFragments[i];

            //
            // Make sure to register the shader once
            //
            if (Q_stricmp(savePoly.shader, szLastShaderName)) {
                hShader = RE_RegisterShader(savePoly.shader);
                if (!hShader) {
                    // Added in 2.30
                    //  Don't mess with decal loading, so drop immediately
                    Com_Error(ERR_DROP, "Decal file lists non-existant shader: %s", savePoly.shader);

                    pPoly->shader           = tr.defaultShader;
                    pPoly->surf.iIndex      = 0;
                    pPoly->surf.surfaceType = SF_SKIP;
                    pPoly->surf.numVerts    = 0;
                    continue;
                }

                shader = R_GetShaderByHandle(hShader);
                Q_strncpyz(szLastShaderName, savePoly.shader, sizeof(szLastShaderName));
            }

            pPoly->shader           = shader;
            pPoly->surf.iIndex      = savePoly.iIndex;
            pPoly->surf.surfaceType = SF_MARK_FRAG;

            //
            // Load vertices
            //

            pPoly->surf.numVerts = savePoly.iNumVerts;
            pPoly->surf.verts    = (polyVert_t *)ri.Hunk_Alloc(sizeof(polyVert_t) * savePoly.iNumVerts, h_low);
            ri.FS_Read(pPoly->surf.verts, sizeof(polyVert_t) * savePoly.iNumVerts, hFile);

            for (j = 0; j < savePoly.iNumVerts; j++) {
                pv = &pPoly->surf.verts[j];

                pv->xyz[0] = LittleFloat(pv->xyz[0]);
                pv->xyz[1] = LittleFloat(pv->xyz[1]);
                pv->xyz[2] = LittleFloat(pv->xyz[2]);
                pv->st[0]  = LittleFloat(pv->st[0]);
                pv->st[1]  = LittleFloat(pv->st[1]);
            }
        }

        //
        // Load all brush models
        //
        for (i = 0; i < tr.world->numBmodels; i++) {
            pBmodel = &tr.world->bmodels[i];

            ri.FS_Read(&pBmodel->iNumMarkFragment, sizeof(int), hFile);
            pBmodel->iNumMarkFragment = LittleLong(pBmodel->iNumMarkFragment);

            //
            // Load brush model fragments
            //
            if (pBmodel->iNumMarkFragment) {
                pBmodel->pFirstMarkFragment = (void **)ri.Hunk_Alloc(sizeof(void *) * pBmodel->iNumMarkFragment, h_low);

                for (j = 0; j < pBmodel->iNumMarkFragment; j++) {
                    ri.FS_Read(&iIndex, sizeof(iIndex), hFile);
                    iIndex = LittleLong(iIndex);

                    pBmodel->pFirstMarkFragment[j] = &lm.pMarkFragments[iIndex];
                }
            }
        }

        //
        // Load all leafs
        //
        for (i = tr.world->numDecisionNodes; i < tr.world->numnodes; i++) {
            pLeaf = &tr.world->nodes[i];

            ri.FS_Read(&pLeaf->iNumMarkFragment, sizeof(int), hFile);
            pLeaf->iNumMarkFragment = LittleLong(pLeaf->iNumMarkFragment);

            if (pLeaf->iNumMarkFragment) {
                pLeaf->pFirstMarkFragment = (void **)ri.Hunk_Alloc(sizeof(void *) * pLeaf->iNumMarkFragment, h_low);

                //
                // Load leaf fragments
                //
                for (j = 0; j < pLeaf->iNumMarkFragment; j++) {
                    ri.FS_Read(&iIndex, sizeof(iIndex), hFile);
                    iIndex = LittleLong(iIndex);

                    pLeaf->pFirstMarkFragment[j] = &lm.pMarkFragments[iIndex];
                }
            }
        }
    }

    ri.FS_CloseFile(hFile);
    ri.Printf(0, "------ Finished loading DCL file %s -----\n", lm.szDCLFilename);
}

/*
=============
R_RemoveEditPolyFromPointerList

=============
*/
void R_RemoveEditPolyFromPointerList(lmEditPoly_t *pPoly, void **pFragmentList, int *piNumFragments)
{
    int i;

    for (i = 0; i < *piNumFragments; i++, pFragmentList++) {
        if (*pFragmentList == pPoly) {
            *pFragmentList = NULL;
        }

        if (!*pFragmentList && i < *piNumFragments - 1) {
            *pFragmentList   = pFragmentList[1];
            pFragmentList[1] = NULL;
        }
    }

    (*piNumFragments)--;
}

/*
=============
R_FreeMarkFragments

Free up all allocated marks
=============
*/
void R_FreeMarkFragments(lmEditMarkDef_t *pMark)
{
    lmEditPoly_t *pPoly;
    int           i;

    while ((pPoly = pMark->pMarkEditPolys)) {
        pMark->pMarkEditPolys = pPoly->pNextPoly;

        if (pPoly->surf.iIndex < 0) {
            //
            // It's a brush model
            //
            bmodel_t *pBmodel;

            pBmodel = &tr.world->bmodels[-pPoly->surf.iIndex];

            R_RemoveEditPolyFromPointerList(pPoly, pBmodel->pFirstMarkFragment, &pBmodel->iNumMarkFragment);
            continue;
        } else {
            //
            // It's a leaf
            //
            mnode_t *pLeaf;

            for (i = 0; i < pPoly->iNumLeafs; i++) {
                pLeaf = pPoly->pLeafs[i];

                R_RemoveEditPolyFromPointerList(pPoly, pLeaf->pFirstMarkFragment, &pLeaf->iNumMarkFragment);
            }
        }

        pPoly->pNextPoly  = lm.pFreeEditPolys;
        lm.pFreeEditPolys = pPoly;
    }

    pMark->iNumEditPolys = 0;
}

/*
=============
R_InitMarkDef

Applies default values to the specified mark definition
=============
*/
void R_InitMarkDef(lmEditMarkDef_t *pMark)
{
    VectorClear(pMark->vPos);
    VectorSet(pMark->vProjection, 0, 0, -1);
    pMark->fRadius        = 8.0;
    pMark->fHeightScale   = 1.0;
    pMark->fWidthScale    = 1.0;
    pMark->fRotation      = 0.0;
    pMark->markShader     = tr.defaultShader;
    pMark->color[0]       = NAN;
    pMark->bDoLighting    = 1;
    pMark->pMarkEditPolys = 0;
    pMark->iNumEditPolys  = 0;
}

/*
=============
R_AllocateMarkDef

Allocates a new mark definition for editing
=============
*/
lmEditMarkDef_t *R_AllocateMarkDef(void)
{
    lmEditMarkDef_t *pNewMark;
    int              i;

    if (!lm.pFreeMarkDefs) {
        lm.pFreeMarkDefs = (lmEditMarkDef_t *)ri.Hunk_Alloc(sizeof(lmEditMarkDef_t) * MAX_MARK_FRAGMENTS, h_low);
        if (!lm.pFreeMarkDefs) {
            return NULL;
        }

        for (i = 0; i < MAX_MARK_FRAGMENTS - 1; i++) {
            lm.pFreeMarkDefs[i].pNextMark = &lm.pFreeMarkDefs[i + 1];
        }
        lm.pFreeMarkDefs[MAX_MARK_FRAGMENTS - 1].pNextMark = NULL;
    }

    pNewMark         = lm.pFreeMarkDefs;
    lm.pFreeMarkDefs = lm.pFreeMarkDefs->pNextMark;

    pNewMark->pNextMark = lm.activeMarkDefs.pNextMark;
    pNewMark->pPrevMark = &lm.activeMarkDefs;

    lm.activeMarkDefs.pNextMark->pPrevMark = pNewMark;
    lm.activeMarkDefs.pNextMark            = pNewMark;

    R_InitMarkDef(pNewMark);
    return pNewMark;
}

/*
=============
R_AllocateEditPoly

Allocates a new mark poly for editing
=============
*/
lmEditPoly_t *R_AllocateEditPoly(void)
{
    lmEditPoly_t *pNewPoly;
    int           i;

    if (!lm.pFreeEditPolys) {
        lm.pFreeEditPolys = (lmEditPoly_t *)ri.Hunk_Alloc(sizeof(lmEditPoly_t) * MAX_MARK_POLYVERTS, h_low);
        if (!lm.pFreeEditPolys) {
            return NULL;
        }

        for (i = 0; i < MAX_MARK_POLYVERTS - 1; i++) {
            lm.pFreeEditPolys[i].pNextPoly = &lm.pFreeEditPolys[i + 1];
        }
        lm.pFreeEditPolys[MAX_MARK_POLYVERTS - 1].pNextPoly = NULL;
    }

    pNewPoly                   = lm.pFreeEditPolys;
    lm.pFreeEditPolys          = lm.pFreeEditPolys->pNextPoly;
    pNewPoly->surf.surfaceType = SF_MARK_FRAG;
    pNewPoly->surf.verts       = pNewPoly->verts;

    return pNewPoly;
}

/*
=============
R_AddEditPolyToPointerList

=============
*/
qboolean R_AddEditPolyToPointerList(lmEditPoly_t *pPoly, void ***pFragmentList, int *piNumFragments)
{
    int i;

    if (!*pFragmentList || (*pFragmentList)[*piNumFragments] == (void *)-1) {
        void **pNewList = (void **)ri.Malloc((*piNumFragments + MAX_MARK_FRAGMENTS) * sizeof(void *));
        if (!pNewList) {
            return 0;
        }

        for (i = 0; i < *piNumFragments + MAX_MARK_FRAGMENTS; i++) {
            pNewList[i] = NULL;
        }
        pNewList[*piNumFragments + MAX_MARK_FRAGMENTS - 1] = (void *)-1;

        if (*pFragmentList) {
            Com_Memcpy(pNewList, *pFragmentList, *piNumFragments * sizeof(void *));
            ri.Free(*pFragmentList);
        }

        *pFragmentList = pNewList;
    }

    (*pFragmentList)[*piNumFragments] = pPoly;
    (*piNumFragments)++;

    return qtrue;
}

/*
=============
R_ApplyLevelDecal

=============
*/
lmEditMarkDef_t *R_ApplyLevelDecal(
    const vec3_t vPos, const vec3_t vDir, qboolean bUseCurrent, qboolean bUseCurrentSettings, int iPathIndex
)
{
    qboolean         bDoLighting;
    qboolean         bDoMerge;
    int              i, j, k, l;
    int              iNumFragments;
    float            fRadius;
    float            fSScale, fTScale;
    float            fR, fG, fB, fA;
    float            fRoll;
    shader_t        *shader;
    qhandle_t        hShader;
    lmEditMarkDef_t *pMark;
    lmEditPoly_t    *pPoly;
    vec3_t           vOrigin;
    vec3_t           vProjection;
    vec3_t           vTmp, vTmp2;
    vec3_t           vNorm, vNorm2;
    vec3_t           bounds[2];
    markFragment_t   markFragments[MAX_MARK_FRAGMENTS];
    markFragment_t  *mf, *mf2;
    polyVert_t       verts[MAX_MARK_POLYVERTS];
    bmodel_t        *pBModel;
    mnode_t         *pNode;
    int              iNumNewVerts, iNumMergeVerts;
    int              iFirstEdgeVert, iFirstEdgeVert2;
    polyVert_t      *pNewVerts[8];
    polyVert_t      *pFragVerts;
    polyVert_t      *pMergeVerts[16];
    float            fDist, fDist2;

    iFirstEdgeVert = iFirstEdgeVert2 = 0;

    if (bUseCurrent && !lm.pCurrentMark) {
        return NULL;
    }

    if (iPathIndex) {
        shader = R_GetShaderByHandle(lm.treadMark.hTreadShader);

        fRadius = lm.treadMark.fWidth;
        fTScale = 1.0;
        fSScale = 1.0;

        fG = 1.0;
        fR = 1.0;
        fA = 1.0;
        fB = 1.0;

        bDoLighting = lm.pCurrentMark->bDoLighting;
        fRoll       = -1.0;
    } else if (bUseCurrent && bUseCurrentSettings) {
        shader = lm.pCurrentMark->markShader;

        fRadius = lm.pCurrentMark->fRadius;
        fSScale = lm.pCurrentMark->fWidthScale;
        fTScale = lm.pCurrentMark->fHeightScale;

        fR = lm.pCurrentMark->color[0];
        fG = lm.pCurrentMark->color[1];
        fB = lm.pCurrentMark->color[2];
        fA = lm.pCurrentMark->color[3];

        bDoLighting = lm.pCurrentMark->bDoLighting;
        fRoll       = lm.pCurrentMark->fRotation;
    } else {
        hShader = RE_RegisterShader(dcl_shader->string);
        if (hShader) {
            shader = R_GetShaderByHandle(hShader);
        } else {
            ri.Printf(PRINT_ALL, "Bad decal shader %s\n", dcl_shader->string);
            shader = tr.defaultShader;
        }

        fRadius = dcl_radius->value;
        fSScale = dcl_widthscale->value;
        fTScale = dcl_heightscale->value;

        fR = Q_clamp_float(dcl_r->value, 0.0, 1.0);
        fG = Q_clamp_float(dcl_g->value, 0.0, 1.0);
        fB = Q_clamp_float(dcl_b->value, 0.0, 1.0);
        fA = Q_clamp_float(dcl_alpha->value, 0.0, 1.0);

        bDoLighting = dcl_dolighting->integer != 0;
        fRoll       = anglemod(dcl_rotation->value);
    }

    if (fRadius < 0.0) {
        fRadius = 0.001;
    }
    if (fSScale < 0.0) {
        fSScale = 0.001;
    }
    if (fTScale < 0.0) {
        fTScale = 0.001;
    }

    VectorCopy(vPos, vOrigin);
    VectorNormalize2(vDir, vProjection);

    if (iPathIndex) {
        iNumFragments =
            ri.CG_PermanentTreadMarkDecal(&lm.treadMark, iPathIndex == 1, bDoLighting, markFragments, verts);
    } else {
        iNumFragments = ri.CG_PermanentMark(
            vOrigin,
            vProjection,
            anglemod(fRoll + 90),
            fSScale * fRadius,
            fTScale * fRadius,
            fR,
            fG,
            fB,
            fA,
            bDoLighting,
            0.5,
            0.5,
            markFragments,
            verts
        );
    }

    if (!iNumFragments) {
        return NULL;
    }

    if (bUseCurrent) {
        pMark = lm.pCurrentMark;
        R_FreeMarkFragments(lm.pCurrentMark);
    } else {
        pMark = R_AllocateMarkDef();
        if (!pMark) {
            return NULL;
        }
    }

    VectorCopy(vOrigin, pMark->vPos);
    VectorCopy(vProjection, pMark->vProjection);

    pMark->markShader   = shader;
    pMark->fRadius      = fRadius;
    pMark->fWidthScale  = fSScale;
    pMark->fHeightScale = fTScale;
    VectorSet4(pMark->color, fR, fG, fB, fA);

    pMark->bDoLighting = bDoLighting;
    pMark->fRotation   = fRoll;

    if (iPathIndex) {
        if (iPathIndex == 1) {
            VectorCopy(lm.treadMark.vStartVerts[1], pMark->vPathCorners[0]);
            VectorCopy(lm.treadMark.vStartVerts[0], pMark->vPathCorners[1]);
            VectorCopy(lm.treadMark.vMidVerts[0], pMark->vPathCorners[2]);
            VectorCopy(lm.treadMark.vMidVerts[1], pMark->vPathCorners[3]);
        } else {
            VectorCopy(lm.treadMark.vMidVerts[1], pMark->vPathCorners[0]);
            VectorCopy(lm.treadMark.vMidVerts[0], pMark->vPathCorners[1]);
            VectorCopy(lm.treadMark.vEndVerts[0], pMark->vPathCorners[2]);
            VectorCopy(lm.treadMark.vEndVerts[1], pMark->vPathCorners[3]);
        }

        VectorClear(vOrigin);

        for (i = 0; i < 4; i++) {
            VectorAdd(vOrigin, pMark->vPathCorners[i], vOrigin);
        }

        vOrigin[0] /= 4.0;
        vOrigin[1] /= 4.0;
        vOrigin[2] = vOrigin[2] / 4.0 + 16.0;
        VectorCopy(vOrigin, pMark->vPos);
    }

    for (i = 0; i < iNumFragments; i++) {
        mf = &markFragments[i];

        if (!mf->numPoints) {
            continue;
        }

        assert(mf->numPoints >= 3);

        iNumNewVerts = mf->numPoints;
        for (j = 0; j < mf->numPoints; ++j) {
            pNewVerts[j] = &verts[mf->firstPoint + j];
        }

        VectorSubtract(pNewVerts[0]->xyz, pNewVerts[1]->xyz, vTmp);
        VectorSubtract(pNewVerts[2]->xyz, pNewVerts[1]->xyz, vTmp2);
        CrossProduct(vTmp, vTmp2, vNorm);
        VectorNormalize(vNorm);

        fDist = DotProduct(vNorm, pNewVerts[0]->xyz);

        if (mf->iIndex <= 0) {
            for (j = i + 1; j < iNumFragments; j++) {
                mf2 = &markFragments[j];

                if (!mf2->numPoints) {
                    continue;
                }
                if (mf2->iIndex > 0 || mf->iIndex == mf2->iIndex) {
                    continue;
                }

                pFragVerts = &verts[mf2->firstPoint];

                if (fabs(DotProduct(vNorm, pFragVerts->xyz) - fDist) > 0.01) {
                    continue;
                }

                VectorSubtract(pFragVerts[0].xyz, pFragVerts[1].xyz, vTmp);
                VectorSubtract(pFragVerts[2].xyz, pFragVerts[1].xyz, vTmp2);
                CrossProduct(vTmp, vTmp2, vNorm2);
                VectorNormalize(vNorm2);

                if (!VectorCompare(vNorm, vNorm2)) {
                    continue;
                }

                bDoMerge = qfalse;
                for (k = 0; k < iNumNewVerts; k++) {
                    for (l = 0; l < mf2->numPoints; l++) {
                        if (VectorCompare(pNewVerts[k]->xyz, pFragVerts[(l + 1) % mf2->numPoints].xyz)
                            && VectorCompare(pNewVerts[(k + 1) % iNumNewVerts]->xyz, pFragVerts[l].xyz)) {
                            bDoMerge        = 1;
                            iFirstEdgeVert  = k;
                            iFirstEdgeVert2 = l;
                            break;
                        }
                    }

                    if (bDoMerge) {
                        break;
                    }
                }

                if (bDoMerge) {
                    iNumMergeVerts = 0;
                    for (k = 1; k < iNumNewVerts; ++k) {
                        pMergeVerts[iNumMergeVerts++] = pNewVerts[(k + iFirstEdgeVert) % iNumNewVerts];
                    }

                    for (k = 1; k < mf2->numPoints; k++) {
                        pMergeVerts[iNumMergeVerts++] = &pFragVerts[(k + iFirstEdgeVert2) % mf2->numPoints];
                    }

                    bDoMerge = qtrue;

                    for (k = 0; k < iNumMergeVerts; k++) {
                        VectorSubtract(pMergeVerts[(k + 1) % iNumMergeVerts]->xyz, pMergeVerts[k]->xyz, vTmp);
                        CrossProduct(vNorm, vTmp, vNorm2);
                        VectorNormalize(vNorm2);

                        for (l = 0; l < iNumMergeVerts; l++) {
                            if (l == k) {
                                break;
                            }

                            if (DotProduct(pMergeVerts[l]->xyz, vNorm)
                                > DotProduct(pMergeVerts[k]->xyz, vNorm2) + 0.01) {
                                bDoMerge = qfalse;
                                break;
                            }
                        }

                        if (!bDoMerge) {
                            break;
                        }
                    }

                    if (bDoMerge) {
                        for (k = 0; k < iNumMergeVerts; k++) {
                            VectorSubtract(pMergeVerts[(k + 1) % iNumMergeVerts]->xyz, pMergeVerts[k]->xyz, vTmp);
                            VectorNormalize(vTmp);
                            VectorSubtract(
                                pMergeVerts[k]->xyz, pMergeVerts[(iNumMergeVerts + k - 1) % iNumMergeVerts]->xyz, vTmp
                            );
                            VectorNormalize(vTmp2);

                            if (DotProduct(vTmp, vTmp2) > 0.999) {
                                for (l = k + 1; l < iNumMergeVerts; l++) {
                                    pMergeVerts[l - 1] = pMergeVerts[l];
                                }
                            }
                        }

                        if (iNumMergeVerts <= 8) {
                            iNumNewVerts = iNumMergeVerts;
                            for (k = 0; k < iNumMergeVerts; k++) {
                                pNewVerts[k] = pMergeVerts[k];
                            }

                            mf2->numPoints = 0;
                        }
                    }
                }
            }
        }

        pPoly = R_AllocateEditPoly();
        if (!pPoly) {
            return pMark;
        }

        pPoly->shader        = shader;
        pPoly->surf.numVerts = iNumNewVerts;

        for (j = 0; j < iNumNewVerts; j++) {
            memcpy(&pPoly->verts[j], pNewVerts[j], sizeof(polyVert_t));
        }

        pPoly->surf.iIndex = mf->iIndex;
        pPoly->iNumLeafs   = 0;

        if (mf->iIndex <= 0 || dcl_doworld->integer && dcl_doterrain->integer) {
            if (mf->iIndex >= 0) {
                if (dcl_doworld->integer) {
                    ClearBounds(bounds[0], bounds[1]);
                    VectorClear(vOrigin);

                    for (j = 0; j < pPoly->surf.numVerts; j++) {
                        VectorAdd(pPoly->verts[j].xyz, vNorm, vTmp);

                        pNode = R_PointInLeaf(vTmp);
                        if (pNode && pNode->contents != -1 && !(pNode->contents & CONTENTS_SOLID)) {
                            for (k = 0; k < pPoly->iNumLeafs; k++) {
                                if (pPoly->pLeafs[k] == pNode) {
                                    break;
                                }
                            }

                            if (k == pPoly->iNumLeafs) {
                                R_AddEditPolyToPointerList(pPoly, &pNode->pFirstMarkFragment, &pNode->iNumMarkFragment);
                                pPoly->pLeafs[pPoly->iNumLeafs] = pNode;
                                pPoly->iNumLeafs++;
                            }
                        }

                        AddPointToBounds(vTmp, bounds[0], bounds[1]);
                        VectorAdd(vOrigin, vTmp, vOrigin);
                    }

                    VectorScale(vOrigin, 1.0 / pPoly->surf.numVerts, vOrigin);

                    pNode = R_PointInLeaf(vOrigin);
                    for (k = 0; k < pPoly->iNumLeafs; k++) {
                        if (pPoly->pLeafs[k] == pNode) {
                            break;
                        }
                    }

                    if (k == pPoly->iNumLeafs) {
                        R_AddEditPolyToPointerList(pPoly, &pNode->pFirstMarkFragment, &pNode->iNumMarkFragment);
                        pPoly->pLeafs[pPoly->iNumLeafs] = pNode;
                        pPoly->iNumLeafs++;
                    }

                    pPoly->pNextPoly      = pMark->pMarkEditPolys;
                    pMark->pMarkEditPolys = pPoly;
                    pMark->iNumEditPolys++;
                }
            } else {
                if (dcl_dobmodels->integer) {
                    R_AddEditPolyToPointerList(
                        pPoly,
                        &tr.world->bmodels[-mf->iIndex].pFirstMarkFragment,
                        &tr.world->bmodels[-mf->iIndex].iNumMarkFragment
                    );

                    pPoly->pNextPoly      = pMark->pMarkEditPolys;
                    pMark->pMarkEditPolys = pPoly;
                    pMark->iNumEditPolys++;
                }
            }
        }
    }

    return pMark;
}

/*
=============
DCLC_Make

=============
*/
void DCLC_Make(void)
{
    lmEditMarkDef_t *pNewMark;
    vec3_t           vProjection;

    VectorNegate(tr.refdef.viewaxis[0], vProjection);

    pNewMark = R_ApplyLevelDecal(tr.refdef.vieworg, vProjection, qfalse, qfalse, 0);
    if (pNewMark) {
        lm.pCurrentMark       = pNewMark;
        lm.bAutoApplySettings = qtrue;
    }
}

/*
=============
DCLC_Reapply

=============
*/
void DCLC_Reapply(void)
{
    lmEditMarkDef_t *pNewMark;

    if (!lm.pCurrentMark) {
        return;
    }
    if (lm.pCurrentMark->fRotation == -1) {
        return;
    }

    pNewMark = R_ApplyLevelDecal(lm.pCurrentMark->vPos, lm.pCurrentMark->vProjection, qtrue, qfalse, 0);
    if (pNewMark) {
        lm.pCurrentMark       = pNewMark;
        lm.bAutoApplySettings = qtrue;
    }
}

/*
=============
DCLC_Delete

=============
*/
void DCLC_Delete(void)
{
    lmEditMarkDef_t *pMark;

    if (!lm.pCurrentMark) {
        return;
    }
    pMark = lm.pCurrentMark;

    R_FreeMarkFragments(lm.pCurrentMark);
    lm.pCurrentMark = pMark->pNextMark;
    if (lm.pCurrentMark == &lm.activeMarkDefs) {
        lm.pCurrentMark = pMark->pPrevMark;
        if (lm.pCurrentMark == &lm.activeMarkDefs) {
            lm.pCurrentMark = NULL;
        }
    }

    pMark->pPrevMark->pNextMark = pMark->pNextMark;
    pMark->pNextMark->pPrevMark = pMark->pPrevMark;
    pMark->pNextMark            = lm.pFreeMarkDefs;
    pMark->pPrevMark            = 0;
    lm.pFreeMarkDefs            = pMark;

    if (dcl_autogetinfo->integer) {
        lm.bAutoApplySettings = qtrue;
        DCLC_GetInfo();
    } else {
        lm.bAutoApplySettings = qfalse;
    }
}

/*
=============
DCLC_SelectNext

=============
*/
void DCLC_SelectNext(void)
{
    lmEditMarkDef_t *pMark;

    lm.bAutoApplySettings = dcl_autogetinfo->integer != 0;

    if (!lm.pCurrentMark) {
        if (lm.activeMarkDefs.pPrevMark != &lm.activeMarkDefs) {
            lm.pCurrentMark = lm.activeMarkDefs.pPrevMark;
        }

        if (dcl_autogetinfo->integer) {
            DCLC_GetInfo();
        }
        return;
    }
    pMark = lm.pCurrentMark;

    lm.pCurrentMark = lm.pCurrentMark->pPrevMark;
    if (lm.pCurrentMark == &lm.activeMarkDefs) {
        lm.pCurrentMark = lm.activeMarkDefs.pPrevMark;
        if (lm.activeMarkDefs.pPrevMark == &lm.activeMarkDefs) {
            lm.pCurrentMark = pMark;
        }
    }

    if (dcl_autogetinfo->integer) {
        DCLC_GetInfo();
    }
}

/*
=============
DCLC_SelectPrev

=============
*/
void DCLC_SelectPrev(void)
{
    lmEditMarkDef_t *pMark;

    lm.bAutoApplySettings = dcl_autogetinfo->integer != 0;

    if (!lm.pCurrentMark) {
        if (lm.activeMarkDefs.pNextMark != &lm.activeMarkDefs) {
            lm.pCurrentMark = lm.activeMarkDefs.pNextMark;
        }

        if (dcl_autogetinfo->integer) {
            DCLC_GetInfo();
        }
        return;
    }
    pMark = lm.pCurrentMark;

    lm.pCurrentMark = lm.pCurrentMark->pNextMark;
    if (lm.pCurrentMark == &lm.activeMarkDefs) {
        lm.pCurrentMark = lm.activeMarkDefs.pNextMark;
        if (lm.activeMarkDefs.pNextMark == &lm.activeMarkDefs) {
            lm.pCurrentMark = pMark;
        }
    }

    if (dcl_autogetinfo->integer) {
        DCLC_GetInfo();
    }
}

/*
=============
DCLC_SelectCh

=============
*/
void DCLC_SelectCh(void)
{
    // Nothing to do
}

/*
=============
DCLC_SelectChNxt

=============
*/
void DCLC_SelectChNxt(void)
{
    // Nothing to do
}

/*
=============
DCLC_GetInfo

=============
*/
void DCLC_GetInfo(void)
{
    if (!lm.pCurrentMark) {
        return;
    }

    if (lm.pCurrentMark->fRotation == -1) {
        ri.Cvar_Set("dcl_shader", lm.pCurrentMark->markShader->name);
        ri.Cvar_SetValue("dcl_radius", lm.pCurrentMark->fRadius);
        ri.Cvar_SetValue("dcl_widthscale", lm.pCurrentMark->fWidthScale);
    } else {
        ri.Cvar_Set("dcl_shader", lm.pCurrentMark->markShader->name);
        ri.Cvar_SetValue("dcl_radius", lm.pCurrentMark->fRadius);
        ri.Cvar_SetValue("dcl_heightscale", lm.pCurrentMark->fHeightScale);
        ri.Cvar_SetValue("dcl_heightscale", lm.pCurrentMark->fHeightScale);
        ri.Cvar_SetValue("dcl_widthscale", lm.pCurrentMark->fWidthScale);
        ri.Cvar_SetValue("dcl_rotation", lm.pCurrentMark->fRotation);
        ri.Cvar_SetValue("dcl_r", lm.pCurrentMark->color[0]);
        ri.Cvar_SetValue("dcl_g", lm.pCurrentMark->color[1]);
        ri.Cvar_SetValue("dcl_b", lm.pCurrentMark->color[2]);
    }

    ri.Cvar_SetValue("dcl_alpha", lm.pCurrentMark->color[3]);
    ri.Cvar_SetValue("dcl_dolighting", lm.pCurrentMark->bDoLighting);

    lm.bAutoApplySettings = qtrue;
}

/*
=============
DCLC_ShiftUp

=============
*/
void DCLC_ShiftUp(void)
{
    vec3_t vAngles;
    vec3_t vUp;

    if (!lm.pCurrentMark) {
        return;
    }
    if (lm.pCurrentMark->fRotation == -1) {
        return;
    }

    vectoangles(lm.pCurrentMark->vProjection, vAngles);
    AngleVectors(vAngles, NULL, NULL, vUp);

    VectorMA(lm.pCurrentMark->vPos, dcl_shiftstep->value, vUp, lm.pCurrentMark->vPos);

    DCLC_Reapply();
}

/*
=============
DCLC_ShiftDown

=============
*/
void DCLC_ShiftDown(void)
{
    vec3_t vAngles;
    vec3_t vUp;

    if (!lm.pCurrentMark) {
        return;
    }
    if (lm.pCurrentMark->fRotation == -1) {
        return;
    }

    vectoangles(lm.pCurrentMark->vProjection, vAngles);
    AngleVectors(vAngles, NULL, NULL, vUp);

    VectorMA(lm.pCurrentMark->vPos, -dcl_shiftstep->value, vUp, lm.pCurrentMark->vPos);

    DCLC_Reapply();
}

/*
=============
DCLC_ShiftLeft

=============
*/
void DCLC_ShiftLeft(void)
{
    vec3_t vAngles;
    vec3_t vRight;

    if (!lm.pCurrentMark) {
        return;
    }
    if (lm.pCurrentMark->fRotation == -1) {
        return;
    }

    vectoangles(lm.pCurrentMark->vProjection, vAngles);
    AngleVectors(vAngles, NULL, vRight, NULL);

    VectorMA(lm.pCurrentMark->vPos, -dcl_shiftstep->value, vRight, lm.pCurrentMark->vPos);

    DCLC_Reapply();
}

/*
=============
DCLC_ShiftRight

=============
*/
void DCLC_ShiftRight(void)
{
    vec3_t vAngles;
    vec3_t vRight;

    if (!lm.pCurrentMark) {
        return;
    }
    if (lm.pCurrentMark->fRotation == -1) {
        return;
    }

    vectoangles(lm.pCurrentMark->vProjection, vAngles);
    AngleVectors(vAngles, NULL, vRight, NULL);

    VectorMA(lm.pCurrentMark->vPos, dcl_shiftstep->value, vRight, lm.pCurrentMark->vPos);

    DCLC_Reapply();
}

/*
=============
DCLC_RandomRoll

=============
*/
void DCLC_RandomRoll(void)
{
    ri.Cvar_SetValue("dcl_rotation", random() * 360.0);
}

/*
=============
R_LevelMarksInit

Initialize the level mark system with variables and commands
=============
*/
void R_LevelMarksInit()
{
    memset(&lm, 0, sizeof(lm));

    dcl_editmode     = ri.Cvar_Get("dcl_editmode", "0", CVAR_LATCH);
    dcl_showcurrent  = ri.Cvar_Get("dcl_showcurrent", "1", 0);
    dcl_autogetinfo  = ri.Cvar_Get("dcl_autogetinfo", "1", 0);
    dcl_shiftstep    = ri.Cvar_Get("dcl_shiftstep", "4", 0);
    dcl_shader       = ri.Cvar_Get("dcl_shader", "blastmark", 0);
    dcl_radius       = ri.Cvar_Get("dcl_radius", "16", 0);
    dcl_heightscale  = ri.Cvar_Get("dcl_heightscale", "1", 0);
    dcl_widthscale   = ri.Cvar_Get("dcl_widthscale", "1", 0);
    dcl_rotation     = ri.Cvar_Get("dcl_rotation", "0", 0);
    dcl_r            = ri.Cvar_Get("dcl_r", "1", 0);
    dcl_g            = ri.Cvar_Get("dcl_g", "1", 0);
    dcl_b            = ri.Cvar_Get("dcl_b", "1", 0);
    dcl_alpha        = ri.Cvar_Get("dcl_alpha", "1", 0);
    dcl_dolighting   = ri.Cvar_Get("dcl_dolighting", "1", 0);
    dcl_doworld      = ri.Cvar_Get("dcl_doworld", "1", 0);
    dcl_doterrain    = ri.Cvar_Get("dcl_doterrain", "1", 0);
    dcl_dobmodels    = ri.Cvar_Get("dcl_dobmodels", "1", 0);
    dcl_dostring     = ri.Cvar_Get("dcl_dostring", "apply to all", 0);
    dcl_pathmode     = ri.Cvar_Get("dcl_pathmode", "0", 0);
    dcl_maxsegment   = ri.Cvar_Get("dcl_maxsegment", "512", 0);
    dcl_minsegment   = ri.Cvar_Get("dcl_minsegment", "24", 0);
    dcl_maxoffset    = ri.Cvar_Get("dcl_maxoffset", "10", 0);
    dcl_texturescale = ri.Cvar_Get("dcl_texturescale", "32", 0);

    if (dcl_editmode->integer) {
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

/*
=============
R_LevelMarksFree

Remove level marks commands and deallocate all memory
=============
*/
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

    if (tr.world) {
        for (i = 0; i < tr.world->numBmodels; i++) {
            bmodel_t *pBmodel = &tr.world->bmodels[i];

            if (pBmodel->iNumMarkFragment && pBmodel->pFirstMarkFragment) {
                ri.Free(pBmodel->pFirstMarkFragment);
                pBmodel->pFirstMarkFragment = NULL;
                pBmodel->iNumMarkFragment   = 0;
            }
        }

        for (i = tr.world->numDecisionNodes; i < tr.world->numnodes; i++) {
            mnode_t *pLeaf = &tr.world->nodes[i];

            if (pLeaf->iNumMarkFragment && pLeaf->pFirstMarkFragment) {
                ri.Free(pLeaf->pFirstMarkFragment);
                pLeaf->pFirstMarkFragment = NULL;
                pLeaf->iNumMarkFragment   = 0;
            }
        }
    }
}

/*
=============
R_InitTreadMark

=============
*/
static void R_InitTreadMark(treadMark_t *treadMark)
{
    treadMark->iReferenceNumber = 0;
    treadMark->iLastTime        = 0;
    treadMark->hTreadShader     = 0;
    treadMark->iState           = 0;
    treadMark->fWidth           = 0;

    VectorClear(treadMark->vStartDir);
    VectorClear(treadMark->vStartVerts[0]);
    VectorClear(treadMark->vStartVerts[1]);
    treadMark->fStartTexCoord = 0;
    treadMark->fStartAlpha    = 0;

    VectorClear(treadMark->vMidPos);
    VectorClear(treadMark->vMidVerts[0]);
    VectorClear(treadMark->vMidVerts[1]);
    treadMark->fMidTexCoord = 0;
    treadMark->fMidAlpha    = 0;

    VectorClear(treadMark->vEndPos);
    VectorClear(treadMark->vEndVerts[0]);
    VectorClear(treadMark->vEndVerts[1]);
    treadMark->fEndTexCoord = 0;
    treadMark->fEndAlpha    = 0;
}

/*
=============
R_UpdateLevelMarksSystem

Draw lines for editing
=============
*/
void R_UpdateLevelMarksSystem()
{
    int   iReturn;
    float fAlpha;
    float fTexScale;

    if (!dcl_editmode->integer) {
        return;
    }

    if (dcl_pathmode->integer) {
        if (!lm.bPathLayingMode) {
            R_InitTreadMark(&lm.treadMark);
            lm.treadMark.iLastTime  = -1000;
            lm.pTreadMarkStartDecal = NULL;
            lm.pTreadMarkEndDecal   = NULL;
            lm.bPathLayingMode      = qtrue;
        }

        lm.treadMark.hTreadShader = RE_RegisterShader(dcl_shader->string);
        lm.treadMark.fWidth       = Q_clamp_float(dcl_radius->value * dcl_widthscale->value, 2, 1024);

        fAlpha    = Q_clamp_float(dcl_alpha->value, 0, 1);
        fTexScale = 1.0 / dcl_texturescale->value;
        iReturn   = ri.CG_PermanentUpdateTreadMark(
            &lm.treadMark, fAlpha, dcl_minsegment->value, dcl_maxsegment->value, dcl_maxoffset->value, fTexScale
        );

        if (iReturn == 1) {
            lm.pTreadMarkStartDecal = lm.pTreadMarkEndDecal;
            lm.pTreadMarkEndDecal   = NULL;

            ri.CG_PermanentUpdateTreadMark(
                &lm.treadMark, fAlpha, dcl_minsegment->value, dcl_maxsegment->value, dcl_maxoffset->value, fTexScale
            );
        }

        if (iReturn != -1) {
            if (!lm.pTreadMarkEndDecal) {
                lm.pTreadMarkEndDecal = R_AllocateMarkDef();
            }

            if (lm.pTreadMarkEndDecal) {
                lm.pCurrentMark = lm.pTreadMarkEndDecal;
                lm.pCurrentMark = R_ApplyLevelDecal(vec_upwards, vec_upwards, qtrue, qtrue, 2);
                if (!lm.pCurrentMark) {
                    R_FreeMarkFragments(lm.pTreadMarkEndDecal);
                }
            }

            if (lm.treadMark.iState == 3) {
                if (!lm.pTreadMarkStartDecal) {
                    lm.pTreadMarkStartDecal = R_AllocateMarkDef();
                }

                if (lm.pTreadMarkStartDecal) {
                    lm.pCurrentMark = lm.pTreadMarkStartDecal;
                    lm.pCurrentMark = R_ApplyLevelDecal(vec_upwards, vec_upwards, qtrue, qtrue, 1);
                    if (!lm.pCurrentMark) {
                        R_FreeMarkFragments(lm.pTreadMarkStartDecal);
                    }
                }
            }
        }

        if (lm.pTreadMarkEndDecal) {
            lm.pCurrentMark = lm.pTreadMarkEndDecal;
        } else if (lm.pTreadMarkStartDecal) {
            lm.pCurrentMark = lm.pTreadMarkStartDecal;
        }

    } else if (lm.bPathLayingMode) {
        lm.bPathLayingMode = qfalse;
    }

    if (lm.bAutoApplySettings && lm.pCurrentMark && lm.pCurrentMark->fRotation != -1) {
        qboolean bDoUpdate = qfalse;
        float    f;

        if (dcl_shader->modified) {
            shader_t *shader;
            qhandle_t hShader;

            hShader = RE_RegisterShader(dcl_shader->string);
            if (hShader) {
                shader = R_GetShaderByHandle(hShader);
                if (shader != tr.defaultShader && lm.pCurrentMark->markShader != shader) {
                    lm.pCurrentMark->markShader = shader;
                    // Set the decal radius
                    ri.Cvar_SetValue("dcl_radius", 16.0);
                    dcl_radius->modified = qfalse;

                    // Set the decal scale
                    ri.Cvar_SetValue(
                        "dcl_heightscale", shader->unfoggedStages[0]->bundle[0].image[0]->height / 16.0 * 0.5
                    );
                    ri.Cvar_SetValue("dcl_widthscale", shader->unfoggedStages[0]->bundle[0].image[0]->width / 16.0 * 0.5);

                    bDoUpdate = qtrue;
                }
            }
            dcl_shader->modified = 0;
        }

        if (dcl_radius->modified) {
            f = dcl_radius->value;
            if (lm.pCurrentMark->fRadius != f) {
                lm.pCurrentMark->fRadius = f;

                bDoUpdate = qtrue;
            }

            dcl_radius->modified = qfalse;
        }

        if (dcl_heightscale->modified) {
            f = dcl_heightscale->value;
            if (lm.pCurrentMark->fHeightScale != f) {
                lm.pCurrentMark->fHeightScale = f;

                bDoUpdate = qtrue;
            }

            dcl_heightscale->modified = qfalse;
        }
        if (dcl_widthscale->modified) {
            f = dcl_widthscale->value;
            if (lm.pCurrentMark->fWidthScale != f) {
                lm.pCurrentMark->fWidthScale = f;

                bDoUpdate = qtrue;
            }

            dcl_widthscale->modified = qfalse;
        }

        if (dcl_rotation->modified) {
            f = anglemod(dcl_rotation->value);
            if (lm.pCurrentMark->fRotation != f) {
                lm.pCurrentMark->fRotation = f;

                bDoUpdate = qtrue;
            }

            dcl_rotation->modified = qfalse;
        }

        if (dcl_r->modified) {
            f = Q_clamp_float(dcl_r->value, 0, 1);
            if (lm.pCurrentMark->color[0] != f) {
                lm.pCurrentMark->color[0] = f;

                bDoUpdate = qtrue;
            }

            dcl_r->modified = qfalse;
        }
        if (dcl_g->modified) {
            f = Q_clamp_float(dcl_g->value, 0, 1);
            if (lm.pCurrentMark->color[1] != f) {
                lm.pCurrentMark->color[1] = f;

                bDoUpdate = qtrue;
            }

            dcl_g->modified = qfalse;
        }
        if (dcl_b->modified) {
            f = Q_clamp_float(dcl_b->value, 0, 1);
            if (lm.pCurrentMark->color[2] != f) {
                lm.pCurrentMark->color[2] = f;

                bDoUpdate = qtrue;
            }

            dcl_b->modified = qfalse;
        }
        if (dcl_alpha->modified) {
            f = Q_clamp_float(dcl_alpha->value, 0, 1);
            if (lm.pCurrentMark->color[3] != f) {
                lm.pCurrentMark->color[3] = f;

                bDoUpdate = qtrue;
            }

            dcl_alpha->modified = qfalse;
        }

        if (dcl_dolighting->modified) {
            qboolean bDoLighting = dcl_dolighting->integer != 0;
            if (lm.pCurrentMark->bDoLighting != bDoLighting) {
                lm.pCurrentMark->bDoLighting = bDoLighting;
                bDoUpdate                    = 1;
            }
            dcl_dolighting->modified = 0;
        }

        if (bDoUpdate) {
            lm.pCurrentMark = R_ApplyLevelDecal(lm.pCurrentMark->vPos, lm.pCurrentMark->vProjection, qtrue, qtrue, 0);
        }
    }

    if (dcl_showcurrent->integer && lm.pCurrentMark && lm.pCurrentMark->iNumEditPolys) {
        float          fColor;
        float          fLength;
        int            j;
        int            iEntNum;
        lmEditPoly_t  *pPoly;
        polyVert_t    *pVert, *pVert2;
        polyVert_t     verts[8];
        trRefEntity_t *ent;
        bmodel_t      *bmodel, *lastbmodel;
        model_t       *pModel;
        vec3_t         vOrigin;
        vec3_t         axis[3];
        vec3_t         vPos, vPos2;
        vec3_t         polyaxis[3];

        iEntNum    = 0;
        ent        = NULL;
        lastbmodel = NULL;
        vectoangles(lm.pCurrentMark->vProjection, vPos);

        vPos[2] = anglemod(vPos[2] + lm.pCurrentMark->fRotation);
        AngleVectorsLeft(vPos, polyaxis[0], polyaxis[1], polyaxis[2]);
        VectorNormalize2(lm.pCurrentMark->vProjection, polyaxis[0]);
        PerpendicularVector(polyaxis[1], polyaxis[0]);
        RotatePointAroundVector(polyaxis[2], polyaxis[0], polyaxis[1], anglemod(lm.pCurrentMark->fRotation + 90.0));
        CrossProduct(polyaxis[0], polyaxis[2], polyaxis[1]);

        fColor = sin(tr.refdef.time / 500.0) / 4.0 + 0.6;

        for (pPoly = lm.pCurrentMark->pMarkEditPolys; pPoly; pPoly = pPoly->pNextPoly) {
            if (pPoly->surf.iIndex > 0) {
                for (j = 0; j < pPoly->surf.numVerts; j++) {
                    VectorMA(pPoly->verts[j % pPoly->surf.numVerts].xyz, 0.1, polyaxis[0], vPos);
                    VectorMA(pPoly->verts[(j + 1) % pPoly->surf.numVerts].xyz, 0.1, polyaxis[0], vPos2);
                    R_DebugLine(vPos, vPos2, fColor, fColor, fColor, 1.0);

                    VectorMA(vPos, 8.0, polyaxis[0], vPos2);
                    R_DebugLine(vPos, vPos2, fColor, 1.0 - fColor, 0.1, 1.0);
                }
            } else if (pPoly->surf.iIndex < 0) {
                bmodel = &tr.world->bmodels[-pPoly->surf.iIndex];
                if (bmodel != lastbmodel) {
                    //
                    // Find the matching entity
                    //

                    ent = &tr.refdef.entities[0];
                    for (iEntNum = 0; iEntNum < tr.refdef.num_entities; iEntNum++) {
                        ent = &tr.refdef.entities[iEntNum];

                        pModel = R_GetModelByHandle(ent->e.hModel);
                        if (pModel->type == MOD_BRUSH && pModel->d.bmodel == bmodel) {
                            break;
                        }
                    }

                    VectorCopy(ent->e.origin, vOrigin);
                    AxisCopy(ent->e.axis, axis);
                }

                if (iEntNum != tr.refdef.num_entities) {
                    for (j = 0; j < pPoly->surf.numVerts; j++) {
                        MatrixTransformVector(pPoly->verts[j].xyz, axis, pPoly->verts[j].xyz);
                        VectorAdd(pPoly->verts[j].xyz, vOrigin, pPoly->verts[j].xyz);
                    }

                    for (j = 0; j < pPoly->surf.numVerts; j++) {
                        VectorMA(pPoly->verts[j % pPoly->surf.numVerts].xyz, 0.1, polyaxis[0], vPos);
                        VectorMA(pPoly->verts[(j + 1) % pPoly->surf.numVerts].xyz, 0.1, polyaxis[0], vPos2);
                        R_DebugLine(vPos, vPos2, fColor, fColor, fColor, 1.0);

                        VectorMA(vPos, 8.0, polyaxis[0], vPos2);
                        R_DebugLine(vPos, vPos2, 0.1, 1.0 - fColor, fColor, 1.0);
                    }
                }
            } else {
                for (j = 0; j < pPoly->surf.numVerts; j++) {
                    VectorMA(pPoly->verts[j % pPoly->surf.numVerts].xyz, 0.1, polyaxis[0], vPos);
                    VectorMA(pPoly->verts[(j + 1) % pPoly->surf.numVerts].xyz, 0.1, polyaxis[0], vPos2);
                    R_DebugLine(vPos, vPos2, fColor, fColor, fColor, 1.0);

                    VectorMA(vPos, 8.0, polyaxis[0], vPos2);
                    R_DebugLine(vPos, vPos2, 0.1, 1.0 - fColor, fColor, 1.0);
                }
            }
        }

        //
        // Draw the front
        //

        fLength = (lm.pCurrentMark->fWidthScale + lm.pCurrentMark->fHeightScale) * lm.pCurrentMark->fRadius * 0.75;
        if (fLength < 4) {
            fLength = 4;
        }

        VectorMA(lm.pCurrentMark->vPos, fLength, polyaxis[0], vPos);

        R_DebugLine(lm.pCurrentMark->vPos, vPos, 0.1, fColor, 1.0 - fColor, 1.0);

        //
        // Draw the right
        //

        fLength = lm.pCurrentMark->fRadius * lm.pCurrentMark->fWidthScale;
        if (fLength < 1) {
            fLength = 1;
        }

        VectorMA(lm.pCurrentMark->vPos, fLength, polyaxis[1], vOrigin);
        VectorMA(lm.pCurrentMark->vPos, -fLength, polyaxis[1], vPos);
        R_DebugLine(vOrigin, vPos, 0.1, fColor, 1.0 - fColor, 1.0);

        //
        // Draw the up
        //

        fLength = lm.pCurrentMark->fRadius * lm.pCurrentMark->fHeightScale;
        if (fLength < 1) {
            fLength = 1;
        }

        VectorMA(lm.pCurrentMark->vPos, fLength, polyaxis[2], vOrigin);
        VectorMA(lm.pCurrentMark->vPos, -fLength, polyaxis[2], vPos);
        R_DebugLine(vOrigin, vPos, 0.1, fColor, 1.0 - fColor, 1.0);
    }
}

/*
=============
R_AddPermanentMarkFragmentSurfaces

=============
*/
void R_AddPermanentMarkFragmentSurfaces(void **pFirstMarkFragment, int iNumMarkFragment)
{
    int       i;
    lmPoly_t *pPoly;

    for (i = 0; i < iNumMarkFragment; i++) {
        pPoly = (lmPoly_t *)pFirstMarkFragment[i];

        if (pPoly->viewCount == tr.viewCount) {
            continue;
        }
        pPoly->viewCount = tr.viewCount;

        R_AddDrawSurf(&pPoly->surf.surfaceType, pPoly->shader, 0);
    }
}