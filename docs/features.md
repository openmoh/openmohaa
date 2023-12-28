# Features

A lot of bugs and exploits from MOH:AA were fixed in OpenMoHAA (BOF exploit, grenade crash bug, command overflow...).

## Additions

### Client-side and server-side

- Features from ioquake3
- IPv6 support (from ioquake3)
- Features from SH 2.15 and BT 2.40

### Server-side

- Bots
- Demo recording
- IP ban (from ioquake3)
- More script commands for mods
- Non-PVS optimization
- Spectate players in first-person

#### Non-PVS optimization

For each client, the server optimizes by only sending them information about other players that they can see. Clients won't see other players they can't see.

Enable this feature with `set sv_netoptimize 2`.

## Fixes

### Client-side and server-side

- Fixes from SH 2.15 and BT 2.40
- Buffer overflow fixes
- Directory traversal fix

### Server-side

- DM message comment fix
- Callvote fix
- Getting weapons from opposite team
- Grenade crash bug
- Grenade spectator charge bug
- `leave_team` bug fix
- Primary DM weapon spectator fix
- Projectile spectator glitch fix (for when firing a projectile then going into spectator to kill anyone)
- `sv_fps` (tickrate) fix, higher values won't cause animation issues anymore

## Planned features

This is a non-exhaustive list of objectives and planned features.

### Server-side

- 100% compatibility with mohaa content
- More feature for mods
- Anticheat
- Stats system
- Multiple roles/abilities for server admins to reduce password-stealing