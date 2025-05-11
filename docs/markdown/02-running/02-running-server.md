# Hosting a server

## Installation

This guide assume you have already acquired and installed a copy of MOH:AA on your server. If not, see [Getting started](../01-intro/01-installation.md).

1. Download the [latest release](https://github.com/openmoh/openmohaa/releases) that matches your server's platform.
2. Extract files from the archive into your MOH:AA directory

You can also use Docker, see [Creating a Docker image](04-docker.md).

## Preparing the server settings

Inside the **main** directory of your game folder, create a `server_opm.cfg` file with the following content:

```c
// Server Name
// Name that will appear on the server browser
sv_hostname "OpenMoHAA Server"

// Private Clients
// Clients that require a password to join
set sv_privateclients 0
set sv_privatepassword "password_for_slots"

// Password
// Allows you to set a password for the server
set g_password ""

// Game Type
// 1 Free for All
// 2 Team Based
// 3 Round Based
// 4 Objective
// 5 Tug-of-War
// 6 Liberation
set g_gametype 4

// RCON Capability
// Leave blank to disable rcon.
set rconpassword ""

// Health Drop
// When enabled players will drop a health pack when killed
set g_healthdrop 1

// Fast Run Speed
// When checked, the speed at which players can run through levels is increased. 
// Leaving it unchecked defaults to Medal of Honor Allied Assault's original run speed
//set sv_dmspeedmult 1.100000

// Team Damage
// Toggles friendly fire
set g_teamdamage 0

// Time Limit
// This sets the elapsed time at which the game ends, the winner being the player with the highest score at that time.
timelimit 10

// Frag Limit
// The score at which the player or team will win the current map.
fraglimit 100

// Team Spawn Delay
// This sets a delay in seconds between spawns. This is useful if you would like the 
// player spawned in batches, rather than one at a time
//set sv_team_spawn_interval 15

// Round Reset Time
// This sets how long each round will take, in minutes. This can be smaller than the map rotation time, but not larger.
roundlimit 0

// Inactive Spectate
// This sets the time in seconds after which an inactive player becomes a spectator. To save server bandwidth, 60 seconds is default.
set g_inactivespectate 60

// Inactive Kick
// This sets the time in seconds after which an inactive player is kicked out of the game. 900 (15 minutes) seconds is default.
set g_inactivekick 900

// Use Gamespy
// To use GameSpy gaming service for internet-based multiplayer games.  
// If this is 0, players will not be able to see your server via the in-game browser nor gamespy arcade.
set sv_gamespy 1

// Flood Protect
// Limits amount of text a player can type
set sv_floodprotect 1

// Forces players to only be able to spectate behind their own team. 
set g_forceteamspectate 1

//  dmflags -- flags that can be set in the dmflags variable.
// DF_NO_HEALTH              (1 << 0)
// DF_NO_POWERUPS            (1 << 1)
// DF_WEAPONS_STAY           (1 << 2)
// DF_NO_FALLING             (1 << 3)
// DF_INSTANT_ITEMS          (1 << 4)
// DF_SAME_LEVEL             (1 << 5)
// DF_SKINTEAMS              (1 << 6)
// DF_MODELTEAMS             (1 << 7)
// DF_FRIENDLY_FIRE          (1 << 8)
// DF_SPAWN_FARTHEST         (1 << 9)
// DF_FORCE_RESPAWN          (1 << 10)
// DF_NO_ARMOR               (1 << 11)
// DF_FAST_WEAPONS           (1 << 12)
// DF_NOEXIT                 (1 << 13)
// DF_INFINITE_AMMO          (1 << 14)
// DF_FIXED_FOV              (1 << 15)
// DF_NO_DROP_WEAPONS        (1 << 16)
// DF_NO_FOOTSTEPS           (1 << 17)
// DF_ALLOW_LEAN_MOVEMENT    (1 << 18)
// DF_OLD_SNIPER             (1 << 19)
// DF_DISALLOW_KAR98_MORTAR  (1 << 20)
// DF_WEAPON_LANDMINE_ALWAYS (1 << 21)
// DF_WEAPON_NO_RIFLE        (1 << 22)
// DF_WEAPON_NO_SNIPER       (1 << 23)
// DF_WEAPON_NO_SMG          (1 << 24)
// DF_WEAPON_NO_MG           (1 << 25)
// DF_WEAPON_NO_ROCKET       (1 << 26)
// DF_WEAPON_NO_SHOTGUN      (1 << 27)
// DF_WEAPON_NO_LANDMINE     (1 << 28)

// allow leaning while moving on Spearhead and Breakthrough
bitset dmflags 18

// Invulnerable Time
// Amount of time (in seconds) a player is invulnerable for after spawning (default=3 seconds)
set sv_invulnerabletime 3

// Team Kill Warning
// Amount of team kills before the player is warned
set g_teamkillwarn 3

// Team Kill Kick
// Amount of team kills before the player is kicked off the server.
set g_teamkillkick 5

// Team Switch Delay
// Frequency at which you can switch teams
set g_teamswitchdelay 15

// Allow Join Time
set g_allowjointime 30

// Keywords
// Keywords which allow browsers to filter servers.
set sv_keywords "MOHAA Server"

// Minimum Ping to allow clients to join with.  0 means anyone
set sv_minping 0

// Maximum Ping to allow clients to join with.  0 means anyone
set sv_maxping 800

// Maximum Rate
// 0 = unlimited
set sv_maxrate 0

// Whether or not to allow sprinting
sv_sprinton 1
//sv_runspeed 250 // spearhead runspeed is 287

// Map Rotation List
sv_maplist "obj/obj_team1 obj/obj_team2 obj/obj_team3 obj/obj_team4 "

// Map
// Starting map on the rotation. Kicks the server into gear
map "obj/obj_team1"
```

### Configuring the server

- For a list of commonly used settings in the `server.cfg` file, see [Server configuration](../03-configuration/02-configuration-server.md).
- For examples and new OpenMoHAA features (like setting up bots), see [Game configuration](../03-configuration/01-configuration.md)

## Starting the server

Run the `omohaaded` executable from your MOHAA directory. Use one of the following commands, depending on the game you want to use:

- For **Allied Assault**:
  
  `./omohaaded +set com_target_game 0 +exec server_opm.cfg`
- For **Spearhead**:

  `./omohaaded +set com_target_game 1 +exec server_opm.cfg` 
- For **Breakthrough**:

  `./omohaaded +set com_target_game 2 +exec server_opm.cfg`
