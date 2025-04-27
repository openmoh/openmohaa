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

// sv_gamespy.h -- Game-specific server GameSpy code

#pragma once

#include "q_gamespy.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char* GS_GetGameKey(unsigned int index);
extern unsigned int GS_GetGameID(unsigned int index);
extern const char* GS_GetGameName(unsigned int index);
extern const char* GS_GetCurrentGameKey();
extern unsigned int GS_GetCurrentGameID();
extern const char* GS_GetCurrentGameName();

void SV_CreateGamespyChallenge(char* challenge);
void SV_GamespyAuthorize(netadr_t from, const char* response);
void SV_GamespyHeartbeat();
void SV_ProcessGamespyQueries();
qboolean SV_InitGamespy();
void SV_ShutdownGamespy();
void SV_RestartGamespy();
void SV_RestartGamespy_f();
void SV_TryRestartGamespy();

#ifdef __cplusplus
}
#endif
