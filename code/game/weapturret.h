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

// weapturret.h: MOH Turret. Usable by sentients and monsters.
//

#ifndef __WEAPTURRET_H__
#define __WEAPTURRET_H__

#include "weapon.h"
#include "camera.h"

extern Event EV_Turret_Enter;
extern Event EV_Turret_Exit;
extern Event EV_Turret_TurnSpeed;

#define FAKEBULLETS			1

class TurretGun : public Weapon {
	friend class Vehicle;

protected:
	bool m_bFakeBullets;
	bool m_bPlayerUsable;
	bool m_bUsable;
	bool m_bRestable;
	Vector m_vIdleCheckOffset;
	Vector m_vViewOffset;
	Vector m_vUserViewAng;
	Vector m_vUserLastCmdAng;
	float m_fIdlePitchSpeed;
	float m_fTurnSpeed;
	float m_fPitchUpCap;
	float m_fPitchDownCap;
	float m_fStartYaw;
	float m_fMaxYawOffset;
	float m_fUserDistance;
	float m_fMinBurstTime;
	float m_fMaxBurstTime;
	float m_fMinBurstDelay;
	float m_fMaxBurstDelay;
	float m_fFireToggleTime;
	int m_iFiring;
	int m_iIdleHitCount;
	CameraPtr m_pUserCamera;
	float m_fViewJitter;
	float m_fCurrViewJitter;
	Vector m_Aim_offset;
	SafePtr<Animate> m_pViewModel;
	ScriptThreadLabel m_UseThread;

public:
	bool m_bHadOwner;

protected:
	void ThinkIdle( void );
	void SetTargetAngles( Vector& vTargAngles );
	void ThinkActive( void );

public:
	CLASS_PROTOTYPE( TurretGun );

	TurretGun();
	virtual ~TurretGun();

	virtual void Think( void );

	qboolean UserAim( usercmd_s *ucmd );
	void PlaceTurret( Event *ev );

	virtual void TurretBeginUsed( Sentient *pEnt );
	virtual void TurretEndUsed( void );
	virtual void TurretUsed( Sentient *pEnt );

	void TurretUsed( Event *ev );
	void SetPlayerUsable( Event *ev );
	void SetViewOffset( Event *ev );
	void SetIdleCheckOffset( Event *ev );
	void SetAimTarget( Entity *ent );
	void SetAimOffset( const Vector& offset );
	void ClearAimTarget( void );
	void StartFiring( void );
	void StopFiring( void );
	bool IsFiring( void );
	void CalcFiringViewJitter( void );
	void ApplyFiringViewJitter( Vector& vAng );
	void TurnSpeed( float speed );
	void PitchCaps( float upcap, float downcap );
	void MaxYawOffset( float max );
	void YawCenter( float center );
	void UserDistance( float dist );
	void BurstFireSettings( float min_bursttime, float max_bursttime, float min_burstdelay, float max_burstdelay );
	void EventSetAimTarget( Event *ev );
	void EventSetAimOffset( Event *ev );
	void EventClearAimTarget( Event *ev );
	void EventStartFiring( Event *ev );
	void EventStopFiring( Event *ev );
	void EventTurnSpeed( Event *ev );
	void EventPitchCaps( Event *ev );
	void EventMaxYawOffset( Event *ev );
	void EventYawCenter( Event *ev );
	void EventUserDistance( Event *ev );
	void EventViewJitter( Event *ev );
	void EventBurstFireSettings( Event *ev );
	void EventSetThread( Event *ev );
	void SetViewangles( Event *ev );
	void GetViewangles( Event *ev );
	void CreateViewModel( void );
	void DeleteViewModel( void );

	virtual qboolean SetWeaponAnim( const char *anim, Event *ev = NULL );
	virtual void StopWeaponAnim( void );

	virtual bool CanTarget( float *pos );
	virtual float FireDelay( firemode_t mode );
	virtual void ShowInfo( float fDot, float fDist );

	virtual void Archive( Archiver& arc );
};

#endif // __WEAPTURRET_H__
