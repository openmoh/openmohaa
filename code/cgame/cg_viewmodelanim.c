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
// FPS view model animation

#include "cg_local.h"
#include "tiki.h"

static const char* AnimPrefixList[] =
{
    "papers",
    "colt45",
    "p38",
    "histandard",
    "garand",
    "kar98",
    "kar98sniper",
    "springfield",
    "thompson",
    "mp40",
    "bar",
    "mp44",
    "fraggrenade",
    "stielhandgranate",
    "bazooka",
    "panzerschreck",
    "shotgun",
    "unarmed"
};

enum animPrefix_e
{
    WPREFIX_PAPERS,
    WPREFIX_COLT45,
    WPREFIX_P38,
    WPREFIX_HISTANDARD,
    WPREFIX_GARAND,
    WPREFIX_KAR98,
    WPREFIX_KAR98SNIPER,
    WPREFIX_SPRINGFIELD,
    WPREFIX_THOMPSON,
    WPREFIX_MP40,
    WPREFIX_BAR,
    WPREFIX_MP44,
    WPREFIX_FRAGGRENADE,
    WPREFIX_STIELHANDGRANATE,
    WPREFIX_BAZOOKA,
    WPREFIX_PANZERSCHRECK,
    WPREFIX_SHOTGUN,
    WPREFIX_UNARMED
};

int CG_GetVMAnimPrefixIndex()
{
    const char* szWeaponName;
    int iWeaponClass;

    iWeaponClass = cg.snap->ps.stats[STAT_EQUIPPED_WEAPON];
    szWeaponName = CG_ConfigString(CS_WEAPONS + cg.snap->ps.activeItems[1]);

    if (iWeaponClass & WEAPON_CLASS_ANY_ITEM)
    {
        if (!Q_stricmp(szWeaponName, "Papers"))
            return WPREFIX_PAPERS;
    }
    else if (iWeaponClass & WEAPON_CLASS_PISTOL)
    {
        if (!Q_stricmp(szWeaponName, "Colt 45"))
            return WPREFIX_COLT45;
        if (!Q_stricmp(szWeaponName, "Walther P38"))
            return WPREFIX_P38;
        if (!Q_stricmp(szWeaponName, "Hi-Standard Silenced"))
            return WPREFIX_HISTANDARD;
    }
    else if (iWeaponClass & WEAPON_CLASS_RIFLE)
    {
        if (!Q_stricmp(szWeaponName, "M1 Garand"))
            return WPREFIX_GARAND;
        if (!Q_stricmp(szWeaponName, "Mauser KAR 98K"))
            return WPREFIX_KAR98;
        if (!Q_stricmp(szWeaponName, "KAR98 - Sniper"))
            return WPREFIX_KAR98SNIPER;
        if (!Q_stricmp(szWeaponName, "Springfield '03 Sniper"))
            return WPREFIX_SPRINGFIELD;
    }
    else if (iWeaponClass & WEAPON_CLASS_SMG)
    {
        if (!Q_stricmp(szWeaponName, "Thompson"))
            return WPREFIX_THOMPSON;
        if (!Q_stricmp(szWeaponName, "MP40"))
            return WPREFIX_MP40;
    }
    else if (iWeaponClass & WEAPON_CLASS_MG)
    {
        if (!Q_stricmp(szWeaponName, "BAR"))
            return WPREFIX_BAR;
        if (!Q_stricmp(szWeaponName, "StG 44"))
            return WPREFIX_MP44;
    }
    else if (iWeaponClass & WEAPON_CLASS_GRENADE)
    {
        if (!Q_stricmp(szWeaponName, "Frag Grenade"))
            return WPREFIX_FRAGGRENADE;
        if (!Q_stricmp(szWeaponName, "Stielhandgranate"))
            return WPREFIX_STIELHANDGRANATE;
    }
    else if (iWeaponClass & WEAPON_CLASS_HEAVY)
    {
        if (!Q_stricmp(szWeaponName, "Bazooka"))
            return WPREFIX_BAZOOKA;
        if (!Q_stricmp(szWeaponName, "Panzerschreck"))
            return WPREFIX_PANZERSCHRECK;
        if (!Q_stricmp(szWeaponName, "Shotgun"))
            return WPREFIX_SHOTGUN;
    }

    return WPREFIX_UNARMED;
}

void CG_ViewModelAnimation(refEntity_t* pModel)
{
    int i;
    int iAnimFlags;
    float fCrossblendTime, fCrossblendFrac, fCrossblendAmount;
    float fAnimLength;
    int iAnimPrefixIndex;
    const char* pszAnimSuffix;
    char szAnimName[MAX_QPATH];
    dtiki_t* pTiki;
    qboolean bAnimChanged;

    fCrossblendFrac = 0.0;
    bAnimChanged = 0;
    pTiki = pModel->tiki;

    if (cgi.anim->g_iLastEquippedWeaponStat == cg.snap->ps.stats[STAT_EQUIPPED_WEAPON]
        && !strcmp(cgi.anim->g_szLastActiveItem, CG_ConfigString(CS_WEAPONS + cg.snap->ps.activeItems[1])))
    {
        iAnimPrefixIndex = cgi.anim->g_iLastAnimPrefixIndex;
    }
    else
    {
        iAnimPrefixIndex = CG_GetVMAnimPrefixIndex();
        cgi.anim->g_iLastEquippedWeaponStat = cg.snap->ps.stats[STAT_EQUIPPED_WEAPON];
        strcpy(cgi.anim->g_szLastActiveItem, CG_ConfigString(CS_WEAPONS + cg.snap->ps.activeItems[1]));
        cgi.anim->g_iLastAnimPrefixIndex = iAnimPrefixIndex;
        bAnimChanged = 1;
    }

    if (cgi.anim->g_iLastVMAnim == -1)
    {
        sprintf(szAnimName, "%s_idle", AnimPrefixList[iAnimPrefixIndex]);
        cgi.anim->g_VMFrameInfo[cgi.anim->g_iCurrentVMAnimSlot].index = cgi.Anim_NumForName(pTiki, szAnimName);

        if (cgi.anim->g_VMFrameInfo[cgi.anim->g_iCurrentVMAnimSlot].index == -1)
        {
            cgi.anim->g_VMFrameInfo[cgi.anim->g_iCurrentVMAnimSlot].index = cgi.Anim_NumForName(pTiki, "idle");
            cgi.DPrintf("Warning: Couldn't find view model animation %s\n", szAnimName);
        }

        cgi.anim->g_VMFrameInfo[cgi.anim->g_iCurrentVMAnimSlot].time = 0.0;
        cgi.anim->g_VMFrameInfo[cgi.anim->g_iCurrentVMAnimSlot].weight = 1.0;
        cgi.anim->g_iLastVMAnim = 0;
    }

    if (cg.snap->ps.iViewModelAnimChanged != cgi.anim->g_iLastVMAnimChanged)
    {
        bAnimChanged = 1;
        cgi.anim->g_iLastVMAnim = cg.snap->ps.iViewModelAnim;
        cgi.anim->g_iLastVMAnimChanged = cg.snap->ps.iViewModelAnimChanged;
    }

    if (bAnimChanged)
    {
        switch (cgi.anim->g_iLastVMAnim)
        {
        case 1:
            pszAnimSuffix = "charge";
            break;
        case 2:
            pszAnimSuffix = "fire";
            break;
        case 3:
            pszAnimSuffix = "fire_secondary";
            break;
        case 4:
            pszAnimSuffix = "rechamber";
            break;
        case 5:
            pszAnimSuffix = "reload";
            break;
        case 6:
            pszAnimSuffix = "reload_single";
            break;
        case 7:
            pszAnimSuffix = "reload_end";
            break;
        case 8:
            pszAnimSuffix = "pullout";
            break;
        case 9:
            pszAnimSuffix = "putaway";
            break;
        case 0xA:
            pszAnimSuffix = "ladderstep";
            break;
        default:
            pszAnimSuffix = "idle";
            break;
        }

        sprintf(szAnimName, "%s_%s", AnimPrefixList[iAnimPrefixIndex], pszAnimSuffix);
        fCrossblendTime = cgi.Anim_CrossblendTime(pTiki, cgi.anim->g_VMFrameInfo[cgi.anim->g_iCurrentVMAnimSlot].index);
        fCrossblendAmount = cgi.anim->g_iCurrentVMDuration / 1000.0;

        if (fCrossblendAmount < fCrossblendTime && fCrossblendAmount > 0.0)
        {
            fCrossblendFrac = fCrossblendAmount / fCrossblendTime;
            for (i = 0; i < MAX_FRAMEINFOS; ++i)
            {
                if (cgi.anim->g_VMFrameInfo[i].weight)
                {
                    if (i == cgi.anim->g_iCurrentVMAnimSlot) {
                        cgi.anim->g_VMFrameInfo[i].weight = fCrossblendFrac;
                    }
                    else {
                        cgi.anim->g_VMFrameInfo[i].weight = (1.0 - fCrossblendFrac) * cgi.anim->g_VMFrameInfo[i].weight;
                    }
                }
            }
        }

        cgi.anim->g_iCurrentVMAnimSlot = (cgi.anim->g_iCurrentVMAnimSlot + 1) % MAX_FRAMEINFOS;
        cgi.anim->g_VMFrameInfo[cgi.anim->g_iCurrentVMAnimSlot].index = cgi.Anim_NumForName(pTiki, szAnimName);

        if (cgi.anim->g_VMFrameInfo[cgi.anim->g_iCurrentVMAnimSlot].index == -1)
        {
            cgi.anim->g_VMFrameInfo[cgi.anim->g_iCurrentVMAnimSlot].index = cgi.Anim_NumForName(pTiki, "idle");
            cgi.DPrintf("Warning: Couldn't find view model animation %s\n", szAnimName);
        }

        cgi.anim->g_VMFrameInfo[cgi.anim->g_iCurrentVMAnimSlot].time = 0.0;
        cgi.anim->g_VMFrameInfo[cgi.anim->g_iCurrentVMAnimSlot].weight = 1.0;
        cgi.anim->g_iCurrentVMDuration = 0;

        if (cgi.Anim_CrossblendTime(pTiki, cgi.anim->g_VMFrameInfo[cgi.anim->g_iCurrentVMAnimSlot].index) == 0.0)
        {
            for (i = 0; i < MAX_FRAMEINFOS; ++i)
            {
                if (i != cgi.anim->g_iCurrentVMAnimSlot) {
                    cgi.anim->g_VMFrameInfo[i].weight = 0.0;
                }
            }

            cgi.anim->g_bCrossblending = qfalse;
        }
        else
        {
            cgi.anim->g_bCrossblending = qtrue;
        }
    }

    cgi.anim->g_iCurrentVMDuration += cg.frametime;
    if (cgi.anim->g_bCrossblending)
    {
        fCrossblendTime = cgi.Anim_CrossblendTime(pTiki, cgi.anim->g_VMFrameInfo[cgi.anim->g_iCurrentVMAnimSlot].index);
        fCrossblendAmount = cgi.anim->g_iCurrentVMDuration / 1000.0;
        if (fCrossblendAmount >= fCrossblendTime || fCrossblendAmount < 0.0) {
            for (i = 0; i < MAX_FRAMEINFOS; ++i)
            {
                if (i != cgi.anim->g_iCurrentVMAnimSlot) {
                    cgi.anim->g_VMFrameInfo[i].weight = 0.0;
                }
            }
            cgi.anim->g_bCrossblending = 0;
        }
        else {
            fCrossblendFrac = fCrossblendAmount / fCrossblendTime;
        }
    }

    for (i = 0; i < 16; ++i)
    {
        if (cgi.anim->g_VMFrameInfo[i].weight == 0.0)
        {
            pModel->frameInfo[i].index = 0;
            pModel->frameInfo[i].time = 0.0;
            pModel->frameInfo[i].weight = 0.0;
        }
        else
        {
            fAnimLength = cgi.Anim_Time(pTiki, cgi.anim->g_VMFrameInfo[i].index);
            cgi.anim->g_VMFrameInfo[i].time = cg.frametime / 1000.0 + cgi.anim->g_VMFrameInfo[i].time;

            if (cgi.anim->g_VMFrameInfo[i].time < fAnimLength)
            {
                if (cgi.Anim_Flags(pTiki, cgi.anim->g_VMFrameInfo[i].index) & TAF_DELTADRIVEN) {
                    cgi.anim->g_VMFrameInfo[i].time = cgi.anim->g_VMFrameInfo[i].time - fAnimLength;
                }
                else {
                    cgi.anim->g_VMFrameInfo[i].time = fAnimLength;
                }
            }

            pModel->frameInfo[i].index = cgi.anim->g_VMFrameInfo[i].index;
            pModel->frameInfo[i].time = cgi.anim->g_VMFrameInfo[i].time;

            if (cgi.anim->g_bCrossblending)
            {
                if (i == cgi.anim->g_iCurrentVMAnimSlot)
                    pModel->frameInfo[i].weight = fCrossblendFrac;
                else
                    pModel->frameInfo[i].weight = (1.0 - fCrossblendFrac) * cgi.anim->g_VMFrameInfo[i].weight;
            }
            else
            {
                pModel->frameInfo[i].weight = 1.0;
            }
        }
    }

    pModel->actionWeight = 1.0;
}

void CG_CalcViewModelMovement(float fViewBobPhase, float fViewBobAmp, vec_t* vVelocity, vec_t* vMovement)
{
    // FIXME: unimplemented
}
