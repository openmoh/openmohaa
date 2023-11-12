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

static constexpr float MAX_TANDEM_YAW_OFFSET  = 0;
static constexpr float MAX_VT_PITCHCAP_OFFSET = 0;
static constexpr float MAX_VT_YAW_OFFSET      = 0;

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
    {&EV_VehicleTurretGun_Lock,                  &VehicleTurretGun::EventLock             },
    {&EV_VehicleTurretGun_Unlock,                &VehicleTurretGun::EventUnlock           },
    {&EV_VehicleTurretGun_SoundSet,              &VehicleTurretGun::SetSoundSet           },
    {&EV_VehicleTurretGun_CollisionEntitySetter, &VehicleTurretGun::EventSetCollisionModel},
    {&EV_VehicleTurretGun_CollisionEntityGetter, &VehicleTurretGun::EventGetCollisionModel},
    {&EV_VehicleTurretGun_WarmupDelay,           &VehicleTurretGun::SetWarmupDelay        },
    {&EV_VehicleTurretGun_FireWarmupDelay,       &VehicleTurretGun::SetFireWarmupDelay    },
    {&EV_VehicleTurretGun_ReloadShots,           &VehicleTurretGun::SetReloadShots        },
    {&EV_VehicleTurretGun_ReloadDelay,           &VehicleTurretGun::SetReloadDelay        },
    {&EV_VehicleTurretGun_AimOffset,             &VehicleTurretGun::SetAimOffset          },
    {&EV_VehicleTurretGun_AimTolerance,          &VehicleTurretGun::SetAimTolerance       },
    {&EV_VehicleTurretGun_SetTargetEntity,       &VehicleTurretGun::SetTargetEntity       },
    {&EV_VehicleTurretGun_PlayReloadSound,       &VehicleTurretGun::PlayReloadSound       },
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

    respondto       = TRIGGER_PLAYERS | TRIGGER_MONSTERS;
    edict->clipmask = MASK_VEHICLETURRET;

    m_bUsable         = true;
    m_bPlayerUsable   = true;
    m_bRestable       = false;
    m_fIdlePitchSpeed = 0;
    m_iIdleHitCount   = 0;

    takedamage = DAMAGE_NO;
    health     = 100.0f;
    max_health = 100.0f;

    setSize(Vector(-16, -16, 0), Vector(16, 16, 32));

    m_fPitchUpCap   = -45;
    m_fPitchDownCap = 45;
    m_fMaxYawOffset = 180;
    m_fTurnSpeed    = 160;
    m_fAIPitchSpeed = 48;
    m_fUserDistance = 64.0f;
    m_vIdleCheckOffset.setXYZ(-56, 0, 0);

    m_fMinBurstTime  = 0;
    m_fMaxBurstTime  = 0;
    m_fMinBurstDelay = 0;
    m_fMaxBurstDelay = 0;

    AxisClear(m_mBaseOrientation);
    m_fFireToggleTime = 0;
    m_iFiring         = 0;
    m_bBOIsSet        = false;

    m_pBaseEntity     = NULL;
    m_vLastBaseAngles = vec_zero;
    m_vBaseAngles     = vec_zero;

    m_vBarrelPos     = origin;
    m_vLastBarrelPos = origin;

    m_bUseRemoteControl = false;
    m_pVehicleOwner     = NULL;

    m_bRemoveOnDeath  = true;
    m_eSoundState     = STT_OFF;
    m_fNextSoundState = level.time;

    m_fWarmupDelay         = 0;
    m_fFireWarmupDelay     = 0;
    m_fTargetReloadTime    = 0;
    m_fWarmupTimeRemaining = 0;
    m_fReloadDelay         = 0;
    m_fReloadTimeRemaining = 0;

    m_iReloadShots = 1;
    m_iAmmo        = 1;

    ammo_in_clip[FIRE_PRIMARY]   = m_iAmmo;
    ammo_clip_size[FIRE_PRIMARY] = m_iReloadShots;

    m_vAimOffset[0]    = 0;
    m_vAimOffset[1]    = 0;
    m_vAimOffset[2]    = 0;
    m_vAimTolerance[0] = 20;
    m_vAimTolerance[1] = 20;

    m_bLocked    = true;
    m_bLockedAim = false;
    m_sSoundSet  = "";
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
    Sentient *sentOwner;
    float     yawOffset, pitchOffset;

    if (m_pCollisionEntity) {
        m_pCollisionEntity->NotSolid();
    }

    sentOwner = GetSentientOwner();

    if (g_gametype->integer == GT_SINGLE_PLAYER && m_pRemoteOwner && m_pRemoteOwner->isSubclassOf(Player)) {
        // always render the turret
        edict->s.renderfx |= RF_DEPTHHACK;
    } else {
        edict->s.renderfx &= ~RF_DEPTHHACK;
    }

    UpdateTimers(yawOffset, pitchOffset);

    if (owner) {
        UpdateAndMoveOwner();
        UpdateCaps(yawOffset, pitchOffset);
    } else if (m_bUseRemoteControl) {
        UpdateRemoteControl();
        UpdateCaps(yawOffset, pitchOffset);
    } else if (aim_target) {
        UpdateAimTarget();
        UpdateCaps(yawOffset, pitchOffset);
    } else if (m_bRestable) {
        IdleToRestPosition();
    }

    UpdateOrientation(false);
    UpdateSound();
    UpdateFireControl();

    sentOwner = GetSentientOwner();
    if (sentOwner) {
        G_TouchTriggers(sentOwner);
        UpdateOwner(sentOwner);
    }

    UpdateCollisionEntity();
}

void VehicleTurretGun::P_UserAim(usercmd_t *ucmd)
{
    Vehicle *pVehicle;
    int      slot;
    int      newSlot;

    TurretGun::P_UserAim(ucmd);

    if (ucmd->buttons & BUTTON_ATTACKRIGHT) {
        m_bLockedAim = false;
        return;
    }

    if (m_bLockedAim) {
        return;
    }

    if (m_iFiring) {
        m_bLockedAim = false;
        return;
    }

    if (!m_pVehicleOwner->IsSubclassOfVehicle()) {
        return;
    }

    pVehicle = static_cast<Vehicle *>(m_pVehicleOwner.Pointer());
    slot     = pVehicle->FindTurretSlotByEntity(this);
    newSlot  = slot + 1;
    if (newSlot == pVehicle->numTurrets) {
        newSlot = 0;
    }

    if (newSlot != slot) {
        Entity *slotEnt;

        slotEnt = pVehicle->QueryTurretSlotEntity(newSlot);
        if (slotEnt->IsSubclassOfVehicleTurretGun()) {
            VehicleTurretGun *existing;
            Vector            newAng;
            bool              wasThisLocked, wasExistingLocked;

            existing = static_cast<VehicleTurretGun *>(slotEnt);

            wasThisLocked       = m_bLocked;
            wasExistingLocked   = existing->m_bLocked;
            m_bLocked           = false;
            existing->m_bLocked = false;

            newAng            = existing->m_vUserViewAng;
            m_vUserLastCmdAng = vec_zero;

            pVehicle->AttachTurretSlot(newSlot, owner, vec_zero, NULL);
            existing->m_vUserViewAng = newAng;

            owner             = NULL;
            edict->r.ownerNum = ENTITYNUM_NONE;

            if (wasThisLocked) {
                m_bLocked = true;
            }
            if (wasExistingLocked) {
                existing->m_bLocked = true;
            }

            existing->m_bLockedAim = true;
            m_bLockedAim           = false;
        }
    }
}

void VehicleTurretGun::TurretBeginUsed(Sentient *pEnt)
{
    if (m_pVehicleOwner) {
        Vehicle *pVehicle = (Vehicle *)m_pVehicleOwner.Pointer();
        int      slot     = pVehicle->FindTurretSlotByEntity(this);

        pVehicle->UpdateTurretSlot(slot);
        m_vBaseAngles = m_pVehicleOwner->angles;
    }

    owner             = pEnt;
    edict->r.ownerNum = pEnt->entnum;
    m_bHadOwner       = true;

    Sound(sPickupSound);

    owner->SetViewAngles(m_vBaseAngles);

    m_vUserViewAng    = m_vBaseAngles;
    m_vUserViewAng[0] = AngleNormalize180(m_vUserViewAng[0]);
    m_vUserLastCmdAng.setXYZ(0, 0, 0);

    setAngles(m_vBaseAngles);

    if (owner->IsSubclassOfPlayer()) {
        Player *player = (Player *)owner.Pointer();

        player->EnterTurret(this);

        if (!m_pUserCamera) {
            m_pUserCamera = new Camera;
        }

        player->SetCamera(m_pUserCamera, 0.5);
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

        RemoveUserCamera();
        player->ExitTurret();
        P_DeleteViewModel();
    }

    owner             = NULL;
    edict->r.ownerNum = ENTITYNUM_NONE;
    m_fIdlePitchSpeed = 0;
    m_iIdleHitCount   = 0;
    m_iFiring         = 0;
    m_vTargetAngles   = m_vLocalAngles;
}

void VehicleTurretGun::TurretUsed(Sentient *pEnt)
{
    if (!owner) {
        TurretBeginUsed(pEnt);
        return;
    }

    if (owner == pEnt) {
        TurretEndUsed();
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

void VehicleTurretGun::SetBaseOrientation(const vec3_t borientation[3], const vec3_t bangles)
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

    flags |= FL_THINK;
}

void VehicleTurretGun::RemoteControl(usercmd_t *ucmd, Sentient *owner)
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

    if (!m_bUseRemoteControl) {
        m_bUseRemoteControl = true;
        m_pRemoteOwner      = owner;
        m_vUserViewAng      = m_vLocalAngles;

        GetRemoteOwner()->SetViewAngles(m_vUserViewAng + m_vBaseAngles);
    }

    if (ucmd->buttons & (BUTTON_ATTACKLEFT | BUTTON_ATTACKRIGHT)) {
        if (!m_iFiring) {
            m_iFiring = 1;
        }
    } else {
        m_iFiring           = 0;
        m_fTargetReloadTime = 0;
        flags |= FL_THINK;
    }
}

void VehicleTurretGun::UpdateOrientation(bool bCollisionCheck)
{
    Entity *ent;
    Vector  localAngles;
    vec3_t  mat[3];

    if (m_pBaseEntity) {
        ent = m_pBaseEntity;
    } else if (edict->s.parent) {
        ent = G_GetEntity(edict->s.parent);
    } else {
        ent = NULL;
    }

    m_iPitchBone = gi.Tag_NumForName(edict->tiki, "pitch");
    m_iBarrelTag = gi.Tag_NumForName(edict->tiki, GetTagBarrel());
    m_iEyeBone   = gi.Tag_NumForName(edict->tiki, "eyebone");

    if (m_iBarrelTag >= 0) {
        orientation_t barrel_or;

        GetTagPositionAndOrientation(m_iBarrelTag, &barrel_or);
        m_vLastBarrelPos = m_vBarrelPos;
        m_vBarrelPos     = barrel_or.origin;
    }

    if (bCollisionCheck && m_iBarrelTag >= 0) {
        trace_t trace;
        // check if the new barrel position is ok
        trace = G_Trace(
            m_vLastBarrelPos,
            Vector(8, 8, 8),
            Vector(-8, -8, -8),
            m_vBarrelPos,
            this,
            edict->clipmask,
            qfalse,
            "VehicleTurretGun::Think.BarrelCheck"
        );

        if (trace.fraction == 1 || trace.startsolid || trace.allsolid) {
            CollisionCorrect(&trace);
        }
    }

    localAngles = m_vLocalAngles;

    if (m_iPitchBone >= 0) {
        vec3_t controllerAngles = {localAngles[0], 0, 0};
        SetControllerAngles(0, controllerAngles);
        localAngles[0] = 0;
    }

    AnglesToAxis(localAngles, mat);

    if (m_bBOIsSet) {
        vec3_t BOmat[3];

        MatrixMultiply(mat, m_mBaseOrientation, BOmat);
        MatrixToEulerAngles(BOmat, angles);
        setAngles(angles);
    } else {
        setAngles(localAngles);
    }
}

void VehicleTurretGun::CollisionCorrect(trace_t *pTr)
{
    float planedot;

    if (pTr->plane.normal == vec_zero) {
        return;
    }

    if (pTr->plane.normal[2]) {
        if (pTr->plane.normal[2] > 0) {
            m_vLocalAngles[0] -= m_fAIPitchSpeed * level.frametime;
        } else {
            m_vLocalAngles[0] += m_fAIPitchSpeed * level.frametime;
        }
    }

    planedot = DotProduct(pTr->plane.normal, orientation[1]);
    if (fabs(planedot) > 0.25f) {
        if (planedot > 0) {
            m_vLocalAngles[1] += m_fTurnSpeed * level.frametime;
        } else {
            m_vLocalAngles[1] -= m_fTurnSpeed * level.frametime;
        }
    }
}

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
    if (g_gametype->integer == GT_TOW && !dmManager.RoundActive()) {
        return;
    }

    if (owner) {
        owner->ProcessEvent(*ev);
        return;
    }

    if (m_pVehicleOwner) {
        m_pVehicleOwner->ProcessEvent(*ev);
        return;
    }

    if (!m_bUseRemoteControl) {
        meansOfDeath_t mod = static_cast<meansOfDeath_t>(ev->GetInteger(9));
        switch (mod) {
        case MOD_BULLET:
        case MOD_BASH:
        case MOD_FAST_BULLET:
        case MOD_VEHICLE:
        case MOD_SHOTGUN:
            return;
        default:
            break;
        }

        DamageEvent(ev);
        return;
    }

    if (m_pRemoteOwner && m_pRemoteOwner->IsSubclassOfSentient()) {
        Sentient *sent = static_cast<Sentient *>(m_pRemoteOwner.Pointer());

        if (sent->GetVehicle()) {
            sent->GetVehicle()->ProcessEvent(*ev);
            return;
        }
    }
}

void VehicleTurretGun::SetVehicleOwner(Entity *e)
{
    m_pVehicleOwner = e;
}

void VehicleTurretGun::EventRemoveOnDeath(Event *ev)
{
    m_bRemoveOnDeath = ev->GetBoolean(1);
}

void VehicleTurretGun::UpdateSound(void)
{
    float fPitchDiff, fYawDiff, fDiff;

    if (level.time < m_fNextSoundState) {
        return;
    }

    fYawDiff   = fabs(AngleSubtract(m_vTargetAngles[1], m_vLocalAngles[1]));
    fPitchDiff = fabs(AngleSubtract(m_vTargetAngles[0], m_vLocalAngles[0]));
    if (fYawDiff > fPitchDiff) {
        fDiff = fYawDiff;
    } else {
        fDiff = fPitchDiff;
    }

    if (!owner && !m_bUseRemoteControl && !aim_target && !m_bRestable) {
        fDiff = 0;
    }

    switch (m_eSoundState) {
    case STT_OFF:
        StopLoopSound();
        m_fNextSoundState = level.time;
        if (fabs(fDiff) > 0.5f) {
            m_eSoundState = STT_OFF_TRANS_MOVING;
        }
        break;

    case STT_OFF_TRANS_MOVING:
        m_fNextSoundState = level.time;
        m_eSoundState     = STT_OFF;
        break;

    case STT_MOVING:
        m_fNextSoundState = level.time;
        if (fDiff < 0.5) {
            m_eSoundState = STT_MOVING_TRANS_OFF;
        }
        LoopSound(m_sSoundSet + "snd_move");
        break;

    case STT_MOVING_TRANS_OFF:
        m_fNextSoundState = level.time;
        m_eSoundState     = STT_OFF;
        Sound(m_sSoundSet + "snd_move_stop");
        break;

    default:
        m_fNextSoundState = level.time;
        m_eSoundState     = STT_OFF;
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
        m_pCollisionEntity->PostEvent(EV_Remove, 0);
    }

    m_pCollisionEntity = new VehicleCollisionEntity(this);
    if (m_pCollisionEntity) {
        m_pCollisionEntity->setModel(pColEnt->model);
        m_pCollisionEntity->setOrigin(origin);
        m_pCollisionEntity->setAngles(angles);

        if (m_pCollisionEntity->model.length() && *m_pCollisionEntity->model == '*') {
            UpdateCollisionEntity();
            m_pCollisionEntity->DisconnectPaths();
        } else {
            // Re-post the event with the correct time
            m_pCollisionEntity->CancelEventsOfType(EV_Remove);
            m_pCollisionEntity->PostEvent(EV_Remove, EV_VEHICLE);
            m_pCollisionEntity = NULL;

            ScriptError("Model for Entity not of a valid type. Must be B-Model.");
        }
    }
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
    Vector        jitter;
    Vector        position;
    Vector        forward, left, up;
    orientation_t bone_or;

    if (m_iEyeBone >= 0) {
        GetRawTag(m_iEyeBone, &bone_or);
        angles.AngleVectorsLeft(&forward, &left, &up);

        position = origin + bone_or.origin[0] * forward + bone_or.origin[1] * left + bone_or.origin[2] * up;
    } else {
        (m_vBaseAngles + m_vLocalAngles).AngleVectorsLeft(&forward, &left, &up);

        position = origin + forward * -16 + up * 5;

        if (m_iPitchBone >= 0) {
            GetRawTag(m_iPitchBone, &bone_or);
            angles.AngleVectorsLeft(&forward, &left, &up);

            position += bone_or.origin[0] * forward + bone_or.origin[1] * left + bone_or.origin[2] * up;
        } else {
            angles.AngleVectorsLeft(&forward, NULL, &up);
            position += up * 40;
        }
    }

    jitter = m_vUserViewAng + m_vBaseAngles;
    P_ApplyFiringViewJitter(jitter);

    if (!m_pUserCamera && pOwner->IsSubclassOfPlayer()) {
        Player *player = static_cast<Player *>(pOwner);

        m_pUserCamera = new Camera();
        player->SetCamera(m_pUserCamera, 0.5);
    }

    if (owner) {
        jitter[1] = m_vUserViewAng[1];
    }

    m_pUserCamera->setOrigin(position);
    m_pUserCamera->setAngles(jitter);

    if (GetTag("tag_seat", &bone_or)) {
        vec3_t ang;

        pOwner->setOrigin(bone_or.origin);
        MatrixToEulerAngles(bone_or.axis, ang);
        pOwner->setAngles(ang);
    } else {
        // no seat
        pOwner->setOrigin(origin);
    }

    if (pOwner->IsSubclassOfPlayer()) {
        Player *player = static_cast<Player *>(pOwner);

        if (!player->IsZoomed()) {
            player->ToggleZoom(80);
        }

        player->client->ps.camera_flags |= CF_CAMERA_ANGLES_TURRETMODE;
    }
}

void VehicleTurretGun::TurretHasBeenMounted()
{
    m_fLastFireTime = level.time + m_fWarmupDelay + fire_delay[FIRE_PRIMARY];
    if (m_fWarmupDelay > 0.25) {
        Sound(m_sSoundSet + "snd_warmup");
    }

    m_fWarmupTimeRemaining = m_fWarmupDelay;
}

void VehicleTurretGun::PlaceTurret(Event *ev)
{
    setSolidType(SOLID_BBOX);
    setContents(CONTENTS_UNKNOWN2);
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
    m_iBarrelTag = gi.Tag_NumForName(edict->tiki, GetTagBarrel());
    m_iEyeBone   = gi.Tag_NumForName(edict->tiki, "eyebone");

    if (m_pRemoteOwner) {
        UpdateOwner(m_pRemoteOwner);
    } else if (owner) {
        UpdateOwner(owner);
    }
}

void VehicleTurretGun::EventTurnSpeed(Event *ev)
{
    AI_TurnSpeed(ev->GetFloat(1));
}

bool VehicleTurretGun::UseRemoteControl(void)
{
    return m_bUseRemoteControl;
}

qboolean VehicleTurretGun::ReadyToFire(firemode_t mode, qboolean playsound)
{
    if (!use_no_ammo) {
        return Weapon::ReadyToFire(mode, playsound);
    }

    if (m_fReloadDelay <= 0) {
        return Weapon::ReadyToFire(mode, playsound);
    }

    if (m_fReloadTimeRemaining <= 0) {
        return Weapon::ReadyToFire(mode, playsound);
    }

    return qfalse;
}

void VehicleTurretGun::AdjustReloadStatus()
{
    if (!use_no_ammo) {
        return;
    }

    if (m_fReloadDelay <= 0) {
        return;
    }

    m_iAmmo--;
    ammo_in_clip[FIRE_PRIMARY] = m_iAmmo;
    if (!m_iAmmo) {
        m_iAmmo                = m_iReloadShots;
        m_fReloadTimeRemaining = m_fReloadDelay;

        PostEvent(EV_VehicleTurretGun_PlayReloadSound, m_fReloadDelay * 0.5);
    }
}

void VehicleTurretGun::GetMuzzlePosition(vec3_t position, vec3_t vBarrelPos, vec3_t forward, vec3_t right, vec3_t up)
{
    Vector        delta;
    Vector        aim_angles;
    Sentient     *viewer;
    orientation_t barrel_or;
    vec3_t        weap_axis[3];
    float         mat[3][3];
    int           i;

    viewer = owner;
    if (!owner) {
        viewer = GetRemoteOwner();
    }

    if (!viewer) {
        if (forward || right || up) {
            if (!m_bUseRemoteControl && aim_target) {
                Vector dir;
                Vector vAng;
                Vector vFwd;

                AngleVectors(angles, vFwd, right, up);
                delta = aim_target->origin;

                if (aim_target->HasVehicle()) {
                    delta.z -= 60;
                }

                dir = delta - origin;
                VectorNormalize(dir);
                dir.x = vFwd.x;
                dir.y = vFwd.y;
                VectorNormalize(dir);
                VectorToAngles(dir, vAng);
                AngleVectors(vAng, forward, right, up);
            } else {
                AngleVectors(angles, forward, right, up);
            }
        }

        VectorCopy(origin, position);

        if (GetRawTag(GetTagBarrel(), &barrel_or)) {
            AnglesToAxis(angles, weap_axis);

            for (i = 0; i < 3; i++) {
                VectorMA(position, barrel_or.origin[i], weap_axis[i], position);
            }
        }

        if (vBarrelPos) {
            VectorCopy(position, vBarrelPos);
        }
    } else if (viewer->IsSubclassOfPlayer()) {
        VectorCopy(origin, position);

        if (GetRawTag(GetTagBarrel(), &barrel_or)) {
            AnglesToAxis(angles, weap_axis);

            for (i = 0; i < 3; i++) {
                VectorMA(position, barrel_or.origin[i], weap_axis[i], position);
            }
        }

        if (vBarrelPos) {
            VectorCopy(position, vBarrelPos);
        }

        delta      = viewer->GunTarget(position) - position;
        aim_angles = delta.toAngles();

        if (IsSubclassOfVehicleTurretGun()) {
            vec3_t ang;

            MatrixMultiply(barrel_or.axis, weap_axis, mat);
            vectoangles(mat[0], ang);

            for (i = 0; i < 2; i++) {
                float diff;

                diff = abs(AngleSubtract(aim_angles[i], ang[i]));
                if (diff > m_vAimTolerance[i]) {
                    aim_angles[i] = m_vAimOffset[i] + ang[i];
                }
            }
        }

        if (forward || right || up) {
            AngleVectors(aim_angles, forward, right, up);
        }
    } else {
        TurretGun::GetMuzzlePosition(position, vBarrelPos, forward, right, up);
    }
}

void VehicleTurretGun::ApplyFireKickback(const Vector& org, float kickback)
{
    Vehicle *pVehicle;

    if (!m_pVehicleOwner || !m_pVehicleOwner->IsSubclassOfVehicle()) {
        return;
    }

    pVehicle = static_cast<Vehicle *>(m_pVehicleOwner.Pointer());
    pVehicle->m_fForwardForce += org.x * kickback;
    pVehicle->m_fLeftForce += org.y * kickback;
}

void VehicleTurretGun::SetRemoteOwner(Sentient *e)
{
    m_bUseRemoteControl = true;
    m_pRemoteOwner      = e;
    flags |= FL_THINK;
}

void VehicleTurretGun::EventGetCollisionModel(Event *ev)
{
    ev->AddEntity(m_pCollisionEntity);
}

float VehicleTurretGun::FireDelay(firemode_t mode)
{
    return fire_delay[mode];
}

void VehicleTurretGun::SetWarmupDelay(Event *ev)
{
    m_fWarmupDelay = ev->GetFloat(1);
}

void VehicleTurretGun::SetFireWarmupDelay(Event *ev)
{
    m_fFireWarmupDelay = ev->GetFloat(1);
}

void VehicleTurretGun::SetReloadDelay(Event *ev)
{
    m_fReloadDelay = ev->GetFloat(1);
}

void VehicleTurretGun::SetReloadShots(Event *ev)
{
    m_iReloadShots               = ev->GetInteger(1);
    m_iAmmo                      = m_iReloadShots;
    ammo_in_clip[FIRE_PRIMARY]   = m_iAmmo;
    ammo_clip_size[FIRE_PRIMARY] = m_iAmmo;
}

void VehicleTurretGun::SetAimOffset(Event *ev)
{
    m_vAimOffset = ev->GetVector(1);
}

void VehicleTurretGun::SetAimTolerance(Event *ev)
{
    m_vAimTolerance = ev->GetVector(1);
}

void VehicleTurretGun::SetTargetEntity(Event *ev)
{
    SetTargetEntity(ev->GetEntity(1));
}

void VehicleTurretGun::PlayReloadSound(Event *ev)
{
    Sound(m_sSoundSet + "snd_reload", CHAN_AUTO);
}

void VehicleTurretGun::SetTargetEntity(Entity *ent)
{
    Vector delta;
    int    i;

    if (!ent) {
        return;
    }

    if (owner || m_pRemoteOwner) {
        return;
    }

    delta = ent->origin - origin;
    delta.normalize();

    vectoangles(delta, m_vLocalAngles);
    m_vLocalAngles -= m_vBaseAngles;

    for (i = 0; i < 2; i++) {
        if (m_vLocalAngles[i] > 180) {
            m_vLocalAngles[i] -= 360;
        }

        if (m_vLocalAngles[i] < -180) {
            m_vLocalAngles[i] += 360;
        }
    }

    m_vLocalAngles[0] = Q_clamp_float(m_vLocalAngles[0], m_fPitchUpCap, m_fPitchDownCap);
    m_vLocalAngles[1] = Q_clamp_float(m_vLocalAngles[1], -(m_fStartYaw + m_fMaxYawOffset), m_fStartYaw + m_fMaxYawOffset);

    UpdateOrientation(false);
}

void VehicleTurretGun::UpdateAndMoveOwner()
{
    Vector  delta;
    Vector  dir;
    Vector  targetAngles;
    Entity *parentEnt;
    float   fDiff;

    parentEnt = GetParent();

    m_vUserViewAng[0] = Q_clamp_float(m_vUserViewAng[0], m_fPitchUpCap - MAX_VT_PITCHCAP_OFFSET, m_fPitchDownCap + MAX_VT_PITCHCAP_OFFSET);

    fDiff = AngleSubtract(m_vUserViewAng[1], m_fStartYaw);
    fDiff = Q_clamp_float(fDiff, -(m_fMaxYawOffset + MAX_VT_YAW_OFFSET), m_fMaxYawOffset + MAX_VT_YAW_OFFSET);
    m_vUserViewAng[1] = m_fStartYaw + fDiff;

    owner->SetViewAngles(m_vUserViewAng + m_vBaseAngles);
    delta = owner->GunTarget(true) - origin;

    if (m_bBOIsSet) {
        dir[0] = m_mBaseOrientation[0] * delta;
        dir[1] = m_mBaseOrientation[1] * delta;
        dir[2] = m_mBaseOrientation[2] * delta;
    } else if (parentEnt) {
        vec3_t mat[3];

        AnglesToAxis(parentEnt->angles, mat);
        dir[0] = mat[0] * delta;
        dir[1] = mat[1] * delta;
        dir[2] = mat[2] * delta;
    } else {
        dir = delta;
    }

    VectorNormalize(dir);
    vectoangles(dir, m_vTargetAngles);
}

void VehicleTurretGun::UpdateTimers(float& yawTimer, float& pitchTimer)
{
    if (m_fReloadTimeRemaining > 0) {
        m_fReloadTimeRemaining -= level.frametime;
        if (m_fReloadTimeRemaining <= 0) {
            ammo_in_clip[FIRE_PRIMARY] = m_iAmmo;
        }
    }

    if (m_fWarmupTimeRemaining > 0) {
        m_fWarmupTimeRemaining -= level.frametime;
        yawTimer   = level.frametime * (m_fTurnSpeed * ((m_fWarmupDelay - m_fWarmupTimeRemaining) / m_fWarmupDelay));
        pitchTimer = level.frametime * (m_fAIPitchSpeed * ((m_fWarmupDelay - m_fWarmupTimeRemaining) / m_fWarmupDelay));
    } else {
        yawTimer   = m_fTurnSpeed * level.frametime;
        pitchTimer = m_fAIPitchSpeed * level.frametime;
    }
}

void VehicleTurretGun::UpdateCaps(float maxYawOffset, float maxPitchOffset)
{
    float fDiff;

    if (m_vTargetAngles[0] > 180) {
        m_vTargetAngles[0] -= 360;
    } else if (m_vTargetAngles[0] < -180) {
        m_vTargetAngles[0] += 360;
    }

    m_vTargetAngles[0] = Q_clamp_float(m_vTargetAngles[0], m_fPitchUpCap, m_fPitchDownCap);

    if (m_fAIPitchSpeed > 1000) {
        m_vLocalAngles[0] = m_vTargetAngles[0];
    } else {
        fDiff = AngleSubtract(m_vTargetAngles[0], m_vLocalAngles[0]);
        if (fabs(fDiff) >= maxPitchOffset) {
            if (fDiff > 0) {
                m_vLocalAngles[0] += maxPitchOffset;
            } else {
                m_vLocalAngles[0] -= maxPitchOffset;
            }
        } else {
            m_vLocalAngles[0] = m_vTargetAngles[0];
        }
    }

    fDiff = AngleSubtract(m_vTargetAngles[1], m_fStartYaw);
    fDiff = Q_clamp_float(fDiff, -m_fMaxYawOffset, m_fMaxYawOffset);

    if (m_fTurnSpeed > 1000) {
        m_vLocalAngles[1] = m_vTargetAngles[1];
    } else {
        m_vTargetAngles[1] = m_fStartYaw + fDiff;

        fDiff = AngleSubtract(m_vTargetAngles[1], m_vLocalAngles[1]);
        if (fabs(fDiff) < 2) {
            Unregister(STRING_ONTARGET);
        }

        if (fabs(fDiff) >= maxYawOffset) {
            if (fDiff > 0) {
                m_vLocalAngles[1] += maxYawOffset;
            } else {
                m_vLocalAngles[1] -= maxYawOffset;
            }
        } else {
            m_vLocalAngles[1] = m_vTargetAngles[1];
        }
    }
}

void VehicleTurretGun::IdleToRestPosition()
{
    Vector  vDir, vNewAngles, vEnd;
    trace_t trace;

    if (angles[0] > 180) {
        angles[0] -= 360;
    }

    if (angles[0] < -80) {
        m_fIdlePitchSpeed = 0;
        m_iIdleHitCount   = 0;
        return;
    }

    if (m_iIdleHitCount > 1) {
        return;
    }

    m_fIdlePitchSpeed -= level.frametime * 300;

    vNewAngles = Vector(angles[0] + level.frametime * m_fIdlePitchSpeed, angles[1], angles[2]);
    vNewAngles.AngleVectorsLeft(&vDir);
    vEnd = origin + vDir * m_vIdleCheckOffset[0];

    trace = G_Trace(origin, vec_zero, vec_zero, vEnd, this, edict->clipmask, false, "TurretGun::Think idle");

    if (trace.fraction == 1) {
        setAngles(vNewAngles);
        m_iIdleHitCount = 0;
        return;
    }

    int iTry;
    for (iTry = 3; iTry > 0; iTry--) {
        vNewAngles[0] = angles[0] + level.frametime * m_fIdlePitchSpeed * iTry * 0.25f;
        if (vNewAngles[0] < m_fMaxIdlePitch) {
            continue;
        }

        vNewAngles.AngleVectorsLeft(&vDir);
        vEnd = origin + vDir * m_vIdleCheckOffset[0];

        trace = G_Trace(origin, vec_zero, vec_zero, vEnd, this, edict->clipmask, false, "TurretGun::Think idle");

        if (trace.fraction == 1) {
            setAngles(vNewAngles);

            m_iIdleHitCount = 0;
            m_fIdlePitchSpeed *= 0.25f * iTry;
            break;
        }
    }

    if (!iTry) {
        m_fIdlePitchSpeed = 0;

        Entity *ent = G_GetEntity(trace.entityNum);

        if (ent && ent == world) {
            m_iIdleHitCount++;
        } else {
            m_iIdleHitCount = 0;
        }
    }
}

void VehicleTurretGun::UpdateFireControl()
{
    if (!m_iFiring) {
        return;
    }

    if (m_iFiring == 1) {
        if (m_fFireWarmupDelay > 0) {
            Sound(m_sSoundSet + "snd_fire_warmup");
            m_fTargetReloadTime = level.time + m_fFireWarmupDelay;
            m_iFiring           = 2;
        } else {
            m_iFiring = 3;
        }
    } else if (m_iFiring == 2) {
        if (level.time >= m_fTargetReloadTime) {
            m_iFiring = 3;
        }
    }

    if (m_iFiring <= 2) {
        return;
    }

    if (!m_fMaxBurstDelay) {
        m_iFiring = 4;

        if (ReadyToFire(FIRE_PRIMARY, false)) {
            Fire(FIRE_PRIMARY);
            assert(!m_pVehicleOwner || (m_pVehicleOwner && m_pVehicleOwner->IsSubclassOfVehicle()));

            m_fCurrViewJitter = m_fViewJitter;
            AdjustReloadStatus();

            if (m_pVehicleOwner && m_pVehicleOwner->IsSubclassOfVehicleTank()) {
                VehicleTank *tank = static_cast<VehicleTank *>(m_pVehicleOwner.Pointer());
                tank->KickSuspension(Vector(orientation[1]) * -1, 6);
            }
        }
    } else if (m_iFiring == 4) {
        if (ReadyToFire(FIRE_PRIMARY, false)) {
            Fire(FIRE_PRIMARY);
            assert(!m_pVehicleOwner || (m_pVehicleOwner && m_pVehicleOwner->IsSubclassOfVehicle()));

            m_fCurrViewJitter = m_fViewJitter;
            AdjustReloadStatus();

            if (m_pVehicleOwner && m_pVehicleOwner->IsSubclassOfVehicleTank()) {
                VehicleTank *tank = static_cast<VehicleTank *>(m_pVehicleOwner.Pointer());
                tank->KickSuspension(Vector(orientation[1]) * -1, 6);
            }

            if (level.time > m_fFireToggleTime) {
                m_iFiring         = 1;
                m_fFireToggleTime = level.time + m_fMinBurstDelay + (m_fMaxBurstDelay - m_fMinBurstDelay) * random();
            }
        }
    } else if (level.time > m_fFireToggleTime) {
        m_iFiring         = 4;
        m_fFireToggleTime = level.time + m_fMinBurstDelay + (m_fMaxBurstDelay - m_fMinBurstDelay) * random();
    }
}

void VehicleTurretGun::UpdateCollisionEntity()
{
    Sentient *sentOwner;
    Vector    amove;
    Vector    newAngles;
    solid_t   oldsolid;

    if (!m_pCollisionEntity) {
        return;
    }

    setSolidType(SOLID_NOT);

    sentOwner = GetSentientOwner();
    if (sentOwner) {
        oldsolid = sentOwner->edict->solid;
        sentOwner->setSolidType(SOLID_NOT);
    }

    m_pCollisionEntity->Solid();
    if (m_iPitchBone < 0) {
        newAngles = angles;
    } else if (m_bBOIsSet) {
        vec3_t axis[3];
        vec3_t mat[3];

        AnglesToAxis(m_vLocalAngles, mat);
        MatrixMultiply(axis, m_mBaseOrientation, mat);
        MatrixToEulerAngles(mat, newAngles);
    } else {
        newAngles = m_vLocalAngles;
    }

    amove[0] = angledist(newAngles[0] - m_pCollisionEntity->angles[0]);
    amove[1] = angledist(newAngles[1] - m_pCollisionEntity->angles[1]);
    amove[2] = angledist(newAngles[2] - m_pCollisionEntity->angles[2]);

    G_PushMove(m_pCollisionEntity, origin - m_pCollisionEntity->origin, amove);

    m_pCollisionEntity->setOrigin(origin);
    m_pCollisionEntity->setAngles(newAngles);
    m_pCollisionEntity->velocity  = velocity;
    m_pCollisionEntity->avelocity = avelocity;

    if (sentOwner) {
        sentOwner->setSolidType(static_cast<solid_t>(oldsolid));
    }
}

void VehicleTurretGun::RestrictPitch()
{
    m_vUserViewAng[0] = Q_clamp_float(m_vUserViewAng[0], m_fPitchUpCap - MAX_VT_PITCHCAP_OFFSET, m_fPitchDownCap + MAX_VT_PITCHCAP_OFFSET);
}

void VehicleTurretGun::RestrictYaw()
{
    float fDiff;

    fDiff = AngleSubtract(m_vUserViewAng[1], m_fStartYaw);
    fDiff = Q_clamp_float(fDiff, -(m_fMaxYawOffset + MAX_VT_YAW_OFFSET), m_fMaxYawOffset + MAX_VT_YAW_OFFSET);

    m_vUserViewAng[1] = m_fStartYaw + fDiff;
}

void VehicleTurretGun::UpdateRemoteControl()
{
    Sentient *sentOwner;
    Vector    position;
    Vector    forward, left, up;
    Vector    start, end;
    Vector    delta;
    Vector    transformed;
    trace_t   trace;

    RestrictPitch();
    RestrictYaw();

    sentOwner = GetSentientOwner();
    if (!sentOwner) {
        return;
    }

    sentOwner->SetViewAngles(m_vUserViewAng + m_vBaseAngles);

    if (m_iEyeBone < 0) {
        (m_vUserViewAng + m_vBaseAngles).AngleVectors(&forward);
        end = m_pRemoteOwner->origin + forward * 16384;

        trace =
            G_Trace(m_pRemoteOwner->origin, vec_zero, vec_zero, end, this, MASK_TRANSPARENT, qfalse, "VehicleTurret");

        position = trace.endpos;
    } else {
        orientation_t eye_or;

        GetRawTag(m_iEyeBone, &eye_or);
        angles.AngleVectorsLeft(&forward, &left, &up);

        start = origin + forward * eye_or.origin[0] + left * eye_or.origin[1] + up * eye_or.origin[2];
        (m_vUserViewAng + m_vBaseAngles).AngleVectorsLeft(&forward);

        end = start + forward * 8192;

        trace = G_Trace(start, vec_zero, vec_zero, end, this, MASK_TRANSPARENT, qfalse, "VehicleTurret");

        position = trace.endpos;
    }

    delta = position - origin;

    if (m_bBOIsSet) {
        transformed[0] = m_mBaseOrientation[0] * delta;
        transformed[1] = m_mBaseOrientation[1] * delta;
        transformed[2] = m_mBaseOrientation[2] * delta;
    } else if (GetParent()) {
        vec3_t mat[3];

        AnglesToAxis(GetParent()->angles, mat);
        transformed[0] = mat[0] * delta;
        transformed[1] = mat[1] * delta;
        transformed[2] = mat[2] * delta;
    } else {
        transformed = delta;
    }

    VectorNormalize(transformed);
    vectoangles(transformed, m_vTargetAngles);
}

void VehicleTurretGun::UpdateAimTarget()
{
    Vector delta;
    Vector transformed;

    delta = aim_target->centroid - centroid;
    if (m_bBOIsSet) {
        transformed[0] = m_mBaseOrientation[0] * delta;
        transformed[1] = m_mBaseOrientation[1] * delta;
        transformed[2] = m_mBaseOrientation[2] * delta;
    } else if (GetParent()) {
        vec3_t mat[3];

        AnglesToAxis(GetParent()->angles, mat);
        transformed[0] = mat[0] * delta;
        transformed[1] = mat[1] * delta;
        transformed[2] = mat[2] * delta;
    } else {
        transformed = delta;
    }

    VectorNormalize(transformed);
    vectoangles(transformed, m_vTargetAngles);
}

Entity *VehicleTurretGun::GetParent() const
{
    if (m_pBaseEntity) {
        return m_pBaseEntity;
    }

    if (edict->s.parent) {
        return G_GetEntity(edict->s.parent);
    }

    return NULL;
}

SentientPtr VehicleTurretGun::GetRemoteOwner(void)
{
    return m_pRemoteOwner;
}

SentientPtr VehicleTurretGun::GetSentientOwner()
{
    if (IsRemoteControlled()) {
        return GetRemoteOwner();
    } else {
        return owner;
    }
}

void VehicleTurretGun::EndRemoteControl()
{
    m_bUseRemoteControl = false;
    m_pRemoteOwner      = NULL;
    m_fIdlePitchSpeed   = 0;
    m_iIdleHitCount     = 0;
    m_iFiring           = 0;
    m_vTargetAngles     = m_vLocalAngles;

    //
    // Added in OPM.
    //  The camera must be removed after ending the remote control.
    RemoveUserCamera();
}

float VehicleTurretGun::GetWarmupFraction() const
{
    float frac;

    if (!m_fTargetReloadTime) {
        return 0;
    }

    frac = (m_fTargetReloadTime - level.time) / m_fFireWarmupDelay;
    // Clamp between [0,1]
    frac = Q_clamp_float(frac, 0, 1);

    return 1.0 - frac;
}

bool VehicleTurretGun::IsRemoteControlled()
{
    return m_bUseRemoteControl;
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

void VehicleTurretGun::Archive(Archiver& arc)
{
    TurretGun::Archive(arc);

    arc.ArchiveVector(&m_vTargetAngles);
    arc.ArchiveVector(&m_vLocalAngles);
    arc.ArchiveVec3(m_mBaseOrientation[0]);
    arc.ArchiveVec3(m_mBaseOrientation[1]);
    arc.ArchiveVec3(m_mBaseOrientation[2]);
    arc.ArchiveInteger(&m_iPitchBone);
    arc.ArchiveVector(&m_vBaseAngles);
    arc.ArchiveVector(&m_vLastBaseAngles);
    arc.ArchiveSafePointer(&m_pBaseEntity);
    arc.ArchiveBool(&m_bBOIsSet);
    arc.ArchiveBool(&m_bUseRemoteControl);
    arc.ArchiveSafePointer(&m_pRemoteOwner);
    arc.ArchiveInteger(&m_iBarrelTag);
    arc.ArchiveInteger(&m_iEyeBone);
    arc.ArchiveVector(&m_vBarrelPos);
    arc.ArchiveVector(&m_vLastBarrelPos);
    arc.ArchiveFloat(&m_fWarmupTimeRemaining);
    arc.ArchiveFloat(&m_fWarmupDelay);
    arc.ArchiveFloat(&m_fFireWarmupDelay);
    arc.ArchiveFloat(&m_fTargetReloadTime);
    arc.ArchiveInteger(&m_iReloadShots);
    arc.ArchiveInteger(&m_iAmmo);
    arc.ArchiveFloat(&m_fReloadDelay);
    arc.ArchiveFloat(&m_fReloadTimeRemaining);
    arc.ArchiveVector(&m_vAimOffset);
    arc.ArchiveVector(&m_vAimTolerance);
    arc.ArchiveSafePointer(&m_pVehicleOwner);
    arc.ArchiveBool(&m_bRemoveOnDeath);
    ArchiveEnum(m_eSoundState, SOUND_STATE_TURRET);
    arc.ArchiveFloat(&m_fNextSoundState);
    arc.ArchiveBool(&m_bLocked);
    arc.ArchiveString(&m_sSoundSet);
    arc.ArchiveSafePointer(&m_pCollisionEntity);
}

EntityPtr VehicleTurretGun::GetVehicle() const
{
    return m_pVehicleOwner;
}

SentientPtr VehicleTurretGun::GetRawRemoteOwner() const
{
    return m_pRemoteOwner;
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
    m_HeadTurret    = this;
    m_ActiveTurret  = NULL;

    m_fSwitchTimeRemaining = 0;
    // 1 second switch delay
    m_fSwitchDelay = 1;
}

VehicleTurretGunTandem::~VehicleTurretGunTandem()
{
    //
    // Added in 2.30
    // Also remove linked turrets
    //
    if (m_Slot.ent) {
        m_Slot.ent->PostEvent(EV_Remove, 0);
    }
}

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

void VehicleTurretGunTandem::EndRemoteControl()
{
    //
    // Added in OPM.
    //  This cleanups remove control stuff from the main turret and linked turrets.
    VehicleTurretGun::EndRemoteControl();

    if (m_HeadTurret && m_HeadTurret != this) {
        return m_HeadTurret->EndRemoteControl();
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

    if (m_ActiveTurret && m_ActiveTurret != m_HeadTurret) {
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
        return;
    }

    if (m_Slot.ent) {
        pTurret = static_cast<VehicleTurretGunTandem*>(m_Slot.ent.Pointer());
        m_SwitchLabel.Execute(pTurret, NULL);
    } else {
        pTurret = m_PrimaryTurret;
        m_PrimaryTurret->m_SwitchLabel.Execute(pTurret, NULL);
    }

    SetActiveTurret(pTurret);
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
    delta = Q_clamp_float(delta, -(m_fMaxYawOffset + MAX_VT_YAW_OFFSET), m_fMaxYawOffset + MAX_VT_YAW_OFFSET);

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
        return m_HeadTurret == this;
    }

    return m_PrimaryTurret->m_HeadTurret == this;
}
