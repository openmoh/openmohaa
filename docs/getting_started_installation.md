# Installation

Ensure that you download the binaries compatible with your platform in the [releases](https://github.com/openmoh/openmohaa/releases) section.

## Which release should be downloaded?

- If running Windows on AMD/Intel, CPU then `*-windows-x64.zip` is recommended
- If running Windows on Qualcomm/Snapdragon (ARM-based) CPU, then `*-windows-arm64.zip` is the way to go
- If running MacOS on Apple Silicon hardware, choose `*-macos-arm64.zip`
- If running MacOS on AMD/Intel CPU, choose `*-macos-arm64.zip`
- If running Linux on AMD/Intel CPU, then `*-linux-amd64` is generally fine. Choose the i686 version for CPUs that don't support 64-bit instructions.
- If running Linux on a Raspberry Pi 4 or 5, then `*-linux-arm64` (if running ARM64) is the way to go

## Requirements

- A valid MOH:AA installation. A fully patched MOH:AA War Chest copy can be acquired from GOG [here](https://www.gog.com/en/game/medal_of_honor_allied_assault_war_chest). Demo versions are also valid, see [obtaining a demo version](#obtaining-a-demo-version).
  - [Extracting GOG Installer on Linux and macOS](#extracting-gog-setup-file-on-linux-and-macos).
- On Windows, Microsoft Visual C++ 2015/2017/2019/2022 Redistributable from https://learn.microsoft.com/en-US/cpp/windows/latest-supported-vc-redist?view=msvc-170 is required.

## System requirements

This matches the hardware requirements (x86) of the original game.

### Medal of Honor: Allied Assault

1.2 GB of free disk space is required for the full game installation + binaries.

#### x86 (AMD and Intel)

|Hardware       |Minimum                                             |Recommended
|---------------|----------------------------------------------------|---------------
|CPU            |450 MHz (Intel Pentium II), or 500 MHz (AMD Athlon) |700 MHz (Intel Pentium III or AMD Athlon)
|Memory         |128MB                                               |256MB
|GPU            |16MB                                                |32MB

#### ARM

|Hardware       |Minimum                                             |Recommended
|---------------|----------------------------------------------------|---------------
|CPU            |600 MHz ARM Cortex-A8                               |800 MHz ARM Cortex-A9
|Memory         |128MB                                               |256MB
|GPU            |16MB                                                |32MB

#### PowerPC

|Hardware       |Minimum                                             |Recommended
|---------------|----------------------------------------------------|---------------
|CPU            |G3 450 MHz                                          |G4 1 GHz
|Memory         |128MB                                               |256MB
|GPU            |16MB                                                |32MB

### Spearhead expansion

1.0 GB of free disk space is required to install Spearhead on top of the base game.

#### x86 (AMD and Intel)

|Hardware       |Minimum                                             |Recommended
|---------------|----------------------------------------------------|---------------
|CPU            |450 MHz (Intel Pentium II), or 500 MHz (AMD Athlon) |700 MHz (Intel Pentium III or AMD Athlon)
|Memory         |128MB                                               |256MB
|GPU            |16MB                                                |32MB

#### ARM

|Hardware       |Minimum                                             |Recommended
|---------------|----------------------------------------------------|---------------
|CPU            |600 MHz ARM Cortex-A8                               |800 MHz ARM Cortex-A9
|Memory         |128MB                                               |256MB
|GPU            |16MB                                                |32MB

#### PowerPC

|Hardware       |Minimum                                             |Recommended
|---------------|----------------------------------------------------|---------------
|CPU            |G3 500 MHz                                          |G4 1 GHz
|Memory         |256MB                                               |256MB
|GPU            |16MB                                                |32MB

### Breakthrough expansion

800 MB of free disk space is required to install Spearhead on top of the base game.

#### x86 (AMD and Intel)

|Hardware       |Minimum                                              |Recommended
|---------------|-----------------------------------------------------|---------------
|CPU            |733 MHz (Intel Pentium III), or 700 MHz (AMD Athlon) |1 GHz (Intel Pentium III or AMD Athlon)
|Memory         |128MB                                                |256MB
|GPU            |16MB                                                 |32MB

#### ARM

|Hardware       |Minimum                                             |Recommended
|---------------|----------------------------------------------------|---------------
|CPU            |800 MHz ARM Cortex-A9                               |1 GHz ARM Cortex-A15
|Memory         |128MB                                               |256MB
|GPU            |16MB                                                |32MB


#### PowerPC

|Hardware       |Minimum                                             |Recommended
|---------------|----------------------------------------------------|---------------
|CPU            |G3 800 MHz                                          |G4 1 GHz
|Memory         |256MB                                               |256MB
|GPU            |16MB                                                |32MB

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

## Obtaining a demo version

Alternatively, free demo versions are available online. Here are the links to the demos:

|Game                |Type                               |Link                                                                                                    |
|--------------------|-----------------------------------|--------------------------------------------------------------------------------------------------------|
|MOH:AA              | Single-player (0.05)              |https://www.fileplanet.com/archive/p-54011/Medal-of-Honor-Single-Player-Demo                            |
|MOH:AA              | Multiplayer<sup>1</sup> (1.00)    |https://www.fileplanet.com/archive/p-8725/Medal-of-Honor-Allied-Assault-Multiplayer-Demo                |
|MOH:AA Spearhead    | Multiplayer<sup>2</sup> (2.11)    |https://www.fileplanet.com/archive/p-68997/Medal-of-Honor-Allied-Assault-Spearhead-Demo                 |
|MOH:AA Breakthrough | Multiplayer<sup>2</sup> (0.30)    |https://www.fileplanet.com/archive/p-50953/Medal-of-Honor-Allied-Assault-Breakthrough-Multiplayer-Demo  |

- MOH:AA singleplayer demo ships with **Mission 3: Level 3**
- MOH:AA multiplayer demo ships with **Stalingrad**
- MOH:AA Spearhead multiplayer demo ships with **Malta** and **Druckkammern**
- MOH:AA Breakthrough multiplayer demo ships with **Anzio** and **Palermo**

--------

**Notes:**

1. Can play on MOH:AA 1.00 servers only.
2. Only compatible with demo servers. These servers can be identified by having the letter `d` at the start of their version number (e.g: `(d2.11)`).
