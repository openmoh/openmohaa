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

// DESCRIPTION:
// cg_servercmds_filter.h -- filtered server commands

#pragma once

#include "cg_local.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Determines whether or not the full statement is allowed.
 * The statement will be allowed if conditions are met:
 *  - Contains a whitelisted command
 *  - Trying to set a whitelisted variable
 *  - Trying to set variable that doesn't exist (user created variables)
 */
qboolean CG_IsStatementAllowed(char *cmd);

#ifdef __cplusplus
}
#endif