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

#include "sys_update_checker.h"
#include "../qcommon/q_version.h"

#ifdef HAS_LIBCURL
#    include <curl/curl.h>
#endif

//#include <httplib.h>
#include "../qcommon/json.hpp"
using json = nlohmann::json;

#include <chrono>

UpdateChecker updateChecker;

void Sys_UpdateChecker_Init()
{
    updateChecker.Init();
}

void Sys_UpdateChecker_Process()
{
    updateChecker.Process();
}

void Sys_UpdateChecker_Shutdown()
{
    updateChecker.Shutdown();
}

UpdateChecker::UpdateChecker()
{
    lastMajor = lastMinor = lastPatch = 0;
    versionChecked                    = false;
}

UpdateChecker::~UpdateChecker() {}

void UpdateChecker::Init()
{
    CheckInitClientThread();
}

void UpdateChecker::CheckInitClientThread()
{
    if (!thread) {
        if (CanHaveRequestThread()) {
            thread = new UpdateCheckerThread();
            thread->Init();
        }
    } else {
        if (!CanHaveRequestThread()) {
            Com_DPrintf("Shutting down the update checker thread\n");

            delete thread;
            thread = NULL;
        }
    }
}

bool UpdateChecker::CanHaveRequestThread() const
{
    if (!Cvar_VariableIntegerValue("net_enabled")) {
        // Network has been disabled by a cvar
        return false;
    }

    if (!com_updatecheck_enabled->integer) {
        // Update checking has been disabled
        return false;
    }

#ifdef HAS_LIBCURL
    return true;
#else
    return false;
#endif
}

void UpdateChecker::SetLatestVersion(int major, int minor, int patch)
{
    lastMajor = major;
    lastMinor = minor;
    lastPatch = patch;

    versionChecked = true;
}

void UpdateChecker::Process()
{
    // Initialize the client thread when necessary
    CheckInitClientThread();

    std::chrono::time_point<std::chrono::steady_clock> currentTime = std::chrono::steady_clock::now();
    if (currentTime < nextMessageTime) {
        return;
    }

    if (!CheckNewVersion()) {
        return;
    }

    Com_Printf(
        "New release v%d.%d.%d published *\\(^ o ^)/*. Your current version is v%s. See www.openmohaa.org\n",
        lastMajor,
        lastMinor,
        lastPatch,
        PRODUCT_VERSION_NUMBER_STRING
    );

    nextMessageTime = currentTime + std::chrono::milliseconds(Q_max(1, com_updatecheck_interval->integer) * 60 * 1000);
}

void UpdateChecker::Shutdown()
{
    if (thread) {
        delete thread;
        thread = NULL;
    }
}

bool UpdateChecker::CheckNewVersion() const
{
    if (!versionChecked) {
        return false;
    }

    if (lastMajor > PRODUCT_VERSION_MAJOR) {
        return true;
    } else if (lastMajor < PRODUCT_VERSION_MAJOR) {
        return false;
    }

    if (lastMinor > PRODUCT_VERSION_MINOR) {
        return true;
    } else if (lastMinor < PRODUCT_VERSION_MINOR) {
        return false;
    }

    if (lastPatch > PRODUCT_VERSION_PATCH) {
        return true;
    }

    return false;
}

bool UpdateChecker::CheckNewVersion(int& major, int& minor, int& patch) const
{
    if (!CheckNewVersion()) {
        return false;
    }

    major = lastMajor;
    minor = lastMinor;
    patch = lastPatch;

    return true;
}

UpdateCheckerThread::UpdateCheckerThread()
{
    handle                = NULL;
    osThread              = NULL;
    requestThreadIsActive = qfalse;
    shouldBeActive        = qfalse;
}

UpdateCheckerThread::~UpdateCheckerThread()
{
    Shutdown();
}

void UpdateCheckerThread::Init()
{
    shouldBeActive        = qtrue;
    requestThreadIsActive = qtrue;

    osThread = new std::thread(&UpdateCheckerThread::RequestThread, this);
}

void UpdateCheckerThread::Shutdown()
{
    if (!shouldBeActive) {
        return;
    }

    shouldBeActive = qfalse;

    if (osThread) {
        // Notify and shutdown the thread
        clientWake.notify_all();
        osThread->join();

        delete osThread;
        osThread = NULL;
    }
}

bool UpdateCheckerThread::IsRoutineActive() const
{
    return requestThreadIsActive;
}

void UpdateCheckerThread::InitClient()
{
#ifdef HAS_LIBCURL
    CURLcode result;

    assert(!handle);

    handle = curl_easy_init();
    if (!handle) {
        Com_DPrintf("Failed to create curl client\n");
        return;
    }

    result = curl_easy_setopt(handle, CURLOPT_URL, "https://api.github.com/repos/openmoh/openmohaa/releases/latest");

    if (result != CURLE_OK) {
        Com_DPrintf("Failed to set curl URL: %s\n", curl_easy_strerror(result));
        curl_easy_cleanup(handle);
        handle = NULL;
        return;
    }

    curl_easy_setopt(handle, CURLOPT_USERAGENT, "curl");
#else
    Com_DPrintf("Project was compiled without libcurl, will not check for updates\n");
#endif
}

void UpdateCheckerThread::ShutdownClient()
{
#ifdef HAS_LIBCURL
    if (!handle) {
        return;
    }

    curl_easy_cleanup(handle);
    handle = NULL;
#endif
}

bool UpdateCheckerThread::ParseVersionNumber(const char *value, int& major, int& minor, int& patch) const
{
    const char *p  = value;
    const char *pn = value;

    if (*p != 'v') {
        return false;
    }

    //
    // Parse the major number
    //
    p++;
    for (pn = p; *pn && *pn != '.'; pn++) {};

    if (*pn != '.') {
        return false;
    }

    major = std::stoi(std::string(p, pn - p));

    //
    // Parse the minor number
    //
    p = pn + 1;
    for (pn = p; *pn && *pn != '.'; pn++) {};

    if (*pn != '.') {
        return false;
    }

    minor = std::stoi(std::string(p, pn - p));

    //
    // Parse the patch number
    //
    p = pn + 1;
    for (pn = p; *pn && *pn != '.'; pn++) {};

    if (*pn) {
        return false;
    }

    patch = std::stoi(std::string(p, pn - p));

    return true;
}

size_t WriteCallback(char *contents, size_t size, size_t nmemb, void *userp)
{
    std::string& responseString = *(std::string *)userp;
    responseString.append(contents, size * nmemb);

    return size * nmemb;
}

void UpdateCheckerThread::DoRequest()
{
#ifdef HAS_LIBCURL
    CURLcode    result;
    std::string responseString;

    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, &WriteCallback);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, &responseString);

    result = curl_easy_perform(handle);
    if (result != CURLE_OK) {
        return;
    }

    nlohmann::json data;

    try {
        data = nlohmann::json::parse(responseString);
    } catch (const std::exception& e) {
        return;
    }

    try {
        const nlohmann::json::string_t& tagName = data.at("tag_name");

        int major, minor, patch;
        ParseVersionNumber(tagName.c_str(), major, minor, patch);

        updateChecker.SetLatestVersion(major, minor, patch);
    } catch (std::out_of_range&) {}
#endif
}

void UpdateCheckerThread::RequestThread()
{
    // Initialize the curl client
    InitClient();

    while (handle && shouldBeActive) {
        DoRequest();
        RequestThreadSleep();
    }

    ShutdownClient();

    requestThreadIsActive = qfalse;
}

void UpdateCheckerThread::RequestThreadSleep()
{
    const std::chrono::seconds interval = std::chrono::seconds(Q_max(1, com_updatecheck_interval->integer) * 60);

    std::unique_lock<std::mutex> l(clientWakeMutex);
    clientWake.wait_for(l, interval);
}
