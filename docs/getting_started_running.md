# Running

## Game selection

**Medal of Honor: Allied Assault Spearhead** and **Medal of Honor: Allied Assault Breakthrough** are supported in OpenMoHAA using the `com_target_game` variable. To play an expansion, append the following command-line arguments to the executable:

- `+set com_target_game 1` for Spearhead (mohaas/mohta)
- `+set com_target_game 2` for Breakthrough (mohaab/mohtt)

On Windows, you can create a shortcut to OpenMoHAA with these command-line arguments.
The default value of `com_target_game` is 0 for the base game (main).

With `com_target_game`, OpenMoHAA will support the network protocol accordingly.

You can now start a local OpenMOHAA server or play on a server.

## Config file

The config file can be found in different locations depending on the platform:

- `%APPDATA%\openmohaa\<game_name>\configs\omconfig.cfg` on Windows
- `~/.openmohaa/<game_name>/configs/omconfig.cfg` on Linux

This has two advantages:
- On a multi-user system, each user will have their own configuration file
- It doesn't overwrite the existing MOHAA configuration in the MOHAA installation directory.

Note that the configuration file isn't created nor written automatically on a dedicated server (**omohaaded**).