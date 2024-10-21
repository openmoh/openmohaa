# Installation

Ensure that you download the binaries compatible with your platform in the release section.

## Requirements

- A valid MOH:AA installation, a MOH:AA Warchest copy can be acquired from GOG [here](https://www.gog.com/en/game/medal_of_honor_allied_assault_war_chest).
  - [Extracting GOG Installer on Linux and macOS](#extracting-gog-setup-file-on-linux-and-macos)
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


## Extracting GOG setup file on Linux and macOS

The MOH:AA Warchest installer files on GOG are Windows binaries. To attain the game files, you can quickly extract them using [Innoextract](https://github.com/dscharrer/innoextract). 

- Install Innoextract using your preferred Package Manager (e.g. [Brew](https://brew.sh/)). 

- Once installed run the command `innoextract setup_medal_of_honor_2.0.0.21.exe`. The MoH:AA game files will be extracted into a folder called 'app'.

Alternatively, you can use [WINE](https://www.winehq.org/) as well, but this process may take longer if you do not already have WINE installed and configured. 
