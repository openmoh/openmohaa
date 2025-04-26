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

#include "q_gamespy.h"
#include "q_shared.h"

master_entry_t entries[16] =
{
    { "master.333networks.com", 28900, 27900 },
    { "master.errorist.eu", 28900, 27900 }
};

int num_entries = 1;

void Com_InitGameSpy()
{
}

unsigned int Com_GetNumMasterEntries() {
    return num_entries;
}

const master_entry_t *Com_GetMasterEntry(int index) {
    if (index >= num_entries) {
        return NULL;
    }

    return &entries[index];
}

const char *Com_GetMasterHost()
{
    return entries[0].host;
}

int Com_GetMasterQueryPort()
{
    return entries[0].queryport;
}

int Com_GetMasterHeartbeatPort()
{
    return entries[0].hbport;
}
