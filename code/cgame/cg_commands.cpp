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
#include "cg_specialfx.h"
#include "scriptexception.h"
#include "tiki.h"
#include "cg_archive.h"
#include "memarchiver.h"

refEntity_t *current_entity        = NULL;
int          current_entity_number = -1;
centity_t   *current_centity       = NULL;
float        current_scale         = -1;
dtiki_t     *current_tiki          = nullptr;
cvar_t      *cg_showemitters;
cvar_t      *cg_physics_fps;

Vector last_trace_end;

void CacheResource(const char *stuff);

ClientGameCommandManager commandManager;

Event EV_Client_StartBlock("(", EV_DEFAULT, NULL, NULL, "Signals the beginning of a block of commands");
Event EV_Client_EndBlock(")", EV_DEFAULT, NULL, NULL, "Signals the end of a block of commands");
Event EV_Client_EmitterStartOff(
    "startoff", EV_DEFAULT, NULL, NULL, "Signals an emitter to start in the off state (no tempmodels are emitted)"
);
Event EV_Client_SetAlpha("alpha", EV_DEFAULT, "f", "alpha", "Set the alpha of the spawned tempmodel");
Event EV_Client_SetDieTouch(
    "dietouch", EV_DEFAULT, NULL, NULL, "Set the spawned tempmodels to die when they touch a solid"
);
Event EV_Client_SetBounceFactor(
    "bouncefactor",
    EV_DEFAULT,
    "f",
    "factor",
    "Set the bounciness of a tempmodel when it hits a solid.\n"
    "A factor > 1 will make the model bounce higher and higher on each hit"
);
Event EV_Client_SetBounceSound(
    "bouncesound",
    EV_DEFAULT,
    "sF",
    "sound [delay]",
    "When bouncing, what sound to play on impact and an option delay (default is 1 second) between playing this sound"
);
Event EV_Client_SetBounceSoundOnce(
    "bouncesoundonce", EV_DEFAULT, "s", "sound", "When bouncing, what sound to play on impact one time"
);
Event EV_Client_SetScale("scale", EV_DEFAULT, "f", "scale", "Set the scale of a spawned tempmodel");
Event EV_Client_SetScaleUpDown(
    "scaleupdown", EV_DEFAULT, NULL, NULL, "Set the tempmodel to scale up to scale value and then down."
);
Event EV_Client_SetScaleMin("scalemin", EV_DEFAULT, "f", "scalemin", "Set the minimum scale of a spawned tempmodel");
Event EV_Client_SetScaleMax("scalemax", EV_DEFAULT, "f", "scalemax", "Set the maximum scale of a spawned tempmodel");
Event EV_Client_SetModel(
    "model",
    EV_DEFAULT,
    "sSSSSS",
    "modelname1 modelname2 modelname3 modelname4 modelname5 modelname6",
    "Set the modelname of the tempmodel.  If more than 1 model is specified, it will\n"
    "be randomly chosen when spawned"
);
Event EV_Client_SetLife("life", EV_DEFAULT, "f", "life", "Set the life (in seconds) of the spawned tempmodel");
Event EV_Client_SetColor(
    "color", EV_DEFAULT, "fffF", "red green blue alpha", "Set the color (modulate) of the spawned tempmodel."
);
Event EV_Client_SetRadialVelocity(
    "radialvelocity",
    EV_DEFAULT,
    "fff",
    "scale min_additional max_additional",
    "Subtracts the particle origin from origin and multiplies by scale, then adds additional velocity\n"
    "between min and max... negative values bring toward origin\n"
);
Event EV_Client_SetVelocity(
    "velocity", EV_DEFAULT, "f", "forwardVelocity", "Set the forward velocity of the spawned tempmodel"
);
Event EV_Client_SetAngularVelocity(
    "avelocity", EV_DEFAULT, "fff", "yawVel pitchVel rollVel", "Set the angular velocity of the spawned tempmodel"
);
Event EV_Client_SetColorVelocity(
    "colorvelocity", EV_DEFAULT, "fff", "rVel gVel bVel", "Set the color velocity of the spawned dlight tempmodel"
);
Event EV_Client_SetRandomVelocity(
    "randvel",
    EV_DEFAULT,
    "SfFSfFSfF",
    "[random|crandom|range] xVel [xVel2] [random|crandom|range] yVel [yVel2] [random|crandom|range] zVel [zVel2]",
    "Add a random component to the regular velocity.\n"
    "If random is specified, the component will range from 0 to specified velocity.\n"
    "If crandom is specified, the component will range from -specified to +specified velocity.\n"
    "If range is specified, the component needs two values; it will randomly pick a number in the range\n"
    "from the first number to the first number plus the second number.\n"
    "If no keyword is explicitly specified, then the component will just be added on\n"
    "without randomness.\n"
    "This velocity is applied using the world axis"
);
Event EV_Client_SetRandomVelocityAlongAxis(
    "randvelaxis",
    EV_DEFAULT,
    "SfFSfFSfF",
    "[random|crandom|range] forwardVel [forwardVel2] [random|crandom|range] rightVel [rightVel2] [random|crandom|rang"
    "e] upVel [upVel2]",
    "Add a random component to the regular velocity.\n"
    "If random is specified, the component will range from 0 to specified velocity.\n"
    "If crandom is specified, the component will range from -specified to +specified velocity.\n"
    "If range is specified, the component needs two values; it will randomly pick a number in the range\n"
    "from the first number to the first number plus the second number.\n"
    "If no keyword is explicitly specified, then the component will just be added on\n"
    "without randomness.\n"
    "This velocity is applied using the parent axis"
);
Event EV_Client_SetAccel(
    "accel",
    EV_DEFAULT,
    "fff",
    "xAcc yAcc zAcc",
    "Set the acceleration of the spawned tempmodel.\n"
    "This acceleration is applied using the world axis"
);
Event EV_Client_SetFriction(
    "friction",
    EV_DEFAULT,
    "f",
    "friction",
    "Set the friction as a fraction of velocity per second... exact effect depends on physics rate:\n"
    "slowdown per second = [1 - (friction / physicsrate)] ^ physicsrate; physicsrate defaults to 10"
);
Event EV_Client_SetSpin(
    "spin",
    EV_DEFAULT,
    "f",
    "rotations_per_second",
    "Sets counterclockwise rotations per second at which the emitter's x/y-axes rotate around its z-axis"
);
Event EV_Client_SetVaryColor(
    "varycolor", EV_DEFAULT, NULL, NULL, "Sets the color to vary by 0 to -20% of specified color"
);
Event EV_Client_SetSmokeParms(
    "smokeparms", EV_DEFAULT, "fff", "typeinfo fademult scalemult", "Sets some misc parms for smoke"
);
Event EV_Client_SetCount(
    "count",
    EV_DEFAULT,
    "i",
    "count",
    "Set the number of tempmodels that are spawned\n"
    "This is only used for the originspawn and tagspawn commands,\n"
    "and not for emitters, use spawnrate instead"
);
Event EV_Client_SetFade("fade", EV_DEFAULT, NULL, NULL, "Set the tempmodel to fade out over it's life");
Event EV_Client_SetFadeDelay("fadedelay", EV_DEFAULT, "f", "time", "Set the amount of time to delay a fade");
Event EV_Client_SetFadeIn("fadein", EV_DEFAULT, "f", "time", "Set the tempmodel to fade in over the specified time");
Event EV_Client_SetTwinkle(
    "twinkle",
    EV_DEFAULT,
    "ffff",
    "mintimeoff maxtimeoff mintimeon maxtimeon",
    "Set the tempmodel to twinkle with the specified settings"
);
Event EV_Client_SetTrail(
    "trail", EV_DEFAULT, "sssf", "shader startTag endTag life", "Set the tempmodel to have a swipe that follows it"
);
Event EV_Client_SetSpawnRange(
    "spawnrange",
    EV_DEFAULT,
    "iI",
    "range1 range2",
    "Sets the range in which this effect will spawn tempmodels.  If one number is specified, it is the max range\n"
    "and 0 is the min range; if two numbers are specified, the larger is the max range.\n"
);
Event EV_Client_SetSpawnRate(
    "spawnrate",
    EV_DEFAULT,
    "f",
    "rate",
    "Set the spawnrate of the emitter (models per second).\n"
    "This is only used for emitters and not for the originspawn and tagspawn commands"
);
Event EV_Client_SetOriginOffset(
    "offset",
    EV_DEFAULT,
    "SfFSfFSfF",
    "[random|crandom|range] offsetX [offsetX2] [random|crandom|range] offsetY [offsetY2] [random|crandom|range] "
    "offsetZ [offsetZ2]",
    "If random is specified, the component will range from 0 to +specified offset.\n"
    "If crandom is specified, the component will range from -specified to +specified offset.\n"
    "If range is specified, the component needs two values; it will randomly pick a number in the range\n"
    "from the first number to the first number plus the second number.\n"
    "If no keyword is explicitly specified, then the component will just be added on\n"
    "without randomness.\n"
    "This offset is applied using the world axis."
);
Event EV_Client_SetScaleRate(
    "scalerate",
    EV_DEFAULT,
    "f",
    "rate",
    "Set the scaling rate of the spawned tempmodel\n"
    "If a negative rate is used, the model will shrink"
);
Event EV_Client_SetCircle(
    "circle",
    EV_DEFAULT,
    NULL,
    NULL,
    "Set the tempmodels to be spawned in a circle around the origin\n"
    "This circle will be generated in the X/Y axis of the model"
);
Event EV_Client_SetSphere(
    "sphere",
    EV_DEFAULT,
    NULL,
    NULL,
    "Set the tempmodels to spawn in a sphere around the origin.\n"
    "If sphereradius is set, the tempmodels will spawn at the radius distance from\n"
    "the origin"
);
Event EV_Client_SetInwardSphere(
    "inwardsphere",
    EV_DEFAULT,
    NULL,
    NULL,
    "Create the tempmodels in a sphere around the origin, and adjust their\n"
    "angle so they point toward the center of the sphere.  This is best used with a\n"
    "spehere radius and some velocity so the models look like they're heading toward the\n"
    "center of the sphere."
);
Event EV_Client_SetRadius(
    "radius", EV_DEFAULT, "f", "radius", "Set the radius of the sphere for the inwardsphere amd sphere settings"
);
Event EV_Client_SetSwarm(
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
Event EV_Client_SetWavy(
    "wavy",
    EV_DEFAULT,
    "SF",
    "[random|crandom] distance",
    "Set the tempmodel to move in a wavy path to a maxmimun of the distance specified\n"
    "If random is specified, the distance is between 0 and +specified distance\n"
    "If crandom is specified, the distance is between -specified and +specified distance"
);
Event EV_Client_SetAlign("align", EV_DEFAULT, NULL, NULL, "Align the tempmodels to the direction they are traveling");
Event EV_Client_SetAlignOnce(
    "alignonce",
    EV_DEFAULT,
    NULL,
    NULL,
    "Align the tempmodels to the direction they are traveling at the time they are initialized"
);
Event EV_Client_SetFlickerAlpha(
    "flicker",
    EV_DEFAULT,
    NULL,
    NULL,
    "Set the tempmodel to change it's alpha every frame.  Creates a flickering effect"
);
Event EV_Client_SetCollision(
    "collision",
    EV_DEFAULT,
    "S",
    "water",
    "Turn on collision for the tempmodel.\n"
    "If the keyword water is specified, then the tempmodel will collide with water"
);
Event EV_Client_TagSpawn(
    "tagspawn",
    EV_DEFAULT,
    "s",
    "tagName",
    "Spawn tempmodels from the specified tag.\n"
    "This command is followed by a ( to specify a block of commands that modify the tempmodels"
);
Event EV_Client_TagSpawnLinked(
    "tagspawnlinked",
    EV_DEFAULT,
    "s",
    "tagName",
    "Spawn tempmodels from the specified tag, linked to the entity at the tag.\n"
    "This command is followed by a ( to specify a block of commands that modify the tempmodels"
);
Event EV_Client_OriginSpawn(
    "originspawn",
    EV_DEFAULT,
    NULL,
    NULL,
    "Spawn tempmodels from the origin.\n"
    "This command is followed by a ( to specify a block of commands that modify the tempmodels"
);
Event EV_Client_OriginBeamSpawn(
    "originbeamspawn",
    EV_DEFAULT,
    NULL,
    NULL,
    "Spawn a beam from the origin.\n"
    "This command is followed by a ( to specify a block of commands that modify the beam"
);
Event EV_Client_OriginBeamEmitter(
    "originbeamemitter",
    EV_DEFAULT,
    NULL,
    NULL,
    "Spawn beams from the origin.\n"
    "This command is followed by a ( to specify a block of commands that modify the beam"
);
Event EV_Client_TagEmitter(
    "tagemitter",
    EV_DEFAULT,
    "ss",
    "tagName emitterName",
    "Create an emitter that spawns tempmodels from the specified tag.\n"
    "This command is followed by a ( to specify a block of commands that modify the tempmodels"
);
Event EV_Client_OriginEmitter(
    "originemitter",
    EV_DEFAULT,
    "s",
    "emitterName",
    "Create an emitter that spawns tempmodels from the origin.\n"
    "This command is followed by a ( to specify a block of commands that modify the tempmodels"
);
Event EV_Client_EmitterOn("emitteron", EV_DEFAULT, "s", "emitterName", "Turn the specified emitter on");
Event EV_Client_EmitterOff("emitteroff", EV_DEFAULT, "s", "emitterName", "Turn the specified emitter off");
Event EV_Client_Sound(
    "sound",
    EV_DEFAULT,
    "sSFFFFS",
    "soundName channelName volume min_distance pitch randompitch randomvolume argstype",
    "Play the specified sound"
);
Event EV_Set_Current_Tiki(
    "settiki",
    EV_DEFAULT,
    "s",
    "settiki tikiname - all subsequent events are applied to this tiki",
    "sets the tiki the aliases should be on in the sound uberfile"
);
Event EV_Client_StopSound("stopsound", EV_DEFAULT, "i", "channel", "Stops the sound on the specified channel.");
Event EV_Client_StopAliasChannel(
    "stopaliaschannel", EV_DEFAULT, "s", "alias", "Stops the sound channel used by the specified alias."
);
Event EV_Client_LoopSound(
    "loopsound", EV_DEFAULT, "sFFf", "soundName volume min_distance pitch", "Play the specifed sound as a looping sound"
);
Event EV_Client_Cache("cache", EV_CACHE, "s", "resourceName", "Cache the specified resource");
Event EV_Client_CacheImage(
    "cacheimage",
    EV_CACHE,
    "s",
    "imageName",
    "Cache the specified image (For menu graphics that should never be picmiped)"
);
Event EV_Client_CacheFont("cachefont", EV_CACHE, "s", "resourceName", "Cache the specified resource");
Event EV_Client_AliasCache(
    "aliascache",
    EV_DEFAULT,
    "ssSSSSSS",
    "alias realPath arg1 arg2 arg3 arg4 arg5 arg6",
    "Create an alias to the specified path and cache the resource"
);
Event EV_Client_Alias(
    "alias",
    EV_DEFAULT,
    "ssSSSSSS",
    "alias realPath arg1 arg2 arg3 arg4 arg5 arg6",
    "Create an alias to the specified path"
);
Event EV_Client_CacheAlias(
    "cachefromalias", EV_DEFAULT, "s", "alias", "Caches all data matching a previously specified alias"
);
Event EV_Client_Footstep(
    "footstep",
    EV_DEFAULT,
    "ssI",
    "tag sRunning iEquipment",
    "Play a footstep sound that is appropriate to the surface we are currently stepping on\n"
    "sRunning should be set to run, walk, or ladder"
);
Event EV_Client_Landing(
    "landing",
    EV_DEFAULT,
    "FI",
    "fVolume iEquipment",
    "Play a landing sound that is appropriate to the surface we are landing on\n"
);
Event EV_Client_BodyFall(
    "bodyfall",
    EV_DEFAULT,
    "F",
    "fVolume",
    "Play a body fall sound that is appropriate to the surface we are falling on\n"
);
Event EV_Client_Client(
    "client", EV_DEFAULT, "SSSSSS", "arg1 arg2 arg3 arg4 arg5 arg6", "Execute the specified command arg string"
);
Event EV_Client_OriginDynamicLight(
    "origindlight",
    EV_DEFAULT,
    "fffffSS",
    "red green blue intensity life type1 type2",
    "Spawn a dynamic light from the origin of the model\n"
    "The red,green,blue parms are the color of the light\n"
    "The intensity is the radius of the light\n"
    "type is the type of light to create (lensflare,viewlensflare,additive)"
);
Event EV_Client_TagDynamicLight(
    "tagdlight",
    EV_DEFAULT,
    "sfffffSS",
    "tagName red green blue intensity life type1 type2",
    "Spawn a dynamic light from the specified tag\n"
    "The red,green,blue parms are the color of the light\n"
    "The intensity is the radius of the light\n"
    "type is the type of light to create (lensflare,viewlensflare,additive)"
);
Event EV_Client_DynamicLight(
    "dlight",
    EV_DEFAULT,
    "ffffSS",
    "red green blue intensity type1 type2",
    "This makes the emitter itself a dynamic light"
    "The red,green,blue parms are the color of the light\n"
    "The intensity is the radius of the light\n"
    "type is the type of light to create (lensflare,viewlensflare,additive)"
);
Event EV_Client_BlockDynamicLight(
    "blockdlight",
    EV_DEFAULT,
    "ffSS",
    "intensity life type1 type2",
    "Spawn a dynamic light from the origin of the model\n"
    "An additional commands block allows the specification of moving & varying dlights\n"
    "The intensity is the radius of the light\n"
    "type is the type of light to create (lensflare,viewlensflare,additive)"
);
Event EV_Client_SetEntityColor(
    "entcolor", EV_DEFAULT, "fffF", "red green blue alpha", "Set the color(modulate) of this entity"
);
Event EV_Client_SetTexAnimTime("texanimtime", EV_DEFAULT, "f", "time", "Set the texture animation speed");
Event EV_Client_SetGlobalFade(
    "globalfade", EV_DEFAULT, "S", "[in|out]", "Set the tempmodels to globally fade in or out together"
);
Event EV_Client_SetParentLink(
    "parentlink", EV_DEFAULT, NULL, NULL, "Set the tempmodels linked to the parent, so they move with the parent model"
);
Event EV_Client_SetHardLink(
    "hardlink",
    EV_DEFAULT,
    NULL,
    NULL,
    "Set the tempmodels linked to the model they are spawned from, so they move with it"
);
Event EV_Client_SetRandomRoll(
    "randomroll", EV_DEFAULT, NULL, NULL, "Set the tempmodels so they pick a random roll value every frame"
);
Event EV_Client_ParentAngles("parentangles", EV_DEFAULT, NULL, NULL, "Set the tempmodels angles to that of its parent");
Event EV_Client_EmitterAngles(
    "emitterangles",
    EV_DEFAULT,
    "FFF",
    "pitchofs yawofs rollofs",
    "Set the tempmodels angles to that of the emitter\n"
    "The three optional parameters are for setting an angle offset from the emitter"
);
Event EV_Client_SetAngles(
    "angles",
    EV_DEFAULT,
    "SfFSfFSfF",
    "[random|crandom|range] pitch [pitch2] [random|crandom|range] yaw [yaw2] [random|crandom|range] roll [roll2]",
    "If random is specified, the component will range from 0 to +specified value.\n"
    "If crandom is specified, the component will range from -specified to +specified value.\n"
    "If range is specified, the component needs two values; it will randomly pick a number in the range\n"
    "from the first number to the first number plus the second number.\n"
    "If no keyword is explicitly specified, then the component will be just set\n"
);
Event EV_Client_SetRelativeAngles(
    "relativeangles",
    EV_DEFAULT,
    "",
    "",
    "makes the spawn angles get applied relative to the orientation of the model"
);
Event EV_Client_Swipe("swipe", EV_DEFAULT, "V", "origin", "Do a swipe and add it on to the swipe rendering list.");
Event EV_Client_SwipeOn(
    "swipeon",
    EV_DEFAULT,
    "ssff",
    "shader startTagName endTagNamelife life",
    "Signal the start of a swipe from the current tag"
);
Event EV_Client_SwipeOff("swipeoff", EV_DEFAULT, NULL, NULL, "Signal the end of a swipe");
Event EV_Client_BeginTagBeamEmitter(
    "tagbeamemitter",
    EV_DEFAULT,
    "sss",
    "tagstart tagend name",
    "Create a beam emitter that uses 2 tags to determine it's start and end position"
);
Event EV_Client_BeginTagBeamSpawn(
    "tagbeamspawn",
    EV_DEFAULT,
    "sf",
    "tagstart name",
    "Create a beam emitter that uses the tag to determine it's starting position."
);
Event EV_Client_AnimateOnce(
    "animateonce", EV_DEFAULT, NULL, NULL, "Set a tempmodel to animate once and then get removed"
);
Event EV_Client_SetAnim("anim", EV_DEFAULT, "s", "animation", "Set a tempmodel the the specified animation");
Event EV_Client_SetSubdivisions("numsegments", EV_DEFAULT, "i", "numsegments", "Set the number of segments in a beam");
Event EV_Client_SetMinOffset("minoffset", EV_DEFAULT, "f", "minoffset", "Set the minimum offset from center in a beam");
Event EV_Client_SetMaxOffset("maxoffset", EV_DEFAULT, "f", "maxoffset", "Set the maximum offset from center in a beam");
Event EV_Client_SetBeamShader("beamshader", EV_DEFAULT, "s", "shadername", "Set the shader to use for the beam");
Event EV_Client_SetBeamLength(
    "beamlength", EV_DEFAULT, "f", "length", "Set the length of the beam or trace length (for decals)"
);
Event EV_Client_SetBeamDelay(
    "beamdelay",
    EV_DEFAULT,
    "SF",
    "[random] delay",
    "Set the delay time between creating new beams.\n"
    "If the keyword random is specified, the delay between beams will occur randomly between 0 and the time specified"
);
Event EV_Client_SetBeamToggleDelay(
    "beamtoggledelay",
    EV_DEFAULT,
    "SF",
    "[random] delay",
    "Set a delay between toggling the beams on and off.\n"
    "If the keyword random is specified, the delay between toggling will occur randomly between 0 and the time "
    "specified"
);
Event EV_Client_SetBeamPersist("beampersist", EV_DEFAULT, NULL, NULL, "Make the beams persist instead of blinking out");
Event EV_Client_SetBeamOffsetEndpoints(
    "beam_offset_endpoints", EV_DEFAULT, NULL, NULL, "Make the beams endpoints offset to reduce the bunching up effect"
);
Event EV_Client_BeamSphere(
    "beamsphere",
    EV_DEFAULT,
    "i",
    "count",
    "Create a sphere shaped beam effect from the origin. Count is the number of beams"
);
Event EV_Client_Spread(
    "spread",
    EV_DEFAULT,
    "ff",
    "spreadx spready",
    "Add a random variance in the spawned beam in the forward direction by the amound specified in spreadx and spready"
);
Event EV_Client_UseLastTraceEnd(
    "uselasttraceend", EV_DEFAULT, NULL, NULL, "Makes this trace command use the end results of the last trace command"
);
Event EV_Client_OffsetAlongAxis(
    "offsetalongaxis",
    EV_DEFAULT,
    "SfFSfFSfF",
    "[crandom|random|range] offsetx [offsetx2] [crandom|random|range] offsety [offsety2] [crandom|random|range] "
    "offsetz [offsetz2]",
    "If random is specified, the component will range from 0 to specified offset.\n"
    "If crandom is specified, the component will range from -specified to +specified offset.\n"
    "If range is specified, the component needs two values; it will randomly pick a number in the range\n"
    "from the first number to the first number plus the second number.\n"
    "If no keyword is explicitly specified, then the component will just be added on\n"
    "without randomness.\n"
    "This offset is applied using the model's local axis"
);
Event EV_Client_Cone(
    "cone",
    EV_DEFAULT,
    "ff",
    "height radius",
    "Randomly spawns the particle somewhere inside a cone along the model's local x axis"
);
Event EV_Client_SetEndAlpha("endalpha", EV_DEFAULT, "f", "alpha", "Set the alpha of the beam's endpoint");
Event EV_Client_RandomChance(
    "randomchance",
    EV_DEFAULT,
    "fSSSSSS",
    "percentage [arg1] [arg2] [arg3] [arg4] [arg5] [arg6]",
    "Set the percentage chance that command will occur"
);
Event EV_Client_DelayedRepeat(
    "delayedrepeat",
    EV_DEFAULT,
    "fiSSSSSS",
    "time commandnumber [arg1] [arg2] [arg3] [arg4] [arg5] [arg6]",
    "Set the time delay between this command getting executed.  This requires a command number to be assigned here.\n"
    "This is internally used to keep track of the commands this entity executes and will resolve naming conflicts.\n"
    "This is only useful if the command gets called continuously but must execute at regular intervals.\n"
);
Event EV_Client_CommandDelay(
    "commanddelay",
    EV_DEFAULT,
    "fiSSSSSS",
    "time commandnumber [arg1] [arg2] [arg3] [arg4] [arg5] [arg6]",
    "Set the time delay between this command getting executed.  This requires a command number to be assigned here.\n"
    "This is internally used to keep track of the commands this entity executes and will resolve naming conflicts."
);
Event EV_Client_BounceDecal(
    "bouncedecal",
    EV_DEFAULT,
    "iI",
    "maxamount temporary",
    "Put a mark when the tempmodel bounces and hits a surface\n"
    "maxamount = Max amount of decals to make when bouncing\n"
    "temporary = specify 1 for a temporary mark that only appears for a short time, 0 for a decal that stays aroung "
    "longer (default is 0)\n"
);
Event EV_Client_SetDecalRadius("decalradius", EV_DEFAULT, "f", "radius", "Set the radius of the decal");
Event EV_Client_SetDecalOrientation(
    "orientation",
    EV_DEFAULT,
    "f",
    "degrees",
    "Set the degrees to orient the decal.  Specify 'random' to use a random orientation"
);
Event EV_Client_SetDecalShader(
    "decalshader", EV_DEFAULT, "s", "shadername", "Set the shader to use for the impact decal"
);
Event EV_Client_SetTraceLength("tracelength", EV_DEFAULT, "f", "length", "Set the length of the trace for the decal");
Event EV_Client_TraceSpawn(
    "tracespawn", EV_DEFAULT, "s", "modelname", "Spawn the specified model when the trace hits a solid object"
);
Event EV_Client_TagList(
    "taglist",
    EV_DEFAULT,
    "ssSSSSSS",
    "arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8",
    "Set the tag list to create a beam that travels between all the tags"
);
Event EV_Client_Lightstyle(
    "lightstyle",
    EV_DEFAULT,
    "s",
    "nameOfImage",
    "Set a lightstyle to determine the color of this tempmodel, the image\n"
    "specified is used to determine the look of the light style"
);
Event EV_Client_PhysicsRate(
    "physicsrate",
    EV_DEFAULT,
    "s",
    "rate",
    "Set the rate (in updates per second) for the tempmodel's physics to be updated"
);
Event EV_Client_Parallel("parallel", EV_DEFAULT, NULL, NULL, "Set tempmodel to be parallel to the viewer");
Event EV_Client_AlwaysDraw(
    "alwaysdraw",
    EV_DEFAULT,
    NULL,
    NULL,
    "Set emitter/tempmodel to be alwaysdraw. Which can be turned off by alwaysdraw"
);
Event EV_Client_Detail(
    "detail", EV_DEFAULT, NULL, NULL, "Set emitter/tempmodel to be detail. Which can be turned off by detail"
);
Event EV_Client_Print("print", EV_DEFAULT, "s", "string", "Prints a string.");
Event EV_Client_SetVolumetric(
    "volumetric", EV_DEFAULT, NULL, NULL, "Set the effect to spawn volumetric sources rather than tempmodels"
);
Event EV_Client_Wind("wind", EV_DEFAULT, NULL, NULL, "Makes the temp model be affected by wind");
Event
    EV_Client_SpriteGridLighting("spritegridlighting", EV_DEFAULT, NULL, NULL, "Calculates grid lighting for a sprite");
Event EV_Client_WaterOnly("wateronly", EV_DEFAULT, NULL, NULL, "makes the temp model remove itself if it leaves water");
Event EV_Client_SetAlignStretch(
    "alignstretch",
    EV_DEFAULT,
    "F",
    "scaleMultiplier",
    "Aligns the temp model to the direction of travel, and stretches it betwen the last and current positions.\n"
);
Event EV_Client_SetClampVelocity(
    "clampvel",
    EV_DEFAULT,
    "ffffff",
    "minX maxX minY maxY minZ maxZ",
    "sets the valid range for velocities along global axes.  Cannot be used with clampvelaxis."
);
Event EV_Client_SetClampVelocityAxis(
    "clampvelaxis",
    EV_DEFAULT,
    "ffffff",
    "minX maxX minY maxY minZ maxZ",
    "sets the valid range for velocities along oriented axes.  Cannot be used with clampvel."
);
Event EV_Client_Treads(
    "treads",
    EV_DEFAULT,
    "ssi",
    "tagName shader localrefnumber",
    "Spawn treads from the specified tag using the specified tread type."
);
Event EV_Client_Treads_Off(
    "treadsoff", EV_DEFAULT, "si", "tagName localrefnumber", "Stops spawning treads from the specified tag."
);
Event EV_Client_ViewKick(
    "viewkick",
    EV_DEFAULT,
    "fffffsff",
    "pitchmin pitchmax yawmin yawmax recenterspeed patters pitchmax yawmax scatterpitchmax",
    "Adds kick to the view of the owner when fired."
);
Event EV_Client_EyeLimits(
    "eyelimits",
    EV_DEFAULT,
    "fff",
    "fPitchmax fYawmax fRollmax",
    "Set the max angle offsets for the eyes from the model's head"
);
Event EV_Client_EyeMovement(
    "eyemovement",
    EV_DEFAULT,
    "f",
    "fMovement",
    "Sets the amount of the head's movement to apply to the players view. 1 is full, 0 is none"
);
Event EV_Client_SFXStart(
    "sfx",
    EV_DEFAULT,
    "sSSSSSSSS",
    "sCommand arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8",
    "Used for adding commands to a special effect"
);
Event EV_Client_SFXStartDelayed(
    "delayedsfx",
    EV_DEFAULT,
    "fsSSSSSSSS",
    "fDelay sCommand arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8",
    "Used for adding commands to a special effect with a time delay"
);

EffectsEventQueueNode EffectsEventQueue;

CLASS_DECLARATION(Listener, ClientGameCommandManager, NULL) {
    {&EV_Client_StartBlock,                 &ClientGameCommandManager::StartBlock                },
    {&EV_Client_EndBlock,                   &ClientGameCommandManager::EndBlock                  },
    {&EV_Client_EmitterStartOff,            &ClientGameCommandManager::EmitterStartOff           },
    {&EV_Client_OriginSpawn,                &ClientGameCommandManager::BeginOriginSpawn          },
    {&EV_Client_OriginBeamSpawn,            &ClientGameCommandManager::BeginOriginBeamSpawn      },
    {&EV_Client_OriginBeamEmitter,          &ClientGameCommandManager::BeginOriginBeamEmitter    },
    {&EV_Client_TagSpawn,                   &ClientGameCommandManager::BeginTagSpawn             },
    {&EV_Client_TagSpawnLinked,             &ClientGameCommandManager::BeginTagSpawnLinked       },
    {&EV_Client_TagEmitter,                 &ClientGameCommandManager::BeginTagEmitter           },
    {&EV_Client_BeginTagBeamEmitter,        &ClientGameCommandManager::BeginTagBeamEmitter       },
    {&EV_Client_BeginTagBeamSpawn,          &ClientGameCommandManager::BeginTagBeamSpawn         },
    {&EV_Client_OriginEmitter,              &ClientGameCommandManager::BeginOriginEmitter        },
    {&EV_Client_EmitterOn,                  &ClientGameCommandManager::EmitterOn                 },
    {&EV_Client_EmitterOff,                 &ClientGameCommandManager::EmitterOff                },
    {&EV_Client_SetAlpha,                   &ClientGameCommandManager::SetAlpha                  },
    {&EV_Client_SetDieTouch,                &ClientGameCommandManager::SetDieTouch               },
    {&EV_Client_SetBounceFactor,            &ClientGameCommandManager::SetBounceFactor           },
    {&EV_Client_SetBounceSound,             &ClientGameCommandManager::SetBounceSound            },
    {&EV_Client_SetBounceSoundOnce,         &ClientGameCommandManager::SetBounceSoundOnce        },
    {&EV_Client_SetScale,                   &ClientGameCommandManager::SetScale                  },
    {&EV_Client_SetScaleUpDown,             &ClientGameCommandManager::SetScaleUpDown            },
    {&EV_Client_SetScaleMin,                &ClientGameCommandManager::SetScaleMin               },
    {&EV_Client_SetScaleMax,                &ClientGameCommandManager::SetScaleMax               },
    {&EV_Client_SetModel,                   &ClientGameCommandManager::SetModel                  },
    {&EV_Client_SetLife,                    &ClientGameCommandManager::SetLife                   },
    {&EV_Client_SetColor,                   &ClientGameCommandManager::SetColor                  },
    {&EV_Client_SetRadialVelocity,          &ClientGameCommandManager::SetRadialVelocity         },
    {&EV_Client_SetVelocity,                &ClientGameCommandManager::SetVelocity               },
    {&EV_Client_SetAngularVelocity,         &ClientGameCommandManager::SetAngularVelocity        },
    {&EV_Client_SetColorVelocity,           &ClientGameCommandManager::SetAngularVelocity        },
    {&EV_Client_SetRandomVelocity,          &ClientGameCommandManager::SetRandomVelocity         },
    {&EV_Client_SetRandomVelocityAlongAxis, &ClientGameCommandManager::SetRandomVelocityAlongAxis},
    {&EV_Client_SetAccel,                   &ClientGameCommandManager::SetAccel                  },
    {&EV_Client_SetFriction,                &ClientGameCommandManager::SetFriction               },
    {&EV_Client_SetSpin,                    &ClientGameCommandManager::SetSpin                   },
    {&EV_Client_SetVaryColor,               &ClientGameCommandManager::SetVaryColor              },
    {&EV_Client_SetSmokeParms,              &ClientGameCommandManager::SetAccel                  },
    {&EV_Client_SetCount,                   &ClientGameCommandManager::SetCount                  },
    {&EV_Client_SetFade,                    &ClientGameCommandManager::SetFade                   },
    {&EV_Client_SetFadeDelay,               &ClientGameCommandManager::SetFadeDelay              },
    {&EV_Client_SetFadeIn,                  &ClientGameCommandManager::SetFadeIn                 },
    {&EV_Client_SetTwinkle,                 &ClientGameCommandManager::SetTwinkle                },
    {&EV_Client_SetTrail,                   &ClientGameCommandManager::SetTrail                  },
    {&EV_Client_SetSpawnRange,              &ClientGameCommandManager::SetSpawnRange             },
    {&EV_Client_SetSpawnRate,               &ClientGameCommandManager::SetSpawnRate              },
    {&EV_Client_SetScaleRate,               &ClientGameCommandManager::SetScaleRate              },
    {&EV_Client_SetOriginOffset,            &ClientGameCommandManager::SetOriginOffset           },
    {&EV_Client_SetSphere,                  &ClientGameCommandManager::SetSphere                 },
    {&EV_Client_SetCircle,                  &ClientGameCommandManager::SetCircle                 },
    {&EV_Client_SetInwardSphere,            &ClientGameCommandManager::SetInwardSphere           },
    {&EV_Client_SetRadius,                  &ClientGameCommandManager::SetRadius                 },
    {&EV_Client_SetSwarm,                   &ClientGameCommandManager::SetSwarm                  },
    {&EV_Client_SetAlign,                   &ClientGameCommandManager::SetAlign                  },
    {&EV_Client_SetAlignOnce,               &ClientGameCommandManager::SetAlignOnce              },
    {&EV_Client_SetCollision,               &ClientGameCommandManager::SetCollision              },
    {&EV_Client_SetFlickerAlpha,            &ClientGameCommandManager::SetFlickerAlpha           },
    {&EV_Client_Sound,                      &ClientGameCommandManager::Sound                     },
    {&EV_Set_Current_Tiki,                  &ClientGameCommandManager::SetCurrentTiki            },
    {&EV_Client_StopSound,                  &ClientGameCommandManager::StopSound                 },
    {&EV_Client_StopAliasChannel,           &ClientGameCommandManager::StopAliasChannel          },
    {&EV_Client_LoopSound,                  &ClientGameCommandManager::LoopSound                 },
    {&EV_Client_Cache,                      &ClientGameCommandManager::Cache                     },
    {&EV_Client_CacheImage,                 &ClientGameCommandManager::CacheImage                },
    {&EV_Client_CacheFont,                  &ClientGameCommandManager::CacheFont                 },
    {&EV_Client_AliasCache,                 &ClientGameCommandManager::AliasCache                },
    {&EV_Client_Alias,                      &ClientGameCommandManager::Alias                     },
    {&EV_Client_CacheAlias,                 &ClientGameCommandManager::CacheAlias                },
    {&EV_Client_Footstep,                   &ClientGameCommandManager::Footstep                  },
    {&EV_Client_Landing,                    &ClientGameCommandManager::LandingSound              },
    {&EV_Client_BodyFall,                   &ClientGameCommandManager::BodyFallSound             },
    {&EV_Client_Client,                     &ClientGameCommandManager::Client                    },
    {&EV_Client_TagDynamicLight,            &ClientGameCommandManager::TagDynamicLight           },
    {&EV_Client_OriginDynamicLight,         &ClientGameCommandManager::OriginDynamicLight        },
    {&EV_Client_DynamicLight,               &ClientGameCommandManager::DynamicLight              },
    {&EV_Client_BlockDynamicLight,          &ClientGameCommandManager::BlockDynamicLight         },
    {&EV_Client_SetEntityColor,             &ClientGameCommandManager::SetEntityColor            },
    {&EV_Client_SetGlobalFade,              &ClientGameCommandManager::SetGlobalFade             },
    {&EV_Client_SetParentLink,              &ClientGameCommandManager::SetParentLink             },
    {&EV_Client_SetHardLink,                &ClientGameCommandManager::SetHardLink               },
    {&EV_Client_SetRandomRoll,              &ClientGameCommandManager::SetRandomRoll             },
    {&EV_Client_SetVolumetric,              &ClientGameCommandManager::SetVolumetric             },
    {&EV_Client_SetAngles,                  &ClientGameCommandManager::SetAngles                 },
    {&EV_Client_SetRelativeAngles,          &ClientGameCommandManager::SetRelativeAngles         },
    {&EV_Client_ParentAngles,               &ClientGameCommandManager::ParentAngles              },
    {&EV_Client_Swipe,                      &ClientGameCommandManager::Swipe                     },
    {&EV_Client_SwipeOn,                    &ClientGameCommandManager::SwipeOn                   },
    {&EV_Client_SwipeOff,                   &ClientGameCommandManager::SwipeOff                  },
    {&EV_Client_SetAnim,                    &ClientGameCommandManager::SetAnim                   },
    {&EV_Client_AnimateOnce,                &ClientGameCommandManager::AnimateOnce               },
    {&EV_Client_SetSubdivisions,            &ClientGameCommandManager::SetSubdivisions           },
    {&EV_Client_SetMinOffset,               &ClientGameCommandManager::SetMinOffset              },
    {&EV_Client_SetMaxOffset,               &ClientGameCommandManager::SetMaxOffset              },
    {&EV_Client_SetBeamShader,              &ClientGameCommandManager::SetShader                 },
    {&EV_Client_SetBeamLength,              &ClientGameCommandManager::SetLength                 },
    {&EV_Client_SetDecalShader,             &ClientGameCommandManager::SetShader                 },
    {&EV_Client_SetTraceLength,             &ClientGameCommandManager::SetLength                 },
    {&EV_Client_SetBeamDelay,               &ClientGameCommandManager::SetBeamDelay              },
    {&EV_Client_SetBeamToggleDelay,         &ClientGameCommandManager::SetBeamToggleDelay        },
    {&EV_Client_SetBeamPersist,             &ClientGameCommandManager::SetBeamPersist            },
    {&EV_Client_SetBeamOffsetEndpoints,     &ClientGameCommandManager::SetBeamOffsetEndpoints    },
    {&EV_Client_BeamSphere,                 &ClientGameCommandManager::SetBeamSphere             },
    {&EV_Client_Spread,                     &ClientGameCommandManager::SetSpread                 },
    {&EV_Client_UseLastTraceEnd,            &ClientGameCommandManager::SetUseLastTraceEnd        },
    {&EV_Client_OffsetAlongAxis,            &ClientGameCommandManager::SetOffsetAlongAxis        },
    {&EV_Client_Cone,                       &ClientGameCommandManager::SetCone                   },
    {&EV_Client_SetEndAlpha,                &ClientGameCommandManager::SetEndAlpha               },
    {&EV_Client_RandomChance,               &ClientGameCommandManager::RandomChance              },
    {&EV_Client_CommandDelay,               &ClientGameCommandManager::CommandDelay              },
    {&EV_Client_BounceDecal,                &ClientGameCommandManager::SetBounceDecal            },
    {&EV_Client_SetDecalRadius,             &ClientGameCommandManager::SetDecalRadius            },
    {&EV_Client_SetDecalOrientation,        &ClientGameCommandManager::SetDecalOrientation       },
    {&EV_Client_TagList,                    &ClientGameCommandManager::TagList                   },
    {&EV_Client_Lightstyle,                 &ClientGameCommandManager::SetLightstyle             },
    {&EV_Client_PhysicsRate,                &ClientGameCommandManager::SetPhysicsRate            },
    {&EV_Client_Parallel,                   &ClientGameCommandManager::SetParallel               },
    {&EV_Client_Detail,                     &ClientGameCommandManager::SetDetail                 },
    {&EV_Client_Wind,                       &ClientGameCommandManager::SetWindAffect             },
    {&EV_Client_SpriteGridLighting,         &ClientGameCommandManager::SpriteGridLighting        },
    {&EV_Client_WaterOnly,                  &ClientGameCommandManager::SetWaterOnly              },
    {&EV_Client_SetAlignStretch,            &ClientGameCommandManager::SetAlignStretch           },
    {&EV_Client_SetClampVelocity,           &ClientGameCommandManager::SetClampVel               },
    {&EV_Client_SetClampVelocityAxis,       &ClientGameCommandManager::SetClampVelAxis           },
    {&EV_Client_Print,                      &ClientGameCommandManager::Print                     },
    {&EV_Client_EyeLimits,                  &ClientGameCommandManager::SetEyeLimits              },
    {&EV_Client_EyeMovement,                &ClientGameCommandManager::SetEyeMovement            },
    {&EV_Client_SFXStart,                   &ClientGameCommandManager::StartSFX                  },
    {&EV_Client_SFXStartDelayed,            &ClientGameCommandManager::StartSFXDelayed           },
    {&EV_Client_Treads,                     &ClientGameCommandManager::SpawnTreads               },
    {&EV_Client_Treads_Off,                 &ClientGameCommandManager::TreadsOff                 },
    {&EV_Client_ViewKick,                   &ClientGameCommandManager::EventViewKick             },
    {NULL,                                  NULL                                                 }
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
    EffectsEventQueueNode *event;
    int                    t;

    t     = cg.time;
    event = EffectsEventQueue.next;
    while (event != &EffectsEventQueue && event->inttime <= t) {
        if (event->GetEntityNum() == current_entity_number) {
            // the event is removed from its list and temporarily added to the active list
            LL_Remove(event, next, prev);

            // ProcessEvent will dispose of this event when it is done
            ProcessEvent(event->event);

            // free up the node
            delete event;

            // start over, since can't guarantee that we didn't process any previous or following events
            event = EffectsEventQueue.next;
        } else {
            event = event->next;
        }
    }
}

qboolean ClientGameCommandManager::PostEventForEntity(Event *ev, float fWait)
{
    unsigned short         evnum;
    ClassDef              *c;
    EffectsEventQueueNode *event;
    EffectsEventQueueNode *node;
    int                    inttime;

    if (!m_fEventWait || current_entity_number < 0) {
        if (!str::icmp(ev->getName(), ")")) {
            m_fEventWait = 0.0;
        }

        delete ev;
        return qfalse;
    }

    evnum = ev->eventnum;
    c     = classinfo();
    if (!c->responseLookup[evnum]) {
        delete ev;
        return qfalse;
    }

    event   = EffectsEventQueue.next;
    inttime = EVENT_msec + (fWait * 1000.0f + 0.5f);

    while (event != &EffectsEventQueue && inttime > event->inttime) {
        event = event->next;
    }

    if (m_fEventWait != 0.0) {
        if (!str::icmp(ev->getName(), ")")) {
            m_fEventWait = 0.0;
        }
    }

    node = new EffectsEventQueueNode(ev, inttime, 0, current_entity_number);
    LL_Add(event, node, next, prev);

    return qtrue;
}

void ClientGameCommandManager::SetBaseAndAmplitude(Event *ev, Vector& base, Vector& amplitude)
{
    int i = 1;
    int j = 0;

    for (j = 0; j < 3; j++) {
        str org;

        org = ev->GetString(i++);
        if (org == "crandom") {
            float value  = ev->GetFloat(i++);
            base[j]      = -value;
            amplitude[j] = value + value;
        } else if (org == "random") {
            base[j]      = 0.0;
            amplitude[j] = ev->GetFloat(i++);
        } else if (org == "range") {
            base[j]      = ev->GetFloat(i++);
            amplitude[j] = ev->GetFloat(i++);
        } else {
            base[j]      = 0.0;
            amplitude[j] = atof(org.c_str());
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

void ClientGameCommandManager::Print(Event *ev)
{
    if (current_entity) {
        cgi.DPrintf("%d:%s\n", current_entity->entityNumber, ev->GetString(1).c_str());
    }
}

qboolean ClientGameCommandManager::IsBlockCommand(const str& name)
{
    if (!str::icmp(name, "originspawn")) {
        return true;
    } else if (!str::icmp(name, "originbeamspawn")) {
        return true;
    } else if (!str::icmp(name, "tagspawn")) {
        return true;
    } else if (!str::icmp(name, "tagbeamspawn")) {
        return true;
    } else if (!str::icmp(name, "blockdlight")) {
        return true;
    }

    return false;
}

void ClientGameCommandManager::DelayedRepeat(Event *ev)
{
    int   i;
    float delay;

    delay = ev->GetFloat(1) * 1000.0;
    if (current_entity) {
        commandtime_t *ct = m_command_time_manager.GetLastCommandTime(current_entity->entityNumber, ev->GetInteger(2));

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
        Class::warning("CCM:DelayedRepeat", "Cannot perform delayed repeat on spawned tempmodels");

        Event ev1(ev->GetString(3));

        for (i = 4; i <= ev->NumArgs(); i++) {
            ev1.AddToken(ev->GetToken(i));
        }

        ProcessEvent(ev1);
        return;
    }

    Event ev1(ev->GetString(3));

    for (i = 4; i <= ev->NumArgs(); i++) {
        ev1.AddToken(ev->GetToken(i));
    }
    ProcessEvent(ev1);
}

//===============
// CommandDelay
//===============
void ClientGameCommandManager::CommandDelay(Event *ev)
{
    int    i;
    int    num;
    float  fWait;
    str    eventName;
    int    delay;
    Event *ev1;

    fWait     = ev->GetFloat(1);
    eventName = ev->GetString(2);

    ev1 = new Event(eventName);

    num = ev->NumArgs();
    for (i = 3; i <= num; i++) {
        ev1->AddValue(ev->GetValue(i));
    }

    delay = ev->GetFloat(1) * 1000;
    if (current_entity_number != -1) {
        PostEventForEntity(ev1, fWait);
    } else {
        warning(
            "CCM:CommandDelay",
            "Can't use commanddelay in temp models. Found illegal commanddelay in '%s'\n",
            current_tiki->name
        );
    }
}

void ClientGameCommandManager::StartSFX(Event *ev)
{
    StartSFXCommand(ev, qfalse);
}

void ClientGameCommandManager::StartSFXDelayed(Event *ev)
{
    StartSFXCommand(ev, qtrue);
}

void ClientGameCommandManager::StartSFXCommand(Event *ev, qboolean bDelayed)
{
    qboolean                bBlockCommand;
    int                     i;
    int                     iArgOfs;
    float                   delay;
    str                     sCommandName;
    specialeffectcommand_t *pCommand;

    if (bDelayed) {
        delay   = ev->GetFloat(1);
        iArgOfs = 1;
    } else {
        delay   = 0.0;
        iArgOfs = 0;
    }

    sCommandName  = ev->GetString(iArgOfs + 1);
    bBlockCommand = IsBlockCommand(sCommandName);
    if (!m_pCurrentSfx) {
        if (bBlockCommand) {
            m_spawnthing = &m_localemitter;
            endblockfcn  = &ClientGameCommandManager::EndIgnoreSfxBlock;
        }

        return;
    }

    if (!current_entity) {
        return;
    }

    pCommand = m_pCurrentSfx->AddNewCommand();
    if (!pCommand) {
        return;
    }

    if (bBlockCommand) {
        m_spawnthing      = new spawnthing_t;
        pCommand->emitter = m_spawnthing;
    }
    pCommand->fCommandTime = delay;

    Event *ev1 = new Event(sCommandName);
    for (i = iArgOfs + 2; i <= ev->NumArgs(); i++) {
        ev1->AddToken(ev->GetToken(i));
    }

    if (bBlockCommand) {
        ProcessEvent(ev1);
        pCommand->endfcn = endblockfcn;
        endblockfcn = NULL;
    } else {
        pCommand->pEvent = ev1;
    }
}

void ClientGameCommandManager::EndIgnoreSfxBlock()
{
    endblockfcn = NULL;
}

void ClientGameCommandManager::SetCurrentSFX(specialeffect_t *pSFX)
{
    m_pCurrentSfx = pSFX;
}

void ClientGameCommandManager::ClearCurrentSFX()
{
    m_pCurrentSfx = NULL;
}

//===============
// RandomChance
//===============
void ClientGameCommandManager::RandomChance(Event *ev)
{
    int   i;
    float percentage = ev->GetFloat(1);

    if ((random() < percentage) && (ev->NumArgs() >= 2)) {
        Event *ev1;

        ev1 = new Event(ev->GetString(2));

        for (i = 3; i <= ev->NumArgs(); i++) {
            ev1->AddToken(ev->GetToken(i));
        }
        ProcessEvent(ev1);
    }
}

void ClientGameCommandManager::SetAlwaysDraw(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

    m_spawnthing->cgd.flags2 |= T2_ALWAYSDRAW;
}

//===============
// SetDetail
//===============
void ClientGameCommandManager::SetDetail(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

    m_spawnthing->cgd.flags |= T_DETAIL;
}

void ClientGameCommandManager::SetWindAffect(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

    m_spawnthing->cgd.flags2 |= T2_WIND_AFFECT;
}

void ClientGameCommandManager::SpriteGridLighting(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

    m_spawnthing->cgd.flags2 |= T2_SPRITEGRIDLIGHTING;
}

void ClientGameCommandManager::SetWaterOnly(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

    m_spawnthing->cgd.flags2 |= T2_WATERONLY;
}

void ClientGameCommandManager::SetClampVel(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

    if (ev->NumArgs() != 6) {
        cgi.Printf("clampvel has 6 arguments\n");
        return;
    }

    m_spawnthing->cgd.flags2 |= T2_CLAMP_VEL;
    // cannot be both clamp vel and clamp vel axis
    m_spawnthing->cgd.flags2 &= ~T2_CLAMP_VEL_AXIS;

    m_spawnthing->cgd.minVel[0] = ev->GetFloat(1);
    m_spawnthing->cgd.maxVel[0] = ev->GetFloat(2);
    m_spawnthing->cgd.minVel[1] = ev->GetFloat(3);
    m_spawnthing->cgd.maxVel[1] = ev->GetFloat(4);
    m_spawnthing->cgd.minVel[2] = ev->GetFloat(5);
    m_spawnthing->cgd.maxVel[2] = ev->GetFloat(6);
}

void ClientGameCommandManager::SetClampVelAxis(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

    if (ev->NumArgs() != 6) {
        cgi.Printf("clampvel has 6 arguments\n");
        return;
    }

    m_spawnthing->cgd.flags2 |= T2_CLAMP_VEL_AXIS;
    // cannot be both clamp vel and clamp vel axis
    m_spawnthing->cgd.flags2 &= ~T2_CLAMP_VEL;

    m_spawnthing->cgd.minVel[0] = ev->GetFloat(1);
    m_spawnthing->cgd.maxVel[0] = ev->GetFloat(2);
    m_spawnthing->cgd.minVel[1] = ev->GetFloat(3);
    m_spawnthing->cgd.maxVel[1] = ev->GetFloat(4);
    m_spawnthing->cgd.minVel[2] = ev->GetFloat(5);
    m_spawnthing->cgd.maxVel[2] = ev->GetFloat(6);
}

void ClientGameCommandManager::SetAlignStretch(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

    m_spawnthing->cgd.flags |= T_ALIGN;
    m_spawnthing->cgd.flags2 |= T2_ALIGNSTRETCH;

    if (ev->NumArgs() > 0) {
        m_spawnthing->cgd.scale2 = ev->GetFloat(1);
    } else {
        m_spawnthing->cgd.scale2 = 1.0;
    }
}

//===============
// SetDecalRadius
//===============
void ClientGameCommandManager::SetDecalRadius(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

    m_spawnthing->cgd.decal_radius = ev->GetFloat(1);
}

//===============
// SetDecalOrientation
//===============
void ClientGameCommandManager::SetDecalOrientation(Event *ev)
{
    str deg;

    if (!m_spawnthing) {
        return;
    }

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
void ClientGameCommandManager::SetBounceDecal(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

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
void ClientGameCommandManager::SetParallel(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

    m_spawnthing->cgd.flags2 |= T2_PARALLEL;
}

//===============
// PhysicsRate
//===============
void ClientGameCommandManager::SetPhysicsRate(Event *ev)
{
    str rate;

    if (!m_spawnthing) {
        return;
    }

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
void ClientGameCommandManager::SetLightstyle(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

    m_spawnthing->cgd.lightstyle = CG_RegisterLightStyle(ev->GetString(1));
}

//===============
// SetBeamSphere
//===============
void ClientGameCommandManager::SetBeamSphere(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

    m_spawnthing->numspherebeams = ev->GetInteger(1);
    m_spawnthing->beamflags      = BEAM_SPHERE_EFFECT;
}

//===============
// SetBeamSpread
//===============
void ClientGameCommandManager::SetSpread(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

    m_spawnthing->spreadx = ev->GetFloat(1);
    m_spawnthing->spready = ev->GetFloat(2);
}

//===============
// SetUseLastTraceEnd
//===============
void ClientGameCommandManager::SetUseLastTraceEnd(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

    m_spawnthing->use_last_trace_end = qtrue;
}

//===============
// SetSubdivisions
//===============
void ClientGameCommandManager::SetSubdivisions(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

    m_spawnthing->numSubdivisions = ev->GetInteger(1);
}

//===============
// SetBeamPersist
//===============
void ClientGameCommandManager::SetBeamPersist(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

    m_spawnthing->beamflags |= BEAM_PERSIST_EFFECT;
}

//===============
// SetBeamOffsetEndponts
//===============
void ClientGameCommandManager::SetBeamOffsetEndpoints(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

    m_spawnthing->beamflags |= BEAM_OFFSET_ENDPOINTS;
}

//===============
// SetBeamDelay
//===============
void ClientGameCommandManager::SetBeamDelay(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

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
void ClientGameCommandManager::SetBeamToggleDelay(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

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
void ClientGameCommandManager::SetLength(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

    m_spawnthing->length = ev->GetFloat(1);
}

//===============
// SetShader
//===============
void ClientGameCommandManager::SetShader(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

    m_spawnthing->cgd.shadername = ev->GetString(1);
}

//===============
// SetMinOffset
//===============
void ClientGameCommandManager::SetMinOffset(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

    m_spawnthing->min_offset = ev->GetInteger(1);
}

//===============
// SetSubdivisions
//===============
void ClientGameCommandManager::SetMaxOffset(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

    m_spawnthing->max_offset = ev->GetInteger(1);
}

//===============
// SetAnim
//===============
void ClientGameCommandManager::SetAnim(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

    m_spawnthing->animName = ev->GetString(1);
}

//===============
// AnimateOnce
//===============
void ClientGameCommandManager::AnimateOnce(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

    m_spawnthing->cgd.flags |= T_ANIMATEONCE;
}

//===============
// EmitterStartOff
//===============
void ClientGameCommandManager::EmitterStartOff(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

    m_spawnthing->startoff = qtrue;
}

//===============
// StartBlock
//===============
void ClientGameCommandManager::StartBlock(Event *ev)
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
void ClientGameCommandManager::EndBlock(Event *ev)
{
    // Make sure there are no more args on this line, because
    // they will be lost

    if (ev->NumArgs() > 1) {
        cgi.DPrintf("CCM::StartBlock : Invalid commands on end block '{'\n");
    }

    if (endblockfcn) {
        (this->*endblockfcn)();
    }

    endblockfcn = NULL;
    // Fixed since version 2.0
    m_spawnthing = NULL;
}

//=============
// SetSwarm
//=============
void ClientGameCommandManager::SetSwarm(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

    m_spawnthing->cgd.swarmfreq     = ev->GetInteger(1);
    m_spawnthing->cgd.swarmmaxspeed = ev->GetFloat(2);
    m_spawnthing->cgd.swarmdelta    = ev->GetFloat(3);
    m_spawnthing->cgd.flags |= T_SWARM;
}

//=============
// SetCircle
//=============
void ClientGameCommandManager::SetCircle(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

    m_spawnthing->cgd.flags |= T_CIRCLE;
}

//=============
// SetSphere
//=============
void ClientGameCommandManager::SetSphere(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

    m_spawnthing->cgd.flags |= T_SPHERE;
}

//=============
// SetSphere
//=============
void ClientGameCommandManager::SetRadius(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

    m_spawnthing->sphereRadius = ev->GetFloat(1);
}

//=============
// SetInwardSphere
//=============
void ClientGameCommandManager::SetInwardSphere(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

    m_spawnthing->cgd.flags |= T_INWARDSPHERE;
    m_spawnthing->sphereRadius = ev->GetFloat(1);
}

//=============
// SetAlign
//=============
void ClientGameCommandManager::SetAlign(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

    m_spawnthing->cgd.flags |= T_ALIGN;
}

//=============
// SetAlignOnce
//=============
void ClientGameCommandManager::SetAlignOnce(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

    m_spawnthing->cgd.flags |= T_ALIGNONCE;
}

//=============
// SetRandomRoll
//=============
void ClientGameCommandManager::SetRandomRoll(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

    m_spawnthing->cgd.flags |= T_RANDOMROLL;
}

void ClientGameCommandManager::SetVolumetric(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

    m_spawnthing->cgd.flags2 |= T2_VOLUMETRIC;
}

//=============
// SetCollision
//=============
void ClientGameCommandManager::SetCollision(Event *ev)
{
    str mask;

    if (!m_spawnthing) {
        return;
    }

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
void ClientGameCommandManager::SetFlickerAlpha(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

    m_spawnthing->cgd.flags |= T_FLICKERALPHA;
}

//=============
// SetEndAlpha
//=============
void ClientGameCommandManager::SetEndAlpha(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

    m_spawnthing->endalpha = ev->GetFloat(1);
}

//=============
// SetOffsetAlongAxis
//=============
void ClientGameCommandManager::SetOffsetAlongAxis(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

    SetBaseAndAmplitude(ev, m_spawnthing->axis_offset_base, m_spawnthing->axis_offset_amplitude);
}

void ClientGameCommandManager::SetCone(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

    m_spawnthing->cgd.flags2 |= T2_CONE;
    m_spawnthing->coneHeight   = ev->GetFloat(1);
    m_spawnthing->sphereRadius = ev->GetFloat(2);
}

//=============
// SetOriginOffset
//=============
void ClientGameCommandManager::SetOriginOffset(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

    SetBaseAndAmplitude(ev, m_spawnthing->origin_offset_base, m_spawnthing->origin_offset_amplitude);
}

//=============
// SetAlpha
//=============
void ClientGameCommandManager::SetAlpha(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

    m_spawnthing->cgd.alpha = ev->GetFloat(1);
}

//=============
// SetDieTouch
//=============
void ClientGameCommandManager::SetDieTouch(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

    m_spawnthing->cgd.flags |= T_DIETOUCH;
}

//=============
// SetBounceFactor
//=============
void ClientGameCommandManager::SetBounceFactor(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

    m_spawnthing->cgd.bouncefactor = ev->GetFloat(1);
    m_spawnthing->cgd.flags |= T_COLLISION;
    m_spawnthing->cgd.collisionmask = (CONTENTS_SOLID);
}

//=============
// SetBounceSound
//=============
void ClientGameCommandManager::SetBounceSound(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

    m_spawnthing->cgd.bouncesound = ev->GetString(1);

    if (ev->NumArgs() == 2) {
        m_spawnthing->cgd.bouncesound_delay = ev->GetFloat(1) * 1000;
    }

    m_spawnthing->cgd.flags |= T_BOUNCESOUND;
}

//=============
// SetBounceSoundOnce
//=============
void ClientGameCommandManager::SetBounceSoundOnce(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

    m_spawnthing->cgd.flags |= T_BOUNCESOUNDONCE;
    SetBounceSound(ev);
}

//=============
// SetScale
//=============
void ClientGameCommandManager::SetScale(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

    m_spawnthing->cgd.scale = ev->GetFloat(1);
}

//=============
// SetScaleUpDown
//=============
void ClientGameCommandManager::SetScaleUpDown(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

    m_spawnthing->cgd.flags |= T_SCALEUPDOWN;
}

//=============
// SetScaleMin
//=============
void ClientGameCommandManager::SetScaleMin(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

    m_spawnthing->cgd.flags |= T_RANDSCALE;
    m_spawnthing->cgd.scalemin = ev->GetFloat(1);
}

//=============
// SetScaleMax
//=============
void ClientGameCommandManager::SetScaleMax(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

    m_spawnthing->cgd.flags |= T_RANDSCALE;
    m_spawnthing->cgd.scalemax = ev->GetFloat(1);
}

//=============
// SetScaleRate
//=============
void ClientGameCommandManager::SetScaleRate(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

    m_spawnthing->cgd.scaleRate = ev->GetFloat(1);
}

//=============
// SetFade
//=============
void ClientGameCommandManager::SetFade(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

    m_spawnthing->cgd.flags |= T_FADE;
}

//=============
// SetFadeDelay
//=============
void ClientGameCommandManager::SetFadeDelay(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

    m_spawnthing->cgd.fadedelay = ev->GetFloat(1) * 1000;
    m_spawnthing->cgd.flags |= T_FADE;
}

void ClientGameCommandManager::SetSpawnRange(Event *ev)
{
    float fVal1, fVal2;

    if (!m_spawnthing) {
        return;
    }

    if (ev->NumArgs() < 1) {
        cgi.DPrintf("too few arguments to spawnrange");
        return;
    }

    fVal1 = ev->GetFloat(1);
    if (ev->NumArgs() > 1) {
        fVal2 = ev->GetFloat(2);
    } else {
        fVal2 = 0.0;
    }

    if (fVal1 * fVal1 > fVal2 * fVal2) {
        m_spawnthing->fMinRangeSquared = fVal2 * fVal2;
        m_spawnthing->fMaxRangeSquared = fVal1 * fVal1;
    } else {
        m_spawnthing->fMinRangeSquared = fVal1 * fVal1;
        m_spawnthing->fMaxRangeSquared = fVal2 * fVal2;
    }
}

//=============
// SetFadeIn
//=============
void ClientGameCommandManager::SetFadeIn(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

    m_spawnthing->cgd.flags |= T_FADEIN;

    if (ev->NumArgs() > 0) {
        m_spawnthing->cgd.fadeintime = ev->GetFloat(1) * 1000;
    }
}

//=============
// SetTwinkle
//=============
void ClientGameCommandManager::SetTwinkle(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

    m_spawnthing->cgd.flags |= T_TWINKLE;
    m_spawnthing->cgd.min_twinkletimeoff = ev->GetFloat(1) * 1000;
    m_spawnthing->cgd.max_twinkletimeoff = ev->GetFloat(2) * 1000;
    m_spawnthing->cgd.min_twinkletimeon  = ev->GetFloat(3) * 1000;
    m_spawnthing->cgd.max_twinkletimeon  = ev->GetFloat(4) * 1000;
}

//=============
// SetSwipe
//=============
void ClientGameCommandManager::SetTrail(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

    m_spawnthing->cgd.flags2 |= T2_TRAIL;
    m_spawnthing->cgd.swipe_shader    = ev->GetString(1);
    m_spawnthing->cgd.swipe_tag_start = ev->GetString(2);
    m_spawnthing->cgd.swipe_tag_end   = ev->GetString(3);
    m_spawnthing->cgd.swipe_life      = ev->GetFloat(4);
}

//=============
// SetSpawnRate
//=============
void ClientGameCommandManager::SetSpawnRate(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

    m_spawnthing->spawnRate = (1.0f / ev->GetFloat(1)) * 1000;
}

//=============
// SetParentLink
//=============
void ClientGameCommandManager::SetParentLink(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

    m_spawnthing->cgd.flags |= T_PARENTLINK;
}

//=============
// SetHardLink
//=============
void ClientGameCommandManager::SetHardLink(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

    m_spawnthing->cgd.flags |= T_HARDLINK;
    m_spawnthing->cgd.parent = current_entity_number;

    if (current_entity) {
        m_spawnthing->cgd.origin = m_spawnthing->cgd.origin - current_entity->origin;
    }
}

//=============
// SetColor
//=============
void ClientGameCommandManager::SetColor(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

    m_spawnthing->cgd.color[0] = ev->GetFloat(1);
    m_spawnthing->cgd.color[1] = ev->GetFloat(2);
    m_spawnthing->cgd.color[2] = ev->GetFloat(3);

    if (ev->NumArgs() == 4) {
        m_spawnthing->cgd.color[3] = ev->GetFloat(4);
        m_spawnthing->cgd.alpha    = ev->GetFloat(4);
    }
}

void ClientGameCommandManager::SetColorRange(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

    // FIXME: unused stub??
}

//=============
// SetEntityColor
//=============
void ClientGameCommandManager::SetEntityColor(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

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
void ClientGameCommandManager::SetGlobalFade(Event *ev)
{
    str mode;

    if (!m_spawnthing) {
        return;
    }

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
void ClientGameCommandManager::SetRandomVelocity(Event *ev)
{
    int    i = 1;
    int    j = 0;
    Vector randval;
    str    vel;

    if (!m_spawnthing) {
        return;
    }

    if (ev->NumArgs() < 3) {
        warning("ClientGameCommandManager::SetRandomVelocity", "Expecting at least 3 args for command randvel");
    }

    SetBaseAndAmplitude(ev, m_spawnthing->randvel_base, m_spawnthing->randvel_amplitude);

    m_spawnthing->cgd.flags2 |= T2_MOVE;
}

//=============
// SetRandomVelocityAlongAxis
//=============
void ClientGameCommandManager::SetRandomVelocityAlongAxis(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

    m_spawnthing->cgd.flags |= T_RANDVELAXIS;
    SetRandomVelocity(ev);
}

void ClientGameCommandManager::SetRadialVelocity(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

    if (ev->NumArgs() < 3) {
        warning("ClientGameCommandManager::SetRadialVelocity", "Missing parameters for command radialvelocity");
    }

    m_spawnthing->cgd.velocity[0] = ev->GetFloat(1);
    m_spawnthing->cgd.velocity[1] = ev->GetFloat(2);
    m_spawnthing->cgd.velocity[2] = ev->GetFloat(3);
    m_spawnthing->cgd.velocity[2] -= m_spawnthing->cgd.velocity[1];
    m_spawnthing->cgd.flags2 |= T2_RADIALVELOCITY | T2_MOVE;
}

//=============
// SetVelocity
//=============
void ClientGameCommandManager::SetVelocity(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

    m_spawnthing->forwardVelocity = ev->GetFloat(1);
    m_spawnthing->cgd.flags2 |= T2_MOVE;
}

//=============
// SetAngularVelocity
//=============
void ClientGameCommandManager::SetAngularVelocity(Event *ev)
{
    int    i = 1;
    int    j = 0;
    Vector randval;
    str    vel;

    if (!m_spawnthing) {
        return;
    }

    if (ev->NumArgs() < 3) {
        warning("ClientGameCommandManager::SetAngularVelocity", "Expecting at least 3 args for command randvel");
    }

    SetBaseAndAmplitude(ev, m_spawnthing->avelocity_base, m_spawnthing->avelocity_amplitude);

    m_spawnthing->cgd.flags2 |= T2_AMOVE;
}

//=============
// SetAngles
//=============
void ClientGameCommandManager::SetAngles(Event *ev)
{
    int    i = 1;
    int    j = 0;
    Vector randval;
    str    vel;

    if (!m_spawnthing) {
        return;
    }

    if (ev->NumArgs() < 3) {
        warning("ClientGameCommandManager::SetAngles", "Expecting at least 3 args for command randvel");
    }

    SetBaseAndAmplitude(ev, m_spawnthing->cgd.angles, m_spawnthing->angles_amplitude);

    // Set the tag axis
    m_spawnthing->cgd.flags |= T_ANGLES;
}

//=============
// SetRelativeAngles
//=============
void ClientGameCommandManager::SetRelativeAngles(Event* ev)
{
    if (!m_spawnthing) {
        return;
    }

    m_spawnthing->cgd.flags2 |= T2_RELATIVEANGLES;
}

//=============
// ParentAngles
//=============
void ClientGameCommandManager::ParentAngles(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

    if (current_centity) {
        m_spawnthing->cgd.angles = Vector(current_centity->currentState.angles);
    }
    m_spawnthing->cgd.flags |= T_ANGLES;
}

void ClientGameCommandManager::EmitterAngles(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

    MatrixToEulerAngles(m_spawnthing->tag_axis, m_spawnthing->cgd.angles);
    if (ev->NumArgs() > 0) {
        m_spawnthing->cgd.angles[0] = ev->GetFloat(1);
    }
    if (ev->NumArgs() > 1) {
        m_spawnthing->cgd.angles[1] = ev->GetFloat(2);
    }
    if (ev->NumArgs() > 2) {
        m_spawnthing->cgd.angles[2] = ev->GetFloat(3);
    }

    m_spawnthing->cgd.flags |= T_ANGLES;
}

//=============
// SetAccel
//=============
void ClientGameCommandManager::SetAccel(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

    m_spawnthing->cgd.accel[0] = ev->GetFloat(1);
    m_spawnthing->cgd.accel[1] = ev->GetFloat(2);
    m_spawnthing->cgd.accel[2] = ev->GetFloat(3);

    m_spawnthing->cgd.flags2 |= T2_ACCEL;
}

void ClientGameCommandManager::SetFriction(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

    m_spawnthing->cgd.friction = ev->GetFloat(1);
    m_spawnthing->cgd.flags2 |= T2_FRICTION;
}

//=============
// SetSpin
//=============
void ClientGameCommandManager::SetSpin(Event *ev)
{
    if (ev->NumArgs() != 1) {
        throw ScriptException("spin needs rotations per second specified\n");
    }

    if (!m_spawnthing) {
        return;
    }

    m_spawnthing->cgd.spin_rotation = ev->GetFloat(1);
    m_spawnthing->cgd.flags2 |= T2_SPIN;
}

//=============
// SetVaryColor
//=============
void ClientGameCommandManager::SetVaryColor(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

    m_spawnthing->cgd.flags2 |= T2_VARYCOLOR;
}

//=============
// SetCount
//=============
void ClientGameCommandManager::SetCount(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

    m_spawnthing->count = ev->GetInteger(1);
}

//=============
// SetLife
//=============
void ClientGameCommandManager::SetLife(Event *ev)
{
    str life;

    if (!m_spawnthing) {
        return;
    }

    life = ev->GetString(1);

    if (!life.icmp("autocalc")) {
        m_spawnthing->cgd.flags |= T_AUTOCALCLIFE;
    } else {
        m_spawnthing->cgd.life = atof(life) * 1000;
        if (ev->NumArgs() > 1) {
            m_spawnthing->life_random = ev->GetFloat(2) * 1000;
        }
    }
}

//=============
// SetModel
//=============
void ClientGameCommandManager::SetModel(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

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
void ClientGameCommandManager::TagList(Event *ev)
{
    int i;
    int num = ev->NumArgs();

    if (!m_spawnthing) {
        return;
    }

    if (num < 2) {
        warning(
            "CCG::TagList",
            "Invalid number of tags specified in taglist.  "
            "Minimum set of tags is 2.\n"
        );
        return;
    }

    for (i = 1; i <= num; i++) {
        str s_arg(ev->GetString(i));
        m_spawnthing->m_taglist.AddObject(s_arg);
    }
}

void ClientGameCommandManager::SetEyeLimits(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

    if (ev->NumArgs() < 3) {
        warning("CCG::SetEyeLimits", "Invalid number of parameters.\n");
        return;
    }

    cg.vEyeOffsetMax[0] = ev->GetFloat(1);
    cg.vEyeOffsetMax[2] = ev->GetFloat(2);
    cg.fEyeOffsetFrac   = ev->GetFloat(3);
}

void ClientGameCommandManager::SetEyeMovement(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

    if (ev->NumArgs() < 1) {
        warning("CCG::SetEyeMovement", "No parameters specified.\n");
        return;
    }

    cg.fEyeOffsetFrac = ev->GetFloat(1);
}

//=============
// InitializeSpawnthing
//=============
spawnthing_t *ClientGameCommandManager::InitializeSpawnthing(spawnthing_t *sp)
{
    int i;

    // Initalize m_spawnthing - these can be overidden with other commands
    sp->m_modellist.ClearObjectList();
    AxisClear(sp->axis);
    AxisClear(sp->tag_axis);

    sp->startoff                = qfalse;
    sp->origin_offset_base      = Vector(0, 0, 0);
    sp->origin_offset_amplitude = Vector(0, 0, 0);
    sp->axis_offset_base        = Vector(0, 0, 0);
    sp->axis_offset_amplitude   = Vector(0, 0, 0);
    sp->randvel_base            = Vector(0, 0, 0);
    sp->randvel_amplitude       = Vector(0, 0, 0);
    sp->avelocity_base          = Vector(0, 0, 0);
    sp->avelocity_amplitude     = Vector(0, 0, 0);
    sp->angles_amplitude        = Vector(0, 0, 0);
    sp->life_random             = 0;
    sp->forwardVelocity         = 0;
    sp->sphereRadius            = 0;
    sp->coneHeight              = 0;
    sp->spawnRate               = 0.1f;
    sp->count                   = 1;
    sp->length                  = 1000;
    sp->max_offset              = 0;
    sp->min_offset              = 0;
    sp->overlap                 = 0;
    sp->numSubdivisions         = 1;
    sp->delay                   = 0;
    sp->toggledelay             = 0;
    sp->endalpha                = 1.0f;
    sp->spreadx                 = 0;
    sp->spready                 = 0;
    sp->beamflags               = 0;
    sp->startoff                = 0;
    sp->numspherebeams          = 0;
    sp->use_last_trace_end      = qfalse;
    sp->tagnum                  = -1;
    sp->numtempmodels           = 0;
    sp->entnum                  = ENTITYNUM_NONE;

    sp->cgd.scale              = 1.0f;
    sp->cgd.createTime         = cg.time;
    sp->cgd.life               = EMITTER_DEFAULT_LIFE;
    sp->cgd.avelocity          = Vector(0, 0, 0);
    sp->cgd.accel              = Vector(0, 0, 0);
    sp->cgd.origin             = Vector(0, 0, 0);
    sp->cgd.oldorigin          = Vector(0, 0, 0);
    sp->cgd.angles             = Vector(0, 0, 0);
    sp->cgd.alpha              = 1.0f;
    sp->cgd.scalemin           = 0;
    sp->cgd.scalemax           = 999999.0f;
    sp->cgd.scaleRate          = 0;
    sp->cgd.bouncefactor       = 0.3f;
    sp->cgd.bouncesound        = "";
    sp->cgd.bouncesound_delay  = 1000;
    sp->cgd.bouncecount        = 0;
    sp->cgd.maxbouncecount     = 3;
    sp->cgd.flags              = 0;
    sp->cgd.flags2             = 0;
    sp->cgd.swarmfreq          = 0;
    sp->cgd.swarmdelta         = 0;
    sp->cgd.swarmmaxspeed      = 0;
    sp->cgd.fadeintime         = 0;
    sp->cgd.fadedelay          = 0;
    sp->cgd.lightIntensity     = 0;
    sp->cgd.lightType          = 0;
    sp->cgd.collisionmask      = MASK_AUTOCALCLIFE;
    sp->cgd.parent             = -1;
    sp->cgd.tiki               = nullptr;
    sp->cgd.lightstyle         = -1;
    sp->cgd.physicsRate        = 10;
    sp->cgd.shadername         = "beamshader";
    sp->cgd.decal_orientation  = 0;
    sp->cgd.decal_radius       = 10;
    sp->cgd.max_twinkletimeoff = 0;
    sp->cgd.min_twinkletimeoff = 0;
    sp->cgd.max_twinkletimeon  = 0;
    sp->cgd.min_twinkletimeon  = 0;
    sp->cgd.swipe_life         = 0;
    sp->cgd.scale2             = 1;
    sp->cgd.spin_rotation      = 0;
    sp->fMinRangeSquared       = 0;
    sp->fMaxRangeSquared       = 9.9999997e37f;

    for (i = 0; i < 3; i++) {
        sp->dcolor[i]    = 1.0f;
        sp->cgd.color[i] = 1.0;
    }

    sp->cgd.color[3] = 1.0;

    return sp;
}

void ClientGameCommandManager::SpawnEffect(int count, spawnthing_t *sp)
{
    m_spawnthing = sp;
    if (m_spawnthing->cgd.flags2 & T2_VOLUMETRIC) {
        // volumetric smoke
        SpawnVSSSource(count, 0);
    } else {
        SpawnTempModel(count);
    }
    m_spawnthing = NULL;
}

void ClientGameCommandManager::SpawnEffect(int count, int timealive)
{
    if (!m_spawnthing) {
        return;
    }

    if (m_spawnthing->cgd.flags2 & T2_VOLUMETRIC) {
        // volumetric smoke
        SpawnVSSSource(count, 0);
    } else {
        SpawnTempModel(count);
    }
}

//==================
// GetEmitterByName
//==================
spawnthing_t *ClientGameCommandManager::GetEmitterByName(str name)
{
    int i;
    for (i = 1; i <= m_emitters.NumObjects(); i++) {
        spawnthing_t *st = m_emitters.ObjectAt(i);
        if (st->emittername == name) {
            return st;
        }
    }
    return NULL;
}

//==================
// CreateNewEmitter
//==================
spawnthing_t *ClientGameCommandManager::CreateNewEmitter(str name)
{
    spawnthing_t *st;

    st = new spawnthing_t;

    // Init the emitter and set the internal pointer at it
    InitializeSpawnthing(st);

    // Set the emitter's name
    st->emittername = name;

    // Store it in the container
    m_emitters.AddObject(st);

    return st;
}

void ClientGameCommandManager::DeleteEmitters(dtiki_t *tiki)
{
    int           i;
    spawnthing_t *spawnthing;

    for (i = m_emitters.NumObjects(); i > 0; i--) {
        spawnthing = m_emitters.ObjectAt(i);
        if (spawnthing->cgd.tiki == tiki) {
            FreeSpawnthing(spawnthing);
        }
    }
}

void CG_DeleteEmitters(dtiki_t *tiki)
{
    commandManager.DeleteEmitters(tiki);
}

//==================
// CreateNewEmitter
//==================
spawnthing_t *ClientGameCommandManager::CreateNewEmitter(void)
{
    return CreateNewEmitter("");
}

//===============
// BeginOriginSpawn
//===============
void ClientGameCommandManager::BeginOriginSpawn(Event *ev)
{
    if (!current_entity) {
        m_spawnthing = NULL;
        return;
    }

    // Setup ending function
    endblockfcn = &ClientGameCommandManager::EndOriginSpawn;

    if (!m_pCurrentSfx) {
        m_spawnthing = &m_localemitter;
    }
    if (!m_spawnthing) return;
    InitializeSpawnthing(m_spawnthing);

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
    if (m_spawnthing) {
        SpawnEffect(m_spawnthing->count, 0);
    }
}

void ClientGameCommandManager::TestEffectEndFunc()
{
    if (!m_spawnthing) {
        return;
    }

    SpawnEffect(m_spawnthing->count, 0);
}

//===============
// UpdateSpawnThing
//===============
void ClientGameCommandManager::UpdateSpawnThing(spawnthing_t *ep)
{
    int           i;
    orientation_t orientation;

    VectorCopy(current_entity->origin, ep->cgd.origin);

    for (i = 0; i < 3; i++) {
        VectorMA(ep->cgd.origin, orientation.origin[i], current_entity->axis[i], ep->cgd.origin);
    }

    MatrixMultiply(orientation.axis, current_entity->axis, ep->axis);
}

//===============
// BeginTagEmitter
//===============
void ClientGameCommandManager::BeginTagEmitter(Event *ev)
{
    str tagname;
    int tagnum;

    if (current_entity) {
        m_spawnthing = NULL;
        return;
    }

    // Get the tagname and orientation
    tagname = ev->GetString(1);

    if (!tagname.length()) {
        warning(
            "CCM::BeginTagEmitter", "Tagname not specified for tagemitter in model: '%s'\n", cgi.TIKI_Name(current_tiki)
        );
    }

    tagnum = cgi.Tag_NumForName(current_tiki, tagname.c_str());
    if (tagnum == -1) {
        throw ScriptException("Tagname '%s' does not exist", tagname.c_str());
    }

    // Setup ending function
    endblockfcn = &ClientGameCommandManager::EndTagEmitter;

    // Init the emitter
    m_spawnthing              = CreateNewEmitter();
    m_spawnthing->tagnum      = tagnum;
    m_spawnthing->emittername = ev->GetString(2);
    if (!m_spawnthing->emittername.length()) {
        warning(
            "CCM::BeginTagEmitter",
            "Emittername not specified for tagemitter in model: '%s'\n",
            cgi.TIKI_Name(current_tiki)
        );
    }

    m_spawnthing->cgd.tiki = current_tiki;
}

//===============
// EndTagEmitter
//===============
void ClientGameCommandManager::EndTagEmitter(void)
{
    endblockfcn = NULL;
}

//===============
// BeginTagBeamEmitter
//===============
void ClientGameCommandManager::BeginTagBeamEmitter(Event *ev)
{
    if (current_entity) {
        m_spawnthing = NULL;
        return;
    }

    // Setup ending function
    endblockfcn = &ClientGameCommandManager::EndTagBeamEmitter;

    // Init the emitter
    m_spawnthing = CreateNewEmitter();
    m_spawnthing->cgd.flags |= T_BEAMTHING;

    // Get the tagname and orientation

    m_spawnthing->startTag = ev->GetString(1);

    if (m_spawnthing->startTag == "USE_TAGLIST") {
        if (ev->NumArgs() == 2) {
            m_spawnthing->emittername = ev->GetString(2);
        }
    } else {
        if (ev->NumArgs() == 3) {
            m_spawnthing->endTag      = ev->GetString(2);
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
void ClientGameCommandManager::EndTagBeamEmitter(void)
{
    endblockfcn = NULL;
}

//===============
// BeginOriginEmitter
//===============
void ClientGameCommandManager::BeginOriginEmitter(Event *ev)
{
    if (current_entity) {
        m_spawnthing = NULL;
        return;
    }

    // Setup ending function
    endblockfcn = &ClientGameCommandManager::EndOriginEmitter;

    // Init the emitter
    m_spawnthing = CreateNewEmitter();

    // Get the emitter's name
    m_spawnthing->emittername = ev->GetString(1);

    if (!m_spawnthing->emittername.length()) {
        warning(
            "CCM::BeginOriginEmitter",
            "Emittername not specified for originemitter in model: '%s'\n",
            cgi.TIKI_Name(current_tiki)
        );
    }

    m_spawnthing->cgd.tiki = current_tiki;
}

//===============
// EndOriginEmitter
//===============
void ClientGameCommandManager::EndOriginEmitter(void)
{
    endblockfcn = NULL;
}

//===============
// BeginOriginBeamEmitter
//===============
void ClientGameCommandManager::BeginOriginBeamEmitter(Event *ev)
{
    // Init the emitter
    if (current_entity) {
        m_spawnthing = NULL;
        return;
    }

    // Setup ending function
    endblockfcn = &ClientGameCommandManager::EndOriginBeamEmitter;

    m_spawnthing = CreateNewEmitter();
    // Get the emitter's name
    m_spawnthing->emittername = ev->GetString(1);

    if (!m_spawnthing->emittername.length()) {
        warning(
            "CCM::BeginOriginEmitter",
            "Emittername not specified for originemitter in model: '%s'\n",
            cgi.TIKI_Name(current_tiki)
        );
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
void ClientGameCommandManager::GetOrientation(int tagnum, spawnthing_t *sp)
{
    int           i;
    orientation_t _or;

    assert(current_entity);
    assert(current_tiki);

    if (!current_entity || !current_tiki) {
        return;
    }

    _or = cgi.TIKI_Orientation(current_entity, tagnum);

    VectorCopy(current_entity->origin, sp->cgd.origin);

    for (i = 0; i < 3; i++) {
        VectorMA(sp->cgd.origin, _or.origin[i], current_entity->axis[i], sp->cgd.origin);
    }

    MatrixMultiply(_or.axis, current_entity->axis, sp->axis);

    // If angles are not set, then use the angles from the tag
    if (!(sp->cgd.flags & T_ANGLES)) {
        vectoangles(sp->axis[0], sp->cgd.angles);
    }

    AxisCopy(sp->axis, sp->tag_axis);
}

//===============
// BeginTagSpawnLinked
//===============
void ClientGameCommandManager::BeginTagSpawnLinked(Event *ev) {
    str tagname;
    int tagnum;
    int i;
    orientation_t ori;

    // Get the tagname and orientation
    tagname = ev->GetString(1);
    tagnum = cgi.Tag_NumForName(current_tiki, tagname.c_str());
    if (tagnum == -1) {
        throw ScriptException("Tagname '%s' does not exist", tagname.c_str());
    }

    // Setup ending function
    endblockfcn = &ClientGameCommandManager::EndTagSpawn;

    // Create a new emitter
    m_spawnthing = CreateNewEmitter();
    m_spawnthing->tagnum = tagnum;
    m_spawnthing->entnum = current_entity->entityNumber;
    m_spawnthing->cgd.tiki = current_tiki;
    m_spawnthing->cgd.flags |= T_WAVE;
    m_spawnthing->cgd.origin = Vector(0, 0, 0);

    ori = cgi.TIKI_Orientation(current_entity, tagnum);

    VectorCopy(current_entity->origin, m_spawnthing->linked_origin);

    for (i = 0; i < 3; i++) {
        VectorMA(m_spawnthing->linked_origin, ori.origin[i], current_entity->axis[i], m_spawnthing->linked_origin);
    }

    MatrixMultiply(ori.axis, current_entity->axis, m_spawnthing->linked_axis);
}

//===============
// BeginTagSpawn
//===============
void ClientGameCommandManager::BeginTagSpawn(Event *ev)
{
    str tagname;
    int tagnum;

    // Get the tagname and orientation
    tagname = ev->GetString(1);
    tagnum  = cgi.Tag_NumForName(current_tiki, tagname.c_str());
    if (tagnum == -1) {
        throw ScriptException("Tagname '%s' does not exist", tagname.c_str());
    }

    // Setup ending function
    endblockfcn = &ClientGameCommandManager::EndTagSpawn;

    if (!m_pCurrentSfx) {
        // Init the thing we are going to spawn
        m_spawnthing = &m_localemitter;
    }
    InitializeSpawnthing(m_spawnthing);

    GetOrientation(tagnum, m_spawnthing);
}

//===============
// EndTagSpawn
//===============
void ClientGameCommandManager::EndTagSpawn(void)
{
    // Okay we should have a valid spawnthing, let's create a render entity
    SpawnEffect(m_spawnthing->count, 0);
}

//===============
// BeginTagBeamShoot
//===============
void ClientGameCommandManager::BeginTagBeamSpawn(Event *ev)
{
    str tagname;
    int tagnum;

    // Get the tagname and orientation
    tagname = ev->GetString(1);
    tagnum  = cgi.Tag_NumForName(current_tiki, tagname.c_str());
    if (tagnum == -1) {
        throw ScriptException("Tagname '%s' does not exist", tagname.c_str());
    }

    // Setup ending function
    endblockfcn = &ClientGameCommandManager::EndTagBeamSpawn;

    if (!m_pCurrentSfx) {
        // Init the thing we are going to spawn
        m_spawnthing = &m_localemitter;
    }
    InitializeSpawnthing(m_spawnthing);

    GetOrientation(tagnum, m_spawnthing);
}

//===============
// EndTagSpawn
//===============
void ClientGameCommandManager::EndTagBeamSpawn(void)
{
    // Okay we should have a valid spawnthing, let's create the beam now
    int     i;
    int     renderfx;
    Vector  end;
    trace_t trace;
    float   scale = 1.0f;

    if (current_entity) {
        scale = current_entity->scale;
    }

    for (i = 0; i < 3; i++) {
        m_spawnthing->cgd.origin +=
            Vector(m_spawnthing->tag_axis[i])
            * (m_spawnthing->axis_offset_base[i] + m_spawnthing->axis_offset_amplitude[i] * random());
    }

    if (m_spawnthing->use_last_trace_end) {
        end = last_trace_end;
    } else {
        end = m_spawnthing->cgd.origin + Vector(m_spawnthing->axis[0]) * m_spawnthing->length
            + Vector(m_spawnthing->axis[1]) * crandom() * m_spawnthing->spreadx
            + Vector(m_spawnthing->axis[2]) * crandom() * m_spawnthing->spready;
        last_trace_end = end;
    }

    CG_Trace(
        &trace,
        m_spawnthing->cgd.origin,
        vec_zero,
        vec_zero,
        end,
        ENTITYNUM_NONE,
        MASK_SHOT,
        qfalse,
        qtrue,
        "EndTagBeamSpawn"
    );

    if (current_entity) {
        renderfx = (current_entity->renderfx & ~(RF_FLAGS_NOT_INHERITED | RF_LIGHTING_ORIGIN));
    } else {
        renderfx = 0;
    }

    CG_CreateBeam(
        m_spawnthing->cgd.origin,
        vec_zero,
        current_entity_number,
        1,
        m_spawnthing->cgd.alpha,
        m_spawnthing->cgd.scale * scale,
        (m_spawnthing->beamflags | BEAM_LIGHTNING_EFFECT),
        m_spawnthing->length,
        m_spawnthing->cgd.life,
        qfalse,
        trace.endpos,
        m_spawnthing->min_offset,
        m_spawnthing->max_offset,
        m_spawnthing->overlap,
        m_spawnthing->numSubdivisions,
        m_spawnthing->delay,
        m_spawnthing->cgd.shadername,
        m_spawnthing->cgd.color,
        m_spawnthing->numspherebeams,
        m_spawnthing->sphereRadius,
        m_spawnthing->toggledelay,
        m_spawnthing->endalpha,
        renderfx,
        m_spawnthing->emittername
    );
}

//===============
// BeginOriginBeamSpawn
//===============
void ClientGameCommandManager::BeginOriginBeamSpawn(Event *ev)
{
    if (!m_pCurrentSfx) {
        m_spawnthing = &m_localemitter;
    }
    // Init the emitter
    InitializeSpawnthing(m_spawnthing);

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
    int     i, renderfx, c, count;
    Vector  end;
    trace_t trace;
    float   scale = 1.0f;

    if (current_entity) {
        scale = current_entity->scale;
    }

    count = m_spawnthing->count * cg_effectdetail->value;

    for (c = 0; c <= count; c++) {
        Vector angles;

        for (i = 0; i < 3; i++) {
            // Randomize angles or set absolute
            angles[i] = m_spawnthing->cgd.angles[i] + random() * m_spawnthing->angles_amplitude[i];

            m_spawnthing->cgd.origin +=
                Vector(m_spawnthing->tag_axis[i])
                * (m_spawnthing->axis_offset_base[i] + m_spawnthing->axis_offset_amplitude[i] * random());
        }

        AnglesToAxis(angles, m_spawnthing->axis);
        end = m_spawnthing->cgd.origin + Vector(m_spawnthing->axis[0]) * m_spawnthing->length
            + Vector(m_spawnthing->axis[1]) * crandom() * m_spawnthing->spreadx
            + Vector(m_spawnthing->axis[2]) * crandom() * m_spawnthing->spready;

        CG_Trace(
            &trace,
            m_spawnthing->cgd.origin,
            vec_zero,
            vec_zero,
            end,
            ENTITYNUM_NONE,
            MASK_SHOT,
            qfalse,
            qtrue,
            "EndOriginBeamSpawn"
        );

        if (current_entity) {
            renderfx = (current_entity->renderfx & ~(RF_FLAGS_NOT_INHERITED | RF_LIGHTING_ORIGIN));
        } else {
            renderfx = 0;
        }

        CG_CreateBeam(
            m_spawnthing->cgd.origin,
            vec_zero,
            current_entity_number,
            1,
            m_spawnthing->cgd.alpha,
            m_spawnthing->cgd.scale * scale,
            (m_spawnthing->beamflags | BEAM_LIGHTNING_EFFECT),
            m_spawnthing->length,
            m_spawnthing->cgd.life,
            qtrue,
            trace.endpos,
            m_spawnthing->min_offset,
            m_spawnthing->max_offset,
            m_spawnthing->overlap,
            m_spawnthing->numSubdivisions,
            m_spawnthing->delay,
            m_spawnthing->cgd.shadername,
            m_spawnthing->cgd.color,
            m_spawnthing->numspherebeams,
            m_spawnthing->sphereRadius,
            m_spawnthing->toggledelay,
            m_spawnthing->endalpha,
            renderfx,
            m_spawnthing->emittername
        );
    }
}

//=============
// InitializeEmitters
//=============
void ClientGameCommandManager::InitializeEmitters(void) {}

//===============
// EmitterOn
//===============
void ClientGameCommandManager::EmitterOn(Event *ev)
{
    int i;
    str name;

    name = ev->GetString(1);

    for (i = 1; i <= m_emitters.NumObjects(); i++) {
        spawnthing_t *st = m_emitters.ObjectAt(i);

        if (st->emittername == name) {
            emittertime_t *et;

            et = st->GetEmitTime(current_entity_number);

            et->active         = qtrue;
            et->last_emit_time = cg.time;
            et->lerp_emitter   = qfalse;
        }
    }
}

//===============
// EmitterOff
//===============
void ClientGameCommandManager::EmitterOff(Event *ev)
{
    int i;
    str name;

    name = ev->GetString(1);

    for (i = 1; i <= m_emitters.NumObjects(); i++) {
        spawnthing_t *st = m_emitters.ObjectAt(i);

        if (st->emittername == name) {
            emittertime_t *et;

            et         = st->GetEmitTime(current_entity_number);
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
void ClientGameCommandManager::PlaySound(
    str sound_name, const vec3_t origin, int channel, float volume, float min_distance, float pitch, int argstype
)
{
    int              aliaschannel;
    float            aliasvolume;
    float            aliasmin_distance;
    float            aliaspitch;
    const char      *name       = NULL;
    static cvar_t   *g_subtitle = cgi.Cvar_Get("g_subtitle", "0", CVAR_ARCHIVE);
    static cvar_t   *debugSound = cgi.Cvar_Get("debugSound", "0", 0);
    float            maxDist;
    AliasListNode_t *soundAlias = NULL;

    // FIXME
    // TODO: Pitch modulation
    // TODO: Alias enhancement

    // Get the real sound to play
    if (current_tiki && current_tiki->a->alias_list) {
        name = cgi.Alias_ListFindRandom((AliasList_t *)current_tiki->a->alias_list, sound_name.c_str(), &soundAlias);
    }

    if (!name) {
        name = cgi.Alias_FindRandom(sound_name.c_str(), &soundAlias);
    }

    if (!name || !soundAlias) {
        Com_Printf(
            "\nERROR PlaySound: %s needs an alias in ubersound.scr or uberdialog.scr - Please fix.\n",
            sound_name.c_str()
        );
        return;
    }

    aliasvolume       = soundAlias->volumeMod * random() + soundAlias->volume;
    aliaspitch        = soundAlias->pitchMod * random() + soundAlias->pitch;
    aliasmin_distance = soundAlias->dist;
    aliaschannel      = soundAlias->channel;
    maxDist           = soundAlias->maxDist;

    if (soundAlias->subtitle) {
        if (g_subtitle->integer || !sound_name.icmpn("den", 3) || !sound_name.icmpn("snd_den", 6)) {
            // show subtitles if near the sound
            if (origin
                && (g_subtitle->integer == 2 || DistanceSquared(origin, cg.refdef.vieworg) < (maxDist * maxDist))) {
                int curSub;

                // also put subtitles if it's from a german dialogue
                curSub = cgi.Cvar_Get("curSubtitle", "0", 0)->integer;
                cgi.Cvar_Set(va("subtitle%d", curSub), va("%s", soundAlias->subtitle));
                // increment the subtitle
                cgi.Cvar_Set("curSubtitle", va("%d", (curSub + 1) % 4));
            }
        }
    }

    if (argstype) {
        if (argstype == 1) {
            if (debugSound->integer) {
                Com_Printf("WARNING: Sound %s had its parm modified by code.\n", sound_name.c_str());
            }

            if (volume >= 0.0) {
                volume = volume * aliasvolume;
            } else {
                volume = aliasvolume;
            }

            if (pitch >= 0.0) {
                pitch = pitch * aliaspitch;
            } else {
                pitch = aliaspitch;
            }

            if (min_distance < 0.0) {
                min_distance = aliasmin_distance;
            }

            if (channel < 0) {
                channel = aliaschannel;
            }
        } else {
            if (debugSound->integer) {
                Com_Printf(
                    "\nWARNING: OVERRIDE OVERRIDE OVERRIDESound %s had all its parm overriden by code.\n\n",
                    sound_name.c_str()
                );
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
    } else {
        volume       = aliasvolume;
        pitch        = aliaspitch;
        min_distance = aliasmin_distance;
        channel      = aliaschannel;
    }

    if (current_entity_number == -1) {
        cgi.S_StartSound(
            origin,
            ENTITYNUM_NONE,
            channel,
            cgi.S_RegisterSound(name, soundAlias->streamed),
            volume,
            min_distance,
            pitch,
            soundAlias->maxDist,
            soundAlias->streamed
        );
    } else {
        cgi.S_StartSound(
            origin,
            current_entity_number,
            channel,
            cgi.S_RegisterSound(name, soundAlias->streamed),
            volume,
            min_distance,
            pitch,
            soundAlias->maxDist,
            soundAlias->streamed
        );
    }
}

//===============
// Sound
//===============
void ClientGameCommandManager::Sound(Event *ev)
{
    int   channel = CHAN_AUTO;
    str   sound_name;
    float volume       = -1.0;
    float min_distance = -1.0;

    if (!current_entity) {
        // don't play sound for nonexistent entities
        return;
    }

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
        PlaySound(sound_name, current_entity->origin, channel, volume, min_distance);
    } else {
        PlaySound(sound_name, NULL, channel, volume, min_distance);
    }
}

//===============
// StopSound
//===============
void ClientGameCommandManager::StopSound(Event *ev)
{
    int channel;

    if (ev->NumArgs() > 0) {
        channel = ev->GetInteger(1);
    } else {
        return;
    }

    cgi.S_StopSound(current_entity_number, channel);
}

void ClientGameCommandManager::StopAliasChannel(Event *ev)
{
    str              sound_name;
    const char      *name;
    AliasListNode_t *soundAlias;

    name       = NULL;
    soundAlias = NULL;

    if (ev->NumArgs() < 1) {
        return;
    }

    sound_name = ev->GetString(1);
    if (current_tiki) {
        if (current_tiki->a->alias_list) {
            name =
                cgi.Alias_ListFindRandom((AliasList_t *)current_tiki->a->alias_list, sound_name.c_str(), &soundAlias);
        }
    }

    if (!name) {
        name = cgi.Alias_FindRandom(sound_name.c_str(), &soundAlias);
    }

    if (!name || !soundAlias) {
        Com_Printf("\nERROR stopaliaschannel: couldn't find alias %s\n", sound_name.c_str());
        return;
    }

    cgi.S_StopSound(current_entity_number, soundAlias->channel);
}

//===============
// LoopSound
//===============
void ClientGameCommandManager::LoopSound(Event *ev)
{
    str              sound_name;
    float            volume     = -1.0;
    float            min_dist   = -1.0;
    float            max_dist   = -1.0;
    float            pitch      = 1.0;
    const char      *name       = NULL;
    AliasListNode_t *soundAlias = NULL;

    if (!current_centity) {
        cgi.DPrintf("CCM::LoopSound : LoopSound in %s without current_centity\n", cgi.TIKI_Name(current_tiki));
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

    if (current_tiki && current_tiki->a->alias_list) {
        name = cgi.Alias_ListFindRandom((AliasList_t *)current_tiki->a->alias_list, sound_name.c_str(), &soundAlias);
    }

    if (!name) {
        name = cgi.Alias_FindRandom(sound_name.c_str(), &soundAlias);
    }

    if (!name) {
        name = sound_name.c_str();
    }

    current_centity->tikiLoopSound        = cgi.S_RegisterSound(name, qfalse);
    current_centity->tikiLoopSoundVolume  = volume;
    current_centity->tikiLoopSoundMinDist = min_dist;
    current_centity->tikiLoopSoundMaxDist = max_dist;
    current_centity->tikiLoopSoundPitch   = pitch;
    current_centity->tikiLoopSoundFlags   = 0;
}

//===============
// CacheResource
//===============
void CacheResource(const char *stuff)
{
    str real_stuff;

    if (!stuff || !stuff[0]) {
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
void ClientGameCommandManager::Cache(Event *ev)
{
    if (ev->NumArgs() < 1) {
        return;
    }

    CacheResource(ev->GetString(1));
}

void ClientGameCommandManager::CacheImage(Event *ev)
{
    // FIXME: stub
}

void ClientGameCommandManager::CacheFont(Event *ev)
{
    // FIXME: stub
}

//===============
// AliasResource
//===============
void AliasResource(dtiki_t *pmdl, const char *alias, const char *realname, const char *parameters)
{
    if (pmdl) {
        if (!pmdl->a->alias_list) {
            pmdl->a->alias_list = cgi.AliasList_New(pmdl->a->name);
        }

        cgi.Alias_ListAdd((AliasList_t *)pmdl->a->alias_list, alias, realname, parameters);
    } else {
        cgi.Alias_Add(alias, realname, parameters);
    }
}

qboolean bLoadForMap(const char *psMapsBuffer, const char *name)
{
    const char *token;
    const char *mapname;
    const char *override;

    // ignore the "maps/"
    mapname = cgs.mapname + strlen("maps/");
    if (!strncmp("test", mapname, strlen("test"))) {
        return true;
    }

    token = COM_GetToken((char **)&psMapsBuffer, qtrue);
    if (!psMapsBuffer) {
        return false;
    }

    if (!token || !token[0]) {
        Com_Printf("ERROR bLoadForMap: %s alias with empty maps specification.\n", name);
        return false;
    }

    override = token;
    while (token && token[0]) {
        if (!Q_stricmpn(token, mapname, strlen(token))) {
            // found the map
            return true;
        }

        token = COM_GetToken((char **)&psMapsBuffer, qtrue);
    }

    return false;
}

//===============
// AliasCache
//===============
void ClientGameCommandManager::AliasCache(Event *ev)
{
    int  i;
    char parmbuffer[2048]; // this holds the parameters to be passed into the
                           // alias command
    const char *psMapsBuffer;
    bool        bAlwaysLoaded = false;

    if (ev->NumArgs() < 2) {
        return;
    }

    // alias is in argument 1
    // real path is argument 2
    // any additional parameters are in arguments 3-n

    parmbuffer[0] = 0;
    psMapsBuffer  = NULL;

    for (i = 3; i <= ev->NumArgs(); i++) {
        if (!strcmp(ev->GetToken(i).c_str(), "maps")) {
            i++;
            psMapsBuffer = ev->GetToken(i);
        } else if (!strcmp(ev->GetToken(i).c_str(), "always")) {
            bAlwaysLoaded = true;
        } else {
            strcat(parmbuffer, ev->GetToken(i));
            strcat(parmbuffer, " ");
        }
    }

    if (bAlwaysLoaded || bLoadForMap(psMapsBuffer, ev->GetString(1))) {
        AliasResource(current_tiki, ev->GetString(1), ev->GetString(2), parmbuffer);
        CacheResource(ev->GetString(2));
    }
}

//===============
// Alias
//===============
void ClientGameCommandManager::Alias(Event *ev)
{
    int  i;
    char parmbuffer[2048]; // this holds the parameters to be passed into the
                           // alias command
    qboolean    subtitle;
    const char *psMapsBuffer;
    bool        bAlwaysLoaded = false;

    if (ev->NumArgs() < 2) {
        return;
    }

    // alias is in argument 1
    // real path is argument 2
    // any additional parameters are in arguments 3-n

    parmbuffer[0] = 0;
    subtitle      = qfalse;
    psMapsBuffer  = NULL;

    for (i = 3; i <= ev->NumArgs(); i++) {
        if (!strcmp(ev->GetToken(i).c_str(), "maps")) {
            i++;
            psMapsBuffer = ev->GetToken(i);
        } else if (!strcmp(ev->GetToken(i).c_str(), "always")) {
            bAlwaysLoaded = true;
        } else {
            if (!subtitle) {
                if (!Q_stricmp(ev->GetToken(i), "subtitle") || !Q_stricmp(ev->GetToken(i), "forcesubtitle")) {
                    subtitle = qtrue;
                    strcat(parmbuffer, ev->GetToken(i));
                } else {
                    strcat(parmbuffer, ev->GetToken(i));
                }
            } else {
                strcat(parmbuffer, "\"");
                strcat(parmbuffer, ev->GetToken(i));
                strcat(parmbuffer, "\"");
                subtitle = qfalse;
            }

            strcat(parmbuffer, " ");
        }
    }

    if (bAlwaysLoaded || bLoadForMap(psMapsBuffer, ev->GetString(1))) {
        AliasResource(current_tiki, ev->GetString(1), ev->GetString(2), parmbuffer);
    }
}

void CacheAliasList(AliasList_t *alias_list, str& name)
{
    if (!alias_list) {
        return;
    }

    int   i;
    int   iFirst, iLast;
    float fTotalWeight;

    cgi.Alias_ListFindRandomRange((AliasList_t *)alias_list, name.c_str(), &iFirst, &iLast, &fTotalWeight);
    if (iFirst >= 0 && iLast >= iFirst) {
        for (i = iFirst; i <= iLast; ++i) {
            CacheResource(alias_list->sorted_list[i]->real_name);
        }
    }
}

void ClientGameCommandManager::CacheAlias(Event *ev)
{
    if (ev->NumArgs() < 1) {
        return;
    }

    str name = ev->GetString(1);

    if (current_tiki) {
        CacheAliasList((AliasList_t *)current_tiki->a->alias_list, name);
    }

    CacheAliasList(cgi.Alias_GetGlobalList(), name);
}

//===============
// Footstep
//===============
void ClientGameCommandManager::Footstep(Event *ev)
{
    int iRunning, iEquipment;
    str sTagName;

    sTagName = ev->GetString(1);

    iRunning = 1;
    if (ev->NumArgs() > 1) {
        str sType = ev->GetString(2);
        if (!str::icmp(sType, "walk")) {
            // walking
            iRunning = 0;
        } else if (!str::icmp(sType, "ladder")) {
            // on ladder
            iRunning = -1;
        } else {
            // running
            iRunning = 1;
        }
    }

    if (ev->NumArgs() > 2) {
        iEquipment = ev->GetInteger(3);
    } else {
        iEquipment = 1;
    }

    if (current_centity && current_entity) {
        CG_Footstep(sTagName.c_str(), current_centity, current_entity, iRunning, iEquipment);
    }
}

void ClientGameCommandManager::LandingSound(Event *ev)
{
    int   iEquipment;
    float fVolume;

    if (ev->NumArgs() <= 0) {
        fVolume = 1.0;
    } else {
        fVolume = ev->GetFloat(1);
    }

    if (ev->NumArgs() <= 1) {
        iEquipment = 1;
    } else {
        iEquipment = ev->GetInteger(2);
    }

    if (current_centity && current_entity) {
        CG_LandingSound(current_centity, current_entity, fVolume, iEquipment);
    }
}

void ClientGameCommandManager::BodyFallSound(Event *ev)
{
    float fVolume;

    if (ev->NumArgs() <= 0) {
        fVolume = 1.0;
    } else {
        fVolume = ev->GetFloat(1);
    }

    if (current_centity && current_entity) {
        CG_BodyFallSound(current_centity, current_entity, fVolume);
    }
}

//===============
// Client
//===============
void ClientGameCommandManager::Client(Event *ev)
{
    Event      *event;
    const char *eventname;
    int         i;

    // see if it was a dummy command
    if (ev->NumArgs() < 1) {
        return;
    }

    eventname = ev->GetString(1);
    event     = new Event(eventname);

    for (i = 2; i <= ev->NumArgs(); i++) {
        event->AddToken(ev->GetToken(i));
    }
    ProcessEvent(event);
}

//===============
// TagDynamicLight
//===============
void ClientGameCommandManager::TagDynamicLight(Event *ev)
{
    str tagname;
    int tagnum;

    if (!m_pCurrentSfx) {
        // Spawn a single tempmodel that is a dynamic light
        m_spawnthing = &m_localemitter;
    }
    InitializeSpawnthing(m_spawnthing);

    tagname = ev->GetString(1);
    tagnum  = cgi.Tag_NumForName(current_tiki, tagname.c_str());
    if (tagnum == -1) {
        throw ScriptException("Tagname '%s' does not exist", tagname.c_str());
    }

    GetOrientation(tagnum, m_spawnthing);

    m_spawnthing->cgd.flags |= T_DLIGHT;
    m_spawnthing->cgd.color[0]       = ev->GetFloat(2);
    m_spawnthing->cgd.color[1]       = ev->GetFloat(3);
    m_spawnthing->cgd.color[2]       = ev->GetFloat(4);
    m_spawnthing->cgd.color[3]       = 1.0;
    m_spawnthing->cgd.lightIntensity = ev->GetFloat(5);
    m_spawnthing->cgd.life           = ev->GetFloat(6) * 1000;
    if (ev->NumArgs() > 6) {
        m_spawnthing->cgd.lightType |= DLightNameToNum(ev->GetString(7));
        if (ev->NumArgs() > 7) {
            m_spawnthing->cgd.lightType |= DLightNameToNum(ev->GetString(8));
        }
    } else {
        m_spawnthing->cgd.lightType = 0;
    }
    SpawnEffect(1, m_spawnthing);
}

//===============
// OriginDynamicLight
//===============
void ClientGameCommandManager::OriginDynamicLight(Event *ev)
{
    str tagname;

    if (!current_entity) {
        m_spawnthing = NULL;
        return;
    }

    if (!m_pCurrentSfx) {
        // Spawn a single tempmodel that is a dynamic light
        m_spawnthing = &m_localemitter;
    }
    InitializeSpawnthing(m_spawnthing);

    m_spawnthing->cgd.origin = current_entity->origin;
    m_spawnthing->cgd.flags |= T_DLIGHT;
    m_spawnthing->cgd.color[0]       = ev->GetFloat(1);
    m_spawnthing->cgd.color[1]       = ev->GetFloat(2);
    m_spawnthing->cgd.color[2]       = ev->GetFloat(3);
    m_spawnthing->cgd.color[3]       = 1.0;
    m_spawnthing->cgd.lightIntensity = ev->GetFloat(4);
    m_spawnthing->cgd.life           = ev->GetFloat(5) * 1000;
    if (ev->NumArgs() > 5) {
        m_spawnthing->cgd.lightType |= DLightNameToNum(ev->GetString(6));
        if (ev->NumArgs() > 6) {
            m_spawnthing->cgd.lightType |= DLightNameToNum(ev->GetString(7));
        }
    } else {
        m_spawnthing->cgd.lightType = 0;
    }
    SpawnEffect(1, m_spawnthing);
}

//===============
// DynamicLight
//===============
void ClientGameCommandManager::DynamicLight(Event *ev)
{
    if (!m_spawnthing) {
        return;
    }

    // The emitter itself has a dynamic light
    m_spawnthing->cgd.flags |= T_DLIGHT;
    m_spawnthing->dcolor[0]          = ev->GetFloat(1);
    m_spawnthing->dcolor[1]          = ev->GetFloat(2);
    m_spawnthing->dcolor[2]          = ev->GetFloat(3);
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

void ClientGameCommandManager::BlockDynamicLight(Event *ev)
{
    if (!current_entity) {
        m_spawnthing = NULL;
        return;
    }

    endblockfcn = &ClientGameCommandManager::EndBlockDynamicLight;

    if (!m_pCurrentSfx) {
        m_spawnthing = &m_localemitter;
    }

    InitializeSpawnthing(m_spawnthing);
    m_spawnthing->cgd.origin = current_entity->origin;
    m_spawnthing->cgd.flags |= T_DLIGHT;
    m_spawnthing->cgd.lightIntensity = ev->GetFloat(1);
    m_spawnthing->cgd.life           = ev->GetFloat(2);

    if (ev->NumArgs() > 2) {
        m_spawnthing->cgd.lightType |= DLightNameToNum(ev->GetString(3));
        if (ev->NumArgs() > 3) {
            m_spawnthing->cgd.lightType |= DLightNameToNum(ev->GetString(4));
        }
    } else {
        m_spawnthing->cgd.lightType = 0;
    }

    AxisCopy(current_entity->axis, m_spawnthing->axis);
    AxisCopy(current_entity->axis, m_spawnthing->tag_axis);
}

void ClientGameCommandManager::EndBlockDynamicLight()
{
    if (!m_spawnthing) {
        return;
    }

    SpawnEffect(m_spawnthing->count, 0);
}

static int emittercount = 0;

//===============
// UpdateBeam
//===============
void ClientGameCommandManager::UpdateBeam(dtiki_t *tiki, int entity_number, spawnthing_t *beamthing)
{
    Vector         start, end;
    int            renderfx;
    int            iLife;
    qboolean       addstartpoint;
    float          scale, alpha;
    emittertime_t *et;

    et = beamthing->GetEmitTime(entity_number);

    if (!et->active) {
        return;
    }

    if (current_entity) {
        renderfx = (current_entity->renderfx & ~(RF_FLAGS_NOT_INHERITED | RF_LIGHTING_ORIGIN));
        scale    = current_entity->scale;
        alpha    = (float)current_entity->shaderRGBA[3] / 255.0f;
    } else {
        renderfx = 0;
        scale    = 1.0f;
        alpha    = 1.0f;
    }

    iLife = m_spawnthing->cgd.life;
    if (m_spawnthing->life_random != 0.0) {
        iLife = (int)(random() * m_spawnthing->life_random + (float)iLife);
    }

    if (beamthing->startTag == "USE_TAGLIST") {
        int    i, count;
        str    startTag, endTag;
        int    tagnum;
        Vector dir;

        count = beamthing->m_taglist.NumObjects();

        if (!count) {
            return;
        }

        startTag = beamthing->m_taglist.ObjectAt(1);
        tagnum   = cgi.Tag_NumForName(current_tiki, startTag.c_str());
        if (tagnum == -1) {
            throw ScriptException("Tagname '%s' does not exist", startTag.c_str());
        }

        GetOrientation(tagnum, beamthing);
        start = beamthing->cgd.origin;
        if (beamthing->beamflags & BEAM_OFFSET_ENDPOINTS) {
            dir = Vector(crandom(), crandom(), crandom());
            start += (crandom() * beamthing->min_offset * dir) + (crandom() * beamthing->max_offset * dir);
        }

        CG_MultiBeamBegin();
        addstartpoint = qtrue;

        for (i = 2; i <= count; i++) {
            str s;

            s = va("%d", i);

            endTag = beamthing->m_taglist.ObjectAt(i);
            tagnum = cgi.Tag_NumForName(current_tiki, endTag.c_str());
            if (tagnum == -1) {
                throw ScriptException("Tagname '%s' does not exist", endTag.c_str());
            }

            GetOrientation(tagnum, beamthing);
            end = beamthing->cgd.origin;

            if (beamthing->beamflags & BEAM_OFFSET_ENDPOINTS) {
                dir = Vector(crandom(), crandom(), crandom());
                end += (crandom() * beamthing->min_offset * dir) + (crandom() * beamthing->max_offset * dir);
            }

            CG_MultiBeamAddPoints(
                start,
                end,
                beamthing->numSubdivisions,
                beamthing->beamflags,
                beamthing->min_offset,
                beamthing->max_offset,
                addstartpoint
            );

            addstartpoint = qfalse;
            startTag      = endTag;
            start         = end;
        }

        byte newcolor[4];

        newcolor[0] = (byte)(beamthing->cgd.color[0] * alpha * 255.0);
        newcolor[1] = (byte)(beamthing->cgd.color[1] * alpha * 255.0);
        newcolor[2] = (byte)(beamthing->cgd.color[2] * alpha * 255.0);
        newcolor[3] = (byte)(beamthing->cgd.color[3] * alpha * 255.0);

        CG_MultiBeamEnd(
            beamthing->cgd.scale * scale,
            renderfx,
            beamthing->cgd.shadername,
            newcolor,
            beamthing->beamflags,
            entity_number,
            beamthing->cgd.life
        );
    } else {
        if (beamthing->startTag.length()) {
            int tagnum = cgi.Tag_NumForName(current_tiki, beamthing->startTag.c_str());
            if (tagnum == -1) {
                throw ScriptException("Tagname '%s' does not exist", beamthing->startTag.c_str());
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
                throw ScriptException("Tagname '%s' does not exist", beamthing->endTag.c_str());
            }

            // Use a dummy sp to get the orientation
            GetOrientation(tagnum, beamthing);
            end = beamthing->cgd.origin;
        } else {
            end = start + Vector(beamthing->axis[0]) * beamthing->length;
        }

        CG_CreateBeam(
            start,
            vec_zero,
            entity_number,
            1,
            beamthing->cgd.alpha * alpha,
            beamthing->cgd.scale * scale,
            (beamthing->beamflags | BEAM_LIGHTNING_EFFECT),
            beamthing->length,
            iLife,
            qfalse,
            end,
            beamthing->min_offset,
            beamthing->max_offset,
            beamthing->overlap,
            beamthing->numSubdivisions,
            beamthing->delay,
            beamthing->cgd.shadername,
            beamthing->cgd.color,
            beamthing->numspherebeams,
            beamthing->sphereRadius,
            beamthing->toggledelay,
            beamthing->endalpha,
            renderfx,
            beamthing->emittername
        );
    }
}

//===============
// UpdateEmitter
//===============
void ClientGameCommandManager::UpdateEmitter(
    dtiki_t *tiki, vec3_t axis[3], int entity_number, int parent_number, Vector entity_origin
)
{
    int            parent, lastparent, i;
    emittertime_t *et = NULL;
    centity_t     *pc;
    int            count = 0;
    float          scale = 1.0f;
    int            entnum;

    if (current_entity)
    {
        scale = current_entity->scale;
        entnum = current_entity->entityNumber;
    }
    else
    {
        entnum = 1023;
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

        if (m_spawnthing->cgd.flags & T_WAVE) {
            if (m_spawnthing->entnum == entnum) {
                orientation_t ori;
                int j;

                // Set the default origin (spawn from the parent's origin)
                VectorCopy(entity_origin, m_spawnthing->cgd.origin);
            
                ori = cgi.TIKI_Orientation(current_entity, m_spawnthing->tagnum);

                VectorCopy(current_entity->origin, m_spawnthing->linked_origin);

                for (j = 0; j < 3; j++) {
                    VectorMA(m_spawnthing->linked_origin, ori.origin[i], current_entity->axis[j], m_spawnthing->linked_origin);
                }
                MatrixMultiply(ori.axis, current_entity->axis, m_spawnthing->linked_axis);
            }

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
        if (m_spawnthing->tagnum != -1) {
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
                pc         = &cg_entities[parent];
                lastparent = parent;
                parent     = pc->currentState.parent;
            }

            // The lastparent is the "real" parent
            if (lastparent != ENTITYNUM_NONE) {
                pc = &cg_entities[lastparent];

                m_spawnthing->cgd.origin -= pc->lerpOrigin;
                m_spawnthing->cgd.parent = lastparent;
            }
        }

        m_spawnthing->cgd.createTime   = cg.time;
        m_spawnthing->cgd.parentOrigin = Vector(entity_origin);

        if (m_spawnthing->cgd.flags & T_DLIGHT) {
            cgi.R_AddLightToScene(
                m_spawnthing->cgd.origin,
                m_spawnthing->cgd.lightIntensity * scale,
                m_spawnthing->dcolor[0],
                m_spawnthing->dcolor[1],
                m_spawnthing->dcolor[2],
                m_spawnthing->cgd.lightType
            );
            continue;
        }

        Vector save_origin = m_spawnthing->cgd.origin;
        Vector delta       = m_spawnthing->cgd.origin - et->oldorigin;

        if (m_spawnthing->spawnRate < 0) {
            warning("ClientGameCommandManager::UpdateEmitter", "Invalid spawnrate (negative)");
            continue;
        }

        if (paused->integer) {
            et->last_emit_time = 0;
        } else if ((et->last_emit_time > 0) && (m_spawnthing->spawnRate)) {
            int   dtime = cg.time - et->last_emit_time;
            float lerp, lerpfrac;

            if (et->last_emit_time > cg.time) {
                et->last_emit_time = cg.time;
            }

            count = dtime / (m_spawnthing->spawnRate * (1.0f / cg_effectdetail->value));

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
                SpawnEffect(1, 0);
                et->last_emit_time = cg.time;
            } else {
                lerpfrac = 1.0f / (float)count;

                lerp = 0;
                while (dtime > (m_spawnthing->spawnRate * (1.0f / cg_effectdetail->value))) {
                    et->last_emit_time = cg.time;

                    dtime -= m_spawnthing->spawnRate * (1.0f / cg_effectdetail->value);

                    if (et->lerp_emitter) {
                        m_spawnthing->cgd.origin = et->oldorigin + (delta * lerp);
                    }

                    SpawnEffect(1, dtime);

                    lerp += lerpfrac;
                }
            }
        } else {
            et->last_emit_time = cg.time;
        }

        if (cg_showemitters->integer) {
            if (count) {
                cgi.DPrintf("%d:%s:%d tempmodels emitted\n", entity_number, m_spawnthing->emittername.c_str(), count);
            }
        }

        // Since we have an oldorigin, now we can do lerping
        m_spawnthing->cgd.origin = save_origin;
        et->oldorigin            = save_origin;
        et->lerp_emitter         = qtrue;
    }
}

//===============
// RemoveClientEntity - Removes an entity from being
// updated if it has emitters attached
//===============
void ClientGameCommandManager::RemoveClientEntity(int number, dtiki_t *tiki, centity_t *cent, ctempmodel_t *p)
{
    int i;

    // Kill the emitters if they are attached to this tiki
    if (number != -1) {
        for (i = 1; i <= m_emitters.NumObjects(); i++) {
            spawnthing_t *st = m_emitters.ObjectAt(i);

            if (st->cgd.tiki == tiki) {
                st->RemoveEntity(number);
            }
        }
    }

    // Turn off any swiping for this entity number
    int        oldnum  = current_entity_number;
    centity_t *oldcent = current_centity;

    current_centity       = cent;
    current_entity_number = number;
    ProcessEvent(EV_Client_SwipeOff);
    current_entity_number = oldnum;
    current_centity       = oldcent;

    // If this tempmodel was assigned a temporary number then remove it from the
    // command_time_manager
    if (p && p->cgd.flags & T_ASSIGNED_NUMBER) {
        m_command_time_manager.RemoveEntity(number);
    }
}

bool ClientGameCommandManager::GetTagPositionAndOrientation(int tagnum, orientation_t *new_or)
{
    int           i;
    orientation_t tag_or;

    if (!current_tiki || !current_entity) {
        return 0;
    }

    tag_or = cgi.TIKI_Orientation(current_entity, tagnum & TAG_MASK);

    new_or->origin[0] = current_entity->origin[0];
    new_or->origin[1] = current_entity->origin[1];
    new_or->origin[2] = current_entity->origin[2];

    for (i = 0; i < 3; ++i) {
        VectorMA(new_or->origin, tag_or.origin[i], current_entity->axis[i], new_or->origin);
    }
    MatrixMultiply(tag_or.axis, current_entity->axis, new_or->axis);

    return true;
}

bool ClientGameCommandManager::GetTagPositionAndOrientation(str tagname, orientation_t *new_or)
{
    int tagnum;

    tagnum = cgi.Tag_NumForName(current_tiki, tagname.c_str());
    if (tagnum < 0) {
        Class::warning(
            "ClientGameCommandManager::GetTagPositionAndOrientation", "Could not find tag \"%s\"", tagname.c_str()
        );

        return false;
    }

    return ClientGameCommandManager::GetTagPositionAndOrientation(tagnum, new_or);
}

//===============
// FreeAllEmitters
//===============
void ClientGameCommandManager::RestartAllEmitters(void)
{
    // FIXME: stub??
}

//===============
// CG_RestartCommandManager
//===============
void CG_RestartCommandManager()
{
    commandManager.FreeAllTempModels();
}

//=================
// CG_CleanUpTempModels
//=================
void CG_CleanUpTempModels()
{
    commandManager.FreeSomeTempModels();
}

//=================
// CG_ProcessInitCommands
//=================
void CG_ProcessInitCommands(dtiki_t *tiki, refEntity_t *ent)
{
    int          i, j;
    int          num_args;
    refEntity_t *old_entity;
    dtiki_t     *old_tiki;
    dtikicmd_t  *pcmd;

    if (!tiki) {
        return;
    }

    old_entity     = current_entity;
    old_tiki       = current_tiki;
    current_entity = ent;
    current_tiki   = tiki;

    for (i = 0; i < tiki->a->num_client_initcmds; i++) {
        Event *ev;

        pcmd     = &tiki->a->client_initcmds[i];
        num_args = pcmd->num_args;

        // Create the event and Process it.
        ev = new Event(pcmd->args[0]);

        for (j = 1; j < num_args; j++) {
            ev->AddToken(pcmd->args[j]);
        }

        if (!commandManager.SelectProcessEvent(ev)) {
            Com_Printf(
                "^~^~^ CG_ProcessInitCommands: Bad init client command '%s' in '%s'\n", pcmd->args[0], tiki->name
            );
        }
    }

    current_entity = old_entity;
    current_tiki   = old_tiki;
}

//=================
// CG_ProcessCacheInitCommands
//=================
void CG_ProcessCacheInitCommands(dtiki_t *tiki)
{
    int          i, j;
    int          num_args;
    refEntity_t *old_entity;
    dtiki_t     *old_tiki;
    dtikicmd_t  *pcmd;

    if (!tiki) {
        return;
    }

    old_entity     = current_entity;
    old_tiki       = current_tiki;
    current_entity = NULL;
    current_tiki   = tiki;

    for (i = 0; i < tiki->a->num_client_initcmds; i++) {
        Event *ev;

        pcmd     = &tiki->a->client_initcmds[i];
        num_args = pcmd->num_args;

        // Create the event and Process it.
        ev = new Event(pcmd->args[0]);
        if (commandManager.GetFlags(ev) & EV_CACHE) {
            // only process even in which the cache flag is set

            for (j = 1; j < num_args; j++) {
                ev->AddToken(pcmd->args[j]);
            }

            if (!commandManager.SelectProcessEvent(ev)) {
                Com_Printf(
                    "^~^~^ CG_ProcessInitCommands: Bad init client command '%s' in '%s'\n", pcmd->args[0], tiki->name
                );
            }
        } else {
            // no cache flags
            delete ev;
        }
    }

    current_entity = old_entity;
    current_tiki   = old_tiki;
}

void CG_EndTiki(dtiki_t *tiki)
{
    CG_DeleteEmitters(tiki);
}

//=================
// CG_UpdateEntity
//=================
void CG_UpdateEntityEmitters(int entnum, refEntity_t *ent, centity_t *cent)
{
    refEntity_t *old_entity = current_entity;
    dtiki_t     *old_tiki   = current_tiki;
    int          oldnum     = current_entity_number;
    centity_t   *oldcent    = current_centity;

    // Check to see if this model has any emitters
    current_entity        = ent;
    current_tiki          = ent->tiki;
    current_centity       = cent;
    current_entity_number = entnum;

    if (cent->currentState.parent != ENTITYNUM_NONE) {
        commandManager.UpdateEmitter(
            ent->tiki, ent->axis, cent->currentState.number, cent->currentState.parent, ent->origin
        );
    } else {
        commandManager.UpdateEmitter(
            ent->tiki, ent->axis, cent->currentState.number, cent->currentState.parent, cent->lerpOrigin
        );
    }
    current_entity        = old_entity;
    current_tiki          = old_tiki;
    current_centity       = oldcent;
    current_entity_number = oldnum;
}

extern "C" {
    void CG_AnimationDebugMessage(int number, const char *fmt, ...);
}

//=================
// CG_ProcessEntityCommands
//=================
qboolean CG_ProcessEntityCommands(int frame, int anim, int entnum, refEntity_t *ent, centity_t *cent)
{
    int i, j;
    int num_args;

    tiki_cmd_t tikicmds;

    if (cgi.Frame_Commands(ent->tiki, anim, frame, &tikicmds)) {
        current_entity        = ent;
        current_centity       = cent;
        current_entity_number = entnum;
        current_tiki          = ent->tiki;
        CG_AnimationDebugMessage(entnum, "Processing Ent Commands: Entity: %3d Anim:#(%i) Frame:#(%i)\n", anim, frame);

        for (i = 0; i < tikicmds.num_cmds; i++) {
            Event *ev;

            num_args = tikicmds.cmds[i].num_args;

            if (num_args > 0) {
                // Create the event and Process it.
                ev = new Event(tikicmds.cmds[i].args[0]);

                for (j = 1; j < num_args; j++) {
                    ev->AddToken(tikicmds.cmds[i].args[j]);
                }

                commandManager.SelectProcessEvent(ev);
            }
        }

        current_tiki          = NULL;
        current_entity_number = -1;
        current_entity        = NULL;
        current_centity       = NULL;
    }

    return qtrue;
}

//=================
// CG_ClientCommandDebugMessage
//=================
void CG_ClientCommandDebugMessage(centity_t *cent, const char *fmt, ...)
{
#ifndef NDEBUG
    if (cg_debugAnim->integer) {
        va_list argptr;
        char    msg[1024];

        va_start(argptr, fmt);
        vsprintf(msg, fmt, argptr);
        va_end(argptr);

        if ((!cg_debugAnimWatch->integer) || ((cg_debugAnimWatch->integer - 1) == cent->currentState.number)) {
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
void CG_ClientCommands(refEntity_t *ent, centity_t *cent, int slot)
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

#    if 0
#        ifndef NDEBUG 
   CG_ClientCommandDebugMessage( 
      cent, 
      "Client Commands: cg.time %d checking Entity %d anim %d frame %d\n", 
      cg.time,
      cent->currentState.number,
      new_anim, 
      new_frame
      );
#        endif
#    endif

    anim = state->last_cmd_anim - 1;
    frame = state->last_cmd_frame;

    // if we had a last anim and it it wasn't same as ours then
    // we need to run any exit commands from the last animation
    if ((anim != new_anim) && (anim >= 0)) {
        // play the exit command
        CG_ProcessEntityCommands(TIKI_FRAME_EXIT, anim,
                                 cent->currentState.number, ent, cent);
#    ifndef NDEBUG
        CG_ClientCommandDebugMessage(
            cent, "Client Commands: Entity %d Exiting Anim: %s\n",
            cent->currentState.number, cgi.Anim_NameForNum(tiki, anim));
#    endif
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
#    ifndef NDEBUG
            CG_ClientCommandDebugMessage(
                cent, "Client Commands: Entity %d Entering Anim: %s\n",
                cent->currentState.number,
                cgi.Anim_NameForNum(tiki, new_anim));
#    endif
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
#    ifndef NDEBUG
                    CG_ClientCommandDebugMessage(
                        cent,
                        "Client Commands: cg.time %d Catching up Entity: %d "
                        "Anim: %s frame: %d numframes: %d\n",
                        cg.time, cent->currentState.number,
                        cgi.Anim_NameForNum(tiki, new_anim), frame,
                        state->numframes);
#    endif
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
#    ifndef NDEBUG
            CG_ClientCommandDebugMessage(
                cent,
                "Client Commands: cg.time %d Processing Entity: %d Anim: %s "
                "frame: %d numframes: %d\n",
                cg.time, cent->currentState.number,
                cgi.Anim_NameForNum(tiki, new_anim), new_frame,
                state->numframes);
#    endif
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

//===============
// CG_RemoveClientEntity
//===============
void CG_RemoveClientEntity(int number, dtiki_t *tiki, centity_t *cent)
{
    commandManager.RemoveClientEntity(number, tiki, cent);

    // Remove the beam list associated with this entity
    RemoveBeamList(number);
}

//=================
// CG_Command_ProcessFile
//=================
qboolean CG_Command_ProcessFile(const char *filename, qboolean quiet, dtiki_t *curTiki)
{
    char       *buffer;
    const char *bufstart;
    char        com_token[MAX_STRING_CHARS];
    char        tempName[MAX_QPATH + 1];

    if (cgi.FS_ReadFile(filename, (void **)&buffer, quiet) == -1) {
        return qfalse;
    }

    if (!quiet) {
        cgi.DPrintf("CG_Command_ProcessFile: %s\n", filename);
    }

    // we are not setting up for a tiki
    current_tiki = NULL;

    // Update the loading screen
    sprintf(tempName, "m%s", filename);
    cgi.LoadResource(tempName);

    bufstart = buffer;

    while (1) {
        Event *ev;

        // grab each line as we go
        strcpy(com_token, COM_ParseExt(&buffer, qtrue));
        if (!com_token[0]) {
            break;
        }

        if (!Q_stricmp(com_token, "end") || !Q_stricmp(com_token, "server")) {
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
        commandManager.SelectProcessEvent(ev);
    }
    cgi.FS_FreeFile((void *)bufstart);

    // Update the loading screen
    sprintf(tempName, "o%s", filename);
    cgi.LoadResource(tempName);

    return qtrue;
}

//===========================
// CG_ConsoleCommand Functions
//===========================
void CG_EventList_f(void)
{
    const char *mask;

    mask = NULL;
    if (cgi.Argc() > 1) {
        mask = cgi.Argv(1);
    }
    Event::ListCommands(mask);
}

void CG_EventHelp_f(void)
{
    const char *mask;

    mask = NULL;
    if (cgi.Argc() > 1) {
        mask = cgi.Argv(1);
    }
    Event::ListDocumentation(mask);
}

void CG_DumpEventHelp_f(void)
{
    const char *mask;

    mask = NULL;
    if (cgi.Argc() > 1) {
        mask = cgi.Argv(1);
    }
    Event::ListDocumentation(mask, true);
}

void CG_PendingEvents_f(void)
{
    const char *mask;

    mask = NULL;
    if (cgi.Argc() > 1) {
        mask = cgi.Argv(1);
    }
    Event::PendingEvents(mask);
}

void CG_ClassList_f(void)
{
    listAllClasses();
}

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

void CG_DumpAllClasses_f(void)
{
    DumpAllClasses();
}

#define throw_assert assert(0), throw

EmitterLoader emitterLoader;

Event EV_EmitterLoader_Emitter("emitter", EV_DEFAULT, "s", "emittername", "Create a new emitter");

CLASS_DECLARATION(Listener, EmitterLoader, NULL) {
    {&EV_EmitterLoader_Emitter, &EmitterLoader::Emitter},
    {NULL,                      NULL                   }
};

EmitterLoader::EmitterLoader()
{
    emitterActive = false;
}

void EmitterLoader::Emitter(Event *ev)
{
    spawnthing_t *st;

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
        Event *ev = NULL;

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
    emitterActive = false;
}

bool EmitterLoader::Load(Script& script)
{
    str token;
    str errortext;

    try {
        while (script.TokenAvailable(true)) {
            Event *ev = NULL;

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
    } catch (const char *s) {
        cgi.DPrintf("emitter: error on line %d: %s\n", script.GetLineNumber(), s);
        return false;
    }

    return true;
}

void CG_Emitter(centity_t *cent)
{
    spawnthing_t  *emitter;
    float          dtime;
    emittertime_t *et;
    vec3_t         mins, maxs;

    emitter = commandManager.GetEmitterByName(CG_ConfigString(CS_IMAGES + cent->currentState.tag_num));

    if (emitter) {
        vec3_t axis[3];
        cgi.R_ModelBounds(cgs.inlineDrawModel[cent->currentState.modelindex], mins, maxs);

        emitter->cgd.origin[0]       = cent->lerpOrigin[0] + mins[0] + (random() * (maxs[0] - mins[0]));
        emitter->cgd.origin[1]       = cent->lerpOrigin[1] + mins[1] + (random() * (maxs[1] - mins[1]));
        emitter->cgd.origin[2]       = cent->lerpOrigin[2] + maxs[2];
        emitter->cgd.parentOrigin    = Vector(cent->lerpOrigin);
        emitter->cgd.parentOrigin[2] = cent->lerpOrigin[2] + maxs[2];
        emitter->cgd.parentMins      = mins;
        emitter->cgd.parentMaxs      = maxs;

        AnglesToAxis(cent->lerpAngles, axis);

        // Update the emitter time and spawn the tempmodels
        et = emitter->GetEmitTime(cent->currentState.number);

        if (et->last_emit_time > 0) {
            dtime = cg.time - et->last_emit_time;
            while (dtime > emitter->spawnRate) {
                dtime -= emitter->spawnRate;
                commandManager.SpawnEffect(1, emitter);
                et->last_emit_time = cg.time;
            }
        }
    }
}

void ClientGameCommandManager::CGEvent(centity_t *cent)
{
    str      modelname;
    dtiki_t *tiki;

    tiki = cgi.R_Model_GetHandle(cgs.model_draw[cent->currentState.modelindex]);

    if (!tiki) {
        return;
    }

    CG_EntityEffects(cent);

    modelname    = cgi.TIKI_Name(tiki);
    m_spawnthing = InitializeSpawnthing(&m_localemitter);

    AnglesToAxis(cent->lerpAngles, m_spawnthing->axis);
    m_spawnthing->cgd.angles = cent->lerpAngles;
    m_spawnthing->cgd.origin = cent->lerpOrigin;
    m_spawnthing->cgd.scale  = cent->currentState.scale;
    m_spawnthing->cgd.alpha  = cent->currentState.alpha;

    m_spawnthing->cgd.color[0] = cent->color[0];
    m_spawnthing->cgd.color[1] = cent->color[1];
    m_spawnthing->cgd.color[2] = cent->color[2];
    m_spawnthing->cgd.color[3] = cent->color[3];

    Event *ev;
    ev = new Event("model");
    ev->AddString(modelname);
    ProcessEvent(ev);

    ev = new Event("anim");
    ev->AddString("idle");
    ProcessEvent(ev);

    SpawnEffect(1, 0);
}

qboolean ClientGameCommandManager::SelectProcessEvent(Event *ev)
{
    if (!m_fEventWait) {
        return ProcessEvent(ev);
    } else {
        return PostEventForEntity(ev, m_fEventWait);
    }
}

void ClientGameCommandManager::AddTreadMarkSources()
{
    // stub
}

void ClientGameCommandManager::InitializeTreadMarkCvars()
{
    // stub
}

void ClientGameCommandManager::InitializeTreadMarkSources()
{
    // stub
}

void ClientGameCommandManager::ResetTreadMarkSources()
{
    // stub
}

void ClientGameCommandManager::ResetTreadMarkSources(Event *ev)
{
    // stub
}

void CG_Event(centity_t *cent)
{
    commandManager.CGEvent(cent);
}

void ClientGameCommandManager::SetCurrentTiki(Event *ev)
{
    str tikiName;

    if (ev->NumArgs() != 1) {
        Com_Printf("ERROR: settiki command takes 1 parameter.\n");
        return;
    }

    tikiName = ev->GetString(1);
    if (str::icmp(tikiName.c_str(), "none")) {
        current_tiki = cgi.TIKI_FindTiki(tikiName.c_str());
    } else {
        current_tiki = NULL;
    }
}

void ClientGameCommandManager::EventViewKick(Event *ev)
{
    float vkmin[2], vkmax[2];
    float fPitchMax, fYawMax, fScatterPitchMax;
    str   sPattern;

    if (current_centity->currentState.parent != cg.snap->ps.clientNum) {
        return;
    }

    if (ev->NumArgs() < 9) {
        throw ScriptException("Wrong number of arguments for viewkick, should be 9\n");
    }

    vkmin[0] = ev->GetFloat(1);
    vkmax[0] = ev->GetFloat(2);
    vkmin[1] = ev->GetFloat(3);
    vkmax[1] = ev->GetFloat(4);

    cg.viewkickRecenter = ev->GetFloat(5);
    sPattern            = ev->GetString(6);

    fPitchMax        = ev->GetFloat(7);
    fYawMax          = ev->GetFloat(8);
    fScatterPitchMax = ev->GetFloat(9);

    if (ev->NumArgs() > 9) {
        cg.viewkickMinDecay = ev->GetFloat(10);
    } else {
        cg.viewkickMinDecay = 12.0;
    }

    if (ev->NumArgs() > 10) {
        cg.viewkickMaxDecay = ev->GetFloat(11);
    } else {
        cg.viewkickMaxDecay = 25.0;
    }

    cg.viewkick[0] += vkmin[0] + random() * (vkmax[0] - vkmin[0]);
    if (sPattern == "T") {
        cg.viewkick[1] += vkmin[1] + random() * (vkmax[1] - vkmin[1]);
    } else if (sPattern == "V") {
        cg.viewkick[1] += cg.viewkick[0] * (vkmin[1] + random() * (vkmax[1] - vkmin[1]));
    }

    if (cg.viewkick[0] < fScatterPitchMax) {
        if (cg.viewkick[0] <= -fScatterPitchMax) {
            cg.viewkick[0] += crandom() * 0.25;
            cg.viewkick[1] += crandom() * 0.25;
        }
    } else {
        cg.viewkick[0] -= crandom() * 0.25;
        cg.viewkick[1] += crandom() * 3.5;
    }

    if (cg.viewkick[0] > fPitchMax) {
        cg.viewkick[0] = fPitchMax;
    } else if (cg.viewkick[0] < -fPitchMax) {
        cg.viewkick[0] = -fPitchMax;
    }

    if (cg.viewkick[1] > fYawMax) {
        cg.viewkick[1] = fYawMax;
    } else if (cg.viewkick[1] < -fYawMax) {
        cg.viewkick[1] = -fYawMax;
    }
}

int ClientGameCommandManager::IdForTempModel(const ctempmodel_t* model)
{
    int index;

    if (model == &m_active_tempmodels) {
        return -1;
    }

    if (!model) {
        return -2;
    }

    index = model - m_tempmodels;
    if (index >= MAX_TEMPMODELS) {
        return -2;
    }

    if (model != &m_tempmodels[index]) {
        return -2;
    }

    return model - m_tempmodels;
}

ctempmodel_t* ClientGameCommandManager::TempModelForId(int id)
{
    if (id == -1) {
        return &m_active_tempmodels;
    }

    if (id == -2) {
        return NULL;
    }

    return &m_tempmodels[id];
}

int ClientGameCommandManager::IdForSpawnThing(const spawnthing_t* sp)
{
    int i;

    if (!sp) {
        return 0;
    }

    for (i = 1; i <= m_emitters.NumObjects(); i++) {
        if (sp == m_emitters.ObjectAt(i)) {
            return i;
        }
    }

    return 0;
}

spawnthing_t* ClientGameCommandManager::SpawnThingForId(int id)
{
    if (!id) {
        return 0;
    }

    return m_emitters.ObjectAt(id);
}

int ClientGameCommandManager::IdForVssSource(const cvssource_t* source)
{
    int index;

    if (source == &m_active_vsssources) {
        return -1;
    }

    if (!source) {
        return -2;
    }

    index = source - m_vsssources;
    if (index < 0 || index >= m_iAllocatedvsssources) {
        return -2;
    }

    if (source != &m_vsssources[index]) {
        return -2;
    }

    return source - m_vsssources;
}

cvssource_t* ClientGameCommandManager::VssSourceForId(int id)
{
    if (id == -1) {
        return &m_active_vsssources;
    }

    if (id == -2) {
        return 0;
    }

    return &m_vsssources[id];
}

void ClientGameCommandManager::ArchiveTempModelPointerToMemory(MemArchiver& archiver, ctempmodel_t** model)
{
    int id;

    if (archiver.IsReading()) {
        archiver.ArchiveInteger(&id);
        *model = TempModelForId(id);
    } else {
        id = IdForTempModel(*model);
        archiver.ArchiveInteger(&id);
    }
}

void ClientGameCommandManager::ArchiveSpawnThingPointerToMemory(MemArchiver& archiver, spawnthing_t** sp)
{
    int id;

    if (archiver.IsReading()) {
        archiver.ArchiveInteger(&id);
        *sp = SpawnThingForId(id);
    } else {
        id = IdForSpawnThing(*sp);
        archiver.ArchiveInteger(&id);
    }
}

void ClientGameCommandManager::ArchiveVssSourcePointerToMemory(MemArchiver& archiver, cvssource_t** source)
{
    int id;

    if (archiver.IsReading()) {
        archiver.ArchiveInteger(&id);
        *source = VssSourceForId(id);
    } else {
        id = IdForVssSource(*source);
        archiver.ArchiveInteger(&id);
    }
}

void ClientGameCommandManager::ArchiveToMemory(MemArchiver& archiver)
{
    spawnthing_t* sp;
    int num;
    int i;

    m_localemitter.ArchiveToMemory(archiver);

    if (archiver.IsReading()) {
        archiver.ArchiveInteger(&num);

        for (i = 0; i < num; i++) {
            sp = new spawnthing_t();
            sp->ArchiveToMemory(archiver);

            m_emitters.AddObjectAt(i + 1, sp);
        }
    } else {
        num = m_emitters.NumObjects();
        archiver.ArchiveInteger(&num);

        for (i = 0; i < num; i++) {
            sp = m_emitters[i];
            sp->ArchiveToMemory(archiver);
        }
    }

    ArchiveTempModelPointerToMemory(archiver, &m_active_tempmodels.prev);
    ArchiveTempModelPointerToMemory(archiver, &m_active_tempmodels.next);
    ArchiveTempModelPointerToMemory(archiver, &m_free_tempmodels);

    for (i = 0; i < MAX_TEMPMODELS; i++) {
        m_tempmodels[i].ArchiveToMemory(archiver);
    }

    if (archiver.IsReading()) {
        archiver.ArchiveInteger(&num);
        if (m_iAllocatedvsssources != num) {
            m_iAllocatedvsssources = num;

            m_vsssources = (cvssource_t*)cgi.Malloc(m_iAllocatedvsssources * sizeof(cvssource_t));
            // instead of using memset, call the constructor
            for (i = 0; i < m_iAllocatedvsssources; i++) {
                new (&m_vsssources[i]) cvssource_t();
            }
        }
    } else {
        archiver.ArchiveInteger(&m_iAllocatedvsssources);
    }

    ArchiveVssSourcePointerToMemory(archiver, &m_active_vsssources.prev);
    ArchiveVssSourcePointerToMemory(archiver, &m_active_vsssources.next);
    ArchiveVssSourcePointerToMemory(archiver, &m_free_vsssources);
    
    for (i = 0; i < m_iAllocatedvsssources; i++) {
        m_vsssources[i].ArchiveToMemory(archiver);
    }

    archiver.ArchiveTime(&m_iLastVSSRepulsionTime);
    m_command_time_manager.ArchiveToMemory(archiver);
    archiver.ArchiveInteger(&m_seed);
    archiver.ArchiveFloat(&m_fEventWait);

    m_pCurrentSfx = NULL;
}

void commandthing_t::ArchiveToMemory(MemArchiver& archiver)
{
    int num;
    int i;

    enttracker_t::ArchiveToMemory(archiver);

    if (archiver.IsReading()) {
        archiver.ArchiveInteger(&num);

        for (i = 0; i < num; i++) {
            commandtime_t ct;

            ct.ArchiveToMemory(archiver);
            m_commandtimes.AddObject(ct);
        }
    } else {
        num = m_commandtimes.NumObjects();

        archiver.ArchiveInteger(&num);
        m_commandtimes.ClearObjectList();

        for (i = 0; i < num; i++) {
            commandtime_t& ct = m_commandtimes.ObjectAt(i);

            ct.ArchiveToMemory(archiver);
        }
    }
}

void cvssource_t::ArchiveToMemory(MemArchiver& archiver)
{
    commandManager.ArchiveVssSourcePointerToMemory(archiver, &next);
    commandManager.ArchiveVssSourcePointerToMemory(archiver, &prev);
    commandManager.ArchiveVssSourcePointerToMemory(archiver, &stnext);
    archiver.ArchiveInteger(&stindex);
    archiver.ArchiveVector(&lastOrigin);
    archiver.ArchiveFloat(&lastRadius);
    archiver.ArchiveFloat(&lastDensity);
    archiver.ArchiveVec3(lastColor);
    archiver.ArchiveVec3(lastLighting);
    archiver.ArchiveVector(&newOrigin);
    archiver.ArchiveFloat(&newRadius);
    archiver.ArchiveFloat(&newDensity);
    archiver.ArchiveVec3(newColor);
    archiver.ArchiveVec3(newLighting);
    archiver.ArchiveFloat(&ooRadius);
    archiver.ArchiveVector(&velocity);
    archiver.ArchiveFloat(&startAlpha);
    archiver.ArchiveInteger(&roll);
    archiver.ArchiveVector(&repulsion);
    archiver.ArchiveInteger(&lifeTime);
    archiver.ArchiveInteger(&collisionmask);
    archiver.ArchiveInteger(&parent);
    archiver.ArchiveInteger(&flags);
    archiver.ArchiveInteger(&flags2);
    archiver.ArchiveInteger(&smokeType);
    archiver.ArchiveFloat(&typeInfo);
    archiver.ArchiveFloat(&fadeMult);
    archiver.ArchiveFloat(&scaleMult);
    archiver.ArchiveTime(&lastPhysicsTime);
    archiver.ArchiveTime(&lastLightingTime);
    archiver.ArchiveBoolean(&lastValid);
}

void ctempmodel_t::ArchiveToMemory(MemArchiver& archiver)
{
    commandManager.ArchiveTempModelPointerToMemory(archiver, &next);
    commandManager.ArchiveTempModelPointerToMemory(archiver, &prev);

    cgd.ArchiveToMemory(archiver);
    archiver.ArchiveString(&modelname);

    CG_ArchiveRefEntity(archiver, &lastEnt);
    CG_ArchiveRefEntity(archiver, &ent);

    archiver.ArchiveInteger(&number);
    archiver.ArchiveTime(&lastAnimTime);
    archiver.ArchiveTime(&lastPhysicsTime);
    archiver.ArchiveTime(&killTime);
    archiver.ArchiveTime(&next_bouncesound_time);
    archiver.ArchiveInteger(&seed);
    archiver.ArchiveInteger(&twinkleTime);
    archiver.ArchiveInteger(&aliveTime);
    archiver.ArchiveBoolean(&addedOnce);
    archiver.ArchiveBoolean(&lastEntValid);
    commandManager.ArchiveSpawnThingPointerToMemory(archiver, &m_spawnthing);
}

void enttracker_t::ArchiveToMemory(MemArchiver& archiver)
{
    archiver.ArchiveRaw(usedNumbers, sizeof(usedNumbers));
}

void emittertime_t::ArchiveToMemory(MemArchiver& archiver)
{
    archiver.ArchiveInteger(&entity_number);
    archiver.ArchiveTime(&last_emit_time);
    archiver.ArchiveVector(&oldorigin);
    archiver.ArchiveBoolean(&active);
    archiver.ArchiveBoolean(&lerp_emitter);
}

void emitterthing_t::ArchiveToMemory(MemArchiver& archiver)
{
    int num;
    int i;

    enttracker_t::ArchiveToMemory(archiver);

    if (archiver.IsReading()) {
        emittertime_t et;

        archiver.ArchiveInteger(&num);
        for (i = 0; i < num; i++) {
            et.ArchiveToMemory(archiver);
            m_emittertimes.AddObject(et);
        }
    } else {
        num = m_emittertimes.NumObjects();
        archiver.ArchiveInteger(&num);

        for (i = 0; i < num; i++) {
            emittertime_t& et = m_emittertimes[i];
            et.ArchiveToMemory(archiver);
        }
    }

    archiver.ArchiveBoolean(&startoff);
}

void cg_common_data::ArchiveToMemory(MemArchiver& archiver)
{
    archiver.ArchiveInteger(&life);
    archiver.ArchiveTime(&createTime);
    archiver.ArchiveVector(&origin);
    archiver.ArchiveVector(&oldorigin);
    archiver.ArchiveVector(&accel);
    archiver.ArchiveVector(&angles);
    archiver.ArchiveVector(&velocity);
    archiver.ArchiveVector(&avelocity);
    archiver.ArchiveVector(&parentOrigin);
    archiver.ArchiveVector(&parentMins);
    archiver.ArchiveVector(&parentMaxs);
    archiver.ArchiveVector(&minVel);
    archiver.ArchiveVector(&maxVel);
    archiver.ArchiveFloat(color);
    archiver.ArchiveFloat(&color[1]);
    archiver.ArchiveFloat(&color[2]);
    archiver.ArchiveFloat(&color[3]);
    archiver.ArchiveFloat(&alpha);
    archiver.ArchiveFloat(&scaleRate);
    archiver.ArchiveFloat(&scalemin);
    archiver.ArchiveFloat(&scalemax);
    archiver.ArchiveFloat(&bouncefactor);
    archiver.ArchiveInteger(&bouncecount);
    archiver.ArchiveInteger(&maxbouncecount);
    archiver.ArchiveString(&bouncesound);
    archiver.ArchiveInteger(&bouncesound_delay);
    archiver.ArchiveInteger(&flags);
    archiver.ArchiveInteger(&flags2);

    CG_ArchiveTikiPointer(archiver, &tiki);

    archiver.ArchiveInteger(&swarmfreq);
    archiver.ArchiveFloat(&swarmmaxspeed);
    archiver.ArchiveFloat(&swarmdelta);
    archiver.ArchiveFloat(&lightIntensity);
    archiver.ArchiveInteger(&lightType);
    archiver.ArchiveInteger(&fadeintime);
    archiver.ArchiveInteger(&fadedelay);
    archiver.ArchiveInteger(&parent);
    archiver.ArchiveInteger(&collisionmask);
    archiver.ArchiveInteger(&min_twinkletimeoff);
    archiver.ArchiveInteger(&max_twinkletimeoff);
    archiver.ArchiveInteger(&min_twinkletimeon);
    archiver.ArchiveInteger(&max_twinkletimeon);
    archiver.ArchiveInteger(&lightstyle);
    archiver.ArchiveInteger(&physicsRate);
    archiver.ArchiveFloat(&scale);
    archiver.ArchiveFloat(&scale2);
    archiver.ArchiveString(&swipe_shader);
    archiver.ArchiveString(&swipe_tag_start);
    archiver.ArchiveString(&swipe_tag_end);
    archiver.ArchiveString(&shadername);
    archiver.ArchiveFloat(&swipe_life);
    archiver.ArchiveFloat(&friction);
    archiver.ArchiveFloat(&spin_rotation);
    archiver.ArchiveFloat(&decal_orientation);
    archiver.ArchiveFloat(&decal_radius);
}

void spawnthing_t::ArchiveToMemory(MemArchiver& archiver)
{
    emitterthing_t::ArchiveToMemory(archiver);

    CG_ArchiveStringContainer(archiver, &m_modellist);
    CG_ArchiveStringContainer(archiver, &m_taglist);

    cgd.ArchiveToMemory(archiver);

    archiver.ArchiveInteger(&entnum);
    archiver.ArchiveVector(&origin_offset_base);
    archiver.ArchiveVector(&origin_offset_amplitude);
    archiver.ArchiveVector(&axis_offset_base);
    archiver.ArchiveVector(&axis_offset_amplitude);
    archiver.ArchiveVector(&randvel_base);
    archiver.ArchiveVector(&randvel_amplitude);
    archiver.ArchiveVector(&avelocity_base);
    archiver.ArchiveVector(&avelocity_amplitude);
    archiver.ArchiveVector(&angles_amplitude);
    archiver.ArchiveVec3(axis[0]);
    archiver.ArchiveVec3(axis[1]);
    archiver.ArchiveVec3(axis[2]);
    archiver.ArchiveVec3(tag_axis[0]);
    archiver.ArchiveVec3(tag_axis[1]);
    archiver.ArchiveVec3(tag_axis[2]);
    archiver.ArchiveFloat(&life_random);
    archiver.ArchiveFloat(&forwardVelocity);
    archiver.ArchiveFloat(&sphereRadius);
    archiver.ArchiveFloat(&coneHeight);
    archiver.ArchiveFloat(&spawnRate);
    archiver.ArchiveTime(&lastTime);
    archiver.ArchiveInteger(&count);
    archiver.ArchiveInteger(&tagnum);
    archiver.ArchiveString(&emittername);
    archiver.ArchiveString(&animName);
    archiver.ArchiveFloat(dcolor);
    archiver.ArchiveFloat(&dcolor[1]);
    archiver.ArchiveFloat(&dcolor[2]);
    archiver.ArchiveBoolean(&dlight);
    archiver.ArchiveInteger(&numtempmodels);
    archiver.ArchiveVec3(linked_origin);
    archiver.ArchiveVec3(linked_axis[0]);
    archiver.ArchiveVec3(linked_axis[1]);
    archiver.ArchiveVec3(linked_axis[2]);
    archiver.ArchiveFloat(&fMinRangeSquared);
    archiver.ArchiveFloat(&fMaxRangeSquared);
    archiver.ArchiveString(&startTag);
    archiver.ArchiveString(&endTag);
    archiver.ArchiveFloat(&length);
    archiver.ArchiveFloat(&min_offset);
    archiver.ArchiveFloat(&max_offset);
    archiver.ArchiveFloat(&overlap);
    archiver.ArchiveFloat(&numSubdivisions);
    archiver.ArchiveFloat(&delay);
    archiver.ArchiveFloat(&toggledelay);
    archiver.ArchiveInteger(&beamflags);
    archiver.ArchiveInteger(&numspherebeams);
    archiver.ArchiveFloat(&endalpha);
    archiver.ArchiveFloat(&spreadx);
    archiver.ArchiveFloat(&spready);
    archiver.ArchiveBoolean(&use_last_trace_end);
}

void commandtime_t::ArchiveToMemory(MemArchiver& archiver)
{
    archiver.ArchiveInteger(&entity_number);
    archiver.ArchiveInteger(&command_number);
    archiver.ArchiveTime(&last_command_time);
}
