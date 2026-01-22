# Installation guide

If you are on Windows, you can skip straight to [Windows quick install guide](02-installation-windows.md) for a simpler setup if MOH:AA is already installed.

## Before you start

The game should be able to run on any modern hardware. You can verify the [System requirements](03-requirements.md).

You will need a copy of MOH:AA, either from [GOG](https://www.gog.com/en/game/medal_of_honor_allied_assault_war_chest), from CD or a from a [demo version](#obtaining-a-demo-version)

💡 *GOG version is recommended*.

If you already have MOH:AA from a CD, make sure to [Install official patches](#installing-official-patches-cd-only).

### Windows users

Download and install [Microsoft Visual C++ Redistributable x64](https://aka.ms/vs/17/release/vc_redist.x64.exe).

### Linux and macOS users

On Linux, you can download and install a copy from GOG:

1. Install [Innoextract](https://github.com/dscharrer/innoextract) (macOS users: `brew install innoextract`).
2. Run the command: `innoextract setup_medal_of_honor_2.0.0.21.exe`.
3. Files from the setup will be extracted to a folder called `app/`.

Alternatively, you can also use [WINE](https://www.winehq.org/) to install the game from the setup file, but this process may be slower/more complex if you do not already have it installed and configured. 

## Installation

### Downloading OpenMoHAA

1. Go to the [latest release page](https://github.com/openmoh/openmohaa/releases/latest)
2. Choose the correct version for your platform:

|OS       |Kind of hardware (CPU, platform...)        |Archive
|---------|-------------------------------------------|-----------------------
|Windows  |AMD/Intel                                  |`*-windows-x64.zip` **(most Windows PCs - choose this if unsure)**
|Windows  |Qualcomm/Snapdragon (ARM-based)            |`*-windows-arm64.zip`
|macOS    |Apple Silicon or Intel                     |`*-macos-multiarch.arm64-x86_64.zip`
|Linux    |AMD/Intel                                  |`*-linux-amd64` **(most Linux PCs - choose this if unsure)**
|Linux    |Raspberry Pi 4 or 5                        |`*-linux-arm64`

ℹ️ On Windows, `-pdb` archives are intended for maintainers and contributors, they contain debug information specific to this build and is used for debugging purposes, you don't need them.

### Installing OpenMoHAA

- Extract the archive<sup>1</sup> into the game folder where MOHAA is installed (the folder that contains the `main` subfolder and the original game files).
  - On Linux/macOS, if MOHAA is installed in `/usr/games/mohaa`, then extract all files inside `/usr/games/mohaa`.
  - On Windows, if MOHAA is installed in `C:\Program Files (x86)\EA Games\Medal of Honor Allied Assault`, then extract all files inside `C:\Program Files (x86)\EA Games\Medal of Honor Allied Assault`.
  - Not sure where your game folder is? See the [Windows quick install guide](02-installation-windows.md#find-your-game-folder) for tips on finding it.
- Create a shortcut to each of the **launch_openmohaa_\*** executable:
  - `launch_openmohaa_base` (Base game)
  - `launch_openmohaa_spearhead` (Spearhead)
  - `launch_openmohaa_breakthrough` (Breakthrough)

🟢 You're now ready to play! To start the game, execute one of the `launch_openmohaa_x` binaries.

----

**Notes:**

1. For servers, only `omohaaded.*`, `game.*`, and `curl` binaries from the archive are required.

## Appendix

### (Optional) Cleaning up the game installation directory

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

## Installing official patches (CD only)

If the **MOH: Warchest** copy is installed, skip directly to the [OpenMoHAA installation](#installation) section.

Otherwise, install the following patches:

|Game           |Patch to download and install
|---------------|------------------
|Allied Assault |Depends on the installed language: [English](https://web.archive.org/web/20160229203048/http://largedownloads.ea.com/pub/patches/MOHAA_UKUS_ONLY_patch111v9safedisk.exe), [German](https://web.archive.org/web/20160229203013/http://largedownloads.ea.com/pub/patches/MOHAA_DE_ONLY_patch111v9safedisk.exe), [French](https://web.archive.org/web/20151201080806/http://largedownloads.ea.com/pub/patches/MOHAA_FR_ONLY_patch111v9safedisk.exe) [Italian](https://web.archive.org/web/20141205065317/http://largedownloads.ea.com/pub/patches/MOHAA_IT_ONLY_patch111v9safedisk.exe), [Spanish](https://web.archive.org/web/20151201080738/http://largedownloads.ea.com/pub/patches/MOHAA_ES_ONLY_patch111v9safedisk.exe), or [Dutch](https://web.archive.org/web/20151201080902/http://largedownloads.ea.com/pub/patches/MOHAA_NL_ONLY_patch111v9safedisk.exe)
|Spearhead      |[2.0 to 2.11](https://web.archive.org/web/20170130184731/ftp://ftp.ea.com/pub/ea/patches/mohaa_spearhead/mohaas_patch_20_to_211.exe), followed by [2.11 to 2.15](https://web.archive.org/web/20170130184725/ftp://ftp.ea.com/pub/ea/patches/mohaa_spearhead/MOHAAS_Patch_211_to_215.exe)
|Breakthrough   |[2.40b](https://web.archive.org/web/20160301122255/http://largedownloads.ea.com/pub/patches/medal_of_honor_allied_assault_breakthrough_patch_2_40.exe)

These patches are required to connect to multiplayer servers and ensure a smooth, bug-free single-player experience.

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

---

## Other platforms and ports

OpenMoHAA is officially supported on *Windows*, *Linux* and *macOS*. Below is a list of unofficial open-source ports that required custom modifications to the OpenMoHAA source code to build and run:

- https://github.com/3246251196/openmohaa/tree/AmigaOS4_0_81_1 AmigaOS
- https://github.com/Cowcat5150/openmohaa MorphOS
- https://github.com/Rinnegatamante/openmohaa PS Vita

To stay up to date with the latest features and fixes from the official repository, these ports depend on their respective maintainers to synchronize changes with the main OpenMoHAA codebase.
