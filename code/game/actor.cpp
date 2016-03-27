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
	SetThinkState( 1, 0 );
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
	m_iCuriousAnimHint = 0;
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

	return Com_Printf( "-------------------------------------------------------------------------------\n" );
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
	// FIXME: stub
	STUB();

	if( ev->IsVectorAt( 1 ) )
	{

	}
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

	if( func->ThinkState )
		( this->*func->ThinkState )();
}

void Actor::setContentsSolid
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::InitThinkStates
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::UpdateEyeOrigin
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

bool Actor::RequireThink
	(
	void
	)

{
	/*
	if( g_entities[ 0 ].entity )
		return ( level.inttime < edict->r.lastNetTime + 60000 );
	else
		return false;
	*/

	return true;
}

void Actor::UpdateEnemy
	(
	int iMaxDirtyTime
	)

{
	// FIXME: stub
	STUB();
}

void Actor::UpdateEnemyInternal
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::SetEnemy
	(
	Sentient *pEnemy,
	bool bForceConfirmed
	)

{
	// FIXME: stub
	STUB();
}

void Actor::SetEnemyPos
	(
	Vector vPos
	)

{
	// FIXME: stub
	STUB();
}

void Actor::ResetBodyQueue
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::AddToBodyQue
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

Vector Actor::GetAntiBunchPoint
	(
	void
	)

{
	// FIXME: stub
	STUB();
	return vec_zero;
}

void Actor::InitVoid
	(
	GlobalFuncs_t *func
	)

{
	func->IsState = &Actor::IsVoidState;
}

char *Actor::DumpCallTrace
	(
	const char *pszFmt,
	...
	) const

{
	// FIXME: stub
	STUB();
	return NULL;
}

void Actor::Init
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::FixAIParameters
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

bool Actor::AttackEntryAnimation
	(
	void
	)

{
	// FIXME: stub
	STUB();
	return false;
}

void Actor::CheckForThinkStateTransition
	(
	void
	)

{
	// FIXME: stub
	STUB();
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
						Vector pos = player->EyePosition();

						return G_SightTrace(
							pos,
							vec_zero,
							vec_zero,
							centroid,
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
		m_iCuriousAnimHint = 6;
		return true;
	}

	if( m_bEnemyIsDisguised || m_Enemy->m_bIsDisguised )
	{
		if( !m_bForceAttackPlayer && m_ThinkState != 4 )
		{
			m_iCuriousAnimHint = 6;
			return true;
		}
	}

	if( m_PotentialEnemies.GetCurrentVisibility() <= 0.999f )
	{
		m_iCuriousAnimHint = 6;
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
	// FIXME: stub
	STUB();
}

void Actor::ThinkStateTransitions
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::TransitionState
	(
	int iNewState,
	int iPadTime
	)

{
	// FIXME: stub
	STUB();
}

void Actor::ChangeAnim
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::UpdateSayAnim
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::UpdateUpperAnim
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::UpdateAnim
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::StoppedWaitFor
	(
	const_str name,
	bool bDeleting
	)

{
	// FIXME: stub
	STUB();
}

bool Actor::ValidGrenadePath
	(
	Vector& vFrom,
	Vector& vTo,
	Vector& vVel
	) const

{
	// FIXME: stub
	STUB();
	return false;
}

Vector Actor::CalcThrowVelocity
	(
	Vector& vFrom,
	Vector& vTo
	)

{
	// FIXME: stub
	STUB();
	return vec_zero;
}

Vector Actor::CanThrowGrenade
	(
	Vector& vFrom,
	Vector& vTo
	) const

{
	// FIXME: stub
	STUB();
	return vec_zero;
}

Vector Actor::CalcRollVelocity
	(
	Vector& vFrom,
	Vector& vTo
	)

{
	// FIXME: stub
	STUB();
	return vec_zero;
}

Vector Actor::CanRollGrenade
	(
	Vector& vFrom,
	Vector& vTo
	) const

{
	// FIXME: stub
	STUB();
	return vec_zero;
}

bool Actor::CanTossGrenadeThroughHint
	(
	GrenadeHint *pHint,
	Vector& vFrom,
	Vector& vTo,
	bool bDesperate,
	Vector *pvVel,
	eGrenadeTossMode *peMode
	) const

{
	// FIXME: stub
	STUB();
	return false;
}

Vector Actor::GrenadeThrowPoint
	(
	Vector& vFrom,
	Vector& vDelta,
	const_str csAnim
	)

{
	// FIXME: stub
	STUB();
	return vec_zero;
}

Vector Actor::CalcKickVelocity
	(
	Vector& vDelta,
	float fDist
	) const

{
	// FIXME: stub
	STUB();
	return vec_zero;
}

bool Actor::CanKickGrenade
	(
	Vector &vFrom,
	Vector &vTo,
	Vector &vFace,
	Vector *pvVel
	) const

{
	// FIXME: stub
	STUB();
	return false;
}

bool Actor::GrenadeWillHurtTeamAt
	(
	Vector& vTo
	) const

{
	// FIXME: stub
	STUB();
	return false;
}

bool Actor::CanGetGrenadeFromAToB
	(
	Vector& vFrom,
	Vector& vTo,
	bool bDesperate,
	Vector *pvVel,
	eGrenadeTossMode *peMode
	) const

{
	// FIXME: stub
	STUB();
	return false;
}

bool Actor::DecideToThrowGrenade
	(
	Vector& vTo,
	Vector *pvVel,
	eGrenadeTossMode *peMode
	) const

{
	// FIXME: stub
	STUB();
	return false;
}

void Actor::Grenade_EventFire
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::GenericGrenadeTossThink
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::CanSee
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::Think
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::PostThink
	(
	bool bDontFaceWall
	)

{
	// FIXME: stub
	STUB();
}

void Actor::SetMoveInfo
	(
	mmove_t *mm
	)

{
	// FIXME: stub
	STUB();
}

void Actor::GetMoveInfo
	(
	mmove_t *mm
	)

{
	// FIXME: stub
	STUB();
}

void Actor::DoFailSafeMove
	(
	vec3_t dest
	)

{
	// FIXME: stub
	STUB();
}

void Actor::TouchStuff
	(
	mmove_t *mm
	)

{
	// FIXME: stub
	STUB();
}

void Actor::ExtractConstraints
	(
	mmove_t *mm
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventGiveWeaponInternal
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventGiveWeapon
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventGetWeapon
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::FireWeapon
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
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
	// FIXME: stub
	STUB();
}

void Actor::EventShareGrenade
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
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
	// FIXME: stub
	STUB();
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
	// FIXME: stub
	STUB();
}

int Actor::PriorityForEventType
	(
	int iType
	)

{
	switch( iType )
	{
	case 1:
		return 7;
	case 2:
		return 5;
	case 3:
		return 6;
	case 4:
	case 5:
		return 3;
	case 6:
	case 7:
		return 4;
	case 8:
		return 1;
	case 9:
	case 10:
		return 2;
	case 11:
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
	float fMaxDistSquared,
	Entity *originator
	)

{
	// FIXME: stub
	STUB();
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
	// FIXME: stub
	STUB();
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
	// FIXME: stub
	STUB();
}

void Actor::GrenadeNotification
	(
	Entity *originator
	)

{
	// FIXME: stub
	STUB();
}

void Actor::SetGrenade
	(
	Entity *pGrenade
	)

{
	// FIXME: stub
	STUB();
}

void Actor::NotifySquadmateKilled
	(
	Sentient *pSquadMate,
	Sentient *pAttacker
	)

{
	// FIXME: stub
	STUB();
}

void Actor::RaiseAlertnessForEventType
	(
	int iType
	)

{
	// FIXME: stub
	STUB();
}

void Actor::RaiseAlertness
	(
	float fAmout
	)

{
	// FIXME: stub
	STUB();
}

bool Actor::CanSee
	(
	Entity *e1,
	float fov,
	float vision_distance
	)

{
	// FIXME: stub
	STUB();
	return false;
}

Vector Actor::GunPosition
	(
	void
	) const

{
	// FIXME: stub
	STUB();
	return vec_zero;
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
	// FIXME: stub
	STUB();
	return false;
}

bool Actor::EnemyInFOV
	(
	int iMaxDirtyTime
	)

{
	// FIXME: stub
	STUB();
	return false;
}

bool Actor::InFOV
	(
	Vector pos
	)

{
	// FIXME: stub
	STUB();
	return false;
}

bool Actor::InFOV
	(
	Entity *ent
	)

{
	// FIXME: stub
	STUB();
	return false;
}

bool Actor::CanSeeNoFOV
	(
	Entity *ent
	)

{
	// FIXME: stub
	STUB();
	return false;
}

bool Actor::CanSeeFOV
	(
	Entity *ent
	)

{
	// FIXME: stub
	STUB();
	return false;
}

bool Actor::CanSeeEnemyFOV
	(
	int iMaxFovDirtyTime,
	int iMaxSightDirtyTime
	)

{
	// FIXME: stub
	STUB();
	return false;
}

bool Actor::CanShoot
	(
	Entity *ent
	)

{
	// FIXME: stub
	STUB();
	return false;
}

bool Actor::CanSeeFrom
	(
	vec3_t pos,
	Entity *ent
	)

{
	// FIXME: stub
	STUB();
	return false;
}

bool Actor::CanSeeEnemy
	(
	int iMaxDirtyTime
	)

{
	// FIXME: stub
	STUB();
	return false;
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
	// FIXME: stub
	STUB();
}

void Actor::DefaultPain
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::HandlePain
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventPain
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::DefaultKilled
	(
	Event *ev,
	bool bPlayDeathAnim
	)

{
	// FIXME: stub
	STUB();
}

void Actor::HandleKilled
	(
	Event *ev,
	bool bPlayDeathAnim
	)

{
	// FIXME: stub
	STUB();
}

void Actor::DispatchEventKilled
	(
	Event *ev,
	bool bPlayDeathAnim
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventKilled
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventBeDead
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::DeathEmbalm
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::DeathSinkStart
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

bool Actor::NoticeShot
	(
	Sentient *pShooter,
	Sentient *pTarget,
	float fDist
	)

{
	// FIXME: stub
	STUB();
	return false;
}

bool Actor::NoticeFootstep
	(
	Sentient *pPedestrian
	)

{
	// FIXME: stub
	STUB();
	return false;
}

bool Actor::NoticeVoice
	(
	Sentient *pVocallist
	)

{
	// FIXME: stub
	STUB();
	return false;
}

void Actor::ClearLookEntity
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::LookAt
	(
	Vector& vec
	)

{
	// FIXME: stub
	STUB();
}

void Actor::LookAt
	(
	Listener *l
	)

{
	// FIXME: stub
	STUB();
}

void Actor::ForwardLook
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::LookAtLookEntity
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::IdleLook
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::IdleLook
	(
	vec3_t dir
	)

{
	// FIXME: stub
	STUB();
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
	// FIXME: stub
	STUB();
}

void Actor::EventEyesLookAt
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::NoPoint
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::IdlePoint
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::ClearPointEntity
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::PointAt
	(
	Vector& vec
	)

{
	// FIXME: stub
	STUB();
}

void Actor::PointAt
	(
	Listener* l
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventPointAt
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::ClearTurnEntity
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::TurnTo
	(
	Vector& vec
	)

{
	// FIXME: stub
	STUB();
}

void Actor::TurnTo
	(
	Listener *l
	)

{
	// FIXME: stub
	STUB();
}

void Actor::IdleTurn
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventTurnTo
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventSetTurnDoneError
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventGetTurnDoneError
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::LookAround
	(
	float fFovAdd
	)

{
	// FIXME: stub
	STUB();
}

bool Actor::SoundSayAnim
	(
	const_str name,
	int bLevelSayAnim
	)

{
	// FIXME: stub
	STUB();
	return false;
}

void Actor::EventSetAnim
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventIdleSayAnim
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventSayAnim
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventSetSayAnim
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventSetMotionAnim
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventSetAimMotionAnim
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventSetActionAnim
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventUpperAnim
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventSetUpperAnim
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventEndActionAnim
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventDamagePuff
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::SafeSetOrigin
	(
	vec3_t newOrigin
	)

{
	// FIXME: stub
	STUB();
}

void Actor::DoMove
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::AnimFinished
	(
	int slot,
	bool stop
	)

{
	// FIXME: stub
	STUB();
}

void Actor::AnimFinished
	(
	int slot
	)

{
	// FIXME: stub
	STUB();
}

void Actor::PlayAnimation
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::PlayScriptedAnimation
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::PlayNoclipAnimation
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::MoveDest
	(
	float fMoveSpeed
	)

{
	// FIXME: stub
	STUB();
}

void Actor::MovePath
	(
	float fMoveSpeed
	)

{
	// FIXME: stub
	STUB();
}

void Actor::MovePathGoal
	(
	float fMoveSpeed
	)

{
	// FIXME: stub
	STUB();
}

void Actor::Dumb
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::PhysicsOn
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::PhysicsOff
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventStart
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventGetMood
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventSetMood
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventGetAngleYawSpeed
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventSetAngleYawSpeed
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventSetAimTarget
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::UpdateAngles
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::SetLeashHome
(
Vector vHome
)

{
	// FIXME: stub
	STUB();
}

void Actor::AimAtTargetPos
	(
	void
	)

{
	// FIXME: stub
	STUB();
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
	// FIXME: stub
	STUB();
}

void Actor::FaceMotion
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::FaceDirectionDuringMotion
	(
	vec3_t vLook
	)

{
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
	// FIXME: stub
	STUB();
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
	// FIXME: stub
	STUB();
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
	// FIXME: stub
	STUB();
}

void Actor::GetLocalYawFromVector
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventGetSight
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventSetSight
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventGetHearing
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventSetHearing
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::ClearPatrolCurrentNode
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::NextPatrolCurrentNode
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::SetPatrolCurrentNode
	(
	Vector& vec
	)

{
	// FIXME: stub
	STUB();
}

void Actor::SetPatrolCurrentNode
	(
	Listener *l
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventSetPatrolPath
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventGetPatrolPath
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventSetPatrolWaitTrigger
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventGetPatrolWaitTrigger
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::ShowInfo_PatrolCurrentNode
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

bool Actor::MoveOnPathWithSquad
	(
	void
	)

{
	// FIXME: stub
	STUB();
	return false;
}

bool Actor::MoveToWaypointWithPlayer
	(
	void
	)

{
	// FIXME: stub
	STUB();
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
	// FIXME: stub
	STUB();
}

bool Actor::MoveToPatrolCurrentNode
	(
	void
	)

{
	// FIXME: stub
	STUB();
	return false;
}

void Actor::ClearAimNode
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::SetAimNode
	(
	Vector& vec
	)

{
	// FIXME: stub
	STUB();
}

void Actor::SetAimNode
	(
	Listener *l
	)

{
	// FIXME: stub
	STUB();
}

void Actor::ShowInfo_AimNode
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventSetAccuracy
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventGetAccuracy
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

int Actor::GetThinkType
	(
	const_str csName
	)

{
	// FIXME: stub
	STUB();
	return false;
}

void Actor::SetThink
	(
	int state,
	int think
	)

{
	// FIXME: stub
	STUB();
}

void Actor::SetThinkIdle
	(
	int think_idle
	)

{
	// FIXME: stub
	STUB();
}

void Actor::SetThinkState
	(
	int state,
	int level
	)

{

}

void Actor::EndCurrentThinkState
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::ClearThinkStates
	(
	void
	)

{
	// FIXME: stub
	STUB();
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
	// FIXME: stub
	STUB();
}

void Actor::EventSetFov
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventSetDestIdle
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventSetDestIdle2
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventSetTypeIdle
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventGetTypeIdle
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventSetTypeAttack
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventGetTypeAttack
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventSetTypeDisguise
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventGetTypeDisguise
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventSetDisguiseLevel
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventGetDisguiseLevel
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventSetTypeGrenade
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventGetTypeGrenade
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventSetMinDistance
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventGetMinDistance
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventSetMaxDistance
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventGetMaxDistance
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventGetLeash
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventSetLeash
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventGetInterval
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventSetInterval
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventDistToEnemy
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventGetRunAnim
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventGetWalkAnim
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventGetAnimName
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventSetAnimName
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventSetDisguiseRange
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventGetDisguiseRange
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventSetDisguisePeriod
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventGetDisguisePeriod
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventSetDisguiseAcceptThread
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventGetDisguiseAcceptThread
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventAttackPlayer
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::ForceAttackPlayer
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventSetAlarmNode
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventGetAlarmNode
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventSetAlarmThread
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventGetAlarmThread
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventSetSoundAwareness
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventGetSoundAwareness
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventSetGrenadeAwareness
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventGetGrenadeAwareness
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
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
	// FIXME: stub
	STUB();
}

void Actor::EventGetTurret
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventEnableEnemy
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventEnablePain
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventActivate
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventGetAmmoGrenade
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventSetAmmoGrenade
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventInterruptPoint
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventAnimScript
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventAnimScript_Scripted
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventAnimScript_Noclip
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventReload_mg42
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::SetPathWithLeash
	(
	Vector vDestPos,
	const char *description,
	int iMaxDirtyTime
	)

{
	// FIXME: stub
	STUB();
}

void Actor::SetPathWithLeash
	(
	SimpleEntity *pDestNode,
	const char *description,
	int iMaxDirtyTime
	)

{
	// FIXME: stub
	STUB();
}

void Actor::FindPathAwayWithLeash
	(
	vec3_t vAwayFrom,
	vec3_t vDirPreferred,
	float fMinSafeDist
	)

{
	// FIXME: stub
	STUB();
}

void Actor::FindPathNearWithLeash
(
vec3_t vNearbyTo,
float fCloseDistSquared
)

{
	// FIXME: stub
	STUB();
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
	// FIXME: stub
	STUB();
	return false;
}

void Actor::StrafeToAttack
(
float fDist,
vec3_t vDir
)

{

	// FIXME: stub
	STUB();
}

Vector Actor::GunTarget
	(
	bool bNoCollision
	)

{
	// FIXME: stub
	STUB();
	return vec_zero;
}

qboolean Actor::setModel
	(
	void
	)

{
	// FIXME: stub
	STUB();

	return Entity::setModel();
}

void Actor::EventSetHeadModel
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventGetHeadModel
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventSetHeadSkin
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventGetHeadSkin
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventSetNoIdle
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventGetNoIdle
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventGetEnemy
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventSetMaxNoticeTimeScale
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventGetMaxNoticeTimeScale
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventSetFixedLeash
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventGetFixedLeash
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::Holster
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::Unholster
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventHolster
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventUnholster
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventSoundDone
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventSound
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventIsEnemyVisible
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventGetEnemyVisibleChangeTime
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventGetLastEnemyVisibleTime
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventSetFallHeight
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventGetFallHeight
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventCanMoveTo
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventMoveDir
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventIntervalDir
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventResetLeash
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventTether
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventGetThinkState
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventGetEnemyShareRange
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventSetEnemyShareRange
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventGetKickDir
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventGetNoLongPain
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventSetNoLongPain
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventGetFavoriteEnemy
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventSetFavoriteEnemy
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventGetMumble
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventSetMumble
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventGetBreathSteam
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventSetBreathSteam
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventSetNextBreathTime
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventCalcGrenadeToss
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventGetNoSurprise
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventSetNoSurprise
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventGetSilent
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventSetSilent
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventGetAvoidPlayer
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventSetAvoidPlayer
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventGetLookAroundAngle
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventSetLookAroundAngle
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventHasCompleteLookahead
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventPathDist
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventCanShootEnemyFrom
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventCanShoot
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventSetInReload
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventGetInReload
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventSetReloadCover
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventBreakSpecial
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::GetVoiceType
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::SetVoiceType
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::ResolveVoiceType
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventSetBalconyHeight
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventGetBalconyHeight
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
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
	// FIXME: stub
	STUB();
}

void Actor::DontFaceWall
	(
	void
	)

{
	// FIXME: stub
	STUB();
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
	// FIXME: stub
	STUB();
}

void Actor::ClearStates
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::CheckUnregister
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::BecomeCorpse
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

void Actor::PathnodeClaimRevoked
	(
	PathNode *node
	)

{
	// FIXME: stub
	STUB();
}

void Actor::SetPathToNotBlockSentient
	(
	Sentient *pOther
	)

{
	// FIXME: stub
	STUB();
}

void Actor::EventSetMoveDoneRadius
	(
	Event *ev
	)

{
	// FIXME: stub
	STUB();
}


bool Actor::CalcFallPath
	(
	void
	)

{
	// FIXME: stub
	STUB();
	return false;
}

void Actor::IdleThink
	(
	void
	)

{
	// FIXME: stub
	STUB();
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
	// FIXME: stub
	STUB();
}

void Actor::DumpAnimInfo
	(
	void
	)

{
	// FIXME: stub
	STUB();
}

bool Actor::AutoArchiveModel
	(
	void
	)

{
	return false;
}
