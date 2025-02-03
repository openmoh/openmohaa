# Events

OpenMoHAA introduces a new way for scripts to monitor for specific events, like players spawning or getting killed.

## Subscribing

Script commands related to events:
```cpp
event_subscribe event_name script_label
    Subscribes to an event, script_label will be called when the event is triggered

event_unsubscribe event_name script_label
    Unsubscribe from an event, script_label will not be called anymore
```

Example:
```cpp
main:
    event_subscribe "player_spawned" event_player_spawned
end

event_player_spawned:
    iprintlnbold("player entity number " + self.entnum + " just spawned!")
end
```

When monitoring is not needed, make sure to call `event_unsubscribe` with the same parameters used when subscribing to events.

## List of events

### Player events

The `self` object in scripts for all events will be the player object.

#### player_connected

The player spawned for the first time.

This event is called:
- When a client spawns for the first time
- When the map restarts, or when the map changes (all players)
- On the next round (all players)

It's called after the player finished spawning with weapons, and before `player_spawned` event.

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

The player just spawned. This is called when:
- The player has entered the battle
- The player respawned or spawned with weapons

The event can be called even for spectators (when the spectator gets respawned).

It's called after the player finished spawning with weapons.

#### player_textMessage


The player sent a text message.

```
player_textMessage local.text local.is_team
```

Parameters:
- local.text: The full text message with battle language tokens applied
- local.is_team: `1` if it's a team message. `0` otherwise.
