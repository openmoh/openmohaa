# Compiling

OpenMoHAA supports any architecture, thanks to the CMake build system and cross-platform code/libraries. While the most common little-Endian architectures (arm, aarch64, x86, x86_64) have been tested and confirmed to work, big-Endian architectures should be supported as well; however, they remain untested.

The following tools are required for all platforms:
- CMake >= 3.12
- Flex (>= 2.6.4) and Bison (>= 3.5.1)
- A C++11 compiler
- [SDL2](https://github.com/libsdl-org/SDL/tree/SDL2)
- [OpenAL SDK](https://github.com/kcat/openal-soft)

The following tools may be useful:
- [cURL](https://github.com/curl/curl)

The installation directory can be set to the MOHAA directory with `-DCMAKE_INSTALL_PREFIX=/path/to/mohaa`.

Compiling debug binaries will result in a `-dbg` suffix appended to the name of the binaries to avoid mixing debug/release code.

cURL is used to access websites. It is currently used to check for a new release. If the project is compiled without cURL, OpenMoHAA will be unable to check for new updates automatically.

## Compiling for Linux

These are the tools required on Linux :
- Clang >= 7.0.1 or GCC >= 9.4.0
- libsdl2-dev
- libopenal-dev
- libssl-dev
- libcurl4-openssl-dev

**clang-7** and **gcc-9** has been tested to work on Ubuntu 20.04. Although it's best to use the latest versions.

1 line install command with clang:
```sh
sudo apt-get install -y cmake ninja-build clang lld flex bison libsdl2-dev libopenal-dev libcurl4-openssl-dev
```

Example with **CMake** and **ninja-build** installed:
```sh
mkdir .cmake && cd .cmake
cmake -G Ninja ../
```

Other compilers can be specified by appending `-DCMAKE_C_COMPILER=/path/to/compiler -DCMAKE_CXX_COMPILER=path/to/compiler` to the CMake command-line.

## Compiling for Windows

Visual Studio (2019 or 2022) is generally preferred.

- Get Flex/Bison: https://github.com/lexxmark/winflexbison/releases/latest
- Get OpenAL: https://github.com/kcat/openal-soft/releases/latest rename `soft_oal.dll` to `OpenAL64.dll` on 64-bit and `OpenAL32.dll` on 32-bit
- Get cURL: https://github.com/curl/curl

Append `-DFLEX_EXECUTABLE=...\win_flex.exe -DBISON_EXECUTABLE=...\win_bison.exe -DOPENAL_INCLUDE_DIR="path/to/oal/include" -DOPENAL_LIBRARY="path/to/oal"` to the CMake command-line to use the package from the link above.

Optionally, The cURL library can be compiled with the following CMake commands:
```
cmake -DCMAKE_INSTALL_PREFIX="E:\Src\openmoh\openmohaa\thirdparty\curl\build\install" -DCURL_USE_LIBPSL=OFF -DCURL_USE_SCHANNEL=ON -B build
cmake --build build --config Release
cmake --install build --config Release
```

afterwards you can can append `-DCURL_ROOT=path\to\curl\install` to specify the install path to cURL.

## Tweaking the build

- `-DBUILD_NO_CLIENT=1` Don't build client binaries. This will only build the dedicated server binaries.
- `-DCMAKE_LIB_SUFFIX=suffix` Default suffix to use for the default `lib` directory. For example `-DCMAKE_LIB_SUFFIX=64`
- `-DNO_MODERN_DMA=1` Use the basic DMA sound system from quake3. This lacks many features such as soundtracks and movie audio from the modern OpenAL-based DMA sound system. This option is not recommended, unless openal is not available.
- `-DTARGET_LOCAL_SYSTEM=1` for compiling and installing on the local system. This will remove the architecture suffix at the end of each binary. This option can be useful for packaging builds.
- `-DUSE_SYSTEM_LIBS=1` for third-party libraries, this will make use of system libraries instead of libraries that are embedded within the project.