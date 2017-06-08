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

// entity.cpp: Base class for all enities that are controlled by Sin.  If you have any
// object that should be called on a periodic basis and it is not an entity,
// then you have to have an dummy entity that calls it.
//
// An entity in Sin is any object that is not part of the world->  Any non-world
// object that is visible in Sin is an entity, although it is not required that
// all entities be visible to the player.  Some objects are basically just virtual
// constructs that act as an instigator of certain actions, for example, some
// triggers are invisible and cannot be touched, but when activated by other
// objects can cause things to happen.
//
// All entities are capable of receiving messages from Sin or from other entities.
// Messages received by an entity may be ignored, passed on to their superclass,
// or acted upon by the entity itself.  The programmer must decide on the proper
// action for the entity to take to any message.  There will be many messages
// that are completely irrelevant to an entity and should be ignored.  Some messages
// may require certain states to exist and if they are received by an entity when
// it these states don't exist may indicate a logic error on the part of the
// programmer or map designer and should be reported as warnings (if the problem is
// not severe enough for the game to be halted) or as errors (if the problem should
// not be ignored at any cost).
//

#include "entity.h"
#include "scriptmaster.h"
#include "sentient.h"
#include "misc.h"
#include "specialfx.h"
#include "object.h"
#include "player.h"
#include "weaputils.h"
#include "soundman.h"
#include "../qcommon/qfiles.h"
#include "g_spawn.h"

// Generic entity events
Event EV_SoundDone
	(
	"saydone",
	EV_DEFAULT,
	NULL,
	NULL,
	"Even used by sound-only special case of say to trigger waittill saydone"
	);
Event EV_Classname
	( 
	"classname" ,
	EV_DEFAULT,
	"s",
	"nameOfClass",
	"Determines what class to use for this entity,\n"
	"this is pre-processed from the BSP at the start\n"
	"of the level.",
	EV_NORMAL
	);
Event EV_SpawnFlags
	( 
	"spawnflags",
	EV_DEFAULT,
	"i",
	"flags",
	"spawnflags from the BSP, these are set inside the editor",
	EV_NORMAL
	);
Event EV_SetTeam
	( 
	"team",
	EV_DEFAULT,
	"s",
	"moveTeam",
	"used to make multiple entities move together.",
	EV_NORMAL
	);
Event EV_Trigger
	( 
	"trigger",
	EV_DEFAULT,
	"s",
	"name",
	"Trigger the specified target or entity.",
	EV_NORMAL
	);
Event EV_Activate
	( 
	"doActivate",
	EV_DEFAULT,
	"e",
	"activatingEntity",
	"General trigger event for all entities",
	EV_NORMAL
	);
Event EV_Use
	( 
	"doUse",
	EV_DEFAULT,
	"e",
	"activatingEntity",
	"sent to entity when it is used by another entity",
	EV_NORMAL
	);

Event EV_FadeNoRemove
	( 
	"fade",
	EV_DEFAULT,
	"F[0,]F[0,1]",
	"fadetime target_alpha",
	"Fade the entity's alpha, reducing it by 0.03\n"
	"every FRAMETIME, until it has faded out, does not remove the entity",
	EV_NORMAL
	);

Event EV_FadeOut
	( 
	"_fadeout",
	EV_DEFAULT,
	NULL,
	NULL,
	"Fade the entity's alpha and scale out, reducing it by 0.03\n"
	"every FRAMETIME, until it has faded out, removes the entity\n"
   "Once the entity has been completely faded, the entity is removed.",
	EV_NORMAL
	);

Event EV_Fade
	( 
	"fadeout",
	EV_DEFAULT,
	"F[0,]F[0,1]",
	"fadetime target_alpha",
	"Fade the entity's alpha and scale out, reducing it by 0.03\n"
	"every FRAMETIME, until it has faded out. If fadetime or\n"
   "target_alpha are defined, they will override the defaults.\n"
   "Once the entity has been completely faded, the entity is removed.",
	EV_NORMAL
	);
Event EV_FadeIn
	( 
	"fadein",
	EV_DEFAULT,
   "F[0,]F[0,1]",
   "fadetime target_alpha",
	"Fade the entity's alpha and scale in, increasing it by 0.03\n"
	"every FRAMETIME, until it has faded completely in to 1.0.\n"
   "If fadetime or target_alpha are defined, they will override\n"
   "the default values.",
	EV_NORMAL
	);
 Event EV_Killed
	( 
	"killed",
	EV_DEFAULT,
	"eievvviiii",
	"attacker damage inflictor position direction normal knockback damageflags meansofdeath location",
	"event which is sent to an entity once it as been killed\n\nLocation values:\n-1 General\n0 Pelvis\n1 Lower Torso\n2 Mid Torso\n3 Upper Torso\n4 Neck\n5 Head\n6 RUpperArm\n7 RForearm\n8 RHand\n9 LUpperArm\n10 LForearm\n11 LHand\n12 RThigh\n13 RCalf\n14 RFoot\n15 LThigh\n16 LCalf\n17 LFoot\n",
	EV_NORMAL
	);
Event EV_GotKill
	( 
	"gotkill" ,
	EV_DEFAULT,
	"eieib",
	"victim damage inflictor meansofdeath gib",
	"event sent to attacker when an entity dies",
	EV_NORMAL
	);
Event EV_Pain
	( 
	"pain",
	EV_DEFAULT,
	"eievvviiii",
	"attacker damage inflictor position direction normal knockback damageflags meansofdeath location",
	"used to inflict pain to an entity",
	EV_NORMAL
	);
Event EV_Damage
	( 
	"damage",
	EV_DEFAULT,
	"eievvviiii",
	"attacker damage inflictor position direction normal knockback damageflags meansofdeath location",
	"general damage event used by all entities\n\nLocation values:\n-1 General\n0 Pelvis\n1 Lower Torso\n2 Mid Torso\n3 Upper Torso\n4 Neck\n5 Head\n6 RUpperArm\n7 RForearm\n8 RHand\n9 LUpperArm\n10 LForearm\n11 LHand\n12 RThigh\n13 RCalf\n14 RFoot\n15 LThigh\n16 LCalf\n17 LFoot\n",
	EV_NORMAL
	);
Event EV_Stun
	( 
	"_stun",
	EV_DEFAULT,
	"f",
	"time",
	"Stun this entity for the specified time",
	EV_NORMAL
	);
Event EV_Kill
	( 
	"kill", 
	EV_CONSOLE,
	NULL,
	NULL,
	"console based command to kill yourself if stuck.",
	EV_NORMAL
	);
Event EV_Gib
	( 
	"gib",
	EV_DEFAULT,
	"iIFS",
	"number power scale gibmodel",
	"causes entity to spawn a number of gibs",
	EV_NORMAL
	);
Event EV_Hurt
	(
	"hurt",
	EV_DEFAULT,
	"iSV",
	"damage means_of_death direction",
	"Inflicts damage if the entity is damageable.  If the number of damage\n"
	"points specified in the command argument is greater or equal than the\n"
	"entity's current health, it will be killed or destroyed.",
	EV_NORMAL
	);
Event EV_Heal
(
	"heal",
	EV_DEFAULT,
	"f",
	"health",
	"Adds health to an entity, 0-1 fraction of max_health"
);

Event EV_TakeDamage
	( 
	"takedamage",
	EV_DEFAULT,
	NULL,
	NULL,
	"makes entity take damage.",
	EV_NORMAL
	);
Event EV_NoDamage
	( 
	"nodamage",
	EV_DEFAULT,
	NULL,
	NULL,
	"entity does not take damage.",
	EV_NORMAL
	);

Event EV_Stationary
	( 
	"stationary",
	EV_DEFAULT,
	NULL,
	NULL,
	"entity does not move, causes no physics to be run on it.",
	EV_NORMAL
	);

// Physics events
Event EV_MoveDone
	( 
	"movedone",
	EV_DEFAULT,
	"e",
	"finishedEntity",
	"Sent to commanding thread when done with move .",
	EV_NORMAL
	);
Event EV_Touch
	( 
	"doTouch",
	EV_DEFAULT,
	"e",
	"touchingEntity",
	"sent to entity when touched.",
	EV_NORMAL
	);
Event EV_Blocked
	( 
	"doBlocked",
	EV_DEFAULT,
	"e",
	"obstacle",
	"sent to entity when blocked.",
	EV_NORMAL
	);
Event EV_UseBoundingBox
	( 
	"usebbox",
	EV_DEFAULT,
	NULL,
	NULL,
	"do not perform perfect collision, use bounding box instead.",
	EV_NORMAL
	);
Event EV_Gravity
	( 
	"gravity",
	EV_DEFAULT,
	"f",
	"gravityValue",
	"Change the gravity on this entity",
	EV_NORMAL
	);
Event EV_Stop
	( 
	"stopped",
	EV_DEFAULT,
	NULL,
	NULL,
	"sent when entity has stopped bouncing for MOVETYPE_TOSS.",
	EV_NORMAL
	);
Event EV_Attach
	( 
	"attach",
	EV_DEFAULT,
	"esI",
	"parent tagname use_angles",
	"attach this entity to the parent's legs tag called tagname",
	EV_NORMAL
	);
Event EV_AttachModel
	( 
	"attachmodel",
	EV_DEFAULT,
	"ssFSBFFFFV",
	"modelname tagname scale targetname detach_at_death removetime fadeintime fadeoutdelay fadetime offset",
	"attach a entity with modelname to this entity to tag called tagname.\n"
	"scale           - scale of attached entities\n"
	"targetname      - targetname for attached entities\n"
	"detach_at_death - when entity dies, should this model be detached.\n"
	"removetime      - when the entity should be removed, if not specified, never.\n"
   "fadeintime      - time to fade the model in over.\n"
   "fadeoutdelay    - time to wait until we fade the attached model out\n"
   "fadeoutspeed    - time the model fades out over\n"
	"offset		     - vector offset for the model from the specified tag",
	EV_NORMAL
	);
Event EV_RemoveAttachedModel
	( 
	"removeattachedmodel",
	EV_DEFAULT,
	"s",
	"tagname",
	"Removes the model attached to this entity at the specified tag.",
	EV_NORMAL
	);
Event EV_Detach
	( 
	"detach",
	EV_DEFAULT,
	NULL,
	NULL,
	"detach this entity from its parent.",
	EV_NORMAL
	);

// script stuff
Event EV_Model
	( 
	"model",
	EV_DEFAULT,
	"e",
	"modelName",
	"set the model to modelName.",
	EV_NORMAL
	);
Event EV_Entity_GetBrushModel
	( 
	"brushmodel",
	EV_DEFAULT,
	NULL,
	NULL,
	"get the brush modelName.",
	EV_GETTER
	);
Event EV_Entity_GetModel
	( 
	"model",
	EV_DEFAULT,
	NULL,
	NULL,
	"get the modelName.",
	EV_GETTER
	);
Event EV_Entity_SetModel
	(
	"model",
	EV_DEFAULT,
	"e",
	"modelName",
	"set the model to modelName.",
	EV_SETTER
	);
Event EV_Hide
	( 
	"hide",
	EV_DEFAULT,
	NULL,
	NULL,
	"hide the entity, opposite of show.",
	EV_NORMAL
	);
Event EV_Show
	( 
	"show",
	EV_DEFAULT,
	NULL,
	NULL,
	"show the entity, opposite of hide.",
	EV_NORMAL
	);
Event EV_BecomeSolid
	( 
	"solid",
	EV_DEFAULT,
	NULL,
	NULL,
	"make solid.",
	EV_NORMAL
	);
Event EV_BecomeNonSolid
	( 
	"notsolid",
	EV_DEFAULT,
	NULL,
	NULL,
	"make non-solid.",
	EV_NORMAL
	);
Event EV_Ghost
	( 
	"ghost",
	EV_DEFAULT,
	NULL,
	NULL,
	"make non-solid but still send to client regardless of hide status.",
	EV_NORMAL
	);
Event EV_TouchTriggers
	( 
	"touchtriggers",
	EV_DEFAULT,
   NULL,
   NULL,
   "this entity should touch triggers.",
	EV_NORMAL
	);


Event EV_Sound
	( 
	"playsound",
	EV_DEFAULT,
	"sIFS",
	"soundName channel volume min_distance",
	"play a sound coming from this entity.\n"
	"default channel, CHAN_BODY.",
	EV_NORMAL
	);
Event EV_StopSound
	( 
	"stopsound",
	EV_DEFAULT,
	"I",
	"channel",
	"stop the current sound on the specified channel.\n"
	"default channel, CHAN_BODY.",
	EV_NORMAL
	);
Event EV_Bind
	( 
	"bind",
	EV_DEFAULT,
	"eB",
	"parent bind_child",
	"bind this entity to the specified entity.",
	EV_NORMAL
	);
Event EV_Unbind
	( 
	"unbind",
	EV_DEFAULT,
	NULL,
	NULL,
	"unbind this entity.",
	EV_NORMAL
	);
Event EV_Glue
	( 
	"glue",
	EV_DEFAULT,
	"ei",
	"parent glueAngles",
	"glue this entity to the specified entity.",
	EV_NORMAL
	);
Event EV_Unglue
	( 
	"unglue",
	EV_DEFAULT,
	NULL,
	NULL,
	"unglue this entity.",
	EV_NORMAL
	);
Event EV_JoinTeam
	( 
	"joinTeam",
	EV_DEFAULT,
	"e",
	"teamMember",
	"join a bind team.",
	EV_NORMAL
	);
Event EV_QuitTeam
	( 
	"quitTeam",
	EV_DEFAULT,
	NULL,
	NULL,
	"quit the current bind team",
	EV_NORMAL
	);
Event EV_Entity_SetHealthOnly
	( 
	"healthonly", 
	EV_CONSOLE | EV_CHEAT,
	"i",
	"newHealth",
	"set the health of the entity to newHealth without changing max_health",
	EV_NORMAL
	);
Event EV_Entity_SetHealthOnly2
	( 
	"healthonly", 
	EV_CONSOLE | EV_CHEAT,
	"i",
	"newHealth",
	"set the health of the entity to newHealth without changing max_health",
	EV_SETTER
	);
Event EV_Entity_SetMaxHealth
	( 
	"max_health", 
	EV_DEFAULT,
	"i",
	"newHealth",
	"sets max_health without changing health",
	EV_NORMAL
	);
Event EV_Entity_SetMaxHealth2
	( 
	"max_health", 
	EV_DEFAULT,
	"i",
	"newHealth",
	"sets max_health without changing health",
	EV_SETTER
	);
Event EV_Entity_GetMaxHealth
	( 
	"max_health", 
	EV_DEFAULT,
	NULL,
	NULL,
	"gets the entity's max health",
	EV_GETTER
	);
Event EV_SetHealth
	( 
	"health", 
	EV_CONSOLE | EV_CHEAT,
	"i",
	"newHealth",
	"set the health of the entity to newHealth",
	EV_NORMAL
	);
Event EV_SetHealth2
	( 
	"health", 
	EV_CONSOLE | EV_CHEAT,
	"i",
	"newHealth",
	"set the health of the entity to newHealth",
	EV_SETTER
	);
Event EV_Entity_GetHealth
	( 
	"health", 
	EV_DEFAULT,
	NULL,
	NULL,
	"entity's health",
	EV_GETTER
	);
Event EV_SetScale
	(
	"scale",
	EV_DEFAULT,
	"f",
	"newScale",
	"set the scale of the entity",
	EV_NORMAL
	);
Event EV_SetScale2
	(
	"scale",
	EV_DEFAULT,
	"f",
	"newScale",
	"set the scale of the entity",
	EV_SETTER
	);
Event EV_GetScale
	(
	"scale",
	EV_DEFAULT,
	"f",
	"newScale",
	"set the scale of the entity",
	EV_GETTER
	);
Event EV_SetSize
   ( 
   "setsize",
   EV_DEFAULT,
   "vv",
   "mins maxs",
   "Set the bounding box of the entity to mins and maxs.",
	EV_NORMAL
   );
Event EV_SetMins
   ( 
   "_setmins",
   EV_DEFAULT,
   "v",
   "mins",
   "Set the mins of the bounding box of the entity to mins.",
	EV_NORMAL
   );
Event EV_SetMaxs
   ( 
   "_setmaxs",
   EV_DEFAULT,
   "v",
   "maxs",
   "Set the maxs of the bounding box of the entity to maxs.",
	EV_NORMAL
   );
Event EV_SetAlpha
   ( 
   "alpha",
   EV_DEFAULT,
   "f",
   "newAlpha",
   "Set the alpha of the entity to alpha.",
	EV_NORMAL
   );
Event EV_SetKillTarget
   ( 
   "killtarget",
   EV_DEFAULT,
   "s",
   "targetName",
   "when dying kill entities with this targetName.",
	EV_NORMAL
   );
Event EV_SetMass
   ( 
   "mass",
   EV_DEFAULT,
   "f",
   "massAmount",
   "set the mass of this entity.",
	EV_NORMAL
   );

Event EV_LoopSound
   ( 
   "loopsound",
   EV_DEFAULT,
   "sFS",
   "soundName volume minimum_distance",
   "play a looped-sound with a certain volume and minimum_distance\n"
   "which is attached to the current entity.",
	EV_NORMAL
   );
Event EV_StopLoopSound
   ( 
   "stoploopsound",
   EV_DEFAULT,
   NULL,
   NULL,
   "Stop the looped-sound on this entity.",
	EV_NORMAL
   );

Event EV_SurfaceModelEvent
   ( 
   "surface",
   EV_DEFAULT,
   "sSSSSSS",
   "surfaceName parameter1 parameter2 parameter3 parameter4 parameter5 parameter6",
   "change a legs surface parameter for the given surface.\n"
   "+ sets the flag, - clears the flag\n"
   "Valid surface commands are:\n"
   "skin1 - set the skin1 offset bit\n"
   "skin2 - set the skin2 offset bit\n"
   "nodraw - don't draw this surface",
	EV_NORMAL
   );
// AI sound events
Event EV_BroadcastAIEvent
	(
	"ai_event",
	EV_DEFAULT,
	"SF",
	"type radius",
	"Let the AI know that this entity made a sound,\ntype is a string specifying what type of sound it is.\nradius determines how far the sound reaches.",
	EV_NORMAL
	);

// Lighting
Event EV_SetLight
   ( 
   "light",
   EV_DEFAULT,
   "ffff",
   "red green blue radius",
   "Create a dynmaic light on this entity.",
	EV_NORMAL
   );

Event EV_LightOn
   ( 
   "lightOn",
   EV_DEFAULT,
   NULL,
   NULL,
   "Turn the configured dynmaic light on this entity on.",
	EV_NORMAL
   );
Event EV_LightOff
   ( 
   "lightOff",
   EV_DEFAULT,
   NULL,
   NULL,
   "Turn the configured dynamic light on this entity off.",
	EV_NORMAL
   );
Event EV_LightStyle
   ( 
   "lightStyle",
   EV_DEFAULT,
   "i",
   "lightStyleIndex",
   "What light style to use for this dynamic light on this entity.",
	EV_NORMAL
   );
Event EV_LightRed
   ( 
   "lightRed",
   EV_DEFAULT,
   "f",
   "red",
   "Set the red component of the dynmaic light on this entity.",
	EV_NORMAL
   );
Event EV_LightGreen
   ( 
   "lightGreen",
   EV_DEFAULT,
   "f",
   "red",
   "Set the red component of the dynmaic light on this entity.",
	EV_NORMAL
   );
Event EV_LightBlue
   ( 
   "lightBlue",
   EV_DEFAULT,
   "f",
   "red",
   "Set the red component of the dynmaic light on this entity.",
	EV_NORMAL
   );
Event EV_LightRadius
   ( 
   "lightRadius",
   EV_DEFAULT,
   "f",
   "red",
   "Set the red component of the dynmaic light on this entity.",
	EV_NORMAL
   );

// Entity flag specific
Event EV_EntityFlags
   ( 
   "flags",
   EV_DEFAULT,
   "SSSSSS",
   "parameter1 parameter2 parameter3 parameter4 parameter5 parameter6",
   "Change the current entity flags.\n"
   "Valid flags are as follows:\n"
   "+ sets a flag, - clears a flag\n"
   "blood - should it bleed\n"
   "explode - should it explode when dead\n"
   "die_gibs - should it spawn gibs when dead\n"
   "god - makes the entity invincible\n",
	EV_NORMAL
   );
Event EV_EntityRenderEffects
   ( 
   "rendereffects",
   EV_DEFAULT,
   "SSSSSS",
   "parameter1 parameter2 parameter3 parameter4 parameter5 parameter6",
   "Change the current render effects flags.\n"
   "Valid flags are as follows:\n"
   "+ sets a flag, - clears a flag\n"
   "dontdraw - send the entity to the client, but don't draw\n"
   "betterlighting - do sphere based vertex lighting on the entity\n"
   "lensflare - add a lens glow to the entity at its origin\n"
   "viewlensflare - add a view dependent lens glow to the entity at its origin\n"
   "lightoffset - use the dynamic color values as a light offset to the model\n"
   "skyorigin - this entity is the portal sky origin\n"
   "minlight - this entity always has some lighting on it\n"
   "fullbright - this entity is always fully lit\n"
   "additivedynamiclight - the dynamic light should have an additive effect\n"
   "lightstyledynamiclight - the dynamic light uses a light style, use the\n"
   "'lightstyle' command to set the index of the light style to be used",
	EV_NORMAL
   );
Event EV_EntityEffects
   ( 
   "effects",
   EV_DEFAULT,
   "SSSSSS",
   "parameter1 parameter2 parameter3 parameter4 parameter5 parameter6",
   "Change the current entity effects flags.\n"
   "Valid flags are as follows:\n"
   "+ sets a flag, - clears a flag\n"
   "antisbjuice - anti sucknblow juice"
   "everyframe - process commands every time entity is rendered",
	EV_NORMAL
   );
Event EV_EntitySVFlags
   ( 
   "svflags",
   EV_DEFAULT,
   "SSSSSS",
   "parameter1 parameter2 parameter3 parameter4 parameter5 parameter6",
   "Change the current server flags.\n"
   "Valid flags are as follows:\n"
   "+ sets a flag, - clears a flag\n"
   "broadcast - always send this entity to the client",
	EV_NORMAL
   );

// Special Effects
Event EV_Censor
   ( 
   "censor",
   EV_DEFAULT,
   NULL,
   NULL,
   "used to ban certain contact when in parentmode\n",
	EV_NORMAL
   );
Event EV_Explosion
   ( 
   "explosionattack",
   EV_DEFAULT,
   "sS",
   "explosionModel tagName",
   "Spawn an explosion optionally from a specific tag",
	EV_NORMAL
   );

Event EV_ShaderEvent
   ( 
   "shader",
   EV_DEFAULT,
   "sfF",
   "shaderCommand argument1 argument2",
   "change a specific shader parameter for the entity.\n"
   "Valid shader commands are:\n"
   "translation [trans_x] [trans_y] - change the texture translation\n"
   "offset [offset_x] [offset_y] - change the texture offset\n"
   "rotation [rot_speed] - change the texture rotation speed\n"
   "frame [frame_num] - change the animated texture frame\n"
   "wavebase [base] - change the base parameter of the wave function\n"
   "waveamp [amp] - change the amp parameter of the wave function\n"
   "wavebase [phase] - change the phase parameter of the wave function\n"
   "wavefreq [freq] - change the frequency parameter of the wave function\n",
	EV_NORMAL
   );

Event EV_ScriptShaderEvent
   ( 
   "scriptshader",
   EV_DEFAULT,
   "sfF",
   "shaderCommand argument1 argument2",
   "alias for shader command, change a specific shader parameter for the entity.\n"
   "Valid shader commands are:\n"
   "translation [trans_x] [trans_y] - change the texture translation\n"
   "offset [offset_x] [offset_y] - change the texture offset\n"
   "rotation [rot_speed] - change the texture rotation speed\n"
   "frame [frame_num] - change the animated texture frame\n"
   "wavebase [base] - change the base parameter of the wave function\n"
   "waveamp [amp] - change the amp parameter of the wave function\n"
   "wavebase [phase] - change the phase parameter of the wave function\n"
   "wavefreq [freq] - change the frequency parameter of the wave function\n",
	EV_NORMAL
   );

Event EV_KillAttach
   ( 
   "killattach",
   EV_DEFAULT,
   NULL,
   NULL,
   "kill all the attached entities.",
	EV_NORMAL
   );
Event EV_DropToFloor
	( 
	"droptofloor",
	EV_DEFAULT,
	"F",
	"maxRange",
	"drops the entity to the ground, if maxRange is not specified 8192 is used.",
	EV_NORMAL
	);
Event EV_AddToSoundManager
	( 
	"_addtosoundmanager",
	EV_DEFAULT,
	NULL,
	NULL,
	"adds the current entity to the sound manager.",
	EV_NORMAL
	);
Event EV_SetControllerAngles
	( 
	"setcontrollerangles",
	EV_DEFAULT,
	"iv",
	"num angles",
	"Sets the control angles for the specified bone.",
	EV_NORMAL
	);
Event EV_DeathSinkStart
	( 
	"deathsinkstart",
	EV_DEFAULT,
	NULL,
	NULL,
	"Makes the entity sink into the ground and then get removed (this starts it).",
	EV_NORMAL
	);
Event EV_DeathSink
	( 
	"deathsinkeachframe",
	EV_DEFAULT,
	NULL,
	NULL,
	"Makes the entity sink into the ground and then get removed (this gets called each frame).",
	EV_NORMAL
	);
Event EV_DamageType
	( 
	"damage_type",
	EV_DEFAULT,
	"s",
   "meansofdeathstring",
	"Set the type of damage that this entity can take",
	EV_NORMAL
	);
Event EV_DetachAllChildren
(
	"detachallchildren",
	EV_DEFAULT,
	NULL,
	NULL,
	"Detach all the children from the entity."
);
Event EV_Entity_MovementStealth
(
	"movementstealth",
	EV_DEFAULT,
	"f",
	"scale",
	"Sets the current movement stealth scalar for the sentient"
);
Event EV_Entity_GetYaw
(
	"yaw",
	EV_DEFAULT,
	NULL,
	NULL,
	"entity's yaw",
	EV_GETTER
);
Event EV_Pusher
(
	"pusher",
	EV_DEFAULT,
	"eevf",
	"inflictor attacker direction force",
	"Push an entity in the specified direction with the specified force"
);
Event EV_NeverDraw
(
	"neverdraw",
	EV_DEFAULT,
	NULL,
	NULL,
	"Sets this entity to never draw"
);
Event EV_NormalDraw
(
	"normaldraw",
	EV_DEFAULT,
	NULL,
	NULL,
	"Sets this entity to normal draw"
);
Event EV_AlwaysDraw
(
	"alwaysdraw",
	EV_DEFAULT,
	NULL,
	NULL,
	"Sets this entity to always draw"
);
Event EV_IsTouching
(
	"istouching",
	EV_DEFAULT,
	"e",
	"ent",
	"returns 1 if the entities are touching, 0 if not",
	EV_RETURN
);
Event EV_CanSee
(
	"cansee",
	EV_DEFAULT,
	"eff",
	"entity fov vision_distance",
	"returns 1 if the entities can see eachother, 0 if not",
	EV_RETURN
);
Event EV_Entity_InPVS
(
	"inpvs",
	EV_DEFAULT,
	"e",
	"entity",
	"returns 1 if the entities have connected pvs, 0 if not",
	EV_RETURN
);
Event EV_SetShaderData
(
	"setshaderdata",
	EV_DEFAULT,
	"ff",
	"data0 data1",
	"sets the shader controllers for this entity.",
	EV_RETURN
);
Event EV_Entity_SetVelocity
(
	"velocity",
	EV_DEFAULT,
	"v",
	"velocity",
	"sets the velocity for this entity.",
	EV_SETTER
);
Event EV_Entity_GetVelocity
(
	"velocity",
	EV_DEFAULT,
	NULL,
	NULL,
	"gets the velocity for this entity.",
	EV_GETTER
);
Event EV_GetAVelocity
(
	"avelocity",
	EV_DEFAULT,
	NULL,
	NULL,
	"gets the angular velocity for this entity.",
	EV_GETTER
);
Event EV_GetTagAngles
(
	"gettagangles",
	EV_DEFAULT,
	"s",
	"tag_name",
	"Gets the world angles of the tag",
	EV_RETURN
);
Event EV_GetTagPosition
(
	"gettagposition",
	EV_DEFAULT,
	"s",
	"tag_name",
	"Gets the world position of the tag",
	EV_RETURN
);
Event EV_GetControllerAngles
(
	"getcontrollerangles",
	EV_DEFAULT,
	"i",
	"num",
	"Gets the control angles for the specified bone.",
	EV_RETURN
);
Event EV_ForceActivate
(
	"forceactivate",
	EV_DEFAULT,
	NULL,
	NULL,
	"Forces an entity to activate outside of the player's PVS"
);
Event EV_DisconnectPaths
(
	"disconnect_paths",
	EV_DEFAULT,
	NULL,
	NULL,
	"disonnects all navigation paths which intersect with the specified entity's volume"
);
Event EV_ConnectPaths
(
	"connect_paths",
	EV_DEFAULT,
	NULL,
	NULL,
	"Connects all navigation paths which intersect with the specified entity's volume"
);
Event EV_VolumeDamage
(
	"volumedamage",
	EV_DEFAULT,
	"f",
	"damage",
	"does damage to any entity within this's volume"
);
Event EV_Entity_AddImmunity
(
	"immune",
	EV_DEFAULT,
	"sSSSSS",
	"immune_string1 immune_string2 immune_string3 immune_string4 immune_string5 immune_string6",
	"Adds to the immunity list for this sentient."
);
Event EV_Entity_RemoveImmunity
(
	"removeimmune",
	EV_DEFAULT,
	"sSSSSS",
	"immune_string1 immune_string2 immune_string3 immune_string4 immune_string5 immune_string6",
	"Removes from the immunity list for this sentient."
);
Event EV_GetEntnum
(
	"entnum",
	EV_DEFAULT,
	NULL,
	NULL,
	"The entity's entity number",
	EV_GETTER
);
Event EV_Entity_SetRadnum
(
	"radnum",
	EV_DEFAULT,
	"i",
	"radnum",
	"set the radnum",
	EV_SETTER
);
Event EV_Entity_GetRadnum
(
	"radnum",
	EV_DEFAULT,
	NULL,
	NULL,
	"entity's radnum",
	EV_GETTER
);
Event EV_Entity_SetRotatedBbox
(
	"rotatedbbox",
	EV_DEFAULT,
	"i",
	"on_off",
	"Sets the entity's bbox to rotate with it.",
	EV_SETTER
);
Event EV_Entity_GetRotatedBbox
(
	"rotatedbbox",
	EV_DEFAULT,
	NULL,
	NULL,
	"Gets te entity's bbox to rotate with it.",
	EV_GETTER
);
Event EV_Entity_GetMaxs
	(
	"getmaxs",
	EV_DEFAULT,
	NULL,
	NULL,
	"Get the maxs of the bounding box of the entity to maxs.",
	EV_RETURN
	);
Event EV_Entity_GetMins
	(
	"getmins",
	EV_DEFAULT,
	NULL,
	NULL,
	"Get the mins of the bounding box of the entity to mins.",
	EV_RETURN
	);
Event EV_Entity_SetDHack
	(
	"depthhack",
	EV_DEFAULT,
	"i",
	"bEnable",
	"Enable or disable depth hack."
	);
Event EV_Entity_GetZone
	(
	"getzone",
	EV_DEFAULT,
	NULL,
	NULL,
	"Gets the current entity zone. NIL if the entity is not in a zone.",
	EV_RETURN
	);
Event EV_Entity_Zone
	(
	"zone",
	EV_DEFAULT,
	NULL,
	NULL,
	"Get the current entity zone. NIL if the entity is not in a zone.",
	EV_GETTER
	);
Event EV_Entity_IsInZone
	(
	"isinzone",
	EV_DEFAULT,
	"e",
	"zone",
	"Returns 1 if the entity is in the specified zone. 0 otherwise",
	EV_RETURN
	);
Event EV_Entity_SetHintRequireLookAt
	(
	"setuselookat",
	EV_DEFAULT,
	"b",
	"lookat",
	"Sets if the client must look at the entity to use it",
	EV_NORMAL
	);
Event EV_Entity_SetHintString
	(
	"sethintstring",
	EV_DEFAULT,
	"s",
	"string",
	"Sets the hint string for an entity when touching it. '&&1' will be replaced by the +use key on the client. If hideent/hide is called on this entity, the hint string won't be shown for the client",
	EV_NORMAL
	);
Event EV_Entity_SetShader
	(
	"setshader",
	EV_DEFAULT,
	"s",
	"shadername",
	"Sets a shader for the entity. An empty string will revert to the normal entity shader.",
	EV_NORMAL
	);

CLASS_DECLARATION( SimpleEntity, Entity, NULL )
	{
		{ &EV_Damage,						&Entity::DamageEvent },
		{ &EV_DamageType,					&Entity::DamageType },
		{ &EV_Kill,							&Entity::Kill },
		{ &EV_FadeNoRemove,					&Entity::FadeNoRemove },
		{ &EV_FadeOut,						&Entity::FadeOut },
		{ &EV_FadeIn,						&Entity::FadeIn },
		{ &EV_Fade,							&Entity::Fade },
		{ &EV_Hide,							&Entity::EventHideModel },
		{ &EV_Show,							&Entity::EventShowModel },
		{ &EV_BecomeSolid,					&Entity::BecomeSolid },
		{ &EV_BecomeNonSolid,				&Entity::BecomeNonSolid },
		{ &EV_Ghost,						&Entity::Ghost },
		{ &EV_TouchTriggers,				&Entity::TouchTriggersEvent },
		{ &EV_Sound,						&Entity::Sound },
		{ &EV_StopSound,					&Entity::StopSound },
		{ &EV_Entity_SetHealthOnly,			&Entity::EventSetHealthOnly },
		{ &EV_Entity_SetHealthOnly2,		&Entity::EventSetHealthOnly },
		{ &EV_Entity_SetMaxHealth,			&Entity::EventSetMaxHealth },
		{ &EV_Entity_SetMaxHealth2,			&Entity::EventSetMaxHealth },
		{ &EV_Entity_GetMaxHealth,			&Entity::EventGetMaxHealth },
		{ &EV_SetHealth,					&Entity::SetHealth },
		{ &EV_SetHealth2,					&Entity::SetHealth },
		{ &EV_Entity_GetHealth,				&Entity::GetHealth },
		{ &EV_SetSize,						&Entity::SetSize },
		{ &EV_SetMins,						&Entity::SetMins },
		{ &EV_SetMaxs,						&Entity::SetMaxs },
		{ &EV_SetScale,						&Entity::SetScale },
		{ &EV_SetScale2,					&Entity::SetScale },
		{ &EV_GetScale,						&Entity::GetScale },
		{ &EV_SetAlpha,						&Entity::SetAlpha },
		{ &EV_SetKillTarget,				&Entity::SetKillTarget },
		{ &EV_SetMass,						&Entity::SetMassEvent },
		{ &EV_LoopSound,					&Entity::LoopSound },
		{ &EV_StopLoopSound,				&Entity::StopLoopSound },
		{ &EV_Model,						&Entity::SetModelEvent },
		{ &EV_Entity_SetModel,				&Entity::SetModelEvent },
		{ &EV_Entity_GetBrushModel,			&Entity::GetBrushModelEvent },
		{ &EV_Entity_GetModel,				&Entity::GetModelEvent },
		{ &EV_SetLight,						&Entity::SetLight },
		{ &EV_LightOn,						&Entity::LightOn },
		{ &EV_LightOff,						&Entity::LightOff },
		{ &EV_LightRed,						&Entity::LightRed },
		{ &EV_LightGreen,					&Entity::LightGreen },
		{ &EV_LightBlue,					&Entity::LightBlue },
		{ &EV_LightRadius,					&Entity::LightRadius },
		{ &EV_LightStyle,					&Entity::LightStyle },
		{ &EV_EntityFlags,					&Entity::Flags },
		{ &EV_EntityEffects,				&Entity::Effects },
		{ &EV_EntitySVFlags,				&Entity::SVFlags },
		{ &EV_EntityRenderEffects,			&Entity::RenderEffects },
		{ &EV_BroadcastAIEvent,				&Entity::BroadcastAIEvent },
		{ &EV_SurfaceModelEvent,			&Entity::SurfaceModelEvent },
		{ &EV_Attach,						&Entity::AttachEvent },
		{ &EV_AttachModel,					&Entity::AttachModelEvent },
		{ &EV_RemoveAttachedModel,			&Entity::RemoveAttachedModelEvent },
		{ &EV_Detach,						&Entity::DetachEvent },
		{ &EV_TakeDamage,					&Entity::TakeDamageEvent },
		{ &EV_NoDamage,						&Entity::NoDamageEvent },
		{ &EV_Gravity,						&Entity::Gravity },                                       
		{ &EV_UseBoundingBox,				&Entity::UseBoundingBoxEvent },
		{ &EV_Hurt,							&Entity::HurtEvent },
		{ &EV_Heal,							&Entity::HealEvent },
		{ &EV_Classname,					&Entity::ClassnameEvent },
		{ &EV_SoundDone,					&Entity::EventSoundDone },
		{ &EV_SpawnFlags,					&Entity::SpawnFlagsEvent },
		{ &EV_SetTeam,						&Entity::SetTeamEvent },
		{ &EV_Trigger,						&Entity::TriggerEvent },
		{ &EV_Censor,						&Entity::Censor },
		{ &EV_Stationary,					&Entity::StationaryEvent },
		{ &EV_Explosion,					&Entity::Explosion },
		{ &EV_ShaderEvent,					&Entity::Shader },
		{ &EV_ScriptShaderEvent,			&Entity::Shader },
		{ &EV_KillAttach,					&Entity::KillAttach },
		{ &EV_DropToFloor,					&Entity::DropToFloorEvent },
		{ &EV_Bind,							&Entity::BindEvent },
		{ &EV_Unbind,						&Entity::EventUnbind },
		{ &EV_Glue,							&Entity::GlueEvent },
		{ &EV_Unglue,						&Entity::EventUnglue },
		{ &EV_JoinTeam,						&Entity::JoinTeam },
		{ &EV_QuitTeam,						&Entity::EventQuitTeam },
		{ &EV_AddToSoundManager,			&Entity::AddToSoundManager },
		{ &EV_SetControllerAngles,			&Entity::SetControllerAngles },
		{ &EV_DeathSinkStart,				&Entity::DeathSinkStart },
		{ &EV_DeathSink,					&Entity::DeathSink },
		{ &EV_DetachAllChildren,			&Entity::DetachAllChildren },
		{ &EV_Entity_MovementStealth,		&Entity::EventMovementStealth },
		{ &EV_Entity_GetYaw,				&Entity::GetYaw },
		{ &EV_Pusher,						&Entity::PusherEvent },
		{ &EV_NeverDraw,					&Entity::NeverDraw },
		{ &EV_NormalDraw,					&Entity::NormalDraw },
		{ &EV_AlwaysDraw,					&Entity::AlwaysDraw },
		{ &EV_IsTouching,					&Entity::IsTouching },
		{ &EV_Entity_GetVelocity,			&Entity::GetVelocity },
		{ &EV_GetAVelocity,					&Entity::GetAVelocity },
		{ &EV_Entity_SetVelocity,			&Entity::SetVelocity },
		{ &EV_CanSee,						&Entity::CanSee },
		{ &EV_Entity_InPVS,					&Entity::EventInPVS },
		{ &EV_SetShaderData,				&Entity::SetShaderData },
		{ &EV_ForceActivate,				&Entity::ForceActivate },
		{ &EV_ScriptThread_Trace,			&Entity::EventTrace },
		{ &EV_ScriptThread_SightTrace,		&Entity::EventSightTrace },
		{ &EV_DisconnectPaths,				&Entity::EventDisconnectPaths },
		{ &EV_Remove,						&Entity::Remove },
		{ &EV_Delete,						&Entity::Remove },
		{ &EV_ScriptRemove,					&Entity::Remove },
		{ &EV_ConnectPaths,					&Entity::EventConnectPaths },
		{ &EV_VolumeDamage,					&Entity::EventVolumeDamage },
		{ &EV_Entity_AddImmunity,			&Entity::AddImmunity },
		{ &EV_Entity_RemoveImmunity,		&Entity::RemoveImmunity },
		{ &EV_GetEntnum,					&Entity::EventGetEntnum },
		{ &EV_Entity_SetRadnum,				&Entity::EventSetRadnum },
		{ &EV_Entity_GetRadnum,				&Entity::EventGetRadnum },
		{ &EV_Entity_SetRotatedBbox,		&Entity::EventSetRotatedBbox },
		{ &EV_Entity_GetRotatedBbox,		&Entity::EventGetRotatedBbox },
		{ &EV_GetTagAngles,					&Entity::GetTagAngles },
		{ &EV_GetTagPosition,				&Entity::GetTagPosition },
		{ &EV_GetControllerAngles,			&Entity::GetControllerAngles },
		{ &EV_Entity_GetMaxs,				&Entity::GetMaxs },
		{ &EV_Entity_GetMins,				&Entity::GetMins },
		{ &EV_Entity_GetZone,				&Entity::GetZone },
		{ &EV_Entity_Zone,					&Entity::GetZone },
		{ &EV_Entity_IsInZone,				&Entity::IsInZone },
		{ &EV_Entity_SetDHack,				&Entity::SetDepthHack },
		{ &EV_Entity_SetHintRequireLookAt,	&Entity::SetHintRequireLookAt },
		{ &EV_Entity_SetHintString,			&Entity::SetHintString },
		{ &EV_Entity_SetShader,				&Entity::SetShader },
		{ NULL, NULL }
	};

Entity::Entity()
{
	int i;

	edict = level.AllocEdict( this );
	client = edict->client;
	entnum = edict->s.number;
	radnum = -1;

	entflags |= EF_ENTITY;

	if( LoadingSavegame )
	{
		return;
	}

	edict->tiki = NULL;

	// spawning variables
	spawnflags = level.spawnflags;
	level.spawnflags = 0;

	// rendering variables
	setAlpha( 1.0f );
	setScale( 1.0f );

	// physics variables
	mass = 0;
	gravity = 1.0;
	groundentity = NULL;
	groundcontents = 0;
	velocity = vec_zero;
	avelocity = vec_zero;
	edict->clipmask = MASK_USABLE;

	m_iNumGlues = 0;
	m_bGlueAngles = false;
	m_pGlueMaster = NULL;

	// team variables
	teamchain = NULL;
	teammaster = NULL;

	// bind variables
	bindmaster = NULL;

	// this is an generic entity
	edict->s.eType = ET_GENERAL;

	setContents( 0 );

	edict->s.parent = ENTITYNUM_NONE;
	edict->r.ownerNum = ENTITYNUM_NONE;

	// model binding variables
	numchildren = 0;

	for( i = 0; i < MAX_MODEL_CHILDREN; i++ )
		children[ i ] = ENTITYNUM_NONE;

	for( int i = 0; i < MAX_GLUE_CHILDREN; i++ )
		m_pGluesFlags[ i ] = 0;

	setOrigin( vec_zero );
	origin.copyTo( edict->s.origin2 );

	setAngles( vec_zero );

	setMoveType( MOVETYPE_NONE );
	setSolidType( SOLID_NOT );

	// Character state
	health = 0;
	max_health = 0;
	deadflag = DEAD_NO;
	flags = 0;

	// underwater variables
	watertype = 0;
	waterlevel = 0;

	// Pain and damage variables
	takedamage = DAMAGE_NO;
	enemy = NULL;
	pain_finished = 0;
	damage_debounce_time = 0;
	damage_type = -1;

	detach_at_death = qtrue;

	// Light variables
	lightRadius = 0;

	stealthMovementScale = 1.0f;
	m_iNumBlockedPaths = 0;
	m_BlockedPaths = NULL;

	m_bHintRequiresLookAt = true;

	// Misc
	m_bBindChilds = false;
}

Entity::~Entity()
{
	Container<Entity *> bindlist;
	Container<Entity *> gluelist;
	Entity *ent;
	int num;
	int i;

	// unbind any entities that are bound to me
	// can't unbind within this loop, so make an array
	// and unbind them outside of it.
	num = 0;
	for( ent = teamchain; ent; ent = ent->teamchain )
	{
		if( ent->bindmaster == this )
		{
			bindlist.AddObject( ent );
		}
	}

	num = bindlist.NumObjects();
	for( i = 1; i <= num; i++ )
	{
		bindlist.ObjectAt( i )->unbind();
	}

	// detach all glued entities
	num = 0;
	for( i = 0; i < m_iNumGlues; i++ )
	{
		if( m_pGlues[ i ] && m_pGlues[ i ]->m_pGlueMaster == this )
		{
			gluelist.AddObject( m_pGlues[ i ] );
		}
	}

	num = gluelist.NumObjects();
	for( i = 1; i <= num; i++ )
	{
		gluelist.ObjectAt( i )->unglue();
	}

	bindlist.FreeObjectList();
	gluelist.FreeObjectList();

	unbind();
	quitTeam();

	unglue();
	detach();

	//
	// go through and set our children
	//
	num = numchildren;
	for( i = 0; ( i < MAX_MODEL_CHILDREN ) && num; i++ )
	{
		if( children[ i ] == ENTITYNUM_NONE )
		{
			continue;
		}
		ent = G_GetEntity( children[ i ] );
		if( ent )
		{
			ent->PostEvent( EV_Remove, 0 );
		}
		num--;
	}

	if( m_BlockedPaths )
	{
		delete m_BlockedPaths;
		m_BlockedPaths = 0;
	}

	level.FreeEdict( edict );

	entflags &= ~EF_ENTITY;
}

inline void Entity::Archive
	(
	Archiver &arc
	)
{
	int tempInt;
	int i;

	SimpleEntity::Archive( arc );

	G_ArchiveEdict( arc, edict );

	arc.ArchiveInteger( &entnum );
	arc.ArchiveInteger( &radnum );
	arc.ArchiveInteger( &spawnflags );

	arc.ArchiveString( &model );
	if( arc.Loading() && model.length() )
	{
		setModel( model );
	}

	arc.ArchiveVector( &mins );
	arc.ArchiveVector( &maxs );
	arc.ArchiveVector( &absmin );
	arc.ArchiveVector( &absmax );
	arc.ArchiveVector( &velocity );
	arc.ArchiveVector( &accel );
	arc.ArchiveVector( &avelocity );
	arc.ArchiveVector( &aaccel );
	arc.ArchiveVector( &size );
	arc.ArchiveInteger( &movetype );
	arc.ArchiveInteger( &mass );
	arc.ArchiveFloat( &gravity );
	arc.ArchiveRaw( orientation, sizeof( orientation ) );

	if( arc.Saving() )
	{
		if( groundentity )
		{
			tempInt = groundentity - g_entities;
		}
		else
		{
			tempInt = -1;
		}
	}

	arc.ArchiveInteger( &tempInt );

	if( arc.Loading() )
	{
		if( tempInt == -1 )
		{
			groundentity = NULL;
		}
		else
		{
			groundentity = &g_entities[ tempInt ];
		}
	}

	arc.ArchiveRaw( &groundplane, sizeof( groundplane ) );
	arc.ArchiveInteger( &groundcontents );

	arc.ArchiveInteger( &numchildren );
	arc.ArchiveRaw( children, sizeof( children ) );

	arc.ArchiveFloat( &lightRadius );

	arc.ArchiveString( &moveteam );
	arc.ArchiveObjectPointer( ( Class ** )&teamchain );
	arc.ArchiveObjectPointer( ( Class ** )&teammaster );

	arc.ArchiveObjectPointer( ( Class ** )&bindmaster );
	arc.ArchiveBoolean( &bind_use_my_angles );
	arc.ArchiveVector( &localorigin );
	arc.ArchiveVector( &localangles );

	// add to target list to rebuild targetlists
	arc.ArchiveString( &killtarget );

	arc.ArchiveFloat( &health );
	arc.ArchiveFloat( &max_health );
	arc.ArchiveInteger( &deadflag );
	arc.ArchiveInteger( &flags );

	arc.ArchiveInteger( &watertype );
	arc.ArchiveInteger( &waterlevel );

	ArchiveEnum( takedamage, damage_t );

	arc.ArchiveInteger( &m_iNumGlues );

	for( i = 0; i < MAX_MODEL_CHILDREN; i++ )
	{
		arc.ArchiveSafePointer( &m_pGlues[ i ] );
		arc.ArchiveInteger( &m_pGluesFlags[ i ] );
	}

	arc.ArchiveObjectPointer( ( Class ** )&m_pGlueMaster );
	arc.ArchiveBool( &m_bGlueAngles );
	arc.ArchiveBoolean( &detach_at_death );

	arc.ArchiveFloat( &stealthMovementScale );
	arc.ArchiveInteger( &m_iNumBlockedPaths );

	if( arc.Loading() )
	{
		m_BlockedPaths = new pathway_ref[ m_iNumBlockedPaths ];
	}

	for( i = 0; i < m_iNumBlockedPaths; i++ )
	{
		arc.ArchiveShort( &m_BlockedPaths[ i ].from );
		arc.ArchiveShort( &m_BlockedPaths[ i ].to );
	}

	immunities.Archive( arc );
}

bool Entity::AutoArchiveModel
	(
	void
	)

{
	return true;
}

void Entity::SetEntNum
   (
   int num
   )

   {
   if ( edict )
      {
      level.FreeEdict( edict );
      }

   level.spawn_entnum = num;
   level.AllocEdict( this );
	client = edict->client;
	entnum = edict->s.number;
   }

void Entity::ClassnameEvent
	(
	Event *ev
   )

   {
	strncpy( edict->entname, ev->GetString( 1 ), sizeof( edict->entname ) - 1 );
	}

void Entity::SpawnFlagsEvent
	(
	Event *ev
   )

   {
	// spawning variables
	spawnflags = ev->GetInteger( 1 );
   if ( spawnflags & SPAWNFLAG_DETAIL )
		{
      edict->s.renderfx |= RF_DETAIL;
		}
	}

bool monkeycheck = false;
cvar_t *thereisnomonkey;

qboolean Entity::CheckEventFlags( Event *event )
{
	int flags = GetFlags( event );

	// Special handling of console events
	if( !( flags & ( EV_CONSOLE | EV_CHEAT ) ) )
	{
		if( isSubclassOf( Entity ) )
		{
			Entity *ent;

			ent = ( Entity * )this;
			gi.SendServerCommand( ent->edict - g_entities, "print \"Command '%s' not available from console\n\"", event->getName().c_str() );
		}

		// don't process
		return false;
	}

	// don't allow console cheats unless the server says it's ok.

	if( ( flags & EV_CHEAT ) )
	{
		if( !monkeycheck )
		{
			thereisnomonkey = gi.Cvar_Get( "thereisnomonkey", "0", CVAR_TEMP );
			monkeycheck = true;
		}

		if( !thereisnomonkey->integer )
		{
			gi.Cvar_Set( "cheats", "0" );
		}

		if( !sv_cheats->integer )
		{
			if( isSubclassOf( Entity ) )
			{
				Entity *ent;
				const char *szConv = gi.LV_ConvertString( "You must run the server with '+set cheats 1' to enable this command." );

				ent = ( Entity * )this;
				gi.SendServerCommand( ent->edict - g_entities, "print \"%s\n\"", szConv );
			}

			// don't process
			return false;
		}
	}

	// ok to process
	return true;
}

void Entity::SetKillTarget
	(
	const char *text
	)

	{
   if ( text )
		{
      killtarget = text;
		}
	else
		{
		killtarget = "";
		}
	}

qboolean Entity::setModel
	(
	void
	)

{
	level.skel_index[ edict->s.number ] = -1;
	return gi.setmodel( edict, model );
}

void Entity::SetSize
	(
	void
	)

{
	mins = edict->r.mins;
	maxs = edict->r.maxs;

	size = maxs - mins;

	edict->r.radius = size.length() * 0.5;
	edict->radius2 = edict->r.radius * edict->r.radius;
}

void Entity::setModel
	(
	const char *mdl
	)

{
	if( strcmp( mdl, "" ) )
	{
		if( *mdl == '*' )
		{
			model = mdl;

			gi.SetBrushModel( edict, mdl );
			if( ( edict->solid == SOLID_BSP ) && !edict->s.modelindex )
			{
				const char *name;

				name = getClassID();
				if( !name )
				{
					name = getClassname();
				}
				gi.DPrintf( "%s with SOLID_BSP and no model - '%s'(%d)\n", name, targetname.c_str(), entnum );

				// Make it non-solid so that the collision code doesn't kick us out.
				setSolidType( SOLID_NOT );
			}

			mins = edict->r.mins;
			maxs = edict->r.maxs;

			size = maxs - mins;
			edict->r.radius = size.length() * 0.5f;
			edict->radius2 = edict->r.radius * edict->r.radius;
			return;
		}
	}

	model = CanonicalTikiName( mdl );

	if( !setModel() )
	{
		Com_Printf( "^~^~^ Bad model name '%s'\n", mdl );
		return;
	}

	mins = vec_zero;
	maxs = vec_zero;

	for( int i = 0; i < MAX_FRAMEINFOS; i++ )
	{
		edict->s.frameInfo[ i ].index = 0;
		edict->s.frameInfo[ i ].time = 0;
		edict->s.frameInfo[ i ].weight = 0;
	}

	if( !strcmp( mdl, "" ) )
	{
		mins = edict->r.mins;
		maxs = edict->r.maxs;

		size = maxs - mins;
		edict->r.radius = size.length() * 0.5f;
		edict->radius2 = edict->r.radius * edict->r.radius;
		return;
	}

	edict->s.frameInfo[ 0 ].index = gi.Anim_NumForName( edict->tiki, "idle" );
	if( edict->s.frameInfo[ 0 ].index < 0 )
	{
		edict->s.frameInfo[ 0 ].index = 0;
	}

	edict->s.frameInfo[ 0 ].weight = 1.0f;
	edict->s.frameInfo[ 0 ].time = 0;

	ProcessInitCommands();

	mins = edict->r.mins;
	maxs = edict->r.maxs;

	size = maxs - mins;
	edict->r.radius = size.length() * 0.5f;
	edict->radius2 = edict->r.radius * edict->r.radius;

	if( edict->tiki && !mins.length() && !maxs.length() )
	{
		vec3_t tempmins, tempmaxs;
		gi.CalculateBounds( edict->tiki, edict->s.scale, tempmins, tempmaxs );
		setSize( tempmins, tempmaxs );
	}
}

void Entity::ProcessInitCommands( void )
{
	dtikianim_t *a;

	if ( LoadingSavegame )
	{
		// Don't process init commands when loading a savegame since
		// it will cause items to be added to inventories unnecessarily.
		// All variables affected by the init commands will be set
		// by the unarchive functions.
		//
		// we do want to process the cache commands though regardless
		return;
	}

	a = edict->tiki->a;

	if( a->num_server_initcmds )
	{
		int i, j;
		Event *event;
		for( i = 0; i < a->num_server_initcmds; i++ )
		{
			event = new Event( a->server_initcmds[ i ].args[ 0 ] );

			for( j = 1; j < a->server_initcmds[ i ].num_args; j++ )
			{
				event->AddToken( a->server_initcmds[ i ].args[ j ] );
			}

			if( !ProcessEvent( event ) )
			{
				Com_Printf( "^~^~^ Entity::ProcessInitCommands: Bad init server command '%s' in '%s'\n", a->server_initcmds[ i ].args[ 0 ], edict->tiki->name );
			}
		}
	}
}

void Entity::EventHideModel
	(
	Event *ev
	)

	{
	hideModel();
	}

void Entity::EventShowModel
	(
	Event *ev
	)

	{
	showModel();
	}

void Entity::SetTeamEvent
	(
	Event *ev
	)

	{
   moveteam = ev->GetString( 1 );
	}

void Entity::TriggerEvent
	(
	Event *ev
	)

{
	const char	*name;
	Event		   *event;
	Entity	   *ent;
	ConSimple	*tlist;
	int         i;
	int         num;

	name = ev->GetString( 1 );

	// Check for object commands
	if( name && name[ 0 ] == '$' )
	{
		str sName = str( name + 1 );

		tlist = world->GetTargetList( sName );
		num = tlist->NumObjects();
		for( i = 1; i <= num; i++ )
		{
			ent = ( Entity * )tlist->ObjectAt( i ).Pointer();

			assert( ent );

			event = new Event( EV_Activate );

			event->AddEntity( this );
			ent->ProcessEvent( event );
		}
	}
	else if( name[ 0 ] == '*' )   // Check for entnum commands
	{
		if( !IsNumeric( &name[ 1 ] ) )
		{
			gi.Printf( "Expecting numeric value for * command, but found '%s'\n", &name[ 1 ] );
		}
		else
		{
			ent = G_GetEntity( atoi( &name[ 1 ] ) );
			if( ent )
			{
				event = new Event( EV_Activate );

				event->AddEntity( this );
				ent->ProcessEvent( event );
			}
			else
			{
				gi.Printf( "Entity not found for * command\n" );
			}
		}
		return;
	}
	else
	{
		gi.Printf( "Invalid entity reference '%s'.\n", name );
	}
}

void Entity::setAlpha
	(
	float alpha
	)

	{
	if ( alpha > 1.0f )
		{
		alpha = 1.0f;
		}
	if ( alpha < 0 )
		{
		alpha = 0;
		}
   edict->s.alpha = alpha;
	}

void Entity::setScale
	(
	float scale
	)

{
	edict->s.scale = scale;
}

void Entity::setContentsSolid
	(
	void
	)

{
	setContents( CONTENTS_BBOX );
}

void Entity::setSolidType
	(
	solid_t type
	)

{
	if(
		( !LoadingSavegame ) &&
		( type == SOLID_BSP ) &&
		( this != world ) &&
		(
			!model.length() ||
			(
				( model[ 0 ] != '*' ) &&
				( !strstr( model.c_str(), ".bsp" ) )
			)
		)
	)
	{
		gi.Printf( "setSolidType", "SOLID_BSP entity at x%.2f y%.2f z%.2f with no BSP model", origin[ 0 ], origin[ 1 ], origin[ 2 ] );
	}
	edict->solid = type;

	//
	// set the appropriate contents type
	if( edict->solid == SOLID_BBOX )
	{
		if( !getContents() )
			setContentsSolid();
	}
	else if( edict->solid == SOLID_NOT )
	{
		if( getContents() & ( CONTENTS_CORPSE | CONTENTS_BODY | CONTENTS_UNKNOWN3 | CONTENTS_UNKNOWN2 | CONTENTS_NOBOTCLIP | CONTENTS_BBOX | CONTENTS_NOTTEAM2 | CONTENTS_NOTTEAM1 | CONTENTS_LADDER ) )
			setContents( 0 );
	}
	else if( edict->solid == SOLID_BSP )
	{
		if( !getContents() )
			setContents( CONTENTS_SOLID );
	}

	link();
}

void Entity::setSize
	(
	Vector min,
	Vector max
	)

{
	Vector delta;

	if( flags & FL_ROTATEDBOUNDS )
	{
		vec3_t tempmins, tempmaxs;

		//
		// rotate the mins and maxs for the model
		//
		min.copyTo( tempmins );
		max.copyTo( tempmaxs );

		CalculateRotatedBounds2( edict->mat, tempmins, tempmaxs );

		VectorCopy( tempmins, edict->r.mins );
		VectorCopy( tempmaxs, edict->r.maxs );

		SetSize();
	}
	else
	{
		if( ( min == edict->r.mins ) && ( max == edict->r.maxs ) )
		{
			return;
		}

		VectorCopy( min, edict->r.mins );
		VectorCopy( max, edict->r.maxs );

		SetSize();

		//
		// get the full mins and maxs for this model
		//
		if( edict->tiki )
		{
			vec3_t fullmins, fullmaxs;
			Vector delta;

			gi.CalculateBounds( edict->tiki, edict->s.scale, fullmins, fullmaxs );

			delta = Vector( fullmaxs ) - Vector( fullmins );
			edict->r.radius = delta.length() * 0.5;
			edict->radius2 = edict->r.radius * edict->r.radius;
		}
	}

	link();
}

Vector Entity::getLocalVector
	(
	Vector vec
	)

	{
	Vector pos;

	pos[ 0 ] = vec * orientation[ 0 ];
	pos[ 1 ] = vec * orientation[ 1 ];
	pos[ 2 ] = vec * orientation[ 2 ];

	return pos;
	}

void Entity::link
	(
	void
	)

{
	gi.LinkEntity( edict );
	absmin = edict->r.absmin;
	absmax = edict->r.absmax;
	centroid = ( absmin + absmax ) * 0.5;
	centroid.copyTo( edict->r.currentOrigin );

	// If this has a parent, then set the areanum the same
	// as the parent's
	if( edict->s.parent != ENTITYNUM_NONE )
	{
		edict->r.areanum = g_entities[ edict->s.parent ].r.areanum;
	}
}

void Entity::addOrigin
	(
	Vector add
	)

{
	setLocalOrigin( localorigin + add );
}

void Entity::setOrigin
	(
	void
	)

{
	Entity *ent;

	if( edict->s.parent == ENTITYNUM_NONE )
	{
		setLocalOrigin( localorigin );
	}
	// If entity has a parent, then set the origin as the
	// centroid of the parent, and set edict->s.netorigin
	// as the local origin of the entity which will be used
	// to position this entity on the client.
	else
	{
		Vector forward, left, up;

		VectorClear( edict->s.netorigin );
		ent = ( Entity * )G_GetEntity( edict->s.parent );

		ent->GetTag( edict->s.tag_num & TAG_MASK, &origin, &forward, &left, &up );

		origin += edict->s.attach_offset[ 0 ] * forward;
		origin += edict->s.attach_offset[ 1 ] * left;
		origin += edict->s.attach_offset[ 2 ] * up;

		localorigin = vec_zero;

		updateOrigin();
	}
}

void Entity::setLocalOrigin
	(
	Vector org
	)

{
	if( m_pGlueMaster )
	{
		org = m_pGlueMaster->origin;
		velocity = m_pGlueMaster->velocity;
	}

	if( bindmaster )
	{
		localorigin = org;

		if( bind_use_my_angles )
			MatrixTransformVector( localorigin, orientation, origin );
		else
			MatrixTransformVector( localorigin, bindmaster->orientation, origin );

		origin += bindmaster->origin;
		origin.copyTo( edict->s.netorigin );
	}
	else
	{
		origin = org;
		localorigin = org;
		origin.copyTo( edict->s.netorigin );
	}

	updateOrigin();
}

void Entity::setOrigin
	(
	Vector org
	)

{
	if( m_pGlueMaster )
	{
		org = m_pGlueMaster->origin;
		velocity = m_pGlueMaster->velocity;
	}

	if( bindmaster )
	{
		origin = org;

		org -= bindmaster->origin;

		if ( bind_use_my_angles )
			MatrixTransformVectorRight( orientation, org, localorigin );
		else
			MatrixTransformVectorRight( bindmaster->orientation, org, localorigin );

		origin.copyTo( edict->s.netorigin );
	}
	else
	{
		if( edict->s.parent != ENTITYNUM_NONE )
		{
			detach();
		}

		origin = org;
		localorigin = org;
		origin.copyTo( edict->s.netorigin );
	}

	updateOrigin();
}

void Entity::setOriginEvent
	(
	Vector org
	)

{
	velocity = vec_zero;
	setOrigin( org );
}

void Entity::updateOrigin
	(
	void
	)

{
	Entity *ent;
	int i, num;

	origin.copyTo( edict->s.origin );

	link();

	//
	// go through and set our children
	//
	num = numchildren;
	for( i = 0; ( i < MAX_MODEL_CHILDREN ) && num; i++ )
	{
		if( children[ i ] == ENTITYNUM_NONE )
		{
			continue;
		}
		ent = ( Entity * )G_GetEntity( children[ i ] );
		ent->setOrigin();
		num--;
	}

	for( i = 0; i < m_iNumGlues; i++ )
	{
		m_pGlues[ i ]->setOrigin();
	}
}

void Entity::ShowInfo
	(
	float fDot,
	float fDist
	)

{

}

void Entity::GetRawTag
	(
	int    tagnum,
	orientation_t *orient
	)

{
	*orient = G_TIKI_Orientation( edict, tagnum & TAG_MASK );
}

qboolean Entity::GetRawTag
	(
	const char *name,
	orientation_t * orient
	)

{
	int tagnum;

	tagnum = gi.Tag_NumForName( edict->tiki, name );

	if( tagnum < 0 )
		return false;

	GetRawTag( tagnum, orient );
	return true;
}

void Entity::GetTag
	(
	int    tagnum,
	orientation_t *orient
	)

{
	orientation_t  o;
	int            i;

	GetRawTag( tagnum, &o );

	VectorCopy( origin, orient->origin );

	for( i = 0; i < 3; i++ )
	{
		VectorMA( orient->origin, o.origin[ i ], orientation[ i ], orient->origin );
	}
	MatrixMultiply( o.axis, orientation, orient->axis );
}

qboolean Entity::GetTag
	(
	const char *name,
	orientation_t * orient
	)

{
	int      tagnum;

	tagnum = gi.Tag_NumForName( edict->tiki, name );

	if( tagnum < 0 )
		return false;

	GetTag( tagnum, orient );
	return true;
}

void Entity::GetTag
	(
	int    tagnum,
	Vector *pos,
	Vector *forward,
	Vector *left,
	Vector *up
	)

{
	orientation_t orient;

	GetTag( tagnum, &orient );

	if( pos )
	{
		*pos = Vector( orient.origin );
	}
	if( forward )
	{
		*forward = Vector( orient.axis[ 0 ] );
	}
	if( left )
	{
		*left = Vector( orient.axis[ 1 ] );
	}
	if( up )
	{
		*up = Vector( orient.axis[ 2 ] );
	}
}

qboolean Entity::GetTag
	(
	const char *name,
	Vector *pos,
	Vector *forward,
	Vector *left,
	Vector *up
	)

{
	int tagnum;

	tagnum = gi.Tag_NumForName( edict->tiki, name );

	if( tagnum < 0 )
		return false;

	GetTag( tagnum, pos, forward, left, up );
	return true;
}

qboolean Entity::GetTagPositionAndOrientation
	(
	str tagname,
	orientation_t *new_or
	)

{
	int tagnum;

	tagnum = gi.Tag_NumForName( edict->tiki, tagname );

	if( tagnum < 0 )
		return false;

	GetTagPositionAndOrientation( tagnum, new_or );
	return true;
}

void Entity::GetTagPositionAndOrientation
	(
	int tagnum,
	orientation_t *new_or
	)

{
	int i;
	orientation_t tag_or;
	float axis[ 3 ][ 3 ];

	GetRawTag( tagnum, &tag_or );

	AnglesToAxis( angles, axis );

	VectorCopy( origin, new_or->origin );

	for( i = 0; i < 3; i++ )
	{
		VectorMA( new_or->origin, tag_or.origin[ i ], tag_or.axis[ i ], new_or->origin );
	}

	MatrixMultiply( tag_or.axis, axis, new_or->axis );
}

void Entity::GetTagAngles
	(
	Event *ev
	)

{
	orientation_t orient;
	Vector ang;
	str tagname;
	int tagnum;

	if( !edict->tiki )
	{
		ScriptError( "Entity '%s' at %.2f %.2f %.2f has no model.", targetname.c_str(), origin[ 0 ], origin[ 1 ], origin[ 2 ] );
	}

	tagname = ev->GetString( 1 );
	tagnum = gi.Tag_NumForName( edict->tiki, tagname );
	if( tagnum < 0 )
	{
		ScriptError( "Could not find tag '%s' in '%s'", tagname.c_str(), edict->tiki->name );
	}

	GetTagPositionAndOrientation( tagnum, &orient );
	MatrixToEulerAngles( orient.axis, ang );

	ev->AddVector( ang );
}

void Entity::GetTagPosition
	(
	Event *ev
	)

{
	Vector pos;
	str tagname;
	int tagnum;
	int i;
	orientation_t tag_or;
	float axis[ 3 ][ 3 ];

	if( !edict->tiki )
	{
		ScriptError( "Entity '%s' at %.2f %.2f %.2f has no model.", targetname.c_str(), origin[ 0 ], origin[ 1 ], origin[ 2 ] );
	}

	tagname = ev->GetString( 1 );
	tagnum = gi.Tag_NumForName( edict->tiki, tagname );
	if( tagnum < 0 )
	{
		ScriptError( "Could not find tag '%s' in '%s'", tagname.c_str(), edict->tiki->name );
	}

	tag_or = G_TIKI_Orientation( edict, tagnum & TAG_MASK );
	AnglesToAxis( angles, axis );

	pos = origin;
	for( i = 0; i < 3; i++ )
	{
		VectorMA( pos, tag_or.origin[ i ], axis[ i ], pos );
	}

	ev->AddVector( pos );
}

void Entity::addAngles
	(
	Vector add
	)

{
	if( bindmaster )
	{
		setAngles( localangles + add );
	}
	else
	{
		setAngles( angles + add );
	}
}

void Entity::setAngles
	(
	void
	)

{
	if( bindmaster )
	{
		setAngles( localangles );
	}
	else
	{
		setAngles( angles );
	}
}

void Entity::setAngles
	(
	Vector ang
	)

{
	Entity * ent;
	int num, i;

	if( m_bGlueAngles && m_pGlueMaster )
	{
		ang = m_pGlueMaster->angles;
		avelocity = m_pGlueMaster->avelocity;
	}

	angles[ 0 ] = AngleMod( ang[ 0 ] );
	angles[ 1 ] = AngleMod( ang[ 1 ] );
	angles[ 2 ] = AngleMod( ang[ 2 ] );

	localangles = angles;
	if( bindmaster )
	{
		float	mat[ 3 ][ 3 ];
		AnglesToAxis( localangles, mat );
		R_ConcatRotations( mat, bindmaster->orientation, orientation );
		MatrixToEulerAngles( orientation, angles );
	}
	else
	{
		AnglesToAxis( angles, orientation );
	}

	angles.copyTo( edict->s.netangles );
	angles.copyTo( edict->s.angles );

	// Fill the edicts matrix
	VectorCopy( orientation[ 0 ], edict->mat[ 0 ] );
	VectorCopy( orientation[ 1 ], edict->mat[ 1 ] );
	VectorCopy( orientation[ 2 ], edict->mat[ 2 ] );

	if( edict->s.eFlags & EF_LINKANGLES )
	{
		angles.copyTo( edict->r.currentAngles );
		link();
	}

	//
	// go through and set our children
	//
	num = numchildren;
	for( i = 0; ( i < MAX_MODEL_CHILDREN ) && num; i++ )
	{
		if( children[ i ] == ENTITYNUM_NONE )
			continue;
		ent = ( Entity * )G_GetEntity( children[ i ] );
		ent->setAngles();
		num--;
	}

	for( i = 0; i < m_iNumGlues; i++ )
	{
		m_pGlues[ i ]->setAngles();
	}
}

qboolean Entity::droptofloor
	(
	float maxfall
	)

	{
	trace_t	trace;
	Vector	end;
   Vector   start;

   start = origin + Vector( "0 0 1" );
   end = origin;
	end[ 2 ]-= maxfall;

	trace = G_Trace( start, mins, maxs, end, this, edict->clipmask, false, "Entity::droptofloor" );
	if ( trace.fraction == 1 || trace.startsolid || trace.allsolid || !trace.ent )
		{
		groundentity = world->edict;
		return false;
		}

	setOrigin( trace.endpos );

	groundentity = trace.ent;

	return true;
	}

void Entity::DamageType
   (
   Event *ev
   )

   {
   str damage;
   damage = ev->GetString( 1 );
   if ( damage == "all" )
      {
      damage_type = -1;
      }
   else
      {
      damage_type = MOD_NameToNum( damage );
      }
   }

void Entity::Damage
	(
	Entity *inflictor,
	Entity *attacker,
	float damage,
	Vector position,
	Vector direction,
	Vector normal,
	int knockback,
	int dflags,
	int meansofdeath,
	int location
	)

{
	Event	*ev;

	// if our damage types do not match, return
	if( !MOD_matches( meansofdeath, damage_type ) )
	{
		return;
	}

	if( !attacker )
	{
		attacker = world;
	}
	if( !inflictor )
	{
		inflictor = world;
	}

	ev = new Event( EV_Damage );
	ev->AddEntity( attacker );
	ev->AddFloat( damage );
	ev->AddEntity( inflictor );
	ev->AddVector( position );
	ev->AddVector( direction );
	ev->AddVector( normal );
	ev->AddInteger( knockback );
	ev->AddInteger( dflags );
	ev->AddInteger( meansofdeath );
	ev->AddInteger( location );
	ProcessEvent( ev );
}

void Entity::DamageEvent
	(
	Event *ev
	)

{
	Entity	*inflictor;
	Entity	*attacker;
	int		damage;
	Vector	dir;
	Vector	momentum;
	Vector	position, direction, normal;
	int		knockback, damageflags, meansofdeath, location;
	Event		*event;
	float		m;

	if ( ( takedamage == DAMAGE_NO ) || ( movetype == MOVETYPE_NOCLIP ) )
	{
		return;
	}

	attacker		= ev->GetEntity( 1 );
	damage			= ev->GetInteger( 2 );
	inflictor		= ev->GetEntity( 3 );

	position		= ev->GetVector( 4 );
	direction		= ev->GetVector( 5 );
	normal			= ev->GetVector( 6 );

	knockback		= ev->GetInteger( 7 );
	damageflags		= ev->GetInteger( 8 );
	meansofdeath	= ev->GetInteger( 9 );
	location		= ev->GetInteger( 10 );

	// figure momentum add
	if ( ( inflictor != world ) &&
		( movetype != MOVETYPE_NONE ) &&
		( movetype != MOVETYPE_STATIONARY ) &&
		( movetype != MOVETYPE_BOUNCE ) &&
		( movetype != MOVETYPE_PUSH ) &&
		( movetype != MOVETYPE_STOP ) )
	{
		dir = origin - ( inflictor->origin + ( inflictor->mins + inflictor->maxs ) * 0.5 );
		dir.normalize();

		if ( mass < 50)
		{
			m = 50;
		}
		else
		{
			m = mass;
		}

		momentum = dir * damage * ( 1700.0 / m );
		velocity += momentum;
	}

	// check for godmode or invincibility
	if( flags & FL_GODMODE )
	{
		return;
	}

	// team play damage avoidance
	//if ( ( global->teamplay == 1 ) && ( edict->team > 0 ) && ( edict->team == attacker->edict->team ) )
	//	{
	//	return;
	//	}

	if ( !g_gametype->integer && IsSubclassOfPlayer() )
	{
		damage *= 0.15;
	}

	if ( deadflag )
	{
		// Check for gib.
		if ( inflictor->IsSubclassOfProjectile() )
		{
			Event *gibEv;

			health -= damage;

			gibEv = new Event( EV_Gib );
			gibEv->AddEntity( this );
			gibEv->AddFloat( health );

			ProcessEvent( gibEv );
		}

		return;
	}

	// do the damage
	health -= damage;
	if ( health <= 0 )
	{
		if ( attacker )
		{
			event = new Event( EV_GotKill );
			event->AddEntity( this );
			event->AddInteger( damage );
			event->AddEntity( inflictor );
			event->AddInteger( meansofdeath );
			event->AddInteger( 0 );

			attacker->ProcessEvent( event );
		}

		event = new Event( EV_Killed );
		event->AddEntity( attacker );
		event->AddFloat( damage );
		event->AddEntity( inflictor );
		event->AddVector( position );
		event->AddVector( direction );
		event->AddVector( normal );
		event->AddInteger( knockback );
		event->AddInteger( damageflags );
		event->AddInteger( meansofdeath );
		event->AddInteger( location );

		ProcessEvent( event );
		return;
	}

	event = new Event( EV_Pain );
	event->AddEntity( attacker );
	event->AddFloat( damage );
	event->AddEntity( inflictor );
	event->AddVector( position );
	event->AddVector( direction );
	event->AddVector( normal );
	event->AddInteger( knockback );
	event->AddInteger( damageflags );
	event->AddInteger( meansofdeath );
	event->AddInteger( location );

	ProcessEvent( event );
}

void Entity::Delete
	(
	void
	)

{
	// Delete the entity the next frame

	if( g_iInThinks )
		PostEvent( EV_Remove, 0 );
	else
		delete this;
}

void Entity::Remove
	(
	Event *ev
	)

{
	if( ev->NumArgs() )
		ScriptError( "Arguments not allowed." );

	Delete();
}

void Entity::EventSoundDone
	(
	Event *ev
	)

{
	int channelNum = ev->GetInteger( 1 );
	str sfxName = ev->GetString( 2 );

	if( gi.S_IsSoundPlaying( channelNum, sfxName ) )
	{
		// Repost the event
		Event *event = new Event( EV_SoundDone );
		event->AddInteger( channelNum );
		event->AddString( sfxName );
		PostEvent( event, level.frametime );
	}
	else
	{
		Unregister( STRING_SOUNDDONE );
	}
}

void Entity::VolumeDamage
	(
	float damage
	)

{
	int i;
	int numtouch;
	Entity *ent;
	int touch[ MAX_GENTITIES ];

	numtouch = gi.AreaEntities( absmin, absmax, touch, MAX_GENTITIES );

	for( i = 0; i < numtouch; i++ )
	{
		ent = G_GetEntity( touch[ i ] );
		ent->Damage(
			world,
			world,
			damage,
			origin,
			vec_zero,
			vec_zero,
			0,
			DAMAGE_RADIUS,
			MOD_EXPLOSION );
	}
}

void Entity::EventVolumeDamage
	(
	Event *ev
	)

{
	VolumeDamage( ev->GetFloat( 1 ) );
}

void Entity::AddImmunity
	(
	Event *ev
	)

{
	str immune_string;
	int new_immunity;
	int number_of_immunities;
	int i;


	number_of_immunities = ev->NumArgs();

	for( i = 1; i <= number_of_immunities; i++ )
	{
		immune_string = ev->GetString( i );

		new_immunity = MOD_NameToNum( immune_string );

		if( new_immunity != -1 )
			immunities.AddUniqueObject( new_immunity );
	}
}

void Entity::RemoveImmunity
	(
	Event *ev
	)

{
	str immune_string;
	int old_immunity;
	int number_of_immunities;
	int i;


	number_of_immunities = ev->NumArgs();

	for( i = 1; i <= number_of_immunities; i++ )
	{
		immune_string = ev->GetString( i );

		old_immunity = MOD_NameToNum( immune_string );

		if( old_immunity != -1 && immunities.ObjectInList( old_immunity ) )
			immunities.RemoveObject( old_immunity );
	}
}

qboolean Entity::Immune
	(
	int meansofdeath
	)

{
	int number_of_immunities, i;

	number_of_immunities = immunities.NumObjects();

	for( i = 1; i <= number_of_immunities; i++ )
	{
		if( meansofdeath == immunities.ObjectAt( i ) )
			return true;
	}

	return false;
}

void Entity::Stun
   (
   float time
   )

   {
   Event *ev = new Event( EV_Stun );
   ev->AddFloat( time );
   ProcessEvent( ev );
   }

/*
============
CanDamage

Returns true if the inflictor can directly damage the target.  Used for
explosions and melee attacks.
============
*/
qboolean Entity::CanDamage
	(
	Entity *target,
	Entity *skip_ent
	)

{
	trace_t	trace;
	Vector	pos;
	Entity	*skip_entity;

	if( skip_ent )
		skip_entity = skip_ent;
	else
		skip_entity = this;

	trace = G_Trace( origin, vec_origin, vec_origin, target->centroid, skip_entity, MASK_SHOT, false, "Entity::CanDamage 1" );
	if( trace.fraction == 1 || trace.ent == target->edict )
	{
		return true;
	}
	pos = target->centroid + Vector( 15, 15, 0 );
	trace = G_Trace( origin, vec_origin, vec_origin, pos, skip_entity, MASK_SHOT, false, "Entity::CanDamage 3" );
	if( trace.fraction == 1 || trace.ent == target->edict )
	{
		return true;
	}
	pos = target->centroid + Vector( -15, 15, 0 );
	trace = G_Trace( origin, vec_zero, vec_zero, pos, skip_entity, MASK_SHOT, false, "Entity::CanDamage 4" );
	if( trace.fraction == 1 || trace.ent == target->edict )
	{
		return true;
	}
	pos = target->centroid + Vector( 15, -15, 0 );
	trace = G_Trace( origin, vec_zero, vec_zero, pos, skip_entity, MASK_SHOT, false, "Entity::CanDamage 5" );
	if( trace.fraction == 1 || trace.ent == target->edict )
	{
		return true;
	}
	pos = target->centroid + Vector( -15, -15, 0 );
	trace = G_Trace( origin, vec_zero, vec_zero, pos, skip_entity, MASK_SHOT, false, "Entity::CanDamage 6" );
	if( trace.fraction == 1 || trace.ent == target->edict )
	{
		return true;
	}

	return false;
}

void Entity::EventInPVS
	(
	Event *ev
	)

{
	ev->AddInteger( gi.AreasConnected( edict->r.areanum, ev->GetEntity( 1 )->edict->r.areanum ) != 0 );
}

qboolean Entity::IsDead
	(
	void
	)

{
	return deadflag != DEAD_NO;
}

qboolean Entity::IsTouching
	(
	Entity *e1
	)

	{
	if ( e1->absmin.x > absmax.x )
		{
		return false;
		}
	if ( e1->absmin.y > absmax.y )
		{
		return false;
		}
	if ( e1->absmin.z > absmax.z )
		{
		return false;
		}
	if ( e1->absmax.x < absmin.x )
		{
		return false;
		}
	if ( e1->absmax.y < absmin.y )
		{
		return false;
		}
	if ( e1->absmax.z < absmin.z )
		{
		return false;
		}

	return true;
	}

void Entity::IsTouching
	(
	Event *ev
	)

{
	Entity *ent = ev->GetEntity( 1 );

	if( !ent )
	{
		ScriptError( "IsTouching used with a NULL entity." );
	}

	ev->AddInteger( IsTouching( ent ) );
}

qboolean Entity::FovCheck
	(
	float *delta,
	float fovdot
	)

{
	float deltadot = delta[ 0 ] * orientation[ 0 ][ 0 ] + delta[ 1 ] * orientation[ 0 ][ 1 ];

	if( fovdot < 0.0f )
	{
		if( deltadot >= 0.0f )
			return true;

		return ( delta[ 0 ] * delta[ 0 ] + delta[ 1 ] * delta[ 1 ] ) * fovdot * fovdot > ( deltadot * deltadot );
	}
	else
	{
		if( deltadot < 0.0f )
			return false;

		return ( deltadot * deltadot ) > ( delta[ 0 ] * delta[ 0 ] + delta[ 1 ] * delta[ 1 ] ) * fovdot * fovdot;
	}
}

bool Entity::CanSee
	(
	Entity *ent,
	float fov,
	float vision_distance
	)

{
	float delta[ 2 ];

	delta[ 0 ] = ent->centroid[ 0 ] - centroid[ 0 ];
	delta[ 1 ] = ent->centroid[ 1 ] - centroid[ 1 ];

	if( ( vision_distance <= 0.0f ) || ( delta[ 0 ] * delta[ 0 ] + delta[ 1 ] * delta[ 1 ] ) <= ( vision_distance * vision_distance ) )
	{
		if( gi.AreasConnected( edict->r.areanum, ent->edict->r.areanum ) &&
			( ( fov <= 0.0f || fov >= 360.0f ) ||
			( FovCheck( delta, cos( fov * ( 0.5 * M_PI / 180.0 ) ) ) ) ) )
		{
			return G_SightTrace( centroid,
				vec_zero,
				vec_zero,
				ent->centroid,
				this,
				ent,
				MASK_CANSEE,
				0,
				"Entity::CanSee" );
		}
	}

	return false;
}

void Entity::CanSee
	(
	Event *ev
	)

{
	Entity *ent;
	float fov = 0.0f;
	float distance = 0.0f;

	if( ev->NumArgs() <= 0 || ev->NumArgs() > 3 )
	{
		ScriptError( "cansee should have 1, 2 or 3 arguments" );
	}

	if( ev->NumArgs() > 2 )
	{
		distance = ev->GetFloat( 3 );
		if( distance < 0 )
		{
			ScriptError( "distance must be >= 0" );
		}
	}

	if( ev->NumArgs() > 1 )
	{
		fov = ev->GetFloat( 2 );
		if( fov < 0.0f || fov > 360.0f )
		{
			ScriptError( "fov must be in range 0 <= fov <= 360" );
		}
	}

	ent = ev->GetEntity( 1 );
	if( !ent )
	{
		ScriptError( "NULL entity in parameter 1" );
	}

	ev->AddInteger( CanSee( ent, fov, distance ) );
}

void Entity::GetVelocity
	(
	Event *ev
	)

{
	ev->AddVector( velocity );
}

void Entity::GetAVelocity
	(
	Event *ev
	)

{
	ev->AddVector( avelocity );
}

void Entity::SetVelocity
	(
	Event *ev
	)

{
	velocity = ev->GetVector( 1 );
}

void Entity::FadeNoRemove
	(
	Event *ev
	)

	{
   float rate;
   float target;
   float myalpha;

   if ( ev->NumArgs() > 1 )
      {
      target = ev->GetFloat( 2 );
      }
   else
      {
      target = 0;
      }

   if ( ev->NumArgs() > 0 )
      {
      rate = ev->GetFloat( 1 );
      assert( rate );
      if ( rate > 0 )
         rate = FRAMETIME / rate;
      }
   else
      {
      rate = 0.03f;
      }

   myalpha = edict->s.alpha;
   myalpha -= rate;
   
   if ( myalpha < target )
      myalpha = target;

   setAlpha( myalpha );

   if ( myalpha > target )
      {
      PostEvent( *ev, FRAMETIME );
      }

   G_SetConstantLight( &edict->s.constantLight, &myalpha, &myalpha, &myalpha, 0 );
	}

void Entity::FadeOut
	(
	Event *ev
	)

	{
   float myscale;
   float myalpha;

   myscale = edict->s.scale;
   myscale -= 0.03f;
   myalpha = edict->s.alpha;
   myalpha -= 0.03f;
   if ( myscale < 0 )
      myscale = 0;
   if ( myalpha < 0 )
      myalpha = 0;

	if ( myscale <= 0 && myalpha <= 0 )
		{
	   PostEvent( EV_Remove, 0 );
		}
   else
      {
	   PostEvent( *ev, FRAMETIME );
      }

   setScale( myscale );
   setAlpha( myalpha );
	}

void Entity::FadeIn
	(
	Event *ev
	)

	{
   float rate;
   float target;
   float myalpha;

   if ( ev->NumArgs() > 1 )
      {
      target = ev->GetFloat( 2 );
      }
   else
      {
      target = 1;
      }

   if ( ev->NumArgs() > 0 )
      {
      rate = ev->GetFloat( 1 );
      assert( rate );
      if ( rate > 0 )
         rate = FRAMETIME / rate;
      }
   else
      {
      rate = 0.03f;
      }


   myalpha = edict->s.alpha;
   myalpha += rate;

   if ( myalpha > target )
      myalpha = target;
   
   if ( myalpha < target )
		{
   	PostEvent( *ev, FRAMETIME );
		}
   setAlpha( myalpha );
	}

void Entity::Fade
	(
	Event *ev
	)

	{
   float rate;
   float target;
   float myalpha;

   if ( ev->NumArgs() > 1 )
      {
      target = ev->GetFloat( 2 );
      }
   else
      {
      target = 0;
      }

   if ( ev->NumArgs() > 0 )
      {
      rate = ev->GetFloat( 1 );
      assert( rate );
      if ( rate > 0 )
         rate = FRAMETIME / rate;
      }
   else
      {
      rate = 0.03f;
      }

   myalpha = edict->s.alpha;
   myalpha -= rate;
   
   if ( myalpha <= 0 )
		{
	   PostEvent( EV_Remove, 0 );
      return;
		}

   if ( myalpha < target )
      myalpha = target;

   if ( myalpha > target )
      {
      PostEvent( *ev, FRAMETIME );
      }

   setAlpha( myalpha );
   G_SetConstantLight( &edict->s.constantLight, &myalpha, &myalpha, &myalpha, 0 );
	}

void Entity::SetMassEvent
	(
	Event *ev
	)

	{
   mass = ev->GetFloat( 1 );
	}

void Entity::CheckGround
	(
	void
	)

{
	Vector	point;
	trace_t	trace;

	if( flags & ( FL_SWIM | FL_FLY ) )
	{
		return;
	}

	if( velocity.z > 100 )
	{
		groundentity = NULL;
		return;
	}

	// if the hull point one-quarter unit down is solid the entity is on ground
	point = origin;
	point.z -= 0.25;
	trace = G_Trace( origin, mins, maxs, point, this, edict->clipmask, false, "Entity::CheckGround" );

	// check steepness
	if( ( trace.plane.normal[ 2 ] <= 0.7 ) && !trace.startsolid )
	{
		groundentity = NULL;
		return;
	}

	groundentity = trace.ent;
	groundplane = trace.plane;
	groundcontents = trace.contents;

	if( !trace.startsolid && !trace.allsolid )
	{
		setOrigin( trace.endpos );
		velocity.z = 0;
	}
}

void Entity::BecomeSolid
	(
	Event *ev
	)

	{
	if ( ( model.length() ) && ( ( model[ 0 ] == '*' ) || ( strstr( model.c_str(), ".bsp" ) ) ) )
		{
		setSolidType( SOLID_BSP );
		}
	else
		{
		setSolidType( SOLID_BBOX );
		}
	}

void Entity::BecomeNonSolid
	(
	Event *ev
	)

{
	setSolidType( SOLID_NOT );
}

void Entity::Ghost
	(
	Event *ev
	)

	{
   // Make not solid, but send still send over whether it is hidden or not
	setSolidType( SOLID_NOT );
   edict->r.svFlags &= ~SVF_NOCLIENT;
	}

void Entity::Sound
	(
	str		sound_name,
	int		channel,
	float	volume,
	float	min_dist,
	Vector	*sound_origin,
	float	pitch,
	int		argstype,
	int		doCallback,
	int		checkSubtitle,
	float	max_dist
	)

{
	const char *name = NULL;
	vec3_t org;
	str random_alias;
	AliasListNode_t *ret = NULL;
	static cvar_t *g_subtitle = NULL;
	static bool _tmp = false;
	int num;

	if( !_tmp )
	{
		g_subtitle = gi.Cvar_Get( "g_subtitle", "0", CVAR_ARCHIVE );
		_tmp = true;
	}

	if( doCallback && g_gametype->integer )
	{
		Com_Printf( "^~^~^ Callback of sound '%s' ignored.\n", sound_name.c_str() );
		doCallback = 0;
	}

	if( sound_name.length() > 0 )
	{
		// Get the real sound to play

		random_alias = GetRandomAlias( sound_name, &ret ).c_str();

		if( random_alias.length() > 0 )
			name = random_alias.c_str();

		if( !name )
			name = sound_name.c_str();

		// Play the sound

		if( name != NULL )
		{
			if( ret )
			{
				if( channel < 0 )
				{
					channel = ret->channel;
				}

				num = entnum;
				if( sound_origin )
				{
					VectorCopy( *sound_origin, org );
					num = ENTITYNUM_NONE;
				}
				else
				{
					VectorCopy( edict->s.origin, org );

					if( doCallback )
					{
						num |= 0x400;
					}
				}


				if( argstype == 0 )
				{
					volume = G_Random() * ret->volumeMod + ret->volume;
					pitch = G_Random() * ret->pitchMod + ret->pitch;
					min_dist = ret->dist;
					max_dist = ret->maxDist;
				}
				else if( argstype == 1 )
				{
					if( volume >= 0.0f )
						volume = G_Random() * ret->volumeMod + volume;
					else
						pitch = G_Random() * ret->pitchMod + ret->pitch;

					if( pitch >= 0.0f )
						pitch = G_Random() * ret->pitchMod + pitch;
					else
						pitch = G_Random() * ret->pitchMod + ret->pitch;
				}
				else
				{
					if( volume <= 0.0f )
						volume = G_Random() * ret->volumeMod + ret->volume;

					if( pitch >= 0.0f )
						pitch = G_Random() * ret->pitchMod + pitch;
					else
						pitch = G_Random() * ret->pitchMod + ret->pitch;

					if( min_dist < 0.0f )
						min_dist = ret->dist;

					if( max_dist < 0.0f )
						max_dist = ret->maxDist;
				}

				if( ( !checkSubtitle || g_subtitle->integer ) && ret->subtitle )
				{
					Entity *p = G_GetEntity( 0 );

					if( g_subtitle->integer == 2 || max_dist * max_dist > DistanceSquared( org, p->edict->s.origin ) )
					{
						cvar_t *curSubtitle = gi.Cvar_Get( "curSubtitle", "0", 0 );

						gi.Cvar_Set( va( "subtitle%d", curSubtitle->integer ), va( "%s", ret->subtitle ) );
						gi.Cvar_Set( "curSubtitle", va( "%d", curSubtitle->integer + 1 ) );
					}
				}

				gi.Sound( &org, num, channel, name, volume, min_dist, pitch, max_dist, ret->streamed );
			}
		}
		else
		{
			gi.DPrintf( "ERROR: Entity::Sound: %s needs an alias in ubersound.scr or uberdialog.scr - Please fix.\n", sound_name.c_str() );
		}
	}
	else
	{
		warning( "Sound", "Null sample pointer" );
	}
}

void Entity::LoopSound
	(
	Event *ev
	)

{
	str sound_name;
	float volume = DEFAULT_VOL;
	float min_dist = DEFAULT_MIN_DIST;
	str min_dist_string;


	if( ev->NumArgs() < 1 )
		return;

	// Get parameters

	sound_name = ev->GetString( 1 );

	if( ev->NumArgs() > 1 )
		volume = ev->GetFloat( 2 );

	if( ev->NumArgs() > 2 )
	{
		min_dist_string = ev->GetString( 3 );

		if( min_dist_string == LEVEL_WIDE_STRING )
			min_dist = LEVEL_WIDE_MIN_DIST;
		else
			min_dist = ev->GetFloat( 3 );
	}

	// Add this sound to loop

	LoopSound( sound_name.c_str(), volume, min_dist );
}

void Entity::LoopSound( str sound_name, float volume, float min_dist, float max_dist, float pitch )
{
	str name = NULL;
	str random_alias;
	AliasListNode_t *ret = NULL;

	// Get the real sound to be played

	if( sound_name.length() > 0 )
	{
		// Get the real sound to play

		name = GetRandomAlias( sound_name, &ret ).c_str();

		if( !name )
			name = sound_name.c_str();

		if( name.length() && ret )
		{
			int aliaschannel = ret->channel;
			float aliasvolume = G_Random() * ret->volumeMod + ret->volume;
			float aliasmin_dist = ret->dist;
			float aliasmax_dist = ret->maxDist;
			float aliaspitch = G_Random() * ret->pitchMod + ret->pitch;
			int aliasstreamed = ret->streamed;
			// Add the looping sound to the entity

			edict->s.loopSound = gi.soundindex( name, aliasstreamed );
			edict->s.loopSoundVolume = aliasvolume;
			edict->s.loopSoundMinDist = aliasmin_dist;
			edict->s.loopSoundMaxDist = aliasmax_dist;
			edict->s.loopSoundPitch = aliaspitch;

			// Local sound will always be heard
			edict->s.loopSoundFlags = aliaschannel == CHAN_LOCAL;
		}
		else
		{
			gi.DPrintf( "ERROR: Entity::LoopSound: %s needs an alias in ubersound.scr or uberdialog.scr - Please fix.\n", sound_name.c_str() );
		}
	}
}

void Entity::StopLoopSound( Event *ev )
{
	StopLoopSound();
}

void Entity::StopLoopSound( void )
{
	edict->s.loopSound = 0;
}

void Entity::ProcessSoundEvent( Event *ev, qboolean checkSubtitle )
{
	str sound_name;
	str wait;
	int waitTillDone = 0;
	float volume;
	int channel = 0;

	// Set defaults
	volume = DEFAULT_VOL;

	if( ev->NumArgs() > 3 )
	{
		Com_Printf( "ERROR Sound.  Format is playsound soundname <wait>\n" );
		return;
	}

	// Get sound parameters

	sound_name = ev->GetString( 1 );

	if( ev->NumArgs() > 1 )
		wait = ev->GetString( 2 );

	if( Q_stricmp( wait.c_str(), "wait" ) )
	{
		if( !Q_stricmp( wait.c_str(), "volume" ) )
		{
			if( ev->NumArgs() != 3 )
				ScriptError( "Entity::Sound: volume not followed by a float" );

			volume = ev->GetFloat( 3 );
		}
	}
	else
	{
		waitTillDone = 1;

		if( ev->NumArgs() == 3 )
			volume = ev->GetFloat( 3 );
	}

	if( volume == DEFAULT_VOL )
	{
		if( wait.length() )
			channel = atoi( wait.c_str() );

		Sound( sound_name, channel, volume, 0, NULL, 0, 0, waitTillDone, checkSubtitle, -1.0f );
	}
	else
	{
		Sound( sound_name, -1.0f, volume, -1.0f, NULL, -1.0f, 1, waitTillDone, checkSubtitle, -1.0f );
	}
}

void Entity::Sound
	(
	Event *ev
	)

{
	if( level.spawning )
	{
		Event *event = new Event( EV_Sound );

		for( int i = 1; i <= ev->NumArgs(); i++ )
		{
			event->AddValue( ev->GetValue( 1 ) );
		}

		PostEvent( event, level.frametime );
	}
	else
	{
		ProcessSoundEvent( ev, true );
	}
}

void Entity::StopSound
	(
	Event *ev
	)

	{
   if (ev->NumArgs() < 1)
      StopSound( CHAN_BODY );
   else
      StopSound( ev->GetInteger( 1 ) );
	}

void Entity::StopSound
	(
	int channel
	)

	{
		gi.StopSound( entnum, channel );
	}

void Entity::SetLight
	(
	Event *ev
	)

	{
   float r, g, b;

   if ( ev->NumArgs() == 1 )
      {
      Vector tmp;

      tmp = ev->GetVector( 1 );
      r = tmp.x;
      g = tmp.y;
      b = tmp.z;
      }
   else
      {
      r = ev->GetFloat( 1 );
      g = ev->GetFloat( 2 );
      b = ev->GetFloat( 3 );
      lightRadius  = ev->GetFloat( 4 );
      }

   G_SetConstantLight( &edict->s.constantLight, &r, &g, &b, &lightRadius );
   }

void Entity::LightOn
	(
	Event *ev
	)

	{
   G_SetConstantLight( &edict->s.constantLight, NULL, NULL, NULL, &lightRadius );
   }

void Entity::LightOff
	(
	Event *ev
	)

	{
   float radius = 0;

   G_SetConstantLight( &edict->s.constantLight, NULL, NULL, NULL, &radius );
   }

void Entity::LightRed
	(
	Event *ev
	)

	{
   float r;

   r = ev->GetFloat( 1 );
   G_SetConstantLight( &edict->s.constantLight, &r, NULL, NULL, NULL );
   }

void Entity::LightGreen
	(
	Event *ev
	)

	{
   float g;

   g = ev->GetFloat( 1 );
   G_SetConstantLight( &edict->s.constantLight, NULL, &g, NULL, NULL );
   }

void Entity::LightBlue
	(
	Event *ev
	)

	{
   float b;

   b = ev->GetFloat( 1 );
   G_SetConstantLight( &edict->s.constantLight, NULL, NULL, &b, NULL );
   }

void Entity::LightRadius
	(
	Event *ev
	)

	{
   lightRadius = ev->GetFloat( 1 );
   G_SetConstantLight( &edict->s.constantLight, NULL, NULL, NULL, &lightRadius );
   }

void Entity::LightStyle
	(
	Event *ev
	)

	{
   int style;

   style = ev->GetInteger( 1 );
   G_SetConstantLight( &edict->s.constantLight, NULL, NULL, NULL, NULL, &style );
   }

void Entity::EventSetHealthOnly
	(
	Event *ev
	)

{
	if( IsDead() )
	{
		ScriptError( "cannot give health to dead entities" );
	}

	health = ev->GetFloat( 1 );
	if( health <= 0.0f )
	{
		ScriptError( "health must be greater than 0" );
	}

	if( health > max_health ) {
		health = max_health;
	}
}

void Entity::EventSetMaxHealth
	(
	Event *ev
	)

{
	max_health = ev->GetFloat( 1 );
	if( max_health <= 0.0f )
	{
		ScriptError( "max_health must be greater than 0" );
	}

	if( health > max_health ) {
		health = max_health;
	}
}

void Entity::EventGetMaxHealth
	(
	Event *ev
	)

{
	ev->AddFloat( max_health );
}

void Entity::SetHealth
	(
	Event *ev
	)

{
	if( IsDead() )
	{
		ScriptError( "cannot give health to dead entities" );
	}

	health = ev->GetFloat( 1 );

	if( health <= 0 )
	{
		ScriptError( "health must be greater than 0" );
	}

	max_health = health;
}

void Entity::GetHealth
	(
	Event *ev
	)

{
	ev->AddFloat( health );
}

void Entity::SetSize
	(
	Event *ev
	)

{
	Vector min, max;

	min = ev->GetVector( 1 );
	max = ev->GetVector( 2 );
	setSize( min, max );
}

void Entity::SetMins
	(
	Event *ev
	)

{
	Vector min;

	min = ev->GetVector( 1 );
	setSize( min, maxs );
}

void Entity::SetMaxs
	(
	Event *ev
	)

{
	Vector max;

	max = ev->GetVector( 1 );
	setSize( mins, max );
}

void Entity::SetScale
	(
	Event *ev
	)

{
	setScale( ev->GetFloat( 1 ) );
}

void Entity::GetScale
	(
	Event *ev
	)

{
	ev->AddFloat( edict->s.scale );
}

void Entity::SetAlpha
	(
	Event *ev
	)

	{
	setAlpha( ev->GetFloat( 1 ) );
   }

void Entity::SetOrigin
	(
	Event *ev
	)

	{
	setOrigin( ev->GetVector( 1 ) );
	}

void Entity::SetKillTarget
	(
	Event *ev
	)

	{
	SetKillTarget( ev->GetString( 1 ) );
	}

void Entity::SetAngles
	(
	Event *ev
	)

{
	setAngles( ev->GetVector( 1 ) );
}

Vector Entity::GetControllerAngles
	(
	int num
	)

{
	Vector controller_angles;

	assert( ( num >= 0 ) && ( num < NUM_BONE_CONTROLLERS ) );

	if( ( num < 0 ) || ( num >= NUM_BONE_CONTROLLERS ) )
	{
		gi.Printf( "GetControllerAngles", "Bone controller index out of range (%d)\n", num );
		return vec_zero;
	}

	controller_angles = edict->s.bone_angles[ num ];

	return controller_angles;
}

void Entity::GetControllerAngles
	(
	Event *ev
	)

{
	int num = ev->GetInteger( 1 );

	// this check is missing in mohaa
	if( ( num < 0 ) || ( num >= NUM_BONE_CONTROLLERS ) )
	{
		ScriptError( "Bone controller index out of range. Index must be between 0-" STRING( NUM_BONE_CONTROLLERS - 1 ) "." );
	}

	ev->AddVector( edict->s.bone_angles[ num ] );
}

void Entity::SetControllerAngles
	(
	int num,
	vec3_t angles
	)

{
	assert( ( num >= 0 ) && ( num < NUM_BONE_CONTROLLERS ) );
	if( ( num < 0 ) || ( num >= NUM_BONE_CONTROLLERS ) )
	{
		gi.Printf( "SetControllerAngles", "Bone controller index out of range (%d)\n", num );
		return;
	}

	VectorCopy( angles, edict->s.bone_angles[ num ] );
	EulerToQuat( edict->s.bone_angles[ num ], edict->s.bone_quat[ num ] );
}

void Entity::SetControllerAngles
	(
	Event *ev
	)

{
	int num;
	Vector angles;

	num = ev->GetInteger( 1 );
	angles = ev->GetVector( 2 );

	// this check is missing in mohaa
	if( ( num < 0 ) || ( num >= NUM_BONE_CONTROLLERS ) )
	{
		ScriptError( "Bone controller index out of range. Index must be between 0-" STRING( NUM_BONE_CONTROLLERS - 1 ) "." );
	}

	VectorCopy( angles, edict->s.bone_angles[ num ] );
	EulerToQuat( edict->s.bone_angles[ num ], edict->s.bone_quat[ num ] );
}

void Entity::SetControllerTag
	(
	int num,
	int tag_num
	)

{
	assert( ( num >= 0 ) && ( num < NUM_BONE_CONTROLLERS ) );

	if( ( num < 0 ) || ( num >= NUM_BONE_CONTROLLERS ) )
	{
		gi.Printf( "SetControllerTag", "Bone controller index out of range (%d)\n", num );
		return;
	}

	edict->s.bone_tag[ num ] = tag_num;
}

qboolean Entity::attach
	(
	int parent_entity_num,
	int tag_num,
	qboolean use_angles,
	Vector offset
	)

{
	int i;
	Entity * parent;

	if( entnum == parent_entity_num )
	{
		warning( "attach", "Trying to attach to oneself." );
		return false;
	}

	if( edict->s.parent != ENTITYNUM_NONE )
		detach();

	// 
	// make sure this is a modelanim entity so that the attach works properly
	//
	if( edict->s.eType == ET_GENERAL )
	{
		edict->s.eType = ET_MODELANIM;
	}

	//
	// get the parent
	//
	parent = ( Entity * )G_GetEntity( parent_entity_num );

	if( parent->numchildren < MAX_MODEL_CHILDREN )
	{
		//
		// find a free spot in the parent
		//
		for( i = 0; i < MAX_MODEL_CHILDREN; i++ )
		{
			if( parent->children[ i ] == ENTITYNUM_NONE )
			{
				break;
			}
		}

		edict->s.parent = parent_entity_num;
		setSolidType( SOLID_NOT );
		parent->children[ i ] = entnum;
		parent->numchildren++;
		edict->s.tag_num = tag_num;
		edict->s.attach_use_angles = use_angles;
		offset.copyTo( edict->s.attach_offset );
		setOrigin();

		return true;
	}
	return false;
}

void Entity::KillAttach
	(
	Event *ev
	)

	{
   int i;
	Entity *child = NULL;


	// Kill all of this entities children

   for ( i = 0 ; i < MAX_MODEL_CHILDREN; i++ )
      {
      if ( children[i] != ENTITYNUM_NONE )
         {
			// Remove child
			child = ( Entity * )G_GetEntity( children[i] );

			if ( child )
				child->ProcessEvent( EV_Remove );

			// Remove child from this entity
			children[i] = ENTITYNUM_NONE;
         }
      }

	numchildren = 0;
	}

void Entity::detach
	(
	void
	)

{
	int i;
	int num;
	Entity * parent;

	if( edict->s.parent == ENTITYNUM_NONE )
		return;

	parent = ( Entity * )G_GetEntity( edict->s.parent );
	if( !parent )
		return;
	for( i = 0, num = parent->numchildren; i < MAX_MODEL_CHILDREN; i++ )
	{
		if( parent->children[ i ] == ENTITYNUM_NONE )
		{
			continue;
		}
		if( parent->children[ i ] == entnum )
		{
			parent->children[ i ] = ENTITYNUM_NONE;
			parent->numchildren--;
			break;
		}
		num--;
		if( !num )
			break;
	}
	edict->s.parent = ENTITYNUM_NONE;
	setOrigin( origin );
}

void Entity::Flags( Event *ev )
   {
   const char *flag;
   int mask;
   int action;
   int i;

   for ( i = 1; i <= ev->NumArgs(); i++ )
      {
      action = FLAG_IGNORE;
      flag = ev->GetString( i );
      switch( flag[0] )
         {
         case '+':
            action = FLAG_ADD;
            flag++;
            break;
         case '-':
            action = FLAG_CLEAR;
            flag++;
            break;
         default:
            gi.Printf( "Entity::Flags", "First character is not '+' or '-', assuming '+'\n" );
            action = FLAG_ADD;
            break;
         }

      //
      // WARNING: please change the Event decleration,
      // to match this function, if flags are added or
      // deleted the event must be updated.
      // 
      if ( !Q_stricmp( flag, "blood" ) )
         mask = FL_BLOOD;
      else if ( !Q_stricmp( flag, "explode" ) )
         mask = FL_DIE_EXPLODE;
      else if ( !Q_stricmp( flag, "die_gibs" ) )
         mask = FL_DIE_GIBS;
      else if ( !Q_stricmp( flag, "autoaim" ) )
         mask = FL_AUTOAIM;
      else if ( !Q_stricmp( flag, "god" ) )
         mask = FL_GODMODE;
      else
         {
         mask = 0;
         action = FLAG_IGNORE;
         gi.Printf( "Unknown flag '%s'", flag );
         }
      switch (action)
         {
         case FLAG_ADD:
			 flags |= mask;
            break;
         case FLAG_CLEAR:
			 flags &= ~mask;
            break;
         case FLAG_IGNORE:
            break;
         }
      }
   }


void Entity::Effects( Event *ev )
   {
   const char *flag;
   int mask=0;
   int action;
   int i;

   for ( i = 1; i <= ev->NumArgs(); i++ )
      {
      action = 0;
      flag = ev->GetString( i );
      switch( flag[0] )
         {
         case '+':
            action = FLAG_ADD;
            flag++;
            break;
         case '-':
            action = FLAG_CLEAR;
            flag++;
            break;
         default:
            gi.Printf( "Entity::Effects", "First character is not '+' or '-', assuming '+'\n" );
            action = FLAG_ADD;
            break;
         }

      //
      // WARNING: please change the Event decleration,
      // to match this function, if flags are added or
      // deleted the event must be updated.
      // 
      if ( !Q_stricmp( flag, "everyframe" ) )
         mask = EF_EVERYFRAME;
      if ( !Q_stricmp( flag, "antisbjuice" ) )
         mask = EF_ANTISBJUICE;
      else
         {
         action = FLAG_IGNORE;
         gi.Printf( "Unknown token %s.", flag );
         }

      switch (action)
         {
         case FLAG_ADD:
            edict->s.eFlags |= mask;
            break;
         case FLAG_CLEAR:
            edict->s.eFlags &= ~mask;
            break;
         case FLAG_IGNORE:
            break;
         }
      }
   }

void Entity::RenderEffects
	(
	Event *ev
	)

{
	const char *flag;
	int mask = 0;
	int action;
	int i;

	for( i = 1; i <= ev->NumArgs(); i++ )
	{
		action = 0;
		flag = ev->GetString( i );
		switch( flag[ 0 ] )
		{
		case '+':
			action = FLAG_ADD;
			flag++;
			break;
		case '-':
			action = FLAG_CLEAR;
			flag++;
			break;
		default:
			gi.Printf( "Entity::RenderEffects", "First character is not '+' or '-', assuming '+'\n" );
			action = FLAG_ADD;
			break;
		}

		//
		// WARNING: please change the Event decleration,
		// to match this function, if flags are added or
		// deleted the event must be updated.
		// 
		if( !Q_stricmp( flag, "dontdraw" ) )
			mask = RF_DONTDRAW;
		else if( !Q_stricmp( flag, "betterlighting" ) )
			mask = RF_EXTRALIGHT;
		else if( !Q_stricmp( flag, "lensflare" ) )
			mask = RF_LENSFLARE;
		else if( !Q_stricmp( flag, "viewlensflare" ) )
			mask = RF_VIEWLENSFLARE;
		else if( !Q_stricmp( flag, "lightoffset" ) )
			mask = RF_LIGHTOFFSET;
		else if( !Q_stricmp( flag, "skyorigin" ) )
			mask = RF_SKYORIGIN;
		else if( !Q_stricmp( flag, "fullbright" ) )
			mask = RF_FULLBRIGHT;
		else if( !Q_stricmp( flag, "minlight" ) )
			mask = RF_MINLIGHT;
		else if( !Q_stricmp( flag, "additivedynamiclight" ) )
			mask = RF_ADDITIVE_DLIGHT;
		else if( !Q_stricmp( flag, "lightstyledynamiclight" ) )
			mask = RF_LIGHTSTYLE_DLIGHT;
		else if( !Q_stricmp( flag, "shadow" ) )
			mask = RF_SHADOW;
		else if( !Q_stricmp( flag, "preciseshadow" ) )
			mask = RF_SHADOW_PRECISE;
		else if( !Q_stricmp( flag, "invisible" ) )
			mask = RF_INVISIBLE;
		else
		{
			action = FLAG_IGNORE;
			gi.Printf( "Unknown token %s.", flag );
		}

		switch( action )
		{
		case FLAG_ADD:
			edict->s.renderfx |= mask;
			break;
		case FLAG_CLEAR:
			edict->s.renderfx &= ~mask;
			break;
		case FLAG_IGNORE:
			break;
		}
	}
}

void Entity::SVFlags
	(
	Event *ev
	)

{
	const char *flag;
	int mask = 0;
	Entity *ent = NULL;
	int singleClient = 0;
	int action;
	int i;

	for( i = 1; i <= ev->NumArgs(); i++ )
	{
		action = 0;
		flag = ev->GetString( i );
		switch( flag[ 0 ] )
		{
		case '+':
			action = FLAG_ADD;
			flag++;
			break;
		case '-':
			action = FLAG_CLEAR;
			flag++;
			break;
		default:
			gi.Printf( "Entity::SVFlags: First character is not '+' or '-', assuming '+'\n" );
			action = FLAG_ADD;
			break;
		}

		//
		// WARNING: please change the Event decleration,
		// to match this function, if flags are added or
		// deleted the event must be updated.
		// 
		if( !Q_stricmp( flag, "broadcast" ) )
		{
			mask = SVF_BROADCAST;
		}
		else if( !Q_stricmp( flag, "sendonce" ) )
		{
			mask = SVF_SENDONCE;
		}
		else if( !Q_stricmp( flag, "singleclient" ) )
		{
			mask = SVF_SINGLECLIENT;
			if( action == FLAG_ADD )
			{
				ent = ev->GetEntity( i + 1 );

				if( !ent )
				{
					ScriptError( "No player specified for +singleclient!" );
				}
			}

			i++;
		}
		else if( !Q_stricmp( flag, "notsingleclient" ) )
		{
			mask = SVF_NOTSINGLECLIENT;
			if( action == FLAG_ADD )
			{
				ent = ev->GetEntity( i + 1 );

				if( !ent )
				{
					ScriptError( "No player specified for +notsingleclient!" );
				}
			}

			i++;
		}
		else
		{
			action = FLAG_IGNORE;
			gi.Printf( "Entity::SVFlags: Unknown token %s.\n", flag );
		}

		switch( action )
		{
		case FLAG_ADD:
			edict->r.svFlags |= mask;

			if( ( mask & SVF_SINGLECLIENT ) || ( mask & SVF_NOTSINGLECLIENT ) )
			{
				edict->r.singleClient = singleClient;
			}

			break;
		case FLAG_CLEAR:
			edict->r.svFlags &= ~mask;

			if( ( mask & SVF_SINGLECLIENT ) || ( mask & SVF_NOTSINGLECLIENT ) )
			{
				edict->r.singleClient = 0;
			}

			break;
		case FLAG_IGNORE:
			break;
		}
	}

	if( edict->r.svFlags & SVF_SENDONCE )
	{
		// Turn this entity into an event if the SENDONCE flag is sent
		edict->s.eType = ET_EVENTS;
	}
}

void Entity::BroadcastAIEvent
	(
	int iType,
	float rad
	)

{
	G_BroadcastAIEvent( this, origin, iType, rad );
}

void Entity::BroadcastAIEvent
	(
	Event *ev
	)

{
	int iType;
	float rad;
	str sTypeName;

	if( !( this->flags & FL_NOTARGET ) )
	{
		iType = 8;

		if( ev->NumArgs() > 0 )
		{
			sTypeName = ev->GetString( 1 );
			iType = G_AIEventTypeFromString( sTypeName );

			rad = ev->NumArgs() <= 1 ? G_AIEventRadius( iType ) : ev->GetFloat( 2 );
		}

		BroadcastAIEvent( iType, rad );
	}
}

void Entity::PreAnimate
	(
	void
	)

{
}

void Entity::DoForceActivate
	(
	void
	)

{
	edict->r.lastNetTime = level.inttime;
}

void Entity::ClientThink
	(
	void
	)

{

}

void Entity::Think
	(
	void
	)

{
}

void Entity::SetWaterType
   (
   void
   )

   {
   qboolean isinwater;

   watertype = gi.PointContents( origin, 0 );
	isinwater = watertype & MASK_WATER;

	if ( isinwater )
		{
		waterlevel = 1;
		}
	else
		{
		waterlevel = 0;
		}
   }

void Entity::DamageSkin
   (
   trace_t * trace,
   float damage
   )

   {
   /* FIXME : Do we need damage skins?
   int surface;

   // FIXME handle different bodyparts
   surface = trace->intersect.surface;
   if ( !edict->s.surfaces[ surface ] )
		{
      edict->s.surfaces[ surface ]++;
		}
   */
   }

void Entity::Kill
	(
	Event *ev
	)

	{
	health = 0;
   Damage( this, this, 10, origin, vec_zero, vec_zero, 0, 0, MOD_SUICIDE );
	}


void Entity::SurfaceCommand
	(
	const char * surf_name,
	const char * token
	)

{
	const char * current_surface_name;
	int surface_num;
	int mask;
	int action;
	qboolean do_all = false;
	qboolean mult = false;

	if( surf_name[ strlen( surf_name ) - 1 ] == '*' )
	{
		mult = true;
		surface_num = 0;
	}
	else if( str( surf_name ) != str( "all" ) )
	{
		surface_num = gi.Surface_NameToNum( edict->tiki, surf_name );

		if( surface_num < 0 )
		{
			warning( "SurfaceCommand", "group %s not found.", surf_name );
			return;
		}
	}
	else
	{
		surface_num = 0;
		do_all = true;
	}

	action = 0;
	switch( token[ 0 ] )
	{
	case '+':
		action = FLAG_ADD;
		token++;
		break;
	case '-':
		action = FLAG_CLEAR;
		token++;
		break;
	default:
		warning( "Entity::SurfaceModelEvent", "First character is not '+' or '-', assuming '+'\n" );
		action = FLAG_ADD;
		break;
	}
	//
	// WARNING: please change the Event decleration,
	// to match this function, if flags are added or
	// deleted the event must be updated.
	// 
	if( !Q_stricmp( token, "skin1" ) )
	{
		mask = MDL_SURFACE_SKINOFFSET_BIT0;
	}
	else if( !Q_stricmp( token, "skin2" ) )
	{
		mask = MDL_SURFACE_SKINOFFSET_BIT1;
	}
	else if( !Q_stricmp( token, "nodraw" ) )
	{
		mask = MDL_SURFACE_NODRAW;
	}
	else if( !Q_stricmp( token, "crossfade" ) )
	{
		mask = MDL_SURFACE_CROSSFADE_SKINS;
	}
	else
	{
		mask = 0;
		warning( "SurfaceCommand", "Unknown token %s.", token );
		action = FLAG_IGNORE;
	}

	int numsurfaces;
	if( edict->tiki )
		numsurfaces = gi.NumSurfaces( edict->tiki );
	else
		numsurfaces = 0;

	for( ; surface_num < numsurfaces; surface_num++ )
	{
		if( mult )
		{
			current_surface_name = gi.Surface_NumToName( edict->tiki, surface_num );

			if( Q_stricmpn( current_surface_name, surf_name, strlen( surf_name ) - 1 ) != 0 )
				continue;
		}

		switch( action )
		{
		case FLAG_ADD:
			edict->s.surfaces[ surface_num ] |= mask;
			break;
		case FLAG_CLEAR:
			edict->s.surfaces[ surface_num ] &= ~mask;
			break;
		case FLAG_IGNORE:
			break;
		}

		if( !do_all && !mult )
			break;
	}
}

void Entity::SurfaceModelEvent
	(
	Event *ev
	)

{
	const char * surf_name;
	const char * token;
	int i;

	surf_name = ev->GetString( 1 );

	for( i = 2; i <= ev->NumArgs(); i++ )
	{
		token = ev->GetString( i );
		SurfaceCommand( surf_name, token );
	}
}

void Entity::SetShaderData
	(
	Event *ev
	)

{
	edict->s.shader_data[ 0 ] = ev->GetFloat( 1 );
	edict->s.shader_data[ 1 ] = ev->GetFloat( 2 );
}

void Entity::AttachEvent
	(
	Event * ev
	)
	{
   Entity * parent;
   const char * bone;
   int tagnum;
	qboolean use_angles = qtrue;
	Vector offset;

	parent = ev->GetEntity( 1 );
   bone = ev->GetString( 2 );

	if ( ev->NumArgs() > 2 )
		use_angles = ev->GetInteger( 3 );

	if ( ev->NumArgs() > 3 )
		offset = ev->GetVector( 4 );

   if ( !parent )
      return;

   tagnum = gi.Tag_NumForName( parent->edict->tiki, bone );
   if ( tagnum >= 0 )
      {
      attach( parent->entnum, tagnum, use_angles, offset );
      }
   else
      {
      warning( "AttachEvent", "Tag %s not found", bone );
      }
   }

void Entity::AttachModelEvent
(
Event * ev
)
{
	Animate *obj;
	const char * bone;
	str modelname;
	int tagnum;
	float remove_time, fade_time, fade_delay;
	Vector offset;

	obj = new Animate;

	modelname = ev->GetString( 1 );
	bone = ev->GetString( 2 );
	if( ev->NumArgs() > 2 )
	{
		obj->setScale( ev->GetFloat( 3 ) );
	}
	if( ev->NumArgs() > 3 )
	{
		obj->SetTargetName( ev->GetString( 4 ) );
	}

	if( ev->NumArgs() > 4 )
		obj->detach_at_death = ev->GetInteger( 5 );

	if( ev->NumArgs() > 5 )
	{
		remove_time = ev->GetFloat( 6 );

		if( remove_time != -1 )
		{
			Event *remove_event = new Event( EV_Remove );
			obj->PostEvent( remove_event, remove_time );
		}
	}

	if( ev->NumArgs() > 6 )
	{
		Event *fade_event;

		fade_time = ev->GetFloat( 7 );

		if( fade_time != -1 )
		{
			obj->setAlpha( 0 );

			fade_event = new Event( EV_FadeIn );
			fade_event->AddFloat( fade_time );
			obj->PostEvent( fade_event, 0 );
		}
	}

	if( ev->NumArgs() > 7 )
	{
		Event *fade_event;

		fade_delay = ev->GetFloat( 8 );

		if( fade_delay != -1 )
		{
			if( ev->NumArgs() > 8 )
				fade_time = ev->GetFloat( 9 );
			else
				fade_time = 0;

			fade_event = new Event( EV_Fade );

			if( fade_time )
				fade_event->AddFloat( fade_time );

			obj->PostEvent( fade_event, fade_delay );
		}
	}

	if( ev->NumArgs() > 9 )
		offset = ev->GetVector( 10 );

	obj->setModel( modelname );

	tagnum = gi.Tag_NumForName( edict->tiki, bone );
	if( tagnum >= 0 )
	{
		if( obj->attach( this->entnum, tagnum, true, offset ) )
		{
			obj->NewAnim( "idle" );
		}
		else
		{
			delete obj;
			return;
		}
	}
	else
	{
		warning( "AttachModelEvent", "Tag %s not found", bone );
	}
}

void Entity::RemoveAttachedModelEvent
	(
	Event *ev
	)
	{
   const char *tag_name;
   int tag_num;
	int num;
	int i;
	Entity *ent;
	float fade_rate = 0;
	Event *fade_event;
	str model_name;

   tag_name = ev->GetString( 1 );
   tag_num = gi.Tag_NumForName( edict->tiki, tag_name );

	if ( ev->NumArgs() > 1 )
		fade_rate = ev->GetFloat( 2 );

	if ( ev->NumArgs() > 2 )
		model_name = ev->GetString( 3 );

   if ( tag_num >= 0 )
      {
		num = numchildren;

		for ( i = 0 ; (i < MAX_MODEL_CHILDREN) && num ; i++ )
			{
			if ( children[i] == ENTITYNUM_NONE )
				continue;

			ent = ( Entity * )G_GetEntity( children[i] );

			if ( ent->edict->s.tag_num == tag_num )
				{
				if ( !model_name.length() || model_name == ent->model )
					{
					if ( fade_rate )
						{
						fade_event = new Event( EV_Fade );
						fade_event->AddFloat( fade_rate );
						fade_event->AddFloat( 0 );
						ent->PostEvent( fade_event, 0 );
						}

					ent->PostEvent( EV_Remove, fade_rate );
					}
				}
			
			num--;
			}
      }
   }

void Entity::DetachEvent
	(
	Event * ev
	)

	{
   if ( edict->s.parent == ENTITYNUM_NONE  )
		{
      return;
		}
   detach();
   }

void Entity::TakeDamageEvent
	(
	Event * ev
	)
	{
   takedamage = DAMAGE_YES;
   }

void Entity::NoDamageEvent
	(
	Event * ev
	)
	{
   takedamage = DAMAGE_NO;
   }

void Entity::Gravity
   (
   Event *ev
   )

   {
   gravity = ev->GetFloat( 1 );
   }

void Entity::UseBoundingBoxEvent
	(
	Event *ev
	)

{
	edict->r.svFlags |= SVF_USEBBOX;
}

void Entity::HurtEvent
	(
	Event *ev
	)

{
	Vector normal;
	float dmg;
	int means_of_death;
	Vector direction;

	if( ev->NumArgs() < 1 )
	{
		dmg = 50;
	}
	else
	{
		dmg = ev->GetFloat( 1 );
	}

	if( ev->NumArgs() > 1 )
		means_of_death = MOD_NameToNum( ev->GetString( 2 ) );
	else
		means_of_death = MOD_CRUSH;

	if( ev->NumArgs() > 2 )
	{
		direction = ev->GetVector( 3 );
		direction.normalize();
	}
	else
	{
		direction = vec_zero;
	}

	normal = Vector( orientation[ 0 ] );
	Damage( world, world, dmg, centroid, direction, normal, dmg, 0, means_of_death );
}

void Entity::HealEvent
	(
	Event *ev
	)

{
	if( IsDead() )
	{
		ScriptError( "cannot heal dead entities" );
	}

	health += ev->GetFloat( 1 ) * max_health;
	if( health > max_health )
	{
		health = max_health;
	}
}

void Entity::Censor
	(
	Event *ev
	)

	{
   Vector delta;
   float oldsize;
   float newsize;

   oldsize = size.length();
	setSolidType( SOLID_NOT );
   setModel( "censored.tik" );
   gi.CalculateBounds( edict->tiki, 1, mins, maxs );
   delta = maxs - mins;
   newsize = delta.length();
   edict->s.scale = oldsize / newsize;
   mins *= edict->s.scale;
   maxs *= edict->s.scale;
   setSize( mins, maxs );
   setOrigin();
	}

void Entity::StationaryEvent
   (
   Event *ev
   )

	{
	setMoveType( MOVETYPE_STATIONARY );
	}

void Entity::Explosion
   (
   Event *ev
   )

   {
   str expmodel;
	str tag_name;
	//orientation_t orient;
	Vector explosion_origin;
   
	expmodel = ev->GetString( 1 );
	explosion_origin = origin;

	if ( ev->NumArgs() > 1 )
		{
		tag_name = ev->GetString( 2 );

		//if ( GetRawTag( tag_name.c_str(), &orient, legs ) )
		//	VectorAdd( orient.origin, origin, explosion_origin );

		GetTag( tag_name.c_str(), &explosion_origin );
		}

   ExplosionAttack( explosion_origin, this, expmodel );
   }

void Entity::Shader
	(
	Event *ev
	)

	{
   const char * token;

   if( gi.modeltiki( model ) )
      {
      gi.Printf( "shader event being called on TIKI model\n" );
      }
   //
   // get sub shader command
   //
   token = ev->GetString( 1 );

   //
   // WARNING: please change the Event decleration,
   // to match this function, if flags are added or
   // deleted the event must be updated.
   // 
   if (!Q_stricmp( token, "translation"))
      {
      float x, y;

      x = ev->GetFloat( 2 );
      y = ev->GetFloat( 3 );
      TRANSLATION_TO_PKT( x, edict->s.tag_num );
      TRANSLATION_TO_PKT( y, edict->s.skinNum );
      }
   else if (!Q_stricmp( token, "offset"))
      {
      float x, y;

      x = ev->GetFloat( 2 );
      y = ev->GetFloat( 3 );
      OFFSET_TO_PKT( x, edict->s.tag_num );
      OFFSET_TO_PKT( y, edict->s.skinNum );
      }
   else if (!Q_stricmp (token, "rotation"))
      {
      float rot;

      rot = ev->GetFloat( 2 );
      ROTATE_TO_PKT( rot, edict->s.tag_num );
      }
   else if (!Q_stricmp (token, "wavebase"))
      {
      float base;

      base = ev->GetFloat( 2 );
      BASE_TO_PKT( base, edict->s.surfaces[ 0 ] );
      }
   else if (!Q_stricmp (token, "waveamp"))
      {
      float amp;

      amp = ev->GetFloat( 2 );
      AMPLITUDE_TO_PKT( amp, edict->s.surfaces[ 1 ] );
      }
   else if (!Q_stricmp (token, "wavephase"))
      {
      float phase;

      phase = ev->GetFloat( 2 );
      PHASE_TO_PKT( phase, edict->s.surfaces[ 2 ] );
      }
   else if (!Q_stricmp (token, "wavefreq"))
      {
      float freq;

      freq = ev->GetFloat( 2 );
      FREQUENCY_TO_PKT( freq, edict->s.surfaces[ 3 ] );
      }
	}

void Entity::DropToFloorEvent
   (
   Event *ev
   )

   {
   float range;

	if ( ev->NumArgs() > 0 )
      {
      range = ev->GetFloat( 1 );
      }
   else
      {
      range = MAP_SIZE;
      }
   if ( !droptofloor( range ) )
      {
      }
	}


//*************************************************************************
//
// BIND code 
//
//*************************************************************************

qboolean Entity::isBoundTo
   (
   Entity *master
   )

   {
   Entity *ent;

   for( ent = bindmaster; ent != NULL; ent = ent->bindmaster )
      {
      if ( ent == master )
         {
         return true;
         }
      }

   return false;
   }

void Entity::bind
	(
	Entity *master,
	qboolean use_my_angles,
	qboolean bBindChilds
	)

{
	float  mat[ 3 ][ 3 ];
	float  local[ 3 ][ 3 ];
	Vector ang;

	assert( master );
	if( !master )
	{
		warning( "bind", "Null master entity" );
		return;
	}

	if( master == this )
	{
		warning( "bind", "Trying to bind to oneself." );
		return;
	}

	m_bBindChilds = bBindChilds;

	// unbind myself from my master
	unbind();

	bindmaster = master;
	bind_use_my_angles = use_my_angles;

	// We are now separated from our previous team and are either
	// an individual, or have a team of our own.  Now we can join
	// the new bindmaster's team.  Bindmaster must be set before
	// joining the team, or we will be placed in the wrong position
	// on the team.
	joinTeam( master );

	// calculate local angles
	TransposeMatrix( bindmaster->orientation, mat );
	R_ConcatRotations( mat, orientation, local );
	MatrixToEulerAngles( local, ang );
	setAngles( ang );

	setLocalOrigin( getParentVector( localorigin - bindmaster->origin ) );

	return;
}

void Entity::unbind
	(
	void
	)

{
	Entity *prev;
	Entity *next;
	Entity *last;
	Entity *ent;

	if( !bindmaster )
	{
		return;
	}

	//bindmaster = NULL;

	// Check this GAMEFIX - should it be origin?
	localorigin = Vector( edict->s.origin );
	localangles = Vector( edict->s.angles );

	if( !teammaster )
	{
		bindmaster = NULL;
		//Teammaster already has been freed
		return;
	}

	// We're still part of a team, so that means I have to extricate myself
	// and any entities that are bound to me from the old team.
	// Find the node previous to me in the team
	prev = teammaster;

	for( ent = teammaster->teamchain; ent && ( ent != this ); ent = ent->teamchain )
	{
		prev = ent;
	}

	// If ent is not pointing to me, then something is very wrong.
	assert( ent );
	if( !ent )
	{
		gi.Printf( "unbind", "corrupt team chain\n" );
	}

	// Find the last node in my team that is bound to me.
	// Also find the first node not bound to me, if one exists.
	last = this;
	for( next = teamchain; next != NULL; next = next->teamchain )
	{
		if( !next->isBoundTo( this ) )
		{
			break;
		}

		// Tell them I'm now the teammaster
		next->teammaster = this;
		last = next;
	}

	// disconnect the last member of our team from the old team
	last->teamchain = NULL;

	// connect up the previous member of the old team to the node that
	// follow the last node bound to me (if one exists).
	if( teammaster != this )
	{
		prev->teamchain = next;
		if( !next && ( teammaster == prev ) )
		{
			prev->teammaster = NULL;
		}
	}
	else if( next )
	{
		// If we were the teammaster, then the nodes that were not bound to me are now
		// a disconnected chain.  Make them into their own team.
		for( ent = next; ent->teamchain != NULL; ent = ent->teamchain )
		{
			ent->teammaster = next;
		}
		next->teammaster = next;
		next->flags &= ~FL_TEAMSLAVE;
	}

	// If we don't have anyone on our team, then clear the team variables.
	if( teamchain )
	{
		// make myself my own team
		teammaster = this;
	}
	else
	{
		// no longer a team
		teammaster = NULL;
	}

	flags &= ~FL_TEAMSLAVE;
	bindmaster = NULL;
}

void Entity::glue
	(
	Entity *master,
	qboolean use_my_angles
	)

{
	int iNumGlues;

	if( m_pGlueMaster ) {
		unglue();
	}

	if( !master )
	{
		warning( "glue", "Cannot glue to master because of: Master is NULL\n" );
		return;
	}

	if( master == this )
	{
		warning( "glue", "Cannot glue to master because of: Binding an entity to itself\n" );
		return;
	}

	iNumGlues = master->m_iNumGlues;
	if( iNumGlues + 1 > MAX_GLUE_CHILDREN )
	{
		warning( "glue", "Cannot glue to master because of: MAX_GLUE_CHILDREN reached\n" );
		return;
	}

	for( int i = 0; i < iNumGlues; i++ )
	{
		if( master->m_pGlues[ i ] == this )
		{
			warning( "glue", "Cannot glue to master because of: entity is already glued\n" );
			return;
		}
	}

	master->m_pGlues[ iNumGlues ] = this;

	if( use_my_angles )
	{
		m_bGlueAngles = true;
		master->m_pGluesFlags[ iNumGlues ] = GL_USEANGLES;
	}
	else
	{
		m_bGlueAngles = false;
		master->m_pGluesFlags[ iNumGlues ] = 0;
	}

	m_pGlueMaster = master;
	master->m_iNumGlues++;

	setAngles( master->angles );
	setOrigin( master->origin );
}

void Entity::unglue
	(
	void
	)

{
	int iNumGlues;
	int i;
	Entity *master = m_pGlueMaster;

	if( !master ) {
		return;
	}

	iNumGlues = master->m_iNumGlues;

	for( i = 0; i < iNumGlues; i++ )
	{
		if( master->m_pGlues[ i ] == this ) {
			break;
		}
	}

	if( i != iNumGlues )
	{
		for( ; i < iNumGlues - 1; i++ )
		{
			master->m_pGlues[ i ] = master->m_pGlues[ i + 1 ];
			master->m_pGluesFlags[ i ] = master->m_pGluesFlags[ i + 1 ];
		}

		master->m_iNumGlues--;

		m_pGlueMaster = NULL;
		m_bGlueAngles = false;
	}
}

void Entity::EventUnbind
	(
	Event *ev
	)

	{
	unbind();
	}

void Entity::BindEvent
	(
	Event *ev
	)

{
	Entity *ent;
	bool bBindChilds;

	ent = ev->GetEntity( 1 );
	if( ent )
	{
		if( ev->NumArgs() > 1 )
		{
			bBindChilds = ev->GetBoolean( 2 );
		}
		else
		{
			bBindChilds = false;
		}

		bind( ent, false, bBindChilds );
	}
}

void Entity::EventUnglue
	(
	Event *ev
	)

{
	unglue();
}

void Entity::GlueEvent
	(
	Event *ev
	)

{
	Entity *ent;
	qboolean glueAngles;

	if( ev->NumArgs() > 1 )
	{
		glueAngles = ev->GetInteger( 2 );
	}
	else
	{
		glueAngles = qtrue;
	}

	ent = ev->GetEntity( 1 );
	if( ent )
	{
		glue( ent, glueAngles );
	}
}

Vector Entity::getParentVector
	(
	Vector vec
	)

{
	Vector pos;

	if( !bindmaster )
	{
		return vec;
	}

	pos[ 0 ] = vec * bindmaster->orientation[ 0 ];
	pos[ 1 ] = vec * bindmaster->orientation[ 1 ];
	pos[ 2 ] = vec * bindmaster->orientation[ 2 ];

	return pos;
}

//
// Team methods
//

void Entity::joinTeam
	(
	Entity *teammember
	)

{
	Entity *ent;
	Entity *master;
	Entity *prev;
	Entity *next;

	if( teammaster && ( teammaster != this ) )
	{
		quitTeam();
	}

	assert( teammember );
	if( !teammember )
	{
		warning( "joinTeam", "Null entity" );
		return;
	}

	master = teammember->teammaster;
	if( !master )
	{
		master = teammember;
		teammember->teammaster = teammember;
		teammember->teamchain = this;

		// make anyone who's bound to me part of the new team
		for( ent = teamchain; ent != NULL; ent = ent->teamchain )
		{
			ent->teammaster = master;
		}
	}
	else
	{
		// skip past the chain members bound to the entity we're teaming up with
		prev = teammember;
		next = teammember->teamchain;
		if( bindmaster )
		{
			// if we have a bindmaster, joing after any entities bound to the entity
			// we're joining
			while( next && ( ( Entity * )next )->isBoundTo( teammember ) )
			{
				prev = next;
				next = next->teamchain;
			}
		}
		else
		{
			// if we're not bound to someone, then put us at the end of the team
			while( next )
			{
				prev = next;
				next = next->teamchain;
			}
		}

		// make anyone who's bound to me part of the new team and
		// also find the last member of my team
		for( ent = this; ent->teamchain != NULL; ent = ent->teamchain )
		{
			ent->teamchain->teammaster = master;
		}

		prev->teamchain = this;
		ent->teamchain = next;
	}

	teammaster = master;
	flags |= FL_TEAMSLAVE;
}

void Entity::quitTeam
	(
	void
	)

{
	Entity *ent;

	if( !teammaster )
	{
		return;
	}

	if( teammaster == this )
	{
		if( !teamchain->teamchain )
		{
			teamchain->teammaster = NULL;
		}
		else
		{
			// make next teammate the teammaster
			for( ent = teamchain; ent; ent = ent->teamchain )
			{
				ent->teammaster = teamchain;
			}
		}

		teamchain->flags &= ~FL_TEAMSLAVE;
	}
	else
	{
		assert( flags & FL_TEAMSLAVE );
		assert( teammaster->teamchain );

		ent = teammaster;
		while( ent->teamchain != this )
		{
			// this should never happen
			assert( ent->teamchain );

			ent = ent->teamchain;
		}

		ent->teamchain = teamchain;

		if( !teammaster->teamchain )
		{
			teammaster->teammaster = NULL;
		}
	}

	teammaster = NULL;
	teamchain = NULL;
	flags &= ~FL_TEAMSLAVE;
}

void Entity::EventQuitTeam
	(
	Event *ev
	)

	{
	quitTeam();
	}


void Entity::JoinTeam
	(
	Event *ev
	)

	{
	Entity *ent;

	ent = ev->GetEntity( 1 );
	if ( ent )
		{
		joinTeam( ent );
		}
	}

void Entity::AddToSoundManager
	(
	Event *ev
	)

	{
   SoundMan.AddEntity( this );
	}

inline qboolean Entity::HitSky
	(
	trace_t *trace
	)

{
	assert( trace );
	if( trace->surfaceFlags & SURF_SKY )
	{
		return true;
	}
	return false;
}

qboolean Entity::HitSky
	(
	void
	)

{
	return HitSky( &level.impact_trace );
}

void Entity::SetAngleEvent
	(
	Event *ev
	)
	{
   Vector movedir;

   movedir = G_GetMovedir( ev->GetFloat( 1 ) );
	setAngles( movedir.toAngles() );
   }

void Entity::NoLerpThisFrame
	(
   void
	)
	{
   edict->s.eFlags ^= EF_TELEPORT_BIT;
   }

void Entity::PostAnimate( void )
{
}

void Entity::Postthink( void )
{
}

void Entity::TouchTriggersEvent
	(
	Event *ev
	)
	{
	flags |= FL_TOUCH_TRIGGERS;
   }

void Entity::DeathSinkStart( Event *ev )
{
	if( g_gametype->integer )
	{
		PostEvent( EV_Remove, 5.0f );
	}
	else
	{
		// Start the sinking 
		ProcessEvent( EV_DeathSink );
	}
}

void Entity::DeathSink( Event *ev )
{
	// Sink just a little

	if( g_gametype->integer )
	{
		origin[ 2 ] -= 0.2f;
		setOrigin( origin );

		// Make sure the sink happens again next frame
		PostEvent( EV_DeathSink, FRAMETIME );
	}
	else
	{
		Entity *player = G_GetEntity( 0 );

		// Remove the entity when the player isn't supposed to see it
		if( player && gi.AreasConnected( this->edict->r.areanum, player->edict->r.areanum ) )
		{
			PostEvent( EV_DeathSink, 1.0f );
		}
		else
		{
			PostEvent( EV_Remove, 0 );
		}
	}
	
}

void Entity::EndFrame
	(
	void
	)

{
}

void Entity::CalcBlend
	(
	void
	)

{
}

void Entity::VelocityModified
	(
	void
	)

{
}

void Entity::DetachAllChildren
	(
	void
	)

{
	int i;

	for( i = 0; i < MAX_MODEL_CHILDREN; i++ )
	{
		Entity * ent;
		if( children[ i ] == ENTITYNUM_NONE )
			continue;

		ent = ( Entity * )G_GetEntity( children[ i ] );
		if( ent )
		{
			ent->PostEvent( EV_Remove, 0 );
		}
	}
}

void Entity::DetachAllChildren
	(
	Event *ev
	)

{
	DetachAllChildren();
}

void Entity::SetMovementStealth( float fStealthScale )
{
	if( fStealthScale < 0.0f )
	{
		stealthMovementScale = 0.0f;
	}
	else
	{
		stealthMovementScale = fStealthScale;
	}
}

void Entity::EventMovementStealth
	(
	Event *ev
	)

{
	SetMovementStealth( ev->GetFloat( 1 ) );
}

void Entity::GetYaw
	(
	Event *ev
	)

{
	ev->AddFloat( angles[ 1 ] );
}

void Entity::PusherEvent
	(
	Event *ev
	)

{
	Entity *inflictor;
	Entity *attacker;
	Vector dir;
	Vector momentum;
	float m;
	float force;

	if( movetype <= MOVETYPE_NOCLIP ||
		movetype == MOVETYPE_FLYMISSILE ||
		movetype == MOVETYPE_STOP ||
		movetype == MOVETYPE_PUSH )
	{
		return;
	}

	inflictor = ev->GetEntity( 1 );
	attacker = ev->GetEntity( 2 );
	dir = ev->GetVector( 3 );
	force = ev->GetFloat( 4 );

	m = mass;
	if( m < 20 )
	{
		m = 20.0f;
	}

	m = 1000.0f / m;
	momentum = dir * force * m;

	velocity += momentum;
}

void Entity::AlwaysDraw
	(
	Event *ev
	)

{
	edict->s.renderfx |= RF_ALWAYSDRAW;
}

void Entity::NormalDraw
	(
	Event *ev
	)

{
	edict->s.renderfx &= RF_DONTDRAW | RF_ALWAYSDRAW;
}

void Entity::NeverDraw
	(
	Event *ev
	)

{
	edict->s.renderfx |= RF_DONTDRAW;
}

void Entity::GetMins
	(
	Event *ev
	)

{
	ev->AddVector( mins );
}

void Entity::GetMaxs
	(
	Event *ev
	)

{
	ev->AddVector( maxs );
}

void Entity::ForceActivate
	(
	Event *ev
	)

{
	DoForceActivate();
}

void Entity::EventTrace
	(
	Event *ev
	)

{
	Vector start, end, mins, maxs;
	int mask = 0x2000B01;
	Entity *ent;
	trace_t trace;

	mins = vec_zero;
	maxs = vec_zero;
	ent = NULL;

	start = ev->GetVector( 1 );
	end = ev->GetVector( 2 );

	if( ev->NumArgs() > 2 )
	{
		if( ev->GetInteger( 3 ) ) {
			mask = 1;
		}
	}

	if( ev->NumArgs() > 3 ) {
		mins = ev->GetVector( 4 );
	}

	if( ev->NumArgs() > 4 ) {
		maxs = ev->GetVector( 5 );
	}

	if( ev->NumArgs() > 5 ) {
		ent = G_GetEntity( ev->GetInteger( 6 ) );
	}

	// call trace
	G_Trace( start,
		mins,
		maxs,
		end,
		ent,
		mask,
		qfalse,
		"Entity::EventTrace" );

	ev->AddVector( trace.endpos );
}

void Entity::EventSightTrace
	(
	Event *ev
	)

{
	Vector start, end, mins, maxs;
	int mask = 0x2000B01;
	Entity *ent;
	qboolean hit = qfalse;

	mins = vec_zero;
	maxs = vec_zero;

	start = ev->GetVector( 1 );
	end = ev->GetVector( 2 );

	if( ev->NumArgs() > 2 )
	{
		if( ev->GetInteger( 3 ) ) {
			mask = 1;
		}
	}

	if( ev->NumArgs() > 3 ) {
		mins = ev->GetVector( 4 );
	}

	if( ev->NumArgs() > 4 ) {
		maxs = ev->GetVector( 5 );
	}

	if( ev->NumArgs() > 5 ) {
		ent = G_GetEntity( ev->GetInteger( 6 ) );
	}

	hit = G_SightTrace(
		start,
		mins,
		maxs,
		end,
		ent,
		NULL,
		mask,
		qfalse,
		"Entity::EventSightTrace"
		);

	ev->AddInteger( hit );
}

void Entity::ConnectPaths
	(
	void
	)

{
	PathNode *node;
	int to;
	int j;

	for( int i = 0; i < m_iNumBlockedPaths; i++ )
	{
		to = m_BlockedPaths[ i ].to;
		node = PathManager.pathnodes[ m_BlockedPaths[ i ].from ];

		j = node->virtualNumChildren;
		pathway_t *pathway = &node->Child[ j ];

		do
		{
			j--;
			pathway--;
		} while( pathway->node != to );

		pathway->numBlockers--;

		if( !pathway->numBlockers )
			node->ConnectChild( j );
	}

	m_iNumBlockedPaths = 0;
	if( m_BlockedPaths )
	{
		delete m_BlockedPaths;
		m_BlockedPaths = NULL;
	}
}

void Entity::DisconnectPaths
	(
	void
	)

{
	int i;
	int j;
	PathNode *node;
	pathway_t *pathway;
	pathway_ref paths[ 1024 ];
	vec3_t mins;
	vec3_t maxs;
	gentity_t *collision_ent;

	if( m_BlockedPaths ) {
		ConnectPaths();
	}

	if( IsSubclassOfVehicle() )
	{
		VehicleCollisionEntity *ent = ( ( Vehicle * )this )->GetCollisionEntity();

		if( !ent )
			return;

		collision_ent = ent->edict;
	}
	else
	{
		collision_ent = edict;
	}

	VectorSet( mins, -15.0f, -15.0f, 0 );
	VectorSet( maxs, 15.0f, 15.0f, 94.0f );

	for( i = 0; i < PathManager.nodecount; i++ )
	{
		node = PathManager.pathnodes[ i ];

		for( j = node->virtualNumChildren - 1; j >= 0; j-- )
		{
			pathway = &node->Child[ j ];

			if( !gi.SightTraceEntity( collision_ent,
				pathway->pos1,
				mins,
				maxs,
				pathway->pos2,
				MASK_PATHSOLID,
				qtrue ) )
			{
				paths[ m_iNumBlockedPaths ].from = node->nodenum;
				paths[ m_iNumBlockedPaths ].to = pathway->node;
				m_iNumBlockedPaths++;

				pathway->numBlockers++;

				if( pathway->numBlockers == 1 )
					node->DisconnectChild( j );
			}
		}
	}

	if( m_iNumBlockedPaths )
	{
		m_BlockedPaths = new pathway_ref[ m_iNumBlockedPaths ];

		for( i = 0; i < m_iNumBlockedPaths; i++ )
		{
			m_BlockedPaths[ i ] = paths[ i ];
		}
	}
}

void Entity::EventDisconnectPaths
	(
	Event *ev
	)

{
	DisconnectPaths();
}

void Entity::EventConnectPaths
	(
	Event *ev
	)

{
	ConnectPaths();
}

void Entity::EventGetEntnum
	(
	Event *ev
	)

{
	ev->AddInteger( entnum );
}

void Entity::EventSetRadnum
	(
	Event *ev
	)

{
	radnum = ev->GetInteger( 1 );
}

void Entity::EventGetRadnum
	(
	Event *ev
	)

{
	ev->AddInteger( radnum );
}

void Entity::EventSetRotatedBbox
	(
	Event *ev
	)

{
	if( ev->GetInteger( 1 ) )
	{
		edict->s.eFlags |= EF_LINKANGLES;
	}
	else
	{
		edict->s.eFlags &= ~EF_LINKANGLES;
	}

	setAngles( angles );
}

void Entity::EventGetRotatedBbox
	(
	Event *ev
	)

{
	ev->AddInteger( ( edict->s.eFlags & EF_LINKANGLES ) ? 1 : 0 );
}

void Entity::DrawBoundingBox
	(
	int showbboxes
	)

{
	if( showbboxes >= 100 )
	{
		if( g_monitorNum->integer != entnum )
		{
			if( !*g_monitor->string || g_monitor->string != targetname ) {
				return;
			}
		}

		showbboxes -= 100;
	}

	switch( showbboxes )
	{
	case 1:
		if( edict->s.solid )
		{
			G_DebugBBox( origin, mins, maxs, 1, 1, 0, 1 );
		}
		break;
	case 2:
		if( edict->s.solid )
		{
			G_DebugBBox( vec_zero, edict->r.absmin, edict->r.absmax, 1, 0, 1, 1 );
		}
		break;
	case 3:
		if( edict->tiki && !( edict->s.renderfx & RF_DONTDRAW ) )
			G_DebugBBox( origin, mins, maxs, 1, 1, 0, 1 );
		break;
	case 4:
		G_DebugBBox( origin, mins, maxs, 1, 1, 0, 1 );
		break;
	case 5:
	default:
		if( IsSubclassOfAnimate() && edict->tiki )
		{
			Animate * anim;
			vec3_t mins, maxs;

			anim = ( Animate * )this;
			// FIXME
			G_DebugBBox( origin, mins, maxs, 0, 1, 0, 1 );
		}
		else
		{
			G_DebugBBox( origin, mins, maxs, 1, 1, 0, 1 );
		}
		break;
	}
}

void Entity::PathnodeClaimRevoked
	(
	PathNode *node
	)

{
	return node->Relinquish();
}

qboolean Entity::BlocksAIMovement
	(
	void
	) const

{
	return true;
}

qboolean Entity::AIDontFace
	(
	void
	) const

{
	return true;
}

void Entity::GetZone( Event *ev )
{
	// FIXME: stub
	STUB();

#if 0
	ZoneInfo *zone = getZone();

	if( zone != NULL ) {
		ev->AddListener( zone );
	} else {
		ev->AddNil();
	}
#endif
}

void Entity::IsInZone( Event *ev )
{
	// FIXME: stub
	STUB();

#if 0
	ZoneInfo *zone = ( ZoneInfo * )ev->GetEntity( 1 );
	ZoneInfo *currentZone = getZone();

	if( zone == NULL ) {
		ScriptError( "The zone must be a valid ZoneInfo!!!\n" );
	}

	if( !zone->inheritsFrom( "ZoneInfo" ) ) {
		ScriptError( "The specified zone is not a ZoneInfo!!!\n" );
	}

	if( currentZone == zone || IsTouching( zone ) ) {
		ev->AddInteger( 1 ); // The entity is in the specified zone
	} else {
		ev->AddInteger( 0 ); // The entity is not in the specified zone
	}
#endif
}

void Entity::SetDepthHack( Event *ev )
{
	int bEnable;
	Entity * entity = (Entity*)this;

	bEnable = ev->GetInteger( 1 );

	if ( bEnable ) {
		entity->edict->s.renderfx |= RF_DEPTHHACK;
	} else {
		entity->edict->s.renderfx &= ~RF_DEPTHHACK;
	}
}

void Entity::ProcessHint(gentity_t* client, bool bShow)
{
	Player* player = (Player*)client->entity;

	if (client->client != NULL)
	{
		qboolean is_hidden = bShow && (edict->s.renderfx & RF_INVISIBLE);
		qboolean can_see = bShow && player->canUse(this, m_bHintRequiresLookAt); //( player->IsTouching( this ) || player->CanSee( this, string->fov, 94 ) == 1 );

		if (bShow && can_see && !is_hidden && !player->IsDead() && !player->IsSpectator())
		{
			if (sv_reborn->integer)
			{
				gi.MSG_SetClient(client - g_entities);

				// Send the hint string once
				gi.MSG_StartCGM(CGM_HINTSTRING);
					gi.MSG_WriteString(m_HintString);
				gi.MSG_EndCGM();
			}
		}
		else
		{
		}
	}
}

void Entity::SetHintRequireLookAt( Event *ev )
{
	m_bHintRequiresLookAt = ev->GetBoolean(1);
}

void Entity::SetHintString( Event * ev )
{
	m_HintString = ev->GetString(1);
}

void Entity::SetShader
	(
	Event *ev
	)

{
	str shadername = ev->GetString( 1 );
	qboolean fReset = false;

	if( !shadername.length() )
	{
		shadername = "default";
		fReset = true;
	}

	gi.SendServerCommand( -1, "setshader %d %s %d", entnum, shadername.c_str(), fReset );
}
