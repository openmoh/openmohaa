/*
===========================================================================
Copyright (C) 2023 the OpenMoHAA team

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

#include "../qcommon/q_version.h"

#ifdef __cplusplus
extern "C" {
#endif

// Setting DEFAULT_BASEDIR to an empty string will make
// Sys_DefaultInstallPath() return the current directory instead
// This was the usual behavior in Quake III Arena.
#define DEFAULT_BASEDIR ""

void Sys_InitEx();
void Sys_ShutdownEx();

void Sys_PrepareBackTrace();
void Sys_PrintBackTrace();
void Sys_PlatformInit_New();

void Sys_UpdateChecker_Init();
void Sys_UpdateChecker_Process();
void Sys_UpdateChecker_Shutdown();

#ifdef __cplusplus
}
#endif