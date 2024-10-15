# Compiling

OpenMoHAA supports any architecture, thanks to the CMake build system and cross-platform code/libraries. While the most common little-Endian architectures (arm, aarch64, x86, x86_64) have been tested and confirmed to work, big-Endian architectures should be supported as well; however, they remain untested.

The following tools are required for all platforms:
- CMake >= 3.5
- Flex (>= 2.6.4) and Bison (>= 3.5.1)
- A C++11 compiler
- OpenAL SDK (can be found [here](https://github.com/kcat/openal-soft))

The installation directory can be set to the MOHAA directory.

By default, the build will produce both the client and dedicated server versions. The client can be omitted from the build by appending `-DBUILD_NO_CLIENT=1` to the CMake command-line arguments. Using this parameter will result in only the server portion being built.

OpenAL can be disabled by appending `-DNO_OPENAL=1` to the CMake command-line arguments. The old SDL-based sound system will be used instead, it lacks some features such as ambient sounds.

## Compiling for Linux

These are the tools required on Linux :
- Clang >= 3.3 or GCC >= 4.8.5
- libsdl2-dev
- libopenal-dev

**clang-3.5** and **gcc-4.8.5** should work (tested on Ubuntu 16.04), but the latest version should be used.

Ubuntu 20.04 is the minimum version required to fully compile the project successfully.

1 line install command with clang:
```sh
sudo apt-get install -y cmake ninja-build clang lld flex bison libsdl2-dev libopenal-dev
```

Example with **CMake** and **ninja-build** installed:
```sh
mkdir .cmake && cd .cmake
cmake -G Ninja ../
```

Other compilers can be specified by appending `-DCMAKE_C_COMPILER=/path/to/compiler -DCMAKE_CXX_COMPILER=path/to/compiler` to the CMake command-line.

## Compiling for Windows

Visual Studio (2019 or 2022) is generally preferred.

Flex and Bison can be downloaded from here: https://github.com/lexxmark/winflexbison/releases/tag/v2.5.25
OpenAL can be downloaded from here: https://github.com/kcat/openal-soft/releases/tag/1.23.1 rename `soft_oal.dll` to `OpenAL64.dll` on 64-bit and `OpenAL32.dll` on 32-bit

Append `-DFLEX_EXECUTABLE=...\win_flex.exe -DBISON_EXECUTABLE=...\win_bison.exe -DOPENAL_INCLUDE_DIR="path/to/oal/include" -DOPENAL_LIBRARY="path/to/oal"` to the CMake command-line to use the package from the link above.
