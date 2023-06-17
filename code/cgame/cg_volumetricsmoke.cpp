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
// Volumetric smoke A.K.A VSS sources

#include "cg_local.h"
#include "cg_commands.h"

const char* cg_vsstypes[] =
{
    "default",
    "gun",
    "bulletimpact",
    "bulletdirtimpact",
    "heavy",
    "steam",
    "mist",
    "smokegrenade",
    "grenade",
    "fire",
    "greasefire",
    "debris"
};

cvssource_t* vss_sorttable[16384];

static int lastVSSFrameTime;
static constexpr float MAX_VSS_COORDS = 8096.0;
static constexpr float MAX_VSS_WIND_DIST = 512;
static constexpr float MAX_VSS_WIND_DIST_SQUARED = MAX_VSS_WIND_DIST * MAX_VSS_WIND_DIST;

extern cvar_t* cg_detail;
extern cvar_t* cg_effectdetail;
cvar_t *vss_draw;
cvar_t *vss_physics_fps;
cvar_t *vss_repulsion_fps;
cvar_t *vss_maxcount;
cvar_t *vss_color;
cvar_t *vss_showsources;
cvar_t *vss_wind_x;
cvar_t *vss_wind_y;
cvar_t *vss_wind_z;
cvar_t *vss_wind_strength;
cvar_t *vss_movement_dampen;
cvar_t *vss_maxvisible;
cvar_t *vss_gridsize;
cvar_t *vss_default_r;
cvar_t *vss_default_g;
cvar_t *vss_default_b;
cvar_t* vss_lighting_fps;

void VSS_ClampAlphaLife(cvssource_t* pSource, int maxlife);

void VSS_AddRepulsion(cvssource_t* pA, cvssource_t* pB)
{
    vec3_t vPush;
    float fDist, fForce, f;

    VectorSubtract(pA->newOrigin, pB->newOrigin, vPush);

    if (vPush[0] || vPush[1] || vPush[2])
    {
        fDist = VectorNormalize(vPush);
        f = fDist - pB->newRadius;

        if (f > 0.0)
        {
            f *= pA->ooRadius;
            if (f > 1.49) {
                f = 0.0;
            }
            else {
                f = f * (f * 0.0161 + -0.3104) + 1.2887;
            }

            if (f < 0.0) {
                f = f * 1.1;
            }

            fForce = f;
        }
        else
        {
            fForce = 1.0;
        }

        f = fDist - pA->newRadius;
        if (f > 0.0)
        {
            f *= pB->ooRadius;
            if (f > 1.49) {
                f = 0.0;
            }
            else {
                f = f * (f * 0.0161 + -0.3104) + 1.2887;
            }

            if (f < 0.0) {
                f = f * 1.1;
            }

            fForce += f;
        }
        else
        {
            fForce += 1.0;
        }

        if (fForce <= -0.05 && fForce >= 0.05)
        {
            fForce = (pA->newRadius + pB->newRadius) * 0.03 * fForce;
            VectorScale(vPush, fForce, vPush);

            VectorAdd(pA->repulsion, vPush, pA->repulsion);
            VectorSubtract(pB->repulsion, vPush, pB->repulsion);
        }
    }
    else
    {
        VectorSet(vPush, crandom(), crandom(), crandom());
        VectorAdd(pA->repulsion, vPush, pA->repulsion);
        VectorSubtract(pB->repulsion, vPush, pB->repulsion);
    }
}

cvssource_t* ClientGameCommandManager::AllocateVSSSource()
{
    cvssource_t* pNew;

    if (!m_free_vsssources) {
        FreeVSSSource(m_active_vsssources.prev);
    }

    pNew = m_free_vsssources;
    m_free_vsssources = m_free_vsssources->next;
    memset(pNew, 0, sizeof(cvssource_t));

    pNew->next = m_active_vsssources.next;
    pNew->prev = &m_active_vsssources;

    m_active_vsssources.next->prev = pNew;
    m_active_vsssources.next = pNew;

    return pNew;
}

void ClientGameCommandManager::FreeVSSSource(cvssource_t* p)
{
    if (!p->prev) {
        cgi.Error(ERR_DROP, "CCM::FreeVSSSource: not active");
    }

    p->prev->next = p->next;
    p->next->prev = p->prev;
    p->next = m_free_vsssources;
    m_free_vsssources = p;
}

void ClientGameCommandManager::ResetVSSSources()
{
    int i;
    cvssource_t* p;
    cvssource_t* next;

    vss_maxvisible = cgi.Cvar_Get("vss_maxvisible", "1024", CVAR_ARCHIVE | CVAR_LATCH);
    if (m_iAllocatedvsssources && m_iAllocatedvsssources == vss_maxvisible->integer)
    {
        // free existing vss sources
        for (p = m_active_vsssources.prev; p != &m_active_vsssources; p = next)
        {
            next = p->prev;
            FreeVSSSource(p);
        }
        return;
    }

    if (m_iAllocatedvsssources) {
        cgi.Free(m_vsssources);
    }

    if (vss_maxvisible->integer >= 128) {
        m_iAllocatedvsssources = vss_maxvisible->integer;
    }
    else {
        m_iAllocatedvsssources = 128;
    }

    m_vsssources = (cvssource_t*)cgi.Malloc(sizeof(cvssource_t) * m_iAllocatedvsssources);
    memset(m_vsssources, 0, sizeof(cvssource_t) * m_iAllocatedvsssources);

    m_active_vsssources.next = &m_active_vsssources;
    m_active_vsssources.prev = &m_active_vsssources;
    m_free_vsssources = m_vsssources;

    for (i = 0; i < m_iAllocatedvsssources - 1; ++i) {
        m_vsssources[i].next = &m_vsssources[i + 1];
    }

    m_vsssources[m_iAllocatedvsssources - 1].next = NULL;
}

void ClientGameCommandManager::ResetVSSSources(Event* ev)
{
    // FIXME: stub??
}

void CG_ResetVSSSources()
{
    commandManager.ResetVSSSources();
    lastVSSFrameTime = cg.time;
}

void ClientGameCommandManager::InitializeVSSSources()
{
    int i;

    vss_maxvisible = cgi.Cvar_Get("vss_maxvisible", "1024", CVAR_ARCHIVE | CVAR_LATCH);
    if (m_iAllocatedvsssources && m_iAllocatedvsssources == vss_maxvisible->integer) {
        // already allocated
        return;
    }

    if (m_iAllocatedvsssources) {
        cgi.Free(m_vsssources);
    }

    if (vss_maxvisible->integer >= 128) {
        m_iAllocatedvsssources = vss_maxvisible->integer;
    }
    else {
        m_iAllocatedvsssources = 128;
    }

    m_vsssources = (cvssource_t*)cgi.Malloc(sizeof(cvssource_t) * m_iAllocatedvsssources);
    memset(m_vsssources, 0, sizeof(sizeof(cvssource_t) * m_iAllocatedvsssources));

    m_active_vsssources.next = &m_active_vsssources;
    m_active_vsssources.prev = &m_active_vsssources;
    m_free_vsssources = m_vsssources;

    for (i = 0; i < m_iAllocatedvsssources - 1; ++i) {
        m_vsssources[i].next = &m_vsssources[i + 1];
    }

    m_vsssources[m_iAllocatedvsssources - 1].next = NULL;
}

void ClientGameCommandManager::InitializeVSSCvars()
{
    vss_draw = cgi.Cvar_Get("vss_draw", "0", CVAR_ARCHIVE);
    vss_physics_fps = cgi.Cvar_Get("vss_physics_fps", "8", 0);
    vss_repulsion_fps = cgi.Cvar_Get("vss_repulsion_fps", "4", 0);
    vss_maxcount = cgi.Cvar_Get("vss_maxcount", "22", CVAR_ARCHIVE);
    vss_color = cgi.Cvar_Get("vss_color", "1", 0);
    vss_showsources = cgi.Cvar_Get("vss_showsources", "1", 0);
    vss_wind_x = cgi.Cvar_Get("vss_wind_x", "8", 0);
    vss_wind_y = cgi.Cvar_Get("vss_wind_y", "4", 0);
    vss_wind_z = cgi.Cvar_Get("vss_wind_z", "2", 0);
    vss_wind_strength = cgi.Cvar_Get("vss_wind_strength", "8", 0);
    vss_movement_dampen = cgi.Cvar_Get("vss_movement_dampen", "4", 0);
    vss_maxvisible = cgi.Cvar_Get("vss_maxvisible", "1024", 33);
    vss_gridsize = cgi.Cvar_Get("vss_gridsize", "12", 0);
    vss_default_r = cgi.Cvar_Get("vss_default_r", "0.5", 0);
    vss_default_g = cgi.Cvar_Get("vss_default_g", "0.45", 0);
    vss_default_b = cgi.Cvar_Get("vss_default_b", "0.4", 0);
    vss_lighting_fps = cgi.Cvar_Get("vss_lighting_fps", "15", 0);
}

qboolean VSS_SourcePhysics(cvssource_t* pSource, float ftime)
{
    int i;
    int iSmokeType;
    float fWind;
    vec3_t vVel, vDelta;
    trace_t trace;
    entityState_t* pEntState;

    fWind = 0.0;

    if ((pSource->flags2 & 5) != 0)
    {
        VectorMA(pSource->velocity, ftime, pSource->repulsion, pSource->velocity);
    }

    pSource->lastOrigin = pSource->newOrigin;

    if (pSource->flags & 0x800)
    {
        CG_ClipMoveToEntities(
            pSource->newOrigin,
            vec3_origin,
            vec3_origin,
            pSource->newOrigin,
            -1,
            MASK_VOLUMETRIC_SMOKE,
            &trace,
            qfalse);

        if (trace.allsolid)
        {
            vec3_t vMins, vMaxs;
            pEntState = &cg_entities[trace.entityNum].currentState;

            IntegerToBoundingBox(pEntState->solid, vMins, vMaxs);
            for (i = 0; i < 3; i++) {
                vDelta[i] = pSource->newOrigin[i] - ((vMins[i] + vMaxs[i]) * 0.5 + pEntState->origin[i]);
            }
            VectorNormalizeFast(vDelta);

            pSource->velocity = Vector(vDelta) * 16.0;
        }
    }

    if (pSource->flags2 & 5) {
        VectorMA(pSource->newOrigin, ftime, pSource->velocity, pSource->newOrigin);
    }

    if (pSource->flags & 0x800)
    {
        CG_Trace(
            &trace,
            pSource->lastOrigin,
            vec3_origin,
            vec3_origin,
            pSource->newOrigin,
            -1,
            MASK_VOLUMETRIC_SMOKE,
            qfalse,
            qfalse,
            "Collision"
        );

        if (trace.fraction != 1.0)
        {
            float fDot;
            vec3_t vNorm;

            VectorAdd(trace.endpos, trace.plane.normal, pSource->newOrigin);
            fDot = DotProduct(vNorm, pSource->velocity);
            VectorMA(pSource->velocity, fDot, vNorm, pSource->velocity);

            if (vNorm[2] > 0.7) {
                VectorMA(pSource->velocity, ftime * -0.2, pSource->velocity, pSource->velocity);
            }

            iSmokeType = abs(pSource->smokeType);
            if (iSmokeType >= 3 && iSmokeType <= 4)
            {
                if (vNorm[2] > 0.7) {
                    pSource->newDensity -= ftime * 0.08;
                }
            }
        }
    }

    if (pSource->newOrigin[0] < -MAX_VSS_COORDS || pSource->newOrigin[0] > MAX_VSS_COORDS
        || pSource->newOrigin[1] < -MAX_VSS_COORDS || pSource->newOrigin[1] > MAX_VSS_COORDS
        || pSource->newOrigin[2] < -MAX_VSS_COORDS || pSource->newOrigin[2] > MAX_VSS_COORDS) {
        return qfalse;
    }

    iSmokeType = abs(pSource->smokeType);
    if (pSource->flags2 & 5)
    {
        VectorCopy(pSource->velocity, vVel);

        for (i = 0; i < 3; i++)
        {
            switch (i)
            {
            case 0:
                fWind = vss_wind_x->value;
                break;
            case 1:
                fWind = vss_wind_y->value;
                break;
            case 2:
                fWind = vss_wind_z->value;
                break;
            }

            if (fWind < 0.0)
            {
                if (vVel[i] > fWind)
                {
                    vVel[i] -= ftime * vss_wind_strength->value;
                    if (vVel[i] > fWind) {
                        vVel[i] = fWind;
                    }
                }
                else
                {
                    vVel[i] += ftime * vss_movement_dampen->value;
                    if (vVel[i] < fWind) {
                        vVel[i] = fWind;
                    }
                }
            }
            else if (vVel[i] > fWind)
            {
                vVel[i] -= ftime * vss_movement_dampen->value;
                if (vVel[i] < fWind) {
                    vVel[i] = fWind;
                }
            }
            else
            {
                vVel[i] += ftime * vss_movement_dampen->value;
                if (vVel[i] > fWind) {
                    vVel[i] = fWind;
                }
            }
        }

        switch (iSmokeType)
        {
        case 3:
            if (vVel[2] > -8.0) {
                vVel[2] -= ftime * 8.0;
            }
            break;
        case 4:
            if (vVel[2] > -5.0) {
                vVel[2] -= ftime * 3.0;
            }
            break;
        case 5:
            if (vVel[2] < 256.0) {
                vVel[2] += ftime * 40.0;
            }
            break;
        case 6:
            if (vVel[2] > -25.0) {
                vVel[2] -= ftime * 10.0;
            }
            break;
        case 7:
            if (vVel[2] > -10.0) {
                vVel[2] -= ftime * 4.0;
            }
            break;
        case 9:
        case 10:
            if (pSource->typeInfo > 8.0)
            {
                if (vVel[2] < pSource->typeInfo) {
                    vVel[2] += ftime * pSource->typeInfo;
                }

                pSource->typeInfo -= ftime * pSource->typeInfo * 0.04;
                if (pSource->typeInfo < 10.0) {
                    pSource->typeInfo = 10.0;
                }
            }
            break;
        case 11:
            if (vVel[2] > -800.0) {
                vVel[2] -= ftime * 300.0;
            }
            break;
        }

        fWind = VectorLengthSquared(vVel);
        if (fWind > MAX_VSS_WIND_DIST_SQUARED)
        {
            VectorNormalizeFast(vVel);
            VectorScale(vVel, MAX_VSS_WIND_DIST, vVel);
        }

        pSource->velocity = vVel;
    }

    pSource->lastRadius = pSource->newRadius;
    switch (iSmokeType)
    {
    case 1:
        pSource->newRadius += ftime * 1.2 * pSource->scaleMult;
        break;
    case 2:
        pSource->newRadius += ftime * 0.7 * pSource->scaleMult;
        break;
    case 3:
        if ((double)pSource->lifeTime >= 1.0) {
            pSource->newRadius += ftime * 0.7 * pSource->scaleMult;
        } else {
            pSource->newRadius += ftime * 1.2 * pSource->scaleMult;
        }
        break;
    case 4:
        pSource->newRadius += ftime * 1.2 * pSource->scaleMult;
        break;
    case 5:
        pSource->newRadius += ftime * 5.0 * pSource->scaleMult;
        break;
    case 6:
        pSource->newRadius += ftime * 0.8 * pSource->scaleMult;
        break;
    case 7:
        if (pSource->newRadius >= 24.0) {
            pSource->newRadius += ftime * 0.4 * pSource->scaleMult;
        } else {
            pSource->newRadius += ftime * 1.6 * pSource->scaleMult;
        }
        break;
    case 8:
        if (pSource->newRadius >= 24.0) {
            pSource->newRadius += ftime * 0.4 * pSource->scaleMult;
        } else {
            pSource->newRadius += ftime * 1.6 * pSource->scaleMult;
        }
        break;
    case 9:
    case 10:
        if (pSource->newRadius >= 16.0) {
            pSource->newRadius += ftime * 0.4 * pSource->scaleMult;
        } else {
            pSource->newRadius += ftime * 0.8 * pSource->scaleMult;
        }
        break;
    case 11:
        pSource->newRadius += ftime * 0.8 * pSource->scaleMult;
        break;
    default:
        pSource->newRadius += ftime * 1.2 * pSource->scaleMult;
        break;
    }

    if (pSource->newRadius < 1.0) {
        pSource->newRadius = 1.0;
    }
    else if (pSource->newRadius > 32.0) {
        pSource->newRadius = 32.0;
    }

    pSource->ooRadius = 1.0 / pSource->newRadius;
    pSource->lastDensity = pSource->newDensity;
    if (pSource->smokeType >= 0)
    {
        switch (iSmokeType)
        {
        case 1:
            pSource->newDensity -= ftime * 0.07 * pSource->fadeMult;
            break;
        case 2:
            pSource->newDensity -= ftime * 0.075 * pSource->fadeMult;
            break;
        case 3:
            if (pSource->newDensity > 0.6) {
                pSource->newDensity -= ftime * 0.05 * pSource->fadeMult;
            }
            else {
                pSource->newDensity -= ftime * 0.4 * pSource->fadeMult;
            }
            break;
        case 4:
            pSource->newDensity -= ftime * 0.0080000004 * pSource->fadeMult;
            break;
        case 5:
            pSource->newDensity -= ftime * 0.75 * pSource->fadeMult;
            break;
        case 6:
            pSource->newDensity -= ftime * 0.016000001 * pSource->fadeMult;
            break;
        case 7:
            pSource->newDensity -= ftime * 0.0049999999 * pSource->fadeMult;
            break;
        case 8:
            if (pSource->newDensity > 0.7) {
                pSource->newDensity -= ftime * 0.025 * pSource->fadeMult;
            }
            else {
                pSource->newDensity -= ftime * 0.38 * pSource->fadeMult;
            }
            break;
        case 11:
            pSource->newDensity = pSource->newDensity - ftime * 0.125 * pSource->fadeMult;
            break;
        default:
            if (pSource->newDensity > 0.4) {
                pSource->newDensity -= ftime * 0.01 * pSource->fadeMult;
            }
            else {
                pSource->newDensity -= ftime * 0.0075 * pSource->fadeMult;
            }
            break;
        }
        if (pSource->newDensity <= 0.06)
            return 0;
    }
    else
    {
        switch (iSmokeType)
        {
        case 3:
            VSS_ClampAlphaLife(pSource, 150);
            break;
        case 4:
            VSS_ClampAlphaLife(pSource, 200);
            break;
        case 5:
        case 11:
            VSS_ClampAlphaLife(pSource, 50);
            break;
        case 6:
        case 8:
            VSS_ClampAlphaLife(pSource, 600);
            break;
        case 7:
            VSS_ClampAlphaLife(pSource, 800);
            break;
        case 9:
        case 10:
            VSS_ClampAlphaLife(pSource, 1500);
            break;
        default:
            VSS_ClampAlphaLife(pSource, 100);
            break;
        }
    }

    VectorCopy(pSource->newColor, pSource->lastColor);
    if (iSmokeType == 1)
    {
        for (i = 0; i < 3; ++i)
        {
            pSource->newColor[i] -= ftime * 0.05f * pSource->fadeMult;
            if (pSource->newColor[i] < 0.0f) {
                pSource->newColor[i] = 0.0f;
            }
        }
    }
    else if (iSmokeType == 9)
    {
        for (i = 0; i < 3; ++i)
        {
            if (pSource->newColor[i] < 0.9f)
            {
                pSource->newColor[i] += ftime * 0.02f * pSource->fadeMult;
                if (pSource->newColor[i] > 0.9f) {
                    pSource->newColor[i] = 0.9f;
                }
            }
        }
    }

    return qtrue;
}

qboolean VSS_LerpSource(cvssource_t* pCurrent, cvssourcestate_t* pState, float fLerpFrac, float fLightingFrac)
{
    int i;

    if (pCurrent->flags & 0xA0000)
    {
        Vector parentOrigin;

        for (i = 0; i < 3; i++) {
            pState->origin[i] = (pCurrent->newOrigin[i] - pCurrent->lastOrigin[i]) * fLerpFrac + pCurrent->lastOrigin[i];
        }

        if (!cg_entities[pCurrent->parent].currentValid) {
            return qfalse;
        }

        refEntity_t* e = cgi.R_GetRenderEntity(pCurrent->parent);
        if (!e) {
            return qfalse;
        }

        parentOrigin = e->origin;
        VectorAdd(pState->origin, parentOrigin, pState->origin);
    }
    else if (pCurrent->flags2 & 5)
    {
        for (i = 0; i < 3; i++) {
            pState->origin[i] = (pCurrent->newOrigin[i] - pCurrent->lastOrigin[i]) * fLerpFrac + pCurrent->lastOrigin[i];
        }
    }

    if (vss_color->integer)
    {
        for (i = 0; i < 3; ++i) {
            pState->color[i] = (pCurrent->newColor[i] - pCurrent->lastColor[i]) * fLerpFrac + pCurrent->lastColor[i];
        }
    }

    if (vss_lighting_fps->integer)
    {
        for (i = 0; i < 3; ++i) {
            pState->color[i] = ((pCurrent->newLighting[i] - pCurrent->lastLighting[i]) * fLightingFrac
                + pCurrent->lastLighting[i])
                * pState->color[i];
        }
    }

    pState->density = (pCurrent->newDensity - pCurrent->lastDensity) * fLerpFrac + pCurrent->lastDensity;
    pState->radius = (pCurrent->newRadius - pCurrent->lastRadius) * fLerpFrac + pCurrent->lastRadius;

    return qtrue;
}

void ClientGameCommandManager::SpawnVSSSource(int count, int timealive)
{
    int i;
    int iSmokeLeft, iSmokeType;
    float fSmokeTypeDataValue = 0.0;
    float fFadeMult = 0.0, fScaleMult = 0.0;
    float fCountScale;
    float fDensity, fRadius;
    float fAngle = 0.0, fAngleStep = 0.0;
    Vector vNewForward;
    str sSmokeName;
    cvssource_t* pSource;

    if (m_spawnthing->cgd.alpha <= 0.0) {
        return;
    }
    fDensity = this->m_spawnthing->cgd.alpha;
    fRadius = this->m_spawnthing->cgd.scale * vss_maxcount->value * 0.1;
    if (fRadius > 32.0) {
        fRadius = 32.0;
    }

    if (m_spawnthing->cgd.flags & 0x10)
    {
        fAngle = 0.0;
        fAngleStep = 360.0 / (count / vss_maxcount->value);
    }

    iSmokeType = 0;
    sSmokeName = m_spawnthing->GetModel();

    for (i = 0; i < 12; ++i)
    {
        if (sSmokeName == cg_vsstypes[i])
        {
            iSmokeType = i;
            if (i < 9 || i > 10)
            {
                fSmokeTypeDataValue = m_spawnthing->cgd.accel[0];
            }
            else
            {
                fSmokeTypeDataValue = m_spawnthing->cgd.accel[0];
                if (fSmokeTypeDataValue == 0.0)
                    fSmokeTypeDataValue = 24.0;
            }

            break;
        }
    }

    iSmokeType = -iSmokeType;
    fFadeMult = m_spawnthing->cgd.accel[1];
    if (fFadeMult < 0.0001) {
        fFadeMult = 1.0;
    }

    fScaleMult = m_spawnthing->cgd.accel[2];
    if (!fScaleMult) {
        fScaleMult = 1.0;
    }

    fCountScale = m_spawnthing->cgd.life / 1000;
    if (count * fCountScale < vss_maxcount->value) {
        iSmokeLeft = count * fCountScale;
    }
    else {
        iSmokeLeft = (int)(fCountScale * count * cg_effectdetail->value);
        if (iSmokeLeft < vss_maxcount->value)
            iSmokeLeft = (int)vss_maxcount->value;
    }

    while (iSmokeLeft > 0)
    {
        pSource = AllocateVSSSource();
        if (!pSource)
        {
            cgi.DPrintf("Out of VSS Sources\n");
            return;
        }

        pSource->startAlpha = (random() * 0.15 + 0.85) * fDensity;
        pSource->newDensity = 0.0;
        if (m_spawnthing->cgd.flags & 1)
        {
            pSource->newRadius = RandomizeRange(m_spawnthing->cgd.scalemin, m_spawnthing->cgd.scalemax);;
            if (pSource->newRadius > 32.0) {
                pSource->newRadius = 32.0;
            }
        }
        else {
            pSource->newRadius = fRadius;
        }

        if (iSmokeLeft < vss_maxcount->value) {
            pSource->newRadius = iSmokeLeft / vss_maxcount->value * pSource->newRadius;
        }

        if (vss_color->value)
        {
            float fRandom = random() * 0.3 + 0.7;
            pSource->newColor[0] = fRandom * m_spawnthing->cgd.color[0];
            pSource->newColor[1] = fRandom * m_spawnthing->cgd.color[1];
            pSource->newColor[2] = fRandom * m_spawnthing->cgd.color[2];
        }

        pSource->parent = m_spawnthing->cgd.parent;
        pSource->flags = m_spawnthing->cgd.flags;
        pSource->flags2 = m_spawnthing->cgd.flags2;
        pSource->smokeType = iSmokeType;
        pSource->typeInfo = fSmokeTypeDataValue;
        pSource->fadeMult = fFadeMult;
        pSource->scaleMult = fScaleMult;
        pSource->roll = anglemod(fAngle);

        if (random() < 0.5) {
            pSource->flags |= 0x40000;
        }

        VectorCopy(m_spawnthing->axis[0], vNewForward);
        if (m_spawnthing->cgd.flags & 4)
        {
            VectorCopy(m_spawnthing->cgd.origin, pSource->newOrigin);
            do
            {
                vNewForward = Vector(crandom(), crandom(), crandom());
            } while (Vector::Dot(vNewForward, vNewForward) < 1.0);
        }
        else if (m_spawnthing->cgd.flags & 0x10)
        {
            if (m_spawnthing->sphereRadius != 0.0)
            {
                Vector dst, end;

                end = m_spawnthing->axis[0];
                RotatePointAroundVector(dst, m_spawnthing->axis[2], end, fAngle);

                VectorAdd(m_spawnthing->cgd.origin, dst, pSource->newOrigin);
                VectorSubtract(pSource->newOrigin, m_spawnthing->cgd.origin, vNewForward);
                VectorNormalizeFast(vNewForward);

                fAngle += fAngleStep;
            }
        }
        else if (m_spawnthing->cgd.flags & 8)
        {
            Vector dir, end;
            do
            {
                dir = Vector(crandom(), crandom(), crandom());
            } while (Vector::Dot(dir, dir) < 1.0);

            end = m_spawnthing->cgd.origin + dir * m_spawnthing->sphereRadius;
            VectorCopy(end, pSource->newOrigin);
            vNewForward = dir * -1.0;
        }
        else if (m_spawnthing->cgd.flags2 & 0x20000)
        {
            float fHeight, fRadius;
            float fAngle;
            float sina, cosa;

            fHeight = random();
            fRadius = random();
            if (fHeight < fRadius)
            {
                float fTemp = fHeight;
                fHeight = fRadius;
                fRadius = fTemp;
            }
            fHeight *= m_spawnthing->coneHeight;
            fRadius *= m_spawnthing->sphereRadius;
            fAngle = random() * 6.2831855;
            cosa = cos(fAngle);
            sina = sin(fAngle);

            VectorMA(m_spawnthing->cgd.origin, fHeight, m_spawnthing->axis[0], pSource->newOrigin);
            VectorMA(m_spawnthing->cgd.origin, fRadius * cosa, m_spawnthing->axis[1], pSource->newOrigin);
            VectorMA(m_spawnthing->cgd.origin, fRadius * sina, m_spawnthing->axis[2], pSource->newOrigin);
        }
        else if (m_spawnthing->sphereRadius) {
            VectorCopy(m_spawnthing->cgd.origin, pSource->newOrigin);
        }
        else
        {
            Vector dir, end;
            do
            {
                dir = Vector(crandom(), crandom(), crandom());
            } while (Vector::Dot(dir, dir) < 1.0);

            dir.normalize();
            end = m_spawnthing->cgd.origin + dir * m_spawnthing->sphereRadius;
            VectorCopy(end, pSource->newOrigin);
            vNewForward = dir;
        }

        for (i = 0; i < 3; i++)
        {
            pSource->newOrigin[i] += random() * m_spawnthing->origin_offset_base[i] + m_spawnthing->origin_offset_amplitude[i];
        }

        VectorCopy(pSource->newOrigin, pSource->lastOrigin);
        if (timealive > 0) {
            pSource->lifeTime = timealive;
        }
        else {
            pSource->lifeTime = 0;
        }

        if (m_spawnthing->forwardVelocity)
        {
            for (i = 0; i < 3; ++i)
            {
                pSource->velocity[i] = vNewForward[i] * m_spawnthing->forwardVelocity;
            }
        }

        for (i = 0; i < 3; ++i)
        {
            float fVel = m_spawnthing->randvel_base[i] + random() * m_spawnthing->randvel_amplitude[i];
        
            if (m_spawnthing->cgd.flags & 0x400000)
            {
                pSource->velocity += Vector(m_spawnthing->tag_axis[i]) * fVel;
            }
            else {
                pSource->velocity[i] += fVel;
            }
        }

        for (i = 0; i < 3; ++i)
        {
            float fDist = m_spawnthing->axis_offset_base[i] + random() * m_spawnthing->axis_offset_amplitude[i];

            if (pSource->flags2 & 0x80)
            {
                pSource->newOrigin += Vector(m_spawnthing->axis[i]) * fDist;
            }
            else {
                pSource->newOrigin += Vector(m_spawnthing->tag_axis[i]) * fDist;
            }
        }

        pSource->velocity *= pSource->lifeTime / 1000.0;
        pSource->newOrigin += pSource->velocity;
        if (vss_lighting_fps->integer)
        {
            cgi.R_GetLightingForSmoke(pSource->newLighting, pSource->newOrigin);
        }

        iSmokeLeft -= vss_maxcount->value;
    }
}

void VSS_CalcRepulsionForces(cvssource_t* pActiveSources)
{
    cvssource_t* pCurrent;
    cvssource_t* pComp;

    pCurrent = pActiveSources->prev;
    if (pCurrent != pActiveSources) {
        qboolean bXUp, bXDown;
        qboolean bYUp, bYDown;
        qboolean bZDown;
        int i;
        int iIndex;
        int iX, iY, iZ;
        int iMinX, iMinY, iMinZ;
        int iMaxX, iMaxY, iMaxZ;
        float fOfs;
        cvssource_t* pSTLatch;

        memset(vss_sorttable, 0, sizeof(vss_sorttable));

        while (pCurrent != pActiveSources) {
            VectorClear(pCurrent->repulsion);

            iIndex = ((int)floor(pCurrent->newOrigin[0] + 8192.0 + 0.5) / 96) % 32;
            iIndex |= ((int)floor(pCurrent->newOrigin[1] + 8192.0 + 0.5) / 96) % 32;
            iIndex |= (((int)floor(pCurrent->newOrigin[2] + 8192.0 + 0.5) / 96) % 16) << 10;
            
            pCurrent->stnext = vss_sorttable[iIndex];
            vss_sorttable[iIndex] = pCurrent;
            pCurrent->stindex = iIndex;
            pCurrent = pCurrent->prev;
        }

        for (pCurrent = pActiveSources->prev; pCurrent != pActiveSources; pCurrent = pCurrent->prev)
        {
            if (vss_sorttable[pCurrent->stindex] == pCurrent)
            {
                pSTLatch = (cvssource_t*)-1;
                pComp = pCurrent->stnext;
            }
            else
            {
                pSTLatch = 0;
                pComp = vss_sorttable[pCurrent->stindex];
            }

            while (pComp)
            {
                VSS_AddRepulsion(pCurrent, pComp);
                if (!pSTLatch && pComp->stnext == pCurrent)
                {
                    pSTLatch = pComp;
                    pComp = pComp->stnext;
                }

                pComp = pComp->stnext;
            }
            iX = ((int)floor(pCurrent->newOrigin[0] + 8192.0 + 0.5) / 96) % 32;
            iY = ((int)floor(pCurrent->newOrigin[1] + 8192.0 + 0.5) / 96) % 32;
            iY *= 2;
            iZ = ((int)floor(pCurrent->newOrigin[2] + 8192.0 + 0.5) / 96) % 16;
            iZ <<= 10;

            fOfs = pCurrent->newRadius + 1.49 + 48.0;
            iMaxX = ((int)floor(pCurrent->newOrigin[0] + 8192.0 + 0.5 + fOfs) / 96) % 32;
            iMaxY = ((int)floor(pCurrent->newOrigin[1] + 8192.0 + 0.5 + fOfs) / 96) % 32;
            iMaxY *= 2;
            iMaxZ = ((int)floor(pCurrent->newOrigin[2] + 8192.0 + 0.5 + fOfs) / 96) % 16;
            iMaxZ <<= 10;

            iMinX = ((int)floor(pCurrent->newOrigin[0] + 8192.0 + 0.5 - fOfs) / 96) % 32;
            iMinY = ((int)floor(pCurrent->newOrigin[1] + 8192.0 + 0.5 - fOfs) / 96) % 32;
            iMinY *= 2;
            iMinZ = ((int)floor(pCurrent->newOrigin[2] + 8192.0 + 0.5 - fOfs) / 96) % 16;
            iMinZ <<= 10;

            bXUp = (iMaxX | (pCurrent->stindex & 0xFFFFFFE0)) != pCurrent->stindex;
            bXDown = (iMinX | (pCurrent->stindex & 0xFFFFFFE0)) != pCurrent->stindex;
            bYUp = (iMaxY | (pCurrent->stindex & 0xFFFFFC1F)) != pCurrent->stindex;
            bYDown = (iMinY | (pCurrent->stindex & 0xFFFFFC1F)) != pCurrent->stindex;

            iIndex = iMinZ | (pCurrent->stindex & 0xFFFFFCC3);
            bZDown = iIndex != pCurrent->stindex;

            if (iIndex == pCurrent->stindex)
            {
                iIndex = iMaxY | pCurrent->stindex & 0xFFFFFC1F;
                i = 9;
            }
            else {
                i = 0;
            }

            while (i < (bZDown ? 26 : 17))
            {
                switch (i)
                {
                case 0:
                    iIndex = iMaxZ | (pCurrent->stindex & 0xFFFFFCC3);
                    break;
                case 1:
                    iIndex = iMaxX | (iIndex & 0xFFFFFFE0);
                    if (bXUp) {
                        break;
                    }
                    continue;
                case 2:
                    iIndex = iMaxY | (iIndex & 0xFFFFFC1F);
                    if (bXUp && bYUp) {
                        break;
                    }
                    continue;
                case 3:
                    iIndex = iMinY | (iIndex & 0xFFFFFC1F);
                    if (bXUp && bYDown) {
                        break;
                    }
                    continue;
                case 4:
                    iIndex = iMinY | (iIndex & 0xFFFFFFE0);
                    if (bYDown) {
                        break;
                    }
                    continue;
                case 5:
                    iIndex = iMinX | (iIndex & 0xFFFFFFE0);
                    if (bXDown && bYDown) {
                        break;
                    }
                    continue;
                case 6:
                    iIndex = iY | (iIndex & 0xFFFFFC1F);
                    if (bXDown) {
                        break;
                    }
                    continue;
                case 7:
                    iIndex = iMaxY | (iIndex & 0xFFFFFC1F);
                    if (bXDown && bYUp) {
                        break;
                    }
                    continue;
                case 8:
                    iIndex = iX | (iIndex & 0xFFFFFFE0);
                    if (bYUp) {
                        break;
                    }
                    continue;
                case 9:
                    iIndex = iZ | (iIndex & 0xFFFFFFC3);
                    if (bYUp) {
                        break;
                    }
                    continue;
                case 10:
                    iIndex = iMaxX | (iIndex & 0xFFFFFFE0);
                    if (bXUp && bYUp) {
                        break;
                    }
                    continue;
                case 11:
                    iIndex = iMinX | (iIndex & 0xFFFFFFE0);
                    if (bXDown && bYUp) {
                        break;
                    }
                    continue;
                case 12:
                    iIndex = iY | (iIndex & 0xFFFFFC1F);
                    if (bXDown) {
                        break;
                    }
                    continue;
                case 13:
                    iIndex = iMinY | (iIndex & 0xFFFFFC1F);
                    if (bXDown && bYDown) {
                        break;
                    }
                    continue;
                case 14:
                    iIndex = iX | (iIndex & 0xFFFFFFE0);
                    if (bYDown) {
                        break;
                    }
                    continue;
                case 15:
                    iIndex = iMaxX | (iIndex & 0xFFFFFFE0);
                    if (bXUp && bYDown) {
                        break;
                    }
                    continue;
                case 16:
                    iIndex = iY | (iIndex & 0xFFFFFC1F);
                    if (bXUp) {
                        break;
                    }
                    continue;
                case 17:
                    iIndex = iMinZ | (iIndex & 0xFFFFFCC3);
                    if (bXUp) {
                        break;
                    }
                    continue;
                case 18:
                    iIndex = iMaxY | (iIndex & 0xFFFFFC1F);
                    if (bXUp && bYUp) {
                        break;
                    }
                    continue;
                case 19:
                    iIndex = iMinY | (iIndex & 0xFFFFFC1F);
                    if (bXUp && bYDown) {
                        break;
                    }
                    continue;
                case 20:
                    iIndex = iX | (iIndex & 0xFFFFFFE0);
                    if (bYDown) {
                        break;
                    }
                    continue;
                case 21:
                    iIndex = iMinX | (iIndex & 0xFFFFFFE0);
                    if (bXDown && bYDown) {
                        break;
                    }
                    continue;
                case 22:
                    iIndex = iY | (iIndex & 0xFFFFFC1F);
                    if (bXDown) {
                        break;
                    }
                    continue;
                case 23:
                    iIndex = iMaxY | (iIndex & 0xFFFFFC1F);
                    if (bXDown && bYUp) {
                        break;
                    }
                    continue;
                case 24:
                    iIndex = iX | (iIndex & 0xFFFFFFE0);
                    if (bYUp) {
                        break;
                    }
                    continue;
                case 25:
                    iIndex = iY | (iIndex & 0xFFFFFC1F);
                    break;
                }

                for (pComp = vss_sorttable[iIndex]; pComp; pComp = pComp->stnext) {
                    VSS_AddRepulsion(pCurrent, pComp);
                }

                i++;
            }

            if (pSTLatch == (cvssource_t*)-1) {
                vss_sorttable[pCurrent->stindex] = pCurrent->stnext;
            } else {
                pSTLatch->stnext = pCurrent->stnext;
            }
        }
    }
}

void CG_AddVSSSources()
{
    commandManager.AddVSSSources();
}

void ClientGameCommandManager::AddVSSSources()
{
    int i, j;
    int frameTime;
    int physics_rate, lighting_rate;
    int mstime;
    float fLerpFrac, fLightingFrac;
    vec3_t vAng;
    cvssource_t* pCurrent;
    cvssource_t* pComp;
    cvssourcestate_t state;
    int hModel, hModel2;
    refEntity_t newEnt;

    hModel = 0;
    hModel2 = 0;

    if (vss_showsources->integer)
    {
        // load sources
        hModel = cgi.R_RegisterModel("VSSSource.spr");
        hModel2 = cgi.R_RegisterModel("VSSSource2.spr");

        memset(&newEnt, 0, sizeof(newEnt));
        memset(vAng, 0, sizeof(vAng));

        AnglesToAxis(vAng, newEnt.axis);

        newEnt.renderfx = 0;
        newEnt.reType = RT_SPRITE;
        newEnt.shaderTime = 0.0;
        newEnt.frameInfo[0].index = 0;
        newEnt.frameInfo[0].weight = 1.0;
        newEnt.frameInfo[0].time = 0.0;
        newEnt.actionWeight = 1.0;
    }

    if (lastVSSFrameTime)
    {
        if (cg.time < lastVSSFrameTime || cg.time - lastVSSFrameTime > 500)
        {
            for (pCurrent = m_active_vsssources.prev; pCurrent != &m_active_vsssources; pCurrent = pCurrent->prev)
            {
                pCurrent->lastPhysicsTime = cg.time;
                pCurrent->lastLightingTime = cg.time;
            }

            m_iLastVSSRepulsionTime = cg.time;
            lastVSSFrameTime = cg.time;

            return;
        }

        frameTime = cg.time - lastVSSFrameTime;
    }
    else
    {
        frameTime = 0;
    }
    if (paused->integer) {
        lastVSSFrameTime = 0;
    }
    else {
        lastVSSFrameTime = cg.time;
    }

    if (lastVSSFrameTime)
    {
        if (cg.time >= m_iLastVSSRepulsionTime && cg.time - m_iLastVSSRepulsionTime <= 500)
        {
            if (cg.time - m_iLastVSSRepulsionTime >= 1000 / vss_repulsion_fps->integer)
            {
                VSS_CalcRepulsionForces(&m_active_vsssources);
                m_iLastVSSRepulsionTime = cg.time;
            }
        }
        else {
            m_iLastVSSRepulsionTime = cg.time;
        }
    }
    else {
        m_iLastVSSRepulsionTime = 0;
    }

    physics_rate = (int)(1000.0 / (float)vss_physics_fps->integer);
    lighting_rate = (int)(1000.0 / (float)vss_lighting_fps->integer);
    for (pCurrent = this->m_active_vsssources.prev; pCurrent != &this->m_active_vsssources; pCurrent = pComp)
    {
        pComp = pCurrent->prev;

        newEnt.renderfx = 0;
        if (pCurrent->flags < 0 && !cg_detail->integer)
        {
            FreeVSSSource(pCurrent);
            continue;
        }

        if ((pCurrent->flags2 & 0x4000) != 0) {
            newEnt.renderfx = 0x4000000;
        }

        if (pCurrent->lastPhysicsTime)
        {
            mstime = cg.time - pCurrent->lastPhysicsTime;
            if (mstime > 2 * physics_rate) {
                mstime = physics_rate;
            }

            if (mstime >= physics_rate || (pCurrent->flags2 & 0x10) != 0)
            {
                if (!VSS_SourcePhysics(pCurrent, (float)mstime * 0.001))
                {
                    FreeVSSSource(pCurrent);
                    continue;
                }

                pCurrent->lastPhysicsTime = cg.time;
            }
        }

        if (pCurrent->lastLightingTime)
        {
            mstime = cg.time - pCurrent->lastLightingTime;
            if (mstime > 2 * lighting_rate) {
                mstime = lighting_rate;
            }

            if (mstime >= lighting_rate)
            {
                pCurrent->lastLighting[0] = pCurrent->newLighting[0];
                pCurrent->lastLighting[1] = pCurrent->newLighting[1];
                pCurrent->lastLighting[2] = pCurrent->newLighting[2];
                cgi.R_GetLightingForSmoke(pCurrent->newLighting, pCurrent->newOrigin);
                pCurrent->lastLightingTime = cg.time;
            }
        }

        fLerpFrac = (cg.time - pCurrent->lastPhysicsTime) / physics_rate;
        if (fLerpFrac > 1.0) {
            fLerpFrac = 1.0;
        }
        else if (fLerpFrac < 0.0) {
            fLerpFrac = 0.0;
        }
        
        fLightingFrac = (cg.time - pCurrent->lastLightingTime) / lighting_rate;
        if (fLightingFrac > 1.0) {
            fLightingFrac = 1.0;
        }
        else if (fLightingFrac < 0.0) {
            fLightingFrac = 0.0;
        }

        if (lastVSSFrameTime) {
            pCurrent->lifeTime += frameTime;
        }

        if (!pCurrent->lastValid)
        {
            if (!VSS_SourcePhysics(pCurrent, (float)physics_rate * 0.001))
            {
                ClientGameCommandManager::FreeVSSSource(pCurrent);
                continue;
            }

            pCurrent->lastLighting[0] = pCurrent->newLighting[0];
            pCurrent->lastLighting[1] = pCurrent->newLighting[1];
            pCurrent->lastLighting[2] = pCurrent->newLighting[2];
            cgi.R_GetLightingForSmoke(pCurrent->newLighting, pCurrent->newOrigin);
            
            fLerpFrac = 0.0;
            fLightingFrac = 0.0;

            pCurrent->lastPhysicsTime = cg.time;
            pCurrent->lastLightingTime = cg.time;
            pCurrent->lastValid = 1;
        }

        if (VSS_LerpSource(pCurrent, &state, fLerpFrac, fLightingFrac))
        {
            if (vss_showsources->integer)
            {
                VectorCopy(state.origin, newEnt.origin);
                newEnt.scale = state.radius / 5.0;

                if (vss_color->integer)
                {
                    newEnt.shaderRGBA[0] = (int)(state.color[0] * 255.0);
                    newEnt.shaderRGBA[1] = (int)(state.color[1] * 255.0);
                    newEnt.shaderRGBA[2] = (int)(state.color[2] * 255.0);
                }
                else
                {
                    newEnt.shaderRGBA[0] = (int)(vss_default_r->value * 255.0);
                    newEnt.shaderRGBA[1] = (int)(vss_default_g->value * 255.0);
                    newEnt.shaderRGBA[2] = (int)(vss_default_b->value * 255.0);
                }

                newEnt.shaderRGBA[3] = (int)(state.density * 255.0);

                if (lastVSSFrameTime)
                {
                    pCurrent->roll += frameTime;

                    for (i = 0; i < 3; ++i)
                    {
                        j = (int)(frameTime * pCurrent->velocity[i] * 0.03);
                        if (pCurrent->velocity[i] < 0.0) {
                            j = -j;
                        }

                        if (j > frameTime)
                        {
                            j = (int)((float)frameTime + (float)(j - frameTime) * 0.75);
                            if (j > 2 * frameTime) {
                                j = 2 * frameTime;
                            }
                        }

                        pCurrent->roll -= j;
                    }

                    if ((pCurrent->flags & 0x40000) != 0) {
                        newEnt.hModel = hModel;
                    }
                    else {
                        newEnt.hModel = hModel2;
                    }
                }
                else if ((pCurrent->flags & 0x40000) != 0) {
                    newEnt.hModel = hModel;
                }
                else {
                    newEnt.hModel = hModel2;
                }

                newEnt.shaderTime = (float)(pCurrent->roll + cg.time - pCurrent->lifeTime) * 0.001;
                cgi.R_AddRefSpriteToScene(&newEnt);
            }
        }
        else {
            FreeVSSSource(pCurrent);
        }
    }

    if (vss_showsources->integer == 2)
    {
        i = 0;

        for (pCurrent = this->m_active_vsssources.prev; pCurrent != &this->m_active_vsssources; pCurrent = pCurrent->prev) {
            ++i;
        }

        cgi.DPrintf("VSS Sources In Use: %i\n", i);
    }
}

void VSS_ClampAlphaLife(cvssource_t* pSource, int maxlife)
{
    if (pSource->lifeTime >= maxlife)
    {
        pSource->smokeType = -pSource->smokeType;
        pSource->newDensity = pSource->startAlpha;
    }
    else
    {
        pSource->newDensity = (float)pSource->lifeTime / (float)maxlife * pSource->startAlpha;
    }
}
