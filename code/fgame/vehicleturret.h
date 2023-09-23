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

// vehicleturret.h: Vehicle Turret.
//

#pragma once

#include "weapturret.h"
#include "vehicle.h"

class VehicleTurretGun : public TurretGun
{
    friend class Vehicle;

protected:
    Vector                          m_vTargetAngles;
    Vector                          m_vLocalAngles;
    float                           m_mBaseOrientation[3][3];
    Vector                          m_vBaseAngles;
    Vector                          m_vLastBaseAngles;
    SafePtr<Entity>                 m_pBaseEntity;
    int                             m_iPitchBone;
    bool                            m_bBOIsSet;
    bool                            m_bUseRemoteControl;
    SentientPtr                     m_pRemoteOwner;
    int                             m_iBarrelTag;
    int                             m_iEyeBone;
    SafePtr<Entity>                 m_pVehicleOwner;
    bool                            m_bRemoveOnDeath;
    SafePtr<VehicleCollisionEntity> m_pCollisionEntity;
    bool                            m_bLocked;
    SOUND_STATE                     m_eSoundState;
    float                           m_fNextSoundState;
    str                             m_sSoundSet;

public:
    CLASS_PROTOTYPE(VehicleTurretGun);

    Vector m_vBarrelPos;
    Vector m_vLastBarrelPos;

    VehicleTurretGun();
    virtual ~VehicleTurretGun();

    void Think(void) override;
    void P_UserAim(usercmd_t *ucmd) override;

    void         SetBaseOrientation(float (*borientation)[3], float *bangles);
    void         SetBaseEntity(Entity *e);
    void         SetVehicleOwner(Entity *e);
    void         SetRemoteOwner(Sentient *e);
    virtual void RemoteControl(usercmd_t *ucmd, Sentient *owner);
    void         CollisionCorrect(trace_t *pTr);
    void         UpdateOrientation(bool bCollisionCheck);
    void         UpdateSound(void);
    void         UpdateOwner(Sentient *pOwner);
    void         SetBaseEntity(Event *ev);
    void         PlaceTurret(Event *ev);
    void         TurretBeginUsed(Sentient *pEnt) override;
    void         TurretEndUsed(void) override;
    void         TurretUsed(Sentient *pEnt) override;
    void         TurretUsed(Event *ev);
    void         EventKilled(Event *ev);
    void         EventDamage(Event *ev);
    void         EventRemoveOnDeath(Event *ev);
    void         EventSetCollisionModel(Event *ev);
    void         EventGetCollisionModel(Event *ev);
    void         EventTurnSpeed(Event *ev);
    void         EventLock(Event *ev);
    void         EventUnlock(Event *ev);
    void         SetSoundSet(Event *ev);
    bool         isLocked(void);
    void         Lock(void);
    void         UnLock(void);
    bool         UseRemoteControl(void);

    virtual void        EndRemoteControl();
    virtual SentientPtr GetSentientOwner();
    virtual SentientPtr GetRemoteOwner(void);
    virtual bool        IsRemoteControlled();
    virtual void        RestrictPitch();
    virtual void        RestrictYaw();
    virtual bool        TurretHasBeenMounted();

    void    UpdateRemoteControl();
    void    UpdateAimTarget();
    Entity *GetParent() const;
    float   GetWarmupFraction() const;

    qboolean ReadyToFire(firemode_t mode, qboolean playsound) override;
    void     AdjustReloadStatus();
    void     GetMuzzlePosition(vec3_t position, vec3_t vBarrelPos, vec3_t forward, vec3_t right, vec3_t up) override;
    void     ApplyFireKickback(const Vector    &org, float kickback) override;
    float    FireDelay(firemode_t mode) override;

    void      SetWarmupDelay(Event *ev);
    void      SetFireWarmupDelay(Event *ev);
    void      SetReloadDelay(Event *ev);
    void      SetReloadShots(Event *ev);
    void      SetAimOffset(Event *ev);
    void      SetAimTolerance(Event *ev);
    void      SetTargetEntity(Event *ev);
    void      PlayReloadSound(Event *ev);
    void      SetTargetEntity(Entity *ent);
    void      UpdateAndMoveOwner();
    void      UpdateTimers(float     &yawTimer, float     &pitchTimer);
    void      UpdateCaps(float maxYawOffset, float maxPitchOffset);
    void      IdleToRestPosition();
    void      UpdateFireControl();
    void      UpdateCollisionEntity();
    EntityPtr GetVehicle() const;

    void Archive(Archiver& arc) override;
};

class VehicleTurretGunTandem : public VehicleTurretGun
{
private:
    cTurretSlot                     m_Slot;
    SafePtr<VehicleTurretGunTandem> m_PrimaryTurret;
    SafePtr<VehicleTurretGunTandem> m_HeadTurret;
    SafePtr<VehicleTurretGunTandem> m_ActiveTurret;
    float                           m_fSwitchTimeRemaining;
    float                           m_fSwitchDelay;
    ScriptThreadLabel               m_SwitchLabel;

public:
    CLASS_PROTOTYPE(VehicleTurretGunTandem);

public:
    VehicleTurretGunTandem();
    ~VehicleTurretGunTandem();

    void EventLinkTurret(Event *ev);
    void AttachLinkedTurret(Entity *ent);
    void UpdateLinkedTurret();
    void OpenSlotsByModel();

    void         Think() override;
    bool         IsRemoteControlled() override;
    SentientPtr  GetRemoteOwner() override;
    virtual void ThinkPrimary();
    virtual void ThinkSecondary();

    void                    SetPrimaryTurret(VehicleTurretGunTandem *pTurret);
    VehicleTurretGunTandem *GetPrimaryTurret();
    bool                    IsActiveTurret() const;

    void RemoteControl(usercmd_t *ucmd, Sentient *owner) override;
    void RemoteControlSecondary(usercmd_t *ucmd, Sentient *owner);
    void RemoteControlFire(usercmd_t *ucmd, Sentient *owner);

    void EventSetSwitchThread(Event *ev);
    void SwitchToLinkedTurret();
    void SetActiveTurret(VehicleTurretGunTandem *pTurret);

    void RestrictYaw();

    void Archive(Archiver& arc);
};
