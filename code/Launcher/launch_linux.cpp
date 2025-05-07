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

// main.cpp: Windows implementation of the launcher

#include "launch.h"

#include <unistd.h>
#include <spawn.h>
#include <sys/wait.h>
#include <stdlib.h>

#include <string>
#include <iostream>

extern "C" char **environ;

std::filesystem::path GetProgramLocation()
{
    char    path[FILENAME_MAX];
    ssize_t count = readlink("/proc/self/exe", path, FILENAME_MAX);

    return std::filesystem::path(std::string(path, (count > 0) ? count : 0)).parent_path();
}

void LaunchProgram(const std::filesystem::path& path, const std::vector<std::string>& argumentList)
{
    pid_t       pid;
    std::string pathString = path.string();
    size_t      argCount   = argumentList.size();
    char      **argv;
    int         status;

    argv    = new char *[argCount + 2];
    argv[0] = (char *)pathString.c_str();

    for (size_t i = 0; i < argCount; i++) {
        argv[i + 1] = (char *)argumentList[i].c_str();
    }
    argv[argCount + 1] = NULL;

    //status = posix_spawn(&pid, pathString.c_str(), NULL, NULL, argv, environ);
    //delete[] argv;
    //if (status != 0) {
    //    std::cout << "posix_spawn returned error: " << status << std::endl;
    //    return;
    //}

    //
    // Using execve rather than posix_spawn
    //  This replaces the current process that is not needed anyway
    //
    status = execve(pathString.c_str(), argv, environ);
    delete[] argv;
    if (status == -1) {
        std::cout << "posix_spawn returned error: " << errno << std::endl;
        return;
    }

    waitpid(pid, NULL, 0);
}
