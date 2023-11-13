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

// trigger.h: Environment based triggers.
//

#pragma once

#include "glb_local.h"
#include "animate.h"
#include "gamescript.h"

class ScriptMaster;

extern Event EV_Trigger_ActivateTargets;
extern Event EV_Trigger_SetWait;
extern Event EV_Trigger_SetDelay;
extern Event EV_Trigger_SetCount;
extern Event EV_Trigger_SetMessage;
extern Event EV_Trigger_SetNoise;
extern Event EV_Trigger_SetThread;
extern Event EV_Trigger_Effect;
extern Event EV_Trigger_StartThread;
extern Event EV_Trigger_SetTriggerable;
extern Event EV_Trigger_SetNotTriggerable;

#define TRIGGER_PLAYERS     4
#define TRIGGER_MONSTERS    8
#define TRIGGER_PROJECTILES 16

class Trigger : public Animate
{
protected:
    float             wait;
    float             delay;
    float             trigger_time;
    qboolean          triggerActivated;
    int               count;
    const_str         noise;
    const_str         message;
    ScriptThreadLabel m_Thread;
    EntityPtr         activator;
    int               respondto;
    qboolean          useTriggerDir;
    float             triggerCone;
    Vector            triggerDir;
    float             triggerDirYaw;
    qboolean          triggerable;
    qboolean          removable;     // if count is 0, should this be removed?
    qboolean          edgeTriggered; // if true, trigger only triggers when entering trigger, not when standing in it
    int               multiFaceted;  // if 0, it isn't. if 1 it is N/S oriented, if 2 it is E/W oriented

public:
    CLASS_PROTOTYPE(Trigger);

    Trigger();
    virtual ~Trigger();

    // override this to allow objects other than players, projectiles, and monsters to activate the trigger
    virtual qboolean respondTo(Entity *other);

    // override this to redirect messages to an entity other than the one who triggered it
    virtual Entity *getActivator(Entity *other);

    void SetModelEvent(Event *ev);
    void Touch(Event *ev);
    void EventSetWait(Event *ev);
    void EventSetDelay(Event *ev);
    void EventSetCount(Event *ev);
    void EventSetThread(Event *ev);
    void SetTriggerDir(Event *ev);
    void SetTriggerable(Event *ev);
    void SetNotTriggerable(Event *ev);
    void SetMultiFaceted(Event *ev);
    void SetEdgeTriggered(Event *ev);
    void SetTriggerCone(Event *ev);

    void EventSetMessage(Event *ev);
    void SetMessage(const char *message);
    str& Message(void);

    void EventSetNoise(Event *ev);
    void SetNoise(const char *text);
    str& Noise(void);

    void     SetTriggerDir(float angle);
    Vector   GetTriggerDir(void);
    qboolean UsingTriggerDir(void);

    void SetMultiFaceted(int newFacet);
    void SetEdgeTriggered(qboolean newEdge);

    int      GetMultiFaceted(void);
    qboolean GetEdgeTriggered(void);

    void StartThread(Event *ev);
    void TriggerStuff(Event *ev);
    void ActivateTargets(Event *ev);
    void DamageEvent(Event *ev) override;
    void EventGetActivator(Event *ev); // Added in 2.30
    void Archive(Archiver& arc) override;
};

class TriggerVehicle : public Trigger
{
public:
    CLASS_PROTOTYPE(TriggerVehicle);

    qboolean respondTo(Entity *other) override;
};

class TriggerAllEntry
{
public:
    EntityPtr ent;
    float     time;

public:
    static void Archive(Archiver& arc, TriggerAllEntry *obj);
};

class TriggerAll : public Trigger
{
private:
    Container<TriggerAllEntry> entries;

public:
    CLASS_PROTOTYPE(TriggerAll);

    void Archive(Archiver& arc) override;

    void TriggerStuff(Event *ev);
    bool IsTriggerable(Entity *other);
    void SetTriggerTime(Entity *other, float time);
};

inline void TriggerAll::Archive(Archiver& arc)
{
    Trigger::Archive(arc);

    entries.Archive(arc, &TriggerAllEntry::Archive);
}

class TouchField : public Trigger
{
private:
    Event    *ontouch;
    EntityPtr owner;

public:
    CLASS_PROTOTYPE(TouchField);

    TouchField();
    virtual void Setup(Entity *ownerentity, Event& ontouch, Vector min, Vector max, int respondto);
    void         SendEvent(Event *ev);
    void         Archive(Archiver        &arc) override;
};

inline void TouchField::Archive(Archiver& arc)
{
    Trigger::Archive(arc);

    arc.ArchiveEventPointer(&ontouch);
    arc.ArchiveSafePointer(&owner);
}

typedef SafePtr<TouchField> TouchFieldPtr;

class TriggerOnce : public Trigger
{
public:
    CLASS_PROTOTYPE(TriggerOnce);
    TriggerOnce();
};

class TriggerSave : public TriggerOnce
{
private:
    str m_sSaveName;

public:
    CLASS_PROTOTYPE(TriggerSave);

    TriggerSave();

    void SaveGame(Event *ev);
    void EventSaveName(Event *ev);
    void Archive(Archiver& arc) override;
};

class TriggerRelay : public Trigger
{
public:
    CLASS_PROTOTYPE(TriggerRelay);

    TriggerRelay();
};

class TriggerSecret : public TriggerOnce
{
public:
    CLASS_PROTOTYPE(TriggerSecret);

    TriggerSecret();
    void FoundSecret(Event *ev);
    void Activate(Event *ev);
};

class TriggerPush : public Trigger
{
protected:
    float speed;

public:
    CLASS_PROTOTYPE(TriggerPush);

    TriggerPush();
    void Push(Event *ev);
    void SetPushDir(Event *ev);
    void SetPushSpeed(Event *ev);
    void Archive(Archiver& arc) override;
};

inline void TriggerPush::Archive(Archiver& arc)
{
    Trigger::Archive(arc);

    arc.ArchiveFloat(&speed);
}

class TriggerPushAny : public Trigger
{
protected:
    float speed;

public:
    CLASS_PROTOTYPE(TriggerPushAny);

    TriggerPushAny();
    void Push(Event *ev);
    void SetSpeed(Event *ev);
    void Archive(Archiver& arc) override;
};

inline void TriggerPushAny::Archive(Archiver& arc)
{
    Trigger::Archive(arc);

    arc.ArchiveFloat(&speed);
}

#define AMBIENT_ON  (1 << 0)
#define AMBIENT_OFF (1 << 1)
#define TOGGLESOUND (1 << 5)

class TriggerPlaySound : public Trigger
{
protected:
    friend class SoundManager;

    int      state;
    float    min_dist;
    float    volume;
    int      channel;
    qboolean ambient;

public:
    CLASS_PROTOTYPE(TriggerPlaySound);

    TriggerPlaySound();
    void ToggleSound(Event *ev);
    void SetVolume(Event *ev);
    void SetMinDist(Event *ev);
    void SetChannel(Event *ev);

    void StartSound(void);
    void SetVolume(float vol);
    void SetMinDist(float dist);

    void Archive(Archiver& arc) override;
};

inline void TriggerPlaySound::Archive(Archiver& arc)
{
    Trigger::Archive(arc);

    arc.ArchiveInteger(&state);
    arc.ArchiveFloat(&min_dist);
    arc.ArchiveFloat(&volume);
    arc.ArchiveInteger(&channel);
    arc.ArchiveBoolean(&ambient);
    if (arc.Loading()) {
        //
        // see if its a toggle sound, if it is, lets start its sound again
        //
        if (spawnflags & TOGGLESOUND) {
            //
            // invert state so that final state will be right
            //
            state = !state;
            PostEvent(EV_Trigger_Effect, EV_POSTSPAWN);
        }
    }
}

class TriggerSpeaker : public TriggerPlaySound
{
public:
    CLASS_PROTOTYPE(TriggerSpeaker);

    TriggerSpeaker();
};

class RandomSpeaker : public TriggerSpeaker
{
protected:
    friend class SoundManager;

    float chance;
    float mindelay;
    float maxdelay;

public:
    CLASS_PROTOTYPE(RandomSpeaker);

    RandomSpeaker();
    void TriggerSound(Event *ev);
    void SetMinDelay(Event *ev);
    void SetMaxDelay(Event *ev);
    void SetChance(Event *ev);

    void SetMinDelay(float value);
    void SetMaxDelay(float value);
    void SetChance(float value);
    void ScheduleSound(void);

    void Archive(Archiver& arc) override;
};

inline void RandomSpeaker::Archive(Archiver& arc)
{
    TriggerSpeaker::Archive(arc);

    arc.ArchiveFloat(&chance);
    arc.ArchiveFloat(&mindelay);
    arc.ArchiveFloat(&maxdelay);
}

class TriggerChangeLevel : public Trigger
{
protected:
    const_str map;
    const_str spawnspot;

public:
    CLASS_PROTOTYPE(TriggerChangeLevel);

    TriggerChangeLevel();
    void        SetMap(Event *ev);
    void        SetSpawnSpot(Event *ev);
    void        ChangeLevel(Event *ev);
    const char *Map(void);
    const char *SpawnSpot(void);
    void        Archive(Archiver       &arc) override;
};

class TriggerExit : public Trigger
{
public:
    CLASS_PROTOTYPE(TriggerExit);

    TriggerExit();
    void DisplayExitSign(Event *ev);
    void TurnExitSignOff(Event *ev);
};

class TriggerUse : public Trigger
{
public:
    CLASS_PROTOTYPE(TriggerUse);

    TriggerUse();
};

class TriggerUseOnce : public TriggerUse
{
public:
    CLASS_PROTOTYPE(TriggerUseOnce);

    TriggerUseOnce();
};

class TriggerHurt : public Trigger
{
protected:
    float damage;

    void Hurt(Event *ev);
    void SetDamage(Event *ev);

public:
    CLASS_PROTOTYPE(TriggerHurt);

    TriggerHurt();
    void Archive(Archiver& arc) override;
};

inline void TriggerHurt::Archive(Archiver& arc)
{
    Trigger::Archive(arc);

    arc.ArchiveFloat(&damage);
}

class TriggerDamageTargets : public Trigger
{
protected:
    float damage;

    void DamageTargets(Event *ev);
    void SetDamage(Event *ev);

public:
    CLASS_PROTOTYPE(TriggerDamageTargets);

    TriggerDamageTargets();
    void PassDamage(Event *ev);
    void Archive(Archiver& arc) override;
};

inline void TriggerDamageTargets::Archive(Archiver& arc)

{
    Trigger::Archive(arc);

    arc.ArchiveFloat(&damage);
}

class TriggerCameraUse : public TriggerUse
{
public:
    CLASS_PROTOTYPE(TriggerCameraUse);

    void TriggerCamera(Event *ev);
};

class TriggerBox : public Trigger
{
public:
    CLASS_PROTOTYPE(TriggerBox);

    void SetMins(Event *ev);
    void SetMaxs(Event *ev);
};

class TriggerMusic : public Trigger
{
private:
    friend class SoundManager;

    qboolean  oneshot;
    const_str current;
    const_str fallback;
    const_str altcurrent;
    const_str altfallback;

public:
    CLASS_PROTOTYPE(TriggerMusic);

    TriggerMusic();
    void SetCurrentMood(Event *ev);
    void SetFallbackMood(Event *ev);
    void SetAltCurrentMood(Event *ev);
    void SetAltFallbackMood(Event *ev);
    void ChangeMood(Event *ev);
    void AltChangeMood(Event *ev);
    void SetOneShot(Event *ev);

    void SetMood(str crnt, str fback);
    void SetAltMood(str crnt, str fback);
    void SetOneShot(qboolean once);

    void Archive(Archiver& arc) override;
};

class TriggerReverb : public Trigger
{
private:
    friend class SoundManager;

    qboolean oneshot;
    int      reverbtype;
    int      altreverbtype;
    float    reverblevel;
    float    altreverblevel;

public:
    CLASS_PROTOTYPE(TriggerReverb);

    TriggerReverb();
    void SetReverbLevel(Event *ev);
    void SetReverbType(Event *ev);
    void SetAltReverbType(Event *ev);
    void SetAltReverbLevel(Event *ev);
    void ChangeReverb(Event *ev);
    void AltChangeReverb(Event *ev);
    void SetOneShot(Event *ev);

    void SetReverb(int type, float level);
    void SetAltReverb(int type, float level);
    void SetOneShot(qboolean once);

    void Archive(Archiver& arc) override;
};

inline void TriggerReverb::Archive(Archiver& arc)

{
    Trigger::Archive(arc);

    arc.ArchiveBoolean(&oneshot);
    arc.ArchiveInteger(&reverbtype);
    arc.ArchiveInteger(&altreverbtype);
    arc.ArchiveFloat(&reverblevel);
    arc.ArchiveFloat(&altreverblevel);
}

class TriggerByPushObject : public TriggerOnce
{
private:
    const_str triggername;

    void setTriggerName(Event *event);

public:
    CLASS_PROTOTYPE(TriggerByPushObject);

    qboolean respondTo(Entity *other) override;
    Entity  *getActivator(Entity *other) override;

    void Archive(Archiver& arc) override;
};

class TriggerGivePowerup : public Trigger
{
private:
    qboolean  oneshot;
    const_str powerup_name;

public:
    CLASS_PROTOTYPE(TriggerGivePowerup);

    TriggerGivePowerup();

    void SetOneShot(Event *ev);
    void SetPowerupName(Event *ev);
    void GivePowerup(Event *ev);
    void Archive(Archiver& arc) override;
};

class TriggerClickItem : public Trigger
{
public:
    CLASS_PROTOTYPE(TriggerClickItem);

    TriggerClickItem();

    void SetClickItemModelEvent(Event *ev);
    void Archive(Archiver& arc) override;
};

inline void TriggerClickItem::Archive(Archiver& arc)
{
    Trigger::Archive(arc);
}

class TriggerNoDamage : public TriggerUse
{
public:
    CLASS_PROTOTYPE(TriggerNoDamage);

public:
    void TakeNoDamage(Event *ev);
};

class TriggerEntity : public Trigger
{
public:
    CLASS_PROTOTYPE(TriggerEntity);
};

class TriggerLandmine : public TriggerEntity
{
public:
    CLASS_PROTOTYPE(TriggerLandmine);

    TriggerLandmine();
    void Archive(Archiver& arc) override;

    void EventIsAbandoned(Event *ev);
    void EventIsImmune(Event *ev);
    void EventSetDamageable(Event *ev);

    void     SetDamageable(qboolean damageable);
    qboolean IsImmune(Entity *other) const;
    void     SetTeam(int team);

private:
    int team;
};

inline void TriggerLandmine::Archive(Archiver& arc)
{
    Trigger::Archive(arc);

    arc.ArchiveInteger(&team);
}
