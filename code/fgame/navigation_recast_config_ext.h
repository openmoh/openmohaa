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

/**
 * @brief Configuration
 * 
 */

#pragma once

#include "bg_public.h"

namespace NavigationMapExtensionConfiguration
{
    static const float agentJumpHeight = 56;

    static const float smallFallHeight = 100;
    static const float mediumFallHeight = 250;
    static const float maxFallHeight = 600;
}

// Areas
static constexpr unsigned char RECAST_AREA_LADDER = 32;
static constexpr unsigned char RECAST_AREA_STRAIGHT = 33;
static constexpr unsigned char RECAST_AREA_JUMP = 34;
static constexpr unsigned char RECAST_AREA_FALL = 35;
static constexpr unsigned char RECAST_AREA_MEDIUM_FALL = 36;
static constexpr unsigned char RECAST_AREA_HIGH_FALL = 37;
