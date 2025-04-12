# Installation

- Windows users can check this [Quick install guide](getting_started_installation_windows.md).
- Linux users may want to check if an OpenMoHAA package is available with the distribution's package manager.

## Requirements

- A MOH:AA installation. Common installations include:
  - GOG copy (recommended). A fully patched MOH:AA War Chest copy can be acquired [here](https://www.gog.com/en/game/medal_of_honor_allied_assault_war_chest).
    - For Linux/macOS, see [Extracting GOG Installer on Linux and macOS](#extracting-gog-setup-file-on-linux-and-macos).
  - From a CD. Make sure to [patch your installed copy](#installing-official-patches)
  - Demo. See [obtaining a demo version](#obtaining-a-demo-version)
- On Windows, Microsoft Visual C++ 2015/2017/2019/2022 Redistributable from https://learn.microsoft.com/en-US/cpp/windows/latest-supported-vc-redist?view=msvc-170 is required.
- The game should be able to run on any modern hardware. See [System requirements](./getting_started_requirements.md)

## Installing official patches

If the **MOH: Warchest** copy is installed, skip directly to the [OpenMoHAA installation](#downloadinginstalling) section.

Otherwise, install the following patches:

|Game           |Patch to download and install
|---------------|------------------
|Allied Assault |Depends on the installed language: [English](https://web.archive.org/web/20160229203048/http://largedownloads.ea.com/pub/patches/MOHAA_UKUS_ONLY_patch111v9safedisk.exe), [German](https://web.archive.org/web/20160229203013/http://largedownloads.ea.com/pub/patches/MOHAA_DE_ONLY_patch111v9safedisk.exe), [French](https://web.archive.org/web/20151201080806/http://largedownloads.ea.com/pub/patches/MOHAA_FR_ONLY_patch111v9safedisk.exe) [Italian](https://web.archive.org/web/20141205065317/http://largedownloads.ea.com/pub/patches/MOHAA_IT_ONLY_patch111v9safedisk.exe), [Spanish](https://web.archive.org/web/20151201080738/http://largedownloads.ea.com/pub/patches/MOHAA_ES_ONLY_patch111v9safedisk.exe), or [Dutch](https://web.archive.org/web/20151201080902/http://largedownloads.ea.com/pub/patches/MOHAA_NL_ONLY_patch111v9safedisk.exe)
|Spearhead      |[2.0 to 2.11](https://web.archive.org/web/20170130184731/ftp://ftp.ea.com/pub/ea/patches/mohaa_spearhead/mohaas_patch_20_to_211.exe), followed by [2.11 to 2.15](https://web.archive.org/web/20170130184725/ftp://ftp.ea.com/pub/ea/patches/mohaa_spearhead/MOHAAS_Patch_211_to_215.exe)
|Breakthrough   |[2.40b](https://web.archive.org/web/20160301122255/http://largedownloads.ea.com/pub/patches/medal_of_honor_allied_assault_breakthrough_patch_2_40.exe)

These patches are required to connect to multiplayer servers and ensure a smooth, bug-free single-player experience.

## Downloading/installing OpenMoHAA

Ensure that you download the binaries compatible with your platform in the [releases](https://github.com/openmoh/openmohaa/releases/latest) section.

### Which release should be downloaded?

It depends on the OS that is installed on your device:

|OS       |Kind of hardware (CPU, platform...)        |Archive
|---------|-------------------------------------------|-----------------------
|Windows  |AMD/Intel                                  |`*-windows-x64.zip`
|Windows  |Qualcomm/Snapdragon (ARM-based)            |`*-windows-arm64.zip`
|macOS    |Apple Silicon (ARM)                        |`*-macos-arm64.zip`
|macOS    |Intel                                      |`*-macos-x86_64.zip`
|Linux    |AMD/Intel                                  |`*-linux-amd64`
|Linux    |Raspberry Pi 4 or 5                        |`*-linux-arm64`

Once the correct archive was downloaded:

a) Extract the archive<sup>1</sup> to your MOHAA installation directory.

-or-

b) Extract the archive<sup>1</sup> somewhere on your hard drive, create a shortcut to each of the **launch_openmohaa_\*** executables (or omohaaded.exe), and set the shortcut's 'Start in' directory to your MOHAA installation directory.

Once you're ready, start one of the three launchers based on whether you want to play the base game, Spearhead, or Breakthrough, and then you can start playing.

----

**Notes:**

1. For servers, only the `omohaaded.*` and `game.*` binaries from the archive are required.

## Appendix

### Cleaning up the game installation directory

If you want to clean up the mohaa installation directory by keeping only what is needed for OpenMoHAA, the following files/directories can be kept:
```cpp
[MOHAA]
├── [main]
|   ├── [sound]
|   ├── [video]
│   ├── Pak*.pk3 // all pak files
├── [mainta]
|   ├── [sound]
|   ├── [video]
│   ├── pak*.pk3 // all pak files
├── [maintt]
|   ├── [sound]
|   ├── [video]
│   ├── pak*.pk3 // all pak files
├── All files from the OpenMoHAA archive
```

### Extracting GOG setup file on Linux and macOS

If your MOH:AA copy was acquired from GOG then this section will be relevant.

The MOH:AA Warchest installer files on GOG are Windows binaries. To attain the game files, you can quickly extract them using [Innoextract](https://github.com/dscharrer/innoextract). 

- Install Innoextract using your preferred Package Manager (e.g. [Brew](https://brew.sh/)). 

- Once installed run the command `innoextract setup_medal_of_honor_2.0.0.21.exe`. The MoH:AA game files will be extracted into a folder called 'app'.

Alternatively, you can use [WINE](https://www.winehq.org/) as well, but this process may take longer if you do not already have WINE installed and configured. 

### Obtaining a demo version

Alternatively, free demo versions are available online. Here are the links to the demos:

|Game                |Type                               |Link                                                                                  |
|--------------------|-----------------------------------|--------------------------------------------------------------------------------------|
|MOH:AA              | Single-player (0.05)              |https://www.gamefront.com/games/medal-of-honor/file/single-player-demo                |
|MOH:AA              | Multiplayer<sup>1</sup> (1.00)    |https://www.gamefront.com/games/medal-of-honor/file/multiplayer-demo                  |
|MOH:AA Spearhead    | Multiplayer<sup>2</sup> (2.11)    |https://www.gamefront.com/games/medal-of-honor/file/mohaa-spearhead-demo-eng          |
|MOH:AA Breakthrough | Multiplayer<sup>2</sup> (0.30)    |https://www.gamefront.com/games/medal-of-honor/file/mohaa-breakthrough-demo           |

- MOH:AA singleplayer demo ships with the level: **Mission 3: Level 3**.
- MOH:AA multiplayer demo ships with the level: **Stalingrad**. **The Hunt** can be added with the [MP Demo addon](https://www.gamefront.com/games/medal-of-honor/file/mp-demo-add-on-the-hunt). 
- MOH:AA Spearhead multiplayer demo ships with multiplayer maps: **Malta** and **Druckkammern**.
- MOH:AA Breakthrough multiplayer demo ships with multiplayer maps: **Anzio** and **Palermo**.

--------

**Notes:**

1. Can play on MOH:AA 1.00 servers only.
2. Only compatible with demo servers. These servers can be identified by having the letter `d` at the start of their version number (e.g: `(d2.11)`).
