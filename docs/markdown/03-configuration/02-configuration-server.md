# Server configuration settings

Below is an explanation of the server settings commonly found in the `server.cfg` file.

## Variables

### `dmflags`

This variable is bit-based, meaning multiple flags can be combined.

Use `bitset dmflags n` to set individual flags, where `n` matches a value from the table below.

The following are valid values across all games:

|Bit  |Description
|-----|-----------
|0    |Disable health item
|2    |Make weapons stay when picked up
|3    |Prevent fall damage
|5    |Kill the player when it reaches a `trigger_changelevel` trigger
|14   |Infinite ammo
|22   |Disable rifle
|23   |Disable sniper
|24   |Disable submachine gun
|25   |Disable machine gun
|26   |Disable rocket
|27   |Disable shotgun
|28   |Disable landmine

The following are valid values across **Spearhead** and **Breakthrough**:
|Bit  |Description
|-----|-----------
|18   |Allow lean while moving
|19   |Allow old Sniper
|20   |Use shotgun for axis instead of the kar98 mortar

The following are valid values on **Breakthrough**:
|Bit  |Description
|-----|-----------
|21   |Always allow landmines

Example:
```
bitset dmflags 3
bitset dmflags 18
```
This sets the `dmflags` to prevent fall damage and allow leaning while moving.

### `fraglimit`

This sets the elapsed time at which the game ends, the winner being the player with the highest score at that time. Defaults to `0`.

### `g_allowjointime`

In round and objective matches, this is the maximum time at which a player can join a team without being put in spectator. Defaults to `30`.

### `g_forceteamspectate`

Forces players to only be able to spectate behind their own team. Defaults to `1`.

### `g_gametype`

The game mode to use. The following values are accepted:

|Value |Name
|------|--------------------------
|1     |Free for All
|2     |Team deathmatch
|3     |Round-based match
|4     |Objective
|5     |Tug-of-War (Spearhead)
|6     |Liberation (Breakthrough)

### `g_healthdrop`

- `0`: Players don't drop an health pack when killed
- `1` (the default): Players drop an health pack when killed

### `g_inactivekick`

The time in second after which an inactive player gets kicked. Defaults to `900`

### `g_inactivespectate`

The time in second after which an inactive player gets moved into spectator. Defaults to `60`

### `g_password`

Make the server joinable only with a password. By default, no password is set.

### `g_teamdamage`

- `0` (the default): No friendly-fire
- `1`: Friendly-fire enabled

### `g_teamkillkick`

Amount of team kills before the player is kicked off the server. Defaults to `5`.

### `g_teamkillwarn`

Amount of team kills before the player is warned. Defaults to `3`.

### `g_teamswitchdelay`

The minimum delay to wait before a player can switch teams again. Defaults to `15`.

### `rconpassword`

The password so players using the same password can execute commands remotely. By default, no password is set.

### `roundlimit`

Sets how long each round takes, in minutes. Defaults to `0`.

### `sv_dmspeedmult`

The speed multiplier in multiplayer. Defaults to `1.1`

### `sv_floodprotect`

Limits the amount of text a player can type.
- `0` (the default): There is no delay between commands
- `1`: Player will have to wait 1 second before typing a new command

### `sv_invulnerabletime`

Time in seconds at which a player is invulnerable after spawning. `0` means no invulnerability.
- Defaults to `0` (Allied Assault)
- Defaults to `3` (Spearhead and Breakthrough)

### `sv_gamespy`

- `0`: Disables gamespy, the server won't appear on the server browser
- `1` (the default): Enable GameSpy, the server will appear on the server browser

### `sv_hostname`

The name that appears on the server browser. Defaults to `Nameless OpenMoHAA Battle`.

### `sv_keywords`

Keywords, only relevant for server browsing software that handle this value. Not useful for the game client. By default, no keyword is set.

### `sv_maplist`

The map rotation, delimited by spaces. After the game ends, the server will start the next map in the rotation list. By default, the list is empty, which means the server restarts after the game ends.

### `sv_maxPing`

Maximum ping to allow clients to join with. `0` no maximum (the default).

### `sv_maxRate`

Maximum rate. `0` is unlimited (the default)

### `sv_minPing`

Minimum ping to allow clients to join with. `0` no minimum (the default).

### `sv_minRate`

Minimum rate at which clients receive data. `0` means no minimum (the default).

### `sv_privateClients`

The number of slots to reserve for players joining with the `password` set for when the server is full. The default is `0` (no reserved slot).

### `sv_privatePassword`

The password for players to use reserved slots. By default, no password is set.

### `sv_sprintmult`

The speed multiplier when the player is sprinting. The default is `1.2`.

### `sv_sprinton`

Whether or not to allow sprinting, which makes the player run faster after some time (defined by `sv_sprinttime_dm`)

- `0` (the default on Allied Assault): No sprint
- `1` (the default on Spearhead and Breakthrough): Enable sprinting

### `sv_sprinttime`

The time in seconds at which the player starts running faster. The default is `5`.

### `sv_team_spawn_interval`

Set the interval between spawns on team games.
- Defaults to `0` on Allied Assault
- Defaults to `15` on Spearhead and Breakthrough

### `timelimit`

This sets the elapsed time at which the game ends, in minutes. The winner being the player with the highest score at that time. Defaults to `0`.
