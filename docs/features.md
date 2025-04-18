# Features

A lot of bugs and exploits from MOH:AA were fixed in OpenMoHAA (BOF exploit, grenade crash bug, command overflow...).

## Summary

- SDL2 backend
- OpenAL sound support (better sound quality)
- Full 64-bit support
- Cross-platform support
- Multiuser support on OS (On Windows, user game data is stored in "%APPDATA%\openmohaa")
- Many bug fixes and additions from ioquake3
- Automatic scaling of UI elements on high resolutions like 4K

Overall, better compatibility on modern systems and bugfixes.

## Additions

### General

- Demo recording
- Features from ioquake3
- Features from MOH: Spearhead 2.15 and MOH: Breakthrough 2.40
- IPv6 support (from ioquake3)

### Client-side

- Customizable FOV
- OpenAL and SDL are used
- Smoother animations

#### Stufftext restriction

Servers can no longer make players run any command. Only a small number of safe commands are now allowed. This change helps prevent abuse, like unbinding player controls, and improves overall security.

To improve safety and the overall experience for everyone, `stufftext` is now limited to trusted, commonly used commands, like those for controlling in-game music.

The full list can be found in [cgame/cg_servercmds_filter.c](../code/cgame/cg_servercmds_filter.c).

### Server-side

- Bots
- IP ban (from ioquake3)
- More script commands for mods
- Non-PVS optimization
- Packet flood protection
- Ability to enable/disable the text chat and the voice message chat and set a delay between chat/instant messages

#### Non-PVS optimization

For each client, the server optimizes by only sending them information about other players that they can see. Clients won't see other players they can't see.

Enable this feature with `set sv_netoptimize 2`.

## Fixes

### Client-side and server-side

- Fixed background ambient sounds being muted when restarting
- Fixes from SH 2.15, BT 2.40 and ioquake3
- Buffer overflow fixes
- Directory traversal fix

### Server-side

- DM message comment fix
- Callvote fix
- Fix to prevent getting weapons from opposite team
- Fixed grenade crash bug
- Fixed grenade spectator charge bug
- Fixed `leave_team` bug
- Fixed memory leaks issues in long matches
- Fixed sharking issues with ladders
- Fix to prevent being able to spectate while still in the team with a NULL primary deathmatch weapon
- Fix to prevent throwing a projectile and going into spectator to kill anyone
- Fix for `sv_fps` (tickrate), higher values won't cause animation issues anymore
- Grenades with 1 ammo can now be picked up
- Prevent crashing when the last weapon has no ammo and other weapons are inventory items
- Prevent doors from getting blocked

## Planned features

This is a non-exhaustive list of objectives and planned features.

### Server-side

- 100% compatibility with mohaa content
- More feature for mods
- Anticheat
- Stats system
- Multiple roles/abilities for server admins to reduce password-stealing