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
#include "scriptmaster.h"
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
#include <parm.h>
#include "bg_local.h"
#include "weapturret.h"
#include "sentient.h"

Vector MINS(-15.0,-15.0,0.0);
Vector MAXS(15.0,15.0,96.0);

extern Vector PLAYER_BASE_MIN;
extern Vector PLAYER_BASE_MAX;

bool bEntinfoInit = false;

cvar_t *g_showinfo;

const char * gAmericanVoices[] = {"a", "c", "h"};
const char * gGermanVoices[] = {"a", "c", "d"};

Event EV_Actor_SetGun
	(
	"gun",
	EV_DEFAULT,
	"s",
	"s",
	"specifies the gun to use"
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
	2
	);

Event EV_Actor_WeaponInternal
	(
	"weapon_internal",
	EV_DEFAULT,
	"s",
	"s",
	"internal use"
	);

Event EV_Actor_TestAnim
	(
	"testanim",
	EV_DEFAULT,
	NULL,
	NULL,
	"",
	0
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
	"Specify the location to move to, with animation anim."
	);

Event EV_Actor_WalkTo
	(
	"walkto",
	EV_DEFAULT,
	"s",
	"dest",
	"Specify the location to walk to."
	);

Event EV_Actor_RunTo
	(
	"runto",
	EV_DEFAULT,
	"s",
	"dest",
	"Specify the location to run to."
	);

Event EV_Actor_CrouchTo
	(
	"crouchto",
	EV_DEFAULT,
	"s",
	"dest",
	"Specify the location to crouch to."
	);

Event EV_Actor_CrawlTo
	(
	"crawlto",
	EV_DEFAULT,
	"s",
	"dest",
	"Specify the location to crawl to."
	);

Event EV_Actor_AimAt
	(
	"aimat",
	EV_DEFAULT,
	"s",
	"target",
	"Specify the target to aim at."
	);

Event EV_Actor_Follow
	(
	"follow",
	EV_DEFAULT,
	"e",
	"entity",
	"Specify the entity to follow."
	);

Event EV_Actor_DeathEmbalm
	(
	"deathembalm",
	EV_DEFAULT,
	NULL,
	NULL,
	"preps the dead actor for turning nonsolid gradually over time"
	);

Event EV_Actor_Anim
	(
	"anim",
	EV_DEFAULT,
	"s",
	"name",
	"Play animation."
	);

Event EV_Actor_Anim_Scripted
	(
	"anim_scripted",
	EV_DEFAULT,
	"s",
	"name",
	"Play scripted animation."
	);

Event EV_Actor_Anim_Noclip
	(
	"anim_noclip",
	EV_DEFAULT,
	"s",
	"name",
	"Play noclip animation."
	);

Event EV_Actor_AnimLoop
	(
	"animloop",
	EV_DEFAULT,
	"s",
	"name",
	"Loop animation."
	);

Event EV_Actor_AnimScript
	(
	"animscript",
	EV_DEFAULT,
	"s",
	"name",
	"Play the animation script"
	);

Event EV_Actor_AnimScript_Scripted
	(
	"animscript_scripted",
	EV_DEFAULT,
	"s",
	"name",
	"Play the scripted animation script"
	);

Event EV_Actor_AnimScript_Noclip
	(
	"animscript_noclip",
	EV_DEFAULT,
	"s",
	"name",
	"Play the noclip animation script"
	);

Event EV_Actor_Reload_mg42
	(
	"reload_mg42",
	EV_DEFAULT,
	NULL,
	NULL,
	"Reload the mg42 - only used by machinegunner"
	);

Event EV_Actor_Dumb
	(
	"dumb",
	EV_DEFAULT,
	NULL,
	NULL,
	"Make Actor dumb."
	);

Event EV_Actor_Physics_On
	(
	"physics_on",
	EV_DEFAULT,
	NULL,
	NULL,
	"turn physics on."
	);

Event EV_Actor_Physics_Off
	(
	"physics_off",
	EV_DEFAULT,
	NULL,
	NULL,
	"turn physics off."
	);

Event EV_Actor_SetAnim
	(
	"setanim",
	EV_DEFAULT,
	"sifs",
	"anim slot weight flagged",
	"Set animation slot"
	);

Event EV_Actor_SetAnimLength
	(
	"setanimlength",
	EV_DEFAULT,
	"f",
	"time",
	"Set the maximum time an animation will play"
	);

Event EV_Actor_EndActionAnim
	(
	"endactionanim",
	EV_DEFAULT,
	"",
	"",
	"End any aiming/action animation which is currently playing"
	);

Event EV_Actor_SetMotionAnim
	(
	"setmotionanim",
	EV_DEFAULT,
	"s",
	"anim",
	"Set motion animation (handler scripts only)"
	);

Event EV_Actor_SetAimMotionAnim
	(
	"setaimmotionanim",
	EV_DEFAULT,
	"ss",
	"anim_crouch anim_stand",
	"Set aim motion animation (handler scripts only)"
	);



Event EV_Actor_SetActionAnim
	(
	"setactionanim",
	EV_DEFAULT,
	"sff",
	"base_anim lower_limit upper_limit",
	"Set the base action animation and range that they cover"
	);

Event EV_Actor_UpperAnim
	(
	"upperanim",
	EV_DEFAULT,
	"s",
	"anim",
	"Set the upper body "
	);

Event EV_Actor_SetUpperAnim
	(
	"setupperanim",
	EV_DEFAULT,
	"s",
	"anim",
	"Set the upper body animation - used by animation script only"
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
	"The Weapon Type of the ",
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
	"Initialize a Actor."
	);

Event EV_Actor_LookAt
	(
	"lookat",
	EV_DEFAULT,
	"e",
	"entity",
	"The actor will look at this entity."
	);

Event EV_Actor_EyesLookAt
	(
	"eyeslookat",
	EV_DEFAULT,
	"e",
	"entity",
	"The actor will look at this entity."
	);

Event EV_Actor_PointAt
	(
	"pointat",
	EV_DEFAULT,
	"e",
	"entity",
	"The actor will point at this entity."
	);

Event EV_Actor_TurnTo
	(
	"turnto",
	EV_DEFAULT,
	"e",
	"entity",
	"The actor will turn to this entity."
	);

Event EV_Actor_SetTurnDoneError
	(
	"turndoneerror",
	EV_DEFAULT,
	"f",
	"error",
	"The error amount that turndone will occur for the turnto command."
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
	"The name of an idle dialog animation to play"
	);

Event EV_Actor_SayAnim
	(
	"say",
	EV_DEFAULT,
	"s",
	"animation",
	"The name of a dialog animation to play"
	);

Event EV_Actor_SetSayAnim
	(
	"setsay",
	EV_DEFAULT,
	"s",
	"animation",
	"The name of a dialog animation to play - used by animation script only"
	);

Event EV_Actor_DamagePuff
	(
	"damagepuff",
	EV_DEFAULT,
	"vv",
	"position direction",
	"Spawns a puff of 'blood' smoke at the speficied location in the specified direction."
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
	"The turn speed of the actor."
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
	"e",
	"entity",
	"Sets the primary weapon's aim target."
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
	"Turns the AI off for this actor."
	);

Event EV_Actor_AIOn
	(
	"ai_on",
	EV_DEFAULT,
	NULL,
	NULL,
	"Turns the AI on for this actor."
	);

Event EV_Actor_GetLocalYawFromVector
	(
	"GetLocalYawFromVector",
	EV_DEFAULT,
	NULL,
	NULL,
	"Turn a worldspace vector into a local space yaw",
	EV_RETURN
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
	"Sets the vision distance of the actor."
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
	"The hearing radius of the actor"
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
	"The fov angle of the actor"
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
	"Sets the idle type of the actor."
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
	"Sets the attack type of the actor."
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
	"Sets the disguise type of the actor."
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
	"Sets the disguise level of the actor. May be 1 or 2"
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
	"Sets the grenade type of the actor."
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
	"Sets the name of the patrol path for the actor (must have type set to patrol for effect)"
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
	"If true, patrol guys and running men wait until triggered to move"
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
	"Sets the name of the alarm node for the actor (must have type set to alarm for effect)"
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

Event EV_Actor_SetAlarmThread
	(
	"alarmthread",
	EV_DEFAULT,
	"s",
	"value",
	"Sets the name of the alarm thread for the actor (must have type set to alarm for effect)"
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
	"Sets the name of the thread for the actor to start when accepting papers"
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
	"Set percent to hit"
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
	"Sets the minimum distance the AI tries to keep between itself and the player"
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
	"Sets the maximum distance the AI tries to allow between itself and the player"
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
	"Sets the maximum distance the AI will wander from its leash home"
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
	"Sets the distance AI tries to keep between squadmates while moving."
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
	"Sets the maximum distance for disguise behavior to get triggered"
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
	"Sets the time between the end of one disguise behavior and start of the next"
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
	"Force Actor to attack the player"
	);

Event EV_Actor_SetSoundAwareness
	(
	"sound_awareness",
	EV_DEFAULT,
	"f",
	"awareness_percent",
	"sets the awareness of sounds in 0EV_DEFAULT00 percent chance of hearing a sound withinhalf of the sound's radius' fades to zero outside sound's radius"
	);

Event EV_Actor_SetSoundAwareness2
	(
	"sound_awareness",
	EV_DEFAULT,
	"f",
	"awareness_percent",
	"sets the awareness of sounds in 0EV_DEFAULT00 percent chance of hearing a sound withinhalf of the sound's radius' fades to zero outside sound's radius",
	EV_SETTER
	);

Event EV_Actor_GetSoundAwareness
	(
	"sound_awareness",
	EV_DEFAULT,
	NULL,
	NULL,
	"gets the awareness of sounds in 0EV_DEFAULT00 percent chance of hearing a sound withinhalf of the sound's radius' fades to zero outside sound's radius",
	EV_GETTER
	);

Event EV_Actor_SetGrenadeAwareness
	(
	"gren_awareness",
	EV_DEFAULT,
	"f",
	"awareness_percent",
	"sets the awareness of grenades in 0EV_DEFAULT00 percent chance of responding to a grenadewhen the AI sees it (applied once every 0.4 seconds)"
	);


Event EV_Actor_SetGrenadeAwareness2
	(
	"gren_awareness",
	EV_DEFAULT,
	"f",
	"awareness_percent",
	"sets the awareness of grenades in 0EV_DEFAULT00 percent chance of responding to a grenadewhen the AI sees it (applied once every 0.4 seconds)",
	EV_SETTER
	);

Event EV_Actor_GetGrenadeAwareness
	(
	"gren_awareness",
	EV_DEFAULT,
	NULL,
	NULL,
	"gets the awareness of grenades in 0EV_DEFAULT00 percent chance of responding to a grenadewhen the AI sees it (applied once every 0.4 seconds)",
	EV_GETTER
	);

Event EV_Actor_SetTurret
	(
	"turret",
	EV_DEFAULT,
	"s",
	"turret",
	"Sets the turret of the actor."
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
	"Used only by grenade return animations to tell the code when to attach the grenade to the actor"
	);

Event EV_Actor_DetachGrenade
	(
	"detachgrenade",
	EV_DEFAULT,
	NULL,
	NULL,
	"Used only by grenade return animations to tell the code when to throw the grenade"
	);

Event EV_Actor_FireGrenade
	(
	"fire_grenade",
	EV_DEFAULT,
	NULL,
	NULL,
	"Used only by grenade throw animations to tell the code when to throw a grenade"
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
	"Gives the AI some grenades"
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
	"sets the head model"
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
	"sets the head skin"
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
	"internal code use only - shares an AI's enemy with his squad mates."
	);

Event EV_Actor_ShareGrenade
	(
	"share_grenade",
	EV_DEFAULT,
	NULL,
	NULL,
	"internal code use only - shares an AI's grenade with his squad mates."
	);

Event EV_Actor_InterruptPoint
	(
	"interrupt_point",
	EV_DEFAULT,
	NULL,
	NULL,
	"hint from animation scripts to AI code that now is a good time to switch animations"
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
	"Set the max multiplier in time to notice an enemy (default 100, half as big notices twice as fast)"
	);

Event EV_Actor_GetFixedLeash
	(
	"fixedleash",
	EV_DEFAULT,
	NULL,
	NULL,
	"if non-zero, the leash will never auto-reset; if zero, the leash may auto-reset",
	EV_GETTER
	);

Event EV_Actor_SetFixedLeash
	(
	"fixedleash",
	EV_DEFAULT,
	"f",
	"multiplier",
	"if non-zero, the leash will never auto-reset; if zero, the leash may auto-reset",
	EV_SETTER
	);

Event EV_Actor_SetFixedLeash2
	(
	"fixedleash",
	EV_DEFAULT,
	"f",
	"multiplier",
	"if non-zero, the leash will never auto-reset; if zero, the leash may auto-reset"
	);

Event EV_Actor_Holster
	(
	"holster",
	EV_DEFAULT,
	NULL,
	NULL,
	"Holster weapon"
	);

Event EV_Actor_Unholster
	(
	"unholster",
	EV_DEFAULT,
	NULL,
	NULL,
	"Unholster weapon"
	);

Event EV_Actor_IsEnemyVisible
	(
	"is_enemy_visible",
	EV_DEFAULT,
	NULL,
	NULL,
	"0 if the enemy is not currently visible, 1 if he is"
	);

Event EV_Actor_GetEnemyVisibleChangeTime
	(
	"enemy_visible_change_time",
	EV_DEFAULT,
	NULL,
	NULL,
	"Get the last time whether or not the enemy is visible changed, in seconds"
	);

Event EV_Actor_GetLastEnemyVisibleTime
	(
	"last_enemy_visible_time",
	EV_DEFAULT,
	NULL,
	NULL,
	"Get the last time the enemy was visible, in seconds"
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
	"Set the fallheight"
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

Event EV_Actor_CanMoveTo
	(
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
	"Returns a unit vector pointing in the current direction of motion, or zero if not moving.This still has meaning if velocity is zero but the AI is starting to move on a path.",
	EV_GETTER
	);

Event EV_Actor_ResetLeash
	(
	"resetleash",
	EV_DEFAULT,
	NULL,
	NULL,
	"resets the AI's leash to their current position"
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
	"the entity to which the AI's leash should be tethered"
	);

Event EV_Actor_GetThinkState
	(
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
	"sets the range outside which the AI will not receive notification that a teammate has a new enemy"
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
	"Set voicetype to magic letter postfix"
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
	"Set to 1 if long pain is not allowed, or 0 if long pain is allowed."
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
	"Gets this AI's favorite enemy"
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
	"Set to 1 if this guy is allowed to mumble, or 0 if he is not"
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
	"Set to 1 if this guy is allowed to have steamy breath, or 0 if he is not"
	);

Event EV_Actor_SetNextBreathTime
	(
	"nextbreathtime",
	EV_DEFAULT,
	"f",
	"time_in_sec",
	"Sets the next time the guy will breath out"
	);

Event EV_Actor_CalcGrenadeToss
	(
	"calcgrenadetoss",
	EV_DEFAULT,
	"v",
	"target_position",
	"Called to calculate a grenade toss.  Must be called before a grenade throwing animation.\nReturns the name of the script to call with animscript if the toss can succeed, or  if the toss won't work.\nShould be called infrequently, and never during the middle of a grenade toss.",
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
	"set to 0 to allow this guy to play a surprised animation when first encountering an enemy.\n"
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
	"set to 0 to prevent this guy from saying stuff besides pain and death sounds.\n"
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
	"set to 0 if this AI shouldn't automatically get out of the way, non-zero if he should.\n"
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

Event EV_Actor_BeDead
	(
	"bedead",
	EV_DEFAULT,
	NULL,
	NULL,
	"Forces the actor to be instantly and totally dead; no death animation is played"
	);

Event EV_Actor_GetLookAroundAngle
	(
	"lookaround",
	EV_DEFAULT,
	NULL,
	NULL,
	"gets the angle in degrees left or right of center that the AI will look around while patrolling",
	EV_GETTER
	);

Event EV_Actor_SetLookAroundAngle
	(
	"lookaround",
	EV_DEFAULT,
	"f",
	"angle",
	"gets the angle in degrees left or right of center that the AI will look around while patrolling",
	EV_SETTER
	);

Event EV_Actor_SetLookAroundAngle2
	(
	"lookaround",
	EV_DEFAULT,
	"f",
	"angle",
	"gets the angle in degrees left or right of center that the AI will look around while patrolling"
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

Event EV_Actor_SetReloadCover
	(
	"setreloadcover",
	EV_DEFAULT,
	NULL,
	NULL,
	"do this command to let the ai know it needs to reload; used to reload while going to cover"
	);

Event EV_Actor_BreakSpecial
	(
	"breakspecial",
	EV_DEFAULT,
	NULL,
	NULL,
	"tell ai to break special attack"
	);

Event EV_Actor_SetBalconyHeight
	(
	"balconyheight",
	EV_DEFAULT,
	"f",
	"height",
	"minimum height a balcony guy must fall to do special balcony death"
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

CLASS_DECLARATION( SimpleActor, Actor, "Actor" )
{
	{ &EV_Entity_Start,							&Actor::EventStart },
	{ &EV_Actor_MoveTo,							&Actor::MoveTo },
	{ &EV_Actor_WalkTo,							&Actor::WalkTo },
	{ &EV_Actor_RunTo,							&Actor::RunTo },
	{ &EV_Actor_CrouchTo,						&Actor::CrouchTo },
	{ &EV_Actor_CrawlTo,						&Actor::CrawlTo },
	{ &EV_Actor_AimAt,							&Actor::AimAt },
	{ &EV_Pain,									&Actor::EventPain },
	{ &EV_Killed,								&Actor::EventKilled },
	{ &EV_Actor_DeathEmbalm,					&Actor::DeathEmbalm },
	{ &EV_Actor_Anim,							&Actor::PlayAnimation },
	{ &EV_Actor_AnimLoop,						&Actor::PlayAnimation },
	{ &EV_Actor_Anim_Scripted,					&Actor::PlayScriptedAnimation },
	{ &EV_Actor_Anim_Noclip,					&Actor::PlayNoclipAnimation },
	{ &EV_Actor_AnimScript,						&Actor::EventAnimScript },
	{ &EV_Actor_AnimScript_Scripted,			&Actor::EventAnimScript_Scripted },
	{ &EV_Actor_AnimScript_Noclip,				&Actor::EventAnimScript_Noclip },
	{ &EV_Actor_Reload_mg42,					&Actor::EventReload_mg42 },
	{ &EV_Actor_Dumb,							&Actor::Dumb },
	{ &EV_Actor_Physics_On,						&Actor::PhysicsOn },
	{ &EV_Actor_Physics_Off,					&Actor::PhysicsOff },
	{ &EV_Actor_SetAnim,						&Actor::EventSetAnim },
	{ &EV_Actor_SetAnimLength,					&SimpleActor::EventSetAnimLength },
	{ &EV_Actor_EndActionAnim,					&Actor::EventEndActionAnim },
	{ &EV_Actor_SetMotionAnim,					&Actor::EventSetMotionAnim },
	{ &EV_Actor_SetAimMotionAnim,				&Actor::EventSetAimMotionAnim },
	{ &EV_Actor_SetActionAnim,					&Actor::EventSetActionAnim },
	{ &EV_Actor_UpperAnim,						&Actor::EventUpperAnim },
	{ &EV_Actor_SetUpperAnim,					&Actor::EventSetUpperAnim },
	{ &EV_Actor_SetCrossblendTime,				&SimpleActor::EventSetCrossblendTime },
	{ &EV_Actor_GetCrossblendTime,				&SimpleActor::EventGetCrossblendTime },
	{ &EV_Actor_GetPosition,					&SimpleActor::EventGetPosition },
	{ &EV_Actor_SetPosition,					&SimpleActor::EventSetPosition },
	{ &EV_Actor_SetEmotion,						&SimpleActor::EventSetEmotion },
	{ &EV_Actor_SetAnimFinal,					&SimpleActor::EventSetAnimFinal },
	{ &EV_Actor_GetWeaponType,					&SimpleActor::EventGetWeaponType },
	{ &EV_Actor_GetWeaponGroup,					&SimpleActor::EventGetWeaponGroup },
	{ &EV_Actor_LookAt,							&Actor::EventLookAt },
	{ &EV_Actor_EyesLookAt,						&Actor::EventEyesLookAt },
	{ &EV_Actor_PointAt,						&Actor::EventPointAt },
	{ &EV_Actor_TurnTo,							&Actor::EventTurnTo },
	{ &EV_Actor_SetTurnDoneError,				&Actor::EventSetTurnDoneError },
	{ &EV_Actor_SetTurnDoneError2,				&Actor::EventSetTurnDoneError },
	{ &EV_Actor_GetTurnDoneError,				&Actor::EventGetTurnDoneError },
	{ &EV_Actor_IdleSayAnim,					&Actor::EventIdleSayAnim },
	{ &EV_Actor_SayAnim,						&Actor::EventSayAnim },
	{ &EV_Actor_SetSayAnim,						&Actor::EventSetSayAnim },
	{ &EV_Sentient_UseItem,						&Actor::EventGiveWeapon },
	{ &EV_Sentient_GiveWeapon,					&Actor::EventGiveWeapon },
	{ &EV_Actor_SetWeapon,						&Actor::EventGiveWeapon },
	{ &EV_Actor_GetWeapon,						&Actor::EventGetWeapon },
	{ &EV_Actor_SetGun,							&Actor::EventGiveWeapon },
	{ &EV_Actor_SetGun2,						&Actor::EventGiveWeapon },
	{ &EV_Actor_GetGun,							&Actor::EventGetWeapon },
	{ &EV_Actor_WeaponInternal,					&Actor::EventGiveWeaponInternal },
	{ &EV_Actor_DamagePuff,						&Actor::EventDamagePuff },
	{ &EV_Actor_SetAngleYawSpeed,				&Actor::EventSetAngleYawSpeed },
	{ &EV_Actor_SetAngleYawSpeed2,				&Actor::EventSetAngleYawSpeed },
	{ &EV_Actor_GetAngleYawSpeed,				&Actor::EventGetAngleYawSpeed },
	{ &EV_Actor_SetAimTarget,					&Actor::EventSetAimTarget },
	{ &EV_Actor_ReadyToFire,					&Actor::ReadyToFire },
	{ &EV_Actor_AIOn,							&SimpleActor::EventAIOn },
	{ &EV_Actor_AIOff,							&SimpleActor::EventAIOff },
	{ &EV_Actor_GetLocalYawFromVector,			&Actor::GetLocalYawFromVector },
	{ &EV_DeathSinkStart,						&Actor::DeathSinkStart },
	{ &EV_Actor_GetSight,						&Actor::EventGetSight },
	{ &EV_Actor_SetSight,						&Actor::EventSetSight },
	{ &EV_Actor_SetSight2,						&Actor::EventSetSight },
	{ &EV_Actor_GetHearing,						&Actor::EventGetHearing },
	{ &EV_Actor_SetHearing,						&Actor::EventSetHearing },
	{ &EV_Actor_SetHearing2,					&Actor::EventSetHearing },
	{ &EV_Actor_SetFov,							&Actor::EventSetFov },
	{ &EV_Actor_SetFov2,						&Actor::EventSetFov },
	{ &EV_Actor_GetFov,							&Actor::EventGetFov },
	{ &EV_Actor_DistToEnemy,					&Actor::EventDistToEnemy },
	{ &EV_Actor_SetPatrolPath,					&Actor::EventSetPatrolPath },
	{ &EV_Actor_SetPatrolPath2,					&Actor::EventSetPatrolPath },
	{ &EV_Actor_GetPatrolPath,					&Actor::EventGetPatrolPath },
	{ &EV_Actor_SetPatrolWaitTrigger,			&Actor::EventSetPatrolWaitTrigger },
	{ &EV_Actor_SetPatrolWaitTrigger2,			&Actor::EventSetPatrolWaitTrigger },
	{ &EV_Actor_GetPatrolWaitTrigger,			&Actor::EventGetPatrolWaitTrigger },
	{ &EV_Actor_SetAccuracy,					&Actor::EventSetAccuracy },
	{ &EV_Actor_SetAccuracy2,					&Actor::EventSetAccuracy },
	{ &EV_Actor_GetAccuracy,					&Actor::EventGetAccuracy },
	{ &EV_Actor_SetTypeIdle,					&Actor::EventSetTypeIdle },
	{ &EV_Actor_SetTypeIdle2,					&Actor::EventSetTypeIdle },
	{ &EV_Actor_GetTypeIdle,					&Actor::EventGetTypeIdle },
	{ &EV_Actor_SetTypeAttack,					&Actor::EventSetTypeAttack },
	{ &EV_Actor_SetTypeAttack2,					&Actor::EventSetTypeAttack },
	{ &EV_Actor_GetTypeAttack,					&Actor::EventGetTypeAttack },
	{ &EV_Actor_SetTypeDisguise,				&Actor::EventSetTypeDisguise },
	{ &EV_Actor_SetTypeDisguise2,				&Actor::EventSetTypeDisguise },
	{ &EV_Actor_GetTypeDisguise,				&Actor::EventGetTypeDisguise },
	{ &EV_Actor_SetDisguiseLevel,				&Actor::EventSetDisguiseLevel },
	{ &EV_Actor_SetDisguiseLevel2,				&Actor::EventSetDisguiseLevel },
	{ &EV_Actor_GetDisguiseLevel,				&Actor::EventGetDisguiseLevel },
	{ &EV_Actor_SetTypeGrenade,					&Actor::EventSetTypeGrenade },
	{ &EV_Actor_SetTypeGrenade2,				&Actor::EventSetTypeGrenade },
	{ &EV_Actor_GetTypeGrenade,					&Actor::EventGetTypeGrenade },
	{ &EV_Actor_SetMinDistance,					&Actor::EventSetMinDistance },
	{ &EV_Actor_SetMinDistance2,				&Actor::EventSetMinDistance },
	{ &EV_Actor_GetMinDistance,					&Actor::EventGetMinDistance },
	{ &EV_Actor_SetMaxDistance,					&Actor::EventSetMaxDistance },
	{ &EV_Actor_SetMaxDistance2,				&Actor::EventSetMaxDistance },
	{ &EV_Actor_GetMaxDistance,					&Actor::EventGetMaxDistance },
	{ &EV_Actor_GetLeash,						&Actor::EventGetLeash },
	{ &EV_Actor_SetLeash,						&Actor::EventSetLeash },
	{ &EV_Actor_SetLeash2,						&Actor::EventSetLeash },
	{ &EV_Actor_GetInterval,					&Actor::EventGetInterval },
	{ &EV_Actor_SetInterval,					&Actor::EventSetInterval },
	{ &EV_Actor_SetInterval2,					&Actor::EventSetInterval },
	{ &EV_Actor_GetRunAnim,						&Actor::EventGetRunAnim },
	{ &EV_Actor_GetWalkAnim,					&Actor::EventGetWalkAnim },
	{ &EV_Actor_GetAnimName,					&Actor::EventGetAnimName },
	{ &EV_Actor_SetAnimName,					&Actor::EventSetAnimName },
	{ &EV_Actor_SetDisguiseRange,				&Actor::EventSetDisguiseRange },
	{ &EV_Actor_SetDisguiseRange2,				&Actor::EventSetDisguiseRange },
	{ &EV_Actor_GetDisguiseRange,				&Actor::EventGetDisguiseRange },
	{ &EV_Actor_SetDisguisePeriod,				&Actor::EventSetDisguisePeriod },
	{ &EV_Actor_SetDisguisePeriod2,				&Actor::EventSetDisguisePeriod },
	{ &EV_Actor_GetDisguisePeriod,				&Actor::EventGetDisguisePeriod },
	{ &EV_Actor_SetDisguiseAcceptThread,		&Actor::EventSetDisguiseAcceptThread },
	{ &EV_Actor_SetDisguiseAcceptThread2,		&Actor::EventSetDisguiseAcceptThread },
	{ &EV_Actor_GetDisguiseAcceptThread,		&Actor::EventGetDisguiseAcceptThread },
	{ &EV_Actor_AttackPlayer,					&Actor::EventAttackPlayer },
	{ &EV_Actor_SetAlarmNode,					&Actor::EventSetAlarmNode },
	{ &EV_Actor_SetAlarmNode2,					&Actor::EventSetAlarmNode },
	{ &EV_Actor_GetAlarmNode,					&Actor::EventGetAlarmNode },
	{ &EV_Actor_SetAlarmThread,					&Actor::EventSetAlarmThread },
	{ &EV_Actor_SetAlarmThread2,				&Actor::EventSetAlarmThread },
	{ &EV_Actor_GetAlarmThread,					&Actor::EventGetAlarmThread },
	{ &EV_Actor_SetSoundAwareness,				&Actor::EventSetSoundAwareness },
	{ &EV_Actor_SetSoundAwareness2,				&Actor::EventSetSoundAwareness },
	{ &EV_Actor_GetSoundAwareness,				&Actor::EventGetSoundAwareness },
	{ &EV_Actor_SetGrenadeAwareness,			&Actor::EventSetGrenadeAwareness },
	{ &EV_Actor_SetGrenadeAwareness2,			&Actor::EventSetGrenadeAwareness },
	{ &EV_Actor_GetGrenadeAwareness,			&Actor::EventGetGrenadeAwareness },
	{ &EV_Actor_SetTurret,						&Actor::EventSetTurret },
	{ &EV_Actor_SetTurret2,						&Actor::EventSetTurret },
	{ &EV_Actor_GetTurret,						&Actor::EventGetTurret },
	{ &EV_Actor_AttachGrenade,					&Actor::Grenade_EventAttach },
	{ &EV_Actor_DetachGrenade,					&Actor::Grenade_EventDetach },
	{ &EV_Actor_FireGrenade,					&Actor::Grenade_EventFire },
	{ &EV_Actor_EnableEnemy,					&Actor::EventEnableEnemy },
	{ &EV_Actor_EnablePain,						&Actor::EventEnablePain },
	{ &EV_Activate,								&Actor::EventActivate },
	{ &EV_Actor_GetAmmoGrenade,					&Actor::EventGetAmmoGrenade },
	{ &EV_Actor_SetAmmoGrenade,					&Actor::EventSetAmmoGrenade },
	{ &EV_Actor_SetAmmoGrenade2,				&Actor::EventSetAmmoGrenade },
	{ &EV_Actor_GetMood,						&Actor::EventGetMood },
	{ &EV_Actor_SetMood,						&Actor::EventSetMood },
	{ &EV_Actor_ShareEnemy,						&Actor::EventShareEnemy },
	{ &EV_Actor_ShareGrenade,					&Actor::EventShareGrenade },
	{ &EV_Actor_InterruptPoint,					&Actor::EventInterruptPoint },
	{ &EV_Actor_GetPainHandler,					&SimpleActor::EventGetPainHandler },
	{ &EV_Actor_SetPainHandler,					&SimpleActor::EventSetPainHandler },
	{ &EV_Actor_GetDeathHandler,				&SimpleActor::EventGetDeathHandler },
	{ &EV_Actor_SetDeathHandler,				&SimpleActor::EventSetDeathHandler },
	{ &EV_Actor_GetAttackHandler,				&SimpleActor::EventGetAttackHandler },
	{ &EV_Actor_SetAttackHandler,				&SimpleActor::EventSetAttackHandler },
	{ &EV_Actor_SetHeadModel,					&Actor::EventSetHeadModel },
	{ &EV_Actor_GetHeadModel,					&Actor::EventGetHeadModel },
	{ &EV_Actor_SetHeadSkin,					&Actor::EventSetHeadSkin },
	{ &EV_Actor_GetHeadSkin,					&Actor::EventGetHeadSkin },
	{ &EV_Actor_SetNoIdle,						&Actor::EventSetNoIdle },
	{ &EV_Actor_GetNoIdle,						&Actor::EventGetNoIdle },
	{ &EV_Actor_GetEnemy,						&Actor::EventGetEnemy },
	{ &EV_Actor_GetMaxNoticeTimeScale,			&Actor::EventGetMaxNoticeTimeScale },
	{ &EV_Actor_SetMaxNoticeTimeScale,			&Actor::EventSetMaxNoticeTimeScale },
	{ &EV_Actor_SetMaxNoticeTimeScale2,			&Actor::EventSetMaxNoticeTimeScale },
	{ &EV_Actor_GetFixedLeash,					&Actor::EventGetFixedLeash },
	{ &EV_Actor_SetFixedLeash,					&Actor::EventSetFixedLeash },
	{ &EV_Actor_SetFixedLeash2,					&Actor::EventSetFixedLeash },
	{ &EV_Actor_Holster,						&Actor::EventHolster },
	{ &EV_Actor_Unholster,						&Actor::EventUnholster },
	{ &EV_SoundDone,							&Actor::EventSoundDone },
	{ &EV_Sound,								&Actor::EventSound },
	{ &EV_Actor_IsEnemyVisible,					&Actor::EventIsEnemyVisible },
	{ &EV_Actor_GetEnemyVisibleChangeTime,		&Actor::EventGetEnemyVisibleChangeTime },
	{ &EV_Actor_GetLastEnemyVisibleTime,		&Actor::EventGetLastEnemyVisibleTime },
	{ &EV_Actor_SetFallHeight,					&Actor::EventSetFallHeight },
	{ &EV_Actor_GetFallHeight,					&Actor::EventGetFallHeight },
	{ &EV_Actor_CanMoveTo,						&Actor::EventCanMoveTo },
	{ &EV_Actor_MoveDir,						&Actor::EventMoveDir },
	{ &EV_Actor_IntervalDir,					&Actor::EventIntervalDir },
	{ &EV_Actor_ResetLeash,						&Actor::EventResetLeash },
	{ &EV_Actor_Tether,							&Actor::EventTether },
	{ &EV_Actor_GetThinkState,					&Actor::EventGetThinkState },
	{ &EV_Actor_GetEnemyShareRange,				&Actor::EventGetEnemyShareRange },
	{ &EV_Actor_SetEnemyShareRange,				&Actor::EventSetEnemyShareRange },
	{ &EV_Actor_SetEnemyShareRange2,			&Actor::EventSetEnemyShareRange },
	{ &EV_Actor_GetVoiceType,					&Actor::GetVoiceType },
	{ &EV_Actor_SetVoiceType,					&Actor::SetVoiceType },
	{ &EV_Actor_SetVoiceType2,					&Actor::SetVoiceType },
	{ &EV_Remove,								&Actor::Remove },
	{ &EV_Delete,								&Actor::Remove },
	{ &EV_ScriptRemove,							&Actor::Remove },
	{ &EV_Actor_KickDir,						&Actor::EventGetKickDir },
	{ &EV_Actor_GetNoLongPain,					&Actor::EventGetNoLongPain },
	{ &EV_Actor_SetNoLongPain,					&Actor::EventSetNoLongPain },
	{ &EV_Actor_SetNoLongPain2,					&Actor::EventSetNoLongPain },
	{ &EV_Actor_GetFavoriteEnemy,				&Actor::EventGetFavoriteEnemy },
	{ &EV_Actor_SetFavoriteEnemy,				&Actor::EventSetFavoriteEnemy },
	{ &EV_Actor_SetFavoriteEnemy2,				&Actor::EventSetFavoriteEnemy },
	{ &EV_Actor_GetMumble,						&Actor::EventGetMumble },
	{ &EV_Actor_SetMumble,						&Actor::EventSetMumble },
	{ &EV_Actor_SetMumble2,						&Actor::EventSetMumble },
	{ &EV_Actor_CalcGrenadeToss,				&Actor::EventCalcGrenadeToss },
	{ &EV_Actor_GetNoSurprise,					&Actor::EventGetNoSurprise },
	{ &EV_Actor_SetNoSurprise,					&Actor::EventSetNoSurprise },
	{ &EV_Actor_SetNoSurprise2,					&Actor::EventSetNoSurprise },
	{ &EV_Actor_GetSilent,						&Actor::EventGetSilent },
	{ &EV_Actor_SetSilent,						&Actor::EventSetSilent },
	{ &EV_Actor_SetSilent2,						&Actor::EventSetSilent },
	{ &EV_Actor_GetAvoidPlayer,					&Actor::EventGetAvoidPlayer },
	{ &EV_Actor_SetAvoidPlayer,					&Actor::EventSetAvoidPlayer },
	{ &EV_Actor_SetAvoidPlayer2,				&Actor::EventSetAvoidPlayer },
	{ &EV_Actor_SetMoveDoneRadius,				&Actor::EventSetMoveDoneRadius },
	{ &EV_Actor_BeDead,							&Actor::EventBeDead },
	{ &EV_Actor_GetLookAroundAngle,				&Actor::EventGetLookAroundAngle },
	{ &EV_Actor_SetLookAroundAngle,				&Actor::EventSetLookAroundAngle },
	{ &EV_Actor_SetLookAroundAngle2,			&Actor::EventSetLookAroundAngle },
	{ &EV_Actor_HasCompleteLookahead,			&Actor::EventHasCompleteLookahead },
	{ &EV_Actor_PathDist,						&Actor::EventPathDist },
	{ &EV_Actor_CanShootEnemyFrom,				&Actor::EventCanShootEnemyFrom },
	{ &EV_Actor_CanShoot,						&Actor::EventCanShoot },
	{ &EV_Actor_GetInReload,					&Actor::EventGetInReload },
	{ &EV_Actor_SetInReload,					&Actor::EventSetInReload },
	{ &EV_Actor_SetReloadCover,					&Actor::EventSetReloadCover },
	{ &EV_Actor_BreakSpecial,					&Actor::EventBreakSpecial },
	{ &EV_Actor_SetBalconyHeight,				&Actor::EventSetBalconyHeight },
	{ &EV_Actor_SetBalconyHeight2,				&Actor::EventSetBalconyHeight },
	{ &EV_Actor_GetBalconyHeight,				&Actor::EventGetBalconyHeight },
	{ &EV_CanSee,								&Actor::CanSee },
	{ NULL, NULL }
};

Actor::GlobalFuncs_t Actor::GlobalFuncs[ MAX_GLOBAL_FUNCS ];
const_str Actor::m_csThinkNames[ MAX_GLOBAL_FUNCS ];
const_str Actor::m_csThinkStateNames[ MAX_THINKMAP ];

Actor::Actor()
{
	entflags |= EF_ACTOR;

	if( LoadingSavegame )
	{
		return;
	}

	edict->s.eType = ET_MODELANIM;
	edict->r.svFlags |= SVF_USE_CURRENT_ORIGIN;
	edict->r.ownerNum = ENTITYNUM_NONE;
	edict->clipmask = MASK_MONSTERSOLID;
	setMoveType( MOVETYPE_WALK );
	mass = 175;
	flags |= FL_THINK;
	flags &= ~FL_UNKNOWN;
	edict->r.contents = CONTENTS_NOBOTCLIP;
	setSolidType( SOLID_BBOX );
	m_fFov = 90.0f;
	takedamage = DAMAGE_AIM;
	m_fFov = cos( 0.78f );
	m_eAnimMode = 0;
	m_eEmotionMode = 0;
	m_bDoPhysics = true;
	path_failed_time = 0;
	health = 100;
	max_health = 100;
	m_fAngleYawSpeed = 360;
	m_fHeadMaxTurnSpeed = 520;
	m_bHeadAnglesAchieved = true;
	VectorClear( m_vHeadDesiredAngles );
	m_fLUpperArmTurnSpeed = 360;
	m_bLUpperArmAnglesAchieved = true;
	VectorClear( m_vLUpperArmDesiredAngles );
	m_fTorsoMaxTurnSpeed = 120;
	m_fTorsoCurrentTurnSpeed = 0;
	m_bTorsoAnglesAchieved = true;
	VectorClear( m_vTorsoDesiredAngles );
	m_pLookEntity = NULL;
	m_pPointEntity = NULL;
	m_pTurnEntity = NULL;
	VectorClear( edict->s.eyeVector );
	m_bDoAI = true;
	m_bTurretNoInitialCover = false;
	m_bThink = false;
	m_bAutoAvoidPlayer = true;
	m_fHearing = 2048;
	m_PainTime = 0;
	m_fSight = world->m_fAIVisionDistance;
	m_bNoLongPain = false;
	hit_obstacle_time = 0;
	VectorClear2D( obstacle_vel );
	m_bLockThinkState = false;
	InitThinkStates();
	SetThinkState(THINKSTATE_IDLE, 0 );
	m_fMinDistance = 128;
	m_fMinDistanceSquared = 128 * 128;
	m_fMaxDistance = 1024;
	m_fMaxDistanceSquared = 1024 * 1024;
	m_fLeash = 512;
	m_fLeashSquared = 512 * 512;
	m_iEyeUpdateTime = level.inttime;
	if( m_iEyeUpdateTime < 1000 )
		m_iEyeUpdateTime = 1000;
	m_iEyeUpdateTime += rand() % 100 + 100;
	m_vIntervalDir = vec_zero;
	m_iIntervalDirTime = 0;
	edict->r.lastNetTime = -10000000;
	m_iEnemyCheckTime = 0;
	m_iEnemyChangeTime = 0;
	m_iEnemyVisibleCheckTime = 0;
	m_bNewEnemy = false;
	m_bEnemyIsDisguised = false;
	m_bEnemyVisible = false;
	m_iEnemyVisibleChangeTime = 0;
	m_iLastEnemyVisibleTime = 0;
	m_iEnemyFovCheckTime = 0;
	m_iEnemyFovChangeTime = 0;
	m_bEnemyInFOV = false;
	m_iIgnoreSoundsMask = 0;
	m_State = -1;
	m_iStateTime = 0;
	m_iGunPositionCheckTime = 0;
	m_vLastEnemyPos = vec_origin;
	m_iLastEnemyPosChangeTime = 0;
	m_vScriptGoal = vec_origin;
	m_iNextWatchStepTime = 0;
	m_iCuriousTime = 0;
	m_iCuriousLevel = 0;
	SetCuriousAnimHint(0);
	m_bScriptGoalValid = false;
	m_bNoSurprise = false;
	memset( &m_pPotentialCoverNode, 0, sizeof( m_pPotentialCoverNode ) );
	m_iPotentialCoverCount = 0;
	m_pCoverNode = 0;
	m_csSpecialAttack = 0;
	m_sCurrentPathNodeIndex = -1;
	m_iDisguiseLevel = 1;
	m_iNextDisguiseTime = 1;
	m_iDisguisePeriod = 30000;
	m_bNeedReload = false;
	m_bInReload = false;
	m_bFaceEnemy = true;
	m_fMaxDisguiseDistSquared = 256 * 256;
	m_patrolCurrentNode = NULL;
	m_csPatrolCurrentAnim = STRING_ANIM_RUN_SCR;
	m_iSquadStandTime = 0;
	m_bPatrolWaitTrigger = false;
	m_fInterval = 128;
	m_aimNode = NULL;
	m_vHome = vec_origin;
	m_bFixedLeash = false;
	m_pTetherEnt = NULL;
	m_pGrenade = NULL;
	m_vGrenadePos = vec_origin;
	m_eGrenadeMode = AI_GREN_TOSS_NONE;
	m_iFirstGrenadeTime = 0;
	m_bGrenadeBounced = false;
	m_vGrenadeVel = vec_zero;
	m_vKickDir = vec_zero;
	m_fNoticeTimeScale = 1.0f;
	m_iCanShootCheckTime = 0;
	m_csAnimScript = STRING_ANIM_IDLE_SCR;
	m_fSoundAwareness = 100;
	m_fGrenadeAwareness = 20;
	m_fMaxNoticeTimeScale = 1.0f;
	m_bCanShootEnemy = false;
	m_bEnableEnemy = true;
	m_bDesiredEnableEnemy = true;
	m_bEnablePain = true;
	m_bAnimScriptSet = 0;
	m_AnimMode = 0;
	m_csAnimName = STRING_EMPTY;
	m_csSayAnim = STRING_EMPTY;
	m_csUpperAnim = STRING_EMPTY;
	m_fDfwRequestedYaw = 0;
	m_fDfwDerivedYaw = 0;
	m_vDfwPos = vec_zero;
	m_fDfwTime = 0;
	m_AlarmNode = NULL;
	m_csHeadSkin = STRING_EMPTY;
	m_iWallDodgeTimeout = 0;
	m_csHeadModel = STRING_EMPTY;
	m_csLoadOut = STRING_EMPTY;
	m_iRunHomeTime = 0;
	m_iLookFlags = 0;
	m_csWeapon = STRING_EMPTY;
	m_eDontFaceWallMode = 0;
	m_WallDir = 0;
	VectorClear2D( m_PrevObstacleNormal );
	m_bNoIdleAfterAnim = false;
	m_fMaxShareDistSquared = 0;
	m_bBreathSteam = false;
	m_fMoveDoneRadiusSquared = 0;
	m_bForceAttackPlayer = false;
	m_iNextLookTime = 0;
	m_fLookAroundFov = 45;
	m_bHasDesiredLookDest = false;
	m_bHasDesiredLookAngles = false;
	m_iLastFaceDecideTime = 0;
	m_iOriginTime = -1;
	m_fTurnDoneError = 0;
	m_fBalconyHeight = 128;
	m_bNoPlayerCollision = false;
	for( int i = 0; i < MAX_ORIGIN_HISTORY; i++ )
	{
		VectorClear2D( m_vOriginHistory[ i ] );
	}
	m_bAnimating = false;
	m_iCurrentHistory = 0;
	m_bDog = false;
	m_bBecomeRunner = false;
}

void Actor::ShowInfo
	(
	void
	)

{
	Com_Printf( "-------------------------------------------------------------------------------\n" );
	Com_Printf( "Info for Actor:\n" );
	Com_Printf( "Current think type: %s %s\n", ThinkName().c_str(), ThinkStateName().c_str() );
	Com_Printf( "leash: %f\n", m_fLeash );
	Com_Printf( "mindist: %f\n", m_fMinDistance );
	Com_Printf( "maxdist: %f\n", m_fMaxDistance );

	GlobalFuncs_t *func = &GlobalFuncs[ m_Think[ m_ThinkLevel ] ];
	if( func->ShowInfo )
		( this->*func->ShowInfo )( );

	Com_Printf( "-------------------------------------------------------------------------------\n" );
}

void Actor::MoveTo
	(
	Event *ev
	)

{
	m_csPatrolCurrentAnim = ev->GetConstString( 1 );

	if( ev->IsVectorAt( 2 ) )
	{
		Vector vec = ev->GetVector( 2 );
		SetPatrolCurrentNode( vec );
	}
	else
	{
		SetPatrolCurrentNode( ev->GetListener( 2 ) );
	}

	if( m_patrolCurrentNode )
	{
		m_vScriptGoal = origin;
		m_bScriptGoalValid = true;
	}

	SetThinkIdle( THINKSTATE_ATTACK );
}

void Actor::WalkTo
	(
	Event *ev
	)

{
	Event event = EV_Listener_ExecuteScript;
	event.AddConstString( STRING_GLOBAL_WALKTO_SCR );
	event.AddValue( ev->GetValue( 1 ) );
	ExecuteScript( &event );
}

void Actor::RunTo
	(
	Event *ev
	)

{
	Event event = EV_Listener_ExecuteScript;
	event.AddConstString( STRING_GLOBAL_RUNTO_SCR );
	event.AddValue( ev->GetValue( 1 ) );
	ExecuteScript( &event );
}

void Actor::CrouchTo
	(
	Event *ev
	)

{
	Event event = EV_Listener_ExecuteScript;
	event.AddConstString( STRING_GLOBAL_CROUCHTO_SCR );
	event.AddValue( ev->GetValue( 1 ) );
	ExecuteScript( &event );
}

void Actor::CrawlTo
	(
	Event *ev
	)

{
	Event event = EV_Listener_ExecuteScript;
	event.AddConstString( STRING_GLOBAL_CRAWLTO_SCR );
	event.AddValue( ev->GetValue( 1 ) );
	ExecuteScript( &event );
}

void Actor::AimAt
	(
	Event *ev
	)

{

	if( ev->IsVectorAt( 1 ) )
	{
		Vector orig = ev->GetVector(1);
		//FIXME: possible inline func /0
		if (m_aimNode)
		{
			if (m_aimNode->IsSubclassOfAnimate())
			{
				delete m_aimNode;
			}
			m_aimNode = NULL;
		}
		// 0/
		TempWaypoint * twp = new TempWaypoint;
		m_aimNode = twp;
		m_aimNode->setOrigin(orig);
	}
	else
	{
		Listener *l = ev->GetListener(1);
		//FIXME: possible inline func /0
		if (m_aimNode)
		{
			if (m_aimNode->IsSubclassOfAnimate())
			{
				delete m_aimNode;
			}
			m_aimNode = NULL;
		}
		// 0/
		if (l)
		{
			if (!checkInheritance(&SimpleEntity::ClassInfo, l->classinfo()))
			{
				ScriptError("Bad aim node with classname '%s' specified for '%s' at (%f %f %f)\n", l->getClassname(), TargetName().c_str(), origin.x, origin.y, origin.z);
			}
			else
			{
				m_aimNode = (SimpleEntity*)l;
			}
		}

	}
	//FIXME: macro THINKSTATE_something
	SetThinkIdle( m_aimNode != NULL ? 24 : 8);
}

void Actor::DefaultRestart
	(
	void
	)

{
	EndState( m_ThinkLevel );
	BeginState();
}

void Actor::SuspendState
	(
	void
	)

{
	GlobalFuncs_t *func = &GlobalFuncs[ m_Think[ m_ThinkLevel ] ];

	if( func->SuspendState )
		( this->*func->SuspendState )( );
}

void Actor::ResumeState
	(
	void
	)

{
	GlobalFuncs_t *func = &GlobalFuncs[ m_Think[ m_ThinkLevel ] ];

	if( func->ResumeState )
		( this->*func->ResumeState )( );
}

void Actor::BeginState
	(
	void
	)

{

	m_Think[ m_ThinkLevel ] = m_ThinkMap[ m_ThinkState ];

	GlobalFuncs_t *func = &GlobalFuncs[ m_Think[ m_ThinkLevel ] ];

	if( func )
		( this->*func->BeginState )();

	m_Think[ m_ThinkLevel ] = m_ThinkMap[ m_ThinkState ];
}

void Actor::EndState
	(
	int level
	)

{
	GlobalFuncs_t *func = &GlobalFuncs[ m_Think[ level ] ];

	m_Think[ level ] = 0;

	if( func->EndState )
		( this->*func->EndState )();

	if( m_pAnimThread )
		m_pAnimThread->AbortRegistration( STRING_EMPTY, this );
}

void Actor::RestartState
	(
	void
	)

{
	GlobalFuncs_t *func = &GlobalFuncs[ m_Think[ m_ThinkLevel ] ];

	if( func->RestartState)
		( this->*func->RestartState)();
}

void Actor::setContentsSolid
	(
	void
	)

{
	setContents(CONTENTS_NOBOTCLIP);
}

void Actor::InitThinkStates
	(
	void
	)

{
	for (size_t i = 0; i < MAX_THINKMAP; i++)
	{
		m_ThinkMap[i] = 0;
	}
	for (size_t i = 0; i < MAX_THINKSTATES; i++)
	{
		m_Think[i] = 0;
		m_ThinkStates[i++] = 0;
	}

	m_bDirtyThinkState = false;

	//FIXME: create proper macros
	m_ThinkMap[0] = 0;
	m_ThinkMap[1] = 8;
	m_ThinkMap[2] = 5;
	m_ThinkMap[3] = 6;
	m_ThinkMap[4] = 1;
	m_ThinkMap[5] = 9;
	m_ThinkMap[6] = 10;
	m_ThinkMap[7] = 16;
	m_ThinkMap[8] = 33;

	m_ThinkLevel = 0;
	m_ThinkState = 0;

}

void Actor::UpdateEyeOrigin
	(
	void
	)

{
	Vector eyeTag; // [esp+50h] [ebp-18h]

	int currLvlTime = level.inttime;
	int currTime = m_iEyeUpdateTime;

	if (currLvlTime > currTime)
	{
		eyeTag.x = 0.0;
		eyeTag.y = 0.0;
		eyeTag.z = 0.0;

		do
			currTime += 100;
		while (currTime < currLvlTime);

		m_iEyeUpdateTime = currTime;

		GetTag( "eyes bone", &eyeTag, &m_vEyeDir);

		eyeposition = eyeTag - origin;

		m_vEyeDir[2] = 0.0;
		VectorNormalizeFast(m_vEyeDir);

		if (eyeposition[0] < -10.5)
		{
			eyeposition[0] = -10.5;
		}
		if (eyeposition[0] > 10.5)
			eyeposition[0] = 10.5;

		if (eyeposition[1] < -10.5)
		{
			eyeposition[1] = -10.5;
		}
		if (eyeposition[1] > 10.5)
			eyeposition[1] = 10.5;

		if (eyeposition[0] < 4.5)
		{
			eyeposition[0] = 4.5;
		}
		if (eyeposition[0] > 89.5)
			eyeposition[0] = 89.5;

	}
}

bool Actor::RequireThink
	(
	void
	)

{
	G_GetEntity(0);
	if( g_entities[ 0 ].entity )
		return ( level.inttime < edict->r.lastNetTime + 60000 );
	else
		return false;
	
}

void Actor::UpdateEnemy
	(
	int iMaxDirtyTime
	)

{
	if (level.inttime > iMaxDirtyTime + m_iEnemyCheckTime)
		Actor::UpdateEnemyInternal();
}

void Actor::UpdateEnemyInternal
	(
	void
	)

{
	for (Sentient *pSent = level.m_HeadSentient[1 - m_Team]; pSent; pSent = pSent->m_NextSentient)
		m_PotentialEnemies.AddPotentialEnemy(pSent);

	m_PotentialEnemies.CheckEnemies(this);
	
	if (m_Enemy != m_PotentialEnemies.GetCurrentEnemy())
		SetEnemy(m_PotentialEnemies.GetCurrentEnemy(), false);

	m_fNoticeTimeScale = (level.inttime - m_iEnemyCheckTime) * 0.0001 + m_fNoticeTimeScale;

	if (m_fNoticeTimeScale > m_fMaxNoticeTimeScale)
		m_fNoticeTimeScale = m_fMaxNoticeTimeScale;

	m_iEnemyCheckTime = level.inttime;
}

void Actor::SetEnemy
	(
	Sentient *pEnemy,
	bool bForceConfirmed
	)

{
	if (pEnemy == m_Enemy)
	{
		return;
	}

	if (m_Enemy)
	{
		m_Enemy->m_iAttackerCount--;
	}
	m_bNewEnemy = m_Enemy == NULL;
	delete m_Enemy;

	m_Enemy = pEnemy;

	m_iEnemyChangeTime = level.inttime;

	if (m_Enemy)
	{
		Event e1(EV_Actor_ShareEnemy);
		//FIXME: macro
		PostEvent(e1, 0.75);

		if (m_Enemy->m_bHasDisguise)
		{
			if (level.inttime > m_iEnemyVisibleCheckTime)
			{
				//this cansee call changes m_bEnemyVisible and m_bEnemyInFOV
				//FIXME: macro
				CanSee(m_Enemy, 0, 0.828 * world->farplane_distance);
			}
		}

		m_bEnemyIsDisguised = m_Enemy->m_bHasDisguise && (m_Enemy->m_bIsDisguised || !m_bEnemyVisible);

		SetEnemyPos(m_Enemy->origin);

		m_Enemy->m_iAttackerCount++;

		if (bForceConfirmed)
		{
			m_PotentialEnemies.ConfirmEnemy(this, m_Enemy);
		}
	}
}

void Actor::SetEnemyPos
(
	Vector vPos
)

{
	if (m_vLastEnemyPos != vPos)
	{
		m_iLastEnemyPosChangeTime = level.inttime;
		m_vLastEnemyPos = vPos;
		if (m_Enemy)
		{
			mTargetPos += m_Enemy->eyeposition;
		}
		else
		{
			mTargetPos.z += 88;
		}
		//FIXME: Virtual function call here.
		if (mTargetPos.z - m_Enemy->eyeposition.z < 128)
		{
			mTargetPos.z -= 16;
		}
	}
}

void Actor::ResetBodyQueue
	(
	void
	)

{
	//weird useless loop ?

	for (int i = MAX_BODYQUEUE-1; i >= 0; i--)
	{
		//do nothing.
	}

	//FIXME: linker err
	//mCurBody = 0;
}

void Actor::AddToBodyQue
	(
	void
	)

{
	//FIXME: wrong notation, maybe mBodyQueu + mCurBody ?
	/*
	if (mBodyQueue[4*mCurBody+3])
	{
		Event ev(EV_Remove);
		PostEvent(ev, 0, 0);
	}
	mBodyQueue[4 * mCurBody] = this;
	mCurBody = (mCurBody + 1) % 5;
	*/
}

Vector Actor::GetAntiBunchPoint
	(
	void
	)

{
	int count = 0;
	Vector ret = vec_origin;
	if (m_pNextSquadMate != this)
	{
		for (auto pSquadMate = m_pNextSquadMate; pSquadMate != this; pSquadMate = pSquadMate->m_pNextSquadMate)
		{

			Vector dist = origin - pSquadMate->origin;
			float distSq = dist * dist;
			if (distSq != 0)
			{
				if (m_fInterval*m_fInterval > distSq)
				{
					count++;
					ret += origin + (dist/sqrt(distSq)) * (m_fInterval - sqrt(distSq));
				}
			}
		}

		if (count)
		{
			if (count != 1)
			{
				ret /= count;
			}
		}
		else
		{
			ret = origin;
		}
	}
	return ret;
}

void Actor::InitVoid
	(
	GlobalFuncs_t *func
	)

{
	func->IsState = &Actor::IsVoidState;
}

const char *Actor::DumpCallTrace
	(
	const char *pszFmt,
	...
	) const

{
	cvar_t *sv_mapname;
	tm *ptm; 
	time_t t;
	Vector vPlayer = vec_zero;
	char szFile[64];
	int i2;
	int i1;
	int i;
	char szTemp[65536];
	char szTemp2[1024];

	va_list va;
	
	va_start(va, pszFmt);
	
	sv_mapname = gi.Cvar_Get("mapname", "unknown", NULL);

	Player *p = (Player *)G_GetEntity(0);
	if (p)
	{
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
		vPlayer[2]);

	if (pszFmt)
	{
		i2 = i + vsprintf(&szTemp[i], pszFmt, va);
	}
	if (i2 != i)
	{
		strcpy(&szTemp[i2], "\n--------------------------------------\n");
	}

	t = time(0);
	ptm = localtime(&t);
	sprintf(
		szFile,
		"main\\ai_trace_%s_%i_%i_%02i%02i.log",
		sv_mapname->string,
		ptm->tm_mon + 1,
		ptm->tm_mday,
		ptm->tm_hour,
		ptm->tm_min);

	for (i=0; szFile[i]; i++)
	{
		if (szFile[i] == '\\' || szFile[i] == '/')
			szFile[i] = '_';
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
		szTemp);
}

void Actor::Init
	(
	void
	)

{
	g_showinfo = gi.Cvar_Get("g_showinfo", "0", NULL);

	for (int i = 0; i < MAX_GLOBAL_FUNCS; i++)
	{
		GlobalFuncs[i].ThinkState = NULL;
		GlobalFuncs[i].BeginState = NULL;
		GlobalFuncs[i].ResumeState = NULL;
		GlobalFuncs[i].EndState = NULL;
		GlobalFuncs[i].SuspendState = NULL;
		GlobalFuncs[i].RestartState = &Actor::DefaultRestart;
		GlobalFuncs[i].FinishedAnimation = NULL;
		GlobalFuncs[i].PostShoot = NULL;
		GlobalFuncs[i].Pain = &Actor::DefaultPain;
		GlobalFuncs[i].Killed = &Actor::DefaultKilled;
		GlobalFuncs[i].PassesTransitionConditions = NULL;
		GlobalFuncs[i].ShowInfo = NULL;
		GlobalFuncs[i].ReceiveAIEvent = &Actor::DefaultReceiveAIEvent;
		GlobalFuncs[i].IsState = NULL;
		GlobalFuncs[i].PathnodeClaimRevoked = NULL;
	}

	GlobalFuncs[0].IsState = &Actor::IsVoidState;

	//FIXME: create enum
	InitTurret(&GlobalFuncs[1]);
	InitCover(&GlobalFuncs[2]);
	InitPatrol(&GlobalFuncs[3]);
	InitRunner(&GlobalFuncs[4]);
	InitPain(&GlobalFuncs[5]);
	InitKilled(&GlobalFuncs[6]);
	InitIdle(&GlobalFuncs[8]);
	InitCurious(&GlobalFuncs[9]);
	InitDisguiseSalute(&GlobalFuncs[10]);
	InitDisguiseSentry(&GlobalFuncs[11]);
	InitDisguiseOfficer(&GlobalFuncs[12]);
	InitDisguiseRover(&GlobalFuncs[13]);
	InitDisguiseNone(&GlobalFuncs[14]);
	InitAlarm(&GlobalFuncs[15]);
	InitGrenade(&GlobalFuncs[16]);
	InitMachineGunner(&GlobalFuncs[17]);
	InitDogIdle(&GlobalFuncs[18]);
	InitDogAttack(&GlobalFuncs[19]);
	InitDogCurious(&GlobalFuncs[20]);
	InitAnim(&GlobalFuncs[22]);
	InitAnimCurious(&GlobalFuncs[23]);
	InitAim(&GlobalFuncs[24]);
	InitBalconyIdle(&GlobalFuncs[25]);
	InitBalconyCurious(&GlobalFuncs[26]);
	InitBalconyAttack(&GlobalFuncs[27]);
	InitBalconyDisguise(&GlobalFuncs[28]);
	InitBalconyGrenade(&GlobalFuncs[29]);
	InitBalconyPain(&GlobalFuncs[30]);
	InitBalconyKilled(&GlobalFuncs[31]);
	InitWeaponless(&GlobalFuncs[32]);
	InitNoClip(&GlobalFuncs[33]);
	InitDead(&GlobalFuncs[34]);
	
	AddWaitTill(STRING_TRIGGER);
	AddWaitTill(STRING_MOVE);
	AddWaitTill(STRING_VISIBLE);
	AddWaitTill(STRING_ANIMDONE);
	AddWaitTill(STRING_UPPERANIMDONE);
	AddWaitTill(STRING_SAYDONE);
	AddWaitTill(STRING_FLAGGEDANIMDONE);
	AddWaitTill(STRING_DEATH);
	AddWaitTill(STRING_PAIN);
	AddWaitTill(STRING_HASENEMY);
	AddWaitTill(STRING_MOVEDONE);
	AddWaitTill(STRING_SOUNDDONE);
	AddWaitTill(STRING_TURNDONE);

	if (developer->integer)
	{
		Com_Printf("sizeof(Actor) == %i\n", sizeof(Actor));
		//FIXME: magic ??
		Com_Printf("Magic sizeof actor numer: %d\n", sizeof(Actor));
	}
}

void Actor::FixAIParameters
	(
	void
	)

{
	//FIXME: needs revision(silly mistakes)

	float distmin, distmin2, distmax;
	if (m_pTetherEnt)
	{
		float leash = 64;
		if ( m_pTetherEnt->IsSubclassOfEntity() )
		{
			leash = m_pTetherEnt->angles.y - m_pTetherEnt->origin.y + m_pTetherEnt->angles.z + m_pTetherEnt->origin.z;
		}
		if ( leash <= m_fLeash)
		{
			distmin = m_fLeash;
		}
		else
		{
			Com_Printf("^~^~^ LD ERROR: (entnum %i, radnum %i, targetname '%s'):    increasing leash from %g to %g.\n"
				"^~^~^ Leash must be larger than the size of the entity to which an AI is tethered.\n"
				"\n",
				entnum,
				radnum,
				targetname.c_str(),
				m_fLeash,
				leash);
			m_fLeash = leash;
			distmin = leash;
			m_fLeashSquared = leash * leash;
		}
	}
	else
	{
		distmin = m_fLeash;
	}
	if (m_fMinDistance <= distmin)
	{
		distmax = m_fMinDistance;
	}
	else
	{
		Com_Printf("^~^~^ LD ERROR: (entnum %i, radnum %i, targetname '%s'):    reducing mindist from %g to %g to match leash.\n"
			"^~^~^ Leash must be greater than mindist, or the AI will want to both run away and stay put.\n"
			"\n",
			entnum,
			radnum,
			targetname.c_str(),
			m_fMinDistance,
			distmin);
		m_fMinDistance = distmin;
		distmax = distmin;
		m_fMinDistanceSquared = distmin * distmin;
	}
	distmax += 128;
	if (distmax - 1 > m_fMaxDistance)
	{
		Com_Printf("^~^~^ LD ERROR: (entnum %i, radnum %i, targetname '%s'):    increasing maxdist from %g to %g to exceed mindist.\n"
			"^~^~^ Maxdist should be %i greater than mindist, or the AI will want to both run away and charge, or just do oscillatory behavior.\n"
			"\n",
			entnum,
			radnum,
			targetname.c_str(),
			m_fMaxDistance,
			distmax,
			128);
		
		m_fMaxDistance = distmax;
		m_fMaxDistanceSquared = distmax * distmax;
	}
	if (world->farplane_distance > 0)
	{
		//FIXME: macro and floats
		if (m_fMaxDistance > world->farplane_distance * 0.828 +1)
		{
			Com_Printf(
				"^~^~^ LD ERROR: (entnum %i, radnum %i, targetname '%s'):    reducing maxdist from %g to %g to be %g%% of farplane.\n"
				"^~^~^ Maxdist should be this distance within fog, or AI will be able to see and attack through fog.\n"
				"\n",
				entnum,
				radnum,
				targetname.c_str(),
				m_fMaxDistance,
				world->farplane_distance * 0.828,
				2);
			distmin2 = m_fMinDistance;
			m_fMaxDistance = world->farplane_distance * 0.828;
			m_fMaxDistanceSquared = m_fMaxDistance * m_fMaxDistance;
			if (distmin2 + 128.0 - 1.0 > m_fMaxDistance)
			{
				Com_Printf(
					"^~^~^ LD ERROR: (entnum %i, radnum %i, targetname '%s'):    reducing mindist from %g to %g to be less than maxdist "
					"after fog adjustment.\n"
					"\n",
					entnum,
					radnum,
					targetname.c_str(),
					distmin2,
					m_fMaxDistance <= 128 ? 0 : m_fMaxDistance - 128);
				m_fMinDistance = m_fMaxDistance - 128.0;
				if (m_fMaxDistance - 128.0 < 0)
				{
					m_fMinDistance = 0.0;
				}
				m_fMinDistanceSquared = m_fMinDistance * m_fMinDistance;
			}
		}
	}
}

bool Actor::AttackEntryAnimation
	(
	void
	)

{
	if (m_Enemy)
	{
		//FIXME: macro
		if (level.inttime >= level.m_iAttackEntryAnimTime + 3000)
		{
			float distSq = (m_Enemy->origin - origin).lengthXY(true);
			//FIXME: macro
			if (m_bNoSurprise || distSq >= 65536) //2 power 16
			{
				if (distSq > 1048576 && !(rand() & 3))//2 power 20
				{
					if (m_pNextSquadMate != this)
					{
						for (auto pSquadMate = m_pNextSquadMate; true; pSquadMate->m_pNextSquadMate)
						{
							if (m_fInterval*m_fInterval*4 > (pSquadMate->origin - origin).lengthSquared())
							{
								break;
							}
							if (pSquadMate == this)
							{
								if (m_bNewEnemy)
								{
									//FIXME: macro
									Anim_Say(STRING_ANIM_SAY_SIGHTED_SCR, 200, false);
									m_bNoSurprise = true;
									m_bNewEnemy = false;
									return false;
								}
							}
						}
						vec2_t vDelta;
						vDelta[0] = m_Enemy->origin.x - origin.x;
						vDelta[1] = m_Enemy->origin.y - origin.y;
						if (vDelta[0] != 0 || vDelta[1] != 0)
						{
							//continue here
							m_YawAchieved = false;
							m_DesiredYaw = vectoyaw(vDelta);
						}

						m_bHasDesiredLookAngles = true;

						vectoangles(m_Enemy->origin - origin, m_DesiredLookAngles);

						m_DesiredLookAngles[0] = AngleNormalize180(m_DesiredLookAngles[0]);
						m_DesiredLookAngles[1] = AngleNormalize180(m_DesiredLookAngles[1] - angles[1]);
						m_csNextAnimString = STRING_ANIM_STANDIDENTIFY_SCR;

						m_bNextForceStart = true;
						this->m_bNoSurprise = true;
						this->m_bNewEnemy = false;

						return true;

					}
				}
			}
			else
			{
				//FIXME: macro
				if (rand() > distSq * 0.0026041667)
				{
					//FIXME: macro
					m_eNextAnimMode = 1;
					m_csNextAnimString = STRING_ANIM_STANDSHOCK_SCR;

					m_bNextForceStart = true;
					this->m_bNoSurprise = true;
					this->m_bNewEnemy = false;

					return true;
				}
			}
		}

		if (m_bNewEnemy)
		{
			//FIXME: macro
			Anim_Say(STRING_ANIM_SAY_SIGHTED_SCR, 200, false);
			m_bNoSurprise = true;
			m_bNewEnemy = false;
			return false;
		}

	}
	return false;
}

void Actor::CheckForThinkStateTransition
	(
	void
	)

{
	if (!CheckForTransition(0, THINKSTATE_GRENADE))
	{
		if (!m_bEnableEnemy)
		{
			CheckForTransition(0, THINKSTATE_IDLE);
		}
		else
		{
			if (!CheckForTransition(0, THINKSTATE_ATTACK))
			{

				if (!CheckForTransition(0, THINKSTATE_DISGUISE))
				{

					if (CheckForTransition(0, THINKSTATE_CURIOUS))
					{
						m_pszDebugState = "from_sight";
					}
					else
					{
						CheckForTransition(0, THINKSTATE_IDLE);
					}
				}
			}
		}
	}
}

bool Actor::CheckForTransition
	(
	int state,
	int level
	)

{
	GlobalFuncs_t *func;

	if( m_ThinkStates[ level ] != state )
	{
		func = &GlobalFuncs[ m_ThinkMap[ state ] ];

		if( func->PassesTransitionConditions && ( this->*( func->PassesTransitionConditions ) )() )
		{
			SetThinkState( state, 0 );
			return true;
		}
	}

	return false;
}
bool Actor::PassesTransitionConditions_Grenade
	(
	void
	)

{
	if( !m_bLockThinkState && m_bEnableEnemy && m_pGrenade )
	{
		return m_fGrenadeAwareness >= random();
	}

	return false;
}

bool Actor::PassesTransitionConditions_Attack
	(
	void
	)

{
	if( level.inttime > m_iEnemyCheckTime )
		UpdateEnemyInternal();

	if( m_bLockThinkState )
		return false;

	if( m_Enemy && !( m_Enemy->flags & FL_NOTARGET ) )
	{
		if( m_bEnemyIsDisguised || m_Enemy->m_bIsDisguised )
		{
			if( !m_bForceAttackPlayer && m_ThinkState != 4 )
				return true;
		}

		if( m_PotentialEnemies.GetCurrentVisibility() > 0.999f )
			return true;
	}

	return false;
}

bool Actor::PassesTransitionConditions_Disguise
	(
	void
	)

{
	if( m_bLockThinkState )
		return false;

	if( m_iNextDisguiseTime > level.inttime )
		return false;

	m_iNextDisguiseTime = level.inttime + 200;

	if( level.inttime > m_iEnemyCheckTime + 200 )
		UpdateEnemyInternal();

	if( !m_Enemy )
		return false;

	if( m_bEnemyIsDisguised || m_Enemy->m_bIsDisguised )
	{
		if( !m_bForceAttackPlayer && m_ThinkState != 4 )
		{
			if( m_PotentialEnemies.GetCurrentVisibility() > 0.999f )
			{
				if( fabs( m_Enemy->origin[ 2 ] - origin[ 2 ] ) <= 48.0f )
				{
					vec2_t delta;
					float fRadius;

					delta[ 0 ] = m_Enemy->origin[ 0 ] - origin[ 0 ];
					delta[ 1 ] = m_Enemy->origin[ 1 ] - origin[ 1 ];

					fRadius = delta[ 0 ] * delta[ 0 ] + delta[ 1 ] * delta[ 1 ];

					if( fRadius > 1024.0f && fRadius < m_fMaxDisguiseDistSquared )
					{
						Player *player = ( Player * )G_GetEntity( 0 );
						Vector pos = EyePosition();

						return G_SightTrace(
							pos,
							vec_zero,
							vec_zero,
							player->centroid,
							this,
							player,
							MASK_TRANSITION,
							qfalse,
							"Actor::PassesTransitionConditions_Disguise" );
					}
				}
			}
		}
	}

	return false;
}

bool Actor::PassesTransitionConditions_Curious
	(
	void
	)

{

	if( level.inttime > m_iEnemyCheckTime + 200 )
		UpdateEnemyInternal();

	if( m_bLockThinkState )
		return false;

	if( !m_Enemy && m_iCuriousTime )
	{
		SetCuriousAnimHint(6);
		return true;
	}

	if( m_bEnemyIsDisguised || m_Enemy->m_bIsDisguised )
	{
		if( !m_bForceAttackPlayer && m_ThinkState != 4 )
		{
			SetCuriousAnimHint(6);
			return true;
		}
	}

	if( m_PotentialEnemies.GetCurrentVisibility() <= 0.999f )
	{
		SetCuriousAnimHint(6);
		return true;
	}

	return false;
}

bool Actor::PassesTransitionConditions_Idle
	(
	void
	)

{

	if( m_bEnableEnemy )
	{
		if( level.inttime > m_iEnemyCheckTime + 500 )
			UpdateEnemyInternal();
	}

	if( m_bLockThinkState )
		return false;

	if( !m_Enemy && !m_iCuriousTime )
		return true;

	return false;
}

void Actor::UpdateEnableEnemy
	(
	void
	)

{
	if (m_bEnableEnemy != m_bDesiredEnableEnemy)
	{
		m_bEnableEnemy = m_bDesiredEnableEnemy;
		if (m_bDesiredEnableEnemy)
		{
			if (!m_bFixedLeash)
			{
				m_vHome = origin;
			}
		}
		else
		{
			//FIXME: macros
			if ((m_ThinkStates[0] - 4) <= 2)
			{
				SetThinkState(THINKSTATE_IDLE, 0);
			}
			SetEnemy(NULL, 0);
		}
	}
}

void Actor::ThinkStateTransitions
	(
	void
	)

{
	int newThinkLevel;
	int newThinkState;

	GlobalFuncs_t *func;// = &GlobalFuncs[m_Think[m_ThinkLevel]];

	for (newThinkLevel = MAX_THINKSTATES -1; newThinkLevel >=0 ; newThinkLevel--)
	{
		newThinkState = m_ThinkMap[newThinkLevel];
		if (newThinkState)
		{
			break;
		}
	}

	if (m_ThinkMap[newThinkState] == m_Think[m_ThinkLevel])
	{
		if (newThinkLevel != m_ThinkLevel || m_ThinkState != newThinkState)
		{
			m_ThinkLevel = newThinkLevel;
			m_ThinkState = newThinkState;
			m_Think[newThinkLevel] = m_ThinkMap[newThinkState];
			return;
		}

		func = &GlobalFuncs[m_ThinkMap[newThinkState]];
		if (func->RestartState)
			(this->*func->RestartState)();
	}
	else
	{
		if (newThinkLevel > 0)
		{
			for (int i = 0 ; i < newThinkLevel; i++)
			{
				if (!m_ThinkStates[i])
					EndState(i);
			}
		}
		if (newThinkLevel > m_ThinkLevel)
		{
			SuspendState();

			m_ThinkLevel = newThinkLevel;
			m_ThinkState = m_ThinkStates[newThinkLevel];
			m_Think[newThinkLevel] = m_ThinkMap[m_ThinkState];

			BeginState();
			return;
		}
		EndState(m_ThinkLevel);

		if (newThinkLevel == this->m_ThinkLevel)
		{
			m_ThinkState = m_ThinkStates[newThinkLevel];
			m_Think[newThinkLevel] = m_ThinkMap[m_ThinkState];

			BeginState();
			return;
		}

		m_ThinkLevel = newThinkLevel;
		m_ThinkState = m_ThinkStates[newThinkLevel];


		if (m_Think[newThinkLevel] != m_ThinkMap[m_ThinkState])
		{
			EndState(newThinkLevel);

			
			m_Think[m_ThinkLevel] = m_ThinkMap[m_ThinkState];

			BeginState();
			return;

		}

		func = &GlobalFuncs[m_ThinkMap[newThinkState]];
		if (func->ResumeState)
			(this->*func->ResumeState)();
	}
}

void Actor::TransitionState
	(
	int iNewState,
	int iPadTime
	)

{
	// not found in ida
	
}

void Actor::ChangeAnim
	(
	void
	)

{
	//FIXME: needs organization.
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
	}
	//FIXME: macro
	if (m_ThinkState != THINKSTATE_ATTACK)
	{
		if (m_ThinkState > THINKSTATE_ATTACK)
		{
			if (m_ThinkState != THINKSTATE_GRENADE)
			{
				if (m_bMotionAnimSet && !m_bLevelMotionAnim)
					AnimFinished(m_iMotionSlot, true);
				if (m_bActionAnimSet && !m_bLevelActionAnim)
					Actor::AnimFinished(m_iActionSlot, true);

				if (m_bSayAnimSet && !m_bLevelSayAnim)
				{
					AnimFinished(m_iSaySlot, true);

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
					}
				}
			}
			else
			{
				if (m_bMotionAnimSet)
					AnimFinished(m_iMotionSlot, true);
				if (m_bActionAnimSet)
					AnimFinished(m_iActionSlot, true);
				if (m_bSayAnimSet)
				{
					if (m_bLevelSayAnim == 2)
					{
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
						}
					}
				}
			}

		}
		else
		{
			if (m_ThinkState < THINKSTATE_PAIN)
			{
				if (m_bMotionAnimSet && !m_bLevelMotionAnim)
					AnimFinished(m_iMotionSlot, true);
				if (m_bActionAnimSet && !m_bLevelActionAnim)
					Actor::AnimFinished(m_iActionSlot, true);

				if (m_bSayAnimSet && !m_bLevelSayAnim)
				{
					AnimFinished(m_iSaySlot, true);

				//LABEL_14:
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
					}
				}
			}
			else
			{
				if (m_bMotionAnimSet)
					AnimFinished(m_iMotionSlot, true);
				if (m_bActionAnimSet)
					AnimFinished(m_iActionSlot, true);

				if (m_bSayAnimSet == 0)
				{
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
					}
				}
			}

		}
	
	}
}

void Actor::UpdateSayAnim
	(
	void
	)

{
	if ((m_ThinkState) > 3)
	{
		int animnum = gi.Anim_NumForName(edict->tiki, Director.GetString(m_csSayAnim).c_str());
		if (animnum == -1)
		{
			return;
		}
		int flags = gi.Anim_FlagsSkel(edict->tiki, animnum);

		if (flags & ANIM_TOGGLEBIT)
		{
			if (m_ThinkState != 4 && m_ThinkState != 7)
			{
				ChangeActionAnim();
				if (flags & ANIM_NOACTION)
				{
					ChangeMotionAnim();

					StartMotionAnimSlot(0, animnum, 1);

					m_bLevelActionAnim = true;
					m_bLevelMotionAnim = true;

					m_iActionSlot = GetMotionSlot(0);
					m_iMotionSlot = GetMotionSlot(0);
				}
				else
				{
					m_bActionAnimSet = true;

					StartActionAnimSlot(animnum);

					m_bLevelActionAnim = true;

					m_iActionSlot = GetActionSlot(0);
				}
				ChangeSayAnim();
				m_bSayAnimSet = true;
				m_bNextLevelSayAnim = false;
				m_bLevelSayAnim = m_bNextLevelSayAnim; 
				m_iSaySlot = m_iActionSlot;
				return;
			}
		}
		else if (m_bNextLevelSayAnim == 2 || m_ThinkState != 4 && m_ThinkState != 7)
		{
			ChangeSayAnim();
			m_bSayAnimSet = true;
			StartSayAnimSlot(animnum);
			m_bNextLevelSayAnim = 0;
			m_bLevelSayAnim = m_bNextLevelSayAnim;
			m_iSaySlot = GetSaySlot();
			return;
		}
	}

	if (!m_bSayAnimSet)
		Unregister(STRING_SAYDONE);
}

void Actor::UpdateUpperAnim
	(
	void
	)

{
	int animnum = gi.Anim_NumForName(edict->tiki, Director.GetString(m_csUpperAnim).c_str());
	if (animnum != -1)
	{
		if (m_ThinkState == 4 || m_ThinkState == 7 || m_ThinkState == 3 || m_ThinkState == 2)
		{
			if (!m_bActionAnimSet)
			{
				Unregister(STRING_UPPERANIMDONE);
			}
		}
		else
		{
			ChangeActionAnim();
			
			m_bActionAnimSet = true;

			StartActionAnimSlot(animnum);

			m_bLevelActionAnim = true;
			m_iActionSlot = SimpleActor::GetActionSlot(0);

		}
	}
}

void Actor::UpdateAnim
	(
	void
	)

{
	m_bAnimating = true;
	SimpleActor::UpdateAim();
	if (UpdateSelectedAnimation())
	{
		ChangeAnim();
	}

	if (Director.iPaused-- == 1)
	{
		Director.ExecuteRunning();
	}
	Director.iPaused++;

	if (m_csSayAnim != STRING_EMPTY)
	{
		UpdateSayAnim();
		m_csSayAnim = STRING_EMPTY;
	}

	if (m_csUpperAnim != STRING_EMPTY)
	{
		UpdateUpperAnim();
		m_csUpperAnim = STRING_EMPTY;
	}

	if (!m_bSayAnimSet && !m_bDog)
	{
		UpdateEmotion();
	}

	//FIXME: macro
	for (int slot = 0; slot < 14; slot++)
	{

		//FIXME: better notation
		if (!((m_bUpdateAnimDoneFlags >> slot) & 1) )
		{
			UpdateAnimSlot(slot);
		}
	}

	float fAnimTime, fAnimWeight, total_weight = 0, time = 0;
	for (int slot = 0; slot < 14; slot++)
	{
		if ((m_weightType[slot] == 1 || m_weightType[slot] == 4) && animFlags[slot] & ANIM_LOOP)
		{
			UseSyncTime(slot, 1);
			fAnimTime = AnimTime(slot);
			fAnimWeight = edict->s.frameInfo[slot].weight;
			//FIXME: macro
			if ((int)time & 0x7F800000 == 0x7F800000)
			{
				Com_Printf("ent %i, targetname '%s', anim '%s', slot %i, fAnimTime %f, fAnimWeight %f\n",
					entnum,
					targetname.c_str(),
					AnimName(slot),
					slot,
					fAnimTime,
					fAnimWeight);
			}
			time += fAnimTime * fAnimWeight;
			total_weight += fAnimWeight;
		}
		else
		{
			UseSyncTime(slot, 0);
		}
	}

	if (total_weight != 0)
	{
		SetSyncRate(time / total_weight);
	}
}

void Actor::StoppedWaitFor
	(
	const_str name,
	bool bDeleting
	)

{

	GlobalFuncs_t *func = &GlobalFuncs[m_Think[m_ThinkLevel]];

	if (func->FinishedAnimation)
		(this->*func->FinishedAnimation)();

	g_iInThinks ^= 1;//toggle
}

bool Actor::ValidGrenadePath
	(
	Vector& vFrom,
	Vector& vTo,
	Vector& vVel
	)

{
	//FIXME: macros
	float fGravity, fTime1, fTime2, fTime3, fTimeLand;
	Vector mins(-4, -4, -4);
	Vector maxs(4, 4, 4);

	Vector vPoint1, vPoint2, vPoint3;

	if (vVel.lengthSquared() > 589824)
	{
		return false;
	}
	fGravity = 0.8 * sv_gravity->value;

	fTime1 = vVel.z * fGravity / 0.5;

	vPoint1.x = vVel.x * fTime1 + vFrom.x;
	vPoint1.y = vVel.y * fTime1 + vFrom.y;
	vPoint1.z = vVel.z * fTime1 * 0.75 + vFrom.z;

	maxs.z = fGravity * 0.125 * fTime1 * fTime1 + 4;

	if (ai_debug_grenades->integer)
	{
		G_DebugLine(vFrom, vPoint1, 1.0, 0.5, 0.5, 1.0);
	}

	if (G_SightTrace(vFrom,
		mins,
		maxs,
		vPoint1,
		this,
		NULL,
		//FIXME: macro
		1107569409,
		qfalse,
		"Actor::ValidGrenadePath 1"))
	{
		fTime2 = fTime1 + fTime1;

		vPoint2.x = vVel.x * fTime2 + vFrom.x;
		vPoint2.y = vVel.y * fTime2 + vFrom.y;
		vPoint2.z = vVel.z * fTime2 * 0.5 + vFrom.z;

		if (ai_debug_grenades->integer)
		{
			G_DebugLine(vPoint1, vPoint2, 1.0, 0.5, 0.5, 1.0);
		}
		if (G_SightTrace(vPoint1,
			mins,
			maxs,
			vPoint2,
			this,
			NULL,
			//FIXME: macro
			1107569409,
			qfalse,
			"Actor::ValidGrenadePath 2"))
		{
			if (fabs(vVel.y) >= fabs(vVel.x))
			{
				fTimeLand = (vTo.y - vFrom.y) / vVel.y;
			}
			else
			{
				fTimeLand = (vTo.x - vFrom.x) / vVel.x;
			}

			fTime3 = (fTime2 + fTimeLand) / 2;
			maxs.z = fGravity * 0.03125 * (fTimeLand - fTime2) * (fTimeLand - fTime2) + 4;


			vPoint3.x = vVel.x * fTime3 + vFrom.x;
			vPoint3.y = vVel.y * fTime3 + vFrom.y;
			vPoint3.z = (vVel.z - fGravity * 0.5 * fTime3) * fTime3 + vFrom.z; 
			
			if (ai_debug_grenades->integer)
			{
				G_DebugLine(vPoint2, vPoint3, 1.0, 0.5, 0.5, 1.0);
			}
			if (G_SightTrace(vPoint2,
				mins,
				maxs,
				vPoint3,
				this,
				NULL,
				//FIXME: macro
				1107569409,
				qfalse,
				"Actor::ValidGrenadePath 3"))
			{

				if (ai_debug_grenades->integer)
				{
					G_DebugLine(vPoint3, vTo, 1.0, 0.5, 0.5, 1.0);
				}
				trace_t trace = G_Trace(vPoint3, mins, maxs, vTo, this, 1107569409, qfalse, "Actor::ValidGrenadePath 4");
				if (!trace.allsolid)
				{
					if (trace.ent)
					{
						if (trace.ent->entity->IsSubclassOfSentient())
						{
							Sentient *sen = (Sentient *)trace.ent->entity;
							if (sen->m_Team != m_Team)
							{
								return true;
							}
						}

					}
				}
				else if(trace.entityNum == 1022 && trace.plane.normal[2] >= 1)
				{
					return true;
				}
			}
		}
	}
	return false;
}

Vector Actor::CalcThrowVelocity
	(
	Vector& vFrom,
	Vector& vTo
	)

{
	//FIXME: needs revivsion + variable names
	Vector ret;
	float v10; // ST0C_4
	long double v11; // fst3
	float v13; // ST0C_4
	//////
	float fVelHorz;
	float fVelVert;
	float fDeltaZ;
	float fDistance;
	float fHorzDistSquared;

	fVelHorz = vTo.x - vFrom.x;
	fVelVert = vTo.y - vFrom.y;
	fDeltaZ = vTo.z - vFrom.z;

	fHorzDistSquared = fVelVert * fVelVert + fVelHorz * fVelHorz;

	fDistance = sqrt(fDeltaZ * fDeltaZ + fHorzDistSquared);

	v10 = sqrt(0.8 * sv_gravity->value * 0.5 * fHorzDistSquared / fDistance);
	//mohaa : vz = sqrt(hd * g/2 * 0.8 * cos(theta)), theta=inclination angle.

	v11 = (fDistance + fDeltaZ) / (fDistance - fDeltaZ) / fHorzDistSquared;
	v13 = v10 * sqrt(v11);
	ret.z = v10;
	ret.x = fVelHorz * v13;
	ret.y = fVelVert * v13;
	return ret;
}

Vector Actor::CanThrowGrenade
	(
	Vector& vFrom,
	Vector& vTo
	)

{
	Vector vVel = vec_zero;

	vVel = CalcThrowVelocity(vFrom, vTo);
	if ( vVel == vec_zero || !ValidGrenadePath(vFrom, vTo, vVel))
	{
		return vec_zero;
	}
	else
	{
		return vVel;
	}
}

Vector Actor::CalcRollVelocity
	(
	Vector& vFrom,
	Vector& vTo
	)

{
	float fOOTime, fVelVert, fVelHorz, fGravity;
	Vector vDelta = vFrom - vTo, vRet;


	if (vDelta[2] > 0)
	{
		fGravity = 0.8 * sv_gravity->value;

		vRet[2] = sqrt(-vDelta[2] * fGravity);
		//FIXME: macro
		//FIXME: not sure of this line.
		fOOTime = fGravity * 0.21961521 / vRet[2];

		fVelHorz = -vDelta[0];
		fVelVert = -vDelta[1];

		vRet[0] = fVelHorz * fOOTime;
		vRet[2] = fVelVert * fOOTime;
	}
	else
	{
		return vec_zero;
	}
}

Vector Actor::CanRollGrenade
	(
	Vector& vFrom,
	Vector& vTo
	)

{
	Vector vVel = vec_zero;
	vVel = CalcRollVelocity(vFrom, vTo);
	if (vVel == vec_zero || !ValidGrenadePath(vFrom, vTo, vVel))
	{
		return vec_zero;
	}
	else
	{
		return vVel;
	}
}

bool Actor::CanTossGrenadeThroughHint
	(
	GrenadeHint *pHint,
	Vector& vFrom,
	Vector& vTo,
	bool bDesperate,
	Vector *pvVel,
	eGrenadeTossMode *peMode
	)

{
	float fGravity = 0.8 * sv_gravity->value;
	float fAngle, fTemp, fTemp2, fTemp3, fTemp4, fVelVert, fVelHorz, fRange, fRangeSquared, fDist, fDistSquared, fHeight;
	bool bSuccess = false;

	Vector vDelta, vHintDelta;

	if (!bDesperate)
	{
		vHintDelta = pHint->origin - vFrom;
		vDelta = vFrom - vTo;
		
		fDist = vDelta.lengthXY();
		fDistSquared = fDist * fDist;

		fRange = vHintDelta.z * fDist - vDelta.z * vHintDelta.lengthXY();

		if (fRange == 0)
		{
			return false;
		}
		fTemp4 = 1 / fRange;
		fRange = (fDist - vHintDelta.lengthXY()) / fRange;

		if (fRange <= 0)
		{
			return false;
		}

		fTemp = sqrt(fRange * (fDist * (fGravity * 0.5 * vHintDelta.lengthXY())));
		fTemp2 = (vHintDelta.z * fDistSquared - vDelta.z * vHintDelta.lengthXY(true)) * fGravity * 0.5;
		fTemp3 = fTemp / vHintDelta.lengthXY();

		pvVel->x = vHintDelta.x * fTemp3;
		pvVel->y = vHintDelta.y * fTemp3;
		pvVel->z = 0.5 * fTemp2 / fTemp * fTemp4;

		*peMode = AI_GREN_TOSS_HINT;

		return ValidGrenadePath(vFrom, pHint->origin, *pvVel);
	}

	vHintDelta = pHint->origin - vFrom;


	fRange = vHintDelta.lengthXY();
	fRangeSquared = fRange * fRange;

	if (fRangeSquared != 0)
	{
		fDist = vHintDelta.length();
		fHeight = fRangeSquared * fGravity / 589824.0 + vHintDelta.z;
		if (-fHeight <= -fDist)
		{
			fAngle = (atan2(vHintDelta.z / fRangeSquared, 1.0) + atan2(fHeight / fDist, sqrt(1.0 - fHeight / fDist * (fHeight / fDist)))) / 2;
			pvVel->x = vHintDelta.x * 768.0 / fRangeSquared * cos(fAngle);
			pvVel->y = vHintDelta.y * 768.0 / fRangeSquared * cos(fAngle);
			pvVel->z = 768.0 * cos(fAngle);
			return ValidGrenadePath(vFrom, pHint->origin, *pvVel);
		}
	}

	return bSuccess;
}

Vector Actor::GrenadeThrowPoint
	(
	const Vector& vFrom,
	const Vector& vDelta,
	const_str csAnim
	)

{
	vec2_t axis;
	Vector vRet;

	VectorCopy2D(vDelta, axis);
	VectorNormalize2D(axis);

	if (csAnim == STRING_ANIM_GRENADEKICK_SCR)
	{
		vRet = vFrom;
		vRet.z += 8;
		return vRet;
	}
	else
	{
		if (csAnim > STRING_ANIM_GRENADEKICK_SCR)
		{
			if (csAnim != STRING_ANIM_GRENADETHROW_SCR && csAnim != STRING_ANIM_GRENADETOSS_SCR)
			{
				vRet = vFrom;
				vRet.z += 80;
				return vRet;
			}
			else
			{
				vRet.x = vFrom.x - axis[0] * 34 + axis[1] * 8;
				vRet.y = vFrom.y - axis[0] * 8 - axis[1] * 34;
				vRet.z = 52 + vFrom.z;
				return vRet;
			}
		}
		else
		{
			if (csAnim != STRING_ANIM_GRENADERETURN_SCR)
			{
				vRet = vFrom;
				vRet.z += 80;
				return vRet;
			}
			else
			{

				vRet.x = vFrom.x + axis[0] * 25 + axis[1] * 2;
				vRet.y = vFrom.y - axis[0] * 2 + axis[1] * 25;
				vRet.z = 89 + vFrom.z;
				return vRet;
			}
		}
	}
}

Vector Actor::CalcKickVelocity
	(
	Vector& vDelta,
	float fDist
	) const

{
	float fScale;

	Vector ret;
	float v4;

	fScale = 0.57735032 * fDist;
	v4 = sqrt(0.8 * sv_gravity->value * 0.5 / (fScale - vDelta[2]));
	ret[0] = vDelta[0] * v4;
	ret[1] = vDelta[1] * v4;
	ret[2] = fScale * v4;

	return ret;
}

bool Actor::CanKickGrenade
	(
	Vector &vFrom,
	Vector &vTo,
	Vector &vFace,
	Vector *pvVel
	)

{
	Vector vEnd, vStart, vDelta, vVel;
	float fDist;

	if (sv_gravity->value <= 0.0)
	{
		return false;
	}

	vStart = GrenadeThrowPoint(vFrom, vFace, STRING_ANIM_GRENADEKICK_SCR);
	vDelta = vTo - vStart;

	if (vDelta.z >= 0 || Vector::Dot(vFace, vDelta) < 0.0 || vDelta.lengthXY() < 256 || vDelta.lengthXY() >= 255401.28 / (sv_gravity->value * 0.8) + 192.0)
	{
		return false;
	}

	if (vDelta.lengthXY() < 512)
		fDist = 192.0 / vDelta.lengthXY() + 0.25;
	else
		fDist = 1.0 - 192.0 / vDelta.lengthXY();

	float vdlxy = vDelta.lengthXY();

	vDelta[0] *= fDist;
	vDelta[1] *= fDist;

	vEnd = vDelta + vStart;

	vVel = CalcKickVelocity(fDist * vDelta, fDist * vdlxy);


	if (vVel == vec_zero || !ValidGrenadePath(vStart, vEnd, vVel))
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool Actor::GrenadeWillHurtTeamAt
	(
	Vector& vTo
	)

{
	if (m_pNextSquadMate == this)
	{
		return false;
	}

	for (Actor * pSquadMate = (Actor *)m_pNextSquadMate.Pointer(); pSquadMate != this; pSquadMate = (Actor *)pSquadMate->m_pNextSquadMate.Pointer())
	{
		//FIXME: macro
		if ((pSquadMate->origin-vTo).length() <= 65536)
		{
			return true;
		}
	}
	return false;
}

bool Actor::CanGetGrenadeFromAToB
	(
	Vector& vFrom,
	Vector& vTo,
	bool bDesperate,
	Vector *pvVel,
	eGrenadeTossMode *peMode
	)

{
	float fDot;
	Vector vHint;
	Vector vStart;
	Vector vAxisY;
	Vector vAxisX;
	Vector vDelta;
	Vector vDelta2 = vec_zero;
	int nHints;
	GrenadeHint *apHint[4];
	float fRangeSquared;

	if (sv_gravity->value <= 0)
		return false;

	vDelta = vTo - vFrom;
	fRangeSquared = vDelta.lengthSquared();

	if (fRangeSquared < 65536)
		return false;
	if (bDesperate)
	{
		vStart = GrenadeThrowPoint(vFrom, vDelta, STRING_ANIM_GRENADERETURN_SCR);
	}

	if (fRangeSquared < 1048576)
	{
		if (!bDesperate)
		{
			vStart = GrenadeThrowPoint(vFrom, vDelta, STRING_ANIM_GRENADETOSS_SCR);
		}

		*pvVel = CanRollGrenade(vStart, vTo);

		if (*pvVel == vec_zero)
		{
			*peMode = AI_GREN_TOSS_ROLL;
			return true;
		}

		if (!bDesperate)
		{
			vStart = GrenadeThrowPoint(vFrom, vDelta, STRING_ANIM_GRENADETHROW_SCR);
		}

		*pvVel = CanThrowGrenade(vStart, vTo);

		if (*pvVel == vec_zero)
		{
			*peMode = AI_GREN_TOSS_THROW;
			return true;
		}
	}

	if (!bDesperate)
	{
		return false;
	}

	nHints = GrenadeHint::GetClosestSet(apHint, 4, vStart, 1048576);

	for (int i = 0; i <= nHints; i++)
	{
		if (bDesperate)
		{
			vStart = GrenadeThrowPoint(vFrom, vDelta2, STRING_ANIM_GRENADERETURN_SCR);
			if (CanTossGrenadeThroughHint(apHint[i], vStart, vTo, bDesperate, pvVel, peMode))
				return true;
			continue;
		}

		vHint = apHint[i]->origin;
		vDelta2 = vHint - vFrom;

		fDot = DotProduct2D(vDelta2, vDelta);
		if (fDot >= 0 && fDot * fDot <= vDelta.lengthXY(true) * vDelta2.lengthXY(true) * 0.89999998)
		{
			vStart = GrenadeThrowPoint(vFrom, vDelta2, STRING_ANIM_GRENADETOSS_SCR);
			if (CanTossGrenadeThroughHint(apHint[i], vStart, vTo, bDesperate, pvVel, peMode))
				return true;
		}

		
	}

	*peMode = AI_GREN_TOSS_NONE;
	return false;
}

bool Actor::DecideToThrowGrenade
	(
	Vector& vTo,
	Vector *pvVel,
	eGrenadeTossMode *peMode
	)

{
	if (Sentient::AmmoCount("grenade"))
	{
		if (!GrenadeWillHurtTeamAt(vTo))
		{
			return CanGetGrenadeFromAToB(origin, vTo, false, pvVel, peMode);
		}
	}
	return false;
}

void Actor::Grenade_EventFire
	(
	Event *ev
	)

{
	const_str csAnim;
	str strGrenade;
	float speed;
	Vector pos, dir, vStart;

	gi.Tag_NumForName(edict->tiki, "tag_weapon_right");
	
	if (m_eGrenadeMode == AI_GREN_TOSS_ROLL)
	{
		csAnim = STRING_ANIM_GRENADETHROW_SCR;
	}
	else
	{
		csAnim = STRING_ANIM_GRENADETOSS_SCR;
	}

	vStart = GrenadeThrowPoint(origin, orientation[0], csAnim);

	dir = m_vGrenadeVel;

	speed = VectorNormalize(dir);

	if (m_Team == TEAM_AMERICAN)
	{
		strGrenade = "models/projectiles/M2FGrenade.tik";
	}
	else
	{
		strGrenade = "models/projectiles/steilhandgranate.tik";
	}

	ProjectileAttack(vStart, dir, this, strGrenade, 0, speed);

	UseAmmo("grenade", 1);

}

void Actor::GenericGrenadeTossThink
	(
	void
	)

{
	Vector vGrenadeVel = vec_zero;
	eGrenadeTossMode eGrenadeMode;

	if (m_Enemy && level.inttime >= m_iStateTime + 200)
	{
		if (Actor::CanGetGrenadeFromAToB(
			origin,
			m_Enemy->velocity-m_Enemy->origin,
			false,
			&vGrenadeVel,
			&eGrenadeMode))
		{
			m_vGrenadeVel = vGrenadeVel;
			m_eGrenadeMode = eGrenadeMode;
		}
		m_iStateTime = level.inttime;
	}

	m_YawAchieved = false;
	m_DesiredYaw = vectoyaw(m_vGrenadeVel);
	ContinueAnimation();
}

void Actor::CanSee
	(
	Event *ev
	)

{
	float fov = 0;
	float vision_distance = 0;
	if (m_Think[m_ThinkLevel] != 17 || !m_pTurret)
	{
		return Entity::CanSee(ev);
	}
	switch (ev->NumArgs())
	{
		case 1:
		{
			//do nothing
		}
		break;
		case 2:
		{
			fov = ev->GetFloat(2);
			if (fov < 0 || fov> 360)
			{
				ScriptError("fov must be in range 0 <= fov <= 360");
			}
		}
		break;
		case 3:
		{
			vision_distance = ev->GetFloat(3);
			if (vision_distance < 0)
			{
				ScriptError("vision distance must be >= 0");
			}
		}
			break;
		default:
		{
			ScriptError("cansee should have 1, 2, or 3 arguments");
		}
		break;
	}


	Entity *ent = ev->GetEntity(1);
	if (!ent)
	{
		ScriptError("NULL entity in parameter 1");
	}
	else
	{
		int cansee = MachineGunner_CanSee(ent, fov, vision_distance);
		ev->AddInteger(cansee);
	}
}

void Actor::Think
	(
	void
	)

{
	int v1, v2, v3, tempHistory,ohIndex;
	m_bAnimating = 0; 
	if (g_ai->integer
		&& m_bDoAI
		&& edict->tiki)
	{
		Director.iPaused++;
		//FIXME: name history calculation variables.
		v1 = level.inttime;
		v2 = v1 / 125;
		v3 = v1 / 125;
		if (v1 / 125 < 0)
			v3 = v2 + 3;
		tempHistory = v2 - (v3 & 0xFFFFFFFC);

		if (m_iCurrentHistory != tempHistory)
		{
			m_iCurrentHistory = tempHistory;
			ohIndex = tempHistory - 1;
			if (ohIndex < 0)
			{
				ohIndex = 3;
			}
			m_vOriginHistory[ohIndex][0] = origin[0];
			m_vOriginHistory[ohIndex][1] = origin[1];
		}
		if (m_bNoPlayerCollision)
		{
			Entity *ent = G_GetEntity(0);
			if (!IsTouching(ent))
			{
				Com_Printf("(entnum %d, radnum %d) is going solid after not getting stuck in the player\n", entnum, radnum);
				setSolidType(SOLID_BBOX);
				m_bNoPlayerCollision = 0;
			}
		}
		m_eNextAnimMode = -1;
		FixAIParameters();
		if (m_bEnableEnemy != m_bDesiredEnableEnemy)
		{
			m_bEnableEnemy = m_bDesiredEnableEnemy;
			if (m_bDesiredEnableEnemy)
			{
				if (!m_bFixedLeash)
				{
					m_vHome = origin;
				}
			}
			else
			{
				//FIXME: possible macros/enums
				if ((m_ThinkStates[0] - 4) <= 2)
				{
					SetThinkState(THINKSTATE_IDLE, 0);
				}
				SetEnemy(0, 0);
			}
		}

		if (m_pTetherEnt)
		{
			m_vHome = m_pTetherEnt->origin;
		}

		if (m_bBecomeRunner)
		{
			parm.movefail = false;
			if (m_ThinkMap[1] != 4 && m_ThinkMap[1] != 3)
			{
				parm.movefail = true;
			}
		}

		if (m_bDirtyThinkState)
		{
			m_bDirtyThinkState = false;
			ThinkStateTransitions();
		}

		GlobalFuncs_t *func = &GlobalFuncs[m_Think[m_ThinkLevel]];

		if (func->ThinkState)
			(this->*func->ThinkState)();

		m_bNeedReload = false;
		mbBreakSpecialAttack = false;
		if (!--Director.iPaused)
			Director.ExecuteRunning();
	}
}

void Actor::PostThink
	(
	bool bDontFaceWall
	)

{
	CheckUnregister();
	if (bDontFaceWall)
	{
		if (!m_pTurnEntity || m_ThinkState != 1)
		{
			DontFaceWall();
		}
	}
	UpdateAngles();
	UpdateAnim();
	DoMove();
	UpdateBoneControllers();
	UpdateFootsteps();
}

void Actor::SetMoveInfo
	(
	mmove_t *mm
	)

{
	memset(mm, 0, sizeof(mmove_t));

	mm->velocity[0] = velocity.x;
	mm->velocity[1] = velocity.y;
	mm->velocity[2] = velocity.z;

	mm->origin[0] = origin.x;
	mm->origin[1] = origin.y;
	mm->origin[2] = origin.z;

	mm->entityNum = entnum;

	mm->walking = m_walking;

	mm->groundPlane = m_groundPlane;
	mm->groundPlaneNormal[0] = m_groundPlaneNormal[0];
	mm->groundPlaneNormal[1] = m_groundPlaneNormal[1];
	mm->groundPlaneNormal[2] = m_groundPlaneNormal[2];

	mm->frametime = level.frametime;

	mm->mins[0] = MINS.x;
	mm->mins[1] = MINS.y;
	mm->mins[2] = MINS.z;
	mm->maxs[0] = MAXS.x;
	mm->maxs[1] = MAXS.y;
	mm->maxs[2] = MAXS.z;

	mm->tracemask = m_bNoPlayerCollision == false ? MASK_PATHSOLID : MASK_TARGETPATH;
}

void Actor::GetMoveInfo
	(
	mmove_t *mm
	)

{
	//FIXME: macros
	m_walking = mm->walking;
	m_groundPlane = mm->groundPlane;
	m_groundPlaneNormal = mm->groundPlaneNormal;
	groundentity = NULL;

	TouchStuff(mm);

	if (m_eAnimMode >= 2)
	{
		if (m_eAnimMode <= 3)
		{
			if (mm->hit_temp_obstacle)
			{
				if (mm->hit_temp_obstacle & 1)
				{
					m_Path.Clear();
					Player *p = (Player *)G_GetEntity(0);

					if (!IsTeamMate(p))
					{
						if (!m_bEnableEnemy)
						{
							m_bDesiredEnableEnemy = true;
							m_bEnableEnemy = true;
							if (!m_bFixedLeash)
							{
								m_vHome = origin;
							}
						}

						BecomeTurretGuy();

						m_PotentialEnemies.ConfirmEnemy(this, p);
						m_bForceAttackPlayer = true;
					}
				}
				velocity = vec_zero;
				return;
			}
			if (mm->hit_obstacle)
			{
				if (level.inttime >= m_Path.Time() + 1000)
				{
					m_Path.ReFindPath(origin, this);
				}
				else
				{
					if (!m_Path.NextNode() || 
							(!(m_Path.NextNode()->bAccurate && m_Path.IsSide())
							&& m_Path.NextNode() == m_Path.CurrentNode()
							&& m_Path.CurrentNode() != m_Path.StartNode())
						)
					{
						m_Path.Clear();
						velocity = vec_zero;
						return;
					}

					if (m_Path.CurrentNode() == m_Path.StartNode())
					{
						Com_Printf(
							"^~^~^ BAD FAILSAFE: %.2f %.2f %.2f, '%s'\n",
							origin[0],
							origin[1],
							origin[2],
							targetname.c_str());
					}

					DoFailSafeMove(m_Path.NextNode()->point);
				}
			}
			else if (velocity.lengthXY(true) < -0.69999999 && level.inttime >= m_Path.Time() + 1000)
			{
				m_Path.ReFindPath(origin, this);
			}
		}
		else if (m_eAnimMode == 4)
		{
			if (mm->hit_temp_obstacle)
			{
				if (mm->hit_temp_obstacle & 1)
				{
					m_Path.Clear();
					Player *p = (Player *)G_GetEntity(0);

					if (!IsTeamMate(p))
					{
						if (!m_bEnableEnemy)
						{
							m_bDesiredEnableEnemy = true;
							m_bEnableEnemy = true;
							if (!m_bFixedLeash)
							{
								m_vHome = origin;
							}
						}

						BecomeTurretGuy();

						m_PotentialEnemies.ConfirmEnemy(this, p);
						m_bForceAttackPlayer = true;
					}
				}
			}
			else if (mm->hit_obstacle)
			{
				Vector end = m_Dest;
				end.z -= 16384.0;
				trace_t trace = G_Trace(
					m_Dest,
					PLAYER_BASE_MIN,
					PLAYER_BASE_MAX,
					end,
					NULL,
					1,
					qfalse,
					"Actor::GetMoveInfo");

				DoFailSafeMove(trace.endpos);
			}
		}
	}

	setOrigin(mm->origin);

	if (VectorLengthSquared(mm->velocity) < 1)
	{
		velocity = vec_zero;
		return;
	}

	velocity = mm->velocity;
}

void Actor::DoFailSafeMove
	(
	vec3_t dest
	)

{
	Com_Printf("(entnum %d, radnum %d) blocked, doing failsafe\n", entnum, radnum);

	VectorCopy(dest, m_NoClipDest);

	//FIXME: macros
	SetThinkState(8, 3);
}

void Actor::TouchStuff
	(
	mmove_t *mm
	)

{
	int i, j;
	gentity_t  *other;
	Event		*event;

	if (getMoveType() != MOVETYPE_NOCLIP)
	{
		G_TouchTriggers( this );
	}

	for (i = 0; i < mm->numtouch; i++)
	{
		other = &g_entities[pm->touchents[i]];

		for (j = 0; j < i; j++)
		{
			gentity_t *ge = &g_entities[j];

			if (ge == other)
				break;
		}

		if (j != i)
		{
			// duplicated
			continue;
		}

		// Don't bother touching the world
		if ((!other->entity) || (other->entity == world))
		{
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

void Actor::ExtractConstraints
	(
	mmove_t *mm
	)

{
	// not found in ida
}

void Actor::EventGiveWeaponInternal
	(
	Event *ev
	)

{
	Holster();
	RemoveWeapons();
	str weapName = ev->GetString(1);
	if (giveItem(weapName))
	{
		Unholster();
	}
}

void Actor::EventGiveWeapon
	(
	Event *ev
	)

{
	Event e1(EV_Listener_ExecuteScript);
	str weapName = ev->GetString(1);
	weapName.tolower();
	const_str csWeapName = Director.AddString(weapName);

	m_csLoadOut = csWeapName;

	if (csWeapName == STRING_MG42)
	{
		csWeapName = STRING_MP40;
	}

	m_csWeapon = csWeapName;

	setModel();

	e1.AddConstString(STRING_GLOBAL_WEAPON_SCR);
	e1.AddString(weapName);

	ExecuteScript(&e1);

}

void Actor::EventGetWeapon
	(
	Event *ev
	)

{
	ev->AddConstString(m_csWeapon);
}

void Actor::FireWeapon
	(
	Event *ev
	)

{
	Sentient::FireWeapon(0, FIRE_PRIMARY);
}

bool Actor::CanTarget
	(
	void
	)

{
	return true;
}

bool Actor::IsImmortal
	(
	void
	)

{
	return false;
}

bool Actor::IsVoidState
	(
	int state
	)

{
	return true;
}

bool Actor::IsIdleState
	(
	int state
	)

{
	return state == THINKSTATE_IDLE;
}

bool Actor::IsCuriousState
	(
	int state
	)

{
	return state == THINKSTATE_CURIOUS;
}

bool Actor::IsDisguiseState
	(
	int state
	)

{
	return state == THINKSTATE_DISGUISE;
}

bool Actor::IsAttackState
	(
	int state
	)

{
	return state == THINKSTATE_ATTACK;
}

bool Actor::IsGrenadeState
	(
	int state
	)

{
	return state == THINKSTATE_GRENADE;
}

bool Actor::IsPainState
	(
	int state
	)

{
	return state == THINKSTATE_PAIN;
}

bool Actor::IsKilledState
	(
	int state
	)

{
	return state == THINKSTATE_KILLED;
}

bool Actor::IsMachineGunnerState
	(
	int state
	)

{
	return true;
}

bool Actor::IsDogState
	(
	int state
	)

{
	return true;
}

void Actor::IgnoreSoundSet
	(
	int iType
	)

{
	m_iIgnoreSoundsMask |= 1 << iType;
}

void Actor::IgnoreSoundSetAll
	(
	void
	)

{
	m_iIgnoreSoundsMask = 65535;
}

void Actor::IgnoreSoundClear
	(
	int iType
	)

{
	m_iIgnoreSoundsMask &= ~iType;
}

void Actor::IgnoreSoundClearAll
	(
	void
	)

{
	m_iIgnoreSoundsMask = 0;
}

bool Actor::IgnoreSound
	(
	int iType
	)

{
	return ( m_iIgnoreSoundsMask >> iType ) & 1;
}

void Actor::EventShareEnemy
	(
	Event *ev
	)

{

	if (m_Enemy)
	{
		if (! ((m_bEnemyIsDisguised || m_Enemy->m_bIsDisguised) && !m_bForceAttackPlayer && m_ThinkState != 4) )
		{
			for (Actor * pSquadMate = (Actor *)m_pNextSquadMate.Pointer(); pSquadMate != this; pSquadMate = (Actor *)pSquadMate->m_pNextSquadMate.Pointer())
			{
				if (pSquadMate->IsSubclassOfActor())
				{
					Vector dist = pSquadMate->origin - origin;
					float distSq = dist * dist;
					if (pSquadMate->m_fMaxShareDistSquared == 0.0 || distSq <= pSquadMate->m_fMaxShareDistSquared)
					{
						pSquadMate->m_PotentialEnemies.ConfirmEnemyIfCanSeeSharerOrEnemy(pSquadMate, this, m_Enemy);
					}
				}
			}
		}
	}
}

void Actor::EventShareGrenade
	(
		Event *ev
	)

{
	
	if (m_pGrenade)
	{
		for (Actor * pSquadMate = (Actor *)m_pNextSquadMate.Pointer(); pSquadMate != this; pSquadMate = (Actor *)pSquadMate->m_pNextSquadMate.Pointer())
		{
			if (pSquadMate->IsSubclassOfActor())
			{
				if (!pSquadMate->m_pGrenade)
				{
					Vector dist = pSquadMate->origin - origin;
					float distSq = dist * dist;
					//FIXME: where did that constant come from?
					if (distSq < 589824)
					{
						//FIXME: where did that constant come from?
						if (DoesTheoreticPathExist(pSquadMate->origin, 1536))
						{
							pSquadMate->SetGrenade(m_pGrenade);
						}
					}
				}
			}
		}
	}
}

void Actor::ReceiveAIEvent
	(
	vec3_t event_origin,
	int iType,
	Entity *originator,
	float fDistSquared,
	float fMaxDistSquared
	)

{
	if (originator != this && originator != GetActiveWeapon(WEAPON_MAIN))
	{

		GlobalFuncs_t *func = &GlobalFuncs[m_Think[m_ThinkLevel]];

		if (func->ReceiveAIEvent)
			(this->*func->ReceiveAIEvent)(event_origin, iType, originator, fDistSquared, fMaxDistSquared);
	}
}

void Actor::DefaultReceiveAIEvent
	(
	vec3_t event_origin,
	int iType,
	Entity *originator,
	float fDistSquared,
	float fMaxDistSquared
	)

{
	if (!IsDead())
	{
		if (!originator->IsDead() && originator->IsSubclassOfSentient() && ((Sentient *)originator)->m_Team == m_Team && !IsSquadMate((Sentient *)originator))
		{
			MergeWithSquad((Sentient *)originator);
		}
		switch (iType)
		{
		case AI_EVENT_WEAPON_FIRE:
		case AI_EVENT_WEAPON_IMPACT:
			if (m_fHearing * m_fHearing > fDistSquared)
				WeaponSound(iType, event_origin, fDistSquared, fMaxDistSquared, originator);
			break;
		case AI_EVENT_EXPLOSION:
		case AI_EVENT_MISC:
		case AI_EVENT_MISC_LOUD:
			if (m_fHearing * m_fHearing > fDistSquared)
				CuriousSound(iType, event_origin, fDistSquared, fMaxDistSquared);
			break;
		case AI_EVENT_AMERICAN_VOICE:
		case AI_EVENT_GERMAN_VOICE:
		case AI_EVENT_AMERICAN_URGENT:
		case AI_EVENT_GERMAN_URGENT:
			if (m_fHearing * m_fHearing > fDistSquared)
				VoiceSound(iType, event_origin, fDistSquared, fMaxDistSquared, originator);
			break;
		case AI_EVENT_FOOTSTEP:
			if (m_fHearing * m_fHearing > fDistSquared)
				FootstepSound(event_origin, fDistSquared, fMaxDistSquared, originator);
			break;
		case AI_EVENT_GRENADE:
			Actor::GrenadeNotification(originator);
			break;
		default:
			{
				assert(!DumpCallTrace("iType = %i", iType));
			}
			break;
		}
	}
}

int Actor::PriorityForEventType
	(
	int iType
	)

{
	switch( iType )
	{

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
		assert( !"PriorityForEventType: unknown ai_event type" );
		return 0;
	}
}

void Actor::CuriousSound
	(
	int iType,
	vec3_t sound_origin,
	float fDistSquared,
	float fMaxDistSquared
	)

{
	float v7, v8, fRangeFactor = 1.0;
	int iPriority;
	if (m_bEnableEnemy)
	{
		if (m_ThinkStates[0] == 1 || m_ThinkStates[0] == 5)
		{
			//FIXME: name variables.
			if (fMaxDistSquared != 0.0)
			{
				v7 = 1*(1.0/3) - fDistSquared * (1*(1.0 / 3)) / fMaxDistSquared;
				if (v7 <= 1.0)
					fRangeFactor = v7;
			}
			v8 = fRangeFactor * m_fSoundAwareness;
			//FIXME: macro
			if (v8 >= rand() * 0.000000046566129)
			{
				RaiseAlertnessForEventType(iType);
				iPriority = PriorityForEventType(iType);
				if (iPriority >= m_iCuriousLevel)
				{
					m_iCuriousLevel = iPriority;
					if (iType == AI_EVENT_WEAPON_IMPACT)
					{
						//FIXME: macro
						if (fDistSquared <= 36864.0)
							SetCuriousAnimHint(1);
					}
					else if (iType > AI_EVENT_WEAPON_IMPACT)
					{
						if (iType == AI_EVENT_EXPLOSION)
						{
							//FIXME: macro
							if (fDistSquared <= 589824.0)
								SetCuriousAnimHint(3);
						}
						else
						{
							SetCuriousAnimHint(5);
						}
					}
					else if (iType == AI_EVENT_WEAPON_FIRE)
					{
						//FIXME: macro
						if (fDistSquared <= 262144.0)
							SetCuriousAnimHint(2);
					}
					else
					{
						SetCuriousAnimHint(5);
					}

					SetEnemyPos(sound_origin);
					
					SetThinkState(THINKSTATE_NONE, m_ThinkLevel);
					SetThinkState(THINKSTATE_CURIOUS, 0);

					m_pszDebugState = (char *)G_AIEventStringFromType(iType);
				}
			}
		}
	}
}

void Actor::WeaponSound
	(
	int iType,
	vec3_t sound_origin,
	float fDistSquared,
	float fMaxDistSquared,
	Entity *originator
	)

{
	Sentient *pOwner;
	if (originator->IsSubclassOfWeapon())
	{
		pOwner = ((Weapon *)originator)->GetOwner();
	}
	else
	{
		if (!originator->IsSubclassOfProjectile())
		{
			/*
			 * useless assert
			 *if ( !dword_39A8FC )
				{
					strcpy(v31, "\"Actor::WeaponSound: non-weapon made a weapon sound.\\n\"\n\tMessage: ");
					memset(&s, 0, 0x3FBDu);
					v7 = Class::getClassname(&originator->baseItem.baseTrigger.baseAnimate.baseEntity.baseSimple.baseListener.baseClass);
					v8 = (*(this->baseSimpleActor.baseSentient.baseAnimate.baseEntity.baseSimple.baseListener.baseClass.vftable + 87))(
						   this,
						   "class = %s",
						   v7);
					Q_strcat(v31, 0x4000, v8);
					v9 = MyAssertHandler(v31, "fgame/actor.cpp", 9553, 0);
					if ( v9 < 0 )
					{
					  dword_39A8FC = 1;
					}
					else if ( v9 > 0 )
					{
					  __debugbreak();
					}
				}
			*/
			return;
		}
		pOwner = ((Projectile *)originator)->GetOwner();
	}

	if (!pOwner)
	{
		return;
	}

	if (pOwner->m_Team == m_Team)
	{
		if (!pOwner->m_Enemy)
		{
			if (pOwner->IsSubclassOfActor())
			{
				
			}
		}
	}
	else
	{
		
	}
	// FIXME: stub
	STUB();
}

void Actor::FootstepSound
	(
	vec3_t sound_origin,
	float fDistSquared,
	float fMaxDistSquared,
	Entity *originator
	)

{
	if (originator->IsSubclassOfSentient())
	{
		if ( (m_ThinkStates[0] == THINKSTATE_IDLE || m_ThinkStates[0] == THINKSTATE_CURIOUS ) && m_bEnableEnemy)
		{
			if (m_Team == ((Sentient *)originator)->m_Team || ((Sentient *)originator)->m_bIsDisguised)
			{
				return;
			}
			if (originator == m_Enemy)
			{
				if (EnemyInFOV(0) && CanSeeEnemy(0))
				{
					return;
				}
			}
			else
			{
				if (InFOV(originator->centroid, m_fFov, m_fFovDot) && CanSeeFrom(EyePosition(), originator))
				{
					return;
				}
			}
			CuriousSound(AI_EVENT_FOOTSTEP, sound_origin, fDistSquared, fMaxDistSquared);
		}
	}
	else
	{
		/*
		 * useless assert
			strcpy(v12, "\"'ai_event footstep' in a tiki used by something besides AI or player.\\n\"\n\tMessage: ");
			memset(&s, 0, 0x3FABu);
			v5 = (*(this->baseSimpleActor.baseSentient.baseAnimate.baseEntity.baseSimple.baseListener.baseClass.vftable + 87))(
					this,
					&nullStr);
			Q_strcat(v12, 0x4000, v5);
			v6 = MyAssertHandler(v12, "fgame/actor.cpp", 9618, 0);
			if ( v6 < 0 )
			{
				dword_39A900 = 1;
			}
			else if ( v6 > 0 )
			{
				__debugbreak();
			}
		 */
	}
}

void Actor::VoiceSound
	(
	int iType,
	vec3_t sound_origin,
	float fDistSquared,
	float fMaxDistSquared,
	Entity *originator
	)

{
	if (iType != 4 && iType != 5 && iType != 6 && iType != 4)
	{
	}
	// FIXME: stub
	STUB();
}

void Actor::GrenadeNotification
	(
	Entity *originator
	)

{
	bool bCanSee = false, bInFOV = false;
	if (!m_pGrenade)
	{
		if (originator == m_Enemy)
		{
			bInFOV = EnemyInFOV(0);
			bCanSee = CanSeeEnemy(0);
		}
		else
		{
			bInFOV = InFOV(originator->centroid, m_fFov, m_fFovDot);
			if (bInFOV)
			{
				if (gi.AreasConnected(edict->r.areanum, originator->edict->r.areanum))
				{
					bCanSee = CanSeeFrom(EyePosition(), originator);
				}
			}

		}
		if (bCanSee && bInFOV)
		{
			
			
			assert(originator->IsSubclassOfProjectile());

			SetGrenade(originator);
			Event e1(EV_Actor_ShareGrenade);
			PostEvent(e1, 0.5);

			Anim_Say(STRING_ANIM_SAY_GRENADE_SIGHTED_SCR, 0, true);
		}
	}

	if (m_pGrenade == originator)
	{

		float fGrav, fDeltaZ, fVelDivGrav, fTimeLand;
		vec2_t vLand;
		vec2_t vMove;

		fGrav = sv_gravity->value * m_pGrenade->gravity;

		fDeltaZ = originator->origin[2] - origin[2];

		fVelDivGrav = originator->velocity[2] / fGrav;

		//from s=v*t + 0.5*a*t^2, apply for z velocity, solve.
		fTimeLand = fVelDivGrav + sqrt((fDeltaZ + fDeltaZ) / fGrav + fVelDivGrav * fVelDivGrav);

		vLand[0] = fTimeLand * originator->velocity[0] + originator->origin[0];
		vLand[1] = fTimeLand * originator->velocity[1] + originator->origin[1];

		vMove[0] = m_vGrenadePos[0] - vLand[0];
		vMove[1] = m_vGrenadePos[1] - vLand[1];
		
		if (VectorLength2D(vMove) > 16.0)
		{
			m_vGrenadePos[0] = vLand[0];
			m_vGrenadePos[1] = vLand[1];
			m_vGrenadePos[2] = origin[2];
			m_bGrenadeBounced = true;
		}
		
	}
}

void Actor::SetGrenade
	(
	Entity *pGrenade
	)

{
	if (m_pGrenade != pGrenade)
	{
		m_pGrenade = pGrenade;
	}
	
	m_bGrenadeBounced = true;
	
	m_iFirstGrenadeTime = level.inttime;

	m_vGrenadePos = pGrenade->origin;

}

void Actor::NotifySquadmateKilled
	(
	Sentient *pSquadMate,
	Sentient *pAttacker
	)

{
	if (m_PotentialEnemies.CaresAboutPerfectInfo(pAttacker))
	{

		Vector dist = pSquadMate->origin - origin;
		float distSq = dist * dist;
		if (distSq < m_fSight*m_fSight)
		{
			bool shouldConfirm = false;
			if (gi.AreasConnected(edict->r.areanum,pSquadMate->edict->r.areanum))
			{
				shouldConfirm = G_SightTrace(
					EyePosition(),
					vec_zero,
					vec_zero,
					pSquadMate->EyePosition(),
					this,
					pSquadMate,
					MASK_TRANSITION,
					qfalse,
					"Actor::NotifySquadmateKilled");
			}
			//FIXME: macro for that constant
			if (!shouldConfirm)
			{
				if (distSq <= 589824)
				{
					Vector start, end;
					if (origin.z <= pSquadMate->origin.z)
					{
						start = origin;
						end = pSquadMate->origin;
					}
					else
					{
						start = pSquadMate->origin;
						end = origin;
					}
					shouldConfirm = m_Path.DoesTheoreticPathExist(start, end, this, 0x44C00000, 0, 0);
				}
			}
			if (shouldConfirm)
			{
				m_PotentialEnemies.ConfirmEnemy(this, pAttacker);
			}
		}
	}
}

void Actor::RaiseAlertnessForEventType
	(
	int iType
	)

{
	//FIXME: inunderstandable variables
	long double v2,v3,v4,v8;
	switch (iType)
	{
	case AI_EVENT_WEAPON_FIRE:
		v2 = 0.2;
		goto ACTOREVENTLABEL_3;
	case AI_EVENT_WEAPON_IMPACT:
		v2 = 0.1;
		goto ACTOREVENTLABEL_3;
	case AI_EVENT_EXPLOSION:
		v2 = 0.40000001;
		goto ACTOREVENTLABEL_3;
	case AI_EVENT_AMERICAN_VOICE:
	case AI_EVENT_AMERICAN_URGENT:
		if (m_Team)
			return;
		v2 = 0.25;
		goto ACTOREVENTLABEL_3;
	case AI_EVENT_MISC:
		v2 = 0.02;
		goto ACTOREVENTLABEL_3;
	case AI_EVENT_MISC_LOUD:
	case AI_EVENT_FOOTSTEP:
		v2 = 0.050000001;
		goto ACTOREVENTLABEL_3;
	case AI_EVENT_GRENADE:
		v2 = 0.039999999;
	ACTOREVENTLABEL_3:
		v8 = m_fNoticeTimeScale;
		v3 = v2;
		v4 = v8 * 0.66666669;
		if (v3 <= v4)
			v4 = v3;
		m_fNoticeTimeScale = v8 - v4;
		break;
	case AI_EVENT_GERMAN_VOICE:
	case AI_EVENT_GERMAN_URGENT:
		if (m_Team == TEAM_AMERICAN)
		{
			RaiseAlertness(0.25);
		}
	default:
		assert(0);
		/* useless assert
		if (!dword_39A90C)
		{
			strcpy(&v9, "\"Actor::RaiseAlertnessForEventType: unknown event type\\n\"\n\tMessage: ");
			memset(&s, 0, 0x3FBBu);
			v5 = (*(this->baseSimpleActor.baseSentient.baseAnimate.baseEntity.baseSimple.baseListener.baseClass.vftable + 87))(
				this,
				&s2);
			Q_strcat(&v9, 0x4000, v5);
			v6 = MyAssertHandler(&v9, "fgame/actor.cpp", 9864, 0);
			if (v6 < 0)
			{
				dword_39A90C = 1;
			}
			else if (v6 > 0)
			{
				__debugbreak();
			}
		}*/
		break;
	}
}

void Actor::RaiseAlertness
	(
	float fAmount
	)

{
	//FIXME: inunderstandable variable
	float v2; // fst7

	v2 = m_fNoticeTimeScale * 0.66666669;
	if (fAmount <= v2)
		v2 = fAmount;
	m_fNoticeTimeScale -= v2;
}

bool Actor::CanSee
	(
	Entity *e1,
	float fov,
	float vision_distance
	)

{
	bool canSee = Sentient::CanSee(e1, fov, vision_distance);
	if (e1 == m_Enemy)
	{
		if (canSee)
		{
			SetEnemyPos(e1->origin);
			m_bEnemyIsDisguised = m_Enemy->m_bIsDisguised;
			m_iEnemyCheckTime = level.inttime;
			m_iLastEnemyVisibleTime = level.inttime;
		}
		//FIXME: possible macro
		if (canSee != m_bEnemyVisible)
		{
			m_bEnemyVisible = true;
			m_iEnemyVisibleChangeTime = level.inttime;
		}

		if (fov != 0)
		{
			//FIXME: possible macro
			m_iEnemyFovCheckTime = level.inttime;
			if (canSee != m_bEnemyInFOV)
			{
				m_bEnemyInFOV = true;
				m_iEnemyFovChangeTime = level.inttime;
			}
		}
	}
	return canSee;
}

Vector Actor::GunPosition
	(
	void
	)

{
	if (m_iGunPositionCheckTime < level.inttime)
	{
		m_iGunPositionCheckTime = level.inttime;
		Weapon *weap = GetActiveWeapon(WEAPON_MAIN);
		if (weap)
		{
			weap->GetMuzzlePosition(&m_vGunPosition);
		}
		else
		{
			m_vGunPosition = vec_zero;
		}
	}
	return m_vGunPosition;
}

bool Actor::WithinVisionDistance
	(
	Entity *ent
	) const

{
	float fRadius = world->m_fAIVisionDistance;

	if( world->m_fAIVisionDistance == 0.0f )
	{
		fRadius = m_fSight;
	}
	else if( m_fSight <= fRadius )
	{
		fRadius = m_fSight;
	}

	if( ent )
	{
		Vector vDelta = ent->origin - origin;
		return vDelta.lengthSquared() < fRadius;
	}

	return false;
}

bool Actor::InFOV
	(
	Vector pos,
	float check_fov,
	float check_fovdot
	)

{

	bool bInFov = true;
	if (check_fov != 360.0)
	{
		Vector delta = pos - EyePosition();
		if (delta.x == 0 && delta.y == 0)
		{
			return bInFov;
		}
		bInFov = false;
		Vector a;
		a[0] = delta.x * orientation[0][0];
		a[1] = delta.y * orientation[0][1] + a[0];
		if (a[0] >= 0)
		{
			bInFov = a[0] * a[0] > delta.lengthXY() * (check_fovdot * check_fovdot);
		}
		
	}
	return bInFov;
}

bool Actor::EnemyInFOV
	(
	int iMaxDirtyTime
	)

{
	bool inFov;
	if (level.inttime > iMaxDirtyTime + m_iEnemyFovCheckTime)
	{
		inFov = m_bEnemyInFOV;
		if (InFOV(m_Enemy->origin, m_fFov, m_fFovDot) != inFov)
		{
			inFov = true;
			m_bEnemyInFOV = inFov;
			m_iEnemyFovChangeTime = level.inttime;
		}
		m_iEnemyFovCheckTime = level.inttime;

	}
	else
	{
		inFov = m_bEnemyInFOV;
	}
	return inFov;
}

bool Actor::InFOV
	(
	Vector pos
	)

{
	//Proposed. Not sure if there are other checks.
	return InFOV(pos, m_fFov, m_fFovDot);
}

bool Actor::InFOV
	(
	Entity *ent
	)

{
	//Proposed. Not sure if there are other checks.
	return InFOV(ent->origin, m_fFov, m_fFovDot);
}

bool Actor::CanSeeNoFOV
	(
	Entity *ent
	)

{
	// not found in ida
	return false;
}

bool Actor::CanSeeFOV
	(
	Entity *ent
	)

{
	// not found in ida
	return false;
}

bool Actor::CanSeeEnemyFOV
	(
	int iMaxFovDirtyTime,
	int iMaxSightDirtyTime
	)

{
	// not found in ida
	return false;
}

bool Actor::CanShoot
	(
	Entity *ent
	)

{
	bool bCanShootEnemy = false;
	if (ent->IsSubclassOfSentient())
	{
		Sentient * sen = (Sentient *)ent;
		//FIXME: macro.
		if (world->farplane_distance == 0 || world->farplane_distance * world->farplane_distance * 0.68558401 > (origin - sen->origin).lengthSquared())
		{
			if (gi.AreasConnected(
				edict->r.areanum,
				sen->edict->r.areanum))
			{
				if (G_SightTrace(
					GunPosition(),
					vec_zero,
					vec_zero,
					sen->centroid,
					this,
					sen,
					MASK_CANSEE,
					qfalse,
					"Actor::CanShoot centroid")
					||
						G_SightTrace(GunPosition(), vec_zero, vec_zero, sen->EyePosition(), this, sen, MASK_CANSEE, qfalse, "Actor::CanShoot eyes"))
				{
					bCanShootEnemy = true;
				}
			}
		}
		if (m_Enemy)
		{
			m_bCanShootEnemy = bCanShootEnemy;
			m_iCanShootCheckTime = level.inttime;
		}
	}
	else
	{

		if (gi.AreasConnected(
			edict->r.areanum,
			ent->edict->r.areanum))
		{
			if (CanSeeFrom(GunPosition(), ent))
			{
				bCanShootEnemy = true;
			}
		}
	}

	return bCanShootEnemy;
}

bool Actor::CanSeeFrom
	(
	vec3_t pos,
	Entity *ent
	)

{
	Vector vPos(pos);
	bool bCanSee = false;
	if (world->farplane_distance == 0 || world->farplane_distance * world->farplane_distance * 0.68558401 > (origin - ent->origin).lengthSquared())
	{
		if (!ent->IsSubclassOfActor() && G_SightTrace(vPos, vec_zero, vec_zero, ent->centroid, this, ent, MASK_CANSEE, qfalse, "Actor::CanSeeFrom"))
		{
			bCanSee = true;
		}
	}
	return bCanSee;
}

bool Actor::CanSeeEnemy
	(
	int iMaxDirtyTime
	)

{
	//FIXME: macro
	if (level.inttime > iMaxDirtyTime + m_iEnemyVisibleCheckTime)
		CanSee(m_Enemy, 0, 0.828 * world->farplane_distance);

	return m_bEnemyVisible;
}

bool Actor::CanShootEnemy
	(
	int iMaxDirtyTime
	)

{
	if( level.inttime > iMaxDirtyTime + m_iCanShootCheckTime )
		CanShoot( m_Enemy );

	return m_bCanShootEnemy;
}

void Actor::ShowInfo
	(
	float fDot,
	float fDist
	)

{
	float fMaxDist, fMinDot;
	static cvar_t *g_entinfo_max;

	if (!bEntinfoInit)
	{
		g_entinfo_max = gi.Cvar_Get("g_entinfo_max", "2048", NULL);
		bEntinfoInit = true;
	}

	fMinDot = 0.89999998;
	fMaxDist = g_entinfo_max->value;

	if (m_ThinkState == THINKSTATE_KILLED)
	{
		fMinDot = 0.99000001;
		fMaxDist = 512;
	}

	if (g_entinfo->integer != 4)
	{
		if (fDot <= fMinDot)
		{
			if (fDot <= 0 || fDist >= 256)
				return;
		}

		if (fMaxDist <= fDist || fDist <= 64)
			return;
	}
	if (!m_bEnableEnemy)
	{
		Vector a = origin;
		a.z += maxs.z + 74;

		G_DebugString(a, 1.0, 1.0, 1.0, 1065353216, "DISABLED");
	}
	
	{
		Vector a = origin;
		a.z += maxs.z + 56;


		G_DebugString(
			a,
			1.0,
			1.0,
			1.0,
			1065353216,
			"%i:%i:%s:%.1f",
			entnum,
			radnum,
			targetname.c_str(),
			health);
	}

	if (m_Enemy)
	{
		Vector a = origin;
		a.z += maxs.z + 38;


		G_DebugString(
			a,
			1.0,
			1.0,
			1.0,
			1065353216,
			"%i:%s:%.1f:%i",
			m_Enemy->entnum,
			m_Enemy->radnum,
			m_Enemy->targetname.c_str(),
			m_Enemy->health,
			m_PotentialEnemies.GetCurrentThreat());
	}

	if (m_ThinkState == THINKSTATE_CURIOUS)
	{

		Vector a = origin;
		a.z += maxs.z + 20;


		G_DebugString(
			a,
			1.0,
			1.0,
			1.0,
			1065353216,
			"%s:%%%.1f",
			Director.GetString(m_csThinkStateNames[THINKSTATE_CURIOUS]).c_str(),
			m_PotentialEnemies.GetCurrentVisibility() * 100);
	}
	else
	{

		Vector a = origin;
		a.z += maxs.z + 20;


		G_DebugString(
			a,
			1.0,
			1.0,
			1.0,
			1065353216,
			"%s",
			Director.GetString(m_csThinkStateNames[m_ThinkState]).c_str());
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
			1065353216,
			"%s:%s:%i - %s",
			Director.GetString(m_csThinkNames[m_Think[m_ThinkLevel]]).c_str(),
			m_pszDebugState,
			m_State,
			sAnimThreadFile.c_str());
	}

	if (g_entinfo->integer == 1)
	{
		if (m_pGrenade)
			G_DebugLine(m_pGrenade->origin, m_vGrenadePos, 1.0, 0.0, 0.0, 1.0);

		Vector a = m_vHome;
		a.z += maxs.z + 18;

		G_DebugLine(centroid, a, 0.0, 1.0, 0.0, 1.0);

		G_DebugCircle(a, m_fLeash, 0.0, 1.0, 0.0, 1.0, qtrue);

		G_DebugCircle(
			centroid,
			m_fMinDistance,
			1.0,
			0.0,
			0.0,
			1.0,
			qtrue);

		G_DebugCircle(
			centroid,
			m_fMaxDistance,
			0.0,
			0.0,
			1.0,
			1.0,
			qtrue);
	}
	else if (g_entinfo->integer == 2)
	{
		G_DebugCircle(
			centroid,
			m_fHearing,
			1.0,
			0.0,
			0.0,
			1.0,
			qtrue);
		G_DebugCircle(
			centroid,
			m_fSight,
			0.0,
			0.0,
			1.0,
			1.0,
			qtrue);
	}
}

void Actor::DefaultPain
	(
	Event *ev
	)

{
	//FIXME: macros
	m_ThinkMap[2] = 5;
	if (m_ThinkState == THINKSTATE_PAIN)
		m_bDirtyThinkState = true;
	HandlePain(ev);
}

void Actor::HandlePain
	(
	Event *ev
	)

{
	Event e1(EV_Listener_ExecuteScript);
	if (m_bEnablePain)
	{

		e1.AddConstString(STRING_GLOBAL_PAIN_SCR);

		for (int i = 1; i < ev->NumArgs(); i++)
		{
			e1.AddValue(ev->GetValue(i));
		}
		ExecuteScript(&e1);
		
		SetThinkState(THINKSTATE_PAIN, 1);

		RaiseAlertness(0.5);

		m_PainTime = level.inttime;

		Entity *ent = ev->GetEntity(1);
		if (ent && ent->IsSubclassOfSentient() && !IsTeamMate((Sentient *)ent))
		{
			m_pLastAttacker = ent;
			//FIXME: macro
			SetCuriousAnimHint(7);

			if (m_bEnableEnemy && m_ThinkStates[0] == THINKSTATE_IDLE)
			{
				SetEnemyPos(ent->origin);
				m_pszDebugState = "from_pain";
				SetThinkState(THINKSTATE_CURIOUS, 0);
			}
		}
		Unregister(STRING_PAIN);
	}
}

void Actor::EventPain
	(
	Event *ev
	)

{
	ShowInfo();

	GlobalFuncs_t *func = &GlobalFuncs[m_Think[m_ThinkLevel]];

	if (func->Pain)
		(this->*func->Pain)(ev);
}

void Actor::DefaultKilled
	(
	Event *ev,
	bool bPlayDeathAnim
	)

{
	//FIXME: macros
	ClearStates();

	m_ThinkMap[3] = 6;

	if (m_ThinkState == THINKSTATE_KILLED)
		m_bDirtyThinkState = true;

	Actor::HandleKilled(ev, bPlayDeathAnim);
}

void Actor::HandleKilled
	(
	Event *ev,
	bool bPlayDeathAnim
	)

{
	deadflag = DEAD_DEAD;
	health = 0.0;
	if (bPlayDeathAnim)
	{
		Event e1(EV_Listener_ExecuteScript);
		e1.AddConstString(STRING_GLOBAL_KILLED_SCR);
		for (int i = 1; i < ev->NumArgs(); i++)
		{
			e1.AddValue(ev->GetValue(i));
		}
		ExecuteScript(&e1);
	}
	else
	{
		//FIXME: macros
		m_ThinkMap[3] = 34;
		if (m_ThinkState == THINKSTATE_KILLED)
			m_bDirtyThinkState = true;
	}

	ClearThinkStates();
	
	SetThinkState(THINKSTATE_KILLED, 2);

	Unregister(STRING_DEATH);
	Unregister(STRING_PAIN);
}

void Actor::DispatchEventKilled
	(
	Event *ev,
	bool bPlayDeathAnim
	)

{
	GlobalFuncs_t *func = &GlobalFuncs[m_Think[m_ThinkLevel]];

	if (func->Killed)
		(this->*func->Killed)(ev, bPlayDeathAnim);
	
	SetEnemy(NULL, false);

	DisbandSquadMate(this);
	if (bPlayDeathAnim)
	{
		DropInventoryItems();
	}

}

void Actor::EventKilled
	(
	Event *ev
	)

{
	DispatchEventKilled(ev, true);

	Player *p1 = (Player *)ev->GetEntity(1);
	if (p1)
	{
		if (p1->IsSubclassOfPlayer() && p1->m_Team == m_Team)
		{
			p1->m_iNumEnemiesKilled++;
		}
		if (p1->IsSubclassOfSentient())
		{
			for (Sentient *pSent = level.m_HeadSentient[m_Team]; pSent; pSent = pSent->m_NextSentient)
			{
				if (pSent != this)
				{
					if (pSent->IsSubclassOfActor())
						NotifySquadmateKilled(pSent, p1);
				}
			}
		}

	}

}

void Actor::EventBeDead
	(
	Event *ev
	)

{
	DispatchEventKilled(ev, false);
}

void Actor::DeathEmbalm
	(
	Event *ev
	)

{
	if (maxs[2] > 8)
	{
		maxs[2] -= 4;
		if (maxs[2] > 8)
		{
			Event e1(EV_Actor_DeathEmbalm);
			PostEvent(e1, 0.5);
		}
		else
		{
			maxs[2] = 8.0;
		}

		setSize(mins, maxs);
	}
}

void Actor::DeathSinkStart
	(
	Event *ev
	)
{
	flags &= ~FL_BLOOD;

	setMoveType(MOVETYPE_NONE);

	Entity::DeathSinkStart(ev);
}

bool Actor::NoticeShot
	(
	Sentient *pShooter,
	Sentient *pTarget,
	float fDist
	)

{
	if (pShooter->m_Team != m_Team)
	{
		return true;
	}

	m_bEnemyIsDisguised = false;

	if (pTarget)
	{
		if (DoesTheoreticPathExist(pShooter->origin, fDist * 1.5) || CanSee(pTarget, 0, 0.828 * world->farplane_distance))
		{
			m_PotentialEnemies.ConfirmEnemy(this, pTarget);
		}
	}

	return false;

}

bool Actor::NoticeFootstep
	(
	Sentient *pPedestrian
	)

{
	if (m_Team == pPedestrian->m_Team || pPedestrian->m_bIsDisguised)
		return false;
	if (pPedestrian == m_Enemy)
	{
		if (EnemyInFOV(0) && CanSeeEnemy(0))
		{
			return false;
		}
	}
	else
	{
		if (InFOV(pPedestrian->centroid, m_fFov, m_fFovDot) && gi.AreasConnected(edict->r.areanum, pPedestrian->edict->r.areanum))
		{
			if (CanSeeFrom(EyePosition(), pPedestrian))
			{
				return false;
			}
		}
	}
	return true;
}

bool Actor::NoticeVoice
	(
	Sentient *pVocallist
	)

{
	if (IsSquadMate(pVocallist))
		return false;
	if (pVocallist == m_Enemy)
	{
		if (EnemyInFOV(0) && CanSeeEnemy(0))
		{
			return false;
		}
	}
	else
	{
		if (InFOV(pVocallist->centroid, m_fFov, m_fFovDot) && gi.AreasConnected(edict->r.areanum, pVocallist->edict->r.areanum))
		{
			if (CanSeeFrom(EyePosition(), pVocallist))
			{
				return false;
			}
		}
	}
	return true;
}

void Actor::ClearLookEntity
	(
	void
	)

{
	if (m_pLookEntity)
	{
		if (m_pLookEntity->IsSubclassOfAnimate())
		{
			delete m_pLookEntity;
		}
		m_pLookEntity = NULL;
	}
}

void Actor::LookAt
	(
	Vector& vec
	)

{
	if (g_showlookat->integer == entnum || g_showlookat->integer == -1)
	{
		Com_Printf(
			"Script lookat: %i %i %s looking at point %.0f %.0f %.0f\n",
			entnum,
			radnum,
			targetname.c_str(),
			vec.x,
			vec.y,
			vec.z);
	}

	ClearLookEntity();

	TempWaypoint *twp = new TempWaypoint();
	m_pLookEntity = twp;

	m_pLookEntity->setOrigin(vec);

}

void Actor::LookAt
	(
	Listener *l
	)

{
	ClearLookEntity();
	if ( l )
	{
		if (!l->isInheritedBy(&SimpleEntity::ClassInfo))
		{
			ScriptError("Bad look entity with classname '%s' specified for '%s' at (%f %f %f)\n", 
				l->getClassname(),
				targetname.c_str(),
				origin.x,
				origin.y,
				origin.z
				);
		}
		if (l != this)
		{
			l = (SimpleEntity*)l;
			if (g_showlookat->integer == entnum || g_showlookat->integer == -1)
			{
				Com_Printf(
					"Script lookat: %i %i %s looking at point %.0f %.0f %.0f\n",
					entnum,
					radnum,
					targetname.c_str(),
					((SimpleEntity*)l)->origin.x,
					((SimpleEntity*)l)->origin.y,
					((SimpleEntity*)l)->origin.z);
			}
			m_pLookEntity = (SimpleEntity*)l;
		}
	}
}

void Actor::ForwardLook
	(
	void
	)

{
	// not found in ida
}

void Actor::LookAtLookEntity
	(
	void
	)

{
	Vector dir;
	if (m_pLookEntity->IsSubclassOfSentient())
	{
		dir = ((Sentient *)m_pLookEntity.Pointer())->EyePosition() - EyePosition();
	}
	else
	{
		dir = m_pLookEntity->centroid - EyePosition();
	}

	SetDesiredLookDir(dir);
}

void Actor::IdleLook
	(
	void
	)

{
	if (m_pLookEntity)
	{
		LookAtLookEntity();
	}
	else
	{
		m_bHasDesiredLookAngles = false;
	}
}

void Actor::IdleLook
	(
	vec3_t dir
	)

{
	if (m_pLookEntity)
	{
		LookAtLookEntity();
	}
	else
	{
		SetDesiredLookDir(dir);
	}
}

void Actor::SetDesiredLookDir
	(
	vec3_t dir
	)

{
	m_bHasDesiredLookAngles = true;
	vectoangles( dir, m_DesiredLookAngles );
	m_DesiredLookAngles[ 1 ] = m_DesiredLookAngles[ 1 ] - angles[ 1 ];
	m_DesiredLookAngles[ 1 ] = AngleNormalize180( m_DesiredLookAngles[ 1 ] );
	m_DesiredLookAngles[ 0 ] = AngleNormalize180( m_DesiredLookAngles[ 0 ] );
}

void Actor::SetDesiredLookAnglesRelative
	(
	vec3_t ang
	)

{
	m_bHasDesiredLookAngles = true;
	m_DesiredLookAngles[ 0 ] = AngleNormalize180( ang[ 0 ] );
	m_DesiredLookAngles[ 1 ] = AngleNormalize180( ang[ 1 ] );
	m_DesiredLookAngles[ 2 ] = AngleNormalize180( ang[ 2 ] );
}

void Actor::EventLookAt
	(
	Event *ev
	)

{
	if (ev->IsVectorAt(1))
	{
		Vector dir = ev->GetVector(1);
		if (g_showlookat->integer == entnum || g_showlookat->integer == -1)
		{
			Com_Printf(
				"Script lookat: %i %i %s looking at point %.0f %.0f %.0f\n",
				entnum,
				radnum,
				targetname.c_str(),
				dir.x,
				dir.y,
				dir.z);
		}
		if (m_pLookEntity)
		{
			if (m_pLookEntity->IsSubclassOfAnimate())
			{
				delete m_pLookEntity;
			}
			m_pLookEntity = NULL;
		}

		TempWaypoint *twp = new TempWaypoint();
		m_pLookEntity = twp;

		m_pLookEntity->setOrigin(dir);
	}
	else
	{
		LookAt(ev->GetListener(1));
	}

	m_iLookFlags = 0;
}

void Actor::EventEyesLookAt
	(
	Event *ev
	)

{
	EventLookAt(ev);

	m_iLookFlags = m_pLookEntity != NULL;
}

void Actor::NoPoint
	(
	void
	)

{
	m_vLUpperArmDesiredAngles[0] = 0;
	m_vLUpperArmDesiredAngles[1] = 0;
	m_vLUpperArmDesiredAngles[2] = 0;
}

void Actor::IdlePoint
	(
	void
	)

{
	if (m_pPointEntity)
	{
		float yaw = AngleNormalize180(origin[1] - angles[1] + 30);
		if (yaw > 100)
			yaw = 100;
		if (yaw < -80)
			yaw = -80;
		m_vLUpperArmDesiredAngles[0] = origin[0];
		m_vLUpperArmDesiredAngles[1] = yaw;
		m_vLUpperArmDesiredAngles[2] = origin[2];
	}
	else
	{
		NoPoint();
	}
}

void Actor::ClearPointEntity
	(
	void
	)

{
	if (m_pPointEntity)
	{
		if (m_pPointEntity->IsSubclassOfAnimate())
		{
			delete m_pPointEntity;
		}
		m_pPointEntity = NULL;
	}
}

void Actor::PointAt
	(
	Vector& vec
	)

{
	TempWaypoint *twp; // ebx

	ClearPointEntity();
	twp = new TempWaypoint();
	m_pPointEntity = twp;
	m_pPointEntity->setOrigin(vec);
}

void Actor::PointAt
	(
	Listener* l
	)

{
	ClearPointEntity();
	if (l)
	{
		if (l->isInheritedBy(&SimpleEntity::ClassInfo))
		{
			ScriptError("Bad point entity with classname '%s' specified for '%s' at (%f %f %f)\n",
				l->getClassname(),
				targetname.c_str(),
				origin.x,
				origin.y,
				origin.z);
		}
		if (l != this)
		{
			m_pPointEntity = (SimpleEntity *)l;
		}
	}
}

void Actor::EventPointAt
	(
	Event *ev
	)

{
	if (ev->IsVectorAt(1))
	{
		Vector dir = ev->GetVector(1);
		
		if (m_pPointEntity)
		{
			if (m_pPointEntity->IsSubclassOfAnimate())
			{
				delete m_pPointEntity;
			}
			m_pPointEntity = NULL;
		}

		TempWaypoint *twp = new TempWaypoint();
		m_pPointEntity = twp;

		m_pPointEntity->setOrigin(dir);
	}
	else
	{
		PointAt(ev->GetListener(1));
	}
}

void Actor::ClearTurnEntity
	(
	void
	)

{
	if (m_pTurnEntity)
	{
		if (m_pTurnEntity->IsSubclassOfAnimate())
		{
			delete m_pTurnEntity;
		}
		m_pTurnEntity = NULL;
	}
}

void Actor::TurnTo
	(
	Vector& vec
	)

{
	TempWaypoint *twp; // ebx

	ClearTurnEntity();
	twp = new TempWaypoint();
	m_pTurnEntity = twp;
	m_pTurnEntity->setOrigin(vec);
}

void Actor::TurnTo
	(
	Listener *l
	)

{
	ClearTurnEntity();
	if (l)
	{
		if (l->isInheritedBy(&SimpleEntity::ClassInfo))
		{
			ScriptError("Bad turn entity with classname '%s' specified for '%s' at (%f %f %f)\n",
				l->getClassname(),
				targetname.c_str(),
				origin.x,
				origin.y,
				origin.z);
		}
		if (l != this)
		{
			m_pTurnEntity = (SimpleEntity *)l;
		}
	}
	else
	{
		m_pTurnEntity = this;
	}
}

void Actor::IdleTurn
	(
	void
	)

{
	if (m_pTurnEntity)
	{
		for (int i = 0; i >= 1; i++)
		{
			if (m_pTurnEntity == this)
			{
				m_YawAchieved = true;
				m_pTurnEntity = NULL;
				return;
			}
			vec2_t facedir;
			facedir[0] = m_pTurnEntity->centroid[0] - origin[0];
			facedir[1] = m_pTurnEntity->centroid[1] - origin[1];
			if (facedir[0] != 0 || facedir[1] != 0)
			{
				m_YawAchieved = false;
				m_DesiredYaw = vectoyaw(facedir);
			}

			float error = m_DesiredYaw - angles[1];
			if (error <= 180)
			{
				if (error < -180.0)
					error += 360;
			}
			else
			{
				error -= 360;
			}
			if (error >= m_fTurnDoneError + 0.001 || -(m_fTurnDoneError + 0.001) >= error)
			{
				return;
			}


			if (Director.iPaused-- == 1)
			{
				Director.ExecuteRunning();
			}
			Unregister(STRING_TURNDONE);
			Director.iPaused++;

			//FIXME: what ?? it was m_pTurnEntity == m_pTurnEntity
			if (m_pTurnEntity == NULL)
			{
				break;
			}
		}
	}
}

void Actor::EventTurnTo
	(
	Event *ev
	)

{
	if (ev->IsVectorAt(1))
	{
		Vector dir = ev->GetVector(1);

		if (m_pTurnEntity)
		{
			if (m_pTurnEntity->IsSubclassOfAnimate())
			{
				delete m_pTurnEntity;
			}
			m_pTurnEntity = NULL;
		}

		TempWaypoint *twp = new TempWaypoint();
		m_pTurnEntity = twp;

		m_pTurnEntity->setOrigin(dir);
	}
	else
	{
		TurnTo(ev->GetListener(1));
	}
}

void Actor::EventSetTurnDoneError
	(
	Event *ev
	)

{
	m_fTurnDoneError = ev->GetFloat(1);
	if (m_fTurnDoneError < 0)
	{
		m_fTurnDoneError = 0;
		ScriptError("turndoneerror was set to a negative value - capped to 0");
	}
}

void Actor::EventGetTurnDoneError
	(
	Event *ev
	)

{
	ev->AddFloat(m_fTurnDoneError);
}

void Actor::LookAround
	(
	float fFovAdd
	)

{
	vec3_t vDest, vAngle;
	float fModTime;

	if (level.inttime >= m_iNextLookTime)
	{

	}

	if (m_bHasDesiredLookDest)
	{

	}
	else
	{
		m_bHasDesiredLookAngles = false;
	}
	// FIXME: stub
	STUB();
}

bool Actor::SoundSayAnim
	(
	const_str name,
	bool bLevelSayAnim
	)

{
	int animnum = gi.Anim_NumForName(edict->tiki, Director.GetString(name).c_str());
	if (animnum == -1)
	{
		ChangeSayAnim();
		m_bSayAnimSet = true;
		m_iSaySlot = -2;
		m_bLevelSayAnim = bLevelSayAnim;

		Com_Printf(
			"Couldn't find animation '%s' in '%s' - trying sound alias instead.\n",
			Director.GetString(name).c_str(),
			edict->tiki->a->name);

		Sound(Director.GetString(name), 0,0,0,0,0,0, 1,1);

		return true;
	}
	return false;
}

void Actor::EventSetAnim
	(
	Event *ev
	)

{
	//"anim slot weight flagged"
	int numArgs = ev->NumArgs(), slot = 0, animnum;
	float weight = 1;
	const_str anim, flagVal;
	qboolean flagged = qfalse;

	//FIXME: better notation, but this works for now.
	if (numArgs < 1 || numArgs > 4)
	{
		ScriptError("bad number of arguments");
	}
	else if (numArgs == 1)
	{
		anim = ev->GetConstString(1);
		animnum = gi.Anim_NumForName(edict->tiki, Director.GetString(anim).c_str());
		if (animnum == -1)
		{
			ScriptException::next_bIsForAnim = true;
			ScriptError("unknown animation '%s' in '%s'", Director.GetString(anim).c_str(), edict->tiki->a->name);
		}
	}
	else if (numArgs == 2)
	{
		anim = ev->GetConstString(1);
		animnum = gi.Anim_NumForName(edict->tiki, Director.GetString(anim).c_str());
		if (animnum == -1)
		{
			ScriptException::next_bIsForAnim = true;
			ScriptError("unknown animation '%s' in '%s'", Director.GetString(anim).c_str(), edict->tiki->a->name);
		}
		slot = ev->GetInteger(2);
		if (slot > 2)
		{
			ScriptError("Bad animation slot, only 0 and 1 supported");
		}
	}
	else if (numArgs == 3)
	{
		weight = ev->GetFloat(3);
		if (weight < 0)
		{
			ScriptError("Negative anim weight not allowed");
		}
		if (weight != 0)
		{
			slot = ev->GetInteger(2);
			if (slot > 2)
			{
				ScriptError("Bad animation slot, only 0 and 1 supported");
			}
		}

		anim = ev->GetConstString(1);
		animnum = gi.Anim_NumForName(edict->tiki, Director.GetString(anim).c_str());
		if (animnum == -1)
		{
			ScriptException::next_bIsForAnim = true;
			ScriptError("unknown animation '%s' in '%s'", Director.GetString(anim).c_str(), edict->tiki->a->name);
		}
	}
	else if (numArgs == 4)
	{
		flagVal = ev->GetConstString(4);
		flagged = qtrue;
		if (flagVal != STRING_FLAGGED)
		{
			ScriptError("unknown keyword '%s', expected 'flagged'", Director.GetString(flagVal).c_str());
		}
		weight = ev->GetFloat(3);
		if (weight < 0)
		{
			ScriptError("Negative anim weight not allowed");
		}
		if (weight != 0)
		{
			slot = ev->GetInteger(2);
			if (slot > 2)
			{
				ScriptError("Bad animation slot, only 0 and 1 supported");
			}
		}

		anim = ev->GetConstString(1);
		animnum = gi.Anim_NumForName(edict->tiki, Director.GetString(anim).c_str());
		if (animnum == -1)
		{
			ScriptException::next_bIsForAnim = true;
			ScriptError("unknown animation '%s' in '%s'", Director.GetString(anim).c_str(), edict->tiki->a->name);
		}
	}
	
	if (!slot)
	{
		flagged = qtrue;
	}
	if (flagged)
	{
		parm.motionfail = qtrue;
	}
	if (!m_bLevelMotionAnim)
	{
		if (slot)
		{
			m_weightType[GetMotionSlot(slot)] = 0;
		}
		else
		{
			ChangeMotionAnim();
			m_bMotionAnimSet = true;
		}
		StartMotionAnimSlot(slot, animnum, weight);
		if (flagged)
		{
			m_iMotionSlot = GetMotionSlot(slot);
			parm.motionfail = qfalse;
		}
	}
}

void Actor::EventIdleSayAnim
	(
	Event *ev
	)

{
	const_str name;
	if (ev->NumArgs())
	{
		if (ev->NumArgs() != 1)
		{
			ScriptError("bad number of arguments");
		}

		name = ev->GetConstString(1);

		if (m_ThinkState <= THINKSTATE_ATTACK || m_ThinkState == THINKSTATE_GRENADE)
		{
			m_csSayAnim = name;
			//FIXME: macro
			m_bNextLevelSayAnim = 1;
		}
		else
		{
			if (!SoundSayAnim(name,true) )
			{
				m_csSayAnim = name;
				//FIXME: macro
				m_bNextLevelSayAnim = 1;
			}

		}
	}
	else if (m_bLevelSayAnim == true)
	{
		AnimFinished(m_iSaySlot, true);
	}
}

void Actor::EventSayAnim
	(
	Event *ev
	)

{
	const_str name;
	if (ev->NumArgs())
	{
		if (ev->NumArgs() != 1)
		{
			ScriptError("bad number of arguments");
		}

		name = ev->GetConstString(1);

		if (m_ThinkState <= THINKSTATE_KILLED)
		{
			m_csSayAnim = name;
			//FIXME: macro
			m_bNextLevelSayAnim = 2;
		}
		else
		{
			if (!SoundSayAnim(name, true))
			{
				m_csSayAnim = name;
				//FIXME: macro
				m_bNextLevelSayAnim = 2;
			}

		}
	}
	else if (m_bLevelSayAnim == true)
	{
		AnimFinished(m_iSaySlot, true);
	}
}

void Actor::EventSetSayAnim
	(
	Event *ev
	)

{
	const_str name;
	if (ev->NumArgs())
	{
		if (ev->NumArgs() != 1)
		{
			ScriptError("bad number of arguments");
		}
		if (!m_bLevelSayAnim)
		{
			name = ev->GetConstString(1);

			parm.sayfail = qtrue;

			if (!SoundSayAnim(name, true))
			{
				//FIXME: assumption, not sure. I think it's an inline func inside UpdateSayAnim()
				UpdateSayAnim();
			}
			parm.sayfail = qfalse;
		}
	}
}

void Actor::EventSetMotionAnim
	(
	Event *ev
	)

{
	if (ev->NumArgs() != 1)
	{
		ScriptError("bad number of arguments");
	}
	const_str anim = ev->GetConstString(1);
	int animnum = gi.Anim_NumForName(edict->tiki, Director.GetString(anim).c_str());
	if (animnum == -1)
	{
		ScriptException::next_bIsForAnim = 1;
		ScriptError("unknown animation '%s' in '%s'", Director.GetString(anim).c_str(), edict->tiki->a->name);
	}

	parm.motionfail = qtrue;

	if (!m_bLevelMotionAnim)
	{
		ChangeMotionAnim();
		m_bMotionAnimSet = true;
		StartMotionAnimSlot(0, animnum, 1.0);
		m_iMotionSlot = GetMotionSlot(0);
		parm.motionfail = qfalse;
	}
}

void Actor::EventSetAimMotionAnim
	(
	Event *ev
	)

{
	if (ev->NumArgs() != 3)
	{
		ScriptError("bad number of arguments");
	}

	//FIXME: maybe inline func ?

	const_str anim = ev->GetConstString(1);
	int animnum = gi.Anim_NumForName(edict->tiki, Director.GetString(anim).c_str());
	if (animnum == -1)
	{
		ScriptException::next_bIsForAnim = 1;
		ScriptError("unknown animation '%s' in '%s'", Director.GetString(anim).c_str(), edict->tiki->a->name);
	}

	const_str anim2 = ev->GetConstString(2);
	int animnum2 = gi.Anim_NumForName(edict->tiki, Director.GetString(anim2).c_str());
	if (animnum2 == -1)
	{
		ScriptException::next_bIsForAnim = 1;
		ScriptError("unknown animation '%s' in '%s'", Director.GetString(anim2).c_str(), edict->tiki->a->name);
	}

	const_str anim3 = ev->GetConstString(3);
	int animnum3 = gi.Anim_NumForName(edict->tiki, Director.GetString(anim3).c_str());
	if (animnum3 == -1)
	{
		ScriptException::next_bIsForAnim = 1;
		ScriptError("unknown animation '%s' in '%s'", Director.GetString(anim3).c_str(), edict->tiki->a->name);
	}


	parm.motionfail = qtrue;

	if (!m_bLevelMotionAnim)
	{
		ChangeMotionAnim();
		m_bMotionAnimSet = true;
		UpdateAimMotion();
		StartAimMotionAnimSlot(0, animnum);
		StartAimMotionAnimSlot(1, animnum2);
		StartAimMotionAnimSlot(2, animnum3);
		m_iMotionSlot = GetMotionSlot(1);
		parm.motionfail = qfalse;
	}

}

void Actor::EventSetActionAnim
	(
	Event *ev
	)

{
	if (ev->NumArgs() != 3)
	{
		ScriptError("bad number of arguments");
	}

	m_fAimLimit_down = ev->GetFloat(2);
	if (m_fAimLimit_down > 0)
	{
		m_fAimLimit_down = -0.001;
		ScriptError("Positive lower_limit not allowed");
	}

	m_fAimLimit_up = ev->GetFloat(3);
	if (m_fAimLimit_up <= 0)
	{
		m_fAimLimit_up = 0.001;
		ScriptError("Negative upper_limit not allowed");
	}

	const_str anim = ev->GetConstString(1);
	int animnum = gi.Anim_NumForName(edict->tiki, Director.GetString(anim).c_str());
	if (animnum == -1)
	{
		ScriptException::next_bIsForAnim = 1;
		ScriptError("unknown animation '%s' in '%s'", Director.GetString(anim).c_str(), edict->tiki->a->name);
	}
	//FIXME: repeated 3 times for no reason ?
	/*
	
		const_str anim2 = ev->GetConstString(2);
		int animnum2 = gi.Anim_NumForName(edict->tiki, Director.GetString(anim2));
		if (animnum2 == -1)
		{
			ScriptException::next_bIsForAnim = 1;
			ScriptError("unknown animation '%s' in '%s'", Director.GetString(anim2), edict->tiki->a->name);
		}

		const_str anim3 = ev->GetConstString(3);
		int animnum3 = gi.Anim_NumForName(edict->tiki, Director.GetString(anim3));
		if (animnum3 == -1)
		{
			ScriptException::next_bIsForAnim = 1;
			ScriptError("unknown animation '%s' in '%s'", Director.GetString(anim3), edict->tiki->a->name);
		}
	*/
	parm.upperfail = qtrue;
	if (!m_bLevelActionAnim)
	{
		ChangeActionAnim();
		m_bAimAnimSet = true;
		m_bActionAnimSet = true;
		UpdateAim();
		//FIXME: 3 calls on same animnum ?? weird.
		StartAimAnimSlot(0, animnum);
		StartAimAnimSlot(1, animnum);
		StartAimAnimSlot(2, animnum);
		m_iActionSlot = GetActionSlot(0);
		parm.upperfail = qfalse;
	}
}

void Actor::EventUpperAnim
	(
	Event *ev
	)

{
	if (ev->NumArgs())
	{
		if (ev->NumArgs() != 1)
		{
			ScriptError("bad number of arguments");
		}
		const_str anim = ev->GetConstString(1);
		int animnum = gi.Anim_NumForName(edict->tiki, Director.GetString(anim).c_str());
		if (animnum == -1)
		{
			ScriptException::next_bIsForAnim = 1;
			ScriptError("unknown animation '%s' in '%s'", Director.GetString(anim).c_str(), edict->tiki->a->name);
		}

		m_csUpperAnim = anim;
	}
	else if (m_bLevelActionAnim)
	{
		AnimFinished(m_iActionSlot, true);
	}
}

void Actor::EventSetUpperAnim
	(
	Event *ev
	)

{
	if (ev->NumArgs() != 1)
	{
		ScriptError("bad number of arguments");
	}

	const_str anim = ev->GetConstString(1);
	int animnum = gi.Anim_NumForName(edict->tiki, Director.GetString(anim).c_str());
	if (animnum == -1)
	{
		ScriptException::next_bIsForAnim = 1;
		ScriptError("unknown animation '%s' in '%s'", Director.GetString(anim).c_str(), edict->tiki->a->name);
	}
	parm.upperfail = qtrue;
	if (!m_bLevelActionAnim)
	{
		ChangeActionAnim();
		m_bActionAnimSet = true;
		StartActionAnimSlot(animnum);
		m_iActionSlot = SimpleActor::GetActionSlot(0);
		parm.upperfail = qfalse;
	}
}

void Actor::EventEndActionAnim
	(
	Event *ev
	)

{
	parm.upperfail = qtrue;
	if (!m_bLevelActionAnim)
	{
		ChangeActionAnim();
		parm.upperfail = qfalse;
	}
}

void Actor::EventDamagePuff
	(
	Event *ev
	)

{
	
	Vector pos = ev->GetVector(1), dir = ev->GetVector(2);
	gi.SetBroadcastVisible(pos, NULL);
	//FIXME: macro
	gi.MSG_StartCGM(7);
	gi.MSG_WriteCoord(pos.x);
	gi.MSG_WriteCoord(pos.y);
	gi.MSG_WriteCoord(pos.z);

	gi.MSG_WriteDir(dir);
	gi.MSG_WriteBits(0, 1);
	gi.MSG_EndCGM();
}

void Actor::SafeSetOrigin
	(
	vec3_t newOrigin
	)

{
	if (newOrigin == origin)
	{
		return;
	}

	setOrigin(newOrigin);
	if (!m_bNoPlayerCollision)
	{
		Player *p = (Player *)G_GetEntity(0);
		if (p && IsTouching(p))
		{
			Com_Printf("(entnum %d, radnum %d) is going not solid to not get stuck in the player\n", entnum, radnum);
			m_bNoPlayerCollision = true;
			setSolidType(SOLID_NOT);
		}
	}
}

void Actor::DoMove
	(
	void
	)

{
	mmove_t mm;
	//FIXME: macros
	if (m_bDoPhysics
		&& m_iOriginTime != level.inttime
		&& !m_pGlueMaster
		&& !bindmaster)
	{
		switch (m_eAnimMode)
		{
		case 1:
		{
			SetMoveInfo(&mm);

			VectorCopy2D(frame_delta, mm.desired_dir);
			mm.desired_speed = VectorNormalize2D(mm.desired_dir) / level.frametime;

			if (mm.desired_speed > m_maxspeed)
				mm.desired_speed = m_maxspeed;

			MmoveSingle(&mm);
			GetMoveInfo(&mm);
			break;

		}
			break;
		case 2:
		{
			MovePath(frame_delta.length() / level.frametime);
		}
			break;
		case 3:
		{
			MovePathGoal(frame_delta.length() / level.frametime);
		}
			break;
		case 4:
		{

			SetMoveInfo(&mm);

			mm.desired_speed = frame_delta.length() / level.frametime;

			if (mm.desired_speed > m_maxspeed)
				mm.desired_speed = m_maxspeed;

			VectorCopy2D(m_Dest, mm.desired_dir);
			VectorAdd2D(-origin, mm.desired_dir, mm.desired_dir);

			VectorNormalize2D(mm.desired_dir);

			MmoveSingle(&mm);
			GetMoveInfo(&mm);
			break;
		}
			break;
		case 5:
		{
			trace_t trace;
			trace = G_Trace(origin, mins, maxs, frame_delta + origin, this, edict->clipmask & 0xF9FFE47D, qtrue, "Actor");
			SafeSetOrigin(trace.endpos);
			velocity = frame_delta / level.frametime;
		}
			break;
		case 6:
		{
			SafeSetOrigin(frame_delta+origin);
			velocity = frame_delta / level.frametime;
		}
			break;
		case 7:
		{
			SafeSetOrigin(m_pFallPath->pos[m_pFallPath->currentPos]);
			m_pFallPath->currentPos++;
			velocity = vec_zero;
		}
			break;
		default:
			break;
		}
		m_maxspeed = 1000000.0;
	}
}

void Actor::AnimFinished
	(
	int slot,
	bool stop
	)

{
	if (stop && slot >= 0)
		animFlags[slot] |= ANIM_NOACTION;
	if (slot == m_iMotionSlot)
	{
		if (stop)
		{
			m_bLevelMotionAnim = 0;
			m_iMotionSlot = -1;
		}
		MPrintf("flagged anim finished slot %d\n", slot);
		Unregister( 41);
	}
	if (slot == m_iActionSlot)
	{
		if (stop)
			ChangeActionAnim();
		if (m_csUpperAnim == 1)
		{
			MPrintf("upper anim finished slot %d\n", slot);
			Unregister(STRING_UPPERANIMDONE);
		}
	}
	if (slot == m_iSaySlot)
	{
		if (stop)
			SimpleActor::ChangeSayAnim();
		if (m_csSayAnim == 1)
		{
			MPrintf("say anim finished slot %d\n", slot);
			Unregister(STRING_SAYDONE);
		}
	}
	else if (slot == GetSaySlot() && stop)
	{
		ChangeSayAnim();
	}
}

void Actor::AnimFinished
	(
	int slot
	)

{
	animFlags[slot] &= ~ANIM_FINISHED;
	AnimFinished(slot, animFlags[slot] & ANIM_LOOP);
}

void Actor::PlayAnimation
	(
	Event *ev
	)

{
	Event e1(EV_Listener_ExecuteScript);
	e1.AddConstString(STRING_GLOBAL_ANIM_SCR);
	e1.AddValue(ev->GetValue(1));
	ExecuteScript(&e1);
}

void Actor::PlayScriptedAnimation
	(
	Event *ev
	)

{
	Event e1(EV_Listener_ExecuteScript);
	e1.AddConstString(STRING_GLOBAL_ANIM_SCRIPTED_SCR);
	e1.AddValue(ev->GetValue(1));
	ExecuteScript(&e1);
}

void Actor::PlayNoclipAnimation
	(
	Event *ev
	)

{
	Event e1(EV_Listener_ExecuteScript);
	e1.AddConstString(STRING_GLOBAL_ANIM_NOCLIP_SCR);
	e1.AddValue(ev->GetValue(1));
	ExecuteScript(&e1);
}

void Actor::MoveDest
	(
	float fMoveSpeed
	)

{
	mmove_t mm;
	vec2_t offset;

	SetMoveInfo(&mm);

	if (fMoveSpeed > m_maxspeed)
		fMoveSpeed = m_maxspeed;

	offset[0] = m_Dest[0] - origin[0];
	offset[1] = m_Dest[1] - origin[1];

	VectorNormalize2D(offset);

	MmoveSingle(&mm);

	GetMoveInfo(&mm);
}

void Actor::MovePath
	(
	float fMoveSpeed
	)

{
	//FIXME: macros
	mmove_t mm;



	SetMoveInfo(&mm);

	mm.desired_speed = fMoveSpeed;
	if (mm.desired_speed > m_maxspeed)
	{
		mm.desired_speed = m_maxspeed;
	}

	m_Path.UpdatePos(origin);

	if (ai_debugpath->integer)
	{
		;
		Vector pos, dest;
		pos = origin;
		pos.z += 32;
		dest = m_Path.CurrentDelta();
		dest += origin;
		dest.z += 32;
		G_DebugLine(pos, dest, 1, 0, 0, 1);
		for (PathInfo *current_path = m_Path.CurrentNode(); current_path != m_Path.LastNode(); current_path = m_Path.NextNode())
		{
			pos = current_path->point;
			pos.z += 32;
			dest = m_Path.CurrentDelta();
			dest += origin;
			//FIXME: not sure.
			dest.x -= 32;
			dest.z += 32;
			G_DebugLine(pos, dest, 0, 1, 0, 1);
		}
	}

	VectorCopy2D(m_Path.CurrentDelta(), mm.desired_dir);

	MmoveSingle(&mm);
	if (!mm.hit_obstacle)
	{
		if (m_WallDir)
		{
			if (level.inttime >= m_iWallDodgeTimeout)
			{
				m_WallDir = 0;
			}
			else
			{
				if (DotProduct2D(mm.desired_dir, m_PrevObstacleNormal) > 0 && CrossProduct2D(mm.desired_dir, m_PrevObstacleNormal) < 0)
				{
					m_iWallDodgeTimeout = level.inttime + 1000;
					m_WallDir = -m_WallDir;
				}
			}
		}
		GetMoveInfo(&mm);
		return;
	}

	velocity.copyTo(mm.velocity);
	origin.copyTo(mm.origin);
	mm.groundPlane = m_groundPlane;
	mm.walking = m_walking;
	m_groundPlaneNormal.copyTo(mm.groundPlaneNormal);
	VectorCopy2D(mm.obstacle_normal, m_PrevObstacleNormal);

	m_Path.UpdatePos(mm.hit_origin);
	if (!m_Path.NextNode())
	{
		if (!m_WallDir)
		{
			auto dir = m_Path.CurrentDelta();
			if (m_Path.NextNode())
			{
				dir = m_Path.NextNode()->dir;
			}

			if (dir[1] * mm.obstacle_normal[0] < dir[0] * mm.obstacle_normal[1])
			{
				mm.desired_dir[0] = mm.obstacle_normal[0];
				mm.desired_dir[1] = -mm.obstacle_normal[1];
				m_WallDir = 1;
			}
			else
			{
				mm.desired_dir[0] = -mm.obstacle_normal[0];
				mm.desired_dir[1] = mm.obstacle_normal[1];
				m_WallDir = -1;
			}
			MmoveSingle(&mm);
			if (!mm.hit_obstacle)
			{
				m_iWallDodgeTimeout = level.inttime + 1000;
			}
			else
			{
				m_WallDir = 0;
				m_iWallDodgeTimeout = 0;
			}
			GetMoveInfo(&mm);
			return;
		}
	}

	if (! (m_Path.NextNode() && m_Path.NextNode()->bAccurate && m_Path.IsSide()) )
	{
		if (m_WallDir)
		{
			if (m_WallDir < 0)
			{
				mm.desired_dir[0] = -mm.obstacle_normal[1];
				mm.desired_dir[1] = mm.obstacle_normal[0];
			}
			else
			{
				mm.desired_dir[0] = mm.obstacle_normal[1];
				mm.desired_dir[1] = -mm.obstacle_normal[0];
			}
			MmoveSingle(&mm);
			if (mm.hit_obstacle)
			{
				m_WallDir = 0;
				m_iWallDodgeTimeout = 0;
			}
			GetMoveInfo(&mm);
			return;
		}
		else
		{

			auto dir = m_Path.CurrentDelta();
			auto pNextNode = m_Path.NextNode();

			if (pNextNode)
			{
				dir = pNextNode->dir;
			}

			vec2_t vDelta;
			vDelta[0] = pNextNode->point[1] - mm.hit_origin[0];
			vDelta[1] = pNextNode->point[0] - mm.hit_origin[1];

			if (DotProduct2D(dir, vDelta) >= 0)
			{
				float fCross = CrossProduct2D(dir, vDelta);
				if (fCross <= -16)
				{
					mm.desired_dir[1] = mm.obstacle_normal[0];
					mm.desired_dir[0] = -mm.obstacle_normal[1];
				}
				else
				{
					if (fCross >= 16)
					{
					
						mm.desired_dir[0] = mm.obstacle_normal[1];
						mm.desired_dir[1] = -mm.obstacle_normal[0];
						MmoveSingle(&mm);
						GetMoveInfo(&mm);
						return;
					}
				}
			}
			{
				if (!m_WallDir)
				{
					auto dir = m_Path.CurrentDelta();
					if (m_Path.NextNode())
					{
						dir = m_Path.NextNode()->dir;
					}

					if (dir[1] * mm.obstacle_normal[0] < dir[0] * mm.obstacle_normal[1])
					{
						mm.desired_dir[0] = mm.obstacle_normal[1];
						mm.desired_dir[1] = -mm.obstacle_normal[0];
						m_WallDir = 1;
					}
					else
					{
						mm.desired_dir[0] = -mm.obstacle_normal[1];
						mm.desired_dir[1] = mm.obstacle_normal[0];
						m_WallDir = -1;
					}
					MmoveSingle(&mm);
					if (!mm.hit_obstacle)
					{
						m_iWallDodgeTimeout = level.inttime + 1000;
					}
					else
					{
						m_WallDir = 0;
						m_iWallDodgeTimeout = 0;
					}
					GetMoveInfo(&mm);
					return;
				}
				else if (m_WallDir < 0)
				{
					mm.desired_dir[0] = -mm.obstacle_normal[1];
					mm.desired_dir[1] = mm.obstacle_normal[0];
				}
				else
				{
					mm.desired_dir[0] = mm.obstacle_normal[1];
					mm.desired_dir[1] = -mm.obstacle_normal[0];
				}
				MmoveSingle(&mm);
				if (mm.hit_obstacle)
				{
					m_WallDir = 0;
					m_iWallDodgeTimeout = 0;
				}
				GetMoveInfo(&mm);
				return;
			}
		}
	}

	m_iWallDodgeTimeout = level.inttime + 1000;

	{
		auto dir = m_Path.CurrentDelta();
		auto pNextNode = m_Path.NextNode();

		if (pNextNode)
		{
			dir = pNextNode->dir;
		}

		vec2_t vDelta;
		vDelta[0] = pNextNode->point[1] - mm.hit_origin[0];
		vDelta[1] = pNextNode->point[0] - mm.hit_origin[1];

		if (vDelta[0] * dir[1] <= vDelta[1] * dir[0])
		{
			mm.desired_dir[0] = mm.obstacle_normal[1];
			mm.desired_dir[1] = -mm.obstacle_normal[0];
			m_WallDir = 1;
		}
		else
		{
			mm.desired_dir[0] = -mm.obstacle_normal[1];
			mm.desired_dir[1] = mm.obstacle_normal[0];
			m_WallDir = -1;
		}
		MmoveSingle(&mm);
		if (mm.hit_obstacle)
		{
			VectorCopy(velocity, mm.velocity);
			VectorCopy(origin, mm.origin);
			mm.groundPlane = m_groundPlane;
			mm.walking = m_walking;
			VectorCopy(m_groundPlaneNormal, mm.groundPlaneNormal);

			VectorCopy2D(mm.obstacle_normal, m_PrevObstacleNormal);

			if (m_WallDir == -1)
			{
				mm.desired_dir[0] = -mm.obstacle_normal[1];
				mm.desired_dir[1] = mm.obstacle_normal[0];
			}
			else
			{
				mm.desired_dir[0] = mm.obstacle_normal[1];
				mm.desired_dir[1] = -mm.obstacle_normal[0];
			}
			MmoveSingle(&mm);
		}
	}
	GetMoveInfo(&mm);
}

void Actor::MovePathGoal
	(
	float fMoveSpeed
	)

{
	float fTimeToGo, fDeltaSquareLen, fSlowdownSpeed;
	vec2_t vDelta;

	if (!m_Path.HasCompleteLookahead())
	{
		MovePath(fMoveSpeed);
		m_fPathGoalTime = 0;
		return;
	}
	fTimeToGo = m_fPathGoalTime - level.time;
	VectorCopy2D(origin, vDelta);

	VectorSub2D(vDelta, m_Path.LastNode()->point, vDelta);

	fDeltaSquareLen = VectorLength2DSquared(vDelta);
	if (fTimeToGo <= -0.001)
	{
		if (fDeltaSquareLen < (fMoveSpeed * fMoveSpeed * 0.0625))
		{
			fTimeToGo = 0.5;
			m_fPathGoalTime = level.time + fTimeToGo;
			if (m_csPathGoalEndAnimScript == STRING_EMPTY)
			{
				m_csPathGoalEndAnimScript = STRING_ANIM_IDLE_SCR;
			}
			m_bStartPathGoalEndAnim = true;
		}
		else
		{
			MovePath(fMoveSpeed);
			m_fPathGoalTime = 0;
			return;
		}
	}
	fSlowdownSpeed = sqrt(fDeltaSquareLen) * (2 / (fTimeToGo + level.frametime));
	if (fSlowdownSpeed > fMoveSpeed + 0.001 && fSlowdownSpeed > 0.4 * sv_runspeed->value)
	{
		m_fPathGoalTime = 0;
		m_eNextAnimMode = 3;
		m_csNextAnimString = NULL;
		m_NextAnimLabel = m_Anim;
		m_bNextForceStart = true;
		MovePath(fMoveSpeed);
	}
	else
	{
		MovePath(fSlowdownSpeed);
		if (level.time >= m_fPathGoalTime)
			m_eAnimMode = 1;
	}
}

void Actor::Dumb
	(
	Event *ev
	)

{
	Event e1(EV_Listener_ExecuteScript);
	e1.AddConstString(STRING_GLOBAL_DISABLE_AI_SCR);
	ExecuteScript(&e1);
}

void Actor::PhysicsOn
	(
	Event *ev
	)

{
	m_bDoPhysics = true;
}

void Actor::PhysicsOff
	(
	Event *ev
	)

{
	m_bDoPhysics = false;
}

void Actor::EventStart
	(
	Event *ev
	)

{
	ResolveVoiceType();
	setSize(MINS, MAXS);//notsure
	//FIXME: macro
	droptofloor(16384);// a power of 2
	
	SetControllerTag( HEAD_TAG, gi.Tag_NumForName( edict->tiki, "Bip01 Head" ) );
	SetControllerTag( TORSO_TAG, gi.Tag_NumForName( edict->tiki, "Bip01" ) );
	SetControllerTag( ARMS_TAG, gi.Tag_NumForName( edict->tiki, "Bip01 L UpperArm" ) );

	JoinNearbySquads();

	if (level.Spawned())
	{
		Unregister(STRING_SPAWN);
	}
}

void Actor::EventGetMood
	(
	Event *ev
	)

{
	ev->AddConstString(m_csMood);
}

void Actor::EventSetMood
	(
	Event *ev
	)

{
	m_csMood = ev->GetConstString(1);
	if (m_csMood > STRING_ALERT)
	{
		m_csMood = STRING_BORED;
		ScriptError("invalid mood - setting to bored");
	}
}

void Actor::EventGetAngleYawSpeed
	(
	Event *ev
	)

{
	ev->AddFloat(m_fAngleYawSpeed);
}

void Actor::EventSetAngleYawSpeed
	(
	Event *ev
	)

{
	float speed = ev->GetFloat(1);
	if (speed <= 0)
	{
		ScriptError("Attempting to set illegal turnspeed %f", speed);
	}
	m_fAngleYawSpeed = speed;
}

void Actor::EventSetAimTarget
	(
	Event *ev
	)

{
	Weapon *weap = GetActiveWeapon(WEAPON_MAIN);
	weap->SetAimTarget(ev->GetEntity(1));
}

void Actor::UpdateAngles
	(
	void
	)

{
	float max_change, error, dist;
	vec3_t new_angles;

	if (!m_YawAchieved)
	{

		error = m_DesiredYaw - angles[1];
		if (error <= 180)
		{
			if (error < -180.0)
				error += 360;
		}
		else
		{
			error -= 360;
		}

		dist = m_fAngleYawSpeed * level.frametime;
		if (-dist <= error)
		{
			max_change = dist;
			if (error <= dist)
			{
				max_change = error;
				m_YawAchieved = true;
			}
		}
		else
		{
			max_change = -dist;
		}
		new_angles[0] = angles[0] + max_change;
		new_angles[1] = angles[1] + max_change;
		new_angles[2] = angles[2] + max_change;
		setAngles(new_angles);
	}
}

void Actor::SetLeashHome
(
	Vector vHome
)

{
	if (!m_bFixedLeash)
	{
		m_vHome = vHome;
	}
}

void Actor::AimAtTargetPos
	(
	void
	)

{
	// not found in ida
}

void Actor::AimAtAimNode
	(
	void
	)

{
	Vector vDir = mTargetPos - EyePosition() + Vector( 0, 0, 16 );
	SetDesiredLookDir( vDir );
	m_DesiredGunDir[ 0 ] = 360.0f - vDir.toPitch();
	m_DesiredGunDir[ 1 ] = vDir.toYaw();

	m_YawAchieved = false;
	m_DesiredYaw = vDir.toYaw();
}

void Actor::AimAtEnemyBehavior
	(
	void
	)

{
	Vector v;
	m_bHasDesiredLookAngles = true;
	
	v = mTargetPos - EyePosition();

	vectoangles(v, m_DesiredLookAngles);
	m_DesiredLookAngles[0] = AngleNormalize180(m_DesiredLookAngles[0]);
	m_DesiredLookAngles[1] = AngleNormalize180(m_DesiredLookAngles[1] - angles[1]);

	m_DesiredGunDir.x = v.toPitch();
	m_DesiredGunDir.y = v.toYaw();
	m_DesiredGunDir.z = 0;
	
	m_YawAchieved = false;

	m_DesiredYaw = v.toYaw();

	Anim_Aim();
}

void Actor::FaceMotion
	(
	void
	)

{
	vec3_t dir;
	Vector delta;
	velocity.copyTo(dir);

	if (VectorLength2DSquared(dir) > 1)
	{
		delta = origin - m_vOriginHistory[m_iCurrentHistory];

		if (delta.lengthXY(true) >= 1)
		{
			if (dir[1] *delta[0] + dir[0] * delta[1] > 0)
			{
				delta.copyTo(dir);
			}
		}
		if (m_ThinkState == 1)
		{
			IdleLook(dir);
		}
		else
		{
			m_bHasDesiredLookAngles = true;

			vectoangles(dir, m_DesiredLookAngles);
			m_DesiredLookAngles[0] = AngleNormalize180(m_DesiredLookAngles[0]);
			m_DesiredLookAngles[1] = AngleNormalize180(m_DesiredLookAngles[1] - angles[1]);
		}
		m_YawAchieved = false;
		m_DesiredYaw = vectoyaw(dir);
	}
	else if (m_ThinkState == 1 && m_pLookEntity)
	{
		LookAtLookEntity();
	}
	else
	{
		m_bHasDesiredLookAngles = false;
	}
}

void Actor::FaceDirectionDuringMotion
	(
	vec3_t vLook
	)

{
	m_bHasDesiredLookAngles = true;
	

	vectoangles(vLook, m_DesiredLookAngles);

	m_DesiredLookAngles[0] = AngleNormalize180(m_DesiredLookAngles[0]);
	m_DesiredLookAngles[1] = AngleNormalize180(m_DesiredLookAngles[1] = angles[1]);

	if (velocity[0] != 0 || velocity[1] != 0)
	{
		if (velocity[1] == 0)
		{
			if (velocity[0] == 0)
			{
			}
		}
	}

	m_YawAchieved = false;
	m_DesiredYaw = vectoyaw(vLook);
	// FIXME: stub
	STUB();
}

float Actor::PathDistanceAlongVector
	(
	vec3_t vDir
	)

{
	Vector vDelta = m_Path.CurrentDelta();

	return DotProduct2D( vDelta, vDir );
}

void Actor::FaceEnemyOrMotion
	(
	int iTimeIntoMove
	)

{
	Vector vDelta = origin - m_vLastEnemyPos;
	vDelta.z = 0;
	if (iTimeIntoMove > 999)
	{
		if (m_Path.CurrentNodeIndex() != m_sCurrentPathNodeIndex)
		{
			float fDot = DotProduct2D(vDelta, m_Path.CurrentDelta());
			m_bFaceEnemy = fDot <= 0 || vDelta.lengthXY(true) * 4096 >= fDot * fDot;

			m_sCurrentPathNodeIndex = m_Path.CurrentNodeIndex();
		}
	}
	else
	{
		m_bFaceEnemy = true;
	}

	if (m_bFaceEnemy && (vDelta.x >= 15.0 || vDelta.x <= -15.0 || vDelta.y >= 15.0 || vDelta.y <= -15.0))
	{
		vDelta.x = -vDelta.x;
		vDelta.y = -vDelta.y;
		FaceDirectionDuringMotion(vDelta);
	}
	else
	{
		FaceMotion();
	}
}

int Actor::NextUpdateTime
	(
	int iLastUpdateTime,
	int iUpdatePeriod
	)

{
	int i = iLastUpdateTime;

	do
		i += iUpdatePeriod;
	while( i < level.inttime );

	return i;
}

void Actor::ResetBoneControllers
	(
	void
	)

{

	if (edict->s.bone_tag[HEAD_TAG] != -1)
	{
		
		SetControllerAngles(HEAD_TAG, vec_zero);
	}
	if (edict->s.bone_tag[TORSO_TAG] != -1)
	{
		SetControllerAngles(TORSO_TAG, vec_zero);
	}
	if (edict->s.bone_tag[ARMS_TAG] != -1)
	{
		SetControllerAngles(ARMS_TAG, vec_zero);
	}
}

void Actor::UpdateBoneControllers
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::ReadyToFire
	(
	Event *ev
	)

{
	bool ready;

	Weapon *weap = GetActiveWeapon(WEAPON_MAIN);
	if (weap)
		ready = weap->ReadyToFire(FIRE_PRIMARY, true);
	else
		ready = 0;
	ev->AddInteger(ready);
}

void Actor::GetLocalYawFromVector
	(
	Event *ev
	)

{
	long double yaw;
	Vector vec;

	vec = ev->GetVector(1);
	yaw = vectoyaw(vec) + 180.0;
	AngleSubtract(yaw, angles[1]);
	ev->AddFloat(yaw);
}

void Actor::EventGetSight
	(
	Event *ev
	)

{
	ev->AddFloat(m_fSight);
}

void Actor::EventSetSight
	(
	Event *ev
	)

{
	m_fSight = ev->GetFloat(1);
}

void Actor::EventGetHearing
	(
	Event *ev
	)

{
	ev->AddFloat(m_fHearing);
}

void Actor::EventSetHearing
	(
	Event *ev
	)

{
	m_fHearing = ev->GetFloat(1);
}

void Actor::ClearPatrolCurrentNode
	(
	void
	)

{
	if (m_patrolCurrentNode)
	{
		if (m_patrolCurrentNode->IsSubclassOfAnimate())
		{
			delete m_patrolCurrentNode;
		}
		m_patrolCurrentNode = NULL;
	}
}

void Actor::NextPatrolCurrentNode
	(
	void
	)

{
	if (this->m_bScriptGoalValid)
	{
		m_bScriptGoalValid = m_patrolCurrentNode->origin != m_vScriptGoal;
	}
	if (m_patrolCurrentNode->IsSubclassOfTempWaypoint())
	{
		ClearPatrolCurrentNode();
	}
	else
	{
		m_patrolCurrentNode = m_patrolCurrentNode->Next();
	}
}

void Actor::SetPatrolCurrentNode
	(
	Vector& vec
	)

{
	if (m_patrolCurrentNode)
	{
		if (m_patrolCurrentNode->IsSubclassOfTempWaypoint())
		{
			delete m_patrolCurrentNode;
		}
		m_patrolCurrentNode = NULL;
	}
	TempWaypoint * twp = new TempWaypoint();

	if (m_patrolCurrentNode != twp)
	{
		m_patrolCurrentNode = twp;
	}
	m_patrolCurrentNode->setOrigin(vec);

}

void Actor::SetPatrolCurrentNode
	(
	Listener *l
	)

{
	if (m_patrolCurrentNode)
	{
		if (m_patrolCurrentNode->IsSubclassOfTempWaypoint())
		{
			delete m_patrolCurrentNode;
		}
		m_patrolCurrentNode = NULL;
	}

	if (!l->isInheritedBy(&SimpleEntity::ClassInfo))
	{
		ScriptError("Bad patrol path with classname '%s' specified for '%s' at (%f %f %f)\n",
			l->getClassname(),
			targetname.c_str(),
			origin.x,
			origin.y,
			origin.z
			);
	}

	if (m_patrolCurrentNode != l)
	{
		m_patrolCurrentNode = (SimpleEntity *)l;
	}
}

void Actor::EventSetPatrolPath
	(
	Event *ev
	)

{
	SetPatrolCurrentNode(ev->GetListener(1));
}

void Actor::EventGetPatrolPath
	(
	Event *ev
	)

{
	ev->AddListener(m_patrolCurrentNode);
}

void Actor::EventSetPatrolWaitTrigger
	(
	Event *ev
	)

{
	m_bPatrolWaitTrigger = ev->GetBoolean(1);
}

void Actor::EventGetPatrolWaitTrigger
	(
	Event *ev
	)

{
	ev->AddInteger(m_bPatrolWaitTrigger);
}

void Actor::ShowInfo_PatrolCurrentNode
	(
	void
	)

{

	if (m_patrolCurrentNode)
	{
		Com_Printf("current patrol node: %s\n", m_patrolCurrentNode->targetname.c_str());
	}
	else
	{
		Com_Printf("no current patrol node\n");
	}
}

bool Actor::MoveOnPathWithSquad
	(
	void
	)

{
	float fDistSquared, fIntervalSquared;
	vec2_t vMyNormalDir, vDelta, vHisDir;
	Player *p;

	//FIXMEL: macro
	if (level.inttime < m_iSquadStandTime + 500 || !PathExists() || PathComplete())
	{
		return false;
	}
	fIntervalSquared = m_fInterval * m_fInterval;
	if (m_iSquadStandTime)
	{
		fIntervalSquared *= 2;
	}

	p = (Player *)G_GetEntity(0);
	
	if (IsTeamMate(p))
	{
		VectorSub2D(p->origin, origin, vDelta);
		fDistSquared = VectorLength2DSquared(vDelta);
		if (fIntervalSquared > fDistSquared && DotProduct2D(vDelta, PathDelta()) > 0)
		{
			VectorSub2D(velocity, p->velocity, vMyNormalDir);
			VectorNormalize2D(vMyNormalDir);
			if (fDistSquared - DotProduct2D(vDelta, vMyNormalDir) < 2304)
			{
				m_iSquadStandTime = level.inttime;
				return false;
			}
		}
	}

	for (Actor *pSquadMate = (Actor *)m_pNextSquadMate.Pointer(); pSquadMate != this; pSquadMate = (Actor *)pSquadMate->m_pNextSquadMate.Pointer())
	{
		if (!pSquadMate->IsSubclassOfActor())
		{
			continue;
		}
		VectorSub2D(pSquadMate->origin, origin, vDelta);
		fDistSquared = VectorLength2DSquared(vDelta);
		if (fDistSquared >= fIntervalSquared || DotProduct2D(vDelta, PathDelta()) <= 0)
		{
			
			continue;
		}
		if (pSquadMate->PathExists() || pSquadMate->PathComplete())
		{
			VectorCopy2D(pSquadMate->velocity, vHisDir);
			if (VectorLength2DSquared(vHisDir) <= 8)
			{
				continue;
			}
		}
		else
		{
			VectorCopy2D(pSquadMate->PathDelta(), vHisDir);

		}

		if (DotProduct2D(PathDelta(), vHisDir) >= 0 && (entnum == pSquadMate->entnum || DotProduct2D(vHisDir, vDelta) >= 0))
		{
			m_iSquadStandTime = level.inttime;
			return false;
		}
	}
	m_iSquadStandTime = 0;
	return true;
}

bool Actor::MoveToWaypointWithPlayer
	(
	void
	)

{
	float fIntervalSquared, fDistSquared;
	vec2_t vMyNormalDir, vDelta, vMyDir;

	if (level.inttime < m_iSquadStandTime + 500)
	{
		return false;
	}
	
	fIntervalSquared = m_fInterval * m_fInterval;
	if (m_iSquadStandTime)
	{
		fIntervalSquared += fIntervalSquared;
	}

	Player *p = (Player *)G_GetEntity(0);
	if (IsTeamMate(p))
	{
		vDelta[0] = p->origin[0] - origin[0];
		vDelta[1] = p->origin[1] - origin[1];
		fDistSquared = VectorLength2DSquared(vDelta);

		if (fDistSquared >= fIntervalSquared)
		{
			m_iSquadStandTime = 0;
			return true;
		}

		vMyDir[0] = m_patrolCurrentNode->origin[0] - origin[0];
		vMyDir[1] = m_patrolCurrentNode->origin[1] - origin[1];
		if (DotProduct2D(vDelta, vMyDir) < 0)
		{
			m_iSquadStandTime = 0;
			return true;
		}

		vMyNormalDir[0] = velocity[0] - p->velocity[0];
		vMyNormalDir[1] = velocity[1] - p->velocity[1];
		VectorNormalize2D(vMyNormalDir);

		if (fDistSquared - DotProduct2D(vDelta, vMyNormalDir) > 2304)
		{
			m_iSquadStandTime = 0;
			return true;
		}
		m_iSquadStandTime = level.inttime;
	}
	return false;
}

bool Actor::PatrolNextNodeExists
	(
	void
	)

{
	return m_patrolCurrentNode && !( m_patrolCurrentNode->IsSubclassOfTempWaypoint() ) && m_patrolCurrentNode->Next();
}

void Actor::UpdatePatrolCurrentNode
	(
	void
	)

{
	bool next = false;
	if (m_patrolCurrentNode)
	{
		if (m_patrolCurrentNode->IsSubclassOfTempWaypoint())
		{
			if (fabs(origin[0] - m_patrolCurrentNode->origin[0]) < 16.0
				&& fabs(origin[1] - m_patrolCurrentNode->origin[1]) < 16.0)
			{
				next = true;
			}
		}
		else if(PathExists() && PathComplete())
		{

			next = true;
		}

	}
	if (next)
	{
		NextPatrolCurrentNode();
	}
}

bool Actor::MoveToPatrolCurrentNode
	(
	void
	)

{
	UpdatePatrolCurrentNode();
	if (m_patrolCurrentNode && !m_bPatrolWaitTrigger)
	{
		if (m_patrolCurrentNode->IsSubclassOfWaypoint())
		{
			if (MoveToWaypointWithPlayer())
			{
				m_bNextForceStart = false;
				m_eNextAnimMode = 4;
				m_csNextAnimString = m_csPatrolCurrentAnim;
				FaceMotion();
			}
			else
			{
				Anim_Stand();
				if (m_pLookEntity)
					LookAtLookEntity();
				else
					m_bHasDesiredLookAngles = false;
			}
			m_Dest[0] = m_patrolCurrentNode->origin[0];
			m_Dest[1] = m_patrolCurrentNode->origin[1];
			m_Dest[2] = m_patrolCurrentNode->origin[2];
			if (m_fMoveDoneRadiusSquared == 0.0 || m_patrolCurrentNode->Next())
				return false;
			Vector delta = m_patrolCurrentNode->origin - origin;
			return m_fMoveDoneRadiusSquared >= delta.lengthXY(true);
		}
		else
		{
			SimpleActor::SetPath(
				m_patrolCurrentNode->origin,
				"Actor::MoveToPatrolCurrentNode",
				0,
				0,
				0);
			if (!PathExists())
			{
				if (m_pLookEntity)
					LookAtLookEntity();
				else
					m_bHasDesiredLookAngles = false;
				Anim_Idle();
				parm.movefail = qtrue;
				return false;
			}
			if (MoveOnPathWithSquad())
			{
				const_str csAnimString;
				if (PatrolNextNodeExists())
				{
					csAnimString = this->m_csPatrolCurrentAnim;
					m_eNextAnimMode = 2;
				}
				else
				{
					csAnimString = this->m_csPatrolCurrentAnim;
					m_eNextAnimMode = 3;
				}
				m_csNextAnimString = csAnimString;
				m_bNextForceStart = true;
				FaceMotion();
			}
			else
			{
				Anim_Stand();
				if (m_pLookEntity)
					LookAtLookEntity();
				else
					m_bHasDesiredLookAngles = false;
			}

			if (m_fMoveDoneRadiusSquared == 0)
				return false;

			if (PathComplete())
			{
				return true;
			}

			if (!m_Path.HasCompleteLookahead() || m_patrolCurrentNode->Next())
			{
				return false;
			}
			Vector delta = PathDelta();
			return m_fMoveDoneRadiusSquared >= delta.lengthXY(true);
		}
	}


	if (m_pLookEntity)
		LookAtLookEntity();
	else
		m_bHasDesiredLookAngles = false;
	Anim_Idle();
	return false;
}

void Actor::ClearAimNode
	(
	void
	)

{
	if (m_aimNode)
	{

		if (m_aimNode->IsSubclassOfTempWaypoint())
		{
			delete m_aimNode;
		}
		m_aimNode = NULL;
	}
}

void Actor::SetAimNode
	(
	Vector& vec
	)

{
	ClearAimNode();

	TempWaypoint * twp = new TempWaypoint();
	m_aimNode = twp;

	m_aimNode->setOrigin(vec);
}

void Actor::SetAimNode
	(
	Listener *l
	)

{
	ClearAimNode();
	if (l)
	{
		if (!l->isInheritedBy(&SimpleEntity::ClassInfo))
		{
			ScriptError(
				"Bad aim node with classname '%s' specified for '%s' at (%f %f %f)\n",
				l->getClassname(),
				targetname.c_str(),
				origin.x,
				origin.y,
				origin.z);
		}
		m_aimNode = (SimpleEntity *)l;
	}
}

void Actor::ShowInfo_AimNode
	(
	void
	)

{
	if (m_aimNode)
	{
		Com_Printf("aim node: %s\n", m_aimNode->targetname.c_str());
	}
	else
	{
		Com_Printf("no current aim node\n");
	}
}

void Actor::EventSetAccuracy
	(
	Event *ev
	)

{
	mAccuracy = ev->GetFloat(1) / 100;
}

void Actor::EventGetAccuracy
	(
	Event *ev
	)

{
	ev->AddFloat(mAccuracy * 100);
}

int Actor::GetThinkType
	(
	const_str csName
	)

{
	int result = 0;
	while (m_csThinkNames[result] != csName)
	{
		if (++result > 34)
		{
			ScriptError("unknown think type '%s'", Director.GetString(csName).c_str());
		}
	}
	return result;
}

void Actor::SetThink
	(
	int state,
	int think
	)

{
	m_ThinkMap[state] = think;
	if (m_ThinkState == state)
		m_bDirtyThinkState = true;
}

void Actor::SetThinkIdle
	(
	int think_idle
	)

{
	//FIXME: macros
	int think_curious;

	switch (think_idle)
	{
	case 3:
	case 4:
		m_bBecomeRunner = true;
		think_curious = 9;
		break;
	case 17:
		think_curious = 17;
		break;
	case 18:
		think_curious = 20;
		m_ThinkMap[7] = 20;
		if (m_ThinkState == 7)
			m_bDirtyThinkState = true;
		break;
	case 22:
		think_curious = 23;
		break;
	case 25:
		think_curious = 26;
		break;
	default:
		think_curious = 9;
		break;
	}


	m_ThinkMap[1] = think_idle;
	if (m_ThinkState == 1)
		m_bDirtyThinkState = true;

	m_ThinkMap[5] = think_curious;
	if (m_ThinkState == 5)
		m_bDirtyThinkState = true;
}

void Actor::SetThinkState
	(
	int state,
	int level
	)

{
	//FIXME: macros
	if (state == THINKSTATE_ATTACK)
	{
		m_csIdleMood = STRING_NERVOUS;
		if (m_ThinkMap[4] != 15 && m_ThinkMap[4] != 32 && m_ThinkMap[4] != 19 && !GetWeapon(WEAPON_MAIN))
		{
			Com_Printf(
				"^~^~^ LD ERROR: (entnum %i, radnum %i, targetname '%s'):    forcing weaponless attack state.\n"
				"^~^~^ Level designers should specify 'type_attack weaponless' for this guy.\n",
				entnum,
				radnum,
				targetname.c_str());
			m_ThinkMap[4] = 32;
			if (m_ThinkState == THINKSTATE_ATTACK)
				m_bDirtyThinkState = true;
		}
	}

	m_ThinkStates[level] = state;

	if (m_ThinkLevel > level)
	{
		m_bDirtyThinkState = true;
	}

}

void Actor::EndCurrentThinkState
	(
	void
	)

{
	SetThinkState(THINKSTATE_NONE, m_ThinkState);
}

void Actor::ClearThinkStates
	(
	void
	)

{
	for (int i = 0; i <= MAX_THINKSTATES-1; i++)
	{
		SetThinkState(THINKSTATE_NONE, i);
	}
}

int Actor::CurrentThink
	(
	void
	) const

{
	return m_Think[ m_ThinkLevel ];
}

bool Actor::IsAttacking
	(
	void
	) const

{
	return m_ThinkStates[ 0 ] == THINKSTATE_ATTACK;
}

void Actor::EventGetFov
	(
	Event *ev
	)

{
	ev->AddFloat(m_fFov);
}

void Actor::EventSetFov
	(
	Event *ev
	)

{
	float fov = ev->GetFloat(1);
	if (fov < 0 || fov > 360)
	{
		ScriptError("fov must be in the range [0,360]");
	}
	m_fFov = fov;
	m_fFovDot = cos(0.5 * fov * M_PI / 180.0);
}

void Actor::EventSetDestIdle
	(
	Event *ev
	)

{
	// not found in ida
}

void Actor::EventSetDestIdle2
	(
	Event *ev
	)

{
	// not found in ida
}

void Actor::EventSetTypeIdle
	(
	Event *ev
	)

{
	int think = GetThinkType(ev->GetConstString(1));
	if (!Actor::GlobalFuncs[think].IsState(THINKSTATE_IDLE))
	{
		SetThinkIdle(8);
		ScriptError("Invalid idle think '%s'", Director.GetString(m_csThinkNames[think]).c_str());
	}

	SetThinkIdle(think);
}

void Actor::EventGetTypeIdle
	(
	Event *ev
	)

{
	//FIXME: helpful to understand thinkmaps
	ev->AddConstString(m_csThinkNames[m_ThinkMap[1]]);
}

void Actor::EventSetTypeAttack
	(
	Event *ev
	)

{
	int think = GetThinkType(ev->GetConstString(1));
	if (!Actor::GlobalFuncs[think].IsState(THINKSTATE_ATTACK))
	{
		m_ThinkMap[THINKSTATE_ATTACK] = 1;
		if (m_ThinkState == THINKSTATE_ATTACK)
		{
			m_bDirtyThinkState = true;
		}
		ScriptError("Invalid attack think '%s'", Director.GetString(m_csThinkNames[think]).c_str());
	}
	m_ThinkMap[THINKSTATE_ATTACK] = think;
	if (m_ThinkState == THINKSTATE_ATTACK)
	{
		m_bDirtyThinkState = true;
	}
}

void Actor::EventGetTypeAttack
	(
	Event *ev
	)

{
	//FIXME: helpful to understand thinkmaps
	ev->AddConstString(m_csThinkNames[m_ThinkMap[4]]);
}

void Actor::EventSetTypeDisguise
	(
	Event *ev
	)

{
	int think = GetThinkType(ev->GetConstString(1));
	if (!Actor::GlobalFuncs[think].IsState(THINKSTATE_DISGUISE))
	{
		m_ThinkMap[THINKSTATE_DISGUISE] = 10;
		if (m_ThinkState == THINKSTATE_DISGUISE)
		{
			m_bDirtyThinkState = true;
		}
		ScriptError("Invalid disguise think '%s'", Director.GetString(m_csThinkNames[think]).c_str());
	}
	m_ThinkMap[THINKSTATE_DISGUISE] = think;
	if (m_ThinkState == THINKSTATE_DISGUISE)
	{
		m_bDirtyThinkState = true;
	}
}

void Actor::EventGetTypeDisguise
	(
	Event *ev
	)

{
	ev->AddConstString(m_csThinkNames[m_ThinkMap[6]]);
}

void Actor::EventSetDisguiseLevel
	(
	Event *ev
	)

{
	m_iDisguiseLevel = ev->GetInteger(1);
	if (m_iDisguiseLevel -1 > 1)
	{
		m_iDisguiseLevel = 1;
		ScriptError("bad disguise level %d for %s, setting to 1\n", m_iDisguiseLevel, targetname.c_str());
	}
}

void Actor::EventGetDisguiseLevel
	(
	Event *ev
	)

{
	ev->AddInteger(m_iDisguiseLevel);
}

void Actor::EventSetTypeGrenade
	(
	Event *ev
	)

{
	//FIXME: not sure of m_ThinkMap[THINKSTATE_GRENADE]
	int think = GetThinkType(ev->GetConstString(1));
	if (!(&GlobalFuncs[think])->IsState(THINKSTATE_GRENADE))
	{
		//FIXME: macro
		m_ThinkMap[THINKSTATE_GRENADE] = 16;
		if (m_ThinkState == THINKSTATE_GRENADE)
		{
			m_bDirtyThinkState = true;
		}
		;
		ScriptError("Invalid grenade think '%s'", Director.GetString(m_csThinkNames[m_ThinkMap[THINKSTATE_GRENADE]]).c_str());
	}
	
	m_ThinkMap[THINKSTATE_GRENADE] = think;

	if (m_ThinkState == THINKSTATE_GRENADE)
	{
		m_bDirtyThinkState = true;
	}
}

void Actor::EventGetTypeGrenade
	(
	Event *ev
	)

{
	ev->AddConstString(m_csThinkNames[m_ThinkMap[THINKSTATE_GRENADE]]);
}

void Actor::EventSetMinDistance
	(
	Event *ev
	)

{
	m_fMinDistance = ev->GetFloat(1);

	if (m_fMinDistance < 0)
	{
		m_fMinDistance = 0;
		Com_Printf(
			"^~^~^ Warning: mindist negative, forcing to %g for entity %i, targetname '%s'\n",
			m_fMinDistance,
			entnum,
			targetname.c_str());
	}
	m_fMinDistanceSquared = m_fMinDistance * m_fMinDistance;
}

void Actor::EventGetMinDistance
	(
	Event *ev
	)

{
	ev->AddFloat(m_fMinDistance);
}

void Actor::EventSetMaxDistance
	(
	Event *ev
	)

{
	m_fMaxDistance = ev->GetFloat(1);
	//FIXME: macro
	if (m_fMaxDistance < 256)
	{
		m_fMaxDistance = 256;
		Com_Printf(
			"^~^~^ Warning: maxdist too small, forcing to %g for entity %i, targetname '%s'\n",
			m_fMaxDistance,
			entnum,
			targetname.c_str());
	}
	m_fMaxDistanceSquared = m_fMaxDistance * m_fMaxDistance;
}

void Actor::EventGetMaxDistance
	(
	Event *ev
	)

{
	ev->AddFloat(m_fMaxDistance);
}

void Actor::EventSetLeash
	(
	Event *ev
	)

{
	m_fLeash = ev->GetFloat(1);
	m_fLeashSquared = m_fLeash * m_fLeash;
}

void Actor::EventGetLeash
(
	Event *ev
)

{
	ev->AddFloat(m_fLeash);
}

void Actor::EventSetInterval
	(
	Event *ev
	)

{
	m_fInterval = ev->GetFloat(1);
}

void Actor::EventDistToEnemy
	(
	Event *ev
	)

{
	float dist = 0;
	if (m_Enemy)
	{
		Vector distV = origin - m_Enemy->origin;
		dist = sqrt(DotProduct(distV, distV));
	}
}

void Actor::EventGetInterval
(
	Event *ev
)

{
	ev->AddFloat(m_fInterval);
}

void Actor::EventGetRunAnim
	(
	Event *ev
	)

{
	ev->AddConstString(GetRunAnim());
}

void Actor::EventGetWalkAnim
	(
	Event *ev
	)

{
	ev->AddConstString(GetWalkAnim());
}

void Actor::EventSetAnimName
	(
	Event *ev
	)

{
	m_csAnimName = ev->GetConstString(1);
}

void Actor::EventGetAnimName
(
	Event *ev
)

{
	ev->AddConstString(m_csAnimName);
}

void Actor::EventSetDisguiseRange
	(
	Event *ev
	)

{
	float range = ev->GetFloat(1);
	m_fMaxDisguiseDistSquared = range * range;
}

void Actor::EventGetDisguiseRange
	(
	Event *ev
	)

{
	ev->AddFloat(sqrt(m_fMaxDisguiseDistSquared));
}

void Actor::EventSetDisguisePeriod
	(
	Event *ev
	)

{
	m_iDisguisePeriod = ev->GetFloat(1) * 1000 + 0.5;
}

void Actor::EventGetDisguisePeriod
	(
	Event *ev
	)

{
	ev->AddFloat(m_iDisguisePeriod / 1000.0);
}

void Actor::EventSetDisguiseAcceptThread
	(
	Event *ev
	)

{

	if (ev->IsFromScript())
	{
		m_DisguiseAcceptThread.SetThread(ev->GetValue());
	}
	else
	{

		m_DisguiseAcceptThread.Set(ev->GetString(1));
	}
}

void Actor::EventGetDisguiseAcceptThread
	(
	Event *ev
	)

{
	m_DisguiseAcceptThread.GetScriptValue(&ev->GetValue());
}

void Actor::EventAttackPlayer
	(
	Event *ev
	)

{
	Player *pPlayer = (Player *)G_GetEntity(0);
	if (!pPlayer)
	{
		ScriptError("player doesn't exist");
	}
	else
	{
		m_PotentialEnemies.ConfirmEnemy(this, pPlayer);
		m_bForceAttackPlayer = true;
	}
}

void Actor::ForceAttackPlayer
	(
	void
	)

{
	m_PotentialEnemies.ConfirmEnemy(this, (Sentient *)G_GetEntity(0));
}

void Actor::EventSetAlarmNode
	(
	Event *ev
	)

{
	Listener *l = ev->GetListener(1);
	if (l)
	{
		if (!l->isInheritedBy(&SimpleEntity::ClassInfo))
		{
			ScriptError("Alarm node must be an entity");
		}
	}
	m_AlarmNode = (SimpleEntity *)l;
}

void Actor::EventGetAlarmNode
	(
	Event *ev
	)

{
	ev->AddListener(m_AlarmNode);
}

void Actor::EventSetAlarmThread
	(
	Event *ev
	)

{
	if (ev->IsFromScript())
	{
		m_AlarmThread.SetThread(ev->GetValue());
	}
	else
	{

		m_AlarmThread.Set(ev->GetString(1));
	}
}

void Actor::EventGetAlarmThread
	(
	Event *ev
	)

{
	
	m_AlarmThread.GetScriptValue(&ev->GetValue());
}

void Actor::EventSetSoundAwareness
	(
	Event *ev
	)

{
	m_fSoundAwareness = ev->GetFloat(1);
}

void Actor::EventGetSoundAwareness
	(
	Event *ev
	)

{
	ev->AddFloat(m_fSoundAwareness);
}

void Actor::EventSetGrenadeAwareness
	(
	Event *ev
	)

{
	m_fGrenadeAwareness = ev->GetFloat(1);
}

void Actor::EventGetGrenadeAwareness
	(
	Event *ev
	)

{
	ev->AddFloat(m_fGrenadeAwareness);
}

str Actor::ThinkName
	(
	void
	) const

{
	return Director.GetString( m_Think[ m_ThinkLevel ] );
}

str Actor::ThinkStateName
	(
	void
	) const

{
	return Director.GetString( m_ThinkState );
}

void Actor::EventSetTurret
	(
	Event *ev
	)

{
	Listener *l = ev->GetListener(1);
	if (l->isInheritedBy(&TurretGun::ClassInfo))
	{
		m_pTurret = (TurretGun *)l;
	}
}

void Actor::EventGetTurret
	(
	Event *ev
	)

{
	ev->AddListener(m_pTurret);
}

void Actor::EventEnableEnemy
	(
	Event *ev
	)

{
	m_bDesiredEnableEnemy = ev->GetInteger(1) != true;
}

void Actor::EventEnablePain
	(
	Event *ev
	)

{
	m_bEnablePain = ev->GetInteger(1) != true;
}

void Actor::EventActivate
	(
	Event *ev
	)

{
	m_bPatrolWaitTrigger = false;
	Unregister(STRING_TRIGGER);
}

void Actor::EventGetAmmoGrenade
	(
	Event *ev
	)

{
	ev->AddInteger(AmmoCount("grenade"));
}

void Actor::EventSetAmmoGrenade
	(
	Event *ev
	)

{
	GiveAmmo("grenade", ev->GetInteger(1));
}

void Actor::EventInterruptPoint
	(
	Event *ev
	)

{

	GlobalFuncs_t *func = &GlobalFuncs[m_Think[m_ThinkLevel]];
	
	if (func->PostShoot)
		(this->*func->PostShoot)();
}

void Actor::EventAnimScript
	(
	Event *ev
	)

{
	m_bAnimScriptSet = true;
	m_csAnimScript = ev->GetConstString(1);

	//FIXME: macro
	m_AnimMode = 1;

	//FIXME: macro: THINKSTATE_*
	SetThinkIdle(22);
}

void Actor::EventAnimScript_Scripted
	(
	Event *ev
	)

{
	m_bAnimScriptSet = true;
	m_csAnimScript = ev->GetConstString(1);

	//FIXME: macro
	m_AnimMode = 5;

	//FIXME: macro: THINKSTATE_*
	SetThinkIdle(22);
}

void Actor::EventAnimScript_Noclip
	(
	Event *ev
	)

{
	m_bAnimScriptSet = true;
	m_csAnimScript = ev->GetConstString(1);
	//FIXME: macro
	m_AnimMode = 6;

	//FIXME: macro: THINKSTATE_*
	SetThinkIdle(22);
}

void Actor::EventReload_mg42
	(
	Event *ev
	)

{
	//FIXME: macos/enum
	if (m_State != 1201)
	{
		if (m_State == 1200)
		{
			m_State = 1201;
			m_bAnimScriptSet = true;
			m_iStateTime = level.inttime;
		}
		else
		{
			Unregister(STRING_ANIMDONE);
		}
	}
}

void Actor::SetPathWithLeash
	(
	Vector vDestPos,
	const char *description,
	int iMaxDirtyTime
	)

{
	vec2_t dist = { vDestPos[0] - m_vHome[0],  vDestPos[1] - m_vHome[1] };
	float distSq = DotProduct2D(dist, dist);
	if (distSq <= m_fLeashSquared)
	{
		dist[0] = origin[0] - m_vHome[0];
		dist[1] = origin[1] - m_vHome[1];
		distSq = DotProduct2D(dist, dist);
		if (distSq <= m_fLeashSquared)
		{
			SetPath(
				vDestPos,
				description,
				iMaxDirtyTime,
				m_vHome,
				m_fLeashSquared);
		}
	}
	else
	{
		ClearPath();
	}
}

void Actor::SetPathWithLeash
	(
	SimpleEntity *pDestNode,
	const char *description,
	int iMaxDirtyTime
	)

{
	if (pDestNode)
	{
		SetPathWithLeash(
			pDestNode->origin,
			description,
			iMaxDirtyTime);
	}
	else
	{
		if (m_bPathErrorTime + 5000 < level.inttime)
		{
			m_bPathErrorTime = level.inttime;
			Com_Printf(
				"^~^~^ No destination node specified for '%s' at (%f %f %f)\n",
				targetname.c_str(),
				origin[0],
				origin[1],
				origin[2]);
		}
		ClearPath();
	}
}

void Actor::FindPathAwayWithLeash
	(
	vec3_t vAwayFrom,
	vec3_t vDirPreferred,
	float fMinSafeDist
	)

{
	m_Path.FindPathAway(
		origin,
		vAwayFrom,
		vDirPreferred,
		this,
		fMinSafeDist,
		m_vHome,
		m_fLeashSquared);
	ShortenPathToAvoidSquadMates();
}

void Actor::FindPathNearWithLeash
	(
	vec3_t vNearbyTo,
	float fCloseDistSquared
	)

{
	vec2_t dist = {vNearbyTo[0] - m_vHome[0],  vNearbyTo[1] - m_vHome[1]};
	float distSq = DotProduct2D(dist,dist);
	
	if (distSq <= m_fLeashSquared + fCloseDistSquared + 2 * sqrt(m_fLeashSquared * fCloseDistSquared))
	{
		dist[0] = origin[0] - m_vHome[0];
		dist[1] = origin[1] - m_vHome[1];
		distSq = DotProduct2D(dist, dist);
		if (distSq <= m_fLeashSquared)
		{
			m_Path.FindPathNear(
				origin,
				vNearbyTo,
				this,
				0,
				fCloseDistSquared,
				m_vHome,
				m_fLeashSquared);
			ShortenPathToAvoidSquadMates();
		}

	}
}

bool Actor::CanMovePathWithLeash
	(
	void
	) const

{
	vec2_t delta;

	if( PathExists() )
	{
		delta[ 0 ] = origin[ 0 ] - m_vHome[ 0 ];
		delta[ 1 ] = origin[ 1 ] - m_vHome[ 1 ];

		return ( VectorLength2DSquared( delta ) < m_fLeashSquared ) || ( DotProduct2D( delta, m_Path.CurrentDelta() ) < 0.0f );
	}

	return false;
}

bool Actor::MovePathWithLeash
	(
	void
	)

{
	if( CanMovePathWithLeash() )
	{
		Anim_RunToInOpen( 2 );
		FaceMotion();
		return true;
	}
	else
	{
		Anim_Aim();
		return false;
	}
}

bool Actor::ShortenPathToAttack
	(
	float fMinDist
	)

{
	if (PathExists() && !PathComplete() && PathAvoidsSquadMates())
	{
		for (auto current_node = CurrentPathNode(); current_node > LastPathNode(); current_node--)
		{
			Vector dist = current_node->point - origin;
			if (dist * dist >= fMinDist * fMinDist)
			{
				if (CanSeeFrom(eyeposition + current_node->point, m_Enemy))
				{
					m_Path.TrimPathFromEnd((current_node - LastPathNode()) / 32);
					return true;
				}
			}
		}
	}

	return false;
}

void Actor::StrafeToAttack
	(
	float fDist,
	vec3_t vDir
	)

{
	Vector vEnemyCentroid, vDelta, vSpot, vDestPos;
	Vector mins(-16,-16,16);
	Vector maxs(16,16,128);

	vDestPos = vDir;
	vDestPos *= fDist;
	vDestPos += origin;

	vDelta = vDestPos - m_vLastEnemyPos;

	if (m_fMaxDistanceSquared <= vDelta.lengthSquared() || vDelta.lengthSquared() <= m_fMinDistanceSquared )
	{
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
		"Actor::StrafeToAttack 1"))
	{
		ClearPath();
		return;
	}
	// FIXME: stub
	STUB();
}

Vector Actor::GunTarget
	(
	bool bNoCollision
	)

{
	float accuracy;
	static bool doInit = true;
	static cvar_t *aiRanges[4];
	Vector vRet;

	if (doInit)
	{
		aiRanges[0] = gi.Cvar_Get("g_aishortrange", "500", 0);
		aiRanges[1] = gi.Cvar_Get("g_aimediumrange", "700", 0);
		aiRanges[2] = gi.Cvar_Get("g_ailongrange", "1000", 0);
		aiRanges[3] = gi.Cvar_Get("g_aisniperrange", "2200", 0);
		doInit = false;
	}

	if (mTargetPos == vec_zero)
	{
		AnglesToAxis(origin, orientation);
		vRet = Vector(orientation[0]) * 2048 + EyePosition();
		return vRet;
	}
	float scatterMult = 1;

	Player *player = NULL;
	if (m_Enemy)
	{
		if (m_Enemy->IsSubclassOfPlayer())
		{
			player = (Player *)m_Enemy.Pointer();


			Weapon *weap = GetActiveWeapon(WEAPON_MAIN);
			Vector dir;
			dir = player->centroid - centroid;

			if (weap)
			{

				MPrintf("WEAPON RANGE: %f  dist: %f\n", aiRanges[weap->mAIRange]->value, dir.length());
				scatterMult = dir.length() / aiRanges[weap->mAIRange]->value;
				if (dir.length() > aiRanges[weap->mAIRange]->value)
				{
					if (scatterMult > 1)
					{
						accuracy = mAccuracy / scatterMult;
					}
					MPrintf("OUT OF RANGE - scatterMult: %f\n", scatterMult);
				}
			}
			else
			{
				/*
				 *useless assert
				  if ( !dword_39A914 )
				  {
					strcpy(&v54, "\"ERROR Actor::GunTarget without a weapon\\n\"\n\tMessage: ");
					memset(&s, 0, 0x3FC9u);
					v48 = (*(this->baseSimpleActor.baseSentient.baseAnimate.baseEntity.baseSimple.baseListener.baseClass.vftable + 87))(
							this,
							&nullStr);
					Q_strcat(&v54, 0x4000, v48);
					v49 = MyAssertHandler(&v54, "fgame/actor.cpp", 10977, 0);
					if ( v49 < 0 )
					{
					  dword_39A914 = 1;
					}
					else if ( v49 > 0 )
					{
					  __debugbreak();
					}
				  }
				 
				 */
			}
		}
		else if (fabs(m_Enemy->origin[2] - origin[2]) >= 128)
		{
			scatterMult = 0.5;
		}
	}

	if (random2() > accuracy)
	{
		MPrintf("MISS - scatterMult: %f\n", scatterMult);
		
		if (player)
		{
			static cvar_t *scatterHeight;
			static cvar_t *scatterWide;
			static bool doInitScatters = false;
			int lastTrail;
			
			lastTrail = player->mCurTrailOrigin - 1;
			if (lastTrail < 0)
				lastTrail = 1;
			
			vRet = player->mvTrail[lastTrail];
			//continue here:
			//Player *tPlayer = player + 12*lastTrail;//weird
			if (!G_SightTrace(GunPosition(), vec_zero, vec_zero, player->mvTrail[lastTrail], this, player, 33819417, 0, "Actor::GunTarget 1"))
			{
				vRet = player->mvTrailEyes[lastTrail];
			}
			if (!doInitScatters)
			{
				scatterWide = gi.Cvar_Get("g_aiScatterWide", "16.0", 0);
				scatterHeight = gi.Cvar_Get("g_aiScatterHeight", "45.0", 0);
				doInitScatters = true;
			}
			vRet.x += crandom() * scatterWide->value * scatterMult;
			vRet.y += crandom() * scatterWide->value * scatterMult;
			vRet.z += crandom() * scatterHeight->value * scatterMult;
			
			//continue here:
			//if (player->GetVehicleTank())
			{
			}
		}
	}
	// FIXME: stub
	STUB();
	return vec_zero;
}

qboolean Actor::setModel
	(
	void
	)

{
	qboolean bSuccess;
	str name, tempstr;

	if (model != NULL)
	{
		if (m_csLoadOut != STRING_EMPTY)
		{

			name = "weapon|" + Director.GetString(m_csLoadOut) + "|";
		}

		if (m_csHeadModel == STRING_EMPTY)
		{
			m_csHeadModel = Director.AddString(level.GetRandomHeadModel(model));
		}
		name += "headmodel|" + Director.GetString(m_csHeadModel) + "|";

		if (m_csHeadSkin == STRING_EMPTY)
		{
			m_csHeadSkin = Director.AddString(level.GetRandomHeadSkin(model));
		}
		name += "headskin|" + Director.GetString(m_csHeadSkin) + "|";//FIXME: not sure of this last "|"
	}
	model = name;
	return Entity::setModel();
}

void Actor::EventSetHeadModel
	(
	Event *ev
	)

{

	m_csHeadModel = ev->GetConstString(1);
	setModel();
}

void Actor::EventGetHeadModel
	(
	Event *ev
	)

{
	ev->AddConstString(m_csHeadModel);
}

void Actor::EventSetHeadSkin
	(
	Event *ev
	)

{
	m_csHeadSkin = ev->GetConstString(1);
	setModel();
}

void Actor::EventGetHeadSkin
	(
	Event *ev
	)

{
	ev->AddConstString(m_csHeadSkin);
}

void Actor::EventSetNoIdle
	(
	Event *ev
	)

{
	m_bNoIdleAfterAnim = ev->GetInteger(1);
}

void Actor::EventGetNoIdle
	(
	Event *ev
	)

{
	ev->AddInteger(m_bNoIdleAfterAnim);
}

void Actor::EventGetEnemy
	(
	Event *ev
	)

{
	ev->AddListener(m_Enemy);
}

void Actor::EventSetMaxNoticeTimeScale
	(
	Event *ev
	)

{
	float noticeScale; // fst7

	noticeScale = ev->GetFloat(1);
	if (noticeScale <= 0.0)
	{
		Com_Printf("^~^~^ ERROR: noticescale: value must be greater than 0\n");
	}
	else
	{
		m_fMaxNoticeTimeScale = noticeScale / 100;
	}
}

void Actor::EventGetMaxNoticeTimeScale
	(
	Event *ev
	)

{
	//FIXME: *100 ?
	ev->AddFloat(m_fMaxNoticeTimeScale * 100);
}

void Actor::EventSetFixedLeash
	(
	Event *ev
	)

{
	m_bFixedLeash = ev->GetInteger(1);
}

void Actor::EventGetFixedLeash
	(
	Event *ev
	)

{
	ev->AddInteger(m_bFixedLeash);
}

void Actor::Holster
	(
	void
	)

{
	if (activeWeaponList[0])
	{
		DeactivateWeapon(WEAPON_MAIN);
	}
}

void Actor::Unholster
	(
	void
	)

{
	Weapon *weap;

	if (!activeWeaponList[0])
	{
		weap = GetWeapon(WEAPON_MAIN);
		if (weap)
		{
			useWeapon(weap, WEAPON_MAIN);
			ActivateNewWeapon();
		}
	}
}

void Actor::EventHolster
	(
	Event *ev
	)

{
	if (activeWeaponList[WEAPON_MAIN])
	{
		DeactivateWeapon(WEAPON_MAIN);
	}
}

void Actor::EventUnholster
	(
	Event *ev
	)

{
	Unholster();
}

void Actor::EventSoundDone
	(
	Event *ev
	)

{
	//soundChannel_t
	int channel = ev->GetInteger(1);
	str name = ev->GetString(2);
	if (gi.S_IsSoundPlaying(channel, name))
	{
		Event e1(EV_SoundDone);
		e1.AddInteger(channel);
		e1.AddString(name);
		PostEvent(e1, level.frametime);
	}
	//FIXME: macro
	else if (m_bSayAnimSet && m_iSaySlot == -2)
	{
		ChangeSayAnim();
		if (m_csSayAnim == STRING_EMPTY)
		{
			Unregister(STRING_SAYDONE);
		}
	}
	else
	{
		Unregister(STRING_SOUNDDONE);
	}
}

void Actor::EventSound
	(
	Event *ev
	)

{
	ProcessSoundEvent(ev, m_Team == TEAM_AMERICAN);
}

void Actor::EventIsEnemyVisible
	(
	Event *ev
	)

{
	ev->AddInteger(m_bEnemyVisible);
}

void Actor::EventGetEnemyVisibleChangeTime
	(
	Event *ev
	)

{
	//FIXME: /100 ??
	ev->AddFloat(m_iEnemyVisibleChangeTime / 100);
}

void Actor::EventGetLastEnemyVisibleTime
	(
	Event *ev
	)

{
	//FIXME: /100 ??
	ev->AddFloat(m_iLastEnemyVisibleTime / 100 );
}

void Actor::EventSetFallHeight
	(
	Event *ev
	)

{
	float fHeight = ev->GetFloat(1);

	if (fHeight < MIN_FALLHEIGHT)
	{
		ScriptError("value less than %d not allowed", MIN_FALLHEIGHT);
	}
	if (fHeight > MAX_FALLHEIGHT)
	{
		ScriptError("value greater than %d not allowed", MAX_FALLHEIGHT);
	}

	m_Path.SetFallHeight(fHeight);
}

void Actor::EventGetFallHeight
	(
	Event *ev
	)

{
	ev->AddFloat(m_Path.GetFallHeight());
}

void Actor::EventCanMoveTo
	(
	Event *ev
	)

{
	float fIntervalSquared, fDistSquared;
	vec2_t vDelta;
	Vector vDest;

	vDest = ev->GetVector(1);
	VectorSub2D(vDest, m_vHome, vDelta);
	fDistSquared = VectorLength2DSquared(vDelta);
	if (fDistSquared >= m_fLeashSquared)
	{
		ev->AddInteger(0);
		return;
	}
	else if(m_Enemy != NULL)
	{
		VectorSub2D(vDest, m_Enemy->origin, vDelta);
		fDistSquared = VectorLength2DSquared(vDelta);
		if (m_fMinDistanceSquared >= fDistSquared || fDistSquared >= m_fMaxDistanceSquared)
		{
			ev->AddInteger(qfalse);
			return;
		}
	}
	if (m_fInterval == 0)
	{
		for (auto pSquadMate = m_pNextSquadMate; pSquadMate != this ; pSquadMate = pSquadMate->m_pNextSquadMate)
		{
			float v27, v29, v30, v32, v33, v34, v35, v37, v38, v45, v44;
			if (pSquadMate->IsSubclassOfActor())
			{
				Sentient *pEnemy = pSquadMate->m_Enemy;
				if (pEnemy)
				{
					v27 = pSquadMate->origin[0];
					v29 = pEnemy->origin[0] - v27;
					v30 = pSquadMate->origin[1];
					v32 = pEnemy->origin[1] - v30;
				}
				else
				{
					continue;
				}
			}
			else
			{
				v27 = pSquadMate->origin[0];
				v29 = pSquadMate->orientation[0][0];
				v32 = pSquadMate->orientation[0][1];
				v30 = pSquadMate->origin[1];
			}
			v33 = v32 * origin[0];
			v34 = v27 * v32 - v30 * v29;
			v35 = v33 - v29 * origin[1] - v34;
			v37 = v32 * v44;
			v38 = v37 - v29 * v45 - v34;

			// check if they both have different sign.
			if (((int)v38 ^ (int)v35) < 0)
			{
				ev->AddInteger(qfalse);
				return;
			}
		}
		ev->AddInteger(qtrue);
		return;
	}

	fIntervalSquared = m_fInterval * m_fInterval;

	for (auto pSquadMate = m_pNextSquadMate; pSquadMate != this; pSquadMate = pSquadMate->m_pNextSquadMate)
	{
		Vector vDel;
		vDel = vDest - pSquadMate->origin;
		if (fIntervalSquared <= vDest.lengthSquared())
		{
			continue;
		}

		if ((origin-pSquadMate->origin).lengthSquared() > vDest.lengthSquared())
		{
			break;
		}
	}
	ev->AddInteger(qfalse);
}

void Actor::EventMoveDir
	(
	Event *ev
	)

{
	
	vec2_t vec;
	if (!PathGoalSlowdownStarted())
	{
		vec[0] = velocity.x;
		vec[1] = velocity.y;
		if ( velocity.x == 0 && velocity.y == 0 && PathExists() && !PathComplete())
		{
				vec[0] = PathDelta()[0];
				vec[1] = PathDelta()[1];
		}
		VectorNormalize2D(vec);
	}

	ev->AddVector(vec);
}

void Actor::EventIntervalDir
	(
	Event *ev
	)

{
	Vector dir;
	dir = m_vIntervalDir;
	//FIXME: macro
	if (level.inttime >= m_iIntervalDirTime + 250)
	{
		m_vIntervalDir = vec_zero;
		m_iIntervalDirTime = level.inttime;
		if (m_Enemy)
		{
			m_vIntervalDir = GetAntiBunchPoint() - origin;
			if (m_vIntervalDir.x != 0 || m_vIntervalDir.y != 0)
			{
				VectorNormalizeFast(m_vIntervalDir);
			}
		}
	}
	//FIXME: not sure if i should add dir or m_vIntervalDir
	ev->AddVector(m_vIntervalDir);
}

void Actor::EventResetLeash
	(
	Event *ev
	)

{
	m_vHome = origin;
	delete m_pTetherEnt;
}

void Actor::EventTether
	(
	Event *ev
	)

{
	m_pTetherEnt = ev->GetSimpleEntity(1);
}

void Actor::EventGetThinkState
	(
	Event *ev
	)

{
	ev->AddConstString(m_csThinkStateNames[m_ThinkState]);
}

void Actor::EventGetEnemyShareRange
	(
	Event *ev
	)

{
	ev->AddFloat(m_fMaxShareDistSquared);
}

void Actor::EventSetEnemyShareRange
	(
	Event *ev
	)

{
	float dist = ev->GetFloat(1);
	m_fMaxShareDistSquared = dist * dist;
}

void Actor::EventGetKickDir
	(
	Event *ev
	)

{
	ev->AddVector(m_vKickDir);
}

void Actor::EventGetNoLongPain
	(
	Event *ev
	)

{
	ev->AddInteger(m_bNoLongPain);
}

void Actor::EventSetNoLongPain
	(
	Event *ev
	)

{
	m_bNoLongPain = ev->GetBoolean(1);
}

void Actor::EventGetFavoriteEnemy
	(
	Event *ev
	)

{
	ev->AddEntity(m_FavoriteEnemy);
}

void Actor::EventSetFavoriteEnemy
	(
	Event *ev
	)

{
	Sentient *fEnemy = (Sentient*)ev->GetEntity(1);
	m_FavoriteEnemy = fEnemy;
}

void Actor::EventGetMumble
	(
	Event *ev
	)

{
	ev->AddInteger(m_bMumble);
}

void Actor::EventSetMumble
	(
	Event *ev
	)

{
	m_bMumble = ev->GetInteger(1);
}

void Actor::EventGetBreathSteam
	(
	Event *ev
	)

{
	ev->AddInteger(m_bBreathSteam);
}

void Actor::EventSetBreathSteam
	(
	Event *ev
	)

{
	m_bBreathSteam = ev->GetInteger(1);
}

void Actor::EventSetNextBreathTime
	(
	Event *ev
	)

{
	ScriptVariable sVar;
	ScriptThread *t = Director.CreateThread("global/breathe.scr", "nextbreathtime");
	
	sVar.setFloatValue(ev->GetFloat(1));
	t->Execute(&sVar, 1);
}

void Actor::EventCalcGrenadeToss
	(
	Event *ev
	)

{
	//FIXME: macros
	Vector vTarget;
	vTarget = ev->GetVector(1);
	if (DecideToThrowGrenade(vTarget, &m_vGrenadeVel, &m_eGrenadeMode))
	{
		if (m_eGrenadeMode == 2)
		{
			ev->AddConstString(STRING_ANIM_GRENADETOSS_SCR);
		}
		else
		{
			if (m_eGrenadeMode > 2 ? m_eGrenadeMode == 3 : m_eGrenadeMode == 1)
			{
				ev->AddConstString(STRING_ANIM_GRENADETHROW_SCR);
			}
			/*
			 useless assert:
			 else if ( !dword_39A8F0 )
			  {
				strcpy(&v13, "\"invalid return condition for Actor::EventCalcGrenadeToss\"\n\tMessage: ");
				memset(&s, 0, 0x3FBAu);
				v7 = (*(this->baseSimpleActor.baseSentient.baseAnimate.baseEntity.baseSimple.baseListener.baseClass.vftable + 87))(
					   this,
					   &nullStr);
				Q_strcat(&v13, 0x4000, v7);
				v8 = MyAssertHandler(&v13, "fgame/actor.cpp", 12490, 0);
				if ( v8 < 0 )
				{
				  dword_39A8F0 = 1;
				}
				else if ( v8 > 0 )
				{
				  __debugbreak();
				}
			  }
			 */
		}
		
		m_YawAchieved = false;
		m_DesiredYaw = vectoyaw(m_vGrenadeVel);
	}
	else
	{
		ev->AddConstString(STRING_EMPTY);
	}
}

void Actor::EventGetNoSurprise
	(
	Event *ev
	)
{
	ev->AddInteger(m_bNoSurprise);
}

void Actor::EventSetNoSurprise
	(
	Event *ev
	)

{

	m_bNoSurprise = ev->GetBoolean(1);
}

void Actor::EventGetSilent
	(
	Event *ev
	)

{
	ev->AddInteger(m_bSilent);
}

void Actor::EventSetSilent
	(
	Event *ev
	)

{
	m_bSilent = ev->GetBoolean(1);
}

void Actor::EventGetAvoidPlayer
	(
	Event *ev
	)

{
	ev->AddInteger(m_bAutoAvoidPlayer);
}

void Actor::EventSetAvoidPlayer
	(
	Event *ev
	)

{
	m_bAutoAvoidPlayer = ev->GetBoolean(1);
}

void Actor::EventGetLookAroundAngle
	(
	Event *ev
	)

{
	//FIXME: weird
	//ev->GetFloat(m_fLookAroundFov);
	ev->AddFloat(m_fLookAroundFov);
}

void Actor::EventSetLookAroundAngle
	(
	Event *ev
	)

{
	float angle = ev->GetFloat(1);
	if (angle < 0.0 || angle > 60.0)
	{
		ScriptError("lookaroundangle must be >= 0 and <= 60");
	}
	m_fLookAroundFov = angle;
}

void Actor::EventHasCompleteLookahead
	(
	Event *ev
	)

{
	int completeLH = PathExists() && PathHasCompleteLookahead();
	
	ev->AddInteger(completeLH);
}

void Actor::EventPathDist
	(
	Event *ev
	)

{
	float dist = 0;
	if (PathExists() && !PathComplete())
	{
		dist = PathDist();
	}
	ev->AddFloat(dist);
}

void Actor::EventCanShootEnemyFrom
	(
	Event *ev
	)

{
	int canShoot = false;
	if (m_Enemy)
	{
		canShoot = CanSeeFrom(ev->GetVector(1), m_Enemy);
	}
	ev->AddInteger(canShoot);
}

void Actor::EventCanShoot
	(
	Event *ev
	)

{
	Entity *target = ev->GetEntity(1);

	if (!target)
	{
		ScriptError("canshoot applied to null listener");
	}

	ev->AddInteger(CanShoot(target));
}

void Actor::EventSetInReload
	(
	Event *ev
	)

{

	m_bInReload = ev->GetBoolean(1);
}

void Actor::EventGetInReload
	(
	Event *ev
	)

{
	ev->AddInteger(m_bInReload);
}

void Actor::EventSetReloadCover
	(
	Event *ev
	)

{
	m_bNeedReload = ev->GetBoolean(1);
}

void Actor::EventBreakSpecial
	(
	Event *ev
	)

{
	mbBreakSpecialAttack = 1;
}

void Actor::GetVoiceType
	(
	Event *ev
	)

{
	//voice type in actor is a char.
	const char *vType = va("%c", mVoiceType);
	str vTypeStr = vType;
	ev->AddString(vTypeStr);
}

void Actor::SetVoiceType
	(
	Event *ev
	)

{
	//voice type in actor is a char.
	str vType = ev->GetString(1);
	if (*vType.c_str())
	{
		mVoiceType = *vType.c_str();
	}
	else
	{
		//FIXME: enum
		mVoiceType = -1;
	}
}

void Actor::ResolveVoiceType
	(
	void
	)

{
	char validVoice[128];

	if (mVoiceType == -1)
	{
		int d = 3.0 * random();
		if (m_Team == TEAM_AMERICAN)
		{
			mVoiceType = *gAmericanVoices[d];
		}
		else
		{
			mVoiceType = *gGermanVoices[d];
		}
	}
	else
	{
		if (m_Team == TEAM_AMERICAN)
		{
			for (int i = 0; *gAmericanVoices[i] != mVoiceType ; i++)
			{
				if (i > 3)
				{
					sprintf(validVoice, "");
					for (int j = 0; j <= 2 ; j++)
					{
						strcat(validVoice, gAmericanVoices[j]);
						strcat(validVoice, " ");
					}
					Com_Printf("ERROR: Bad voice type %c.  Valid American voicetypes are: %s\n", mVoiceType, validVoice);
					mVoiceType = -1;

					int d = 3.0 * random();
					mVoiceType = *gAmericanVoices[d];
				}
			}
		}
		else
		{

			for (int i = 0; *gGermanVoices[i] != mVoiceType; i++)
			{
				if (i > 3)
				{
					sprintf(validVoice, "");
					for (int j = 0; j <= 2; j++)
					{
						strcat(validVoice, gGermanVoices[j]);
						strcat(validVoice, " ");
					}
					Com_Printf("ERROR: Bad voice type %c.  Valid German voicetypes are: %s\n", mVoiceType, validVoice);
					mVoiceType = -1;

					int d = 3.0 * random();
					mVoiceType = *gGermanVoices[d];
				
				}
			}
		}

	}
}

void Actor::EventSetBalconyHeight
	(
	Event *ev
	)

{
	m_fBalconyHeight = ev->GetFloat(1);
}

void Actor::EventGetBalconyHeight
	(
	Event *ev
	)

{
	ev->AddFloat(m_fBalconyHeight);
}

PathNode *Actor::FindSniperNodeAndSetPath
	(
	bool *pbTryAgain
	)

{
	PathNode *pSniperNode = PathManager.FindNearestSniperNode( this, origin, m_Enemy );

	if( pSniperNode )
	{
		SetPathWithLeash( pSniperNode, NULL, NULL );

		if( PathExists() && ( PathComplete() || PathAvoidsSquadMates() ) )
		{
			*pbTryAgain = true;
			return pSniperNode;
		}

		pSniperNode->MarkTemporarilyBad();
		*pbTryAgain = false;
	}
	else
	{
		*pbTryAgain = true;
	}

	return NULL;
}

void Actor::Remove
	(
	Event *ev
	)

{
	EndStates();

	if (deadflag != DEAD_DEAD)
	{
		health = 0;
		deadflag = DEAD_DEAD;
	}

	Delete();
}

void Actor::DontFaceWall
	(
	void
	)

{
	PathNode *pNode;
	vec2_t vDelta;
	float fErrorLerp, fEyeRadius, fCosAngle, fSinAngle, fAngle, fAngleRad, fTime;
	Vector start;
	Vector end;
	
	//FIXME: macro: AI_*
	if (m_pCoverNode && m_pCoverNode->nodeflags & (AI_CORNER_LEFT | AI_CORNER_RIGHT | AI_SNIPER | AI_CRATE))
	{
		m_eDontFaceWallMode = 1;
		return;
	}

	if (velocity.lengthXY(true) > 64)
	{
		m_eDontFaceWallMode = 2;
		return;
	}

	fAngle = AngleNormalize360(m_YawAchieved ? angles.y : m_DesiredYaw);

	fTime = (level.time - m_fDfwTime) * 0.5;
	if (fTime > 1)
	{
		fTime = 1;
	}


	VectorSub2D(m_vDfwPos, origin, vDelta);

	if (Square(fTime * -14.0 + 16.0) > VectorLength2DSquared(vDelta) 
		&& (fabs(AngleNormalize180(m_fDfwRequestedYaw - m_DesiredYaw)) <= fTime * -29.0 + 30.0
				|| fabs(AngleNormalize180(m_fDfwDerivedYaw - m_DesiredYaw)) <= fTime * -29.0 + 30.0)
		)
	{
		if (m_eDontFaceWallMode <= 8)
		{
			m_YawAchieved = false;
			m_DesiredYaw = m_fDfwDerivedYaw;
		}
	}
	else
	{
		m_vDfwPos = origin;
		m_fDfwRequestedYaw = fAngle;
		m_fDfwTime = level.time;

		fAngleRad = fAngle * M_PI / 180;
		fSinAngle = sin(fAngleRad);
		fCosAngle = cos(fAngleRad);

		VectorSub2D(EyePosition(), origin, vDelta);
		fEyeRadius = VectorLength2D(vDelta);

		start = vec_zero;
		start.x = fEyeRadius * fCosAngle + origin.x;
		start.y = fEyeRadius * fSinAngle + origin.y;

		end = vec_zero;
		end.x = 64 * fCosAngle + start.x;
		end.x = 64 * fSinAngle + start.y;
		trace_t trace = G_Trace(start, vec_zero, vec_zero, end, this, 33819417, qfalse, "Actor::DontFaceWall");
		if (trace.entityNum == 1023 || trace.fraction >= 1 || trace.startsolid)
		{
			m_eDontFaceWallMode = 3;
		}
		else if (trace.entityNum == 1022 || trace.ent->entity->AIDontFace())
		{
			if (trace.plane.normal[2] < -0.69999999 || trace.plane.normal[2] > 0.69999999)
			{
				m_eDontFaceWallMode = 5;
			}
			else
			{
				if (m_Enemy && Vector::Dot(trace.plane.normal, m_Enemy->origin) - trace.plane.dist < 0 )
				{
					end = m_Enemy->origin;
				}
				else
				{
					end = start + (end - start)*128;
				}


				PathNode *node = PathManager.FindCornerNodeForWall(origin, end, this, 0.0f, trace.plane.normal);
				if (node)
				{

					VectorSub2D(node->m_PathPos, origin, vDelta);
					if (vDelta[0] != 0 || vDelta[1] != 0)
					{
						m_YawAchieved = false;
						m_DesiredYaw = vectoyaw(vDelta);
					}
					m_eDontFaceWallMode = 6;
				}
				else if (trace.fraction <= 0.46875)
				{
					m_YawAchieved = false;
					m_DesiredYaw = vectoyaw(trace.plane.normal);
					m_eDontFaceWallMode = 7;
				}
				else
				{
					if (m_DesiredYaw < 90.0)
						m_DesiredYaw += 270.0;
					else
						m_DesiredYaw -=  90.0;

					m_YawAchieved = false;
					m_eDontFaceWallMode = 8;
				}
				m_fDfwDerivedYaw = m_DesiredYaw;
				return;
			}
		}
		else
		{
			m_eDontFaceWallMode = 4;
		}

		m_fDfwDerivedYaw = m_fDfwRequestedYaw;
	}

}

bool Actor::AvoidingFacingWall
	(
	void
	) const

{
	return m_eDontFaceWallMode <= 8;
}

void Actor::EndStates
	(
	void
	)

{
	for (int i = 0; i < MAX_THINKSTATES; i++)
	{
		GlobalFuncs_t *func = &GlobalFuncs[m_Think[i]];
		
		if (func->EndState)
			(this->*func->EndState)();

		if (m_pAnimThread)
		{
			m_pAnimThread->AbortRegistration(STRING_EMPTY, this);
		}
	}
}

void Actor::ClearStates
	(
	void
	)

{
	for (int i = 0; i <= MAX_THINKMAP; i++)
	{
		m_ThinkMap[i] = 0;
		if (m_ThinkState == i)
		{
			m_bDirtyThinkState = true;
		}
	}
}

void Actor::CheckUnregister
	(
	void
	)

{
	m_bBecomeRunner = false;
	if (parm.movefail)
	{
		parm.movedone = false;
		Unregister(STRING_MOVEDONE);
	}
	if (m_Enemy)
	{
		Unregister(STRING_HASENEMY);
	}
	if (m_bEnemyVisible)
	{
		Unregister(STRING_VISIBLE);
	}
}

void Actor::BecomeCorpse
	(
	void
	)

{
	Event e1(EV_DeathSinkStart);

	AddToBodyQue();

	edict->r.contents = CONTENTS_TRIGGER;
	edict->r.svFlags &= ~SVF_USE_CURRENT_ORIGIN;
	setSolidType(SOLID_NOT);
	movetype = MOVETYPE_NONE;
	edict->s.renderfx &= ~RF_SHADOW;

	
	PostEvent(e1, 20, 0);
}

void Actor::PathnodeClaimRevoked
	(
	PathNode *node
	)

{
	for (int i = m_ThinkLevel; i >= 0; --i)
	{


		GlobalFuncs_t *func = &GlobalFuncs[m_Think[i]];
		if (func->PathnodeClaimRevoked)
			(this->*func->PathnodeClaimRevoked)();
	}
	node->Relinquish();
	m_pCoverNode = NULL;
}

void Actor::SetPathToNotBlockSentient
	(
	Sentient *pOther
	)

{
	Vector vDest, vPerp, vAway;

	if (pOther)
	{
		if (!pOther->IsDead())
		{
			if (IsTeamMate(pOther))
			{
				if (pOther->velocity.lengthSquared() > 1)
				{
					vDest = origin - pOther->origin;
					if (vDest.lengthSquared() < 2304 && DotProduct2D(pOther->velocity, vDest) > 0)
					{
						if (CrossProduct2D(vDest, pOther->velocity) >= 0)
						{
							vPerp.x = -pOther->velocity.y;
							vPerp.y = pOther->velocity.x;
							vPerp.z = 0;
						}
						else
						{
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
							"Actor::SetPathToNotBlockSentient 1"))
						{
							SetPathWithinDistance(vAway, "", 96, 0);
						}

						if (!PathExists())
						{
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
								"Actor::SetPathToNotBlockSentient 2"))
							{
								SetPathWithinDistance(vAway, "", 96, 0);
							}

							if (!PathExists())
							{
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
									"Actor::SetPathToNotBlockSentient 3"))
								{
									SetPathWithinDistance(vAway, "", 96, 0);
								}

								if (!PathExists())
								{
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

void Actor::EventSetMoveDoneRadius
	(
	Event *ev
	)

{
	float moveDoneR = ev->GetFloat(1);
	m_fMoveDoneRadiusSquared = moveDoneR * moveDoneR;
}


bool Actor::CalcFallPath
	(
	void
	)

{
	float startTime, animTime, startDeltaTime, nextTime;
	vec3_t vAbsDelta, vRelDelta, pos[200];
	int anim, loop, currentPos, i;
	mmove_t mm;

	SetMoveInfo(&mm);

	mm.desired_speed = 80;
	mm.tracemask &= 0xFDFFF4FF;

	VectorCopy2D(orientation[0], mm.desired_dir);

	anim = gi.Anim_NumForName(edict->tiki, "death_balcony_intro");
	animTime = gi.Anim_Time(edict->tiki, anim);

	startTime = 0.64999998;

	i = 0;
	while (true)
	{
		MmoveSingle(&mm);
		
		i++;
		VectorCopy(mm.origin, pos[i]);

		if (i >= 200)
		{
			break;
		}

		if (mm.hit_obstacle)
		{
			for (int i = 0.64999998; i < startTime; i = nextTime)
			{
				nextTime = i + level.frametime;
				if (nextTime >= startTime - 0.0099999998)
					nextTime = startTime;
				startDeltaTime = i;
				(gi.Anim_DeltaOverTime)(
					edict->tiki,
					anim,
					startDeltaTime,
					nextTime,
					vAbsDelta);
				MatrixTransformVector(vAbsDelta, orientation, vRelDelta);

				i++;
				
				VectorAdd(vRelDelta, mm.origin, mm.origin);
				VectorCopy(mm.origin, pos[i]);

				if (i >= 200)
					return false;
			}
			mm.desired_speed = 0;
			mm.groundPlane = qfalse;
			mm.walking = qfalse;
			mm.velocity[0] = 0;
			mm.velocity[1] = 0;
			mm.velocity[2] = -171;
			
			loop = i;

			while (true)
			{
				MmoveSingle(&mm);

				i++;
				VectorCopy(mm.origin, pos[i]);

				if (i >= 200)
				{
					break;
				}

				if (mm.hit_obstacle)
					return false;

				if (mm.groundPlane)
				{
					if (m_fBalconyHeight > origin[2] - pos[i][2])
						return false;

					m_pFallPath = (FallPath *)gi.Malloc(sizeof(FallPath) * i);

					m_pFallPath->length = i;

					m_pFallPath->currentPos = 0;
					m_pFallPath->startTime = startTime;
					m_pFallPath->loop = loop;

					if (i > 0)
					{
						for (int j = i; j ; j--)
						{
							VectorCopy(pos[j], m_pFallPath->pos[j]);
						}
					}
					return true;
				}
			}

			return false;
		}


		if (mm.groundPlane)
		{
			startTime -= level.frametime;
			if (startTime >= 0)
				continue;
		}
		return false;
	}

}

void Actor::IdleThink
	(
	void
	)

{
	//FIXME: revision
	IdlePoint();
	IdleLook();
	if (PathExists() && PathComplete())
	{
		ClearPath();
	}
	if (m_bAutoAvoidPlayer && !PathExists())
	{
		SetPathToNotBlockSentient(( Sentient * )G_GetEntity(0));
	}
	if (!PathExists())
	{
		Anim_Idle();
		IdleTurn();
		PostThink(true);
	}
	else
	{
		//FIXME: macros
		Anim_WalkTo(2);

		if (PathDist() <= 128.0)
		{
			IdleTurn();
			PostThink(true);
		}
		else
		{
			FaceMotion();
		}
	}
	
}

void Actor::ClearEnemies
	(
	void
	)

{
	m_PotentialEnemies.RemoveAll();
	SetEnemy( NULL, false );
}

bool Actor::EnemyIsDisguised
	(
	void
	)

{
	return ( m_bEnemyIsDisguised || m_Enemy->m_bIsDisguised ) && ( !m_bForceAttackPlayer && m_ThinkState != 4 );
}

void Actor::setOriginEvent
	(
	Vector org
	)

{
	bool bRejoin = false;
	//FIXME: macro
	if ((org - origin).lengthSquared() > 1048576)
	{
		bRejoin = true;
		DisbandSquadMate(this);
	}
	
	m_iOriginTime = level.inttime;

	m_vOriginHistory[0][0] = org.x;
	m_vOriginHistory[0][1] = org.y;
	m_vOriginHistory[1][0] = org.x;
	m_vOriginHistory[1][1] = org.y;
	m_vOriginHistory[2][0] = org.x;
	m_vOriginHistory[2][1] = org.y;
	m_vOriginHistory[3][0] = org.x;
	m_vOriginHistory[3][1] = org.y;

	VectorClear(velocity);

	if (level.Spawned())
	{
		if (org != origin)
		{
			setOrigin(org);
		}
		if (!m_bNoPlayerCollision)
		{
			Player *p = (Player *)G_GetEntity(0);
			if (p && IsTouching(p))
			{
				Com_Printf(
					"(entnum %d, radnum %d) is going not solid to not get stuck in the player\n", entnum, radnum);
				m_bNoPlayerCollision = 1;
				setSolidType(SOLID_NOT);
			}
		}
	}
	else
	{
		setOrigin(org);
	}
	m_vHome = origin;

	if (bRejoin)
	{
		//FIXME: macro
		JoinNearbySquads(1024);
	}
}

void Actor::DumpAnimInfo
	(
	void
	)

{
	Animate::DumpAnimInfo();
	Vector desiredLook = m_bHasDesiredLookAngles ? m_DesiredLookAngles : vec_zero;
	//FIXME: macros: bones
	Vector head = GetControllerAngles(0);
	Vector torso = GetControllerAngles(1);
	MPrintf("Desired look yaw: %.1f, pitch: %.1f.  Head yaw: %.1f, pitch %.1f.  Torso yaw: %.1f, pitch: %.1f\n",
		desiredLook.x,
		desiredLook.y,
		head.x,
		head.y,
		torso.x,
		torso.y
		);

}

bool Actor::AutoArchiveModel
	(
	void
	)

{
	return false;
}
