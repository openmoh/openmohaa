# OpenMoHAA

![Build](https://github.com/openmoh/openmohaa/actions/workflows/build-cmake.yml/badge.svg?branch=main) [![Release](https://img.shields.io/github/v/release/openmoh/openmohaa)](https://github.com/openmoh/openmohaa/releases) [![Downloads](https://img.shields.io/github/downloads/openmoh/openmohaa/total)](https://github.com/openmoh/openmohaa/releases)

![License](https://img.shields.io/github/license/openmoh/openmohaa) ![Commits](https://img.shields.io/github/commit-activity/t/openmoh/openmohaa)

![Discord](https://img.shields.io/discord/596049712579215361?logo=discord&logoColor=white&color=5865F2)


```
     / _ \ _ __   ___ _ __ |  \/  |/ _ \| | | |  / \      / \
    | | | | '_ \ / _ \ '_ \| |\/| | | | | |_| | / _ \    / _ \
    | |_| | |_) |  __/ | | | |  | | |_| |  _  |/ ___ \  / ___ \
     \___/| .__/ \___|_| |_|_|  |_|\___/|_| |_/_/   \_\/_/   \_\
          |_|
```

## Current state

### Multiplayer

The Multiplayer part is almost fully stable.

Server admins can use OpenMoHAA to [host](#server) a mohaa/mohaas/mohaab server. Bugs must be reported by creating a new [issue](https://github.com/openmoh/openmohaa/issues) on the OpenMoHAA GitHub project. Players can use OpenMoHAA to [play](#client) on a mohaa/mohaas/mohaab server.

All game modes including `Tug-of-War` from Medal of Honor: Spearhead, and `Liberation` from Medal of Honor: Breakthrough are implemented and are working as intended. Mods like `Freeze-Tag` are working as well.

### Single-player

The `training` map can be played from start to end. The single-player campaign is not fully functional, but at least AI can breathe, talk, run, and shoot.

## Intro

The main goal of OpenMoHAA is to ensure the future and continuity of **Medal of Honor: Allied Assault**. It has always been a dream in the community to provide patches and security fixes for the game. Thanks to the ioquake3 project, F.A.K.K SDK and other quality tools, OpenMoHAA has already reached more than half of its goal: to create an open-source version of MoH:AA (based on version 2.40) that is fully compatible with the original game (in terms of protocol, assets, and scripts).

## Getting started

- [Installing OpenMoHAA](docs/getting_started_installation.md)
- [Running OpenMoHAA and using expansion assets](docs/getting_started_running.md)

### Playing with bots

OpenMoHAA has a basic bot system that emulates real players. The maximum number of bots is defined by the `sv_maxbots` variable, and the number of initial bots is defined by the `sv_numbots` variable. Bots can also be added or removed using the `addbot` and `removebot` commands.

This feature is a great way to test the gameplay and mods.

## Features

- [What's working and what's not](docs/features_implementation.md)
- [New features](docs/features.md)
- [Scripting commands](docs/features_g_allclasses.html)

## Compiling

- See [Compiling OpenMoHAA](docs/compiling.md)

## Screenshots

|                                                                                   |                                                                            |
|-----------------------------------------------------------------------------------|----------------------------------------------------------------------------|
| ![](docs/images/v0.60.0-x86_64/mohdm1_1.png)                                      | ![](docs/images/v0.60.0-x86_64/training_1.png)                               |
| ![](docs/images/v0.60.0-x86_64/flughafen_1.png)                                   | ![](docs/images/v0.60.0-x86_64/flughafen_2.png)                            |
| ![](docs/images/v0.60.0-x86_64/mohdm2_1.png "Playing Freeze-Tag mode with bots")  | ![](docs/images/v0.60.0-x86_64/training_3.png "Single-Player training")    |

*More screenshots [here](docs/images)*

## Third party

### SDL

http://www.libsdl.org/

### OpenAL

https://www.openal.org/

### LibMAD

http://www.underbit.com/products/mad/

### cURL

https://curl.se/

### Libogg

https://github.com/gcp/libogg

### Libvorbis

https://xiph.org/vorbis/

### Libopus

https://opus-codec.org/

## Communities

### URLs

- https://github.com/openmoh/openmohaa/
- https://mohaaaa.co.uk/AAAAMOHAA/index.php
- https://x-null.net/

### Discord

[![Discord Banner 1](https://discordapp.com/api/guilds/596049712579215361/widget.png?style=banner2)](https://discord.gg/NYtH58R)
