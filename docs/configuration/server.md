# Server configuration and commands

This documentation currently only lists new changes that were introduced in OpenMoHAA.

If you want to use containers, see [Building a server container in Docker](../../docker/server/README.md) to build an image for your dedicated server.

## Home directory

In original MOH:AA, the game installation directory is used to store mods and data, it's not the case in OpenMoHAA as it uses the home directory by default to write data in here, and the home directory can be used to store mods. This behavior can be changed:

- `set fs_homepath Z:\openmohaa_data`: User data will be read and written in the directory located in `Z:\openmohaa_data`
- `set fs_homepath homedata`: The subdirectory `homedata` in the game directory will be used to read and store user data
- `set fs_homepath .`: Not recommended, the game directory will be used for storing user data, just like the original MOH:AA

The variable defaults to the following value depending on the OS:

- `%APPDATA%\openmohaa` on Windows
- `~/.openmohaa` on Linux
- `~/Library/Application Support/openmohaa` on macOS

## Networking

### Configure the network components

The network settings can be adjusted to use either IPv4, IPv6, or both. By default, IPv6 is disabled on dedicated servers. The following commands adjust network settings:

- `set net_enabled 1`: This enables IPv4 only (the default setting for dedicated servers).
- `set net_enabled 2`: This enables IPv6 only.
- `set net_enabled 3`: This enables both IPv4 and IPv6 (the default setting when running the standalone game).

*Note: The master server (using the GameSpy protocol) does not support IPv6. If IPv4 is disabled, the server won't appear in the online server list for internet games, even if IPv6 is enabled.*

### Optimization / Antichams

A new variable, `sv_netoptimize`, enables a feature that optimizes network bandwidth by not sending players information about others they can't see. For each client, the server optimizes by only transmitting data about players within their view. Clients will not receive information about players they can't see. This feature also helps protect against cheaters:

- `set sv_netoptimize 0`: This disables the optimization - the default
- `set sv_netoptimize 1`: This enables the optimization for entities that are moving
- `set sv_netoptimize 2`: This enables the optimization, always

This option exists since **Medal of Honor: Allied Assault Breakthrough** 2.30, however it was improved in OpenMoHAA: sounds like footsteps will be sent so players don't get confused.

### Managing bans

A new feature was introduced to ban IP addresses, thanks to the [ioquake3](https://ioquake3.org/) project. Bans are saved by default in `serverbans.dat` but it can be modified with the `sv_banFile` variable. Here are commands to manage bans:

|Name       |Parameters                             |Description
|-----------|---------------------------------------|-----------
|rehashbans |                                       |Loads saved bans from the banlist file
|listbans   |                                       |Lists all banned IP addresses
|banaddr    |ip[*/subnet*] \| clientnum [*subnet*]  |Bans an IP through its address or through a client number, a subnet can be specified to ban a network range
|exceptaddr |ip[*/subnet*] \| clientnum [*subnet*]  |Adds an IP as an exception, for example IP ranges can be banned but one or more exceptions can be added
|bandel     |ip[*/subnet*] \| num                   |Unbans an IP address or a subnet, the entry number can be specified as an alternative
|exceptdel  |ip[*/subnet*] \| num                   |Removes a ban exception
|flushbans  |                                       |Removes all bans

Examples:

- `banaddr 192.168.5.2` will ban the IP address **192.168.5.2**.
- `banaddr 192.168.1.0/24` will ban all **192.168.1.x** IP addresses (in the range **192.168.1.0**-**192.168.1.255**).
- `banaddr 2` will ban the IP address of the client **#2**.
- `banaddr 4 24` will ban the subnet of client **#4** - i.e if client .**#4** has IP **192.168.8.4**, then it will ban all IPs ranging from **192.168.8.0**-**192.168.8.255**.
- `exceptaddr 3` will add the IP of client **#3** as an exception.
- `bandel 192.168.8.4` will unban **192.168.8.4**.
- `bandel 192.168.1.0/24` will unban the entire **192.168.1.0** subnet (IP ranging from **192.168.1.0**-**192.168.1.255**).

To calculate IP subnets, search for `IP subnet calculator` on Internet.

## Flood protection differences with MOH: Spearhead

Flood protection is turned on by default in all games (`sv_floodProtection 1`).

- In MOH: Allied Assault and OpenMoHAA, flood protection checks all commands.
- In MOH: Spearhead 2.0 and later, flood protection only checks for text messages.

While flood protection prevents spam, it can sometimes be annoying in certain situations like reloading and checking scores within a short period of time. If needed, it can be disabled with `set sv_floodProtection 0`.

For more details on preventing message spamming, check out the [Chat](#chat) section below.

## Game

### Chat

Chat messages will be logged in the console and in the logfile without requiring to set the `developer` variable.

The in-game chat can be tweaked:

- `set g_instamsg_allowed 0`: This disables voice instant messages.
- `set g_instamsg_minDelay x`: x is the delay in milliseconds, the minimum delay between each instant messages to avoid spamming. Defaults to 1000.
- `set g_textmsg_allowed 0`: This disables text messages. All, team and private messages will be disabled.
- `set g_textmsg_minDelay x`: x is the delay in milliseconds, the minimum delay between each text message to avoid spamming. Defaults to 1000.

Temporarily disabling text messages can be useful in situations where tensions arise in the chat. Otherwise, it's best to keep them enabled under normal circumstances.

### Balancing teams

This setting prevents clients from joining a team if that team already has more players than the others. By default, it's turned off, but it can be switched on with the command `set g_teambalance 1`.

This feature is passive: it only checks the team sizes when someone tries to join, so it won't automatically balance teams during the game.

*Note: This check doesn't apply in server scripts; it only works when clients join teams directly.*

### Bots

Bots are supported and can be used for testing or for entertainment. The [mp-navigation](https://github.com/openmoh/mp-navigation) pk3 will be needed so bots can navigate and move on MP maps. The following variables are used to configure bots:

- `set sv_maxbots x`: **Required**, configure the maximum number of bots allowed in the game. Since the game can only handle a total of 64 players (clients), the number of bots will be limited to 64 minus the number of real players (`sv_maxclients`). For example, if you set `sv_maxclients` to 48, the maximum number of bots (sv_maxbots) can be 16.
- `set sv_numbots x`: Set the number of bots to spawn. It will be capped to the value of `sv_maxbots`.
- `set sv_minPlayers x`: Configure the minimum number of players required. If the number of real players is below the specified value, the game will automatically add bots to fill the gap. For example, if `sv_minPlayers` is set to 8 and only 5 real players are connected, the game will spawn 3 bots to make sure there are always 8 players in the game.

Commands:

- `addbot x`: x is the number of bots to add. It only changes the `sv_numbots` variable.
- `removebot x`: x is the number of bots to remove. It only changes the `sv_numbots` variable.

Example with the requirement of 6 players:
```cpp
set sv_maxbots 16 // Reserve 16 client slots for bots
set sv_minPlayers 6 // When there is 0 player in a team, there will be 6 bots. When there is 1 player in a team, there will be 5 bots, and so on
```

Example with 4 bots playing:
```cpp
set sv_maxbots 16 // Reserve 16 client slots for bots
set sv_numbots 4 // Spawn 4 bots
```
