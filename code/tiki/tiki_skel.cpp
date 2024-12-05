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

// tiki_skel.cpp : TIKI skeletor loader
//
// Some part of the code uses CopyLittleLong / LongNoSwapPtr, etc. because some CPUs can't read misaligned memory
//

#include "q_shared.h"
#include "qcommon.h"
#include "../skeletor/skeletor.h"
#include "../client/client.h"
#include "tiki.h"

/*
===============
LoadBoneFromBuffer2
===============
*/
void LoadBoneFromBuffer2(boneFileData_t *fileData, boneData_t *boneData)
{
    char *newChannelName;
    char *newBoneRefName;
    int   i;

    memset(boneData, 0, sizeof(boneData_t));
    boneData->channel  = skeletor_c::m_boneNames.RegisterChannel(fileData->name);
    boneData->boneType = fileData->boneType;

    if (boneData->boneType == SKELBONE_HOSEROT) {
        i = LittleLongPtr(fileData->parent[fileData->ofsBaseData + 4]);

        if (i == 1) {
            boneData->boneType = SKELBONE_HOSEROTPARENT;
        } else if (i == 2) {
            boneData->boneType = SKELBONE_HOSEROTBOTH;
        }
    }

    newChannelName        = (char *)fileData + fileData->ofsChannelNames;
    boneData->numChannels = skelBone_Base::GetNumChannels(boneData->boneType);

    for (i = 0; i < boneData->numChannels; i++) {
        boneData->channelIndex[i] = skeletor_c::m_channelNames.RegisterChannel(newChannelName);
        if (boneData->channelIndex[i] < 0) {
            SKEL_Warning("Channel named %s not added. (Bone will not work without it)\n", newChannelName);
            boneData->boneType = SKELBONE_ZERO;
        }

        newChannelName += strlen(newChannelName) + 1;
    }

    newBoneRefName    = (char *)fileData + fileData->ofsBoneNames;
    boneData->numRefs = skelBone_Base::GetNumBoneRefs(boneData->boneType);

    for (i = 0; i < boneData->numRefs; i++) {
        boneData->refIndex[i] = skeletor_c::m_boneNames.RegisterChannel(newBoneRefName);
        newBoneRefName += strlen(newBoneRefName) + 1;
    }

    if (!strcmp(fileData->parent, SKEL_BONENAME_WORLD)) {
        boneData->parent = -1;
    } else {
        boneData->parent = skeletor_c::m_boneNames.RegisterChannel(fileData->parent);
    }

    switch (boneData->boneType) {
    case SKELBONE_ROTATION:
        {
            float *baseData = (float *)((char *)fileData + fileData->ofsBaseData);
            CopyLittleLong(&boneData->offset[0], &baseData[0]);
            CopyLittleLong(&boneData->offset[1], &baseData[1]);
            CopyLittleLong(&boneData->offset[2], &baseData[2]);
            break;
        }
    case SKELBONE_IKSHOULDER:
        {
            float *baseData = (float *)((char *)fileData + fileData->ofsBaseData);
            CopyLittleLong(&boneData->offset[0], &baseData[4]);
            CopyLittleLong(&boneData->offset[1], &baseData[5]);
            CopyLittleLong(&boneData->offset[2], &baseData[6]);
            break;
        }
    case SKELBONE_IKELBOW:
    case SKELBONE_IKWRIST:
        {
            float *pBaseData = (float *)((char *)fileData + fileData->ofsBaseData);
            vec3_t baseData;
            CopyLittleLong(&baseData[0], &pBaseData[0]);
            CopyLittleLong(&baseData[1], &pBaseData[1]);
            CopyLittleLong(&baseData[2], &pBaseData[2]);
            boneData->length = VectorLength(baseData);
            break;
        }
    case SKELBONE_AVROT:
        {
            float *baseData = (float *)((char *)fileData + fileData->ofsBaseData);
            CopyLittleLong(&boneData->length, &baseData[0]);
            CopyLittleLong(&boneData->offset[0], &baseData[1]);
            CopyLittleLong(&boneData->offset[1], &baseData[2]);
            CopyLittleLong(&boneData->offset[2], &baseData[3]);
            break;
        }
    case SKELBONE_HOSEROT:
    case SKELBONE_HOSEROTBOTH:
    case SKELBONE_HOSEROTPARENT:
        {
            float *baseData = (float *)((char *)fileData + fileData->ofsBaseData);
            CopyLittleLong(&boneData->offset[0], &baseData[3]);
            CopyLittleLong(&boneData->offset[1], &baseData[4]);
            CopyLittleLong(&boneData->offset[2], &baseData[5]);
            CopyLittleLong(&boneData->bendRatio, &baseData[0]);
            CopyLittleLong(&boneData->bendMax, &baseData[1]);
            CopyLittleLong(&boneData->spinRatio, &baseData[2]);
            break;
        }
    default:
        break;
    }
}

/*
===============
TIKI_CacheFileSkel
===============
*/
void TIKI_CacheFileSkel(skelHeader_t *pHeader, skelcache_t *cache, int length)
{
    skelHeaderGame_t  *pSkel;
    skelSurfaceGame_t *pGameSurf;
    skelSurface_t     *pSurf;
    int                i, j, k;
    size_t             nSurfBytes;
    size_t             nBoneBytes;
    size_t             nBoxBytes;
    size_t             nMorphBytes;
    size_t             nVertBytes;
    size_t             nTriBytes;
    byte              *start_gs_ptr;
    byte              *max_gs_ptr;
    byte              *gs_ptr = NULL;

    pSurf      = (skelSurface_t *)((byte *)pHeader + pHeader->ofsSurfaces);
    nSurfBytes = 0;

    for (i = 0; i < pHeader->numSurfaces; i++) {
        skeletorVertex_t *pVert    = (skeletorVertex_t *)((byte *)pSurf + LongNoSwapPtr(&pSurf->ofsVerts));
        const int         numVerts = LongNoSwapPtr(&pSurf->numVerts);

        nVertBytes = 0;
        for (j = 0; j < numVerts; j++) {
            const int numMorphs  = LongNoSwapPtr(&pVert->numMorphs);
            const int numWeights = LongNoSwapPtr(&pVert->numWeights);

            int iOffset = sizeof(skeletorMorph_t) * numMorphs;
            iOffset += sizeof(skelWeight_t) * numWeights;
            iOffset += sizeof(skeletorVertex_t);

            nVertBytes += iOffset;
            pVert = (skeletorVertex_t *)((byte *)pVert + iOffset);
        }

        nSurfBytes += sizeof(skelSurfaceGame_t);
        // triangles
        nSurfBytes += LongNoSwapPtr(&pSurf->numTriangles) * sizeof(skelIndex_t) * 3;
        nSurfBytes = PAD(nSurfBytes, sizeof(void *));
        // vertices
        nSurfBytes += nVertBytes;
        nSurfBytes = PAD(nSurfBytes, sizeof(void *));
        // collapse
        nSurfBytes += numVerts * sizeof(skelIndex_t);

        if (pHeader->version > TIKI_SKB_HEADER_VER_3) {
            // collapse indices
            nSurfBytes += numVerts * sizeof(skelIndex_t);
        }

        pSurf = (skelSurface_t *)((byte *)pSurf + LongNoSwapPtr(&pSurf->ofsEnd));
    }

    nBoneBytes  = pHeader->numBones * sizeof(boneData_t);
    nBoxBytes   = 0;
    nMorphBytes = 0;

    if (pHeader->version > TIKI_SKB_HEADER_VERSION) {
        char    *pMorphTargets;
        intptr_t nLen;

        nBoxBytes     = pHeader->numBoxes * sizeof(skelHitBox_t);
        pMorphTargets = (char *)((byte *)pHeader + pHeader->ofsMorphTargets);

        if (pHeader->ofsMorphTargets > 0 || (pHeader->ofsMorphTargets + pHeader->numMorphTargets) < length) {
            for (i = 0; i < pHeader->numMorphTargets; i++) {
                nLen = strlen(pMorphTargets) + 1;
                nMorphBytes += nLen;
                pMorphTargets += nLen;
            }
        } else {
            nMorphBytes = pHeader->numMorphTargets;
        }
    } else if (pHeader->version == TIKI_SKB_HEADER_VERSION) {
        nBoxBytes = pHeader->numBoxes * sizeof(skelHitBox_t);
    }

    cache->size = sizeof(skelHeaderGame_t);
    cache->size += nSurfBytes;
    cache->size = PAD(cache->size, sizeof(void *));
    cache->size += nBoneBytes;
    cache->size = PAD(cache->size, sizeof(void *));
    cache->size += nBoxBytes;
    cache->size = PAD(cache->size, sizeof(void *));
    cache->size += nMorphBytes;

    cache->skel = pSkel = (skelHeaderGame_t *)TIKI_Alloc(cache->size);
    byte *start_ptr     = (byte *)pSkel;
    byte *max_ptr       = start_ptr + cache->size;
    byte *ptr           = start_ptr + sizeof(skelHeaderGame_t);

    pSkel->version     = pHeader->version;

    //
    // Added in 2.0
    //
    if (pHeader->version >= TIKI_SKD_HEADER_VERSION) {
        pSkel->scale = pHeader->scale * 0.52;
    } else {
        pSkel->scale = 0.52;
    }

    pSkel->numSurfaces = pHeader->numSurfaces;
    pSkel->numBones    = pHeader->numBones;
    pSkel->pSurfaces   = (skelSurfaceGame_t *)ptr;
    ptr += nSurfBytes;
    ptr           = (byte *)PADP(ptr, sizeof(void *));
    pSkel->pBones = (boneData_t *)ptr;
    ptr += nBoneBytes;
    ptr             = (byte *)PADP(ptr, sizeof(void *));
    pSkel->numBoxes = pHeader->numBoxes;
    pSkel->pBoxes   = (skelHitBox_t *)ptr;
    ptr += nBoxBytes;
    ptr                    = (byte *)PADP(ptr, sizeof(void *));
    pSkel->pLOD            = NULL;
    pSkel->numMorphTargets = pHeader->numMorphTargets;
    pSkel->pMorphTargets   = (char *)ptr;
    ptr += nMorphBytes;
    ptr = (byte *)PADP(ptr, sizeof(void *));
    memcpy(pSkel->name, pHeader->name, sizeof(pSkel->name));

    for (i = 0; i < sizeof(pSkel->lodIndex) / sizeof(pSkel->lodIndex[0]); i++) {
        pSkel->lodIndex[i] = LongNoSwapPtr(&pHeader->lodIndex[i]);
    }

    pSurf     = (skelSurface_t *)((byte *)pHeader + pHeader->ofsSurfaces);
    pGameSurf = pSkel->pSurfaces;

    for (i = 0; i < pHeader->numSurfaces; i++) {
        size_t            nBytesUsed;
        skeletorVertex_t *pVert;
        const int         numTriangles = LongNoSwapPtr(&pSurf->numTriangles);
        const int         numVerts     = LongNoSwapPtr(&pSurf->numVerts);

        nTriBytes  = numTriangles * sizeof(skelIndex_t) * 3;
        nSurfBytes = 0;
        if (pHeader->version > TIKI_SKB_HEADER_VER_3) {
            nSurfBytes = numVerts * sizeof(skelIndex_t);
        }

        pVert = (skeletorVertex_t *)((byte *)pSurf + LongNoSwapPtr(&pSurf->ofsVerts));

        nVertBytes = 0;
        for (j = 0; j < numVerts; j++) {
            const int numMorphs  = LongNoSwapPtr(&pVert->numMorphs);
            const int numWeights = LongNoSwapPtr(&pVert->numWeights);

            int iOffset = sizeof(skeletorMorph_t) * numMorphs;
            iOffset += sizeof(skelWeight_t) * numWeights;
            iOffset += sizeof(skeletorVertex_t);

            nVertBytes += iOffset;
            pVert = (skeletorVertex_t *)((byte *)pVert + iOffset);
        }

        nBytesUsed = sizeof(skelSurfaceGame_t);
        nBytesUsed += nTriBytes;
        nBytesUsed = PAD(nBytesUsed, sizeof(void *));
        nBytesUsed += nVertBytes;
        nBytesUsed = PAD(nBytesUsed, sizeof(void *));
        nBytesUsed += nSurfBytes;

        start_gs_ptr = (byte *)pGameSurf;
        max_gs_ptr   = (byte *)pGameSurf + nBytesUsed;
        gs_ptr       = start_gs_ptr + sizeof(skelSurfaceGame_t);

        pGameSurf->ident            = LongNoSwapPtr(&pSurf->ident);
        pGameSurf->numTriangles     = numTriangles;
        pGameSurf->numVerts         = numVerts;
        pGameSurf->pStaticXyz       = NULL;
        pGameSurf->pStaticNormal    = NULL;
        pGameSurf->pStaticTexCoords = NULL;
        pGameSurf->pTriangles       = (skelIndex_t *)gs_ptr;
        gs_ptr += nTriBytes;
        gs_ptr            = (byte *)PADP(gs_ptr, sizeof(void *));
        pGameSurf->pVerts = (skeletorVertex_t *)gs_ptr;
        gs_ptr += nVertBytes;
        gs_ptr               = (byte *)PADP(gs_ptr, sizeof(void *));
        pGameSurf->pCollapse = (skelIndex_t *)gs_ptr;
        gs_ptr += sizeof(*pGameSurf->pCollapse) * numVerts;
        if (pHeader->version > TIKI_SKB_HEADER_VER_3) {
            pGameSurf->pCollapseIndex = (skelIndex_t *)gs_ptr;
            gs_ptr += sizeof(*pGameSurf->pCollapseIndex) * numVerts;
        }

        memcpy(pGameSurf->name, pSurf->name, sizeof(pGameSurf->name));

        if (pGameSurf->numTriangles) {
            const int *pTriangles = (const int *)((byte *)pSurf + LongNoSwapPtr(&pSurf->ofsTriangles));
            for (j = 0; j < numTriangles; j++) {
                pGameSurf->pTriangles[j * 3 + 0] = LongNoSwapPtr(&pTriangles[j * 3 + 0]);
                pGameSurf->pTriangles[j * 3 + 1] = LongNoSwapPtr(&pTriangles[j * 3 + 1]);
                pGameSurf->pTriangles[j * 3 + 2] = LongNoSwapPtr(&pTriangles[j * 3 + 2]);
            }
        } else {
            pGameSurf->pTriangles = NULL;
        }

        if (pGameSurf->numVerts) {
            skeletorVertex_t *skelVert = pGameSurf->pVerts;
            pVert                      = (skeletorVertex_t *)((char *)pSurf + LongNoSwapPtr(&pSurf->ofsVerts));

            for (j = 0; j < pGameSurf->numVerts; j++) {
                const int numMorphs  = LongNoSwapPtr(&pVert->numMorphs);
                const int numWeights = LongNoSwapPtr(&pVert->numWeights);

                int iOffset = sizeof(skeletorMorph_t) * numMorphs;
                iOffset += sizeof(skelWeight_t) * numWeights;
                iOffset += sizeof(skeletorVertex_t);

                skelVert->normal[0]    = FloatNoSwapPtr(&pVert->normal[0]);
                skelVert->normal[1]    = FloatNoSwapPtr(&pVert->normal[1]);
                skelVert->normal[2]    = FloatNoSwapPtr(&pVert->normal[2]);
                skelVert->texCoords[0] = FloatNoSwapPtr(&pVert->texCoords[0]);
                skelVert->texCoords[1] = FloatNoSwapPtr(&pVert->texCoords[1]);
                skelVert->numWeights   = numWeights;
                skelVert->numMorphs    = numMorphs;

                // copy morphs
                skeletorMorph_t *pMorph    = (skeletorMorph_t *)((byte *)pVert + sizeof(skeletorVertex_t));
                skeletorMorph_t *skelMorph = (skeletorMorph_t *)((byte *)skelVert + sizeof(skeletorVertex_t));
                for (k = 0; k < numMorphs; k++, pMorph++, skelMorph++) {
                    skelMorph->morphIndex = LongNoSwapPtr(&pMorph->morphIndex);
                    skelMorph->offset[0]  = FloatNoSwapPtr(&pMorph->offset[0]);
                    skelMorph->offset[1]  = FloatNoSwapPtr(&pMorph->offset[1]);
                    skelMorph->offset[2]  = FloatNoSwapPtr(&pMorph->offset[2]);
                }

                skelWeight_t *pWeight    = (skelWeight_t *)((byte *)pMorph);
                skelWeight_t *skelWeight = (skelWeight_t *)((byte *)skelMorph);
                for (k = 0; k < numWeights; k++, skelWeight++, pWeight++) {
                    skelWeight->boneIndex  = LongNoSwapPtr(&pWeight->boneIndex);
                    skelWeight->boneWeight = FloatNoSwapPtr(&pWeight->boneWeight);
                    skelWeight->offset[0]  = FloatNoSwapPtr(&pWeight->offset[0]);
                    skelWeight->offset[1]  = FloatNoSwapPtr(&pWeight->offset[1]);
                    skelWeight->offset[2]  = FloatNoSwapPtr(&pWeight->offset[2]);
                }

                pVert    = (skeletorVertex_t *)((byte *)pVert + iOffset);
                skelVert = (skeletorVertex_t *)((byte *)skelVert + iOffset);
            }

            assert((byte *)skelVert - (byte *)pGameSurf <= (byte *)pGameSurf->pCollapse - (byte *)pGameSurf);

            const int *pCollapse = (const int *)((byte *)pSurf + LongNoSwapPtr(&pSurf->ofsCollapse));
            for (j = 0; j < numVerts; j++) {
                pGameSurf->pCollapse[j] = LongNoSwapPtr(&pCollapse[j]);
            }

            if (pHeader->version > TIKI_SKB_HEADER_VER_3) {
                const int *pCollapseIndex = (const int *)((byte *)pSurf + LongNoSwapPtr(&pSurf->ofsCollapseIndex));
                for (j = 0; j < numVerts; j++) {
                    pGameSurf->pCollapseIndex[j] = LongNoSwapPtr(&pCollapseIndex[j]);
                }
            } else {
                pGameSurf->pCollapseIndex = NULL;
            }
        } else {
            pGameSurf->pVerts         = NULL;
            pGameSurf->pCollapse      = NULL;
            pGameSurf->pCollapseIndex = NULL;
        }

        if (i != pHeader->numSurfaces - 1) {
            pGameSurf->pNext = (skelSurfaceGame_t *)gs_ptr;
            pGameSurf        = pGameSurf->pNext;
            pSurf            = (skelSurface_t *)((byte *)pSurf + LongNoSwapPtr(&pSurf->ofsEnd));
        }
    }

    pGameSurf->pNext = NULL;

    assert(((byte *)pGameSurf - (byte *)pSkel) <= cache->size);

    if (nBoneBytes) {
        if (pHeader->version <= TIKI_SKB_HEADER_VERSION) {
            skelBoneName_t *TIKI_bones = (skelBoneName_t *)((byte *)pHeader + LongNoSwapPtr(&pHeader->ofsBones));
            for (i = 0; i < pSkel->numBones; i++) {
                const char *boneName;

                if (TIKI_bones->parent == -1) {
                    boneName = SKEL_BONENAME_WORLD;
                } else {
                    boneName = TIKI_bones[TIKI_bones->parent].name;
                }

                CreatePosRotBoneData(TIKI_bones->name, boneName, &pSkel->pBones[i]);
                TIKI_bones++;
            }
        } else {
            boneFileData_t *boneBuffer = (boneFileData_t *)((byte *)pHeader + LongNoSwapPtr(&pHeader->ofsBones));
            for (i = 0; i < pSkel->numBones; i++) {
                LoadBoneFromBuffer2(boneBuffer, &pSkel->pBones[i]);
                boneBuffer = (boneFileData_t *)((byte *)boneBuffer + LongNoSwapPtr(&boneBuffer->ofsEnd));
            }
        }
    } else {
        pSkel->numBones = 0;
        pSkel->pBones   = NULL;
    }

    if (pHeader->version <= TIKI_SKB_HEADER_VER_3) {
        pSkel->numBoxes        = 0;
        pSkel->pBoxes          = NULL;
        pSkel->numMorphTargets = 0;
        pSkel->pMorphTargets   = NULL;
        return;
    }

    if (nBoxBytes) {
        if (pHeader->ofsBoxes <= 0 || (nBoxBytes + pHeader->ofsBoxes) > length) {
            Com_Printf("^~^~^ Box data is corrupted for '%s'\n", cache->path);
            pSkel->numBoxes = 0;
            pSkel->pBoxes   = NULL;
        } else {
            memcpy(pSkel->pBoxes, ((byte *)pHeader + pHeader->ofsBoxes), nBoxBytes);
        }
    } else {
        pSkel->numBoxes = 0;
        pSkel->pBoxes   = NULL;
    }

    if (pHeader->version <= TIKI_SKB_HEADER_VERSION) {
        pSkel->numMorphTargets = 0;
        pSkel->pMorphTargets   = NULL;
        return;
    }

    if (nMorphBytes) {
        if (pHeader->ofsMorphTargets <= 0 || (nMorphBytes + pHeader->ofsMorphTargets) > length) {
            Com_Printf("^~^~^ Morph targets data is corrupted for '%s'\n", cache->path);
            pSkel->numMorphTargets = 0;
            pSkel->pMorphTargets   = NULL;
        } else {
            memcpy(pSkel->pMorphTargets, ((byte *)pHeader + pHeader->ofsMorphTargets), nMorphBytes);
        }
    } else {
        pSkel->numMorphTargets = 0;
        pSkel->pMorphTargets   = NULL;
    }
}

/*
===============
TIKI_SortLOD
===============
*/
void TIKI_SortLOD(skelHeaderGame_t *skelmodel)
{
    skelSurfaceGame_t *surf;
    int                render_count;
    int                nTriVerts;
    int                i;
    int                collapse[TIKI_MAX_VERTEXES];
    int                iTemp;

    for (surf = skelmodel->pSurfaces; surf != NULL; surf = surf->pNext) {
        for (i = 0; i < surf->numVerts; i++) {
            collapse[i] = i;
        }

        render_count = surf->numTriangles * 3;

        nTriVerts = surf->numVerts - 1;
        while (nTriVerts > 1) {
            for (i = nTriVerts; i > 1; i--) {
                if (surf->pCollapseIndex[i - 1] != surf->pCollapseIndex[i]) {
                    break;
                }
            }

            nTriVerts = i - 1;
            for (; i < surf->numVerts; i++) {
                collapse[i] = collapse[surf->pCollapse[i]];
            }

            i = 0;
            while (i < render_count) {
                if (collapse[surf->pTriangles[i]] != collapse[surf->pTriangles[i + 1]]
                    && collapse[surf->pTriangles[i + 1]] != collapse[surf->pTriangles[i + 2]]
                    && collapse[surf->pTriangles[i + 2]] != collapse[surf->pTriangles[i]]) {
                    i += 3;
                } else {
                    render_count -= 3;

                    iTemp                          = surf->pTriangles[i];
                    surf->pTriangles[i]            = surf->pTriangles[render_count];
                    surf->pTriangles[render_count] = iTemp;

                    iTemp                              = surf->pTriangles[i + 1];
                    surf->pTriangles[i + 1]            = surf->pTriangles[render_count + 1];
                    surf->pTriangles[render_count + 1] = iTemp;

                    iTemp                              = surf->pTriangles[i + 2];
                    surf->pTriangles[i + 2]            = surf->pTriangles[render_count + 2];
                    surf->pTriangles[render_count + 2] = iTemp;
                }
            }
        }
    }
}

/*
===============
TIKI_LoadSKB
===============
*/
qboolean TIKI_LoadSKB(const char *path, skelcache_t *cache)
{
    int            i, j, k;
    skelHeader_t  *pheader;
    skelSurface_t *surf;
    int            version;
    unsigned int   header;
    int            length;
    char          *buf;
    int            totalVerts;
    int            newLength;
    skelHeader_t  *newHeader;
    skelSurface_t *oldSurf;
    skelSurface_t *newSurf;

    length = TIKI_ReadFileEx(path, (void **)&buf, true);
    if (length < 0) {
        TIKI_DPrintf("Tiki:LoadAnim Couldn't load %s\n", path);
        return qfalse;
    }

    pheader = (skelHeader_t *)TIKI_Alloc(length);
    memcpy(pheader, buf, length);
    TIKI_FreeFile(buf);
    memset(cache, 0, sizeof(skelcache_t));
    strncpy(cache->path, path, sizeof(cache->path));

    header = pheader->ident;
    if (header != TIKI_SKB_HEADER_IDENT) {
        TIKI_Error("TIKI_LoadSKB: Tried to load '%s' as a skeletal base frame (File has invalid header)\n", path);
        TIKI_Free(pheader);
        return qfalse;
    }

    version = LittleLong(pheader->version);
    if (version != TIKI_SKB_HEADER_VER_3 && version != TIKI_SKB_HEADER_VERSION) {
        TIKI_Error(
            "TIKI_LoadSKB: %s has wrong version (%i should be %i or %i)\n",
            path,
            version,
            TIKI_SKB_HEADER_VER_3,
            TIKI_SKB_HEADER_VERSION
        );
        TIKI_Free(pheader);
        return qfalse;
    }

    TIKI_SwapSkel(pheader);

    surf = (skelSurface_t *)((char *)pheader + pheader->ofsSurfaces);
    for (i = 0; i < pheader->numSurfaces; i++) {
        int numSurfVerts, numSurfTriangles;

        numSurfVerts     = LongNoSwapPtr(&surf->numVerts);
        numSurfTriangles = LongNoSwapPtr(&surf->numTriangles);

        if (numSurfVerts > TIKI_MAX_VERTEXES) {
            TIKI_Error(
                "TIKI_LoadSKB: %s has more than %i verts on a surface (%i)", path, TIKI_MAX_VERTEXES, numSurfVerts
            );
            TIKI_Free(pheader);
            return qfalse;
        }

        if (numSurfTriangles > TIKI_MAX_TRIANGLES) {
            TIKI_Error(
                "TIKI_LoadSKB: %s has more than %i triangles on a surface (%i)",
                path,
                TIKI_MAX_TRIANGLES,
                numSurfTriangles
            );
            TIKI_Free(pheader);
            return qfalse;
        }

        surf = (skelSurface_t *)((char *)surf + LongNoSwapPtr(&surf->ofsEnd));
    }

    if (pheader->numBones > TIKI_MAX_BONES) {
        TIKI_Error("TIKI_LoadSKB: %s has more than %i bones (%i)\n", path, TIKI_MAX_BONES, pheader->numBones);
        TIKI_Free(pheader);
        return qfalse;
    }

    totalVerts = 0;
    surf       = (skelSurface_t *)((byte *)pheader + pheader->ofsSurfaces);
    for (i = 0; i < pheader->numSurfaces; i++) {
        totalVerts += LongNoSwapPtr(&surf->numVerts);
        surf = (skelSurface_t *)((byte *)surf + LongNoSwapPtr(&surf->ofsEnd));
    }

    newLength = totalVerts * sizeof(unsigned int);
    newHeader = (skelHeader_t *)TIKI_Alloc(length + newLength);
    memcpy(newHeader, pheader, pheader->ofsSurfaces);

    if (newHeader->ofsBones > newHeader->ofsSurfaces) {
        newHeader->ofsBones += newLength;
    }
    if (newHeader->ofsBoxes > newHeader->ofsSurfaces) {
        newHeader->ofsBoxes += newLength;
    }
    if (newHeader->ofsEnd > newHeader->ofsSurfaces) {
        newHeader->ofsEnd += newLength;
    }

    oldSurf = (skelSurface_t *)((byte *)pheader + pheader->ofsSurfaces);
    newSurf = (skelSurface_t *)((byte *)newHeader + newHeader->ofsSurfaces);

    for (i = 0; i < pheader->numSurfaces; i++) {
        skeletorVertex_t *newVerts;
        skelVertex_t     *oldVerts;
        int               numVerts;

        memcpy(newSurf, oldSurf, oldSurf->ofsVerts);
        if (newSurf->ofsCollapse > LongNoSwapPtr(&newSurf->ofsVerts)) {
            int newOffset =
                LongNoSwapPtr(&newSurf->ofsCollapse) + sizeof(unsigned int) * LongNoSwapPtr(&newSurf->numVerts);
            memcpy(&newSurf->ofsCollapse, &newOffset, sizeof(newSurf->ofsCollapse));
        }
        if (newSurf->ofsCollapseIndex > LongNoSwapPtr(&newSurf->ofsVerts)) {
            int newOffset =
                LongNoSwapPtr(&newSurf->ofsCollapseIndex) + sizeof(unsigned int) * LongNoSwapPtr(&newSurf->numVerts);
            memcpy(&newSurf->ofsCollapseIndex, &newOffset, sizeof(newSurf->ofsCollapseIndex));
        }
        if (newSurf->ofsTriangles > LongNoSwapPtr(&newSurf->ofsVerts)) {
            int newOffset =
                LongNoSwapPtr(&newSurf->ofsTriangles) + sizeof(unsigned int) * LongNoSwapPtr(&newSurf->numVerts);
            memcpy(&newSurf->ofsTriangles, &newOffset, sizeof(newSurf->ofsTriangles));
        }
        if (newSurf->ofsEnd > LongNoSwapPtr(&newSurf->ofsVerts)) {
            int newOffset = LongNoSwapPtr(&newSurf->ofsEnd) + sizeof(unsigned int) * LongNoSwapPtr(&newSurf->numVerts);
            memcpy(&newSurf->ofsEnd, &newOffset, sizeof(newSurf->ofsEnd));
        }

        oldVerts = (skelVertex_t *)((byte *)oldSurf + oldSurf->ofsVerts);
        newVerts = (skeletorVertex_t *)((byte *)newSurf + newSurf->ofsVerts);

        numVerts = LongNoSwapPtr(&oldSurf->numVerts);
        for (j = 0; j < numVerts; j++) {
            int numMorphs, numWeights;

            memcpy(newVerts->normal, oldVerts->normal, sizeof(newVerts->normal));
            memcpy(newVerts->texCoords, oldVerts->texCoords, sizeof(newVerts->texCoords));

            numMorphs  = 0;
            numWeights = LongNoSwapPtr(&oldVerts->numWeights);
            memcpy(&newVerts->numMorphs, &numMorphs, sizeof(newVerts->numMorphs));
            memcpy(&newVerts->numWeights, &numWeights, sizeof(newVerts->numWeights));

            skelWeight_t *newWeights = (skelWeight_t *)((byte *)newVerts + sizeof(skeletorVertex_t));

            for (k = 0; k < numWeights; k++) {
                memcpy(newWeights, &oldVerts->weights[k], sizeof(skelWeight_t));
                newWeights++;
            }

            oldVerts = (skelVertex_t *)((byte *)oldVerts + sizeof(skelWeight_t) * numWeights
                                        + (sizeof(skelVertex_t) - sizeof(skelWeight_t)));
            newVerts =
                (skeletorVertex_t *)((byte *)newVerts + sizeof(skeletorVertex_t) + sizeof(skelWeight_t) * numWeights);
        }

        memcpy(newVerts, oldVerts, LongNoSwapPtr(&oldSurf->ofsEnd) - ((byte *)oldVerts - (byte *)oldSurf));
        oldSurf = (skelSurface_t *)((byte *)oldSurf + LongNoSwapPtr(&oldSurf->ofsEnd));
        newSurf = (skelSurface_t *)((byte *)newSurf + LongNoSwapPtr(&newSurf->ofsEnd));
    }

    memcpy(newSurf, oldSurf, pheader->ofsEnd - ((char *)oldSurf - (char *)pheader));
    TIKI_Free(pheader);
    TIKI_CacheFileSkel(newHeader, cache, length);
    TIKI_Free(newHeader);
    cache_numskel++;

    return qtrue;
}

/*
===============
SaveLODFile
===============
*/
void SaveLODFile(const char *path, lodControl_t *LOD)
{
    fileHandle_t file = FS_FOpenFileWrite(path);
    if (!file) {
        TIKI_Warning("SaveLODFile: Failed to open file %s\n", path);
        return;
    }

    FS_Write(LOD, sizeof(lodControl_t), file);
}

/*
===============
GetLODFile
===============
*/
void GetLODFile(skelcache_t *cache)
{
    lodControl_t      *LOD;
    char               pathLOD[256];
    char              *buf;
    int                i;
    skelSurfaceGame_t *pSurf;
    bool               bCanLod = false;
    char              *ext;
    int                length;

    pSurf = cache->skel->pSurfaces;

    for (i = 0; i < cache->skel->numSurfaces; i++) {
        if (pSurf->pCollapseIndex[0] != pSurf->pCollapseIndex[pSurf->numVerts - 1]) {
            bCanLod = true;
            break;
        }

        pSurf = pSurf->pNext;
    }

    if (!bCanLod) {
        return;
    }

    Q_strncpyz(pathLOD, cache->path, sizeof(pathLOD));
    ext = strstr(pathLOD, "skd");
    strcpy(ext, "lod");

    length = TIKI_ReadFileEx(pathLOD, (void **)&buf, true);
    if (length >= 0) {
        LOD = (lodControl_t *)TIKI_Alloc(sizeof(lodControl_t));
        memcpy(LOD, buf, length);
        TIKI_FreeFile(buf);

        LOD->minMetric = LittleFloat(LOD->minMetric);
        LOD->maxMetric = LittleFloat(LOD->maxMetric);

        for (i = 0; i < MAX_LOD_CURVE_POINTS; i++) {
            LOD->curve[i].pos = LittleFloat(LOD->curve[i].pos);
            LOD->curve[i].val = LittleFloat(LOD->curve[i].val);
        }

        for (i = 0; i < MAX_LOD_CURVE_CONSTS; i++) {
            LOD->consts[i].base   = LittleFloat(LOD->consts[i].base);
            LOD->consts[i].scale  = LittleFloat(LOD->consts[i].scale);
            LOD->consts[i].cutoff = LittleFloat(LOD->consts[i].cutoff);
        }
    } else {
        LOD               = (lodControl_t *)TIKI_Alloc(sizeof(lodControl_t));
        LOD->minMetric    = 1.0f;
        LOD->maxMetric    = 0.2f;
        LOD->curve[0].pos = 0.0f;
        LOD->curve[0].val = 0.0f;
        LOD->curve[1].pos = 0.5f;
        LOD->curve[1].val = (float)cache->skel->lodIndex[1];
        LOD->curve[2].pos = 0.8f;
        LOD->curve[2].val = (float)cache->skel->lodIndex[1];
        LOD->curve[3].pos = 0.95f;
        LOD->curve[3].val = (float)cache->skel->lodIndex[1];
        LOD->curve[4].pos = 1.0f;

        for (i = TIKI_SKEL_LOD_INDEXES; cache->skel->lodIndex[i] > cache->skel->lodIndex[3]; i--) {
            if (i <= 2) {
                break;
            }
        }

        LOD->curve[4].val = (float)cache->skel->lodIndex[i];

        for (i = 1; i < MAX_LOD_CURVE_POINTS - 1; i++) {
            if (LOD->curve[i].pos < LOD->curve[i - 1].pos) {
                LOD->curve[i].val = LOD->curve[i - 1].pos;
            }
        }
    }

    TIKI_CalcLodConsts(LOD);
    cache->skel->pLOD = LOD;
}

/*
===============
TIKI_CalcLodConsts
===============
*/
void TIKI_CalcLodConsts(lodControl_t *LOD)
{
    int i;

    for (i = 0; i < 4; i++) {
        float common =
            (LOD->curve[i + 1].val - LOD->curve[i + 0].val) / (LOD->curve[i + 1].pos - LOD->curve[i + 0].pos);

        LOD->consts[i].base = LOD->curve[i + 0].val
                            + (LOD->minMetric / (LOD->minMetric - LOD->maxMetric) - LOD->curve[i + 0].pos) * common;
        LOD->consts[i].scale  = common / (LOD->maxMetric - LOD->minMetric);
        LOD->consts[i].cutoff = LOD->minMetric + (LOD->maxMetric - LOD->minMetric) * LOD->curve[i + 0].pos;
    }
}

/*
===============
TIKI_LoadSKD
===============
*/
qboolean TIKI_LoadSKD(const char *path, skelcache_t *cache)
{
    int            i;
    skelHeader_t  *pheader;
    skelSurface_t *surf;
    int            version;
    unsigned int   header;
    int            length;

    length = TIKI_ReadFileEx(path, (void **)&pheader, qtrue);
    if (length < 0) {
        TIKI_DPrintf("Tiki:LoadAnim Couldn't load %s\n", path);
        return qfalse;
    }

    // The SKD must have at least one surface
    if (LittleLong(pheader->numSurfaces) <= 0) {
        TIKI_Error("^~^~^ TIKI_LoadSKD: %s has no surfaces\n", path);
        TIKI_FreeFile(pheader);
        return qfalse;
    }

    memset(cache, 0, sizeof(skelcache_t));
    strncpy(cache->path, path, sizeof(cache->path));

    // Check the signature
    header = pheader->ident;
    if (header != TIKI_SKD_HEADER_IDENT) {
        TIKI_Error("TIKI_LoadSKD: Tried to load '%s' as a skeletal base frame (File has invalid header)\n", path);
        TIKI_FreeFile(pheader);
        return qfalse;
    }

    // Check the version
    version = LittleLong(pheader->version);
    if (version != TIKI_SKD_HEADER_OLD_VERSION && version != TIKI_SKD_HEADER_VERSION) {
        TIKI_Error("TIKI_LoadSKD: %s has wrong version (%i should be %i)\n", path, version, TIKI_SKD_HEADER_VERSION);
        TIKI_FreeFile(pheader);
        return qfalse;
    }

    TIKI_SwapSkel(pheader);

    surf = (skelSurface_t *)((byte *)pheader + pheader->ofsSurfaces);

    for (i = 0; i < pheader->numSurfaces; i++) {
        if (surf->numVerts > TIKI_MAX_VERTEXES) {
            TIKI_Error(
                "TIKI_LoadSKD: %s has more than %i verts on a surface (%i)", path, TIKI_MAX_VERTEXES, surf->numVerts
            );
            TIKI_FreeFile(pheader);
            return qfalse;
        }

        if (surf->numTriangles > TIKI_MAX_TRIANGLES) {
            TIKI_Error(
                "TIKI_LoadSKD: %s has more than %i triangles on a surface (%i)",
                path,
                TIKI_MAX_TRIANGLES,
                surf->numTriangles
            );
            TIKI_FreeFile(pheader);
            return qfalse;
        }

        surf = (skelSurface_t *)((byte *)surf + surf->ofsEnd);
    }

    if (pheader->numBones > TIKI_MAX_BONES) {
        TIKI_FreeFile(pheader);
        TIKI_Error("TIKI_LoadSKD: %s has more than %i bones (%i)\n", path, TIKI_MAX_BONES, pheader->numBones);
        return qfalse;
    }

    // Cache the skeleton
    TIKI_CacheFileSkel(pheader, cache, length);
    TIKI_FreeFile(pheader);
    cache_numskel++;

    // Get the lod file associated with the skeleton
    GetLODFile(cache);
    TIKI_SortLOD(cache->skel);

    return true;
}

/*
===============
TIKI_GetSkel
===============
*/
skelHeaderGame_t *TIKI_GetSkel(int index)
{
    return skelcache[index].skel;
}

/*
===============
TIKI_GetSkelCache
===============
*/
int TIKI_GetSkelCache(skelHeaderGame_t *pSkel)
{
    for (int index = 0; index < cache_maxskel; index++) {
        if (skelcache[index].skel == pSkel) {
            return index;
        }
    }

    return -1;
}

/*
===============
TIKI_FreeSkel
===============
*/
void TIKI_FreeSkel(int index)
{
    if (index < 0 || index > cache_maxskel) {
        TIKI_Error("TIKI_FreeSkel: handle %d out of range\n", index);
        return;
    }

    if (!skelcache[index].skel) {
        TIKI_Error("TIKI_FreeSkel: NULL pointer for %s\n", skelcache[index].path);
        return;
    }

    TIKI_FreeSkelCache(&skelcache[index]);
    cache_numskel--;
}

/*
===============
TIKI_FreeSkelCache
===============
*/
void TIKI_FreeSkelCache(skelcache_t *cache)
{
    skelSurfaceGame_t *pSurf;

    if (cache->skel == NULL) {
        return;
    }

    for (pSurf = cache->skel->pSurfaces; pSurf != NULL; pSurf = pSurf->pNext) {
        if (pSurf->pStaticXyz) {
            TIKI_Free(pSurf->pStaticXyz);
        }
    }

    if (cache->skel->pLOD) {
        TIKI_Free(cache->skel->pLOD);
    }

    TIKI_Free(cache->skel);
    cache->skel    = NULL;
    cache->size    = 0;
    cache->path[0] = 0;
    cache->numuses = 0;
}

/*
===============
TIKI_FindSkel
===============
*/
skelcache_t *TIKI_FindSkel(const char *path)
{
    int          i;
    int          num;
    skelcache_t *cache;

    num = cache_numskel;

    for (i = 0; i < TIKI_MAX_SKELCACHE; i++) {
        cache = &skelcache[i];

        if (!cache->skel) {
            continue;
        }

        if (!strcmp(path, cache->path)) {
            return cache;
        }

        if (num == 0) {
            break;
        }
        num--;
    }

    return NULL;
}

/*
===============
TIKI_FindSkelByHeader

Added in OPM
Return the skelcache associated with the specified skelmodel
===============
*/
skelcache_t *TIKI_FindSkelByHeader(skelHeaderGame_t *skelmodel)
{
    int          i;
    int          num;
    skelcache_t *cache;

    num = cache_numskel;

    for (i = 0; i < TIKI_MAX_SKELCACHE; i++) {
        cache = &skelcache[i];

        if (!cache->skel) {
            continue;
        }

        if (cache->skel == skelmodel) {
            return cache;
        }

        if (num == 0) {
            break;
        }
        num--;
    }

    return NULL;
}

/*
===============
TIKI_FindFreeSkel
===============
*/
skelcache_t *TIKI_FindFreeSkel(void)
{
    int          i;
    skelcache_t *cache;

    for (i = 0; i < TIKI_MAX_SKELCACHE; i++) {
        cache = &skelcache[i];

        if (!cache->skel) {
            return cache;
        }
    }

    return NULL;
}

/*
===============
TIKI_RegisterSkel
===============
*/
int TIKI_RegisterSkel(const char *path, dtiki_t *tiki)
{
    char         tempName[257];
    skelcache_t *cache;
    const char  *extension;

    // Find a skel with the same name
    cache = TIKI_FindSkel(path);
    if (cache) {
        return cache - skelcache;
    }

    // Find a free skel
    cache = TIKI_FindFreeSkel();
    if (!cache) {
        TIKI_Error("TIKI_RegisterSkel: No free spots open in skel cache for %s\n", path);
        return -1;
    }
    
    cache->numuses = 0;

    if (cache - skelcache + 1 > cache_maxskel) {
        cache_maxskel = cache - skelcache + 1;
    }

    extension = TIKI_FileExtension(path);
    if (!strcmp(extension, "skb")) {
        if (!TIKI_LoadSKB(path, cache)) {
            return -1;
        }
    } else if (!strcmp(extension, "skd")) {
        if (!TIKI_LoadSKD(path, cache)) {
            return -1;
        }
    } else {
        TIKI_Error("TIKI_RegisterSkel: Unknown extension %s\n", extension);
        return -1;
    }

    Com_sprintf(tempName, sizeof(tempName), "l%s", path);
    UI_LoadResource(tempName);

    return cache - skelcache;
}
