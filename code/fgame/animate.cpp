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
// animate.cpp : Animate Class

#include "animate.h"
#include "player.h"
#include "qfiles.h"
#include "scriptexception.h"
#include "g_spawn.h"
#include "animationevent.h"
#include <tiki.h>

extern Event EV_Entity_Start;

// Leg Animation events
Event EV_SetAnim
(
    "anim",
    EV_ZERO,
    "s",
    "animName",
    "Exec anim commands on server or client.",
    EV_NORMAL
);
Event EV_SetSyncTime
(
    "setsynctime",
    EV_ZERO,
    "f",
    "synctime",
    "Set sync time for entity.",
    EV_NORMAL
);
Event EV_Animate_IsLoopingAnim
(
    "isloopinganim",
    EV_DEFAULT,
    "s",
    "anim_name",
    "returns 1 if the anim is a looping anim, or 0 otherwise",
    EV_RETURN
);
Event EV_Animate_SetYawfromBone
(
    "setyawfrombone",
    EV_DEFAULT,
    "s",
    "bone_name",
    "Set the yaw of the model based on the current animation time",
    EV_NORMAL
);
Event EV_Animate_PlayerSpawn
(
    "playerspawn",
    EV_DEFAULT,
    "sFVFF",
    "model_name range vector_offset inFOV speed",
    "spawn something near the player, either within the player's view or behind him\n"
    "model - name of model to spawn\n"
    "range - how close does this need to be to the player to actually get spawned, default 480 (30 foot radius).\n"
    "vector_offset - oriented vector offset of where to spawn the item, default (0 0 0)\n"
    "inFOV - \n"
    "\t\t1 - Only spawn when this position is within the FOV of the player\n"
    "\t   -1 - Only spawn when this position is outside the FOV of the player\n"
    "       0 - (default) don't care, always spawn\n"
    "speed - how fast the effect should travel, in other words, how long before the effect gets spawned.\n"
    "\t     delay is calculated based on the distance between object and player divided by the speed\n"
    "\t   0 - no delay\n"
    "\t 960 - (default) 60 feet per second.  If the object is 60 feet from the player, the player effect will spawn "
    "one second later.",
    EV_NORMAL
);
Event EV_Animate_PlayerSpawn_Utility
(
    "testmojo",
    EV_DEFAULT,
    "sv",
    "model_name vector_offset",
    "INTERNAL EVENT",
    EV_NORMAL
);
Event EV_Animate_PauseAnim
(
    "pauseanims",
    EV_DEFAULT,
    "i",
    "pause",
    "Pause (or unpause) animations",
    EV_NORMAL
);
Event EV_Animate_Client_Sound
(
    "_client_sound",
    EV_DEFAULT,
    NULL,
    NULL,
    NULL,
    EV_NORMAL
);

CLASS_DECLARATION(Entity, Animate, "animate") {
    {&EV_SetControllerAngles,         &Animate::SetControllerAngles    },
    {&EV_SetAnim,                     &Animate::ForwardExec            },
    {&EV_SetSyncTime,                 &Animate::EventSetSyncTime       },
    {&EV_Animate_IsLoopingAnim,       &Animate::EventIsLoopingAnim     },
    {&EV_Animate_SetYawfromBone,      &Animate::EventSetYawFromBone    },
    {&EV_Animate_PlayerSpawn,         &Animate::EventPlayerSpawn       },
    {&EV_Animate_PlayerSpawn_Utility, &Animate::EventPlayerSpawnUtility},
    {&EV_Animate_PauseAnim,           &Animate::EventPauseAnim         },
    {&EV_Animate_Client_Sound,        &Animate::ClientSound            },
    {NULL,                            NULL                             }
};

static size_t GetLongestClientCmdLength(const tiki_cmd_t& cmds)
{
    size_t ii;
    size_t longest = 0;

    for (ii = 0; ii < cmds.num_cmds; ii++) {
        const tiki_singlecmd_t& single_cmd = cmds.cmds[ii];
        size_t                  length     = strlen(single_cmd.args[0]);
        if (length > longest) {
            longest = length;
        }
    }

    return longest;
}

static size_t GetLongestClientCmdAllFramesLength(dtiki_t *tiki, int animnum, int numframes)
{
    tiki_cmd_t cmds;
    size_t     longest = 0;
    int        i;

    for (i = 0; i < numframes; i++) {
        // we want normal frame commands to occur right on the frame
        if (gi.Frame_Commands_Client(tiki, animnum, i, &cmds)) {
            size_t length = GetLongestClientCmdLength(cmds);
            if (length > longest) {
                longest = length;
            }
        }
    }

    return longest;
}

Animate::Animate()
{
    entflags |= ECF_ANIMATE;

    syncTime = 0.0f;
    syncRate = 1.0f;

    pauseSyncTime = 0.0f;
    is_paused     = false;

    frame_delta   = vec_zero;
    angular_delta = 0;

    if (!LoadingSavegame) {
        edict->s.actionWeight = 1.0f;
        for (int i = 0; i < MAX_FRAMEINFOS; i++) {
            edict->s.frameInfo[i].index  = 0;
            edict->s.frameInfo[i].time   = 0.0f;
            edict->s.frameInfo[i].weight = 0.0f;

            animtimes[i]  = 0;
            frametimes[i] = 0;
            doneEvents[i] = NULL;
            animFlags[i]  = ANIM_LOOP | ANIM_NODELTA | ANIM_NOEXIT | ANIM_PAUSED;
            ClearAnimSlot(i);
        }

        flags |= FL_ANIMATE;
    }
}

Animate::~Animate()
{
    entflags &= ~ECF_ANIMATE;
}

void Animate::ForwardExec(Event *ev)
{
    if (!edict->tiki) {
        ScriptError(
            "trying to play animation on( entnum: %d, targetname : '%s', classname : '%s' ) which does not have a "
            "model",
            entnum,
            targetname.c_str(),
            getClassname()
        );
    }

    NewAnim(ev->GetString(1), 0);
    RestartAnimSlot(0);
}

void Animate::SetSyncTime(float s)
{
    if (s < 0.0f || s > 1.0f) {
        Com_Printf("\nERROR SetSyncTime:  synctime must be 0 to 1 - attempt to set to %f\n", s);
        return;
    }

    syncTime = s;

    for (int i = 0; i < MAX_FRAMEINFOS; i++) {
        if (!(animFlags[i] & ANIM_SYNC)) {
            continue;
        }

        SlotChanged(i);
    }
}

void Animate::EventSetSyncTime(Event *ev)
{
    SetSyncTime(ev->GetFloat(1));
}

void Animate::NewAnim(int animnum, int slot, float weight)
{
    qboolean        newanim = edict->s.frameInfo[slot].index != animnum;
    tiki_cmd_t      cmds;
    float           time;
    int             numframes;
    AnimationEvent *ev;
    int             i;

    if (newanim) {
        DoExitCommands(slot);
    }

    if (doneEvents[slot]) {
        delete doneEvents[slot];
        doneEvents[slot] = NULL;
    }

    CancelFlaggedEvents(1 << slot);

    edict->s.frameInfo[slot].index = animnum;

    animFlags[slot] = ANIM_LOOP | ANIM_NODELTA | ANIM_NOEXIT;

    if (!(gi.Anim_FlagsSkel(edict->tiki, animnum) & ANIM_LOOP)) {
        SetOnceType(slot);
    }

    SetWeight(slot, weight);
    animtimes[slot]  = gi.Anim_Time(edict->tiki, animnum);
    frametimes[slot] = gi.Anim_Frametime(edict->tiki, animnum);

    if (edict->s.eType == ET_GENERAL) {
        edict->s.eType = ET_MODELANIM;
    }

    qboolean hascommands =
        gi.Anim_HasCommands(edict->tiki, animnum) || gi.Anim_HasCommands_Client(edict->tiki, animnum);

    // enter this animation
    if (newanim) {
        if (!hascommands) {
            return;
        }

        if (gi.Frame_Commands(edict->tiki, animnum, TIKI_FRAME_ENTRY, &cmds)) {
            int ii, j;

            for (ii = 0; ii < cmds.num_cmds; ii++) {
                const tiki_singlecmd_t& single_cmd = cmds.cmds[ii];

                AnimationEvent ev(single_cmd.args[0], single_cmd.num_args);

                ev.SetAnimationNumber(animnum);
                ev.SetAnimationFrame(0);

                for (j = 1; j < single_cmd.num_args; j++) {
                    ev.AddToken(single_cmd.args[j]);
                }

                ProcessEvent(ev);
            }
        }

        if (gi.Frame_Commands_Client(edict->tiki, animnum, TIKI_FRAME_ENTRY, &cmds)) {
            size_t longest = GetLongestClientCmdLength(cmds);
            int    ii, j;

            if (longest) {
                char *cmdName = (char *)alloca(longest + 8 + 1);

                for (ii = 0; ii < cmds.num_cmds; ii++) {
                    const tiki_singlecmd_t& single_cmd = cmds.cmds[ii];
                    int                     eventNum;

                    strcpy(cmdName, "_client_");
                    strcpy(cmdName + 8, single_cmd.args[0]);

                    eventNum = Event::FindEventNum(cmdName);

                    AnimationEvent ev(eventNum, single_cmd.num_args);
                    if (!ev.eventnum) {
                        continue;
                    }

                    ev.SetAnimationNumber(animnum);
                    ev.SetAnimationFrame(0);

                    for (j = 1; j < single_cmd.num_args; j++) {
                        ev.AddToken(single_cmd.args[j]);
                    }

                    ProcessEvent(ev);
                }
            }
        }
    }

    if (!hascommands) {
        return;
    }

    if (!edict->tiki) {
        return;
    }

    time      = 0.0f;
    numframes = gi.Anim_NumFrames(edict->tiki, animnum);

    for (i = 0; i < numframes; i++, time += frametimes[slot]) {
        // we want normal frame commands to occur right on the frame
        if (gi.Frame_Commands(edict->tiki, animnum, i, &cmds)) {
            int ii, j;

            for (ii = 0; ii < cmds.num_cmds; ii++) {
                const tiki_singlecmd_t& single_cmd = cmds.cmds[ii];

                ev = new AnimationEvent(single_cmd.args[0], single_cmd.num_args);

                ev->SetAnimationNumber(animnum);
                ev->SetAnimationFrame(i);

                for (j = 1; j < single_cmd.num_args; j++) {
                    ev->AddToken(single_cmd.args[j]);
                }

                PostEvent(ev, time, 1 << slot);
            }
        }
    }

    time = 0.0f;

    const size_t longestAllFrames = GetLongestClientCmdAllFramesLength(edict->tiki, animnum, numframes);
    if (longestAllFrames) {
        char *cmdName = (char *)alloca(longestAllFrames + 8 + 1);

        for (i = 0; i < numframes; i++, time += frametimes[slot]) {
            // we want normal frame commands to occur right on the frame
            if (gi.Frame_Commands_Client(edict->tiki, animnum, i, &cmds)) {
                int ii, j;

                for (ii = 0; ii < cmds.num_cmds; ii++) {
                    const tiki_singlecmd_t& single_cmd = cmds.cmds[ii];
                    int                     eventNum;

                    strcpy(cmdName, "_client_");
                    strcpy(cmdName + 8, single_cmd.args[0]);

                    eventNum = Event::FindEventNum(cmdName);

                    AnimationEvent ev(eventNum, single_cmd.num_args);
                    if (!ev.eventnum) {
                        continue;
                    }

                    ev.SetAnimationNumber(animnum);
                    ev.SetAnimationFrame(i);

                    for (j = 1; j < single_cmd.num_args; j++) {
                        ev.AddToken(single_cmd.args[j]);
                    }

                    PostEvent(ev, time, 1 << slot);
                }
            }
        }
    }
}

void Animate::NewAnim(int animnum, Event& newevent, int slot, float weight)
{
    if (animnum != -1) {
        NewAnim(animnum, slot);
        SetAnimDoneEvent(newevent, slot);
    } else {
        PostEvent(newevent, level.frametime);
    }
}

void Animate::NewAnim(int animnum, Event *newevent, int slot, float weight)
{
    if (animnum == -1 && newevent) {
        PostEvent(newevent, level.frametime);
    }

    NewAnim(animnum, slot);
    SetAnimDoneEvent(newevent, slot);
}

void Animate::NewAnim(const char *animname, int slot, float weight)
{
    int animnum = gi.Anim_NumForName(edict->tiki, animname);

    if (animnum != -1) {
        NewAnim(animnum, slot);
    }
}

void Animate::NewAnim(const char *animname, Event *endevent, int slot, float weight)
{
    int animnum = gi.Anim_NumForName(edict->tiki, animname);

    if (animnum != -1) {
        NewAnim(animnum, slot, weight);
        SetAnimDoneEvent(endevent, slot);
    } else if (endevent) {
        PostEvent(endevent, level.frametime);
    }
}

void Animate::NewAnim(const char *animname, Event& endevent, int slot, float weight)
{
    int animnum = gi.Anim_NumForName(edict->tiki, animname);

    if (animnum != -1) {
        NewAnim(animnum, slot, weight);
        SetAnimDoneEvent(endevent, slot);
    } else {
        PostEvent(endevent, level.frametime);
    }
}

void Animate::SetAnimDoneEvent(Event *event, int slot)
{
    if (doneEvents[slot]) {
        delete doneEvents[slot];
    }

    doneEvents[slot] = event;
}

void Animate::SetAnimDoneEvent(Event& event, int slot)
{
    SetAnimDoneEvent(new Event(event), slot);
}

void Animate::SetFrame(void)
{
    edict->s.frameInfo[0].time = 0;
    animFlags[0]               = (animFlags[0] | ANIM_NODELTA) & ~ANIM_FINISHED;
}

qboolean Animate::HasAnim(const char *animname)
{
    int num;

    num = gi.Anim_Random(edict->tiki, animname);
    return (num >= 0);
}

void Animate::StopAnimating(int slot)
{
    DoExitCommands(slot);

    if (doneEvents[slot]) {
        delete doneEvents[slot];
        doneEvents[slot] = NULL;
    }

    CancelFlaggedEvents(1 << slot);

    if (edict->s.frameInfo[slot].index || gi.TIKI_NumAnims(edict->tiki) <= 1) {
        edict->s.frameInfo[slot].index = 0;
    } else {
        edict->s.frameInfo[slot].index = 1;
    }

    edict->s.frameInfo[slot].weight = 0.0f;

    animFlags[slot] = ANIM_LOOP | ANIM_NODELTA | ANIM_NOEXIT | ANIM_PAUSED;
    animtimes[slot] = 0.0f;

    SlotChanged(slot);
}

void Animate::DoExitCommands(int slot)
{
    tiki_cmd_t      cmds;
    AnimationEvent *ev;

    if (animFlags[slot] & ANIM_NOEXIT) {
        return;
    }

    // exit the previous animation
    if (gi.Frame_Commands(edict->tiki, edict->s.frameInfo[slot].index, TIKI_FRAME_EXIT, &cmds)) {
        int ii, j;

        for (ii = 0; ii < cmds.num_cmds; ii++) {
            const tiki_singlecmd_t& single_cmd = cmds.cmds[ii];

            ev = new AnimationEvent(single_cmd.args[0], single_cmd.num_args);

            ev->SetAnimationNumber(edict->s.frameInfo[slot].index);
            ev->SetAnimationFrame(0);

            for (j = 1; j < single_cmd.num_args; j++) {
                ev->AddToken(single_cmd.args[j]);
            }

            PostEvent(ev, 0);
        }
    }

    if (gi.Frame_Commands_Client(edict->tiki, edict->s.frameInfo[slot].index, TIKI_FRAME_EXIT, &cmds)) {
        size_t longest = GetLongestClientCmdLength(cmds);
        if (longest) {
            char *cmdName = (char *)alloca(longest + 8 + 1);
            int   ii, j;

            for (ii = 0; ii < cmds.num_cmds; ii++) {
                const tiki_singlecmd_t& single_cmd = cmds.cmds[ii];
                int                     eventNum;

                strcpy(cmdName, "_client_");
                strcpy(cmdName + 8, single_cmd.args[0]);

                eventNum = Event::FindEventNum(cmdName);

                AnimationEvent ev(eventNum, single_cmd.num_args);
                if (!ev.eventnum) {
                    continue;
                }

                ev.SetAnimationNumber(edict->s.frameInfo[slot].index);
                ev.SetAnimationFrame(0);

                for (j = 1; j < single_cmd.num_args; j++) {
                    ev.AddToken(single_cmd.args[j]);
                }

                PostEvent(ev, 0);
            }
        }
    }

    animFlags[slot] |= ANIM_NOEXIT;
}

int Animate::CurrentAnim(int slot) const
{
    return edict->s.frameInfo[slot].index;
}

float Animate::AnimTime(int slot)
{
    return animtimes[slot];
}

int Animate::NumAnims(void)
{
    return gi.TIKI_NumAnims(edict->tiki);
}

const char *Animate::AnimName(int slot)
{
    return gi.Anim_NameForNum(edict->tiki, edict->s.frameInfo[slot].index);
}

void Animate::AnimFinished(int slot)
{
    animFlags[slot] &= ~ANIM_FINISHED;

    if (doneEvents[slot]) {
        Event *ev        = doneEvents[slot];
        doneEvents[slot] = NULL;

        ProcessEvent(ev);
    }
}

void Animate::PreAnimate(void)
{
    int i;

    for (i = 0; i < MAX_FRAMEINFOS; i++) {
        if (animFlags[i] & ANIM_FINISHED) {
            AnimFinished(i);
        }
    }
}

void Animate::PostAnimate(void)
{
    float  startTime;
    float  deltaSyncTime;
    float  total_weight;
    float  total_angular_delta;
    Vector vFrameDelta;
    bool   hasAction = false;

    if (!edict->tiki) {
        return;
    }

    deltaSyncTime = syncTime;

    if (!pauseSyncTime) {
        syncTime = 1.0f / syncRate * level.frametime + deltaSyncTime;
    }

    total_weight        = 0;
    total_angular_delta = 0;

    for (int i = 0; i < MAX_FRAMEINFOS; i++) {
        if (edict->s.frameInfo[i].weight > 0.0f && !(animFlags[i] & ANIM_NOACTION)) {
            hasAction = true;
        }

        if (animFlags[i] & ANIM_PAUSED) {
            continue;
        }

        if (animFlags[i] & ANIM_SYNC) {
            startTime                  = deltaSyncTime * animtimes[i];
            edict->s.frameInfo[i].time = animtimes[i] * syncTime;
        } else {
            startTime = edict->s.frameInfo[i].time;
            edict->s.frameInfo[i].time += level.frametime;
        }

        if (animtimes[i] == 0.0f) {
            animFlags[i] &= ~ANIM_NODELTA;
            animFlags[i] |= ANIM_FINISHED;
            RestartAnimSlot(i);
        } else {
            if (!(animFlags[i] & ANIM_NODELTA) && gi.Anim_HasDelta(edict->tiki, edict->s.frameInfo[i].index)) {
                vec3_t vDelta;
                float  angleDelta;

                //
                // Get the animation's delta position from start time
                //
                gi.Anim_DeltaOverTime(
                    edict->tiki, edict->s.frameInfo[i].index, startTime, edict->s.frameInfo[i].time, vDelta
                );
                VectorScale(vDelta, edict->s.frameInfo[i].weight, vDelta);
                vFrameDelta += vDelta;
                total_weight += edict->s.frameInfo[i].weight;

                //
                // Get the animation's delta angle from start time
                //
                gi.Anim_AngularDeltaOverTime(
                    edict->tiki, edict->s.frameInfo[i].index, startTime, edict->s.frameInfo[i].time, &angleDelta
                );
                angleDelta *= edict->s.frameInfo[i].weight;
                total_angular_delta += angleDelta;
            }

            animFlags[i] &= ~ANIM_NODELTA;

            if (animFlags[i] & ANIM_SYNC) {
                if (edict->s.frameInfo[i].time < animtimes[i]) {
                    continue;
                }
            } else {
                if (edict->s.frameInfo[i].time < (animtimes[i] - 0.01f)) {
                    continue;
                }
            }

            if (IsRepeatType(i)) {
                animFlags[i] |= ANIM_FINISHED;

                do {
                    edict->s.frameInfo[i].time -= animtimes[i];
                } while (edict->s.frameInfo[i].time >= animtimes[i]);

                if (edict->s.frameInfo[i].time < 0) {
                    edict->s.frameInfo[i].time = 0;
                }

            } else {
                if (startTime != animtimes[i]) {
                    animFlags[i] |= ANIM_FINISHED;
                }

                edict->s.frameInfo[i].time = animtimes[i];
            }
        }
    }

    if (total_weight) {
        vFrameDelta *= 1.f / total_weight;
        total_angular_delta *= 1.f / total_weight;
    }

    MatrixTransformVector(vFrameDelta, orientation, frame_delta);
    angular_delta = total_angular_delta;

    while (syncTime >= 1.0f) {
        syncTime -= 1.0f;
    }

    total_weight = level.frametime * 4.0f;

    if (hasAction) {
        edict->s.actionWeight += total_weight;

        if (edict->s.actionWeight > 1.0f) {
            edict->s.actionWeight = 1.0f;
        }
    } else {
        edict->s.actionWeight -= total_weight;

        if (edict->s.actionWeight < 0.0f) {
            edict->s.actionWeight = 0.0f;
        }
    }
}

void Animate::SetTime(int slot, float time)
{
    if (time < 0.0) {
        Com_Printf("ERROR:  SetTime %f lesser than anim length %f\n", time, animtimes[slot]);
        return;
    }

    if (time > animtimes[slot]) {
        Com_Printf("ERROR:  SetTime %f greater than anim length %f\n", time, animtimes[slot]);
        return;
    }

    edict->s.frameInfo[slot].time = time;
}

void Animate::SetNormalTime(int slot, float normal)
{
    if (normal < 0.0f || normal > 1.0f) {
        Com_Printf("ERROR:  Animate::SetNormalTime: Normal must be between 0 and 1\n");
    } else {
        edict->s.frameInfo[slot].time = animtimes[slot] * normal;
    }
}

float Animate::GetTime(int slot)
{
    return edict->s.frameInfo[slot].time;
}

float Animate::GetNormalTime(int slot)
{
    return edict->s.frameInfo[slot].time / animtimes[slot];
}

void Animate::Pause(int slot, int pause)
{
    if (pause) {
        animFlags[slot] |= ANIM_PAUSED;
    } else {
        if (animFlags[slot] & ANIM_PAUSED) {
            if (animFlags[slot] & ANIM_SYNC) {
                SlotChanged(animFlags[slot]);
            }

            animFlags[slot] &= ~ANIM_PAUSED;
        }
    }
}

void Animate::UseSyncTime(int slot, int sync)
{
    if (sync) {
        if (!(animFlags[slot] & ANIM_SYNC)) {
            animFlags[slot] |= ANIM_SYNC;
            SlotChanged(slot);
        }
    } else {
        if (animFlags[slot] & ANIM_SYNC) {
            animFlags[slot] &= ~ANIM_SYNC;
            SlotChanged(slot);
        }
    }
}

void Animate::SetSyncRate(float rate)
{
    if (rate < 0.001f) {
        Com_Printf("ERROR SetSyncRate:  canot set syncrate below 0.001.\n");
        syncRate = 0.001f;
    } else {
        syncRate = rate;
    }
}

float Animate::GetCrossTime(int slot)
{
    return gi.Anim_CrossTime(edict->tiki, edict->s.frameInfo[slot].index);
}

void Animate::SetRepeatType(int slot)
{
    animFlags[slot] |= ANIM_LOOP;
}

void Animate::SetOnceType(int slot)
{
    animFlags[slot] &= ~ANIM_LOOP;
}

void Animate::EventIsLoopingAnim(Event *ev)
{
    str anim_name = ev->GetString(1);
    int animnum;

    if (!edict->tiki) {
        ScriptError("^~^~^ no tiki set");
    }

    animnum = gi.Anim_NumForName(edict->tiki, anim_name.c_str());
    if (animnum < 0) {
        ScriptError("anim '%s' not found, so can't tell if it is looping", anim_name.c_str());
    }

    if (gi.Anim_FlagsSkel(edict->tiki, animnum) & ANIM_LOOP) {
        ev->AddInteger(1);
    } else {
        ev->AddInteger(0);
    }
}

void Animate::EventSetYawFromBone(Event *ev)
{
    str           bonename;
    int           tagnum;
    orientation_t ori;
    vec3_t        vAngles;

    bonename = ev->GetString(1);
    tagnum   = gi.Tag_NumForName(edict->tiki, bonename.c_str());
    if (tagnum < 0) {
        ScriptError("Could not find tag '%s' in '%s'", bonename.c_str(), edict->tiki->name);
    }

    GetTagPositionAndOrientation(tagnum, &ori);
    MatrixToEulerAngles(ori.axis, vAngles);
    angles.y += vAngles[1];
    setAngles(angles);

    NoLerpThisFrame();
}

void Animate::EventPlayerSpawn(Event *ev)
{
    Player *player;
    float   range;
    Vector  vector_offset;
    float   inFOV;
    float   speed;
    Vector  delta;
    float   dist;
    float   time;
    float   dot;
    Event  *event;

    player = static_cast<Player *>(G_FindTarget(this, "player"));
    if (!player) {
        ScriptError("Could not find player!");
    }

    if (ev->NumArgs() > 1) {
        range = ev->GetFloat(2);
    } else {
        range = 480;
    }

    if (ev->NumArgs() > 2) {
        vector_offset = ev->GetVector(3);
    } else {
        vector_offset = vec_zero;
    }

    if (ev->NumArgs() > 3) {
        inFOV = ev->GetFloat(4);
    } else {
        inFOV = 0;
    }

    if (ev->NumArgs() > 4) {
        speed = ev->GetFloat(5);
    } else {
        speed = 960;
    }

    delta = origin - player->origin;
    dist  = delta.length();
    if (dist > range) {
        // above the range, won't spawn
        return;
    }

    time = dist / speed;
    dot  = DotProduct(delta, player->orientation[0]);

    if (inFOV < 0) {
        if (dot > cos(DEG2RAD(45))) {
            return;
        }
    } else if (inFOV > 0) {
        if (dot < cos(DEG2RAD(45))) {
            return;
        }
    }

    event = new Event(EV_Animate_PlayerSpawn_Utility);
    event->AddString(ev->GetString(1));
    event->AddVector(vector_offset);
    player->PostEvent(event, time);
}

void Animate::EventPlayerSpawnUtility(Event *ev)
{
    Player   *player;
    str       modelname;
    Vector    vector_offset;
    Vector    transformed;
    SpawnArgs args;
    ClassDef *cls;
    Entity   *newEnt;
    Event    *event;

    player = static_cast<Player *>(G_FindTarget(this, "player"));
    if (!player) {
        ScriptError("Could not find player!");
    }

    modelname     = ev->GetString(1);
    vector_offset = ev->GetVector(2);

    MatrixTransformVector(vector_offset, player->orientation, transformed);
    transformed += player->origin;

    args.setArg("classname", modelname.c_str());
    args.setArg("model", modelname.c_str());
    cls = args.getClassDef();
    if (!cls) {
        cls = Entity::classinfostatic();
    }

    // Spawn the new entity
    newEnt = static_cast<Entity *>(cls->newInstance());

    event = new Event(EV_Model);
    event->AddString(modelname);
    newEnt->PostEvent(event, EV_SETUP_ROPEPIECE);

    event = new Event(EV_SetOrigin);
    event->AddVector(transformed);
    newEnt->PostEvent(event, EV_SETUP_ROPEBASE);

    event = new Event(EV_SetAngles);
    event->AddVector(transformed.toAngles());
    newEnt->PostEvent(event, EV_SETUP_ROPEBASE);

    newEnt->ProcessPendingEvents();
    newEnt->ProcessEvent(EV_Entity_Start);
}

float Animate::GetYawOffset()
{
    SkelMat4 *transform;
    int       boneNum;

    boneNum = gi.Tag_NumForName(edict->tiki, "Bip01");
    if (boneNum == -1) {
        return 0.0;
    }

    transform = G_TIKI_Transform(edict, boneNum);
    if (transform->val[0][0] != 0) {
        return RAD2DEG(atan2(transform->val[0][1], transform->val[0][0]));
    } else {
        return 0;
    }
}

void Animate::DumpAnimInfo(void)
{
    MPrintf("----------------------------------------\n");

    for (int i = 0; i < MAX_FRAMEINFOS; i++) {
        if (edict->s.frameInfo[i].weight <= 0.0f) {
            continue;
        }

        str animname = gi.Anim_NameForNum(edict->tiki, CurrentAnim(i));

        MPrintf(
            "slot: %d  anim: %s weight: %f  time: %f  len: %f\n",
            i,
            animname.c_str(),
            edict->s.frameInfo[i].weight,
            edict->s.frameInfo[i].time,
            animtimes[i]
        );
    }

    MPrintf("actionWeight: %f\n", edict->s.actionWeight);
}

void Animate::ClientSound(Event *ev)
{
    PlayNonPvsSound(ev->GetString(1));
}

void Animate::EventPauseAnim(Event *ev)
{
    is_paused = ev->GetInteger(1) ? true : false;
    for (int i = 0; i < MAX_FRAMEINFOS; i++) {
        Pause(i, is_paused);
    }
}
