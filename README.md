# OpenMoHAA

[![Build](https://github.com/openmoh/openmohaa/actions/workflows/branches-build.yml/badge.svg?branch=main)](https://github.com/openmoh/openmohaa/actions/workflows/branches-build.yml) [![Release](https://img.shields.io/github/v/release/openmoh/openmohaa)](https://github.com/openmoh/openmohaa/releases) [![Downloads](https://img.shields.io/github/downloads/openmoh/openmohaa/total)](https://github.com/openmoh/openmohaa/releases)

![License](https://img.shields.io/github/license/openmoh/openmohaa) ![Commits](https://img.shields.io/github/commit-activity/t/openmoh/openmohaa)

![Discord](https://img.shields.io/discord/596049712579215361?logo=discord&logoColor=white&color=5865F2)

![logo](misc/openmohaa-text-sm.png)

## Intro

The main goal of OpenMoHAA is to ensure the future and continuity of **Medal of Honor: Allied Assault** (including Spearhead and Breakthrough expansions) on all platforms/architectures by providing new patches/features. Thanks to the ioquake3 project and the F.A.K.K SDK, OpenMoHAA has already reached more than half of its goal: an open-source version of MoH:AA (based on version 2.40) that is fully compatible with both the original game and expansions, in terms of network protocol, assets, and scripts. OpenMoHAA is compatible with both MOH:AA servers and clients (Spearhead and Breakthrough included).

## Getting started

- [Downloading and installing OpenMoHAA](docs/getting_started_installation.md)
- [Running OpenMoHAA and using expansion assets](docs/getting_started_running.md)
- [Game configuration](docs/configuration.md)

If you encounter any issues, please refer to the [FAQ](docs/faq.md) for possible solutions.

## Current state

### Single-player

The entire single-player campaign should work (Allied Assault, Spearhead and Breakthrough). If you encounter any bug, please create a new [GitHub issue](https://github.com/openmoh/openmohaa/issues) describing them.

### Multiplayer

The Multiplayer part is almost fully stable, all game modes including `Tug-of-War` from Medal of Honor: Spearhead, and `Liberation` from Medal of Honor: Breakthrough are implemented and are working as intended. Mods like `Freeze-Tag` are working as well.

OpenMoHAA can be used to [host](docs/getting_started_installation.md#server) a mohaa/mohaas/mohaab server and players can use OpenMoHAA to [play](docs/getting_started_installation.md#client) on any server. If some bugs/strange behavior occurs, the best way to report them is by [creating a new issue](https://github.com/openmoh/openmohaa/issues) on the [OpenMoHAA GitHub](https://github.com/openmoh/openmohaa) project with the steps to reproduce (with eventually the problematic mods).

One of the big multiplayer features are bots. They were introduced for testing and for entertainment purposes so players can have fun alone. See [Configuring bots](docs/configuration/server.md#bots) in the dedicated server configuration documentation to set them up.

## Features

- [List of new features](docs/features.md)
- [Scripting](docs/scripting.md)

## Reporting issues

If you encounter a bug, report it by creating a new [issue](https://github.com/openmoh/openmohaa/issues). Make sure to select the `Bug report` template and fill in the appropriate fields.

If you need help or have a problem, you can head over to the [discussions](https://github.com/openmoh/openmohaa/discussions) or join the OpenMoHAA Discord server (the link is at the bottom of this README).

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
