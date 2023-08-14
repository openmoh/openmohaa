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
#include "scriptthread.h"
#include "game.h"
#include "weapon.h"
#include "weaputils.h"
#include "actor.h"
#include "sentient.h"

Event EV_PG_SetID("ID", EV_DEFAULT, "i", "ID", "Sets the ID for this projectile generator\nDefault=0:");
Event EV_PG_SetModel(
    "Model", EV_DEFAULT, "s", "Model", "Projectile model to use.\nDefault=models/projectiles/bazookashell.tik"
);
Event EV_PG_MinDuration(
    "MinDuration", EV_DEFAULT, "f", "MinDuration", "Sets the minimum duration of the bursts (in seconds)\nDefault=3"
);
Event EV_PG_MaxDuration(
    "MaxDuration", EV_DEFAULT, "f", "MaxDuration", "Sets the maximum duration of bursts(in seconds)\nDefault=3"
);
Event EV_PG_MinNumShots(
    "MinNumShots", EV_DEFAULT, "i", "MinNumShots", "Sets the minimum # of shots to fire in a cycle\nDefault=1"
);
Event EV_PG_MaxNumShots(
    "MaxNumShots", EV_DEFAULT, "i", "MaxNumShots", "Sets the maximum # of shots to fire in a cycle\nDefault=1"
);
Event EV_PG_Cycles("Cycles", EV_DEFAULT, "i", "Cycles", "Number of cycles. 0=infinte\nDefault=0");
Event EV_PG_MinDelay("MinDelay", EV_DEFAULT, "f", "MinDelay", "Minimum time between bursts.\n");
Event EV_PG_MaxDelay("MaxDelay", EV_DEFAULT, "f", "MaxDelay", "Maximum time between bursts\nDefault=10");
Event EV_PG_Accuracy("Accuracy", EV_DEFAULT, "f", "Accuracy", "Accuracy 0-25 feet\nDefault=10");
Event EV_PG_ClearTarget("ClearTarget", EV_DEFAULT, NULL, NULL, "Pick another target...");
Event EV_PG_BeginCycle("BeginCycle", EV_DEFAULT, NULL, NULL, "Startup the cycle..");
Event EV_TickCycle("TickCycle", EV_DEFAULT, NULL, NULL, "Tick the cycle..");
Event EV_EndCycle("EndCycle", EV_DEFAULT, NULL, NULL, "End the cycle..");
Event EV_TurnOn("TurnOn", EV_DEFAULT, NULL, NULL, "Turn On and begin the first cycle");
Event EV_TurnOff("TurnOff", EV_DEFAULT, NULL, NULL, "Turn Off and cancel the current cycle (if any)");
Event
    EV_Set_FireDelay("firedelay", EV_DEFAULT, "f", "fFireDelay", "Set the minimum time between shots from the weapon");
Event EV_Initialize("initialize", EV_DEFAULT, NULL, NULL, "Initialize object");
Event EV_Set_FireOnStartUp(
    "FireOnStartUp", EV_DEFAULT, 0, "i", "Default=1; Set this to 0 if you don't want the PG to fire on startup."
);
Event EV_PG_isDonut("isdonut", EV_DEFAULT, "i", "isdonut", "will target donut");
Event EV_PG_arcDonut("arc", EV_DEFAULT, "f", "arc", "arc in front of the player");
Event EV_PG_minDonut("mindonut", EV_DEFAULT, "f", "mindonut", "min extent of donut from player");
Event EV_PG_maxDonut("maxdonut", EV_DEFAULT, "f", "maxdonut", "max extent of donut from player");
Event EV_PG_PlayPreImpactSound("playpreimpact", EV_DEFAULT, NULL, NULL, "Play a pre-impact sound");
Event EV_PG_SetPreImpactSound("preimpactsound", EV_DEFAULT, "s", "Sound", "Set a pre-impact sound.");
Event EV_PG_SetPreImpactSoundTime(
    "preimpactsoundtime", EV_DEFAULT, "f", "delay", "Set the time before impact to play the preimpact sound."
);
Event EV_PG_SetPreImpactSoundProbability(
    "preimpactsoundprob",
    EV_DEFAULT,
    "f",
    "probability",
    "Set the chance for a pre-impact sound to occur.\n"
    "Range: ( 0.0, 1.0 ), with 0 having no chance, and 1 always occurring."
);
Event EV_PG_SetLaunchSound("launchsound", EV_DEFAULT, "s", "Sound", "Set a launch sound for the projectile.");
Event EV_PG_GetTargetEntity(
    "gettargetentity", EV_DEFAULT, NULL, NULL, "Returns entity being targeted by the projectile generator,", EV_GETTER
);
Event EV_PG_IsTurnedOn("isturnedon", EV_DEFAULT, NULL, NULL, "Returns whether the generator is on or off.", EV_GETTER);

CLASS_DECLARATION(Entity, ProjectileTarget, "ProjectileGenerator_Target") {
    {&EV_PG_SetID, &ProjectileTarget::EventSetId},
    {NULL,         NULL                         }
};

ProjectileTarget::ProjectileTarget()
{
    // FIXME: unimplemented
}

void ProjectileTarget::EventSetId(Event *ev)
{
    // FIXME: unimplemented
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
    // FIXME: unimplemented
}

void ProjectileGenerator::ShouldStartOn()
{
    // FIXME: unimplemented
}

void ProjectileGenerator::ShouldHideModel()
{
    // FIXME: unimplemented
}

void ProjectileGenerator::ShouldPlayFireSound()
{
    // FIXME: unimplemented
}

void ProjectileGenerator::EventIsTurnedOn(Event *ev)
{
    // FIXME: unimplemented
}

void ProjectileGenerator::EventGetTargetEntity(Event *ev)
{
    // FIXME: unimplemented
}

void ProjectileGenerator::EventLaunchSound(Event *ev)
{
    // FIXME: unimplemented
}

void ProjectileGenerator::SetTarget(Event *ev)
{
    // FIXME: unimplemented
}

void ProjectileGenerator::OnInitialize(Event *ev)
{
    // FIXME: unimplemented
}

void ProjectileGenerator::TurnOff(Event *ev)
{
    // FIXME: unimplemented
}

void ProjectileGenerator::TurnOn(Event *ev)
{
    // FIXME: unimplemented
}

void ProjectileGenerator::SetupNextCycle()
{
    // FIXME: unimplemented
}

void ProjectileGenerator::ShouldTargetRandom()
{
    // FIXME: unimplemented
}

void ProjectileGenerator::ChooseTarget()
{
    // FIXME: unimplemented
}

void ProjectileGenerator::GetLocalTargets()
{
    // FIXME: unimplemented
}

void ProjectileGenerator::ShouldTargetPlayer()
{
    // FIXME: unimplemented
}

void ProjectileGenerator::GetTargetPos(Entity *target)
{
    // FIXME: unimplemented
}

void ProjectileGenerator::EventAccuracy(Event *ev)
{
    // FIXME: unimplemented
}

void ProjectileGenerator::EventMaxDelay(Event *ev)
{
    // FIXME: unimplemented
}

void ProjectileGenerator::EventMinDelay(Event *ev)
{
    // FIXME: unimplemented
}

void ProjectileGenerator::EventFireOnStartUp(Event *ev)
{
    // FIXME: unimplemented
}

void ProjectileGenerator::EventMaxNumShots(Event *ev)
{
    // FIXME: unimplemented
}

void ProjectileGenerator::EventMinNumShots(Event *ev)
{
    // FIXME: unimplemented
}

void ProjectileGenerator::EventCycles(Event *ev)
{
    // FIXME: unimplemented
}

void ProjectileGenerator::EventMaxDuration(Event *ev)
{
    // FIXME: unimplemented
}

void ProjectileGenerator::EventMinDuration(Event *ev)
{
    // FIXME: unimplemented
}

void ProjectileGenerator::SetWeaponModel(Event *ev)
{
    // FIXME: unimplemented
}

void ProjectileGenerator::EventSetId(Event *ev)
{
    // FIXME: unimplemented
}

void ProjectileGenerator::BeginCycle(Event *ev)
{
    // FIXME: unimplemented
}

void ProjectileGenerator::TickCycle(Event *ev)
{
    // FIXME: unimplemented
}

void ProjectileGenerator::Attack(int count)
{
    // FIXME: unimplemented
}

void ProjectileGenerator::EndCycle(Event *ev)
{
    // FIXME: unimplemented
}

void ProjectileGenerator::EventarcDonut(Event *ev)
{
    // FIXME: unimplemented
}

void ProjectileGenerator::EventmaxDonut(Event *ev)
{
    // FIXME: unimplemented
}

void ProjectileGenerator::EventminDonut(Event *ev)
{
    // FIXME: unimplemented
}

void ProjectileGenerator::EventisDonut(Event *ev)
{
    // FIXME: unimplemented
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
    // FIXME: unimplemented
}

void ProjectileGenerator_Projectile::SetPreImpactSoundProbability(Event *ev)
{
    // FIXME: unimplemented
}

void ProjectileGenerator_Projectile::SetPreImpactSoundTime(Event *ev)
{
    // FIXME: unimplemented
}

void ProjectileGenerator_Projectile::SetPreImpactSound(Event *ev)
{
    // FIXME: unimplemented
}

void ProjectileGenerator_Projectile::PlayPreImpactSound(Event *ev)
{
    // FIXME: unimplemented
}

void ProjectileGenerator_Projectile::SetProjectileModel(Event *ev)
{
    // FIXME: unimplemented
}

void ProjectileGenerator_Projectile::Attack(int count)
{
    // FIXME: unimplemented
}

void ProjectileGenerator_Projectile::Archive(Archiver& arc)
{
    ProjectileGenerator::Archive(arc);

    arc.ArchiveString(&m_sProjectileModel);
    arc.ArchiveString(&m_sPreImpactSound);
    arc.ArchiveFloat(&m_fImpactSoundTime);
    arc.ArchiveFloat(&m_fImpactSoundProbability);
}

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
    // FIXME: unimplemented
}

void ProjectileGenerator_Gun::SetFireDelay(Event *ev)
{
    // FIXME: unimplemented
}

void ProjectileGenerator_Gun::SetMeansOfDeath(Event *ev)
{
    // FIXME: unimplemented
}

void ProjectileGenerator_Gun::SetBulletThroughWood(Event *ev)
{
    // FIXME: unimplemented
}

void ProjectileGenerator_Gun::SetBulletThroughMetal(Event *ev)
{
    // FIXME: unimplemented
}

void ProjectileGenerator_Gun::SetBulletKnockback(Event *ev)
{
    // FIXME: unimplemented
}

void ProjectileGenerator_Gun::SetTracerSpeed(Event *ev)
{
    // FIXME: unimplemented
}

void ProjectileGenerator_Gun::SetBulletLarge(Event *ev)
{
    // FIXME: unimplemented
}

void ProjectileGenerator_Gun::SetFakeBullets(Event *ev)
{
    // FIXME: unimplemented
}

void ProjectileGenerator_Gun::SetBulletCount(Event *ev)
{
    // FIXME: unimplemented
}

void ProjectileGenerator_Gun::SetBulletDamage(Event *ev)
{
    // FIXME: unimplemented
}

void ProjectileGenerator_Gun::SetTracerFrequency(Event *ev)
{
    // FIXME: unimplemented
}

void ProjectileGenerator_Gun::SetBulletSpread(Event *ev)
{
    // FIXME: unimplemented
}

void ProjectileGenerator_Gun::SetBulletRange(Event *ev)
{
    // FIXME: unimplemented
}

void ProjectileGenerator_Gun::Attack(int count)
{
    // FIXME: unimplemented
}

void ProjectileGenerator_Gun::TickCycle(Event *ev)
{
    // FIXME: unimplemented
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

CLASS_DECLARATION(ProjectileGenerator, ProjectileGenerator_Heavy, "ProjectileGenerator_Heavy") {
    {&EV_Weapon_SetProjectile, &ProjectileGenerator_Heavy::SetProjectileModel},
    {NULL,                     NULL                                          }
};

ProjectileGenerator_Heavy::ProjectileGenerator_Heavy()
{
    // FIXME: unimplemented
}

void ProjectileGenerator_Heavy::SetProjectileModel(Event *ev)
{
    // FIXME: unimplemented
}

void ProjectileGenerator_Heavy::Attack(int count)
{
    // FIXME: unimplemented
}

void ProjectileGenerator_Heavy::Archive(Archiver& arc)
{
    ProjectileGenerator::Archive(arc);

    arc.ArchiveString(&m_sProjectileModel);
}

Event EV_DestroyModel("DestroyModel", EV_DEFAULT, "s", "DestroyModel", 0);
Event EV_UsedModel("UsedModel", EV_DEFAULT, "s", "UsedModel", 0);
Event EV_ExplosionSound("ExplosionSound", EV_DEFAULT, "s", "ExplosionSound", 0);
Event EV_ActivateSound("ActivateSound", EV_DEFAULT, "s", "ActivateSound", 0);
Event EV_TickSound("TickSound", EV_DEFAULT, "s", "TickSound", 0);
Event EV_SetDamage("SetDamage", EV_DEFAULT, "f", "Damage", 0);
Event EV_Radius("Radius", EV_DEFAULT, "f", "Radius", 0);
Event EV_StopWatchDuration("StopWatchDuration", EV_DEFAULT, "f", "StopWatchDuration", 0);
Event EV_SetThread("setthread", EV_DEFAULT, "s", "thread", "Set the thread to execute when this object is used");
Event EV_SetUseThread("setusethread", EV_DEFAULT, "s", "thread", "Set the thread to execute when this object is used");
Event EV_DoExplosion("Explode", EV_DEFAULT, NULL, NULL, "Cause the explosion to happen.");
Event EV_SetTriggered("triggered", EV_DEFAULT, "i", "0 or 1", "Set the triggered status");
Event EV_SetExplosionEffect("explosioneffect", EV_DEFAULT, "s", "effect model", "Set the explosion effect model");
Event EV_SetExplosionOffset("explosionoffset", EV_DEFAULT, "v", "offset vector", "Set the explosion offset");
Event EV_BlowUp("BlowUp", EV_DEFAULT, NULL, NULL, NULL);

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
    // FIXME: unimplemented
}

void ThrobbingBox_Explosive::SetExplosionOffset(Event *ev)
{
    // FIXME: unimplemented
}

void ThrobbingBox_Explosive::SetExplosionEffect(Event *ev)
{
    // FIXME: unimplemented
}

void ThrobbingBox_Explosive::SetTriggered(Event *ev)
{
    // FIXME: unimplemented
}

void ThrobbingBox_Explosive::DoExplosion(Event *ev)
{
    // FIXME: unimplemented
}

void ThrobbingBox_Explosive::SetUseThread(Event *ev)
{
    // FIXME: unimplemented
}

void ThrobbingBox_Explosive::SetThread(Event *ev)
{
    // FIXME: unimplemented
}

void ThrobbingBox_Explosive::SetStopWatchDuration(Event *ev)
{
    // FIXME: unimplemented
}

void ThrobbingBox_Explosive::SetRadius(Event *ev)
{
    // FIXME: unimplemented
}

void ThrobbingBox_Explosive::SetDamage(Event *ev)
{
    // FIXME: unimplemented
}

void ThrobbingBox_Explosive::TickSound(Event *ev)
{
    // FIXME: unimplemented
}

void ThrobbingBox_Explosive::ActivateSound(Event *ev)
{
    // FIXME: unimplemented
}

void ThrobbingBox_Explosive::ExplosionSound(Event *ev)
{
    // FIXME: unimplemented
}

void ThrobbingBox_Explosive::UsedModel(Event *ev)
{
    // FIXME: unimplemented
}

void ThrobbingBox_Explosive::SetDestroyModel(Event *ev)
{
    // FIXME: unimplemented
}

void ThrobbingBox_Explosive::OnBlowUp(Event *ev)
{
    // FIXME: unimplemented
}

void ThrobbingBox_Explosive::OnUse(Event *ev)
{
    // FIXME: unimplemented
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

CLASS_DECLARATION(ThrobbingBox_Explosive, ThrobbingBox_ExplodePlayerFlak88, "ThrobbingBox_ExplodePlayerFlak88") {
    {NULL, NULL}
};

CLASS_DECLARATION(ThrobbingBox_Explosive, ThrobbingBox_ExplodeFlak88, "ThrobbingBox_ExplodeFlak88") {
    {NULL, NULL}
};

CLASS_DECLARATION(ThrobbingBox_Explosive, ThrobbingBox_ExplodeNebelwerfer, "ThrobbingBox_ExplodeNebelwerfer") {
    {NULL, NULL}
};

CLASS_DECLARATION(ThrobbingBox_Explosive, ThrobbingBox_ExplodePlayerNebelwerfer, "ThrobbingBox_ExplodeNebelwerfer") {
    {NULL, NULL}
};

Event EV_StickyBombWet("stickybombwet", EV_DEFAULT, NULL, NULL, NULL);

CLASS_DECLARATION(ThrobbingBox_Explosive, ThrobbingBox_Stickybomb, "ThrobbingBox_ExplodeNebelwerfer") {
    {&EV_Damage,        &ThrobbingBox_Stickybomb::OnBlowUp       },
    {&EV_Use,           &ThrobbingBox_Stickybomb::OnStickyBombUse},
    {&EV_StickyBombWet, &ThrobbingBox_Stickybomb::OnStickyBombWet},
    {NULL,              NULL                                     }
};

void ThrobbingBox_Stickybomb::OnStickyBombWet(Event *ev)
{
    // FIXME: unimplemented
}

void ThrobbingBox_Stickybomb::OnStickyBombUse(Event *ev)
{
    // FIXME: unimplemented
}

void ThrobbingBox_Stickybomb::Archive(Archiver& arc)
{
    ThrobbingBox_Explosive::Archive(arc);

    arc.ArchiveFloat(&m_fStopwatchStartTime);
}

Event EV_Complete("Complete", EV_DEFAULT, NULL, NULL, "Complete this objective.");

Event EV_SetCurrent("SetCurrent", EV_DEFAULT, NULL, NULL, "Set this objective as current.");

Event EV_SetText("Text", EV_DEFAULT, "s", "text", "Set current text.");

Event EV_SetObjectiveNbr("ObjectiveNbr", EV_DEFAULT, "i", "index", "Sets the objective number.");

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
    m_sText = ev->GetString(2);
}

CLASS_DECLARATION(Entity, FencePost, "func_fencepost") {
    {&EV_Killed, NULL},
    {NULL,       NULL}
};

Event EV_SetEnemyName("enemyname", EV_DEFAULT, "s", "enemyname", "");
Event EV_SetEnemyName2("enemyname", EV_DEFAULT, "s", "enemyname", "", EV_GETTER);
Event EV_GetEnemyName("enemyname", EV_DEFAULT, "s", "enemyname", "", EV_SETTER);
Event EV_Sentient_GetDontDropWeapons("dontdropweapons", EV_DEFAULT, NULL, NULL, "dontdropweapons getter", EV_GETTER);
Event EV_SetDontDropHealth("dontdrophealth", EV_DEFAULT, NULL, NULL, "dontdrophealth setter");
Event EV_GetDontDropHealth("dontdrophealth", EV_DEFAULT, NULL, NULL, "dontdrophealth getter", EV_GETTER);

CLASS_DECLARATION(SimpleArchivedEntity, AISpawnPoint, "info_aispawnpoint") {
    {&EV_Model, &AISpawnPoint::SetModel},
    {&EV_GetModel, &AISpawnPoint::GetModel},
    {&EV_SetHealth2, &AISpawnPoint::SetHealth},
    {&EV_Entity_GetHealth, &AISpawnPoint::GetHealth},
    {&EV_SetEnemyName, &AISpawnPoint::SetEnemyName},
    {
     &EV_SetEnemyName2,
     &AISpawnPoint::SetEnemyName,
     },
    {&EV_GetEnemyName, &AISpawnPoint::GetEnemyName},
    {&EV_Actor_SetAccuracy, &AISpawnPoint::SetAccuracy},
    {&EV_Actor_GetAccuracy, &AISpawnPoint::GetAccuracy},
    {&EV_Actor_SetAmmoGrenade, &AISpawnPoint::SetAmmoGrenade},
    {&EV_Actor_GetAmmoGrenade, &AISpawnPoint::GetAmmoGrenade},
    {&EV_Actor_SetBalconyHeight, &AISpawnPoint::SetBalconyHeight},
    {&EV_Actor_GetBalconyHeight, &AISpawnPoint::GetBalconyHeight},
    {&EV_Actor_SetDisguiseLevel, &AISpawnPoint::SetDisguiseLevel},
    {&EV_Actor_GetDisguiseLevel, &AISpawnPoint::GetDisguiseLevel},
    {&EV_Actor_SetDisguisePeriod2, &AISpawnPoint::SetDisguisePeriod},
    {&EV_Actor_GetDisguisePeriod, &AISpawnPoint::GetDisguisePeriod},
    {&EV_Actor_SetDisguiseRange2, &AISpawnPoint::SetDisguiseRange},
    {&EV_Actor_GetDisguiseRange, &AISpawnPoint::GetDisguiseRange},
    {&EV_Actor_SetEnemyShareRange2, &AISpawnPoint::SetEnemyShareRange},
    {&EV_Actor_GetEnemyShareRange, &AISpawnPoint::GetEnemyShareRange},
    {&EV_Actor_GetFixedLeash, &AISpawnPoint::GetFixedLeash},
    {&EV_Actor_SetFixedLeash2, &AISpawnPoint::SetFixedLeash},
    {&EV_Actor_SetGrenadeAwareness, &AISpawnPoint::SetGrenadeAwareness},
    {&EV_Actor_GetGrenadeAwareness, &AISpawnPoint::GetGrenadeAwareness},
    {&EV_Actor_SetGun, &AISpawnPoint::SetGun},
    {&EV_Actor_GetGun, &AISpawnPoint::GetGun},
    {&EV_Actor_GetMaxNoticeTimeScale, &AISpawnPoint::GetMaxNoticeTimeScale},
    {&EV_Actor_SetMaxNoticeTimeScale, &AISpawnPoint::SetMaxNoticeTimeScale},
    {&EV_Actor_GetSoundAwareness, &AISpawnPoint::GetSoundAwareness},
    {&EV_Actor_SetSoundAwareness, &AISpawnPoint::SetSoundAwareness},
    {&EV_Actor_SetTypeAttack, &AISpawnPoint::SetTypeAttack},
    {&EV_Actor_GetTypeAttack, &AISpawnPoint::GetTypeAttack},
    {&EV_Actor_SetTypeDisguise, &AISpawnPoint::SetTypeDisguise},
    {&EV_Actor_GetTypeDisguise, &AISpawnPoint::GetTypeDisguise},
    {&EV_Actor_SetTypeGrenade, &AISpawnPoint::SetTypeGrenade},
    {&EV_Actor_GetTypeGrenade, &AISpawnPoint::GetTypeGrenade},
    {&EV_Actor_SetTypeIdle, &AISpawnPoint::SetTypeIdle},
    {&EV_Actor_GetTypeIdle, &AISpawnPoint::GetTypeIdle},
    {&EV_Actor_SetPatrolWaitTrigger, &AISpawnPoint::SetPatrolWaitTrigger},
    {&EV_Actor_GetPatrolWaitTrigger, &AISpawnPoint::GetPatrolWaitTrigger},
    {&EV_Actor_SetHearing, &AISpawnPoint::SetHearing},
    {&EV_Actor_GetHearing, &AISpawnPoint::GetHearing},
    {&EV_Actor_GetSight, &AISpawnPoint::GetSight},
    {&EV_Actor_SetSight2, &AISpawnPoint::SetSight},
    {&EV_Actor_SetFov, &AISpawnPoint::SetFov},
    {&EV_Actor_GetFov, &AISpawnPoint::GetFov},
    {&EV_Actor_SetLeash2, &AISpawnPoint::SetLeash},
    {&EV_Actor_GetLeash, &AISpawnPoint::GetLeash},
    {&EV_Actor_SetMinDistance, &AISpawnPoint::SetMinDistance},
    {&EV_Actor_GetMinDistance, &AISpawnPoint::GetMinDistance},
    {&EV_Actor_SetMaxDistance, &AISpawnPoint::SetMaxDistance},
    {&EV_Actor_GetMaxDistance, &AISpawnPoint::GetMaxDistance},
    {&EV_Actor_SetInterval2, &AISpawnPoint::SetInterval},
    {&EV_Actor_GetInterval, &AISpawnPoint::GetInterval},
    {&EV_Sentient_DontDropWeapons, &AISpawnPoint::SetDontDropWeapons},
    {&EV_Sentient_GetDontDropWeapons, &AISpawnPoint::GetDontDropWeapons},
    {&EV_SetDontDropHealth, &AISpawnPoint::SetDontDropHealth},
    {&EV_GetDontDropHealth, &AISpawnPoint::GetDontDropHealth},
    {&EV_Actor_GetFavoriteEnemy, &AISpawnPoint::GetFavoriteEnemy},
    {&EV_Actor_SetFavoriteEnemy2, &AISpawnPoint::SetFavoriteEnemy},
    {&EV_Actor_SetNoSurprise2, &AISpawnPoint::SetNoSurprise},
    {&EV_Actor_GetNoSurprise, &AISpawnPoint::GetNoSurprise},
    {&EV_Actor_SetPatrolPath, &AISpawnPoint::SetPatrolPath},
    {&EV_Actor_GetPatrolPath, &AISpawnPoint::GetPatrolPath},
    {&EV_Actor_SetTurret, &AISpawnPoint::SetTurret},
    {&EV_Actor_GetTurret, &AISpawnPoint::GetTurret},
    {&EV_Actor_SetAlarmNode, &AISpawnPoint::SetAlarmNode},
    {&EV_Actor_GetAlarmNode, &AISpawnPoint::GetAlarmNode},
    {&EV_Actor_SetWeapon, &AISpawnPoint::SetWeapon},
    {&EV_Actor_GetWeapon, &AISpawnPoint::GetWeapon},
    {&EV_SetTarget, &AISpawnPoint::SetTarget},
    {&EV_GetTarget, &AISpawnPoint::GetTarget},
    {&EV_Actor_SetVoiceType, &AISpawnPoint::SetVoiceType},
    {&EV_Actor_GetVoiceType, &AISpawnPoint::GetVoiceType},
    {&EV_Sentient_ForceDropWeapon, &AISpawnPoint::SetForceDropWeapon},
    {&EV_Sentient_GetForceDropWeapon, &AISpawnPoint::GetForceDropWeapon},
    {&EV_Sentient_ForceDropHealth, &AISpawnPoint::SetForceDropHealth},
    {&EV_Sentient_GetForceDropHealth, &AISpawnPoint::GetForceDropHealth},
    {NULL, NULL}
};

AISpawnPoint::AISpawnPoint()
{
    // FIXME: unimplemented
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
