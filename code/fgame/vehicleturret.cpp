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

// vehicleturret.cpp: Vehicle Turret.
//

#include "g_phys.h"
#include "vehicleturret.h"
#include "player.h"
#include "explosion.h"
#include "gibs.h"
#include "scriptexception.h"

static const Vector g_vUserMins(-16, -16, 0);
static const Vector g_vUserMaxs(16, 16, 96);

static constexpr float MAX_TANDEM_YAWOFFSET = 0;

Event EV_VehicleTurretGun_SetBaseEntity
(
    "setbaseentity",
    EV_DEFAULT,
    "e",
    "base_entity",
    "Sets the base entity to take its orientation from.",
    EV_NORMAL
);
Event EV_VehicleTurretGun_RemoveOnDeath
(
    "removeondeath",
    EV_DEFAULT,
    "i",
    "removeondeath",
    "If set to a non-zero value, vehicles will not be removed when they die",
    EV_NORMAL
);
Event EV_VehicleTurretGun_SetCollisionEntity
(
    "setcollisionentity",
    EV_DEFAULT,
    "e",
    "entity",
    "Sets the Collision Entity.",
    EV_NORMAL
);
Event EV_VehicleTurretGun_Lock
(
    "lock",
    EV_DEFAULT,
    NULL,
    NULL,
    "The turret cannot be used.",
    EV_NORMAL
);
Event EV_VehicleTurretGun_Unlock
(
    "unlock",
    EV_DEFAULT,
    NULL,
    NULL,
    "The turret can be used.",
    EV_NORMAL
);
Event EV_VehicleTurretGun_SoundSet
(
    "SoundSet",
    EV_DEFAULT,
    "s",
    "soundset",
    "Sets the Sound Set to use.",
    EV_NORMAL
);
Event EV_VehicleTurretGun_CollisionEntitySetter
(
    "collisionent",
    EV_DEFAULT,
    "e",
    "entity",
    "Sets the Collision Entity",
    EV_SETTER
);
Event EV_VehicleTurretGun_CollisionEntityGetter
(
    "collisionent",
    EV_DEFAULT,
    NULL,
    NULL,
    "Gets the Collision Entity",
    EV_GETTER
);
Event EV_VehicleTurretGun_WarmupDelay
(
    "warmupdelay",
    EV_DEFAULT,
    "f",
    "value",
    "Set the warmup delay before use after mounting the weapon.",
    EV_NORMAL
);
Event EV_VehicleTurretGun_FireWarmupDelay
(
    "firewarmupdelay",
    EV_DEFAULT,
    "f",
    "value",
    "Set the warmup delay before use after mounting the weapon.",
    EV_NORMAL
);
Event EV_VehicleTurretGun_ReloadShots
(
    "reloadshots",
    EV_DEFAULT,
    "i",
    "value",
    "Set the number of shots fired before forcing a reload",
    EV_NORMAL
);
Event EV_VehicleTurretGun_ReloadDelay
(
    "reloaddelay",
    EV_DEFAULT,
    "f",
    "value",
    "Set a delay that implies a reload. Will also play a sound",
    EV_NORMAL
);
Event EV_VehicleTurretGun_AimOffset
(
    "aimoffset",
    EV_DEFAULT,
    "v",
    "value",
    "Adjust aiming angles by offset.",
    EV_NORMAL
);
Event EV_VehicleTurretGun_AimTolerance
(
    "aimtolerance",
    EV_DEFAULT,
    "v",
    "caps",
    "Sets a tolerance for the angles.",
    EV_NORMAL
);
Event EV_VehicleTurretGun_SetTargetEntity
(
    "settargetentity",
    EV_DEFAULT,
    "e",
    "ent",
    "Set the entity to point the turret at visually.",
    EV_NORMAL
);
Event EV_VehicleTurretGun_PlayReloadSound
(
    "playreloadsound",
    EV_DEFAULT,
    NULL,
    NULL,
    "Play the turret reload sound.",
    EV_NORMAL
);

CLASS_DECLARATION(TurretGun, VehicleTurretGun, NULL) {
    {&EV_Item_DropToFloor,                       &VehicleTurretGun::PlaceTurret           },
    {&EV_Item_Pickup,                            NULL                                     },
    {&EV_Touch,                                  NULL                                     },
    {&EV_Use,                                    &VehicleTurretGun::TurretUsed            },
    {&EV_VehicleTurretGun_SetBaseEntity,         &VehicleTurretGun::SetBaseEntity         },
    {&EV_VehicleTurretGun_RemoveOnDeath,         &VehicleTurretGun::EventRemoveOnDeath    },
    {&EV_VehicleTurretGun_SetCollisionEntity,    &VehicleTurretGun::EventSetCollisionModel},
    {&EV_Damage,                                 &VehicleTurretGun::EventDamage           },
    {&EV_Killed,                                 &VehicleTurretGun::EventKilled           },
    {&EV_Turret_AI_TurnSpeed,                    &VehicleTurretGun::EventTurnSpeed        },
    {&EV_VehicleTurretGun_Lock,                  &VehicleTurretGun::EventLock             },
    {&EV_VehicleTurretGun_Unlock,                &VehicleTurretGun::EventUnlock           },
    {&EV_VehicleTurretGun_SoundSet,              &VehicleTurretGun::SetSoundSet           },
    {&EV_VehicleTurretGun_CollisionEntitySetter, &VehicleTurretGun::EventSetCollisionModel},
    {&EV_VehicleTurretGun_CollisionEntityGetter, &VehicleTurretGun::EventGetCollisionModel},
    {NULL,                                       NULL                                     }
};

VehicleTurretGun::VehicleTurretGun()
{
    entflags |= EF_VEHICLETURRET;

    AddWaitTill(STRING_DEATH);

    if (LoadingSavegame) {
        return;
    }

    edict->s.eType = ET_MODELANIM;
    setRespawn(false);

    respondto         = TRIGGER_PLAYERS | TRIGGER_MONSTERS;
    edict->clipmask   = MASK_VEHICLETURRET;

    m_bUsable         = true;
    m_bPlayerUsable   = true;
    m_bRestable       = false;
    m_fIdlePitchSpeed = 0;
    m_iIdleHitCount   = 0;

    takedamage        = DAMAGE_NO;
    health            = 100.0f;
    max_health        = 100.0f;

    setSize(Vector(-16, -16, 0), Vector(16, 16, 32));
    
    m_fPitchUpCap       = -45;
    m_fPitchDownCap     = 45;
    m_fMaxYawOffset     = 180;
    m_fTurnSpeed        = 160;
    m_fAIPitchSpeed     = 48;
    m_fUserDistance     = 64.0f;
    m_vIdleCheckOffset.setXYZ(-56, 0, 0);

    m_fMinBurstTime  = 0;
    m_fMaxBurstTime  = 0;
    m_fMinBurstDelay = 0;
    m_fMaxBurstDelay = 0;

    AxisClear(m_mBaseOrientation);
    m_fFireToggleTime = 0;
    m_iFiring         = 0;
    m_bBOIsSet        = false;

    m_pBaseEntity       = NULL;
    m_vLastBaseAngles   = vec_zero;
    m_vBaseAngles       = vec_zero;

    m_vBarrelPos        = origin;
    m_vLastBarrelPos    = origin;

    m_bUseRemoteControl = false;
    m_pVehicleOwner     = NULL;

    m_bRemoveOnDeath    = true;
    m_eSoundState       = ST_OFF;
    m_fNextSoundState   = level.time;

    m_fWarmupDelay         = 0;
    m_fFireWarmupDelay     = 0;
    m_fTargetReloadTime    = 0;
    m_fWarmupTimeRemaining = 0;
    m_fReloadDelay         = 0;
    m_fReloadTimeRemaining = 0;

    m_iReloadShots = 1;
    m_iAmmo = 1;

    ammo_in_clip[FIRE_PRIMARY]   = m_iAmmo;
    ammo_clip_size[FIRE_PRIMARY] = m_iReloadShots;

    m_vAimOffset[0]     = 0;
    m_vAimOffset[1]     = 0;
    m_vAimOffset[2]     = 0;
    m_vAimTolerance[0] = 20;
    m_vAimTolerance[1] = 20;

    m_bLocked           = true;
    m_sSoundSet         = "";
}

VehicleTurretGun::~VehicleTurretGun()
{
    Unregister(STRING_ONTARGET);

    if (m_pCollisionEntity) {
        m_pCollisionEntity->PostEvent(EV_Remove, 0);
    }

    entflags &= ~EF_VEHICLETURRET;
}

void VehicleTurretGun::Think(void)
{
    // FIXME: unimplemented
}

void VehicleTurretGun::P_UserAim(usercmd_t *ucmd)
{
    // FIXME: unimplemented
}

void VehicleTurretGun::TurretBeginUsed(Sentient *pEnt)
{
    if (m_pVehicleOwner) {
        Vehicle *pVehicle = (Vehicle *)m_pVehicleOwner.Pointer();
        int      slot     = pVehicle->FindTurretSlotByEntity(this);

        pVehicle->UpdateTurretSlot(slot);
        m_vBaseAngles = m_pVehicleOwner->angles;
    }

    owner = pEnt;

    edict->r.ownerNum = pEnt->entnum;
    m_bHadOwner       = true;

    Sound(sPickupSound);

    owner->SetViewAngles(m_vBaseAngles);

    m_vUserViewAng    = m_vBaseAngles;
    m_vUserViewAng[0] = AngleNormalize180(m_vUserViewAng[0]);
    m_vUserLastCmdAng = vec_zero;

    setAngles(m_vBaseAngles);

    if (owner->IsSubclassOfPlayer()) {
        Player *player = (Player *)owner.Pointer();

        player->EnterTurret(this);

        if (!m_pUserCamera) {
            m_pUserCamera = new Camera;
        }

        player->SetCamera(m_pUserCamera, 0.0f);
        m_pUserCamera->setAngles(m_vBaseAngles);
    }

    UpdateOwner(owner);
    flags &= ~FL_THINK;
    current_attachToTag = "";
    ForceIdle();
    P_CreateViewModel();
}

void VehicleTurretGun::TurretEndUsed(void)
{
    if (owner->IsSubclassOfPlayer()) {
        Player *player = (Player *)owner.Pointer();

        if (m_pUserCamera) {
            player->SetCamera(NULL, 0);
            player->ZoomOff();
            m_pUserCamera->PostEvent(EV_Remove, 0);
            m_pUserCamera = NULL;
        }

        player->ExitTurret();
        P_DeleteViewModel();
    }

    owner             = NULL;
    edict->r.ownerNum = ENTITYNUM_NONE;
    m_fIdlePitchSpeed = 0;
    m_iIdleHitCount   = 0;
    m_iFiring         = 0;
}

void VehicleTurretGun::TurretUsed(Sentient *pEnt)
{
    if (owner) {
        if (owner == pEnt) {
            TurretEndUsed();
            m_iFiring = 0;
        }
    } else {
        TurretBeginUsed(pEnt);
    }
}

void VehicleTurretGun::TurretUsed(Event *ev)
{
    Sentient *pEnt = (Sentient *)ev->GetEntity(1);

    if (!pEnt || !pEnt->IsSubclassOfSentient()) {
        return;
    }

    if (!m_bUsable || m_bLocked) {
        return;
    }

    if (pEnt->IsSubclassOfPlayer() && !m_bPlayerUsable) {
        return;
    }

    TurretUsed(pEnt);
}

void VehicleTurretGun::SetBaseOrientation(float (*borientation)[3], float *bangles)
{
    m_bBOIsSet = true;

    VectorCopy(borientation[0], m_mBaseOrientation[0]);
    VectorCopy(borientation[1], m_mBaseOrientation[1]);
    VectorCopy(borientation[2], m_mBaseOrientation[2]);
    m_vLastBaseAngles = m_vBaseAngles;
    if (bangles) {
        m_vBaseAngles = bangles;
    } else {
        MatrixToEulerAngles(borientation, m_vBaseAngles);
    }

    flags |= FL_THINK;
}

void VehicleTurretGun::SetBaseEntity(Entity *e)
{
    m_pBaseEntity     = e;
    m_vLastBaseAngles = m_vBaseAngles;

    if (e) {
        m_vBaseAngles = e->angles;
    } else {
        m_vBaseAngles = vec_zero;
    }

    flags |= FL_THINK;
}

void VehicleTurretGun::SetBaseEntity(Event *ev)
{
    SetBaseEntity(ev->GetEntity(1));
}

void VehicleTurretGun::RemoteControl(usercmd_t *ucmd, Sentient *owner) {}

void VehicleTurretGun::UpdateOrientation(bool bCollisionCheck) {}

void VehicleTurretGun::CollisionCorrect(trace_t *pTr) {}

void VehicleTurretGun::EventKilled(Event *ev)
{
    Entity *ent;
    Entity *attacker;

    deadflag = DEAD_DEAD;
    Unregister(STRING_DEATH);

    if (!m_bRemoveOnDeath) {
        return;
    }

    takedamage = DAMAGE_NO;
    setSolidType(SOLID_NOT);
    hideModel();

    attacker = ev->GetEntity(1);

    if (flags & FL_DIE_EXPLODE) {
        CreateExplosion(origin, 150 * edict->s.scale, this, this, this);
    }

    if (flags & FL_DIE_GIBS) {
        setSolidType(SOLID_NOT);
        hideModel();
        CreateGibs(this, -150, edict->s.scale, 3);
    }

    if (*killtarget) {
        ent = NULL;
        while ((ent = (Entity *)G_FindTarget(ent, killtarget)) != NULL) {
            ent->PostEvent(EV_Remove, 0);
        }
    }

    if (*target) {
        ent = NULL;
        while ((ent = (Entity *)G_FindTarget(ent, target)) != NULL) {
            ent->ProcessEvent(EV_Activate);
        }
    }

    PostEvent(EV_Remove, 0);
}

void VehicleTurretGun::EventDamage(Event *ev)
{
    if (owner) {
        owner->ProcessEvent(*ev);
        return;
    }

    if (m_pVehicleOwner) {
        m_pVehicleOwner->ProcessEvent(*ev);
        return;
    }

    if (!m_bUseRemoteControl) {
        DamageEvent(ev);
        return;
    }

    if (m_pRemoteOwner && m_pRemoteOwner->IsSubclassOfPlayer()) {
        Player *player = (Player *)m_pRemoteOwner.Pointer();

        if (player->m_pVehicle) {
            player->m_pVehicle->ProcessEvent(*ev);
            return;
        }
    }
}

void VehicleTurretGun::SetVehicleOwner(Entity *e)
{
    m_pVehicleOwner = e;
}

void VehicleTurretGun::SetRemoteOwner(Sentient *e)
{
    m_bUseRemoteControl = true;
    m_pRemoteOwner      = e;
}

void VehicleTurretGun::EventRemoveOnDeath(Event *ev)
{
    m_bRemoveOnDeath = ev->GetBoolean(1);
}

void VehicleTurretGun::UpdateSound(void)
{
    if (level.time < m_fNextSoundState) {
        return;
    }

    float fDiff = AngleSubtract(m_vTargetAngles[1], m_vLocalAngles[1]);

    switch (m_eSoundState) {
    case ST_OFF:
        StopLoopSound();
        m_fNextSoundState = level.time;
        if (fabs(fDiff) > 0.5f) {
            m_eSoundState = ST_OFF_TRANS_IDLE;
        }
        break;

    case ST_OFF_TRANS_IDLE:
        m_eSoundState     = ST_IDLE_TRANS_OFF;
        m_fNextSoundState = level.time;
        LoopSound(m_sSoundSet + "snd_move");
        break;

    case ST_IDLE_TRANS_OFF:
        m_fNextSoundState = level.time;
        if (fabs(fDiff) < 0.5f) {
            m_eSoundState = ST_IDLE;
        }

        LoopSound(m_sSoundSet + "snd_move");
        break;

    case ST_IDLE:
        m_eSoundState     = ST_OFF;
        m_fNextSoundState = level.time;
        LoopSound(m_sSoundSet + "snd_move");
        break;

    default:
        // FIXME: default sound?
        break;
    }
}

void VehicleTurretGun::EventSetCollisionModel(Event *ev)
{
    Entity *pColEnt = ev->GetEntity(1);

    if (!pColEnt) {
        ScriptError("Trying to set a collision model with a NULL entity.");
    }

    if (m_pCollisionEntity) {
        m_pCollisionEntity->PostEvent(EV_Remove, EV_VEHICLE);
    }

    m_pCollisionEntity = new VehicleCollisionEntity(this);
    m_pCollisionEntity->setModel(pColEnt->model);

    if (!m_pCollisionEntity->model.length() || *m_pCollisionEntity->model != '*') {
        // Re-post the event with the correct time
        m_pCollisionEntity->CancelEventsOfType(EV_Remove);
        m_pCollisionEntity->PostEvent(EV_Remove, EV_VEHICLE);
        m_pCollisionEntity = NULL;

        ScriptError("Model for Entity not of a valid type. Must be B-Model.");
    }

    m_pCollisionEntity->setOrigin(origin);
    m_pCollisionEntity->setAngles(angles);
}

void VehicleTurretGun::EventLock(Event *ev)
{
    Lock();
}

void VehicleTurretGun::EventUnlock(Event *ev)
{
    UnLock();
}

void VehicleTurretGun::SetSoundSet(Event *ev)
{
    m_sSoundSet = ev->GetString(1);
}

void VehicleTurretGun::UpdateOwner(Sentient *pOwner)
{
    // FIXME: unimplemented
    STUB();
}

bool VehicleTurretGun::TurretHasBeenMounted()
{
    // FIXME: unimplemented
    return false;
}

void VehicleTurretGun::PlaceTurret(Event *ev)
{
    setSolidType(SOLID_BBOX);
    edict->r.contents = CONTENTS_UNKNOWN2;
    setMoveType(MOVETYPE_NONE);
    showModel();
    groundentity = NULL;
    m_fStartYaw  = angles[1];
    flags |= FL_THINK;
    m_vLastBaseAngles = angles;
    m_vBaseAngles     = angles;

    if (m_vBaseAngles.length() != 0.0f) {
        AnglesToAxis(m_vBaseAngles, m_mBaseOrientation);
        m_bBOIsSet = true;
    }

    m_iPitchBone = gi.Tag_NumForName(edict->tiki, "pitch");
    SetControllerTag(0, m_iPitchBone);
    m_iBarrelTag = gi.Tag_NumForName(edict->tiki, "tag_barrel");
    m_iEyeBone   = gi.Tag_NumForName(edict->tiki, "eyebone");

    if (m_pRemoteOwner || owner) {
        if (m_pRemoteOwner) {
            UpdateOwner(m_pRemoteOwner);
        } else {
            UpdateOwner(owner);
        }
    }
}

void VehicleTurretGun::EventGetCollisionModel(Event *ev)
{
    ev->AddEntity(m_pCollisionEntity);
}

void VehicleTurretGun::EventTurnSpeed(Event *ev)
{
    AI_TurnSpeed(ev->GetFloat(1));
}

bool VehicleTurretGun::isLocked(void)
{
    return m_bLocked;
}

void VehicleTurretGun::Lock(void)
{
    m_bLocked = true;
}

void VehicleTurretGun::UnLock(void)
{
    m_bLocked = false;
}

bool VehicleTurretGun::UseRemoteControl(void)
{
    return m_bUseRemoteControl;
}

qboolean VehicleTurretGun::ReadyToFire(firemode_t mode, qboolean playsound)
{
    // FIXME: unimplemented
    return qfalse;
}

void VehicleTurretGun::AdjustReloadStatus()
{
    // FIXME: unimplemented
}

void VehicleTurretGun::GetMuzzlePosition(vec3_t position, vec3_t vBarrelPos, vec3_t forward, vec3_t right, vec3_t up)
{
    // FIXME: unimplemented
}

void VehicleTurretGun::ApplyFireKickback(const Vector& org, float kickback)
{
    // FIXME: unimplemented
}

float VehicleTurretGun::FireDelay(firemode_t mode)
{
    // FIXME: unimplemented
    return 0;
}

void VehicleTurretGun::SetWarmupDelay(Event *ev)
{
    // FIXME: unimplemented
}

void VehicleTurretGun::SetFireWarmupDelay(Event *ev)
{
    // FIXME: unimplemented
}

void VehicleTurretGun::SetReloadDelay(Event *ev)
{
    // FIXME: unimplemented
}

void VehicleTurretGun::SetReloadShots(Event *ev)
{
    // FIXME: unimplemented
}

void VehicleTurretGun::SetAimOffset(Event *ev)
{
    // FIXME: unimplemented
}

void VehicleTurretGun::SetAimTolerance(Event *ev)
{
    // FIXME: unimplemented
}

void VehicleTurretGun::SetTargetEntity(Event *ev)
{
    // FIXME: unimplemented
}

void VehicleTurretGun::PlayReloadSound(Event *ev)
{
    // FIXME: unimplemented
}

void VehicleTurretGun::SetTargetEntity(Entity *ent)
{
    // FIXME: unimplemented
}

void VehicleTurretGun::UpdateAndMoveOwner()
{
    // FIXME: unimplemented
}

void VehicleTurretGun::UpdateTimers(float& yawTimer, float& pitchTimer)
{
    // FIXME: unimplemented
}

void VehicleTurretGun::UpdateCaps(float maxYawOffset, float maxPitchOffset)
{
    // FIXME: unimplemented
}

void VehicleTurretGun::IdleToRestPosition()
{
    // FIXME: unimplemented
}

void VehicleTurretGun::UpdateFireControl()
{
    // FIXME: unimplemented
}

void VehicleTurretGun::UpdateCollisionEntity()
{
    // FIXME: unimplemented
}

void VehicleTurretGun::RestrictPitch()
{
    // FIXME: unimplemented
}

void VehicleTurretGun::RestrictYaw()
{
    // FIXME: unimplemented
}

void VehicleTurretGun::UpdateRemoteControl()
{
    // FIXME: unimplemented
}

void VehicleTurretGun::UpdateAimTarget()
{
    // FIXME: unimplemented
}

Entity *VehicleTurretGun::GetParent() const
{
    // FIXME: unimplemented
    return NULL;
}

SentientPtr VehicleTurretGun::GetRemoteOwner(void)
{
    return m_pRemoteOwner;
}

SentientPtr VehicleTurretGun::GetSentientOwner()
{
    // FIXME: unimplemented
    return m_pRemoteOwner;
}

void VehicleTurretGun::EndRemoteControl()
{
    // FIXME: unimplemented
}

float VehicleTurretGun::GetWarmupFraction() const
{
    // FIXME: unimplemented
    return 0;
}

bool VehicleTurretGun::IsRemoteControlled()
{
    // FIXME: unimplemented
    return false;
}

void VehicleTurretGun::Archive(Archiver& arc)
{
    // FIXME: unimplemented
}

EntityPtr VehicleTurretGun::GetVehicle() const
{
    // FIXME: unimplemented
    return NULL;
}

Event EV_VTGP_LinkTurret
(
    "linkturret",
    EV_DEFAULT,
    "s",
    "name",
    "Sets the next turret in the link.",
    EV_NORMAL
);
Event EV_VTGP_SetSwitchThread
(
    "setswitchthread",
    EV_DEFAULT,
    "s",
    "thread",
    "Set the thread to execute when turret is switched",
    EV_NORMAL
);

CLASS_DECLARATION(VehicleTurretGun, VehicleTurretGunTandem, "VehicleTurretGunTandem") {
    {&EV_VTGP_LinkTurret,      &VehicleTurretGunTandem::EventLinkTurret     },
    {&EV_VTGP_SetSwitchThread, &VehicleTurretGunTandem::EventSetSwitchThread},
    {NULL,                     NULL                                         }
};

VehicleTurretGunTandem::VehicleTurretGunTandem()
{
    m_Slot.ent             = NULL;
    m_Slot.flags           = SLOT_FREE;
    m_Slot.boneindex       = -1;
    m_Slot.enter_boneindex = -1;

    m_PrimaryTurret = NULL;
    m_HeadTurret    = NULL;
    m_ActiveTurret  = NULL;

    m_fSwitchTimeRemaining = 0;
    // 1 second switch delay
    m_fSwitchDelay = 1;
}

VehicleTurretGunTandem::~VehicleTurretGunTandem() {}

void VehicleTurretGunTandem::EventLinkTurret(Event *ev)
{
    VehicleTurretGunTandem *linkedTurret;

    linkedTurret = new VehicleTurretGunTandem();
    linkedTurret->SetBaseOrientation(orientation, NULL);
    linkedTurret->setModel(ev->GetString(1));

    AttachLinkedTurret(linkedTurret);
    UpdateLinkedTurret();
}

void VehicleTurretGunTandem::AttachLinkedTurret(Entity *ent)
{
    if (!ent) {
        return;
    }

    OpenSlotsByModel();
    m_Slot.ent      = ent;
    m_Slot.flags    = SLOT_BUSY;
    ent->takedamage = DAMAGE_NO;
    ent->PostEvent(EV_BecomeNonSolid, level.frametime);

    // process every end of frame
    flags |= FL_POSTTHINK;

    m_Slot.ent->setAngles(angles);

    if (m_PrimaryTurret) {
        VehicleTurretGunTandem *pTurret = static_cast<VehicleTurretGunTandem *>(m_Slot.ent.Pointer());
        pTurret->SetPrimaryTurret(m_PrimaryTurret);
    } else {
        VehicleTurretGunTandem *pTurret = static_cast<VehicleTurretGunTandem *>(m_Slot.ent.Pointer());
        pTurret->SetPrimaryTurret(this);
    }
}

void VehicleTurretGunTandem::UpdateLinkedTurret()
{
    VehicleTurretGun *pTurret;
    orientation_t     tag_or;

    if (!(m_Slot.flags & SLOT_BUSY) || !m_Slot.ent) {
        return;
    }
    if (m_Slot.boneindex == -1) {
        m_Slot.ent->setOrigin(origin);
        m_Slot.ent->avelocity = avelocity;
        m_Slot.ent->velocity  = velocity;

        pTurret = static_cast<VehicleTurretGun *>(m_Slot.ent.Pointer());
        pTurret->SetBaseOrientation(orientation, NULL);
    } else {
        GetTag(m_Slot.boneindex, &tag_or);

        m_Slot.ent->setOrigin(tag_or.origin);
        m_Slot.ent->avelocity = avelocity;
        m_Slot.ent->velocity  = velocity;

        pTurret = static_cast<VehicleTurretGun *>(m_Slot.ent.Pointer());
        pTurret->SetBaseOrientation(tag_or.axis, NULL);
    }
}

void VehicleTurretGunTandem::OpenSlotsByModel()
{
    int tagnum;

    tagnum = gi.Tag_NumForName(edict->tiki, "turret0");
    if (tagnum >= 0) {
        m_Slot.boneindex = tagnum;
        m_Slot.ent       = NULL;
        m_Slot.flags     = SLOT_FREE;
    }
}

void VehicleTurretGunTandem::Think()
{
    if (m_PrimaryTurret) {
        ThinkSecondary();
    } else {
        ThinkPrimary();
    }
}

bool VehicleTurretGunTandem::IsRemoteControlled()
{
    if (m_PrimaryTurret) {
        return m_PrimaryTurret->m_bUseRemoteControl;
    } else {
        return m_bUseRemoteControl;
    }
}

SentientPtr VehicleTurretGunTandem::GetRemoteOwner()
{
    if (m_PrimaryTurret) {
        return m_PrimaryTurret->m_pRemoteOwner;
    } else {
        return m_pRemoteOwner;
    }
}

void VehicleTurretGunTandem::ThinkSecondary()
{
    float     yawOffset, pitchOffset;
    Sentient *sentOwner;

    if (g_gametype->integer == GT_SINGLE_PLAYER && m_PrimaryTurret->edict->s.renderfx & RF_DEPTHHACK) {
        edict->s.renderfx |= RF_DEPTHHACK;
    }

    if (m_pCollisionEntity) {
        m_pCollisionEntity->NotSolid();
    }

    UpdateTimers(yawOffset, pitchOffset);

    if (IsRemoteControlled() && IsActiveTurret()) {
        UpdateRemoteControl();
        UpdateCaps(yawOffset, pitchOffset);
    } else if (m_bRestable) {
        IdleToRestPosition();
    }

    UpdateOrientation(false);
    UpdateSound();
    UpdateFireControl();

    sentOwner = GetSentientOwner();
    if (IsActiveTurret() && sentOwner) {
        G_TouchTriggers(sentOwner);
        UpdateOwner(sentOwner);
    }

    UpdateCollisionEntity();
}

void VehicleTurretGunTandem::ThinkPrimary()
{
    float     yawOffset, pitchOffset;
    Sentient *sentOwner;

    if (m_fSwitchTimeRemaining > 0) {
        m_fSwitchTimeRemaining -= level.frametime;
    }

    if (m_ActiveTurret) {
        if (m_ActiveTurret != m_HeadTurret) {
            m_fSwitchTimeRemaining = m_fSwitchDelay;
            m_HeadTurret->m_pUserCamera->PostEvent(EV_Remove, 0);
            m_HeadTurret->m_pUserCamera = NULL;

            // switch to the active turret
            m_HeadTurret   = m_ActiveTurret;
            m_ActiveTurret = NULL;

            // clear angles
            m_vUserLastCmdAng               = vec_zero;
            m_HeadTurret->m_vUserLastCmdAng = vec_zero;
            m_vTargetAngles                 = m_vLocalAngles;
        }
    }

    if (m_pCollisionEntity) {
        m_pCollisionEntity->NotSolid();
    }

    if (g_gametype->integer == GT_SINGLE_PLAYER && m_pRemoteOwner && m_pRemoteOwner->isSubclassOf(Player)) {
        // always render the turret
        edict->s.renderfx |= RF_DEPTHHACK;
    } else {
        edict->s.renderfx &= ~RF_DEPTHHACK;
    }

    UpdateTimers(yawOffset, pitchOffset);

    if (IsRemoteControlled()) {
        UpdateRemoteControl();
        UpdateCaps(yawOffset, pitchOffset);
    } else if (m_bRestable) {
        IdleToRestPosition();
    }

    UpdateOrientation(false);
    UpdateSound();
    UpdateFireControl();

    sentOwner = GetSentientOwner();
    if (IsActiveTurret() && sentOwner) {
        G_TouchTriggers(sentOwner);
        UpdateOwner(sentOwner);
    }

    UpdateCollisionEntity();
}

void VehicleTurretGunTandem::SetPrimaryTurret(VehicleTurretGunTandem *pTurret)
{
    m_PrimaryTurret = pTurret;
    if (m_PrimaryTurret) {
        m_HeadTurret = NULL;
    }
}

void VehicleTurretGunTandem::RemoteControl(usercmd_t *ucmd, Sentient *owner)
{
    Vector vNewCmdAng;

    if (!ucmd || !owner) {
        return;
    }

    if (m_HeadTurret && m_HeadTurret != this) {
        m_HeadTurret->RemoteControlSecondary(ucmd, owner);
        return;
    }

    vNewCmdAng = Vector(SHORT2ANGLE(ucmd->angles[0]), SHORT2ANGLE(ucmd->angles[1]), SHORT2ANGLE(ucmd->angles[2]));

    if (vNewCmdAng[0] || vNewCmdAng[1] || vNewCmdAng[2]) {
        m_vUserViewAng[0] += AngleSubtract(vNewCmdAng[0], m_vUserLastCmdAng[0]);
        m_vUserViewAng[1] += AngleSubtract(vNewCmdAng[1], m_vUserLastCmdAng[1]);
        m_vUserViewAng[2] += AngleSubtract(vNewCmdAng[2], m_vUserLastCmdAng[2]);
    }

    m_vUserLastCmdAng = vNewCmdAng;

    RemoteControlFire(ucmd, owner);
}

void VehicleTurretGunTandem::RemoteControlSecondary(usercmd_t *ucmd, Sentient *owner)
{
    Vector vNewCmdAng;

    if (!ucmd || !owner) {
        return;
    }

    vNewCmdAng = Vector(SHORT2ANGLE(ucmd->angles[0]), SHORT2ANGLE(ucmd->angles[1]), SHORT2ANGLE(ucmd->angles[2]));

    if (vNewCmdAng[0] || vNewCmdAng[1] || vNewCmdAng[2]) {
        m_vUserViewAng[0] += AngleSubtract(vNewCmdAng[0], m_vUserLastCmdAng[0]);
        m_vUserViewAng[1] += AngleSubtract(vNewCmdAng[1], m_vUserLastCmdAng[1]);
        m_vUserViewAng[2] += AngleSubtract(vNewCmdAng[2], m_vUserLastCmdAng[2]);
    }

    m_vUserLastCmdAng = vNewCmdAng;

    RemoteControlFire(ucmd, owner);
}

void VehicleTurretGunTandem::RemoteControlFire(usercmd_t *ucmd, Sentient *owner)
{
    if (ucmd->buttons & BUTTON_ATTACKLEFT) {
        if (!m_iFiring) {
            m_iFiring = 1;
        }
    } else {
        if (ucmd->buttons & BUTTON_ATTACKRIGHT) {
            SwitchToLinkedTurret();
        }
        m_iFiring = 0;
    }

    flags |= FL_THINK;
}

void VehicleTurretGunTandem::EventSetSwitchThread(Event *ev)
{
    if (ev->IsFromScript()) {
        m_SwitchLabel.SetThread(ev->GetValue(1));
    } else {
        m_SwitchLabel.Set(ev->GetString(1));
    }
}

void VehicleTurretGunTandem::SwitchToLinkedTurret()
{
    VehicleTurretGunTandem *pTurret;

    if (GetPrimaryTurret()->m_fSwitchTimeRemaining > 0) {
        if (m_Slot.ent) {
            pTurret = static_cast<VehicleTurretGunTandem *>(m_Slot.ent.Pointer());
            m_SwitchLabel.Execute(pTurret, NULL);
        } else {
            pTurret = m_PrimaryTurret;
            m_PrimaryTurret->m_SwitchLabel.Execute(pTurret, NULL);
        }

        SetActiveTurret(pTurret);
    }
}

void VehicleTurretGunTandem::SetActiveTurret(VehicleTurretGunTandem *pTurret)
{
    if (m_PrimaryTurret) {
        if (!m_PrimaryTurret->m_ActiveTurret) {
            m_PrimaryTurret->m_ActiveTurret = pTurret;
        }
    } else if (!m_ActiveTurret) {
        m_ActiveTurret = pTurret;
    }
}

void VehicleTurretGunTandem::RestrictYaw()
{
    float delta;

    delta = AngleSubtract(m_vUserViewAng[1], m_fStartYaw);
    if (delta > m_fMaxYawOffset + MAX_TANDEM_YAWOFFSET) {
        delta = m_fMaxYawOffset + MAX_TANDEM_YAWOFFSET;
    } else if (delta < -(m_fMaxYawOffset + MAX_TANDEM_YAWOFFSET)) {
        delta = -(m_fMaxYawOffset + MAX_TANDEM_YAWOFFSET);
    }

    m_vUserViewAng[1] = m_fStartYaw + delta;
}

void VehicleTurretGunTandem::Archive(Archiver& arc)
{
    VehicleTurretGun::Archive(arc);

    m_Slot.Archive(arc);
    arc.ArchiveSafePointer(&m_PrimaryTurret);
    arc.ArchiveSafePointer(&m_HeadTurret);
    arc.ArchiveSafePointer(&m_ActiveTurret);
    arc.ArchiveFloat(&m_fSwitchTimeRemaining);
    arc.ArchiveFloat(&m_fSwitchDelay);
}

VehicleTurretGunTandem *VehicleTurretGunTandem::GetPrimaryTurret()
{
    if (m_PrimaryTurret) {
        return m_PrimaryTurret;
    } else {
        return this;
    }
}

bool VehicleTurretGunTandem::IsActiveTurret() const
{
    if (!m_PrimaryTurret) {
        return m_HeadTurret;
    }

    if (m_PrimaryTurret == this) {
        return true;
    }

    return false;
}
