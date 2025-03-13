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

#pragma once

#include "sys_local.h"

#ifdef __cplusplus
extern "C" {
#endif

    void Sys_UpdateChecker_Init();
    void Sys_UpdateChecker_Process();
    void Sys_UpdateChecker_Shutdown();

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

#    include <thread>
#    include <shared_mutex>
#    include <chrono>
#    include <condition_variable>

class UpdateCheckerThread
{
public:
    UpdateCheckerThread();
    ~UpdateCheckerThread();

    void Init();
    void Shutdown();
    bool IsRoutineActive() const;

private:
    void ShutdownThread();

    void InitClient();
    void ShutdownClient();

    bool ParseVersionNumber(const char *value, int& major, int& minor, int& patch) const;
    void RequestThread();
    void DoRequest();

private:
    void                   *handle;
    std::mutex              clientWakeMutex;
    std::condition_variable clientWake;
    std::thread            *osThread;
    qboolean                requestThreadIsActive;
    qboolean                shouldBeActive;
};

class UpdateChecker
{
public:
    UpdateChecker();
    ~UpdateChecker();

    void Init();
    void Process();
    void Shutdown();

    bool CheckNewVersion() const;
    bool CheckNewVersion(int& major, int& minor, int& patch) const;
    void SetLatestVersion(int major, int minor, int patch);

private:
    void CheckInitClientThread();
    bool CanHaveRequestThread() const;

private:
    //
    // Version information
    //
    int  lastMajor;
    int  lastMinor;
    int  lastPatch;
    bool versionChecked;

    std::chrono::time_point<std::chrono::steady_clock> nextMessageTime;

    UpdateCheckerThread *thread;
};

extern UpdateChecker updateChecker;

#endif
