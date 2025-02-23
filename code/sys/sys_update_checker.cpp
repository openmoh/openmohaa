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

#include <httplib.h>
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
    client                            = NULL;
    thread                            = NULL;
}

UpdateChecker::~UpdateChecker()
{
    if (client) {
        Shutdown();
    }
}

void UpdateChecker::Init()
{
    assert(!client);
    assert(!thread);

    try {
        httplib::Client *updateCheckerClient = new httplib::Client("https://api.github.com");

        client = updateCheckerClient;
        thread = new std::thread(&UpdateChecker::RequestThread, this);
    } catch (const std::exception& e) {
        client = NULL;
        thread = NULL;
        Com_DPrintf("Failed to create update checker: %s\n", e.what());
    }
}

void UpdateChecker::Process()
{
    std::chrono::time_point<std::chrono::steady_clock> currentTime = std::chrono::steady_clock::now();
    if (currentTime < lastMessageTime + std::chrono::milliseconds(std::max(1, com_updateCheckInterval->integer) * 60 * 1000)) {
        return;
    }

    if (!CheckNewVersion()) {
        return;
    }
    lastMessageTime = currentTime;

    Com_Printf(
        "New release v%d.%d.%d published *\\(^ o ^)/*. Your current version is v%s. See www.openmohaa.org\n",
        lastMajor,
        lastMinor,
        lastPatch,
        PRODUCT_VERSION_NUMBER_STRING
    );
}

void UpdateChecker::Shutdown()
{
    ShutdownClient();
    ShutdownThread();
}

void UpdateChecker::ShutdownClient()
{
    std::lock_guard<std::shared_mutex> l(clientMutex);

    if (!client) {
        return;
    }

    delete (httplib::Client *)client;
    client = NULL;
}

void UpdateChecker::ShutdownThread()
{
    if (!thread) {
        return;
    }

    thread->join();
    delete thread;
    thread = NULL;
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

bool UpdateChecker::ParseVersionNumber(const char *value, int& major, int& minor, int& patch) const
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

void UpdateChecker::DoRequest()
{
    std::lock_guard<std::shared_mutex> l(clientMutex);

    httplib::Client *updateCheckerClient = (httplib::Client *)client;

    if (!updateCheckerClient) {
        return;
    }

    httplib::Result result = updateCheckerClient->Get("/repos/openmoh/openmohaa/releases/latest");
    if (result.error() != httplib::Error::Success) {
        return;
    }
    httplib::Response response = result.value();
    nlohmann::json    data     = nlohmann::json::parse(response.body);

    try {
        const nlohmann::json::string_t& tagName = data.at("tag_name");

        int major, minor, patch;
        ParseVersionNumber(tagName.c_str(), major, minor, patch);

        lastMajor = major;
        lastMinor = minor;
        lastPatch = patch;

        versionChecked = true;
    } catch (std::out_of_range&) {}
}

void UpdateChecker::RequestThread()
{
    std::chrono::time_point<std::chrono::steady_clock> currentTime = std::chrono::steady_clock::now();
    std::chrono::time_point<std::chrono::steady_clock> lastCheckTime;

    while (client) {
        currentTime = std::chrono::steady_clock::now();
        if (currentTime >= lastCheckTime + std::chrono::milliseconds(std::max(1, com_updateCheckInterval->integer) * 60 * 1000)) {
            lastCheckTime = currentTime;

            DoRequest();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}
