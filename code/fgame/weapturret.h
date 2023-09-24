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

// weapturret.h: MOH Turret. Usable by sentients and monsters.
//

#pragma once

#include "weapon.h"
#include "camera.h"

extern Event EV_Turret_Enter;
extern Event EV_Turret_Exit;
extern Event EV_Turret_AI_TurnSpeed;
extern Event EV_Turret_SetMaxUseAngle;

typedef enum {
    TURRETAISTATE_DEFAULT,
    TURRETAISTATE_TRACK,
    TURRETAISTATE_SUPPRESS,
    TURRETAISTATE_SUPPRESS_WAIT
} turretaistate_e;

#define FAKEBULLETS 1

class TurretGun : public Weapon
{
    friend class Vehicle;

protected:
    bool              m_bFakeBullets;
    bool              m_bPlayerUsable;
    bool              m_bUsable;
    bool              m_bRestable;
    Vector            m_vIdleCheckOffset;
    Vector            m_vViewOffset;
    float             m_fMaxIdlePitch;
    float             m_fMaxIdleYaw;
    Vector            m_vUserViewAng;
    Vector            m_vUserLastCmdAng;
    float             m_fIdlePitchSpeed;
    float             m_fTurnSpeed;
    float             m_fAIPitchSpeed;
    float             m_fPitchUpCap;
    float             m_fPitchDownCap;
    float             m_fStartYaw;
    float             m_fMaxYawOffset;
    float             m_fUserDistance;
    float             m_fMinBurstTime;
    float             m_fMaxBurstTime;
    float             m_fMinBurstDelay;
    float             m_fMaxBurstDelay;
    float             m_fFireToggleTime;
    int               m_iFiring;
    int               m_iIdleHitCount;
    CameraPtr         m_pUserCamera;
    float             m_fViewJitter;
    float             m_fCurrViewJitter;
    Vector            m_vAIBulletSpread[MAX_FIREMODES];
    int               m_iTargetType;
    SentientPtr       m_pAIEnemy;
    Vector            m_Aim_offset;
    SafePtr<Animate>  m_pViewModel;
    ScriptThreadLabel m_UseThread;

public:
    bool m_bHadOwner;

private:
    int    m_iAIState;
    float  m_fAIConvergeTime;
    int    m_iAISuppressTime;
    int    m_iAISuppressWaitTime;
    int    m_iAILastTrackTime;
    int    m_iAIStartSuppressTime;
    vec3_t m_vDesiredTargetAngles;
    float  m_fAIDesiredTargetSpeed;
    vec3_t m_vAIDesiredTargetPosition;
    vec3_t m_vAITargetPosition;
    vec3_t m_vAICurrentTargetPosition;
    vec3_t m_vAITargetSpeed;
    int    m_iAINextSuppressTime;
    float  m_fAISuppressWidth;
    float  m_fAISuppressHeight;
    Vector m_vMuzzlePosition;
    float  m_fMaxUseAngle;

protected:
    void         ThinkIdle();
    void         P_SetTargetAngles(Vector        &vTargAngles);
    virtual void P_ThinkActive();

public:
    CLASS_PROTOTYPE(TurretGun);

    TurretGun();
    virtual ~TurretGun();

    // added in 2.0
    //====
    void AI_SetTargetAngles(vec3_t vTargAngles, float speed);
    void AI_SetDesiredTargetAngles(const vec3_t angles, float speed);
    void AI_DoTargetNone();
    void AI_MoveToDefaultPosition();
    void AI_DoTargetAutoDefault();
    void AI_StartDefault();
    void AI_StartSuppress();
    void AI_StartSuppressWait();
    void AI_StartTrack();
    void AI_DoTargetAutoTrack();
    void AI_DoSuppressionAiming();
    void AI_DoTargetAutoSuppress();
    void AI_DoTargetAutoSuppressWait();
    void AI_DoTargetAuto();
    void AI_DoAiming();
    void AI_DoFiring();
    void AI_ThinkActive();
    //====

    void Think() override;

    void PlaceTurret(Event *ev);

    virtual void P_UserAim(usercmd_t *ucmd);
    qboolean     UserAim(usercmd_t *ucmd);
    virtual void TurretBeginUsed(Sentient *pEnt);
    virtual void P_TurretBeginUsed(Player *pEnt);
    virtual void AI_TurretBeginUsed(Sentient *pEnt); // added in 2.0
    virtual void TurretEndUsed();
    virtual void P_TurretEndUsed();
    virtual void AI_TurretEndUsed(); // added in 2.0
    void         RemoveUserCamera(); // added in 2.0
    virtual void TurretUsed(Sentient *pEnt);
    virtual void P_TurretUsed(Player *player);

    void TurretUsed(Event *ev);
    void P_SetPlayerUsable(Event *ev);
    void EventSetUsable(Event *ev); // added in 2.0
    void P_SetViewOffset(Event *ev);
    void EventMaxIdlePitch(Event *ev); // added in 2.0
    void EventMaxIdleYaw(Event *ev);   // added in 2.0
    void SetIdleCheckOffset(Event *ev);
    bool IsFiring();
    void CalcFiringViewJitter();
    void P_ApplyFiringViewJitter(Vector& vAng);
    void AI_TurnSpeed(float speed);
    void AI_PitchSpeed(float speed);
    void PitchCaps(float upcap, float downcap);
    void MaxYawOffset(float max);
    void YawCenter(float center);
    void P_UserDistance(float dist);
    void AI_BurstFireSettings(float min_bursttime, float max_bursttime, float min_burstdelay, float max_burstdelay);
    void AI_EventSetAimTarget(Event *ev);
    void AI_EventSetAimOffset(Event *ev);
    void AI_EventClearAimTarget(Event *ev);
    void AI_EventStartFiring(Event *ev);
    void AI_EventStopFiring(Event *ev);
    void AI_EventTurnSpeed(Event *ev);
    void AI_EventPitchSpeed(Event *ev); // added in 2.0
    void EventPitchCaps(Event *ev);
    void EventMaxYawOffset(Event *ev);
    void EventYawCenter(Event *ev);
    void P_EventUserDistance(Event *ev);
    void P_EventViewJitter(Event *ev);
    void P_EventDoJitter(Event *ev); // added in 2.0
    void AI_EventBurstFireSettings(Event *ev);
    void P_EventSetThread(Event *ev);
    void P_SetMaxUseAngle(Event *ev); // added in 2.0
    void P_SetStartYaw(Event *ev);    // added in 2.0
    void P_SetViewangles(Event *ev);
    void P_SetViewAnglesForTurret(Event *ev);
    void P_GetViewangles(Event *ev);
    void P_CreateViewModel();
    void P_DeleteViewModel();

    qboolean AI_SetWeaponAnim(const char *anim, Event *ev = NULL);
    qboolean P_SetWeaponAnim(const char *anim, Event *ev = NULL);
    qboolean SetWeaponAnim(const char *anim, Event *ev = NULL) override;
    void     AI_StopWeaponAnim();
    void     P_StopWeaponAnim();
    void     StopWeaponAnim() override;

    bool          AI_CanTarget(const vec3_t org);
    virtual float FireDelay(firemode_t mode);
    virtual void  SetFireDelay(Event *ev); // added in 2.0
    void          ShowInfo(float fDot, float fDist) override;

    // added in 2.0
    //====
    void AI_EventSetTargetType(Event *ev);
    void AI_EventGetTargetType(Event *ev);
    void AI_EventSetConvergeTime(Event *ev);
    void AI_EventSetSuppressTime(Event *ev);
    void AI_EventSetSuppressWaitTime(Event *ev);
    void AI_EventSetSuppressWidth(Event *ev);
    void AI_EventSetSuppressHeight(Event *ev);
    void AI_EventSetBulletSpread(Event *ev);
    void GetMuzzlePosition(
        vec3_t position, vec3_t vBarrelPos = NULL, vec3_t forward = NULL, vec3_t right = NULL, vec3_t up = NULL
    ) override;
    Vector EyePosition();
    void   setAngles(Vector ang) override;
    //====

    const Vector& GetUserViewAngles() const;
    void SetUserViewAngles(const Vector& vAngles);

    void Archive(Archiver& arc) override;
};
