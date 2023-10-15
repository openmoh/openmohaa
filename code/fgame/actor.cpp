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

// actor.cpp: Base class for character AI.

#include "g_local.h"
#include "actor.h"
#include "scriptthread.h"
#include "scriptclass.h"
#include "doors.h"
#include "gibs.h"
#include "misc.h"
#include "specialfx.h"
#include "object.h"
#include "scriptslave.h"
#include "explosion.h"
#include "misc.h"
#include "playerstart.h"
#include "characterstate.h"
#include "weaputils.h"
#include "player.h"
#include "bg_local.h"
#include "weapturret.h"
#include "sentient.h"
#include "g_phys.h"
#include "debuglines.h"
#include "scriptexception.h"
#include "parm.h"
#include "../qcommon/tiki.h"

#include <cmath>

extern Vector PLAYER_BASE_MIN;
extern Vector PLAYER_BASE_MAX;

bool bEntinfoInit = false;

cvar_t *g_showinfo;

const char *gAmericanVoices[] = {"a", "c", "h"};
const char *gGermanVoices[]   = {"a", "c", "d"};

static const float DEFAULT_NEARBY_SQUAD_DIST = 1024;
static const float MIN_BADPLACE_UPDATE_DIST = 256;

Event EV_Actor_SetGun
(
    "gun",
    EV_DEFAULT,
    "s",
    "s",
    "specifies the gun to use",
    EV_NORMAL
);
Event EV_Actor_SetGun2
(
    "gun",
    EV_DEFAULT,
    "s",
    "s",
    "specifies the gun to use",
    EV_SETTER
);
Event EV_Actor_GetGun
(
    "gun",
    EV_DEFAULT,
    NULL,
    NULL,
    "gets the gun to being used",
    EV_GETTER
);
Event EV_Actor_WeaponInternal
(
    "weapon_internal",
    EV_DEFAULT,
    "s",
    "s",
    "internal use",
    EV_NORMAL
);
Event EV_Actor_TestAnim
(
    "testanim",
    EV_DEFAULT,
    NULL,
    NULL,
    "",
    EV_NORMAL
);
Event EV_Actor_DistToEnemy
(
    "distancetoenemy",
    EV_DEFAULT,
    NULL,
    NULL,
    "Get the distance from the Actor to its enemy",
    EV_GETTER
);
Event EV_Actor_MoveTo
(
    "moveto",
    EV_DEFAULT,
    "ss",
    "anim dest",
    "Specify the location to move to, with animation anim.",
    EV_NORMAL
);
Event EV_Actor_WalkTo
(
    "walkto",
    EV_DEFAULT,
    "s",
    "dest",
    "Specify the location to walk to.",
    EV_NORMAL
);
Event EV_Actor_RunTo
(
    "runto",
    EV_DEFAULT,
    "s",
    "dest",
    "Specify the location to run to.",
    EV_NORMAL
);
Event EV_Actor_CrouchTo
(
    "crouchto",
    EV_DEFAULT,
    "s",
    "dest",
    "Specify the location to crouch to.",
    EV_NORMAL
);
Event EV_Actor_CrawlTo
(
    "crawlto",
    EV_DEFAULT,
    "s",
    "dest",
    "Specify the location to crawl to.",
    EV_NORMAL
);
Event EV_Actor_AimAt
(
    "aimat",
    EV_DEFAULT,
    "s",
    "target",
    "Specify the target to aim at.",
    EV_NORMAL
);
Event EV_Actor_Follow
(
    "follow",
    EV_DEFAULT,
    "e",
    "entity",
    "Specify the entity to follow.",
    EV_NORMAL
);
Event EV_Actor_DeathEmbalm
(
    "deathembalm",
    EV_DEFAULT,
    NULL,
    NULL,
    "preps the dead actor for turning nonsolid gradually over time",
    EV_NORMAL
);
Event EV_Actor_Anim
(
    "anim",
    EV_DEFAULT,
    "s",
    "name",
    "Play animation.",
    EV_NORMAL
);
Event EV_Actor_Anim_Scripted
(
    "anim_scripted",
    EV_DEFAULT,
    "s",
    "name",
    "Play scripted animation.",
    EV_NORMAL
);
Event EV_Actor_Anim_Noclip
(
    "anim_noclip",
    EV_DEFAULT,
    "s",
    "name",
    "Play noclip animation.",
    EV_NORMAL
);
Event EV_Actor_Anim_Attached
(
    "anim_attached",
    EV_DEFAULT,
    "s",
    "name",
    "Play attached animation.",
    EV_NORMAL
);
Event EV_Actor_AnimLoop
(
    "animloop",
    EV_DEFAULT,
    "s",
    "name",
    "Loop animation.",
    EV_NORMAL
);
Event EV_Actor_AnimScript
(
    "animscript",
    EV_DEFAULT,
    "s",
    "name",
    "Play the animation script",
    EV_NORMAL
);
Event EV_Actor_AnimScript_Scripted
(
    "animscript_scripted",
    EV_DEFAULT,
    "s",
    "name",
    "Play the scripted animation script",
    EV_NORMAL
);
Event EV_Actor_AnimScript_Noclip
(
    "animscript_noclip",
    EV_DEFAULT,
    "s",
    "name",
    "Play the noclip animation script",
    EV_NORMAL
);
Event EV_Actor_AnimScript_Attached
(
    "animscript_attached",
    EV_DEFAULT,
    "s",
    "name",
    "Play a noclip animation even when attached",
    EV_NORMAL
);
Event EV_Actor_Reload_mg42
(
    "reload_mg42",
    EV_DEFAULT,
    NULL,
    NULL,
    "Reload the mg42 - only used by machinegunner",
    EV_NORMAL
);
Event EV_Actor_Dumb
(
    "dumb",
    EV_DEFAULT,
    NULL,
    NULL,
    "Make Actor dumb.",
    EV_NORMAL
);
Event EV_Actor_Physics_On
(
    "physics_on",
    EV_DEFAULT,
    NULL,
    NULL,
    "turn physics on.",
    EV_NORMAL
);
Event EV_Actor_Physics_Off
(
    "physics_off",
    EV_DEFAULT,
    NULL,
    NULL,
    "turn physics off.",
    EV_NORMAL
);
Event EV_Actor_SetAnim
(
    "setanim",
    EV_DEFAULT,
    "sifs",
    "anim slot weight flagged",
    "Set animation slot",
    EV_NORMAL
);
Event EV_Actor_SetAnimLength
(
    "setanimlength",
    EV_DEFAULT,
    "f",
    "time",
    "Set the maximum time an animation will play",
    EV_NORMAL
);
Event EV_Actor_EndActionAnim
(
    "endactionanim",
    EV_DEFAULT,
    "",
    "",
    "End any aiming/action animation which is currently playing",
    EV_NORMAL
);
Event EV_Actor_SetMotionAnim
(
    "setmotionanim",
    EV_DEFAULT,
    "s",
    "anim",
    "Set motion animation (handler scripts only)",
    EV_NORMAL
);
Event EV_Actor_SetAimMotionAnim
(
    "setaimmotionanim",
    EV_DEFAULT,
    "ss",
    "anim_crouch anim_stand",
    "Set aim motion animation (handler scripts only)",
    EV_NORMAL
);
Event EV_Actor_SetActionAnim
(
    "setactionanim",
    EV_DEFAULT,
    "sff",
    "base_anim lower_limit upper_limit",
    "Set the base action animation and range that they cover",
    EV_NORMAL
);
Event EV_Actor_UpperAnim
(
    "upperanim",
    EV_DEFAULT,
    "s",
    "anim",
    "Set the upper body animation",
    EV_NORMAL
);
Event EV_Actor_SetUpperAnim
(
    "setupperanim",
    EV_DEFAULT,
    "s",
    "anim",
    "Set the upper body animation - used by animation script only",
    EV_NORMAL
);
Event EV_Actor_SetCrossblendTime
(
    "blendtime",
    EV_DEFAULT,
    NULL,
    NULL,
    "Set the crossblend time to something other than the default, in seconds",
    EV_SETTER
);
Event EV_Actor_GetCrossblendTime
(
    "blendtime",
    EV_DEFAULT,
    NULL,
    NULL,
    "Get the crossblend time",
    EV_GETTER
);
Event EV_Actor_SetPosition
(
    "position",
    EV_DEFAULT,
    NULL,
    NULL,
    "The Position the Actor wants to be and should animate towards",
    EV_SETTER
);
Event EV_Actor_GetPosition
(
    "position",
    EV_DEFAULT,
    NULL,
    NULL,
    "The Position the Actor wants to be and should animate towards",
    EV_GETTER
);
Event EV_Actor_SetEmotion
(
    "emotion",
    EV_DEFAULT,
    NULL,
    NULL,
    "The method of setting the facial expression of the Actor",
    EV_SETTER
);
Event EV_Actor_SetAnimFinal
(
    "animfinal",
    EV_DEFAULT,
    NULL,
    NULL,
    "Whether the animation was succesfully finished",
    EV_SETTER
);
Event EV_Actor_GetWeaponType
(
    "weapontype",
    EV_DEFAULT,
    NULL,
    NULL,
    "The Weapon Type of the Actor",
    EV_GETTER
);
Event EV_Actor_GetWeaponGroup
(
    "weapongroup",
    EV_DEFAULT,
    NULL,
    NULL,
    "Specifies weapon animation set to use in anim scripts",
    EV_GETTER
);
Event EV_Entity_Start
(
    "entitystart",
    EV_DEFAULT,
    NULL,
    NULL,
    "Initialize a Actor.",
    EV_NORMAL
);
Event EV_Actor_LookAt
(
    "lookat",
    EV_DEFAULT,
    "e",
    "entity",
    "The actor will look at this entity.",
    EV_NORMAL
);
Event EV_Actor_EyesLookAt
(
    "eyeslookat",
    EV_DEFAULT,
    "e",
    "entity",
    "The actor will look at this entity.",
    EV_NORMAL
);
Event EV_Actor_PointAt
(
    "pointat",
    EV_DEFAULT,
    "e",
    "entity",
    "The actor will point at this entity.",
    EV_NORMAL
);
Event EV_Actor_TurnTo
(
    "turnto",
    EV_DEFAULT,
    "e",
    "entity",
    "The actor will turn to this entity.",
    EV_NORMAL
);
Event EV_Actor_SetTurnDoneError
(
    "turndoneerror",
    EV_DEFAULT,
    "f",
    "error",
    "The error amount that turndone will occur for the turnto command.",
    EV_NORMAL
);
Event EV_Actor_SetTurnDoneError2
(
    "turndoneerror",
    EV_DEFAULT,
    "f",
    "error",
    "The error amount that turndone will occur for the turnto command.",
    EV_SETTER
);
Event EV_Actor_GetTurnDoneError
(
    "turndoneerror",
    EV_DEFAULT,
    NULL,
    NULL,
    "The error amount that turndone will occur for the turnto command.",
    EV_GETTER
);
Event EV_Actor_IdleSayAnim
(
    "idlesay",
    EV_DEFAULT,
    "s",
    "animation",
    "The name of an idle dialog animation to play",
    EV_NORMAL
);
Event EV_Actor_SayAnim
(
    "say",
    EV_DEFAULT,
    "s",
    "animation",
    "The name of a dialog animation to play",
    EV_NORMAL
);
Event EV_Actor_SetSayAnim
(
    "setsay",
    EV_DEFAULT,
    "s",
    "animation",
    "The name of a dialog animation to play - used by animation script only",
    EV_NORMAL
);
Event EV_Actor_DamagePuff
(
    "damagepuff",
    EV_DEFAULT,
    "vv",
    "position direction",
    "Spawns a puff of 'blood' smoke at the speficied location in the specified direction.",
    EV_NORMAL
);
Event EV_Actor_SetAngleYawSpeed
(
    "turnspeed",
    EV_DEFAULT,
    "f",
    "speed",
    "The turn speed of the actor.",
    EV_SETTER
);
Event EV_Actor_SetAngleYawSpeed2
(
    "turnspeed",
    EV_DEFAULT,
    "f",
    "speed",
    "The turn speed of the actor.",
    EV_NORMAL
);
Event EV_Actor_GetAngleYawSpeed
(
    "turnspeed",
    EV_DEFAULT,
    NULL,
    NULL,
    "The turn speed of the actor.",
    EV_GETTER
);
Event EV_Actor_SetAimTarget
(
    "setaimtarget",
    EV_DEFAULT,
    "ei",
    "entity bMakeEnemy",
    "Sets the primary weapon's aim target. if you pass a 1 for p2, the target will become the current enemy...",
    EV_NORMAL
);
Event EV_Actor_ReadyToFire
(
    "ReadyToFire",
    EV_DEFAULT,
    NULL,
    NULL,
    "Returns if ready to fire",
    EV_RETURN
);
Event EV_Actor_AIOff
(
    "ai_off",
    EV_DEFAULT,
    NULL,
    NULL,
    "Turns the AI off for this actor.",
    EV_NORMAL
);
Event EV_Actor_AIOn
(
    "ai_on",
    EV_DEFAULT,
    NULL,
    NULL,
    "Turns the AI on for this actor.",
    EV_NORMAL
);
Event EV_Actor_GetSight
(
    "sight",
    EV_DEFAULT,
    NULL,
    NULL,
    "Gets the vision distance of the actor.",
    EV_GETTER
);
Event EV_Actor_SetSight
(
    "sight",
    EV_DEFAULT,
    "f",
    "max_sight_range",
    "Sets the vision distance of the actor.",
    EV_SETTER
);
Event EV_Actor_SetSight2
(
    "sight",
    EV_DEFAULT,
    "f",
    "max_sight_range",
    "Sets the vision distance of the actor.",
    EV_NORMAL
);
Event EV_Actor_GetHearing
(
    "hearing",
    EV_DEFAULT,
    "f",
    "radius",
    "The hearing radius of the actor",
    EV_GETTER
);
Event EV_Actor_SetHearing
(
    "hearing",
    EV_DEFAULT,
    "f",
    "radius",
    "The hearing radius of the actor",
    EV_NORMAL
);
Event EV_Actor_SetHearing2
(
    "hearing",
    EV_DEFAULT,
    "f",
    "radius",
    "The hearing radius of the actor",
    EV_SETTER
);
Event EV_Actor_GetFov
(
    "fov",
    EV_DEFAULT,
    NULL,
    NULL,
    "The fov angle of the actor",
    EV_GETTER
);
Event EV_Actor_SetFov
(
    "fov",
    EV_DEFAULT,
    "f",
    "angle",
    "The fov angle of the actor",
    EV_NORMAL
);
Event EV_Actor_SetFov2
(
    "fov",
    EV_DEFAULT,
    "f",
    "angle",
    "The fov angle of the actor",
    EV_SETTER
);
Event EV_Actor_SetTypeIdle
(
    "type_idle",
    EV_DEFAULT,
    "s",
    "value",
    "Sets the idle type of the actor.",
    EV_NORMAL
);
Event EV_Actor_SetTypeIdle2
(
    "type_idle",
    EV_DEFAULT,
    "s",
    "value",
    "Sets the idle type of the actor.",
    EV_SETTER
);
Event EV_Actor_GetTypeIdle
(
    "type_idle",
    EV_DEFAULT,
    NULL,
    NULL,
    "Gets the idle type of the actor.",
    EV_GETTER
);
Event EV_Actor_SetTypeAttack
(
    "type_attack",
    EV_DEFAULT,
    "s",
    "value",
    "Sets the attack type of the actor.",
    EV_NORMAL
);
Event EV_Actor_SetTypeAttack2
(
    "type_attack",
    EV_DEFAULT,
    "s",
    "value",
    "Sets the attack type of the actor.",
    EV_SETTER
);
Event EV_Actor_GetTypeAttack
(
    "type_attack",
    EV_DEFAULT,
    NULL,
    NULL,
    "Gets the attack type of the actor.",
    EV_GETTER
);
Event EV_Actor_SetTypeDisguise
(
    "type_disguise",
    EV_DEFAULT,
    "s",
    "value",
    "Sets the disguise type of the actor.",
    EV_NORMAL
);
Event EV_Actor_SetTypeDisguise2
(
    "type_disguise",
    EV_DEFAULT,
    "s",
    "value",
    "Sets the disguise type of the actor.",
    EV_SETTER
);
Event EV_Actor_GetTypeDisguise
(
    "type_disguise",
    EV_DEFAULT,
    NULL,
    NULL,
    "Gets the disguise type of the actor.",
    EV_GETTER
);
Event EV_Actor_SetDisguiseLevel
(
    "disguise_level",
    EV_DEFAULT,
    "i",
    "value",
    "Sets the disguise level of the actor. May be 1 or 2",
    EV_NORMAL
);
Event EV_Actor_SetDisguiseLevel2
(
    "disguise_level",
    EV_DEFAULT,
    "i",
    "value",
    "Sets the disguise level of the actor. May be 1 or 2",
    EV_SETTER
);
Event EV_Actor_GetDisguiseLevel
(
    "disguise_level",
    EV_DEFAULT,
    NULL,
    NULL,
    "Gets the disguise level of the actor. May be 1 or 2",
    EV_GETTER
);
Event EV_Actor_SetTypeGrenade
(
    "type_grenade",
    EV_DEFAULT,
    "s",
    "value",
    "Sets the grenade type of the actor.",
    EV_NORMAL
);
Event EV_Actor_SetTypeGrenade2
(
    "type_grenade",
    EV_DEFAULT,
    "s",
    "value",
    "Sets the grenade type of the actor.",
    EV_SETTER
);
Event EV_Actor_GetTypeGrenade
(
    "type_grenade",
    EV_DEFAULT,
    NULL,
    NULL,
    "Gets the grenade type of the actor.",
    EV_GETTER
);
Event EV_Actor_SetPatrolPath
(
    "patrolpath",
    EV_DEFAULT,
    "s",
    "value",
    "Sets the name of the patrol path for the actor (must have type set to patrol for effect)",
    EV_NORMAL
);
Event EV_Actor_SetPatrolPath2
(
    "patrolpath",
    EV_DEFAULT,
    "s",
    "value",
    "Sets the name of the patrol path for the actor (must have type set to patrol for effect)",
    EV_SETTER
);
Event EV_Actor_GetPatrolPath
(
    "patrolpath",
    EV_DEFAULT,
    NULL,
    NULL,
    "Gets the name of the patrol path for the actor (must have type set to patrol for effect)",
    EV_GETTER
);
Event EV_Actor_SetPatrolWaitTrigger
(
    "waittrigger",
    EV_DEFAULT,
    "b",
    "bool",
    "If true, patrol guys and running men wait until triggered to move",
    EV_NORMAL
);
Event EV_Actor_SetPatrolWaitTrigger2
(
    "waittrigger",
    EV_DEFAULT,
    "b",
    "bool",
    "If true, patrol guys and running men wait until triggered to move",
    EV_SETTER
);
Event EV_Actor_GetPatrolWaitTrigger
(
    "waittrigger",
    EV_DEFAULT,
    NULL,
    NULL,
    "If true, patrol guys and running men wait until triggered to move",
    EV_GETTER
);
Event EV_Actor_SetAlarmNode
(
    "alarmnode",
    EV_DEFAULT,
    "s",
    "value",
    "Sets the name of the alarm node for the actor (must have type set to alarm for effect)",
    EV_NORMAL
);
Event EV_Actor_SetAlarmNode2
(
    "alarmnode",
    EV_DEFAULT,
    "s",
    "value",
    "Sets the name of the alarm node for the actor (must have type set to alarm for effect)",
    EV_SETTER
);
Event EV_Actor_GetAlarmNode
(
    "alarmnode",
    EV_DEFAULT,
    NULL,
    NULL,
    "Gets the name of the alarm node for the actor (must have type set to alarm for effect)",
    EV_GETTER
);
Event EV_Actor_SetPreAlarmThread
(
    "prealarmthread",
    EV_DEFAULT,
    "s",
    "value",
    "Sets the name of the pre alarm thread for the actor (must have type set to alarm for effect)",
    EV_NORMAL
);
Event EV_Actor_SetPreAlarmThread2
(
    "prealarmthread",
    EV_DEFAULT,
    "s",
    "value",
    "Sets the name of the pre alarm thread for the actor (must have type set to alarm for effect)",
    EV_SETTER
);
Event EV_Actor_SetAlarmThread
(
    "alarmthread",
    EV_DEFAULT,
    "s",
    "value",
    "Sets the name of the alarm thread for the actor (must have type set to alarm for effect)",
    EV_NORMAL
);
Event EV_Actor_SetAlarmThread2
(
    "alarmthread",
    EV_DEFAULT,
    "s",
    "value",
    "Sets the name of the alarm thread for the actor (must have type set to alarm for effect)",
    EV_SETTER
);
Event EV_Actor_GetAlarmThread
(
    "alarmthread",
    EV_DEFAULT,
    NULL,
    NULL,
    "Gets the name of the alarm thread for the actor (must have type set to alarm for effect)",
    EV_GETTER
);
Event EV_Actor_SetDisguiseAcceptThread
(
    "disguise_accept_thread",
    EV_DEFAULT,
    "s",
    "value",
    "Sets the name of the thread for the actor to start when accepting papers",
    EV_NORMAL
);
Event EV_Actor_SetDisguiseAcceptThread2
(
    "disguise_accept_thread",
    EV_DEFAULT,
    "s",
    "value",
    "Sets the name of the thread for the actor to start when accepting papers",
    EV_SETTER
);
Event EV_Actor_GetDisguiseAcceptThread
(
    "disguise_accept_thread",
    EV_DEFAULT,
    NULL,
    NULL,
    "Gets the name of the thread for the actor to start when accepting papers",
    EV_GETTER
);
Event EV_Actor_SetAccuracy
(
    "accuracy",
    EV_DEFAULT,
    "f",
    "value",
    "Set percent to hit",
    EV_NORMAL
);
Event EV_Actor_SetAccuracy2
(
    "accuracy",
    EV_DEFAULT,
    "f",
    "value",
    "Set percent to hit",
    EV_SETTER
);
Event EV_Actor_GetAccuracy
(
    "accuracy",
    EV_DEFAULT,
    NULL,
    NULL,
    "Set percent to hit",
    EV_GETTER
);
Event EV_Actor_SetMinDistance
(
    "mindist",
    EV_DEFAULT,
    "f",
    "distance",
    "Sets the minimum distance the AI tries to keep between itself and the player",
    EV_NORMAL
);
Event EV_Actor_SetMinDistance2
(
    "mindist",
    EV_DEFAULT,
    "f",
    "distance",
    "Sets the minimum distance the AI tries to keep between itself and the player",
    EV_SETTER
);
Event EV_Actor_GetMinDistance
(
    "mindist",
    EV_DEFAULT,
    NULL,
    NULL,
    "Gets the minimum distance the AI tries to keep between itself and the player",
    EV_GETTER
);
Event EV_Actor_SetMaxDistance
(
    "maxdist",
    EV_DEFAULT,
    "f",
    "distance",
    "Sets the maximum distance the AI tries to allow between itself and the player",
    EV_NORMAL
);
Event EV_Actor_SetMaxDistance2
(
    "maxdist",
    EV_DEFAULT,
    "f",
    "distance",
    "Sets the maximum distance the AI tries to allow between itself and the player",
    EV_SETTER
);
Event EV_Actor_GetMaxDistance
(
    "maxdist",
    EV_DEFAULT,
    NULL,
    NULL,
    "Gets the maximum distance the AI tries to keep between itself and the player",
    EV_GETTER
);
Event EV_Actor_GetLeash
(
    "leash",
    EV_DEFAULT,
    NULL,
    NULL,
    "Gets the maximum distance the AI will wander from its leash home",
    EV_GETTER
);
Event EV_Actor_SetLeash
(
    "leash",
    EV_DEFAULT,
    "f",
    "distance",
    "Sets the maximum distance the AI will wander from its leash home",
    EV_SETTER
);
Event EV_Actor_SetLeash2
(
    "leash",
    EV_DEFAULT,
    "f",
    "distance",
    "Sets the maximum distance the AI will wander from its leash home",
    EV_NORMAL
);
Event EV_Actor_GetInterval
(
    "interval",
    EV_DEFAULT,
    NULL,
    NULL,
    "Gets the distance AI tries to keep between squadmates while moving.",
    EV_GETTER
);
Event EV_Actor_SetInterval
(
    "interval",
    EV_DEFAULT,
    "f",
    "distance",
    "Sets the distance AI tries to keep between squadmates while moving.",
    EV_SETTER
);
Event EV_Actor_SetInterval2
(
    "interval",
    EV_DEFAULT,
    "f",
    "distance",
    "Sets the distance AI tries to keep between squadmates while moving.",
    EV_NORMAL
);
Event EV_Actor_GetRunAnim
(
    "GetRunAnim",
    EV_DEFAULT,
    NULL,
    NULL,
    "Internal usage",
    EV_RETURN
);
Event EV_Actor_GetWalkAnim
(
    "GetWalkAnim",
    EV_DEFAULT,
    NULL,
    NULL,
    "Internal usage",
    EV_RETURN
);
Event EV_Actor_GetAnimName
(
    "animname",
    EV_DEFAULT,
    NULL,
    NULL,
    "Gets the animname.",
    EV_GETTER
);
Event EV_Actor_SetAnimName
(
    "animname",
    EV_DEFAULT,
    NULL,
    NULL,
    "Sets the animname.",
    EV_SETTER
);
Event EV_Actor_SetDisguiseRange
(
    "disguise_range",
    EV_DEFAULT,
    "f",
    "range_in_units",
    "Sets the maximum distance for disguise behavior to get triggered",
    EV_SETTER
);
Event EV_Actor_SetDisguiseRange2
(
    "disguise_range",
    EV_DEFAULT,
    "f",
    "range_in_units",
    "Sets the maximum distance for disguise behavior to get triggered",
    EV_NORMAL
);
Event EV_Actor_GetDisguiseRange
(
    "disguise_range",
    EV_DEFAULT,
    NULL,
    NULL,
    "Gets the maximum distance for disguise behavior to get triggered",
    EV_GETTER
);
Event EV_Actor_SetDisguisePeriod
(
    "disguise_period",
    EV_DEFAULT,
    "f",
    "period_in_seconds",
    "Sets the time between the end of one disguise behavior and start of the next",
    EV_SETTER
);
Event EV_Actor_SetDisguisePeriod2
(
    "disguise_period",
    EV_DEFAULT,
    "f",
    "period_in_seconds",
    "Sets the time between the end of one disguise behavior and start of the next",
    EV_NORMAL
);
Event EV_Actor_GetDisguisePeriod
(
    "disguise_period",
    EV_DEFAULT,
    NULL,
    NULL,
    "Gets the time between the end of one disguise behavior and start of the next",
    EV_GETTER
);
Event EV_Actor_AttackPlayer
(
    "attackplayer",
    EV_DEFAULT,
    NULL,
    NULL,
    "Force Actor to attack the player",
    EV_NORMAL
);
Event EV_Actor_SetSoundAwareness
(
    "sound_awareness",
    EV_DEFAULT,
    "f",
    "awareness_percent",
    "sets the awareness of sounds in 0-100 percent chance of hearing a sound withinhalf of the sound's radius' fades "
    "to z"
    "ero outside sound's radius",
    EV_NORMAL
);
Event EV_Actor_SetSoundAwareness2
(
    "sound_awareness",
    EV_DEFAULT,
    "f",
    "awareness_percent",
    "sets the awareness of sounds in 0-100 percent chance of hearing a sound withinhalf of the sound's radius' fades "
    "to z"
    "ero outside sound's radius",
    EV_SETTER
);
Event EV_Actor_GetSoundAwareness
(
    "sound_awareness",
    EV_DEFAULT,
    NULL,
    NULL,
    "gets the awareness of sounds in 0-100 percent chance of hearing a sound withinhalf of the sound's radius' fades "
    "to z"
    "ero outside sound's radius",
    EV_GETTER
);
Event EV_Actor_SetGrenadeAwareness
(
    "gren_awareness",
    EV_DEFAULT,
    "f",
    "awareness_percent",
    "sets the awareness of grenades in 0-100 percent chance of responding to a grenadewhen the AI sees it (applied "
    "once e"
    "very 0.4 seconds)",
    EV_NORMAL
);
Event EV_Actor_SetGrenadeAwareness2
(
    "gren_awareness",
    EV_DEFAULT,
    "f",
    "awareness_percent",
    "sets the awareness of grenades in 0-100 percent chance of responding to a grenadewhen the AI sees it (applied "
    "once e"
    "very 0.4 seconds)",
    EV_SETTER
);
Event EV_Actor_GetGrenadeAwareness
(
    "gren_awareness",
    EV_DEFAULT,
    NULL,
    NULL,
    "gets the awareness of grenades in 0-100 percent chance of responding to a grenadewhen the AI sees it (applied "
    "once e"
    "very 0.4 seconds)",
    EV_GETTER
);
Event EV_Actor_SetTurret
(
    "turret",
    EV_DEFAULT,
    "s",
    "turret",
    "Sets the turret of the actor.",
    EV_NORMAL
);
Event EV_Actor_SetTurret2
(
    "turret",
    EV_DEFAULT,
    "s",
    "turret",
    "Sets the turret of the actor.",
    EV_SETTER
);
Event EV_Actor_GetTurret
(
    "turret",
    EV_DEFAULT,
    NULL,
    NULL,
    "Gets the turret of the actor.",
    EV_GETTER
);
Event EV_Actor_AttachGrenade
(
    "attachgrenade",
    EV_DEFAULT,
    NULL,
    NULL,
    "Used only by grenade return animations to tell the code when to attach the grenade to the actor",
    EV_NORMAL
);
Event EV_Actor_DetachGrenade
(
    "detachgrenade",
    EV_DEFAULT,
    NULL,
    NULL,
    "Used only by grenade return animations to tell the code when to throw the grenade",
    EV_NORMAL
);
Event EV_Actor_FireGrenade
(
    "fire_grenade",
    EV_DEFAULT,
    NULL,
    NULL,
    "Used only by grenade throw animations to tell the code when to throw a grenade",
    EV_NORMAL
);
Event EV_Actor_EnableEnemy
(
    "enableEnemy",
    EV_DEFAULT,
    NULL,
    NULL,
    "sets enableEnemy variable",
    EV_SETTER
);
Event EV_Actor_EnablePain
(
    "enablePain",
    EV_DEFAULT,
    NULL,
    NULL,
    "sets enablePain variable",
    EV_SETTER
);
Event EV_Actor_SetPainHandler
(
    "painhandler",
    EV_DEFAULT,
    NULL,
    NULL,
    "Sets the current script that will handle pain events",
    EV_SETTER
);
Event EV_Actor_GetPainHandler
(
    "painhandler",
    EV_DEFAULT,
    NULL,
    NULL,
    "Gets the current script that will handle pain events",
    EV_GETTER
);
Event EV_Actor_SetDeathHandler
(
    "deathhandler",
    EV_DEFAULT,
    NULL,
    NULL,
    "Sets the current script that will handle death events",
    EV_SETTER
);
Event EV_Actor_GetDeathHandler
(
    "deathhandler",
    EV_DEFAULT,
    NULL,
    NULL,
    "Gets the current script that will handle death events",
    EV_GETTER
);
Event EV_Actor_SetAttackHandler
(
    "attackhandler",
    EV_DEFAULT,
    NULL,
    NULL,
    "Sets the current script that will handle attack events",
    EV_SETTER
);
Event EV_Actor_GetAttackHandler
(
    "attackhandler",
    EV_DEFAULT,
    NULL,
    NULL,
    "Gets the current script that will handle attack events",
    EV_GETTER
);
Event EV_Actor_SetAmmoGrenade
(
    "ammo_grenade",
    EV_DEFAULT,
    "i",
    "grenade_count",
    "Gives the AI some grenades",
    EV_NORMAL
);
Event EV_Actor_SetAmmoGrenade2
(
    "ammo_grenade",
    EV_DEFAULT,
    "i",
    "grenade_count",
    "Gives the AI some grenades",
    EV_SETTER
);
Event EV_Actor_GetAmmoGrenade
(
    "ammo_grenade",
    EV_DEFAULT,
    NULL,
    NULL,
    "Returns how many grenades an AI has",
    EV_GETTER
);
Event EV_Actor_SetMood
(
    "mood",
    EV_DEFAULT,
    "s",
    "new_mood",
    "sets the AI mood... must be 'bored', 'nervous', 'curious', or 'alert'.",
    EV_SETTER
);
Event EV_Actor_GetMood
(
    "mood",
    EV_DEFAULT,
    NULL,
    NULL,
    "gets the AI mood: 'bored', 'nervous', 'curious', or 'alert'.",
    EV_GETTER
);
Event EV_Actor_SetHeadModel
(
    "headmodel",
    EV_DEFAULT,
    "s",
    "headmodel",
    "sets the head model",
    EV_NORMAL
);
Event EV_Actor_GetHeadModel
(
    "headmodel",
    EV_DEFAULT,
    NULL,
    NULL,
    "gets the head model",
    EV_GETTER
);
Event EV_Actor_SetHeadSkin
(
    "headskin",
    EV_DEFAULT,
    "s",
    "headskin",
    "sets the head skin",
    EV_NORMAL
);
Event EV_Actor_GetHeadSkin
(
    "headskin",
    EV_DEFAULT,
    NULL,
    NULL,
    "gets the head skin",
    EV_GETTER
);
Event EV_Actor_ShareEnemy
(
    "share_enemy",
    EV_DEFAULT,
    NULL,
    NULL,
    "internal code use only - shares an AI's enemy with his squad mates.",
    EV_NORMAL
);
Event EV_Actor_ShareGrenade
(
    "share_grenade",
    EV_DEFAULT,
    NULL,
    NULL,
    "internal code use only - shares an AI's grenade with his squad mates.",
    EV_NORMAL
);
Event EV_Actor_InterruptPoint
(
    "interrupt_point",
    EV_DEFAULT,
    NULL,
    NULL,
    "hint from animation scripts to AI code that now is a good time to switch animations",
    EV_NORMAL
);
Event EV_Actor_SetNoIdle
(
    "no_idle",
    EV_DEFAULT,
    NULL,
    NULL,
    "Specifies if the actor will not go into idle after playing an animation",
    EV_SETTER
);
Event EV_Actor_GetNoIdle
(
    "no_idle",
    EV_DEFAULT,
    NULL,
    NULL,
    "Gets if the actor will not go into idle after playing an animation",
    EV_GETTER
);
Event EV_Actor_GetEnemy
(
    "enemy",
    EV_DEFAULT,
    NULL,
    NULL,
    "Get the actor's current enemy",
    EV_GETTER
);
Event EV_Actor_GetMaxNoticeTimeScale
(
    "noticescale",
    EV_DEFAULT,
    NULL,
    NULL,
    "Get the max multiplier in time to notice an enemy (default 100, half as big notices twice as fast)",
    EV_GETTER
);
Event EV_Actor_SetMaxNoticeTimeScale
(
    "noticescale",
    EV_DEFAULT,
    "f",
    "multiplier",
    "Set the max multiplier in time to notice an enemy (default 100, half as big notices twice as fast)",
    EV_SETTER
);
Event EV_Actor_SetMaxNoticeTimeScale2
(
    "noticescale",
    EV_DEFAULT,
    "f",
    "multiplier",
    "Set the max multiplier in time to notice an enemy (default 100, half as big notices twice as fast)",
    EV_NORMAL
);
Event EV_Actor_GetFixedLeash(
    "fixedleash",
    EV_DEFAULT,
    NULL,
    NULL,
    "if non-zero, the leash will never auto-reset; if zero, the leash may auto-reset",
    EV_GETTER
);
Event EV_Actor_SetFixedLeash(
    "fixedleash",
    EV_DEFAULT,
    "f",
    "multiplier",
    "if non-zero, the leash will never auto-reset; if zero, the leash may auto-reset",
    EV_SETTER
);
Event EV_Actor_SetFixedLeash2(
    "fixedleash",
    EV_DEFAULT,
    "f",
    "multiplier",
    "if non-zero, the leash will never auto-reset; if zero, the leash may auto-reset",
    EV_NORMAL
);
Event EV_Actor_Holster
(
    "holster",
    EV_DEFAULT,
    "i",
    "if non-zero, affects offhand",
    "Holster weapon",
    EV_NORMAL
);
Event EV_Actor_Unholster
(
    "unholster",
    EV_DEFAULT,
    "i",
    "if non-zero, affects offhand",
    "Unholster weapon",
    EV_NORMAL
);
Event EV_Actor_IsEnemyVisible
(
    "is_enemy_visible",
    EV_DEFAULT,
    NULL,
    NULL,
    "0 if the enemy is not currently visible, 1 if he is",
    EV_NORMAL
);
Event EV_Actor_GetEnemyVisibleChangeTime
(
    "enemy_visible_change_time",
    EV_DEFAULT,
    NULL,
    NULL,
    "Get the last time whether or not the enemy is visible changed, in seconds",
    EV_NORMAL
);
Event EV_Actor_GetLastEnemyVisibleTime
(
    "last_enemy_visible_time",
    EV_DEFAULT,
    NULL,
    NULL,
    "Get the last time the enemy was visible, in seconds",
    EV_NORMAL
);
Event EV_Actor_SetFallHeight
(
    "fallheight",
    EV_DEFAULT,
    "f",
    "height",
    "Set the fallheight",
    EV_SETTER
);
Event EV_Actor_SetFallHeight2
(
    "fallheight",
    EV_DEFAULT,
    "f",
    "height",
    "Set the fallheight",
    EV_NORMAL
);
Event EV_Actor_GetFallHeight
(
    "fallheight",
    EV_DEFAULT,
    NULL,
    NULL,
    "Set the fallheight",
    EV_GETTER
);
Event EV_Actor_CanMoveTo(
    "canmoveto",
    EV_DEFAULT,
    "v",
    "position",
    "returns a boolean if the AI can move to a point; for use in anim scripts",
    EV_RETURN
);
Event EV_Actor_MoveDir
(
    "movedir",
    EV_DEFAULT,
    NULL,
    NULL,
    "Returns a unit vector pointing in the current direction of motion, or zero if not moving.This still has meaning "
    "if v"
    "elocity is zero but the AI is starting to move on a path.",
    EV_GETTER
);
Event EV_Actor_ResetLeash
(
    "resetleash",
    EV_DEFAULT,
    NULL,
    NULL,
    "resets the AI's leash to their current position",
    EV_NORMAL
);
Event EV_Actor_IntervalDir
(
    "intervaldir",
    EV_DEFAULT,
    NULL,
    NULL,
    "the direction the AI would like to move to maintain its interval",
    EV_GETTER
);
Event EV_Actor_Tether
(
    "tether",
    EV_DEFAULT,
    "e",
    "entity",
    "the entity to which the AI's leash should be tethered",
    EV_NORMAL
);
Event EV_Actor_GetThinkState(
    "thinkstate",
    EV_DEFAULT,
    NULL,
    NULL,
    "current ai think state; can be void, idle, pain, killed, attack, curious, disguise, or grenade.",
    EV_GETTER
);
Event EV_Actor_GetEnemyShareRange
(
    "enemysharerange",
    EV_DEFAULT,
    NULL,
    NULL,
    "gets the range outside which the AI will not receive notification that a teammate has a new enemy",
    EV_GETTER
);
Event EV_Actor_SetEnemyShareRange
(
    "enemysharerange",
    EV_DEFAULT,
    "f",
    "range",
    "sets the range outside which the AI will not receive notification that a teammate has a new enemy",
    EV_SETTER
);
Event EV_Actor_SetEnemyShareRange2
(
    "enemysharerange",
    EV_DEFAULT,
    "f",
    "range",
    "sets the range outside which the AI will not receive notification that a teammate has a new enemy",
    EV_NORMAL
);
Event EV_Actor_SetWeapon
(
    "weapon",
    EV_DEFAULT,
    "s",
    "weapon_modelname",
    "Sets the weapon.",
    EV_SETTER
);
Event EV_Actor_GetWeapon
(
    "weapon",
    EV_DEFAULT,
    NULL,
    NULL,
    "Gets the weapon.",
    EV_GETTER
);
Event EV_Actor_GetVoiceType
(
    "voicetype",
    EV_DEFAULT,
    NULL,
    NULL,
    "Gets the voice type",
    EV_GETTER
);
Event EV_Actor_SetVoiceType
(
    "voicetype",
    EV_DEFAULT,
    NULL,
    NULL,
    "Set voicetype to magic letter postfix",
    EV_NORMAL
);
Event EV_Actor_SetVoiceType2
(
    "voicetype",
    EV_DEFAULT,
    NULL,
    NULL,
    "Set voicetype to magic letter postfix",
    EV_SETTER
);
Event EV_Actor_KickDir
(
    "kickdir",
    EV_DEFAULT,
    NULL,
    NULL,
    "Gets the direction the AI wants to kick",
    EV_GETTER
);
Event EV_Actor_GetNoLongPain
(
    "nolongpain",
    EV_DEFAULT,
    NULL,
    NULL,
    "Returns 1 if long pain is not allowed, or 0 if long pain is allowed.",
    EV_GETTER
);
Event EV_Actor_SetNoLongPain
(
    "nolongpain",
    EV_DEFAULT,
    "i",
    "allow",
    "Set to 1 if long pain is not allowed, or 0 if long pain is allowed.",
    EV_SETTER
);
Event EV_Actor_SetNoLongPain2
(
    "nolongpain",
    EV_DEFAULT,
    "i",
    "allow",
    "Set to 1 if long pain is not allowed, or 0 if long pain is allowed.",
    EV_NORMAL
);
Event EV_Actor_GetFavoriteEnemy
(
    "favoriteenemy",
    EV_DEFAULT,
    NULL,
    NULL,
    "Gets this AI's favorite enemy",
    EV_GETTER
);
Event EV_Actor_SetFavoriteEnemy
(
    "favoriteenemy",
    EV_DEFAULT,
    "e",
    "ai_or_player",
    "Gets this AI's favorite enemy",
    EV_SETTER
);
Event EV_Actor_SetFavoriteEnemy2
(
    "favoriteenemy",
    EV_DEFAULT,
    "e",
    "ai_or_player",
    "Gets this AI's favorite enemy",
    EV_NORMAL
);
Event EV_Actor_GetMumble
(
    "mumble",
    EV_DEFAULT,
    NULL,
    NULL,
    "Returns 1 if this guy is allowed to mumble, or 0 if he is not",
    EV_GETTER
);
Event EV_Actor_SetMumble
(
    "mumble",
    EV_DEFAULT,
    "i",
    "can_mumble",
    "Set to 1 if this guy is allowed to mumble, or 0 if he is not",
    EV_SETTER
);
Event EV_Actor_SetMumble2
(
    "mumble",
    EV_DEFAULT,
    "i",
    "can_mumble",
    "Set to 1 if this guy is allowed to mumble, or 0 if he is not",
    EV_NORMAL
);
Event EV_Actor_GetBreathSteam
(
    "breathsteam",
    EV_DEFAULT,
    NULL,
    NULL,
    "Returns 1 if this guy is allowed to have steamy breath, or 0 if he is not",
    EV_GETTER
);
Event EV_Actor_SetBreathSteam
(
    "breathsteam",
    EV_DEFAULT,
    "i",
    "can_breathe_out",
    "Set to 1 if this guy is allowed to have steamy breath, or 0 if he is not",
    EV_SETTER
);
Event EV_Actor_SetBreathSteam2
(
    "breathsteam",
    EV_DEFAULT,
    "i",
    "can_breathe_out",
    "Set to 1 if this guy is allowed to have steamy breath, or 0 if he is not",
    EV_NORMAL
);
Event EV_Actor_SetNextBreathTime
(
    "nextbreathtime",
    EV_DEFAULT,
    "f",
    "time_in_sec",
    "Sets the next time the guy will breath out",
    EV_NORMAL
);
Event EV_Actor_CalcGrenadeToss
(
    "calcgrenadetoss",
    EV_DEFAULT,
    "v",
    "target_position",
    "Called to calculate a grenade toss.  Must be called before a grenade throwing animation.\n"
    "Returns the name of the script to call with animscript if the toss can succeed, or  if the toss won't work.\n"
    "Should be called infrequently, and never during the middle of a grenade toss.",
    EV_RETURN
);
Event EV_Actor_CalcGrenadeToss2
(
    "calcgrenadetoss2",
    EV_DEFAULT,
    "vf",
    "target_position speed",
    "Called to calculate a grenade toss.  Must be called before a grenade throwing animation.\n"
    "Returns the name of the script to call with animscript if the toss can succeed, or  if the toss won't work.\n"
    "Should be called infrequently, and never during the middle of a grenade toss.\n"
    " The speed parameter is optional.  Pass a speed if you just want to override and throw no matter what...",
    EV_RETURN
);
Event EV_Actor_GetNoSurprise
(
    "nosurprise",
    EV_DEFAULT,
    NULL,
    NULL,
    "gets whether or not this guy is allowed to play a surprised animation when first encountering an enemy.\n",
    EV_GETTER
);
Event EV_Actor_SetNoSurprise
(
    "nosurprise",
    EV_DEFAULT,
    "i",
    "nosurprise",
    "set to 0 to allow this guy to play a surprised animation when first encountering an enemy.\n",
    EV_SETTER
);
Event EV_Actor_SetNoSurprise2
(
    "nosurprise",
    EV_DEFAULT,
    "i",
    "nosurprise",
    "set to 0 to allow this guy to play a surprised animation when first encountering an enemy.\n",
    EV_NORMAL
);
Event EV_Actor_GetSilent
(
    "silent",
    EV_DEFAULT,
    NULL,
    NULL,
    "gets whether or not this guy is allowed to say stuff besides pain and death sounds",
    EV_GETTER
);
Event EV_Actor_SetSilent
(
    "silent",
    EV_DEFAULT,
    "i",
    "silent",
    "set to 0 to prevent this guy from saying stuff besides pain and death sounds.\n",
    EV_SETTER
);
Event EV_Actor_SetSilent2
(
    "silent",
    EV_DEFAULT,
    "i",
    "silent",
    "set to 0 to prevent this guy from saying stuff besides pain and death sounds.\n",
    EV_NORMAL
);
Event EV_Actor_GetAvoidPlayer
(
    "avoidplayer",
    EV_DEFAULT,
    NULL,
    NULL,
    "is 0 if this AI won't automatically get out of the way, non-zero if he will\n",
    EV_GETTER
);
Event EV_Actor_SetAvoidPlayer
(
    "avoidplayer",
    EV_DEFAULT,
    "i",
    "allowavoid",
    "set to 0 if this AI shouldn't automatically get out of the way, non-zero if he should.\n",
    EV_SETTER
);
Event EV_Actor_SetAvoidPlayer2
(
    "avoidplayer",
    EV_DEFAULT,
    "i",
    "allowavoid",
    "set to 0 if this AI shouldn't automatically get out of the way, non-zero if he should.\n",
    EV_NORMAL
);
Event EV_Actor_SetMoveDoneRadius
(
    "movedoneradius",
    EV_DEFAULT,
    "f",
    "radius",
    "Set the waittill movedone radius, default 0 means don't use manual radius",
    EV_SETTER
);
Event EV_Actor_GetMoveDoneRadius
(
    "movedoneradius",
    EV_DEFAULT,
    "f",
    "radius",
    "Set the waittill movedone radius, default 0 means don't use manual radius",
    EV_GETTER
);
Event EV_Actor_BeDead(
    "bedead",
    EV_DEFAULT,
    NULL,
    NULL,
    "Forces the actor to be instantly and totally dead; no death animation is played",
    EV_NORMAL
);
Event EV_Actor_GetLookAroundAngle
(
    "lookaroundangle",
    EV_DEFAULT,
    NULL,
    NULL,
    "gets the angle in degrees left or right of center that the AI will look around while patrolling",
    EV_GETTER
);
Event EV_Actor_SetLookAroundAngle
(
    "lookaroundangle",
    EV_DEFAULT,
    "f",
    "angle",
    "gets the angle in degrees left or right of center that the AI will look around while patrolling",
    EV_SETTER
);
Event EV_Actor_SetLookAroundAngle2
(
    "lookaroundangle",
    EV_DEFAULT,
    "f",
    "angle",
    "gets the angle in degrees left or right of center that the AI will look around while patrolling",
    EV_NORMAL
);
Event EV_Actor_HasCompleteLookahead
(
    "hascompletelookahead",
    EV_DEFAULT,
    NULL,
    NULL,
    "returns true if there are no corners to turn on the rest of the AI's current path",
    EV_GETTER
);
Event EV_Actor_PathDist
(
    "pathdist",
    EV_DEFAULT,
    NULL,
    NULL,
    "returns total distance along current path to the path goal",
    EV_GETTER
);
Event EV_Actor_CanShootEnemyFrom
(
    "canshootenemyfrom",
    EV_DEFAULT,
    "v",
    "shootOrigin",
    "Determines if it would be possible to shoot the sentient's enemy from the given position.",
    EV_RETURN
);
Event EV_Actor_CanShoot
(
    "canshoot",
    EV_DEFAULT,
    "v",
    "shootOrigin",
    "Determines if it would be possible to shoot the sentient's enemy from the given position.",
    EV_RETURN
);
Event EV_Actor_GetInReload
(
    "inreload",
    EV_DEFAULT,
    NULL,
    NULL,
    "returns non-zero if the AI is in a reload",
    EV_GETTER
);
Event EV_Actor_SetInReload
(
    "inreload",
    EV_DEFAULT,
    "i",
    "reloading",
    "set to non-zero to indicate the AI is in a reload",
    EV_SETTER
);
Event EV_Actor_SetReloadCover(
    "setreloadcover",
    EV_DEFAULT,
    NULL,
    NULL,
    "do this command to let the ai know it needs to reload; used to reload while going to cover",
    EV_NORMAL
);
Event EV_Actor_BreakSpecial
(
    "breakspecial",
    EV_DEFAULT,
    NULL,
    NULL,
    "tell ai to break special attack",
    EV_NORMAL
);
Event EV_Actor_SetBalconyHeight
(
    "balconyheight",
    EV_DEFAULT,
    "f",
    "height",
    "minimum height a balcony guy must fall to do special balcony death",
    EV_NORMAL
);
Event EV_Actor_SetBalconyHeight2
(
    "balconyheight",
    EV_DEFAULT,
    "f",
    "height",
    "minimum height a balcony guy must fall to do special balcony death",
    EV_SETTER
);
Event EV_Actor_GetBalconyHeight
(
    "balconyheight",
    EV_DEFAULT,
    NULL,
    NULL,
    "minimum height a balcony guy must fall to do special balcony death",
    EV_GETTER
);
Event EV_Actor_SetVisibilityThreshold
(
    "nonvislevel",
    EV_DEFAULT,
    NULL,
    NULL,
    "visibility level in range 0-1 below which an enemy is treated as non-visible",
    EV_NORMAL
);
Event EV_Actor_SetVisibilityThreshold2
(
    "nonvislevel",
    EV_DEFAULT,
    NULL,
    NULL,
    "visibility level in range 0-1 below which an enemy is treated as non-visible",
    EV_SETTER
);
Event EV_Actor_GetVisibilityThreshold
(
    "nonvislevel",
    EV_DEFAULT,
    NULL,
    NULL,
    "visibility level in range 0-1 below which an enemy is treated as non-visible",
    EV_GETTER
);
Event EV_Actor_SetDefaultVisibilityThreshold
(
    "defaultnonvislevel",
    EV_DEFAULT,
    NULL,
    NULL,
    "updates the default value for 'nonvislevel'",
    EV_NORMAL
);
Event EV_Actor_SetSuppressChance
(
    "suppresschance",
    EV_DEFAULT,
    NULL,
    NULL,
    "sets the percent chance of doing suppressing fire when appropriate (0-100)",
    EV_NORMAL
);
Event EV_Actor_SetSuppressChance2
(
    "suppresschance",
    EV_DEFAULT,
    NULL,
    NULL,
    "sets the percent chance of doing suppressing fire when appropriate (0-100)",
    EV_SETTER
);
Event EV_Actor_GetSuppressChance
(
    "suppresschance",
    EV_DEFAULT,
    NULL,
    NULL,
    "sets the percent chance of doing suppressing fire when appropriate (0-100)",
    EV_GETTER
);
Event EV_Actor_SetIgnoreBadPlace
(
    "ignorebadplaces",
    EV_DEFAULT,
    "i",
    "ignore",
    "sets whether or not this AI guy will ignore bad places (0 = not suicidal)",
    EV_NORMAL
);
Event EV_Actor_SetIgnoreBadPlace2
(
    "ignorebadplaces",
    EV_DEFAULT,
    "i",
    "ignore",
    "sets whether or not this AI guy will ignore bad places (0 = not suicidal)",
    EV_SETTER
);
Event EV_Actor_GetIgnoreBadPlace
(
    "ignorebadplaces",
    EV_DEFAULT,
    NULL,
    NULL,
    "gets whether or not this AI guy will ignore bad places (0 = not suicidal)",
    EV_GETTER
);
Event EV_Actor_FindEnemy
(
    "findenemy",
    EV_DEFAULT,
    NULL,
    NULL,
    "Finds the best enemy to target",
    EV_RETURN
);
Event EV_Actor_DisableEnemySwitch
(
    "enemyswitchdisable",
    EV_DEFAULT,
    NULL,
    NULL,
    "Disable enemy switching...",
    EV_NORMAL
);
Event EV_Actor_EnableEnemySwitch
(
    "enemyswitchenable",
    EV_DEFAULT,
    NULL,
    NULL,
    "Enable enemy switching...",
    EV_NORMAL
);
Event EV_Actor_SetRunAnimRate
(
    "runanimrate",
    EV_DEFAULT,
    "f",
    "multiplier",
    "Set the rate at which the run animation plays back",
    EV_SETTER
);
Event EV_Actor_SetRunAnimRate2
(
    "runanimrate",
    EV_DEFAULT,
    "f",
    "multiplier",
    "Set the rate at which the run animation plays back",
    EV_NORMAL
);
Event EV_Actor_GetRunAnimRate
(
    "runanimrate",
    EV_DEFAULT,
    NULL,
    NULL,
    "Get the rate at which the run animation plays back",
    EV_GETTER
);
Event EV_Actor_SetNationality
(
    "nationality",
    EV_DEFAULT,
    "s",
    "nationality",
    "Set the nationality of an actor. Valid entries are default, ger, it, usa, uk, and ussr.",
    EV_NORMAL
);
Event EV_Actor_SetNationality2
(
    "nationality",
    EV_DEFAULT,
    "s",
    "nationality",
    "Set the nationality of an actor. Valid entries are default, ger, it, usa, uk, and ussr.",
    EV_SETTER
);
Event EV_Actor_GetNationality
(
    "nationality",
    EV_DEFAULT,
    NULL,
    NULL,
    "Get the nationality of an actor. Return values are ger, it, usa, uk, ussr and unset.",
    EV_GETTER
);
Event EV_Actor_WriteStats
(
    "writestats",
    EV_DEFAULT,
    NULL,
    NULL,
    "Used internally to write stats to a CSV file.",
    EV_NORMAL
);
Event EV_Actor_CuriousOff
(
    "curiousoff",
    EV_DEFAULT,
    NULL,
    NULL,
    "Turn off an actor's curious state",
    EV_NORMAL
);
Event EV_Actor_CuriousOn
(
    "curiouson",
    EV_DEFAULT,
    NULL,
    NULL,
    "Turn on an actor's curious state",
    EV_NORMAL
);

CLASS_DECLARATION(SimpleActor, Actor, "Actor") {
    {&EV_Entity_Start,                        &Actor::EventStart                              },
    {&EV_Actor_MoveTo,                        &Actor::MoveTo                                  },
    {&EV_Actor_WalkTo,                        &Actor::WalkTo                                  },
    {&EV_Actor_RunTo,                         &Actor::RunTo                                   },
    {&EV_Actor_CrouchTo,                      &Actor::CrouchTo                                },
    {&EV_Actor_CrawlTo,                       &Actor::CrawlTo                                 },
    {&EV_Actor_AimAt,                         &Actor::AimAt                                   },
    {&EV_Pain,                                &Actor::EventPain                               },
    {&EV_Killed,                              &Actor::EventKilled                             },
    {&EV_Actor_DeathEmbalm,                   &Actor::DeathEmbalm                             },
    {&EV_Actor_Anim,                          &Actor::PlayAnimation                           },
    {&EV_Actor_AnimLoop,                      &Actor::PlayAnimation                           },
    {&EV_Actor_Anim_Scripted,                 &Actor::PlayScriptedAnimation                   },
    {&EV_Actor_Anim_Noclip,                   &Actor::PlayNoclipAnimation                     },
    {&EV_Actor_Anim_Attached,                 &Actor::PlayAttachedAnimation                   },
    {&EV_Actor_AnimScript,                    &Actor::EventAnimScript                         },
    {&EV_Actor_AnimScript_Scripted,           &Actor::EventAnimScript_Scripted                },
    {&EV_Actor_AnimScript_Noclip,             &Actor::EventAnimScript_Noclip                  },
    {&EV_Actor_AnimScript_Attached,           &Actor::EventAnimScript_Attached                },
    {&EV_Actor_Reload_mg42,                   &Actor::EventReload_mg42                        },
    {&EV_Actor_Dumb,                          &Actor::Dumb                                    },
    {&EV_Actor_Physics_On,                    &Actor::PhysicsOn                               },
    {&EV_Actor_Physics_Off,                   &Actor::PhysicsOff                              },
    {&EV_Actor_SetAnim,                       &Actor::EventSetAnim                            },
    {&EV_Actor_SetAnimLength,                 &SimpleActor::EventSetAnimLength                },
    {&EV_Actor_EndActionAnim,                 &Actor::EventEndActionAnim                      },
    {&EV_Actor_SetMotionAnim,                 &Actor::EventSetMotionAnim                      },
    {&EV_Actor_SetAimMotionAnim,              &Actor::EventSetAimMotionAnim                   },
    {&EV_Actor_SetActionAnim,                 &Actor::EventSetActionAnim                      },
    {&EV_Actor_UpperAnim,                     &Actor::EventUpperAnim                          },
    {&EV_Actor_SetUpperAnim,                  &Actor::EventSetUpperAnim                       },
    {&EV_Actor_SetCrossblendTime,             &SimpleActor::EventSetCrossblendTime            },
    {&EV_Actor_GetCrossblendTime,             &SimpleActor::EventGetCrossblendTime            },
    {&EV_Actor_GetPosition,                   &SimpleActor::EventGetPosition                  },
    {&EV_Actor_SetPosition,                   &SimpleActor::EventSetPosition                  },
    {&EV_Actor_SetEmotion,                    &SimpleActor::EventSetEmotion                   },
    {&EV_Actor_SetAnimFinal,                  &SimpleActor::EventSetAnimFinal                 },
    {&EV_Actor_GetWeaponType,                 &SimpleActor::EventGetWeaponType                },
    {&EV_Actor_GetWeaponGroup,                &SimpleActor::EventGetWeaponGroup               },
    {&EV_Actor_LookAt,                        &Actor::EventLookAt                             },
    {&EV_Actor_EyesLookAt,                    &Actor::EventEyesLookAt                         },
    {&EV_Actor_PointAt,                       &Actor::EventPointAt                            },
    {&EV_Actor_TurnTo,                        &Actor::EventTurnTo                             },
    {&EV_Actor_SetTurnDoneError,              &Actor::EventSetTurnDoneError                   },
    {&EV_Actor_SetTurnDoneError2,             &Actor::EventSetTurnDoneError                   },
    {&EV_Actor_GetTurnDoneError,              &Actor::EventGetTurnDoneError                   },
    {&EV_Actor_IdleSayAnim,                   &Actor::EventIdleSayAnim                        },
    {&EV_Actor_SayAnim,                       &Actor::EventSayAnim                            },
    {&EV_Actor_SetSayAnim,                    &Actor::EventSetSayAnim                         },
    {&EV_Sentient_UseItem,                    &Actor::EventGiveWeapon                         },
    {&EV_Sentient_GiveWeapon,                 &Actor::EventGiveWeapon                         },
    {&EV_Actor_SetWeapon,                     &Actor::EventGiveWeapon                         },
    {&EV_Actor_GetWeapon,                     &Actor::EventGetWeapon                          },
    {&EV_Actor_SetGun,                        &Actor::EventGiveWeapon                         },
    {&EV_Actor_SetGun2,                       &Actor::EventGiveWeapon                         },
    {&EV_Actor_GetGun,                        &Actor::EventGetWeapon                          },
    {&EV_Actor_WeaponInternal,                &Actor::EventGiveWeaponInternal                 },
    {&EV_Actor_DamagePuff,                    &Actor::EventDamagePuff                         },
    {&EV_Actor_SetAngleYawSpeed,              &Actor::EventSetAngleYawSpeed                   },
    {&EV_Actor_SetAngleYawSpeed2,             &Actor::EventSetAngleYawSpeed                   },
    {&EV_Actor_GetAngleYawSpeed,              &Actor::EventGetAngleYawSpeed                   },
    {&EV_Actor_SetAimTarget,                  &Actor::EventSetAimTarget                       },
    {&EV_Actor_ReadyToFire,                   &Actor::ReadyToFire                             },
    {&EV_Actor_AIOn,                          &SimpleActor::EventAIOn                         },
    {&EV_Actor_AIOff,                         &SimpleActor::EventAIOff                        },
    {&EV_DeathSinkStart,                      &Actor::DeathSinkStart                          },
    {&EV_Actor_GetSight,                      &Actor::EventGetSight                           },
    {&EV_Actor_SetSight,                      &Actor::EventSetSight                           },
    {&EV_Actor_SetSight2,                     &Actor::EventSetSight                           },
    {&EV_Actor_GetHearing,                    &Actor::EventGetHearing                         },
    {&EV_Actor_SetHearing,                    &Actor::EventSetHearing                         },
    {&EV_Actor_SetHearing2,                   &Actor::EventSetHearing                         },
    {&EV_Actor_SetFov,                        &Actor::EventSetFov                             },
    {&EV_Actor_SetFov2,                       &Actor::EventSetFov                             },
    {&EV_Actor_GetFov,                        &Actor::EventGetFov                             },
    {&EV_Actor_DistToEnemy,                   &Actor::EventDistToEnemy                        },
    {&EV_Actor_SetPatrolPath,                 &Actor::EventSetPatrolPath                      },
    {&EV_Actor_SetPatrolPath2,                &Actor::EventSetPatrolPath                      },
    {&EV_Actor_GetPatrolPath,                 &Actor::EventGetPatrolPath                      },
    {&EV_Actor_SetPatrolWaitTrigger,          &Actor::EventSetPatrolWaitTrigger               },
    {&EV_Actor_SetPatrolWaitTrigger2,         &Actor::EventSetPatrolWaitTrigger               },
    {&EV_Actor_GetPatrolWaitTrigger,          &Actor::EventGetPatrolWaitTrigger               },
    {&EV_Actor_SetAccuracy,                   &Actor::EventSetAccuracy                        },
    {&EV_Actor_SetAccuracy2,                  &Actor::EventSetAccuracy                        },
    {&EV_Actor_GetAccuracy,                   &Actor::EventGetAccuracy                        },
    {&EV_Actor_SetTypeIdle,                   &Actor::EventSetTypeIdle                        },
    {&EV_Actor_SetTypeIdle2,                  &Actor::EventSetTypeIdle                        },
    {&EV_Actor_GetTypeIdle,                   &Actor::EventGetTypeIdle                        },
    {&EV_Actor_SetTypeAttack,                 &Actor::EventSetTypeAttack                      },
    {&EV_Actor_SetTypeAttack2,                &Actor::EventSetTypeAttack                      },
    {&EV_Actor_GetTypeAttack,                 &Actor::EventGetTypeAttack                      },
    {&EV_Actor_SetTypeDisguise,               &Actor::EventSetTypeDisguise                    },
    {&EV_Actor_SetTypeDisguise2,              &Actor::EventSetTypeDisguise                    },
    {&EV_Actor_GetTypeDisguise,               &Actor::EventGetTypeDisguise                    },
    {&EV_Actor_SetDisguiseLevel,              &Actor::EventSetDisguiseLevel                   },
    {&EV_Actor_SetDisguiseLevel2,             &Actor::EventSetDisguiseLevel                   },
    {&EV_Actor_GetDisguiseLevel,              &Actor::EventGetDisguiseLevel                   },
    {&EV_Actor_SetTypeGrenade,                &Actor::EventSetTypeGrenade                     },
    {&EV_Actor_SetTypeGrenade2,               &Actor::EventSetTypeGrenade                     },
    {&EV_Actor_GetTypeGrenade,                &Actor::EventGetTypeGrenade                     },
    {&EV_Actor_SetMinDistance,                &Actor::EventSetMinDistance                     },
    {&EV_Actor_SetMinDistance2,               &Actor::EventSetMinDistance                     },
    {&EV_Actor_GetMinDistance,                &Actor::EventGetMinDistance                     },
    {&EV_Actor_SetMaxDistance,                &Actor::EventSetMaxDistance                     },
    {&EV_Actor_SetMaxDistance2,               &Actor::EventSetMaxDistance                     },
    {&EV_Actor_GetMaxDistance,                &Actor::EventGetMaxDistance                     },
    {&EV_Actor_GetLeash,                      &Actor::EventGetLeash                           },
    {&EV_Actor_SetLeash,                      &Actor::EventSetLeash                           },
    {&EV_Actor_SetLeash2,                     &Actor::EventSetLeash                           },
    {&EV_Actor_GetInterval,                   &Actor::EventGetInterval                        },
    {&EV_Actor_SetInterval,                   &Actor::EventSetInterval                        },
    {&EV_Actor_SetInterval2,                  &Actor::EventSetInterval                        },
    {&EV_Actor_GetRunAnim,                    &Actor::EventGetRunAnim                         },
    {&EV_Actor_GetWalkAnim,                   &Actor::EventGetWalkAnim                        },
    {&EV_Actor_GetAnimName,                   &Actor::EventGetAnimName                        },
    {&EV_Actor_SetAnimName,                   &Actor::EventSetAnimName                        },
    {&EV_Actor_SetDisguiseRange,              &Actor::EventSetDisguiseRange                   },
    {&EV_Actor_SetDisguiseRange2,             &Actor::EventSetDisguiseRange                   },
    {&EV_Actor_GetDisguiseRange,              &Actor::EventGetDisguiseRange                   },
    {&EV_Actor_SetDisguisePeriod,             &Actor::EventSetDisguisePeriod                  },
    {&EV_Actor_SetDisguisePeriod2,            &Actor::EventSetDisguisePeriod                  },
    {&EV_Actor_GetDisguisePeriod,             &Actor::EventGetDisguisePeriod                  },
    {&EV_Actor_SetDisguiseAcceptThread,       &Actor::EventSetDisguiseAcceptThread            },
    {&EV_Actor_SetDisguiseAcceptThread2,      &Actor::EventSetDisguiseAcceptThread            },
    {&EV_Actor_GetDisguiseAcceptThread,       &Actor::EventGetDisguiseAcceptThread            },
    {&EV_Actor_AttackPlayer,                  &Actor::EventAttackPlayer                       },
    {&EV_Actor_SetAlarmNode,                  &Actor::EventSetAlarmNode                       },
    {&EV_Actor_SetAlarmNode2,                 &Actor::EventSetAlarmNode                       },
    {&EV_Actor_GetAlarmNode,                  &Actor::EventGetAlarmNode                       },
    {&EV_Actor_SetPreAlarmThread,             &Actor::EventSetPreAlarmThread                  },
    {&EV_Actor_SetPreAlarmThread2,            &Actor::EventSetPreAlarmThread                  },
    {&EV_Actor_SetAlarmThread,                &Actor::EventSetAlarmThread                     },
    {&EV_Actor_SetAlarmThread2,               &Actor::EventSetAlarmThread                     },
    {&EV_Actor_GetAlarmThread,                &Actor::EventGetAlarmThread                     },
    {&EV_Actor_SetSoundAwareness,             &Actor::EventSetSoundAwareness                  },
    {&EV_Actor_SetSoundAwareness2,            &Actor::EventSetSoundAwareness                  },
    {&EV_Actor_GetSoundAwareness,             &Actor::EventGetSoundAwareness                  },
    {&EV_Actor_SetGrenadeAwareness,           &Actor::EventSetGrenadeAwareness                },
    {&EV_Actor_SetGrenadeAwareness2,          &Actor::EventSetGrenadeAwareness                },
    {&EV_Actor_GetGrenadeAwareness,           &Actor::EventGetGrenadeAwareness                },
    {&EV_Actor_SetTurret,                     &Actor::EventSetTurret                          },
    {&EV_Actor_SetTurret2,                    &Actor::EventSetTurret                          },
    {&EV_Actor_GetTurret,                     &Actor::EventGetTurret                          },
    {&EV_Actor_AttachGrenade,                 &Actor::Grenade_EventAttach                     },
    {&EV_Actor_DetachGrenade,                 &Actor::Grenade_EventDetach                     },
    {&EV_Actor_FireGrenade,                   &Actor::Grenade_EventFire                       },
    {&EV_Actor_EnableEnemy,                   &Actor::EventEnableEnemy                        },
    {&EV_Actor_EnablePain,                    &Actor::EventEnablePain                         },
    {&EV_Activate,                            &Actor::EventActivate                           },
    {&EV_Actor_GetAmmoGrenade,                &Actor::EventGetAmmoGrenade                     },
    {&EV_Actor_SetAmmoGrenade,                &Actor::EventSetAmmoGrenade                     },
    {&EV_Actor_SetAmmoGrenade2,               &Actor::EventSetAmmoGrenade                     },
    {&EV_Actor_GetMood,                       &Actor::EventGetMood                            },
    {&EV_Actor_SetMood,                       &Actor::EventSetMood                            },
    {&EV_Actor_ShareEnemy,                    &Actor::EventShareEnemy                         },
    {&EV_Actor_ShareGrenade,                  &Actor::EventShareGrenade                       },
    {&EV_Actor_InterruptPoint,                &Actor::EventInterruptPoint                     },
    {&EV_Actor_GetVisibilityThreshold,        &Actor::EventGetVisibilityThreshold             },
    {&EV_Actor_SetVisibilityThreshold,        &Actor::EventSetVisibilityThreshold             },
    {&EV_Actor_SetVisibilityThreshold2,       &Actor::EventSetVisibilityThreshold             },
    {&EV_Actor_SetDefaultVisibilityThreshold, &Actor::EventSetDefaultVisibilityThreshold      },
    {&EV_Actor_GetSuppressChance,             &Actor::EventGetSuppressChance                  },
    {&EV_Actor_SetSuppressChance,             &Actor::EventSetSuppressChance                  },
    {&EV_Actor_SetSuppressChance2,            &Actor::EventSetSuppressChance                  },
    {&EV_Actor_GetPainHandler,                &Actor::EventGetPainHandler                     },
    {&EV_Actor_SetPainHandler,                &Actor::EventSetPainHandler                     },
    {&EV_Actor_GetDeathHandler,               &Actor::EventGetDeathHandler                    },
    {&EV_Actor_SetDeathHandler,               &Actor::EventSetDeathHandler                    },
    {&EV_Actor_GetAttackHandler,              &Actor::EventGetAttackHandler                   },
    {&EV_Actor_SetAttackHandler,              &Actor::EventSetAttackHandler                   },
    {&EV_Actor_SetHeadModel,                  &Actor::EventSetHeadModel                       },
    {&EV_Actor_GetHeadModel,                  &Actor::EventGetHeadModel                       },
    {&EV_Actor_SetHeadSkin,                   &Actor::EventSetHeadSkin                        },
    {&EV_Actor_GetHeadSkin,                   &Actor::EventGetHeadSkin                        },
    {&EV_Actor_SetNoIdle,                     &Actor::EventSetNoIdle                          },
    {&EV_Actor_GetNoIdle,                     &Actor::EventGetNoIdle                          },
    {&EV_Actor_GetEnemy,                      &Actor::EventGetEnemy                           },
    {&EV_Actor_GetMaxNoticeTimeScale,         &Actor::EventGetMaxNoticeTimeScale              },
    {&EV_Actor_SetMaxNoticeTimeScale,         &Actor::EventSetMaxNoticeTimeScale              },
    {&EV_Actor_SetMaxNoticeTimeScale2,        &Actor::EventSetMaxNoticeTimeScale              },
    {&EV_Actor_GetFixedLeash,                 &Actor::EventGetFixedLeash                      },
    {&EV_Actor_SetFixedLeash,                 &Actor::EventSetFixedLeash                      },
    {&EV_Actor_SetFixedLeash2,                &Actor::EventSetFixedLeash                      },
    {&EV_Actor_Holster,                       &Actor::EventHolster                            },
    {&EV_Actor_Unholster,                     &Actor::EventUnholster                          },
    {&EV_SoundDone,                           &Actor::EventSoundDone                          },
    {&EV_Sound,                               &Actor::EventSound                              },
    {&EV_Actor_IsEnemyVisible,                &Actor::EventIsEnemyVisible                     },
    {&EV_Actor_GetEnemyVisibleChangeTime,     &Actor::EventGetEnemyVisibleChangeTime          },
    {&EV_Actor_GetLastEnemyVisibleTime,       &Actor::EventGetLastEnemyVisibleTime            },
    {&EV_Actor_SetFallHeight,                 &Actor::EventSetFallHeight                      },
    {&EV_Actor_GetFallHeight,                 &Actor::EventGetFallHeight                      },
    {&EV_Actor_CanMoveTo,                     &Actor::EventCanMoveTo                          },
    {&EV_Actor_MoveDir,                       &Actor::EventMoveDir                            },
    {&EV_Actor_IntervalDir,                   &Actor::EventIntervalDir                        },
    {&EV_Actor_ResetLeash,                    &Actor::EventResetLeash                         },
    {&EV_Actor_Tether,                        &Actor::EventTether                             },
    {&EV_Actor_GetThinkState,                 &Actor::EventGetThinkState                      },
    {&EV_Actor_GetEnemyShareRange,            &Actor::EventGetEnemyShareRange                 },
    {&EV_Actor_SetEnemyShareRange,            &Actor::EventSetEnemyShareRange                 },
    {&EV_Actor_SetEnemyShareRange2,           &Actor::EventSetEnemyShareRange                 },
    {&EV_Actor_GetVoiceType,                  &Actor::GetVoiceType                            },
    {&EV_Actor_SetVoiceType,                  &Actor::SetVoiceType                            },
    {&EV_Actor_SetVoiceType2,                 &Actor::SetVoiceType                            },
    {&EV_Remove,                              &Actor::Remove                                  },
    {&EV_Delete,                              &Actor::Remove                                  },
    {&EV_ScriptRemove,                        &Actor::Remove                                  },
    {&EV_Actor_KickDir,                       &Actor::EventGetKickDir                         },
    {&EV_Actor_GetNoLongPain,                 &Actor::EventGetNoLongPain                      },
    {&EV_Actor_SetNoLongPain,                 &Actor::EventSetNoLongPain                      },
    {&EV_Actor_SetNoLongPain2,                &Actor::EventSetNoLongPain                      },
    {&EV_Actor_GetFavoriteEnemy,              &Actor::EventGetFavoriteEnemy                   },
    {&EV_Actor_SetFavoriteEnemy,              &Actor::EventSetFavoriteEnemy                   },
    {&EV_Actor_SetFavoriteEnemy2,             &Actor::EventSetFavoriteEnemy                   },
    {&EV_Actor_GetMumble,                     &Actor::EventGetMumble                          },
    {&EV_Actor_SetMumble,                     &Actor::EventSetMumble                          },
    {&EV_Actor_SetMumble2,                    &Actor::EventSetMumble                          },
    // FIXME: removed since 2.0?
    //{&EV_Actor_GetBreathSteam,                &Actor::EventGetBreathSteam                     },
    //{&EV_Actor_SetBreathSteam,                &Actor::EventSetBreathSteam                     },
    //{&EV_Actor_SetBreathSteam2,               &Actor::EventSetBreathSteam                     },
    {&EV_Actor_CalcGrenadeToss,               &Actor::EventCalcGrenadeToss                    },
    {&EV_Actor_CalcGrenadeToss2,              &Actor::EventCalcGrenadeToss                    },
    {&EV_Actor_GetNoSurprise,                 &Actor::EventGetNoSurprise                      },
    {&EV_Actor_SetNoSurprise,                 &Actor::EventSetNoSurprise                      },
    {&EV_Actor_SetNoSurprise2,                &Actor::EventSetNoSurprise                      },
    {&EV_Actor_GetSilent,                     &Actor::EventGetSilent                          },
    {&EV_Actor_SetSilent,                     &Actor::EventSetSilent                          },
    {&EV_Actor_SetSilent2,                    &Actor::EventSetSilent                          },
    {&EV_Actor_GetAvoidPlayer,                &Actor::EventGetAvoidPlayer                     },
    {&EV_Actor_SetAvoidPlayer,                &Actor::EventSetAvoidPlayer                     },
    {&EV_Actor_SetAvoidPlayer2,               &Actor::EventSetAvoidPlayer                     },
    {&EV_Actor_SetMoveDoneRadius,             &Actor::EventSetMoveDoneRadius                  },
    {&EV_Actor_GetMoveDoneRadius,             &Actor::EventGetMoveDoneRadius                  },
    {&EV_Actor_BeDead,                        &Actor::EventBeDead                             },
    {&EV_Actor_GetLookAroundAngle,            &Actor::EventGetLookAroundAngle                 },
    {&EV_Actor_SetLookAroundAngle,            &Actor::EventSetLookAroundAngle                 },
    {&EV_Actor_SetLookAroundAngle2,           &Actor::EventSetLookAroundAngle                 },
    {&EV_Actor_HasCompleteLookahead,          &Actor::EventHasCompleteLookahead               },
    {&EV_Actor_PathDist,                      &Actor::EventPathDist                           },
    {&EV_Actor_CanShootEnemyFrom,             &Actor::EventCanShootEnemyFrom                  },
    {&EV_Actor_CanShoot,                      &Actor::EventCanShoot                           },
    {&EV_Actor_GetInReload,                   &Actor::EventGetInReload                        },
    {&EV_Actor_SetInReload,                   &Actor::EventSetInReload                        },
    {&EV_Actor_SetReloadCover,                &Actor::EventSetReloadCover                     },
    {&EV_Actor_BreakSpecial,                  &Actor::EventBreakSpecial                       },
    {&EV_Actor_SetBalconyHeight,              &Actor::EventSetBalconyHeight                   },
    {&EV_Actor_SetBalconyHeight2,             &Actor::EventSetBalconyHeight                   },
    {&EV_Actor_GetBalconyHeight,              &Actor::EventGetBalconyHeight                   },
    {&EV_Actor_SetIgnoreBadPlace,             &Actor::EventSetIgnoreBadPlace                  },
    {&EV_Actor_SetIgnoreBadPlace2,            &Actor::EventSetIgnoreBadPlace                  },
    {&EV_Actor_GetIgnoreBadPlace,             &Actor::EventGetIgnoreBadPlace                  },
    {&EV_Actor_FindEnemy,                     &Actor::EventFindEnemy                          },
    {&EV_Actor_EnableEnemySwitch,             &Actor::EventEnableEnemySwitch                  },
    {&EV_Actor_DisableEnemySwitch,            &Actor::EventDisableEnemySwitch                 },
    {&EV_Actor_SetRunAnimRate,                &Actor::EventSetRunAnimRate                     },
    {&EV_Actor_SetRunAnimRate2,               &Actor::EventSetRunAnimRate                     },
    {&EV_Actor_GetRunAnimRate,                &Actor::EventGetRunAnimRate                     },
    {&EV_Stop,                                &Actor::Landed                                  },
    {&EV_Actor_SetNationality,                &Actor::SetNationality                          },
    {&EV_Actor_SetNationality2,               &Actor::SetNationality                          },
    {&EV_Actor_GetNationality,                &Actor::GetNationality                          },
    {&EV_Actor_WriteStats,                    &Actor::EventWriteStats                         },
    {&EV_Actor_CuriousOff,                    &Actor::EventCuriousOff                         },
    {&EV_Actor_CuriousOn,                     &Actor::EventCuriousOn                          },
    {NULL,                                    NULL                                            }
};

Actor::GlobalFuncs_t Actor::GlobalFuncs[NUM_THINKS];
const_str            Actor::m_csThinkNames[NUM_THINKS] = {
    STRING_VOID,
    STRING_TURRET,
    STRING_COVER,
    STRING_PATROL,
    STRING_RUNNER,
    STRING_PAIN,
    STRING_KILLED,
    STRING_MOVETO,
    STRING_IDLE,
    STRING_CURIOUS,
    STRING_DISGUISE_SALUTE,
    STRING_DISGUISE_SENTRY,
    STRING_DISGUISE_OFFICER,
    STRING_DISGUISE_ROVER,
    STRING_DISGUISE_NONE,
    STRING_ALARM,
    STRING_GRENADE,
    STRING_MACHINEGUNNER,
    STRING_DOG_IDLE,
    STRING_DOG_ATTACK,
    STRING_DOG_CURIOUS,
    STRING_DOG_GRENADE,
    STRING_ANIM,
    STRING_ANIM_CURIOUS,
    STRING_AIM,
    STRING_BALCONY_IDLE,
    STRING_BALCONY_CURIOUS,
    STRING_BALCONY_ATTACK,
    STRING_BALCONY_DISGUISE,
    STRING_BALCONY_GRENADE,
    STRING_BALCONY_PAIN,
    STRING_BALCONY_KILLED,
    STRING_WEAPONLESS,
    STRING_NOCLIP,
    STRING_DEAD};
const_str Actor::m_csThinkStateNames[NUM_THINKSTATES] = {
    STRING_VOID,
    STRING_IDLE,
    STRING_PAIN,
    STRING_KILLED,
    STRING_ATTACK,
    STRING_CURIOUS,
    STRING_DISGUISE,
    STRING_GRENADE,
    STRING_NOCLIP};

SafePtr<Actor> Actor::mBodyQueue[MAX_BODYQUEUE];
int            Actor::mCurBody;

/*
===============
Actor::Actor

Constructor
===============
*/
Actor::Actor()
    : mVoiceType(-1)
{
    entflags |= EF_ACTOR;

    m_pszDebugState = "";
    m_pFallPath = NULL;
    m_iOriginTime = -1;

    if (LoadingSavegame) {
        return;
    }

    m_PainState = 500;
    m_iEnemyShowPapersTime = 0;
    m_eGrenadeState = eGrenadeState::AI_GRENSTATE_FLEE;

    edict->s.eType = ET_MODELANIM;
    edict->r.svFlags |= SVF_MONSTER;
    edict->r.ownerNum = ENTITYNUM_NONE;
    edict->clipmask   = MASK_MONSTERSOLID;
    setMoveType(MOVETYPE_WALK);

    mass = 175;
    flags |= FL_THINK;
    flags &= ~FL_ANIMATE;

    setContentsSolid();
    setSolidType(SOLID_BBOX);

    m_fFov      = 90.0f;
    takedamage  = DAMAGE_AIM;
    m_fFovDot   = cos(DEG2RAD(m_fFov / 2.0));

    m_eAnimMode = ANIM_MODE_NONE;
    Anim_Emotion(EMOTION_NONE);

    m_bDoPhysics          = true;
    path_failed_time      = 0;

    health                = 100;
    max_health            = 100;

    m_fAngleYawSpeed      = 360;
    m_fHeadMaxTurnSpeed   = 520;
    m_bHeadAnglesAchieved = true;
    VectorClear(m_vHeadDesiredAngles);

    m_fLUpperArmTurnSpeed      = 360;
    m_bLUpperArmAnglesAchieved = true;
    VectorClear(m_vLUpperArmDesiredAngles);

    m_fTorsoMaxTurnSpeed     = 120;
    m_fTorsoCurrentTurnSpeed = 0;
    m_bTorsoAnglesAchieved   = true;
    VectorClear(m_vTorsoDesiredAngles);

    m_pLookEntity  = NULL;
    m_pPointEntity = NULL;
    m_pTurnEntity  = NULL;

    VectorClear(edict->s.eyeVector);

    m_bDoAI                 = true;
    m_bTurretNoInitialCover = false;
    m_bThink                = false;
    m_fSight                = world->m_fAIVisionDistance;
    m_fHearing              = 2048;
    m_PainTime              = 0;
    m_bNoLongPain           = false;
    hit_obstacle_time       = 0;
    VectorClear2D(obstacle_vel);

    m_bLockThinkState       = false;
    m_bAutoAvoidPlayer      = true;
    m_bIsCurious            = true;

    InitThinkStates();
    SetThinkState(THINKSTATE_IDLE, THINKLEVEL_NORMAL);

    m_fMinDistance        = 128;
    m_fMinDistanceSquared = Square(m_fMinDistance);
    m_fMaxDistance        = 1024;
    m_fMaxDistanceSquared = Square(m_fMaxDistanceSquared);
    m_fLeash              = 512;
    m_fLeashSquared       = Square(m_fLeashSquared);

    m_iEyeUpdateTime      = level.inttime;
    if (m_iEyeUpdateTime < 1000) {
        m_iEyeUpdateTime = 1000;
    }
    m_iEyeUpdateTime += rand() % 100 + 100;

    m_vIntervalDir            = vec_zero;
    m_iIntervalDirTime        = 0;
    edict->r.lastNetTime      = -10000000;
    
    m_bNewEnemy               = false;
    m_iEnemyCheckTime         = 0;
    m_iEnemyChangeTime        = 0;
    m_bEnemyIsDisguised       = false;
    m_bEnemyVisible           = false;
    m_iEnemyVisibleCheckTime  = 0;
    m_iEnemyVisibleChangeTime = 0;
    m_iLastEnemyVisibleTime   = 0;
    m_fVisibilityAlpha        = 0;
    m_fVisibilityThreshold    = 0.5f;
    m_bHasVisibilityThreshold = false;

    m_bEnemyInFOV             = false;
    m_iEnemyFovCheckTime      = 0;
    m_iEnemyFovChangeTime     = 0;
    m_iIgnoreSoundsMask       = 0;

    m_State                   = -1;
    m_iStateTime              = 0;

    m_iGunPositionCheckTime   = 0;
    m_vLastEnemyPos           = vec3_origin;
    m_iLastEnemyPosChangeTime = 0;
    m_vScriptGoal             = vec3_origin;
    m_bScriptGoalValid        = false;
    m_iNextWatchStepTime      = 0;

    m_iCuriousTime            = 0;
    m_iCuriousLevel           = 0;
    SetCuriousAnimHint(0);

    m_bNoSurprise      = false;
    memset(&m_pPotentialCoverNode, 0, sizeof(m_pPotentialCoverNode));
    m_iPotentialCoverCount    = 0;
    m_pCoverNode              = NULL;

    m_csSpecialAttack         = STRING_NULL;
    m_bNeedReload             = false;
    m_bInReload               = false;
    m_bFaceEnemy              = true;
    m_sCurrentPathNodeIndex   = -1;

    m_iNextDisguiseTime       = 1;
    m_iDisguisePeriod         = 30000;
    m_fMaxDisguiseDistSquared = 256 * 256;
    m_iDisguiseLevel          = 1;

    m_patrolCurrentNode       = NULL;
    m_csPatrolCurrentAnim     = STRING_ANIM_RUN_SCR;
    m_bPatrolWaitTrigger      = false;
    m_fInterval               = 128;
    m_iSquadStandTime         = 0;
    m_aimNode                 = NULL;

    m_vHome                   = vec3_origin;
    m_bFixedLeash             = false;
    m_pTetherEnt              = NULL;

    m_pGrenade                = NULL;
    m_vGrenadePos             = vec3_origin;
    m_iFirstGrenadeTime       = 0;
    m_bGrenadeBounced         = false;
    m_eGrenadeMode            = AI_GREN_TOSS_NONE;
    m_vGrenadeVel             = vec_zero;
    m_vKickDir                = vec_zero;
    
    m_fSoundAwareness         = 100;
    m_fGrenadeAwareness       = 20;
    m_fMaxNoticeTimeScale     = 1.0f;
    m_fNoticeTimeScale        = 1.0f;

    m_iCanShootCheckTime      = 0;
    m_bCanShootEnemy          = false;
    m_bEnableEnemy            = true;
    m_bDesiredEnableEnemy     = true;
    m_bEnablePain             = true;

    m_csAnimScript            = STRING_ANIM_IDLE_SCR;
    m_bAnimScriptSet          = false;
    m_AnimMode                = ANIM_MODE_NORMAL;
    m_csAnimName              = STRING_EMPTY;
    m_csSayAnim               = STRING_EMPTY;
    m_csUpperAnim             = STRING_EMPTY;

    m_fRunAnimRate            = 1;
    m_fDfwRequestedYaw        = 0;
    m_fDfwDerivedYaw          = 0;
    m_vDfwPos                 = vec_zero;
    m_fDfwTime                = 0;

    m_AlarmNode               = NULL;

    m_iWallDodgeTimeout       = 0;
    m_WallDir                 = 0;
    VectorClear2D(m_PrevObstacleNormal);
    
    m_csHeadModel             = STRING_EMPTY;
    m_csHeadSkin              = STRING_EMPTY;
    m_bNoIdleAfterAnim        = false;
    m_csWeapon                = STRING_EMPTY;
    m_csLoadOut               = STRING_EMPTY;

    m_fMaxShareDistSquared    = 0;
    m_iRunHomeTime            = 0;
    m_iSuppressChance         = 50;

    m_bBreathSteam           = false;

    m_fMoveDoneRadiusSquared = 0;

    m_bForceAttackPlayer     = false;

    m_iLookFlags              = 0;
    m_iNextLookTime          = 0;
    m_fLookAroundFov         = 45;
    m_bHasDesiredLookDest    = false;
    m_bHasDesiredLookAngles  = false;

    m_eDontFaceWallMode       = 0;
    m_iLastFaceDecideTime    = 0;
    m_iOriginTime            = -1;
    m_fTurnDoneError         = 0;

    m_fBalconyHeight         = 128;
    m_bNoPlayerCollision     = false;
    for (int i = 0; i < MAX_ORIGIN_HISTORY; i++) {
        VectorClear2D(m_vOriginHistory[i]);
    }
    m_iCurrentHistory = 0;
    m_bAnimating = false;
    m_bIgnoreBadPlace = false;
    m_bBecomeRunner = false;
    m_bEnemySwitch = true;
    m_iNationality = 0;

    if (g_aistats) {
        PostEvent(EV_Actor_WriteStats, 1.0);
    }

    mVoiceType        = -1;
}

/*
===============
Actor::~Actor

Destructor
===============
*/
Actor::~Actor()
{
    EndStates();
    ClearPatrolCurrentNode();
    ClearAimNode();
    ClearLookEntity();
    ClearPointEntity();
    ClearTurnEntity();

    entflags &= ~EF_ACTOR;
}

/*
===============
Actor::setContentsSolid

===============
*/
void Actor::setContentsSolid(void)
{
    setContents(CONTENTS_NOBOTCLIP);
}

void Actor::EndStates(void)
{
    for (int i = 0; i < NUM_THINKLEVELS; i++) {
        EndState(i);
    }
}

void Actor::ClearStates(void)
{
    for (int i = 0; i < NUM_THINKSTATES; i++) {
        SetThink((eThinkState)i, THINK_VOID);
    }
}

void Actor::ResolveVoiceType(void)
{
    char validVoice[128];

    if (mVoiceType != -1) {
        if (m_Team == TEAM_AMERICAN) {
            for (int i = 0; i < 3; i++) {
                if (gAmericanVoices[i][0] == mVoiceType) {
                    return;
                }
            }

            validVoice[0] = '\0';
            for (int i = 0; i < 3; i++) {
                strcat(validVoice, gAmericanVoices[i]);
                strcat(validVoice, " ");
            }
            Com_Printf("ERROR: Bad voice type %c.  Valid American voicetypes are: %s\n", mVoiceType, validVoice);
        } else {
            for (int i = 0; i < 3; i++) {
                if (gGermanVoices[i][0] == mVoiceType) {
                    return;
                }
            }

            validVoice[0] = '\0';
            for (int i = 0; i < 3; i++) {
                strcat(validVoice, gGermanVoices[i]);
                strcat(validVoice, " ");
            }
            Com_Printf("ERROR: Bad voice type %c.  Valid German voicetypes are: %s\n", mVoiceType, validVoice);
        }

        mVoiceType = -1;
    }

    if (mVoiceType == -1) {
        int d = random() * 3.0 * 0.99;

        if (m_Team == TEAM_AMERICAN) {
            mVoiceType = gAmericanVoices[d][0];
        } else {
            mVoiceType = gGermanVoices[d][0];
        }
    }
}

/*
===============
Actor::EventStart

Initialize actor.
===============
*/
void Actor::EventStart(Event *ev)
{
    ResolveVoiceType();
    setSize(MINS, MAXS); //notsure

    droptofloor(16384);

    SetControllerTag(HEAD_TAG, gi.Tag_NumForName(edict->tiki, "Bip01 Head"));
    SetControllerTag(TORSO_TAG, gi.Tag_NumForName(edict->tiki, "Bip01"));
    SetControllerTag(ARMS_TAG, gi.Tag_NumForName(edict->tiki, "Bip01 L UpperArm"));

    JoinNearbySquads(1024);

    if (level.Spawned()) {
        Unregister(STRING_SPAWN);
    }
}

void Actor::ClearEnemies(void)
{
    m_PotentialEnemies.RemoveAll();
    SetEnemy(NULL, false);
}

/*
===============
Actor::SetMoveInfo

Copies current move information to mm.
===============
*/
void Actor::SetMoveInfo(mmove_t *mm)
{
    memset(mm, 0, sizeof(mmove_t));

    velocity.copyTo(mm->velocity);
    origin.copyTo(mm->origin);

    mm->entityNum = entnum;

    mm->walking = m_walking;
    mm->groundPlane = m_groundPlane;
    VectorCopy(m_groundPlaneNormal, mm->groundPlaneNormal);

    mm->frametime = level.frametime;

    if (m_bNoPlayerCollision) {
        mm->tracemask = MASK_TARGETPATH;
    } else {
        mm->tracemask = MASK_PATHSOLID;
    }

    VectorCopy(MINS, mm->mins);
    VectorCopy(MAXS, mm->maxs);
    mm->maxs[2] = 94;
}

/*
===============
Actor::DoFailSafeMove

===============
*/
void Actor::DoFailSafeMove(vec3_t dest)
{
    Com_Printf("(entnum %d, radnum %d) blocked, doing failsafe\n", entnum, radnum);

    VectorCopy(dest, m_NoClipDest);

    SetThinkState(THINKSTATE_NOCLIP, THINKLEVEL_NOCLIP);
}

/*
===============
Actor::GetMoveInfo

Fetch current move information from mm.
===============
*/
void Actor::GetMoveInfo(mmove_t *mm)
{
    trace_t trace;

    m_walking           = mm->walking;
    m_groundPlane       = mm->groundPlane;
    VectorCopy(mm->groundPlaneNormal, m_groundPlaneNormal);
    groundentity        = NULL;

    TouchStuff(mm);
    
    switch (m_eAnimMode) {
    case ANIM_MODE_DEST:
        if (!mm->hit_temp_obstacle && mm->hit_obstacle) {
            trace = G_Trace(
                m_Dest,
                PLAYER_BASE_MIN,
                PLAYER_BASE_MAX,
                m_Dest - Vector(0, 0, 16384),
                (Entity*)NULL,
                MASK_MOVEINFO,
                qfalse,
                "Actor::GetMoveInfo"
            );

            DoFailSafeMove(trace.endpos);
        } else if (mm->hit_temp_obstacle & 1) {
            Player* p;

            p = static_cast<Player*>(G_GetEntity(0));

            if (!IsTeamMate(p)) {
                if (!m_bEnableEnemy) {
                    m_bDesiredEnableEnemy = true;
                    UpdateEnableEnemy();
                }

                BecomeTurretGuy();
                ForceAttackPlayer();
            }
        }
        break;
    case ANIM_MODE_PATH:
    case ANIM_MODE_PATH_GOAL:
        if (mm->hit_temp_obstacle) {
            if (mm->hit_temp_obstacle & 1) {
                Player* p;

                m_Path.Clear();
                p = static_cast<Player*>(G_GetEntity(0));

                if (!IsTeamMate(p)) {
                    if (!m_bEnableEnemy) {
                        m_bDesiredEnableEnemy = true;
                        UpdateEnableEnemy();
                    }

                    BecomeTurretGuy();
                    ForceAttackPlayer();
                }
            }

            VectorClear(velocity);
        } else if (mm->hit_obstacle) {
            if (level.inttime >= m_Path.Time() + 1000) {
                m_Path.ReFindPath(origin, this);
            } else {
                PathInfo* node = m_Path.NextNode();

                if (!node || !m_Path.IsAccurate() && (node == m_Path.LastNode() && m_Path.CurrentNode() != m_Path.StartNode())) {
                    m_Path.Clear();
                    VectorClear(velocity);
                    return;
                }

                if (m_Path.CurrentNode() == m_Path.StartNode()) {
                    Com_Printf(
                        "^~^~^ BAD FAILSAFE: %.2f %.2f %.2f, '%s'\n",
                        origin[0],
                        origin[1],
                        origin[2],
                        targetname.c_str()
                    );
                }

                DoFailSafeMove(node->point);
            }
        } else if (DotProduct(mm->velocity, velocity) < -0.7f && level.inttime >= m_Path.Time() + 1000) {
            m_Path.ReFindPath(origin, this);
        }
        break;
    }

    setOrigin(mm->origin);

    if (VectorLengthSquared(mm->velocity) < 1) {
        velocity = vec_zero;
    } else {
        velocity = mm->velocity;
    }
}

/*
===============
Actor::TouchStuff

===============
*/
void Actor::TouchStuff(mmove_t *mm)
{
    int        i, j;
    gentity_t *other;
    Event     *event;

    if (getMoveType() != MOVETYPE_NOCLIP) {
        G_TouchTriggers(this);
    }

    for (i = 0; i < mm->numtouch; i++) {
        other = &g_entities[mm->touchents[i]];

        for (j = 0; j < i; j++) {
            if (&g_entities[j] == other) {
                break;
            }
        }

        if (j != i) {
            // duplicated
            continue;
        }

        // Don't bother touching the world
        if (!other->entity || other->entity == world) {
            continue;
        }

        event = new Event(EV_Touch);
        event->AddEntity(this);
        other->entity->ProcessEvent(event);

        event = new Event(EV_Touch);
        event->AddEntity(other->entity);
        ProcessEvent(event);
    }
}

/*
===============
Actor::ResetBoneControllers

Reset Bone Controllers.
===============
*/
void Actor::ResetBoneControllers(void)
{
    if (edict->s.bone_tag[HEAD_TAG] != -1) {
        SetControllerAngles(HEAD_TAG, vec_zero);
    }
    if (edict->s.bone_tag[TORSO_TAG] != -1) {
        SetControllerAngles(TORSO_TAG, vec_zero);
    }
    if (edict->s.bone_tag[ARMS_TAG] != -1) {
        SetControllerAngles(ARMS_TAG, vec_zero);
    }
}

/*
===============
Actor::UpdateBoneControllers

Update Bone Controllers.
Reversed incorrectly, since decompiled output has different behaviour as reversed output.
===============
*/
void Actor::UpdateBoneControllers(void)
{
    float pitchError, yawError, error;
    float max_change, max_accel_change, min_accel_change;
    Vector headAngles, torsoAngles;
    float tolerable_error;
    vec3_t LocalLookAngles;
    float torsoYawError;

    if (m_bHasDesiredLookAngles) {
        vec3_t myAxis[3];
        vec3_t eyePosition;
        vec3_t lookAxis[3];
        vec3_t endPosition;
        vec3_t worldLookAngles;
        vec3_t lineVec;
        int tagnum;
        orientation_t tag_or;
        Vector vError;

        if (g_showlookat->integer == entnum || g_showlookat->integer == -1)
        {
            tagnum = gi.Tag_NumForName(edict->tiki, "eyes bone");
            tag_or = G_TIKI_Orientation(edict, tagnum & TAG_MASK);
            AnglesToAxis(angles, myAxis);

            VectorCopy(origin, eyePosition);

            for (int i = 0; i < 3; i++) {
                VectorMA(eyePosition, tag_or.origin[i], myAxis[i], eyePosition);
            }

            Vector vEndPoint;

            if (m_bHasDesiredLookDest) {
                vEndPoint = m_vDesiredLookDest;
            } else if (m_pLookEntity) {
                if (m_pLookEntity->IsSubclassOfSentient()) {
                    Sentient* sent = static_cast<Sentient*>(m_pLookEntity.Pointer());
                    vEndPoint = sent->EyePosition();
                } else {
                    vEndPoint = m_pLookEntity->centroid;
                }
            }

            for (int i = 0; i < 3; i++) {
                worldLookAngles[i] = m_DesiredLookAngles[i] + angles[i];
            }

            AnglesToAxis(worldLookAngles, lookAxis);

            Vector vEndPointDir = vEndPoint - eyePosition;
            vEndPointDir.normalize();

            // calculate the error
            Vector vError = vEndPointDir - lookAxis[0];

            if (vError.length() >= 0.1f) {
                VectorSet(lineVec, 128, 0, 0);
                VectorCopy(eyePosition, endPosition);

                for (int i = 0; i < 3; i++) {
                    VectorMA(endPosition, lineVec[i], lookAxis[i], endPosition);
                }
            } else {
                int boxalpha;

                VectorCopy(vEndPoint, endPosition);

                if (vError.length() >= 0.04f) {
                    boxalpha = (vError.length() - 0.1f) / 0.06f;
                } else {
                    boxalpha = 1;
                }

                G_DebugLine(vEndPoint + Vector(8, 8, 8), vEndPoint + Vector(8, 8, -8), 0, 0, boxalpha, boxalpha);
                G_DebugLine(vEndPoint + Vector(8, -8, 8), vEndPoint + Vector(8, -8, -8), 0, 0, boxalpha, boxalpha);
                G_DebugLine(vEndPoint + Vector(-8, 8, 8), vEndPoint + Vector(-8, 8, -8), 0, 0, boxalpha, boxalpha);
                G_DebugLine(vEndPoint + Vector(-8, -8, 8), vEndPoint + Vector(-8, -8, -8), 0, 0, boxalpha, boxalpha);
                G_DebugLine(vEndPoint + Vector(8, 8, 8), vEndPoint + Vector(8, -8, 8), 0, 0, boxalpha, boxalpha);
                G_DebugLine(vEndPoint + Vector(8, 8, -8), vEndPoint + Vector(8, -8, -8), 0, 0, boxalpha, boxalpha);
                G_DebugLine(vEndPoint + Vector(-8, 8, 8), vEndPoint + Vector(-8, -8, 8), 0, 0, boxalpha, boxalpha);
                G_DebugLine(vEndPoint + Vector(-8, 8, -8), vEndPoint + Vector(-8, -8, -8), 0, 0, boxalpha, boxalpha);
                G_DebugLine(vEndPoint + Vector(8, 8, 8), vEndPoint + Vector(-8, 8, 8), 0, 0, boxalpha, boxalpha);
                G_DebugLine(vEndPoint + Vector(8, 8, -8), vEndPoint + Vector(-8, 8, -8), 0, 0, boxalpha, boxalpha);
                G_DebugLine(vEndPoint + Vector(8, -8, 8), vEndPoint + Vector(-8, -8, 8), 0, 0, boxalpha, boxalpha);
                G_DebugLine(vEndPoint + Vector(8, -8, -8), vEndPoint + Vector(-8, -8, -8), 0, 0, boxalpha, boxalpha);
            }
            G_DebugLine(eyePosition, endPosition, 0, 0, 1, 1);

            VectorSet(lineVec, 0, 0, -2);
            for (int i = 0; i < 3; i++) {
                VectorMA(eyePosition, lineVec[i], lookAxis[i], eyePosition);
                VectorMA(endPosition, lineVec[i], lookAxis[i], endPosition);
            }

            G_DebugLine(eyePosition, endPosition, 0, 0, 1, 1);

            VectorSet(lineVec, 0, 2, 0);
            for (int i = 0; i < 3; i++) {
                VectorMA(eyePosition, lineVec[i], lookAxis[i], eyePosition);
                VectorMA(endPosition, lineVec[i], lookAxis[i], endPosition);
            }

            G_DebugLine(eyePosition, endPosition, 0, 0, 1, 1);
        }

        VectorCopy(m_DesiredLookAngles, LocalLookAngles);

        Vector EyeDelta;
        Vector fwd;

        EyeDelta = LocalLookAngles;
        EyeDelta.AngleVectorsLeft(&fwd, NULL, NULL);
        VectorCopy(fwd, edict->s.eyeVector);
    } else {
        VectorClear(edict->s.eyeVector);
        VectorClear(LocalLookAngles);
    }

    torsoAngles = GetControllerAngles(TORSO_TAG);
    headAngles = GetControllerAngles(HEAD_TAG);

    yawError = LocalLookAngles[1] - (torsoAngles[1] + headAngles[1]);
    pitchError = LocalLookAngles[0] - headAngles[0];
    torsoYawError = LocalLookAngles[1] - torsoAngles[1];

    if (torsoYawError > 180) {
        torsoYawError -= 360;
    } else if (torsoYawError < -180) {
        torsoYawError += 360;
    }

    if (torsoYawError >= 0) {
        if (yawError < -120) {
            yawError += 360;
        }
    } else {
        if (yawError > 120) {
            yawError -= 360;
        }
    }

    if (!(m_iLookFlags & 1)) {
        if (yawError > 25) {
            max_change = (yawError - 25) * 0.2f;
        } else if (yawError < -25) {
            max_change = (yawError + 25) * 0.2f;
        } else {
            max_change = 0;
        }

        tolerable_error = yawError + headAngles[1];
        if (-torsoAngles[1] < tolerable_error) {
            max_change = Q_clamp_float(max_change, -torsoAngles[1], tolerable_error);
        } else {
            max_change = Q_clamp_float(max_change, tolerable_error, -torsoAngles[1]);
        }

        max_accel_change = m_fTorsoCurrentTurnSpeed + level.frametime * 15;
        min_accel_change = m_fTorsoCurrentTurnSpeed - level.frametime * 15;
    
        max_change = Q_clamp_float(max_change, min_accel_change, max_accel_change);
        max_change = Q_clamp_float(max_change, level.frametime * -m_fTorsoCurrentTurnSpeed, level.frametime * m_fTorsoCurrentTurnSpeed);

        torsoAngles[1] += max_change;

        if (torsoAngles[1] > 30) {
            max_change -= torsoAngles[1] - 30;
            torsoAngles[1] = 30;
        } else if (torsoAngles[1] < -30) {
            max_change -= torsoAngles[1] + 30;
            torsoAngles[1] = -30;
        }

        m_fTorsoCurrentTurnSpeed = max_change;

        // clear pitch/roll
        torsoAngles[0] = 0;
        torsoAngles[2] = 0;

        SetControllerAngles(TORSO_TAG, torsoAngles);
        yawError -= max_change;
    }

    if (yawError > 15.f) {
        max_change = (yawError - 15.f) * level.frametime * 4.f;
    } else if (yawError < -15.f) {
        max_change = (yawError + 15.f) * level.frametime * 4.f;
    } else {
        max_change = 0;
    }

    if (-headAngles[1] < yawError) {
        max_change = Q_clamp_float(max_change, -headAngles[1], yawError);
    } else {
        max_change = Q_clamp_float(max_change, yawError, -headAngles[1]);
    }

    max_change = Q_clamp_float(max_change, level.frametime * -m_fHeadMaxTurnSpeed, level.frametime * m_fHeadMaxTurnSpeed);
    
    headAngles[1] += max_change;
    // clamp head yaw to 60 degrees
    headAngles[1] = Q_clamp_float(headAngles[1], -60, 60);

    error = 15.f;

    if (fabs(LocalLookAngles[0]) < 15) {
        error = fabs(LocalLookAngles[0]);
    }

    if (pitchError > error) {
        max_change = (pitchError - error) * 0.2f;
    } else if (pitchError < -error) {
        max_change = (pitchError + error) * 0.2f;
    } else {
        max_change = 0;
    }

    max_change = Q_clamp_float(max_change, level.frametime * -m_fHeadMaxTurnSpeed, level.frametime * m_fHeadMaxTurnSpeed);
    headAngles[0] += max_change;
    // clamp head pitch to 35 degrees
    headAngles[0] = Q_clamp_float(headAngles[0], -35, 35);
    headAngles[2] = 0;

    if (m_iLookFlags & 1) {
        // half view
        headAngles[0] /= 2.f;
        headAngles[1] /= 2.f;
    }

    SetControllerAngles(HEAD_TAG, headAngles);

    Vector tmp_angles;
    Vector new_angles;

    // arms
    tmp_angles = GetControllerAngles(ARMS_TAG);

    max_change = m_vLUpperArmDesiredAngles[1] - tmp_angles[1];
    if (max_change > 180) {
        max_change -= 360;
    } else if (max_change < -180) {
        max_change += 360;
    }

    max_change = Q_clamp_float(max_change, level.frametime * -m_fLUpperArmTurnSpeed, level.frametime * m_fLUpperArmTurnSpeed);
    // set the new arms angles
    new_angles[0] = m_vLUpperArmDesiredAngles[0];
    new_angles[1] = tmp_angles[1] + max_change;
    new_angles[2] = 0;
    SetControllerAngles(ARMS_TAG, new_angles);
}

void Actor::setOriginEvent(Vector org)
{
    float dist;
    bool bRejoin = false;

    dist = (org - origin).lengthSquared();

    if (dist > Square(DEFAULT_NEARBY_SQUAD_DIST)) {
        bRejoin = true;
        DisbandSquadMate(this);
    }

    m_iOriginTime = level.inttime;

    VectorCopy2D(org, m_vOriginHistory[0]);
    VectorCopy2D(org, m_vOriginHistory[1]);
    VectorCopy2D(org, m_vOriginHistory[2]);
    VectorCopy2D(org, m_vOriginHistory[3]);

    VectorClear(velocity);

    if (level.Spawned()) {
        SafeSetOrigin(org);
    } else {
        setOrigin(org);
    }

    m_vHome = origin;

    if (bRejoin) {
        JoinNearbySquads(DEFAULT_NEARBY_SQUAD_DIST);
    }

    if (dist > Square(MIN_BADPLACE_UPDATE_DIST)) {
        UpdateBadPlaces();
    }
}

/*
===============
Actor::SafeSetOrigin

Safe set origin.
===============
*/
void Actor::SafeSetOrigin(vec3_t newOrigin)
{
    // don't update the origin if it's the same
    if (newOrigin == origin) {
        return;
    }

    setOrigin(newOrigin);

    if (!m_bNoPlayerCollision) {
        Player *p = (Player *)G_GetEntity(0);

        if (p && IsTouching(p)) {
            Com_Printf("(entnum %d, radnum %d) is going not solid to not get stuck in the player\n", entnum, radnum);
            m_bNoPlayerCollision = true;
            setSolidType(SOLID_NOT);
        }
    }
}

/*
===============
Actor::DoMove

Move the actor based on m_eAnimMode.
===============
*/
void Actor::DoMove(void)
{
    mmove_t mm;
    trace_t trace;

    if (m_eAnimMode != ANIM_MODE_ATTACHED && (!m_bDoPhysics || m_iOriginTime == level.inttime || m_pGlueMaster || bindmaster)) {
        return;
    }

    switch (m_eAnimMode) {
    case ANIM_MODE_NORMAL:
        SetMoveInfo(&mm);

        VectorCopy2D(frame_delta, mm.desired_dir);
        mm.desired_speed = VectorNormalize2D(mm.desired_dir) / level.frametime;

        if (mm.desired_speed > m_maxspeed) {
            mm.desired_speed = m_maxspeed;
        }

        MmoveSingle(&mm);
        GetMoveInfo(&mm);
        break;
    case ANIM_MODE_PATH:
        MovePath(frame_delta.length() / level.frametime);
        break;
    case ANIM_MODE_PATH_GOAL:
        MovePathGoal(frame_delta.length() / level.frametime);
        break;
    case ANIM_MODE_DEST:
        MoveDest(frame_delta.length() / level.frametime);
        break;
    case ANIM_MODE_SCRIPTED:
        trace = G_Trace(
            origin, mins, maxs, origin + frame_delta, this, edict->clipmask & ~MASK_SCRIPT_SLAVE, qtrue, "Actor"
        );
        SafeSetOrigin(trace.endpos);
        velocity = frame_delta / level.frametime;
        break;
    case ANIM_MODE_NOCLIP:
        SafeSetOrigin(origin + frame_delta);
        velocity = frame_delta / level.frametime;
        break;
    case ANIM_MODE_FALLING_PATH:
        SafeSetOrigin(m_pFallPath->pos[m_pFallPath->currentPos]);
        m_pFallPath->currentPos++;
        velocity = vec_zero;
        break;
    case ANIM_MODE_ATTACHED:
    {
        Vector frame_offset;
        Vector attach_angles;

        frame_offset = edict->s.attach_offset;
        frame_offset += frame_delta;
        frame_offset.copyTo(edict->s.attach_offset);
        setOrigin();

        attach_angles = angles;
        attach_angles[1] += angular_delta;
        setAngles(attach_angles);

        velocity = frame_delta / level.frametime;
        break;
    }
    default:
        break;
    }

    m_maxspeed = 1000000.0f;
}

/*
===============
Actor::UpdateAngles

Update current angles.
===============
*/
void Actor::UpdateAngles(void)
{
    float max_change, error, dist;

    if (m_YawAchieved) {
        return;
    }

    error = m_DesiredYaw - angles[1];
    if (error > 180) {
        error -= 360;
    } else if (error < -180) {
        error += 360;
    }

    dist = m_fAngleYawSpeed * level.frametime;
    if (error < -dist) {
        max_change = -dist;
    } else if (error > dist) {
        max_change = dist;
    } else {
        max_change = error;
        StopTurning();
    }

    setAngles(Vector(0, angles[1] + max_change, 0));
}

/*
===============
Actor::SetLeashHome

Set leash home.
===============
*/
void Actor::SetLeashHome(Vector vHome)
{
    if (!m_bFixedLeash) {
        m_vHome = vHome;
    }
}

/*
===============
Actor::GunPosition

Returns current gun position.
===============
*/
Vector Actor::GunPosition(void)
{
    if (m_iGunPositionCheckTime < level.inttime) {
        m_iGunPositionCheckTime = level.inttime;
        Weapon *weap            = GetActiveWeapon(WEAPON_MAIN);
        if (weap) {
            weap->GetMuzzlePosition(m_vGunPosition);
        } else {
            m_vGunPosition = vec_zero;
        }
    }
    return m_vGunPosition;
}

/*
===============
Actor::CanSee

Returns true if actor cansee entity through fov and vision_distance.
===============
*/
bool Actor::CanSee(Entity *e1, float fov, float vision_distance, bool bNoEnts)
{
    bool bCanSee = Sentient::CanSee(e1, fov, vision_distance, false);
    if (e1 == m_Enemy) {
        m_iEnemyVisibleCheckTime = level.inttime;
        // Added in 2.0.
        //  Check for obfuscation like smoke sprite
        m_fVisibilityAlpha = 0;
        if (bCanSee) {
            m_fVisibilityAlpha = 1.0f - G_VisualObfuscation(VirtualEyePosition(), m_Enemy->EyePosition());
            if (m_fVisibilityAlpha < m_fVisibilityThreshold) {
                // enemy is obfuscated behind something
                bCanSee = false;
            }
        }

        if (bCanSee) {
            SetEnemyPos(m_Enemy->origin);
            m_bEnemyIsDisguised     = m_Enemy->m_bIsDisguised;
            m_iLastEnemyVisibleTime = level.inttime;
        }

        if (bCanSee != m_bEnemyVisible) {
            m_bEnemyVisible           = !m_bEnemyVisible;
            m_iEnemyVisibleChangeTime = level.inttime;
        }

        if (fov != 0) {
            m_iEnemyFovCheckTime = level.inttime;
            if (bCanSee != m_bEnemyInFOV) {
                m_bEnemyInFOV         = !m_bEnemyInFOV;
                m_iEnemyFovChangeTime = level.inttime;
            }
        }
    }

    return bCanSee;
}

/*
===============
Actor::WithinVisionDistance

Returns true if entity is witthin vision distance.
===============
*/
bool Actor::WithinVisionDistance(Entity *ent) const
{
    float distance = world->farplane_distance;

    if (!distance || distance < m_fSight) {
        return WithinDistance(ent, m_fSight);
    }

    return WithinDistance(ent, distance);
}

/*
===============
Actor::InFOV

Returns true if positin is within fov.
===============
*/
bool Actor::InFOV(Vector pos, float check_fov, float check_fovdot)
{
    if (check_fov == 360) {
        return true;
    }

    Vector delta = pos - VirtualEyePosition();
    if (!delta.x && !delta.y) {
        return true;
    }

    float fDot = DotProduct2D(delta, orientation[0]);
    if (fDot < 0) {
        return false;
    }

    return Square(fDot) > (delta.lengthXYSquared() * Square(check_fovdot));
}

/*
===============
Actor::CanShoot

Returns true if actor can shoot entity.
===============
*/
bool Actor::CanShoot(Entity *ent)
{
    bool bCanShoot = false;
    Vector vGunPos;

    if (FriendlyInLineOfFire(ent)) {
        bCanShoot = false;
    } else if (ent->IsSubclassOfSentient()) {
        Sentient* sen = static_cast<Sentient*>(ent);

        vGunPos = GunPosition();
        bCanShoot = false;

        if (WithinFarplaneDistance(origin - ent->origin) && AreasConnected(ent)) {
            if (G_SightTrace(
                vGunPos,
                vec_zero,
                vec_zero,
                sen->centroid,
                this,
                sen,
                MASK_CANSEE,
                qfalse,
                "Actor::CanShoot centroid"
            )) {
                bCanShoot = true;
            } else if (G_SightTrace(
                vGunPos,
                vec_zero,
                vec_zero,
                sen->EyePosition(),
                this,
                sen,
                MASK_CANSEE,
                qfalse,
                "Actor::CanShoot eyes"
            )) {
                bCanShoot = true;
            }
        }
    } else {
        bCanShoot = false;
        if (AreasConnected(ent)) {
            vGunPos = GunPosition();
            bCanShoot = CanSeeFrom(vGunPos, ent);
        }
    }

    if (ent == m_Enemy) {
        m_iCanShootCheckTime = level.inttime;
        m_bCanShootEnemy     = bCanShoot;
    }

    return bCanShoot;
}

/*
===============
Actor::CanSeeFrom

Returns true if actor can see entity from pos.
===============
*/
bool Actor::CanSeeFrom(vec3_t pos, Entity *ent)
{
    if (!WithinFarplaneDistance(pos)) {
        // outside the farplane
        return false;
    }

    if (ent->flags & FL_NOTARGET) {
        // ent must be ignored by AI
        return false;
    }

    return G_SightTrace(pos, vec_zero, vec_zero, ent->centroid, this, ent, MASK_CANSEE, qfalse, "Actor::CanSeeFrom");
}

/*
===============
Actor::EnemyInFOV

Returns true if enemy is within fov.
===============
*/
bool Actor::EnemyInFOV(int iMaxDirtyTime)
{
    if (level.inttime > iMaxDirtyTime + m_iEnemyFovCheckTime) {
        if (m_bEnemyInFOV != InFOV(m_Enemy->centroid)) {
            m_bEnemyInFOV = !m_bEnemyInFOV;
            m_iEnemyFovChangeTime = level.inttime;
        }
        m_iEnemyFovCheckTime = level.inttime;
    }

    return m_bEnemyInFOV;
}

/*
===============
Actor::CanSeeEnemy

Returns true if actor can see enemy.
===============
*/
bool Actor::CanSeeEnemy(int iMaxDirtyTime)
{
    if (level.inttime > iMaxDirtyTime + m_iEnemyVisibleCheckTime) {
        CanSee(m_Enemy, 0, world->farplane_distance * 0.828f, false);
    }

    return m_bEnemyVisible;
}

/*
===============
Actor::CanShootEnemy

Returns true if actor can shoot enemy.
===============
*/
bool Actor::CanShootEnemy(int iMaxDirtyTime)
{
    if (level.inttime > iMaxDirtyTime + m_iCanShootCheckTime) {
        CanShoot(m_Enemy);
    }

    return m_bCanShootEnemy;
}

bool Actor::FriendlyInLineOfFire(Entity *other)
{
    Vector delta;
    float inverseDot;

    delta = other->origin - origin;
    inverseDot = 1.0 / (delta * delta);

    for (Sentient* pSquad = m_pNextSquadMate; pSquad != this; pSquad = pSquad->m_pNextSquadMate) {
        Vector squadDelta;
        float squadDot;

        squadDelta = pSquad->origin - origin;
        squadDot = squadDelta * delta;
        if (squadDot >= 0) {
            Vector org;

            org = squadDot * inverseDot * delta - squadDelta;
            if (org * org >= 4096) {
                return true;
            }
        }
    }

    return false;
}

Vector Actor::VirtualEyePosition()
{
    if (m_pTurret && CurrentThink() == THINK_MACHINEGUNNER) {
        // return the turret eye position
        return m_pTurret->EyePosition();
    } else {
        return EyePosition();
    }
}

/*
===============
Actor::CanTarget

Actor can target.
===============
*/
bool Actor::CanTarget(void)
{
    return true;
}

/*
===============
Actor::IsImmortal

Actor is immortal ?
===============
*/
bool Actor::IsImmortal(void)
{
    return false;
}

/*
===============
Actor::ShowInfo

Prints basic actor information.
===============
*/
void Actor::ShowInfo(void)
{
    Com_Printf("-------------------------------------------------------------------------------\n");
    Com_Printf("Info for Actor:\n");
    Com_Printf("Current think type: %s %s\n", ThinkName().c_str(), ThinkStateName().c_str());
    Com_Printf("leash: %f\n", m_fLeash);
    Com_Printf("mindist: %f\n", m_fMinDistance);
    Com_Printf("maxdist: %f\n", m_fMaxDistance);

    GlobalFuncs_t *func = &GlobalFuncs[CurrentThink()];
    if (func->ShowInfo) {
        (this->*func->ShowInfo)();
    }

    Com_Printf("-------------------------------------------------------------------------------\n");
}

/*
===============
Actor::ShowInfo

Display actor info.
===============
*/
void Actor::ShowInfo(float fDot, float fDist)
{
    float          fMaxDist, fMinDot;
    static cvar_t *g_entinfo_max;

    if (!bEntinfoInit) {
        g_entinfo_max = gi.Cvar_Get("g_entinfo_max", "2048", 0);
        bEntinfoInit  = true;
    }

    fMinDot  = 0.9f;
    fMaxDist = g_entinfo_max->value;

    if (IsKilledState(m_ThinkState)) {
        fMinDot  = 0.99f;
        fMaxDist = 512;
    }

    if (g_entinfo->integer != 4) {
        if (fDot <= fMinDot) {
            if (fDot <= 0 || fDist >= 256) {
                return;
            }
        }

        if (fMaxDist <= fDist || fDist <= 64) {
            return;
        }
    }
    if (!m_bEnableEnemy) {
        Vector a = origin;
        a.z += maxs.z + 74;

        G_DebugString(a, 1.0, 1.0, 1.0, 1.0, "DISABLED");
    }

    {
        Vector a = origin;
        a.z += maxs.z + 56;

        G_DebugString(a, 1.0, 1.0, 1.0, 1.0, "%i:%i:%s:%.1f", entnum, radnum, targetname.c_str(), health);
    }

    if (m_Enemy) {
        Vector a = origin;
        a.z += maxs.z + 38;

        G_DebugString(
            a,
            1.0,
            1.0,
            1.0,
            1.0,
            "%i:%s:%.1f:%i",
            m_Enemy->entnum,
            m_Enemy->targetname.c_str(),
            m_Enemy->health,
            m_PotentialEnemies.GetCurrentThreat()
        );
    }

    if (IsCuriousState(m_ThinkState)) {
        Vector a = origin;
        a.z += maxs.z + 20;

        G_DebugString(
            a,
            1.0,
            1.0,
            1.0,
            1.0,
            "%s:%%%.1f",
            Director.GetString(m_csThinkStateNames[THINKSTATE_CURIOUS]).c_str(),
            m_PotentialEnemies.GetCurrentVisibility() * 100
        );
    } else {
        Vector a = origin;
        a.z += maxs.z + 20;

        G_DebugString(a, 1.0, 1.0, 1.0, 1.0, "%s", ThinkStateName().c_str());
    }

    {
        Vector a = origin;
        a.z += maxs.z + 2;

        str sAnimThreadFile = m_pAnimThread ? m_pAnimThread->FileName() : "(null)";

        G_DebugString(
            a,
            1.0,
            1.0,
            1.0,
            1.0,
            "%s:%s:%i - %s",
            ThinkName().c_str(),
            m_pszDebugState,
            m_State,
            sAnimThreadFile.c_str()
        );
    }

    if (g_entinfo->integer == 1) {
        if (m_pGrenade) {
            G_DebugLine(m_pGrenade->origin, m_vGrenadePos, 1.0, 0.0, 0.0, 1.0);
        }

        Vector a = m_vHome;
        a.z += maxs.z + 18;

        G_DebugLine(centroid, a, 0.0, 1.0, 0.0, 1.0);

        G_DebugCircle(a, m_fLeash, 0.0, 1.0, 0.0, 1.0, qtrue);

        G_DebugCircle(centroid, m_fMinDistance, 1.0, 0.0, 0.0, 1.0, qtrue);

        G_DebugCircle(centroid, m_fMaxDistance, 0.0, 0.0, 1.0, 1.0, qtrue);
    } else if (g_entinfo->integer == 2) {
        G_DebugCircle(centroid, m_fHearing, 1.0, 0.0, 0.0, 1.0, qtrue);
        G_DebugCircle(centroid, m_fSight, 0.0, 0.0, 1.0, 1.0, qtrue);
    }
}

str Actor::ThinkName(void) const
{
    return Director.GetString(m_csThinkNames[CurrentThink()]);
}

str Actor::ThinkStateName(void) const
{
    return Director.GetString(m_csThinkStateNames[m_ThinkState]);
}

/*
===============
Actor::MoveTo

Move actor to specific location/listener with specific animation.
===============
*/
void Actor::MoveTo(Event *ev)
{
    m_csPatrolCurrentAnim = ev->GetConstString(1);

    if (ev->IsVectorAt(2)) {
        Vector vec = ev->GetVector(2);
        SetPatrolCurrentNode(vec);
    } else {
        SetPatrolCurrentNode(ev->GetListener(2));
    }

    if (m_patrolCurrentNode) {
        m_vScriptGoal      = origin;
        m_bScriptGoalValid = true;
    }

    SetThinkIdle(THINK_RUNNER);
}

/*
===============
Actor::WalkTo

Walk to specific location.
===============
*/
void Actor::WalkTo(Event *ev)
{
    Event event = Event(EV_Listener_ExecuteScript, 2);
    event.AddConstString(STRING_GLOBAL_WALKTO_SCR);
    event.AddValue(ev->GetValue(1));
    ExecuteScript(&event);
}

/*
===============
Actor::RunTo

Run to specific location.
===============
*/
void Actor::RunTo(Event *ev)
{
    Event event = Event(EV_Listener_ExecuteScript, 2);
    event.AddConstString(STRING_GLOBAL_RUNTO_SCR);
    event.AddValue(ev->GetValue(1));
    ExecuteScript(&event);
}

/*
===============
Actor::CrouchTo

Crouch to specific location.
===============
*/
void Actor::CrouchTo(Event *ev)
{
    Event event = Event(EV_Listener_ExecuteScript, 2);
    event.AddConstString(STRING_GLOBAL_CROUCHTO_SCR);
    event.AddValue(ev->GetValue(1));
    ExecuteScript(&event);
}

/*
===============
Actor::CrawlTo

Crawl to specific location.
===============
*/
void Actor::CrawlTo(Event *ev)
{
    Event event = Event(EV_Listener_ExecuteScript, 2);
    event.AddConstString(STRING_GLOBAL_CRAWLTO_SCR);
    event.AddValue(ev->GetValue(1));
    ExecuteScript(&event);
}

/*
===============
Actor::ClearAimNode

Clear aim node.
===============
*/
void Actor::ClearAimNode(void)
{
    if (m_aimNode) {
        if (m_aimNode->IsSubclassOfTempWaypoint()) {
            delete m_aimNode;
        }
        m_aimNode = NULL;
    }
}

/*
===============
Actor::SetAimNode

Change aim node.
===============
*/
void Actor::SetAimNode(const Vector& vec)
{
    ClearAimNode();

    TempWaypoint *twp = new TempWaypoint();
    m_aimNode         = twp;

    m_aimNode->setOrigin(vec);
}

/*
===============
Actor::SetAimNode

Change aim node.
===============
*/
void Actor::SetAimNode(Listener *l)
{
    ClearAimNode();
    if (l) {
        if (!l->inheritsFrom(&SimpleEntity::ClassInfo)) {
            ScriptError(
                "Bad aim node with classname '%s' specified for '%s' at (%f %f %f)\n",
                l->getClassname(),
                targetname.c_str(),
                origin.x,
                origin.y,
                origin.z
            );
        }
        m_aimNode = (SimpleEntity *)l;
    }
}

/*
===============
Actor::AimAt

Aim at specified target.
===============
*/
void Actor::AimAt(Event *ev)
{
    if (ev->IsVectorAt(1)) {
        SetAimNode(ev->GetVector(1));
    } else {
        SetAimNode(ev->GetListener(1));
    }

    SetThinkIdle(m_aimNode ? THINK_AIM : THINK_IDLE);
}

/*
===============
Actor::LookAtLookEntity

Change current look entity.
===============
*/
void Actor::LookAtLookEntity(void)
{
    Vector dir;

    if (m_pLookEntity->IsSubclassOfSentient()) {
        Sentient* sen = static_cast<Sentient*>(m_pLookEntity.Pointer());
        dir = sen->EyePosition() - EyePosition();
    } else {
        dir = m_pLookEntity->centroid - EyePosition();
    }

    SetDesiredLookDir(dir);
}

/*
===============
Actor::IdleLook

Idle look behaviour.
===============
*/
void Actor::IdleLook(void)
{
    if (m_pLookEntity) {
        LookAtLookEntity();
    } else {
        ForwardLook();
    }
}

/*
===============
Actor::IdleLook

Idle look behaviour.
===============
*/
void Actor::IdleLook(vec3_t dir)
{
    if (m_pLookEntity) {
        LookAtLookEntity();
    } else {
        SetDesiredLookDir(dir);
    }
}

/*
===============
Actor::ClearLookEntity

Clear look entity.
===============
*/
void Actor::ClearLookEntity(void)
{
    if (m_pLookEntity) {
        if (m_pLookEntity->IsSubclassOfTempWaypoint()) {
            delete m_pLookEntity;
        }
        m_pLookEntity = NULL;
    }
}

/*
===============
Actor::LookAt

Change current look entity.
===============
*/
void Actor::LookAt(const Vector& vec)
{
    if (g_showlookat->integer == entnum || g_showlookat->integer == -1) {
        Com_Printf(
            "Script lookat: %i %i %s looking at point %.0f %.0f %.0f\n",
            entnum,
            radnum,
            targetname.c_str(),
            vec.x,
            vec.y,
            vec.z
        );
    }

    ClearLookEntity();

    TempWaypoint *twp = new TempWaypoint();
    m_pLookEntity     = twp;

    m_pLookEntity->setOrigin(vec);
}

/*
===============
Actor::LookAt

Change current look entity.
===============
*/
void Actor::LookAt(Listener *l)
{
    ClearLookEntity();
    if (!l && (g_showlookat->integer == entnum || g_showlookat->integer == -1)) {
        Com_Printf("Script lookat: %i %i %s cleared lookat\n", entnum, radnum, TargetName().c_str());
        return;
    }

    if (!l->isSubclassOf(SimpleEntity)) {
        ScriptError(
            "Bad look entity with classname '%s' specified for '%s' at (%f %f %f)\n",
            l->getClassname(),
            targetname.c_str(),
            origin.x,
            origin.y,
            origin.z
        );
    }

    if (l != this) {
        l = (SimpleEntity*)l;
        if (g_showlookat->integer == entnum || g_showlookat->integer == -1) {
            Com_Printf(
                "Script lookat: %i %i %s looking at point %.0f %.0f %.0f\n",
                entnum,
                radnum,
                targetname.c_str(),
                ((SimpleEntity*)l)->origin.x,
                ((SimpleEntity*)l)->origin.y,
                ((SimpleEntity*)l)->origin.z
            );
        }
        m_pLookEntity = (SimpleEntity*)l;
    }
}

/*
===============
Actor::EventLookAt

Look at event.
===============
*/
void Actor::EventLookAt(Event *ev)
{
    if (ev->IsVectorAt(1)) {
        LookAt(ev->GetVector(1));
    } else {
        LookAt(ev->GetListener(1));
    }

    m_iLookFlags = 0;
}

/*
===============
Actor::EventEyesLookAt

Eyes look at event.
===============
*/
void Actor::EventEyesLookAt(Event *ev)
{
    EventLookAt(ev);

    m_iLookFlags = LOOK_FLAG_EYE;
}

/*
===============
Actor::NoPoint

Don't point at anything.
===============
*/
void Actor::NoPoint(void)
{
    VectorClear(m_vLUpperArmDesiredAngles);
}

/*
===============
Actor::IdlePoint

Idle point behaviour.
===============
*/
void Actor::IdlePoint(void)
{
    if (!m_pPointEntity) {
        NoPoint();
        return;
    }

    Vector delta = m_pPointEntity->centroid - origin;
    delta.normalize();

    Vector pointAngles = delta.toAngles();
    pointAngles.y -= angles.y;

    pointAngles += Vector(0, 30, 0);
    pointAngles.y = AngleNormalize180(delta.y);
    pointAngles.y = Q_clamp_float(pointAngles.y, -80, 100);

    m_vLUpperArmDesiredAngles[0] = pointAngles[0];
    m_vLUpperArmDesiredAngles[1] = pointAngles[1];
    m_vLUpperArmDesiredAngles[2] = pointAngles[2];
}

/*
===============
Actor::ClearPointEntity

Clear point entity.
===============
*/
void Actor::ClearPointEntity(void)
{
    if (m_pPointEntity) {
        if (m_pPointEntity->IsSubclassOfTempWaypoint()) {
            delete m_pPointEntity;
        }
        m_pPointEntity = NULL;
    }
}

/*
===============
Actor::PointAt

Change point entity.
===============
*/
void Actor::PointAt(const Vector& vec)
{
    ClearPointEntity();

    TempWaypoint *twp = new TempWaypoint();
    m_pPointEntity    = twp;
    m_pPointEntity->setOrigin(vec);
}

/*
===============
Actor::PointAt

Change point entity.
===============
*/
void Actor::PointAt(Listener *l)
{
    ClearPointEntity();

    if (l) {
        if (l->isSubclassOf(SimpleEntity)) {
            ScriptError(
                "Bad point entity with classname '%s' specified for '%s' at (%f %f %f)\n",
                l->getClassname(),
                targetname.c_str(),
                origin.x,
                origin.y,
                origin.z
            );
        }

        if (l != this) {
            m_pPointEntity = static_cast<SimpleEntity*>(l);
        }
    }
}

/*
===============
Actor::EventPointAt

Point at event.
===============
*/
void Actor::EventPointAt(Event *ev)
{
    if (ev->IsVectorAt(1)) {
        PointAt(ev->GetVector(1));
    } else {
        PointAt(ev->GetListener(1));
    }
}

/*
===============
Actor::IdleTurn

Idle turn behaviour.
===============
*/
void Actor::IdleTurn(void)
{
    if (!m_pTurnEntity) {
        return;
    }

    for (int i = 0; i < 2; i++) {
        if (m_pTurnEntity == this) {
            StopTurning();
            m_pTurnEntity = NULL;
            return;
        }

        SetDesiredYawDest(m_pTurnEntity->centroid);

        float error = m_DesiredYaw - angles[1];
        if (error > 180) {
            error -= 360;
        } else if (error < -180) {
            error += 360;
        }

        if (error >= m_fTurnDoneError + 0.001f || error <= -m_fTurnDoneError - 0.001f) {
            return;
        }

        SafePtr<SimpleEntity> prevTurnEntity = m_pTurnEntity;

        Director.Unpause();
        Unregister(STRING_TURNDONE);
        Director.Pause();

        if (m_pTurnEntity == prevTurnEntity) {
            break;
        }
    }
}

/*
===============
Actor::ClearTurnEntity

Clear turn entity.
===============
*/
void Actor::ClearTurnEntity(void)
{
    if (m_pTurnEntity) {
        if (m_pTurnEntity->IsSubclassOfTempWaypoint()) {
            delete m_pTurnEntity;
        }
        m_pTurnEntity = NULL;
    }
}

/*
===============
Actor::TurnTo

Change turn entity.
===============
*/
void Actor::TurnTo(const Vector& vec)
{
    ClearTurnEntity();

    TempWaypoint *twp = new TempWaypoint();
    m_pTurnEntity     = twp;
    m_pTurnEntity->setOrigin(vec);
}

/*
===============
Actor::TurnTo

Change turn entity.
===============
*/
void Actor::TurnTo(Listener *l)
{
    ClearTurnEntity();

    if (!l) {
        m_pTurnEntity = this;
        return;
    }

    if (!l->isSubclassOf(SimpleEntity)) {
        ScriptError(
            "Bad turn entity with classname '%s' specified for '%s' at (%f %f %f)\n",
            l->getClassname(),
            targetname.c_str(),
            origin.x,
            origin.y,
            origin.z
        );
    }

    m_pTurnEntity = (SimpleEntity*)l;
}

/*
===============
Actor::EventTurnTo

Turn to event.
===============
*/
void Actor::EventTurnTo(Event *ev)
{
    if (ev->IsVectorAt(1)) {
        TurnTo(ev->GetVector(1));
    } else {
        TurnTo(ev->GetListener(1));
    }
}

/*
===============
Actor::EventSetTurnDoneError

Set the error amount that turndone will occur for the turnto command.
===============
*/
void Actor::EventSetTurnDoneError(Event *ev)
{
    m_fTurnDoneError = ev->GetFloat(1);
    if (m_fTurnDoneError < 0) {
        m_fTurnDoneError = 0;
        ScriptError("turndoneerror was set to a negative value - capped to 0");
    }
}

/*
===============
Actor::EventGetTurnDoneError

Get the error amount that turndone will occur for the turnto command.
===============
*/
void Actor::EventGetTurnDoneError(Event *ev)
{
    ev->AddFloat(m_fTurnDoneError);
}

/*
===============
Actor::EventGiveWeaponInternal

Give weapon to actor.

Called from STRING_GLOBAL_WEAPON_SCR.
===============
*/
void Actor::EventGiveWeaponInternal(Event *ev)
{
    Holster();
    RemoveWeapons();

    const str weapName = ev->GetString(1);
    if (weapName.length() > 0 && giveItem(weapName)) {
        Unholster();
    }
}

/*
===============
Actor::EventGiveWeapon

Give weapon to actor.
===============
*/
void Actor::EventGiveWeapon(Event *ev)
{
    Event e1(EV_Listener_ExecuteScript);

    str weapName = ev->GetString(1);
    weapName.tolower();

    m_csLoadOut = Director.AddString(weapName);

    if (m_csLoadOut == STRING_MG42) {
        m_csWeapon = STRING_MP40;
    } else {
        m_csWeapon = m_csLoadOut;
    }

    setModel();

    e1.AddConstString(STRING_GLOBAL_WEAPON_SCR);
    e1.AddString(weapName);
    gi.Printf(
        "EventGiveWeapon script: %s weapName: %s \n",
        Director.GetString(STRING_GLOBAL_WEAPON_SCR).c_str(),
        weapName.c_str()
    );
    ExecuteScript(&e1);
}

/*
===============
Actor::EventGetWeapon

Returns weapon path to script.
===============
*/
void Actor::EventGetWeapon(Event *ev)
{
    ev->AddConstString(m_csWeapon);
}

/*
===============
Actor::DefaultPain

Default pain handler.
===============
*/
void Actor::DefaultPain(Event *ev)
{
    SetThink(THINKSTATE_PAIN, THINK_PAIN);
    HandlePain(ev);
}

/*
===============
Actor::HandlePain

Hangled pain event.
===============
*/
void Actor::HandlePain(Event *ev)
{
    int i;
    Event event(EV_Listener_ExecuteScript);
    int num;
    Entity* attacker;

    if (!m_bEnablePain) {
        return;
    }

    attacker = ev->GetEntity(1);
    if (attacker && attacker->IsSubclassOfSentient() && IsTeamMate(static_cast<Sentient*>(attacker))) {
        return;
    }

    if (!m_bNoLongPain) {
        event.AddConstString(STRING_GLOBAL_PAIN_SCR);

        num = ev->NumArgs();
        for (i = 1; i <= num; i++) {
            event.AddValue(ev->GetValue(i));
        }

        ExecuteScript(&event);
        SetThinkState(THINKSTATE_PAIN, THINKLEVEL_PAIN);
    }

    RaiseAlertness(0.5f);

    m_PainTime = level.inttime;

    if (attacker && attacker->IsSubclassOfSentient() && !IsTeamMate(static_cast<Sentient*>(attacker))) {
        m_pLastAttacker = attacker;

        m_iCuriousLevel = 9;

        //FIXME: macro
        SetCuriousAnimHint(7);

        if (m_bEnableEnemy && m_ThinkStates[THINKLEVEL_NORMAL] == THINKSTATE_IDLE) {
            SetEnemyPos(attacker->origin);
            m_pszDebugState = "from_pain";
            SetThinkState(THINKSTATE_CURIOUS, THINKLEVEL_NORMAL);
        }
    }

    Unregister(STRING_PAIN);
}

/*
===============
Actor::EventPain

Pain event.
===============
*/
void Actor::EventPain(Event *ev)
{
    if (g_showinfo->integer) {
        ShowInfo();
    }

    GlobalFuncs_t *func = &GlobalFuncs[CurrentThink()];

    if (func->Pain) {
        (this->*func->Pain)(ev);
    }
}

/*
===============
Actor::DefaultKilled

Default killed handler.
===============
*/
void Actor::DefaultKilled(Event *ev, bool bPlayDeathAnim)
{
    ClearStates();
    SetThink(THINKSTATE_KILLED, THINK_KILLED);
    HandleKilled(ev, bPlayDeathAnim);
}

/*
===============
Actor::HandleKilled

Hangled killed event.
===============
*/
void Actor::HandleKilled(Event *ev, bool bPlayDeathAnim)
{
    deadflag = DEAD_DEAD;
    health   = 0.0;

    if (bPlayDeathAnim) {
        Event event(EV_Listener_ExecuteScript);
        event.AddConstString(STRING_GLOBAL_KILLED_SCR);
        for (int i = 1; i < ev->NumArgs(); i++) {
            event.AddValue(ev->GetValue(i));
        }
        ExecuteScript(&event);
    } else {
        SetThink(THINKSTATE_KILLED, THINK_DEAD);
    }

    ClearThinkStates();

    SetThinkState(THINKSTATE_KILLED, THINKLEVEL_KILLED);

    Unregister(STRING_DEATH);
    Unregister(STRING_PAIN);
}

/*
===============
Actor::DispatchEventKilled

Dispatch killed event.
===============
*/
void Actor::DispatchEventKilled(Event *ev, bool bPlayDeathAnim)
{
    GlobalFuncs_t *func = &GlobalFuncs[CurrentThink()];

    if (func->Killed) {
        (this->*func->Killed)(ev, bPlayDeathAnim);
    }

    SetEnemy(NULL, false);

    DisbandSquadMate(this);
    if (bPlayDeathAnim) {
        DropInventoryItems();
    }
}

/*
===============
Actor::EventKilled

Killed event.
===============
*/
void Actor::EventKilled(Event *ev)
{
    Entity* attacker;
    Sentient* pBuddy;
    Sentient* sent;
    Player* player;

    DispatchEventKilled(ev, true);

    attacker = ev->GetEntity(1);
    if (attacker && attacker->IsSubclassOfPlayer()) {
        player = static_cast<Player*>(attacker);
        if (player->m_Team != m_Team) {
            player->m_iNumEnemiesKilled++;
        }
    }

    if (attacker && attacker->IsSubclassOfSentient()) {
        sent = static_cast<Sentient*>(attacker);

        for (pBuddy = level.m_HeadSentient[m_Team]; pBuddy; pBuddy = pBuddy->m_NextSentient) {
            if (pBuddy != this && pBuddy->IsSubclassOfActor()) {
                Actor* actor = static_cast<Actor*>(pBuddy);
                actor->NotifySquadmateKilled(this, sent);
            }
        }
    }
}

/*
===============
Actor::EventBeDead

Become dead.
===============
*/
void Actor::EventBeDead(Event *ev)
{
    health = 0;
    DispatchEventKilled(ev, false);
}

/*
===============
Actor::DeathEmbalm

preps the dead actor for turning nonsolid gradually over time
===============
*/
void Actor::DeathEmbalm(Event *ev)
{
    if (maxs.z > 8) {
        maxs.z -= 4;

        if (maxs.z > 8) {
            PostEvent(EV_Actor_DeathEmbalm, 0.5f);
        } else {
            maxs.z = 8.0;
        }

        setSize(mins, maxs);
    }
}

/*
===============
Actor::DeathSinkStart

Makes the actor sink into the ground and then get removed(this starts it).
===============
*/
void Actor::DeathSinkStart(Event *ev)
{
    setMoveType(MOVETYPE_NONE);
    flags &= ~FL_THINK;
    Entity::DeathSinkStart(ev);
}

/*
===============
Actor::FireWeapon

Fire weapon from script.
===============
*/
void Actor::FireWeapon(Event *ev)
{
    if (ev->NumArgs() > 0 && ev->GetInteger(1) == WEAPON_OFFHAND) {
        // shoot using off hand
        Sentient::FireWeapon(WEAPON_OFFHAND, FIRE_PRIMARY);
    } else {
        Sentient::FireWeapon(WEAPON_MAIN, FIRE_PRIMARY);
    }
}

/*
===============
Actor::PlayAnimation

Play animation
===============
*/
void Actor::PlayAnimation(Event *ev)
{
    Event event(EV_Listener_ExecuteScript, 2);
    event.AddConstString(STRING_GLOBAL_ANIM_SCR);
    event.AddValue(ev->GetValue(1));
    ExecuteScript(&event);
}

/*
===============
Actor::PlayScriptedAnimation

Play scripted animation
===============
*/
void Actor::PlayScriptedAnimation(Event *ev)
{
    Event event(EV_Listener_ExecuteScript, 2);
    event.AddConstString(STRING_GLOBAL_ANIM_SCRIPTED_SCR);
    event.AddValue(ev->GetValue(1));
    ExecuteScript(&event);
}

/*
===============
Actor::PlayNoclipAnimation

Play noclip animation
===============
*/
void Actor::PlayNoclipAnimation(Event *ev)
{
    Event event(EV_Listener_ExecuteScript, 2);
    event.AddConstString(STRING_GLOBAL_ANIM_NOCLIP_SCR);
    event.AddValue(ev->GetValue(1));
    ExecuteScript(&event);
}

/*
===============
Actor::PlayAttachedAnimation

Play attached animation
===============
*/
void Actor::PlayAttachedAnimation(Event *ev)
{
    Event event(EV_Listener_ExecuteScript, 2);
    event.AddConstString(STRING_GLOBAL_ANIM_ATTACHED_SCR);
    event.AddValue(ev->GetValue(1));
    ExecuteScript(&event);
}

/*
===============
Actor::MoveDest

Play noclip animation
===============
*/
void Actor::MoveDest(float fMoveSpeed)
{
    mmove_t mm;
    vec2_t  offset;

    SetMoveInfo(&mm);

    mm.desired_speed = fMoveSpeed;
    if (mm.desired_speed > m_maxspeed) {
        mm.desired_speed = m_maxspeed;
    }

    VectorSub2D(m_Dest, origin, offset);
    VectorNormalize2D(offset);
    VectorCopy2D(offset, mm.desired_dir);

    MmoveSingle(&mm);

    GetMoveInfo(&mm);
}

/*
===============
Actor::MovePath

Move on path.
===============
*/
void Actor::MovePath(float fMoveSpeed)
{
    mmove_t mm;
    const float* dir;
    vec2_t delta;

    SetMoveInfo(&mm);

    mm.desired_speed = fMoveSpeed;
    if (mm.desired_speed > m_maxspeed) {
        mm.desired_speed = m_maxspeed;
    }

    m_Path.UpdatePos(origin);

    if (ai_debugpath->integer) {
        Vector pos, dest;
        PathInfo* current_path;

        pos = origin;
        pos.z += 32;

        dest = m_Path.CurrentDelta() + origin;
        dest.z += 32;

        G_DebugLine(pos, dest, 1, 0, 0, 1);

        for (current_path = m_Path.CurrentNode(); current_path != m_Path.LastNode(); current_path = m_Path.NextNode()) {
            pos = current_path->point;
            current_path--;
            dest = current_path->point;

            G_DebugLine(pos + Vector(0, 0, 32), dest + Vector(0, 0, 32), 0, 1, 0, 1);
        }
    }

    VectorCopy2D(m_Path.CurrentDelta(), mm.desired_dir);
    MmoveSingle(&mm);

    if (!mm.hit_obstacle) {
        if (m_WallDir) {
            if (level.inttime >= m_iWallDodgeTimeout) {
                m_WallDir = 0;
            } else if (DotProduct2D(mm.desired_dir, m_PrevObstacleNormal) > 0 && CrossProduct2D(mm.desired_dir, m_PrevObstacleNormal) < 0) {
                m_iWallDodgeTimeout = level.inttime + 1000;
                m_WallDir = -m_WallDir;
            }
        }
        GetMoveInfo(&mm);
        return;
    }

    velocity.copyTo(mm.velocity);
    origin.copyTo(mm.origin);
    mm.groundPlane = m_groundPlane;
    mm.walking     = m_walking;
    VectorCopy(m_groundPlaneNormal, mm.groundPlaneNormal);
    VectorCopy2D(mm.obstacle_normal, m_PrevObstacleNormal);

    m_Path.UpdatePos(mm.hit_origin);

    if (m_Path.NextNode() && m_Path.IsAccurate()) {
        dir = m_Path.CurrentPathDir();

        m_iWallDodgeTimeout = level.inttime + 1000;
        VectorSub2D(m_Path.NextNode()->point, mm.hit_origin, delta);

        if (delta[0] * dir[1] > delta[1] * dir[0]) {
            m_WallDir = -1;
            mm.desired_dir[0] = -mm.obstacle_normal[1];
            mm.desired_dir[1] = mm.obstacle_normal[0];
        }
        else {
            mm.desired_dir[0] = mm.obstacle_normal[1];
            mm.desired_dir[1] = -mm.obstacle_normal[0];
        }

        MmoveSingle(&mm);

        if (!mm.hit_obstacle) {
            GetMoveInfo(&mm);
            return;
        }

        velocity.copyTo(mm.velocity);
        origin.copyTo(mm.origin);

        mm.groundPlane = m_groundPlane;
        mm.walking = m_walking;
        VectorCopy(m_groundPlaneNormal, mm.groundPlaneNormal);
        VectorCopy2D(mm.obstacle_normal, m_PrevObstacleNormal);

        if (m_WallDir == -1) {
            mm.desired_dir[0] = -mm.obstacle_normal[1];
            mm.desired_dir[1] = mm.obstacle_normal[0];
        } else {
            mm.desired_dir[0] = mm.obstacle_normal[1];
            mm.desired_dir[1] = -mm.obstacle_normal[0];
        }

        MmoveSingle(&mm);
        GetMoveInfo(&mm);
    } else {
        if (m_Path.NextNode() && !m_Path.IsAccurate() && m_WallDir == 0) {
            dir = m_Path.CurrentPathDir();

            VectorSub2D(m_Path.NextNode()->point, mm.hit_origin, delta);

            if (DotProduct2D(dir, delta) >= 0) {
                float cross = CrossProduct2D(dir, delta);
                if (cross <= -16) {
                    mm.desired_dir[0] = -mm.obstacle_normal[1];
                    mm.desired_dir[1] = mm.obstacle_normal[0];

                    MmoveSingle(&mm);
                    GetMoveInfo(&mm);
                    return;

                } else if (cross >= 16) {
                    mm.desired_dir[0] = mm.obstacle_normal[1];
                    mm.desired_dir[1] = -mm.obstacle_normal[0];

                    MmoveSingle(&mm);
                    GetMoveInfo(&mm);
                    return;
                }
            }
        }

        if (m_WallDir == 0) {
            dir = m_Path.CurrentPathDir();

            if (m_Path.NextNode()) {
                dir = m_Path.NextNode()->dir;
            }

            if (dir[0] * mm.obstacle_normal[1] > dir[1] * mm.obstacle_normal[0]) {
                m_WallDir = 1;
                mm.desired_dir[0] = mm.obstacle_normal[1];
                mm.desired_dir[1] = -mm.obstacle_normal[0];
            }
            else {
                m_WallDir = -1;
                mm.desired_dir[0] = -mm.obstacle_normal[1];
                mm.desired_dir[1] = mm.obstacle_normal[0];
            }

            MmoveSingle(&mm);
            if (mm.hit_obstacle) {
                m_WallDir = 0;
                m_iWallDodgeTimeout = 0;
            } else {
                m_iWallDodgeTimeout = level.inttime + 1000;
            }

            GetMoveInfo(&mm);
        } else {
            if (m_WallDir >= 0) {
                mm.desired_dir[0] = mm.obstacle_normal[1];
                mm.desired_dir[1] = -mm.obstacle_normal[0];
            } else {
                mm.desired_dir[0] = -mm.obstacle_normal[1];
                mm.desired_dir[1] = mm.obstacle_normal[0];
            }

            MmoveSingle(&mm);

            if (mm.hit_obstacle) {
                m_WallDir = 0;
                m_iWallDodgeTimeout = 0;
            }
        }
    }
}

/*
===============
Actor::MovePathGoal

Move on path end(goal).
===============
*/
void Actor::MovePathGoal(float fMoveSpeed)
{
    vec2_t vDelta;
    float fSlowdownSpeed;
    float fTimeToGo;
    float fDeltaSquareLen;

    if (!m_Path.HasCompleteLookahead()) {
        MovePath(fMoveSpeed);
        m_fPathGoalTime = 0;
        return;
    }

    VectorSub2D(origin, m_Path.CurrentPathGoal(), vDelta);
    fDeltaSquareLen = VectorLength2DSquared(vDelta);
    fTimeToGo = m_fPathGoalTime - level.time;

    if (fTimeToGo > -0.001f) {
        fSlowdownSpeed = sqrt(fDeltaSquareLen) * (2.f / (fTimeToGo + level.frametime));

        if (fSlowdownSpeed > (fMoveSpeed + 0.001f) && fSlowdownSpeed > sv_runspeed->value * 0.4f) {
            m_fPathGoalTime = 0;
            StartAnimation(ANIM_MODE_PATH_GOAL, m_Anim);
            MovePath(fMoveSpeed);
        } else {
            MovePath(fSlowdownSpeed);
            if (level.time >= m_fPathGoalTime) {
                m_eAnimMode = ANIM_MODE_NORMAL;
            }
        }
    } else if (fDeltaSquareLen < Square(fMoveSpeed * 0.5f)) {
        fTimeToGo = 0.5f;
        m_fPathGoalTime = level.time + fTimeToGo;
        if (m_csPathGoalEndAnimScript == STRING_EMPTY) {
            m_csPathGoalEndAnimScript = STRING_ANIM_IDLE_SCR;
        }
        m_bStartPathGoalEndAnim = true;
    } else {
        MovePath(fMoveSpeed);
        m_fPathGoalTime = 0;
        return;
    }
}

/*
===============
Actor::Dumb

Make actor dumb.
===============
*/
void Actor::Dumb(Event *ev)
{
    Event e1(EV_Listener_ExecuteScript);
    e1.AddConstString(STRING_GLOBAL_DISABLE_AI_SCR);
    ExecuteScript(&e1);
}

/*
===============
Actor::PhysicsOn

Enable physics.
===============
*/
void Actor::PhysicsOn(Event *ev)
{
    m_bDoPhysics = true;
}

/*
===============
Actor::PhysicsOn

Disable PhysicsOff.
===============
*/
void Actor::PhysicsOff(Event *ev)
{
    m_bDoPhysics = false;
}

/*
===============
Actor::EventGetMood

Get current mood.
===============
*/
void Actor::EventGetMood(Event *ev)
{
    ev->AddConstString(m_csMood);
}

/*
===============
Actor::EventSetMood

Set current mood.
===============
*/
void Actor::EventSetMood(Event *ev)
{
    m_csMood = ev->GetConstString(1);
    if (m_csMood > STRING_ALERT) {
        m_csMood = STRING_BORED;
        ScriptError("invalid mood - setting to bored");
    }
}

/*
===============
Actor::EventDamagePuff

Spawns a puff of 'blood' smoke at the speficied location in the specified direction.
===============
*/
void Actor::EventDamagePuff(Event *ev)
{
    Vector pos = ev->GetVector(1);
    Vector dir = ev->GetVector(2);

    gi.SetBroadcastVisible(pos, NULL);

    gi.MSG_StartCGM(BG_MapCGMToProtocol(g_protocol, CGM_BULLET_8));
    gi.MSG_WriteCoord(pos.x);
    gi.MSG_WriteCoord(pos.y);
    gi.MSG_WriteCoord(pos.z);

    gi.MSG_WriteDir(dir);
    gi.MSG_WriteBits(0, 1);
    gi.MSG_EndCGM();
}

/*
===============
Actor::EventGetAngleYawSpeed

Get current AngleYawSpeed.
===============
*/
void Actor::EventGetAngleYawSpeed(Event *ev)
{
    ev->AddFloat(m_fAngleYawSpeed);
}

/*
===============
Actor::EventSetAngleYawSpeed

Set current AngleYawSpeed.
===============
*/
void Actor::EventSetAngleYawSpeed(Event *ev)
{
    float speed = ev->GetFloat(1);
    if (speed <= 0) {
        ScriptError("Attempting to set illegal turnspeed %f", speed);
    }
    m_fAngleYawSpeed = speed;
}

/*
===============
Actor::EventSetAimTarget

Set current weapon's aim tagret.
===============
*/
void Actor::EventSetAimTarget(Event *ev)
{
    Weapon *weap = GetActiveWeapon(WEAPON_MAIN);
    weap->SetAimTarget(ev->GetEntity(1));
}

/*
===============
Actor::ReadyToFire

Returns true if weapon is ready to fire.
===============
*/
void Actor::ReadyToFire(Event *ev)
{
    bool ready;

    Weapon *weap = GetActiveWeapon(WEAPON_MAIN);

    ready = weap && weap->ReadyToFire(FIRE_PRIMARY, true);

    ev->AddInteger(ready);
}

/*
===============
Actor::EventGetSight

Return current sight (vision distance).
===============
*/
void Actor::EventGetSight(Event *ev)
{
    ev->AddFloat(m_fSight);
}

/*
===============
Actor::EventSetSight

Set current sight (vision distance).
===============
*/
void Actor::EventSetSight(Event *ev)
{
    m_fSight = ev->GetFloat(1);
}

/*
===============
Actor::EventGetHearing

Get current hearing distance.
===============
*/
void Actor::EventGetHearing(Event *ev)
{
    ev->AddFloat(m_fHearing);
}

/*
===============
Actor::EventSetHearing

Set current hearing distance.
===============
*/
void Actor::EventSetHearing(Event *ev)
{
    m_fHearing = ev->GetFloat(1);
}

/*
===============
Actor::EventGetFov

Get current fov.
===============
*/
void Actor::EventGetFov(Event *ev)
{
    ev->AddFloat(m_fFov);
}

/*
===============
Actor::EventSetFov

Set current fov.
============
*/
void Actor::EventSetFov(Event *ev)
{
    float fov = ev->GetFloat(1);
    if (fov < 0 || fov > 360) {
        ScriptError("fov must be in the range [0,360]");
    }
    m_fFov    = fov;
    m_fFovDot = cos(0.5 * fov * M_PI / 180.0);
}

/*
===============
Actor::ClearPatrolCurrentNode

Clear current patrol node.
===============
*/
void Actor::ClearPatrolCurrentNode(void)
{
    if (m_patrolCurrentNode) {
        if (m_patrolCurrentNode->IsSubclassOfTempWaypoint()) {
            delete m_patrolCurrentNode;
        }
        m_patrolCurrentNode = NULL;
    }
}

/*
===============
Actor::NextPatrolCurrentNode

Switch to next patrol node.
===============
*/
void Actor::NextPatrolCurrentNode(void)
{
    if (m_bScriptGoalValid) {
        if (m_patrolCurrentNode->origin == m_vScriptGoal) {
            m_bScriptGoalValid = false;
        }
    }
    if (m_patrolCurrentNode->IsSubclassOfTempWaypoint()) {
        ClearPatrolCurrentNode();
    } else {
        m_patrolCurrentNode = m_patrolCurrentNode->Next();
    }
}

/*
===============
Actor::SetPatrolCurrentNode

Set current patrol node.
===============
*/
void Actor::SetPatrolCurrentNode(Vector& vec)
{
    ClearPatrolCurrentNode();

    TempWaypoint *twp = new TempWaypoint();

    m_patrolCurrentNode = twp;

    m_patrolCurrentNode->setOrigin(vec);
}

/*
===============
Actor::SetPatrolCurrentNode

Set current patrol node.
===============
*/
void Actor::SetPatrolCurrentNode(Listener *l)
{
    ClearPatrolCurrentNode();

    if (l && !l->inheritsFrom(&SimpleEntity::ClassInfo)) {
        ScriptError(
            "Bad patrol path with classname '%s' specified for '%s' at (%f %f %f)\n",
            l->getClassname(),
            targetname.c_str(),
            origin.x,
            origin.y,
            origin.z
        );
    }

    m_patrolCurrentNode = (SimpleEntity *)l;
}

/*
===============
Actor::EventSetPatrolPath

Set current patrol path.
===============
*/
void Actor::EventSetPatrolPath(Event *ev)
{
    SetPatrolCurrentNode(ev->GetListener(1));
}

/*
===============
Actor::EventGetPatrolPath

Get current patrol path.
===============
*/
void Actor::EventGetPatrolPath(Event *ev)
{
    ev->AddListener(m_patrolCurrentNode);
}

/*
===============
Actor::EventSetPatrolWaitTrigger

Set m_bPatrolWaitTrigger.
===============
*/
void Actor::EventSetPatrolWaitTrigger(Event *ev)
{
    m_bPatrolWaitTrigger = ev->GetBoolean(1);
}

/*
===============
Actor::EventGetPatrolWaitTrigger

Get m_bPatrolWaitTrigger.
===============
*/
void Actor::EventGetPatrolWaitTrigger(Event *ev)
{
    ev->AddInteger(m_bPatrolWaitTrigger);
}

/*
===============
Actor::ShowInfo_PatrolCurrentNode

Show current patrol node info.
===============
*/
void Actor::ShowInfo_PatrolCurrentNode(void)
{
    if (m_patrolCurrentNode) {
        Com_Printf("current patrol node: %s\n", m_patrolCurrentNode->targetname.c_str());
    } else {
        Com_Printf("no current patrol node\n");
    }
}

/*
===============
Actor::ShowInfo_AimNode

Show current aim node info.
===============
*/
void Actor::ShowInfo_AimNode(void)
{
    if (m_aimNode) {
        Com_Printf("aim node: %s\n", m_aimNode->targetname.c_str());
    } else {
        Com_Printf("no current aim node\n");
    }
}

/*
===============
Actor::MoveOnPathWithSquad

Move on path with squad.
===============
*/
bool Actor::MoveOnPathWithSquad(void)
{
    float   fDistSquared, fIntervalSquared;
    vec2_t  vMyNormalDir, vDelta, vHisDir;
    Player *p;

    //FIXMEL: macro
    if (level.inttime < m_iSquadStandTime + 500 || !PathExists() || PathComplete()) {
        return false;
    }
    fIntervalSquared = Square(m_fInterval);
    if (m_iSquadStandTime) {
        fIntervalSquared *= 2;
    }

    p = (Player *)G_GetEntity(0);

    if (IsTeamMate(p)) {
        VectorSub2D(p->origin, origin, vDelta);
        fDistSquared = VectorLength2DSquared(vDelta);
        if (fIntervalSquared > fDistSquared && DotProduct2D(vDelta, PathDelta()) > 0) {
            VectorSub2D(velocity, p->velocity, vMyNormalDir);
            VectorNormalize2D(vMyNormalDir);
            if (fDistSquared - DotProduct2D(vDelta, vMyNormalDir) < 2304) {
                m_iSquadStandTime = level.inttime;
                return false;
            }
        }
    }

    for (Actor *pSquadMate = (Actor *)m_pNextSquadMate.Pointer(); pSquadMate != this;
         pSquadMate        = (Actor *)pSquadMate->m_pNextSquadMate.Pointer()) {
        if (!pSquadMate->IsSubclassOfActor()) {
            continue;
        }
        VectorSub2D(pSquadMate->origin, origin, vDelta);
        fDistSquared = VectorLength2DSquared(vDelta);
        if (fDistSquared >= fIntervalSquared || DotProduct2D(vDelta, PathDelta()) <= 0) {
            continue;
        }
        if (pSquadMate->PathExists() || pSquadMate->PathComplete()) {
            VectorCopy2D(pSquadMate->velocity, vHisDir);
            if (VectorLength2DSquared(vHisDir) <= 8) {
                continue;
            }
        } else {
            VectorCopy2D(pSquadMate->PathDelta(), vHisDir);
        }

        if (DotProduct2D(PathDelta(), vHisDir) >= 0
            && (entnum == pSquadMate->entnum || DotProduct2D(vHisDir, vDelta) >= 0)) {
            m_iSquadStandTime = level.inttime;
            return false;
        }
    }
    m_iSquadStandTime = 0;
    return true;
}

/*
===============
Actor::MoveToWaypointWithPlayer

Move to waypoint with player.
===============
*/
bool Actor::MoveToWaypointWithPlayer(void)
{
    float  fIntervalSquared, fDistSquared;
    vec2_t vMyNormalDir, vDelta, vMyDir;

    if (level.inttime < m_iSquadStandTime + 500) {
        return false;
    }

    fIntervalSquared = Square(m_fInterval);
    if (m_iSquadStandTime) {
        fIntervalSquared += fIntervalSquared;
    }

    Player *p = (Player *)G_GetEntity(0);
    if (IsTeamMate(p)) {
        vDelta[0]    = p->origin[0] - origin[0];
        vDelta[1]    = p->origin[1] - origin[1];
        fDistSquared = VectorLength2DSquared(vDelta);

        if (fDistSquared >= fIntervalSquared) {
            m_iSquadStandTime = 0;
            return true;
        }

        vMyDir[0] = m_patrolCurrentNode->origin[0] - origin[0];
        vMyDir[1] = m_patrolCurrentNode->origin[1] - origin[1];
        if (DotProduct2D(vDelta, vMyDir) < 0) {
            m_iSquadStandTime = 0;
            return true;
        }

        vMyNormalDir[0] = velocity[0] - p->velocity[0];
        vMyNormalDir[1] = velocity[1] - p->velocity[1];
        VectorNormalize2D(vMyNormalDir);

        if (fDistSquared - DotProduct2D(vDelta, vMyNormalDir) > 2304) {
            m_iSquadStandTime = 0;
            return true;
        }
        m_iSquadStandTime = level.inttime;
    }
    return false;
}

/*
===============
Actor::PatrolNextNodeExists

Returns true if next patrol node exits.
===============
*/
bool Actor::PatrolNextNodeExists(void)
{
    return m_patrolCurrentNode && !(m_patrolCurrentNode->IsSubclassOfTempWaypoint()) && m_patrolCurrentNode->Next();
}

/*
===============
Actor::UpdatePatrolCurrentNode

Update current patrol node.
===============
*/
void Actor::UpdatePatrolCurrentNode(void)
{
    bool next = false;
    if (m_patrolCurrentNode) {
        if (m_patrolCurrentNode->IsSubclassOfTempWaypoint()) {
            if (fabs(origin[0] - m_patrolCurrentNode->origin[0]) < 16.0
                && fabs(origin[1] - m_patrolCurrentNode->origin[1]) < 16.0) {
                next = true;
            }
        } else if (PathExists() && PathComplete()) {
            next = true;
        }
    }
    if (next) {
        NextPatrolCurrentNode();
    }
}

/*
===============
Actor::MoveToPatrolCurrentNode


===============
*/
bool Actor::MoveToPatrolCurrentNode(void)
{
    UpdatePatrolCurrentNode();
    if (m_patrolCurrentNode && !m_bPatrolWaitTrigger) {
        Vector delta;
        if (m_patrolCurrentNode->IsSubclassOfWaypoint()) {
            if (MoveToWaypointWithPlayer()) {
                m_bNextForceStart  = false;
                m_eNextAnimMode    = ANIM_MODE_DEST;
                m_csNextAnimString = m_csPatrolCurrentAnim;
                FaceMotion();
            } else {
                Anim_Stand();
                IdleLook();
            }

            SetDest(m_patrolCurrentNode->origin);

            if (m_fMoveDoneRadiusSquared == 0.0 || m_patrolCurrentNode->Next()) {
                return false;
            }
            delta = m_patrolCurrentNode->origin - origin;
        } else {
            SimpleActor::SetPath(m_patrolCurrentNode->origin, "Actor::MoveToPatrolCurrentNode", 0, NULL, 0);
            if (!PathExists()) {
                IdleLook();

                Anim_Idle();
                parm.movefail = qtrue;
                return false;
            }
            if (MoveOnPathWithSquad()) {
                if (PatrolNextNodeExists()) {
                    m_eNextAnimMode = ANIM_MODE_PATH;
                } else {
                    m_eNextAnimMode = ANIM_MODE_PATH_GOAL;
                }
                m_csNextAnimString = m_csPatrolCurrentAnim;
                m_bNextForceStart  = false;
                FaceMotion();
            } else {
                Anim_Stand();
                IdleLook();
            }

            if (m_fMoveDoneRadiusSquared == 0) {
                return false;
            }

            if (PathComplete()) {
                return true;
            }

            if (!m_Path.HasCompleteLookahead() || m_patrolCurrentNode->Next()) {
                return false;
            }
            delta = PathDelta();
        }
        return m_fMoveDoneRadiusSquared >= delta.lengthXYSquared();
    }

    IdleLook();
    Anim_Idle();
    return false;
}

/*
===============
Actor::EventSetAccuracy

Set current accuracy.
===============
*/
void Actor::EventSetAccuracy(Event *ev)
{
    mAccuracy = ev->GetFloat(1) / 100;
}

/*
===============
Actor::EventGetAccuracy

Get current accuracy.
===============
*/
void Actor::EventGetAccuracy(Event *ev)
{
    ev->AddFloat(mAccuracy * 100);
}

void Actor::EventSetMinDistance(Event *ev)
{
    m_fMinDistance = ev->GetFloat(1);

    if (m_fMinDistance < 0) {
        m_fMinDistance = 0;
        Com_Printf(
            "^~^~^ Warning: mindist negative, forcing to %g for entity %i, targetname '%s'\n",
            m_fMinDistance,
            entnum,
            targetname.c_str()
        );
    }
    m_fMinDistanceSquared = m_fMinDistance * m_fMinDistance;
}

void Actor::EventGetMinDistance(Event *ev)
{
    ev->AddFloat(m_fMinDistance);
}

void Actor::EventSetMaxDistance(Event *ev)
{
    m_fMaxDistance = ev->GetFloat(1);
    //FIXME: macro
    if (m_fMaxDistance < 256) {
        m_fMaxDistance = 256;
        Com_Printf(
            "^~^~^ Warning: maxdist too small, forcing to %g for entity %i, targetname '%s'\n",
            m_fMaxDistance,
            entnum,
            targetname.c_str()
        );
    }
    m_fMaxDistanceSquared = m_fMaxDistance * m_fMaxDistance;
}

void Actor::EventGetMaxDistance(Event *ev)
{
    ev->AddFloat(m_fMaxDistance);
}

void Actor::EventGetLeash(Event *ev)
{
    ev->AddFloat(m_fLeash);
}

void Actor::EventSetLeash(Event *ev)
{
    m_fLeash        = ev->GetFloat(1);
    m_fLeashSquared = Square(m_fLeash);
}

void Actor::EventGetInterval(Event *ev)
{
    ev->AddFloat(m_fInterval);
}

void Actor::EventSetInterval(Event *ev)
{
    m_fInterval = ev->GetFloat(1);
}

void Actor::EventDistToEnemy(Event *ev)
{
    float dist = 0;
    if (m_Enemy) {
        Vector distV = origin - m_Enemy->origin;
        dist         = distV.length();
    }
    ev->AddFloat(dist);
}

/*
===============
Actor::Init

Initialize global actor variables.
Called from G_InitGame()
===============
*/
void Actor::Init(void)
{
    g_showinfo = gi.Cvar_Get("g_showinfo", "0", 0);

    for (int i = 0; i < NUM_THINKS; i++) {
        GlobalFuncs[i].ThinkState                 = NULL;
        GlobalFuncs[i].BeginState                 = NULL;
        GlobalFuncs[i].ResumeState                = NULL;
        GlobalFuncs[i].EndState                   = NULL;
        GlobalFuncs[i].SuspendState               = NULL;
        GlobalFuncs[i].RestartState               = &Actor::DefaultRestart;
        GlobalFuncs[i].FinishedAnimation          = NULL;
        GlobalFuncs[i].PostShoot                  = NULL;
        GlobalFuncs[i].Pain                       = &Actor::DefaultPain;
        GlobalFuncs[i].Killed                     = &Actor::DefaultKilled;
        GlobalFuncs[i].PassesTransitionConditions = NULL;
        GlobalFuncs[i].ShowInfo                   = NULL;
        GlobalFuncs[i].ReceiveAIEvent             = &Actor::DefaultReceiveAIEvent;
        GlobalFuncs[i].IsState                    = NULL;
        GlobalFuncs[i].PathnodeClaimRevoked       = NULL;
    }

    InitVoid(&GlobalFuncs[THINK_VOID]);
    InitTurret(&GlobalFuncs[THINK_TURRET]);
    InitCover(&GlobalFuncs[THINK_COVER]);
    InitPatrol(&GlobalFuncs[THINK_PATROL]);
    InitRunner(&GlobalFuncs[THINK_RUNNER]);
    InitIdle(&GlobalFuncs[THINK_IDLE]);
    InitPain(&GlobalFuncs[THINK_PAIN]);
    InitKilled(&GlobalFuncs[THINK_KILLED]);
    InitCurious(&GlobalFuncs[THINK_CURIOUS]);
    InitAlarm(&GlobalFuncs[THINK_ALARM]);
    InitDisguiseSalute(&GlobalFuncs[THINK_DISGUISE_SALUTE]);
    InitDisguiseSentry(&GlobalFuncs[THINK_DISGUISE_SENTRY]);
    InitDisguiseOfficer(&GlobalFuncs[THINK_DISGUISE_OFFICER]);
    InitDisguiseRover(&GlobalFuncs[THINK_DISGUISE_ROVER]);
    InitDisguiseNone(&GlobalFuncs[THINK_DISGUISE_NONE]);
    InitGrenade(&GlobalFuncs[THINK_GRENADE]);
    InitMachineGunner(&GlobalFuncs[THINK_MACHINEGUNNER]);
    InitDogIdle(&GlobalFuncs[THINK_DOG_IDLE]);
    InitDogAttack(&GlobalFuncs[THINK_DOG_ATTACK]);
    InitDogCurious(&GlobalFuncs[THINK_DOG_CURIOUS]);
    InitAnim(&GlobalFuncs[THINK_ANIM]);
    InitAnimCurious(&GlobalFuncs[THINK_ANIM_CURIOUS]);
    InitAim(&GlobalFuncs[THINK_AIM]);
    InitBalconyIdle(&GlobalFuncs[THINK_BALCONY_IDLE]);
    InitBalconyCurious(&GlobalFuncs[THINK_BALCONY_CURIOUS]);
    InitBalconyAttack(&GlobalFuncs[THINK_BALCONY_ATTACK]);
    InitBalconyDisguise(&GlobalFuncs[THINK_BALCONY_DISGUISE]);
    InitBalconyGrenade(&GlobalFuncs[THINK_BALCONY_GRENADE]);
    InitBalconyPain(&GlobalFuncs[THINK_BALCONY_PAIN]);
    InitBalconyKilled(&GlobalFuncs[THINK_BALCONY_KILLED]);
    InitWeaponless(&GlobalFuncs[THINK_WEAPONLESS]);
    InitNoClip(&GlobalFuncs[THINK_NOCLIP]);
    InitDead(&GlobalFuncs[THINK_DEAD]);
    InitBadPlace(&GlobalFuncs[THINK_BADPLACE]);
    InitRunAndShoot(&GlobalFuncs[THINK_RUN_AND_SHOOT]);

    AddWaitTill(STRING_VISIBLE);
    AddWaitTill(STRING_PAIN);
    AddWaitTill(STRING_DEATH);
    AddWaitTill(STRING_HASENEMY);
    AddWaitTill(STRING_FLAGGEDANIMDONE);
    AddWaitTill(STRING_UPPERANIMDONE);
    AddWaitTill(STRING_SAYDONE);
    AddWaitTill(STRING_SOUNDDONE);
    AddWaitTill(STRING_ANIMDONE);
    AddWaitTill(STRING_MOVEDONE);
    AddWaitTill(STRING_TRIGGER);
    AddWaitTill(STRING_MOVE);
    AddWaitTill(STRING_TURNDONE);

    if (developer->integer) {
        Com_Printf("sizeof(Actor) == %zi\n", sizeof(Actor));
        //FIXME: magic ??
        Com_Printf("Magic sizeof actor number: %zd\n", sizeof(Actor) - 64);
    }
}

/*
===============
Actor::UpdateEyeOrigin

Update eye position.
===============
*/
void Actor::UpdateEyeOrigin(void)
{
    Vector eyeTag = vec_zero;

    int currLvlTime = level.inttime;
    int currTime    = m_iEyeUpdateTime;

    if (currLvlTime > currTime) {
        m_iEyeUpdateTime = NextUpdateTime(currTime, 100);

        GetTag("eyes bone", &eyeTag, &m_vEyeDir);

        eyeposition = eyeTag - origin;

        m_vEyeDir[2] = 0.0;
        VectorNormalizeFast(m_vEyeDir);

        if (eyeposition[0] < -10.5) {
            eyeposition[0] = -10.5;
        }
        if (eyeposition[0] > 10.5) {
            eyeposition[0] = 10.5;
        }

        if (eyeposition[1] < -10.5) {
            eyeposition[1] = -10.5;
        }
        if (eyeposition[1] > 10.5) {
            eyeposition[1] = 10.5;
        }

        if (eyeposition[2] < 4.5) {
            eyeposition[2] = 4.5;
        }
        if (eyeposition[2] > 89.5) {
            eyeposition[2] = 89.5;
        }
    }
}

/*
===============
Actor::RequireThink

Do I need to think ?
===============
*/
bool Actor::RequireThink(void)
{
    if (G_GetEntity(0)) {
        return (level.inttime < edict->r.lastNetTime + 60000);
    } else {
        return false;
    }
}

/*
===============
Actor::UpdateEnemyInternal

Real update enemy.
===============
*/
void Actor::UpdateEnemyInternal(void)
{
    for (Sentient *pSent = level.m_HeadSentient[1 - m_Team]; pSent; pSent = pSent->m_NextSentient) {
        m_PotentialEnemies.AddPotentialEnemy(pSent);
    }

    m_PotentialEnemies.CheckEnemies(this);

    if (m_Enemy != m_PotentialEnemies.GetCurrentEnemy()) {
        SetEnemy(m_PotentialEnemies.GetCurrentEnemy(), false);
    }

    m_fNoticeTimeScale += (level.inttime - m_iEnemyCheckTime) / 10000;

    if (m_fNoticeTimeScale > m_fMaxNoticeTimeScale) {
        m_fNoticeTimeScale = m_fMaxNoticeTimeScale;
    }

    m_iEnemyCheckTime = level.inttime;
}

/*
===============
Actor::UpdateEnemy

Update enemy w.r.t m_iEnemyCheckTime.
===============
*/
void Actor::UpdateEnemy(int iMaxDirtyTime)
{
    if (level.inttime > iMaxDirtyTime + m_iEnemyCheckTime) {
        UpdateEnemyInternal();
    }
}

/*
===============
Actor::DetectSmokeGrenades
===============
*/
void Actor::DetectSmokeGrenades(void)
{
    // FIXME: unimplemented
}

/*
===============
Actor::SetEnemy

Set current enemy.
===============
*/
void Actor::SetEnemy(Sentient *pEnemy, bool bForceConfirmed)
{
    if (pEnemy == m_Enemy) {
        return;
    }

    if (m_Enemy) {
        m_Enemy->m_iAttackerCount--;
    }
    m_bNewEnemy = m_Enemy == NULL;
    //delete m_Enemy;

    m_Enemy = pEnemy;

    m_iEnemyChangeTime = level.inttime;

    if (m_Enemy) {
        Event e1(EV_Actor_ShareEnemy);
        //FIXME: macro
        PostEvent(e1, 0.75);

        if (m_Enemy->m_bHasDisguise) {
            if (level.inttime > m_iEnemyVisibleCheckTime) {
                //this cansee call changes m_bEnemyVisible and m_bEnemyInFOV
                //FIXME: macro
                CanSee(m_Enemy, 0, 0.828 * world->farplane_distance, false);
            }
        }

        m_bEnemyIsDisguised = m_Enemy->m_bHasDisguise && (m_Enemy->m_bIsDisguised || !m_bEnemyVisible);

        SetEnemyPos(m_Enemy->origin);

        m_Enemy->m_iAttackerCount++;

        if (bForceConfirmed) {
            m_PotentialEnemies.ConfirmEnemy(this, m_Enemy);
        }
    }
}

/*
===============
Actor::SetEnemyPos

Update stored enemy position information.
===============
*/
void Actor::SetEnemyPos(Vector vPos)
{
    if (m_vLastEnemyPos != vPos) {
        m_iLastEnemyPosChangeTime = level.inttime;
        m_vLastEnemyPos           = vPos;
        mTargetPos                = m_vLastEnemyPos;
        if (m_Enemy) {
            mTargetPos += m_Enemy->eyeposition;
        } else {
            mTargetPos.z += 88;
        }

        if (mTargetPos.z - EyePosition().z < 128) {
            mTargetPos.z -= 16;
        }
    }
}

static bool UnknownAnim(const char *name, dtiki_t *tiki)
{
    ScriptException::next_bIsForAnim = true;
    ScriptError("unknown animation '%s' in '%s'", name, tiki->a->name);
}

/*
===============
Actor::EventSetAnim

Set animation event.
===============
*/
void Actor::EventSetAnim(Event *ev)
{
    //"anim slot weight flagged"
    int       numArgs = ev->NumArgs(), slot = 0, animnum;
    float     weight = 1;
    const_str anim, flagVal;
    qboolean  flagged = qfalse;

    //FIXME: better notation, but this works for now.
    if (numArgs < 1 || numArgs > 4) {
        ScriptError("bad number of arguments");
    } else if (numArgs == 1) {
        anim    = ev->GetConstString(1);
        animnum = gi.Anim_NumForName(edict->tiki, Director.GetString(anim).c_str());
        if (animnum == -1) {
            UnknownAnim(Director.GetString(anim).c_str(), edict->tiki);
        }
    } else if (numArgs == 2) {
        anim    = ev->GetConstString(1);
        animnum = gi.Anim_NumForName(edict->tiki, Director.GetString(anim).c_str());
        if (animnum == -1) {
            UnknownAnim(Director.GetString(anim).c_str(), edict->tiki);
        }
        slot = ev->GetInteger(2);
        if (slot > 2) {
            ScriptError("Bad animation slot, only 0 and 1 supported");
        }
    } else if (numArgs == 3) {
        weight = ev->GetFloat(3);
        if (weight < 0) {
            ScriptError("Negative anim weight not allowed");
        }
        if (weight != 0) {
            slot = ev->GetInteger(2);
            if (slot > 2) {
                ScriptError("Bad animation slot, only 0 and 1 supported");
            }
        }

        anim    = ev->GetConstString(1);
        animnum = gi.Anim_NumForName(edict->tiki, Director.GetString(anim).c_str());
        if (animnum == -1) {
            UnknownAnim(Director.GetString(anim).c_str(), edict->tiki);
        }
    } else if (numArgs == 4) {
        flagVal = ev->GetConstString(4);
        flagged = qtrue;
        if (flagVal != STRING_FLAGGED) {
            ScriptError("unknown keyword '%s', expected 'flagged'", Director.GetString(flagVal).c_str());
        }
        weight = ev->GetFloat(3);
        if (weight < 0) {
            ScriptError("Negative anim weight not allowed");
        }
        if (weight != 0) {
            slot = ev->GetInteger(2);
            if (slot > 2) {
                ScriptError("Bad animation slot, only 0 and 1 supported");
            }
        }

        anim    = ev->GetConstString(1);
        animnum = gi.Anim_NumForName(edict->tiki, Director.GetString(anim).c_str());
        if (animnum == -1) {
            UnknownAnim(Director.GetString(anim).c_str(), edict->tiki);
        }
    }

    if (!slot) {
        flagged = qtrue;
    }
    if (flagged) {
        parm.motionfail = qtrue;
    }
    if (!m_bLevelMotionAnim) {
        if (slot) {
            m_weightType[GetMotionSlot(slot)] = 0;
        } else {
            ChangeMotionAnim();
            m_bMotionAnimSet = true;
        }
        StartMotionAnimSlot(slot, animnum, weight);
        if (flagged) {
            m_iMotionSlot   = GetMotionSlot(slot);
            parm.motionfail = qfalse;
        }
    }
}

/*
===============
Actor::EventEndActionAnim

End action animation.
===============
*/
void Actor::EventEndActionAnim(Event *ev)
{
    parm.upperfail = qtrue;
    if (!m_bLevelActionAnim) {
        ChangeActionAnim();
        parm.upperfail = qfalse;
    }
}

/*
===============
Actor::EventSetMotionAnim

Set motion animation.
===============
*/
void Actor::EventSetMotionAnim(Event *ev)
{
    gi.DPrintf("Actor::EventSetMotionAnim\n");

    if (ev->NumArgs() != 1) {
        ScriptError("bad number of arguments");
    }
    const_str anim    = ev->GetConstString(1);
    str       animstr = Director.GetString(anim);
    int       animnum = gi.Anim_NumForName(edict->tiki, animstr.c_str());
    if (animnum == -1) {
        UnknownAnim(animstr.c_str(), edict->tiki);
    }

    gi.DPrintf("Actor::EventSetMotionAnim %s %d\n", animstr.c_str(), animnum);
    parm.motionfail = qtrue;

    if (!m_bLevelMotionAnim) {
        ChangeMotionAnim();
        m_bMotionAnimSet = true;
        StartMotionAnimSlot(0, animnum, 1.0);
        m_iMotionSlot   = GetMotionSlot(0);
        parm.motionfail = qfalse;
    }
}

/*
===============
Actor::EventSetAimMotionAnim

Set aim motion animation.
===============
*/
void Actor::EventSetAimMotionAnim(Event *ev)
{
    int       anim_crouch, anim_stand, anim_high;
    const_str name;
    if (ev->NumArgs() != 3) {
        ScriptError("bad number of arguments");
    }

    //FIXME: maybe inline func ?

    name        = ev->GetConstString(1);
    anim_crouch = gi.Anim_NumForName(edict->tiki, Director.GetString(name).c_str());
    if (anim_crouch == -1) {
        UnknownAnim(Director.GetString(name).c_str(), edict->tiki);
    }

    name       = ev->GetConstString(2);
    anim_stand = gi.Anim_NumForName(edict->tiki, Director.GetString(name).c_str());
    if (anim_stand == -1) {
        UnknownAnim(Director.GetString(name).c_str(), edict->tiki);
    }

    name      = ev->GetConstString(3);
    anim_high = gi.Anim_NumForName(edict->tiki, Director.GetString(name).c_str());
    if (anim_high == -1) {
        UnknownAnim(Director.GetString(name).c_str(), edict->tiki);
    }

    parm.motionfail = qtrue;

    if (!m_bLevelMotionAnim) {
        ChangeMotionAnim();
        m_bMotionAnimSet = true;
        UpdateAimMotion();
        StartAimMotionAnimSlot(0, anim_crouch);
        StartAimMotionAnimSlot(1, anim_stand);
        StartAimMotionAnimSlot(2, anim_high);
        m_iMotionSlot   = GetMotionSlot(1);
        parm.motionfail = qfalse;
    }
}

/*
===============
Actor::EventSetActionAnim

Set action animation.
===============
*/
void Actor::EventSetActionAnim(Event *ev)
{
    if (ev->NumArgs() != 3) {
        ScriptError("bad number of arguments");
    }

    m_fAimLimit_down = ev->GetFloat(2);
    if (m_fAimLimit_down > 0) {
        m_fAimLimit_down = -0.001f;
        ScriptError("Positive lower_limit not allowed");
    }

    m_fAimLimit_up = ev->GetFloat(3);
    if (m_fAimLimit_up <= 0) {
        m_fAimLimit_up = 0.001f;
        ScriptError("Negative upper_limit not allowed");
    }

    const_str anim    = ev->GetConstString(1);
    int       animnum = gi.Anim_NumForName(edict->tiki, Director.GetString(anim).c_str());
    if (animnum == -1) {
        UnknownAnim(Director.GetString(anim).c_str(), edict->tiki);
    }

    const_str anim2    = ev->GetConstString(2);
    int       animnum2 = gi.Anim_NumForName(edict->tiki, Director.GetString(anim2));
    if (animnum2 == -1) {
        UnknownAnim(Director.GetString(anim2).c_str(), edict->tiki);
    }

    const_str anim3    = ev->GetConstString(3);
    int       animnum3 = gi.Anim_NumForName(edict->tiki, Director.GetString(anim3));
    if (animnum3 == -1) {
        UnknownAnim(Director.GetString(anim3).c_str(), edict->tiki);
    }

    parm.upperfail = qtrue;
    if (!m_bLevelActionAnim) {
        ChangeActionAnim();
        m_bAimAnimSet    = true;
        m_bActionAnimSet = true;
        UpdateAim();

        StartAimAnimSlot(0, animnum);
        StartAimAnimSlot(1, anim2);
        StartAimAnimSlot(2, anim3);
        m_iActionSlot  = GetActionSlot(0);
        parm.upperfail = qfalse;
    }
}

/*
===============
Actor::EventUpperAnim

Set upper body.
===============
*/
void Actor::EventUpperAnim(Event *ev)
{
    if (ev->NumArgs()) {
        if (ev->NumArgs() != 1) {
            ScriptError("bad number of arguments");
        }
        const_str anim    = ev->GetConstString(1);
        int       animnum = gi.Anim_NumForName(edict->tiki, Director.GetString(anim).c_str());
        if (animnum == -1) {
            UnknownAnim(Director.GetString(anim).c_str(), edict->tiki);
        }

        m_csUpperAnim = anim;
    } else if (m_bLevelActionAnim) {
        AnimFinished(m_iActionSlot, true);
    }
}

/*
===============
Actor::EventSetUpperAnim

Set upper body animation.
===============
*/
void Actor::EventSetUpperAnim(Event *ev)
{
    if (ev->NumArgs() != 1) {
        ScriptError("bad number of arguments");
    }

    const_str anim    = ev->GetConstString(1);
    int       animnum = gi.Anim_NumForName(edict->tiki, Director.GetString(anim).c_str());
    if (animnum == -1) {
        UnknownAnim(Director.GetString(anim).c_str(), edict->tiki);
    }

    parm.upperfail = qtrue;
    if (!m_bLevelActionAnim) {
        ChangeActionAnim();
        m_bActionAnimSet = true;
        StartActionAnimSlot(animnum);
        m_iActionSlot  = SimpleActor::GetActionSlot(0);
        parm.upperfail = qfalse;
    }
}

/*
===============
Actor::SoundSayAnim

Returns true if animation not found.
===============
*/
bool Actor::SoundSayAnim(const_str name, byte bLevelSayAnim)
{
    int animnum = gi.Anim_NumForName(edict->tiki, Director.GetString(name).c_str());
    if (animnum == -1) {
        ChangeSayAnim();
        m_bSayAnimSet   = true;
        m_iSaySlot      = -2;
        m_bLevelSayAnim = bLevelSayAnim;

        Com_Printf(
            "Couldn't find animation '%s' in '%s' - trying sound alias instead.\n",
            Director.GetString(name).c_str(),
            edict->tiki->a->name
        );

        Sound(Director.GetString(name), 0, 0, 0, NULL, 0, 1, 1, -1);

        return true;
    }
    return false;
}

/*
===============
Actor::EventIdleSayAnim

Play idle say dialogue.
===============
*/
void Actor::EventIdleSayAnim(Event *ev)
{
    const_str name;
    if (ev->NumArgs()) {
        if (ev->NumArgs() != 1) {
            ScriptError("bad number of arguments");
        }

        name = ev->GetConstString(1);

        if (m_ThinkState <= THINKSTATE_ATTACK || IsGrenadeState(m_ThinkState)) {
            m_csSayAnim = name;
            //FIXME: macro
            m_bNextLevelSayAnim = 1;
        } else {
            if (!SoundSayAnim(name, 1)) {
                m_csSayAnim = name;
                //FIXME: macro
                m_bNextLevelSayAnim = 1;
            }
        }
    } else if (m_bLevelSayAnim == 1) {
        AnimFinished(m_iSaySlot, true);
    }
}

/*
===============
Actor::EventSayAnim

Play idle dialogue.
===============
*/
void Actor::EventSayAnim(Event *ev)
{
    const_str name;
    if (ev->NumArgs()) {
        if (ev->NumArgs() != 1) {
            ScriptError("bad number of arguments");
        }

        name = ev->GetConstString(1);

        if (m_ThinkState <= THINKSTATE_KILLED || !SoundSayAnim(name, 2)) {
            m_csSayAnim = name;
            //FIXME: macro
            m_bNextLevelSayAnim = 2;
            gi.DPrintf("Actor::EventSayAnim: 1 %s\n", targetname.c_str());
        }
        gi.DPrintf("Actor::EventSayAnim: 2 %s\n", targetname.c_str());
    } else if (m_bLevelSayAnim == 1) {
        AnimFinished(m_iSaySlot, true);
        gi.DPrintf("Actor::EventSayAnim: 3 %s\n", targetname.c_str());
    }
    gi.DPrintf("Actor::EventSayAnim: 4 %s\n", targetname.c_str());
}

/*
===============
Actor::EventSetSayAnim

Set say animation.
===============
*/
void Actor::EventSetSayAnim(Event *ev)
{
    const_str name;
    str       sName;
    if (ev->NumArgs() != 1) {
        ScriptError("bad number of arguments");
    }

    if (m_bLevelSayAnim == 0) {
        name = ev->GetConstString(1);

        parm.sayfail = qtrue;
        sName        = Director.GetString(name);
        int animnum  = gi.Anim_NumForName(edict->tiki, sName.c_str());

        Com_Printf("EventSetSayAnim sName: %s, animnum: %d, mVoiceType: %d\n", sName.c_str(), animnum, mVoiceType);
        if (!SoundSayAnim(name, m_bLevelSayAnim)) {
            int flags = gi.Anim_FlagsSkel(edict->tiki, animnum);
            if (flags & TAF_HASUPPER) {
                if (m_bLevelActionAnim) {
                    if (!m_bSayAnimSet) {
                        m_iSaySlot = m_iActionSlot;
                    }
                    return;
                }
                if (flags & TAF_HASDELTA) {
                    if (m_bLevelMotionAnim) {
                        if (!m_bSayAnimSet) {
                            m_iSaySlot = m_iMotionSlot;
                        }
                        return;
                    }
                    ChangeActionAnim();
                    ChangeMotionAnim();
                    StartMotionAnimSlot(0, animnum, 1.0);

                    m_iMotionSlot = m_iActionSlot = GetMotionSlot(0);
                } else {
                    ChangeActionAnim();
                    m_bActionAnimSet = true;
                    StartActionAnimSlot(animnum);
                    m_iActionSlot = GetActionSlot(0);
                }
                ChangeSayAnim();
                m_bSayAnimSet = true;
                m_iSaySlot    = m_iActionSlot;
            } else {
                ChangeSayAnim();
                m_bSayAnimSet = true;
                StartSayAnimSlot(animnum);
                m_iSaySlot = GetSaySlot();
            }
        }
        parm.sayfail = qfalse;
    }
}

/*
===============
Actor::ChangeAnim

Change current animation.
===============
*/

void Actor::ChangeAnim(void)
{
    bool v3; // zf

    if (m_pAnimThread) {
        if (g_scripttrace->integer) {
            if (m_pAnimThread->CanScriptTracePrint()) {
                Com_Printf("--- Change Anim\n");
            }
        }
        m_pAnimThread->AbortRegistration(STRING_EMPTY, this);
        ScriptClass *sc = m_pAnimThread->GetScriptClass();
        if (sc) {
            delete sc;
        }
        Com_Printf("ChangeAnim: m_pAnimThread aborted\n");
    }

    if (m_ThinkState != 4) // THINKSTATE_ATTACK
    {
        if (m_ThinkState <= 4) // THINKSTATE_ATTACK
        {
            if (m_ThinkState >= 2) // THINKSTATE_PAIN
            {
                if (m_bMotionAnimSet) {
                    AnimFinished(m_iMotionSlot, true);
                }
                if (m_bActionAnimSet) {
                    AnimFinished(m_iActionSlot, true);
                }
                v3 = m_bSayAnimSet == 0;
            LABEL_13:
                if (v3) {
                    goto LABEL_14;
                }
                goto LABEL_20;
            }
            goto LABEL_21;
        }
        if (m_ThinkState != 7) // m_ThinkState != THINKSTATE_GRENADE
        {
        LABEL_21:
            if (m_bMotionAnimSet) {
                if (!m_bLevelMotionAnim) {
                    AnimFinished(m_iMotionSlot, true);
                }
            }
            if (m_bActionAnimSet) {
                if (!m_bLevelActionAnim) {
                    AnimFinished(m_iActionSlot, true);
                }
            }
            if (m_bSayAnimSet && !m_bLevelSayAnim) {
            LABEL_20:
                AnimFinished(m_iSaySlot, true);
                goto LABEL_14;
            }
            goto LABEL_14;
        }
    }
    if (m_bMotionAnimSet) {
        AnimFinished(m_iMotionSlot, true);
    }
    if (m_bActionAnimSet) {
        AnimFinished(m_iActionSlot, true);
    }
    if (m_bSayAnimSet) {
        v3 = m_bLevelSayAnim == 2;
        goto LABEL_13;
    }
LABEL_14:
    m_fCrossblendTime = 0.5;
    m_pAnimThread     = m_Anim.Create(this);
    if (m_pAnimThread) {
        if (g_scripttrace->integer) {
            if (m_pAnimThread->CanScriptTracePrint()) {
                Com_Printf("+++ Change Anim\n");
            }
        }
        m_pAnimThread->Register(STRING_EMPTY, this);

        m_pAnimThread->StartTiming();
        Com_Printf("ChangeAnim: m_pAnimThread started\n");
    }
}

/*
void Actor::ChangeAnim
    (
    void
    )
{
    if (m_pAnimThread)
    {
        if (g_scripttrace->integer && m_pAnimThread->CanScriptTracePrint())
        {
            Com_Printf("--- Change Anim\n");
        }
        m_pAnimThread->AbortRegistration(STRING_EMPTY, this);
        ScriptClass * sc = m_pAnimThread->GetScriptClass();
        if (sc)
        {
            delete sc;
        }
        Com_Printf("ChangeAnim: m_pAnimThread aborted\n");
    }
    
    if (m_ThinkState != THINKSTATE_ATTACK)
    {
        if (m_ThinkState > THINKSTATE_ATTACK)
        {
            if (m_ThinkState != THINKSTATE_GRENADE)
            {
                if (m_bMotionAnimSet && !m_bLevelMotionAnim)
                    AnimFinished(m_iMotionSlot, true);
                if (m_bActionAnimSet && !m_bLevelActionAnim)
                    AnimFinished(m_iActionSlot, true);
                if (m_bSayAnimSet && !m_bLevelSayAnim)
                    AnimFinished(m_iSaySlot, true);
            }
            else
            {
                if (m_bMotionAnimSet)
                    AnimFinished(m_iMotionSlot, true);
                if (m_bActionAnimSet)
                    AnimFinished(m_iActionSlot, true);
                if (m_bSayAnimSet && m_bLevelSayAnim != 2)
                    AnimFinished(m_iSaySlot, true);

            }

        }
        else
        {
            if (m_ThinkState < THINKSTATE_PAIN)
            {
                if (m_bMotionAnimSet && !m_bLevelMotionAnim)
                    AnimFinished(m_iMotionSlot, true);
                if (m_bActionAnimSet && !m_bLevelActionAnim)
                    AnimFinished(m_iActionSlot, true);
                if (m_bSayAnimSet && !m_bLevelSayAnim)
                    AnimFinished(m_iSaySlot, true);
            }
            else
            {
                if (m_bMotionAnimSet)
                    AnimFinished(m_iMotionSlot, true);
                if (m_bActionAnimSet)
                    AnimFinished(m_iActionSlot, true);
                if (m_bSayAnimSet)
                    AnimFinished(m_iSaySlot, true);
            }

        }

    }
    else
    {
        if (m_bMotionAnimSet)
            AnimFinished(m_iMotionSlot, true);
        if (m_bActionAnimSet)
            AnimFinished(m_iActionSlot, true);
        if (m_bSayAnimSet && m_bLevelSayAnim != 2)
            AnimFinished(m_iSaySlot, true);
    }

    //FIXME: macro
    m_fCrossblendTime = 0.5;

    m_pAnimThread = m_Anim.Create(this);

    if (m_pAnimThread)
    {
        if (g_scripttrace->integer)
        {
            if (m_pAnimThread->CanScriptTracePrint())
                Com_Printf("+++ Change Anim\n");
        }
        m_pAnimThread->Register(STRING_EMPTY, this);

        m_pAnimThread->StartTiming();
        Com_Printf("ChangeAnim: m_pAnimThread started\n");
    }
}
*/

/*
===============
Actor::UpdateSayAnim

Update say animation.
===============
*/
void Actor::UpdateSayAnim(void)
{
    gi.DPrintf("Actor::UpdateSayAnim 1\n");

    if (m_ThinkState > THINKSTATE_KILLED) {
        gi.DPrintf("Actor::UpdateSayAnim 2\n");
        int animnum = gi.Anim_NumForName(edict->tiki, Director.GetString(m_csSayAnim).c_str());
        if (animnum == -1) {
            return;
        }
        gi.DPrintf("Actor::UpdateSayAnim 3\n");

        int flags = gi.Anim_FlagsSkel(edict->tiki, animnum);

        //FIXME: macro
        if (flags & 256) {
            gi.DPrintf("Actor::UpdateSayAnim 4\n");
            if (!IsAttackState(m_ThinkState) && !IsGrenadeState(m_ThinkState)) {
                ChangeActionAnim();
                if (flags & ANIM_NOACTION) {
                    ChangeMotionAnim();

                    StartMotionAnimSlot(0, animnum, 1);

                    m_bLevelActionAnim = true;
                    m_bLevelMotionAnim = true;

                    m_iMotionSlot = m_iActionSlot = GetMotionSlot(0);
                } else {
                    m_bActionAnimSet = true;

                    StartActionAnimSlot(animnum);

                    m_bLevelActionAnim = true;

                    m_iActionSlot = GetActionSlot(0);
                }
                ChangeSayAnim();
                m_bSayAnimSet       = true;
                m_bLevelSayAnim     = m_bNextLevelSayAnim;
                m_bNextLevelSayAnim = 0;
                m_iSaySlot          = m_iActionSlot;
                return;
            }
            gi.DPrintf("Actor::UpdateSayAnim 4\n");
        } else if (m_bNextLevelSayAnim == 2 || (!IsAttackState(m_ThinkState) && !IsGrenadeState(m_ThinkState))) {
            gi.DPrintf("Actor::UpdateSayAnim 6\n");
            ChangeSayAnim();
            m_bSayAnimSet = true;
            StartSayAnimSlot(animnum);
            m_bLevelSayAnim     = m_bNextLevelSayAnim;
            m_bNextLevelSayAnim = 0;
            m_iSaySlot          = GetSaySlot();
            gi.DPrintf("Actor::UpdateSayAnim 7\n");
            return;
        }
    }

    if (!m_bSayAnimSet) {
        Unregister(STRING_SAYDONE);
        gi.DPrintf("Actor::UpdateSayAnim 8\n");
    }
}

/*
===============
Actor::UpdateUpperAnim

Update upper animation.
===============
*/
void Actor::UpdateUpperAnim(void)
{
    int animnum = gi.Anim_NumForName(edict->tiki, Director.GetString(m_csUpperAnim).c_str());
    if (animnum != -1) {
        if (IsAttackState(m_ThinkState) || IsGrenadeState(m_ThinkState) || IsKilledState(m_ThinkState)
            || IsPainState(m_ThinkState)) {
            if (!m_bActionAnimSet) {
                Unregister(STRING_UPPERANIMDONE);
            }
        } else {
            ChangeActionAnim();

            m_bActionAnimSet = true;

            StartActionAnimSlot(animnum);

            m_bLevelActionAnim = true;
            m_iActionSlot      = SimpleActor::GetActionSlot(0);
        }
    }
}

/*
===============
Actor::UpdateAnim

Update animation.
===============
*/
void Actor::UpdateAnim(void)
{
    m_bAnimating = true;
    SimpleActor::UpdateAim();
    if (SimpleActor::UpdateSelectedAnimation()) {
        Com_Printf("ChangeAnim\n");
        ChangeAnim();
    }
    Director.Unpause();
    Director.Pause();

    if (m_csSayAnim != STRING_EMPTY) {
        UpdateSayAnim();
        m_csSayAnim = STRING_EMPTY;
    }

    if (m_csUpperAnim != STRING_EMPTY) {
        UpdateUpperAnim();
        m_csUpperAnim = STRING_EMPTY;
    }

    if (!m_bSayAnimSet && !m_bDog) {
        SimpleActor::UpdateEmotion();
    }

    //FIXME: macro
    for (int slot = 0; slot < 14; slot++) {
        //FIXME: better notation
        if (!((m_bUpdateAnimDoneFlags >> slot) & 1)) {
            SimpleActor::UpdateAnimSlot(slot);
        }
    }

    float fAnimTime, fAnimWeight, total_weight = 0, time = 0;
    for (int slot = 0; slot < 14; slot++) {
        if ((m_weightType[slot] == 1 || m_weightType[slot] == 4) && animFlags[slot] & ANIM_LOOP) {
            UseSyncTime(slot, 1);
            fAnimTime   = AnimTime(slot);
            fAnimWeight = edict->s.frameInfo[slot].weight;
            time += fAnimTime * fAnimWeight;
            if (std::isinf(time)) {
                Com_Printf(
                    "ent %i, targetname '%s', anim '%s', slot %i, fAnimTime %f, fAnimWeight %f\n",
                    entnum,
                    targetname.c_str(),
                    AnimName(slot),
                    slot,
                    fAnimTime,
                    fAnimWeight
                );
            }
            total_weight += fAnimWeight;
        } else {
            UseSyncTime(slot, 0);
        }
    }

    if (total_weight != 0) {
        SetSyncRate(time / total_weight);
    }

    PostAnimate();
}

/*
===============
Actor::Think

Think function for actor.
Calls think function related to the current thinkstate.
===============
*/
void Actor::Think(void)
{
    int v1, v2, v3, tempHistory, ohIndex;
    m_bAnimating = false;
    if (g_ai->integer && m_bDoAI && edict->tiki) {
        //gi.DPrintf("Actor::Think 1\n");
        Director.iPaused++;

        v1 = level.inttime;
        v2 = v1 / 125;
        v3 = v1 / 125;
        if (v1 / 125 < 0) {
            v3 = v2 + 3;
        }
        tempHistory = v2 - (v3 & 0xFFFFFFFC);

        if (m_iCurrentHistory != tempHistory) {
            m_iCurrentHistory = tempHistory;
            ohIndex           = tempHistory - 1;
            if (ohIndex < 0) {
                ohIndex = 3;
            }

            VectorCopy2D(origin, m_vOriginHistory[ohIndex]);
        }
        if (m_bNoPlayerCollision) {
            Entity *ent = G_GetEntity(0);
            if (!IsTouching(ent)) {
                Com_Printf(
                    "(entnum %d, radnum %d) is going solid after not getting stuck in the player\n", entnum, radnum
                );
                setSolidType(SOLID_BBOX);
                m_bNoPlayerCollision = false;
            }
        }
        //gi.DPrintf("Actor::Think 2\n");
        m_eNextAnimMode = -1;
        FixAIParameters();
        UpdateEnableEnemy();

        if (m_pTetherEnt) {
            m_vHome = m_pTetherEnt->origin;
        }

        //gi.DPrintf("Actor::Think 3\n");
        if (m_bBecomeRunner) {
            parm.movefail = false;
            if (m_ThinkMap[THINKSTATE_IDLE] != THINK_RUNNER && m_ThinkMap[THINKSTATE_IDLE] != THINK_PATROL) {
                parm.movefail = true;
            }
        }

        if (m_bDirtyThinkState) {
            m_bDirtyThinkState = false;
            ThinkStateTransitions();
        }

        //gi.DPrintf("Actor::Think 4\n");
        GlobalFuncs_t *func = &GlobalFuncs[m_Think[m_ThinkLevel]];

        if (func->ThinkState) {
            (this->*func->ThinkState)();
        }

        m_bNeedReload        = false;
        mbBreakSpecialAttack = false;
        Director.Unpause();

        //gi.DPrintf("Actor::Think 5: entnum %d, classname: %s\n", entnum, G_GetEntity(0) ? G_GetEntity(0)->getClassname() : "");
    }
}

void Actor::CheckUnregister(void)
{
    m_bBecomeRunner = false;
    if (parm.movefail) {
        parm.movedone = false;
        Unregister(STRING_MOVEDONE);
    }
    if (m_Enemy) {
        Unregister(STRING_HASENEMY);
    }
    if (m_bEnemyVisible) {
        Unregister(STRING_VISIBLE);
    }
}

/*
===============
Actor::PostThink

Called after think is finished.
===============
*/
void Actor::PostThink(bool bDontFaceWall)
{
    CheckUnregister();
    if (bDontFaceWall) {
        if (!m_pTurnEntity || m_ThinkState != THINKSTATE_IDLE) {
            DontFaceWall();
        }
    }
    UpdateAngles();
    UpdateAnim();
    DoMove();
    //gi.DPrintf("Actor::PostThink 1\n");
    UpdateBoneControllers();
    //gi.DPrintf("Actor::PostThink 2\n");
    UpdateFootsteps();
}

/*
===============
Actor::AnimFinished

===============
*/
void Actor::AnimFinished(int slot, bool stop)
{
    if (stop && slot >= 0) {
        animFlags[slot] |= ANIM_NOACTION;
    }
    if (slot == m_iMotionSlot) {
        if (stop) {
            m_bLevelMotionAnim = false;
            m_iMotionSlot      = -1;
        }
        MPrintf("flagged anim finished slot %d\n", slot);
        Unregister(STRING_FLAGGEDANIMDONE);
    }
    if (slot == m_iActionSlot) {
        if (stop) {
            ChangeActionAnim();
        }
        if (m_csUpperAnim == STRING_EMPTY) {
            MPrintf("upper anim finished slot %d\n", slot);
            Unregister(STRING_UPPERANIMDONE);
        }
    }
    if (slot == m_iSaySlot) {
        if (stop) {
            ChangeSayAnim();
        }
        if (m_csSayAnim == STRING_EMPTY) {
            MPrintf("say anim finished slot %d\n", slot);
            //gi.DPrintf("unregister STRING_SAYDONE\n");
            Unregister(STRING_SAYDONE);
        }
    } else if (slot == GetSaySlot() && stop) {
        ChangeSayAnim();
    }
}

/*
===============
Actor::AnimFinished

===============
*/
void Actor::AnimFinished(int slot)
{
    animFlags[slot] &= ~ANIM_FINISHED;
    AnimFinished(slot, ((animFlags[slot] & ANIM_LOOP) != ANIM_LOOP));
}

/*
===============
Actor::SetThink

Modifies think num of current thinkstate inside m_ThinkMap.
===============
*/
void Actor::SetThink(eThinkState state, eThinkNum think)
{
    m_ThinkMap[state] = think;
    if (m_ThinkState == state) {
        m_bDirtyThinkState = true;
    }
}

/*
===============
Actor::SetThink

Set think to idle.
===============
*/
void Actor::SetThinkIdle(eThinkNum think_idle)
{
    eThinkNum think_curious;

    switch (think_idle) {
    case THINK_PATROL:
    case THINK_RUNNER:
        m_bBecomeRunner = true;
        think_curious   = THINK_CURIOUS;
        break;
    case THINK_MACHINEGUNNER:
        think_curious = THINK_MACHINEGUNNER;
        break;
    case THINK_DOG_IDLE:
        think_curious = THINK_DOG_CURIOUS;
        SetThink(THINKSTATE_GRENADE, think_curious);
        break;
    case THINK_ANIM:
        think_curious = THINK_ANIM_CURIOUS;
        break;
    case THINK_BALCONY_IDLE:
        think_curious = THINK_BALCONY_CURIOUS;
        break;
    default:
        think_curious = THINK_CURIOUS;
        break;
    }

    SetThink(THINKSTATE_IDLE, think_idle);

    SetThink(THINKSTATE_CURIOUS, think_curious);
}

/*
===============
Actor::GetThinkType

Get think for csName.
===============
*/
int Actor::GetThinkType(const_str csName)
{
    int result = 0;
    while (m_csThinkNames[result] != csName) {
        if (++result > NUM_THINKS - 1) {
            ScriptError("unknown think type '%s'", Director.GetString(csName).c_str());
        }
    }
    return result;
}

/*
===============
Actor::SetThinkState

Set thinkstate.
===============
*/
void Actor::SetThinkState(eThinkState state, eThinkLevel level)
{
    if (IsAttackState(state)) {
        m_csIdleMood = STRING_NERVOUS;
        if (m_ThinkMap[THINKSTATE_ATTACK] != THINK_ALARM && m_ThinkMap[THINKSTATE_ATTACK] != THINK_WEAPONLESS
            && m_ThinkMap[THINKSTATE_ATTACK] != THINK_DOG_ATTACK && !GetWeapon(WEAPON_MAIN)) {
            Com_Printf(
                "^~^~^ LD ERROR: (entnum %i, radnum %i, targetname '%s'):    forcing weaponless attack state.\n"
                "^~^~^ Level designers should specify 'type_attack weaponless' for this guy.\n",
                entnum,
                radnum,
                targetname.c_str()
            );
            SetThink(THINKSTATE_ATTACK, THINK_WEAPONLESS);
        }
    }

    m_ThinkStates[level] = state;

    if (level >= m_ThinkLevel) {
        m_bDirtyThinkState = true;
    }
}

/*
===============
Actor::EndCurrentThinkState

End current think state.
===============
*/
void Actor::EndCurrentThinkState(void)
{
    SetThinkState(THINKSTATE_VOID, m_ThinkLevel);
}

/*
===============
Actor::ClearThinkStates

Clear all thinkstates.
===============
*/
void Actor::ClearThinkStates(void)
{
    for (int i = 0; i <= NUM_THINKLEVELS - 1; i++) {
        SetThinkState(THINKSTATE_VOID, (eThinkLevel)i);
    }
}

void Actor::EventSetAlarmNode(Event *ev)
{
    Listener *l = ev->GetListener(1);
    if (l) {
        if (!l->inheritsFrom(&SimpleEntity::ClassInfo)) {
            ScriptError("Alarm node must be an entity");
        }
    }
    m_AlarmNode = (SimpleEntity *)l;
}

void Actor::EventGetAlarmNode(Event *ev)
{
    ev->AddListener(m_AlarmNode);
}

void Actor::EventSetPreAlarmThread(Event *ev)
{
    if (ev->IsFromScript()) {
        m_PreAlarmThread.SetThread(ev->GetValue());
    } else {
        m_PreAlarmThread.Set(ev->GetString(1));
    }
}

void Actor::EventSetAlarmThread(Event *ev)
{
    if (ev->IsFromScript()) {
        m_AlarmThread.SetThread(ev->GetValue());
    } else {
        m_AlarmThread.Set(ev->GetString(1));
    }
}

void Actor::EventGetAlarmThread(Event *ev)
{
    m_AlarmThread.GetScriptValue(&ev->GetValue());
}

void Actor::EventSetSoundAwareness(Event *ev)
{
    m_fSoundAwareness = ev->GetFloat(1);
}

void Actor::EventGetSoundAwareness(Event *ev)
{
    ev->AddFloat(m_fSoundAwareness);
}

void Actor::EventSetGrenadeAwareness(Event *ev)
{
    m_fGrenadeAwareness = ev->GetFloat(1);
}

void Actor::EventGetGrenadeAwareness(Event *ev)
{
    ev->AddFloat(m_fGrenadeAwareness);
}

void Actor::EventSetTypeIdle(Event *ev)
{
    eThinkNum think = (eThinkNum)GetThinkType(ev->GetConstString(1));
    glbs.Printf("EventSetTypeIdle %s : %d\n", ev->GetString(1).c_str(), think);
    if (!Actor::GlobalFuncs[think].IsState(THINKSTATE_IDLE)) {
        SetThinkIdle(THINK_IDLE);
        ScriptError("Invalid idle think '%s'", Director.GetString(m_csThinkNames[think]).c_str());
    }

    SetThinkIdle(think);
}

void Actor::EventGetTypeIdle(Event *ev)
{
    ev->AddConstString(m_csThinkNames[m_ThinkMap[THINKSTATE_IDLE]]);
}

void Actor::EventSetTypeAttack(Event *ev)
{
    eThinkNum think = (eThinkNum)GetThinkType(ev->GetConstString(1));
    glbs.Printf("EventSetTypeAttack %s : %d\n", ev->GetString(1).c_str(), think);
    //check if the wanted think is an attack one.
    if (!Actor::GlobalFuncs[think].IsState(THINKSTATE_ATTACK)) {
        SetThink(THINKSTATE_ATTACK, THINK_TURRET);

        ScriptError("Invalid attack think '%s'", Director.GetString(m_csThinkNames[think]).c_str());
    }

    SetThink(THINKSTATE_ATTACK, think);
}

void Actor::EventGetTypeAttack(Event *ev)
{
    ev->AddConstString(m_csThinkNames[m_ThinkMap[THINKSTATE_ATTACK]]);
}

void Actor::EventSetTypeDisguise(Event *ev)
{
    eThinkNum think = (eThinkNum)GetThinkType(ev->GetConstString(1));
    glbs.Printf("EventSetTypeDisguise %s : %d\n", ev->GetString(1).c_str(), think);
    if (!Actor::GlobalFuncs[think].IsState(THINKSTATE_DISGUISE)) {
        SetThink(THINKSTATE_DISGUISE, THINK_DISGUISE_SALUTE);

        ScriptError("Invalid disguise think '%s'", Director.GetString(m_csThinkNames[think]).c_str());
    }

    SetThink(THINKSTATE_DISGUISE, think);
}

void Actor::EventSetDisguiseAcceptThread(Event *ev)
{
    if (ev->IsFromScript()) {
        m_DisguiseAcceptThread.SetThread(ev->GetValue());
    } else {
        m_DisguiseAcceptThread.Set(ev->GetString(1));
    }
}

void Actor::EventGetDisguiseAcceptThread(Event *ev)
{
    m_DisguiseAcceptThread.GetScriptValue(&ev->GetValue());
}

void Actor::EventGetTypeDisguise(Event *ev)
{
    ev->AddConstString(m_csThinkNames[m_ThinkMap[THINKSTATE_DISGUISE]]);
}

void Actor::EventSetDisguiseLevel(Event *ev)
{
    m_iDisguiseLevel = ev->GetInteger(1);
    if (m_iDisguiseLevel - 1 > 1) {
        m_iDisguiseLevel = 1;
        ScriptError("bad disguise level %d for %s, setting to 1\n", m_iDisguiseLevel, targetname.c_str());
    }
}

void Actor::EventGetDisguiseLevel(Event *ev)
{
    ev->AddInteger(m_iDisguiseLevel);
}

void Actor::EventSetTypeGrenade(Event *ev)
{
    eThinkNum think = (eThinkNum)GetThinkType(ev->GetConstString(1));
    glbs.Printf("EventSetTypeGrenade %s : %d\n", ev->GetString(1).c_str(), think);
    if (!Actor::GlobalFuncs[think].IsState(THINKSTATE_GRENADE)) {
        SetThink(THINKSTATE_GRENADE, THINK_GRENADE);

        ScriptError(
            "Invalid grenade think '%s'", Director.GetString(m_csThinkNames[m_ThinkMap[THINKSTATE_GRENADE]]).c_str()
        );
    }

    SetThink(THINKSTATE_GRENADE, think);
}

void Actor::EventGetTypeGrenade(Event *ev)
{
    ev->AddConstString(m_csThinkNames[m_ThinkMap[THINKSTATE_GRENADE]]);
}

/*
===============
Actor::InitThinkStates

Initialize think related stuff.
===============
*/
void Actor::InitThinkStates(void)
{
    for (size_t i = 0; i < NUM_THINKSTATES; i++) {
        m_ThinkMap[i] = THINK_VOID;
    }
    for (size_t i = 0; i < NUM_THINKLEVELS; i++) {
        m_Think[i]       = THINK_VOID;
        m_ThinkStates[i] = THINKSTATE_VOID;
    }

    m_ThinkMap[THINKSTATE_VOID]     = THINK_VOID;
    m_ThinkMap[THINKSTATE_IDLE]     = THINK_IDLE;
    m_ThinkMap[THINKSTATE_PAIN]     = THINK_PAIN;
    m_ThinkMap[THINKSTATE_KILLED]   = THINK_KILLED;
    m_ThinkMap[THINKSTATE_ATTACK]   = THINK_TURRET;
    m_ThinkMap[THINKSTATE_CURIOUS]  = THINK_CURIOUS;
    m_ThinkMap[THINKSTATE_DISGUISE] = THINK_DISGUISE_SALUTE;
    m_ThinkMap[THINKSTATE_GRENADE]  = THINK_GRENADE;
    m_ThinkMap[THINKSTATE_NOCLIP]   = THINK_NOCLIP;

    m_ThinkLevel       = THINKLEVEL_NORMAL;
    m_ThinkState       = THINKSTATE_VOID;
    m_bDirtyThinkState = false;
}

/*
===============
Actor::DefaultRestart

Default restart of current think state.
===============
*/
void Actor::DefaultRestart(void)
{
    EndState(m_ThinkLevel);
    BeginState();
}

/*
===============
Actor::SuspendState

Suspend current think state.
===============
*/
void Actor::SuspendState(void)
{
    GlobalFuncs_t *func = &GlobalFuncs[m_Think[m_ThinkLevel]];

    if (func->SuspendState) {
        (this->*func->SuspendState)();
    }
}

/*
===============
Actor::ResumeState

Resume current think state.
===============
*/
void Actor::ResumeState(void)
{
    GlobalFuncs_t *func = &GlobalFuncs[m_Think[m_ThinkLevel]];

    if (func->ResumeState) {
        (this->*func->ResumeState)();
    }
}

/*
===============
Actor::BeginState

Begin current think state.
===============
*/
void Actor::BeginState(void)
{
    m_Think[m_ThinkLevel] = m_ThinkMap[m_ThinkState];

    GlobalFuncs_t *func = &GlobalFuncs[m_Think[m_ThinkLevel]];

    if (func->BeginState) {
        (this->*func->BeginState)();
    }

    m_Think[m_ThinkLevel] = m_ThinkMap[m_ThinkState];
}

/*
===============
Actor::EndState

End current think state.
===============
*/
void Actor::EndState(int level)
{
    GlobalFuncs_t *func = &GlobalFuncs[m_Think[level]];

    m_Think[level] = THINK_VOID;

    if (func->EndState) {
        (this->*func->EndState)();
    }

    if (m_pAnimThread) {
        m_pAnimThread->AbortRegistration(STRING_EMPTY, this);
    }
}

/*
===============
Actor::RestartState

Restart current think state.
===============
*/
void Actor::RestartState(void)
{
    GlobalFuncs_t *func = &GlobalFuncs[m_Think[m_ThinkLevel]];

    if (func->RestartState) {
        (this->*func->RestartState)();
    }
}

/*
===============
Actor::UpdateEnableEnemy

Enable/disable enemy based on desired value.
Can be changed from script.
===============
*/
void Actor::UpdateEnableEnemy(void)
{
    if (m_bEnableEnemy != m_bDesiredEnableEnemy) {
        m_bEnableEnemy = m_bDesiredEnableEnemy;

        if (m_bEnableEnemy) {
            SetLeashHome(origin);
        } else {
            if (m_ThinkStates[THINKLEVEL_NORMAL] == THINKSTATE_ATTACK
                || m_ThinkStates[THINKLEVEL_NORMAL] == THINKSTATE_CURIOUS
                || m_ThinkStates[THINKLEVEL_NORMAL] == THINKSTATE_DISGUISE) {
                SetThinkState(THINKSTATE_IDLE, THINKLEVEL_NORMAL);
            }

            SetEnemy(NULL, false);
        }
    }
}

/*
===============
Actor::ThinkStateTransitions

Transition think state and level.
Called when thinkstate/level are change.
===============
*/
void Actor::ThinkStateTransitions(void)
{
    int newThinkLevel;
    int newThinkState;

    //GlobalFuncs_t *func;// = &GlobalFuncs[m_Think[m_ThinkLevel]];

    //loop on all think levels.
    for (newThinkLevel = NUM_THINKLEVELS - 1; newThinkLevel >= 0; newThinkLevel--) {
        newThinkState = m_ThinkStates[newThinkLevel];
        //a new think level is found if it's think state is not THINKSTATE_VOID
        if (newThinkState != THINKSTATE_VOID) {
            break;
        }
    }

    if (m_ThinkMap[newThinkState] == m_Think[m_ThinkLevel]) {
        if (newThinkLevel != m_ThinkLevel || m_ThinkState != newThinkState) {
            m_ThinkLevel           = (eThinkLevel)newThinkLevel;
            m_ThinkState           = (eThinkState)newThinkState;
            m_Think[newThinkLevel] = m_ThinkMap[newThinkState];
        } else {
            RestartState();
        }
    } else {
        if (newThinkLevel > THINKLEVEL_NORMAL) {
            for (int i = 0; i < newThinkLevel; i++) {
                if (m_ThinkStates[i] == THINKSTATE_VOID) {
                    EndState(i);
                }
            }
        }

        if (newThinkLevel > m_ThinkLevel) {
            SuspendState();

            m_ThinkLevel           = (eThinkLevel)newThinkLevel;
            m_ThinkState           = m_ThinkStates[newThinkLevel];
            m_Think[newThinkLevel] = m_ThinkMap[m_ThinkState];

            BeginState();
            m_Think[m_ThinkLevel] = m_ThinkMap[m_ThinkState];
        } else {
            EndState(m_ThinkLevel);

            if (newThinkLevel == m_ThinkLevel) {
                m_ThinkState           = m_ThinkStates[newThinkLevel];
                m_Think[newThinkLevel] = m_ThinkMap[m_ThinkState];

                BeginState();
                m_Think[m_ThinkLevel] = m_ThinkMap[m_ThinkState];
            } else {
                m_ThinkLevel = (eThinkLevel)newThinkLevel;
                m_ThinkState = m_ThinkStates[newThinkLevel];

                if (m_Think[newThinkLevel] != m_ThinkMap[m_ThinkState]) {
                    EndState(newThinkLevel);

                    m_Think[m_ThinkLevel] = m_ThinkMap[m_ThinkState];

                    BeginState();
                    m_Think[m_ThinkLevel] = m_ThinkMap[m_ThinkState];
                } else {
                    ResumeState();
                }
            }
        }
    }
}

/*
===============
Actor::FixAIParameters

Fix path related parameters.
===============
*/
void Actor::FixAIParameters(void)
{
    //FIXME: needs revision(silly mistakes)

    float distmin, distmin2, distmax;
    if (m_pTetherEnt) {
        float leash = 64;
        if (m_pTetherEnt->IsSubclassOfEntity()) {
            leash = m_pTetherEnt->angles.y - m_pTetherEnt->origin.y + m_pTetherEnt->angles.z + m_pTetherEnt->origin.z;
        }
        if (leash <= m_fLeash) {
            distmin = m_fLeash;
        } else {
            Com_Printf(
                "^~^~^ LD ERROR: (entnum %i, radnum %i, targetname '%s'):    increasing leash from %g to %g.\n"
                "^~^~^ Leash must be larger than the size of the entity to which an AI is tethered.\n"
                "\n",
                entnum,
                radnum,
                targetname.c_str(),
                m_fLeash,
                leash
            );
            m_fLeash        = leash;
            distmin         = leash;
            m_fLeashSquared = Square(leash);
        }
    } else {
        distmin = m_fLeash;
    }
    if (m_fMinDistance <= distmin) {
        distmax = m_fMinDistance;
    } else {
        Com_Printf(
            "^~^~^ LD ERROR: (entnum %i, radnum %i, targetname '%s'):    reducing mindist from %g to %g to match "
            "leash.\n"
            "^~^~^ Leash must be greater than mindist, or the AI will want to both run away and stay put.\n"
            "\n",
            entnum,
            radnum,
            targetname.c_str(),
            m_fMinDistance,
            distmin
        );
        m_fMinDistance        = distmin;
        distmax               = distmin;
        m_fMinDistanceSquared = distmin * distmin;
    }
    distmax += 128;
    if (distmax - 1 > m_fMaxDistance) {
        Com_Printf(
            "^~^~^ LD ERROR: (entnum %i, radnum %i, targetname '%s'):    increasing maxdist from %g to %g to exceed "
            "mindist.\n"
            "^~^~^ Maxdist should be %i greater than mindist, or the AI will want to both run away and charge, or just "
            "do oscillatory behavior.\n"
            "\n",
            entnum,
            radnum,
            targetname.c_str(),
            m_fMaxDistance,
            distmax,
            128
        );

        m_fMaxDistance        = distmax;
        m_fMaxDistanceSquared = distmax * distmax;
    }
    if (world->farplane_distance > 0) {
        //FIXME: macro and floats
        if (m_fMaxDistance > world->farplane_distance * 0.828 + 1) {
            Com_Printf(
                "^~^~^ LD ERROR: (entnum %i, radnum %i, targetname '%s'):    reducing maxdist from %g to %g to be %g%% "
                "of farplane.\n"
                "^~^~^ Maxdist should be this distance within fog, or AI will be able to see and attack through fog.\n"
                "\n",
                entnum,
                radnum,
                targetname.c_str(),
                m_fMaxDistance,
                world->farplane_distance * 0.828,
                2.0
            );
            distmin2              = m_fMinDistance;
            m_fMaxDistance        = world->farplane_distance * 0.828;
            m_fMaxDistanceSquared = m_fMaxDistance * m_fMaxDistance;
            if (distmin2 + 128.0 - 1.0 > m_fMaxDistance) {
                Com_Printf(
                    "^~^~^ LD ERROR: (entnum %i, radnum %i, targetname '%s'):    reducing mindist from %g to %g to be "
                    "less than maxdist "
                    "after fog adjustment.\n"
                    "\n",
                    entnum,
                    radnum,
                    targetname.c_str(),
                    distmin2,
                    m_fMaxDistance <= 128 ? 0 : m_fMaxDistance - 128
                );
                m_fMinDistance = m_fMaxDistance - 128.0;
                if (m_fMaxDistance - 128.0 < 0) {
                    m_fMinDistance = 0.0;
                }
                m_fMinDistanceSquared = Square(m_fMinDistance);
            }
        }
    }
}

/*
===============
Actor::AttackEntryAnimation

Returns true if actor stands/draws his weapon.
===============
*/
bool Actor::AttackEntryAnimation(void)
{
    if (m_Enemy) {
        if (level.inttime >= level.m_iAttackEntryAnimTime + 3000) {
            float distSq = (m_Enemy->origin - origin).lengthXYSquared();

            if (m_bNoSurprise || distSq >= Square(256)) {
                if (distSq > Square(1024) && !(rand() & 3)) {
                    if (m_pNextSquadMate != this) {
                        for (auto pSquadMate = m_pNextSquadMate;; pSquadMate = pSquadMate->m_pNextSquadMate) {
                            if (Square(m_fInterval) * Square(2) > (pSquadMate->origin - origin).lengthSquared()) {
                                break;
                            }
                            if (pSquadMate == this) {
                                if (m_bNewEnemy) {
                                    Anim_Say(STRING_ANIM_SAY_SIGHTED_SCR, 200, false);
                                }

                                m_bNoSurprise = true;
                                m_bNewEnemy   = false;
                                return false;
                            }
                        }
                        vec2_t vDelta;
                        vDelta[0] = m_Enemy->origin.x - origin.x;
                        vDelta[1] = m_Enemy->origin.y - origin.y;
                        if (vDelta[0] != 0 || vDelta[1] != 0) {
                            SetDesiredYawDir(vDelta);
                        }

                        SetDesiredLookDir(m_Enemy->origin - origin);

                        m_csNextAnimString = STRING_ANIM_STANDIDENTIFY_SCR;

                        m_bNextForceStart = true;
                        m_bNoSurprise     = true;
                        m_bNewEnemy       = false;

                        return true;
                    }
                }
            } else {
                //FIXME: macro
                if (rand() > distSq * 0.0026041667) //rand() / 0x17F
                {
                    //FIXME: macro
                    StartAnimation(ANIM_MODE_NORMAL, STRING_ANIM_STANDSHOCK_SCR);
                    m_bNoSurprise = true;
                    m_bNewEnemy   = false;

                    return true;
                }
            }
        }

        //FIXME: macro
        if (m_bNewEnemy) {
            Anim_Say(STRING_ANIM_SAY_SIGHTED_SCR, 200, false);
        }

        m_bNoSurprise = true;
        m_bNewEnemy   = false;
    }
    return false;
}

/*
===============
Actor::CheckForThinkStateTransition

Check for all thinkstates transitions.
===============
*/
void Actor::CheckForThinkStateTransition(void)
{
    if (!CheckForTransition(THINKSTATE_GRENADE, THINKLEVEL_NORMAL)) {
        if (!m_bEnableEnemy) {
            CheckForTransition(THINKSTATE_IDLE, THINKLEVEL_NORMAL);
        } else {
            if (!CheckForTransition(THINKSTATE_ATTACK, THINKLEVEL_NORMAL)) {
                if (!CheckForTransition(THINKSTATE_DISGUISE, THINKLEVEL_NORMAL)) {
                    if (CheckForTransition(THINKSTATE_CURIOUS, THINKLEVEL_NORMAL)) {
                        m_pszDebugState = "from_sight";
                    } else {
                        CheckForTransition(THINKSTATE_IDLE, THINKLEVEL_NORMAL);
                    }
                }
            }
        }
    }
}

/*
===============
Actor::CheckForTransition

Check for thinkstate transition.
===============
*/
bool Actor::CheckForTransition(eThinkState state, eThinkLevel level)
{
    GlobalFuncs_t *func;

    if (m_ThinkStates[level] != state) {
        func = &GlobalFuncs[m_ThinkMap[state]];

        if (func->PassesTransitionConditions && (this->*(func->PassesTransitionConditions))()) {
            SetThinkState(state, THINKLEVEL_NORMAL);
            return true;
        }
    }

    return false;
}

/*
===============
Actor::PassesTransitionConditions_Grenade

Should actor transition think state to grenade ?
===============
*/
bool Actor::PassesTransitionConditions_Grenade(void)
{
    if (!m_bLockThinkState && m_bEnableEnemy && m_pGrenade) {
        return m_fGrenadeAwareness >= random();
    }

    return false;
}

/*
===============
Actor::PassesTransitionConditions_BadPlace

Should actor transition think state to bad place?
===============
*/
bool Actor::PassesTransitionConditions_BadPlace(void)
{
    if (m_bLockThinkState) {
        return false;
    }

    return m_iBadPlaceIndex != 0;
}

/*
===============
Actor::PassesTransitionConditions_Attack

Should actor transition think state to attack ?
===============
*/
bool Actor::PassesTransitionConditions_Attack(void)
{
    UpdateEnemy(0);

    if (m_bLockThinkState) {
        return false;
    }

    if (m_Enemy && !(m_Enemy->flags & FL_NOTARGET)) {
        if (EnemyIsDisguised()) {
            return true;
        }

        if (m_PotentialEnemies.GetCurrentVisibility() > 0.999f) {
            return true;
        }
    }

    return false;
}

/*
===============
Actor::PassesTransitionConditions_Disguise

Should actor transition think state to disguise ?
===============
*/
bool Actor::PassesTransitionConditions_Disguise(void)
{
    if (m_bLockThinkState) {
        return false;
    }

    if (m_iNextDisguiseTime > level.inttime) {
        return false;
    }

    m_iNextDisguiseTime = level.inttime + 200;

    UpdateEnemy(200);

    if (!m_Enemy) {
        return false;
    }

    if (EnemyIsDisguised()) {
        {
            if (m_PotentialEnemies.GetCurrentVisibility() > 0.999f) {
                if (fabs(m_Enemy->origin[2] - origin[2]) <= 48.0f) {
                    vec2_t delta;
                    float  fHorzDistanceSquared;

                    VectorSub2D(m_Enemy->origin, origin, delta);

                    fHorzDistanceSquared = VectorLength2DSquared(delta);

                    if (fHorzDistanceSquared > Square(32) && fHorzDistanceSquared < m_fMaxDisguiseDistSquared) {
                        Player *player = (Player *)G_GetEntity(0);
                        Vector  pos    = EyePosition();

                        return G_SightTrace(
                            pos,
                            vec_zero,
                            vec_zero,
                            player->centroid,
                            this,
                            player,
                            MASK_TRANSITION,
                            qfalse,
                            "Actor::PassesTransitionConditions_Disguise"
                        );
                    }
                }
            }
        }
    }

    return false;
}

/*
===============
Actor::PassesTransitionConditions_Curious

Should actor transition think state to curious ?
===============
*/
bool Actor::PassesTransitionConditions_Curious(void)
{
    UpdateEnemy(200);

    if (m_bLockThinkState) {
        return false;
    }

    if (!m_Enemy && m_iCuriousTime) {
        SetCuriousAnimHint(6);
        return true;
    }

    if (m_Enemy && !EnemyIsDisguised() && m_PotentialEnemies.GetCurrentVisibility() <= 0.999f) {
        //THINKSTATE_CURIOUS
        SetCuriousAnimHint(6);
        return true;
    }

    return false;
}

/*
===============
Actor::PassesTransitionConditions_Idle

Should actor transition think state to idle ?
===============
*/
bool Actor::PassesTransitionConditions_Idle(void)
{
    glbs.Printf("PassesTransitionConditions_Idle\n");

    UpdateEnemy(500);

    if (m_bLockThinkState) {
        return false;
    }

    if (!m_Enemy && !m_iCuriousTime) {
        return true;
    }

    return false;
}

void Actor::EventGetRunAnim(Event *ev)
{
    ev->AddConstString(GetRunAnim());
}

void Actor::EventGetWalkAnim(Event *ev)
{
    ev->AddConstString(GetWalkAnim());
}

/*
===============
Actor::PathDistanceAlongVector

Returns projected distance from vDir along path.
===============
*/
float Actor::PathDistanceAlongVector(vec3_t vDir)
{
    Vector vDelta = m_Path.CurrentDelta();

    return DotProduct2D(vDelta, vDir);
}

/*
===============
Actor::FaceEnemyOrMotion


===============
*/
void Actor::FaceEnemyOrMotion(int iTimeIntoMove)
{
    Vector vDelta = origin - m_vLastEnemyPos;
    vDelta.z      = 0;
    if (iTimeIntoMove > 999) {
        if (m_Path.CurrentNodeIndex() != m_sCurrentPathNodeIndex) {
            float fDist  = PathDistanceAlongVector(vDelta);
            m_bFaceEnemy = fDist <= 0 || vDelta.lengthXYSquared() * Square(64) >= Square(fDist);

            m_sCurrentPathNodeIndex = m_Path.CurrentNodeIndex();
        }
    } else {
        m_bFaceEnemy = true;
    }

    if (m_bFaceEnemy && (vDelta.x >= 15.0 || vDelta.x <= -15.0 || vDelta.y >= 15.0 || vDelta.y <= -15.0)) {
        vDelta.x = -vDelta.x;
        vDelta.y = -vDelta.y;
        FaceDirectionDuringMotion(vDelta);
    } else {
        FaceMotion();
    }
}

/*
===============
Actor::FaceDirectionDuringMotion

Face direction during motion.
===============
*/
void Actor::FaceDirectionDuringMotion(vec3_t vLook)
{
    float yaw;

    SetDesiredLookDir(vLook);

    if (velocity[0] != 0 || velocity[1] != 0) {
        float fMagsSquared = VectorLength2DSquared(vLook) * VectorLength2DSquared(velocity);
        float fDot         = DotProduct2D(velocity, vLook);
        float fDotSquared  = Square(fDot);

        yaw = velocity.toYaw();

        float fYaw2;
        if (fMagsSquared * 0.5 < fDotSquared) {
            if (fDot >= 0) {
                fYaw2 = yaw;
            } else if (yaw - 180 >= 0) {
                fYaw2 = yaw - 180;
            } else {
                fYaw2 = yaw - 180 + 360;
            }
        } else {
            if (vLook[0] * velocity[1] <= vLook[1] * velocity[0]) {
                fYaw2 = yaw - 270;
            } else {
                fYaw2 = yaw - 90;
            }

            if (fYaw2 < 0) {
                fYaw2 += 360;
            }
        }

        SetDesiredYaw(fYaw2);
    } else {
        SetDesiredYawDir(vLook);
    }
}

void Actor::EventGetAnimName(Event *ev)
{
    ev->AddConstString(m_csAnimName);
}

void Actor::EventSetAnimName(Event *ev)
{
    m_csAnimName = ev->GetConstString(1);
}

void Actor::EventSetDisguiseRange(Event *ev)
{
    float range               = ev->GetFloat(1);
    m_fMaxDisguiseDistSquared = Square(range);
}

void Actor::EventGetDisguiseRange(Event *ev)
{
    ev->AddFloat(sqrt(m_fMaxDisguiseDistSquared));
}

void Actor::EventSetDisguisePeriod(Event *ev)
{
    m_iDisguisePeriod = ev->GetFloat(1) * 1000 + 0.5;
}

void Actor::EventGetDisguisePeriod(Event *ev)
{
    ev->AddFloat(m_iDisguisePeriod / 1000.0);
}

/*
===============
Actor::FaceMotion

===============
*/
void Actor::FaceMotion(void)
{
    Vector delta;

    if (VectorLength2DSquared(velocity) > 1) {
        vec3_t dir;
        velocity.copyTo(dir);
        delta = origin - m_vOriginHistory[m_iCurrentHistory];

        if (delta.lengthXYSquared() >= 1) {
            if (DotProduct2D(velocity, delta) > 0) {
                delta.copyTo(dir);
            }
        }

        if (IsIdleState(m_ThinkState)) {
            IdleLook(dir);
        } else {
            SetDesiredLookDir(dir);
        }

        SetDesiredYawDir(dir);
    } else if (IsIdleState(m_ThinkState) && m_pLookEntity) {
        LookAtLookEntity();
    } else {
        m_bHasDesiredLookAngles = false;
    }
}

void Actor::ForceAttackPlayer(void)
{
    m_PotentialEnemies.ConfirmEnemy(this, (Sentient *)G_GetEntity(0));
}

void Actor::EventAttackPlayer(Event *ev)
{
    Player *pPlayer = (Player *)G_GetEntity(0);
    if (!pPlayer) {
        ScriptError("player doesn't exist");
    } else {
        m_PotentialEnemies.ConfirmEnemy(this, pPlayer);
        m_bForceAttackPlayer = true;
    }
}

/*
===============
Actor::EventShareEnemy

Share enemy with squad mates.
===============
*/
void Actor::EventShareEnemy(Event *ev)
{
    if (m_Enemy) {
        if (!EnemyIsDisguised()) {
            for (Actor *pSquadMate = (Actor *)m_pNextSquadMate.Pointer(); pSquadMate != this;
                 pSquadMate        = (Actor *)pSquadMate->m_pNextSquadMate.Pointer()) {
                if (pSquadMate->IsSubclassOfActor()) {
                    Vector dist   = pSquadMate->origin - origin;
                    float  distSq = dist * dist;
                    if (pSquadMate->m_fMaxShareDistSquared == 0.0 || distSq <= pSquadMate->m_fMaxShareDistSquared) {
                        pSquadMate->m_PotentialEnemies.ConfirmEnemyIfCanSeeSharerOrEnemy(pSquadMate, this, m_Enemy);
                    }
                }
            }
        }
    }
}

/*
===============
Actor::EventShareGrenade

Share grenade with squad mates.
===============
*/
void Actor::EventShareGrenade(Event *ev)
{
    if (m_pGrenade) {
        for (Actor *pSquadMate = (Actor *)m_pNextSquadMate.Pointer(); pSquadMate != this;
             pSquadMate        = (Actor *)pSquadMate->m_pNextSquadMate.Pointer()) {
            if (pSquadMate->IsSubclassOfActor()) {
                if (!pSquadMate->m_pGrenade) {
                    Vector dist   = pSquadMate->origin - origin;
                    float  distSq = Square(dist);

                    if (distSq < Square(768)) {
                        if (DoesTheoreticPathExist(pSquadMate->origin, 1536)) {
                            pSquadMate->SetGrenade(m_pGrenade);
                        }
                    }
                }
            }
        }
    }
}

/*
===============
Actor::ReceiveAIEvent

Calls proper RecieveAIEvent for current ThinkState.
===============
*/
void Actor::ReceiveAIEvent(
    vec3_t event_origin, int iType, Entity *originator, float fDistSquared, float fMaxDistSquared
)
{
    if (originator != this && originator != GetActiveWeapon(WEAPON_MAIN)) {
        GlobalFuncs_t *func = &GlobalFuncs[m_Think[m_ThinkLevel]];

        if (func->ReceiveAIEvent) {
            (this->*func->ReceiveAIEvent)(event_origin, iType, originator, fDistSquared, fMaxDistSquared);
        }
    }
}

/*
===============
Actor::DefaultReceiveAIEvent

Default AI event handler.
===============
*/
void Actor::DefaultReceiveAIEvent(
    vec3_t event_origin, int iType, Entity *originator, float fDistSquared, float fMaxDistSquared
)
{
    if (IsDead()) {
        return;
    }

    if (originator
        && !originator->IsDead()
        && originator->IsSubclassOfSentient()
        && ((Sentient*)originator)->m_Team == m_Team
        && !IsSquadMate((Sentient*)originator)) {
        MergeWithSquad((Sentient*)originator);
    }

    switch (iType) {
    case AI_EVENT_WEAPON_FIRE:
    case AI_EVENT_WEAPON_IMPACT:
        if (Square(m_fHearing) > fDistSquared) {
            WeaponSound(iType, event_origin, fDistSquared, fMaxDistSquared, originator);
        }
        break;
    case AI_EVENT_EXPLOSION:
    case AI_EVENT_MISC:
    case AI_EVENT_MISC_LOUD:
        if (Square(m_fHearing) > fDistSquared) {
            CuriousSound(iType, event_origin, fDistSquared, fMaxDistSquared);
        }
        break;
    case AI_EVENT_AMERICAN_VOICE:
    case AI_EVENT_GERMAN_VOICE:
    case AI_EVENT_AMERICAN_URGENT:
    case AI_EVENT_GERMAN_URGENT:
        if (Square(m_fHearing) > fDistSquared) {
            VoiceSound(iType, event_origin, fDistSquared, fMaxDistSquared, originator);
        }
        break;
    case AI_EVENT_FOOTSTEP:
        if (Square(m_fHearing) > fDistSquared) {
            FootstepSound(event_origin, fDistSquared, fMaxDistSquared, originator);
        }
        break;
    case AI_EVENT_GRENADE:
        GrenadeNotification(originator);
        break;
    case AI_EVENT_BADPLACE:
        UpdateBadPlaces();
        break;
    default:
    {
        char assertStr[16317] = { 0 };
        strcpy(assertStr, "\"unknown ai_event type\"\n\tMessage: ");
        Q_strcat(assertStr, sizeof(assertStr), DumpCallTrace("iType = %i", iType));
        assert(false && assertStr);
    }
    break;
    }
}

/*
===============
Actor::PriorityForEventType

Returns priority for event type.
===============
*/
int Actor::PriorityForEventType(int iType)
{
    switch (iType) {
    //FIXME: return macros
    case AI_EVENT_WEAPON_FIRE:
        return 7;
    case AI_EVENT_WEAPON_IMPACT:
        return 5;
    case AI_EVENT_EXPLOSION:
        return 6;
    case AI_EVENT_AMERICAN_VOICE:
    case AI_EVENT_GERMAN_VOICE:
        return 3;
    case AI_EVENT_AMERICAN_URGENT:
    case AI_EVENT_GERMAN_URGENT:
        return 4;
    case AI_EVENT_MISC:
        return 1;
    case AI_EVENT_MISC_LOUD:
    case AI_EVENT_FOOTSTEP:
        return 2;
    case AI_EVENT_GRENADE:
        return 8;
    default:
        assert(!"PriorityForEventType: unknown ai_event type");
        return 0;
    }
}

const char *DebugStringForEvent(int iType)
{
    switch (iType) {
    case AI_EVENT_WEAPON_FIRE:
        return "weapon_fire";
        break;
    case AI_EVENT_WEAPON_IMPACT:
        return "weapon_impact";
        break;
    case AI_EVENT_EXPLOSION:
        return "explosion";
        break;
    case AI_EVENT_AMERICAN_VOICE:
        return "american_voice";
        break;
    case AI_EVENT_GERMAN_VOICE:
        return "german_voice";
        break;
    case AI_EVENT_AMERICAN_URGENT:
        return "american_urgent";
        break;
    case AI_EVENT_GERMAN_URGENT:
        return "german_urgent";
        break;
    case AI_EVENT_MISC:
        return "misc";
        break;
    case AI_EVENT_MISC_LOUD:
        return "misc_loud";
        break;
    case AI_EVENT_FOOTSTEP:
        return "footstep";
        break;
    case AI_EVENT_GRENADE:
        return "grenade";
        break;
    default:
        return "????";
        break;
    }
}

/*
===============
Actor::CuriousSound

Handles curious sound.
===============
*/
void Actor::CuriousSound(int iType, vec3_t sound_origin, float fDistSquared, float fMaxDistSquared)
{
    float v7, v8, fRangeFactor = 1.0;
    int   iPriority;
    if (m_bEnableEnemy) {
        if (m_ThinkStates[THINKLEVEL_NORMAL] == THINKSTATE_IDLE
            || m_ThinkStates[THINKLEVEL_NORMAL] == THINKSTATE_CURIOUS) {
            //FIXME: name variables.
            if (fMaxDistSquared != 0.0) {
                v7 = 1 * (1.0 / 3) - fDistSquared * (1 * (1.0 / 3)) / fMaxDistSquared;
                if (v7 <= 1.0) {
                    fRangeFactor = v7;
                }
            }
            v8 = fRangeFactor * m_fSoundAwareness;
            //FIXME: macro
            if (v8 >= rand() * 0.000000046566129) {
                RaiseAlertnessForEventType(iType);
                iPriority = PriorityForEventType(iType);
                if (iPriority >= m_iCuriousLevel) {
                    m_iCuriousLevel = iPriority;
                    if (iType == AI_EVENT_WEAPON_IMPACT) {
                        //FIXME: macro
                        if (fDistSquared <= Square(192)) {
                            SetCuriousAnimHint(1);
                        }
                    } else if (iType > AI_EVENT_WEAPON_IMPACT) {
                        if (iType == AI_EVENT_EXPLOSION) {
                            //FIXME: macro
                            if (fDistSquared <= Square(768)) {
                                SetCuriousAnimHint(3);
                            }
                        } else {
                            SetCuriousAnimHint(5);
                        }
                    } else if (iType == AI_EVENT_WEAPON_FIRE) {
                        //FIXME: macro
                        if (fDistSquared <= Square(512)) {
                            SetCuriousAnimHint(2);
                        }
                    } else {
                        SetCuriousAnimHint(5);
                    }

                    SetEnemyPos(sound_origin);

                    EndCurrentThinkState();
                    SetThinkState(THINKSTATE_CURIOUS, THINKLEVEL_NORMAL);

                    m_pszDebugState = G_AIEventStringFromType(iType);
                }
            }
        }
    }
}

/*
===============
Actor::WeaponSound

Handles weapon sound.
===============
*/
void Actor::WeaponSound(int iType, vec3_t sound_origin, float fDistSquared, float fMaxDistSquared, Entity *originator)
{
    Sentient *pOwner;
    gi.Printf("Actor::WeaponSound");
    if (originator->IsSubclassOfWeapon()) {
        pOwner = ((Weapon *)originator)->GetOwner();
    } else {
        if (!originator->IsSubclassOfProjectile()) {
            char assertStr[16317] = {0};
            strcpy(assertStr, "\"Actor::WeaponSound: non-weapon made a weapon sound.\\n\"\n\tMessage: ");
            Q_strcat(assertStr, sizeof(assertStr), DumpCallTrace("class = %s", originator->getClassname()));
            assert(false && assertStr);

            return;
        }
        pOwner = ((Projectile *)originator)->GetOwner();
    }

    if (!pOwner) {
        return;
    }
    Sentient *pEnemy;

    pEnemy = pOwner->m_Enemy;
    if (pOwner->m_Team == m_Team) {
        if (!pEnemy) {
            if (pOwner->IsSubclassOfActor()) {
                Actor *pAOwner = (Actor *)pOwner;
                //FIXME: macro
                if (originator->IsSubclassOfWeapon() && pAOwner->m_Think[0] == THINK_MACHINEGUNNER) {
                    Weapon *pWOriginator = (Weapon *)originator;
                    if (pWOriginator->aim_target) {
                        if (pWOriginator->aim_target->IsSubclassOfSentient()) {
                            if (((Sentient *)(pWOriginator->aim_target.Pointer()))->m_Team != pAOwner->m_Team) {
                                pEnemy = (Sentient *)(pWOriginator->aim_target.Pointer());
                            }
                        } else if (pAOwner->m_Team == TEAM_GERMAN) {
                            for (pEnemy = level.m_HeadSentient[1];; pEnemy = pEnemy->m_NextSentient) {
                                if (!pEnemy) {
                                    return;
                                }
                                Vector vDist = pEnemy->centroid - pWOriginator->aim_target->origin;
                                if (vDist.lengthSquared() <= 2304 || vDist.lengthSquared() != 2304) {
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
        if (!pEnemy) {
            return;
        }
    } else {
        if (!pEnemy) {
            return;
        }
    }

    //v14 = &this->m_PotentialEnemies;
    if (m_PotentialEnemies.CaresAboutPerfectInfo(pEnemy)) {
        float fDist = sqrt(fDistSquared);
        if (NoticeShot(pOwner, pEnemy, fDist)) {
            m_PotentialEnemies.ConfirmEnemy(this, pOwner);
            CuriousSound(iType, sound_origin, fDistSquared, fMaxDistSquared);
        }
    }
}

/*
===============
Actor::FootstepSound

Handles footstep sound.
===============
*/
void Actor::FootstepSound(vec3_t sound_origin, float fDistSquared, float fMaxDistSquared, Entity *originator)
{
    if (originator->IsSubclassOfSentient()) {
        if ((m_ThinkStates[THINKLEVEL_NORMAL] == THINKSTATE_IDLE
             || m_ThinkStates[THINKLEVEL_NORMAL] == THINKSTATE_CURIOUS)
            && m_bEnableEnemy) {
            if (NoticeFootstep((Sentient *)originator)) {
                CuriousSound(AI_EVENT_FOOTSTEP, sound_origin, fDistSquared, fMaxDistSquared);
            }
        }
    } else {
        char assertStr[16317] = {0};
        strcpy(assertStr, "\"'ai_event footstep' in a tiki used by something besides AI or player.\\n\"\n\tMessage: ");
        Q_strcat(assertStr, sizeof(assertStr), DumpCallTrace(""));
        assert(false && assertStr);
    }
}

/*
===============
Actor::VoiceSound

Handles voice sound.
===============
*/
void Actor::VoiceSound(int iType, vec3_t sound_origin, float fDistSquared, float fMaxDistSquared, Entity *originator)
{
    bool bFriendly;
    //FIXME: macros
    if ((m_ThinkStates[THINKLEVEL_NORMAL] != THINKSTATE_IDLE && m_ThinkStates[THINKLEVEL_NORMAL] != THINKSTATE_CURIOUS)
        || !m_bEnableEnemy) {
        return;
    }

    bFriendly = m_Team == TEAM_GERMAN;

    if (iType <= AI_EVENT_GERMAN_VOICE) {
        assert(iType == AI_EVENT_AMERICAN_VOICE);
    } else {
        assert(iType <= AI_EVENT_GERMAN_URGENT);

        if (iType == AI_EVENT_AMERICAN_URGENT) {
            bFriendly = m_Team == TEAM_AMERICAN;
        }
    }

    if (bFriendly) {
        if (NoticeVoice((Sentient *)originator)) {
            CuriousSound(iType, sound_origin, fDistSquared, fMaxDistSquared);
        }
    }
}

/*
===============
Actor::GrenadeNotification

Handles grenade notification.
===============
*/
void Actor::GrenadeNotification(Entity *originator)
{
    if (!m_pGrenade) {
        if (CanSeeFOV(originator)) {
            assert(originator->IsSubclassOfProjectile());

            SetGrenade(originator);
            Event e1(EV_Actor_ShareGrenade);
            PostEvent(e1, 0.5);

            Anim_Say(STRING_ANIM_SAY_GRENADE_SIGHTED_SCR, 0, true);
        }
    }

    if (m_pGrenade == originator) {
        float  fGrav, fDeltaZ, fVelDivGrav, fTimeLand;
        vec2_t vLand;
        vec2_t vMove;

        fGrav = sv_gravity->value * m_pGrenade->gravity;

        fDeltaZ = originator->origin[2] - origin[2];

        fVelDivGrav = originator->velocity[2] / fGrav;

        //from s=v*t + 0.5*a*t^2, apply for z velocity, solve.
        fTimeLand = fVelDivGrav + sqrt((fDeltaZ + fDeltaZ) / fGrav + Square(fVelDivGrav));

        VectorMA2D(originator->origin, fTimeLand, originator->velocity, vLand);

        VectorSub2D(m_vGrenadePos, vLand, vMove);

        if (VectorLength2D(vMove) > 16) {
            m_vGrenadePos     = Vector(vLand[0], vLand[1], origin[2]);
            m_bGrenadeBounced = true;
        }
    }
}

/*
===============
Actor::SetGrenade

Set current grenade.
===============
*/
void Actor::SetGrenade(Entity *pGrenade)
{
    m_pGrenade = pGrenade;

    m_bGrenadeBounced = true;

    m_iFirstGrenadeTime = level.inttime;

    m_vGrenadePos = pGrenade->origin;
}

/*
===============
Actor::UpdateBadPlaces

Update bad places, with a new path
===============
*/
void Actor::UpdateBadPlaces(void)
{
    m_iBadPlaceIndex = 0;
    if (!m_bIgnoreBadPlace) {
        if (PathExists()) {
            m_Path.ReFindPath(origin, this);
        }

        m_iBadPlaceIndex = level.GetNearestBadPlace(origin, 64, 1 << m_Team);
    }
}

/*
===============
Actor::NotifySquadmateKilled

Handle squadmate killed notification.
===============
*/
void Actor::NotifySquadmateKilled(Sentient *pSquadMate, Sentient *pAttacker)
{
    if (m_PotentialEnemies.CaresAboutPerfectInfo(pAttacker)) {
        Vector dist   = pSquadMate->origin - origin;
        float  distSq = dist * dist;
        if (distSq < m_fSight * m_fSight) {
            bool shouldConfirm = false;
            if (gi.AreasConnected(edict->r.areanum, pSquadMate->edict->r.areanum)) {
                shouldConfirm = G_SightTrace(
                    EyePosition(),
                    vec_zero,
                    vec_zero,
                    pSquadMate->EyePosition(),
                    this,
                    pSquadMate,
                    MASK_TRANSITION,
                    qfalse,
                    "Actor::NotifySquadmateKilled"
                );
            }
            //FIXME: macro for that constant
            if (!shouldConfirm) {
                if (distSq <= Square(768)) {
                    Vector start, end;
                    if (origin.z <= pSquadMate->origin.z) {
                        start = origin;
                        end   = pSquadMate->origin;
                    } else {
                        start = pSquadMate->origin;
                        end   = origin;
                    }
                    shouldConfirm = m_Path.DoesTheoreticPathExist(start, end, this, 1536, 0, 0);
                }
            }
            if (shouldConfirm) {
                m_PotentialEnemies.ConfirmEnemy(this, pAttacker);
            }
        }
    }
}

/*
===============
Actor::RaiseAlertnessForEventType

Raise alertness(enemy notice) for specifc event.
===============
*/
void Actor::RaiseAlertnessForEventType(int iType)
{
    float fAmount;

    switch (iType) {
    case AI_EVENT_WEAPON_FIRE:
        fAmount = 0.2f;
    case AI_EVENT_WEAPON_IMPACT:
        fAmount = 0.1f;
    case AI_EVENT_EXPLOSION:
        fAmount = 0.4f;
    case AI_EVENT_AMERICAN_VOICE:
    case AI_EVENT_AMERICAN_URGENT:
        if (m_Team == TEAM_AMERICAN) {
            return;
        }
        fAmount = 0.25f;
    case AI_EVENT_MISC:
        fAmount = 0.02f;
    case AI_EVENT_MISC_LOUD:
    case AI_EVENT_FOOTSTEP:
        fAmount = 0.05f;
    case AI_EVENT_GRENADE:
        fAmount = 0.04f;
        break;
    case AI_EVENT_GERMAN_VOICE:
    case AI_EVENT_GERMAN_URGENT:
        if (m_Team == TEAM_AMERICAN) {
            RaiseAlertness(0.25);
        }
    default:
        char assertStr[16317] = {0};
        strcpy(assertStr, "\"Actor::RaiseAlertnessForEventType: unknown event type\\n\"\n\tMessage: ");
        Q_strcat(assertStr, sizeof(assertStr), DumpCallTrace(""));
        assert(false && assertStr);
        return;
        break;
    }
    RaiseAlertness(fAmount);
}

/*
===============
Actor::RaiseAlertness

Lower enemy notice time by fAmount.
===============
*/
void Actor::RaiseAlertness(float fAmount)
{
    float fMaxAmount;

    fMaxAmount = m_fNoticeTimeScale * 2 / 3;

    m_fNoticeTimeScale -= fAmount <= fMaxAmount ? fAmount : fMaxAmount;
}

/*
===============
Actor::NoticeShot

Returns true if shooter is an enemy.
Otherwise the shooter is a team mate and target is registered an enemy if he's visible/reachable.
===============
*/
bool Actor::NoticeShot(Sentient *pShooter, Sentient *pTarget, float fDist)
{
    gi.Printf("Notice shot");
    if (pShooter->m_Team != m_Team) {
        return true;
    }

    m_bEnemyIsDisguised = false;

    if (pTarget) {
        if (DoesTheoreticPathExist(pShooter->origin, fDist * 1.5)
            || CanSee(pTarget, 0, 0.828 * world->farplane_distance, false)) {
            m_PotentialEnemies.ConfirmEnemy(this, pTarget);
        }
    }

    return false;
}

/*
===============
Actor::NoticeFootstep

Returns true if actor should notice this footstep sound (pedestrian is not visible for actor).
Otherwise returns false.
===============
*/
bool Actor::NoticeFootstep(Sentient *pPedestrian)
{
    if (m_Team == pPedestrian->m_Team || pPedestrian->m_bIsDisguised) {
        return false;
    }

    return !CanSeeFOV(pPedestrian);
}

/*
===============
Actor::NoticeVoice

Returns true if actor should notice this voice sound (vocalist is not visible for actor).
Otherwise returns false.
===============
*/
bool Actor::NoticeVoice(Sentient *pVocallist)
{
    if (IsSquadMate(pVocallist)) {
        return false;
    }

    return !CanSeeFOV(pVocallist);
}

/*
===============
Actor::ValidGrenadePath

Returns true if grenade trajectory is valid.
i.e grenade can get from vFrom to vTo with vVel with any obstacles.
===============
*/
bool Actor::ValidGrenadePath(const Vector& vFrom, const Vector& vTo, Vector& vVel)
{
    //FIXME: macros
    float  fGravity, fTime1, fTime2, fTime3, fTimeLand;
    Vector mins(-4, -4, -4);
    Vector maxs(4, 4, 4);

    Vector vPoint1, vPoint2, vPoint3;

    if (vVel.lengthSquared() > 589824) {
        return false;
    }
    fGravity = 0.8 * sv_gravity->value;

    fTime1 = vVel.z * fGravity / 0.5;

    vPoint1.x = vVel.x * fTime1 + vFrom.x;
    vPoint1.y = vVel.y * fTime1 + vFrom.y;
    vPoint1.z = vVel.z * fTime1 * 0.75 + vFrom.z;

    maxs.z = fGravity * 0.125 * fTime1 * fTime1 + 4;

    if (ai_debug_grenades->integer) {
        G_DebugLine(vFrom, vPoint1, 1.0, 0.5, 0.5, 1.0);
    }

    if (G_SightTrace(
            vFrom,
            mins,
            maxs,
            vPoint1,
            this,
            NULL,
            //FIXME: macro
            1107569409,
            qfalse,
            "Actor::ValidGrenadePath 1"
        )) {
        fTime2 = fTime1 + fTime1;

        vPoint2.x = vVel.x * fTime2 + vFrom.x;
        vPoint2.y = vVel.y * fTime2 + vFrom.y;
        vPoint2.z = vVel.z * fTime2 * 0.5 + vFrom.z;

        if (ai_debug_grenades->integer) {
            G_DebugLine(vPoint1, vPoint2, 1.0, 0.5, 0.5, 1.0);
        }
        if (G_SightTrace(
                vPoint1,
                mins,
                maxs,
                vPoint2,
                this,
                NULL,
                //FIXME: macro
                1107569409,
                qfalse,
                "Actor::ValidGrenadePath 2"
            )) {
            if (fabs(vVel.y) >= fabs(vVel.x)) {
                fTimeLand = (vTo.y - vFrom.y) / vVel.y;
            } else {
                fTimeLand = (vTo.x - vFrom.x) / vVel.x;
            }

            fTime3 = (fTime2 + fTimeLand) / 2;
            maxs.z = fGravity * 0.03125 * (fTimeLand - fTime2) * (fTimeLand - fTime2) + 4;

            vPoint3.x = vVel.x * fTime3 + vFrom.x;
            vPoint3.y = vVel.y * fTime3 + vFrom.y;
            vPoint3.z = (vVel.z - fGravity * 0.5 * fTime3) * fTime3 + vFrom.z;

            if (ai_debug_grenades->integer) {
                G_DebugLine(vPoint2, vPoint3, 1.0, 0.5, 0.5, 1.0);
            }
            if (G_SightTrace(
                    vPoint2,
                    mins,
                    maxs,
                    vPoint3,
                    this,
                    NULL,
                    //FIXME: macro
                    1107569409,
                    qfalse,
                    "Actor::ValidGrenadePath 3"
                )) {
                if (ai_debug_grenades->integer) {
                    G_DebugLine(vPoint3, vTo, 1.0, 0.5, 0.5, 1.0);
                }
                trace_t trace =
                    G_Trace(vPoint3, mins, maxs, vTo, this, 1107569409, qfalse, "Actor::ValidGrenadePath 4");
                if (!trace.allsolid) {
                    if (trace.ent) {
                        if (trace.ent->entity->IsSubclassOfSentient()) {
                            Sentient *sen = (Sentient *)trace.ent->entity;
                            if (sen->m_Team != m_Team) {
                                return true;
                            }
                        }
                    }
                } else if (trace.entityNum == 1022 && trace.plane.normal[2] >= 1) {
                    return true;
                }
            }
        }
    }
    return false;
}

/*
===============
Actor::CalcThrowVelocity

Calculates required grenade throw velocity to get grenade from vFrom to vTo.
===============
*/
Vector Actor::CalcThrowVelocity(const Vector& vFrom, const Vector& vTo)
{
    Vector ret;
    Vector vDelta;
    float  v10; // ST0C_4
    float  v11; // fst3
    //////
    float fGravity;
    float fVelHorz;
    float fVelVert;
    float fDeltaZ;
    float fDistance;
    float fHorzDistSquared;
    float fOOTime;

    fGravity = 0.8 * sv_gravity->value;

    vDelta = vTo - vFrom;

    fVelHorz = vDelta.x;
    fVelVert = vDelta.y;
    fDeltaZ  = vDelta.z;

    fHorzDistSquared = fVelVert * fVelVert + fVelHorz * fVelHorz;

    fDistance = sqrt(fDeltaZ * fDeltaZ + fHorzDistSquared);

    // original irl equation: v10 = sqrt(fGravity * 0.5 * fHorzDistSquared / (fDistance * trigMult ))
    // trigMult = (cos(th)/ tan(al) - sin(th)/tanSquared(al))
    // al = inital velocity angle with ground plane.
    // th = angle between vDelta and ground plane.
    // mohaa devs decided to let trigMult be 1, for the sake of simplicity I guess.
    v10 = sqrt(fGravity * 0.5 * fHorzDistSquared / fDistance);

    // no I dea what this means.
    // maybe it's related to their angle choice.
    // calculates the 1/distanceSquared necessary for fOOTime calculation.
    v11 = (fDistance + fDeltaZ) / (fDistance - fDeltaZ) / fHorzDistSquared;

    // 1/(speed * sqrt(1/distanceSquared))
    // 1/(speed * 1/distance)
    // 1/(1/time)
    // time
    fOOTime = 1 / (v10 * sqrt(v11));

    ret.z = v10;
    ret.x = fVelHorz / fOOTime;
    ret.y = fVelVert / fOOTime;
    return ret;
}

/*
===============
Actor::CanThrowGrenade

Returns required velocity to throw grenade from vFrom to vTo.
Or vec_zero if it's not possible.
===============
*/
Vector Actor::CanThrowGrenade(const Vector& vFrom, const Vector& vTo)
{
    Vector vVel = vec_zero;

    vVel = CalcThrowVelocity(vFrom, vTo);
    if (vVel == vec_zero || !ValidGrenadePath(vFrom, vTo, vVel)) {
        return vec_zero;
    } else {
        return vVel;
    }
}

/*
===============
Actor::CalcRollVelocity

Calculates required grenade roll velocity to get grenade from vFrom to vTo.
Roll here means a low toss.
===============
*/
Vector Actor::CalcRollVelocity(const Vector& vFrom, const Vector& vTo)
{
    float  fOOTime, fVelVert, fVelHorz, fGravity;
    Vector vDelta = vFrom - vTo, vRet;

    // you must throw from above.
    // start point must be above (higher) than end point.
    if (vDelta.z > 0) {
        fGravity = 0.8 * sv_gravity->value;

        // Required down velocity to hit the ground twice as fast as freefall time.
        vRet.z = sqrt(vDelta.z * fGravity);

        // accel = speed / time, hence : time = speed / accel, 0.21961521 is an arbitary scalar.
        // since the scalar is way less than 1, it will take more time to hit the ground than to arrive to target dest.
        // this is kinda like a low toss rather than a roll. if I understand correctly.
        fOOTime = vRet.z / fGravity * 0.21961521;

        // speed = distance / time
        vRet.x = fVelHorz = -vDelta.x / fOOTime;
        vRet.y = fVelVert = -vDelta.y / fOOTime;

        return vRet;
    } else {
        return vec_zero;
    }
}

/*
===============
Actor::CanRollGrenade

Returns required velocity to roll grenade from vFrom to vTo.
Or vec_zero if it's not possible.
Roll here means a low toss.
===============
*/
Vector Actor::CanRollGrenade(const Vector& vFrom, const Vector& vTo)
{
    Vector vVel = vec_zero;
    vVel        = CalcRollVelocity(vFrom, vTo);
    if (vVel == vec_zero || !ValidGrenadePath(vFrom, vTo, vVel)) {
        return vec_zero;
    } else {
        return vVel;
    }
}

/*
===============
Actor::CanTossGrenadeThroughHint

Returns true if actor can toss grenade through specified hint.
pvVel and peMode are modified.
===============
*/
bool Actor::CanTossGrenadeThroughHint(
    GrenadeHint *pHint, const Vector& vFrom, const Vector& vTo, bool bDesperate, Vector *pvVel, eGrenadeTossMode *peMode
)
{
    float fGravity = 0.8 * sv_gravity->value;
    float fAngle, fTemp, fTemp2, fTemp3, fTemp4, /*fVelVert, fVelHorz,*/ fRange, fRangeSquared, fDist, fDistSquared,
        fHeight;
    bool bSuccess = false;

    Vector vDelta, vHintDelta;

    if (!bDesperate) {
        vHintDelta = pHint->origin - vFrom;
        vDelta     = vFrom - vTo;

        fDist        = vDelta.lengthXY();
        fDistSquared = fDist * fDist;

        fRange = vHintDelta.z * fDist - vDelta.z * vHintDelta.lengthXY();

        if (fRange == 0) {
            return false;
        }
        fTemp4 = 1 / fRange;
        fRange = (fDist - vHintDelta.lengthXY()) / fRange;

        if (fRange <= 0) {
            return false;
        }

        fTemp  = sqrt(fRange * (fDist * (fGravity * 0.5 * vHintDelta.lengthXY())));
        fTemp2 = (vHintDelta.z * fDistSquared - vDelta.z * vHintDelta.lengthXYSquared()) * fGravity * 0.5;
        fTemp3 = fTemp / vHintDelta.lengthXY();

        pvVel->x = vHintDelta.x * fTemp3;
        pvVel->y = vHintDelta.y * fTemp3;
        pvVel->z = 0.5 * fTemp2 / fTemp * fTemp4;

        *peMode = AI_GREN_TOSS_HINT;

        return ValidGrenadePath(vFrom, pHint->origin, *pvVel);
    }

    vHintDelta = pHint->origin - vFrom;

    fRange        = vHintDelta.lengthXY();
    fRangeSquared = fRange * fRange;

    if (fRangeSquared != 0) {
        fDist   = vHintDelta.length();
        fHeight = fRangeSquared * fGravity / 589824.0 + vHintDelta.z;
        if (-fHeight <= -fDist) {
            fAngle = (atan2(vHintDelta.z / fRangeSquared, 1.0)
                      + atan2(fHeight / fDist, sqrt(1.0 - fHeight / fDist * (fHeight / fDist))))
                   / 2;
            pvVel->x = vHintDelta.x * 768.0 / fRangeSquared * cos(fAngle);
            pvVel->y = vHintDelta.y * 768.0 / fRangeSquared * cos(fAngle);
            pvVel->z = 768.0 * cos(fAngle);
            return ValidGrenadePath(vFrom, pHint->origin, *pvVel);
        }
    }

    return bSuccess;
}

/*
===============
Actor::GrenadeThrowPoint

Returns real grenade throw point.
===============
*/
Vector Actor::GrenadeThrowPoint(const Vector& vFrom, const Vector& vDelta, const_str csAnim)
{
    vec2_t axis;
    Vector vRet;

    VectorCopy2D(vDelta, axis);
    VectorNormalize2D(axis);

    if (csAnim == STRING_ANIM_GRENADEKICK_SCR) {
        vRet = vFrom;
        vRet.z += 8;
        return vRet;
    } else {
        if (csAnim > STRING_ANIM_GRENADEKICK_SCR) {
            if (csAnim != STRING_ANIM_GRENADETHROW_SCR && csAnim != STRING_ANIM_GRENADETOSS_SCR) {
                vRet = vFrom;
                vRet.z += 80;
                return vRet;
            } else {
                vRet.x = vFrom.x - axis[0] * 34 + axis[1] * 8;
                vRet.y = vFrom.y - axis[0] * 8 - axis[1] * 34;
                vRet.z = 52 + vFrom.z;
                return vRet;
            }
        } else {
            if (csAnim != STRING_ANIM_GRENADERETURN_SCR) {
                vRet = vFrom;
                vRet.z += 80;
                return vRet;
            } else {
                vRet.x = vFrom.x + axis[0] * 25 + axis[1] * 2;
                vRet.y = vFrom.y - axis[0] * 2 + axis[1] * 25;
                vRet.z = 89 + vFrom.z;
                return vRet;
            }
        }
    }
}

/*
===============
Actor::CalcKickVelocity

Calculates required grenade kick velocity.
===============
*/
Vector Actor::CalcKickVelocity(Vector& vDelta, float fDist) const
{
    float  fScale, fGravity;
    Vector ret;
    float  v4;

    fGravity = 0.8 * sv_gravity->value;

    fScale = 0.57735032 * fDist;
    v4     = sqrt(fGravity * 0.5 / (fScale - vDelta[2]));
    ret[0] = vDelta[0] * v4;
    ret[1] = vDelta[1] * v4;
    ret[2] = fScale * v4;

    return ret;
}

/*
===============
Actor::CanKickGrenade

Returns true if actor can kick grenade from vFrom to vTo.
Or false if it's not possible.
pvVel is changed.
===============
*/
bool Actor::CanKickGrenade(Vector& vFrom, Vector& vTo, Vector& vFace, Vector *pvVel)
{
    Vector vEnd, vStart, vDelta, vVel;
    float  fDist, fGravity, fScale;
    fGravity = sv_gravity->value * 0.8;
    if (fGravity <= 0.0) {
        return false;
    }

    vStart = GrenadeThrowPoint(vFrom, vFace, STRING_ANIM_GRENADEKICK_SCR);
    vDelta = vTo - vStart;

    fDist = vDelta.lengthXY();
    if (vDelta.z >= 0 || Vector::Dot(vDelta, vFace) < 0.0 || fDist < 256 || fDist >= 255401.28 / fGravity + 192.0) {
        return false;
    }

    if (fDist < 512) {
        fScale = 192.0 / fDist + 0.25;
    } else {
        fScale = 1.0 - 192.0 / fDist;
    }

    vEnd = vStart + vDelta;

    vDelta[0] *= fScale;
    vDelta[1] *= fScale;

    *pvVel = CalcKickVelocity(vDelta, fScale * fDist);

    if (*pvVel == vec_zero || !ValidGrenadePath(vStart, vEnd, *pvVel)) {
        return false;
    } else {
        return true;
    }
}

/*
===============
Actor::GrenadeWillHurtTeamAt

Returns true if grenade will hurt team at vTo.
===============
*/
bool Actor::GrenadeWillHurtTeamAt(const Vector& vTo)
{
    if (m_pNextSquadMate == this) {
        return false;
    }

    for (Actor *pSquadMate = (Actor *)m_pNextSquadMate.Pointer(); pSquadMate != this;
         pSquadMate        = (Actor *)pSquadMate->m_pNextSquadMate.Pointer()) {
        if ((pSquadMate->origin - vTo).length() <= 65536) {
            return true;
        }
    }
    return false;
}

/*
===============
Actor::CanGetGrenadeFromAToB

Returns true if actor can get a grenade from vFrom to vTo.
pvVel is the kick/roll/throw velocity.
peMode is the possible toss mode.
===============
*/
bool Actor::CanGetGrenadeFromAToB(
    const Vector& vFrom, const Vector& vTo, bool bDesperate, Vector *pvVel, eGrenadeTossMode *peMode
)
{
    float        fDot;
    Vector       vHint;
    Vector       vStart;
    Vector       vAxisY;
    Vector       vAxisX;
    Vector       vDelta;
    Vector       vDelta2 = vec_zero;
    int          nHints;
    GrenadeHint *apHint[4];
    float        fRangeSquared;

    if (sv_gravity->value <= 0) {
        return false;
    }

    vDelta        = vTo - vFrom;
    fRangeSquared = vDelta.lengthSquared();

    //range < 256
    if (fRangeSquared < Square(256)) {
        return false;
    }
    if (bDesperate) {
        vStart = GrenadeThrowPoint(vFrom, vDelta, STRING_ANIM_GRENADERETURN_SCR);
    }

    //range < 32
    if (fRangeSquared < Square(1024)) {
        if (!bDesperate) {
            vStart = GrenadeThrowPoint(vFrom, vDelta, STRING_ANIM_GRENADETOSS_SCR);
        }

        *pvVel = CanRollGrenade(vStart, vTo);

        if (*pvVel != vec_zero) {
            *peMode = AI_GREN_TOSS_ROLL;
            return true;
        }

        if (!bDesperate) {
            vStart = GrenadeThrowPoint(vFrom, vDelta, STRING_ANIM_GRENADETHROW_SCR);
        }

        *pvVel = CanThrowGrenade(vStart, vTo);

        if (*pvVel != vec_zero) {
            *peMode = AI_GREN_TOSS_THROW;
            return true;
        }
    }

    if (!bDesperate) {
        return false;
    }

    nHints = GrenadeHint::GetClosestSet(apHint, 4, vStart, Square(1024));

    for (int i = 0; i <= nHints; i++) {
        if (bDesperate) {
            vStart = GrenadeThrowPoint(vFrom, vDelta2, STRING_ANIM_GRENADERETURN_SCR);
            if (CanTossGrenadeThroughHint(apHint[i], vStart, vTo, bDesperate, pvVel, peMode)) {
                return true;
            }
            continue;
        }

        vHint   = apHint[i]->origin;
        vDelta2 = vHint - vFrom;

        fDot = DotProduct2D(vDelta2, vDelta);
        if (fDot >= 0 && fDot * fDot <= vDelta.lengthXYSquared() * vDelta2.lengthXYSquared() * 0.89999998) {
            vStart = GrenadeThrowPoint(vFrom, vDelta2, STRING_ANIM_GRENADETOSS_SCR);
            if (CanTossGrenadeThroughHint(apHint[i], vStart, vTo, bDesperate, pvVel, peMode)) {
                return true;
            }
        }
    }

    *peMode = AI_GREN_TOSS_NONE;
    return false;
}

/*
===============
Actor::DecideToThrowGrenade

Returns true if actor will throw grenade to vTo.
pvVel is the kick/roll/throw velocity.
peMode is the toss mode.
===============
*/
bool Actor::DecideToThrowGrenade(const Vector& vTo, Vector *pvVel, eGrenadeTossMode *peMode)
{
    if (Sentient::AmmoCount("grenade")) {
        if (!GrenadeWillHurtTeamAt(vTo)) {
            return CanGetGrenadeFromAToB(origin, vTo, false, pvVel, peMode);
        }
    }
    return false;
}

/*
===============
Actor::GenericGrenadeTossThink

Called when actor in grenade state.
i.e. actor noticed a grenade and must think fast.
===============
*/
void Actor::GenericGrenadeTossThink(void)
{
    Vector           vGrenadeVel = vec_zero;
    eGrenadeTossMode eGrenadeMode;

    if (m_Enemy && level.inttime >= m_iStateTime + 200) {
        if (CanGetGrenadeFromAToB(origin, m_Enemy->velocity - m_Enemy->origin, false, &vGrenadeVel, &eGrenadeMode)) {
            m_vGrenadeVel  = vGrenadeVel;
            m_eGrenadeMode = eGrenadeMode;
        }
        m_iStateTime = level.inttime;
    }

    SetDesiredYawDir(m_vGrenadeVel);
    ContinueAnimation();
}

/*
===============
Actor::Grenade_EventFire

Throw grenade animation
===============
*/
void Actor::Grenade_EventFire(Event *ev)
{
    const_str csAnim;
    str       strGrenade;
    float     speed;
    Vector    pos, dir, vStart;

    gi.Tag_NumForName(edict->tiki, "tag_weapon_right");

    if (m_eGrenadeMode == AI_GREN_TOSS_ROLL) {
        csAnim = STRING_ANIM_GRENADETHROW_SCR;
    } else {
        csAnim = STRING_ANIM_GRENADETOSS_SCR;
    }

    vStart = GrenadeThrowPoint(origin, orientation[0], csAnim);

    dir = m_vGrenadeVel;

    speed = VectorNormalize(dir);

    if (m_Team == TEAM_AMERICAN) {
        strGrenade = "models/projectiles/M2FGrenade.tik";
    } else {
        strGrenade = "models/projectiles/steilhandgranate.tik";
    }

    ProjectileAttack(vStart, dir, this, strGrenade, 0, speed);

    UseAmmo("grenade", 1);
}

void Actor::EventSetTurret(Event *ev)
{
    Listener *l = ev->GetListener(1);
    if (l && l->inheritsFrom(&TurretGun::ClassInfo)) {
        m_pTurret = (TurretGun *)l;
    }
}

void Actor::EventGetTurret(Event *ev)
{
    ev->AddListener(m_pTurret);
}

void Actor::EventGetAmmoGrenade(Event *ev)
{
    ev->AddInteger(AmmoCount("grenade"));
}

void Actor::EventSetAmmoGrenade(Event *ev)
{
    GiveAmmo("grenade", ev->GetInteger(1));
}

void Actor::EventEnableEnemy(Event *ev)
{
    m_bDesiredEnableEnemy = ev->GetBoolean(1);
}

void Actor::EventEnablePain(Event *ev)
{
    m_bEnablePain = ev->GetBoolean(1);
}

void Actor::EventActivate(Event *ev)
{
    m_bPatrolWaitTrigger = false;
    Unregister(STRING_TRIGGER);
}

void Actor::EventInterruptPoint(Event *ev)
{
    GlobalFuncs_t *func = &GlobalFuncs[m_Think[m_ThinkLevel]];

    if (func->PostShoot) {
        (this->*func->PostShoot)();
    }
}

void Actor::EventGetVisibilityThreshold(Event *ev)
{
    ev->AddFloat(m_fVisibilityThreshold);
}

void Actor::EventSetVisibilityThreshold(Event *ev)
{
    float threshold;

    threshold = ev->GetFloat(1);

    if (threshold < 0 || threshold > 1) {
        ScriptError("DefaultNonVisLevel must be in range 0-1");
    }

    m_bHasVisibilityThreshold = true;
    m_fVisibilityThreshold    = threshold;
}

void Actor::EventSetDefaultVisibilityThreshold(Event *ev)
{
    float threshold;

    threshold = ev->GetFloat(1);

    if (threshold < 0 || threshold > 1) {
        ScriptError("DefaultNonVisLevel must be in range 0-1");
    }

    if (!m_bHasVisibilityThreshold) {
        m_fVisibilityThreshold = threshold;
    }
}

void Actor::EventGetSuppressChance(Event *ev)
{
    ev->AddInteger(m_iSuppressChance);
}

void Actor::EventSetSuppressChance(Event *ev)
{
    // FIXME: unimplemented
}

void Actor::EventAnimScript(Event *ev)
{
    m_bAnimScriptSet = true;
    m_csAnimScript   = ev->GetConstString(1);
    m_AnimMode       = ANIM_MODE_NORMAL;

    SetThinkIdle(THINK_ANIM);
}

void Actor::EventAnimScript_Scripted(Event *ev)
{
    m_csAnimScript   = ev->GetConstString(1);
    m_bAnimScriptSet = true;
    m_AnimMode       = ANIM_MODE_SCRIPTED;

    SetThinkIdle(THINK_ANIM);
}

void Actor::EventAnimScript_Noclip(Event *ev)
{
    m_csAnimScript   = ev->GetConstString(1);
    m_bAnimScriptSet = true;
    m_AnimMode       = ANIM_MODE_NOCLIP;

    SetThinkIdle(THINK_ANIM);
}

void Actor::EventAnimScript_Attached(Event *ev)
{
    m_csAnimScript   = ev->GetConstString(1);
    m_bAnimScriptSet = true;
    m_AnimMode       = ANIM_MODE_ATTACHED;

    SetThinkIdle(THINK_ANIM);
}

void Actor::EventReload_mg42(Event *ev)
{
    //FIXME: macos/enum
    if (m_State != 1201) {
        if (m_State == 1200) {
            m_bAnimScriptSet = true;
            TransitionState(1201, 0);
        } else {
            Unregister(STRING_ANIMDONE);
        }
    }
}

bool Actor::CanMovePathWithLeash(void) const
{
    vec2_t delta;

    if (PathExists()) {
        delta[0] = origin[0] - m_vHome[0];
        delta[1] = origin[1] - m_vHome[1];

        return (VectorLength2DSquared(delta) < m_fLeashSquared) || (DotProduct2D(delta, m_Path.CurrentDelta()) < 0.0f);
    }

    return false;
}

bool Actor::MovePathWithLeash(void)
{
    if (CanMovePathWithLeash()) {
        Anim_RunToInOpen(2);
        FaceMotion();
        return true;
    } else {
        Anim_Aim();
        return false;
    }
}

Vector Actor::GunTarget(bool bNoCollision)
{
    float          accuracy;
    static bool    doInit = true;
    static cvar_t *aiRanges[4];
    Vector         vRet;

    if (doInit) {
        aiRanges[0] = gi.Cvar_Get("g_aishortrange", "500", 0);
        aiRanges[1] = gi.Cvar_Get("g_aimediumrange", "700", 0);
        aiRanges[2] = gi.Cvar_Get("g_ailongrange", "1000", 0);
        aiRanges[3] = gi.Cvar_Get("g_aisniperrange", "2200", 0);
        doInit      = false;
    }

    if (mTargetPos == vec_zero) {
        AnglesToAxis(origin, orientation);
        vRet = Vector(orientation[0]) * 2048 + EyePosition();
        return vRet;
    }
    float scatterMult = 1;

    Player *player = NULL;
    if (m_Enemy) {
        if (m_Enemy->IsSubclassOfPlayer()) {
            player = (Player *)m_Enemy.Pointer();

            Weapon *weap = GetActiveWeapon(WEAPON_MAIN);
            Vector  dir;
            dir = player->centroid - centroid;

            if (weap) {
                MPrintf("WEAPON RANGE: %f  dist: %f\n", aiRanges[weap->mAIRange]->value, dir.length());
                scatterMult = dir.length() / aiRanges[weap->mAIRange]->value;
                if (dir.length() > aiRanges[weap->mAIRange]->value) {
                    if (scatterMult > 1) {
                        accuracy = mAccuracy / scatterMult;
                    }
                    MPrintf("OUT OF RANGE - scatterMult: %f\n", scatterMult);
                }
            } else {
                char assertStr[16317] = {0};
                strcpy(assertStr, "\"ERROR Actor::GunTarget without a weapon\\n\"\n\tMessage: ");
                Q_strcat(assertStr, sizeof(assertStr), DumpCallTrace(""));
                assert(false && assertStr);
            }
        } else if (fabs(m_Enemy->origin[2] - origin[2]) >= 128) {
            scatterMult = 0.5;
        }
    }

    if (random2() > accuracy) {
        MPrintf("MISS - scatterMult: %f\n", scatterMult);

        if (player) {
            static cvar_t *scatterHeight;
            static cvar_t *scatterWide;
            static bool    doInitScatters = false;
            int            lastTrail;

            lastTrail = player->mCurTrailOrigin - 1;
            if (lastTrail < 0) {
                lastTrail = 1;
            }

            vRet = player->mvTrail[lastTrail];
            //continue here:
            //Player *tPlayer = player + 12*lastTrail;//weird
            if (!G_SightTrace(
                    GunPosition(),
                    vec_zero,
                    vec_zero,
                    player->mvTrail[lastTrail],
                    this,
                    player,
                    33819417,
                    0,
                    "Actor::GunTarget 1"
                )) {
                vRet = player->mvTrailEyes[lastTrail];
            }
            if (!doInitScatters) {
                scatterWide    = gi.Cvar_Get("g_aiScatterWide", "16.0", 0);
                scatterHeight  = gi.Cvar_Get("g_aiScatterHeight", "45.0", 0);
                doInitScatters = true;
            }
            vRet.x += crandom() * scatterWide->value * scatterMult;
            vRet.y += crandom() * scatterWide->value * scatterMult;
            vRet.z += crandom() * scatterHeight->value * scatterMult;

            if (player && player->GetVehicleTank()) {
                vRet.z -= 128;
            }
        } else {
            int sign;

            if (random() > 0.5) {
                sign = 1;
            } else {
                sign = -1;
            }
            vRet.x = sign * 32 * (random() + 1);

            if (random() > 0.5) {
                sign = 1;
            } else {
                sign = -1;
            }
            vRet.y = sign * 32 * (random() + 1);

            if (random() > 0.5) {
                sign = 1;
            } else {
                sign = -1;
            }
            vRet.z = sign * 48 * (random() + 1);

            vRet += mTargetPos;

            if (player && player->GetVehicleTank()) {
                vRet.z -= 128;
            }
        }
        return vRet;
    }

    MPrintf("HIT\n");

    if (player) {
        vRet = player->centroid;
    } else {
        vRet = mTargetPos;
    }

    if (m_Enemy) {
        if (!G_SightTrace(
                GunPosition(), vec_zero, vec_zero, vRet, this, m_Enemy, 33819417, qfalse, "Actor::GunTarget 1"
            )) {
            vRet = EyePosition();
        }
    }

    if (player && player->GetVehicleTank()) {
        vRet.z -= 128;
    }
    return vRet;
}

qboolean Actor::setModel(void)
{
    str headModel, headSkin, weapon;
    headModel = level.GetRandomHeadModel(model);
    headSkin  = level.GetRandomHeadSkin(model);
    //qboolean bSuccess;
    str name = "", tempstr;

    if (model != "") {
        if (m_csLoadOut != STRING_EMPTY) {
            name = "weapon|" + Director.GetString(m_csLoadOut) + "|";
        }

        if (m_csHeadModel == STRING_EMPTY) {
            m_csHeadModel = Director.AddString(level.GetRandomHeadModel(model));
        }

        if (m_csHeadModel != STRING_EMPTY) {
            name += "headmodel|" + Director.GetString(m_csHeadModel) + "|";
        }

        if (m_csHeadSkin == STRING_EMPTY) {
            m_csHeadSkin = Director.AddString(level.GetRandomHeadSkin(model));
        }

        if (m_csHeadSkin != STRING_EMPTY) {
            name += "headskin|" + Director.GetString(m_csHeadSkin) + "|"; //FIXME: not sure of this last "|"
        }
        name += model;
    }
    gi.DPrintf2(
        "Actor::setModel(): name: %s, model: %s, headModel: %s, headSkin: %s\n",
        name.c_str(),
        model.c_str(),
        headModel.c_str(),
        headSkin.c_str()
    );
    level.skel_index[edict->s.number] = -1;
    return gi.setmodel(edict, name);
}

void Actor::EventSetHeadModel(Event *ev)
{
    m_csHeadModel = ev->GetConstString(1);
    setModel();
}

void Actor::EventGetHeadModel(Event *ev)
{
    ev->AddConstString(m_csHeadModel);
}

void Actor::EventSetHeadSkin(Event *ev)
{
    m_csHeadSkin = ev->GetConstString(1);
    setModel();
}

void Actor::EventGetHeadSkin(Event *ev)
{
    ev->AddConstString(m_csHeadSkin);
}

void Actor::SetPathWithLeash(Vector vDestPos, const char *description, int iMaxDirtyTime)
{
    vec2_t dist   = {vDestPos[0] - m_vHome[0], vDestPos[1] - m_vHome[1]};
    float  distSq = DotProduct2D(dist, dist);
    if (distSq <= m_fLeashSquared) {
        dist[0] = origin[0] - m_vHome[0];
        dist[1] = origin[1] - m_vHome[1];
        distSq  = DotProduct2D(dist, dist);
        if (distSq <= m_fLeashSquared) {
            SetPath(vDestPos, description, iMaxDirtyTime, m_vHome, m_fLeashSquared);
        }
    } else {
        ClearPath();
    }
}

void Actor::SetPathWithLeash(SimpleEntity *pDestNode, const char *description, int iMaxDirtyTime)
{
    if (pDestNode) {
        SetPathWithLeash(pDestNode->origin, description, iMaxDirtyTime);
    } else {
        if (m_bPathErrorTime + 5000 < level.inttime) {
            m_bPathErrorTime = level.inttime;
            Com_Printf(
                "^~^~^ No destination node specified for '%s' at (%f %f %f)\n",
                targetname.c_str(),
                origin[0],
                origin[1],
                origin[2]
            );
        }
        ClearPath();
    }
}

void Actor::FindPathAwayWithLeash(vec3_t vAwayFrom, vec3_t vDirPreferred, float fMinSafeDist)
{
    m_Path.FindPathAway(origin, vAwayFrom, vDirPreferred, this, fMinSafeDist, m_vHome, m_fLeashSquared);
    ShortenPathToAvoidSquadMates();
}

void Actor::FindPathNearWithLeash(vec3_t vNearbyTo, float fCloseDistSquared)
{
    vec2_t dist   = {vNearbyTo[0] - m_vHome[0], vNearbyTo[1] - m_vHome[1]};
    float  distSq = DotProduct2D(dist, dist);

    if (distSq <= m_fLeashSquared + fCloseDistSquared + 2 * sqrt(m_fLeashSquared * fCloseDistSquared)) {
        dist[0] = origin[0] - m_vHome[0];
        dist[1] = origin[1] - m_vHome[1];
        distSq  = DotProduct2D(dist, dist);
        if (distSq <= m_fLeashSquared) {
            m_Path.FindPathNear(origin, vNearbyTo, this, 0, fCloseDistSquared, m_vHome, m_fLeashSquared);
            ShortenPathToAvoidSquadMates();
        }
    }
}

/*
===============
Actor::GetAntiBunchPoint

===============
*/
Vector Actor::GetAntiBunchPoint(void)
{
    int    count = 0;
    Vector ret   = vec_origin;
    if (m_pNextSquadMate != this) {
        for (auto pSquadMate = m_pNextSquadMate; pSquadMate != this; pSquadMate = pSquadMate->m_pNextSquadMate) {
            Vector dist   = origin - pSquadMate->origin;
            float  distSq = dist * dist;
            if (distSq != 0) {
                if (Square(m_fInterval) > distSq) {
                    count++;
                    ret += origin + (dist / sqrt(distSq)) * (m_fInterval - sqrt(distSq));
                }
            }
        }

        if (count) {
            if (count != 1) {
                ret /= count;
            }
        } else {
            ret = origin;
        }
    }
    return ret;
}

bool Actor::AutoArchiveModel(void)
{
    return false;
}

/*
===============
Actor::AddToBodyQue

Add this to body queue.
===============
*/
void Actor::AddToBodyQue(void)
{
    SafePtr<Actor> lastActor = mBodyQueue[mCurBody + 3];

    if (lastActor) {
        Event ev(EV_Remove);
        lastActor->PostEvent(ev, 0);
    }

    mBodyQueue[mCurBody] = this;

    //update current body index
    mCurBody = (mCurBody + 1) % 5;
}

/*
===============
Actor::ResetBodyQueue

Clear body queue.
Called upon Level::Cleanup()
===============
*/
void Actor::ResetBodyQueue(void)
{
    //weird useless loop ?

    for (int i = MAX_BODYQUEUE - 1; i >= 0; i--) {
        //do nothing.
    }
    //mBodyQueue
    //this resets curBody, but all the magic happens in AddToBodyQue
    mCurBody = 0;
}

void Actor::EventSetNoIdle(Event *ev)
{
    m_bNoIdleAfterAnim = ev->GetInteger(1);
}

void Actor::EventGetNoIdle(Event *ev)
{
    ev->AddInteger(m_bNoIdleAfterAnim);
}

void Actor::EventGetEnemy(Event *ev)
{
    ev->AddListener(m_Enemy);
}

void Actor::EventSetMaxNoticeTimeScale(Event *ev)
{
    float noticeScale; // fst7

    noticeScale = ev->GetFloat(1);
    if (noticeScale <= 0.0) {
        Com_Printf("^~^~^ ERROR: noticescale: value must be greater than 0\n");
    } else {
        m_fMaxNoticeTimeScale = noticeScale / 100;
    }
}

void Actor::EventGetMaxNoticeTimeScale(Event *ev)
{
    ev->AddFloat(m_fMaxNoticeTimeScale * 100);
}

void Actor::EventSetFixedLeash(Event *ev)
{
    m_bFixedLeash = ev->GetBoolean(1);
}

void Actor::EventGetFixedLeash(Event *ev)
{
    ev->AddInteger(m_bFixedLeash);
}

void Actor::Holster(void)
{
    if (activeWeaponList[0]) {
        DeactivateWeapon(WEAPON_MAIN);
    }
}

void Actor::HolsterOffHand(void)
{
    if (activeWeaponList[WEAPON_OFFHAND]) {
        DeactivateWeapon(WEAPON_OFFHAND);
    }
}

void Actor::Unholster(void)
{
    Weapon *weap;

    if (!activeWeaponList[WEAPON_MAIN]) {
        weap = GetWeapon(WEAPON_MAIN);
        if (weap) {
            useWeapon(weap, WEAPON_MAIN);
            ActivateNewWeapon();
        }
    }
}

void Actor::UnholsterOffHand(void)
{
    Weapon *weap;

    if (!activeWeaponList[WEAPON_OFFHAND]) {
        weap = GetWeapon(WEAPON_OFFHAND);
        if (weap) {
            useWeapon(weap, WEAPON_OFFHAND);
            ActivateNewWeapon();
        }
    }
}

void Actor::EventHolster(Event *ev)
{
    if (activeWeaponList[WEAPON_MAIN]) {
        DeactivateWeapon(WEAPON_MAIN);
    }
}

void Actor::EventUnholster(Event *ev)
{
    Unholster();
}

void Actor::EventIsEnemyVisible(Event *ev)
{
    ev->AddInteger(m_bEnemyVisible);
}

void Actor::EventGetEnemyVisibleChangeTime(Event *ev)
{
    //FIXME: /100 ??
    ev->AddFloat(m_iEnemyVisibleChangeTime / 100);
}

void Actor::EventGetLastEnemyVisibleTime(Event *ev)
{
    //FIXME: /100 ??
    ev->AddFloat(m_iLastEnemyVisibleTime / 100);
}

void Actor::EventSoundDone(Event *ev)
{
    //soundChannel_t
    int channel = ev->GetInteger(1);
    str name    = ev->GetString(2);
    if (gi.S_IsSoundPlaying(channel, name)) {
        Event e1(EV_SoundDone);
        e1.AddInteger(channel);
        e1.AddString(name);
        PostEvent(e1, level.frametime);
    } else if (m_bSayAnimSet && m_iSaySlot == -2) //FIXME: macro
    {
        ChangeSayAnim();
        if (m_csSayAnim == STRING_EMPTY) {
            //gi.DPrintf("unregister STRING_SAYDONE\n");
            Unregister(STRING_SAYDONE);
        }
    } else {
        Unregister(STRING_SOUNDDONE);
    }
}

void Actor::EventSound(Event *ev)
{
    ProcessSoundEvent(ev, m_Team == TEAM_AMERICAN);
}

void Actor::EventSetFallHeight(Event *ev)
{
    float fHeight = ev->GetFloat(1);

    if (fHeight < MIN_FALLHEIGHT) {
        ScriptError("value less than %d not allowed", MIN_FALLHEIGHT);
    }
    if (fHeight > MAX_FALLHEIGHT) {
        ScriptError("value greater than %d not allowed", MAX_FALLHEIGHT);
    }

    m_Path.SetFallHeight(fHeight);
}

void Actor::EventGetFallHeight(Event *ev)
{
    ev->AddFloat(m_Path.GetFallHeight());
}

void Actor::EventCanMoveTo(Event *ev)
{
    float  fIntervalSquared, fDistSquared;
    vec2_t vDelta;
    Vector vDest;

    vDest = ev->GetVector(1);
    VectorSub2D(vDest, m_vHome, vDelta);
    fDistSquared = VectorLength2DSquared(vDelta);
    if (fDistSquared >= m_fLeashSquared) {
        ev->AddInteger(0);
        return;
    } else if (m_Enemy != NULL) {
        VectorSub2D(vDest, m_Enemy->origin, vDelta);
        fDistSquared = VectorLength2DSquared(vDelta);
        if (m_fMinDistanceSquared >= fDistSquared || fDistSquared >= m_fMaxDistanceSquared) {
            ev->AddInteger(qfalse);
            return;
        }
    }
    if (m_fInterval == 0) {
        for (auto pSquadMate = m_pNextSquadMate; pSquadMate != this; pSquadMate = pSquadMate->m_pNextSquadMate) {
            float v27, v29, v30, v32, v33, v34, v35, v37, v38;
            if (pSquadMate->IsSubclassOfActor()) {
                Sentient *pEnemy = pSquadMate->m_Enemy;
                if (pEnemy) {
                    v27 = pSquadMate->origin[0];
                    v29 = pEnemy->origin[0] - v27;
                    v30 = pSquadMate->origin[1];
                    v32 = pEnemy->origin[1] - v30;
                } else {
                    continue;
                }
            } else {
                v27 = pSquadMate->origin[0];
                v29 = pSquadMate->orientation[0][0];
                v32 = pSquadMate->orientation[0][1];
                v30 = pSquadMate->origin[1];
            }
            v33 = v32 * origin[0];
            v34 = v27 * v32 - v30 * v29;
            v35 = v33 - v29 * origin[1] - v34;
            v37 = v32 * vDest[0];
            v38 = v37 - v29 * vDest[1] - v34;

            // check if they both have different sign.
            if (((int)v38 ^ (int)v35) < 0) {
                ev->AddInteger(qfalse);
                return;
            }
        }
        ev->AddInteger(qtrue);
        return;
    }

    fIntervalSquared = Square(m_fInterval);

    for (auto pSquadMate = m_pNextSquadMate; pSquadMate != this; pSquadMate = pSquadMate->m_pNextSquadMate) {
        Vector vDel;
        vDel = vDest - pSquadMate->origin;
        if (fIntervalSquared <= vDest.lengthSquared()) {
            continue;
        }

        if ((origin - pSquadMate->origin).lengthSquared() > vDest.lengthSquared()) {
            break;
        }
    }
    ev->AddInteger(qfalse);
}

void Actor::EventMoveDir(Event *ev)
{
    vec2_t vec;
    if (!PathGoalSlowdownStarted()) {
        vec[0] = velocity.x;
        vec[1] = velocity.y;
        if (velocity.x == 0 && velocity.y == 0 && PathExists() && !PathComplete()) {
            vec[0] = PathDelta()[0];
            vec[1] = PathDelta()[1];
        }
        VectorNormalize2D(vec);
    }

    ev->AddVector(vec);
}

void Actor::EventIntervalDir(Event *ev)
{
    //FIXME: macro
    if (level.inttime >= m_iIntervalDirTime + 250) {
        m_vIntervalDir     = vec_zero;
        m_iIntervalDirTime = level.inttime;
        if (m_Enemy) {
            m_vIntervalDir = GetAntiBunchPoint() - origin;
            if (m_vIntervalDir.x != 0 || m_vIntervalDir.y != 0) {
                VectorNormalizeFast(m_vIntervalDir);
            }
        }
    }

    ev->AddVector(m_vIntervalDir);
}

void Actor::EventResetLeash(Event *ev)
{
    m_vHome = origin;
    //delete m_pTetherEnt;
    m_pTetherEnt = NULL;
}

void Actor::EventTether(Event *ev)
{
    m_pTetherEnt = ev->GetSimpleEntity(1);
}

bool Actor::ShortenPathToAttack(float fMinDist)
{
    if (PathExists() && !PathComplete() && PathAvoidsSquadMates()) {
        for (auto current_node = CurrentPathNode(); current_node >= LastPathNode(); current_node--) {
            Vector dist = current_node->point - origin;
            if (dist * dist >= fMinDist * fMinDist) {
                if (CanSeeFrom(eyeposition + current_node->point, m_Enemy)) {
                    m_Path.TrimPathFromEnd((current_node - LastPathNode()) / 32);
                    return true;
                }
            }
        }
    }

    return false;
}

void Actor::StrafeToAttack(float fDist, vec3_t vDir)
{
    Vector vEnemyCentroid, vDelta, vSpot, vDestPos;
    Vector mins(-16, -16, 16);
    Vector maxs(16, 16, 128);

    vDestPos = vDir;
    vDestPos *= fDist;
    vDestPos += origin;

    vDelta = vDestPos - m_vLastEnemyPos;

    if (m_fMaxDistanceSquared <= vDelta.lengthSquared() || vDelta.lengthSquared() <= m_fMinDistanceSquared) {
        ClearPath();
        return;
    }
    if (!G_SightTrace(
            origin,
            mins,
            maxs,
            vDestPos,
            this,
            NULL,
            //FIXME: macro
            1107437825,
            qtrue,
            "Actor::StrafeToAttack 1"
        )) {
        ClearPath();
        return;
    }
    vSpot = vDir;
    vSpot *= fDist;

    vDelta = m_vLastEnemyPos;
    vDelta.z += m_Enemy->centroid.z - m_Enemy->origin.z;

    if (!G_SightTrace(
            EyePosition() + vSpot,
            vec_zero,
            vec_zero,
            vDelta,
            this,
            m_Enemy,
            //FIXME: macro
            33819417,
            qfalse,
            "Actor::StrafeToAttack 1"
        )) {
        SetPathWithLeash(vDestPos, NULL, 0);
    } else {
        ClearPath();
    }
}

void Actor::EventGetThinkState(Event *ev)
{
    ev->AddConstString(m_csThinkStateNames[m_ThinkState]);
}

void Actor::EventGetEnemyShareRange(Event *ev)
{
    ev->AddFloat(m_fMaxShareDistSquared);
}

void Actor::EventSetEnemyShareRange(Event *ev)
{
    float fRange           = ev->GetFloat(1);
    m_fMaxShareDistSquared = fRange * fRange;
}

void Actor::GetVoiceType(Event *ev)
{
    //voice type in actor is a char.
    const char *vType    = va("%c", mVoiceType);
    str         vTypeStr = vType;
    ev->AddString(vTypeStr);
}

void Actor::SetVoiceType(Event *ev)
{
    //voice type in actor is a char.
    str vType = ev->GetString(1);
    if (vType[0]) {
        mVoiceType = vType[0];
    } else {
        //FIXME: enum
        mVoiceType = -1;
    }
}

PathNode *Actor::FindSniperNodeAndSetPath(bool *pbTryAgain)
{
    PathNode *pSniperNode = PathManager.FindNearestSniperNode(this, origin, m_Enemy);

    if (pSniperNode) {
        SetPathWithLeash(pSniperNode, NULL, 0);

        if (PathExists() && (PathComplete() || PathAvoidsSquadMates())) {
            *pbTryAgain = true;
            return pSniperNode;
        }

        pSniperNode->MarkTemporarilyBad();
        *pbTryAgain = false;
    } else {
        *pbTryAgain = true;
    }

    return NULL;
}

void Actor::Remove(Event *ev)
{
    EndStates();

    if (deadflag != DEAD_DEAD) {
        health   = 0;
        deadflag = DEAD_DEAD;
    }

    Delete();
}

void Actor::EventGetKickDir(Event *ev)
{
    ev->AddVector(m_vKickDir);
}

void Actor::EventGetNoLongPain(Event *ev)
{
    ev->AddInteger(m_bNoLongPain);
}

void Actor::EventSetNoLongPain(Event *ev)
{
    m_bNoLongPain = ev->GetBoolean(1);
}

void Actor::DontFaceWall(void)
{
    //PathNode *pNode;
    vec2_t                vDelta;
    float /*fErrorLerp,*/ fEyeRadius, fCosAngle, fSinAngle, fAngle, fAngleRad, fTime;
    Vector                start;
    Vector                end;

    //FIXME: macro: AI_*
    if (m_pCoverNode && m_pCoverNode->nodeflags & (AI_CORNER_LEFT | AI_CORNER_RIGHT | AI_SNIPER | AI_CRATE)) {
        m_eDontFaceWallMode = 1;
        return;
    }

    if (velocity.lengthXYSquared() > Square(8)) {
        m_eDontFaceWallMode = 2;
        return;
    }

    fAngle = AngleNormalize360(m_YawAchieved ? angles.y : m_DesiredYaw);

    fTime = (level.time - m_fDfwTime) * 0.5;
    if (fTime > 1) {
        fTime = 1;
    }

    VectorSub2D(m_vDfwPos, origin, vDelta);

    if (Square(fTime * -14.0 + 16.0) > VectorLength2DSquared(vDelta)
        && (fabs(AngleNormalize180(m_fDfwRequestedYaw - m_DesiredYaw)) <= fTime * -29.0 + 30.0
            || fabs(AngleNormalize180(m_fDfwDerivedYaw - m_DesiredYaw)) <= fTime * -29.0 + 30.0)) {
        if (m_eDontFaceWallMode <= 8) {
            SetDesiredYaw(m_fDfwDerivedYaw);
        }
    } else {
        m_vDfwPos          = origin;
        m_fDfwRequestedYaw = fAngle;
        m_fDfwTime         = level.time;

        fAngleRad = fAngle * M_PI / 180;
        fSinAngle = sin(fAngleRad);
        fCosAngle = cos(fAngleRad);

        VectorSub2D(EyePosition(), origin, vDelta);
        fEyeRadius = VectorLength2D(vDelta);

        start   = vec_zero;
        start.x = fEyeRadius * fCosAngle + origin.x;
        start.y = fEyeRadius * fSinAngle + origin.y;

        end           = vec_zero;
        end.x         = 64 * fCosAngle + start.x;
        end.x         = 64 * fSinAngle + start.y;
        trace_t trace = G_Trace(start, vec_zero, vec_zero, end, this, 33819417, qfalse, "Actor::DontFaceWall");
        if (trace.entityNum == 1023 || trace.fraction >= 1 || trace.startsolid) {
            m_eDontFaceWallMode = 3;
        } else if (trace.entityNum == 1022 || trace.ent->entity->AIDontFace()) {
            if (trace.plane.normal[2] < -0.69999999 || trace.plane.normal[2] > 0.69999999) {
                m_eDontFaceWallMode = 5;
            } else {
                if (m_Enemy && Vector::Dot(trace.plane.normal, m_Enemy->origin) - trace.plane.dist < 0) {
                    end = m_Enemy->origin;
                } else {
                    end = start + (end - start) * 128;
                }

                PathNode *node = PathManager.FindCornerNodeForWall(origin, end, this, 0.0f, trace.plane.normal);
                if (node) {
                    VectorSub2D(node->m_PathPos, origin, vDelta);
                    if (vDelta[0] != 0 || vDelta[1] != 0) {
                        SetDesiredYawDir(vDelta);
                    }
                    m_eDontFaceWallMode = 6;
                } else if (trace.fraction <= 0.46875) {
                    SetDesiredYawDir(trace.plane.normal);
                    m_eDontFaceWallMode = 7;
                } else {
                    if (m_DesiredYaw < 90.0) {
                        m_DesiredYaw += 270.0;
                    } else {
                        m_DesiredYaw -= 90.0;
                    }

                    m_YawAchieved       = false;
                    m_eDontFaceWallMode = 8;
                }
                m_fDfwDerivedYaw = m_DesiredYaw;
                return;
            }
        } else {
            m_eDontFaceWallMode = 4;
        }

        m_fDfwDerivedYaw = m_fDfwRequestedYaw;
    }
}

bool Actor::IsVoidState(int state)
{
    return true;
}

bool Actor::IsIdleState(int state)
{
    return state == THINKSTATE_IDLE;
}

bool Actor::IsCuriousState(int state)
{
    return state == THINKSTATE_CURIOUS;
}

bool Actor::IsDisguiseState(int state)
{
    return state == THINKSTATE_DISGUISE;
}

bool Actor::IsAttackState(int state)
{
    return state == THINKSTATE_ATTACK;
}

bool Actor::IsGrenadeState(int state)
{
    return state == THINKSTATE_GRENADE;
}

bool Actor::IsBadPlaceState(int state)
{
    return state == THINKSTATE_BADPLACE;
}

bool Actor::IsPainState(int state)
{
    return state == THINKSTATE_PAIN;
}

bool Actor::IsKilledState(int state)
{
    return state == THINKSTATE_KILLED;
}

bool Actor::IsNoClipState(int state)
{
    return state == THINKSTATE_NOCLIP;
}

/*
===============
Actor::InitVoid

Init void global func
===============
*/
void Actor::InitVoid(GlobalFuncs_t *func)
{
    func->IsState = &Actor::IsVoidState;
}

void Actor::BecomeCorpse(void)
{
    Event e1(EV_DeathSinkStart);

    AddToBodyQue();

    edict->r.contents = CONTENTS_TRIGGER;
    edict->r.svFlags &= ~SVF_MONSTER;
    setSolidType(SOLID_NOT);
    movetype = MOVETYPE_NONE;
    edict->s.renderfx &= ~RF_SHADOW;

    PostEvent(e1, 20);
}

void Actor::EventGetFavoriteEnemy(Event *ev)
{
    ev->AddEntity(m_FavoriteEnemy);
}

void Actor::EventSetFavoriteEnemy(Event *ev)
{
    Sentient *fEnemy = (Sentient *)ev->GetEntity(1);
    m_FavoriteEnemy  = fEnemy;
}

void Actor::EventFindEnemy(Event *ev)
{
    if (level.inttime > m_iEnemyCheckTime + 200) {
        UpdateEnemyInternal();
    }

    ev->AddEntity(m_Enemy);
}

void Actor::EventGetMumble(Event *ev)
{
    ev->AddInteger(m_bMumble);
}

void Actor::EventSetMumble(Event *ev)
{
    m_bMumble = ev->GetInteger(1);
}

void Actor::EventCalcGrenadeToss2(Event *ev)
{
    // FIXME: unimplemented
}

void Actor::EventCalcGrenadeToss(Event *ev)
{
    //FIXME: macros
    Vector vTarget;
    vTarget = ev->GetVector(1);
    if (DecideToThrowGrenade(vTarget, &m_vGrenadeVel, &m_eGrenadeMode)) {
        if (m_eGrenadeMode == AI_GREN_TOSS_ROLL) {
            ev->AddConstString(STRING_ANIM_GRENADETOSS_SCR);
        } else {
            if (m_eGrenadeMode > AI_GREN_TOSS_ROLL ? m_eGrenadeMode == AI_GREN_TOSS_HINT
                                                   : m_eGrenadeMode == AI_GREN_TOSS_THROW) {
                ev->AddConstString(STRING_ANIM_GRENADETHROW_SCR);
            } else {
                char assertStr[16317] = {0};
                strcpy(assertStr, "\"invalid return condition for Actor::EventCalcGrenadeToss\"\n\tMessage: ");
                Q_strcat(assertStr, sizeof(assertStr), DumpCallTrace(""));
                assert(false && assertStr);
            }
        }

        SetDesiredYawDir(m_vGrenadeVel);
    } else {
        ev->AddConstString(STRING_EMPTY);
    }
}

void Actor::EventGetNoSurprise(Event *ev)
{
    ev->AddInteger(m_bNoSurprise);
}

void Actor::EventSetNoSurprise(Event *ev)
{
    m_bNoSurprise = ev->GetBoolean(1);
}

void Actor::EventGetSilent(Event *ev)
{
    ev->AddInteger(m_bSilent);
}

void Actor::EventSetSilent(Event *ev)
{
    m_bSilent = ev->GetBoolean(1);
}

void Actor::EventGetAvoidPlayer(Event *ev)
{
    ev->AddInteger(m_bAutoAvoidPlayer);
}

void Actor::EventSetAvoidPlayer(Event *ev)
{
    m_bAutoAvoidPlayer = ev->GetBoolean(1);
}

void Actor::PathnodeClaimRevoked(PathNode *node)
{
    for (int i = m_ThinkLevel; i >= 0; --i) {
        GlobalFuncs_t *func = &GlobalFuncs[m_Think[i]];
        if (func->PathnodeClaimRevoked) {
            (this->*func->PathnodeClaimRevoked)();
        }
    }
    node->Relinquish();
    m_pCoverNode = NULL;
}

void Actor::SetPathToNotBlockSentient(Sentient *pOther)
{
    Vector vDest, vPerp, vAway;

    if (pOther) {
        if (!pOther->IsDead()) {
            if (IsTeamMate(pOther)) {
                if (pOther->velocity.lengthSquared() > 1) {
                    vDest = origin - pOther->origin;
                    if (vDest.lengthSquared() < 2304 && DotProduct2D(pOther->velocity, vDest) > 0) {
                        if (CrossProduct2D(vDest, pOther->velocity) >= 0) {
                            vPerp.x = -pOther->velocity.y;
                            vPerp.y = pOther->velocity.x;
                            vPerp.z = 0;
                        } else {
                            vPerp.x = pOther->velocity.y;
                            vPerp.y = -pOther->velocity.x;
                            vPerp.z = 0;
                        }

                        VectorNormalizeFast(vPerp);

                        vAway = vPerp * 48 + origin;

                        if (G_SightTrace(
                                vAway,
                                mins,
                                maxs,
                                vAway,
                                this,
                                pOther,
                                33557249,
                                0,
                                "Actor::SetPathToNotBlockSentient 1"
                            )) {
                            SetPathWithinDistance(vAway, NULL, 96, 0);
                        }

                        if (!PathExists()) {
                            vAway = vDest;
                            VectorNormalizeFast(vAway);

                            vAway = vAway * 48 + origin;
                            if (G_SightTrace(
                                    vAway,
                                    mins,
                                    maxs,
                                    vAway,
                                    this,
                                    pOther,
                                    33557249,
                                    0,
                                    "Actor::SetPathToNotBlockSentient 2"
                                )) {
                                SetPathWithinDistance(vAway, NULL, 96, 0);
                            }

                            if (!PathExists()) {
                                vAway = vPerp * 48 - origin;
                                if (G_SightTrace(
                                        vAway,
                                        mins,
                                        maxs,
                                        vAway,
                                        this,
                                        pOther,
                                        33557249,
                                        0,
                                        "Actor::SetPathToNotBlockSentient 3"
                                    )) {
                                    SetPathWithinDistance(vAway, NULL, 96, 0);
                                }

                                if (!PathExists()) {
                                    vAway = vDest;
                                    VectorNormalizeFast(vAway);

                                    FindPathAway(origin, vAway * 100, 96);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

/*
===============
Actor::LookAround

Look around behaviour.
===============
*/
void Actor::LookAround(float fFovAdd)
{
    Vector vDest, vAngle;
    float  fModTime;

    if (level.inttime >= m_iNextLookTime) {
        fModTime = level.time;
        vAngle   = angles;

        vAngle.y += fFovAdd * (2 * (fModTime * 0.125 - floor(fModTime * 0.125)) - 1.0);
        vAngle.x += (noise1(fModTime * 1.005309626678312 + entnum) + 1.0) * 15.0;

        AngleVectors(vAngle, vDest, NULL, NULL);

        vDest += EyePosition() * 1024;
        trace_t trace = G_Trace(EyePosition(), vec_zero, vec_zero, vDest, this, 25, qfalse, "Actor::LookAround");
        if (trace.fraction > 0.125) {
            m_bHasDesiredLookDest = true;
            VectorCopy(trace.endpos, m_vDesiredLookDest);
            m_iNextLookTime = level.inttime + rand() % 500 + 750;
        } else {
            m_bHasDesiredLookDest = false;
            m_iNextLookTime       = level.inttime + 187;
        }
    }

    if (m_bHasDesiredLookDest) {
        SetDesiredLookDir(m_vDesiredLookDest - EyePosition());
    } else {
        m_bHasDesiredLookAngles = false;
    }
}

void Actor::EventGetLookAroundAngle(Event *ev)
{
    //FIXME: weird
    //ev->GetFloat(m_fLookAroundFov);
    ev->AddFloat(m_fLookAroundFov);
}

void Actor::EventSetLookAroundAngle(Event *ev)
{
    float angle = ev->GetFloat(1);
    if (angle < 0.0 || angle > 60.0) {
        ScriptError("lookaroundangle must be >= 0 and <= 60");
    }
    m_fLookAroundFov = angle;
}

void Actor::DumpAnimInfo(void)
{
    Animate::DumpAnimInfo();
    Vector desiredLook = m_bHasDesiredLookAngles ? m_DesiredLookAngles : vec_zero;
    //FIXME: macros: bones
    Vector head  = GetControllerAngles(HEAD_TAG);
    Vector torso = GetControllerAngles(TORSO_TAG);
    MPrintf(
        "Desired look yaw: %.1f, pitch: %.1f.  Head yaw: %.1f, pitch %.1f.  Torso yaw: %.1f, pitch: %.1f\n",
        desiredLook.x,
        desiredLook.y,
        head.x,
        head.y,
        torso.x,
        torso.y
    );
}

/*
===============
Actor::DumpCallTrace

Dump useful debug info.
===============
*/
const char *Actor::DumpCallTrace(const char *pszFmt, ...) const
{
    cvar_t *sv_mapname;
    tm     *ptm;
    time_t  t;
    Vector  vPlayer = vec_zero;
    char    szFile[64];
    int     i2;
    //int i1;
    int  i;
    char szTemp[65536];
    char szTemp2[1024];

    va_list va;

    va_start(va, pszFmt);

    sv_mapname = gi.Cvar_Get("mapname", "unknown", 0);

    Player *p = (Player *)G_GetEntity(0);
    if (p) {
        vPlayer = p->origin;
    }

    i = sprintf(
        szTemp,
        "map = %s\n"
        "time = %i (%i:%02i)\n"
        "entnum = %i, targetname = '%s'\n"
        "origin = (%g %g %g)\n"
        "think = %s, thinkstate = %s\n"
        "type_idle = %s\n"
        "type_attack = %s\n"
        "m_State = %i, m_pszDebugState = %s\n"
        "player is at (%g %g %g)\n"
        "--------------------------------------\n",
        sv_mapname->string,
        level.inttime,
        level.inttime / 60000,
        level.inttime / 1000 - 60 * (level.inttime / 60000),
        entnum,
        targetname.c_str(),
        origin[0],
        origin[1],
        origin[2],
        ThinkName().c_str(),
        ThinkStateName().c_str(),
        Director.GetString(m_csThinkNames[m_ThinkMap[THINKSTATE_IDLE]]).c_str(),
        Director.GetString(m_csThinkNames[m_ThinkMap[THINKSTATE_ATTACK]]).c_str(),
        m_State,
        m_pszDebugState,
        vPlayer[0],
        vPlayer[1],
        vPlayer[2]
    );

    if (pszFmt) {
        i2 = i + vsprintf(&szTemp[i], pszFmt, va);
    }
    if (i2 != i) {
        strcpy(&szTemp[i2], "\n--------------------------------------\n");
    }

    t   = time(0);
    ptm = localtime(&t);
    sprintf(
        szFile,
        "main\\ai_trace_%s_%i_%i_%02i%02i.log",
        sv_mapname->string,
        ptm->tm_mon + 1,
        ptm->tm_mday,
        ptm->tm_hour,
        ptm->tm_min
    );

    for (i = 0; szFile[i]; i++) {
        if (szFile[i] == '\\' || szFile[i] == '/') {
            szFile[i] = '_';
        }
    }
    sprintf(szTemp2, "Include '%s' in your bug report!", szFile);
    //PreAssertMessage:
    fprintf(stderr, "IMPORTANT: %s\n", szTemp2);

    va_end(va);

    return ::va(
        "\n"
        "\t-------------------------- IMPORTANT REMINDER --------------------------\n"
        "\n"
        "\tinclude '%s' in your bug report! \n"
        "\n"
        "\t----------------------- END IMPORTANT REMINDER -----------------------\n"
        "\n"
        "%s",
        szFile,
        szTemp
    );
}

void Actor::EventSetMoveDoneRadius(Event *ev)
{
    float moveDoneR          = ev->GetFloat(1);
    m_fMoveDoneRadiusSquared = Square(moveDoneR);
}

void Actor::EventGetMoveDoneRadius(Event *ev)
{
    ev->AddFloat(sqrtf(m_fMoveDoneRadiusSquared));
}

/*
===============
Actor::StoppedWaitFor

Called when stopped wait for an actor.
===============
*/
void Actor::StoppedWaitFor(const_str name, bool bDeleting)
{
    g_iInThinks++;

    GlobalFuncs_t *func = &GlobalFuncs[m_Think[m_ThinkLevel]];

    if (func->FinishedAnimation) {
        (this->*func->FinishedAnimation)();
    }

    g_iInThinks--;
}

void Actor::EventHasCompleteLookahead(Event *ev)
{
    int completeLH = PathExists() && PathHasCompleteLookahead();

    ev->AddInteger(completeLH);
}

void Actor::EventPathDist(Event *ev)
{
    float dist = 0;
    if (PathExists() && !PathComplete()) {
        dist = PathDist();
    }
    ev->AddFloat(dist);
}

void Actor::EventCanShootEnemyFrom(Event *ev)
{
    int canShoot = false;
    if (m_Enemy) {
        canShoot = CanSeeFrom(ev->GetVector(1), m_Enemy);
    }
    ev->AddInteger(canShoot);
}

void Actor::EventSetInReload(Event *ev)
{
    m_bInReload = ev->GetBoolean(1);
}

void Actor::EventGetInReload(Event *ev)
{
    ev->AddInteger(m_bInReload);
}

void Actor::EventSetReloadCover(Event *ev)
{
    m_bNeedReload = ev->GetBoolean(1);
}

void Actor::EventBreakSpecial(Event *ev)
{
    mbBreakSpecialAttack = true;
}

void Actor::EventCanShoot(Event *ev)
{
    Entity *target = ev->GetEntity(1);

    if (!target) {
        ScriptError("canshoot applied to null listener");
    }

    ev->AddInteger(CanShoot(target));
}

void Actor::EventSetBalconyHeight(Event *ev)
{
    m_fBalconyHeight = ev->GetFloat(1);
}

void Actor::EventSetIgnoreBadPlace(Event *ev)
{
    if (m_bIgnoreBadPlace != ev->GetBoolean(1)) {
        m_bIgnoreBadPlace = ev->GetBoolean(1);

        if (level.m_badPlaces.NumObjects() && m_bIgnoreBadPlace) {
            UpdateBadPlaces();
        }
    }
}

void Actor::EventGetIgnoreBadPlace(Event *ev)
{
    ev->AddInteger(m_bIgnoreBadPlace);
}

void Actor::EventGetBalconyHeight(Event *ev)
{
    ev->AddFloat(m_fBalconyHeight);
}

void Actor::BecomeTurretGuy(void)
{
    SetThinkIdle(THINK_IDLE);

    SetThink(THINKSTATE_ATTACK, THINK_TURRET);
    SetThink(THINKSTATE_DISGUISE, THINK_DISGUISE_SALUTE);
    SetThink(THINKSTATE_GRENADE, THINK_GRENADE);

    if (CurrentThink() == THINK_IDLE && Turret_DecideToSelectState()) {
        TransitionState(100, 0);
    }
}

/*
===============
Actor::AimAtEnemyBehavior

Aiming at enemy behaviour.
Usally called after SetEnemyPos()
===============
*/
void Actor::AimAtEnemyBehavior(void)
{
    AimAtTargetPos();

    Anim_Aim();
}

/*
===============
Actor::EventEnableEnemySwitch
===============
*/
void Actor::EventEnableEnemySwitch(Event *ev)
{
    m_bEnemySwitch = true;
}

/*
===============
Actor::EventDisableEnemySwitch
===============
*/
void Actor::EventDisableEnemySwitch(Event *ev)
{
    m_bEnemySwitch = false;
}

/*
===============
Actor::EventSetRunAnimRate
===============
*/
void Actor::EventSetRunAnimRate(Event *ev)
{
    m_fRunAnimRate = ev->GetFloat(1);
}

/*
===============
Actor::EventGetRunAnimRate
===============
*/
void Actor::EventGetRunAnimRate(Event *ev)
{
    ev->AddFloat(m_fRunAnimRate);
}

/*
===============
Actor::Landed
===============
*/
void Actor::Landed(Event *ev)
{
    if (groundentity && groundentity->entity != world) {
        warning("Actor::Landed", "Actor %d has landed on an entity that might move\n", entnum);
    }

    setMoveType(MOVETYPE_NONE);
}

bool Actor::IsOnFloor(void)
{
    str name = AnimName(0);
    name.toupper();
    return strstr(name, "FLOOR") != NULL;
}

/*
===============
Actor::GetNationality
===============
*/
void Actor::GetNationality(Event *ev)
{
    switch (m_iNationality) {
    case ACTOR_NATIONALITY_DEFAULT:
    default:
        switch (m_Team) {
        case TEAM_AMERICAN:
            ev->AddString("usa");
            break;
        case TEAM_GERMAN:
            ev->AddString("ger");
            break;
        default:
            ev->AddString("unset");
            break;
        }
        break;
    case ACTOR_NATIONALITY_AMERICAN:
        ev->AddString("usa");
        break;
    case ACTOR_NATIONALITY_GERMAN:
        ev->AddString("ger");
        break;
    case ACTOR_NATIONALITY_ITALIAN:
        ev->AddString("it");
        break;
    case ACTOR_NATIONALITY_BRITISH:
        ev->AddString("uk");
        break;
    case ACTOR_NATIONALITY_RUSSIAN:
        ev->AddString("ussr");
        break;
    }
}

/*
===============
Actor::SetNationality
===============
*/
void Actor::SetNationality(Event *ev)
{
    str name;

    if (ev->NumArgs() != 1) {
        ScriptError("Bad bad nationality specified for '%s' at (%f %f %f)\n", TargetName().c_str(), origin[0], origin[1], origin[2]);
    }

    name = ev->GetString(1);

    if (!str::icmpn(name, "default", 8)) {
        m_iNationality = ACTOR_NATIONALITY_DEFAULT;
    } else if (!str::icmpn(name, "ger", 4)) {
        m_iNationality = ACTOR_NATIONALITY_GERMAN;
    } else if (!str::icmpn(name, "it", 3)) {
        m_iNationality = ACTOR_NATIONALITY_ITALIAN;
    } else if (!str::icmpn(name, "usa", 4)) {
        m_iNationality = ACTOR_NATIONALITY_AMERICAN;
    } else if (!str::icmpn(name, "uk", 3)) {
        m_iNationality = ACTOR_NATIONALITY_BRITISH;
    } else if (!str::icmpn(name, "ussr", 5)) {
        m_iNationality = ACTOR_NATIONALITY_RUSSIAN;
    } else {
        ScriptError("Bad bad nationality specified for '%s', must be one of: ger, it, usa, uk, ussr or default\n", TargetName().c_str());
    }
}

/*
===============
Actor::EventWriteStats
===============
*/
void Actor::EventWriteStats(Event *ev)
{
    if (g_aistats->integer) {
        level.OpenActorStats();
    }
}

/*
===============
Actor::EventCuriousOff
===============
*/
void Actor::EventCuriousOff(Event *ev)
{
    m_bIsCurious = false;
}

/*
===============
Actor::EventCuriousOn
===============
*/
void Actor::EventCuriousOn(Event *ev)
{
    m_bIsCurious = true;
}

void Actor::EventSetDestIdle(Event *ev)
{
    // not found in ida
}

void Actor::EventSetDestIdle2(Event *ev)
{
    // not found in ida
}

void Actor::EventGetBreathSteam(Event *ev)
{
    ev->AddInteger(m_bBreathSteam);
}

void Actor::EventSetBreathSteam(Event *ev)
{
    m_bBreathSteam = ev->GetInteger(1);
}

void Actor::EventSetNextBreathTime(Event *ev)
{
    ScriptVariable sVar;
    ScriptThread  *t = Director.CreateThread("global/breathe.scr", "nextbreathtime");

    sVar.setFloatValue(ev->GetFloat(1));
    t->Execute(&sVar, 1);
}

bool Actor::EnemyIsDisguised(void)
{
    return (m_bEnemyIsDisguised || (m_Enemy && m_Enemy->m_bIsDisguised))
        && (!m_bForceAttackPlayer && m_ThinkState != THINKSTATE_ATTACK);
}

bool Actor::IsDisabled() const
{
    return !m_bDoAI;
}

/*
===============
Actor::ExtractConstraints

===============
*/
void Actor::ExtractConstraints(mmove_t *mm)
{
    // not found in ida
}
