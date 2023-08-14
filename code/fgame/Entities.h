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

#pragma once

#include "entity.h"
#include "animate.h"

typedef enum {
	CONTROLLER_ALLIES,
	CONTROLLER_AXIS,
    CONTROLLER_DRAW
} eController;

class ProjectileTarget : public Entity
{
private:
    int m_iTarget;

public:
    CLASS_PROTOTYPE(ProjectileTarget);

    ProjectileTarget();

    void EventSetId(Event *ev);
};

#define MAX_PROJECTILE_GENERATOR_TARGETS 16

class ProjectileGenerator : public Animate
{
private:
    int                           m_iId;
    int                           m_iCycles;
    float                         m_fMinDuration;
    float                         m_fMaxDuration;
    int                           m_iMinNumShots;
    int                           m_iMaxNumShots;
    float                         m_fMinDelay;
    float                         m_fMaxDelay;
    float                         m_fAccuracy;
    str                           m_sLaunchSound;
    bool                          m_bFireOnStartUp;
    qboolean                      m_bIsDonut;
    float                         m_fArcDonut;
    float                         m_fMinDonut;
    float                         m_fMaxDonut;
    float                         m_fCycleTime;
    float                         m_fShotsPerSec;
    float                         m_fCurrentTime;
    int                           m_iTargetIndex;
    float                         m_fLastShotTime;
    int                           m_iAnimSlot;
    bool                          m_bIsTurnedOn;
    int                           m_iCurrentCycle;
    Entity                       *m_pTarget;
    Container<ProjectileTarget *> m_projectileTargets;
    Entity                       *m_pCurrent;
    Vector                        m_vTargetOrg;

public:
    CLASS_PROTOTYPE(ProjectileGenerator);

    ProjectileGenerator();

    virtual void SetupNextCycle();
    virtual void BeginCycle(Event *ev);
    virtual void TickCycle(Event *ev);
    virtual void EndCycle(Event *ev);

    void ShouldStartOn();
    void ShouldHideModel();
    void ShouldPlayFireSound();

    void EventIsTurnedOn(Event *ev);
    void EventGetTargetEntity(Event *ev);
    void EventLaunchSound(Event *ev);
    void SetTarget(Event *ev);
    void OnInitialize(Event *ev);
    void TurnOff(Event *ev);
    void TurnOn(Event *ev);
    void ShouldTargetRandom();
    void ChooseTarget();

    void         GetLocalTargets();
    void         ShouldTargetPlayer();
    void         GetTargetPos(Entity *target);
    void         EventAccuracy(Event *ev);
    void         EventMaxDelay(Event *ev);
    void         EventMinDelay(Event *ev);
    void         EventFireOnStartUp(Event *ev);
    void         EventMaxNumShots(Event *ev);
    void         EventMinNumShots(Event *ev);
    void         EventCycles(Event *ev);
    void         EventMaxDuration(Event *ev);
    void         EventMinDuration(Event *ev);
    void         SetWeaponModel(Event *ev);
    void         EventSetId(Event *ev);
    virtual void Attack(int count);
    void         EventarcDonut(Event *ev);
    void         EventmaxDonut(Event *ev);
    void         EventminDonut(Event *ev);
    void         EventisDonut(Event *ev);

    void Archive(Archiver& arc) override;
};

class ProjectileGenerator_Projectile : public ProjectileGenerator
{
private:
    str   m_sProjectileModel;
    str   m_sPreImpactSound;
    float m_fImpactSoundTime;
    float m_fImpactSoundProbability;

public:
    CLASS_PROTOTYPE(ProjectileGenerator_Projectile);

    ProjectileGenerator_Projectile();

    void SetPreImpactSoundProbability(Event *ev);
    void SetPreImpactSoundTime(Event *ev);
    void SetPreImpactSound(Event *ev);
    void PlayPreImpactSound(Event *ev);
    void SetProjectileModel(Event *ev);

    void Attack(int count) override;
    void Archive(Archiver& arc) override;
};

class ProjectileGenerator_Gun : public ProjectileGenerator
{
private:
    float    m_fBulletRange;
    float    m_fBulletDamage;
    int      m_iBulletCount;
    Vector   m_vBulletSpread;
    int      m_iTracerFrequency;
    int      m_iBulletLarge;
    qboolean m_bFakeBullets;
    int      m_iMeansOfDeath;
    float    m_fBulletThroughWood;
    float    m_fBulletThroughMetal;
    float    m_iBulletKnockback;
    int      m_iAttackCount;
    float    m_fFireDelay;
    float    m_fTracerSpeed;

public:
    CLASS_PROTOTYPE(ProjectileGenerator_Gun);

    ProjectileGenerator_Gun();

    void SetFireDelay(Event *ev);
    void SetMeansOfDeath(Event *ev);
    void SetBulletThroughWood(Event *ev);
    void SetBulletThroughMetal(Event *ev);
    void SetBulletKnockback(Event *ev);
    void SetTracerSpeed(Event *ev);
    void SetBulletLarge(Event *ev);
    void SetFakeBullets(Event *ev);
    void SetBulletCount(Event *ev);
    void SetBulletDamage(Event *ev);
    void SetTracerFrequency(Event *ev);
    void SetBulletSpread(Event *ev);
    void SetBulletRange(Event *ev);

    void Attack(int count) override;
    void TickCycle(Event *ev) override;
    void Archive(Archiver& arc) override;
};

class ProjectileGenerator_Heavy : public ProjectileGenerator
{
private:
    str m_sProjectileModel;

public:
    CLASS_PROTOTYPE(ProjectileGenerator_Heavy);

    ProjectileGenerator_Heavy();

    void SetProjectileModel(Event *ev);

    void Attack(int count) override;
    void Archive(Archiver& arc) override;
};

class ThrobbingBox_Explosive : public Animate
{
private:
    str               m_sUsedModel;
    str               m_sSound;
    bool              m_bUsed;
    float             m_fExplosionDamage;
    float             m_fRadius;
    float             m_fStopwatchDuration;
    str               m_sActivateSound;
    str               m_sTickSound;
    str               m_sDestroyedModel;
    ScriptThreadLabel m_thread;
    ScriptThreadLabel m_useThread;
    str               m_sEffect;
    Vector            m_vOffset;

public:
    CLASS_PROTOTYPE(ThrobbingBox_Explosive);

    ThrobbingBox_Explosive();

    void SetExplosionOffset(Event *ev);
    void SetExplosionEffect(Event *ev);
    void SetTriggered(Event *ev);
    void DoExplosion(Event *ev);
    void SetUseThread(Event *ev);
    void SetThread(Event *ev);
    void SetStopWatchDuration(Event *ev);
    void SetRadius(Event *ev);
    void SetDamage(Event *ev);
    void TickSound(Event *ev);
    void ActivateSound(Event *ev);
    void ExplosionSound(Event *ev);
    void UsedModel(Event *ev);
    void SetDestroyModel(Event *ev);
    void OnBlowUp(Event *ev);
    void OnUse(Event *ev);

    void Archive(Archiver& arc);
};

class ThrobbingBox_ExplodePlayerFlak88 : public ThrobbingBox_Explosive
{
public:
    CLASS_PROTOTYPE(ThrobbingBox_ExplodePlayerFlak88);
};

class ThrobbingBox_ExplodeFlak88 : public ThrobbingBox_Explosive
{
public:
    CLASS_PROTOTYPE(ThrobbingBox_ExplodeFlak88);
};

class ThrobbingBox_ExplodeNebelwerfer : public ThrobbingBox_Explosive
{
public:
    CLASS_PROTOTYPE(ThrobbingBox_ExplodeNebelwerfer);
};

class ThrobbingBox_ExplodePlayerNebelwerfer : public ThrobbingBox_Explosive
{
public:
    CLASS_PROTOTYPE(ThrobbingBox_ExplodePlayerNebelwerfer);
};

class ThrobbingBox_Stickybomb : public ThrobbingBox_Explosive
{
private:
    float m_fStopwatchStartTime;

public:
    CLASS_PROTOTYPE(ThrobbingBox_Stickybomb);

    void OnStickyBombWet(Event *ev);
    void OnStickyBombUse(Event *ev);

    void Archive(Archiver& arc);
};

#define OBJECTIVE_SPAWNFLAG_TURN_ON 1

class Objective : public Entity
{
private:
    str m_sText;
    int m_iObjectiveIndex;

public:
    CLASS_PROTOTYPE(Objective);

    Objective();

    void   Archive(Archiver  &arc);
    void   TurnOn(Event *ev);
    void   TurnOff(Event *ev);
    void   Complete(Event *ev);
    void   SetCurrent(Event *ev);
    Vector GetOrigin() const;
    void   SetObjectiveNbr(Event *ev);
    void   SetText(Event *ev);
    int    GetObjectiveIndex() const;
};

class FencePost : public Entity
{
public:
    CLASS_PROTOTYPE(FencePost);
};

class AISpawnPoint : public SimpleArchivedEntity
{
private:
    str   m_sModel;
    int   m_iHealth;
    str   m_sEnemyName;
    float m_iAccuracy;
    int   m_iAmmoGrenade;
    float m_iBalconyHeight;
    int   m_iDisguiseLevel;
    float m_fDisguisePeriod;
    float m_fDisguiseRange;
    float m_fEnemyShareRange;
    float m_fFixedLeash;
    float m_fGrenadeAwareness;
    str   m_sGun;
    float m_fMaxNoticeTimeScale;
    float m_fSoundAwareness;
    str   m_sTypeAttack;
    str   m_sTypeDisguise;
    str   m_sTypeGrenade;
    str   m_sTypeIdle;
    bool  m_bPatrolWaitTrigger;
    float m_fHearing;
    float m_fSight;
    float m_fFov;
    float m_fLeash;
    float m_fMinDist;
    float m_fMaxDist;
    float m_fInterval;
    bool  m_bDontDropWeapons;
    bool  m_bDontDropHealth;
    str   m_sFavoriteEnemy;
    bool  m_bNoSurprise;
    str   m_sPatrolPath;
    str   m_sTurret;
    str   m_sAlarmNode;
    str   m_sWeapon;
    str   m_sTarget;
    str   m_sVoiceType;
    bool  m_bForceDropWeapon;
    bool  m_bForceDropHealth;

public:
    CLASS_PROTOTYPE(AISpawnPoint);

    AISpawnPoint();

    void GetForceDropHealth(Event *ev);
    void SetForceDropHealth(Event *ev);
    void GetForceDropWeapon(Event *ev);
    void SetForceDropWeapon(Event *ev);
    void GetVoiceType(Event *ev);
    void SetVoiceType(Event *ev);
    void GetTarget(Event *ev);
    void SetTarget(Event *ev);
    void GetWeapon(Event *ev);
    void SetWeapon(Event *ev);
    void GetAlarmNode(Event *ev);
    void SetAlarmNode(Event *ev);
    void GetTurret(Event *ev);
    void SetTurret(Event *ev);
    void GetPatrolPath(Event *ev);
    void SetPatrolPath(Event *ev);
    void GetNoSurprise(Event *ev);
    void SetNoSurprise(Event *ev);
    void SetFavoriteEnemy(Event *ev);
    void GetFavoriteEnemy(Event *ev);
    void GetDontDropHealth(Event *ev);
    void SetDontDropHealth(Event *ev);
    void GetDontDropWeapons(Event *ev);
    void SetDontDropWeapons(Event *ev);
    void GetInterval(Event *ev);
    void SetInterval(Event *ev);
    void GetMaxDistance(Event *ev);
    void SetMaxDistance(Event *ev);
    void GetMinDistance(Event *ev);
    void SetMinDistance(Event *ev);
    void GetLeash(Event *ev);
    void SetLeash(Event *ev);
    void GetFov(Event *ev);
    void SetFov(Event *ev);
    void SetSight(Event *ev);
    void GetSight(Event *ev);
    void GetHearing(Event *ev);
    void SetHearing(Event *ev);
    void GetPatrolWaitTrigger(Event *ev);
    void SetPatrolWaitTrigger(Event *ev);
    void GetTypeIdle(Event *ev);
    void SetTypeIdle(Event *ev);
    void GetTypeGrenade(Event *ev);
    void SetTypeGrenade(Event *ev);
    void GetTypeDisguise(Event *ev);
    void SetTypeDisguise(Event *ev);
    void GetTypeAttack(Event *ev);
    void SetTypeAttack(Event *ev);
    void SetSoundAwareness(Event *ev);
    void GetSoundAwareness(Event *ev);
    void SetMaxNoticeTimeScale(Event *ev);
    void GetMaxNoticeTimeScale(Event *ev);
    void GetGun(Event *ev);
    void SetGun(Event *ev);
    void GetGrenadeAwareness(Event *ev);
    void SetGrenadeAwareness(Event *ev);
    void SetFixedLeash(Event *ev);
    void GetFixedLeash(Event *ev);
    void GetEnemyShareRange(Event *ev);
    void SetEnemyShareRange(Event *ev);
    void GetDisguiseRange(Event *ev);
    void SetDisguiseRange(Event *ev);
    void GetDisguisePeriod(Event *ev);
    void SetDisguisePeriod(Event *ev);
    void GetDisguiseLevel(Event *ev);
    void SetDisguiseLevel(Event *ev);
    void GetBalconyHeight(Event *ev);
    void SetBalconyHeight(Event *ev);
    void GetAmmoGrenade(Event *ev);
    void SetAmmoGrenade(Event *ev);
    void GetAccuracy(Event *ev);
    void SetAccuracy(Event *ev);
    void GetEnemyName(Event *ev);
    void SetEnemyName(Event *ev);
    void GetHealth(Event *ev);
    void SetHealth(Event *ev);
    void GetModel(Event *ev);
    void SetModel(Event *ev);

    void Archive(Archiver& arc);
};
