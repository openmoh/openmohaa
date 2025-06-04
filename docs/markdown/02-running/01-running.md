# Running the game

## Game selection

**Medal of Honor: Allied Assault** is the default game, but expansions are also supported.

### Start using launchers

Base game and expansions can be started from one of the 3 launchers:

- `launch_openmohaa_base`, use this to play **Medal of Honor: Allied Assault**
- `launch_openmohaa_spearhead`, use this to play **Medal of Honor: Allied Assault: Spearhead**
- `launch_openmohaa_breakthrough`, use this to play **Medal of Honor: Allied Assault: Breakthrough**

### Start from the command-line

**Spearhead** and **Breakthrough** are supported in OpenMoHAA using the `com_target_game` variable.

To change the target game, append the following command-line arguments to the `openmohaa` and `omohaaded` executable:

- `+set com_target_game 0` for the default base game (mohaa, uses `main` folder)
- `+set com_target_game 1` for the Spearhead expansion (mohaas, uses `mainta` folder)
- `+set com_target_game 2` for the Breakthrough expansion (mohaab, uses `maintt` folder)

OpenMoHAA will also use the correct network protocol version accordingly. The default value of `com_target_game` is 0.

On Windows, a shortcut can be created to the `openmohaa` executable, with the command-line argument appended from above to play an expansion.

### Using a demo version

The argument `+set com_target_demo 1` must be appended to command-line to play the game or host a server using demo assets. Allied Assault, Spearhead and Breakthrough demos are supported.

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

To configure bots, see [Bots configuration](../03-configuration/01-configuration.md#bots).
