/*
===========================================================================
Copyright (C) 2025 the OpenMoHAA team

This file is part of OpenMoHAA source code.

OpenMoHAA source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

OpenMoHAA source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with OpenMoHAA source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

// main.cpp: The launcher

#include "launch.h"

#include <iostream>
#include <string>
#include <vector>

const char *targetGameList[] = {"Base", "Spearhead", "Breakthrough"};

int main(int argc, const char *argv[])
{
    std::vector<std::string> argumentList;
#if !defined(DEDICATED) || !DEDICATED
    const char *programName = "openmohaa" DLL_SUFFIX EXE_EXT;
#else
    const char *programName = "omohaaded" DLL_SUFFIX EXE_EXT;
#endif

    argumentList.push_back("+set");
    argumentList.push_back("com_target_game");
    argumentList.push_back(std::to_string(TARGET_GAME));

    for (size_t i = 1; i < argc; i++) {
        argumentList.push_back(argv[i]);
    }

    std::cout << "Using the target game: " << targetGameList[TARGET_GAME] << std::endl;
    std::cout << std::endl;
    std::cout << "Expected program name: " << programName << std::endl;

    const std::filesystem::path programLocation = GetProgramLocation();
    const std::filesystem::path programPath     = programLocation / programName;

    std::cout << "Program location: " << programLocation << std::endl;
    std::cout << "Expected path: " << programPath << std::endl;

    LaunchProgram(programPath, argumentList);

    return 0;
}
