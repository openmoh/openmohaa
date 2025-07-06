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

// tiki_anim.cpp : TIKI Anim

#include "q_shared.h"
#include "qcommon.h"
#include "../skeletor/skeletor.h"
#include <mem_blockalloc.h>
#include <con_set.h>
#include "tiki_files.h"

/*
===============
AnimCompareFunc
===============
*/
static void *context;

static int AnimCompareFunc(const void *a, const void *b)
{
    dloaddef_t *ld = (dloaddef_t *)context;
    return Q_stricmp(ld->loadanims[*(int *)a]->alias, ld->loadanims[*(int *)b]->alias);
}

/*
===============
TIKI_GetAnimOrder
===============
*/
void TIKI_GetAnimOrder(dloaddef_t *ld, int *order)
{
    int i;

    for (i = 0; i < ld->numanims; i++) {
        order[i] = i;
    }
    context = ld;
    qsort(order, ld->numanims, sizeof(int), AnimCompareFunc);
}

/*
===============
TIKI_Anim_NameForNum
===============
*/
const char *TIKI_Anim_NameForNum(dtiki_t *pmdl, int animnum)
{
    dtikianimdef_t *panimdef;

    if (!pmdl || !pmdl->a) {
        return NULL;
    }

    if (animnum < 0 || animnum >= pmdl->a->num_anims) {
        return NULL;
    }

    panimdef = pmdl->a->animdefs[animnum];
    return panimdef->alias;
}

/*
===============
TIKI_Anim_NumForName
===============
*/
int TIKI_Anim_NumForName(dtiki_t *pmdl, const char *name)
{
    int             iTop;
    int             iBottom;
    int             iMiddle;
    int             iComp;
    dtikianimdef_t *panimdef;
    float           fAnimWeights[MAX_FRAMEINFOS];
    float           fWeight;
    float           fTotalWeight;
    int             iAnimCount;
    int             i;

    if (!pmdl) {
        return -1;
    }

    iBottom = 0;
    iTop    = pmdl->a->num_anims - 1;

    while (iBottom <= iTop) {
        iMiddle = (iBottom + iTop) / 2;

        panimdef = pmdl->a->animdefs[iMiddle];
        iComp    = Q_stricmp(panimdef->alias, name);

        if (!iComp) {
            if (!(panimdef->flags & TAF_RANDOM)) {
                return iMiddle;
            }

            for (iTop = iMiddle; iTop > 0; iTop--) {
                assert(pmdl->a->animdefs[iTop - 1]);
                if (Q_stricmp(panimdef->alias, pmdl->a->animdefs[iTop - 1]->alias)) {
                    break;
                }
            }

            for (iBottom = iMiddle; iBottom < pmdl->a->num_anims - 1; iBottom++) {
                assert(pmdl->a->animdefs[iBottom + 1]);
                if (Q_stricmp(panimdef->alias, pmdl->a->animdefs[iBottom + 1]->alias)) {
                    break;
                }
            }

            fTotalWeight = 0.0f;
            iAnimCount   = 0;

            for (i = iTop; i <= iBottom; i++) {
                panimdef = pmdl->a->animdefs[i];
                if (!panimdef) {
                    continue;
                }

                if (panimdef->flags & TAF_AUTOSTEPS) {
                    fAnimWeights[iAnimCount] = 0.0f;
                    panimdef->flags &= ~TAF_AUTOSTEPS;
                } else {
                    fAnimWeights[iAnimCount] = panimdef->weight;
                    fTotalWeight += panimdef->weight;
                }

                iAnimCount++;
            }

            fWeight = randweight() * fTotalWeight;
            for (i = 0; i < iAnimCount; i++) {
                fWeight -= fAnimWeights[i];
                if (fWeight <= 0) {
                    break;
                }
            }

            panimdef = pmdl->a->animdefs[iTop + i];
            if (panimdef && panimdef->flags & TAF_NOREPEAT) {
                panimdef->flags |= TAF_AUTOSTEPS;
            }

            return iTop + i;
        }

        if (iComp > 0) {
            iTop = iMiddle - 1;
        } else {
            iBottom = iMiddle + 1;
        }
    }

    return -1;
}

/*
===============
TIKI_Anim_Random
===============
*/
int TIKI_Anim_Random(dtiki_t *pmdl, const char *name)
{
    dtikianimdef_t *panimdef;
    int             i;
    float           totalweight;
    float           weights[MAX_FRAMEINFOS];
    int             anim[MAX_FRAMEINFOS];
    int             num;
    size_t          len;
    int             diff;
    float           weight;

    len = strlen(name);
    if (!len || !pmdl) {
        return -1;
    }

    num         = 0;
    totalweight = 0.0f;
    for (i = 0; i < pmdl->a->num_anims; i++) {
        panimdef = pmdl->a->animdefs[i];
        diff     = strnicmp(panimdef->alias, name, len);
        if (diff || panimdef->alias[len] == '_') {
            if (diff > 0) {
                break;
            }
        } else {
            if (num >= MAX_FRAMEINFOS) {
                break;
            }

            totalweight += panimdef->weight;
            anim[num]    = i;
            weights[num] = panimdef->weight;

            num++;
        }
    }

    // animation name not found
    if (!num) {
        return -1;
    }

    // find a random animation based on the weight
    weight = randweight() * totalweight;
    for (i = 0; i < num; i++) {
        if (weight < weights[i]) {
            break;
        }

        weight -= weights[i];
    }

    return anim[i];
}

/*
===============
TIKI_Anim_NumFrames
===============
*/
int TIKI_Anim_NumFrames(dtiki_t *pmdl, int animnum)
{
    if (!pmdl) {
        return 0;
    }

    if (!pmdl->a || animnum < 0 || animnum >= pmdl->a->num_anims) {
        return 0;
    }

    skelAnimDataGameHeader_t *animData = SkeletorCacheGetData(pmdl->a->m_aliases[animnum]);
    return animData->numFrames;
}

/*
===============
TIKI_Anim_Time
===============
*/
float TIKI_Anim_Time(dtiki_t *pmdl, int animnum)
{
    if (!pmdl) {
        return 0.0;
    }

    if (!pmdl->a || animnum < 0 || animnum >= pmdl->a->num_anims) {
        return 0.0;
    }

    skelAnimDataGameHeader_t *animData = SkeletorCacheGetData(pmdl->a->m_aliases[animnum]);

    return animData->flags & TAF_DELTADRIVEN ? animData->frameTime * animData->numFrames
                                             : animData->frameTime * (animData->numFrames - 1);
}

/*
===============
TIKI_Anim_Frametime
===============
*/
float TIKI_Anim_Frametime(dtiki_t *pmdl, int animnum)
{
    if (!pmdl) {
        return 0.0;
    }

    if (!pmdl->a || animnum < 0 || animnum >= pmdl->a->num_anims) {
        return 0.0;
    }

    skelAnimDataGameHeader_t *animData = SkeletorCacheGetData(pmdl->a->m_aliases[animnum]);
    return animData->frameTime;
}

/*
===============
TIKI_Anim_Delta
===============
*/
void TIKI_Anim_Delta(dtiki_t *pmdl, int animnum, float *delta)
{
    if (!pmdl) {
        VectorClear(delta);
        return;
    }

    if (!pmdl->a || animnum < 0 || animnum >= pmdl->a->num_anims) {
        VectorClear(delta);
        return;
    }

    skelAnimDataGameHeader_t *animData = SkeletorCacheGetData(pmdl->a->m_aliases[animnum]);
    VectorScale(animData->totalDelta, pmdl->load_scale, delta);
}

/*
===============
TIKI_Anim_AngularDelta

Added in 2.0
===============
*/
void TIKI_Anim_AngularDelta(dtiki_t *pmdl, int animnum, float *delta)
{
    if (!pmdl) {
        *delta = 0;
        return;
    }

    if (!pmdl->a || animnum < 0 || animnum >= pmdl->a->num_anims) {
        *delta = 0;
        return;
    }

    skelAnimDataGameHeader_t *animData = SkeletorCacheGetData(pmdl->a->m_aliases[animnum]);
    *delta                             = animData->totalAngleDelta;
}

/*
===============
TIKI_Anim_HasDelta
===============
*/
qboolean TIKI_Anim_HasDelta(dtiki_t *pmdl, int animnum)
{
    if (!pmdl) {
        return qfalse;
    }

    if (!pmdl->a || animnum < 0 || animnum >= pmdl->a->num_anims) {
        return qfalse;
    }

    skelAnimDataGameHeader_t *animData = SkeletorCacheGetData(pmdl->a->m_aliases[animnum]);
    return animData->bHasDelta;
}

/*
===============
TIKI_Anim_DeltaOverTime
===============
*/
void TIKI_Anim_DeltaOverTime(dtiki_t *pTiki, int iAnimnum, float fTime1, float fTime2, vec3_t vDelta)
{
    int                       realAnimIndex;
    skelAnimDataGameHeader_t *animData;
    SkelVec3                  absDelta;

    if (!pTiki || !pTiki->a) {
        return;
    }

    if (iAnimnum >= pTiki->a->num_anims || iAnimnum < 0) {
        return;
    }

    if (fTime2 < fTime1) {
        return;
    }

    realAnimIndex = pTiki->a->m_aliases[iAnimnum];
    if (realAnimIndex != -1) {
        animData = SkeletorCacheGetData(realAnimIndex);
        absDelta = animData->GetDeltaOverTime(fTime1, fTime2);
        VectorScale(absDelta, pTiki->load_scale, vDelta);
    } else {
        TIKI_Error("Skeletor GetDeltaOverTime: Couldn't find animation with index %i\n", iAnimnum);
        VectorClear(vDelta);
    }
}

/*
===============
TIKI_Anim_AngularDeltaOverTime
===============
*/
void TIKI_Anim_AngularDeltaOverTime(dtiki_t *pTiki, int iAnimnum, float fTime1, float fTime2, float *fDelta)
{
    int                       realAnimIndex;
    skelAnimDataGameHeader_t *animData;

    *fDelta = 0;

    if (!pTiki || !pTiki->a) {
        return;
    }

    if (iAnimnum >= pTiki->a->num_anims || iAnimnum < 0) {
        return;
    }

    if (fTime2 < fTime1) {
        return;
    }

    realAnimIndex = pTiki->a->m_aliases[iAnimnum];
    if (realAnimIndex != -1) {
        animData = SkeletorCacheGetData(realAnimIndex);
        *fDelta  = animData->GetAngularDeltaOverTime(fTime1, fTime2);
    } else {
        TIKI_Error("Skeletor GetAngularDeltaOverTime: Couldn't find animation with index %i\n", iAnimnum);
        *fDelta = 0;
    }
}

/*
===============
TIKI_Anim_Flags
===============
*/
int TIKI_Anim_Flags(dtiki_t *pmdl, int animnum)
{
    if (!pmdl) {
        return 0;
    }

    if (!pmdl->a || animnum < 0 || animnum >= pmdl->a->num_anims) {
        return 0;
    }

    dtikianimdef_t *panimdef = pmdl->a->animdefs[animnum];
    return panimdef->flags;
}

/*
===============
TIKI_Anim_FlagsSkel
===============
*/
int TIKI_Anim_FlagsSkel(dtiki_t *pmdl, int animnum)
{
    if (!pmdl) {
        return 0;
    }

    if (!pmdl->a || animnum < 0 || animnum >= pmdl->a->num_anims) {
        return 0;
    }

    skelAnimDataGameHeader_t *animData;
    int                       flags;

    animData = SkeletorCacheGetData(pmdl->a->m_aliases[animnum]);
    flags    = animData->flags;

    if (animData->bHasDelta) {
        flags |= TAF_HASDELTA;
    }

    if (animData->bHasMorph) {
        flags |= TAF_HASMORPH;
    }

    if (animData->bHasUpper) {
        flags |= TAF_HASUPPER;
    }

    return flags;
}

/*
===============
TIKI_Anim_HasServerCommands
===============
*/
qboolean TIKI_Anim_HasServerCommands(dtiki_t *pmdl, int animnum)
{
    if (!pmdl) {
        return qfalse;
    }

    if (!pmdl->a || animnum < 0 || animnum >= pmdl->a->num_anims) {
        return qfalse;
    }

    dtikianimdef_t *panimdef = pmdl->a->animdefs[animnum];
    return panimdef->num_server_cmds > 0;
}

/*
===============
TIKI_Anim_HasClientCommands
===============
*/
qboolean TIKI_Anim_HasClientCommands(dtiki_t *pmdl, int animnum)
{
    if (!pmdl) {
        return qfalse;
    }

    if (!pmdl->a || animnum < 0 || animnum >= pmdl->a->num_anims) {
        return qfalse;
    }

    dtikianimdef_t *panimdef = pmdl->a->animdefs[animnum];
    return panimdef->num_client_cmds > 0;
}

/*
===============
TIKI_Anim_CrossblendTime
===============
*/
float TIKI_Anim_CrossblendTime(dtiki_t *pmdl, int animnum)
{
    if (!pmdl) {
        return 0.0;
    }

    if (!pmdl->a || animnum < 0 || animnum >= pmdl->a->num_anims) {
        return 0.0;
    }

    dtikianimdef_t *panimdef = pmdl->a->animdefs[animnum];
    return panimdef->blendtime;
}
