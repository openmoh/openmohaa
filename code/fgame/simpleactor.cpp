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

// simpleactor.cpp: Base class for character AI

#include "actor.h"
#include "bg_local.h"
#include "scriptexception.h"
#include "scriptthread.h"
#include "../script/scriptclass.h"
#include "weapturret.h"
#include <tiki.h>

Event EV_NoAnimLerp
(
    "noanimlerp",
    EV_DEFAULT,
    NULL,
    NULL,
    "Do not LERP to the next animation",
    EV_NORMAL
);

CLASS_DECLARATION(Sentient, SimpleActor, NULL) {
    {&EV_NoAnimLerp, &SimpleActor::EventNoAnimLerp},
    {NULL,           NULL                         }
};

#define OVERLOADED_ERROR() assert(!"overloaded version should always get called")

SimpleActor::SimpleActor()
{
    m_ChangeMotionAnimIndex = -1;
    m_ChangeActionAnimIndex = -1;
    m_ChangeSayAnimIndex    = -1;

    if (LoadingSavegame) {
        return;
    }

    m_AnimMotionHigh = true;
    m_AnimActionHigh = true;
    m_AnimDialogHigh = true;

    m_fAimLimit_up   = 60.0f;
    m_fAimLimit_down = -60.0f;

    VectorClear(m_DesiredGunDir);
    VectorClear(m_DesiredLookAngles);
    VectorClear(m_Dest);
    VectorClear(m_NoClipDest);

    for (int i = 0; i < MAX_FRAMEINFOS; i++) {
        m_weightBase[i]       = 0.0f;
        m_weightCrossBlend[i] = 0.0f;
    }

    m_eEmotionMode            = EMOTION_NEUTRAL;
    m_eNextAnimMode           = -1;
    m_csPathGoalEndAnimScript = STRING_EMPTY;
    m_bNextForceStart         = false;
    m_fCrossblendTime         = 0.5f;
    m_csCurrentPosition       = STRING_STAND;
    m_fPathGoalTime           = 0.0f;
    m_bStartPathGoalEndAnim   = false;
    m_csNextAnimString        = STRING_NULL;

    for (int i = 0; i < MAX_FRAMEINFOS; i++) {
        m_weightType[i] = 0.0f;
    }

    m_ChangeActionAnimIndex = -1;
    m_ChangeActionAnimIndex = -1;
    m_ChangeSayAnimIndex    = -1;

    m_bMotionAnimSet = false;
    m_bActionAnimSet = false;
    m_bSayAnimSet    = false;
    m_iVoiceTime     = 0;
    m_bAimAnimSet    = false;

    m_iMotionSlot       = -1;
    m_iActionSlot       = -1;
    m_iSaySlot          = -1;
    m_bLevelMotionAnim  = false;
    m_bLevelActionAnim  = false;
    m_bLevelSayAnim     = false;
    m_bNextLevelSayAnim = false;
    m_DesiredYaw        = 0.0f;
    m_YawAchieved       = true;
    m_bPathErrorTime    = -10000000;

    m_PainHandler.TrySetScript(STRING_ANIM_PAIN_SCR);
    m_DeathHandler.TrySetScript(STRING_ANIM_KILLED_SCR);
    m_AttackHandler.TrySetScript(STRING_ANIM_ATTACK_SCR);
    m_SniperHandler.TrySetScript(STRING_ANIM_SNIPER_SCR);

    m_bHasDesiredLookDest  = false;
    m_bUpdateAnimDoneFlags = false;

    m_NearestNode   = NULL;
    m_fCrouchWeight = 0.0f;

    m_csMood     = STRING_BORED;
    m_csIdleMood = STRING_BORED;

    m_groundPlane = qfalse;
    m_walking     = qfalse;
    VectorClear(m_groundPlaneNormal);
    m_maxspeed = 1000000.0f;
}

SimpleActor::~SimpleActor()
{
    if (m_pAnimThread) {
        delete m_pAnimThread->GetScriptClass();
    }
}

void SimpleActor::SetMoveInfo(mmove_t *)
{
    OVERLOADED_ERROR();
}

void SimpleActor::GetMoveInfo(mmove_t *)
{
    OVERLOADED_ERROR();
}

bool SimpleActor::CanSeeFrom(vec3_t pos, Entity *ent)
{
    return false;
}

bool SimpleActor::CanTarget(void)
{
    OVERLOADED_ERROR();
    return false;
}

bool SimpleActor::IsImmortal(void)
{
    OVERLOADED_ERROR();
    return false;
}

bool SimpleActor::DoesTheoreticPathExist(Vector vDestPos, float fMaxPath)
{
    return m_Path.DoesTheoreticPathExist(origin, vDestPos, this, fMaxPath, NULL, 0);
}

void SimpleActor::SetPath(
    Vector vDestPos, const char *description, int iMaxDirtyTime, float *vLeashHome, float fLeashDistSquared
)
{
    if (PathExists()) {
        if (level.inttime < iMaxDirtyTime + m_Path.Time() && !m_Path.Complete(origin)) {
            // Too soon
            return;
        }

        if (PathGoal() == vDestPos && PathIsValid()) {
            // Still a valid path
            return;
        }
    }

    m_Path.FindPath(origin, vDestPos, this, 0.0, vLeashHome, fLeashDistSquared);

    if (!PathExists()) {
        if (g_patherror->integer && description
            && (g_patherror->integer
                || g_patherror->integer == 2
                       && (static_cast<Actor *>(this)->m_ThinkState == THINKSTATE_IDLE
                           || static_cast<Actor *>(this)->m_ThinkState == THINKSTATE_CURIOUS)
                       && m_bPathErrorTime + 5000 < level.inttime)) {
            m_bPathErrorTime = level.inttime;

            Com_Printf(
                "^~^~^ Path not found in '%s' for (entnum %d, radnum %d, targetname '%s') from (%f %f "
                "%f) to (%f %f %f)\n",
                description,
                entnum,
                radnum,
                targetname.c_str(),
                origin.x,
                origin.y,
                origin.z,
                vDestPos.x,
                vDestPos.y,
                vDestPos.z
            );
            Com_Printf("Reason: %s\n", PathSearch::last_error);
        }
    }
}

void SimpleActor::SetPath(SimpleEntity *pDestNode, const char *description, int iMaxDirtyTime)
{
    if (pDestNode) {
        SetPath(pDestNode->origin, description, iMaxDirtyTime, NULL, 0.0);
        return;
    }

    if (m_bPathErrorTime + 5000 < level.inttime) {
        m_bPathErrorTime = level.inttime;
        Com_Printf(
            "^~^~^ No destination node specified for '%s' at (%f %f %f)\n",
            targetname.c_str(),
            origin.x,
            origin.y,
            origin.z
        );
    }

    ClearPath();
}

void SimpleActor::SetPathWithinDistance(Vector vDestPos, char *description, float fMaxPath, int iMaxDirtyTime)
{
    if (PathExists()) {
        if (level.inttime < iMaxDirtyTime + m_Path.Time()) {
            // Too soon
            return;
        }

        if (!m_Path.Complete(origin)) {
            // The current path has not complete yet
            return;
        }

        if (PathGoal() == vDestPos && PathIsValid()) {
            // Still a valid path
            return;
        }
    }

    m_Path.FindPath(origin, vDestPos, this, fMaxPath, 0, 0);

    if (!PathExists()) {
        if (g_patherror->integer && description
            && (g_patherror->integer
                || g_patherror->integer == 2
                       && (static_cast<Actor *>(this)->m_ThinkState == THINKSTATE_IDLE
                           || static_cast<Actor *>(this)->m_ThinkState == THINKSTATE_CURIOUS)
                       && m_bPathErrorTime + 5000 < level.inttime)) {
            m_bPathErrorTime = level.inttime;

            Com_Printf(
                "^~^~^ Path not found in '%s' for '%s' from (%f %f %f) to (%f %f %f)\n",
                description,
                targetname.c_str(),
                origin.x,
                origin.y,
                origin.z,
                vDestPos.x,
                vDestPos.y,
                vDestPos.z
            );
            Com_Printf("Reason: %s\n", PathSearch::last_error);
        }
    }
}

void SimpleActor::FindPathAway(vec3_t vAwayFrom, vec2_t vDirPreferred, float fMinSafeDist)
{
    m_Path.FindPathAway(origin, vAwayFrom, vDirPreferred, this, fMinSafeDist, NULL, 0);

    ShortenPathToAvoidSquadMates();
}

void SimpleActor::ClearPath(void)
{
    m_Path.Clear();
}

bool SimpleActor::PathComplete(void) const
{
    if (level.time < m_fPathGoalTime) {
        return false;
    }

    return m_Path.Complete(origin);
}

bool SimpleActor::PathExists(void) const
{
    return m_Path.CurrentNode() != NULL;
}

bool SimpleActor::PathIsValid(void) const
{
    //Called by SetPath...
    return true;
}

bool SimpleActor::PathAvoidsSquadMates(void) const
{
    Entity   *player;
    float     fDelta;
    float     fDistSoFar;
    float     fDistCap;
    vec2_t    vDelta2;
    vec3_t    vMins, vMaxs;
    vec3_t    vPos;
    PathInfo *pNode;
    Sentient *pOther;
    Sentient *pBuddy[256];
    int       iNumBuddies;
    int       i;
    float     fRatio;

    if (ai_pathchecktime->value <= 0.0) {
        return true;
    }

    player = static_cast<Sentient *>(G_GetEntity(0));
    if (!player) {
        return true;
    }

    VectorSub2D(player->origin, origin, vDelta2);
    if (VectorLength2D(vDelta2) > Square(ai_pathcheckdist->value)) {
        return true;
    }

    pNode = CurrentPathNode();
    VectorCopy(pNode->point, vMins);
    VectorCopy(pNode->point, vMaxs);

    fDistCap   = (ai_pathchecktime->value * 250.0);
    fDistSoFar = 0;

    for (pNode = CurrentPathNode() - 1; pNode >= LastPathNode(); pNode--) {
        if (fDistSoFar >= fDistCap) {
            break;
        }

        fDelta = fDistCap + 0.001 - fDistSoFar;

        if (fDelta > pNode->dist) {
            VectorCopy(pNode->point, vPos);
        } else {
            VectorSubtract(pNode[1].point, pNode[0].point, vPos);
            VectorMA(pNode[1].point, fDelta / pNode->dist, vPos, vPos);
        }

        fDistSoFar += fDelta;

        for (i = 0; i < 3; i++) {
            if (vMaxs[i] < vPos[i]) {
                vMaxs[i] = vPos[i];
            } else if (vMins[i] > vPos[i]) {
                vMins[i] = vPos[i];
            }
        }
    }

    vMins[0] -= 30;
    vMins[1] -= 30;
    vMins[2] -= 94;

    vMaxs[0] += 30;
    vMaxs[1] += 30;
    vMaxs[2] += 94;

    iNumBuddies = 0;
    for (pOther = m_pNextSquadMate; pOther != this && iNumBuddies < ARRAY_LEN(pBuddy);
         pOther = pOther->m_pNextSquadMate) {
        if (vMins[0] >= pOther->origin[0] || pOther->origin[0] >= vMaxs[0]) {
            continue;
        }
        if (vMins[1] >= pOther->origin[1] || pOther->origin[1] >= vMaxs[1]) {
            continue;
        }
        if (vMins[2] >= pOther->origin[2] || pOther->origin[2] >= vMaxs[2]) {
            continue;
        }

        VectorSub2D(pOther->origin, origin, vDelta2);
        if (vDelta2[0] <= -32 || vDelta2[0] >= 32 || vDelta2[1] <= -32 || vDelta2[1] >= 32) {
            if (DotProduct2D(vDelta2, pOther->velocity) <= 0) {
                pBuddy[iNumBuddies++] = pOther;
            }
        }
    }

    if (!iNumBuddies) {
        // No buddy, can safely avoid
        return true;
    }

    do {
        for (i = 0; i < iNumBuddies; i++) {
            VectorSub2D(pOther->origin, vPos, vDelta2);

            if (VectorLength2DSquared(vDelta2) <= 900) {
                return false;
            }

            fRatio = DotProduct2D(vDelta2, pNode->dir);
            if (fRatio < 0 && fRatio >= -fDelta) {
                vec2_t vInvDelta2 = {vDelta2[1], vDelta2[0]};

                if (Square(DotProduct2D(vInvDelta2, pNode->dir)) <= 900) {
                    return false;
                }
            }
        }

        pNode++;

        VectorCopy2D(pNode->point, vPos);
        fDelta = pNode->dist;
    } while (pNode < CurrentPathNode());

    return true;
}

void SimpleActor::ShortenPathToAvoidSquadMates(void)
{
    if (!PathExists() || PathComplete()) {
        return;
    }

    Vector    vGoal;
    Sentient *pBuddy;

retry:
    vGoal = PathGoal();

    for (pBuddy = m_pNextSquadMate; pBuddy != this; pBuddy = pBuddy->m_pNextSquadMate) {
        Vector vBuddyPos;
        Vector vDelta;

        vBuddyPos = pBuddy->origin;
        if (pBuddy->IsSubclassOfActor()) {
            Actor *pBuddyActor = static_cast<Actor *>(pBuddy);
            if (pBuddyActor->PathExists()) {
                vBuddyPos = pBuddyActor->PathGoal();
            }
        }

        vDelta = vGoal - vBuddyPos;

        if (vDelta.x >= -15 && vDelta.x <= 15 && vDelta.y >= -15 && vDelta.y <= 15 && vDelta.z >= 0 && vDelta.z <= 94) {
            m_Path.Shorten(45.0);

            if (!PathExists()) {
                return;
            }

            goto retry;
        }
    }
}

Vector SimpleActor::PathGoal(void) const
{
    return m_Path.CurrentPathGoal();
}

bool SimpleActor::PathGoalSlowdownStarted(void) const
{
    return m_fPathGoalTime >= level.time;
}

const float *SimpleActor::PathDelta(void) const
{
    return m_Path.CurrentDelta();
}

PathInfo *SimpleActor::CurrentPathNode(void) const
{
    return m_Path.CurrentNode();
}

PathInfo *SimpleActor::LastPathNode(void) const
{
    return m_Path.LastNode();
}

float SimpleActor::PathDist(void) const
{
    return m_Path.TotalDist();
}

bool SimpleActor::PathHasCompleteLookahead(void) const
{
    return m_Path.HasCompleteLookahead();
}

void SimpleActor::UpdateEmotion(void)
{
    int anim;

    if (deadflag != DEAD_NO) {
        m_eEmotionMode = EMOTION_DEAD;
    }

    anim = GetEmotionAnim();

    if (anim == -1) {
        Com_Printf(
            "Failed to set emotion for (entnum %d, radnum %d, targetname '%s'\n", entnum, radnum, TargetName().c_str()
        );
        return;
    }

    m_bSayAnimSet = true;
    StartSayAnimSlot(anim);
}

int SimpleActor::GetMotionSlot(int slot)
{
    if (m_AnimMotionHigh) {
        return slot + 3;
    } else {
        return slot;
    }
}

void SimpleActor::ChangeMotionAnim(void)
{
    int slot;
    int lastMotionSlot;
    int firstMotionSlot;

    m_bMotionAnimSet   = false;
    m_iMotionSlot      = -1;
    m_bLevelMotionAnim = false;

    if (m_ChangeMotionAnimIndex != level.frame_skel_index) {
        m_ChangeMotionAnimIndex = level.frame_skel_index;

        firstMotionSlot = GetMotionSlot(0);
        lastMotionSlot  = firstMotionSlot + 3;

        MPrintf("Swapping motion channels....\n");
        for (slot = firstMotionSlot; slot < lastMotionSlot; slot++) {
            StartCrossBlendAnimSlot(slot);

            // Added in 2.0
            //  Don't lerp animations
            if (edict->s.eFlags & EF_NO_LERP) {
                m_weightCrossBlend[slot] = 0;
                m_weightBase[slot]       = 0;
                // Next animation should lerp
                edict->s.eFlags &= ~EF_NO_LERP;
            }
        }

        m_AnimMotionHigh = !m_AnimMotionHigh;
    }

    firstMotionSlot = GetMotionSlot(0);
    lastMotionSlot  = firstMotionSlot + 3;

    for (slot = firstMotionSlot; slot < lastMotionSlot; slot++) {
        StopAnimating(slot);
    }
}

int SimpleActor::GetActionSlot(int slot)
{
    if (m_AnimActionHigh) {
        return slot + 9;
    } else {
        return slot + 6;
    }
}

void SimpleActor::ChangeActionAnim(void)
{
    int slot;
    int firstActionSlot;
    int lastActionSlot;

    m_bAimAnimSet      = false;
    m_bActionAnimSet   = false;
    m_iActionSlot      = -1;
    m_bLevelActionAnim = false;

    if (m_ChangeActionAnimIndex != level.frame_skel_index) {
        m_ChangeActionAnimIndex = level.frame_skel_index;

        firstActionSlot = GetActionSlot(0);
        lastActionSlot  = firstActionSlot + 3;

        MPrintf("Swapping action channels....\n");

        for (slot = firstActionSlot; slot < lastActionSlot; slot++) {
            animFlags[slot] |= ANIM_NOACTION;
            StartCrossBlendAnimSlot(slot);
        }

        m_AnimActionHigh = !m_AnimActionHigh; // toggle
    }

    firstActionSlot = GetActionSlot(0);
    lastActionSlot  = firstActionSlot + 3;

    for (slot = firstActionSlot; slot < lastActionSlot; slot++) {
        StopAnimating(slot);
    }
}

int SimpleActor::GetSaySlot(void)
{
    return m_AnimDialogHigh ? 13 : 12;
}

void SimpleActor::ChangeSayAnim(void)
{
    m_bSayAnimSet   = false;
    m_iVoiceTime    = level.inttime;
    m_iSaySlot      = -1;
    m_bLevelSayAnim = false;

    if (m_ChangeSayAnimIndex != level.frame_skel_index) {
        m_ChangeSayAnimIndex = level.frame_skel_index;

        MPrintf("Swapping dialog channel....\n");

        StartCrossBlendAnimSlot(GetSaySlot());

        m_AnimDialogHigh = !m_AnimDialogHigh; // toggle
    }

    StopAnimating(GetSaySlot());
}

void SimpleActor::StopAnimating(int slot)
{
    int index = 0;

    m_weightType[slot] = 0;
    DoExitCommands(slot);

    if (edict->s.frameInfo[slot].index || gi.TIKI_NumAnims(edict->tiki) <= 1) {
        edict->s.frameInfo[slot].index = 0;
    } else {
        edict->s.frameInfo[slot].index = 1;
    }

    animFlags[slot] = ANIM_LOOP | ANIM_NODELTA | ANIM_NOEXIT | ANIM_PAUSED;
    SetWeight(slot, 0);

    animtimes[slot] = 0;
    SlotChanged(slot);
}

void SimpleActor::EventSetAnimLength(Event *ev)
{
    int   slot;
    float length;

    if (ev->NumArgs() != 1) {
        ScriptError("bad number of arguments");
    }

    length = ev->GetFloat(1);

    if (length <= 0) {
        ScriptError("Positive lengths only allowed");
    }

    if (!m_bMotionAnimSet) {
        ScriptError("Must set anim before length");
    }

    slot = GetMotionSlot(0);

    if (IsRepeatType(slot) && edict->tiki) {
        int numframes;

        numframes       = gi.Anim_NumFrames(edict->tiki, edict->s.frameInfo[slot].index);
        animtimes[slot] = gi.Anim_Frametime(edict->tiki, edict->s.frameInfo[slot].index) * numframes;

        SlotChanged(slot);
        SetOnceType(slot);
    }

    SetSyncTime(0);

    if (length > animtimes[slot]) {
        ScriptError("cannot lengthen animation which has length %f", animtimes[slot]);
    }

    animtimes[slot] = length;
    SlotChanged(slot);
}

void SimpleActor::EventSetCrossblendTime(Event *ev)
{
    m_fCrossblendTime = ev->GetFloat(1);
}

void SimpleActor::EventGetCrossblendTime(Event *ev)
{
    ev->AddFloat(m_fCrossblendTime);
}

void SimpleActor::StartCrossBlendAnimSlot(int slot)
{
    if (m_weightType[slot] == ANIM_WEIGHT_NONE) {
        return;
    }

    switch (m_weightType[slot]) {
    case ANIM_WEIGHT_MOTION:
        m_weightType[slot] = ANIM_WEIGHT_CROSSBLEND_2;
        break;
    case ANIM_WEIGHT_SAY:
        m_weightType[slot] = ANIM_WEIGHT_CROSSBLEND_DIALOG;
        break;
    default:
        m_weightType[slot] = ANIM_WEIGHT_CROSSBLEND_1;
        break;
    }

    m_weightCrossBlend[slot] = 1.0;
    m_weightBase[slot]       = GetWeight(slot);
}

void SimpleActor::StartMotionAnimSlot(int slot, int anim, float weight)
{
    slot = GetMotionSlot(slot);

    m_weightType[slot]       = ANIM_WEIGHT_MOTION;
    m_weightCrossBlend[slot] = 0.0;
    m_weightBase[slot]       = weight;
    NewAnim(anim, slot, 1.0);

    animFlags[slot] |= ANIM_NOACTION;
    RestartAnimSlot(slot);
    UpdateNormalAnimSlot(slot);
}

void SimpleActor::StartAimMotionAnimSlot(int slot, int anim)
{
    slot = GetMotionSlot(slot);

    m_weightType[slot]       = ANIM_WEIGHT_MOTION;
    m_weightCrossBlend[slot] = 0.0;
    NewAnim(anim, slot, 1.0);

    animFlags[slot] |= ANIM_NOACTION;
    RestartAnimSlot(slot);
    UpdateNormalAnimSlot(slot);
}

void SimpleActor::StartActionAnimSlot(int anim)
{
    int slot = GetActionSlot(0);

    m_weightType[slot]       = ANIM_WEIGHT_ACTION;
    m_weightCrossBlend[slot] = 0.0;
    m_weightBase[slot]       = 1.0;
    NewAnim(anim, slot, 1.0);

    RestartAnimSlot(slot);
    UpdateNormalAnimSlot(slot);
}

void SimpleActor::StartSayAnimSlot(int anim)
{
    int slot = GetSaySlot();

    m_weightType[slot]       = ANIM_WEIGHT_SAY;
    m_weightCrossBlend[slot] = 0.0;
    m_weightBase[slot]       = 1.0;
    NewAnim(anim, slot, 1.0);

    animFlags[slot] |= ANIM_NOACTION;
    RestartAnimSlot(slot);
    UpdateNormalAnimSlot(slot);
}

void SimpleActor::StartAimAnimSlot(int slot, int anim)
{
    slot = GetActionSlot(slot);

    m_weightType[slot]       = ANIM_WEIGHT_AIM;
    m_weightCrossBlend[slot] = 0.0;
    m_weightBase[slot]       = 1.0;
    NewAnim(anim, slot, 1.0);

    RestartAnimSlot(slot);
    UpdateNormalAnimSlot(slot);
}

void SimpleActor::SetBlendedWeight(int slot)
{
    m_bUpdateAnimDoneFlags |= 1 << slot;

    if (m_weightCrossBlend[slot] < 1.0) {
        float w;

        w = (3.0 - m_weightCrossBlend[slot] - m_weightCrossBlend[slot]) * Square(m_weightCrossBlend[slot]);
        SetWeight(slot, m_weightBase[slot] * w);
    } else {
        m_weightCrossBlend[slot] = 1.0;
        SetWeight(slot, m_weightBase[slot]);
    }
}

void SimpleActor::UpdateNormalAnimSlot(int slot)
{
    if (m_fCrossblendTime) {
        m_weightCrossBlend[slot] += level.frametime / m_fCrossblendTime;
    } else {
        m_weightCrossBlend[slot] += 1.f;
    }
    SetBlendedWeight(slot);
}

void SimpleActor::UpdateCrossBlendAnimSlot(int slot)
{
    if (m_fCrossblendTime) {
        m_weightCrossBlend[slot] -= level.frametime / m_fCrossblendTime;
    } else {
        m_weightCrossBlend[slot] -= 1.f;
    }

    if (m_weightCrossBlend[slot] > 0) {
        SetBlendedWeight(slot);
    } else {
        m_weightType[slot] = ANIM_WEIGHT_LASTFRAME;
        SetWeight(slot, 0);
    }
}

void SimpleActor::UpdateCrossBlendDialogAnimSlot(int slot)
{
    if (m_iSaySlot >= 0) {
        m_weightCrossBlend[m_iSaySlot] -= level.frametime / 0.1f;
    } else {
        m_weightCrossBlend[m_iSaySlot] -= level.frametime / 0.5f;
    }

    if (m_weightCrossBlend[slot] > 0.0) {
        SetBlendedWeight(slot);
    } else {
        m_weightType[slot] = ANIM_WEIGHT_LASTFRAME;
        SetWeight(slot, 0);
    }
}

void SimpleActor::UpdateSayAnimSlot(int slot)
{
    if (m_iSaySlot >= 0) {
        m_weightCrossBlend[m_iSaySlot] += level.frametime / 0.1f;
    } else {
        m_weightCrossBlend[m_iSaySlot] += level.frametime / 0.5f;
    }

    SetBlendedWeight(slot);
}

void SimpleActor::UpdateLastFrameSlot(int slot)
{
    StopAnimating(slot);
}

void SimpleActor::UpdateAnimSlot(int slot)
{
    switch (m_weightType[slot]) {
    case ANIM_WEIGHT_NONE:
        break;
    case ANIM_WEIGHT_MOTION:
    case ANIM_WEIGHT_ACTION:
    case ANIM_WEIGHT_AIM:
        UpdateNormalAnimSlot(slot);
        break;
    case ANIM_WEIGHT_CROSSBLEND_1:
    case ANIM_WEIGHT_CROSSBLEND_2:
        UpdateCrossBlendAnimSlot(slot);
        break;
    case ANIM_WEIGHT_CROSSBLEND_DIALOG:
        UpdateCrossBlendDialogAnimSlot(slot);
        break;
    case ANIM_WEIGHT_SAY:
        UpdateSayAnimSlot(slot);
        break;
    case ANIM_WEIGHT_LASTFRAME:
        UpdateLastFrameSlot(slot);
        break;
    default:
        assert(!"SimpleActor::UpdateAnimSlot: Bad weight type.");
        break;
    }
}

void SimpleActor::StopAllAnimating(void)
{
    SetSyncTime(0);

    for (int slot = 0; slot < MAX_FRAMEINFOS; slot++) {
        StopAnimating(slot);
    }
}

void SimpleActor::UpdateAim(void)
{
    int   aimUpSlot;
    int   aimForwardSlot;
    int   aimDownSlot;
    float dir;

    if (m_bAimAnimSet) {
        aimForwardSlot = GetActionSlot(0);
        aimUpSlot      = aimForwardSlot + 1;
        aimDownSlot    = aimForwardSlot + 2;
        dir            = -m_DesiredGunDir[0];

        if (dir > 180) {
            dir -= 360;
        } else if (dir < -180) {
            dir += 360;
        }

        if (dir < 0) {
            if (dir < m_fAimLimit_down) {
                dir = m_fAimLimit_down;
            }

            m_weightBase[aimForwardSlot] = 0;
            m_weightBase[aimUpSlot]      = 1 - dir / m_fAimLimit_down;
            m_weightBase[aimDownSlot]    = dir / m_fAimLimit_down;
        } else {
            if (dir > m_fAimLimit_up) {
                dir = m_fAimLimit_up;
            }

            m_weightBase[aimForwardSlot] = dir / m_fAimLimit_up;
            m_weightBase[aimUpSlot]      = 1 - dir / m_fAimLimit_up;
            m_weightBase[aimDownSlot]    = 0;
        }

        SetControllerAngles(TORSO_TAG, vec_origin);
    }
}

void SimpleActor::UpdateAimMotion(void)
{
    if (m_fCrouchWeight < 0.0) {
        m_weightBase[GetMotionSlot(0)] = 0.0;
        m_weightBase[GetMotionSlot(1)] = m_fCrouchWeight + 1.0;
        m_weightBase[GetMotionSlot(2)] = -m_fCrouchWeight;
    } else {
        m_weightBase[GetMotionSlot(0)] = m_fCrouchWeight;
        m_weightBase[GetMotionSlot(1)] = 1.0 - m_fCrouchWeight;
        m_weightBase[GetMotionSlot(2)] = 0.0;
    }
}

void SimpleActor::EventGetPosition(Event *ev)
{
    ev->AddConstString(m_csCurrentPosition);
}

void SimpleActor::EventSetPosition(Event *ev)
{
    m_csCurrentPosition = ev->GetConstString(1);
}

void SimpleActor::EventSetEmotion(Event *ev)
{
    switch (ev->GetConstString(1)) {
    case STRING_EMOTION_NONE:
        Anim_Emotion(EMOTION_NONE);
        break;
    case STRING_EMOTION_NEUTRAL:
        Anim_Emotion(EMOTION_NEUTRAL);
        break;
    case STRING_EMOTION_WORRY:
        Anim_Emotion(EMOTION_WORRY);
        break;
    case STRING_EMOTION_PANIC:
        Anim_Emotion(EMOTION_PANIC);
        break;
    case STRING_EMOTION_FEAR:
        Anim_Emotion(EMOTION_FEAR);
        break;
    case STRING_EMOTION_DISGUST:
        Anim_Emotion(EMOTION_DISGUST);
        break;
    case STRING_EMOTION_ANGER:
        Anim_Emotion(EMOTION_ANGER);
        break;
    case STRING_EMOTION_AIMING:
        Anim_Emotion(EMOTION_AIMING);
        break;
    case STRING_EMOTION_DETERMINED:
        Anim_Emotion(EMOTION_DETERMINED);
        break;
    case STRING_EMOTION_DEAD:
        Anim_Emotion(EMOTION_DEAD);
        break;
    case STRING_EMOTION_CURIOUS:
        Anim_Emotion(EMOTION_CURIOUS);
        break;
    default:
        assert(!"Unknown emotion mode specified in script.");
        break;
    }
}

int SimpleActor::GetEmotionAnim(void)
{
    const char *emotionanim = NULL;
    int         anim;

    if (m_eEmotionMode) {
        switch (m_eEmotionMode) {
        case EMOTION_NEUTRAL:
            emotionanim = "facial_idle_neutral";
            break;
        case EMOTION_WORRY:
            emotionanim = "facial_idle_worry";
            break;
        case EMOTION_PANIC:
            emotionanim = "facial_idle_panic";
            break;
        case EMOTION_FEAR:
            emotionanim = "facial_idle_fear";
            break;
        case EMOTION_DISGUST:
            emotionanim = "facial_idle_disgust";
            break;
        case EMOTION_ANGER:
            emotionanim = "facial_idle_anger";
            break;
        case EMOTION_AIMING:
            emotionanim = "facial_idle_neutral";
            break;
        case EMOTION_DETERMINED:
            emotionanim = "facial_idle_determined";
            break;
        case EMOTION_DEAD:
            emotionanim = "facial_idle_dead";
            break;
        case EMOTION_CURIOUS:
            emotionanim = "facial_idle_determined";
            break;
        default:

            char assertStr[16317] = {0};
            strcpy(assertStr, "\"Unknown value for m_EmotionMode in SimpleActor::GetEmotionAnim\"\n\tMessage: ");
            Q_strcat(assertStr, sizeof(assertStr), DumpCallTrace(""));
            assert(!assertStr);
            return -1;
            break;
        }
    } else {
        switch (m_csMood) {
        case STRING_NERVOUS:
            emotionanim = "facial_idle_determined";
            break;
        case STRING_CURIOUS:
            emotionanim = "facial_idle_determined";
            break;
        case STRING_ALERT:
            emotionanim = "facial_idle_anger";
            break;
        case STRING_BORED:
            emotionanim = "facial_idle_neutral";
            break;
        default:

            char assertStr[16317] = {0};
            strcpy(assertStr, "\"Unknown value for m_csMood in SimpleActor::GetEmotionAnim\"\n\tMessage: ");
            Q_strcat(assertStr, sizeof(assertStr), DumpCallTrace(""));
            assert(!assertStr);
            return -1;
            break;
        }
    }

    assert(emotionanim);

    anim = gi.Anim_NumForName(edict->tiki, emotionanim);
    if (anim == -1) {
        Com_Printf(
            "^~^~^ SimpleActor::GetEmotionAnim: unknown animation '%s' in '%s'\n", emotionanim, edict->tiki->a->name
        );
    }
    return anim;
}

void SimpleActor::EventSetAnimFinal(Event *ev)
{
    ScriptError("animfinal is obsolete");
}

void SimpleActor::EventGetWeaponType(Event *ev)
{
    Weapon   *weapon;
    const_str csWeaponType;

    if (m_pTurret) {
        weapon = m_pTurret;
    } else {
        weapon = GetActiveWeapon(WEAPON_MAIN);
    }

    if (weapon) {
        switch (weapon->GetWeaponClass()) {
        case WEAPON_CLASS_PISTOL:
            csWeaponType = STRING_PISTOL;
            break;
        case WEAPON_CLASS_RIFLE:
            csWeaponType = STRING_RIFLE;
            break;
        case WEAPON_CLASS_SMG:
            csWeaponType = STRING_SMG;
            break;
        case WEAPON_CLASS_MG:
            csWeaponType = STRING_MG;
            break;
        case WEAPON_CLASS_GRENADE:
            csWeaponType = STRING_GRENADE;
            break;
        case WEAPON_CLASS_HEAVY:
            csWeaponType = STRING_HEAVY;
            break;
        case WEAPON_CLASS_CANNON:
            csWeaponType = STRING_CANNON;
            break;
        case WEAPON_CLASS_ITEM:
            csWeaponType = STRING_ITEM;
            break;
        case WEAPON_CLASS_ITEM2:
            csWeaponType = STRING_ITEM2;
            break;
        case WEAPON_CLASS_ITEM3:
            csWeaponType = STRING_ITEM3;
            break;
        case WEAPON_CLASS_ITEM4:
            csWeaponType = STRING_ITEM4;
            break;
        default:
            csWeaponType = STRING_EMPTY;
            break;
        }
    } else {
        csWeaponType = STRING_RIFLE;
    }

    ev->AddConstString(csWeaponType);
}

void SimpleActor::EventGetWeaponGroup(Event *ev)
{
    const_str csWeaponGroup;
    Weapon   *weapon = GetActiveWeapon(WEAPON_MAIN);

    if (weapon) {
        csWeaponGroup = weapon->GetWeaponGroup();
        if (csWeaponGroup == STRING_EMPTY) {
            csWeaponGroup = STRING_UNARMED;
        }
    } else {
        csWeaponGroup = STRING_UNARMED;
    }

    ev->AddConstString(csWeaponGroup);
}

void SimpleActor::EventAIOn(Event *ev)
{
    m_bDoAI = true;
}

void SimpleActor::EventAIOff(Event *ev)
{
    m_bDoAI = false;
}

void SimpleActor::AnimFinished(int slot)
{
    assert(!DumpCallTrace("\"never should be called\"\n\tMessage: "));
}

void SimpleActor::EventGetPainHandler(Event *ev)
{
    ScriptVariable var;

    m_PainHandler.GetScriptValue(&var);
    ev->AddValue(var);
}

void SimpleActor::EventSetPainHandler(Event *ev)
{
    if (ev->IsFromScript()) {
        m_PainHandler.SetScript(ev->GetValue(1));
    } else {
        m_PainHandler.SetScript(ev->GetString(1));
    }
}

void SimpleActor::EventGetDeathHandler(Event *ev)
{
    ScriptVariable var;

    m_DeathHandler.GetScriptValue(&var);
    ev->AddValue(var);
}

void SimpleActor::EventSetDeathHandler(Event *ev)
{
    if (ev->IsFromScript()) {
        m_DeathHandler.SetScript(ev->GetValue(1));
    } else {
        m_DeathHandler.SetScript(ev->GetString(1));
    }
}

void SimpleActor::EventGetAttackHandler(Event *ev)
{
    ScriptVariable var;

    m_AttackHandler.GetScriptValue(&var);
    ev->AddValue(var);
}

void SimpleActor::EventSetAttackHandler(Event *ev)
{
    if (ev->IsFromScript()) {
        m_AttackHandler.SetScript(ev->GetValue(1));
    } else {
        m_AttackHandler.SetScript(ev->GetString(1));
    }
}

void SimpleActor::EventGetSniperHandler(Event *ev)
{
    ScriptVariable var;

    m_SniperHandler.GetScriptValue(&var);
    ev->AddValue(var);
}

void SimpleActor::EventSetSniperHandler(Event *ev)
{
    if (ev->IsFromScript()) {
        m_SniperHandler.SetScript(ev->GetValue(1));
    } else {
        m_SniperHandler.SetScript(ev->GetString(1));
    }
}

bool SimpleActor::UpdateSelectedAnimation(void)
{
    if (m_csNextAnimString != STRING_NULL) {
        if (!m_bNextForceStart && m_pAnimThread && m_eAnimMode == m_eNextAnimMode
            && (m_fPathGoalTime > level.time || m_Anim.IsFile(m_csNextAnimString))) {
            // Clear the next animation
            m_eNextAnimMode = -1;

            if (!m_bStartPathGoalEndAnim) {
                return false;
            }
            m_bStartPathGoalEndAnim = false;

            if (m_Anim.IsFile(m_csPathGoalEndAnimScript)) {
                return false;
            }

            m_Anim.TrySetScript(m_csPathGoalEndAnimScript);
            return true;
        }

        m_Anim.TrySetScript(m_csNextAnimString);
    } else {
        if (!m_bNextForceStart && m_pAnimThread && m_eAnimMode == m_eNextAnimMode && m_Anim == m_NextAnimLabel) {
            m_bStartPathGoalEndAnim = false;
            m_eNextAnimMode         = -1;
            return false;
        }

        m_Anim = m_NextAnimLabel;
    }

    m_eAnimMode = m_eNextAnimMode;

    if (m_eAnimMode != ANIM_MODE_PATH_GOAL) {
        m_csPathGoalEndAnimScript = STRING_EMPTY;
    }

    m_bStartPathGoalEndAnim = false;
    m_eNextAnimMode         = -1;

    return true;
}

void SimpleActor::EventNoAnimLerp(Event *ev)
{
    edict->s.eFlags |= EF_NO_LERP;
    NoLerpThisFrame();
}

const char *SimpleActor::DumpCallTrace(const char *pszFmt, ...) const
{
    OVERLOADED_ERROR();
    return "overloaded version should always get called";
}

void SimpleActor::EventGetAnimMode(Event *ev) {}

void SimpleActor::EventSetAnimMode(Event *ev) {}
