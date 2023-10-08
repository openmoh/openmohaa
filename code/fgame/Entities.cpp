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

#include "Entities.h"
#include "g_phys.h"
#include "g_spawn.h"
#include "scriptthread.h"
#include "game.h"
#include "weapon.h"
#include "weaputils.h"
#include "actor.h"
#include "sentient.h"
#include "player.h"

Container<ProjectileTarget *> g_projectileTargets;

/*QUAKED ProjectileGenerator_Target (1.0 0.0 1.0) (-12 -12 -12) (12 12 12)

  Place this target down and assign it a ProjectileGenerator ID and it will
  shoot here....

  Here are the params:

  ID      <nIdNum>   (Generator ID number.)

******************************************************************************/

Event EV_PG_SetID
(
    "ID",
    EV_DEFAULT,
    "i",
    "ID",
    "Sets the ID for this projectile generator\nDefault=0:",
    EV_NORMAL
);
Event EV_PG_SetModel
(
    "Model",
    EV_DEFAULT,
    "s",
    "Model",
    "Projectile model to use.\nDefault=models/projectiles/bazookashell.tik",
    EV_NORMAL
);
Event EV_PG_MinDuration
(
    "MinDuration",
    EV_DEFAULT,
    "f",
    "MinDuration",
    "Sets the minimum duration of the bursts (in seconds)\nDefault=3",
    EV_NORMAL
);
Event EV_PG_MaxDuration
(
    "MaxDuration",
    EV_DEFAULT,
    "f",
    "MaxDuration",
    "Sets the maximum duration of bursts(in seconds)\nDefault=3",
    EV_NORMAL
);
Event EV_PG_MinNumShots(
    "MinNumShots",
    EV_DEFAULT,
    "i",
    "MinNumShots",
    "Sets the minimum # of shots to fire in a cycle\nDefault=1",
    EV_NORMAL
);
Event EV_PG_MaxNumShots(
    "MaxNumShots",
    EV_DEFAULT,
    "i",
    "MaxNumShots",
    "Sets the maximum # of shots to fire in a cycle\nDefault=1",
    EV_NORMAL
);
Event EV_PG_Cycles
(
    "Cycles",
    EV_DEFAULT,
    "i",
    "Cycles",
    "Number of cycles. 0=infinte\nDefault=0",
    EV_NORMAL
);
Event EV_PG_MinDelay
(
    "MinDelay",
    EV_DEFAULT,
    "f",
    "MinDelay",
    "Minimum time between bursts.\n",
    EV_NORMAL
);
Event EV_PG_MaxDelay
(
    "MaxDelay",
    EV_DEFAULT,
    "f",
    "MaxDelay",
    "Maximum time between bursts\nDefault=10",
    EV_NORMAL
);
Event EV_PG_Accuracy
(
    "Accuracy",
    EV_DEFAULT,
    "f",
    "Accuracy",
    "Accuracy 0-25 feet\nDefault=10",
    EV_NORMAL
);
Event EV_PG_ClearTarget
(
    "ClearTarget",
    EV_DEFAULT,
    NULL,
    NULL,
    "Pick another target...",
    EV_NORMAL
);
Event EV_PG_BeginCycle
(
    "BeginCycle",
    EV_DEFAULT,
    NULL,
    NULL,
    "Startup the cycle..",
    EV_NORMAL
);
Event EV_TickCycle
(
    "TickCycle",
    EV_DEFAULT,
    NULL,
    NULL,
    "Tick the cycle..",
    EV_NORMAL
);
Event EV_EndCycle
(
    "EndCycle",
    EV_DEFAULT,
    NULL,
    NULL,
    "End the cycle..",
    EV_NORMAL
);
Event EV_TurnOn
(
    "TurnOn",
    EV_DEFAULT,
    NULL,
    NULL,
    "Turn On and begin the first cycle",
    EV_NORMAL
);
Event EV_TurnOff
(
    "TurnOff",
    EV_DEFAULT,
    NULL,
    NULL,
    "Turn Off and cancel the current cycle (if any)",
    EV_NORMAL
);
Event EV_Set_FireDelay
(
    "firedelay",
    EV_DEFAULT,
    "f",
    "fFireDelay",
    "Set the minimum time between shots from the weapon",
    EV_NORMAL
);
Event EV_Initialize
(
    "initialize",
    EV_DEFAULT,
    NULL,
    NULL,
    "Initialize object",
    EV_NORMAL
);
Event EV_Set_FireOnStartUp(
    "FireOnStartUp",
    EV_DEFAULT,
    NULL,
    "i",
    "Default=1; Set this to 0 if you don't want the PG to fire on startup.",
    EV_NORMAL
);
Event EV_PG_isDonut
(
    "isdonut",
    EV_DEFAULT,
    "i",
    "isdonut",
    "will target donut",
    EV_NORMAL
);
Event EV_PG_arcDonut
(
    "arc",
    EV_DEFAULT,
    "f",
    "arc",
    "arc in front of the player",
    EV_NORMAL
);
Event EV_PG_minDonut
(
    "mindonut",
    EV_DEFAULT,
    "f",
    "mindonut",
    "min extent of donut from player",
    EV_NORMAL
);
Event EV_PG_maxDonut
(
    "maxdonut",
    EV_DEFAULT,
    "f",
    "maxdonut",
    "max extent of donut from player",
    EV_NORMAL
);
Event EV_PG_PlayPreImpactSound
(
    "playpreimpact",
    EV_DEFAULT,
    NULL,
    NULL,
    "Play a pre-impact sound",
    EV_NORMAL
);
Event EV_PG_SetPreImpactSound
(
    "preimpactsound",
    EV_DEFAULT,
    "s",
    "Sound",
    "Set a pre-impact sound.",
    EV_NORMAL
);
Event EV_PG_SetPreImpactSoundTime
(
    "preimpactsoundtime",
    EV_DEFAULT,
    "f",
    "delay",
    "Set the time before impact to play the preimpact sound.",
    EV_NORMAL
);
Event EV_PG_SetPreImpactSoundProbability(
    "preimpactsoundprob",
    EV_DEFAULT,
    "f",
    "probability",
    "Set the chance for a pre-impact sound to occur.\n"
    "Range: ( 0.0, 1.0 ), with 0 having no chance, and 1 always occurring.",
    EV_NORMAL
);
Event EV_PG_SetLaunchSound
(
    "launchsound",
    EV_DEFAULT,
    "s",
    "Sound",
    "Set a launch sound for the projectile.",
    EV_NORMAL
);
Event EV_PG_GetTargetEntity
(
    "gettargetentity",
    EV_DEFAULT,
    NULL,
    NULL,
    "Returns entity being targeted by the projectile generator,",
    EV_GETTER
);
Event EV_PG_IsTurnedOn
(
    "isturnedon",
    EV_DEFAULT,
    NULL,
    NULL,
    "Returns whether the generator is on or off.",
    EV_GETTER
);

CLASS_DECLARATION(Entity, ProjectileTarget, "ProjectileGenerator_Target") {
    {&EV_PG_SetID, &ProjectileTarget::EventSetId},
    {NULL,         NULL                         }
};

void ClearProjectileTargets()
{
    g_projectileTargets.ClearObjectList();
}

ProjectileTarget::ProjectileTarget()
{
    m_iTarget = -1;
    setMoveType(MOVETYPE_NONE);
    setSolidType(SOLID_NOT);
    hideModel();

    g_projectileTargets.AddObject(this);
}

void ProjectileTarget::EventSetId(Event *ev)
{
    m_iTarget = ev->GetInteger(1);
}

int ProjectileTarget::GetTarget() const
{
    return m_iTarget;
}

CLASS_DECLARATION(Animate, ProjectileGenerator, "ProjectileGenerator") {
    {&EV_PG_SetID,           &ProjectileGenerator::EventSetId          },
    {&EV_PG_SetModel,        &ProjectileGenerator::SetWeaponModel      },
    {&EV_PG_MinDuration,     &ProjectileGenerator::EventMinDuration    },
    {&EV_PG_MaxDuration,     &ProjectileGenerator::EventMaxDuration    },
    {&EV_PG_Cycles,          &ProjectileGenerator::EventCycles         },
    {&EV_PG_MinNumShots,     &ProjectileGenerator::EventMinNumShots    },
    {&EV_PG_MaxNumShots,     &ProjectileGenerator::EventMaxNumShots    },
    {&EV_Set_FireOnStartUp,  &ProjectileGenerator::EventFireOnStartUp  },
    {&EV_PG_MinDelay,        &ProjectileGenerator::EventMinDelay       },
    {&EV_PG_MaxDelay,        &ProjectileGenerator::EventMaxDelay       },
    {&EV_PG_Accuracy,        &ProjectileGenerator::EventAccuracy       },
    {&EV_PG_BeginCycle,      &ProjectileGenerator::BeginCycle          },
    {&EV_TickCycle,          &ProjectileGenerator::TickCycle           },
    {&EV_EndCycle,           &ProjectileGenerator::EndCycle            },
    {&EV_TurnOn,             &ProjectileGenerator::TurnOn              },
    {&EV_TurnOff,            &ProjectileGenerator::TurnOff             },
    {&EV_Initialize,         &ProjectileGenerator::OnInitialize        },
    {&EV_Activate,           &ProjectileGenerator::TurnOn              },
    {&EV_SetTarget,          &ProjectileGenerator::SetTarget           },
    {&EV_PG_SetLaunchSound,  &ProjectileGenerator::EventLaunchSound    },
    {&EV_PG_GetTargetEntity, &ProjectileGenerator::EventGetTargetEntity},
    {&EV_PG_IsTurnedOn,      &ProjectileGenerator::EventIsTurnedOn     },
    {NULL,                   NULL                                      }
};

ProjectileGenerator::ProjectileGenerator()
{
    m_iId            = -1;
    m_fMinDuration   = 1;
    m_fMaxDuration   = 3;
    m_iCycles        = 0;
    m_fMinDelay      = 3.0;
    m_fMaxDelay      = 10;
    m_fAccuracy      = 10;
    m_iCurrentCycle  = 0;
    m_iMinNumShots   = 1;
    m_iMaxNumShots   = 1;
    m_fCurrentTime   = 0;
    m_fShotsPerSec   = 0;
    m_iTargetIndex   = -1;
    m_fLastShotTime  = 0;
    m_iAnimSlot      = 0;
    m_bIsTurnedOn    = false;
    m_bFireOnStartUp = true;
    m_bIsDonut       = false;
    m_fArcDonut      = 0;
    m_fMinDonut      = 0;
    m_fMaxDonut      = 0;
    m_pTarget        = NULL;

    m_fCycleTime = 1;
    setMoveType(MOVETYPE_NONE);
    setSolidType(SOLID_NOT);
    if (ShouldPlayFireSound()) {
        if (ShouldHideModel()) {
            hideModel();
            edict->r.svFlags |= SVF_NOCLIENT;
        }
    } else {
        hideModel();
    }

    if (!LoadingSavegame && ShouldStartOn()) {
        PostEvent(EV_TurnOn, 0.1f);
    }

    m_pCurrent = NULL;
    if (!LoadingSavegame) {
        PostEvent(EV_Initialize, 0.001f);
    }
}

bool ProjectileGenerator::ShouldStartOn() const
{
    return (spawnflags & PT_SPAWNFLAG_TURN_ON) ? true : false;
}

bool ProjectileGenerator::ShouldHideModel() const
{
    return (spawnflags & PT_SPAWNFLAG_HIDDEN) ? true : false;
}

bool ProjectileGenerator::ShouldPlayFireSound() const
{
    return (spawnflags & PT_SPAWNFLAG_PLAY_FIRE_SOUND) ? true : false;
}

bool ProjectileGenerator::ShouldPlayMotionSound() const
{
    return (spawnflags & PT_SPAWNFLAG_PLAY_MOTION_SOUND) ? true : false;
}

bool ProjectileGenerator::ShouldRotateYaw() const
{
    return (spawnflags & PT_SPAWNFLAG_ROTATE_YAW) ? true : false;
}

bool ProjectileGenerator::ShouldRotateRoll() const
{
    return (spawnflags & PT_SPAWNFLAG_ROTATE_ROLL) ? true : false;
}

void ProjectileGenerator::EventIsTurnedOn(Event *ev)
{
    ev->AddInteger(m_bIsTurnedOn);
}

void ProjectileGenerator::EventGetTargetEntity(Event *ev)
{
    ev->AddEntity(m_pCurrent);
}

void ProjectileGenerator::EventLaunchSound(Event *ev)
{
    m_sLaunchSound = ev->GetString(1);
}

void ProjectileGenerator::SetTarget(Event *ev)
{
    SetTarget(ev->GetEntity(1));
}

void ProjectileGenerator::SetTarget(Entity *ent)
{
    m_pTarget = ent;
}

void ProjectileGenerator::OnInitialize(Event *ev)
{
    if (ShouldRotateYaw()) {
        angles.setYaw(angles.yaw() + 180);
    }
    if (ShouldRotateRoll()) {
        angles.setRoll(angles.roll() + 180);
    }
}

void ProjectileGenerator::TurnOff(Event *ev)
{
    CancelEventsOfType(EV_PG_BeginCycle);
    CancelEventsOfType(EV_TickCycle);
    m_bIsTurnedOn = false;
}

void ProjectileGenerator::TurnOn(Event *ev)
{
    if (!m_bIsTurnedOn) {
        CancelEventsOfType(EV_PG_BeginCycle);
        CancelEventsOfType(EV_TickCycle);
        m_bIsTurnedOn = true;
    }
}

void ProjectileGenerator::SetupNextCycle()
{
    float numShots;
    float delay;

    if (m_iCycles && m_iCurrentCycle >= m_iCycles) {
        m_pCurrent = NULL;
        TurnOff(NULL);
        return;
    }

    // retrieve the target
    m_pCurrent   = ChooseTarget();
    m_fCycleTime = G_Random(m_fMaxDuration - m_fMinDuration) + m_fMinDuration;

    // get a random number of shots
    numShots = m_iMinNumShots;
    if (numShots < m_iMaxNumShots) {
        numShots = rand() % (m_iMaxNumShots - m_iMinNumShots) + m_iMinNumShots;
    }

    // get a random delay
    m_fShotsPerSec = numShots / m_fCycleTime;
    if (m_bIsTurnedOn || !m_bFireOnStartUp) {
        delay = G_Random(m_fMaxDelay - m_fMinDelay) + m_fMinDelay;
    }

    PostEvent(EV_PG_BeginCycle, delay);
}

bool ProjectileGenerator::ShouldTargetRandom() const
{
    return (spawnflags & PT_SPAWNFLAG_TARGET_RANDOM) ? true : false;
}

Entity *ProjectileGenerator::ChooseTarget()
{
    int numTargets;

    if (m_pTarget) {
        m_vTargetOrg = GetTargetPos(m_pTarget);
        return m_pTarget;
    }

    if (ShouldTargetPlayer()) {
        Entity *target = G_FindTarget(this, "player");
        if (target) {
            m_vTargetOrg = GetTargetPos(target);
            return target;
        }
    }

    if (!m_projectileTargets.NumObjects()) {
        GetLocalTargets();
    }

    numTargets = m_projectileTargets.NumObjects();
    if (numTargets) {
        ProjectileTarget *target;

        if (ShouldTargetRandom()) {
            target = m_projectileTargets.ObjectAt((rand() % numTargets) + 1);
        } else {
            m_iTargetIndex = (m_iTargetIndex + 1) % numTargets;
            target         = m_projectileTargets.ObjectAt(m_iTargetIndex + 1);
        }

        m_vTargetOrg = GetTargetPos(target);
        return target;
    }

    spawnflags |= PT_SPAWNFLAG_TARGET_PLAYER;
    return ChooseTarget();
}

void ProjectileGenerator::GetLocalTargets()
{
    int i;

    m_projectileTargets.ClearObjectList();
    for (i = 1; i <= g_projectileTargets.NumObjects(); i++) {
        ProjectileTarget *target = g_projectileTargets.ObjectAt(i);
        if (m_iId == target->GetTarget()) {
            m_projectileTargets.AddObject(target);
        }
    }
}

bool ProjectileGenerator::ShouldTargetPlayer() const
{
    return (spawnflags & PT_SPAWNFLAG_TARGET_PLAYER) ? true : false;
}

Vector ProjectileGenerator::GetTargetPos(Entity *target)
{
    Vector vOrg;
    Vector vForward;
    Vector vAngles;
    float  fRandDonut;

    if (!target) {
        return Vector(0, 0, 0);
    }

    vOrg = target->origin;

    if (target->IsSubclassOfActor()) {
        vOrg += Vector(0, 0, 36);
    }

    if (m_bIsDonut) {
        Entity *playerTarget = G_FindTarget(this, "player");

        if (playerTarget) {
            vAngles = playerTarget->client->ps.viewangles;
        } else {
            vAngles = target->angles;
        }

        if (rand() & 1) {
            vAngles.setYaw(vAngles[1] + m_fArcDonut / 2.0);
        } else {
            vAngles.setYaw(vAngles[1] - m_fArcDonut / 2.0);
        }

        AngleVectors(vAngles, vForward, NULL, NULL);

        fRandDonut = G_Random(m_fMaxDonut - m_fMinDonut) + m_fMinDonut;
        vOrg += vForward * (fRandDonut * 16);
    } else {
        vAngles.setYaw(G_Random(360));
        AngleVectors(vAngles, vForward, NULL, NULL);

        fRandDonut = G_Random(m_fAccuracy) * 16;
        vOrg += vForward * fRandDonut;
    }

    return vOrg;
}

void ProjectileGenerator::GetMuzzlePos(Vector& pos)
{
    orientation_t orient;

    pos = origin;

    if (GetRawTag("tag_barrel", &orient)) {
        vec3_t axis[3];
        int    i;

        AnglesToAxis(angles, axis);
        for (i = 0; i < 3; i++) {
            VectorMA(pos, orient.origin[i], axis[i], pos);
        }
    }
}

void ProjectileGenerator::Fire()
{
    m_fLastShotTime = level.time;
    SetWeaponAnim("Fire", 0);
}

void ProjectileGenerator::TryLaunchSound()
{
    if (m_sLaunchSound != "") {
        Sound(m_sLaunchSound, CHAN_AUTO);
    }
}

void ProjectileGenerator::SetWeaponAnim(const char *name, Event *ev)
{
    int animnum = gi.Anim_NumForName(edict->tiki, name);
    if (animnum == -1) {
        return;
    }

    StopAnimating(m_iAnimSlot);
    RestartAnimSlot(m_iAnimSlot);

    int idleanim = gi.Anim_NumForName(edict->tiki, "idle");

    edict->s.frameInfo[m_iAnimSlot].index = idleanim;
    m_iAnimSlot                           = (m_iAnimSlot + 1) & 3;
    edict->s.frameInfo[m_iAnimSlot].index = idleanim;

    if (ev) {
        NewAnim(animnum, ev, m_iAnimSlot);
    } else {
        NewAnim(animnum, m_iAnimSlot);
    }

    SetOnceType(m_iAnimSlot);
    RestartAnimSlot(m_iAnimSlot);
}

void ProjectileGenerator::EventAccuracy(Event *ev)
{
    m_fAccuracy = ev->GetFloat(1);
}

void ProjectileGenerator::EventMaxDelay(Event *ev)
{
    m_fMaxDelay = ev->GetFloat(1);
}

void ProjectileGenerator::EventMinDelay(Event *ev)
{
    m_fMinDelay = ev->GetFloat(1);
}

void ProjectileGenerator::EventFireOnStartUp(Event *ev)
{
    m_bFireOnStartUp = ev->GetBoolean(1);
}

void ProjectileGenerator::EventMaxNumShots(Event *ev)
{
    m_iMaxNumShots = ev->GetInteger(1);
}

void ProjectileGenerator::EventMinNumShots(Event *ev)
{
    m_iMinNumShots = ev->GetInteger(1);
}

void ProjectileGenerator::EventCycles(Event *ev)
{
    m_iCycles = ev->GetInteger(1);
}

void ProjectileGenerator::EventMaxDuration(Event *ev)
{
    m_fMaxDuration = ev->GetFloat(1);
}

void ProjectileGenerator::EventMinDuration(Event *ev)
{
    m_fMinDuration = ev->GetFloat(1);
}

void ProjectileGenerator::SetWeaponModel(Event *ev)
{
    setModel(ev->GetString(1));
}

void ProjectileGenerator::EventSetId(Event *ev)
{
    m_iId = ev->GetInteger(1);
}

void ProjectileGenerator::BeginCycle(Event *ev)
{
    if (!m_bIsTurnedOn) {
        return;
    }

    if (!m_fShotsPerSec) {
        SetupNextCycle();
        return;
    }

    if (m_pCurrent) {
        m_vTargetOrg = GetTargetPos(m_pCurrent);
    }

    m_iCurrentCycle++;
    m_fCurrentTime = 0;

    TickCycle(ev);

    PostEvent(EV_EndCycle, m_fCycleTime);
}

void ProjectileGenerator::TickCycle(Event *ev)
{
    if (!m_bIsTurnedOn) {
        return;
    }

    m_fCurrentTime += level.frametime * m_fShotsPerSec;
    if (m_fCurrentTime >= 1.f) {
        float f = floor(m_fCurrentTime);
        if (Attack(floor(f))) {
            m_fCurrentTime -= floor(f);
        }
    }

    PostEvent(EV_TickCycle, 0.01f);
}

void ProjectileGenerator::EndCycle(Event *ev)
{
    CancelEventsOfType(EV_TickCycle);
    SetupNextCycle();
}

void ProjectileGenerator::EventmaxDonut(Event *ev)
{
    m_fMaxDonut = ev->GetFloat(1);
}

void ProjectileGenerator::EventminDonut(Event *ev)
{
    m_fMinDonut = ev->GetFloat(1);
}

void ProjectileGenerator::EventarcDonut(Event *ev)
{
    m_fArcDonut = ev->GetFloat(1);
}

void ProjectileGenerator::EventisDonut(Event *ev)
{
    m_bIsDonut = ev->GetBoolean(1);
}

bool ProjectileGenerator::Attack(int count)
{
    return true;
}

void ProjectileGenerator::Archive(Archiver& arc)
{
    Animate::Archive(arc);

    arc.ArchiveInteger(&m_iId);
    arc.ArchiveInteger(&m_iCycles);
    arc.ArchiveFloat(&m_fMinDuration);
    arc.ArchiveFloat(&m_fMaxDuration);
    arc.ArchiveInteger(&m_iMinNumShots);
    arc.ArchiveInteger(&m_iMaxNumShots);
    arc.ArchiveFloat(&m_fMinDelay);
    arc.ArchiveFloat(&m_fMaxDelay);
    arc.ArchiveFloat(&m_fAccuracy);
    arc.ArchiveInteger(&m_iCurrentCycle);
    arc.ArchiveFloat(&m_fCycleTime);
    arc.ArchiveFloat(&m_fShotsPerSec);
    arc.ArchiveFloat(&m_fCurrentTime);
    arc.ArchiveInteger(&m_iTargetIndex);
    arc.ArchiveFloat(&m_fLastShotTime);
    arc.ArchiveInteger(&m_iAnimSlot);
    arc.ArchiveBool(&m_bIsTurnedOn);
    arc.ArchiveInteger(&m_bIsDonut);
    arc.ArchiveFloat(&m_fArcDonut);
    arc.ArchiveFloat(&m_fMinDonut);
    arc.ArchiveFloat(&m_fMaxDonut);
    arc.ArchiveString(&m_sLaunchSound);
    arc.ArchiveBool(&m_bFireOnStartUp);
    arc.ArchiveObjectPointer((Class **)&m_pTarget);
    arc.ArchiveObjectPointer((Class **)&m_pCurrent);
}

/*QUAKED ProjectileGenerator_Projectile (1.0 0.0 1.0) (-12 -12 -12) (12 12 12)  LaunchSound x TargetRandom StartOn FlipYaw FlipRoll TargetPlayer HideModel

  Allows you to launch projectiles without the need for actual AI.  Useful for faking large battles.
  Here are the params:

  ID <nIdNum>
  Model		 <sModelName>			(Defaults to bazooka weapon)
  Projectile <sModelName>			(Defaults to bazooka projectile)
  MinDuration <nMinDur=1>           (Min burst cycle duration)
  MaxDuration <nMaxDur=3>           (Max burst cycle duration)
  Cycles <nCycles=0>                (0=infinite)
  MinNumShots <nMinNumShots=1>      (Minimum # of shots per cycle)
  MaxNumShots <nMaxNumShots=1>      (Maximum # of shots per cycle)

  MinDelay <nDelay=2>               (min amount of time between cycles)
  MaxDelay <nDelay=10>              (max amount of time between cycles)
  Accuracy <nAccuracy=10>           (accuracy measured in feet. 0-25 feet range)
  FireOnStartUp						(defaults to 1.  This flag tells the PG to fire immediately when turned on.  Then do its delay from then on.)

  IsDonut	<nIsDonut=1>			(indicates to use donut to calculate impact location)
  Arc		<nArc=#degrees)			(how many degrees does the arc have)
  minDonut <nMinDonut=15>			(mindistance from player)
  maxDonut <nMaxDonut=30>			(maxdistance from player)
  launchsound <sSoundName>          (Defaults to no sound)

  preimpactsound <sSoundName>       (Defaults to no sound)
  preimpactsoundtime <nTime=1.0>    (Time before impact to begin preimpact sound)
  preimpactsoundprob <nProb=0.0>    (Chance of an impact sound being played. 0..1 range )

  Here are some useful script commands:

  $<generatorName> TurnOn
  $<generatorName> TurnOff
  $<generatorName> playpreimpact     ( plays the preimpact sound immediately )
  $<generatorName> gettargetentity   ( return the entity that is targeted )
  $<generatorName> isturnedon        ( is the projectil generator turned on? )

******************************************************************************/

CLASS_DECLARATION(ProjectileGenerator, ProjectileGenerator_Projectile, "ProjectileGenerator_Projectile") {
    {&EV_Weapon_SetProjectile,            &ProjectileGenerator_Projectile::SetProjectileModel          },
    {&EV_PG_isDonut,                      &ProjectileGenerator_Projectile::EventisDonut                },
    {&EV_PG_arcDonut,                     &ProjectileGenerator_Projectile::EventarcDonut               },
    {&EV_PG_minDonut,                     &ProjectileGenerator_Projectile::EventminDonut               },
    {&EV_PG_maxDonut,                     &ProjectileGenerator_Projectile::EventmaxDonut               },
    {&EV_PG_PlayPreImpactSound,           &ProjectileGenerator_Projectile::PlayPreImpactSound          },
    {&EV_PG_SetPreImpactSound,            &ProjectileGenerator_Projectile::SetPreImpactSound           },
    {&EV_PG_SetPreImpactSoundTime,        &ProjectileGenerator_Projectile::SetPreImpactSoundTime       },
    {&EV_PG_SetPreImpactSoundProbability, &ProjectileGenerator_Projectile::SetPreImpactSoundProbability},
    {NULL,                                NULL                                                         }
};

ProjectileGenerator_Projectile::ProjectileGenerator_Projectile()
{
    if (LoadingSavegame) {
        return;
    }

    m_sProjectileModel = "models/projectiles/bazookashell.tik";
    setModel("models/weapons/bazooka.tik");
    m_sPreImpactSound         = "";
    m_fImpactSoundTime        = 1;
    m_fImpactSoundProbability = 0;
}

void ProjectileGenerator_Projectile::SetPreImpactSoundProbability(Event *ev)
{
    m_fImpactSoundProbability = ev->GetFloat(1);
}

void ProjectileGenerator_Projectile::SetPreImpactSoundTime(Event *ev)
{
    m_fImpactSoundTime = ev->GetFloat(1);
}

void ProjectileGenerator_Projectile::SetPreImpactSound(Event *ev)
{
    m_sPreImpactSound = ev->GetString(1);
}

void ProjectileGenerator_Projectile::PlayPreImpactSound(Event *ev)
{
    Sound(m_sPreImpactSound, CHAN_AUTO);
}

void ProjectileGenerator_Projectile::SetProjectileModel(Event *ev)
{
    m_sProjectileModel = ev->GetString(1);
}

float ProjectileGenerator_Projectile::EstimateImpactTime(
    const Vector& targetOrigin, const Vector& fromOrigin, float speed
) const
{
    Vector delta = fromOrigin - targetOrigin;

    return delta.length() / speed;
}

bool ProjectileGenerator_Projectile::Attack(int count)
{
    Projectile *proj;
    static int  NukePlayer = 0;
    Vector      dir(0, 0, 1);
    float       speed = 500;

    Vector muzzlePos = origin;
    GetMuzzlePos(muzzlePos);

    if (m_pCurrent) {
        m_vTargetOrg = GetTargetPos(m_pCurrent);

        if (m_bIsDonut) {
            Entity *playerTarget = G_FindTarget(this, "player");

            if (playerTarget) {
                if (playerTarget->takedamage) {
                    if (!Q_stricmp(targetname, "artillerykiller")) {
                        NukePlayer++;
                    }
                } else {
                    NukePlayer = 0;
                }

                if (NukePlayer > 2) {
                    m_vTargetOrg = playerTarget->origin;
                    muzzlePos    = playerTarget->origin;
                    muzzlePos[2] += 256;
                    NukePlayer = 0;
                } else {
                    muzzlePos = m_vTargetOrg;
                    muzzlePos[2] += random() * 500.0 + 500.0;
                }
            } else {
                muzzlePos = m_vTargetOrg;
                muzzlePos[2] += 1000.0;
            }
        }

        dir = m_vTargetOrg - muzzlePos;
        dir.normalize();

        Vector newAngles;
        VectorToAngles(dir, newAngles);

        if (ShouldRotateYaw()) {
            newAngles.setYaw(newAngles.yaw() + 180);
        }

        if (ShouldRotateRoll()) {
            newAngles.setRoll(newAngles.roll() + 180);
        }

        setAngles(newAngles);
    } else {
        AngleVectors(angles, dir, NULL, NULL);
    }

    Fire();
    TryLaunchSound();

    proj = ProjectileAttack(muzzlePos, dir, this, m_sProjectileModel, 1.0, speed);

    if (proj->flags & FL_THINK) {
        gi.DPrintf("Projectile used is thinking...pre-impact sound cannot be predicted.\n");
        return true;
    }

    if (G_Random() <= m_fImpactSoundProbability && m_sPreImpactSound.length()) {
        float time;

        time = EstimateImpactTime(m_vTargetOrg, muzzlePos, speed);
        time -= 1.0;

        if (time > 0) {
            PostEvent(EV_PG_PlayPreImpactSound, time);
        }
    }

    return true;
}

void ProjectileGenerator_Projectile::Archive(Archiver& arc)
{
    ProjectileGenerator::Archive(arc);

    arc.ArchiveString(&m_sProjectileModel);
    arc.ArchiveString(&m_sPreImpactSound);
    arc.ArchiveFloat(&m_fImpactSoundTime);
    arc.ArchiveFloat(&m_fImpactSoundProbability);
}

/*QUAKED ProjectileGenerator_Gun (1.0 0.0 1.0) (-12 -12 -12) (12 12 12) LaunchSound x TargetRandom StartOn FlipYaw FlipRoll TargetPlayer HideModel

  Allows you to launch projectiles without the need for actual AI.  Useful for faking large battles.
  Here are the params:

  ID <nIdNum>
  MinDuration <nMinDur=1>           (Min burst cycle duration)
  MaxDuration <nMaxDur=3>           (Max burst cycle duration)
  Cycles <nCycles=0>                (0=infinite)
  MinNumShots <nMinNumShots=1>      (Minimum # of shots per cycle)
  MaxNumShots <nMaxNumShots=1>      (Maximum # of shots per cycle)

  MinDelay <nDelay=2>               (min amount of time between cycles)
  MaxDelay <nDelay=10>              (max amount of time between cycles)
  Accuracy <nAccuracy=10>           (accuracy measured in feet. 0-25 feet range)
  launchsound <sSoundName>          (Defaults to no sound)
  FireOnStartUp						(defaults to 1.  This flag tells the PG to fire immediately when turned on.  Then do its delay from then on.)

  bulletLarge <0,1,2,3>				(whether to fire normal, rifle, artillery or larger bullets)
  tracerSpeed <0,2>					(scale factor for tracer speed)

  Here are some useful script commands:

  $<generatorName> TurnOn
  $<generatorName> TurnOff
  $<generatorName> gettargetentity   ( return the entity that is targeted )
  $<generatorName> isturnedon        ( is the projectil generator turned on? )

******************************************************************************/

Event EV_PG_SetFakeBullets(
    "fakebullets",
    EV_DEFAULT,
    "b",
    "useFakeBullets",
    "Whether or not the projectile generator should fire real bullets."
);

CLASS_DECLARATION(ProjectileGenerator, ProjectileGenerator_Gun, "ProjectileGenerator_Gun") {
    {&EV_PG_SetID,                     &ProjectileGenerator_Gun::EventSetId           },
    {&EV_Weapon_SetBulletRange,        &ProjectileGenerator_Gun::SetBulletRange       },
    {&EV_Weapon_SetBulletSpread,       &ProjectileGenerator_Gun::SetBulletSpread      },
    {&EV_Weapon_SetTracerFrequency,    &ProjectileGenerator_Gun::SetTracerFrequency   },
    {&EV_Weapon_SetBulletDamage,       &ProjectileGenerator_Gun::SetBulletDamage      },
    {&EV_Weapon_SetBulletCount,        &ProjectileGenerator_Gun::SetBulletCount       },
    {&EV_PG_SetFakeBullets,            &ProjectileGenerator_Gun::SetFakeBullets       },
    {&EV_Weapon_SetBulletLarge,        &ProjectileGenerator_Gun::SetBulletLarge       },
    {&EV_Weapon_SetTracerSpeed,        &ProjectileGenerator_Gun::SetTracerSpeed       },
    {&EV_Weapon_SetBulletKnockback,    &ProjectileGenerator_Gun::SetBulletKnockback   },
    {&EV_Weapon_SetBulletThroughWood,  &ProjectileGenerator_Gun::SetBulletThroughWood },
    {&EV_Weapon_SetBulletThroughMetal, &ProjectileGenerator_Gun::SetBulletThroughMetal},
    {&EV_Weapon_SetMeansOfDeath,       &ProjectileGenerator_Gun::SetMeansOfDeath      },
    {&EV_Weapon_FireDelay,             &ProjectileGenerator_Gun::SetFireDelay         },
    {NULL,                             NULL                                           }
};

ProjectileGenerator_Gun::ProjectileGenerator_Gun()
{
    m_fBulletRange        = 4000;
    m_fBulletDamage       = 40;
    m_iBulletCount        = 1;
    m_vBulletSpread       = Vector(40, 40, 0);
    m_iTracerFrequency    = 0;
    m_iBulletLarge        = 0;
    m_iMeansOfDeath       = MOD_BULLET;
    m_fBulletThroughWood  = 0;
    m_fBulletThroughMetal = 0;
    m_iBulletKnockback    = 0;
    m_iAttackCount        = 0;
    m_fFireDelay          = 1;
    m_fTracerSpeed        = 1.0;
    m_bFakeBullets        = 0;
}

void ProjectileGenerator_Gun::SetFireDelay(Event *ev)
{
    m_fFireDelay = ev->GetFloat(1);
}

void ProjectileGenerator_Gun::SetMeansOfDeath(Event *ev)
{
    m_iMeansOfDeath = MOD_string_to_int(ev->GetString(1));
}

void ProjectileGenerator_Gun::SetBulletThroughWood(Event *ev)
{
    m_fBulletThroughWood = ev->GetFloat(1);
}

void ProjectileGenerator_Gun::SetBulletThroughMetal(Event *ev)
{
    m_fBulletThroughMetal = ev->GetFloat(1);
}

void ProjectileGenerator_Gun::SetBulletKnockback(Event *ev)
{
    m_iBulletKnockback = ev->GetInteger(1);
}

void ProjectileGenerator_Gun::SetTracerSpeed(Event *ev)
{
    m_fTracerSpeed = ev->GetFloat(1);
}

void ProjectileGenerator_Gun::SetBulletLarge(Event *ev)
{
    m_iBulletLarge = ev->GetFloat(1);
}

void ProjectileGenerator_Gun::SetFakeBullets(Event *ev)
{
    m_bFakeBullets = ev->GetInteger(1);
}

void ProjectileGenerator_Gun::SetBulletCount(Event *ev)
{
    m_iBulletCount = ev->GetInteger(1);
}

void ProjectileGenerator_Gun::SetBulletDamage(Event *ev)
{
    m_fBulletDamage = ev->GetFloat(1);
}

void ProjectileGenerator_Gun::SetTracerFrequency(Event *ev)
{
    m_iTracerFrequency = ev->GetInteger(1);
}

void ProjectileGenerator_Gun::SetBulletSpread(Event *ev)
{
    m_vBulletSpread.x = ev->GetFloat(1);
    m_vBulletSpread.y = ev->GetFloat(2);
}

void ProjectileGenerator_Gun::SetBulletRange(Event *ev)
{
    m_fBulletRange = ev->GetFloat(1);
}

bool ProjectileGenerator_Gun::Attack(int count)
{
    Vector dir;
    Vector right, up;
    Vector muzzlePos;
    int    iTracerCount = 0;

    if (ShouldPlayFireSound() && m_fLastShotTime + m_fFireDelay > level.time) {
        return false;
    }

    GetMuzzlePos(muzzlePos);

    if (m_pCurrent) {
        dir = m_vTargetOrg - muzzlePos;
        dir.normalize();

        Vector newAngles;
        VectorToAngles(dir, newAngles);

        AngleVectors(newAngles, NULL, right, up);

        if (ShouldRotateYaw()) {
            newAngles.setYaw(newAngles.yaw() + 180);
        }

        if (ShouldRotateRoll()) {
            newAngles.setRoll(newAngles.roll() + 180);
        }

        setAngles(newAngles);
    } else {
        AngleVectors(angles, dir, right, up);
    }

    int iTracerFrequency = 0;

    m_iAttackCount += count;
    if (!m_iTracerFrequency) {
        m_iTracerFrequency = 4;
    }

    if (!(m_iAttackCount % m_iTracerFrequency)) {
        m_iAttackCount   = 0;
        iTracerFrequency = count;
    }

    if (ShouldPlayFireSound()) {
        Fire();
    }

    TryLaunchSound();

    if (m_bFakeBullets) {
        FakeBulletAttack(
            muzzlePos,
            muzzlePos,
            dir,
            right,
            up,
            m_fBulletRange,
            m_fBulletDamage,
            m_iBulletLarge,
            m_vBulletSpread,
            count,
            this,
            iTracerFrequency,
            &iTracerCount,
            m_fTracerSpeed
        );
    } else {
        BulletAttack(
            muzzlePos,
            muzzlePos,
            dir,
            right,
            up,
            m_fBulletRange,
            m_fBulletDamage,
            m_iBulletLarge,
            m_iBulletKnockback,
            0,
            m_iMeansOfDeath,
            m_vBulletSpread,
            count,
            this,
            iTracerFrequency,
            &iTracerCount,
            m_fBulletThroughWood,
            m_fBulletThroughMetal,
            NULL,
            m_fTracerSpeed
        );
    }

    return true;
}

bool ProjectileGenerator_Gun::TickWeaponAnim()
{
    return m_fLastShotTime + m_fFireDelay < level.time;
}

void ProjectileGenerator_Gun::TickCycle(Event *ev)
{
    ProjectileGenerator::TickCycle(ev);
    TickWeaponAnim();
}

void ProjectileGenerator_Gun::Archive(Archiver& arc)
{
    ProjectileGenerator::Archive(arc);

    arc.ArchiveFloat(&m_fBulletRange);
    arc.ArchiveFloat(&m_fBulletDamage);
    arc.ArchiveInteger(&m_iBulletCount);
    arc.ArchiveVector(&m_vBulletSpread);
    arc.ArchiveInteger(&m_iTracerFrequency);
    arc.ArchiveInteger(&m_iBulletLarge);
    arc.ArchiveInteger(&m_bFakeBullets);
    arc.ArchiveInteger(&m_iMeansOfDeath);
    arc.ArchiveFloat(&m_fBulletThroughWood);
    arc.ArchiveFloat(&m_fBulletThroughMetal);
    arc.ArchiveFloat(&m_iBulletKnockback);
    arc.ArchiveInteger(&m_iAttackCount);
    arc.ArchiveFloat(&m_fFireDelay);
    arc.ArchiveFloat(&m_fTracerSpeed);
}

/*QUAKED ProjectileGenerator_Heavy (1.0 0.0 1.0) (-12 -12 -12) (12 12 12)  LaunchSound x TargetRandom StartOn FlipYaw FlipRoll TargetPlayer HideModel

  Allows you to launch projectiles without the need for actual AI.  Useful for faking large battles.
  Here are the params:

  ID <nIdNum>
  Model		 <sModelName>			(Defaults to bazooka weapon)
  Projectile <sModelName>			(Defaults to bazooka projectile)
  MinDuration <nMinDur=1>           (Min burst cycle duration)
  MaxDuration <nMaxDur=3>           (Max burst cycle duration)
  Cycles <nCycles=0>                (0=infinite)
  MinNumShots <nMinNumShots=1>      (Minimum # of shots per cycle)
  MaxNumShots <nMaxNumShots=1>      (Maximum # of shots per cycle)

  MinDelay <nDelay=2>               (min amount of time between cycles)
  MaxDelay <nDelay=10>              (max amount of time between cycles)
  Accuracy <nAccuracy=10>           (accuracy measured in feet. 0-25 feet range)

  FireOnStartUp						(defaults to 1.  This flag tells the PG to fire immediately when turned on.  Then do its delay from then on.)

  IsDonut	<nIsDonut=1>			(indicates to use donut to calculate impact location)
  Arc		<nArc=#degrees)			(how many degrees does the arc have)
  minDonut <nMinDonut=15>			(mindistance from player)
  maxDonut <nMaxDonut=30>			(maxdistance from player)
  launchsound <sSoundName>          (Defaults to no sound)

  Here are some useful script commands:

  $<generatorName> TurnOn
  $<generatorName> TurnOff
  $<generatorName> gettargetentity   ( return the entity that is targeted )
  $<generatorName> isturnedon        ( is the projectil generator turned on? )

******************************************************************************/

CLASS_DECLARATION(ProjectileGenerator, ProjectileGenerator_Heavy, "ProjectileGenerator_Heavy") {
    {&EV_Weapon_SetProjectile, &ProjectileGenerator_Heavy::SetProjectileModel},
    {NULL,                     NULL                                          }
};

ProjectileGenerator_Heavy::ProjectileGenerator_Heavy()
{
    if (LoadingSavegame) {
        return;
    }

    setModel("models/weapons/bazooka.tik");
    m_sProjectileModel = "models/projectiles/bazookashell.tik";
}

void ProjectileGenerator_Heavy::SetProjectileModel(Event *ev)
{
    m_sProjectileModel = ev->GetString(1);
}

bool ProjectileGenerator_Heavy::Attack(int count)
{
    static int NukePlayer = 0;
    Vector     dir(0, 0, 1);
    float      speed = 500;

    Vector muzzlePos = origin;
    GetMuzzlePos(muzzlePos);

    if (m_pCurrent) {
        m_vTargetOrg = GetTargetPos(m_pCurrent);

        if (m_bIsDonut) {
            Entity *playerTarget = G_FindTarget(this, "player");

            if (playerTarget) {
                if (playerTarget->takedamage) {
                    if (!Q_stricmp(targetname, "artillerykiller")) {
                        NukePlayer++;
                    }
                } else {
                    NukePlayer = 0;
                }

                if (NukePlayer > 2) {
                    m_vTargetOrg = playerTarget->origin;
                    muzzlePos    = playerTarget->origin;
                    muzzlePos[2] += 256;
                    NukePlayer = 0;
                } else {
                    muzzlePos = m_vTargetOrg;
                    muzzlePos[2] += random() * 1000.0 + 500.0;
                }
            } else {
                muzzlePos = m_vTargetOrg;
                muzzlePos[2] += 2000.0;
            }
        }

        dir = m_vTargetOrg - muzzlePos;
        dir.normalize();

        Vector newAngles;
        VectorToAngles(dir, newAngles);

        if (ShouldRotateYaw()) {
            newAngles.setYaw(newAngles.yaw() + 180);
        }

        if (ShouldRotateRoll()) {
            newAngles.setRoll(newAngles.roll() + 180);
        }

        setAngles(newAngles);
    } else {
        AngleVectors(angles, dir, NULL, NULL);
    }

    TryLaunchSound();
    // spawn the projectile
    HeavyAttack(muzzlePos, dir, m_sProjectileModel, speed, this, NULL);

    return true;
}

void ProjectileGenerator_Heavy::Archive(Archiver& arc)
{
    ProjectileGenerator::Archive(arc);

    arc.ArchiveString(&m_sProjectileModel);
}

/*QUAKED ThrobbingBox_explosive (1.0 0.0 1.0) (-12 -12 -12) (12 12 12) ReplaceModel

  Explosive throbbing box.  Connect to the object you want to make explode.

  ReplaceModel - check this if you want the destroy model to replace the model instead of doing
                 a model switch.


  Parameters:
		Model - name of pulse model (default=items/pulse_explosive.tik")
		UsedModel - name of non-pulse version (default=items/explosive.tik")
		ExplosionSound - name of explosion sound (default=explode_flak88)
		ActivateSound - name of activation sound (default=plantbomb)
		TickSound - name of 'ticking' sound (default=bombtick)
		Damage - amount of damage to do (default=300)
		radius - radius to do the damage (default=300)
		stopwatchduration - how long should the stopwatch go (default=5 seconds)
		DestroyModel - name of model to switch target to.
		setthread - name of thread to call when the explosion happens...
		setusethread - name of thread to call when explosive was placed.
		explosionpos - vector of the position the explosion should occur

  Note:
	* Set stopwatchduration to 0 to prevent the explosion and stopwatch from happening.
	* You may then send the Explode event to this object to make the explosion happen.

 ---------------------------------------------------------------------------------------------

*/

Event EV_DestroyModel
(
    "DestroyModel",
    EV_DEFAULT,
    "s",
    "DestroyModel",
    NULL,
    EV_NORMAL
);
Event EV_UsedModel
(
    "UsedModel",
    EV_DEFAULT,
    "s",
    "UsedModel",
    NULL,
    EV_NORMAL
);
Event EV_ExplosionSound
(
    "ExplosionSound",
    EV_DEFAULT,
    "s",
    "ExplosionSound",
    NULL,
    EV_NORMAL
);
Event EV_ActivateSound
(
    "ActivateSound",
    EV_DEFAULT,
    "s",
    "ActivateSound",
    NULL,
    EV_NORMAL
);
Event EV_TickSound
(
    "TickSound",
    EV_DEFAULT,
    "s",
    "TickSound",
    NULL,
    EV_NORMAL
);
Event EV_SetDamage
(
    "SetDamage",
    EV_DEFAULT,
    "f",
    "Damage",
    NULL,
    EV_NORMAL
);
Event EV_Radius
(
    "Radius",
    EV_DEFAULT,
    "f",
    "Radius",
    NULL,
    EV_NORMAL
);
Event EV_StopWatchDuration
(
    "StopWatchDuration",
    EV_DEFAULT,
    "f",
    "StopWatchDuration",
    NULL,
    EV_NORMAL
);
Event EV_SetThread
(
    "setthread",
    EV_DEFAULT,
    "s",
    "thread",
    "Set the thread to execute when this object is used",
    EV_NORMAL
);
Event EV_SetUseThread
(
    "setusethread",
    EV_DEFAULT,
    "s",
    "thread",
    "Set the thread to execute when this object is used",
    EV_NORMAL
);
Event EV_DoExplosion
(
    "Explode",
    EV_DEFAULT,
    NULL,
    NULL,
    "Cause the explosion to happen.",
    EV_NORMAL
);
Event EV_SetTriggered
(
    "triggered",
    EV_DEFAULT,
    "i",
    "0 or 1",
    "Set the triggered status",
    EV_NORMAL
);
Event EV_SetExplosionEffect
(
    "explosioneffect",
    EV_DEFAULT,
    "s",
    "effect model",
    "Set the explosion effect model",
    EV_NORMAL
);
Event EV_SetExplosionOffset
(
    "explosionoffset",
    EV_DEFAULT,
    "v",
    "offset vector",
    "Set the explosion offset",
    EV_NORMAL
);
Event EV_BlowUp
(
    "BlowUp",
    EV_DEFAULT,
    NULL,
    NULL,
    NULL,
    EV_NORMAL
);

CLASS_DECLARATION(Animate, ThrobbingBox_Explosive, "ThrobbingBox_Explosive") {
    {&EV_Use,                &ThrobbingBox_Explosive::OnUse               },
    {&EV_BlowUp,             &ThrobbingBox_Explosive::OnBlowUp            },
    {&EV_DestroyModel,       &ThrobbingBox_Explosive::SetDestroyModel     },
    {&EV_UsedModel,          &ThrobbingBox_Explosive::UsedModel           },
    {&EV_ExplosionSound,     &ThrobbingBox_Explosive::ExplosionSound      },
    {&EV_ActivateSound,      &ThrobbingBox_Explosive::ActivateSound       },
    {&EV_TickSound,          &ThrobbingBox_Explosive::TickSound           },
    {&EV_SetDamage,          &ThrobbingBox_Explosive::SetDamage           },
    {&EV_Radius,             &ThrobbingBox_Explosive::SetRadius           },
    {&EV_StopWatchDuration,  &ThrobbingBox_Explosive::SetStopWatchDuration},
    {&EV_SetThread,          &ThrobbingBox_Explosive::SetThread           },
    {&EV_SetUseThread,       &ThrobbingBox_Explosive::SetUseThread        },
    {&EV_DoExplosion,        &ThrobbingBox_Explosive::DoExplosion         },
    {&EV_SetTriggered,       &ThrobbingBox_Explosive::SetTriggered        },
    {&EV_SetExplosionEffect, &ThrobbingBox_Explosive::SetExplosionEffect  },
    {NULL,                   NULL                                         }
};

ThrobbingBox_Explosive::ThrobbingBox_Explosive()
{
    if (LoadingSavegame) {
        return;
    }

    setModel("items\\\\pulse_explosive.tik");
    m_sUsedModel     = "items/explosive.tik";
    m_sSound         = "explose_flak88";
    m_sActivateSound = "plantbomb";
    m_sTickSound     = "bombtick";

    m_bUsed              = false;
    m_fExplosionDamage   = 300;
    m_fRadius            = 300;
    m_fStopwatchDuration = 5;

    m_sEffect = "emitters/mortar_higgins.tik";
    m_vOffset = Vector(0, 0, 0);
}

bool ThrobbingBox_Explosive::ShouldDoExplosion()
{
    return m_fStopwatchDuration;
}

void ThrobbingBox_Explosive::SetExplosionOffset(Event *ev)
{
    m_vOffset = ev->GetVector(1);
}

void ThrobbingBox_Explosive::SetExplosionEffect(Event *ev)
{
    m_sEffect = ev->GetString(1);
}

void ThrobbingBox_Explosive::SetTriggered(Event *ev)
{
    m_bUsed = ev->GetInteger(1) ? true : false;
}

void ThrobbingBox_Explosive::DoExplosion(Event *ev)
{
    SpawnArgs    sp;
    ScriptModel *spawned;

    sp.setArg("model", m_sEffect);
    sp.setArg("origin", va("%f %f %f", origin.x, origin.y, origin.z));
    sp.setArg("classname", "ScriptModel");

    if (target.length()) {
        SimpleEntity *targetEnt = G_FindTarget(NULL, target.c_str());
        if (targetEnt) {
            sp.setArg("origin", va("%f %f %f", targetEnt->origin.x, targetEnt->origin.y, targetEnt->origin.z));
        }
    }

    spawned = static_cast<ScriptModel *>(sp.Spawn());
    if (spawned) {
        spawned->NewAnim("start");
        spawned->setSolidType(SOLID_NOT);
    }

    RadiusDamage(origin + Vector(0, 0, 128), this, this, m_fExplosionDamage, this, MOD_EXPLOSION, m_fRadius);

    Sound(m_sSound, CHAN_BODY);

    m_thread.Execute(this);
    PostEvent(EV_Remove, 0);
}

void ThrobbingBox_Explosive::SetUseThread(Event *ev)
{
    if (ev->IsFromScript()) {
        m_useThread.SetThread(ev->GetValue(1));
    } else {
        m_useThread.Set(ev->GetString(1));
    }
}

void ThrobbingBox_Explosive::SetThread(Event *ev)
{
    if (ev->IsFromScript()) {
        m_thread.SetThread(ev->GetValue(1));
    } else {
        m_thread.Set(ev->GetString(1));
    }
}

void ThrobbingBox_Explosive::SetStopWatchDuration(Event *ev)
{
    m_fStopwatchDuration = ev->GetFloat(1);
}

void ThrobbingBox_Explosive::SetRadius(Event *ev)
{
    m_fRadius = ev->GetFloat(1);
}

void ThrobbingBox_Explosive::SetDamage(Event *ev)
{
    m_fExplosionDamage = ev->GetFloat(1);
}

void ThrobbingBox_Explosive::TickSound(Event *ev)
{
    m_sTickSound = ev->GetString(1);
}

void ThrobbingBox_Explosive::ActivateSound(Event *ev)
{
    m_sActivateSound = ev->GetString(1);
}

void ThrobbingBox_Explosive::ExplosionSound(Event *ev)
{
    m_sSound = ev->GetString(1);
}

void ThrobbingBox_Explosive::UsedModel(Event *ev)
{
    m_sUsedModel = ev->GetString(1);
}

void ThrobbingBox_Explosive::SetDestroyModel(Event *ev)
{
    m_sDestroyedModel = ev->GetString(1);
}

void ThrobbingBox_Explosive::OnBlowUp(Event *ev)
{
    Player      *player;
    Entity      *targetEnt;
    ScriptModel *mdl;

    StopLoopSound();
    setSolidType(SOLID_NOT);

    player = static_cast<Player *>(G_FindTarget(this, "player"));
    if (player) {
        // clear the player's stopwatch
        player->SetStopwatch(0);
    }

    if (target.length() && (targetEnt = static_cast<Entity *>(G_FindTarget(NULL, target.c_str())))
        && m_sDestroyedModel.length()) {
        if (spawnflags & TBE_SPAWNFLAG_DESTROYED_MODEL) {
            SpawnArgs sp;

            sp.setArg("model", m_sDestroyedModel.c_str());
            sp.setArg("origin", va("%f %f %f", targetEnt->origin.x, targetEnt->origin.y, targetEnt->origin.z));
            sp.setArg("angles", va("%f %f %f", targetEnt->angles.x, targetEnt->angles.y, targetEnt->angles.z));
            sp.setArg("classname", "ScriptModel");

            mdl = static_cast<ScriptModel *>(sp.Spawn());
            if (mdl) {
                mdl->NewAnim("start");
                mdl->setSolidType(static_cast<solid_t>(targetEnt->getSolidType()));
            }

            if (targetEnt->IsSubclassOfVehicle()) {
                Vehicle *veh = static_cast<Vehicle *>(targetEnt);
                Entity  *ent;
                int      i;

                // remove all turrets
                for (i = 0; i < MAX_TURRETS; i++) {
                    ent = veh->QueryTurretSlotEntity(i);
                    if (ent) {
                        ent->PostEvent(EV_Remove, 0);
                    }
                }

                for (i = 0; i < MAX_PASSENGERS; i++) {
                    ent = veh->QueryPassengerSlotEntity(i);
                    if (ent) {
                        ent->Damage(this, this, ent->health * 2.f, vec_zero, vec_zero, vec_zero, 50, 0, MOD_VEHICLE);
                    }
                }

                ent = veh->QueryDriverSlotEntity(0);
                if (ent) {
                    ent->Damage(this, this, ent->health * 2.f, vec_zero, vec_zero, vec_zero, 50, 0, MOD_VEHICLE);
                }
            }

            targetEnt->PostEvent(EV_Remove, 0);
        } else {
            targetEnt->setModel(m_sDestroyedModel.c_str());
        }
    }

    if (ShouldDoExplosion()) {
        DoExplosion(NULL);
    }
}

void ThrobbingBox_Explosive::OnUse(Event *ev)
{
    if (m_bUsed) {
        // don't use more than once
        return;
    }

    if (edict->s.renderfx & RF_DONTDRAW) {
        // ignore if invisible
        return;
    }

    m_bUsed = true;

    setModel(m_sUsedModel);

    if (m_fStopwatchDuration > 0) {
        Player *player = static_cast<Player *>(G_FindTarget(this, "player"));
        player->SetStopwatch(m_fStopwatchDuration, SWT_FUSE);
        LoopSound(m_sTickSound);
    }

    // Play the activation sound
    Sound(m_sActivateSound, CHAN_BODY);

    Unregister(STRING_TRIGGER);
    m_useThread.Execute(this);

    if (m_fStopwatchDuration > 0) {
        PostEvent(EV_BlowUp, m_fStopwatchDuration);
    }
}

void ThrobbingBox_Explosive::Archive(Archiver& arc)
{
    Animate::Archive(arc);

    arc.ArchiveString(&m_sUsedModel);
    arc.ArchiveString(&m_sSound);
    arc.ArchiveBool(&m_bUsed);
    arc.ArchiveFloat(&m_fExplosionDamage);
    arc.ArchiveFloat(&m_fRadius);
    arc.ArchiveFloat(&m_fStopwatchDuration);
    arc.ArchiveString(&m_sActivateSound);
    arc.ArchiveString(&m_sTickSound);
    arc.ArchiveString(&m_sDestroyedModel);
    arc.ArchiveString(&m_sEffect);
    arc.ArchiveVector(&m_vOffset);
    m_thread.Archive(arc);
    m_useThread.Archive(arc);
}

/*QUAKED ThrobbingBox_ExplodePlayerFlak88 (1.0 0.0 1.0) (-12 -12 -12) (12 12 12)

  Explosive throbbing box.  Connect to the object you want to make explode.


  Parameters:
		Model - name of pulse model (default=items/pulse_explosive.tik")
		UsedModel - name of non-pulse version (default=items/explosive.tik")
		ExplosionSound - name of explosion sound (default=explode_flak88)
		ActivateSound - name of activation sound (default=plantbomb)
		TickSound - name of 'ticking' sound (default=bombtick)
		Damage - amount of damage to do (default=300)
		radius - radius to do the damage (default=300)
		stopwatchduration - how long should the stopwatch go (default=5 seconds)
		setthread - name of thread to call when the explosion happens...

 ---------------------------------------------------------------------------------------------
*/

CLASS_DECLARATION(ThrobbingBox_Explosive, ThrobbingBox_ExplodePlayerFlak88, "ThrobbingBox_ExplodePlayerFlak88") {
    {NULL, NULL}
};

/*QUAKED ThrobbingBox_ExplodeFlak88 (1.0 0.0 1.0) (-12 -12 -12) (12 12 12)

  Explosive throbbing box.  Connect to the object you want to make explode.

  Parameters:
		Model - name of pulse model (default=items/pulse_explosive.tik")
		UsedModel - name of non-pulse version (default=items/explosive.tik")
		ExplosionSound - name of explosion sound (default=explode_flak88)
		ActivateSound - name of activation sound (default=plantbomb)
		TickSound - name of 'ticking' sound (default=bombtick)
		Damage - amount of damage to do (default=300)
		radius - radius to do the damage (default=300)
		stopwatchduration - how long should the stopwatch go (default=5 seconds)
		setthread - name of thread to call when the explosion happens...

 ---------------------------------------------------------------------------------------------
*/

CLASS_DECLARATION(ThrobbingBox_Explosive, ThrobbingBox_ExplodeFlak88, "ThrobbingBox_ExplodeFlak88") {
    {NULL, NULL}
};

/*QUAKED ThrobbingBox_ExplodeNebelwerfer (1.0 0.0 1.0) (-12 -12 -12) (12 12 12) ReplaceModel

  Explosive throbbing box.  Connect to the object you want to make explode.

  Parameters:
		Model - name of pulse model (default=items/pulse_explosive.tik")
		UsedModel - name of non-pulse version (default=items/explosive.tik")
		ExplosionSound - name of explosion sound (default=explode_flak88)
		ActivateSound - name of activation sound (default=plantbomb)
		TickSound - name of 'ticking' sound (default=bombtick)
		Damage - amount of damage to do (default=300)
		radius - radius to do the damage (default=300)
		stopwatchduration - how long should the stopwatch go (default=5 seconds)
		setthread - name of thread to call when the explosion happens...

 ---------------------------------------------------------------------------------------------
*/

CLASS_DECLARATION(ThrobbingBox_Explosive, ThrobbingBox_ExplodeNebelwerfer, "ThrobbingBox_ExplodeNebelwerfer") {
    {NULL, NULL}
};

/*QUAKED ThrobbingBox_ExplodePlayerNebelwerfer (1.0 0.0 1.0) (-12 -12 -12) (12 12 12) ReplaceModel

  Explosive throbbing box.  Connect to the object you want to make explode.

  Parameters:
		Model - name of pulse model (default=items/pulse_explosive.tik")
		UsedModel - name of non-pulse version (default=items/explosive.tik")
		ExplosionSound - name of explosion sound (default=explode_flak88)
		ActivateSound - name of activation sound (default=plantbomb)
		TickSound - name of 'ticking' sound (default=bombtick)
		Damage - amount of damage to do (default=300)
		radius - radius to do the damage (default=300)
		stopwatchduration - how long should the stopwatch go (default=5 seconds)
		setthread - name of thread to call when the explosion happens...

 ---------------------------------------------------------------------------------------------
*/

CLASS_DECLARATION(ThrobbingBox_Explosive, ThrobbingBox_ExplodePlayerNebelwerfer, "ThrobbingBox_ExplodeNebelwerfer") {
    {NULL, NULL}
};

/*QUAKED ThrobbingBox_Stickybomb (1.0 0.0 1.0) (-12 -12 -12) (12 12 12) ReplaceModel WetFuse

  Stickybomb throbbing box.  Connect to the object you want to make explode.
  When WetFuse is selected, the bomb will fizzle out and can only be destroyed by shooting it.

  Parameters:
		ExplosionSound - name of explosion sound (default=explode_flak88)
		Damage - amount of damage to do (default=300)
		radius - radius to do the damage (default=300)
		stopwatchduration - how long should the fuse go (default=5 seconds)
		setthread - name of thread to call when the explosion happens...
		DestroyModel - name of model to switch target to.
		setusethread - name of thread to call when stickybomb is placed.

 ---------------------------------------------------------------------------------------------
*/

Event EV_StickyBombWet("stickybombwet", EV_DEFAULT, NULL, NULL, NULL);

CLASS_DECLARATION(ThrobbingBox_Explosive, ThrobbingBox_Stickybomb, "ThrobbingBox_ExplodeNebelwerfer") {
    {&EV_Damage,        &ThrobbingBox_Stickybomb::OnBlowUp       },
    {&EV_Use,           &ThrobbingBox_Stickybomb::OnStickyBombUse},
    {&EV_StickyBombWet, &ThrobbingBox_Stickybomb::OnStickyBombWet},
    {NULL,              NULL                                     }
};

void ThrobbingBox_Stickybomb::OnStickyBombWet(Event *ev)
{
    Player *player;

    CancelEventsOfType(EV_BlowUp);
    NewAnim("idle_fuse_wet");
    StopLoopSound();
    Sound("stickybomb_fuse_out", CHAN_BODY);

    player = static_cast<Player *>(G_FindTarget(this, "player"));
    player->SetStopwatch(0, SWT_NORMAL);
    player->SetStopwatch(m_fStopwatchDuration - (level.svsFloatTime - m_fStopwatchStartTime), SWT_FUSE);
}

void ThrobbingBox_Stickybomb::OnStickyBombUse(Event *ev)
{
    if (m_bUsed) {
        return;
    }

    setSolidType(SOLID_BBOX);
    setContents(CONTENTS_WEAPONCLIP);
    takedamage = DAMAGE_YES;
    m_bUsed    = true;

    setModel(m_sUsedModel);

    if (m_fStopwatchDuration > 0) {
        Player *player = static_cast<Player *>(G_FindTarget(this, "player"));
        player->SetStopwatch(m_fStopwatchDuration, SWT_FUSE);
        LoopSound(m_sTickSound);
        m_fStopwatchStartTime = level.svsFloatTime;
    }

    // Play the activation sound
    Sound(m_sActivateSound, CHAN_BODY);

    Unregister(STRING_TRIGGER);
    m_useThread.Execute(this);
    PostEvent(EV_BlowUp, m_fStopwatchDuration);

    if (spawnflags & TBE_SPAWNFLAG_MAKE_WET) {
        PostEvent(EV_StickyBombWet, G_Random(m_fStopwatchDuration / 2.0) + m_fStopwatchDuration / 4.0);
    }

    NewAnim("idle_fuse_lit");
}

void ThrobbingBox_Stickybomb::Archive(Archiver& arc)
{
    ThrobbingBox_Explosive::Archive(arc);

    arc.ArchiveFloat(&m_fStopwatchStartTime);
}

/*QUAKED func_objective (1.0 0.0 1.0) (-12 -12 -12) (12 12 12) StartOn

  object used to simplify placing objectives.  You may optionally connect this object to the
  target you want to use for the compass position....

  Checkboxes:
	StartOn - check this if you want this objective to be displayed when the level first loads

  Parameters:

  ObjectiveNbr <nObjNbr=0>
  Text <sText>

  Events:
	TurnOn
	TurnOff
	Complete
	Text <sNewText>
	SetCurrent

//---------------------------------------------------------------------------------------------
*/

Event EV_Complete
(
    "Complete",
    EV_DEFAULT,
    NULL,
    NULL,
    "Complete this objective.",
    EV_NORMAL
);

Event EV_SetCurrent
(
    "SetCurrent",
    EV_DEFAULT,
    NULL,
    NULL,
    "Set this objective as current.",
    EV_NORMAL
);

Event EV_SetText
(
    "Text",
    EV_DEFAULT,
    "s",
    "text",
    "Set current text.",
    EV_NORMAL
);

Event EV_SetObjectiveNbr
(
    "ObjectiveNbr",
    EV_DEFAULT,
    "i",
    "index",
    "Sets the objective number.",
    EV_NORMAL
);

CLASS_DECLARATION(Entity, Objective, "func_objective") {
    {&EV_TurnOn,          &Objective::TurnOn         },
    {&EV_TurnOff,         &Objective::TurnOff        },
    {&EV_Complete,        &Objective::Complete       },
    {&EV_SetCurrent,      &Objective::SetCurrent     },
    {&EV_SetText,         &Objective::SetText        },
    {&EV_SetObjectiveNbr, &Objective::SetObjectiveNbr},
    {&EV_Activate,        &Objective::SetCurrent     },
    {NULL,                NULL                       }
};

Objective::Objective()
{
    m_iObjectiveIndex = 0;

    if (LoadingSavegame) {
        return;
    }

    if (spawnflags & OBJECTIVE_SPAWNFLAG_TURN_ON) {
        PostEvent(EV_TurnOn, 0.1f);
    }
}

void Objective::Archive(Archiver& arc)
{
    Entity::Archive(arc);

    arc.ArchiveString(&m_sText);
    arc.ArchiveInteger(&m_iObjectiveIndex);
}

void Objective::TurnOn(Event *ev)
{
    ScriptThread::AddObjective(m_iObjectiveIndex, OBJ_STATUS_CURRENT, m_sText, GetOrigin());
}

void Objective::TurnOff(Event *ev)
{
    ScriptThread::AddObjective(m_iObjectiveIndex, OBJ_STATUS_HIDDEN, m_sText, GetOrigin());
}

void Objective::Complete(Event *ev)
{
    ScriptThread::AddObjective(m_iObjectiveIndex, OBJ_STATUS_COMPLETED, m_sText, GetOrigin());
}

void Objective::SetCurrent(Event *ev)
{
    ScriptThread::SetCurrentObjective(m_iObjectiveIndex, TEAM_NONE);
}

Vector Objective::GetOrigin() const
{
    if (target.c_str()) {
        SimpleEntity *targetEnt;

        targetEnt = G_FindTarget(NULL, target.c_str());

        if (targetEnt) {
            return targetEnt->origin;
        }
    }

    return origin;
}

void Objective::SetObjectiveNbr(Event *ev)
{
    m_iObjectiveIndex = ev->GetInteger(1);
}

void Objective::SetText(Event *ev)
{
    m_sText = ev->GetString(1);
}

int Objective::GetObjectiveIndex() const
{
    return m_iObjectiveIndex;
}

/*QUAKED func_fencepost (0 1 0) ?
  fencepost that can be blown up
  "health" how much health the fence post should have (default 50)
//---------------------------------------------------------------------------------------------
*/

CLASS_DECLARATION(Entity, FencePost, "func_fencepost") {
    {&EV_Killed, NULL},
    {NULL,       NULL}
};

Event EV_SetEnemyName
(
    "enemyname",
    EV_DEFAULT,
    "s",
    "enemyname",
    "",
    EV_NORMAL
);
Event EV_SetEnemyName2
(
    "enemyname",
    EV_DEFAULT,
    "s",
    "enemyname",
    "",
    EV_GETTER
);
Event EV_GetEnemyName
(
    "enemyname",
    EV_DEFAULT,
    "s",
    "enemyname",
    "",
    EV_SETTER
);
Event EV_Sentient_GetDontDropWeapons
(
    "dontdropweapons",
    EV_DEFAULT,
    NULL,
    NULL,
    "dontdropweapons getter",
    EV_GETTER
);
Event EV_SetDontDropHealth
(
    "dontdrophealth",
    EV_DEFAULT,
    NULL,
    NULL,
    "dontdrophealth setter",
    EV_NORMAL
);
Event EV_GetDontDropHealth
(
    "dontdrophealth",
    EV_DEFAULT,
    NULL,
    NULL,
    "dontdrophealth getter",
    EV_GETTER
);

CLASS_DECLARATION(SimpleArchivedEntity, AISpawnPoint, "info_aispawnpoint") {
    {&EV_Model,                       &AISpawnPoint::SetModel             },
    {&EV_GetModel,                    &AISpawnPoint::GetModel             },
    {&EV_SetHealth2,                  &AISpawnPoint::SetHealth            },
    {&EV_Entity_GetHealth,            &AISpawnPoint::GetHealth            },
    {&EV_SetEnemyName,                &AISpawnPoint::SetEnemyName         },
    {&EV_SetEnemyName2,               &AISpawnPoint::SetEnemyName         },
    {&EV_GetEnemyName,                &AISpawnPoint::GetEnemyName         },
    {&EV_Actor_SetAccuracy,           &AISpawnPoint::SetAccuracy          },
    {&EV_Actor_GetAccuracy,           &AISpawnPoint::GetAccuracy          },
    {&EV_Actor_SetAmmoGrenade,        &AISpawnPoint::SetAmmoGrenade       },
    {&EV_Actor_GetAmmoGrenade,        &AISpawnPoint::GetAmmoGrenade       },
    {&EV_Actor_SetBalconyHeight,      &AISpawnPoint::SetBalconyHeight     },
    {&EV_Actor_GetBalconyHeight,      &AISpawnPoint::GetBalconyHeight     },
    {&EV_Actor_SetDisguiseLevel,      &AISpawnPoint::SetDisguiseLevel     },
    {&EV_Actor_GetDisguiseLevel,      &AISpawnPoint::GetDisguiseLevel     },
    {&EV_Actor_SetDisguisePeriod2,    &AISpawnPoint::SetDisguisePeriod    },
    {&EV_Actor_GetDisguisePeriod,     &AISpawnPoint::GetDisguisePeriod    },
    {&EV_Actor_SetDisguiseRange2,     &AISpawnPoint::SetDisguiseRange     },
    {&EV_Actor_GetDisguiseRange,      &AISpawnPoint::GetDisguiseRange     },
    {&EV_Actor_SetEnemyShareRange2,   &AISpawnPoint::SetEnemyShareRange   },
    {&EV_Actor_GetEnemyShareRange,    &AISpawnPoint::GetEnemyShareRange   },
    {&EV_Actor_GetFixedLeash,         &AISpawnPoint::GetFixedLeash        },
    {&EV_Actor_SetFixedLeash2,        &AISpawnPoint::SetFixedLeash        },
    {&EV_Actor_SetGrenadeAwareness,   &AISpawnPoint::SetGrenadeAwareness  },
    {&EV_Actor_GetGrenadeAwareness,   &AISpawnPoint::GetGrenadeAwareness  },
    {&EV_Actor_SetGun,                &AISpawnPoint::SetGun               },
    {&EV_Actor_GetGun,                &AISpawnPoint::GetGun               },
    {&EV_Actor_GetMaxNoticeTimeScale, &AISpawnPoint::GetMaxNoticeTimeScale},
    {&EV_Actor_SetMaxNoticeTimeScale, &AISpawnPoint::SetMaxNoticeTimeScale},
    {&EV_Actor_GetSoundAwareness,     &AISpawnPoint::GetSoundAwareness    },
    {&EV_Actor_SetSoundAwareness,     &AISpawnPoint::SetSoundAwareness    },
    {&EV_Actor_SetTypeAttack,         &AISpawnPoint::SetTypeAttack        },
    {&EV_Actor_GetTypeAttack,         &AISpawnPoint::GetTypeAttack        },
    {&EV_Actor_SetTypeDisguise,       &AISpawnPoint::SetTypeDisguise      },
    {&EV_Actor_GetTypeDisguise,       &AISpawnPoint::GetTypeDisguise      },
    {&EV_Actor_SetTypeGrenade,        &AISpawnPoint::SetTypeGrenade       },
    {&EV_Actor_GetTypeGrenade,        &AISpawnPoint::GetTypeGrenade       },
    {&EV_Actor_SetTypeIdle,           &AISpawnPoint::SetTypeIdle          },
    {&EV_Actor_GetTypeIdle,           &AISpawnPoint::GetTypeIdle          },
    {&EV_Actor_SetPatrolWaitTrigger,  &AISpawnPoint::SetPatrolWaitTrigger },
    {&EV_Actor_GetPatrolWaitTrigger,  &AISpawnPoint::GetPatrolWaitTrigger },
    {&EV_Actor_SetHearing,            &AISpawnPoint::SetHearing           },
    {&EV_Actor_GetHearing,            &AISpawnPoint::GetHearing           },
    {&EV_Actor_GetSight,              &AISpawnPoint::GetSight             },
    {&EV_Actor_SetSight2,             &AISpawnPoint::SetSight             },
    {&EV_Actor_SetFov,                &AISpawnPoint::SetFov               },
    {&EV_Actor_GetFov,                &AISpawnPoint::GetFov               },
    {&EV_Actor_SetLeash2,             &AISpawnPoint::SetLeash             },
    {&EV_Actor_GetLeash,              &AISpawnPoint::GetLeash             },
    {&EV_Actor_SetMinDistance,        &AISpawnPoint::SetMinDistance       },
    {&EV_Actor_GetMinDistance,        &AISpawnPoint::GetMinDistance       },
    {&EV_Actor_SetMaxDistance,        &AISpawnPoint::SetMaxDistance       },
    {&EV_Actor_GetMaxDistance,        &AISpawnPoint::GetMaxDistance       },
    {&EV_Actor_SetInterval2,          &AISpawnPoint::SetInterval          },
    {&EV_Actor_GetInterval,           &AISpawnPoint::GetInterval          },
    {&EV_Sentient_DontDropWeapons,    &AISpawnPoint::SetDontDropWeapons   },
    {&EV_Sentient_GetDontDropWeapons, &AISpawnPoint::GetDontDropWeapons   },
    {&EV_SetDontDropHealth,           &AISpawnPoint::SetDontDropHealth    },
    {&EV_GetDontDropHealth,           &AISpawnPoint::GetDontDropHealth    },
    {&EV_Actor_GetFavoriteEnemy,      &AISpawnPoint::GetFavoriteEnemy     },
    {&EV_Actor_SetFavoriteEnemy2,     &AISpawnPoint::SetFavoriteEnemy     },
    {&EV_Actor_SetNoSurprise2,        &AISpawnPoint::SetNoSurprise        },
    {&EV_Actor_GetNoSurprise,         &AISpawnPoint::GetNoSurprise        },
    {&EV_Actor_SetPatrolPath,         &AISpawnPoint::SetPatrolPath        },
    {&EV_Actor_GetPatrolPath,         &AISpawnPoint::GetPatrolPath        },
    {&EV_Actor_SetTurret,             &AISpawnPoint::SetTurret            },
    {&EV_Actor_GetTurret,             &AISpawnPoint::GetTurret            },
    {&EV_Actor_SetAlarmNode,          &AISpawnPoint::SetAlarmNode         },
    {&EV_Actor_GetAlarmNode,          &AISpawnPoint::GetAlarmNode         },
    {&EV_Actor_SetWeapon,             &AISpawnPoint::SetWeapon            },
    {&EV_Actor_GetWeapon,             &AISpawnPoint::GetWeapon            },
    {&EV_SetTarget,                   &AISpawnPoint::SetTarget            },
    {&EV_GetTarget,                   &AISpawnPoint::GetTarget            },
    {&EV_Actor_SetVoiceType,          &AISpawnPoint::SetVoiceType         },
    {&EV_Actor_GetVoiceType,          &AISpawnPoint::GetVoiceType         },
    {&EV_Sentient_ForceDropWeapon,    &AISpawnPoint::SetForceDropWeapon   },
    {&EV_Sentient_GetForceDropWeapon, &AISpawnPoint::GetForceDropWeapon   },
    {&EV_Sentient_ForceDropHealth,    &AISpawnPoint::SetForceDropHealth   },
    {&EV_Sentient_GetForceDropHealth, &AISpawnPoint::GetForceDropHealth   },
    {NULL,                            NULL                                }
};

AISpawnPoint::AISpawnPoint()
{
    m_iHealth             = 100;
    m_iAccuracy           = 20;
    m_iAmmoGrenade        = 0;
    m_iBalconyHeight      = 128;
    m_iDisguiseLevel      = 1;
    m_fDisguisePeriod     = 30000;
    m_fDisguiseRange      = 256;
    m_fEnemyShareRange    = 0;
    m_fFixedLeash         = 0.0;
    m_fGrenadeAwareness   = 20.0;
    m_fMaxNoticeTimeScale = 1;
    m_fSoundAwareness     = 100;
    m_bPatrolWaitTrigger  = 0;
    m_fHearing            = 2048;
    m_fSight              = world->m_fAIVisionDistance;
    m_fFov                = 90;
    m_fLeash              = 512;
    m_fMinDist            = 128.0;
    m_fMaxDist            = 1024;
    m_fInterval           = 128.0;
    m_bDontDropWeapons    = 0;
    m_bDontDropHealth     = 0;
    m_bNoSurprise         = 0;
    m_bForceDropWeapon    = 0;
    m_bForceDropHealth    = 0;
}

void AISpawnPoint::GetForceDropHealth(Event *ev)
{
    ev->AddInteger(m_bForceDropHealth);
}

void AISpawnPoint::SetForceDropHealth(Event *ev)
{
    // This is a bug on original mohaab (2.30 and 2.40), it should be m_bForceDropHealth
    //m_bForceDropWeapon = ev->GetBoolean(1);
    m_bForceDropHealth = ev->GetBoolean(1);
}

void AISpawnPoint::GetForceDropWeapon(Event *ev)
{
    ev->AddInteger(m_bForceDropWeapon);
}

void AISpawnPoint::SetForceDropWeapon(Event *ev)
{
    m_bForceDropWeapon = ev->GetBoolean(1);
}

void AISpawnPoint::GetVoiceType(Event *ev)
{
    ev->AddString(m_sVoiceType);
}

void AISpawnPoint::SetVoiceType(Event *ev)
{
    m_sVoiceType = ev->GetString(1);
}

void AISpawnPoint::GetTarget(Event *ev)
{
    ev->AddString(m_sTarget);
}

void AISpawnPoint::SetTarget(Event *ev)
{
    m_sTarget = ev->GetString(1);
}

void AISpawnPoint::GetWeapon(Event *ev)
{
    ev->AddString(m_sWeapon);
}

void AISpawnPoint::SetWeapon(Event *ev)
{
    m_sWeapon = ev->GetString(1);
}

void AISpawnPoint::GetAlarmNode(Event *ev)
{
    ev->AddString(m_sAlarmNode);
}

void AISpawnPoint::SetAlarmNode(Event *ev)
{
    m_sAlarmNode = ev->GetString(1);
}

void AISpawnPoint::GetTurret(Event *ev)
{
    ev->AddString(m_sTurret);
}

void AISpawnPoint::SetTurret(Event *ev)
{
    m_sTurret = ev->GetString(1);
}

void AISpawnPoint::GetPatrolPath(Event *ev)
{
    ev->AddString(m_sPatrolPath);
}

void AISpawnPoint::SetPatrolPath(Event *ev)
{
    m_sPatrolPath = ev->GetString(1);
}

void AISpawnPoint::GetNoSurprise(Event *ev)
{
    ev->AddInteger(m_bNoSurprise);
}

void AISpawnPoint::SetNoSurprise(Event *ev)
{
    m_bNoSurprise = ev->GetBoolean(1);
}

void AISpawnPoint::SetFavoriteEnemy(Event *ev)
{
    m_sFavoriteEnemy = ev->GetString(1);
}

void AISpawnPoint::GetFavoriteEnemy(Event *ev)
{
    ev->AddString(m_sFavoriteEnemy);
}

void AISpawnPoint::GetDontDropHealth(Event *ev)
{
    ev->AddInteger(m_bDontDropHealth);
}

void AISpawnPoint::SetDontDropHealth(Event *ev)
{
    m_bDontDropHealth = ev->GetBoolean(1);
}

void AISpawnPoint::GetDontDropWeapons(Event *ev)
{
    ev->AddInteger(m_bDontDropWeapons);
}

void AISpawnPoint::SetDontDropWeapons(Event *ev)
{
    m_bDontDropWeapons = ev->GetBoolean(1);
}

void AISpawnPoint::GetInterval(Event *ev)
{
    ev->AddFloat(m_fInterval);
}

void AISpawnPoint::SetInterval(Event *ev)
{
    m_fInterval = ev->GetFloat(1);
}

void AISpawnPoint::GetMaxDistance(Event *ev)
{
    ev->AddFloat(m_fMaxDist);
}

void AISpawnPoint::SetMaxDistance(Event *ev)
{
    m_fMaxDist = ev->GetFloat(1);
}

void AISpawnPoint::GetMinDistance(Event *ev)
{
    ev->AddFloat(m_fMinDist);
}

void AISpawnPoint::SetMinDistance(Event *ev)
{
    m_fMinDist = ev->GetFloat(1);
}

void AISpawnPoint::GetLeash(Event *ev)
{
    ev->AddFloat(m_fLeash);
}

void AISpawnPoint::SetLeash(Event *ev)
{
    m_fLeash = ev->GetFloat(1);
}

void AISpawnPoint::GetFov(Event *ev)
{
    ev->AddFloat(m_fFov);
}

void AISpawnPoint::SetFov(Event *ev)
{
    m_fFov = ev->GetFloat(1);
}

void AISpawnPoint::SetSight(Event *ev)
{
    ev->AddFloat(m_fSight);
}

void AISpawnPoint::GetSight(Event *ev)
{
    m_fSight = ev->GetFloat(1);
}

void AISpawnPoint::GetHearing(Event *ev)
{
    ev->AddFloat(m_fHearing);
}

void AISpawnPoint::SetHearing(Event *ev)
{
    m_fHearing = ev->GetFloat(1);
}

void AISpawnPoint::GetPatrolWaitTrigger(Event *ev)
{
    ev->AddInteger(m_bPatrolWaitTrigger);
}

void AISpawnPoint::SetPatrolWaitTrigger(Event *ev)
{
    m_bPatrolWaitTrigger = ev->GetBoolean(1);
}

void AISpawnPoint::GetTypeIdle(Event *ev)
{
    ev->AddString(m_sTypeIdle);
}

void AISpawnPoint::SetTypeIdle(Event *ev)
{
    m_sTypeIdle = ev->GetString(1);
}

void AISpawnPoint::GetTypeGrenade(Event *ev)
{
    ev->AddString(m_sTypeGrenade);
}

void AISpawnPoint::SetTypeGrenade(Event *ev)
{
    m_sTypeGrenade = ev->GetString(1);
}

void AISpawnPoint::GetTypeDisguise(Event *ev)
{
    ev->AddString(m_sTypeDisguise);
}

void AISpawnPoint::SetTypeDisguise(Event *ev)
{
    m_sTypeDisguise = ev->GetString(1);
}

void AISpawnPoint::GetTypeAttack(Event *ev)
{
    ev->AddString(m_sTypeAttack);
}

void AISpawnPoint::SetTypeAttack(Event *ev)
{
    m_sTypeAttack = ev->GetString(1);
}

void AISpawnPoint::SetSoundAwareness(Event *ev)
{
    m_fSoundAwareness = ev->GetFloat(1);
}

void AISpawnPoint::GetSoundAwareness(Event *ev)
{
    ev->AddFloat(m_fSoundAwareness);
}

void AISpawnPoint::SetMaxNoticeTimeScale(Event *ev)
{
    m_fMaxNoticeTimeScale = ev->GetFloat(1);
}

void AISpawnPoint::GetMaxNoticeTimeScale(Event *ev)
{
    ev->AddFloat(m_fMaxNoticeTimeScale);
}

void AISpawnPoint::GetGun(Event *ev)
{
    ev->AddString(m_sGun);
}

void AISpawnPoint::SetGun(Event *ev)
{
    m_sGun = ev->GetString(1);
}

void AISpawnPoint::GetGrenadeAwareness(Event *ev)
{
    ev->AddFloat(m_fGrenadeAwareness);
}

void AISpawnPoint::SetGrenadeAwareness(Event *ev)
{
    m_fGrenadeAwareness = ev->GetFloat(1);
}

void AISpawnPoint::SetFixedLeash(Event *ev)
{
    m_fFixedLeash = ev->GetFloat(1);
}

void AISpawnPoint::GetFixedLeash(Event *ev)
{
    ev->AddFloat(m_fFixedLeash);
}

void AISpawnPoint::GetEnemyShareRange(Event *ev)
{
    ev->AddFloat(m_fEnemyShareRange);
}

void AISpawnPoint::SetEnemyShareRange(Event *ev)
{
    m_fEnemyShareRange = ev->GetFloat(1);
}

void AISpawnPoint::GetDisguiseRange(Event *ev)
{
    ev->AddFloat(m_fDisguiseRange);
}

void AISpawnPoint::SetDisguiseRange(Event *ev)
{
    m_fDisguiseRange = ev->GetFloat(1);
}

void AISpawnPoint::GetDisguisePeriod(Event *ev)
{
    ev->AddFloat(m_fDisguisePeriod);
}

void AISpawnPoint::SetDisguisePeriod(Event *ev)
{
    m_fDisguisePeriod = ev->GetFloat(1);
}

void AISpawnPoint::GetDisguiseLevel(Event *ev)
{
    ev->AddInteger(m_iDisguiseLevel);
}

void AISpawnPoint::SetDisguiseLevel(Event *ev)
{
    m_iDisguiseLevel = ev->GetInteger(1);
}

void AISpawnPoint::GetBalconyHeight(Event *ev)
{
    ev->AddInteger(m_iBalconyHeight);
}

void AISpawnPoint::SetBalconyHeight(Event *ev)
{
    m_iBalconyHeight = ev->GetInteger(1);
}

void AISpawnPoint::GetAmmoGrenade(Event *ev)
{
    ev->AddInteger(m_iAmmoGrenade);
}

void AISpawnPoint::SetAmmoGrenade(Event *ev)
{
    m_iAmmoGrenade = ev->GetInteger(1);
}

void AISpawnPoint::GetAccuracy(Event *ev)
{
    ev->AddInteger(m_iAccuracy);
}

void AISpawnPoint::SetAccuracy(Event *ev)
{
    m_iAccuracy = ev->GetInteger(1);
}

void AISpawnPoint::GetEnemyName(Event *ev)
{
    ev->AddString(m_sEnemyName);
}

void AISpawnPoint::SetEnemyName(Event *ev)
{
    m_sEnemyName = ev->GetString(1);
}

void AISpawnPoint::GetHealth(Event *ev)
{
    ev->AddInteger(m_iHealth);
}

void AISpawnPoint::SetHealth(Event *ev)
{
    m_iHealth = ev->GetInteger(1);
}

void AISpawnPoint::GetModel(Event *ev)
{
    ev->AddString(m_sModel);
}

void AISpawnPoint::SetModel(Event *ev)
{
    m_sModel = ev->GetString(1);
}

void AISpawnPoint::Archive(Archiver& arc)
{
    SimpleEntity::Archive(arc);

    arc.ArchiveString(&m_sModel);
    arc.ArchiveInteger(&m_iHealth);
    arc.ArchiveString(&m_sEnemyName);
    arc.ArchiveFloat(&m_iAccuracy);
    arc.ArchiveInteger(&m_iAmmoGrenade);
    arc.ArchiveFloat(&m_iBalconyHeight);
    arc.ArchiveInteger(&m_iDisguiseLevel);
    arc.ArchiveFloat(&m_fDisguisePeriod);
    arc.ArchiveFloat(&m_fDisguiseRange);
    arc.ArchiveFloat(&m_fEnemyShareRange);
    arc.ArchiveFloat(&m_fFixedLeash);
    arc.ArchiveFloat(&m_fGrenadeAwareness);
    arc.ArchiveString(&m_sGun);
    arc.ArchiveFloat(&m_fMaxNoticeTimeScale);
    arc.ArchiveFloat(&m_fSoundAwareness);
    arc.ArchiveString(&m_sTypeAttack);
    arc.ArchiveString(&m_sTypeDisguise);
    arc.ArchiveString(&m_sTypeGrenade);
    arc.ArchiveString(&m_sTypeIdle);
    arc.ArchiveBool(&m_bPatrolWaitTrigger);
    arc.ArchiveFloat(&m_fHearing);
    arc.ArchiveFloat(&m_fSight);
    arc.ArchiveFloat(&m_fFov);
    arc.ArchiveFloat(&m_fLeash);
    arc.ArchiveFloat(&m_fMinDist);
    arc.ArchiveFloat(&m_fMaxDist);
    arc.ArchiveFloat(&m_fInterval);
    arc.ArchiveBool(&m_bDontDropWeapons);
    arc.ArchiveBool(&m_bDontDropHealth);
    arc.ArchiveString(&m_sFavoriteEnemy);
    arc.ArchiveBool(&m_bNoSurprise);
    arc.ArchiveString(&m_sPatrolPath);
    arc.ArchiveString(&m_sTurret);
    arc.ArchiveString(&m_sAlarmNode);
    arc.ArchiveString(&m_sWeapon);
    arc.ArchiveString(&m_sTarget);
    arc.ArchiveString(&m_sVoiceType);
    arc.ArchiveBool(&m_bForceDropWeapon);
    arc.ArchiveBool(&m_bForceDropHealth);
}
