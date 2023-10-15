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

// actorenemy.cpp:

#include "actorenemy.h"
#include "worldspawn.h"
#include "sentient.h"
#include "actor.h"

float ActorEnemy::UpdateLMRF(Actor *pSelf, bool *pbInFovAndRange, bool *pbVisible)
{
    float  fMaxRange, fRange, fRangeScale, fNormalizedRange;
    float  fForward;
    float  fFovScale;
    float  fMinSightTime;
    float  fLMRF;
    float  fFarPlane;
    Vector vDelta;
    float  fObfuscation;

    *pbInFovAndRange = false;
    *pbVisible       = false;

    vDelta    = pSelf->VirtualEyePosition() - GetEnemy()->origin;
    vDelta[2] = 0;

    fRange    = vDelta.lengthXYSquared();
    fMaxRange = pSelf->m_fSight;

    if (world->farplane_distance > 0) {
        fFarPlane = world->farplane_distance * 0.828f;

        if (fMaxRange > fFarPlane) {
            fMaxRange = fFarPlane;
        }
    }

    if (fRange > Square(fMaxRange)) {
        return 8.0;
    }

    fForward = sqrt(fRange);

    fMinSightTime = -DotProduct2D(pSelf->m_vEyeDir, vDelta);
    if (fMinSightTime < 0) {
        return 8.0;
    }

    fMinSightTime += 128;
    if (pSelf->m_fFovDot * fForward > fMinSightTime) {
        return 8.0;
    }

    *pbInFovAndRange = true;
    if (!pSelf->CanSee(m_pEnemy, 0, 0, false)) {
        return 8.0;
    }

    // Added in 2.0.
    //  Check for obfuscation
    fObfuscation = G_VisualObfuscation(pSelf->VirtualEyePosition(), m_pEnemy->EyePosition());
    if (fObfuscation >= 1.f) {
        return 8.0;
    }

    *pbVisible = true;

    fFovScale = (fMinSightTime / (fForward + 128.0) * -1.3 - (pSelf->m_fFovDot * 0.2 - 1.5)) / (1.0 - pSelf->m_fFovDot);
    fRangeScale      = fForward / fMaxRange;
    fNormalizedRange = (((fRangeScale * 16.0 + -16.0) * fRangeScale + -1.0) * fRangeScale + 7.0) * fRangeScale;
    fNormalizedRange = fNormalizedRange / 3.0;

    fLMRF = Square(m_pEnemy->stealthMovementScale * fNormalizedRange) * (fForward + 128.0) / fMinSightTime;
    fLMRF *= pSelf->m_fNoticeTimeScale * g_ai_noticescale->value;
    if (fLMRF < fFovScale) {
        return fFovScale;
    }

    return fLMRF;
}

float ActorEnemy::UpdateVisibility(Actor *pSelf, bool *pbInFovAndRange, bool *pbVisible)
{
    float fLMRF, fFrameTime;

    fFrameTime = level.time - m_fLastLookTime;

    m_fLastLookTime = level.time;

    fLMRF = UpdateLMRF(pSelf, pbInFovAndRange, pbVisible);
    if (fLMRF < 8.0) {
        m_fVisibility += fFrameTime / fLMRF;
    } else if (m_fVisibility < 1.0) {
        m_fVisibility -= fFrameTime * 0.25;
        if (m_fVisibility < 0.0) {
            m_fVisibility = 0.0;
        }
    }

    m_fTotalVisibility = m_fVisibility + GetEnemy()->m_fPlayerSightLevel;
    if (m_fTotalVisibility > 1) {
        m_fVisibility      = 1;
        m_fTotalVisibility = 1;
    }
    return m_fTotalVisibility;
}

int ActorEnemy::UpdateThreat(Actor *pSelf)
{
    static float fRangeThreatSquared[] = {
        Square(2048),
        Square(1400),
        Square(1100),
        Square(916),
        Square(784),
        Square(683),
        Square(600),
        Square(529),
        Square(466),
        Square(410),
        Square(357),
        Square(305),
        Square(254),
        Square(200),
        Square(137)};
    static int iWeaponThreat[7][5] = {
        {5,  3,  1,  1, 1},
        {8,  6,  4,  2, 1},
        {10, 9,  7,  8, 4},
        {12, 10, 8,  4, 3},
        {12, 12, 9,  7, 4},
        {12, 10, 5,  2, 1},
        {10, 12, 10, 7, 5}
    };
    int    iWeapon;
    int    iZone;
    Vector vDelta;
    int    i;

    m_iThreat              = 0;
    m_fCurrentRangeSquared = 1e38f;

    if (m_pEnemy->m_bIsDisguised || m_fTotalVisibility < 0.999f || level.inttime < m_iNextEnemyTime) {
        return m_iThreat;
    }

    m_iThreat = 10000;
    if (m_bVisible == true) {
        m_iThreat = 10500;
    }

    vDelta                 = m_pEnemy->origin - pSelf->origin;
    m_fCurrentRangeSquared = vDelta.lengthSquared();

    if (m_fCurrentRangeSquared < Square(256)) {
        iZone = 0;
    } else if (m_fCurrentRangeSquared < Square(768)) {
        iZone = 1;
    } else if (m_fCurrentRangeSquared < Square(1280)) {
        iZone = 2;
    } else if (m_fCurrentRangeSquared < Square(2048)) {
        iZone = 3;
    } else {
        iZone = 4;
    }

    Weapon *pEnemyWeapon = m_pEnemy->GetActiveWeapon(WEAPON_MAIN);
    iWeapon              = 0;

    if (pEnemyWeapon) {
        int iWeapClass = pEnemyWeapon->GetWeaponClass();
        if (iWeapClass & WEAPON_CLASS_PISTOL) {
            iWeapon = 1;
        } else if (iWeapClass & WEAPON_CLASS_RIFLE) {
            iWeapon = 2;
        } else if (iWeapClass & WEAPON_CLASS_SMG) {
            iWeapon = 3;
        } else if (iWeapClass & WEAPON_CLASS_MG) {
            iWeapon = 4;
        } else if (iWeapClass & WEAPON_CLASS_GRENADE) {
            iWeapon = 5;
        } else {
            iWeapon = 6;
        }
    }

    for (i = 0; i < ARRAY_LEN(fRangeThreatSquared) && fRangeThreatSquared[i] > m_fCurrentRangeSquared; i++) {}

    m_iThreat += iWeaponThreat[iWeapon][iZone] + m_pEnemy->m_iThreatBias + i;

    float fMinSafeDistSquared = 1.21 * pSelf->m_fMinDistanceSquared + 16384;

    if (m_fCurrentRangeSquared < fMinSafeDistSquared) {
        m_iThreat += 500 - (sqrt(m_fCurrentRangeSquared / fMinSafeDistSquared) * 500);
    }

    vec2_t vLine;
    VectorSub2D(m_vLastKnownPos, pSelf->origin, vLine);
    float     fDot = DotProduct2D(vLine, m_vLastKnownPos);
    Sentient *pSquadMate;
    for (pSquadMate = m_pEnemy->m_pNextSquadMate; pSquadMate != m_pEnemy; pSquadMate = pSquadMate->m_pNextSquadMate) {
        if (fDot > DotProduct2D(vLine, pSquadMate->origin)) {
            m_iThreat -= 4;
        }
    }

    if (m_pEnemy->IsSubclassOfActor()) {
        Actor *pActorEnemy = static_cast<Actor *>(m_pEnemy.Pointer());
        if (pActorEnemy->m_ThinkState == THINKSTATE_PAIN) {
            m_iThreat -= 2;
        }
    }

    fDot = vDelta * pSelf->orientation[0];
    if (fDot > 0) {
        if (m_fCurrentRangeSquared * 0.5 >= Square(fDot) && m_fCurrentRangeSquared * 0.5 > Square(fDot)) {
            m_iThreat++;
        }
    } else {
        if (m_fCurrentRangeSquared * 0.5 > Square(fDot)) {
            m_iThreat++;
        }
    }
    int iEnemyDiscount = m_pEnemy->m_iAttackerCount;

    if (m_pEnemy == pSelf->m_Enemy) {
        iEnemyDiscount -= 2;
        if (level.inttime < pSelf->m_iEnemyChangeTime + 1000) {
            m_iThreat += 5;
        }
    }

    if (iEnemyDiscount > 4) {
        iEnemyDiscount = 4;
    }
    m_iThreat -= iEnemyDiscount;

    if (m_pEnemy == pSelf->m_pLastAttacker) {
        m_iThreat += 5;
    }
    if (m_pEnemy == pSelf->m_FavoriteEnemy) {
        m_iThreat += 250;
    }
    return m_iThreat;
}

ActorEnemySet::ActorEnemySet()
{
    m_iCheckCount        = 0;
    m_pCurrentEnemy      = NULL;
    m_iCurrentThreat     = 0;
    m_fCurrentVisibility = 0.0;
}

ActorEnemy *ActorEnemySet::AddPotentialEnemy(Sentient *pEnemy)
{
    ActorEnemy NewEnemy;

    if (pEnemy->IsDead() || pEnemy->m_iThreatBias == THREATBIAS_IGNOREME) {
        return NULL;
    }

    for (int i = 0; i < m_Enemies.NumObjects(); i++) {
        ActorEnemy *pActorEnemy = &m_Enemies[i];
        if (pActorEnemy->m_pEnemy == pEnemy) {
            pActorEnemy->m_iAddTime = level.inttime;
            return pActorEnemy;
        }
    }

    NewEnemy.m_iAddTime             = level.inttime;
    NewEnemy.m_iNextEnemyTime       = 0;
    NewEnemy.m_fLastLookTime        = level.time;
    NewEnemy.m_fVisibility          = 0.0;
    NewEnemy.m_fTotalVisibility     = 0.0;
    NewEnemy.m_iThreat              = 0;
    NewEnemy.m_pEnemy               = pEnemy;
    NewEnemy.m_fCurrentRangeSquared = 1e38f;
    NewEnemy.m_vLastKnownPos        = vec_zero;
    NewEnemy.m_iLastSightChangeTime = 0;
    NewEnemy.m_bVisible             = false;

    int index = m_Enemies.AddObject(NewEnemy);
    return m_Enemies.AddressOfObjectAt(index);
}

void ActorEnemySet::FlagBadEnemy(Sentient *pEnemy)
{
    ActorEnemy *pActorEnemy = NULL;

    for (int i = 1; i <= m_Enemies.NumObjects(); i++) {
        if (m_Enemies.ObjectAt(i).m_pEnemy == pEnemy) {
            pActorEnemy = m_Enemies.AddressOfObjectAt(i);
            break;
        }
    }

    if (!pActorEnemy) {
        return;
    }

    // Added in 2.0.
    //  Set the reaction time
    pActorEnemy->m_iNextEnemyTime       = level.inttime + 5000;

    pActorEnemy->m_fLastLookTime        = level.time;
    pActorEnemy->m_fVisibility          = 0.0;
    pActorEnemy->m_fTotalVisibility     = 0.0;
    pActorEnemy->m_iThreat              = 0;
    pActorEnemy->m_iLastSightChangeTime = level.inttime;
    pActorEnemy->m_bVisible             = false;

    if (pEnemy == m_pCurrentEnemy) {
        m_pCurrentEnemy      = NULL;
        m_fCurrentVisibility = 0.0;
        m_iCurrentThreat     = 0;
    }
}

void ActorEnemySet::CheckEnemies(Actor *pSelf)
{
    float fVisibility;
    int   iThreat;
    int   i;
    int   nChecked;
    bool  bInFovAndRange;
    bool  bVisible;
    float fRangeSquared;

    ActorEnemy *pActorEnemy;

    for (i = 1; i <= m_Enemies.NumObjects();) {
        pActorEnemy = &m_Enemies[i - 1];
        if (!pActorEnemy->GetEnemy() || pActorEnemy->GetEnemy()->m_Team == pSelf->m_Team
            || pActorEnemy->GetEnemy()->IsDead() || level.inttime > pActorEnemy->m_iAddTime + 10000
            || pActorEnemy->GetEnemy()->m_iThreatBias == THREATBIAS_IGNOREME) {
            m_Enemies.RemoveObjectAt(i);
        } else {
            i++;
        }
    }

    if (!m_Enemies.NumObjects()) {
        m_iCurrentThreat = 0;
        m_pCurrentEnemy  = NULL;
        return;
    }

    bInFovAndRange = false;
    for (nChecked = 0; nChecked < m_Enemies.NumObjects() && !bInFovAndRange; nChecked++) {
        m_iCheckCount++;
        if (m_iCheckCount > m_Enemies.NumObjects()) {
            m_iCheckCount = 1;
        }

        pActorEnemy = &m_Enemies[m_iCheckCount - 1];

        fVisibility = pActorEnemy->UpdateVisibility(pSelf, &bInFovAndRange, &bVisible);
        if (fVisibility > 0) {
            if (fVisibility >= m_fCurrentVisibility) {
                m_fCurrentVisibility = fVisibility;
                m_pCurrentEnemy      = pActorEnemy->GetEnemy();
            } else if (pActorEnemy->m_pEnemy == m_pCurrentEnemy) {
                m_fCurrentVisibility = fVisibility;
            }

            if (g_showawareness->integer) {
                Com_Printf(
                    "ent #%3i: enemy #%i: awareness = %5.1f%%, threat = %i\n",
                    pSelf->entnum,
                    pActorEnemy->GetEnemy()->entnum,
                    (fVisibility * 100.0),
                    0
                );
            }
        } else if (pActorEnemy->m_pEnemy == m_pCurrentEnemy) {
            m_iCurrentThreat     = 0;
            m_pCurrentEnemy      = NULL;
            m_fCurrentVisibility = 0;
        }

        if (bVisible) {
            if (!pActorEnemy->m_bVisible) {
                pActorEnemy->m_bVisible             = true;
                pActorEnemy->m_iLastSightChangeTime = level.inttime;
            }
            pActorEnemy->m_vLastKnownPos = pActorEnemy->m_pEnemy->origin;
        } else if (pActorEnemy->m_bVisible) {
            pActorEnemy->m_bVisible             = false;
            pActorEnemy->m_iLastSightChangeTime = level.inttime;
        }
    }

    if (m_pCurrentEnemy && m_pCurrentEnemy->IsDead()) {
        m_pCurrentEnemy      = NULL;
        m_iCurrentThreat     = 0;
        m_fCurrentVisibility = 0;
    }

    m_iCurrentThreat = 0;
    fRangeSquared    = 1e37f;

    if (m_fCurrentVisibility >= 1) {
        iThreat = 0;

        for (i = 1; i <= m_Enemies.NumObjects(); i++) {
            pActorEnemy = &m_Enemies.ObjectAt(i);
            pActorEnemy->UpdateThreat(pSelf);

            if (m_iCurrentThreat >= pActorEnemy->GetThreat()) {
                if (m_iCheckCount != pActorEnemy->GetThreat()) {
                    continue;
                }

                if (pActorEnemy->GetRangeSquared() < fRangeSquared) {
                    continue;
                }
            }

            iThreat         = pActorEnemy->GetThreat();
            m_pCurrentEnemy = pActorEnemy->GetEnemy();
            fRangeSquared   = pActorEnemy->m_fCurrentRangeSquared;
        }

        m_iCurrentThreat = iThreat;
    }

    if ((!m_pCurrentEnemy || !m_pCurrentEnemy->m_bIsDisguised) && m_iCurrentThreat <= 0) {
        m_pCurrentEnemy      = NULL;
        m_iCurrentThreat     = 0;
        m_fCurrentVisibility = 0;
    }
}

bool ActorEnemySet::HasAlternateEnemy(void) const
{
    for (int i = 1; i <= m_Enemies.NumObjects(); i++) {
        const ActorEnemy& actorEnemy = m_Enemies.ObjectAt(i);

        if (actorEnemy.GetVisibility() >= 0.999f && actorEnemy.GetEnemy() != m_pCurrentEnemy) {
            return true;
        }
    }
    return false;
}

void ActorEnemySet::RemoveAll(void)
{
    m_Enemies.ClearObjectList();
    m_iCheckCount        = 0;
    m_pCurrentEnemy      = NULL;
    m_fCurrentVisibility = 0.0;
    m_iCurrentThreat     = 0;
}

void ActorEnemySet::ConfirmEnemy(Actor *pSelf, Sentient *pEnemy)
{
    ActorEnemy *pActorEnemy;

    pSelf->m_bEnemyIsDisguised = false;
    pActorEnemy                = ActorEnemySet::AddPotentialEnemy(pEnemy);
    if (!pActorEnemy) {
        return;
    }

    // Added in 2.0.
    //  Ignore if below reaction time
    if (level.inttime < pActorEnemy->m_iNextEnemyTime) {
        return;
    }

    pActorEnemy->m_fVisibility      = 1.0;
    pActorEnemy->m_fTotalVisibility = 1.0;
    pActorEnemy->m_vLastKnownPos    = pEnemy->origin;

    if (m_fCurrentVisibility < 1.0) {
        m_iCurrentThreat     = pActorEnemy->UpdateThreat(pSelf);
        m_fCurrentVisibility = 1.0;
        m_pCurrentEnemy      = pEnemy;
    }
}

void ActorEnemySet::ConfirmEnemyIfCanSeeSharerOrEnemy(Actor *pSelf, Actor *pSharer, Sentient *pEnemy)
{
    ActorEnemy *pActorEnemy;

    pSelf->m_bEnemyIsDisguised = false;
    pActorEnemy                = ActorEnemySet::AddPotentialEnemy(pEnemy);
    if (!pActorEnemy) {
        return;
    }

    // Added in 2.0.
    //  Ignore if below reaction time
    if (level.inttime < pActorEnemy->m_iNextEnemyTime) {
        return;
    }

    if (pActorEnemy->m_fTotalVisibility >= 1.f) {
        pActorEnemy->m_vLastKnownPos = pEnemy->origin;
        return;
    }

    if (!pActorEnemy->m_bVisible
        && !pSelf->CanSee(pSharer, pSelf->m_bSilent ? 90 : 0, world->farplane_distance * 0.828f, false)) {
        return;
    }

    pActorEnemy->m_fVisibility      = 1.0;
    pActorEnemy->m_fTotalVisibility = 1.0;
    pActorEnemy->m_vLastKnownPos    = pEnemy->origin;

    if (m_fCurrentVisibility < 1.0) {
        m_iCurrentThreat     = pActorEnemy->UpdateThreat(pSelf);
        m_fCurrentVisibility = 1.0;
        m_pCurrentEnemy      = pEnemy;
    }
}

bool ActorEnemySet::CaresAboutPerfectInfo(Sentient *pEnemy)
{
    ActorEnemy *pActorEnemy = ActorEnemySet::AddPotentialEnemy(pEnemy);
    if (!pActorEnemy) {
        return false;
    }

    // Added in 2.0.
    //  Ignore if below reaction time
    if (level.inttime < pActorEnemy->m_iNextEnemyTime) {
        return false;
    }

    if (pActorEnemy->m_fTotalVisibility < 1.0f) {
        return true;
    }

    if ((pEnemy->origin - pActorEnemy->m_vLastKnownPos).lengthSquared() <= Square(512)) {
        return false;
    }

    return true;
}
