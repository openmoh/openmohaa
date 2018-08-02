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

// actor.h: Base class for character AI.

#ifndef __ACTOR_H__
#define __ACTOR_H__

#include "g_local.h"
#include "simpleactor.h"
#include "actorenemy.h"
#include <gamescript.h>
#include <scriptmaster.h>
#include "grenadehint.h"
#include <parm.h>

extern Event EV_Actor_Start;
extern Event EV_Actor_Dead;
extern Event EV_Actor_LookAt;
extern Event EV_Actor_TurnTo;
extern Event EV_Actor_FinishedBehavior;
extern Event EV_Actor_NotifyBehavior;
extern Event EV_Actor_FinishedMove;
extern Event EV_Actor_FinishedAnim;
extern Event EV_Actor_WalkTo;
extern Event EV_Actor_RunTo;
extern Event EV_Actor_Anim;
extern Event EV_Actor_AttackFinished;
extern Event EV_Actor_Attack;
extern Event EV_Actor_AttackPlayer;
extern Event EV_Actor_AIOn;
extern Event EV_Actor_AIOff;
extern Event EV_Actor_AIDeaf;
extern Event EV_Actor_AIDumb;
extern Event EV_ActorIncomingProjectile;
extern Event EV_Anim_Done;
extern Event EV_ActorOnlyShootable;
extern Event EV_Actor_BounceOff;
extern Event EV_Actor_Push;
extern Event EV_Actor_Statemap;
extern Event EV_Actor_SetTargetable;
extern Event EV_Actor_DeathEmbalm;

// Bones used by actor

#define ACTOR_MOUTH_TAG  0
#define ACTOR_HEAD_TAG   1
#define ACTOR_TORSO_TAG  2

// Dialog stuff

#define LIP_SYNC_HZ  20.0

#define MAX_DIALOG_PARAMETERS_LENGTH  100

#define MAX_DIALOG_PARM_LENGTH  64
#define MAX_DIALOG_PARMS  10

#define DIALOG_PARM_TYPE_NONE          0
#define DIALOG_PARM_TYPE_PLAYERHAS     1
#define DIALOG_PARM_TYPE_PLAYERHASNOT  2
#define DIALOG_PARM_TYPE_HAS           3
#define DIALOG_PARM_TYPE_HASNOT        4
#define DIALOG_PARM_TYPE_DEPENDS       5
#define DIALOG_PARM_TYPE_DEPENDSNOT    6  

typedef struct
{
	byte type;
	char parm[ MAX_DIALOG_PARM_LENGTH ];
} DialogParm_t;

typedef struct DialogNode_s
   {
	char alias_name[ MAX_ALIAS_NAME_LENGTH ];
	int random_flag;
	int number_of_parms;
	float random_percent;
	DialogParm_t parms[ MAX_DIALOG_PARMS ];
   struct DialogNode_s *next;
   } DialogNode_t;

typedef enum
	{
   IS_INANIMATE,
	IS_MONSTER,
	IS_ENEMY,
	IS_CIVILIAN,
	IS_FRIEND,
   IS_ANIMAL,
	NUM_ACTORTYPES
	} actortype_t;

// Stimuli types

#define STIMULI_ALL    -1
#define STIMULI_NONE    0
#define STIMULI_SIGHT   (1<<0)
#define STIMULI_SOUND   (1<<1)
#define STIMULI_PAIN    (1<<2)
#define STIMULI_SCRIPT  (1<<3)

#define MAX_INACTIVE_TIME 30.0

// State flags

#define STATE_FLAG_IN_PAIN			(1<<0)
#define STATE_FLAG_MELEE_HIT		(1<<1)
#define STATE_FLAG_TOUCHED			(1<<2)
#define STATE_FLAG_ACTIVATED		(1<<3)
#define STATE_FLAG_USED				(1<<4)
#define STATE_FLAG_TWITCH			(1<<5)
#define STATE_FLAG_BLOCKED_HIT		(1<<6)
#define STATE_FLAG_SMALL_PAIN		(1<<7)
#define STATE_FLAG_OTHER_DIED		(1<<8)
#define STATE_FLAG_STUCK			(1<<9)
#define STATE_FLAG_NO_PATH			(1<<10)

// Actor modes

#define ACTOR_MODE_NONE			0
#define ACTOR_MODE_IDLE			1
#define ACTOR_MODE_AI			2
#define ACTOR_MODE_SCRIPT		3
#define ACTOR_MODE_TALK			4

// Pain types

#define PAIN_SMALL  0
#define PAIN_BIG    1

typedef struct
{
	EntityPtr ent;	
	unsigned int state_flags;
} part_t;

// Actor flags

#define ACTOR_FLAG_NOISE_HEARD					0
#define ACTOR_FLAG_INVESTIGATING					1
#define ACTOR_FLAG_DEATHGIB						2
#define ACTOR_FLAG_DEATHFADE						3
#define ACTOR_FLAG_NOCHATTER						4
#define ACTOR_FLAG_INACTIVE						5
#define ACTOR_FLAG_ANIM_DONE						6
#define ACTOR_FLAG_STATE_DONE_TIME_VALID		7
#define ACTOR_FLAG_AI_ON							8
#define ACTOR_FLAG_LAST_CANSEEENEMY				9
#define ACTOR_FLAG_LAST_CANSEEENEMY_NOFOV		10
#define ACTOR_FLAG_DIALOG_PLAYING				11
#define ACTOR_FLAG_ALLOW_TALK						12
#define ACTOR_FLAG_DAMAGE_ONCE_ON				13
#define ACTOR_FLAG_DAMAGE_ONCE_DAMAGED			14
#define ACTOR_FLAG_BOUNCE_OFF						15
#define ACTOR_FLAG_NOTIFY_OTHERS_AT_DEATH		16
#define ACTOR_FLAG_HAS_THING1						17
#define ACTOR_FLAG_HAS_THING2						18
#define ACTOR_FLAG_HAS_THING3						19
#define ACTOR_FLAG_HAS_THING4						20
#define ACTOR_FLAG_LAST_ATTACK_HIT				21
#define ACTOR_FLAG_STARTED							22
#define ACTOR_FLAG_ALLOW_HANGBACK				23
#define ACTOR_FLAG_USE_GRAVITY					24
#define ACTOR_FLAG_SPAWN_FAILED					25
#define ACTOR_FLAG_FADING_OUT						26
#define ACTOR_FLAG_DEATHSHRINK					27
#define ACTOR_FLAG_DEATHSINK					   28
#define ACTOR_FLAG_STAYSOLID					   29
#define ACTOR_FLAG_STUNNED						   30
#define ACTOR_FLAG_ALLOW_FALL					   31
#define ACTOR_FLAG_FINISHED					   32
#define ACTOR_FLAG_IN_LIMBO					   33
#define ACTOR_FLAG_CAN_WALK_ON_OTHERS		   34
#define ACTOR_FLAG_PUSHABLE					   35
#define ACTOR_FLAG_LAST_TRY_TALK				   36
#define ACTOR_FLAG_ATTACKABLE_BY_ACTORS	   37
#define ACTOR_FLAG_TARGETABLE					   38
#define ACTOR_FLAG_ATTACK_ACTORS				   39
#define ACTOR_FLAG_IMMORTAL						40
#define ACTOR_FLAG_TURNING_HEAD					41
#define ACTOR_FLAG_DIE_COMPLETELY				42
#define ACTOR_FLAG_BLEED_AFTER_DEATH			43
#define ACTOR_FLAG_IGNORE_STUCK_WARNING		44
#define ACTOR_FLAG_IGNORE_OFF_GROUND_WARNING	45
#define ACTOR_FLAG_ALLOWED_TO_KILL				46
#define ACTOR_FLAG_TOUCH_TRIGGERS				47
#define ACTOR_FLAG_IGNORE_WATER					48
#define ACTOR_FLAG_NEVER_IGNORE_SOUNDS			49
#define ACTOR_FLAG_SIMPLE_PATHFINDING			50
#define ACTOR_FLAG_HAVE_MOVED						51
#define ACTOR_FLAG_NO_PAIN_SOUNDS				52
#define ACTOR_FLAG_UPDATE_BOSS_HEALTH			53
#define ACTOR_FLAG_IGNORE_PAIN_FROM_ACTORS	54
#define ACTOR_FLAG_DAMAGE_ALLOWED				55
#define ACTOR_FLAG_ALWAYS_GIVE_WATER			56


// The last actor_flag number and this one (ACTOR_FLAG_MAX) should match

#define ACTOR_FLAG_MAX								56

#define MAX_ORIGIN_HISTORY	4
#define MAX_THINKMAP		9
#define MAX_THINKSTATES		4
#define MAX_COVER_NODES		16
#define MAX_BODYQUEUE		5
#define MAX_GLOBAL_FUNCS	35

typedef enum
{
	AI_GREN_TOSS_NONE,
	AI_GREN_TOSS_THROW,
	AI_GREN_TOSS_ROLL,
	AI_GREN_TOSS_HINT,
	AI_GREN_KICK
} eGrenadeTossMode;

typedef struct {
	byte length;
	byte currentPos;
	byte loop;
	float startTime;
	vec3_t pos[ 1 ];
} FallPath;

#define THINKSTATE_NONE				0
#define THINKSTATE_IDLE				1
#define THINKSTATE_PAIN				2
#define THINKSTATE_KILLED			3
#define THINKSTATE_ATTACK			4
#define THINKSTATE_CURIOUS			5
#define THINKSTATE_DISGUISE			6
#define THINKSTATE_GRENADE			7

#define AI_EVENT_NONE				0
#define AI_EVENT_WEAPON_FIRE		1
#define AI_EVENT_WEAPON_IMPACT		2
#define AI_EVENT_EXPLOSION			3
#define AI_EVENT_AMERICAN_VOICE		4
#define AI_EVENT_GERMAN_VOICE		5
#define AI_EVENT_AMERICAN_URGENT	6
#define AI_EVENT_GERMAN_URGENT		7
#define AI_EVENT_MISC				8
#define AI_EVENT_MISC_LOUD			9
#define AI_EVENT_FOOTSTEP			10
#define AI_EVENT_GRENADE			11

class Actor;
typedef SafePtr<Actor> ActorPtr;

class Actor : public SimpleActor //#19
{
	struct GlobalFuncs_t {
		void ( Actor::*ThinkState )( void );
		void ( Actor::*BeginState )( void );
		void ( Actor::*ResumeState )( void );
		void ( Actor::*EndState )( void );
		void ( Actor::*SuspendState )( void );
		void ( Actor::*un3 )( void );
		void ( Actor::*FinishedAnimation )( void );
		void ( Actor::*PostShoot )( void );
		void ( Actor::*Pain )( Event *ev );
		void ( Actor::*Killed )( Event *ev, bool bPlayDeathAnim );
		bool ( Actor::*PassesTransitionConditions )( void );
		void ( Actor::*ShowInfo )( void );
		void ( Actor::*PathnodeClaimRevoked )( void );
		void ( Actor::*ReceiveAIEvent )( vec3_t event_origin, int iType, Entity *originator, float fDistSquared, float fMaxDistSquared );
		bool ( *IsState )( int state );
	};

public:
	static GlobalFuncs_t GlobalFuncs[ MAX_GLOBAL_FUNCS ];
	static const_str m_csThinkNames[ MAX_GLOBAL_FUNCS ];
	static const_str m_csThinkStateNames[ MAX_THINKMAP ];
	int m_ThinkMap[ MAX_THINKMAP ];
	int m_ThinkStates[ MAX_THINKSTATES ];
	int m_Think[ MAX_THINKSTATES ];
	int m_ThinkLevel;
	int m_ThinkState;
	int m_State;
	int m_iStateTime;
	bool m_bLockThinkState;
	bool m_bDirtyThinkState;
	char *m_pszDebugState;
	bool m_bAnimating;
	bool m_bDog;
	int mVoiceType;
	bool m_bSilent;
	bool m_bNoSurprise;
	bool m_bMumble;
	bool m_bBreathSteam;
	const_str m_csHeadModel;
	const_str m_csHeadSkin;
	const_str m_csWeapon;
	const_str m_csLoadOut;
	SentientPtr m_FavoriteEnemy;
	int m_iEnemyCheckTime;
	int m_iEnemyChangeTime;
	int m_iEnemyVisibleCheckTime;
	int m_iEnemyVisibleChangeTime;
	int m_iLastEnemyVisibleTime;
	int m_iEnemyFovCheckTime;
	int m_iEnemyFovChangeTime;
	Vector m_vLastEnemyPos;
	int m_iLastEnemyPosChangeTime;
	float m_fMaxShareDistSquared;
	int m_iCanShootCheckTime;
	bool m_bCanShootEnemy;
	bool m_bDesiredEnableEnemy;
	bool m_bEnableEnemy;
	bool m_bEnablePain;
	bool m_bNoLongPain;
	bool m_bNewEnemy;
	bool m_bEnemyIsDisguised;
	bool m_bEnemyVisible;
	bool m_bEnemyInFOV;
	bool m_bForceAttackPlayer;
	bool m_bAutoAvoidPlayer;
	bool m_bNoIdleAfterAnim;
	bool m_bAnimScriptSet;
	const_str m_csAnimScript;
	int m_AnimMode;
	float m_fDfwRequestedYaw;
	float m_fDfwDerivedYaw;
	Vector m_vDfwPos;
	float m_fDfwTime;
	int m_iGunPositionCheckTime;
	Vector m_vGunPosition;
	int m_iWallDodgeTimeout;
	vec2_t m_PrevObstacleNormal;
	char m_WallDir;
	float m_fMoveDoneRadiusSquared;
	int m_iOriginTime;
	bool m_bFaceEnemy;
	bool m_bDoPhysics;
	bool m_bBecomeRunner;
	bool m_bPatrolWaitTrigger;
	bool m_bScriptGoalValid;
	Vector m_vScriptGoal;
	int m_iNextWatchStepTime;
	SafePtr<SimpleEntity> m_patrolCurrentNode;
	const_str m_csPatrolCurrentAnim;
	int m_iSquadStandTime;
	float m_fInterval;
	int m_iIntervalDirTime;
	Vector m_vIntervalDir;
	short m_sCurrentPathNodeIndex;
	int m_PainState;
	int m_iCuriousTime;
	int m_iCuriousLevel;
	int m_iCuriousAnimHint;
	int m_iNextDisguiseTime;
	int m_iDisguisePeriod;
	float m_fMaxDisguiseDistSquared;
	int m_iEnemyShowPapersTime;
	ScriptThreadLabel m_DisguiseAcceptThread;
	int m_iDisguiseLevel;
	SafePtr< SimpleEntity > m_AlarmNode;
	ScriptThreadLabel m_AlarmThread;
	int m_iRunHomeTime;
	bool m_bTurretNoInitialCover;
	PathNode *m_pPotentialCoverNode[ MAX_COVER_NODES ];
	int m_iPotentialCoverCount;
	PathNode *m_pCoverNode;
	const_str m_csSpecialAttack;
	bool m_bInReload;
	bool m_bNeedReload;
	bool mbBreakSpecialAttack;
	bool m_bGrenadeBounced;
	SafePtr<Entity> m_pGrenade;
	Vector m_vGrenadePos;
	int m_iFirstGrenadeTime;
	int m_eGrenadeState;
	eGrenadeTossMode m_eGrenadeMode;
	Vector m_vGrenadeVel;
	Vector m_vKickDir;
	FallPath *m_pFallPath;
	float m_fBalconyHeight;
	bool m_bNoPlayerCollision;
	float m_fNoticeTimeScale;
	float m_fMaxNoticeTimeScale;
	ActorEnemySet m_PotentialEnemies;
	float m_fSight;
	float m_fHearing;
	float m_fSoundAwareness;
	float m_fGrenadeAwareness;
	int m_iIgnoreSoundsMask;
	float m_fFov;
	float m_fFovDot;
	int m_iEyeUpdateTime;
	Vector m_vEyeDir;
	int m_iNextLookTime;
	float m_fLookAroundFov;
	SafePtr<SimpleEntity> m_pLookEntity;
	int m_iLookFlags;
	SafePtr<SimpleEntity> m_pPointEntity;
	SafePtr<SimpleEntity> m_pTurnEntity;
	float m_fTurnDoneError;
	float m_fAngleYawSpeed;
	SafePtr<SimpleEntity> m_aimNode;
	int m_eDontFaceWallMode;
	int m_iLastFaceDecideTime;
	vec2_t m_vOriginHistory[ MAX_ORIGIN_HISTORY ];
	int m_iCurrentHistory;
	bool m_bHeadAnglesAchieved;
	bool m_bLUpperArmAnglesAchieved;
	bool m_bTorsoAnglesAchieved;
	bool align3;
	float m_fHeadMaxTurnSpeed;
	vec3_t m_vHeadDesiredAngles;
	float m_fLUpperArmTurnSpeed;
	vec3_t m_vLUpperArmDesiredAngles;
	float m_fTorsoMaxTurnSpeed;
	float m_fTorsoCurrentTurnSpeed;
	vec3_t m_vTorsoDesiredAngles;
	static SafePtr< Actor > mBodyQueue[ MAX_BODYQUEUE ];
	static int mCurBody;
	Vector m_vHome;
	SafePtr<SimpleEntity> m_pTetherEnt;
	float m_fMinDistance;
	float m_fMinDistanceSquared;
	float m_fMaxDistance;
	float m_fMaxDistanceSquared;
	float m_fLeash;
	float m_fLeashSquared;
	bool m_bFixedLeash;

public:
	CLASS_PROTOTYPE( Actor );

protected:
	void MoveTo( Event *ev );
	void WalkTo( Event *ev );
	void RunTo( Event *ev );
	void CrouchTo( Event *ev );
	void CrawlTo( Event *ev );
	void AimAt( Event *ev );
	void DefaultRestart( void );
	void SuspendState( void );
	void ResumeState( void );
	void BeginState( void );
	void EndState( int level );
	void RestartState( void );

public:
	Actor();
	
	virtual void setContentsSolid( void );
	void InitThinkStates( void );
	void UpdateEyeOrigin( void );
	bool RequireThink( void );
	void UpdateEnemy( int iMaxDirtyTime );
	void UpdateEnemyInternal( void );
	void SetEnemy( Sentient *pEnemy, bool bForceConfirmed );
	void SetEnemyPos( Vector vPos );
	static void ResetBodyQueue( void );
	void AddToBodyQue( void );
	Vector GetAntiBunchPoint( void );
	static void InitVoid( GlobalFuncs_t *func );
	virtual char *DumpCallTrace( const char *pszFmt, ... ) const;
	static void Init( void );
	void FixAIParameters( void );
	bool AttackEntryAnimation( void );
	void CheckForThinkStateTransition( void );
	bool CheckForTransition( int state, int level );
	bool PassesTransitionConditions_Grenade( void );
	bool PassesTransitionConditions_Attack( void );
	bool PassesTransitionConditions_Disguise( void );
	bool PassesTransitionConditions_Curious( void );
	bool PassesTransitionConditions_Idle( void );
	void UpdateEnableEnemy( void );
	void ThinkStateTransitions( void );
	void TransitionState( int iNewState, int iPadTime );
	void ChangeAnim( void );
	void UpdateSayAnim( void );
	void UpdateUpperAnim( void );
	void UpdateAnim( void );
	virtual void StoppedWaitFor( const_str name, bool bDeleting );
	static void InitTurret( GlobalFuncs_t *func );
	void Begin_Turret( void );
	void End_Turret( void );
	void Suspend_Turret( void );
	void Think_Turret( void );
	void FinishedAnimation_Turret( void );
	void ReceiveAIEvent_Turret( vec3_t event_origin, int iType, Entity *originator, float fDistSquared, float fMaxDistSquared );
	void InterruptPoint_Turret( void );
	void PathnodeClaimRevoked_Turret( void );
	bool Turret_IsRetargeting( void ) const;
	bool Turret_DecideToSelectState( void );
	void Turret_SelectState( void );
	bool Turret_CheckRetarget( void );
	bool Turret_TryToBecomeCoverGuy( void );
	void Turret_BeginRetarget( void );
	void Turret_NextRetarget( void );
	void Turret_SideStep( int iStepSize, vec3_t vDir );
	void State_Turret_Combat( void );
	void State_Turret_Reacquire( void );
	void State_Turret_TakeSniperNode( void );
	void State_Turret_SniperNode( void );
	bool State_Turret_RunHome( bool bAttackOnFail );
	void State_Turret_RunAway( void );
	void State_Turret_Charge( void );
	void State_Turret_Grenade( void );
	void State_Turret_FakeEnemy( void );
	void State_Turret_Wait( void );
	void State_Turret_Retarget_Sniper_Node( void );
	void State_Turret_Retarget_Step_Side_Small( void );
	void State_Turret_Retarget_Path_Exact( void );
	void State_Turret_Retarget_Path_Near( void );
	void State_Turret_Retarget_Step_Side_Medium( void );
	void State_Turret_Retarget_Step_Side_Large( void );
	void State_Turret_Retarget_Step_Face_Medium( void );
	void State_Turret_Retarget_Step_Face_Large( void );
	static void InitCover( GlobalFuncs_t *func );
	bool Cover_IsValid( PathNode *node );
	bool Cover_SetPath( PathNode *node );
	void Cover_FindCover( bool bCheckAll );
	void Begin_Cover( void );
	void End_Cover( void );
	void Suspend_Cover( void );
	void Think_Cover( void );
	void FinishedAnimation_Cover( void );
	void PathnodeClaimRevoked_Cover( void );
	void State_Cover_NewEnemy( void );
	void State_Cover_FindCover( void );
	void State_Cover_TakeCover( void );
	void State_Cover_FinishReloading( void );
	void State_Cover_SpecialAttack( void );
	void State_Cover_Target( void );
	void State_Cover_Hide( void );
	void State_Cover_Shoot( void );
	void State_Cover_Grenade( void );
	void State_Cover_FindEnemy( void );
	void State_Cover_SearchNode( void );
	void State_Cover_HuntEnemy( void );
	void State_Cover_FakeEnemy( void );
	static void InitPatrol( GlobalFuncs_t *func );
	void Begin_Patrol( void );
	void End_Patrol( void );
	void Resume_Patrol( void );
	void Think_Patrol( void );
	void ShowInfo_Patrol( void );
	void IdleThink( void );
	static void InitRunner( GlobalFuncs_t *func );
	void Begin_Runner( void );
	void End_Runner( void );
	void Resume_Runner( void );
	void Think_Runner( void );
	void ShowInfo_Runner( void );
	static void InitAlarm( GlobalFuncs_t *func );
	void Begin_Alarm( void );
	void End_Alarm( void );
	void State_Alarm_StartThread( void );
	void State_Alarm_Move( void );
	void State_Alarm_Idle( void );
	void Think_Alarm( void );
	void FinishedAnimation_Alarm( void );
	static void InitNoClip( GlobalFuncs_t *func );
	bool IsNoClipState( int state );
	void Think_NoClip( void );
	bool ValidGrenadePath( Vector& vFrom, Vector& vTo, Vector& vVel ) const;
	static Vector CalcThrowVelocity( Vector& vFrom, Vector& vTo );
	Vector CanThrowGrenade( Vector& vFrom, Vector& vTo ) const;
	static Vector CalcRollVelocity( Vector& vFrom, Vector& vTo );
	Vector CanRollGrenade( Vector& vFrom, Vector& vTo ) const;
	bool CanTossGrenadeThroughHint( GrenadeHint *pHint, Vector& vFrom, Vector& vTo, bool bDesperate, Vector *pvVel, eGrenadeTossMode *peMode ) const;
	static Vector GrenadeThrowPoint( Vector& vFrom, Vector& vDelta, const_str csAnim );
	Vector CalcKickVelocity( Vector& vDelta, float fDist ) const;
	bool CanKickGrenade( Vector &vFrom, Vector &vTo, Vector &vFace, Vector *pvVel ) const;
	bool GrenadeWillHurtTeamAt( Vector& vTo ) const;
	bool CanGetGrenadeFromAToB( Vector& vFrom, Vector& vTo, bool bDesperate, Vector *pvVel, eGrenadeTossMode *peMode ) const;
	bool DecideToThrowGrenade( Vector& vTo, Vector *pvVel, eGrenadeTossMode *peMode ) const;
	void Grenade_EventFire( Event *ev );
	void GenericGrenadeTossThink( void );
	static void InitGrenade( GlobalFuncs_t *func );
	bool Grenade_Acquire( int eNextState, const_str csReturnAnim );
	void Grenade_Flee( void );
	void Grenade_ThrowAcquire( void );
	void Grenade_Throw( void );
	void Grenade_KickAcquire( void );
	void Grenade_Kick( void );
	void Grenade_MartyrAcquire( void );
	void Grenade_Martyr( void );
	void Grenade_Wait( void );
	void Grenade_NextThinkState( void );
	void Grenade_EventAttach( Event *ev );
	void Grenade_EventDetach( Event *ev );
	void Begin_Grenade( void );
	void End_Grenade( void );
	void Resume_Grenade( void );
	void Think_Grenade( void );
	void FinishedAnimation_Grenade( void );
	static void InitCurious( GlobalFuncs_t *func );
	void SetCuriousAnimHint( int iAnimHint );
	void Begin_Curious( void );
	void End_Curious( void );
	void Resume_Curious( void );
	void Suspend_Curious( void );
	void Think_Curious( void );
	void FinishedAnimation_Curious( void );
	void LookAtCuriosity( void );
	void TimeOutCurious( void );
	void State_Disguise_Wait( void );
	void State_Disguise_Papers( void );
	void State_Disguise_Fake_Papers( void );
	void State_Disguise_Enemy( void );
	void State_Disguise_Halt( void );
	void State_Disguise_Accept( void );
	void State_Disguise_Deny( void );
	static void InitDisguiseSalute( GlobalFuncs_t *func );
	void Begin_DisguiseSalute( void );
	void End_DisguiseSalute( void );
	void Resume_DisguiseSalute( void );
	void Suspend_DisguiseSalute( void );
	void Think_DisguiseSalute( void );
	void FinishedAnimation_DisguiseSalute( void );
	static void InitDisguiseSentry( GlobalFuncs_t *func );
	void Begin_DisguiseSentry( void );
	void End_DisguiseSentry( void );
	void Resume_DisguiseSentry( void );
	void Suspend_DisguiseSentry( void );
	void Think_DisguiseSentry( void );
	static void InitDisguiseOfficer( GlobalFuncs_t *func );
	void Begin_DisguiseOfficer( void );
	void End_DisguiseOfficer( void );
	void Resume_DisguiseOfficer( void );
	void Suspend_DisguiseOfficer( void );
	void Think_DisguiseOfficer( void );
	static void InitDisguiseRover( GlobalFuncs_t *func );
	void Begin_DisguiseRover( void );
	void End_DisguiseRover( void );
	void Resume_DisguiseRover( void );
	void Suspend_DisguiseRover( void );
	void Think_DisguiseRover( void );
	static void InitDisguiseNone( GlobalFuncs_t *func );
	static void InitIdle( GlobalFuncs_t *func );
	void Begin_Idle( void );
	void Think_Idle( void );
	static void InitMachineGunner( GlobalFuncs_t *func );
	void Begin_MachineGunner( void );
	void End_MachineGunner( void );
	void BecomeTurretGuy( void );
	void Think_MachineGunner( void );
	void ThinkHoldGun( void );
	void FinishedAnimation_MachineGunner( void );
	bool MachineGunner_CanSee( Entity *ent, float fov, float vision_distance );
	void CanSee( Event *ev );
	static void InitDogIdle( GlobalFuncs_t *func );
	static void InitDogAttack( GlobalFuncs_t *func );
	static void InitDogCurious( GlobalFuncs_t *func );
	void Begin_Dog( void );
	void End_Dog( void );
	void Think_Dog_Idle( void );
	void Think_Dog_Attack( void );
	void Think_Dog_Curious( void );
	static void InitAnim( GlobalFuncs_t *func );
	void Begin_Anim( void );
	void Think_Anim( void );
	void FinishedAnimation_Anim( void );
	void ShowInfo_Anim( void );
	static void InitAnimCurious( GlobalFuncs_t *func );
	void Begin_AnimCurious( void );
	void Think_AnimCurious( void );
	void FinishedAnimation_AnimCurious( void );
	static void InitAim( GlobalFuncs_t *func );
	void Begin_Aim( void );
	void Think_Aim( void );
	void ShowInfo_Aim( void );
	static void InitBalconyIdle( GlobalFuncs_t *func );
	static void InitBalconyCurious( GlobalFuncs_t *func );
	static void InitBalconyAttack( GlobalFuncs_t *func );
	static void InitBalconyDisguise( GlobalFuncs_t *func );
	static void InitBalconyGrenade( GlobalFuncs_t *func );
	static void InitBalconyPain( GlobalFuncs_t *func );
	static void InitBalconyKilled( GlobalFuncs_t *func );
	void Pain_Balcony( Event *ev );
	void Killed_Balcony( Event *ev, bool bPlayDeathAnim );
	void Think_BalconyAttack( void );
	void Begin_BalconyAttack( void );
	void FinishedAnimation_BalconyAttack( void );
	void State_Balcony_PostShoot( void );
	void State_Balcony_FindEnemy( void );
	void State_Balcony_Target( void );
	void State_Balcony_Shoot( void );
	void Begin_BalconyKilled( void );
	void End_BalconyKilled( void );
	void Think_BalconyKilled( void );
	void FinishedAnimation_BalconyKilled( void );
	bool CalcFallPath( void );
	static void InitPain( GlobalFuncs_t *func );
	void Begin_Pain( void );
	void Think_Pain( void );
	void FinishedAnimation_Pain( void );
	static void InitDead( GlobalFuncs_t *func );
	static void InitKilled( GlobalFuncs_t *func );
	void Begin_Killed( void );
	void Think_Killed( void );
	void FinishedAnimation_Killed( void );
	static void InitWeaponless( GlobalFuncs_t *func );
	void Begin_Weaponless( void );
	void Suspend_Weaponless( void );
	void Think_Weaponless( void );
	void FinishedAnimation_Weaponless( void );
	void State_Weaponless_Normal( void );
	virtual void Think( void );
	void PostThink( bool bDontFaceWall );
	virtual void SetMoveInfo( mmove_t *mm );
	virtual void GetMoveInfo( mmove_t *mm );
	void DoFailSafeMove( vec3_t dest );
	void TouchStuff( mmove_t *mm );
	void ExtractConstraints( mmove_t *mm );
	void EventGiveWeaponInternal( Event *ev );
	void EventGiveWeapon( Event *ev );
	void EventGetWeapon( Event *ev );
	void FireWeapon( Event *ev );
	virtual bool CanTarget( void );
	virtual bool IsImmortal( void );
	static bool IsVoidState( int state );
	static bool IsIdleState( int state );
	static bool IsCuriousState( int state );
	static bool IsDisguiseState( int state );
	static bool IsAttackState( int state );
	static bool IsGrenadeState( int state );
	static bool IsPainState( int state );
	static bool IsKilledState( int state );
	static bool IsMachineGunnerState( int state );
	static bool IsDogState( int state );
	void IgnoreSoundSet( int iType );
	void IgnoreSoundSetAll( void );
	void IgnoreSoundClear( int iType );
	void IgnoreSoundClearAll( void );
	bool IgnoreSound( int iType );
	void EventShareEnemy( Event *ev );
	void EventShareGrenade( Event *ev );
	void ReceiveAIEvent( vec3_t event_origin, int iType, Entity *originator, float fDistSquared, float fMaxDistSquared );
	void DefaultReceiveAIEvent( vec3_t event_origin, int iType, Entity *originator, float fDistSquared, float fMaxDistSquared );
	int PriorityForEventType( int iType );
	void CuriousSound( int iType, vec3_t sound_origin, float fDistSquared, float fMaxDistSquared );
	void WeaponSound( int iType, vec3_t sound_origin, float fDistSquared, float fMaxDistSquared, Entity *originator );
	void FootstepSound( vec3_t sound_origin, float fDistSquared, float fMaxDistSquared, Entity *originator );
	void VoiceSound( int iType, vec3_t sound_origin, float fDistSquared, float fMaxDistSquared, Entity *originator );
	void GrenadeNotification( Entity *originator );
	void SetGrenade( Entity *pGrenade );
	void NotifySquadmateKilled( Sentient *pSquadMate, Sentient *pAttacker );
	void RaiseAlertnessForEventType( int iType );
	void RaiseAlertness( float fAmout );
	virtual bool CanSee( Entity *e1, float fov, float vision_distance );
	virtual Vector GunPosition( void );
	bool WithinVisionDistance( Entity *ent ) const;
	bool InFOV( Vector pos, float check_fov, float check_fovdot );
	bool EnemyInFOV( int iMaxDirtyTime );
	bool InFOV( Vector pos );
	bool InFOV( Entity *ent );
	bool CanSeeNoFOV( Entity *ent );
	bool CanSeeFOV( Entity *ent );
	bool CanSeeEnemyFOV( int iMaxFovDirtyTime, int iMaxSightDirtyTime );
	bool CanShoot( Entity *ent );
	virtual bool CanSeeFrom( vec3_t pos, Entity *ent );
	bool CanSeeEnemy( int iMaxDirtyTime );
	bool CanShootEnemy( int iMaxDirtyTime );
	void ShowInfo( void );
	virtual void ShowInfo( float fDot, float fDist );
	void DefaultPain( Event *ev );
	void HandlePain( Event *ev );
	void EventPain( Event *ev );
	void DefaultKilled( Event *ev, bool bPlayDeathAnim );
	void HandleKilled( Event *ev, bool bPlayDeathAnim );
	void DispatchEventKilled( Event *ev, bool bPlayDeathAnim );
	void EventKilled( Event *ev );
	void EventBeDead( Event *ev );
	void DeathEmbalm( Event *ev );
	void DeathSinkStart( Event *ev );
	bool NoticeShot( Sentient *pShooter, Sentient *pTarget, float fDist );
	bool NoticeFootstep( Sentient *pPedestrian );
	bool NoticeVoice( Sentient *pVocallist );
	void ClearLookEntity( void );
	void LookAt( Vector& vec );
	void LookAt( Listener *l );
	void ForwardLook( void );
	void LookAtLookEntity( void );
	void IdleLook( void );
	void IdleLook( vec3_t dir );
	void SetDesiredLookDir( vec3_t dir );
	void SetDesiredLookAnglesRelative( vec3_t ang );
	void EventLookAt( Event *ev );
	void EventEyesLookAt( Event *ev );
	void NoPoint( void );
	void IdlePoint( void );
	void ClearPointEntity( void );
	void PointAt( Vector& vec );
	void PointAt( Listener* l );
	void EventPointAt( Event *ev );
	void ClearTurnEntity( void );
	void TurnTo( Vector& vec );
	void TurnTo( Listener *l  );
	void IdleTurn( void );
	void EventTurnTo( Event *ev );
	void EventSetTurnDoneError( Event *ev );
	void EventGetTurnDoneError( Event *ev );
	void LookAround( float fFovAdd );
	bool SoundSayAnim( const_str name, int bLevelSayAnim );
	void EventSetAnim( Event *ev );
	void EventIdleSayAnim( Event *ev );
	void EventSayAnim( Event *ev );
	void EventSetSayAnim( Event *ev );
	void EventSetMotionAnim( Event *ev );
	void EventSetAimMotionAnim( Event *ev );
	void EventSetActionAnim( Event *ev );
	void EventUpperAnim( Event *ev );
	void EventSetUpperAnim( Event *ev );
	void EventEndActionAnim( Event *ev );
	void EventDamagePuff( Event *ev );
	void SafeSetOrigin( vec3_t newOrigin );
	void DoMove( void );
	void AnimFinished( int slot, bool stop );
	virtual void AnimFinished( int slot );
	void PlayAnimation( Event *ev );
	void PlayScriptedAnimation( Event *ev );
	void PlayNoclipAnimation( Event *ev );
	void MoveDest( float fMoveSpeed );
	void MovePath( float fMoveSpeed );
	void MovePathGoal( float fMoveSpeed );
	void Dumb( Event *ev );
	void PhysicsOn( Event *ev );
	void PhysicsOff( Event *ev );
	void EventStart( Event *ev );
	void EventGetMood( Event *ev );
	void EventSetMood( Event *ev );
	void EventGetAngleYawSpeed( Event *ev );
	void EventSetAngleYawSpeed( Event *ev );
	void EventSetAimTarget( Event *ev );
	void UpdateAngles( void );
	void SetLeashHome( Vector vHome );
	void AimAtTargetPos( void );
	void AimAtAimNode( void );
	void AimAtEnemyBehavior( void );
	void FaceMotion( void );
	void FaceDirectionDuringMotion( vec3_t vLook );
	float PathDistanceAlongVector( vec3_t vDir );
	void FaceEnemyOrMotion( int iTimeIntoMove );
	static int NextUpdateTime( int iLastUpdateTime, int iUpdatePeriod );
	void ResetBoneControllers( void );
	void UpdateBoneControllers( void );
	void ReadyToFire( Event *ev );
	void GetLocalYawFromVector( Event *ev );
	void EventGetSight( Event *ev );
	void EventSetSight( Event *ev );
	void EventGetHearing( Event *ev );
	void EventSetHearing( Event *ev );
	void ClearPatrolCurrentNode( void );
	void NextPatrolCurrentNode( void );
	void SetPatrolCurrentNode( Vector& vec );
	void SetPatrolCurrentNode( Listener *l );
	void EventSetPatrolPath( Event *ev );
	void EventGetPatrolPath( Event *ev );
	void EventSetPatrolWaitTrigger( Event *ev );
	void EventGetPatrolWaitTrigger( Event *ev );
	void ShowInfo_PatrolCurrentNode( void );
	bool MoveOnPathWithSquad( void );
	bool MoveToWaypointWithPlayer( void );
	bool PatrolNextNodeExists( void );
	void UpdatePatrolCurrentNode( void );
	bool MoveToPatrolCurrentNode( void );
	void ClearAimNode( void );
	void SetAimNode( Vector& vec );
	void SetAimNode( Listener *l );
	void ShowInfo_AimNode( void );
	void EventSetAccuracy( Event *ev );
	void EventGetAccuracy( Event *ev );
	int GetThinkType( const_str csName );
	void SetThink( int state, int think );
	void SetThinkIdle( int think_idle );
	void SetThinkState( int state, int level );
	void EndCurrentThinkState( void );
	void ClearThinkStates( void );
	int CurrentThink( void ) const;
	bool IsAttacking( void ) const;
	void EventGetFov( Event *ev );
	void EventSetFov( Event *ev );
	void EventSetDestIdle( Event *ev );
	void EventSetDestIdle2( Event *ev );
	void EventSetTypeIdle( Event *ev );
	void EventGetTypeIdle( Event *ev );
	void EventSetTypeAttack( Event *ev );
	void EventGetTypeAttack( Event *ev );
	void EventSetTypeDisguise( Event *ev );
	void EventGetTypeDisguise( Event *ev );
	void EventSetDisguiseLevel( Event *ev );
	void EventGetDisguiseLevel( Event *ev );
	void EventSetTypeGrenade( Event *ev );
	void EventGetTypeGrenade( Event *ev );
	void EventSetMinDistance( Event *ev );
	void EventGetMinDistance( Event *ev );
	void EventSetMaxDistance( Event *ev );
	void EventGetMaxDistance( Event *ev );
	void EventGetLeash( Event *ev );
	void EventSetLeash( Event *ev );
	void EventGetInterval( Event *ev );
	void EventSetInterval( Event *ev );
	void EventDistToEnemy( Event *ev );
	void EventGetRunAnim( Event *ev );
	void EventGetWalkAnim( Event *ev );
	void EventGetAnimName( Event *ev );
	void EventSetAnimName( Event *ev );
	void EventSetDisguiseRange( Event *ev );
	void EventGetDisguiseRange( Event *ev );
	void EventSetDisguisePeriod( Event *ev );
	void EventGetDisguisePeriod( Event *ev );
	void EventSetDisguiseAcceptThread( Event *ev );
	void EventGetDisguiseAcceptThread( Event *ev );
	void EventAttackPlayer( Event *ev );
	void ForceAttackPlayer( void );
	void EventSetAlarmNode( Event *ev );
	void EventGetAlarmNode( Event *ev );
	void EventSetAlarmThread( Event *ev );
	void EventGetAlarmThread( Event *ev );
	void EventSetSoundAwareness( Event *ev );
	void EventGetSoundAwareness( Event *ev );
	void EventSetGrenadeAwareness( Event *ev );
	void EventGetGrenadeAwareness( Event *ev );
	str ThinkName( void ) const;
	str ThinkStateName( void ) const;
	void EventSetTurret( Event *ev );
	void EventGetTurret( Event *ev );
	void EventEnableEnemy( Event *ev );
	void EventEnablePain( Event *ev );
	void EventActivate( Event *ev );
	void EventGetAmmoGrenade( Event *ev );
	void EventSetAmmoGrenade( Event *ev );
	void EventInterruptPoint( Event *ev );
	void EventAnimScript( Event *ev );
	void EventAnimScript_Scripted( Event *ev );
	void EventAnimScript_Noclip( Event *ev );
	void EventReload_mg42( Event *ev );
	void SetPathWithLeash( Vector vDestPos, const char *description, int iMaxDirtyTime );
	void SetPathWithLeash( SimpleEntity *pDestNode, const char *description, int iMaxDirtyTime );
	void FindPathAwayWithLeash( vec3_t vAwayFrom, vec3_t vDirPreferred, float fMinSafeDist );
	void FindPathNearWithLeash( vec3_t vNearbyTo, float fCloseDistSquared );
	bool CanMovePathWithLeash( void ) const;
	bool MovePathWithLeash( void );
	bool ShortenPathToAttack( float fMinDist );
	void StrafeToAttack( float fDist, vec3_t vDir );
	virtual Vector GunTarget( bool bNoCollision );
	virtual qboolean setModel( void );
	void EventSetHeadModel( Event *ev );
	void EventGetHeadModel( Event *ev );
	void EventSetHeadSkin( Event *ev );
	void EventGetHeadSkin( Event *ev );
	void EventSetNoIdle( Event *ev );
	void EventGetNoIdle( Event *ev );
	void EventGetEnemy( Event *ev );
	void EventSetMaxNoticeTimeScale( Event *ev );
	void EventGetMaxNoticeTimeScale( Event *ev );
	void EventSetFixedLeash( Event *ev );
	void EventGetFixedLeash( Event *ev );
	void Holster( void );
	void Unholster( void );
	void EventHolster( Event *ev );
	void EventUnholster( Event *ev );
	void EventSoundDone( Event *ev );
	void EventSound( Event *ev );
	void EventIsEnemyVisible( Event *ev );
	void EventGetEnemyVisibleChangeTime( Event *ev );
	void EventGetLastEnemyVisibleTime( Event *ev );
	void EventSetFallHeight( Event *ev );
	void EventGetFallHeight( Event *ev );
	void EventCanMoveTo( Event *ev );
	void EventMoveDir( Event *ev );
	void EventIntervalDir( Event *ev );
	void EventResetLeash( Event *ev );
	void EventTether( Event *ev );
	void EventGetThinkState( Event *ev );
	void EventGetEnemyShareRange( Event *ev );
	void EventSetEnemyShareRange( Event *ev );
	void EventGetKickDir( Event *ev );
	void EventGetNoLongPain( Event *ev );
	void EventSetNoLongPain( Event *ev );
	void EventGetFavoriteEnemy( Event *ev );
	void EventSetFavoriteEnemy( Event *ev );
	void EventGetMumble( Event *ev );
	void EventSetMumble( Event *ev );
	void EventGetBreathSteam( Event *ev );
	void EventSetBreathSteam( Event *ev );
	void EventSetNextBreathTime( Event *ev );
	void EventCalcGrenadeToss( Event *ev );
	void EventGetNoSurprise( Event *ev );
	void EventSetNoSurprise( Event *ev );
	void EventGetSilent( Event *ev );
	void EventSetSilent( Event *ev );
	void EventGetAvoidPlayer( Event *ev );
	void EventSetAvoidPlayer( Event *ev );
	void EventGetLookAroundAngle( Event *ev );
	void EventSetLookAroundAngle( Event *ev );
	void EventHasCompleteLookahead( Event *ev );
	void EventPathDist( Event *ev );
	void EventCanShootEnemyFrom( Event *ev );
	void EventCanShoot( Event *ev );
	void EventSetInReload( Event *ev );
	void EventGetInReload( Event *ev );
	void EventSetReloadCover( Event *ev );
	void EventBreakSpecial( Event *ev );
	void GetVoiceType( Event *ev );
	void SetVoiceType( Event *ev );
	void ResolveVoiceType( void );
	void EventSetBalconyHeight( Event *ev );
	void EventGetBalconyHeight( Event *ev );
	PathNode *FindSniperNodeAndSetPath( bool *pbTryAgain );
	void Remove( Event *ev );
	void DontFaceWall( void );
	bool AvoidingFacingWall( void ) const;
	void EndStates( void );
	void ClearStates( void );
	void CheckUnregister( void );
	void BecomeCorpse( void );
	virtual void PathnodeClaimRevoked( PathNode *node );
	void SetPathToNotBlockSentient( Sentient *pOther );
	void EventSetMoveDoneRadius( Event *ev );
	virtual void ClearEnemies( void );
	bool EnemyIsDisguised( void );
	virtual void setOriginEvent( Vector org );
	virtual void DumpAnimInfo( void );
	static void ArchiveStatic( Archiver& arc );
	virtual void Archive( Archiver& arc );
	virtual bool AutoArchiveModel( void );
};

#define SAVE_FLAG_NEW_ANIM						(1<<0)
#define SAVE_FLAG_FORWARD_SPEED				(1<<1)
#define SAVE_FLAG_BEHAVIOR						(1<<2)
#define SAVE_FLAG_PATH							(1<<3)
#define SAVE_FLAG_NOISE							(1<<4)
#define SAVE_FLAG_SCRIPT_THREAD				(1<<5)
#define SAVE_FLAG_ACTOR_THREAD				(1<<6)
#define SAVE_FLAG_KILL_THREAD					(1<<7)
#define SAVE_FLAG_STATE							(1<<8)
#define SAVE_FLAG_IDLE_THREAD					(1<<7)
#define SAVE_FLAG_PARTS							(1<<10)
#define SAVE_FLAG_TRIGGER						(1<<11)
#define SAVE_FLAG_STATE_FLAGS					(1<<12)
#define SAVE_FLAG_COMMAND						(1<<13)
#define SAVE_FLAG_STAGE							(1<<14)
#define SAVE_FLAG_NUM_OF_SPAWNS				(1<<15)
#define SAVE_FLAG_SPAWN_PARENT				(1<<16)
#define SAVE_FLAG_DIALOG						(1<<17)
#define SAVE_FLAG_SAVED_STUFF					(1<<18)
#define SAVE_FLAG_LAST_ANIM_EVENT			(1<<19)
#define SAVE_FLAG_PICKUP_ENT					(1<<20)
#define SAVE_FLAG_PAIN							(1<<21)
#define SAVE_FLAG_SPAWN_ITEMS					(1<<22)

inline void Actor::ArchiveStatic
	(
	Archiver &arc
	)

{
	for( int i = MAX_BODYQUEUE - 1; i >= 0; i-- )
	{
		arc.ArchiveSafePointer( &mBodyQueue[ i ] );
	}

	arc.ArchiveInteger( &mCurBody );
}

inline void Actor::Archive
	(
	Archiver &arc
	)

{
	SimpleActor::Archive( arc );

	for( int i = MAX_THINKMAP - 1; i >= 0; i-- )
	{
		arc.ArchiveInteger( &m_ThinkMap[ i ] );
	}

	for( int i = MAX_THINKSTATES - 1; i >= 0; i-- )
	{
		arc.ArchiveInteger( &m_ThinkStates[ i ] );
		arc.ArchiveInteger( &m_Think[ i ] );
	}

	arc.ArchiveInteger( &m_ThinkLevel );
	arc.ArchiveInteger( &m_ThinkState);
	arc.ArchiveInteger( &m_State);
	arc.ArchiveInteger( &m_iStateTime);
	arc.ArchiveBool( &m_bLockThinkState);
	arc.ArchiveBool( &m_bDirtyThinkState );
	arc.ArchiveInteger( &mVoiceType );
	arc.ArchiveBool( &m_bSilent);
	arc.ArchiveBool( &m_bNoSurprise);
	arc.ArchiveBool( &m_bMumble);
	arc.ArchiveBool( &m_bBreathSteam);

	Director.ArchiveString( arc, m_csHeadModel );
	Director.ArchiveString( arc, m_csHeadSkin );
	Director.ArchiveString( arc, m_csWeapon );
	Director.ArchiveString( arc, m_csLoadOut );
	arc.ArchiveSafePointer( &m_FavoriteEnemy );
	arc.ArchiveInteger( &m_iEnemyCheckTime );
	arc.ArchiveInteger( &m_iEnemyChangeTime );
	arc.ArchiveInteger( &m_iEnemyVisibleCheckTime );
	arc.ArchiveInteger( &m_iEnemyVisibleChangeTime );
	arc.ArchiveInteger( &m_iLastEnemyVisibleTime );
	arc.ArchiveInteger( &m_iEnemyFovCheckTime );
	arc.ArchiveInteger( &m_iEnemyFovChangeTime );
	arc.ArchiveVector( &m_vLastEnemyPos );
	arc.ArchiveInteger( &m_iLastEnemyPosChangeTime );
	arc.ArchiveFloat( &m_fMaxShareDistSquared );
	arc.ArchiveInteger( &m_iCanShootCheckTime );
	arc.ArchiveBool( &m_bCanShootEnemy );
	arc.ArchiveBool( &m_bDesiredEnableEnemy );
	arc.ArchiveBool( &m_bEnableEnemy );
	arc.ArchiveBool( &m_bEnablePain );
	arc.ArchiveBool( &m_bNoLongPain );
	arc.ArchiveBool( &m_bNewEnemy );
	arc.ArchiveBool( &m_bEnemyIsDisguised );
	arc.ArchiveBool( &m_bEnemyVisible );
	arc.ArchiveBool( &m_bEnemyInFOV );
	arc.ArchiveBool( &m_bForceAttackPlayer );
	arc.ArchiveBool( &m_bAutoAvoidPlayer );
	arc.ArchiveBool( &m_bNoIdleAfterAnim );
	Director.ArchiveString( arc, m_csAnimScript );
	arc.ArchiveBool( &m_bAnimScriptSet );
	arc.ArchiveInteger( &m_AnimMode );
	arc.ArchiveFloat( &m_fDfwRequestedYaw );
	arc.ArchiveFloat( &m_fDfwDerivedYaw );
	arc.ArchiveVector( &m_vDfwPos );
	arc.ArchiveFloat( &m_fDfwTime );
	arc.ArchiveInteger( &m_iGunPositionCheckTime );
	arc.ArchiveVector( &m_vGunPosition );
	arc.ArchiveInteger( &m_iWallDodgeTimeout );
	arc.ArchiveFloat( &m_PrevObstacleNormal[ 0 ] );
	arc.ArchiveFloat( &m_PrevObstacleNormal[ 1 ] );
	arc.ArchiveChar( &m_WallDir );
	arc.ArchiveFloat( &m_fMoveDoneRadiusSquared );
	arc.ArchiveBool( &m_bFaceEnemy );
	arc.ArchiveBool( &m_bDoPhysics );
	arc.ArchiveBool( &m_bPatrolWaitTrigger );
	arc.ArchiveBool( &m_bBecomeRunner );
	arc.ArchiveBool( &m_bScriptGoalValid );
	arc.ArchiveVector( &m_vScriptGoal );
	arc.ArchiveInteger( &m_iNextWatchStepTime );
	arc.ArchiveSafePointer( &m_patrolCurrentNode );
	Director.ArchiveString( arc, m_csPatrolCurrentAnim );
	arc.ArchiveInteger( &m_iSquadStandTime );
	arc.ArchiveFloat( &m_fInterval );
	arc.ArchiveInteger( &m_iIntervalDirTime );
	arc.ArchiveVector( &m_vIntervalDir );
	arc.ArchiveShort( &m_sCurrentPathNodeIndex );
	arc.ArchiveInteger( &m_PainState );
	arc.ArchiveInteger( &m_iCuriousTime );
	arc.ArchiveInteger( &m_iCuriousLevel );
	arc.ArchiveInteger( &m_iCuriousAnimHint );
	arc.ArchiveInteger( &m_iNextDisguiseTime );
	arc.ArchiveInteger( &m_iDisguisePeriod );
	arc.ArchiveFloat( &m_fMaxDisguiseDistSquared );
	arc.ArchiveInteger( &m_iEnemyShowPapersTime );
	m_DisguiseAcceptThread.Archive( arc );
	arc.ArchiveInteger( &m_iDisguiseLevel );
	arc.ArchiveSafePointer( &m_AlarmNode );
	m_AlarmThread.Archive( arc );
	arc.ArchiveInteger( &m_iRunHomeTime );
	arc.ArchiveBool( &m_bTurretNoInitialCover );
	arc.ArchiveInteger( &m_iPotentialCoverCount );

	for( int i = MAX_COVER_NODES - 1; i >= 0; i-- )
	{
		arc.ArchiveObjectPointer( ( Class ** ) &m_pPotentialCoverNode[ i ] );
	}

	arc.ArchiveObjectPointer( ( Class ** )&m_pCoverNode );
	Director.ArchiveString( arc, m_csSpecialAttack );

	arc.ArchiveBool( &m_bInReload);
	arc.ArchiveBool( &m_bNeedReload);
	arc.ArchiveBool( &mbBreakSpecialAttack);
	arc.ArchiveBool( &m_bGrenadeBounced);
	arc.ArchiveSafePointer( &m_pGrenade);
	arc.ArchiveVector( &m_vGrenadePos);
	arc.ArchiveInteger( &m_iFirstGrenadeTime);
	arc.ArchiveInteger( &m_eGrenadeState);
	ArchiveEnum( m_eGrenadeMode, eGrenadeTossMode );
	arc.ArchiveVector( &m_vGrenadeVel);
	arc.ArchiveVector( &m_vKickDir);
	arc.ArchiveFloat( &m_fNoticeTimeScale);
	arc.ArchiveFloat( &m_fMaxNoticeTimeScale);
	m_PotentialEnemies.Archive( arc );
	arc.ArchiveFloat( &m_fSight );
	arc.ArchiveFloat( &m_fHearing );
	arc.ArchiveFloat( &m_fSoundAwareness );
	arc.ArchiveFloat( &m_fGrenadeAwareness );
	arc.ArchiveInteger( &m_iIgnoreSoundsMask );
	arc.ArchiveFloat( &m_fFov );
	arc.ArchiveFloat( &m_fFovDot );
	arc.ArchiveInteger( &m_iEyeUpdateTime );
	arc.ArchiveVector( &m_vEyeDir );
	arc.ArchiveInteger( &m_iNextLookTime );
	arc.ArchiveFloat( &m_fLookAroundFov );
	arc.ArchiveSafePointer( &m_pLookEntity );
	arc.ArchiveInteger( &m_iLookFlags );
	arc.ArchiveSafePointer( &m_pPointEntity );
	arc.ArchiveSafePointer( &m_pTurnEntity );
	arc.ArchiveFloat( &m_fTurnDoneError );
	arc.ArchiveFloat( &m_fAngleYawSpeed );
	arc.ArchiveSafePointer( &m_aimNode );
	arc.ArchiveInteger( &m_eDontFaceWallMode );
	arc.ArchiveInteger( &m_iLastFaceDecideTime );
	arc.ArchiveBool( &m_bHeadAnglesAchieved );
	arc.ArchiveBool( &m_bLUpperArmAnglesAchieved );
	arc.ArchiveBool( &m_bTorsoAnglesAchieved );
	arc.ArchiveFloat( &m_fHeadMaxTurnSpeed );
	arc.ArchiveVec3( m_vHeadDesiredAngles );
	arc.ArchiveFloat( &m_fLUpperArmTurnSpeed );
	arc.ArchiveVec3( m_vLUpperArmDesiredAngles );
	arc.ArchiveFloat( &m_fTorsoMaxTurnSpeed );
	arc.ArchiveFloat( &m_fTorsoCurrentTurnSpeed );
	arc.ArchiveVec3( m_vTorsoDesiredAngles );
	arc.ArchiveVector( &m_vHome );
	arc.ArchiveSafePointer( &m_pTetherEnt );
	arc.ArchiveFloat( &m_fMinDistance );
	arc.ArchiveFloat( &m_fMinDistanceSquared );
	arc.ArchiveFloat( &m_fMaxDistance );
	arc.ArchiveFloat( &m_fMaxDistanceSquared );
	arc.ArchiveFloat( &m_fLeash );
	arc.ArchiveFloat( &m_fLeashSquared );
	arc.ArchiveBool( &m_bFixedLeash );
	arc.ArchiveBool( &m_bAnimating );
	arc.ArchiveBool( &m_bDog );

	byte length;

	if( arc.Saving() )
	{
		if( m_pFallPath )
			length = m_pFallPath->length;
		else
			length = 0;
	}
	arc.ArchiveByte( &length );

	if( length )
	{
		if( arc.Loading() )
		{
			m_pFallPath = ( FallPath * )gi.Malloc( sizeof( FallPath ) * length );
			m_pFallPath->length = length;
		}

		arc.ArchiveByte( &m_pFallPath->currentPos );
		arc.ArchiveByte( &m_pFallPath->loop );

		for( int i = 0; i < length; i++ )
		{
			arc.ArchiveVec3( m_pFallPath->pos[ i ] );
		}
	}

	arc.ArchiveFloat( &m_fBalconyHeight );
	arc.ArchiveBool( &m_bNoPlayerCollision );

	for( int i = MAX_ORIGIN_HISTORY - 1; i >= 0; i-- )
	{
		arc.ArchiveVec2( m_vOriginHistory[ i ] );
	}

	arc.ArchiveInteger( &m_iCurrentHistory );

	// set the model
	setModel();
}

#if 0

// Set destination to node with duck or cover set.  Class will find a path to that node, or a closer one.
class FindCoverMovement : public StandardMovement
	{
	public:
		Actor			*self;

	inline qboolean validpath
		(
		PathNode *node,
		int i
		)

		{
		pathway_t *path;
		PathNode *n;

		path = &node->Child[ i ];

		if ( !StandardMovement::validpath( node, i ) )
			{
			return false;
			}

		n = AI_GetNode( path->node );
      if ( !n || self->CloseToEnemy( n->origin, 128 ) )
			{
			return false;
			}

		return true;
		}

	inline qboolean done
		(
		PathNode *node,
		PathNode *end
		)

		{
		if ( node == end )
			{
			return true;
			}

		//if ( node->reject )
		if ( node->reject || !( node->nodeflags & ( AI_DUCK | AI_COVER ) ) )
			{
			return false;
			}

		if ( self )
			{
         node->reject = self->CanSeeEnemyFrom( node->origin );
			return !node->reject;
			}

		return false;
		}
	};

// Set destination to node with flee set.  Class will find a path to that node, or a closer one.
class FindFleeMovement : public StandardMovement
	{
	public:
		Actor			*self;

	inline qboolean validpath
		(
		PathNode *node,
		int i
		)

		{
		pathway_t *path;
		PathNode *n;

		path = &node->Child[ i ];

		if ( !StandardMovement::validpath( node, i ) )
			{
			return false;
			}

		n = AI_GetNode( path->node );
      if ( !n || self->CloseToEnemy( n->origin, 128 ) )
			{
			return false;
			}

		return true;
		}

	inline qboolean done
		(
		PathNode *node,
		PathNode *end
		)

		{
		if ( node == end )
			{
			return true;
			}

		//if ( node->reject )
		if ( node->reject || !( node->nodeflags & AI_FLEE ) )
			{
			return false;
			}

		if ( self )
			{
         node->reject = self->CanSeeEnemyFrom( node->origin );
			return !node->reject;
			}

		return false;
		}
	};

class FindEnemyMovement : public StandardMovement
	{
	public:
		Actor			*self;

	inline qboolean done
		(
		PathNode *node,
		PathNode *end
		)

		{
		if ( node == end )
			{
			return true;
			}

		if ( node->reject )
			{
			return false;
			}

		if ( self )
			{
			if ( self->currentEnemy )
   			{
            node->reject = !self->CanShootFrom( node->origin, self->currentEnemy, false );
			   }
			else
			   {
				node->reject = false;
			   }

         return !node->reject;
			}

		return false;
		}
	};

typedef PathFinder<FindCoverMovement> FindCoverPath;
typedef PathFinder<FindFleeMovement> FindFleePath;
typedef PathFinder<FindEnemyMovement> FindEnemyPath;
#endif

void AI_TargetPlayer( void );

class SpinningPlant : public Actor
	{
	public:
		Mover *spinner_model;
		Mover *spinner_clip;

		CLASS_PROTOTYPE( SpinningPlant );

				SpinningPlant();
				~SpinningPlant();

		void	GetClip( Event *ev );
	};

#endif