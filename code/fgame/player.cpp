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
// player.h: Class definition of the player.

#include "g_local.h"
#include "bg_local.h"
#include "g_spawn.h"
#include "g_phys.h"
#include "entity.h"
#include "consoleevent.h"
#include "player.h"
#include "worldspawn.h"
#include "weapon.h"
#include "trigger.h"
#include "scriptmaster.h"
#include "scriptexception.h"
#include "navigate.h"
#include "misc.h"
#include "earthquake.h"
#include "gravpath.h"
#include "armor.h"
#include "inventoryitem.h"
#include "gibs.h"
#include "actor.h"
#include "object.h"
#include "characterstate.h"
#include "weaputils.h"
#include "dm_manager.h"
#include "parm.h"
#include "body.h"
#include "playerstart.h"
#include "camera.h"
#include "weapturret.h"
#include "vehicleturret.h"
#include "portableturret.h"
#include "fixedturret.h"

const Vector power_color(0.0, 1.0, 0.0);
const Vector acolor(1.0, 1.0, 1.0);
const Vector bcolor(1.0, 0.0, 0.0);

//
// mohaas 2.0 and above
//
const char *pInstantMsgEng[6][9] = {
    {"Good job team!",
     "Alright!", "We've done it!",
     "Wooohoo!", "Objective achieved.",
     "We've completed an objective.", "We've lost an objective!",
     "The enemy has overrun our objective!", NULL},
    {"Squad, move in!",
     "Squad, fall back!", "Squad, attack right flank!",
     "Squad, attack left flank!", "Squad, hold this position!",
     "Squad, covering fire!", "Squad, regroup!",
     "Squad, split up!", NULL},
    {"Cover me!",
     "I'll cover you!", "Follow me!",
     "You take point.", "Taking Fire!  Need some help!",
     "Get ready to move in on my signal.", "Attack!",
     "Open fire!", NULL},
    {"Yes sir!",
     "No sir!", "Enemy Spotted.",
     "Sniper!", "Grenade! Take Cover!",
     "Area Clear.", "Thanks.",
     "I owe you one.", NULL},
    {"Who wants more?!",
     "Never send boys to do a man's job.", "This is too easy!",
     "You mess with the best, you die like the rest.", "Watch that friendly fire!",
     "Hey!  I'm on your team!", "Come on out you cowards!",
     "Where are you hiding?", NULL},
    //
    // Added in 2.30
    //
    {"Guard our jail!",
     "Capture the enemy jail!", "I'm defending our jail!",
     "I'm attacking the enemy jail!", "Rescue the Prisoners!",
     "The enemy is attacking our jail!"}
};

//
// for mohaa version 1.11 and below
//
const char *pInstantMsgEng_ver6[5][9] = {
    {"Squad, move in!",
     "Squad, fall back!", "Squad, attack right flank!",
     "Squad, attack left flank!", "Squad, hold this position!",
     "Squad, covering fire!", "Squad, regroup!",
     "", ""},
    {
     "Cover me!", "I'll cover you!",
     "Follow me!", "You take point.",
     "You take the lead.", "Taking Fire!  Need some help!",
     "Charge!", "Attack!",
     "Open fire!", },
    {
     "Yes sir!", "No sir!",
     "Enemy Spotted.", "Sniper!",
     "Grenade! Take Cover!", "Area Clear.",
     "Great Shot!", "Thanks.",
     "I owe you one.", },
    {
     "Is that all you've got?", "I think they are all out of real men!",
     "Go on and run, you yellow-bellies!", "They're a bunch of cowards!",
     "Come back when you've had some target practice!", "Come prepared next time!",
     "Try again!", "I've seen French school girls shoot better!",
     "That made a mess.", },
    {"He's going to get us killed!",
     "A lot of good men are going to die because of his poor leadership", "Good riddance!",
     "That guy is going to get us all killed!", "Hey buddy, get down!",
     "Stay out of my foxhole, pal!", "Find your own hiding place!",
     "Get out of my way!", ""}
};

qboolean TryPush(int entnum, vec3_t move_origin, vec3_t move_end);

Event EV_Player_DumpState
(
    "state",
    EV_CHEAT,
    NULL,
    NULL,
    "Dumps the player's state to the console.",
    EV_NORMAL
);
Event EV_Player_ForceTorsoState
(
    "forcetorsostate",
    EV_DEFAULT,
    "s",
    "torsostate",
    "Force the player's torso to a certain state",
    EV_NORMAL
);
Event EV_Player_ForceLegsState
(
    "forcelegsstate",
    EV_DEFAULT,
    "s",
    "legsstate",
    "Force the player's legs to a certain state",
    EV_NORMAL
);
Event EV_Player_GiveAllCheat
(
    "wuss",
    EV_CONSOLE | EV_CHEAT,
    NULL,
    NULL,
    "Gives player all weapons.",
    EV_NORMAL
);
Event EV_Player_GiveNewWeaponsCheat
(
    "giveweapon",
    EV_CONSOLE | EV_CHEAT,
    "s",
    "weapon_name",
    "Gives player all weapons.",
    EV_NORMAL
);
Event EV_Player_EndLevel
(
    "endlevel",
    EV_DEFAULT,
    NULL,
    NULL,
    "Called when the player gets to the end of the level.",
    EV_NORMAL
);
Event EV_Player_DevGodCheat
(
    "dog",
    EV_CHEAT | EV_CONSOLE,
    "I",
    "god_mode",
    "Sets the god mode cheat or toggles it.",
    EV_NORMAL
);
Event EV_Player_FullHeal
(
    "fullheal",
    EV_CHEAT | EV_CONSOLE,
    NULL,
    NULL,
    "Heals player.",
    EV_NORMAL
);
Event EV_Player_DevNoTargetCheat
(
    "notarget",
    EV_CHEAT,
    NULL,
    NULL,
    "Toggles the notarget cheat.",
    EV_NORMAL
);
Event EV_Player_DevNoClipCheat
(
    "noclip",
    EV_CHEAT | EV_CONSOLE,
    NULL,
    NULL,
    "Toggles the noclip cheat.",
    EV_NORMAL
);
Event EV_Player_PrevItem
(
    "invprev",
    EV_CONSOLE,
    NULL,
    NULL,
    "Cycle to player's previous item.",
    EV_NORMAL
);
Event EV_Player_NextItem
(
    "invnext",
    EV_CONSOLE,
    NULL,
    NULL,
    "Cycle to player's next item.",
    EV_NORMAL
);
Event EV_Player_PrevWeapon
(
    "weapprev",
    EV_CONSOLE,
    NULL,
    NULL,
    "Cycle to player's previous weapon.",
    EV_NORMAL
);
Event EV_Player_NextWeapon
(
    "weapnext",
    EV_CONSOLE,
    NULL,
    NULL,
    "Cycle to player's next weapon.",
    EV_NORMAL
);
Event EV_Player_DropWeapon
(
    "weapdrop",
    EV_CONSOLE,
    NULL,
    NULL,
    "Drops the player's current weapon.",
    EV_NORMAL
);
Event EV_Player_Reload
(
    "reload",
    EV_CONSOLE,
    NULL,
    NULL,
    "Reloads the player's weapon",
    EV_NORMAL
);
Event EV_Player_CorrectWeaponAttachments
(
    "correctweaponattachments",
    EV_CONSOLE,
    NULL,
    NULL,
    "makes sure the weapons is properly attached when interupting a reload",
    EV_NORMAL
);
Event EV_Player_GiveCheat
(
    "give",
    EV_CONSOLE | EV_CHEAT,
    "sI",
    "name amount",
    "Gives the player the specified thing (weapon, ammo, item, etc.) and optionally the amount.",
    EV_NORMAL
);
Event EV_Player_GiveWeaponCheat
(
    "giveweapon",
    EV_CONSOLE | EV_CHEAT,
    "s",
    "weapon_name",
    "Gives the player the specified weapon.",
    EV_NORMAL
);
Event EV_Player_GameVersion
(
    "gameversion",
    EV_CONSOLE,
    NULL,
    NULL,
    "Prints the game version.",
    EV_NORMAL
);
Event EV_Player_Fov
(
    "fov",
    EV_CONSOLE,
    "F",
    "fov",
    "Sets the fov.",
    EV_NORMAL
);
Event EV_Player_Dead
(
    "dead",
    EV_DEFAULT,
    NULL,
    NULL,
    "Called when the player is dead.",
    EV_NORMAL
);
Event EV_Player_SpawnEntity
(
    "spawn",
    EV_CHEAT,
    "sSSSSSSSS",
    "entityname keyname1 value1 keyname2 value2 keyname3 value3 keyname4 value4",
    "Spawns an entity.",
    EV_NORMAL
);
Event EV_Player_SpawnActor
(
    "actor",
    EV_CHEAT,
    "sSSSSSSSS",
    "modelname keyname1 value1 keyname2 value2 keyname3 value3 keyname4 value4",
    "Spawns an actor.",
    EV_NORMAL
);
Event EV_Player_Respawn
(
    "respawn",
    EV_DEFAULT,
    NULL,
    NULL,
    "Respawns the player.",
    EV_NORMAL
);
Event EV_Player_TestThread
(
    "testthread",
    EV_CHEAT,
    "sS",
    "scriptfile label",
    "Starts the named thread at label if provided.",
    EV_NORMAL
);
Event EV_Player_PowerupTimer
(
    "poweruptimer",
    EV_DEFAULT,
    "ii",
    "poweruptimer poweruptype",
    "Sets the powerup timer and powerup type.",
    EV_NORMAL
);
Event EV_Player_UpdatePowerupTimer
(
    "updatepoweruptime",
    EV_DEFAULT,
    NULL,
    NULL,
    "Called once a second to decrement powerup time.",
    EV_NORMAL
);
Event EV_Player_ResetState
(
    "resetstate",
    EV_CHEAT,
    NULL,
    NULL,
    "Reset the player's state table.",
    EV_NORMAL
);
Event EV_Player_WhatIs
(
    "whatis",
    EV_CHEAT,
    "i",
    "entity_number",
    "Prints info on the specified entity.",
    EV_NORMAL
);
Event EV_Player_ActorInfo
(
    "actorinfo",
    EV_CHEAT,
    "i",
    "actor_number",
    "Prints info on the specified actor.",
    EV_NORMAL
);
Event EV_Player_KillEnt
(
    "killent",
    EV_CHEAT,
    "i",
    "entity_number",
    "Kills the specified entity.",
    EV_NORMAL
);
Event EV_Player_KillClass
(
    "killclass",
    EV_CHEAT,
    "sI",
    "classname except_entity_number",
    "Kills all of the entities in the specified class.",
    EV_NORMAL
);
Event EV_Player_RemoveEnt
(
    "removeent",
    EV_CHEAT,
    "i",
    "entity_number",
    "Removes the specified entity.",
    EV_NORMAL
);
Event EV_Player_RemoveClass
(
    "removeclass",
    EV_CHEAT,
    "sI",
    "classname except_entity_number",
    "Removes all of the entities in the specified class.",
    EV_NORMAL
);
Event EV_Player_Jump
(
    "jump",
    EV_DEFAULT,
    "f",
    "height",
    "Makes the player jump.",
    EV_NORMAL
);
Event EV_Player_AnimLoop_Torso
(
    "animloop_torso",
    EV_DEFAULT,
    NULL,
    NULL,
    "Called when the torso animation has finished.",
    EV_NORMAL
);
Event EV_Player_AnimLoop_Legs
(
    "animloop_legs",
    EV_DEFAULT,
    NULL,
    NULL,
    "Called when the legs animation has finished.",
    EV_NORMAL
);
Event EV_Player_AnimLoop_Pain // Added in 2.0
    ("animloop_pain", EV_DEFAULT, NULL, NULL, "Called when the pain animation has finished.", EV_NORMAL);
Event EV_Player_DoUse
(
    "usestuff",
    EV_DEFAULT,
    NULL,
    NULL,
    "Makes the player try to use whatever is in front of her.",
    EV_NORMAL
);
Event EV_Player_ListInventory
(
    "listinventory",
    EV_CONSOLE,
    NULL,
    NULL,
    "List of the player's inventory.",
    EV_NORMAL
);
Event EV_Player_ActivateShield
(
    "activateshield",
    EV_DEFAULT,
    NULL,
    NULL,
    "Activates the player's shield",
    EV_NORMAL
);
Event EV_Player_DeactivateShield
(
    "deactivateshield",
    EV_DEFAULT,
    NULL,
    NULL,
    "Deactivates the player's shield",
    EV_NORMAL
);
Event EV_Player_Turn
(
    "turn",
    EV_DEFAULT,
    "f",
    "yawangle",
    "Causes player to turn the specified amount.",
    EV_NORMAL
);
Event EV_Player_TurnUpdate
(
    "turnupdate",
    EV_DEFAULT,
    "ff",
    "yaw timeleft",
    "Causes player to turn the specified amount.",
    EV_NORMAL
);
Event EV_Player_TurnLegs
(
    "turnlegs",
    EV_DEFAULT,
    "f",
    "yawangle",
    "Turns the players legs instantly by the specified amount.",
    EV_NORMAL
);
Event EV_Player_NextPainTime(
    "nextpaintime",
    EV_DEFAULT,
    "f",
    "seconds",
    "Set the next time the player experiences pain (Current time + seconds specified).",
    EV_NORMAL
);

Event EV_Player_FinishUseAnim
(
    "finishuseanim",
    EV_DEFAULT,
    NULL,
    NULL,
    "Fires off all targets associated with a particular useanim.",
    EV_NORMAL
);
Event EV_Player_Holster
(
    "holster",
    EV_CONSOLE,
    NULL,
    NULL,
    "Holsters all wielded weapons, or unholsters previously put away weapons",
    EV_NORMAL
);
Event EV_Player_SafeHolster
(
    "safeholster",
    EV_CONSOLE,
    "b",
    "putaway",
    "Holsters all wielded weapons, or unholsters previously put away weapons\n"
    "preserves state, so it will not holster or unholster unless necessary",
    EV_NORMAL
);
Event EV_Player_SafeZoom(
    "safezoom",
    EV_DEFAULT,
    "b",
    "zoomin",
    "0 turns off zoom,"
    "and 1 returns zoom to previous setting"
);
Event EV_Player_ZoomOff
(
    "zoomoff",
    EV_DEFAULT,
    NULL,
    NULL,
    "makes sure that zoom is off",
    EV_NORMAL
);
Event EV_Player_StartUseObject
(
    "startuseobject",
    EV_DEFAULT,
    NULL,
    NULL,
    "starts up the useobject's animations.",
    EV_NORMAL
);
Event EV_Player_FinishUseObject
(
    "finishuseobject",
    EV_DEFAULT,
    NULL,
    NULL,
    "Fires off all targets associated with a particular useobject.",
    EV_NORMAL
);
Event EV_Player_WatchActor
(
    "watchactor",
    EV_DEFAULT,
    "e",
    "actor_to_watch",
    "Makes the player's camera watch the specified actor.",
    EV_NORMAL
);
Event EV_Player_StopWatchingActor
(
    "stopwatchingactor",
    EV_DEFAULT,
    "e",
    "actor_to_stop_watching",
    "Makes the player's camera stop watching the specified actor.",
    EV_NORMAL
);
Event EV_Player_SetDamageMultiplier
(
    "damage_multiplier",
    EV_DEFAULT,
    "f",
    "damage_multiplier",
    "Sets the current damage multiplier",
    EV_NORMAL
);
Event EV_Player_WaitForState
(
    "waitForState",
    EV_DEFAULT,
    "s",
    "stateToWaitFor",
    "When set, the player will clear waitforplayer when this state is hit\n"
    "in the legs or torso.",
    EV_NORMAL
);
Event EV_Player_LogStats
(
    "logstats",
    EV_CHEAT,
    "b",
    "state",
    "Turn on/off the debugging playlog",
    EV_NORMAL
);
Event EV_Player_TakePain
(
    "takepain",
    EV_DEFAULT,
    "b",
    "bool",
    "Set whether or not to take pain",
    EV_NORMAL
);
Event EV_Player_SkipCinematic
(
    "skipcinematic",
    EV_CONSOLE,
    NULL,
    NULL,
    "Skip the current cinematic",
    EV_NORMAL
);
Event EV_Player_ResetHaveItem
(
    "resethaveitem",
    EV_CONSOLE,
    "s",
    "weapon_name",
    "Resets the game var that keeps track that we have gotten this weapon",
    EV_NORMAL
);
Event EV_Player_ModifyHeight
(
    "modheight",
    EV_DEFAULT,
    "s",
    "height",
    "change the maximum height of the player\ncan specify 'stand', 'duck' or 'prone'.",
    EV_NORMAL
);
Event EV_Player_ModifyHeightFloat // Added in 2.40
    ("modheightfloat",
     EV_DEFAULT,
     "ff",
     "height max_z",
     "Specify the view height of the player and the height of his bounding box.",
     EV_NORMAL);
Event EV_Player_SetMovePosFlags
(
    "moveposflags",
    EV_DEFAULT,
    "sS",
    "position movement",
    "used by the state files to tell the game dll what the player is doing",
    EV_NORMAL
);
Event EV_Player_GetPosition
(
    "getposition",
    EV_DEFAULT,
    NULL,
    NULL,
    "returns the player current position",
    EV_RETURN
);
Event EV_Player_GetMovement
(
    "getmovement",
    EV_DEFAULT,
    NULL,
    NULL,
    "returns the player current movement",
    EV_RETURN
);
Event EV_Player_Score
(
    "score",
    EV_CONSOLE,
    NULL,
    NULL,
    "Show the score for the current deathmatch game",
    EV_NORMAL
);
Event EV_Player_JoinDMTeam
(
    "join_team",
    EV_CONSOLE,
    "s",
    "team",
    "Join the specified team (allies or axis)",
    EV_NORMAL
);
Event EV_Player_AutoJoinDMTeam
(
    "auto_join_team",
    EV_CONSOLE,
    NULL,
    NULL,
    "Join the team with fewer players",
    EV_NORMAL
);
Event EV_Player_PickWeapon
(
    "pickweapon",
    EV_CONSOLE,
    NULL,
    NULL,
    "Pick your weapon.",
    EV_NORMAL
);
Event EV_Player_SetInJail // Added in 2.30
    ("injail",
     EV_DEFAULT,
     "i",
     "boolean",
     "set to 1 to indicate when player is in jail,"
     "0 when they are free",
     EV_SETTER);
Event EV_Player_GetInJail // Added in 2.30
    ("injail",
     EV_DEFAULT,
     NULL,
     NULL,
     "returns 1 if player is in jail,"
     "0 if out",
     EV_GETTER);
Event EV_Player_GetNationalityPrefix // Added in 2.30
    ("nationalityprefix",
     EV_DEFAULT,
     NULL,
     NULL,
     "get the three or five letter prefix that denotes the player's nationality",
     EV_GETTER);
Event EV_Player_IsSpectator // Added in 2.30
    ("isSpectator", EV_DEFAULT, NULL, NULL, "Check to see if player is a spectator (non-zero return value)", EV_GETTER);
Event EV_Player_Spectator
(
    "spectator",
    EV_CONSOLE,
    NULL,
    NULL,
    "Become a spectator",
    EV_NORMAL
);
Event EV_Player_JoinArena
(
    "join_arena",
    EV_CONSOLE,
    "i",
    "arena_id_num",
    "Join the specified arena",
    EV_NORMAL
);
Event EV_Player_LeaveArena
(
    "leave_arena",
    EV_CONSOLE,
    NULL,
    NULL,
    "Leave the current arena",
    EV_NORMAL
);
Event EV_Player_CreateTeam
(
    "create_team",
    EV_CONSOLE,
    NULL,
    NULL,
    "Create a team in the current arena",
    EV_NORMAL
);
Event EV_Player_LeaveTeam
(
    "leave_team",
    EV_CONSOLE,
    NULL,
    NULL,
    "Leave the current team",
    EV_NORMAL
);
Event EV_Player_RefreshArenaUI
(
    "arena_ui",
    EV_CONSOLE,
    NULL,
    NULL,
    "Refresh the arena UI",
    EV_NORMAL
);
Event EV_Player_CallVote
(
    "callvote",
    EV_CONSOLE,
    "ss",
    "arg1 arg2",
    "Player calls a vote",
    EV_NORMAL
);
Event EV_Player_Vote
(
    "vote",
    EV_CONSOLE,
    "s",
    "arg1",
    "Player votes either yes or no",
    EV_NORMAL
);
Event EV_Player_RetrieveVoteOptions // Added in 2.0
    ("gvo", EV_CONSOLE, NULL, NULL, "Retrieves the server's vote options file", EV_NORMAL);
Event EV_Player_PrimaryDMWeapon
(
    "primarydmweapon",
    EV_CONSOLE,
    "s",
    "weaptype",
    "Sets the player's primary DM weapon",
    EV_NORMAL
);
Event EV_Player_DeadBody
(
    "deadbody",
    EV_DEFAULT,
    NULL,
    NULL,
    "Spawn a dead body",
    EV_NORMAL
);
Event EV_Player_Physics_On
(
    "physics_on",
    EV_DEFAULT,
    NULL,
    NULL,
    "turn player physics on.",
    EV_NORMAL
);
Event EV_Player_Physics_Off
(
    "physics_off",
    EV_DEFAULT,
    NULL,
    NULL,
    "turn player physics off.",
    EV_NORMAL
);
Event EV_Player_ArmWithWeapons // Added in 2.30
    ("armwithweapons", EV_DEFAULT, NULL, NULL, "give player their primary and secondary weapons.", EV_NORMAL);
Event EV_Player_GetCurrentDMWeaponType // Added in 2.30
    ("getcurrentdmweapontype", EV_DEFAULT, NULL, NULL, "get the player's current DM weapon type.", EV_GETTER);
Event EV_Player_AttachToLadder
(
    "attachtoladder",
    EV_DEFAULT,
    NULL,
    NULL,
    "Attaches the sentient to a ladder",
    EV_NORMAL
);
Event EV_Player_UnattachFromLadder
(
    "unattachfromladder",
    EV_DEFAULT,
    NULL,
    NULL,
    "Unattaches the sentient from a ladder",
    EV_NORMAL
);
Event EV_Player_TweakLadderPos
(
    "tweakladderpos",
    EV_DEFAULT,
    NULL,
    NULL,
    "Tweaks the player's position on a ladder to be proper",
    EV_NORMAL
);
Event EV_Player_EnsureOverLadder
(
    "ensureoverladder",
    EV_DEFAULT,
    NULL,
    NULL,
    "Ensures that the player is at the proper height when getting off the top of a ladder",
    EV_NORMAL
);
Event EV_Player_EnsureForwardOffLadder
(
    "ensureforwardoffladder",
    EV_DEFAULT,
    NULL,
    NULL,
    "Ensures that the player went forward off the ladder.",
    EV_NORMAL
);
Event EV_Player_JailIsEscaping // Added in 2.30
    ("isEscaping", EV_DEFAULT, NULL, NULL, "Return non-zero if escaping or assisting escape", EV_GETTER);
Event EV_Player_JailEscape // Added in 2.30
    ("jailescape", EV_DEFAULT, NULL, NULL, "Start the escape from jail animation", EV_NORMAL);
Event EV_Player_JailAssistEscape // Added in 2.30
    ("jailassistescape", EV_DEFAULT, NULL, NULL, "Start the assist jail escape animation", EV_NORMAL);
Event EV_Player_JailEscapeStop // Added in 2.30
    ("jailescapestop", EV_DEFAULT, NULL, NULL, "Stop either the escape from jail or assist animation", EV_NORMAL);
Event EV_Player_GetIsDisguised
(
    "is_disguised",
    EV_DEFAULT,
    NULL,
    NULL,
    "zero = not disguised"
    "non-zero = disguised",
    EV_GETTER
);
Event EV_Player_GetHasDisguise
(
    "has_disguise",
    EV_DEFAULT,
    NULL,
    NULL,
    "zero = does not have a disguise,"
    "non - zero = has a disguise",
    EV_GETTER
);

Event EV_Player_SetHasDisguise
(
    "has_disguise",
    EV_DEFAULT,
    "i",
    "is_disguised",
    "zero = does not have a disguise,"
    "non - zero = has a disguise",
    EV_SETTER
);
Event EV_Player_ObjectiveCount
(
    "objective",
    EV_DEFAULT,
    "ii",
    "num_completed out_of",
    "Sets the number of objectives completed and the total number of objectives",
    EV_NORMAL
);
Event EV_Player_Stats
(
    "stats",
    EV_CONSOLE,
    NULL,
    NULL,
    "Display the MissionLog.",
    EV_NORMAL
);
Event EV_Player_Teleport
(
    "tele",
    EV_CHEAT | EV_CONSOLE,
    "v",
    "location",
    "Teleport to location",
    EV_NORMAL
);
Event EV_Player_Face
(
    "face",
    EV_CHEAT | EV_CONSOLE,
    "v",
    "angles",
    "Force angles to specified vector",
    EV_NORMAL
);
Event EV_Player_Coord
(
    "coord",
    EV_CONSOLE,
    NULL,
    NULL,
    "Prints out current location and angles",
    EV_NORMAL
);
Event EV_Player_TestAnim
(
    "testplayeranim",
    EV_CHEAT,
    "fS",
    "weight anim",
    "Plays a test animation on the player",
    EV_NORMAL
);
Event EV_Player_StuffText
(
    "stufftext",
    EV_DEFAULT,
    "s",
    "stuffstrings",
    "Stuffs text to the player's console",
    EV_NORMAL
);
Event EV_Player_DMMessage
(
    "dmmessage",
    EV_CONSOLE,
    "is",
    "mode stuffstrings",
    "sends a DM message to the appropriate players",
    EV_NORMAL
);
Event EV_Player_IPrint
(
    "iprint",
    EV_CONSOLE,
    "sI",
    "string bold",
    "prints a string to the player,"
    "optionally in bold",
    EV_NORMAL
);
Event EV_SetViewangles
(
    "viewangles",
    EV_DEFAULT,
    "v",
    "newAngles",
    "set the view angles of the entity to newAngles.",
    EV_SETTER
);
Event EV_GetViewangles
(
    "viewangles",
    EV_DEFAULT,
    NULL,
    NULL,
    "get the angles of the entity.",
    EV_GETTER
);
Event EV_GetUseHeld
(
    "useheld",
    EV_DEFAULT,
    NULL,
    NULL,
    "returns 1 if this player is holding use,"
    "or 0 if he is not",
    EV_GETTER
);
Event EV_GetFireHeld
(
    "fireheld",
    EV_DEFAULT,
    NULL,
    NULL,
    "returns 1 if this player is holding fire,"
    "or 0 if he is not",
    EV_GETTER
);
Event EV_GetPrimaryFireHeld // Added in 2.30
    ("primaryfireheld",
     EV_DEFAULT,
     NULL,
     NULL,
     "returns 1 if this player is holding the primary fire, or 0 if not",
     EV_GETTER);
Event EV_GetSecondaryFireHeld // Added in 2.30
    ("secondaryfireheld",
     EV_DEFAULT,
     NULL,
     NULL,
     "returns 1 if this player is holding the secondary fire, or 0 if not",
     EV_GETTER);
Event EV_Player_GetReady
(
    "ready",
    EV_DEFAULT,
    NULL,
    NULL,
    "returns 1 if this player is ready,"
    "0 otherwise",
    EV_GETTER
);
Event EV_Player_SetReady
(
    "ready",
    EV_CONSOLE,
    NULL,
    NULL,
    "makes this player ready for the round to start",
    EV_NORMAL
);
Event EV_Player_SetNotReady
(
    "notready",
    EV_CONSOLE,
    NULL,
    NULL,
    "makes this player not ready for the round to start",
    EV_NORMAL
);
Event EV_Player_GetName
(
    "netname",
    EV_DEFAULT,
    NULL,
    NULL,
    "returns player's name",
    EV_GETTER
);
Event EV_Player_GetDMTeam
(
    "dmteam",
    EV_DEFAULT,
    NULL,
    NULL,
    "returns 'allies',"
    "'axis',"
    "'spectator',"
    "or 'freeforall'",
    EV_GETTER
);
Event EV_Player_SetViewModelAnim
(
    "viewmodelanim",
    EV_DEFAULT,
    "sI",
    "name force_restart",
    "Sets the player's view model animation.",
    EV_NORMAL
);
Event EV_Player_DMDeathDrop
(
    "dmdeathdrop",
    EV_DEFAULT,
    NULL,
    NULL,
    "Drops the player inventory in DM after's he's been killed",
    EV_NORMAL
);
Event EV_Player_Stopwatch
(
    "stopwatch",
    EV_DEFAULT,
    "i",
    "duration",
    "Starts a stopwatch for a given duration... use 0 to clear the stopwatch",
    EV_NORMAL
);
Event EV_Player_EnterIntermission
(
    "_enterintermission",
    EV_CODEONLY,
    NULL,
    NULL,
    "CODE USE ONLY",
    EV_NORMAL
);
Event EV_Player_SetPerferredWeapon
(
    "perferredweapon",
    EV_DEFAULT,
    "s",
    "weapon_name",
    "Overrides your perferred weapon that is displayed in the stats screen.",
    EV_NORMAL
);
Event EV_Player_SetVoiceType
(
    "voicetype",
    EV_DEFAULT,
    "s",
    "voice_name",
    "Sets the voice type to use the player.",
    EV_NORMAL
);

Event EV_Player_AddKills // Added in 2.0
    ("addkills", EV_DEFAULT, "i", "kills", "Give or take kills from the player", EV_NORMAL);

Event EV_Player_KillAxis // Added in 2.30
    ("killaxis",
     EV_CHEAT,
     "f",
     "radius",
     "Kills all of the axis that are in the passed radius, or all of them if radius is 0.",
     EV_NORMAL);
Event EV_Player_GetTurret // Added in 2.30
    ("turret",
     EV_DEFAULT,
     NULL,
     NULL,
     "Returns the turret the player is using. NULL if player isn't using a turret.",
     EV_GETTER);
Event EV_Player_GetVehicle // Added in 2.30
    ("vehicle",
     EV_DEFAULT,
     NULL,
     NULL,
     "Returns the vehicle the player is using. NULL if player isn't using a vehicle.",
     EV_GETTER);

////////////////////////////
//
// Openmohaa additions
//
////////////////////////////
Event EV_Player_AddDeaths
(
    "adddeaths",
    EV_DEFAULT,
    "i",
    "deaths",
    "adds deaths number to player",
    EV_NORMAL
);

Event EV_Player_AdminRights
(
    "adminrights",
    EV_DEFAULT,
    NULL,
    NULL,
    "returns client admin rights",
    EV_GETTER
);

Event EV_Player_BindWeap
(
    "bindweap",
    EV_DEFAULT,
    "ei",
    "weapon handnum",
    "binds weapon to player and sets him as weapon owner",
    EV_NORMAL
);

Event EV_Player_CanSwitchTeams
(
    "canswitchteams",
    EV_DEFAULT,
    "bB",
    "allow_team_change allow_spectate",
    "Specify if this player is allowed to switch teams and spectate. Doesn't override global canswitchteams.",
    EV_NORMAL
);

Event EV_Player_ClearCommand
(
    "clearcommand",
    EV_DEFAULT,
    "S",
    "command",
    "Clears any or a specific client command",
    EV_NORMAL
);

Event EV_Player_Dive
(
    "dive",
    EV_DEFAULT,
    "fF",
    "height airborne_duration",
    "Makes the player dive into prone position.",
    EV_NORMAL
);

Event EV_Player_Earthquake
(
    "earthquake2",
    EV_DEFAULT,
    "ffbbVF",
    "duration magnitude no_rampup no_rampdown location radius",
    "Create a smooth realistic earthquake for a player. Requires sv_reborn to be set.",
    EV_NORMAL
);

Event EV_Player_FreezeControls
(
    "freezecontrols",
    EV_DEFAULT,
    "b",
    "freeze_state",
    "Blocks or unblocks control input from this player.",
    EV_NORMAL
);

Event EV_Player_GetConnState
(
    "getconnstate",
    EV_DEFAULT,
    NULL,
    NULL,
    "gets connection state. [DEPRECATED]",
    EV_RETURN
);

Event EV_Player_GetDamageMultiplier
(
    "damage_multiplier",
    EV_DEFAULT,
    "Gets the current damage multiplier",
    NULL,
    NULL,
    EV_GETTER
);

Event EV_Player_GetKillHandler
(
    "killhandler",
    EV_DEFAULT,
    "s",
    "label",
    "Gets the player's current killed event handler. Returns NIL if no custom killhandler was set.",
    EV_GETTER
);

Event EV_Player_GetKills
(
    "getkills",
    EV_DEFAULT,
    NULL,
    NULL,
    "gets kills number of player",
    EV_RETURN
);

Event EV_Player_GetDeaths
(
    "getdeaths",
    EV_DEFAULT,
    NULL,
    NULL,
    "gets deaths number of player",
    EV_RETURN
);

Event EV_Player_GetLegsState
(
    "getlegsstate",
    EV_DEFAULT,
    NULL,
    NULL,
    "Gets the player's current legs state name",
    EV_RETURN
);

Event EV_Player_GetStateFile
(
    "statefile",
    EV_DEFAULT,
    NULL,
    NULL,
    "Gets the player's current state file.",
    EV_GETTER
);

Event EV_Player_GetTorsoState
(
    "gettorsostate",
    EV_DEFAULT,
    NULL,
    NULL,
    "Gets the player's current torso state name",
    EV_RETURN
);

Event EV_Player_HideEnt
(
    "hideent",
    EV_DEFAULT,
    "e",
    "entity",
    "Hides the specified entity to the player.",
    EV_NORMAL
);

Event EV_Player_Inventory
(
    "inventory",
    EV_DEFAULT,
    NULL,
    NULL,
    "returns player's inventory",
    EV_GETTER
);

Event EV_Player_InventorySet
(
    "inventory",
    EV_DEFAULT,
    "e",
    "array",
    "Set up the player's inventory",
    EV_SETTER
);

Event EV_Player_IsAdmin
(
    "isadmin",
    EV_DEFAULT,
    NULL,
    NULL,
    "checks if player is logged as admin",
    EV_RETURN
);

Event EV_Player_LeanLeftHeld
(
    "leanleftheld",
    EV_DEFAULT,
    NULL,
    NULL,
    "Returns 1 if this player is holding lean left key, or 0 if he is not",
    EV_GETTER
);

Event EV_Player_LeanRightHeld
(
    "leanrightheld",
    EV_DEFAULT,
    NULL,
    NULL,
    "returns EV_RETURN if this player is holding lean right key, or 0 if he is not",
    EV_GETTER
);

Event EV_Player_MoveSpeedScale
(
    "moveSpeedScale",
    EV_DEFAULT,
    "f",
    "speed",
    "Sets the player's speed multiplier (default 1.0).",
    EV_SETTER
);

Event EV_Player_MoveSpeedScaleGet
(
    "moveSpeedScale",
    EV_DEFAULT,
    NULL,
    NULL,
    "Gets the player's speed multiplier.",
    EV_GETTER
);

Event EV_Player_PlayLocalSound
(
    "playlocalsound",
    EV_DEFAULT,
    "sBF",
    "soundName loop time",
    "Plays a local sound to the player. The sound must be aliased globally. Requires sv_reborn to be set for stereo "
    "sounds.",
    EV_NORMAL
);

Event EV_Player_Replicate
(
    "replicate",
    EV_DEFAULT,
    "s",
    "variable",
    "Replicate a variable to the client (needs patch 1.12).",
    EV_NORMAL
);

Event EV_Player_RunHeld
(
    "runheld",
    EV_DEFAULT,
    NULL,
    NULL,
    "returns 1 if this player is holding run key,"
    "or 0 if he is not",
    EV_GETTER
);

Event EV_Player_SecFireHeld
(
    "secfireheld",
    EV_DEFAULT,
    NULL,
    NULL,
    "returns EV_RETURN if this player is holding secondary fire, or 0 if he is not",
    EV_GETTER
);

Event EV_Player_SetAnimSpeed
(
    "setanimspeed",
    EV_DEFAULT,
    "f",
    "speed",
    "set the player's animation speed multiplier (default 1.0).",
    EV_NORMAL
);

Event EV_Player_SetFov
(
    "setfov",
    EV_DEFAULT,
    "f",
    "fov",
    "set the player's fov (default 80).",
    EV_NORMAL
);

Event EV_Player_SetKillHandler
(
    "killhandler",
    EV_DEFAULT,
    "s",
    "label",
    "Replace the player's killed event by a new scripted handler. None or an empty string will revert to the default "
    "killed event handler.",
    EV_SETTER
);

Event EV_Player_SetClientFlag
(
    "setclientflag",
    EV_DEFAULT,
    "s",
    "name",
    "Calls a flag to the script client.",
    EV_NORMAL
);

Event EV_Player_SetEntityShader
(
    "setentshader",
    EV_DEFAULT,
    "es",
    "entity shadername",
    "Sets an entity shader for this player. An empty string will revert to the normal entity shader.",
    EV_NORMAL
);

Event EV_Player_SetLocalSoundRate
(
    "setlocalsoundrate",
    EV_DEFAULT,
    "sfF",
    "name rate time",
    "Sets the local sound rate.",
    EV_NORMAL
);

Event EV_Player_SetSpeed
(
    "setspeed",
    EV_DEFAULT,
    "fI",
    "speed index",
    "Sets the player's speed multiplier (default 1.0). Index specify which array value will be used (maximum 4).",
    EV_NORMAL
);

Event EV_Player_SetStateFile
(
    "statefile",
    EV_DEFAULT,
    "S",
    "statefile",
    "Sets the player's current state file (setting NIL, NULL or an empty string will revert to the global statefile).",
    EV_SETTER
);

Event EV_Player_SetTeam
(
    "setteam",
    EV_DEFAULT,
    "s",
    "team_name",
    "sets the player's team without respawning.\n"
    "Available team names are 'none', 'spectator', 'freeforall', 'axis' and 'allies'.",
    EV_NORMAL
);

Event EV_Player_SetViewModelAnimSpeed
(
    "setvmaspeed",
    EV_DEFAULT,
    "sf",
    "name speed",
    "Sets the player's animation speed when playing it.",
    EV_NORMAL
);

Event EV_Player_ShowEnt
(
    "showent",
    EV_DEFAULT,
    "e",
    "entity",
    "Shows the specified entity to the player.",
    EV_NORMAL
);

Event EV_Player_StopLocalSound
(
    "stoplocalsound",
    EV_DEFAULT,
    "sF",
    "soundName time",
    "Stops the specified sound.",
    EV_NORMAL
);

Event EV_Player_Userinfo
(
    "userinfo",
    EV_DEFAULT,
    NULL,
    NULL,
    "returns userinfo string",
    EV_GETTER
);

Event EV_Player_ViewModelGetAnim
(
    "viewmodelgetanim",
    EV_DEFAULT,
    "B",
    "fullanim",
    "Gets the player's current view model animation.",
    EV_RETURN
);

Event EV_Player_ViewModelAnimFinished
(
    "viewmodelanimfinished",
    EV_DEFAULT,
    NULL,
    NULL,
    "True if the player's current view model finished its animation.",
    EV_RETURN
);

Event EV_Player_ViewModelAnimValid
(
    "viewmodelanimvalid",
    EV_DEFAULT,
    "sB",
    "anim fullanim",
    "True if the view model animation is valid.",
    EV_RETURN
);

Event EV_Player_VisionSetBlur
(
    "visionsetblur",
    EV_DEFAULT,
    "fF",
    "level transition_time",
    "Sets the player's blur level. Level is a fraction from 0-1",
    EV_NORMAL
);

Event EV_Player_VisionGetNaked
(
    "visiongetnaked",
    EV_DEFAULT,
    NULL,
    NULL,
    "Gets the player's current naked-eye vision.",
    EV_RETURN
);

Event EV_Player_VisionSetNaked
(
    "visionsetnaked",
    EV_DEFAULT,
    "sFF",
    "vision_name transition_time phase",
    "Sets the player's naked-eye vision. Optionally give a transition time from the current vision. If vision_name is "
    "an empty string, it will revert to the current global vision.",
    EV_NORMAL
);

/*
==============================================================================

PLAYER

==============================================================================
*/

CLASS_DECLARATION(Sentient, Player, "player") {
    {&EV_Vehicle_Enter,                   &Player::EnterVehicle                 },
    {&EV_Vehicle_Exit,                    &Player::ExitVehicle                  },
    {&EV_Turret_Enter,                    &Player::EnterTurret                  },
    {&EV_Turret_Exit,                     &Player::ExitTurret                   },
    {&EV_Player_EndLevel,                 &Player::EndLevel                     },
    {&EV_Player_PrevItem,                 &Player::SelectPreviousItem           },
    {&EV_Player_NextItem,                 &Player::SelectNextItem               },
    {&EV_Player_PrevWeapon,               &Player::SelectPreviousWeapon         },
    {&EV_Player_NextWeapon,               &Player::SelectNextWeapon             },
    {&EV_Player_DropWeapon,               &Player::DropCurrentWeapon            },
    {&EV_Player_Reload,                   &Player::PlayerReload                 },
    {&EV_Player_CorrectWeaponAttachments, &Player::EventCorrectWeaponAttachments},
    {&EV_Player_GiveCheat,                &Player::GiveCheat                    },
    {&EV_Player_GiveWeaponCheat,          &Player::GiveWeaponCheat              },
    {&EV_Player_GiveAllCheat,             &Player::GiveAllCheat                 },
    {&EV_Player_GiveNewWeaponsCheat,      &Player::GiveNewWeaponsCheat          },
    {&EV_Player_DevGodCheat,              &Player::GodCheat                     },
    {&EV_Player_FullHeal,                 &Player::FullHeal                     },
    {&EV_Player_DevNoTargetCheat,         &Player::NoTargetCheat                },
    {&EV_Player_DevNoClipCheat,           &Player::NoclipCheat                  },
    {&EV_Player_GameVersion,              &Player::GameVersion                  },
    {&EV_Player_DumpState,                &Player::DumpState                    },
    {&EV_Player_ForceTorsoState,          &Player::ForceTorsoState              },
    {&EV_Player_ForceLegsState,           &Player::ForceLegsState               },
    {&EV_Player_Fov,                      &Player::EventSetSelectedFov          },
    {&EV_Kill,                            &Player::Kill                         },
    {&EV_Player_Dead,                     &Player::Dead                         },
    {&EV_Player_SpawnEntity,              &Player::SpawnEntity                  },
    {&EV_Player_SpawnActor,               &Player::SpawnActor                   },
    {&EV_Player_Respawn,                  &Player::Respawn                      },
    {&EV_Player_DoUse,                    &Player::DoUse                        },
    {&EV_Pain,                            &Player::Pain                         },
    {&EV_Killed,                          &Player::Killed                       },
    {&EV_GotKill,                         &Player::GotKill                      },
    {&EV_Player_TestThread,               &Player::TestThread                   },
    {&EV_Player_PowerupTimer,             &Player::SetPowerupTimer              },
    {&EV_Player_UpdatePowerupTimer,       &Player::UpdatePowerupTimer           },
    {&EV_Player_ResetState,               &Player::ResetState                   },
    {&EV_Player_WhatIs,                   &Player::WhatIs                       },
    {&EV_Player_ActorInfo,                &Player::ActorInfo                    },
    {&EV_Player_KillEnt,                  &Player::KillEnt                      },
    {&EV_Player_RemoveEnt,                &Player::RemoveEnt                    },
    {&EV_Player_KillClass,                &Player::KillClass                    },
    {&EV_Player_RemoveClass,              &Player::RemoveClass                  },
    {&EV_Player_AnimLoop_Legs,            &Player::EndAnim_Legs                 },
    {&EV_Player_AnimLoop_Torso,           &Player::EndAnim_Torso                },
    {&EV_Player_AnimLoop_Pain,            &Player::EndAnim_Pain                 },
    {&EV_Player_Jump,                     &Player::Jump                         },
    {&EV_Sentient_JumpXY,                 &Player::JumpXY                       },
    {&EV_Player_ListInventory,            &Player::ListInventoryEvent           },
    {&EV_Player_NextPainTime,             &Player::NextPainTime                 },
    {&EV_Player_Turn,                     &Player::Turn                         },
    {&EV_Player_TurnUpdate,               &Player::TurnUpdate                   },
    {&EV_Player_TurnLegs,                 &Player::TurnLegs                     },
    {&EV_Player_FinishUseAnim,            &Player::FinishUseAnim                },
    {&EV_Player_Holster,                  &Player::HolsterToggle                },
    {&EV_Player_SafeHolster,              &Player::Holster                      },
    {&EV_Player_SafeZoom,                 &Player::SafeZoomed                   },
    {&EV_Player_ZoomOff,                  &Player::ZoomOffEvent                 },
    {&EV_Player_StartUseObject,           &Player::StartUseObject               },
    {&EV_Player_FinishUseObject,          &Player::FinishUseObject              },
    {&EV_Player_WatchActor,               &Player::WatchActor                   },
    {&EV_Player_StopWatchingActor,        &Player::StopWatchingActor            },
    {&EV_Player_SetDamageMultiplier,      &Player::SetDamageMultiplier          },
    {&EV_Player_WaitForState,             &Player::WaitForState                 },
    {&EV_Player_LogStats,                 &Player::LogStats                     },
    {&EV_Player_TakePain,                 &Player::SetTakePain                  },
    {&EV_Player_SkipCinematic,            &Player::SkipCinematic                },
    {&EV_Player_ResetHaveItem,            &Player::ResetHaveItem                },
    {&EV_Show,                            &Player::PlayerShowModel              },
    {&EV_Player_ModifyHeight,             &Player::ModifyHeight                 },
    {&EV_Player_ModifyHeightFloat,        &Player::ModifyHeightFloat            },
    {&EV_Player_SetMovePosFlags,          &Player::SetMovePosFlags              },
    {&EV_Player_GetPosition,              &Player::GetPositionForScript         },
    {&EV_Player_GetMovement,              &Player::GetMovementForScript         },
    {&EV_Player_Teleport,                 &Player::EventTeleport                },
    {&EV_Player_Face,                     &Player::EventFace                    },
    {&EV_Player_Coord,                    &Player::EventCoord                   },
    {&EV_Player_TestAnim,                 &Player::EventTestAnim                },
    {&EV_Player_JailIsEscaping,           &Player::EventGetIsEscaping           },
    {&EV_Player_JailEscapeStop,           &Player::EventJailEscapeStop          },
    {&EV_Player_JailAssistEscape,         &Player::EventJailAssistEscape        },
    {&EV_Player_JailEscape,               &Player::EventJailEscape              },
    {&EV_Player_Score,                    &Player::Score                        },
    {&EV_Player_JoinDMTeam,               &Player::Join_DM_Team                 },
    {&EV_Player_AutoJoinDMTeam,           &Player::Auto_Join_DM_Team            },
    {&EV_Player_LeaveTeam,                &Player::Leave_DM_Team                },
    {&EV_Player_IsSpectator,              &Player::GetIsSpectator               },
    {&EV_Player_GetNationalityPrefix,     &Player::GetNationalityPrefix         },
    {&EV_Player_SetInJail,                &Player::EventSetInJail               },
    {&EV_Player_GetInJail,                &Player::EventGetInJail               },
    {&EV_Player_Spectator,                &Player::Spectator                    },
    {&EV_Player_PickWeapon,               &Player::PickWeaponEvent              },
    {&EV_Player_CallVote,                 &Player::CallVote                     },
    {&EV_Player_Vote,                     &Player::Vote                         },
    {&EV_Player_RetrieveVoteOptions,      &Player::RetrieveVoteOptions          },
    {&EV_Player_PrimaryDMWeapon,          &Player::EventPrimaryDMWeapon         },
    {&EV_Player_DeadBody,                 &Player::DeadBody                     },
    {&EV_Player_ArmWithWeapons,           &Player::ArmWithWeapons               },
    {&EV_Player_GetCurrentDMWeaponType,   &Player::EventGetCurrentDMWeaponType  },
    {&EV_Player_Physics_On,               &Player::PhysicsOn                    },
    {&EV_Player_Physics_Off,              &Player::PhysicsOff                   },
    {&EV_Player_AttachToLadder,           &Player::AttachToLadder               },
    {&EV_Player_UnattachFromLadder,       &Player::UnattachFromLadder           },
    {&EV_Player_TweakLadderPos,           &Player::TweakLadderPos               },
    {&EV_Player_EnsureOverLadder,         &Player::EnsureOverLadder             },
    {&EV_Player_EnsureForwardOffLadder,   &Player::EnsureForwardOffLadder       },
    {&EV_Damage,                          &Player::ArmorDamage                  },
    {&EV_Player_GetIsDisguised,           &Player::GetIsDisguised               },
    {&EV_Player_GetHasDisguise,           &Player::GetHasDisguise               },
    {&EV_Player_SetHasDisguise,           &Player::SetHasDisguise               },
    {&EV_Player_ObjectiveCount,           &Player::SetObjectiveCount            },
    {&EV_Player_Stats,                    &Player::Stats                        },
    {&EV_Player_StuffText,                &Player::EventStuffText               },
    {&EV_Player_DMMessage,                &Player::EventDMMessage               },
    {&EV_Player_IPrint,                   &Player::EventIPrint                  },
    {&EV_SetViewangles,                   &Player::SetViewangles                },
    {&EV_GetViewangles,                   &Player::GetViewangles                },
    {&EV_GetUseHeld,                      &Player::EventGetUseHeld              },
    {&EV_GetFireHeld,                     &Player::EventGetFireHeld             },
    {&EV_GetPrimaryFireHeld,              &Player::EventGetPrimaryFireHeld      },
    {&EV_GetSecondaryFireHeld,            &Player::EventGetSecondaryFireHeld    },
    {&EV_Player_GetReady,                 &Player::EventGetReady                },
    {&EV_Player_SetReady,                 &Player::EventSetReady                },
    {&EV_Player_SetNotReady,              &Player::EventSetNotReady             },
    {&EV_Player_GetDMTeam,                &Player::EventGetDMTeam               },
    {&EV_Player_GetName,                  &Player::EventGetNetName              },
    {&EV_Player_SetViewModelAnim,         &Player::EventSetViewModelAnim        },
    {&EV_Player_DMDeathDrop,              &Player::EventDMDeathDrop             },
    {&EV_Player_Stopwatch,                &Player::EventStopwatch               },
    {&EV_Player_EnterIntermission,        &Player::EventEnterIntermission       },
    {&EV_Player_SetPerferredWeapon,       &Player::EventSetPerferredWeapon      },
    {&EV_Player_SetVoiceType,             &Player::EventSetVoiceType            },
    {&EV_Player_AddKills,                 &Player::EventAddKills                },
    {&EV_Player_KillAxis,                 &Player::EventKillAxis                },
    {&EV_Player_GetTurret,                &Player::EventGetTurret               },
    {&EV_Player_GetVehicle,               &Player::EventGetVehicle              },

    {&EV_Player_AddDeaths,                &Player::AddDeaths                    },
    {&EV_Player_AdminRights,              &Player::AdminRights                  },
    {&EV_Player_BindWeap,                 &Player::BindWeap                     },
    {&EV_Player_CanSwitchTeams,           &Player::CanSwitchTeams               },
    {&EV_Player_ClearCommand,             &Player::ClearCommand                 },
    {&EV_Player_Dive,                     &Player::Dive                         },
    {&EV_Player_DMMessage,                &Player::EventDMMessage               },
    {&EV_Player_Earthquake,               &Player::EventEarthquake              },
    {&EV_Player_FreezeControls,           &Player::FreezeControls               },
    {&EV_Player_SetTeam,                  &Player::EventSetTeam                 },
    {&EV_Player_GetConnState,             &Player::GetConnState                 },
    {&EV_Player_GetDamageMultiplier,      &Player::GetDamageMultiplier          },
    {&EV_Player_GetDeaths,                &Player::GetDeaths                    },
    {&EV_Player_GetKillHandler,           &Player::GetKillHandler               },
    {&EV_Player_GetKills,                 &Player::GetKills                     },
    {&EV_Player_GetLegsState,             &Player::GetLegsState                 },
    {&EV_Player_GetStateFile,             &Player::GetStateFile                 },
    {&EV_Player_GetTorsoState,            &Player::GetTorsoState                },
    {&EV_Player_HideEnt,                  &Player::HideEntity                   },
    {&EV_Player_Inventory,                &Player::Inventory                    },
    {&EV_Player_InventorySet,             &Player::InventorySet                 },
    {&EV_Player_IsSpectator,              &Player::GetIsSpectator               },
    {&EV_Player_IsAdmin,                  &Player::IsAdmin                      },
    {&EV_Player_LeanLeftHeld,             &Player::LeanLeftHeld                 },
    {&EV_Player_LeanRightHeld,            &Player::LeanRightHeld                },
    {&EV_Player_MoveSpeedScale,           &Player::SetSpeed                     },
    {&EV_Player_MoveSpeedScaleGet,        &Player::GetMoveSpeedScale            },
    {&EV_Player_PlayLocalSound,           &Player::PlayLocalSound               },
    {&EV_Player_RunHeld,                  &Player::RunHeld                      },
    {&EV_Player_SecFireHeld,              &Player::SecFireHeld                  },
    {&EV_Player_SetAnimSpeed,             &Player::SetAnimSpeed                 },
    {&EV_Player_SetClientFlag,            &Player::SetClientFlag                },
    {&EV_Player_SetEntityShader,          &Player::SetEntityShader              },
    {&EV_Player_SetKillHandler,           &Player::SetKillHandler               },
    {&EV_Player_SetLocalSoundRate,        &Player::SetLocalSoundRate            },
    {&EV_Player_SetSpeed,                 &Player::SetSpeed                     },
    {&EV_Player_SetStateFile,             &Player::SetStateFile                 },
    {&EV_Player_SetViewModelAnimSpeed,    &Player::SetVMASpeed                  },
    {&EV_Player_ShowEnt,                  &Player::ShowEntity                   },
    {&EV_Player_Spectator,                &Player::Spectator                    },
    {&EV_Player_StopLocalSound,           &Player::StopLocalSound               },
    {&EV_Player_Userinfo,                 &Player::Userinfo                     },
    {&EV_Player_ViewModelAnimFinished,    &Player::EventGetViewModelAnimFinished},
    {&EV_Player_ViewModelGetAnim,         &Player::EventGetViewModelAnim        },
    {&EV_Player_ViewModelAnimValid,       &Player::EventGetViewModelAnimValid   },
    {&EV_Player_VisionGetNaked,           &Player::VisionGetNaked               },
    {&EV_Player_VisionSetBlur,            &Player::VisionSetBlur                },
    {&EV_Player_VisionSetNaked,           &Player::VisionSetNaked               },
    {NULL,                                NULL                                  }
};

movecontrolfunc_t Player::MoveStartFuncs[] = {
    NULL, // MOVECONTROL_USER,				// Quake style
    NULL, // MOVECONTROL_LEGS,				// Quake style, legs state system active
    NULL, // MOVECONTROL_USER_MOVEANIM,		// Quake style, legs state system active
    NULL, // MOVECONTROL_ANIM,				// move based on animation, with full collision testing
    NULL, // MOVECONTROL_ABSOLUTE,			// move based on animation, with full collision testing but no turning
    NULL, // MOVECONTROL_HANGING,				// move based on animation, with full collision testing, hanging
    NULL, // MOVECONTROL_ROPE_GRAB
    NULL, // MOVECONTROL_ROPE_RELEASE
    NULL, // MOVECONTROL_ROPE_MOVE
    NULL, // MOVECONTROL_PICKUPENEMY
    &Player::StartPush,        // MOVECONTROL_PUSH
    NULL,                      // MOVECONTROL_CLIMBWALL
    &Player::StartUseAnim,     // MOVECONTROL_USEANIM
    NULL,                      // MOVECONTROL_CROUCH
    &Player::StartLoopUseAnim, // MOVECONTROL_LOOPUSEANIM
    &Player::SetupUseObject,   // MOVECONTROL_USEOBJECT
    NULL,                      // MOVECONTROL_COOLOBJECT
};

Player::Player()
{
    //
    // set the entity type
    //
    entflags |= EF_PLAYER;

    mCurTrailOrigin   = 0;
    mLastTrailTime    = 0;
    m_pLastSpawnpoint = NULL;

    voted                      = false;
    m_fInvulnerableTimeElapsed = 0;
    m_voiceType                = PVT_NONE_SET;
    m_fTalkTime                = 0;
    num_deaths                 = 0;
    num_kills                  = 0;
    num_won_matches            = 0;
    num_lost_matches           = 0;
    num_team_kills             = 0;
    m_iLastNumTeamKills        = 0;
    m_bTempSpectator           = false;
    m_bSpectator               = false;
    m_bSpectatorSwitching      = false;
    m_bAllowFighting           = false;
    m_bReady                   = false;
    m_iPlayerSpectating        = 0;
    dm_team                    = TEAM_NONE;
    m_fTeamSelectTime          = -30;
    votecount                  = 0;
    m_fNextVoteOptionTime      = 0;
    m_fWeapSelectTime          = 0;

    fAttackerDispTime   = 0;
    m_iInfoClient       = 0;
    m_iInfoClientHealth = 0;
    m_fInfoClientTime   = 0;

    m_bDeathSpectator            = false;
    m_fSpawnTimeLeft             = 0;
    m_bWaitingForRespawn         = 0;
    m_bShouldRespawn             = false;
    last_camera_type             = -1;
    m_fLastInvulnerableTime      = 0;
    m_iInvulnerableTimeRemaining = 0;
    m_fLastVoteTime              = 0;

    if (LoadingSavegame) {
        return;
    }

    actor_camera_right          = false;
    starting_actor_camera_right = false;
    useanim_numloops            = 1;
    move_right_vel              = 0;

    m_iInfoClient       = -1;
    m_iInfoClientHealth = 0;
    m_fInfoClientTime   = 0.0;

    music_current_volume   = -1;
    music_saved_volume     = -1;
    music_volume_fade_time = -1;

    feetfalling        = false;
    edict->s.eType     = ET_PLAYER;
    buttons            = 0;
    new_buttons        = 0;
    server_new_buttons = 0;
    respawn_time       = -1.0;

    //
    // State
    //
    statemap_Legs      = NULL;
    statemap_Torso     = NULL;
    m_fPartBlends[0]   = 0;
    m_fPartBlends[1]   = 0;
    m_iPartSlot[legs]  = 0;
    m_iPartSlot[torso] = 2;
    partBlendMult[0]   = 0;
    partBlendMult[1]   = 0;
    m_fPainBlend       = 0;
    animdone_Pain      = false;

    m_fLastDeltaTime = level.time;

    camera         = NULL;
    atobject       = NULL;
    atobject_dist  = 0;
    toucheduseanim = NULL;
    useitem_in_use = NULL;

    damage_blood = 0;
    damage_count = 0;
    damage_from  = vec_zero;
    damage_alpha = 0;
    damage_yaw   = 0;

    fAttackerDispTime    = 0;
    pAttackerDistPointer = NULL;
    last_attack_button   = 0;
    attack_blocked       = false;
    canfall              = false;

    move_left_vel     = 0;
    move_right_vel    = 0;
    move_backward_vel = 0;
    move_forward_vel  = 0;
    move_up_vel       = 0;
    move_down_vel     = 0;

    moveresult                = 0;
    animspeed                 = 0;
    airspeed                  = 200.0f;
    weapons_holstered_by_code = false;
    actor_camera              = NULL;

    damage_multiplier = 1.0f;
    take_pain         = true;
    m_bIsInJail       = false;
    dm_team           = TEAM_NONE;
    current_team      = NULL;

    m_bTempSpectator      = false;
    m_bSpectator          = false;
    m_bSpectatorSwitching = false;
    m_bAllowFighting      = false;
    m_bReady              = true;
    m_fTeamSelectTime     = -30;
    m_fTalkTime           = 0;

    num_deaths            = 0;
    num_kills             = 0;
    num_team_kills        = 0;
    m_iLastNumTeamKills   = 0;
    num_won_matches       = 0;
    num_lost_matches      = 0;
    client->ps.voted      = false;
    votecount             = 0;
    m_fLastVoteTime       = 0;
    m_fNextVoteOptionTime = 0;
    m_fWeapSelectTime     = 0;
    m_jailstate           = JAILSTATE_NONE;

    SetSelectedFov(atof(Info_ValueForKey(client->pers.userinfo, "fov")));
    SetFov(selectedfov);

    m_iInZoomMode       = 0;
    m_iNumShotsFired    = 0;
    m_iNumHits          = 0;
    m_iNumGroinShots    = 0;
    m_iNumHeadShots     = 0;
    m_iNumLeftArmShots  = 0;
    m_iNumLeftLegShots  = 0;
    m_iNumRightArmShots = 0;
    m_iNumRightLegShots = 0;
    m_iNumTorsoShots    = 0;

    m_sPerferredWeaponOverride = "";

    SetTargetName("player");

    Init();

    for (int i = 0; i < MAX_TRAILS; i++) {
        mvTrail[i] = Vector(0, 0, 0);
    }

    for (int i = 0; i < MAX_TRAILS; i++) {
        mvTrailEyes[i] = Vector(0, 0, 0);
    }

    client->ps.pm_flags &= ~PMF_NO_HUD;

    m_fLastSprintTime = 0;
    m_bHasJumped      = false;

    m_fLastInvulnerableTime      = 0;
    m_iInvulnerableTimeRemaining = -1;
    m_fSpawnTimeLeft             = 0;
    m_bWaitingForRespawn         = false;
    m_bShouldRespawn             = false;
    m_bDeathSpectator            = false;

    m_vViewPos = vec_zero;

    //
    // Openmohaa additions
    //
    m_bShowingHint      = false;
    disable_spectate    = false;
    disable_team_change = false;
    m_bFrozen           = false;
    animDoneVM          = true;
    m_fVMAtime          = 0;
    m_fpsTiki           = NULL;

    for (int i = 0; i < MAX_SPEED_MULTIPLIERS; i++) {
        speed_multiplier[i] = 1.0f;
    }

    m_pKilledEvent = NULL;
    m_bConnected   = false;
}

Player::~Player()
{
    int          i, num;
    Conditional *cond;

    num = legs_conditionals.NumObjects();
    for (i = num; i > 0; i--) {
        cond = legs_conditionals.ObjectAt(i);
        delete cond;
    }

    num = torso_conditionals.NumObjects();
    for (i = num; i > 0; i--) {
        cond = torso_conditionals.ObjectAt(i);
        delete cond;
    }

    legs_conditionals.FreeObjectList();
    torso_conditionals.FreeObjectList();

    entflags &= ~EF_PLAYER;
}

static qboolean logfile_started = qfalse;

void Player::Init(void)
{
    InitClient();
    InitPhysics();
    InitPowerups();
    InitWorldEffects();
    InitSound();
    InitView();
    InitState();
    InitEdict();
    InitMaxAmmo();
    InitWeapons();
    InitInventory();
    InitHealth();
    InitStats();
    InitModel();
    InitInvulnerable();

    LoadStateTable();

    if (g_gametype->integer != GT_SINGLE_PLAYER) {
        InitDeathmatch();
    } else if (!LoadingSavegame) {
        ChooseSpawnPoint();
        JoinNearbySquads();
    }

    // make sure we put the player back into the world
    link();
    logfile_started = qfalse;

    // notify scripts for the spawning player
    parm.other = this;
    parm.owner = this;
    level.Unregister(STRING_PLAYERSPAWN);

    //
    // Added for openmohaa
    //
    if (!m_bConnected) {
        m_bConnected = true;

        Event *ev = new Event;
        ev->AddEntity(this);
        scriptedEvents[SE_CONNECTED].Trigger(ev);
    }

    Spawned();
}

void Player::InitStats(void)
{
    m_iNumObjectives       = 0;
    m_iObjectivesCompleted = 0;
    m_iNumHitsTaken        = 0;
    m_iNumEnemiesKilled    = 0;
    m_iNumObjectsDestroyed = 0;
}

void Player::InitEdict(void)
{
    // entity state stuff
    setSolidType(SOLID_BBOX);
    if (m_bSpectator) {
        //
        // 2.0: always noclip when spectating
        //
        setMoveType(MOVETYPE_NOCLIP);
    } else {
        setMoveType(MOVETYPE_WALK);
    }

    setSize(Vector(-16, -16, 0), Vector(16, 16, 72));

    edict->clipmask   = MASK_PLAYERSOLID;
    edict->r.ownerNum = ENTITYNUM_NONE;

    // clear entity state values
    edict->s.eFlags   = 0;
    edict->s.wasframe = 0;

    // players have precise shadows
    edict->s.renderfx |= RF_SHADOW_PRECISE | RF_SHADOW;
}

void Player::InitSound(void)

{
    //
    // reset the music
    //
    client->ps.current_music_mood  = mood_normal;
    client->ps.fallback_music_mood = mood_normal;
    ChangeMusic("normal", "normal", false);

    client->ps.music_volume           = 1.0;
    client->ps.music_volume_fade_time = 0.0;
    ChangeMusicVolume(1.0, 0.0);

    music_forced = false;

    // Reset the reverb stuff

    client->ps.reverb_type  = eax_generic;
    client->ps.reverb_level = 0;
    SetReverb(client->ps.reverb_type, client->ps.reverb_level);
}

void Player::InitClient(void)
{
    client_persistant_t saved;

    // deathmatch wipes most client data every spawn
    if (g_gametype->integer != GT_SINGLE_PLAYER) {
        char       userinfo[MAX_INFO_STRING];
        char       dm_primary[MAX_QPATH];
        float      enterTime   = client->pers.enterTime;
        teamtype_t team        = client->pers.teamnum;
        int        round_kills = client->pers.round_kills;

        memcpy(userinfo, client->pers.userinfo, sizeof(userinfo));
        memcpy(dm_primary, client->pers.dm_primary, sizeof(dm_primary));
        G_InitClientPersistant(client);
        G_ClientUserinfoChanged(edict, userinfo);

        memcpy(client->pers.dm_primary, dm_primary, sizeof(client->pers.dm_primary));
        client->pers.enterTime   = enterTime;
        client->pers.teamnum     = team;
        client->pers.round_kills = round_kills;
    }

    // clear everything but the persistant data and fov
    saved = client->pers;

    memset(client, 0, sizeof(*client));
    client->pers = saved;

    client->ps.clientNum   = client - game.clients;
    client->lastActiveTime = level.inttime;
    client->ps.commandTime = level.svsTime;

    SetStopwatch(0);

    m_bShowingHint = false;
}

void Player::InitState(void)
{
    gibbed       = false;
    pain         = 0;
    nextpaintime = 0;

    m_fMineDist      = 1000;
    m_fMineCheckTime = 0;
    m_sDmPrimary     = "";

    knockdown     = false;
    pain_dir      = PAIN_NONE;
    pain_type     = MOD_NONE;
    pain_location = -2;
    takedamage    = DAMAGE_AIM;
    deadflag      = DEAD_NO;
    flags &= ~FL_TEAMSLAVE;
    flags |= (FL_POSTTHINK | FL_THINK | FL_DIE_EXPLODE | FL_BLOOD);
    m_iMovePosFlags = MPF_POSITION_STANDING;

    if (!com_blood->integer) {
        flags &= ~(FL_DIE_EXPLODE | FL_BLOOD);
    }
}

void Player::InitHealth(void)

{
    static cvar_t *pMaxHealth = gi.Cvar_Get("g_maxplayerhealth", "250", 0);
    static cvar_t *pDMHealth  = gi.Cvar_Get("g_playerdmhealth", "100", 0);

    // Don't do anything if we're loading a server game.
    // This is either a loadgame or a restart
    if (LoadingSavegame) {
        return;
    }

    if (g_gametype->integer != GT_SINGLE_PLAYER) {
        if (pDMHealth->integer > 0) {
            max_health = pDMHealth->integer;
        } else {
            max_health = 100.f;
        }

        health = max_health;
    } else if (!g_realismmode->integer) {
        max_health = pMaxHealth->integer;
        health     = max_health;
    } else {
        // reset the health values
        health     = 100;
        max_health = health;
    }

    // 2.0:
    //  Make sure to clear the heal rate and the dead flag when respawning
    //
    m_fHealRate = 0;
    edict->s.eFlags &= ~EF_DEAD;
}

void Player::InitModel(void)
{
    // 2.0:
    //  Make sure to detach from any object before initializing
    //  To prevent any glitches
    RemoveFromVehiclesAndTurrets();
    UnattachFromLadder(NULL);

    gi.clearmodel(edict);

    if (g_gametype->integer == GT_SINGLE_PLAYER) {
        setModel("models/player/" + str(g_playermodel->string) + ".tik");
    } else if (dm_team == TEAM_AXIS) {
        if (Q_stricmpn(client->pers.dm_playermodel, "german", 6)
            && Q_stricmpn(client->pers.dm_playermodel, "axis", 4)
            //
            // 2.30 models
            //
            && Q_stricmpn(client->pers.dm_playergermanmodel, "it", 2)
            && Q_stricmpn(client->pers.dm_playergermanmodel, "sc", 2)) {
            setModel("models/player/german_wehrmacht_soldier.tik");
        } else {
            setModel("models/player/" + str(client->pers.dm_playergermanmodel) + ".tik");
        }
    } else {
        if (Q_stricmpn(client->pers.dm_playermodel, "american", 8)
            && Q_stricmpn(client->pers.dm_playermodel, "allied", 6)) {
            setModel("models/player/american_army.tik");
        } else {
            setModel("models/player/" + str(client->pers.dm_playermodel) + ".tik");
        }
    }

    //
    // Fallback to a default model if not found
    //
    if (!edict->tiki) {
        if (dm_team == TEAM_AXIS) {
            setModel("models/player/german_wehrmacht_soldier.tik");
        } else {
            setModel("models/player/american_army.tik");
        }
    }

    SetControllerTag(HEAD_TAG, gi.Tag_NumForName(edict->tiki, "Bip01 Head"));
    SetControllerTag(TORSO_TAG, gi.Tag_NumForName(edict->tiki, "Bip01 Spine2"));
    SetControllerTag(ARMS_TAG, gi.Tag_NumForName(edict->tiki, "Bip01 Spine1"));
    SetControllerTag(PELVIS_TAG, gi.Tag_NumForName(edict->tiki, "Bip01 Pelvis"));

    if (g_gametype->integer != GT_SINGLE_PLAYER && IsSpectator()) {
        hideModel();
    } else {
        showModel();
    }

    if (GetActiveWeapon(WEAPON_MAIN)) {
        // Show the arms
        edict->s.eFlags &= ~EF_UNARMED;
    } else {
        edict->s.eFlags |= EF_UNARMED;
    }

    edict->s.eFlags &= ~EF_ANY_TEAM;

    if (dm_team == TEAM_ALLIES) {
        edict->s.eFlags |= EF_ALLIES;
    } else if (dm_team == TEAM_AXIS) {
        edict->s.eFlags |= EF_AXIS;
    }

    G_SetClientConfigString(edict);

    client->ps.iViewModelAnim        = 0;
    client->ps.iViewModelAnimChanged = 0;

    if (g_protocol >= protocol_e::PROTOCOL_MOHTA_MIN) {
        if (dm_team == TEAM_AXIS) {
            if (m_voiceType <= PVT_AXIS_START || m_voiceType >= PVT_AXIS_END) {
                m_voiceType = PVT_AXIS_GERMAN;
            }
        } else {
            if (m_voiceType <= PVT_ALLIED_START || m_voiceType >= PVT_ALLIED_END) {
                m_voiceType = PVT_ALLIED_AMERICAN;
            }
        }
    } else {
        if (dm_team == TEAM_AXIS) {
            if (m_voiceType >= PVT_AXIS_END) {
                m_voiceType = PVT_AXIS_AXIS4;
            }
        } else {
            if (m_voiceType >= PVT_ALLIED_END) {
                m_voiceType = PVT_ALLIED_PILOT;
            }
        }
    }

    char  model_name[MAX_STRING_TOKENS];
    char *model_replace;

    strcpy(model_name, model.c_str());
    size_t len = strlen(model_name);

    model_replace = model_name + len - 4;

    strcpy(model_replace, "_fps.tik");

    m_fpsTiki = gi.modeltiki(model_name);
}

void Player::InitPhysics(void)
{
    // Physics stuff
    oldvelocity  = vec_zero;
    velocity     = vec_zero;
    old_v_angle  = v_angle;
    gravity      = 1.0;
    falling      = false;
    mediumimpact = false;
    hardimpact   = false;
    setContents(CONTENTS_BODY);
    mass = 500;
    memset(&last_ucmd, 0, sizeof(last_ucmd));

    client->ps.groundTrace.fraction = 1.0f;
}

void Player::InitPowerups(void)

{
    // powerups
    poweruptimer = 0;
    poweruptype  = 0;
}

void Player::InitWorldEffects(void)
{
    // world effects
    next_painsound_time = 0;
}

void Player::InitMaxAmmo(void)
{
    GiveAmmo("pistol", 0, 200);
    GiveAmmo("rifle", 0, 200);
    GiveAmmo("smg", 0, 300);
    GiveAmmo("mg", 0, 500);
    GiveAmmo("grenade", 0, 5);
    GiveAmmo("agrenade", 0, 5);
    GiveAmmo("heavy", 0, 5);
    GiveAmmo("shotgun", 0, 50);

    //
    // Team tactics ammunition
    //
    GiveAmmo("landmine", 0, 5);

    //
    // Team assault ammunition
    //
    GiveAmmo("smokegrenade", 0, 5);
    GiveAmmo("asmokegrenade", 0, 5);
    GiveAmmo("riflegrenade", 0, 3);
}

void Player::InitWeapons(void)

{
    // Don't do anything if we're loading a server game.
    // This is either a loadgame or a restart
    if (LoadingSavegame) {
        return;
    }
}

void Player::InitInventory(void) {}

void Player::InitView(void)
{
    // view stuff
    camera  = NULL;
    v_angle = vec_zero;
    SetViewAngles(v_angle);
    viewheight = DEFAULT_VIEWHEIGHT;

    // blend stuff
    damage_blend = vec_zero;
}

void Player::ChooseSpawnPoint(void)
{
    // set up the player's spawn location
    PlayerStart *p = SelectSpawnPoint(this);
    setOrigin(p->origin + Vector(0, 0, 1));
    origin.copyTo(edict->s.origin2);
    edict->s.renderfx |= RF_FRAMELERP;

    if (g_gametype->integer != GT_SINGLE_PLAYER && !IsSpectator()) {
        KillBox(this);
    }

    setAngles(p->angles);
    SetViewAngles(p->angles);
    SetupView();

    VectorCopy(origin, client->ps.vEyePos);
    client->ps.vEyePos[2] += client->ps.viewheight;

    if (g_gametype->integer != GT_SINGLE_PLAYER) {
        for (int i = 1; i <= 4; i++) {
            Event *ev = new Event(EV_SetViewangles);
            ev->AddVector(p->angles);
            PostEvent(ev, level.frametime * i);
        }
    }

    if (p->m_bDeleteOnSpawn) {
        delete p;
    } else {
        p->Unregister(STRING_SPAWN);
        m_pLastSpawnpoint = p;
    }
}

void Player::EndLevel(Event *ev)

{
    InitPowerups();
    if (health > max_health) {
        health = max_health;
    }

    if (health < 1) {
        health = 1;
    }
}

void Player::Respawn(Event *ev)
{
    if (g_gametype->integer != GT_SINGLE_PLAYER) {
        bool bOldVoted;

        if (health <= 0.0f) {
            DeadBody(NULL);
            hideModel();
        }

        respawn_time = level.time;

        // This is not present in MOHAA
        ProcessEvent(EV_Player_UnattachFromLadder);
        RemoveFromVehiclesAndTurrets();

        FreeInventory();

        // Save the previous vote value
        bOldVoted = client->ps.voted;
        Init();
        client->ps.voted = bOldVoted;
        client->ps.pm_flags |= PMF_RESPAWNED;

        SetInvulnerable();

        // Clear the center message
        gi.centerprintf(edict, " ");
        m_bShouldRespawn = false;
    } else {
        if (g_lastsave->string && *g_lastsave->string) {
            gi.SendConsoleCommand("loadlastgame\n");
        } else {
            gi.SendConsoleCommand("restart\n");
        }

        logfile_started = qfalse;
    }

    //
    // Added in openmohaa
    //
    Unregister(STRING_RESPAWN);
}

void Player::SetDeltaAngles(void)

{
    int i;

    // Use v_angle since we may be in a camera
    for (i = 0; i < 3; i++) {
        client->ps.delta_angles[i] = ANGLE2SHORT(v_angle[i]);
    }
}

void Player::Obituary(Entity *attacker, Entity *inflictor, int meansofdeath, int iLocation)
{
    str      s1;
    str      s2;
    qboolean bDispLocation;

    if (g_gametype->integer == GT_SINGLE_PLAYER) {
        return;
    }

    s1            = "x";
    s2            = "x";
    bDispLocation = qfalse;

    if (attacker == this) {
        //
        // Player killed themselves
        //
        switch (meansofdeath) {
        case MOD_SUICIDE:
            s1 = "took himself out of commision";
            break;
        case MOD_LAVA:
            s1 = "was burned to a crisp";
            break;
        case MOD_SLIME:
            s1 = "was melted to nothing";
            break;
        case MOD_FALLING:
            s1 = "cratered";
            break;
        case MOD_EXPLOSION:
            s1 = "blew himself up";
            break;
        case MOD_GRENADE:
            if (G_Random() >= 0.5f) {
                s1 = "played catch with himself";
            } else {
                s1 = "tripped on his own grenade";
            }
            break;
        case MOD_ROCKET:
            s1 = "rocketed himself";
            break;
        case MOD_BULLET:
            if (iLocation > -1) {
                s1 = "shot himself";
            } else {
                s1            = "shot himself in the";
                bDispLocation = qtrue;
            }
            break;
        case MOD_FAST_BULLET:
            if (iLocation == HITLOC_GENERAL || iLocation == HITLOC_MISS) {
                s1 = "shot himself";
            } else {
                s1            = "shot himself in the";
                bDispLocation = qtrue;
            }
            break;
        case MOD_LANDMINE:
            s1 = "was hoist on his own pitard";
            break;
        default:
            s1 = "died";
            break;
        }

        if (bDispLocation && g_obituarylocation->integer) {
            str szConv1 = s1 + str(" ") + G_LocationNumToDispString(iLocation);

            if (dedicated->integer) {
                gi.Printf("%s %s\n", client->pers.netname, gi.LV_ConvertString(szConv1.c_str()));
            }

            G_PrintDeathMessage(szConv1, s2.c_str(), "x", client->pers.netname, this, "s");
        } else {
            if (dedicated->integer) {
                gi.Printf("%s %s\n", client->pers.netname, gi.LV_ConvertString(s1.c_str()));
            }

            G_PrintDeathMessage(s1.c_str(), s2.c_str(), "x", client->pers.netname, this, "s");
        }
    } else if (attacker && attacker->client) {
        //
        // Killed by another player
        //
        Weapon *pAttackerWeap = NULL;

        if (attacker->IsSubclassOfPlayer()) {
            pAttackerWeap = ((Player *)attacker)->GetActiveWeapon(WEAPON_MAIN);
        }

        switch (meansofdeath) {
        case MOD_CRUSH:
        case MOD_CRUSH_EVERY_FRAME:
            s1 = "was crushed by";
            break;
        case MOD_TELEFRAG:
            s1 = "was telefragged by";
            break;
        case MOD_LAVA:
        case MOD_FIRE:
        case MOD_ON_FIRE:
            s1 = "was burned up by";
            break;
        case MOD_SLIME:
            s1 = "was melted by";
            break;
        case MOD_FALLING:
            s1 = "was pushed over the edge by";
            break;
        case MOD_EXPLOSION:
            s1 = "was blown away by";
            break;
        case MOD_GRENADE:
            if (G_Random() >= 0.5f) {
                s1 = "tripped on";
                s2 = "'s grenade";
            } else {
                s1 = "is picking";
                s2 = "'s shrapnel out of his teeth";
            }
            break;
        case MOD_ROCKET:
            s1 = "took";
            if (G_Random() >= 0.5f) {
                s2 = "'s rocket right in the kisser";
            } else {
                s2 = "'s rocket in the face";
            }
            break;
        case MOD_IMPACT:
            s1 = "was knocked out by";
            break;
        case MOD_BULLET:
        case MOD_FAST_BULLET:
            s1 = "was shot by";

            if (pAttackerWeap) {
                if (pAttackerWeap->GetWeaponClass() & WEAPON_CLASS_PISTOL) {
                    s1 = "was gunned down by";
                } else if (pAttackerWeap->GetWeaponClass() & WEAPON_CLASS_RIFLE) {
                    if (pAttackerWeap->GetZoom()) {
                        s1 = "was sniped by";
                    } else {
                        s1 = "was rifled by";
                    }
                } else if (pAttackerWeap->GetWeaponClass() & WEAPON_CLASS_SMG) {
                    s1 = "was perforated by";
                    s2 = "'s' SMG";
                } else if (pAttackerWeap->GetWeaponClass() & WEAPON_CLASS_MG) {
                    s1 = "was machine-gunned by";
                }
            }

            if (iLocation > -1) {
                bDispLocation = qtrue;
            }
            break;
        case MOD_VEHICLE:
            s1 = "was run over by";
            break;
        case MOD_IMPALE:
            s1 = "was impaled by";
            break;
        case MOD_BASH:
            if (G_Random() >= 0.5f) {
                s1 = "was bashed by";
            } else {
                s1 = "was clubbed by";
            }
            break;
        case MOD_SHOTGUN:
            if (G_Random() >= 0.5f) {
                s1 = "was hunted down by";
            } else {
                s1 = "was pumped full of buckshot by";
            }
            break;
        case MOD_LANDMINE:
            s1 = "stepped on";
            s2 = "'s landmine";
            break;
        default:
            s1 = "was killed by";
            break;
        }

        if (bDispLocation && g_obituarylocation->integer) {
            str szConv1 = s1 + str(" ") + G_LocationNumToDispString(iLocation);

            G_PrintDeathMessage(
                szConv1.c_str(), s2.c_str(), attacker->client->pers.netname, client->pers.netname, this, "p"
            );

            if (dedicated->integer) {
                str szLoc1, szLoc2;

                szLoc1 = gi.LV_ConvertString(szConv1.c_str());
                if (s2 == 'x') {
                    gi.Printf("%s %s %s\n", client->pers.netname, szLoc1.c_str(), attacker->client->pers.netname);
                } else {
                    szLoc2 = gi.LV_ConvertString(s2.c_str());
                    gi.Printf(
                        "%s %s %s%s\n",
                        client->pers.netname,
                        szLoc1.c_str(),
                        attacker->client->pers.netname,
                        szLoc2.c_str()
                    );
                }
            }
        } else {
            G_PrintDeathMessage(
                s1.c_str(), s2.c_str(), attacker->client->pers.netname, client->pers.netname, this, "p"
            );

            if (dedicated->integer) {
                str szLoc1, szLoc2;

                szLoc1 = gi.LV_ConvertString(s1.c_str());
                if (s2 == 'x') {
                    gi.Printf("%s %s %s\n", client->pers.netname, szLoc1.c_str(), attacker->client->pers.netname);
                } else {
                    szLoc2 = gi.LV_ConvertString(s2.c_str());
                    gi.Printf(
                        "%s %s %s%s\n",
                        client->pers.netname,
                        szLoc1.c_str(),
                        attacker->client->pers.netname,
                        szLoc2.c_str()
                    );
                }
            }
        }
    } else {
        //
        // No attacker and not self
        //
        switch (meansofdeath) {
        case MOD_LAVA:
            s1 = "was burned to a crisp";
            break;
        case MOD_SLIME:
            s1 = "was melted to nothing";
            break;
        case MOD_FALLING:
            s1 = "cratered";
            break;
        case MOD_EXPLOSION:
            s1 = "blew up";
            break;
        case MOD_GRENADE:
            s1 = "caught some shrapnel";
            break;
        case MOD_ROCKET:
            s1 = "caught a rocket";
            break;
        case MOD_BULLET:
        case MOD_FAST_BULLET:
            if (iLocation == HITLOC_GENERAL || iLocation == HITLOC_MISS) {
                s1 = "was shot";
            } else {
                s1            = "was shot in the";
                bDispLocation = qtrue;
            }
            break;
        case MOD_LANDMINE:
            s1 = "stepped on a land mine";
            break;
        default:
            s1 = "died";
            break;
        }

        if (bDispLocation && g_obituarylocation->integer) {
            str szConv1 = s1 + str(" ") + G_LocationNumToDispString(iLocation);

            G_PrintDeathMessage(szConv1.c_str(), s2.c_str(), "x", client->pers.netname, this, "w");

            if (dedicated->integer) {
                gi.Printf("%s %s\n", client->pers.netname, gi.LV_ConvertString(s1.c_str()));
            }
        } else {
            G_PrintDeathMessage(s1.c_str(), s2.c_str(), "x", client->pers.netname, this, "w");

            if (dedicated->integer) {
                gi.Printf("%s %s\n", client->pers.netname, gi.LV_ConvertString(s1.c_str()));
            }
        }
    }
}

void Player::Dead(Event *ev)
{
    if (deadflag == DEAD_DEAD) {
        return;
    }

    health   = 0;
    deadflag = DEAD_DEAD;

    edict->s.renderfx &= ~RF_SHADOW;
    server_new_buttons = 0;

    CancelEventsOfType(EV_Player_Dead);

    // stop animating
    StopPartAnimating(legs);

    // pause the torso anim
    PausePartAnim(torso);

    partAnim[torso] = "";

    if (m_fPainBlend != 0) {
        // Clear pain blend
        StopAnimating(ANIMSLOT_PAIN);
        edict->s.frameInfo[ANIMSLOT_PAIN].weight = 0;
        m_fPainBlend                             = 0;
        animdone_Pain                            = false;
    }

    if (g_gametype->integer != GT_SINGLE_PLAYER) {
        if (dmManager.AllowRespawn()) {
            respawn_time = level.time + 1.0f;
        } else {
            respawn_time = level.time + 2.0f;
        }
    } else if (level.current_map && *level.current_map) {
        G_BeginIntermission(level.current_map, TRANS_LEVEL);
    } else {
        respawn_time = level.time + 1.f;
    }

    ZoomOff();

    if (ShouldForceSpectatorOnDeath()) {
        m_bDeathSpectator = true;

        Spectator();
        SetPlayerSpectateRandom();
    }
}

void Player::Killed(Event *ev)
{
    Entity *attacker;
    Entity *inflictor;
    int     meansofdeath;
    int     location;
    Event  *event;

    //
    // This one is openmohaa-specific
    //  Custom killed event will do the job
    //
    if (m_pKilledEvent) {
        event = new Event(*m_pKilledEvent);
        for (int i = 1; i <= ev->NumArgs(); i++) {
            event->AddValue(ev->GetValue(i));
        }
        ProcessEvent(event);

        Unregister(STRING_DEATH);
        return;
    }

    if (g_gametype->integer != GT_SINGLE_PLAYER) {
        current_team->AddDeaths(this, 1);
    } else {
        AddDeaths(1);
    }

    attacker     = ev->GetEntity(1);
    inflictor    = ev->GetEntity(3);
    meansofdeath = ev->GetInteger(9);
    location     = ev->GetInteger(10);

    if (attacker && inflictor) {
        Obituary(attacker, inflictor, meansofdeath, location);
    }

    RemoveFromVehiclesAndTurrets();

    if (g_gametype->integer != GT_SINGLE_PLAYER && attacker && attacker->IsSubclassOfPlayer()) {
        ((Player *)attacker)->KilledPlayerInDeathmatch(this);
    }

    deadflag = DEAD_DYING;
    health   = 0;

    event = new Event(EV_Pain);

    event->AddEntity(attacker);
    event->AddFloat(ev->GetFloat(2));
    event->AddEntity(inflictor);
    event->AddVector(ev->GetVector(4));
    event->AddVector(ev->GetVector(5));
    event->AddVector(ev->GetVector(6));
    event->AddInteger(ev->GetInteger(7));
    event->AddInteger(ev->GetInteger(8));
    event->AddInteger(ev->GetInteger(9));
    event->AddInteger(ev->GetInteger(10));

    ProcessEvent(event);

    if (g_gametype->integer != GT_SINGLE_PLAYER) {
        if (HasItem("Binoculars")) {
            takeItem("Binoculars");
        }

        PostEvent(EV_Player_DMDeathDrop, 0.1f);
        edict->s.eFlags |= EF_DEAD;
    }

    edict->clipmask = MASK_DEADSOLID;
    setContents(CONTENTS_CORPSE);
    setSolidType(SOLID_NOT);
    setMoveType(MOVETYPE_TOSS);

    angles.x = 0;
    angles.z = 0;
    setAngles(angles);

    //
    // change music
    //
    ChangeMusic("failure", "normal", true);

    takedamage = DAMAGE_NO;

    // Post a dead event just in case
    PostEvent(EV_Player_Dead, 5.0f);
    ZoomOff();

    if (g_voiceChat->integer) {
        if (m_voiceType == PVT_ALLIED_MANON) {
            //
            // manon_death doesn't exist in 2.0 anymore.
            // The code is left just in case
            //
            Sound("manon_death", CHAN_VOICE, -1.0f, 160, NULL, -1.0f, 1, 0, 1, 1200);
        } else {
            Sound("player_death");
        }
    } else {
        Sound("player_death");
    }

    if (m_fPainBlend) {
        //
        // 2.0: No more pain animation after death
        //
        animdone_Pain = true;
    }

    //
    // Openmohaa scripted events
    //
    event = new Event;

    event->AddEntity(ev->GetEntity(1));
    event->AddFloat(ev->GetFloat(2));
    event->AddEntity(ev->GetEntity(3));
    event->AddVector(ev->GetVector(4));
    event->AddVector(ev->GetVector(5));
    event->AddVector(ev->GetVector(6));
    event->AddInteger(ev->GetInteger(7));
    event->AddInteger(ev->GetInteger(8));
    event->AddInteger(ev->GetInteger(9));
    event->AddInteger(ev->GetInteger(10));
    event->AddEntity(this);

    scriptedEvents[SE_KILL].Trigger(event);

    Unregister(STRING_DEATH);
}

void Player::EventDMDeathDrop(Event *ev)
{
    Weapon   *weapon = GetActiveWeapon(WEAPON_MAIN);
    SpawnArgs args;
    ClassDef *cls;

    if (!m_bDontDropWeapons && weapon && weapon->IsSubclassOfWeapon()) {
        weapon->Drop();
    }

    args.setArg("model", "models/items/dm_50_healthbox.tik");

    cls = args.getClassDef();
    if (cls) {
        Item *item = (Item *)cls->newInstance();
        if (item) {
            if (item->IsSubclassOfItem()) {
                item->setModel("models/items/dm_50_healthbox.tik");

                item->SetOwner(this);
                item->ProcessPendingEvents();
                item->Drop();
            } else {
                // useless and not pickupable, delete it
                delete item;
            }
        }
    }

    FreeInventory();
}

void Player::EventStopwatch(Event *ev)
{
    stopWatchType_t eType = SWT_NORMAL;

    int iDuration = ev->GetInteger(1);
    if (iDuration < 0) {
        ScriptError("duration < 0");
    }

    if (ev->NumArgs() > 1) {
        eType = static_cast<stopWatchType_t>(ev->GetInteger(2));
    } else {
        eType = SWT_NORMAL;
    }

    SetStopwatch(iDuration, eType);
}

void Player::SetStopwatch(int iDuration, stopWatchType_t type)
{
    int  iStartTime;
    char szCmd[256];

    if (g_protocol >= protocol_e::PROTOCOL_MOHTA_MIN) {
        if (type != SWT_NORMAL) {
            iStartTime = (int)(level.svsFloatTime * 1000.f);
        } else {
            iStartTime = 0;
            if (iDuration) {
                iStartTime = ceil(level.svsFloatTime * 1000.f);
            }
        }

        sprintf(szCmd, "stopwatch %i %i %i", iStartTime, iDuration, type);
    } else {
        iStartTime = 0;
        if (iDuration) {
            iStartTime = (int)level.svsFloatTime;
        }

        sprintf(szCmd, "stopwatch %i %i", iStartTime, iDuration);
    }

    gi.SendServerCommand(edict - g_entities, szCmd);
}

void Player::KilledPlayerInDeathmatch(Player *killed)
{
    DM_Team *pDMTeam;

    pDMTeam = killed->GetDM_Team();

    if (killed == this) {
        pDMTeam->AddKills(this, -1);
        gi.SendServerCommand(
            edict - g_entities, "print \"" HUD_MESSAGE_WHITE "%s\n\"", gi.LV_ConvertString("You killed yourself")
        );
    } else {
        if (pDMTeam != GetDM_Team() || g_gametype->integer <= GT_FFA || g_gametype->integer >= GT_MAX_GAME_TYPE) {
            current_team->AddKills(this, 1);
        } else {
            //
            // A teammate was killed
            //
            current_team->AddKills(this, -1);

            num_team_kills++;
        }

        gi.SendServerCommand(
            edict - g_entities,
            "print \"" HUD_MESSAGE_WHITE "%s %s\n\"",
            gi.LV_ConvertString("You killed"),
            killed->client->pers.netname
        );
    }
}

void Player::Pain(Event *ev)
{
    float   damage, yawdiff;
    Entity *attacker;
    int     meansofdeath;
    Vector  dir, pos, attack_angle;
    int     iLocation;

    attacker     = ev->GetEntity(1);
    damage       = ev->GetFloat(2);
    pos          = ev->GetVector(4);
    dir          = ev->GetVector(5);
    meansofdeath = ev->GetInteger(9);
    iLocation    = ev->GetInteger(10);

    if (!damage && !knockdown) {
        return;
    }

    client->ps.stats[STAT_LAST_PAIN] = damage;

    // Determine direction
    attack_angle = dir.toAngles();
    yawdiff      = angles[YAW] - attack_angle[YAW] + 180;
    yawdiff      = AngleNormalize180(yawdiff);

    if (yawdiff > -45 && yawdiff < 45) {
        pain_dir = PAIN_FRONT;
    } else if (yawdiff < -45 && yawdiff > -135) {
        pain_dir = PAIN_LEFT;
    } else if (yawdiff > 45 && yawdiff < 135) {
        pain_dir = PAIN_RIGHT;
    } else {
        pain_dir = PAIN_REAR;
    }

    pain_type     = (meansOfDeath_t)meansofdeath;
    pain_location = iLocation;

    // Only set the regular pain level if enough time since last pain has passed
    if ((level.time > nextpaintime) && take_pain) {
        pain = damage;
    }

    // add to the damage inflicted on a player this frame
    // the total will be turned into screen blends and view angle kicks
    // at the end of the frame
    damage_blood += damage;
    damage_from += dir * damage;
    damage_yaw = dir.toYaw() * 10.0f;

    if (damage_yaw == client->ps.stats[STAT_DAMAGEDIR]) {
        if (damage_yaw < 1800.0f) {
            damage_yaw += 1.0f;
        } else {
            damage_yaw -= 1.0f;
        }
    }

    if (g_gametype->integer != GT_SINGLE_PLAYER && attacker && attacker->client && attacker != this) {
        gi.MSG_SetClient(attacker->edict - g_entities);
        if (IsDead()) {
            gi.MSG_StartCGM(BG_MapCGMToProtocol(g_protocol, CGM_NOTIFY_KILL));
        } else {
            gi.MSG_StartCGM(BG_MapCGMToProtocol(g_protocol, CGM_NOTIFY_HIT));
        }
        gi.MSG_EndCGM();
    }

    if (IsDead()) {
        return;
    }

    if (g_voiceChat->integer) {
        if (m_voiceType == PVT_ALLIED_MANON) {
            //
            // Should have been removed since 2.0
            //
            Sound("manon_pain", CHAN_DIALOG, -1, 160, NULL, -1, 1, 0, 1, 1200);
        } else {
            Sound("player_pain");
        }
    } else {
        Sound("player_pain");
    }
}

void Player::DoUse(Event *ev)
{
    gentity_t *hit;
    int        touch[MAX_GENTITIES];
    int        num;
    int        i;

    if (g_gametype->integer != GT_SINGLE_PLAYER && IsSpectator()) {
        // Prevent using stuff while spectating
        return;
    }

    if (IsDead()) {
        // Dead players mustn't use
        return;
    }

    if (m_pVehicle || m_pTurret) {
        RemoveFromVehiclesAndTurretsInternal();
        return;
    }

    if ((buttons & BUTTON_ATTACKLEFT) || (buttons & BUTTON_ATTACKRIGHT)) {
        //
        // Added in 2.0
        //  Only allow use if the player isn't holding attack buttons
        //
        return;
    }

    num = getUseableEntities(touch, MAX_GENTITIES, true);

    for (i = 0; i < num; i++) {
        hit = &g_entities[touch[i]];

        if (!hit->inuse) {
            continue;
        }

        Event *event = new Event(EV_Use);
        event->AddListener(this);

        hit->entity->ProcessEvent(event);

        if (m_pVehicle || m_pTurret) {
            break;
        }
    }

    if (i < num && m_pVehicle) {
        //
        // Added in 2.30
        //  Make the vehicle also invincible if the player is invincible
        //
        if (flags & FL_GODMODE) {
            m_pVehicle->flags |= FL_GODMODE;
        } else {
            m_pVehicle->flags &= ~FL_GODMODE;
        }
    }
}

void Player::TouchStuff(pmove_t *pm)
{
    gentity_t *other;
    Event     *event;
    int        i;
    int        j;

    //
    // clear out any conditionals that are controlled by touching
    //
    toucheduseanim = NULL;

    if (getMoveType() != MOVETYPE_NOCLIP) {
        G_TouchTriggers(this);
    }

    // touch other objects
    for (i = 0; i < pm->numtouch; i++) {
        other = &g_entities[pm->touchents[i]];

        for (j = 0; j < i; j++) {
            gentity_t *ge = &g_entities[j];

            if (ge == other) {
                break;
            }
        }

        if (j != i) {
            // duplicated
            continue;
        }

        // Don't bother touching the world
        if ((!other->entity) || (other->entity == world)) {
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

void Player::GetMoveInfo(pmove_t *pm)
{
    moveresult = pm->moveresult;

    if (!deadflag || (g_gametype->integer != GT_SINGLE_PLAYER && IsSpectator())) {
        v_angle[0] = pm->ps->viewangles[0];
        v_angle[1] = pm->ps->viewangles[1];
        v_angle[2] = pm->ps->viewangles[2];

        if (moveresult == MOVERESULT_TURNED) {
            angles.y = v_angle[1];
            setAngles(angles);
            SetViewAngles(angles);
        }
    }

    setOrigin(Vector(pm->ps->origin[0], pm->ps->origin[1], pm->ps->origin[2]));

    if (pm->ps->groundEntityNum != ENTITYNUM_NONE) {
        float backoff;
        float change;
        int   i;

        backoff = DotProduct(pm->ps->groundTrace.plane.normal, pm->ps->velocity);

        for (i = 0; i < 3; i++) {
            change = pm->ps->groundTrace.plane.normal[i] * backoff;
            pm->ps->velocity[i] -= change;
        }
    }

    // Set the ground entity
    groundentity = NULL;
    if (pm->ps->groundEntityNum != ENTITYNUM_NONE) {
        groundentity = &g_entities[pm->ps->groundEntityNum];
        airspeed     = 200;

        if (!groundentity->entity || groundentity->entity->getMoveType() == MOVETYPE_NONE) {
            m_vPushVelocity = vec_zero;
        }

        //
        // Openmohaa fix
        //  Disable predictions when the groundentity is moving up/down, looks like shaky otherwise
        if (groundentity->entity && groundentity->entity != this && groundentity->entity->velocity[2] != 0) {
            pm->ps->pm_flags |= PMF_NO_PREDICTION;
        }
    }

    velocity = Vector(pm->ps->velocity[0], pm->ps->velocity[1], pm->ps->velocity[2]);

    if ((client->ps.pm_flags & PMF_FROZEN) || (client->ps.pm_flags & PMF_NO_MOVE)) {
        velocity = vec_zero;
    } else {
        setSize(pm->mins, pm->maxs);
        viewheight = pm->ps->viewheight;
    }

    // water type and level is set in the predicted code
    waterlevel = pm->waterlevel;
    watertype  = pm->watertype;
}

void Player::SetMoveInfo(pmove_t *pm, usercmd_t *ucmd)
{
    Vector move;

    // set up for pmove
    memset(pm, 0, sizeof(pmove_t));

    velocity.copyTo(client->ps.velocity);

    pm->ps = &client->ps;

    if (ucmd) {
        pm->cmd = *ucmd;
    }

    if (sv_drawtrace->integer <= 1) {
        pm->trace = gi.trace;
    } else {
        pm->trace = &G_PMDrawTrace;
    }

    pm->tracemask     = MASK_PLAYERSOLID;
    pm->pointcontents = gi.pointcontents;

    pm->ps->origin[0] = origin.x;
    pm->ps->origin[1] = origin.y;
    pm->ps->origin[2] = origin.z;

    /*
    pm->mins[0] = mins.x;
    pm->mins[1] = mins.y;
    pm->mins[2] = mins.z;

    pm->maxs[0] = maxs.x;
    pm->maxs[1] = maxs.y;
    pm->maxs[2] = maxs.z;
    */

    pm->ps->velocity[0] = velocity.x;
    pm->ps->velocity[1] = velocity.y;
    pm->ps->velocity[2] = velocity.z;

    pm->pmove_fixed = pmove_fixed->integer;
    pm->pmove_msec  = pmove_msec->integer;

    if (pmove_msec->integer < 8) {
        pm->pmove_msec = 8;
    } else if (pmove_msec->integer > 33) {
        pm->pmove_msec = 33;
    }

    if (g_protocol >= protocol_e::PROTOCOL_MOHTA_MIN) {
        if (g_gametype->integer != GT_SINGLE_PLAYER) {
            //
            // Added in 2.0
            // In multiplayer mode, specify if the player can lean while moving
            //
            if (dmflags->integer & DF_ALLOW_LEAN_MOVEMENT) {
                pm->alwaysAllowLean = qtrue;
            } else {
                pm->alwaysAllowLean = qfalse;
            }
        } else {
            pm->alwaysAllowLean = qfalse;
        }

        pm->leanMax          = 45.f;
        pm->leanAdd          = 6.f;
        pm->leanRecoverSpeed = 8.5f;
        pm->leanSpeed        = 2.f;
    } else {
        pm->alwaysAllowLean = qtrue;

        pm->leanMax          = 40.f;
        pm->leanAdd          = 10.f;
        pm->leanRecoverSpeed = 15.f;
        pm->leanSpeed        = 4.f;
    }

    pm->protocol = g_protocol;
}

pmtype_t Player::GetMovePlayerMoveType(void)
{
    if (getMoveType() == MOVETYPE_NOCLIP || IsSpectator()) {
        return PM_NOCLIP;
    } else if (deadflag) {
        return PM_DEAD;
    } else if (movecontrol == MOVECONTROL_CLIMBWALL) {
        return PM_CLIMBWALL;
    } else {
        return PM_NORMAL;
    }
}

void Player::CheckGround(void)
{
    pmove_t pm;

    SetMoveInfo(&pm, current_ucmd);
    Pmove_GroundTrace(&pm);
    GetMoveInfo(&pm);
}

qboolean Player::AnimMove(Vector& move, Vector *endpos)
{
    Vector  up;
    Vector  down;
    trace_t trace;
    int     mask;
    Vector  start(origin);
    Vector  end(origin + move);

    mask = MASK_PLAYERSOLID;

    // test the player position if they were a stepheight higher
    trace = G_Trace(start, mins, maxs, end, this, mask, true, "AnimMove");
    if (trace.fraction < 1) {
        if ((movecontrol == MOVECONTROL_HANGING) || (movecontrol == MOVECONTROL_CLIMBWALL)) {
            up = origin;
            up.z += move.z;
            trace = G_Trace(origin, mins, maxs, up, this, mask, true, "AnimMove");
            if (trace.fraction < 1) {
                if (endpos) {
                    *endpos = origin;
                }
                return qfalse;
            }

            origin = trace.endpos;
            end    = origin;
            end.x += move.x;
            end.y += move.y;

            trace = G_Trace(origin, mins, maxs, end, this, mask, true, "AnimMove");
            if (endpos) {
                *endpos = trace.endpos;
            }

            return (trace.fraction > 0);
        } else {
            return TestMove(move, endpos);
        }
    } else {
        if (endpos) {
            *endpos = trace.endpos;
        }

        return qtrue;
    }
}

qboolean Player::TestMove(Vector& move, Vector *endpos)

{
    trace_t trace;
    Vector  pos(origin + move);

    trace = G_Trace(origin, mins, maxs, pos, this, MASK_PLAYERSOLID, true, "TestMove");
    if (trace.allsolid) {
        // player is completely trapped in another solid
        if (endpos) {
            *endpos = origin;
        }
        return qfalse;
    }

    if (trace.fraction < 1.0f) {
        Vector up(origin);
        up.z += STEPSIZE;

        trace = G_Trace(origin, mins, maxs, up, this, MASK_PLAYERSOLID, true, "TestMove");
        if (trace.fraction == 0.0f) {
            if (endpos) {
                *endpos = origin;
            }
            return qfalse;
        }

        Vector temp(trace.endpos);
        Vector end(temp + move);

        trace = G_Trace(temp, mins, maxs, end, this, MASK_PLAYERSOLID, true, "TestMove");
        if (trace.fraction == 0.0f) {
            if (endpos) {
                *endpos = origin;
            }
            return qfalse;
        }

        temp = trace.endpos;

        Vector down(trace.endpos);
        down.z = origin.z;

        trace = G_Trace(temp, mins, maxs, down, this, MASK_PLAYERSOLID, true, "TestMove");
    }

    if (endpos) {
        *endpos = trace.endpos;
    }

    return qtrue;
}

float Player::TestMoveDist(Vector& move)

{
    Vector endpos;

    TestMove(move, &endpos);
    endpos -= origin;

    return endpos.length();
}

static Vector vec_up = Vector(0, 0, 1);

void Player::CheckMoveFlags(void)
{
    trace_t trace;
    Vector  start;
    Vector  end;
    float   oldsp;
    Vector  olddir(oldvelocity.x, oldvelocity.y, 0);

    //
    // Check if moving forward will cause the player to fall
    //
    start = origin + yaw_forward * 52.0f;
    end   = start;
    end.z -= STEPSIZE * 2;

    trace   = G_Trace(start, mins, maxs, end, this, MASK_PLAYERSOLID, true, "CheckMoveFlags");
    canfall = (trace.fraction >= 1.0f);

    if (!groundentity && !(client->ps.walking)) {
        falling      = true;
        hardimpact   = false;
        mediumimpact = false;
    } else {
        falling      = false;
        mediumimpact = oldvelocity.z <= -180.0f;
        hardimpact   = oldvelocity.z < -400.0f;
    }

    // check for running into walls
    oldsp = VectorNormalize(olddir);
    if ((oldsp > 220.0f) && (velocity * olddir < 2.0f)) {
        moveresult = MOVERESULT_HITWALL;
    }

    move_forward_vel  = DotProduct(yaw_forward, velocity);
    move_backward_vel = -move_forward_vel;

    if (move_forward_vel < 0.0f) {
        move_forward_vel = 0.0f;
    }

    if (move_backward_vel < 0.0f) {
        move_backward_vel = 0.0f;
    }

    move_left_vel  = DotProduct(yaw_left, velocity);
    move_right_vel = -move_left_vel;

    if (move_left_vel < 0.0f) {
        move_left_vel = 0.0f;
    }

    if (move_right_vel < 0.0f) {
        move_right_vel = 0.0f;
    }

    move_up_vel   = DotProduct(vec_up, velocity);
    move_down_vel = -move_up_vel;

    if (move_up_vel < 0.0f) {
        move_up_vel = 0.0f;
    }

    if (move_down_vel < 0.0f) {
        move_down_vel = 0.0f;
    }
}

qboolean Player::CheckMove(Vector& move, Vector *endpos)
{
    return AnimMove(move, endpos);
}

float Player::CheckMoveDist(Vector& move)

{
    Vector endpos;

    CheckMove(move, &endpos);
    endpos -= origin;

    return endpos.length();
}

void Player::ClientMove(usercmd_t *ucmd)
{
    pmove_t pm;
    Vector  move;

#if 0
    int  touch[MAX_GENTITIES];
    int  num        = getUseableEntities(touch, MAX_GENTITIES, true);
    bool bHintShown = false;

    for (int i = 0; i < num; i++) {
        Entity *entity = g_entities[touch[i]].entity;
        if (entity && entity->m_HintString.length()) {
            entity->ProcessHint(edict, true);
            bHintShown     = true;
            m_bShowingHint = true;
            break;
        }
    }

    if (!bHintShown && m_bShowingHint) {
        m_bShowingHint = false;

        // FIXME: delete
        if (sv_specialgame->integer)
        {
            gi.MSG_SetClient(edict - g_entities);

            // Send the hint string once
            gi.MSG_StartCGM(CGM_HINTSTRING);
                gi.MSG_WriteString("");
            gi.MSG_EndCGM();
        }
    }
#endif

    oldorigin = origin;

    client->ps.pm_type = GetMovePlayerMoveType();
    // set move flags
    client->ps.pm_flags &=
        ~(PMF_FROZEN | PMF_NO_PREDICTION | PMF_NO_MOVE | PMF_DUCKED | PMF_NO_GRAVITY | PMF_VIEW_PRONE
          | PMF_VIEW_DUCK_RUN | PMF_VIEW_JUMP_START);

    if (level.playerfrozen || m_bFrozen) {
        client->ps.pm_flags |= PMF_FROZEN;
    }

    if ((flags & FL_IMMOBILE) || (flags & FL_PARTIAL_IMMOBILE)) {
        client->ps.pm_flags |= PMF_NO_MOVE;
        client->ps.pm_flags |= PMF_NO_PREDICTION;
    }

    if (m_pGlueMaster) {
        //
        // Added in 2.0.
        // Disable movement prediction/movement if the player is glued to something
        //
        client->ps.pm_flags |= PMF_NO_PREDICTION;
        client->ps.pm_flags |= PMF_NO_MOVE;
    }

    /*
    if (maxs.z == 60.0f) {
        client->ps.pm_flags |= PMF_DUCKED;
    } else if (maxs.z == 54.0f) {
        client->ps.pm_flags |= PMF_DUCKED | PMF_VIEW_PRONE;
    } else if (maxs.z == 20.0f) {
        client->ps.pm_flags |= PMF_VIEW_PRONE;
    } else if (maxs.z == 53.0f) {
        client->ps.pm_flags |= PMF_VIEW_DUCK_RUN;
    } else if (viewheight == 52) {
        client->ps.pm_flags |= PMF_VIEW_JUMP_START;
    }
    */
    if (maxs.z == 54.0f || maxs.z == 60.0f) {
        client->ps.pm_flags |= PMF_DUCKED;
    } else if (viewheight == 52) {
        client->ps.pm_flags |= PMF_VIEW_JUMP_START;
    }

    switch (movecontrol) {
    case MOVECONTROL_USER:
    case MOVECONTROL_LEGS:
    case MOVECONTROL_USER_MOVEANIM:
        break;

    case MOVECONTROL_CROUCH:
        client->ps.pm_flags |= PMF_NO_PREDICTION | PMF_DUCKED | PMF_VIEW_PRONE;
        break;

    default:
        client->ps.pm_flags |= PMF_NO_PREDICTION;
    }

    if (movetype == MOVETYPE_NOCLIP) {
        if (!(last_ucmd.buttons & BUTTON_RUN)) {
            client->ps.speed = sv_runspeed->value * sv_walkspeedmult->value;
        } else {
            client->ps.speed = sv_runspeed->value;
        }
    } else if (!groundentity) {
        client->ps.speed = airspeed;
    } else {
        Weapon *pWeap;

        if (last_ucmd.buttons & BUTTON_RUN) {
            client->ps.speed = GetRunSpeed();
        } else {
            client->ps.speed = sv_runspeed->value * sv_walkspeedmult->value;
        }

        if (m_iMovePosFlags & MPF_POSITION_CROUCHING) {
            client->ps.speed = (float)client->ps.speed * sv_crouchspeedmult->value;
        }

        pWeap = GetActiveWeapon(WEAPON_MAIN);
        if (pWeap) {
            //
            // Also use the weapon movement speed
            //
            if (!IsZoomed()) {
                client->ps.speed = (float)client->ps.speed * pWeap->m_fMovementSpeed;
            } else {
                client->ps.speed = (float)client->ps.speed * pWeap->m_fZoomMovement * pWeap->m_fMovementSpeed;
            }
        }
    }

    if (g_gametype->integer != GT_SINGLE_PLAYER) {
        client->ps.speed = (int)((float)client->ps.speed * sv_dmspeedmult->value);
    }

    //====
    // Openmohaa addition
    for (int i = 0; i < MAX_SPEED_MULTIPLIERS; i++) {
        client->ps.speed = (int)((float)client->ps.speed * speed_multiplier[i]);
    }
    //====

    client->ps.gravity = sv_gravity->value * gravity;

    if ((movecontrol != MOVECONTROL_ABSOLUTE) && (movecontrol != MOVECONTROL_PUSH)
        && (movecontrol != MOVECONTROL_CLIMBWALL)) {
        Vector oldpos(origin);

        SetMoveInfo(&pm, ucmd);
        Pmove(&pm);
        GetMoveInfo(&pm);

        if (g_gametype->integer != GT_SINGLE_PLAYER && groundentity && groundentity->entity
            && groundentity->entity->IsSubclassOfSentient()) {
            //
            // Added in 2.0
            // If the player is on another sentient, try to make it fall off
            //
            velocity -= Vector(orientation[0]) * (random() * 20.f);
            velocity -= Vector(orientation[1]) * (random() * 10.f);
        }

        ProcessPmoveEvents(pm.pmoveEvent);

        // if we're not moving, set the blocked flag in case the user is trying to move
        if ((ucmd->forwardmove || ucmd->rightmove) && ((oldpos - origin).length() < 0.005f)) {
            moveresult = MOVERESULT_BLOCKED;
        }
        if (client->ps.walking && moveresult >= MOVERESULT_BLOCKED) {
            setOrigin(oldpos);
            VectorCopy(origin, client->ps.origin);
        }
    } else {
        if (movecontrol == MOVECONTROL_CLIMBWALL) {
            PM_UpdateViewAngles(&client->ps, ucmd);
            v_angle = client->ps.viewangles;
        } else if (!deadflag) {
            v_angle = client->ps.viewangles;
        }

        // should collect objects to touch against
        memset(&pm, 0, sizeof(pmove_t));

        // keep the command time up to date or else the next PMove we run will try to catch up
        client->ps.commandTime = ucmd->serverTime;

        velocity = vec_zero;
    }

    if ((getMoveType() != MOVETYPE_NOCLIP) && (client->ps.pm_flags & PMF_NO_PREDICTION)) {
        if ((movecontrol == MOVECONTROL_ABSOLUTE) || (movecontrol == MOVECONTROL_CLIMBWALL)) {
            velocity = vec_zero;
        }

        if ((movecontrol == MOVECONTROL_ANIM) || (movecontrol == MOVECONTROL_CLIMBWALL)
            || (movecontrol == MOVECONTROL_USEANIM) || (movecontrol == MOVECONTROL_LOOPUSEANIM)
            || (movecontrol == MOVECONTROL_USER_MOVEANIM)) {
            Vector delta = vec_zero;
            PlayerAnimDelta(delta);

            // using PM_NOCLIP for a smooth move
            //client->ps.pm_type = PM_NOCLIP;

            if (delta != vec_zero) {
                float mat[3][3];
                AngleVectors(angles, mat[0], mat[1], mat[2]);
                MatrixTransformVector(delta, mat, move);
                AnimMove(move, &origin);
                setOrigin(origin);
                CheckGround();
            }
        }
    }

    m_fLastDeltaTime = level.time;

    TouchStuff(&pm);
}

void Player::VehicleMove(usercmd_t *ucmd)
{
    if (!m_pVehicle) {
        return;
    }

    oldorigin = origin;

    client->ps.pm_type = GetMovePlayerMoveType();

    // set move flags
    client->ps.pm_flags &=
        ~(PMF_FROZEN | PMF_NO_PREDICTION | PMF_NO_MOVE | PMF_DUCKED | PMF_NO_GRAVITY | PMF_VIEW_PRONE
          | PMF_VIEW_DUCK_RUN | PMF_VIEW_JUMP_START);

    // disable prediction
    client->ps.pm_flags |= PMF_NO_GRAVITY | PMF_NO_PREDICTION;

    if (level.playerfrozen || m_bFrozen) {
        client->ps.pm_flags |= PMF_FROZEN;
    }

    client->ps.gravity = gravity * sv_gravity->value;

    if (m_pVehicle->Drive(current_ucmd)) {
        client->ps.commandTime = ucmd->serverTime;
    } else {
        ClientMove(ucmd);
    }
}

void Player::TurretMove(usercmd_t *ucmd)
{
    if (!m_pTurret) {
        return;
    }

    oldorigin = origin;

    client->ps.pm_type = GetMovePlayerMoveType();

    // set move flags
    client->ps.pm_flags &=
        ~(PMF_FROZEN | PMF_NO_PREDICTION | PMF_NO_MOVE | PMF_DUCKED | PMF_NO_GRAVITY | PMF_VIEW_PRONE
          | PMF_VIEW_DUCK_RUN | PMF_VIEW_JUMP_START);

    // disable prediction
    client->ps.pm_flags |= PMF_NO_GRAVITY | PMF_NO_PREDICTION;
    if (getMoveType() == MOVETYPE_PORTABLE_TURRET) {
        client->ps.pm_flags |= PMF_NO_GRAVITY;
    }

    if (level.playerfrozen || m_bFrozen) {
        client->ps.pm_flags |= PMF_FROZEN;
    }

    client->ps.gravity = gravity * sv_gravity->value;

    if (!m_pTurret->IsSubclassOfTurretGun() || !m_pTurret->UserAim(current_ucmd)) {
        ClientMove(ucmd);
    } else {
        client->ps.commandTime = ucmd->serverTime;
    }
}

void Player::ClientInactivityTimer(void)
{
    if (g_gametype->integer == GT_SINGLE_PLAYER) {
        return;
    }

    if (g_inactivekick->integer && g_inactivekick->integer < 60) {
        gi.cvar_set("g_inactiveKick", "60");
    }

    if (g_inactivespectate->integer && g_inactivespectate->integer < 20) {
        gi.cvar_set("g_inactiveSpectate", "20");
    }

    if (num_team_kills >= g_teamkillkick->integer) {
        const str message = gi.LV_ConvertString("was removed from the server for killing too many teammates.");

        //
        // The player reached maximum team kills
        //
        G_PrintToAllClients(va("%s %s", client->pers.netname, message.c_str()), qfalse);

        if (Q_stricmp(Info_ValueForKey(client->pers.userinfo, "ip"), "localhost")) {
            //
            // Make sure to not kick the local host
            //
            gi.DropClient(client->ps.clientNum, message.c_str());
        } else if (!m_bSpectator) {
            // if it's the host, put it back in spectator mode
            num_team_kills      = 0;
            m_iLastNumTeamKills = 0;

            PostEvent(EV_Player_Spectator, 0);
        }

        return;
    }

    if (num_team_kills >= g_teamkillwarn->integer && num_team_kills > m_iLastNumTeamKills) {
        const str sWarning   = gi.LV_ConvertString("Warning:");
        const str sTeamKills = gi.LV_ConvertString("more team kill(s) and you will be removed from the server.");

        m_iLastNumTeamKills = num_team_kills;

        gi.centerprintf(
            edict, "%s %i %s", sWarning.c_str(), g_teamkillkick->integer - num_team_kills, sTeamKills.c_str()
        );
    }

    if (current_ucmd->buttons & BUTTON_MOUSE || (!g_inactivespectate->integer && !g_inactivekick->integer)
        || current_ucmd->forwardmove || current_ucmd->rightmove || current_ucmd->upmove
        || (m_bTempSpectator && client->lastActiveTime >= level.inttime - 5000)) {
        client->lastActiveTime = level.inttime;
        client->activeWarning  = 0;
        return;
    }

    if (g_inactivekick->integer && client->lastActiveTime < level.inttime - 1000 * g_inactivekick->integer) {
        const char *s = Info_ValueForKey(client->pers.userinfo, "ip");

        if (Q_stricmp(s, "localhost")) {
            gi.DropClient(client->ps.clientNum, "was dropped for inactivity");
            return;
        }

        if (m_bSpectator) {
            return;
        }

        PostEvent(EV_Player_Spectator, 0);
        return;
    }

    if (g_inactivespectate->integer && client->lastActiveTime < level.inttime - g_inactivespectate->integer * 1000
        && !m_bSpectator) {
        PostEvent(EV_Player_Spectator, 0);
        return;
    }

    if (g_inactivekick->integer) {
        static struct {
            int iLevel;
            int iTime;
        } warnkick[7] = {
            {1,  30},
            {8,  15},
            {9,  5 },
            {10, 4 },
            {11, 3 },
            {12, 2 },
            {13, 1 }
        };

        int iKickWait = g_inactivekick->integer - (level.inttime - client->lastActiveTime) / 1000 - 1;

        for (int i = 0; i < 7; i++) {
            if (client->activeWarning < warnkick[i].iLevel && iKickWait < warnkick[i].iTime) {
                const str sActionIn = gi.LV_ConvertString("You will be kicked for inactivity in");
                const str sSeconds  = gi.LV_ConvertString("seconds");

                client->activeWarning = warnkick[i].iLevel;

                gi.centerprintf(edict, "%s %i %s", sActionIn.c_str(), warnkick[i].iTime, sSeconds.c_str());

                return;
            }
        }
    }

    if (g_inactivespectate->integer && dm_team != TEAM_SPECTATOR) {
        static struct {
            int iLevel;
            int iTime;
        } warnspectate[6] = {2, 15, 3, 5, 4, 4, 5, 3, 6, 2, 7, 1};

        int iSpectateWait = g_inactivespectate->integer - (level.inttime - client->lastActiveTime) / 1000 - 1;

        for (int i = 0; i < 6; i++) {
            if (client->activeWarning < warnspectate[i].iLevel && iSpectateWait < warnspectate[i].iTime) {
                const str sActionIn = gi.LV_ConvertString("You will be moved to spectator for inactivity in");
                const str sSeconds  = gi.LV_ConvertString("seconds");

                client->activeWarning = warnspectate[i].iLevel;

                gi.centerprintf(edict, "%s %i %s", sActionIn.c_str(), warnspectate[i].iTime, sSeconds.c_str());

                return;
            }
        }
    }
}

void Player::UpdateEnemies(void)
{
    float  fFov;
    float  fMaxDist;
    float  fMaxCosSquared;
    Vector vLookDir;

    if (g_gametype->integer != GT_SINGLE_PLAYER) {
        return;
    }

    if (m_pNextSquadMate == this) {
        return;
    }

    fFov           = fov * 0.9f;
    fMaxDist       = world->farplane_distance * 0.7867f;
    fMaxCosSquared = 0.0f;

    AngleVectors(m_vViewAng, vLookDir, NULL, NULL);

    if (m_Enemy) {
        m_Enemy->m_iAttackerCount -= 3;
        m_Enemy = NULL;
    }

    for (Sentient *obj = level.m_HeadSentient[0]; obj != NULL; obj = obj->m_NextSentient) {
        Vector vDelta;
        float  fDot;
        float  fDotSquared;

        if (CanSee(obj, fFov, fMaxDist, false)) {
            obj->m_fPlayerSightLevel += level.frametime;

            vDelta      = obj->origin - origin;
            fDot        = DotProduct(vDelta, vLookDir);
            fDotSquared = fDot * fDot;

            if (fDotSquared > fMaxCosSquared * vDelta.lengthSquared()) {
                fMaxCosSquared = fDotSquared / vDelta.lengthSquared();
                m_Enemy        = obj;
            }
        } else {
            obj->m_fPlayerSightLevel = 0.0f;
        }
    }

    if (m_Enemy) {
        m_Enemy->m_iAttackerCount += 3;
    }
}

/*
==============
ClientThink

This will be called once for each client frame, which will
usually be a couple times for each server frame.
==============
*/
void Player::ClientThink(void)
{
    // sanity check the command time to prevent speedup cheating
    if (current_ucmd->serverTime > level.svsTime) {
        //
        // we don't want any future commands, these could be from the previous game
        //
        return;
    }

    if (current_ucmd->serverTime < level.svsTime - 1000) {
        current_ucmd->serverTime = level.svsTime - 1000;
    }

    if ((current_ucmd->serverTime - client->ps.commandTime) < 1) {
        return;
    }

    TickSprint();

    if (g_gametype->integer != GT_SINGLE_PLAYER && dm_team == TEAM_SPECTATOR && !IsSpectator()) {
        Spectator();
    }

    last_ucmd = *current_ucmd;
    server_new_buttons |= current_ucmd->buttons & ~buttons;
    new_buttons = current_ucmd->buttons & ~buttons;
    buttons     = current_ucmd->buttons;

    if (camera) {
        m_vViewPos = camera->origin;
        m_vViewAng = camera->angles;
    } else {
        m_vViewPos[0] = (float)current_eyeinfo->ofs[0] + origin[0];
        m_vViewPos[1] = (float)current_eyeinfo->ofs[1] + origin[1];
        m_vViewPos[2] = (float)current_eyeinfo->ofs[2] + origin[2];

        m_vViewAng[0] = current_eyeinfo->angles[0];
        m_vViewAng[1] = current_eyeinfo->angles[1];
        m_vViewAng[2] = 0.0f;
    }

    VectorCopy(m_vViewPos, client->ps.vEyePos);

    if (!level.intermissiontime) {
        if (new_buttons & BUTTON_ATTACKRIGHT) {
            Weapon *weapon = GetActiveWeapon(WEAPON_MAIN);

            if (weapon && (weapon->GetZoom())) {
                ToggleZoom(weapon->GetZoom());
            }
        }

        if (new_buttons & BUTTON_USE) {
            DoUse(NULL);
        }

        moveresult = MOVERESULT_NONE;

        if (m_pTurret) {
            TurretMove(current_ucmd);
        } else if (m_pVehicle) {
            VehicleMove(current_ucmd);
        } else {
            ClientMove(current_ucmd);
        }

        // Save cmd angles so that we can get delta angle movements next frame
        client->cmd_angles[0] = SHORT2ANGLE(current_ucmd->angles[0]);
        client->cmd_angles[1] = SHORT2ANGLE(current_ucmd->angles[1]);
        client->cmd_angles[2] = SHORT2ANGLE(current_ucmd->angles[2]);

        if (g_gametype->integer != GT_SINGLE_PLAYER && g_smoothClients->integer && !IsSubclassOfBot()) {
            VectorCopy(client->ps.velocity, edict->s.pos.trDelta);
            edict->s.pos.trTime = client->ps.commandTime;
        } else {
            VectorClear(edict->s.pos.trDelta);
            edict->s.pos.trTime = 0;
        }

        ClientInactivityTimer();
    } else {
        if (g_gametype->integer != GT_SINGLE_PLAYER) {
            client->ps.pm_flags |= PMF_FROZEN;
            client->ps.pm_flags |= PMF_INTERMISSION;

            if (level.time - level.intermissiontime > 5.0f
                && (new_buttons & (BUTTON_ATTACKLEFT | BUTTON_ATTACKRIGHT))) {
                level.exitintermission = true;
            }
        } else {
            if (level.intermissiontype == TRANS_MISSION_FAILED || IsDead()) {
                gi.cvar_set("g_success", "0");
                gi.cvar_set("g_failed", "1");
            } else {
                gi.cvar_set("g_success", "1");
                gi.cvar_set("g_failed", "0");
            }

            // prevent getting medals from cheats
            if (g_medal0->modificationCount > 1 || g_medal1->modificationCount > 1 || g_medal2->modificationCount > 1
                || g_medal3->modificationCount > 1 || g_medal4->modificationCount > 1 || g_medal5->modificationCount > 1
                || g_medalbt1->modificationCount > 1 || g_medalbt2->modificationCount > 1
                || g_medalbt3->modificationCount > 1 || g_medalbt4->modificationCount > 1
                || g_medalbt5->modificationCount > 1 || g_eogmedal0->modificationCount > 1
                || g_eogmedal1->modificationCount > 1 || g_eogmedal2->modificationCount > 1) {
                gi.cvar_set("g_gotmedal", "0");
            } else {
                gi.cvar_set("g_gotmedal", "1");
            }

            client->ps.pm_flags |= PMF_FROZEN;

            if (level.time - level.intermissiontime > 4.0f) {
                if (level.intermissiontype) {
                    if (level.intermissiontype == TRANS_MISSION_FAILED) {
                        if ((new_buttons & BUTTON_ATTACKLEFT) || (new_buttons & BUTTON_ATTACKRIGHT)) {
                            G_MissionFailed();
                        }
                    } else if ((new_buttons & BUTTON_ATTACKLEFT) || (new_buttons & BUTTON_ATTACKRIGHT)) {
                        g_medal0->modificationCount    = 1;
                        g_medal1->modificationCount    = 1;
                        g_medal2->modificationCount    = 1;
                        g_medal3->modificationCount    = 1;
                        g_medal4->modificationCount    = 1;
                        g_medal5->modificationCount    = 1;
                        g_medalbt0->modificationCount  = 1;
                        g_medalbt1->modificationCount  = 1;
                        g_medalbt2->modificationCount  = 1;
                        g_medalbt3->modificationCount  = 1;
                        g_medalbt4->modificationCount  = 1;
                        g_medalbt5->modificationCount  = 1;
                        g_eogmedal0->modificationCount = 1;
                        g_eogmedal1->modificationCount = 1;
                        g_eogmedal2->modificationCount = 1;
                        g_medal0->modified             = false;
                        g_medal1->modified             = false;
                        g_medal2->modified             = false;
                        g_medal3->modified             = false;
                        g_medal4->modified             = false;
                        g_medal5->modified             = false;
                        g_eogmedal0->modified          = false;
                        g_eogmedal1->modified          = false;
                        g_eogmedal2->modified          = false;

                        level.exitintermission = true;
                    }
                } else {
                    level.exitintermission = true;
                }
            }
        }

        // Save cmd angles so that we can get delta angle movements next frame
        client->cmd_angles[0]  = SHORT2ANGLE(current_ucmd->angles[0]);
        client->cmd_angles[1]  = SHORT2ANGLE(current_ucmd->angles[1]);
        client->cmd_angles[2]  = SHORT2ANGLE(current_ucmd->angles[2]);
        client->ps.commandTime = current_ucmd->serverTime;
    }
}

void Player::Think(void)
{
    static cvar_t *g_aimLagTime = NULL;

    int     m_iClientWeaponCommand;
    Event  *m_pWeaponCommand = NULL;
    Weapon *pWeap;

    if (whereami->integer && origin != oldorigin) {
        gi.DPrintf("x %8.2f y %8.2f z %8.2f area %2d\n", origin[0], origin[1], origin[2], edict->r.areanum);
    }

    if (g_gametype->integer == GT_SINGLE_PLAYER && g_playermodel->modified) {
        setModel("models/player/" + str(g_playermodel->string) + ".tik");

        if (!edict->tiki) {
            setModel("models/player/american_army.tik");
        }

        g_playermodel->modified = qfalse;
    }

    if (g_gametype->integer == GT_SINGLE_PLAYER) {
        m_bIsDisguised = false;

        if (m_bHasDisguise && !level.m_bAlarm) {
            pWeap = GetActiveWeapon(WEAPON_MAIN);

            if (!pWeap || pWeap->IsSubclassOfInventoryItem()) {
                m_bIsDisguised = true;

                for (Sentient *pSent = level.m_HeadSentient[0]; pSent != NULL; pSent = pSent->m_NextSentient) {
                    Actor *act = (Actor *)pSent;

                    if (pSent->m_Enemy == this && act->IsAttacking()) {
                        m_bIsDisguised = false;
                        break;
                    }
                }
            }
        }

        PathSearch::PlayerCover(this);
        UpdateEnemies();
    }

    if (movetype == MOVETYPE_NOCLIP) {
        StopPartAnimating(torso);
        SetPartAnim("idle");

        client->ps.walking = qfalse;
        groundentity       = 0;
    } else {
        CheckMoveFlags();
        EvaluateState();
    }

    oldvelocity = velocity;
    old_v_angle = v_angle;

    if (g_gametype->integer == GT_SINGLE_PLAYER) {
        if ((server_new_buttons & BUTTON_ATTACKLEFT) && (!GetActiveWeapon(WEAPON_MAIN)) && !IsDead()
            && !IsNewActiveWeapon() && !LoadingSavegame) {
            Event *ev = new Event("useweaponclass");
            ev->AddString("item1");

            ProcessEvent(ev);
        }
    } else {
        // Added in 2.0: Invulnerability and team spawn
        TickInvulnerable();
        TickTeamSpawn();

        if (deadflag == DEAD_DEAD && level.time > respawn_time) {
            if (dmManager.AllowRespawn() && AllowTeamRespawn()) {
                if (((server_new_buttons & BUTTON_ATTACKLEFT) || (server_new_buttons & BUTTON_ATTACKRIGHT))
                    || (g_forcerespawn->integer > 0 && level.time > g_forcerespawn->integer + respawn_time)) {
                    // 2.0
                    //  Check for team respawn
                    //
                    if (!m_fSpawnTimeLeft) {
                        if (AllowTeamRespawn()) {
                            EndSpectator();
                            PostEvent(EV_Player_Respawn, 0);
                        }
                    } else {
                        m_bWaitingForRespawn = true;
                    }
                }
            } else if (!IsSpectator()) {
                BeginTempSpectator();
            }
        }

        if (IsSpectator()) {
            if (!m_bTempSpectator && level.time > respawn_time && (server_new_buttons & BUTTON_ATTACKLEFT)) {
                if (current_team && dm_team != TEAM_SPECTATOR) {
                    if (client->pers.dm_primary[0]) {
                        if ((g_gametype->integer == GT_FFA
                             || (g_gametype->integer >= GT_TEAM && dm_team > TEAM_FREEFORALL))
                            && deadflag != DEAD_DEAD) {
                            // 2.0
                            //  Check for team respawn
                            //
                            if (!m_fSpawnTimeLeft) {
                                if (AllowTeamRespawn()) {
                                    EndSpectator();
                                    PostEvent(EV_Player_Respawn, 0);
                                }
                            } else {
                                m_bWaitingForRespawn = true;
                            }
                        }
                    } else if (m_fWeapSelectTime < level.time) {
                        m_fWeapSelectTime = level.time + 1.0;
                        UserSelectWeapon(false);
                    }
                } else if (m_fWeapSelectTime < level.time) {
                    m_fWeapSelectTime = level.time + 1.0;
                    gi.SendServerCommand(edict - g_entities, "stufftext \"pushmenu_teamselect\"");
                }
            }
        } else if (!client->pers.dm_primary[0]) {
            Spectator();
            if (m_fWeapSelectTime < level.time) {
                gi.SendServerCommand(edict - g_entities, "stufftext \"pushmenu_weaponselect\"");
            }
        }

        if (IsSpectator()) {
            if (g_protocol >= PROTOCOL_MOHTA_MIN) {
                if (m_iPlayerSpectating) {
                    if (last_ucmd.upmove) {
                        if (!m_bSpectatorSwitching) {
                            m_bSpectatorSwitching = true;

                            if (last_ucmd.upmove > 0) {
                                SetPlayerSpectate(true);
                            } else {
                                SetPlayerSpectate(false);
                            }
                        }
                    } else {
                        m_bSpectatorSwitching = false;
                    }
                } else if ((server_new_buttons & BUTTON_USE)) {
                    SetPlayerSpectateRandom();
                    server_new_buttons &= ~BUTTON_USE;
                }
            } else {
                if ((server_new_buttons & BUTTON_USE)) {
                    SetPlayerSpectateRandom();
                }
            }

            if (g_gametype->integer >= GT_TEAM && g_forceteamspectate->integer && GetTeam() > TEAM_FREEFORALL) {
                if (!m_iPlayerSpectating) {
                    SetPlayerSpectateRandom();
                } else {
                    gentity_t *ent = g_entities + m_iPlayerSpectating - 1;

                    if (!ent->inuse || !ent->entity) {
                        // Invalid spectate entity
                        SetPlayerSpectateRandom();
                    } else if (ent->entity->deadflag >= DEAD_DEAD || static_cast<Player *>(ent->entity)->IsSpectator() || IsValidSpectatePlayer(static_cast<Player *>(ent->entity))) {
                        SetPlayerSpectateRandom();
                    }
                }
            } else {
                if (g_protocol >= protocol_e::PROTOCOL_MOHTA_MIN) {
                    // Since 2.0, use = clear spectator
                    if (m_iPlayerSpectating && (server_new_buttons & BUTTON_USE)) {
                        m_iPlayerSpectating = 0;
                    }
                } else {
                    // On 1.11 and below, up = clear spectator
                    if (last_ucmd.upmove) {
                        m_iPlayerSpectating = 0;
                    }
                }

                if (m_iPlayerSpectating) {
                    gentity_t *ent = g_entities + m_iPlayerSpectating - 1;

                    if (!ent->inuse || !ent->entity) {
                        // Invalid spectate entity
                        SetPlayerSpectateRandom();
                    } else if (ent->entity->deadflag >= DEAD_DEAD || static_cast<Player *>(ent->entity)->IsSpectator() || !IsValidSpectatePlayer(static_cast<Player *>(ent->entity))) {
                        SetPlayerSpectateRandom();
                    } else if (g_gametype->integer >= GT_TEAM && GetTeam() > TEAM_FREEFORALL && static_cast<Player*>(ent->entity)->GetTeam() != GetTeam()) {
                        SetPlayerSpectateRandom();
                    }
                }
            }
        } else {
            m_iPlayerSpectating = 0;
        }
    }

    if (g_logstats->integer) {
        if (!logfile_started) {
            ProcessEvent(EV_Player_LogStats);
            logfile_started = qtrue;
        }
    }

    if (!IsDead()) {
        m_iClientWeaponCommand = (server_new_buttons & WEAPON_COMMAND_MASK) >> 7;

        switch (m_iClientWeaponCommand) {
        case 0:
            // No command
            break;
        case WEAPON_COMMAND_USE_PISTOL:
            m_pWeaponCommand = new Event(EV_Sentient_UseWeaponClass);
            m_pWeaponCommand->AddString("pistol");
            break;
        case WEAPON_COMMAND_USE_RIFLE:
            m_pWeaponCommand = new Event(EV_Sentient_UseWeaponClass);
            m_pWeaponCommand->AddString("rifle");
            break;
        case WEAPON_COMMAND_USE_SMG:
            m_pWeaponCommand = new Event(EV_Sentient_UseWeaponClass);
            m_pWeaponCommand->AddString("smg");
            break;
        case WEAPON_COMMAND_USE_MG:
            m_pWeaponCommand = new Event(EV_Sentient_UseWeaponClass);
            m_pWeaponCommand->AddString("mg");
            break;
        case WEAPON_COMMAND_USE_GRENADE:
            m_pWeaponCommand = new Event(EV_Sentient_UseWeaponClass);
            m_pWeaponCommand->AddString("grenade");
            break;
        case WEAPON_COMMAND_USE_HEAVY:
            m_pWeaponCommand = new Event(EV_Sentient_UseWeaponClass);
            m_pWeaponCommand->AddString("heavy");
            break;
        case WEAPON_COMMAND_USE_ITEM1:
            m_pWeaponCommand = new Event(EV_Sentient_ToggleItemUse);
            break;
        case WEAPON_COMMAND_USE_ITEM2:
            m_pWeaponCommand = new Event(EV_Sentient_UseWeaponClass);
            m_pWeaponCommand->AddString("item2");
            break;
        case WEAPON_COMMAND_USE_ITEM3:
            m_pWeaponCommand = new Event(EV_Sentient_UseWeaponClass);
            m_pWeaponCommand->AddString("item3");
            break;
        case WEAPON_COMMAND_USE_ITEM4:
            m_pWeaponCommand = new Event(EV_Sentient_UseWeaponClass);
            m_pWeaponCommand->AddString("item4");
            break;
        case WEAPON_COMMAND_USE_PREV_WEAPON:
            m_pWeaponCommand = new Event(EV_Player_PrevWeapon);
            break;
        case WEAPON_COMMAND_USE_NEXT_WEAPON:
            m_pWeaponCommand = new Event(EV_Player_NextWeapon);
            break;
        case WEAPON_COMMAND_USE_LAST_WEAPON:
            m_pWeaponCommand = new Event(EV_Sentient_UseLastWeapon);
            break;
        case WEAPON_COMMAND_HOLSTER:
            m_pWeaponCommand = new Event(EV_Player_Holster);
            break;
        case WEAPON_COMMAND_DROP:
            m_pWeaponCommand = new Event(EV_Player_DropWeapon);
            break;
        default:
            gi.DPrintf("Unrecognized weapon command %d\n", m_iClientWeaponCommand);
        }

        if (m_pWeaponCommand) {
            PostEvent(m_pWeaponCommand, 0);
        }
    }

    if (g_gametype->integer == GT_SINGLE_PLAYER) {
        if (!g_aimLagTime) {
            g_aimLagTime = gi.Cvar_Get("g_aimLagTime", "250", 0);
        }

        if (mLastTrailTime + g_aimLagTime->integer < level.inttime) {
            mLastTrailTime = level.inttime;

            mvTrail[0]        = centroid;
            mvTrailEyes[0]    = centroid;
            mvTrailEyes[0][0] = EyePosition()[0];
        }
    }
    UpdateFootsteps();

    //
    // Added in 2.0
    // Heal rate
    //
    if (m_fHealRate && !IsDead()) {
        float newrate;

        if (g_healrate->value && g_gametype->integer != GT_SINGLE_PLAYER) {
            newrate = max_health * (g_healrate->value / 100.f) * level.frametime;
            if (newrate >= m_fHealRate) {
                newrate     = m_fHealRate;
                m_fHealRate = 0;
            } else {
                m_fHealRate -= newrate;
            }
        } else {
            newrate     = m_fHealRate;
            m_fHealRate = 0;
        }

        // heal
        health += newrate;
        if (health > max_health) {
            // make sure it doesn't go above the maximum player health
            health = max_health;
        }
    }

    //
    // Added in 2.0: talk icon
    //
    if (buttons & BUTTON_TALK) {
        edict->s.eFlags |= EF_PLAYER_TALKING;
    } else {
        edict->s.eFlags &= ~EF_PLAYER_TALKING;
    }

    if (m_fTalkTime > level.time) {
        edict->s.eFlags |= EF_PLAYER_TALKING;
    } else {
        edict->s.eFlags &= ~EF_PLAYER_TALKING;
    }

    //
    // Openmohaa additions
    //
    if (!animDoneVM) {
        int    index;
        float  anim_time;
        vma_t *vma = vmalist.find(m_sVMcurrent);

        index = m_fpsTiki == NULL ? -1 : gi.Anim_NumForName(m_fpsTiki, m_sVMAcurrent);

        if (index >= 0) {
            anim_time = gi.Anim_Time(m_fpsTiki, index);

            if (m_fVMAtime < anim_time) {
                if (vma) {
                    m_fVMAtime += level.frametime * vma->speed;
                } else {
                    m_fVMAtime += level.frametime;
                }
            } else {
                animDoneVM = true;
                m_fVMAtime = 0;

                Notify("viewmodelanim_done");
            }
        } else {
            animDoneVM = true;
            m_fVMAtime = 0;

            Notify("viewmodelanim_done");
        }
    }

    server_new_buttons = 0;
}

void Player::InitLegsStateTable(void)
{
    animdone_Legs     = false;
    currentState_Legs = statemap_Legs->FindState("STAND");

    str legsAnim(currentState_Legs->getLegAnim(*this, &legs_conditionals));
    if (legsAnim == "") {
        StopPartAnimating(legs);
    } else if (legsAnim != "none") {
        SetPartAnim(legsAnim.c_str(), legs);
    }
}

void Player::InitTorsoStateTable(void)
{
    animdone_Torso = false;

    currentState_Torso = statemap_Torso->FindState("STAND");

    str torsoAnim(currentState_Torso->getActionAnim(*this, &torso_conditionals));
    if (torsoAnim == "") {
        StopPartAnimating(torso);
    } else if (torsoAnim != "none") {
        SetPartAnim(torsoAnim.c_str(), torso);
    }
}

void Player::LoadStateTable(void)
{
    statemap_Legs  = NULL;
    statemap_Torso = NULL;

    legs_conditionals.FreeObjectList();
    torso_conditionals.FreeObjectList();

    statemap_Legs =
        GetStatemap(str(g_statefile->string) + "_Legs.st", (Condition<Class> *)Conditions, &legs_conditionals, false);
    statemap_Torso =
        GetStatemap(str(g_statefile->string) + "_Torso.st", (Condition<Class> *)Conditions, &torso_conditionals, false);

    movecontrol = MOVECONTROL_LEGS;

    InitLegsStateTable();
    InitTorsoStateTable();

    movecontrol = currentState_Legs->getMoveType();
    if (!movecontrol) {
        movecontrol = MOVECONTROL_LEGS;
    }

    for (int i = 1; i <= legs_conditionals.NumObjects(); i++) {
        Conditional *c = legs_conditionals.ObjectAt(i);

        if (Q_stricmp(c->getName(), "PAIN") && !c->parmList.NumObjects()) {
            m_pLegsPainCond = c;
            break;
        }
    }

    for (int i = 1; i <= torso_conditionals.NumObjects(); i++) {
        Conditional *c = torso_conditionals.ObjectAt(i);

        if (Q_stricmp(c->getName(), "PAIN") && !c->parmList.NumObjects()) {
            m_pTorsoPainCond = c;
            break;
        }
    }

    if ((movecontrol < (sizeof(MoveStartFuncs) / sizeof(MoveStartFuncs[0]))) && (MoveStartFuncs[movecontrol])) {
        (this->*MoveStartFuncs[movecontrol])();
    }

    SetViewAngles(v_angle);
}

void Player::ResetState(Event *ev)
{
    movecontrol = MOVECONTROL_LEGS;
    LoadStateTable();
}

void Player::StartPush(void)

{
    trace_t trace;
    Vector  end(origin + yaw_forward * 64.0f);

    trace = G_Trace(origin, mins, maxs, end, this, MASK_SOLID, true, "StartPush");
    if (trace.fraction == 1.0f) {
        return;
    }
    v_angle.y = vectoyaw(trace.plane.normal) - 180;
    SetViewAngles(v_angle);

    setOrigin(trace.endpos - yaw_forward * 0.4f);
}

void Player::StartClimbLadder(void)

{
    trace_t trace;
    Vector  end(origin + yaw_forward * 20.0f);

    trace = G_Trace(origin, mins, maxs, end, this, MASK_SOLID, true, "StartClimbLadder");
    if ((trace.fraction == 1.0f) || !(trace.surfaceFlags & SURF_LADDER)) {
        return;
    }

    v_angle.y = vectoyaw(trace.plane.normal) - 180;
    SetViewAngles(v_angle);

    setOrigin(trace.endpos - yaw_forward * 0.4f);
}

void Player::StartUseAnim(void)
{
    UseAnim *ua;
    Vector   neworg;
    Vector   newangles;
    str      newanim;
    str      state;
    str      camera;
    trace_t  trace;

    if (toucheduseanim) {
        ua = (UseAnim *)(Entity *)toucheduseanim;
    } else if (atobject) {
        ua = (UseAnim *)(Entity *)atobject;
    } else {
        return;
    }

    useitem_in_use = ua;
    toucheduseanim = NULL;
    atobject       = NULL;

    if (ua->GetInformation(this, &neworg, &newangles, &newanim, &useanim_numloops, &state, &camera)) {
        trace = G_Trace(origin, mins, maxs, neworg, this, MASK_PLAYERSOLID, true, "StartUseAnim");
        if (trace.startsolid || (trace.fraction < 1.0f)) {
            gi.DPrintf("Move to UseAnim was blocked.\n");
        }

        if (!trace.startsolid) {
            setOrigin(trace.endpos);
        }

        setAngles(newangles);
        v_angle.y = newangles.y;
        SetViewAngles(v_angle);

        movecontrol = MOVECONTROL_ABSOLUTE;

        if (state.length()) {
            State *newState;

            newState = statemap_Torso->FindState(state);
            if (newState) {
                EvaluateState(newState);
            } else {
                gi.DPrintf("Could not find state %s on UseAnim\n", state.c_str());
            }
        } else {
            if (currentState_Torso) {
                if (camera.length()) {
                    currentState_Torso->setCameraType(camera);
                } else {
                    currentState_Torso->setCameraType("behind");
                }
            }
            SetPartAnim(newanim, legs);
        }
    }
}

void Player::StartLoopUseAnim(void)

{
    useanim_numloops--;
}

void Player::FinishUseAnim(Event *ev)

{
    UseAnim *ua;

    if (!useitem_in_use) {
        return;
    }

    ua = (UseAnim *)(Entity *)useitem_in_use;
    ua->TriggerTargets(this);
    useitem_in_use = NULL;
}

void Player::SetupUseObject(void)

{
    UseObject *uo;
    Vector     neworg;
    Vector     newangles;
    str        state;
    trace_t    trace;

    if (atobject) {
        uo = (UseObject *)(Entity *)atobject;
    } else {
        return;
    }

    useitem_in_use = uo;

    uo->Setup(this, &neworg, &newangles, &state);
    {
        trace = G_Trace(neworg, mins, maxs, neworg, this, MASK_PLAYERSOLID, true, "SetupUseObject - 1");
        if (trace.startsolid || trace.allsolid) {
            trace = G_Trace(origin, mins, maxs, neworg, this, MASK_PLAYERSOLID, true, "SetupUseObject - 2");
            if (trace.startsolid || (trace.fraction < 1.0f)) {
                gi.DPrintf("Move to UseObject was blocked.\n");
            }
        }

        if (!trace.startsolid) {
            setOrigin(trace.endpos);
        }

        setAngles(newangles);
        v_angle.y = newangles.y;
        SetViewAngles(v_angle);

        movecontrol = MOVECONTROL_ABSOLUTE;

        if (state.length()) {
            State *newState;

            newState = statemap_Torso->FindState(state);
            if (newState) {
                EvaluateState(newState);
            } else {
                gi.DPrintf("Could not find state %s on UseObject\n", state.c_str());
            }
        }
    }
}

void Player::StartUseObject(Event *ev)

{
    UseObject *uo;

    if (!useitem_in_use) {
        return;
    }

    uo = (UseObject *)(Entity *)useitem_in_use;
    uo->Start();
}

void Player::FinishUseObject(Event *ev)

{
    UseObject *uo;

    if (!useitem_in_use) {
        return;
    }

    uo = (UseObject *)(Entity *)useitem_in_use;
    uo->Stop(this);
    useitem_in_use = NULL;
}

void Player::Turn(Event *ev)

{
    float  yaw;
    Vector oldang(v_angle);

    yaw = ev->GetFloat(1);

    v_angle[YAW] = (int)(anglemod(v_angle[YAW]) / 22.5f) * 22.5f;
    SetViewAngles(v_angle);

    if (!CheckMove(vec_zero)) {
        SetViewAngles(oldang);
        return;
    }

    CancelEventsOfType(EV_Player_TurnUpdate);

    ev = new Event(EV_Player_TurnUpdate);
    ev->AddFloat(yaw / 5.0f);
    ev->AddFloat(0.5f);
    ProcessEvent(ev);
}

void Player::TurnUpdate(Event *ev)

{
    float  yaw;
    float  timeleft;
    Vector oldang(v_angle);

    yaw      = ev->GetFloat(1);
    timeleft = ev->GetFloat(2);
    timeleft -= 0.1f;

    if (timeleft > 0) {
        ev = new Event(EV_Player_TurnUpdate);
        ev->AddFloat(yaw);
        ev->AddFloat(timeleft);
        PostEvent(ev, 0.1f);

        v_angle[YAW] += yaw;
        SetViewAngles(v_angle);
    } else {
        v_angle[YAW] = (int)(anglemod(v_angle[YAW]) / 22.5f) * 22.5f;
        SetViewAngles(v_angle);
    }

    if (!CheckMove(vec_zero)) {
        SetViewAngles(oldang);
    }
}

void Player::TurnLegs(Event *ev)

{
    float yaw;

    yaw = ev->GetFloat(1);

    angles[YAW] += yaw;
    setAngles(angles);
}

void Player::EvaluateState(State *forceTorso, State *forceLegs)
{
    int           count;
    State        *laststate_Legs;
    State        *laststate_Torso;
    State        *startstate_Legs;
    State        *startstate_Torso;
    movecontrol_t move;

    if (getMoveType() == MOVETYPE_NOCLIP) {
        return;
    }

    // Evaluate the current state.
    // When the state changes, we reevaluate the state so that if the
    // conditions aren't met in the new state, we don't play one frame of
    // the animation for that state before going to the next state.
    startstate_Torso = laststate_Torso = currentState_Torso;
    count                              = 0;
    do {
        // since we could get into an infinite loop here, do a check
        // to make sure we don't.
        count++;
        if (count > 10) {
            gi.DPrintf("Possible infinite loop in state '%s'\n", currentState_Torso->getName());
            if (count > 20) {
                assert(0);
                gi.Error(ERR_DROP, "Stopping due to possible infinite state loop\n");
                break;
            }
        }

        laststate_Torso = currentState_Torso;

        if (forceTorso) {
            currentState_Torso = forceTorso;
        } else {
            currentState_Torso = currentState_Torso->Evaluate(*this, &torso_conditionals);
        }

        if (currentState_Torso) {
            // Process exit commands of the last state
            laststate_Torso->ProcessExitCommands(this);

            // Process entry commands of the new state
            currentState_Torso->ProcessEntryCommands(this);

            if (waitForState.length() && (!waitForState.icmpn(currentState_Torso->getName(), waitForState.length()))) {
                waitForState = "";
            }

            move = currentState_Torso->getMoveType();

            // use the current movecontrol
            if (move == MOVECONTROL_NONE) {
                move = movecontrol;
            }

            str legsAnim;
            str torsoAnim(currentState_Torso->getActionAnim(*this, &torso_conditionals));

            if (move == MOVECONTROL_LEGS) {
                if (!currentState_Legs) {
                    animdone_Legs     = false;
                    currentState_Legs = statemap_Legs->FindState("STAND");
                    legsAnim          = currentState_Legs->getLegAnim(*this, &legs_conditionals);

                    if (legsAnim == "") {
                        StopPartAnimating(legs);
                    } else if (legsAnim != "none") {
                        SetPartAnim(legsAnim.c_str(), legs);
                    }
                }

                if (torsoAnim == "none") {
                    StopPartAnimating(torso);
                    animdone_Torso = true;
                } else if (torsoAnim != "") {
                    SetPartAnim(torsoAnim.c_str(), torso);
                }
            } else {
                if (torsoAnim == "none") {
                    StopPartAnimating(torso);
                    animdone_Torso = true;
                } else if (torsoAnim != "") {
                    SetPartAnim(torsoAnim.c_str(), torso);
                }

                legsAnim = currentState_Torso->getLegAnim(*this, &torso_conditionals);

                if (legsAnim == "none" || legsAnim == "") {
                    StopPartAnimating(legs);
                } else {
                    SetPartAnim(legsAnim.c_str(), legs);
                }
            }

            if (movecontrol != move) {
                movecontrol = move;
                if ((move < (sizeof(MoveStartFuncs) / sizeof(MoveStartFuncs[0]))) && (MoveStartFuncs[move])) {
                    (this->*MoveStartFuncs[move])();
                }

                if (movecontrol == MOVECONTROL_CLIMBWALL) {
                    edict->s.eFlags |= EF_CLIMBWALL;
                } else {
                    edict->s.eFlags &= ~EF_CLIMBWALL;
                }
            }

            SetViewAngles(v_angle);
        } else {
            currentState_Torso = laststate_Torso;
        }
    } while (laststate_Torso != currentState_Torso);

    // Evaluate the current state.
    // When the state changes, we reevaluate the state so that if the
    // conditions aren't met in the new state, we don't play one frame of
    // the animation for that state before going to the next state.
    startstate_Legs = laststate_Legs = currentState_Legs;
    if (movecontrol == MOVECONTROL_LEGS) {
        count = 0;
        do {
            // since we could get into an infinite loop here, do a check
            // to make sure we don't.
            count++;
            if (count > 10) {
                gi.DPrintf("Possible infinite loop in state '%s'\n", currentState_Legs->getName());
                if (count > 20) {
                    assert(0);
                    gi.Error(ERR_DROP, "Stopping due to possible infinite state loop\n");
                    break;
                }
            }

            if (!laststate_Legs) {
                if ((m_iMovePosFlags & MPF_POSITION_CROUCHING)) {
                    currentState_Legs = statemap_Legs->FindState("CROUCH_IDLE");
                } else {
                    currentState_Legs = statemap_Legs->FindState("STAND");
                }
            }

            laststate_Legs = currentState_Legs;

            if (forceLegs) {
                currentState_Legs = forceLegs;
            } else {
                currentState_Legs = currentState_Legs->Evaluate(*this, &legs_conditionals);
            }

            animdone_Legs = false;
            if (currentState_Legs) {
                // Process exit commands of the last state
                laststate_Legs->ProcessExitCommands(this);

                // Process entry commands of the new state
                currentState_Legs->ProcessEntryCommands(this);

                if (waitForState.length()
                    && (!waitForState.icmpn(currentState_Legs->getName(), waitForState.length()))) {
                    waitForState = "";
                }

                str legsAnim(currentState_Legs->getLegAnim(*this, &legs_conditionals));

                if (legsAnim == "none") {
                    StopPartAnimating(legs);
                    animdone_Legs = true;
                } else if (legsAnim != "") {
                    SetPartAnim(legsAnim, legs);
                }
            } else {
                currentState_Legs = laststate_Legs;
            }
        } while (laststate_Legs != currentState_Legs);
    } else {
        currentState_Legs = NULL;
    }

    if (g_showplayeranim->integer) {
        if (last_leg_anim_name != AnimName(legs)) {
            gi.DPrintf("Legs change from %s to %s\n", last_leg_anim_name.c_str(), AnimName(legs));
            last_leg_anim_name = AnimName(legs);
        }

        if (last_torso_anim_name != AnimName(torso)) {
            gi.DPrintf("Torso change from %s to %s\n", last_torso_anim_name.c_str(), AnimName(torso));
            last_torso_anim_name = AnimName(torso);
        }
    }

    if (g_showplayerstate->integer) {
        if (startstate_Legs != currentState_Legs) {
            gi.DPrintf(
                "Legs change from %s to %s\n",
                startstate_Legs ? startstate_Legs->getName() : "NULL",
                currentState_Legs ? currentState_Legs->getName() : "NULL"
            );
        }

        if (startstate_Torso != currentState_Torso) {
            gi.DPrintf(
                "Torso change from %s to %s\n",
                startstate_Torso ? startstate_Torso->getName() : "NULL",
                currentState_Torso ? currentState_Torso->getName() : "NULL"
            );
        }
    }

    // This is so we don't remember pain when we change to a state that has a PAIN condition
    pain = 0;
}

void Player::SelectPreviousItem(Event *ev)
{
    if (deadflag) {
        return;
    }

    Item *item = GetActiveWeapon(WEAPON_MAIN);

    item = PrevItem(item);

    if (item) {
        useWeapon((Weapon *)item, WEAPON_MAIN);
    }
}

void Player::SelectNextItem(Event *ev)
{
    if (deadflag) {
        return;
    }

    Item *item = GetActiveWeapon(WEAPON_MAIN);

    item = NextItem(item);

    if (item) {
        useWeapon((Weapon *)item, WEAPON_MAIN);
    }
}

void Player::SelectPreviousWeapon(Event *ev)
{
    if (deadflag) {
        return;
    }

    Weapon *weapon       = GetActiveWeapon(WEAPON_MAIN);
    Weapon *activeWeapon = weapon;

    if (weapon) {
        weapon = PreviousWeapon(weapon);

        if (g_gametype->integer != GT_SINGLE_PLAYER) {
            while (weapon && weapon != activeWeapon && weapon->IsSubclassOfInventoryItem()) {
                weapon = PreviousWeapon(weapon);
            }
        }
    } else {
        weapon = BestWeapon();
    }

    if (weapon && weapon != activeWeapon) {
        useWeapon(weapon);
    }
}

void Player::SelectNextWeapon(Event *ev)
{
    if (deadflag) {
        return;
    }

    Weapon *weapon       = GetActiveWeapon(WEAPON_MAIN);
    Weapon *activeWeapon = weapon;

    if (weapon) {
        weapon = NextWeapon(weapon);

        if (g_gametype->integer != GT_SINGLE_PLAYER) {
            while (weapon && weapon != activeWeapon && weapon->IsSubclassOfInventoryItem()) {
                weapon = NextWeapon(weapon);
            }
        }
    } else {
        weapon = WorstWeapon();
    }

    if (weapon && weapon != activeWeapon) {
        useWeapon(weapon);
    }
}

void Player::DropCurrentWeapon(Event *ev)
{
    Weapon *weapon;
    Vector  forward;

    if (g_gametype->integer == GT_SINGLE_PLAYER) {
        return;
    }

    weapon = GetActiveWeapon(WEAPON_MAIN);

    if (!weapon) {
        return;
    }

    // Don't drop the weapon if we're charging
    if (charge_start_time) {
        return;
    }

    if ((weapon->GetWeaponClass() & WEAPON_CLASS_ITEM)) {
        SelectNextWeapon(NULL);
        takeItem(weapon->model);
    } else {
        if (weapon->GetCurrentAttachToTag() != "tag_weapon_right") {
            EventCorrectWeaponAttachments(NULL);
        }

        // This check isn't in MOHAA
        if (!weapon->IsDroppable()) {
            return;
        }

        weapon->Drop();

        AngleVectors(m_vViewAng, forward, NULL, NULL);

        // make the weapon looks like it's thrown
        weapon->velocity = forward * 200.0f;

        edict->s.eFlags |= EF_UNARMED;

        SelectNextWeapon(NULL);

        if (holsteredWeapon == weapon) {
            holsteredWeapon = NULL;
        }
        if (lastActiveWeapon.weapon == weapon) {
            lastActiveWeapon.weapon = NULL;
        }
    }
}

void Player::GiveWeaponCheat(Event *ev)

{
    giveItem(ev->GetString(1));
}

void Player::GiveCheat(Event *ev)

{
    str name;

    if (deadflag) {
        return;
    }

    name = ev->GetString(1);

    if (!name.icmp("all")) {
        GiveAllCheat(ev);
        return;
    }
    EventGiveItem(ev);
}

void Player::GiveAllCheat(Event *ev)
{
    char *buffer;
    char *buf;
    char  com_token[MAX_STRING_CHARS];

    if (deadflag) {
        return;
    }

    if (gi.FS_ReadFile("global/giveall.scr", (void **)&buf, true) != -1) {
        buffer = buf;
        while (1) {
            strcpy(com_token, COM_ParseExt(&buffer, qtrue));

            if (!com_token[0]) {
                break;
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

            this->ProcessEvent(ev);
        }
        gi.FS_FreeFile(buf);
    }
}

void Player::GiveNewWeaponsCheat(Event *ev)
{
    char       *buffer;
    char       *current;
    const char *token;

    if (deadflag != DEAD_NO) {
        return;
    }

    if (gi.FS_ReadFile("global/givenewweapons.scr", (void **)&buffer, qtrue) != -1) {
        return;
    }

    current = buffer;
    for (;;) {
        Event *event;

        token = COM_ParseExt(&current, qtrue);
        if (!token[0]) {
            break;
        }

        event = new Event(token);

        for (;;) {
            token = COM_ParseExt(&current, qfalse);
            if (!token[0]) {
                break;
            }

            event->AddToken(token);
        }

        ProcessEvent(event);
    }

    gi.FS_FreeFile(buffer);
}

void Player::GodCheat(Event *ev)
{
    const char *msg;

    if (ev->NumArgs() > 0) {
        if (ev->GetInteger(1)) {
            flags |= FL_GODMODE;
            // Also enable the god mode for the vehicle
            if (m_pVehicle) {
                m_pVehicle->flags |= FL_GODMODE;
            }
        } else {
            flags &= ~FL_GODMODE;
            // Also disable the god mode for the vehicle
            if (m_pVehicle) {
                m_pVehicle->flags &= ~FL_GODMODE;
            }
        }
    } else {
        if (flags & FL_GODMODE) {
            flags &= ~FL_GODMODE;
            if (m_pVehicle) {
                m_pVehicle->flags &= ~FL_GODMODE;
            }
        } else {
            flags |= FL_GODMODE;
            if (m_pVehicle) {
                m_pVehicle->flags |= FL_GODMODE;
            }
        }
    }

    if (ev->isSubclassOf(ConsoleEvent)) {
        if (!(flags & FL_GODMODE)) {
            msg = "CHEAT: godmode OFF\n";
        } else {
            msg = "CHEAT: godmode ON\n";
        }

        gi.SendServerCommand(edict - g_entities, "print \"%s\"", msg);
    }
}

void Player::Kill(Event *ev)
{
    if ((level.time - respawn_time) < 5) {
        return;
    }

    flags &= ~FL_GODMODE;
    health = 1;
    Damage(this, this, 10, origin, vec_zero, vec_zero, 0, DAMAGE_NO_PROTECTION, MOD_SUICIDE);
}

void Player::NoTargetCheat(Event *ev)
{
    const char *msg;

    flags ^= FL_NOTARGET;
    if (!(flags & FL_NOTARGET)) {
        msg = "notarget OFF\n";
    } else {
        msg = "notarget ON\n";
    }

    gi.SendServerCommand(edict - g_entities, "print \"%s\"", msg);
}

void Player::NoclipCheat(Event *ev)
{
    const char *msg;

    if (m_pVehicle) {
        msg = "Must exit vehicle first\n";
    } else if (m_pTurret) {
        msg = "Must exit turret first\n";
    } else if (getMoveType() == MOVETYPE_NOCLIP) {
        setMoveType(MOVETYPE_WALK);
        msg = "noclip OFF\n";

        // reset the state machine so that his animations are correct
        ResetState(NULL);
    } else {
        client->ps.feetfalling = false;
        movecontrol            = MOVECONTROL_LEGS;

        setMoveType(MOVETYPE_NOCLIP);
        msg = "noclip ON\n";
    }

    gi.SendServerCommand(edict - g_entities, "print \"%s\"", msg);
}

void Player::GameVersion(Event *ev)

{
    gi.SendServerCommand(edict - g_entities, "print \"%s : %s\n\"", GAMEVERSION, __DATE__);
}

void Player::SetFov(float newFov)
{
    fov = newFov;

    if (fov < 1) {
        fov = 80;
    } else if (fov > 160) {
        fov = 160;
    }
}

void Player::EventSetSelectedFov(Event *ev)
{
    if (ev->NumArgs() < 1) {
        gi.SendServerCommand(edict - g_entities, "print \"Fov = %d\n\"", (unsigned int)fov);
        return;
    }

    SetSelectedFov(ev->GetFloat(1));
    SetFov(selectedfov);
}

void Player::SetSelectedFov(float newFov)
{
    selectedfov = newFov;

    if (selectedfov < 1) {
        selectedfov = 80;
    } else if (selectedfov > 160) {
        selectedfov = 160;
    }

    /*
    if( g_gametype->integer != GT_SINGLE_PLAYER && !developer->integer )
    {
        if( selectedfov < 80 )
        {
            selectedfov = 80;
        }
        else if( selectedfov > 80 )
        {
            selectedfov = 80;
        }
    }
    */
}

/*
===============
CalcRoll

===============
*/
float Player::CalcRoll(void)

{
    float  sign;
    float  side;
    float  value;
    Vector l;

    angles.AngleVectors(NULL, &l, NULL);
    side = velocity * l;
    sign = side < 0 ? 4 : -4;
    side = fabs(side);

    value = sv_rollangle->value;

    if (side < sv_rollspeed->value) {
        side = side * value / sv_rollspeed->value;
    } else {
        side = value;
    }

    return side * sign;
}

//
// PMove Events
//
void Player::ProcessPmoveEvents(int event)
{
    float damage;

    switch (event) {
    case EV_NONE:
        break;
    case EV_FALL_SHORT:
    case EV_FALL_MEDIUM:
    case EV_FALL_FAR:
    case EV_FALL_FATAL:
        if (event == EV_FALL_FATAL) {
            if (g_protocol >= protocol_e::PROTOCOL_MOHTA_MIN) {
                damage = 101;
            } else {
                damage = max_health + 1.0f;
            }
        } else if (event == EV_FALL_FAR) {
            if (g_protocol >= protocol_e::PROTOCOL_MOHTA_MIN) {
                damage = 25;
            } else {
                damage = 20;
            }
        } else if (event == EV_FALL_MEDIUM) {
            if (g_protocol >= protocol_e::PROTOCOL_MOHTA_MIN) {
                damage = 15;
            } else {
                damage = 10;
            }
        } else {
            damage = 5;
        }

        if (g_protocol >= protocol_e::PROTOCOL_MOHTA_MIN) {
            // since 2.0, remove a percentage of the health
            damage = damage * (max_health / 100.0);
        }
        if (g_gametype->integer == GT_SINGLE_PLAYER || !DM_FLAG(DF_NO_FALLING)) {
            Damage(this, this, (int)damage, origin, vec_zero, vec_zero, 0, DAMAGE_NO_ARMOR, MOD_FALLING);
        }
        break;
    case EV_TERMINAL_VELOCITY:
        Sound("snd_fall", CHAN_VOICE);
        break;
    case EV_WATER_LEAVE: // foot leaves
        Sound("impact_playerleavewater", CHAN_AUTO);
        break;
    case EV_WATER_UNDER: // head touches
        Sound("impact_playersubmerge", CHAN_AUTO);
        break;
    case EV_WATER_CLEAR: // head leaves
        Sound("snd_gasp", CHAN_LOCAL);
        break;
    }
}

/*
=============
WorldEffects
=============
*/
void Player::WorldEffects(void)
{
    if (deadflag == DEAD_DEAD || getMoveType() == MOVETYPE_NOCLIP) {
        // if we are dead or no-cliping, no world effects
        return;
    }

    //
    // check for on fire
    //
    if (on_fire) {
        if (next_painsound_time < level.time) {
            next_painsound_time = level.time + 4;
            Sound("snd_onfire", CHAN_LOCAL);
        }
    }
}

/*
=============
AddBlend
=============
*/
void Player::AddBlend(float r, float g, float b, float a)

{
    float a2;
    float a3;

    if (a <= 0) {
        return;
    }

    // new total alpha
    a2 = blend[3] + (1 - blend[3]) * a;

    // fraction of color from old
    a3 = blend[3] / a2;

    blend[0] = blend[0] * a3 + r * (1 - a3);
    blend[1] = blend[1] * a3 + g * (1 - a3);
    blend[2] = blend[2] * a3 + b * (1 - a3);
    blend[3] = a2;
}

/*
=============
CalcBlend
=============
*/
void Player::CalcBlend(void)

{
    int    contents;
    Vector vieworg;

    client->ps.stats[STAT_ADDFADE] = 0;
    blend[0] = blend[1] = blend[2] = blend[3] = 0;

    // add for contents
    vieworg = m_vViewPos;

    contents = gi.pointcontents(vieworg, 0);

    if (contents & CONTENTS_SOLID) {
        // Outside of world
        //AddBlend( 0.8, 0.5, 0.0, 0.2 );
    } else if (contents & CONTENTS_LAVA) {
        AddBlend(level.lava_color[0], level.lava_color[1], level.lava_color[2], level.lava_alpha);
    } else if (contents & CONTENTS_WATER) {
        AddBlend(level.water_color[0], level.water_color[1], level.water_color[2], level.water_alpha);
    }

    // add for damage
    if (damage_alpha > 0) {
        AddBlend(damage_blend[0], damage_blend[1], damage_blend[2], damage_alpha);

        // drop the damage value
        damage_alpha -= 0.06f;
        if (damage_alpha < 0) {
            damage_alpha = 0;
        }
        client->ps.blend[0] = blend[0];
        client->ps.blend[1] = blend[1];
        client->ps.blend[2] = blend[2];
        client->ps.blend[3] = blend[3];
    }

    // Do the cinematic fading
    float alpha = 1;

    level.m_fade_time -= level.frametime;

    // Return if we are completely faded in
    if ((level.m_fade_time <= 0) && (level.m_fade_type == fadein)) {
        client->ps.blend[3] = 0 + damage_alpha;
        return;
    }

    // If we are faded out, and another fade out is coming in, then don't bother
    if ((level.m_fade_time_start > 0) && (level.m_fade_type == fadeout)) {
        if (client->ps.blend[3] >= 1) {
            return;
        }
    }

    if (level.m_fade_time_start > 0) {
        alpha = level.m_fade_time / level.m_fade_time_start;
    }

    if (level.m_fade_type == fadeout) {
        alpha = 1.0f - alpha;
    }

    if (alpha < 0) {
        alpha = 0;
    }

    if (alpha > 1) {
        alpha = 1;
    }

    if (level.m_fade_style == additive) {
        client->ps.blend[0]            = level.m_fade_color[0] * level.m_fade_alpha * alpha;
        client->ps.blend[1]            = level.m_fade_color[1] * level.m_fade_alpha * alpha;
        client->ps.blend[2]            = level.m_fade_color[2] * level.m_fade_alpha * alpha;
        client->ps.blend[3]            = level.m_fade_alpha * alpha;
        client->ps.stats[STAT_ADDFADE] = 1;
    } else {
        client->ps.blend[0]            = level.m_fade_color[0];
        client->ps.blend[1]            = level.m_fade_color[1];
        client->ps.blend[2]            = level.m_fade_color[2];
        client->ps.blend[3]            = level.m_fade_alpha * alpha;
        client->ps.stats[STAT_ADDFADE] = 0;
    }
}

/*
===============
P_DamageFeedback

Handles color blends and view kicks
===============
*/

void Player::DamageFeedback(void)
{
    float  realcount;
    float  count;
    vec3_t vDir;
    str    painAnim;
    int    animnum;

    // if we are dead, don't setup any feedback
    if (IsDead()) {
        damage_count = 0;
        damage_blood = 0;
        damage_alpha = 0;
        VectorClear(damage_angles);
        return;
    }

    if (damage_count) {
        // decay damage_count over time
        damage_count *= 0.8f;
        damage_from *= 0.8f;
        damage_angles *= 0.8f;
        if (damage_count < 0.1f) {
            damage_count = 0;
            damage_from  = Vector(0, 0, 0);
        }
    }

    // total points of damage shot at the player this frame
    if (!damage_blood) {
        // didn't take any damage
        return;
    }

    VectorNormalize2(damage_from, vDir);

    damage_angles.x -=
        DotProduct(vDir, orientation[0]) * damage_blood * g_viewkick_pitch->value * g_viewkick_dmmult->value;
    damage_angles.x = Q_clamp_float(damage_angles.x, -30, 30);

    damage_angles.y -=
        DotProduct(vDir, orientation[1]) * damage_blood * g_viewkick_yaw->value * g_viewkick_dmmult->value;
    damage_angles.y = Q_clamp_float(damage_angles.y, -30, 30);

    damage_angles.z +=
        DotProduct(vDir, orientation[2]) * damage_blood * g_viewkick_roll->value * g_viewkick_dmmult->value;
    damage_angles.z = Q_clamp_float(damage_angles.y, -25, 25);

    damage_count += damage_blood;
    count     = damage_blood;
    realcount = count;
    if (count < 10) {
        // always make a visible effect
        count = 10;
    }

    // the total alpha of the blend is always proportional to count
    if (damage_alpha < 0) {
        damage_alpha = 0;
    }

    damage_alpha += count * 0.001;
    if (damage_alpha < 0.2f) {
        damage_alpha = 0.2f;
    }
    if (damage_alpha > 0.6f) {
        // don't go too saturated
        damage_alpha = 0.6f;
    }

    // the color of the blend will vary based on how much was absorbed
    // by different armors
    damage_blend = vec_zero;
    if (damage_blood) {
        damage_blend += (damage_blood / realcount) * bcolor;
    }

    //
    // Since 2.0: Try to find and play pain animation
    //
    if (getMoveType() == MOVETYPE_PORTABLE_TURRET) {
        // use mg42 pain animation
        painAnim = "mg42_tripod_";
    } else {
        Weapon     *pWeap;
        const char *itemName;
        // try to find an animation

        pWeap = GetActiveWeapon(WEAPON_MAIN);
        if (pWeap) {
            int weapon_class;

            weapon_class = pWeap->GetWeaponClass();
            if (weapon_class & WEAPON_CLASS_PISTOL) {
                painAnim = "pistol_";
            } else if (weapon_class & WEAPON_CLASS_RIFLE) {
                painAnim = "rifle_";
            } else if (weapon_class & WEAPON_CLASS_SMG) {
                // get the animation name from the item name
                itemName = pWeap->GetItemName();

                if (!Q_stricmp(itemName, "MP40")) {
                    painAnim = "mp40_";
                } else if (!Q_stricmp(itemName, "Sten Mark II")) {
                    painAnim = "sten_";
                } else {
                    painAnim = "smg_";
                }
            } else if (weapon_class & WEAPON_CLASS_MG) {
                itemName = pWeap->GetItemName();

                if (!Q_stricmp(itemName, "StG 44")) {
                    painAnim = "mp44_";
                } else {
                    painAnim = "mg_";
                }
            } else if (weapon_class & WEAPON_CLASS_GRENADE) {
                itemName = pWeap->GetItemName();

                // 2.30: use landmine animations
                if (!Q_stricmp(itemName, "Minedetector")) {
                    painAnim = "minedetector_";
                } else if (!Q_stricmp(itemName, "Minensuchgerat")) {
                    painAnim = "minedetectoraxis_";
                } else if (!Q_stricmp(itemName, "LandmineAllies")) {
                    painAnim = "mine_";
                } else if (!Q_stricmp(itemName, "LandmineAxis")) {
                    painAnim = "mine_";
                } else if (!Q_stricmp(itemName, "LandmineAxis")) {
                    painAnim = "grenade_";
                }
            } else if (weapon_class & WEAPON_CLASS_HEAVY) {
                itemName = pWeap->GetItemName();

                if (!Q_stricmp(itemName, "Shotgun")) {
                    painAnim = "shotgun_";
                } else {
                    // Defaults to bazooka
                    painAnim = "bazooka_";
                }
            } else {
                itemName = pWeap->GetItemName();

                if (!Q_stricmp(itemName, "Packed MG42 Turret")) {
                    painAnim = "mg42_";
                } else {
                    // Default animation if not found
                    painAnim = "unarmed_";
                }
            }
        } else {
            painAnim = "unarmed_";
        }

        // use the animation based on the movement
        if (m_iMovePosFlags & MPF_POSITION_CROUCHING) {
            painAnim += "crouch_";
        } else {
            painAnim += "stand_";
        }
    }

    painAnim += "hit_";

    if (pain_dir == PAIN_REAR || pain_location == HITLOC_TORSO_MID || HITLOC_TORSO_LOWER) {
        painAnim += "back";
    } else {
        switch (pain_location) {
        case HITLOC_HEAD:
        case HITLOC_HELMET:
        case HITLOC_NECK:
            painAnim += "head";
            break;
        case HITLOC_TORSO_UPPER:
        case HITLOC_TORSO_MID:
            painAnim += "uppertorso";
            break;
        case HITLOC_TORSO_LOWER:
        case HITLOC_PELVIS:
            painAnim += "lowertorso";
            break;
        case HITLOC_R_ARM_UPPER:
        case HITLOC_R_ARM_LOWER:
        case HITLOC_R_HAND:
            painAnim += "rarm";
            break;
        case HITLOC_L_ARM_UPPER:
        case HITLOC_L_ARM_LOWER:
        case HITLOC_L_HAND:
            painAnim += "larm";
            break;
        case HITLOC_R_LEG_UPPER:
        case HITLOC_L_LEG_UPPER:
        case HITLOC_R_LEG_LOWER:
        case HITLOC_L_LEG_LOWER:
        case HITLOC_R_FOOT:
        case HITLOC_L_FOOT:
            painAnim += "leg";
            break;
        default:
            painAnim += "uppertorso";
            break;
        }
    }

    animnum = gi.Anim_NumForName(edict->tiki, painAnim.c_str());
    if (animnum == -1) {
        gi.DPrintf("WARNING: Could not find player pain animation '%s'\n", painAnim.c_str());
    } else {
        NewAnim(animnum, EV_Player_AnimLoop_Pain, ANIMSLOT_PAIN);
        RestartAnimSlot(ANIMSLOT_PAIN);
        m_sPainAnim   = painAnim;
        m_fPainBlend  = 1.f;
        animdone_Pain = false;
    }

    //
    // clear totals
    //
    damage_blood = 0;

    if (IsSubclassOfPlayer()) {
        damage_count  = 0;
        damage_blood  = 0;
        damage_alpha  = 0;
        damage_angles = vec_zero;
    }
}

void Player::GetPlayerView(Vector *pos, Vector *angle)
{
    if (pos) {
        *pos = origin;
        pos->z += viewheight;
    }

    if (angle) {
        *angle = Vector(client->ps.viewangles);
    }
}

void Player::SetPlayerView(
    Camera *camera, Vector position, float cameraoffset, Vector ang, Vector vel, float camerablend[4], float camerafov
)
{
    VectorCopy(ang, client->ps.viewangles);
    client->ps.viewheight = cameraoffset;

    VectorCopy(position, client->ps.origin);
    VectorCopy(vel, client->ps.velocity);

    /*
    client->ps.blend[ 0 ] = camerablend[ 0 ];
    client->ps.blend[ 1 ] = camerablend[ 1 ];
    client->ps.blend[ 2 ] = camerablend[ 2 ];
    client->ps.blend[ 3 ] = camerablend[ 3 ];
    */

    client->ps.fov = camerafov;

    if (camera) {
        if (camera->IsSubclassOfCamera()) {
            VectorCopy(camera->angles, client->ps.camera_angles);
            VectorCopy(camera->origin, client->ps.camera_origin);

            Vector vOfs = camera->GetPositionOffset();
            VectorCopy(vOfs, client->ps.camera_posofs);

            client->ps.pm_flags |= PMF_CAMERA_VIEW;

            if (camera->ShowQuakes()) {
                client->ps.pm_flags |= PMF_DAMAGE_ANGLES;
            } else {
                client->ps.pm_flags &= ~PMF_DAMAGE_ANGLES;
            }

            //
            // clear out the flags, but preserve the CF_CAMERA_CUT_BIT
            //
            client->ps.camera_flags = client->ps.camera_flags & CF_CAMERA_CUT_BIT;
        } else {
            Vector vVec;

            if (camera->IsSubclassOfPlayer()) {
                Vector  vPos;
                Player *pPlayer = (Player *)camera;

                GetSpectateFollowOrientation(pPlayer, vPos, vVec);

                VectorCopy(vVec, client->ps.camera_angles);
                VectorCopy(vPos, client->ps.camera_origin);

                SetViewAngles(vVec);

                vPos[2] -= viewheight;
                setOrigin(vPos);

                vVec.setXYZ(0, 0, 0);
            } else {
                VectorCopy(ang, client->ps.camera_angles);
                VectorCopy(position, client->ps.camera_angles);

                vVec.setXYZ(0, 0, 0);
            }

            VectorCopy(vVec, client->ps.camera_posofs);
            client->ps.pm_flags |= PMF_CAMERA_VIEW;
            client->ps.camera_flags = client->ps.camera_flags & CF_CAMERA_CUT_BIT;
        }
    } else {
        client->ps.pm_flags &= ~PMF_CAMERA_VIEW;
        //
        // make sure the third person camera is setup correctly.
        //

        if (getMoveType() != MOVETYPE_NOCLIP) {
            qboolean do_cut;
            int      camera_type;

            if (currentState_Torso) {
                camera_type = currentState_Torso->getCameraType();
            } else {
                camera_type = CAMERA_BEHIND;
            }
            if (last_camera_type != camera_type) {
                //
                // clear out the flags, but preserve the CF_CAMERA_CUT_BIT
                //
                client->ps.camera_flags = client->ps.camera_flags & CF_CAMERA_CUT_BIT;
                do_cut                  = qtrue;
                switch (camera_type) {
                case CAMERA_TOPDOWN:
                    client->ps.camera_flags |= CF_CAMERA_ANGLES_IGNORE_PITCH;
                    client->ps.camera_offset[PITCH] = -75;
                    client->ps.camera_flags |= CF_CAMERA_ANGLES_ALLOWOFFSET;
                    do_cut = qfalse;
                    break;
                case CAMERA_FRONT:
                    client->ps.camera_flags |= CF_CAMERA_ANGLES_IGNORE_PITCH;
                    client->ps.camera_flags |= CF_CAMERA_ANGLES_ALLOWOFFSET;
                    client->ps.camera_offset[YAW]   = 180;
                    client->ps.camera_offset[PITCH] = 0;
                    break;
                case CAMERA_SIDE:
                    client->ps.camera_flags |= CF_CAMERA_ANGLES_IGNORE_PITCH;
                    client->ps.camera_flags |= CF_CAMERA_ANGLES_ALLOWOFFSET;
                    // randomly invert the YAW
                    if (G_Random(1) > 0.5f) {
                        client->ps.camera_offset[YAW] = -90;
                    } else {
                        client->ps.camera_offset[YAW] = 90;
                    }
                    client->ps.camera_offset[PITCH] = 0;
                    break;
                case CAMERA_SIDE_LEFT:
                    client->ps.camera_flags |= CF_CAMERA_ANGLES_IGNORE_PITCH;
                    client->ps.camera_flags |= CF_CAMERA_ANGLES_ALLOWOFFSET;
                    client->ps.camera_offset[YAW]   = 90;
                    client->ps.camera_offset[PITCH] = 0;
                    break;
                case CAMERA_SIDE_RIGHT:
                    client->ps.camera_flags |= CF_CAMERA_ANGLES_IGNORE_PITCH;
                    client->ps.camera_flags |= CF_CAMERA_ANGLES_ALLOWOFFSET;
                    client->ps.camera_offset[YAW]   = -90;
                    client->ps.camera_offset[PITCH] = 0;
                    break;
                case CAMERA_BEHIND_FIXED:
                    do_cut                          = qfalse;
                    client->ps.camera_offset[YAW]   = 0;
                    client->ps.camera_offset[PITCH] = 0;
                    client->ps.camera_flags |= CF_CAMERA_ANGLES_ALLOWOFFSET;
                    break;
                case CAMERA_BEHIND_NOPITCH:
                    do_cut = qfalse;
                    client->ps.camera_flags |= CF_CAMERA_ANGLES_IGNORE_PITCH;
                    client->ps.camera_offset[YAW]   = 0;
                    client->ps.camera_offset[PITCH] = 0;
                    break;
                case CAMERA_BEHIND:
                    do_cut                          = qfalse;
                    client->ps.camera_offset[YAW]   = 0;
                    client->ps.camera_offset[PITCH] = 0;
                    break;
                default:
                    do_cut                          = qfalse;
                    client->ps.camera_offset[YAW]   = 0;
                    client->ps.camera_offset[PITCH] = 0;
                    break;
                }
                last_camera_type = camera_type;
                if (do_cut) {
                    CameraCut();
                }
            }
        } else {
            client->ps.camera_flags = client->ps.camera_flags & CF_CAMERA_CUT_BIT;
        }

        //
        // these are explicitly not cleared so that when the client lerps it still has the last
        // camera position for reference. Additionally this causes no extra hits to the network
        // traffic.
        //
        //VectorClear( client->ps.camera_angles );
        //VectorClear( client->ps.camera_origin );
    }

#define EARTHQUAKE_SCREENSHAKE_PITCH 2
#define EARTHQUAKE_SCREENSHAKE_YAW   2
#define EARTHQUAKE_SCREENSHAKE_ROLL  3

    if (level.earthquake_magnitude != 0.0f) {
        client->ps.damage_angles[PITCH] = G_CRandom() * level.earthquake_magnitude * EARTHQUAKE_SCREENSHAKE_PITCH;
        client->ps.damage_angles[YAW]   = G_CRandom() * level.earthquake_magnitude * EARTHQUAKE_SCREENSHAKE_YAW;
        client->ps.damage_angles[ROLL]  = G_CRandom() * level.earthquake_magnitude * EARTHQUAKE_SCREENSHAKE_ROLL;
    } else if (damage_count) {
        client->ps.damage_angles[PITCH] = damage_angles[PITCH];
        client->ps.damage_angles[YAW]   = damage_angles[YAW];
        client->ps.damage_angles[ROLL]  = damage_angles[ROLL];
    } else {
        VectorClear(client->ps.damage_angles);
    }

    if (m_vViewVariation != vec_zero) {
        for (int i = 0; i < 3; i++) {
            if (m_vViewVariation[i] == 0.0f) {
                continue;
            }

            client->ps.damage_angles[i] += G_CRandom() * m_vViewVariation[i];

            m_vViewVariation[i] = m_vViewVariation[i] - m_vViewVariation[i] * level.frametime * 8.0f;

            if (m_vViewVariation[i] < 0.01f) {
                m_vViewVariation[i] = 0.0f;
            }
        }
    }
}

void Player::SetupView(void)
{
    // if we currently are not in a camera or the camera we are looking through is automatic, evaluate our camera choices

    if (actor_to_watch || actor_camera) {
        Vector   dir;
        Vector   watch_angles;
        float    dist = 0;
        Vector   focal_point;
        Vector   left;
        trace_t  trace;
        qboolean delete_actor_camera = false;
        Vector   camera_mins;
        Vector   camera_maxs;

        if (actor_to_watch) {
            dir  = actor_to_watch->origin - origin;
            dist = dir.length();
        }

        // See if we still want to watch this actor

        if (!actor_to_watch || dist > 150 || actor_to_watch->deadflag) {
            delete_actor_camera = true;
        } else {
            // Create the camera if we don't have one yet

            if (!actor_camera) {
                actor_camera = new Camera();

                if (G_Random() < .5) {
                    actor_camera_right          = true;
                    starting_actor_camera_right = true;
                } else {
                    actor_camera_right          = false;
                    starting_actor_camera_right = false;
                }
            }

            // Setup the new position of the actor camera

            // Go a little above the view height

            actor_camera->origin = origin;
            actor_camera->origin[2] += DEFAULT_VIEWHEIGHT + 10;

            // Find the focal point ( either the actor's watch offset or top of the bounding box)

            if (actor_to_watch->watch_offset != vec_zero) {
                MatrixTransformVector(actor_to_watch->watch_offset, actor_to_watch->orientation, focal_point);
                focal_point += actor_to_watch->origin;
            } else {
                focal_point    = actor_to_watch->origin;
                focal_point[2] = actor_to_watch->maxs[2];
            }

            // Shift the camera back just a little

            dir = focal_point - actor_camera->origin;
            dir.normalize();
            actor_camera->origin -= dir * 15;

            // Shift the camera a little to the left or right

            watch_angles = dir.toAngles();
            watch_angles.AngleVectors(NULL, &left);

            if (actor_camera_right) {
                actor_camera->origin -= left * 15;
            } else {
                actor_camera->origin += left * 15;
            }

            // Make sure this camera position is ok

            camera_mins = "-5 -5 -5";
            camera_maxs = "5 5 5";

            trace = G_Trace(
                actor_camera->origin,
                camera_mins,
                camera_maxs,
                actor_camera->origin,
                actor_camera,
                MASK_DEADSOLID,
                false,
                "SetupView"
            );

            if (trace.startsolid) {
                // Try other side

                if (actor_camera_right == starting_actor_camera_right) {
                    if (actor_camera_right) {
                        actor_camera->origin += left * 30;
                    } else {
                        actor_camera->origin -= left * 30;
                    }

                    actor_camera_right = !actor_camera_right;

                    trace = G_Trace(
                        actor_camera->origin,
                        camera_mins,
                        camera_maxs,
                        actor_camera->origin,
                        actor_camera,
                        MASK_DEADSOLID,
                        false,
                        "SetupView2"
                    );

                    if (trace.startsolid) {
                        // Both spots have failed stop doing actor camera
                        delete_actor_camera = true;
                    }
                } else {
                    // Both spots have failed stop doing actor camera
                    delete_actor_camera = true;
                }
            }

            if (!delete_actor_camera) {
                // Set the camera's position

                actor_camera->setOrigin(actor_camera->origin);

                // Set the camera's angles

                dir          = focal_point - actor_camera->origin;
                watch_angles = dir.toAngles();
                actor_camera->setAngles(watch_angles);

                // Set this as our camera

                SetCamera(actor_camera, .5);
            }
        }

        if (delete_actor_camera) {
            // Get rid of this camera

            actor_to_watch = NULL;

            if (actor_camera) {
                delete actor_camera;
                actor_camera = NULL;
                SetCamera(NULL, .5);
            }
        }
    } else if ((level.automatic_cameras.NumObjects() > 0) && (!camera || camera->IsAutomatic())) {
        int     i;
        float   score, bestScore;
        Camera *cam, *bestCamera;

        bestScore  = 999;
        bestCamera = NULL;
        for (i = 1; i <= level.automatic_cameras.NumObjects(); i++) {
            cam   = level.automatic_cameras.ObjectAt(i);
            score = cam->CalculateScore(this, currentState_Torso->getName());
            // if this is our current camera, scale down the score a bit to favor it.
            if (cam == camera) {
                score *= 0.9f;
            }

            if (score < bestScore) {
                bestScore  = score;
                bestCamera = cam;
            }
        }
        if (bestScore <= 1.0f) {
            // we have a camera to switch to
            if (bestCamera != camera) {
                float time;

                if (camera) {
                    camera->AutomaticStop(this);
                }
                time = bestCamera->AutomaticStart(this);
                SetCamera(bestCamera, time);
            }
        } else {
            // we don't have a camera to switch to
            if (camera) {
                float time;

                time = camera->AutomaticStop(this);
                SetCamera(NULL, time);
            }
        }
    }

    // If there is no camera, use the player's view
    if (!camera) {
        if (g_gametype->integer != GT_SINGLE_PLAYER && IsSpectator() && m_iPlayerSpectating != 0) {
            gentity_t *ent = g_entities + m_iPlayerSpectating - 1;

            if (ent->inuse && ent->entity && ent->entity->deadflag <= DEAD_DYING) {
                Player *m_player = (Player *)ent->entity;
                Vector  vAngles;

                m_player->GetPlayerView(NULL, &vAngles);

                SetPlayerView(
                    (Camera *)m_player,
                    m_player->origin,
                    m_player->viewheight,
                    vAngles,
                    m_player->velocity,
                    blend,
                    m_player->fov
                );
            } else {
                SetPlayerView(NULL, origin, viewheight, v_angle, velocity, blend, fov);
            }
        } else {
            SetPlayerView(NULL, origin, viewheight, v_angle, velocity, blend, fov);
        }
    } else {
        SetPlayerView(camera, origin, viewheight, v_angle, velocity, blend, camera->Fov());
    }
}

Vector Player::GetAngleToTarget(Entity *ent, str tag, float yawclamp, float pitchclamp, Vector baseangles)

{
    assert(ent);

    if (ent) {
        Vector        delta, angs;
        orientation_t tag_or;

        int tagnum = gi.Tag_NumForName(edict->tiki, tag.c_str());

        if (tagnum < 0) {
            return Vector(0, 0, 0);
        }

        GetTagPositionAndOrientation(tagnum, &tag_or);

        delta = ent->centroid - tag_or.origin;
        delta.normalize();

        angs = delta.toAngles();

        AnglesSubtract(angs, baseangles, angs);

        angs[PITCH] = AngleNormalize180(angs[PITCH]);
        angs[YAW]   = AngleNormalize180(angs[YAW]);

        if (angs[PITCH] > pitchclamp) {
            angs[PITCH] = pitchclamp;
        } else if (angs[PITCH] < -pitchclamp) {
            angs[PITCH] = -pitchclamp;
        }

        if (angs[YAW] > yawclamp) {
            angs[YAW] = yawclamp;
        } else if (angs[YAW] < -yawclamp) {
            angs[YAW] = -yawclamp;
        }

        return angs;
    } else {
        return Vector(0, 0, 0);
    }
}

void Player::DebugWeaponTags(int controller_tag, Weapon *weapon, str weapon_tagname)

{
    int           i;
    orientation_t bone_or, tag_weapon_or, barrel_or, final_barrel_or;

    GetTagPositionAndOrientation(edict->s.bone_tag[controller_tag], &bone_or);
    //G_DrawCoordSystem( Vector( bone_or.origin ), Vector( bone_or.axis[0] ), Vector( bone_or.axis[1] ), Vector( bone_or.axis[2] ), 20 );

    GetTagPositionAndOrientation(gi.Tag_NumForName(edict->tiki, weapon_tagname), &tag_weapon_or);
    //G_DrawCoordSystem( Vector( tag_weapon_or.origin ), Vector( tag_weapon_or.axis[0] ), Vector( tag_weapon_or.axis[1] ), Vector( tag_weapon_or.axis[2] ), 40 );

    weapon->GetRawTag("tag_barrel", &barrel_or);
    VectorCopy(tag_weapon_or.origin, final_barrel_or.origin);

    for (i = 0; i < 3; i++) {
        VectorMA(final_barrel_or.origin, barrel_or.origin[i], tag_weapon_or.axis[i], final_barrel_or.origin);
    }

    MatrixMultiply(barrel_or.axis, tag_weapon_or.axis, final_barrel_or.axis);
    //G_DrawCoordSystem( Vector( final_barrel_or.origin ), Vector( final_barrel_or.axis[0] ), Vector( final_barrel_or.axis[1] ), Vector( final_barrel_or.axis[2] ), 80 );

#if 0
   if ( g_crosshair->integer )
      {
      trace_t trace;
      Vector  start,end,ang,dir,delta;
      vec3_t  mat[3];

      AnglesToAxis( v_angle, mat );

      dir   = mat[0];
      start = final_barrel_or.origin;
      end   = start + ( dir *  MAX_MAP_BOUNDS ); 

      G_DrawCoordSystem( start, Vector( mat[0] ), Vector( mat[1] ), Vector( mat[2] ), 80 );
      
      trace = G_Trace( start, vec_zero, vec_zero, end, this, MASK_PROJECTILE|MASK_WATER, qfalse, "Crosshair" );
      crosshair->setOrigin( trace.endpos );

      delta = trace.endpos - start;
      float length = delta.length();
      float scale  = g_crosshair_maxscale->value * length / MAX_MAP_BOUNDS;
      
      if ( scale < 1 )
         scale = 1;

      crosshair->setScale( scale );

      if ( trace.ent )
         {
         vectoangles( trace.plane.normal, ang );
         }
      else
         {
         vectoangles( dir, ang );
         }

      crosshair->setAngles( ang );
      }
#endif
}

void Player::AcquireTarget(void) {}

void Player::RemoveTarget(Entity *ent_to_remove) {}

void Player::AutoAim(void) {}

/*
===============
PlayerAngles
===============
*/
void Player::PlayerAngles(void)
{
    if (getMoveType() == MOVETYPE_PORTABLE_TURRET) {
        PortableTurret *portableTurret = static_cast<PortableTurret *>(m_pTurret.Pointer());
        angles[0]                      = portableTurret->GetGroundPitch();
        angles[1]                      = portableTurret->GetStartYaw();
    }

    PmoveAdjustAngleSettings(v_angle, angles, &client->ps, &edict->s);

    SetViewAngles(v_angle);
    setAngles(angles);
}

void Player::FinishMove(void)
{
    //
    // If the origin or velocity have changed since ClientThink(),
    // update the pmove values.  This will happen when the client
    // is pushed by a bmodel or kicked by an explosion.
    //
    // If it wasn't updated here, the view position would lag a frame
    // behind the body position when pushed -- "sinking into plats",
    //
    if (!(client->ps.pm_flags & PMF_FROZEN) && !(client->ps.pm_flags & PMF_NO_MOVE)) {
        origin.copyTo(client->ps.origin);
        velocity.copyTo(client->ps.velocity);
    }

    // This check is in mohaa but the animation will look bad
    if (!(client->ps.pm_flags & PMF_FROZEN)) {
        PlayerAngles();
        AdjustAnimBlends();
    }

    // burn from lava, etc
    WorldEffects();

    // determine the view offsets
    DamageFeedback();
    CalcBlend();

    if (g_gametype->integer != GT_SINGLE_PLAYER && g_smoothClients->integer && !IsSubclassOfBot()) {
        VectorCopy(client->ps.velocity, edict->s.pos.trDelta);
        edict->s.pos.trTime = client->ps.commandTime;
    } else {
        VectorClear(edict->s.pos.trDelta);
        edict->s.pos.trTime = 0;
    }
}

void Player::CopyStats(Player *player)
{
    gentity_t *ent;
    int        i;

    origin = player->origin;
    SetViewAngles(player->GetViewAngles());

    client->ps.bobCycle = player->client->ps.bobCycle;

    client->ps.pm_flags |=
        player->client->ps.pm_flags & (PMF_DUCKED | PMF_VIEW_DUCK_RUN | PMF_VIEW_JUMP_START | PMF_VIEW_PRONE);

    memcpy(&client->ps.stats, &player->client->ps.stats, sizeof(client->ps.stats));
    memcpy(&client->ps.activeItems, &player->client->ps.activeItems, sizeof(client->ps.activeItems));
    memcpy(&client->ps.ammo_name_index, &player->client->ps.ammo_name_index, sizeof(client->ps.ammo_name_index));
    memcpy(&client->ps.ammo_amount, &player->client->ps.ammo_amount, sizeof(client->ps.ammo_amount));
    memcpy(&client->ps.max_ammo_amount, &player->client->ps.max_ammo_amount, sizeof(client->ps.max_ammo_amount));

    VectorCopy(player->client->ps.origin, client->ps.origin);
    VectorCopy(player->client->ps.velocity, client->ps.velocity);

    if (client->ps.iViewModelAnim != player->client->ps.iViewModelAnim) {
        ViewModelAnim(player->m_sVMcurrent, qfalse, 0);
    } else if (client->ps.iViewModelAnimChanged != player->client->ps.iViewModelAnimChanged) {
        ViewModelAnim(player->m_sVMcurrent, qtrue, 0);
    }

    client->ps.gravity = player->client->ps.gravity;
    client->ps.speed   = player->client->ps.speed;

    // copy angles
    memcpy(&client->ps.delta_angles, &player->client->ps.delta_angles, sizeof(client->ps.delta_angles));

    memcpy(&client->ps.blend, &player->client->ps.blend, sizeof(client->ps.blend));
    memcpy(&client->ps.damage_angles, &player->client->ps.damage_angles, sizeof(client->ps.damage_angles));
    memcpy(&client->ps.viewangles, &player->client->ps.viewangles, sizeof(client->ps.delta_angles));

    // copy camera stuff
    //memcpy( &client->ps.camera_origin, &player->client->ps.camera_origin, sizeof( client->ps.camera_origin ) );
    //memcpy( &client->ps.camera_angles, &player->client->ps.camera_angles, sizeof( client->ps.camera_angles ) );
    //memcpy( &client->ps.camera_offset, &player->client->ps.camera_offset, sizeof( client->ps.camera_offset ) );
    //memcpy( &client->ps.camera_posofs, &player->client->ps.camera_posofs, sizeof( client->ps.camera_posofs ) );
    //client->ps.camera_time = player->client->ps.camera_time;
    //client->ps.camera_flags = player->client->ps.camera_flags;

    client->ps.fLeanAngle = player->client->ps.fLeanAngle;
    client->ps.fov        = player->client->ps.fov;

    client->ps.viewheight      = player->client->ps.viewheight;
    client->ps.walking         = player->client->ps.walking;
    client->ps.groundPlane     = player->client->ps.groundPlane;
    client->ps.groundEntityNum = player->client->ps.groundEntityNum;
    memcpy(&client->ps.groundTrace, &player->client->ps.groundTrace, sizeof(trace_t));

    edict->s.eFlags &= ~EF_UNARMED;
    edict->r.svFlags &= ~SVF_NOCLIENT;
    edict->s.renderfx &= ~RF_DONTDRAW;

    player->edict->r.svFlags |= SVF_PORTAL;
    player->edict->r.singleClient = client->ps.clientNum;

    edict->r.svFlags |= SVF_SINGLECLIENT;
    edict->r.singleClient = client->ps.clientNum;

    client->ps.pm_flags |= PMF_FROZEN | PMF_NO_MOVE | PMF_NO_PREDICTION;

    memcpy(&edict->s.frameInfo, &player->edict->s.frameInfo, sizeof(edict->s.frameInfo));

    DetachAllChildren(NULL);

    for (i = 0; i < MAX_MODEL_CHILDREN; i++) {
        Entity *dest;

        if (player->children[i] == ENTITYNUM_NONE) {
            continue;
        }

        ent = g_entities + player->children[i];

        if (!ent->inuse || !ent->entity) {
            continue;
        }

        dest = new Entity;

        CloneEntity(dest, ent->entity);

        dest->edict->r.svFlags |= SVF_SINGLECLIENT;
        dest->edict->r.singleClient = client->ps.clientNum;

        dest->edict->s.modelindex   = ent->entity->edict->s.modelindex;
        dest->edict->tiki           = ent->entity->edict->tiki;
        dest->edict->s.actionWeight = ent->entity->edict->s.actionWeight;
        memcpy(&dest->edict->s.frameInfo, &ent->entity->edict->s.frameInfo, sizeof(dest->edict->s.frameInfo));
        dest->CancelPendingEvents();
        dest->attach(entnum, ent->entity->edict->s.tag_num);

        dest->PostEvent(EV_DetachAllChildren, level.frametime);
    }
}

void Player::UpdateStats(void)
{
    int    i, count;
    Vector vObjectiveLocation;
    float  healthfrac;
    float  healfrac;

    //
    // Health
    //

    if (g_spectatefollow_firstperson->integer && IsSpectator() && m_iPlayerSpectating != 0) {
        //
        // Openmohaa addition
        // First-person spectate
        //
        gentity_t *ent = g_entities + (m_iPlayerSpectating - 1);

        if (ent->inuse && ent->entity && ent->entity->deadflag <= DEAD_DYING) {
            CopyStats((Player *)ent->entity);
            return;
        }
    }

    if (g_gametype->integer == GT_SINGLE_PLAYER) {
        client->ps.stats[STAT_TEAM]              = TEAM_ALLIES;
        client->ps.stats[STAT_KILLS]             = 0;
        client->ps.stats[STAT_DEATHS]            = 0;
        client->ps.stats[STAT_HIGHEST_SCORE]     = 0;
        client->ps.stats[STAT_ATTACKERCLIENT]    = -1;
        client->ps.stats[STAT_INFOCLIENT]        = -1;
        client->ps.stats[STAT_INFOCLIENT_HEALTH] = 0;

        vObjectiveLocation = level.m_vObjectiveLocation;
    } else {
        client->ps.stats[STAT_TEAM] = dm_team;

        if (g_gametype->integer >= GT_TEAM && current_team != NULL) {
            client->ps.stats[STAT_KILLS]  = current_team->m_teamwins;
            client->ps.stats[STAT_DEATHS] = current_team->m_iDeaths;
        } else {
            client->ps.stats[STAT_KILLS]  = num_kills;
            client->ps.stats[STAT_DEATHS] = num_deaths;
        }

        if (g_gametype->integer < GT_TEAM) {
            gentity_t *ent;
            int        i;
            int        bestKills = -9999;

            // Get the best player
            for (i = 0, ent = g_entities; i < game.maxclients; i++, ent++) {
                if (!ent->inuse || !ent->client || !ent->entity) {
                    continue;
                }

                Player *p = (Player *)ent->entity;
                if (p->GetNumKills() > bestKills) {
                    bestKills = p->GetNumKills();
                }
            }

            client->ps.stats[STAT_HIGHEST_SCORE] = bestKills;
        } else {
            if (dmManager.GetTeamAxis()->m_teamwins > dmManager.GetTeamAllies()->m_teamwins) {
                client->ps.stats[STAT_HIGHEST_SCORE] = dmManager.GetTeamAxis()->m_teamwins;
            } else {
                client->ps.stats[STAT_HIGHEST_SCORE] = dmManager.GetTeamAllies()->m_teamwins;
            }
        }

        if (!pAttackerDistPointer) {
            client->ps.stats[STAT_ATTACKERCLIENT] = -1;
        } else if (fAttackerDispTime <= level.time && deadflag == DEAD_NO) {
            pAttackerDistPointer                  = NULL;
            client->ps.stats[STAT_ATTACKERCLIENT] = -1;
        } else {
            client->ps.stats[STAT_ATTACKERCLIENT] = pAttackerDistPointer->edict - g_entities;
        }

        client->ps.stats[STAT_INFOCLIENT]        = -1;
        client->ps.stats[STAT_INFOCLIENT_HEALTH] = 0;

        if (IsSpectator() || g_gametype->integer >= GT_TEAM) {
            if (m_iPlayerSpectating && IsSpectator()) {
                gentity_t *ent = g_entities + (m_iPlayerSpectating - 1);

                if (ent->inuse && ent->entity && deadflag < DEAD_DEAD) {
                    m_iInfoClient       = ent - g_entities;
                    m_iInfoClientHealth = ent->entity->health;
                    m_fInfoClientTime   = level.time;

                    float percent = ent->entity->health / ent->entity->max_health * 100.0f;

                    if (percent > 0.0f && percent < 1.0f) {
                        percent = 1.0f;
                    }

                    client->ps.stats[STAT_INFOCLIENT_HEALTH] = percent;
                }
            } else {
                Vector  vForward;
                trace_t trace;

                AngleVectors(m_vViewAng, vForward, NULL, NULL);

                Vector vEnd = m_vViewPos + vForward * 2048.0f;

                trace = G_Trace(m_vViewPos, vec_zero, vec_zero, vEnd, this, MASK_BEAM, qfalse, "infoclientcheck");

                if (trace.ent && trace.ent->entity->IsSubclassOfPlayer()) {
                    Player *p = static_cast<Player *>(trace.ent->entity);

                    if (IsSpectator() || p->GetTeam() == GetTeam()) {
                        m_iInfoClient       = trace.ent - g_entities;
                        m_iInfoClientHealth = p->health;
                        m_fInfoClientTime   = level.time;

                        float percent = trace.ent->entity->health / trace.ent->entity->max_health * 100.0f;

                        if (percent > 0.0f && percent < 1.0f) {
                            percent = 1.0f;
                        }

                        client->ps.stats[STAT_INFOCLIENT_HEALTH] = percent;
                    }
                }
            }

            if (m_iInfoClient != -1) {
                if (level.time <= m_fInfoClientTime + 1.5f) {
                    client->ps.stats[STAT_INFOCLIENT]        = m_iInfoClient;
                    client->ps.stats[STAT_INFOCLIENT_HEALTH] = m_iInfoClientHealth;
                } else {
                    m_iInfoClient = -1;
                }
            }
        }

        if (g_gametype->integer < GT_TOW) {
            vObjectiveLocation = level.m_vObjectiveLocation;
        } else {
            if (GetTeam() == TEAM_AXIS) {
                vObjectiveLocation = level.m_vAxisObjectiveLocation;
            } else if (GetTeam() == TEAM_ALLIES) {
                vObjectiveLocation = level.m_vAlliedObjectiveLocation;
            }
        }

        if (g_protocol < protocol_e::PROTOCOL_MOHTA_MIN && vObjectiveLocation == vec_zero) {
            //
            // try to use the nearest teammate instead.
            // the reason is that mohaa 1.11 and below doesn't have a radar
            // for teammates
            //
            if (g_gametype->integer > GT_FFA && !IsDead() && !IsSpectator()) {
                gentity_t *ent;
                int        i;
                Player    *p;
                float      fNearest = 9999.0f;
                float      fLength;

                // match the compass direction to the nearest player
                for (i = 0, ent = g_entities; i < game.maxclients; i++, ent++) {
                    if (!ent->inuse || !ent->client || !ent->entity || ent->entity == this) {
                        continue;
                    }

                    p = (Player *)ent->entity;
                    if (p->IsDead() || p->IsSpectator() || p->dm_team != dm_team) {
                        continue;
                    }

                    fLength = (p->centroid - centroid).length();

                    if (fLength < fNearest) {
                        fNearest           = fLength;
                        vObjectiveLocation = p->centroid;
                    }
                }
            }
        }
    }

    if (m_pVehicle && !m_pTurret) {
        client->ps.stats[STAT_VEHICLE_HEALTH]     = m_pVehicle->health;
        client->ps.stats[STAT_VEHICLE_MAX_HEALTH] = m_pVehicle->max_health;
    }

    //
    // Health fraction
    //
    healthfrac = (health / max_health * 100.0f);

    if (m_pVehicle && !m_pTurret) {
        if (m_pVehicle->isSubclassOf(FixedTurret)) {
            healthfrac = (m_pVehicle->health / m_pVehicle->max_health * 100.f);
        }
    }

    if (healthfrac < 1 && healthfrac > 0) {
        healthfrac = 1;
    }
    if (healthfrac < 0) {
        healthfrac = 0;
    }

    client->ps.stats[STAT_HEALTH] = healthfrac;

    //
    // Healing
    //
    if (m_fHealRate && (!m_pVehicle || m_pTurret || m_pVehicle->isSubclassOf(FixedTurret))) {
        healfrac = (health + m_fHealRate) / max_health * 100.f;
    } else {
        healfrac = 0;
    }
    if (healfrac < 1 && healfrac > 0) {
        healfrac = 1;
    }
    if (healfrac < 0) {
        healfrac = 0;
    }

    client->ps.stats[STAT_NEXTHEALTH] = healfrac;
    client->ps.stats[STAT_MAXHEALTH]  = 100;

    Weapon *activeweap = GetActiveWeapon(WEAPON_MAIN);

    client->ps.stats[STAT_WEAPONS]         = 0;
    client->ps.stats[STAT_EQUIPPED_WEAPON] = 0;
    client->ps.stats[STAT_AMMO]            = 0;
    client->ps.stats[STAT_MAXAMMO]         = 0;
    client->ps.stats[STAT_CLIPAMMO]        = 0;
    client->ps.stats[STAT_MAXCLIPAMMO]     = 0;

    client->ps.activeItems[ITEM_AMMO]   = -1;
    client->ps.activeItems[ITEM_WEAPON] = -1;
    client->ps.activeItems[2]           = -1;
    client->ps.activeItems[3]           = -1;
    client->ps.activeItems[4]           = -1;
    client->ps.activeItems[5]           = -1;

    if (m_pTurret) {
        client->ps.activeItems[ITEM_WEAPON] = m_pTurret->getIndex();
        if (getMoveType() == MOVETYPE_PORTABLE_TURRET || getMoveType() == MOVETYPE_TURRET) {
            // Use the turret's ammo
            client->ps.stats[STAT_CLIPAMMO]    = m_pTurret->ammo_in_clip[FIRE_PRIMARY];
            client->ps.stats[STAT_MAXCLIPAMMO] = m_pTurret->ammo_clip_size[FIRE_PRIMARY];
        }
    } else if (activeweap) {
        if (activeweap->m_bSecondaryAmmoInHud) {
            client->ps.stats[STAT_AMMO]           = AmmoCount(activeweap->GetAmmoType(FIRE_SECONDARY));
            client->ps.stats[STAT_MAXAMMO]        = MaxAmmoCount(activeweap->GetAmmoType(FIRE_SECONDARY));
            client->ps.stats[STAT_SECONDARY_AMMO] = AmmoCount(activeweap->GetAmmoType(FIRE_PRIMARY));
        } else {
            client->ps.stats[STAT_AMMO]    = AmmoCount(activeweap->GetAmmoType(FIRE_PRIMARY));
            client->ps.stats[STAT_MAXAMMO] = MaxAmmoCount(activeweap->GetAmmoType(FIRE_PRIMARY));
        }

        client->ps.stats[STAT_CLIPAMMO]    = activeweap->ClipAmmo(FIRE_PRIMARY);
        client->ps.stats[STAT_MAXCLIPAMMO] = activeweap->GetClipSize(FIRE_PRIMARY);

        client->ps.activeItems[ITEM_AMMO] = AmmoIndex(activeweap->GetAmmoType(FIRE_PRIMARY));

        // grenade and rockets must match the number of ammo
        if (client->ps.stats[STAT_MAXCLIPAMMO] == 1) {
            client->ps.stats[STAT_MAXAMMO]++;
            client->ps.stats[STAT_AMMO] += client->ps.stats[STAT_CLIPAMMO];
        }

        if (!activeweap->IsSubclassOfInventoryItem()) {
            client->ps.stats[STAT_EQUIPPED_WEAPON] = activeweap->GetWeaponClass();
        }

        client->ps.activeItems[ITEM_WEAPON] = activeweap->getIndex();
    } else if (m_pVehicle) {
        Entity *pEnt = m_pVehicle->QueryTurretSlotEntity(0);
        if (pEnt && pEnt->IsSubclassOfVehicleTurretGun()) {
            VehicleTurretGun *vt = static_cast<VehicleTurretGun *>(pEnt);

            client->ps.activeItems[ITEM_WEAPON]   = vt->getIndex();
            client->ps.stats[STAT_CLIPAMMO]       = vt->ammo_in_clip[FIRE_PRIMARY];
            client->ps.stats[STAT_MAXCLIPAMMO]    = vt->ammo_clip_size[FIRE_PRIMARY];
            client->ps.stats[STAT_SECONDARY_AMMO] = vt->GetWarmupFraction() * 100.f;
        }
    }

    //
    // set boss health
    //
    client->ps.stats[STAT_BOSSHEALTH] = bosshealth->value * 100.0f;

    if (bosshealth->value * 100.0f > 0 && client->ps.stats[STAT_BOSSHEALTH] == 0) {
        client->ps.stats[STAT_BOSSHEALTH] = 1;
    }

    // Set cinematic stuff

    client->ps.stats[STAT_CINEMATIC] = 0;

    if (level.cinematic) {
        client->ps.stats[STAT_CINEMATIC] = (1 << 0);
    }

    if (actor_camera) {
        client->ps.stats[STAT_CINEMATIC] += (1 << 1);
    }

    count = inventory.NumObjects();

    int iItem = 0;

    for (i = 1; i <= count; i++) {
        int     entnum = inventory.ObjectAt(i);
        Weapon *weapon = (Weapon *)G_GetEntity(entnum);
        int     weapon_class;

        if (weapon->IsSubclassOfWeapon()) {
            if (weapon->IsSubclassOfInventoryItem()) {
                if (iItem > 3) {
                    weapon->SetItemSlot(0);
                } else {
                    client->ps.activeItems[iItem + 2] = weapon->getIndex();
                    weapon->SetItemSlot(256 << iItem);

                    if (activeweap && weapon == activeweap) {
                        client->ps.stats[STAT_EQUIPPED_WEAPON] = 256 << iItem;
                    }

                    iItem++;
                }
            } else {
                weapon_class = weapon->GetWeaponClass();

                if (weapon_class & WEAPON_CLASS_GRENADE) {
                    if (weapon->HasAmmo(FIRE_PRIMARY)) {
                        client->ps.stats[STAT_WEAPONS] |= weapon_class;
                    }
                } else {
                    client->ps.stats[STAT_WEAPONS] |= weapon_class;
                }
            }
        }
    }

    // Go through all the player's ammo and send over the names/amounts
    memset(client->ps.ammo_amount, 0, sizeof(client->ps.ammo_amount));
    memset(client->ps.ammo_name_index, 0, sizeof(client->ps.ammo_name_index));
    memset(client->ps.max_ammo_amount, 0, sizeof(client->ps.max_ammo_amount));

    count = ammo_inventory.NumObjects();

    for (i = 1; i <= count; i++) {
        Ammo *ammo = ammo_inventory.ObjectAt(i);

        if (ammo) {
            client->ps.ammo_amount[i - 1]     = ammo->getAmount();
            client->ps.max_ammo_amount[i - 1] = ammo->getMaxAmount();
            client->ps.ammo_name_index[i - 1] = ammo->getIndex();
        }
    }

    if (m_iInZoomMode == -1) {
        client->ps.stats[STAT_INZOOM] = fov;
    } else {
        client->ps.stats[STAT_INZOOM] = 0;
    }

    client->ps.stats[STAT_CROSSHAIR] =
        ((!client->ps.stats[STAT_INZOOM] || client->ps.stats[STAT_INZOOM] > 30)
         && (activeweap && !activeweap->IsSubclassOfInventoryItem() && activeweap->GetUseCrosshair()))
        || m_pTurret || (m_pVehicle && m_pVehicle->IsSubclassOfVehicleTank());

    client->ps.stats[STAT_COMPASSNORTH] = ANGLE2SHORT(world->m_fNorth);

    if (VectorCompare(vObjectiveLocation, vec_zero)) {
        client->ps.stats[STAT_OBJECTIVELEFT]   = 1730;
        client->ps.stats[STAT_OBJECTIVERIGHT]  = 1870;
        client->ps.stats[STAT_OBJECTIVECENTER] = 1800;
    } else {
        Vector vDelta;
        float  yaw;
        float  fOffset;

        vDelta = vObjectiveLocation - centroid;
        yaw    = AngleSubtract(v_angle[1], vDelta.toYaw()) + 180.0f;

        vDelta  = yaw_left * 300.0f + yaw_forward * vDelta.length() + centroid - centroid;
        fOffset = AngleSubtract(vDelta.toYaw(), v_angle[1]);
        if (fOffset < 0.0f) {
            fOffset = -fOffset;
        }

        fOffset = 53.0f - fOffset + 7.0f;
        if (fOffset < 7.0f) {
            fOffset = 7.0f;
        }

        client->ps.stats[STAT_OBJECTIVELEFT] = anglemod(yaw - fOffset) * 10.0f;
        if (client->ps.stats[STAT_OBJECTIVELEFT] <= 0) {
            client->ps.stats[STAT_OBJECTIVELEFT] = 1;
        } else if (client->ps.stats[STAT_OBJECTIVELEFT] > 3599) {
            client->ps.stats[STAT_OBJECTIVELEFT] = 3599;
        }

        client->ps.stats[STAT_OBJECTIVERIGHT] = anglemod(yaw + fOffset) * 10.0f;
        if (client->ps.stats[STAT_OBJECTIVERIGHT] <= 0) {
            client->ps.stats[STAT_OBJECTIVERIGHT] = 1;
        } else if (client->ps.stats[STAT_OBJECTIVERIGHT] > 3599) {
            client->ps.stats[STAT_OBJECTIVERIGHT] = 3599;
        }

        client->ps.stats[STAT_OBJECTIVECENTER] = anglemod(yaw) * 10.0f;
        if (client->ps.stats[STAT_OBJECTIVECENTER] <= 0) {
            client->ps.stats[STAT_OBJECTIVECENTER] = 1;
        } else if (client->ps.stats[STAT_OBJECTIVECENTER] > 3599) {
            client->ps.stats[STAT_OBJECTIVECENTER] = 3599;
        }
    }

    client->ps.stats[STAT_DAMAGEDIR] = damage_yaw;
    if (client->ps.stats[STAT_DAMAGEDIR] < 0) {
        client->ps.stats[STAT_DAMAGEDIR] = 0;
    } else if (client->ps.stats[STAT_DAMAGEDIR] > 3600) {
        client->ps.stats[STAT_DAMAGEDIR] = 3600;
    }

    // Do letterbox

    // Check for letterbox fully out
    if ((level.m_letterbox_time <= 0) && (level.m_letterbox_dir == letterbox_in)) {
        client->ps.stats[STAT_LETTERBOX] = level.m_letterbox_fraction * MAX_LETTERBOX_SIZE;
        return;
    } else if ((level.m_letterbox_time <= 0) && (level.m_letterbox_dir == letterbox_out)) {
        client->ps.stats[STAT_LETTERBOX] = 0;
        return;
    }

    float frac;

    level.m_letterbox_time -= level.intframetime;

    frac = level.m_letterbox_time / level.m_letterbox_time_start;

    if (frac > 1) {
        frac = 1;
    }
    if (frac < 0) {
        frac = 0;
    }

    if (level.m_letterbox_dir == letterbox_in) {
        frac = 1.0f - frac;
    }

    client->ps.stats[STAT_LETTERBOX] = (frac * level.m_letterbox_fraction) * MAX_LETTERBOX_SIZE;
}

void Player::UpdateMusic(void)
{
    if (music_forced) {
        client->ps.current_music_mood  = music_current_mood;
        client->ps.fallback_music_mood = music_fallback_mood;
    }

    // Copy music volume and fade time to player state
    client->ps.music_volume           = music_current_volume;
    client->ps.music_volume_fade_time = music_volume_fade_time;
}

void Player::SetReverb(int type, float level)

{
    reverb_type  = type;
    reverb_level = level;
}

void Player::SetReverb(str type, float level)

{
    reverb_type  = EAXMode_NameToNum(type);
    reverb_level = level;
}

void Player::SetReverb(Event *ev)
{
    if (ev->NumArgs() < 2) {
        return;
    }

    SetReverb(ev->GetInteger(1), ev->GetFloat(2));
}

void Player::UpdateReverb(void)
{
    client->ps.reverb_type  = reverb_type;
    client->ps.reverb_level = reverb_level;
}

void Player::UpdateMisc(void)
{
    //
    // clear out the level exit flag
    //
    client->ps.pm_flags &= ~PMF_LEVELEXIT;

    //
    // see if our camera is the level exit camera
    //
    if (camera && camera->IsLevelExit()) {
        client->ps.pm_flags |= PMF_LEVELEXIT;
    } else if (level.near_exit) {
        client->ps.pm_flags |= PMF_LEVELEXIT;
    }

    //
    // do anything special for respawns
    //
    if (client->ps.pm_flags & PMF_RESPAWNED) {
        //
        // change music
        //
        if (music_current_mood != mood_success) {
            ChangeMusic("success", "normal", false);
        }
    }
}

/*
=================
EndFrame

Called for each player at the end of the server frame
and right after spawning
=================
*/
void Player::EndFrame(void)
{
    FinishMove();
    UpdateStats();
    UpdateMusic();
    UpdateReverb();
    UpdateMisc();

    if (!g_spectatefollow_firstperson->integer || !IsSpectator() || !m_iPlayerSpectating) {
        SetupView();
    } else {
        gentity_t *ent = g_entities + m_iPlayerSpectating - 1;

        if (!ent->inuse || !ent->entity || ent->entity->deadflag >= DEAD_DEAD) {
            SetupView();
        }
    }
}

void Player::GotKill(Event *ev)

{
    /*
    Entity *victim;
   Entity *inflictor;
   float   damage;
   int     meansofdeath;
   qboolean gibbed;

   if ( deathmatch->integer )
        {
      return;
        }

    victim = ev->GetEntity( 1 );
    damage = ev->GetInteger( 2 );
    inflictor = ev->GetEntity( 3 );
    meansofdeath = ev->GetInteger( 4 );
    gibbed = ev->GetInteger( 5 );
*/
}

void Player::SetPowerupTimer(Event *ev)

{
    Event *event;

    poweruptimer = ev->GetInteger(1);
    poweruptype  = ev->GetInteger(2);
    event        = new Event(EV_Player_UpdatePowerupTimer);
    PostEvent(event, 1);
}

void Player::UpdatePowerupTimer(Event *ev)

{
    poweruptimer -= 1;
    if (poweruptimer > 0) {
        PostEvent(ev, 1);
    } else {
        poweruptype = 0;
    }
}

void Player::ChangeMusic(const char *current, const char *fallback, qboolean force)
{
    int current_mood_num;
    int fallback_mood_num;

    music_forced = force;

    if (current) {
        current_mood_num = MusicMood_NameToNum(current);
        if (current_mood_num < 0) {
            gi.DPrintf("current music mood %s not found", current);
        } else {
            music_current_mood = current_mood_num;
        }
    }

    if (fallback) {
        fallback_mood_num = MusicMood_NameToNum(fallback);
        if (fallback_mood_num < 0) {
            gi.DPrintf("fallback music mood %s not found", fallback);
            fallback = NULL;
        } else {
            music_fallback_mood = fallback_mood_num;
        }
    }
}

void Player::ChangeMusicVolume(float volume, float fade_time)

{
    music_volume_fade_time = fade_time;
    music_saved_volume     = music_current_volume;
    music_current_volume   = volume;
}

void Player::RestoreMusicVolume(float fade_time)

{
    music_volume_fade_time = fade_time;
    music_current_volume   = music_saved_volume;
    music_saved_volume     = -1.0;
}

void Player::addOrigin(Vector org)
{
    setLocalOrigin(localorigin + org);

    animspeed         = org.x * (1.f / level.frametime);
    airspeed          = org.y * (1.f / level.frametime);
    m_vPushVelocity.x = org.z * (1.f / level.frametime);
}

void Player::Jump(Event *ev)

{
    float maxheight;

    maxheight = ev->GetFloat(1);

    if (maxheight > 16) {
        // v^2 = 2ad
        velocity[2] += sqrt(2 * sv_gravity->integer * maxheight);

        // make sure the player leaves the ground
        client->ps.walking = qfalse;
    }
}

void Player::JumpXY(Event *ev)
{
    float forwardmove;
    float sidemove;
    float distance;
    float time;
    float speed;

    forwardmove = ev->GetFloat(1);
    sidemove    = ev->GetFloat(2);
    speed       = ev->GetFloat(3);

    velocity = yaw_forward * forwardmove - yaw_left * sidemove;
    distance = velocity.length();
    velocity *= speed / distance;
    time        = distance / speed;
    velocity[2] = sv_gravity->integer * time * 0.5f;

    airspeed = distance;

    // make sure the player leaves the ground
    client->ps.walking = qfalse;
}

void Player::SetViewAngles(Vector newViewangles)
{
    // set the delta angle
    client->ps.delta_angles[0] = ANGLE2SHORT(newViewangles.x - client->cmd_angles[0]);
    client->ps.delta_angles[1] = ANGLE2SHORT(newViewangles.y - client->cmd_angles[1]);
    client->ps.delta_angles[2] = ANGLE2SHORT(newViewangles.z - client->cmd_angles[2]);

    v_angle = newViewangles;

    // get the pitch and roll from our leg angles
    newViewangles.x = angles.x;
    newViewangles.z = angles.z;
    AnglesToMat(newViewangles, orientation);
    yaw_forward = orientation[0];
    yaw_left    = orientation[1];
}

void Player::SetTargetViewAngles(Vector angles)
{
    v_angle = angles;
}

void Player::DumpState(Event *ev)

{
    gi.DPrintf(
        "Legs: %s Torso: %s\n", currentState_Legs ? currentState_Legs->getName() : "NULL", currentState_Torso->getName()
    );
}

void Player::ForceTorsoState(Event *ev)
{
    State *ts = statemap_Torso->FindState(ev->GetString(1));
    EvaluateState(ts);
}

void Player::ForceLegsState(Event *ev)
{
    State *ls = statemap_Legs->FindState(ev->GetString(1));
    EvaluateState(NULL, ls);
}

void Player::TouchedUseAnim(Entity *ent)
{
    toucheduseanim = ent;
}

void Player::NextPainTime(Event *ev)
{
    float time = ev->GetFloat(1);

    nextpaintime = level.time + time;

    if (time >= 0.0f) {
        pain          = 0.0f;
        pain_type     = MOD_NONE;
        pain_location = HITLOC_MISS;

        m_pLegsPainCond->clearCheck();
        m_pTorsoPainCond->clearCheck();
    }
}

void Player::EnterVehicle(Event *ev)
{
    Entity *ent;

    ent = ev->GetEntity(1);
    if (ent && ent->IsSubclassOfVehicle()) {
        flags |= FL_PARTIAL_IMMOBILE;
        viewheight = STAND_EYE_HEIGHT;
        velocity   = vec_zero;
        m_pVehicle = (Vehicle *)ent;
        if (m_pVehicle->IsDrivable()) {
            setMoveType(MOVETYPE_VEHICLE);
        } else {
            setMoveType(MOVETYPE_NOCLIP);
        }

        SafeHolster(true);
    }
}

void Player::ExitVehicle(Event *ev)
{
    flags &= ~FL_PARTIAL_IMMOBILE;
    setMoveType(MOVETYPE_WALK);
    m_pVehicle = NULL;

    if (camera) {
        SetCamera(NULL, 0.5f);
        ZoomOff();
    }

    SafeHolster(false);
    takedamage = DAMAGE_YES;
    setSolidType(SOLID_BBOX);
}

void Player::EnterTurret(TurretGun *ent)
{
    flags |= FL_PARTIAL_IMMOBILE;
    viewheight = DEFAULT_VIEWHEIGHT;
    velocity   = vec_zero;
    m_pTurret  = ent;

    if (ent->inheritsFrom(PortableTurret::classinfostatic())) {
        // carryable turret
        setMoveType(MOVETYPE_PORTABLE_TURRET);
        StopPartAnimating(torso);
        SetPartAnim("mg42tripod_aim_straight_straight");
    } else {
        // standard turret
        setMoveType(MOVETYPE_TURRET);
    }

    SafeHolster(true);
}

void Player::EnterTurret(Event *ev)
{
    TurretGun *ent = (TurretGun *)ev->GetEntity(1);

    if (!ent) {
        return;
    }

    if (!ent->inheritsFrom(TurretGun::classinfostatic())) {
        return;
    }

    EnterTurret(ent);
}

void Player::ExitTurret(void)
{
    if (m_pTurret->inheritsFrom(PortableTurret::classinfostatic())) {
        StopPartAnimating(torso);
        SetPartAnim("mg42tripod_aim_straight_straight");
    }

    flags &= ~FL_PARTIAL_IMMOBILE;
    setMoveType(MOVETYPE_WALK);
    m_pTurret = NULL;

    SafeHolster(qfalse);

    new_buttons        = 0;
    server_new_buttons = 0;
}

void Player::ExitTurret(Event *ev)
{
    ExitTurret();
}

void Player::HolsterToggle(Event *ev)
{
    if (deadflag) {
        return;
    }

    if (WeaponsOut()) {
        // fucking compiler bug
        // it won't call the parent's override function
        ((Sentient *)this)->Holster(qtrue);
    } else {
        ((Sentient *)this)->Holster(qfalse);
    }
}

void Player::Holster(Event *ev)
{
    SafeHolster(ev->GetBoolean(1));
}

void Player::WatchActor(Event *ev)

{
    if (camera || currentState_Torso->getCameraType() != CAMERA_BEHIND) {
        return;
    }

    actor_to_watch = (Actor *)ev->GetEntity(1);
}

void Player::StopWatchingActor(Event *ev)

{
    Actor *old_actor;

    old_actor = (Actor *)ev->GetEntity(1);

    if (old_actor == actor_to_watch) {
        actor_to_watch = NULL;
    }
}

void Player::setAngles(Vector ang)
{
    // set the angles normally

    if (bindmaster) {
        ang -= bindmaster->angles;
    }

    Entity::setAngles(ang);
}

painDirection_t Player::Pain_string_to_int(str pain)

{
    if (!pain.icmp(pain, "Front")) {
        return PAIN_FRONT;
    } else if (!pain.icmp(pain, "Left")) {
        return PAIN_LEFT;
    } else if (!pain.icmp(pain, "Right")) {
        return PAIN_RIGHT;
    } else if (!pain.icmp(pain, "Rear")) {
        return PAIN_REAR;
    } else {
        return PAIN_NONE;
    }
}

void Player::ArchivePersistantData(Archiver& arc)
{
    str model_name;

    Sentient::ArchivePersistantData(arc);

    model_name = g_playermodel->string;

    arc.ArchiveString(&model_name);

    if (arc.Loading()) {
        // set the cvar
        gi.cvar_set("g_playermodel", model_name.c_str());

        model_name += ".tik";
        setModel(model_name.c_str());
    }

    str name;
    if (arc.Saving()) {
        if (holsteredWeapon) {
            name = holsteredWeapon->getName();
        } else {
            name = "none";
        }
    }
    arc.ArchiveString(&name);
    if (arc.Loading()) {
        if (name != "none") {
            holsteredWeapon = (Weapon *)FindItem(name);
        }
    }

    UpdateWeapons();

    // Force a re-evaluation of the player's state
    LoadStateTable();
}

void Player::VelocityModified(void) {}

int Player::GetKnockback(int original_knockback, qboolean blocked)
{
    int new_knockback;

    new_knockback = original_knockback - 50;

    // See if we still have enough knockback to knock the player down
    if (new_knockback >= 200 && take_pain) {
        knockdown = true;

        if (blocked) {
            float damage;

            damage = new_knockback / 50;

            if (damage > 10) {
                damage = 10;
            }

            Damage(world, world, damage, origin, vec_zero, vec_zero, 0, DAMAGE_NO_ARMOR, MOD_CRUSH);
        }
    }

    // Make sure knockback is still at least 0

    if (new_knockback < 0) {
        new_knockback = 0;
    }

    return new_knockback;
}

void Player::ResetHaveItem(Event *ev)
{
    str             fullname;
    ScriptVariable *var;

    fullname = str("playeritem_") + ev->GetString(1);

    var = game.vars->GetVariable(fullname.c_str());

    if (var) {
        var->setIntValue(0);
    }
}

void Player::ReceivedItem(Item *item) {}

void Player::RemovedItem(Item *item) {}

void Player::AmmoAmountChanged(Ammo *ammo, int ammo_in_clip)
{
    str             fullname;
    ScriptVariable *var;

    //
    // set our level variables
    //
    fullname = str("playerammo_") + ammo->getName();

    var = level.vars->GetVariable(fullname.c_str());
    if (!var) {
        level.vars->SetVariable(fullname.c_str(), ammo->getAmount() + ammo_in_clip);
    } else {
        var->setIntValue(ammo->getAmount() + ammo_in_clip);
    }
}

void Player::WaitForState(Event *ev)
{
    waitForState = ev->GetString(1);
}

void Player::SetDamageMultiplier(Event *ev)
{
    damage_multiplier = ev->GetFloat(1);
}

void Player::SetTakePain(Event *ev)
{
    take_pain = ev->GetBoolean(1);
}

void Player::Loaded(void)
{
    UpdateWeapons();
}

void Player::PlayerShowModel(Event *ev)
{
    Entity::showModel();
    UpdateWeapons();
}

void Player::showModel(void)
{
    Entity::showModel();
    UpdateWeapons();
}

Vector Player::EyePosition(void)
{
    return m_vViewPos;
}

void Player::ModifyHeight(Event *ev)
{
    str height = ev->GetString(1);

    if (!height.icmp("stand")) {
        viewheight   = DEFAULT_VIEWHEIGHT;
        maxs.z       = 94.0f;
        m_bHasJumped = false;
    } else if (!height.icmp("jumpstart")) {
        //viewheight = JUMP_START_VIEWHEIGHT;
        maxs.z = 94.0f;
    } else if (!height.icmp("duck")) {
        viewheight = CROUCH_VIEWHEIGHT;
        maxs.z     = 54.0f;
    } else if (!height.icmp("duckrun")) {
        viewheight = CROUCH_RUN_VIEWHEIGHT;
        maxs.z     = 60.0f;
    } else if (!height.icmp("prone")) {
        //
        // Prone was added in openmohaa
        //
        viewheight = PRONE_VIEWHEIGHT;
        maxs.z     = 20.0f;
    } else {
        gi.Printf("Unknown modheight '%s' defaulting to stand\n", height.c_str());
        viewheight = DEFAULT_VIEWHEIGHT;
        maxs.z     = 94.0f;
    }
}

// Specify the view height of the player and the height of his bounding box
void Player::ModifyHeightFloat(Event *ev)
{
    // params
    int   height;
    float max_z;

    height = ev->GetInteger(1);
    max_z  = ev->GetFloat(2);

    viewheight = height;

    if (max_z >= 94.0) {
        max_z = 94.0;
    } else if (max_z >= 74.0 && max_z < 94.0) {
        max_z = 54.0;
    } else if (max_z >= 30.0 && max_z < 54.0) {
        max_z = 20.0;
    } else if (max_z <= 20.0) {
        max_z = 20.0;
    }

    maxs.z = max_z;

    client->ps.pm_flags &= ~(PMF_DUCKED | PMF_VIEW_PRONE | PMF_VIEW_DUCK_RUN | PMF_VIEW_JUMP_START);

    // FIXME...
    /*
    gi.MSG_SetClient(edict - g_entities);

    gi.MSG_StartCGM(CGM_MODHEIGHTFLOAT);
    gi.MSG_WriteLong(height);
    gi.MSG_WriteFloat(max_z);
    gi.MSG_EndCGM();
    */
}

void Player::SetMovePosFlags(Event *ev)
{
    str sParm;

    if (ev->NumArgs() <= 0) {
        Com_Printf("moveposflags command without any parameters\n");
        return;
    }

    sParm = ev->GetString(1);

    if (!sParm.icmp("crouching")) {
        m_iMovePosFlags = MPF_POSITION_CROUCHING;
    } else if (!sParm.icmp("prone")) {
        m_iMovePosFlags = MPF_POSITION_PRONE;
    } else if (!sParm.icmp("offground")) {
        m_iMovePosFlags = MPF_POSITION_OFFGROUND;
    } else {
        m_iMovePosFlags = MPF_POSITION_STANDING;
    }

    if (ev->NumArgs() > 1) {
        sParm = ev->GetString(2);

        if (!sParm.icmp("walking") || !sParm.icmp("walking\"") // there is a mistake in WALK_FORWARD
        ) {
            m_iMovePosFlags |= MPF_MOVEMENT_WALKING;
        } else if (!sParm.icmp("running")) {
            m_iMovePosFlags |= MPF_MOVEMENT_RUNNING;
        } else if (!sParm.icmp("falling")) {
            m_iMovePosFlags |= MPF_MOVEMENT_FALLING;
        }
    }
}

void Player::GetPositionForScript(Event *ev)
{
    if (m_iMovePosFlags & MPF_POSITION_CROUCHING) {
        ev->AddConstString(STRING_CROUCHING);
    } else if (m_iMovePosFlags & MPF_POSITION_PRONE) {
        ev->AddConstString(STRING_PRONE);
    } else if (m_iMovePosFlags & MPF_POSITION_OFFGROUND) {
        ev->AddConstString(STRING_OFFGROUND);
    } else {
        ev->AddConstString(STRING_STANDING);
    }
}

void Player::GetMovementForScript(Event *ev)
{
    if (m_iMovePosFlags & MPF_MOVEMENT_WALKING) {
        ev->AddConstString(STRING_WALKING);
    } else if (m_iMovePosFlags & MPF_MOVEMENT_RUNNING) {
        ev->AddConstString(STRING_RUNNING);
    } else if (m_iMovePosFlags & MPF_MOVEMENT_FALLING) {
        ev->AddConstString(STRING_FALLING);
    } else {
        ev->AddConstString(STRING_STANDING);
    }
}

void Player::ToggleZoom(int iZoom)
{
    if (iZoom && m_iInZoomMode == -1) {
        SetFov(selectedfov);
        m_iInZoomMode = 0;
    } else {
        SetFov(iZoom);
        m_iInZoomMode = -1;
    }
}

void Player::ZoomOff(void)
{
    SetFov(selectedfov);
    m_iInZoomMode = 0;
}

void Player::ZoomOffEvent(Event *ev)
{
    ZoomOff();
}

qboolean Player::IsZoomed(void)
{
    return m_iInZoomMode == -1;
}

void Player::SafeZoomed(Event *ev)
{
    if (ev->GetInteger(1)) {
        if (m_iInZoomMode > 0) {
            SetFov(m_iInZoomMode);
            m_iInZoomMode = -1;
        }
    } else {
        if (m_iInZoomMode == -1) {
            m_iInZoomMode = fov;
            SetFov(selectedfov);
        }
    }
}

void Player::AttachToLadder(Event *ev)
{
    Vector      vStart, vEnd, vOffset;
    trace_t     trace;
    FuncLadder *pLadder;

    if (deadflag) {
        return;
    }

    AngleVectors(m_vViewAng, vOffset, NULL, NULL);

    vStart = m_vViewPos - vOffset * 12.0f;
    vEnd   = m_vViewPos + vOffset * 128.0f;

    trace = G_Trace(vStart, vec_zero, vec_zero, vEnd, this, MASK_LADDER, qfalse, "Player::AttachToLadder");

    if (trace.fraction == 1.0f || !trace.ent || !trace.ent->entity || !trace.ent->entity->isSubclassOf(FuncLadder)) {
        return;
    }

    pLadder   = (FuncLadder *)trace.ent->entity;
    m_pLadder = pLadder;

    pLadder->PositionOnLadder(this);

    SetViewAngles(Vector(v_angle[0], angles[1], v_angle[2]));
}

void Player::UnattachFromLadder(Event *ev)
{
    m_pLadder = NULL;
}

void Player::TweakLadderPos(Event *ev)
{
    FuncLadder *pLadder = (FuncLadder *)m_pLadder.Pointer();

    if (pLadder) {
        pLadder->AdjustPositionOnLadder(this);
    }
}

void Player::EnsureOverLadder(Event *ev)
{
    FuncLadder *pLadder = (FuncLadder *)m_pLadder.Pointer();

    if (pLadder) {
        pLadder->EnsureOverLadder(this);
    }
}

void Player::EnsureForwardOffLadder(Event *ev)
{
    FuncLadder *pLadder = (FuncLadder *)m_pLadder.Pointer();

    if (pLadder) {
        pLadder->EnsureForwardOffLadder(this);
    }
}

void Player::EventForceLandmineMeasure(Event *ev)
{
    MeasureLandmineDistances();
}

str Player::GetCurrentDMWeaponType() const
{
    return m_sDmPrimary;
}

void Player::Score(Event *ev)
{
    if (g_gametype->integer == GT_SINGLE_PLAYER) {
        // Of course useless in single-player mode
        return;
    }

    dmManager.Score(this);
}

// Was between 2.0 and 2.15
/*
nationality_t GetAlliedType(const char* name)
{
    if (!Q_stricmpn(name, "american", 8)) {
        return NA_AMERICAN;
    } else if (!Q_stricmpn(name, "allied_russian", 14)) {
        return NA_RUSSIAN;
    } else if (!Q_stricmpn(name, "allied_british", 14)) {
        return NA_BRITISH;
    } else if (!Q_stricmpn(name, "allied", 6)) {
        return NA_AMERICAN;
    } else {
        return NA_NONE;
    }
}
*/

nationality_t GetPlayerTeamType(const char *name)
{
    if (!Q_stricmpn(name, "american", 8)) {
        return NA_AMERICAN;
    } else if (!Q_stricmpn(name, "allied_russian", 14)) {
        return NA_RUSSIAN;
    } else if (!Q_stricmpn(name, "allied_british", 14)) {
        return NA_BRITISH;
    } else if (!Q_stricmpn(name, "allied_sas", 10)) {
        return NA_BRITISH;
    } else if (!Q_stricmpn(name, "allied", 6)) {
        return NA_AMERICAN;
    } else if (!Q_stricmpn(name, "german", 6)) {
        return NA_GERMAN;
    } else if (!Q_stricmpn(name, "it", 2)) {
        return NA_ITALIAN;
    } else if (!Q_stricmpn(name, "sc", 2)) {
        return NA_ITALIAN;
    } else {
        return NA_NONE;
    }
}

void Player::InitDeathmatch(void)
{
    fAttackerDispTime    = 0.0f;
    pAttackerDistPointer = nullptr;
    m_iInfoClient        = -1;
    m_fWeapSelectTime    = level.time - 9.0f;

    if (!g_realismmode->integer) {
        m_fDamageMultipliers[HITLOC_HEAD]        = 2.0f;
        m_fDamageMultipliers[HITLOC_HELMET]      = 2.0f;
        m_fDamageMultipliers[HITLOC_NECK]        = 2.0f;
        m_fDamageMultipliers[HITLOC_TORSO_UPPER] = 1.0f;
        m_fDamageMultipliers[HITLOC_TORSO_MID]   = 0.95f;
        m_fDamageMultipliers[HITLOC_TORSO_LOWER] = 0.90f;
        m_fDamageMultipliers[HITLOC_PELVIS]      = 0.85f;
        m_fDamageMultipliers[HITLOC_R_ARM_UPPER] = 0.80f;
        m_fDamageMultipliers[HITLOC_L_ARM_UPPER] = 0.80f;
        m_fDamageMultipliers[HITLOC_R_LEG_UPPER] = 0.80f;
        m_fDamageMultipliers[HITLOC_L_LEG_UPPER] = 0.80f;
        m_fDamageMultipliers[HITLOC_R_ARM_LOWER] = 0.60f;
        m_fDamageMultipliers[HITLOC_L_ARM_LOWER] = 0.60f;
        m_fDamageMultipliers[HITLOC_R_LEG_LOWER] = 0.60f;
        m_fDamageMultipliers[HITLOC_L_LEG_LOWER] = 0.60f;
        m_fDamageMultipliers[HITLOC_R_HAND]      = 0.50f;
        m_fDamageMultipliers[HITLOC_L_HAND]      = 0.50f;
        m_fDamageMultipliers[HITLOC_R_FOOT]      = 0.50f;
        m_fDamageMultipliers[HITLOC_L_FOOT]      = 0.50f;
    }

    if (current_team) {
        if (AllowTeamRespawn()) {
            EndSpectator();

            if (dmManager.GetMatchStartTime() > 0.0f && !dmManager.AllowRespawn() && g_allowjointime->value > 0.0f
                && (level.time - dmManager.GetMatchStartTime()) > g_allowjointime->value) {
                m_bTempSpectator = true;
            }

            switch (g_gametype->integer) {
            case GT_TEAM_ROUNDS:
            case GT_OBJECTIVE:
            case GT_TOW:
            case GT_LIBERATION:
                if (!m_bTempSpectator) {
                    BeginFight();
                } else {
                    Spectator();
                }
                break;
            default:
                BeginFight();
                break;
            }
        }
    } else {
        if (client->pers.teamnum) {
            SetTeam(client->pers.teamnum);
        } else {
            SetTeam(TEAM_SPECTATOR);
        }
    }

    edict->s.eFlags &= ~(TEAM_ALLIES | TEAM_AXIS);

    if (GetTeam() == TEAM_ALLIES) {
        edict->s.eFlags |= TEAM_ALLIES;
    } else if (GetTeam() == TEAM_AXIS) {
        edict->s.eFlags |= TEAM_AXIS;
    }

    G_SetClientConfigString(edict);

    if (g_gametype->integer >= GT_TEAM_ROUNDS) {
        if (client->pers.round_kills) {
            num_deaths               = client->pers.round_kills;
            client->pers.round_kills = 0;
        }
    }

    ChooseSpawnPoint();
    EquipWeapons();

    if (current_team) {
        current_team->m_bHasSpawnedPlayers = qtrue;
    }
}

bool Player::QueryLandminesAllowed() const
{
    const char *mapname;

    if (dmflags->integer & DF_WEAPON_NO_LANDMINE) {
        return qfalse;
    }

    if (dmflags->integer & DF_WEAPON_LANDMINE_ALWAYS) {
        return qtrue;
    }

    mapname = level.mapname.c_str();

    if (!Q_stricmpn(mapname, "obj/obj_", 8u)) {
        return qfalse;
    }
    if (!Q_stricmpn(mapname, "dm/mohdm", 8u)) {
        return qfalse;
    }
    if (!Q_stricmp(mapname, "DM/MP_Bahnhof_DM")) {
        return qfalse;
    }
    if (!Q_stricmp(mapname, "obj/MP_Ardennes_TOW")) {
        return qfalse;
    }
    if (!Q_stricmp(mapname, "DM/MP_Bazaar_DM")) {
        return qfalse;
    }
    if (!Q_stricmp(mapname, "obj/MP_Berlin_TOW")) {
        return qfalse;
    }
    if (!Q_stricmp(mapname, "DM/MP_Brest_DM")) {
        return qfalse;
    }
    if (!Q_stricmp(mapname, "obj/MP_Druckkammern_TOW")) {
        return qfalse;
    }
    if (!Q_stricmp(mapname, "DM/MP_Gewitter_DM")) {
        return qfalse;
    }
    if (!Q_stricmp(mapname, "obj/MP_Flughafen_TOW")) {
        return qfalse;
    }
    if (!Q_stricmp(mapname, "DM/MP_Holland_DM")) {
        return qfalse;
    }
    if (!Q_stricmp(mapname, "DM/MP_Malta_DM")) {
        return qfalse;
    }
    if (!Q_stricmp(mapname, "DM/MP_Stadt_DM")) {
        return qfalse;
    }
    if (!Q_stricmp(mapname, "DM/MP_Unterseite_DM")) {
        return qfalse;
    }
    if (!Q_stricmp(mapname, "DM/MP_Verschneit_DM")) {
        return qfalse;
    }
    if (!Q_stricmp(mapname, "lib/mp_ship_lib")) {
        return qfalse;
    }

    return qtrue;
}

void Player::EnsurePlayerHasAllowedWeapons()
{
    int i;

    //if (client != (gclient_t*)-2190)
    if (!client) {
        return;
    }

    if (!client->pers.dm_primary[0]) {
        return;
    }

    for (i = 0; i < 7; i++) {
        if (!Q_stricmp(client->pers.dm_primary, "sniper")) {
            if (!(dmflags->integer & DF_WEAPON_NO_SNIPER)) {
                return;
            }

            strcpy(client->pers.dm_primary, "rifle");
        } else if (!Q_stricmp(client->pers.dm_primary, "rifle")) {
            if (!(dmflags->integer & DF_WEAPON_NO_RIFLE)) {
                return;
            }

            strcpy(client->pers.dm_primary, "smg");
        } else if (!Q_stricmp(client->pers.dm_primary, "smg")) {
            if (!(dmflags->integer & DF_WEAPON_NO_RIFLE)) {
                return;
            }

            strcpy(client->pers.dm_primary, "mg");
        } else if (!Q_stricmp(client->pers.dm_primary, "mg")) {
            if (!(dmflags->integer & DF_WEAPON_NO_RIFLE)) {
                return;
            }

            strcpy(client->pers.dm_primary, "shotgun");
        } else if (!Q_stricmp(client->pers.dm_primary, "shotgun")) {
            if (!(dmflags->integer & DF_WEAPON_NO_RIFLE)) {
                return;
            }

            strcpy(client->pers.dm_primary, "heavy");
        } else if (!Q_stricmp(client->pers.dm_primary, "heavy")) {
            if (!(dmflags->integer & DF_WEAPON_NO_RIFLE)) {
                return;
            }

            strcpy(client->pers.dm_primary, "landmine");
        } else if (!Q_stricmp(client->pers.dm_primary, "landmine")) {
            if (QueryLandminesAllowed()) {
                return;
            }

            strcpy(client->pers.dm_primary, "sniper");
        }
    }

    gi.cvar_set("dmflags", va("%i", dmflags->integer & ~DF_WEAPON_NO_RIFLE));
    Com_Printf("No valid weapons -- re-allowing the rifle\n");
    strcpy(client->pers.dm_primary, "rifle");
}

void Player::EquipWeapons()
{
    Event *event;

    nationality_t nationality;

    if (IsSpectator()) {
        FreeInventory();
        return;
    }

    if (GetTeam() == TEAM_AXIS) {
        nationality = GetPlayerTeamType(client->pers.dm_playergermanmodel);
    } else {
        nationality = GetPlayerTeamType(client->pers.dm_playermodel);
    }

    event = new Event(EV_Sentient_UseItem);

    if (!m_sDmPrimary.length()) {
        // Set the primary weapon
        m_sDmPrimary = client->pers.dm_primary;
    }

    EnsurePlayerHasAllowedWeapons();

    if (!Q_stricmp(client->pers.dm_primary, "sniper") && !(dmflags->integer & DF_WEAPON_NO_SNIPER)) {
        switch (nationality) {
        case NA_BRITISH:
            giveItem("weapons/Uk_W_L42A1.tik");
            event->AddString("Enfield L42A1");
            break;
        case NA_RUSSIAN:
            if (dmflags->integer & DF_OLD_SNIPER) {
                giveItem("weapons/springfield.tik");
                event->AddString("Springfield '03 Sniper");
            } else {
                giveItem("weapons/svt_rifle.tik");
                event->AddString("SVT 40");
            }
            break;
        case NA_GERMAN:
            if (dmflags->integer & DF_OLD_SNIPER) {
                giveItem("weapons/kar98sniper.tik");
                event->AddString("KAR98 - Sniper");
            } else {
                giveItem("weapons/g43.tik");
                event->AddString("G 43");
            }
            break;
        case NA_ITALIAN:
            giveItem("weapons/kar98sniper.tik");
            event->AddString("KAR98 - Sniper");
            break;
        default:
            giveItem("weapons/kar98sniper.tik");
            event->AddString("KAR98 - Sniper");
            break;
        }
    } else if (!Q_stricmp(client->pers.dm_primary, "smg") && !(dmflags->integer & DF_WEAPON_NO_SMG)) {
        switch (nationality) {
        case NA_BRITISH:
            giveItem("weapons/sten.tik");
            event->AddString("Sten Mark II");
            break;
        case NA_RUSSIAN:
            giveItem("weapons/ppsh_smg.tik");
            event->AddString("PPSH SMG");
            break;
        case NA_GERMAN:
            giveItem("weapons/mp40.tik");
            event->AddString("MP40");
            break;
        case NA_ITALIAN:
            giveItem("weapons/it_w_moschetto.tik");
            event->AddString("Moschetto");
            break;
        default:
            giveItem("weapons/thompsonsmg.tik");
            event->AddString("Thompson");
            break;
        }
    } else if (!Q_stricmp(client->pers.dm_primary, "mg") && !(dmflags->integer & DF_WEAPON_NO_MG)) {
        switch (nationality) {
        case NA_BRITISH:
            giveItem("weapons/Uk_W_Vickers.tik");
            event->AddString("Vickers-Berthier");
            break;
        case NA_GERMAN:
            giveItem("weapons/mp44.tik");
            event->AddString("StG 44");
            break;
        case NA_ITALIAN:
            giveItem("weapons/It_W_Breda.tik");
            event->AddString("MP40");
            break;
        default:
            giveItem("weapons/bar.tik");
            event->AddString("BAR");
            break;
        }
    } else if (!Q_stricmp(client->pers.dm_primary, "heavy") && !(dmflags->integer & DF_WEAPON_NO_ROCKET)) {
        switch (nationality) {
        case NA_GERMAN:
        case NA_ITALIAN:
            giveItem("weapons/panzerschreck.tik");
            event->AddString("Panzerschreck");
            break;
        case NA_BRITISH:
            giveItem("weapons/Uk_W_Piat.tik");
            event->AddString("PIAT");
            break;
        default:
            giveItem("weapons/bazooka.tik");
            event->AddString("Bazooka");
            break;
        }
    } else if (!Q_stricmp(client->pers.dm_primary, "shotgun") && !(dmflags->integer & DF_WEAPON_NO_SHOTGUN)) {
        switch (nationality) {
        case NA_BRITISH:
            giveItem("weapons/DeLisle.tik");
            event->AddString("DeLisle");
            break;
        case NA_GERMAN:
            if (dmflags->integer & DF_DISALLOW_KAR98_MORTAR) {
                // Fallback to shotgun
                giveItem("weapons/shotgun.tik");
                event->AddString("Shotgun");
            } else {
                giveItem("weapons/kar98_mortar.tik");
                event->AddString("Gewehrgranate");
            }
            break;
        default:
            giveItem("weapons/shotgun.tik");
            event->AddString("Shotgun");
            break;
        }
    } else if (!Q_stricmp(client->pers.dm_primary, "landmine") && QueryLandminesAllowed()) {
        //gi.Cvar_Get("g_rifles_for_sweepers", "0", 0);

        switch (nationality) {
        case NA_BRITISH:
            giveItem("weapons/US_W_Minedetector.tik");
            event->AddString("Minedetector");

            if (g_rifles_for_sweepers->integer) {
                // Give a lite version of the rifle
                giveItem("weapons/enfield_lite.tik");
            } else {
                // Just give some ammo for the pistol
                GiveAmmo("pistol", 12);
            }
            break;
        case NA_RUSSIAN:
            giveItem("weapons/US_W_Minedetector.tik");
            event->AddString("Minedetector");

            if (g_rifles_for_sweepers->integer) {
                // Give a lite version of the rifle
                giveItem("weapons/Mosin_Nagant_Rifle_lite.tik");
            } else {
                // Just give some ammo for the pistol
                GiveAmmo("pistol", 14);
            }
            break;
        case NA_GERMAN:
            giveItem("weapons/Gr_W_Minedetector.tik");
            event->AddString("Minensuchgerat");

            if (g_rifles_for_sweepers->integer) {
                // Give a lite version of the rifle
                giveItem("weapons/kar98_lite.tik");
            } else {
                // Just give some ammo for the pistol
                GiveAmmo("pistol", 16);
            }
            break;
        default:
            giveItem("weapons/US_W_Minedetector.tik");
            event->AddString("Minedetector");

            if (g_rifles_for_sweepers->integer) {
                // Give a lite version of the rifle
                giveItem("weapons/m1_garand_lite.tik");
            } else {
                // Just give some ammo for the pistol
                GiveAmmo("pistol", 14);
            }
            break;
        }
    } else if (!(dmflags->integer & DF_WEAPON_NO_RIFLE)) {
        switch (nationality) {
        case NA_BRITISH:
            giveItem("weapons/enfield.tik");
            event->AddString("Lee-Enfield");
            break;
        case NA_RUSSIAN:
            giveItem("weapons/Mosin_Nagant_Rifle.tik");
            event->AddString("Mosin Nagant Rifle");
            break;
        case NA_GERMAN:
            giveItem("weapons/kar98.tik");
            event->AddString("Mauser KAR 98K");
            break;
        case NA_ITALIAN:
            giveItem("weapons/it_w_carcano.tik");
            event->AddString("Carcano");
            break;
        default:
            giveItem("weapons/m1_garand.tik");
            event->AddString("M1 Garand");
            break;
        }
    }

    // Make the player switch to the weapon some time after spawning
    PostEvent(event, 0.3f);

    //
    // Pistols and grenades
    //
    switch (nationality) {
    case NA_BRITISH:
        giveItem("weapons/mills_grenade.tik");
        giveItem("weapons/M18_smoke_grenade.tik");
        giveItem("weapons/Webley_Revolver.tik");
        break;
    case NA_RUSSIAN:
        giveItem("weapons/Russian_F1_grenade.tik");
        giveItem("weapons/RDG-1_Smoke_grenade.tik");
        giveItem("weapons/Nagant_revolver.tik");
        break;
    case NA_GERMAN:
        giveItem("weapons/steilhandgranate.tik");
        giveItem("weapons/nebelhandgranate.tik");
        giveItem("weapons/p38.tik");
        break;
    case NA_ITALIAN:
        giveItem("weapons/it_w_bomba.tik");
        giveItem("weapons/it_w_bombabreda.tik");
        giveItem("weapons/it_w_beretta.tik");
        break;
    default:
        giveItem("weapons/m2frag_grenade.tik");
        giveItem("weapons/M18_smoke_grenade.tik");
        giveItem("weapons/colt45.tik");
        break;
    }
}

void Player::EquipWeapons_ver8()
{
    // spectators should not have weapons
    if (IsSpectator()) {
        FreeInventory();
    } else {
        Event *ev = new Event("use");

        if (!Q_stricmp(client->pers.dm_primary, "rifle")) {
            if (dm_team == TEAM_ALLIES) {
                giveItem("models/weapons/m1_garand.tik");
                ev->AddString("models/weapons/m1_garand.tik");
            } else {
                giveItem("models/weapons/kar98.tik");
                ev->AddString("models/weapons/kar98.tik");
            }

            GiveAmmo("rifle", 100);
        } else if (!Q_stricmp(client->pers.dm_primary, "sniper")) {
            if (dm_team == TEAM_ALLIES) {
                giveItem("models/weapons/springfield.tik");
                ev->AddString("models/weapons/springfield.tik");
            } else {
                giveItem("models/weapons/kar98sniper.tik");
                ev->AddString("models/weapons/kar98sniper.tik");
            }
        } else if (!Q_stricmp(client->pers.dm_primary, "smg")) {
            if (dm_team == TEAM_ALLIES) {
                giveItem("models/weapons/thompsonsmg.tik");
                ev->AddString("models/weapons/thompsonsmg.tik");
            } else {
                giveItem("models/weapons/mp40.tik");
                ev->AddString("models/weapons/mp40.tik");
            }
        } else if (!Q_stricmp(client->pers.dm_primary, "mg")) {
            if (dm_team == TEAM_ALLIES) {
                giveItem("models/weapons/bar.tik");
                ev->AddString("models/weapons/bar.tik");
            } else {
                giveItem("models/weapons/mp44.tik");
                ev->AddString("models/weapons/mp44.tik");
            }
        } else if (!Q_stricmp(client->pers.dm_primary, "heavy")) {
            if (dm_team == TEAM_ALLIES) {
                giveItem("models/weapons/bazooka.tik");
                ev->AddString("models/weapons/bazooka.tik");
            } else {
                giveItem("models/weapons/panzerschreck.tik");
                ev->AddString("models/weapons/panzerschreck.tik");
            }
        } else if (!Q_stricmp(client->pers.dm_primary, "shotgun")) {
            giveItem("models/weapons/shotgun.tik");
            ev->AddString("models/weapons/shotgun.tik");
        }

        PostEvent(ev, 0.3f);

        if (dm_team == TEAM_ALLIES) {
            giveItem("models/weapons/colt45.tik");
            giveItem("models/weapons/m2frag_grenade.tik");
        } else {
            giveItem("models/weapons/p38.tik");
            giveItem("models/weapons/steilhandgranate.tik");
        }

        giveItem("models/items/binoculars.tik");
    }
}

void Player::Spectator(void)
{
    if (!IsSpectator()) {
        respawn_time = level.time + 1.0f;
    }

    RemoveFromVehiclesAndTurrets();

    m_bSpectator        = !m_bTempSpectator;
    m_iPlayerSpectating = 0;
    takedamage          = DAMAGE_NO;
    deadflag            = DEAD_NO;
    health              = max_health;

    client->ps.feetfalling = 0;
    movecontrol            = MOVECONTROL_USER;
    client->ps.pm_flags |= PMF_SPECTATING;

    EvaluateState(statemap_Torso->FindState("STAND"), statemap_Legs->FindState("STAND"));

    setSolidType(SOLID_NOT);
    setMoveType(MOVETYPE_NOCLIP);

    FreeInventory();

    hideModel();

    SetPlayerSpectateRandom();
}

bool Player::IsValidSpectatePlayer(Player *pPlayer)
{
    if (g_gametype->integer <= GT_FFA) {
        return true;
    }

    if (GetTeam() <= TEAM_FREEFORALL) {
        return true;
    }

    if (g_forceteamspectate->integer) {
        if (!GetDM_Team()->NumLivePlayers()) {
            return true;
        }

        if (pPlayer->GetTeam() == GetTeam()) {
            return true;
        }

        return false;
    }

    return true;
}

void Player::SetPlayerSpectate(bool bNext)
{
    int        i;
    int        dir;
    int        num;
    gentity_t *ent;
    Player    *pPlayer;

    if (bNext) {
        dir = 1;
        num = m_iPlayerSpectating;
    } else {
        dir = -1;
        if (m_iPlayerSpectating) {
            num = m_iPlayerSpectating - 2;
        } else {
            num = game.maxclients - 1;
        }
    }

    if (m_iPlayerSpectating >= game.maxclients) {
        m_iPlayerSpectating = 0;
    }

    for (i = num; i < game.maxclients && i >= 0; i += dir) {
        ent = &g_entities[i];
        if (!ent->inuse || !ent->entity) {
            continue;
        }

        pPlayer = (Player *)ent->entity;

        if (!pPlayer->IsDead() && !pPlayer->IsSpectator() && IsValidSpectatePlayer(pPlayer)) {
            m_iPlayerSpectating = i + 1;
            client->ps.camera_flags &= ~CF_CAMERA_CUT_BIT;
            client->ps.camera_flags |= (client->ps.camera_flags & CF_CAMERA_CUT_BIT) ^ CF_CAMERA_CUT_BIT;
            return;
        }

        if (!m_iPlayerSpectating) {
            return;
        }

        m_iPlayerSpectating = 0;
    }

    if (m_iPlayerSpectating) {
        m_iPlayerSpectating = 0;
        SetPlayerSpectate(bNext);
    }
}

void Player::SetPlayerSpectateRandom(void)
{
    Player *pPlayer;
    int     i;
    int     numvalid;
    int     iRandom;

    numvalid = 0;

    for (i = 0; i < game.maxclients; i++) {
        gentity_t *ent = &g_entities[i];
        if (!ent->inuse || !ent->entity) {
            continue;
        }

        pPlayer = static_cast<Player *>(ent->entity);
        if (!pPlayer->IsDead() && !pPlayer->IsSpectator() && IsValidSpectatePlayer(pPlayer)) {
            numvalid++;
        }
    }

    if (!numvalid) {
        // There is no valid player to spectate

        // Added in OPM.
        //  Clear the player spectating value
        m_iPlayerSpectating = 0;
        return;
    }

    iRandom = (int)(random() * numvalid);

    for (i = 0; i < game.maxclients; i++) {
        gentity_t *ent = &g_entities[i];
        if (!ent->inuse || !ent->entity) {
            continue;
        }

        pPlayer = static_cast<Player *>(ent->entity);
        if (!pPlayer->IsDead() && !pPlayer->IsSpectator() && IsValidSpectatePlayer(pPlayer)) {
            if (!iRandom) {
                m_iPlayerSpectating = i + 1;

                client->ps.camera_flags &= ~CF_CAMERA_CUT_BIT;
                client->ps.camera_flags |= (client->ps.camera_flags & CF_CAMERA_CUT_BIT) ^ CF_CAMERA_CUT_BIT;
                break;
            }

            iRandom--;
        }
    }
}

void Player::GetSpectateFollowOrientation(Player *pPlayer, Vector& vPos, Vector& vAng)
{
    Vector  forward, right, up;
    Vector  vCamOfs;
    Vector  start;
    trace_t trace;

    if (!g_spectatefollow_firstperson->integer) {
        // spectating a player
        vAng = pPlayer->GetVAngles();

        AngleVectors(vAng, forward, right, up);

        vCamOfs = pPlayer->origin;
        vCamOfs[2] += pPlayer->viewheight;

        vCamOfs += forward * g_spectatefollow_forward->value;
        vCamOfs += right * g_spectatefollow_right->value;
        vCamOfs += up * g_spectatefollow_up->value;

        if (pPlayer->client->ps.fLeanAngle != 0.0f) {
            vCamOfs += pPlayer->client->ps.fLeanAngle * 0.65f * right;
        }

        start = pPlayer->origin;
        start[2] += pPlayer->maxs[2] - 2.0;

        Vector vMins = Vector(-2, -2, 2);
        Vector vMaxs = Vector(2, 2, 2);

        trace =
            G_Trace(start, vMins, vMaxs, vCamOfs, pPlayer, MASK_SHOT, false, "Player::GetSpectateFollowOrientation");

        vAng[0] += g_spectatefollow_pitch->value * trace.fraction;
        vPos = trace.endpos;
    } else {
        vAng = pPlayer->angles;
        vPos = pPlayer->origin;
    }
}

void Player::Spectator(Event *ev)
{
    client->pers.dm_primary[0] = 0;
    SetTeam(TEAM_SPECTATOR);
}

void Player::Leave_DM_Team(Event *ev)
{
    // FIXME: should it stays disabled ?
#if 0
    if (current_team)
    {
        dmManager.LeaveTeam(this);
    }
    else
    {
        gi.centerprintf(edict, gi.LV_ConvertString("You are not on a team"));
    }
#endif
}

void Player::Join_DM_Team(Event *ev)
{
    teamtype_t  team;
    str         teamname;
    const char *join_message;
    float       startTime;
    Entity     *ent;

    teamname = ev->GetString(1);

    if (!teamname.icmp("allies")) {
        team = TEAM_ALLIES;
    } else if (!teamname.icmp("axis") || !teamname.icmp("german") || !teamname.icmp("nazi")) {
        team = TEAM_AXIS;
    } else {
        team = TEAM_AXIS;
    }

    if (current_team && current_team->m_teamnumber == team) {
        //
        // don't switch if on same team
        //
        return;
    }

    if (deadflag && deadflag != DEAD_DEAD) {
        // ignore team switching if the player is dying and not dead
        return;
    }

    startTime = dmManager.GetMatchStartTime();

    if (startTime >= 0.0f && (level.time - startTime) > 30.0
        && (level.time - m_fTeamSelectTime) < g_teamswitchdelay->integer) {
        int seconds = g_teamswitchdelay->integer - (level.time - m_fTeamSelectTime);

        gi.SendServerCommand(
            edict - g_entities,
            "print \"" HUD_MESSAGE_WHITE "%s %i %s\n\"",
            gi.LV_ConvertString("Can not change teams again for another"),
            seconds + 1,
            gi.LV_ConvertString("seconds")
        );
        return;
    }

    m_fTeamSelectTime = level.time;
    SetTeam(team);
    // Make sure to remove player from turret
    RemoveFromVehiclesAndTurrets();

    //
    // Since 2.0: Remove projectiles the player shot
    //
    for (ent = G_NextEntity(NULL); ent; ent = G_NextEntity(ent)) {
        if (ent->IsSubclassOfProjectile() && ent->edict->r.ownerNum == edict->r.ownerNum) {
            ent->PostEvent(EV_Remove, 0);
        }
    }

    if (client->pers.dm_primary[0]) {
        if (IsSpectator()) {
            if (m_fSpawnTimeLeft) {
                m_bWaitingForRespawn = true;
            } else if (AllowTeamRespawn()) {
                EndSpectator();

                if (deadflag) {
                    deadflag = DEAD_DEAD;
                }

                PostEvent(EV_Player_Respawn, 0);
                gi.centerprintf(edict, " ");
            }
        } else if (g_gametype->integer >= GT_TEAM) {
            client->pers.dm_primary[0] = 0;
            UserSelectWeapon(false);
            Spectator();
        } else {
            PostEvent(EV_Player_Respawn, 0);
        }
    } else if (IsSpectator()) {
        UserSelectWeapon(true);
    }

    if (g_gametype->integer >= GT_TEAM) {
        //
        // in team game modes, display a message to indicate
        // a player joined a team
        //
        if (GetTeam() == TEAM_ALLIES) {
            join_message = "has joined the Allies";
        } else if (GetTeam() == TEAM_AXIS) {
            join_message = "has joined the Axis";
        } else {
            return;
        }

        G_PrintToAllClients(va("%s %s\n", client->pers.netname, gi.LV_ConvertString(join_message)), 2);
    }
}

void Player::Auto_Join_DM_Team(Event *ev)
{
    Event *event = new Event(EV_Player_JoinDMTeam);

    if (dmManager.GetAutoJoinTeam() == TEAM_AXIS) {
        event->AddString("axis");
    } else {
        event->AddString("allies");
    }

    ProcessEvent(event);
}

teamtype_t Player::GetTeam() const
{
    return dm_team;
}

void Player::SetTeam(teamtype_t team)
{
    dmManager.JoinTeam(this, team);

    if (dm_team == TEAM_SPECTATOR) {
        Spectator();
    }
}

void Player::SetDM_Team(DM_Team *team)
{
    current_team = team;

    // clear the player's team
    edict->s.eFlags &= ~EF_ANY_TEAM;

    if (team) {
        dm_team = static_cast<teamtype_t>(team->getNumber());
        if (dm_team == TEAM_ALLIES) {
            edict->s.eFlags |= EF_ALLIES;
        } else if (dm_team == TEAM_AXIS) {
            edict->s.eFlags |= EF_AXIS;
        }
    } else {
        dm_team = TEAM_NONE;
    }

    client->pers.teamnum = dm_team;
    G_SetClientConfigString(edict);

    if (m_fTeamSelectTime != level.time && (edict->s.eFlags & (EF_ANY_TEAM))) {
        InitModel();
    }
}

DM_Team *Player::GetDM_Team()
{
    return current_team;
}

bool Player::IsSpectator(void)
{
    return (m_bSpectator || m_bTempSpectator);
}

void Player::BeginFight(void)
{
    m_bAllowFighting = true;
}

void Player::EndFight(void)
{
    m_bAllowFighting = false;
}

void Player::WarpToPoint(Entity *spawnpoint)
{
    if (spawnpoint) {
        setOrigin(spawnpoint->origin + Vector(0, 0, 1));
        setAngles(spawnpoint->angles);
        SetViewAngles(angles);
        client->ps.camera_flags &= ~CF_CAMERA_CUT_BIT;
        client->ps.camera_flags |= (client->ps.camera_flags & CF_CAMERA_CUT_BIT) ^ CF_CAMERA_CUT_BIT;
    }
}

void Player::UpdateStatus(const char *s)
{
    gi.SendServerCommand(edict - g_entities, "status \"%s\"", s);
}

void Player::HUDPrint(const char *s)
{
    gi.SendServerCommand(edict - g_entities, "hudprint \"%s\"\n", s);
}

void Player::GibEvent(Event *ev)
{
    qboolean hidemodel;

    hidemodel = !ev->GetInteger(1);

    if (com_blood->integer) {
        if (hidemodel) {
            gibbed     = true;
            takedamage = DAMAGE_NO;
            setSolidType(SOLID_NOT);
            hideModel();
        }

        CreateGibs(this, health, 0.75f, 3);
    }
}

void Player::ArmorDamage(Event *ev)
{
    int mod = ev->GetInteger(9);

    if (g_gametype->integer != GT_SINGLE_PLAYER) {
        // players that are not allowed fighting mustn't take damage
        if (!m_bAllowFighting && mod != MOD_TELEFRAG) {
            return;
        }

        Player *attacker = (Player *)ev->GetEntity(1);

        if (attacker && attacker->IsSubclassOfPlayer()) {
            if (attacker != this) {
                if (g_gametype->integer > 1 && !g_teamdamage->integer) {
                    // check for team damage
                    if (attacker->GetDM_Team() == GetDM_Team() && mod != MOD_TELEFRAG) {
                        return;
                    }
                }

                pAttackerDistPointer = attacker;
                fAttackerDispTime    = g_drawattackertime->value + level.time;
            }
        }
    }

    m_iNumHitsTaken++;

    Sentient::ArmorDamage(ev);

    Event *event = new Event;

    event->AddEntity(ev->GetEntity(1));
    event->AddFloat(ev->GetFloat(2));
    event->AddEntity(ev->GetEntity(3));
    event->AddVector(ev->GetVector(4));
    event->AddVector(ev->GetVector(5));
    event->AddVector(ev->GetVector(6));
    event->AddInteger(ev->GetInteger(7));
    event->AddInteger(ev->GetInteger(8));
    event->AddInteger(ev->GetInteger(9));
    event->AddInteger(ev->GetInteger(10));
    event->AddEntity(this);

    scriptedEvents[SE_DAMAGE].Trigger(event);
}

void Player::Disconnect(void)
{
    Event *ev = new Event;

    ev->AddListener(this);
    scriptedEvents[SE_DISCONNECTED].Trigger(ev);

    if (g_gametype->integer != GT_SINGLE_PLAYER) {
        dmManager.RemovePlayer(this);
    }
}

void Player::CallVote(Event *ev)
{
    str arg1;
    str arg2;
    int numVoters;

    if (g_gametype->integer == GT_SINGLE_PLAYER) {
        return;
    }

    if (!g_allowvote->integer) {
        HUDPrint(gi.LV_ConvertString("Voting not allowed here."));
        return;
    }

    if (level.m_voteTime != 0.0f) {
        HUDPrint(gi.LV_ConvertString("A vote is already in progress."));
        return;
    }

    if (votecount >= MAX_VOTE_COUNT) {
        if (m_fLastVoteTime) {
            while (m_fLastVoteTime < level.time && votecount > 0) {
                m_fLastVoteTime += 60;
                votecount--;
            }
        }

        if (votecount >= MAX_VOTE_COUNT) {
            HUDPrint(
                va("%s %d %s.\n",
                   gi.LV_ConvertString("You cannot call another vote for"),
                   (unsigned int)(m_fLastVoteTime - level.time + 1),
                   gi.LV_ConvertString("seconds"))
            );
            return;
        }
    }

    if (IsSpectator() || IsDead()) {
        HUDPrint(gi.LV_ConvertString("You are not allowed to call a vote as a spectator."));
        return;
    }

    arg1 = ev->GetString(1);
    if (ev->NumArgs() > 1) {
        arg2 = ev->GetString(2);
    }

    if (!atoi(arg1.c_str())) {
        if (strchr(arg1.c_str(), ';') || strchr(arg2.c_str(), ';')) {
            HUDPrint(gi.LV_ConvertString("Invalid vote string."));
            return;
        }

        if (Q_stricmp(arg1.c_str(), "restart") && Q_stricmp(arg1.c_str(), "nextmap") && Q_stricmp(arg1.c_str(), "map")
            && Q_stricmp(arg1.c_str(), "g_gametype") && Q_stricmp(arg1.c_str(), "kick")
            && Q_stricmp(arg1.c_str(), "clientkick") && Q_stricmp(arg1.c_str(), "fraglimit")) {
            HUDPrint(gi.LV_ConvertString("Invalid vote string."));
            HUDPrint(va(
                "%s restart, nextmap, map <mapname>, g_gametype <n>, fraglimit <n>, timelimit <n>, kick <player>, and "
                "clientkick <player #>.",
                gi.LV_ConvertString("Vote commands are:")
            ));

            return;
        }

        if (!Q_stricmp(arg1.c_str(), "kick")) {
            //
            // check for a valid player
            //
            gentity_t *ent;
            int        i;

            for (i = 0; i < game.maxclients; i++) {
                ent = &g_entities[i];

                if (!ent->inuse || !ent->client || !ent->entity) {
                    continue;
                }

                if (!Q_stricmp(ent->client->pers.netname, arg2.c_str())) {
                    // Prevent the player from kicking himself out
                    if (ent->entity == this) {
                        HUDPrint(gi.LV_ConvertString("You are not allowed to kick yourself."));
                        return;
                    }

                    break;
                }
            }

            if (i == game.maxclients) {
                HUDPrint(
                    va("%s %s", ent->client->pers.netname, gi.LV_ConvertString("is not a valid player name to kick."))
                );
            }
        } else if (!Q_stricmp(arg1.c_str(), "map") && *sv_nextmap->string) {
            level.m_voteString = va("%s %s; set next map \"%s\"", arg1.c_str(), arg2.c_str(), arg2.c_str());
        } else {
            level.m_voteString = va("%s %s", arg1.c_str(), arg2.c_str());
        }

        if (level.m_nextVoteTime) {
            level.m_nextVoteTime = 0;
            gi.SendConsoleCommand(va("%s", level.m_voteString.c_str()));
        }

        if (!Q_stricmp(arg1.c_str(), "g_gametype")) {
            int gametypeNum;

            // get the gametype number
            gametypeNum = atoi(arg2.c_str());
            if (gametypeNum <= GT_SINGLE_PLAYER || gametypeNum >= GT_MAX_GAME_TYPE) {
                HUDPrint(va("%s", gi.LV_ConvertString("Invalid gametype for a vote.")));
                return;
            }

            level.m_voteString = va("%s %i", arg1.c_str(), gametypeNum);

            switch (gametypeNum) {
            case GT_FFA:
                level.m_voteName = "Game Type Free-For-All";
                break;
            case GT_TEAM:
                level.m_voteName = "Game Type Match";
                break;
            case GT_TEAM_ROUNDS:
                level.m_voteName = "Game Type Round-Based-Match";
                break;
            case GT_OBJECTIVE:
                level.m_voteName = "Game Type Objective-Match";
                break;
            case GT_TOW:
                level.m_voteName = "Game Type Tug of War";
                break;
            case GT_LIBERATION:
                level.m_voteName = "Game Type Liberation";
                break;
            default:
                HUDPrint(va("%s %s %d", gi.LV_ConvertString("Game Type"), arg1.c_str(), gametypeNum));
                return;
            }
        } else if (!Q_stricmp(arg1.c_str(), "map")) {
            if (*sv_nextmap->string) {
                level.m_voteString = va("%s %s; set nextmap \"%s\"", arg1.c_str(), arg2.c_str(), sv_nextmap->string);
            } else {
                level.m_voteString = va("%s %s", arg1.c_str(), arg2.c_str());
            }

            level.m_voteName = va("Map %s", arg2.c_str());
        } else {
            level.m_voteString = va("%s %s", arg1.c_str(), arg2.c_str());
            level.m_voteName   = level.m_voteString;
        }
    } else {
        int              voteIndex;
        int              subListIndex;
        str              voteOptionCommand;
        str              voteOptionSubCommand;
        str              voteOptionName;
        str              voteOptionSubName;
        voteoptiontype_t optionType;

        union {
            int   optionInteger;
            float optionFloat;
            int   optionClientNum;
        };

        gentity_t *ent;

        char buffer[64];

        voteIndex = atoi(arg1.c_str());
        if (!level.GetVoteOptionMain(voteIndex, &voteOptionCommand, &optionType)) {
            HUDPrint(va("%s", gi.LV_ConvertString("Invalid vote option.")));
            return;
        }

        level.GetVoteOptionMainName(voteIndex, &voteOptionName);

        switch (optionType) {
        case VOTE_NO_CHOICES:
            level.m_voteString = voteOptionCommand;
            level.m_voteName   = voteOptionName;
            break;
        case VOTE_OPTION_LIST:
            subListIndex = atoi(arg2.c_str());

            if (!level.GetVoteOptionSub(voteIndex, subListIndex, &voteOptionSubCommand)) {
                HUDPrint(
                    va("%s %i %s \"%s\".\n",
                       gi.LV_ConvertString("Invalid vote choice"),
                       subListIndex,
                       gi.LV_ConvertString("for vote option"),
                       voteOptionName.c_str())
                );
                return;
            }

            level.m_voteString = va("%s %s", voteOptionCommand.c_str(), voteOptionSubCommand.c_str());
            // get the sub-option name
            level.GetVoteOptionSubName(voteIndex, subListIndex, &voteOptionSubName);
            level.m_voteName =
                va("%s %s", gi.LV_ConvertString(voteOptionName.c_str()), gi.LV_ConvertString(voteOptionSubName.c_str())
                );
            break;
        case VOTE_OPTION_TEXT:
            if (strchr(arg2.c_str(), ';')) {
                HUDPrint(va("%s\n", gi.LV_ConvertString("Invalid vote text entered.")));
                return;
            }

            level.m_voteString = va("%s %s", voteOptionCommand.c_str(), arg2.c_str());
            level.m_voteName   = va("%s %s", gi.LV_ConvertString(voteOptionName.c_str()), arg2.c_str());
            break;
        case VOTE_OPTION_INTEGER:
            optionInteger = atoi(arg2.c_str());
            Com_sprintf(buffer, sizeof(buffer), "%d", optionInteger);

            if (Q_stricmp(buffer, arg2.c_str())) {
                HUDPrint(va("%s\n", gi.LV_ConvertString("Invalid vote integer entered.")));
                return;
            }

            level.m_voteString = va("%s %i", voteOptionCommand.c_str(), optionInteger);
            level.m_voteName   = va("%s %i", gi.LV_ConvertString(voteOptionName.c_str()), optionInteger);
            break;
        case VOTE_OPTION_FLOAT:
            optionFloat = atof(arg2.c_str());
            Com_sprintf(buffer, sizeof(buffer), "%f", optionFloat);

            if (Q_stricmp(buffer, arg2.c_str())) {
                HUDPrint(va("%s\n", gi.LV_ConvertString("Invalid vote float entered.")));
                return;
            }

            level.m_voteString = va("%s %g", voteOptionCommand.c_str(), optionFloat);
            level.m_voteName   = va("%s %g", gi.LV_ConvertString(voteOptionName.c_str()), optionFloat);
            break;
        case VOTE_OPTION_CLIENT:
        case VOTE_OPTION_CLIENT_NOT_SELF:
            optionClientNum = atoi(arg2.c_str());
            if (optionClientNum < 0 || optionClientNum >= game.maxclients) {
                HUDPrint(va("%s\n", gi.LV_ConvertString("Invalid client number for a vote.")));
                return;
            }

            ent = &g_entities[optionClientNum];
            if (!ent->inuse || !ent->client || !ent->entity) {
                HUDPrint(va("%s\n", gi.LV_ConvertString("Client selected for the vote is not connected.")));
                return;
            }

            level.m_voteString = va("%s %i", voteOptionCommand.c_str(), optionClientNum);
            level.m_voteName =
                va("%s #%i: %s", gi.LV_ConvertString(voteOptionName.c_str()), optionClientNum, ent->client->pers.netname
                );
            break;
        default:
            level.GetVoteOptionMainName(voteIndex, &voteOptionName);
            gi.Printf(
                "ERROR: Vote option (\"%s\" \"%s\") with unknown vote option type\n",
                voteOptionName.c_str(),
                voteOptionCommand.c_str()
            );
            return;
        }
    }

    G_PrintToAllClients(va("%s %s.\n", client->pers.netname, gi.LV_ConvertString("called a vote")));

    level.m_voteTime = (level.svsFloatTime - level.svsStartFloatTime) * 1000;
    level.m_voteYes  = 1;
    level.m_voteNo   = 0;

    // Reset all player's vote
    numVoters = 0;

    for (int i = 0; i < game.maxclients; i++) {
        gentity_t *ent = &g_entities[i];

        if (!ent->client || !ent->inuse) {
            continue;
        }

        Player *p = (Player *)ent->entity;
        p->voted  = false;

        numVoters++;
    }

    level.m_numVoters = numVoters;
    client->ps.voted  = true;
    voted             = true;
    votecount++;

    m_fLastVoteTime = level.time + 60;

    if (g_protocol >= protocol_e::PROTOCOL_MOHTA_MIN) {
        //
        // clients below version 2.0 don't support vote cs
        //
        gi.setConfigstring(CS_VOTE_TIME, va("%i", level.m_voteTime));
        gi.setConfigstring(CS_VOTE_STRING, level.m_voteName.c_str());
        gi.setConfigstring(CS_VOTE_YES, va("%i", level.m_voteYes));
        gi.setConfigstring(CS_VOTE_NO, va("%i", level.m_voteNo));
        gi.setConfigstring(CS_VOTE_UNDECIDED, va("%i", level.m_numVoters - (level.m_voteYes + level.m_voteNo)));
    }
}

void Player::Vote(Event *ev)
{
    str arg1;

    if (level.m_voteTime == 0.0f) {
        HUDPrint(gi.LV_ConvertString("No vote in progress."));
        return;
    }

    if (client->ps.voted) {
        HUDPrint(gi.LV_ConvertString("Vote already cast."));
        return;
    }

    if (ev->NumArgs() != 1) {
        HUDPrint(va("%s: vote <1|0|y|n>", gi.LV_ConvertString("Usage")));
        return;
    }

    HUDPrint(gi.LV_ConvertString("Vote cast."));
    client->ps.voted = true;

    arg1  = ev->GetString(1);
    voted = (arg1[0] == 'y') || (arg1[0] == 'Y') || (arg1[0] == '1');
}

void Player::RetrieveVoteOptions(Event *ev)
{
    if (m_fNextVoteOptionTime > level.time) {
        gi.SendServerCommand(edict - g_entities, "vo0 \"\"\n");
        gi.SendServerCommand(edict - g_entities, "vo2 \"\"\n");
    } else {
        m_fNextVoteOptionTime = level.time + 2.0;
        level.SendVoteOptionsFile(edict);
    }
}

void Player::EventPrimaryDMWeapon(Event *ev)
{
    str  dm_weapon = ev->GetString(1);
    bool bIsBanned;

    if (!str::icmp(dm_weapon, "shotgun")) {
        bIsBanned = (dmflags->integer & DF_WEAPON_NO_SHOTGUN);
    } else if (!str::icmp(dm_weapon, "rifle")) {
        bIsBanned = (dmflags->integer & DF_WEAPON_NO_RIFLE);
    } else if (!str::icmp(dm_weapon, "sniper")) {
        bIsBanned = (dmflags->integer & DF_WEAPON_NO_SNIPER);
    } else if (!str::icmp(dm_weapon, "smg")) {
        bIsBanned = (dmflags->integer & DF_WEAPON_NO_SMG);
    } else if (!str::icmp(dm_weapon, "mg")) {
        bIsBanned = (dmflags->integer & DF_WEAPON_NO_MG);
    } else if (!str::icmp(dm_weapon, "heavy")) {
        bIsBanned = (dmflags->integer & DF_WEAPON_NO_ROCKET);
    } else if (!str::icmp(dm_weapon, "landmine")) {
        bIsBanned = (dmflags->integer & DF_WEAPON_NO_LANDMINE) || !QueryLandminesAllowed();
    }

    if (bIsBanned) {
        gi.SendServerCommand(
            edict - g_entities, "print \"" HUD_MESSAGE_WHITE "%s\n\"", "That weapon is currently banned."
        );
        return;
    }

    Q_strncpyz(client->pers.dm_primary, dm_weapon.c_str(), sizeof(client->pers.dm_primary));

    if (m_bSpectator) {
        if (current_team && (current_team->m_teamnumber == TEAM_AXIS || current_team->m_teamnumber == TEAM_ALLIES)) {
            if (m_fSpawnTimeLeft) {
                m_bWaitingForRespawn = true;
            } else if (AllowTeamRespawn()) {
                EndSpectator();

                if (deadflag) {
                    deadflag = DEAD_DEAD;
                }

                PostEvent(EV_Player_Respawn, 0);

                gi.centerprintf(edict, "");
            }
        } else {
            gi.SendServerCommand(edict - g_entities, "stufftext \"wait 250;pushmenu_teamselect\"");
        }
    } else {
        gi.SendServerCommand(
            edict - g_entities, "print \"" HUD_MESSAGE_WHITE "%s\n\"", "Will switch to new weapon next time you respawn"
        );
    }
}

void Player::DeadBody(Event *ev)
{
    Body *body;

    if (knockdown) {
        return;
    }

    knockdown = true;

    body = new Body;
    body->setModel(model);

    for (int i = 0; i < MAX_FRAMEINFOS; i++) {
        body->edict->s.frameInfo[i] = edict->s.frameInfo[i];
    }

    body->edict->s.actionWeight = edict->s.actionWeight;
    body->edict->s.scale        = edict->s.scale;

    body->setOrigin(origin);
    body->setAngles(angles);

    body->edict->s.eFlags &= ~(EF_AXIS | EF_ALLIES);

    if (GetTeam() == TEAM_ALLIES) {
        edict->s.eFlags |= EF_ALLIES;
    } else if (GetTeam() == TEAM_AXIS) {
        edict->s.eFlags |= EF_AXIS;
    }
}

void Player::WonMatch(void)
{
    num_won_matches++;
}

void Player::LostMatch(void)
{
    num_lost_matches++;
}

void Player::ArmWithWeapons(Event *ev)
{
    EquipWeapons();
}

void Player::EventGetCurrentDMWeaponType(Event *ev)
{
    ev->AddString(GetCurrentDMWeaponType());
}

void Player::PhysicsOff(Event *ev)
{
    flags |= FL_IMMOBILE;
}

void Player::PhysicsOn(Event *ev)
{
    flags &= ~FL_IMMOBILE;
}

void Player::GetIsSpectator(Event *ev)
{
    ev->AddInteger(IsSpectator());
}

void Player::EventSetInJail(Event *ev)
{
    m_bIsInJail = ev->GetBoolean(1);
}

bool Player::IsInJail() const
{
    return m_bIsInJail;
}

void Player::EventGetInJail(Event *ev)
{
    ev->AddInteger(m_bIsInJail);
}

void Player::GetNationalityPrefix(Event *ev)
{
    nationality_t nationality;

    if (GetTeam() == TEAM_AXIS) {
        nationality = GetPlayerTeamType(client->pers.dm_playergermanmodel);
    } else {
        nationality = GetPlayerTeamType(client->pers.dm_playermodel);
    }

    switch (nationality) {
    case NA_RUSSIAN:
        ev->AddString("dfrru");
        break;
    case NA_ITALIAN:
        ev->AddString("denit");
        break;
    case NA_BRITISH:
        ev->AddString("dfruk");
        break;
    case NA_AMERICAN:
        ev->AddString("dfr");
        break;

    case NA_NONE:
    default:
        ev->AddString("dfr");
        break;
    }
}

void Player::GetIsDisguised(Event *ev)
{
    ev->AddInteger(m_bIsDisguised);
}

void Player::GetHasDisguise(Event *ev)
{
    ev->AddInteger(m_bHasDisguise);
}

void Player::SetHasDisguise(Event *ev)
{
    m_bHasDisguise = ev->GetBoolean(1);
}

void Player::SetObjectiveCount(Event *ev)
{
    m_iObjectivesCompleted = ev->GetInteger(1);
    m_iNumObjectives       = ev->GetInteger(2);
}

void Player::Stats(Event *ev)
{
    char entry[2048];
    int  i;
    str  szPreferredWeapon;
    str  szGunneryEvaluation;
    int  iNumHeadShots;
    int  iNumTorsoShots;
    int  iNumLeftLegShots;
    int  iNumRightLegShots;
    int  iNumGroinShots;
    int  iNumLeftArmShots;
    int  iNumRightArmShots;
    int  iNumShotsFired;
    int  iNumHits;
    int  iBestNumHits;

    if (g_gametype->integer != GT_SINGLE_PLAYER) {
        // Only works in singleplayer
        return;
    }

    szPreferredWeapon   = "none";
    szGunneryEvaluation = "none";
    iNumHeadShots       = m_iNumHeadShots;
    iNumTorsoShots      = m_iNumTorsoShots;
    iNumLeftLegShots    = m_iNumLeftLegShots;
    iNumRightLegShots   = m_iNumRightLegShots;
    iNumGroinShots      = m_iNumGroinShots;
    iNumLeftArmShots    = m_iNumLeftArmShots;
    iNumRightArmShots   = m_iNumRightArmShots;
    iNumShotsFired      = m_iNumShotsFired;
    iNumHits            = m_iNumHits;
    iBestNumHits        = 0;

    for (i = 1; i <= inventory.NumObjects(); i++) {
        Entity *pEnt = G_GetEntity(inventory.ObjectAt(i));
        if (pEnt->IsSubclassOfWeapon()) {
            Weapon *pWeap = static_cast<Weapon *>(pEnt);

            iNumHeadShots += pWeap->m_iNumHeadShots;
            iNumTorsoShots += pWeap->m_iNumTorsoShots;
            iNumLeftLegShots += pWeap->m_iNumLeftLegShots;
            iNumRightLegShots += pWeap->m_iNumRightLegShots;
            iNumGroinShots += pWeap->m_iNumGroinShots;
            iNumLeftArmShots += pWeap->m_iNumLeftArmShots;
            iNumRightArmShots += pWeap->m_iNumRightArmShots;
            iNumShotsFired += pWeap->m_iNumShotsFired;
            iNumHits += pWeap->m_iNumHits;

            if (pWeap->m_iNumHits > iBestNumHits) {
                szPreferredWeapon = pWeap->item_name;
                iBestNumHits      = pWeap->m_iNumHits;
            }
        }
    }

    szPreferredWeapon = m_sPerferredWeaponOverride;

    if (iNumHits) {
        Com_sprintf(
            entry,
            sizeof(entry),
            "%i %i %i %i %.1f \"%s\" %i %i %i \"%.1f\" \"%.1f\" \"%.1f\" \"%.1f\" \"%.1f\" \"%.1f\" \"%.1f\" \"%s\" %i "
            "%i %i",
            m_iNumObjectives,
            m_iObjectivesCompleted,
            iNumShotsFired,
            iNumHits,
            (iNumHits / iNumShotsFired * 100.f),
            szPreferredWeapon.c_str(),
            m_iNumHitsTaken,
            m_iNumObjectsDestroyed,
            m_iNumEnemiesKilled,
            iNumHeadShots * 100.f / iNumHits,
            iNumTorsoShots * 100.f / iNumHits,
            iNumLeftLegShots * 100.f / iNumHits,
            iNumRightLegShots * 100.f / iNumHits,
            iNumGroinShots * 100.f / iNumHits,
            iNumLeftArmShots * 100.f / iNumHits,
            iNumRightArmShots * 100.f / iNumHits,
            szGunneryEvaluation.c_str(),
            g_gotmedal->integer,
            g_success->integer,
            g_failed->integer
        );
    } else {
        Com_sprintf(
            entry,
            sizeof(entry),
            "%i %i %i %i %i \"%s\" %i %i %i \"%i\" \"%i\" \"%i\" \"%i\" \"%i\" \"%i\" \"%i\" \"%s\" %i %i %i",
            m_iNumObjectives,
            m_iObjectivesCompleted,
            iNumShotsFired,
            0,
            0,
            szPreferredWeapon.c_str(),
            m_iNumHitsTaken,
            m_iNumObjectsDestroyed,
            m_iNumEnemiesKilled,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            szGunneryEvaluation.c_str(),
            g_gotmedal->integer,
            g_success->integer,
            g_failed->integer
        );
    }

    gi.SendServerCommand(edict - g_entities, "stats %s", entry);
}

void Player::EventStuffText(Event *ev)
{
    if (level.spawning) {
        Event *event = new Event(EV_Player_StuffText);
        event->AddValue(ev->GetValue(1));
        PostEvent(event, level.frametime, 0);
    } else {
        gi.SendServerCommand(edict - g_entities, "stufftext \"%s\"", ev->GetString(1).c_str());
    }
}

void Player::EventSetVoiceType(Event *ev)
{
    str sVoiceName = ev->GetString(1);

    if (!sVoiceName.icmp("airborne")) {
        m_voiceType = PVT_ALLIED_AIRBORNE;
    } else if (!sVoiceName.icmp("manon")) {
        m_voiceType = PVT_ALLIED_MANON;
    } else if (!sVoiceName.icmp("SAS")) {
        m_voiceType = PVT_ALLIED_SAS;
    } else if (!sVoiceName.icmp("pilot")) {
        m_voiceType = PVT_ALLIED_PILOT;
    } else if (!sVoiceName.icmp("army")) {
        m_voiceType = PVT_ALLIED_ARMY;
    } else if (!sVoiceName.icmp("ranger")) {
        m_voiceType = PVT_ALLIED_RANGER;
    } else if (!sVoiceName.icmp("axis1")) {
        m_voiceType = PVT_AXIS_AXIS1;
    } else if (!sVoiceName.icmp("axis2")) {
        m_voiceType = PVT_AXIS_AXIS2;
    } else if (!sVoiceName.icmp("axis3")) {
        m_voiceType = PVT_AXIS_AXIS3;
    } else if (!sVoiceName.icmp("axis4")) {
        m_voiceType = PVT_AXIS_AXIS4;
    } else if (!sVoiceName.icmp("axis5")) {
        m_voiceType = PVT_AXIS_AXIS5;
    } else {
        m_voiceType = PVT_NONE_SET;
    }
}

void Player::GetTeamDialogPrefix(str& outPrefix)
{
    if (GetTeam() == TEAM_AXIS) {
        outPrefix = "axis_";
    } else {
        outPrefix = "allied_";
    }

    if (g_protocol >= protocol_e::PROTOCOL_MOHTA_MIN) {
        switch (m_voiceType) {
        case PVT_ALLIED_AMERICAN:
            outPrefix += "american_";
            break;
        case PVT_ALLIED_BRITISH:
            outPrefix += "british_";
            break;
        case PVT_ALLIED_RUSSIAN:
            outPrefix += "russian_";
            break;
        case PVT_AXIS_START:
            outPrefix += "german_";
            break;
        case PVT_AXIS_ITALIAN:
            outPrefix += "italian_";
            break;
        default:
            if (GetTeam() == TEAM_AXIS) {
                outPrefix += "german_";
            } else {
                outPrefix += "american_";
            }
            break;
        }
    } else {
        switch (m_voiceType) {
        case PVT_ALLIED_AIRBORNE:
            outPrefix += "airborne_";
            break;

        case PVT_ALLIED_MANON:
            outPrefix += "manon_";
            break;

        case PVT_ALLIED_SAS:
            outPrefix += "sas_";
            break;

        case PVT_ALLIED_PILOT:
            outPrefix += "pilot_";
            break;

        case PVT_ALLIED_ARMY:
            outPrefix += "army_";
            break;

        case PVT_ALLIED_RANGER:
            outPrefix += "ranger_";
            break;

        case PVT_AXIS_AXIS1:
            outPrefix += "axis1_";
            break;

        case PVT_AXIS_AXIS2:
            outPrefix += "axis2_";
            break;

        case PVT_AXIS_AXIS3:
            outPrefix += "axis3_";
            break;

        case PVT_AXIS_AXIS4:
            outPrefix += "axis4_";
            break;

        case PVT_AXIS_AXIS5:
            outPrefix += "axis5_";
            break;

        default:
            if (dm_team != TEAM_AXIS) {
                outPrefix += "army_";
            } else {
                outPrefix += "axis4_";
            }
        }
    }
}

void Player::PlayInstantMessageSound(const char *name)
{
    str soundName;

    if (g_protocol < PROTOCOL_MOHTA_MIN) {
        gi.DPrintf2("Instant message sound isn't supported on protocol below version 15");
        return;
    }

    GetTeamDialogPrefix(soundName);

    soundName += name;

    gi.MSG_SetClient(client->ps.clientNum);
    gi.MSG_StartCGM(CGM_VOICE_CHAT);
    gi.MSG_WriteCoord(m_vViewPos[0]);
    gi.MSG_WriteCoord(m_vViewPos[1]);
    gi.MSG_WriteCoord(m_vViewPos[2]);
    gi.MSG_WriteBits(0, 1);
    gi.MSG_WriteBits(edict - g_entities, 6);
    gi.MSG_WriteString(soundName.c_str());
    gi.MSG_EndCGM();
}

void Player::EventDMMessage(Event *ev)
{
    int              i;
    int              iStringLength;
    int              iMode = 0;
    str              sToken;
    char             szPrintString[MAX_SAY_TEXT]; // it's MAX_STRING_CHARS in mohaa
    const char      *pTmpInstantMsg = "";
    qboolean         bInstaMessage  = qfalse;
    AliasListNode_t *pSoundAlias    = NULL;
    const char      *pszAliasname   = NULL;
    str              sAliasName;
    str              sRandomAlias;
    gentity_t       *ent;

    if (g_gametype->integer == GT_SINGLE_PLAYER) {
        return;
    }

    if (ev->NumArgs() <= 1) {
        return;
    }

    sToken = ev->GetString(2);

    // Check for taunts
    if (sToken.length() == 3 && *sToken == '*' && sToken[1] > '0' && sToken[1] <= '9' && sToken[2] > '0'
        && sToken[2] <= '9') {
        unsigned int n1, n2;

        if (IsSpectator() || IsDead()) {
            // spectators or death players can't talk
            return;
        }

        GetTeamDialogPrefix(sAliasName);
        sAliasName += va("%c%c", (sToken[1] + '0'), (sToken[2] + '0'));
        sRandomAlias = GetRandomAlias(sAliasName, &pSoundAlias);

        // find a random alias
        if (sRandomAlias.length() > 0) {
            pszAliasname = sRandomAlias.c_str();
        }

        if (!pszAliasname) {
            pszAliasname = gi.GlobalAlias_FindRandom(sAliasName, &pSoundAlias);
        }

        if (!pszAliasname || !pSoundAlias) {
            return;
        }

        n1 = sToken[1] - '1';
        n2 = sToken[2] - '1';

        if (g_protocol >= PROTOCOL_MOHTA_MIN) {
            if (n1 >= ARRAY_LEN(pInstantMsgEng) || n2 >= ARRAY_LEN(pInstantMsgEng[0])) {
                return;
            }

            pTmpInstantMsg = pInstantMsgEng[n1][n2];
        } else {
            if (n1 >= ARRAY_LEN(pInstantMsgEng_ver6) || n2 >= ARRAY_LEN(pInstantMsgEng_ver6[0])) {
                return;
            }

            // fallback to old version
            pTmpInstantMsg = pInstantMsgEng_ver6[n1][n2];
        }

        bInstaMessage = qtrue;

        if (g_gametype->integer == GT_FFA) {
            iMode = 0;
        } else {
            if (n1 == 4) {
                iMode = 0;
            } else {
                iMode = -1;
            }

            if (g_voiceChatTime->value > 0) {
                m_fTalkTime = g_voiceChatTime->value + level.time;
            }
        }
    }

    if (!bInstaMessage) {
        iMode = ev->GetInteger(1);
        if (g_textChatTime->value > 0) {
            m_fTalkTime = g_textChatTime->value + level.time;
        }
    }

    strcpy(szPrintString, "print \"" HUD_MESSAGE_CHAT_WHITE);

    if (m_bSpectator) {
        if (iMode <= 0) {
            strcat(szPrintString, gi.CL_LV_ConvertString("(spectator)"));
            strcat(szPrintString, " ");
        } else if (iMode <= game.maxclients) {
            ent = &g_entities[iMode - 1];

            if (ent->inuse && ent->entity && !static_cast<Player *>(ent->entity)->IsSpectator()) {
                str errorString = gi.LV_ConvertString("Message Error");
                str reasonString =
                    gi.LV_ConvertString("Spectators are not allowed to send private messages to non-spectators");

                gi.SendServerCommand(
                    edict - g_entities,
                    "print \"" HUD_MESSAGE_CHAT_WHITE "%s: %s.\n\"",
                    errorString.c_str(),
                    reasonString.c_str()
                );
                return;
            }
        }
    } else if (IsDead() || m_bTempSpectator) {
        if (iMode <= 0) {
            strcat(szPrintString, gi.CL_LV_ConvertString("(dead)"));
            strcat(szPrintString, " ");
        } else if (iMode <= game.maxclients) {
            ent = &g_entities[iMode - 1];

            if (ent->inuse && ent->entity && !static_cast<Player *>(ent->entity)->IsSpectator()) {
                str errorString = gi.LV_ConvertString("Message Error");
                str reasonString =
                    gi.LV_ConvertString("Dead players are not allowed to send private messages to active players");

                gi.SendServerCommand(
                    edict - g_entities,
                    "print \"" HUD_MESSAGE_CHAT_WHITE "%s: %s.\n\"",
                    errorString.c_str(),
                    reasonString.c_str()
                );
                return;
            }
        }
    } else if (iMode < 0) {
        strcat(szPrintString, gi.CL_LV_ConvertString("(team)"));
        strcat(szPrintString, " ");
    } else if (iMode > 0) {
        strcat(szPrintString, gi.CL_LV_ConvertString("(private)"));
        strcat(szPrintString, " ");
    }

    strcat(szPrintString, client->pers.netname);

    if (bInstaMessage) {
        strcat(szPrintString, ": ");
        strcat(szPrintString, gi.LV_ConvertString(pTmpInstantMsg));
    } else {
        bool met_comment;

        strcat(szPrintString, ":");
        iStringLength = strlen(szPrintString);

        for (i = 2; i <= ev->NumArgs(); i++) {
            sToken = ev->GetString(i);
            // Added in 2.40
            //  Special battle language tokens
            //  So players can easily tell their position, health, etc.
            sToken = TranslateBattleLanguageTokens(sToken);

            if (iStringLength + sToken.length() > (ARRAY_LEN(szPrintString) - 1)) {
                break;
            }

            // Added in OPM.
            //  Checks for comments in string (as COM_Parse will parse them)
            if (strstr(sToken, "/*")) {
                met_comment = true;
            }

            if (strstr(sToken, "*/") && met_comment) {
                // ignore messages containing comments
                return;
            }

            strcat(szPrintString, " ");
            strcat(szPrintString, gi.LV_ConvertString(sToken));
        }
    }

    strcat(szPrintString, "\n");

    // ignore names containing comments
    if (strstr(client->pers.netname, "//")
        || (strstr(client->pers.netname, "/*") && strstr(client->pers.netname, "*/"))) {
        return;
    }

    if (iMode == 0) {
        //
        // team message
        //
        if (!IsSpectator() || g_spectate_allow_full_chat->integer) {
            for (i = 0; i < game.maxclients; i++) {
                ent = &g_entities[i];

                if (!ent->inuse || !ent->entity) {
                    continue;
                }

                gi.SendServerCommand(i, "%s", szPrintString);

                if (bInstaMessage) {
                    gi.MSG_SetClient(i);
                    gi.MSG_StartCGM(BG_MapCGMToProtocol(g_protocol, CGM_VOICE_CHAT));
                    gi.MSG_WriteCoord(m_vViewPos[0]);
                    gi.MSG_WriteCoord(m_vViewPos[1]);
                    gi.MSG_WriteCoord(m_vViewPos[2]);
                    gi.MSG_WriteBits(qtrue, 1);
                    gi.MSG_WriteBits(edict - g_entities, 6);
                    gi.MSG_WriteString(sAliasName.c_str());
                    gi.MSG_EndCGM();
                }
            }
        } else {
            //
            // send a message to spectators
            //
            for (i = 0; i < game.maxclients; i++) {
                ent = &g_entities[i];

                if (!ent->inuse || !ent->entity) {
                    continue;
                }

                if (!static_cast<Player *>(ent->entity)->IsSpectator()) {
                    continue;
                }

                gi.SendServerCommand(i, "%s", szPrintString);
            }
        }
    } else if (iMode < 0) {
        //
        // team message
        //
        if (IsSpectator()) {
            for (i = 0; i < game.maxclients; i++) {
                ent = &g_entities[i];

                if (!ent->inuse || !ent->entity) {
                    continue;
                }

                if (!static_cast<Player *>(ent->entity)->IsSpectator()) {
                    continue;
                }

                gi.SendServerCommand(i, "%s", szPrintString);
            }
        } else {
            for (i = 0; i < game.maxclients; i++) {
                ent = &g_entities[i];

                if (!ent->inuse || !ent->entity) {
                    continue;
                }

                if (static_cast<Player *>(ent->entity)->GetTeam() != GetTeam()) {
                    gi.MSG_SetClient(i);
                    gi.MSG_StartCGM(BG_MapCGMToProtocol(g_protocol, CGM_VOICE_CHAT));
                    gi.MSG_WriteCoord(m_vViewPos[0]);
                    gi.MSG_WriteCoord(m_vViewPos[1]);
                    gi.MSG_WriteCoord(m_vViewPos[2]);
                    gi.MSG_WriteBits(qtrue, 1);
                    gi.MSG_WriteBits(edict - g_entities, 6);
                    gi.MSG_WriteString(sAliasName.c_str());
                    gi.MSG_EndCGM();

                    continue;
                }

                gi.SendServerCommand(i, "%s", szPrintString);

                if (bInstaMessage) {
                    gi.MSG_SetClient(i);
                    gi.MSG_StartCGM(BG_MapCGMToProtocol(g_protocol, CGM_VOICE_CHAT));
                    gi.MSG_WriteCoord(m_vViewPos[0]);
                    gi.MSG_WriteCoord(m_vViewPos[1]);
                    gi.MSG_WriteCoord(m_vViewPos[2]);
                    gi.MSG_WriteBits(qfalse, 1);
                    gi.MSG_WriteBits(edict - g_entities, 6);
                    gi.MSG_WriteString(sAliasName.c_str());
                    gi.MSG_EndCGM();
                }
            }
        }
    } else if (iMode <= game.maxclients) {
        ent = &g_entities[iMode - 1];

        if (!ent->inuse || !ent->entity) {
            str errorString  = gi.LV_ConvertString("Message Error");
            str reasonString = gi.LV_ConvertString("is not a connected client");

            gi.SendServerCommand(
                edict - g_entities,
                "print \"" HUD_MESSAGE_CHAT_WHITE "%s: %i %s.\n\"",
                errorString.c_str(),
                iMode,
                reasonString.c_str()
            );
            return;
        }

        gi.SendServerCommand(iMode - 1, "%s", szPrintString);

        if (ent->entity != this) {
            gi.SendServerCommand(
                edict - g_entities,
                "print \"" HUD_MESSAGE_CHAT_WHITE "%s %i:\n\"",
                gi.LV_ConvertString("Message to player"),
                iMode
            );
            gi.SendServerCommand(edict - g_entities, "%s", szPrintString);
        }
    } else {
        str errorString  = gi.LV_ConvertString("Message Error");
        str reasonString = gi.LV_ConvertString("is a bad client number");

        gi.SendServerCommand(
            edict - g_entities,
            "print \"" HUD_MESSAGE_CHAT_WHITE "%s: %i %s.\n\"",
            errorString.c_str(),
            iMode,
            reasonString.c_str()
        );
        return;
    }
}

str Player::GetBattleLanguageCondition() const
{
    int healthRatio;

    if (health <= 0) {
        return "dead";
    }

    if (health >= max_health) {
        return "just peachy";
    }

    healthRatio = (health * 5.f) / max_health;
    switch (healthRatio) {
    case 0:
        return "almost dead";
    case 1:
        return "severly wounded";
    case 2:
        return "wounded";
    case 3:
        return "slightly wounded";
    case 4:
    default:
        return "pretty good";
    }
}

str Player::GetBattleLanguageDirection() const
{
    int dir = ((m_vViewAng.y - world->m_fNorth) + 22.5f + 360.f) / 45.f;
    switch (dir % 8) {
    case 0:
        return "North";
    case 1:
        return "North West";
    case 2:
        return "West";
    case 3:
        return "South West";
    case 4:
        return "South";
    case 5:
        return "South East";
    case 6:
        return "East";
    case 7:
        return "North East";
    default:
        return "???";
    }
}

str Player::GetBattleLanguageLocation() const
{
    return gi.CL_LV_ConvertString(level.GetDMLocation(m_vViewPos).c_str());
}

str Player::GetBattleLanguageLocalFolks()
{
    static char buf[256];
    char       *p;
    char       *curP;
    size_t      remaining;
    size_t      length;
    Player     *pPlayer;
    Player     *pFolk;
    gentity_t  *ent;
    int         i;

    remaining = ARRAY_LEN(buf) - 1;
    p         = buf;
    curP      = NULL;
    pFolk     = NULL;

    for (i = 0; i < game.maxclients; i++) {
        ent = &g_entities[i];
        if (!ent->inuse || !ent->entity) {
            continue;
        }

        pPlayer = static_cast<Player *>(ent->entity);
        if (pPlayer != this && pPlayer->GetTeam() == GetTeam() && CanSee(pPlayer, 360, 1600, false)) {
            if (p != buf) {
                if (remaining < 2) {
                    // No more space remaining
                    break;
                }

                strcpy(p, ", ");
                p += 2;
                curP = p;
                remaining -= 2;
            }

            length = strlen(client->pers.netname);
            if (remaining < length) {
                break;
            }

            strcpy(p, client->pers.netname);
            p += length;
            remaining -= length;
            pFolk = pPlayer;
        }
    }

    if (curP && remaining >= 2) {
        strcpy(curP, "and ");
        strcpy(curP + strlen(curP), pFolk->client->pers.netname);
    } else if (!pFolk) {
        return "nobody";
    }

    return buf;
}

str Player::GetBattleLanguageWeapon() const
{
    return GetCurrentDMWeaponType().c_str();
}

str Player::GetBattleLanguageDistance() const
{
    Vector  vStart, vEnd;
    Vector  vForward;
    trace_t trace;
    float   dist;

    vStart = m_vViewPos;
    AngleVectors(m_vViewAng, vForward, NULL, NULL);

    vEnd = vStart + vForward * 10240;

    trace = G_Trace(
        vStart,
        vec_zero,
        vec_zero,
        vEnd,
        static_cast<const Entity *>(this),
        MASK_BATTLELANGUAGE,
        qfalse,
        "Player::GetBattleLanguageDistance"
    );

    dist = (vStart - trace.endpos).length();

    if (g_qunits_to_feet->integer) {
        return GetBattleLanguageDistanceFeet(dist);
    } else {
        return GetBattleLanguageDistanceMeters(dist);
    }
}

str Player::GetBattleLanguageDistanceMeters(float dist) const
{
    int meters;

    meters = (int)((dist + 26.f) / 52.f);
    if (meters >= 5) {
        if (meters < 21) {
            meters = 5 * ((meters + 2) / 5);
        } else if (meters < 101) {
            meters = 10 * ((meters + 5) / 10);
        } else {
            meters = 25 * ((meters + 12) / 25);
        }
    }

    return va("%d meters", meters);
}

str Player::GetBattleLanguageDistanceFeet(float dist) const
{
    int ft;

    ft = (int)((dist + 26.f) / 52.f);
    if (ft >= 11) {
        if (ft < 51) {
            ft = 5 * ((ft + 2) / 5);
        } else if (ft < 251) {
            ft = 10 * ((ft + 5) / 10);
        } else {
            ft = 25 * ((ft + 12) / 25);
        }
    }

    return va("%d feet", ft);
}

str Player::GetBattleLanguageTarget() const
{
    Vector  vStart, vEnd;
    Vector  vForward;
    trace_t trace;

    vStart = m_vViewPos;
    AngleVectors(m_vViewAng, vForward, NULL, NULL);

    vEnd = vStart + vForward * 10240;

    trace = G_Trace(
        vStart,
        vec_zero,
        vec_zero,
        vEnd,
        static_cast<const Entity *>(this),
        MASK_BATTLELANGUAGE,
        qfalse,
        "Player::GetBattleLanguageDistance"
    );

    if (!trace.ent) {
        return "something";
    }

    if (!trace.ent->entity || trace.ent->entity == world) {
        return "something";
    }

    if (trace.ent->entity->IsSubclassOfPlayer()) {
        Player *pPlayer = static_cast<Player *>(trace.ent->entity);
        return pPlayer->client->pers.netname;
    }

    if (trace.ent->entity->IsSubclassOfSentient()) {
        return "someone";
    }

    return "something";
}

str Player::TranslateBattleLanguageTokens(const char *string)
{
    str token;
    int type;

    if (!g_chat_expansions->integer) {
        return string;
    }

    if (!string) {
        return NULL;
    }

    if (string[0] != '$') {
        return string;
    }

    type = string[1];
    if (!type || string[2]) {
        return string;
    }

    switch (type) {
    case 'a':
        token = GetBattleLanguageTarget();
        break;
    case 'c':
        token = GetBattleLanguageCondition();
        break;
    case 'd':
        token = GetBattleLanguageDirection();
        break;
    case 'l':
        token = GetBattleLanguageLocation();
        break;
    case 'n':
        token = GetBattleLanguageLocalFolks();
        break;
    case 'r':
        token = GetBattleLanguageDistance();
        break;
    case 'w':
        token = GetBattleLanguageWeapon();
        break;
    default:
        return string;
    }

    return gi.LV_ConvertString(token);
}

void Player::EventIPrint(Event *ev)
{
    str      sString = ev->GetString(1);
    qboolean iBold   = qfalse;

    if (ev->NumArgs() > 1) {
        iBold = ev->GetInteger(2);
    }

    if (iBold) {
        gi.SendServerCommand(edict - g_entities, "print \"" HUD_MESSAGE_WHITE "%s\n\"", sString.c_str());
    } else {
        gi.SendServerCommand(edict - g_entities, "print \"" HUD_MESSAGE_YELLOW "%s\n\"", sString.c_str());
    }
}

void Player::SetViewangles(Event *ev)
{
    SetViewAngles(ev->GetVector(1));
}

void Player::GetViewangles(Event *ev)
{
    ev->AddVector(GetVAngles());
}

void Player::EventGetUseHeld(Event *ev)
{
    ev->AddInteger((buttons & BUTTON_USE) ? true : false);
}

void Player::EventGetFireHeld(Event *ev)
{
    ev->AddInteger(buttons & (BUTTON_ATTACKLEFT | BUTTON_ATTACKRIGHT) ? qtrue : qfalse);
}

void Player::EventGetPrimaryFireHeld(Event *ev)
{
    ev->AddInteger(buttons & BUTTON_ATTACKLEFT ? true : false);
}

void Player::EventGetSecondaryFireHeld(Event *ev)
{
    ev->AddInteger(buttons & BUTTON_ATTACKRIGHT ? true : false);
}

void Player::BeginTempSpectator(void)
{
    m_bTempSpectator = true;
    Spectator();
}

void Player::EndSpectator(void)
{
    m_bSpectator     = false;
    m_bTempSpectator = false;

    client->ps.pm_flags &= ~(PMF_SPECTATING | PMF_SPECTATE_FOLLOW);
}

void Player::EventGetReady(Event *ev)
{
    ev->AddInteger(m_bReady);
}

void Player::EventSetReady(Event *ev)
{
    if (m_bReady) {
        return;
    }

    m_bReady = true;
    gi.Printf("%s is ready\n", client->pers.netname);
}

void Player::EventSetNotReady(Event *ev)
{
    if (!m_bReady) {
        return;
    }

    m_bReady = false;
    gi.Printf("%s is not ready\n", client->pers.netname);
}

void Player::EventGetDMTeam(Event *ev)
{
    if (dm_team == TEAM_FREEFORALL) {
        ev->AddConstString(STRING_FREEFORALL);
    } else if (dm_team == TEAM_AXIS) {
        ev->AddConstString(STRING_AXIS);
    } else if (dm_team == TEAM_ALLIES) {
        ev->AddConstString(STRING_ALLIES);
    } else if (dm_team == TEAM_SPECTATOR) {
        ev->AddConstString(STRING_SPECTATOR);
    } else {
        ScriptError("dmteam is invalid in single player");
    }
}

void Player::EventGetNetName(Event *ev)
{
    ev->AddString(client->pers.netname);
}

void Player::EventSetViewModelAnim(Event *ev)
{
    str      anim;
    int      force_restart = 0;
    qboolean bfullanim     = 0;

    anim = ev->GetString(1);

    if (ev->NumArgs() > 1) {
        force_restart = ev->GetInteger(2);
    }

    if (ev->NumArgs() > 2) {
        bfullanim = ev->GetInteger(3);
    }

    ViewModelAnim(anim, force_restart, bfullanim);
}

void Player::FullHeal(Event *ev)
{
    if (IsDead()) {
        if (!ev->IsFromScript()) {
            HUDPrint("TESTING:  Cannot resurrect yourself with the fullheal.\n");
        }
    } else {
        if (!ev->IsFromScript()) {
            HUDPrint("TESTING:  You used the fullheal.\n");
        }

        health = max_health;
    }
}

void Player::RemoveFromVehiclesAndTurretsInternal(void)
{
    if (m_pVehicle) {
        Event *event;

        m_pVehicle->flags &= ~FL_GODMODE;

        event = new Event(EV_Use);
        event->AddEntity(this);
        m_pVehicle->ProcessEvent(event);
    } else if (m_pTurret) {
        m_pTurret->TurretUsed(this);
    }
}

void Player::RemoveFromVehiclesAndTurrets(void)
{
    Weapon *activeWeap = GetActiveWeapon(WEAPON_MAIN);
    if (activeWeap && activeWeap->IsCarryableTurret()) {
        CarryableTurret *pTurret = static_cast<CarryableTurret *>(activeWeap);
        pTurret->DropTurret(NULL);
    }

    if (!m_pVehicle && !m_pTurret) {
        return;
    }

    if (m_pVehicle && m_pVehicle->isLocked()) {
        m_pVehicle->UnLock();

        if (m_pTurret && m_pTurret->IsSubclassOfVehicleTurretGun()) {
            VehicleTurretGun *turret = (VehicleTurretGun *)m_pTurret.Pointer();

            if (turret->isLocked()) {
                turret->UnLock();
                RemoveFromVehiclesAndTurretsInternal();
                turret->Lock();
            } else {
                RemoveFromVehiclesAndTurretsInternal();
            }
        } else {
            RemoveFromVehiclesAndTurretsInternal();
        }

        // the vehicle might have been modified
        if (m_pVehicle) {
            m_pVehicle->Lock();
        }
    } else if (m_pTurret && m_pTurret->IsSubclassOfVehicleTurretGun()) {
        VehicleTurretGun *turret = (VehicleTurretGun *)m_pTurret.Pointer();

        if (turret->isLocked()) {
            turret->UnLock();
            RemoveFromVehiclesAndTurretsInternal();

            // the turret might have been modified
            if (m_pTurret) {
                turret->Lock();
            }
        } else {
            RemoveFromVehiclesAndTurretsInternal();
        }
    } else {
        RemoveFromVehiclesAndTurretsInternal();
    }
}

void Player::EventEnterIntermission(Event *ev)
{
    if (!level.intermissiontime) {
        return;
    }

    if (level.intermissiontype) {
        G_DisplayScores(this);

        if (level.intermissiontype == TRANS_MISSION_FAILED || IsDead()) {
            gi.cvar_set("g_success", "0");
            gi.cvar_set("g_failed", "1");
        } else {
            gi.cvar_set("g_success", "1");
            gi.cvar_set("g_failed", "0");
        }
    } else {
        G_HideScores(this);
    }
}

bool Player::BlocksAIMovement()
{
    return false;
}

void Player::EventSetPerferredWeapon(Event *ev)
{
    m_sPerferredWeaponOverride = ev->GetString(1);
}

void Player::SetMouthAngle(Event *ev)

{
    int    tag_num;
    float  angle_percent;
    Vector mouth_angles;

    angle_percent = ev->GetFloat(1);

    if (angle_percent < 0) {
        angle_percent = 0;
    }

    if (angle_percent > 1) {
        angle_percent = 1;
    }

    tag_num = gi.Tag_NumForName(edict->tiki, "tag_mouth");

    if (tag_num != -1) {
        SetControllerTag(MOUTH_TAG, tag_num);

        mouth_angles        = vec_zero;
        mouth_angles[PITCH] = max_mouth_angle * angle_percent;

        SetControllerAngles(MOUTH_TAG, mouth_angles);
    }
}

int Player::GetMoveResult(void)
{
    return moveresult;
}

qboolean Player::ViewModelAnim(str anim, qboolean force_restart, qboolean bFullAnim)
{
    Unregister(STRING_VIEWMODELANIM_DONE);

    if (client == NULL) {
        return true;
    }

    int            viewModelAnim;
    playerState_t *playerState = &client->ps;
    Weapon        *weapon;

    if (!anim.length()) {
        anim = "";
    }

    // Copy the item prefix and the anim name
    weapon = GetActiveWeapon(WEAPON_MAIN);

    if (!Q_stricmp(anim, "charge")) {
        viewModelAnim = VM_ANIM_CHARGE;
    } else if (!Q_stricmp(anim, "fire")) {
        viewModelAnim = VM_ANIM_FIRE;
    } else if (!Q_stricmp(anim, "fire_secondary")) {
        viewModelAnim = VM_ANIM_FIRE_SECONDARY;
    } else if (!Q_stricmp(anim, "rechamber")) {
        viewModelAnim = VM_ANIM_RECHAMBER;
    } else if (!Q_stricmp(anim, "reload")) {
        viewModelAnim = VM_ANIM_RELOAD;
    } else if (!Q_stricmp(anim, "reload_single")) {
        viewModelAnim = VM_ANIM_RELOAD_SINGLE;
    } else if (!Q_stricmp(anim, "reload_end")) {
        viewModelAnim = VM_ANIM_RELOAD_END;
    } else if (!Q_stricmp(anim, "pullout")) {
        viewModelAnim = VM_ANIM_PULLOUT;
    } else if (!Q_stricmp(anim, "putaway")) {
        viewModelAnim = VM_ANIM_PUTAWAY;
    } else if (!Q_stricmp(anim, "ladderstep")) {
        viewModelAnim = VM_ANIM_LADDERSTEP;
    } else {
        if (!Q_stricmp(anim, "idle")) {
            viewModelAnim = VM_ANIM_IDLE;
        } else if (!Q_stricmp(anim, "idle0")) {
            viewModelAnim = VM_ANIM_IDLE_0;
        } else if (!Q_stricmp(anim, "idle1")) {
            viewModelAnim = VM_ANIM_IDLE_1;
        } else if (!Q_stricmp(anim, "idle2")) {
            viewModelAnim = VM_ANIM_IDLE_2;
        } else {
            // Defaults to idle
            viewModelAnim = VM_ANIM_IDLE;
        }

        //
        // check the fire movement speed if the weapon has a max fire movement
        //
        if (weapon && weapon->m_fMaxFireMovement < 1) {
            float length;

            length = velocity.length();
            if (length / sv_runspeed->value > ((weapon->m_fMaxFireMovement * weapon->m_fMovementSpeed) + 0.1f)) {
                // Set the view model animation to disabled
                viewModelAnim = VM_ANIM_DISABLED;
            }
        }
    }

    if (!weapon) {
        weapon = newActiveWeapon.weapon;
    }

    if (weapon) {
        m_sVMAcurrent = GetItemPrefix(weapon->getName()) + str("_") + anim;
    } else {
        m_sVMAcurrent = "unarmed_" + anim;
    }

    m_sVMcurrent = anim;

    if (viewModelAnim != playerState->iViewModelAnim || force_restart) {
        playerState->iViewModelAnimChanged = (playerState->iViewModelAnimChanged + 1) & 3;
    }

    playerState->iViewModelAnim = viewModelAnim;

    if (m_fpsTiki && gi.Anim_NumForName(m_fpsTiki, m_sVMAcurrent) < 0) {
        //gi.DPrintf("WARNING: Invalid view model anim \"%s\"\n", m_sVMAcurrent.c_str());
    }

    animDoneVM = false;

    m_fVMAtime = 0;

    return true;
}

void Player::FindAlias(str& output, str name, AliasListNode_t **node)
{
    const char *alias = gi.Alias_FindRandom(edict->tiki, name, node);

    if (alias == NULL) {
        alias = gi.GlobalAlias_FindRandom(name, node);
    }

    if (alias != NULL) {
        output = alias;
    }
}

bool Player::HasVotedYes() const
{
    return voted;
}

bool Player::HasVotedNo() const
{
    return !voted;
}

void Player::TickSprint()
{
    float timeHeld;

    if (last_ucmd.buttons & BUTTON_RUN && last_ucmd.forwardmove) {
        timeHeld = 0;

        if (!m_fLastSprintTime) {
            m_fLastSprintTime = level.time;
        }
    } else {
        timeHeld          = 0;
        m_fLastSprintTime = 0;
    }

    if (last_ucmd.rightmove) {
        m_fLastSprintTime = timeHeld;
    }
    if (last_ucmd.upmove) {
        m_fLastSprintTime = timeHeld;
    }
}

float Player::GetRunSpeed() const
{
    float sprintTime;
    float sprintMult;

    sprintTime = sv_sprinttime->value;
    sprintMult = sv_sprintmult->value;
    if (g_gametype->integer != GT_SINGLE_PLAYER) {
        sprintTime = sv_sprinttime_dm->value;
        sprintMult = sv_sprintmult_dm->value;
    }

    if (sv_sprinton->integer == 1 && m_fLastSprintTime && (level.time - m_fLastSprintTime) > sprintTime) {
        return sv_runspeed->value * sprintMult;
    } else {
        return sv_runspeed->value;
    }
}

void Player::FireWeapon(int number, firemode_t mode)
{
    if (m_pVehicle || m_pTurret) {
        return;
    }

    Sentient::FireWeapon(number, mode);

    if (g_gametype->integer != GT_SINGLE_PLAYER) {
        //
        // Make sure to remove the player's invulnerability
        //
        CancelInvulnerable();
    }
}

void Player::SetInvulnerable()
{
    if (IsInvulnerable()) {
        return;
    }

    if (!sv_invulnerabletime->integer) {
        return;
    }

    if (gi.Cvar_Get("g_invulnoverride", "0", 0)->integer == 1) {
        return;
    }

    if (IsDead()) {
        return;
    }

    if (IsSpectator() || GetTeam() == TEAM_SPECTATOR) {
        return;
    }

    //
    // The player can now be invulnerable
    //
    takedamage                   = DAMAGE_NO;
    m_iInvulnerableTimeRemaining = sv_invulnerabletime->integer;
    m_fLastInvulnerableTime      = level.time;
    m_fInvulnerableTimeElapsed   = level.time;

    TickInvulnerable();
}

void Player::TickInvulnerable()
{
    if (m_iInvulnerableTimeRemaining >= 0 && level.time >= m_fInvulnerableTimeElapsed) {
        if (m_iInvulnerableTimeRemaining) {
            m_fInvulnerableTimeElapsed = m_fInvulnerableTimeElapsed + 1.f;
        } else {
            SetVulnerable();
            m_fInvulnerableTimeElapsed = 0;
        }

        m_iInvulnerableTimeRemaining--;
    }
}

void Player::SetVulnerable()
{
    if (IsInvulnerable()) {
        takedamage              = DAMAGE_AIM;
        m_fLastInvulnerableTime = 0;
    }
}

bool Player::IsInvulnerable()
{
    return m_fLastInvulnerableTime != 0;
}

void Player::CancelInvulnerable()
{
    if (IsInvulnerable()) {
        SetVulnerable();
        m_iInvulnerableTimeRemaining = -1;
        gi.centerprintf(edict, " ");
    }
}

void Player::InitInvulnerable()
{
    m_fLastInvulnerableTime      = 0;
    m_iInvulnerableTimeRemaining = -1;
}

void Player::TickTeamSpawn()
{
    int timeLeft;

    if (!IsSpectator() && !IsDead() || (GetTeam() == TEAM_SPECTATOR || !client->pers.dm_primary[0])) {
        return;
    }

    timeLeft = dmManager.GetTeamSpawnTimeLeft();
    if (timeLeft == -1) {
        // Can spawn
        m_fSpawnTimeLeft = 0;
        return;
    }

    if (timeLeft == m_fSpawnTimeLeft) {
        // Still waiting
        return;
    }

    if (m_bShouldRespawn) {
        // The player can spawn
        m_fSpawnTimeLeft = 0;
        return;
    }

    m_fSpawnTimeLeft = timeLeft;
    if (timeLeft) {
        if (AllowTeamRespawn()) {
            const char *string;

            if (timeLeft == 1) {
                string = va("Next respawn in 1 second");
            } else {
                string = va("Next respawn in %d seconds", timeLeft);
            }

            gi.centerprintf(edict, string);
        }
    } else if (m_bWaitingForRespawn && AllowTeamRespawn()) {
        m_bWaitingForRespawn = false;
        m_bDeathSpectator    = false;
        EndSpectator();

        PostEvent(EV_Player_Respawn, 0);
    } else {
        m_bShouldRespawn = true;
    }
}

bool Player::ShouldForceSpectatorOnDeath() const
{
    return dmManager.GetTeamSpawnTimeLeft() > 0;
}

bool Player::HasVehicle() const
{
    return m_pVehicle != NULL;
}

void Player::setContentsSolid()
{
    edict->r.contents = CONTENTS_BODY;
}

void Player::UserSelectWeapon(bool bWait)
{
    nationality_t nationality;
    char          buf[256];

    if (g_protocol < PROTOCOL_MOHTA_MIN) {
        //
        // nationality was first introduced in 2.0
        //
        if (bWait) {
            gi.SendServerCommand(edict - g_entities, "stufftext \"wait 250;pushmenu_weaponselect\"");
        } else {
            gi.SendServerCommand(edict - g_entities, "stufftext \"pushmenu_weaponselect\"");
        }
        return;
    }

    if (GetTeam() == TEAM_AXIS) {
        nationality = GetPlayerTeamType(client->pers.dm_playergermanmodel);
    } else {
        nationality = GetPlayerTeamType(client->pers.dm_playermodel);
    }

    if (bWait) {
        strcpy(buf, "stufftext \"wait 250;pushmenu ");
    } else {
        strcpy(buf, "stufftext \"pushmenu ");
    }

    if (dmflags->integer & DF_WEAPON_NO_RIFLE && dmflags->integer & DF_WEAPON_NO_SNIPER
        && dmflags->integer & DF_WEAPON_NO_SMG && dmflags->integer & DF_WEAPON_NO_MG
        && dmflags->integer & DF_WEAPON_NO_ROCKET && dmflags->integer & DF_WEAPON_NO_SHOTGUN
        && dmflags->integer & DF_WEAPON_NO_LANDMINE && !QueryLandminesAllowed()) {
        gi.cvar_set("dmflags", va("%i", dmflags->integer & ~DF_WEAPON_NO_RIFLE));
        Com_Printf("No valid weapons -- re-allowing the rifle\n");
        strcpy(client->pers.dm_primary, "rifle");
    }

    switch (nationality) {
    case NA_BRITISH:
        strcat(buf, "SelectPrimaryWeapon_british\"");
        break;
    case NA_RUSSIAN:
        strcat(buf, "SelectPrimaryWeapon_russian\"");
        break;
    case NA_GERMAN:
        strcat(buf, "SelectPrimaryWeapon_german\"");
        break;
    case NA_ITALIAN:
        strcat(buf, "SelectPrimaryWeapon_italian\"");
        break;
    default:
        strcat(buf, "SelectPrimaryWeapon\"");
        break;
    }

    gi.SendServerCommand(edict - g_entities, buf);
}

void Player::PickWeaponEvent(Event *ev)
{
    if (g_gametype->integer == GT_SINGLE_PLAYER) {
        return;
    }

    UserSelectWeapon(false);
}

bool Player::AllowTeamRespawn() const
{
    if (m_bSpectator && !m_bDeathSpectator
        && (!dmManager.AllowTeamRespawn(TEAM_ALLIES) || !dmManager.AllowTeamRespawn(TEAM_AXIS))) {
        return false;
    }

    if (GetTeam() > TEAM_AXIS || GetTeam() < TEAM_ALLIES) {
        return false;
    }

    return dmManager.AllowTeamRespawn(GetTeam());
}

void Player::EventUseWeaponClass(Event *ev)
{
    if (m_pTurret || level.playerfrozen) {
        return;
    }

    Sentient::EventUseWeaponClass(ev);
}

void Player::EventAddKills(Event *ev)
{
    SafePtr<DM_Team> pTeam = GetDM_Team();
    if (pTeam) {
        // Add kills to the team
        pTeam->AddKills(this, ev->GetInteger(1));
    }
}

bool Player::CanKnockback(float minHealth) const
{
    if (m_pTurret || m_pVehicle) {
        return minHealth >= health;
    } else {
        return 1;
    }
}

void Player::EventKillAxis(Event *ev)
{
    float radius = 0;

    if (ev->NumArgs() >= 1) {
        radius = ev->GetFloat(1);
    }

    for (Sentient *pSent = level.m_HeadSentient[TEAM_GERMAN]; pSent; pSent = pSent->m_NextSentient) {
        if (radius > 0) {
            Vector delta = pSent->origin - origin;
            if (radius < delta.length()) {
                continue;
            }
        }

        pSent->Damage(this, this, pSent->max_health + 25, origin, vec_zero, vec_zero, 0, 0, MOD_NONE);
    }
}

void Player::EventGetTurret(Event *ev)
{
    ev->AddEntity(m_pTurret);
}

void Player::EventGetVehicle(Event *ev)
{
    ev->AddEntity(m_pVehicle);
}

bool Player::IsReady(void) const
{
    return m_bReady && !IsDead();
}

void Player::Spawned(void)
{
    Event *ev = new Event;

    ev->AddEntity(this);
    scriptedEvents[SE_SPAWN].Trigger(ev);
}

// reborn stuff

void Player::AddDeaths(Event *ev)
{
    AddDeaths(ev->GetInteger(1));
}

void Player::AdminRights(Event *ev)
{
    // FIXME: Admin manager ?
    ev->AddInteger(0);
}

void Player::BindWeap(Event *ev)
{
    // FIXME: TODO
}

void Player::CanSwitchTeams(Event *ev)
{
    qboolean bAllow = ev->GetBoolean(1);

    disable_team_change = !bAllow;
    if (ev->NumArgs() > 1) {
        qboolean bAllow2 = ev->GetBoolean(2);

        disable_spectate = !bAllow2;
    }
}

void Player::ClearCommand(Event *ev)
{
    str command;
    int clientNum = G_GetClientNumber(this);

    if (ev->NumArgs() > 0) {
        command = ev->GetString(1);
    }

    m_lastcommand = "";
}

void Player::Dive(Event *ev)
{
    float height, airborne_duration, speed;

    Vector forwardvector = orientation[0];

    height = ev->GetFloat(1);

    if (ev->NumArgs() < 2 || ev->IsNilAt(2)) {
        airborne_duration = 1;
    } else {
        airborne_duration = ev->GetFloat(2);
    }

    speed = height * airborne_duration;

    velocity[0] += height * forwardvector[0] * (speed / 16);
    velocity[1] += height * forwardvector[1] * (speed / 16);
    velocity[2] += height * speed / 6.80f;
}

void Player::EventEarthquake(Event *ev)
{
    float    duration    = ev->GetFloat(1);
    float    magnitude   = ev->GetFloat(2);
    qboolean no_rampup   = ev->GetBoolean(3);
    qboolean no_rampdown = ev->GetBoolean(4);

    // full realistic, smooth earthquake
    if (ev->NumArgs() > 4) {
        Vector location = ev->GetVector(5);
        float  radius   = 1.0f;

        if (ev->NumArgs() > 5) {
            radius = ev->GetFloat(6);
        }

        gi.SendServerCommand(
            edict - g_entities,
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
        gi.SendServerCommand(edict - g_entities, "eq %f %f %d %d", duration, magnitude, no_rampup, no_rampdown);
    }
}

void Player::EventSetTeam(Event *ev)
{
    str        team_name;
    teamtype_t teamType;

    team_name = ev->GetString(1);
    if (!team_name.length()) {
        ScriptError("Invalid team name !");
        return;
    }

    if (Q_stricmp(team_name, "none") == 0) {
        teamType = TEAM_NONE;
    } else if (Q_stricmp(team_name, "spectator") == 0) {
        teamType = TEAM_SPECTATOR;
    } else if (Q_stricmp(team_name, "freeforall") == 0) {
        teamType = TEAM_FREEFORALL;
    } else if (Q_stricmp(team_name, "allies") == 0) {
        teamType = TEAM_ALLIES;
    } else if (Q_stricmp(team_name, "axis") == 0) {
        teamType = TEAM_AXIS;
    } else {
        ScriptError("Unknown team name \"%s\"\n", team_name.c_str());
        return;
    }

    SetTeam(teamType);

    gi.Printf("Player::SetTeam : Player is now on team \"%s\"\n", team_name.c_str());
}

void Player::EventGetViewModelAnim(Event *ev)
{
    ev->AddString(m_sVMcurrent);
}

void Player::EventGetViewModelAnimFinished(Event *ev)
{
    ev->AddInteger(animDoneVM);
}

void Player::EventGetViewModelAnimValid(Event *ev)
{
    str  anim_name = ev->GetString(1);
    str  fullanim;
    bool bFullAnim = false;

    if (ev->NumArgs() > 1) {
        bFullAnim = ev->GetBoolean(2);
    }

    if (!bFullAnim) {
        // Copy the item prefix and the anim name
        Item *item = GetActiveWeapon(WEAPON_MAIN);

        if (!item) {
            item = (Item *)newActiveWeapon.weapon.Pointer();
        }

        if (item) {
            fullanim = GetItemPrefix(item->getName()) + str("_") + anim_name;
        } else {
            fullanim = "unarmed_" + anim_name;
        }
    } else {
        fullanim = anim_name;
    }

    if (!m_fpsTiki || gi.Anim_NumForName(m_fpsTiki, fullanim.c_str()) < 0) {
        ev->AddInteger(0);
    } else {
        ev->AddInteger(1);
    }
}

void Player::FreezeControls(Event *ev)
{
    m_bFrozen = ev->GetBoolean(1);
}

void Player::GetConnState(Event *ev)
{
    ScriptDeprecated("Player::GetConnState");
}

void Player::GetDamageMultiplier(Event *ev)
{
    ev->AddFloat(damage_multiplier);
}

void Player::GetDeaths(Event *ev)
{
    ev->AddInteger(num_deaths);
}

void Player::GetKillHandler(Event *ev)
{
    if (m_pKilledEvent) {
        ev->AddValue(m_pKilledEvent->GetValue(1));
    } else {
        ev->AddNil();
    }
}

void Player::GetKills(Event *ev)
{
    ev->AddInteger(num_kills);
}

void Player::GetMoveSpeedScale(Event *ev)
{
    ev->AddFloat(speed_multiplier[0]);
}

void Player::GetLegsState(Event *ev)
{
    const char *name;

    if (currentState_Legs != NULL) {
        name = currentState_Legs->getName();
    } else {
        name = "none";
    }

    ev->AddString(name);
}

void Player::GetStateFile(Event *ev)
{
    int clientNum = G_GetClientNumber(this);

    if (m_sStateFile.length()) {
        ev->AddString(m_sStateFile);
    } else {
        ev->AddString(g_statefile->string);
    }
}

void Player::GetTorsoState(Event *ev)
{
    const char *name;

    if (currentState_Torso != NULL) {
        name = currentState_Torso->getName();
    } else {
        name = "none";
    }

    ev->AddString(name);
}

void Player::HideEntity(Event *ev)
{
    // FIXME: todo
}

void Player::ShowEntity(Event *ev)
{
    // FIXME: REDO
}

void Player::Inventory(Event *ev)
{
    Entity         *ent = NULL;
    ScriptVariable *ref = new ScriptVariable, *array = new ScriptVariable;
    int             i = 0;

    ref->setRefValue(array);

    for (i = 0; i < inventory.NumObjects(); i++) {
        ent = G_GetEntity(inventory[i]);

        if (ent == NULL) {
            continue;
        }

        ScriptVariable *index = new ScriptVariable, *value = new ScriptVariable;

        index->setIntValue(i + 1);
        value->setListenerValue((Listener *)ent);

        ref->setArrayAt(*index, *value);
    }

    ev->AddValue(*array);
}

void Player::InventorySet(Event *ev)
{
    ScriptVariable  array;
    ScriptVariable *value;
    Entity         *ent;
    int             arraysize;

    if (ev->IsNilAt(1)) {
        // Just clear the inventory
        inventory.ClearObjectList();
        return;
    }

    // Retrieve the array
    array = ev->GetValue(1);

    // Cast the array
    array.CastConstArrayValue();
    arraysize = array.arraysize();

    // Detach all active weapons and free the inventory

    if (inventory.NumObjects() > 0) {
        inventory.FreeObjectList();
    }

    if (arraysize < 1) {
        return;
    }

    // Allocate an inventory
    for (int i = 1; i <= arraysize; i++) {
        // Retrieve the value from the array
        value = array[i];

        // Get the entity from the value
        ent = (Entity *)value->entityValue();

        if (ent == NULL || !ent->edict->inuse) {
            continue;
        }

        // Add the entity to the inventory
        inventory.AddObject(ent->entnum);
    }

    // Clear the variable
    array.Clear();
}

void Player::IsAdmin(Event *ev)
{
    // FIXME: Admin manager ?
    ev->AddInteger(0);
}

void Player::LeanLeftHeld(Event *ev)
{
    Player *player     = NULL;
    int     buttonheld = 0;

    player = (Player *)this;

    buttonheld = !!(player->buttons & BUTTON_LEAN_LEFT);

    ev->AddInteger(buttonheld);
}

void Player::LeanRightHeld(Event *ev)
{
    Player *player     = NULL;
    int     buttonheld = 0;

    player = (Player *)this;

    buttonheld = !!(player->buttons & BUTTON_LEAN_RIGHT);

    ev->AddInteger(buttonheld);
}

void Player::PlayLocalSound(Event *ev)
{
    str      soundName = ev->GetString(1);
    qboolean loop      = false;
    float    time;

    if (ev->NumArgs() > 1) {
        loop = ev->GetBoolean(2);
    }

    if (ev->NumArgs() > 2) {
        time = ev->GetFloat(3);
    } else {
        time = 0.0f;
    }

    AliasListNode_t *alias = NULL;

    const char *found = gi.GlobalAlias_FindRandom(soundName, &alias);

    if (found == NULL) {
        gi.DPrintf("ERROR: Player::PlayLocalSound: %s needs to be aliased - Please fix.\n", soundName.c_str());
        return;
    }

    // FIXME...
    /*
        gi.MSG_SetClient( client->ps.clientNum );

        gi.MSG_StartCGM( CGM_PLAYLOCALSOUND );
        gi.MSG_WriteString( found );
        gi.MSG_WriteBits( !!loop, 1 );
        gi.MSG_WriteFloat( time );
        gi.MSG_WriteFloat( alias->volume );
        gi.MSG_EndCGM();

        return;
    */

    if (loop) {
        edict->s.loopSound        = gi.soundindex(found, alias->streamed);
        edict->s.loopSoundVolume  = 1.0f;
        edict->s.loopSoundMinDist = 0;
        edict->s.loopSoundMaxDist = 96;
        edict->s.loopSoundPitch   = 1.0f;
        edict->s.loopSoundFlags   = 1; // local sound
    } else {
        gi.Sound(&edict->s.origin, entnum, CHAN_LOCAL, found, 1.0f, 0, 1.0f, 96, alias->streamed);
    }
}

void Player::RunHeld(Event *ev)
{
    Player *player     = NULL;
    int     buttonheld = 0;

    player = (Player *)this;

    buttonheld = !!(player->buttons & BUTTON_RUN);

    ev->AddInteger(buttonheld);
}

void Player::SecFireHeld(Event *ev)
{
    Player *player     = NULL;
    int     buttonheld = 0;

    player = (Player *)this;

    buttonheld = !!(player->buttons & BUTTON_ATTACKRIGHT);

    ev->AddInteger(buttonheld);
}

void Player::SetAnimSpeed(Event *ev)
{
    float   speed;
    Player *player = (Player *)this;

    speed = ev->GetFloat(1);

    if (speed < 0.0f) {
        speed = 0.0f;
    }

    //player->baseSentient.baseAnimate.baseEntity.client->customanimspeed = speed;
}

void Player::SetClientFlag(Event *ev)
{
    str name = ev->GetString(1);

    gi.SendServerCommand(client->ps.clientNum, "cf %s", name.c_str());
}

void Player::SetEntityShader(Event *ev)
{
    Entity  *entity     = ev->GetEntity(1);
    str      shadername = ev->GetString(2);
    qboolean fReset     = false;

    if (entity == NULL) {
        ScriptError("Invalid entity !");
    }

    if (!shadername.length()) {
        shadername = "default";
        fReset     = true;
    }

    gi.SendServerCommand(edict - g_entities, "setshader %d %s %d", entity->entnum, shadername.c_str(), fReset);
}

void Player::SetKillHandler(Event *ev)
{
    if (ev->IsNilAt(1) || (ev->IsStringAt(1) && !ev->GetString(1).icmp("none"))) {
        if (m_pKilledEvent != NULL) {
            delete m_pKilledEvent;
            m_pKilledEvent = NULL;
        }
    } else {
        m_pKilledEvent = new Event(EV_Listener_ExecuteScript);
        m_pKilledEvent->AddValue(ev->GetValue(1));
    }
}

void Player::SetLocalSoundRate(Event *ev)
{
    str   name = ev->GetString(1);
    float rate = ev->GetFloat(2);
    float time;

    if (ev->NumArgs() > 2) {
        time = ev->GetFloat(3);
    } else {
        time = 0.0f;
    }

    AliasListNode_t *alias = NULL;
    const char      *found = gi.GlobalAlias_FindRandom(name, &alias);

    if (found == NULL) {
        gi.DPrintf("ERROR: Player::SetLocalSoundRate: %s needs to be aliased - Please fix.\n", name.c_str());
        return;
    }

    gi.MSG_SetClient(client->ps.clientNum);

    // FIXME...
    /*
    gi.MSG_StartCGM( CGM_SETLOCALSOUNDRATE );
        gi.MSG_WriteString( found );
        gi.MSG_WriteFloat( rate );
        gi.MSG_WriteFloat( time );
    gi.MSG_EndCGM();
    */
}

void Player::SetSpeed(Event *ev)
{
    float   speed;
    Player *player    = (Player *)this;
    int     clientNum = G_GetClientNumber(this);
    int     index     = 0;

    speed = ev->GetFloat(1);

    if (speed < 0.0f) {
        speed = 0.0f;
    }

    if (ev->NumArgs() > 1) {
        index = ev->GetInteger(2);

        /* Reserve a space for moveSpeedScale */
        if (index < 1 || index > MAX_SPEED_MULTIPLIERS) {
            gi.Printf("Player::SetSpeed : invalid index %d. Index must be between 1-%d\n", index, speed_multiplier);

            return;
        }
    }

    speed_multiplier[index] = speed;
}

void Player::SetStateFile(Event *ev)
{
    int      clientNum = G_GetClientNumber(this);
    qboolean bRemove   = false;
    str      string;

    if (ev->NumArgs() <= 0) {
        bRemove = true;
    } else {
        string = ev->GetString(1);

        if (!string) {
            bRemove = true;
        }
    }

    if (bRemove) {
        m_sStateFile = "";
    } else {
        m_sStateFile = string;
    }
}

void Player::SetVMASpeed(Event *ev)
{
    str   name  = ev->GetString(1);
    float speed = ev->GetFloat(2);

    if (!client || !level.specialgame) {
        return;
    }

    vma_t *vma = &vmalist[name];

    if (speed < 0.0f) {
        speed = 0.0f;
    }

    vma->name  = name;
    vma->speed = speed;

    // FIXME...
    /*
    gi.MSG_SetClient( edict - g_entities );

    gi.MSG_StartCGM( CGM_SETVMASPEED );
        gi.MSG_WriteString( name );
        gi.MSG_WriteFloat( speed );
    gi.MSG_EndCGM();
    */
}

void Player::StopLocalSound(Event *ev)
{
    str   soundName = ev->GetString(1);
    float time;

    if (ev->NumArgs() > 1) {
        time = ev->GetFloat(2);
    } else {
        time = 0.0f;
    }

    AliasListNode_t *alias = NULL;
    const char      *found = gi.GlobalAlias_FindRandom(soundName, &alias);

    if (found == NULL) {
        gi.DPrintf("ERROR: Player::StopLocalSound: %s needs to be aliased - Please fix.\n", soundName.c_str());
        return;
    }

    edict->s.loopSound = 0;
    gi.StopSound(entnum, CHAN_LOCAL);
}

void Player::Userinfo(Event *ev)
{
    if (!client) {
        ScriptError("Entity is probably not of player type - userinfo\n");
        return;
    }

    ev->AddString(client->pers.userinfo);
}

void Player::VisionGetNaked(Event *ev)
{
    // return the global vision
    if (!m_sVision.length()) {
        ev->AddString(vision_current);
    } else {
        ev->AddString(m_sVision);
    }
}

void Player::VisionSetBlur(Event *ev)
{
    float blur_level = ev->GetFloat(1);
    float fade_time;

    if (ev->NumArgs() > 1) {
        fade_time = ev->GetFloat(2);
    } else {
        fade_time = 0.0f;
    }

    gi.SendServerCommand(edict - g_entities, "vsb %f %f", blur_level, fade_time);
}

void Player::VisionSetNaked(Event *ev)
{
    str   vision = ev->GetString(1);
    float fade_time;
    float phase;

    if (ev->NumArgs() > 1) {
        fade_time = ev->GetFloat(2);
    } else {
        fade_time = 0.0f;
    }

    if (ev->NumArgs() > 2) {
        phase = ev->GetFloat(3);
    } else {
        phase = 0.0f;
    }

    if (!vision.length()) {
        vision = vision_current;
    }

    if (vision.length() >= MAX_STRING_TOKENS) {
        ScriptError("vision_name exceeds the maximum vision name limit (256) !\n");
    }

    m_sVision = vision;

    gi.SendServerCommand(edict - g_entities, "vsn %s %f %f", vision.c_str(), fade_time, phase);
}

int Player::GetNumKills(void) const
{
    return num_kills;
}

int Player::GetNumDeaths(void) const
{
    return num_deaths;
}

void Player::AddKills(int num)
{
    num_kills += num;

    if (g_gametype->integer >= GT_TEAM_ROUNDS) {
        num_deaths += num;
    }
}

void Player::AddDeaths(int num)
{
    num_deaths += num;
}

qboolean Player::canUse()
{
    int touch[MAX_GENTITIES];
    int num = getUseableEntities(touch, MAX_GENTITIES);

    return num ? true : false;
}

qboolean Player::canUse(Entity *entity, bool requiresLookAt)
{
    gentity_t *hit;
    int        touch[MAX_GENTITIES];
    int        num;
    int        i;

    num = getUseableEntities(touch, MAX_GENTITIES, requiresLookAt);

    for (i = 0; i < num; i++) {
        hit = &g_entities[touch[i]];

        if (!hit->inuse || hit->entity == NULL) {
            continue;
        }

        if (hit->entity == entity) {
            return true;
        }
    }

    return false;
}

int Player::getUseableEntities(int *touch, int maxcount, bool requiresLookAt)
{
    Vector  end;
    Vector  start;
    trace_t trace;
    Vector  offset;
    Vector  max;
    Vector  min;

    if ((g_gametype->integer != GT_SINGLE_PLAYER && IsSpectator()) || IsDead()) {
        return 0;
    }

    if (m_pTurret) {
        *touch = m_pTurret->entnum;
        return 1;
    }

    if (m_pTurret) {
        return 0;
    }

    AngleVectors(client->ps.viewangles, offset, NULL, NULL);

    start = origin;
    start.z += client->ps.viewheight;

    if (requiresLookAt) {
        min = Vector(-4.f, -4.f, -4.f);
        max = Vector(4.f, 4.f, 4.f);

        end[0] = start[0] + (offset[0] * 64.f);
        end[1] = start[1] + (offset[1] * 64.f);

        if (v_angle[0] <= 0.0f) {
            end[2] = start[2] + (offset[2] * 40.f);
        } else {
            end[2] = start[2] + (offset[2] * 88.f);
        }

        trace = G_Trace(start, min, max, end, this, MASK_SOLID, false, "Player::getUseableEntity");

        offset = trace.endpos;

        min = offset - Vector(16.f, 16.f, 16.f);
        max = offset + Vector(16.f, 16.f, 16.f);
    } else {
        min = start - Vector(31.f, 31.f, 31.f);
        max = start + Vector(31.f, 31.f, 31.f);
    }

    return gi.AreaEntities(min, max, touch, maxcount);
}

void Player::Postthink(void)
{
    if (bindmaster) {
        SetViewAngles(GetViewAngles() + Vector(0, bindmaster->avelocity[YAW] * level.frametime, 0));
    }
}
