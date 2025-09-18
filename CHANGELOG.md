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

#### General

- Added the bot count (if there are any) in the server browser. This will only work for servers running at least the version **0.83.0** of OpenMoHAA. The bot count will also show on the [333networks](https://master.333networks.com/) master server website.

### Fixed

#### Server

- Allow `g_teamkillkick` to be set to 0. Players would get kicked when this variable was set to 0 (also occurs in the original game).
- Fix a possible crash caused by scripts using `angles_pointat` (also occurs in the original game).
- Fix Actors not doing anything without client #0 (also occurs in the original game).
- Fix death animation movement for bots.
- Fix improper weapon falling rotation.
- Fix players temporarily losing user input control on ARM-based OpenMoHAA servers when a message contained a special ASCII character.
- Fix some RCON programs not working with `dumpuser`.
- Fix the round limit being always `15` on Tug-of-War.
- The player would spawn with no weapons on single-player when `sv_maxbots` was set (#783).

#### Clients

- Fix not being able to view sniper zoom when spectating.
- Fix the issue where the network would no longer work after hosting a multiplayer game on Allied Assault using the client.

### Changed

- Completely refactor CMake build system (from [ioquake3's](<https://github.com/ioquake/ioq3>) new CMake build system).

## [0.82.1] - 2025-08-05

### Fixed

#### Crashes

- Fixed a crash that would occur when loading custom maps in some map packs (due to a UI incorrectly declaring a widget: loadingbar, twice) (#764)
- Fixed an issue where the server could crash MOHAA base (1.11) clients in certain situations (#778)
- The game no longer terminate with error `FS_ReadFile with empty name`. (#772)

#### Drawing & rendering

- Fixed force model always using an allied skin for dead bodies.
- Fixed HUD being shown during briefings (#767)
- Fixed in-game mouse cursor disappearing on some platforms (#765)
- Set a more distant fog for d-day by default (`set g_ddayfog 0`). This fixes an issue with sky artifacts in m3l1a. (#770)

#### Multiplayer

- Fixed frequent telefrags on servers with a lot of players, like 25 players.
- Fixed taunts being played as team rather than global (#775)

#### Miscellaneous

- Update third-party libraries like jpeg and zlib.

## [0.82.0] - 2025-08-05

### Added

#### General

- Commands `add`, `subtract`, `scale`, `append` and `bitset`. `bitset`, which comes from mohaab, is particulary useful to set flags for flag-based variables like `dmflags`.
- Logfile timestamping. It's enabled by default with the `com_logfile_timestamps` cvar. The current time is displayed at the beginning of each log entry.
- Setup files (.msi packages) are now distributed for Windows.
- Update check. The game will verify the latest version in the background (by querying the latest GitHub release of the repository) and print a 
- Added the **Instant Action** feature from Breakthrough. This feature, although only usable through the Breakthrough UI, allows anyone to quickly find and join popular low-ping multiplayer servers.

#### Multiplayer

- Added `(all)` prefix in text message when a player is talking to all
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
- Fixed enhanced audio spatialization being forcefully enabled when it shouldn't for audio devices detected as headphones that aren't headphones.

#### General

- Fixed `LOCALIZATION ERROR` spam.
- Fixed an infinite loop that would occur during name sanitization.
- Fixed a crash that would occur when using special menus (like admin menus).
- Fixed memory leaks after the process has been running for a long time.
- Fixed player name being cutoff at the first negative character
- Fixed player being able to shoot bullets from under the LOD terrain (also occurs on the original game)
- Fixed the `version` string, ensuring most software such as HLSW and mods work as intended depending on the target game version.

#### Client

- Fixed bullet impacts having multiple effects sometimes.
- Fixed cinematic not stopping on map change.
- Fixed delayed SFX (such as smoke sound) being played on unrelated entities
- Fixed `Illegible server message` error when firing some type of bullets on Spearhead and Breakthrough.
- Fixed lean angle slightly not matching the original game.
- Fixed localization issues.
- Fixed Waffen skins not being usable on servers
- Player models (selected skins) will be reset to their default value in-game if it doesn't exist, and force models will not work if the model doesn't exist on the server.
- Reallow client `name` to be changed by the server. Names are often changed by administrators to fix duplicate names.

#### Multiplayer

- Correctly print the text of instant messages in the console/logfile.
- Fixed AllocEdict when some Breakthrough maps are started in multiplayer mode (for co-op).
- Fixed a crash related to spawnpoints.
- Fixed a crash that would occur when playing a sound with subtitle without player #0.
- Fixed a crash that occured when a player reconnects to the same slot (after timing out) and then displays the scoreboard
- Fixed an issue preventing spawning next to spawn points with an offset. This lead to frequent telefrags with lot of players.
- Fixed a crash that would occur when the player sets a first-person model as a skin, causing an infinite state loop (also occurs in the original game).
- Fixed clients joining during intermission and having a corrupt `Time` value in the scoreboard (also occurs in the original game).
- Fixed fog sometimes not updating on maps such as Gewitter.
- Fixed incorrect number of wins shown for allies and axis in the scoreboard.
- Fixed spawnpoints metrics.
- Fixed missing player starts on some maps.
- Fixed players temporarily having the name of another player when rejoining after losing connection (also occurs in original game)
- Fixed projectiles with no owner getting removed when a player joins a team (also occurs in Spearhead and Breakthrough)
- Fixed score not being able to have negative values in the result returned by the `status` server command.
- Fixed scores being incorrectly sorted in round-based matches (they must be sorted by total kills).
- Fixed some issues with laser mods, like the `Freeze-Tag` mode where the laser would not trace from the player sight.
- Fixed `sv_precache` being ignored.
- Fixed the issue where the text message gets empty when exceeding character limit.
- Fixed the round ending incorrectly in mods like beach soccer.
- Fixed weapon being dropped sometimes when switching weapons too fast (also occurs in original game)
- Increase text message character limit to 400 characters (taking the name of the player into account).

#### Game module

- Bazooka was never showing as the preferred weapon in singleplayer.
- Fixed AI not shooting when squadmates are next to eachother in Spearhead and Breakthrough (also occurs in original game).
- Fixed AI not moving to specific location when ordered.
- Fixed AI misbehaving when curious.
- Fixed AI moving too fast on Allied Assault.
- Fixed AI having a notepad.
- Fixed ammo type not properly localized.
- Fixed an infinite loop that would occur on some maps when a vehicle start driving to a path.
- Fixed an issue where legs animations (like idle pistol stand) would not loop.
- Fixed an undefined behavior issue in some mods that could lead to infinite loop, like, `while (self.variable)` (where `self` is `NULL`).
- Fixed a crash (MAX_GAMESTATE_CHARS) and a memory leak in long matches after many models are loaded. This would occur mostly in ToW maps during long matches, or in maps spawning many `Actor`.
- Fixed a crash related to VehicleTurretGun having no vehicle owner (also occurs in original mohaab).
- Fixed a crash that would occur when a turret shoots projectiles.
- Fixed a crash when a player quits the server while in a vehicle (also occurs in original game).
- Fixed damage blend not working in Spearhead and Breakthrough.
- Fixed leaning not making a difference when trying to use.
- Fixed mods like Elgbot server-side not working properly.
- Fixed inconsistent cross-blending of torso animations.
- Fixed "silent walk", where rapidly switching weapons would prevent emitting footstep sounds (also occurs in original game).
- Fixed weapon banning not working properly when banning rifles.

#### Rendering

- Fixed a crash that would occur when there are insufficient triangles for LOD terrain tessellation (also occurs in original game).
- Fixed a possible crash when loading a map with lightmaps and then loading a map that has no lightmaps.
- Fixed a rare crash that would occur when toggling the scoreboard (also occurs in original game, but was fixed in Breakthrough 2.30).
- Fixed blurry text.
- Fixed disconnected water lakes
- Fixed console prompt being hidden when there are too many characters
- Fixed line breaks occurring mid-word
- Fixed long messages being cut off in the console
- Fixed omaha beach shorelines being incorrectly rendered
- Fixed the server list being requested twice on Allied Assault when browsing internet servers.

#### Server

- Fixed `CM_AdjustAreaPortalState` error
- Fixed clients rejected for too low/high ping not being able to reconnect again.
- Fixed some programs (like gamedig) not being able to retrieve gamespy data from the server.
- Fixed some programs not being able to retrieve the complete list of players
- Fixed `SV_FindIndex: overflow` errors that would occur more often than the original game during long matches.
- Fixed server crashing all clients under rare occasions.

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

- Fixed the text of pulldown elements being invisible on some menus
- Fixed the server not taking the client rate into account and transmitting network fragments too fast, this may be the reason why some clients were unable to connect

## [0.81.0] - 2025-01-02

### Added

#### Audio

- Added support for IMA-ADPCM Wave sound files (4-bit compressed sound). Sounds such as sea waves (in the d-day landing mission) and subpen are now played accordingly

#### Client

- Added support for smaller lightmaps (BSP files having `_sml` suffix). Smaller lightmaps are enabled at startup when the texture detail is set to `Low` and if the map supports it (single-player levels of Spearhead and Breakthrough), useful for old low-end hardware.
- Added a new variable to customize the FOV client-side `cg_fov` (defaults to 80), the FOV is no longer an ephemeral server-side setting.
- The FOV is automatically adjusted depending on the aspect ratio

#### UI

- Automatically scale UI and HUD for resolutions above 1920x1080 (support for 2K and above). UI and HUD elements no longer look tiny on 4K resolutions

### Fixed

- The OpenAL library has been updated to version [1.24.1](https://github.com/kcat/openal-soft/releases/tag/1.24.1).

#### Client

- Fixed the client getting stuck in the loading screen when disconnected due to an error
- Fixed the client loading the map twice when the server restarts and change map immediately
- Fixed the first person model not matching the player model

#### General

- Fixed letters with diacritic not being parsed nor displayed correctly in Spearhead and Breakthrough.
- Fixed a crash that would occur on some custom maps with a lot of grid patches (increased `MAX_PATCH_PLANES`).
- Fixed a rare crash that would occur when rendering static models under certain configurations.
- Fixed a stack overflow crash that would occur with a name of more than 30 characters.

#### AI

- Fixed singleplayer AI not aiming at the enemy while taking cover
- Fixed singleplayer AI not dropping weapons if forcedropweapon is set on the sentient and weapon drops are disabled in the level (should be better for Breakthrough single-player maps)
- Fixed a random crash that would occur when using multiplayer bots
- Improved multiplayer bot logic
- Fixed the match not ending after the clock reached 0:00 in round-based games

#### Game

- Fixed doors closing immediately when blocked (this bug also occurs in the original game)
- Fixed smoke grenades not reapparing when loading from save
- Fixed vehicle tank tandem causing a crash when a single turret was attached
- Make players non-solid when they're a passenger in a vehicle (fixes players getting hit/damaged by the moving vehicle)
- Prevent the player from going out of the maximum world bounds in noclip mode (as in mohaas/mohaab)
- Prevent the player from shooting silently by firing and dropping the weapon at the exact same time (this is trick is possible in the original game)
- When a door is blocked (voluntarily by a player or not), it will now try to open again in the other direction, to make the door not impossible to open (this is also an issue in the original game)

#### Multiplayer

- Fixed game messages not having a newline
- Fixed letters with accents not displaying in the chat box
- Fixed a bug that would make the player die or spawn with no inventory. This occurs when getting killed and respawning immediately (this bug also occurs in the original game)
- Fixed an issue where remote clients would lose track of other clients team that have changed their name
- Fixed players getting the score from previous maps if they reconnect at the right moment (this bug also occurs in the original game)
- Fixed players deaths (or total kills) being added twice in round-based matches
- Fixed spawn metrics being terrible in FFA (like players respawning right behind other players)
- Fixed teamkill kick not displayed in Spearhead and Breakthrough

#### Server

- Fixed the result of the `status` command which prevented tools like `Advanced Security` from getting IP addresses (note: `Advanced Security` doesn't support IPv6 addresses)
- Fixed an issue where connecting clients would disconnect with the error `tried to parse cg message without cgame loaded`. This issue occurs in original 1.11, but was fixed in original 2.0 (Spearhead and Breakthrough)

#### UI

- Fixed an issue when resizing an in-game window, the window would stop being resized when the mouse was rapidly moving
- Fixed huddraw elements being wrongly positioned on wide screen when elements have virtualscale enabled
- Fixed pulldown menus being improperly focused and activated at the same time
- Fixed the crosshair being too small or hidden when a lower `Texture Detail` value (`r_picmip`) is set
- Properly handle long strings in the chat message box and in the game message box

### Changed

#### General

- Don't execute newconfig.cfg anymore as it overrides graphical settings.
- Some errors that show in console will only print in developer mode now (`developer` set to 1), as those are not relevant for the end-user.
- The `unnamedsoldier.cfg` configuration file will be used as a template for creating the `omconfig.cfg` configuration file if the latter doesn't exist

## [0.80.0] - 2024-11-24

### Added

#### Audio

- Added "tmvolume", so single-player scripts can adjust the music volume.

#### Client

- Added fuse stopwatch (Medal of Honor: Spearhead and Breakthrough feature).
- Added testemitter support for creating emitters.
- Briefings videos are now played in mohaas/mohaab.
- The game intro is now played when the game starts.

#### Game module

- Added a server feature to tweak in-game text messages and instant messages.
- Added a server feature to prevent unbalancing teams.
- Added Breakthrough liberation instant messages.

#### Rendering

- Added dynamic light support for terrains
- Added lighting for smoke FX
- Added support for sunflares and lensflares
- Improved the overall lighting. Objects should now be lit correctly (some issues could still occur)
- JPEG screenshots are now supported using the `screenshotJPEG` command

### Fixed

#### Audio

- Ambient sounds should now play correctly on all maps.
- Fixed a crash when the sound quality was set to a value lower to `High` (which means 44 kHz).
- Fixed an occurence where some sounds would loop forever. This would occur mostly when loading a saved game.
- Fixed background ambient sounds being muted when restarting after the intermission (this is a bug in the original game).
- The ambient sound is now completely reset on each map, which means ambient sounds from previous maps won't be heard (this is a bug in the original game).

#### Client

- Better FX
- Fixed a rare crash that would occur when loading a map.
- Fixed an issue where delayed effects wouldn't spawn, like the K5 cannon in MOH: Breakthrough, e3l3.
- Fixed animations issue when the player dies (weird shaking, etc).
- Fixed barrel water/oil leaks not showing.
- Fixed bullet holes disappearing on doors.
- Fixed emitters leaking, this would cause a massive lag in a server with a lot of players.
- Fixed issues with shaky animation in a multiplayer game, like when dying, the animation would repeat and look buggy.
- Fixed the client not being able to play on standalone non-dedicated MOH: Breakthrough servers (which would prevent awesome LAN sessions).
- Fixed the quick message mode window being shown as a full chat window.
- In mohaas/mohaab mode, team radar icon should now blink when the teammate is talking.
- It's now possible to navigate and play voices from the in-game voice menu.

#### Filesystem

- Fixed pk3 ordering: for example, files starting with `zzzzz_name` must take precedence over `zz_name`. This should fix the issue where some mods would unexpectedly take precedence over other mods

#### General

- Fixed a crash that could occur when no valid animation exist for a model.
- Fixed long subtitles not being displayed.
- Lowered log verbosity to avoid spamming console with useless stuff. Set `developer` to 1 for more verbosity.
- Missing animations are no longer missing.

#### Rendering

- Dynamic lights are now rendered properly and they no longer cause rendering artifacts all over the place
- Fixed a crash that would occur in terrain code on some platforms
- Fixed static models being incorrectly culled/hidden
- Fixed water and oceans being strangely deformed
- Improved the overall lighting. Objects should now be lit correctly (some issues could still occur)
- Sky seams and similar graphical artifacts should no longer be seen on most configurations

#### Server

- Don't send pak names to clients if download (`sv_allowDownload`) is disabled
- Fixed animations not playing (like in e2l1) when they are played while the server is starting
- Fixed an hang (infinite loop) that would occur when sv_fps is above 1000 (this bug also occurs in the original game)
- Fixed entities being sent through skyportals
- Fixed `status` returning too long columns for some programs
- Fixed velocity being incorrect on ARM systems due to a signed integer overflow
- Reject demo clients on full servers
- Reject non-Breakthrough clients when targeting Breakthrough

#### UI

- Fixed a bug where the game would get stuck in the loading screen if the loading screen doesn't have a "continue" button
- Fixed bugs in the UI map rotation list when adding/applying
- Improved in-game Notepad support
- The UI should no longer lock up when refreshing/canceling the server list (this bug also occurs in the original game)

### Changed

#### Client

- As a protection, stufftext commands (commands that the server want clients to execute) are now filtered out, only some stufftext commands are allowed to prevent any alteration from a remote server.
- Animations now play smoothly, a lower timescale won't result in a poorly laggy animation anymore (this is an issue in the original game).

#### Game

- Don't allow lean in single-player in the base game.
- Don't count telefrags as teamkills (this is an issue in the original game).
- Fixed a bug where the map would not end when the remaining time left reached 0:00 and the fraglimit was set
- Fixed a crash that would occur when loading a saved game
- Fixed a memory leak issue that can be seen after a long play time, some of the memory leaks also occur in the original game
- Fixed AI animations
- Fixed AI incorrectly holding a turret too far away
- Fixed AI moving incorrectly after loading from save
- Fixed AI not turning away from walls
- Fixed AI not being able to see through a window frame
- Fixed AI running too slowly, and a rare occurence where AIs would get stuck when going down the stairs
- Fixed an issue where bullets would not hit anything behind triggers in Allied Assault, like in m2l3 from inside the vent
- Fixed an issue where the player would be invulnerable when using heavy weapons, like the Granatwerfer
- Fixed an issue where some AIs would not have a speech animation when speaking
- Fixed barrel jittering
- Fixed errors related to the script lexer & grammar
- Fixed Flak88 and Nebelwerfer destroyed models in Spearhead and Breakthrough
- Fixed jeep vehicle position on m3l2
- Fixed ladder positioning (the issue would only occur on 64-bit systems)
- Fixed player position on a vehicle turret gun (like the AA gun, or the Granatwerfer)
- Fixed sentients still dropping their weapons even if the level has disabled weapon drops
- Fixed some rare mods not loading properly due to an error in the script lexer
- Fixed some script models not having any animation after loading from save
- Fixed the barrel jittering
- Fixed the roof top door on e1l3 (with Gino Corelli) opening when closing the handle (due to an incorrect returned result of a script command)
- Fixed vehicle turret gun not making any sounds
- Fixed vehicle turret gun being out of sync with the vehicle after loading from save (this issue also occurs in the original game)
- Fixed weapon pickups where picking up a weapon with only 1 ammo remaining (like a grenade) would not pick the ammo up (this is a bug in the original game)
- Fixed weapprev/weappnext/weapdrop switching to inventory item like binoculars
- Navigation fixes

#### Server

- Clients who are not sent to others (invisible + not sent to snapshots) won't be able to use anything like doors, and won't be able to send instant/voice messages

[unreleased]: https://github.com/openmoh/openmohaa/compare/v0.82.1...HEAD
[0.82.1]: https://github.com/openmoh/openmohaa/compare/v0.82.0...v0.82.1
[0.82.0]: https://github.com/openmoh/openmohaa/compare/v0.81.1...v0.82.0
[0.81.1]: https://github.com/openmoh/openmohaa/compare/v0.81.0...v0.81.1
[0.81.0]: https://github.com/openmoh/openmohaa/compare/v0.80.0...v0.81.0
[0.80.0]: https://github.com/openmoh/openmohaa/releases/tag/v0.80.0
