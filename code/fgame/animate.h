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
// animate.h -- Animate class

#pragma once

#if defined(GAME_DLL)
#    include "entity.h"
#elif defined(CGAME_DLL)
#    include "script/canimate.h"
#endif

#include "archive.h"

extern Event EV_SetAnim;
extern Event EV_SetSyncTime;

#define ANIM_PAUSED                      1
#define ANIM_SYNC                        2
#define ANIM_FINISHED                    4
#define ANIM_NOEXIT                      8
#define ANIM_NODELTA                     16
#define ANIM_LOOP                        32
#define ANIM_NOACTION                    64

#define MINIMUM_DELTA_MOVEMENT           8
#define MINIMUM_DELTA_MOVEMENT_PER_FRAME (MINIMUM_DELTA_MOVEMENT / 20.0f)

#define FLAGGED_ANIMATE_SLOT             20

class Animate;

typedef SafePtr<Animate> AnimatePtr;

class Animate : public Entity
{
protected:
    int animFlags[MAX_FRAMEINFOS];

    float syncTime;
    float syncRate;
    int   pauseSyncTime;
    bool  is_paused;

    Event *doneEvents[MAX_FRAMEINFOS];

    float animtimes[MAX_FRAMEINFOS];
    float frametimes[MAX_FRAMEINFOS];

public:
    Vector frame_delta;
    float  angular_delta;

public:
    CLASS_PROTOTYPE(Animate);

    Animate();
    ~Animate();

    void        NewAnim(int animnum, int slot = 0, float weight = 1.0f);
    void        NewAnim(int animnum, Event *endevent, int slot = 0, float weight = 1.0f);
    void        NewAnim(int animnum, Event       &endevent, int slot = 0, float weight = 1.0f);
    void        NewAnim(const char *animname, int slot = 0, float weight = 1.0f);
    void        NewAnim(const char *animname, Event *endevent, int slot = 0, float weight = 1.0f);
    void        NewAnim(const char *animname, Event       &endevent, int slot = 0, float weight = 1.0f);
    void        SetFrame(void);
    qboolean    HasAnim(const char *animname);
    Event      *AnimDoneEvent(int slot = 0);
    void        SetAnimDoneEvent(Event       &event, int slot = 0);
    void        SetAnimDoneEvent(Event *event, int slot = 0);
    int         NumAnims(void);
    const char *AnimName(int slot = 0);
    float       AnimTime(int slot = 0);

    int   CurrentAnim(int slot = 0) const override;
    float CurrentTime(int slot = 0) const override;

    void         Archive(Archiver        &arc) override;
    virtual void AnimFinished(int slot = 0);

    void  PreAnimate(void) override;
    void  PostAnimate(void) override;
    void  SetTime(int slot = 0, float time = 0.0f);
    void  SetNormalTime(int slot = 0, float normal = 1.0f);
    float GetTime(int slot = 0);
    float GetNormalTime(int slot = 0);
    void  SetWeight(int slot = 0, float weight = 1.0f);
    float GetWeight(int slot = 0);
    void  SetRepeatType(int slot = 0);
    void  SetOnceType(int slot = 0);
    bool  IsRepeatType(int slot);
    void  Pause(int slot = 0, int pause = 1);
    void  StopAnimating(int slot = 0);
    void  UseSyncTime(int slot, int sync);
    void  SetSyncTime(float s);
    float GetSyncTime();
    void  SetSyncRate(float rate);

    float GetSyncRate();
    void  PauseSyncTime(int pause);
    float GetYawOffset();
    float GetCrossTime(int slot);

    void         DoExitCommands(int slot = 0);
    void         ForwardExec(Event *ev);
    void         EventSetSyncTime(Event *ev);
    void         EventIsLoopingAnim(Event *ev);
    void         EventSetYawFromBone(Event *ev);
    void         EventPlayerSpawn(Event *ev);
    void         EventPlayerSpawnUtility(Event *ev);
    void         EventPauseAnim(Event *ev);
    virtual void DumpAnimInfo();
    void         SlotChanged(int slot);

    void ClientSound(Event *ev);

    // FIXME: delete this, fakk2 remnant
    int NumFrames(int slot = 0);
};

inline void Animate::SetWeight(int slot, float weight)
{
    edict->s.frameInfo[slot].weight = weight;
}

inline float Animate::GetWeight(int slot)
{
    return edict->s.frameInfo[slot].weight;
}

inline bool Animate::IsRepeatType(int slot)
{
    return (animFlags[slot] & ANIM_LOOP) != 0;
}

inline float Animate::GetSyncTime()
{
    return syncTime;
}

inline float Animate::GetSyncRate()
{
    return syncRate;
}

inline void Animate::PauseSyncTime(int pause)
{
    pauseSyncTime = pause;
}

inline void Animate::Archive(Archiver& arc)
{
    int i;

    Entity::Archive(arc);

    for (i = 0; i < MAX_FRAMEINFOS; i++) {
        arc.ArchiveInteger(&animFlags[i]);
    }
    arc.ArchiveFloat(&syncTime);
    arc.ArchiveFloat(&syncRate);
    arc.ArchiveInteger(&pauseSyncTime);
    arc.ArchiveBool(&is_paused);
    for (i = 0; i < MAX_FRAMEINFOS; i++) {
        arc.ArchiveEventPointer(&doneEvents[i]);
    }
    for (i = 0; i < MAX_FRAMEINFOS; i++) {
        arc.ArchiveFloat(&animtimes[i]);
    }
    for (i = 0; i < MAX_FRAMEINFOS; i++) {
        arc.ArchiveFloat(&frametimes[i]);
    }
    arc.ArchiveVector(&frame_delta);
    arc.ArchiveFloat(&angular_delta);
}

inline void Animate::SlotChanged(int slot)
{
    animFlags[slot] = (animFlags[slot] | ANIM_NODELTA) & ~ANIM_FINISHED;
}

inline float Animate::CurrentTime(int slot) const
{
    return edict->s.frameInfo[slot].time;
}

inline Event *Animate::AnimDoneEvent(int slot)
{
    return doneEvents[slot];
}

inline int Animate::NumFrames(int slot)
{
    return gi.Anim_NumFrames(edict->tiki, edict->s.frameInfo[slot].index);
}
