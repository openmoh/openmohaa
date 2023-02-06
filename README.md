# OpenMoHAA

     / _ \ _ __   ___ _ __ |  \/  |/ _ \| | | |  / \      / \
    | | | | '_ \ / _ \ '_ \| |\/| | | | | |_| | / _ \    / _ \
    | |_| | |_) |  __/ | | | |  | | |_| |  _  |/ ___ \  / ___ \
     \___/| .__/ \___|_| |_|_|  |_|\___/|_| |_/_/   \_\/_/   \_\
          |_|
      


## Discord

[![Discord Banner 1](https://discordapp.com/api/guilds/596049712579215361/widget.png?style=banner2)](https://discord.gg/NYtH58R)

## Current State

### Server

The server version can successfully be built. There are few unimplemented stuff from the game library :

- Actor
- Vehicle

### Client

There is currently no client build because the client code is currently unusable. Here is what's missing or unimplemented :

- cgame (using F.A.K.K build)
- rendering (mostly advanced)
- sound (must be reworked)
- UI

## Running

Backup the existing **gamex86.dll** (rename it to **gamex86.bak**).

a) extract archive to your MOHAA installation directory.

-or-

b) Copy all pak*.pk3 files and the sound/ directory from your **MOHAA/main** directory into openmohaa's 'main' directory.

If you have problems running the game :

You may need to install Microsoft Visual C++ 2015/2017/2019/2022 Redistributable from

https://learn.microsoft.com/en-US/cpp/windows/latest-supported-vc-redist?view=msvc-170

You can now start a local OpenMOHAA server or play on a server.

## Compiling

These are the tools required for all platforms :
- CMake
- Flex and Bison
- Clang (at least version 11)

At least a C++17 compiler is required, clang is preferred over GCC.

The installation directory can be set to MOHAA directory.

### Linux

1 line install command with latest clang version : `sudo apt-get install -y ninja-build cmake make clang-15 lld-15 flex bison`

### Windows

Visual Studio (2019 or 2022) is generally preferred.

## Third party

### SDL

http://www.libsdl.org/

download Source code, compile

### OpenAL

https://www.openal.org/ download OpenAL soft source

run cmake on OpenAL, compile

### LibMAD

http://www.underbit.com/products/mad/

download source code, compile

### cURL

https://curl.se/

### Libogg

https://github.com/gcp/libogg

### Libvorbis

https://xiph.org/vorbis/

### Libopus

https://opus-codec.org/

## URL

- https://openmohaa.sourceforge.net/
- https://x-null.net/

