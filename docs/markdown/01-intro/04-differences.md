# List of differences

A lot of bugs and exploits from MOH:AA were fixed in OpenMoHAA (BOF exploit, grenade crash bug, command overflow...) in the engine.

This document lists all differences in the engine between versions of the original game and openmohaa.

- [Original Game](#the-original-game)
    - [Spearhead v2.0](#spearhead-v20)
    - [Spearhead v2.1](#spearhead-v21)
    - [Spearhead v2.11](#spearhead-v211)
    - [Spearhead v2.15](#spearhead-v215)
    - [Breakthrough v2.30](#breakthrough-v230)
    - [Breakthrough v2.40](#breakthrough-v240)
- [OpenMoHAA](#openmohaa)

## Summary

- SDL2 backend
- OpenAL sound support (better sound quality)
- Full 64-bit support
- Cross-platform support
- Multiuser support on OS (On Windows, user game data is stored in "%APPDATA%\openmohaa")
- Many bug fixes and additions from ioquake3
- Automatic scaling of UI elements on high resolutions like 4K

Overall, better compatibility on modern systems and bugfixes.

## The original game

### Spearhead v2.0

#### Added

##### General

- Added smaller lightmap support (`_sml` bsp files)
- Added network profiling support
- Automatically add idle anims for TIKI without animations

##### Client

- Added `stoploopsound` client command
- Added support for real death messages
- Added `notagaxis` effect
- Added vote system
- Added `tmvolume` to tweak the music volume

##### Game/Server

- Actors now check for obfuscation (smoke sprites)
- Added Actor script functions: `animscript_attached`, `nonvislevel`, `suppresschance`, `ignorebadplaces`, `findenemy`, `enemyswitchdisable`, `enemyswitchenable`, `runanimrate`,
- Added bad places support
- Added damage and run sounds for vehicles
- Added Entity script functions: `canseenoents`, `mins`, `maxs`
- Added Level `badplace`, `removebadplace`, `ignoreclock`, 
- Added navigation low wall arc support
- Added objective locations per team
- Added portable turret
- Added `reloadmap` command
- Added relative yaw for ScriptSlave
- Added script functions `dprintln`, `debugline`, `playmovie`, `debug_int3`
- Added suppression anim for actors
- Added support for animation movement for vehicles
- Added Tug-of-War game mode
- Added warmup feature for vehicle turrets
- Allow disabling axis or allies respawning individually
- Better vehicle turret support
- Clock can now be ignored by script in multiplayer
- Health now have a targetname: `Health`
- Set the targetname for vehicles turret. Like `vhicletargetname_turret_0`

###### New classes

- FencePost
- Objective
- ProjectileTarget
- ProjectileGenerator
- ThrobbingBox
- TOWObjective
- TriggerNoDamage

###### Players

- Added heal rate
- Added invulnerability support
- Added pain animations
- Added portable turret animations
- Added sprint support
- Added team spawn interval
- Allow dropping the weapon on single-player when realism is enabled
- Draw artillery icon above players head when they're using statweapons
- Draw menu icon above players head when they're in menu
- Draw talk icon above players head after they sent a message

###### Weapon

- Added additional start ammo
- Added maximum fire movement (bazooka)
- Added overcook for grenades
- Added zoom movement speed
- Bullets can now traverse through wood/metal
- Projectiles now have a targetname: `Projectile`
- Projectiles are deleted if the owner's team changed

##### Networking

- String scrambling

##### Rendering

- Farclip
- Farplane bias
- `ifCvar` and `ifCvarnot` for shaders
- Sky portal
- Terrain rendering can now be switched off from script

##### UI

- Added fade sequence
- Added ignored files support for file picker
- Added support for scalable widget from variable
- Added UI Radar support
- Added `playerstatalpha`, `statbar_shader_marker`

#### Changed

##### General

- All files in 'ubersound' folder are parsed
- Don't localize strings server-side

##### Client

- Don't refresh the server list when browsing internet servers
- Slightly less leaning angle

##### Game/Server

- Actors won't fire if a teammate is in sight
- Run speed is 287 by default
- Slightly decrease the animation rate for non-german actors
- Unlimited ammo was modified so bullets still use clips

###### Player

- Don't show the damage in god mode
- Fall damage remove a percentage of health rather than a raw value
- Player get teleported back to (0,0,0) when out of world bounds in noclip mode
- Player can't use while holding attack buttons
- Player can't lean while moving unless enabled by `dmflags`
- Player fall when on another sentient's head
- Player torso state get reset to STAND when physics is disabled
- Removed manon voice
- Spectators can no longer vote
- Spectators can navigate to next/prev players by jumping/crouching, and use to stop spectating a player
- The torso/legs state can't be NULL anymore

#### Fixed

##### General

- Crash fixes related to TIKI animations
- `FS_ReadFile` should no longer crash with empty name

##### Game/Server

- Allow different firedelay for turrets
- `drawhud` now works on all clients
- Disable `sv_floodProtect` by default
- Fixed game crashing connecting clients in a specific case
- Fixed crash when charging a weapon and the weapon disappears
- Fixed `setaimtarget` crash when entity is NULL
- Fixed buggy path delta
- Fixed players head rotating like monsters
- Remove respawning players from vehicles/turrets/ladders
- Set the entity as enemy in `setaimtarget`

### Spearhead v2.1

#### Additions

- Added tunak easter eggs

#### Fixed

##### Game/Server

- Prevent players from messing with developer commands

#### Changed

##### Game/Server

- Let clients choose their primary weapon on map change

### Spearhead v2.11

#### Fixed

##### Game/Server

- Clean up vehicles/turrets when player disconnects
- Prevent dropping the binoculars
- When sprint is disabled, revert `sv_runspeed` to 250

### Spearhead v2.15

#### Fixed

##### Game/Server

- Allow `vstr` in `nextmap`
- Prevent spectators, team spectators and dead players from voting

### Breakthrough v2.30

This game is a fork of Spearhead v2.15, so it has all fixes from Spearhead v2.15.

#### Added

##### Client

- Fixed a rare crash when hiding the scoreboard menu and it doesn't exist

##### Game/Server

- Added `bitset` command
- Added `reloadmap` server command
- Added network optimization support using `g_netoptimize`, which won't send clients that can't see eaechother
- Added Actor script functions: `curiousoff`, `curiouson`, `nationality`, `prealarmthread`, `writestats`
- Added AISpawnPoint class
- Added Door script function: `isopen`
- Added landmarks support (`InfoLandmark`)
- Added landmine support
- Added level script function: `nodropweapons`
- Added liberation game mode
- Added nationality support for actors and players
- Added PathDrive support for vehicles
- Added Player script functions:
    - `armwithweapons`
    - `getcurrentdmweapontype`
    - `injail`
    - `isEscaping`
    - `isSpectator`
    - `jailassistescape`
    - `jailescape`
    - `jailescapestop`
    - `killaxis`
    - `nationalityprefix`
    - `primaryfireheld`
    - `secondaryfireheld`
    - `turret`
    - `vehicle`
- Added prealarm thread support for actors
- Added RunAndShoot for actors
- Added ScriptThread functions: `getentbyentnum`, `landminedamage`, the `skipFade` option on `bsptransition`, `stopteamrespawn`, `setscoreboardtoggle`
- Added Sentient script functions: `forcedropweapon`
- Added `TriggerEntity` and `TriggerSidedUse` classes
- Allow ducking when glued onto something
- For trigger thread, now the first parameter is `parm.other` and the second is `parm.owner`
- Store the map filename in `mapfilename` variable
- Projectile can die in water using `dieinwater` command
- Unregister `Done` for doors that finished opening
- Vehicle can bounce backwards
- Vehicle can follow a path and be driven
- Vehicle can be invulnerable to projectiles
- Vehicle now has a maximum speed
- VehicleTurretGunTandem can have a `setswitchthread` that is executed when switching to the other tandem turret

##### UI

- Added instant action menu

#### Changed

##### Game/Server

- ScriptOrigin can only have a yaw angle now
- The reload sound for vehicle turret is played once when finished reloading

#### Fixed

##### Game/Server

- Fixed bash causing barrel leaks
- Fixed non-solid actors being stuck when the player was on it
- Make the vehicle also invulnerable if the player is invulnerable

### Breakthrough v2.40

#### Added

##### Game/Server

- Added battle language for players
- Added player script command: `modheightfloat`

## OpenMoHAA

OpenMoHAA includes all known fixes and features from the latest version of Spearhead and Breakthrough.

### General

- Demo recording
- Features from ioquake3
- IPv6 support (from ioquake3)
- Multiple master servers for the serverlist (using [333networks](https://333networks.com/))

### Client-side

- 4K support
- Customizable FOV
- Improved mouse support on windowed mode
- OpenAL and SDL are used
- Smoother animations

#### Stufftext restriction

Servers can no longer make players run any command. Only a small number of safe commands are now allowed. This change helps prevent abuse, like unbinding player controls, and improves overall security.

`stufftext` is now limited to trusted, commonly used commands, like those for controlling in-game music. The full list can be found in [cgame/cg_servercmds_filter.cpp](../../../../code/cgame/cg_servercmds_filter.cpp).

### Server-side

- Bots
- IP ban (from ioquake3)
- Improved logging: print client number / IP for each action, log name changes
- More script commands for mods
- Non-PVS optimization
- Packet flood protection
- Text chat management (enable/disable, configurable delay between messages)
- Use OS cursor in windowed mode

#### Non-PVS optimization

For each client, the server optimizes by only sending them information about other players that they can see. Clients won't see other players they can't see.

Enable this feature with `set sv_netoptimize 2`.

### Fixes

#### Client-side

- Animated shaders can be used in UI widgets
- Cleanup userinfo when disconnecting, it caused waste and connection problem to other servers
- Fixed a rare crash when displaying the scoreboard
- Fixed a very rare crash related to invalid remote models
- Fixed a crash related to invalid temp models being used
- Fixed background ambient sounds being muted when restarting
- Fixed bullet holes on door disappearing when the door moves
- Fixed terrain crashes
- Fixed UI locking up in rare case in the server list
- Keep the cursor at the same location when disconnecting
- Properly draw the players' world model when viewing from camera
- Server list redundancy (multiple master servers support)
- Reset the ambient sound when the server restarts

#### General

- Buffer overflow fixes
- Directory traversal fix
- Fixed `wait` command not working on dedicated servers

#### Server-side

- Allow using kar98 sniper on Breakthrough, when using german panzer skins
- Correctly clean up camera when exiting vehicle turrets
- DM message comment fix
- Fixed AI not shooting on Spearhead and Breakthrough
- Fixed AI not moving on multiplayer
- Fixed a crash when player disconnects from the server while inside a vehicle
- Fixed crash when increasing `sv_maxclients` and restarting
- Fixed a crash related to invalid turret for vehicles
- Fixed a crash related to invalid entity being attached to vehicles
- Fixed crashes related to scripts passing an invalid entity
- Fixed grenade crash bug
- Fixed grenade spectator charge bug
- Fixed an hang that could occur when an entitiy is removed from script
- Fixed huge time when a player joins during intermission
- Fixed infinite loop when firing bullets outside the map
- Fixed `leave_team` bug
- Fixed memory leaks issues in long matches
- Fixed players having the name of another player when rejoining after losing connection
- Fixed an issue where a player could die immediately after respawning
- Fixed sharking issues with ladders
- Fixed score from previous map glitch
- Fixed shaky camera when the ground entity is moving
- Fixed weapon being dropped sometimes when switching weapons too fast
- Fix to prevent getting weapons from opposite team
- Fix to prevent being able to spectate while still in the team with a NULL primary deathmatch weapon
- Fix to prevent throwing a projectile and going into spectator to kill anyone
- Fix for `sv_fps` (tickrate), higher values won't cause animation issues anymore
- Grenades with 1 ammo can now be picked up
- Prevent crashing when the last weapon has no ammo and other weapons are inventory items
- Prevent doors from getting blocked
- Prevent empty weapons from being picked up by a sentient who already have the weapon
- Prevent firing from below the terrain
- Prevent players from choosing the `_fps` model and crashing the server
- Prevent players from equiping weapons from the other team with the skin
- Prevent the "silent walk" glitch (rapidly switching weapons while walking)
- Vehicles will still continue moving on death sentients

### Changed

#### Server-side

- Better logging of clients and chat
- Don't set `g_shownpc` to true automatically
- Improved portable turret placement
- Increased the DM message limit in multiplayer

## Planned features

This is a non-exhaustive list of objectives and planned features.

### Server-side

- Anticheat
- Autodownload
- More feature for mods
- Privileged commands
- Stats system