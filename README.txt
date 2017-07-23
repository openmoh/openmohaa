<<<<<<< HEAD
 / _ \ _ __   ___ _ __ |  \/  |/ _ \| | | |  / \      / \
| | | | '_ \ / _ \ '_ \| |\/| | | | | |_| | / _ \    / _ \
| |_| | |_) |  __/ | | | |  | | |_| |  _  |/ ___ \  / ___ \
 \___/| .__/ \___|_| |_|_|  |_|\___/|_| |_/_/   \_\/_/   \_\
      |_|

Alert
=======================
The OpenMoHAA client build is currently unusable due to the UI being reimplemented, please don't use it.
Currently only the server version is working.

Running
=======================
Backup gamex86.dll (rename it to gamex86.bak).
Rename cgamex86.dll to cgamex86mohaa.dll.

a) extract archive to your MOHAA installation directory.
-or-
b) Copy all pak*.pk3 files and the sound/ directory from your MOHAA/main
directory into openmohaa's 'main' directory.

If you have problems running the game :

You may need to install Microsoft Visual C++ 2010 Redistributable from
http://www.microsoft.com/download/en/details.aspx?id=5555

or you may need to install Microsoft Visual C++ Redistributable Packages for Visual Studio 2013 from
http://www.microsoft.com/en-us/download/details.aspx?id=40784

You can now start a local MOHAA/OpenMOHAA server or play on a server.

Compiling
=======================
*NIX:
make sure you have compiler + libraries installed
Required extra libaries: libopenal-dev libsdl-dev libmad-dev
OpenGL: sudo apt-get install libglu1-mesa-dev freeglut3-dev mesa-common-dev
OpenAL: sudo apt-get install libopenal1 libopenal-dev
ALUT:   sudo apt-get install libalut0 libalut-dev

Download and install Eclipse with your NIX OS version
http://www.eclipse.org/downloads/packages/eclipse-ide-cc-developers/marsr

Start Eclipse and set the workspace path to misc/nix.
Then import an existing workspace using the path to misc/nix.

Clean and compile by using Eclipse

WIN:

Visual Studio 2008 -> Open misc/msvc8/openmohaa/openmohaa.sln.
Visual Studio 2012 -> Open misc/msvc12_13/openmohaa/openmohaa.sln and set all projects platform toolset to v110 if you get the compilation error.
Visual Studio 2013 -> Open misc/msvc12_13/openmohaa/openmohaa.sln
OpenGL32.lib should be shipped with any Visual Studio versions.

GENERAL:
You don't need anything else as everything needed is in the repository.
Now you can compile the solution.

=====
SDL
http://www.libsdl.org/ download Source code, compile
=====
OpenAL
http://connect.creativelabs.com/openal/default.aspx download OpenAL soft source
download CMake at http://www.cmake.org/cmake/resources/software.html
run cmake on OpenAL, compile
=====
LibMAD
http://www.underbit.com/products/mad/ download source code, compile
=====

Miscellaneous
=======================

Output files :

build/testutils_x** --> can be placed anywhere, it's just a test tool for scripts.

build/openmohaa_x** --> MOHAA/
build/omohaaded_x** --> MOHAA/

build/main/cgamex**opm --> MOHAA/main
build/main/gamex** --> MOHAA/main (BACKUP MOHAA/main/gamex86.dll FIRST)
build/main/uix**opm --> MOHAA/main
build/main/cgamex86 --> MOHAA/main (RENAME MOHAA/main/cgamex86.dll to cgamex86mohaa.dll FIRST)

That's all, now run MOHAA or OpenMOHAA and start a local server.

URL
=======================
http://openmohaa.sf.net/
http://www.x-null.net/
http://sfxmodding.net/
