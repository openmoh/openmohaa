# Bot settings

## Global settings

### `g_bot_initial_spawn_delay`

- **Default**: 0
- **Type**: float (seconds)

#### Description

This sets how long the game should wait before spawning bots after loading a new map.

#### Usage

- `0`: Bots spawn instantly at map start (default).
- `5`: Bots spawn 5 seconds after the map begins.

#### Notes

- Applies only once when a new map has finished loading. It is not triggered on restarts or between rounds.
- Doesn't affect individual bot respawns during gameplay.

## Altering behavior

There is no skill system yet, however some settings can be modified to alter bot difficulty:

### `g_bot_attack_burst_min_time`

- **Default**: 0.1
- **Type**: float (seconds)

#### Description

Minimum time to pause firing (burst).

### `g_bot_attack_burst_random_delay`

- **Default**: 0.5
- **Type**: float (seconds)

#### Description

Random time added to pause firing (burst).

### `g_bot_attack_continuousfire_min_firetime`

- **Default**: 0.5
- **Type**: float (seconds)

#### Description

Minimum duration of continuous firing.

### `g_bot_attack_continuousfire_random_firetime`

- **Default**: 1.5
- **Type**: float (seconds)

#### Description

Random time added to the continuous firing duration.

### `g_bot_attack_react_min_delay`

- **Default**: 0.2
- **Type**: float (seconds)

#### Description

The minimum delay before shooting the enemy.

### `g_bot_attack_react_random_delay`

- **Default**: 1.0
- **Type**: float (seconds)

#### Description

Random delay added before shooting the enemy.

### `g_bot_attack_spreadmult`

- **Default**: 1.0
- **Type**: float

#### Description

Controls how accurate bots are when shooting.

#### Usage

- Lower values (< 1.0): More accurate, more likely to land headshots.
- Higher values (> 1.0): Less accurate, more likely to miss their target.

### `g_bot_turn_speed`

- **Default**: 15
- **Type**: float (degrees)

#### Description

The rate of degrees per second when turning.

### `g_bot_instamsg_chance`

- **Default**: 5
- **Type**: integer

#### Description

The chance at which the bot sends an instant message when shooting.

#### Usage

- 0: Disable.
- higher values: Less frequent messages.

### `g_bot_instamsg_delay`

- **Default**: 5.0
- **Type**: float (seconds)

#### Description

The minimum delay between instant messages.
