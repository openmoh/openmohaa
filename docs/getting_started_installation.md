# Installation

Ensure that you download the binaries compatible with your platform in the release section.

## Requirements

- A valid MOH:AA installation, a MOH:AA Warchest copy can be acquired from GOG [here](https://www.gog.com/en/game/medal_of_honor_allied_assault_war_chest)
- On Windows, Microsoft Visual C++ 2015/2017/2019/2022 Redistributable from https://learn.microsoft.com/en-US/cpp/windows/latest-supported-vc-redist?view=msvc-170 may be required.

## System requirements

This matches the hardware requirements (x86) of the original game:

|Hardware|Minimum
|--------|-------
|CPU     |450 MHz
|Memory  |128MB
|Storage |1.2GB
|GPU     |16MB

## Client

a) extract archive to your MOHAA installation directory.

-or-

b) extract archive somewhere on your hard drive, create a shortcut to openmohaa (or omohaaded), and set the start directory on the shortcut to your MOHAA installation directory.

## Server

Extract the archive to your MOHAA server installation directory. Then proceed like you would do with mohaa server:

- `.\omohaaded.exe +exec server.cfg` on Windows
- `./omohaaded +exec server.cfg` on Linux

Only `omohaaded*` and `game*` binaries are required for the server version, other binaries can be omitted.
