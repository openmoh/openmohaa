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

#include "scriptthread.h"
#include "scriptmaster.h"
#include "scriptclass.h"
#include "scriptvariable.h"
#include "scriptexception.h"

#include "g_spawn.h"
#include "level.h"
#include "game.h"
#include "camera.h"
#include "dm_manager.h"
#include "hud.h"
#include "huddraw.h"
#include "object.h"
#include "weaputils.h"
#include "consoleevent.h"
#include "player.h"
#include "lightstyleclass.h"
#include "debuglines.h"

#include "scriptflags.h"

#include "slre.h"

#include "md5.h"

#ifdef WIN32
#    include <direct.h>
#else
#    include <sys/stat.h>
#    include <unistd.h>
#endif

#define scriptfiles sv_scriptfiles

Event EV_ScriptThread_GetCvar
(
    "getcvar",
    EV_DEFAULT,
    "s",
    "name",
    "getcvar",
    EV_RETURN
);
Event EV_ScriptThread_GetRandomFloat
(
    "randomfloat",
    EV_DEFAULT,
    "f",
    "max",
    "randomfloat",
    EV_RETURN
);
Event EV_ScriptThread_GetRandomInt
(
    "randomint",
    EV_DEFAULT,
    "i",
    "max",
    "randomint",
    EV_RETURN
);
Event EV_ScriptThread_GetAbs
(
    "abs",
    EV_DEFAULT,
    "f",
    "arg",
    "Absolute value of int or float",
    EV_RETURN
);
//////////
// Added in 2.30
Event EV_ScriptThread_GetCos
(
    "cos",
    EV_DEFAULT,
    "f",
    "arg",
    "Cosine of float (expects degrees not radians)",
    EV_RETURN
);
Event EV_ScriptThread_GetSin
(
    "sin",
    EV_DEFAULT,
    "f",
    "arg",
    "Sine of float (expects degrees not radians)",
    EV_RETURN
);
Event EV_ScriptThread_GetTan
(
    "tan",
    EV_DEFAULT,
    "f",
    "arg",
    "Tangent of float (expects degrees not radians)",
    EV_RETURN
);
Event EV_ScriptThread_GetATan
(
    "atan",
    EV_DEFAULT,
    "fF",
    "arg arg2",
    "ArcTangent of float (in degrees not radians)",
    EV_RETURN
);
Event EV_ScriptThread_GetSqrt
(
    "sqrt",
    EV_DEFAULT,
    "f",
    "arg",
    "Sqrt of float",
    EV_RETURN
);
//////////
Event EV_ScriptThread_GetSelf
(
    "self",
    EV_DEFAULT,
    NULL,
    NULL,
    "self",
    EV_GETTER
);
Event EV_ScriptThread_Goto
(
    "goto",
    EV_DEFAULT,
    "s",
    "label",
    "Goes to the specified label.",
    EV_NORMAL
);
Event EV_ScriptThread_Wait
(
    "wait",
    EV_DEFAULT,
    "f",
    "wait_time",
    "Wait for the specified amount of time.",
    EV_NORMAL
);
Event EV_ScriptThread_WaitFrame
(
    "waitframe",
    EV_DEFAULT,
    NULL,
    NULL,
    "Wait for one server frame.",
    EV_NORMAL
);
Event EV_ScriptThread_Pause
(
    "pause",
    EV_DEFAULT,
    NULL,
    NULL,
    "Pauses the thread.",
    EV_NORMAL
);
Event EV_ScriptThread_End
(
    "end",
    EV_DEFAULT,
    NULL,
    NULL,
    "Ends the script",
    EV_NORMAL
);
Event EV_ScriptThread_Print
(
    "print",
    EV_DEFAULT,
    "s",
    "string",
    "Prints a string.",
    EV_NORMAL
);
Event EV_ScriptThread_Println
(
    "println",
    EV_DEFAULT,
    "s",
    "string",
    "Prints a string. followed by a newline.",
    EV_NORMAL
);
Event EV_ScriptThread_DPrintln // Added in 2.0
(
    "dprintln",
    EV_DEFAULT,
    "s",
    "string",
    "Prints a debug string.followed by a newline.",
    EV_NORMAL
);
Event EV_ScriptThread_IPrintln
(
    "iprintln",
    EV_DEFAULT,
    "s",
    "string",
    "Prints a string. followed by a newline.",
    EV_NORMAL
);
Event EV_ScriptThread_IPrintlnBold
(
    "iprintlnbold",
    EV_DEFAULT,
    "s",
    "string",
    "Prints a string.followed by a newline in a bold/important way.",
    EV_NORMAL
);
Event EV_ScriptThread_IPrintln_NoLoc
(
    "iprintln_noloc",
    EV_DEFAULT,
    "s",
    "string",
    "Prints a string.followed by a newline with no localization conversion.",
    EV_NORMAL
);
Event EV_ScriptThread_IPrintlnBold_NoLoc
(
    "iprintlnbold_noloc",
    EV_DEFAULT,
    "s",
    "string",
    "Prints a string.followed by a newline in a bold/important way with no localization conversion.",
    EV_NORMAL
);
Event EV_ScriptThread_MPrint
(
    "mprint",
    EV_DEFAULT,
    "s",
    "string",
    "Prints a string.",
    EV_NORMAL
);
Event EV_ScriptThread_MPrintln
(
    "mprintln",
    EV_DEFAULT,
    "s",
    "string",
    "Prints a string. followed by a newline",
    EV_NORMAL
);
Event EV_ScriptThread_Assert
(
    "assert",
    EV_DEFAULT,
    "f",
    "value",
    "Assert if value is 0.",
    EV_NORMAL
);
Event EV_ScriptThread_CastInt
(
    "int",
    EV_DEFAULT,
    "i",
    "value",
    "Casts value to an int.",
    EV_RETURN
);
Event EV_ScriptThread_CastFloat
(
    "float",
    EV_DEFAULT,
    "i",
    "value",
    "Casts value to a float.",
    EV_RETURN
);
Event EV_ScriptThread_CastString
(
    "string",
    EV_DEFAULT,
    "i",
    "value",
    "Casts value to a string.",
    EV_RETURN
);
Event EV_ScriptThread_CastBoolean
(
    "bool",
    EV_DEFAULT,
    "i",
    "value",
    "Casts value to a bool.",
    EV_RETURN
);
Event EV_ScriptThread_CastEntity
(
    "entity",
    EV_DEFAULT,
    "i",
    "value",
    "Casts value to an entity.",
    EV_RETURN
);
Event EV_ScriptThread_Clear
(
    "clear",
    EV_DEFAULT,
    "s",
    "var_group",
    "Clears the specified var group."
);
Event EV_ScriptThread_Trigger
(
    "trigger",
    EV_DEFAULT,
    "s",
    "name",
    "Trigger the specified target or entity."
);
Event EV_ScriptThread_Spawn
(
    "spawn",
    EV_DEFAULT,
    "sSSSSSSSS",
    "entityname keyname1 value1 keyname2 value2 keyname3 value3 keyname4 value4",
    "Spawns an entity.",
    EV_NORMAL
);
Event EV_ScriptThread_SpawnReturn
(
    "spawn",
    EV_DEFAULT,
    "sSSSSSSSS",
    "entityname keyname1 value1 keyname2 value2 keyname3 value3 keyname4 value4",
    "Spawns an entity.",
    EV_RETURN
);
Event EV_ScriptThread_Map
(
    "map",
    EV_DEFAULT,
    "s",
    "map_name",
    "Starts the specified map."
);
Event EV_ScriptThread_SetCvar
(
    "setcvar",
    EV_DEFAULT,
    "ss",
    "cvar_name value",
    "Sets the value of the specified cvar."
);
Event EV_ScriptThread_DebugLine // Added in 2.0
(
    "debugline",
     EV_DEFAULT,
     "vvFFFF",
     "start end red green blue alpha",
     "Draws a debug line from start to end using the given color and alpha for g_numdebuglinedelays server frames"
);
Event EV_ScriptThread_CueCamera
(
    "cuecamera",
    EV_DEFAULT,
    "eF",
    "entity switchTime",
    "Cue the camera. If switchTime is specified, then the camera\n"
    "will switch over that length of time."
);
Event EV_ScriptThread_CuePlayer
(
    "cueplayer",
    EV_DEFAULT,
    "F",
    "switchTime",
    "Go back to the normal camera. If switchTime is specified,\n"
    "then the camera will switch over that length of time."
);
Event EV_ScriptThread_FreezePlayer
(
    "freezeplayer",
    EV_DEFAULT,
    NULL,
    NULL,
    "Freeze the player."
);
Event EV_ScriptThread_ReleasePlayer
(
    "releaseplayer",
    EV_DEFAULT,
    NULL,
    NULL,
    "Release the player."
);
Event EV_ScriptThread_KillEnt
(
    "killent",
    EV_CHEAT,
    "i",
    "ent_num",
    "Kill the specified entity."
);
Event EV_ScriptThread_IsAlive
(
    "IsAlive", EV_DEFAULT,
    "e",
    "ent",
    "Returns true if the specified entity exists and has health > 0.",
    EV_RETURN
);
Event EV_ScriptThread_KillClass
(
    "killclass",
    EV_CHEAT,
    "sI",
    "class_name except",
    "Kills everything in the specified class except for the specified entity (optional)."
);
Event EV_ScriptThread_GetEntByEntnum // Added in 2.30
(
    "getentbyentnum",
     EV_CHEAT,
     "i",
     "ent_num",
     "Gets the specified entity",
     EV_RETURN
);
Event EV_ScriptThread_RemoveEnt
(
    "removeent",
    EV_CHEAT,
    "i",
    "ent_num",
    "Removes the specified entity."
);
Event EV_ScriptThread_RemoveClass
(
    "removeclass",
    EV_CHEAT,
    "sI",
    "class_name except",
    "Removes everything in the specified class except for the specified entity (optional)."
);

// client/server flow control

Event EV_ScriptThread_ServerOnly
(
    "server",
    EV_DEFAULT,
    "SSSSSS",
    "arg1 arg2 arg3 arg4 arg5 arg6",
    "Server only command."
);
Event EV_ScriptThread_StuffCommand
(
    "stuffcmd",
    EV_DEFAULT,
    "SSSSSS",
    "arg1 arg2 arg3 arg4 arg5 arg6",
    "Server only command."
);
Event EV_ScriptThread_SetCinematic
(
    "cinematic",
    EV_DEFAULT,
    NULL,
    NULL,
    "Turns on cinematic."
);
Event EV_ScriptThread_SetNonCinematic
(
    "noncinematic",
    EV_DEFAULT,
    NULL,
    NULL,
    "Turns off cinematic."
);
Event EV_ScriptThread_AllAIOff
(
    "all_ai_off",
    EV_DEFAULT,
    NULL,
    NULL,
    "Turns all AI off."
);
Event EV_ScriptThread_AllAIOn
(
    "all_ai_on",
    EV_DEFAULT,
    NULL,
    NULL,
    "Turns all AI on."
);
// Precache specific
Event EV_ScriptThread_AliasCache
(
    "aliascache",
    EV_DEFAULT,
    "ssSSSSSS",
    "alias realPath arg1 arg2 arg3 arg4 arg5 arg6",
    "Create an alias to the specified path and cache the resource"
);
Event EV_ScriptThread_Precache_Cache
(
    "cache",
    EV_DEFAULT,
    "s",
    "resource_name",
    "Cache the specified resource."
);
// fades for movies
Event EV_ScriptThread_FadeIn
(
    "fadein",
    EV_DEFAULT,
    "fffffI",
    "time red green blue alpha mode",
    "Sets up fadein in values."
);
Event EV_ScriptThread_FadeOut
(
    "fadeout",
    EV_DEFAULT,
    "fffffI",
    "time red green blue alpha mode",
    "Sets up fadeout values."
);
Event EV_ScriptThread_FadeSound
(
    "fadesound",
    -1,
    "fFE",
    "time min_vol player",
    "Fades the sound out over the given time, optionally to one single player.\n"
    "min_vol being a 0-1 fraction is the minimum volume.",
    EV_NORMAL
);
Event EV_ScriptThread_CameraCommand
(
    "cam",
    EV_DEFAULT,
    "sSSSSSS",
    "command arg1 arg2 arg3 arg4 arg5 arg6",
    "Processes a camera command."
);

// music command
Event EV_ScriptThread_MusicEvent
(
    "music",
    EV_DEFAULT,
    "sS",
    "current fallback",
    "Sets the current and fallback (optional) music moods."
);
Event EV_ScriptThread_ForceMusicEvent
(
    "forcemusic",
    EV_DEFAULT,
    "sS",
    "current fallback",
    "Forces the current and fallback (optional) music moods."
);
Event EV_ScriptThread_MusicVolumeEvent
(
    "musicvolume",
    EV_DEFAULT,
    "ff",
    "volume fade_time",
    "Sets the volume and fade time of the music."
);
Event EV_ScriptThread_RestoreMusicVolumeEvent
(
    "restoremusicvolume",
    EV_DEFAULT,
    "f",
    "fade_time",
    "Restores the music volume to its previous value."
);
Event EV_ScriptThread_SoundtrackEvent
(
    "soundtrack",
    EV_DEFAULT,
    "s",
    "soundtrack_name",
    "Changes the soundtrack."
);
Event EV_ScriptThread_RestoreSoundtrackEvent
(
    "restoresoundtrack",
    EV_DEFAULT,
    NULL,
    NULL,
    "Restores the soundtrack to the previous one."
);
Event EV_ScriptThread_ClearFade
(
    "clearfade",
    EV_DEFAULT,
    NULL,
    NULL,
    "Clear the fade from the screen"
);
Event EV_ScriptThread_Letterbox
(
    "letterbox",
    EV_DEFAULT,
    "f",
    "time",
    "Puts the game in letterbox mode."
);
Event EV_ScriptThread_ClearLetterbox
(
    "clearletterbox",
    EV_DEFAULT,
    "f",
    "time",
    "Clears letterbox mode."
);
Event EV_ScriptThread_SetDialogScript
(
    "setdialogscript",
    EV_DEFAULT,
    "s",
    "dialog_script",
    "Set the script to be used when dialog:: is used"
);
Event EV_ScriptThread_SetLightStyle
(
    "setlightstyle",
    EV_DEFAULT,
    "is",
    "lightstyleindex lightstyledata",
    "Set up the lightstyle with lightstyleindex to the specified data"
);
Event EV_ScriptThread_CenterPrint
(
    "centerprint",
    EV_DEFAULT,
    "s",
    "stuffToPrint",
    "prints the included message in the middle of all player's screens"
);
Event EV_ScriptThread_LocationPrint
(
    "locprint",
    EV_DEFAULT,
    "iis",
    "xoffset yoffset stuffToPrint",
    "prints the included message in the specified location of all player's screens"
);
Event EV_ScriptThread_MissionFailed
(
    "missionfailed",
    EV_DEFAULT,
    NULL,
    NULL,
    "Makes the player fail their mission,"
    "level restarts."
);
Event EV_ScriptThread_Vector_Length
(
    "vector_length",
    EV_DEFAULT,
    "v",
    "vector",
    "Returns the length of the specified vector.",
    EV_RETURN
);
Event EV_ScriptThread_Vector_Normalize
(
    "vector_normalize",
    EV_DEFAULT,
    "v",
    "vector",
    "Returns the normalized vector of the specified vector.",
    EV_RETURN
);
Event EV_ScriptThread_Vector_Add
(
    "vector_add", EV_DEFAULT,
    "vv",
    "vector1 vector2",
    "Returns vector1 + vector2.",
    EV_RETURN
);
Event EV_ScriptThread_Vector_Subtract
(
    "vector_subtract",
    EV_DEFAULT,
    "vv",
    "vector1 vector2",
    "Returns vector1 - vector2.",
    EV_RETURN
);
Event EV_ScriptThread_Vector_Scale
(
    "vector_scale", EV_DEFAULT,
    "vf",
    "vector1 scale_factor",
    "Returns vector1 * scale_factor.",
    EV_RETURN
);
Event EV_ScriptThread_Vector_DotProduct
(
    "vector_dot", EV_DEFAULT,
    "vv",
    "vector1 vector2",
    "Returns vector1 * vector2.",
    EV_RETURN
);
Event EV_ScriptThread_Vector_CrossProduct
(
    "vector_cross",
    EV_DEFAULT,
    "vv",
    "vector1 vector2",
    "Returns vector1 x vector2.",
    EV_RETURN
);
Event EV_ScriptThread_Vector_ToAngles
(
    "vector_toangles",
    EV_DEFAULT,
    "v",
    "vector1",
    "Returns vector1 converted to angles.",
    EV_RETURN
);
Event EV_ScriptThread_Angles_ToForward
(
    "angles_toforward",
    EV_DEFAULT,
    "v",
    "angles",
    "Returns the forward vector of the specified angles",
    EV_RETURN
);
Event EV_ScriptThread_Angles_ToLeft
(
    "angles_toleft",
    EV_DEFAULT,
    "v",
    "angles",
    "Returns the left vector of the specified angles",
    EV_RETURN
);
Event EV_ScriptThread_Angles_ToUp
(
    "angles_toup",
    EV_DEFAULT,
    "v",
    "angles",
    "Returns the up vector of the specified angles",
    EV_RETURN
);
Event EV_ScriptThread_Angles_PointAt
(
    "angles_pointat",
    EV_DEFAULT,
    "eee",
    "parent_entity entity target_entity",
    "Returns the angles that points at the target_entity given the base orientation of the parent_entity and the "
    "position of the entity.",
    EV_RETURN
);
Event EV_ScriptThread_AIsCloserThanBToC
(
    "vector_closer",
    EV_DEFAULT,
    "vvv",
    "vec_a vec_b vec_c",
    "returns 1 if the first vector is closer than the second vector to the third vector.",
    EV_RETURN
);
Event EV_ScriptThread_PointsWithinDist
(
    "vector_within",
    EV_DEFAULT,
    "vvf",
    "position1 position2 distance",
    "returns 1 if the two points are <= distance apart, or 0 if they are greater than distance apart.",
    EV_RETURN
);
Event EV_ScriptThread_Popmenu
(
    "popmenu",
    EV_DEFAULT,
    "i",
    "index",
    "pop menu"
);
Event EV_ScriptThread_Showmenu
(
    "showmenu",
    EV_DEFAULT,
    "sI",
    "name bForce",
    "show menu,"
    "with option to force it on"
);
Event EV_ScriptThread_Hidemenu
(
    "hidemenu",
    EV_DEFAULT,
    "sI",
    "name bForce",
    "hide menu,"
    "with option to force it off"
);
Event EV_ScriptThread_PlayMovie // Added in 2.0
(
    "PlayMovie",
    EV_DEFAULT,
    "sI",
    "name",
    "Play a Movie..."
);
Event EV_ScriptThread_Pushmenu
(
    "pushmenu",
    EV_DEFAULT,
    "s",
    "name",
    "push menu"
);
Event EV_ScriptThread_HideMouse
(
    "hidemouse",
    EV_DEFAULT,
    NULL,
    NULL,
    "hide mouse cursor"
);
Event EV_ScriptThread_CreateListener
(
    "CreateListener",
    EV_DEFAULT,
    NULL,
    NULL,
    "Creates a Listener instance.",
    EV_RETURN
);
Event EV_ScriptThread_Trace
(
    "trace",
    EV_DEFAULT,
    "vvIVV",
    "start end pass_entities mins maxs",
    "Performs a Trace Line from the start to the end, returns the end or the position it hit at.",
    EV_RETURN
);
Event EV_ScriptThread_SightTrace
(
    "sighttrace",
    EV_DEFAULT,
    "vvIVV",
    "start end pass_entities mins maxs",
    "Performs a trace line from the start to the end, returns 0 if something was hit and 1 otherwise.",
    EV_RETURN
);
Event EV_ScriptThread_Print3D
(
    "print3d",
    EV_DEFAULT,
    "vfs",
    "origin scale string",
    "prints a string in 3D space",
    EV_NORMAL
);
Event EV_ScriptThread_HudDraw_3d
(
    "huddraw_3d",
    EV_DEFAULT,
    "iviiE",
    "index vector_or_offset always_show depth entity",
    "Sets this huddraw element to be a 3D world icon and can specify if this icon is always shown on-screen even if "
    "the player isn't looking at.\n"
    "Uses xy pos from huddraw_rect.\n"
    "If entity is specified, the vector will be an offset relative to the entity.\n"
    "depth specify if the icon is shown through walls.",
    EV_NORMAL
);
Event EV_ScriptThread_HudDraw_Timer
(
    "huddraw_timer",
    EV_DEFAULT,
    "iff",
    "index duration fade_out_time",
    "Sets a timer to be displayed and fade within the given time in seconds. Clears the string value and the shader "
    "value of huddraw element.",
    EV_NORMAL
);
Event EV_ScriptThread_HudDraw_Shader
(
    "huddraw_shader",
    EV_DEFAULT,
    "is",
    "index shader",
    "Sets the shader to use for a particular huddraw element for given player",
    EV_NORMAL
);
Event EV_ScriptThread_HudDraw_Align
(
    "huddraw_align",
    EV_DEFAULT,
    "iss",
    "index h_align v_align",
    "Sets the alignment of a huddraw element for given player. Specified with 'left', 'center', or 'right'",
    EV_NORMAL
);
Event EV_ScriptThread_HudDraw_Rect
(
    "huddraw_rect",
    EV_DEFAULT,
    "iiiii",
    "index x y width height",
    "Specifies the position of the upper left corner and size of a huddraw element for given player",
    EV_NORMAL
);
Event EV_ScriptThread_HudDraw_VirtualSize
(
    "huddraw_virtualsize",
    EV_DEFAULT,
    "ii",
    "index virtual",
    "Sets if the huddraw element for given player should use virutal screen resolution for positioning and size",
    EV_NORMAL
);
Event EV_ScriptThread_HudDraw_Color
(
    "huddraw_color",
    EV_DEFAULT,
    "ifff",
    "index red green blue",
    "Sets the color for a huddraw element for given player",
    EV_NORMAL
);
Event EV_ScriptThread_HudDraw_Alpha
(
    "huddraw_alpha",
    EV_DEFAULT,
    "if",
    "index alpha",
    "Sets the alpha of a huddraw element for given player",
    EV_NORMAL
);
Event EV_ScriptThread_HudDraw_String
(
    "huddraw_string",
    EV_DEFAULT,
    "is",
    "index string",
    "Sets a string to be displayed for given player. Clears the shader value",
    EV_NORMAL
);
Event EV_ScriptThread_HudDraw_Font
(
    "huddraw_font",
    EV_DEFAULT,
    "is",
    "index fontname",
    "Sets the font to use for given player huddraw element",
    EV_NORMAL
);
Event EV_ScriptThread_Error
(
    "error",
    EV_DEFAULT,
    NULL,
    NULL,
    "Ends the script",
    EV_NORMAL
);
Event EV_ScriptThread_DebugInt3 // Added in 2.0
(
    "debug_int3",
    EV_DEFAULT,
    NULL,
    NULL,
    "Generate an asm int 3",
    EV_NORMAL
);
Event EV_ScriptThread_Timeout
(
    "timeout",
    EV_DEFAULT,
    "f",
    "time",
    "specifies script timeout time",
    EV_NORMAL
);
Event EV_ScriptThread_RadiusDamage
(
    "radiusdamage",
    EV_DEFAULT,
    "viiI",
    "origin damage radius constant_damage",
    "radius damage at origin"
);
Event EV_ScriptThread_LandmineDamage // Added in 2.30
(
    "landminedamage",
    EV_DEFAULT,
    "eff",
    "landmine damage radius",
    "radius damage at landmine"
);
Event EV_ScriptThread_BspTransition
(
    "bsptransition",
    EV_DEFAULT,
    "s",
    "next_map",
    "Transitions to the next BSP. Keeps player data,"
    "and game data."
);
Event EV_ScriptThread_LevelTransition
(
    "leveltransition",
    EV_DEFAULT,
    "s",
    "next_map",
    "Transitions to the next Level. Statistics to Map Loading,"
    "does not keep player data or game data."
);
Event EV_ScriptThread_MissionTransition
(
    "missiontransition",
    EV_DEFAULT,
    "s",
    "next_map",
    "Transitions to the next Mission. Statistics to Main Menu,"
    "Next Level should be unlocked."
);
Event EV_ScriptThread_Earthquake
(
    "earthquake",
    EV_DEFAULT,
    "ffff",
    "duration magnitude no_rampup no_rampdown",
    "Create an earthquake"
);
Event EV_ScriptThread_TeamWin
(
    "teamwin",
    EV_DEFAULT,
    "s",
    "axis_or_allies",
    "Sets that the 'axis' or the 'allies' have won the map."
);
Event EV_ScriptThread_StopTeamRespawn // Added in 2.30
(
    "stopteamrespawn",
     EV_DEFAULT,
     "s",
     "team",
     "Stops a team (anerican, german, both) from spawning in non-wave deathmatches"
);
Event EV_ScriptThread_GetBoundKey1
(
    "getboundkey1",
    EV_DEFAULT,
    "s",
    "keyname",
    "return a string describing the key",
    EV_RETURN
);
Event EV_ScriptThread_GetBoundKey2
(
    "getboundkey2",
    EV_DEFAULT,
    "s",
    "keyname",
    "return a string describing the key",
    EV_RETURN
);
Event EV_ScriptThread_LocConvertString
(
    "loc_convert_string",
    EV_DEFAULT,
    "s",
    "in",
    "returns a localized version of the string.",
    EV_RETURN
);
Event EV_ScriptThread_SetScoreboardToggle // Added in 2.30
(
    "setscoreboardtoggle",
    EV_DEFAULT,
    "si",
    "team value",
    "Sets the specified team's scoreboard toggle",
    EV_NORMAL
);
Event EV_ScriptThread_AddObjective
(
    "addobjective",
    EV_DEFAULT,
    "iisv",
    "objective_number status text location",
    "Adds/Changes an Objective"
);
Event EV_ScriptThread_SetCurrentObjective
(
    "setcurrentobjective",
    EV_DEFAULT,
    "i",
    "objective_number",
    "Sets the specified objective as the current objective"
);
Event EV_ScriptThread_SetObjectiveLocation
(
    "set_objective_pos",
    EV_DEFAULT,
    "v",
    "pos",
    "Sets the position in the world of the current objective"
);
Event EV_ScriptThread_ClearObjectiveLocation(
    "clear_objective_pos",
    EV_DEFAULT,
    NULL,
    NULL,
    "Clears the position of the current objective,"
    "for when you don't have one"
);
Event EV_ScriptThread_DrawHud
(
    "drawhud",
    EV_DEFAULT,
    "i",
    "value",
    "Specify if hud is to be drawn"
);
Event EV_ScriptThread_RegisterCommand
(
    "registercmd",
    EV_DEFAULT,
    "ss",
    "name label",
    "Registers a command.",
    EV_NORMAL
);

//
// Openmohaa additions
//
Event EV_ScriptThread_CancelWaiting
(
    "_cancelwaiting",
    EV_CODEONLY,
    NULL,
    NULL,
    "internal event"
);
Event EV_ScriptThread_CanSwitchTeams
(
    "canswitchteams",
    EV_DEFAULT,
    "bB",
    "allow_team_change allow_spectator",
    "Specify if players are allowed to switch teams and spectate. Override player's canswitchteams.",
    EV_NORMAL
);
Event EV_ScriptThread_RemoveArchivedClass
(
    "removearchivedclass",
    EV_DEFAULT,
    "sI",
    "class except_entity_number",
    "Removes all of the simple archived entities in the specified class.",
    EV_NORMAL
);
Event EV_ScriptThread_RestoreSound(
    "restoresound",
    -1,
    "fFE",
    "time max_vol player",
    "Fades the sound in over the given time, optionally to one single player.\n"
    "max_vol being a 0-1 fraction is the maximum volume.",
    EV_NORMAL
);
Event EV_ScriptThread_Earthquake2
(
    "earthquake2",
    EV_DEFAULT,
    "ffbbVF",
    "duration magnitude no_rampup no_rampdown location radius",
    "Create a smooth realistic earthquake. Requires sv_reborn to be set.",
    EV_NORMAL
);
Event EV_ScriptThread_GetPlayerNetname
(
    "netname",
    EV_DEFAULT,
    "e",
    "player",
    "Gets player's netname",
    EV_RETURN
);
Event EV_ScriptThread_GetPlayerIP
(
    "getip",
    EV_DEFAULT,
    "e",
    "player",
    "Gets player's ip",
    EV_RETURN
);
Event EV_ScriptThread_ServerStufftext
(
    "stuffsrv",
    EV_DEFAULT,
    "s",
    "string",
    "Sends command to server",
    EV_NORMAL
);
Event EV_ScriptThread_GetAreaEntities
(
    "getareaentities",
    EV_DEFAULT,
    "vvv",
    "origin mins maxs",
    "Get all entities around the origin",
    EV_RETURN
);
Event EV_ScriptThread_GetPlayerPing
(
    "getping",
    EV_DEFAULT,
    "e",
    "player",
    "Gets player's ping",
    EV_RETURN
);
Event EV_ScriptThread_GetPlayerClientNum
(
    "getclientnum",
    EV_DEFAULT,
    "e",
    "player",
    "Gets player's client number",
    EV_RETURN
);
Event EV_ScriptThread_iHudDraw3d
(
    "ihuddraw_3d",
    EV_DEFAULT,
    "eiviiE",
    "player index vector_or_offset always_show depth entity",
    "Sets this huddraw element to be a 3D world icon for individual player and can specify if this icon is always "
    "shown on-screen even if the player isn't looking at.\n"
    "Uses xy pos from huddraw_rect.\n"
    "If entity is specified, the vector will be an offset relative to the entity.\n"
    "depth specify if the icon is shown through walls.",
    EV_NORMAL
);
Event EV_ScriptThread_iHudDrawAlign
(
    "ihuddraw_align",
    EV_DEFAULT,
    "eiss",
    "player index h_align v_align",
    "Sets the alignment of a huddraw element for given player. Specified with 'left', 'center', or 'right'",
    EV_NORMAL
);
Event EV_ScriptThread_iHudDrawAlpha
(
    "ihuddraw_alpha",
    EV_DEFAULT,
    "eif",
    "player index alpha",
    "Sets the alpha of a huddraw element for given player",
    EV_NORMAL
);
Event EV_ScriptThread_iHudDrawColor
(
    "ihuddraw_color",
    EV_DEFAULT,
    "eifff",
    "player index red green blue",
    "Sets the color for a huddraw element for given player",
    EV_NORMAL
);
Event EV_ScriptThread_iHudDrawFont
(
    "ihuddraw_font",
    EV_DEFAULT,
    "eis",
    "player index fontname",
    "Sets the font to use for given player huddraw element",
    EV_NORMAL
);
Event EV_ScriptThread_iHudDrawRect
(
    "ihuddraw_rect",
    EV_DEFAULT,
    "eiiiii",
    "player index x y width height",
    "Specifies the position of the upper left corner and size of a huddraw element for given player",
    EV_NORMAL
);
Event EV_ScriptThread_iHudDrawShader
(
    "ihuddraw_shader",
    EV_DEFAULT,
    "eis",
    "player index shader",
    "Sets the shader to use for a particular huddraw element for given player",
    EV_NORMAL
);
Event EV_ScriptThread_iHudDrawString
(
    "ihuddraw_string",
    EV_DEFAULT,
    "eis",
    "player index string",
    "Sets a string to be displayed for given player. Clears the shader value",
    EV_NORMAL
);
Event EV_ScriptThread_iHudDrawTimer
(
    "ihuddraw_timer",
    EV_DEFAULT,
    "eiff",
    "player index duration fade_out_time",
    "Sets a timer to be displayed and fade within the given time in seconds for given player. Clears the string value "
    "and the shader value of huddraw element.",
    EV_NORMAL
);
Event EV_ScriptThread_iHudDrawVirtualSize
(
    "ihuddraw_virtualsize",
    EV_DEFAULT,
    "eii",
    "player index virtual",
    "Sets if the huddraw element for given player should use virutal screen resolution for positioning and size",
    EV_NORMAL
);
Event EV_ScriptThread_IsArray
(
    "isarray",
    EV_DEFAULT,
    "s",
    "object",
    "Checks whether this variable is an array",
    EV_RETURN
);
Event EV_ScriptThread_IsDefined
(
    "isdefined",
    EV_DEFAULT,
    "s",
    "object",
    "Checks whether this entity/variable is defined",
    EV_RETURN
);
Event EV_ScriptThread_IsOnGround
(
    "isonground",
    EV_DEFAULT,
    "e",
    "entity",
    "Returns 1 if the entity is on ground. 0 otherwise",
    EV_RETURN
);
Event EV_ScriptThread_IsOutOfBounds
(
    "isoutofbounds",
    EV_DEFAULT,
    "e",
    "entity",
    "Returns 1 if the entity is out of bounds. 0 otherwise",
    EV_RETURN
);
Event EV_ScriptThread_FileOpen(
    "fopen",
    EV_DEFAULT,
    "ss",
    "filename accesstype",
    "Opens file,"
    "returning it's handle",
    EV_RETURN
);
Event EV_ScriptThread_FileWrite
(
    "fwrite",
    EV_DEFAULT,
    "eii",
    "player index virtual",
    "Writes binary buffer to file",
    EV_NORMAL
);
Event EV_ScriptThread_FileRead
(
    "fread",
    EV_DEFAULT,
    "eii",
    "player index virtual",
    "Reads binary buffer from file",
    EV_NORMAL
);
Event EV_ScriptThread_FileClose
(
    "fclose",
    EV_DEFAULT,
    "i",
    "filehandle",
    "Closes file of given file handle",
    EV_RETURN
);
Event EV_ScriptThread_FileEof
(
    "feof",
    EV_DEFAULT,
    "i",
    "filehandle",
    "Checks for end of file",
    EV_RETURN
);
Event EV_ScriptThread_FileSeek
(
    "fseek",
    EV_DEFAULT,
    "iii",
    "filehandle offset startpos",
    "Sets file carret at given position",
    EV_RETURN
);
Event EV_ScriptThread_FileTell
(
    "ftell",
    EV_DEFAULT,
    "i",
    "filehandle",
    "Gets current file carret position",
    EV_RETURN
);
Event EV_ScriptThread_FileRewind
(
    "frewind",
    EV_DEFAULT,
    "i",
    "filehandle",
    "Rewinds file carret to files beginning",
    EV_NORMAL
);
Event EV_ScriptThread_FilePutc
(
    "fputc",
    EV_DEFAULT,
    "ii",
    "filehandle character",
    "Writes single character to file",
    EV_RETURN
);
Event EV_ScriptThread_FilePuts
(
    "fputs",
    EV_DEFAULT,
    "is",
    "filehandle text",
    "Writes string line to file",
    EV_RETURN
);
Event EV_ScriptThread_FileGetc
(
    "fgetc",
    EV_DEFAULT,
    "i",
    "filehandle",
    "Reads single character from file",
    EV_RETURN
);
Event EV_ScriptThread_FileGets
(
    "fgets",
    EV_DEFAULT,
    "ii",
    "filehandle maxbuffsize",
    "Reads string line from file",
    EV_RETURN
);
Event EV_ScriptThread_FileError
(
    "ferror",
    EV_DEFAULT,
    "i",
    "filehandle",
    "Checks for last file i/o error",
    EV_RETURN
);
Event EV_ScriptThread_FileFlush
(
    "fflush",
    EV_DEFAULT,
    "i",
    "filehandle",
    "Flushes given stream. Writes all unsaved data from stream buffer to stream",
    EV_RETURN
);
Event EV_ScriptThread_FlagClear
(
    "flag_clear",
    EV_DEFAULT,
    "s",
    "name",
    "Clears and deletes a flag.",
    EV_NORMAL
);
Event EV_ScriptThread_FlagInit
(
    "flag_init",
    EV_DEFAULT,
    "s",
    "name",
    "Initializes a flag so it can be set. Flags MUST be initialized before they can be used for the first time.",
    EV_NORMAL
);
Event EV_ScriptThread_FlagSet
(
    "flag_set",
    EV_DEFAULT,
    "s",
    "name",
    "Sets a flag which alerts all flag_wait()'s and changes are reflected in flag() checks thereafter.",
    EV_NORMAL
);
Event EV_ScriptThread_FlagWait
(
    "flag_wait",
    EV_DEFAULT,
    "s",
    "name",
    "Pauses execution flow until a flag has been set.",
    EV_NORMAL
);
Event EV_ScriptThread_Lock
(
    "lock",
    EV_DEFAULT,
    "l",
    "lock",
    "Lock the thread."
);
Event EV_ScriptThread_UnLock
(
    "unlock",
    EV_DEFAULT,
    "l",
    "lock",
    "Unlock the thread."
);
Event EV_ScriptThread_GetArrayKeys
(
    "getarraykeys",
    EV_DEFAULT,
    "s",
    "array",
    "Retrieves a full list containing the name of arrays",
    EV_RETURN
);
Event EV_ScriptThread_GetArrayValues
(
    "getarrayvalues",
    EV_DEFAULT,
    "s",
    "array",
    "Retrieves the full list of an array that was set-up with name, such as local.array[ \"name\" ] and return their "
    "values",
    EV_RETURN
);
Event EV_ScriptThread_GetEntArray
(
    "getentarray",
    EV_DEFAULT,
    "ss",
    "name key",
    "Gets an array of entities that have the given key, name pair (case sensitive)",
    EV_RETURN
);
Event EV_ScriptThread_GetTime
(
    "gettime",
    EV_DEFAULT,
    NULL,
    NULL,
    "Gets current time",
    EV_RETURN
);
Event EV_ScriptThread_GetTimeZone
(
    "gettimezone",
    EV_DEFAULT,
    NULL,
    NULL,
    "Gets current time zone",
    EV_RETURN
);
Event EV_ScriptThread_PregMatch
(
    "preg_match",
    EV_DEFAULT,
    "ss",
    "pattern subject",
    "Searches subject for a match to the regular expression given in pattern.",
    EV_RETURN
);
Event EV_ScriptThread_GetDate
(
    "getdate",
    EV_DEFAULT,
    NULL,
    NULL,
    "Gets current date",
    EV_RETURN
);
Event EV_ScriptThread_RegisterEv
(
    "registerev",
    EV_DEFAULT,
    "ss",
    "eventname script",
    "Registers script callback handler for specified event",
    EV_RETURN
);
Event EV_ScriptThread_UnregisterEv
(
    "unregisterev",
    EV_DEFAULT,
    "s",
    "eventname",
    "Unregisters script callback handler for specified event",
    EV_RETURN
);
Event EV_ScriptThread_Conprintf
(
    "conprintf",
    EV_DEFAULT,
    "s",
    "text",
    "Prints to console",
    EV_NORMAL
);
Event EV_ScriptThread_TeamSwitchDelay
(
    "teamswitchdelay",
    EV_DEFAULT,
    "f",
    "delay",
    "Sets delay between switching teams. [DEPRECATED]",
    EV_NORMAL
);
Event EV_ScriptThread_CharToInt
(
    "chartoint",
    EV_DEFAULT,
    "s",
    "character",
    "Converts char to int",
    EV_RETURN
);
Event EV_ScriptThread_FileExists
(
    "fexists",
    EV_DEFAULT,
    "s",
    "filename",
    "Checks if file exists",
    EV_RETURN
);
Event EV_ScriptThread_FileReadAll
(
    "freadall",
    EV_DEFAULT,
    "i",
    "filehandle",
    "Reads whole file and returns it as string",
    EV_RETURN
);
Event EV_ScriptThread_FileSaveAll
(
    "fsaveall",
    EV_DEFAULT,
    "is",
    "filehandle text",
    "Saves whole text to file",
    EV_RETURN
);
Event EV_ScriptThread_FileRemove
(
    "fremove",
    EV_DEFAULT,
    "s",
    "filename",
    "Removes the file",
    EV_RETURN
);
Event EV_ScriptThread_FileRename
(
    "frename",
    EV_DEFAULT,
    "ss",
    "oldfilename newfilename",
    "Renames the file",
    EV_RETURN
);
Event EV_ScriptThread_FileCopy
(
    "fcopy",
    EV_DEFAULT,
    "ss",
    "filename copyfilename",
    "Copies the file",
    EV_RETURN
);
Event EV_ScriptThread_FileReadPak
(
    "freadpak",
    EV_DEFAULT,
    "s",
    "filename",
    "Reads file from pak file",
    EV_RETURN
);
Event EV_ScriptThread_FileList
(
    "flist",
    EV_DEFAULT,
    "ssi",
    "path extension wantSubs",
    "Returns the list of files with given extension",
    EV_RETURN
);
Event EV_ScriptThread_FileNewDirectory
(
    "fnewdir",
    EV_DEFAULT,
    "s",
    "path",
    "Creates a new directory",
    EV_RETURN
);
Event EV_ScriptThread_FileRemoveDirectory
(
    "fremovedir",
    EV_DEFAULT,
    "s",
    "path",
    "Removes a directory",
    EV_RETURN
);
Event EV_ScriptThread_GetACos
(
    "acos",
    EV_DEFAULT,
    "f",
    "x",
    "Compute arc cosine",
    EV_RETURN
);
Event EV_ScriptThread_GetASin
(
    "asin",
    EV_DEFAULT,
    "f",
    "x",
    "Compute arc sine",
    EV_RETURN
);
Event EV_ScriptThread_GetATan2
(
    "atan2",
    EV_DEFAULT,
    "ff",
    "x y",
    "Compute arc tangent with two parameters",
    EV_RETURN
);
Event EV_ScriptThread_GetCosH
(
    "cosh",
    EV_DEFAULT,
    "f",
    "x",
    "Compute hyperbolic cosine",
    EV_RETURN
);
Event EV_ScriptThread_GetSinH
(
    "sinh",
    EV_DEFAULT,
    "f",
    "x",
    "Compute hyperbolic sine",
    EV_RETURN
);
Event EV_ScriptThread_GetTanH
(
    "tanh",
    EV_DEFAULT,
    "f",
    "x",
    "Compute hyperbolic tangent",
    EV_RETURN
);
Event EV_ScriptThread_GetExp
(
    "exp",
    EV_DEFAULT,
    "f",
    "x",
    "Compute exponential function",
    EV_RETURN
);
Event EV_ScriptThread_GetFrexp
(
    "frexp",
    EV_DEFAULT,
    "f",
    "x",
    "Get significand and exponent",
    EV_RETURN
);
Event EV_ScriptThread_GetLdexp
(
    "ldexp",
    EV_DEFAULT,
    "f",
    "x",
    "Generate number from significand and exponent",
    EV_RETURN
);
Event EV_ScriptThread_GetLog
(
    "log",
    EV_DEFAULT,
    "f",
    "x",
    "Compute natural logarithm",
    EV_RETURN
);
Event EV_ScriptThread_GetLog10
(
    "log10",
    EV_DEFAULT,
    "f",
    "x",
    "Compute common logarithm",
    EV_RETURN
);
Event EV_ScriptThread_GetModf
(
    "modf",
    EV_DEFAULT,
    "f",
    "x",
    "Break into fractional and integral parts",
    EV_RETURN
);
Event EV_ScriptThread_GetPow
(
    "pow",
    EV_DEFAULT,
    "ff",
    "x y",
    "Raise to power",
    EV_RETURN
);
Event EV_ScriptThread_GetCeil
(
    "ceil",
    EV_DEFAULT,
    "f",
    "x",
    "Round up value",
    EV_RETURN
);
Event EV_ScriptThread_GetFloor
(
    "floor",
    EV_DEFAULT,
    "f",
    "x",
    "Round down value",
    EV_RETURN
);
Event EV_ScriptThread_GetFmod
(
    "fmod",
    EV_DEFAULT,
    "f",
    "x",
    "Compute remainder of division",
    EV_RETURN
);
Event EV_ScriptThread_strncpy
(
    "strncpy",
    EV_DEFAULT,
    "is",
    "bytes source",
    "Returns the copied string with the specified bytes",
    EV_RETURN
);
Event EV_ScriptThread_CreateHUD
(
    "newhud",
    EV_DEFAULT,
    "E",
    "player",
    "Create a new HUD element. Optionally for a particular client",
    EV_RETURN
);
Event EV_ScriptThread_TraceDetails(
    "traced",
    EV_DEFAULT,
    "vvIVVI",
    "start end pass_entities mins maxs mask",
    "Performs a Trace Line from the start to the end, returns the array with detailed results",
    EV_RETURN
);
Event EV_ScriptThread_TypeOf
(
    "typeof",
    EV_DEFAULT,
    "i",
    "variable",
    "Returns the type of variable",
    EV_RETURN
);
Event EV_ScriptThread_Md5String
(
    "md5string",
    EV_DEFAULT,
    "s",
    "text",
    "generates MD5 hash of given text",
    EV_RETURN
);
Event EV_ScriptThread_SetTimer
(
    "settimer",
    EV_DEFAULT,
    "is",
    "interval script",
    "Sets timer that will execute script after given interval.",
    EV_RETURN
);
Event EV_ScriptThread_TeamGetScore
(
    "team_getscore",
    EV_DEFAULT,
    "s",
    "team",
    "Get a team's current score.",
    EV_NORMAL
);
Event EV_ScriptThread_TeamSetScore
(
    "team_setscore",
    EV_DEFAULT,
    "siI",
    "team score bAdd",
    "Change/Add score to a team.",
    EV_NORMAL
);
Event EV_ScriptThread_VisionGetNaked
(
    "visiongetnaked",
    EV_DEFAULT,
    NULL,
    NULL,
    "Gets players' global current naked-eye vision.",
    EV_NORMAL
);
Event EV_ScriptThread_VisionSetNaked
(
    "visionsetnaked",
    EV_DEFAULT,
    "sF",
    "vision_name transition_time",
    "Sets players' naked-eye vision. Optionally give a transition time from the current vision. If vision_name is an "
    "empty string, it will be set to the current map's name.",
    EV_NORMAL
);

CLASS_DECLARATION(Listener, ScriptThread, NULL) {
    {&EV_ScriptThread_GetCvar,                 &ScriptThread::Getcvar                 },
    {&EV_ScriptThread_GetRandomFloat,          &ScriptThread::GetRandomFloat          },
    {&EV_ScriptThread_GetRandomInt,            &ScriptThread::GetRandomInt            },
    {&EV_ScriptThread_GetAbs,                  &ScriptThread::GetAbs                  },
    {&EV_ScriptThread_GetSelf,                 &ScriptThread::GetSelf                 },
    {&EV_ScriptThread_GetCos,                  &ScriptThread::EventCos                },
    {&EV_ScriptThread_GetSin,                  &ScriptThread::EventSin                },
    {&EV_ScriptThread_GetTan,                  &ScriptThread::EventTan                },
    {&EV_ScriptThread_GetATan,                 &ScriptThread::EventATan               },
    {&EV_ScriptThread_GetSqrt,                 &ScriptThread::EventSqrt               },
    {&EV_ScriptThread_Vector_Length,           &ScriptThread::Vector_Length           },
    {&EV_ScriptThread_Vector_Normalize,        &ScriptThread::Vector_Normalize        },
    {&EV_ScriptThread_Vector_Add,              &ScriptThread::Vector_Add              },
    {&EV_ScriptThread_Vector_Subtract,         &ScriptThread::Vector_Subtract         },
    {&EV_ScriptThread_Vector_Scale,            &ScriptThread::Vector_Scale            },
    {&EV_ScriptThread_Vector_DotProduct,       &ScriptThread::Vector_DotProduct       },
    {&EV_ScriptThread_Vector_CrossProduct,     &ScriptThread::Vector_CrossProduct     },
    {&EV_ScriptThread_Vector_ToAngles,         &ScriptThread::Vector_ToAngles         },
    {&EV_ScriptThread_AIsCloserThanBToC,       &ScriptThread::EventAIsCloserThanBToC  },
    {&EV_ScriptThread_PointsWithinDist,        &ScriptThread::EventPointsWithinDist   },
    {&EV_ScriptThread_Angles_ToForward,        &ScriptThread::Angles_ToForward        },
    {&EV_ScriptThread_Angles_ToLeft,           &ScriptThread::Angles_ToLeft           },
    {&EV_ScriptThread_Angles_ToUp,             &ScriptThread::Angles_ToUp             },
    {&EV_ScriptThread_Angles_PointAt,          &ScriptThread::Angles_PointAt          },
    {&EV_Listener_CreateThread,                &ScriptThread::CreateThread            },
    {&EV_Listener_CreateReturnThread,          &ScriptThread::CreateReturnThread      },
    {&EV_Listener_ExecuteScript,               &ScriptThread::ExecuteScript           },
    {&EV_Listener_ExecuteReturnScript,         &ScriptThread::ExecuteReturnScript     },
    {&EV_ScriptThread_Goto,                    &ScriptThread::EventGoto               },
    {&EV_Throw,                                &ScriptThread::EventThrow              },
    {&EV_DelayThrow,                           &ScriptThread::EventDelayThrow         },
    {&EV_ScriptThread_Wait,                    &ScriptThread::EventWait               },
    {&EV_ScriptThread_WaitFrame,               &ScriptThread::EventWaitFrame          },
    {&EV_ScriptThread_Pause,                   &ScriptThread::EventPause              },
    {&EV_ScriptThread_End,                     &ScriptThread::EventEnd                },
    {&EV_ScriptThread_Print,                   &ScriptThread::Print                   },
    {&EV_ScriptThread_Println,                 &ScriptThread::Println                 },
    {&EV_ScriptThread_DPrintln,                &ScriptThread::DPrintln                },
    {&EV_ScriptThread_IPrintln,                &ScriptThread::IPrintln                },
    {&EV_ScriptThread_IPrintlnBold,            &ScriptThread::IPrintlnBold            },
    {&EV_ScriptThread_IPrintln_NoLoc,          &ScriptThread::IPrintln_NoLoc          },
    {&EV_ScriptThread_IPrintlnBold_NoLoc,      &ScriptThread::IPrintlnBold_NoLoc      },
    {&EV_ScriptThread_MPrint,                  &ScriptThread::MPrint                  },
    {&EV_ScriptThread_MPrintln,                &ScriptThread::MPrintln                },
    {&EV_ScriptThread_Assert,                  &ScriptThread::Assert                  },
    {&EV_ScriptThread_CastInt,                 &ScriptThread::CastInt                 },
    {&EV_ScriptThread_CastFloat,               &ScriptThread::CastFloat               },
    {&EV_ScriptThread_CastString,              &ScriptThread::CastString              },
    {&EV_ScriptThread_CastBoolean,             &ScriptThread::CastBoolean             },
    {&EV_ScriptThread_CastEntity,              &ScriptThread::CastEntity              },
    {&EV_ScriptThread_Trigger,                 &ScriptThread::TriggerEvent            },
    {&EV_ScriptThread_ServerOnly,              &ScriptThread::ServerEvent             },
    {&EV_ScriptThread_StuffCommand,            &ScriptThread::StuffCommand            },
    {&EV_ScriptThread_Spawn,                   &ScriptThread::Spawn                   },
    {&EV_ScriptThread_SpawnReturn,             &ScriptThread::SpawnReturn             },
    {&EV_ScriptThread_Map,                     &ScriptThread::MapEvent                },
    {&EV_ScriptThread_AliasCache,              &ScriptThread::RegisterAliasAndCache   },
    {&EV_ScriptThread_Precache_Cache,          &ScriptThread::CacheResourceEvent      },
    {&EV_ScriptThread_SetCvar,                 &ScriptThread::SetCvarEvent            },
    {&EV_ScriptThread_DebugLine,               &ScriptThread::EventDebugLine          },
    {&EV_ScriptThread_CueCamera,               &ScriptThread::CueCamera               },
    {&EV_ScriptThread_CuePlayer,               &ScriptThread::CuePlayer               },
    {&EV_ScriptThread_FreezePlayer,            &ScriptThread::FreezePlayer            },
    {&EV_ScriptThread_ReleasePlayer,           &ScriptThread::ReleasePlayer           },
    {&EV_ScriptThread_FadeIn,                  &ScriptThread::FadeIn                  },
    {&EV_ScriptThread_FadeOut,                 &ScriptThread::FadeOut                 },
    {&EV_ScriptThread_FadeSound,               &ScriptThread::FadeSound               },
    {&EV_ScriptThread_ClearFade,               &ScriptThread::ClearFade               },
    {&EV_ScriptThread_Letterbox,               &ScriptThread::Letterbox               },
    {&EV_ScriptThread_ClearLetterbox,          &ScriptThread::ClearLetterbox          },
    {&EV_ScriptThread_MusicEvent,              &ScriptThread::MusicEvent              },
    {&EV_ScriptThread_ForceMusicEvent,         &ScriptThread::ForceMusicEvent         },
    {&EV_ScriptThread_MusicVolumeEvent,        &ScriptThread::MusicVolumeEvent        },
    {&EV_ScriptThread_RestoreMusicVolumeEvent, &ScriptThread::RestoreMusicVolumeEvent },
    {&EV_ScriptThread_SoundtrackEvent,         &ScriptThread::SoundtrackEvent         },
    {&EV_ScriptThread_RestoreSoundtrackEvent,  &ScriptThread::RestoreSoundtrackEvent  },
    {&EV_ScriptThread_CameraCommand,           &ScriptThread::CameraCommand           },
    {&EV_ScriptThread_SetCinematic,            &ScriptThread::SetCinematic            },
    {&EV_ScriptThread_SetNonCinematic,         &ScriptThread::SetNonCinematic         },
    {&EV_ScriptThread_AllAIOff,                &ScriptThread::SetAllAIOff             },
    {&EV_ScriptThread_AllAIOn,                 &ScriptThread::SetAllAIOn              },
    {&EV_ScriptThread_KillEnt,                 &ScriptThread::KillEnt                 },
    {&EV_ScriptThread_GetEntByEntnum,          &ScriptThread::GetEntByEntnum          },
    {&EV_ScriptThread_RemoveEnt,               &ScriptThread::RemoveEnt               },
    {&EV_ScriptThread_KillClass,               &ScriptThread::KillClass               },
    {&EV_ScriptThread_RemoveClass,             &ScriptThread::RemoveClass             },
    {&EV_ScriptThread_SetLightStyle,           &ScriptThread::SetLightStyle           },
    {&EV_ScriptThread_CenterPrint,             &ScriptThread::CenterPrint             },
    {&EV_ScriptThread_LocationPrint,           &ScriptThread::LocationPrint           },
    {&EV_ScriptThread_MissionFailed,           &ScriptThread::MissionFailed           },
    {&EV_ScriptThread_IsAlive,                 &ScriptThread::EventIsAlive            },
    {&EV_ScriptThread_Popmenu,                 &ScriptThread::EventPopmenu            },
    {&EV_ScriptThread_Showmenu,                &ScriptThread::EventPopmenu            },
    {&EV_ScriptThread_Hidemenu,                &ScriptThread::EventHidemenu           },
    {&EV_ScriptThread_PlayMovie,               &ScriptThread::EventPlayMovie          },
    {&EV_ScriptThread_Pushmenu,                &ScriptThread::EventPushmenu           },
    {&EV_ScriptThread_HideMouse,               &ScriptThread::EventHideMouse          },
    {&EV_ScriptThread_CreateListener,          &ScriptThread::EventCreateListener     },
    {&EV_ScriptThread_Trace,                   &ScriptThread::EventTrace              },
    {&EV_ScriptThread_SightTrace,              &ScriptThread::EventSightTrace         },
    {&EV_ScriptThread_Print3D,                 &ScriptThread::EventPrint3D            },
    {&EV_ScriptThread_HudDraw_Shader,          &ScriptThread::EventHudDrawShader      },
    {&EV_ScriptThread_HudDraw_Align,           &ScriptThread::EventHudDrawAlign       },
    {&EV_ScriptThread_HudDraw_Rect,            &ScriptThread::EventHudDrawRect        },
    {&EV_ScriptThread_HudDraw_VirtualSize,     &ScriptThread::EventHudDrawVirtualSize },
    {&EV_ScriptThread_HudDraw_Color,           &ScriptThread::EventHudDrawColor       },
    {&EV_ScriptThread_HudDraw_Alpha,           &ScriptThread::EventHudDrawAlpha       },
    {&EV_ScriptThread_HudDraw_String,          &ScriptThread::EventHudDrawString      },
    {&EV_ScriptThread_HudDraw_Font,            &ScriptThread::EventHudDrawFont        },
    {&EV_ScriptThread_Timeout,                 &ScriptThread::EventTimeout            },
    {&EV_ScriptThread_Error,                   &ScriptThread::EventError              },
    {&EV_ScriptThread_DebugInt3,               &ScriptThread::EventDebugInt3          },
    {&EV_ScriptThread_RadiusDamage,            &ScriptThread::EventRadiusDamage       },
    {&EV_ScriptThread_LandmineDamage,          &ScriptThread::EventLandmineDamage     },
    {&EV_ScriptThread_BspTransition,           &ScriptThread::EventBspTransition      },
    {&EV_ScriptThread_LevelTransition,         &ScriptThread::EventLevelTransition    },
    {&EV_ScriptThread_MissionTransition,       &ScriptThread::EventMissionTransition  },
    {&EV_ScriptThread_Earthquake,              &ScriptThread::EventEarthquake         },
    {&EV_ScriptThread_TeamWin,                 &ScriptThread::EventTeamWin            },
    {&EV_ScriptThread_StopTeamRespawn,         &ScriptThread::EventStopTeamRespawn    },
    {&EV_ScriptThread_GetBoundKey1,            &ScriptThread::EventGetBoundKey1       },
    {&EV_ScriptThread_GetBoundKey2,            &ScriptThread::EventGetBoundKey2       },
    {&EV_ScriptThread_LocConvertString,        &ScriptThread::EventLocConvertString   },
    {&EV_ScriptThread_SetScoreboardToggle,     &ScriptThread::EventSetScoreboardToggle},
    {&EV_ScriptThread_AddObjective,            &ScriptThread::EventAddObjective       },
    {&EV_ScriptThread_SetCurrentObjective,     &ScriptThread::EventSetCurrentObjective},
    {&EV_ScriptThread_SetObjectiveLocation,    &ScriptThread::SetObjectiveLocation    },
    {&EV_ScriptThread_ClearObjectiveLocation,  &ScriptThread::ClearObjectiveLocation  },
    {&EV_ScriptThread_DrawHud,                 &ScriptThread::EventDrawHud            },

    {&EV_ScriptThread_RegisterCommand,         &ScriptThread::EventRegisterCommand    },
    {&EV_ScriptThread_IsArray,                 &ScriptThread::EventIsArray            },
    {&EV_ScriptThread_IsDefined,               &ScriptThread::EventIsDefined          },
    {&EV_ScriptThread_GetACos,                 &ScriptThread::EventACos               },
    {&EV_ScriptThread_GetASin,                 &ScriptThread::EventASin               },
    {&EV_ScriptThread_GetATan2,                &ScriptThread::EventATan2              },
    {&EV_ScriptThread_GetCeil,                 &ScriptThread::EventCeil               },
    {&EV_ScriptThread_GetCosH,                 &ScriptThread::EventCosH               },
    {&EV_ScriptThread_GetExp,                  &ScriptThread::EventExp                },
    {&EV_ScriptThread_GetFloor,                &ScriptThread::EventFloor              },
    {&EV_ScriptThread_GetFmod,                 &ScriptThread::EventFmod               },
    {&EV_ScriptThread_GetFrexp,                &ScriptThread::EventFrexp              },
    {&EV_ScriptThread_GetLdexp,                &ScriptThread::EventLdexp              },
    {&EV_ScriptThread_GetLog,                  &ScriptThread::EventLog                },
    {&EV_ScriptThread_GetLog10,                &ScriptThread::EventLog10              },
    {&EV_ScriptThread_GetModf,                 &ScriptThread::EventModf               },
    {&EV_ScriptThread_GetPow,                  &ScriptThread::EventPow                },
    {&EV_ScriptThread_GetSinH,                 &ScriptThread::EventSinH               },
    {&EV_ScriptThread_GetTanH,                 &ScriptThread::EventTanH               },
    {&EV_ScriptThread_strncpy,                 &ScriptThread::StringBytesCopy         },
    {&EV_ScriptThread_Md5String,               &ScriptThread::Md5String               },
    {&EV_ScriptThread_TypeOf,                  &ScriptThread::TypeOfVariable          },
    {&EV_ScriptThread_RegisterEv,              &ScriptThread::RegisterEvent           },
    {&EV_ScriptThread_UnregisterEv,            &ScriptThread::UnregisterEvent         },
    {&EV_ScriptThread_CancelWaiting,           &ScriptThread::CancelWaiting           },
    {&EV_ScriptThread_GetTime,                 &ScriptThread::GetTime                 },
    {&EV_ScriptThread_GetTimeZone,             &ScriptThread::GetTimeZone             },
    {&EV_ScriptThread_PregMatch,               &ScriptThread::PregMatch               },
    {&EV_ScriptThread_FlagClear,               &ScriptThread::FlagClear               },
    {&EV_ScriptThread_FlagInit,                &ScriptThread::FlagInit                },
    {&EV_ScriptThread_FlagSet,                 &ScriptThread::FlagSet                 },
    {&EV_ScriptThread_FlagWait,                &ScriptThread::FlagWait                },
    {&EV_ScriptThread_HudDraw_3d,              &ScriptThread::EventHudDraw3d          },
    {&EV_ScriptThread_HudDraw_Timer,           &ScriptThread::EventHudDrawTimer       },
    {&EV_ScriptThread_CanSwitchTeams,          &ScriptThread::CanSwitchTeams          },
    {&EV_ScriptThread_CharToInt,               &ScriptThread::CharToInt               },
    {&EV_ScriptThread_Conprintf,               &ScriptThread::Conprintf               },
    {&EV_ScriptThread_CreateHUD,               &ScriptThread::CreateHUD               },
    {&EV_ScriptThread_Earthquake2,             &ScriptThread::Earthquake              },
    {&EV_ScriptThread_FadeSound,               &ScriptThread::FadeSound               },
    {&EV_ScriptThread_FileClose,               &ScriptThread::FileClose               },
    {&EV_ScriptThread_FileCopy,                &ScriptThread::FileCopy                },
    {&EV_ScriptThread_FileEof,                 &ScriptThread::FileEof                 },
    {&EV_ScriptThread_FileError,               &ScriptThread::FileError               },
    {&EV_ScriptThread_FileExists,              &ScriptThread::FileExists              },
    {&EV_ScriptThread_FileFlush,               &ScriptThread::FileFlush               },
    {&EV_ScriptThread_FileGetc,                &ScriptThread::FileGetc                },
    {&EV_ScriptThread_FileGets,                &ScriptThread::FileGets                },
    {&EV_ScriptThread_FileList,                &ScriptThread::FileList                },
    {&EV_ScriptThread_FileNewDirectory,        &ScriptThread::FileNewDirectory        },
    {&EV_ScriptThread_FileOpen,                &ScriptThread::FileOpen                },
    {&EV_ScriptThread_FilePutc,                &ScriptThread::FilePutc                },
    {&EV_ScriptThread_FilePuts,                &ScriptThread::FilePuts                },
    {&EV_ScriptThread_FileRead,                &ScriptThread::FileRead                },
    {&EV_ScriptThread_FileReadAll,             &ScriptThread::FileReadAll             },
    {&EV_ScriptThread_FileRemove,              &ScriptThread::FileRemove              },
    {&EV_ScriptThread_FileRemoveDirectory,     &ScriptThread::FileRemoveDirectory     },
    {&EV_ScriptThread_FileRename,              &ScriptThread::FileRename              },
    {&EV_ScriptThread_FileRewind,              &ScriptThread::FileRewind              },
    {&EV_ScriptThread_FileSaveAll,             &ScriptThread::FileSaveAll             },
    {&EV_ScriptThread_FileSeek,                &ScriptThread::FileSeek                },
    {&EV_ScriptThread_FileTell,                &ScriptThread::FileTell                },
    {&EV_ScriptThread_FileWrite,               &ScriptThread::FileWrite               },
    {&EV_ScriptThread_GetAreaEntities,         &ScriptThread::GetAreaEntities         },
    {&EV_ScriptThread_GetArrayKeys,            &ScriptThread::GetArrayKeys            },
    {&EV_ScriptThread_GetArrayValues,          &ScriptThread::GetArrayValues          },
    {&EV_ScriptThread_GetDate,                 &ScriptThread::GetDate                 },
    {&EV_ScriptThread_GetEntArray,             &ScriptThread::GetEntArray             },
    {&EV_ScriptThread_GetPlayerClientNum,      &ScriptThread::GetPlayerClientNum      },
    {&EV_ScriptThread_GetPlayerIP,             &ScriptThread::GetPlayerIP             },
    {&EV_ScriptThread_GetPlayerNetname,        &ScriptThread::GetPlayerNetname        },
    {&EV_ScriptThread_GetPlayerPing,           &ScriptThread::GetPlayerPing           },
    {&EV_ScriptThread_iHudDraw3d,              &ScriptThread::EventIHudDraw3d         },
    {&EV_ScriptThread_iHudDrawAlign,           &ScriptThread::EventIHudDrawAlign      },
    {&EV_ScriptThread_iHudDrawAlpha,           &ScriptThread::EventIHudDrawAlpha      },
    {&EV_ScriptThread_iHudDrawColor,           &ScriptThread::EventIHudDrawColor      },
    {&EV_ScriptThread_iHudDrawFont,            &ScriptThread::EventIHudDrawFont       },
    {&EV_ScriptThread_iHudDrawRect,            &ScriptThread::EventIHudDrawRect       },
    {&EV_ScriptThread_iHudDrawShader,          &ScriptThread::EventIHudDrawShader     },
    {&EV_ScriptThread_iHudDrawString,          &ScriptThread::EventIHudDrawString     },
    {&EV_ScriptThread_iHudDrawTimer,           &ScriptThread::EventIHudDrawTimer      },
    {&EV_ScriptThread_iHudDrawVirtualSize,     &ScriptThread::EventIHudDrawVirtualSize},
    {&EV_ScriptThread_IsOnGround,              &ScriptThread::EventIsOnGround         },
    {&EV_ScriptThread_IsOutOfBounds,           &ScriptThread::EventIsOutOfBounds      },
    {&EV_ScriptThread_RestoreSound,            &ScriptThread::RestoreSound            },
    {&EV_ScriptThread_RemoveArchivedClass,     &ScriptThread::RemoveArchivedClass     },
    {&EV_ScriptThread_ServerStufftext,         &ScriptThread::ServerStufftext         },
    {&EV_ScriptThread_SetTimer,                &ScriptThread::SetTimer                },
    {&EV_ScriptThread_TeamGetScore,            &ScriptThread::TeamGetScore            },
    {&EV_ScriptThread_TeamSetScore,            &ScriptThread::TeamSetScore            },
    {&EV_ScriptThread_TraceDetails,            &ScriptThread::TraceDetails            },
    {&EV_ScriptThread_VisionGetNaked,          &ScriptThread::VisionGetNaked          },
    {&EV_ScriptThread_VisionSetNaked,          &ScriptThread::VisionSetNaked          },
    {NULL,                                     NULL                                   }
};

MEM_BlockAlloc<ScriptThread> ScriptThread_allocator;

void *ScriptThread::operator new(size_t size)
{
    return ScriptThread_allocator.Alloc();
}

void ScriptThread::operator delete(void *ptr)
{
    ScriptThread_allocator.Free(ptr);
}

ScriptThread::ScriptThread()
{
    m_ScriptVM = NULL;
}

ScriptThread::ScriptThread(ScriptClass *scriptClass, unsigned char *pCodePos)
{
    m_ScriptVM = new ScriptVM(scriptClass, pCodePos, this);

    m_ScriptVM->m_ThreadState = THREAD_RUNNING;

    if (g_scripttrace->integer && CanScriptTracePrint()) {
        gi.DPrintf2("+++Constructor THREAD: %p %p\n", this, scriptClass);
    }
}

ScriptThread::~ScriptThread()
{
    ScriptVM* vm;

    if (g_scripttrace->integer && CanScriptTracePrint()) {
        gi.DPrintf2("---Destructor THREAD: %p\n", this);
    }

    assert(m_ScriptVM);
    if (!m_ScriptVM) {
        // should never happen
        throw ScriptException("Attempting to delete a dead thread.");
    }

    vm = m_ScriptVM;
    m_ScriptVM = NULL;
    if (vm->ThreadState() == THREAD_WAITING) {
        vm->m_ThreadState = THREAD_RUNNING;
        Director.RemoveTiming(this);
    } else if (vm->ThreadState() == THREAD_SUSPENDED) {
        vm->m_ThreadState = THREAD_RUNNING;
        CancelWaitingAll();
    }

    vm->NotifyDelete();
}

ScriptClass *ScriptThread::GetScriptClass(void)
{
    return m_ScriptVM->GetScriptClass();
}

str ScriptThread::FileName(void)
{
    return m_ScriptVM->Filename();
}

void ScriptThread::StartedWaitFor(void)
{
    Stop();
    StartWaiting();
    m_ScriptVM->Suspend();
}

void ScriptThread::StoppedWaitFor(const_str name, bool bDeleting)
{
    if (!m_ScriptVM) {
        return;
    }

    // The thread is deleted if the listener is deleting
    if (bDeleting) {
        delete this;
        return;
    }

    ///////
    // Openmohaa: allows waittill to timeout
    //
    CancelEventsOfType(EV_ScriptThread_CancelWaiting);
    ///////

    if (m_ScriptVM->m_ThreadState == THREAD_SUSPENDED) {
        if (name != 0) {
            if (m_ScriptVM->IsSuspended()) {
                Execute();
            } else {
                m_ScriptVM->Resume();
            }
        } else {
            StartTiming();
        }
    }
}

void ScriptThread::StoppedNotify(void)
{
    // This is invalid and we mustn't get here
    if (m_ScriptVM) {
        delete this;
    }
}

void ScriptThread::ScriptExecute(ScriptVariable *data, int dataSize, ScriptVariable& returnValue)
{
    m_ScriptVM->m_ReturnValue = returnValue;

    ScriptExecuteInternal(data, dataSize);
}

void ScriptThread::ScriptExecuteInternal(ScriptVariable *data, int dataSize)
{
    SafePtr<ScriptThread> currentThread;
    SafePtr<ScriptThread> previousThread;

    currentThread = Director.CurrentThread();
    previousThread = this;

    Director.m_PreviousThread = Director.CurrentThread();
    Director.m_CurrentThread  = this;

    Stop();
    m_ScriptVM->Execute(data, dataSize);

    // restore the previous values
    Director.m_CurrentThread  = currentThread;
    Director.m_PreviousThread = previousThread;

    Director.ExecuteRunning();
}

void ScriptThread::Execute(Event& ev)
{
    Execute(&ev);
}

void ScriptThread::Execute(Event *ev)
{
    assert(m_ScriptVM);

    try {
        if (ev == NULL) {
            ScriptExecuteInternal();
        } else {
            ScriptVariable returnValue;

            returnValue.newPointer();

            ScriptExecute(ev->data, ev->dataSize, returnValue);

            ev->AddValue(returnValue);
        }
    } catch (ScriptException& exc) {
        if (exc.bAbort) {
            gi.Error(ERR_DROP, "%s\n", exc.string.c_str());
        } else {
            Com_Printf("^~^~^ Script Error: %s\n", exc.string.c_str());
        }
    }
}

void ScriptThread::Execute(ScriptVariable *data, int dataSize)
{
    ScriptExecuteInternal(data, dataSize);
}

void ScriptThread::Execute()
{
    if (Director.iPaused > 0) {
        StartTiming();
    } else {
        ScriptExecuteInternal(NULL, 0);
    }
}

ScriptThread *ScriptThread::CreateThreadInternal(const ScriptVariable& label)
{
    return GetScriptClass()->CreateThreadInternal(label);
}

ScriptThread *ScriptThread::CreateScriptInternal(const ScriptVariable& label)
{
    return GetScriptClass()->CreateScriptInternal(label);
}

void ScriptThread::StartTiming(void)
{
    Stop();

    m_ScriptVM->m_ThreadState = THREAD_WAITING;

    Director.AddTiming(this, level.inttime);
}

void ScriptThread::StartTiming(int time)
{
    Stop();

    m_ScriptVM->m_ThreadState = THREAD_WAITING;

    if (time < level.inttime) {
        time = level.inttime;
    }

    Director.AddTiming(this, time);
}

void ScriptThread::StartWaiting()
{
    m_ScriptVM->m_ThreadState = THREAD_SUSPENDED;
}

void ScriptThread::Stop(void)
{
    if (m_ScriptVM->ThreadState() == THREAD_WAITING) {
        m_ScriptVM->m_ThreadState = THREAD_RUNNING;
        Director.RemoveTiming(this);
    } else if (m_ScriptVM->ThreadState() == THREAD_SUSPENDED) {
        m_ScriptVM->m_ThreadState = THREAD_RUNNING;
        CancelWaitingAll();
    }
}

void ScriptThread::Wait(float time)
{
    StartTiming(level.inttime + time);
    m_ScriptVM->Suspend();
}

void ScriptThread::Pause()
{
    Stop();
    m_ScriptVM->Suspend();
}

void ScriptThread::Getcvar(Event *ev)
{
    str varName = ev->GetString(1);
    str s = gi.Cvar_Get(varName.c_str(), "", 0)->string;

    if (strstr(s.c_str(), ".")) {
        for (int i = s.length() - 1; i >= 0; i--) {
            if (s[i] == '0') {
                s[i] = 0;
            } else {
                if (s[i] == '.') {
                    s[i] = 0;
                }
                break;
            }
        }
    }

    ev->AddString(s);
}

void ScriptThread::GetRandomFloat(Event *ev)
{
    ev->AddFloat(G_Random(ev->GetFloat(1)));
}

void ScriptThread::GetRandomInt(Event *ev)
{
    ev->AddInteger(G_Random(ev->GetInteger(1)));
}

void ScriptThread::GetAbs(Event *ev)
{
    ScriptVariable& val = ev->GetValue(1);
    if (val.GetType() == VARIABLE_INTEGER) {
        ev->AddInteger(ev->GetInteger(1));
    } else {
        if (val.GetType() != VARIABLE_FLOAT) {
            throw ScriptException("abs applied to bad type '%s'", val.GetTypeName());
        }
        ev->AddFloat(fabs(ev->GetFloat(1)));
    }
}

void ScriptThread::EventCos(Event *ev)
{
    float arg;
    float value;

    arg   = ev->GetFloat(1);
    value = cos(DEG2RAD(arg));
    if (isnan(value)) {
        ScriptError("cos(%f) = NaN", arg);
    }

    ev->AddFloat(value);
}

void ScriptThread::EventSin(Event *ev)
{
    float arg;
    float value;

    arg   = ev->GetFloat(1);
    value = sin(DEG2RAD(arg));
    if (isnan(value)) {
        ScriptError("sin(%f) = NaN", arg);
    }

    ev->AddFloat(value);
}

void ScriptThread::EventTan(Event *ev)
{
    float arg;
    float value;

    arg   = ev->GetFloat(1);
    value = tan(DEG2RAD(arg));
    if (isnan(value)) {
        ScriptError("tan(%f) = NaN", arg);
    }

    ev->AddFloat(value);
}

void ScriptThread::EventATan(Event *ev)
{
    float arg, arg2;
    float value;

    if (ev->NumArgs() > 1) {
        arg   = ev->GetFloat(1);
        arg2  = ev->GetFloat(2);
        value = RAD2DEG(atan2(arg, arg2));
        if (isnan(value)) {
            ScriptError("atan(%f, %f) = NaN", arg, arg2);
        }
    } else {
        arg   = ev->GetFloat(1);
        value = RAD2DEG(atan(arg));
        if (isnan(value)) {
            ScriptError("atan(%f) = NaN", arg);
        }
    }

    ev->AddFloat(value);
}

void ScriptThread::EventSqrt(Event *ev)
{
    float arg;
    float value;

    arg   = ev->GetFloat(1);
    value = sqrtf(DEG2RAD(arg));
    if (isnan(value)) {
        ScriptError("sqrt(%f) = NaN", arg);
    }

    ev->AddFloat(value);
}

void ScriptThread::GetSelf(Event *ev)
{
    ev->AddListener(m_ScriptVM->GetScriptClass()->GetSelf());
}

void ScriptThread::CreateReturnThread(Event *ev)
{
    m_ScriptVM->GetScriptClass()->CreateReturnThread(ev);
}

void ScriptThread::CreateThread(Event *ev)
{
    m_ScriptVM->GetScriptClass()->CreateThread(ev);
}

void ScriptThread::ExecuteScript(Event *ev)
{
    m_ScriptVM->GetScriptClass()->ExecuteScript(ev);
}

void ScriptThread::ExecuteReturnScript(Event *ev)
{
    m_ScriptVM->GetScriptClass()->ExecuteReturnScript(ev);
}

void ScriptThread::EventGoto(Event *ev)
{
    m_ScriptVM->EventGoto(ev);

    if (m_ScriptVM->IsSuspended()) {
        ScriptExecuteInternal();
    } else {
        Stop();
        m_ScriptVM->Resume();
    }
}

void ScriptThread::EventThrow(Event *ev)
{
    if (!m_ScriptVM->m_PrevCodePos) {
        return;
    }

    if (m_ScriptVM->EventThrow(ev)) {
        if (m_ScriptVM->IsSuspended()) {
            ScriptExecuteInternal();
        } else {
            Stop();
            m_ScriptVM->Resume();
        }
    } else {
        // we make sure this won't get deleted
        SafePtr<ScriptThread> This = this;

        Stop();

        if (!BroadcastEvent(0, *ev)) {
            m_ScriptVM->GetScriptClass()->EventThrow(ev);
        }

        if (This) {
            delete this;
        }
    }
}

// seems to work like EventThrow
void ScriptThread::EventDelayThrow(Event *ev)
{
    if (!m_ScriptVM->m_PrevCodePos) {
        return;
    }

    if (m_ScriptVM->EventThrow(ev)) {
        if (m_ScriptVM->IsSuspended()) {
            StartTiming();
        } else {
            Stop();
            m_ScriptVM->Resume();
        }
    } else {
        // we make sure this won't get deleted
        SafePtr<ScriptThread> This = this;

        Stop();

        if (!BroadcastEvent(0, *ev)) {
            m_ScriptVM->GetScriptClass()->EventDelayThrow(ev);
        }

        if (This) {
            delete this;
        }
    }
}

void ScriptThread::EventWait(Event *ev)
{
    Wait((int)(ev->GetFloat(1) * 1000.0f + 0.5f));
}

void ScriptThread::EventWaitFrame(Event *ev)
{
    Wait((int)(level.frametime * 1000.0f + 0.5f));
}

void ScriptThread::EventResume(Event *ev)
{
    // Empty
}

void ScriptThread::EventPause(Event *ev)
{
    Pause();
}

void ScriptThread::EventEnd(Event *ev)
{
    if (ev->NumArgs() > 0) {
        m_ScriptVM->End(ev->GetValue(1));
    } else {
        m_ScriptVM->End();
    }
}

void ScriptThread::Print(Event *ev)
{
    if (!developer->integer) {
        return;
    }

    for (int i = 1; i <= ev->NumArgs(); i++) {
        gi.DPrintf2(ev->GetString(i).c_str());
    }
}

void ScriptThread::Println(Event *ev)
{
    if (!developer->integer) {
        return;
    }

    Print(ev);
    gi.DPrintf2("\n");
}

void ScriptThread::DPrintln(Event *ev)
{
    gentity_t *ent;
    int        i;
    str        message;

    if (!developer->integer) {
        return;
    }

    for (i = 1; i <= ev->NumArgs(); i++) {
        message += ev->GetString(i);
    }

    for (i = 0, ent = g_entities; i < game.maxclients; i++, ent++) {
        if (!ent->inuse || !ent->client) {
            continue;
        }

        gi.SendServerCommand(i, "print \"" HUD_MESSAGE_YELLOW "%s\n\"", message.c_str());
    }
}

void ScriptThread::IPrintln(Event *ev)
{
    str sString;
    int num = ev->NumArgs();
    int i;

    for (i = 1; i <= ev->NumArgs(); i++) {
        sString += ev->GetString(i);
    }

    G_PrintToAllClients(gi.LV_ConvertString(sString), false);
}

void ScriptThread::IPrintlnBold(Event *ev)
{
    str sString;
    int num = ev->NumArgs();
    int i;

    for (i = 1; i <= ev->NumArgs(); i++) {
        sString += ev->GetString(i);
    }

    G_PrintToAllClients(gi.LV_ConvertString(sString), true);
}

void ScriptThread::IPrintln_NoLoc(Event *ev)
{
    str sString;
    int num = ev->NumArgs();
    int i;

    for (i = 1; i <= ev->NumArgs(); i++) {
        sString += ev->GetString(i);
    }

    G_PrintToAllClients(sString, false);
}

void ScriptThread::IPrintlnBold_NoLoc(Event *ev)
{
    str sString;
    int num = ev->NumArgs();
    int i;

    for (i = 1; i <= ev->NumArgs(); i++) {
        sString += ev->GetString(i);
    }

    G_PrintToAllClients(sString, true);
}

void ScriptThread::MPrint(Event *ev)
{
    SimpleEntity *m_Self = (SimpleEntity *)m_ScriptVM->GetScriptClass()->GetSelf();

    if (!m_Self || !m_Self->isSubclassOf(SimpleEntity)) {
        return;
    }

    for (int i = 1; i <= ev->NumArgs(); i++) {
        m_Self->MPrintf(ev->GetString(i));
    }
}

void ScriptThread::MPrintln(Event *ev)
{
    SimpleEntity *m_Self = (SimpleEntity *)m_ScriptVM->GetScriptClass()->GetSelf();

    if (!m_Self || !m_Self->isSubclassOf(SimpleEntity)) {
        return;
    }

    MPrint(ev);
    m_Self->MPrintf("\n");
}

void ScriptThread::Assert(Event *ev)
{
    assert(ev->GetFloat(1));
}

void ScriptThread::CastInt(Event *ev)
{
    ev->AddInteger(ev->GetInteger(1));
}

void ScriptThread::CastFloat(Event *ev)
{
    ev->AddFloat(ev->GetFloat(1));
}

void ScriptThread::CastString(Event *ev)
{
    ev->AddString(ev->GetString(1));
}

void ScriptThread::CastBoolean(Event *ev)
{
    ev->AddInteger(ev->GetBoolean(1));
}

void ScriptThread::CastEntity(Event *ev)
{
    ev->AddListener((Listener *)ev->GetEntity(1));
}

void ScriptThread::TriggerEvent(Event *ev)
{
    ScriptVariable var;
    Entity        *ent;

    var = ev->GetValue(1);
    var.CastConstArrayValue();

    for (int i = var.arraysize(); i > 0; i--) {
        ent = var[i]->entityValue();
        if (ent) {
            Event *event = new Event(EV_Activate);
            event->AddEntity(world);
            ent->ProcessEvent(event);
        }
    }
}

void ScriptThread::RegisterAliasAndCache(Event *ev)
{
    //assert(!"ScriptThread::RegisterAliasAndCache needs to be implemented like ClientGameCommandManager::AliasCache");
    gi.DPrintf("ScriptThread::RegisterAliasAndCache needs to be implemented like ClientGameCommandManager::AliasCache\n");
}

void ScriptThread::CacheResourceEvent(Event *ev)
{
    CacheResource(ev->GetString(1));
}

void ScriptThread::MapEvent(Event *ev)
{
    if (level.mission_failed) {
        return;
    }

    G_BeginIntermission(ev->GetString(1), TRANS_BSP);
}

void ScriptThread::SetCvarEvent(Event *ev)
{
    gi.cvar_set(ev->GetString(1), ev->GetString(2));
}

void ScriptThread::EventDebugLine(Event *ev)
{
    Vector start, end;
    float  red, green, blue, alpha;

    red   = 1;
    green = 1;
    blue  = 1;
    alpha = 1;

    if (ev->NumArgs() == 5) {
        red   = ev->GetFloat(3);
        green = ev->GetFloat(4);
        blue  = ev->GetFloat(5);
    } else if (ev->NumArgs() == 6) {
        red   = ev->GetFloat(3);
        green = ev->GetFloat(4);
        blue  = ev->GetFloat(5);
        alpha = ev->GetFloat(6);
    } else if (ev->NumArgs() != 2) {
        ScriptError("debugline should have 2, 5, or 6 arguments");
    }

    if (ev->IsVectorAt(1)) {
        start = ev->GetVector(1);
    } else if (ev->GetEntity(1)) {
        start = ev->GetEntity(1)->centroid;
    } else {
        ScriptError("argument 1 to debugline should be an entity or vector");
    }

    if (ev->IsVectorAt(2)) {
        end = ev->GetVector(2);
    } else if (ev->GetEntity(2)) {
        end = ev->GetEntity(2)->centroid;
    } else {
        ScriptError("argument 2 to debugline should be an entity or vector");
    }

    G_DebugLine(start, end, red, green, blue, alpha);
}

void ScriptThread::CueCamera(Event *ev)
{
    float   switchTime;
    Entity *ent;

    if (ev->NumArgs() > 1) {
        switchTime = ev->GetFloat(2);
    } else {
        switchTime = 0;
    }

    ent = ev->GetEntity(1);
    if (ent) {
        SetCamera(ent, switchTime);
    } else {
        throw ScriptException("Camera named %s not found", ev->GetString(1).c_str());
    }
}

void ScriptThread::CuePlayer(Event *ev)
{
    float switchTime;

    if (ev->NumArgs() > 0) {
        switchTime = ev->GetFloat(1);
    } else {
        switchTime = 0;
    }

    SetCamera(NULL, switchTime);
}

void ScriptThread::FreezePlayer(Event *ev)
{
    level.playerfrozen = true;
}

void ScriptThread::ReleasePlayer(Event *ev)
{
    level.playerfrozen = false;
}

Listener *ScriptThread::SpawnInternal(Event *ev)
{
    SpawnArgs args;
    str       classname;
    Listener *l;

    if (ev->NumArgs() <= 0) {
        throw ScriptException("Usage: spawn entityname [keyname] [value]...");
    }

    classname = ev->GetString(1);

    if (getClassForID(classname) || getClass(classname)) {
        args.setArg("classname", classname);
    } else {
        if (!strstr(classname.c_str(), ".tik")) {
            classname.append(".tik");
        }

        args.setArg("model", classname);
    }

    for (int i = 2; i < ev->NumArgs(); i += 2) {
        args.setArg(ev->GetString(i), ev->GetString(i + 1));
    }

    if (!args.getClassDef()) {
        throw ScriptException("'%s' is not a valid entity name", classname.c_str());
    }

    const char *spawntarget = args.getArg("spawntarget");

    if (spawntarget) {
        SimpleEntity *target = G_FindTarget(NULL, spawntarget);

        if (!target) {
            throw ScriptException("Can't find targetname %s", spawntarget);
        }

        args.setArg("origin", va("%f %f %f", target->origin[0], target->origin[1], target->origin[2]));
        args.setArg("angle", va("%f", target->angles[1]));
    }

    level.spawnflags = 0;

    const char *s = args.getArg("spawnflags");
    if (s) {
        level.spawnflags = atoi(s);
    }

    level.m_bScriptSpawn = true;
    l                    = args.Spawn();
    level.m_bScriptSpawn = false;

    if (level.m_bRejectSpawn) {
        level.m_bRejectSpawn = false;
        throw ScriptException("Spawn command rejected for %s", classname.c_str());
    }

    return l;
}

void ScriptThread::Spawn(Event *ev)
{
    Listener *listener = SpawnInternal(ev);

    if (listener && checkInheritance(&Object::ClassInfo, listener->classinfo())) {
        throw ScriptException("You must specify an explicit classname for misc object tik models");
    }
}

void ScriptThread::SpawnReturn(Event *ev)
{
    Listener *listener = SpawnInternal(ev);

    ev->AddListener(listener);

    if (listener && checkInheritance(&Object::ClassInfo, listener->classinfo())) {
        throw ScriptException("You must specify an explicit classname for misc object tik models");
    }
}

void ScriptThread::Letterbox(Event *ev)
{
    level.m_letterbox_fraction   = 1.0f / 8.0f;
    level.m_letterbox_time       = ev->GetFloat(1);
    level.m_letterbox_time_start = ev->GetFloat(1);
    level.m_letterbox_dir        = letterbox_in;

    if (ev->NumArgs() > 1) {
        level.m_letterbox_fraction = ev->GetFloat(2);
    }
}

void ScriptThread::ClearLetterbox(Event *ev)
{
    level.m_letterbox_time = level.m_letterbox_time_start;
    level.m_letterbox_dir  = letterbox_out;
}

void ScriptThread::SetLightStyle(Event *ev)
{
    lightStyles.SetLightStyle(ev->GetInteger(1), ev->GetString(2));
}

void ScriptThread::FadeIn(Event *ev)
{
    level.m_fade_time_start = ev->GetFloat(1);
    level.m_fade_time       = ev->GetFloat(1);
    level.m_fade_color[0]   = ev->GetFloat(2);
    level.m_fade_color[1]   = ev->GetFloat(3);
    level.m_fade_color[2]   = ev->GetFloat(4);
    level.m_fade_alpha      = ev->GetFloat(5);
    level.m_fade_type       = fadein;
    level.m_fade_style      = alphablend;

    if (ev->NumArgs() > 5) {
        level.m_fade_style = (fadestyle_t)ev->GetInteger(6);
    }
}

void ScriptThread::ClearFade(Event *ev)
{
    level.m_fade_time = -1;
    level.m_fade_type = fadein;
}

void ScriptThread::FadeOut(Event *ev)
{
    level.m_fade_time_start = ev->GetFloat(1);
    level.m_fade_time       = ev->GetFloat(1);
    level.m_fade_color[0]   = ev->GetFloat(2);
    level.m_fade_color[1]   = ev->GetFloat(3);
    level.m_fade_color[2]   = ev->GetFloat(4);
    level.m_fade_alpha      = ev->GetFloat(5);
    level.m_fade_type       = fadeout;
    level.m_fade_style      = alphablend;

    if (ev->NumArgs() > 5) {
        level.m_fade_style = (fadestyle_t)ev->GetInteger(6);
    }
}

void ScriptThread::FadeSound(Event *ev)
{
    // params
    float   delaytime;
    float   min_vol;
    Player *player;

    // variables
    float time;

    // code

    delaytime = ev->GetFloat(1);

    min_vol = ev->GetFloat(2);

    if (ev->NumArgs() > 2) {
        player = (Player *)ev->GetEntity(3);
    } else {
        player = NULL;
    }

    time = delaytime * 1000.0f;

    gi.SendServerCommand(player != nullptr ? player->edict - g_entities : 0, "fadesound2 %0.2f %f", time, min_vol);
}

void ScriptThread::MusicEvent(Event *ev)
{
    const char *current;
    const char *fallback;

    current  = NULL;
    fallback = NULL;
    current  = ev->GetString(1);

    if (ev->NumArgs() > 1) {
        fallback = ev->GetString(2);
    }

    ChangeMusic(current, fallback, false);
}

void ScriptThread::MusicVolumeEvent(Event *ev)
{
    float volume;
    float fade_time;

    volume    = ev->GetFloat(1);
    fade_time = ev->GetFloat(2);

    ChangeMusicVolume(volume, fade_time);
}

void ScriptThread::RestoreMusicVolumeEvent(Event *ev)
{
    float fade_time;

    fade_time = ev->GetFloat(1);

    RestoreMusicVolume(fade_time);
}

void ScriptThread::ForceMusicEvent(Event *ev)
{
    const char *current;
    const char *fallback;

    current  = NULL;
    fallback = NULL;
    current  = ev->GetString(1);

    if (ev->NumArgs() > 1) {
        fallback = ev->GetString(2);
    }

    ChangeMusic(current, fallback, true);
}

void ScriptThread::SoundtrackEvent(Event *ev)
{
    ChangeSoundtrack(ev->GetString(1));
}

void ScriptThread::RestoreSoundtrackEvent(Event *ev)
{
    RestoreSoundtrack();
}

void ScriptThread::SetCinematic(Event *ev)
{
    G_StartCinematic();
}

void ScriptThread::SetNonCinematic(Event *ev)
{
    G_StopCinematic();
}

void ScriptThread::SetAllAIOff(Event *ev)
{
    level.ai_on = qfalse;
}

void ScriptThread::SetAllAIOn(Event *ev)
{
    level.ai_on = qtrue;
}

void ScriptThread::CenterPrint(Event *ev)
{
    int        j;
    gentity_t *other;

    for (j = 0; j < game.maxclients; j++) {
        other = &g_entities[j];
        if (other->inuse && other->client) {
            gi.centerprintf(other, ev->GetString(1));
        }
    }
}

void ScriptThread::LocationPrint(Event *ev)
{
    int        j;
    gentity_t *other;
    int        x, y;

    x = ev->GetInteger(1);
    y = ev->GetInteger(2);

    for (j = 0; j < game.maxclients; j++) {
        other = &g_entities[j];
        if (other->inuse && other->client) {
            gi.locationprintf(other, x, y, ev->GetString(3));
        }
    }
}

void ScriptThread::ServerEvent(Event *ev)
{
    Event *event = new Event(ev->GetString(1));

    for (int i = 2; i <= ev->NumArgs(); i++) {
        event->AddValue(ev->GetValue(i));
    }

    ProcessScriptEvent(event);
}

void ScriptThread::StuffCommand(Event *ev)
{
    gi.SendConsoleCommand(va("%s\n", ev->GetString(1).c_str()));
}

void ScriptThread::KillEnt(Event *ev)
{
    int     num;
    Entity *ent;

    if (ev->NumArgs() != 1) {
        throw ScriptException("No args passed in");
    }

    num = ev->GetInteger(1);
    if ((num < 0) || (num >= globals.max_entities)) {
        throw ScriptException("Value out of range.  Possible values range from 0 to %d.\n", globals.max_entities);
    }

    ent = G_GetEntity(num);
    ent->Damage(world, world, ent->max_health + 25, vec_zero, vec_zero, vec_zero, 0, 0, 0);
}

void ScriptThread::GetEntByEntnum(Event *ev)
{
    int     entnum = -1;
    Entity *ent;

    entnum = ev->GetInteger(1);

    if (entnum < 0 || entnum > globals.max_entities) {
        throw ScriptException("Entity number %d out of scope!\n", entnum);
    }

    ent = G_GetEntity(entnum);
    ev->AddEntity(ent);
}

void ScriptThread::RemoveEnt(Event *ev)
{
    int     num;
    Entity *ent;

    if (ev->NumArgs() != 1) {
        throw ScriptException("No args passed in");
    }

    num = ev->GetInteger(1);
    if ((num < 0) || (num >= globals.max_entities)) {
        throw ScriptException("Value out of range.  Possible values range from 0 to %d.\n", globals.max_entities);
    }

    ent = G_GetEntity(num);
    ent->PostEvent(Event(EV_Remove), 0);
}

void ScriptThread::KillClass(Event *ev)
{
    int        except;
    str        classname;
    gentity_t *from;
    Entity    *ent;

    if (ev->NumArgs() < 1) {
        throw ScriptException("No args passed in");
    }

    classname = ev->GetString(1);

    except = 0;
    if (ev->NumArgs() == 2) {
        except = ev->GetInteger(1);
    }

    for (from = &g_entities[game.maxclients]; from < &g_entities[globals.num_entities]; from++) {
        if (!from->inuse) {
            continue;
        }

        assert(from->entity);

        ent = from->entity;

        if (ent->entnum == except) {
            continue;
        }

        if (ent->inheritsFrom(classname.c_str())) {
            ent->Damage(world, world, ent->max_health + 25, vec_zero, vec_zero, vec_zero, 0, 0, 0);
        }
    }
}

void ScriptThread::RemoveClass(Event *ev)
{
    int        except;
    str        classname;
    gentity_t *from;
    Entity    *ent;

    if (ev->NumArgs() < 1) {
        throw ScriptException("No args passed in");
    }

    classname = ev->GetString(1);

    except = 0;
    if (ev->NumArgs() == 2) {
        except = ev->GetInteger(1);
    }

    for (from = &g_entities[game.maxclients]; from < &g_entities[globals.num_entities]; from++) {
        if (!from->inuse) {
            continue;
        }

        assert(from->entity);

        ent = from->entity;

        if (ent->entnum == except) {
            continue;
        }

        if (ent->inheritsFrom(classname.c_str())) {
            ent->PostEvent(Event(EV_Remove), 0);
        }
    }
}

void ScriptThread::CameraCommand(Event *ev)
{
    Event *e;
    str    cmd;
    int    i;
    int    n;

    if (!ev->NumArgs()) {
        throw ScriptException("Usage: cam [command] [arg 1]...[arg n]");
    }

    cmd = ev->GetString(1);
    if (Event::Exists(cmd)) {
        e = new ConsoleEvent(cmd);

        n = ev->NumArgs();
        for (i = 2; i <= n; i++) {
            e->AddToken(ev->GetToken(i));
        }

        CameraMan.ProcessEvent(e);
    } else {
        throw ScriptException("Unknown camera command '%s'.\n", cmd.c_str());
    }
}

void ScriptThread::MissionFailed(Event *ev)
{
    G_MissionFailed();
}

void ScriptThread::Archive(Archiver& arc) {}

void ScriptThread::ArchiveInternal(Archiver& arc)
{
    Listener::Archive(arc);

    arc.ArchiveObjectPosition(this);
    m_ScriptVM->Archive(arc);
}

void ScriptThread::Vector_Add(Event *ev)
{
    ev->AddVector(ev->GetVector(1) + ev->GetVector(2));
}

void ScriptThread::Vector_Subtract(Event *ev)
{
    ev->AddVector(ev->GetVector(1) - ev->GetVector(2));
}

void ScriptThread::Vector_Length(Event *ev)
{
    ev->AddFloat(ev->GetVector(1).length());
}

void ScriptThread::Vector_Normalize(Event *ev)
{
    Vector vector = ev->GetVector(1);

    vector.normalize();

    ev->AddVector(vector);
}

void ScriptThread::Vector_Scale(Event *ev)
{
    Vector vector = ev->GetVector(1);

    vector *= ev->GetFloat(2);

    ev->AddVector(vector);
}

void ScriptThread::Vector_DotProduct(Event *ev)
{
    Vector vector1 = ev->GetVector(1), vector2 = ev->GetVector(2);

    ev->AddFloat(Vector::Dot(vector1, vector2));
}

void ScriptThread::Vector_CrossProduct(Event *ev)
{
    ev->AddVector(Vector::Cross(ev->GetVector(1), ev->GetVector(2)));
}

void ScriptThread::Vector_ToAngles(Event *ev)
{
    ev->AddVector(ev->GetVector(1).toAngles());
}

void ScriptThread::Angles_ToForward(Event *ev)
{
    Vector fwd;

    ev->GetVector(1).AngleVectors(&fwd);
    ev->AddVector(fwd);
}

void ScriptThread::Angles_ToLeft(Event *ev)
{
    Vector left;

    ev->GetVector(1).AngleVectors(NULL, &left);
    ev->AddVector(left);
}

void ScriptThread::Angles_ToUp(Event *ev)
{
    Vector up;

    ev->GetVector(1).AngleVectors(NULL, NULL, &up);
    ev->AddVector(up);
}

void ScriptThread::Angles_PointAt(Event *ev)
{
    Entity *pParent, *pEnt, *pTarget;
    Vector  vDelta, vVec, vAngles;

    pParent = ev->GetEntity(1);
    pEnt    = ev->GetEntity(2);
    pTarget = ev->GetEntity(3);

    if (pParent) {
        vDelta  = pEnt->centroid - pTarget->centroid;
        vVec[0] = DotProduct(vDelta, pParent->orientation[0]);
        vVec[1] = DotProduct(vDelta, pParent->orientation[1]);
        vVec[2] = DotProduct(vDelta, pParent->orientation[2]);
    } else {
        vVec = pEnt->centroid - pTarget->centroid;
    }

    VectorNormalize(vVec);
    vectoangles(vVec, vAngles);

    ev->AddVector(vAngles);
}

void ScriptThread::EventIsAlive(Event *ev)
{
    Entity *ent = ev->GetEntity(1);

    if (ent) {
        ev->AddInteger(!ent->IsDead());
    } else {
        ev->AddInteger(0);
    }
}

void ScriptThread::EventPopmenu(Event *ev)
{
    gentity_t *ent;
    int        i;
    int        index;

    if (game.maxclients <= 0) {
        return;
    }

    index = ev->GetInteger(1);

    for (i = 0, ent = g_entities; i < game.maxclients; i++, ent++) {
        if (!ent->inuse || !ent->client) {
            continue;
        }

        gi.Popmenu(i, index);
    }
}

void Showmenu(const str& name, qboolean bForce)
{
    gentity_t *ent;
    int        i;

    if (game.maxclients <= 0) {
        return;
    }

    for (i = 0, ent = g_entities; i < game.maxclients; i++, ent++) {
        if (!ent->inuse || !ent->client) {
            continue;
        }

        gi.Showmenu(i, name.c_str(), bForce);
    }
}

void ScriptThread::EventShowmenu(Event *ev)
{
    str name = ev->GetString(1);

    if (ev->NumArgs() > 1) {
        Showmenu(name, ev->GetBoolean(2));
    } else {
        Showmenu(name, false);
    }

    Director.AddMenu(name);
}

void Hidemenu(const str& name, qboolean bForce)
{
    gentity_t *ent;
    int        i;

    if (game.maxclients <= 0) {
        return;
    }

    for (i = 0, ent = g_entities; i < game.maxclients; i++, ent++) {
        if (!ent->inuse || !ent->client) {
            continue;
        }

        gi.Hidemenu(i, name.c_str(), bForce);
    }
}

void ScriptThread::EventHidemenu(Event *ev)
{
    str name = ev->GetString(1);

    Director.RemoveMenu(name);

    if (ev->NumArgs() > 1) {
        Hidemenu(name, ev->GetBoolean(2));
    } else {
        Hidemenu(name, false);
    }
}

void ScriptThread::EventPlayMovie(Event *ev)
{
    gi.SendConsoleCommand(va("cinematic %s", ev->GetString(1).c_str()));
}

void ScriptThread::EventPushmenu(Event *ev)
{
    gentity_t *ent;
    int        i;
    str        name;

    if (game.maxclients <= 0) {
        return;
    }

    name = ev->GetString(1);

    for (i = 0, ent = g_entities; i < game.maxclients; i++, ent++) {
        if (!ent->inuse || !ent->client) {
            continue;
        }

        gi.Pushmenu(i, name.c_str());
    }
}

void ScriptThread::EventHideMouse(Event *ev)
{
    gentity_t *ent;
    int        i;
    str        name;

    if (game.maxclients <= 0) {
        return;
    }

    name = ev->GetString(1);

    for (i = 0, ent = g_entities; i < game.maxclients; i++, ent++) {
        if (!ent->inuse || !ent->client) {
            continue;
        }

        gi.HideMouseCursor(i);
    }
}

void ScriptThread::EventCreateListener(Event *ev)
{
    ev->AddListener(new Listener);
}

void ScriptThread::EventTrace(Event *ev)
{
    int     content_mask = MASK_LINE;
    Vector  start;
    Vector  mins;
    Vector  maxs;
    Vector  end;
    trace_t trace;

    mins = vec_zero;
    maxs = vec_zero;

    switch (ev->NumArgs()) {
    case 5:
        maxs = ev->GetVector(5);
    case 4:
        mins = ev->GetVector(4);
    case 3:
        if (ev->GetInteger(3)) {
            content_mask = MASK_SOLID;
        }
    case 2:
        end = ev->GetVector(2);
    case 1:
        start = ev->GetVector(1);
        break;
    default:
        throw ScriptException("Wrong number of arguments for trace.");
    }

    // call trace
    trace = G_Trace(start, mins, maxs, end, NULL, content_mask, false, "ScriptThread::EventTrace");

    ev->AddVector(trace.endpos);
}

void ScriptThread::EventSightTrace(Event *ev)
{
    int    content_mask = MASK_LINE;
    Vector start;
    Vector mins;
    Vector maxs;
    Vector end;

    mins = vec_zero;
    maxs = vec_zero;

    switch (ev->NumArgs()) {
    case 5:
        maxs = ev->GetVector(5);
    case 4:
        mins = ev->GetVector(4);
    case 3:
        if (ev->GetInteger(3)) {
            content_mask = MASK_SOLID;
        }
    case 2:
        end = ev->GetVector(2);
    case 1:
        start = ev->GetVector(1);
        break;
    default:
        throw ScriptException("Wrong number of arguments for sighttrace.");
    }

    // call trace
    ev->AddInteger(G_SightTrace(
        start,
        mins,
        maxs,
        end,
        (gentity_t *)NULL,
        (gentity_t *)NULL,
        content_mask,
        false,
        "ScriptThread::EventSightTrace"
    ));
}

void ScriptThread::EventPrint3D(Event *ev)
{
    Vector origin;
    float  scale;
    str    string;

    origin = ev->GetVector(1);
    scale  = ev->GetFloat(2);
    string = ev->GetString(3);

    G_DebugString(origin, scale, 1.0f, 1.0f, 1.0f, string);
}

void ScriptThread::EventHudDrawShader(Event *ev)
{
    int index = -1;
    str shadername;

    index = ev->GetInteger(1);

    if (index < 0 && index > 255) {
        throw ScriptException("Wrong index for huddraw_shader!\n");
    }

    shadername = ev->GetString(2);

    HudDrawShader(index, shadername);
}

void ScriptThread::EventHudDrawAlign(Event *ev)
{
    int index        = -1;
    int h_alignement = -1;
    int v_alignement = -1;
    str h_align;
    str v_align;

    index = ev->GetInteger(1);

    if (index < 0 && index > 255) {
        throw ScriptException("Wrong index for huddraw_align!\n");
    }

    h_align = ev->GetString(2);

    if (!h_align) {
        throw ScriptException("h_align is NULL for huddraw_align!\n");
    }

    v_align = ev->GetString(3);

    if (!v_align) {
        throw ScriptException("v_align is NULL for huddraw_align!\n");
    }

    if (h_align == "left") {
        h_alignement = 0;
    } else if (h_align == "center") {
        h_alignement = 1;
    } else if (h_align == "right") {
        h_alignement = 2;
    } else {
        throw ScriptException("Wrong alignement h_align string for huddraw_align!\n");
    }

    if (v_align == "top") {
        v_alignement = 0;
    } else if (v_align == "center") {
        v_alignement = 1;
    } else if (v_align == "bottom") {
        v_alignement = 2;
    } else {
        throw ScriptException("Wrong alignement v_align string for huddraw_align!\n");
    }

    HudDrawAlign(index, h_alignement, v_alignement);
}

void ScriptThread::EventHudDrawRect(Event *ev)
{
    int index  = -1;
    int x      = 0;
    int y      = 0;
    int width  = 0;
    int height = 0;

    index = ev->GetInteger(1);

    if (index < 0 && index > 255) {
        throw ScriptException("Wrong index for huddraw_rect!\n");
    }

    x      = ev->GetInteger(2);
    y      = ev->GetInteger(3);
    width  = ev->GetInteger(4);
    height = ev->GetInteger(5);

    HudDrawRect(index, x, y, width, height);
}

void ScriptThread::EventHudDrawVirtualSize(Event *ev)
{
    int index = -1;
    int virt  = -1;

    index = ev->GetInteger(1);

    if (index < 0 && index > 255) {
        throw ScriptException("Wrong index for huddraw_virtualsize!\n");
    }

    virt = ev->GetInteger(2);

    if (virt != 0) {
        virt = 1;
    }

    HudDrawVirtualSize(index, virt);
}

void ScriptThread::EventHudDrawColor(Event *ev)
{
    int    numArgs = -1;
    int    index   = -1;
    Vector color;

    numArgs = ev->NumArgs();

    index = ev->GetInteger(1);

    if (index < 0 && index > 255) {
        throw ScriptException("Wrong index for ihuddraw_color!\n");
    }

    color[0] = ev->GetFloat(2); // red
    color[1] = ev->GetFloat(3); // green
    color[2] = ev->GetFloat(4); // blue

    Vector::Clamp(color, vec_zero, Vector(1, 1, 1));

    HudDrawColor(index, color);
}

void ScriptThread::EventHudDrawAlpha(Event *ev)
{
    int   index = -1;
    float alpha = 0.0f;

    index = ev->GetInteger(1);

    if (index < 0 && index > 255) {
        throw ScriptException("Wrong index for ihuddraw_alpha!\n");
    }

    alpha = ev->GetFloat(2);
    Q_clamp(alpha, 0, 1);

    HudDrawAlpha(index, alpha);
}

void ScriptThread::EventHudDrawString(Event *ev)
{
    int index = -1;
    str string;

    index = ev->GetInteger(1);

    if (index < 0 && index > 255) {
        throw ScriptException("Wrong index for ihuddraw_string!\n");
    }

    string = ev->GetString(2);

    HudDrawString(index, string);
}

void ScriptThread::EventHudDrawFont(Event *ev)
{
    int index = -1;
    str fontname;

    index = ev->GetInteger(1);

    if (index < 0 && index > 255) {
        throw ScriptException("Wrong index for ihuddraw_font!\n");
    }

    fontname = ev->GetString(2);

    HudDrawFont(index, fontname);
}

void ScriptThread::EventError(Event *ev)
{
    if (ev->NumArgs() > 1) {
        ScriptException::next_abort = 0;
    } else {
        ScriptException::next_abort = ev->GetInteger(2);
        if (ScriptException::next_abort < 0) {
            ScriptException::next_abort = 0;
        }
    }

    throw ScriptException(ev->GetString(1));
}

void ScriptThread::EventDebugInt3(Event *ev)
{
#ifdef _MSC_VER
    __debugbreak();
#endif
    // FIXME: break on other platforms?
}

void ScriptThread::EventTimeout(Event *ev)
{
    Director.maxTime = ev->GetFloat(1) * 1000.0f + 0.5f;
}

void ScriptThread::EventLandmineDamage(Event *ev)
{
    Entity *landmine;
    float   damage;
    float   radius;
    Entity *owner;

    landmine = ev->GetEntity(1);
    damage   = ev->GetFloat(2);
    radius   = ev->GetFloat(3);

    if (!landmine || !landmine->edict) {
        ScriptError("Landmine doesn't exist, yet explodes!.\n");
    }

    owner = G_GetEntity(landmine->edict->r.ownerNum);

    RadiusDamage(landmine->origin, landmine, owner, damage, NULL, MOD_LANDMINE, radius);
}

void ScriptThread::EventRadiusDamage(Event *ev)
{
    Vector origin;
    float  damage;
    float  radius;
    int    constant_damage;

    origin = ev->GetVector(1);
    damage = ev->GetFloat(2);
    radius = ev->GetFloat(3);

    if (ev->NumArgs() > 3) {
        constant_damage = ev->GetInteger(4);
    } else {
        constant_damage = 0;
    }

    RadiusDamage(origin, world, world, damage, NULL, MOD_EXPLOSION, radius, 0, constant_damage);
}

void ScriptThread::EventBspTransition(Event *ev)
{
    str map = ev->GetString(1);

    if (level.intermissiontime == 0.0f) {
        G_BeginIntermission(map, TRANS_BSP);
    }
}

void ScriptThread::EventLevelTransition(Event *ev)
{
    str map = ev->GetString(1);

    if (level.intermissiontime == 0.0f) {
        G_BeginIntermission(map, TRANS_LEVEL);
    }
}

void ScriptThread::EventMissionTransition(Event *ev)
{
    str map = ev->GetString(1);

    if (level.intermissiontime == 0.0f) {
        G_BeginIntermission(map, TRANS_MISSION);
    }
}

void ScriptThread::EventStopTeamRespawn(Event *ev)
{
    const_str team = ev->GetConstString(1);
    switch (team) {
    case STRING_GERMAN:
        dmManager.StopTeamRespawn(CONTROLLER_AXIS);
        break;
    case STRING_AMERICAN:
        dmManager.StopTeamRespawn(CONTROLLER_ALLIES);
        break;
    case STRING_BOTH:
        dmManager.StopTeamRespawn(CONTROLLER_AXIS);
        dmManager.StopTeamRespawn(CONTROLLER_ALLIES);
        break;
    default:
        ScriptError("stopteamrespawn must belong to 'american' or 'german' or 'both'");
        break;
    }
}

void ScriptThread::EventTeamWin(Event *ev)
{
    const_str team;
    int       teamnum;

    if (g_gametype->integer < GT_OBJECTIVE) {
        throw ScriptException("'teamwin' only valid for objective-based DM games");
    }

    team = ev->GetConstString(1);
    if (team == STRING_ALLIES) {
        teamnum = TEAM_ALLIES;
    } else if (team == STRING_AXIS) {
        teamnum = TEAM_AXIS;
    } else if (team == STRING_DISGUISE_NONE) {
        teamnum = TEAM_NONE;
    } else {
        throw ScriptException("'teamwin' must be called with 'axis', 'allies' or 'none' as its argument");
    }

    dmManager.TeamWin(teamnum);
}

void ScriptThread::EventEarthquake(Event *ev)
{
    earthquake_t e;

    e.duration = (int)(ev->GetFloat(1) * 1000.0f + 0.5f);
    if (e.duration <= 0) {
        return;
    }

    e.magnitude   = ev->GetFloat(2);
    e.no_rampup   = ev->GetBoolean(3);
    e.no_rampdown = ev->GetBoolean(4);

    e.starttime = level.inttime;
    e.endtime   = level.inttime + e.duration;

    e.m_Thread = this;

    level.AddEarthquake(&e);
}

void ScriptThread::EventAIsCloserThanBToC(Event *ev)
{
    Vector vA, vB, vC;

    vA = ev->GetVector(1);
    vB = ev->GetVector(2);
    vC = ev->GetVector(3);

    ev->AddInteger((vA - vC).lengthSquared() < (vB - vC).lengthSquared());
}

void ScriptThread::EventPointsWithinDist(Event *ev)
{
    Vector delta;
    float  dist = ev->GetFloat(3);

    delta = ev->GetVector(1) - ev->GetVector(2);

    // check squared distance
    ev->AddInteger(((delta * delta) < (dist * dist)));
}

bool ScriptThread::CanScriptTracePrint(void)
{
    return m_ScriptVM->CanScriptTracePrint();
}

void ScriptThread::EventGetBoundKey1(Event *ev)
{
    int         iKey1;
    int         iKey2;
    const char *pszKeyName;
    str         sCommand = ev->GetString(1);

    gi.Key_GetKeysForCommand(sCommand, &iKey1, &iKey2);
    if (iKey1) {
        pszKeyName = gi.Key_KeynumToBindString(iKey1);
    } else {
        pszKeyName = "";
    }

    ev->AddString(pszKeyName);
}

void ScriptThread::EventGetBoundKey2(Event *ev)
{
    int         iKey1;
    int         iKey2;
    const char *pszKeyName;
    str         sCommand = ev->GetString(1);

    gi.Key_GetKeysForCommand(sCommand, &iKey1, &iKey2);
    if (iKey2) {
        pszKeyName = gi.Key_KeynumToBindString(iKey2);
    } else {
        pszKeyName = "";
    }

    ev->AddString(pszKeyName);
}

void ScriptThread::EventLocConvertString(Event *ev)
{
    ev->AddString(gi.LV_ConvertString(ev->GetString(1)));
}

void ScriptThread::AddObjective(int index, int status, str text, Vector location)
{
    static int last_time;
    int        flags;
    char       szSend[2048];
    char      *sTmp;

    flags = OBJ_FLAG_NONE;
    sTmp  = gi.getConfigstring(CS_OBJECTIVES + index);

    switch (status) {
    case OBJ_STATUS_HIDDEN:
        flags = OBJ_FLAG_HIDDEN;
        break;
    case OBJ_STATUS_CURRENT:
        sTmp = Info_ValueForKey(sTmp, "flags");
        if (!(atoi(sTmp) & OBJ_FLAG_CURRENT)) {
            if (last_time != level.inttime) {
                gi.Printf("An objective has been added!\n");
                last_time = level.inttime;
            }
        }
        flags = OBJ_FLAG_CURRENT;
        break;
    case OBJ_STATUS_COMPLETED:
        if (last_time != level.inttime) {
            gi.Printf("An objective has been completed!\n");
            last_time = level.inttime;
        }
        if (g_gametype->integer == GT_SINGLE_PLAYER) {
            if (g_entities->entity->IsSubclassOfPlayer()) {
                ((Player *)g_entities->entity)->m_iObjectivesCompleted++;
            }
        }
        flags = OBJ_FLAG_COMPLETED;
        break;
    }

    szSend[0] = 0;

    Info_SetValueForKey(szSend, "flags", va("%i", flags));
    Info_SetValueForKey(szSend, "text", text.c_str());
    Info_SetValueForKey(szSend, "loc", va("%f %f %f", location[0], location[1], location[2]));

    gi.setConfigstring(CS_OBJECTIVES + index, szSend);
}

void ScriptThread::SetCurrentObjective(int iObjective, int iTeam)
{
    gi.setConfigstring(CS_CURRENT_OBJECTIVE, va("%i", iObjective));

    if (iObjective == -1) {
        level.m_vObjectiveLocation = vec_zero;
        level.m_vAlliedObjectiveLocation = vec_zero;
        level.m_vAxisObjectiveLocation = vec_zero;
    } else {
        const char *s   = gi.getConfigstring(CS_OBJECTIVES + iObjective);
        const char *loc = Info_ValueForKey(s, "loc");
        Vector v;

        sscanf(
            loc,
            "%f %f %f",
            &v[0],
            &v[1],
            &v[2]
        );

        //
        // Since 2.0, allow objective location per team
        //
        switch (iTeam)
        {
        case TEAM_NONE:
        default:
            level.m_vObjectiveLocation = v;
            break;
        case TEAM_ALLIES:
            level.m_vAlliedObjectiveLocation = v;
            break;
        case TEAM_AXIS:
            level.m_vAxisObjectiveLocation = v;
            break;
        }
    }
}

void ScriptThread::EventAddObjective(Event *ev)
{
    int    index;
    int    status;
    str    text;
    Vector location;

    index  = ev->GetInteger(1) - 1;
    status = ev->GetInteger(2);

    if (index >= MAX_OBJECTIVES) {
        throw ScriptException("Index Out Of Range");
    }

    if (status > 3) {
        throw ScriptException("Invalid Status");
    }

    if (ev->IsNilAt(3)) {
        text = Info_ValueForKey(gi.getConfigstring(index + CS_OBJECTIVES), "text");
    } else {
        text = ev->GetString(3);
    }

    if (ev->IsNilAt(4)) {
        sscanf(
            Info_ValueForKey(gi.getConfigstring(index + CS_OBJECTIVES), "loc"),
            "%f %f %f",
            &location.x,
            &location.y,
            &location.z
        );
    } else {
        location = ev->GetVector(4);
    }

    AddObjective(index, status, text, location);
}

void ScriptThread::EventSetScoreboardToggle(Event *ev)
{
    int       value;
    const_str team;

    if (ev->NumArgs() != 2) {
        ScriptError("wrong number of arguments: setscoreboardtoggle (axis|allies) (0|1)");
    }

    value = ev->GetInteger(2);
    team  = ev->GetConstString(1);
    switch (team) {
    case STRING_AXIS:
        gi.cvar_set("scoreboard_toggle2", va("%i", value));
        break;
    case STRING_ALLIES:
        gi.cvar_set("scoreboard_toggle1", va("%i", value));
        break;
    default:
        ScriptError("1st argument can only be \"axis\" or \"allies\"");
        break;
    }
}

void ScriptThread::EventSetCurrentObjective(Event *ev)
{
    int iObjective = ev->GetInteger(1);
    int iTeam = 0;

    if (iObjective > MAX_OBJECTIVES) {
        ScriptError("Index Out Of Range");
    }

    if (ev->NumArgs() >= 2)
    {
        const_str teamStr = ev->GetConstString(2);
        if (teamStr == STRING_ALLIES) {
            iTeam = TEAM_ALLIES;
        } else if (teamStr == STRING_AXIS) {
            iTeam = TEAM_AXIS;
        } else {
            ScriptError("Option 2nd argument can only be \"allies\" or \"axis\"");
        }
    }

    SetCurrentObjective(iObjective - 1, iTeam);
}

void ScriptThread::SetObjectiveLocation(Vector vLocation)
{
    level.m_vObjectiveLocation = vLocation;
}

void ScriptThread::SetObjectiveLocation(Event *ev)
{
    SetObjectiveLocation(ev->GetVector(1));
}

void ScriptThread::ClearObjectiveLocation(void)
{
    level.m_vObjectiveLocation = vec_zero;
}

void ScriptThread::ClearObjectiveLocation(Event *ev)
{
    ClearObjectiveLocation();
}

void ScriptThread::EventDrawHud(Event *ev)
{
    int        i;
    gentity_t *ent;

    // TRIVIA: in mohaa, drawhud worked only for the first player
    for (i = 0, ent = g_entities; i < game.maxclients; i++, ent++) {
        if (!ent->inuse || !ent->entity || !ent->client) {
            continue;
        }

        if (ev->GetBoolean(1)) {
            ent->client->ps.pm_flags &= ~PMF_NO_HUD;
        } else {
            ent->client->ps.pm_flags |= PMF_NO_HUD;
        }
    }
}

void ScriptThread::CancelWaiting(Event *ev)
{
    CancelWaitingAll();
}

void ScriptThread::DelayExecute(Event& ev)
{
    DelayExecute(&ev);
}

void ScriptThread::DelayExecute(Event *ev)
{
    assert(m_ScriptVM);

    if (ev) {
        ScriptVariable returnValue;

        m_ScriptVM->SetFastData(ev->data, ev->dataSize);

        returnValue.newPointer();
        m_ScriptVM->m_ReturnValue = returnValue;
        ev->AddValue(returnValue);
    }

    Director.AddTiming(this, 0);
}

int ScriptThread::GetThreadState(void)
{
    return m_ScriptVM->ThreadState();
}

void ScriptThread::ServerStufftext(Event *ev)
{
    gi.SendConsoleCommand(ev->GetString(1));
}

void ScriptThread::RemoveArchivedClass(Event *ev)
{
    str classname;
    int except_entity_number = -1;

    classname = ev->GetString(1);

    if (ev->NumArgs() > 1) {
        except_entity_number = ev->GetInteger(1);
    }

    for (int i = level.m_SimpleArchivedEntities.NumObjects(); i > 0; i--) {
        SimpleArchivedEntity *m_SimpleArchivedEntity = level.m_SimpleArchivedEntities.ObjectAt(i);

        if (m_SimpleArchivedEntity == NULL) {
            continue;
        }

        // If found, remove the simple archived entity
        if (m_SimpleArchivedEntity->inheritsFrom(classname)) {
            m_SimpleArchivedEntity->PostEvent(EV_Remove, EV_REMOVE);
        }
    }

    L_ProcessPendingEvents();
}

void ScriptThread::SetTimer(Event *ev)
{
    int    interval = -1;
    void  *scr_var  = NULL;
    int    i        = 0;
    Event *event;

    if (ev->NumArgs() != 2) {
        throw ScriptException("Wrong arguments count for settimer!\n");
    }

    interval = ev->GetInteger(1);

    if (interval <= 0) {
        ev->AddInteger(1);
        return;
    }

    event = new Event(EV_Listener_ExecuteScript);
    event->AddValue(ev->GetValue(2));

    PostEvent(event, (float)interval / 1000.0f);
}

void ScriptThread::EventRegisterCommand(Event *ev)
{
    ScriptThreadLabel scriptLabel;

    scriptLabel.SetThread(ev->GetValue(2));

    m_scriptCmds.addKeyValue(ev->GetString(1)) = scriptLabel;
}

void ScriptThread::CreateHUD(Event *ev)
{
    int clientNum = -1;

    if (ev->NumArgs() > 0) {
        Player *player = (Player *)ev->GetEntity(1);
        if (!player || !player->IsSubclassOfPlayer()) {
            throw ScriptException("Invalid player entity!\n");
        }

        clientNum = player->client->ps.clientNum;
    }

    Hud *hud = new Hud(clientNum);

    ev->AddListener(hud);
}

void ScriptThread::Earthquake(Event *ev)
{
    float    duration    = ev->GetFloat(1);
    float    magnitude   = ev->GetFloat(2);
    qboolean no_rampup   = ev->GetBoolean(3);
    qboolean no_rampdown = ev->GetBoolean(4);

    if (ev->NumArgs() > 4) {
        Vector location = ev->GetVector(5);
        float  radius   = 1.0f;

        if (ev->NumArgs() > 5) {
            radius = ev->GetFloat(6);
        }

        gi.SendServerCommand(
            -1,
            "eq %f %f %d %d %f %f %f %f",
            duration,
            magnitude,
            no_rampup,
            no_rampdown,
            location[0],
            location[1],
            location[2],
            radius
        );
    } else {
        gi.SendServerCommand(-1, "eq %f %f %d %d", duration, magnitude, no_rampup, no_rampdown);
    }
}

void ScriptThread::EventClearObjectives(Event *ev) {}

void ScriptThread::SendObjective() {}

void ScriptThread::SendObjectives() {}

void ScriptThread::ClearObjectives() {}

void ScriptThread::WaitSkip(Event *ev) {}

void ScriptThread::ThreadSkip(Event *ev) {}

void ScriptThread::RestoreSound(Event *ev)
{
    // params
    float   delaytime;
    float   max_vol;
    Player *player;

    // variables
    float time;

    // code

    delaytime = ev->GetFloat(1);

    if (ev->NumArgs() > 1) {
        max_vol = ev->GetFloat(2);
    } else {
        max_vol = 1.0f;
    }

    if (ev->NumArgs() > 2) {
        player = (Player *)ev->GetEntity(3);
    } else {
        player = NULL;
    }

    time = delaytime * 1000.0f;

    gi.SendServerCommand(player != NULL ? player->edict - g_entities : NULL, "restoresound %0.2f %f", time, max_vol);
}

void ScriptThread::EventHudDraw3d(Event *ev)
{
    int      index;
    float   *tmp;
    vec3_t   vector;
    int      ent_num;
    qboolean bAlwaysShow, bDepth;

    index = ev->GetInteger(1);

    if (index < 0 && index > 255) {
        throw ScriptException("Wrong index for huddraw_3d!\n");
    }

    tmp = ev->GetVector(2);
    memcpy(&vector, tmp, sizeof(vec3_t));

    ent_num     = ev->GetInteger(3);
    bAlwaysShow = ev->GetInteger(4);
    bDepth      = ev->GetInteger(5);

    HudDraw3d(index, vector, ent_num, bAlwaysShow, bDepth);
}

void ScriptThread::EventHudDrawTimer(Event *ev)
{
    int   index;
    float duration;
    float fade_out_time;

    index = ev->GetInteger(1);

    if (index < 0 && index > 255) {
        throw ScriptException("Wrong index for huddraw_timer!\n");
    }

    duration      = ev->GetFloat(2);
    fade_out_time = ev->GetFloat(3);

    HudDrawTimer(index, duration, fade_out_time);
}

void ScriptThread::CanSwitchTeams(Event *ev)
{
    qboolean bAllow = ev->GetBoolean(1);

    disable_team_change = !bAllow;

    if (ev->NumArgs() > 1) {
        qboolean bAllow2 = ev->GetBoolean(2);

        disable_team_spectate = !bAllow2;
    }
}

void ScriptThread::GetPlayerNetname(Event *ev)
{
    Entity *ent = NULL;

    ent = (Entity *)ev->GetEntity(1);

    if (ent == NULL) {
        ev->AddString("");

        return;
    } else if (ent->client == NULL) {
        ev->AddString("");
        return;
    }

    ev->AddString(ent->client->pers.netname);
}

void ScriptThread::GetPlayerIP(Event *ev)
{
    Entity *ent = NULL;
    char   *ip  = NULL;
    char    ip_buff[65];
    ent = (Entity *)ev->GetEntity(1);

    if (ent == NULL) {
        ev->AddString("NIL");

        return;
    } else if (ent->client == NULL) {
        ev->AddString("NIL");
        return;
    }

    ip = ent->client->pers.ip;

    sprintf(ip_buff, "%s:%i", ip, ent->client->pers.port);

    ev->AddString(ip_buff);
}

void ScriptThread::GetPlayerPing(Event *ev)
{
    Entity *ent  = NULL;
    int     ping = -1;

    ent = (Entity *)ev->GetEntity(1);

    if (ent == NULL) {
        throw ScriptException("Player entity is NULL for getping!\n");
    } else if (ent->client == NULL) {
        throw ScriptException("Entity is probably not of player type - getping\n");
    }

    ping = ent->client->ps.ping;

    ev->AddInteger(ping);
}

void ScriptThread::GetPlayerClientNum(Event *ev)
{
    Entity *ent    = NULL;
    int     cl_num = -1;

    ent = (Entity *)ev->GetEntity(1);

    if (ent == NULL) {
        throw ScriptException("Player entity is NULL for getclientnum!\n");
    } else if (ent->client == NULL) {
        throw ScriptException("Entity is probably not of player type - getclientnum\n");
    }

    cl_num = ent->client->ps.clientNum;

    ev->AddInteger(cl_num);
}

void ScriptThread::GetAreaEntities(Event *ev)
{
    Vector          origin;
    Vector          mins;
    Vector          maxs;
    Vector          o_min;
    Vector          o_max;
    int             touch[MAX_GENTITIES];
    int             count;
    int             j   = 0;
    ScriptVariable *ref = new ScriptVariable, *array = new ScriptVariable;

    origin = ev->GetVector(1);
    mins   = ev->GetVector(2);
    maxs   = ev->GetVector(3);

    o_min = origin + mins;
    o_max = origin + maxs;

    count = gi.AreaEntities(o_min, o_max, touch, MAX_GENTITIES);

    ref->setRefValue(array);

    for (int i = 0; i < count; i++) {
        Entity *entity = G_GetEntity(touch[i]);

        if (entity == NULL) {
            continue;
        }

        ScriptVariable *index = new ScriptVariable, *value = new ScriptVariable;

        index->setIntValue(j);
        value->setListenerValue(entity);

        ref->setArrayAt(*index, *value);

        j++;
    }

    ev->AddValue(*array);
}

void ScriptThread::GetEntArray(Event *ev)
{
    str name = ev->GetString(1);
    str key  = ev->GetString(2);

    ScriptVariable array;
    ScriptVariable index, value;
    gentity_t     *gentity = globals.gentities;

    Event   *event       = new Event(key, EV_GETTER);
    qboolean useEvent    = event != NULL;
    qboolean createEvent = false;

    if (!event->eventnum) {
        delete event;
        useEvent = false;
    }

    int x = 0;

    for (int i = 0; i < globals.num_entities; i++, gentity++) {
        ScriptVariable returnValue;

        if (!gentity->inuse || gentity->entity == NULL) {
            continue;
        }

        if (createEvent) {
            event       = new Event(key, EV_GETTER);
            createEvent = false;
        }

        Entity *entity = gentity->entity;

        if (useEvent) {
            createEvent = true;
        }

        if (!useEvent) {
            // Now look for variables if predefined keys are not specified

            ScriptVariableList *vars = entity->Vars();

            if (vars == NULL) {
                continue;
            }

            // Get the variable from the key
            ScriptVariable *variable = vars->GetVariable(key);

            if (variable == NULL) {
                continue;
            }

            // Check if it matches with the name
            if (variable->stringValue() == name) {
                continue;
            }
        } else {
            const char *value = NULL;

            returnValue = entity->ProcessEventReturn(event);

            if (!event->NumArgs()) {
                continue;
            }

            value = returnValue.stringValue();

            if (value == NULL) {
                continue;
            }

            if (strcmp(value, name) != 0) {
                continue;
            }
        }

        index.setIntValue(x);
        value.setListenerValue(entity);

        array.setArrayAt(index, value);

        x++;
    }

    ev->AddValue(array);
}

void ScriptThread::EventIHudDraw3d(Event *ev)
{
    Player  *player;
    int      index;
    float   *tmp;
    vec3_t   vector;
    int      ent_num;
    qboolean bAlwaysShow, bDepth;

    player = (Player *)ev->GetEntity(1);

    if (player == NULL) {
        throw ScriptException("Player entity is NULL for ihuddraw_3d!\n");
    }

    index = ev->GetInteger(2);

    if (index < 0 && index > 255) {
        throw ScriptException("Wrong index for ihuddraw_3d!\n");
    }

    tmp = ev->GetVector(3);
    memcpy(&vector, tmp, sizeof(vec3_t));

    ent_num     = ev->GetInteger(4);
    bAlwaysShow = ev->GetInteger(5);
    bDepth      = ev->GetInteger(6);

    iHudDraw3d(player->edict - g_entities, index, vector, ent_num, bAlwaysShow, bDepth);
}

void ScriptThread::EventIHudDrawTimer(Event *ev)
{
    Player *player;
    int     index;
    float   duration;
    float   fade_out_time;

    player = (Player *)ev->GetEntity(1);

    if (player == NULL) {
        throw ScriptException("Player entity is NULL for ihuddraw_timer!\n");
    }

    index = ev->GetInteger(2);

    if (index < 0 && index > 255) {
        throw ScriptException("Wrong index for ihuddraw_timer!\n");
    }

    duration      = ev->GetFloat(3);
    fade_out_time = ev->GetFloat(4);

    iHudDrawTimer(player->edict - g_entities, index, duration, fade_out_time);
}

void ScriptThread::EventIHudDrawShader(Event *ev)
{
    int     numArgs = -1;
    int     index   = -1;
    Entity *player  = NULL;
    str     shadername;

    numArgs = ev->NumArgs();

    if (numArgs != 3) {
        throw ScriptException("Wrong arguments count for ihuddraw_shader!\n");
    }

    player = (Entity *)ev->GetEntity(1);

    if (player == NULL) {
        throw ScriptException("Player entity is NULL for ihuddraw_shader!\n");
    }

    index = ev->GetInteger(2);

    if (index < 0 && index > 255) {
        throw ScriptException("Wrong index for ihuddraw_shader!\n");
    }

    shadername = ev->GetString(3);

    iHudDrawShader(player->edict - g_entities, index, shadername);
}

void ScriptThread::EventIHudDrawAlign(Event *ev)
{
    int     numArgs      = -1;
    int     index        = -1;
    int     h_alignement = -1;
    int     v_alignement = -1;
    Entity *player       = NULL;
    str     h_align;
    str     v_align;

    numArgs = ev->NumArgs();

    if (numArgs != 4) {
        throw ScriptException("Wrong arguments count for ihuddraw_align!\n");
    }

    player = (Entity *)ev->GetEntity(1);

    if (player == NULL) {
        throw ScriptException("Player entity is NULL for ihuddraw_align!\n");
    }

    index = ev->GetInteger(2);

    if (index < 0 && index > 255) {
        throw ScriptException("Wrong index for ihuddraw_align!\n");
    }

    h_align = ev->GetString(3);

    if (!h_align) {
        throw ScriptException("h_align is NULL for ihuddraw_align!\n");
    }

    v_align = ev->GetString(4);

    if (!v_align) {
        throw ScriptException("v_align is NULL for ihuddraw_align!\n");
    }

    if (h_align == "left") {
        h_alignement = 0;
    } else if (h_align == "center") {
        h_alignement = 1;
    } else if (h_align == "right") {
        h_alignement = 2;
    } else {
        throw ScriptException("Wrong alignement h_align string for ihuddraw_align!\n");
    }

    if (v_align == "top") {
        v_alignement = 0;
    } else if (v_align == "center") {
        v_alignement = 1;
    } else if (v_align == "bottom") {
        v_alignement = 2;
    } else {
        throw ScriptException("Wrong alignement v_align string for ihuddraw_align!\n");
    }

    iHudDrawAlign(player->edict - g_entities, index, h_alignement, v_alignement);
}

void ScriptThread::EventIHudDrawRect(Event *ev)
{
    int     numArgs = -1;
    int     index   = -1;
    Entity *player  = NULL;
    int     x       = 0;
    int     y       = 0;
    int     width   = 0;
    int     height  = 0;

    numArgs = ev->NumArgs();

    if (numArgs != 6) {
        throw ScriptException("Wrong arguments count for ihuddraw_rect!\n");
    }

    player = (Entity *)ev->GetEntity(1);

    if (player == NULL) {
        throw ScriptException("Player entity is NULL for ihuddraw_rect!\n");
    }

    index = ev->GetInteger(2);

    if (index < 0 && index > 255) {
        throw ScriptException("Wrong index for ihuddraw_rect!\n");
    }

    x      = ev->GetInteger(3);
    y      = ev->GetInteger(4);
    width  = ev->GetInteger(5);
    height = ev->GetInteger(6);

    iHudDrawRect(player->edict - g_entities, index, x, y, width, height);
}

void ScriptThread::EventIHudDrawVirtualSize(Event *ev)
{
    int     numArgs = -1;
    int     index   = -1;
    Entity *player  = NULL;
    int     virt    = -1;

    numArgs = ev->NumArgs();

    if (numArgs != 3) {
        throw ScriptException("Wrong arguments count for ihuddraw_virtualsize!\n");
    }

    player = (Entity *)ev->GetEntity(1);

    if (player == NULL) {
        throw ScriptException("Player entity is NULL for ihuddraw_virtualsize!\n");
    }

    index = ev->GetInteger(2);

    if (index < 0 && index > 255) {
        throw ScriptException("Wrong index for ihuddraw_virtualsize!\n");
    }

    virt = ev->GetInteger(3);

    if (virt != 0) {
        virt = 1;
    }

    iHudDrawVirtualSize(player->edict - g_entities, index, virt);
}

void ScriptThread::EventIHudDrawColor(Event *ev)
{
    int     numArgs  = -1;
    int     index    = -1;
    Entity *player   = NULL;
    float   color[3] = {0.0f, 0.0f, 0.0f};

    numArgs = ev->NumArgs();

    if (numArgs != 5) {
        throw ScriptException("Wrong arguments count for ihuddraw_color!\n");
    }

    player = (Entity *)ev->GetEntity(1);

    if (player == NULL) {
        throw ScriptException("Player entity is NULL for ihuddraw_color!\n");
    }

    index = ev->GetInteger(2);

    if (index < 0 && index > 255) {
        throw ScriptException("Wrong index for ihuddraw_color!\n");
    }

    color[0] = ev->GetFloat(3); // red
    color[1] = ev->GetFloat(4); // green
    color[2] = ev->GetFloat(5); // blue

    iHudDrawColor(player->edict - g_entities, index, color);
}

void ScriptThread::EventIHudDrawAlpha(Event *ev)
{
    int     numArgs = -1;
    int     index   = -1;
    Entity *player  = NULL;
    float   alpha   = 0.0f;

    numArgs = ev->NumArgs();

    if (numArgs != 3) {
        throw ScriptException("Wrong arguments count for ihuddraw_alpha!\n");
    }

    player = (Entity *)ev->GetEntity(1);

    if (player == NULL) {
        throw ScriptException("Player entity is NULL for ihuddraw_alpha!\n");
    }

    index = ev->GetInteger(2);

    if (index < 0 && index > 255) {
        throw ScriptException("Wrong index for ihuddraw_alpha!\n");
    }

    alpha = ev->GetFloat(3);

    iHudDrawAlpha(player->edict - g_entities, index, alpha);
}

void ScriptThread::EventIHudDrawString(Event *ev)
{
    int     numArgs = -1;
    int     index   = -1;
    Entity *player  = NULL;
    str     string;

    numArgs = ev->NumArgs();

    if (numArgs != 3) {
        throw ScriptException("Wrong arguments count for ihuddraw_string!\n");
    }

    player = (Entity *)ev->GetEntity(1);

    if (player == NULL) {
        throw ScriptException("Player entity is NULL for ihuddraw_string!\n");
    }

    index = ev->GetInteger(2);

    if (index < 0 && index > 255) {
        throw ScriptException("Wrong index for ihuddraw_string!\n");
    }

    string = ev->GetString(3);

    iHudDrawString(player->edict - g_entities, index, string);
}

void ScriptThread::EventIHudDrawFont(Event *ev)
{
    int     index;
    Entity *player = NULL;
    str     fontname;

    if (ev->NumArgs() != 3) {
        throw ScriptException("Wrong arguments count for ihuddraw_font!\n");
    }

    player = (Entity *)ev->GetEntity(1);

    if (player == NULL) {
        throw ScriptException("Player entity is NULL for ihuddraw_font!\n");
    }

    index = ev->GetInteger(2);

    if (index < 0 && index > 255) {
        throw ScriptException("Wrong index for ihuddraw_font!\n");
    }

    fontname = ev->GetString(3);

    iHudDrawFont(player->edict - g_entities, index, fontname);
}

void ScriptThread::EventIsOnGround(Event *ev)
{
    Entity *entity = ev->GetEntity(1);

    ev->AddInteger(entity->groundentity != NULL);
}

void ScriptThread::EventIsOutOfBounds(Event *ev)
{
    Entity *entity  = ev->GetEntity(1);
    int     areanum = gi.AreaForPoint(entity->origin);

    if (areanum == -1) {
        ev->AddInteger(1);
    } else {
        ev->AddInteger(0);
    }
}

void ScriptThread::TeamGetScore(Event *ev)
{
    str      teamname = ev->GetString(1);
    DM_Team *team     = dmManager.GetTeam(teamname);

    if (!team) {
        throw ScriptException("Invalid team \"%s\"", teamname.c_str());
    }

    if (team) {
        ev->AddInteger(team->m_teamwins);
    }
}

void ScriptThread::TeamSetScore(Event *ev)
{
    str      teamname;
    DM_Team *team;
    int      score;
    qboolean bAdd = false;

    teamname = ev->GetString(1);

    team = dmManager.GetTeam(teamname);
    if (!team) {
        throw ScriptException("Invalid team \"%s\"", teamname.c_str());
    }

    score = ev->GetInteger(2);

    if (ev->NumArgs() > 2) {
        bAdd = ev->GetInteger(3);
    }

    if (bAdd) {
        team->m_iKills += score;

        if (g_gametype->integer < 3) {
            team->m_teamwins += score;
        }
    } else {
        team->m_iKills = score;

        if (g_gametype->integer < 3) {
            team->m_teamwins = score;
        }
    }
}

void ScriptThread::CharToInt(Event *ev)
{
    str c = ev->GetString(1);

    ev->AddInteger(c[0]);
}

void ScriptThread::Conprintf(Event *ev)
{
    gi.Printf("%s", ev->GetString(1).c_str());
}

void ScriptThread::FileOpen(Event *ev)
{
    int   numArgs = -1;
    str   filename;
    str   accesstype;
    FILE *f       = NULL;
    char  buf[16] = {0};

    numArgs = ev->NumArgs();

    if (numArgs != 2) {
        throw ScriptException("Wrong arguments count for fopen!\n");
    }

    if (scriptfiles->integer == 32) {
        throw ScriptException(
            "Maximum count (32) of opened files is reached. Close at least one of them, to open new file - fopen!\n"
        );
    }

    filename = ev->GetString(1);

    accesstype = ev->GetString(2);

    f = fopen(filename, accesstype);

    if (f == NULL) {
        ev->AddInteger(0);
        return;
    } else {
        ev->AddInteger((int)(size_t)f);
        sprintf(buf, "%i", scriptfiles->integer + 1);
        gi.cvar_set("sv_scriptfiles", buf);
        return;
    }
}

void ScriptThread::FileWrite(Event *ev) {}

void ScriptThread::FileRead(Event *ev) {}

void ScriptThread::FileClose(Event *ev)
{
    int   id      = 0;
    int   numArgs = 0;
    int   ret     = 0;
    FILE *f       = NULL;
    char  buf[16] = {0};

    numArgs = ev->NumArgs();

    if (numArgs != 1) {
        throw ScriptException("Wrong arguments count for fclose!\n");
    }

    id = ev->GetInteger(1);

    /*if( (int)scriptFiles[0].f != id && (int)scriptFiles[1].f != id )
    {
    gi.Printf("Wrong file handle for fclose!\n");
    return;
    }

    if( (int)scriptFiles[0].f == id )
    {
    scriptFiles[0].inUse = 0;
    fclose( scriptFiles[0].f );
    return;
    }
    else if( (int)scriptFiles[1].f == id )
    {
    scriptFiles[1].inUse = 0;
    fclose( scriptFiles[1].f );
    return;
    }
    else
    {
    gi.Printf("Unknown error while closing file - fclose!\n");
    return;
    }*/

    f = (FILE *)id;

    if (f == NULL) {
        throw ScriptException("File handle is NULL for fclose!\n");
    }

    ret = fclose(f);

    if (ret == 0) {
        ev->AddInteger(0);
        sprintf(buf, "%i", scriptfiles->integer - 1);
        gi.cvar_set("sv_scriptfiles", buf);
        return;
    } else {
        ev->AddInteger(ret);
        return;
    }
}

void ScriptThread::FileEof(Event *ev)
{
    int   id      = 0;
    int   numArgs = 0;
    int   ret     = 0;
    FILE *f       = NULL;

    numArgs = ev->NumArgs();

    if (numArgs != 1) {
        throw ScriptException("Wrong arguments count for feof!\n");
    }

    id = ev->GetInteger(1);

    f = (FILE *)id;

    ret = feof(f);

    ev->AddInteger(ret);
}

void ScriptThread::FileSeek(Event *ev)
{
    int      id      = 0;
    int      numArgs = 0;
    int      pos     = 0;
    long int offset  = 0;
    int      ret     = 0;
    FILE    *f       = NULL;

    numArgs = ev->NumArgs();

    if (numArgs != 3) {
        throw ScriptException("Wrong arguments count for fseek!\n");
    }

    id = ev->GetInteger(1);

    f = (FILE *)id;

    offset = ev->GetInteger(2);

    if (offset < 0) {
        throw ScriptException("Wrong file offset! Should be starting from 0. - fseek\n");
    }

    pos = ev->GetInteger(3);

    if (pos != 0 && pos != 1 && pos != 2) {
        throw ScriptException("Wrong file offset start! Should be between 0 - 2! - fseek\n");
    }

    ret = fseek(f, offset, pos);

    ev->AddInteger(ret);
}

void ScriptThread::FileTell(Event *ev)
{
    int      id      = 0;
    int      numArgs = 0;
    long int ret     = 0;
    FILE    *f       = NULL;

    numArgs = ev->NumArgs();

    if (numArgs != 1) {
        throw ScriptException("Wrong arguments count for ftell!\n");
    }

    id = ev->GetInteger(1);

    f = (FILE *)id;

    ret = ftell(f);

    ev->AddInteger(ret);
}

void ScriptThread::FileRewind(Event *ev)
{
    int      id      = 0;
    int      numArgs = 0;
    long int ret     = 0;
    FILE    *f       = NULL;

    numArgs = ev->NumArgs();

    if (numArgs != 1) {
        throw ScriptException("Wrong arguments count for frewind!\n");
    }

    id = ev->GetInteger(1);

    f = (FILE *)id;

    rewind(f);
}

void ScriptThread::FilePutc(Event *ev)
{
    int   id      = 0;
    int   numArgs = 0;
    int   ret     = 0;
    FILE *f       = NULL;
    int   c       = 0;

    numArgs = ev->NumArgs();

    if (numArgs != 2) {
        throw ScriptException("Wrong arguments count for fputc!\n");
    }

    id = ev->GetInteger(1);

    f = (FILE *)id;

    c = ev->GetInteger(2);

    ret = fputc((char)c, f);

    ev->AddInteger(ret);
}

void ScriptThread::FilePuts(Event *ev)
{
    int   id      = 0;
    int   numArgs = 0;
    int   ret     = 0;
    FILE *f       = NULL;
    str   c;

    numArgs = ev->NumArgs();

    if (numArgs != 2) {
        throw ScriptException("Wrong arguments count for fputs!\n");
    }

    id = ev->GetInteger(1);

    f = (FILE *)id;

    c = ev->GetString(2);
    //gi.Printf("Putting line into a file\n");
    ret = fputs(c, f);
    //gi.Printf("Ret val: %i\n", ret);
    ev->AddInteger(ret);
}

void ScriptThread::FileGetc(Event *ev)
{
    int   id      = 0;
    int   numArgs = 0;
    int   ret     = 0;
    FILE *f       = NULL;

    numArgs = ev->NumArgs();

    if (numArgs != 1) {
        throw ScriptException("Wrong arguments count for fgetc!\n");
    }

    id = ev->GetInteger(1);

    f = (FILE *)id;

    ret = fgetc(f);

    ev->AddInteger(ret);
}

void ScriptThread::FileGets(Event *ev)
{
    int   id       = 0;
    int   numArgs  = 0;
    int   maxCount = 0;
    FILE *f        = NULL;
    char *c        = NULL;
    char *buff     = NULL;

    numArgs = ev->NumArgs();

    if (numArgs != 2) {
        throw ScriptException("Wrong arguments count for fgets!\n");
    }

    id = ev->GetInteger(1);

    f = (FILE *)id;

    maxCount = ev->GetInteger(2);

    if (maxCount <= 0) {
        throw ScriptException("Maximum buffer size should be higher than 0! - fgets\n");
    }

    buff = (char *)gi.Malloc(maxCount + 1);

    if (buff == NULL) {
        throw ScriptException(
            "Failed to allocate memory during fputs scriptCommand text buffer initialization! Try setting maximum "
            "buffer length lower.\n"
        );
        ev->AddInteger(-1);
    }

    memset(buff, 0, maxCount + 1);

    c = fgets(buff, maxCount, f);

    if (c == NULL) {
        ev->AddString("");
    } else {
        ev->AddString(c);
    }

    gi.Free(buff);
}

void ScriptThread::FileError(Event *ev)
{
    int   id      = 0;
    int   numArgs = 0;
    int   ret     = 0;
    FILE *f       = NULL;

    numArgs = ev->NumArgs();

    if (numArgs != 1) {
        throw ScriptException("Wrong arguments count for ferror!\n");
    }

    id = ev->GetInteger(1);

    f = (FILE *)id;

    ret = ferror(f);

    ev->AddInteger(ret);
}

void ScriptThread::FileFlush(Event *ev)
{
    int   id      = 0;
    int   numArgs = 0;
    int   ret     = 0;
    FILE *f       = NULL;

    numArgs = ev->NumArgs();

    if (numArgs != 1) {
        throw ScriptException("Wrong arguments count for fflush!\n");
    }

    id = ev->GetInteger(1);

    f = (FILE *)id;

    ret = fflush(f);

    ev->AddInteger(ret);
}

void ScriptThread::FileExists(Event *ev)
{
    int   id      = 0;
    int   numArgs = 0;
    FILE *f       = 0;
    str   filename;

    numArgs = ev->NumArgs();

    if (numArgs != 1) {
        throw ScriptException("Wrong arguments count for fexists!\n");
    }

    filename = ev->GetString(1);

    if (filename == NULL) {
        throw ScriptException("Empty file name passed to fexists!\n");
    }

    f = fopen(filename, "r");
    if (f) {
        fclose(f);
        ev->AddInteger(1);
    } else {
        ev->AddInteger(0);
    }
}

void ScriptThread::FileReadAll(Event *ev)
{
    int    id         = 0;
    int    numArgs    = 0;
    FILE  *f          = NULL;
    char  *ret        = NULL;
    long   currentPos = 0;
    size_t size       = 0;
    size_t sizeRead   = 0;

    numArgs = ev->NumArgs();

    if (numArgs != 1) {
        throw ScriptException("Wrong arguments count for freadall!\n");
    }

    id = ev->GetInteger(1);

    f = (FILE *)id;

    currentPos = ftell(f);
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, currentPos, SEEK_SET);

    ret = (char *)gi.Malloc(sizeof(char) * size + 1);

    if (ret == NULL) {
        ev->AddInteger(-1);
        throw ScriptException("Error while allocating memory buffer for file content - freadall!\n");
    }

    sizeRead      = fread(ret, 1, size, f);
    ret[sizeRead] = '\0';

    ev->AddString(ret);

    gi.Free(ret);
}

void ScriptThread::FileSaveAll(Event *ev)
{
    int    id        = 0;
    int    numArgs   = 0;
    FILE  *f         = NULL;
    size_t sizeWrite = 0;
    str    text;

    numArgs = ev->NumArgs();

    if (numArgs != 2) {
        throw ScriptException("Wrong arguments count for fsaveall!\n");
    }

    id = ev->GetInteger(1);
    f  = (FILE *)id;

    text = ev->GetString(2);

    if (text == NULL) {
        ev->AddInteger(-1);
        throw ScriptException("Text to be written is NULL - fsaveall!\n");
    }

    sizeWrite = fwrite(text, 1, strlen(text), f);

    ev->AddInteger((int)sizeWrite);
}

void ScriptThread::FileRemove(Event *ev)
{
    int id      = 0;
    int numArgs = 0;
    int ret     = 0;
    str filename;

    numArgs = ev->NumArgs();

    if (numArgs != 1) {
        throw ScriptException("Wrong arguments count for fremove!\n");
    }

    filename = ev->GetString(1);

    if (filename == NULL) {
        throw ScriptException("Empty file name passed to fremove!\n");
    }

    ret = remove(filename);

    ev->AddInteger(ret);
}

void ScriptThread::FileRename(Event *ev)
{
    int id      = 0;
    int numArgs = 0;
    int ret     = 0;
    str oldfilename, newfilename;

    numArgs = ev->NumArgs();

    if (numArgs != 2) {
        throw ScriptException("Wrong arguments count for frename!\n");
    }

    oldfilename = ev->GetString(1);
    newfilename = ev->GetString(2);

    if (!oldfilename) {
        throw ScriptException("Empty old file name passed to frename!\n");
    }

    if (!newfilename) {
        throw ScriptException("Empty new file name passed to frename!\n");
    }

    ret = rename(oldfilename, newfilename);

    ev->AddInteger(ret);
}

void ScriptThread::FileCopy(Event *ev)
{
    size_t       n       = 0;
    int          numArgs = 0;
    unsigned int ret     = 0;
    str          filename, copyfilename;
    FILE        *f = NULL, *fCopy = NULL;
    char         buffer[4096];

    numArgs = ev->NumArgs();

    if (numArgs != 2) {
        throw ScriptException("Wrong arguments count for fcopy!\n");
    }

    filename     = ev->GetString(1);
    copyfilename = ev->GetString(2);

    if (!filename) {
        throw ScriptException("Empty file name passed to fcopy!\n");
    }

    if (copyfilename) {
        throw ScriptException("Empty copy file name passed to fcopy!\n");
    }

    f = fopen(filename, "rb");

    if (f == NULL) {
        ev->AddInteger(-1);
        throw ScriptException("Could not open \"%s\" for copying - fcopy!\n", filename.c_str());
    }

    fCopy = fopen(copyfilename, "wb");

    if (fCopy == NULL) {
        fclose(f);
        ev->AddInteger(-2);
        throw ScriptException("Could not open \"%s\" for copying - fcopy!\n", copyfilename.c_str());
    }

    while ((n = fread(buffer, sizeof(char), sizeof(buffer), f)) > 0) {
        if (fwrite(buffer, sizeof(char), n, fCopy) != n) {
            fclose(f);
            fflush(fCopy);
            fclose(fCopy);
            ev->AddInteger(-3);
            throw ScriptException("There was an error while copying files - fcopy!\n");
        }
    }

    fclose(f);
    fflush(fCopy);
    fclose(fCopy);

    ev->AddInteger(0);
}

void ScriptThread::FileReadPak(Event *ev)
{
    str   filename;
    char *content = NULL;
    int   numArgs = 0;
    int   ret     = 0;

    numArgs = ev->NumArgs();

    if (numArgs != 1) {
        throw ScriptException("Wrong arguments count for freadpak!\n");
    }

    filename = ev->GetString(1);

    if (filename == NULL) {
        throw ScriptException("Filename is NULL - freadpak!\n");
    }

    ret = gi.FS_ReadFile(filename, (void **)&content, qtrue);

    if (content == NULL) {
        ev->AddInteger(-1);
        throw ScriptException("Error while reading pak file content - freadpak!\n");
    }

    ev->AddString(content);
}

void ScriptThread::FileList(Event *ev)
{
    int             i = 0, numArgs = 0;
    const char     *path = NULL;
    str             extension;
    int             wantSubs = 0;
    int             numFiles = 0;
    char          **list     = NULL;
    ScriptVariable *ref      = new ScriptVariable;
    ScriptVariable *array    = new ScriptVariable;

    numArgs = ev->NumArgs();

    if (numArgs != 3) {
        throw ScriptException("Wrong arguments count for flist!\n");
    }

    path      = ev->GetString(1);
    extension = ev->GetString(2);
    wantSubs  = ev->GetInteger(3);

    list = gi.FS_ListFiles(path, extension, wantSubs, &numFiles);

    if (numFiles == 0) {
        gi.FS_FreeFileList(list);
        return;
    }

    ref->setRefValue(array);

    for (i = 0; i < numFiles; i++) {
        ScriptVariable *indexes = new ScriptVariable;
        ScriptVariable *values  = new ScriptVariable;

        indexes->setIntValue(i);
        values->setStringValue(list[i]);

        ref->setArrayAt(*indexes, *values);
    }

    gi.FS_FreeFileList(list);

    ev->AddValue(*array);

    return;
}

void ScriptThread::FileNewDirectory(Event *ev)
{
    str path;
    int numArgs = 0;
    int ret     = 0;

    numArgs = ev->NumArgs();

    if (numArgs != 1) {
        throw ScriptException("Wrong arguments count for fnewdir!\n");
    }

    path = ev->GetString(1);

    if (path == NULL) {
        throw ScriptException("Path is NULL - fnewdir!\n");
    }

#ifdef WIN32
    ret = _mkdir(path);
#else
    ret = mkdir(path, 0777);
#endif

    ev->AddInteger(ret);
    return;
}

void ScriptThread::FileRemoveDirectory(Event *ev)
{
    str path;
    int numArgs = 0;
    int ret     = 0;

    numArgs = ev->NumArgs();

    if (numArgs != 1) {
        throw ScriptException("Wrong arguments count for fremovedir!\n");
    }

    path = ev->GetString(1);

    if (path == NULL) {
        throw ScriptException("Path is NULL - fremovedir!\n");
    }

#ifdef WIN32
    ret = _rmdir(path);
#else
    ret = rmdir(path);
#endif

    ev->AddInteger(ret);
    return;
}

void ScriptThread::GetArrayKeys(Event *ev)
{
    Entity         *ent = NULL;
    ScriptVariable  array;
    ScriptVariable *value;
    int             i = 0;
    int             arraysize;

    /* Retrieve the array */
    array = ev->GetValue(1);

    /* Cast the array */
    array.CastConstArrayValue();
    arraysize = array.arraysize();

    if (arraysize < 1) {
        return;
    }

    ScriptVariable *ref = new ScriptVariable, *newArray = new ScriptVariable;

    ref->setRefValue(newArray);

    for (int i = 1; i <= arraysize; i++) {
        value = array[i];

        /* Get the array's name */
        //str name = value->getName();

        gi.Printf("name = %s\n", value->GetTypeName());

        ScriptVariable *newIndex = new ScriptVariable, *newValue = new ScriptVariable;

        newIndex->setIntValue(i);
        newValue->setStringValue("NIL");

        //name.removeRef();

        ref->setArrayAt(*newIndex, *newValue);
    }

    ev->AddValue(*newArray);
}

void ScriptThread::GetArrayValues(Event *ev)
{
    Entity         *ent = NULL;
    ScriptVariable  array;
    ScriptVariable *value;
    int             i = 0;
    int             arraysize;

    /* Retrieve the array */
    array = ev->GetValue(1);

    if (array.GetType() == VARIABLE_NONE) {
        return;
    }

    /* Cast the array */
    array.CastConstArrayValue();
    arraysize = array.arraysize();

    if (arraysize < 1) {
        return;
    }

    ScriptVariable *ref = new ScriptVariable, *newArray = new ScriptVariable;

    ref->setRefValue(newArray);

    for (int i = 1; i <= arraysize; i++) {
        value = array[i];

        ScriptVariable *newIndex = new ScriptVariable;

        newIndex->setIntValue(i - 1);

        ref->setArrayAt(*newIndex, *value);
    }

    ev->AddValue(*newArray);
}

void ScriptThread::GetDate(Event *ev)
{
    char       buff[1024];
    time_t     rawtime;
    struct tm *timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buff, 64, "%d.%m.%Y %r", timeinfo);

    ev->AddString(buff);
}

void ScriptThread::GetTimeZone(Event *ev)
{
    int gmttime;
    int local;

    time_t     rawtime;
    struct tm *timeinfo, *ptm;

    int timediff;
    int tmp;

    tmp = ev->GetInteger(1);

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    local = timeinfo->tm_hour;

    ptm = gmtime(&rawtime);

    gmttime = ptm->tm_hour;

    timediff = local - gmttime;

    ev->AddInteger(timediff);
}

// IMPORTANT NOTE:
// SLRE is buggy, consider switch to Boost.Regex or .xpressive

void ScriptThread::PregMatch(Event *ev)
{
    slre_cap       sl_cap[32];
    int            i, j;
    size_t         iMaxLength;
    size_t         iLength;
    size_t         iFoundLength = 0;
    str            pattern, subject;
    ScriptVariable index, value, subindex, subvalue;
    ScriptVariable array, subarray;

    memset(sl_cap, 0, sizeof(sl_cap));

    pattern = ev->GetString(1);
    subject = ev->GetString(2);

    iMaxLength = strlen(subject);
    iLength    = 0;
    i          = 0;

    while (
        iLength < iMaxLength
        && (iFoundLength = slre_match(
                pattern, subject.c_str() + iLength, iMaxLength - iLength, sl_cap, sizeof(sl_cap) / sizeof(sl_cap[0]), 0
            )) > 0
    ) {
        subarray.Clear();

        for (j = 0; sl_cap[j].ptr != NULL; j++) {
            char *buffer;

            buffer                = (char *)gi.Malloc(sl_cap[j].len + 1);
            buffer[sl_cap[j].len] = 0;
            strncpy(buffer, sl_cap[j].ptr, sl_cap[j].len);

            subindex.setIntValue(j);
            subvalue.setStringValue(buffer);
            subarray.setArrayAtRef(subindex, subvalue);

            gi.Free(buffer);

            iLength += sl_cap[j].ptr - subject.c_str();
        }

        index.setIntValue(i);
        array.setArrayAtRef(index, subarray);

        i++;
    }

    ev->AddValue(array);
}

void ScriptThread::EventIsArray(Event *ev)
{
    ScriptVariable *value = &ev->GetValue(1);

    if (value == NULL) {
        return ev->AddInteger(0);
    }

    ev->AddInteger(
        value->type == VARIABLE_ARRAY || value->type == VARIABLE_CONSTARRAY || value->type == VARIABLE_SAFECONTAINER
    );
}

void ScriptThread::EventIsDefined(Event *ev)
{
    ev->AddInteger(!ev->IsNilAt(1));
}

void ScriptThread::FlagClear(Event *ev)
{
    str   name;
    Flag *flag;

    name = ev->GetString(1);

    flag = flags.FindFlag(name);

    if (flag == NULL) {
        throw ScriptException("Invalid flag '%s'\n", name.c_str());
    }

    delete flag;
}

void ScriptThread::FlagInit(Event *ev)
{
    str   name;
    Flag *flag;

    name = ev->GetString(1);

    flag = flags.FindFlag(name);

    if (flag != NULL) {
        flag->Reset();
        return;
    }

    flag            = new Flag;
    flag->bSignaled = false;
    strcpy(flag->flagName, name);
}

void ScriptThread::FlagSet(Event *ev)
{
    str   name;
    Flag *flag;

    name = ev->GetString(1);

    flag = flags.FindFlag(name);

    if (flag == NULL) {
        throw ScriptException("Invalid flag '%s'.\n", name.c_str());
    }

    flag->Set();
}

void ScriptThread::FlagWait(Event *ev)
{
    str   name;
    Flag *flag;

    name = ev->GetString(1);

    flag = flags.FindFlag(name);

    if (flag == NULL) {
        throw ScriptException("Invalid flag '%s'.\n", name.c_str());
    }

    flag->Wait(this);
}

void ScriptThread::EventATan2(Event *ev)
{
    int    numArgs = 0;
    double x = 0.0f, y = 0.0f, res = 0.0f;

    numArgs = ev->NumArgs();

    if (numArgs != 2) {
        throw ScriptException("Wrong arguments count for atan2!\n");
    }

    y = ev->GetFloat(1);
    x = ev->GetFloat(2);

    res = atan2(y, x);

    ev->AddFloat((float)res);
}

void ScriptThread::EventCosH(Event *ev)
{
    int    numArgs = 0;
    double x = 0.0f, res = 0.0f;

    numArgs = ev->NumArgs();

    if (numArgs != 1) {
        throw ScriptException("Wrong arguments count for cosh!\n");
    }

    x   = ev->GetFloat(1);
    res = cosh(x);

    ev->AddFloat((float)res);
}

void ScriptThread::EventSinH(Event *ev)
{
    int    numArgs = 0;
    double x = 0.0f, res = 0.0f;

    numArgs = ev->NumArgs();

    if (numArgs != 1) {
        throw ScriptException("Wrong arguments count for sinh!\n");
    }

    x   = ev->GetFloat(1);
    res = sinh(x);

    ev->AddFloat((float)res);
}

void ScriptThread::EventTanH(Event *ev)
{
    int    numArgs = 0;
    double x = 0.0f, res = 0.0f;

    numArgs = ev->NumArgs();

    if (numArgs != 1) {
        throw ScriptException("Wrong arguments count for tanh!\n");
    }

    x   = ev->GetFloat(1);
    res = tanh(x);

    ev->AddFloat((float)res);
}

void ScriptThread::EventExp(Event *ev)
{
    int    numArgs = 0;
    double x = 0.0f, res = 0.0f;

    numArgs = ev->NumArgs();

    if (numArgs != 1) {
        throw ScriptException("Wrong arguments count for exp!\n");
    }

    x   = ev->GetFloat(1);
    res = exp(x);

    ev->AddFloat((float)res);
}

void ScriptThread::EventFrexp(Event *ev)
{
    int             numArgs = 0;
    double          x = 0.0f, res = 0.0f;
    int             exp              = 0;
    ScriptVariable *ref              = new ScriptVariable;
    ScriptVariable *array            = new ScriptVariable;
    ScriptVariable *SignificandIndex = new ScriptVariable;
    ScriptVariable *ExponentIndex    = new ScriptVariable;
    ScriptVariable *SignificandVal   = new ScriptVariable;
    ScriptVariable *ExponentVal      = new ScriptVariable;

    numArgs = ev->NumArgs();

    if (numArgs != 1) {
        throw ScriptException("Wrong arguments count for frexp!\n");
    }

    x   = ev->GetFloat(1);
    res = frexp(x, &exp);

    ref->setRefValue(array);

    SignificandIndex->setStringValue("significand");
    ExponentIndex->setStringValue("exponent");

    SignificandVal->setFloatValue((float)res);
    ExponentVal->setIntValue(exp);

    ref->setArrayAt(*SignificandIndex, *SignificandVal);
    ref->setArrayAt(*ExponentIndex, *ExponentVal);

    ev->AddValue(*array);
}

void ScriptThread::EventLdexp(Event *ev)
{
    int    numArgs = 0;
    double x = 0.0f, res = 0.0f;
    int    exp = 0;

    numArgs = ev->NumArgs();

    if (numArgs != 2) {
        throw ScriptException("Wrong arguments count for ldexp!\n");
    }

    x   = ev->GetFloat(1);
    exp = ev->GetInteger(2);

    res = ldexp(x, exp);

    ev->AddFloat((float)res);
}

void ScriptThread::EventLog(Event *ev)
{
    int    numArgs = 0;
    double x = 0.0f, res = 0.0f;

    numArgs = ev->NumArgs();

    if (numArgs != 1) {
        throw ScriptException("Wrong arguments count for log!\n");
    }

    x   = ev->GetFloat(1);
    res = log(x);

    ev->AddFloat((float)res);
}

void ScriptThread::EventLog10(Event *ev)
{
    int    numArgs = 0;
    double x = 0.0f, res = 0.0f;

    numArgs = ev->NumArgs();

    if (numArgs != 1) {
        throw ScriptException("Wrong arguments count for log10!\n");
    }

    x   = ev->GetFloat(1);
    res = log10(x);

    ev->AddFloat((float)res);
}

void ScriptThread::EventModf(Event *ev)
{
    int    numArgs = 0;
    double x = 0.0f, res = 0.0f;
    double intpart = 0;
    //char varIntpartIndex[16] = { 0 }, varFractionalIndex[16] = { 0 }, varIntpartVal[16] = { 0 }, varFractionalVal[16] = { 0 }, varArray[16] = { 0 }, varRef[16] = { 0 };
    ScriptVariable *array           = new ScriptVariable;
    ScriptVariable *ref             = new ScriptVariable;
    ScriptVariable *IntpartIndex    = new ScriptVariable;
    ScriptVariable *FractionalIndex = new ScriptVariable;
    ScriptVariable *FractionalVal   = new ScriptVariable;
    ScriptVariable *IntpartVal      = new ScriptVariable;

    numArgs = ev->NumArgs();

    if (numArgs != 1) {
        throw ScriptException("Wrong arguments count for modf!\n");
    }

    x   = ev->GetFloat(1);
    res = modf(x, &intpart);

    ref->setRefValue(array);

    IntpartIndex->setStringValue("intpart");
    FractionalIndex->setStringValue("fractional");
    FractionalVal->setFloatValue((float)res);
    IntpartVal->setFloatValue((float)intpart);

    ref->setArrayAt(*IntpartIndex, *IntpartVal);
    ref->setArrayAt(*FractionalIndex, *FractionalVal);

    ev->AddValue(*array);
}

void ScriptThread::EventPow(Event *ev)
{
    int    numArgs = 0;
    double base = 0.0f, res = 0.0f;
    int    exponent = 0;

    numArgs = ev->NumArgs();

    if (numArgs != 2) {
        throw ScriptException("Wrong arguments count for pow!\n");
    }

    base     = ev->GetFloat(1);
    exponent = ev->GetInteger(2);
    res      = pow(base, exponent);

    ev->AddFloat((float)res);
}

void ScriptThread::EventACos(Event *ev)
{
    int    numArgs = 0;
    double x = 0.0f, res = 0.0f;

    numArgs = ev->NumArgs();

    if (numArgs != 1) {
        throw ScriptException("Wrong arguments count for acos!\n");
    }

    x   = ev->GetFloat(1);
    res = acos(x);

    ev->AddFloat((float)res);
}

void ScriptThread::EventASin(Event *ev)
{
    int    numArgs = 0;
    double x = 0.0f, res = 0.0f;

    numArgs = ev->NumArgs();

    if (numArgs != 1) {
        throw ScriptException("Wrong arguments count for asin!\n");
    }

    x   = ev->GetFloat(1);
    res = asin(x);

    ev->AddFloat((float)res);
}

void ScriptThread::EventCeil(Event *ev)
{
    int    numArgs = 0;
    double x = 0.0f, res = 0.0f;

    numArgs = ev->NumArgs();

    if (numArgs != 1) {
        throw ScriptException("Wrong arguments count for ceil!\n");
    }

    x   = ev->GetFloat(1);
    res = ceil(x);

    ev->AddFloat((float)res);
}

void ScriptThread::EventFloor(Event *ev)
{
    int    numArgs = 0;
    double x = 0.0f, res = 0.0f;

    numArgs = ev->NumArgs();

    if (numArgs != 1) {
        throw ScriptException("Wrong arguments count for floor!\n");
    }

    x   = ev->GetFloat(1);
    res = floor(x);

    ev->AddFloat((float)res);
}

void ScriptThread::EventFmod(Event *ev)
{
    int    numArgs   = 0;
    double numerator = 0.0f, denominator = 0.0f, res = 0.0f;

    numArgs = ev->NumArgs();

    if (numArgs != 2) {
        throw ScriptException("Wrong arguments count for fmod!\n");
    }

    numerator   = ev->GetFloat(1);
    denominator = ev->GetFloat(2);
    res         = fmod(numerator, denominator);

    ev->AddFloat((float)res);
}

int checkMD5(const char *filepath, char *md5Hash)
{
    md5_state_t state;
    md5_byte_t  digest[16];
    int         di;

    FILE  *f         = NULL;
    char  *buff      = NULL;
    size_t filesize  = 0;
    size_t bytesread = 0;

    f = fopen(filepath, "rb");

    if (f == NULL) {
        return -1;
    }

    fseek(f, 0, SEEK_END);
    filesize = ftell(f);
    rewind(f);

    //gi.Printf("Size: %i\n", filesize);

    buff = (char *)gi.Malloc(filesize + 1);

    if (buff == NULL) {
        fclose(f);
        Com_Printf("error0\n");
        return -2;
    }

    buff[filesize] = '\0';

    bytesread = fread(buff, 1, filesize, f);

    if (bytesread < filesize) {
        gi.Free(buff);
        fclose(f);
        Com_Printf("error1: %zi\n", bytesread);
        return -3;
    }

    fclose(f);

    md5_init(&state);
    md5_append(&state, (const md5_byte_t *)buff, filesize);
    md5_finish(&state, digest);

    for (di = 0; di < 16; ++di) {
        sprintf(md5Hash + di * 2, "%02x", digest[di]);
    }

    gi.Free(buff);

    return 0;
}

int checkMD5String(const char *string, char *md5Hash)
{
    md5_state_t state;
    md5_byte_t  digest[16];
    int         di;

    char  *buff      = NULL;
    size_t stringlen = 0;

    stringlen = strlen(string);

    buff = (char *)gi.Malloc(stringlen + 1);

    if (buff == NULL) {
        return -1;
    }

    buff[stringlen] = '\0';
    memcpy(buff, string, stringlen);

    md5_init(&state);
    md5_append(&state, (const md5_byte_t *)buff, stringlen);
    md5_finish(&state, digest);

    for (di = 0; di < 16; ++di) {
        sprintf(md5Hash + di * 2, "%02x", digest[di]);
    }

    gi.Free(buff);

    return 0;
}

void ScriptThread::Md5File(Event *ev)
{
    char hash[64];
    str  filename;
    int  ret = 0;

    if (ev->NumArgs() != 1) {
        throw ScriptException("Wrong arguments count for md5file!\n");
    }

    filename = ev->GetString(1);

    ret = checkMD5(filename, hash);
    if (ret != 0) {
        ev->AddInteger(-1);
        throw ScriptException("Error while generating MD5 checksum for file - md5file!\n");
    }

    ev->AddString(hash);
}

void ScriptThread::StringBytesCopy(Event *ev)
{
    int   bytes  = ev->GetInteger(1);
    str   source = ev->GetString(2);
    char *buffer;

    buffer = (char *)gi.Malloc(bytes + 1);

    strncpy(buffer, source, bytes);
    buffer[bytes] = 0;

    ev->AddString(buffer);

    gi.Free(buffer);
}

void ScriptThread::Md5String(Event *ev)
{
    char hash[64];
    str  text;
    int  ret = 0;

    if (ev->NumArgs() != 1) {
        throw ScriptException("Wrong arguments count for md5string!\n");
    }

    text = ev->GetString(1);

    ret = checkMD5String(text, hash);
    if (ret != 0) {
        ev->AddInteger(-1);
        throw ScriptException("Error while generating MD5 checksum for strin!\n");
    }

    ev->AddString(hash);
}

scriptedEvType_t EventNameToType(const char *eventname, char *fullname)
{
    scriptedEvType_t evType;

    const char *eventname_full;

    if (strcmp(eventname, "connected") == 0) {
        eventname_full = "ConnectedEvent";
        evType         = SE_CONNECTED;
    } else if (strcmp(eventname, "disconnected") == 0) {
        eventname_full = "DisconnectedEvent";
        evType         = SE_DISCONNECTED;
    } else if (strcmp(eventname, "spawn") == 0) {
        eventname_full = "SpawnEvent";
        evType         = SE_SPAWN;
    } else if (strcmp(eventname, "damage") == 0) {
        eventname_full = "DamageEvent";
        evType         = SE_DAMAGE;
    } else if (strcmp(eventname, "kill") == 0) {
        eventname_full = "KillEvent";
        evType         = SE_KILL;
    } else if (strcmp(eventname, "keypress") == 0) {
        eventname_full = "KeypressEvent";
        evType         = SE_KEYPRESS;
    } else if (strcmp(eventname, "intermission") == 0) {
        eventname_full = "IntermissionEvent";
        evType         = SE_INTERMISSION;
    } else if (strcmp(eventname, "servercommand") == 0) {
        eventname_full = "ServerCommandEvent";
        evType         = SE_SERVERCOMMAND;
    } else if (strcmp(eventname, "changeteam") == 0) {
        eventname_full = "ChangeTeamEvent";
        evType         = SE_CHANGETEAM;
    } else {
        return SE_DEFAULT;
    }

    if (fullname != NULL) {
        strcpy(fullname, eventname_full);
    }

    return evType;
}

void ScriptThread::RegisterEvent(Event *ev)
{
    str              eventname;
    char             eventname_full[64];
    scriptedEvType_t evType;

    eventname = ev->GetString(1);

    evType = EventNameToType(eventname, eventname_full);

    if (evType == SE_DEFAULT) {
        ev->AddInteger(0);
        throw ScriptException("Wrong event type name for registerev!\n");
    }

    if (scriptedEvents[evType].IsRegistered()) {
        ev->AddInteger(1);
        throw ScriptException("Scripted event '%s' is already registered\n", eventname.c_str());
    }

    scriptedEvents[evType].label.SetThread(ev->GetValue(2));

    if (evType == SE_KEYPRESS) {
        gi.cvar_set("sv_keypressevents", "1");
    } else if (evType == SE_SERVERCOMMAND) {
        gi.cvar_set("sv_servercmdevents", "1");
    }

    ev->AddInteger(0);
}

void ScriptThread::UnregisterEvent(Event *ev)
{
    str              eventname;
    int              numArgs = 0;
    scriptedEvType_t evType;

    eventname = ev->GetString(1);

    evType = EventNameToType(eventname, NULL);

    if (evType == scriptedEvType_t(~0u)) {
        ev->AddInteger(0);
        throw ScriptException("Wrong event type name for unregisterev!\n");
    }

    numArgs = ev->NumArgs();

    if (numArgs != 1) {
        throw ScriptException("Wrong arguments count for unregisterev!\n");
    }

    eventname = ev->GetString(1);

    if (!scriptedEvents[evType].IsRegistered()) {
        ev->AddInteger(1);
        return;
    }

    scriptedEvents[evType].label.Set("");

    if (evType == SE_KEYPRESS) {
        gi.cvar_set("sv_keypressevents", "0");
    } else if (evType == SE_SERVERCOMMAND) {
        gi.cvar_set("sv_servercmdevents", "0");
    }

    ev->AddInteger(0);
}

void ScriptThread::TypeOfVariable(Event *ev)
{
    int             numArgs = 0;
    char           *type    = NULL;
    ScriptVariable *variable;

    numArgs = ev->NumArgs();

    if (numArgs != 1) {
        throw ScriptException("Wrong arguments count for typeof!\n");
    }

    variable = (ScriptVariable *)&ev->GetValue(1);
    type     = (char *)variable->GetTypeName();

    ev->AddString(type);
}

void ScriptThread::VisionGetNaked(Event *ev)
{
    ev->AddString(vision_current);
}

void ScriptThread::VisionSetNaked(Event *ev)
{
    str     vision = ev->GetString(1);
    float   fade_time;
    cvar_t *mapname = gi.Cvar_Get("mapname", "", 0);

    if (ev->NumArgs() > 1) {
        fade_time = ev->GetFloat(2);
    } else {
        fade_time = 0.0f;
    }

    if (!vision.length()) {
        vision = mapname->string;
    }

    // We won't malicously overflow client commands :)
    if (vision.length() >= MAX_STRING_TOKENS) {
        throw ScriptException("vision_name exceeds the maximum vision name limit (256) !\n");
    }

    vision_current = vision;

    gi.SendServerCommand(-1, "vsn %s %f", vision.c_str(), fade_time);
}

void ScriptThread::GetTime(Event *ev)
{
    int  timearray[3], gmttime;
    char buff[1024];

    time_t     rawtime;
    struct tm *timeinfo, *ptm;

    int timediff;

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    timearray[0] = timeinfo->tm_hour;
    timearray[1] = timeinfo->tm_min;
    timearray[2] = timeinfo->tm_sec;

    ptm = gmtime(&rawtime);

    gmttime = ptm->tm_hour;

    timediff = timearray[0] - gmttime;

    sprintf(buff, "%02i:%02i:%02i", (int)timearray[0], (int)timearray[1], (int)timearray[2]);

    ev->AddString(buff);
}

void ScriptThread::TraceDetails(Event *ev)
{
    int     numArgs     = 0;
    int     pass_entity = 0;
    int     mask        = 0x2000B01;
    trace_t trace;
    Vector  vecStart, vecEnd, vecMins, vecMaxs;
    Entity *entity;
    //todo : remove all these vars and add one for index and one for value

    ScriptVariable array;
    ScriptVariable allSolidIndex, allSolidValue;
    ScriptVariable startSolidIndex, startSolidValue;
    ScriptVariable fractionIndex, fractionValue;
    ScriptVariable endPosIndex, endPosValue;
    ScriptVariable surfaceFlagsIndex, surfaceFlagsValue;
    ScriptVariable shaderNumIndex, shaderNumValue;
    ScriptVariable contentsIndex, contentsValue;
    ScriptVariable entityNumIndex, entityNumValue;
    ScriptVariable locationIndex, locationValue;
    ScriptVariable entityIndex, entityValue;

    numArgs = ev->NumArgs();

    if (numArgs < 2 || numArgs > 6) {
        throw ScriptException("Wrong arguments count for traced!\n");
    }

    vecStart = ev->GetVector(1);
    vecEnd   = ev->GetVector(2);

    if (numArgs >= 3) {
        pass_entity = ev->GetInteger(3);
    }

    if (numArgs >= 4) {
        vecMins = ev->GetVector(4);
    }

    if (numArgs >= 5) {
        vecMaxs = ev->GetVector(5);
    }

    if (numArgs == 6) {
        mask = ev->GetInteger(6);
    }

    gi.trace(&trace, vecStart, vecMins, vecMaxs, vecEnd, pass_entity, mask, 0, 0);

    allSolidIndex.setStringValue("allSolid");
    startSolidIndex.setStringValue("startSolid");
    fractionIndex.setStringValue("fraction");
    endPosIndex.setStringValue("endpos");
    surfaceFlagsIndex.setStringValue("surfaceFlags");
    shaderNumIndex.setStringValue("shaderNum");
    contentsIndex.setStringValue("contents");
    entityNumIndex.setStringValue("entityNum");
    locationIndex.setStringValue("location");
    entityIndex.setStringValue("entity");

    allSolidValue.setIntValue(trace.allsolid);
    startSolidValue.setIntValue(trace.startsolid);
    fractionValue.setFloatValue(trace.fraction);
    endPosValue.setVectorValue(trace.endpos);
    surfaceFlagsValue.setIntValue(trace.surfaceFlags);
    shaderNumValue.setIntValue(trace.shaderNum);
    contentsValue.setIntValue(trace.contents);
    entityNumValue.setIntValue(trace.entityNum);
    locationValue.setIntValue(trace.location);

    entity = G_GetEntity(trace.entityNum);

    // Have to use G_GetEntity instead otherwise it won't work
    if (entity != NULL) {
        entityValue.setListenerValue(entity);
    }

    array.setArrayAtRef(allSolidIndex, allSolidValue);
    array.setArrayAtRef(startSolidIndex, startSolidValue);
    array.setArrayAtRef(fractionIndex, fractionValue);
    array.setArrayAtRef(endPosIndex, endPosValue);
    array.setArrayAtRef(surfaceFlagsIndex, surfaceFlagsValue);
    array.setArrayAtRef(shaderNumIndex, shaderNumValue);
    array.setArrayAtRef(contentsIndex, contentsValue);
    array.setArrayAtRef(entityNumIndex, entityNumValue);
    array.setArrayAtRef(locationIndex, locationValue);
    array.setArrayAtRef(entityIndex, entityValue);

    ev->AddValue(array);
}