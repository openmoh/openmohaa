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

// simpleactor.h: Base class for character AI.

#include "weapon.h"
#include "sentient.h"
#include "container.h"
#include "stack.h"
#include "navigate.h"
#include "scriptmaster.h"
#include "characterstate.h"
#include "actorpath.h"

class SimpleActor;

typedef SafePtr<SimpleActor> SimpleActorPtr;

class SimpleActor : public Sentient
{
public:
	int m_eAnimMode;
	ScriptThreadLabel m_Anim;
	SafePtr<ScriptThread> m_pAnimThread;
	int m_eNextAnimMode;
	const_str m_csNextAnimString;
	ScriptThreadLabel m_NextAnimLabel;
	bool m_bNextForceStart;
	float m_fCrouchWeight;
	bool m_YawAchieved;
	float m_DesiredYaw;
	bool m_bHasDesiredLookDest;
	bool m_bHasDesiredLookAngles;
	Vector m_vDesiredLookDest;
	Vector m_DesiredLookAngles;
	Vector m_DesiredGunDir;
	ActorPath m_Path;
	float m_Dest[ 3 ];
	float m_NoClipDest[ 3 ];
	float path_failed_time;
	float m_fPathGoalTime;
	bool m_bStartPathGoalEndAnim;
	const_str m_csPathGoalEndAnimScript;
	qboolean m_walking;
	qboolean m_groundPlane;
	Vector m_groundPlaneNormal;
	Vector watch_offset;
	bool m_bThink;
	int m_PainTime;
	int m_eEmotionMode;
	float m_fAimLimit_up;
	float m_fAimLimit_down;
	int m_ChangeMotionAnimIndex;
	int m_ChangeActionAnimIndex;
	int m_ChangeSayAnimIndex;

protected:
	unsigned int m_weightType[ MAX_FRAMEINFOS ];
	float m_weightBase[ MAX_FRAMEINFOS ];
	float m_weightCrossBlend[ MAX_FRAMEINFOS ];
	bool m_AnimMotionHigh;
	bool m_AnimActionHigh;
	bool m_AnimDialogHigh;

public:
	int hit_obstacle_time;
	float obstacle_vel[ 2 ];
	const_str m_csAnimName;
	const_str m_csSayAnim;
	const_str m_csUpperAnim;
	const_str m_csCurrentPosition;
	int m_bPathErrorTime;
	class PathNode *m_NearestNode;
	Vector m_vNearestNodePos;
	short int m_bUpdateAnimDoneFlags;
	float m_maxspeed;
	const_str m_csMood;
	const_str m_csIdleMood;
	int m_iMotionSlot;
	int m_iActionSlot;
	int m_iSaySlot;
	bool m_bLevelMotionAnim;
	bool m_bLevelActionAnim;
	byte m_bLevelSayAnim;
	byte m_bNextLevelSayAnim;
	bool m_bMotionAnimSet;
	bool m_bActionAnimSet;
	bool m_bSayAnimSet;
	bool m_bAimAnimSet;
	int m_iVoiceTime;
	bool m_bDoAI;
	ScriptThreadLabel m_PainHandler;
	ScriptThreadLabel m_DeathHandler;
	ScriptThreadLabel m_AttackHandler;
	ScriptThreadLabel m_SniperHandler;
	float m_fCrossblendTime;

public:
	CLASS_PROTOTYPE( SimpleActor );

	SimpleActor();

	virtual void Archive( Archiver & arc );
	virtual void SetMoveInfo( mmove_t *mm );
	virtual void GetMoveInfo( mmove_t *mm );
	bool CanSeeFrom( vec3_t pos, Entity *ent );
	virtual bool CanTarget( void );
	virtual bool IsImmortal( void );
	bool DoesTheoreticPathExist( Vector vDestPos, float fMaxPath );
	void SetPath( Vector vDestPos, char *description, int iMaxDirtyTime, float *vLeashHome, float fLeashDistSquared );
	void SetPath( SimpleEntity *pDestNode, char *description, int iMaxDirtyTime );
	void SetPathWithinDistance( Vector vDestPos, char *description, float fMaxPath, int iMaxDirtyTime );
	void FindPathAway( vec3_t vAwayFrom, vec3_t vDirPreferred, float fMinSafeDist );
	void ClearPath( void );
	bool PathComplete( void ) const;
	bool PathExists( void ) const;
	bool PathIsValid( void ) const;
	bool PathAvoidsSquadMates( void ) const;
	void ShortenPathToAvoidSquadMates( void );
	PathInfo *CurrentPathNode( void ) const;
	PathInfo *LastPathNode( void ) const;
	float PathDist( void ) const;
	bool PathHasCompleteLookahead( void ) const;
	Vector PathGoal( void ) const;
	float *PathDelta( void ) const;
	bool PathGoalSlowdownStarted( void ) const;
	void SetDest( vec3_t dest );
	void StopTurning( void );
	void SetDesiredYaw( float yaw );
	void SetDesiredYawDir( vec3_t vec );
	void SetDesiredYawDest( vec3_t vec );
	void UpdateEmotion( void );
	int GetEmotionAnim( void );
	int GetMotionSlot( int slot );
	int GetActionSlot( int slot );
	int GetSaySlot( void );
	void StartCrossBlendAnimSlot( int slot );
	void StartMotionAnimSlot( int slot, int anim, float weight );
	void StartAimMotionAnimSlot( int slot, int anim );
	void StartActionAnimSlot( int anim );
	void StartSayAnimSlot( int anim );
	void StartAimAnimSlot( int slot, int anim );
	void SetBlendedWeight( int slot = 0 );
	void EventSetAnimLength( Event *ev );
	void UpdateNormalAnimSlot( int slot = 0 );
	void UpdateCrossBlendAnimSlot( int slot = 0 );
	void UpdateCrossBlendDialogAnimSlot( int slot = 0 );
	void UpdateSayAnimSlot( int slot = 0 );
	void UpdateLastFrameSlot( int slot = 0 );
	void UpdateAnimSlot( int slot = 0 );
	void StopAllAnimating( void );
	void ChangeMotionAnim( void );
	void ChangeActionAnim( void );
	void ChangeSayAnim( void );
	void StopAnimating( int slot = 0 );
	void AnimFinished( int slot = 0 );
	void UpdateAim( void );
	void UpdateAimMotion( void );
	void EventAIOn( Event *ev );
	void EventAIOff( Event *ev );
	void EventGetWeaponGroup( Event *ev );
	void EventGetWeaponType( Event *ev );
	void EventGetPainHandler( Event *ev );
	void EventSetPainHandler( Event *ev );
	void EventGetDeathHandler( Event *ev );
	void EventSetDeathHandler( Event *ev );
	void EventGetAttackHandler( Event *ev );
	void EventSetAttackHandler( Event *ev );
	void EventGetSniperHandler( Event *ev );
	void EventSetSniperHandler( Event *ev );
	void EventSetCrossblendTime( Event *ev );
	void EventGetCrossblendTime( Event *ev );
	void EventSetEmotion( Event *ev );
	void EventGetPosition( Event *ev );
	void EventSetPosition( Event *ev );
	void EventGetAnimMode( Event *ev );
	void EventSetAnimMode( Event *ev );
	void EventSetAnimFinal( Event *ev );
	const_str GetRunAnim( void );
	const_str GetWalkAnim( void );
	void DesiredAnimation( int eAnimMode, const_str csAnimString );
	void StartAnimation( int eAnimMode, const_str csAnimString );
	void DesiredAnimation( int eAnimMode, ScriptThreadLabel AnimLabel );
	void StartAnimation( int eAnimMode, ScriptThreadLabel AnimLabel );
	void ContinueAnimationAllowNoPath( void );
	void ContinueAnimation( void );
	void SetPathGoalEndAnim( const_str csEndAnim );
	bool UpdateSelectedAnimation( void );
	void Anim_Attack( void );
	void Anim_Sniper( void );
	void Anim_Aim( void );
	void Anim_Shoot( void );
	void Anim_Idle( void );
	void Anim_Crouch( void );
	void Anim_Prone( void );
	void Anim_Stand( void );
	void Anim_Cower( void );
	void Anim_Killed( void );
	void Anim_StartPain( void );
	void Anim_Pain( void );
	void Anim_CrouchRunTo( int eAnimMode );
	void Anim_CrouchWalkTo( int eAnimMode );
	void Anim_StandRunTo( int eAnimMode );
	void Anim_StandWalkTo( int eAnimMode );
	void Anim_RunTo( int eAnimMode );
	void Anim_WalkTo( int eAnimMode );
	void Anim_RunAwayFiring( int eAnimMode );
	void Anim_RunToShooting( int eAnimMode );
	void Anim_RunToAlarm( int eAnimMode );
	void Anim_RunToCasual( int eAnimMode );
	void Anim_RunToCover( int eAnimMode );
	void Anim_RunToDanger( int eAnimMode );
	void Anim_RunToDive( int eAnimMode );
	void Anim_RunToFlee( int eAnimMode );
	void Anim_RunToInOpen( int eAnimMode );
	void Anim_Emotion( int eEmotionMode );
	void Anim_Say( const_str csSayAnimScript, int iMinTimeSinceLastSay, bool bCanInterrupt );
	void Anim_FullBody( const_str csFullBodyAnim, int eAnimMode );
	virtual char *DumpCallTrace( const char *pszFmt, ... ) const;
};
