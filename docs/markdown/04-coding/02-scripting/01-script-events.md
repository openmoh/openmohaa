# Script events

OpenMoHAA introduces a new way for scripts to monitor for specific events, like players spawning or getting killed. Multiple scripts can subscribe to the same events.

## Subscribing

Commands related to events:
```cpp
event_subscribe event_name script_label
    Subscribes to an event, script_label will be called when the event is triggered

event_unsubscribe event_name script_label
    Unsubscribe from an event, script_label will not be called anymore
```

When an event is not needed anymore, make sure to call `event_unsubscribe` with the same parameters used when subscribing to the event.

### Example

```cpp
main:
    event_subscribe "player_spawned" event_player_spawned
    // Can specify another script:
    //event_subscribe "player_spawned" global/test_script::event_player_spawned
end

event_player_spawned:
    iprintlnbold("player entity number " + self.entnum + " just spawned!")
end
```

## List of events

### Level events

#### level_exit

The level is exiting.

Called when:
- The server disconnects
- The map restarts or changes

This event is triggered at the start of the level exit process. At this stage, scripts cannot wait, and delayed commands will not be executed.

```
level_exit local.different_map
```

Parameters:
- local.different_map: `1` when switching to a different map or when fully reloading the map. `0` when restarting the level (for example, restarting for the next round).

#### level_intermission

The intermission has begun.

Called when the final intermission screen is shown to players.

This event is triggered at the start of the intermission screen.

### Player events

The `self` object is the player object for all triggered player events.

#### player_connected

The player entered the game.

Called when:
- When a client spawns for the first time
- When the map restarts, or when the map changes (for all players)
- On the next round (for all players)

This is called after the player finished spawning, and before `player_spawned` event.

#### player_damaged

The player just got hit.

The parameters are the same as the `damage` command:
```
player_damaged local.attacker local.damage local.inflictor local.position local.direction local.normal local.knockback local.damageflags local.meansofdeath local.location
```

#### player_disconnecting

The player is disconnecting.

#### player_killed

The player got killed.

The parameters are the same as the `killed` command:
```
player_killed local.attacker local.damage local.inflictor local.position local.direction local.normal local.knockback local.damageflags local.meansofdeath local.location
```

#### player_spawned

The player just spawned.

Called when:
- The player has entered the battle
- The player respawned or spawned with weapons

This is called after the player finished spawning.

The event can be called even for spectators (when the spectator gets respawned).

#### player_textMessage

The player sent a text message.

```
player_textMessage local.text local.is_team
```

Parameters:
- local.text: The raw text message the client sent to the server
- local.is_team: `1` if it's a team message. `0` otherwise (everyone)

The script can optionally return the following value:
- `0`: the message won't be sent.
- Any string value: the message won't be sent and a reply will be sent to the player.
