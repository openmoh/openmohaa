# Compiling

OpenMoHAA supports any architecture, thanks to the CMake build system and cross-platform code/libraries. While the most common little-Endian architectures (arm, aarch64, x86, x86_64, ppc64le) have been tested and confirmed to work, big-Endian architectures should be supported as well; however, they remain untested.

The following tools are required for all platforms:
- CMake >= 3.25
- Flex (>= 2.6.4) and Bison (>= 3.5.1)
- A C++11 compiler
- [SDL2](https://github.com/libsdl-org/SDL/tree/SDL2)
- [OpenAL SDK](https://github.com/kcat/openal-soft)

The following tools may be useful:
- [cURL](https://github.com/curl/curl)

The installation directory can be set to the MOHAA directory with `-DCMAKE_INSTALL_PREFIX=/path/to/mohaa`.

Compiling debug binaries will result in a `-dbg` suffix appended to the name of the binaries to avoid mixing debug/release code.

> [!NOTE]
> OpenMoHAA is loosely coupled to libcurl, meaning the library is not required for the game to work normally. Currently it only checks for update, it can be used for more features.

## Compiling for Linux

These are the tools required on Linux :
- Clang >= 7.0.1 or GCC >= 9.4.0
- libsdl2-dev
- libopenal-dev
- libcurl4-openssl-dev

**clang-7** and **gcc-9** are the minimum required versions. These are EOLs, the project is built using currently maintained compiler versions, which are recommended.

1 line install command with clang:
```sh
sudo apt-get install -y cmake ninja-build clang lld flex bison libsdl2-dev libopenal-dev libcurl4-openssl-dev
```

Example with **CMake** and **ninja-build** installed:
```sh
mkdir .cmake && cd .cmake
cmake ../
cmake --build .
cmake --install .
```

Usually binaries get installed inside the `/usr/local/lib/openmohaa` directory.

Other compilers can be specified by appending `-DCMAKE_C_COMPILER=/path/to/compiler -DCMAKE_CXX_COMPILER=path/to/compiler` to the CMake command-line.

## Compiling for Windows

Visual Studio (2019-2026) is generally preferred.

- Get Flex/Bison: https://github.com/lexxmark/winflexbison/releases/latest
- Get OpenAL: https://github.com/kcat/openal-soft/releases/latest rename `soft_oal.dll` to `OpenAL64.dll` on 64-bit and `OpenAL32.dll` on 32-bit
- Get cURL: https://github.com/curl/curl

Optionally, The cURL library can be compiled with the following CMake commands:
```
cmake -DCMAKE_INSTALL_PREFIX="C:\source\openmohaa\thirdparty\curl\build\install" -DCURL_USE_LIBPSL=OFF -DCURL_USE_SCHANNEL=ON -B build
cmake --build build --config Release
cmake --install build --config Release
```

Append `-DFLEX_EXECUTABLE=...\win_flex.exe -DBISON_EXECUTABLE=...\win_bison.exe -DOPENAL_INCLUDE_DIR="path/to/oal/include" -DOPENAL_LIBRARY="path/to/oal"` to the CMake command-line to compile the project.

Example after cloning the repository:
```sh
mkdir .cmake && cd .cmake
cmake -DFLEX_EXECUTABLE=path/to/win_flex.exe -DBISON_EXECUTABLE=path/to/win_bison.exe -DOPENAL_INCLUDE_DIR="path/to/oal/include" -DOPENAL_LIBRARY="path/to/oal" ../
cmake --build .
cmake --install .
```

Usually binaries get installed inside the `Program Files (x86)` folder.

afterwards you can can append `-DCURL_ROOT=path\to\curl\install` to specify the install path to cURL.

## Tweaking the build

- `-DBUILD_CLIENT=0` Build without the client program. This will only build the dedicated server binaries alongside the game & cgame modules.
- `-DCMAKE_LIB_SUFFIX=suffix` Default suffix to use for the default `lib` directory. For example `-DCMAKE_LIB_SUFFIX=64`.

For other options, check `CMakeLists.txt` file at the root of the project.
