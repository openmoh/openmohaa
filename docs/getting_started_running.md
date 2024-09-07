# Running

## Game selection

**Medal of Honor: Allied Assault** is the default game, but expansions are also supported.

**Medal of Honor: Allied Assault Spearhead** and **Medal of Honor: Allied Assault Breakthrough** are supported in OpenMoHAA using the `com_target_game` variable. To play an expansion, append the following command-line arguments to the executable:

- `+set com_target_game 1` for Spearhead (mohaas/mohta, mainta)
- `+set com_target_game 2` for Breakthrough (mohaab/mohtt, maintt)

On Windows, you can create a shortcut to OpenMoHAA with these command-line arguments.
The default value of `com_target_game` is 0 for the base game (mohaa, main).

With `com_target_game`, OpenMoHAA will support the network protocol accordingly.

You can now start a local OpenMOHAA server or play on a server.

## User data location

The location of user data, like console logs, saves and config files can be found in different locations depending on the platform:

- `%APPDATA%\openmohaa\<game_name>` on Windows
- `~/.openmohaa/<game_name>` on Linux

This has two advantages:
- On a multi-user system, each user will have their own configuration file
- It doesn't overwrite the existing MOHAA configuration in the MOHAA installation directory.

Note that the configuration file isn't created nor written automatically on a dedicated server (**omohaaded**).

## Playing with bots

OpenMoHAA has a basic bot system that emulates real players. The maximum number of bots is defined by the `sv_maxbots` variable, and the number of initial bots is defined by the `sv_numbots` variable. Bots can also be added or removed using the `addbot` and `removebot` commands.

This feature is a great way to test the gameplay and mods.