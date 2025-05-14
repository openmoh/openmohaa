# Quick installation on Windows

This guide assumes you installed **Medal of Honor: Allied Assault Warchest** from GOG, but it also works with CD or other versions.

You can get it from [GOG here](https://www.gog.com/en/game/medal_of_honor_allied_assault_war_chest).

## Download

1. Download and install [Microsoft Visual C++ Redistributable x64](https://aka.ms/vs/17/release/vc_redist.x64.exe)
2. Grab the latest `openmohaa-*-windows-x64.zip` from the [releases page](https://github.com/openmoh/openmohaa/releases).

ℹ️ On Windows, `-pdb` archives are intended for maintainers and contributors, they contain debug information specific to this build and is used for debugging purposes, you don't need them.

## Find your game folder

1. Right-click your **Medal of Honor - Allied Assault** desktop shortcut → **Properties**
2. Then check the "**Start in**" field, that's your game folder.

![Shortcut](screenshots/install_guide/windows_installation_shortcut.png)

Alternatively, try one of these common locations:
- C:\GOG Games\Medal of Honor - Allied Assault War Chest
- C:\Program Files (x86)\EA Games\MOHAA
- C:\Program Files\Origin Games\Medal of Honor Allied Assault Warchest
- C:\Program Files\GOG Galaxy\Games\Medal of Honor - Allied Assault War Chest

If not there, you'll need to search where you installed it.

## Install

1. **Extract the .zip file** you downloaded into the game folder you found earlier.
2. **Create desktop shortcuts** for these 3 files:
    * `launch_openmohaa_base.exe` - *Allied Assault*
    * `launch_openmohaa_spearhead.exe` - *Spearhead*
    * `launch_openmohaa_breakthrough.exe` - *Breakthrough*

To do that, **right-click each file**, then choose **Send to → Desktop (create shortcut)**.

![Send To](screenshots/install_guide/windows_installation_sendto.png)

Now you can launch any version of the game directly from your desktop using OpenMoHAA.
