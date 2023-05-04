/*
===========================================================================
Copyright (C) 2023 the OpenMoHAA team

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

// DESCRIPTION:
// Client side command events.  These are in the .TIK files and 
// are processed when the animation specifies it to.

#include "cg_commands.h"
#include "scriptexception.h"
#include "tiki.h"

refEntity_t    *current_entity = NULL;
int            current_entity_number = -1;
centity_t      *current_centity = NULL;
float          current_scale = -1;
dtiki_t*       current_tiki = nullptr;
cvar_t         *cg_showtempmodels;
cvar_t         *cg_showemitters;
cvar_t         *cg_physics_fps;
cvar_t         *cg_detail;
cvar_t         *cg_effectdetail;
cvar_t         *cg_effect_physicsrate;

Vector			last_trace_end;

void CacheResource( const char * stuff );

ClientGameCommandManager commandManager;

Event EV_Client_StartBlock
   (
   "(",
   EV_DEFAULT,
   NULL,
   NULL,
   "Signals the beginning of a block of commands"
   );
Event EV_Client_EndBlock
   (
   ")",
   EV_DEFAULT,
   NULL,
   NULL,
   "Signals the end of a block of commands"
   );
Event EV_Client_EmitterStartOff
   (
   "startoff",
   EV_DEFAULT,
   NULL,
   NULL,
   "Signals an emitter to start in the off state (no tempmodels are emitted)"
   );
Event EV_Client_SetAlpha
   (
   "alpha",
   EV_DEFAULT,
   "f",
   "alpha",
   "Set the alpha of the spawned tempmodel"
   );
Event EV_Client_SetDieTouch
   (
   "dietouch",
   EV_DEFAULT,
   NULL,
   NULL,
   "Set the spawned tempmodels to die when they touch a solid"
   );
Event EV_Client_SetBounceFactor
   (
   "bouncefactor",
   EV_DEFAULT,
   "f",
   "factor",
   "Set the bounciness of a tempmodel when it hits a solid.\n"
   "A factor > 1 will make the model bounce higher and higher on each hit"
   );
Event EV_Client_SetBounceSound
   (
   "bouncesound",
   EV_DEFAULT,
   "sF",
   "sound [delay]",
   "When bouncing, what sound to play on impact and an option delay (default is 1 second) between playing this sound"
   );
Event EV_Client_SetBounceSoundOnce
   (
   "bouncesoundonce",
   EV_DEFAULT,
   "s",
   "sound",
   "When bouncing, what sound to play on impact one time"
   );
Event EV_Client_SetScale
   (
   "scale",
   EV_DEFAULT,
   "f",
   "scale",
   "Set the scale of a spawned tempmodel"
   );
Event EV_Client_SetScaleUpDown
   (
   "scaleupdown",
   EV_DEFAULT,
   NULL,
   NULL,
   "Set the tempmodel to scale up to scale value and then down."
   );
Event EV_Client_SetScaleMin
   (
   "scalemin",
   EV_DEFAULT,
   "f",
   "scalemin",
   "Set the minimum scale of a spawned tempmodel"
   );
Event EV_Client_SetScaleMax
   (
   "scalemax",
   EV_DEFAULT,
   "f",
   "scalemax",
   "Set the maximum scale of a spawned tempmodel"
   );
Event EV_Client_SetModel
   (
   "model",
   EV_DEFAULT,
   "sSSSSS",
   "modelname1 modelname2 modelname3 modelname4 modelname5 modelname6",
   "Set the modelname of the tempmodel.  If more than 1 model is specified, it will\n"
   "be randomly chosen when spawned"
   );
Event EV_Client_SetLife
   (
   "life",
   EV_DEFAULT,
   "f",
   "life",
   "Set the life (in seconds) of the spawned tempmodel"
   );
Event EV_Client_SetColor
   (
   "color",
   EV_DEFAULT,
   "fffF",
   "red green blue alpha",
   "Set the color (modulate) of the spawned tempmodel."
   );
Event EV_Client_SetVelocity
   (
   "velocity",
   EV_DEFAULT,
   "f",
   "forwardVelocity",
   "Set the forward velocity of the spawned tempmodel"
   );
Event EV_Client_SetAngularVelocity
   (
   "avelocity",
   EV_DEFAULT,
   "fff",
   "yawVel pitchVel rollVel",
   "Set the angular velocity of the spawned tempmodel"
   );
Event EV_Client_SetRandomVelocity
   (
   "randvel",
   EV_DEFAULT,
   "SFSFSF",
   "[random|crandom] xVel [random|crandom] yVel [random|crandom] zVel",
   "Add a random component to the regular velocity.\n"
   "If random is specified, the component will range from 0 to specified velocity.\n"
   "If crandom is specified, the component will range from -specified to +specified velocity.\n"
   "If neither random or crandom is specified, then the component will just be added on\n"
	"without randomness.\n"
   "This velocity is applied using the world axis"
   );
Event EV_Client_SetRandomVelocityAlongAxis
   (
   "randvelaxis",
   EV_DEFAULT,
   "SFSFSF",
   "[random|crandom] forwardVel [random|crandom] rightVel [random|crandom] upVel",
   "Add a random component to the regular velocity.\n"
   "If random is specified, the component will range from 0 to specified velocity.\n"
   "If crandom is specified, the component will range from -specified to +specified velocity.\n"
   "If neither random or crandom is specified, then the component will just be added on\n"
	"without randomness.\n"
   "This velocity is applied using the parent axis"
   );
Event EV_Client_SetAccel
   (
   "accel",
   EV_DEFAULT,
   "fff",
   "xAcc yAcc zAcc",
   "Set the acceleration of the spawned tempmodel.\n"
   "This acceleration is applied using the world axis"
   );
Event EV_Client_SetCount
   (
   "count",
   EV_DEFAULT,
   "i",
   "count",
   "Set the number of tempmodels that are spawned\n"
   "This is only used for the originspawn and tagspawn commands,\n"
   "and not for emitters, use spawnrate instead"
   );
Event EV_Client_SetTraceCount
   (
   "tracecount",
   EV_DEFAULT,
   "i",
   "count",
   "Set the number of traces that are done\n"
   );
Event EV_Client_SetFade
   (
   "fade",
   EV_DEFAULT,
   NULL,
   NULL,
   "Set the tempmodel to fade out over it's life"
   );
Event EV_Client_SetFadeDelay
   (
   "fadedelay",
   EV_DEFAULT,
   "f",
   "time",
   "Set the amount of time to delay a fade"
   );
Event EV_Client_SetFadeIn
   (
   "fadein",
   EV_DEFAULT,
   "f",
   "time",
   "Set the tempmodel to fade in over the specified time"
   );
Event EV_Client_SetTwinkle
   (
   "twinkle",
   EV_DEFAULT,
   "ffff",
   "mintimeoff maxtimeoff mintimeon maxtimeon",
   "Set the tempmodel to twinkle with the specified settings"
   );
Event EV_Client_SetTrail
   (
   "trail",
   EV_DEFAULT,
   "sssf",
   "shader startTag endTag life",
   "Set the tempmodel to have a swipe that follows it"
   );
Event EV_Client_SetSpawnRate
   (
   "spawnrate",
   EV_DEFAULT,
   "f",
   "rate",
   "Set the spawnrate of the emitter (models per second).\n"
   "This is only used for emitters and not for the originspawn and tagspawn commands"
   );
Event EV_Client_SetOriginOffset
   (
   "offset",
   EV_DEFAULT,
   "SFSFSF",
   "[random|crandom] offsetX [random|crandom] offsetY [random|crandom] offsetZ",
   "If random is specified, the component will range from 0 to +specified offset.\n"
   "If crandom is specified, the component will range from -specified to +specified offset.\n"
   "If neither random or crandom is specified, then the component will just be added on\n"
	"without randomness.\n"
   "This offset is applied using the world axis."
   );
Event EV_Client_SetScaleRate
   (
   "scalerate",
   EV_DEFAULT,
   "f",
   "rate",
   "Set the scaling rate of the spawned tempmodel\n"
   "If a negative rate is used, the model will shrink"
   );
Event EV_Client_SetCircle
   (
   "circle",
   EV_DEFAULT,
   NULL,
   NULL,
   "Set the tempmodels to be spawned in a circle around the origin\n"
   "This circle will be generated in the X/Y axis of the model"
   );
Event EV_Client_SetSphere
   (
   "sphere",
   EV_DEFAULT,
   NULL,
   NULL,
   "Set the tempmodels to spawn in a sphere around the origin.\n"
   "If sphereradius is set, the tempmodels will spawn at the radius distance from\n"
   "the origin"
   );
Event EV_Client_SetInwardSphere
   (
   "inwardsphere",
   EV_DEFAULT,
   NULL,
   NULL,
   "Create the tempmodels in a sphere around the origin, and adjust their\n"
   "angle so they point toward the center of the sphere.  This is best used with a\n"
   "spehere radius and some velocity so the models look like they're heading toward the\n"
   "center of the sphere."
   );
Event EV_Client_SetRadius
   (
   "radius",
   EV_DEFAULT,
   "f",
   "radius",
   "Set the radius of the sphere for the inwardsphere amd sphere settings"
   );
Event EV_Client_SetSwarm
   (
   "swarm",
   EV_DEFAULT,
   "iff",
   "frequency maxspeed delta",
   "Create a swarm like effect that the tempmodels follow when they are spawned\n"
   "frequency is how often they change direction\n"
   "maxspeed is how fast the tempmodel will move (it's randomly generated every\n"
	"time the frequency is hit\n"
   "delta is how much the tempmodel moves toward the origin every frame"
   );
Event EV_Client_SetWavy
   (
   "wavy",
   EV_DEFAULT,
   "SF",
   "[random|crandom] distance",
   "Set the tempmodel to move in a wavy path to a maxmimun of the distance specified\n"
   "If random is specified, the distance is between 0 and +specified distance\n"
   "If crandom is specified, the distance is between -specified and +specified distance"
   );
Event EV_Client_SetAlign
   (
   "align",
   EV_DEFAULT,
   NULL,
   NULL,
   "Align the tempmodels to the direction they are traveling"
   );
Event EV_Client_SetAlignOnce
   (
   "alignonce",
   EV_DEFAULT,
   NULL,
   NULL,
   "Align the tempmodels to the direction they are traveling at the time they are initialized"
   );
Event EV_Client_SetFlickerAlpha
   (
   "flicker",
   EV_DEFAULT,
   NULL,
   NULL,
   "Set the tempmodel to change it's alpha every frame.  Creates a flickering effect"
   );
Event EV_Client_SetCollision
   (
   "collision",
   EV_DEFAULT,
   "S",
   "water",
   "Turn on collision for the tempmodel.\n"
   "If the keyword water is specified, then the tempmodel will collide with water"
   );
Event EV_Client_TagSpawn
   (
   "tagspawn",
   EV_DEFAULT,
   "s",
   "tagName",
   "Spawn tempmodels from the specified tag.\n"
   "This command is followed by a ( to specify a block of commands that modify the tempmodels"
   );
Event EV_Client_OriginSpawn
   (
   "originspawn",
   EV_DEFAULT,
   NULL,
   NULL,
   "Spawn tempmodels from the origin.\n"
   "This command is followed by a ( to specify a block of commands that modify the tempmodels"
   );
Event EV_Client_OriginBeamSpawn
   (
   "originbeamspawn",
   EV_DEFAULT,
   NULL,
   NULL,
   "Spawn a beam from the origin.\n"
   "This command is followed by a ( to specify a block of commands that modify the beam"
   );
Event EV_Client_OriginBeamEmitter
   (
   "originbeamemitter",
   EV_DEFAULT,
   NULL,
   NULL,
   "Spawn beams from the origin.\n"
   "This command is followed by a ( to specify a block of commands that modify the beam"
   );
Event EV_Client_TagEmitter
   (
   "tagemitter",
   EV_DEFAULT,
   "ss",
   "tagName emitterName",
   "Create an emitter that spawns tempmodels from the specified tag.\n"
   "This command is followed by a ( to specify a block of commands that modify the tempmodels"
   );
Event EV_Client_OriginEmitter
   (
   "originemitter",
   EV_DEFAULT,
   "s",
   "emitterName",
   "Create an emitter that spawns tempmodels from the origin.\n"
   "This command is followed by a ( to specify a block of commands that modify the tempmodels"
   );
Event EV_Client_EmitterOn
   (
   "emitteron",
   EV_DEFAULT,
   "s",
   "emitterName",
   "Turn the specified emitter on"
   );
Event EV_Client_EmitterOff
   (
   "emitteroff",
   EV_DEFAULT,
   "s",
   "emitterName",
   "Turn the specified emitter off"
   );
Event EV_Client_Sound
   (
   "sound",
   EV_DEFAULT,
   "sIFF",
   "soundName channel volume min_distance",
   "Play the specified sound"
   );
Event EV_Client_StopSound
   (
   "stopsound",
   EV_DEFAULT,
   "i",
   "channel",
   "Stops the sound on the specified channel."
   );
Event EV_Client_LoopSound
   (
   "loopsound",
   EV_DEFAULT,
   "sFF",
   "soundName volume min_distance",
   "Play the specifed sound as a looping sound"
   );
Event EV_Client_Cache
   (
   "cache",
   EV_DEFAULT,
   "s",
   "resourceName",
   "Cache the specified resource"
   );
Event EV_Client_AliasCache
   (
   "aliascache",
   EV_DEFAULT,
   "ssSSSSSS",
   "alias realPath arg1 arg2 arg3 arg4 arg5 arg6",
   "Create an alias to the specified path and cache the resource"
   );
Event EV_Client_Alias
   (
   "alias",
   EV_DEFAULT,
   "ssSSSSSS",
   "alias realPath arg1 arg2 arg3 arg4 arg5 arg6",
   "Create an alias to the specified path"
   );
Event EV_Client_Footstep
   (
   "footstep",
   EV_DEFAULT,
   NULL,
   NULL,
   "Play a footstep sound that is appropriate to the surface we are currently stepping on"
   );
Event EV_Client_Client
   (
   "client",
   EV_DEFAULT,
   "SSSSSS",
   "arg1 arg2 arg3 arg4 arg5 arg6",
   "Execute the specified command arg string"
   );
Event EV_Client_OriginDynamicLight
   (
   "origindlight",
   EV_DEFAULT,
   "fffffSS",
   "red green blue intensity life type1 type2",
   "Spawn a dynamic light from the origin of the model\n"
   "The red,green,blue parms are the color of the light\n"
   "The intensity is the radius of the light\n"
   "type is the type of light to create (lensflare,viewlensflare,additive)"
   );
Event EV_Client_TagDynamicLight
   (
   "tagdlight",
   EV_DEFAULT,
   "sfffffSS",
   "tagName red green blue intensity life type1 type2",
   "Spawn a dynamic light from the specified tag\n"
   "The red,green,blue parms are the color of the light\n"
   "The intensity is the radius of the light\n"
   "type is the type of light to create (lensflare,viewlensflare,additive)"
   );
Event EV_Client_DynamicLight
   (
   "dlight",
   EV_DEFAULT,
   "ffffSS",   
   "red green blue intensity type1 type2",
   "This makes the emitter itself a dynamic light"
   "The red,green,blue parms are the color of the light\n"
   "The intensity is the radius of the light\n"
   "type is the type of light to create (lensflare,viewlensflare,additive)"
   );
Event EV_Client_SetEntityColor
   (
   "entcolor",
   EV_DEFAULT,
   "fffF",
   "red green blue alpha",
   "Set the color(modulate) of this entity"
   );
Event EV_Client_SetTexAnimTime
   (
   "texanimtime",
   EV_DEFAULT,
   "f",
   "time",
   "Set the texture animation speed"
   );
Event EV_Client_SetGlobalFade
   (
   "globalfade",
   EV_DEFAULT,
   "S",
   "[in|out]",
   "Set the tempmodels to globally fade in or out together"
   );
Event EV_Client_SetParentLink
   (
   "parentlink",
   EV_DEFAULT,
   NULL,
   NULL,
   "Set the tempmodels linked to the parent, so they move with the parent model"
   );
Event EV_Client_SetHardLink
   (
   "hardlink",
   EV_DEFAULT,
   NULL,
   NULL,
   "Set the tempmodels linked to the model they are spawned from, so they move with it"
   );
Event EV_Client_SetRandomRoll
   (
   "randomroll",
   EV_DEFAULT,
   NULL,
   NULL,
   "Set the tempmodels so they pick a random roll value every frame"
   );
Event EV_Client_ParentAngles
   (
   "parentangles",
   EV_DEFAULT,
   NULL,
   NULL,
   "Set the tempmodels angles to that of its parent"
   );
Event EV_Client_SetAngles
   (
   "angles",
   EV_DEFAULT,
   "SFSFSF",
   "[random|crandom] pitch [random|crandom] yaw [random|crandom] roll",
   "If random is specified, the component will range from 0 to +specified value.\n"
   "If crandom is specified, the component will range from -specified to +specified value.\n"
   "If neither random or crandom is specified, then the component will be just set\n"
	"without randomness."
   );
Event EV_Client_Swipe
   (
   "swipe",
   EV_DEFAULT,
   "V",
   "origin",
   "Do a swipe and add it on to the swipe rendering list."
   );
Event EV_Client_SwipeOn
   (
   "swipeon",
   EV_DEFAULT,
   "ssff",
   "shader startTagName endTagNamelife life",
   "Signal the start of a swipe from the current tag"
   );
Event EV_Client_SwipeOff
   (
   "swipeoff",
   EV_DEFAULT,
   NULL,
   NULL,
   "Signal the end of a swipe"
   );
Event EV_Client_BeginTagBeamEmitter
   (
   "tagbeamemitter",
   EV_DEFAULT,
   "sss",
   "tagstart tagend name",
   "Create a beam emitter that uses 2 tags to determine it's start and end position"
   );
Event EV_Client_BeginTagBeamSpawn
   (
   "tagbeamspawn",
   EV_DEFAULT,
   "sf",
   "tagstart name",
   "Create a beam emitter that uses the tag to determine it's starting position."
   );
Event EV_Client_AnimateOnce
   (
   "animateonce",
   EV_DEFAULT,
   NULL,
   NULL,
   "Set a tempmodel to animate once and then get removed"
   );
Event EV_Client_SetAnim
   (
   "anim",
   EV_DEFAULT,
   "s",
   "animation",
   "Set a tempmodel the the specified animation"
   );
Event EV_Client_SetSubdivisions
   (
   "numsegments",
   EV_DEFAULT,
   "i",
   "numsegments",
   "Set the number of segments in a beam"
   );
Event EV_Client_SetMinOffset
   (
   "minoffset",
   EV_DEFAULT,
   "f",
   "minoffset",
   "Set the minimum offset from center in a beam"
   );
Event EV_Client_SetMaxOffset
   (
   "maxoffset",
   EV_DEFAULT,
   "f",
   "maxoffset",
   "Set the maximum offset from center in a beam"
   );
Event EV_Client_SetBeamShader
   (
   "beamshader",
   EV_DEFAULT,
   "s",
   "shadername",
   "Set the shader to use for the beam"
   );
Event EV_Client_SetBeamLength
   (
   "beamlength",
   EV_DEFAULT,
   "f",
   "length",
   "Set the length of the beam or trace length (for decals)"
   );
Event EV_Client_SetBeamDelay
   (
   "beamdelay",
   EV_DEFAULT,
   "SF",
   "[random] delay",
   "Set the delay time between creating new beams.\n"
   "If the keyword random is specified, the delay between beams will occur randomly between 0 and the time specified"
   );
Event EV_Client_SetBeamToggleDelay
   (
   "beamtoggledelay",
   EV_DEFAULT,
   "SF",
   "[random] delay",
   "Set a delay between toggling the beams on and off.\n"
   "If the keyword random is specified, the delay between toggling will occur randomly between 0 and the time specified"
   );
Event EV_Client_SetBeamPersist
   (
   "beampersist",
   EV_DEFAULT,
   NULL,
   NULL,
   "Make the beams persist instead of blinking out"
   );
Event EV_Client_SetBeamOffsetEndpoints
   (
   "beam_offset_endpoints",
   EV_DEFAULT,
   NULL,
   NULL,
   "Make the beams endpoints offset to reduce the bunching up effect"
   );
Event EV_Client_BeamSphere
   (
   "beamsphere",
   EV_DEFAULT,
   "i",
   "count",
   "Create a sphere shaped beam effect from the origin. Count is the number of beams"
   );
Event EV_Client_Spread
   (
   "spread",
   EV_DEFAULT,
   "ff",
   "spreadx spready",
   "Add a random variance in the spawned beam in the forward direction by the amound specified in spreadx and spready"
   );
Event EV_Client_UseLastTraceEnd
   (
   "uselasttraceend",
   EV_DEFAULT,
   NULL,
   NULL,
   "Makes this trace command use the end results of the last trace command"
   );
Event EV_Client_OffsetAlongAxis
   (
   "offsetalongaxis",
   EV_DEFAULT,
   "SfSfSf",
   "[crandom|random] offsetx [crandom|random] offsety [crandom|random] offsetz",
   "If random is specified, the component will range from 0 to specified offset.\n"
   "If crandom is specified, the component will range from -specified to +specified offset.\n"
   "If neither random or crandom is specified, then the component will just be added on\n"
	"without randomness.\n"
   "This offset is applied using the model's local axis"
   );
Event EV_Client_SetEndAlpha
   (
   "endalpha",
   EV_DEFAULT,
   "f",
   "alpha",
   "Set the alpha of the beam's endpoint"
   );
Event EV_Client_RandomChance
   (
   "randomchance",
   EV_DEFAULT,
   "fSSSSSS",
   "percentage [arg1] [arg2] [arg3] [arg4] [arg5] [arg6]",
   "Set the percentage chance that command will occur"
   );
Event EV_Client_CommandDelay
   (
   "commanddelay",
   EV_DEFAULT,
   "fiSSSSSS",
   "time commandnumber [arg1] [arg2] [arg3] [arg4] [arg5] [arg6]",
   "Set the time delay between this command getting executed.  This requires a command number to be assigned here.\n"
   "This is internally used to keep track of the commands this entity executes and will resolve naming conflicts."
   );
Event EV_Client_TagTraceImpactSound
   (
   "tagtraceimpactsound",
   EV_DEFAULT,
   "si",
   "tagname maxlength",
   "Perform a trace from the specified tag to the maxlength and play a sound at that position"
   );
Event EV_Client_TagTraceImpactSpawn
   (
   "tagtraceimpactspawn",
   EV_DEFAULT,
   "s",
   "tagname",
   "Perform a trace from the specified tag to the maxlength and spawn the specified model there."
   );
Event EV_Client_TagTraceImpactMark
   (
   "tagtraceimpactmark",
   EV_DEFAULT,
   "sI",
   "tagname temporary",
   "Perform a trace from the specified tag to the maxlength and put the shader as a decal on the surface it hits\n"
   "temporary = specify 1 for a temporary mark that only appears for a short time, 0 for a decal that stays aroung longer (default is 0)\n"
   );
Event EV_Client_BounceDecal
   (
   "bouncedecal",
   EV_DEFAULT,
   "iI",
   "maxamount temporary",
   "Put a mark when the tempmodel bounces and hits a surface\n"
   "maxamount = Max amount of decals to make when bouncing\n"
   "temporary = specify 1 for a temporary mark that only appears for a short time, 0 for a decal that stays aroung longer (default is 0)\n"
   );
Event EV_Client_SetDecalRadius
   (
   "decalradius",
   EV_DEFAULT,
   "f",
   "radius",
   "Set the radius of the decal"
   );
Event EV_Client_SetDecalOrientation
   (
   "orientation",
   EV_DEFAULT,
   "f",
   "degrees",
   "Set the degrees to orient the decal.  Specify 'random' to use a random orientation"
   );
Event EV_Client_SetDecalShader
   (
   "decalshader",
   EV_DEFAULT,
   "s",
   "shadername",
   "Set the shader to use for the impact decal"
   );
Event EV_Client_SetTraceLength
   (
   "tracelength",
   EV_DEFAULT,
   "f",
   "length",
   "Set the length of the trace for the decal"
   );
Event EV_Client_TraceSpawn
   (
   "tracespawn",
   EV_DEFAULT,
   "s",
   "modelname",
   "Spawn the specified model when the trace hits a solid object"
   );
Event EV_Client_TagList
   (
   "taglist",
   EV_DEFAULT,
   "ssSSSSSS",
   "arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8",
   "Set the tag list to create a beam that travels between all the tags"
   );
Event EV_Client_Lightstyle
   (
   "lightstyle",
   EV_DEFAULT,
   "s",
   "nameOfImage",
   "Set a lightstyle to determine the color of this tempmodel, the image\n"
   "specified is used to determine the look of the light style"
   );
Event EV_Client_DuplicateCount
   (
   "duplicatecount",
   EV_DEFAULT,
   "i",
   "num",
   "Set a duplication number for the tempmodels"
   );
Event EV_Client_PhysicsRate
   (
   "physicsrate",
   EV_DEFAULT,
   "s",
   "rate",
   "Set the rate (in updates per second) for the tempmodel's physics to be updated"
   );
Event EV_Client_Parallel
   (
   "parallel",
   EV_DEFAULT,
   NULL,
   NULL,
   "Set tempmodel to be parallel to the viewer"
   );
Event EV_Client_Detail
   (
   "detail",
   EV_DEFAULT,
   NULL,
   NULL,
   "Set emitter/tempmodel to be detail. Which can be turned off by detail"
   );
Event EV_Client_Print
	(
	"print",
	EV_DEFAULT,
	"s",
	"string",
	"Prints a string."
	);

CLASS_DECLARATION( Listener, ClientGameCommandManager, NULL )
   {
      { &EV_Client_StartBlock,                              &ClientGameCommandManager::StartBlock },
      { &EV_Client_EndBlock,                                &ClientGameCommandManager::EndBlock },
      { &EV_Client_EmitterStartOff,                         &ClientGameCommandManager::EmitterStartOff },
      { &EV_Client_OriginSpawn,                             &ClientGameCommandManager::BeginOriginSpawn },
      { &EV_Client_OriginBeamSpawn,                         &ClientGameCommandManager::BeginOriginBeamSpawn },
      { &EV_Client_OriginBeamEmitter,                       &ClientGameCommandManager::BeginOriginBeamEmitter },
      { &EV_Client_TagSpawn,                                &ClientGameCommandManager::BeginTagSpawn },
      { &EV_Client_TagEmitter,                              &ClientGameCommandManager::BeginTagEmitter },
      { &EV_Client_BeginTagBeamEmitter,                     &ClientGameCommandManager::BeginTagBeamEmitter },
      { &EV_Client_BeginTagBeamSpawn,                       &ClientGameCommandManager::BeginTagBeamSpawn },
      { &EV_Client_OriginEmitter,                           &ClientGameCommandManager::BeginOriginEmitter },
      { &EV_Client_EmitterOn,                               &ClientGameCommandManager::EmitterOn },
      { &EV_Client_EmitterOff,                              &ClientGameCommandManager::EmitterOff },
      { &EV_Client_SetAlpha,                                &ClientGameCommandManager::SetAlpha },
      { &EV_Client_SetDieTouch,                             &ClientGameCommandManager::SetDieTouch },
      { &EV_Client_SetBounceFactor,                         &ClientGameCommandManager::SetBounceFactor },
      { &EV_Client_SetBounceSound,                          &ClientGameCommandManager::SetBounceSound },
      { &EV_Client_SetBounceSoundOnce,                      &ClientGameCommandManager::SetBounceSoundOnce },
      { &EV_Client_SetScale,                                &ClientGameCommandManager::SetScale },
      { &EV_Client_SetScaleUpDown,                          &ClientGameCommandManager::SetScaleUpDown },
      { &EV_Client_SetScaleMin,                             &ClientGameCommandManager::SetScaleMin },
      { &EV_Client_SetScaleMax,                             &ClientGameCommandManager::SetScaleMax },
      { &EV_Client_SetModel,                                &ClientGameCommandManager::SetModel },
      { &EV_Client_SetLife,                                 &ClientGameCommandManager::SetLife },
      { &EV_Client_SetColor,                                &ClientGameCommandManager::SetColor },
      { &EV_Client_SetVelocity,                             &ClientGameCommandManager::SetVelocity },
      { &EV_Client_SetAngularVelocity,                      &ClientGameCommandManager::SetAngularVelocity },
      { &EV_Client_SetRandomVelocity,                       &ClientGameCommandManager::SetRandomVelocity },
      { &EV_Client_SetRandomVelocityAlongAxis,              &ClientGameCommandManager::SetRandomVelocityAlongAxis },
      { &EV_Client_SetAccel,                                &ClientGameCommandManager::SetAccel },
      { &EV_Client_SetCount,                                &ClientGameCommandManager::SetCount },
      { &EV_Client_SetFade,                                 &ClientGameCommandManager::SetFade },
      { &EV_Client_SetFadeDelay,                            &ClientGameCommandManager::SetFadeDelay },
      { &EV_Client_SetFadeIn,                               &ClientGameCommandManager::SetFadeIn },
      { &EV_Client_SetTwinkle,                              &ClientGameCommandManager::SetTwinkle },
      { &EV_Client_SetTrail,                                &ClientGameCommandManager::SetTrail },
      { &EV_Client_SetSpawnRate,                            &ClientGameCommandManager::SetSpawnRate },
      { &EV_Client_SetScaleRate,                            &ClientGameCommandManager::SetScaleRate },
      { &EV_Client_SetOriginOffset,                         &ClientGameCommandManager::SetOriginOffset },
      { &EV_Client_SetSphere,                               &ClientGameCommandManager::SetSphere },
      { &EV_Client_SetCircle,                               &ClientGameCommandManager::SetCircle },
      { &EV_Client_SetInwardSphere,                         &ClientGameCommandManager::SetInwardSphere },
      { &EV_Client_SetRadius,                               &ClientGameCommandManager::SetRadius },
      { &EV_Client_SetSwarm,                                &ClientGameCommandManager::SetSwarm },
      { &EV_Client_SetAlign,                                &ClientGameCommandManager::SetAlign },
      { &EV_Client_SetAlignOnce,                            &ClientGameCommandManager::SetAlignOnce },
      { &EV_Client_SetCollision,                            &ClientGameCommandManager::SetCollision },
      { &EV_Client_SetFlickerAlpha,                         &ClientGameCommandManager::SetFlickerAlpha },
		{ &EV_Client_Sound,				                    &ClientGameCommandManager::Sound },
		{ &EV_Client_StopSound,								&ClientGameCommandManager::StopSound },
		{ &EV_Client_LoopSound,				                &ClientGameCommandManager::LoopSound },
		{ &EV_Client_Cache,				                    &ClientGameCommandManager::Cache },
		{ &EV_Client_AliasCache,		                    &ClientGameCommandManager::AliasCache },
		{ &EV_Client_Alias,			                        &ClientGameCommandManager::Alias },
		{ &EV_Client_Footstep,  	                        &ClientGameCommandManager::Footstep },
		{ &EV_Client_Client,			                    &ClientGameCommandManager::Client },
      { &EV_Client_TagDynamicLight,                         &ClientGameCommandManager::TagDynamicLight },
      { &EV_Client_OriginDynamicLight,                      &ClientGameCommandManager::OriginDynamicLight },
      { &EV_Client_DynamicLight,                            &ClientGameCommandManager::DynamicLight },
      { &EV_Client_SetEntityColor,                          &ClientGameCommandManager::SetEntityColor },
      { &EV_Client_SetGlobalFade,                           &ClientGameCommandManager::SetGlobalFade },
      { &EV_Client_SetParentLink,                           &ClientGameCommandManager::SetParentLink },
      { &EV_Client_SetHardLink,                             &ClientGameCommandManager::SetHardLink },
      { &EV_Client_SetRandomRoll,                           &ClientGameCommandManager::SetRandomRoll },
      { &EV_Client_SetAngles,                               &ClientGameCommandManager::SetAngles },
      { &EV_Client_ParentAngles,                            &ClientGameCommandManager::ParentAngles },
      { &EV_Client_Swipe,                                   &ClientGameCommandManager::Swipe },
      { &EV_Client_SwipeOn,                                 &ClientGameCommandManager::SwipeOn },
      { &EV_Client_SwipeOff,                                &ClientGameCommandManager::SwipeOff },
      { &EV_Client_SetAnim,                                 &ClientGameCommandManager::SetAnim },
      { &EV_Client_AnimateOnce,                             &ClientGameCommandManager::AnimateOnce },
      { &EV_Client_SetSubdivisions,                         &ClientGameCommandManager::SetSubdivisions },
      { &EV_Client_SetMinOffset,                            &ClientGameCommandManager::SetMinOffset },
      { &EV_Client_SetMaxOffset,                            &ClientGameCommandManager::SetMaxOffset },
      { &EV_Client_SetBeamShader,                           &ClientGameCommandManager::SetShader },
      { &EV_Client_SetBeamLength,                           &ClientGameCommandManager::SetLength },
      { &EV_Client_SetDecalShader,                          &ClientGameCommandManager::SetShader },
      { &EV_Client_SetTraceLength,                          &ClientGameCommandManager::SetLength },
      { &EV_Client_SetBeamDelay,                            &ClientGameCommandManager::SetBeamDelay },
      { &EV_Client_SetBeamToggleDelay,                      &ClientGameCommandManager::SetBeamToggleDelay },
      { &EV_Client_SetBeamPersist,                          &ClientGameCommandManager::SetBeamPersist },
      { &EV_Client_SetBeamOffsetEndpoints,                  &ClientGameCommandManager::SetBeamOffsetEndpoints },
      { &EV_Client_BeamSphere,                              &ClientGameCommandManager::SetBeamSphere },
      { &EV_Client_Spread,                                  &ClientGameCommandManager::SetSpread },
		{ &EV_Client_UseLastTraceEnd,                       &ClientGameCommandManager::SetUseLastTraceEnd },
      { &EV_Client_OffsetAlongAxis,                         &ClientGameCommandManager::SetOffsetAlongAxis },
      { &EV_Client_SetEndAlpha,                             &ClientGameCommandManager::SetEndAlpha },
      { &EV_Client_RandomChance,                            &ClientGameCommandManager::RandomChance },
      { &EV_Client_CommandDelay,                            &ClientGameCommandManager::CommandDelay },
      { &EV_Client_BounceDecal,                             &ClientGameCommandManager::SetBounceDecal },
      { &EV_Client_SetDecalRadius,                          &ClientGameCommandManager::SetDecalRadius },
      { &EV_Client_SetDecalOrientation,                     &ClientGameCommandManager::SetDecalOrientation },
      { &EV_Client_TagList,                                 &ClientGameCommandManager::TagList },
      { &EV_Client_Lightstyle,                              &ClientGameCommandManager::SetLightstyle },
      { &EV_Client_PhysicsRate,                             &ClientGameCommandManager::SetPhysicsRate },
      { &EV_Client_Parallel,                                &ClientGameCommandManager::SetParallel },
      { &EV_Client_Detail,                                  &ClientGameCommandManager::SetDetail },
      { &EV_Client_Print,                                   &ClientGameCommandManager::Print },
      { NULL, NULL }
   };

static int DLightNameToNum(str s)

{
    if (!s.icmp("normal")) {
        return 0;
    } else if (!s.icmp("lensflare")) {
        return lensflare;
    } else if (!s.icmp("viewlensflare")) {
        return viewlensflare;
    } else if (!s.icmp("additive")) {
        return additive;
    } else {
        return 0;
    }
}

float RandomizeRange(float start, float end)

{
    float t;

    if (start > end) {
        return start;
    }

    t = start + ((end - start) * random());
    return t;
}

void ClientGameCommandManager::ProcessPendingEventsForEntity()
{
    // FIXME: unimplemented
}

qboolean ClientGameCommandManager::PostEventForEntity(Event* ev, float fWait)
{
    // FIXME: unimplemented
    return qfalse;
}

void ClientGameCommandManager::SetBaseAndAmplitude(Event* ev, Vector& base, Vector& amplitude)
{
    int i = 1;
    int j = 0;

    while (j < 3) {
        str org;

        org = ev->GetString(i++);
        if (org == "crandom") {
            float value = ev->GetFloat(i++);
            base[j] = -value;
            amplitude[j] = value + value;
        } else if (org == "random") {
            base[j] = 0.0;
            amplitude[j] = ev->GetFloat(i++);
        } else if (org == "range") {
            base[j] = ev->GetFloat(i++);
            amplitude[j] = ev->GetFloat(i++);
        }
    }
}

//=============
// ClientGameCommandManager
//=============
ClientGameCommandManager::ClientGameCommandManager()

{
    m_seed = 0;

    InitializeTempModels();
    InitializeEmitters();
}

void ClientGameCommandManager::Print(Event* ev)

{
    if (current_entity) {
        cgi.DPrintf("%d:%s\n", current_entity->entityNumber, ev->GetString(1));
    }
}

//===============
// CommandDelay
//===============
void ClientGameCommandManager::CommandDelay(Event* ev)

{
    int i;
    int delay;
    Event* ev1;

    delay = ev->GetFloat(1) * 1000;
    if (current_entity) {
        commandtime_t* ct = m_command_time_manager.GetLastCommandTime(
            current_entity->entityNumber, ev->GetInteger(2));

        if (!ct->last_command_time) {
            ct->last_command_time = cg.time + delay;
            return;
        }

        if (ct->last_command_time > cg.time) {
            return;
        } else {
            ct->last_command_time = cg.time + delay;
        }
    } else {
        warning("CCM:CommandDelay",
                "Cannot perform command delay on spawned tempmodels");
    }

    ev1 = new Event(ev->GetString(3));

    for (i = 4; i <= ev->NumArgs(); i++) {
        ev1->AddToken(ev->GetToken(i));
    }
    ProcessEvent(ev1);
}

void ClientGameCommandManager::SetCurrentSFX(Event* ev)
{
    // FIXME: unimplemented
}

void ClientGameCommandManager::ClearCurrentSFX()
{
    // FIXME: unimplemented
}

//===============
// RandomChance
//===============
void ClientGameCommandManager::RandomChance(Event* ev)

{
    int i;
    float percentage = ev->GetFloat(1);

    if ((random() < percentage) && (ev->NumArgs() >= 2)) {
        Event* ev1;

        ev1 = new Event(ev->GetString(2));

        for (i = 3; i <= ev->NumArgs(); i++) {
            ev1->AddToken(ev->GetToken(i));
        }
        ProcessEvent(ev1);
    }
}
//===============
// SetDetail
//===============
void ClientGameCommandManager::SetDetail(Event* ev)

{
    m_spawnthing->cgd.flags |= T_DETAIL;
}

//===============
// SetDecalRadius
//===============
void ClientGameCommandManager::SetDecalRadius(Event* ev)

{
    m_spawnthing->cgd.decal_radius = ev->GetFloat(1);
}

//===============
// SetDecalOrientation
//===============
void ClientGameCommandManager::SetDecalOrientation(Event* ev)

{
    str deg;

    deg = ev->GetString(1);

    if (!deg.icmp("random")) {
        m_spawnthing->cgd.decal_orientation = random() * 360;
    } else {
        m_spawnthing->cgd.decal_orientation = ev->GetFloat(1);
    }
}

//===============
// SetBounceDecal
//===============
void ClientGameCommandManager::SetBounceDecal(Event* ev)
{
    m_spawnthing->cgd.flags2 |= T2_BOUNCE_DECAL;
    if (ev->NumArgs() > 1) {
        m_spawnthing->cgd.maxbouncecount = ev->GetInteger(1);

        if (ev->NumArgs() > 2) {
            qboolean b = ev->GetBoolean(2);
            if (b) {
                m_spawnthing->cgd.flags2 |= T2_TEMPORARY_DECAL;
            }
        }
    }
}

//===============
// SetParallel
//===============
void ClientGameCommandManager::SetParallel(Event* ev)

{
    m_spawnthing->cgd.flags2 |= T2_PARALLEL;
}

//===============
// PhysicsRate
//===============
void ClientGameCommandManager::SetPhysicsRate(Event* ev)

{
    str rate;

    rate = ev->GetString(1);

    if (!rate.icmp(rate, "every")) {
        m_spawnthing->cgd.flags2 |= T2_PHYSICS_EVERYFRAME;
    } else {
        m_spawnthing->cgd.physicsRate = atof(rate);
    }
}

//===============
// SetLightstyle
//===============
void ClientGameCommandManager::SetLightstyle(Event* ev)

{
    m_spawnthing->cgd.lightstyle = CG_RegisterLightStyle(ev->GetString(1));
}

//===============
// SetBeamSphere
//===============
void ClientGameCommandManager::SetBeamSphere(Event* ev)

{
    m_spawnthing->numspherebeams = ev->GetInteger(1);
    m_spawnthing->beamflags = BEAM_SPHERE_EFFECT;
}

//===============
// SetBeamSpread
//===============
void ClientGameCommandManager::SetSpread(Event* ev)

{
    m_spawnthing->spreadx = ev->GetFloat(1);
    m_spawnthing->spready = ev->GetFloat(2);
}

//===============
// SetUseLastTraceEnd
//===============
void ClientGameCommandManager::SetUseLastTraceEnd(Event* ev)

{
    m_spawnthing->use_last_trace_end = qtrue;
}

//===============
// SetSubdivisions
//===============
void ClientGameCommandManager::SetSubdivisions(Event* ev)

{
    m_spawnthing->numSubdivisions = ev->GetInteger(1);
}

//===============
// SetBeamPersist
//===============
void ClientGameCommandManager::SetBeamPersist(Event* ev)

{
    m_spawnthing->beamflags |= BEAM_PERSIST_EFFECT;
}

//===============
// SetBeamOffsetEndponts
//===============
void ClientGameCommandManager::SetBeamOffsetEndpoints(Event* ev)

{
    m_spawnthing->beamflags |= BEAM_OFFSET_ENDPOINTS;
}

//===============
// SetBeamDelay
//===============
void ClientGameCommandManager::SetBeamDelay(Event* ev)

{
    if (ev->NumArgs() > 1) {
        str r = ev->GetString(1);
        if (r.icmp("random")) {
            m_spawnthing->beamflags |= BEAM_RANDOM_DELAY;
        }
        m_spawnthing->delay = ev->GetFloat(2) * 1000.0f;
    } else {
        m_spawnthing->delay = ev->GetFloat(1) * 1000.0f;
    }
}

//===============
// SetBeamToggleDelay
//===============
void ClientGameCommandManager::SetBeamToggleDelay(Event* ev)

{
    m_spawnthing->beamflags |= BEAM_TOGGLE;
    if (ev->NumArgs() > 1) {
        str r = ev->GetString(1);
        if (!r.icmp("random")) {
            m_spawnthing->beamflags |= BEAM_RANDOM_TOGGLEDELAY;
        }

        m_spawnthing->toggledelay = ev->GetFloat(2) * 1000.0f;
    } else {
        m_spawnthing->toggledelay = ev->GetFloat(1) * 1000.0f;
    }
}

//===============
// SetBeamLength
//===============
void ClientGameCommandManager::SetLength(Event* ev)

{
    m_spawnthing->length = ev->GetFloat(1);
}

//===============
// SetShader
//===============
void ClientGameCommandManager::SetShader(Event* ev)

{
    m_spawnthing->cgd.shadername = ev->GetString(1);
}

//===============
// SetMinOffset
//===============
void ClientGameCommandManager::SetMinOffset(Event* ev)

{
    m_spawnthing->min_offset = ev->GetInteger(1);
}

//===============
// SetSubdivisions
//===============
void ClientGameCommandManager::SetMaxOffset(Event* ev)

{
    m_spawnthing->max_offset = ev->GetInteger(1);
}

//===============
// SetAnim
//===============
void ClientGameCommandManager::SetAnim(Event* ev)

{
    m_spawnthing->animName = ev->GetString(1);
}

//===============
// AnimateOnce
//===============
void ClientGameCommandManager::AnimateOnce(Event* ev)

{
    m_spawnthing->cgd.flags |= T_ANIMATEONCE;
}

//===============
// EmitterStartOff
//===============
void ClientGameCommandManager::EmitterStartOff(Event* ev)

{
    m_spawnthing->startoff = qtrue;
}

//===============
// StartBlock
//===============
void ClientGameCommandManager::StartBlock(Event* ev)

{
    // Make sure there are no more args on this line, because
    // they will be lost
    if (ev->NumArgs() > 1) {
        cgi.DPrintf("CCM::StartBlock : Invalid commands on start block '{'\n");
    }
}

//===============
// EndBlock
//===============
void ClientGameCommandManager::EndBlock(Event* ev)

{
    // Make sure there are no more args on this line, because
    // they will be lost

    if (ev->NumArgs() > 1) {
        cgi.DPrintf("CCM::StartBlock : Invalid commands on start block '{'\n");
    }

    if (endblockfcn) {
        (this->*endblockfcn)();
    }

    endblockfcn = NULL;
}

//=============
// SetSwarm
//=============
void ClientGameCommandManager::SetSwarm(Event* ev)

{
    m_spawnthing->cgd.swarmfreq = ev->GetInteger(1);
    m_spawnthing->cgd.swarmmaxspeed = ev->GetFloat(2);
    m_spawnthing->cgd.swarmdelta = ev->GetFloat(3);
    m_spawnthing->cgd.flags |= T_SWARM;
}

//=============
// SetCircle
//=============
void ClientGameCommandManager::SetCircle(Event* ev)

{
    m_spawnthing->cgd.flags |= T_CIRCLE;
}

//=============
// SetSphere
//=============
void ClientGameCommandManager::SetSphere(Event* ev)

{
    m_spawnthing->cgd.flags |= T_SPHERE;
}

//=============
// SetSphere
//=============
void ClientGameCommandManager::SetRadius(Event* ev)

{
    m_spawnthing->sphereRadius = ev->GetFloat(1);
}

//=============
// SetInwardSphere
//=============
void ClientGameCommandManager::SetInwardSphere(Event* ev)

{
    m_spawnthing->cgd.flags |= T_INWARDSPHERE;
    m_spawnthing->sphereRadius = ev->GetFloat(1);
}

//=============
// SetAlign
//=============
void ClientGameCommandManager::SetAlign(Event* ev)

{
    m_spawnthing->cgd.flags |= T_ALIGN;
}

//=============
// SetAlignOnce
//=============
void ClientGameCommandManager::SetAlignOnce(Event* ev)

{
    m_spawnthing->cgd.flags |= T_ALIGNONCE;
}

//=============
// SetRandomRoll
//=============
void ClientGameCommandManager::SetRandomRoll(Event* ev)

{
    m_spawnthing->cgd.flags |= T_RANDOMROLL;
}

//=============
// SetCollision
//=============
void ClientGameCommandManager::SetCollision(Event* ev)

{
    str mask;

    m_spawnthing->cgd.flags |= T_COLLISION;
    m_spawnthing->cgd.collisionmask = (CONTENTS_SOLID);

    if (ev->NumArgs() > 1) {
        mask = ev->GetString(1);

        if (mask == "water") {
            m_spawnthing->cgd.collisionmask = (CONTENTS_SOLID | CONTENTS_WATER);
        }
    }
}

//=============
// SetFlickerAlpha
//=============
void ClientGameCommandManager::SetFlickerAlpha(Event* ev)

{
    m_spawnthing->cgd.flags |= T_FLICKERALPHA;
}

//=============
// SetEndAlpha
//=============
void ClientGameCommandManager::SetEndAlpha(Event* ev)
{
    m_spawnthing->endalpha = ev->GetFloat(1);
}

//=============
// SetOffsetAlongAxis
//=============
void ClientGameCommandManager::SetOffsetAlongAxis(Event* ev)
{
    if (m_spawnthing) {
        SetBaseAndAmplitude(ev, m_spawnthing->axis_offset_base, m_spawnthing->axis_offset_amplitude);
    }
}

//=============
// SetOriginOffset
//=============
void ClientGameCommandManager::SetOriginOffset(Event* ev)
{
    if (m_spawnthing) {
        SetBaseAndAmplitude(ev, m_spawnthing->origin_offset_base,
                            m_spawnthing->origin_offset_amplitude);
    }
}

//=============
// SetAlpha
//=============
void ClientGameCommandManager::SetAlpha(Event* ev)

{
    m_spawnthing->cgd.alpha = ev->GetFloat(1);
}

//=============
// SetDieTouch
//=============
void ClientGameCommandManager::SetDieTouch(Event* ev)

{
    m_spawnthing->cgd.flags |= T_DIETOUCH;
}

//=============
// SetBounceFactor
//=============
void ClientGameCommandManager::SetBounceFactor(Event* ev)

{
    m_spawnthing->cgd.bouncefactor = ev->GetFloat(1);
    m_spawnthing->cgd.flags |= T_COLLISION;
    m_spawnthing->cgd.collisionmask = (CONTENTS_SOLID);
}

//=============
// SetBounceSound
//=============
void ClientGameCommandManager::SetBounceSound(Event* ev)

{
    m_spawnthing->cgd.bouncesound = ev->GetString(1);

    if (ev->NumArgs() == 2) {
        m_spawnthing->cgd.bouncesound_delay = ev->GetFloat(1) * 1000;
    }

    m_spawnthing->cgd.flags |= T_BOUNCESOUND;
}

//=============
// SetBounceSoundOnce
//=============
void ClientGameCommandManager::SetBounceSoundOnce(Event* ev)

{
    m_spawnthing->cgd.flags |= T_BOUNCESOUNDONCE;
    SetBounceSound(ev);
}

//=============
// SetScale
//=============
void ClientGameCommandManager::SetScale(Event* ev)

{
    m_spawnthing->cgd.scale = ev->GetFloat(1);
}

//=============
// SetScaleUpDown
//=============
void ClientGameCommandManager::SetScaleUpDown(Event* ev)

{
    m_spawnthing->cgd.flags |= T_SCALEUPDOWN;
}

//=============
// SetScaleMin
//=============
void ClientGameCommandManager::SetScaleMin(Event* ev)

{
    m_spawnthing->cgd.flags |= T_RANDSCALE;
    m_spawnthing->cgd.scalemin = ev->GetFloat(1);
}

//=============
// SetScaleMax
//=============
void ClientGameCommandManager::SetScaleMax(Event* ev)

{
    m_spawnthing->cgd.flags |= T_RANDSCALE;
    m_spawnthing->cgd.scalemax = ev->GetFloat(1);
}

//=============
// SetScaleRate
//=============
void ClientGameCommandManager::SetScaleRate(Event* ev)

{
    m_spawnthing->cgd.scaleRate = ev->GetFloat(1);
}

//=============
// SetFade
//=============
void ClientGameCommandManager::SetFade(Event* ev)

{
    m_spawnthing->cgd.flags |= T_FADE;
}

//=============
// SetFadeDelay
//=============
void ClientGameCommandManager::SetFadeDelay(Event* ev)

{
    m_spawnthing->cgd.fadedelay = ev->GetFloat(1) * 1000;
    m_spawnthing->cgd.flags |= T_FADE;
}

//=============
// SetFadeIn
//=============
void ClientGameCommandManager::SetFadeIn(Event* ev)

{
    m_spawnthing->cgd.flags |= T_FADEIN;

    if (ev->NumArgs() > 0) {
        m_spawnthing->cgd.fadeintime = ev->GetFloat(1) * 1000;
    }
}

//=============
// SetTwinkle
//=============
void ClientGameCommandManager::SetTwinkle(Event* ev)

{
    m_spawnthing->cgd.flags |= T_TWINKLE;
    m_spawnthing->cgd.min_twinkletimeoff = ev->GetFloat(1) * 1000;
    m_spawnthing->cgd.max_twinkletimeoff = ev->GetFloat(2) * 1000;
    m_spawnthing->cgd.min_twinkletimeon = ev->GetFloat(3) * 1000;
    m_spawnthing->cgd.max_twinkletimeon = ev->GetFloat(4) * 1000;
}

//=============
// SetSwipe
//=============
void ClientGameCommandManager::SetTrail(Event* ev)

{
    m_spawnthing->cgd.flags2 |= T2_TRAIL;
    m_spawnthing->cgd.swipe_shader = ev->GetString(1);
    m_spawnthing->cgd.swipe_tag_start = ev->GetString(2);
    m_spawnthing->cgd.swipe_tag_end = ev->GetString(3);
    m_spawnthing->cgd.swipe_life = ev->GetFloat(4);
}

//=============
// SetSpawnRate
//=============
void ClientGameCommandManager::SetSpawnRate(Event* ev)

{
    m_spawnthing->spawnRate = (1.0f / ev->GetFloat(1)) * 1000;
}

//=============
// SetParentLink
//=============
void ClientGameCommandManager::SetParentLink(Event* ev)

{
    m_spawnthing->cgd.flags |= T_PARENTLINK;
}

//=============
// SetHardLink
//=============
void ClientGameCommandManager::SetHardLink(Event* ev)

{
    m_spawnthing->cgd.flags |= T_HARDLINK;
    m_spawnthing->cgd.parent = current_entity_number;

    if (current_entity) {
        m_spawnthing->cgd.origin =
            m_spawnthing->cgd.origin - current_entity->origin;
    }
}

//=============
// SetColor
//=============
void ClientGameCommandManager::SetColor(Event* ev)

{
    m_spawnthing->cgd.color[0] = ev->GetFloat(1) * 0xff;
    m_spawnthing->cgd.color[1] = ev->GetFloat(2) * 0xff;
    m_spawnthing->cgd.color[2] = ev->GetFloat(3) * 0xff;

    if (ev->NumArgs() == 4) {
        m_spawnthing->cgd.color[3] = ev->GetFloat(4) * 0xff;
        m_spawnthing->cgd.alpha = ev->GetFloat(4);
    }
}

//=============
// SetEntityColor
//=============
void ClientGameCommandManager::SetEntityColor(Event* ev)

{
    if (current_centity) {
        current_centity->client_color[0] = ev->GetFloat(1);
        current_centity->client_color[1] = ev->GetFloat(2);
        current_centity->client_color[2] = ev->GetFloat(3);
        if (ev->NumArgs() == 4) {
            current_centity->client_color[3] = ev->GetFloat(4);
        }
    }
}

//=============
// SetGlobalFade
//=============
void ClientGameCommandManager::SetGlobalFade(Event* ev)

{
    str mode;

    mode = ev->GetString(1);

    if (mode == "in") {
        m_spawnthing->cgd.flags |= T_GLOBALFADEIN;
    } else if (mode == "out") {
        m_spawnthing->cgd.flags |= T_GLOBALFADEOUT;
    } else {
        cgi.DPrintf("Illegal globalfade parm: %s\n", mode.c_str());
    }
}

//=============
// SetRandomVelocity
//=============
void ClientGameCommandManager::SetRandomVelocity(Event* ev)
{
    int i = 1;
    int j = 0;
    Vector randval;
    str vel;

    if (!m_spawnthing) {
        return;
    }

    if (ev->NumArgs() < 3) {
        warning("ClientGameCommandManager::SetRandomVelocity",
                "Expecting at least 3 args for command randvel");
    }

    SetBaseAndAmplitude(ev, m_spawnthing->randvel_base,
                        m_spawnthing->randvel_amplitude);

    m_spawnthing->cgd.flags2 |= T2_MOVE;
}

//=============
// SetRandomVelocityAlongAxis
//=============
void ClientGameCommandManager::SetRandomVelocityAlongAxis(Event* ev)
{
    m_spawnthing->cgd.flags |= T_RANDVELAXIS;
    SetRandomVelocity(ev);
}

//=============
// SetVelocity
//=============
void ClientGameCommandManager::SetVelocity(Event* ev)

{
    m_spawnthing->forwardVelocity = ev->GetFloat(1);
    m_spawnthing->cgd.flags2 |= T2_MOVE;
}

//=============
// SetAngularVelocity
//=============
void ClientGameCommandManager::SetAngularVelocity(Event* ev)

{
    int i = 1;
    int j = 0;
    Vector randval;
    str vel;

    if (!m_spawnthing) {
        return;
    }

    if (ev->NumArgs() < 3) {
        warning("ClientGameCommandManager::SetAngularVelocity",
                "Expecting at least 3 args for command randvel");
    }

    SetBaseAndAmplitude(ev, m_spawnthing->avelocity_base,
                        m_spawnthing->avelocity_amplitude);

    m_spawnthing->cgd.flags2 |= T2_AMOVE;
}

//=============
// SetAngles
//=============
void ClientGameCommandManager::SetAngles(Event* ev)
{
    int i = 1;
    int j = 0;
    Vector randval;
    str vel;

    if (!m_spawnthing) {
        return;
    }

    if (ev->NumArgs() < 3) {
        warning("ClientGameCommandManager::SetAngles",
                "Expecting at least 3 args for command randvel");
    }

    SetBaseAndAmplitude(ev, m_spawnthing->cgd.angles,
                        m_spawnthing->angles_amplitude);

    // Set the tag axis
    m_spawnthing->cgd.flags |= T_ANGLES;
}

//=============
// ParentAngles
//=============
void ClientGameCommandManager::ParentAngles(Event* ev)

{
    if (current_centity) {
        m_spawnthing->cgd.angles = Vector(current_centity->currentState.angles);
    }
    m_spawnthing->cgd.flags |= T_ANGLES;
}

//=============
// SetAccel
//=============
void ClientGameCommandManager::SetAccel(Event* ev)

{
    m_spawnthing->cgd.accel[0] = ev->GetFloat(1);
    m_spawnthing->cgd.accel[1] = ev->GetFloat(2);
    m_spawnthing->cgd.accel[2] = ev->GetFloat(3);

    m_spawnthing->cgd.flags2 |= T2_ACCEL;
}

//=============
// SetCount
//=============
void ClientGameCommandManager::SetCount(Event* ev)

{
    m_spawnthing->count = ev->GetInteger(1);
}

//=============
// SetLife
//=============
void ClientGameCommandManager::SetLife(Event* ev)

{
    str life;

    life = ev->GetString(1);

    if (!life.icmp("autocalc")) {
        m_spawnthing->cgd.flags |= T_AUTOCALCLIFE;
    } else {
        m_spawnthing->cgd.life = atof(life) * 1000;
    }
}

//=============
// SetModel
//=============
void ClientGameCommandManager::SetModel(Event* ev)

{
    int i;
    int num = ev->NumArgs();

    for (i = 1; i <= num; i++) {
        str s_arg(ev->GetString(i));
        m_spawnthing->m_modellist.AddObject(s_arg);
        CacheResource(ev->GetString(i));
    }
}

//=============
// TagList
//=============
void ClientGameCommandManager::TagList(Event* ev)

{
    int i;
    int num = ev->NumArgs();

    if (num < 2) {
        warning("CCG::TagList", "Invalid number of tags specified in taglist.  "
                                "Minimum set of tags is 2.\n");
        return;
    }

    for (i = 1; i <= num; i++) {
        str s_arg(ev->GetString(i));
        m_spawnthing->m_taglist.AddObject(s_arg);
    }
}

//=============
// InitializeSpawnthing
//=============
spawnthing_t* ClientGameCommandManager::InitializeSpawnthing(spawnthing_t* sp)

{
    int i;

    // Initalize m_spawnthing - these can be overidden with other commands
    sp->m_modellist.ClearObjectList();
    AxisClear(sp->axis);
    AxisClear(sp->tag_axis);

    sp->startoff = qfalse;
    sp->origin_offset_base = Vector(0, 0, 0);
    sp->origin_offset_amplitude = Vector(0, 0, 0);
    sp->axis_offset_base = Vector(0, 0, 0);
    sp->axis_offset_amplitude = Vector(0, 0, 0);
    sp->randvel_base = Vector(0, 0, 0);
    sp->randvel_amplitude = Vector(0, 0, 0);
    sp->avelocity_base = Vector(0, 0, 0);
    sp->avelocity_amplitude = Vector(0, 0, 0);
    sp->angles_amplitude = Vector(0, 0, 0);
    sp->life_random = 0;
    sp->forwardVelocity = 0;
    sp->sphereRadius = 0;
    sp->coneHeight = 0;
    sp->spawnRate = 0.1f;
    sp->count = 1;
    sp->length = 1000;
    sp->max_offset = 0;
    sp->min_offset = 0;
    sp->overlap = 0;
    sp->numSubdivisions = 1;
    sp->delay = 0;
    sp->toggledelay = 0;
    sp->endalpha = 1.0f;
    sp->spreadx = 0;
    sp->spready = 0;
    sp->beamflags = 0;
    sp->startoff = 0;
    sp->numspherebeams = 0;
    sp->use_last_trace_end = qfalse;
    sp->tagnum = -1;
    sp->numtempmodels = 0;
    sp->entnum = ENTITYNUM_NONE;

    sp->cgd.scale = 1.0f;
    sp->cgd.createTime = cg.time;
    sp->cgd.life = EMITTER_DEFAULT_LIFE;
    sp->cgd.avelocity = Vector(0, 0, 0);
    sp->cgd.accel = Vector(0, 0, 0);
    sp->cgd.origin = Vector(0, 0, 0);
    sp->cgd.oldorigin = Vector(0, 0, 0);
    sp->cgd.angles = Vector(0, 0, 0);
    sp->cgd.alpha = 1.0f;
    sp->cgd.scalemin = 0;
    sp->cgd.scalemax = 999999.0f;
    sp->cgd.scaleRate = 0;
    sp->cgd.bouncefactor = 0.3f;
    sp->cgd.bouncesound = "";
    sp->cgd.bouncesound_delay = 1000;
    sp->cgd.bouncecount = 0;
    sp->cgd.maxbouncecount = 3;
    sp->cgd.flags = 0;
    sp->cgd.flags2 = 0;
    sp->cgd.swarmfreq = 0;
    sp->cgd.swarmdelta = 0;
    sp->cgd.swarmmaxspeed = 0;
    sp->cgd.fadeintime = 0;
    sp->cgd.fadedelay = 0;
    sp->cgd.lightIntensity = 0;
    sp->cgd.lightType = 0;
    sp->cgd.collisionmask = MASK_AUTOCALCLIFE;
    sp->cgd.parent = -1;
    sp->cgd.tiki = nullptr;
    sp->cgd.lightstyle = -1;
    sp->cgd.physicsRate = 10;
    sp->cgd.shadername = "beamshader";
    sp->cgd.decal_orientation = 0;
    sp->cgd.decal_radius = 10;
    sp->cgd.max_twinkletimeoff = 0;
    sp->cgd.min_twinkletimeoff = 0;
    sp->cgd.max_twinkletimeon = 0;
    sp->cgd.min_twinkletimeon = 0;
    sp->cgd.swipe_life = 0;
    sp->fMinRangeSquared = 0;
    sp->fMaxRangeSquared = 9.9999997e37f;

    for (i = 0; i < 3; i++) {
        sp->dcolor[i] = 1.0f;
        sp->cgd.color[i] = 1.0;
    }

    sp->cgd.color[3] = 1.0;

    return sp;
}

//==================
// GetEmitterByName
//==================
spawnthing_t* ClientGameCommandManager::GetEmitterByName(str name)

{
    int i;
    for (i = 1; i <= m_emitters.NumObjects(); i++) {
        spawnthing_t* st = m_emitters.ObjectAt(i);
        if (st->emittername == name) {
            return st;
        }
    }
    return NULL;
}

//==================
// CreateNewEmitter
//==================
spawnthing_t* ClientGameCommandManager::CreateNewEmitter(str name)

{
    spawnthing_t* st;

    st = new spawnthing_t;

    // Init the emitter and set the internal pointer at it
    InitializeSpawnthing(st);

    // Set the emitter's name
    st->emittername = name;

    // Store it in the container
    m_emitters.AddObject(st);

    return st;
}

void ClientGameCommandManager::DeleteEmitters(dtiki_t* tiki)
{
    // FIXME: unimpleme,nted
}

void CG_DeleteEmitters(dtiki_t* tiki)
{
    // FIXME: unimplemented
}

//==================
// CreateNewEmitter
//==================
spawnthing_t* ClientGameCommandManager::CreateNewEmitter(void)

{
    return CreateNewEmitter("");
}

//===============
// BeginOriginSpawn
//===============
void ClientGameCommandManager::BeginOriginSpawn(Event* ev)
{
    // FIXME: partially implemented

    if (!current_entity) {
        return;
    }

    // Setup ending function
    endblockfcn = &ClientGameCommandManager::EndOriginSpawn;

    // Init the thing we are going to spawn
    m_spawnthing = InitializeSpawnthing(&m_localemitter);

    // Set the origin based on the entity's origin
    m_spawnthing->cgd.origin = current_entity->origin;

    // Set the axis based on the entity's axis
    AxisCopy(current_entity->axis, m_spawnthing->axis);
    AxisCopy(current_entity->axis, m_spawnthing->tag_axis);
    MatrixToEulerAngles(m_spawnthing->axis, m_spawnthing->cgd.angles);
}

//===============
// EndOriginSpawn
//===============
void ClientGameCommandManager::EndOriginSpawn(void)

{
    // Okay we should have a valid spawnthing, let's create a render entity
    SpawnTempModel(m_spawnthing->count);
}

void ClientGameCommandManager::TestEffectEndFunc()
{
    // FIXME: unimplemented
}

//===============
// UpdateSpawnThing
//===============
void ClientGameCommandManager::UpdateSpawnThing(spawnthing_t* ep)
{
    int i;
    orientation_t orientation;

    VectorCopy(current_entity->origin, ep->cgd.origin);

    for (i = 0; i < 3; i++) {
        VectorMA(ep->cgd.origin, orientation.origin[i], current_entity->axis[i],
                 ep->cgd.origin);
    }

    MatrixMultiply(orientation.axis, current_entity->axis, ep->axis);
}

//===============
// BeginTagEmitter
//===============
void ClientGameCommandManager::BeginTagEmitter(Event* ev)
{
    str tagname;
    int tagnum;

    // Get the tagname and orientation
    tagname = ev->GetString(1);

    if (!tagname.length()) {
        warning("CCM::BeginTagEmitter",
                "Tagname not specified for tagemitter in model: '%s'\n",
                cgi.TIKI_Name(current_tiki));
    }

    tagnum = cgi.Tag_NumForName(current_tiki, tagname.c_str());
    if (tagnum == -1) {
        throw ScriptException("Tagname '%s' does not exist", tagname.c_str());
    }

    // Setup ending function
    endblockfcn = &ClientGameCommandManager::EndTagEmitter;

    // Init the emitter
    m_spawnthing = CreateNewEmitter();
    m_spawnthing->tagnum = tagnum;
    m_spawnthing->emittername = ev->GetString(2);
    if (!m_spawnthing->emittername.length()) {
        warning("CCM::BeginTagEmitter",
                "Emittername not specified for tagemitter in model: '%s'\n",
                cgi.TIKI_Name(current_tiki));
    }

    m_spawnthing->cgd.tiki = current_tiki;
}

//===============
// EndTagEmitter
//===============
void ClientGameCommandManager::EndTagEmitter(void) { endblockfcn = NULL; }

//===============
// BeginTagBeamEmitter
//===============
void ClientGameCommandManager::BeginTagBeamEmitter(Event* ev)

{
    // Setup ending function
    endblockfcn = &ClientGameCommandManager::EndTagBeamEmitter;

    // Init the emitter
    m_spawnthing = CreateNewEmitter();
    if (!m_spawnthing) {
        return;
    }

    m_spawnthing->cgd.flags |= T_BEAMTHING;

    // Get the tagname and orientation

    m_spawnthing->startTag = ev->GetString(1);

    if (m_spawnthing->startTag == "USE_TAGLIST") {
        if (ev->NumArgs() == 2) {
            m_spawnthing->emittername = ev->GetString(2);
        }
    } else {
        if (ev->NumArgs() == 3) {
            m_spawnthing->endTag = ev->GetString(2);
            m_spawnthing->emittername = ev->GetString(3);
        } else if (ev->NumArgs() == 2) {
            m_spawnthing->emittername = ev->GetString(2);
        }
    }

    m_spawnthing->cgd.tiki = current_tiki;
}

//===============
// EndTagBeamEmitter
//===============
void ClientGameCommandManager::EndTagBeamEmitter(void) { endblockfcn = NULL; }

//===============
// BeginOriginEmitter
//===============
void ClientGameCommandManager::BeginOriginEmitter(Event* ev)

{
    // Setup ending function
    endblockfcn = &ClientGameCommandManager::EndOriginEmitter;

    // Init the emitter
    m_spawnthing = CreateNewEmitter();
    if (!m_spawnthing) {
        return;
    }

    // Get the emitter's name
    m_spawnthing->emittername = ev->GetString(1);

    if (!m_spawnthing->emittername.length()) {
        warning("CCM::BeginOriginEmitter",
                "Emittername not specified for originemitter in model: '%s'\n",
                cgi.TIKI_Name(current_tiki));
    }

    m_spawnthing->cgd.tiki = current_tiki;
}

//===============
// EndOriginEmitter
//===============
void ClientGameCommandManager::EndOriginEmitter(void) { endblockfcn = NULL; }

//===============
// BeginOriginBeamEmitter
//===============
void ClientGameCommandManager::BeginOriginBeamEmitter(Event* ev)

{
    // Setup ending function
    endblockfcn = &ClientGameCommandManager::EndOriginBeamEmitter;

    // Init the emitter
    m_spawnthing = CreateNewEmitter();
    if (!m_spawnthing) {
        return;
    }

    // Get the emitter's name
    m_spawnthing->emittername = ev->GetString(1);

    if (!m_spawnthing->emittername.length()) {
        warning("CCM::BeginOriginEmitter",
                "Emittername not specified for originemitter in model: '%s'\n",
                cgi.TIKI_Name(current_tiki));
    }

    m_spawnthing->cgd.tiki = current_tiki;
    m_spawnthing->cgd.flags |= T_BEAMTHING;
}

//===============
// EndOriginBeamEmitter
//===============
void ClientGameCommandManager::EndOriginBeamEmitter(void)

{
    endblockfcn = NULL;
}

//===============
// GetOrientation - Calculates the orientation of a tag
//===============
void ClientGameCommandManager::GetOrientation(int tagnum, spawnthing_t* sp)
{
    int i;
    orientation_t or;

    assert(current_entity);
    assert(current_tiki);

    if (!current_entity || !current_tiki) {
        return;
    }

    or = cgi.TIKI_Orientation(current_entity, tagnum);

    VectorCopy(current_entity->origin, sp->cgd.origin);

    for (i = 0; i < 3; i++) {
        VectorMA(sp->cgd.origin, or.origin[i], current_entity->axis[i],
                 sp->cgd.origin);
    }

    MatrixMultiply(or.axis, current_entity->axis, sp->axis);

    // If angles are not set, then use the angles from the tag
    if (!(sp->cgd.flags & T_ANGLES)) {
        vectoangles(sp->axis[0], sp->cgd.angles);
    }

    AxisCopy(sp->axis, sp->tag_axis);
}

//===============
// BeginTagSpawn
//===============
void ClientGameCommandManager::BeginTagSpawn(Event* ev)
{
    str tagname;
    int tagnum;

    // Setup ending function
    endblockfcn = &ClientGameCommandManager::EndTagSpawn;

    // Init the thing we are going to spawn
    m_spawnthing = InitializeSpawnthing(&m_localemitter);

    // Get the tagname and orientation
    tagname = ev->GetString(1);
    tagnum = cgi.Tag_NumForName(current_tiki, tagname.c_str());
    if (tagnum == -1) {
        throw ScriptException("Tagname '%s' does not exist", tagname.c_str());
    }

    GetOrientation(tagnum, m_spawnthing);
}

//===============
// EndTagSpawn
//===============
void ClientGameCommandManager::EndTagSpawn(void)

{
    // Okay we should have a valid spawnthing, let's create a render entity
    SpawnTempModel(m_spawnthing->count);
}

//===============
// BeginTagBeamShoot
//===============
void ClientGameCommandManager::BeginTagBeamSpawn(Event* ev)
{
    str tagname;
    int tagnum;

    // Setup ending function
    endblockfcn = &ClientGameCommandManager::EndTagBeamSpawn;

    // Init the thing we are going to spawn
    m_spawnthing = InitializeSpawnthing(&m_localemitter);

    // Get the tagname and orientation
    tagname = ev->GetString(1);
    tagnum = cgi.Tag_NumForName(current_tiki, tagname.c_str());
    if (tagnum == -1) {
        throw ScriptException("Tagname '%s' does not exist", tagname.c_str());
    }

    GetOrientation(tagnum, m_spawnthing);
}

//===============
// EndTagSpawn
//===============
void ClientGameCommandManager::EndTagBeamSpawn(void)

{
    // Okay we should have a valid spawnthing, let's create the beam now
    int i;
    int renderfx;
    Vector end;
    trace_t trace;
    float scale = 1.0f;

    if (current_entity) {
        scale = current_entity->scale;
    }

    for (i = 0; i < 3; i++) {
        m_spawnthing->cgd.origin += Vector(m_spawnthing->tag_axis[i])
                                    * (m_spawnthing->axis_offset_base[i]
                                    + m_spawnthing->axis_offset_amplitude[i] * random());
    }

    if (m_spawnthing->use_last_trace_end) {
        end = last_trace_end;
    } else {
        end = m_spawnthing->cgd.origin +
              Vector(m_spawnthing->axis[0]) * m_spawnthing->length +
              Vector(m_spawnthing->axis[1]) * crandom() * m_spawnthing->spreadx +
              Vector(m_spawnthing->axis[2]) * crandom() * m_spawnthing->spready;
        last_trace_end = end;
    }

    CG_Trace(&trace, m_spawnthing->cgd.origin, vec_zero, vec_zero, end,
             ENTITYNUM_NONE, MASK_SHOT, qfalse, qtrue, "EndTagBeamSpawn");

    if (current_entity) {
        renderfx = (current_entity->renderfx &
                    ~(RF_FLAGS_NOT_INHERITED | RF_LIGHTING_ORIGIN));
    } else {
        renderfx = 0;
    }

    CG_CreateBeam(m_spawnthing->cgd.origin, vec_zero, current_entity_number, 1,
                  m_spawnthing->cgd.alpha, m_spawnthing->cgd.scale * scale,
                  (m_spawnthing->beamflags | BEAM_LIGHTNING_EFFECT),
                  m_spawnthing->length, m_spawnthing->cgd.life, qfalse,
                  trace.endpos, m_spawnthing->min_offset,
                  m_spawnthing->max_offset, m_spawnthing->overlap,
                  m_spawnthing->numSubdivisions, m_spawnthing->delay,
                  m_spawnthing->cgd.shadername, m_spawnthing->cgd.color,
                  m_spawnthing->numspherebeams, m_spawnthing->sphereRadius,
                  m_spawnthing->toggledelay, m_spawnthing->endalpha, renderfx,
                  m_spawnthing->emittername);
}

//===============
// BeginOriginBeamSpawn
//===============
void ClientGameCommandManager::BeginOriginBeamSpawn(Event* ev)
{

    // Init the emitter
    m_spawnthing = CreateNewEmitter();
    if (!m_spawnthing) {
        return;
    }

    // Set the origin based on the entity's origin
    m_spawnthing->cgd.origin = current_entity->origin;

    // Set the axis based on the entity's axis
    AxisCopy(current_entity->axis, m_spawnthing->axis);
    AxisCopy(current_entity->axis, m_spawnthing->tag_axis);
    MatrixToEulerAngles(m_spawnthing->axis, m_spawnthing->cgd.angles);

    // Setup ending function
    endblockfcn = &ClientGameCommandManager::EndOriginBeamSpawn;
}

//===============
// EndOriginBeamSpawn
//===============
void ClientGameCommandManager::EndOriginBeamSpawn(void)

{
    // Okay we should have a valid spawnthing, let's create the beam now
    int i, renderfx, c, count;
    Vector end;
    trace_t trace;
    float scale = 1.0f;

    if (current_entity) {
        scale = current_entity->scale;
    }

    count = m_spawnthing->count * cg_effectdetail->value;

    for (c = 0; c <= count; c++) {
        Vector angles;

        for (i = 0; i < 3; i++) {
            // Randomize angles or set absolute
            angles[i] = m_spawnthing->cgd.angles[i] +
                        random() * m_spawnthing->angles_amplitude[i];

            m_spawnthing->cgd.origin +=
                Vector(m_spawnthing->tag_axis[i]) *
                (m_spawnthing->axis_offset_base[i] +
                 m_spawnthing->axis_offset_amplitude[i] * random());
        }

        AnglesToAxis(angles, m_spawnthing->axis);
        end = m_spawnthing->cgd.origin +
              Vector(m_spawnthing->axis[0]) * m_spawnthing->length +
              Vector(m_spawnthing->axis[1]) * crandom() * m_spawnthing->spreadx +
              Vector(m_spawnthing->axis[2]) * crandom() * m_spawnthing->spready;

        CG_Trace(&trace, m_spawnthing->cgd.origin, vec_zero, vec_zero, end,
                 ENTITYNUM_NONE, MASK_SHOT, qfalse, qtrue,
                 "EndOriginBeamSpawn");

        if (current_entity) {
            renderfx = (current_entity->renderfx &
                        ~(RF_FLAGS_NOT_INHERITED | RF_LIGHTING_ORIGIN));
        } else {
            renderfx = 0;
        }

        CG_CreateBeam(
            m_spawnthing->cgd.origin, vec_zero, current_entity_number, 1,
            m_spawnthing->cgd.alpha, m_spawnthing->cgd.scale * scale,
            (m_spawnthing->beamflags | BEAM_LIGHTNING_EFFECT),
            m_spawnthing->length, m_spawnthing->cgd.life, qtrue, trace.endpos,
            m_spawnthing->min_offset, m_spawnthing->max_offset,
            m_spawnthing->overlap, m_spawnthing->numSubdivisions,
            m_spawnthing->delay, m_spawnthing->cgd.shadername,
            m_spawnthing->cgd.color, m_spawnthing->numspherebeams,
            m_spawnthing->sphereRadius, m_spawnthing->toggledelay,
            m_spawnthing->endalpha, renderfx, m_spawnthing->emittername);
    }
}

//=============
// AllocateTempModel
//=============
ctempmodel_t* ClientGameCommandManager::AllocateTempModel(void)

{
    ctempmodel_t* p;

    if (!m_free_tempmodels) {
        // no free entities, so free the one at the end of the chain
        // remove the oldest active entity
        FreeTempModel(m_active_tempmodels.prev);
    }

    p = m_free_tempmodels;
    m_free_tempmodels = m_free_tempmodels->next;

    // link into the active list
    p->next = m_active_tempmodels.next;
    p->prev = &m_active_tempmodels;
    m_active_tempmodels.next->prev = p;
    m_active_tempmodels.next = p;

    return p;
}

//===============
// FreeTempModel
//===============
void ClientGameCommandManager::FreeTempModel(ctempmodel_t* p)

{
    if (!p->prev) {
        cgi.Error(ERR_DROP, "CCM::FreeTempModel: not active");
    }

    RemoveClientEntity(p->number, p->cgd.tiki, NULL, p);

    // remove from the doubly linked active list
    p->prev->next = p->next;
    p->next->prev = p->prev;

    // the free list is only singly linked
    p->next = m_free_tempmodels;
    m_free_tempmodels = p;
}

void ClientGameCommandManager::ResetTempModels(void)

{
    // Go through all the active tempmodels and free them
    ctempmodel_t *p, *next;

    p = m_active_tempmodels.prev;
    for (; p != &m_active_tempmodels; p = next) {
        next = p->prev;
        FreeTempModel(p);
    }
}

//=============
// InitializeTempModels
//=============
void ClientGameCommandManager::InitializeTempModels(void)

{
    int i;
    int numtempmodels = MAX_TEMPMODELS;

    m_active_tempmodels.next = &m_active_tempmodels;
    m_active_tempmodels.prev = &m_active_tempmodels;

    m_free_tempmodels = &m_tempmodels[0];

    for (i = 0; i < numtempmodels - 1; i++) {
        m_tempmodels[i].next = &m_tempmodels[i + 1];
    }
    m_tempmodels[numtempmodels - 1].next = NULL;
}

void ClientGameCommandManager::InitializeTempModelCvars(void)
{
    cg_showtempmodels = cgi.Cvar_Get("cg_showtempmodels", "0", 0);
    cg_detail = cgi.Cvar_Get("detail", "1", 1);
    cg_effectdetail = cgi.Cvar_Get("cg_effectdetail", "0.2", 1);
    cg_effect_physicsrate = cgi.Cvar_Get("cg_effect_physicsrate", "10", 1);
}

//=============
// InitializeEmitters
//=============
void ClientGameCommandManager::InitializeEmitters(void) {}

//===============
// UpdateSwarm
//===============
void ClientGameCommandManager::UpdateSwarm(ctempmodel_t* p)

{
    if (p->cgd.swarmfreq == 0) {
        return;
    }

    // If the frequency is hit, set a new velocity
    if (!(rand() % p->cgd.swarmfreq)) {
        p->cgd.velocity.x = crandom() * p->cgd.swarmmaxspeed;
        p->cgd.velocity.y = crandom() * p->cgd.swarmmaxspeed;
        p->cgd.velocity.z = crandom() * p->cgd.swarmmaxspeed;
    }

    // Try to move toward the origin by the specified delta
    if (p->cgd.origin.x < p->cgd.parentOrigin.x) {
        p->cgd.velocity.x += p->cgd.swarmdelta;
    } else {
        p->cgd.velocity.x -= p->cgd.swarmdelta;
    }

    if (p->cgd.origin.y < p->cgd.parentOrigin.y) {
        p->cgd.velocity.y += p->cgd.swarmdelta;
    } else {
        p->cgd.velocity.y -= p->cgd.swarmdelta;
    }

    if (p->cgd.origin.z < p->cgd.parentOrigin.z) {
        p->cgd.velocity.z += p->cgd.swarmdelta;
    } else {
        p->cgd.velocity.z -= p->cgd.swarmdelta;
    }
}

void ClientGameCommandManager::OtherTempModelEffects(ctempmodel_t* p,
                                                     Vector origin,
                                                     refEntity_t* newEnt)

{
    vec3_t axis[3];

    if (p->number != -1) {
        // Set the axis
        AnglesToAxis(p->cgd.angles, axis);

        current_scale = newEnt->scale;
        current_entity = newEnt;
        current_tiki = p->cgd.tiki;
        current_entity_number = p->number;

        // Update any emitters that are active on this tempmodel
        UpdateEmitter(p->cgd.tiki, axis, p->number, p->cgd.parent,
                      origin);

        // Add in trails for this tempmodel
        if (p->cgd.flags2 & T2_TRAIL) {
            Event* ev = new Event(EV_Client_Swipe);
            ev->AddVector(origin);
            commandManager.ProcessEvent(ev);
        }

        current_entity_number = -1;
        current_tiki = NULL;
        current_entity = NULL;
        current_scale = -1;
    }
}

//===============
// AnimateTempModel - animate temp models
//===============
void ClientGameCommandManager::AnimateTempModel(ctempmodel_t* p, Vector origin,
                                                refEntity_t* newEnt)

{
    int numframes;
    int deltatime;
    int frametime;

    // This code is for animating tempmodels that are spawned from the client
    // side

    if (p->cgd.tiki < 0) {
        return;
    }

    // Calc frame stuff
    frametime = 1000.0f * cgi.Anim_Frametime(p->cgd.tiki, p->ent.frameInfo[0].index);
    deltatime = cg.time - p->lastAnimTime;
    numframes = cgi.Anim_NumFrames(p->cgd.tiki, p->ent.frameInfo[0].index);

    if (!p->addedOnce) {
        // Process entry commands
        CG_ProcessEntityCommands(TIKI_FRAME_ENTRY,
                                 p->ent.frameInfo[0].index, -1, &p->ent, NULL);
    }

    if (numframes < 2) {
        return;
    }

    // Go through all the frames, and process any commands associated with the
    // tempmodel as well
    while (deltatime >= frametime) {
        p->lastAnimTime += frametime;
        p->ent.wasframe = (p->ent.wasframe + 1) % numframes;
        CG_ProcessEntityCommands(p->ent.wasframe, p->ent.frameInfo[0].index,
                                 -1, &p->ent, NULL);
    }
}

qboolean ClientGameCommandManager::TempModelRealtimeEffects(ctempmodel_t* p,
                                                            float ftime,
                                                            float dtime,
                                                            float scale)

{
    float fade, fadein;
    byte tempColor[4];

    if (p->cgd.flags & (T_FADE | T_SCALEUPDOWN)) {
        fade = 1.0f - (float)(p->aliveTime - p->cgd.fadedelay) /
                          (float)(p->cgd.life - p->cgd.fadedelay);

        // Clamp the fade
        if (fade > 1) {
            fade = 1;
        }
        if (fade < 0) {
            fade = 0;
        }
    } else {
        fade = 1.0f;
    }

    dtime = (cg.time - p->cgd.createTime);

    // Calculate fade in value
    if (p->cgd.flags & T_FADEIN) {
        fadein = dtime / (float)p->cgd.fadeintime;
    } else {
        fadein = 0;
    }

    // Convert dtime to seconds
    dtime *= 0.001f;

    // Do the scale animation
    if (ftime && p->cgd.scaleRate) {
        p->ent.scale += p->ent.scale * (p->cgd.scaleRate * ftime);
    }

    if (p->cgd.flags & T_SCALEUPDOWN) {
        p->ent.scale = p->cgd.scale * sin((fade)*M_PI);

        if (p->ent.scale < p->cgd.scalemin) {
            p->ent.scale = p->cgd.scalemin;
        }
        if (p->ent.scale > p->cgd.scalemax) {
            p->ent.scale = p->cgd.scalemax;
        }
    }

    if (p->cgd.lightstyle >= 0) {
        int i;
        float color[4];
        CG_LightStyleColor(p->cgd.lightstyle, dtime * 1000, color);
        for (i = 0; i < 4; i++) {
            tempColor[i] = color[i] * 255;
        }
    } else {
        memcpy(tempColor, p->cgd.color, 4);
    }

    if (p->cgd.flags & T_TWINKLE) {
        // See if we should toggle the twinkle
        if (cg.time > p->twinkleTime) {
            // If off, turn it on
            if (p->cgd.flags & T_TWINKLE_OFF) {
                p->cgd.flags &= ~T_TWINKLE_OFF;
                p->twinkleTime = cg.time + p->cgd.min_twinkletimeon +
                                 random() * p->cgd.max_twinkletimeon;
            } else {
                p->cgd.flags |= T_TWINKLE_OFF;
                p->twinkleTime = cg.time + p->cgd.min_twinkletimeoff +
                                 random() * p->cgd.max_twinkletimeoff;
            }
        }

        if (p->cgd.flags & T_TWINKLE_OFF) {
            memset(tempColor, 0, 4);
        }
    }

    if (p->cgd.flags & T_FADEIN && (fadein < 1)) // Do the fadein effect
    {
        p->ent.shaderRGBA[0] = (int)(tempColor[0] * (fadein * p->cgd.alpha));
        p->ent.shaderRGBA[1] = (int)(tempColor[1] * (fadein * p->cgd.alpha));
        p->ent.shaderRGBA[2] = (int)(tempColor[2] * (fadein * p->cgd.alpha));
        p->ent.shaderRGBA[3] = (int)(tempColor[3] * (fadein * p->cgd.alpha));
    } else if (p->cgd.flags & T_FADE) // Do a fadeout effect
    {
        p->ent.shaderRGBA[0] = (tempColor[0] * (fade * p->cgd.alpha));
        p->ent.shaderRGBA[1] = (tempColor[1] * (fade * p->cgd.alpha));
        p->ent.shaderRGBA[2] = (tempColor[2] * (fade * p->cgd.alpha));
        p->ent.shaderRGBA[3] = (tempColor[3] * (fade * p->cgd.alpha));
    } else {
        p->ent.shaderRGBA[0] = tempColor[0] * p->cgd.alpha;
        p->ent.shaderRGBA[1] = tempColor[1] * p->cgd.alpha;
        p->ent.shaderRGBA[2] = tempColor[2] * p->cgd.alpha;
        p->ent.shaderRGBA[3] = tempColor[3] * p->cgd.alpha;
    }

    if (p->cgd.flags & T_FLICKERALPHA) {
        float random = random();

        if (p->cgd.flags & (T_FADE | T_FADEIN)) {
            p->ent.shaderRGBA[0] *= random;
            p->ent.shaderRGBA[1] *= random;
            p->ent.shaderRGBA[2] *= random;
            p->ent.shaderRGBA[3] *= random;
        } else {
            p->ent.shaderRGBA[0] = p->cgd.color[0] * random;
            p->ent.shaderRGBA[1] = p->cgd.color[1] * random;
            p->ent.shaderRGBA[2] = p->cgd.color[2] * random;
            p->ent.shaderRGBA[3] = p->cgd.color[3] * random;
        }
    }

    // Check for completely faded out model
    if (fade <= 0 && p->addedOnce) {
        return false;
    }

    // Check for completely scaled out model
    if ((p->ent.scale <= 0 && p->addedOnce) &&
        !(p->cgd.flags & T_SCALEUPDOWN)) {
        return false;
    }

    // Do swarming flies effects
    if (p->cgd.flags & T_SWARM) {
        UpdateSwarm(p);
    }

    return true;
}

qboolean ClientGameCommandManager::TempModelPhysics(ctempmodel_t* p,
                                                    float ftime, float time2,
                                                    float scale)

{
    int dtime;
    Vector parentOrigin(0, 0, 0);
    Vector parentAngles(0, 0, 0);
    Vector tempangles;
    trace_t trace;
    float dot;

    VectorCopy(p->ent.origin, p->lastEnt.origin);
    AxisCopy(p->ent.axis, p->lastEnt.axis);

    dtime = (cg.time - p->cgd.createTime);

    // Save oldorigin
    p->cgd.oldorigin = p->cgd.origin;

    // Update the orign and the angles based on velocities first
    if (p->cgd.flags2 & (T2_MOVE | T2_ACCEL)) {
        p->cgd.origin = p->cgd.origin + (p->cgd.velocity * ftime * scale) +
                        (time2 * p->cgd.accel);
    }

    // If linked to the parent or hardlinked, get the parent's origin
    if ((p->cgd.flags & (T_PARENTLINK | T_HARDLINK)) &&
        (p->cgd.parent != ENTITYNUM_NONE)) {
        centity_t* pc;
        pc = &cg_entities[p->cgd.parent];

        if (pc->currentValid) {
            refEntity_t* e;

            e = cgi.R_GetRenderEntity(p->cgd.parent);

            if (!e) {
                return false;
            }

            parentOrigin = e->origin;
            vectoangles(e->axis[0], parentAngles);
        } else {
            return false;
        }
    }

    // Align the object along it's traveling vector
    if (p->cgd.flags & T_ALIGN) {
        p->cgd.angles = p->cgd.velocity.toAngles();
        parentAngles = vec_zero;
    }

    if (p->cgd.flags & T_RANDOMROLL) {
        p->cgd.angles[ROLL] = random() * 360;
    }

    // Update the angles based on angular velocity
    if (p->cgd.flags2 & T2_AMOVE) {
        p->cgd.angles = p->cgd.angles + (ftime * p->cgd.avelocity);
    }

    // Mod the angles if needed
    p->cgd.angles[0] = AngleMod(p->cgd.angles[0]);
    p->cgd.angles[1] = AngleMod(p->cgd.angles[1]);
    p->cgd.angles[2] = AngleMod(p->cgd.angles[2]);

    // Add in parent angles
    tempangles = p->cgd.angles + parentAngles;

    // Convert to axis
    if ((p->cgd.flags &
         (T_ALIGN | T_RANDOMROLL | T_PARENTLINK | T_HARDLINK | T_ANGLES)) ||
        (p->cgd.flags2 & T2_AMOVE)) {
        AnglesToAxis(tempangles, p->ent.axis);
    }

    // Only do real collision if necessary
    if (p->cgd.flags & T_COLLISION) {
        // trace a line from previous position to new position
        CG_Trace(&trace, p->cgd.oldorigin, NULL, NULL, p->cgd.origin, -1,
                 p->cgd.collisionmask, qfalse, qfalse, "Collision");
    } else {
        // Fake it out so it never collides
        trace.fraction = 1.0;
    }

    // Check for collision
    if (trace.fraction == 1.0) {
        // Acceleration of velocity
        if (p->cgd.flags2 & T2_ACCEL) {
            p->cgd.velocity = p->cgd.velocity + ftime * p->cgd.accel;
        }
    } else {
        if (p->touchfcn) {
            p->touchfcn(p, &trace);
        }

        if (p->cgd.flags & T_DIETOUCH) {
            return false;
        }

        Vector normal;

        // Set the origin
        p->cgd.origin = trace.endpos;

        if ((p->cgd.flags2 & T2_BOUNCE_DECAL) &&
            (p->cgd.bouncecount < p->cgd.maxbouncecount)) {
            // Put down a bounce decal
            qhandle_t shader = cgi.R_RegisterShader(p->cgd.shadername);

            CG_ImpactMark(shader, trace.endpos, trace.plane.normal,
                          p->cgd.decal_orientation,
                          (float)p->cgd.color[0] / 255.0f,
                          (float)p->cgd.color[1] / 255.0f,
                          (float)p->cgd.color[2] / 255.0f, p->cgd.alpha,
                          p->cgd.flags & T_FADE, p->cgd.decal_radius,
                          p->cgd.flags2 & T2_TEMPORARY_DECAL, p->cgd.lightstyle,
                          p->cgd.flags & T_FADEIN,
                          0.f,
                          0.f);

            p->cgd.bouncecount++;
        }

        // calculate the bounce
        normal = trace.plane.normal;

        // reflect the velocity on the trace plane
        if (p->cgd.flags2 & T2_ACCEL) {
            p->cgd.velocity =
                p->cgd.velocity + ftime * trace.fraction * p->cgd.accel;
        }

        dot = p->cgd.velocity * normal;
        p->cgd.velocity = p->cgd.velocity + ((-2 * dot) * normal);
        p->cgd.velocity *= p->cgd.bouncefactor;

        // check for stop
        if (trace.plane.normal[2] > 0 && p->cgd.velocity[2] < 40) {
            p->cgd.velocity = Vector(0, 0, 0);
            p->cgd.avelocity = Vector(0, 0, 0);
            p->cgd.flags &= ~T_WAVE;
        } else {
            if (p->cgd.flags & T_BOUNCESOUNDONCE) {
                vec3_t org;
                VectorCopy(p->cgd.origin, org);
                PlaySound(p->cgd.bouncesound, org);
                p->cgd.flags &= ~(T_BOUNCESOUNDONCE | T_BOUNCESOUND);
            } else if ((p->cgd.flags & T_BOUNCESOUND) &&
                       (p->next_bouncesound_time < cg.time)) {
                vec3_t org;
                VectorCopy(p->cgd.origin, org);
                PlaySound(p->cgd.bouncesound, org);
                p->next_bouncesound_time = cg.time + p->cgd.bouncesound_delay;
            }
        }
    }

    // copy over origin
    VectorCopy(p->cgd.origin, p->ent.origin);

    // Add in parent's origin if linked
    if (p->cgd.flags & (T_PARENTLINK | T_HARDLINK)) {
        VectorAdd(p->ent.origin, parentOrigin, p->ent.origin);
    }

    if (!p->lastEntValid) {
        // Make the lastEnt valid, by setting it to p->ent and setting the
        // origin to the tempmodel's oldorigin
        p->lastEnt = p->ent;
        VectorCopy(p->cgd.oldorigin, p->lastEnt.origin);
        p->lastEntValid = true;
    }

    return true;
}

qboolean ClientGameCommandManager::LerpTempModel(refEntity_t* newEnt,
                                                 ctempmodel_t* p, float frac)

{
    int i, j;

    // If the tempmodel is parentlinked, then we need to get the origin of the
    // parent and add it to the tempmodel's origin
    if (p->cgd.flags & (T_PARENTLINK | T_HARDLINK)) {
        centity_t* pc;
        Vector parentOrigin;

        // Lerp the tempmodel's local origin
        for (i = 0; i < 3; i++) {
            newEnt->origin[i] = p->cgd.oldorigin[i] +
                                frac * (p->cgd.origin[i] - p->cgd.oldorigin[i]);
        }

        // Find the parent entity
        pc = &cg_entities[p->cgd.parent];

        if (pc->currentValid) {
            refEntity_t* e;

            e = cgi.R_GetRenderEntity(p->cgd.parent);

            if (!e) {
                return false;
            }

            parentOrigin = e->origin;
        } else {
            return false;
        }

        // Add the parent ent's origin to the local origin
        VectorAdd(newEnt->origin, parentOrigin, newEnt->origin);
    } else {
        if (p->cgd.flags2 & (T2_MOVE | T2_ACCEL)) {
            // Lerp the ent's origin
            for (i = 0; i < 3; i++) {
                newEnt->origin[i] =
                    p->lastEnt.origin[i] +
                    frac * (p->ent.origin[i] - p->lastEnt.origin[i]);
            }
        }
    }

    if (p->cgd.flags2 & T2_PARALLEL) {
        Vector v1 = p->cgd.origin - cg.refdef.vieworg;
        vectoangles(v1, p->cgd.angles);
        AnglesToAxis(p->cgd.angles, newEnt->axis);
    } else if ((p->cgd.flags &
                (T_ALIGN | T_RANDOMROLL | T_PARENTLINK | T_HARDLINK)) ||
               (p->cgd.flags2 & T2_AMOVE)) {
        // Lerp axis
        for (i = 0; i < 3; i++) {
            for (j = 0; j < 3; j++) {
                newEnt->axis[i][j] =
                    p->lastEnt.axis[i][j] +
                    frac * (p->ent.axis[i][j] - p->lastEnt.axis[i][j]);
            }
        }
    }

    return true;
}

static int lastTempModelFrameTime = 0;

//===============
// AddTempModels - Update and add tempmodels to the ref
//===============
#define TOO_MUCH_TIME_PASSED 500
void ClientGameCommandManager::AddTempModels(void)

{
    ctempmodel_t *p, *next;
    int count = 0; // Tempmodel count
    int frameTime;
    float effectTime, effectTime2;
    int mstime = 0;
    float ftime = 0;
    float time2 = 0;
    float scale = 1.0f;
    float lerpfrac = 0;
    int physics_rate = 0;
    qboolean ret;
    refEntity_t newEnt;
    dtiki_t* old_tiki;
    int old_num;
    refEntity_t* old_ent;

    // To counteract cg.time going backwards
    if (lastTempModelFrameTime) {
        if ((cg.time < lastTempModelFrameTime) ||
            (cg.time - lastTempModelFrameTime > TOO_MUCH_TIME_PASSED)) {
            p = m_active_tempmodels.prev;
            for (; p != &m_active_tempmodels; p = next) {
                next = p->prev;
                p->lastPhysicsTime = cg.time;
            }
            lastTempModelFrameTime = cg.time;
            return;
        }
    }

    if (lastTempModelFrameTime != 0) {
        frameTime = cg.time - lastTempModelFrameTime;
    } else {
        frameTime = 0;
    }

    if (paused->integer) {
        lastTempModelFrameTime = 0;
    } else {
        lastTempModelFrameTime = cg.time;
    }

    memset(&newEnt, 0, sizeof(newEnt));
    // Set this frame time for the next one
    effectTime = (float)frameTime / 1000.0f;
    effectTime2 = effectTime * effectTime;

    // If there is a current entity, it's scale is used as a factor
    if (current_entity) {
        scale = current_entity->scale;
    }

    // Go through all the temp models and run the physics if necessary,
    // then add them to the ref
    old_ent = current_entity;
    old_tiki = current_tiki;
    old_num = current_entity_number;

    p = m_active_tempmodels.prev;
    for (; p != &m_active_tempmodels; p = next) {
        // grab next now, so if the local entity is freed we still have it
        next = p->prev;

        if ((p->cgd.flags & T_DETAIL) && !cg_detail->integer) {
            FreeTempModel(p);
            continue;
        }

        p->ent.tiki = p->cgd.tiki;
        current_entity = &p->ent;
        current_tiki = p->cgd.tiki;
        current_entity_number = p->number;

        TempModelRealtimeEffects(p, effectTime, effectTime2, scale);

        if (p->lastPhysicsTime) {
            mstime = cg.time - p->lastPhysicsTime;

            // Check for physics
            physics_rate =
                1000 / p->cgd.physicsRate; // Physics rate in milliseconds

            // Avoid large jumps in time
            if (mstime > physics_rate * 2) {
                mstime = physics_rate;
            }

            if ((mstime >= physics_rate) ||
                (p->cgd.flags2 & T2_PHYSICS_EVERYFRAME)) {
                ftime = mstime / 1000.0f;
                time2 = ftime * ftime;
                ret = TempModelPhysics(p, ftime, time2, scale);

                if (!ret) {
                    FreeTempModel(p);
                    continue;
                }

                p->lastPhysicsTime = cg.time;
            }
        }

        // Calculate the lerp value based on the time passed since last physics
        // time of this tempmodel
        lerpfrac = (float)(cg.time - p->lastPhysicsTime) / (float)physics_rate;

        // Clamp
        if (lerpfrac > 1) {
            lerpfrac = 1;
        }
        if (lerpfrac < 0) {
            lerpfrac = 0;
        }

        // Increment the time this tempmodel has been alive
        p->aliveTime += frameTime;

        // Dead, and free up the tempmodel
        if (p->aliveTime >= p->cgd.life && p->addedOnce) {
            FreeTempModel(p);
            continue;
        }

        // Run physics if the lastEnt is not valid to get a valid lerp
        if (!p->lastEntValid) {
            float t, t2;
            t = physics_rate / 1000.0f;
            t2 = t * t;

            ret = TempModelPhysics(p, t, t2, scale);
            if (!ret) {
                FreeTempModel(p);
                continue;
            }

            lerpfrac = 0;
            p->lastPhysicsTime = cg.time;
        }

        // clear out the new entity and initialize it
        // this will become the current_entity if anything is spawned off it

        newEnt.scale = p->ent.scale;
        memcpy(newEnt.shaderRGBA, p->ent.shaderRGBA, 4);
        AxisCopy(p->ent.axis, newEnt.axis);
        VectorCopy(p->ent.origin, newEnt.origin);

        // Lerp the tempmodel
        if (!LerpTempModel(&newEnt, p, lerpfrac)) {
            FreeTempModel(p);
            continue;
        }

        if (p->cgd.flags & T_WAVE) {
            Vector origin;
            float axis[3][3];
            
            origin = Vector(m_spawnthing->linked_origin) +
                Vector(m_spawnthing->linked_axis[0]) * newEnt.origin[0] +
                Vector(m_spawnthing->linked_axis[1]) * newEnt.origin[1] +
                Vector(m_spawnthing->linked_axis[2]) * newEnt.origin[2];

            VectorCopy(origin, newEnt.origin);

            MatrixMultiply(newEnt.axis, p->m_spawnthing->linked_axis, axis);
            AxisCopy(axis, newEnt.axis);
        }

        // Animate and do trails (swipes)
        newEnt.renderfx = p->ent.renderfx;
        newEnt.hModel = p->ent.hModel;
        newEnt.reType = p->ent.reType;
        newEnt.shaderTime = p->ent.shaderTime;

        newEnt.frameInfo[0] = p->ent.frameInfo[0];
        newEnt.wasframe = p->ent.wasframe;
        newEnt.actionWeight = 1.0;
        newEnt.entityNumber = ENTITYNUM_NONE;
        newEnt.tiki = p->ent.tiki;

        AnimateTempModel(p, newEnt.origin, &newEnt);

        OtherTempModelEffects(p, newEnt.origin, &newEnt);

        // Add to the ref
        if (p->cgd.flags & T_DLIGHT) {
            // Tempmodel is a Dynamic Light
            cgi.R_AddLightToScene(p->cgd.origin, p->cgd.lightIntensity * scale,
                                  (float)p->cgd.color[0] / 255.0f,
                                  (float)p->cgd.color[1] / 255.0f,
                                  (float)p->cgd.color[2] / 255.0f,
                                  p->cgd.lightType);
        } else if (p->ent.reType == RT_SPRITE) {
            vec3_t vTestAngles;
            cgi.R_AddRefSpriteToScene(&newEnt); // Sprite
            MatrixToEulerAngles(newEnt.axis, vTestAngles);
        } else {
            cgi.R_AddRefEntityToScene(&newEnt, ENTITYNUM_NONE); // Model
        }

        // Set the added once flag so we can delete it later
        p->addedOnce = qtrue;

        // Local tempmodel count stat
        count++;
    }

    current_entity = old_ent;
    current_tiki = old_tiki;
    current_entity_number = old_num;

    // stats
    if (cg_showtempmodels->integer) {
        cgi.DPrintf("TC:%i\n", count);
    }
}

//=================
// SpawnTempModel
//=================
void ClientGameCommandManager::SpawnTempModel(int count, spawnthing_t* sp)
{
    m_spawnthing = sp;
    SpawnTempModel(count);
}

//=================
// SpawnTempModel
//=================
void ClientGameCommandManager::SpawnTempModel(int mcount, int timeAlive)

{
    int i;
    ctempmodel_t* p;
    refEntity_t ent;
    int count;
    float current_entity_scale = 1.0f;
    Vector newForward;
    Vector delta;
    Vector start;

    if (current_entity) {
        current_entity_scale = current_entity->scale;
    } else {
        current_entity_scale = 1.0f;
    }

    if (current_scale > 0) {
        current_entity_scale *= current_scale;
    }

    if (mcount > 1) {
        mcount *= cg_effectdetail->value;
    }

    if (mcount < 1) {
        mcount = 1;
    }

    for (count = 0; count < mcount; count++) {
        p = AllocateTempModel();

        if (!p) {
            cgi.DPrintf("Out of tempmodels\n");
            return;
        }

        memset(&ent, 0, sizeof(refEntity_t));
        memset(&p->lastEnt, 0, sizeof(refEntity_t));

        if (p->cgd.flags & T_WAVE)
        {
            p->m_spawnthing = m_spawnthing;
            start = Vector(0, 0, 0);
        }
        else
        {
            p->m_spawnthing = NULL;
            start = m_spawnthing->cgd.origin;
        }

        // Copy over the common data block
        p->cgd = m_spawnthing->cgd;

        // newForward may be changed by spehere or circle
        newForward = m_spawnthing->axis[0];

        // Set up the origin of the tempmodel
        if (m_spawnthing->cgd.flags & T_SPHERE) {
            // Create a random forward vector so the particles burst out in a
            // sphere
            newForward = Vector(crandom(), crandom(), crandom());
        } else if (m_spawnthing->cgd.flags & T_CIRCLE) {
            if (m_spawnthing->sphereRadius != 0) // Offset by the radius
            {
                Vector dst;
                // Create a circular shaped burst around the up vector
                float angle = ((float)count / (float)m_spawnthing->count) *
                              360; // * M_PI * 2;

                Vector end = Vector(m_spawnthing->axis[1]) *
                             m_spawnthing->sphereRadius * current_entity_scale;
                RotatePointAroundVector(dst, Vector(m_spawnthing->axis[0]), end, angle);

                VectorAdd(dst, m_spawnthing->cgd.origin, p->cgd.origin);
                newForward = p->cgd.origin - m_spawnthing->cgd.origin;
                newForward.normalize();
            }
        } else if (m_spawnthing->cgd.flags & T_INWARDSPHERE) {
            // Project the origin along a random ray, and set the forward
            // vector pointing back to the origin
            Vector dir, end;

            dir = Vector(crandom(), crandom(), crandom());

            end = m_spawnthing->cgd.origin +
                  dir * m_spawnthing->sphereRadius * current_entity_scale;
            VectorCopy(end, p->cgd.origin);
            newForward = dir * -1;
        } else if (m_spawnthing->sphereRadius !=
                   0) // Offset in a spherical pattern
        {
            Vector dir, end;

            dir = Vector(crandom(), crandom(), crandom());

            dir.normalize();

            end = m_spawnthing->cgd.origin +
                  dir * m_spawnthing->sphereRadius * current_entity_scale;
            VectorCopy(end, p->cgd.origin);
            newForward = dir;
        } else {
            VectorCopy(m_spawnthing->cgd.origin, p->cgd.origin);
        }

        // Randomize the origin based on offsets
        for (i = 0; i < 3; i++) {
            p->cgd.origin[i] += (random() * m_spawnthing->origin_offset_amplitude[i] + m_spawnthing->origin_offset_base[i]) *
                                current_entity_scale;
        }

        p->cgd.oldorigin = p->cgd.origin;
        p->modelname = m_spawnthing->GetModel();
        p->addedOnce = qfalse;
        p->lastEntValid = qfalse;

        if (p->modelname.length()) {
            ent.hModel = cgi.R_RegisterModel(p->modelname.c_str());
        }

        // Initialize the refEntity
        ent.shaderTime = cg.time / 1000.0f;

        // Get the tikihandle
        p->cgd.tiki = cgi.R_Model_GetHandle(ent.hModel);

        // Set the reftype based on the modelname
        if (p->modelname == "*beam") {
            ent.reType = RT_BEAM;
            ent.customShader = cgi.R_RegisterShader("beamshader");
        } else if (strstr(p->modelname, ".spr")) {
            ent.reType = RT_SPRITE;
        } else {
            ent.reType = RT_MODEL;
        }

        // Set the animation
        if (m_spawnthing->animName.length() && (p->cgd.tiki > 0)) {
            ent.frameInfo[0].index =
                cgi.Anim_NumForName(p->cgd.tiki, m_spawnthing->animName);
        } else if (ent.reType == RT_MODEL && (p->cgd.tiki > 0)) {
            ent.frameInfo[0].index = cgi.Anim_NumForName(p->cgd.tiki, "idle");
        }

        // Randomize the scale
        if (m_spawnthing->cgd.flags & T_RANDSCALE) // Check for random scale
        {
            ent.scale = RandomizeRange(m_spawnthing->cgd.scalemin,
                                       m_spawnthing->cgd.scalemax);
            p->cgd.scale = ent.scale;
        } else {
            ent.scale = m_spawnthing->cgd.scale;
        }

        ent.scale *= current_entity_scale;

        // CURRENT ENTITY INFLUENCES ON THE TEMPMODELS HAPPEN HERE
        // copy over the renderfx from the current_entity, but only the flags we
        // want
        if (current_entity) {
            // explicitly add RF_LIGHTING ORIGIN and RF_SHADOWPLANE because we
            // don't want those on dynamic objects
            ent.renderfx |= (current_entity->renderfx &
                             ~(RF_FLAGS_NOT_INHERITED | RF_LIGHTING_ORIGIN));
        }

        // Set up modulation for constant color
        for (i = 0; i < 4; i++) {
            p->cgd.color[i] = ent.shaderRGBA[i] = m_spawnthing->cgd.color[i];
        }

        // Apply the alpha from the current_entity to the tempmodel
        if (current_entity) {
            float ealpha;

            ealpha = (float)current_entity->shaderRGBA[3] / 255.0f;

            if (ealpha != 1.0) {
                // pre-multiply the alpha from the entity
                for (i = 0; i < 4; i++) {
                    p->cgd.color[i] = ent.shaderRGBA[i] =
                        ent.shaderRGBA[i] * ealpha;
                }
            }
        }

        p->ent = ent;
        p->lastEnt = ent;
        p->number = -1;

        // If createTime is specified, the use it.  Otherwise use the createTime
        // from the spawnthing.
        if (timeAlive > 0) {
            p->aliveTime = timeAlive;
        } else {
            p->aliveTime = 0;
        }

        // If animateonce is set, set the life = to the length of the anim
        if ((m_spawnthing->cgd.flags & T_ANIMATEONCE) &&
            (p->ent.frameInfo[0].index > 0)) {
            p->cgd.life =
                cgi.Anim_Time(p->cgd.tiki, p->ent.frameInfo[0].index) * 1000.0f;
        } else {
            p->cgd.life = m_spawnthing->cgd.life;
        }

        p->lastAnimTime = p->cgd.createTime;
        p->lastPhysicsTime = p->cgd.createTime;
        p->killTime = cg.time + p->cgd.life;  // The time the tempmodel will die
        p->seed = m_seed++;                   // For use with randomizations
        p->cgd.velocity = Vector(0, 0, 0);    // Zero out the velocity
        p->next_bouncesound_time = 0; // Init the next bouncesound time

        if (p->cgd.flags & T_TWINKLE) {
            if (random() > 0.5f) {
                p->cgd.flags |= T_TWINKLE_OFF;
            }
        }

        if (p->cgd.flags2 & T2_TRAIL) {
            // Assign a global number to this entity from the
            // command_time_manager Tempmodels with trails need their own unique
            // number.
            p->number = m_command_time_manager.AssignNumber();
            p->cgd.flags |= T_ASSIGNED_NUMBER;
            int oldnum = current_entity_number;
            centity_t* oldcent = current_centity;
            current_centity = NULL;
            current_entity_number = p->number;

            Event* swipeEv = new Event(EV_Client_SwipeOn);
            swipeEv->AddString(p->cgd.swipe_shader);
            swipeEv->AddString(p->cgd.swipe_tag_start);
            swipeEv->AddString(p->cgd.swipe_tag_end);
            swipeEv->AddFloat(p->cgd.swipe_life);

            commandManager.ProcessEvent(swipeEv);

            current_centity = oldcent;
            current_entity_number = oldnum;
        }

        // Check to see if this tiki has any emitters bound to it and update
        // it's number.  This is used for updating emitters that are attached to
        // tempmodels.
        if (p->cgd.tiki) {
            for (i = 1; i <= m_emitters.NumObjects(); i++) {
                spawnthing_t* st = m_emitters.ObjectAt(i);

                if (st->cgd.tiki == p->cgd.tiki) {
                    // Assign this tempmodel a number if he doesn't already have
                    // one
                    if (p->number == -1) {
                        p->number = st->AssignNumber();
                    }

                    st->GetEmitTime(p->number);
                }
            }
        }

        for (i = 0; i < 3; i++) {
            // Randomize avelocity or set absolute
            p->cgd.avelocity[i] =
                m_spawnthing->avelocity_amplitude[i] * random() +
                m_spawnthing->avelocity_base[i];

            // Randomize angles or set absolute
            p->cgd.angles[i] += random() * m_spawnthing->angles_amplitude[i];
        }

        // If forward velocity is set, just use that otherwise use random
        // variation of the velocity
        if (m_spawnthing->forwardVelocity != 0) {
            for (i = 0; i < 3; i++) {
                p->cgd.velocity[i] = newForward[i] *
                                     m_spawnthing->forwardVelocity *
                                     current_entity_scale;
            }
        }

        if (p->cgd.flags2 & T2_PARALLEL) {
            Vector v1 = p->cgd.origin - cg.refdef.vieworg;
            vectoangles(v1, p->cgd.angles);
        }

        AnglesToAxis(p->cgd.angles, m_spawnthing->axis);
        AxisCopy(m_spawnthing->axis, p->ent.axis);

        // If align flag is set, adjust the angles to the direction of velocity
        if (p->cgd.flags & (T_ALIGN | T_ALIGNONCE)) {
            p->cgd.angles = p->cgd.velocity.toAngles();
        }

        // Random offsets along axis
        for (i = 0; i < 3; i++) {
            if (p->cgd.flags2 & T2_PARALLEL) {
                p->cgd.origin +=
                    (Vector(m_spawnthing->axis[i]) *
                         m_spawnthing->axis_offset_amplitude[i] * random() +
                     m_spawnthing->axis_offset_base) *
                    current_entity_scale;
            } else {
                p->cgd.origin +=
                    (Vector(m_spawnthing->tag_axis[i]) *
                         m_spawnthing->axis_offset_amplitude[i] * random() +
                     m_spawnthing->axis_offset_base) *
                    current_entity_scale;
            }
        }

        // Calculate one tick of velocity based on time alive ( passed in )
        p->cgd.origin =
            p->cgd.origin + (p->cgd.velocity * ((float)p->aliveTime / 1000.0f) *
                             current_entity_scale);

        if (p->cgd.flags & T_AUTOCALCLIFE) {
            Vector end, delta;
            float length, speed;
            vec3_t axis[3];
            trace_t trace;

            AnglesToAxis(p->cgd.angles, axis);

            end = p->cgd.origin + Vector(axis[0]) * MAP_SIZE;
            CG_Trace(&trace, p->cgd.origin, vec_zero, vec_zero, end,
                     ENTITYNUM_NONE, CONTENTS_SOLID | CONTENTS_WATER, qfalse,
                     qfalse, "AutoCalcLife");

            delta = trace.endpos - p->cgd.origin;
            length = delta.length();
            speed = p->cgd.velocity.length();

            p->cgd.life = (length / speed) * 1000.0f;
        }

        // global fading is based on the number of animations in the
        // current_entity's animation
        if (current_entity) {
            if (m_spawnthing->cgd.flags & (T_GLOBALFADEIN | T_GLOBALFADEOUT)) {
                int numframes = cgi.Anim_NumFrames(current_tiki,
                                                   current_entity->frameInfo[0].index);

                p->cgd.alpha = (float)current_entity->wasframe / (float)numframes;

                if (m_spawnthing->cgd.flags & T_GLOBALFADEOUT) {
                    p->cgd.alpha = 1.0f - p->cgd.alpha;
                }
            }
        }
    }
}

//===============
// EmitterOn
//===============
void ClientGameCommandManager::EmitterOn(Event* ev)

{
    int i;
    str name;

    name = ev->GetString(1);

    for (i = 1; i <= m_emitters.NumObjects(); i++) {
        spawnthing_t* st = m_emitters.ObjectAt(i);

        if (st->emittername == name) {
            emittertime_t* et;

            et = st->GetEmitTime(current_entity_number);

            et->active = qtrue;
            et->last_emit_time = cg.time;
            et->lerp_emitter = qfalse;
        }
    }
}

//===============
// EmitterOff
//===============
void ClientGameCommandManager::EmitterOff(Event* ev)

{
    int i;
    str name;

    name = ev->GetString(1);

    for (i = 1; i <= m_emitters.NumObjects(); i++) {
        spawnthing_t* st = m_emitters.ObjectAt(i);

        if (st->emittername == name) {
            emittertime_t* et;

            et = st->GetEmitTime(current_entity_number);
            et->active = qfalse;

            if (st->cgd.flags & T_BEAMTHING) {
                // Kill any active beams emitted from this thing
                CG_KillBeams(current_entity_number);
            }
        }
    }
}

//===============
// Sound
//===============
void ClientGameCommandManager::PlaySound(str sound_name, vec3_t origin,
                                         int channel, float volume,
                                         float min_distance,
                                         float pitch, int argstype)

{
    int aliaschannel;
    float aliasvolume;
    float aliasmin_distance;
    float aliaspitch;
    const char* name = NULL;
    static cvar_t* g_subtitle = cgi.Cvar_Get("g_subtitle", "0", CVAR_ARCHIVE);
    static cvar_t* debugSound = cgi.Cvar_Get("debugSound", "0", 0);
    float maxDist;
    AliasListNode_t* soundAlias = NULL;

    // FIXME
    // TODO: Pitch modulation
    // TODO: Alias enhancement

    // Get the real sound to play
    if (current_tiki && current_tiki->a->alias_list) {
        name = cgi.Alias_ListFindRandom((AliasList_t*)current_tiki->a->alias_list,
                                        sound_name.c_str(), &soundAlias);
    }

    if (!name) {
        name = cgi.Alias_FindRandom(sound_name.c_str(), &soundAlias);
    }

    if (!name || !soundAlias) {
        Com_Printf("\nERROR PlaySound: %s needs an alias in ubersound.scr or uberdialog.scr - Please fix.\n", sound_name.c_str());
        return;
    }

    aliasvolume = soundAlias->volumeMod * random() + soundAlias->volume;
    aliaspitch = soundAlias->pitchMod * random() + soundAlias->pitch;
    aliasmin_distance = soundAlias->dist;
    aliaschannel = soundAlias->channel;
    maxDist = soundAlias->maxDist;

    if (soundAlias->subtitle)
    {
        if (g_subtitle->integer
            || !sound_name.icmpn("den", 3)
            || !sound_name.icmpn("snd_den", 6))
        {
            // show subtitles if near the sound
            if (origin
                && (g_subtitle->integer == 2 || DistanceSquared(origin, cg.refdef.vieworg) < (maxDist * maxDist)))
            {
                int curSub;

                // also put subtitles if it's from a german dialogue
                curSub = cgi.Cvar_Get("curSubtitle", "0", 0)->integer;
                cgi.Cvar_Set(va("subtitle%d", curSub), va("%s", soundAlias->subtitle));
                // increment the subtitle
                cgi.Cvar_Set("curSubtitle", va("%d", (curSub + 1) % 4));
            }
        }
    }

    if (argstype)
    {
        if (argstype == 1)
        {
            if (debugSound->integer) {
                Com_Printf("WARNING: Sound %s had its parm modified by code.\n", sound_name.c_str());
            }

            if (volume >= 0.0) {
                volume = volume * aliasvolume;
            }
            else {
                volume = aliasvolume;
            }

            if (pitch >= 0.0) {
                pitch = pitch * aliaspitch;
            }
            else {
                pitch = aliaspitch;
            }

            if (min_distance < 0.0) {
                min_distance = aliasmin_distance;
            }

            if (channel < 0) {
                channel = aliaschannel;
            }
        }
        else
        {
            if (debugSound->integer) {
                Com_Printf("\nWARNING: OVERRIDE OVERRIDE OVERRIDESound %s had all its parm overriden by code.\n\n", sound_name.c_str());
            }

            if (volume < 0.0) {
                volume = aliasvolume;
            }

            if (pitch < 0.0) {
                pitch = aliaspitch;
            }

            if (min_distance < 0.0) {
                min_distance = aliasmin_distance;
            }

            if (channel < 0) {
                channel = aliaschannel;
            }
        }
    }
    else {
        volume = aliasvolume;
        pitch = aliaspitch;
        min_distance = aliasmin_distance;
        channel = aliaschannel;
    }

    if (current_entity_number == -1) {
        cgi.S_StartSound(origin, ENTITYNUM_NONE, channel,
                         cgi.S_RegisterSound(name, soundAlias->streamed),
                         volume, min_distance, pitch, soundAlias->maxDist,
                         soundAlias->streamed);
    }
    else {
        cgi.S_StartSound(origin, current_entity_number, channel,
            cgi.S_RegisterSound(name, soundAlias->streamed),
            volume, min_distance, pitch, soundAlias->maxDist,
            soundAlias->streamed);
    }
}

//===============
// Sound
//===============
void ClientGameCommandManager::Sound(Event* ev)

{
    int channel = CHAN_AUTO;
    str sound_name;
    float volume = -1.0;
    float min_distance = -1.0;

    if (ev->NumArgs() < 1) {
        return;
    }

    // Get all of the parameters

    sound_name = ev->GetString(1);

    if (ev->NumArgs() > 1) {
        channel = ev->GetInteger(2);
    }

    if (ev->NumArgs() > 2) {
        volume = ev->GetFloat(3);
    }

    if (ev->NumArgs() > 3) {
        min_distance = ev->GetFloat(4);
    }

    // play the sound
    if (current_entity) {
        PlaySound(sound_name, current_entity->origin, channel, volume,
                  min_distance);
    } else {
        PlaySound(sound_name, NULL, channel, volume, min_distance);
    }
}

//===============
// StopSound
//===============
void ClientGameCommandManager::StopSound(Event* ev)

{
    int channel;

    if (ev->NumArgs() > 0) {
        channel = ev->GetInteger(1);
    } else {
        return;
    }

    cgi.S_StopSound(current_entity_number, channel);
}

//===============
// LoopSound
//===============
void ClientGameCommandManager::LoopSound(Event* ev)

{
    str sound_name;
    float volume = -1.0;
    float min_dist = -1.0;
    float max_dist = -1.0;
    float pitch = 1.0;
    const char* name = NULL;
    AliasListNode_t* soundAlias = NULL;

    if (!current_centity) {
        cgi.DPrintf(
            "CCM::LoopSound : LoopSound in %s without current_centity\n",
            cgi.TIKI_Name(current_tiki));
        return;
    }

    if (ev->NumArgs() < 1) {
        return;
    }

    sound_name = ev->GetString(1);

    if (ev->NumArgs() > 1) {
        volume = ev->GetFloat(2);
    }

    if (ev->NumArgs() > 2) {
        min_dist = ev->GetFloat(3);
    }

    if (ev->NumArgs() > 3) {
        max_dist = ev->GetFloat(4);
    }

    if (ev->NumArgs() > 4) {
        pitch = ev->GetFloat(5);
    }

    // Get the real sound to play

    if (current_tiki) {
        name = cgi.Alias_ListFindRandom((AliasList_t*)current_tiki->a->alias_list,
                                        sound_name.c_str(), &soundAlias);
    }

    if (!name) {
        name = cgi.Alias_FindRandom(sound_name.c_str(), &soundAlias);
    }

    if (!name) {
        name = sound_name.c_str();
    }

    current_centity->tikiLoopSound = cgi.S_RegisterSound(name, qfalse);
    current_centity->tikiLoopSoundVolume = volume;
    current_centity->tikiLoopSoundMinDist = min_dist;
    current_centity->tikiLoopSoundMaxDist = max_dist;
    current_centity->tikiLoopSoundPitch = pitch;
    current_centity->tikiLoopSoundFlags = 0;
}

//===============
// CacheResource
//===============
void CacheResource(const char* stuff)

{
    str real_stuff;

    if (!stuff) {
        return;
    }

    real_stuff = stuff;
    real_stuff.tolower();

    if (strstr(real_stuff.c_str(), ".wav")) {
        cgi.S_RegisterSound(real_stuff.c_str(), qfalse);
    } else if (strstr(real_stuff.c_str(), ".mp3")) {
        cgi.S_RegisterSound(real_stuff.c_str(), qfalse);
    } else if (strstr(real_stuff.c_str(), ".tik")) {
        cgi.R_RegisterServerModel(real_stuff.c_str());
    } else if (strstr(real_stuff.c_str(), ".spr")) {
        cgi.R_RegisterShader(real_stuff.c_str());
    }
}

//===============
// Cache
//===============
void ClientGameCommandManager::Cache(Event* ev)

{
    if (ev->NumArgs() < 1) {
        return;
    }

    CacheResource(ev->GetString(1));
}

//===============
// AliasResource
//===============
void AliasResource(dtiki_t* pmdl, const char* alias, const char* realname,
                   const char* parameters)
{
    if (pmdl) {
        if (!pmdl->a->alias_list) {
            pmdl->a->alias_list = cgi.AliasList_New(pmdl->a->name);
        }

        cgi.Alias_ListAdd((AliasList_t*)pmdl->a->alias_list, alias, realname, parameters);
    } else {
        cgi.Alias_Add(alias, realname, parameters);
    }
}

//===============
// AliasCache
//===============
void ClientGameCommandManager::AliasCache(Event* ev)

{
    int i;
    char parmbuffer[512]; // this holds the parameters to be passed into the
                          // alias command

    if (ev->NumArgs() < 2) {
        return;
    }

    // alias is in argument 1
    // real path is argument 2
    // any additional parameters are in arguments 3-n

    parmbuffer[0] = 0;

    for (i = 3; i <= ev->NumArgs(); i++) {
        strcat(parmbuffer, ev->GetToken(i));
        strcat(parmbuffer, " ");
    }

    AliasResource(current_tiki, ev->GetString(1), ev->GetString(2), parmbuffer);
    CacheResource(ev->GetString(2));
}

//===============
// Alias
//===============
void ClientGameCommandManager::Alias(Event* ev)

{
    int i;
    char parmbuffer[512]; // this holds the parameters to be passed into the
                          // alias command

    if (ev->NumArgs() < 2) {
        return;
    }

    // alias is in argument 1
    // real path is argument 2
    // any additional parameters are in arguments 3-n

    parmbuffer[0] = 0;

    for (i = 3; i <= ev->NumArgs(); i++) {
        strcat(parmbuffer, ev->GetToken(i));
        strcat(parmbuffer, " ");
    }

    AliasResource(current_tiki, ev->GetString(1), ev->GetString(2), parmbuffer);
}

//===============
// Footstep
//===============
void ClientGameCommandManager::Footstep(Event* ev)
{
    float volume;

    if (ev->NumArgs() > 0) {
        volume = ev->GetFloat(1);
    } else {
        volume = 1.0f;
    }

    if (current_centity && current_entity) {
        // FIXME: unimplemented
        //CG_Footstep(current_centity, current_entity, volume);
    }
}

//===============
// Client
//===============
void ClientGameCommandManager::Client(Event* ev)

{
    Event* event;
    const char* eventname;
    int i;

    // see if it was a dummy command
    if (ev->NumArgs() < 1) {
        return;
    }

    eventname = ev->GetString(1);
    event = new Event(eventname);

    for (i = 2; i <= ev->NumArgs(); i++) {
        event->AddToken(ev->GetToken(i));
    }
    ProcessEvent(event);
}

//===============
// TagDynamicLight
//===============
void ClientGameCommandManager::TagDynamicLight(Event* ev)
{
    str tagname;
    int tagnum;

    // Spawn a single tempmodel that is a dynamic light
    m_spawnthing = InitializeSpawnthing(&m_localemitter);

    tagname = ev->GetString(1);
    tagnum = cgi.Tag_NumForName(current_tiki, tagname.c_str());
    if (tagnum == -1) {
        throw ScriptException("Tagname '%s' does not exist", tagname.c_str());
    }

    GetOrientation(tagnum, m_spawnthing);

    m_spawnthing->cgd.flags |= T_DLIGHT;
    m_spawnthing->cgd.color[0] = ev->GetFloat(2) * 255;
    m_spawnthing->cgd.color[1] = ev->GetFloat(3) * 255;
    m_spawnthing->cgd.color[2] = ev->GetFloat(4) * 255;
    m_spawnthing->cgd.color[3] = 255;
    m_spawnthing->cgd.lightIntensity = ev->GetFloat(5);
    m_spawnthing->cgd.life = ev->GetFloat(6) * 1000;
    if (ev->NumArgs() > 6) {
        m_spawnthing->cgd.lightType |= DLightNameToNum(ev->GetString(7));
        if (ev->NumArgs() > 7) {
            m_spawnthing->cgd.lightType |= DLightNameToNum(ev->GetString(8));
        }
    } else {
        m_spawnthing->cgd.lightType = 0;
    }
    SpawnTempModel(1, m_spawnthing);
}

//===============
// OriginDynamicLight
//===============
void ClientGameCommandManager::OriginDynamicLight(Event* ev)

{
    str tagname;

    if (!current_entity) {
        cgi.DPrintf("ClientGameCommandManager::OriginDynamicLight : Illegal "
                    "use of \"origindlight\"\n");
        return;
    }

    // Spawn a single tempmodel that is a dynamic light
    m_spawnthing = InitializeSpawnthing(&m_localemitter);

    m_spawnthing->cgd.origin = current_entity->origin;
    m_spawnthing->cgd.flags |= T_DLIGHT;
    m_spawnthing->cgd.color[0] = ev->GetFloat(1) * 255;
    m_spawnthing->cgd.color[1] = ev->GetFloat(2) * 255;
    m_spawnthing->cgd.color[2] = ev->GetFloat(3) * 255;
    m_spawnthing->cgd.color[3] = 255;
    m_spawnthing->cgd.lightIntensity = ev->GetFloat(4);
    m_spawnthing->cgd.life = ev->GetFloat(5) * 1000;
    if (ev->NumArgs() > 5) {
        m_spawnthing->cgd.lightType |= DLightNameToNum(ev->GetString(6));
        if (ev->NumArgs() > 6) {
            m_spawnthing->cgd.lightType |= DLightNameToNum(ev->GetString(7));
        }
    } else {
        m_spawnthing->cgd.lightType = 0;
    }
    SpawnTempModel(1, m_spawnthing);
}

//===============
// DynamicLight
//===============
void ClientGameCommandManager::DynamicLight(Event* ev)

{
    str tagname;

    // The emitter itself has a dynamic light
    m_spawnthing->cgd.flags |= T_DLIGHT;
    m_spawnthing->dcolor[0] = ev->GetFloat(1);
    m_spawnthing->dcolor[1] = ev->GetFloat(2);
    m_spawnthing->dcolor[2] = ev->GetFloat(3);
    m_spawnthing->cgd.lightIntensity = ev->GetFloat(4);
    if (ev->NumArgs() > 4) {
        m_spawnthing->cgd.lightType |= (dlighttype_t)ev->GetInteger(5);
        if (ev->NumArgs() > 5) {
            m_spawnthing->cgd.lightType |= DLightNameToNum(ev->GetString(6));
        }
    } else {
        m_spawnthing->cgd.lightType = 0;
    }
}

static int emittercount = 0;

//===============
// UpdateBeam
//===============
void ClientGameCommandManager::UpdateBeam(dtiki_t* tiki, int entity_number,
                                          spawnthing_t* beamthing)
{
    Vector start, end;
    int renderfx;
    qboolean addstartpoint;
    float scale, alpha;
    emittertime_t* et;

    et = beamthing->GetEmitTime(entity_number);

    if (!et->active) {
        return;
    }

    if (current_entity) {
        renderfx = (current_entity->renderfx &
                    ~(RF_FLAGS_NOT_INHERITED | RF_LIGHTING_ORIGIN));
        scale = current_entity->scale;
        alpha = (float)current_entity->shaderRGBA[3] / 255.0f;
    } else {
        renderfx = 0;
        scale = 1.0f;
        alpha = 1.0f;
    }

    if (beamthing->startTag == "USE_TAGLIST") {
        int i, count;
        str startTag, endTag;
        int tagnum;
        Vector dir;

        count = beamthing->m_taglist.NumObjects();

        if (!count) {
            return;
        }

        startTag = beamthing->m_taglist.ObjectAt(1);
        tagnum = cgi.Tag_NumForName(current_tiki, startTag.c_str());
        if (tagnum == -1) {
            throw ScriptException("Tagname '%s' does not exist",
                                  startTag.c_str());
        }

        GetOrientation(tagnum, beamthing);
        start = beamthing->cgd.origin;
        if (beamthing->beamflags & BEAM_OFFSET_ENDPOINTS) {
            dir = Vector(crandom(), crandom(), crandom());
            start += (crandom() * beamthing->min_offset * dir) +
                     (crandom() * beamthing->max_offset * dir);
        }

        CG_MultiBeamBegin();
        addstartpoint = qtrue;

        for (i = 2; i <= count; i++) {
            str s;

            s = va("%d", i);

            endTag = beamthing->m_taglist.ObjectAt(i);
            tagnum = cgi.Tag_NumForName(current_tiki, endTag.c_str());
            if (tagnum == -1) {
                throw ScriptException("Tagname '%s' does not exist",
                                      endTag.c_str());
            }

            GetOrientation(tagnum, beamthing);
            end = beamthing->cgd.origin;

            if (beamthing->beamflags & BEAM_OFFSET_ENDPOINTS) {
                dir = Vector(crandom(), crandom(), crandom());
                end += (crandom() * beamthing->min_offset * dir) +
                       (crandom() * beamthing->max_offset * dir);
            }

            CG_MultiBeamAddPoints(start, end, beamthing->numSubdivisions,
                                  beamthing->beamflags, beamthing->min_offset,
                                  beamthing->max_offset, addstartpoint);

            addstartpoint = qfalse;
            startTag = endTag;
            start = end;
        }

        byte newcolor[4];

        newcolor[0] = beamthing->cgd.color[0] * alpha;
        newcolor[1] = beamthing->cgd.color[1] * alpha;
        newcolor[2] = beamthing->cgd.color[2] * alpha;
        newcolor[3] = beamthing->cgd.color[3] * alpha;

        CG_MultiBeamEnd(
            beamthing->cgd.scale * scale, renderfx, beamthing->cgd.shadername,
            newcolor, beamthing->beamflags, entity_number, beamthing->cgd.life);
    } else {
        if (beamthing->startTag.length()) {
            int tagnum = cgi.Tag_NumForName(current_tiki, beamthing->startTag.c_str());
            if (tagnum == -1) {
                throw ScriptException("Tagname '%s' does not exist",
                                      beamthing->startTag.c_str());
            }

            // Use a dummy sp to get the orientation
            GetOrientation(tagnum, beamthing);
            start = beamthing->cgd.origin;
        } else {
            start = beamthing->cgd.origin;
        }

        if (beamthing->endTag.length()) {
            int tagnum = cgi.Tag_NumForName(current_tiki, beamthing->endTag.c_str());
            if (tagnum == -1) {
                throw ScriptException("Tagname '%s' does not exist",
                                      beamthing->endTag.c_str());
            }

            // Use a dummy sp to get the orientation
            GetOrientation(tagnum, beamthing);
            end = beamthing->cgd.origin;
        } else {
            end = start + Vector(beamthing->axis[0]) * beamthing->length;
        }

        CG_CreateBeam(start, vec_zero, entity_number, 1,
                      beamthing->cgd.alpha * alpha,
                      beamthing->cgd.scale * scale,
                      (beamthing->beamflags | BEAM_LIGHTNING_EFFECT),
                      beamthing->length, beamthing->cgd.life, qfalse, end,
                      beamthing->min_offset, beamthing->max_offset,
                      beamthing->overlap, beamthing->numSubdivisions,
                      beamthing->delay, beamthing->cgd.shadername,
                      beamthing->cgd.color, beamthing->numspherebeams,
                      beamthing->sphereRadius, beamthing->toggledelay,
                      beamthing->endalpha, renderfx, beamthing->emittername);
    }
}

//===============
// UpdateEmitter
//===============
void ClientGameCommandManager::UpdateEmitter(dtiki_t* tiki, vec3_t axis[3],
                                             int entity_number,
                                             int parent_number,
                                             Vector entity_origin)

{
    int parent, lastparent, i;
    emittertime_t* et = NULL;
    centity_t* pc;
    int count = 0;
    float scale = 1.0f;

    if (current_entity) {
        scale = current_entity->scale;
    }

    // Find the emitter associated with this model
    for (i = 1; i <= m_emitters.NumObjects(); i++) {
        m_spawnthing = m_emitters.ObjectAt(i);

        if (m_spawnthing->cgd.tiki != tiki) {
            continue;
        }

        et = m_spawnthing->GetEmitTime(entity_number);

        if (!et->active) {
            continue;
        }

        if ((m_spawnthing->cgd.flags & T_DETAIL) && !cg_detail->integer) {
            et->last_emit_time = 0;
            continue;
        }

        // Set the default origin (spawn from the parent's origin)
        VectorCopy(entity_origin, m_spawnthing->cgd.origin);

        if (m_spawnthing->cgd.flags & T_BEAMTHING) {
            if (m_spawnthing->cgd.flags & T_ANGLES) {
                vec3_t axis[3];

                AnglesToAxis(m_spawnthing->cgd.angles, axis);
                AxisCopy(axis, m_spawnthing->axis);
            }

            UpdateBeam(tiki, entity_number, m_spawnthing);
            continue;
        }
        if (m_spawnthing->tagnum == -1) {
            // Set the axis and origin based on the tag's axis and origin
            GetOrientation(m_spawnthing->tagnum, m_spawnthing);
        } else if (axis) {
            // Set the axis based on the entity's axis.
            AxisCopy(axis, m_spawnthing->axis);
        } else {
            AxisClear(m_spawnthing->axis);
        }

        // If we are HARDLINKed then subtract off the parent's origin
        if (m_spawnthing->cgd.flags & T_HARDLINK) {
            m_spawnthing->cgd.parent = entity_number;
            m_spawnthing->cgd.origin -= entity_origin;
        } else if (m_spawnthing->cgd.flags & T_PARENTLINK) {
            lastparent = ENTITYNUM_NONE;

            parent = parent_number;

            // Find the topmost parent
            while (parent != ENTITYNUM_NONE) {
                pc = &cg_entities[parent];
                lastparent = parent;
                parent = pc->currentState.parent;
            }

            // The lastparent is the "real" parent
            if (lastparent != ENTITYNUM_NONE) {
                pc = &cg_entities[lastparent];

                m_spawnthing->cgd.origin -= pc->lerpOrigin;
                m_spawnthing->cgd.parent = lastparent;
            }
        }

        m_spawnthing->cgd.createTime = cg.time;
        m_spawnthing->cgd.parentOrigin = Vector(entity_origin);

        if (m_spawnthing->cgd.flags & T_DLIGHT) {
            cgi.R_AddLightToScene(
                m_spawnthing->cgd.origin,
                m_spawnthing->cgd.lightIntensity * scale,
                m_spawnthing->dcolor[0], m_spawnthing->dcolor[1],
                m_spawnthing->dcolor[2], m_spawnthing->cgd.lightType);
            continue;
        }

        Vector save_origin = m_spawnthing->cgd.origin;
        Vector delta = m_spawnthing->cgd.origin - et->oldorigin;

        if (m_spawnthing->spawnRate < 0) {
            warning("ClientGameCommandManager::UpdateEmitter",
                    "Invalid spawnrate (negative)");
            continue;
        }

        if (paused->integer) {
            et->last_emit_time = 0;
        } else if ((et->last_emit_time > 0) && (m_spawnthing->spawnRate)) {
            int dtime = cg.time - et->last_emit_time;
            float lerp, lerpfrac;

            if (et->last_emit_time > cg.time) {
                et->last_emit_time = cg.time;
            }

            count = dtime /
                    (m_spawnthing->spawnRate * (1.0f / cg_effectdetail->value));

            // This is kind of a nasty bit of code.  If the count is 1, just
            // spawn a single tempmodel, if it's greater than 1, then spawn the
            // number of tempmodels over a period of time, and adjust their
            // create times accordingly.  Also lerp the origins so they appear
            // where they are supposed to.  This helps smoothing out low frame
            // rate situations where this is only get called a few times a
            // second, but the spawn rate is high, and it will look a lot
            // smoother.
            if (!count) {
                continue;
            } else if (count == 1) {
                SpawnTempModel(1);
                et->last_emit_time = cg.time;
            } else {
                lerpfrac = 1.0f / (float)count;

                lerp = 0;
                while (dtime > (m_spawnthing->spawnRate *
                                (1.0f / cg_effectdetail->value))) {
                    et->last_emit_time = cg.time;

                    dtime -= m_spawnthing->spawnRate *
                             (1.0f / cg_effectdetail->value);

                    if (et->lerp_emitter) {
                        m_spawnthing->cgd.origin =
                            et->oldorigin + (delta * lerp);
                    }

                    SpawnTempModel(1, dtime);

                    lerp += lerpfrac;
                }
            }
        } else {
            et->last_emit_time = cg.time;
        }

        if (cg_showemitters->integer) {
            if (count) {
                cgi.DPrintf("%d:%s:%d tempmodels emitted\n", entity_number,
                            m_spawnthing->emittername.c_str(), count);
            }
        }

        // Since we have an oldorigin, now we can do lerping
        m_spawnthing->cgd.origin = save_origin;
        et->oldorigin = save_origin;
        et->lerp_emitter = qtrue;
    }
}

//===============
// RemoveClientEntity - Removes an entity from being
// updated if it has emitters attached
//===============
void ClientGameCommandManager::RemoveClientEntity(int number, dtiki_t* tiki,
                                                  centity_t* cent,
                                                  ctempmodel_t* p)

{
    int i;

    // Kill the emitters if they are attached to this tiki
    if (number != -1) {
        for (i = 1; i <= m_emitters.NumObjects(); i++) {
            spawnthing_t* st = m_emitters.ObjectAt(i);

            if (st->cgd.tiki == tiki) {
                st->RemoveEntity(number);
            }
        }
    }

    // Turn off any swiping for this entity number
    int oldnum = current_entity_number;
    centity_t* oldcent = current_centity;

    current_centity = cent;
    current_entity_number = number;
    ProcessEvent(EV_Client_SwipeOff);
    current_entity_number = oldnum;
    current_centity = oldcent;

    // If this tempmodel was assigned a temporary number then remove it from the
    // command_time_manager
    if (p && p->cgd.flags & T_ASSIGNED_NUMBER) {
        m_command_time_manager.RemoveEntity(number);
    }
}

//===============
// FreeAllTempModels
//===============
void ClientGameCommandManager::FreeAllTempModels(void)

{
    ctempmodel_t *p, *next;

    // Go through all the temp models and run the physics if necessary,
    // then add them to the ref
    p = m_active_tempmodels.prev;
    for (; p != &m_active_tempmodels; p = next) {
        // grab next now, so if the local entity is freed we still have it
        next = p->prev;
        FreeTempModel(p);
    }
}

//===============
// FreeAllEmitters
//===============
void ClientGameCommandManager::FreeAllEmitters(void)

{
    m_emitters.ClearObjectList();
}

//===============
// CG_RestartCommandManager
//===============
void CG_RestartCommandManager()

{
    commandManager.FreeAllTempModels();
}

//===============
// CG_FlushCommandManager
//===============
void CG_FlushCommandManager(void)

{
    commandManager.FreeAllTempModels();
    commandManager.FreeAllEmitters();
}

//=================
// CG_ProcessInitCommands
//=================
void CG_ProcessInitCommands(dtiki_t* tiki, refEntity_t* ent)
{
    // FIXME: unimplemented
}

//=================
// CG_ProcessCacheInitCommands
//=================
void CG_ProcessCacheInitCommands(dtiki_t* tiki)
{
    // FIXME: unimplemented
}

void CG_EndTiki(dtiki_t* tiki)
{
    CG_DeleteEmitters(tiki);
}

//=================
// CG_UpdateEntity
//=================
void CG_UpdateEntityEmitters(int entnum, refEntity_t* ent, centity_t* cent)
{
    refEntity_t* old_entity = current_entity;
    dtiki_t* old_tiki = current_tiki;
    int oldnum = current_entity_number;
    centity_t* oldcent = current_centity;

    // Check to see if this model has any emitters
    current_entity = ent;
    current_tiki = ent->tiki;
    current_centity = cent;
    current_entity_number = entnum;

    if (cent->currentState.parent != ENTITYNUM_NONE) {
        commandManager.UpdateEmitter(ent->tiki, ent->axis,
                                     cent->currentState.number,
                                     cent->currentState.parent, ent->origin);
    } else {
        commandManager.UpdateEmitter(
            ent->tiki, ent->axis, cent->currentState.number,
            cent->currentState.parent, cent->lerpOrigin);
    }
    current_entity = old_entity;
    current_tiki = old_tiki;
    current_centity = oldcent;
    current_entity_number = oldnum;
}

extern "C" {
void CG_AnimationDebugMessage(int number, const char* fmt, ...);
}

//=================
// CG_ProcessEntityCommands
//=================
qboolean CG_ProcessEntityCommands(int frame, int anim, int entnum, refEntity_t* ent, centity_t* cent)
{
    int i, j;
    int num_args;

    tiki_cmd_t tikicmds;

    if (cgi.Frame_Commands(ent->tiki, anim, frame, &tikicmds)) {
        current_entity = ent;
        current_centity = cent;
        current_entity_number = entnum;
        current_tiki = ent->tiki;
        CG_AnimationDebugMessage(
            entnum,
            "Processing Ent Commands: Entity: %3d Anim:#(%i) Frame:#(%i)\n",
            anim, frame);

        for (i = 0; i < tikicmds.num_cmds; i++) {
            Event* ev;

            num_args = tikicmds.cmds[i].num_args;

            if (num_args > 0) {
                // Create the event and Process it.
                ev = new Event(tikicmds.cmds[i].args[0]);

                for (j = 1; j < num_args; j++) {
                    ev->AddToken(tikicmds.cmds[i].args[j]);
                }

                commandManager.ProcessEvent(ev);
            }
        }

        current_tiki = NULL;
        current_entity_number = -1;
        current_entity = NULL;
        current_centity = NULL;
    }

    return qtrue;
}

//=================
// CG_ClientCommandDebugMessage
//=================
void CG_ClientCommandDebugMessage(centity_t* cent, const char* fmt, ...)
{
#ifndef NDEBUG
    if (cg_debugAnim->integer) {
        va_list argptr;
        char msg[1024];

        va_start(argptr, fmt);
        vsprintf(msg, fmt, argptr);
        va_end(argptr);

        if ((!cg_debugAnimWatch->integer) ||
            ((cg_debugAnimWatch->integer - 1) == cent->currentState.number)) {
            if (cg_debugAnim->integer == 2) {
                cgi.DebugPrintf(msg);
            } else {
                cgi.Printf(msg);
            }
        }
    }
#endif
}

//=================
// CG_ClientCommands
//=================
void CG_ClientCommands(refEntity_t* ent, centity_t* cent, int slot)
{
#if 0
    int anim;
    int frame;

    if (!tiki) {
        return;
    }

    if (paused->integer) {
        return;
    }

    if (cent->currentState.eFlags & EF_DONT_PROCESS_COMMANDS) {
        return;
    }

    assert(cent);

    // don't do anything if the frame is illegal
    if ((new_frame < 0) || (new_frame >= state->numframes)) {
        return;
    }

#if 0
#ifndef NDEBUG 
   CG_ClientCommandDebugMessage( 
      cent, 
      "Client Commands: cg.time %d checking Entity %d anim %d frame %d\n", 
      cg.time,
      cent->currentState.number,
      new_anim, 
      new_frame
      );
#endif
#endif

    anim = state->last_cmd_anim - 1;
    frame = state->last_cmd_frame;

    // if we had a last anim and it it wasn't same as ours then
    // we need to run any exit commands from the last animation
    if ((anim != new_anim) && (anim >= 0)) {
        // play the exit command
        CG_ProcessEntityCommands(TIKI_FRAME_EXIT, anim,
                                 cent->currentState.number, ent, cent);
#ifndef NDEBUG
        CG_ClientCommandDebugMessage(
            cent, "Client Commands: Entity %d Exiting Anim: %s\n",
            cent->currentState.number, cgi.Anim_NameForNum(tiki, anim));
#endif
        frame = 0;

        // Reset the tiki looping sound if changing animation
        cent->tikiLoopSound = NULL;
    }

    if (state->has_commands) {
        // if we are entering a new animation, than
        // we need to run any entry commands for the new animation
        if (anim != new_anim) {
            // play the exit command
            CG_ProcessEntityCommands(TIKI_FRAME_ENTRY, new_anim,
                                     cent->currentState.number, ent, cent);
#ifndef NDEBUG
            CG_ClientCommandDebugMessage(
                cent, "Client Commands: Entity %d Entering Anim: %s\n",
                cent->currentState.number,
                cgi.Anim_NameForNum(tiki, new_anim));
#endif
        }

        if (state->driven) {
            // make sure we process this frame if we are on a new frame
            // although this is a hack, it guarantees that the frame will be
            // processed below.
            frame %= state->numframes;
            if (frame < new_frame) {
                frame = new_frame;
            }
        } else {
            //
            // we need to catch up on the frames we haven't played yet
            //
            if ((new_frame != (frame - 1)) && (state->numframes > 1)) {
                // and frame number so that it wraps properly
                frame %= state->numframes;

                // lerp to one minus the current frame
                while (frame != new_frame) {
#ifndef NDEBUG
                    CG_ClientCommandDebugMessage(
                        cent,
                        "Client Commands: cg.time %d Catching up Entity: %d "
                        "Anim: %s frame: %d numframes: %d\n",
                        cg.time, cent->currentState.number,
                        cgi.Anim_NameForNum(tiki, new_anim), frame,
                        state->numframes);
#endif
                    state->last_cmd_time =
                        cg.time + TIKI_FRAME_MAXFRAMERATE;
                    CG_ProcessEntityCommands(frame, new_anim,
                                             cent->currentState.number, ent,
                                             cent);
                    frame = (frame + 1) % state->numframes;
                }
            }
        }

        //
        // handle the single frame and every frame case
        //
        if ((frame == new_frame) ||
            ((cg.time > state->last_cmd_time) &&
             (cent->currentState.eFlags & EF_EVERYFRAME))) {
#ifndef NDEBUG
            CG_ClientCommandDebugMessage(
                cent,
                "Client Commands: cg.time %d Processing Entity: %d Anim: %s "
                "frame: %d numframes: %d\n",
                cg.time, cent->currentState.number,
                cgi.Anim_NameForNum(tiki, new_anim), new_frame,
                state->numframes);
#endif
            state->last_cmd_time = cg.time + TIKI_FRAME_MAXFRAMERATE;
            CG_ProcessEntityCommands(new_frame, new_anim,
                                     cent->currentState.number, ent, cent);
        }
    }

    if (cent->clientFlags & CF_UPDATESWIPE) {
        current_entity = ent;
        current_centity = cent;
        current_entity_number = cent->currentState.number;
        current_tiki = tiki;

        commandManager.ProcessEvent(EV_Client_Swipe);

        current_tiki = NULL;
        current_entity_number = -1;
        current_entity = NULL;
        current_centity = NULL;
    }

    state->last_cmd_anim = new_anim + 1;
    state->last_cmd_frame = new_frame + 1;
#endif
}

//===============
// CG_AddTempModels
//===============
void CG_AddTempModels(void) { commandManager.AddTempModels(); }

//===============
// CG_InitializeCommandManager
//===============
void CG_InitializeCommandManager(void)
{
    cg_showemitters = cgi.Cvar_Get("cg_showemitters", "0", 0);
    commandManager.InitializeTempModelCvars();
    commandManager.InitializeVSSCvars();
    commandManager.InitializeTempModels();
    commandManager.InitializeVSSSources();
    commandManager.InitializeEmitters();
    commandManager.InitializeRainCvars();
    commandManager.InitializeBeams();
    CG_InitTestEmitter();
    CG_InitTestTreadMark();
}

void CG_ResetTempModels(void)
{
    commandManager.ResetTempModels();
    lastTempModelFrameTime = cg.time;
}

//===============
// CG_RemoveClientEntity
//===============
void CG_RemoveClientEntity(int number, dtiki_t* tiki, centity_t* cent)

{
    commandManager.RemoveClientEntity(number, tiki, cent);

    // Remove the beam list associated with this entity
    RemoveBeamList(number);
}

//=================
// CG_Command_ProcessFile
//=================
qboolean CG_Command_ProcessFile(const char* filename, qboolean quiet, dtiki_t *curTiki)
{
    char* buffer;
    const char* bufstart;
    char com_token[MAX_STRING_CHARS];

    if (cgi.FS_ReadFile(filename, (void**)&buffer) == -1) {
        return qfalse;
    }

    if (!quiet) {
        cgi.DPrintf("CG_Command_ProcessFile: %s\n", filename);
    }

    // we are not setting up for a tiki
    current_tiki = NULL;

    bufstart = buffer;

    while (1) {
        Event* ev;

        // grab each line as we go
        strcpy(com_token, COM_ParseExt(&buffer, qtrue));
        if (!com_token[0]) {
            break;
        }

        if (!strcmpi(com_token, "end") || !strcmpi(com_token, "server")) {
            // skip the line
            while (1) {
                strcpy(com_token, COM_ParseExt(&buffer, qfalse));
                if (!com_token[0]) {
                    break;
                }
            }
            continue;
        }

        // Create the event
        ev = new Event(com_token);

        // get the rest of the line
        while (1) {
            strcpy(com_token, COM_ParseExt(&buffer, qfalse));
            if (!com_token[0]) {
                break;
            }

            ev->AddToken(com_token);
        }
        commandManager.ProcessEvent(ev);
    }
    cgi.FS_FreeFile((void*)bufstart);

    return qtrue;
}

//===========================
// CG_ConsoleCommand Functions
//===========================
void CG_EventList_f(void)
{
    const char* mask;

    mask = NULL;
    if (cgi.Argc() > 1) {
        mask = cgi.Argv(1);
    }
    Event::ListCommands(mask);
}

void CG_EventHelp_f(void)
{
    const char* mask;

    mask = NULL;
    if (cgi.Argc() > 1) {
        mask = cgi.Argv(1);
    }
    Event::ListDocumentation(mask);
}

void CG_DumpEventHelp_f(void)
{
    const char* mask;

    mask = NULL;
    if (cgi.Argc() > 1) {
        mask = cgi.Argv(1);
    }
    Event::ListDocumentation(mask, true);
}

void CG_PendingEvents_f(void)
{
    const char* mask;

    mask = NULL;
    if (cgi.Argc() > 1) {
        mask = cgi.Argv(1);
    }
    Event::PendingEvents(mask);
}

void CG_ClassList_f(void) { listAllClasses(); }

void CG_ClassTree_f(void)
{
    if (cgi.Argc() > 1) {
        listInheritanceOrder(cgi.Argv(1));
    } else {
        cgi.Printf("Syntax: cg_classtree [classname].\n");
    }
}

void CG_ClassEvents_f(void)
{
    if (cgi.Argc() > 1) {
        ClassEvents(cgi.Argv(1), qfalse);
    } else {
        Com_Printf("Syntax: cg_classevents [classname].\n");
    }
}

void CG_DumpClassEvents_f(void)
{
    if (cgi.Argc() > 1) {
        ClassEvents(cgi.Argv(1), qtrue);
    } else {
        Com_Printf("Syntax: cg_dumpclassevents [classname].\n");
    }
}

void CG_DumpAllClasses_f(void) { DumpAllClasses(); }

#define throw_assert assert(0), throw

EmitterLoader emitterLoader;

Event EV_EmitterLoader_Emitter("emitter", EV_DEFAULT, "s", "emittername",
                               "Create a new emitter");

CLASS_DECLARATION(Listener, EmitterLoader, NULL){
    {&EV_EmitterLoader_Emitter, &EmitterLoader::Emitter},
    {NULL,                      NULL   }
};

EmitterLoader::EmitterLoader() { emitterActive = false; }

void EmitterLoader::Emitter(Event* ev)

{
    spawnthing_t* st;

    st = commandManager.CreateNewEmitter(ev->GetString(1));

    if (st) {
        emitterActive = true;
        commandManager.SetSpawnthing(st);
    }
}

void EmitterLoader::ProcessEmitter(Script& script)

{
    str token;

    while (script.TokenAvailable(true)) {
        Event* ev = NULL;

        token = script.GetToken(true);

        if (!str::cmp("}", token)) {
            break;
        }

        ev = new Event(token);

        while (script.TokenAvailable(false)) {
            ev->AddToken(script.GetToken(false));
        }

        if (emitterActive) {
            commandManager.ProcessEvent(ev);
        }
    }

    commandManager.SetSpawnthing(NULL);
    emitterActive = NULL;
}

bool EmitterLoader::Load(Script& script)

{
    str token;
    str errortext;

    try {
        while (script.TokenAvailable(true)) {
            Event* ev = NULL;

            token = script.GetToken(true);

            if (!str::cmp("{", token)) {
                ProcessEmitter(script);
            } else if (!token.length() || !ValidEvent(token)) {
                throw_assert "invalid token";
            } else {
                ev = new Event(token);

                while (script.TokenAvailable(false)) {
                    ev->AddToken(script.GetToken(false));
                }

                ProcessEvent(ev);
            }
        }
    } catch (const char* s) {
        cgi.DPrintf("emitter: error on line %d: %s\n", script.GetLineNumber(),
                    s);
        return false;
    }

    return true;
}

void CG_Emitter(centity_t* cent)
{
    spawnthing_t* emitter;
    float dtime;
    emittertime_t* et;
    vec3_t mins, maxs;

    emitter = commandManager.GetEmitterByName(
        CG_ConfigString(CS_IMAGES + cent->currentState.tag_num));

    if (emitter) {
        vec3_t axis[3];
        cgi.R_ModelBounds(cgs.inlineDrawModel[cent->currentState.modelindex],
                          mins, maxs);

        emitter->cgd.origin[0] =
            cent->lerpOrigin[0] + mins[0] + (random() * (maxs[0] - mins[0]));
        emitter->cgd.origin[1] =
            cent->lerpOrigin[1] + mins[1] + (random() * (maxs[1] - mins[1]));
        emitter->cgd.origin[2] = cent->lerpOrigin[2] + maxs[2];
        emitter->cgd.parentOrigin = Vector(cent->lerpOrigin);
        emitter->cgd.parentOrigin[2] = cent->lerpOrigin[2] + maxs[2];
        emitter->cgd.parentMins = mins;
        emitter->cgd.parentMaxs = maxs;

        AnglesToAxis(cent->lerpAngles, axis);

        // Update the emitter time and spawn the tempmodels
        et = emitter->GetEmitTime(cent->currentState.number);

        if (et->last_emit_time > 0) {
            dtime = cg.time - et->last_emit_time;
            while (dtime > emitter->spawnRate) {
                dtime -= emitter->spawnRate;
                commandManager.SpawnTempModel(1, emitter);
                et->last_emit_time = cg.time;
            }
        }
    }
}

void ClientGameCommandManager::CGEvent(centity_t* cent)

{
    str modelname;
    dtiki_t *tiki;

    tiki = cgi.R_Model_GetHandle(cgs.model_draw[cent->currentState.modelindex]);

    if (!tiki) {
        return;
    }

    CG_EntityEffects(cent);

    modelname = cgi.TIKI_Name(tiki);
    m_spawnthing = InitializeSpawnthing(&m_localemitter);

    AnglesToAxis(cent->lerpAngles, m_spawnthing->axis);
    m_spawnthing->cgd.angles = cent->lerpAngles;
    m_spawnthing->cgd.origin = cent->lerpOrigin;
    m_spawnthing->cgd.scale = cent->currentState.scale;
    m_spawnthing->cgd.alpha = cent->currentState.alpha;

    m_spawnthing->cgd.color[0] = cent->color[0] * 255;
    m_spawnthing->cgd.color[1] = cent->color[1] * 255;
    m_spawnthing->cgd.color[2] = cent->color[2] * 255;
    m_spawnthing->cgd.color[3] = cent->color[3] * 255;

    Event* ev;
    ev = new Event("model");
    ev->AddString(modelname);
    ProcessEvent(ev);

    ev = new Event("anim");
    ev->AddString("idle");
    ProcessEvent(ev);

    SpawnTempModel(1);
}

qboolean ClientGameCommandManager::SelectProcessEvent(Event* ev)
{
    // FIXME: unimplemented
    return qfalse;
}

void ClientGameCommandManager::AddTreadMarkSources()
{
    // FIXME: unimplemented
}

void ClientGameCommandManager::InitializeTreadMarkCvars()
{
    // FIXME: unimplemented
}

void ClientGameCommandManager::InitializeTreadMarkSources()
{
    // FIXME: unimplemented
}

void ClientGameCommandManager::ResetTreadMarkSources()
{
    // FIXME: unimplemented
}

void ClientGameCommandManager::ResetTreadMarkSources(Event* ev)
{
    // FIXME: unimplemented
}

void ClientGameCommandManager::InitializeBeams()
{
    // FIXME: unimplemented
}

void CG_Event(centity_t* cent) { commandManager.CGEvent(cent); }
