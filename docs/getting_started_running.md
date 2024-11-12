# Running

## Game selection

### From command-line

**Medal of Honor: Allied Assault** is the default game, but expansions are also supported.

**Medal of Honor: Allied Assault Spearhead** and **Medal of Honor: Allied Assault Breakthrough** are supported in OpenMoHAA using the `com_target_game` variable.

To play an expansion, append the following command-line arguments to the executable:

- `+set com_target_game 1` for the Spearhead expansion (mohaas/mohta, mainta, a.k.a Team Assault)
- `+set com_target_game 2` for the Breakthrough expansion (mohaab/mohtt, maintt, a.k.a Team Tactics)

With `com_target_game`, OpenMoHAA will support the network protocol accordingly. The default value of `com_target_game` is 0 for the base game (mohaa, main).

On Windows, a shortcut can be created to the `openmohaa` executable, with the command-line argument appended from above to play an expansion.

You can now start a local OpenMOHAA server or play on a server.

### Playing with the downloadable demo

The argument `+set com_target_demo 1` must be appended to command-line to play the game/host a server using demo assets: Allied Assault, Spearhead and Breakthrough demo are supported.

### From the launchers

For simplicity, there are 3 launchers for the standalone game:

- `launch_openmohaa_base`, this starts OpenMoHAA in **Medal of Honor: Allied Assault** mode which is the base game
- `launch_openmohaa_spearhead`, this starts OpenMoHAA in **Medal of Honor: Allied Assault: Spearhead** mode
- `launch_openmohaa_breakthrough`, this starts OpenMoHAA in **Medal of Honor: Allied Assault: Breakthrough** mode

## User data location

The location of user-writable data, like the console logfile, saves and configuration files can be found in different locations depending on the platform:

- `%APPDATA%\openmohaa` on Windows
- `~/.openmohaa` on Linux
- `~/Library/Application Support/openmohaa` on macOS

There will be one or more subdirectories like in the game installation folder, they match the game being used: either base game `main` or expansions `mainta`/`maintt`.

This is by design since ioquake3 and has two advantages:
- On a multi-user system, each user will have their own configuration file
- It doesn't overwrite the existing MOHAA configuration in the MOHAA installation directory.

If necessary, the location of user-writable data can be changed manually by setting the `fs_homepath` variable in the command-line argument. This is useful when running a dedicated server that can only use the game directory to store/read data. The value can be a relative path (relative to the current working directory) or an absolute path. Example:
- `+set fs_homepath Z:\openmohaa_data` data will be written inside the fully qualified path `Z:\openmohaa_data`
- `+set fs_homepath homedata` will use the subfolder `homedata` in the process current working directory to write data (will be created automatically)
- `+set fs_homepath .` not recommended, will write data inside the process current working directory

The game directory is intended to be read-only, which is the reason why the home path exists. This prevents existing files in the game directory from being accidentally overwritten.

Note that the configuration file isn't created nor written automatically on a dedicated server (**omohaaded**).

## Playing with bots

OpenMoHAA has a basic bot system that emulates real players. The maximum number of bots is defined by the `sv_maxbots` variable, and the number of initial bots is defined by the `sv_numbots` variable. Bots can also be added or removed using the `addbot` and `removebot` commands.

This feature is a great way to test the gameplay and mods.

Note that maps must have path built for bots to move, multiplayer maps don't have path built by default. The [mp-navigation](https://github.com/openmoh/mp-navigation) PK3 file can be downloaded and installed in the `main` directory, it adds navigation for all multiplayer maps, including multiplayer maps from Spearhead and Breakthrough.
