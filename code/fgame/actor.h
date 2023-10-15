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

#pragma once

#include "g_local.h"
#include "simpleactor.h"
#include "actorenemy.h"
#include "level.h"
#include "game.h"
#include "gamescript.h"
#include "scriptmaster.h"
#include "grenadehint.h"
#include "parm.h"

//
// Export these events for AISpawnPoint (since 2.30)
//
extern Event EV_Actor_SetGun;
extern Event EV_Actor_SetGun2;
extern Event EV_Actor_GetGun;
extern Event EV_Actor_GetSight;
extern Event EV_Actor_SetSight;
extern Event EV_Actor_SetSight2;
extern Event EV_Actor_GetHearing;
extern Event EV_Actor_SetHearing;
extern Event EV_Actor_SetHearing2;
extern Event EV_Actor_GetFov;
extern Event EV_Actor_SetFov;
extern Event EV_Actor_SetFov2;
extern Event EV_Actor_SetTypeIdle;
extern Event EV_Actor_SetTypeIdle2;
extern Event EV_Actor_GetTypeIdle;
extern Event EV_Actor_SetTypeAttack;
extern Event EV_Actor_SetTypeAttack2;
extern Event EV_Actor_GetTypeAttack;
extern Event EV_Actor_SetTypeDisguise;
extern Event EV_Actor_SetTypeDisguise2;
extern Event EV_Actor_GetTypeDisguise;
extern Event EV_Actor_SetDisguiseLevel;
extern Event EV_Actor_SetDisguiseLevel2;
extern Event EV_Actor_GetDisguiseLevel;
extern Event EV_Actor_SetTypeGrenade;
extern Event EV_Actor_SetTypeGrenade2;
extern Event EV_Actor_GetTypeGrenade;
extern Event EV_Actor_SetPatrolPath;
extern Event EV_Actor_SetPatrolPath2;
extern Event EV_Actor_GetPatrolPath;
extern Event EV_Actor_SetPatrolWaitTrigger;
extern Event EV_Actor_SetPatrolWaitTrigger2;
extern Event EV_Actor_GetPatrolWaitTrigger;
extern Event EV_Actor_SetAlarmNode;
extern Event EV_Actor_SetAlarmNode2;
extern Event EV_Actor_GetAlarmNode;
extern Event EV_Actor_SetAccuracy;
extern Event EV_Actor_SetAccuracy2;
extern Event EV_Actor_GetAccuracy;
extern Event EV_Actor_SetMinDistance;
extern Event EV_Actor_SetMinDistance2;
extern Event EV_Actor_GetMinDistance;
extern Event EV_Actor_SetMaxDistance;
extern Event EV_Actor_SetMaxDistance2;
extern Event EV_Actor_GetMaxDistance;
extern Event EV_Actor_GetLeash;
extern Event EV_Actor_SetLeash;
extern Event EV_Actor_SetLeash2;
extern Event EV_Actor_GetInterval;
extern Event EV_Actor_SetInterval;
extern Event EV_Actor_SetInterval2;
extern Event EV_Actor_SetDisguiseRange;
extern Event EV_Actor_SetDisguiseRange2;
extern Event EV_Actor_GetDisguiseRange;
extern Event EV_Actor_SetDisguisePeriod;
extern Event EV_Actor_SetDisguisePeriod2;
extern Event EV_Actor_GetDisguisePeriod;
extern Event EV_Actor_SetGrenadeAwareness;
extern Event EV_Actor_SetGrenadeAwareness2;
extern Event EV_Actor_GetGrenadeAwareness;
extern Event EV_Actor_SetTurret;
extern Event EV_Actor_SetTurret2;
extern Event EV_Actor_GetTurret;
extern Event EV_Actor_SetSoundAwareness;
extern Event EV_Actor_SetSoundAwareness2;
extern Event EV_Actor_GetSoundAwareness;
extern Event EV_Actor_SetGrenadeAwareness;
extern Event EV_Actor_SetGrenadeAwareness2;
extern Event EV_Actor_SetAmmoGrenade;
extern Event EV_Actor_SetAmmoGrenade2;
extern Event EV_Actor_GetAmmoGrenade;
extern Event EV_Actor_GetMaxNoticeTimeScale;
extern Event EV_Actor_SetMaxNoticeTimeScale;
extern Event EV_Actor_SetMaxNoticeTimeScale2;
extern Event EV_Actor_GetFixedLeash;
extern Event EV_Actor_SetFixedLeash;
extern Event EV_Actor_SetFixedLeash2;
extern Event EV_Actor_GetEnemyShareRange;
extern Event EV_Actor_SetEnemyShareRange;
extern Event EV_Actor_SetEnemyShareRange2;
extern Event EV_Actor_SetWeapon;
extern Event EV_Actor_GetWeapon;
extern Event EV_Actor_GetVoiceType;
extern Event EV_Actor_SetVoiceType;
extern Event EV_Actor_SetVoiceType2;
extern Event EV_Actor_GetFavoriteEnemy;
extern Event EV_Actor_SetFavoriteEnemy;
extern Event EV_Actor_SetFavoriteEnemy2;
extern Event EV_Actor_SetBalconyHeight;
extern Event EV_Actor_SetBalconyHeight2;
extern Event EV_Actor_GetBalconyHeight;
extern Event EV_Actor_GetNoSurprise;
extern Event EV_Actor_SetNoSurprise;
extern Event EV_Actor_SetNoSurprise2;
extern Event EV_Actor_DeathEmbalm;

// Bones used by actor

//#define ACTOR_MOUTH_TAG  0
#define ACTOR_HEAD_TAG  0
#define ACTOR_TORSO_TAG 1

// Dialog stuff

#define LIP_SYNC_HZ                   20.0

#define MAX_DIALOG_PARAMETERS_LENGTH  100

#define MAX_DIALOG_PARM_LENGTH        64
#define MAX_DIALOG_PARMS              10

#define DIALOG_PARM_TYPE_NONE         0
#define DIALOG_PARM_TYPE_PLAYERHAS    1
#define DIALOG_PARM_TYPE_PLAYERHASNOT 2
#define DIALOG_PARM_TYPE_HAS          3
#define DIALOG_PARM_TYPE_HASNOT       4
#define DIALOG_PARM_TYPE_DEPENDS      5
#define DIALOG_PARM_TYPE_DEPENDSNOT   6

typedef struct {
    byte type;
    char parm[MAX_DIALOG_PARM_LENGTH];
} DialogParm_t;

typedef struct DialogNode_s {
    char                 alias_name[MAX_ALIAS_NAME_LENGTH];
    int                  random_flag;
    int                  number_of_parms;
    float                random_percent;
    DialogParm_t         parms[MAX_DIALOG_PARMS];
    struct DialogNode_s *next;
} DialogNode_t;

typedef enum {
    IS_INANIMATE,
    IS_MONSTER,
    IS_ENEMY,
    IS_CIVILIAN,
    IS_FRIEND,
    IS_ANIMAL,
    NUM_ACTORTYPES
} actortype_t;

// Stimuli types

#define STIMULI_ALL       -1
#define STIMULI_NONE      0
#define STIMULI_SIGHT     (1 << 0)
#define STIMULI_SOUND     (1 << 1)
#define STIMULI_PAIN      (1 << 2)
#define STIMULI_SCRIPT    (1 << 3)

#define MAX_INACTIVE_TIME 30.0

// State flags

#define STATE_FLAG_IN_PAIN     (1 << 0)
#define STATE_FLAG_MELEE_HIT   (1 << 1)
#define STATE_FLAG_TOUCHED     (1 << 2)
#define STATE_FLAG_ACTIVATED   (1 << 3)
#define STATE_FLAG_USED        (1 << 4)
#define STATE_FLAG_TWITCH      (1 << 5)
#define STATE_FLAG_BLOCKED_HIT (1 << 6)
#define STATE_FLAG_SMALL_PAIN  (1 << 7)
#define STATE_FLAG_OTHER_DIED  (1 << 8)
#define STATE_FLAG_STUCK       (1 << 9)
#define STATE_FLAG_NO_PATH     (1 << 10)

// Actor modes

#define ACTOR_MODE_NONE   0
#define ACTOR_MODE_IDLE   1
#define ACTOR_MODE_AI     2
#define ACTOR_MODE_SCRIPT 3
#define ACTOR_MODE_TALK   4

// Pain types

#define PAIN_SMALL 0
#define PAIN_BIG   1

typedef struct {
    EntityPtr    ent;
    unsigned int state_flags;
} part_t;

// Actor flags

#define ACTOR_FLAG_NOISE_HEARD               0
#define ACTOR_FLAG_INVESTIGATING             1
#define ACTOR_FLAG_DEATHGIB                  2
#define ACTOR_FLAG_DEATHFADE                 3
#define ACTOR_FLAG_NOCHATTER                 4
#define ACTOR_FLAG_INACTIVE                  5
#define ACTOR_FLAG_ANIM_DONE                 6
#define ACTOR_FLAG_STATE_DONE_TIME_VALID     7
#define ACTOR_FLAG_AI_ON                     8
#define ACTOR_FLAG_LAST_CANSEEENEMY          9
#define ACTOR_FLAG_LAST_CANSEEENEMY_NOFOV    10
#define ACTOR_FLAG_DIALOG_PLAYING            11
#define ACTOR_FLAG_ALLOW_TALK                12
#define ACTOR_FLAG_DAMAGE_ONCE_ON            13
#define ACTOR_FLAG_DAMAGE_ONCE_DAMAGED       14
#define ACTOR_FLAG_BOUNCE_OFF                15
#define ACTOR_FLAG_NOTIFY_OTHERS_AT_DEATH    16
#define ACTOR_FLAG_HAS_THING1                17
#define ACTOR_FLAG_HAS_THING2                18
#define ACTOR_FLAG_HAS_THING3                19
#define ACTOR_FLAG_HAS_THING4                20
#define ACTOR_FLAG_LAST_ATTACK_HIT           21
#define ACTOR_FLAG_STARTED                   22
#define ACTOR_FLAG_ALLOW_HANGBACK            23
#define ACTOR_FLAG_USE_GRAVITY               24
#define ACTOR_FLAG_SPAWN_FAILED              25
#define ACTOR_FLAG_FADING_OUT                26
#define ACTOR_FLAG_DEATHSHRINK               27
#define ACTOR_FLAG_DEATHSINK                 28
#define ACTOR_FLAG_STAYSOLID                 29
#define ACTOR_FLAG_STUNNED                   30
#define ACTOR_FLAG_ALLOW_FALL                31
#define ACTOR_FLAG_FINISHED                  32
#define ACTOR_FLAG_IN_LIMBO                  33
#define ACTOR_FLAG_CAN_WALK_ON_OTHERS        34
#define ACTOR_FLAG_PUSHABLE                  35
#define ACTOR_FLAG_LAST_TRY_TALK             36
#define ACTOR_FLAG_ATTACKABLE_BY_ACTORS      37
#define ACTOR_FLAG_TARGETABLE                38
#define ACTOR_FLAG_ATTACK_ACTORS             39
#define ACTOR_FLAG_IMMORTAL                  40
#define ACTOR_FLAG_TURNING_HEAD              41
#define ACTOR_FLAG_DIE_COMPLETELY            42
#define ACTOR_FLAG_BLEED_AFTER_DEATH         43
#define ACTOR_FLAG_IGNORE_STUCK_WARNING      44
#define ACTOR_FLAG_IGNORE_OFF_GROUND_WARNING 45
#define ACTOR_FLAG_ALLOWED_TO_KILL           46
#define ACTOR_FLAG_TOUCH_TRIGGERS            47
#define ACTOR_FLAG_IGNORE_WATER              48
#define ACTOR_FLAG_NEVER_IGNORE_SOUNDS       49
#define ACTOR_FLAG_SIMPLE_PATHFINDING        50
#define ACTOR_FLAG_HAVE_MOVED                51
#define ACTOR_FLAG_NO_PAIN_SOUNDS            52
#define ACTOR_FLAG_UPDATE_BOSS_HEALTH        53
#define ACTOR_FLAG_IGNORE_PAIN_FROM_ACTORS   54
#define ACTOR_FLAG_DAMAGE_ALLOWED            55
#define ACTOR_FLAG_ALWAYS_GIVE_WATER         56

// Look flags

#define LOOK_FLAG_EYE (1 << 0)

// The last actor_flag number and this one (ACTOR_FLAG_MAX) should match

#define ACTOR_FLAG_MAX     56

#define MAX_ORIGIN_HISTORY 4
#define MAX_COVER_NODES    16
#define MAX_BODYQUEUE      5

typedef enum {
    AI_GREN_TOSS_NONE,
    AI_GREN_TOSS_THROW,
    AI_GREN_TOSS_ROLL,
    AI_GREN_TOSS_HINT,
    AI_GREN_KICK
} eGrenadeTossMode;

typedef enum {
    AI_GRENSTATE_FLEE,
    AI_GRENSTATE_THROW_ACQUIRE,
    AI_GRENSTATE_THROW,
    AI_GRENSTATE_KICK_ACQUIRE,
    AI_GRENSTATE_KICK,
    AI_GRENSTATE_MARTYR_ACQUIRE,
    AI_GRENSTATE_MARTYR,
    AI_GRENSTATE_UNK,
    AI_GRENSTATE_FLEE_SUCCESS, //fled the grenade successfully, I'm safe
    AI_GRENSTATE_FLEE_FAIL,    //failed to flee, I'm gonna get hurt or die :'(
} eGrenadeState;

typedef struct {
    byte   length;
    byte   currentPos;
    byte   loop;
    float  startTime;
    vec3_t pos[1];
} FallPath;

enum eThinkState {
    THINKSTATE_VOID,
    THINKSTATE_IDLE,
    THINKSTATE_PAIN,
    THINKSTATE_KILLED,
    THINKSTATE_ATTACK,
    THINKSTATE_CURIOUS,
    THINKSTATE_DISGUISE,
    THINKSTATE_GRENADE,
    THINKSTATE_BADPLACE, // Added in 2.0
    THINKSTATE_NOCLIP,
    NUM_THINKSTATES,
};

enum eThinkNum {
    THINK_VOID,
    THINK_TURRET,
    THINK_COVER,
    THINK_PATROL,
    THINK_RUNNER,
    THINK_PAIN,
    THINK_KILLED,
    THINK_MOVETO,
    THINK_IDLE,
    THINK_CURIOUS,
    THINK_DISGUISE_SALUTE,
    THINK_DISGUISE_SENTRY,
    THINK_DISGUISE_OFFICER,
    THINK_DISGUISE_ROVER,
    THINK_DISGUISE_NONE,
    THINK_ALARM,
    THINK_GRENADE,
    THINK_MACHINEGUNNER,
    THINK_DOG_IDLE,
    THINK_DOG_ATTACK,
    THINK_DOG_CURIOUS,
    THINK_DOG_GRENADE,
    THINK_ANIM,
    THINK_ANIM_CURIOUS,
    THINK_AIM,
    THINK_BALCONY_IDLE,
    THINK_BALCONY_CURIOUS,
    THINK_BALCONY_ATTACK,
    THINK_BALCONY_DISGUISE,
    THINK_BALCONY_GRENADE,
    THINK_BALCONY_PAIN,
    THINK_BALCONY_KILLED,
    THINK_WEAPONLESS,
    THINK_NOCLIP,
    THINK_DEAD,
    THINK_BADPLACE,      // Added in 2.0
    THINK_RUN_AND_SHOOT, // Added in 2.30
    NUM_THINKS,
};

enum eThinkLevel {
    THINKLEVEL_IDLE, //I think it should be THINKLEVEL_IDLE
    THINKLEVEL_PAIN,
    THINKLEVEL_KILLED,
    THINKLEVEL_NOCLIP,
    NUM_THINKLEVELS
};

enum eAIEvent {
    AI_EVENT_NONE,
    AI_EVENT_WEAPON_FIRE,
    AI_EVENT_WEAPON_IMPACT,
    AI_EVENT_EXPLOSION,
    AI_EVENT_AMERICAN_VOICE,
    AI_EVENT_GERMAN_VOICE,
    AI_EVENT_AMERICAN_URGENT,
    AI_EVENT_GERMAN_URGENT,
    AI_EVENT_MISC,
    AI_EVENT_MISC_LOUD,
    AI_EVENT_FOOTSTEP,
    AI_EVENT_GRENADE,
    AI_EVENT_BADPLACE // Added in 2.0
};

enum eActorNationality {
    ACTOR_NATIONALITY_DEFAULT,
    ACTOR_NATIONALITY_AMERICAN,
    ACTOR_NATIONALITY_GERMAN,
    ACTOR_NATIONALITY_ITALIAN,
    ACTOR_NATIONALITY_BRITISH,
    ACTOR_NATIONALITY_RUSSIAN,
};

//
// Actor states
//
enum eActorState {
    ACTOR_STATE_DISGUISE       = 0,
    ACTOR_STATE_TURRET         = 100,
    ACTOR_STATE_BALCONY_ATTACK = 200,
    ACTOR_STATE_BALCONY_COVER  = 300,
    ACTOR_STATE_BALCONY_ALARM  = 600,
    ACTOR_STATE_KILLED         = 700,
    ACTOR_STATE_BALCONY_KILLED = 800,
    ACTOR_STATE_WEAPONLESS     = 900,
    ACTOR_STATE_ANIMATION      = 1000,
    ACTOR_STATE_CURIOUS        = 1100,
    ACTOR_STATE_MACHINE_GUNNER = 1200,
    // Added in 2.30
    ACTOR_STATE_RUN_AND_SHOOT = 1300,
};

//
// Disguise think state
//
enum eActorState_Disguise {
    ACTOR_STATE_DISGUISE_START = ACTOR_STATE_DISGUISE,
    ACTOR_STATE_DISGUISE_WAIT  = ACTOR_STATE_DISGUISE_START,
    ACTOR_STATE_DISGUISE_PAPERS,
    ACTOR_STATE_DISGUISE_ACCEPT,
    ACTOR_STATE_DISGUISE_ENEMY,
    ACTOR_STATE_DISGUISE_HALT,
    ACTOR_STATE_DISGUISE_DENY,
};

//
// Turret think state
//
enum eActorState_Turret {
    ACTOR_STATE_TURRET_START = ACTOR_STATE_TURRET,
};

//
// Balcony attack think state
//
enum eActorState_BalconyAttack {
    ACTOR_STATE_BALCONY_ATTACK_START = ACTOR_STATE_BALCONY_ATTACK,
};

//
// Balcony cover think state
//
enum eActorState_BalconyCover {
    ACTOR_STATE_BALCONY_COVER_START = ACTOR_STATE_BALCONY_COVER,
};

//
// Balcony alarm think state
//
enum eActorState_BalconyAlarm {
    ACTOR_STATE_BALCONY_ALARM_START = ACTOR_STATE_BALCONY_ALARM,
};

//
// Killed think state
//
enum eActorState_Killed {
    ACTOR_STATE_KILLED_START = ACTOR_STATE_KILLED,
};

//
// Balcony killed think state
//
enum eActorState_BalconyKilled {
    ACTOR_STATE_BALCONY_KILLED_START = ACTOR_STATE_BALCONY_KILLED,
};

//
// Weaponless think state
//
enum eActorState_WeaponLess {
    ACTOR_STATE_WEAPONLESS_START = ACTOR_STATE_WEAPONLESS,
};

//
// Animation think state
//
enum eActorState_Animation {
    ACTOR_STATE_ANIMATION_START = ACTOR_STATE_ANIMATION,
};

//
// Curious think state
//
enum eActorState_Curious {
    ACTOR_STATE_CURIOUS_START = ACTOR_STATE_CURIOUS,
};

//
// Machine gunner think state
//
enum eActorState_MachineGunner {
    ACTOR_STATE_MACHINE_GUNNER_START = ACTOR_STATE_MACHINE_GUNNER,
};

//
// Run and shoot think state
//
enum eActorState_RunAndShoot {
    ACTOR_STATE_RUN_AND_SHOOT_START = ACTOR_STATE_RUN_AND_SHOOT,
    ACTOR_STATE_RUN_AND_SHOOT_RUN   = ACTOR_STATE_RUN_AND_SHOOT_START,
    ACTOR_STATE_RUN_AND_SHOOT_RUNNING
};

class Actor;
typedef SafePtr<Actor> ActorPtr;

class Actor : public SimpleActor
{
    struct GlobalFuncs_t {
        void (Actor::*ThinkState)(void);
        void (Actor::*BeginState)(void);
        void (Actor::*ResumeState)(void);
        void (Actor::*EndState)(void);
        void (Actor::*SuspendState)(void);
        void (Actor::*RestartState)(void);
        void (Actor::*FinishedAnimation)(void);
        void (Actor::*PostShoot)(void);
        void (Actor::*Pain)(Event *ev);
        void (Actor::*Killed)(Event *ev, bool bPlayDeathAnim);
        bool (Actor::*PassesTransitionConditions)(void);
        void (Actor::*ShowInfo)(void);
        void (Actor::*PathnodeClaimRevoked)(void);
        void (Actor::*ReceiveAIEvent)(
            vec3_t event_origin, int iType, Entity *originator, float fDistSquared, float fMaxDistSquared
        );
        bool (*IsState)(int state);
    };

public:
    /* GlobalFuncs: contains different funcs needed for each actor think 
	 * think is basically a mode for the actor
	 * when m_ThinkLevel changes, new think value is inside m_Think
	 * to access current think : m_Think[m_ThinkLevel]
	 * to access GlobalFuncs related to current think
	 * GlobalFuncs[m_Think[m_ThinkLevel]];
	 **/
    static GlobalFuncs_t GlobalFuncs[NUM_THINKS];
    /* const string array containig think names */
    static const_str m_csThinkNames[NUM_THINKS];
    /* const string array containig think state names */
    static const_str m_csThinkStateNames[NUM_THINKSTATES];
    /* map contating every think value for each thinkstate */
    eThinkNum m_ThinkMap[NUM_THINKSTATES];
    /* think state for every think level */
    eThinkState m_ThinkStates[NUM_THINKLEVELS];
    /* think value for every think level */
    eThinkNum m_Think[NUM_THINKLEVELS];
    /* current think level
	 * think levels are more like priorities
	 * highest level is used.
	 **/
    eThinkLevel m_ThinkLevel;
    /* current think state*/
    eThinkState m_ThinkState;
    /* current state (different than think state) */
    int m_State;
    /* current state change time */
    int m_iStateTime;
    /* should lock think state ? */
    bool m_bLockThinkState;
    /* think state changed */
    bool m_bDirtyThinkState;
    /*  debug state for m_State */
    const char *m_pszDebugState;
    /* currently animating ( used in G_RunFrame ) */
    bool m_bAnimating;
    /* Am I a doggo ? */
    bool m_bDog;
    /* 2.0: ignore bad place? */
    bool m_bIgnoreBadPlace;
    /* 2.0: bad place index? (0=none) */
    int m_iBadPlaceIndex;
    /* char refereing to voice type, chec gAmericanVoices and gGermanVoices */
    int mVoiceType;
    /* check EV_Actor_GetSilent, EV_Actor_SetSilent and EV_Actor_SetSilent2 */
    bool m_bSilent;
    /* check EV_Actor_GetNoSurprise, EV_Actor_SetNoSurprise and EV_Actor_SetNoSurprise2 */
    bool m_bNoSurprise;
    /* actor can mumble ? */
    bool m_bMumble;
    /* actor is allowed to have steamy breath */
    bool m_bBreathSteam;
    /* const string of head model */
    const_str m_csHeadModel;
    /* const string of head skin */
    const_str m_csHeadSkin;
    /* const string of weapon model */
    const_str m_csWeapon;
    /* const string of REAL weapon model (check Actor::EventGiveWeapon) */
    const_str m_csLoadOut;
    /* favorite enemy */
    SentientPtr m_FavoriteEnemy;
    /* last time enemy was checked */
    int m_iEnemyCheckTime;
    /* last time enemy was changed */
    int m_iEnemyChangeTime;
    /* last time a visible(CanSee) enemy was checked */
    int m_iEnemyVisibleCheckTime;
    /* last time a visible(CanSee) enemy was changed */
    int m_iEnemyVisibleChangeTime;
    /* last time a visible(CanSee) enemy was seen */
    int m_iLastEnemyVisibleTime;
    /* 2.0: visibility alpha */
    float m_fVisibilityAlpha;
    /* 2.0: max visibility threshold */
    float m_fVisibilityThreshold;
    /* last time a visible(CanSee + infov) enemy was checked */
    int m_iEnemyFovCheckTime;
    /* last time a visible(CanSee + infov) enemy was changed */
    int m_iEnemyFovChangeTime;
    /* last known enemy position. */
    Vector m_vLastEnemyPos;
    /* last time enemy position was changed. */
    int m_iLastEnemyPosChangeTime;
    /*  check EV_Actor_GetEnemyShareRange and EV_Actor_SetEnemyShareRange */
    float m_fMaxShareDistSquared;
    /* can actor shoot enemy ? */
    bool m_bCanShootEnemy;
    /* 2.0: does it has visibility threshold set? */
    bool m_bHasVisibilityThreshold;
    /* last time m_bCanShootEnemy was changed */
    int m_iCanShootCheckTime;
    /* desired enable enemy(changed from script) */
    bool m_bDesiredEnableEnemy;
    /* enable enemy (change from code only, Actor::UpdateEnableEnemy) */
    bool m_bEnableEnemy;
    /* can take pain ? */
    bool m_bEnablePain;
    /* allow long pain ? */
    bool m_bNoLongPain;
    /* last set enemy is new ? */
    bool m_bNewEnemy;
    /* is enemy disguised ? */
    bool m_bEnemyIsDisguised;
    /* is enemy visible (CanSee) ? */
    bool m_bEnemyVisible;
    /* is enemy in fov (CanSee) ? */
    bool m_bEnemyInFOV;
    /* attack player even if disguised. */
    bool m_bForceAttackPlayer;
    /* actor should avoud player (Actor::IdleThink) (get out of the players way) */
    bool m_bAutoAvoidPlayer;
    /* actor will not go into idle after playing an animation */
    bool m_bNoIdleAfterAnim;
    /* is anim script set ? */
    bool m_bAnimScriptSet;
    /* const string of anim script path */
    const_str m_csAnimScript;
    /* anim mode */
    int m_AnimMode;
    /* 2.0: the run anim rate */
    float m_fRunAnimRate;
    /* Don't Face Wall request yaw. */
    float m_fDfwRequestedYaw;
    /* Don't Face Wall derived yaw. */
    float m_fDfwDerivedYaw;
    /* Don't Face Wall derived position. */
    Vector m_vDfwPos;
    /* Don't Face Wall time. */
    float m_fDfwTime;
    /* last time GunPostiton() was called */
    int m_iGunPositionCheckTime;
    /* gun position */
    Vector m_vGunPosition;
    int    m_iWallDodgeTimeout;
    vec2_t m_PrevObstacleNormal;
    char   m_WallDir;
    /* EV_Actor_SetMoveDoneRadius */
    float m_fMoveDoneRadiusSquared;
    /* last time origin was changed */
    int m_iOriginTime;
    /* should I face enemy ? */
    bool m_bFaceEnemy;
    /* physics on/off ? */
    bool m_bDoPhysics;
    /* should become runner/patrol guy */
    bool m_bBecomeRunner;
    /* If true, patrol guys and running men wait until triggered to move */
    bool   m_bPatrolWaitTrigger;
    bool   m_bScriptGoalValid;
    Vector m_vScriptGoal;
    int    m_iNextWatchStepTime;
    /* current patrol node */
    SafePtr<SimpleEntity> m_patrolCurrentNode;
    /* current patrol anim */
    const_str m_csPatrolCurrentAnim;
    int       m_iSquadStandTime;
    /* distance AI tries to keep between squadmates while moving. */
    float m_fInterval;
    int   m_iIntervalDirTime;
    /* the direction the AI would like to move to maintain its interval */
    Vector m_vIntervalDir;
    short  m_sCurrentPathNodeIndex;
    /* current pain state(similar to m_State) */
    int m_PainState;
    /* last time actor switched to curious state. */
    int m_iCuriousTime;
    /* Current level of curiousity. It's value is from PriorityForEventType() */
    int m_iCuriousLevel;
    int m_iCuriousAnimHint;
    /* next time to check for state change to disguise. PassesTransitionConditions_Disguise() */
    int m_iNextDisguiseTime;
    /* EV_Actor_SetDisguisePeriod */
    int m_iDisguisePeriod;
    /* EV_Actor_SetDisguiseRange */
    float m_fMaxDisguiseDistSquared;
    /* next time enemy should show papers */
    int m_iEnemyShowPapersTime;
    /* the thread for actor when accepting papers */
    ScriptThreadLabel m_DisguiseAcceptThread;
    /* disguise level of the actor, might be 1 or 2 */
    int m_iDisguiseLevel;
    /* node for actor to raise alaram against player. */
    SafePtr<SimpleEntity> m_AlarmNode;
    /* alarm thread for actor */
    ScriptThreadLabel m_AlarmThread;
    /* 2.30: pre-alarm thread for actor */
    ScriptThreadLabel m_PreAlarmThread;
    /* 2.0: Suppress chance */
    int m_iSuppressChance;
    /* used for turret actot to run back to home Turret_SelectState() */
    int m_iRunHomeTime;
    /* no cover path for initial turret state */
    bool m_bTurretNoInitialCover;
    /* potential cover nodes */
    PathNode *m_pPotentialCoverNode[MAX_COVER_NODES];
    /* potential cover node count */
    int m_iPotentialCoverCount;
    /* current cover node */
    PathNode *m_pCoverNode;
    /* special cover node attack script. */
    const_str m_csSpecialAttack;
    /* actor is reloading */
    bool m_bInReload;
    /* actor needs reloading */
    bool m_bNeedReload;
    /* should break(stop) special attack ? */
    bool mbBreakSpecialAttack;
    /* 2.30: is the AI curious? */
    bool m_bIsCurious;
    /* grenade has bounced ? (usually means actor should flee) */
    bool m_bGrenadeBounced;
    /* current grenade */
    SafePtr<Entity> m_pGrenade;
    /* grenade position */
    Vector m_vGrenadePos;
    /* first time grenade was noticed */
    int m_iFirstGrenadeTime;
    /* grenade state */
    eGrenadeState m_eGrenadeState;
    /* grenade mode */
    eGrenadeTossMode m_eGrenadeMode;
    /* grenade velocity */
    Vector m_vGrenadeVel;
    /* grenade kick direction */
    Vector m_vKickDir;
    /* falling path */
    FallPath *m_pFallPath;
    /* minimum height a balcony guy must fall to do special balcony death */
    float m_fBalconyHeight;
    /* actor should not collide with player */
    bool m_bNoPlayerCollision;
    /* multiplier in time to notice an enemy */
    float m_fNoticeTimeScale;
    /* max multiplier in time to notice an enemy */
    float m_fMaxNoticeTimeScale;
    /* set of potential enemies */
    ActorEnemySet m_PotentialEnemies;
    /* vision distance of the actor */
    float m_fSight;
    /* hearing radius of the actor */
    float m_fHearing;
    /* EV_Actor_GetSoundAwareness */
    float m_fSoundAwareness;
    /* EV_Actor_GetGrenadeAwareness */
    float m_fGrenadeAwareness;
    /* mask of AI_EVENT* bits for the actor to ignore. */
    int m_iIgnoreSoundsMask;
    /* fov angle of the actor */
    float m_fFov;
    /* used for infov check */
    float m_fFovDot;
    /* eye update time */
    int m_iEyeUpdateTime;
    /* eye direction */
    Vector m_vEyeDir;
    /* next time to look around */
    int m_iNextLookTime;
    /* fov angle for look around */
    float m_fLookAroundFov;
    /* entity to look at */
    SafePtr<SimpleEntity> m_pLookEntity;
    /* look flags(should be a bool) */
    int m_iLookFlags;
    /* entity to point at */
    SafePtr<SimpleEntity> m_pPointEntity;
    /* entity to turn to */
    SafePtr<SimpleEntity> m_pTurnEntity;
    /* allowed error(difference) in angles after doing turnto command */
    float m_fTurnDoneError;
    /* turn speed of the actor */
    float m_fAngleYawSpeed;
    /* node to aim at */
    SafePtr<SimpleEntity> m_aimNode;
    /* dont face wall mode */
    int m_eDontFaceWallMode;
    int m_iLastFaceDecideTime;
    /*  origin history */
    vec2_t m_vOriginHistory[MAX_ORIGIN_HISTORY];
    /* current origin history index */
    int  m_iCurrentHistory;
    bool m_bHeadAnglesAchieved;
    bool m_bLUpperArmAnglesAchieved;
    bool m_bTorsoAnglesAchieved;
    bool align3;
    /* max head turn speed */
    float m_fHeadMaxTurnSpeed;
    /* desired head angles */
    vec3_t m_vHeadDesiredAngles;
    /* up arm turn speed */
    float m_fLUpperArmTurnSpeed;
    /* upper arm desired angles */
    vec3_t m_vLUpperArmDesiredAngles;
    /* max torso turn speed */
    float m_fTorsoMaxTurnSpeed;
    /* currnet torso turn speed */
    float m_fTorsoCurrentTurnSpeed;
    /* desired torso angles */
    vec3_t m_vTorsoDesiredAngles;
    /* global body queue */
    static SafePtr<Actor> mBodyQueue[MAX_BODYQUEUE];
    /* current body queue index */
    static int mCurBody;
    /* leash home */
    Vector m_vHome;
    /* tether entity */
    SafePtr<SimpleEntity> m_pTetherEnt;
    /* minimum distance actor tries to keep between itself and the player */
    float m_fMinDistance;
    /* square of minimum distance actor tries to keep between itself and the player */
    float m_fMinDistanceSquared;
    /* maximum distance actor tries to allow between itself and the player */
    float m_fMaxDistance;
    /* square of maximum distance actor tries to allow between itself and the player */
    float m_fMaxDistanceSquared;
    /* maximum distance actor will wander from its leash home */
    float m_fLeash;
    /* square of maximum distance actor will wander from its leash home */
    float m_fLeashSquared;
    /* if true, leash will not change. */
    bool m_bFixedLeash;
    /* 2.0: if true, enemy switch will be disabled. */
    bool m_bEnemySwitch;
    /* 2.30: the nationality index. */
    int m_iNationality;

public:
    CLASS_PROTOTYPE(Actor);

protected:
    void MoveTo(Event *ev);
    void WalkTo(Event *ev);
    void RunTo(Event *ev);
    void CrouchTo(Event *ev);
    void CrawlTo(Event *ev);
    void AimAt(Event *ev);
    void DefaultRestart(void);
    void SuspendState(void);
    void ResumeState(void);
    void BeginState(void);
    void EndState(int level);
    void RestartState(void);

public:
    Actor();
    ~Actor();

    virtual void        setContentsSolid(void) override;
    void                InitThinkStates(void);
    void                UpdateEyeOrigin(void);
    bool                RequireThink(void);
    void                UpdateEnemy(int iMaxDirtyTime);
    void                UpdateEnemyInternal(void);
    void                DetectSmokeGrenades(void);
    void                SetEnemy(Sentient *pEnemy, bool bForceConfirmed);
    void                SetEnemyPos(Vector vPos);
    static void         ResetBodyQueue(void);
    void                AddToBodyQue(void);
    Vector              GetAntiBunchPoint(void);
    static void         InitVoid(GlobalFuncs_t *func);
    virtual const char *DumpCallTrace(const char *pszFmt, ...) const override;
    static void         Init(void);
    void                FixAIParameters(void);
    bool                AttackEntryAnimation(void);
    void                CheckForThinkStateTransition(void);
    bool                CheckForTransition(eThinkState state, eThinkLevel level);
    bool                PassesTransitionConditions_Grenade(void);
    bool                PassesTransitionConditions_BadPlace(void); // Added in 2.0
    bool                PassesTransitionConditions_Attack(void);
    bool                PassesTransitionConditions_Disguise(void);
    bool                PassesTransitionConditions_Curious(void);
    bool                PassesTransitionConditions_Idle(void);
    void                UpdateEnableEnemy(void);
    void                ThinkStateTransitions(void);
    void                TransitionState(int iNewState, int iPadTime);
    void                ChangeAnim(void);
    void                UpdateSayAnim(void);
    void                UpdateUpperAnim(void);
    void                UpdateAnim(void);
    virtual void        StoppedWaitFor(const_str name, bool bDeleting) override;
    static void         InitTurret(GlobalFuncs_t *func);
    void                Begin_Turret(void);
    void                End_Turret(void);
    void                Suspend_Turret(void);
    void                Think_Turret(void);
    void                FinishedAnimation_Turret(void);
    void                ReceiveAIEvent_Turret(
                       vec3_t event_origin, int iType, Entity *originator, float fDistSquared, float fMaxDistSquared
                   );
    void          InterruptPoint_Turret(void);
    void          PathnodeClaimRevoked_Turret(void);
    bool          Turret_IsRetargeting(void) const;
    bool          Turret_DecideToSelectState(void);
    void          Turret_SelectState(void);
    bool          Turret_CheckRetarget(void);
    bool          Turret_TryToBecomeCoverGuy(void);
    void          Turret_BeginRetarget(void);
    void          Turret_NextRetarget(void);
    void          Turret_SideStep(int iStepSize, vec3_t vDir);
    void          State_Turret_Combat(void);
    void          State_Turret_Reacquire(void);
    void          State_Turret_TakeSniperNode(void);
    void          State_Turret_SniperNode(void);
    bool          State_Turret_RunHome(bool bAttackOnFail);
    void          State_Turret_RunAway(void);
    void          State_Turret_Charge(void);
    void          State_Turret_Grenade(void);
    void          State_Turret_FakeEnemy(void);
    void          State_Turret_Wait(void);
    void          State_Turret_Shoot(void);             // Added in 2.0
    void          State_Turret_Retarget_Suppress(void); // Added in 2.0
    void          State_Turret_Retarget_Sniper_Node(void);
    void          State_Turret_Retarget_Step_Side_Small(void);
    void          State_Turret_Retarget_Path_Exact(void);
    void          State_Turret_Retarget_Path_Near(void);
    void          State_Turret_Retarget_Step_Side_Medium(void);
    void          State_Turret_Retarget_Step_Side_Large(void);
    void          State_Turret_Retarget_Step_Face_Medium(void);
    void          State_Turret_Retarget_Step_Face_Large(void);
    static void   InitCover(GlobalFuncs_t *func);
    bool          Cover_IsValid(PathNode *node);
    bool          Cover_SetPath(PathNode *node);
    void          Cover_FindCover(bool bCheckAll);
    void          Begin_Cover(void);
    void          End_Cover(void);
    void          Suspend_Cover(void);
    void          Think_Cover(void);
    void          FinishedAnimation_Cover(void);
    void          PathnodeClaimRevoked_Cover(void);
    void          State_Cover_NewEnemy(void);
    void          State_Cover_FindCover(void);
    void          State_Cover_TakeCover(void);
    void          State_Cover_FinishReloading(void);
    void          State_Cover_SpecialAttack(void);
    void          State_Cover_Target(void);
    void          State_Cover_Hide(void);
    void          State_Cover_Shoot(void);
    void          State_Cover_Grenade(void);
    void          State_Cover_FindEnemy(void);
    void          State_Cover_SearchNode(void);
    void          State_Cover_HuntEnemy(void);
    void          State_Cover_FakeEnemy(void);
    static void   InitPatrol(GlobalFuncs_t *func);
    void          Begin_Patrol(void);
    void          End_Patrol(void);
    void          Resume_Patrol(void);
    void          Think_Patrol(void);
    void          ShowInfo_Patrol(void);
    void          IdleThink(void);
    static void   InitRunner(GlobalFuncs_t *func);
    void          Begin_Runner(void);
    void          End_Runner(void);
    void          Resume_Runner(void);
    void          Think_Runner(void);
    void          ShowInfo_Runner(void);
    static void   InitAlarm(GlobalFuncs_t *func);
    void          Begin_Alarm(void);
    void          End_Alarm(void);
    void          State_Alarm_StartThread(void);
    void          State_Alarm_Move(void);
    void          State_Alarm_Idle(void);
    void          Think_Alarm(void);
    void          FinishedAnimation_Alarm(void);
    static void   InitNoClip(GlobalFuncs_t *func);
    bool          IsNoClipState(int state);
    void          Think_NoClip(void);
    bool          ValidGrenadePath(const Vector         &vFrom, const Vector         &vTo, Vector         &vVel);
    static Vector CalcThrowVelocity(const Vector& vFrom, const Vector& vTo);
    Vector        CanThrowGrenade(const Vector       &vFrom, const Vector       &vTo);
    static Vector CalcRollVelocity(const Vector& vFrom, const Vector& vTo);
    Vector        CanRollGrenade(const Vector       &vFrom, const Vector       &vTo);
    bool          CanTossGrenadeThroughHint(
                 GrenadeHint      *pHint,
                 const Vector             &vFrom,
                 const Vector             &vTo,
                 bool              bDesperate,
                 Vector           *pvVel,
                 eGrenadeTossMode *peMode
             );
    static Vector GrenadeThrowPoint(const Vector& vFrom, const Vector& vDelta, const_str csAnim);
    Vector        CalcKickVelocity(Vector       &vDelta, float fDist) const;
    bool          CanKickGrenade(Vector         &vFrom, Vector         &vTo, Vector         &vFace, Vector *pvVel);
    bool          GrenadeWillHurtTeamAt(const Vector         &vTo);
    bool          CanGetGrenadeFromAToB(
                 const Vector         &vFrom, const Vector         &vTo, bool bDesperate, Vector *pvVel, eGrenadeTossMode *peMode
             );
    bool        DecideToThrowGrenade(const Vector       &vTo, Vector *pvVel, eGrenadeTossMode *peMode);
    void        Grenade_EventFire(Event *ev);
    void        GenericGrenadeTossThink(void);
    static void InitGrenade(GlobalFuncs_t *func);
    bool        Grenade_Acquire(eGrenadeState eNextState, const_str csReturnAnim);
    void        Grenade_Flee(void);
    void        Grenade_ThrowAcquire(void);
    void        Grenade_Throw(void);
    void        Grenade_KickAcquire(void);
    void        Grenade_Kick(void);
    void        Grenade_MartyrAcquire(void);
    void        Grenade_Martyr(void);
    void        Grenade_Wait(void);
    void        Grenade_NextThinkState(void);
    void        Grenade_EventAttach(Event *ev);
    void        Grenade_EventDetach(Event *ev);
    void        Begin_Grenade(void);
    void        End_Grenade(void);
    void        Resume_Grenade(void);
    void        Think_Grenade(void);
    void        FinishedAnimation_Grenade(void);
    static void InitCurious(GlobalFuncs_t *func);
    void        SetCuriousAnimHint(int iAnimHint);
    void        Begin_Curious(void);
    void        End_Curious(void);
    void        Resume_Curious(void);
    void        Suspend_Curious(void);
    void        Think_Curious(void);
    void        FinishedAnimation_Curious(void);
    void        LookAtCuriosity(void);
    void        TimeOutCurious(void);
    void        State_Disguise_Wait(void);
    void        State_Disguise_Papers(void);
    void        State_Disguise_Fake_Papers(void);
    void        State_Disguise_Enemy(void);
    void        State_Disguise_Halt(void);
    void        State_Disguise_Accept(void);
    void        State_Disguise_Deny(void);
    static void InitDisguiseSalute(GlobalFuncs_t *func);
    void        Begin_DisguiseSalute(void);
    void        End_DisguiseSalute(void);
    void        Resume_DisguiseSalute(void);
    void        Suspend_DisguiseSalute(void);
    void        Think_DisguiseSalute(void);
    void        FinishedAnimation_DisguiseSalute(void);
    static void InitDisguiseSentry(GlobalFuncs_t *func);
    void        Begin_DisguiseSentry(void);
    void        End_DisguiseSentry(void);
    void        Resume_DisguiseSentry(void);
    void        Suspend_DisguiseSentry(void);
    void        Think_DisguiseSentry(void);
    static void InitDisguiseOfficer(GlobalFuncs_t *func);
    void        Begin_DisguiseOfficer(void);
    void        End_DisguiseOfficer(void);
    void        Resume_DisguiseOfficer(void);
    void        Suspend_DisguiseOfficer(void);
    void        Think_DisguiseOfficer(void);
    static void InitDisguiseRover(GlobalFuncs_t *func);
    void        Begin_DisguiseRover(void);
    void        End_DisguiseRover(void);
    void        Resume_DisguiseRover(void);
    void        Suspend_DisguiseRover(void);
    void        Think_DisguiseRover(void);
    static void InitDisguiseNone(GlobalFuncs_t *func);
    static void InitIdle(GlobalFuncs_t *func);
    void        Begin_Idle(void);
    void        Think_Idle(void);
    static void InitMachineGunner(GlobalFuncs_t *func);
    void        Begin_MachineGunner(void);
    void        End_MachineGunner(void);
    void        BecomeTurretGuy(void);
    void        ThinkHoldGun_TurretGun(void);        // Added in 2.0
    void        Think_MachineGunner_TurretGun(void); // Added in 2.0
    void        Think_MachineGunner(void);
    void        FinishedAnimation_MachineGunner(void);
    bool        MachineGunner_CanSee(Entity *ent, float fov, float vision_distance);
    static void InitDogIdle(GlobalFuncs_t *func);
    static void InitDogAttack(GlobalFuncs_t *func);
    static void InitDogCurious(GlobalFuncs_t *func);
    void        Begin_Dog(void);
    void        End_Dog(void);
    void        Think_Dog_Idle(void);
    void        Think_Dog_Attack(void);
    void        Think_Dog_Curious(void);
    static void InitAnim(GlobalFuncs_t *func);
    void        Begin_Anim(void);
    void        Think_Anim(void);
    void        FinishedAnimation_Anim(void);
    void        ShowInfo_Anim(void);
    static void InitAnimCurious(GlobalFuncs_t *func);
    void        Begin_AnimCurious(void);
    void        Think_AnimCurious(void);
    void        FinishedAnimation_AnimCurious(void);
    static void InitAim(GlobalFuncs_t *func);
    void        Begin_Aim(void);
    void        Think_Aim(void);
    void        ShowInfo_Aim(void);
    static void InitBalconyIdle(GlobalFuncs_t *func);
    static void InitBalconyCurious(GlobalFuncs_t *func);
    static void InitBalconyAttack(GlobalFuncs_t *func);
    static void InitBalconyDisguise(GlobalFuncs_t *func);
    static void InitBalconyGrenade(GlobalFuncs_t *func);
    static void InitBalconyPain(GlobalFuncs_t *func);
    static void InitBalconyKilled(GlobalFuncs_t *func);
    void        Pain_Balcony(Event *ev);
    void        Killed_Balcony(Event *ev, bool bPlayDeathAnim);
    void        Think_BalconyAttack(void);
    void        Begin_BalconyAttack(void);
    void        FinishedAnimation_BalconyAttack(void);
    void        State_Balcony_PostShoot(void);
    void        State_Balcony_FindEnemy(void);
    void        State_Balcony_Target(void);
    void        State_Balcony_Shoot(void);
    void        Begin_BalconyKilled(void);
    void        End_BalconyKilled(void);
    void        Think_BalconyKilled(void);
    void        FinishedAnimation_BalconyKilled(void);
    bool        CalcFallPath(void);
    static void InitPain(GlobalFuncs_t *func);
    void        Begin_Pain(void);
    void        Think_Pain(void);
    void        FinishedAnimation_Pain(void);
    static void InitDead(GlobalFuncs_t *func);
    static void InitKilled(GlobalFuncs_t *func);
    void        Begin_Killed(void);
    void        Think_Killed(void);
    void        FinishedAnimation_Killed(void);
    static void InitWeaponless(GlobalFuncs_t *func);
    void        Begin_Weaponless(void);
    void        Suspend_Weaponless(void);
    void        Think_Weaponless(void);
    void        FinishedAnimation_Weaponless(void);
    void        State_Weaponless_Normal(void);
    void        State_Weaponless_Grenade(void);
    static void InitBadPlace(GlobalFuncs_t *func);
    // Added in 2.0
    //====
    void Begin_BadPlace(void);
    void End_BadPlace(void);
    void Think_BadPlace(void);
    //====
    // Added in 2.30
    //====
    static void InitRunAndShoot(GlobalFuncs_t *func);
    void        Begin_RunAndShoot(void);
    void        End_RunAndShoot(void);
    void        Resume_RunAndShoot(void);
    void        Think_RunAndShoot(void);
    void        ShowInfo_RunAndShoot(void);
    void        State_RunAndShoot_Running(void);
    bool        RunAndShoot_MoveToPatrolCurrentNode(void);
    //====
    virtual void Think(void) override;
    void         PostThink(bool bDontFaceWall);
    virtual void SetMoveInfo(mmove_t *mm) override;
    virtual void GetMoveInfo(mmove_t *mm) override;
    void         DoFailSafeMove(vec3_t dest);
    void         TouchStuff(mmove_t *mm);
    void         ExtractConstraints(mmove_t *mm);
    void         EventGiveWeaponInternal(Event *ev);
    void         EventGiveWeapon(Event *ev);
    void         EventGetWeapon(Event *ev);
    void         FireWeapon(Event *ev);
    bool         FriendlyInLineOfFire(Entity *other); // Added in 2.0
    Vector       VirtualEyePosition();                // Added in 2.0
    virtual bool CanTarget(void) override;
    virtual bool IsImmortal(void) override;
    static bool  IsVoidState(int state);
    static bool  IsIdleState(int state);
    static bool  IsCuriousState(int state);
    static bool  IsDisguiseState(int state);
    static bool  IsAttackState(int state);
    static bool  IsGrenadeState(int state);
    static bool  IsBadPlaceState(int state); // Added in 2.0
    static bool  IsPainState(int state);
    static bool  IsKilledState(int state);
    static bool  IsMachineGunnerState(int state);
    static bool  IsDogState(int state);
    void         IgnoreSoundSet(int iType);
    void         IgnoreSoundSetAll(void);
    void         IgnoreSoundClear(int iType);
    void         IgnoreSoundClearAll(void);
    bool         IgnoreSound(int iType);
    void         EventShareEnemy(Event *ev);
    void         EventShareGrenade(Event *ev);
    void ReceiveAIEvent(vec3_t event_origin, int iType, Entity *originator, float fDistSquared, float fMaxDistSquared);
    void DefaultReceiveAIEvent(
        vec3_t event_origin, int iType, Entity *originator, float fDistSquared, float fMaxDistSquared
    );
    int  PriorityForEventType(int iType);
    void CuriousSound(int iType, vec3_t sound_origin, float fDistSquared, float fMaxDistSquared);
    void WeaponSound(int iType, vec3_t sound_origin, float fDistSquared, float fMaxDistSquared, Entity *originator);
    void FootstepSound(vec3_t sound_origin, float fDistSquared, float fMaxDistSquared, Entity *originator);
    void VoiceSound(int iType, vec3_t sound_origin, float fDistSquared, float fMaxDistSquared, Entity *originator);
    void GrenadeNotification(Entity *originator);
    void SetGrenade(Entity *pGrenade);
    void UpdateBadPlaces(void); // Added in 2.0
    void NotifySquadmateKilled(Sentient *pSquadMate, Sentient *pAttacker);
    void RaiseAlertnessForEventType(int iType);
    void RaiseAlertness(float fAmount);
    virtual bool   CanSee(Entity *e1, float fov, float vision_distance, bool bNoEnts) override;
    virtual Vector GunPosition(void) override;
    bool           WithinVisionDistance(Entity *ent) const;
    bool           InFOV(Vector pos, float check_fov, float check_fovdot);
    bool           EnemyInFOV(int iMaxDirtyTime);
    bool           InFOV(Vector pos);
    bool           InFOV(Entity *ent);
    bool           CanSeeNoFOV(Entity *ent);
    bool           CanSeeFOV(Entity *ent);
    bool           CanSeeEnemyFOV(int iMaxFovDirtyTime, int iMaxSightDirtyTime);
    bool           CanShoot(Entity *ent);
    virtual bool   CanSeeFrom(vec3_t pos, Entity *ent);
    bool           CanSeeEnemy(int iMaxDirtyTime);
    bool           CanShootEnemy(int iMaxDirtyTime);
    void           ShowInfo(void);
    virtual void   ShowInfo(float fDot, float fDist) override;
    void           DefaultPain(Event *ev);
    void           HandlePain(Event *ev);
    void           EventPain(Event *ev);
    void           DefaultKilled(Event *ev, bool bPlayDeathAnim);
    void           HandleKilled(Event *ev, bool bPlayDeathAnim);
    void           DispatchEventKilled(Event *ev, bool bPlayDeathAnim);
    void           EventKilled(Event *ev);
    void           EventBeDead(Event *ev);
    void           DeathEmbalm(Event *ev);
    void           DeathSinkStart(Event *ev);
    bool           NoticeShot(Sentient *pShooter, Sentient *pTarget, float fDist);
    bool           NoticeFootstep(Sentient *pPedestrian);
    bool           NoticeVoice(Sentient *pVocallist);
    void           ClearLookEntity(void);
    void           LookAt(const Vector          &vec);
    void           LookAt(Listener *l);
    void           ForwardLook(void);
    void           LookAtLookEntity(void);
    void           IdleLook(void);
    void           IdleLook(vec3_t dir);
    void           SetDesiredLookDir(vec3_t dir);
    void           SetDesiredLookAnglesRelative(vec3_t ang);
    void           EventLookAt(Event *ev);
    void           EventEyesLookAt(Event *ev);
    void           NoPoint(void);
    void           IdlePoint(void);
    void           ClearPointEntity(void);
    void           PointAt(const Vector          &vec);
    void           PointAt(Listener *l);
    void           EventPointAt(Event *ev);
    void           ClearTurnEntity(void);
    void           TurnTo(const Vector          &vec);
    void           TurnTo(Listener *l);
    void           IdleTurn(void);
    void           EventTurnTo(Event *ev);
    void           EventSetTurnDoneError(Event *ev);
    void           EventGetTurnDoneError(Event *ev);
    void           LookAround(float fFovAdd);
    bool           SoundSayAnim(const_str name, byte bLevelSayAnim);
    void           EventSetAnim(Event *ev);
    void           EventIdleSayAnim(Event *ev);
    void           EventSayAnim(Event *ev);
    void           EventSetSayAnim(Event *ev);
    void           EventSetMotionAnim(Event *ev);
    void           EventSetAimMotionAnim(Event *ev);
    void           EventSetActionAnim(Event *ev);
    void           EventUpperAnim(Event *ev);
    void           EventSetUpperAnim(Event *ev);
    void           EventEndActionAnim(Event *ev);
    void           EventDamagePuff(Event *ev);
    void           SafeSetOrigin(vec3_t newOrigin);
    void           DoMove(void);
    void           AnimFinished(int slot, bool stop);
    virtual void   AnimFinished(int slot) override;
    void           PlayAnimation(Event *ev);
    void           PlayScriptedAnimation(Event *ev);
    void           PlayNoclipAnimation(Event *ev);
    void           PlayAttachedAnimation(Event *ev); // Added in 2.0
    void           MoveDest(float fMoveSpeed);
    void           MovePath(float fMoveSpeed);
    void           MovePathGoal(float fMoveSpeed);
    void           Dumb(Event *ev);
    void           PhysicsOn(Event *ev);
    void           PhysicsOff(Event *ev);
    void           EventStart(Event *ev);
    void           EventGetMood(Event *ev);
    void           EventSetMood(Event *ev);
    void           EventGetAngleYawSpeed(Event *ev);
    void           EventSetAngleYawSpeed(Event *ev);
    void           EventSetAimTarget(Event *ev);
    void           UpdateAngles(void);
    void           SetLeashHome(Vector vHome);
    void           AimAtTargetPos(void);
    void           AimAtAimNode(void);
    void           AimAtEnemyBehavior(void);
    void           FaceMotion(void);
    void           FaceDirectionDuringMotion(vec3_t vLook);
    float          PathDistanceAlongVector(vec3_t vDir);
    void           FaceEnemyOrMotion(int iTimeIntoMove);
    static int     NextUpdateTime(int iLastUpdateTime, int iUpdatePeriod);
    void           ResetBoneControllers(void);
    void           UpdateBoneControllers(void);
    void           ReadyToFire(Event *ev);
    void           EventGetSight(Event *ev);
    void           EventSetSight(Event *ev);
    void           EventGetHearing(Event *ev);
    void           EventSetHearing(Event *ev);
    void           ClearPatrolCurrentNode(void);
    void           NextPatrolCurrentNode(void);
    void           SetPatrolCurrentNode(Vector          &vec);
    void           SetPatrolCurrentNode(Listener *l);
    void           EventSetPatrolPath(Event *ev);
    void           EventGetPatrolPath(Event *ev);
    void           EventSetPatrolWaitTrigger(Event *ev);
    void           EventGetPatrolWaitTrigger(Event *ev);
    void           ShowInfo_PatrolCurrentNode(void);
    bool           MoveOnPathWithSquad(void);
    bool           MoveToWaypointWithPlayer(void);
    bool           PatrolNextNodeExists(void);
    void           UpdatePatrolCurrentNode(void);
    bool           MoveToPatrolCurrentNode(void);
    void           ClearAimNode(void);
    void           SetAimNode(const Vector          &vec);
    void           SetAimNode(Listener *l);
    void           ShowInfo_AimNode(void);
    void           EventSetAccuracy(Event *ev);
    void           EventGetAccuracy(Event *ev);
    int            GetThinkType(const_str csName);
    void           SetThink(eThinkState state, eThinkNum think);
    void           SetThinkIdle(eThinkNum think_idle);
    void           SetThinkState(eThinkState state, eThinkLevel level);
    void           EndCurrentThinkState(void);
    void           ClearThinkStates(void);
    int            CurrentThink(void) const;
    bool           IsAttacking(void) const;
    void           EventGetFov(Event *ev);
    void           EventSetFov(Event *ev);
    void           EventSetDestIdle(Event *ev);
    void           EventSetDestIdle2(Event *ev);
    void           EventSetTypeIdle(Event *ev);
    void           EventGetTypeIdle(Event *ev);
    void           EventSetTypeAttack(Event *ev);
    void           EventGetTypeAttack(Event *ev);
    void           EventSetTypeDisguise(Event *ev);
    void           EventGetTypeDisguise(Event *ev);
    void           EventSetDisguiseLevel(Event *ev);
    void           EventGetDisguiseLevel(Event *ev);
    void           EventSetTypeGrenade(Event *ev);
    void           EventGetTypeGrenade(Event *ev);
    void           EventSetMinDistance(Event *ev);
    void           EventGetMinDistance(Event *ev);
    void           EventSetMaxDistance(Event *ev);
    void           EventGetMaxDistance(Event *ev);
    void           EventGetLeash(Event *ev);
    void           EventSetLeash(Event *ev);
    void           EventGetInterval(Event *ev);
    void           EventSetInterval(Event *ev);
    void           EventDistToEnemy(Event *ev);
    void           EventGetRunAnim(Event *ev);
    void           EventGetWalkAnim(Event *ev);
    void           EventGetAnimName(Event *ev);
    void           EventSetAnimName(Event *ev);
    void           EventSetDisguiseRange(Event *ev);
    void           EventGetDisguiseRange(Event *ev);
    void           EventSetDisguisePeriod(Event *ev);
    void           EventGetDisguisePeriod(Event *ev);
    void           EventSetDisguiseAcceptThread(Event *ev);
    void           EventGetDisguiseAcceptThread(Event *ev);
    void           EventAttackPlayer(Event *ev);
    void           ForceAttackPlayer(void);
    void           EventSetAlarmNode(Event *ev);
    void           EventGetAlarmNode(Event *ev);
    void           EventSetPreAlarmThread(Event *ev); // Added in 2.30
    void           EventSetAlarmThread(Event *ev);
    void           EventGetAlarmThread(Event *ev);
    void           EventSetSoundAwareness(Event *ev);
    void           EventGetSoundAwareness(Event *ev);
    void           EventSetGrenadeAwareness(Event *ev);
    void           EventGetGrenadeAwareness(Event *ev);
    str            ThinkName(void) const;
    str            ThinkStateName(void) const;
    void           EventSetTurret(Event *ev);
    void           EventGetTurret(Event *ev);
    void           EventEnableEnemy(Event *ev);
    void           EventEnablePain(Event *ev);
    void           EventActivate(Event *ev);
    void           EventGetAmmoGrenade(Event *ev);
    void           EventSetAmmoGrenade(Event *ev);
    void           EventInterruptPoint(Event *ev);
    // Added in 2.0
    //====
    void EventGetVisibilityThreshold(Event *ev);
    void EventSetVisibilityThreshold(Event *ev);
    void EventSetDefaultVisibilityThreshold(Event *ev);
    void EventGetSuppressChance(Event *ev);
    void EventSetSuppressChance(Event *ev);
    //====
    void             EventAnimScript(Event *ev);
    void             EventAnimScript_Scripted(Event *ev);
    void             EventAnimScript_Noclip(Event *ev);
    void             EventAnimScript_Attached(Event *ev);
    void             EventReload_mg42(Event *ev);
    void             SetPathWithLeash(Vector vDestPos, const char *description, int iMaxDirtyTime);
    void             SetPathWithLeash(SimpleEntity *pDestNode, const char *description, int iMaxDirtyTime);
    void             FindPathAwayWithLeash(vec3_t vAwayFrom, vec3_t vDirPreferred, float fMinSafeDist);
    void             FindPathNearWithLeash(vec3_t vNearbyTo, float fCloseDistSquared);
    bool             CanMovePathWithLeash(void) const;
    bool             MovePathWithLeash(void);
    bool             ShortenPathToAttack(float fMinDist);
    void             StrafeToAttack(float fDist, vec3_t vDir);
    virtual Vector   GunTarget(bool bNoCollision) override;
    virtual qboolean setModel(void) override;
    void             EventSetHeadModel(Event *ev);
    void             EventGetHeadModel(Event *ev);
    void             EventSetHeadSkin(Event *ev);
    void             EventGetHeadSkin(Event *ev);
    void             EventSetNoIdle(Event *ev);
    void             EventGetNoIdle(Event *ev);
    void             EventGetEnemy(Event *ev);
    void             EventSetMaxNoticeTimeScale(Event *ev);
    void             EventGetMaxNoticeTimeScale(Event *ev);
    void             EventSetFixedLeash(Event *ev);
    void             EventGetFixedLeash(Event *ev);
    void             Holster(void);
    void             HolsterOffHand(void); // Added in 2.0
    void             Unholster(void);
    void             UnholsterOffHand(void); // Added in 2.0
    void             EventHolster(Event *ev);
    void             EventUnholster(Event *ev);
    void             EventSoundDone(Event *ev);
    void             EventSound(Event *ev);
    void             EventIsEnemyVisible(Event *ev);
    void             EventGetEnemyVisibleChangeTime(Event *ev);
    void             EventGetLastEnemyVisibleTime(Event *ev);
    void             EventSetFallHeight(Event *ev);
    void             EventGetFallHeight(Event *ev);
    void             EventCanMoveTo(Event *ev);
    void             EventMoveDir(Event *ev);
    void             EventIntervalDir(Event *ev);
    void             EventResetLeash(Event *ev);
    void             EventTether(Event *ev);
    void             EventGetThinkState(Event *ev);
    void             EventGetEnemyShareRange(Event *ev);
    void             EventSetEnemyShareRange(Event *ev);
    void             EventGetKickDir(Event *ev);
    void             EventGetNoLongPain(Event *ev);
    void             EventSetNoLongPain(Event *ev);
    void             EventGetFavoriteEnemy(Event *ev);
    void             EventSetFavoriteEnemy(Event *ev);
    void             EventFindEnemy(Event *ev); // Added in 2.0
    void             EventGetMumble(Event *ev);
    void             EventSetMumble(Event *ev);
    void             EventGetBreathSteam(Event *ev);
    void             EventSetBreathSteam(Event *ev);
    void             EventSetNextBreathTime(Event *ev);
    void             EventCalcGrenadeToss2(Event *ev);
    void             EventCalcGrenadeToss(Event *ev);
    void             EventGetNoSurprise(Event *ev);
    void             EventSetNoSurprise(Event *ev);
    void             EventGetSilent(Event *ev);
    void             EventSetSilent(Event *ev);
    void             EventGetAvoidPlayer(Event *ev);
    void             EventSetAvoidPlayer(Event *ev);
    void             EventGetLookAroundAngle(Event *ev);
    void             EventSetLookAroundAngle(Event *ev);
    void             EventHasCompleteLookahead(Event *ev);
    void             EventPathDist(Event *ev);
    void             EventCanShootEnemyFrom(Event *ev);
    void             EventCanShoot(Event *ev);
    void             EventSetInReload(Event *ev);
    void             EventGetInReload(Event *ev);
    void             EventSetReloadCover(Event *ev);
    void             EventBreakSpecial(Event *ev);
    void             GetVoiceType(Event *ev);
    void             SetVoiceType(Event *ev);
    void             ResolveVoiceType(void);
    void             EventSetBalconyHeight(Event *ev);
    void             EventGetBalconyHeight(Event *ev);
    // Added in 2.0
    //====
    void EventSetIgnoreBadPlace(Event *ev);
    void EventGetIgnoreBadPlace(Event *ev);
    void EventEnableEnemySwitch(Event *ev);
    void EventDisableEnemySwitch(Event *ev);
    void EventSetRunAnimRate(Event *ev);
    void EventGetRunAnimRate(Event *ev);
    void Landed(Event *ev);
    bool IsOnFloor(void);
    //====
    // Added in 2.30
    //====
    void GetNationality(Event *ev);
    void SetNationality(Event *ev);
    void EventWriteStats(Event *ev);
    void EventCuriousOff(Event *ev);
    void EventCuriousOn(Event *ev);
    //====
    PathNode    *FindSniperNodeAndSetPath(bool *pbTryAgain);
    void         Remove(Event *ev);
    void         DontFaceWall(void);
    bool         AvoidingFacingWall(void) const;
    void         EndStates(void);
    void         ClearStates(void);
    void         CheckUnregister(void);
    void         BecomeCorpse(void);
    virtual void PathnodeClaimRevoked(PathNode *node) override;
    void         SetPathToNotBlockSentient(Sentient *pOther);
    void         EventSetMoveDoneRadius(Event *ev);
    void         EventGetMoveDoneRadius(Event *ev); // Added in 2.0
    virtual void ClearEnemies(void) override;
    bool         EnemyIsDisguised(void);
    virtual void setOriginEvent(Vector org) override;
    virtual void DumpAnimInfo(void) override;
    static void  ArchiveStatic(Archiver &arc);
    virtual void Archive(Archiver& arc) override;
    virtual bool AutoArchiveModel(void) override;
    virtual bool IsDisabled() const override; // Added in 2.30
};

#define SAVE_FLAG_NEW_ANIM        (1 << 0)
#define SAVE_FLAG_FORWARD_SPEED   (1 << 1)
#define SAVE_FLAG_BEHAVIOR        (1 << 2)
#define SAVE_FLAG_PATH            (1 << 3)
#define SAVE_FLAG_NOISE           (1 << 4)
#define SAVE_FLAG_SCRIPT_THREAD   (1 << 5)
#define SAVE_FLAG_ACTOR_THREAD    (1 << 6)
#define SAVE_FLAG_KILL_THREAD     (1 << 7)
#define SAVE_FLAG_STATE           (1 << 8)
#define SAVE_FLAG_IDLE_THREAD     (1 << 7)
#define SAVE_FLAG_PARTS           (1 << 10)
#define SAVE_FLAG_TRIGGER         (1 << 11)
#define SAVE_FLAG_STATE_FLAGS     (1 << 12)
#define SAVE_FLAG_COMMAND         (1 << 13)
#define SAVE_FLAG_STAGE           (1 << 14)
#define SAVE_FLAG_NUM_OF_SPAWNS   (1 << 15)
#define SAVE_FLAG_SPAWN_PARENT    (1 << 16)
#define SAVE_FLAG_DIALOG          (1 << 17)
#define SAVE_FLAG_SAVED_STUFF     (1 << 18)
#define SAVE_FLAG_LAST_ANIM_EVENT (1 << 19)
#define SAVE_FLAG_PICKUP_ENT      (1 << 20)
#define SAVE_FLAG_PAIN            (1 << 21)
#define SAVE_FLAG_SPAWN_ITEMS     (1 << 22)

/*
===============
Actor::TransitionState

===============
*/
inline void Actor::TransitionState(int iNewState, int iPadTime)
{
    m_State      = iNewState;
    m_iStateTime = level.inttime + iPadTime;
}

/*
===============
Actor::NextUpdateTime

Returns next update time.
===============
*/
inline int Actor::NextUpdateTime(int iLastUpdateTime, int iUpdatePeriod)
{
    int i = iLastUpdateTime;

    do {
        i += iUpdatePeriod;
    } while (i < level.inttime);

    return i;
}

/*
===============
Actor::InFOV

Returns true if pos is within fov.
===============
*/
inline bool Actor::InFOV(Vector pos)
{
    return InFOV(pos, m_fFov, m_fFovDot);
}

/*
===============
Actor::InFOV

Returns true if ent is within fov.
===============
*/
inline bool Actor::InFOV(Entity *ent)
{
    if (ent == m_Enemy) {
        return EnemyInFOV(0);
    } else {
        return InFOV(ent->centroid, m_fFov, m_fFovDot);
    }
}

inline bool Actor::CanSeeNoFOV(Entity *ent)
{
    if (ent == m_Enemy) {
        return CanSeeEnemy(0);
    } else {
        bool bCanSee = false;
        if (gi.AreasConnected(edict->r.areanum, ent->edict->r.areanum)) {
            bCanSee = CanSeeFrom(EyePosition(), ent);
        }
        return bCanSee;
    }
}

inline bool Actor::CanSeeFOV(Entity *ent)
{
    //fixme: this is an inline function.
    if (ent == m_Enemy) {
        return CanSeeEnemyFOV(0, 0);
    } else {
        bool bCanSee = false;
        if (InFOV(ent->centroid, m_fFov, m_fFovDot) && gi.AreasConnected(edict->r.areanum, ent->edict->r.areanum)) {
            bCanSee = CanSeeFrom(EyePosition(), ent);
        }
        return bCanSee;
    }
}

inline bool Actor::CanSeeEnemyFOV(int iMaxFovDirtyTime, int iMaxSightDirtyTime)
{
    return EnemyInFOV(iMaxFovDirtyTime) && CanSeeEnemy(iMaxSightDirtyTime);
}

/*
===============
Actor::AimAtTargetPos

Aim at mTargetPos.
===============
*/
inline void Actor::AimAtTargetPos(void)
{
    Vector vDir = mTargetPos - EyePosition() + Vector(0, 0, 16);
    SetDesiredLookDir(vDir);
    m_DesiredGunDir[0] = 360.0f - vDir.toPitch();
    m_DesiredGunDir[1] = vDir.toYaw();
    m_DesiredGunDir[2] = 0;

    SetDesiredYawDir(vDir);
}

/*
===============
Actor::AimAtAimNode

Aim at m_aimNode.
===============
*/
inline void Actor::AimAtAimNode(void)
{
    mTargetPos = m_aimNode->origin;

    AimAtTargetPos();
}

/*
===============
Actor::IgnoreSoundSet

Make actor ignore iType sound.
===============
*/
inline void Actor::IgnoreSoundSet(int iType)
{
    m_iIgnoreSoundsMask |= 1 << iType;
}

/*
===============
Actor::IgnoreSoundSetAll

Make actor ignore all types of sound.
===============
*/
inline void Actor::IgnoreSoundSetAll(void)
{
    m_iIgnoreSoundsMask = ~AI_EVENT_NONE;
}

/*
===============
Actor::IgnoreSoundClear

Don't ignore iType of sound.
===============
*/
inline void Actor::IgnoreSoundClear(int iType)
{
    m_iIgnoreSoundsMask &= ~iType;
}

/*
===============
Actor::IgnoreSoundClearAll

Make actor ignore no type of sound.
===============
*/
inline void Actor::IgnoreSoundClearAll(void)
{
    m_iIgnoreSoundsMask = 0;
}

/*
===============
Actor::IgnoreSoundClearAll

returns true if actor should ignore iType of sound.
===============
*/
inline bool Actor::IgnoreSound(int iType)
{
    return (m_iIgnoreSoundsMask >> iType) & 1;
}

/*
===============
Actor::CurrentThink

Current think.
===============
*/
inline int Actor::CurrentThink(void) const
{
    return m_Think[m_ThinkLevel];
}

/*
===============
Actor::IsAttacking

Returns true if actor is in attack state.
===============
*/
inline bool Actor::IsAttacking(void) const
{
    return m_ThinkStates[THINKLEVEL_IDLE] == THINKSTATE_ATTACK;
}

inline void Actor::ArchiveStatic(Archiver& arc)
{
    for (int i = MAX_BODYQUEUE - 1; i >= 0; i--) {
        arc.ArchiveSafePointer(&mBodyQueue[i]);
    }

    arc.ArchiveInteger(&mCurBody);
}

inline void Actor::Archive(Archiver& arc)
{
    int i;

    SimpleActor::Archive(arc);

    for (i = 0; i < NUM_THINKSTATES; i++) {
        ArchiveEnum(m_ThinkMap[i], eThinkNum);
    }

    for (i = 0; i < NUM_THINKLEVELS; i++) {
        ArchiveEnum(m_ThinkStates[i], eThinkState);
        ArchiveEnum(m_Think[i], eThinkNum);
    }

    ArchiveEnum(m_ThinkLevel, eThinkLevel);
    ArchiveEnum(m_ThinkState, eThinkState);
    arc.ArchiveInteger(&m_State);
    arc.ArchiveInteger(&m_iStateTime);
    arc.ArchiveBool(&m_bLockThinkState);
    arc.ArchiveBool(&m_bDirtyThinkState);

    arc.ArchiveBool(&m_bAnimating);

    arc.ArchiveBool(&m_bIgnoreBadPlace);
    arc.ArchiveInteger(&m_iBadPlaceIndex);

    arc.ArchiveInteger(&mVoiceType);
    arc.ArchiveBool(&m_bSilent);

    arc.ArchiveBool(&m_bNoSurprise);
    arc.ArchiveBool(&m_bMumble);
    arc.ArchiveBool(&m_bBreathSteam);
    arc.ArchiveBool(&m_bIsCurious);

    Director.ArchiveString(arc, m_csHeadModel);
    Director.ArchiveString(arc, m_csHeadSkin);
    Director.ArchiveString(arc, m_csWeapon);
    Director.ArchiveString(arc, m_csLoadOut);

    arc.ArchiveSafePointer(&m_FavoriteEnemy);
    arc.ArchiveInteger(&m_iEnemyCheckTime);
    arc.ArchiveInteger(&m_iEnemyChangeTime);
    arc.ArchiveInteger(&m_iEnemyVisibleCheckTime);
    arc.ArchiveInteger(&m_iEnemyVisibleChangeTime);
    arc.ArchiveInteger(&m_iLastEnemyVisibleTime);
    arc.ArchiveFloat(&m_fVisibilityAlpha);
    arc.ArchiveFloat(&m_fVisibilityThreshold);
    arc.ArchiveInteger(&m_iEnemyFovCheckTime);
    arc.ArchiveInteger(&m_iEnemyFovChangeTime);
    arc.ArchiveVector(&m_vLastEnemyPos);
    arc.ArchiveInteger(&m_iLastEnemyPosChangeTime);
    arc.ArchiveFloat(&m_fMaxShareDistSquared);
    arc.ArchiveInteger(&m_iCanShootCheckTime);
    arc.ArchiveBool(&m_bCanShootEnemy);
    arc.ArchiveBool(&m_bHasVisibilityThreshold);
    arc.ArchiveBool(&m_bDesiredEnableEnemy);
    arc.ArchiveBool(&m_bEnableEnemy);
    arc.ArchiveBool(&m_bEnablePain);
    arc.ArchiveBool(&m_bNoLongPain);
    arc.ArchiveBool(&m_bNewEnemy);
    arc.ArchiveBool(&m_bEnemyIsDisguised);
    arc.ArchiveBool(&m_bEnemyVisible);
    arc.ArchiveBool(&m_bEnemyInFOV);
    arc.ArchiveBool(&m_bForceAttackPlayer);
    arc.ArchiveBool(&m_bAutoAvoidPlayer);

    arc.ArchiveBool(&m_bNoIdleAfterAnim);
    Director.ArchiveString(arc, m_csAnimScript);
    arc.ArchiveBool(&m_bAnimScriptSet);
    arc.ArchiveInteger(&m_AnimMode);
    arc.ArchiveFloat(&m_fRunAnimRate);
    arc.ArchiveFloat(&m_fDfwRequestedYaw);
    arc.ArchiveFloat(&m_fDfwDerivedYaw);
    arc.ArchiveVector(&m_vDfwPos);
    arc.ArchiveFloat(&m_fDfwTime);
    arc.ArchiveInteger(&m_iGunPositionCheckTime);
    arc.ArchiveVector(&m_vGunPosition);

    arc.ArchiveInteger(&m_iWallDodgeTimeout);
    arc.ArchiveFloat(&m_PrevObstacleNormal[0]);
    arc.ArchiveFloat(&m_PrevObstacleNormal[1]);
    arc.ArchiveChar(&m_WallDir);
    arc.ArchiveFloat(&m_fMoveDoneRadiusSquared);

    arc.ArchiveBool(&m_bFaceEnemy);
    arc.ArchiveBool(&m_bDoPhysics);
    arc.ArchiveBool(&m_bPatrolWaitTrigger);
    arc.ArchiveBool(&m_bBecomeRunner);
    arc.ArchiveBool(&m_bScriptGoalValid);
    arc.ArchiveVector(&m_vScriptGoal);
    arc.ArchiveInteger(&m_iNextWatchStepTime);

    arc.ArchiveSafePointer(&m_patrolCurrentNode);
    Director.ArchiveString(arc, m_csPatrolCurrentAnim);
    arc.ArchiveInteger(&m_iSquadStandTime);
    arc.ArchiveFloat(&m_fInterval);
    arc.ArchiveInteger(&m_iIntervalDirTime);
    arc.ArchiveVector(&m_vIntervalDir);
    arc.ArchiveShort(&m_sCurrentPathNodeIndex);
    arc.ArchiveInteger(&m_PainState);

    arc.ArchiveInteger(&m_iCuriousTime);
    arc.ArchiveInteger(&m_iCuriousLevel);
    arc.ArchiveInteger(&m_iCuriousAnimHint);
    arc.ArchiveInteger(&m_iNextDisguiseTime);
    arc.ArchiveInteger(&m_iDisguisePeriod);
    arc.ArchiveFloat(&m_fMaxDisguiseDistSquared);
    arc.ArchiveInteger(&m_iEnemyShowPapersTime);

    m_DisguiseAcceptThread.Archive(arc);
    arc.ArchiveInteger(&m_iDisguiseLevel);
    arc.ArchiveSafePointer(&m_AlarmNode);
    m_AlarmThread.Archive(arc);
    m_PreAlarmThread.Archive(arc);

    arc.ArchiveInteger(&m_iSuppressChance);
    arc.ArchiveInteger(&m_iRunHomeTime);
    arc.ArchiveBool(&m_bTurretNoInitialCover);
    arc.ArchiveInteger(&m_iPotentialCoverCount);

    for (i = 0; i < MAX_COVER_NODES; i++) {
        arc.ArchiveObjectPointer((Class **)&m_pPotentialCoverNode[i]);
    }

    arc.ArchiveObjectPointer((Class **)&m_pCoverNode);

    Director.ArchiveString(arc, m_csSpecialAttack);
    arc.ArchiveBool(&m_bInReload);
    arc.ArchiveBool(&m_bNeedReload);
    arc.ArchiveBool(&mbBreakSpecialAttack);

    arc.ArchiveBool(&m_bGrenadeBounced);
    arc.ArchiveSafePointer(&m_pGrenade);
    arc.ArchiveVector(&m_vGrenadePos);
    arc.ArchiveInteger(&m_iFirstGrenadeTime);
    ArchiveEnum(m_eGrenadeState, eGrenadeState);
    ArchiveEnum(m_eGrenadeMode, eGrenadeTossMode);
    arc.ArchiveVector(&m_vGrenadeVel);
    arc.ArchiveVector(&m_vKickDir);

    arc.ArchiveFloat(&m_fNoticeTimeScale);
    arc.ArchiveFloat(&m_fMaxNoticeTimeScale);
    m_PotentialEnemies.Archive(arc);

    arc.ArchiveFloat(&m_fSight);
    arc.ArchiveFloat(&m_fHearing);
    arc.ArchiveFloat(&m_fSoundAwareness);
    arc.ArchiveFloat(&m_fGrenadeAwareness);
    arc.ArchiveInteger(&m_iIgnoreSoundsMask);

    arc.ArchiveFloat(&m_fFov);
    arc.ArchiveFloat(&m_fFovDot);
    arc.ArchiveInteger(&m_iEyeUpdateTime);
    arc.ArchiveVector(&m_vEyeDir);
    arc.ArchiveInteger(&m_iNextLookTime);
    arc.ArchiveFloat(&m_fLookAroundFov);
    arc.ArchiveSafePointer(&m_pLookEntity);
    arc.ArchiveInteger(&m_iLookFlags);
    arc.ArchiveSafePointer(&m_pPointEntity);
    arc.ArchiveSafePointer(&m_pTurnEntity);
    arc.ArchiveFloat(&m_fTurnDoneError);
    arc.ArchiveFloat(&m_fAngleYawSpeed);
    arc.ArchiveSafePointer(&m_aimNode);
    arc.ArchiveInteger(&m_eDontFaceWallMode);
    arc.ArchiveInteger(&m_iLastFaceDecideTime);

    arc.ArchiveBool(&m_bHeadAnglesAchieved);
    arc.ArchiveBool(&m_bLUpperArmAnglesAchieved);
    arc.ArchiveBool(&m_bTorsoAnglesAchieved);
    arc.ArchiveFloat(&m_fHeadMaxTurnSpeed);
    arc.ArchiveVec3(m_vHeadDesiredAngles);
    arc.ArchiveFloat(&m_fLUpperArmTurnSpeed);
    arc.ArchiveVec3(m_vLUpperArmDesiredAngles);
    arc.ArchiveFloat(&m_fTorsoMaxTurnSpeed);
    arc.ArchiveFloat(&m_fTorsoCurrentTurnSpeed);
    arc.ArchiveVec3(m_vTorsoDesiredAngles);

    arc.ArchiveVector(&m_vHome);
    arc.ArchiveSafePointer(&m_pTetherEnt);

    arc.ArchiveFloat(&m_fMinDistance);
    arc.ArchiveFloat(&m_fMinDistanceSquared);
    arc.ArchiveFloat(&m_fMaxDistance);
    arc.ArchiveFloat(&m_fMaxDistanceSquared);

    arc.ArchiveFloat(&m_fLeash);
    arc.ArchiveFloat(&m_fLeashSquared);
    arc.ArchiveBool(&m_bFixedLeash);

    byte length;

    if (arc.Saving()) {
        if (m_pFallPath) {
            length = m_pFallPath->length;
        } else {
            length = 0;
        }
    }
    arc.ArchiveByte(&length);

    if (arc.Loading() && length) {
        m_pFallPath =
            (FallPath *)gi.Malloc((sizeof(FallPath::pos)) * length + (sizeof(FallPath) - sizeof(FallPath::pos)));
        m_pFallPath->length = length;
    }

    if (length) {
        arc.ArchiveByte(&m_pFallPath->currentPos);
        arc.ArchiveByte(&m_pFallPath->loop);

        for (i = 0; i < length; i++) {
            arc.ArchiveVec3(m_pFallPath->pos[i]);
        }
    }

    arc.ArchiveFloat(&m_fBalconyHeight);
    arc.ArchiveBool(&m_bNoPlayerCollision);

    for (i = 0; i < MAX_ORIGIN_HISTORY; i++) {
        arc.ArchiveVec2(m_vOriginHistory[i]);
    }

    arc.ArchiveInteger(&m_iCurrentHistory);

    arc.ArchiveBool(&m_bEnemySwitch);
    arc.ArchiveInteger(&m_iNationality);

    // set the model
    setModel();
}

/*
===============
Actor::SetDesiredLookDir

Change desired look direction.
===============
*/
inline void Actor::SetDesiredLookDir(vec3_t dir)
{
    m_bHasDesiredLookAngles = true;
    vectoangles(dir, m_DesiredLookAngles);
    m_DesiredLookAngles[1] = m_DesiredLookAngles[1] - angles[1];
    m_DesiredLookAngles[1] = AngleNormalize180(m_DesiredLookAngles[1]);
    m_DesiredLookAngles[0] = AngleNormalize180(m_DesiredLookAngles[0]);
}

/*
===============
Actor::SetDesiredLookAnglesRelative

Change desired look angles relatively.
===============
*/
inline void Actor::SetDesiredLookAnglesRelative(vec3_t ang)
{
    m_bHasDesiredLookAngles = true;
    m_DesiredLookAngles[0]  = AngleNormalize180(ang[0]);
    m_DesiredLookAngles[1]  = AngleNormalize180(ang[1]);
    m_DesiredLookAngles[2]  = AngleNormalize180(ang[2]);
}

/*
===============
Actor::ForwardLook


===============
*/
inline void Actor::ForwardLook(void)
{
    m_bHasDesiredLookAngles = false;
}

inline bool Actor::AvoidingFacingWall(void) const
{
    return m_eDontFaceWallMode <= 8;
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

/*void AI_TargetPlayer( void );

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
*/
