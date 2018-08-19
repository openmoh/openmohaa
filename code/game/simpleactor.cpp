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

// simpleactor.cpp: Base class for character AI

#include "simpleactor.h"

CLASS_DECLARATION( Sentient, SimpleActor, NULL )
{
	{ NULL, NULL }
};

#define OVERLOADED_ERROR() assert( !"overloaded version should always get called" )

SimpleActor::SimpleActor()
{
	if( LoadingSavegame )
		return;

	m_AnimMotionHigh			= true;
	m_DesiredGunDir				= vec_zero;
	m_eEmotionMode				= 1;
	m_eNextAnimMode				= -1;
	m_fPathGoalTime				= 0.0f;
	m_csPathGoalEndAnimScript	= STRING_EMPTY;

	m_AnimActionHigh			= 1;
	m_AnimDialogHigh			= 1;
	m_fAimLimit_up				= 60.0f;
	m_fAimLimit_down			= -60.0f;

	m_csNextAnimString			= 0;
	m_bNextForceStart			= false;
	m_fCrossblendTime			= 0.5f;
	m_csCurrentPosition			= STRING_STAND;
	m_bStartPathGoalEndAnim		= false;

	for( int i = 0; i < MAX_FRAMEINFOS; i++ )
	{
		m_weightType[ i ] = 0.0f;
	}

	m_bMotionAnimSet			= false;
	m_ChangeActionAnimIndex		= -1;
	m_ChangeActionAnimIndex		= -1;
	m_ChangeSayAnimIndex		= -1;
	m_iMotionSlot				= -1;
	m_iActionSlot				= -1;
	m_bActionAnimSet			= false;
	m_bSayAnimSet				= false;
	m_iVoiceTime				= 0;
	m_bAimAnimSet				= false;
	m_iSaySlot					= -1;
	m_bLevelMotionAnim			= false;
	m_bLevelActionAnim			= false;
	m_bLevelSayAnim				= false;
	m_bNextLevelSayAnim			= false;
	m_DesiredYaw				= 0.0f;
	m_YawAchieved				= true;
	m_bPathErrorTime			= -10000000.0f;

	m_PainHandler.TrySetScript( STRING_ANIM_PAIN_SCR );
	m_DeathHandler.TrySetScript( STRING_ANIM_KILLED_SCR );
	m_AttackHandler.TrySetScript( STRING_ANIM_ATTACK_SCR );
	m_SniperHandler.TrySetScript( STRING_ANIM_SNIPER_SCR );

	m_bHasDesiredLookDest	= false;
	m_bUpdateAnimDoneFlags	= false;

	m_NearestNode			= NULL;
	m_fCrouchWeight			= 0.0f;

	m_csMood				= STRING_BORED;
	m_csIdleMood			= STRING_BORED;

	m_groundPlane			= qfalse;
	m_walking				= qfalse;
	m_groundPlaneNormal		= vec_zero;
	m_maxspeed				= 1000000.0f;
}

void SimpleActor::Archive
	(
	Archiver& arc
	)

{
	Sentient::Archive( arc );

	arc.ArchiveInteger( &m_eAnimMode );
	m_Anim.Archive( arc );

	arc.ArchiveBool( &m_bHasDesiredLookDest );
	arc.ArchiveBool( &m_bHasDesiredLookAngles );
	arc.ArchiveVector( &m_vDesiredLookDest );
	arc.ArchiveVec3( m_DesiredLookAngles );
	arc.ArchiveVec3( m_DesiredGunDir );

	m_Path.Archive( arc );
	arc.ArchiveVec3( m_Dest );
	arc.ArchiveVec3( m_NoClipDest );

	arc.ArchiveFloat( &path_failed_time );
	arc.ArchiveFloat( &m_fPathGoalTime );
	arc.ArchiveBool( &m_bStartPathGoalEndAnim );
	Director.ArchiveString( arc, m_csPathGoalEndAnimScript );

	arc.ArchiveInteger( &m_eNextAnimMode );
	Director.ArchiveString( arc, m_csNextAnimString );
	m_NextAnimLabel.Archive( arc );
	arc.ArchiveBool( &m_bNextForceStart );

	arc.ArchiveBoolean( &m_walking );
	arc.ArchiveBoolean( &m_groundPlane );
	arc.ArchiveVec3( m_groundPlaneNormal );

	arc.ArchiveVector( &watch_offset );
	arc.ArchiveBool( &m_bThink );
	arc.ArchiveInteger( &m_PainTime );

	arc.ArchiveBool( &m_bAimAnimSet );
	arc.ArchiveBool( &m_bActionAnimSet );

	Director.ArchiveString( arc, m_csMood );
	Director.ArchiveString( arc, m_csIdleMood );
	arc.ArchiveInteger( &m_eEmotionMode );

	arc.ArchiveFloat( &m_fAimLimit_up );
	arc.ArchiveFloat( &m_fAimLimit_down );

	for( int i = MAX_FRAMEINFOS - 1; i >= 0; i-- )
	{
		arc.ArchiveUnsigned( &m_weightType[ i ] );
	}

	for( int i = MAX_FRAMEINFOS - 1; i >= 0; i-- )
	{
		arc.ArchiveFloat( &m_weightBase[ i ] );
	}

	for( int i = MAX_FRAMEINFOS - 1; i >= 0; i-- )
	{
		arc.ArchiveFloat( &m_weightCrossBlend[ i ] );
	}

	arc.ArchiveBool( &m_AnimMotionHigh );
	arc.ArchiveBool( &m_AnimActionHigh );
	arc.ArchiveBool( &m_AnimDialogHigh );

	arc.ArchiveVec2( obstacle_vel );

	Director.ArchiveString( arc, m_csCurrentPosition );

	arc.ArchiveBool( &m_bMotionAnimSet );
	arc.ArchiveBool( &m_bDoAI );

	arc.ArchiveFloat( &m_fCrossblendTime );

	arc.ArchiveSafePointer( &m_pAnimThread );

	arc.ArchiveBool( &m_YawAchieved );
	arc.ArchiveFloat( &m_DesiredYaw );

	arc.ArchiveInteger( &m_iVoiceTime );
	arc.ArchiveBool( &m_bSayAnimSet );

	arc.ArchiveInteger( &hit_obstacle_time );

	Director.ArchiveString( arc, m_csAnimName );

	arc.ArchiveInteger( &m_bPathErrorTime );
	arc.ArchiveInteger( &m_iMotionSlot );
	arc.ArchiveInteger( &m_iActionSlot );
	arc.ArchiveInteger( &m_iSaySlot );

	arc.ArchiveBool( &m_bLevelMotionAnim );
	arc.ArchiveBool( &m_bLevelActionAnim );
	arc.ArchiveByte( &m_bLevelSayAnim );
	arc.ArchiveByte( &m_bNextLevelSayAnim );

	Director.ArchiveString( arc, m_csSayAnim );
	Director.ArchiveString( arc, m_csUpperAnim );

	m_PainHandler.Archive( arc );
	m_DeathHandler.Archive( arc );
	m_AttackHandler.Archive( arc );
	m_SniperHandler.Archive( arc );

	arc.ArchiveObjectPointer( ( Class ** )&m_NearestNode );
	arc.ArchiveVector( &m_vNearestNodePos );

	arc.ArchiveFloat( &m_fCrouchWeight );
	arc.ArchiveFloat( &m_maxspeed );
}

void SimpleActor::SetMoveInfo
	(
	mmove_t *
	)

{
	OVERLOADED_ERROR();
}

void SimpleActor::GetMoveInfo
(
mmove_t *
)

{
	OVERLOADED_ERROR();
}

void SimpleActor::StopAnimating
	(
	int slot
	)

{
	int index = 0;

	groundplane.normal[ slot ] = 0;
	DoExitCommands( slot );

	if( edict->s.frameInfo[ slot ].index || gi.TIKI_NumAnims( edict->tiki ) <= 1 )
	{
		edict->s.frameInfo[ slot ].index = 0;
	}
	else
	{
		edict->s.frameInfo[ slot ].index = 1;
	}

	animFlags[ slot ] = ANIM_LOOP | ANIM_NODELTA | ANIM_NOEXIT | ANIM_PAUSED;
	edict->s.frameInfo[ slot ].weight = 0;
	animtimes[ slot ] = 0;
	animFlags[ slot ] = ( animFlags[ slot ] | ANIM_NODELTA ) & ANIM_FINISHED;
}

void SimpleActor::AnimFinished
	(
	int slot
	)

{
	DumpCallTrace( "" );
	assert( !"" );
}

bool SimpleActor::CanTarget
	(
	void
	)

{
	OVERLOADED_ERROR();
	return 0;
}

bool SimpleActor::IsImmortal
	(
	void
	)

{
	OVERLOADED_ERROR();
	return 0;
}

bool SimpleActor::DoesTheoreticPathExist
	(
	Vector vDestPos,
	float fMaxPath
	)

{
	return m_Path.DoesTheoreticPathExist( origin, vDestPos, this, fMaxPath, 0, 0 );
}

void SimpleActor::SetPath
	(
	Vector vDestPos,
	const char *description,
	int iMaxDirtyTime,
	float *vLeashHome,
	float fLeashDistSquared
	)

{
	// FIXME: stub
	STUB();
}

void SimpleActor::SetPath
	(
	SimpleEntity *pDestNode,
	char *description,
	int iMaxDirtyTime
	)

{
	// FIXME: stub
	STUB();
}

void SimpleActor::SetPathWithinDistance
	(
	Vector vDestPos,
	char *description,
	float fMaxPath,
	int iMaxDirtyTime
	)

{
	// FIXME: stub
	STUB();
}

void SimpleActor::FindPathAway
	(
	vec3_t vAwayFrom,
	vec3_t vDirPreferred,
	float fMinSafeDist
	)

{
	// FIXME: stub
	STUB();
}

void SimpleActor::ClearPath
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

bool SimpleActor::PathComplete
	(
	void
	) const

{
	if( level.time >= m_fPathGoalTime )
	{
		if( m_Path.Complete( origin ) )
			return true;
	}

	return false;
}

bool SimpleActor::PathExists
	(
	void
	) const

{
	return m_Path.CurrentNode() != NULL;
}

bool SimpleActor::PathIsValid
	(
	void
	) const

{
	return true;
}

bool SimpleActor::PathAvoidsSquadMates
	(
	void
	) const

{
	// FIXME: stub
	STUB();
	return false;
}

void SimpleActor::ShortenPathToAvoidSquadMates
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

PathInfo *SimpleActor::CurrentPathNode
	(
	void
	) const

{
	return m_Path.CurrentNode();
}

PathInfo *SimpleActor::LastPathNode
	(
	void
	) const

{
	return m_Path.LastNode();
}

float SimpleActor::PathDist
	(
	void
	) const

{
	return m_Path.TotalDist();
}

bool SimpleActor::PathHasCompleteLookahead
	(
	void
	) const

{
	return m_Path.HasCompleteLookahead();
}

Vector SimpleActor::PathGoal
	(
	void
	) const

{
	return m_Path.CurrentPathGoal();
}

float *SimpleActor::PathDelta
	(
	void
	) const

{
	return m_Path.CurrentDelta();
}

bool SimpleActor::PathGoalSlowdownStarted
	(
	void
	) const

{
	return m_fPathGoalTime >= level.time;
}

void SimpleActor::SetDest
	(
	vec3_t dest
	)

{
	// FIXME: stub
	STUB();
}

void SimpleActor::StopTurning
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void SimpleActor::SetDesiredYaw
	(
	float yaw
	)

{
	// FIXME: stub
	STUB();
}

void SimpleActor::SetDesiredYawDir
	(
	vec3_t vec
	)

{
	// FIXME: stub
	STUB();
}

void SimpleActor::SetDesiredYawDest
	(
	vec3_t vec
	)

{
	// FIXME: stub
	STUB();
}

void SimpleActor::UpdateEmotion
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

int SimpleActor::GetEmotionAnim
	(
	void
	)

{
	// FIXME: stub
	STUB();
	return 0;
}

int SimpleActor::GetMotionSlot
	(
	int slot
	)

{
	if( m_AnimMotionHigh )
		return slot + 3;
	else
		return slot;
}

int SimpleActor::GetActionSlot
	(
	int slot
	)

{
	if( m_AnimActionHigh )
		return slot + 9;
	else
		return slot + 6;
}

int SimpleActor::GetSaySlot
	(
	void
	)

{
	return m_AnimDialogHigh ? 12 : 11;
}

void SimpleActor::StartCrossBlendAnimSlot
	(
	int slot
	)

{
	// FIXME: stub
	STUB();
}

void SimpleActor::StartMotionAnimSlot
	(
	int slot,
	int anim,
	float weight
	)

{
	// FIXME: stub
	STUB();
}

void SimpleActor::StartAimMotionAnimSlot
	(
	int slot,
	int anim
	)

{
	// FIXME: stub
	STUB();
}

void SimpleActor::StartActionAnimSlot
	(
	int anim
	)

{
	// FIXME: stub
	STUB();
}

void SimpleActor::StartSayAnimSlot
	(
	int anim
	)

{
	// FIXME: stub
	STUB();
}

void SimpleActor::StartAimAnimSlot
	(
	int slot,
	int anim
	)

{
	// FIXME: stub
	STUB();
}

void SimpleActor::SetBlendedWeight
	(
	int slot
	)

{
	// FIXME: stub
	STUB();
}

void SimpleActor::EventSetAnimLength
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void SimpleActor::UpdateNormalAnimSlot
	(
	int slot
	)

{
	// FIXME: stub
	STUB();
}

void SimpleActor::UpdateCrossBlendAnimSlot
	(
	int slot
	)

{
	// FIXME: stub
	STUB();
}

void SimpleActor::UpdateCrossBlendDialogAnimSlot
	(
	int slot
	)

{
	// FIXME: stub
	STUB();
}

void SimpleActor::UpdateSayAnimSlot
	(
	int slot
	)

{
	// FIXME: stub
	STUB();
}

void SimpleActor::UpdateLastFrameSlot
	(
	int slot
	)

{
	// FIXME: stub
	STUB();
}

void SimpleActor::UpdateAnimSlot
	(
	int slot
	)

{
	// FIXME: stub
	STUB();
}

void SimpleActor::StopAllAnimating
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void SimpleActor::ChangeMotionAnim
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void SimpleActor::ChangeActionAnim
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void SimpleActor::ChangeSayAnim
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void SimpleActor::UpdateAim
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void SimpleActor::UpdateAimMotion
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void SimpleActor::EventAIOn
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void SimpleActor::EventAIOff
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void SimpleActor::EventGetWeaponGroup
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void SimpleActor::EventGetWeaponType
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void SimpleActor::EventGetPainHandler
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void SimpleActor::EventSetPainHandler
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void SimpleActor::EventGetDeathHandler
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void SimpleActor::EventSetDeathHandler
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void SimpleActor::EventGetAttackHandler
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void SimpleActor::EventSetAttackHandler
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void SimpleActor::EventGetSniperHandler
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void SimpleActor::EventSetSniperHandler
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void SimpleActor::EventSetCrossblendTime
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void SimpleActor::EventGetCrossblendTime
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void SimpleActor::EventSetEmotion
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void SimpleActor::EventGetPosition
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void SimpleActor::EventSetPosition
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void SimpleActor::EventGetAnimMode
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void SimpleActor::EventSetAnimMode
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void SimpleActor::EventSetAnimFinal
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void SimpleActor::DesiredAnimation
	(
	int eAnimMode,
	const_str csAnimString
	)

{
	// FIXME: stub
	STUB();
}

void SimpleActor::StartAnimation
	(
	int eAnimMode,
	const_str csAnimString
	)

{
	// FIXME: stub
	STUB();
}

void SimpleActor::DesiredAnimation
	(
	int eAnimMode,
	ScriptThreadLabel AnimLabel
	)

{
	// FIXME: stub
	STUB();
}

void SimpleActor::StartAnimation
	(
	int eAnimMode,
	ScriptThreadLabel AnimLabel
	)

{
	// FIXME: stub
	STUB();
}

void SimpleActor::ContinueAnimationAllowNoPath
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void SimpleActor::ContinueAnimation
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void SimpleActor::SetPathGoalEndAnim
	(
	const_str csEndAnim
	)

{
	// FIXME: stub
	STUB();
}

bool SimpleActor::UpdateSelectedAnimation
	(
	void
	)

{
	// FIXME: stub
	STUB();
	return false;
}

const char *SimpleActor::DumpCallTrace
	(
	const char *pszFmt,
	...
	) const

{
	return "overloaded version should always get called";
}
