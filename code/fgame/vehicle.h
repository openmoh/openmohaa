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

// vehicle.h: Script controlled vehicles.
//

#pragma once

#include "g_local.h"
#include "entity.h"
#include "sentient.h"
#include "scriptslave.h"
#include "spline.h"
#include "VehicleSlot.h"
#include "VehicleCollisionEntity.h"
#include "VehicleSoundEntity.h"
#include "movegrid.h"

extern Event EV_Vehicle_Enter;
extern Event EV_Vehicle_Exit;
extern Event EV_Vehicle_Drivable;
extern Event EV_Vehicle_UnDrivable;
extern Event EV_Vehicle_Lock;
extern Event EV_Vehicle_UnLock;
extern Event EV_Vehicle_SeatAnglesOffset;
extern Event EV_Vehicle_SeatOffset;
extern Event EV_Vehicle_DriverAnimation;
extern Event EV_Vehicle_SetWeapon;
extern Event EV_Vehicle_ShowWeapon;
extern Event EV_Vehicle_SetSpeed;
extern Event EV_Vehicle_SetTurnRate;

extern cvar_t *g_showvehiclemovedebug;
extern cvar_t *g_showvehicleentrypoints;
extern cvar_t *g_showvehicleslotpoints;
extern cvar_t *g_showvehicletags;
extern cvar_t *g_showvehiclepath;

#define NUM_VEHICLE_TIRES 4

using cVehicleSpline = cSpline<4, 512>;

typedef struct wheel_s {
    Vector   vOrigin;
    Vector   vAcceleration;
    Vector   vVelocity;
    Vector   vLongForce;
    Vector   vLatForce;
    Vector   vForce;
    Vector   vOrientation;
    Vector   vResistance;
    float    fYawOffset;
    float    fSlipAngle;
    float    fGripCoef;
    float    fLoad;
    float    fLongForce;
    float    fLatForce;
    float    fForce;
    float    fTraction;
    float    fLongTorque;
    float    fLatTorque;
    float    fTorque;
    float    fLongDist;
    float    fLatDist;
    qboolean bSkidding;
    qboolean bSpinning;
    int      iPowered;
} wheel_t;

class VehicleBase : public Animate
{
public:
    VehicleBase *vlink;
    Vector       offset;

    CLASS_PROTOTYPE(VehicleBase);

    VehicleBase(void);
    void Archive(Archiver& arc) override;
};

inline void VehicleBase::Archive(Archiver& arc)
{
    Animate::Archive(arc);

    arc.ArchiveObjectPointer((Class **)&vlink);
    arc.ArchiveVector(&offset);
}

typedef enum {
    ST_OFF,
    ST_OFF_TRANS_IDLE,
    ST_IDLE_TRANS_OFF,
    ST_IDLE,
    ST_IDLE_TRANS_RUN,
    ST_RUN,
    ST_RUN_TRANS_IDLE,
    ST_IDLE_TRANS_TURN,
    ST_TURN,
    ST_TURN_TRANS_IDLE
} SOUND_STATE;

#define MAX_DRIVERS          1
#define MAX_PASSENGERS       32
#define MAX_TURRETS          8
#define MAX_CORNERS          4

#define MAX_SKIPPED_ENTITIES 64

class Vehicle : public VehicleBase
{
    friend class VehicleSoundEntity;
    friend class VehicleTurretGun;

protected:
    cVehicleSlot                    driver;
    cVehicleSlot                    lastdriver;
    cVehicleSlot                    Passengers[MAX_PASSENGERS];
    cTurretSlot                     Turrets[MAX_TURRETS];
    int                             numPassengers;
    int                             numTurrets;
    int                             moveresult;
    int                             isBlocked;
    int                             m_iFrameCtr;
    int                             m_iGear;
    int                             m_iRPM;
    int                             m_iLastTiresUpdate;
    cVehicleSpline                 *m_pAlternatePath;
    int                             m_iAlternateNode;
    cVehicleSpline                 *m_pCurPath;
    int                             m_iCurNode;
    cVehicleSpline                 *m_pNextPath;
    int                             m_iNextPathStartNode;
    float                           maxturnrate;
    float                           currentspeed;
    float                           turnangle;
    float                           turnimpulse;
    float                           moveimpulse;
    float                           prev_moveimpulse;
    float                           jumpimpulse;
    float                           speed;
    float                           conesize;
    float                           maxtracedist;
    float                           airspeed;
    float                           m_fGearRatio[6];
    float                           m_fMass;
    float                           m_fFrontMass;
    float                           m_fBackMass;
    float                           m_fWheelBase;
    float                           m_fWheelFrontLoad;
    float                           m_fWheelFrontInnerLoad;
    float                           m_fWheelFrontOutterLoad;
    float                           m_fWheelFrontDist;
    float                           m_fWheelFrontSuspension;
    float                           m_fWheelBackLoad;
    float                           m_fWheelBackInnerLoad;
    float                           m_fWheelBackOutterLoad;
    float                           m_fWheelBackDist;
    float                           m_fWheelBackSuspension;
    float                           m_fCGHeight;
    float                           m_fBankAngle;
    float                           m_fTread;
    float                           m_fTrackWidth;
    float                           m_fTireFriction;
    float                           m_fDrag;
    float                           m_fTireRadius;
    float                           m_fFrontBrakes;
    float                           m_fBackBrakes;
    float                           m_fRollingResistance;
    float                           m_fTireRotationalSpeed;
    float                           m_fFrontBrakingForce;
    float                           m_fBackBrakingForce;
    float                           m_fBrakingPerformance;
    float                           m_fLastTurn;
    float                           m_fTangForce;
    float                           m_fInertia;
    float                           m_fDifferentialRatio;
    float                           m_fGearEfficiency;
    float                           m_fMaxTraction;
    float                           m_fTractionForce;
    float                           m_fAccelerator;
    float                           m_fTorque;
    float                           m_fDownForce;
    float                           m_fUpForce;
    float                           m_fLeftForce;
    float                           m_fRightForce;
    float                           m_fForwardForce;
    float                           m_fBackForce;
    float                           m_fBouncyCoef;
    float                           m_fSpringyCoef;
    float                           m_fYawMin;
    float                           m_fYawMax;
    float                           m_fRollMin;
    float                           m_fRollMax;
    float                           m_fZMin;
    float                           m_fZMax;
    float                           m_fYawCoef;
    float                           m_fRollCoef;
    float                           m_fZCoef;
    float                           m_fShaderOffset;
    float                           m_fSoundMinSpeed;
    float                           m_fSoundMinPitch;
    float                           m_fSoundMaxSpeed;
    float                           m_fSoundMaxPitch;
    float                           m_fVolumeMinSpeed;
    float                           m_fVolumeMinPitch;
    float                           m_fVolumeMaxSpeed;
    float                           m_fVolumeMaxPitch;
    float                           m_fStopStartDistance;
    float                           m_fStopStartSpeed;
    float                           m_fStopEndDistance;
    float                           m_fSkidAngle;
    float                           m_fSkidLeftForce;
    float                           m_fSkidRightForce;
    str                             vehicleName;
    str                             weaponName;
    str                             m_sAnimationSet;
    str                             m_sSoundSet;
    str                             m_sExplosionModel;
    Vector                          last_origin;
    Vector                          seatangles;
    Vector                          seatoffset;
    Vector                          driveroffset;
    Vector                          Corners[MAX_CORNERS];
    Vector                          v_angle;
    Vector                          yaw_forward;
    Vector                          yaw_left;
    Vector                          ground_normal;
    Vector                          base_angles;
    Vector                          prev_velocity;
    Vector                          real_velocity;
    Vector                          prev_origin;
    Vector                          real_acceleration;
    Vector                          prev_acceleration;
    Vector                          m_vOldMins;
    Vector                          m_vOldMaxs;
    Vector                          m_vCG;
    Vector                          m_vAngularAcceleration;
    Vector                          m_vAngularVelocity;
    Vector                          m_vAngles;
    Vector                          m_vFrontNormal;
    Vector                          m_vBackNormal;
    Vector                          m_vFrontAngles;
    Vector                          m_vBackAngles;
    Vector                          m_vBaseNormal;
    Vector                          m_vBaseAngles;
    Vector                          m_vPrevNormal;
    Vector                          m_vResistance;
    Vector                          m_vWheelForce;
    Vector                          m_vTangForce;
    Vector                          m_vForce;
    Vector                          m_vAcceleration;
    Vector                          m_vOriginOffset;
    Vector                          m_vOriginOffset2;
    Vector                          m_vOriginCornerOffset;
    Vector                          m_vAnglesOffset;
    Vector                          m_vSaveAngles;
    Vector                          m_vSkidOrigin;
    qboolean                        drivable;
    qboolean                        pathDrivable;
    qboolean                        locked;
    qboolean                        hasweapon;
    qboolean                        showweapon;
    qboolean                        steerinplace;
    qboolean                        jumpable;
    qboolean                        m_bWheelSpinning;
    qboolean                        m_bIsSkidding;
    qboolean                        m_bIsBraking;
    qboolean                        m_bBackSlipping;
    qboolean                        m_bFrontSlipping;
    qboolean                        m_bAutomatic;
    qboolean                        m_bThinkCalled;
    qboolean                        m_bMovementLocked;
    qboolean                        m_bRemoveOnDeath;
    qboolean                        m_bStopEnabled;
    qboolean                        m_bEnableSkidding;
    qboolean                        m_bAnimMove;
    qboolean                        m_bDamageSounds;
    qboolean                        m_bRunSounds;
    int                             m_iProjectileHitsRemaining;
    vehicleState_t                  vs;
    cMoveGrid                      *m_sMoveGrid;
    float                           m_fIdealSpeed;
    float                           m_fMaxSpeed;
    bool                            m_bBounceBackwards;
    Vector                          m_vIdealPosition;
    Vector                          m_vIdealDir;
    float                           m_fIdealAccel;
    float                           m_fIdealDistance;
    float                           m_fLookAhead;
    bool                            m_bAutoPilot;
    SafePtr<VehicleCollisionEntity> m_pCollisionEntity;
    SOUND_STATE                     m_eSoundState;
    float                           m_fNextSoundState;
    SafePtr<VehicleSoundEntity>     m_pVehicleSoundEntities[MAX_CORNERS];
    Vector                          m_vTireEnd[NUM_VEHICLE_TIRES];
    bool                            m_bTireHit[NUM_VEHICLE_TIRES];
    Vector                          m_vNormalSum;
    int                             m_iNumNormals;
    float                           m_fMaxUseAngle;
    bool                            m_bBounceStayFullSpeed;

protected:
    virtual void WorldEffects(void);
    virtual void CheckWater(void);
    virtual void DriverUse(Event *ev);
    virtual void VehicleStart(Event *ev);
    virtual void VehicleTouched(Event *ev);
    virtual void VehicleBlocked(Event *ev);
    // Added in 2.30
    //====
    void SetProjectileVulnerable(Event *ev);
    void DoProjectileVulnerability(Entity *pProjectile, Entity *pOwner, meansOfDeath_t meansOfDeath);
    //====
    void         Think(void) override;
    void         Postthink(void) override;
    virtual void Drivable(Event *ev);
    virtual void PathDrivable(Event *ev); // Added in 2.30
    virtual void UnDrivable(Event *ev);
    virtual void Jumpable(Event *ev);
    virtual void SeatAnglesOffset(Event *ev);
    virtual void SeatOffset(Event *ev);
    virtual void SetDriverAngles(Vector angles);
    virtual void Lock(Event *ev);
    virtual void UnLock(Event *ev);
    virtual void SetWeapon(Event *ev);
    virtual void SetName(Event *ev); // added in 2.0
    virtual void ShowWeaponEvent(Event *ev);
    // added in 2.30
    //====
    virtual void BounceForwardsVelocity(Event *ev);
    virtual void BounceBackwardsVelocity(Event *ev);
    virtual void StopForwardsVelocity(Event *ev);
    virtual void StopBackwardsVelocity(Event *ev);
    //====
    virtual void  SetSpeed(Event *ev);
    virtual void  SetTurnRate(Event *ev);
    virtual void  SteerInPlace(Event *ev);
    virtual void  VehicleDestroyed(Event *ev);
    virtual void  SetMoveInfo(vmove_t *vm);
    virtual void  GetMoveInfo(vmove_t *vm);
    virtual void  SetCEMoveInfo(vmove_t *vm);
    virtual void  GetCEMoveInfo(vmove_t *vm);
    virtual void  SetViewAngles(Vector newViewangles);
    virtual void  SetMass(Event *ev);
    virtual void  SetFrontMass(Event *ev);
    virtual void  SetBackMass(Event *ev);
    virtual void  SetTread(Event *ev);
    virtual void  SetTireRadius(Event *ev);
    virtual void  SetRollingResistance(Event *ev);
    virtual void  SetDrag(Event *ev);
    virtual float TorqueLookup(int rpm);
    virtual void  UpdateVariables(
         Vector *acceleration, Vector *vpn, Vector *vup, Vector *vright, Vector *t_vpn, Vector *t_vup, Vector *t_vright
     );
    virtual void ModelInit(Event *ev);
    virtual void BouncyCoef(Event *ev);
    virtual void SpringyCoef(Event *ev);
    virtual void YawMinMax(Event *ev);
    virtual void RollMinMax(Event *ev);
    virtual void ZMinMax(Event *ev);
    virtual void SetAnimationSet(Event *ev);
    virtual void SetSoundSet(Event *ev);
    virtual void SpawnTurret(Event *ev);
    void         EventLockMovement(Event *ev);
    void         EventUnlockMovement(Event *ev);
    virtual void QuerySpeed(Event *ev);
    virtual void QueryFreePassengerSlot(Event *ev);
    virtual void QueryFreeDriverSlot(Event *ev);
    virtual void QueryFreeTurretSlot(Event *ev);
    virtual void QueryPassengerSlotPosition(Event *ev);
    virtual void QueryDriverSlotPosition(Event *ev);
    virtual void QueryTurretSlotPosition(Event *ev);
    virtual void QueryPassengerSlotAngles(Event *ev);
    virtual void QueryDriverSlotAngles(Event *ev);
    virtual void QueryTurretSlotAngles(Event *ev);
    virtual void QueryPassengerSlotStatus(Event *ev);
    virtual void QueryDriverSlotStatus(Event *ev);
    virtual void QueryTurretSlotStatus(Event *ev);
    virtual void QueryPassengerSlotEntity(Event *ev);
    virtual void QueryDriverSlotEntity(Event *ev);
    virtual void QueryTurretSlotEntity(Event *ev);
    virtual void AttachPassengerSlot(Event *ev);
    virtual void AttachDriverSlot(Event *ev);
    virtual void AttachTurretSlot(Event *ev);
    virtual void DetachPassengerSlot(Event *ev);
    virtual void DetachDriverSlot(Event *ev);
    virtual void DetachTurretSlot(Event *ev);
    virtual void DetachRemoteOwner(); // added in 2.0
    virtual void SetWheelCorners(Event *ev);
    virtual void EventDriveInternal(Event *ev, bool wait);
    virtual void EventDrive(Event *ev);
    virtual void EventDriveNoWait(Event *ev);
    virtual void EventStop(Event *ev);
    virtual void EventFullStop(Event *ev);
    virtual void EventModifyDrive(Event *ev);
    virtual void EventNextDrive(Event *ev);
    void         EventModel(Event *ev);
    void         EventRemoveOnDeath(Event *ev);
    void         EventSetExplosionModel(Event *ev);
    void         EventSetCollisionModel(Event *ev);
    void         EventGetCollisionModel(Event *ev);
    void         EventSetSoundParameters(Event *ev);
    void         EventSetVolumeParameters(Event *ev);
    void         EventDamage(Event *ev);
    void         EventStopAtEnd(Event *ev);
    void         EventSkidding(Event *ev);
    void         EventContinueSkidding(Event *ev);
    void         EventVehicleAnim(Event *ev);
    void         EventVehicleAnimDone(Event *ev);
    // added in 2.0
    //====
    void EventVehicleMoveAnim(Event *ev);
    void EventVehicleMoveAnimDone(Event *ev);
    void EventDamageSounds(Event *ev);
    void EventRunSounds(Event *ev);
    void Remove(Event *ev);
    //====
    void         TouchStuff(vmove_t *vm);
    virtual void ResetSlots(void);
    virtual void OpenSlotsByModel(void);
    virtual void MoveVehicle(void);
    virtual void AnimMoveVehicle(void);
    virtual bool AssertMove(Vector vNewOrigin, Vector vOldOrigin);
    virtual bool AssertRotation(Vector vNewAngles, Vector vOldAngles);
    virtual void NoMove(void);
    virtual void SlidePush(Vector vPush);
    void         SetupPath(cVehicleSpline *pPath, SimpleEntity *se);
    virtual void UpdateSound(void);
    void         SetupVehicleSoundEntities(void);
    void         RemoveVehicleSoundEntities(void);
    void         TurnOnVehicleSoundEntities(void);
    void         TurnOffVehicleSoundEntities(void);
    void         UpdateTires(void);
    void         UpdateNormals(void);
    void         UpdateBones(void);
    void         UpdateShaderOffset(void);
    virtual void UpdateTurretSlot(int iSlot);
    void         UpdatePassengerSlot(int iSlot);
    void         UpdateDriverSlot(int iSlot);
    float        GetPathPosition(cVehicleSpline *pPath, int iNode);
    void         UpdateSkidAngle(void);
    void         FactorInSkidOrigin(void);
    virtual void FactorInOriginOffset(void);
    virtual void CalculateOriginOffset(void);
    virtual void FactorOutOriginOffset(void);
    virtual void FactorInAnglesOffset(Vector *vAddedAngles);
    virtual void CalculateAnglesOffset(Vector acceleration);
    virtual void FactorOutAnglesOffset(void);
    qboolean     GetTagPositionAndOrientation(str tagname, orientation_t *new_or);
    qboolean     GetTagPositionAndOrientation(int tagnum, orientation_t *new_or);
    void         CalculateAnimationData(Vector vAngles, Vector vOrigin);
    bool         IsDamagedBy(Entity *ent);

public:
    CLASS_PROTOTYPE(Vehicle);

    Vehicle();
    virtual ~Vehicle();

    virtual void     DriverAdded(); // added in 2.0
    virtual qboolean Drive(usercmd_t *ucmd);
    virtual qboolean PathDrive(usercmd_t *ucmd);
    void             AutoPilot(void);
    virtual qboolean HasWeapon(void);
    virtual qboolean ShowWeapon(void);
    Entity          *Driver(void);
    virtual qboolean IsDrivable(void);
    void             Archive(Archiver            &arc) override;
    str              GetSoundSet(void);
    virtual float    QuerySpeed(void);
    virtual int      QueryFreePassengerSlot(void);
    virtual int      QueryFreeDriverSlot(void);
    virtual int      QueryFreeTurretSlot(void);
    virtual void     QueryPassengerSlotPosition(int slot, float *pos);
    virtual void     QueryDriverSlotPosition(int slot, float *pos);
    virtual void     QueryTurretSlotPosition(int slot, float *pos);
    virtual void     QueryPassengerSlotAngles(int slot, float *ang);
    virtual void     QueryDriverSlotAngles(int slot, float *ang);
    virtual void     QueryTurretSlotAngles(int slot, float *ang);
    virtual int      QueryPassengerSlotStatus(int slot);
    virtual int      QueryDriverSlotStatus(int slot);
    virtual int      QueryTurretSlotStatus(int slot);
    virtual Entity  *QueryPassengerSlotEntity(int slot);
    virtual Entity  *QueryDriverSlotEntity(int slot);
    virtual Entity  *QueryTurretSlotEntity(int slot);
    bool             FindExitPosition(Entity *pEnt, const Vector            &vOrigin, const Vector *vAngles); // added in 2.0
    virtual void     AttachPassengerSlot(int slot, Entity *ent, Vector vExitPosition);
    virtual void     AttachDriverSlot(int slot, Entity *ent, Vector vExitPosition);
    virtual void     AttachTurretSlot(int slot, Entity *ent, Vector vExitPosition, Vector *vExitAngles);
    virtual void     DetachPassengerSlot(int slot, Vector vExitPosition, Vector *vExitAngles);
    virtual void     DetachDriverSlot(int slot, Vector vExitPosition, Vector *vExitAngles);
    virtual void     DetachTurretSlot(int slot, Vector vExitPosition, Vector *vExitAngles);
    virtual int      FindPassengerSlotByEntity(Entity *ent);
    virtual int      FindDriverSlotByEntity(Entity *ent);
    virtual int      FindTurretSlotByEntity(Entity *ent);
    virtual void     SetSlotsNonSolid(void);
    virtual void     SetSlotsSolid(void);
    void             KickSuspension(Vector vDirection, float fForce);
    qboolean         isLocked(void);
    void             Lock(void);
    void             UnLock(void);
    const str      & getName() const; // added in 2.0
    VehicleCollisionEntity *GetCollisionEntity(void);
    // Added in 2.30
    //====
    void EventSetMaxUseAngle(Event *ev);
    void EventCanUse(Event *ev);
    //====
};

class DrivableVehicle : public Vehicle
{
public:
    CLASS_PROTOTYPE(DrivableVehicle);

    DrivableVehicle(void);

    virtual void Killed(Event *ev);
};

class VehicleWheelsX4 : public DrivableVehicle
{
private:
    wheel_t m_sWheels[4];

public:
    CLASS_PROTOTYPE(VehicleWheelsX4);

    VehicleWheelsX4();

    virtual void UpdateVariables(
        Vector *acceleration, Vector *vpn, Vector *vup, Vector *vright, Vector *t_vpn, Vector *t_vup, Vector *t_vright
    ) override;
    virtual float TorqueLookup(int rpm) override;
    virtual void  Think() override;
    virtual void  Postthink() override;
    virtual void  Killed(Event *ev) override;
};

class VehicleWheelsX2 : public DrivableVehicle
{
private:
    wheel_t m_sWheels[2];

public:
    CLASS_PROTOTYPE(VehicleWheelsX2);

    VehicleWheelsX2();

    virtual void UpdateVariables(
        Vector *acceleration, Vector *vpn, Vector *vup, Vector *vright, Vector *t_vpn, Vector *t_vup, Vector *t_vright
    ) override;
    virtual float TorqueLookup(int rpm) override;
    virtual void  Think() override;
    virtual void  Postthink() override;
    virtual void  Killed(Event *ev) override;
};

class VehicleTank : public DrivableVehicle
{
public:
    CLASS_PROTOTYPE(VehicleTank);

    VehicleTank();
    ~VehicleTank();

    virtual void     DriverAdded() override;
    virtual qboolean Drive(usercmd_t *ucmd) override;
    void             EventDamage(Event *ev);
    virtual void     Think() override;
    virtual void     Postthink() override;
    virtual void     CalculateOriginOffset() override;
    void             UpdateSound() override;
    void             AttachDriverSlot(Event *ev) override;
};

class VehicleHalfTrack : public DrivableVehicle
{
public:
    CLASS_PROTOTYPE(VehicleHalfTrack);

    VehicleHalfTrack();

    virtual void Think() override;
    virtual void Postthink() override;
    virtual void Killed(Event *ev) override;
};

typedef SafePtr<Vehicle> VehiclePtr;
