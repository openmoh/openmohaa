# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

#### Server

- Feature to name bots (most requested bot feature). See the [bot documentation](https://github.com/openmoh/openmohaa/blob/main/docs/markdown/03-configuration/01-configuration.md#bots). This may change in the future.
- More settings for bots. See the [documentation](https://github.com/openmoh/openmohaa/blob/main/docs/markdown/03-configuration/03-configuration-bots.md) for settings.
- Reason will be sent to clients getting kicked automatically by the game for teamkill or inactivity.
- Reflective team damage feature. See the [documentation](https://github.com/openmoh/openmohaa/blob/main/docs/markdown/03-configuration/02-configuration-server.md#g_teamdamage) for settings and explanations.
- Store the map file name in the `mapfilename` variable.

##### Scripting

- Add a third optional parameter for `removeclass` to specify if it should include subclasses (defaults to true).
- The owner of a projectile is now accessible from scripts.

#### General

- Add the bot count (if there are any) in the server browser. This will only work for servers running at least the version **0.83.0** of OpenMoHAA. The bot count will also show on the [333networks](https://master.333networks.com/) master server website.

#### Client

- Add support for one-shot animated shaders in the UI (#838) (not supported in the original game).

#### Platform

- Allow separation of config, data, and state directories within the home path (XDG directory support) (#588).

### Changed

- Older saves are incompatible due to changes from commit 5df5735965298b26b9bd4b90034843c743ff7158.

#### Server

- Prevent dropping the binoculars (this change was introduced in Spearhead v2.11).
- Servers now read and write into a configuration file `omconfig_server.cfg`, separate from the client configuration for convenience.

### Fixed

#### General

- Fix network profiling connectionless packet improperly counted.
- Fix Spearhead getting medals from Allied Assault when the config file doesn't exist (also occurs in the original game) (#818).

#### Server

- Allow `g_teamkillkick` to be set to 0. Players would get kicked when this variable was set to 0 (also occurs in the original game).
- Allow spectators and dead players to vote (#843).
- Fix a bug where players who switched to spectator mode immediately after dying would have 0 health and drop DM items (also occurs in the original game).
- Fix a crash that occurs when the server receives a gamespy cdkey authorize request.
- Fix a crash that could occur when starting a `jvbot` game.
- Fix a possible crash caused by scripts using `angles_pointat` (also occurs in the original game).
- Fix a rare glitch causing the player to be teleported to (0,0,0) and stuck (also occurs in the original game).
- Fix Actors not doing anything without client #0 (also occurs in the original game).
- Fix Actors squad formation.
- Fix an issue where clients could crash because of too many models being loaded from the server.
- Fix bad animation blending with portable turret.
- Fix bots being counted for voting.
- Fix death animation movement for bots.
- Fix holstered weapon attachment position (#847).
- Fix improper weapon falling rotation.
- Fix mismatched animation issues between the original clients and the server (#842).
- Fix players not having binoculars on Allied Assault.
- Fix players temporarily losing user input control on ARM-based OpenMoHAA servers when a message contained a special ASCII character.
- Fix some death animations flickering for original clients playing on an OpenMoHAA servers (#857).
- Fix some RCON programs not working with `dumpuser`.
- Fix the carryable turret disappearing when being dropped (#812).
- Fix the round limit being always `15` on Tug-of-War.
- Fix votes being messed up after calling a vote the second time during the match.
- Fix `wait` command not working on dedicated server binaries. Further commands would not be able to be executed (also occurs in the original game).
- Fix weapon select mod not working (#849).
- Fix wrong portable turret placement.
- Large vote configurations will no longer cause clients to disconnect (also occurs in the original game) (#858).
- The gamespy port `net_gamespy_port` now reflects the real port being used by the server (the issue also occurs in the original game).
- The player would spawn with no weapons on single-player when `sv_maxbots` was set (#783).

#### Clients

- Fix a crash caused when firing bullets to specific entities with no surface type (#831).
- Fix entities not being able to emit a light when attached to another entity (also occurs on the original Allied Assault but was fixed since Spearhead 2.0) (#824).
- Fix mapinfo (.min) files never being saved, resulting in the loading bar never showing on some maps (#832).
- Fix not being able to view sniper zoom when spectating.
- Fix sky being partially culled out on some single-player maps (#817).
- Fix the issue where the network would no longer work after hosting a multiplayer game on Allied Assault using the client.
- Fix wrong alignment for UI elements using `centery` (#848).

### Changed

#### Server

- Free-For-All is now the default gametype on dedicated servers (`g_gametype 1`) when unspecified, rather than single-player (the issue also occurs in the original game).
- Freezeplayer now freezes player animation/state, because frozen players would still emit footsteps sound when frozen.
- Nocliping when dead no longer makes the player animation sketchy.

#### Clients

- Slightly increase the brightness of red/green chat messages.

#### Misc

- Completely refactor CMake build system (from [ioquake3's](<https://github.com/ioquake/ioq3>) new CMake build system).

## [0.82.1] - 2025-08-05

### Fixed

#### Crashes

- Fix a crash that would occur when loading custom maps in some map packs (due to a UI incorrectly declaring a widget: loadingbar, twice) (#764)
- Fix an issue where the server could crash MOHAA base (1.11) clients in certain situations (#778)
- The game no longer terminate with error `FS_ReadFile with empty name`. (#772)

#### Drawing & rendering

- Fix force model always using an allied skin for dead bodies.
- Fix HUD being shown during briefings (#767)
- Fix in-game mouse cursor disappearing on some platforms (#765)
- Set a more distant fog for d-day by default (`set g_ddayfog 0`). This fixes an issue with sky artifacts in m3l1a. (#770)

#### Multiplayer

- Fix frequent telefrags on servers with a lot of players, like 25 players.
- Fix taunts being played as team rather than global (#775)

#### Miscellaneous

- Update third-party libraries like jpeg and zlib.

## [0.82.0] - 2025-08-05

### Added

#### General

- Commands `add`, `subtract`, `scale`, `append` and `bitset`. `bitset`, which comes from mohaab, is particulary useful to set flags for flag-based variables like `dmflags`.
- Logfile timestamping. It's enabled by default with the `com_logfile_timestamps` cvar. The current time is displayed at the beginning of each log entry.
- Setup files (.msi packages) are now distributed for Windows.
- Update check. The game will verify the latest version in the background (by querying the latest GitHub release of the repository) and print a 
- Add the **Instant Action** feature from Breakthrough. This feature, although only usable through the Breakthrough UI, allows anyone to quickly find and join popular low-ping multiplayer servers.

#### Multiplayer

- Add `(all)` prefix in text message when a player is talking to all
- A message will be printed into console/log when a client changes their name.
- In Spearhead and Breakthrough, axis players can now select the kar98 sniper by choosing a german panzer skin (this was one of the feature of Daven's patch).
- The client number and their IP address will be printed next to each action (text message, joined the game...)

##### Bots

- [Recast Navigation](https://recastnav.com/) is now used by default to generate the navigation for bots. It's all automatic and works out of the box for any level, there is no need for `mp-navigation.pk3`. It provides a better and more accurate ways for bots to navigate through the maps.
  - Bots can now take path that requires jumping, falling or climbing ladders.
  - Bots will now try to get around small obstacles in front of them.

*Note: bigger obstacles are minimally supported, but may cause some issues in some area, like the West axis spawn in Omaha Beach.*

It takes less than 2 seconds to generate the navigation mesh on large maps like Berlin and Druckkammern.

If you have issues, executing `set g_navigation_legacy 1` command and then reloading the map will fallback to the basic pathnode-based navigation system.

### Fixed

#### Audio

- Updated OpenAL to version 1.24.3
  - The sound quality has improved a little bit, the new version of OpenAL uses another sound resampler by default.
- Fix enhanced audio spatialization being forcefully enabled when it shouldn't for audio devices detected as headphones that aren't headphones.

#### General

- Fix `LOCALIZATION ERROR` spam.
- Fix an infinite loop that would occur during name sanitization.
- Fix a crash that would occur when using special menus (like admin menus).
- Fix memory leaks after the process has been running for a long time.
- Fix player name being cutoff at the first negative character
- Fix player being able to shoot bullets from under the LOD terrain (also occurs on the original game)
- Fix the `version` string, ensuring most software such as HLSW and mods work as intended depending on the target game version.

#### Client

- Fix bullet impacts having multiple effects sometimes.
- Fix cinematic not stopping on map change.
- Fix delayed SFX (such as smoke sound) being played on unrelated entities
- Fix `Illegible server message` error when firing some type of bullets on Spearhead and Breakthrough.
- Fix lean angle slightly not matching the original game.
- Fix localization issues.
- Fix Waffen skins not being usable on servers
- Player models (selected skins) will be reset to their default value in-game if it doesn't exist, and force models will not work if the model doesn't exist on the server.
- Reallow client `name` to be changed by the server. Names are often changed by administrators to fix duplicate names.

#### Multiplayer

- Correctly print the text of instant messages in the console/logfile.
- Fix AllocEdict when some Breakthrough maps are started in multiplayer mode (for co-op).
- Fix a crash related to spawnpoints.
- Fix a crash that would occur when playing a sound with subtitle without player #0.
- Fix a crash that occured when a player reconnects to the same slot (after timing out) and then displays the scoreboard
- Fix an issue preventing spawning next to spawn points with an offset. This lead to frequent telefrags with lot of players.
- Fix a crash that would occur when the player sets a first-person model as a skin, causing an infinite state loop (also occurs in the original game).
- Fix clients joining during intermission and having a corrupt `Time` value in the scoreboard (also occurs in the original game).
- Fix fog sometimes not updating on maps such as Gewitter.
- Fix incorrect number of wins shown for allies and axis in the scoreboard.
- Fix spawnpoints metrics.
- Fix missing player starts on some maps.
- Fix players temporarily having the name of another player when rejoining after losing connection (also occurs in original game)
- Fix projectiles with no owner getting removed when a player joins a team (also occurs in Spearhead and Breakthrough)
- Fix score not being able to have negative values in the result returned by the `status` server command.
- Fix scores being incorrectly sorted in round-based matches (they must be sorted by total kills).
- Fix some issues with laser mods, like the `Freeze-Tag` mode where the laser would not trace from the player sight.
- Fix `sv_precache` being ignored.
- Fix the issue where the text message gets empty when exceeding character limit.
- Fix the round ending incorrectly in mods like beach soccer.
- Fix weapon being dropped sometimes when switching weapons too fast (also occurs in original game)
- Increase text message character limit to 400 characters (taking the name of the player into account).

#### Game module

- Bazooka was never showing as the preferred weapon in singleplayer.
- Fix AI not shooting when squadmates are next to eachother in Spearhead and Breakthrough (also occurs in original game).
- Fix AI not moving to specific location when ordered.
- Fix AI misbehaving when curious.
- Fix AI moving too fast on Allied Assault.
- Fix AI having a notepad.
- Fix ammo type not properly localized.
- Fix an infinite loop that would occur on some maps when a vehicle start driving to a path.
- Fix an issue where legs animations (like idle pistol stand) would not loop.
- Fix an undefined behavior issue in some mods that could lead to infinite loop, like, `while (self.variable)` (where `self` is `NULL`).
- Fix a crash (MAX_GAMESTATE_CHARS) and a memory leak in long matches after many models are loaded. This would occur mostly in ToW maps during long matches, or in maps spawning many `Actor`.
- Fix a crash related to VehicleTurretGun having no vehicle owner (also occurs in original mohaab).
- Fix a crash that would occur when a turret shoots projectiles.
- Fix a crash when a player quits the server while in a vehicle (also occurs in original game).
- Fix damage blend not working in Spearhead and Breakthrough.
- Fix leaning not making a difference when trying to use.
- Fix mods like Elgbot server-side not working properly.
- Fix inconsistent cross-blending of torso animations.
- Fix "silent walk", where rapidly switching weapons would prevent emitting footstep sounds (also occurs in original game).
- Fix weapon banning not working properly when banning rifles.

#### Rendering

- Fix a crash that would occur when there are insufficient triangles for LOD terrain tessellation (also occurs in original game).
- Fix a possible crash when loading a map with lightmaps and then loading a map that has no lightmaps.
- Fix a rare crash that would occur when toggling the scoreboard (also occurs in original game, but was fixed in Breakthrough 2.30).
- Fix blurry text.
- Fix disconnected water lakes
- Fix console prompt being hidden when there are too many characters
- Fix line breaks occurring mid-word
- Fix long messages being cut off in the console
- Fix omaha beach shorelines being incorrectly rendered
- Fix the server list being requested twice on Allied Assault when browsing internet servers.

#### Server

- Fix `CM_AdjustAreaPortalState` error
- Fix clients rejected for too low/high ping not being able to reconnect again.
- Fix some programs (like gamedig) not being able to retrieve gamespy data from the server.
- Fix some programs not being able to retrieve the complete list of players
- Fix `SV_FindIndex: overflow` errors that would occur more often than the original game during long matches.
- Fix server crashing all clients under rare occasions.

### Changed

- Removed the architecture suffix from executables. They're now simply named `openmohaa` and `omohaaded`, for multiple reasons:
  - Avoids conflicts with shipped third-party DLLs (like SDL, OpenAL) which have no suffix.
  - Easier to build an installer and document.
  - Makes launchers and automation scripts simpler (universal name for executable files).
  - The architecture is already indicated in the archive name, on startup and in the `version` variable.*

- Older saves are incompatible due to changes from commit 4e8ddf3c358baef7cbfe2c8ff66c2426338cd2c7.

#### Client

- A random number is now added for the default name, like `UnnamedSoldier#1234`, or `*** Blank Name #1234 ***`.

#### Multiplayer

- All player models inside the `models/player/` folder will be cached in multiplayer. This avoids short lags/hitches when a player spawns with an un-cached model.

##### Master server

- Updated the master server to use [333networks](https://333networks.com/).
- Add support for multiple master servers. The server browser queries them all at once, and servers send updates to each. They are powered by the **333networks** community, managed independently. Even if one master goes down, others are used, ensuring constant availability of multiplayer.
- Iccups will occur less when fetching the server list if the network is really slow, or if the master server is unreachable.

#### Rendering

- On windowed mode, the hardware cursor is now used to navigate in the UI. This quickly allow users to get out of the window when bringing the console or the main menu up, instead of doing ALT+TAB.

#### Server

- The server is now showing the project version number in the server list.

## [0.81.1] - 2025-01-04

### Fixed

- Fix the text of pulldown elements being invisible on some menus
- Fix the server not taking the client rate into account and transmitting network fragments too fast, this may be the reason why some clients were unable to connect

## [0.81.0] - 2025-01-02

### Added

#### Audio

- Add support for IMA-ADPCM Wave sound files (4-bit compressed sound). Sounds such as sea waves (in the d-day landing mission) and subpen are now played accordingly

#### Client

- Add support for smaller lightmaps (BSP files having `_sml` suffix). Smaller lightmaps are enabled at startup when the texture detail is set to `Low` and if the map supports it (single-player levels of Spearhead and Breakthrough), useful for old low-end hardware.
- Add a new variable to customize the FOV client-side `cg_fov` (defaults to 80), the FOV is no longer an ephemeral server-side setting.
- The FOV is automatically adjusted depending on the aspect ratio

#### UI

- Automatically scale UI and HUD for resolutions above 1920x1080 (support for 2K and above). UI and HUD elements no longer look tiny on 4K resolutions

### Fixed

- The OpenAL library has been updated to version [1.24.1](https://github.com/kcat/openal-soft/releases/tag/1.24.1).

#### Client

- Fix the client getting stuck in the loading screen when disconnected due to an error
- Fix the client loading the map twice when the server restarts and change map immediately
- Fix the first person model not matching the player model

#### General

- Fix letters with diacritic not being parsed nor displayed correctly in Spearhead and Breakthrough.
- Fix a crash that would occur on some custom maps with a lot of grid patches (increased `MAX_PATCH_PLANES`).
- Fix a rare crash that would occur when rendering static models under certain configurations.
- Fix a stack overflow crash that would occur with a name of more than 30 characters.

#### AI

- Fix singleplayer AI not aiming at the enemy while taking cover
- Fix singleplayer AI not dropping weapons if forcedropweapon is set on the sentient and weapon drops are disabled in the level (should be better for Breakthrough single-player maps)
- Fix a random crash that would occur when using multiplayer bots
- Improved multiplayer bot logic
- Fix the match not ending after the clock reached 0:00 in round-based games

#### Game

- Fix doors closing immediately when blocked (this bug also occurs in the original game)
- Fix smoke grenades not reapparing when loading from save
- Fix vehicle tank tandem causing a crash when a single turret was attached
- Make players non-solid when they're a passenger in a vehicle (fixes players getting hit/damaged by the moving vehicle)
- Prevent the player from going out of the maximum world bounds in noclip mode (as in mohaas/mohaab)
- Prevent the player from shooting silently by firing and dropping the weapon at the exact same time (this is trick is possible in the original game)
- When a door is blocked (voluntarily by a player or not), it will now try to open again in the other direction, to make the door not impossible to open (this is also an issue in the original game)

#### Multiplayer

- Fix game messages not having a newline
- Fix letters with accents not displaying in the chat box
- Fix a bug that would make the player die or spawn with no inventory. This occurs when getting killed and respawning immediately (this bug also occurs in the original game)
- Fix an issue where remote clients would lose track of other clients team that have changed their name
- Fix players getting the score from previous maps if they reconnect at the right moment (this bug also occurs in the original game)
- Fix players deaths (or total kills) being added twice in round-based matches
- Fix spawn metrics being terrible in FFA (like players respawning right behind other players)
- Fix teamkill kick not displayed in Spearhead and Breakthrough

#### Server

- Fix the result of the `status` command which prevented tools like `Advanced Security` from getting IP addresses (note: `Advanced Security` doesn't support IPv6 addresses)
- Fix an issue where connecting clients would disconnect with the error `tried to parse cg message without cgame loaded`. This issue occurs in original 1.11, but was fixed in original 2.0 (Spearhead and Breakthrough)

#### UI

- Fix an issue when resizing an in-game window, the window would stop being resized when the mouse was rapidly moving
- Fix huddraw elements being wrongly positioned on wide screen when elements have virtualscale enabled
- Fix pulldown menus being improperly focused and activated at the same time
- Fix the crosshair being too small or hidden when a lower `Texture Detail` value (`r_picmip`) is set
- Properly handle long strings in the chat message box and in the game message box

### Changed

#### General

- Don't execute newconfig.cfg anymore as it overrides graphical settings.
- Some errors that show in console will only print in developer mode now (`developer` set to 1), as those are not relevant for the end-user.
- The `unnamedsoldier.cfg` configuration file will be used as a template for creating the `omconfig.cfg` configuration file if the latter doesn't exist

## [0.80.0] - 2024-11-24

### Added

#### Audio

- Add "tmvolume", so single-player scripts can adjust the music volume.

#### Client

- Add fuse stopwatch (Medal of Honor: Spearhead and Breakthrough feature).
- Add testemitter support for creating emitters.
- Briefings videos are now played in mohaas/mohaab.
- The game intro is now played when the game starts.

#### Game module

- Add a server feature to tweak in-game text messages and instant messages.
- Add a server feature to prevent unbalancing teams.
- Add Breakthrough liberation instant messages.

#### Rendering

- Add dynamic light support for terrains
- Add lighting for smoke FX
- Add support for sunflares and lensflares
- Improved the overall lighting. Objects should now be lit correctly (some issues could still occur)
- JPEG screenshots are now supported using the `screenshotJPEG` command

### Fixed

#### Audio

- Ambient sounds should now play correctly on all maps.
- Fix a crash when the sound quality was set to a value lower to `High` (which means 44 kHz).
- Fix an occurence where some sounds would loop forever. This would occur mostly when loading a saved game.
- Fix background ambient sounds being muted when restarting after the intermission (this is a bug in the original game).
- The ambient sound is now completely reset on each map, which means ambient sounds from previous maps won't be heard (this is a bug in the original game).

#### Client

- Better FX
- Fix a rare crash that would occur when loading a map.
- Fix an issue where delayed effects wouldn't spawn, like the K5 cannon in MOH: Breakthrough, e3l3.
- Fix animations issue when the player dies (weird shaking, etc).
- Fix barrel water/oil leaks not showing.
- Fix bullet holes disappearing on doors.
- Fix emitters leaking, this would cause a massive lag in a server with a lot of players.
- Fix issues with shaky animation in a multiplayer game, like when dying, the animation would repeat and look buggy.
- Fix the client not being able to play on standalone non-dedicated MOH: Breakthrough servers (which would prevent awesome LAN sessions).
- Fix the quick message mode window being shown as a full chat window.
- In mohaas/mohaab mode, team radar icon should now blink when the teammate is talking.
- It's now possible to navigate and play voices from the in-game voice menu.

#### Filesystem

- Fix pk3 ordering: for example, files starting with `zzzzz_name` must take precedence over `zz_name`. This should fix the issue where some mods would unexpectedly take precedence over other mods

#### General

- Fix a crash that could occur when no valid animation exist for a model.
- Fix long subtitles not being displayed.
- Lowered log verbosity to avoid spamming console with useless stuff. Set `developer` to 1 for more verbosity.
- Missing animations are no longer missing.

#### Rendering

- Dynamic lights are now rendered properly and they no longer cause rendering artifacts all over the place
- Fix a crash that would occur in terrain code on some platforms
- Fix static models being incorrectly culled/hidden
- Fix water and oceans being strangely deformed
- Improved the overall lighting. Objects should now be lit correctly (some issues could still occur)
- Sky seams and similar graphical artifacts should no longer be seen on most configurations

#### Server

- Don't send pak names to clients if download (`sv_allowDownload`) is disabled
- Fix animations not playing (like in e2l1) when they are played while the server is starting
- Fix an hang (infinite loop) that would occur when sv_fps is above 1000 (this bug also occurs in the original game)
- Fix entities being sent through skyportals
- Fix `status` returning too long columns for some programs
- Fix velocity being incorrect on ARM systems due to a signed integer overflow
- Reject demo clients on full servers
- Reject non-Breakthrough clients when targeting Breakthrough

#### UI

- Fix a bug where the game would get stuck in the loading screen if the loading screen doesn't have a "continue" button
- Fix bugs in the UI map rotation list when adding/applying
- Improved in-game Notepad support
- The UI should no longer lock up when refreshing/canceling the server list (this bug also occurs in the original game)

### Changed

#### Client

- As a protection, stufftext commands (commands that the server want clients to execute) are now filtered out, only some stufftext commands are allowed to prevent any alteration from a remote server.
- Animations now play smoothly, a lower timescale won't result in a poorly laggy animation anymore (this is an issue in the original game).

#### Game

- Don't allow lean in single-player in the base game.
- Don't count telefrags as teamkills (this is an issue in the original game).
- Fix a bug where the map would not end when the remaining time left reached 0:00 and the fraglimit was set
- Fix a crash that would occur when loading a saved game
- Fix a memory leak issue that can be seen after a long play time, some of the memory leaks also occur in the original game
- Fix AI animations
- Fix AI incorrectly holding a turret too far away
- Fix AI moving incorrectly after loading from save
- Fix AI not turning away from walls
- Fix AI not being able to see through a window frame
- Fix AI running too slowly, and a rare occurence where AIs would get stuck when going down the stairs
- Fix an issue where bullets would not hit anything behind triggers in Allied Assault, like in m2l3 from inside the vent
- Fix an issue where the player would be invulnerable when using heavy weapons, like the Granatwerfer
- Fix an issue where some AIs would not have a speech animation when speaking
- Fix barrel jittering
- Fix errors related to the script lexer & grammar
- Fix Flak88 and Nebelwerfer destroyed models in Spearhead and Breakthrough
- Fix jeep vehicle position on m3l2
- Fix ladder positioning (the issue would only occur on 64-bit systems)
- Fix player position on a vehicle turret gun (like the AA gun, or the Granatwerfer)
- Fix sentients still dropping their weapons even if the level has disabled weapon drops
- Fix some rare mods not loading properly due to an error in the script lexer
- Fix some script models not having any animation after loading from save
- Fix the barrel jittering
- Fix the roof top door on e1l3 (with Gino Corelli) opening when closing the handle (due to an incorrect returned result of a script command)
- Fix vehicle turret gun not making any sounds
- Fix vehicle turret gun being out of sync with the vehicle after loading from save (this issue also occurs in the original game)
- Fix weapon pickups where picking up a weapon with only 1 ammo remaining (like a grenade) would not pick the ammo up (this is a bug in the original game)
- Fix weapprev/weappnext/weapdrop switching to inventory item like binoculars
- Navigation fixes

#### Server

- Clients who are not sent to others (invisible + not sent to snapshots) won't be able to use anything like doors, and won't be able to send instant/voice messages

## [0.70.0] - 2024-09-02

### Added

#### Binaries

- This new version ships with binaries for PowerPC, PowerPC64 and PowerPC64 Little-Endian. Currently experimental and the client may not work correctly.
- For linux binaries, AARCH64 was renamed to ARM64

#### General

- Add a default idle animation for TIKI files not having any animation

#### Rendering

- Add support for DDS textures
- Add morphs: dynamic facial expressions for characters
- Implemented sky portal

#### Sound

- Add OpenAL support:
  - Ambient sound (soundtrack)
  - HRTF can be used on OpenAL by setting the speaker type to `Headphones`
  - Loopsounds
  - Music support (triggered music)
  - Proper sound entity spatialization
  - Streamed sounds - audio files (such as music) are streamed from disk without them being entirely loaded in memory
  - Volume/Pitch variation

#### UI

- Add in-game Notepad (thanks to @pryon)
- Add screenshots for saved games

### Fixed

#### General

- Animation fixes
- Fix a crash related to localization (due to some strings containing a backtick)
- Fix some models having surfaces without a shader
- Fix entity collision
- Fix bad terrain/patch collision with grenades
- Fix improper collision on fences - it's now possible to target flak88 behind barbwires with binoculars
- Fix load/save game crashes
- Networking fixes
- Improved support for big-endian architectures (like PowerPC 32/64 bits and ARM in big-endian mode)

#### Client

- Fix entities being incorrectly attached sometimes

##### CGame module

- Bullet hit effects like snow, water and glass are fixed, as well as bullet tracers and flesh impacts
- Corrected Gewehrgranate viewmodel animations
- Fix occasional crashes due to volumetric smoke
- Fix marks not showing on entities like doors
- Invisible models fix
- Rain/snow should now be properly working
- Various fixes for special FX

#### Server

- Fix configstrings overflow on big maps, like t1l2 on MOH: Spearhead

##### Game module

- Door fixes, like door opening in the wrong direction
- Physics fixes, like grenade bouncing
- Fix AIs: they're less dumb than on previous versions
- Fix the artillery system (ProjectileGenerator) - fixes for t2l3, t2l4...
- Fix game loading/saving having wrong datas
- Fix the scoreboard on FFA and for spectators
- Fix the script engine - all scripts must be working now
- Fix an issue with spawnpoints where the player wouldn't spawn at the correct spot
- The mine detector correctly detect mines, in single-player
- The mission date (such as "JUNE 6, 1944") is now displayed
- The vehicle system is now fully working properly: no more vehicles getting stuck
- Singleplayer maps are all completable now (although some rare bugs may occur)

#### Rendering

- Fix crashes related to terrain
- Fix crashes related to `vid_restart` and `snd_restart`
- Fix strange lighting when `Full entity lighting` is disabled
- Lighting related fixes
- Most shaders should correctly load

#### UI

- Fix crashes related to UI
- Fix `vid_restart` breaking the UI
- Fix the multiplayer map picker displaying the same maps twice
- Player model picker should now be displaying characters properly
- Properly show the "disconnected" menu when the client get disconnected from a server

## [0.61.0] - 2024-01-08

### Added

#### Game Module

- Landmines are now fully working (a **Medal of Honor: Allied Assault Breakthrough** feature)
- Script command documentation can now be exported properly using `dumpallclasses` command

#### Client

- Add single-player map list and multiplayer map list
- Add map rotation list for multiplayer
- Add player model selection list
- Add LAN game searcher
- Add subtitles, centerprint, and locationprint
- Add the miniconsole
- Binds can now be set

### Fixed

#### Game module

- Crash fixes
- Fix callvote with invalid strings
- Fix grenade crash
- Fix weapons being attached to the wrong player's hand under some circumstances
- Players should now get the correct loadout in multiplayer
- `sv_fps` with a value higher than 20 shouldn't cause issues anymore with animations

#### Client

- 3D player models are now shown in the main menu screen
- Configstring can now be handled in multiple chunks
- Fix mohaas server version mismatch in the server list when using mohaab target game
- Fix vote menu not working
- Fix UI sounds not working
- Fix locationprint and centerprint not working

## [0.60.2] - 2023-11-29

### Fixed

#### Game Module
- Fix pain animations
- Fix portable turret animations
- Fix game crashing when saving and/or loading from save

#### ClientGame Module
- Fix for the client game module not hiding the stats screen

#### Server
- Fix level file archive

## [0.60.1] - 2023-11-15

### Changed

- The carryable turret (packed MG42 turret) can now be placed

### Fixed

- Fix bullet tracers, they should now be working correctly
- Fix a bug where player could get stuck in turret mode after the turret was removed

## [0.60.0] - 2023-11-15

### Added

- Add non-pvs feature (hide players from other clients who cannot see them)
- Add more SH and BT features
- Add ioq3 server features
- Mostly implemented AI Actor (still buggy)
- Implemented Vehicle and TurretGun from BT 2.40
- Implemented voting system

### Changed

- Configs are now stored in the homepath
- Binaries are now stored in the root folder (no need to replace existing MOH binaries anymore)
- Crashes fixes
- Rendering fixes, client fixes, game fixes and script engine fixes

## [0.58.1] - 2023-08-21

### Fixed

- Fix demo recordings not working with different protocols
- Publish `sv_fps` in server info variables

## [0.58.0] - 2023-08-20

### Added

- Multiplayer bot support
- Server browser

#### Spearhead and Breakthrough features

- Animated farplanes
- Bullets through wood/metal
- Smokes
- Sprint
- Compass radar (icon of teammates on the compass)
- Landmines
- Tug-of-War and Liberation game types
- ...

### Fixed

- Fix bugs, and crashes
- Fix training map not working
- The script engine should now be fully compatible with mohaa scripts

## [0.57.0] - 2023-07-25

### Added

#### Renderer

- Sphere lights for entities
- Light grid entity lighting
- Light grid support from Spearhead and Breakthrough

#### Game

- Allow/disallow leaning while moving (Spearhead and Breakthrough)

## [0.56.0] - 2023-07-23

### Added

- Add bullets, bullet trails, emitters, explosions, marks, model attachment, special effects, viewmodel anim
- Add fog, font, dynamic lights, shader fade, sprite, trees

### Changed

- Use a single executable to play on mohaa, moh_spearhead or moh_breakthrough servers, by starting openmohaa with `+set com_target_game x` commandline (check README)

### Fixed

- Fix disconnect menu not working
- Fix compass not working
- Fix crashes

## [0.55.3] - 2023-06-27

## [0.55.2] - 2023-06-27

### Added

- Add Breakthrough-compatible executable

## [0.55.1] - 2023-06-24

## [0.54.0] - 2023-05-30

### Added

- ioquake3 backport
- SDL2 support, which means input, rendering and sound improvements
- Better console support (using OS console host, rather than a custom Windows 95 console)
- Rendering improvement: sprite support, light glow support (light coronas), more shaders support, minor fixes, and beam fixes
- Sound improvement: MP3 support + better sound/format support
- Client game: better emitter support, scoreboard fixes, kar98 overlay fix
- Networking improvement
- UI improvement: using Spearhead/Breakthrough gmbox/dmbox placement

### Fixed

- Fix compass scale

## [0.53.2] - 2023-05-23

### Added

- ARM support
- DDS texture support
- Death messages on Spearhead/Breakthrough

### Fixed

- More SH/BT fixes
- Networking fixes
- UI/HUD fixes
- Weapon binding fixes

## [0.53.1] - 2023-05-22

### Fixed

- Crashes fixes
- Minor fixes
- Rendering fixes (especially for menus in SH/BT)
- Scoreboard fixes for SH/BT

## [0.53.0] - 2023-05-21

### Added

- Add support for Spearhead (TA/Team Assault) and Breakthrough (TT/Team Tactics) server/client

### Fixed

- Bug fixes

## [0.52.0] - 2023-05-21

### Changed

- Use semantic version number

### Fixed

- Rendering fixes
- UI fixes

## [0.51a] - 2023-05-20

### Fixed

- Crash fixes
- Lighting tweaks
- Performance fixes

## [0.51] - 2023-05-20

### Added

- Loading screens
- Scoreboard
- Weapon buttons

## [0.50a] - 2023-05-19

### Added

- Fog support

### Fixed

- Fix static model

## [0.50] - 2023-05-18

### Added

- Almost full LOD terrain support
- Basic sound implementation
- Better multi-texture support

## [0.49f] - 2023-05-18

### Added

- Client binaries

## [0.49e] - 2023-02-05

## [0.49d] - 2023-02-05

## [0.49c] - 2023-02-03

## [0.49b] - 2023-01-30

## [0.49] - 2023-01-29

### Added

- This is the first public release on GitHub with CMake support. Dedicated-only binaries.

[unreleased]: https://github.com/openmoh/openmohaa/compare/v0.82.1...HEAD
[0.82.1]: https://github.com/openmoh/openmohaa/compare/v0.82.0...v0.82.1
[0.82.0]: https://github.com/openmoh/openmohaa/compare/v0.81.1...v0.82.0
[0.81.1]: https://github.com/openmoh/openmohaa/compare/v0.81.0...v0.81.1
[0.81.0]: https://github.com/openmoh/openmohaa/compare/v0.80.0...v0.81.0
[0.80.0]: https://github.com/openmoh/openmohaa/compare/v0.70.0...v0.80.0
[0.70.0]: https://github.com/openmoh/openmohaa/compare/v0.61.0...v0.70.0
[0.61.0]: https://github.com/openmoh/openmohaa/compare/v0.60.2...v0.61.0
[0.60.2]: https://github.com/openmoh/openmohaa/compare/v0.60.1...v0.60.2
[0.60.1]: https://github.com/openmoh/openmohaa/compare/v0.60.0...v0.60.1
[0.60.0]: https://github.com/openmoh/openmohaa/compare/v0.58.0...v0.60.0
[0.58.1]: https://github.com/openmoh/openmohaa/compare/v0.58.0...v0.58.1
[0.58.0]: https://github.com/openmoh/openmohaa/compare/v0.57.0...v0.58.0
[0.57.0]: https://github.com/openmoh/openmohaa/compare/v0.56.0...v0.57.0
[0.56.0]: https://github.com/openmoh/openmohaa/compare/v0.55.3...v0.56.0
[0.55.3]: https://github.com/openmoh/openmohaa/compare/v0.55.2...v0.55.3
[0.55.2]: https://github.com/openmoh/openmohaa/compare/v0.55.1...v0.55.2
[0.55.1]: https://github.com/openmoh/openmohaa/compare/v0.54.0...v0.55.1
[0.54.0]: https://github.com/openmoh/openmohaa/compare/v0.53.2...v0.54.0
[0.53.2]: https://github.com/openmoh/openmohaa/compare/v0.53.1...v0.53.2
[0.53.1]: https://github.com/openmoh/openmohaa/compare/v0.53.0...v0.53.1
[0.53.0]: https://github.com/openmoh/openmohaa/compare/v0.52.0...v0.53.0
[0.52.0]: https://github.com/openmoh/openmohaa/compare/0.51a...v0.52.0
[0.51a]: https://github.com/openmoh/openmohaa/compare/0.51...0.51a
[0.51]: https://github.com/openmoh/openmohaa/compare/0.50a...0.51
[0.50a]: https://github.com/openmoh/openmohaa/compare/0.50...0.50a
[0.50]: https://github.com/openmoh/openmohaa/compare/0.49f...0.50
[0.49f]: https://github.com/openmoh/openmohaa/compare/0.49e...0.49f
[0.49e]: https://github.com/openmoh/openmohaa/compare/0.49d...0.49e
[0.49d]: https://github.com/openmoh/openmohaa/compare/0.49c...0.49d
[0.49c]: https://github.com/openmoh/openmohaa/compare/0.49b...0.49c
[0.49b]: https://github.com/openmoh/openmohaa/compare/0.49...0.49b
[0.49]: https://github.com/openmoh/openmohaa/releases/tag/0.49
