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
#include "../qcommon/q_shared.h"
#include "../qcommon/qcommon.h"

#define MAX_MASTERS 8
#define MASTER_DEFAULT_MSPORT 28900
#define MASTER_DEFAULT_HBPORT 27900

cvar_t *com_master_host[MAX_MASTERS];
cvar_t *com_master_msport[MAX_MASTERS];
cvar_t *com_master_hbport[MAX_MASTERS];

master_entry_t entries[MAX_MASTERS];
int num_entries = 0;

static void CreateMasterVar(int index, const char *host, int msport, int hbport) {
    com_master_host[index] = Cvar_Get(va("com_master%d_host", index), host, CVAR_INIT);
    com_master_msport[index] = Cvar_Get(va("com_master%d_msport", index), va("%d", msport), CVAR_INIT);
    com_master_hbport[index] = Cvar_Get(va("com_master%d_hbport", index), va("%d", hbport), CVAR_INIT);
}

void Com_InitGameSpy() {
    int i;

    //
    // These entries come from the 333networks community and use the same software
    // that emulate the GameSpy protocol -- see https://333networks.com/
    // These masters are managed by different entities, they are independent and are syncing eachother.
    //
    CreateMasterVar(0, "master.333networks.com", MASTER_DEFAULT_MSPORT, MASTER_DEFAULT_HBPORT);
    CreateMasterVar(1, "master.errorist.eu", MASTER_DEFAULT_MSPORT, MASTER_DEFAULT_HBPORT);
    CreateMasterVar(2, "master.noccer.de", MASTER_DEFAULT_MSPORT, MASTER_DEFAULT_HBPORT);
    CreateMasterVar(3, "master-au.unrealarchive.org", MASTER_DEFAULT_MSPORT, MASTER_DEFAULT_HBPORT);
    CreateMasterVar(4, "master.frag-net.com", MASTER_DEFAULT_MSPORT, MASTER_DEFAULT_HBPORT);

    for(i = 5; i < MAX_MASTERS; i++) {
        CreateMasterVar(i, "", MASTER_DEFAULT_MSPORT, MASTER_DEFAULT_HBPORT);
    }

    num_entries = 0;

    //
    // Find and insert valid entries
    //
    for(i = 0; i < MAX_MASTER_SERVERS; i++) {
        master_entry_t *entry = &entries[num_entries];

        if (com_master_host[i]->string && com_master_host[i]->string[0]) {
            entry->host = com_master_host[i]->string;
            entry->queryport = com_master_msport[i]->integer;
            entry->hbport = com_master_hbport[i]->integer;
            num_entries++;
        }
    }
}

unsigned int Com_GetNumMasterEntries()
{
    return num_entries;
}

void Com_GetMasterEntry(int index, master_entry_t* entry)
{
    if (index >= num_entries) {
        entry->host = NULL;
        entry->hbport = 0;
        entry->queryport = 0;
        return;
    }

    entry->host = com_master_host[index]->string;
    entry->queryport = com_master_msport[index]->integer;
    entry->hbport = com_master_hbport[index]->integer;
}

const char *Com_GetMasterHost()
{
    return com_master_host[0]->string;
}

int Com_GetMasterQueryPort()
{
    return com_master_msport[0]->integer;
}

int Com_GetMasterHeartbeatPort()
{
    return com_master_hbport[0]->integer;
}
