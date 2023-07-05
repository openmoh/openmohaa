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
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110EV_DEFAULT301  USA
===========================================================================
*/

// weapturret.cpp: MOH Turret. Usable by sentients and monsters.
//

#include "g_phys.h"
#include "weapturret.h"
#include "player.h"

Event EV_Turret_IdleCheckOffset
(
	"idleCheckOffset",
	EV_DEFAULT,
	"v",
	"offset",
	"Sets the to trace to for collision checking when idling"
);

Event EV_Turret_ViewOffset
(
	"viewOffset",
	EV_DEFAULT,
	"v",
	"offset",
	"Sets the view to use for the turret"
);

Event EV_Turret_SetPlayerUsable
(
	"setPlayerUsable",
	EV_DEFAULT,
	"i",
	"state",
	"Sets wether the turret can be used by players. 0 means no, 1 means yes."
);

Event EV_Turret_SetAimTarget
(
	"setAimTarget",
	EV_DEFAULT,
	"S",
	"target",
	"Makes the turret aim at an entity"
);

Event EV_Turret_SetAimOffset
(
	"setAimOffset",
	EV_DEFAULT,
	"v",
	"offset",
	"Makes the turret aim with specified offset"
);

Event EV_Turret_ClearAimTarget
(
	"clearAimTarget",
	EV_DEFAULT,
	"S",
	"target",
	"Makes the turret aim at an entity"
);

Event EV_Turret_StartFiring
(
	"startFiring",
	EV_DEFAULT,
	0,
	0,
	"Makes the turret start shooting"
);

Event EV_Turret_StopFiring
(
	"stopFiring",
	EV_DEFAULT,
	0,
	0,
	"Makes the turret stop shooting"
);

Event EV_Turret_TurnSpeed
(
	"turnSpeed",
	EV_DEFAULT,
	"f",
	"speed",
	"Sets the turret's turn speed"
);

Event EV_Turret_PitchCaps
(
	"pitchCaps",
	EV_DEFAULT,
	"v",
	"caps",
	"Sets the pitch caps for the turret. First number is upward cap, second is downward cap, and the third just makes it nice little vector"
);

Event EV_Turret_MaxYawOffset
(
	"maxYawOffset",
	EV_DEFAULT,
	"f",
	"maxoffset",
	"Sets the max yaw from the turrets central facing direction"
);

Event EV_Turret_YawCenter
(
	"yawCenter",
	EV_DEFAULT,
	"f",
	"yaw",
	"Sets the yaw for the center of the turret's turning arc"
);

Event EV_Turret_UserDistance
(
	"userdistance",
	EV_DEFAULT,
	"f",
	"min_dist",
	"Sets the distance the user should be placed at while using this turret"
);

Event EV_Turret_ViewJitter
(
	"viewjitter",
	EV_DEFAULT,
	"f",
	"amount",
	"Sets the amount that the owner's view should be jittered when fired"
);

Event EV_Turret_BurstFireSettings
(
	"burstFireSettings",
	EV_DEFAULT,
	"ffff",
	"mintime maxtime mindelay maxdelay",
	"Sets the settings for burst mode firing"
);

Event EV_Turret_Enter
(
	"turretenter",
	EV_DEFAULT,
	"eS",
	"turret driver_anim",
	"Called when someone gets into a turret."
);

Event EV_Turret_Exit
(
	"turretexit",
	EV_DEFAULT,
	"e",
	"turret",
	"Called when driver gets out of the turret."
);

Event EV_Turret_SetThread
(
	"setthread",
	EV_DEFAULT,
	"s",
	"value",
	"Sets the name of the thread called when a player uses the turret"
);

CLASS_DECLARATION( Weapon, TurretGun, NULL )
{
	{ &EV_Trigger_Effect,					NULL },
	{ &EV_Item_DropToFloor,					&TurretGun::PlaceTurret },
	{ &EV_Item_Pickup,						NULL },
	{ &EV_Weapon_Shoot,						&TurretGun::Shoot },
	{ &EV_Use, 								&TurretGun::TurretUsed },
	{ &EV_Turret_SetPlayerUsable, 			&TurretGun::SetPlayerUsable },
	{ &EV_Turret_IdleCheckOffset, 			&TurretGun::SetIdleCheckOffset },
	{ &EV_Turret_ViewOffset, 				&TurretGun::SetViewOffset },
	{ &EV_Turret_SetAimTarget, 				&TurretGun::EventSetAimTarget },
	{ &EV_Turret_SetAimOffset, 				&TurretGun::EventSetAimOffset },
	{ &EV_Turret_ClearAimTarget, 			&TurretGun::EventClearAimTarget },
	{ &EV_Turret_StartFiring, 				&TurretGun::EventStartFiring },
	{ &EV_Turret_StopFiring, 				&TurretGun::EventStopFiring },
	{ &EV_Turret_TurnSpeed, 				&TurretGun::EventTurnSpeed },
	{ &EV_Turret_PitchCaps, 				&TurretGun::EventPitchCaps },
	{ &EV_Turret_MaxYawOffset, 				&TurretGun::EventMaxYawOffset },
	{ &EV_Turret_YawCenter, 				&TurretGun::EventYawCenter },
	{ &EV_Turret_UserDistance, 				&TurretGun::EventUserDistance },
	{ &EV_Turret_ViewJitter, 				&TurretGun::EventViewJitter },
	{ &EV_Turret_BurstFireSettings, 		&TurretGun::EventBurstFireSettings },
	{ &EV_Turret_SetThread, 				&TurretGun::EventSetThread },
	{ &EV_SetViewangles, 					&TurretGun::SetViewangles },
	{ &EV_GetViewangles, 					&TurretGun::GetViewangles },
	{ NULL, NULL }
};

TurretGun::TurretGun()
{
	entflags |= EF_TURRET;

	AddWaitTill(STRING_ONTARGET);

	if( LoadingSavegame )
	{
		return;
	}

	// turrets must not respawn
	// it can't be picked up
	setRespawn( qfalse );

	// allow monsters to use the turret
	respondto = TRIGGER_PLAYERS | TRIGGER_MONSTERS;

	// set the clipmask
	edict->clipmask = MASK_AUTOCALCLIFE;

	m_iIdleHitCount = 0;

	// set the fakebullets
	m_bFakeBullets = ( spawnflags & FAKEBULLETS );

	// make the turret already usable
	m_bUsable = true;
	m_bPlayerUsable = true;
	m_bRestable = true;

	m_fIdlePitchSpeed = 0;

	// set the size
	setSize( Vector( -16, -16, -8 ), Vector( 16, 16, 32 ) );

	// setup the turret angles cap
	m_fTurnSpeed = 180.0f;
	m_fPitchUpCap = -45.0f;
	m_fPitchDownCap = 45.0f;
	m_fMaxYawOffset = 180.0f;
	m_fUserDistance = 64.0f;

	// set the idle angles
	m_vIdleCheckOffset = Vector( -56, 0, 0 );

	// set the burst time
	m_fMinBurstTime = 0;
	m_fMaxBurstTime = 0;

	// set the burst delay
	m_fMinBurstDelay = 0;
	m_fMaxBurstDelay = 0;

	m_iFiring = 0;
	m_fFireToggleTime = level.time;

	// set the camera
	m_pUserCamera = NULL;

	// setup the view jitter
	m_fViewJitter = 0;
	m_fCurrViewJitter = 0;

	// turret doesn't have an owner when spawning
	m_bHadOwner = 0;

	m_pViewModel = NULL;
}

TurretGun::~TurretGun()
{
	Unregister( STRING_ONTARGET );

	entflags &= ~EF_TURRET;
}

void TurretGun::Archive
	(
	Archiver& arc
	)
{
	Weapon::Archive( arc );

	arc.ArchiveBool( &m_bFakeBullets );
	arc.ArchiveBool( &m_bPlayerUsable );

	arc.ArchiveFloat( &m_fIdlePitchSpeed );
	arc.ArchiveInteger( &m_iIdleHitCount );
	arc.ArchiveVector( &m_vIdleCheckOffset );
	arc.ArchiveVector( &m_vViewOffset );

	arc.ArchiveFloat( &m_fTurnSpeed );
	arc.ArchiveFloat( &m_fPitchUpCap );
	arc.ArchiveFloat( &m_fPitchDownCap );
	arc.ArchiveFloat( &m_fStartYaw );
	arc.ArchiveFloat( &m_fMaxYawOffset );
	arc.ArchiveFloat( &m_fUserDistance );

	arc.ArchiveFloat( &m_fMinBurstTime );
	arc.ArchiveFloat( &m_fMaxBurstTime );
	arc.ArchiveFloat( &m_fMinBurstDelay );
	arc.ArchiveFloat( &m_fMaxBurstDelay );

	arc.ArchiveFloat( &m_fFireToggleTime );
	arc.ArchiveInteger( &m_iFiring );
	arc.ArchiveVector( &m_vUserViewAng );
	arc.ArchiveSafePointer( &m_pUserCamera );

	arc.ArchiveFloat( &m_fViewJitter );
	arc.ArchiveFloat( &m_fCurrViewJitter );

	arc.ArchiveVector( &m_Aim_offset );
	arc.ArchiveSafePointer( &m_pViewModel );
	arc.ArchiveBool( &m_bUsable );

	m_UseThread.Archive( arc );
	
	arc.ArchiveBool( &m_bHadOwner );
	arc.ArchiveBool( &m_bRestable );

	if( arc.Loading() )
	{
		m_vUserLastCmdAng = vec_zero;
	}
}

void TurretGun::SetTargetAngles
	(
	Vector& vTargAngles
	)
{
	float fDiff;
	float fTurnAmount;

	if( vTargAngles[ 0 ] > 180.0f )
	{
		vTargAngles[ 0 ] -= 360.0f;
	}
	else if( vTargAngles[ 0 ] < -180.0f )
	{
		vTargAngles[ 0 ] += 360.0f;
	}

	if( vTargAngles[ 0 ] < m_fPitchUpCap )
	{
		vTargAngles[ 0 ] = m_fPitchUpCap;
	}

	if( vTargAngles[ 0 ] > m_fPitchDownCap )
	{
		vTargAngles[ 0 ] = m_fPitchDownCap;
	}

	if( owner && owner->IsSubclassOfPlayer() )
	{
		fTurnAmount = 180.0f;
	}
	else
	{
		fTurnAmount = level.frametime * m_fTurnSpeed;
	}

	fDiff = AngleSubtract( m_fPitchDownCap, angles[ 0 ] );
	if( fTurnAmount <= fabs( fDiff ) )
	{
		if( fDiff <= 0.0f )
		{
			angles[ 0 ] = angles[ 0 ] - fTurnAmount;
		}
		else
		{
			angles[ 0 ] = fTurnAmount + angles[ 0 ];
		}
	}
	else
	{
		angles[ 0 ] = vTargAngles[ 0 ];
	}

	fDiff = AngleSubtract( vTargAngles[ 1 ], m_fStartYaw );
	if( fDiff <= m_fMaxYawOffset )
	{
		vTargAngles[ 1 ] = fDiff + m_fStartYaw;

		if( -( m_fMaxYawOffset ) > fDiff )
		{
			vTargAngles[ 1 ] = m_fStartYaw - m_fMaxYawOffset;
		}
	}
	else
	{
		vTargAngles[ 1 ] = m_fMaxYawOffset + m_fStartYaw;
	}

	fDiff = AngleSubtract( vTargAngles[ 1 ], angles[ 1 ] );

	if( fTurnAmount <= fabs( fDiff ) )
	{
		if( fDiff <= 0.0f )
		{
			angles[ 1 ] = angles[ 1 ] - fTurnAmount;
		}
		else
		{
			angles[ 1 ] = angles[ 1 ] + fTurnAmount;
		}
	}
	else
	{
		angles[ 1 ] = vTargAngles[ 1 ];
	}

	setAngles( angles );

	if( fabs( fDiff ) < 2.0f )
	{
		Unregister( STRING_ONTARGET );
	}
}

void TurretGun::ThinkActive
	(
	void
	)
{
	Vector vTarg;
	Vector vTargAngles;
	Vector vDelta;
	Vector vAngles;

	if( ( owner ) && owner->IsSubclassOfPlayer() )
	{
		if( m_vUserViewAng[ 0 ] < m_fPitchUpCap )
		{
			m_vUserViewAng[ 0 ] = m_fPitchUpCap;
		}
		else if( m_vUserViewAng[ 0 ] > m_fPitchDownCap )
		{
			m_vUserViewAng[ 0 ] = m_fPitchDownCap;
		}

		float fDiff = AngleSubtract( m_vUserViewAng[ 1 ], m_fStartYaw );

		if( fDiff <= m_fMaxYawOffset )
		{
			m_vUserViewAng[ 1 ] = fDiff + m_fStartYaw;

			if( -( m_fMaxYawOffset ) > fDiff )
			{
				m_vUserViewAng[ 1 ] = m_fStartYaw - m_fMaxYawOffset;
			}
		}
		else
		{
			m_vUserViewAng[ 1 ] = m_fMaxYawOffset + m_fStartYaw;
		}

		owner->SetViewAngles( m_vUserViewAng );
		vTarg = owner->GunTarget( false ) - origin;
		vTargAngles = vTarg.toAngles();

		SetTargetAngles( vTargAngles );
	}
	else if( aim_target )
	{
		Vector vNewOfs;

		vDelta = aim_target->centroid - origin;
		vNewOfs = vDelta + m_Aim_offset;

		vectoangles( vDelta, vTargAngles );
		SetTargetAngles( vTargAngles );

		if( owner )
		{
			Vector forward;

			AngleVectorsLeft( angles, forward, NULL, NULL );

			origin = forward * vNewOfs.length();
		}
	}

	if( m_iFiring )
	{
		if( m_fMaxBurstTime == 0 || ( owner != NULL && owner->client ) )
		{
			m_iFiring = 2;
			if( ReadyToFire( FIRE_PRIMARY ) )
			{
				Fire( FIRE_PRIMARY );

				if( owner->IsSubclassOfPlayer() )
				{
					m_fCurrViewJitter = m_fViewJitter;
				}
			}
		}
		else if( m_iFiring != 2 )
		{
			if( level.time > m_fFireToggleTime )
			{
				m_iFiring = 2;
				m_fFireToggleTime = G_Random( m_fMaxBurstTime - m_fMinBurstTime ) + ( level.time + m_fMinBurstTime );
			}
		}
		else if( ReadyToFire( FIRE_PRIMARY ) )
		{
			Fire( FIRE_PRIMARY );

			if( owner->IsSubclassOfPlayer() )
			{
				m_fCurrViewJitter = m_fViewJitter;
			}
		}
		else if( level.time > m_fFireToggleTime )
		{
			m_iFiring = 1;
			m_fFireToggleTime = G_Random( m_fMaxBurstDelay - m_fMaxBurstDelay) + ( level.time + m_fMinBurstDelay );
		}
	}

	if( owner && owner->IsSubclassOfPlayer() )
	{
		vAngles = m_vUserViewAng;

		if( !m_pUserCamera ) {
			m_pUserCamera = new Camera;
		}

		if( m_fCurrViewJitter > 0.0f )
		{
			float x = ( float )( rand() & 0x7FFF );
			float y = ( float )( rand() & 0x7FFF );
			float z = ( float )( rand() & 0x7FFF );

			vAngles[ 0 ] += ( x * 0.00003f + x * 0.00003f - 1.0f ) * m_fCurrViewJitter;
			vAngles[ 1 ] += ( y * 0.00003f + y * 0.00003f - 1.0f ) * m_fCurrViewJitter;
			vAngles[ 2 ] += ( z * 0.00003f + z * 0.00003f - 1.0f ) * m_fCurrViewJitter;

			m_fCurrViewJitter -= level.frametime * 6.0f;

			if( m_fCurrViewJitter < 0.0f ) {
				m_fCurrViewJitter = 0.0f;
			}
		}

		m_pUserCamera->setOrigin( origin );
		m_pUserCamera->setAngles( vAngles );
		m_pUserCamera->SetPositionOffset( m_vViewOffset );

		owner->client->ps.camera_flags |= CF_CAMERA_ANGLES_ALLOWOFFSET;

		Player *player = ( Player * )owner.Pointer();

		if( !player->IsZoomed() )
		{
			player->ToggleZoom( 80 );
		}
	}

	if( owner && owner->client )
	{
		Vector vPos;
		Vector vEnd;
		Vector vAng;
		Vector vForward;
		Vector vMins;
		Vector vMaxs;
		trace_t trace;

		Vector( 0, angles[ 1 ], 0 ).AngleVectorsLeft( &vForward );

		vPos = origin - vForward * m_fUserDistance;
		vPos[ 2 ] -= 16.0f;

		vEnd = vPos;
		vEnd[ 2 ] -= 64.0f;

		vMins = owner->mins;
		vMaxs = owner->maxs;
		vMaxs[ 2 ] = owner->mins[ 2 ] + 4.0f;

		trace = G_Trace(
			vPos,
			vMins,
			vMaxs,
			vEnd,
			owner,
			MASK_PLAYERSOLID,
			qtrue,
			"TurretGun::ThinkActive 1"
			);

		vPos = trace.endpos;

		trace = G_Trace(
			vPos,
			owner->mins,
			owner->maxs,
			vPos,
			owner,
			MASK_PLAYERSOLID,
			qtrue,
			"TurretGun::ThinkActive 2"
			);

		if( !trace.allsolid && !trace.startsolid )
		{
			owner->setOrigin( vPos );
		}
	}
}

void TurretGun::ThinkIdle
	(
	void
	)
{
	Vector vDir, vNewAngles, vEnd;
	trace_t trace;

	if( !m_bRestable ) {
		return;
	}

	if( angles[ 0 ] > 180.0f )
	{
		angles[ 0 ] -= 360.0f;
	}

	if( angles[ 0 ] < -80.0f )
	{
		m_fIdlePitchSpeed = 0;
		m_iIdleHitCount = 0;
		return;
	}

	if( m_iIdleHitCount > 1 )
	{
		return;
	}

	m_fIdlePitchSpeed -= level.frametime * 300.0f;

	vNewAngles = Vector( angles[ 0 ] + level.frametime * m_fIdlePitchSpeed, angles[ 1 ], angles[ 2 ] );
	vNewAngles.AngleVectorsLeft( &vDir );
	vEnd = origin + vDir * m_vIdleCheckOffset[ 0 ];

	trace = G_Trace(
		origin,
		vec_zero,
		vec_zero,
		vEnd,
		this,
		edict->clipmask,
		false,
		"TurretGun::Think" );

	if( trace.fraction == 1.0f )
	{
		setAngles( vNewAngles );
		m_iIdleHitCount = 0;
		return;
	}

	int iTry;
	for( iTry = 3; iTry > 0; iTry-- )
	{
		vNewAngles[ 0 ] = angles[ 0 ] + level.frametime * m_fIdlePitchSpeed * iTry * 0.25f;
		vNewAngles.AngleVectorsLeft( &vDir );
		vEnd = origin + vDir * m_vIdleCheckOffset[ 0 ];

		trace = G_Trace(
			origin,
			vec_zero,
			vec_zero,
			vEnd,
			this,
			edict->clipmask,
			false,
			"TurretGun::Think" );

		if( trace.fraction == 1.0f )
		{
			setAngles( vNewAngles );

			m_iIdleHitCount = 0;
			m_fIdlePitchSpeed *= 0.25f * iTry;
			break;
		}
	}

	if( !iTry )
	{
		m_fIdlePitchSpeed = 0;

		Entity *ent = G_GetEntity( trace.entityNum );

		if( ent && ent == world )
		{
			m_iIdleHitCount++;
		}
		else
		{
			m_iIdleHitCount = 0;
		}
	}
}

void TurretGun::Think
	(
	void
	)
{
	if (owner || (!m_bHadOwner && aim_target))
	{
		ThinkActive();
	}
	else
	{
		ThinkIdle();
	}
}

qboolean TurretGun::UserAim
	(
	usercmd_s *ucmd
	)
{
	Vector vNewCmdAng;

	if( owner == NULL )
	{
		return qfalse;
	}

	vNewCmdAng = Vector( ucmd->angles[ 0 ], ucmd->angles[ 1 ], ucmd->angles[ 2 ] ) * 0.005493f;

	if( vNewCmdAng[ 0 ] || vNewCmdAng[ 1 ] || vNewCmdAng[ 2 ] )
	{
		m_vUserViewAng[ 0 ] += AngleSubtract( vNewCmdAng[ 0 ], m_vUserLastCmdAng[ 0 ] );
		m_vUserViewAng[ 1 ] += AngleSubtract( vNewCmdAng[ 1 ], m_vUserLastCmdAng[ 1 ] );
		m_vUserViewAng[ 2 ] += AngleSubtract( vNewCmdAng[ 2 ], m_vUserLastCmdAng[ 2 ] );
	}

	m_vUserLastCmdAng = vNewCmdAng;

	if( ( ucmd->buttons & BUTTON_ATTACKLEFT ) || ( ucmd->buttons & BUTTON_ATTACKRIGHT ) )
	{
		if( !m_iFiring ) {
			m_iFiring = 1;
		}
	}
	else
	{
		m_iFiring = 0;
	}

	flags |= FL_THINK;

	return qtrue;
}

void TurretGun::PlaceTurret
	(
	Event *ev
	)
{
	// Don't make the turret solid
	setSolidType( SOLID_NOT );

	// The turret shouldn't move
	setMoveType( MOVETYPE_NONE );

	showModel();

	groundentity = NULL;
	m_fStartYaw = angles[ 1 ];

	if( m_bFakeBullets ) {
		firetype[ FIRE_PRIMARY ] = FT_FAKEBULLET;
	}

	flags |= FL_THINK;
}

void TurretGun::TurretBeginUsed
	(
	Sentient *pEnt
	)
{
	owner = pEnt;

	edict->r.ownerNum = pEnt->entnum;
	m_bHadOwner = true;

	Sound( sPickupSound );

	if( m_vUserViewAng[ 0 ] > 180.0f ) {
		m_vUserViewAng[ 0 ] -= -360.0f;
	}

	m_vUserLastCmdAng = vec_zero;

	if( owner->IsSubclassOfPlayer() )
	{
		Player *player = ( Player * )owner.Pointer();

		player->EnterTurret( this );

		if( !m_pUserCamera ) {
			m_pUserCamera = new Camera;
		}

		player->SetCamera( m_pUserCamera, 0.0f );
	}

	current_attachToTag = "";
	ForceIdle();
	CreateViewModel();
}

void TurretGun::TurretEndUsed
	(
	void
	)
{
	if( owner->IsSubclassOfPlayer() )
	{
		Player *player = ( Player * )owner.Pointer();

		if( m_pUserCamera )
		{
			player->SetCamera( NULL, 1.0f );
			player->ZoomOff();
			player->client->ps.camera_flags &= ~CF_CAMERA_ANGLES_TURRETMODE;

			m_pUserCamera->PostEvent( EV_Remove, 0 );
			m_pUserCamera = NULL;
		}

		player->ExitTurret();

		if( m_pViewModel )
		{
			delete m_pViewModel;
			m_pViewModel = NULL;

			// Make the turret visible to everyone
			edict->r.svFlags &= ~SVF_PORTAL;
		}
	}

	owner = NULL;
	edict->r.ownerNum = ENTITYNUM_NONE;

	m_fIdlePitchSpeed = 0;
	m_iIdleHitCount = 0;
	m_iFiring = 0;
}

void TurretGun::TurretUsed
	(
	Sentient *pEnt
	)
{
	if( ( pEnt->IsSubclassOfPlayer() ) && ( pEnt == owner ) )
	{
		if( ( !m_bPlayerUsable || !m_bUsable ) &&
			owner->health > 0.0f )
		{
			return;
		}
	}

	if( owner )
	{
		if( owner == pEnt )
		{
			TurretEndUsed();
			m_iFiring = 0;
		}
	}
	else
	{
		m_vUserViewAng = pEnt->GetViewAngles();

		if( fabs( AngleSubtract( m_vUserViewAng[ 1 ], angles[ 1 ] ) ) <= 80.0f )
		{
			TurretBeginUsed( pEnt );

			flags &= ~FL_THINK;
			m_iFiring = 0;

			if( pEnt->IsSubclassOfPlayer() )
			{
				m_UseThread.Execute( this );
			}
		}
	}
}

void TurretGun::SetAimTarget
	(
	Entity *ent
	)
{
	aim_target = ent;
}

void TurretGun::SetAimOffset
	(
	const Vector& offset
	)
{
	m_Aim_offset = offset;
}

void TurretGun::ClearAimTarget
	(
	void
	)
{
	aim_target = NULL;

	// Clear idle values
	m_fIdlePitchSpeed = 0;
	m_iIdleHitCount = 0;
}

void TurretGun::StartFiring
	(
	void
	)
{
	m_iFiring = 1;
}

void TurretGun::StopFiring
	(
	void
	)
{
	m_iFiring = 0;
}

bool TurretGun::IsFiring
	(
	void
	)
{
	return m_iFiring == 2;
}

void TurretGun::CalcFiringViewJitter
	(
	void
	)
{
	if( owner && owner->IsSubclassOfPlayer() )
	{
		m_fCurrViewJitter = m_fViewJitter;
	}
}

void TurretGun::ApplyFiringViewJitter
	(
	Vector& vAng
	)
{
}

void TurretGun::TurnSpeed
	(
	float speed
	)
{
	m_fTurnSpeed = speed;
}

void TurretGun::PitchCaps
	(
	float upcap,
	float downcap
	)
{
	m_fPitchUpCap = upcap;
	m_fPitchDownCap = downcap;

	if( upcap > downcap )
	{
		m_fPitchUpCap = -30.0f;
		m_fPitchDownCap = 10.0f;
	}
}

void TurretGun::MaxYawOffset
	(
	float max
	)
{
	m_fMaxYawOffset = max;
	if( max < 0.0f )
	{
		m_fMaxYawOffset = 0.0f;
	}
	else if( max > 180.0f )
	{
		m_fMaxYawOffset = 180.0f;
	}
}

void TurretGun::YawCenter
	(
	float center
	)
{
	m_fStartYaw = center;
}

void TurretGun::UserDistance
	(
	float dist
	)
{
	m_fUserDistance = dist;
}

void TurretGun::BurstFireSettings
	(
	float min_bursttime,
	float max_bursttime,
	float min_burstdelay,
	float max_burstdelay
	)
{
	m_fMinBurstTime = min_bursttime;
	m_fMaxBurstTime = max_bursttime;
	m_fMinBurstDelay = min_burstdelay;
	m_fMaxBurstDelay = max_burstdelay;
}

void TurretGun::CreateViewModel
	(
	void
	)
{
	char newmodel[ MAX_STRING_TOKENS ];
	int tagnum;

	// Owner must be a client
	if( !owner->IsSubclassOfPlayer() ) {
		return;
	}

	m_pViewModel = new Animate;

	COM_StripExtension( model.c_str(), newmodel, sizeof( newmodel ) );
	strcat( newmodel, "_viewmodel.tik" );

	m_pViewModel->setScale( edict->s.scale );
	m_pViewModel->detach_at_death = qtrue;
	m_pViewModel->setModel( newmodel );
	m_pViewModel->edict->s.renderfx |= RF_VIEWMODEL;

	if( !m_pViewModel->edict->tiki )
	{
		delete m_pViewModel;
		m_pViewModel = NULL;

		warning( "CreateViewModel", "Couldn't find turret view model tiki %s", newmodel );
		return;
	}

	// Get the eyes bone
	tagnum = gi.Tag_NumForName( owner->edict->tiki, "eyes bone" );
	if( tagnum < 0 )
	{
		warning( "CreateViewModel", "Tag eyes bone not found" );
	}
	else if( !m_pViewModel->attach( owner->entnum, tagnum ) )
	{
		delete m_pViewModel;
		m_pViewModel = NULL;

		warning( "CreateViewModel", "Could not attach model %s", newmodel );
		return;
	}

	m_pViewModel->NewAnim( "idle" );

	// Make the world model invisible to the owner
	edict->r.svFlags |= SVF_PORTAL;
	edict->r.singleClient |= owner->edict->s.number;

	// Make the viewmodel visible only to the owner
	m_pViewModel->edict->r.svFlags |= SVF_SINGLECLIENT;
	m_pViewModel->edict->r.singleClient |= owner->edict->s.number;
}

void TurretGun::DeleteViewModel
	(
	void
	)
{
	delete m_pViewModel;
	m_pViewModel = NULL;

	edict->r.svFlags &= ~SVF_PORTAL;
}

qboolean TurretGun::SetWeaponAnim
	(
	const char *anim,
	Event *ev
	)
{
	int slot;
	int animnum;

	if( !Weapon::SetWeaponAnim( anim, ev ) )
	{
		return qfalse;
	}

	if( !m_pViewModel )
	{
		return qtrue;
	}

	slot = ( m_iAnimSlot + 3 ) & 3;

	animnum = gi.Anim_NumForName( m_pViewModel->edict->tiki, anim );
	if( animnum < 0 )
	{
		return qtrue;
	}

	m_pViewModel->StopAnimating( slot );
	m_pViewModel->SetTime( slot );

	m_pViewModel->edict->s.frameInfo[ slot ].index = gi.Anim_NumForName( m_pViewModel->edict->tiki, "idle" );

	m_pViewModel->NewAnim( animnum, m_iAnimSlot );
	m_pViewModel->SetOnceType();
	m_pViewModel->SetTime( slot );

	return qtrue;
}

void TurretGun::StopWeaponAnim
	(
	void
	)
{
	if( m_pViewModel )
	{
		m_pViewModel->SetTime( m_iAnimSlot );
		m_pViewModel->StopAnimating( m_iAnimSlot );

		m_pViewModel->edict->s.frameInfo[ m_iAnimSlot ].index = gi.Anim_NumForName( m_pViewModel->edict->tiki, "idle" );
		m_pViewModel->edict->s.frameInfo[ m_iAnimSlot ].weight = 1.0f;
		m_pViewModel->edict->s.frameInfo[ m_iAnimSlot ].time = 0.0f;
	}

	Weapon::StopWeaponAnim();
}

bool TurretGun::CanTarget
	(
	float *pos
	)
{
	Vector vAngles;
	float ang;

	vectoangles( pos, vAngles );

	ang = AngleSubtract( vAngles[ 1 ], m_fStartYaw );

	if( vAngles[ 0 ] <= 180.0f )
	{
		if( vAngles[ 0 ] >= -180.0f )
		{
			return m_fPitchUpCap <= vAngles[ 0 ] &&
				m_fPitchDownCap >= vAngles[ 0 ] &&
				ang <= m_fMaxYawOffset &&
				-m_fMaxYawOffset <= ang;
		}

		vAngles[ 0 ] += 360.0f;
	}
	else
	{
		vAngles[ 0 ] -= 360.0f;
	}

	return m_fPitchUpCap <= vAngles[ 0 ] &&
		m_fPitchDownCap >= vAngles[ 0 ] &&
		ang <= m_fMaxYawOffset &&
		-m_fMaxYawOffset <= ang;
}

float TurretGun::FireDelay
	(
	firemode_t mode
	)
{
	if( owner && owner->IsSubclassOfPlayer() )
	{
		return 0.06f;
	}
	else
	{
		return Weapon::FireDelay( mode );
	}
}

void TurretGun::ShowInfo
	(
	float fDot,
	float fDist
	)
{
	if( fDot <= 0.90f && ( fDot <= 0.0f || fDist >= 256.0f ) ) {
		return;
	}

	if( fDist >= 2048.0f || fDist <= 64.0f ) {
		return;
	}

	G_DebugString( origin + Vector( 0, 0, maxs[ 2 ] + 56.0f ), 1.0f, 1.0f, 1.0f, 1.0f, "%d:%d:%s", entnum, radnum, targetname.c_str() );
	G_DebugString( origin + Vector( 0, 0, maxs[ 2 ] + 38.0f ), 1.0f, 1.0f, 1.0f, 1.0f, "aim_target: %d", aim_target ? aim_target->entnum : -1 );
	G_DebugString( origin + Vector( 0, 0, maxs[ 2 ] + 20.0f ), 1.0f, 1.0f, 1.0f, 1.0f, "owner: %d", owner ? owner->entnum : -1 );
}

void TurretGun::TurretUsed
	(
	Event *ev
	)
{
	Entity *pEnt = ev->GetEntity( 1 );

	if( !pEnt ) {
		return;
	}

	// Must be a player
	if( !pEnt->IsSubclassOfPlayer() ) {
		return;
	}

	if( !m_bUsable ) {
		return;
	}

	if( m_bPlayerUsable )
	{
		// Make the sentient use the turret
		TurretUsed( ( Sentient * )pEnt );
	}
}

void TurretGun::SetPlayerUsable
	(
	Event *ev
	)
{
	m_bPlayerUsable = ( ev->GetInteger( 1 ) != 0 );
}

void TurretGun::SetViewOffset
	(
	Event *ev
	)
{
	m_vViewOffset = ev->GetVector( 1 );
}

void TurretGun::SetIdleCheckOffset
	(
	Event *ev
	)
{
	m_vIdleCheckOffset = ev->GetVector( 1 );
}

void TurretGun::EventSetAimTarget
	(
	Event *ev
	)
{
	SetAimTarget( ev->GetEntity( 1 ) );
}

void TurretGun::EventSetAimOffset
	(
	Event *ev
	)
{
	SetAimOffset( ev->GetVector( 1 ) );
}

void TurretGun::EventClearAimTarget
	(
	Event *ev
	)
{
	ClearAimTarget();
}

void TurretGun::EventStartFiring
	(
	Event *ev
	)
{
	StartFiring();
}

void TurretGun::EventStopFiring
	(
	Event *ev
	)
{
	StopFiring();
}

void TurretGun::EventTurnSpeed
	(
	Event *ev
	)
{
	TurnSpeed( ev->GetFloat( 1 ) );
}

void TurretGun::EventPitchCaps
	(
	Event *ev
	)
{
	Vector caps = ev->GetVector( 1 );
	PitchCaps( caps[ 0 ], caps[ 1 ] );
}

void TurretGun::EventMaxYawOffset
	(
	Event *ev
	)
{
	MaxYawOffset( ev->GetFloat( 1 ) );
}

void TurretGun::EventYawCenter
	(
	Event *ev
	)
{
	YawCenter( ev->GetFloat( 1 ) );
}

void TurretGun::EventUserDistance
	(
	Event *ev
	)
{
	UserDistance( ev->GetFloat( 1 ) );
}

void TurretGun::EventViewJitter
	(
	Event *ev
	)
{
	m_fViewJitter = ev->GetFloat( 1 );
}

void TurretGun::EventBurstFireSettings
	(
	Event *ev
	)
{
	if( ev->NumArgs() <= 3 ) {
		return;
	}

	BurstFireSettings( ev->GetFloat( 1 ), ev->GetFloat( 2 ), ev->GetFloat( 3 ), ev->GetFloat( 4 ) );
}

void TurretGun::EventSetThread
	(
	Event *ev
	)
{
	if( ev->IsFromScript() )
	{
		m_UseThread.SetThread( ev->GetValue( 1 ) );
	}
	else
	{
		m_UseThread.Set( ev->GetString( 1 ) );
	}
}

void TurretGun::SetViewangles
	(
	Event *ev
	)
{
	m_vUserViewAng = ev->GetVector( 1 );
}

void TurretGun::GetViewangles
	(
	Event *ev
	)
{
	ev->AddVector( m_vUserViewAng );
}

