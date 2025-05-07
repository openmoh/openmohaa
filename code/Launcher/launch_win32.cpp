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

#include <Windows.h>
#include <string>
#include <iostream>

std::filesystem::path GetProgramLocation()
{
    wchar_t path[FILENAME_MAX] = {0};
    GetModuleFileNameW(nullptr, path, FILENAME_MAX);

    return std::filesystem::path(path).parent_path();
}

void LaunchProgram(const std::filesystem::path& path, const std::vector<std::string>& argumentList)
{
    PROCESS_INFORMATION processInfo;
    STARTUPINFOW        startupInfo;
    BOOL                returnValue;
    std::wstring        osCommandLine;
    std::wstring        commandLine;
    size_t              argCount = argumentList.size();

    memset(&processInfo, 0, sizeof(processInfo));
    memset(&startupInfo, 0, sizeof(startupInfo));
    startupInfo.cb = sizeof(startupInfo);

    for (size_t i = 0; i < argCount; i++) {
        commandLine += L"\"";
        commandLine += std::wstring(argumentList[i].begin(), argumentList[i].end());
        commandLine += L"\"";
        if (i != argCount - 1) {
            commandLine += L" ";
        }
    }

    osCommandLine = L"\"" + path.wstring() + L"\"";
    osCommandLine += L" ";
    osCommandLine += commandLine;

    returnValue = CreateProcessW(
        path.wstring().c_str(),
        (LPWSTR)osCommandLine.c_str(),
        NULL,
        NULL,
        FALSE,
        0,
        NULL,
        std::filesystem::current_path().wstring().c_str(),
        &startupInfo,
        &processInfo
    );

    if (!returnValue) {
        std::cout << "Error in CreateProcess: " << GetLastError() << std::endl;
        return;
    }

    WaitForSingleObject(processInfo.hProcess, INFINITE);

    CloseHandle(processInfo.hProcess);
    CloseHandle(processInfo.hThread);
}
